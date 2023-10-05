/*
 * Copyright (C) 2019 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "GloveModeService"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>

#include "GloveMode.h"

namespace vendor {
namespace lineage {
namespace touch {
namespace V1_0 {
namespace implementation {

constexpr const char kControlPath[] = "/sys/devices/virtual/input/clearpad/glove";

// Methods from ::vendor::lineage::touch::V1_0::IGloveMode follow.
Return<bool> GloveMode::isEnabled() {
    std::string buf;

    if (!android::base::ReadFileToString(kControlPath, &buf)) {
        LOG(ERROR) << "Failed to read " << kControlPath;
        return false;
    }

    return std::stoi(android::base::Trim(buf)) == 1;
}

Return<bool> GloveMode::setEnabled(bool enabled) {
    if (!android::base::WriteStringToFile((enabled ? "1" : "0"), kControlPath)) {
        LOG(ERROR) << "Failed to write " << kControlPath;
        return false;
    }

    return true;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace lineage
}  // namespace vendor
