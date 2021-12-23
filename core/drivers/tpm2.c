/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 *
 */

#include <assert.h>
#include <delay.h>
#include <initcall.h>
#include <io.h>
#include <keep.h>
#include <kernel/panic.h>
#include <kernel/tee_time.h>
#include <platform_config.h>
#include <tpm2.h>
#include <trace.h>

static bool tpm2_check_ops(struct tpm2_ops *ops)
{
	if (!ops || !ops->rx32 || !ops->tx32 || !ops->rx8 || !ops->tx8)
		return false;

	return true;
}

static bool tpm2_check_locality(struct tpm2_chip *chip, int loc)
{
	struct tpm2_ops *ops = chip->ops;
	uint8_t locality;

	ops->rx8(chip, TPM2_ACCESS(loc), 1, &locality);
	if ((locality & (TPM2_ACCESS_ACTIVE_LOCALITY | TPM2_ACCESS_VALID |
	     TPM2_ACCESS_REQUEST_USE)) ==
	    (TPM2_ACCESS_ACTIVE_LOCALITY | TPM2_ACCESS_VALID)) {
		chip->locality = loc;
		return true;
	}

	return false;
}

static void tpm2_delay(unsigned long *to, unsigned long *to_prev)
{
	mdelay(TPM2_TIMEOUT_MS);
	/*
	 * Use timeout_prev in case timeout
	 * becomes a -ve number, i.e. a big
	 * +ve number.
	 */
	to_prev = to;
	to -= TPM2_TIMEOUT_MS;
}

static enum tpm2_result tpm2_get_locality(struct tpm2_chip *chip, int loc)
{
	struct tpm2_ops *ops = chip->ops;
	uint8_t buf = TPM2_ACCESS_REQUEST_USE;
	unsigned long timeout = chip->timeout_a;
	unsigned long timeout_prev = timeout;

	/* first check if locality exists */
	if (tpm2_check_locality(chip, loc))
		return TPM2_OK;

	/* if not get one */
	ops->tx8(chip, TPM2_ACCESS(loc), 1, &buf);
	do {
		/* keep trying to get one until timeout */
		if (tpm2_check_locality(chip, loc))
			return TPM2_OK;

		tpm2_delay(&timeout, &timeout_prev);
	} while (timeout != 0 || timeout < timeout_prev);

	return TPM2_ERROR_TIMEOUT;
}

enum tpm2_result tpm2_free_locality(struct tpm2_chip *chip)
{
	enum tpm2_result ret = TPM2_OK;
	struct tpm2_ops *ops = chip->ops;
	uint8_t buf = TPM2_ACCESS_ACTIVE_LOCALITY;

	if (chip->locality < 0)
		return TPM2_OK;

	ret = ops->tx8(chip, TPM2_ACCESS(chip->locality), 1, &buf);
	chip->locality = -1;

	return ret;
}

enum tpm2_result tpm2_start(struct tpm2_chip *chip)
{
	enum tpm2_result ret = TPM2_OK;
	tpm2_ops *ops = chip->ops;
	uint32_t flags;

	if (!tpm2_check_ops(ops)) {
		EMSG("No rx tx functions defined");
		return TPM2_ERROR_GENERIC;
	}
	ret = tpm2_get_locality(chip, 0);
	if (ret)
		return ret;

	chip->timeout_a = TPM2_SHORT_TIMEOUT_MS;
	chip->timeout_b = TPM2_LONG_TIMEOUT_MS;
	chip->timeout_c = TPM2_SHORT_TIMEOUT_MS;
	chip->timeout_d = TPM2_SHORT_TIMEOUT_MS;

	/* disable interrupts */
	chip->ops->rx32(chip, TPM2_INT_ENABLE(chip->locality), &flags);
	flags |= TPM2_INTF_CMD_READY_INT | TPM2_INTF_LOCALITY_CHANGE_INT |
		 TPM2_INTF_DATA_AVAIL_INT | TPM2_INTF_STS_VALID_INT;
	flags &= ~TPM2_GLOBAL_INT_ENABLE;
	chip->ops->tx32(chip, TPM2_INT_ENABLE(chip->locality), flags);

	chip->ops->rx8(chip, TPM2_RID(chip->locality), 1, &chip->rid);
	chip->ops->rx32(chip, TPM2_DID_VID(chip->locality), &chip->vend_dev);

	return tpm2_free_locality(chip);
}

static enum tpm2_result tpm2_get_ready(struct tpm2_chip *chip)
{
	struct tpm2_ops *ops = chip->ops;
	uint8_t buf = TPM2_STS_COMMAND_READY;

	/*
	 * Cancel all pending commands and
	 * put module on ready.
	 */
	return ops->tx8(chip, TPM2_STS(chip->locality), 1, &buf);
}

enum tpm2_result tpm2_end(struct tpm2_chip *chip)
{
	tpm2_get_ready(chip);
	tpm2_free_locality(chip);

	return TPM2_OK;
}

enum tpm2_result tpm2_open(struct tpm2_chip *chip)
{
	enum tpm2_result ret = TPM2_OK;

	if (chip->is_open)
		return TPM2_ERROR_GENERIC;

	ret = tpm2_get_locality(chip, 0);
	if (!ret)
		chip->is_open = 1;

	return ret;
}

enum tpm2_result tpm2_close(struct tpm2_chip *chip)
{
	enum tpm2_result ret = TPM2_OK;

	if (chip->is_open) {
		ret = tpm2_free_locality(chip);
		chip->is_open = 0;
	}

	return ret;
}

static enum tpm2_result tpm2_get_status(struct tpm2_chip *chip, uint8_t *status)
{
	struct tpm2_ops *ops = chip->phy_ops;

	if (chip->locality < 0)
		return TPM2_ERROR_INVALID_ARG;

	ops->rx8(chip, TPM2_STS(chip->locality), 1, status);

	if ((*status & TPM2_STS_READ_ZERO)) {
		EMSG("Invalid status");
		return TPM2_ERROR_INVALID_ARG;
	}

	return TPM2_OK;
}

static enum tpm2_result tpm2_wait_for_status(struct tpm2_chip *chip,
					     uint8_t mask,
					     unsigned long timeout,
					     uint8_t *status)
{
	enum tpm2_result ret = TPM2_OK;
	unsigned long timeout_prev = timeout;

	do {
		ret = tpm2_get_status(chip, status);
		if (ret)
			return ret;

		if ((*status & mask) == mask)
			return TPM2_OK;

		tpm2_delay(&timeout, &timeout_prev);
	} while (timeout != 0 || timeout < timeout_prev);

	return TPM2_ERROR_TIMEOUT;
}

static enum tpm2_result tpm2_get_burstcount(struct tpm2_chip *chip,
					    size_t *burstcount)
{
	struct tpm2_ops *ops = chip->ops;
	unsigned long timeout = chip->timeout_a;
	unsigned long timeout_prev = timeout;
	uint32_t burst;

	if (chip->locality < 0)
		return TPM2_ERROR_INVALID_ARG;

	/* wait for burstcount */
	do {
		ops->rx32(chip, TPM2_STS(chip->locality), &burst);
		*burstcount = (burst >> 8) & 0xFFFF;
		if (*burstcount)
			return TPM2_OK;

		tpm2_delay(&timeout, &timeout_prev);
	} while (timeout != 0 || timeout < timeout_prev);

	return TPM2_ERROR_TIMEOUT;
}


enum tpm2_result tpm2_tx(struct tpm2_chip *chip, uint8_t *buf, size_t len)
{
	enum tpm2_result ret = TPM2_OK;
	size_t burstcnt = 0;
	size_t sent = 0;
	size_t wr_size = 0;
	struct tpm2_ops *ops = chip->ops;
	uint8_t data = TPM2_STS_GO;
	uint8_t status = 0;

	if (!chip)
		return TPM2_ERROR_GENERIC;

	/* free in tpm2_rx */
	ret = tpm2_get_locality(chip, 0);
	if (ret)
		return ret;

	ret = tpm2_get_status(chip, &status);
	if (ret)
		goto free_locality;

	if (!(status & TPM2_STS_COMMAND_READY)) {
		ret = tpm2_get_ready(chip);
		if (ret) {
			EMSG("Previous cmd cancel failed");
			goto free_locality;
		}
		ret = tpm2_wait_for_status(chip, TPM2_STS_COMMAND_READY,
					   chip->timeout_b, &status);
		if (ret) {
			EMSG("Module not ready\n");
			goto free_locality;
		}
	}

	while (len > 0) {
		ret = tpm2_get_burstcount(chip, &burstcnt);
		if (ret)
			goto free_locality;

		wr_size = MIN(len, burstcnt);
		ret = ops->tx8(chip, TPM2_DATA_FIFO(chip->locality), wr_size,
			       buf + sent);
		if (ret < 0)
			goto free_locality;

		ret = tpm2_wait_for_status(chip, TPM2_STS_VALID,
					   chip->timeout_c, &status);
		if (ret)
			goto free_locality;

		sent += wr_size;
		len -= wr_size;
		/* TPM2 should expect more data */
		if (len && !(status & TPM2_STS_DATA_EXPECT)) {
			ret = TPM2_ERROR_IO;
			goto free_locality;
		}
	}

	/* last check everything is ok and TPM2 expects no more data */
	ret = tpm2_wait_for_status(chip, TPM2_STS_VALID, chip->timeout_c,
				   &status);
	if (ret)
		goto free_locality;

	if (status & TPM2_STS_DATA_EXPECT) {
		ret = TPM2_ERROR_IO;
		goto free_locality;
	}

	ret = ops->tx8(chip, TPM2_STS(chip->locality), 1, &data);
	if (ret)
		goto free_locality;

	return sent;

free_locality:
	tpm2_get_ready(chip);
	tpm2_free_locality(chip);

	return ret;
}

static enum tpm2_result tpm2_rx_dat(struct tpm2_chip *chip, uint8_t *buf,
				    size_t len)
{
	enum tpm2_result ret = TPM2_OK;
	enum tpm2_result size = TPM2_OK;
	int len = 0;
	size_t burstcnt = 0;
	struct tpm2_ops *ops = chip->ops;
	uint8_t status = 0;

	while (size < len &&
	       tpm2_wait_for_status(chip, TPM2_STS_DATA_AVAIL | TPM2_STS_VALID,
				    chip->timeout_c, &status) == 0) {
		ret = tpm2_get_burstcount(chip, &burstcnt);
		if (ret)
			return ret;

		len = MIN(burstcnt, len - size)
		ret = ops->rx8(chip, TPM2_DATA_FIFO(chip->locality), len,
			       buf + size);
		if (ret)
			return ret;

		size += len;
	}

	return size;
}

static int tpm2_convert2be(uint8_t *buf)
{
	return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
}

enum tpm2_result tpm2_rx(struct tpm2_chip *chip, uint8_t *buf, size_t len)
{
	enum tpm2_result size = 0;
	int expected = 0;

	if (len < TPM2_HEADER_SIZE)
		return TPM2_ERROR_ARG_LIST_TOO_LONG;

	size = tpm2_rx_dat(chip, buf, TPM2_HEADER_SIZE);
	if (size < TPM2_HEADER_SIZE) {
		EMSG("Unable to read TPM2 header\n");
		goto out;
	}

	expected = tpm2_convert2be(buf + TPM2_CMD_COUNT_OFFSET);
	if (expected > len) {
		size = TPM2_ERROR_IO;
		EMSG("Too much data: %d > %zu", expected, len);
		goto out;
	}

	size += tpm2_rx_dat(chip, &buf[TPM2_HEADER_SIZE],
			    expected - TPM2_HEADER_SIZE);
	if (size < expected) {
		EMSG("Unable to rx remaining data");
		size = TPM2_ERROR_IO;
		goto out;
	}

out:
	tpm2_get_ready(chip);
	/* gotten from tpm2_tx */
	tpm2_free_locality(chip);

	return size;
}

