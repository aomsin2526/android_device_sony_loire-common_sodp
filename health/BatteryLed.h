/*
 * Copyright (C) 2023 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/health/HealthInfo.h>
#include <android-base/file.h>
#include <android-base/logging.h>
#include <batteryservice/BatteryService.h>

namespace device {
namespace sony {
namespace loire {
namespace health {

#define AS3668_PATH "/sys/class/leds/as3668/"
#define LED_PATH(led, file) "/sys/class/leds/" led "/" file

class BatteryLed {
  public:
    BatteryLed();
    void update(aidl::android::hardware::health::HealthInfo* health_info);

  private:
    enum {
        RED_LED = 0x01 << 0,
        GREEN_LED = 0x01 << 1,
        BLUE_LED = 0x01 << 2,
        NUM_LEDS,
    };

    enum led_states { LED_STATE_BLINK, LED_STATE_BRIGHTNESS };

    struct led_ctl {
        int color;
        const char* path;
    };

    struct soc_led_color_mapping {
        int soc;
        int color;
        int state;
    };

    struct led_ctl leds[NUM_LEDS] = {{RED_LED, LED_PATH("as3668:red", "brightness")},
                                     {GREEN_LED, LED_PATH("as3668:green", "brightness")},
                                     {BLUE_LED, LED_PATH("as3668:blue", "brightness")}};

    struct led_ctl leds_pattern[NUM_LEDS] = {
            {RED_LED, LED_PATH("as3668:red", "pattern_brightness")},
            {GREEN_LED, LED_PATH("as3668:green", "pattern_brightness")},
            {BLUE_LED, LED_PATH("as3668:blue", "pattern_brightness")}};

    struct soc_led_color_mapping soc_leds[NUM_LEDS] = {
            {15, RED_LED, LED_STATE_BLINK},
            {90, RED_LED | GREEN_LED, LED_STATE_BRIGHTNESS},
            {100, GREEN_LED, LED_STATE_BRIGHTNESS},
    };

    bool writeIntToFile(const std::string& path, const int value);
    bool disable_tricolor_led();
    bool set_tricolor_led(int color, int state, int on);
};

}  // namespace health
}  // namespace loire
}  // namespace sony
}  // namespace device
