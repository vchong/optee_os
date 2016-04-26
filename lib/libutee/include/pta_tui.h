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

#ifndef __PTA_TUI_H
#define __PTA_TUI_H

#define PTA_TUI_UUID	{ 0x9c199eb0, 0x4d2d, 0x41a5, { \
			  0x8c, 0xe9, 0xf2, 0x08, 0x52, 0x15, 0x77, 0xd1 } }

#define PTA_TUI_SCREEN_24BPP	0

/*
 * [out]  value[0].a:	width
 * [out]  value[0].b:	height
 * [out]  value[1].a:	width_dpi
 * [out]  value[1].b:	height_dpi
 * [out]  value[2].a:	bpp
 * Returns TEE_Result
 */
#define PTA_TUI_GET_SCREEN_INFO		1

/* See TEE_TUIInitSession() */
#define PTA_TUI_INIT_SESSION		3

/* See TEE_TUICloseSession() */
#define PTA_TUI_CLOSE_SESSION		4

/*
 * [in] value[0].a:	color
 */
#define PTA_TUI_INIT_SCREEN		5

/*
 * [in] value[0].a:	xpos
 * [in] value[0].b:	ypos
 * [in] value[1].a:	width
 * [in] value[1].b:	height
 * [in] memref[2]:	image
 */
#define PTA_TUI_SET_SCREEN_IMAGE	6

/*
 * See TEE_TUIDisplayScreen()
 * [in]  value[0].a	clear_input	(bool)
 * [in]  value[0].b	time_out	(milliseconds)
 * [out] value[1].a	is_timedout	(bool)
 * [out] value[2].a	is_key		(bool)
 * [out] value[2].b	key
 * [out] value[3].a	xpos
 * [out] value[3].b	ypos
 */
#define PTA_TUI_DISPLAY_SCREEN		7

#endif /*__PTA_TUI_H*/
