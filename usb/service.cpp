/*
 * Copyright (C) 2017-2018 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>
#include "Usb.h"

using android::sp;

// libhwbinder:
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

// Generated HIDL files
using android::hardware::usb::V1_0::IUsb;
using android::hardware::usb::V1_0::implementation::Usb;

int main() {
    android::sp<IUsb> service = new Usb();

    configureRpcThreadpool(1, true /*callerWillJoin*/);
    android::status_t status = service->registerAsService();

    if (status != android::OK) {
        LOG(ERROR) << "Cannot register USB HAL service";
        return 1;
    }

    LOG(INFO) << "USB HAL Ready.";
    joinRpcThreadpool();
    // Under normal cases, execution will not reach this line.
    LOG(ERROR) << "USB HAL failed to join thread pool.";
    return 1;
}
