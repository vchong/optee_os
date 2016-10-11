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
	spi_test();
	pl011_init(console_base(), CONSOLE_UART_CLK_IN_HZ, CONSOLE_BAUDRATE);
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

static void platform_spi_enable(void)
{

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
	pl061_set_direction(GPIO6_2, GPIO_DIR_OUT);
	pl061_set_value(GPIO6_2, GPIO_LEVEL_HIGH);
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
