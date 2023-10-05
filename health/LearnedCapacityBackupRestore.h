/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>
#include <string>

namespace device {
namespace sony {
namespace loire {
namespace health {

class LearnedCapacityBackupRestore {
  public:
    LearnedCapacityBackupRestore();
    void Restore();
    void Backup();

  private:
    int sw_cap_;
    int hw_cap_;
    int nom_cap_;

    void ReadPersistData();
    void SaveToStorage();
    void ReadNominalCapacity();
    void ReadCapacity();
    void SaveToSRAM();
    void UpdateAndSave();
};

}  // namespace health
}  // namespace loire
}  // namespace sony
}  // namespace device
