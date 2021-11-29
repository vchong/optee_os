/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 *
 */

#include <assert.h>
#include <initcall.h>
#include <io.h>
#include <keep.h>
#include <kernel/panic.h>
#include <kernel/tee_time.h>
//#include <platform_config.h>
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

	ops->rx8(chip, TPM_ACCESS(loc), 1, &locality);
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
	uint64_t start, stop;

	if (tpm2_check_locality(chip, loc))
		return 0;

	ops->tx8(chip, TPM_ACCESS(loc), 1, &buf);
	start = get_timer(0);
	stop = chip->timeout_a;
	do {
		if (tpm2_check_locality(chip, loc))
			return 0;
		mdelay(TPM2_TIMEOUT_MS);
	} while (get_timer(start) < stop);
	// TODO: impl get_timer()

	return -1;
}

int tpm_tis_release_locality(struct tpm2_chip *chip, int loc)
{
	struct tpm2_ops *ops = chip->ops;
	uint8_ buf = TPM2_ACCESS_ACTIVE_LOCALITY;
	int ret;

	if (chip->locality < 0)
		return 0;

	ret = ops->tx8(chip, TPM_ACCESS(loc), 1, &buf);
	chip->locality = -1;

	return ret;
}

int tpm2_init(struct tpm2_chip *chip)
{
	int ret;
	u32 tmp;
	tpm2_ops *ops = chip->ops;

	if (!tpm2_check_ops(ops)) {
		EMSG("No read write functions defined\n");
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
	chip->ops->rx32(chip, TPM_INT_ENABLE(chip->locality), &tmp);
	tmp |= TPM2_INTF_CMD_READY_INT | TPM2_INTF_LOCALITY_CHANGE_INT |
	       TPM2_INTF_DATA_AVAIL_INT | TPM2_INTF_STS_VALID_INT;
	tmp &= ~TPM2_GLOBAL_INT_ENABLE;
	chip->ops->tx32(chip, TPM_INT_ENABLE(chip->locality), tmp);

	chip->ops->rx8(chip, TPM_RID(chip->locality), 1, &chip->rid);
	chip->ops->rx32(chip, TPM_DID_VID(chip->locality), &chip->vend_dev);

	return tpm2_release_locality(chip, chip->locality);
}

