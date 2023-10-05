/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "BattInfoUpdate"

#include "BatteryInfoUpdate.h"

using aidl::android::hardware::health::BatteryStatus;
using aidl::android::hardware::health::HealthInfo;

namespace device {
namespace sony {
namespace loire {
namespace health {

BatteryInfoUpdate::BatteryInfoUpdate() {}

void BatteryInfoUpdate::update(HealthInfo* health_info) {
    if (health_info->batteryStatus == BatteryStatus::NOT_CHARGING)
        health_info->batteryStatus = BatteryStatus::DISCHARGING;
}

}  // namespace health
}  // namespace loire
}  // namespace sony
}  // namespace device
