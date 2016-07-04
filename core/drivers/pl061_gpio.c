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
 */

#include <assert.h>
#include <drivers/common.h>
#include <drivers/pl061_gpio.h>
#include <io.h>
#include <trace.h>
#include <util.h>

#ifndef PLAT_PL061_MAX_GPIOS
# define PLAT_PL061_MAX_GPIOS	32
#endif	/* PLAT_PL061_MAX_GPIOS */

#define MAX_GPIO_DEVICES	((PLAT_PL061_MAX_GPIOS + \
	(GPIOS_PER_PL061 - 1)) / GPIOS_PER_PL061)

#define GPIOS_PER_PL061		8

/* gpio register offsets */
#define GPIODIR		0x400
#define GPIOIS		0x404
#define GPIOIBE		0x408
#define GPIOIEV		0x40C
#define GPIOIE		0x410
#define GPIORIS		0x414
#define GPIOMIS		0x418
#define GPIOIC		0x41C
#define GPIOAFSEL	0x420

/* gpio register masks */
#define GPIOIE_ENABLED		SHIFT_U32(1, 0)
#define GPIOIE_MASKED		SHIFT_U32(0, 0)
#define GPIOAFSEL_HW		SHIFT_U32(1, 0)
#define GPIOAFSEL_SW		SHIFT_U32(0, 0)
#define GPIODIR_OUT			SHIFT_U32(1, 0)
#define GPIODIR_IN			SHIFT_U32(0, 0)

static enum gpio_dir pl061_get_direction(unsigned int gpio_pin);
static void pl061_set_direction(unsigned int gpio_pin, enum gpio_dir direction);
static enum gpio_level pl061_get_value(unsigned int gpio_pin);
static void pl061_set_value(unsigned int gpio_pin, enum gpio_level value);

static vaddr_t pl061_reg_base[MAX_GPIO_DEVICES];

static const struct gpio_ops pl061_gpio_ops = {
	.get_direction	= pl061_get_direction,
	.set_direction	= pl061_set_direction,
	.get_value	= pl061_get_value,
	.set_value	= pl061_set_value,
};

static enum gpio_dir pl061_get_direction(unsigned int gpio_pin)
{
	vaddr_t base_addr;
	uint8_t data;
	unsigned int offset;

	assert(gpio_pin < PLAT_PL061_MAX_GPIOS);

	base_addr = pl061_reg_base[gpio_pin / GPIOS_PER_PL061];
	offset = gpio_pin % GPIOS_PER_PL061;

	DMSG("base_addr: 0x%" PRIxVA "\n", base_addr);
	DMSG("offset: %u\n", offset);

	DMSG("addr: 0x%" PRIxVA "\n", base_addr + GPIODIR);

	data = read8(base_addr + GPIODIR);
	if (data & BIT(offset))
	{
		DMSG("dir: GPIO_DIR_OUT: %u\n", GPIO_DIR_OUT);
		return GPIO_DIR_OUT;
	}
	DMSG("dir: GPIO_DIR_IN: %u\n", GPIO_DIR_IN);
	return GPIO_DIR_IN;
}

static void pl061_set_direction(unsigned int gpio_pin, enum gpio_dir direction)
{
	vaddr_t base_addr;
	uint8_t data;
	unsigned int offset;

	assert(gpio_pin < PLAT_PL061_MAX_GPIOS);

	base_addr = pl061_reg_base[gpio_pin / GPIOS_PER_PL061];
	offset = gpio_pin % GPIOS_PER_PL061;

	DMSG("base_addr: 0x%" PRIxVA "\n", base_addr);
	DMSG("offset: %u\n", offset);

	DMSG("addr: 0x%" PRIxVA "\n", base_addr + GPIODIR);
	DMSG("before: 0x%x\n", read32(base_addr + GPIODIR));

	if (direction == GPIO_DIR_OUT) {
		DMSG("dir: GPIO_DIR_OUT: %u\n", direction);
		data = read8(base_addr + GPIODIR) | BIT(offset);
		write8(data, base_addr + GPIODIR);
	} else {
		DMSG("dir: GPIO_DIR_IN: %u\n", direction);
		data = read8(base_addr + GPIODIR) & ~BIT(offset);
		write8(data, base_addr + GPIODIR);
	}

	DMSG("after: 0x%x\n", read32(base_addr + GPIODIR));
}

/*
 * The offset of GPIODATA register is 0.
 * The values read from GPIODATA are determined for each bit, by the mask bit
 * derived from the address used to access the data register, PADDR[9:2].
 * Bits that are 1 in the address mask cause the corresponding bits in GPIODATA
 * to be read, and bits that are 0 in the address mask cause the corresponding
 * bits in GPIODATA to be read as 0, regardless of their value.
 */
static enum gpio_level pl061_get_value(unsigned int gpio_pin)
{
	vaddr_t base_addr;
	unsigned int offset;

	assert(gpio_pin < PLAT_PL061_MAX_GPIOS);

	base_addr = pl061_reg_base[gpio_pin / GPIOS_PER_PL061];
	offset = gpio_pin % GPIOS_PER_PL061;

	DMSG("base_addr: 0x%" PRIxVA "\n", base_addr);
	DMSG("offset: %u\n", offset);
	DMSG("base_addr + BIT(offset + 2): 0x%" PRIxVA "\n", base_addr + BIT(offset + 2));

	if (read8(base_addr + BIT(offset + 2)))
	{
		DMSG("value: GPIO_LEVEL_HIGH: %u\n", GPIO_LEVEL_HIGH);
		return GPIO_LEVEL_HIGH;
	}
	DMSG("value: GPIO_LEVEL_LOW: %u\n", GPIO_LEVEL_LOW);
	return GPIO_LEVEL_LOW;
}

/*
 * In order to write GPIODATA, the corresponding bits in the mask, resulting
 * from the address bus, PADDR[9:2], must be HIGH. Otherwise the bit values
 * remain unchanged by the write.
 */
static void pl061_set_value(unsigned int gpio_pin, enum gpio_level value)
{
	vaddr_t base_addr;
	unsigned int offset;

	assert(gpio_pin < PLAT_PL061_MAX_GPIOS);

	base_addr = pl061_reg_base[gpio_pin / GPIOS_PER_PL061];
	offset = gpio_pin % GPIOS_PER_PL061;

	DMSG("base_addr: 0x%" PRIxVA "\n", base_addr);
	DMSG("offset: %u, BIT(offset) = 0x%x\n", offset, BIT(offset));
	DMSG("base_addr + BIT(offset + 2): 0x%" PRIxVA "\n", base_addr + BIT(offset + 2));
	DMSG("value: %u\n", value);

	DMSG("before: 0x%x\n", read32(BIT(offset + 2));

	if (value == GPIO_LEVEL_HIGH)
	{
		DMSG("value: GPIO_LEVEL_HIGH: %u\n", value);
		write32(BIT(offset), base_addr + BIT(offset + 2));
	}
	else
	{
		DMSG("value: GPIO_LEVEL_LOW: %u\n", value);
		write32(0, base_addr + BIT(offset + 2));
	}

	DMSG("before: 0x%x\n", read32(BIT(offset + 2));
}

/*
 * Register the PL061 GPIO controller with a base address and the offset
 * of start pin in this GPIO controller.
 * This function is called after pl061_gpio_init().
 */
void pl061_gpio_register(vaddr_t base_addr, unsigned int gpio_dev)
{
	assert(gpio_dev < MAX_GPIO_DEVICES);

	pl061_reg_base[gpio_dev] = base_addr;
}

/*
 * Initialize PL061 GPIO controller with the total GPIO numbers in SoC.
 */
void pl061_gpio_init(void)
{
	DMSG("PLAT_PL061_MAX_GPIOS: %d\n", PLAT_PL061_MAX_GPIOS);
	DMSG("MAX_GPIO_DEVICES: %d\n", MAX_GPIO_DEVICES);

	COMPILE_TIME_ASSERT(PLAT_PL061_MAX_GPIOS > 0);

	gpio_init(&pl061_gpio_ops);
}

void pl061_set_interrupt(unsigned int gpio_pin, enum pl061_interrupt ena_dis)
{
	vaddr_t base_addr;
	unsigned int offset;

	assert(gpio_pin < PLAT_PL061_MAX_GPIOS);

	base_addr = pl061_reg_base[gpio_pin / GPIOS_PER_PL061];
	offset = gpio_pin % GPIOS_PER_PL061;

	DMSG("base_addr: 0x%" PRIxVA "\n", base_addr);
	DMSG("offset: %u\n", offset);

	set_register(base_addr + GPIOIE, SHIFT_U32(ena_dis, offset), BIT(offset));
}

void pl061_set_mode_control(unsigned int gpio_pin, enum pl061_mode_control hw_sw)
{
	vaddr_t base_addr;
	unsigned int offset;

	assert(gpio_pin < PLAT_PL061_MAX_GPIOS);

	base_addr = pl061_reg_base[gpio_pin / GPIOS_PER_PL061];
	offset = gpio_pin % GPIOS_PER_PL061;

	DMSG("base_addr: 0x%" PRIxVA "\n", base_addr);
	DMSG("offset: %u\n", offset);

	set_register(base_addr + GPIOAFSEL, SHIFT_U32(hw_sw, offset), BIT(offset));
}
