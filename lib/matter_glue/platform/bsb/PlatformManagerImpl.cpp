/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          Provides an implementation of the PlatformManager object
 *          for Silabs platforms using the Silicon Labs SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <crypto/CHIPCryptoPAL.h>

#include <platform/KeyValueStoreManager.h>
#include <platform/PlatformManager.h>

#include <lib/support/CHIPPlatformMemory.h>

#include <mbedtls/platform.h>

#include "DiagnosticDataProviderImpl.h"
#include "SystemTimeSupport.h"

#if defined(SL_MBEDTLS_USE_TINYCRYPT)
#include "tinycrypt/ecc.h"
#endif // SL_MBEDTLS_USE_TINYCRYPT

using namespace chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

#if defined(SL_MBEDTLS_USE_TINYCRYPT)
FuriMutex* PlatformManagerImpl::rngMutexHandle = nullptr;

int PlatformManagerImpl::uECC_RNG_Function(uint8_t* dest, unsigned int size) {
    furi_mutex_acquire(rngMutexHandle, FuriWaitForever);
    const int res = (chip::Crypto::DRBG_get_bytes(dest, size) == CHIP_NO_ERROR) ? size : 0;
    furi_mutex_release(rngMutexHandle);

    return res;
}
#endif // SL_MBEDTLS_USE_TINYCRYPT

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void) {
    CHIP_ERROR err;
    // Initialize the configuration system.
    err = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();
    SuccessOrExit(err);

    ReturnErrorOnFailure(System::Clock::InitClock_RealTime());

#if defined(SL_MBEDTLS_USE_TINYCRYPT)
    /* Set RNG function for tinycrypt operations. */
    rngMutexHandle = furi_mutex_alloc(FuriMutexTypeNormal);
    uECC_set_rng(PlatformManagerImpl::uECC_RNG_Function);
#endif // SL_MBEDTLS_USE_TINYCRYPT

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_Furi<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

    PlatformMgr().LockChipStack();
    // Start timer to increment TotalOperationalHours every hour
    SystemLayer().StartTimer(
        System::Clock::Seconds32(kSecondsPerHour), UpdateOperationalHours, NULL);
    PlatformMgr().UnlockChipStack();

exit:
    return err;
}

void PlatformManagerImpl::UpdateOperationalHours(System::Layer* systemLayer, void* appState) {
    uint32_t totalOperationalHours = 0;

    if(ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours) == CHIP_NO_ERROR) {
        ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours + 1);
    } else {
        ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
    }

    SystemLayer().StartTimer(
        System::Clock::Seconds32(kSecondsPerHour), UpdateOperationalHours, NULL);
}

void PlatformManagerImpl::_Shutdown() {
    Internal::GenericPlatformManagerImpl_Furi<PlatformManagerImpl>::_Shutdown();
}

} // namespace DeviceLayer
} // namespace chip
