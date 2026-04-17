/***************************************************************************/ /**
 * @file bh170fvi.c
 * @brief bh170 API implementation
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
 * @brief : This file contains the BH170 sensor's driver file for Sensor Hub
 * application
 * This example uses simple communication APIs for transferring the BH170
 * sensor data through I2C interface.
 * This sensor will detect the temperature value.
 ============================================================================**/
#include "bh1750fvi.h"
#include "sensor_hub.h"
#include "rsi_debug.h"
#include "Driver_Common.h"
#include "Driver_I2C.h"
#include "sensorhub_error_codes.h"
/*******************************************************************************
 ************************ BH170 Defines / Macros  ******************************
 ******************************************************************************/
#define SL_BH_1750_MEASUREMENT_ACCURACY 1.2  /*!< the typical measurement accuracy of  BH1750 sensor */
#define SL_BH1750_POWER_DOWN            0x00 /*!< Command to set Power Down*/
#define SL_BH1750_POWER_ON              0x01 /*!< Command to set Power On*/
#define SL_BH1750_DATA_REG_RESET        0x07 /*!< Command to reset data register, not acceptable in power down mode*/
static bool is_light_init = false;           //Light sensor status
extern ARM_DRIVER_I2C *I2Cdrv;               //i2c driver
static sl_bh1750_handle_t bh1750 = NULL;     //bh170 driver handle
//#define SL_MAX_TIME_TO_WAIT 5000;
//TickType_t xTicksToWait = SL_MAX_TIME_TO_WAIT;
TimeOut_t xTimeOut;

/*******************************************************************************
*  @fn          sl_bh1750_handle_t sl_si91x_bh1750_create(sl_sensor_bus_t bus,
*                                                              uint8_t dev_addr)
*  @brief       Creates BH1750 sensor in the Sensor Hub.
*  @param[in]   bus:  I2C bus handle
*  @param[in]   dev_addr: BH1750 I2C secondary address
*  @return      Returns the BH1750 sensor handle
*******************************************************************************/
sl_bh1750_handle_t sl_si91x_bh1750_create(sl_sensor_bus_t bus, uint8_t dev_addr)
{
  sl_bh1750_dev_t *sensor = (sl_bh1750_dev_t *)pvPortMalloc(sizeof(sl_bh1750_dev_t));
  uint8_t a[5];
  (void)bus;
  if (sensor == NULL) {
    DEBUGOUT("\r\n BH1750 Create fail Unable to allocate memory \r\n");
  }
  a[0]            = (uint8_t)(0x01 & 0xFF);
  sensor->i2c_dev = I2Cdrv->MasterTransmit(SL_BH1750_I2C_ADDRESS_DEFAULT, a, 1, false);

  if (sensor->i2c_dev != SL_DRIVER_OK) {
    DEBUGOUT("bh1750 create fail I2C unable to transmit:%ld\r\n", sensor->i2c_dev);
    return NULL;
  }

  DEBUGOUT("\r\n I2C - BH1750 Light Sensor Create \r\n");
  while (I2Cdrv->GetStatus().busy)
    ;
  /*TODO: check the time out */
  //  vTaskSetTimeOutState( &xTimeOut );
  //  while (I2Cdrv->GetStatus().busy) {
  //      if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE ) {
  //          DEBUGOUT("\r\n I2C Status TimeOut \r\n");
  //          return RSI_FAIL;
  //      }
  //  }

  sensor->dev_addr = dev_addr;
  return (sl_bh1750_handle_t)sensor;
}

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_delete(sl_bh1750_handle_t *sensor)
*  @brief       Deletes BH1750 sensor handle in the Sensor Hub.
*  @param[in]   sensor: BH1750 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_delete(sl_bh1750_handle_t *sensor)
{
  if (*sensor == NULL) {
    return RSI_OK;
  }
  free(sensor);
  sensor = NULL;
  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_power_down
*                                                    (sl_bh1750_handle_t sensor)
*  @brief       Power downs the BH1750 sensor in the Sensor Hub.
*  @param[in]   sensor: BH1750 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_power_down(sl_bh1750_handle_t sensor)
{
  int32_t ret           = 0;
  sl_bh1750_dev_t *sens = (sl_bh1750_dev_t *)sensor;
  (void)sens;
  uint8_t a[5] = "\0";

  a[0] = (uint8_t)(SL_BH1750_POWER_DOWN & 0xFF);
  ret  = (int32_t)I2Cdrv->MasterTransmit(SL_BH1750_I2C_ADDRESS_DEFAULT, a, 1, false);
  if (ret != SL_DRIVER_OK) {
    return ret;
  }
  //DEBUGOUT("\r\n I2C sensor create \r\n");
  while (I2Cdrv->GetStatus().busy)
    ;

  return SL_STATUS_OK;
}

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_power_on
*                                                    (sl_bh1750_handle_t sensor)
*  @brief       Power ON the BH1750 sensor in the Sensor Hub.
*  @param[in]   sensor: BH1750 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_power_on(sl_bh1750_handle_t sensor)
{
  uint8_t a[5]          = "\0";
  int32_t ret           = 0;
  sl_bh1750_dev_t *sens = (sl_bh1750_dev_t *)sensor;
  (void)sens;

  a[0] = (uint8_t)(SL_BH1750_POWER_ON & 0xFF);
  ret  = (int32_t)I2Cdrv->MasterTransmit(SL_BH1750_I2C_ADDRESS_DEFAULT, a, 1, false); //sens->i2c_dev
  if (ret != ARM_DRIVER_OK) {
    DEBUGOUT("\r\n BH1750 Power on command transmit fail %ld \r\n", ret);
    return ret;
  }

  while (I2Cdrv->GetStatus().busy)
    ;

  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_reset_data_register
*                                                    (sl_bh1750_handle_t sensor)
*  @brief       Resets the data register
*  @param[in]   sensor: BH1750 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_reset_data_register(sl_bh1750_handle_t sensor)
{
  int32_t ret           = 0;
  sl_bh1750_dev_t *sens = (sl_bh1750_dev_t *)sensor;
  (void)sens;
  sl_si91x_bh1750_power_on(sensor);
  uint8_t a[5] = "\0";

  a[0] = (uint8_t)(SL_BH1750_DATA_REG_RESET & 0xFF);
  ret  = (int32_t)I2Cdrv->MasterTransmit(SL_BH1750_I2C_ADDRESS_DEFAULT, a, 1, false);
  if (ret != ARM_DRIVER_OK) {
    return ret;
  }
  while (I2Cdrv->GetStatus().busy)
    ;

  return SL_STATUS_OK;
}
/*******************************************************************************
*  @fn          sl_si91x_bh1750_change_measure_time(sl_bh1750_handle_t sensor,
*                                               uint8_t measure_time);
*  @brief       Get light intensity
*  @param[in]   sensor: BH1750 Sensor handle
*  @param[in]   measure_time: Measure time
*  @param[in]   data: Processed BH1750 light intensity data
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_change_measure_time(sl_bh1750_handle_t sensor, uint8_t measure_time)
{
  int32_t ret           = 0;
  sl_bh1750_dev_t *sens = (sl_bh1750_dev_t *)sensor;
  (void)sens;
  uint8_t buf[2] = { 0x40, 0x60 };
  buf[0] |= measure_time >> 5;
  uint8_t bh1750_time = measure_time & 0x1F;
  ;
  buf[1] |= bh1750_time;

  ret = (int32_t)I2Cdrv->MasterTransmit(SL_BH1750_I2C_ADDRESS_DEFAULT, buf, 2, false);
  if (ret != SL_DRIVER_OK) {
    return ret;
  }
  while (I2Cdrv->GetStatus().busy)
    ;
  return SL_STATUS_OK;
}
/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_set_measure_mode
*               (sl_bh1750_handle_t sensor, sl_bh1750_cmd_measure_t cmd_measure)
*  @brief       Sets the Measure Mode
*  @param[in]   sensor: BH1750 Sensor handle
*  @param[in]   cmd_measure Command Measure
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_set_measure_mode(sl_bh1750_handle_t sensor, sl_bh1750_cmd_measure_t cmd_measure)
{
  uint8_t a[1];
  int32_t ret;
  sl_bh1750_dev_t *sens = (sl_bh1750_dev_t *)sensor;
  (void)sens;

  a[0] = (uint8_t)(cmd_measure & 0xFF);
  ret  = (int32_t)I2Cdrv->MasterTransmit(SL_BH1750_I2C_ADDRESS_DEFAULT, a, 1, false);
  if (ret != ARM_DRIVER_OK) {
    DEBUGOUT("\r\n BH1750 set measure mode command transmit fail :%ld\r\n", ret);
    return ret;
  }
  /* TODO: Add timeout and PASS error*/
  while (I2Cdrv->GetStatus().busy)
    ;
  return ret;
}

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_get_data
*                                       (sl_bh1750_handle_t sensor, float *data)
*  @brief       Data from the sensor is received and calculated and stored
*  @param[in]   sensor: BH1750 Sensor handle
*  @param[in]   data: Processed BH1750 data
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_get_data(sl_bh1750_handle_t sensor, float *data)
{
  uint8_t len = 0;
  int32_t ret;
  sl_bh1750_dev_t *sens = (sl_bh1750_dev_t *)sensor;
  (void)sens;
  (void)len;
  uint8_t bh1750_data[2] = { 0 };
  ret                    = (int32_t)I2Cdrv->MasterReceive(SL_BH1750_I2C_ADDRESS_DEFAULT, bh1750_data, 2, false);
  if (ret != RSI_OK) {
    DEBUGOUT("\r\n BH1750 Data transmit fail :%ld\r\n", ret);
    return ret;
  }
  /* TODO: Add timeout and PASS error*/
  while (I2Cdrv->GetStatus().busy)
    ;
  *data = (float)((bh1750_data[0] << 8 | bh1750_data[1]) / SL_BH_1750_MEASUREMENT_ACCURACY);
  return ret;
}

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_get_light_intensity
*               (sl_bh1750_handle_t sensor, sl_bh1750_cmd_measure_t cmd_measure,
                                                                   float *data)
*  @brief       Get light intensity
*  @param[in]   sensor: BH1750 Sensor handle
*  @param[in]   cmd_measure: Command Measure
*  @param[in]   data: Processed BH1750 light intensity data
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_get_light_intensity(sl_bh1750_handle_t sensor,
                                                   sl_bh1750_cmd_measure_t cmd_measure,
                                                   float *data)
{
  sl_bh170_err_t ret = sl_si91x_bh1750_set_measure_mode(sensor, cmd_measure);

  if (ret != RSI_OK) {
    return ret;
  }

  if ((cmd_measure == SL_BH1750_CONTINUE_4LX_RES) || (cmd_measure == SL_BH1750_ONETIME_4LX_RES)) {
    // vTaskDelay(300 / portTICK_RATE_MS);
  } else {
    //vTaskDelay(180 / portTICK_RATE_MS);
  }

  ret = sl_si91x_bh1750_get_data(sensor, data);
  return ret;
}

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_init(sl_sensor_bus_t i2c_bus)
*  @brief       Initializing the BH1750 sensor in the Sensor Hub.
*  @param[in]   i2c_bus: BH1750 Sensor I2C Bus handle
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_init(sl_sensor_bus_t i2c_bus)
{
  bh1750 = sl_si91x_bh1750_create(i2c_bus, SL_BH1750_I2C_ADDRESS_DEFAULT);

  if (bh1750 == NULL) {
    DEBUGOUT("\r\n BH1750 Init fail \r\n");
    return RSI_FAIL;
  }
  is_light_init = true;
  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_deinit(void)
*  @brief       DeInitializing the BH1750 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_deinit(void)
{
  if (!is_light_init) {
    DEBUGOUT("\r\n BH1750 denit fail, not initialized \r\n");
    return RSI_FAIL;
  }

  sl_bh170_err_t ret = sl_si91x_bh1750_delete(&bh1750);
  if (ret != RSI_OK) {
    DEBUGOUT("\r\n BH1750 deinit fail \r\n");
    return RSI_FAIL;
  }

  is_light_init = false;

  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_test(void)
*  @brief       Testing the BH1750 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_test(void)
{
  if (!is_light_init) {
    return RSI_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_sample_light(float *light_data)
*  @brief       Processes and Reads the value of Light sensor and sends
*               the value to Sensor Hub
*  @param[in]   light_data: Value of Light data
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_sample_light(float *light_data)
{
  if (!is_light_init) {
    return RSI_FAIL;
  }

  sl_bh1750_cmd_measure_t cmd_measure;
  float bh1750_data;
  sl_bh170_err_t ret = sl_si91x_bh1750_power_on(bh1750);
  cmd_measure        = SL_BH1750_CONTINUE_4LX_RES;
  ret                = sl_si91x_bh1750_set_measure_mode(bh1750, cmd_measure);
  //vTaskDelay(30 / portTICK_RATE_MS); /*TODO:check if need or change*/
  ret = sl_si91x_bh1750_get_data(bh1750, &bh1750_data);

  if (ret == RSI_OK) {
    *light_data = bh1750_data;
    return RSI_OK;
  }

  *light_data = 0;
  return RSI_FAIL;
}
