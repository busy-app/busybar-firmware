/***************************************************************************/ /**
* @file hub_hal_intf.h
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

#ifndef HUB_HAL_INTF_H_
#define HUB_HAL_INTF_H_

#include "sensors_config.h"
#include "sensor_type.h"
#include "lightsensor_hal.h"
#include "tempSensor_hal.h"
#include "apds9960Sensor_hal.h"
#include "accelerometerSensor_hal.h"
#include "adc_sensor_hal.h"
#include "sl_si91x_sdc.h"

/*******************************************************************************
 ******************** Sensor HUB Operations Structure  *************************
 ******************************************************************************/
typedef void *sl_sensor_driver_handle_t; //Hub hal interface driver handle
typedef struct {
  sl_sensor_type_t type;                                                  /* sensor type  */
  sl_sensor_driver_handle_t (*create)(sl_sensor_bus_t, int sensor_id);    /* create a sensor  */
  int32_t (*delete)(sl_sensor_driver_handle_t *);                         /*delete a sensor  */
  int32_t (*sample)(sl_sensor_driver_handle_t, sl_sensor_data_group_t *); /* acquire a group of sensor data  */
  int32_t (*control)(sl_sensor_driver_handle_t,
                     sl_sensor_command_t cmd,
                     void *args); /* modify the sensor configuration  */
} sl_sensor_impl_type_t;

int32_t sample_null_func(void *, sl_sensor_data_group_t *);
int32_t control_null_func(void *, sl_sensor_command_t, void *);
void *gpio_create(sl_sensor_bus_t bus, int id);
uint32_t sl_si91x_get_implementation_size();

#endif /* HUB_HAL_INTF_H_ */
