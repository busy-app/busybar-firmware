/***************************************************************************/ /**
 * @file accelerometerSensor_hal.c
 * @brief ACCELEROMETERSENSOR_HAL API implementation
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
/**=============================================================================
 * @brief : This file contains the Accelerometer HAL file for Sensor Hub
 * application
 * This example file uses simple communication APIs between sensor hub and
 * Accelerometer HAL.
 ============================================================================**/
#include <stdio.h>
#include "rsi_debug.h"
#include "sensor_hub.h"
#include "sensor_type.h"

/*******************************************************************************
 ***************** Accelerometer Sensor implementation structure ***************
 ******************************************************************************/
typedef struct {
  sl_accelerometer_sensor_id_t id;
  sl_acceleromerter_error_t (*init)(sl_sensor_bus_t);
  sl_acceleromerter_error_t (*deinit)(void);
  sl_acceleromerter_error_t (*test)(void);
  sl_acceleromerter_error_t (*sample_accelerometer)(float *x, float *y, float *z);
  sl_acceleromerter_error_t (*sleep)(void);
  sl_acceleromerter_error_t (*wakeup)(void);
} sl_accelerometer_sensor_impl_t;

/*******************************************************************************
 ***************** Accelerometer Sensor Information structure ******************
 ******************************************************************************/
typedef struct {
  sl_accelerometer_sensor_id_t id;
  sl_sensor_bus_t bus;
  bool sl_acceleromerter_is_init;
  const sl_accelerometer_sensor_impl_t *impl;
} sl_sensor_accelerometer_t;

/*******************************************************************************
 *************** Accelerometer Sensor implementation structure *****************
 ******************************************************************************/
static const sl_accelerometer_sensor_impl_t accelerometer_sensor_implementations[] = {
#ifdef SL_CONFIG_SENSOR_ACCELEROMETER_INCLUDED_ADXL345
  {
    .id                   = SL_ADXL345_ID,
    .init                 = sl_si91x_adxl345_init,
    .deinit               = sl_si91x_adxl345_deinit,
    .test                 = sl_si91x_adxl345_test,
    .sample_accelerometer = sl_si91x_adxl345_sample_accelerometer,
    .sleep                = NULL,
    .wakeup               = NULL,
  },
#endif
};

/*******************************************************************************
 * fn       static const sl_accelerometer_sensor_impl_t *find_implementation(int id)
 * @brief   FInd the implementation of accelerometer Sensor structure
 * @param   id: id of accelerometer Sensor
 * @return  Returns active_driver sensor structure if succeed
 ******************************************************************************/
static const sl_accelerometer_sensor_impl_t *find_implementation(int id)
{
  const sl_accelerometer_sensor_impl_t *active_driver = NULL;
  int count = sizeof(accelerometer_sensor_implementations) / sizeof(sl_accelerometer_sensor_impl_t);
  for (int i = 0; i < count; i++) {
    if (accelerometer_sensor_implementations[i].id == id) {
      active_driver = &accelerometer_sensor_implementations[i];
      break;
    }
  }
  return active_driver;
}

/*******************************************************************************
* @fn       sl_sensor_accelerometer_handle_t
*           sl_si91x_accelerometer_sensor_create
*                                (sl_sensor_bus_t bus, int id)
* @brief   Create a ADXL345 accelerometer sensor instance.
*          same series' sensor or sensor with same address can only
*          be created once.
* @param   bus i2c bus handle the sensor attached to
* @param   id declared in sl_accelerometer_sensor_id_t
* @return  sl_sensor_accelerometer_handle_t returns ADXL 345 sensor handle if
*          succeed, return NULL if failed.
*******************************************************************************/
sl_sensor_accelerometer_handle_t sl_si91x_accelerometer_sensor_create(sl_sensor_bus_t bus, int id)
{
  const sl_accelerometer_sensor_impl_t *sensor_impl = find_implementation(id);

  if (sensor_impl == NULL) {
    DEBUGOUT("no driver founded, ACCELEROMETER ID = %d", id);
    return NULL;
  }
  sl_sensor_accelerometer_t *p_sensor = (sl_sensor_accelerometer_t *)pvPortMalloc(sizeof(sl_sensor_accelerometer_t));
  if (p_sensor == NULL) {
    DEBUGOUT("\r\n accelerometer sensor create failed:%u \r\n", sizeof(sl_sensor_accelerometer_t));
    return NULL;
  }
  p_sensor->id                  = id;
  p_sensor->bus                 = bus;
  p_sensor->impl                = sensor_impl;
  sl_acceleromerter_error_t ret = p_sensor->impl->init(bus);
  if (ret != RSI_OK) {
    free(p_sensor);
    DEBUGOUT("accelerometer sensor init failed:%lu", ret);
    return NULL;
  }
  p_sensor->sl_acceleromerter_is_init = true;
  return (sl_sensor_accelerometer_handle_t)p_sensor;
}

/*******************************************************************************
 * @fn      sl_acceleromerter_error_t sl_si91x_accelerometer_sensor_delete
 *                                    (sl_sensor_accelerometer_handle_t *sensor)
 * @brief   Deletes and release the sensor resource.
 * @param   sensor point to ADXL345 sensor handle, will set to NULL if delete
 *          succeed.
 * @return  sl_acceleromerter_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_acceleromerter_error_t sl_si91x_accelerometer_sensor_delete(sl_sensor_accelerometer_handle_t *sensor)
{
  if (sensor == NULL || *sensor == NULL) {
    return RSI_FAIL;
  }
  sl_sensor_accelerometer_t *p_sensor = (sl_sensor_accelerometer_t *)(*sensor);

  if (!p_sensor->sl_acceleromerter_is_init) {
    free(p_sensor);
    return RSI_OK;
  }

  p_sensor->sl_acceleromerter_is_init = false;
  sl_acceleromerter_error_t ret       = p_sensor->impl->deinit();
  if (ret != RSI_OK) {
    return RSI_FAIL;
  }

  free(p_sensor);
  *sensor = NULL;
  return RSI_OK;
}

/*******************************************************************************
 * @fn      sl_acceleromerter_error_t sl_si91x_accelerometer_sensor_test
 *                                     (sl_sensor_accelerometer_handle_t sensor)
 * @brief   Test if sensor is active.
 * @param   sensor ADXL345 sensor handle to operate.
 * @return  sl_acceleromerter_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_acceleromerter_error_t sl_si91x_accelerometer_sensor_test(sl_sensor_accelerometer_handle_t sensor)
{
  if (sensor == NULL) {
    return RSI_FAIL;
  }
  sl_sensor_accelerometer_t *p_sensor = (sl_sensor_accelerometer_t *)(sensor);
  if (!p_sensor->sl_acceleromerter_is_init) {
    return RSI_FAIL;
  }
  sl_acceleromerter_error_t ret = p_sensor->impl->test();
  return ret;
}

/*******************************************************************************
 * @fn      sl_acceleromerter_error_t
 *          sl_si91x_accelerometer_sensor_sample_accelerometer
 *                    (sl_sensor_accelerometer_handle_t sensor, sl_axis_t *axis)
 * @brief   Sample ADXL345 sensor result one time.
 * @param   sensor ADXL345 sensor handle to operate.
 * @param   axis: value of axis
 * @return  sl_acceleromerter_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_acceleromerter_error_t sl_si91x_accelerometer_sensor_sample_accelerometer(sl_sensor_accelerometer_handle_t sensor,
                                                                             sl_axis_t *axis)
{
  if (sensor == NULL || axis == NULL) {
    return RSI_FAIL;
  }
  sl_sensor_accelerometer_t *p_sensor = (sl_sensor_accelerometer_t *)(sensor);
  sl_acceleromerter_error_t ret       = p_sensor->impl->sample_accelerometer(&axis->x, &axis->y, &axis->z);
  return ret;
}

/*******************************************************************************
 * @fn      sl_acceleromerter_error_t sl_si91x_accelerometer_sensor_sleep
 *                                     (sl_sensor_accelerometer_handle_t sensor)
 * @brief   Setting the ADXL345 Sensor to SLeep
 * @param   sensor ADXL345 sensor handle to operate.
 * @return  sl_acceleromerter_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_acceleromerter_error_t sl_si91x_accelerometer_sensor_sleep(sl_sensor_accelerometer_handle_t sensor)
{
  if (sensor == NULL) {
    return RSI_FAIL;
  }
  sl_sensor_accelerometer_t *p_sensor = (sl_sensor_accelerometer_t *)(sensor);
  sl_acceleromerter_error_t ret       = p_sensor->impl->sleep();
  return ret;
}

/*******************************************************************************
 * @fn      sl_acceleromerter_error_t sl_si91x_accelerometer_sensor_wakeup
 *                                     (sl_sensor_accelerometer_handle_t sensor)
 * @brief   Wakeup sensor from sleep mode.
 * @param   sensor ADXL345 sensor handle to operate.
 * @return  sl_acceleromerter_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_acceleromerter_error_t sl_si91x_accelerometer_sensor_wakeup(sl_sensor_accelerometer_handle_t sensor)
{
  if (sensor == NULL) {
    return RSI_FAIL;
  }
  sl_sensor_accelerometer_t *p_sensor = (sl_sensor_accelerometer_t *)(sensor);
  sl_acceleromerter_error_t ret       = p_sensor->impl->wakeup();
  return ret;
}

/*******************************************************************************
 * @fn      sl_acceleromerter_error_t sl_si91x_accelerometer_sensor_set_power
 *          (sl_sensor_accelerometer_handle_t sensor,
 *                                            sl_sensor_power_mode_t power_mode)
 * @brief   Set the power state of the sensor ADXL345
 * @param   sensor: ADXL345 sensor handle to operate.
 * @param   power_mode: Mode of the power which can be Sleep or Wakeup
 * @return  sl_acceleromerter_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_acceleromerter_error_t sl_si91x_accelerometer_sensor_set_power(sl_sensor_accelerometer_handle_t sensor,
                                                                  sl_sensor_power_mode_t power_mode)
{
  sl_sensor_accelerometer_t *p_sensor = (sl_sensor_accelerometer_t *)(sensor);
  sl_acceleromerter_error_t ret       = 0;

  if (sensor == NULL || power_mode == 0) {
    return RSI_FAIL;
  }

  switch (power_mode) {
    case SL_SENSOR_POWER_MODE_WAKEUP:
      ret = p_sensor->impl->wakeup();
      break;
    case SL_SENSOR_POWER_MODE_SLEEP:
      ret = p_sensor->impl->sleep();
      break;
    default:
      ret = 0;
      break;
  }
  return ret;
}

/*******************************************************************************
 * @fn      sl_acceleromerter_error_t sl_si91x_accelerometer_sensor_sample
 *          (sl_sensor_accelerometer_handle_t sensor,
 *                                           sl_sensor_data_group_t *data_group)
 * @brief   Sample a group of sensor data
 * @param   sensor ADXL345 sensor handle to operate
 * @param   data_group sampled data.
 * @return  sl_acceleromerter_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_acceleromerter_error_t sl_si91x_accelerometer_sensor_sample(sl_sensor_accelerometer_handle_t sensor,
                                                               sl_sensor_data_group_t *data_group)
{
  sl_sensor_accelerometer_t *p_sensor = (sl_sensor_accelerometer_t *)(sensor);
  sl_acceleromerter_error_t ret       = 0;

  if (sensor == NULL || data_group == NULL) {
    return RSI_FAIL;
  }

  if ((p_sensor->impl->sample_accelerometer) != NULL) {
    ret = p_sensor->impl->sample_accelerometer(&data_group->sensor_data[data_group->number].accelerometer.x,
                                               &data_group->sensor_data[data_group->number].accelerometer.y,
                                               &data_group->sensor_data[data_group->number].accelerometer.z);
    if (RSI_OK == ret) {
      data_group->number++;
    }
  }
  return RSI_OK;
}

/*******************************************************************************
 * @fn      sl_acceleromerter_error_t sl_si91x_accelerometer_sensor_control
 *          (sl_sensor_accelerometer_handle_t sensor, sl_sensor_command_t cmd,
                                                                     void *args)
 * @brief   Control sensor mode with control commands and args
 * @param   sensor ADXL345 sensor handle to operate
 * @param   cmd control commands detailed in sensor_command_t
 * @param   args control commands args
 * @return  sl_acceleromerter_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_acceleromerter_error_t sl_si91x_accelerometer_sensor_control(sl_sensor_accelerometer_handle_t sensor,
                                                                sl_sensor_command_t cmd,
                                                                void *args)
{
  sl_acceleromerter_error_t ret = RSI_OK;
  (void)args;
  if (sensor == NULL || cmd == 0) {
    return RSI_FAIL;
  }

  switch (cmd) {
    case SL_COMMAND_SET_MODE:
      ret = RSI_OK;
      break;
    case SL_COMMAND_SET_RANGE:
      ret = RSI_OK;
      break;
    case SL_COMMAND_SET_ODR:
      ret = RSI_OK;
      break;
    case SL_COMMAND_SET_POWER:
      //ret = accelerometer_sensor_set_power(sensor, (sensor_power_mode_t)args);//if it is required enable
      ret = RSI_OK;
      break;
    case SL_COMMAND_SELF_TEST:
      //ret = accelerometer_sensor_test(sensor);//if it is required enable
      ret = RSI_OK;
      break;
    default:
      ret = RSI_OK;
      break;
  }
  return ret;
}
