/**
 * @file sntp.h
 * @brief SNTP (Simple Network Time Protocol) service API.
 */
#pragma once

#include "sntp_settings.h"

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Record name for SNTP service */
#define RECORD_SNTP "sntp"

/** SNTP service opaque type declaration */
typedef struct Sntp Sntp;

/**
 * @brief Update the SNTP synchronization status.
 *
 * This function is used internally to report the result of a time synchronization attempt.
 *
 * @param[in,out] instance pointer to the SNTP service instance
 * @param[in] success true if synchronization succeeded, false otherwise
 */
void sntp_status_update(Sntp* instance, bool success);

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

#ifdef __cplusplus
}
#endif
