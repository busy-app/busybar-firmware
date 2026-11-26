/**
 * @file low_power.h
 * @brief Low power mode API.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>

#define RECORD_LOW_POWER "low_power"

typedef struct LowPower LowPower;

/**
 * @brief Low-power sleep mode
 */
typedef enum {
    LowPowerModeSleepAfterHoldoff, //<! Enter sleep mode after locks have been lifted and a set time expires
    LowPowerModeDisabled, //<! Never enter sleep mode
    LowPowerModeMAX,
} LowPowerMode;

/**
 * @brief Complete low-power sleep settings structure
 */
typedef struct {
    LowPowerMode mode; //<! See @c LowPowerMode
    uint32_t
        holdoff_duration; //<! Amount of OS ticks to wait after all locks have been lifted before entering sleep (for `LowPowerModeSleepAfterHoldoff`)
} LowPowerSleepSettings;

/**
 * @brief Lock low power mode entry.
 *
 * @param[in] instance LowPower service instance
 */
void low_power_lock(LowPower* instance);

/**
 * @brief Unlock low power mode entry. If nobody else is locking entry either,
 * wait for the holdoff period to pass (see
 * @c low_power_set_sleep_holdoff_duration) and enter low power mode.
 *
 * @param[in] instance LowPower service instance
 */
void low_power_unlock(LowPower* instance);

/**
 * @brief Gets the sleep settings
 * 
 * @param[in] instance LowPower service instance
 * @param[out] settings Sleep settings structure
 */
void low_power_get_sleep_settings(LowPower* instance, LowPowerSleepSettings* settings);

/**
 * @brief Sets the sleep settings
 * 
 * @param[in] instance LowPower service instance
 * @param[in] settings Sleep settings structure
 */
void low_power_set_sleep_settings(LowPower* instance, LowPowerSleepSettings* settings);

#ifdef __cplusplus
}
#endif
