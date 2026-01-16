/**
 * @file sntp.h
 * @brief SNTP (Simple Network Time Protocol) service API.
 */
#pragma once

#include "settings/settings.h"

#include <furi.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Record name for SNTP service */
#define RECORD_SNTP "sntp"

/** SNTP service opaque type declaration */
typedef struct Sntp Sntp;

/**
 * @brief Get the current SNTP settings.
 *
 * Retrieves the current configuration of the SNTP service including server address,
 * timezone offset, sync interval, and enabled state.
 *
 * @param[in] instance pointer to the SNTP service instance
 * @param[out] settings pointer to a structure to be filled with current settings
 */
void sntp_get_settings(const Sntp* instance, SntpSettings* settings);

/**
 * @brief Set new SNTP settings.
 *
 * Updates the SNTP service configuration. Changes take effect immediately and will
 * trigger a reconfiguration of the background synchronization task if the service
 * is enabled.
 *
 * @param[in,out] instance pointer to the SNTP service instance
 * @param[in] settings pointer to a structure containing the new settings
 * @return true if settings were successfully applied, false otherwise
 */
bool sntp_set_settings(Sntp* instance, const SntpSettings* settings);

/**
 * @brief Get local Unix seconds timestamp
 * 
 * @param[in] instance
 * @return 64-bit local Unix seconds timestamp
 */
time_t sntp_get_local_timestamp(Sntp* instance);

/**
 * @brief Get local Unix milliseconds timestamp
 *
 * @param[in] instance
 * @return 64-bit local Unix milliseconds timestamp
 */
time_t sntp_get_local_timestamp_ms(Sntp* instance);

#ifdef __cplusplus
}
#endif
