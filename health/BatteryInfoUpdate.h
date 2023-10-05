/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/health/HealthInfo.h>
#include <batteryservice/BatteryService.h>

namespace device {
namespace sony {
namespace loire {
namespace health {

class BatteryInfoUpdate {
  public:
    BatteryInfoUpdate();
    void update(aidl::android::hardware::health::HealthInfo* health_info);
};

}  // namespace health
}  // namespace loire
}  // namespace sony
}  // namespace device
