/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2018, Linaro Limited */

#include <keymaster_ta.h>
#include <sys/queue.h>
#include <tee_internal_api.h>
#include <util.h>

#include "km.h"

static void empty_key_param_head(struct km_key_param_head *kph)
{
	while (true) {
		struct km_key_param *kp = TAILQ_FIRST(kph);

		if (!kp)
			return;
		TAILQ_REMOVE(kph, kp, link);
		TEE_Free(kp);
	}
}

static TEE_Result copy_in_data(void *dst, const void *src, size_t slen,
			       size_t soffs, size_t size)
{
	size_t o;

	if (ADD_OVERFLOW(soffs, size, &o))
		return TEE_ERROR_BAD_PARAMETERS;
	if (o > slen)
		return TEE_ERROR_BAD_PARAMETERS;

	TEE_MemMove(dst, (const uint8_t *)src + soffs, size);

	return TEE_SUCCESS;
}

static TEE_Result new_key_param(uint32_t tag, uint32_t size,
				struct km_key_param **kp)
{
	size_t sz;

	if (ADD_OVERFLOW(size, sizeof(struct km_key_param), &sz))
		return TEE_ERROR_BAD_PARAMETERS;

	*kp = TEE_Malloc(sz, TEE_MALLOC_FILL_ZERO);
	if (!*kp)
		return TEE_ERROR_OUT_OF_MEMORY;

	(*kp)->tag = tag;
	(*kp)->size = size;
	return TEE_SUCCESS;
}

static TEE_Result deserialize_key_param(const void *data, size_t len,
					struct km_key_param_head *kph)
{
	TEE_Result res;
	size_t offs;

	for (offs = 0; offs < len; ) {
		uint32_t tag;
		uint32_t size;
		struct km_key_param *kp;

		res = copy_in_data(&tag, data, len, offs, sizeof(tag));
		if (res)
			goto err;
		offs += sizeof(tag);
		res = copy_in_data(&size, data, len, offs, sizeof(size));
		if (res)
			goto err;
		offs += sizeof(size);

		res = new_key_param(tag, size, &kp);
		if (res)
			goto err;

		TAILQ_INSERT_TAIL(kph, kp, link);
		res = copy_in_data(kp->data, data, len, offs, kp->size);
		if (res)
			goto err;

		offs += kp->size;
	}

	return TEE_SUCCESS;

err:
	empty_key_param_head(kph);
	return res;
}

static void copy_out_data(void *dst, size_t dlen, size_t *doffs,
			  const void *src, size_t slen)
{
	size_t next_doffs = 0;

	if (ADD_OVERFLOW(*doffs, slen, &next_doffs))
		TEE_Panic(0); /* "Can't happen" */

	if (next_doffs <= dlen)
		TEE_MemMove((uint8_t *)dst + *doffs, src, slen);

	*doffs = next_doffs;
}

static TEE_Result serialize_key_param(struct km_key_param_head *kph,
				      void *data, size_t *size)
{
	TEE_Result res;
	size_t offs = 0;
	struct km_key_param *kp;

	TAILQ_FOREACH(kp, kph, link) {
		copy_out_data(data, *size, &offs, &kp->tag, sizeof(kp->tag));
		copy_out_data(data, *size, &offs, &kp->size, sizeof(kp->size));
		copy_out_data(data, *size, &offs, kp->data, kp->size);
	}

	if (offs > *size)
		res = TEE_ERROR_SHORT_BUFFER;
	else
		res = TEE_SUCCESS;

	*size = offs;
	return res;
}

static TEE_Result generate_key(uint32_t pt, TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_VALUE_OUTPUT,
						TEE_PARAM_TYPE_MEMREF_OUTPUT,
						TEE_PARAM_TYPE_MEMREF_OUTPUT);
	TEE_Result res;
	struct km_key_param_head kph_arg;
	struct km_key_param_head kph_ret;

	if (pt != exp_pt)
		return TEE_ERROR_BAD_PARAMETERS;

	TAILQ_INIT(&kph_arg);
	TAILQ_INIT(&kph_ret);

	res = deserialize_key_param(params[0].memref.buffer,
				    params[0].memref.size, &kph_arg);
	if (res)
		goto out;

	res = km_gen_key(&kph_arg, &params[1].value.a, params[2].memref.buffer,
			 &params[2].memref.size, &kph_ret);
	if (res)
		goto out;

	empty_key_param_head(&kph_arg);
	res = serialize_key_param(&kph_ret, params[3].memref.buffer,
				  &params[3].memref.size);
out:
	empty_key_param_head(&kph_ret);
	empty_key_param_head(&kph_arg);
	return res;
}

static TEE_Result add_rng_entropy(uint32_t pt, TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE);

	if (pt != exp_pt)
		return TEE_ERROR_BAD_PARAMETERS;

	return km_add_rng_entropy(params[0].memref.buffer,
				  params[0].memref.size);
}

TEE_Result TA_CreateEntryPoint(void)
{
	return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t __unused param_types,
				    TEE_Param __unused params[4],
				    void __unused **session)
{
	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void __unused *sess)
{
}

TEE_Result TA_InvokeCommandEntryPoint(void __unused *sess, uint32_t cmd,
				      uint32_t ptypes,
				      TEE_Param params[TEE_NUM_PARAMS])
{
	switch (cmd) {
	case KEYMASTER_CMD_GENERATE_KEY:
		return generate_key(ptypes, params);
	case KEYMASTER_CMD_ADD_RNG_ENTROPY:
		return add_rng_entropy(ptypes, params);
	default:
		EMSG("Command ID 0x%x is not supported", cmd);
		return TEE_ERROR_NOT_SUPPORTED;
	}
}
