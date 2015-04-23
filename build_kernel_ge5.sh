#!/bin/sh
export KERNELDIR=`readlink -f .`
export PARENT_DIR=`readlink -f ..`
export USE_SEC_FIPS_MODE=true
export CROSS_COMPILE=/Kernel_Folder/Toolchain_4.9.3-2015.03_a15/bin/arm-cortex_a15-linux-gnueabihf-

if [ "${2}" = "x" ];then
 make mrproper || exit 1
# make -j5 0googymax3_defconfig || exit 1
fi

# if [ ! -f $KERNELDIR/.config ];
# if [ "${2}" = "y" ];then
find -name '*.ko' -exec rm -rf {} \;
# fi

# 
make 0hulk_GE5_defconfig VARIANT_DEFCONFIG=jf_eur_defconfig SELINUX_DEFCONFIG=selinux_defconfig SELINUX_LOG_DEFCONFIG=selinux_log_defconfig DEBUG_DEFCONFIG=jfuserdebug_defconfig || exit 1

. $KERNELDIR/.config

export KCONFIG_NOTIMESTAMP=true
export ARCH=arm

cd $KERNELDIR/
make -j3 CONFIG_NO_ERROR_ON_MISMATCH=y || exit 1

