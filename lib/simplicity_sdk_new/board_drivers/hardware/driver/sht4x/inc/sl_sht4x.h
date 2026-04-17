/***************************************************************************//**
 * @file
 * @brief Driver for the SHT4X Relative Humidity and Temperature
 * sensor
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SHT4X_H
#define SL_SHT4X_H

#include <stdbool.h>
#include "sl_status.h"
#include "sl_i2cspm.h"

/***************************************************************************//**
 * @addtogroup sht4x SHT4x - RHT Sensor
 * @brief SHT4x Relative Humidity and Temperature Sensor I2C driver.

   @n @section sht4x_example SHT4X example code

   Basic example for relative humidity and temperature measurement using
   an SHT4x sensor: @n @n
   @verbatim

 #include "sl_i2cspm_instances.h"
 #include "sl_sht4x.h"

   int main( void )
   {

   ...

   int32_t temp_data;
   uint32_t rh_data;

   sl_sht4x_init(sl_i2cspm_sensor, SHT4X_ADDR);
   sl_sht4x_measure_rh_and_temp(sl_i2cspm_sensor, SHT4X_ADDR, &rh_data, &temp_data);

   ...

   } @endverbatim
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @name Sensor Defines
 * @{
 ******************************************************************************/
/** I2C device address for SHT4X */
#define SHT4X_ADDR      0x44

/** @}  */

/**************************************************************************//**
 * @brief
 *   Initialize the SHT4x sensor.
 * @param[in] i2cspm
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to probe.
 * @retval SL_STATUS_OK An SHT4x device is present on the I2C bus
 * @retval SL_STATUS_INITIALIZATION No SHT4x device present
 *****************************************************************************/
sl_status_t sl_sht4x_init(sl_i2cspm_t *i2cspm, uint8_t addr);

/**************************************************************************//**
 * @brief
 *   Check whether an SHT4x sensor is present on the I2C bus or not.
 * @param[in] i2cspm
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to probe.
 * @param[out] device_id
 *   Write device ID if device responds. Pass in NULL to discard.
 *   Each SHT4x sensor has unique 48-bit serial number.
 * @retval SL_STATUS_OK An SHT4x device responded and device_id is valid (if non-NULL)
 * @retval SL_STATUS_TRANSMIT I2C transmission error or no device response
 *****************************************************************************/
sl_status_t sl_sht4x_present(sl_i2cspm_t *i2cspm, uint8_t addr, uint64_t *device_id);

/**************************************************************************//**
 * @brief
 *  Measure relative humidity and temperature from an SHT4x sensor.
 * @param[in] i2cspm
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address of the sensor.
 * @param[out] rhData
 *   The relative humidity in percent (multiplied by 1000).
 * @param[out] tData
 *   The temperature in degree Celsius (multiplied by 1000).
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_TRANSMIT I2C transmission error
 *****************************************************************************/
sl_status_t sl_sht4x_measure_rh_and_temp(sl_i2cspm_t *i2cspm, uint8_t addr,
                                         uint32_t *rhData, int32_t *tData);

/**************************************************************************//**
 * @brief
 *  Read relative humidity and temperature from an SHT4x sensor.
 * @param[in] i2cspm
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address of the sensor.
 * @param[out] rhData
 *   The relative humidity in percent.
 * @param[out] tData
 *   The temperature in degree Celsius.
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_TRANSMIT I2C transmission error
 *****************************************************************************/
sl_status_t sl_sht4x_read_rh_and_temp(sl_i2cspm_t *i2cspm, uint8_t addr,
                                      uint32_t *rhData, int32_t *tData);

/***************************************************************************//**
 * @brief
 *    Enables the low power mode.
 * @param[in] enable_low_power_mode
 *    If true, enables the SHT4x sensor in low power mode.
 ******************************************************************************/
void sl_sht4x_enable_low_power_mode(bool enable_low_power_mode, uint8_t *sht4x_cmd_measure);

#ifdef __cplusplus
}
#endif

/**@}* (sht4x) */

#endif /* SL_SHT4X_H */
