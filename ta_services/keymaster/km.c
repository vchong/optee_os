/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2018, Linaro Limited */

#include <sys/queue.h>
#include <tee_internal_api.h>

#include <mbedtls/x509.h>

#include "km.h"

TEE_Result km_gen_key(struct km_key_param_head __unused *kph_params,
		      uint32_t __unused *error,
		      void __unused *key_blob, size_t __unused *kblen,
		      struct km_key_param_head __unused *kph_chars)
{
	return TEE_ERROR_NOT_IMPLEMENTED;
}

TEE_Result km_add_rng_entropy(const void __unused *buf, size_t __unused blen)
{
	/* Stubbed until the system PTA is available */
	return TEE_ERROR_NOT_IMPLEMENTED;
}
