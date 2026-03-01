/**
 * @file sl_info.h
 * @brief Wireless co-processor (Si917) information service API.
 *
 * The SlInfo service keeps the static copy of the device_info output in a searchable form,
 * allowing key-value lookups for particular values or outputting them all at once
 * via the Property callback API.
 */
#pragma once

#include <toolbox/property.h>

/**
 * @brief The string key for SlInfo instance access
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_SL_INFO);`
 */
#define RECORD_SL_INFO "sl_info"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque data type for the SlInfo service instance.
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_SL_INFO);`
 */
typedef struct SlInfo SlInfo;

/**
 * @brief Enumeration of possible return statuses used in getter functions.
 */
typedef enum {
    SlInfoStatusOk, /**< Success, data was found and/or is valid */
    SlInfoStatusNotReady, /**< Data is not ready (too early or Intercom failure) */
    SlInfoStatusNotFound, /**< Value was not found (wrong key) */
} SlInfoStatus;

/**
 * @brief Get a string value by key.
 *
 * On success, the value pointer is guaranteed to be valid during
 * the entire run time of the firmware and the data itself will never change.
 *
 * @note To get the list of available keys, run `device_info` inside `sl_cli`.
 *
 * @param[in] instance Pointer to the SlInfo service instance
 * @param[in] key Pointer to a C-string containing the key
 * @param[out] value Pointer to a pointer to a C-string to contain the value (no allocation needed)
 * @returns @c SlInfoStatusOk on success, any other value from SlInfoStatus otherwise
 */
SlInfoStatus sl_info_get_value(const SlInfo* instance, const char* key, const char** value);

/**
 * @brief Get all available key-value pairs via the Property callback API.
 *
 * @param[in] instance Pointer to the SlInfo service instance
 * @param[in] value_callback Pointer to the output callback function
 * @param[in,out] context Pointer to a user-specific object (will be passed to the value callback)
 * @returns @c SlInfoStatusOk on success, any other value from SlInfoStatus otherwise
 */
SlInfoStatus
    sl_info_get_values(const SlInfo* instance, PropertyValueCallback value_callback, void* context);

#ifdef __cplusplus
}
#endif
