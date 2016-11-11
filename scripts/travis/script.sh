#!/bin/bash

echo "Enter script.sh"

set -ev

cd $HOME/build/vchong/optee_os
export PATH=${PWD}/gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf/bin:${PATH}
export PATH=${PWD}/gcc-linaro-5.3-2016.02-x86_64_aarch64-linux-gnu/bin:${PATH}
export MYHOME=$PWD
export DST_KERNEL=$PWD/linux
export DL_DIR=$HOME/downloads
export make="make -j3 -s"
export PATH=${HOME}/bc-1.06/bc:$PATH
export PATH=$HOME/inst/bin:$PATH
export PATH=$HOME/bin:$PATH
export PATH=$HOME/git-2.9.3/:$DST_KERNEL/scripts/:$PATH
source ${HOME}/optee_repo/optee_os/scripts/checkpatch_inc.sh

# Run checkpatch.pl on:
# - the tip of the branch if we're not in a pull request
# - each commit in the development branch that's not in the target branch otherwise
if [ "$TRAVIS_PULL_REQUEST" == "false" ]
then
	checkpatch HEAD
else
	for c in $(git rev-list HEAD^1..HEAD^2)
		do checkpatch $c || failed=1
	done
	[ -z "$failed" ]
fi

# If we have a pull request with more than 1 commit, also check the squashed commits
# Useful to check if fix-up commits do indeed solve previous checkpatch errors
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
	if [ "$(git rev-list --count HEAD^1..HEAD^2)" -gt 1 ]
	then
		checkdiff $(git rev-parse HEAD^1) $(git rev-parse HEAD^2)
	fi
fi

# b2260
$make PLATFORM=stm-b2260
$make PLATFORM=stm-b2260 CFG_TEE_CORE_LOG_LEVEL=4 DEBUG=1
$make PLATFORM=stm-b2260 CFG_TEE_CORE_LOG_LEVEL=0 CFG_TEE_TA_LOG_LEVEL=0 DEBUG=0

# Cannes
$make PLATFORM=stm-cannes
$make PLATFORM=stm-cannes CFG_TEE_CORE_LOG_LEVEL=4 DEBUG=1
$make PLATFORM=stm-cannes CFG_TEE_CORE_LOG_LEVEL=0 CFG_TEE_TA_LOG_LEVEL=0 DEBUG=0

# FVP
$make PLATFORM=vexpress-fvp CFG_ARM32_core=y
$make PLATFORM=vexpress-fvp CFG_TEE_CORE_LOG_LEVEL=4 DEBUG=1 CFG_TZC400=y
$make PLATFORM=vexpress-fvp CFG_TEE_CORE_LOG_LEVEL=0 CFG_TEE_TA_LOG_LEVEL=0 DEBUG=0 CFG_TZC400=y
$make PLATFORM=vexpress-fvp CFG_ARM64_core=y
$make PLATFORM=vexpress-fvp CFG_ARM64_core=y CFG_TEE_CORE_LOG_LEVEL=4 DEBUG=1 CFG_TZC400=y
$make PLATFORM=vexpress-fvp CFG_ARM64_core=y CFG_TEE_CORE_LOG_LEVEL=0 CFG_TEE_TA_LOG_LEVEL=0 DEBUG=0 CFG_TZC400=y

# Juno
$make PLATFORM=vexpress-juno
$make PLATFORM=vexpress-juno CFG_TEE_CORE_LOG_LEVEL=4 DEBUG=1
$make PLATFORM=vexpress-juno CFG_TEE_CORE_LOG_LEVEL=0 CFG_TEE_TA_LOG_LEVEL=0 DEBUG=0
$make PLATFORM=vexpress-juno CFG_ARM64_core=y
$make PLATFORM=vexpress-juno CFG_ARM64_core=y CFG_TEE_CORE_LOG_LEVEL=4 DEBUG=1
$make PLATFORM=vexpress-juno CFG_ARM64_core=y CFG_TEE_CORE_LOG_LEVEL=0 CFG_TEE_TA_LOG_LEVEL=0 DEBUG=0

# QEMU-virt (PLATFORM=vexpress-qemu_virt)
$make
$make CFG_TEE_CORE_LOG_LEVEL=4 DEBUG=1
$make CFG_TEE_CORE_LOG_LEVEL=3 DEBUG=1
$make CFG_TEE_CORE_LOG_LEVEL=2 DEBUG=1
$make CFG_TEE_CORE_LOG_LEVEL=1 CFG_TEE_CORE_DEBUG=y DEBUG=1
$make CFG_TEE_CORE_LOG_LEVEL=1 CFG_TEE_CORE_DEBUG=n DEBUG=0
$make CFG_TEE_CORE_LOG_LEVEL=0 CFG_TEE_CORE_DEBUG=y DEBUG=1
$make CFG_TEE_CORE_LOG_LEVEL=0 CFG_TEE_CORE_DEBUG=n DEBUG=0
$make CFG_TEE_CORE_LOG_LEVEL=0 CFG_TEE_CORE_DEBUG=n CFG_TEE_TA_LOG_LEVEL=0 DEBUG=0
$make CFG_TEE_CORE_MALLOC_DEBUG=y
$make CFG_CORE_SANITIZE_UNDEFINED=y
$make CFG_CORE_SANITIZE_KADDRESS=y
$make CFG_CRYPTO=n
$make CFG_CRYPTO_{AES,DES}=n
$make CFG_CRYPTO_{DSA,RSA,DH}=n
$make CFG_CRYPTO_{DSA,RSA,DH,ECC}=n
$make CFG_CRYPTO_{H,C,CBC_}MAC=n
$make CFG_CRYPTO_{G,C}CM=n
$make CFG_CRYPTO_{MD5,SHA{1,224,256,384,512}}=n
$make CFG_CRYPTO=n CFG_CRYPTO_ECC=y
$make CFG_WITH_PAGER=y
$make CFG_WITH_PAGER=y CFG_TEE_CORE_DEBUG=y
$make CFG_WITH_PAGER=y CFG_WITH_LPAE=y
$make CFG_WITH_LPAE=y
$make CFG_WITH_STATS=y
$make CFG_RPMB_FS=y
$make CFG_RPMB_FS=y CFG_RPMB_TESTKEY=y
$make CFG_REE_FS=n CFG_RPMB_FS=y
$make CFG_WITH_USER_TA=n CFG_CRYPTO=n CFG_SE_API=n CFG_PCSC_PASSTHRU_READER_DRV=n
$make CFG_SMALL_PAGE_USER_TA=n
$make CFG_SQL_FS=y
$make CFG_WITH_PAGER=y CFG_WITH_LPAE=y CFG_RPMB_FS=y CFG_SQL_FS=y CFG_DT=y CFG_PS2MOUSE=y CFG_PL050=y CFG_PL111=y CFG_TEE_CORE_LOG_LEVEL=1 CFG_TEE_CORE_DEBUG=y DEBUG=1
$make CFG_WITH_PAGER=y CFG_WITH_LPAE=y CFG_RPMB_FS=y CFG_SQL_FS=y CFG_DT=y CFG_PS2MOUSE=y CFG_PL050=y CFG_PL111=y CFG_TEE_CORE_LOG_LEVEL=0 CFG_TEE_CORE_DEBUG=n DEBUG=0
$make CFG_BUILT_IN_ARGS=y CFG_PAGEABLE_ADDR=0 CFG_NS_ENTRY_ADDR=0 CFG_DT_ADDR=0 CFG_DT=y

# QEMU-ARMv8A
$make PLATFORM=vexpress-qemu_armv8a CFG_ARM64_core=y
$make PLATFORM=vexpress-qemu_armv8a CFG_ARM64_core=y CFG_RPMB_FS=y CFG_SQL_FS=y

# SUNXI(Allwinner A80)
$make PLATFORM=sunxi CFG_TEE_CORE_LOG_LEVEL=4 DEBUG=1
$make PLATFORM=sunxi CFG_TEE_CORE_LOG_LEVEL=0 CFG_TEE_TA_LOG_LEVEL=0 DEBUG=0

# HiKey board (HiSilicon Kirin 620)
$make PLATFORM=hikey
$make PLATFORM=hikey CFG_ARM64_core=y
$make PLATFORM=hikey CFG_ARM64_core=y CFG_TEE_TA_LOG_LEVEL=4 DEBUG=1

# Mediatek mt8173 EVB
$make PLATFORM=mediatek-mt8173 CFG_ARM64_core=y

# i.MX6UL 14X14 EVK
$make PLATFORM=imx-mx6ulevk

# i.MX6Quad SABRE
$make PLATFORM=imx-mx6qsabrelite
$make PLATFORM=imx-mx6qsabresd

# Texas Instruments dra7xx
$make PLATFORM=ti-dra7xx

# Spreadtrum sc9860
$make PLATFORM=sprd-sc9860
$make PLATFORM=sprd-sc9860 CFG_ARM64_core=y

# FSL ls1021a
$make PLATFORM=ls-ls1021atwr
$make PLATFORM=ls-ls1021aqds

# Xilinx ZynqMP
$make PLATFORM=zynqmp-zcu102
$make PLATFORM=zynqmp-zcu102 CFG_ARM64_core=y

# HiSilicon D02
$make PLATFORM=d02
$make PLATFORM=d02 CFG_ARM64_core=y

# Renesas RCAR H3
$make PLATFORM=rcar
$make PLATFORM=rcar CFG_ARM64_core=y

# Raspberry Pi 3
$make PLATFORM=rpi3
$make PLATFORM=rpi3 CFG_ARM64_core=y

echo "Run regression tests (xtest in QEMU)"
cd ${HOME}/optee_repo/build
$make check CROSS_COMPILE="ccache arm-linux-gnueabihf-" AARCH32_CROSS_COMPILE=arm-linux-gnueabihf- CFG_TEE_CORE_DEBUG=y DUMP_LOGS_ON_ERROR=1

echo "Exit script.sh"
