/***************************************************************************/ /**
 * @file APDS9960.h
 * @brief APDS9960SENSOR_HAL API implementation
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

#ifndef APDS9960_HAL_H_
#define APDS9960_HAL_H_
#include "sensor_type.h"

/*******************************************************************************
 ******************* APDS sensor Typedefs/ Defines  ****************************
 ******************************************************************************/
typedef int32_t sl_apds9960_error_t;
typedef void *sl_apds9960_bus_handle_t;
typedef void *sl_sensor_apds9960_handle_t; /*!< APDS9960 sensor handle*/
#define SL_CONFIG_SENSOR_PROXIGESTURGB_INCLUDED_APDS9960 1
#ifdef SL_CONFIG_SENSOR_PROXIGESTURGB_INCLUDED_APDS9960
#include "apds9960.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * @fn       sl_sensor_apds9960_handle_t sl_si91x_apds9960_sensor_create
 *                                       (sl_apds9960_bus_handle_t bus, int id);
 * @brief   Create a APDS9960 sensor instance.
 *          same series sensor or sensor with the same address can only
 *          be created once.
 * @param   bus i2c bus handles the sensor attached
 * @param   id id declared in apds9960_sensor_id_t
 * @return  sensor_apds9960_handle_t return APDS9960 sensor handle if succeed,
 *          return NULL if failed.
 ******************************************************************************/
sl_sensor_apds9960_handle_t sl_si91x_apds9960_sensor_create(sl_sensor_bus_t bus, int id);

/*******************************************************************************
 * @fn       sl_apds9960_error_t sl_si91x_apds9960_sensor_delete
 *                                        (sl_sensor_apds9960_handle_t *sensor);
 * @brief   Delete and release the sensor resource.
 * @param   sensor point to APDS9960 sensor handle, will set to NULL if delete
 *          succeed.
 * @return  sl_apds9960_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_apds9960_error_t sl_si91x_apds9960_sensor_delete(sl_sensor_apds9960_handle_t *sensor);

/*******************************************************************************
 * @fn       sl_apds9960_error_t sl_si91x_apds9960_test_sensor
 *                                        (sl_sensor_apds9960_handle_t sensor);
 * @brief   Test if the sensor is active.
 * @param   sensor APDS9960 sensor handle to operate.
 * @return  sl_apds9960_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_apds9960_error_t sl_si91x_apds9960_test_sensor(sl_sensor_apds9960_handle_t sensor);
/*******************************************************************************
 * @fn      sl_apds9960_error_t sl_si91x_apds9960_set_power_of_sensor
 *          (sl_sensor_apds9960_handle_t sensor,
 *          sl_sensor_power_mode_t power_mode);
 * @brief   Set the power state of the sensor APDS9960
 * @param   sensor: APDS9960 sensor handle to operate.
 *          power_mode: Mode of the power which can be Sleep or Wakeup
 * @return  sl_apds9960_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_apds9960_error_t sl_si91x_apds9960_set_power_of_sensor(sl_sensor_apds9960_handle_t sensor,
                                                          sl_sensor_power_mode_t power_mode);

/*******************************************************************************
 * @fn      sl_apds9960_error_t sl_si91x_apds9960_sample_proximity_sensor
 *                      (sl_sensor_apds9960_handle_t sensor, float *proximity);
 * @brief   Sample APDS9960 sensor proximity result one time.
 * @param   sensor APDS9960 sensor handle to operate.
 * @param   distance result data
 * @return  sl_apds9960_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_apds9960_error_t sl_si91x_apds9960_sample_proximity_sensor(sl_sensor_apds9960_handle_t sensor, float *proximity);

/*******************************************************************************
 * @fn      sl_apds9960_error_t sl_si91x_apds9960_sample_rgb_sensor
 *                        (sl_sensor_apds9960_handle_t sensor, sl_rgbw_t *rgbw);
 * @brief   Sample APDS9960 sensor RGB color result in one time.
 *          APDS9960 color includes red green blue and white.
 * @param   sensor APDS9960 sensor handle to operate.
 * @param   rgbw result data (unit:lux)
 * @return  sl_apds9960_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_apds9960_error_t sl_si91x_apds9960_sample_rgb_sensor(sl_sensor_apds9960_handle_t sensor, sl_rgbw_t *rgbw);

/*******************************************************************************
 * @fn      sl_apds9960_error_t sl_si91x_apds9960_sample_gesture_sensor
 *                          (sl_sensor_apds9960_handle_t sensor, float *gesture)
 * @brief   Sample apds9960 sensor Gesture result one time.
 * @param   sensor apds9960 sensor handle to operate.
 * @param   distance result data
 * @return  sl_apds9960_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_apds9960_error_t sl_si91x_apds9960_sample_gesture_sensor(sl_sensor_apds9960_handle_t sensor,
                                                            float *gesture); //comment

/*******************************************************************************
 * @fn      sl_apds9960_error_t sl_si91x_apds9960_sample_gesture_sensor
 *                          (sl_sensor_apds9960_handle_t sensor, float *gesture)
 * @brief   Sample a group of sensor data
 * @param   sensor APDS9960 sensor handle to operate
 * @param   data_group sampled data.
 * @param   distance result data
 * @return  sl_apds9960_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_apds9960_error_t sl_si91x_apds9960_sensor_sample(sl_sensor_apds9960_handle_t sensor,
                                                    sl_sensor_data_group_t *data_group);

/*******************************************************************************
 * @fn      sl_apds9960_error_t sl_si91x_apds9960_sensor_control
 *          (sl_sensor_apds9960_handle_t sensor, sl_sensor_command_t cmd,
                                                                    void *args);
 * @brief   Control sensor mode with control commands and args
 * @param   sensor APDS9960 sensor handle to operate
 * @param   cmd control commands detailed in sensor_command_t
 * @param   args control commands args
 * @return  sl_apds9960_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_apds9960_error_t sl_si91x_apds9960_sensor_control(sl_sensor_apds9960_handle_t sensor,
                                                     sl_sensor_command_t cmd,
                                                     void *args);
/*******************************************************************************
 * @fn      sl_apds9960_error_t sl_si91x_apds9960_sleep_sensor(
 *                                          sl_sensor_apds9960_handle_t sensor)
 * @brief   Put the APDS sensor in the sleep.
 * @param   Sensor APDS9960 sensor handle to operate.
 * @param   rgbw result data (unit:lux)
 * @return  sl_apds9960_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_apds9960_error_t sl_si91x_apds9960_sleep_sensor(sl_sensor_apds9960_handle_t sensor);
/*******************************************************************************
 * @fn      sl_apds9960_error_t sl_si91x_apds9960_wakeup_sensor(sl_sensor_apds9960_handle_t sensor)
 * @brief   Wakeup the sensor from the sleep.
 * @param   Sensor APDS9960 sensor handle to operate.
 * @param   rgbw result data (unit:lux)
 * @return  sl_apds9960_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_apds9960_error_t sl_si91x_apds9960_wakeup_sensor(sl_sensor_apds9960_handle_t sensor);
#ifdef __cplusplus
extern "C"
}
#endif

#endif /* APDS9960SENSOR_HAL_H_ */
