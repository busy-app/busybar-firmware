/*
 *  Copyright (c) 2025, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file implements the OpenThread platform abstraction for the settings API.
 */

#include <openthread-core-config.h>

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#if defined(SL_CATALOG_NVM3_PRESENT)

extern "C" {
#include "nvm3_default.h"
}

#include "platform-efr32.h"
#include "radio_instance.h"
#include "sl_memory_manager.h"
#include <string.h>
#include <openthread/instance.h>
#include <openthread/platform/settings.h>
#include "common/array.hpp"
#include "common/code_utils.hpp"
#include "common/debug.hpp"
#include "common/logging.hpp"
#include "utils/code_utils.h"

constexpr uint32_t NVM3KEY_DOMAIN_OPENTHREAD = 0x20000U;

// Indexed key types are only supported for kKeyChildInfo (=='child table').
constexpr int NUM_INDEXED_SETTINGS = OPENTHREAD_CONFIG_MLE_MAX_CHILDREN;

// List size used when enumerating nvm3 keys.
constexpr unsigned char ENUM_NVM3_KEY_LIST_SIZE = 4;

using nvm3KeyList = ot::Array<nvm3_ObjectKey_t, ENUM_NVM3_KEY_LIST_SIZE>;

// Static variables
static bool sNvm3OpenedByOT = false;
static int  sInstanceCount  = 0;

// ============================================================================
// Types and Constants
// ============================================================================
class SettingsKey
{
public:
    SettingsKey(otInstance *aInstance, const uint16_t aKey, const uint8_t aIndex)
        : mDomain(NVM3KEY_DOMAIN_OPENTHREAD + getInstanceDomainOffset(aInstance))
        , mKey(aKey)
        , mIndex(aIndex)
    {
    }

    nvm3_ObjectKey_t toNvm3Key() const { return (mDomain | ((uint8_t)mKey << 8) | (mIndex & 0xFF)); }

    static bool isSupportedKey(const uint16_t aKey) { return aKey <= kLastSupportedKeyValue; }

    static SettingsKey getFirstInstanceKey(otInstance *aInstance, const uint16_t aKey)
    {
        return SettingsKey(aInstance, aKey, 0);
    }
    static SettingsKey getFirstInstanceKey(otInstance *aInstance) { return getFirstInstanceKey(aInstance, 0); }
    static SettingsKey getLastInstanceKey(otInstance *aInstance, const uint16_t aKey)
    {
        return SettingsKey(aInstance, aKey, kLastSupportedIndexValue);
    }
    static SettingsKey getLastInstanceKey(otInstance *aInstance)
    {
        return SettingsKey(aInstance, kLastSupportedKeyValue, kLastSupportedIndexValue);
    }

    static int getLastIndex() { return kLastSupportedIndexValue; }

private:
    static constexpr size_t   kInstanceDomainOffset    = 0x2000;
    static constexpr uint16_t kLastSupportedKeyValue   = (kInstanceDomainOffset >> 8) - 1;
    static constexpr int      kLastSupportedIndexValue = NUM_INDEXED_SETTINGS - 1;

    static uint32_t getInstanceDomainOffset(otInstance *aInstance)
    {
        return sli_ot_radio_instance_get_index(aInstance) * kInstanceDomainOffset;
    }

    uint32_t mDomain;
    uint16_t mKey;
    int      mIndex;
};

// ============================================================================
// Error Mapping
// ============================================================================

static otError mapNvm3Error(sl_status_t aNvm3Status)
{
    otError error;

    switch (aNvm3Status)
    {
    case SL_STATUS_OK:
        error = OT_ERROR_NONE;
        break;
    case SL_STATUS_NOT_FOUND:
        error = OT_ERROR_NOT_FOUND;
        break;
    case SL_STATUS_NOT_SUPPORTED:
        error = OT_ERROR_REJECTED;
        break;
    case SL_STATUS_INVALID_PARAMETER:
    case SL_STATUS_NULL_POINTER:
    case SL_STATUS_NOT_INITIALIZED:
        error = OT_ERROR_INVALID_ARGS;
        break;
    default:
        error = OT_ERROR_FAILED;
        break;
    }

    return error;
}

sl_status_t settingsApiCheck(otInstance *aInstance)
{
    sl_status_t status = SL_STATUS_OK;

    otEXPECT_ACTION(sl_ot_rtos_task_can_access_pal(), status = SL_STATUS_NOT_SUPPORTED);
    otEXPECT_ACTION(aInstance != nullptr, status = SL_STATUS_NULL_POINTER);
    otEXPECT_ACTION(otInstanceIsInitialized(aInstance), status = SL_STATUS_NOT_INITIALIZED);

exit:
    return status;
}

sl_status_t settingsApiCheck(otInstance *aInstance, uint16_t aKey)
{
    sl_status_t status = settingsApiCheck(aInstance);

    otEXPECT(status == SL_STATUS_OK);
    otEXPECT_ACTION(SettingsKey::isSupportedKey(aKey), status = SL_STATUS_NOT_SUPPORTED);

exit:
    return status;
}

// ============================================================================
// NVM3 Operations
// ============================================================================

static sl_status_t readNvm3Object(SettingsKey key, uint8_t *buffer, size_t bufferSize, size_t *dataSize)
{
    sl_status_t status = SL_STATUS_FAIL;
    uint32_t    objType;
    size_t      objLen;

    status = nvm3_getObjectInfo(nvm3_defaultHandle, key.toNvm3Key(), &objType, &objLen);
    otEXPECT(status == SL_STATUS_OK);

    if (dataSize != nullptr)
    {
        *dataSize = objLen;
    }

    if (buffer != nullptr && bufferSize > 0)
    {
        status = nvm3_readData(nvm3_defaultHandle, key.toNvm3Key(), buffer, objLen);
    }

exit:
    return status;
}

static sl_status_t readNvm3Object(SettingsKey key, uint8_t *buffer, size_t bufferSize)
{
    return readNvm3Object(key, buffer, bufferSize, nullptr);
}

static sl_status_t writeNvm3Object(SettingsKey key, const uint8_t *data, size_t dataSize)
{
    return nvm3_writeData(nvm3_defaultHandle, key.toNvm3Key(), data, dataSize);
}

static sl_status_t deleteNvm3Object(nvm3_ObjectKey_t key)
{
    return nvm3_deleteObject(nvm3_defaultHandle, key);
}

static sl_status_t deleteNvm3Object(SettingsKey key)
{
    return deleteNvm3Object(key.toNvm3Key());
}

static sl_status_t getNvm3ObjectSize(SettingsKey key, size_t *dataSize)
{
    return readNvm3Object(key, nullptr, 0, dataSize);
}

static bool isNvm3ObjectEmpty(SettingsKey key)
{
    size_t objLen = 0;
    return getNvm3ObjectSize(key, &objLen) == SL_STATUS_NOT_FOUND;
}

static size_t getNvm3ObjectRange(SettingsKey                                           startKey,
                                 SettingsKey                                           endKey,
                                 ot::Array<nvm3_ObjectKey_t, ENUM_NVM3_KEY_LIST_SIZE> &keys,
                                 size_t                                                count)
{
    return nvm3_enumObjects(nvm3_defaultHandle, keys.GetArrayBuffer(), count, startKey.toNvm3Key(), endKey.toNvm3Key());
}

static size_t getNvm3ObjectRange(SettingsKey                                           startKey,
                                 SettingsKey                                           endKey,
                                 ot::Array<nvm3_ObjectKey_t, ENUM_NVM3_KEY_LIST_SIZE> &keys)
{
    return getNvm3ObjectRange(startKey, endKey, keys, ENUM_NVM3_KEY_LIST_SIZE);
}

static sl_status_t checkSettingExists(SettingsKey settingsKey)
{
    size_t objLen = 0;

    return getNvm3ObjectSize(settingsKey, &objLen);
}
// ============================================================================
// Settings Operations
// ============================================================================

static sl_status_t deleteSettingsRange(SettingsKey startKey, SettingsKey endKey)
{
    sl_status_t status = SL_STATUS_OK;
    nvm3KeyList keys;

    size_t objCount = getNvm3ObjectRange(startKey, endKey, keys);
    otEXPECT_ACTION(objCount > 0, status = SL_STATUS_NOT_FOUND);

    do
    {
        size_t i = 0;
        do
        {
            status = deleteNvm3Object(keys[static_cast<nvm3KeyList::IndexType>(i)]);
            ++i;
        } while (i < objCount && status == SL_STATUS_OK);
        objCount = getNvm3ObjectRange(startKey, endKey, keys);
    } while (objCount > 0 && status == SL_STATUS_OK);

exit:
    return status;
}

static sl_status_t deleteAllSettingsForKey(uint16_t key, otInstance *aInstance)
{
    SettingsKey startKey = SettingsKey::getFirstInstanceKey(aInstance, key);
    SettingsKey endKey   = SettingsKey::getLastInstanceKey(aInstance, key);

    return deleteSettingsRange(startKey, endKey);
}

static sl_status_t deleteSettingByIndex(uint16_t key, int index, otInstance *aInstance)
{
    SettingsKey settingsKey(aInstance, key, static_cast<uint8_t>(index));

    return deleteNvm3Object(settingsKey);
}

static sl_status_t addSetting(uint16_t key, const uint8_t *value, uint16_t valueLength, otInstance *aInstance)
{
    sl_status_t status         = SL_STATUS_FAIL;
    bool        availableIndex = false;
    uint8_t     idx            = 0;
    SettingsKey settingsKey(aInstance, key, 0);

    otEXPECT_ACTION((value != nullptr) && (valueLength != 0), status = SL_STATUS_INVALID_PARAMETER);

    // Iterate from first to last instance key for this key, looking for the first available slot
    while (idx < SettingsKey::getLastIndex() && availableIndex == false)
    {
        settingsKey = SettingsKey(aInstance, key, idx);
        if (isNvm3ObjectEmpty(settingsKey))
        {
            status         = writeNvm3Object(settingsKey, value, valueLength);
            availableIndex = true;
        }
        idx++;
    }

exit:
    return status;
}

static sl_status_t readSetting(SettingsKey settingsKey, uint8_t *value, uint16_t *valueLength)
{
    sl_status_t status   = SL_STATUS_FAIL;
    size_t      dataSize = 0;
    size_t      copySize = 0;
    uint8_t    *buffer   = nullptr;

    OT_ASSERT(value != nullptr && valueLength != nullptr);

    status = getNvm3ObjectSize(settingsKey, &dataSize);
    otEXPECT(status == SL_STATUS_OK);
    otEXPECT_ACTION(dataSize > 0, status = SL_STATUS_NOT_FOUND);

    status = sl_memory_alloc(dataSize, BLOCK_TYPE_LONG_TERM, (void **)&buffer);
    otEXPECT(status == SL_STATUS_OK);
    otEXPECT_ACTION(buffer != nullptr, status = SL_STATUS_FAIL);

    status = readNvm3Object(settingsKey, buffer, dataSize);
    otEXPECT(status == SL_STATUS_OK);

    copySize = (dataSize < *valueLength) ? dataSize : *valueLength;
    memcpy(value, buffer, copySize);
    *valueLength = static_cast<uint16_t>(dataSize);

exit:
    if (buffer != nullptr)
    {
        sl_free(buffer);
    }
    return status;
}

// ============================================================================
// Public API Implementation
// ============================================================================

// Sensitive keys are not handled specifically in these platform settings.
// NVM3 is encrypted, so there is no need for a separate secure storage interface.
void otPlatSettingsInit(otInstance *aInstance, const uint16_t *aSensitiveKeys, uint16_t aSensitiveKeysLength)
{
    OT_UNUSED_VARIABLE(aSensitiveKeys);
    OT_UNUSED_VARIABLE(aSensitiveKeysLength);

    sl_status_t status = settingsApiCheck(aInstance);

    otEXPECT_ACTION(status == SL_STATUS_OK, otLogDebgPlat("Error initializing settings: %lu", status));

    sInstanceCount++;

    otEXPECT(!nvm3_defaultHandle->hasBeenOpened);
    status = nvm3_open(nvm3_defaultHandle, nvm3_defaultInit);

    otEXPECT_ACTION(status == SL_STATUS_OK, otLogDebgPlat("Error initializing nvm3 instance: %lu", status));
    sNvm3OpenedByOT = true;

exit:
    return;
}

void otPlatSettingsDeinit(otInstance *aInstance)
{
    sl_status_t status = settingsApiCheck(aInstance);
    otEXPECT_ACTION(status == SL_STATUS_OK, otLogDebgPlat("Error deinitializing settings: %lu", status));

    sInstanceCount--;
    otEXPECT(sNvm3OpenedByOT && sInstanceCount == 0);

    nvm3_close(nvm3_defaultHandle);
    sNvm3OpenedByOT = false;

exit:
    return;
}

otError otPlatSettingsGet(otInstance *aInstance, uint16_t aKey, int aIndex, uint8_t *aValue, uint16_t *aValueLength)
{
    sl_status_t status = settingsApiCheck(aInstance, aKey);
    SettingsKey settingsKey(aInstance, aKey, static_cast<uint8_t>(aIndex));

    otEXPECT(status == SL_STATUS_OK);

    if (aValue != nullptr && aValueLength != nullptr)
    {
        status = readSetting(settingsKey, aValue, aValueLength);
    }
    else
    {
        status = checkSettingExists(settingsKey);
    }

exit:
    return mapNvm3Error(status);
}

otError otPlatSettingsSet(otInstance *aInstance, uint16_t aKey, const uint8_t *aValue, uint16_t aValueLength)
{
    sl_status_t status = settingsApiCheck(aInstance, aKey);
    otEXPECT(status == SL_STATUS_OK);

    // Delete existing settings for this key
    status = deleteAllSettingsForKey(aKey, aInstance);
    otEXPECT((status == SL_STATUS_OK) || (status == SL_STATUS_NOT_FOUND));

    status = addSetting(aKey, aValue, aValueLength, aInstance);

exit:
    return mapNvm3Error(status);
}

otError otPlatSettingsAdd(otInstance *aInstance, uint16_t aKey, const uint8_t *aValue, uint16_t aValueLength)
{
    sl_status_t status = settingsApiCheck(aInstance, aKey);
    otEXPECT(status == SL_STATUS_OK);

    status = addSetting(aKey, aValue, aValueLength, aInstance);

exit:
    return mapNvm3Error(status);
}

otError otPlatSettingsDelete(otInstance *aInstance, uint16_t aKey, int aIndex)
{
    sl_status_t status = settingsApiCheck(aInstance, aKey);
    otEXPECT(status == SL_STATUS_OK);

    if (aIndex >= 0)
    {
        status = deleteSettingByIndex(aKey, aIndex, aInstance);
    }
    else
    {
        status = deleteAllSettingsForKey(aKey, aInstance);
    }

exit:
    return mapNvm3Error(status);
}

void otPlatSettingsWipe(otInstance *aInstance)
{
    sl_status_t status = settingsApiCheck(aInstance);
    otEXPECT_ACTION(status == SL_STATUS_OK, otLogDebgPlat("Error wiping settings: %lu", status));

    deleteSettingsRange(SettingsKey::getFirstInstanceKey(aInstance), SettingsKey::getLastInstanceKey(aInstance));

exit:
    return;
}

#if TESTING
// ============================================================================
// Test Utilities
// ============================================================================
namespace Testing {
namespace Settings {
void ResetState()
{
    sInstanceCount  = 0;
    sNvm3OpenedByOT = false;
    memset(nvm3_defaultHandle, 0, sizeof(nvm3_Handle_t));
}
} // namespace Settings
} // namespace Testing
#endif // TESTING

#endif // SL_CATALOG_NVM3_PRESENT
