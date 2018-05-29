/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2018, Linaro Limited */

#include <tee_internal_api.h>

#include "km.h"

bool version_info_set = false;
uint32_t boot_os_version = 0;
uint32_t boot_os_patchlevel = 0;

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

TEE_Result km_add_rng_entropy(const void __unused *buf, size_t __unused blen)
{
	/* Stubbed until the system PTA is available */
	return TEE_ERROR_NOT_IMPLEMENTED;
}
