/*
 * Copyright (C) 2019 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.lineage.touch@1.0-service.loire"

#include <android-base/logging.h>
#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>

#include "GloveMode.h"

using android::sp;
using android::OK;

using ::vendor::lineage::touch::V1_0::IGloveMode;
using ::vendor::lineage::touch::V1_0::implementation::GloveMode;

int main() {
    sp<IGloveMode> gloveMode = new GloveMode();

    android::hardware::configureRpcThreadpool(1, true /*callerWillJoin*/);

    if (gloveMode->registerAsService() != OK) {
        LOG(ERROR) << "Cannot register glove mode HAL service.";
        return 1;
    }
    LOG(INFO) << "Touch HAL service is ready.";

    android::hardware::joinRpcThreadpool();

    LOG(ERROR) << "Touch HAL service failed to join thread pool.";
    return 1;
}
