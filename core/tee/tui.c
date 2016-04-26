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

#include <tee/tui.h>
#include <kernel/mutex.h>
#include <kernel/tee_ta_manager.h>
#include <kernel/tee_time.h>
#include <display.h>
#include <utee_defines.h>

static struct {
	struct mutex mu;
	void *sess;
	void *ctx;
	bool busy;
	TEE_Time timeout_time;
} tui_sess = { .mu = MUTEX_INITIALIZER };

TEE_Result tui_init_session(struct tee_ta_session *sess)
{
	TEE_Result res;
	TEE_Time t;
	const TEE_Time timeout = {
		.seconds = TUI_SESSION_TIMEOUT / TEE_TIME_MILLIS_BASE,
		.millis = TUI_SESSION_TIMEOUT % TEE_TIME_MILLIS_BASE,
	};

	res = tee_time_get_sys_time(&t);
	if (res != TEE_SUCCESS)
		return res;

	mutex_lock(&tui_sess.mu);

	if (tui_sess.sess && tui_sess.sess != sess && !tui_sess.busy &&
	    TEE_TIME_LE(t, tui_sess.timeout_time)) {
		/*
		 * There's a current session and it's either busy of hasn't
		 * timed out yet.
		 */
		res = TEE_ERROR_BUSY;
		goto out;
	}

	tui_sess.busy = true;
	TEE_TIME_ADD(t, timeout, tui_sess.timeout_time);
	tui_sess.sess = sess;
	/*
	 * We may only dereference sess if our thread is
	 * using sess so save the context pointer for later
	 * use here.
	 */
	tui_sess.ctx = sess->ctx;
out:
	mutex_unlock(&tui_sess.mu);
	if (res == TEE_SUCCESS)
		display_init();
	return res;
}


static TEE_Result time_to_session_timeout_unlocked(struct tee_ta_session *sess,
						   const TEE_Time *current_time,
						   uint32_t *millis)
{
	TEE_Result res = TEE_SUCCESS;
	TEE_Time t;
	const TEE_Time *ct = current_time;

	if (tui_sess.sess != sess)
		return TEE_ERROR_BUSY;

	if (!ct) {
		res = tee_time_get_sys_time(&t);
		if (res != TEE_SUCCESS)
			return res;
		ct = &t;
	}

	if (TEE_TIME_LE(tui_sess.timeout_time, *ct))
		return TEE_ERROR_BAD_STATE;

	if (millis) {
		TEE_TIME_SUB(*ct, tui_sess.timeout_time, t);
		*millis = t.seconds * TEE_TIME_MILLIS_BASE + t.millis;

		/*
		 * Check if it wrapped ("can't happen"), "bad state" is the
		 * best we can return then.
		 */
		if ((*millis) / TEE_TIME_MILLIS_BASE < t.seconds)
			res = TEE_ERROR_BAD_STATE;
	}

	return res;
}

TEE_Result tui_get_time_to_session_timeout(struct tee_ta_session *sess,
					   const TEE_Time *current_time,
					   uint32_t *millis)
{
	TEE_Result res;

	mutex_lock(&tui_sess.mu);
	res = time_to_session_timeout_unlocked(sess, current_time, millis);
	mutex_unlock(&tui_sess.mu);
	return res;
}

TEE_Result tui_set_session_busy(struct tee_ta_session *sess)
{
	TEE_Result res;
	TEE_Time t;
	const TEE_Time timeout = {
		.seconds = TUI_SESSION_TIMEOUT / TEE_TIME_MILLIS_BASE,
		.millis = TUI_SESSION_TIMEOUT % TEE_TIME_MILLIS_BASE,
	};

	res = tee_time_get_sys_time(&t);
	if (res != TEE_SUCCESS)
		return res;

	mutex_lock(&tui_sess.mu);
	res = time_to_session_timeout_unlocked(sess, &t, NULL);
	if (res == TEE_SUCCESS) {
		TEE_TIME_ADD(t, timeout, tui_sess.timeout_time);
		tui_sess.busy = true;
	}
	mutex_unlock(&tui_sess.mu);
	return res;
}

TEE_Result tui_clear_session_busy(struct tee_ta_session *sess)
{
	TEE_Result res = TEE_ERROR_BAD_STATE;

	mutex_lock(&tui_sess.mu);
	if (tui_sess.sess == sess) {
		tui_sess.busy = false;
		res = TEE_SUCCESS;
	}
	mutex_unlock(&tui_sess.mu);
	return res;
}

TEE_Result tui_close_session(struct tee_ta_session *sess)
{
	TEE_Result res = TEE_SUCCESS;

	mutex_lock(&tui_sess.mu);
	if (sess != tui_sess.sess) {
		/*
		 * TEE_ERROR_BUSY: If the TUI resources are currently in
		 * use, i.e. a TUI screen is displayed. This error code can
		 * only be returned by a TEE implementation supporting
		 * multi-threading within a TA and will occur when a thread
		 * tries to close a TUI session that is displaying a TUI
		 * screen in another thread.
		 */
		if (sess->ctx == tui_sess.ctx)
			res = TEE_ERROR_BUSY;
		else
			res = TEE_ERROR_BAD_STATE;
	}
	mutex_unlock(&tui_sess.mu);

	if (res != TEE_SUCCESS)
		return res;

	display_final();
	tui_input_disable();
	mutex_lock(&tui_sess.mu);
	tui_sess.sess = NULL;
	tui_sess.ctx = NULL;
	mutex_unlock(&tui_sess.mu);
	return TEE_SUCCESS;;
}
