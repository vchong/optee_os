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

static int tpm2_get_locality(struct tpm2_chip *chip, int loc)
{
	struct tpm2_ops *ops = chip->ops;
	uint8_t buf = TPM2_ACCESS_REQUEST_USE;
	unsigned long timeout = chip->timeout_a;
	unsigned long timeout_prev = 0;

	/* first check if locality exists */
	if (tpm2_check_locality(chip, loc))
		return 0;

	/* if not get one */
	ops->tx8(chip, TPM2_ACCESS(loc), 1, &buf);
	do {
		/* keep trying to get one until timeout */
		if (tpm2_check_locality(chip, loc))
			return 0;
		mdelay(TPM2_TIMEOUT_MS);
		/*
		 * Use timeout_prev in case timeout
		 * becomes a -ve number, i.e. a big
		 * +ve number.
		 */
		timeout_prev = timeout;
		timeout -= TPM2_TIMEOUT_MS;
	} while (timeout > 0 || timeout < timeout_prev);

	return -1;
}

int tpm2_free_locality(struct tpm2_chip *chip)
{
	struct tpm2_ops *ops = chip->ops;
	uint8_t buf = TPM2_ACCESS_ACTIVE_LOCALITY;
	int ret;

	if (chip->locality < 0)
		return 0;

	ret = ops->tx8(chip, TPM2_ACCESS(chip->locality), 1, &buf);
	chip->locality = -1;

	return ret;
}

int tpm2_start(struct tpm2_chip *chip)
{
	int ret;
	tpm2_ops *ops = chip->ops;
	uint32_t flags;

	if (!tpm2_check_ops(ops)) {
		EMSG("TPM2: No rx tx functions defined\n");
		return -1;
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

static int tpm2_ready(struct tpm2_chip *chip)
{
	struct tpm2_ops *ops = chip->ops;
	uint8_t buf = TPM2_STS_COMMAND_READY;

	/*
	 * cancel all pending commands and
	 * put module on ready
	 */
	return ops->tx8(chip, TPM2_STS(chip->locality), 1, &buf);
}

int tpm2_end(struct tpm2_chip *chip)
{
	tpm2_ready(chip);
	tpm2_free_locality(chip);

	return 0;
}

int tpm2_open(struct tpm2_chip *chip)
{
	int ret;

	if (chip->is_open)
		return -1;

	ret = tpm2_get_locality(chip, 0);
	if (!ret)
		chip->is_open = 1;

	return ret;
}

int tpm2_close(struct tpm2_chip *chip)
{
	int ret = 0;

	if (chip->is_open) {
		ret = tpm2_free_locality(chip);
		chip->is_open = 0;
	}

	return ret;
}

/**
 * tpm_tis_status - Check the current device status
 *
 * @dev:   TPM device
 * @status: return value of status
 *
 * Return: 0 on success, negative on failure
 */
static int tpm2_get_status(struct tpm2_chip *chip, uint8_t *status)
{
	struct tpm2_ops *ops = chip->phy_ops;

	if (chip->locality < 0)
		return -1;

	ops->rx8(chip, TPM2_STS(chip->locality), 1, status);

	if ((*status & TPM2_STS_READ_ZERO)) {
		EMSG("TPM2: invalid status\n");
		return -1;
	}

	return 0;
}

int tpm2_tx(struct tpm2_chip *chip, uint8_t *buf, size_t len)
{
}

int tpm2_rx(struct tpm2_chip *chip, uint8_t *buf, size_t len)
{
}

