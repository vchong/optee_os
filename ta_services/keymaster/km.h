/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2018, Linaro Limited */

#include <sys/queue.h>
#include <tee_internal_api.h>

struct km_key_param {
	TAILQ_ENTRY(km_key_param) link;
	uint32_t tag;
	uint32_t size;
	uint8_t data[];
};

TAILQ_HEAD(km_key_param_head, km_key_param);

TEE_Result km_gen_key(struct km_key_param_head *kph_params, uint32_t *error,
		      void *key_blob, size_t *kblen,
		      struct km_key_param_head *kph_chars);
