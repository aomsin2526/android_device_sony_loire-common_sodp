/*
 * Copyright (C) 2017 The Android Open Source Project
 *               2018 Shane Francis / Jens Andersen
 *               2019 Marijn Suijten
 *               2018-2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ANDROID_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_BIOMETRICSFINGERPRINT_H
#define ANDROID_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_BIOMETRICSFINGERPRINT_H

#include <android/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#include <android/log.h>
#include <hardware/fingerprint.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <log/log.h>
#include <mutex>

#include "FingerprintScanner.h"
#include "WorkerThread.h"

namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace V2_1 {
namespace implementation {

using ::android::sp;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;
using ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprintClientCallback;
using ::android::hardware::biometrics::fingerprint::V2_1::RequestStatus;
using namespace ::SynchronizedWorkerThread;

struct BiometricsFingerprint : public IBiometricsFingerprint, public WorkHandler {
  public:
    BiometricsFingerprint();
    ~BiometricsFingerprint();

    // Methods from ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint
    // follow.
    Return<uint64_t> setNotify(
            const sp<IBiometricsFingerprintClientCallback>& clientCallback) override;
    Return<uint64_t> preEnroll() override;
    Return<RequestStatus> enroll(const hidl_array<uint8_t, 69>& hat, uint32_t gid,
                                 uint32_t timeoutSec) override;
    Return<RequestStatus> postEnroll() override;
    Return<uint64_t> getAuthenticatorId() override;
    Return<RequestStatus> cancel() override;
    Return<RequestStatus> enumerate() override;
    Return<RequestStatus> remove(uint32_t gid, uint32_t fid) override;
    Return<RequestStatus> setActiveGroup(uint32_t gid, const hidl_string& storePath) override;
    Return<RequestStatus> authenticate(uint64_t operationId, uint32_t gid) override;

    // Methods from ::SynchronizedWorkerThread::WorkHandler
    inline WorkerThread& getWorker() override { return mWt; }
    void AuthenticateAsync() override;
    void EnrollAsync() override;
    void IdleAsync() override;

  private:
    static Return<RequestStatus> ErrorFilter(int32_t error);

    int __setActiveGroup(uint32_t gid);

    WorkerThread mWt;
    char db_path[255];
    fpc_imp_data_t* fpc = NULL;
    sp<IBiometricsFingerprintClientCallback> mClientCallback = NULL;
    std::mutex mClientCallbackMutex;
    uint32_t gid;
    uint64_t auth_challenge, enroll_challenge;
};

}  // namespace implementation
}  // namespace V2_1
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_BIOMETRICSFINGERPRINT_H
