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
	.cs_gpio_pin = GPIO6_2,
	.mode = SPI_MODE0,
	.data_size_bits = 8,
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

/*
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
*/

static vaddr_t get_va(paddr_t pa)
{
	static void *va;

	if (cpu_mmu_enabled()) {
		if (!va)
			va = phys_to_virt(pa, MEM_AREA_IO_NSEC);
		return (vaddr_t)va;
	}
	return (vaddr_t)pa;
}

static void platform_spi_enable(void)
{
	vaddr_t peri_base = get_va(PERI_BASE);
	vaddr_t pmx0_base = get_va(PMX0_BASE);
	vaddr_t pmx1_base = get_va(PMX1_BASE);
	uint32_t shifted_val, read_val;

	DMSG("peri_base: 0x%" PRIxVA "\n", peri_base);
	DMSG("pmx0_base: 0x%" PRIxVA "\n", pmx0_base);
	DMSG("pmx1_base: 0x%" PRIxVA "\n", pmx1_base);

	/* take SPI0 out of reset */
	/* no need to read PERI_SC_PERIPH_RSTDIS3 first as all the bits are processed and cleared after writing */
	shifted_val = PERI_RST3_SSP;
	write32(shifted_val, peri_base + PERI_SC_PERIPH_RSTDIS3);

	DMSG("shifted_val: 0x%x\n", shifted_val);
	DMSG("PERI_SC_PERIPH_RSTDIS3: 0x%x\n", read32(peri_base + PERI_SC_PERIPH_RSTDIS3));

	/* wait until the requested device is out of reset, and ready to be used */
	do {
	  read_val = read32(peri_base + PERI_SC_PERIPH_RSTSTAT3);
	} while (read_val & shifted_val);

	DMSG("read_val: 0x%x\n", read_val);
	DMSG("PERI_SC_PERIPH_RSTSTAT3: 0x%x\n", read32(peri_base + PERI_SC_PERIPH_RSTSTAT3));

	/* enable SPI clock */
	/* no need to read PERI_SC_PERIPH_CLKEN3 first as all the bits are processed and cleared after writing */
	shifted_val = PERI_CLK3_SSP;
	write32(shifted_val, peri_base + PERI_SC_PERIPH_CLKEN3);

	DMSG("shifted_val: 0x%x\n", shifted_val);
	DMSG("PERI_SC_PERIPH_CLKEN3: 0x%x\n", read32(peri_base + PERI_SC_PERIPH_CLKEN3));
	DMSG("read_val: 0x%x\n", read_val);
	DMSG("PERI_SC_PERIPH_CLKSTAT3: 0x%x\n", read32(peri_base + PERI_SC_PERIPH_CLKSTAT3));

	/* configure pin mux: 0: gpio, 1: spi*/
	DMSG("configure gpio6_{0,1,3} as spi\n");
	DMSG("configure gpio6_2 as gpio, else hw ip will try to control it as well, causing interference\n");
	write32(0, pmx0_base + PMX0_IOMG104); /* 0xF70101A0 */
	write32(0, pmx0_base + PMX0_IOMG105); /* 0xF70101A4 */
	write32(0, pmx0_base + PMX0_IOMG106); /* 0xF70101A8 */
	write32(0, pmx0_base + PMX0_IOMG107); /* 0xF70101AC */

	/* configure pin bias: 0: nopull, 1: pullup, 2: pulldown */
	DMSG("configure gpio6_{0:3} as nopull and 02ma drive\n");
	write32(0, pmx1_base + PMX1_IOCG104); /* 0xF70109B0 */
	write32(0, pmx1_base + PMX1_IOCG105); /* 0xF70109B4 */
	write32(0, pmx1_base + PMX1_IOCG106); /* 0xF70109B8 */
	write32(0, pmx1_base + PMX1_IOCG107); /* 0xF70109BC */
}

static void peri_init(void)
{
	vaddr_t gpio6_base = get_va(GPIO6_BASE);

	pl061_gpio_init();
	pl061_gpio_register(gpio6_base, 6);

	DMSG("mask/disable interrupt for cs\n");
	pl061_set_interrupt(GPIO6_2, PL061_INTERRUPT_DISABLE);

	DMSG("enable software mode control for cs\n");
	pl061_set_mode_control(GPIO6_2, PL061_MC_SW);

	platform_spi_enable();

	platform_pl022_cfg.base = get_va(SPI_BASE);
	platform_pl022_cfg.cs_gpio_base = gpio6_base;
	#if 1
	platform_pl022_cfg.data_size_bits = 8;
	#else
	platform_pl022_cfg.data_size_bits = 16;
	#endif
	pl022_init(&platform_pl022_cfg);

	pl022_configure();
}

void spi_test2(void)
{
	uint8_t __maybe_unused data8[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	uint8_t __maybe_unused data8_long[20] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0xaa, 0xbb, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};
	uint8_t __maybe_unused data8_100[100] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, \
									10, 11, 12, 13, 14, 15, 16, 17, 18, 19, \
									20, 21, 22, 23, 24, 25, 26, 27, 28, 29, \
									30, 31, 32, 33, 34, 35, 36, 37, 38, 39, \
									40, 41, 42, 43, 44, 45, 46, 47, 48, 49, \
									50, 51, 52, 53, 54, 55, 56, 57, 58, 59, \
									60, 61, 62, 63, 64, 65, 66, 67, 68, 69, \
									70, 71, 72, 73, 74, 75, 76, 77, 78, 79, \
									80, 81, 82, 83, 84, 85, 86, 87, 88, 89, \
									90, 91, 92, 93, 94, 95, 96, 97, 98, 99};
	uint8_t __maybe_unused rdata8[100];

	uint16_t __maybe_unused data16[10] = {0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888, 0x9999, 0xaaaa};
	uint16_t __maybe_unused data16_long[20] = {0xbbbb, 0xcccc, 0xdddd, 0xeeee, 0xffff, 0, 0x1111, 0x2222, 0x3333, 0x4444, 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888, 0x9999, 0xaaaa};
	uint16_t __maybe_unused data16_100[100] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, \
									10, 11, 12, 13, 14, 15, 16, 17, 18, 19, \
									20, 21, 22, 23, 24, 25, 26, 27, 28, 29, \
									30, 31, 32, 33, 34, 35, 36, 37, 38, 39, \
									40, 41, 42, 43, 44, 45, 46, 47, 48, 49, \
									50, 51, 52, 53, 54, 55, 56, 57, 58, 59, \
									60, 61, 62, 63, 64, 65, 66, 67, 68, 69, \
									70, 71, 72, 73, 74, 75, 76, 77, 78, 79, \
									80, 81, 82, 83, 84, 85, 86, 87, 88, 89, \
									90, 91, 92, 93, 94, 95, 96, 97, 98, 99};
	uint16_t __maybe_unused rdata16[100];

	uint8_t tx[3], rx[3];
	uint32_t num_rxpkts, i;

	DMSG("Hello!\n");
	peri_init();
	pl022_start();

	#if 1
	tx[0] = 0x1;
	tx[1] = 0x80;
	tx[2] = 0;
	rx[0] = 0;
	rx[1] = 0;
	rx[2] = 0;

	spi_txrx8(tx, rx, 3, &num_rxpkts);
	for (i=0; i<num_rxpkts; i++)
	{
		DMSG("rx[%u] = 0x%x\n", i,rx[i]);
	}

	spi_txrx8(data8, rdata8, 10, &num_rxpkts);
	for (i=0; i<num_rxpkts; i++)
	{
		DMSG("rx[%u] = 0x%x\n", i, rdata8[i]);
	}

	spi_txrx8(data8_long, rdata8, 20, &num_rxpkts);
	for (i=0; i<num_rxpkts; i++)
	{
		DMSG("rx[%u] = 0x%x\n", i, rdata8[i]);
	}

	spi_txrx8(data8_100, rdata8, 100, &num_rxpkts);
	for (i=0; i<num_rxpkts; i++)
	{
		DMSG("rx[%u] = 0x%x\n", i, rdata8[i]);
	}
	//#else
	spi_txrx16(data16, rdata16, 10, &num_rxpkts);
	for (i=0; i<num_rxpkts; i++)
	{
		DMSG("rx[%u] = 0x%x\n", i, data16[i]);
	}

	spi_txrx16(data16_long, rdata16, 20, &num_rxpkts);
	for (i=0; i<num_rxpkts; i++)
	{
		DMSG("rx[%u] = 0x%x\n", i, data16[i]);
	}

	spi_txrx16(data16_100, rdata16, 100, &num_rxpkts);
	for (i=0; i<num_rxpkts; i++)
	{
		DMSG("rx[%u] = 0x%x\n", i, data16[i]);
	}
	#endif

	pl022_end();
}

static TEE_Result spi_test(void)
{
	spi_test2();
	return TEE_SUCCESS;
}

service_init(spi_test);
