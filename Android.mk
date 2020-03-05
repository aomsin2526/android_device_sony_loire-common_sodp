#
# Copyright (C) 2018 The LineageOS Project
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

LOCAL_PATH := $(call my-dir)

ifneq ($(filter kugo suzu, $(TARGET_DEVICE)),)

include $(call all-makefiles-under,$(LOCAL_PATH))

include $(CLEAR_VARS)

RFS_APQ_GNSS_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/apq/gnss/
$(RFS_APQ_GNSS_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS APQ GNSS folder structure: $@"
	@rm -rf $@
	@mkdir -p $(dir $@)/readonly
	$(hide) ln -sf /data/vendor/tombstones/rfs/modem $@/ramdumps
	$(hide) ln -sf /persist/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /persist/rfs/apq/gnss $@/readwrite
	$(hide) ln -sf /persist/rfs/shared $@/shared
	$(hide) ln -sf /firmware $@/readonly/firmware
	$(hide) ln -sf /vendor/firmware $@/readonly/vendor

RFS_MDM_ADSP_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/mdm/adsp/
$(RFS_MDM_ADSP_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS MDM ADSP folder structure: $@"
	@rm -rf $@
	@mkdir -p $(dir $@)/readonly
	$(hide) ln -sf /data/vendor/tombstones/rfs/lpass $@/ramdumps
	$(hide) ln -sf /persist/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /persist/rfs/mdm/adsp $@/readwrite
	$(hide) ln -sf /persist/rfs/shared $@/shared
	$(hide) ln -sf /firmware $@/readonly/firmware
	$(hide) ln -sf /vendor/firmware $@/readonly/vendor

RFS_MDM_MPSS_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/mdm/mpss/
$(RFS_MDM_MPSS_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS MDM MPSS folder structure: $@"
	@rm -rf $@
	@mkdir -p $(dir $@)/readonly
	$(hide) ln -sf /data/vendor/tombstones/rfs/modem $@/ramdumps
	$(hide) ln -sf /persist/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /persist/rfs/mdm/mpss $@/readwrite
	$(hide) ln -sf /persist/rfs/shared $@/shared
	$(hide) ln -sf /firmware $@/readonly/firmware
	$(hide) ln -sf /vendor/firmware $@/readonly/vendor

RFS_MDM_SLPI_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/mdm/slpi/
$(RFS_MDM_SLPI_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS MDM SLPI folder structure: $@"
	@rm -rf $@
	@mkdir -p $(dir $@)/readonly
	$(hide) ln -sf /data/vendor/tombstones/rfs/slpi $@/ramdumps
	$(hide) ln -sf /persist/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /persist/rfs/mdm/slpi $@/readwrite
	$(hide) ln -sf /persist/rfs/shared $@/shared
	$(hide) ln -sf /firmware $@/readonly/firmware

RFS_MDM_TN_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/mdm/tn/
$(RFS_MDM_TN_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS MDM TN folder structure: $@"
	@rm -rf $@
	@mkdir -p $(dir $@)/readonly
	$(hide) ln -sf /data/vendor/tombstones/rfs/tn $@/ramdumps
	$(hide) ln -sf /persist/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /persist/rfs/mdm/tn $@/readwrite
	$(hide) ln -sf /persist/rfs/shared $@/shared
	$(hide) ln -sf /firmware $@/readonly/firmware

RFS_MSM_ADSP_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/msm/adsp/
$(RFS_MSM_ADSP_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS MSM ADSP folder structure: $@"
	@rm -rf $@
	@mkdir -p $(dir $@)/readonly
	$(hide) ln -sf /data/vendor/tombstones/rfs/lpass $@/ramdumps
	$(hide) ln -sf /persist/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /persist/rfs/msm/adsp $@/readwrite
	$(hide) ln -sf /persist/rfs/shared $@/shared
	$(hide) ln -sf /firmware $@/readonly/firmware
	$(hide) ln -sf /vendor/firmware $@/readonly/vendor

RFS_MSM_MPSS_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/msm/mpss/
$(RFS_MSM_MPSS_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS MSM MPSS folder structure: $@"
	@rm -rf $@
	@mkdir -p $(dir $@)/readonly
	$(hide) ln -sf /data/vendor/tombstones/rfs/modem $@/ramdumps
	$(hide) ln -sf /persist/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /persist/rfs/msm/mpss $@/readwrite
	$(hide) ln -sf /persist/rfs/shared $@/shared
	$(hide) ln -sf /firmware $@/readonly/firmware
	$(hide) ln -sf /vendor/firmware $@/readonly/vendor

RFS_MSM_SLPI_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/msm/slpi/
$(RFS_MSM_SLPI_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS MSM SLPI folder structure: $@"
	@rm -rf $@
	@mkdir -p $(dir $@)/readonly
	$(hide) ln -sf /data/vendor/tombstones/rfs/slpi $@/ramdumps
	$(hide) ln -sf /persist/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /persist/rfs/msm/slpi $@/readwrite
	$(hide) ln -sf /persist/rfs/shared $@/shared
	$(hide) ln -sf /firmware $@/readonly/firmware

ALL_DEFAULT_INSTALLED_MODULES += $(RFS_APQ_GNSS_SYMLINKS) $(RFS_MDM_ADSP_SYMLINKS) $(RFS_MDM_MPSS_SYMLINKS) $(RFS_MDM_SLPI_SYMLINKS) $(RFS_MDM_TN_SYMLINKS) $(RFS_MSM_ADSP_SYMLINKS) $(RFS_MSM_MPSS_SYMLINKS) $(RFS_MSM_SLPI_SYMLINKS)

IMS_LIBS := libimscamera_jni.so libimsmedia_jni.so
IMS_SYMLINKS := $(addprefix $(TARGET_OUT_VENDOR)/app/ims/lib/arm64/,$(notdir $(IMS_LIBS)))
$(IMS_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "IMS lib link: $@"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf /system/lib64/$(notdir $@) $@

ALL_DEFAULT_INSTALLED_MODULES += $(IMS_SYMLINKS)

EGL_LIBS := libEGL_adreno.so libGLESv2_adreno.so libq3dtools_adreno.so
EGL_32_SYMLINKS := $(addprefix $(TARGET_OUT_VENDOR)/lib/,$(notdir $(EGL_LIBS)))
$(EGL_32_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "EGL 32 lib link: $@"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf egl/$(notdir $@) $@

EGL_64_SYMLINKS := $(addprefix $(TARGET_OUT_VENDOR)/lib64/,$(notdir $(EGL_LIBS)))
$(EGL_64_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "EGL 64 lib link: $@"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf egl/$(notdir $@) $@

ALL_DEFAULT_INSTALLED_MODULES += $(EGL_32_SYMLINKS) $(EGL_64_SYMLINKS)

VULKAN_32_SYMLINK := $(TARGET_OUT_VENDOR)/lib/vulkan.adreno.so
$(VULKAN_32_SYMLINK): $(LOCAL_INSTALLED_MODULE)
	@echo "Vulkan 32 lib symlink: $@"
	@mkdir -p $(dir $@)
	$(hide) ln -sf hw/$(notdir $@) $@

VULKAN_64_SYMLINK := $(TARGET_OUT_VENDOR)/lib64/vulkan.adreno.so
$(VULKAN_64_SYMLINK): $(LOCAL_INSTALLED_MODULE)
	@echo "Vulkan 64 lib symlink: $@"
	@mkdir -p $(dir $@)
	$(hide) ln -sf hw/$(notdir $@) $@

ALL_DEFAULT_INSTALLED_MODULES += $(VULKAN_32_SYMLINK) $(VULKAN_64_SYMLINK)

endif
