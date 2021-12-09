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
#include <util.h>

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

static enum tpm2_result tpm2_get_locality(struct tpm2_chip *chip, int loc)
{
	struct tpm2_ops *ops = chip->ops;
	uint8_t buf = TPM2_ACCESS_REQUEST_USE;
	unsigned long timeout = chip->timeout_a;
	unsigned long timeout_prev = 0;

	/* first check if locality exists */
	if (tpm2_check_locality(chip, loc))
		return TPM2_OK;

	/* if not get one */
	ops->tx8(chip, TPM2_ACCESS(loc), 1, &buf);
	do {
		/* keep trying to get one until timeout */
		if (tpm2_check_locality(chip, loc))
			return TPM2_OK;
		mdelay(TPM2_TIMEOUT_MS);
		/*
		 * Use timeout_prev in case timeout
		 * becomes a -ve number, i.e. a big
		 * +ve number.
		 */
		timeout_prev = timeout;
		timeout -= TPM2_TIMEOUT_MS;
	} while (timeout > 0 || timeout < timeout_prev);

	return TPM2_ERROR_GENERIC;
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
		EMSG("TPM2: No rx tx functions defined\n");
		return TPM2_ERROR_GENERIC;
	}
	ret = tpm2_get_locality(chip, 0);
	if (ret)
		return ret;

	chip->timeout_a = TPM2_SHORT_TIMEOUT_MS;
	chip->timeout_b = TPM2_LONG_TIMEOUT_MS;
	chip->timeout_c = TPM2_SHORT_TIMEOUT_MS;
	chip->timeout_d = TPM2_SHORT_TIMEOUT_MS;

	/* Disable interrupts */
	chip->ops->rx32(chip, TPM2_INT_ENABLE(chip->locality), &flags);
	flags |= TPM2_INTF_CMD_READY_INT | TPM2_INTF_LOCALITY_CHANGE_INT |
		 TPM2_INTF_DATA_AVAIL_INT | TPM2_INTF_STS_VALID_INT;
	flags &= ~TPM2_GLOBAL_INT_ENABLE;
	chip->ops->tx32(chip, TPM2_INT_ENABLE(chip->locality), flags);

	chip->ops->rx8(chip, TPM2_RID(chip->locality), 1, &chip->rid);
	chip->ops->rx32(chip, TPM2_DID_VID(chip->locality), &chip->vend_dev);

	return tpm2_free_locality(chip);
}

static enum tpm2_result tpm2_ready(struct tpm2_chip *chip)
{
	struct tpm2_ops *ops = chip->ops;
	uint8_t buf = TPM2_STS_COMMAND_READY;

	/*
	 * cancel all pending commands and
	 * put module on ready
	 */
	return ops->tx8(chip, TPM2_STS(chip->locality), 1, &buf);
}

enum tpm2_result tpm2_end(struct tpm2_chip *chip)
{
	tpm2_ready(chip);
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
		EMSG("TPM2: invalid status\n");
		return TPM2_ERROR_INVALID_ARG;
	}

	return TPM2_OK;
}

enum tpm2_result tpm2_tx(struct tpm2_chip *chip, uint8_t *buf, size_t len)
{
	enum tpm2_result ret = TPM2_OK;
	size_t burstcnt = 0;
	size_t sent = 0;
	size_t wr_size = 0;
	struct tpm2_ops *phy_ops = chip->ops;
	uint8_t data = TPM2_STS_GO;
	uint8_t status;

	if (!chip)
		return TPM2_ERROR_GENERIC;

	ret = tpm2_get_locality(chip, 0);
	if (ret)
		return ret;

	ret = tpm_tis_status(dev, &status);
	if (ret)
		goto release_locality;

	if (!(status & TPM_STS_COMMAND_READY)) {
		ret = tpm_tis_ready(dev);
		if (ret) {
			log_err("Can't cancel previous TPM operation\n");
			goto release_locality;
		}
		ret = tpm_tis_wait_for_stat(dev, TPM_STS_COMMAND_READY,
					    chip->timeout_b, &status);
		if (ret) {
			log_err("TPM not ready\n");
			goto release_locality;
		}
	}

	while (len > 0) {
		ret = tpm_tis_get_burstcount(dev, &burstcnt);
		if (ret)
			goto release_locality;

		wr_size = min(len, burstcnt);
		ret = phy_ops->write_bytes(dev, TPM_DATA_FIFO(chip->locality),
					   wr_size, buf + sent);
		if (ret < 0)
			goto release_locality;

		ret = tpm_tis_wait_for_stat(dev, TPM_STS_VALID,
					    chip->timeout_c, &status);
		if (ret)
			goto release_locality;

		sent += wr_size;
		len -= wr_size;
		/* make sure the TPM expects more data */
		if (len && !(status & TPM_STS_DATA_EXPECT)) {
			ret = -EIO;
			goto release_locality;
		}
	}

	/*
	 * Make a final check ensuring everything is ok and the TPM expects no
	 * more data
	 */
	ret = tpm_tis_wait_for_stat(dev, TPM_STS_VALID, chip->timeout_c,
				    &status);
	if (ret)
		goto release_locality;

	if (status & TPM_STS_DATA_EXPECT) {
		ret = -EIO;
		goto release_locality;
	}

	ret = phy_ops->write_bytes(dev, TPM_STS(chip->locality), 1, &data);
	if (ret)
		goto release_locality;

	return sent;

release_locality:
	tpm_tis_ready(dev);
	tpm_tis_release_locality(dev, chip->locality);

	return ret;
}

enum tpm2_result tpm2_rx(struct tpm2_chip *chip, uint8_t *buf, size_t len)
{
}

