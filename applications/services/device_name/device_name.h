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

#include <furi.h>

#include "device_name_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_DEVICE_NAME "device_name"

/**
 * @brief Opaque data type for the Device Name service instance.
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_DEVICE_NAME);`
 */
typedef struct DeviceName DeviceName;

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
 * @brief Device name event type (published via FuriPubSub on rename)
 */
typedef enum {
    DeviceNameEventTypeNameChanged,
} DeviceNameEventType;

typedef struct {
    const char* name;
} DeviceNameEventNameChanged;

typedef struct {
    DeviceNameEventType type;
    union {
        DeviceNameEventNameChanged name_changed;
    };
} DeviceNameEvent;

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
DeviceNameError device_name_set(DeviceName* instance, const FuriString* name);

/**
 * @brief Get PubSub instance which indicates that name was changed

 * Use furi_pubsub_subscribe() to subscribe to the Device Name service events.
 * The delivered events will be of type DeviceNameEvent.
 *
 * @param[in] instance Device name service instance
 * @returns pubsub instance available for subscription
 */
FuriPubSub* device_name_get_pubsub(DeviceName* instance);

#ifdef __cplusplus
}
#endif
