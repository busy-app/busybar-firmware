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
 * @brief Get a string value by key.
 *
 * On success, the return value is guaranteed to be valid during
 * the entire run time of the firmware and will never change.
 *
 * This function will fail in the following cases:
 * - The value under the requested key does not exist,
 * - Data has not yet been received from the wireless co-processor (too early or Intercom failure).
 *
 * @param[in] instance Pointer to the SlInfo service instance
 * @param[in] key Pointer to a C-string containing the key
 * @returns Pointer to a C-string containing the value on success, or @c NULL on failure.
 */
const char* sl_info_get_value(const SlInfo* instance, const char* key);

/**
 * @brief Get all available key-value pairs via the Property callback API.
 *
 * @param[in] instance Pointer to the SlInfo service instance
 * @param[in] value_callback Pointer to the output callback function
 * @param[in,out] context Pointer to a user-specific object (will be passed to the value callback)
 * @returns @c true on success, @c false otherwise
 */
bool sl_info_get_values(
    const SlInfo* instance,
    PropertyValueCallback value_callback,
    void* context);

#ifdef __cplusplus
}
#endif
