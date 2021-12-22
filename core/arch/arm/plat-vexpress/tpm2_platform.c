/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 *
 */

#include <drivers/tpm2_mmio.h>
#include <io.h>
#include <stdint.h>
#include <tpm2_platform.h>
#include <trace.h>
#include <util.h>

void tpm2_init(void)
{
	if (tpm2_mmio_init(&tpm2_data, TPM2_BASE))
		EMSG("Failed to initialize TPM2 MMIO");
}

