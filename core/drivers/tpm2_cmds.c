/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 *
 */

#include <stdint.h>
#include <tpm2_cmds.h>
#include <trace.h>

static enum tpm2_result tpm2_run_cmd(struct tpm2_chip *chip, uint8_t *cmd,
				     uint8_t *rsp, size_t *rsp_len)
{
	enum tpm2_result ret = TPM2_OK;
	int i = 0;
	size_t resp_len = 0;
	uint32_t size = 0;
	uint8_t resp[TPM2_CMD_RSP_BUF_MAX];

	if (rsp) {
		resp_len = *rsp_len;
	} else {
		rsp = resp;
		resp_len = sizeof(resp);
	}

	size = tpm_command_size(command);

	if (size > COMMAND_BUFFER_SIZE)
		return log_msg_ret("size", -E2BIG);

	DMSG("TPM request [size:%d]: ", size);
	for (i = 0; i < size; i++)
		DMSG("%02x ", ((u8 *)command)[i]);
	DMSG("\n");

	err = tpm_xfer(dev, command, size, response, &response_length);

	if (err < 0)
		return err;

	if (size_ptr)
		*size_ptr = response_length;

	ret = tpm_return_code(response);

	DMSG("TPM response [ret:%d]: ", ret);
	for (i = 0; i < response_length; i++)
		DMSG("%02x ", ((u8 *)response)[i]);
	DMSG("\n");

	return ret;
}

enum tpm2_result tpm2_startup(struct tpm2_chip *chip,
			      enum tpm2_startup_state state)
{
	enum tpm2_result ret = TPM2_OK;
	uint8_t cmd[TPM2_CMD_RSP_BUF_MAX] = {
		TPM2_CMD_PREFIX_STARTUP,
		0, state
	};

	ret = tpm2_run_cmd(chip, cmd, NULL, NULL);
	/*
	 * TPM2_RSP_INITIALIZE is ok.
	 * It just means startup was done.
	 */
	if (ret && ret != TPM2_RSP_INITIALIZE)
		return ret;

	return TPM2_OK;
}

