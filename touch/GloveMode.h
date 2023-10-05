/*
 * Copyright (C) 2019 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef VENDOR_LINEAGE_TOUCH_V1_0_GLOVEMODE_H
#define VENDOR_LINEAGE_TOUCH_V1_0_GLOVEMODE_H

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <vendor/lineage/touch/1.0/IGloveMode.h>

namespace vendor {
namespace lineage {
namespace touch {
namespace V1_0 {
namespace implementation {

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

class GloveMode : public IGloveMode {
  public:
    // Methods from ::vendor::lineage::touch::V1_0::IGloveMode follow.
    Return<bool> isEnabled() override;
    Return<bool> setEnabled(bool enabled) override;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace lineage
}  // namespace vendor

#endif  // VENDOR_LINEAGE_TOUCH_V1_0_GLOVEMODE_H
