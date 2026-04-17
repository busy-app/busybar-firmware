/***************************************************************************/ /**
 * @file bh170.h
 * @brief BH170 API implementation
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
#ifndef BH1750FVI_H_
#define BH1750FVI_H_
#include <stdbool.h>
#include "rsi_data_types.h"
#include "sensors_config.h"
#include "sensor_type.h"

/*******************************************************************************
 ************************ BH170 Typedefs/ Defines  *****************************
 ******************************************************************************/
#define SL_BH1750_I2C_ADDRESS_DEFAULT   (0x23) //BH170 Sensor address
#define SL_BH_1750_MEASUREMENT_ACCURACY 1.2    //The typical measurement accuracy of  BH1750 sensor
typedef int32_t sl_bh170_err_t;                //BH170 error define
typedef void *sl_bh1750_handle_t;              //Bh170 error handle
typedef void *sl_i2c_bus_handle_t;             //I2C bus handle

/*******************************************************************************
 **************** Global Enums/Structures for BH170 Sensor  ********************
 ******************************************************************************/
enum bh170_sensor_en {
  SL_BH170_FAIL = -1, //bh170 sensor fail
  SL_BH170_OK   = 0   //bh170 sensor success
};

typedef enum {
  SL_BH1750_CONTINUE_1LX_RES    = 0x10, /*!< Command to set measure mode as Continuously H-Resolution mode*/
  SL_BH1750_CONTINUE_HALFLX_RES = 0x11, /*!< Command to set measure mode as Continuously H-Resolution mode2*/
  SL_BH1750_CONTINUE_4LX_RES    = 0x13, /*!< Command to set measure mode as Continuously L-Resolution mode*/
  SL_BH1750_ONETIME_1LX_RES     = 0x20, /*!< Command to set measure mode as One Time H-Resolution mode*/
  SL_BH1750_ONETIME_HALFLX_RES  = 0x21, /*!< Command to set measure mode as One Time H-Resolution mode2*/
  SL_BH1750_ONETIME_4LX_RES     = 0x23, /*!< Command to set measure mode as One Time L-Resolution mode*/
} sl_bh1750_cmd_measure_t;

typedef struct {
  int32_t i2c_dev;  //i2c device
  uint8_t dev_addr; //device address
} sl_bh1750_dev_t;

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*  @fn          sl_bh1750_handle_t sl_si91x_bh1750_create(sl_sensor_bus_t bus,
*                                                              uint8_t dev_addr)
*  @brief       Creates BH1750 sensor in the Sensor Hub.
*  @param[in]   bus:  I2C bus handle
*  @param[in]   dev_addr: BH1750 I2C secondary address
*  @return      Returns the BH1750 sensor handle
*******************************************************************************/
sl_bh1750_handle_t sl_si91x_bh1750_create(sl_sensor_bus_t bus, uint8_t dev_addr);

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_delete(sl_bh1750_handle_t *sensor)
*  @brief       Deletes BH1750 sensor handle in the Sensor Hub.
*  @param[in]   sensor: BH1750 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_delete(sl_bh1750_handle_t *sensor);

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_power_down
*                                                    (sl_bh1750_handle_t sensor)
*  @brief       Power down the BH1750 sensor in the Sensor Hub.
*  @param[in]   sensor: BH1750 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_power_down(sl_bh1750_handle_t sensor);

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_power_on
*                                                    (sl_bh1750_handle_t sensor)
*  @brief       Power ON the BH1750 sensor in the Sensor Hub.
*  @param[in]   sensor: BH1750 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_power_on(sl_bh1750_handle_t sensor);

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_reset_data_register
*                                                    (sl_bh1750_handle_t sensor)
*  @brief       Resets the data register
*  @param[in]   sensor: BH1750 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_reset_data_register(sl_bh1750_handle_t sensor);

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_set_measure_mode
*               (sl_bh1750_handle_t sensor, sl_bh1750_cmd_measure_t cmd_measure)
*  @brief       Sets the Measure Mode
*  @param[in]   sensor: BH1750 Sensor handle
*  @param[in]   cmd_measure Command Measure
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_set_measure_mode(sl_bh1750_handle_t sensor, sl_bh1750_cmd_measure_t cmd_measure);

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_get_data
*                                       (sl_bh1750_handle_t sensor, float *data)
*  @brief       Data from the sensor is received calculated and stored
*  @param[in]   sensor: BH1750 Sensor handle
*  @param[in]   data: Processed BH1750 data
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_get_data(sl_bh1750_handle_t sensor, float *data);

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
                                                   float *data);

/*******************************************************************************
*  @fn          sl_si91x_bh1750_change_measure_time(sl_bh1750_handle_t sensor,
*                                               uint8_t measure_time);
*  @brief       Get light intensity
*  @param[in]   sensor: BH1750 Sensor handle
*  @param[in]   measure_time: Measure time
*  @param[in]   data: Processed BH1750 light intensity data
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_change_measure_time(sl_bh1750_handle_t sensor, uint8_t measure_time);

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_init(sl_sensor_bus_t i2c_bus)
*  @brief       Initializing the BH1750 sensor in the Sensor Hub.
*  @param[in]   i2c_bus: BH1750 Sensor I2C Bus handle
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_init(sl_sensor_bus_t i2c_bus);

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_deinit(void)
*  @brief       DeInitializing the BH1750 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_deinit(void);

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_test(void)
*  @brief       Testing the BH1750 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_test(void);

/*******************************************************************************
*  @fn          sl_bh170_err_t sl_si91x_bh1750_sample_light(float *light_data)
*  @brief       Processes and Reads the value of the Light sensor and sends
*               The value of Sensor Hub
*  @param[in]   light_data: Value of Light data
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_bh170_err_t sl_si91x_bh1750_sample_light(float *light_data);

#ifdef __cplusplus
}
#endif

#endif /* BH1750FVI_H_ */
