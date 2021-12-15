/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 *
 */

#ifndef __TPM2_MMIO_H__
#define __TPM2_MMIO_H__

#include <tpm2.h>

/* not sure about this */
#define MMIO_REG_SIZE	0x1000

struct tpm2_mmio_data {
	struct tpm2_chip *chip;	
	uint32_t pcr_count;
	uint32_t pcr_select_min;
	vaddr_t base;
}

#endif	/* __TPM2_MMIO_H__ */

