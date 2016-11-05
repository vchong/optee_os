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

#include <drivers/pl022_spi.h>
#include <drivers/pl061_gpio.h>
#include <hikey_peripherals.h>
#include <io.h>
#include <kernel/tee_time.h>
#include <stdint.h>
#include <trace.h>
#include <util.h>

#include <drivers/pl011.h>

#define PL022_STAT	0x00C
#define PL022_STAT_BSY	SHIFT_U32(1, 4)

static void spi_cs_callback(enum gpio_level value)
{
	static bool inited;
	static struct pl061_data pd;
	vaddr_t gpio6_base = nsec_periph_base(GPIO6_BASE);
	vaddr_t spi_base = nsec_periph_base(SPI_BASE);

	if (!inited) {
		pl061_init(&pd);
		pl061_register(gpio6_base, 6);
		pl061_set_mode_control(GPIO6_2, PL061_MC_SW);
		pd.chip.ops->set_interrupt(GPIO6_2, GPIO_INTERRUPT_DISABLE);
		pd.chip.ops->set_direction(GPIO6_2, GPIO_DIR_OUT);
		inited = true;
	}

	if (read8(spi_base + PL022_STAT) & PL022_STAT_BSY)
		DMSG("pl022 busy - do NOT set CS!");
	while (read8(spi_base + PL022_STAT) & PL022_STAT_BSY)
		;
	DMSG("pl022 done - set CS!");

	pd.chip.ops->set_value(GPIO6_2, value);
}

static void spi_set_cs_mux(uint32_t val)
{
	uint32_t data;
	vaddr_t pmx0_base = nsec_periph_base(PMX0_BASE);

	if (val == PINMUX_SPI) {
		DMSG("configure gpio6 pin2 as SPI");
		write32(PINMUX_SPI, pmx0_base + PMX0_IOMG106); /* 0xF70101A8 */
	} else {
		DMSG("configure gpio6 pin2 as GPIO");
		write32(PINMUX_GPIO, pmx0_base + PMX0_IOMG106); /* 0xF70101A8 */
	}

	data = read32(pmx0_base + PMX0_IOMG106); /* 0xF70101A8 */
	if (data)
		DMSG("gpio6 pin2 is SPI");
	else
		DMSG("gpio6 pin2 is GPIO");
}

void spi_test_lb(void)
{
	struct pl061_data pd061 __maybe_unused;
	struct pl022_data pd022;
	vaddr_t __maybe_unused gpio6_base = nsec_periph_base(GPIO6_BASE);
	vaddr_t spi_base = nsec_periph_base(SPI_BASE);
	vaddr_t pmx0_base = nsec_periph_base(PMX0_BASE);
	uint8_t __maybe_unused data8[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	uint8_t __maybe_unused data8_long[20] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0xaa, 0xbb, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};
	uint8_t __maybe_unused rdata8[20];
	uint16_t __maybe_unused data16[10] = {0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888, 0x9999, 0xaaaa};
	uint16_t __maybe_unused data16_long[20] = {0xbbbb, 0xcccc, 0xdddd, 0xeeee, 0xffff, 0, 0x1111, 0x2222, 0x3333, 0x4444, 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888, 0x9999, 0xaaaa};
	uint16_t __maybe_unused rdata16[20];

	size_t i, j, nloops1 = 1, cs = 1;

	spi_set_cs_mux(PINMUX_GPIO);

	DMSG("cs = 0x%x", read32(pmx0_base + PMX0_IOMG106));

	DMSG("Enter # loops (: for 10, D for 20)");
	nloops1 = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", nloops1);

	DMSG("Enter 1 no cs cb, 0 for cs cb");
	cs = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", cs);

	if (cs) {
		DMSG("gpio6_base: 0x%" PRIxVA "\n", gpio6_base);
		DMSG("configure GPIO");

		pl061_init(&pd061);
		pl061_register(gpio6_base, 6);
		DMSG("enable software mode control for chip select");
		pl061_set_mode_control(GPIO6_2, PL061_MC_SW);

		pd022.cs_data.gpio_data.chip = &pd061.chip;
		pd022.cs_data.gpio_data.pin_num = GPIO6_2;
		pd022.cs_control = PL022_CS_CTRL_AUTO_GPIO;
	} else {
		DMSG("set CS callback");
		pd022.cs_data.cs_cb = spi_cs_callback;
		pd022.cs_control = PL022_CS_CTRL_CB;
	}

	DMSG("spi_base: 0x%" PRIxVA "\n", spi_base);
	DMSG("configure SPI");
	pd022.base = spi_base;
	pd022.clk_hz = SPI_CLK_HZ;
	pd022.speed_hz = SPI_10_KHZ;
	pd022.mode = SPI_MODE0;
	pd022.data_size_bits = 8;
	pd022.loopback = true; //chg in PR

	pl022_init(&pd022);
	pd022.chip.ops->configure(&pd022.chip);
	pd022.chip.ops->start(&pd022.chip);
	DMSG("cs = 0x%x", read32(pmx0_base + PMX0_IOMG106));
	for (j = 0; j < nloops1; j++) {
		pd022.chip.ops->txrx8(&pd022.chip, data8, rdata8, 10);
		for (i = 0; i < 10; i++)
			DMSG("rx[%zu] = 0x%x", i, rdata8[i]);
		pd022.chip.ops->txrx8(&pd022.chip, data8_long, rdata8, 20);
		for (i = 0; i < 20; i++)
			DMSG("rx[%zu] = 0x%x", i, rdata8[i]);
	}
	pd022.chip.ops->end(&pd022.chip);


	pd022.data_size_bits = 16;
	pd022.chip.ops->configure(&pd022.chip);
	pd022.chip.ops->start(&pd022.chip);
	DMSG("cs = 0x%x", read32(pmx0_base + PMX0_IOMG106));
	for (j = 0; j < nloops1; j++) {
		pd022.chip.ops->txrx16(&pd022.chip, data16, rdata16, 10);
		for (i = 0; i < 10; i++)
			DMSG("rx[%zu] = 0x%x", i, rdata16[i]);
		pd022.chip.ops->txrx16(&pd022.chip, data16_long, rdata16, 20);
		for (i = 0; i < 20; i++)
			DMSG("rx[%zu] = 0x%x", i, rdata16[i]);
	}
	pd022.chip.ops->end(&pd022.chip);
}

void spi_test_with_manual_cs_control(void)
{
	struct pl022_data pd022;
	vaddr_t spi_base = nsec_periph_base(SPI_BASE);
	uint8_t tx[3] = {0x01, 0x80, 0x00};
	uint8_t rx[3] = {0};
	size_t i, j, len = 3, nloops1 = 1;
	enum spi_result res;

	spi_set_cs_mux(PINMUX_GPIO);

	DMSG("Enter # txrx loops (: for 10, D for 20)");
	nloops1 = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", nloops1);

	DMSG("set CS callback");
	pd022.cs_control = PL022_CS_CTRL_MANUAL;

	DMSG("spi_base: 0x%" PRIxVA "\n", spi_base);
	DMSG("configure SPI");
	pd022.base = spi_base;
	pd022.clk_hz = SPI_CLK_HZ;
	pd022.speed_hz = SPI_10_KHZ;
	pd022.mode = SPI_MODE0;
	pd022.data_size_bits = 8;
	pd022.loopback = false; //chg in PR

	pl022_init(&pd022);
	pd022.chip.ops->configure(&pd022.chip);
	pd022.chip.ops->start(&pd022.chip);

	/* tst that this will not work due to no cs ctl */
	for (j = 0; j < nloops1; j++) {
		DMSG("SPI test loop: %zu", j);
		pd022.chip.ops->txrx8(&pd022.chip, tx, rx, len);
		for (i = 0; i < len; i++)
			DMSG("rx[%zu] = 0x%x", i, rx[i]);

		tee_time_busy_wait(20);
	}

	pl011_getchar(console_base());

	/*
	 * Pulse CS only once for the whole transmission.
	 * This is the scheme used by the pl022 driver.
	 */
	spi_cs_callback(GPIO_LEVEL_HIGH);
	tee_time_busy_wait(2);
	spi_cs_callback(GPIO_LEVEL_LOW);
	for (j = 0; j < nloops1; j++) {
		DMSG("SPI test loop: %zu", j);
		res = pd022.chip.ops->txrx8(&pd022.chip, tx, rx, len);
		if (res) {
			EMSG("SPI transceive error %s", spi_result_desc(res));
			break;
		}

		for (i = 0; i < len; i++)
			DMSG("rx[%zu] = 0x%x", i, rx[i]);

		tee_time_busy_wait(20);
	}
	spi_cs_callback(GPIO_LEVEL_HIGH);

	pl011_getchar(console_base());

	/* pulse CS once per transfer */
	spi_cs_callback(GPIO_LEVEL_HIGH);
	tee_time_busy_wait(2);
	for (j = 0; j < nloops1; j++) {
		DMSG("SPI test loop: %zu", j);
		spi_cs_callback(GPIO_LEVEL_LOW);
		res = pd022.chip.ops->txrx8(&pd022.chip, tx, rx, len);
		if (res) {
			EMSG("SPI transceive error %s", spi_result_desc(res));
			break;
		}

		for (i = 0; i < len; i++)
			DMSG("rx[%zu] = 0x%x", i, rx[i]);

		tee_time_busy_wait(20);
		spi_cs_callback(GPIO_LEVEL_HIGH);
	}

	pl011_getchar(console_base());

	/* pulse CS once per word/byte */
	spi_set_cs_mux(PINMUX_SPI);
	tee_time_busy_wait(2);
	for (j = 0; j < nloops1; j++) {
		DMSG("SPI test loop: %zu", j);
		res = pd022.chip.ops->txrx8(&pd022.chip, tx, rx, len);
		if (res) {
			EMSG("SPI transceive error %s", spi_result_desc(res));
			break;
		}

		for (i = 0; i < len; i++)
			DMSG("rx[%zu] = 0x%x", i, rx[i]);

		tee_time_busy_wait(20);
	}

	pd022.chip.ops->end(&pd022.chip);
}

void spi_test3(void)
{
	struct pl061_data pd061 __maybe_unused;
	struct pl022_data pd022;
	vaddr_t __maybe_unused gpio6_base = nsec_periph_base(GPIO6_BASE);
	vaddr_t spi_base = nsec_periph_base(SPI_BASE);
	vaddr_t pmx0_base = nsec_periph_base(PMX0_BASE);
	uint8_t tx[3] = {0x01, 0x80, 0x00};
	uint8_t rx[3] = {0};
	size_t i, j, len = 3;

	size_t nloops1 = 1, nloops2 = 0, nloops3 = 1, nloops4 = 1, nloops5 = 1, cs = 1, lb = 1;

	spi_set_cs_mux(PINMUX_GPIO);

	DMSG("cs = 0x%x", read32(pmx0_base + PMX0_IOMG106));

	DMSG("Enter # tx loops (: for 10, D for 20)");
	nloops1 = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", nloops1);

	DMSG("Enter # rx1 loops (: for 10, D for 20)");
	nloops2 = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", nloops2);

	DMSG("Enter # txrx loops (: for 10, D for 20)");
	nloops3 = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", nloops3);

	DMSG("Enter # rx2 loops (: for 10, D for 20)");
	nloops4 = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", nloops4);

	DMSG("Enter # loops to toggle cs n flush fifo (: for 10, D for 20)");
	nloops5 = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", nloops5);

	DMSG("Enter 1 no cs cb, 0 for cs cb");
	cs = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", cs);

	DMSG("Enter 1 for lb, 0 no lb");
	lb = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", lb);

	if (cs) {
		DMSG("gpio6_base: 0x%" PRIxVA "\n", gpio6_base);
		DMSG("configure GPIO");
		pl061_init(&pd061);
		pl061_register(gpio6_base, 6);
		DMSG("enable software mode control for chip select");
		pl061_set_mode_control(GPIO6_2, PL061_MC_SW);

		pd022.cs_data.gpio_data.chip = &pd061.chip;
		pd022.cs_data.gpio_data.pin_num = GPIO6_2;
		pd022.cs_control = PL022_CS_CTRL_AUTO_GPIO;
	} else {
		DMSG("set CS callback");
		pd022.cs_data.cs_cb = spi_cs_callback;
		pd022.cs_control = PL022_CS_CTRL_CB;
	}

	if (lb) {
		DMSG("loopback");
		pd022.loopback = true;
	} else {
		DMSG("no loopback");
		pd022.loopback = false;
	}

	DMSG("spi_base: 0x%" PRIxVA "\n", spi_base);
	DMSG("configure SPI");
	pd022.base = spi_base;
	pd022.clk_hz = SPI_CLK_HZ;
	pd022.speed_hz = SPI_10_KHZ;
	pd022.mode = SPI_MODE0;
	pd022.data_size_bits = 8;

	pl022_init(&pd022);
	pd022.chip.ops->configure(&pd022.chip);
	pd022.chip.ops->start(&pd022.chip);

	//tx1, rx0, txrx1, rx1
	//tx3, rx0, txrx3, rx3
	//tx1, rx1, txrx1, rx0
	//tx3, rx3, txrx3, rx0

	DMSG("cs = 0x%x", read32(pmx0_base + PMX0_IOMG106));
	for (j = 0; j < nloops1; j++) {
		pd022.chip.ops->txrx8(&pd022.chip, tx, NULL, len);
		tee_time_busy_wait(4);
	}

	DMSG("cs = 0x%x", read32(pmx0_base + PMX0_IOMG106));
	for (j = 0; j < nloops2; j++) {
		pd022.chip.ops->txrx8(&pd022.chip, NULL, rx, len);
		for (i = 0; i < len; i++)
			DMSG("rx[%zu] = 0x%x", i, rx[i]);
		tee_time_busy_wait(4);
	}

	DMSG("cs = 0x%x", read32(pmx0_base + PMX0_IOMG106));
	for (j = 0; j < nloops3; j++) {
		pd022.chip.ops->txrx8(&pd022.chip, tx, rx, len);
		for (i = 0; i < len; i++)
			DMSG("rx[%zu] = 0x%x", i, rx[i]);
		tee_time_busy_wait(4);
	}

	DMSG("cs = 0x%x", read32(pmx0_base + PMX0_IOMG106));
	for (j = 0; j < nloops4; j++) {
		pd022.chip.ops->txrx8(&pd022.chip, NULL, rx, len);
		for (i = 0; i < len; i++)
			DMSG("rx[%zu] = 0x%x", i, rx[i]);
		tee_time_busy_wait(4);
	}

	DMSG("cs = 0x%x", read32(pmx0_base + PMX0_IOMG106));
	for (j = 0; j < nloops5; j++) {
		pd.chip.ops->set_value(GPIO6_2, GPIO_LEVEL_HIGH);
		tee_time_busy_wait(4);
		pd.chip.ops->set_value(GPIO6_2, GPIO_LEVEL_LOW);
		tee_time_busy_wait(4);
		pd.chip.ops->set_value(GPIO6_2, GPIO_LEVEL_HIGH);
		tee_time_busy_wait(4);
		pl022_flush_fifo(&pd022);
	}

	pd022.chip.ops->end(&pd022.chip);
}

void spi_test2(void)
{
	struct pl061_data pd061 __maybe_unused;
	struct pl022_data pd022;
	vaddr_t __maybe_unused gpio6_base = nsec_periph_base(GPIO6_BASE);
	vaddr_t spi_base = nsec_periph_base(SPI_BASE);
	uint8_t tx[4] = {0x01, 0x80, 0x00, 0x42};
	uint8_t tx_string[2] = {"2b"};
	uint8_t tx_long[8] = {"8 bytes"}; //str only 7 chars long so tx_long[7] = 0
	uint8_t rx[4] = {0};
	uint8_t rx_string[2] = {0};
	uint8_t rx_long[8] = {0};
	size_t j, len = 4, len_string = 2, len_long = 8;

	#if 0
	i=0, str[i]=0x32 2
	i=1, str[i]=0x62 b

	i=0, str[i]=0x38 8
	i=1, str[i]=0x20
	i=2, str[i]=0x62 b
	i=3, str[i]=0x79 y
	i=4, str[i]=0x74 t
	i=5, str[i]=0x65 e
	i=6, str[i]=0x73 s
	i=7, str[i]=0x0
	#endif

	size_t cs = 1;

	spi_set_cs_mux(PINMUX_GPIO);

	DMSG("Enter 1 no cs cb, 0 for cs cb");
	cs = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", cs);

	if (cs) {
		DMSG("gpio6_base: 0x%" PRIxVA "\n", gpio6_base);
		DMSG("configure GPIO");
		pl061_init(&pd061);
		pl061_register(gpio6_base, 6);
		DMSG("enable software mode control for chip select");
		pl061_set_mode_control(GPIO6_2, PL061_MC_SW);

		pd022.cs_data.gpio_data.chip = &pd.chip;
		pd022.cs_data.gpio_data.pin_num = GPIO6_2;
		pd022.cs_control = PL022_CS_CTRL_AUTO_GPIO;
	} else {
		DMSG("set CS callback");
		pd022.cs_data.cs_cb = spi_cs_callback;
		pd022.cs_control = PL022_CS_CTRL_CB;
	}

	DMSG("spi_base: 0x%" PRIxVA "\n", spi_base);
	DMSG("configure SPI");
	pd022.base = spi_base;
	pd022.clk_hz = SPI_CLK_HZ;
	pd022.speed_hz = SPI_10_KHZ;
	pd022.mode = SPI_MODE0;
	pd022.data_size_bits = 8;
	pd022.loopback = false; //chg in PR

	pl022_init(&pd022);
	pd022.chip.ops->configure(&pd022.chip);
	pd022.chip.ops->start(&pd022.chip);

	for (j = 0; j < 2; j++) {
		pd022.chip.ops->txrx8(&pd022.chip, tx, rx, len);
		tee_time_busy_wait(2);
	}
	tee_time_busy_wait(2);

	pd022.chip.ops->txrx8(&pd022.chip, tx_string, rx_string, len_string);
	tee_time_busy_wait(4);

	pd022.chip.ops->txrx8(&pd022.chip, tx_long, rx_long, len_long);
	tee_time_busy_wait(4);

	for (j = 0; j < 2; j++) {
		pd022.chip.ops->txrx8(&pd022.chip, tx, rx, len);
		tee_time_busy_wait(2);
	}
	tee_time_busy_wait(2);

	for (j = 0; j < 2; j++) {
		pd022.chip.ops->txrx8(&pd022.chip, tx, rx, len);
		tee_time_busy_wait(2);
	}
	tee_time_busy_wait(4);

	pd022.chip.ops->end(&pd022.chip);
}

void spi_test16(void)
{
	struct pl061_data pd061 __maybe_unused;
	struct pl022_data pd022;
	vaddr_t __maybe_unused gpio6_base = nsec_periph_base(GPIO6_BASE);
	vaddr_t spi_base = nsec_periph_base(SPI_BASE);
	uint16_t tx16[3] = {0x0180, 0x0001, 0x8000};
	uint16_t rx16[3] = {0};
	size_t i, j, len = 3;

	size_t nloops1 = 1, cs = 1;

	spi_set_cs_mux(PINMUX_GPIO);

	DMSG("Enter # txrx loops (: for 10, D for 20)");
	nloops1 = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", nloops1);

	DMSG("Enter 1 no cs cb, 0 for cs cb");
	cs = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", cs);

	if (cs) {
		DMSG("gpio6_base: 0x%" PRIxVA "\n", gpio6_base);
		DMSG("configure GPIO");
		pl061_init(&pd061);
		pl061_register(gpio6_base, 6);
		DMSG("enable software mode control for chip select");
		pl061_set_mode_control(GPIO6_2, PL061_MC_SW);

		pd022.cs_data.gpio_data.chip = &pd061.chip;
		pd022.cs_data.gpio_data.pin_num = GPIO6_2;
		pd022.cs_control = PL022_CS_CTRL_AUTO_GPIO;
	} else {
		DMSG("set CS callback");
		pd022.cs_data.cs_cb = spi_cs_callback;
		pd022.cs_control = PL022_CS_CTRL_CB;
	}

	DMSG("spi_base: 0x%" PRIxVA "\n", spi_base);
	DMSG("configure SPI");
	pd022.base = spi_base;
	pd022.clk_hz = SPI_CLK_HZ;
	pd022.speed_hz = SPI_10_KHZ;
	pd022.mode = SPI_MODE0;
	pd022.data_size_bits = 16;
	pd022.loopback = false; //chg in PR

	pl022_init(&pd022);
	pd022.chip.ops->configure(&pd022.chip);
	pd022.chip.ops->start(&pd022.chip);

	for (j = 0; j < nloops1; j++) {
		DMSG("SPI test loop: %zu", j);
		pd022.chip.ops->txrx16(&pd022.chip, tx16, rx16, len);
		for (i = 0; i < len; i++)
			DMSG("rx16[%zu] = 0x%x", i, rx16[i]);

		tee_time_busy_wait(20);
	}

	pd022.chip.ops->end(&pd022.chip);
}

/*
 * spi_init() MUST be run before calling this function!
 *
 * spi_test runs some loopback tests, so the SPI module will just receive
 * what is transmitted, i.e. 0x01, 0x80, 0x00.
 *
 * In non-loopback mode, the transmitted value will elicit a readback of
 * the measured value from the ADC chip on the Linksprite 96Boards
 * Mezzanine card [1], which can be connected to either a sliding
 * rheostat [2] or photoresistor [3].
 *
 * [1] http://linksprite.com/wiki/index.php5?title=Linker_Mezzanine_card_for_96board
 * [2] http://learn.linksprite.com/96-board/sliding-rheostat
 * [3] http://learn.linksprite.com/96-board/photoresistor
 */
void spi_test(void)
{
	struct pl061_data pd061 __maybe_unused;
	struct pl022_data pd022;
	vaddr_t __maybe_unused gpio6_base = nsec_periph_base(GPIO6_BASE);
	vaddr_t spi_base = nsec_periph_base(SPI_BASE);
	uint8_t tx[3] = {0x01, 0x80, 0x00};
	uint8_t rx[3] = {0};
	size_t i, j, len = 3;

	#if 0
	i=0, str[i]=0x6f o
	i=1, str[i]=0x70 p
	i=2, str[i]=0x74 t
	i=3, str[i]=0x65 e
	i=4, str[i]=0x65 e
	i=5, str[i]=0x73 s
	i=6, str[i]=0x70 p
	i=7, str[i]=0x69 i
	#endif

	uint8_t tx_string[8] = {"opteespi"};
	uint8_t rx_string[8] = {0};
	size_t len_string = 8;

	//int ch;
	size_t nloops1 = 1, cs = 1, str = 1;

	spi_set_cs_mux(PINMUX_GPIO);

	DMSG("Enter # txrx loops (: for 10, D for 20)");
	nloops1 = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", nloops1);

	DMSG("Enter 1 no cs cb, 0 for cs cb");
	cs = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", cs);

	DMSG("Enter 1 send str array, 0 no send");
	str = pl011_getchar(console_base()) - 48;
	DMSG("got %zu", str);

	if (cs) {
		DMSG("gpio6_base: 0x%" PRIxVA "\n", gpio6_base);
		DMSG("configure GPIO");
		pl061_init(&pd061);
		pl061_register(gpio6_base, 6);
		DMSG("enable software mode control for chip select");
		pl061_set_mode_control(GPIO6_2, PL061_MC_SW);

		pd022.cs_data.gpio_data.chip = &pd061.chip;
		pd022.cs_data.gpio_data.pin_num = GPIO6_2;
		pd022.cs_control = PL022_CS_CTRL_AUTO_GPIO;
	} else {
		DMSG("set CS callback");
		pd022.cs_data.cs_cb = spi_cs_callback;
		pd022.cs_control = PL022_CS_CTRL_CB;
	}

	DMSG("spi_base: 0x%" PRIxVA "\n", spi_base);
	DMSG("configure SPI");
	pd022.base = spi_base;
	pd022.clk_hz = SPI_CLK_HZ;
	pd022.speed_hz = SPI_10_KHZ;
	pd022.mode = SPI_MODE0;
	pd022.data_size_bits = 8;
	pd022.loopback = false; //chg in PR

	pl022_init(&pd022);
	pd022.chip.ops->configure(&pd022.chip);
	pd022.chip.ops->start(&pd022.chip);

	for (j = 0; j < nloops1; j++) {
		DMSG("SPI test loop: %zu", j);
		pd022.chip.ops->txrx8(&pd022.chip, tx, rx, len);
		for (i = 0; i < len; i++)
			DMSG("rx[%zu] = 0x%x", i, rx[i]);

		tee_time_busy_wait(20);

		if (str) {
			DMSG("SPI test to send a string");
			pd022.chip.ops->txrx8(&pd022.chip, tx_string,
				rx_string, len_string);
			for (i = 0; i < len_string; i++)
				DMSG("rx[%zu] = 0x%x", i, rx_string[i]);

			for (i = 0; i < 100000000; i++)
				;
		}

		//ch = pl011_getchar(console_base());
		//DMSG("got 0x%x %c", ch, (char)ch);
	}

	pd022.chip.ops->end(&pd022.chip);
}

void spi_manage_test(void)
{
	struct pl061_data pd;
	vaddr_t gpio6_base = nsec_periph_base(GPIO6_BASE);
	int ch, cs_mux;

	pl061_init(&pd);
	pl061_register(gpio6_base, 6);
	pl061_set_mode_control(GPIO6_2, PL061_MC_SW);
	pd.chip.ops->set_interrupt(GPIO6_2,
												GPIO_INTERRUPT_DISABLE);
	pd.chip.ops->set_direction(GPIO6_2, GPIO_DIR_OUT);

	while (1)
	{
		DMSG("a:test, b:test16, c:test2, d:test3, e:manual_cs");
		DMSG("l:test_lb, t:set_cs_mux");
		DMSG("u:cs lo iomask, v:cs hi iomask");
		DMSG("w:cs lo write, x:cs hi write");
		DMSG("y:cs lo set_val, z:cs hi set_val");
		ch = pl011_getchar(console_base());
		DMSG("got 0x%x %c", ch, (char)ch);

		switch (ch)
		{
			case 'a':
				spi_test();
				break;
			case 'b':
				spi_test16();
				break;
			case 'c':
				spi_test2();
				break;
			case 'd':
				spi_test3();
				break;
			case 'e':
				spi_test_with_manual_cs_control();
				break;
			case 'f':
				break;
			case 'g':
				break;
			case 'h':
				break;
			case 'i':
				break;
			case 'j':
				break;
			case 'k':
				break;
			case 'l':
				spi_test_lb();
				break;
			case 't':
				DMSG("1:spi, 0:gpio");
				cs_mux = pl011_getchar(console_base()) - 48;
				DMSG("got %d", cs_mux);
				spi_set_cs_mux(cs_mux);
				break;
			case 'u':
				io_mask32(gpio6_base + (1<<4), 0, (1<<2));
				break;
			case 'v':
				io_mask32(gpio6_base + (1<<4), (1<<2), (1<<2));
				break;
			case 'w':
				write8(0, gpio6_base + (1<<4));
				break;
			case 'x':
				write8(4, gpio6_base + (1<<4));
				break;
			case 'y':
				pd.chip.ops->set_value(GPIO6_2, GPIO_LEVEL_LOW);
				break;
			case 'z':
				pd.chip.ops->set_value(GPIO6_2, GPIO_LEVEL_HIGH);
				break;
			default:
				break;
		}

		if (ch == 'q')
			break;
	}
}
