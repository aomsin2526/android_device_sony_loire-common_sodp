/*
 * Copyright (C) 2018 The Android Open Source Project
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
#define LOG_TAG "android.hardware.health-service.loire"
#include <android-base/logging.h>

#include <android-base/file.h>
#include <android-base/parseint.h>
#include <android-base/strings.h>
#include <android/hardware/health/translate-ndk.h>
#include <health-impl/Health.h>
#include <health/utils.h>

#ifndef __ANDROID_RECOVERY__
#include <health-impl/ChargerUtils.h>
#include "BatteryInfoUpdate.h"
#include "BatteryRechargingControl.h"
#include "CycleCountBackupRestore.h"
#include "LearnedCapacityBackupRestore.h"
#endif  // !__ANDROID_RECOVERY__

#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

namespace {

using namespace std::literals;

using aidl::android::hardware::health::DiskStats;
using aidl::android::hardware::health::HalHealthLoop;
using aidl::android::hardware::health::HealthInfo;
using aidl::android::hardware::health::StorageInfo;
using android::hardware::health::InitHealthdConfig;

#ifndef __ANDROID_RECOVERY__
using aidl::android::hardware::health::charger::ChargerCallback;
using aidl::android::hardware::health::charger::ChargerModeMain;
using ::device::sony::loire::health::BatteryInfoUpdate;
using ::device::sony::loire::health::BatteryRechargingControl;
using ::device::sony::loire::health::CycleCountBackupRestore;
using ::device::sony::loire::health::LearnedCapacityBackupRestore;

constexpr char kCycleCountsBins[] = "/sys/class/power_supply/bms/device/cycle_counts_bins";

static BatteryRechargingControl battRechargingControl;
static BatteryInfoUpdate battInfoUpdate;
static CycleCountBackupRestore ccBackupRestoreBMS(
        8, kCycleCountsBins, "/mnt/vendor/persist/battery/qcom_cycle_counts_bins");
static LearnedCapacityBackupRestore lcBackupRestore;
#endif  // !__ANDROID_RECOVERY__

#define EMMC_DIR "/sys/devices/platform/soc/7824900.sdhci"
const std::string kEmmcHealthEol{EMMC_DIR "/health/eol"};
const std::string kEmmcHealthLifetimeA{EMMC_DIR "/health/lifetimeA"};
const std::string kEmmcHealthLifetimeB{EMMC_DIR "/health/lifetimeB"};
const std::string kEmmcVersion{"/sys/block/mmcblk0/device/fwrev"};
const std::string kDiskStatsFile{"/sys/block/mmcblk0/stat"};

std::ifstream assert_open(const std::string& path) {
    std::ifstream stream(path);
    if (!stream.is_open()) {
        LOG(WARNING) << "Cannot read " << path;
    }
    return stream;
}

template <typename T>
void read_value_from_file(const std::string& path, T* field) {
    auto stream = assert_open(path);
    stream.unsetf(std::ios_base::basefield);
    stream >> *field;
}

void read_emmc_version(StorageInfo* info) {
    uint64_t value;
    read_value_from_file(kEmmcVersion, &value);
    std::stringstream ss;
    ss << "mmc0 " << std::hex << value;
    info->version = ss.str();
}

#ifdef __ANDROID_RECOVERY__
void private_healthd_board_init(struct healthd_config*) {}
int private_healthd_board_battery_update(HealthInfo*) {
    return 0;
}
#else  // !__ANDROID__RECOVERY__
void private_healthd_board_init(struct healthd_config*) {
    ccBackupRestoreBMS.Restore();
    lcBackupRestore.Restore();
}

int private_healthd_board_battery_update(HealthInfo* health_info) {
    battRechargingControl.updateBatteryProperties(health_info);
    battInfoUpdate.update(health_info);
    ccBackupRestoreBMS.Backup(health_info->batteryLevel);
    lcBackupRestore.Backup();
    return 0;
}

#endif  // __ANDROID_RECOVERY__

void private_get_storage_info(std::vector<StorageInfo>* vec_storage_info) {
    vec_storage_info->resize(1);
    StorageInfo* storage_info = &vec_storage_info->at(0);

    read_emmc_version(storage_info);
    read_value_from_file(kEmmcHealthEol, &storage_info->eol);
    read_value_from_file(kEmmcHealthLifetimeA, &storage_info->lifetimeA);
    read_value_from_file(kEmmcHealthLifetimeB, &storage_info->lifetimeB);
    return;
}

void private_get_disk_stats(std::vector<DiskStats>* vec_stats) {
    vec_stats->resize(1);
    DiskStats* stats = &vec_stats->at(0);

    auto stream = assert_open(kDiskStatsFile);
    // Regular diskstats entries
    stream >> stats->reads >> stats->readMerges >> stats->readSectors >> stats->readTicks >>
            stats->writes >> stats->writeMerges >> stats->writeSectors >> stats->writeTicks >>
            stats->ioInFlight >> stats->ioTicks >> stats->ioInQueue;
    return;
}
}  // anonymous namespace

namespace aidl::android::hardware::health::implementation {
class HealthImpl : public Health {
  public:
    HealthImpl(std::string_view instance_name, std::unique_ptr<healthd_config>&& config)
        : Health(std::move(instance_name), std::move(config)) {}

    ndk::ScopedAStatus getDiskStats(std::vector<DiskStats>* out) override;
    ndk::ScopedAStatus getStorageInfo(std::vector<StorageInfo>* out) override;

  protected:
    void UpdateHealthInfo(HealthInfo* health_info) override;
};

void HealthImpl::UpdateHealthInfo(HealthInfo* health_info) {
    private_healthd_board_battery_update(health_info);
}

ndk::ScopedAStatus HealthImpl::getStorageInfo(std::vector<StorageInfo>* out) {
    private_get_storage_info(out);
    if (out->empty()) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus HealthImpl::getDiskStats(std::vector<DiskStats>* out) {
    private_get_disk_stats(out);
    if (out->empty()) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
    return ndk::ScopedAStatus::ok();
}

#ifndef __ANDROID_RECOVERY__
class ChargerCallbackImpl : public ChargerCallback {
  public:
    ChargerCallbackImpl(const std::shared_ptr<HealthImpl>& service) : ChargerCallback(service) {}
    bool ChargerEnableSuspend() override { return true; }
};
#endif
}  // namespace aidl::android::hardware::health::implementation

int main(int argc, char** argv) {
#ifndef __ANDROID_RECOVERY__
    using ::aidl::android::hardware::health::implementation::ChargerCallbackImpl;
#endif
    using ::aidl::android::hardware::health::implementation::HealthImpl;

    // Use kernel logging in recovery
#ifdef __ANDROID_RECOVERY__
    android::base::InitLogging(argv, android::base::KernelLogger);
#endif

    auto config = std::make_unique<healthd_config>();
    InitHealthdConfig(config.get());

    private_healthd_board_init(config.get());

    auto binder = ndk::SharedRefBase::make<HealthImpl>("default"sv, std::move(config));

    if (argc >= 2 && argv[1] == "--charger"sv) {
        // In regular mode, start charger UI.
#ifndef __ANDROID_RECOVERY__
        LOG(INFO) << "Starting charger mode with UI.";
        auto charger_callback = std::make_shared<ChargerCallbackImpl>(binder);
        return ChargerModeMain(binder, charger_callback);
#endif
        // In recovery, ignore --charger arg.
        LOG(INFO) << "Starting charger mode without UI.";
    } else {
        LOG(INFO) << "Starting health HAL.";
    }

    auto hal_health_loop = std::make_shared<HalHealthLoop>(binder, binder);
    return hal_health_loop->StartLoop();
}
