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

#ifndef __TEE__TUI_H
#define __TEE__TUI_H

#include <tee_tui_api_types.h>
#include <tee_api_types.h>

struct tee_ta_session;

#ifdef CFG_WITH_TUI

/* TUI session timeout in milliseconds */
#define TUI_SESSION_TIMEOUT	10000

#define TUI_PROP_SECURITY_INDICATOR	0

TEE_Result tui_init_session(struct tee_ta_session *sess);

TEE_Result tui_set_session_busy(struct tee_ta_session *sess);

TEE_Result tui_clear_session_busy(struct tee_ta_session *sess);

TEE_Result tui_close_session(struct tee_ta_session *sess);


TEE_Result tui_get_time_to_session_timeout(struct tee_ta_session *sess,
					   const TEE_Time *current_time,
					   uint32_t *millis);

TEE_Result tui_input_enable(void);
void tui_input_disable(void);
void tui_async_input(uint16_t key, uint16_t x, uint16_t y);
TEE_Result tui_input_get(struct tee_ta_session *sess, size_t timeout,
			  bool *is_timedout, bool *is_key,
			  uint32_t *key, size_t *xpos, size_t *ypos);

#else
static inline void tui_async_input(uint16_t key __unused, uint16_t x __unused,
				   uint16_t y __unused)
{
}
static inline TEE_Result tui_close_session(struct tee_ta_session *sess __unused)
{
	return TEE_SUCCESS;
}
#endif

#endif /*__TEE__TUI_H*/
