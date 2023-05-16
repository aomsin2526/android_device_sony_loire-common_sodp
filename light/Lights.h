/*
 * Copyright (C) 2020 The Android Open Source Project
 * Copyright (C) 2021-2023 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/light/BnLights.h>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

enum led_type {
    RED,
    GREEN,
    BLUE,
    NUM_LIGHTS,
};

class Lights : public BnLights {
  public:
    Lights();

    ndk::ScopedAStatus setLightState(int32_t id, const HwLightState& state) override;
    ndk::ScopedAStatus getLights(std::vector<HwLight>* lights) override;

  private:
    void setSpeakerLightLocked(const HwLightState& state);

    bool setLedBrightness(led_type led, const std::string& file, uint32_t value);

    bool isLit(uint32_t color);
    uint32_t RgbaToBrightness(uint32_t color);
    bool fileWriteable(const std::string& file);
    uint32_t readIntFromFile(const std::string& path, uint32_t defaultValue);
    bool writeToFile(const std::string& path, uint32_t content);

    HwLightState mNotification;
    HwLightState mBattery;

    std::string mBacklightPath;
    uint32_t mMaxBrightness;
    uint32_t mLEDMaxBrightness[NUM_LIGHTS];
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
