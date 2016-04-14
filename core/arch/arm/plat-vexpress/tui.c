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

#include <platform_config.h>
#include <types_ext.h>
#include <tee_api_types.h>

#include <tee/tui.h>
#include <drivers/pl050.h>
#include <drivers/ps2mouse.h>
#include <drivers/pl111.h>
#include <drivers/frame_buffer.h>
#include <drivers/tzc400.h>
#include <kernel/tz_proc.h>
#include <utee_defines.h>
#include <initcall.h>
#include <mm/core_memprot.h>
#include <io.h>
#include <kernel/panic.h>
#include <display.h>
#include <trace.h>
#include <keep.h>

static vaddr_t clcd_base;

#ifdef KMI1_BASE

register_phys_mem(MEM_AREA_IO_SEC, KMI1_BASE, PL050_REG_SIZE);

static void kmi1_ps2m_cb(void *data __unused, uint8_t button, int16_t xdelta,
			 int16_t ydelta)
{
	static uint16_t last_x;
	static uint16_t last_y;
	static uint16_t last_button;
	struct frame_buffer *fb = display_get_frame_buffer();
	ssize_t x = last_x + xdelta;
	ssize_t y = last_y + ydelta;

	if (x < 0)
		x = 0;
	if (x >= (ssize_t)fb->width)
		x = fb->width - 1;
	if (y < 0)
		y = 0;
	if (y >= (ssize_t)fb->width)
		y = fb->width - 1;

	if (!(button & PS2MOUSE_BUTTON_LEFT_DOWN) &&
	    (last_button & PS2MOUSE_BUTTON_LEFT_DOWN)) {
		/* Button being pressed down is considered a click */
		tui_async_input(0, x, y);
	}

	pl111_set_cursor_xy(clcd_base, x, y);

	last_x = x;
	last_y = y;
	last_button = button;
}

static struct pl050_data kmi1_data;
static struct ps2mouse_data kmi1_ps2m_data;

static TEE_Result init_ps2mouse(void)
{
	vaddr_t base;

	DMSG("Initializing PS/2 mouse");

	base = (vaddr_t)phys_to_virt(KMI1_BASE, MEM_AREA_IO_SEC);
	if (!base) {
		EMSG("KMI1 not mapped");
		return TEE_ERROR_GENERIC;
	}
	pl050_init(&kmi1_data, base, 2);
	ps2mouse_init(&kmi1_ps2m_data, &kmi1_data.chip, IT_KMI1,
		      kmi1_ps2m_cb, NULL);
	return TEE_SUCCESS;
}
/*
 * The PL050 driver is used from interrupt context from which we can't
 * handle paging yet.
 */
KEEP_PAGER(pl050_init);

driver_init(init_ps2mouse);
#endif /*KMI1_BASE*/

#ifdef CFG_PL111

register_phys_mem(MEM_AREA_IO_SEC, FRAMEBUFFER_BASE, FRAMEBUFFER_SIZE);
register_phys_mem(MEM_AREA_IO_SEC, PL111_BASE, PL111_REG_SIZE);

#if PLATFORM_FLAVOR_IS(fvp)

static void init_framebuffer_protection(void)
{
       /*
        * LCD controller (filter 2 device ?) should be allowed to read from
        * FB. It work in secure mode. So, enable secure read on filter 2.
        */
        tzc_configure_region((1 << 2), 4, FRAMEBUFFER_BASE,
			     FRAMEBUFFER_BASE + (FRAMEBUFFER_SIZE - 1),
                             TZC_REGION_S_RD,
                             0);
	tzc_dump_state();
}

#define V2M_SYS_CFGDATA		0xa0
#define V2M_SYS_CFGCTRL		0xa4
#define V2M_SYS_CFGSTAT		0xa8

#define V2M_SYS_CFGCTRL_START		(1ul << 31)
#define V2M_SYS_CFGCTRL_WRITE		(1ul << 30)
#define V2M_SYS_CFGCTRL_DCC_SHIFT	20
#define V2M_SYS_CFGCTRL_FUNC_SHIFT	20
#define V2M_SYS_CFGCTRL_SITE_SHIFT	16
#define V2M_SYS_CFGCTRL_POS_SHIFT	12
#define V2M_SYS_CFGCTRL_DEV_SHIFT	0

#define V2M_SYS_CFGCTRL_DCC0		0

#define V2M_SYS_CFG_OSC1 \
	((V2M_SYS_CFGCTRL_DCC0 << V2M_SYS_CFGCTRL_DCC_SHIFT) | \
	 (1 << V2M_SYS_CFGCTRL_FUNC_SHIFT) | \
	 (0 << V2M_SYS_CFGCTRL_SITE_SHIFT) | \
	 (0 << V2M_SYS_CFGCTRL_POS_SHIFT) | \
	 (1 << V2M_SYS_CFGCTRL_DEV_SHIFT))
#define V2M_SYS_CFG_MUXFPGA \
	((V2M_SYS_CFGCTRL_DCC0 << V2M_SYS_CFGCTRL_DCC_SHIFT) | \
	 (7 << V2M_SYS_CFGCTRL_FUNC_SHIFT) | \
	 (0 << V2M_SYS_CFGCTRL_SITE_SHIFT) | \
	 (0 << V2M_SYS_CFGCTRL_POS_SHIFT) | \
	 (0 << V2M_SYS_CFGCTRL_DEV_SHIFT))

/* Value for V2M_SYS_CFGDATA when using V2M_SYS_CFG_MUXFPGA */
#define V2M_SYS_CFG_MUXFPGA_MB		0

#define V2M_SYS_CFGSTAT_COMPLETE	(1 << 0)
#define V2M_SYS_CFGSTAT_ERROR		(1 << 1)

register_phys_mem(MEM_AREA_IO_SEC, VE_SYSREG_BASE, 0x1000);

static void init_plat_lcd(void)
{
	vaddr_t v = (vaddr_t)phys_to_virt(VE_SYSREG_BASE, MEM_AREA_IO_SEC);
	uint32_t st;

	/*
	 * Setup CLCD clock
	 * Set ocscillator 1 rate value to 5.4MHz
	 */
	write32(0, v + V2M_SYS_CFGSTAT);
	write32(5400000, v + V2M_SYS_CFGDATA);
	write32(V2M_SYS_CFGCTRL_START | V2M_SYS_CFGCTRL_WRITE |
		V2M_SYS_CFG_OSC1, v + V2M_SYS_CFGCTRL);
	do {
		st = read32(v + V2M_SYS_CFGSTAT);
		if (st & V2M_SYS_CFGSTAT_ERROR)
			panic();
	} while (!(st & V2M_SYS_CFGSTAT_COMPLETE));

	/* Set DVI mutex to Motherboard (MB) */
	write32(0, v + V2M_SYS_CFGSTAT);
	write32(V2M_SYS_CFG_MUXFPGA_MB, v + V2M_SYS_CFGDATA);
	write32(V2M_SYS_CFGCTRL_START | V2M_SYS_CFGCTRL_WRITE |
		V2M_SYS_CFG_MUXFPGA, v + V2M_SYS_CFGCTRL);
	do {
		st = read32(v + V2M_SYS_CFGSTAT);
		if (st & V2M_SYS_CFGSTAT_ERROR)
			panic();
	} while (!(st & V2M_SYS_CFGSTAT_COMPLETE));
}
#else
static void init_framebuffer_protection(void)
{
}
static void init_plat_lcd(void)
{
}
#endif

static struct frame_buffer display_frame_buffer = {
	.width = DISPLAY_WIDTH,
	.height = DISPLAY_HEIGHT,
	.width_dpi = 80,
	.height_dpi = 80,
	.bpp = FB_24BPP,
};

static const struct pl111_videomode plat_lcd_vmode = {
	.hactive = DISPLAY_WIDTH,
	.hback_porch = 152,
	.hfront_porch = 48,
	.hsync_len = 104,
	.vactive = DISPLAY_HEIGHT,
	.vback_porch = 23,
	.vfront_porch = 3,
	.vsync_len = 4,
};

void display_init(void)
{
	/* 24-bit RGB: gray */
	frame_buffer_clear(&display_frame_buffer, 0x00DFDFDF);
	pl111_cursor(clcd_base, true);
}

struct frame_buffer *display_get_frame_buffer(void)
{
	return &display_frame_buffer;
}

void display_final(void)
{
	/* 24-bit RGB: gray */
	frame_buffer_clear(&display_frame_buffer, 0x00DFDFDF);
	pl111_cursor(clcd_base, false);
}

static TEE_Result init_display(void)
{
	struct frame_buffer *fb = &display_frame_buffer;
	size_t w = fb->width;
	size_t h = fb->height;
	size_t fb_size = frame_buffer_get_image_size(fb, w, h);

	DMSG("Initializing LCD");

	TEE_ASSERT(fb_size < FRAMEBUFFER_SIZE);

	clcd_base = (vaddr_t)phys_to_virt(PL111_BASE, MEM_AREA_IO_SEC);
	if (!clcd_base) {
		EMSG("PL111 not mapped");
		return TEE_ERROR_GENERIC;
	}

	fb->base = phys_to_virt(FRAMEBUFFER_BASE, MEM_AREA_IO_SEC);
	if (!fb->base)
		panic();

	init_framebuffer_protection();
	init_plat_lcd();
	pl111_init(clcd_base, FRAMEBUFFER_BASE, &plat_lcd_vmode);
	display_final();
	return TEE_SUCCESS;
}

driver_init(init_display);
#endif /*CFG_PL111*/
