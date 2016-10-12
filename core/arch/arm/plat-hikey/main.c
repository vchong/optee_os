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
#include <kernel/misc.h>
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

static enum teecore_memtypes g_memtype_dev = MEM_AREA_IO_NSEC;

register_phys_mem(MEM_AREA_IO_NSEC, CONSOLE_UART_BASE, PL011_REG_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, GPIO6_BASE, PL061_REG_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, PERI_BASE, PERI_BASE_REG_SIZE);

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

static TEE_Result spi_test(void);
void console_init(void)
{
	pl011_init(console_base(), CONSOLE_UART_CLK_IN_HZ, CONSOLE_BAUDRATE);
	spi_test();
}

void console_putc(int ch)
{
	vaddr_t base = console_base();

	if (ch == '\n')
		pl011_putc('\r', base);
	pl011_putc(ch, base);
}

void console_flush(void)
{
	pl011_flush(console_base());
}

static vaddr_t get_va(paddr_t pa)
{
	/* WARNING: Make sure NOT static as in console_base */
	void *va;

	if (cpu_mmu_enabled()) {
		va = phys_to_virt(pa, g_memtype_dev);
		return (vaddr_t)va;
	}
	return (vaddr_t)pa;
}

#define PIN_MUX0 0
#define PIN_MUX1 1
#define PIN_MUX3 3

//0 nopull, 1 pullup, 2 pulldown
#define PIN_NP 0
#define PIN_PU 1
#define PIN_PD 2
#define DRIVE1_04MA 8
#define DRIVE1_08MA 0x10
#define DRIVE1_10MA 0x18

static void platform_spi_enable(void)
{
	int i;

	vaddr_t peribase = get_va(PERI_BASE);
	vaddr_t pmx0base = get_va(PMX0_BASE);
	vaddr_t pmx1base = get_va(PMX1_BASE);
	vaddr_t pmx2base = get_va(PMX2_BASE);

	uint32_t shifted_val, read_val;

	DMSG("peribase: 0x%" PRIxVA "\n", peribase);
	DMSG("pmx0base: 0x%" PRIxVA "\n", pmx0base);
	DMSG("pmx1base: 0x%" PRIxVA "\n", pmx1base);
	DMSG("pmx2base: 0x%" PRIxVA "\n", pmx2base);

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* take SPI0 out of reset */
	/* no need to read PERI_SC_PERIPH_RSTDIS3 first as all the bits are processed and cleared after writing */
	shifted_val = PERI_RST3_SSP;
	write32(shifted_val, peribase + PERI_SC_PERIPH_RSTDIS3);

	/* wait until the requested device is out of reset, and ready to be used */
	do {
	  read_val = read32(peribase + PERI_SC_PERIPH_RSTSTAT3);
	} while (read_val & shifted_val);

	DMSG("read_val: 0x%x\n", read_val);
	DMSG("PERI_SC_PERIPH_RSTSTAT3: 0x%x\n", read32(peribase + PERI_SC_PERIPH_RSTSTAT3));
	DMSG("shifted_val: 0x%x\n", shifted_val);
	DMSG("PERI_SC_PERIPH_RSTDIS3: 0x%x\n", read32(peribase + PERI_SC_PERIPH_RSTDIS3));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* enable SPI clock */
	/* no need to read PERI_SC_PERIPH_CLKEN3 first as all the bits are processed and cleared after writing */
	shifted_val = PERI_CLK3_SSP;
	write32(shifted_val, peribase + PERI_SC_PERIPH_CLKEN3);

	DMSG("PERI_SC_PERIPH_CLKSTAT3: 0x%x\n", read32(peribase + PERI_SC_PERIPH_CLKSTAT3));
	DMSG("shifted_val: 0x%x\n", shifted_val);
	DMSG("PERI_SC_PERIPH_CLKEN3: 0x%x\n", read32(peribase + PERI_SC_PERIPH_CLKEN3));

	DMSG("configure fr dt\n");
	DMSG("configure pmx0\n");

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	DMSG("pmx0base + PMX_IOXG000: 0x%x\n", read32(pmx0base + PMX_IOXG000));
	write32(PIN_MUX0, pmx0base + PMX_IOXG000);
	DMSG("pmx0base + PMX_IOXG000: 0x%x\n", read32(pmx0base + PMX_IOXG000));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	for (i=0; i<10; i++) {
		DMSG("pmx0base + PMX_IOXG064 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG064 + i*4));
		write32(PIN_MUX0, pmx0base + PMX_IOXG064 + i*4);
		DMSG("pmx0base + PMX_IOXG064 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG064 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* spi uses sd_pmx_func which is pinctrl-0 */
	for (i=0; i<6; i++) {
		DMSG("pmx0base + PMX_IOXG003 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG003 + i*4));
		write32(PIN_MUX0, pmx0base + PMX_IOXG003 + i*4);
		DMSG("pmx0base + PMX_IOXG003 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG003 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* spi uses sdio_pmx_func which is pinctrl-0 */
	for (i=0; i<6; i++) {
		DMSG("pmx0base + PMX_IOXG074 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG074 + i*4));
		write32(PIN_MUX0, pmx0base + PMX_IOXG074 + i*4);
		DMSG("pmx0base + PMX_IOXG074 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG074 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* isp_pmx_func */
	for (i=0; i<3; i++) {
		DMSG("pmx0base + PMX_IOXG009 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG009 + i*4));
		write32(PIN_MUX0, pmx0base + PMX_IOXG009 + i*4);
		DMSG("pmx0base + PMX_IOXG009 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG009 + i*4));
	}
	for (i=0; i<3; i++) {
		DMSG("pmx0base + PMX_IOXG012 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG012 + i*4));
		write32(PIN_MUX1, pmx0base + PMX_IOXG012 + i*4);
		DMSG("pmx0base + PMX_IOXG012 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG012 + i*4));
	}
	for (i=0; i<4; i++) {
		DMSG("pmx0base + PMX_IOXG015 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG015 + i*4));
		write32(PIN_MUX0, pmx0base + PMX_IOXG015 + i*4);
		DMSG("pmx0base + PMX_IOXG015 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG015 + i*4));
	}
	for (i=0; i<2; i++) {
		DMSG("pmx0base + PMX_IOXG019 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG019 + i*4));
		write32(PIN_MUX1, pmx0base + PMX_IOXG019 + i*4);
		DMSG("pmx0base + PMX_IOXG019 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG019 + i*4));
	}
	for (i=0; i<4; i++) {
		DMSG("pmx0base + PMX_IOXG021 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG021 + i*4));
		write32(PIN_MUX0, pmx0base + PMX_IOXG021 + i*4);
		DMSG("pmx0base + PMX_IOXG021 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG021 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* hkadc_ssi_pmx_func */
	DMSG("pmx0base + PMX_IOXG026 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG026 + i*4));
	write32(PIN_MUX0, pmx0base + PMX_IOXG026 + i*4);
	DMSG("pmx0base + PMX_IOXG026 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG026 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* codec_clk_pmx_func */
	DMSG("pmx0base + PMX_IOXG027 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG027 + i*4));
	write32(PIN_MUX0, pmx0base + PMX_IOXG027 + i*4);
	DMSG("pmx0base + PMX_IOXG027 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG027 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* codec_pmx_func */
	DMSG("pmx0base + PMX_IOXG028 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG028 + i*4));
	write32(PIN_MUX1, pmx0base + PMX_IOXG028 + i*4);
	DMSG("pmx0base + PMX_IOXG028 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG028 + i*4));

	for (i=0; i<3; i++) {
		DMSG("pmx0base + PMX_IOXG029 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG029 + i*4));
		write32(PIN_MUX0, pmx0base + PMX_IOXG029 + i*4);
		DMSG("pmx0base + PMX_IOXG029 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG029 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* fm_pmx_func */
	for (i=0; i<4; i++) {
		DMSG("pmx0base + PMX_IOXG032 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG032 + i*4));
		write32(PIN_MUX1, pmx0base + PMX_IOXG032 + i*4);
		DMSG("pmx0base + PMX_IOXG032 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG032 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* bt_pmx_func */
	for (i=0; i<4; i++) {
		DMSG("pmx0base + PMX_IOXG036 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG036 + i*4));
		write32(PIN_MUX0, pmx0base + PMX_IOXG036 + i*4);
		DMSG("pmx0base + PMX_IOXG036 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG036 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* pwm_in_pmx_func */
	DMSG("pmx0base + PMX_IOXG046 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG046 + i*4));
	write32(PIN_MUX1, pmx0base + PMX_IOXG046 + i*4);
	DMSG("pmx0base + PMX_IOXG046 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG046 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* bl_pwm_pmx_func */
	DMSG("pmx0base + PMX_IOXG047 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG047 + i*4));
	write32(PIN_MUX1, pmx0base + PMX_IOXG047 + i*4);
	DMSG("pmx0base + PMX_IOXG047 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG047 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* uart0-2_pmx_func */
	for (i=0; i<10; i++) {
		DMSG("pmx0base + PMX_IOXG048 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG048 + i*4));
		write32(PIN_MUX0, pmx0base + PMX_IOXG048 + i*4);
		DMSG("pmx0base + PMX_IOXG048 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG048 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* uart3_pmx_func */
	for (i=0; i<4; i++) {
		DMSG("pmx0base + PMX_IOXG096 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG096 + i*4));
		write32(PIN_MUX1, pmx0base + PMX_IOXG096 + i*4);
		DMSG("pmx0base + PMX_IOXG096 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG096 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* uart4-5_pmx_func */
	for (i=0; i<6; i++) {
		DMSG("pmx0base + PMX_IOXG114 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG114 + i*4));
		write32(PIN_MUX1, pmx0base + PMX_IOXG114 + i*4);
		DMSG("pmx0base + PMX_IOXG114 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG114 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* i2c0-2_pmx_func */
	for (i=0; i<6; i++) {
		DMSG("pmx0base + PMX_IOXG058 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG058 + i*4));
		write32(PIN_MUX0, pmx0base + PMX_IOXG058 + i*4);
		DMSG("pmx0base + PMX_IOXG058 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG058 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* spi0_pmx_func */
	for (i=0; i<4; i++) {
		DMSG("pmx0base + PMX_IOXG104 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG104 + i*4));
		write32(PIN_MUX1, pmx0base + PMX_IOXG104 + i*4);
		DMSG("pmx0base + PMX_IOXG104 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG104 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* modem_pmx_func */
	for (i=0; i<2; i++) {
		DMSG("pmx0base + PMX_IOXG102 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG102 + i*4));
		write32(PIN_MUX3, pmx0base + PMX_IOXG102 + i*4);
		DMSG("pmx0base + PMX_IOXG102 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG102 + i*4));
	}
	for (i=0; i<2; i++) {
		DMSG("pmx0base + PMX_IOXG116 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG116 + i*4));
		write32(PIN_MUX3, pmx0base + PMX_IOXG116 + i*4);
		DMSG("pmx0base + PMX_IOXG116 + 0x%x: 0x%x\n", i*4, read32(pmx0base + PMX_IOXG116 + i*4));
	}






	DMSG("configure pmx1\n");

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* boot_sel_cfg_func */
	DMSG("pmx1base + PMX_IOXG000 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG000 + i*4));
	write32(PIN_PU, pmx1base + PMX_IOXG000 + i*4);
	DMSG("pmx1base + PMX_IOXG000 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG000 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* hkadc_ssi_cfg_func */
	DMSG("pmx1base + PMX_IOXG027 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG027 + i*4));
	write32(PIN_NP, pmx1base + PMX_IOXG027 + i*4);
	DMSG("pmx1base + PMX_IOXG027 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG027 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* emmc_clk_cfg_func */
	DMSG("pmx1base + PMX_IOXG065 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG065 + i*4));
	write32(DRIVE1_08MA, pmx1base + PMX_IOXG065 + i*4);
	DMSG("pmx1base + PMX_IOXG065 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG065 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* emmc_cfg_func */
	for (i=0; i<9; i++) {
		DMSG("pmx1base + PMX_IOXG066 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG066 + i*4));
		write32(PIN_PU+DRIVE1_04MA, pmx1base + PMX_IOXG066 + i*4);
		DMSG("pmx1base + PMX_IOXG066 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG066 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* emmc_rst_cfg_func */
	DMSG("pmx1base + PMX_IOXG075 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG075 + i*4));
	write32(DRIVE1_04MA, pmx1base + PMX_IOXG075 + i*4);
	DMSG("pmx1base + PMX_IOXG075 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG075 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* sd_clk_cfg_func */
	DMSG("pmx1base + PMX_IOXG003 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG003 + i*4));
	write32(DRIVE1_10MA, pmx1base + PMX_IOXG003 + i*4);
	DMSG("pmx1base + PMX_IOXG003 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG003 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* sd_cfg_func */
	for (i=0; i<5; i++) {
		DMSG("pmx1base + PMX_IOXG004 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG004 + i*4));
		write32(DRIVE1_08MA, pmx1base + PMX_IOXG004 + i*4);
		DMSG("pmx1base + PMX_IOXG004 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG004 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* sdio_clk_cfg_func */
	DMSG("pmx1base + PMX_IOXG077 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG077 + i*4));
	write32(DRIVE1_08MA, pmx1base + PMX_IOXG077 + i*4);
	DMSG("pmx1base + PMX_IOXG077 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG077 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* sdio_cfg_func */
	for (i=0; i<5; i++) {
		DMSG("pmx1base + PMX_IOXG078 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG078 + i*4));
		write32(PIN_PU+DRIVE1_04MA, pmx1base + PMX_IOXG078 + i*4);
		DMSG("pmx1base + PMX_IOXG078 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG078 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* isp_cfg_func1 */
	for (i=0; i<11; i++) {
		DMSG("pmx1base + PMX_IOXG010 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG010 + i*4));
		write32(PIN_NP, pmx1base + PMX_IOXG010 + i*4);
		DMSG("pmx1base + PMX_IOXG010 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG010 + i*4));
	}
	for (i=0; i<4; i++) {
		DMSG("pmx1base + PMX_IOXG022 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG022 + i*4));
		write32(PIN_NP, pmx1base + PMX_IOXG022 + i*4);
		DMSG("pmx1base + PMX_IOXG022 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG022 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* isp_cfg_func2 */
	DMSG("pmx1base + PMX_IOXG021 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG021 + i*4));
	write32(PIN_PD, pmx1base + PMX_IOXG021 + i*4);
	DMSG("pmx1base + PMX_IOXG021 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG021 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* codec_clk_cfg_func */
	DMSG("pmx1base + PMX_IOXG028 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG028 + i*4));
	write32(DRIVE1_04MA, pmx1base + PMX_IOXG028 + i*4);
	DMSG("pmx1base + PMX_IOXG028 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG028 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* codec_cfg_func1 */
	DMSG("pmx1base + PMX_IOXG029 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG029 + i*4));
	write32(PIN_PD, pmx1base + PMX_IOXG029 + i*4);
	DMSG("pmx1base + PMX_IOXG029 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG029 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* codec_cfg_func2 */
	for (i=0; i<3; i++) {
		DMSG("pmx1base + PMX_IOXG030 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG030 + i*4));
		write32(DRIVE1_04MA, pmx1base + PMX_IOXG030 + i*4);
		DMSG("pmx1base + PMX_IOXG030 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG030 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* fm_cfg_func */
	for (i=0; i<4; i++) {
		DMSG("pmx1base + PMX_IOXG033 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG033 + i*4));
		write32(PIN_PD, pmx1base + PMX_IOXG033 + i*4);
		DMSG("pmx1base + PMX_IOXG033 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG033 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* bt_cfg_func */
	for (i=0; i<4; i++) {
		DMSG("pmx1base + PMX_IOXG037 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG037 + i*4));
		write32(PIN_NP, pmx1base + PMX_IOXG037 + i*4);
		DMSG("pmx1base + PMX_IOXG037 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG037 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* pwm_in_cfg_func */
	DMSG("pmx1base + PMX_IOXG047 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG047 + i*4));
	write32(PIN_PD, pmx1base + PMX_IOXG047 + i*4);
	DMSG("pmx1base + PMX_IOXG047 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG047 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* bl_pwm_cfg_func */
	DMSG("pmx1base + PMX_IOXG048 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG048 + i*4));
	write32(PIN_PD, pmx1base + PMX_IOXG048 + i*4);
	DMSG("pmx1base + PMX_IOXG048 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG048 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* uart0_cfg_func1 */
	DMSG("pmx1base + PMX_IOXG049 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG049 + i*4));
	write32(PIN_PU, pmx1base + PMX_IOXG049 + i*4);
	DMSG("pmx1base + PMX_IOXG049 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG049 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* uart0_cfg_func2 */
	DMSG("pmx1base + PMX_IOXG050 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG050 + i*4));
	write32(DRIVE1_04MA, pmx1base + PMX_IOXG050 + i*4);
	DMSG("pmx1base + PMX_IOXG050 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG050 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* uart1_cfg_func1 */
	DMSG("pmx1base + PMX_IOXG051 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG051 + i*4));
	write32(PIN_PU, pmx1base + PMX_IOXG051 + i*4);
	DMSG("pmx1base + PMX_IOXG051 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG051 + i*4));
	DMSG("pmx1base + PMX_IOXG053 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG053 + i*4));
	write32(PIN_PU, pmx1base + PMX_IOXG053 + i*4);
	DMSG("pmx1base + PMX_IOXG053 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG053 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* uart1_cfg_func2 */
	DMSG("pmx1base + PMX_IOXG052 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG052 + i*4));
	write32(PIN_NP, pmx1base + PMX_IOXG052 + i*4);
	DMSG("pmx1base + PMX_IOXG052 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG052 + i*4));
	DMSG("pmx1base + PMX_IOXG054 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG054 + i*4));
	write32(PIN_NP, pmx1base + PMX_IOXG054 + i*4);
	DMSG("pmx1base + PMX_IOXG054 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG054 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* uart2_cfg_func */
	for (i=0; i<4; i++) {
		DMSG("pmx1base + PMX_IOXG055 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG055 + i*4));
		write32(PIN_NP, pmx1base + PMX_IOXG055 + i*4);
		DMSG("pmx1base + PMX_IOXG055 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG055 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* uart3_cfg_func */
	for (i=0; i<4; i++) {
		DMSG("pmx1base + PMX_IOXG100 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG100 + i*4));
		write32(PIN_PD, pmx1base + PMX_IOXG100 + i*4);
		DMSG("pmx1base + PMX_IOXG100 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG100 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* uart4-5_cfg_func */
	for (i=0; i<6; i++) {
		DMSG("pmx1base + PMX_IOXG118 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG118 + i*4));
		write32(PIN_PD, pmx1base + PMX_IOXG118 + i*4);
		DMSG("pmx1base + PMX_IOXG118 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG118 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* i2c0-2_cfg_func */
	for (i=0; i<6; i++) {
		DMSG("pmx1base + PMX_IOXG059 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG059 + i*4));
		write32(PIN_NP, pmx1base + PMX_IOXG059 + i*4);
		DMSG("pmx1base + PMX_IOXG059 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG059 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* spi0_cfg_func */
	for (i=0; i<4; i++) {
		DMSG("pmx1base + PMX_IOXG108 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG108 + i*4));
		write32(PIN_NP, pmx1base + PMX_IOXG108 + i*4);
		DMSG("pmx1base + PMX_IOXG108 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG108 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* modem_pcm_cfg_func */
	for (i=0; i<2; i++) {
		DMSG("pmx1base + PMX_IOXG106 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG106 + i*4));
		write32(PIN_PD, pmx1base + PMX_IOXG106 + i*4);
		DMSG("pmx1base + PMX_IOXG106 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG106 + i*4));
	}
	for (i=0; i<2; i++) {
		DMSG("pmx1base + PMX_IOXG120 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG120 + i*4));
		write32(PIN_PD, pmx1base + PMX_IOXG120 + i*4);
		DMSG("pmx1base + PMX_IOXG120 + 0x%x: 0x%x\n", i*4, read32(pmx1base + PMX_IOXG120 + i*4));
	}






	DMSG("configure pmx2\n");

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* rstout_n_cfg_func */
	/* pmu_peri_en_cfg_func */
	/* sysclk0_en_cfg_funcc */
	for (i=0; i<3; i++) {
		DMSG("pmx2base + PMX_IOXG000 + 0x%x: 0x%x\n", i*4, read32(pmx2base + PMX_IOXG000 + i*4));
		write32(PIN_NP, pmx2base + PMX_IOXG000 + i*4);
		DMSG("pmx2base + PMX_IOXG000 + 0x%x: 0x%x\n", i*4, read32(pmx2base + PMX_IOXG000 + i*4));
	}

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* jtag_tdo_cfg_func */
	DMSG("pmx2base + PMX_IOXG003 + 0x%x: 0x%x\n", i*4, read32(pmx2base + PMX_IOXG003 + i*4));
	write32(DRIVE1_08MA, pmx2base + PMX_IOXG003 + i*4);
	DMSG("pmx2base + PMX_IOXG003 + 0x%x: 0x%x\n", i*4, read32(pmx2base + PMX_IOXG003 + i*4));

	while (!pl011_have_rx_data(CONSOLE_UART_BASE));
	DMSG("cpu0 %zu: got key=%c", get_core_pos(), (char)pl011_getchar(CONSOLE_UART_BASE));

	/* rf_reset_cfg_func */
	for (i=0; i<2; i++) {
		DMSG("pmx2base + PMX_IOXG028 + 0x%x: 0x%x\n", i*4, read32(pmx2base + PMX_IOXG028 + i*4));
		write32(PIN_NP, pmx2base + PMX_IOXG028 + i*4);
		DMSG("pmx2base + PMX_IOXG028 + 0x%x: 0x%x\n", i*4, read32(pmx2base + PMX_IOXG028 + i*4));
	}
}

static struct pl061_data platform_pl061_data;
static vaddr_t gpio6bs;

static void peri_init_n_config(void)
{
	vaddr_t gpio6base = get_va(GPIO6_BASE);

	gpio6bs = gpio6base;

	DMSG("gpio6base: 0x%" PRIxVA "\n", gpio6base);

	/* WARNING: Do this FIRST before anything else, even if gpio stuffs!!!!!! */
	platform_spi_enable();

	pl061_init(&platform_pl061_data);
	pl061_register(gpio6base, 6);
	DMSG("mask/disable interrupt for cs\n");
	platform_pl061_data.chip.ops->set_interrupt(GPIO6_2, GPIO_INTERRUPT_DISABLE);
	DMSG("enable software mode control for cs\n");
	pl061_set_mode_control(GPIO6_2, PL061_MC_SW);
	platform_pl061_data.chip.ops->set_direction(GPIO6_2, GPIO_DIR_OUT);
	platform_pl061_data.chip.ops->set_value(GPIO6_2, GPIO_LEVEL_HIGH);
}

static void spi_test_linksprite(void)
{
	int ch = 'c';
	paddr_t uart_base = console_base();

	while (1)
	{
		while (!pl011_have_rx_data(uart_base));

		ch = pl011_getchar(uart_base);
		DMSG("cpu %zu: got 0x%x %c", get_core_pos(), ch, (char)ch);

		switch (ch)
		{
			case 't':
				platform_pl061_data.chip.ops->set_value(GPIO6_2, GPIO_LEVEL_LOW);
				break;
			case 'u':
				platform_pl061_data.chip.ops->set_value(GPIO6_2, GPIO_LEVEL_HIGH);
				break;
			case 'v':
				io_mask32(gpio6bs + (1<<4), 0, (1<<2));
				break;
			case 'w':
				io_mask32(gpio6bs + (1<<4), (1<<2), (1<<2));
				break;
			case 'x':
				write8(0, gpio6bs + (1<<4));
				break;
			case 'y':
				write8(4, gpio6bs + (1<<4));
			default:
				break;
		}

		if (ch == 'q')
			break;
	}
}

void spi_test2(void)
{
	DMSG("Hello!\n");
	DMSG("sizeof(void *): %lu\n", sizeof(void *));
	DMSG("sizeof(bool): %lu\n", sizeof(bool));
	DMSG("sizeof(uint8_t): %lu\n", sizeof(uint8_t));
	DMSG("sizeof(uint16_t): %lu\n", sizeof(uint16_t));
	DMSG("sizeof(uint32_t): %lu\n", sizeof(uint32_t));
	DMSG("sizeof(uint64_t): %lu\n", sizeof(uint64_t));
	DMSG("sizeof(vaddr_t): %lu\n", sizeof(vaddr_t));
	DMSG("sizeof(paddr_t): %lu\n", sizeof(paddr_t));
	DMSG("sizeof(int): %lu\n", sizeof(int));
	DMSG("sizeof(unsigned int ): %lu\n", sizeof(unsigned int));
	DMSG("sizeof(unsigned): %lu\n", sizeof(unsigned));
	DMSG("sizeof(long): %lu\n", sizeof(long));
	DMSG("sizeof(unsigned long): %lu\n", sizeof(unsigned long));
	DMSG("sizeof(long long): %lu\n", sizeof(long long));
	DMSG("sizeof(unsigned long long): %lu\n", sizeof(unsigned long long));
	DMSG("sizeof(float): %lu\n", sizeof(float));
	DMSG("sizeof(struct gpio_ops): %lu\n", sizeof(struct gpio_ops));
	DMSG("sizeof(struct gpio_chip): %lu\n", sizeof(struct gpio_chip));
	DMSG("sizeof(struct pl061_data): %lu\n", sizeof(struct pl061_data));
	DMSG("sizeof(struct spi_ops): %lu\n", sizeof(struct spi_ops));
	DMSG("sizeof(struct spi_chip): %lu\n", sizeof(struct spi_chip));
	DMSG("sizeof(struct pl022_data): %lu\n", sizeof(struct pl022_data));

	peri_init_n_config();

	spi_test_linksprite();
}

static TEE_Result spi_test(void)
{
	if (cpu_mmu_enabled())
		DMSG("cpu_mmu_enabled true\n");
	else
		DMSG("cpu_mmu_enabled false\n");
	spi_test2();
	return TEE_SUCCESS;
}

driver_init_late(spi_test);
//service_init(spi_test);
