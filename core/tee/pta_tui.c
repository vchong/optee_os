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

#include <kernel/static_ta.h>
#include <pta_tui.h>
#include <tee/tui.h>
#include <drivers/frame_buffer.h>
#include <display.h>
#include <assert.h>

static TEE_Result taf_get_screen_info(struct tee_ta_session *sess __unused,
			uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	struct frame_buffer *fb;
	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT,
					  TEE_PARAM_TYPE_VALUE_OUTPUT,
					  TEE_PARAM_TYPE_VALUE_OUTPUT,
					  TEE_PARAM_TYPE_NONE);

	if (exp_pt != param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	fb = display_get_frame_buffer();
	params[0].value.a = fb->width;
	params[0].value.b = fb->height;
	params[1].value.a = fb->width_dpi;
	params[1].value.b = fb->height_dpi;
	COMPILE_TIME_ASSERT(PTA_TUI_SCREEN_24BPP == FB_24BPP);
	params[2].value.a = fb->bpp;
	return TEE_SUCCESS;
}

static TEE_Result taf_init_session(struct tee_ta_session *sess,
			uint32_t param_types,
			TEE_Param params[TEE_NUM_PARAMS] __unused)
{
	TEE_Result res;
	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE);

	if (exp_pt != param_types)
		return TEE_ERROR_BAD_PARAMETERS;
	res = tui_init_session(sess);
	if (res == TEE_SUCCESS)
		tui_clear_session_busy(sess);
	return res;
}

static TEE_Result taf_close_session(struct tee_ta_session *sess,
			uint32_t param_types,
			TEE_Param params[TEE_NUM_PARAMS] __unused)
{
	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE);

	if (exp_pt != param_types)
		return TEE_ERROR_BAD_PARAMETERS;
	return tui_close_session(sess);
}

static TEE_Result taf_init_screen(struct tee_ta_session *sess,
			uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res;
	struct frame_buffer *fb;
	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					  TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE);

	if (exp_pt != param_types)
		return TEE_ERROR_BAD_PARAMETERS;
	res = tui_set_session_busy(sess);
	if (res != TEE_SUCCESS)
		return res;
	fb = display_get_frame_buffer();
	frame_buffer_clear(fb, params[0].value.a);
	tui_clear_session_busy(sess);
	return TEE_SUCCESS;
}

static TEE_Result taf_set_screen_image(struct tee_ta_session *sess,
			uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res;
	struct frame_buffer *fb;
	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					  TEE_PARAM_TYPE_VALUE_INPUT,
					  TEE_PARAM_TYPE_MEMREF_INPUT,
					  TEE_PARAM_TYPE_NONE);

	if (exp_pt != param_types || !params[2].memref.buffer)
		return TEE_ERROR_BAD_PARAMETERS;

	fb = display_get_frame_buffer();
	if (params[2].memref.size !=
	    frame_buffer_get_image_size(fb, params[1].value.a,
					params[1].value.b))
		return TEE_ERROR_BAD_PARAMETERS;

	res = tui_set_session_busy(sess);
	if (res != TEE_SUCCESS)
		return res;

	frame_buffer_set_image(fb, params[0].value.a, params[0].value.b,
			       params[1].value.a, params[1].value.b,
			       params[2].memref.buffer);

	tui_clear_session_busy(sess);
	return TEE_SUCCESS;
}

static TEE_Result taf_display_screen(struct tee_ta_session *sess,
			uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res;
	bool clear_input;
	bool is_key;
	bool is_timedout;
	size_t timeout;
	uint32_t key;
	size_t xpos;
	size_t ypos;
	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					  TEE_PARAM_TYPE_VALUE_OUTPUT,
					  TEE_PARAM_TYPE_VALUE_OUTPUT,
					  TEE_PARAM_TYPE_VALUE_OUTPUT);

	if (exp_pt != param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	res = tui_set_session_busy(sess);
	if (res != TEE_SUCCESS)
		return res;

	clear_input = params[0].value.a;
	timeout = params[0].value.b;

	if (clear_input)
		tui_input_enable();

	res = tui_input_get(sess, timeout, &is_timedout,
			    &is_key, &key, &xpos, &ypos);

	params[1].value.a = is_timedout;
	params[2].value.a = is_key;
	params[2].value.b = key;
	params[3].value.a = xpos;
	params[3].value.b = ypos;
	tui_clear_session_busy(sess);
	return res;
}

typedef TEE_Result (*ta_func)(struct tee_ta_session *sess, uint32_t param_types,
			TEE_Param params[TEE_NUM_PARAMS]);

static const ta_func ta_funcs[] = {
	[PTA_TUI_GET_SCREEN_INFO] = taf_get_screen_info,
	[PTA_TUI_INIT_SESSION] = taf_init_session,
	[PTA_TUI_CLOSE_SESSION] = taf_close_session,
	[PTA_TUI_INIT_SCREEN] = taf_init_screen,
	[PTA_TUI_SET_SCREEN_IMAGE] = taf_set_screen_image,
	[PTA_TUI_DISPLAY_SCREEN] = taf_display_screen,
};

/*
 * Trusted Application Entry Points
 */

static TEE_Result create_ta(void)
{
	return TEE_SUCCESS;
}

static void destroy_ta(void)
{
}

static TEE_Result open_session(uint32_t param_types __unused,
			TEE_Param pParams[TEE_NUM_PARAMS] __unused,
			void **sess_ctx __unused)
{
	TEE_Result res;
	struct tee_ta_session *s;

	/* Check that we're called from a TA */
	res = tee_ta_get_current_session(&s);
	if (res != TEE_SUCCESS)
		return res;
	if (s->clnt_id.login != TEE_LOGIN_TRUSTED_APP)
		return TEE_ERROR_ACCESS_DENIED;

	return TEE_SUCCESS;
}

static void close_session(void *sess_ctx __unused)
{
}

static TEE_Result invoke_command(void *sess_ctx __unused, uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res;
	struct tee_ta_session *s;

	res = tee_ta_get_current_session(&s);
	if (res != TEE_SUCCESS)
		return res;

	if (cmd_id < ARRAY_SIZE(ta_funcs) && ta_funcs[cmd_id])
		return ta_funcs[cmd_id](tee_ta_get_calling_session(),
					param_types, params);

	return TEE_ERROR_NOT_IMPLEMENTED;
}

static_ta_register(.uuid = PTA_TUI_UUID, .name = "tui",
		   .create_entry_point = create_ta,
		   .destroy_entry_point = destroy_ta,
		   .open_session_entry_point = open_session,
		   .close_session_entry_point = close_session,
		   .invoke_command_entry_point = invoke_command);
