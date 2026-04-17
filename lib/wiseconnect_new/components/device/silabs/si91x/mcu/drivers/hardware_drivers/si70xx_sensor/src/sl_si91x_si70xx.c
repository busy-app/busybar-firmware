/***************************************************************************/ /**
 * @file sl_si91x_si70xx.c
 * @brief SI70xx Sensor API implementation
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
#include <sl_si91x_si70xx.h>
#include "sl_si91x_clock_manager.h"

/*******************************************************************************
 ***************************   Defines / Macros   ******************************
 ******************************************************************************/
#define RX_LEN           2    ///< Read buffer length 2 bytes
#define TX_LEN           2    ///< Write buffer length 2 bytes
#define RD_BUF           6    ///< Read buffer length 6 bytes
#define WR_BUF           1    ///< Write buffer length 1 byte
#define MS_DELAY_COUNTER 4600 // Delay count

/*******************************************************************************
 ***********************  Local function Prototypes ***************************
 ******************************************************************************/
static sl_status_t si70xx_send_command(sl_i2c_instance_t i2c_instance, uint8_t addr, uint32_t *data, uint8_t command);
static float si70xx_get_celcius_temperature(int32_t temp_data);
static float si70xx_get_percent_relative_humidity(uint32_t rh_data);
static void wait_till_i2c_gets_idle(sl_i2c_instance_t i2c_instance);
static void *si70xx_get_i2c_base_address(sl_i2c_instance_t i2c_instance);

/*******************************************************************************
 * Initializes the Si70xx - RHT sensor by reading electronic ID 1st byte (or)
 *  2nd byte based on eid selected of type \ref sl_si70xx_eid_type_t
 ******************************************************************************/
sl_status_t sl_si91x_si70xx_init(sl_i2c_instance_t i2c_instance, uint8_t addr, sl_si70xx_eid_type_t eid)
{
  // Validate invalid parameters
  if ((i2c_instance >= SL_I2C_LAST) || (eid >= SL_LAST_EID)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // To check EID of sensor before starting measurement
  if (sl_si91x_si70xx_is_present(i2c_instance, addr, eid)) {
    return SL_STATUS_INITIALIZATION;
  }
  return SL_STATUS_OK;
}

/******************************************************************************
 *  Reads Internal Firmware Revision from Si70xx - RHT sensor via I2C.
 *****************************************************************************/
sl_status_t sl_si91x_si70xx_get_firmware_revision(sl_i2c_instance_t i2c_instance,
                                                  uint8_t addr,
                                                  uint8_t *firmware_revision)
{
  sl_status_t status        = SL_STATUS_FAIL;
  uint8_t read_buffer_size  = RX_LEN;
  uint8_t write_buffer_size = TX_LEN;
  uint8_t i2c_read_data[read_buffer_size];
  uint8_t i2c_write_data[write_buffer_size];
  // Firmware command to issue
  i2c_write_data[0] = SL_FIRMWARE_REV1;
  i2c_write_data[1] = SL_FIRMWARE_REV2;
  // Validate invalid parameters
  if (i2c_instance >= SL_I2C_LAST) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Validate NULL parameters
  if (firmware_revision == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Send firmware command to sensor
  status = sl_i2c_driver_send_data_blocking(i2c_instance, addr, i2c_write_data, write_buffer_size);
  if (status != SL_STATUS_OK) {
    return status;
  }
  wait_till_i2c_gets_idle(i2c_instance);
  // Read firmware command response from sensor
  status = sl_i2c_driver_receive_data_blocking(i2c_instance, addr, i2c_read_data, read_buffer_size);
  if (status != SL_STATUS_OK) {
    return status;
  }
  wait_till_i2c_gets_idle(i2c_instance);
  *firmware_revision = i2c_read_data[0];
  return SL_STATUS_OK;
}

/******************************************************************************
 * Starts no hold master mode measurement of relative humidity (or) temperature from Si70xx - RHT sensor.
 *****************************************************************************/
sl_status_t sl_si91x_si70xx_start_no_hold_measure_rh_or_temp(sl_i2c_instance_t i2c_instance,
                                                             uint8_t addr,
                                                             sl_si70xx_measurement_type_t type,
                                                             uint32_t *data)
{
  sl_status_t status             = SL_STATUS_FAIL;
  uint8_t i2c_read_data[RX_LEN]  = { 0 };
  uint8_t i2c_write_data[WR_BUF] = { 0 };
  uint8_t cmd                    = 0;
  // Validate invalid parameters
  if ((i2c_instance >= SL_I2C_LAST) || (type >= SL_LAST_MEASUREMENT)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Check if type is selected as humidity measurement
  if (type == SL_HUMIDITY) {
    cmd = SL_HUMIDITY_NHM;
  }
  // Check if type is selected as temperature measurement
  else if (type == SL_TEMPERATURE) {
    cmd = SL_TEMPERATURE_NHM;
  }
  i2c_write_data[0] = cmd;
  // Send no hold master mode command to sensor
  status = sl_i2c_driver_send_data_blocking(i2c_instance, addr, i2c_write_data, WR_BUF);
  if (status != SL_STATUS_OK) {
    return status;
  }
  wait_till_i2c_gets_idle(i2c_instance);
  // Adding delay as No-Hold sensor command requires some time to respond to the command sent.
  sl_si91x_delay_ms(20);
  // Receive no hold master mode response from sensor
  status = sl_i2c_driver_receive_data_blocking(i2c_instance, addr, i2c_read_data, RX_LEN);
  if (status != SL_STATUS_OK) {
    return status;
  }
  wait_till_i2c_gets_idle(i2c_instance);
  *data = (uint32_t)((i2c_read_data[0] << 8) | (i2c_read_data[1]));
  if (i2c_write_data[0] == SL_HUMIDITY_NHM) {
    // Convert relative humidity measurement to percent relative humidity
    *data = (uint32_t)si70xx_get_percent_relative_humidity(*data);
  } else {
    // Convert temperature measurement to temperature in degrees Celcius
    *data = (int32_t)si70xx_get_celcius_temperature(*data);
  }
  return SL_STATUS_OK;
}

/******************************************************************************
 *  Reads temperature value from previous relative humidity measurement from Si70xx - RHT sensor.
 *****************************************************************************/
sl_status_t sl_si91x_si70xx_read_temp_from_rh(sl_i2c_instance_t i2c_instance,
                                              uint8_t addr,
                                              uint32_t *humid_data,
                                              int32_t *temp_data)
{
  sl_status_t status = SL_STATUS_FAIL;
  // Validate invalid parameters
  if (i2c_instance >= SL_I2C_LAST) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Validate NULL parameters
  if ((humid_data == NULL) || (temp_data == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }
  // send humidity hold master mode command and receive response from sensor
  status = si70xx_send_command(i2c_instance, addr, (uint32_t *)humid_data, SL_HUMIDITY_HM);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Convert relative humidity measurement to percent relative humidity
  *humid_data = (uint32_t)si70xx_get_percent_relative_humidity(*humid_data);
  // send temperature command and receive response from sensor from previous RH
  status = si70xx_send_command(i2c_instance, addr, (uint32_t *)temp_data, SL_TEMPERATURE_AH);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Convert temperature measurement to temperature in degrees Celcius
  *temp_data = (int32_t)si70xx_get_celcius_temperature(*temp_data);
  return SL_STATUS_OK;
}

/******************************************************************************
 *  Measures relative humidity(hold master mode) from Si70xx - RHT sensor.
 *****************************************************************************/
sl_status_t sl_si91x_si70xx_measure_humidity(sl_i2c_instance_t i2c_instance, uint8_t addr, uint32_t *humid_data)
{
  sl_status_t status = SL_STATUS_FAIL;
  // Validate invalid parameters
  if (i2c_instance >= SL_I2C_LAST) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Validate NULL parameters
  if (humid_data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // send humidity hold master mode command and receive response from sensor
  status = si70xx_send_command(i2c_instance, addr, (uint32_t *)humid_data, SL_HUMIDITY_HM);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Convert relative humidity measurement to percent relative humidity
  *humid_data = (uint32_t)si70xx_get_percent_relative_humidity(*humid_data);
  return SL_STATUS_OK;
}

/******************************************************************************
 *  Measures temperature(hold master mode) from Si70xx - RHT sensor.
 *****************************************************************************/
sl_status_t sl_si91x_si70xx_measure_temperature(sl_i2c_instance_t i2c_instance, uint8_t addr, int32_t *temp_data)
{
  sl_status_t status = SL_STATUS_FAIL;
  // Validate invalid parameters
  if (i2c_instance >= SL_I2C_LAST) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Validate NULL parameters
  if (temp_data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // send temperature hold master mode command and receive response from sensor
  status = si70xx_send_command(i2c_instance, addr, (uint32_t *)temp_data, SL_TEMPERATURE_HM);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Convert temperature measurement to temperature in degrees Celcius
  *temp_data = (int32_t)si70xx_get_celcius_temperature(*temp_data);
  return SL_STATUS_OK;
}

/******************************************************************************
 *  Checks if a Si70xx - RHT sensor is present on the I2C bus (or) not by reading
 *   EID bytes from Si70xx - RHT sensor
 *****************************************************************************/
sl_status_t sl_si91x_si70xx_is_present(sl_i2c_instance_t i2c_instance, uint8_t addr, sl_si70xx_eid_type_t eid)
{
  sl_status_t status        = SL_STATUS_FAIL;
  uint8_t read_buffer_size  = RD_BUF;
  uint8_t write_buffer_size = TX_LEN;
  uint8_t i2c_read_data[read_buffer_size];
  uint8_t i2c_write_data[write_buffer_size];
  // Validate invalid parameters
  if ((i2c_instance >= SL_I2C_LAST) || (eid >= SL_LAST_EID)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Check if eid is selected as 1st byte
  if (eid == SL_EID_FIRST_BYTE) {
    i2c_write_data[0] = SL_EID_BYTEL1;
    i2c_write_data[1] = SL_EID_BYTEL2;
  }
  // Check if eid is selected as 2nd byte
  else if (eid == SL_EID_SECOND_BYTE) {
    i2c_write_data[0] = SL_EID_BYTE21;
    i2c_write_data[1] = SL_EID_BYTE22;
  }
  // Send EID byte command to sensor
  status = sl_i2c_driver_send_data_blocking(i2c_instance, addr, i2c_write_data, write_buffer_size);
  if (status != SL_STATUS_OK) {
    return status;
  }
  wait_till_i2c_gets_idle(i2c_instance);
  // Receive response on EID byte command from sensor
  status = sl_i2c_driver_receive_data_blocking(i2c_instance, addr, i2c_read_data, read_buffer_size);
  if (status != SL_STATUS_OK) {
    return status;
  }
  wait_till_i2c_gets_idle(i2c_instance);
  return SL_STATUS_OK;
}

/******************************************************************************
 *  Measures both relative humidity and temperature in same API for hold master mode
 *   from Si70xx - RHT sensor.
 *****************************************************************************/
sl_status_t sl_si91x_si70xx_measure_rh_and_temp(sl_i2c_instance_t i2c_instance,
                                                uint8_t addr,
                                                uint32_t *humid_data,
                                                int32_t *temp_data)
{
  sl_status_t status = SL_STATUS_FAIL;
  // Validate invalid parameters
  if (i2c_instance >= SL_I2C_LAST) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Validate NULL parameters
  if ((humid_data == NULL) || (temp_data == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }
  // send humidity hold master mode command and receive response from sensor
  status = si70xx_send_command(i2c_instance, addr, (uint32_t *)humid_data, SL_HUMIDITY_HM);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Convert relative humidity measurement to percent relative humidity
  *humid_data = (uint32_t)si70xx_get_percent_relative_humidity(*humid_data);
  // send temperature hold master mode command and receive response from sensor
  status = si70xx_send_command(i2c_instance, addr, (uint32_t *)temp_data, SL_TEMPERATURE_HM);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Convert temperature measurement to temperature in degrees Celcius
  *temp_data = (int32_t)si70xx_get_celcius_temperature(*temp_data);
  return SL_STATUS_OK;
}

/******************************************************************************
 * @brief  Sends command to the sensor and reads the result over the I2C bus
 * @param[in] i2c_instance : I2C peripheral to use.
 * @param[in] addr : I2C address of the sensor.
 * @param[out] data : data read from the sensor.
 * @param[in] command : command to send to device.
 * @return returns on SL_STATUS_OK Success
 *****************************************************************************/
static sl_status_t si70xx_send_command(sl_i2c_instance_t i2c_instance, uint8_t addr, uint32_t *data, uint8_t command)
{
  sl_status_t status        = SL_STATUS_FAIL;
  uint8_t read_buffer_size  = RX_LEN;
  uint8_t write_buffer_size = WR_BUF;
  uint8_t i2c_read_data[read_buffer_size];
  uint8_t i2c_write_data[write_buffer_size];
  i2c_write_data[0] = command;
  // Validate invalid parameters
  if (i2c_instance >= SL_I2C_LAST) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Validate NULL parameters
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Send command to sensor
  status = sl_i2c_driver_send_data_blocking(i2c_instance, addr, i2c_write_data, write_buffer_size);
  if (status != SL_STATUS_OK) {
    return status;
  }
  wait_till_i2c_gets_idle(i2c_instance);
  // Receive response from sensor
  status = sl_i2c_driver_receive_data_blocking(i2c_instance, addr, i2c_read_data, read_buffer_size);
  if (status != SL_STATUS_OK) {
    return status;
  }
  wait_till_i2c_gets_idle(i2c_instance);
  *data = (uint32_t)((i2c_read_data[0] << 8) | (i2c_read_data[1]));
  return SL_STATUS_OK;
}

/******************************************************************************
 * @brief  Converts a relative humidity measurement to percent relative humidity
 * @param[in] rh_data : Relative humidity measurement data to convert
 * @return  returns the converted relative humidity data
 *****************************************************************************/
static float si70xx_get_percent_relative_humidity(uint32_t rh_data)
{
  float value = (float)(((125.0 * rh_data) / 65536.0) - 6.0);
  if (value < 0)
    return 0;
  else if (value > 100)
    return 100;
  else
    return value;
}

/******************************************************************************
 * @brief  Converts a temperature measurement to temperature in degrees Celcius
 * @param[in] temp_data : Temperature measurement data to convert
 * @return returns the converted temperature measurement
 *****************************************************************************/
static float si70xx_get_celcius_temperature(int32_t temp_data)
{
  return (float)(((175.72 * temp_data) / 65536.0) - 46.85);
}

/******************************************************************************
 * Resets the si70xx - RHT sensor by sending sensor reset command via I2C
 *****************************************************************************/
sl_status_t sl_si91x_si70xx_reset(sl_i2c_instance_t i2c_instance, uint8_t addr)
{
  sl_status_t status        = SL_STATUS_FAIL;
  uint8_t write_buffer_size = WR_BUF;
  uint8_t i2c_write_data[write_buffer_size];
  i2c_write_data[0] = SL_SI70XX_RESET;
  // Validate invalid parameters
  if (i2c_instance >= SL_I2C_LAST) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Send sensor reset command to sensor
  status = sl_i2c_driver_send_data_blocking(i2c_instance, addr, i2c_write_data, write_buffer_size);
  if (status != SL_STATUS_OK) {
    return status;
  }
  wait_till_i2c_gets_idle(i2c_instance);
  return SL_STATUS_OK;
}

/******************************************************************************
 * Reads data from user register 1 and heater control register of the si70xx - RHT sensor
 *****************************************************************************/
sl_status_t sl_si91x_si70xx_read_control_register(sl_i2c_instance_t i2c_instance,
                                                  uint8_t addr,
                                                  sl_si70xx_registers_t reg,
                                                  uint8_t *data)
{
  sl_status_t status        = SL_STATUS_FAIL;
  uint8_t cmd               = 0;
  uint8_t read_buffer_size  = RX_LEN;
  uint8_t write_buffer_size = WR_BUF;
  uint8_t i2c_write_data[write_buffer_size];
  uint8_t i2c_read_data[read_buffer_size];
  // Validate invalid parameters
  if ((i2c_instance >= SL_I2C_LAST) || (reg >= SL_LAST_CONTROL_REG)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Validate NULL parameters
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Check if user register 1/ heater control register command is selected
  if (reg == SL_RH_T_USER_REG) {
    cmd = SL_R_RHT_U_REG;
  } else if (reg == SL_HEATER_CONTROL_REG) {
    cmd = SL_R_HEATER_C_REG;
  }
  i2c_write_data[0] = cmd;
  // Send user register 1/ heater control register command to sensor
  status = sl_i2c_driver_send_data_blocking(i2c_instance, addr, i2c_write_data, write_buffer_size);
  if (status != SL_STATUS_OK) {
    return status;
  }
  wait_till_i2c_gets_idle(i2c_instance);
  // Receive response for user register 1/ heater control register command from sensor
  status = sl_i2c_driver_receive_data_blocking(i2c_instance, addr, i2c_read_data, read_buffer_size);
  if (status != SL_STATUS_OK) {
    return status;
  }
  wait_till_i2c_gets_idle(i2c_instance);
  *data = i2c_read_data[0];
  return SL_STATUS_OK;
}

/******************************************************************************
 * Writes data to user register 1 and heater control register of the si70xx - RHT sensor
 *****************************************************************************/
sl_status_t sl_si91x_si70xx_write_control_register(sl_i2c_instance_t i2c_instance,
                                                   uint8_t addr,
                                                   sl_si70xx_registers_t reg,
                                                   uint8_t value)
{
  uint8_t cmd               = 0;
  sl_status_t status        = SL_STATUS_FAIL;
  uint8_t write_buffer_size = TX_LEN;
  uint8_t i2c_write_data[write_buffer_size];
  // Validate invalid parameters
  if ((i2c_instance >= SL_I2C_LAST) || (reg >= SL_LAST_CONTROL_REG)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Check if user register 1/ heater control register command is selected
  if (reg == SL_RH_T_USER_REG) {
    cmd = SL_W_RHT_U_REG;
  } else if (reg == SL_HEATER_CONTROL_REG) {
    cmd = SL_W_HEATER_C_REG;
  }
  i2c_write_data[0] = cmd;
  // write data into register
  i2c_write_data[1] = value;
  // Send user register 1/ heater control register command to sensor
  status = sl_i2c_driver_send_data_blocking(i2c_instance, addr, i2c_write_data, write_buffer_size);
  if (status != SL_STATUS_OK) {
    return status;
  }
  wait_till_i2c_gets_idle(i2c_instance);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Function to wait till I2C comes to idle state
 *
 * @param i2c (I2C_TypeDef) Pointer to the I2C base address.
 * @return none
 ******************************************************************************/
static void wait_till_i2c_gets_idle(sl_i2c_instance_t i2c_instance)
{
  I2C0_Type *i2c;

  // Updating i2c pointer as per instance number
  i2c = (I2C0_Type *)si70xx_get_i2c_base_address(i2c_instance);
  // Checking I2C ACTIVITY bit status
  while (i2c->IC_STATUS_b.ACTIVITY)
    ;
}

/*******************************************************************************
 * Function to return pointer to I2C instance base address as per I2C instance
 *number
 *
 * @param i2c_instance   I2C Instance \ref sl_i2c_instance_t
 * @return Pointer(void*) to i2c instance base address
 ******************************************************************************/
static void *si70xx_get_i2c_base_address(sl_i2c_instance_t i2c_instance)
{
  I2C0_Type *i2c = NULL;
  // Updating i2c pointer as per instance number
  if (i2c_instance == SL_I2C0) {
    i2c = ((I2C0_Type *)I2C0_BASE);
  } else if (i2c_instance == SL_I2C1) {
    i2c = ((I2C0_Type *)I2C1_BASE);
  } else if (i2c_instance == SL_ULP_I2C) {
    i2c = ((I2C0_Type *)I2C2_BASE);
  }
  return i2c;
}
