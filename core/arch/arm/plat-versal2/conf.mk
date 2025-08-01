# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2023-2024, Advanced Micro Devices, Inc. All rights reserved.
#
#

PLATFORM_FLAVOR ?= generic

include core/arch/arm/cpu/cortex-armv8-0.mk

CFG_MMAP_REGIONS ?= 24

# Disable Non-Standard Crypto Algorithms
$(call force,CFG_CRYPTO_SM2_PKE,n)
$(call force,CFG_CRYPTO_SM2_DSA,n)
$(call force,CFG_CRYPTO_SM2_KEP,n)
$(call force,CFG_CRYPTO_SM3,n)
$(call force,CFG_CRYPTO_SM4,n)

# platform does not support paging; explicitly disable CFG_WITH_PAGER
$(call force,CFG_WITH_PAGER,n)

# Platform specific configurations
$(call force,CFG_SECURE_TIME_SOURCE_CNTPCT,y)
$(call force,CFG_WITH_ARM_TRUSTED_FW,y)
$(call force,CFG_TEE_CORE_NB_CORE,8)
$(call force,CFG_ARM_GICV3,y)
$(call force,CFG_PL011,y)
$(call force,CFG_GIC,y)
$(call force,CFG_DT,y)

CFG_CORE_RESERVED_SHM	?= n
CFG_CORE_DYN_SHM	?= y
CFG_WITH_STATS		?= y
CFG_ARM64_core		?= y
CFG_AUTO_MAX_PA_BITS	?= y

# Enable ARM Crypto Extensions(CE)
CFG_CRYPTO_WITH_CE ?= y

# Define the number of cores per cluster used in calculating core position.
# The cluster number is shifted by this value and added to the core ID,
# so its value represents log2(cores/cluster).
# For AMD Versal Gen 2 there are 4 clusters and 2 cores per cluster.
$(call force,CFG_CORE_CLUSTER_SHIFT,1)

# By default optee_os is located at the following location.
# This range to contain optee_os, TEE RAM and TA RAM.
# Default size is 128MB.
CFG_TZDRAM_START   ?= 0x1800000
CFG_TZDRAM_SIZE    ?= 0x8000000

# Maximum size of the Device Tree Blob to accommodate
# device tree with additional nodes.
CFG_DTB_MAX_SIZE ?= 0x200000

# Console selection
# 0 : UART0[pl011, pl011_0] (default)
# 1 : UART1[pl011_1]
CFG_CONSOLE_UART ?= 0

# PS GPIO Controller configuration.
CFG_AMD_PS_GPIO ?= n

ifeq ($(CFG_AMD_PS_GPIO),y)
$(call force,CFG_MAP_EXT_DT_SECURE,y)
$(call force,CFG_DRIVERS_GPIO,y)
endif

CFG_CORE_HEAP_SIZE ?= 262144
