/*
 * Copyright (c) 2016, Linaro Ltd and Contributors. All rights reserved.
 * Copyright (c) 2016, Hisilicon Ltd and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
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
 */

#ifndef __HI6220_H__
#define __HI6220_H__

#include <hi6220_regs_ao.h>
#include <hi6220_regs_peri.h>
#include <hi6220_regs_pmctrl.h>

#define	SPI_BASE				0xF7106000
#define SPI_CLK_HZ				150000000 /* 150mhz */

#define PMX0_BASE				0xF7010000
#define PMX1_BASE				0xF7010800
#define PMX2_BASE				0xF8001800

#define PMX0_REG_SIZE			0x400
#define PMX1_REG_SIZE			0x400
#define PMX2_REG_SIZE			0x400

#define PMX0_IOMG104			0x1a0
#define PMX0_IOMG105			0x1a4
#define PMX0_IOMG106			0x1a8
#define PMX0_IOMG107			0x1ac
#define PMX1_IOCG104			0x1b0
#define PMX1_IOCG105			0x1b4
#define PMX1_IOCG106			0x1b8
#define PMX1_IOCG107			0x1bc
#define PMX2_IOCG0				0
#define PMX2_IOCG1				4
#define PMX2_IOCG2				8
#define PMX2_IOCG28				0x70
#define PMX2_IOCG29				0x74

#define PMUSSI_BASE				0xF8000000

#define SP804_TIMER0_BASE		0xF8008000

#define GPIO0_BASE				0xF8011000
#define GPIO1_BASE				0xF8012000
#define GPIO2_BASE				0xF8013000
#define GPIO3_BASE				0xF8014000
#define GPIO4_BASE				0xF7020000
#define GPIO5_BASE				0xF7021000
#define GPIO6_BASE				0xF7022000
#define GPIO7_BASE				0xF7023000
#define GPIO8_BASE				0xF7024000
#define GPIO9_BASE				0xF7025000
#define GPIO10_BASE				0xF7026000
#define GPIO11_BASE				0xF7027000
#define GPIO12_BASE				0xF7028000
#define GPIO13_BASE				0xF7029000
#define GPIO14_BASE				0xF702A000
#define GPIO15_BASE				0xF702B000
#define GPIO16_BASE				0xF702C000
#define GPIO17_BASE				0xF702D000
#define GPIO18_BASE				0xF702E000
#define GPIO19_BASE				0xF702F000

#define GPIO6_2					50

#endif	/* __HI6220_H__ */
