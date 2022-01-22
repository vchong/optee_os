// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2021, Linaro Limited
 *
 */

#include <stdint.h>
#include <tpm2_cmds.h>
#include <trace.h>

static enum tpm2_result tpm2_chk_cmd(uint8_t *out, uint32_t wlen)
{
	uint32_t cnt = 0;
	uint32_t ord = 0;

	/* change endian */
	cnt = tpm2_convert2be(out + TPM2_CMD_LEN_CNT);
	ord = tpm2_convert2be(out + TPM2_CMD_LEN_ORD);

	if (!cnt) {
		EMSG("no cmd found");
		return TPM2_ERR_VALUE;
	}
	if (cnt > wlen) {
		EMSG("cmd len mismatch %" PRIu32 " %" PRIu32, cnt, wlen);
		return TPM2_ERR_SHORT_BUF;
	}

	return TPM2_OK;
}

static enum tpm2_result tpm2_txrx(struct tpm2_chip *chip, uint8_t *out,
				  uint32_t wlen, uint8_t *in, uint32_t *rlen)
{
	ulong start, stop;
	int ret2 = 0;


	enum tpm2_result ret = TPM2_OK;
	struct tpm2_drv *drv = chip->drv;

	if (drv->txrx)
		return drv->txrx(chip, out, wlen, in, rlen);

	if (!drv->tx || !drv->rx) {
		EMSG("No driver functions defined");
		return TPM2_ERR_INVALID_ARG;
	}

	ret = tpm2_chk_cmd(out, wlen);
	if (ret)
		return ret;

	ret = drv->tx(chip, out, wlen);
	if (ret)
		return ret;

	start = get_timer(0);
	stop = TPM2_CMD_TIMEOUT_ORD;
	do {
		ret = drv->rx(chip, priv->buf, sizeof(priv->buf));
		if (ret >= 0) {
			if (ret > *recv_size)
				return -ENOSPC;
			memcpy(recvbuf, priv->buf, ret);
			*recv_size = ret;
			ret = 0;
			break;
		} else if (ret != -EAGAIN) {
			return ret;
		}

		mdelay(priv->retry_time_ms);
		if (get_timer(start) > stop) {
			ret = -ETIMEDOUT;
			break;
		}
	} while (ret);

	if (ret) {
		if (drv->end) {
			ret2 = drv->end(chip);
			if (ret2)
				return log_msg_ret("cleanup", ret2);
		}
		return log_msg_ret("xfer", ret);
	}

	return 0;
}

static enum tpm2_result tpm2_run_cmd(struct tpm2_chip *chip, uint8_t *cmd,
				     uint8_t *rsp, uint32_t *p_rsp_len)
{
	enum tpm2_result ret = TPM2_OK;
	int i = 0;
	uint32_t size = 0;
	uint32_t rsp_code = 0;
	uint8_t dum_rsp[TPM2_CMD_RSP_BUF_MAX] = { 0 };
	uint8_t *tmp_rsp = NULL;
	uint32_t tmp_rsp_len = 0;

	/*
	 * callers are allowed NULL rsp and p_rsp_len
	 */
	if (rsp)
		tmp_rsp = rsp;
	else
		tmp_rsp = dum_rsp;

	if (p_rsp_len)
		tmp_rsp_len = *p_rsp_len;
	else
		tmp_rsp_len = TPM2_CMD_RSP_BUF_MAX;

	size = tpm2_convert2be(cmd + TPM2_OFFSET_CMD_SIZE);

	if (size > TPM2_CMD_RSP_BUF_MAX) {
		EMSG("Cmd size (%" PRIu32 ") > buffer size (%" PRIu32 ")",
		     size, TPM2_CMD_RSP_BUF_MAX);
		return TPM2_ERR_SHORT_BUF;
	}

	DMSG("TPM2 cmd size: %" PRIu32, size);
	DHEXDUMP(cmd, size);

	ret = tpm2_txrx(chip, cmd, size, tmp_rsp, &tmp_rsp_len);
	if (ret)
		return ret;

	if (p_rsp_len)
		*p_rsp_len = tmp_rsp_len;

	rsp_code = tpm2_convert2be(rsp + TPM2_OFFSET_RSP_CODE);

	DMSG("TPM2 rsp code: %" PRIu32, rsp_code);
	DHEXDUMP(rsp, tmp_rsp_len);

	return ret;
}

enum tpm2_result tpm2_startup(struct tpm2_chip *chip,
			      enum tpm2_startup_state state)
{
	enum tpm2_result ret = TPM2_OK;
	uint8_t cmd[TPM2_CMD_LEN_STARTUP] = {
		TPM2_CMD_PREFIX_STARTUP,
		0, state
	};

	ret = tpm2_run_cmd(chip, cmd, NULL, NULL);
	/*
	 * TPM2_ERR_INIT = startup done in a previous call
	 */
	if (ret && ret != TPM2_ERR_INIT)
		return ret;

	return TPM2_OK;
}

enum tpm2_result tpm2_selftest(struct tpm2_chip *chip, bool yes)
{
	uint8_t cmd[TPM2_CMD_LEN_SELFTEST] = {
		TPM2_CMD_PREFIX_SELF_TEST,
		yes
	};

	return = tpm2_run_cmd(chip, cmd, NULL, NULL);
}

