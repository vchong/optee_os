/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2018, Linaro Limited */

#ifndef __KEYMASTER_TA_H
#define __KEYMASTER_TA_H

#define TA_KEYMASTER_UUID { 0xd36b30c7, 0x5a5f, 0x472c, \
			    {0xaf, 0x97, 0x7f, 0x38, 0xa2, 0xed, 0xab, 0x7d } }

/*
 * Add (re-seed) caller-provided entropy to the RNG pool. Keymaster
 * implementations need to securely mix the provided entropy into their
 * pool, which also must contain internally-generated entropy from a
 * hardware random number generator.
 *
 * in	params[0].memref: entropy input data
 */
#define KEYMASTER_CMD_ADD_RNG_ENTROPY	0

/*
 * Configure keymaster with KM_TAG_OS_VERSION and
 * KM_TAG_OS_PATCHLEVEL. Until keymaster is configured, all other
 * functions return TEE_ERROR_NOT_CONFIGURED. Values are only accepted
 * once. Subsequent calls return TEE_SUCCESS, but do nothing.
 *
 * in	params[0].value.a: KM_TAG_OS_VERSION
 * in	parmas[0].value.b: KM_TAG_OS_PATCHLEVEL
 */
#define KEYMASTER_CMD_CONFIGURE		1

/*
 * AOSP Keymaster specific error codes
 */
#define TEE_ERROR_NOT_CONFIGURED          0x80000000

#endif /*__KEYMASTER_TA_H*/
