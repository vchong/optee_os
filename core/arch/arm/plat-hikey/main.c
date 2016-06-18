/*
 * Copyright (c) 2015, Linaro Limited
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

#include <console.h>
#include <drivers/pl011.h>
#include <drivers/pl022_spi.h>
#include <drivers/pl061_gpio.h>
#include <hi6220.h>
#include <initcall.h>
#include <kernel/generic_boot.h>
#include <kernel/panic.h>
#include <kernel/pm_stubs.h>
#include <mm/tee_pager.h>
#include <mm/core_memprot.h>
#include <platform_config.h>
#include <stdint.h>
#include <tee/entry_std.h>
#include <tee/entry_fast.h>

static void main_fiq(void);

static const struct thread_handlers handlers = {
	.std_smc = tee_entry_std,
	.fast_smc = tee_entry_fast,
	.fiq = main_fiq,
	.cpu_on = cpu_on_handler,
	.cpu_off = pm_do_nothing,
	.cpu_suspend = pm_do_nothing,
	.cpu_resume = pm_do_nothing,
	.system_off = pm_do_nothing,
	.system_reset = pm_do_nothing,
};

register_phys_mem(MEM_AREA_IO_NSEC, CONSOLE_UART_BASE, PL011_REG_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, PERI_BASE, PERI_BASE_REG_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, SPI_BASE, PL022_REG_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, GPIO6_BASE, PL061_REG_SIZE);

//speed
//10000; //10khz
//50000; //50khz
//500000; //500khz

static const struct pl022_spi_cfg hikey_spi_cfg = {
	.base = SPI_BASE,
	.cs_gpio_base = GPIO6_BASE,
	.spi_clk = HI6220_SPI_CLK,
	.speed = 500000,
	.cs_gpio_pin = 2,
	.mode = 0,
	.data_size = 8,
};

const struct thread_handlers *generic_boot_get_handlers(void)
{
	return &handlers;
}

static void main_fiq(void)
{
	panic();
}

static vaddr_t console_base(void)
{
	static void *va;

	if (cpu_mmu_enabled()) {
		if (!va)
			va = phys_to_virt(CONSOLE_UART_BASE, MEM_AREA_IO_NSEC);
		return (vaddr_t)va;
	}
	return CONSOLE_UART_BASE;
}

void console_init(void)
{
	pl011_init(console_base(), CONSOLE_UART_CLK_IN_HZ, CONSOLE_BAUDRATE);
}

void console_putc(int ch)
{
	vaddr_t base = console_base();

	pl011_putc(ch, base);
	if (ch == '\n')
		pl011_putc('\r', base);
}

void console_flush(void)
{
	pl011_flush(console_base());
}

void hikey_spi_enable (void)
{
	uint32_t shifted_val, read_val;

	DMSG ("peri_base = 0x%x\n", PERI_BASE);

	/* take SPI0 out of reset */
	/* no need to read SC_PERIPH_RSTDIS3 first as all the bits are processed and cleared after writing */
	shifted_val = PERI_RST3_SSP;
	write32 (shifted_val, PERI_SC_PERIPH_RSTDIS3);

	/* wait until the requested device is out of reset, and ready to be used */
	do {
	  read_val = read32 (PERI_SC_PERIPH_RSTSTAT3);
	} while (read_val & shifted_val);

	DMSG ("read_val = 0x%x\n", read_val);
	DMSG ("SC_PERIPH_RSTSTAT3 = 0x%x\n", read32 (PERI_SC_PERIPH_RSTSTAT3));
	DMSG ("shifted_val = 0x%x\n", shifted_val);
	DMSG ("SC_PERIPH_RSTDIS3 = 0x%x\n", read32 (PERI_SC_PERIPH_RSTDIS3));
}

void peri_init (void)
{
	pl061_gpio_init();
	pl061_gpio_register(GPIO0_BASE, 0);
	pl061_gpio_register(GPIO1_BASE, 1);
	pl061_gpio_register(GPIO2_BASE, 2);
	pl061_gpio_register(GPIO3_BASE, 3);
	pl061_gpio_register(GPIO4_BASE, 4);
	pl061_gpio_register(GPIO5_BASE, 5);
	pl061_gpio_register(GPIO6_BASE, 6);
	pl061_gpio_register(GPIO7_BASE, 7);
	pl061_gpio_register(GPIO8_BASE, 8);
	pl061_gpio_register(GPIO9_BASE, 9);
	pl061_gpio_register(GPIO10_BASE, 10);
	pl061_gpio_register(GPIO11_BASE, 11);
	pl061_gpio_register(GPIO12_BASE, 12);
	pl061_gpio_register(GPIO13_BASE, 13);
	pl061_gpio_register(GPIO14_BASE, 14);
	pl061_gpio_register(GPIO15_BASE, 15);
	pl061_gpio_register(GPIO16_BASE, 16);
	pl061_gpio_register(GPIO17_BASE, 17);
	pl061_gpio_register(GPIO18_BASE, 18);
	pl061_gpio_register(GPIO19_BASE, 19);

	hikey_spi_enable();
	pl022_init (&hikey_spi_cfg);
	pl022_configure();
}

static TEE_Result spi_test (void)
{
	peri_init();
	return TEE_SUCCESS;
}

service_init(spi_test);
