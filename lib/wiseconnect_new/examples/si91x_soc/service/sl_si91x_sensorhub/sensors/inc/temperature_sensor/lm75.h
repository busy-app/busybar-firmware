/***************************************************************************/ /**
 * @file lm75.h
 * @brief LM75 API implementation
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

#ifndef LM75_H_
#define LM75_H_
#include <stdbool.h>
#include "rsi_data_types.h"
#include "sensors_config.h"
#include "sensor_type.h"

/*******************************************************************************
 ************************ LM_75 Typedefs/ Defines  *****************************
 ******************************************************************************/
#define SL_LM75_I2C_ADDRESS_DEFAULT (0x48) //Lm75 sensor address
typedef void *sl_lm75_handle_t;            //lm75 sensor handle
typedef int32_t sl_lm75_err_t;             //lm75 error

/*******************************************************************************
 ******************** Global Structures for LM75 Sensor  ***********************
 ******************************************************************************/

typedef struct {
  int32_t i2c_dev;  //i2c device
  uint8_t dev_addr; //device address
} sl_lm75_dev_t;

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*  @fn          sl_lm75_handle_t sl_si91x_lm75_create
*                                        (sl_sensor_bus_t bus, uint8_t dev_addr)
*  @brief       Creates LM75 sensor in the Sensor Hub.
*  @param[in]   bus:  I2C bus handle
*  @param[in]   dev_addr: LM75 I2C secondary address
*  @return      Returns the LM75 sensor handle
*******************************************************************************/
sl_lm75_handle_t sl_si91x_lm75_create(sl_sensor_bus_t bus, uint8_t dev_addr);

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_delete(sl_lm75_handle_t *sensor)
*  @brief       Deletes LM75 sensor handle in the Sensor Hub.
*  @param[in]   sensor: LM75 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_delete(sl_lm75_handle_t *sensor);

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_power_down(sl_lm75_handle_t sensor)
*  @brief       Power downs the LM75 sensor in the Sensor Hub.
*  @param[in]   sensor: LM75 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_power_down(sl_lm75_handle_t sensor);

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_power_on(sl_lm75_handle_t sensor)
*  @brief       Power ON the LM75 sensor in the Sensor Hub.
*  @param[in]   sensor: LM75 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_power_on(sl_lm75_handle_t sensor);

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_get_data(sl_lm75_handle_t sensor,
*                                                                   float *data)
*  @brief       Data from the sensor is received and calculated and stored
*  @param[in]   sensor: LM75 Sensor handle
*  @param[in]   data: Processed LM75 data
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_get_data(sl_lm75_handle_t sensor, float *data);

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_change_measure_time(void)
*  @brief       change the measure time LM75 sensor in the Sensor Hub.
*  @param[in]   Sensor : handle of the sensor
*  @param[in]   measure_time : Measure time of the sensor
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_change_measure_time(sl_lm75_handle_t sensor, uint8_t measure_time);

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_init(sl_sensor_bus_t i2c_bus)
*  @brief       Initializing the LM75 sensor in the Sensor Hub.
*  @param[in]   i2c_bus: LM75 Sensor I2C Bus handle
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_init(sl_sensor_bus_t i2c_bus);

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_deinit(void)
*  @brief       DeInitializing the LM75 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_deinit(void);

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_sample_temperature(float *l
*  @brief       Processes and Reads the value of Temperature sensor and sends
*               the value to Sensor Hub
*  @param[in]   l: Value of Temperature data
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_sample_temperature(float *l);

/*******************************************************************************
*  @fn          sl_lm75_err_t sl_si91x_lm75_test(void)
*  @brief       Testing the LM75 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_lm75_err_t sl_si91x_lm75_test(void);

#ifdef __cplusplus
}
#endif

#endif /* LM75_H_ */
