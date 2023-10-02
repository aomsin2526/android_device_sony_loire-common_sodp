/*
 * Copyright (C) 2023 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "BatteryLed"

#include "BatteryLed.h"

using aidl::android::hardware::health::BatteryStatus;
using aidl::android::hardware::health::HealthInfo;

namespace device {
namespace sony {
namespace loire {
namespace health {

#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))

BatteryLed::BatteryLed() {}

bool BatteryLed::writeIntToFile(const std::string& path, const int value) {
    bool success = android::base::WriteStringToFile(std::to_string(value), path);
    if (!success) {
        LOG(ERROR) << "Failed to write " << path << ", ret: " << strerror(errno);
    }

    return success;
}

bool BatteryLed::disable_tricolor_led() {
    int i;
    bool rc = true;

    for (i = 0; i < NUM_LEDS; i++) {
        rc = writeIntToFile(leds[i].path, 0);
        rc &= writeIntToFile(leds_pattern[i].path, 0);
    }
    rc &= writeIntToFile(AS3668_PATH "pattern_pwm_dim_speed_down_ms", 0);
    rc &= writeIntToFile(AS3668_PATH "pattern_pwm_dim_speed_up_ms", 0);
    rc &= writeIntToFile(AS3668_PATH "pattern_run", 0);

    return rc;
}

bool BatteryLed::set_tricolor_led(int color, int state, int on) {
    int i;
    bool rc = true;

    if (state == LED_STATE_BRIGHTNESS) {
        for (i = 0; i < NUM_LEDS; i++) {
            if (color & leds[i].color) {
                rc = writeIntToFile(leds[i].path, on ? 120 : 0);
            }
        }
    } else {
        for (i = 0; i < NUM_LEDS; i++) {
            if (color & leds_pattern[i].color) {
                rc = writeIntToFile(leds_pattern[i].path, on ? 120 : 0);
            }
        }
        rc &= writeIntToFile(AS3668_PATH "pattern_pwm_dim_speed_down_ms", on ? 3200 : 0);
        rc &= writeIntToFile(AS3668_PATH "pattern_pwm_dim_speed_up_ms", on ? 500 : 0);
        rc &= writeIntToFile(AS3668_PATH "pattern_run", on);
    }

    return rc;
}

void BatteryLed::update(HealthInfo* health_info) {
    static int old_color = 0;
    int i, color, soc, state;
    bool rc = true;

    if (!health_info) {
        return;
    }

    if (health_info->batteryStatus == BatteryStatus::UNKNOWN ||
#ifdef __ANDROID_RECOVERY__
        health_info->batteryStatus == BatteryStatus::DISCHARGING) {
#else   // !__ANDROID__RECOVERY__
        health_info->batteryStatus == BatteryStatus::NOT_CHARGING) {
#endif  // __ANDROID_RECOVERY__
        rc = disable_tricolor_led();
        if (rc < 0) LOG(ERROR) << "Error in disabling tricolor_led";

        return;
    }

    soc = health_info->batteryLevel;

    for (i = 0; i < ((int)ARRAY_SIZE(soc_leds) - 1); i++) {
        if (soc <= soc_leds[i].soc) break;
    }
    color = soc_leds[i].color;
    state = soc_leds[i].state;

    if (old_color != color) {
        rc = set_tricolor_led(old_color, state, 0);
        if (!rc) LOG(ERROR) << "Error in setting old_color on tricolor_led";

        rc = set_tricolor_led(color, state, 1);
        if (!rc) LOG(ERROR) << "Error in setting color on tricolor_led";

        if (rc) {
            old_color = color;
            LOG(VERBOSE) << "soc = " << soc << ", set led color 0x" << soc_leds[i].color;
        }
    }
}

}  // namespace health
}  // namespace loire
}  // namespace sony
}  // namespace device
