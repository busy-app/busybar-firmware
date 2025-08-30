/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          Provides an generic implementation of PlatformManager features
 *          for use on FreeRTOS platforms.
 */

#pragma once

#include <system/SystemClock.h>

#include <lib/core/CHIPError.h>

#include <platform/CHIPDeviceConfig.h>
#include <platform/PlatformManager.h>

#include <platform/internal/GenericPlatformManagerImpl.h>

#include <FreeRTOS.h>
#include <task.h>

#include <atomic>

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
class GenericPlatformManagerImpl_Furi : public GenericPlatformManagerImpl<ImplClass> {
protected:
    TimeOut_t mNextTimerBaseTime;
    TickType_t mNextTimerDurationTicks;
    bool mChipTimerActive;

    FuriThread* mEventLoopTask = NULL;
    FuriMutex* mChipStackLock = NULL;
    FuriMessageQueue* mChipEventQueue = NULL;

    CHIP_ERROR _InitChipStack();

    void _LockChipStack(void);
    bool _TryLockChipStack(void);
    void _UnlockChipStack(void);

    CHIP_ERROR _PostEvent(const ChipDeviceEvent* event);
    void _RunEventLoop(void);
    CHIP_ERROR _StartEventLoopTask(void);
    CHIP_ERROR _StopEventLoopTask();
    CHIP_ERROR _StartChipTimer(System::Clock::Timeout duration);
    void _Shutdown(void);

#if CHIP_STACK_LOCK_TRACKING_ENABLED
    bool _IsChipStackLockedByCurrentThread() const;
#endif

    CHIP_ERROR _PostBackgroundEvent(const ChipDeviceEvent* event);

private:
    inline ImplClass* Impl() {
        return static_cast<ImplClass*>(this);
    }

    std::atomic<bool> mShouldRunEventLoop;
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericPlatformManagerImpl_Furi<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
