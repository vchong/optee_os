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

struct pl022_spi_cfg {
	vaddr_t		base;
	vaddr_t		cs_gpio_base; /* gpio register base address for chip select */
	uint32_t	spi_clk; /* hz */
	uint32_t	speed; /* hz */
	uint16_t	cs_gpio_pin; /* gpio pin number for chip select */
	uint8_t		mode;
	uint8_t		data_size;
};

void pl022_set_register (vaddr_t reg, uint32_t shifted_val, uint32_t mask);
void pl022_print_peri_id (void);
void pl022_print_cell_id (void);
void pl022_sanity_check (void);
void pl022_configure (void);
void pl022_init (const struct pl022_spi_cfg *cfg_ptr);

#endif	/* __PL022_SPI_H__ */

