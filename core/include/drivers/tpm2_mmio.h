/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 *
 */

#ifndef __TPM2_MMIO_H__
#define __TPM2_MMIO_H__

#include <assert.h>
#include <stdbool.h>
#include <tpm2.h>
#include <types_ext.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>

struct tpm2_mmio_data {
	struct io_pa_va base;
	struct tpm2_chip chip;
	uint32_t pcr_count;
	uint32_t pcr_select_min;
};

enum tpm2_result tpm2_mmio_init(struct tpm2_mmio_data *md, paddr_t pbase);

#endif	/* __TPM2_MMIO_H__ */

