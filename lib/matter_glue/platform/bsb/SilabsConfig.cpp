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
 *          Utilities for accessing persisted device configuration on
 *          platforms based on the Silicon Labs SDK.
 */
#include "SilabsConfig.h"

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>
#include <platform/internal/testing/ConfigUnitTest.h>
#include <platform/silabs/CHIPDevicePlatformConfig.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR SilabsConfig::Init() {
    mNvmInstance = static_cast<Nvm*>(furi_record_open(RECORD_NVM));
    return CHIP_NO_ERROR;
}

void SilabsConfig::DeInit() {
    mNvmInstance = nullptr;
    furi_record_close(RECORD_NVM);
}

CHIP_ERROR SilabsConfig::ReadConfigValue(Key key, bool& val) {
    return ReadConfigValueHelper(key, val);
}

CHIP_ERROR SilabsConfig::ReadConfigValue(Key key, uint16_t& val) {
    return ReadConfigValueHelper(key, val);
}

CHIP_ERROR SilabsConfig::ReadConfigValue(Key key, uint32_t& val) {
    return ReadConfigValueHelper(key, val);
}

CHIP_ERROR SilabsConfig::ReadConfigValue(Key key, uint64_t& val) {
    return ReadConfigValueHelper(key, val);
}

CHIP_ERROR SilabsConfig::ReadConfigValueStr(Key key, char* buf, size_t bufSize, size_t& outLen) {
    CHIP_ERROR err = CHIP_NO_ERROR;

    outLen = 0;
    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    size_t dataLen;
    VerifyOrExit(
        nvm_exists(mNvmInstance, key, &dataLen), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    VerifyOrExit(dataLen > 0, err = CHIP_ERROR_INVALID_STRING_LENGTH);

    if(buf != NULL) {
        VerifyOrExit((bufSize > dataLen), err = CHIP_ERROR_BUFFER_TOO_SMALL);
        VerifyOrExit(nvm_read(mNvmInstance, key, buf, dataLen), err = CHIP_ERROR_INTERNAL);

        outLen = ((dataLen == 1) && (buf[0] == 0)) ? 0 : dataLen;
        buf[outLen] = 0;

    } else {
        if(dataLen > 1) {
            outLen = dataLen;
        } else {
            // Read the first byte of the nvm3 string into a tmp var.
            char firstByte;
            VerifyOrExit(nvm_read(mNvmInstance, key, &firstByte, 1), err = CHIP_ERROR_INTERNAL);

            outLen = (firstByte == 0) ? 0 : dataLen;
        }
    }

exit:
    return err;
}

CHIP_ERROR SilabsConfig::ReadConfigValueBin(
    Key key,
    uint8_t* buf,
    size_t bufSize,
    size_t& outLen,
    size_t offset) {
    CHIP_ERROR err = CHIP_NO_ERROR;

    outLen = 0;
    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    size_t dataLen;
    VerifyOrExit(
        nvm_exists(mNvmInstance, key, &dataLen), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    if(buf != NULL) {
        // Read nvm3 bytes directly into output buffer- check buffer is long enough to take the data
        // else read what we can but return CHIP_ERROR_BUFFER_TOO_SMALL.
        size_t maxReadLength = dataLen - offset;

        if(bufSize >= maxReadLength) {
            VerifyOrExit(
                nvm_read_partial(mNvmInstance, key, buf, offset, maxReadLength),
                err = CHIP_ERROR_INTERNAL);
            outLen = maxReadLength;

        } else {
            VerifyOrExit(
                nvm_read_partial(mNvmInstance, key, buf, offset, bufSize),
                err = CHIP_ERROR_INTERNAL);
            // read was successful, but we did not read all the data from the object.
            err = CHIP_ERROR_BUFFER_TOO_SMALL;
            outLen = bufSize;
        }
    }

exit:
    return err;
}

CHIP_ERROR SilabsConfig::ReadConfigValueCounter(uint8_t counterIdx, uint32_t& val) {
    CHIP_ERROR err = CHIP_NO_ERROR;

    const Key key = kMinConfigKey_MatterCounter + counterIdx;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    VerifyOrExit(nvm_read_counter(mNvmInstance, key, &val), err = CHIP_ERROR_INTERNAL);

exit:
    return err;
}

CHIP_ERROR SilabsConfig::WriteConfigValue(Key key, bool val) {
    return WriteConfigValueHelper(key, val);
}

CHIP_ERROR SilabsConfig::WriteConfigValue(Key key, uint16_t val) {
    return WriteConfigValueHelper(key, val);
}

CHIP_ERROR SilabsConfig::WriteConfigValue(Key key, uint32_t val) {
    return WriteConfigValueHelper(key, val);
}

CHIP_ERROR SilabsConfig::WriteConfigValue(Key key, uint64_t val) {
    return WriteConfigValueHelper(key, val);
}

CHIP_ERROR SilabsConfig::WriteConfigValueStr(Key key, const char* str, size_t strLen) {
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    if(str != NULL) {
        if(strLen == 0) {
            strLen = strlen(str);
        }

        VerifyOrExit(
            nvm_write(mNvmInstance, key, str, (strLen > 0) ? strLen : 1),
            err = CHIP_ERROR_INTERNAL);
    }

exit:
    return err;
}

CHIP_ERROR SilabsConfig::WriteConfigValueBin(Key key, const uint8_t* data, size_t dataLen) {
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    if((data != NULL) || (dataLen == 0)) {
        VerifyOrExit(nvm_write(mNvmInstance, key, data, dataLen), err = CHIP_ERROR_INTERNAL);
    } else {
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

exit:
    return err;
}

CHIP_ERROR SilabsConfig::WriteConfigValueCounter(uint8_t counterIdx, uint32_t val) {
    CHIP_ERROR err = CHIP_NO_ERROR;

    const Key key = kMinConfigKey_MatterCounter + counterIdx;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    VerifyOrExit(nvm_write_counter(mNvmInstance, key, val), err = CHIP_ERROR_INTERNAL);

exit:
    return err;
}

CHIP_ERROR SilabsConfig::ClearConfigValue(Key key) {
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(nvm_delete(mNvmInstance, key), err = CHIP_ERROR_INTERNAL);

exit:
    return err;
}

bool SilabsConfig::ConfigValueExists(Key key) {
    return nvm_exists(mNvmInstance, key, nullptr);
}

bool SilabsConfig::ConfigValueExists(Key key, size_t& dataLen) {
    return nvm_exists(mNvmInstance, key, &dataLen);
}

CHIP_ERROR SilabsConfig::FactoryResetConfig(void) {
    for(Key k = kMinConfigKey_MatterConfig; k <= kMaxConfigKey_MatterConfig; ++k) {
        ClearConfigValue(k);
    }

    return CHIP_NO_ERROR;
}

bool SilabsConfig::ValidConfigKey(Key key) {
    // Returns true if the key is in the Matter nvm3 reserved key range.
    // or if the key is in the User Domain key range
    // Additional check validates that the user consciously defined the expected key range
    if(((key >= kMatterNvm3KeyLoLimit) && (key <= kMatterNvm3KeyHiLimit) &&
        (key >= kMinConfigKey_MatterFactory) && (key <= kMaxConfigKey_MatterKvs)) ||
       ((key >= kUserNvm3KeyDomainLoLimit) && (key <= kUserNvm3KeyDomainHiLimit))) {
        return true;
    }

    return false;
}

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
void SilabsConfig::RunConfigUnitTest() {
    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<SilabsConfig>();
}
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

void SilabsConfig::RepackNvm3Flash(void) {
    nvm_repack(mNvmInstance);
}

template <typename T>
CHIP_ERROR SilabsConfig::ReadConfigValueHelper(Key key, T& val) {
    VerifyOrReturnError(ValidConfigKey(key), CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    size_t dataLen;
    VerifyOrReturnError(
        nvm_exists(mNvmInstance, key, &dataLen), CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    VerifyOrReturnError(dataLen == sizeof(T), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(nvm_read(mNvmInstance, key, &val, dataLen), CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR SilabsConfig::WriteConfigValueHelper(Key key, const T& val) {
    VerifyOrReturnError(ValidConfigKey(key), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(nvm_write(mNvmInstance, key, &val, sizeof(T)), CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

Nvm* SilabsConfig::mNvmInstance = nullptr;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
