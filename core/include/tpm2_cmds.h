/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 *
 */

#ifndef __TPM2_CMDS_H__
#define __TPM2_CMDS_H__

#include <tpm2.h>

#define TPM2_OFFSET_CMD_SIZE 2
#define TPM2_OFFSET_RSP_CODE 6

enum tpm2_cmd {
	TPM2_CMD_SELF_TEST = 0x0143,
	TPM2_CMD_STARTUP = 0x0144,
	TPM2_CMD_PCR_READ = 0x017E,
	TPM2_CMD_PCR_EXTEND = 0x0182,
};

enum tpm2_sessions {
	TPM2_ST_NO_SESSIONS = 0x8001,
	TPM2_ST_SESSIONS = 0x8002,
};

enum tpm2_startup_state {
	TPM2_SU_CLEAR,
	TPM2_SU_STATE,
};

#define TPM2_SPLIT2CHARS(b16) ((b16) >> 8), ((b16) & 0xFF)
#define TPM2_CMD_RSP_BUF_MAX 256
#define TPM2_CMD_LEN_STARTUP 12
#define TPM2_CMD_LEN_SELFTEST 11
#define TPM2_CMD_LEN_CNT 2
#define TPM2_CMD_LEN_ORD 6
#define TPM2_CMD_TIMEOUT_ORD 120000 /* 2 minutes in ms */

#define TPM2_CMD_PREFIX_STARTUP \
	TPM2_SPLIT2CHARS(TPM2_ST_NO_SESSIONS), \
	0, 0, 0, TPM2_CMD_LEN_STARTUP, \
	0, 0, TPM2_SPLIT2CHARS(TPM2_CMD_STARTUP)

#define TPM2_CMD_PREFIX_SELF_TEST \
	TPM2_SPLIT2CHARS(TPM2_ST_NO_SESSIONS), \
	0, 0, 0, TPM2_CMD_LEN_SELFTEST, \
	0, 0, TPM2_SPLIT2CHARS(TPM2_CMD_SELF_TEST)

enum tpm2_result tpm2_startup(struct tpm2_chip *chip,
			      enum tpm2_startup_state state);
enum tpm2_result tpm2_selftest(struct tpm2_chip *chip, bool yes);

#endif	/* __TPM2_CMDS_H__ */

