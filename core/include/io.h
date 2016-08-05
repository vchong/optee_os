/*
 * Copyright (c) 2014, Linaro Limited
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
 */
#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <trace.h>
#include <types_ext.h>
#ifdef ARM64
#include <arm64.h>
#else
#include <arm32.h>
#endif

/*
 * IO access macro, please avoid using this macro, since it's going to be
 * deprecated.
 */
#define  IO(addr)  (*((volatile unsigned long *)(addr)))

static inline void write8(uint8_t val, vaddr_t addr)
{
	dsb();
	isb();
	*(volatile uint8_t *)addr = val;
}

static inline void write16(uint16_t val, vaddr_t addr)
{
	dsb();
	isb();
	*(volatile uint16_t *)addr = val;
}

static inline void write32(uint32_t val, vaddr_t addr)
{
	dsb();
	isb();
	*(volatile uint32_t *)addr = val;
}

static inline uint8_t read8(vaddr_t addr)
{
	uint8_t val;

	val = *(volatile uint8_t *)addr;
	dsb();
	isb();
	return val;
}

static inline uint16_t read16(vaddr_t addr)
{
	uint16_t val;

	val = *(volatile uint16_t *)addr;
	dsb();
	isb();
	return val;
}

static inline uint32_t read32(vaddr_t addr)
{
	uint32_t val;

	val = *(volatile uint32_t *)addr;
	dsb();
	isb();
	return val;
}

static inline void io_mask8(vaddr_t addr, uint8_t val, uint8_t mask)
{
	DMSG("addr: 0x%" PRIxVA "\n", addr);
	DMSG("before: 0x%x\n", read8(addr));
	write8((read8(addr) & ~mask) | (val & mask), addr);
	DMSG("after: 0x%x\n", read8(addr));
}

static inline void io_mask16(vaddr_t addr, uint16_t val, uint16_t mask)
{
	DMSG("addr: 0x%" PRIxVA "\n", addr);
	DMSG("before: 0x%x\n", read16(addr));
	write16((read16(addr) & ~mask) | (val & mask), addr);
	DMSG("after: 0x%x\n", read16(addr));
}

static inline void io_mask32(vaddr_t addr, uint32_t val, uint32_t mask)
{
	DMSG("addr: 0x%" PRIxVA "\n", addr);
	DMSG("before: 0x%x\n", read32(addr));
	write32((read32(addr) & ~mask) | (val & mask), addr);
	DMSG("after: 0x%x\n", read32(addr));
}

#endif /*IO_H*/
