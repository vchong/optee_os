/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 *
 */

#ifndef __TPM2_H__
#define __TPM2_H__

#include <stdint.h>
#include <types_ext.h>
#include <util.h>

/* from Ruchi's u-boot log */
#define TPM2_REG_SIZE 0x5000

#define TPM2_HDR_LEN 10

#define TPM2_ACCESS(l)		 SHIFT_U32(l, 12)
#define TPM2_INT_ENABLE(l)	(SHIFT_U32(l, 12) | BIT32(3))
#define TPM2_STS(l)		(SHIFT_U32(l, 12) | SHIFT_U32(3,3))
#define TPM2_INT_CAPS(l)	(SHIFT_U32(l, 12) | SHIFT_U32(5,2))
#define TPM2_DATA_FIFO(l)	(SHIFT_U32(l, 12) | SHIFT_U32(9,2))
#define TPM2_DID_VID(l)		(SHIFT_U32(l, 12) | SHIFT_U32(0xF,8))
#define TPM2_RID(l)		(SHIFT_U32(l, 12) | SHIFT_U32(0x3C1,2))

//
enum tpm2_int_flags {
	TPM2_GLOBAL_INT_ENABLE = BIT32(31),
	TPM2_INT_BURST_COUNT_STATIC = BIT32(8),
	TPM2_INT_CMD_READY_INT = BIT32(7),
	TPM2_INT_INT_EDGE_FALLING = BIT32(6),
	TPM2_INT_INT_EDGE_RISING = BIT32(5),
	TPM2_INT_INT_LEVEL_LOW = BIT32(4),
	TPM2_INT_INT_LEVEL_HIGH = BIT32(3),
	TPM2_INT_LOCALITY_CHANGE_INT = BIT32(2),
	TPM2_INT_STS_VALID_INT = BIT32(1),
	TPM2_INT_DATA_AVAIL_INT = BIT32(0),
};

enum tpm2_result {
	TPM2_OK = 0,
	TPM2_ERROR_GENERIC = -1,
	TPM2_ERROR_INVALID_ARG = -2,
	TPM2_ERROR_BUSY = -3,
	TPM2_ERROR_TIMEOUT = -4,
	TPM2_ERROR_IO = -5,
	TPM2_ERROR_ARG_LIST_TOO_LONG = -6,
};

//
enum tpm2_timeout {
	TPM2_TIMEOUT_MS = 5,
	TPM2_SLEEP_DURATION_US = 60,
	TPM2_DURATION_LONG_US = 210,
	TPM2_SHORT_TIMEOUT_MS = 750,
	TPM2_LONG_TIMEOUT_MS = 2000,
};

enum {
	TPM2_ACCESS_VALID = BIT(7),
	TPM2_ACCESS_ACTIVE_LOCALITY = BIT(5),
	TPM2_ACCESS_REQUEST_PENDING = BIT(2),
	TPM2_ACCESS_REQUEST_USE = BIT(1),
	TPM2_ACCESS_ESTABLISHMENT = BIT(0),
};

enum {
	TPM2_STS_FAMILY_SHIFT = SHIFT_U32(0xD, 1),
	TPM2_STS_FAMILY_MASK = SHIFT_U32(3, TPM2_STS_FAMILY_SHIFT),
	TPM2_STS_FAMILY_TPM2 = BIT32(TPM2_STS_FAMILY_SHIFT),
	TPM2_STS_RESE_TESTABLISMENT_BIT = BIT32(25),
	TPM2_STS_COMMAND_CANCEL = BIT32(24),
	TPM2_STS_BURST_COUNT_SHIFT = BIT32(3),
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

enum {
	TPM2_CMD_COUNT_OFFSET = BIT32(1),
	TPM2_CMD_ORDINAL_OFFSET = SHIFT_U32(3, 1),
	TPM2_MAX_BUF_SIZE = 1260,
};

struct tpm2_chip {
	struct tpm2_ops *ops;
	unsigned long type;
	unsigned long timeout_a;
	unsigned long timeout_b;
	unsigned long timeout_c;
	unsigned long timeout_d;
	uint32_t vend_dev;
	int32_t is_open;
	int32_t locality;
	uint8_t rid;
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

enum tpm2_result tpm2_init(struct tpm2_chip *chip);
enum tpm2_result tpm2_end(struct tpm2_chip *chip);
enum tpm2_result tpm2_open(struct tpm2_chip *chip);
enum tpm2_result tpm2_close(struct tpm2_chip *chip);
enum tpm2_result tpm2_tx(struct tpm2_chip *chip, uint8_t *buf, size_t len);
enum tpm2_result tpm2_rx(struct tpm2_chip *chip, uint8_t *buf, size_t len);

#endif	/* __TPM2_H__ */

