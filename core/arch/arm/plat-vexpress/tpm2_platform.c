/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 *
 */

#include <tee_api_types.h>
#include <tpm2_cmds.h>
#include <tpm2_platform.h>
#include <trace.h>

TEE_Result test_tpm2(struct tpm2_mmio_data *md)
{
	DMSG("1");
	tpm2_startup(&md->chip, TPM2_SU_CLEAR);
	tpm2_startup(&md->chip, TPM2_SU_STATE);
	return TEE_SUCCESS;
}

