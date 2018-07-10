/*
 * Copyright (c) 2017 GlobalLogic
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

#include <crypto/crypto.h>
#include <kernel/pseudo_ta.h>
#include <tee_api_types.h>
#include <tee_api_defines.h>
#include <tomcrypt.h>
#include <trace.h>

#define TA_NAME		"rng_entropy.ta"
#define RNG_ENTROPY_UUID \
		{ 0x57ff3310, 0x0919, 0x4935, \
			{ 0xb9, 0xc8, 0x32, 0xa4, 0x1d, 0x94, 0xb9, 0x5b } }

#define CMD_ADD_RNG_ENTROPY 0
#define KM_ERROR_UNIMPLEMENTED -100

#define MAX_RNG_LENGTH 32

static unsigned int system_pnum;
/*
 * INPUT
 * params[0].memref.buffer - RNG entropy data
 * params[0].memref.size - RNG entopy size
 */
static TEE_Result TA_add_rng_entropy(const uint32_t ptypes,
				 TEE_Param params[TEE_NUM_PARAMS])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(
					TEE_PARAM_TYPE_MEMREF_INPUT,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE);
	uint32_t res = CRYPT_OK;
	uint8_t *entropy = params[0].memref.buffer;
	size_t entropy_l = params[0].memref.size;

	if (ptypes != exp_param_types) {
		EMSG("Wrong parameters\n");
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (MAX_RNG_LENGTH > 0 && entropy_l > MAX_RNG_LENGTH)
		entropy_l = MAX_RNG_LENGTH;

	crypto_rng_add_event(CRYPTO_RNG_SRC_NONSECURE, &system_pnum,
			     entropy, entropy_l);

	return res;
}

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

static TEE_Result open_session(uint32_t ptype __unused,
				TEE_Param params[TEE_NUM_PARAMS] __unused,
				void **ppsess __unused)
{
	return TEE_SUCCESS;
}

static void close_session(void *psess __unused)
{
}

static TEE_Result invoke_command(void *psess __unused,
				 uint32_t cmd, uint32_t ptypes,
				 TEE_Param params[TEE_NUM_PARAMS])
{
	switch (cmd) {
	case CMD_ADD_RNG_ENTROPY:
		return TA_add_rng_entropy(ptypes, params);
	default:
		break;
	}
	return TEE_ERROR_BAD_PARAMETERS;
}

pseudo_ta_register(.uuid = RNG_ENTROPY_UUID, .name = TA_NAME,
		.create_entry_point = create_ta,
		.destroy_entry_point = destroy_ta,
		.open_session_entry_point = open_session,
		.close_session_entry_point = close_session,
		.invoke_command_entry_point = invoke_command);
