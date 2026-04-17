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
#include <stddef.h>
#include "sl_sht4x.h"
#include "sl_i2cspm.h"
#include "sl_sleeptimer.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/** Measure Temp and Relative Humidity in High Precision Mode */
#define SHT4X_CMD_MEASURE_HPM    0xFD
/** Measure Temp and Relative Humidity in Low Precision Mode */
#define SHT4X_CMD_MEASURE_LPM    0xE0
/*Read the serial data */
#define SHT4X_CMD_READ_SERIAL    0x89

static uint8_t sht4x_cmd_measure;
static uint8_t sht4x_cmd_read_serial = SHT4X_CMD_READ_SERIAL;
/** @endcond */

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Local prototypes
static sl_status_t sl_sht4x_send_command(sl_i2cspm_t *i2cspm, uint8_t addr, uint8_t command);
static int32_t sl_sht4x_get_celsius_temperature(int32_t temp_data);
static uint32_t sl_sht4x_get_percent_relative_humidity(uint32_t rh_data);
static sl_status_t sl_sht4x_read_data(sl_i2cspm_t *i2cspm, uint8_t addr, uint64_t *data);
/** @endcond */

/***************************************************************************//**
 *    Initializes the SHT4x sensor
 ******************************************************************************/
sl_status_t sl_sht4x_init(sl_i2cspm_t *i2cspm, uint8_t addr)
{
  sl_status_t status = SL_STATUS_OK;

  /* check if the sensor is ready */
  status = sl_sht4x_present(i2cspm, addr, NULL);

  if (status != SL_STATUS_OK) {
    /* Wait for sensor to become ready */
    sl_sleeptimer_delay_millisecond(80);

    if (sl_sht4x_present(i2cspm, addr, NULL) != SL_STATUS_OK) {
      status = SL_STATUS_INITIALIZATION;
    } else {
      status = SL_STATUS_OK;
    }
  }

  return status;
}

/**************************************************************************//**
 *  Measures relative humidity and temperature from a SHT4x sensor.
 *****************************************************************************/
sl_status_t sl_sht4x_measure_rh_and_temp(sl_i2cspm_t *i2cspm, uint8_t addr, uint32_t *rhData,
                                         int32_t *tData)
{
  sl_status_t retval;
  uint64_t Data;

  /* Measure in High Precision mode*/
  sl_sht4x_enable_low_power_mode(false, &sht4x_cmd_measure);

  /* Send the command to measure the Relative Humidity and temperature data*/
  retval = sl_sht4x_send_command(i2cspm, addr, sht4x_cmd_measure);

  if (retval != SL_STATUS_OK) {
    return retval;
  }

  /* Wait for sensor to get ready */
  sl_sleeptimer_delay_millisecond(10);

  /* Read the Temp and Relative Humidity values*/
  retval = sl_sht4x_read_data(i2cspm, addr, &Data);

  *tData = (Data >> 32) & 0xFFFF;
  *rhData = (Data >> 8) & 0xFFFF;

  /* Convert the relative humidity measurement to percent relative humidity*/
  *rhData = sl_sht4x_get_percent_relative_humidity(*rhData);

  /* Convert the temperature measurement to temperature in degrees Celsius*/
  *tData = sl_sht4x_get_celsius_temperature(*tData);

  return retval;
}

/**************************************************************************//**
 *  Reads the raw relative humidity and temperature from a SHT4x sensor.
 *  Data is of [6 bytes = 2 * 8-bit Temp-data + 8-bit CRC + 2 * 8-bit RH-data + 8-bit CRC]
 *****************************************************************************/
sl_status_t sl_sht4x_read_rh_and_temp(sl_i2cspm_t *i2cspm, uint8_t addr, uint32_t *rhData,
                                      int32_t *tData)
{
  sl_status_t retval;
  uint64_t Data;

  /* Measure in High Precision mode*/
  sl_sht4x_enable_low_power_mode(false, &sht4x_cmd_measure);

  /* Send the command to measure the Relative Humidity and temperature data*/
  retval = sl_sht4x_send_command(i2cspm, addr, sht4x_cmd_measure);

  if (retval != SL_STATUS_OK) {
    return retval;
  }

  /* Wait for sensor to become ready */
  sl_sleeptimer_delay_millisecond(10);

  /* Read the Temp and Relative Humidity values */
  retval = sl_sht4x_read_data(i2cspm, addr, &Data);

  *tData = (Data >> 32) & 0xFFFF;
  *rhData = (Data >> 8) & 0xFFFF;

  return retval;
}

/**************************************************************************//**
 *   Checks if a SHT4x is present on the I2C bus or not.
 *   device_id is of [6 bytes = 2 * 8-bit data + 8-bit CRC + 2 * 8-bit data + 8-bit CRC]
 *****************************************************************************/
sl_status_t sl_sht4x_present(sl_i2cspm_t *i2cspm, uint8_t addr, uint64_t *device_id)
{
  sl_status_t               retval;
  I2C_TransferSeq_TypeDef   seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t                   i2c_read_data[6];

  /* Send the command to read the unique serial number of the sensor */
  retval = sl_sht4x_send_command(i2cspm, addr, sht4x_cmd_read_serial);
  if (retval != SL_STATUS_OK) {
    return retval;
  }

  /* Give the device a short time before reading */
  sl_sleeptimer_delay_millisecond(1);

  seq.addr  = addr << 1;
  seq.flags = I2C_FLAG_READ;
  seq.buf[0].data = i2c_read_data;
  seq.buf[0].len  = 6;

  ret = I2CSPM_Transfer(i2cspm, &seq);

  if (ret != i2cTransferDone) {
    *device_id = 0;
    return SL_STATUS_TRANSMIT;
  }

  if (device_id != NULL) {
    /* Combine the 6 bytes of the read data into a 64-bit variable */
    *device_id = (uint64_t)((uint32_t)i2c_read_data[0] << 24)
                | (uint64_t)((uint32_t)i2c_read_data[1] << 16)
                | (uint64_t)((uint32_t)i2c_read_data[3] << 8)
                | (uint64_t)i2c_read_data[4];
  }

  return SL_STATUS_OK;
}

/**************************************************************************//**
 *   Enables the low-power mode in SHT4x sensor.
 *****************************************************************************/
void sl_sht4x_enable_low_power_mode(bool enable_low_power_mode, uint8_t *sht4x_cmd_measure)
{
  if (enable_low_power_mode) {
    /* Measure in Low Precision Mode*/
    *sht4x_cmd_measure = SHT4X_CMD_MEASURE_LPM;
  } else {
    /* Measure in High Precision Mode*/
    *sht4x_cmd_measure = SHT4X_CMD_MEASURE_HPM;
  }
}

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
/**************************************************************************//**
 * @brief
 *  Sends a command and reads the result over the I2C bus
 * @param[in] i2cspm
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address of the sensor.
 * @param[out] data
 *   The data read from the sensor.
 * @param[in] command
 *   The command to send to device. See the \#define's for details.
 * @return
 *    @retval SL_STATUS_OK Success
 *    @retval SL_STATUS_TRANSMIT I2C transmission error
 *****************************************************************************/
static sl_status_t sl_sht4x_send_command(sl_i2cspm_t *i2cspm, uint8_t addr, uint8_t command)
{
  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t                    i2c_write_data[1];
  uint8_t                    i2c_read_data[1];

  seq.addr  = addr << 1;
  seq.flags = I2C_FLAG_WRITE;
  /* Select command to issue */
  i2c_write_data[0] = command;
  seq.buf[0].data   = i2c_write_data;
  seq.buf[0].len    = 1;
  /* No data to be read */
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len  = 0;

  ret = I2CSPM_Transfer(i2cspm, &seq);

  if (ret != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/**************************************************************************//**
 * @brief
 *  Reads data from the SHT4x sensor.
 * @param[in] i2cspm
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address of the sensor.
 * @param[out] data
 *   The data read from the sensor.
 * @return
 *    @retval SL_STATUS_OK Success
 *    @retval SL_STATUS_TRANSMIT I2C transmission error
 *****************************************************************************/
static sl_status_t sl_sht4x_read_data(sl_i2cspm_t *i2cspm, uint8_t addr, uint64_t *data)
{
  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t                    i2c_read_data[6];

  seq.addr  = addr << 1;
  seq.flags = I2C_FLAG_READ;
  /* Select location/length of data to be read */
  seq.buf[0].data = i2c_read_data;
  seq.buf[0].len  = 6;

  ret = I2CSPM_Transfer(i2cspm, &seq);

  if (ret != i2cTransferDone) {
    *data = 0;
    return SL_STATUS_TRANSMIT;
  }

  // Combine the 6 bytes into a 64-bit variable
  *data = ((uint64_t)i2c_read_data[0] << 40)
          | ((uint64_t)i2c_read_data[1] << 32)
          | ((uint64_t)i2c_read_data[2] << 24)
          | ((uint64_t)i2c_read_data[3] << 16)
          | ((uint64_t)i2c_read_data[4] << 8)
          | (uint64_t)i2c_read_data[5];

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * @brief
 *   Converts a relative humidity measurement to percent relative humidity
 *   (multiplied by 1000)
 *   Formula for conversion of the sensor signals, optimized for fixed
 *   point algebra:
 *   Relative Humidity = 125 * (SHT4x_RH / 65535) - 6
 * @param[in] rh_data
 *   Relative humidity measurement data to convert
 * @return
 *   The converted relative humidity data
 *****************************************************************************/
static uint32_t sl_sht4x_get_percent_relative_humidity(uint32_t rh_data)
{
  return (((rh_data) * 15625L) >> 13) - 6000;
}

/**************************************************************************//**
 * @brief
 *   Converts a temperature measurement to temperature in degrees Celsius
 *   (multiplied by 1000)
 *   Formula for conversion of the sensor signals, optimized for fixed
 *   point algebra:
 *   Temperature = 175 * (SHT4x_T / 65535) - 45
 * @param[in] temp_data
 *   Temperature measurement data to convert
 * @return
 *   The converted temperature measurement
 *****************************************************************************/
static int32_t sl_sht4x_get_celsius_temperature(int32_t temp_data)
{
  return (((temp_data) * 21875L) >> 13) - 45000;
}
/** @endcond */
