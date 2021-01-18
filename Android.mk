LOCAL_PATH := $(call my-dir)

## include variants like TA_DEV_KIT_DIR
## and OPTEE_BIN
INCLUDE_FOR_BUILD_TA := false
include $(BUILD_OPTEE_MK)
INCLUDE_FOR_BUILD_TA :=

VERSION = $(shell git describe --always --dirty=-dev 2>/dev/null || echo Unknown)

# TA_DEV_KIT_DIR must be set to non-empty value to
# avoid the Android build scripts complaining about
# includes pointing outside the Android source tree.
# This var is expected to be set when OPTEE OS built.
# We set the default value to an invalid path.
TA_DEV_KIT_DIR ?= ../invalid_include_path

-include $(TA_DEV_KIT_DIR)/host_include/conf.mk

include $(LOCAL_PATH)/ta/Android.mk
