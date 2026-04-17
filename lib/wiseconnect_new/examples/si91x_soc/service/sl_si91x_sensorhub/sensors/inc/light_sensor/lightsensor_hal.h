/***************************************************************************/ /**
 * @file lightsensor_hal.h
 * @brief Light sensor HAL API implementation
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

#ifndef LIGHTSENSOR_HAL_H_
#define LIGHTSENSOR_HAL_H_
#include "sensor_type.h"

/*******************************************************************************
 ********************** Light sensor Typedefs/ Defines  ************************
 ******************************************************************************/
typedef int32_t sl_light_error_t;
typedef void *sl_light_bus_handle_t;
typedef void *sl_sensor_light_handle_t; /*!< light sensor handle*/

#define SL_CONFIG_SENSOR_LIGHT_INCLUDED_BH1750 1
#ifdef SL_CONFIG_SENSOR_LIGHT_INCLUDED_BH1750
#include "bh1750fvi.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* @fn      sl_sensor_light_handle_t sl_si91x_lightsensor_create
*                                               (sl_sensor_bus_t bus, int id)
* @brief   Create a Bh1750 Light sensor instance.
*          Same series sensor or sensor with the same address can only
*          be created once.
* @param   bus i2c bus handles the sensor attached to
* @param   id id declared in sl_light_sensor_id_t
* @return  sl_sensor_light_handle_t Returns BH1750 sensor handle if succeeds,
*          returns NULL if failed.
******************************************************************************/
sl_sensor_light_handle_t sl_si91x_lightsensor_create(sl_sensor_bus_t bus, int id);

/*******************************************************************************
* @fn      sl_light_error_t sl_si91x_lightsensor_delete
*                                            (sl_sensor_light_handle_t *sensor)
* @brief   Delete and release the sensor resource.
* @param   sensor point to BH1750 sensor handle, will set to NULL if delete
*          succeed.
* @return  sl_light_error_t
*          - RSI_OK Success
*          - RSI_FAIL Fail
******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_delete(sl_sensor_light_handle_t *sensor);

/*******************************************************************************
 * @fn      sl_light_error_t sl_si91x_lightsensor_test
 *                                            (sl_sensor_light_handle_t sensor)
 * @brief   Test if the sensor is active.
 * @param   sensor BH1750 sensor handle to operate.
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_test(sl_sensor_light_handle_t sensor);

/*******************************************************************************
 *  @fn     sl_light_error_t sl_si91x_lightsensor_sample_light
 *                                 (sl_sensor_light_handle_t sensor, float *lux)
 * @brief   Sample light sensor illuminance result in one time.
 * @param   sensor light sensor handle to operate.
 * @param   lux result data (unit:lux)
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_sample_light(sl_sensor_light_handle_t sensor, float *lux);

/*******************************************************************************
 *  @fn     sl_light_error_t sl_si91x_lightsensor_sample_rgbw
 *                            (sl_sensor_light_handle_t sensor, sl_rgbw_t *rgbw)
 * @brief   Sample RGBW light sensor illuminance result in one time.
 *          Light color includes red green blue and white.
 * @param   sensor light sensor handle to operate.
 * @param   rgbw result data (unit:lux)
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_sample_rgbw(sl_sensor_light_handle_t sensor, sl_rgbw_t *rgbw);

/*******************************************************************************
 *  @fn     sl_light_error_t sl_si91x_lightsensor_sample_uv
 *                                (sl_sensor_light_handle_t sensor, sl_uv_t *uv)
 * @brief   Sample light sensor ultraviolet result one time.
 *          Light Ultraviolet includes UVA UVB and UV.
 * @param   sensor light sensor handle to operate.
 * @param   uv result data (unit:lux)
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_sample_uv(sl_sensor_light_handle_t sensor, sl_uv_t *uv);

/*******************************************************************************
 * @fn      sl_light_error_t sl_si91x_lightsensor_sleep
 *                                             (sl_sensor_light_handle_t sensor)
 * @brief   Set sensor to sleep mode.
 * @param   sensor light sensor handle to operate.
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_sleep(sl_sensor_light_handle_t sensor);

/*******************************************************************************
 * @fn      sl_light_error_t sl_si91x_lightsensor_wakeup
 *                                             (sl_sensor_light_handle_t sensor)
 * @brief   Wakeup sensor from sleep mode.
 * @param   sensor light sensor handle to operate.
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_wakeup(sl_sensor_light_handle_t sensor);

/*******************************************************************************
 * @fn      sl_light_error_t sl_si91x_lightsensor_sample
 *          (sl_sensor_light_handle_t sensor,sl_sensor_data_group_t *data_group)
 * @brief   Sample a group of sensor data
 * @param   sensor light sensor handle to operate.
 * @param   data_group acquired data
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_sample(sl_sensor_light_handle_t sensor, sl_sensor_data_group_t *data_group);

/*******************************************************************************
 * @fn      sl_light_error_t sl_si91x_lightsensor_control
 *                    (sl_sensor_light_handle_t sensor, sl_sensor_command_t cmd,
 *                                                                  void *args)
 * @brief   Control sensor mode with control commands and args
 * @param   sensor light sensor handle to operate
 * @param   cmd control commands detailed in sensor_command_t
 * @param   args control commands args
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_control(sl_sensor_light_handle_t sensor, sl_sensor_command_t cmd, void *args);

/*******************************************************************************
 * fn       sl_light_error_t sl_si91x_lightsensor_set_power
 *          (sl_sensor_light_handle_t sensor, sl_sensor_power_mode_t power_mode)
 * @brief   Set the power state of the sensor BH1750
 * @param   sensor: BH1750 sensor handle to operate.
 *          power_mode: Mode of the power which can be Sleep or Wakeup
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_set_power(sl_sensor_light_handle_t sensor, sl_sensor_power_mode_t power_mode);

#ifdef __cplusplus
extern "C"
}
#endif

#endif /* LIGHTSENSOR_HAL_H_ */
