/**
 * @file low_power.h
 * @brief Low power mode API.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_LOW_POWER "low_power"

typedef struct LowPower LowPower;

/**
 * @brief Lock low power mode entry. If low power mode is already active, exit it.
 *
 * @param[in] instance LowPower service instance
 *
 */
void low_power_lock(LowPower* instance);

/**
 * @brief Unlock low power mode entry. Enter low power mode if nobody is locking it anymore.
 *
 * @param[in] instance LowPower service instance
 *
 */
void low_power_unlock(LowPower* instance);

#ifdef __cplusplus
}
#endif
