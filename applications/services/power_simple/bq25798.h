#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "bq25798_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FuriHalI2cBusHandle FuriHalI2cBusHandle;

/** Initialize charger
 * 
 * Check device presence
 *
 * @param      handle  The I2C bus handle
 *
 * @return     true on success
 */
bool bq25798_init(FuriHalI2cBusHandle* handle);

/** Reset charger
 *
 * @param      handle  The I2C bus handle
 *
 * @return     true on success
 */
bool bq25798_reset(FuriHalI2cBusHandle* handle);

bool bq25798_set_cfg(FuriHalI2cBusHandle* handle);

bool bq25798_get_charger_status(FuriHalI2cBusHandle* handle, Bq25987ChargerStatus* status);

bool bq25798_get_charger_flags(FuriHalI2cBusHandle* handle, uint32_t* flags);

bool bq25798_get_battery_voltage(FuriHalI2cBusHandle* handle, float* value);

bool bq25798_get_vbus_voltage(FuriHalI2cBusHandle* handle, float* value);

bool bq25798_set_input_current_limit(FuriHalI2cBusHandle* handle, float value);

bool bq25798_set_charge_current_limit(FuriHalI2cBusHandle* handle, float value);

void bq25798_power_switch(FuriHalI2cBusHandle* handle, Bq25987PowerSwitch mode);

#ifdef __cplusplus
}
#endif
