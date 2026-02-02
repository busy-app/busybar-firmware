#pragma once

#include <furi.h>

#define RECORD_DEVICE_NAME "device_name"

typedef struct DeviceName DeviceName;

/**
 * @brief Get current device name
 *
 *
 * @param[in] instance of device name record
 * @param[out] name device name returned by function. Default name will be returned 
 * if failed to read name for some reason
 */
void device_name_get(DeviceName* instance, FuriString* name);

/**
 * @brief Set new device name
 *
 *
 * @param[in] instance of device name record
 * @param[in] name new device name
 * @param[out] error if present, contains error message for failure case, when NULL ignored
 * @returns true - on success, false - when failed to set name. More details in error string
 */
bool device_name_set(DeviceName* instance, FuriString* name, FuriString* error);

/**
 * @brief Get PubSub instance which indicates that name was changed
 *
 *
 * @param[in] instance of device name record
 * @returns pubsub instance available for subscription
 */
FuriPubSub* device_name_get_pubsub(DeviceName* instance);
