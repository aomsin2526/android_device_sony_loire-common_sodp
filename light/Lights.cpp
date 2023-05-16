/*
 * Copyright (C) 2019 The Android Open Source Project
 * Copyright (C) 2021-2023 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Lights.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <fcntl.h>

using ::android::base::ReadFileToString;
using ::android::base::WriteStringToFile;

namespace aidl {
namespace android {
namespace hardware {
namespace light {

#define LED_PATH(led) "/sys/class/leds/" led "/"
#define AS3668_PATH LED_PATH("as3668")

static const std::string led_paths[]{
        [RED] = LED_PATH("as3668:red"),
        [GREEN] = LED_PATH("as3668:green"),
        [BLUE] = LED_PATH("as3668:blue"),
};

static const std::string kLCDFile = "/sys/class/leds/lcd-backlight/brightness";
static const std::string kLCDMaxFile = "/sys/class/leds/lcd-backlight/max_brightness";
static const std::string kBacklightFile = "/sys/class/backlight/backlight/brightness";
static const std::string kBacklightMaxFile = "/sys/class/backlight/backlight/max_brightness";

#define AutoHwLight(light) \
    { .id = (int32_t)light, .type = light, .ordinal = 0 }

// List of supported lights
const static std::vector<HwLight> kAvailableLights = {AutoHwLight(LightType::BACKLIGHT),
                                                      AutoHwLight(LightType::BATTERY),
                                                      AutoHwLight(LightType::NOTIFICATIONS)};

Lights::Lights() {
    std::string maxBrightnessPath;
    mBacklightPath = fileWriteable(kLCDFile) ? kLCDFile : kBacklightFile;
    maxBrightnessPath = fileWriteable(kLCDMaxFile) ? kLCDMaxFile : kBacklightMaxFile;

    mMaxBrightness = readIntFromFile(maxBrightnessPath, 0xFF);

    for (int i = 0; i < NUM_LIGHTS; i++)
        mLEDMaxBrightness[i] = readIntFromFile(led_paths[i] + "max_brightness", 0xFF);
}

// AIDL methods
ndk::ScopedAStatus Lights::setLightState(int32_t id, const HwLightState& state) {
    LightType type = static_cast<LightType>(id);
    switch (type) {
        case LightType::BACKLIGHT:
            if (!mBacklightPath.empty())
                writeToFile(mBacklightPath, RgbaToBrightness(state.color) * mMaxBrightness / 0xFF);
            break;
        case LightType::BATTERY:
            mBattery = state;
            break;
        case LightType::NOTIFICATIONS:
            mNotification = state;
            break;
        default:
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
            break;
    }

    if (isLit(mNotification.color))
        setSpeakerLightLocked(mNotification);
    else
        setSpeakerLightLocked(mBattery);

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Lights::getLights(std::vector<HwLight>* lights) {
    for (auto& light : kAvailableLights) lights->push_back(light);

    return ndk::ScopedAStatus::ok();
}

// device methods
void Lights::setSpeakerLightLocked(const HwLightState& state) {
    uint32_t alpha, red, green, blue;

    // Extract brightness from AARRGGBB.
    alpha = (state.color >> 24) & 0xFF;

    // Retrieve each of the RGB colors
    red = (state.color >> 16) & 0xFF;
    green = (state.color >> 8) & 0xFF;
    blue = state.color & 0xFF;

    // Scale RGB colors if a brightness has been applied by the user
    if (alpha > 0 && alpha < 255) {
        red = red * alpha / 0xFF;
        green = green * alpha / 0xFF;
        blue = blue * alpha / 0xFF;
    }

    switch (state.flashMode) {
        case FlashMode::HARDWARE:
        case FlashMode::TIMED:
            setLedBrightness(RED, "pattern_brightness", red);
            setLedBrightness(GREEN, "pattern_brightness", green);
            setLedBrightness(BLUE, "pattern_brightness", blue);
            writeToFile(AS3668_PATH "pattern_pwm_dim_speed_down_ms", state.flashOffMs);
            writeToFile(AS3668_PATH "pattern_pwm_dim_speed_up_ms", state.flashOnMs);
            writeToFile(AS3668_PATH "pattern_run", 1);
            break;
        case FlashMode::NONE:
        default:
            writeToFile(AS3668_PATH "pattern_run", 0);
            setLedBrightness(RED, "brightness", red);
            setLedBrightness(GREEN, "brightness", green);
            setLedBrightness(BLUE, "brightness", blue);
            break;
    }

    return;
}

bool Lights::setLedBrightness(led_type led, const std::string& file, uint32_t value) {
    return writeToFile(led_paths[led] + file, value * mLEDMaxBrightness[led] / 0xFF);
}

// Utils
bool Lights::isLit(uint32_t color) {
    return color & 0x00ffffff;
}

uint32_t Lights::RgbaToBrightness(uint32_t color) {
    // Extract brightness from AARRGGBB.
    uint32_t alpha = (color >> 24) & 0xFF;

    // Retrieve each of the RGB colors
    uint32_t red = (color >> 16) & 0xFF;
    uint32_t green = (color >> 8) & 0xFF;
    uint32_t blue = color & 0xFF;

    // Scale RGB colors if a brightness has been applied by the user
     if (alpha > 0 && alpha < 255) {
        red = red * alpha / 0xFF;
        green = green * alpha / 0xFF;
        blue = blue * alpha / 0xFF;
    }

    return (77 * red + 150 * green + 29 * blue) >> 8;
}

bool Lights::fileWriteable(const std::string& path) {
    return !access(path.c_str(), W_OK);
}

uint32_t Lights::readIntFromFile(const std::string& path, uint32_t defaultValue) {
    std::string buf;

    if (ReadFileToString(path, &buf)) {
        return std::stoi(buf);
    }
    return defaultValue;
}

bool Lights::writeToFile(const std::string& path, uint32_t content) {
    return WriteStringToFile(std::to_string(content), path);
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
