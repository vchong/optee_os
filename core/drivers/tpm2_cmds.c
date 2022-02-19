// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2021, Linaro Limited
 */

#include <kernel/delay.h>
#include <stdint.h>
#include <string.h>
#include <tpm2_cmds.h>
#include <trace.h>

static enum tpm2_result tpm2_chk_cmd(uint8_t *out, uint32_t wlen)
{
	uint32_t cnt = 0;
	//uint32_t ord = 0;

	/* change endian */
	cnt = tpm2_convert2be(out + TPM2_CMD_LEN_CNT);
	//ord = tpm2_convert2be(out + TPM2_CMD_LEN_ORD);

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
	enum tpm2_result ret = TPM2_OK;
	struct tpm2_drv *drv = chip->drv;
	uint32_t tmp_rlen = 0;
	uint32_t t_cnt = 0;

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

	do {
		ret = drv->rx(chip, chip->buf, TPM2_BUF_LEN, &tmp_rlen);
		if (!ret && tmp_rlen) {
			if (tmp_rlen > *rlen)
				return TPM2_ERR_SHORT_BUF;
			memcpy(in, chip->buf, tmp_rlen);
			*rlen = tmp_rlen;
			break;
		} else if (ret != TPM2_ERR_RETRY) {
			return ret;
		}

		mdelay(chip->retry_delay);
		t_cnt += chip->retry_delay;
	} while (t_cnt < TPM2_CMD_TIMEOUT_ORD);

	if (t_cnt >= TPM2_CMD_TIMEOUT_ORD) {
		if (drv->end) {
			ret = drv->end(chip);
			if (ret) {
				EMSG("Failed to end session");
				return ret;
			}
		}
		EMSG("Timed out!");
		return TPM2_ERR_TIMEOUT;
	}

	return TPM2_OK;
}

static enum tpm2_result tpm2_run_cmd(struct tpm2_chip *chip, uint8_t *cmd,
				     uint8_t *rsp, uint32_t *p_rsp_len)
{
	enum tpm2_result ret = TPM2_OK;
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

	rsp_code = tpm2_convert2be(tmp_rsp + TPM2_OFFSET_RSP_CODE);

	DMSG("TPM2 rsp code: %" PRIu32, rsp_code);
	DHEXDUMP(tmp_rsp, tmp_rsp_len);

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

	return tpm2_run_cmd(chip, cmd, NULL, NULL);
}

