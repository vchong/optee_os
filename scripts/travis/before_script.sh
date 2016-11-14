#!/bin/bash

echo "Enter before_script.sh"

# Store the home repository
export MYHOME=$PWD

# Download checkpatch.pl
export DST_KERNEL=$PWD/linux && mkdir -p $DST_KERNEL/scripts && cd $DST_KERNEL/scripts
wget https://raw.githubusercontent.com/torvalds/linux/master/scripts/checkpatch.pl && chmod a+x checkpatch.pl
wget https://raw.githubusercontent.com/torvalds/linux/master/scripts/spelling.txt
echo "invalid.struct.name" >const_structs.checkpatch
cd $MYHOME

export DL_DIR=$HOME/downloads
function _download() { url="$1"; f="${2:-$(basename $url)}"; if [ ! -e $DL_DIR/$f ] ; then mkdir -p $DL_DIR ; wget $url -O $DL_DIR/$f ; fi }
function download() { _download "$1" "" ; }

# Travis assigns 2 CPU cores to the container-based environment, so -j3 is
# a good concurrency level
# https://docs.travis-ci.com/user/ci-environment/
export make="make -j3 -s"

# Download  and build Git to be used by the checkpatch step
# The Travis container-based infrastructure runs Ubuntu 12.04 (Precise) which
# comes with git 1.8.5.6. The path exclusion syntax ':(exclude)' used below
# requires a more recent version.
cd $HOME
_download https://github.com/git/git/archive/v2.9.3.tar.gz git-2.9.3.tar.gz
tar xf $DL_DIR/git-2.9.3.tar.gz
$make -C git-2.9.3 CC="ccache gcc" NO_CURL=1

# Tools required for QEMU tests
# 'apt-get install' cannot be used in the new container-based infrastructure
# (which is the only allowing caching), so we just build from sources
# bc is used during kernel configuration
cd $HOME
download http://ftp.gnu.org/gnu/bc/bc-1.06.tar.gz
tar xf $DL_DIR/bc-1.06.tar.gz
(cd bc-1.06 && CC="ccache gcc" ./configure --quiet && $make)
export PATH=${HOME}/bc-1.06/bc:$PATH
# Tcl/Expect
download http://prdownloads.sourceforge.net/tcl/tcl8.6.4-src.tar.gz
tar xf $DL_DIR/tcl8.6.4-src.tar.gz
(cd tcl8.6.4/unix && ./configure --quiet --prefix=${HOME}/inst CC="ccache gcc" && $make install)
_download http://sourceforge.net/projects/expect/files/Expect/5.45/expect5.45.tar.gz/download expect5.45.tar.gz
tar xf $DL_DIR/expect5.45.tar.gz
(cd expect5.45 && ./configure --quiet --with-tcl=${HOME}/inst/lib --prefix=${HOME}/inst CC="ccache gcc" && $make expect && $make install)
export PATH=$HOME/inst/bin:$PATH
# pycrypto 2.6.1 or later has Crypto.Signature, 2.4.1 does not. It is needed to sign the test TAs.
pip install --upgrade --user pycrypto
pip install --upgrade --user wand
# Clone repositories for the QEMU test environment
mkdir $HOME/bin
(cd $HOME/bin && wget https://storage.googleapis.com/git-repo-downloads/repo && chmod +x repo)
export PATH=$HOME/bin:$PATH
mkdir $HOME/optee_repo
#(cd $HOME/optee_repo && repo forall -vc "git clean -xdf; git reset --hard")
#(cd $HOME/optee_repo && repo forall -vc "git reset --hard")
#(cd $HOME/optee_repo && repo init -u https://github.com/OP-TEE/manifest.git -m travis.xml </dev/null && repo forall -vc "git reset --hard" && repo sync --force-sync --no-clone-bundle --no-tags --quiet -j 2)
(cd $HOME/optee_repo && repo init -u https://github.com/OP-TEE/manifest.git -m travis.xml </dev/null && repo sync --force-sync --no-clone-bundle --no-tags --quiet -j 2)
(cd $HOME/optee_repo/qemu && git submodule update --init dtc)
(cd $HOME/optee_repo && mv optee_os optee_os_old && ln -s $MYHOME optee_os)
cd $MYHOME
git fetch https://github.com/OP-TEE/optee_os --tags
unset CC

export PATH=$HOME/git-2.9.3/:$DST_KERNEL/scripts/:$PATH
source ${HOME}/optee_repo/optee_os/scripts/checkpatch_inc.sh
# Several compilation options are checked

echo "Exit before_script.sh"
