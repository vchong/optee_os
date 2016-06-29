/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <assert.h>
#include <drivers/common.h>
#include <drivers/pl022_spi.h>
#include <initcall.h>
#include <io.h>
#include <trace.h>
#include <util.h>

/* spi register offsets */
#define SSPCR0		0x000
#define SSPCR1		0x004
#define SSPDR		0x008
#define SSPSR		0x00C
#define SSPCPSR		0x010
#define SSPIMSC		0x014
#define SSPRIS		0x018
#define SSPMIS		0x01C
#define SSPICR		0x020
#define SSPDMACR	0x024

/* hikey extensions */
#define SSPTXFIFOCR	0x028
#define SSPRXFIFOCR	0x02C
#define SSPB2BTRANS	0x030

/* test registers */
#define SSPTCR	0x080
#define SSPITIP	0x084
#define SSPITOP	0x088
#define SSPTDR	0x08C

#define SSPPeriphID0	0xFE0
#define SSPPeriphID1	0xFE4
#define SSPPeriphID2	0xFE8
#define SSPPeriphID3	0xFEC

#define SSPPCellID0	0xFF0
#define SSPPCellID1	0xFF4
#define SSPPCellID2	0xFF8
#define SSPPCellID3	0xFFC

/* spi register masks */
#define SSPCR0_SCR			SHIFT_U64(0xFF, 8)
#define SSPCR0_SPH			SHIFT_U64(1, 7)
#define SSPCR0_SPH1			SHIFT_U64(1, 7)
#define SSPCR0_SPH0			SHIFT_U64(0, 7)
#define SSPCR0_SPO			SHIFT_U64(1, 6)
#define SSPCR0_SPO1			SHIFT_U64(1, 6)
#define SSPCR0_SPO0			SHIFT_U64(0, 6)
#define SSPCR0_FRF			SHIFT_U64(3, 4)
#define SSPCR0_FRF_SPI		SHIFT_U64(0, 4)
#define SSPCR0_DSS			SHIFT_U64(0xFF, 0)
#define SSPCR0_DSS_16BIT	SHIFT_U64(0xF, 0)
#define SSPCR0_DSS_8BIT		SHIFT_U64(7, 0)

#define SSPCR1_SOD			SHIFT_U64(1, 3)
#define SSPCR1_SOD_ENABLE	SHIFT_U64(1, 3)
#define SSPCR1_SOD_DISABLE	SHIFT_U64(0, 3)
#define SSPCR1_MS			SHIFT_U64(1, 2)
#define SSPCR1_MS_SLAVE		SHIFT_U64(1, 2)
#define SSPCR1_MS_MASTER	SHIFT_U64(0, 2)
#define SSPCR1_SSE			SHIFT_U64(1, 1)
#define SSPCR1_SSE_ENABLE	SHIFT_U64(1, 1)
#define SSPCR1_SSE_DISABLE	SHIFT_U64(0, 1)
#define SSPCR1_LBM			SHIFT_U64(1, 0)
#define SSPCR1_LBM_YES		SHIFT_U64(1, 0)
#define SSPCR1_LBM_NO		SHIFT_U64(0, 0)

#define SSPDR_DATA	SHIFT_U64(0xFFFF, 0)

#define SSPSR_BSY	SHIFT_U64(1, 4)
#define SSPSR_RNF	SHIFT_U64(1, 3)
#define SSPSR_RNE	SHIFT_U64(1, 2)
#define SSPSR_TNF	SHIFT_U64(1, 1)
#define SSPSR_TFE	SHIFT_U64(1, 0)

#define SSPCPSR_CPSDVR	SHIFT_U64(0xFF, 0)

#define SSPIMSC_TXIM	SHIFT_U64(1, 3)
#define SSPIMSC_RXIM	SHIFT_U64(1, 2)
#define SSPIMSC_RTIM	SHIFT_U64(1, 1)
#define SSPIMSC_RORIM	SHIFT_U64(1, 0)

#define SSPRIS_TXRIS	SHIFT_U64(1, 3)
#define SSPRIS_RXRIS	SHIFT_U64(1, 2)
#define SSPRIS_RTRIS	SHIFT_U64(1, 1)
#define SSPRIS_RORRIS	SHIFT_U64(1, 0)

#define SSPMIS_TXMIS	SHIFT_U64(1, 3)
#define SSPMIS_RXMIS	SHIFT_U64(1, 2)
#define SSPMIS_RTMIS	SHIFT_U64(1, 1)
#define SSPMIS_RORMIS	SHIFT_U64(1, 0)

#define SSPICR_RTIC		SHIFT_U64(1, 1)
#define SSPICR_RORIC	SHIFT_U64(1, 0)

#define SSPDMACR_TXDMAE	SHIFT_U64(1, 1)
#define SSPDMACR_RXDMAE	SHIFT_U64(1, 0)

#define SSPPeriphID0_PartNumber0	SHIFT_U64(0xFF, 0) /* 0x22 */
#define SSPPeriphID1_Designer0		SHIFT_U64(0xF, 4) /* 0x1 */
#define SSPPeriphID1_PartNumber1	SHIFT_U64(0xF, 0) /* 0x0 */
#define SSPPeriphID2_Revision		SHIFT_U64(0xF, 4)
#define SSPPeriphID2_Designer1		SHIFT_U64(0xF, 0) /* 0x4 */
#define SSPPeriphID3_Configuration	SHIFT_U64(0xFF, 0) /* 0x00 */

#define SSPPCellID_0	SHIFT_U64(0xFF, 0) /* 0x0D */
#define SSPPCellID_1	SHIFT_U64(0xFF, 0) /* 0xF0 */
#define SSPPPCellID_2	SHIFT_U64(0xFF, 0) /* 0x05 */
#define SSPPPCellID_3	SHIFT_U64(0xFF, 0) /* 0xB1 */

#define MASK_32 0xFFFFFFFF
#define MASK_28 0xFFFFFFF
#define MASK_24 0xFFFFFF
#define MASK_20 0xFFFFF
#define MASK_16 0xFFFF
#define MASK_12 0xFFF
#define MASK_8 0xFF
#define MASK_4 0xF
/* spi register masks */

#define SSP_CPSDVR_MAX	254
#define SSP_CPSDVR_MIN	2
#define SSP_SCR_MAX		255
#define SSP_SCR_MIN		0

enum pl022_data_size {
	PL022_DATA_SIZE4 = 0x3,
	PL022_DATA_SIZE5,
	PL022_DATA_SIZE6,
	PL022_DATA_SIZE7,
	PL022_DATA_SIZE8,
	PL022_DATA_SIZE9,
	PL022_DATA_SIZE10,
	PL022_DATA_SIZE11,
	PL022_DATA_SIZE12,
	PL022_DATA_SIZE13,
	PL022_DATA_SIZE14,
	PL022_DATA_SIZE15,
	PL022_DATA_SIZE16
};

enum pl022_spi_mode {
	PL022_SPI_MODE0 = SSPCR0_SPO0 | SSPCR0_SPH0, /* 0x00 */
	PL022_SPI_MODE1 = SSPCR0_SPO0 | SSPCR0_SPH1, /* 0x80 */
	PL022_SPI_MODE2 = SSPCR0_SPO1 | SSPCR0_SPH0, /* 0x40 */
	PL022_SPI_MODE3 = SSPCR0_SPO1 | SSPCR0_SPH1  /* 0xC0 */
};

static const struct pl022_cfg *cfg;

static const struct spi_ops pl022_ops = {
	.txrx8 = pl022_txrx8,
	.txrx16 = pl022_txrx16,
	.tx8 = pl022_tx8,
	.tx16 = pl022_tx16,
	.rx8 = pl022_rx8,
	.rx16 = pl022_rx16,
};

static void pl022_print_peri_id(void)
{
	DMSG("Expected: 0x 22 10 #4 0");
	DMSG("Read: 0x %x %x %x %x\n", read32(cfg->base + SSPPeriphID0), read32(cfg->base + SSPPeriphID1), read32(cfg->base + SSPPeriphID2), read32(cfg->base + SSPPeriphID3));
}

static void pl022_print_cell_id(void)
{
	DMSG("Expected: 0x 0D F0 05 B1");
	DMSG("Read: 0x %x %x %x %x\n", read32(cfg->base + SSPPCellID0), read32(cfg->base + SSPPCellID1), read32(cfg->base + SSPPCellID2), read32(cfg->base + SSPPCellID3));
}

static void pl022_sanity_check(void)
{
	DMSG("SSPB2BTRANS: Expected: 0x2. Read: 0x%x\n", read32(cfg->base + SSPB2BTRANS));
	pl022_print_peri_id();
	pl022_print_cell_id();
}

static inline uint32_t pl022_calc_freq(uint8_t cpsdvr, uint8_t scr)
{
	return cfg->clk_hz / (cpsdvr * (1 + scr));
}

static void pl022_calc_clk_divisors(uint8_t *cpsdvr, uint8_t *scr)
{
	uint32_t freq;
	uint8_t tmp_cpsdvr, tmp_scr;

	for (tmp_scr=SSP_SCR_MIN; tmp_scr<SSP_SCR_MAX; tmp_scr++)
	{
		for (tmp_cpsdvr=SSP_CPSDVR_MIN; tmp_cpsdvr<SSP_CPSDVR_MAX; tmp_cpsdvr++)
		{
			freq = pl022_calc_freq(tmp_cpsdvr,tmp_scr);
			if (freq <= cfg->speed_hz)
			{
				goto done;
			}
		}
	}

done:
	*cpsdvr = tmp_cpsdvr;
	*scr = tmp_scr;
	DMSG("cpsdvr: %u (0x%x), scr: %u (0x%x)\n", *cpsdvr, *cpsdvr, *scr, *scr);
	DMSG("speed: requested: %u, closest = %u\n", cfg->speed_hz, freq);
}

void pl022_configure(void)
{
	uint32_t mode, data_size;
	uint8_t cpsdvr, scr;

	assert(cfg);
	pl022_sanity_check();
	pl022_calc_clk_divisors(&cpsdvr, &scr);

	/* configure ssp based on platform settings */
	switch (cfg->mode)
	{
		case SPI_MODE0:
			mode = PL022_SPI_MODE0;
			break;
		case SPI_MODE1:
			mode = PL022_SPI_MODE1;
			break;
		case SPI_MODE2:
			mode = PL022_SPI_MODE2;
			break;
		case SPI_MODE3:
			mode = PL022_SPI_MODE3;
			break;
		default:
			EMSG("Invalid spi mode: %u\n", cfg->mode);
			return;
	}
	
	switch (cfg->data_size_bits)
	{
		case 8:
			data_size = PL022_DATA_SIZE8;
			break;
		case 16:
			data_size = PL022_DATA_SIZE16;
			break;
		default:
			EMSG("Unsupported data size: %u bits\n", cfg->data_size_bits);
			return;
	}

	set_register(cfg->base + SSPCR0, (scr << 8) | mode | SSPCR0_FRF_SPI | data_size, MASK_16);

	/* disable loopback */
	set_register(cfg->base + SSPCR1, SSPCR1_SOD_DISABLE | SSPCR1_MS_MASTER | SSPCR1_SSE_DISABLE | SSPCR1_LBM_NO, MASK_4);

	set_register(cfg->base + SSPCPSR, cpsdvr, SSPCPSR_CPSDVR);

	/* disable interrupts */
	set_register(cfg->base + SSPIMSC, 0, MASK_4);

	DMSG("set cs gpio dir to out\n");
	gpio_set_direction(cfg->cs_gpio_pin, GPIO_DIR_OUT);

	DMSG("pull cs high\n");
	gpio_set_value(cfg->cs_gpio_pin, GPIO_LEVEL_HIGH);
}

void pl022_init(const struct pl022_cfg *cfg_ptr)
{
	assert(cfg_ptr);
	cfg = cfg_ptr;
	spi_init(&pl022_ops);
}

static void pl022_txrx8(uint8_t *wdat, uint8_t *rdat, uint32_t num_txpkts, uint32_t *num_rxpkts)
{

}

static void pl022_txrx16(uint16_t *wdat, uint16_t *rdat, uint32_t num_txpkts, uint32_t *num_rxpkts)
{
}

static void pl022_tx8(uint8_t *wdat, uint32_t num_txpkts)
{

}

static void pl022_tx16(uint16_t *wdat, uint32_t num_txpkts)
{

}

static void pl022_rx8(uint8_t *rdat, uint32_t *num_rxpkts)
{

}

static void pl022_rx16(uint16_t *rdat, uint32_t *num_rxpkts)
{

}

