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

#define PMX0_REG_SIZE			0x27c
#define PMX1_REG_SIZE			0x28c
#define PMX2_REG_SIZE			0x78

#define PMX_IOXG000			0
#define PMX_IOXG001			0x4
#define PMX_IOXG002			0x8
#define PMX_IOXG003			0xc
#define PMX_IOXG004			0x10
#define PMX_IOXG005			0x14
#define PMX_IOXG006			0x18
#define PMX_IOXG007			0x1c
#define PMX_IOXG008			0x20
#define PMX_IOXG009			0x24
#define PMX_IOXG010			0x28
#define PMX_IOXG011			0x2c
#define PMX_IOXG012			0x30
#define PMX_IOXG013			0x34
#define PMX_IOXG014			0x38
#define PMX_IOXG015			0x3c
#define PMX_IOXG016			0x40
#define PMX_IOXG017			0x44
#define PMX_IOXG018			0x48
#define PMX_IOXG019			0x4c
#define PMX_IOXG020			0x50
#define PMX_IOXG021			0x54
#define PMX_IOXG022			0x58
#define PMX_IOXG023			0x5c
#define PMX_IOXG024			0x60
#define PMX_IOXG025			0x64
#define PMX_IOXG026			0x68
#define PMX_IOXG027			0x6c
#define PMX_IOXG028			0x70
#define PMX_IOXG029			0x74
#define PMX_IOXG030			0x78
#define PMX_IOXG031			0x7c
#define PMX_IOXG032			0x80
#define PMX_IOXG033			0x84
#define PMX_IOXG034			0x88
#define PMX_IOXG035			0x8c
#define PMX_IOXG036			0x90
#define PMX_IOXG037			0x94
#define PMX_IOXG038			0x98
#define PMX_IOXG039			0x9c
#define PMX_IOXG040			0xa0
#define PMX_IOXG046			0xb8
#define PMX_IOXG047			0xbc
#define PMX_IOXG048			0xc0
#define PMX_IOXG049			0xc4
#define PMX_IOXG050			0xc8
#define PMX_IOXG051			0xcc
#define PMX_IOXG052			0xd0
#define PMX_IOXG053			0xd4
#define PMX_IOXG054			0xd8
#define PMX_IOXG055			0xdc
#define PMX_IOXG056			0xe0
#define PMX_IOXG057			0xe4
#define PMX_IOXG058			0xe8
#define PMX_IOXG059			0xec
#define PMX_IOXG060			0xf0
#define PMX_IOXG061			0xf4
#define PMX_IOXG062			0xf8
#define PMX_IOXG063			0xfc
#define PMX_IOXG064			0x100
#define PMX_IOXG065			0x104
#define PMX_IOXG066			0x108
#define PMX_IOXG067			0x10c
#define PMX_IOXG068			0x110
#define PMX_IOXG069			0x114
#define PMX_IOXG070			0x118
#define PMX_IOXG071			0x11c
#define PMX_IOXG072			0x120
#define PMX_IOXG073			0x124
#define PMX_IOXG074			0x128
#define PMX_IOXG075			0x12c
#define PMX_IOXG076			0x130
#define PMX_IOXG077			0x134
#define PMX_IOXG078			0x138
#define PMX_IOXG079			0x13c
#define PMX_IOXG080			0x140
#define PMX_IOXG081			0x144
#define PMX_IOXG082			0x148
#define PMX_IOXG096			0x180
#define PMX_IOXG097			0x184
#define PMX_IOXG098			0x188
#define PMX_IOXG099			0x18c
#define PMX_IOXG100			0x190
#define PMX_IOXG101			0x194
#define PMX_IOXG102			0x198
#define PMX_IOXG103			0x19c
#define PMX_IOXG104			0x1a0
#define PMX_IOXG105			0x1a4
#define PMX_IOXG106			0x1a8
#define PMX_IOXG107			0x1ac
#define PMX_IOXG108			0x1b0
#define PMX_IOXG109			0x1b4
#define PMX_IOXG110			0x1b8
#define PMX_IOXG111			0x1bc
#define PMX_IOXG114			0x1c8
#define PMX_IOXG115			0x1cc
#define PMX_IOXG116			0x1d0
#define PMX_IOXG117			0x1d4
#define PMX_IOXG118			0x1d8
#define PMX_IOXG119			0x1dc
#define PMX_IOXG120			0x1e0
#define PMX_IOXG121			0x1e4
#define PMX_IOXG122			0x1e8
#define PMX_IOXG123			0x1ec

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
