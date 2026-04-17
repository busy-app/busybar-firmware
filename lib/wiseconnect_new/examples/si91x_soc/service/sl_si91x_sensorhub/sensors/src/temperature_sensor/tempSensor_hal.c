/***************************************************************************/ /**
 * @file tempSensor.c
 * @brief Temperature sensor HAL API implementation
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
 * @brief : This file contains the temperature sensor's hal file for Sensor Hub
 * application
 * This example file uses simple communication APIs between sensor hub and
 * Temperature sensor.
 ============================================================================**/

#include <stdio.h>
#include "rsi_debug.h"
#include "sensor_hub.h"
#include "sensor_type.h"

#ifdef SL_CONFIG_SENSOR_TEMPERATURE_INCLUDED_LM75
#include "lm75.h"
#endif
#ifdef CONFIG_SENSOR_temperature_INCLUDED_HTS221
#include "hts221.h"
#endif

/*******************************************************************************
 *************** Temperature Sensor implementation structure *******************
 ******************************************************************************/
typedef struct {
  sl_temp_sensor_id_t id;
  sl_temperature_error_t (*init)(sl_sensor_bus_t);
  sl_temperature_error_t (*deinit)(void);
  sl_temperature_error_t (*test)(void);
  sl_temperature_error_t (*sample_temperature)(float *);
  sl_temperature_error_t (*sleep)(void);
  sl_temperature_error_t (*wakeup)(void);
} sl_temp_sensor_impl_t;

/*******************************************************************************
 **************** Temperature Sensor Information structure *********************
 ******************************************************************************/
typedef struct {
  sl_temp_sensor_id_t id;            //Temperature sensor ID
  sl_sensor_bus_t bus;               //Interface bus
  bool is_temperature_init;          //Temperature sensor initialization status
  const sl_temp_sensor_impl_t *impl; //Sensor implementation structure
} sl_sensor_temperature_t;

/*******************************************************************************
 ****************** Temperature Sensor functions structure *********************
 ******************************************************************************/
static const sl_temp_sensor_impl_t temp_sensor_implementations[] = {
#ifdef SL_CONFIG_SENSOR_TEMPERATURE_INCLUDED_LM75
  {
    .id                 = SL_LM75_ID,                       //LM75 id
    .init               = sl_si91x_lm75_init,               //lm75 initialization
    .deinit             = sl_si91x_lm75_deinit,             //lm75 deinitialization
    .test               = sl_si91x_lm75_test,               //lm75 test
    .sample_temperature = sl_si91x_lm75_sample_temperature, //lm75 sample temperature
    .sleep              = NULL,
    .wakeup             = NULL,
  },
#endif
#ifdef SL_CONFIG_SENSOR_temperature_INCLUDED_HTS221
  {
    .id                 = HTS221_ID,
    .init               = temperature_hts221_init,
    .deinit             = temperature_hts221_deinit,
    .test               = temperature_hts221_test,
    .sample_humidity    = temperature_hts221_sample_humidity,
    .sample_temperature = temperature_hts221_sample_temperature,
    .sleep              = temperature_hts221_sleep,
    .wakeup             = temperature_hts221_wakeup,
  },
#endif
};

/****************************private functions*************************************/
/*******************************************************************************
 * @fn      static const sl_temp_sensor_impl_t *find_implementation(int id)
 * @brief   Create Temperature sensor instance.
 * @param   id id declared in sl_temp_sensor_id_t
 * @return  Returns temperature sensor driver operations
 ******************************************************************************/
static const sl_temp_sensor_impl_t *find_implementation(int id)
{
  const sl_temp_sensor_impl_t *active_driver = NULL;
  int count                                  = sizeof(temp_sensor_implementations) / sizeof(sl_temp_sensor_impl_t);

  for (int i = 0; i < count; i++) {
    if (temp_sensor_implementations[i].id == id) {
      active_driver = &temp_sensor_implementations[i];
      break;
    }
  }
  return active_driver;
}

/****************************public functions*************************************/

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
sl_sensor_temperature_handle_t sl_si91x_temperature_sensor_create(sl_sensor_bus_t bus, int id)
{

  const sl_temp_sensor_impl_t *sensor_impl = find_implementation(id);

  if (sensor_impl == NULL) {
    DEBUGOUT("no driver founded, Temperature ID = %d", id);
    return NULL;
  }

  sl_sensor_temperature_t *p_sensor = (sl_sensor_temperature_t *)pvPortMalloc(sizeof(sl_sensor_temperature_t));
  if (p_sensor == NULL) {
    DEBUGOUT("\r\n Temperature sensor HAL Memory allocation fail:%u \r\n", sizeof(sl_sensor_temperature_t));
    return NULL;
  }

  p_sensor->id               = id;
  p_sensor->bus              = bus;
  p_sensor->impl             = sensor_impl;
  sl_temperature_error_t ret = p_sensor->impl->init(bus);

  if (ret != RSI_OK) {
    free(p_sensor);
    DEBUGOUT("Temperature sensor init failed");
    return NULL;
  }

  p_sensor->is_temperature_init = true;
  return (sl_sensor_temperature_handle_t)p_sensor;
}

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
sl_temperature_error_t sl_si91x_temperature_sensor_delete(sl_sensor_temperature_handle_t *sensor)
{
  if (sensor == NULL) {
    return RSI_FAIL;
  }

  sl_sensor_temperature_t *p_sensor = (sl_sensor_temperature_t *)(*sensor);

  if (!p_sensor->is_temperature_init) {
    free(p_sensor);
    return RSI_OK;
  }

  p_sensor->is_temperature_init = false;
  sl_temperature_error_t ret    = p_sensor->impl->deinit();
  if (ret != RSI_OK) {
    return RSI_FAIL;
  }
  return RSI_OK;
}

/*******************************************************************************
 * @fn      sl_temperature_error_t sl_si91x_temperature_sensor_test
 *                                        (sl_sensor_temperature_handle_t sensor)
 * @brief   Test if sensor is active.
 * @param   sensor LM75 sensor handle to operate.
 * @return  sl_temperature_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_temperature_error_t sl_si91x_temperature_sensor_test(sl_sensor_temperature_handle_t sensor)
{
  if (sensor == NULL) {
    return RSI_FAIL;
  }

  sl_sensor_temperature_t *p_sensor = (sl_sensor_temperature_t *)(sensor);

  if (!p_sensor->is_temperature_init) {
    return RSI_FAIL;
  }

  sl_temperature_error_t ret = p_sensor->impl->test();
  return ret;
}

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
                                                                               float *temperature)
{
  if (sensor == NULL) {
    return RSI_FAIL;
  }
  sl_sensor_temperature_t *p_sensor = (sl_sensor_temperature_t *)(sensor);
  sl_temperature_error_t ret        = p_sensor->impl->sample_temperature(temperature);
  return ret;
}

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
sl_temperature_error_t sl_si91x_temperature_sensor_sleep(sl_sensor_temperature_handle_t sensor)
{
  if (sensor == NULL) {
    return RSI_FAIL;
  }
  sl_sensor_temperature_t *p_sensor = (sl_sensor_temperature_t *)(sensor);
  sl_temperature_error_t ret        = p_sensor->impl->sleep();
  return ret;
}

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
sl_temperature_error_t sl_si91x_temperature_sensor_wakeup(sl_sensor_temperature_handle_t sensor)
{
  if (sensor == NULL) {
    return RSI_FAIL;
  }
  sl_sensor_temperature_t *p_sensor = (sl_sensor_temperature_t *)(sensor);
  sl_temperature_error_t ret        = p_sensor->impl->wakeup();
  return ret;
}

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
                                                             sl_sensor_power_mode_t power_mode)
{
  sl_sensor_temperature_t *p_sensor = (sl_sensor_temperature_t *)(sensor);
  sl_temperature_error_t ret        = 0;

  if (sensor == NULL) {
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
      break;
  }
  return ret;
}

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
                                                          sl_sensor_data_group_t *data_group)
{
  sl_sensor_temperature_t *p_sensor = (sl_sensor_temperature_t *)(sensor);
  sl_temperature_error_t ret        = 0;

  if (sensor == NULL) {
    return RSI_FAIL;
  }

  if ((p_sensor->impl->sample_temperature) != NULL) {
    ret = p_sensor->impl->sample_temperature(&data_group->sensor_data[data_group->number].temperature);
    if (RSI_OK == ret) {
      data_group->number++;
    }
  }
  return RSI_OK;
}

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
                                                           void *args)
{
  int32_t ret;
  (void)args;
  (void)sensor;
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
      //ret = sl_si91x_temperature_sensor_set_power(sensor, (sl_sensor_power_mode_t)args); //Enable if required
      ret = RSI_OK;
      break;
    case SL_COMMAND_SELF_TEST:
      //ret = temperature_test(sensor);                                 //Enable if required
      ret = RSI_OK;
      break;
    default:
      ret = RSI_OK;
      break;
  }
  return ret;
}
