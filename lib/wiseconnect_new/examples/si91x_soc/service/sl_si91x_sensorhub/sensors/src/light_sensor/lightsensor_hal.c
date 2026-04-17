/***************************************************************************/ /**
 * @file lightsensor_hal.c
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
/**=============================================================================
 * @brief : This file contains the Light sensor's hal file for Sensor Hub
 * application
 * This example file uses simple communication APIs between sensor hub and
 * Light sensor.
 ============================================================================**/
#include <stdio.h>
#include "rsi_debug.h"
#include "sensor_hub.h"
#include "sensor_type.h"

/*******************************************************************************
 ******************* Light Sensor implementation structure *********************
 ******************************************************************************/
typedef struct {
  sl_light_sensor_id_t id;
  sl_light_error_t (*init)(sl_sensor_bus_t);
  sl_light_error_t (*deinit)(void);
  sl_light_error_t (*test)(void);
  sl_light_error_t (*sample_light)(float *l);
  sl_light_error_t (*sample_rgbw)(float *r, float *g, float *b, float *w);
  sl_light_error_t (*sample_uv)(float *uv, float *uva, float *uvb);
  sl_light_error_t (*sleep)(void);
  sl_light_error_t (*wakeup)(void);
} sl_light_sensor_impl_t;

/*******************************************************************************
 ******************** Light Sensor Information structure ***********************
 ******************************************************************************/
typedef struct {
  sl_light_sensor_id_t id;
  sl_sensor_bus_t bus;
  bool is_light_init;
  const sl_light_sensor_impl_t *impl;
} sl_sensor_light_t;

/*******************************************************************************
 ********************* Light Sensor functions structure ************************
 ******************************************************************************/
static const sl_light_sensor_impl_t light_sensor_implementations[] = {
#ifdef SL_CONFIG_SENSOR_LIGHT_INCLUDED_BH1750
  {
    .id           = SL_BH1750_ID,
    .init         = sl_si91x_bh1750_init,
    .deinit       = sl_si91x_bh1750_deinit,
    .test         = sl_si91x_bh1750_test,
    .sample_light = sl_si91x_bh1750_sample_light,
    .sample_rgbw  = NULL,
    .sample_uv    = NULL,
    .sleep        = NULL,
    .wakeup       = NULL,
  },
#endif
};

/****************************private functions*************************************/
/*******************************************************************************
 * @fn      static const sl_light_sensor_impl_t *find_implementation(int id)
 * @brief   Create Light sensor instance.
 * @param   id light sensor id
 * @return  Returns temperature sensor driver operations
 ******************************************************************************/
static const sl_light_sensor_impl_t *find_implementation(int id)
{
  const sl_light_sensor_impl_t *active_driver = NULL;
  int count                                   = sizeof(light_sensor_implementations) / sizeof(sl_light_sensor_impl_t);
  for (int i = 0; i < count; i++) {
    if (light_sensor_implementations[i].id == id) {
      active_driver = &light_sensor_implementations[i];
      break;
    }
  }
  return active_driver;
}

/*******************************************************************************
 * @fn      sl_sensor_light_handle_t sl_si91x_lightsensor_create
 *                                                 (sl_sensor_bus_t bus, int id)
 * @brief   Create a Bh1750 Light sensor instance.
 *          Same series sensor or sensor with same address can only
 *          be created once.
 * @param   bus i2c bus handle the sensor attached to
 * @param   id id declared in sl_light_sensor_id_t
 * @return  sl_sensor_light_handle_t Returns BH1750 sensor handle if succeed,
 *          returns NULL if failed.
 ******************************************************************************/
sl_sensor_light_handle_t sl_si91x_lightsensor_create(sl_sensor_bus_t bus, int id)
{
  const sl_light_sensor_impl_t *sensor_impl = find_implementation(id);

  if (sensor_impl == NULL) {
    DEBUGOUT("No driver founded, LIGHT ID = %d", id);
    return NULL;
  }
  sl_sensor_light_t *p_sensor = (sl_sensor_light_t *)pvPortMalloc(sizeof(sl_sensor_light_t));
  if (p_sensor == NULL) {
    DEBUGOUT("\r\n Light sensor HAL Memory allocation fail:%u \r\n", sizeof(sl_sensor_light_t));
    return NULL;
  }
  p_sensor->id         = id;
  p_sensor->bus        = bus;
  p_sensor->impl       = sensor_impl;
  sl_light_error_t ret = p_sensor->impl->init(bus);
  if (ret != RSI_OK) {
    free(p_sensor);
    DEBUGOUT("light sensor init failed");
    return NULL;
  }
  p_sensor->is_light_init = true;
  return (sl_sensor_light_handle_t)p_sensor;
}

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
sl_light_error_t sl_si91x_lightsensor_delete(sl_sensor_light_handle_t *sensor)
{
  if (sensor == NULL || *sensor == NULL) {
    return SL_STATUS_FAIL;
  }
  sl_sensor_light_t *p_sensor = (sl_sensor_light_t *)(*sensor);

  if (!p_sensor->is_light_init) {
    free(p_sensor);
    return SL_STATUS_OK;
  }

  p_sensor->is_light_init = false;
  sl_light_error_t ret    = p_sensor->impl->deinit();
  if (ret != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  free(p_sensor);
  *sensor = NULL;
  return RSI_OK;
}

/*******************************************************************************
 * @fn      sl_light_error_t sl_si91x_lightsensor_test
 *                                            (sl_sensor_light_handle_t sensor)
 * @brief   Test if sensor is active.
 * @param   sensor BH1750 sensor handle to operate.
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_test(sl_sensor_light_handle_t sensor)
{
  if (sensor == NULL) {
    return SL_STATUS_FAIL;
  }
  sl_sensor_light_t *p_sensor = (sl_sensor_light_t *)(sensor);
  if (!p_sensor->is_light_init) {
    return SL_STATUS_FAIL;
  }
  sl_light_error_t ret = p_sensor->impl->test();
  return ret;
}

/*******************************************************************************
 *  @fn     sl_light_error_t sl_si91x_lightsensor_sample_light
 *                                 (sl_sensor_light_handle_t sensor, float *lux)
 * @brief   Sample light sensor illuminance result one time.
 * @param   sensor light sensor handle to operate.
 * @param   lux result data (unit:lux)
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
 ******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_sample_light(sl_sensor_light_handle_t sensor, float *lux)
{
  if (sensor == NULL || lux == NULL) {
    return SL_STATUS_FAIL;
  }
  sl_sensor_light_t *p_sensor = (sl_sensor_light_t *)(sensor);
  sl_light_error_t ret        = p_sensor->impl->sample_light(lux);
  return ret;
}

/*******************************************************************************
 *  @fn     sl_light_error_t sl_si91x_lightsensor_sample_rgbw
 *                            (sl_sensor_light_handle_t sensor, sl_rgbw_t *rgbw)
 * @brief   Sample RGBW light sensor illuminance result one time.
 *          Light color includes red green blue and white.
 * @param   sensor light sensor handle to operate.
 * @param   rgbw result data (unit:lux)
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_sample_rgbw(sl_sensor_light_handle_t sensor, sl_rgbw_t *rgbw)
{
  if (sensor == NULL || rgbw == NULL) {
    return SL_STATUS_FAIL;
  }
  sl_sensor_light_t *p_sensor = (sl_sensor_light_t *)(sensor);
  sl_light_error_t ret        = p_sensor->impl->sample_rgbw(&rgbw->r, &rgbw->g, &rgbw->b, &rgbw->w);
  return ret;
}

/*******************************************************************************
 *  @fn     sl_light_error_t sl_si91x_lightsensor_sample_uv
 *                                (sl_sensor_light_handle_t sensor, sl_uv_t *uv)
 * @brief   Sample light sensor ultra violet result one time.
 *          Light Ultraviolet includes UVA UVB and UV.
 * @param   sensor light sensor handle to operate.
 * @param   uv result data (unit:lux)
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 *          - RSI_ERR_NOT_SUPPORTED Function not supported on this sensor
******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_sample_uv(sl_sensor_light_handle_t sensor, sl_uv_t *uv)
{
  if (sensor == NULL || uv == NULL) {
    return SL_STATUS_FAIL;
  }
  sl_sensor_light_t *p_sensor = (sl_sensor_light_t *)(sensor);
  sl_light_error_t ret        = p_sensor->impl->sample_uv(&uv->uv, &uv->uva, &uv->uvb);
  return ret;
}

/*******************************************************************************
 * @fn      sl_light_error_t sl_si91x_lightsensor_sleep
 *                                             (sl_sensor_light_handle_t sensor)
 * @brief   Set sensor to sleep mode.
 * @param   sensor light sensor handle to operate.
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_sleep(sl_sensor_light_handle_t sensor)
{
  if (sensor == NULL) {
    return SL_STATUS_FAIL;
  }
  sl_sensor_light_t *p_sensor = (sl_sensor_light_t *)(sensor);
  sl_light_error_t ret        = p_sensor->impl->sleep();
  return ret;
}

/*******************************************************************************
 * @fn      sl_light_error_t sl_si91x_lightsensor_wakeup
 *                                             (sl_sensor_light_handle_t sensor)
 * @brief   Wakeup sensor from sleep mode.
 * @param   sensor light sensor handle to operate.
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_wakeup(sl_sensor_light_handle_t sensor)
{
  if (sensor == NULL) {
    return SL_STATUS_FAIL;
  }
  sl_sensor_light_t *p_sensor = (sl_sensor_light_t *)(sensor);
  sl_light_error_t ret        = p_sensor->impl->wakeup();
  return ret;
}

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
sl_light_error_t sl_si91x_lightsensor_set_power(sl_sensor_light_handle_t sensor, sl_sensor_power_mode_t power_mode)
{
  sl_sensor_light_t *p_sensor = (sl_sensor_light_t *)(sensor);
  sl_light_error_t ret        = 0;

  if (sensor == NULL || power_mode == 0) {
    return SL_STATUS_FAIL;
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
 * @fn      sl_light_error_t sl_si91x_lightsensor_sample
 *          (sl_sensor_light_handle_t sensor,sl_sensor_data_group_t *data_group)
 * @brief   Sample a group of sensor data
 * @param   sensor light sensor handle to operate.
 * @param   data_group acquired data
 * @return  sl_light_error_t
 *          - RSI_OK Success
 *          - RSI_FAIL Fail
 ******************************************************************************/
sl_light_error_t sl_si91x_lightsensor_sample(sl_sensor_light_handle_t sensor, sl_sensor_data_group_t *data_group)
{
  sl_sensor_light_t *p_sensor = (sl_sensor_light_t *)(sensor);
  sl_light_error_t ret        = 0;

  if (sensor == NULL || data_group == NULL) {
    return SL_STATUS_FAIL;
  }

  if ((p_sensor->impl->sample_light) != NULL) {
    ret = p_sensor->impl->sample_light(&data_group->sensor_data[data_group->number].light);
    if (RSI_OK == ret) {
      data_group->number++;
    }
  }
  if ((p_sensor->impl->sample_rgbw) != NULL) {
    ret = p_sensor->impl->sample_rgbw(&data_group->sensor_data[data_group->number].rgbw.r,
                                      &data_group->sensor_data[data_group->number].rgbw.g,
                                      &data_group->sensor_data[data_group->number].rgbw.b,
                                      &data_group->sensor_data[data_group->number].rgbw.w);
    if (RSI_OK == ret) {
      data_group->number++;
    }
  }
  if ((p_sensor->impl->sample_uv) != NULL) {
    ret = p_sensor->impl->sample_uv(&data_group->sensor_data[data_group->number].uv.uv,
                                    &data_group->sensor_data[data_group->number].uv.uva,
                                    &data_group->sensor_data[data_group->number].uv.uvb);
    if (RSI_OK == ret) {
      data_group->number++;
    }
  }
  return RSI_OK;
}

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
sl_light_error_t sl_si91x_lightsensor_control(sl_sensor_light_handle_t sensor, sl_sensor_command_t cmd, void *args)
{
  sl_light_error_t ret = RSI_OK;

  (void)args;
  if (sensor == NULL || cmd == 0) {
    return SL_STATUS_FAIL;
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
      //ret = light_sensor_set_power(sensor, (sensor_power_mode_t)args);//if it is required enable
      ret = RSI_OK;
      break;
    case SL_COMMAND_SELF_TEST:
      //ret = light_sensor_test(sensor);//if it is required enable
      ret = RSI_OK;
      break;
    default:
      ret = RSI_OK;
      break;
  }
  return ret;
}
