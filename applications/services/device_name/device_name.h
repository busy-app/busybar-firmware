/**
 * @file device_name.h
 * @brief Device Name service API.
 *
 * The Device Name service is responsible for storing and managing
 * the device name. It provides API for getting and setting the name,
 * and publishes rename events via FuriPubSub. If device is linked to
 * account, device name service will send name through MQTT each time
 * the name was updated.
 *
 * The device name is persisted using SettingsProvider.
 */
#pragma once

#include <core/string.h>
#include <core/state.h>

#include "device_name_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The string key for DeviceName instance access.
 */
#define RECORD_DEVICE_NAME "device_name"

/**
 * @brief Opaque data type for the Device Name service instance.
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_DEVICE_NAME);`
 */
typedef struct DeviceName DeviceName;

/**
 * @brief DeviceName information structure.
 */
typedef struct {
    char name[DEVICE_NAME_MAX_SIZE]; /**< Current device name */
} DeviceNameInfo;

/**
 * @brief Validation status returned by device_name_set().
 */
typedef enum {
    DeviceNameErrorNone, /**< Name is valid and was applied successfully */
    DeviceNameErrorEmpty, /**< Name is empty */
    DeviceNameErrorTooLong, /**< Name exceeds the maximum allowed length */
    DeviceNameErrorIllegalChar, /**< Name contains illegal characters */
    DeviceNameErrorOnlySpaces, /**< Name consists only of spaces */
    DeviceNameErrorSaveFailed, /**< Failed to save new name */
    DeviceNameErrorMax, /**< Special value, internal use */
} DeviceNameError;

/**
 * @brief Get current device name
 *
 * @param[in] instance Device name service instance
 * @param[out] name Device name returned by function. Default name will be returned
 * if no name has been set.
 */
void device_name_get(DeviceName* instance, FuriString* name);

/**
 * @brief Set new device name
 *
 * Validates the provided name and, if valid, persists it and publishes it.
 *
 * @param[in] instance Device name service instance
 * @param[in] name New device name to set
 * @return DeviceNameErrorNone on success, error otherwise
 */
DeviceNameError device_name_set(DeviceName* instance, const char* name);

/**
 * @brief Get the DeviceName state object.
 *
 * The return value will be of @ref DeviceNameInfo underlying type.
 *
 * @param[in] instance Device name service instance
 * @returns pointer to the state object
 */
FuriState* device_name_get_state(const DeviceName* instance);

#ifdef __cplusplus
}
#endif
