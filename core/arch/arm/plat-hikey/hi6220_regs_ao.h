/*
 * Copyright (c) 2016, Linaro Ltd and Contributors. All rights reserved.
 * Copyright (c) 2016, Hisilicon Ltd and Contributors. All rights reserved.
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

#ifndef __HI6220_AO_H__
#define __HI6220_AO_H__

#define AO_CTRL_BASE				0xF7800000

#define AO_SC_SYS_CTRL0				(0x000)
#define AO_SC_SYS_CTRL1				(0x004)
#define AO_SC_SYS_CTRL2				(0x008)
#define AO_SC_SYS_STAT0				(0x010)
#define AO_SC_SYS_STAT1				(0x014)
#define AO_SC_MCU_IMCTRL			(0x018)
#define AO_SC_MCU_IMSTAT			(0x01C)
#define AO_SC_SECONDRY_INT_EN0			(0x044)
#define AO_SC_SECONDRY_INT_STATR0		(0x048)
#define AO_SC_SECONDRY_INT_STATM0		(0x04C)
#define AO_SC_MCU_WKUP_INT_EN6			(0x054)
#define AO_SC_MCU_WKUP_INT_STATR6		(0x058)
#define AO_SC_MCU_WKUP_INT_STATM6		(0x05C)
#define AO_SC_MCU_WKUP_INT_EN5			(0x064)
#define AO_SC_MCU_WKUP_INT_STATR5		(0x068)
#define AO_SC_MCU_WKUP_INT_STATM5		(0x06C)
#define AO_SC_MCU_WKUP_INT_EN4			(0x094)
#define AO_SC_MCU_WKUP_INT_STATR4		(0x098)
#define AO_SC_MCU_WKUP_INT_STATM4		(0x09C)
#define AO_SC_MCU_WKUP_INT_EN0			(0x0A8)
#define AO_SC_MCU_WKUP_INT_STATR0		(0x0AC)
#define AO_SC_MCU_WKUP_INT_STATM0		(0x0B0)
#define AO_SC_MCU_WKUP_INT_EN1			(0x0B4)
#define AO_SC_MCU_WKUP_INT_STATR1		(0x0B8)
#define AO_SC_MCU_WKUP_INT_STATM1		(0x0BC)
#define AO_SC_INT_STATR				(0x0C4)
#define AO_SC_INT_STATM				(0x0C8)
#define AO_SC_INT_CLEAR				(0x0CC)
#define AO_SC_INT_EN_SET			(0x0D0)
#define AO_SC_INT_EN_DIS			(0x0D4)
#define AO_SC_INT_EN_STAT			(0x0D8)
#define AO_SC_INT_STATR1			(0x0E4)
#define AO_SC_INT_STATM1			(0x0E8)
#define AO_SC_INT_CLEAR1			(0x0EC)
#define AO_SC_INT_EN_SET1			(0x0F0)
#define AO_SC_INT_EN_DIS1			(0x0F4)
#define AO_SC_INT_EN_STAT1			(0x0F8)
#define AO_SC_TIMER_EN0				(0x1D0)
#define AO_SC_TIMER_EN1				(0x1D4)
#define AO_SC_TIMER_EN4				(0x1F0)
#define AO_SC_TIMER_EN5				(0x1F4)
#define AO_SC_MCU_SUBSYS_CTRL0			(0x400)
#define AO_SC_MCU_SUBSYS_CTRL1			(0x404)
#define AO_SC_MCU_SUBSYS_CTRL2			(0x408)
#define AO_SC_MCU_SUBSYS_CTRL3			(0x40C)
#define AO_SC_MCU_SUBSYS_CTRL4			(0x410)
#define AO_SC_MCU_SUBSYS_CTRL5			(0x414)
#define AO_SC_MCU_SUBSYS_CTRL6			(0x418)
#define AO_SC_MCU_SUBSYS_CTRL7			(0x41C)
#define AO_SC_MCU_SUBSYS_STAT0			(0x440)
#define AO_SC_MCU_SUBSYS_STAT1			(0x444)
#define AO_SC_MCU_SUBSYS_STAT2			(0x448)
#define AO_SC_MCU_SUBSYS_STAT3			(0x44C)
#define AO_SC_MCU_SUBSYS_STAT4			(0x450)
#define AO_SC_MCU_SUBSYS_STAT5			(0x454)
#define AO_SC_MCU_SUBSYS_STAT6			(0x458)
#define AO_SC_MCU_SUBSYS_STAT7			(0x45C)
#define AO_SC_PERIPH_CLKEN4			(0x630)
#define AO_SC_PERIPH_CLKDIS4			(0x634)
#define AO_SC_PERIPH_CLKSTAT4			(0x638)
#define AO_SC_PERIPH_CLKEN5			(0x63C)
#define AO_SC_PERIPH_CLKDIS5			(0x640)
#define AO_SC_PERIPH_CLKSTAT5			(0x644)
#define AO_SC_PERIPH_RSTEN4			(0x6F0)
#define AO_SC_PERIPH_RSTDIS4			(0x6F4)
#define AO_SC_PERIPH_RSTSTAT4			(0x6F8)
#define AO_SC_PERIPH_RSTEN5			(0x6FC)
#define AO_SC_PERIPH_RSTDIS5			(0x700)
#define AO_SC_PERIPH_RSTSTAT5			(0x704)
#define AO_SC_PW_CLKEN0				(0x800)
#define AO_SC_PW_CLKDIS0			(0x804)
#define AO_SC_PW_CLK_STAT0			(0x808)
#define AO_SC_PW_RSTEN0				(0x810)
#define AO_SC_PW_RSTDIS0			(0x814)
#define AO_SC_PW_RST_STAT0			(0x818)
#define AO_SC_PW_ISOEN0				(0x820)
#define AO_SC_PW_ISODIS0			(0x824)
#define AO_SC_PW_ISO_STAT0			(0x828)
#define AO_SC_PW_MTCMOS_EN0			(0x830)
#define AO_SC_PW_MTCMOS_DIS0			(0x834)
#define AO_SC_PW_MTCMOS_STAT0			(0x838)
#define AO_SC_PW_MTCMOS_ACK_STAT0		(0x83C)
#define AO_SC_PW_MTCMOS_TIMEOUT_STAT0		(0x840)
#define AO_SC_PW_STAT0				(0x850)
#define AO_SC_PW_STAT1				(0x854)
#define AO_SC_SYSTEST_STAT			(0x880)
#define AO_SC_SYSTEST_SLICER_CNT0		(0x890)
#define AO_SC_SYSTEST_SLICER_CNT1		(0x894)
#define AO_SC_PW_CTRL1				(0x8C8)
#define AO_SC_PW_CTRL				(0x8CC)
#define AO_SC_MCPU_VOTEEN			(0x8D0)
#define AO_SC_MCPU_VOTEDIS			(0x8D4)
#define AO_SC_MCPU_VOTESTAT			(0x8D8)
#define AO_SC_MCPU_VOTE_MSK0			(0x8E0)
#define AO_SC_MCPU_VOTE_MSK1			(0x8E4)
#define AO_SC_MCPU_VOTESTAT0_MSK		(0x8E8)
#define AO_SC_MCPU_VOTESTAT1_MSK		(0x8EC)
#define AO_SC_PERI_VOTEEN			(0x8F0)
#define AO_SC_PERI_VOTEDIS			(0x8F4)
#define AO_SC_PERI_VOTESTAT			(0x8F8)
#define AO_SC_PERI_VOTE_MSK0			(0x900)
#define AO_SC_PERI_VOTE_MSK1			(0x904)
#define AO_SC_PERI_VOTESTAT0_MSK		(0x908)
#define AO_SC_PERI_VOTESTAT1_MSK		(0x90C)
#define AO_SC_ACPU_VOTEEN			(0x910)
#define AO_SC_ACPU_VOTEDIS			(0x914)
#define AO_SC_ACPU_VOTESTAT			(0x918)
#define AO_SC_ACPU_VOTE_MSK0			(0x920)
#define AO_SC_ACPU_VOTE_MSK1			(0x924)
#define AO_SC_ACPU_VOTESTAT0_MSK		(0x928)
#define AO_SC_ACPU_VOTESTAT1_MSK		(0x92C)
#define AO_SC_MCU_VOTEEN			(0x930)
#define AO_SC_MCU_VOTEDIS			(0x934)
#define AO_SC_MCU_VOTESTAT			(0x938)
#define AO_SC_MCU_VOTE_MSK0			(0x940)
#define AO_SC_MCU_VOTE_MSK1			(0x944)
#define AO_SC_MCU_VOTESTAT0_MSK			(0x948)
#define AO_SC_MCU_VOTESTAT1_MSK			(0x94C)
#define AO_SC_MCU_VOTE1EN			(0x960)
#define AO_SC_MCU_VOTE1DIS			(0x964)
#define AO_SC_MCU_VOTE1STAT			(0x968)
#define AO_SC_MCU_VOTE1_MSK0			(0x970)
#define AO_SC_MCU_VOTE1_MSK1			(0x974)
#define AO_SC_MCU_VOTE1STAT0_MSK		(0x978)
#define AO_SC_MCU_VOTE1STAT1_MSK		(0x97C)
#define AO_SC_MCU_VOTE2EN			(0x980)
#define AO_SC_MCU_VOTE2DIS			(0x984)
#define AO_SC_MCU_VOTE2STAT			(0x988)
#define AO_SC_MCU_VOTE2_MSK0			(0x990)
#define AO_SC_MCU_VOTE2_MSK1			(0x994)
#define AO_SC_MCU_VOTE2STAT0_MSK		(0x998)
#define AO_SC_MCU_VOTE2STAT1_MSK		(0x99C)
#define AO_SC_VOTE_CTRL				(0x9A0)
#define AO_SC_VOTE_STAT				(0x9A4)
#define AO_SC_ECONUM				(0xF00)
#define AO_SCCHIPID				(0xF10)
#define AO_SCSOCID				(0xF1C)
#define AO_SC_SOC_FPGA_RTL_DEF			(0xFE0)
#define AO_SC_SOC_FPGA_PR_DEF			(0xFE4)
#define AO_SC_SOC_FPGA_RES_DEF0			(0xFE8)
#define AO_SC_SOC_FPGA_RES_DEF1			(0xFEC)
#define AO_SC_XTAL_CTRL0			(0x102)
#define AO_SC_XTAL_CTRL1			(0x102)
#define AO_SC_XTAL_CTRL3			(0x103)
#define AO_SC_XTAL_CTRL5			(0x103)
#define AO_SC_XTAL_STAT0			(0x106)
#define AO_SC_XTAL_STAT1			(0x107)
#define AO_SC_EFUSE_CHIPID0			(0x108)
#define AO_SC_EFUSE_CHIPID1			(0x108)
#define AO_SC_EFUSE_SYS_CTRL			(0x108)
#define AO_SC_DEBUG_CTRL1			(0x128)
#define AO_SC_DBG_STAT				(0x12B)
#define AO_SC_ARM_DBG_KEY0			(0x12B)
#define AO_SC_RESERVED31			(0x13A)
#define AO_SC_RESERVED32			(0x13A)
#define AO_SC_RESERVED33			(0x13A)
#define AO_SC_RESERVED34			(0x13A)
#define AO_SC_RESERVED35			(0x13B)
#define AO_SC_RESERVED36			(0x13B)
#define AO_SC_RESERVED37			(0x13B)
#define AO_SC_RESERVED38			(0x13B)
#define AO_SC_ALWAYSON_SYS_CTRL0		(0x148)
#define AO_SC_ALWAYSON_SYS_CTRL1		(0x148)
#define AO_SC_ALWAYSON_SYS_CTRL2		(0x148)
#define AO_SC_ALWAYSON_SYS_CTRL3		(0x148)
#define AO_SC_ALWAYSON_SYS_CTRL10		(0x14A)
#define AO_SC_ALWAYSON_SYS_CTRL11		(0x14A)
#define AO_SC_ALWAYSON_SYS_STAT0		(0x14C)
#define AO_SC_ALWAYSON_SYS_STAT1		(0x14C)
#define AO_SC_ALWAYSON_SYS_STAT2		(0x14C)
#define AO_SC_ALWAYSON_SYS_STAT3		(0x14C)
#define AO_SC_PWUP_TIME0			(0x188)
#define AO_SC_PWUP_TIME1			(0x188)
#define AO_SC_PWUP_TIME2			(0x188)
#define AO_SC_PWUP_TIME3			(0x188)
#define AO_SC_PWUP_TIME4			(0x189)
#define AO_SC_PWUP_TIME5			(0x189)
#define AO_SC_PWUP_TIME6			(0x189)
#define AO_SC_PWUP_TIME7			(0x189)
#define AO_SC_SECURITY_CTRL1			(0x1C0)
#define AO_SC_SYSTEST_SLICER_CNT0		(0x890)
#define AO_SC_SYSTEST_SLICER_CNT1		(0x894)

#define AO_SC_SYS_CTRL0_MODE_NORMAL				0x004
#define AO_SC_SYS_CTRL0_MODE_MASK				0x007

#define AO_SC_SYS_CTRL1_AARM_WD_RST_CFG				(1 << 0)
#define AO_SC_SYS_CTRL1_REMAP_SRAM_AARM				(1 << 1)
#define AO_SC_SYS_CTRL1_EFUSEC_REMAP				(1 << 2)
#define AO_SC_SYS_CTRL1_EXT_PLL_SEL				(1 << 3)
#define AO_SC_SYS_CTRL1_MCU_WDG0_RSTMCU_CFG			(1 << 4)
#define AO_SC_SYS_CTRL1_USIM0_HPD_DE_BOUNCE_CFG			(1 << 6)
#define AO_SC_SYS_CTRL1_USIM0_HPD_OE_CFG			(1 << 7)
#define AO_SC_SYS_CTRL1_USIM1_HPD_DE_BOUNCE_CFG			(1 << 8)
#define AO_SC_SYS_CTRL1_USIM1_HPD_OE_CFG			(1 << 9)
#define AO_SC_SYS_CTRL1_BUS_DFS_FORE_HD_CFG			(1 << 10)
#define AO_SC_SYS_CTRL1_BUS_DFS_FORE_HD_CFG1			(1 << 11)
#define AO_SC_SYS_CTRL1_USIM0_HPD_OE_SFT			(1 << 12)
#define AO_SC_SYS_CTRL1_USIM1_HPD_OE_SFT			(1 << 13)
#define AO_SC_SYS_CTRL1_MCU_CLKEN_HARDCFG			(1 << 15)
#define AO_SC_SYS_CTRL1_AARM_WD_RST_CFG_MSK			(1 << 16)
#define AO_SC_SYS_CTRL1_REMAP_SRAM_AARM_MSK			(1 << 17)
#define AO_SC_SYS_CTRL1_EFUSEC_REMAP_MSK			(1 << 18)
#define AO_SC_SYS_CTRL1_EXT_PLL_SEL_MSK				(1 << 19)
#define AO_SC_SYS_CTRL1_MCU_WDG0_RSTMCU_CFG_MSK			(1 << 20)
#define AO_SC_SYS_CTRL1_USIM0_HPD_DE_BOUNCE_CFG_MSK		(1 << 22)
#define AO_SC_SYS_CTRL1_USIM0_HPD_OE_CFG_MSK			(1 << 23)
#define AO_SC_SYS_CTRL1_USIM1_HPD_DE_BOUNCE_CFG_MSK		(1 << 24)
#define AO_SC_SYS_CTRL1_USIM1_HPD_OE_CFG_MSK			(1 << 25)
#define AO_SC_SYS_CTRL1_BUS_DFS_FORE_HD_CFG_MSK			(1 << 26)
#define AO_SC_SYS_CTRL1_BUS_DFS_FORE_HD_CFG1_MSK		(1 << 27)
#define AO_SC_SYS_CTRL1_USIM0_HPD_OE_SFT_MSK			(1 << 28)
#define AO_SC_SYS_CTRL1_USIM1_HPD_OE_SFT_MSK			(1 << 29)
#define AO_SC_SYS_CTRL1_MCU_CLKEN_HARDCFG_MSK			(1 << 31)

#define AO_SC_SYS_CTRL2_MCU_SFT_RST_STAT_CLEAR			(1 << 26)
#define AO_SC_SYS_CTRL2_MCU_WDG0_RST_STAT_CLEAR			(1 << 27)
#define AO_SC_SYS_CTRL2_TSENSOR_RST_STAT_CLEAR			(1 << 28)
#define AO_SC_SYS_CTRL2_ACPU_WDG_RST_STAT_CLEAR			(1 << 29)
#define AO_SC_SYS_CTRL2_MCU_WDG1_RST_STAT_CLEAR			(1 << 30)
#define AO_SC_SYS_CTRL2_GLB_SRST_STAT_CLEAR			(1 << 31)

#define AO_SC_SYS_STAT0_MCU_RST_STAT				(1 << 25)
#define AO_SC_SYS_STAT0_MCU_SOFTRST_STAT			(1 << 26)
#define AO_SC_SYS_STAT0_MCU_WDGRST_STAT				(1 << 27)
#define AO_SC_SYS_STAT0_TSENSOR_HARDRST_STAT			(1 << 28)
#define AO_SC_SYS_STAT0_ACPU_WD_GLB_RST_STAT			(1 << 29)
#define AO_SC_SYS_STAT0_CM3_WDG1_RST_STAT			(1 << 30)
#define AO_SC_SYS_STAT0_GLB_SRST_STAT				(1 << 31)

#define AO_SC_SYS_STAT1_MODE_STATUS				(1 << 0)
#define AO_SC_SYS_STAT1_BOOT_SEL_LOCK				(1 << 16)
#define AO_SC_SYS_STAT1_FUNC_MODE_LOCK				(1 << 17)
#define AO_SC_SYS_STAT1_BOOT_MODE_LOCK				(1 << 19)
#define AO_SC_SYS_STAT1_FUN_JTAG_MODE_OUT			(1 << 20)
#define AO_SC_SYS_STAT1_SECURITY_BOOT_FLG			(1 << 27)
#define AO_SC_SYS_STAT1_EFUSE_NANDBOOT_MSK			(1 << 28)
#define AO_SC_SYS_STAT1_EFUSE_NAND_BITWIDE			(1 << 29)

#define AO_SC_PERIPH_RSTDIS4_RESET_MCU_ECTR_N			(1 << 0)
#define AO_SC_PERIPH_RSTDIS4_RESET_MCU_SYS_N			(1 << 1)
#define AO_SC_PERIPH_RSTDIS4_RESET_MCU_POR_N			(1 << 2)
#define AO_SC_PERIPH_RSTDIS4_RESET_MCU_DAP_N			(1 << 3)
#define AO_SC_PERIPH_RSTDIS4_PRESET_CM3_TIMER0_N		(1 << 4)
#define AO_SC_PERIPH_RSTDIS4_PRESET_CM3_TIMER1_N		(1 << 5)
#define AO_SC_PERIPH_RSTDIS4_PRESET_CM3_WDT0_N			(1 << 6)
#define AO_SC_PERIPH_RSTDIS4_PRESET_CM3_WDT1_N			(1 << 7)
#define AO_SC_PERIPH_RSTDIS4_HRESET_IPC_S_N			(1 << 8)
#define AO_SC_PERIPH_RSTDIS4_HRESET_IPC_NS_N			(1 << 9)
#define AO_SC_PERIPH_RSTDIS4_PRESET_EFUSEC_N			(1 << 10)
#define AO_SC_PERIPH_RSTDIS4_PRESET_WDT0_N			(1 << 12)
#define AO_SC_PERIPH_RSTDIS4_PRESET_WDT1_N			(1 << 13)
#define AO_SC_PERIPH_RSTDIS4_PRESET_WDT2_N			(1 << 14)
#define AO_SC_PERIPH_RSTDIS4_PRESET_TIMER0_N			(1 << 15)
#define AO_SC_PERIPH_RSTDIS4_PRESET_TIMER1_N			(1 << 16)
#define AO_SC_PERIPH_RSTDIS4_PRESET_TIMER2_N			(1 << 17)
#define AO_SC_PERIPH_RSTDIS4_PRESET_TIMER3_N			(1 << 18)
#define AO_SC_PERIPH_RSTDIS4_PRESET_TIMER4_N			(1 << 19)
#define AO_SC_PERIPH_RSTDIS4_PRESET_TIMER5_N			(1 << 20)
#define AO_SC_PERIPH_RSTDIS4_PRESET_TIMER6_N			(1 << 21)
#define AO_SC_PERIPH_RSTDIS4_PRESET_TIMER7_N			(1 << 22)
#define AO_SC_PERIPH_RSTDIS4_PRESET_TIMER8_N			(1 << 23)
#define AO_SC_PERIPH_RSTDIS4_PRESET_UART0_N			(1 << 24)
#define AO_SC_PERIPH_RSTDIS4_RESET_RTC0_N			(1 << 25)
#define AO_SC_PERIPH_RSTDIS4_RESET_RTC1_N			(1 << 26)
#define AO_SC_PERIPH_RSTDIS4_PRESET_PMUSSI_N			(1 << 27)
#define AO_SC_PERIPH_RSTDIS4_RESET_JTAG_AUTH_N			(1 << 28)
#define AO_SC_PERIPH_RSTDIS4_RESET_CS_DAPB_ON_N			(1 << 29)
#define AO_SC_PERIPH_RSTDIS4_MDM_SUBSYS_GLB			(1 << 30)

#define AO_SC_PERIPH_CLKEN4_HCLK_MCU				(1 << 0)
#define AO_SC_PERIPH_CLKEN4_CLK_MCU_DAP				(1 << 3)
#define AO_SC_PERIPH_CLKEN4_PCLK_CM3_TIMER0			(1 << 4)
#define AO_SC_PERIPH_CLKEN4_PCLK_CM3_TIMER1			(1 << 5)
#define AO_SC_PERIPH_CLKEN4_PCLK_CM3_WDT0			(1 << 6)
#define AO_SC_PERIPH_CLKEN4_PCLK_CM3_WDT1			(1 << 7)
#define AO_SC_PERIPH_CLKEN4_HCLK_IPC_S				(1 << 8)
#define AO_SC_PERIPH_CLKEN4_HCLK_IPC_NS				(1 << 9)
#define AO_SC_PERIPH_CLKEN4_PCLK_EFUSEC				(1 << 10)
#define AO_SC_PERIPH_CLKEN4_PCLK_TZPC				(1 << 11)
#define AO_SC_PERIPH_CLKEN4_PCLK_WDT0				(1 << 12)
#define AO_SC_PERIPH_CLKEN4_PCLK_WDT1				(1 << 13)
#define AO_SC_PERIPH_CLKEN4_PCLK_WDT2				(1 << 14)
#define AO_SC_PERIPH_CLKEN4_PCLK_TIMER0				(1 << 15)
#define AO_SC_PERIPH_CLKEN4_PCLK_TIMER1				(1 << 16)
#define AO_SC_PERIPH_CLKEN4_PCLK_TIMER2				(1 << 17)
#define AO_SC_PERIPH_CLKEN4_PCLK_TIMER3				(1 << 18)
#define AO_SC_PERIPH_CLKEN4_PCLK_TIMER4				(1 << 19)
#define AO_SC_PERIPH_CLKEN4_PCLK_TIMER5				(1 << 20)
#define AO_SC_PERIPH_CLKEN4_PCLK_TIMER6				(1 << 21)
#define AO_SC_PERIPH_CLKEN4_PCLK_TIMER7				(1 << 22)
#define AO_SC_PERIPH_CLKEN4_PCLK_TIMER8				(1 << 23)
#define AO_SC_PERIPH_CLKEN4_CLK_UART0				(1 << 24)
#define AO_SC_PERIPH_CLKEN4_CLK_RTC0				(1 << 25)
#define AO_SC_PERIPH_CLKEN4_CLK_RTC1				(1 << 26)
#define AO_SC_PERIPH_CLKEN4_PCLK_PMUSSI				(1 << 27)
#define AO_SC_PERIPH_CLKEN4_CLK_JTAG_AUTH			(1 << 28)
#define AO_SC_PERIPH_CLKEN4_CLK_CS_DAPB_ON			(1 << 29)
#define AO_SC_PERIPH_CLKEN4_CLK_PDM				(1 << 30)
#define AO_SC_PERIPH_CLKEN4_CLK_SSI_PAD				(1 << 31)

#define AO_SC_PERIPH_CLKEN5_PCLK_PMUSSI_CCPU			(1 << 0)
#define AO_SC_PERIPH_CLKEN5_PCLK_EFUSEC_CCPU			(1 << 1)
#define AO_SC_PERIPH_CLKEN5_HCLK_IPC_CCPU			(1 << 2)
#define AO_SC_PERIPH_CLKEN5_HCLK_IPC_NS_CCPU			(1 << 3)
#define AO_SC_PERIPH_CLKEN5_PCLK_PMUSSI_MCU			(1 << 16)
#define AO_SC_PERIPH_CLKEN5_PCLK_EFUSEC_MCU			(1 << 17)
#define AO_SC_PERIPH_CLKEN5_HCLK_IPC_MCU			(1 << 18)
#define AO_SC_PERIPH_CLKEN5_HCLK_IPC_NS_MCU			(1 << 19)

#define AO_SC_MCU_SUBSYS_CTRL3_RCLK_3				0x003
#define AO_SC_MCU_SUBSYS_CTRL3_RCLK_MASK			0x007
#define AO_SC_MCU_SUBSYS_CTRL3_CSSYS_CTRL_PROT			(1 << 3)
#define AO_SC_MCU_SUBSYS_CTRL3_TCXO_AFC_OEN_CRG			(1 << 4)
#define AO_SC_MCU_SUBSYS_CTRL3_AOB_IO_SEL18_USIM1		(1 << 8)
#define AO_SC_MCU_SUBSYS_CTRL3_AOB_IO_SEL18_USIM0		(1 << 9)
#define AO_SC_MCU_SUBSYS_CTRL3_AOB_IO_SEL18_SD			(1 << 10)
#define AO_SC_MCU_SUBSYS_CTRL3_MCU_SUBSYS_CTRL3_RESERVED	(1 << 11)

#define PCLK_TIMER1						(1 << 16)
#define PCLK_TIMER0						(1 << 15)

#endif /* __HI6220_AO_H__ */
