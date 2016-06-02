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

#define SSP_MASK_32	0xFFFFFFFF
#define SSP_MASK_28	0xFFFFFFF
#define SSP_MASK_24	0xFFFFFF
#define SSP_MASK_20	0xFFFFF
#define SSP_MASK_16	0xFFFF
#define SSP_MASK_12	0xFFF
#define SSP_MASK_8	0xFF
#define SSP_MASK_4	0xF
/* spi register masks */

#define SSP_CPSDVR_MAX	254
#define SSP_CPSDVR_MIN	2
#define SSP_SCR_MAX		255
#define SSP_SCR_MIN		0

static void pl022_txrx8 (uint8_t *wdat, uint8_t *rdat, uint32_t num_txpkts, uint32_t *num_rxpkts);
static void pl022_txrx16 (uint16_t *wdat, uint16_t *rdat, uint32_t num_txpkts, uint32_t *num_rxpkts);
static void pl022_tx8 (uint8_t *wdat, uint32_t num_txpkts);
static void pl022_tx16 (uint16_t *wdat, uint32_t num_txpkts);
static void pl022_rx8 (uint8_t *rdat, uint32_t *num_rxpkts);
static void pl022_rx16 (uint16_t *rdat, uint32_t *num_rxpkts);

static const struct pl022_spi_cfg *cfg;

static const struct spi_ops pl022_ops = {
	.txrx8 = pl022_txrx8,
	.txrx16 = pl022_txrx16,
	.tx8 = pl022_tx8,
	.tx16 = pl022_tx16,
	.rx8 = pl022_rx8,
	.rx16 = pl022_rx16,
};

static void pl022_txrx8 (uint8_t *wdat, uint8_t *rdat, uint32_t num_txpkts, uint32_t *num_rxpkts)
{

}

static void pl022_txrx16 (uint16_t *wdat, uint16_t *rdat, uint32_t num_txpkts, uint32_t *num_rxpkts)
{
}

static void pl022_tx8 (uint8_t *wdat, uint32_t num_txpkts)
{

}

static void pl022_tx16 (uint16_t *wdat, uint32_t num_txpkts)
{

}

static void pl022_rx8 (uint8_t *rdat, uint32_t *num_rxpkts)
{

}

static void pl022_rx16 (uint16_t *rdat, uint32_t *num_rxpkts)
{

}

void pl022_set_register (vaddr_t reg, uint32_t shifted_val, uint32_t mask)
{
	DMSG ("addr: 0x%x\n", (uint32_t)reg);
	DMSG ("before: 0x%x\n", read32 (reg));
	write32 ((read32 (reg) & ~mask) | shifted_val, reg);
	DMSG ("after: 0x%x\n", read32 (reg));
}

void pl022_print_peri_id (void)
{
	DMSG ("Expected: 0x 22 10 #4 0");
	DMSG ("Read: 0x %x %x %x %x\n", read32 (cfg->base + SSPPeriphID0), read32 (cfg->base + SSPPeriphID1), read32 (cfg->base + SSPPeriphID2), read32 (cfg->base + SSPPeriphID3));
}

void pl022_print_cell_id (void)
{
	DMSG ("Expected: 0x 0D F0 05 B1");
	DMSG ("Read: 0x %x %x %x %x\n", read32 (cfg->base + SSPPCellID0), read32 (cfg->base + SSPPCellID1), read32 (cfg->base + SSPPCellID2), read32 (cfg->base + SSPPCellID3));
}

void pl022_sanity_check (void)
{
	DMSG ("SSPB2BTRANS: Expected: 0x2. Read: 0x%x\n", read32 (cfg->base + SSPB2BTRANS));
	pl022_print_peri_id();
	pl022_print_cell_id();
}

void pl022_configure (void)
{
	pl022_sanity_check ();
}

void pl022_init(const struct pl022_spi_cfg *cfg_ptr)
{
	assert(cfg_ptr != 0);
	cfg = cfg_ptr;

	spi_init (&pl022_ops);
}

