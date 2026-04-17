/***************************************************************************/ /**
 * @file tempSensor.h
 * @brief Temperature Sensor API implementation
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
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef TEMPERATURESENSOR_HAL_H_
#define TEMPERATURESENSOR_HAL_H_
#include "sensor_type.h"

/*******************************************************************************
 **************** temperature sensor Typedefs/ Defines  ************************
 ******************************************************************************/
typedef int32_t sl_temperature_error_t;              //temperature sensor error status
typedef void *sl_sensor_temperature_handle_t;        //temperature sensor handle
#define SL_CONFIG_SENSOR_TEMPERATURE_INCLUDED_LM75 1 //LM75 define
#ifdef SL_CONFIG_SENSOR_TEMPERATURE_INCLUDED_LM75
#include "lm75.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* @fn      sl_sensor_temperature_handle_t sl_si91x_temperature_sensor_create
*                                                 (sl_sensor_bus_t bus, int id)
* @brief   Create a LM75 Temperature sensor instance.
*          Same series sensor or sensor with same address can only
*          be created once.
* @param   bus i2c bus handle the sensor attached to
* @param   id id declared in sl_temp_sensor_id_t
* @return  sl_sensor_temperature_handle_t Returns LM75 sensor handle if succeed,
*          returns NULL if failed.
******************************************************************************/
sl_sensor_temperature_handle_t sl_si91x_temperature_sensor_create(sl_sensor_bus_t bus, int id);

/*******************************************************************************
 * @fn      sl_temperature_error_t sl_si91x_temperature_sensor_delete
 *                                      (sl_sensor_temperature_handle_t *sensor)
 * @brief   Delete and release the sensor resource.
 * @param   sensor point to LM75 sensor handle, will set to NULL if delete
 *          succeed.
 * @return  sl_temperature_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_temperature_error_t sl_si91x_temperature_sensor_delete(sl_sensor_temperature_handle_t *sensor);

/*******************************************************************************
 * @fn      sl_temperature_error_t sl_si91x_temperature_sensor_test
 *                                       (sl_sensor_temperature_handle_t sensor)
 * @brief   Test if sensor is active.
 * @param   sensor LM75 sensor handle to operate.
 * @return  sl_temperature_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_temperature_error_t sl_si91x_temperature_sensor_test(sl_sensor_temperature_handle_t sensor);

/*******************************************************************************
 * @fn      sl_temperature_error_t sl_si91x_sl_si91x_temperature_sensor_sample_temperature
 *                  (sl_sensor_temperature_handle_t sensor, float *temperature)
 * @brief   Samples temperature sensor temperature result one time.
 * @param   sensor: temperature sensor handle to operate.
 * @param   temperature: celcius result data (unit:celcius)
 * @return  sl_temperature_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_temperature_error_t sl_si91x_sl_si91x_temperature_sensor_sample_temperature(sl_sensor_temperature_handle_t sensor,
                                                                               float *temperature);

/*******************************************************************************
* @fn       sl_temperature_error_t sl_si91x_temperature_sensor_sleep
*                                       (sl_sensor_temperature_handle_t sensor)
* @brief    Set sensor to sleep mode.
* @param    sensor temperature sensor handle to operate.
* @return   sl_temperature_error_t
*           - RSI_OK Success
*           - RSI_FAIL Fail
*           - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
*******************************************************************************/
sl_temperature_error_t sl_si91x_temperature_sensor_sleep(sl_sensor_temperature_handle_t sensor);

/*******************************************************************************
* @fn       sl_temperature_error_t sl_si91x_temperature_sensor_wakeup
*                                        (sl_sensor_temperature_handle_t sensor)
* @brief    Wakeup sensor from sleep mode.
* @param    sensor temperature sensor handle to operate.
* @return   sl_temperature_error_t
*           - RSI_OK Success
*           - RSI_FAIL Fail
*           - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
*******************************************************************************/
sl_temperature_error_t sl_si91x_temperature_sensor_wakeup(sl_sensor_temperature_handle_t sensor);

/*******************************************************************************
 * @fn      sl_temperature_error_t sl_si91x_temperature_sensor_sample
 *          (sl_sensor_temperature_handle_t sensor,
 *                                           sl_sensor_data_group_t *data_group)
 * @brief   Sample a group of sensor data
 * @param   sensor LM75 sensor handle to operate
 * @param   data_group sampled data.
 * @return  sl_temperature_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_temperature_error_t sl_si91x_temperature_sensor_sample(sl_sensor_temperature_handle_t sensor,
                                                          sl_sensor_data_group_t *data_group);

/*******************************************************************************
 * @fn      sl_temperature_error_t sl_si91x_temperature_sensor_control
 *          (sl_sensor_temperature_handle_t sensor, sl_sensor_command_t cmd,
                                                                     void *args)
 * @brief   Control sensor mode with control commands and args
 * @param   sensor LM75 sensor handle to operate
 * @param   cmd control commands detailed in sensor_command_t
 * @param   args control commands args
 * @return  sl_temperature_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_temperature_error_t sl_si91x_temperature_sensor_control(sl_sensor_temperature_handle_t sensor,
                                                           sl_sensor_command_t cmd,
                                                           void *args);
/*******************************************************************************
 * fn       sl_temperature_error_t sl_si91x_temperature_sensor_set_power
 *          (sl_sensor_temperature_handle_t sensor,
 *                                            sl_sensor_power_mode_t power_mode)
 * @brief   Set the power state of the sensor LM75
 * @param   sensor: lm75 sensor handle to operate.
 *          power_mode: Mode of the power which can be Sleep or Wakeup
 * @return  sl_temperature_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_temperature_error_t sl_si91x_temperature_sensor_set_power(sl_sensor_temperature_handle_t sensor,
                                                             sl_sensor_power_mode_t power_mode);

#ifdef __cplusplus
extern "C"
}
#endif

#endif /* TEMPERATURESENSOR_HAL_H_ */
