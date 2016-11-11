#!/bin/bash

echo "Enter before_install.sh"
# Install the cross compilers
wget http://releases.linaro.org/components/toolchain/binaries/5.3-2016.02/arm-linux-gnueabihf/gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf.tar.xz
tar xf gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf.tar.xz
export PATH=${PWD}/gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf/bin:${PATH}
arm-linux-gnueabihf-gcc --version
wget http://releases.linaro.org/components/toolchain/binaries/5.3-2016.02/aarch64-linux-gnu/gcc-linaro-5.3-2016.02-x86_64_aarch64-linux-gnu.tar.xz
tar xf gcc-linaro-5.3-2016.02-x86_64_aarch64-linux-gnu.tar.xz
export PATH=${PWD}/gcc-linaro-5.3-2016.02-x86_64_aarch64-linux-gnu/bin:${PATH}
aarch64-linux-gnu-gcc --version
echo "Exit before_install.sh"
