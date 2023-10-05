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

class CycleCountBackupRestore {
  public:
    CycleCountBackupRestore(int nb_buckets, const char* sysfs_path, const char* persist_path);
    void Restore();
    void Backup(int battery_level);

  private:
    int nb_buckets_;
    int* sw_bins_;
    int* hw_bins_;
    int saved_soc_;
    int soc_inc_;
    std::string sysfs_path_;
    std::string persist_path_;

    void Read(const std::string& path, int* bins);
    void Write(int* bins, const std::string& path);
    void UpdateAndSave();
};

}  // namespace health
}  // namespace loire
}  // namespace sony
}  // namespace device
