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

#define TPM2_ACCESS(l)		(0x0000 | SHIFT_U32(l, 12))
#define TPM2_INT_ENABLE(l)	(0x0008 | SHIFT_U32(l, 12))
#define TPM2_INTF_CAPS(l)	(0x0014 | SHIFT_U32(l, 12))
#define TPM2_STS(l)		(0x0018 | SHIFT_U32(l, 12))
#define TPM2_DATA_FIFO(l)	(0x0024 | SHIFT_U32(l, 12))
#define TPM2_DID_VID(l)		(0x0F00 | SHIFT_U32(l, 12))
#define TPM2_RID(l)		(0x0F04 | SHIFT_U32(l, 12))

enum tpm2_access {
	TPM2_ACCESS_VALID = 0x80,
	TPM2_ACCESS_ACTIVE_LOCALITY = 0x20,
	TPM2_ACCESS_REQUEST_PENDING = 0x04,
	TPM2_ACCESS_REQUEST_USE = 0x02,
};

enum tpm2_timeout {
	TPM2_TIMEOUT_MS = 5,
	TPM2_SHORT_TIMEOUT_MS = 750,
	TPM2_LONG_TIMEOUT_MS = 2000,
	TPM2_SLEEP_DURATION_US = 60,
	TPM2_DURATION_LONG_US = 210,
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

enum tpm2_status {
	TPM2_STS_VALID = 0x80,
	TPM2_STS_COMMAND_READY = 0x40,
	TPM2_STS_GO = 0x20,
	TPM2_STS_DATA_AVAIL = 0x10,
	TPM2_STS_DATA_EXPECT = 0x08,
};

enum {
	TPM2_STS_FAMILY_SHIFT = 26,
	TPM2_STS_FAMILY_MASK = SHIFT_U32(0x3, TPM2_STS_FAMILY_SHIFT),
	TPM2_STS_FAMILY_TPM2 = BIT32(TPM2_STS_FAMILY_SHIFT),
	TPM2_STS_RESE_TESTABLISMENT_BIT = BIT32(25),
	TPM2_STS_COMMAND_CANCEL = BIT32(24),
	TPM2_STS_BURST_COUNT_SHIFT = 8,
	TPM2_STS_BURST_COUNT_MASK =
		SHIFT_U32(0xFFFF, TPM2_STS_BURST_COUNT_SHIFT),
	TPM2_STS_VALID = BIT32(7),
	TPM2_STS_COMMAND_READY = BIT32(6),
	TPM2_STS_GO = BIT32(5),
	TPM2_STS_DATA_AVAIL = BIT32(4),
	TPM2_STS_DATA_EXPECT = BIT32(3),
	TPM2_STS_SELF_TEST_DONE = BIT32(2),
	TPM2_STS_RESPONSE_RETRY = BIT32(1),
	TPM2_STS_READ_ZERO = 0x23,
};

enum tpm2_result {
	TPM2_OK = 0,
	TPM2_ERROR_GENERIC = -1,
	TPM2_ERROR_INVALID_ARG = -2,
	TPM2_ERROR_BUSY = -3,
};

struct tpm2_ops {
	enum tpm2_result (*rx32)(struct tpm2_chip *chip, uint32_t adr,
				 uint32_t *buf);
	enum tpm2_result (*tx32)(struct tpm2_chip *chip, uint32_t adr,
				 uint32_t val);
	enum tpm2_result (*rx8)(struct tpm2_chip *chip, uint32_t adr,
				uint16_t len, uint8_t *buf);
	enum tpm2_result (*tx8)(struct tpm2_chip *chip, uint32_t adr,
				uint16_t len, uint8_t *buf);
};

struct tpm2_chip {
	struct tpm2_ops *ops;
	unsigned long chip_type;
	unsigned long timeout_a;
	unsigned long timeout_b;
	unsigned long timeout_c;
	unsigned long timeout_d;
	uint32_t vend_dev;
	int32_t is_open;
	int32_t locality;
	uint8_t rid;
};

#endif	/* __TPM2_H__ */

