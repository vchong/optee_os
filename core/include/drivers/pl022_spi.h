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

#ifndef __PL022_SPI_H__
#define __PL022_SPI_H__

#include <spi.h>
#include <types_ext.h>

#define PL022_REG_SIZE	0x10000

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
	PL022_SPI_MODE0,
	PL022_SPI_MODE1 = 0x80,
	PL022_SPI_MODE2 = 0x40,
	PL022_SPI_MODE3 = 0xC0
};

struct pl022_cfg {
	vaddr_t		base;
	vaddr_t		cs_gpio_base; /* gpio register base address for chip select */
	uint32_t	clk_hz;
	uint32_t	speed_hz;
	uint16_t	cs_gpio_pin; /* gpio pin number for chip select */
	uint8_t		mode;
	uint8_t		data_size_nbits;
};

void pl022_set_register (vaddr_t reg, uint32_t shifted_val, uint32_t mask);
void pl022_print_peri_id (void);
void pl022_print_cell_id (void);
void pl022_sanity_check (void);
void pl022_configure (void);
void pl022_init (const struct pl022_cfg *cfg_ptr);

#endif	/* __PL022_SPI_H__ */

