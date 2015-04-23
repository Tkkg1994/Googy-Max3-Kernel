#!/bin/bash
{
	make mrproper
	0hulk_cm12_defconfig VARIANT_DEFCONFIG=jf_eur_defconfig SELINUX_DEFCONFIG=selinux_defconfig SELINUX_LOG_DEFCONFIG=selinux_log_defconfig DEBUG_DEFCONFIG=jfuserdebug_defconfig
        make -j3
}
