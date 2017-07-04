/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <io.h>
#include <hi3660.h>
#include <hikey960_private.h>
#include <kernel/panic.h>
#include <kernel/tee_time.h>
#include <mm/core_memprot.h>
#include <stdint.h>
#include <trace.h>
#include <util.h>

#define ADC_ADCIN0				0
#define ADC_ADCIN1				1
#define ADC_ADCIN2				2

#define HKADC_DATA_GRADE0			0
#define HKADC_DATA_GRADE1			100
#define HKADC_DATA_GRADE2			300
#define HKADC_DATA_GRADE3			500
#define HKADC_DATA_GRADE4			700
#define HKADC_DATA_GRADE5			900
#define HKADC_DATA_GRADE6			1100
#define HKADC_DATA_GRADE7			1300
#define HKADC_DATA_GRADE8			1500
#define HKADC_DATA_GRADE9			1700
#define HKADC_DATA_GRADE10			1800

#define BOARDID_VALUE0				0
#define BOARDID_VALUE1				1
#define BOARDID_VALUE2				2
#define BOARDID_VALUE3				3
#define BOARDID_VALUE4				4
#define BOARDID_VALUE5				5
#define BOARDID_VALUE6				6
#define BOARDID_VALUE7				7
#define BOARDID_VALUE8				8
#define BOARDID_VALUE9				9
#define BOARDID_UNKNOWN				0xF

#define BOARDID3_BASE				5

static void init_adc(void)
{
	vaddr_t base = (vaddr_t)phys_to_virt_io(CRG_REG_BASE);

	/* reset hkadc */
	write32(PERRSTEN2_HKADCSSI, base + CRG_PERRSTEN2_OFFSET);
	/* wait a few clock cycles */
	tee_time_busy_wait(1);
	write32(PERRSTEN2_HKADCSSI, base + CRG_PERRSTDIS2_OFFSET);
	tee_time_busy_wait(1);
	/* enable hkadc clock */
	write32(PEREN2_HKADCSSI, base + CRG_PERDIS2_OFFSET);
	tee_time_busy_wait(1);
	write32(PEREN2_HKADCSSI, base + CRG_PEREN2_OFFSET);
	tee_time_busy_wait(1);
}

static TEE_Result get_adc(uint32_t channel, uint32_t *value)
{
	uint32_t data, value1, value0;
	vaddr_t base = (vaddr_t)phys_to_virt_io(HKADC_SSI_REG_BASE);
	
	if (channel > HKADC_CHANNEL_MAX) {
		EMSG("invalid channel:%d\n", channel);
		return TEE_ERROR_BAD_PARAMETERS;
	}
	/* configure the read/write operation for external HKADC */
	write32(HKADC_WR01_VALUE | channel, base + HKADC_WR01_DATA);
	write32(HKADC_WR23_VALUE, base + HKADC_WR23_DATA);
	write32(HKADC_WR45_VALUE, base + HKADC_WR45_DATA);
	/* configure the number of accessing registers */
	write32(HKADC_WR_NUM_VALUE, base + HKADC_WR_NUM);
	/* configure delay of accessing registers */
	write32(HKADC_CHANNEL0_DELAY01_VALUE, base + HKADC_DELAY01);
	write32(HKADC_DELAY23_VALUE, base + HKADC_DELAY23);
	
	/* start HKADC */
	write32(1, base + HKADC_DSP_START);
	do {
		data = read32(HKADC_DSP_START);
	} while (data & 1);
	
	/* convert AD result */
	value1 = read32(HKADC_DSP_RD2_DATA) & 0xffff;
	value0 = read32(HKADC_DSP_RD3_DATA) & 0xffff;
	
	data = ((value1 << 4) & HKADC_VALUE_HIGH) |
	       ((value0 >> 4) & HKADC_VALUE_LOW);
	*value = data;
	return TEE_SUCCESS;
}

static int32_t adcin_data_remap(uint32_t adcin_value)
{
	int32_t	ret;

	if (adcin_value < HKADC_DATA_GRADE0) {
		ret = BOARDID_UNKNOWN;
	} else if (adcin_value < HKADC_DATA_GRADE1) {
		ret = BOARDID_VALUE0;
	} else if (adcin_value < HKADC_DATA_GRADE2) {
		ret = BOARDID_VALUE1;
	} else if (adcin_value < HKADC_DATA_GRADE3) {
		ret = BOARDID_VALUE2;
	} else if (adcin_value < HKADC_DATA_GRADE4) {
		ret = BOARDID_VALUE3;
	} else if (adcin_value < HKADC_DATA_GRADE5) {
		ret = BOARDID_VALUE4;
	} else if (adcin_value < HKADC_DATA_GRADE6) {
		ret = BOARDID_VALUE5;
	} else if (adcin_value < HKADC_DATA_GRADE7) {
		ret = BOARDID_VALUE6;
	} else if (adcin_value < HKADC_DATA_GRADE8) {
		ret = BOARDID_VALUE7;
	} else if (adcin_value < HKADC_DATA_GRADE9) {
		ret = BOARDID_VALUE8;
	} else if (adcin_value < HKADC_DATA_GRADE10) {
		ret = BOARDID_VALUE9;
	} else {
		ret = BOARDID_UNKNOWN;
	}
	return ret;
}

static TEE_Result get_value(uint32_t channel, uint32_t *value)
{
	TEE_Result ret;

	ret = get_adc(channel, value);
	if (ret)
		return ret;

	/* convert ADC value to micro-volt */
	ret = ((*value & HKADC_VALID_VALUE) * HKADC_VREF_1V8) / HKADC_ACCURACY;
	*value = ret;
	return TEE_SUCCESS;
}

TEE_Result hikey960_read_boardid(uint32_t *id)
{
	uint32_t	adcin0, adcin1, adcin2;
	uint32_t	adcin0_remap, adcin1_remap, adcin2_remap;
	TEE_Result ret;

	assert(id != NULL);

	init_adc();

	/* read ADC channel0 data */
	ret = get_value(ADC_ADCIN0, &adcin0);
	if (ret)
		return ret;
	adcin0_remap = adcin_data_remap(adcin0);
	DMSG("[BDID]adcin0:%d adcin0_remap:%d\n", adcin0, adcin0_remap);
	if (adcin0_remap == BOARDID_UNKNOWN) {
		return TEE_ERROR_BAD_PARAMETERS;
	}
	/* read ADC channel1 data */
	ret = get_value(ADC_ADCIN1, &adcin1);
	if (ret)
		return ret;
	adcin1_remap = adcin_data_remap(adcin1);
	IMSG("[BDID]adcin1:%d adcin1_remap:%d\n", adcin1, adcin1_remap);
	if (adcin1_remap == BOARDID_UNKNOWN) {
		return TEE_ERROR_BAD_PARAMETERS;
	}
	/* read ADC channel2 data */
	ret = get_value(ADC_ADCIN2, &adcin2);
	if (ret)
		return ret;
	adcin2_remap = adcin_data_remap(adcin2);
	IMSG("[BDID]adcin2:%d adcin2_remap:%d\n", adcin2, adcin2_remap);
	if (adcin2_remap == BOARDID_UNKNOWN) {
		return TEE_ERROR_BAD_PARAMETERS;
	}
	*id = BOARDID3_BASE * 1000 + (adcin2_remap * 100) +
		(adcin1_remap * 10) + adcin0_remap;
	IMSG("[BDID]boardid: %d\n", *id);
	return TEE_SUCCESS;
}
