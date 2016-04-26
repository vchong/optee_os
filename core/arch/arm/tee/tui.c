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
#include <types_ext.h>
#include <tee_api_types.h>
#include <tee/tui.h>
#include <kernel/tz_proc.h>
#include <kernel/thread.h>
#include <kernel/tee_time.h>
#include <kernel/tee_ta_manager.h>
#include <utee_defines.h>


#define TUI_INPUT_BUFFER_SIZE	16

static struct {
	unsigned lock;
	struct {
		int16_t key;
		uint16_t x;
		uint16_t y;
	} queue[TUI_INPUT_BUFFER_SIZE];
	size_t write_pos;
	size_t read_pos;
} tui_input = {
	.lock = SPINLOCK_UNLOCK,
	.read_pos = TUI_INPUT_BUFFER_SIZE,
};

TEE_Result tui_input_enable(void)
{
	uint32_t exceptions = thread_mask_exceptions(THREAD_EXCP_ALL);

	cpu_spin_lock(&tui_input.lock);

	tui_input.read_pos = tui_input.write_pos;

	cpu_spin_unlock(&tui_input.lock);
	thread_set_exceptions(exceptions);
	return TEE_SUCCESS;
}

void tui_input_disable(void)
{
	uint32_t exceptions = thread_mask_exceptions(THREAD_EXCP_ALL);

	cpu_spin_lock(&tui_input.lock);

	tui_input.read_pos = TUI_INPUT_BUFFER_SIZE;

	cpu_spin_unlock(&tui_input.lock);
	thread_set_exceptions(exceptions);
}

static size_t next_pos(size_t pos)
{
	return (pos + 1) % TUI_INPUT_BUFFER_SIZE;
}

TEE_Result tui_input_get(struct tee_ta_session *sess, size_t timeout,
			 bool *is_timedout, bool *is_key,
			 uint32_t *key, size_t *xpos, size_t *ypos)
{
	TEE_Result res;
	uint32_t exceptions;
	TEE_Time current_time;
	TEE_Time base_time;
	TEE_Time timeout_time = {
		.seconds = timeout / TEE_TIME_MILLIS_BASE,
		.millis = timeout % TEE_TIME_MILLIS_BASE,
	};

	res = tee_time_get_sys_time(&base_time);
	if (res != TEE_SUCCESS)
		return res;
	current_time = base_time;
	TEE_TIME_ADD(timeout_time, base_time, timeout_time);

	exceptions = thread_mask_exceptions(THREAD_EXCP_ALL);
	cpu_spin_lock(&tui_input.lock);

	if (tui_input.read_pos >= TUI_INPUT_BUFFER_SIZE) {
		/* Input cancelled or not enabled */
		res = TEE_ERROR_EXTERNAL_CANCEL;
		goto out;
	}

	while (tui_input.read_pos == tui_input.write_pos &&
	       TEE_TIME_LT(current_time, timeout_time)) {
		cpu_spin_unlock(&tui_input.lock);
		thread_set_exceptions(exceptions);

		/*
		 * Will take eventual interrupt(s) here.
		 */

		res = tee_time_get_sys_time(&current_time);
		if (res != TEE_SUCCESS)
			return res;

		res = tui_get_time_to_session_timeout(sess, &current_time,
						      NULL);
		if (res != TEE_SUCCESS)
			return res;

		if (tee_ta_session_is_cancelled(sess, &current_time))
			return TEE_ERROR_CANCEL;

		exceptions = thread_mask_exceptions(THREAD_EXCP_ALL);
		cpu_spin_lock(&tui_input.lock);
	}

	if (tui_input.read_pos >= TUI_INPUT_BUFFER_SIZE) {
		/* Input cancelled or not enabled */
		res = TEE_ERROR_EXTERNAL_CANCEL;
		goto out;
	}


	if (res == TEE_SUCCESS) {
		if (TEE_TIME_LT(current_time, timeout_time)) {
			*is_timedout = false;
			*is_key = !!tui_input.queue[tui_input.read_pos].key;
			*key = tui_input.queue[tui_input.read_pos].key;
			*xpos = tui_input.queue[tui_input.read_pos].x;
			*ypos = tui_input.queue[tui_input.read_pos].y;
			tui_input.read_pos = next_pos(tui_input.read_pos);
		} else {
			*is_timedout = true;
			*is_key = false;
			*key = 0;
			*xpos = 0;
			*ypos = 0;
		}
	}

out:
	cpu_spin_unlock(&tui_input.lock);
	thread_set_exceptions(exceptions);
	return res;
}

void tui_async_input(uint16_t key, uint16_t x, uint16_t y)
{
	uint32_t exceptions = thread_mask_exceptions(THREAD_EXCP_ALL);

	cpu_spin_lock(&tui_input.lock);

	if (tui_input.read_pos < TUI_INPUT_BUFFER_SIZE &&
	    next_pos(tui_input.write_pos) != tui_input.read_pos) {
		tui_input.queue[tui_input.write_pos].key = key;
		tui_input.queue[tui_input.write_pos].x = x;
		tui_input.queue[tui_input.write_pos].y = y;
		tui_input.write_pos = next_pos(tui_input.write_pos);
	}

	cpu_spin_unlock(&tui_input.lock);
	thread_set_exceptions(exceptions);
}
