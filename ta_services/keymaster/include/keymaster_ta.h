/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2018, Linaro Limited */

#ifndef __KEYMASTER_TA_H
#define __KEYMASTER_TA_H

#define TA_KEYMASTER_UUID { 0xd36b30c7, 0x5a5f, 0x472c, \
			    {0xaf, 0x97, 0x7f, 0x38, 0xa2, 0xed, 0xab, 0x7d } }

// Commands
enum optee_keymaster_cmd_id {
	OPTEE_KEYMASTER_CMD_ID_INIT                   = (0x00),
	OPTEE_KEYMASTER_CMD_ID_TERM                   = (0x01),
	OPTEE_KEYMASTER_CMD_ID_GENERATE_KEYPAIR       = (0x02),
	OPTEE_KEYMASTER_CMD_ID_GET_KEYPAIR_PUBLIC     = (0x03),
	OPTEE_KEYMASTER_CMD_ID_IMPORT_KEYPAIR         = (0x04),
	OPTEE_KEYMASTER_CMD_ID_QUERY_KEY_EXISTENCE    = (0x05),
	OPTEE_KEYMASTER_CMD_ID_SIGN_DIGEST            = (0x06),
	OPTEE_KEYMASTER_CMD_ID_VERIFY_SIGNATURE       = (0x07),
	OPTEE_KEYMASTER_CMD_ID_DELETE_KEYPAIR         = (0x08),
	OPTEE_KEYMASTER_CMD_ID_ALLOCATE_TRANSIENT_OBJ = (0x09),
	OPTEE_KEYMASTER_CMD_ID_FREE_TRANSIENT_OBJ     = (0x0a),
	OPTEE_KEYMASTER_CMD_ID_ALLOCATE_OPERATION     = (0x0b),
	OPTEE_KEYMASTER_CMD_ID_FREE_OPERATION         = (0x0c),
	OPTEE_KEYMASTER_CMD_ID_DIGEST_DO_FINAL        = (0x0d),
	OPTEE_KEYMASTER_CMD_ID_SET_OPERATION_KEY      = (0x0e),
	OPTEE_KEYMASTER_CMD_ID_POPULATE_TRANSIENT_OBJ = (0x0f),
	OPTEE_KEYMASTER_CMD_ID_GET_OBJ_BUF_ATTR       = (0x10),
	OPTEE_KEYMASTER_CMD_ID_GET_OBJ_VALUE_ATTR     = (0x11),
	OPTEE_KEYMASTER_CMD_ID_DIGEST_UPDATE          = (0x12),
	OPTEE_KEYMASTER_CMD_ID_ASYMMETRIC_EN_DE_CRYPT = (0x13),
	OPTEE_KEYMASTER_CMD_ID_CIPHER_INIT            = (0x14),
	OPTEE_KEYMASTER_CMD_ID_CIPHER_UPDATE          = (0x15),
	OPTEE_KEYMASTER_CMD_ID_CIPHER_DO_FINAL        = (0x16),
	OPTEE_KEYMASTER_CMD_ID_AE_INIT                = (0x17),
	OPTEE_KEYMASTER_CMD_ID_AE_UPDATE_AAD          = (0x18),
	OPTEE_KEYMASTER_CMD_ID_AE_UPDATE              = (0x19),
	OPTEE_KEYMASTER_CMD_ID_AE_ENCRYPT_FINAL       = (0x1a),
	OPTEE_KEYMASTER_CMD_ID_AE_DECRYPT_FINAL       = (0x1b),
	OPTEE_KEYMASTER_CMD_ID_MAC_INIT               = (0x1c),
	OPTEE_KEYMASTER_CMD_ID_MAC_UPDATE             = (0x1d),
	OPTEE_KEYMASTER_CMD_ID_MAC_DO_FINAL           = (0x1e),
	OPTEE_KEYMASTER_CMD_ID_MAC_DO_FINAL_COMPARE   = (0x1f),
	OPTEE_KEYMASTER_CMD_ID_IMPORT_SYMMETRIC_KEY   = (0x20),
	OPTEE_KEYMASTER_CMD_ID_LOAD_KEY               = (0x21),
	OPTEE_KEYMASTER_CMD_ID_MAC_KEYBLOB_INIT       = (0x22),
	OPTEE_KEYMASTER_CMD_ID_UNKNOWN                = (0x7FFFFFFE),
	OPTEE_KEYMASTER_CMD_ID_MAX                    = (0x7FFFFFFF)
};

enum keymaster_command {
    KEYMASTER_RESP_BIT              = 1,
    KEYMASTER_STOP_BIT              = 2,
    KEYMASTER_REQ_SHIFT             = 2,
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
    KM_GENERATE_KEY                 = (0 << KEYMASTER_REQ_SHIFT),
    KM_BEGIN_OPERATION              = (1 << KEYMASTER_REQ_SHIFT),
    KM_UPDATE_OPERATION             = (2 << KEYMASTER_REQ_SHIFT),
    KM_FINISH_OPERATION             = (3 << KEYMASTER_REQ_SHIFT),
    KM_ABORT_OPERATION              = (4 << KEYMASTER_REQ_SHIFT),
    KM_IMPORT_KEY                   = (5 << KEYMASTER_REQ_SHIFT),
    KM_EXPORT_KEY                   = (6 << KEYMASTER_REQ_SHIFT),
    KM_GET_VERSION                  = (7 << KEYMASTER_REQ_SHIFT),
    /*
     * Add (re-seed) caller-provided entropy to the RNG pool. Keymaster
     * implementations need to securely mix the provided entropy into their
     * pool, which also must contain internally-generated entropy from a
     * hardware random number generator.
     *
     * in	params[0].memref: entropy input data
     */
    KM_ADD_RNG_ENTROPY              = (8 << KEYMASTER_REQ_SHIFT),
    KM_GET_SUPPORTED_ALGORITHMS     = (9 << KEYMASTER_REQ_SHIFT),
    KM_GET_SUPPORTED_BLOCK_MODES    = (10 << KEYMASTER_REQ_SHIFT),
    KM_GET_SUPPORTED_PADDING_MODES  = (11 << KEYMASTER_REQ_SHIFT),
    KM_GET_SUPPORTED_DIGESTS        = (12 << KEYMASTER_REQ_SHIFT),
    KM_GET_SUPPORTED_IMPORT_FORMATS = (13 << KEYMASTER_REQ_SHIFT),
    KM_GET_SUPPORTED_EXPORT_FORMATS = (14 << KEYMASTER_REQ_SHIFT),
    KM_GET_KEY_CHARACTERISTICS      = (15 << KEYMASTER_REQ_SHIFT),
    KM_ATTEST_KEY                   = (16 << KEYMASTER_REQ_SHIFT),
    KM_UPGRADE_KEY                  = (17 << KEYMASTER_REQ_SHIFT),
    KM_CONFIGURE                    = (18 << KEYMASTER_REQ_SHIFT),
};

#endif /*__KEYMASTER_TA_H*/
