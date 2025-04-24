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

bool bq25798_get_charger_status(FuriHalI2cBusHandle* handle, Bq25798ChargerStatus* status);

bool bq25798_get_charger_fault(FuriHalI2cBusHandle* handle, Bq25798ChargerFault* fault);

bool bq25798_get_charger_irq_flags(FuriHalI2cBusHandle* handle, uint32_t* flags);

bool bq25798_get_adc_values(FuriHalI2cBusHandle* handle, Bq25798AdcValues* values);

bool bq25798_set_input_current_limit(FuriHalI2cBusHandle* handle, uint32_t value_ma);

bool bq25798_set_charge_current_limit(FuriHalI2cBusHandle* handle, uint32_t value_ma);

bool bq25798_set_charge_voltage_limit(FuriHalI2cBusHandle* handle, uint32_t value_mv);

bool bq25798_charge_enable(FuriHalI2cBusHandle* handle, bool enabled);

void bq25798_power_switch(FuriHalI2cBusHandle* handle, Bq25798PowerSwitch mode);

#ifdef __cplusplus
}
#endif
