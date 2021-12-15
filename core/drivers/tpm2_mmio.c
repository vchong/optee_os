/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 *
 */

#include <assert.h>
#include <delay.h>
#include <initcall.h>
#include <io.h>
#include <keep.h>
#include <kernel/panic.h>
#include <kernel/tee_time.h>
#include <platform_config.h>
#include <tpm2.h>
#include <tpm2_mmio.h>
#include <trace.h>
#include <util.h>

static vaddr_t *chip_to_base(struct tpm2_chip *chip)
{
	struct tpm2_mmio_data *md =
		container_of(chip, struct tpm2_mmio_data, chip);

	return io_pa_or_va(&md->base, MMIO_REG_SIZE);
}

static enum tpm2_result tpm2_mmio_rx8(tpm2_chip *chip, uint32_t adr,
				      uint16_t len, uint8_t *buf)
{
	vaddr_t base = chip_to_base(chip);

	while (len--)
		*buf++ = io_read8(base + adr);

	return TPM2_OK;
}

static enum tpm2_result tpm2_mmio_tx8(tpm2_chip *chip, uint32_t adr,
				      uint16_t len, uint8_t *buf)
{
	vaddr_t base = chip_to_base(chip);

	while (len--)
		io_write8(base + adr, *buf++);

	return TPM2_OK;
}

