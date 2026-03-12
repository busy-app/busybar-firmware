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
 * @brief Device name event type (published via FuriPubSub on rename)
 */
typedef struct {
    const char* name; /**< New device name (valid only within callback context) */
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
 * @param[in] instance Device name service instance
 * @param[in] name New device name
 * @param[out] error If present, contains error message for failure case, when NULL ignored
 * @returns true on success, false when failed to set name. More details in error string
 */
bool device_name_set(DeviceName* instance, FuriString* name, FuriString* error);

/**
 * @brief Get PubSub instance which indicates that name was changed
 *
 * @param[in] instance Device name service instance
 * @returns pubsub instance available for subscription
 */
FuriPubSub* device_name_get_pubsub(DeviceName* instance);

#ifdef __cplusplus
}
#endif
