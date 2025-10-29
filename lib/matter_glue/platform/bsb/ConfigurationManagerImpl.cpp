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
 *          Provides the implementation of the Device Layer ConfigurationManager object
 *          for Silabs platforms using the Silicon Labs SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/ConfigurationManager.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#include "SilabsConfig.h"

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ConfigurationManagerImpl& ConfigurationManagerImpl::GetDefaultInstance() {
    static ConfigurationManagerImpl sInstance;
    return sInstance;
}

CHIP_ERROR ConfigurationManagerImpl::Init() {
    CHIP_ERROR err;

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<SilabsConfig>::Init();
    SuccessOrExit(err);

    IncreaseBootCount();
    err = CHIP_NO_ERROR;

exit:
    return err;
}

bool ConfigurationManagerImpl::CanFactoryReset() {
    // TODO: query the application to determine if factory reset is allowed.
    return true;
}

void ConfigurationManagerImpl::InitiateFactoryReset() {
    PlatformMgr().ScheduleWork(DoFactoryReset);
}

CHIP_ERROR ConfigurationManagerImpl::GetRebootCount(uint32_t& rebootCount) {
    return SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_BootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::IncreaseBootCount(void) {
    uint32_t bootCount = 0;

    if(SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_BootCount)) {
        GetRebootCount(bootCount);
    }

    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_BootCount, bootCount + 1);
}

CHIP_ERROR ConfigurationManagerImpl::GetBootReason(uint32_t& bootReason) {
    bootReason = to_underlying(BootReasonType::kUnspecified);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigurationManagerImpl::GetTotalOperationalHours(uint32_t& totalOperationalHours) {
    if(!SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_TotalOperationalHours)) {
        totalOperationalHours = 0;
        return CHIP_NO_ERROR;
    }

    return SilabsConfig::ReadConfigValue(
        SilabsConfig::kConfigKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::StoreTotalOperationalHours(uint32_t totalOperationalHours) {
    return SilabsConfig::WriteConfigValue(
        SilabsConfig::kConfigKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::ReadPersistedStorageValue(
    ::chip::Platform::PersistedStorage::Key persistedStorageKey,
    uint32_t& value) {
    // This method reads CHIP Persisted Counter type nvm3 objects.
    // (where persistedStorageKey represents an index to the counter).
    CHIP_ERROR err;

    err = SilabsConfig::ReadConfigValueCounter(persistedStorageKey, value);
    if(err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND) {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::WritePersistedStorageValue(
    ::chip::Platform::PersistedStorage::Key persistedStorageKey,
    uint32_t value) {
    // This method reads CHIP Persisted Counter type nvm3 objects.
    // (where persistedStorageKey represents an index to the counter).
    CHIP_ERROR err;

    err = SilabsConfig::WriteConfigValueCounter(persistedStorageKey, value);
    if(err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND) {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool& val) {
    return SilabsConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t& val) {
    return SilabsConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t& val) {
    return SilabsConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(
    Key key,
    char* buf,
    size_t bufSize,
    size_t& outLen) {
    return SilabsConfig::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(
    Key key,
    uint8_t* buf,
    size_t bufSize,
    size_t& outLen) {
    return SilabsConfig::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val) {
    return SilabsConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val) {
    return SilabsConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val) {
    return SilabsConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char* str) {
    return SilabsConfig::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char* str, size_t strLen) {
    return SilabsConfig::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR
ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t* data, size_t dataLen) {
    return SilabsConfig::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest(void) {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    SilabsConfig::RunConfigUnitTest();
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg) {
    UNUSED(arg);

    ChipLogProgress(DeviceLayer, "Performing factory reset");

    const auto err = SilabsConfig::FactoryResetConfig();

    if(!CHIP_ERROR::IsSuccess(err)) {
        ChipLogError(DeviceLayer, "Factory reset failed: %s", chip::ErrorStr(err));
    }

    PersistedStorage::KeyValueStoreMgrImpl().ErasePartition();
}

ConfigurationManager& ConfigurationMgrImpl() {
    return ConfigurationManagerImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
