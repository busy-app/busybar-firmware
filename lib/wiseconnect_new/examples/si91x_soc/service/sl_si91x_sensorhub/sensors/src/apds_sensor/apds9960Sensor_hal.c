/***************************************************************************/ /**
 * @file apds9960Sensor_hal.c
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
/**=============================================================================
 * @brief : This file contains the APDS9960SENSOR HAL file for Sensor Hub
 * application
 * This example file uses simple communication APIs between sensor hub and
 * APDS9960SENSOR HAL.
 ============================================================================**/
#include <stdio.h>
#include "rsi_debug.h"
#include "sensor_hub.h"
#include "sensor_type.h"
/*******************************************************************************
 ****************** APDS Sensor implementation structure ***********************
 ******************************************************************************/
typedef struct {
  sl_apds9960_sensor_id_t id;
  sl_apds9960_error_t (*init)(sl_sensor_bus_t);
  sl_apds9960_error_t (*deinit)(void);
  sl_apds9960_error_t (*test)(void);
  sl_apds9960_error_t (*sample_proximity)(float *l);
  //sl_apds9960_error_t (*sample_gesture)(char *l);
  sl_apds9960_error_t (*sample_rgb)(float *r, float *g, float *b, float *w);
  sl_apds9960_error_t (*sleep)(void);
  sl_apds9960_error_t (*wakeup)(void);
} sl_apds9960_sensor_impl_t;

/*******************************************************************************
 ********************* APDS Sensor Information structure ***********************
 ******************************************************************************/
typedef struct {
  sl_apds9960_sensor_id_t id;
  sl_sensor_bus_t bus;
  bool is_apds9960_init;
  const sl_apds9960_sensor_impl_t *impl;
} sl_sensor_apds9960_t;

/*******************************************************************************
 *********************** APDS Sensor functions structure ***********************
 ******************************************************************************/
static const sl_apds9960_sensor_impl_t apds9960_sensor_implementations[] = {
#ifdef SL_CONFIG_SENSOR_PROXIGESTURGB_INCLUDED_APDS9960
  {
    .id               = SL_APDS9960_ID,
    .init             = sl_si91x_apds9960_init,
    .deinit           = sl_si91x_apds9960_deinit,
    .test             = sl_si91x_apds9960_test,
    .sample_proximity = sl_si91x_apds9960_sample_proximitydata,
    .sample_rgb       = sl_si91x_apds9960_sample_colordata,
    // .sample_gesture         = sl_si91x_apds9960_sample_gesturedata,
    .sleep  = NULL,
    .wakeup = NULL,
  },
#endif
};

/*******************************************************************************
 * fn       static const sl_apds9960_sensor_impl_t *find_implementation(int id)
 * @brief   FInd the implementation of APDS9960 structure
 * @param   id: id declared in apds9960_sensor_id_t
 * @return  Returns active_driver sensor structure if succeed
 ******************************************************************************/
static const sl_apds9960_sensor_impl_t *find_implementation(int id)
{
  const sl_apds9960_sensor_impl_t *active_driver = NULL;
  int count = sizeof(apds9960_sensor_implementations) / sizeof(sl_apds9960_sensor_impl_t);
  for (int i = 0; i < count; i++) {
    if (apds9960_sensor_implementations[i].id == id) {
      active_driver = &apds9960_sensor_implementations[i];
      break;
    }
  }
  return active_driver;
}

/*******************************************************************************
 * fn       sl_sensor_apds9960_handle_t sl_si91x_apds9960_sensor_create
 *                                       (sl_apds9960_bus_handle_t bus, int id);
 * @brief   Create a apds9960 sensor instance.
 *          same series' sensor or sensor with same address can only
 *          be created once.
 * @param   bus i2c bus handle the sensor attached to
 * @param   id id declared in apds9960_sensor_id_t
 * @return  sensor_apds9960_handle_t return apds9960 sensor handle if succeed,
 *          return NULL if failed.
 ******************************************************************************/
sl_sensor_apds9960_handle_t sl_si91x_apds9960_sensor_create(sl_sensor_bus_t bus, int id)
{
  const sl_apds9960_sensor_impl_t *sensor_impl = find_implementation(id);

  if (sensor_impl == NULL) {
    DEBUGOUT("no driver founded, apds9960 ID = %d", id);
    return NULL;
  }
  sl_sensor_apds9960_t *p_sensor = (sl_sensor_apds9960_t *)pvPortMalloc(sizeof(sl_sensor_apds9960_t));
  if (p_sensor == NULL) {
    DEBUGOUT("\r\n apds9960 sensor create failed:%u \r\n", sizeof(sl_sensor_apds9960_t));
    return NULL;
  }
  p_sensor->id            = id;
  p_sensor->bus           = bus;
  p_sensor->impl          = sensor_impl;
  sl_apds9960_error_t ret = p_sensor->impl->init(bus);
  if (ret != RSI_OK) {
    free(p_sensor);
    DEBUGOUT("apds9960 sensor init failed");
    return NULL;
  }
  p_sensor->is_apds9960_init = true;
  return (sl_sensor_apds9960_handle_t)p_sensor;
}

/*******************************************************************************
 * fn       sl_apds9960_error_t sl_si91x_apds9960_sensor_delete
 *                                        (sl_sensor_apds9960_handle_t *sensor);
 * @brief   Delete and release the sensor resource.
 * @param   sensor point to apds9960 sensor handle, will set to NULL if delete
 *          succeed.
 * @return  sl_apds9960_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_apds9960_error_t sl_si91x_apds9960_sensor_delete(sl_sensor_apds9960_handle_t *sensor)
{
  if (sensor == NULL || *sensor == NULL) {
    return RSI_FAIL;
  }
  sl_sensor_apds9960_t *p_sensor = (sl_sensor_apds9960_t *)(*sensor);

  if (!p_sensor->is_apds9960_init) {
    free(p_sensor);
    return RSI_OK;
  }

  p_sensor->is_apds9960_init = false;
  sl_apds9960_error_t ret    = p_sensor->impl->deinit();
  if (ret != RSI_OK) {
    return RSI_FAIL;
  }

  free(p_sensor);
  *sensor = NULL;
  return RSI_OK;
}

/*******************************************************************************
 * fn       sl_apds9960_error_t sl_si91x_apds9960_test_sensor
 *                                        (sl_sensor_apds9960_handle_t sensor);
 * @brief   Test if sensor is active.
 * @param   sensor apds9960 sensor handle to operate.
 * @return  sl_apds9960_error_t
 *            - RSI_OK Success
 *            - RSI_FAIL Fail
 ******************************************************************************/
sl_apds9960_error_t sl_si91x_apds9960_test_sensor(sl_sensor_apds9960_handle_t sensor)
{
  if (sensor == NULL) {
    return RSI_FAIL;
  }
  sl_sensor_apds9960_t *p_sensor = (sl_sensor_apds9960_t *)(sensor);
  if (!p_sensor->is_apds9960_init) {
    return RSI_FAIL;
  }
  sl_apds9960_error_t ret = p_sensor->impl->test();
  return ret;
}

/*******************************************************************************
 * @fn      sl_apds9960_error_t sl_si91x_apds9960_sample_proximity_sensor
 *                      (sl_sensor_apds9960_handle_t sensor, float *proximity);
 * @brief   Sample apds9960 sensor proximity result one time.
 * @param   sensor apds9960 sensor handle to operate.
 * @param   distance result data
 * @return  sl_apds9960_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_apds9960_error_t sl_si91x_apds9960_sample_proximity_sensor(sl_sensor_apds9960_handle_t sensor, float *proximity)
{
  if (sensor == NULL || proximity == NULL) {
    return RSI_FAIL;
  }
  sl_sensor_apds9960_t *p_sensor = (sl_sensor_apds9960_t *)(sensor);
  sl_apds9960_error_t ret        = p_sensor->impl->sample_proximity(proximity);
  return ret;
}

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
sl_apds9960_error_t sl_si91x_apds9960_sample_gesture_sensor(sl_sensor_apds9960_handle_t sensor, float *gesture)
{
  sl_apds9960_error_t ret;
  if (sensor == NULL || gesture == NULL) {
    return RSI_FAIL;
  }
  sl_sensor_apds9960_t *p_sensor = (sl_sensor_apds9960_t *)(sensor);
  (void)p_sensor;
  //ret           = p_sensor->impl->sample_gesture(gesture);
  return ret;
}

/*******************************************************************************
 * @fn      sl_apds9960_error_t sl_si91x_apds9960_sample_rgb_sensor
 *                        (sl_sensor_apds9960_handle_t sensor, sl_rgbw_t *rgbw);
 * @brief   Sample APDS9960 sensor RGB color result one time.
 *          APDS9960 color includes red green blue and white.
 * @param   sensor APDS9960 sensor handle to operate.
 * @param   rgbw result data (unit:lux)
 * @return  sl_apds9960_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_apds9960_error_t sl_si91x_apds9960_sample_rgb_sensor(sl_sensor_apds9960_handle_t sensor, sl_rgbw_t *rgbw)
{
  if (sensor == NULL || rgbw == NULL) {
    return RSI_FAIL;
  }
  sl_sensor_apds9960_t *p_sensor = (sl_sensor_apds9960_t *)(sensor);
  //DEBUGOUT("&rgbw->r %f \r\n", &rgbw->r);
  sl_apds9960_error_t ret = p_sensor->impl->sample_rgb(&rgbw->r, &rgbw->g, &rgbw->b, &rgbw->w);
  return ret;
}

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
sl_apds9960_error_t sl_si91x_apds9960_sleep_sensor(sl_sensor_apds9960_handle_t sensor)
{
  if (sensor == NULL) {
    return RSI_FAIL;
  }
  sl_sensor_apds9960_t *p_sensor = (sl_sensor_apds9960_t *)(sensor);
  sl_apds9960_error_t ret        = p_sensor->impl->sleep();
  return ret;
}

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
sl_apds9960_error_t sl_si91x_apds9960_wakeup_sensor(sl_sensor_apds9960_handle_t sensor)
{
  if (sensor == NULL) {
    return RSI_FAIL;
  }
  sl_sensor_apds9960_t *p_sensor = (sl_sensor_apds9960_t *)(sensor);
  sl_apds9960_error_t ret        = p_sensor->impl->wakeup();
  return ret;
}

/*******************************************************************************
 * fn       sl_apds9960_error_t sl_si91x_apds9960_set_power_of_sensor
 *          (sl_sensor_apds9960_handle_t sensor,
 *          sl_sensor_power_mode_t power_mode);
 * @brief   Set the power state of the sensor APDS9960
 * @param   sensor: apds9960 sensor handle to operate.
 *          power_mode: Mode of the power which can be Sleep or Wakeup
 * @return  sl_apds9960_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_apds9960_error_t sl_si91x_apds9960_set_power_of_sensor(sl_sensor_apds9960_handle_t sensor,
                                                          sl_sensor_power_mode_t power_mode)
{
  sl_sensor_apds9960_t *p_sensor = (sl_sensor_apds9960_t *)(sensor);
  sl_apds9960_error_t ret        = 0;

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
      //ret = NULL;
      break;
  }
  return ret;
}

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
                                                    sl_sensor_data_group_t *data_group)
{
  sl_sensor_apds9960_t *p_sensor = (sl_sensor_apds9960_t *)(sensor);
  sl_apds9960_error_t ret = 0, retu = 0;

  if (sensor == NULL || data_group == NULL) {
    return RSI_FAIL;
  }

  if (((p_sensor->impl->sample_proximity) != NULL) || ((p_sensor->impl->sample_rgb) != NULL)) {
    ret  = p_sensor->impl->sample_proximity(&data_group->sensor_data[data_group->number].rgbw.proximity);
    retu = p_sensor->impl->sample_rgb(&data_group->sensor_data[data_group->number].rgbw.r,
                                      &data_group->sensor_data[data_group->number].rgbw.g,
                                      &data_group->sensor_data[data_group->number].rgbw.b,
                                      &data_group->sensor_data[data_group->number].rgbw.w);
    if (RSI_OK == ret || RSI_OK == retu) {
      data_group->number++;
    }
  }
  /*  if ((p_sensor->impl->sample_gesture) != NULL) {
    ret = p_sensor->impl->sample_gesture(&data_group->sensor_data[data_group->number].gesture);
    if (RSI_OK == ret) {
      data_group->number++;
    }
  }*/
  return RSI_OK;
}

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
                                                     void *args)
{
  (void)sensor;
  (void)args;
  int32_t ret;
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
      ret = RSI_OK;
      break;
    case SL_COMMAND_SELF_TEST:
      ret = RSI_OK;
      break;
    default:
      ret = RSI_OK;
      break;
  }
  return ret;
}
