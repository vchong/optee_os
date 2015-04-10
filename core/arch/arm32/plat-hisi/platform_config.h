/*
 * Copyright (c) 2014, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
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

#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

#define PLATFORM_FLAVOR_ID_hikey	0
#define PLATFORM_FLAVOR_IS(flav) \
	(PLATFORM_FLAVOR == PLATFORM_FLAVOR_ID_ ## flav)

/* Make stacks aligned to data cache line length */
#define STACK_ALIGNMENT		64

#ifdef ARM32
#define PLATFORM_LINKER_FORMAT	"elf32-littlearm"
#define PLATFORM_LINKER_ARCH	arm
#endif /*ARM32*/

#ifdef ARM64
#define PLATFORM_LINKER_FORMAT	"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH	aarch64
#endif /*ARM64*/

#if PLATFORM_FLAVOR_IS(hikey)

#define GIC_BASE		0xF6800000

/* PL011 UART0 */
#define UART0_BASE		0xF8015000

#define UART0_CLK_IN_HZ		19200000

/* UART0 interrupt */
#define IT_UART0		68

#define CONSOLE_UART_BASE	UART0_BASE
#define IT_CONSOLE_UART		IT_UART0
#define CONSOLE_UART_CLK_IN_HZ	UART0_CLK_IN_HZ

#else
#error "Unknown platform flavor"
#endif

// TODO: Leave this alone for now till we're sure it's not needed in kern.ld.S
#define HEAP_SIZE		(24 * 1024)

#if PLATFORM_FLAVOR_IS(hikey)
/*
 * Hikey specifics.
 */

#define DRAM0_BASE		0x00000000
#define DRAM0_SIZE		0x3F000000
#define DRAM_SCP_SIZE	0x00000000
//#define NS_IMAGE_OFFSET			(DRAM0_BASE + 0x37000000)  /* 880MB */
#define NS_IMAGE_OFFSET			(DRAM0_BASE + 0x35000000)  /* 848MB */

/*
* op-tee <> arm-tf
* asram <> sram
* esram <> dram
* dram <> dram
* dram <> sram //test atf sram s optee dram //Actual SRAM - stuck at INFO:    opteed_synchronous_sp_entry 4
*/

#ifdef CFG_WITH_PAGER

#ifndef CFG_EMULATED_SRAM
/* Actual SRAM - stuck at INFO:    opteed_synchronous_sp_entry 4 */
#define TZSRAM_BASE		0xF9898000
#define TZSRAM_SIZE		(512 * 1024) //200KiB no diff

#define TZDRAM_SIZE		(0x01000000 - DRAM_SCP_SIZE) //ARM-TF DRAM_SEC_SIZE
#ifndef CFG_USE_NS_DRAM
#define TZDRAM_BASE		0x3F000000 //ARM-TF DRAM_SEC_BASE
#else
#define TZDRAM_BASE		(NS_IMAGE_OFFSET - TZDRAM_SIZE)
#endif

#else /*CFG_EMULATED_SRAM*/
/* Emulated SRAM - stuck at INFO:    opteed_synchronous_sp_entry 4 */
#ifndef CFG_USE_NS_DRAM
#define TZSRAM_BASE		0x3F000000 //ARM-TF DRAM_SEC_BASE
#else
#define TZSRAM_BASE		(NS_IMAGE_OFFSET - TZDRAM_SIZE)
#endif
#define TZSRAM_SIZE		(512 * 1024) //200KiB no diff

#define TZDRAM_BASE		(TZSRAM_BASE + CFG_TEE_RAM_VA_SIZE)
#define TZDRAM_SIZE		(0x01000000 - DRAM_SCP_SIZE - CFG_TEE_RAM_VA_SIZE)
#endif /*CFG_EMULATED_SRAM*/

#else /*CFG_WITH_PAGER*/
/*
 * Last part of DRAM is reserved as secure dram, note that the last 2MiB
 * of DRAM0 is used by SCP dor DDR retraining.
 */
#ifndef CFG_USE_NS_DRAM
#define TZDRAM_BASE		0x3F000000 //ARM-TF DRAM_SEC_BASE
#else
#define TZDRAM_BASE		(NS_IMAGE_OFFSET - TZDRAM_SIZE)
//#define TZDRAM_BASE		0xf9898000 //test atf sram s optee dram /* Actual SRAM - stuck at INFO:    opteed_synchronous_sp_entry 4 */
#endif
#define TZDRAM_SIZE		(0x01000000 - DRAM_SCP_SIZE) //ARM-TF DRAM_SEC_SIZE
//#define TZDRAM_SIZE		(2 * 1024 * 1024) //test atf sram s optee dram /* Actual SRAM - stuck at INFO:    opteed_synchronous_sp_entry 4 */
#endif /*CFG_WITH_PAGER*/

#define CFG_TEE_CORE_NB_CORE	8

#ifndef CFG_USE_NS_DRAM
#define CFG_SHMEM_START		(DRAM0_BASE + DRAM0_SIZE - CFG_SHMEM_SIZE)
#else
#define CFG_SHMEM_START		(TZDRAM_BASE - CFG_SHMEM_SIZE) //BL32_BASE- CFG_SHMEM_SIZE
//#define CFG_SHMEM_START		(TZDRAM_BASE + TZDRAM_SIZE) //test atf sram s optee dram /* Actual SRAM - stuck at INFO:    opteed_synchronous_sp_entry 4 */
#endif
#define CFG_SHMEM_SIZE		0x100000

#define GICC_OFFSET		0x2000
#define GICD_OFFSET		0x1000

#else
#error "Unknown platform flavor"
#endif

#define CFG_TEE_RAM_VA_SIZE	(1024 * 1024)

#ifndef CFG_TEE_LOAD_ADDR
#define CFG_TEE_LOAD_ADDR	CFG_TEE_RAM_START
#endif

#ifdef CFG_WITH_PAGER
/*
 * Have TZSRAM either as real physical or emulated by reserving an area
 * somewhere else.
 *
 * +------------------+
 * | TZSRAM | TEE_RAM |
 * +--------+---------+
 * | TZDRAM | TA_RAM  |
 * +--------+---------+
 */
#define CFG_TEE_RAM_PH_SIZE	TZSRAM_SIZE
#define CFG_TEE_RAM_START	TZSRAM_BASE
#define CFG_TA_RAM_START	ROUNDUP(TZDRAM_BASE, CORE_MMU_DEVICE_SIZE)
#define CFG_TA_RAM_SIZE		ROUNDDOWN(TZDRAM_SIZE, CORE_MMU_DEVICE_SIZE)
#else
/*
 * Assumes that either TZSRAM isn't large enough or TZSRAM doesn't exist,
 * everything is in TZDRAM.
 * +------------------+
 * |        | TEE_RAM |
 * + TZDRAM +---------+
 * |        | TA_RAM  |
 * +--------+---------+
 */
#define CFG_TEE_RAM_PH_SIZE	CFG_TEE_RAM_VA_SIZE
#define CFG_TEE_RAM_START	TZDRAM_BASE
#define CFG_TA_RAM_START	ROUNDUP((TZDRAM_BASE + CFG_TEE_RAM_VA_SIZE), \
					CORE_MMU_DEVICE_SIZE)
#define CFG_TA_RAM_SIZE		ROUNDDOWN((TZDRAM_SIZE - CFG_TEE_RAM_VA_SIZE), \
					  CORE_MMU_DEVICE_SIZE)
#endif

#define DEVICE0_BASE		ROUNDDOWN(CONSOLE_UART_BASE, \
					  CORE_MMU_DEVICE_SIZE)
#define DEVICE0_SIZE		CORE_MMU_DEVICE_SIZE

#define DEVICE1_BASE		ROUNDDOWN(GIC_BASE, CORE_MMU_DEVICE_SIZE)
#define DEVICE1_SIZE		CORE_MMU_DEVICE_SIZE

#define DEVICE2_BASE		ROUNDDOWN(GIC_BASE + GICD_OFFSET, \
					  CORE_MMU_DEVICE_SIZE)
#define DEVICE2_SIZE		CORE_MMU_DEVICE_SIZE

#ifdef CFG_WITH_LPAE
#define MAX_XLAT_TABLES		5
#endif

#ifndef UART_BAUDRATE
#define UART_BAUDRATE		115200
#endif
#ifndef CONSOLE_BAUDRATE
#define CONSOLE_BAUDRATE	UART_BAUDRATE
#endif

#endif /*PLATFORM_CONFIG_H*/
