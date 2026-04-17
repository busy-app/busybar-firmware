/***************************************************************************/ /**
 * @file lm75.c
 * @brief lm75 API implementation
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
 * @brief : This file contains the lm75 sensor's driver file for Sensor Hub
 * application
 * This example uses simple communication APIs for transferring the lm75
 * sensor data through I2C interface.
 * This sensor will detect the temperature value.
 ============================================================================**/
#include "lm75.h"
#include "sensor_hub.h"
#include "rsi_debug.h"
#include "Driver_I2C.h"
#include "Driver_Common.h"
/*******************************************************************************
 ************************ LM_75 Defines / Macros  ******************************
 ******************************************************************************/
#define SL_LM75_TEMPERATURE           0x00 /*!< Command to set Power Down*/
#define SL_LM75_CONFIGURATION         0x01 /*!< Command to set Power Down*/
#define SL_LM75_HIGH_IMPEDENCE        0x02 /*!< Command to set Power Down*/
#define SL_LM75_OVERTEMPERATURE_ALARM 0x03 /*!< Command to set Power On*/

/*******************************************************************************
 ********************** Local/extern variables  ********************************
 ******************************************************************************/
static bool is_lm_init       = false; //for temperature sensor initialization
static sl_lm75_handle_t lm75 = NULL;  //LM75 handle
extern ARM_DRIVER_I2C *I2Cdrv;        //I2C driver handle

/*******************************************************************************
*  @fn          sl_lm75_handle_t sl_si91x_lm75_create
*                                        (sl_sensor_bus_t bus, uint8_t dev_addr)
*  @brief       Creates LM75 sensor in the Sensor Hub.
*  @param[in]   bus:  I2C bus handle
*  @param[in]   dev_addr: LM75 I2C secondary address
*  @return      Returns the LM75 sensor handle
*******************************************************************************/
sl_lm75_handle_t sl_si91x_lm75_create(sl_sensor_bus_t bus, uint8_t dev_addr)
{
  (void)(bus);
  sl_lm75_dev_t *sensor = (sl_lm75_dev_t *)pvPortMalloc(sizeof(sl_lm75_dev_t));
  uint8_t a[3];
  if (sensor == NULL) {
    DEBUGOUT("\r\n LM75 Create fail Unable to allocate memory \r\n");
  }

  a[2]            = '\0';
  a[0]            = (uint8_t)(SL_LM75_TEMPERATURE & 0xFF);
  sensor->i2c_dev = I2Cdrv->MasterTransmit(SL_LM75_I2C_ADDRESS_DEFAULT, a, 1, false);
  while (I2Cdrv->GetStatus().busy)
    ;
  if (sensor->i2c_dev != SL_DRIVER_OK) {
    DEBUGOUT("lm75 create fail I2C unable to transmit1:%ld\r\n", sensor->i2c_dev);
    return NULL;
  }

  a[2]            = '\0';
  a[0]            = (uint8_t)(SL_LM75_CONFIGURATION & 0xFF);
  sensor->i2c_dev = I2Cdrv->MasterTransmit(SL_LM75_I2C_ADDRESS_DEFAULT, a, 1, false);
  while (I2Cdrv->GetStatus().busy)
    ;
  if (sensor->i2c_dev != SL_DRIVER_OK) {
    DEBUGOUT("lm75 create fail I2C unable to transmit2:%ld\r\n", sensor->i2c_dev);
    return NULL;
  }

  DEBUGOUT("\r\n I2C - LM75 Temperature sensor create \r\n");
  /*TODO: check the time out */
  //  vTaskSetTimeOutState( &xTimeOut );
  //  while (I2Cdrv->GetStatus().busy) {
  //      if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE ) {
  //          DEBUGOUT("\r\n I2C Status TimeOut \r\n");
  //          return RSI_FAIL;
  //      }
  //  }

  sensor->dev_addr = dev_addr;
  return (sl_lm75_handle_t)sensor;
}

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_delete(sl_lm75_handle_t *sensor)
*  @brief       Deletes LM75 sensor handle in the Sensor Hub.
*  @param[in]   sensor: LM75 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_delete(sl_lm75_handle_t *sensor)
{
  if (*sensor == NULL) {
    return RSI_OK;
  }
  free(sensor);
  sensor = NULL;
  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_power_down(sl_lm75_handle_t sensor)
*  @brief       Power downs the LM75 sensor in the Sensor Hub.
*  @param[in]   sensor: LM75 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_power_down(sl_lm75_handle_t sensor)
{
  uint8_t ret         = 0;
  sl_lm75_dev_t *sens = (sl_lm75_dev_t *)sensor;
  uint8_t a[5]        = "\0";
  (void)sens;
  a[2] = '\0';
  a[0] = (uint8_t)(SL_LM75_TEMPERATURE & 0xFF);
  ret  = (uint8_t)I2Cdrv->MasterTransmit(SL_LM75_I2C_ADDRESS_DEFAULT, a, 1, false);

  a[2] = '\0';
  a[0] = (uint8_t)(SL_LM75_CONFIGURATION & 0xFF);
  ret  = (uint8_t)I2Cdrv->MasterTransmit(SL_LM75_I2C_ADDRESS_DEFAULT, a, 1, false);

  a[2] = '\0';
  a[0] = (uint8_t)(SL_LM75_HIGH_IMPEDENCE & 0xFF);
  a[1] = (uint8_t)(0x0000 & 0xFFFF);
  ret  = (uint8_t)I2Cdrv->MasterTransmit(SL_LM75_I2C_ADDRESS_DEFAULT, a, 2, false);

  a[2] = '\0';
  a[0] = (uint8_t)(SL_LM75_OVERTEMPERATURE_ALARM & 0xFF);
  a[1] = (uint8_t)(0x0000 & 0xFFFF);
  ret  = (uint8_t)I2Cdrv->MasterTransmit(SL_LM75_I2C_ADDRESS_DEFAULT, a, 2, false);

  DEBUGOUT("\r\n I2C sensor create  Power down\r\n");
  while (I2Cdrv->GetStatus().busy)
    ;

  return ret;
}

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_power_on(sl_lm75_handle_t sensor)
*  @brief       Power ON the LM75 sensor in the Sensor Hub.
*  @param[in]   sensor: LM75 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_power_on(sl_lm75_handle_t sensor)
{
  uint8_t a[5] = "\0";
  int32_t ret;
  sl_lm75_dev_t *sens = (sl_lm75_dev_t *)sensor;
  (void)sens;
  a[0] = (uint8_t)(SL_LM75_TEMPERATURE & 0xFF);
  ret  = (uint8_t)I2Cdrv->MasterTransmit(SL_LM75_I2C_ADDRESS_DEFAULT, a, 1, false); //sens->i2c_dev
  if (ret != ARM_DRIVER_OK) {
    DEBUGOUT("\r\n LM75 Power on command transmit fail %ld \r\n", ret);
    return ret;
  }
  while (I2Cdrv->GetStatus().busy)
    ;

  return ret;
}
/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_change_measure_time(void)
*  @brief       change the measure time LM75 sensor in the Sensor Hub.
*  @param[in]   Sensor : handle of the sensor
*  @param[in]   measure_time : Measure time of the sensor
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_change_measure_time(sl_lm75_handle_t sensor, uint8_t measure_time)
{
  uint8_t ret         = 0;
  sl_lm75_dev_t *sens = (sl_lm75_dev_t *)sensor;
  uint8_t buf[2]      = { 0x40, 0x60 };
  buf[0] |= (measure_time >> 5);
  uint8_t lm75_time = (measure_time & 0x1F);
  buf[1] |= lm75_time;
  (void)sens;
  ret = (uint8_t)I2Cdrv->MasterTransmit(SL_LM75_I2C_ADDRESS_DEFAULT, buf, 2, false);
  while (I2Cdrv->GetStatus().busy)
    ;
  return ret;
}
/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_get_data(sl_lm75_handle_t sensor,
*                                                                   float *data)
*  @brief       Data from the sensor is received and calculated and stored
*  @param[in]   sensor: LM75 Sensor handle
*  @param[in]   data: Processed LM75 data
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_get_data(sl_lm75_handle_t sensor, float *data)
{
  uint8_t len = 0;
  int32_t ret;
  sl_lm75_dev_t *sens   = (sl_lm75_dev_t *)sensor;
  uint8_t lm75_data[10] = { 0 };
  uint8_t temp_data     = 0;

  (void)len;
  (void)sens;
  ret = I2Cdrv->MasterReceive(SL_LM75_I2C_ADDRESS_DEFAULT, lm75_data, 4, false);
  if (ret != RSI_OK) {
    DEBUGOUT("\r\n LM75 Data transmit fail :%ld\r\n", ret);
    return ret;
  }
  /* TODO: Add timeout and PASS error*/
  while (I2Cdrv->GetStatus().busy)
    ;

  // To convert into negative value and converting into floating points
  if ((lm75_data[0] >= 128) && (lm75_data[1] < 128)) {
    lm75_data[0] = ~(lm75_data[0]);
    temp_data    = (lm75_data[0] + 1);
    *data        = temp_data;
    *data        = -(*data);

  } else if ((lm75_data[0] >= 128) && (lm75_data[1] >= 128)) {
    lm75_data[0] = ~(lm75_data[0]);
    temp_data    = lm75_data[0];
    *data        = (float)(temp_data + 0.5);
    *data        = -(*data);

    // Convert into floating value
  } else if ((lm75_data[0] < 128) && (lm75_data[1] >= 128)) {
    temp_data = lm75_data[0];
    *data     = (float)(temp_data + 0.5);

  } else if ((lm75_data[0] < 128) && (lm75_data[1] < 128)) {
    temp_data = lm75_data[0];
    *data     = temp_data;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_init(sl_sensor_bus_t i2c_bus)
*  @brief       Initializing the LM75 sensor in the Sensor Hub.
*  @param[in]   i2c_bus: LM75 Sensor I2C Bus handle
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_init(sl_sensor_bus_t i2c_bus)
{
  lm75 = sl_si91x_lm75_create(i2c_bus, SL_LM75_I2C_ADDRESS_DEFAULT);
  if (lm75 == NULL) {
    DEBUGOUT("\r\n LM75 Init fail \r\n");
    return RSI_FAIL;
  }
  is_lm_init = true;
  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_deinit(void)
*  @brief       DeInitializing the LM75 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_deinit(void)
{
  if (!is_lm_init) {
    DEBUGOUT("\r\n LM75 denit fail, not initialized \r\n");
    return RSI_FAIL;
  }

  sl_lm75_err_t ret = sl_si91x_lm75_delete(&lm75);

  if (ret != RSI_OK) {
    DEBUGOUT("\r\n BH1750 deinit fail \r\n");
    return RSI_FAIL;
  }

  is_lm_init = false;
  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_test(void)
*  @brief       Testing the LM75 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_test(void)
{
  if (!is_lm_init) {
    DEBUGOUT("\r\n LM75 test fail, not initialized \r\n");
    return RSI_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_sample_temperature(float *l
*  @brief       Processes and Reads the value of Temperature sensor and sends
*               the value to Sensor Hub
*  @param[in]   l: Value of Temperature data
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_sample_temperature(float *l)
{
  if (!is_lm_init) {
    return RSI_FAIL;
  }

  // sl_lm75_cmd_measure_t cmd_measure;
  float lm75_data;
  sl_lm75_err_t ret = sl_si91x_lm75_power_on(lm75);
  if (ret != RSI_OK) {
    DEBUGOUT("\r\n LM75 Sample temperature fail \r\n");
    return RSI_FAIL;
  }
  //vTaskDelay(300 / portTICK_RATE_MS); /*TODO:check if need or change*/
  ret = sl_si91x_lm75_get_data(lm75, &lm75_data);

  if (ret != RSI_OK) {
    DEBUGOUT("\r\n LM75 Sample temperature fail \r\n");
    *l = 0;
    return RSI_FAIL;
  }
  *l = lm75_data;
  return RSI_OK;
}
