/**
 * @file device_info.h
 * Device Info service. Aggregates info publishers and provides access to them
 * to info queriers.
 */

#pragma once

#include <furi.h>
#include <toolbox/property.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DeviceInfo DeviceInfo;

/**
 * Type `DeviceInfo*`
 */
#define RECORD_DEVICE_INFO "device_info"

typedef void (*DeviceInfoCallback)(
    PropertyValueCallback print_callback,
    char separator,
    void* info_context,
    void* print_context);

/**
 * Register a callback that provides a piece of information about the system.
 * 
 * @param[inout] dev_info Device info service
 * @param[in] callback Function that will be called when system information is requested
 * @param[inout] context Passed verbatim to `callback` parameter. May be NULL.
 */
void device_info_register_segment(DeviceInfo* dev_info, DeviceInfoCallback callback, void* context);

/**
 * De-register a callback that provides a piece of information about the system.
 * 
 * @param[inout] dev_info Device info service
 * @param[in] callback Function that will no longer be called when system information is requested
 */
void device_info_unregister_segment(DeviceInfo* dev_info, DeviceInfoCallback callback);

/**
 * Query all registered information about the system
 * 
 * @param[inout] dev_info Device info service
 * @param[in] callback Function that will be called for each key-value pair of system information
 * @param[in] separator Placed in between parts of the key
 * @param[inout] context Passed verbatim to `callback` parameter. May be NULL.
 */
void device_info_query(
    DeviceInfo* dev_info,
    PropertyValueCallback callback,
    char separator,
    void* context);

#ifdef __cplusplus
}
#endif
