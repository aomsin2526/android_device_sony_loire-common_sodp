/*
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "vendor.qti.vibrator"

#include <dirent.h>
#include <fcntl.h>
#include <inttypes.h>
#include <log/log.h>
#include <string.h>
#include <thread>
#include <map>

#include "Vibrator.h"

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

#define AMPLITUDE_LIGHT 0.25
#define AMPLITUDE_MEDIUM 0.5
#define AMPLITUDE_STRONG 1

#define DURATION_AMPLITUDE_LIGHT 0.65
#define DURATION_AMPLITUDE_MEDIUM 0.8
#define DURATION_AMPLITUDE_STRONG 1

static std::map<EffectStrength, float> DURATION_AMPLITUDE = {
    { EffectStrength::LIGHT, DURATION_AMPLITUDE_LIGHT },
    { EffectStrength::MEDIUM, DURATION_AMPLITUDE_MEDIUM },
    { EffectStrength::STRONG, DURATION_AMPLITUDE_STRONG }
};

static const char LED_DEVICE[] = "/sys/class/leds/vibrator";

LedVibratorDevice::LedVibratorDevice() {
    char devicename[PATH_MAX];
    int fd;

    snprintf(devicename, sizeof(devicename), "%s/%s", LED_DEVICE, "activate");
    fd = TEMP_FAILURE_RETRY(open(devicename, O_RDWR));
    if (fd < 0) {
        ALOGE("open %s failed, errno = %d", devicename, errno);
        return;
    }
}

int LedVibratorDevice::write_value(const char *file, const char *value) {
    int fd;
    int ret;

    fd = TEMP_FAILURE_RETRY(open(file, O_WRONLY));
    if (fd < 0) {
        ALOGE("open %s failed, errno = %d", file, errno);
        return -errno;
    }

    ret = TEMP_FAILURE_RETRY(write(fd, value, strlen(value) + 1));
    if (ret == -1) {
        ret = -errno;
    } else if (ret != strlen(value) + 1) {
        /* even though EAGAIN is an errno value that could be set
           by write() in some cases, none of them apply here.  So, this return
           value can be clearly identified when debugging and suggests the
           caller that it may try to call vibrator_on() again */
        ret = -EAGAIN;
    } else {
        ret = 0;
    }

    errno = 0;
    close(fd);

    return ret;
}

int LedVibratorDevice::on(int32_t timeoutMs) {
    char file[PATH_MAX];
    char value[32];
    int ret;

    snprintf(file, sizeof(file), "%s/%s", LED_DEVICE, "state");
    ret = write_value(file, "1");
    if (ret < 0)
       goto error;

    snprintf(file, sizeof(file), "%s/%s", LED_DEVICE, "duration");
    snprintf(value, sizeof(value), "%u\n", timeoutMs);
    ret = write_value(file, value);
    if (ret < 0)
       goto error;

    snprintf(file, sizeof(file), "%s/%s", LED_DEVICE, "activate");
    ret = write_value(file, "1");
    if (ret < 0)
       goto error;

    return 0;

error:
    ALOGE("Failed to turn on vibrator ret: %d\n", ret);
    return ret;
}

int LedVibratorDevice::off()
{
    char file[PATH_MAX];
    int ret;

    snprintf(file, sizeof(file), "%s/%s", LED_DEVICE, "activate");
    ret = write_value(file, "0");
    return ret;
}

ndk::ScopedAStatus Vibrator::getCapabilities(int32_t* _aidl_return) {
    *_aidl_return = IVibrator::CAP_ON_CALLBACK | IVibrator::CAP_PERFORM_CALLBACK |
                    IVibrator::CAP_AMPLITUDE_CONTROL;
    ALOGD("QTI Vibrator reporting capabilities: %d", *_aidl_return);

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::off() {
    int ret;

    ALOGD("QTI Vibrator off");
    ret = ledVib.off();
    if (ret != 0)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_SERVICE_SPECIFIC));

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::on(int32_t timeoutMs,
                                const std::shared_ptr<IVibratorCallback>& callback) {
    int ret;

    timeoutMs *= mDurationAmplitude;

    ALOGD("Vibrator on for timeoutMs: %d", timeoutMs);

    ret = ledVib.on(timeoutMs);
    if (ret != 0)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_SERVICE_SPECIFIC));

    if (callback != nullptr) {
        std::thread([=] {
            ALOGD("Starting on on another thread");
            usleep(timeoutMs * 1000);
            ALOGD("Notifying on complete");
            if (!callback->onComplete().isOk()) {
                ALOGE("Failed to call onComplete");
            }
        }).detach();
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::perform(Effect effect, EffectStrength es,
                                     const std::shared_ptr<IVibratorCallback>& callback,
                                     int32_t* _aidl_return) {
    ndk::ScopedAStatus status;
    int ret;
    float amplitude;
    uint32_t ms;

    amplitude = strengthToAmplitude(es, &status);
    if (!status.isOk())
        return status;

    setAmplitude(amplitude);

    ms = effectToMs(effect, &status);
    ms *= DURATION_AMPLITUDE[es];

    ALOGD("Vibrator perform effect %s (%d ms)", effectToName(effect).c_str(), ms);

    ret = ledVib.on(ms);
    if (ret != 0)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_SERVICE_SPECIFIC));

    if (callback != nullptr) {
        std::thread([=] {
            ALOGD("Starting perform on another thread");
            usleep(ms * 1000);
            ALOGD("Notifying perform complete");
            callback->onComplete();
        }).detach();
    }

    *_aidl_return = ms;
    return status;
}

ndk::ScopedAStatus Vibrator::getSupportedEffects(std::vector<Effect>* _aidl_return) {
    *_aidl_return = { Effect::CLICK, Effect::TICK, Effect::TEXTURE_TICK };
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setAmplitude(float amplitude) {
    if (amplitude <= 0.0f || amplitude > 1.0f)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_ILLEGAL_ARGUMENT));

    mDurationAmplitude = durationAmplitude(amplitude);

    ALOGD("Vibrator set amplitude: %f (%f)", amplitude, mDurationAmplitude);

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setExternalControl(bool enabled  __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getCompositionDelayMax(int32_t* maxDelayMs  __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getCompositionSizeMax(int32_t* maxSize __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getSupportedPrimitives(std::vector<CompositePrimitive>* supported __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getPrimitiveDuration(CompositePrimitive primitive __unused,
                                                  int32_t* durationMs __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::compose(const std::vector<CompositeEffect>& composite __unused,
                                     const std::shared_ptr<IVibratorCallback>& callback __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getSupportedAlwaysOnEffects(std::vector<Effect>* _aidl_return __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::alwaysOnEnable(int32_t id __unused, Effect effect __unused,
                                            EffectStrength strength __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::alwaysOnDisable(int32_t id __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getResonantFrequency(float *resonantFreqHz __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getQFactor(float *qFactor __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getFrequencyResolution(float *freqResolutionHz __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getFrequencyMinimum(float *freqMinimumHz __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getBandwidthAmplitudeMap(std::vector<float> *_aidl_return __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getPwlePrimitiveDurationMax(int32_t *durationMs __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getPwleCompositionSizeMax(int32_t *maxSize __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getSupportedBraking(std::vector<Braking> *supported __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::composePwle(const std::vector<PrimitivePwle> &composite __unused,
                           const std::shared_ptr<IVibratorCallback> &callback __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

float Vibrator::strengthToAmplitude(EffectStrength es, ndk::ScopedAStatus* status) {
    *status = ndk::ScopedAStatus::ok();

    switch (es) {
        case EffectStrength::LIGHT:
            return AMPLITUDE_LIGHT;
        case EffectStrength::MEDIUM:
            return AMPLITUDE_MEDIUM;
        case EffectStrength::STRONG:
            return AMPLITUDE_STRONG;
    }

    *status = ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    return 0;
}

const std::string Vibrator::effectToName(Effect effect) {
    return toString(effect);
}

uint32_t Vibrator::effectToMs(Effect effect, ndk::ScopedAStatus* status) {
    *status = ndk::ScopedAStatus::ok();
    switch (effect) {
        case Effect::CLICK:
            return 25;
        case Effect::DOUBLE_CLICK:
            return 45;
        case Effect::HEAVY_CLICK:
            return 35;
        case Effect::TICK:
            return 20;
        case Effect::TEXTURE_TICK:
            return 15;
        default:
            break;
    }
    *status = ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    return 0;
}

float Vibrator::durationAmplitude(float amplitude) {
    if (amplitude == 1) {
        return DURATION_AMPLITUDE_STRONG;
    } else if (amplitude >= 0.5) {
        return DURATION_AMPLITUDE_MEDIUM;
    }

    return DURATION_AMPLITUDE_LIGHT;
}

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
