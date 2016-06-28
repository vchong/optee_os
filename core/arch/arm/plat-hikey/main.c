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
#include <io.h>
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
static vaddr_t peri_base(void);
static vaddr_t spi_base(void);
static vaddr_t gpio6_base(void);
static vaddr_t pmx0_base(void);
static vaddr_t pmx1_base(void);
void platform_spi_enable(void);
void peri_init(void);

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

static struct pl022_cfg platform_pl022_cfg = {
	.clk_hz = SPI_CLK_HZ,
	.speed_hz = 500000,
	.cs_gpio_pin = 2,
	.mode = SPI_MODE0,
	.data_size_nbits = 8,
};

register_phys_mem(MEM_AREA_IO_NSEC, CONSOLE_UART_BASE, PL011_REG_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, PERI_BASE, PERI_BASE_REG_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, SPI_BASE, PL022_REG_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, GPIO6_BASE, PL061_REG_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, PMX0_BASE, PMX0_REG_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, PMX1_BASE, PMX1_REG_SIZE);

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

static vaddr_t peri_base(void)
{
	static void *va;

	if (cpu_mmu_enabled()) {
		if (!va)
			va = phys_to_virt(PERI_BASE, MEM_AREA_IO_NSEC);
		return (vaddr_t)va;
	}
	return PERI_BASE;
}

static vaddr_t spi_base(void)
{
	static void *va;

	if (cpu_mmu_enabled()) {
		if (!va)
			va = phys_to_virt(SPI_BASE, MEM_AREA_IO_NSEC);
		return (vaddr_t)va;
	}
	return SPI_BASE;
}

static vaddr_t gpio6_base(void)
{
	static void *va;

	if (cpu_mmu_enabled()) {
		if (!va)
			va = phys_to_virt(GPIO6_BASE, MEM_AREA_IO_NSEC);
		return (vaddr_t)va;
	}
	return GPIO6_BASE;
}

static vaddr_t pmx0_base(void)
{
	static void *va;

	if (cpu_mmu_enabled()) {
		if (!va)
			va = phys_to_virt(PMX0_BASE, MEM_AREA_IO_NSEC);
		return (vaddr_t)va;
	}
	return PMX0_BASE;
}

static vaddr_t pmx1_base(void)
{
	static void *va;

	if (cpu_mmu_enabled()) {
		if (!va)
			va = phys_to_virt(PMX1_BASE, MEM_AREA_IO_NSEC);
		return (vaddr_t)va;
	}
	return PMX1_BASE;
}

void platform_spi_enable(void)
{
	vaddr_t peri_sc_base = peri_base();
	vaddr_t pmx0_iomg_base = pmx0_base();
	vaddr_t pmx1_iocg_base = pmx1_base();
	uint32_t shifted_val, read_val;

	DMSG("peri_sc_base: 0x%" PRIxVA "\n", peri_sc_base);
	DMSG("pmx0_iomg_base: 0x%" PRIxVA "\n", pmx0_iomg_base);
	DMSG("pmx1_iocg_base: 0x%" PRIxVA "\n", pmx1_iocg_base);

	/* take SPI0 out of reset */
	/* no need to read PERI_SC_PERIPH_RSTDIS3 first as all the bits are processed and cleared after writing */
	shifted_val = PERI_RST3_SSP;
	write32(shifted_val, peri_sc_base + PERI_SC_PERIPH_RSTDIS3);

	DMSG("shifted_val: 0x%x\n", shifted_val);
	DMSG("PERI_SC_PERIPH_RSTDIS3: 0x%x\n", read32(peri_sc_base + PERI_SC_PERIPH_RSTDIS3));

	/* wait until the requested device is out of reset, and ready to be used */
	do {
	  read_val = read32(peri_sc_base + PERI_SC_PERIPH_RSTSTAT3);
	} while (read_val & shifted_val);

	DMSG("read_val: 0x%x\n", read_val);
	DMSG("PERI_SC_PERIPH_RSTSTAT3: 0x%x\n", read32(peri_sc_base + PERI_SC_PERIPH_RSTSTAT3));

	/* enable SPI clock */
	/* no need to read PERI_SC_PERIPH_CLKEN3 first as all the bits are processed and cleared after writing */
	shifted_val = PERI_CLK3_SSP;
	write32(shifted_val, peri_sc_base + PERI_SC_PERIPH_CLKEN3);

	DMSG("shifted_val: 0x%x\n", shifted_val);
	DMSG("PERI_SC_PERIPH_CLKEN3: 0x%x\n", read32(peri_sc_base + PERI_SC_PERIPH_CLKEN3));

	/* wait until the requested device is out of reset, and ready to be used */
	do {
	  read_val = read32(peri_sc_base + PERI_SC_PERIPH_CLKSTAT3);
	} while (read_val & shifted_val);

	DMSG("read_val: 0x%x\n", read_val);
	DMSG("PERI_SC_PERIPH_CLKSTAT3: 0x%x\n", read32(peri_sc_base + PERI_SC_PERIPH_CLKSTAT3));

	/* configure pin mux: 0: gpio, 1: spi*/
	DMSG("configure gpio6_{0,1,3} as spi\n");
	DMSG("configure gpio6_2 as gpio, else hw ip will try to control it as well, causing interference\n");
	write32(0, pmx0_iomg_base + PMX0_IOMG104);
	write32(0, pmx0_iomg_base + PMX0_IOMG105);
	write32(0, pmx0_iomg_base + PMX0_IOMG106);
	write32(0, pmx0_iomg_base + PMX0_IOMG107);

	/*
	write32(1, 0xF70101A0);
	write32(1, 0xF70101A4);
	write32(0, 0xF70101A8);
	write32(1, 0xF70101AC);
	*/

	/* configure pin bias: 0: nopull, 1: pullup, 2: pulldown */
	DMSG("configure gpio6_{0:3} as nopull and 02ma drive\n");
	write32(0, pmx1_iocg_base + PMX1_IOCG104);
	write32(0, pmx1_iocg_base + PMX1_IOCG105);
	write32(0, pmx1_iocg_base + PMX1_IOCG106);
	write32(0, pmx1_iocg_base + PMX1_IOCG107);

	/*
	write32(0, 0xF70109B0);
	write32(0, 0xF70109B4);
	write32(0, 0xF70109B8);
	write32(0, 0xF70109BC);
	*/
}

void peri_init(void)
{
	pl061_gpio_init();
	pl061_gpio_register(gpio6_base(), 6);

	platform_spi_enable();

	platform_pl022_cfg.base = spi_base(),
	platform_pl022_cfg.cs_gpio_base = gpio6_base(),
	pl022_init(&platform_pl022_cfg);
	pl022_configure();
}

void spi_test2(void)
{
	DMSG("Hello!\n");
	peri_init();
}

static TEE_Result spi_test(void)
{
	peri_init();
	return TEE_SUCCESS;
}

service_init(spi_test);
