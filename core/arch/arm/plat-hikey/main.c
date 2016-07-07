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
#include <drivers/common.h>
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

register_phys_mem(MEM_AREA_IO_NSEC, CONSOLE_UART_BASE, PL011_REG_SIZE);
#if 1
register_phys_mem(MEM_AREA_IO_NSEC, PERI_BASE, PERI_BASE_REG_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, SPI_BASE, PL022_REG_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, GPIO6_BASE, PL061_REG_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, PMX0_BASE, PMX0_REG_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, PMX1_BASE, PMX1_REG_SIZE);
#else
register_phys_mem(MEM_AREA_IO_NSEC, PERI_BASE, CORE_MMU_DEVICE_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, SPI_BASE, CORE_MMU_DEVICE_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, GPIO6_BASE, CORE_MMU_DEVICE_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, PMX0_BASE, CORE_MMU_DEVICE_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, PMX1_BASE, CORE_MMU_DEVICE_SIZE);
#endif

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

#if 0
static vaddr_t peri_base(void)
{
	static void *va1;

	if (cpu_mmu_enabled()) {
		if (!va1)
			va1 = phys_to_virt(PERI_BASE, MEM_AREA_IO_NSEC);
		return (vaddr_t)va1;
	}
	return PERI_BASE;
}

static vaddr_t pmx0_base(void)
{
	static void *va2;

	if (cpu_mmu_enabled()) {
		if (!va2)
			va2 = phys_to_virt(PMX0_BASE, MEM_AREA_IO_NSEC);
		return (vaddr_t)va2;
	}
	return PMX0_BASE;
}

static vaddr_t pmx1_base(void)
{
	static void *va3;

	if (cpu_mmu_enabled()) {
		if (!va3)
			va3 = phys_to_virt(PMX1_BASE, MEM_AREA_IO_NSEC);
		return (vaddr_t)va3;
	}
	return PMX1_BASE;
}

static vaddr_t gpio6_base(void)
{
	static void *va4;

	if (cpu_mmu_enabled()) {
		if (!va4)
			va4 = phys_to_virt(GPIO6_BASE, MEM_AREA_IO_NSEC);
		return (vaddr_t)va4;
	}
	return GPIO6_BASE;
}

static vaddr_t spi_base(void)
{
	static void *va5;

	if (cpu_mmu_enabled()) {
		if (!va5)
			va5 = phys_to_virt(SPI_BASE, MEM_AREA_IO_NSEC);
		return (vaddr_t)va5;
	}
	return SPI_BASE;
}
#endif

static vaddr_t get_va(paddr_t pa)
{
	/* WARNING: Make sure NOT static as in console_base */
	void *va;

	if (cpu_mmu_enabled()) {
		va = phys_to_virt(pa, MEM_AREA_IO_NSEC);
		return (vaddr_t)va;
	}
	return (vaddr_t)pa;
}

static void platform_spi_enable(void)
{
	#if 1
	vaddr_t peribase = get_va(PERI_BASE);
	vaddr_t pmx0base = get_va(PMX0_BASE);
	vaddr_t pmx1base = get_va(PMX1_BASE);
	#else
	vaddr_t peribase = peri_base();
	vaddr_t pmx0base = pmx0_base();
	vaddr_t pmx1base = pmx1_base();
	#endif
	//vaddr_t tst1 = get_va(CONSOLE_UART_BASE);
	//vaddr_t tst2 = get_va(CONSOLE_UART_BASE);

	uint32_t shifted_val, read_val;

	//DMSG("tst1: 0x%" PRIxVA "\n", tst1);
	//DMSG("tst2: 0x%" PRIxVA "\n", tst2);

	DMSG("peribase: 0x%" PRIxVA "\n", peribase);
	DMSG("pmx0base: 0x%" PRIxVA "\n", pmx0base);
	DMSG("pmx1base: 0x%" PRIxVA "\n", pmx1base);

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

	/* enable SPI clock */
	/* no need to read PERI_SC_PERIPH_CLKEN3 first as all the bits are processed and cleared after writing */
	shifted_val = PERI_CLK3_SSP;
	write32(shifted_val, peribase + PERI_SC_PERIPH_CLKEN3);

	DMSG("PERI_SC_PERIPH_CLKSTAT3: 0x%x\n", read32(peribase + PERI_SC_PERIPH_CLKSTAT3));
	DMSG("shifted_val: 0x%x\n", shifted_val);
	DMSG("PERI_SC_PERIPH_CLKEN3: 0x%x\n", read32(peribase + PERI_SC_PERIPH_CLKEN3));

	/* configure pin bias: 0: nopull, 1: pullup, 2: pulldown */
	DMSG("configure gpio6_{0:3} as nopull and 02ma drive\n");
	write32(0, pmx1base + PMX1_IOCG104); /* 0xF70109B0 */
	write32(0, pmx1base + PMX1_IOCG105); /* 0xF70109B4 */
	write32(0, pmx1base + PMX1_IOCG106); /* 0xF70109B8 */
	write32(0, pmx1base + PMX1_IOCG107); /* 0xF70109BC */
	DMSG("pmx1base + PMX1_IOCG104: 0x%x\n", read32(pmx1base + PMX1_IOCG104));
	DMSG("pmx1base + PMX1_IOCG105: 0x%x\n", read32(pmx1base + PMX1_IOCG105));
	DMSG("pmx1base + PMX1_IOCG106: 0x%x\n", read32(pmx1base + PMX1_IOCG106));
	DMSG("pmx1base + PMX1_IOCG107: 0x%x\n", read32(pmx1base + PMX1_IOCG107));

	/* configure pin mux: 0: gpio, 1: spi*/
	DMSG("configure gpio6_{0,1,3} as spi\n");
	DMSG("configure gpio6_2 as gpio, else hw ip will try to control it as well, causing interference\n");
	write32(1, pmx0base + PMX0_IOMG104); /* 0xF70101A0 */
	write32(1, pmx0base + PMX0_IOMG105); /* 0xF70101A4 */
	write32(0, pmx0base + PMX0_IOMG106); /* 0xF70101A8 */
	write32(1, pmx0base + PMX0_IOMG107); /* 0xF70101AC */
	DMSG("pmx0base + PMX0_IOMG104: 0x%x\n", read32(pmx0base + PMX0_IOMG104));
	DMSG("pmx0base + PMX0_IOMG105: 0x%x\n", read32(pmx0base + PMX0_IOMG105));
	DMSG("pmx0base + PMX0_IOMG106: 0x%x\n", read32(pmx0base + PMX0_IOMG106));
	DMSG("pmx0base + PMX0_IOMG107: 0x%x\n", read32(pmx0base + PMX0_IOMG107));
}


/* default preset values that can be overwritten */
static struct pl022_cfg platform_pl022_cfg = {
	.clk_hz = SPI_CLK_HZ,
	.speed_hz = 500000,
	.cs_gpio_pin = GPIO6_2,
	.mode = SPI_MODE0,
	.data_size_bits = 8,
	.loopback = false,
};

static vaddr_t gp6bs;
static void peri_init_n_config(void)
{
	#if 1
	vaddr_t gpio6base = get_va(GPIO6_BASE);
	vaddr_t spibase = get_va(SPI_BASE);
	#else
	vaddr_t gpio6base = gpio6_base();
	vaddr_t spibase = spi_base();
	#endif

	gp6bs = gpio6base;

	DMSG("gpio6base: 0x%" PRIxVA "\n", gpio6base);
	DMSG("spibase: 0x%" PRIxVA "\n", spibase);

	/* WARNING: Do this FIRST before anything else, even if gpio stuffs!!!!!! */
	platform_spi_enable();

	pl061_gpio_init();
	pl061_gpio_register(gpio6base, 6);

	DMSG("mask/disable interrupt for cs\n");
	pl061_set_interrupt(GPIO6_2, PL061_INTERRUPT_DISABLE);
	DMSG("enable software mode control for cs\n");
	pl061_set_mode_control(GPIO6_2, PL061_MC_SW);
	
	platform_pl022_cfg.base = spibase;
	platform_pl022_cfg.cs_gpio_base = gpio6base;
	platform_pl022_cfg.cs_gpio_pin = GPIO6_2;
	#if 1
	platform_pl022_cfg.data_size_bits = 8;
	platform_pl022_cfg.loopback = false;
	#else
	platform_pl022_cfg.data_size_bits = 16;
	platform_pl022_cfg.loopback = true;
	#endif
	pl022_init(&platform_pl022_cfg);
	pl022_configure(&platform_pl022_cfg);
}

static void spi_test_lbm(void)
{
	uint8_t __maybe_unused data8[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	uint8_t __maybe_unused data8_long[20] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0xaa, 0xbb, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};
	/*
	 * WARNING: too many arrays can cause error like below!!!!
	 * ERROR: TEE-CORE: Assertion 'GET_START_CANARY(stack_tmp, n) == START_CANARY_VALUE' failed at core/arch/arm/kernel/thr0
	 */
	#if 0
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
	#else
	uint8_t __maybe_unused rdata8[20];
	#endif

	uint16_t __maybe_unused data16[10] = {0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888, 0x9999, 0xaaaa};
	uint16_t __maybe_unused data16_long[20] = {0xbbbb, 0xcccc, 0xdddd, 0xeeee, 0xffff, 0, 0x1111, 0x2222, 0x3333, 0x4444, 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888, 0x9999, 0xaaaa};
	#if 0
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
	#else
	uint16_t __maybe_unused rdata16[20];
	#endif

	uint32_t num_rxpkts, i;

	#if 1
	platform_pl022_cfg->ops->spi_txrx8(&platform_pl022_cfg, data8, rdata8, 10, &num_rxpkts);
	for (i=0; i<num_rxpkts; i++)
	{
		DMSG("rx[%u] = 0x%x\n", i, rdata8[i]);
	}

	platform_pl022_cfg->ops->spi_txrx8(&platform_pl022_cfg, data8_long, rdata8, 20, &num_rxpkts);
	for (i=0; i<num_rxpkts; i++)
	{
		DMSG("rx[%u] = 0x%x\n", i, rdata8[i]);
	}

	#if 0
	platform_pl022_cfg->ops->spi_txrx8(&platform_pl022_cfg, data8_100, rdata8, 100, &num_rxpkts);
	for (i=0; i<num_rxpkts; i++)
	{
		DMSG("rx[%u] = 0x%x\n", i, rdata8[i]);
	}
	#endif
	//#else
	platform_pl022_cfg->ops->spi_txrx16(&platform_pl022_cfg, data16, rdata16, 10, &num_rxpkts);
	for (i=0; i<num_rxpkts; i++)
	{
		DMSG("rx[%u] = 0x%x\n", i, rdata16[i]);
	}

	platform_pl022_cfg->ops->spi_txrx16(&platform_pl022_cfg, data16_long, rdata16, 20, &num_rxpkts);
	for (i=0; i<num_rxpkts; i++)
	{
		DMSG("rx[%u] = 0x%x\n", i, rdata16[i]);
	}

	#if 0
	platform_pl022_cfg->ops->spi_txrx16(&platform_pl022_cfg, data16_100, rdata16, 100, &num_rxpkts);
	for (i=0; i<num_rxpkts; i++)
	{
		DMSG("rx[%u] = 0x%x\n", i, rdata16[i]);
	}
	#endif
	#endif
}

static void spi_test_linksprite(void)
{
	uint8_t tx[3], rx[3] = {0};
	uint32_t num_rxpkts, i, j;
	int ch = 'c';
	paddr_t uart_base = console_base();

	tx[0] = 0x1;
	tx[1] = 0x80;
	tx[2] = 0;

	while (1)
	{
		while (!pl011_have_rx_data(uart_base));

		ch = pl011_getchar(uart_base);
		DMSG("cpu %zu: got 0x%x %c", get_core_pos(), ch, (char)ch);

		switch (ch)
		{
			case 'c':
				for (j=0; j<20; j++)
				{
					DMSG("cycle: %u\n", j);
					platform_pl022_cfg->ops->spi_txrx8(&platform_pl022_cfg, tx, rx, 3, &num_rxpkts);
					for (i=0; i<num_rxpkts; i++)
					{
						DMSG("rx[%u] = 0x%x\n", i,rx[i]);
					}

					//sleep some, ~1-2s
					for (i=0; i<100000000; i++)
					{
					}
				}
				break;
			case 't':
				gpio_set_value(platform_pl022_cfg.cs_gpio_pin, GPIO_LEVEL_LOW);
				break;
			case 'u':
				gpio_set_value(platform_pl022_cfg.cs_gpio_pin, GPIO_LEVEL_HIGH);
				break;
			case 'v':
				io_mask32(gp6bs + (1<<4), 0, (1<<2));
				break;
			case 'w':
				io_mask32(gp6bs + (1<<4), (1<<2), (1<<2));
				break;
			case 'x':
				write8(0, gp6bs + (1<<4));
				break;
			case 'y':
				write8(4, gp6bs + (1<<4));
			default:
				break;
		}

		if (ch == 'q')
			break;
	}
}

void spi_test2(void)
{
	#if 0
	sizeof(bool): 1
	sizeof(uint8_t): 1
	sizeof(uint16_t): 2
	sizeof(uint32_t): 4
	sizeof(uint64_t): 8
	sizeof(vaddr_t): 8
	sizeof(paddr_t): 8
	sizeof(int): 4
	sizeof(unsigned int ): 4
	sizeof(unsigned): 4
	sizeof(long): 8
	sizeof(unsigned long): 8
	sizeof(long long): 8
	sizeof(unsigned long long): 8
	sizeof(float): 4
	#endif
	DMSG("Hello!\n");
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

	peri_init_n_config();
	pl022_start(&platform_pl022_cfg);

	if (platform_pl022_cfg.loopback)
		spi_test_lbm();
	else
		spi_test_linksprite();

	pl022_end(&platform_pl022_cfg);
}

static TEE_Result spi_test(void)
{
	spi_test2();
	return TEE_SUCCESS;
}

service_init(spi_test);
