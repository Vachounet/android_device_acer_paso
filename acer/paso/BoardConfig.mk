# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# This file sets variables that control the way modules are built
# thorughout the system. It should not be used to conditionally
# disable makefiles (the proper mechanism to control what gets
# included in a build is to use PRODUCT_PACKAGES in a product
# definition file).
#

# WARNING: This line must come *before* including the proprietary
# variant, so that it gets overwritten by the parent (which goes
# against the traditional rules of inheritance).

# inherit from the proprietary version
-include vendor/acer/paso/BoardConfigVendor.mk

TARGET_NO_BOOTLOADER := true
TARGET_NO_KERNEL := true
TARGET_NO_RADIOIMAGE := true

# For Koush's recovery
BOARD_HAS_NO_SELECT_BUTTON := true

USE_CAMERA_STUB := true

TARGET_BOARD_PLATFORM := msm7x30
TARGET_BOARD_PLATFORM_GPU := qcom-adreno200

TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_ARCH_VARIANT := armv7-a-neon
ARCH_ARM_HAVE_TLS_REGISTER := true

TARGET_BOOTLOADER_BOARD_NAME := paso

# Wifi related defines
BOARD_WPA_SUPPLICANT_DRIVER := WEXT
WPA_SUPPLICANT_VERSION      := VER_0_6_X
BOARD_WLAN_DEVICE           := ar6000
WIFI_DRIVER_MODULE_PATH     := "/system/wifi/ar6000.ko"
#WIFI_DRIVER_FW_STA_PATH     := "/vendor/firmware/athwlan.bin.z77"
#WIFI_DRIVER_MODULE_ARG      := "firmware_path=/vendor/firmware/athwlan.bin.z77 nvram_path=/proc/calibration"
WIFI_DRIVER_MODULE_NAME     := "ar6000"

BOARD_USES_GENERIC_AUDIO := false

BOARD_KERNEL_CMDLINE := no_console_suspend=1 console=null
BOARD_KERNEL_RECOVERY_CMDLINE := $(BOARD_KERNEL_CMDLINE) msmsdcc_power_gpio=88
BOARD_KERNEL_BASE    := 0x20000000
BOARD_NAND_PAGE_SIZE := 4096
BOARD_PAGE_SIZE := 0x00001000

BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_BCM := true

BOARD_VENDOR_QCOM_AMSS_VERSION := 1200

BOARD_EGL_CFG := device/acer/paso/egl.cfg

BOARD_USES_QCOM_LIBS := true
BOARD_USES_QCOM_LIBRPC := true
BOARD_USES_QCOM_GPS := true

BOARD_OVERLAY_FORMAT_YCbCr_420_SP := true

# Current drivers don't support new EGL config
BOARD_NO_RGBX_8888 := true

BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE := paso
BOARD_VENDOR_QCOM_GPS_LOC_API_AMSS_VERSION := 1240

# Use nasty hack to make Kineto work
BOARD_USE_KINETO_COMPATIBILITY := true

# cat /proc/emmc
# dev:        size     erasesize name
# mmcblk0p17: 00040000 00000200 "misc"
# mmcblk0p21: 0087f400 00000200 "recovery"
# mmcblk0p22: 00400000 00000200 "boot"
# mmcblk0p25: 19fbfa00 00000200 "system"
# mmcblk0p27: 0cccce00 00000200 "cache"
# mmcblk0p26: 53200200 00000200 "userdata"
# mmcblk0p28: 01400000 00000200 "devlog"


TARGET_PREBUILT_KERNEL := device/acer/paso/kernel

BOARD_USES_MMCUTILS := true
BOARD_HAS_NO_MISC_PARTITION := true
