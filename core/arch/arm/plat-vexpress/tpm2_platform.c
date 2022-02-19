// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2022, Linaro Limited
 */

#include <tee_api_types.h>
#include <tpm2_cmds.h>
#include <tpm2_platform.h>
#include <trace.h>

TEE_Result test_tpm2(struct tpm2_mmio_data *md)
{
	DMSG("Call tpm2_startup() and other cmds here");
	tpm2_startup(&md->chip, TPM2_SU_CLEAR);
	tpm2_startup(&md->chip, TPM2_SU_STATE);
	tpm2_selftest(&md->chip, true);
	tpm2_selftest(&md->chip, false);
	return TEE_SUCCESS;
}

