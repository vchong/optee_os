/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 *
 */

#ifndef __TPM2_H__
#define __TPM2_H__

#include <stdlib.h>
#include <sys/queue.h>
#include <types_ext.h>

// TODO: use SHIFT
#define TPM_ACCESS(l)                   (0x0000 | ((l) << 12))
#define TPM_INT_ENABLE(l)               (0x0008 | ((l) << 12))
#define TPM_STS(l)                      (0x0018 | ((l) << 12))
#define TPM_DATA_FIFO(l)                (0x0024 | ((l) << 12))
#define TPM_DID_VID(l)                  (0x0f00 | ((l) << 12))
#define TPM_RID(l)                      (0x0f04 | ((l) << 12))
#define TPM_INTF_CAPS(l)                (0x0014 | ((l) << 12))

enum tpm2_access {
	TPM2_ACCESS_VALID		= 0x80,
	TPM2_ACCESS_ACTIVE_LOCALITY	= 0x20,
	TPM2_ACCESS_REQUEST_PENDING	= 0x04,
	TPM2_ACCESS_REQUEST_USE		= 0x02,
};

enum tpm2_timeout {
	TPM2_TIMEOUT_MS			= 5,
	TPM2_SHORT_TIMEOUT_MS		= 750,
	TPM2_LONG_TIMEOUT_MS		= 2000,
	TPM2_SLEEP_DURATION_US		= 60,
	TPM2_DURATION_LONG_US		= 210,
};

enum tpm2_int_flags {
	TPM2_GLOBAL_INT_ENABLE = 0x80000000,
	TPM2_INTF_BURST_COUNT_STATIC = 0x100,
	TPM2_INTF_CMD_READY_INT = 0x080,
	TPM2_INTF_INT_EDGE_FALLING = 0x040,
	TPM2_INTF_INT_EDGE_RISING = 0x020,
	TPM2_INTF_INT_LEVEL_LOW = 0x010,
	TPM2_INTF_INT_LEVEL_HIGH = 0x008,
	TPM2_INTF_LOCALITY_CHANGE_INT = 0x004,
	TPM2_INTF_STS_VALID_INT = 0x002,
	TPM2_INTF_DATA_AVAIL_INT = 0x001,
};

struct tpm2_ops {
	int (*rx32)(struct tpm2_chip *chip, uint32_t addr, uint32_t *result);
	int (*tx32)(struct tpm2_chip *chip, uint32_t addr, uint32_t src);
	int (*rx8)(struct tpm2_chip *chip, uint32_t addr, uint16_t len,
		   uint8_t *result);
	int (*tx8)(struct tpm2_chip *chip, uint32_t addr, uint16_t len,
		   const uint8_t *value);
};

struct tpm2_chip {
	const struct tpm2_ops *ops;
	uint64_t chip_type;
	/* ms */
	uint64_t timeout_a;
	uint64_t timeout_b;
	uint64_t timeout_c;
	uint64_t timeout_d;
	uint32_t vend_dev;
	int32_t is_open;
	int32_t locality;
	uint8_t rid;
};

struct tpm2_data {
	struct tpm2_chip *chip;
}

#endif	/* __TPM2_H__ */

