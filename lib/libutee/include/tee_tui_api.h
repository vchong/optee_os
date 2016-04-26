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

#ifndef __TEE_TUI_API_H
#define __TEE_TUI_API_H

#include <tee_tui_api_types.h>

TEE_Result TEE_TUICheckTextFormat(const char *text, uint32_t *width,
			uint32_t *height, uint32_t *lastIndex);
TEE_Result TEE_TUIGetScreenInfo(TEE_TUIScreenOrientation screenOrientation,
			uint32_t nbEntryFields, TEE_TUIScreenInfo *screenInfo);
TEE_Result TEE_TUIInitSession(void);
TEE_Result TEE_TUICloseSession(void);
TEE_Result TEE_TUIDisplayScreen(
			const TEE_TUIScreenConfiguration *screenConfiguration,
			bool closeTUISession, TEE_TUIEntryField *entryFields,
			uint32_t entryFieldCount,
			TEE_TUIButtonType *selectedButton);


#endif /*__TEE_TUI_API_H*/


