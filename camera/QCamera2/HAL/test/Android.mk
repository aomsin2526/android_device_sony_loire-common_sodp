LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    qcamera_test.cpp \

LOCAL_SHARED_LIBRARIES:= \
    libdl \
    libui \
    libutils \
    libcutils \
    libbinder \
    libmedia \
    libui \
    libgui \
    libcamera_client \
    libskia \
    libstagefright \
    libstagefright_foundation \

ifneq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) >= 18 ))" )))

LOCAL_SHARED_LIBRARIES += \
    libmedia_native \

LOCAL_32_BIT_ONLY := $(BOARD_QTI_CAMERA_32BIT_ONLY)
LOCAL_CFLAGS += -DUSE_JB_MR1

endif

LOCAL_C_INCLUDES += \
    external/skia/include/core \
    external/skia/include/images \
    $(TARGET_OUT_HEADERS)/qcom/display \
    $(LOCAL_PATH)/QCamera2/stack/common \
    $(LOCAL_PATH)/QCamera2/stack/mm-camera-interface/inc

LOCAL_HEADER_LIBRARIES := generated_kernel_headers

LOCAL_MODULE:= camera_test
LOCAL_VENDOR_MODULE := true
include $(SDCLANG_COMMON_DEFS)
LOCAL_MODULE_TAGS:= tests

LOCAL_CFLAGS += -Wall -Wextra -Werror -Wno-unused-parameter
LOCAL_CFLAGS += -O0

ifeq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) >= 20 ))" )))

LOCAL_CFLAGS += -DUSE_SDK_20_OR_HIGHER

ifeq ($(TARGET_USES_AOSP),true)
LOCAL_CFLAGS += -DVANILLA_HAL
endif

endif

#include $(BUILD_EXECUTABLE)
