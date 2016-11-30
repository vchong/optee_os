#!/bin/bash

echo "Enter coverity_build.sh"

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

# QEMU-virt (PLATFORM=vexpress-qemu_virt)
$make

# HiKey board (HiSilicon Kirin 620)
$make PLATFORM=hikey CFG_ARM64_core=y

echo "Exit coverity_build.sh"
