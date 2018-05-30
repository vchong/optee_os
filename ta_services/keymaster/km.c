/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2018, Linaro Limited */

#include <sys/queue.h>
#include <tee_internal_api.h>

#include <mbedtls/x509.h>

#include "km.h"

bool version_info_set = false;
uint32_t boot_os_version = 0;
uint32_t boot_os_patchlevel = 0;

TEE_Result km_gen_key(struct km_key_param_head __unused *kph_params,
		      uint32_t __unused *error,
		      void __unused *key_blob, uint32_t __unused *kblen,
		      struct km_key_param_head __unused *kph_chars)
{
	return TEE_ERROR_NOT_IMPLEMENTED;
}

TEE_Result km_add_rng_entropy(const void __unused *buf, size_t __unused blen)
{
	/* Stubbed until the system PTA is available */
	return TEE_ERROR_NOT_IMPLEMENTED;
}

TEE_Result km_configure(uint32_t os_version, uint32_t os_patchlevel)
{
	IMSG("os_version = %u", os_version);
	IMSG("os_patchlevel = %u", os_patchlevel);

	if (!version_info_set) {
		//https://android.googlesource.com/trusty/app/keymaster/+/994293cc45700fa58512b312c94da0f46d95403e
		// Note that version info is now set by Configure, rather than by the bootloader.  This is
		// to ensure that system-only updates can be done, to avoid breaking Project Treble.
		boot_os_version = os_version;
		boot_os_patchlevel = os_patchlevel;
		version_info_set = true;
    }

    return TEE_SUCCESS;
}
