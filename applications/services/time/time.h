/**
 * @file time.h
 * @brief TIME (Simple Network Time Protocol) service API.
 */
#pragma once

#include "settings/settings.h"

#include <datetime.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Record name for Time service */
#define RECORD_TIME "time"

/** Time service opaque type declaration */
typedef struct Time Time;

/**
 * @brief Get the current time settings.
 *
 * Retrieves the current configuration of the Time service including server address,
 * timezone offset, sync interval, and enabled state.
 *
 * @param[in] instance pointer to the Time service instance
 * @param[out] settings pointer to a structure to be filled with current settings
 */
void time_get_settings(const Time* instance, TimeSettings* settings);

/**
 * @brief Set new time settings.
 *
 * Updates the Time service configuration. Changes take effect immediately and will
 * trigger a reconfiguration of the background synchronization task if the service
 * is enabled.
 *
 * @param[in,out] instance pointer to the Time service instance
 * @param[in] settings pointer to a structure containing the new settings
 * @return true if settings were successfully applied, false otherwise
 */
bool time_set_settings(Time* instance, const TimeSettings* settings);

/**
 * @brief Get Unix seconds timestamp
 * 
 * @return 64-bit Unix seconds timestamp
 */
time_t time_get_timestamp(void);

/**
 * @brief Get date, time, and timezone offset in local timezone
 *
 * @param[in] instance
 * @return local DateTime
 */
LocalTime time_get_local_time(Time* instance);

/**
 * @brief Get Unix milliseconds timestamp
 *
 * @return 64-bit Unix milliseconds timestamp
 */
time_t time_get_timestamp_ms(void);

/**
 * @brief Get FuriState holding TimeSettings.
 *
 * @param[in] instance
 */
FuriState* time_get_settings_state(Time* instance);

#ifdef __cplusplus
}
#endif
