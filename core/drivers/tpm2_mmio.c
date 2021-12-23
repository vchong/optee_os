/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 *
 */

#include <assert.h>
#include <drivers/tpm2_mmio.h>
#include <initcall.h>
#include <io.h>
#include <keep.h>
#include <kernel/delay.h>
#include <kernel/panic.h>
#include <kernel/tee_time.h>
#include <platform_config.h>
#include <trace.h>
#include <util.h>

static vaddr_t *chip_to_base(struct tpm2_chip *chip)
{
	struct tpm2_mmio_data *md =
		container_of(chip, struct tpm2_mmio_data, chip);

	return io_pa_or_va(&md->base, TPM2_REG_SIZE);
}

static enum tpm2_result tpm2_mmio_rx32(struct tpm2_chip *chip, uint32_t adr,
				       uint32_t *buf)
{
	vaddr_t base = chip_to_base(chip);

	*buf = io_read32(base + adr);

	return TPM2_OK;
}

static enum tpm2_result tpm2_mmio_tx32(struct tpm2_chip *chip, uint32_t adr,
				       uint32_t val)
{
	vaddr_t base = chip_to_base(chip);

	io_write32(val, base + adr);

	return TPM2_OK;
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

static struct tpm2_ops tpm2_mmio_ops = {
	.rx32 = tpm2_mmio_rx32,
	.tx32 = tpm2_mmio_tx32,
	.rx8 = tpm2_mmio_rx8,
	.tx8 = tpm2_mmio_tx8,
};
DECLARE_KEEP_PAGER(tpm2_mmio_ops);

enum tpm2_result tpm2_mmio_init(struct tpm2_mmio_data *md, paddr_t pbase)
{
	enum tpm2_result ret = TPM2_OK;
	vaddr_t base;

	md->base.pa = pbase;
	md->chip.ops = &tpm2_mmio_ops;

	base = io_pa_or_va(&md->base, TPM2_REG_SIZE);

	return tpm2_start(md->chip);
}

