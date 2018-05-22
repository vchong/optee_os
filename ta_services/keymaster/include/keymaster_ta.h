/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2018, Linaro Limited */

#ifndef __KEYMASTER_TA_H
#define __KEYMASTER_TA_H

#define TA_KEYMASTER_UUID { 0xd36b30c7, 0x5a5f, 0x472c, \
			    {0xaf, 0x97, 0x7f, 0x38, 0xa2, 0xed, 0xab, 0x7d } }

/*
 * struct keymaster_key_param - holds a key parameter
 * @tag		opaque value when serializing
 * @size	size of data below
 * @data	data of the key parameter @size bytes large
 *
 * When serializing this struct in an array, each element is 32-bit
 * aligned. This means that unless @size is an multiple of 4 there will be
 * some padding before next struct.
 *
 * struct keymaster_key_param {
 *	uint32_t tag;	@tag is an opaque value when serializing
 *	uint32_t size;
 *	uint8_t data[]; @data is @size large
 * };
 */

/*
 * See description of genereateKey at [0] for the different elements.
 * [0] Link: https://source.android.com/reference/hidl/android/hardware/keymaster/3.0/types#keyparameter
 *
 * in	params[0].memref  = serialized array of struct key_param
 * out	params[1].value.a = error
 * out	params[2].memref  = keyBlob
 * out	params[3].memref  = serialized array of struct key_param representing
 *			    the teeEnforced array of keyCharacteristics.
 */
#define KEYMASTER_CMD_GENERATE_KEY	0

#endif /*__KEYMASTER_TA_H*/
