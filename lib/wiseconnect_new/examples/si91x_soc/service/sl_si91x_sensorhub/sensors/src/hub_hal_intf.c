/***************************************************************************/ /**
* @file hub_hal_intf.c
* @brief Sensor HUB and Peripherals HAL interface example
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 *    If you use this software In in a product,
 *    an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
/**=============================================================================
 * @brief : This file contains the Sensor hub and Peripherals Hal interface file
 * for Sensor Hub application
 * This example file uses simple communication APIs between sensor hub and
 * Peripherals Hal .
 ============================================================================**/
#include "hub_hal_intf.h"
#include "rsi_error.h"

#pragma GCC diagnostic ignored "-Wint-conversion"
//Null functions
int32_t sample_null_func(void *bus, sl_sensor_data_group_t *id)
{
  (void)bus;
  (void)id;
  return RSI_OK;
}

int32_t control_null_func(void *bus, sl_sensor_command_t id, void *arg)
{
  (void)bus;
  (void)id;
  (void)arg;
  return RSI_OK;
}
//gpio null function
void *gpio_create(sl_sensor_bus_t bus, int id)
{
  (void)bus;
  (void)id;
  return 0xFF;
}
sl_sensor_impl_type_t sensor_impls[] = {

#ifdef SL_CONFIG_SENSOR_LIGHT //Light sensor operations
  {
    .type    = SL_LIGHT_SENSOR_ID,
    .create  = sl_si91x_lightsensor_create,
    .delete  = sl_si91x_lightsensor_delete,
    .sample  = sl_si91x_lightsensor_sample,
    .control = sl_si91x_lightsensor_control,
  },
#endif
#ifdef SL_CONFIG_SENSOR_TEMPERATURE //Temperature sensor operations
  {
    .type    = SL_TEMPERATURE_SENSOR_ID,
    .create  = sl_si91x_temperature_sensor_create,
    .delete  = sl_si91x_temperature_sensor_delete,
    .sample  = sl_si91x_temperature_sensor_sample,
    .control = sl_si91x_temperature_sensor_control,
  },
#endif
#ifdef SL_CONFIG_SENSOR_APDS9960 //APDS9960 sensor operations
  {
    .type    = SL_GESTURE_PROXIMITY_RGB_SENSOR_ID,
    .create  = sl_si91x_apds9960_sensor_create,
    .delete  = sl_si91x_apds9960_sensor_delete,
    .sample  = sl_si91x_apds9960_sensor_sample,
    .control = sl_si91x_apds9960_sensor_control,
  },
#endif
#ifdef SL_CONFIG_SENSOR_ADXL345 //ADXL345 sensor operations
  {
    .type    = SL_ACCELEROMETER_SENSOR_ID,
    .create  = sl_si91x_accelerometer_sensor_create,
    .delete  = sl_si91x_accelerometer_sensor_delete,
    .sample  = sl_si91x_accelerometer_sensor_sample,
    .control = sl_si91x_accelerometer_sensor_control,
  },
#endif
  {
    //GPIO sensor operations
    .type    = SL_GPIO_SENSOR_ID,
    .create  = gpio_create,
    .delete  = NULL,
    .sample  = sample_null_func,
    .control = control_null_func,
  },
#ifdef SL_CONFIG_SENSOR_ADC
  {
    .type   = SL_ADC_SENSOR_ID,
    .create = sl_si91x_adc_sensor_create,
    .delete = sl_si91x_adc_sensor_delete,
#ifdef SH_ADC_ENABLE
    .sample = sl_si91x_adc_sensor_sample,
#endif

#ifdef SH_SDC_ENABLE
    .sample = sl_si91x_sdc_sensor_sample,
#endif
    .control = sl_si91x_adc_sensor_control,
  },
#endif

};

/*******************************************************************************
 * @fn      sl_si91x_get_implementation_size()
 * @brief   Get the size the Sensor operations structure.
 * @param   NULL
 * @return  Returns size of sensor operations
 ******************************************************************************/
uint32_t sl_si91x_get_implementation_size()
{
  return (sizeof(sensor_impls) / sizeof(sl_sensor_impl_type_t));
}
