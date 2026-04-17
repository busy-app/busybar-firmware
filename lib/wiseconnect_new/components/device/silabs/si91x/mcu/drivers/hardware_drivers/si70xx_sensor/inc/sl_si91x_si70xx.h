/***************************************************************************/ /**
 * @file sl_si91x_si70xx.h
 * @brief SI70xx sensor API implementation
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
#ifndef SL_SI91X_SI70XX_H
#define SL_SI91X_SI70XX_H

#ifdef __cplusplus
extern "C" {
#endif

//// Includes
#include "sl_si91x_si70xx_config.h"
#include "sl_status.h"
#include "sl_si91x_i2c.h"

/***************************************************************************/
/**
 * @addtogroup SI70XX Si70XX Humidity and Temperature Sensor
 * @ingroup SI91X_HARDWARE_DRIVER_APIS
 * @{
 *
 ******************************************************************************/
/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define SI70XX_I2C_INSTANCE SL_SI70XX_CONFIG_I2C_INSTANCE ///<si70xx I2C Instance
#define SI70XX_SLAVE_ADDR   SL_SI70XX_CONFIG_SLAVE_ADDR   ///<si70xx Slave Address

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/
/***************************************************************************/
/**
 * @brief Enum for Si70xx commands.
 * 
 * @details This enumeration defines the command codes used to interact with the Si70xx sensor family. 
 *          These commands are used to measure humidity and temperature, read and write user registers, 
 *          control the heater, and read the electronic ID and firmware revision.
 */
typedef enum sl_si70xx_commands {
  SL_HUMIDITY_HM     = 0xE5, ///< Measure Relative Humidity, Hold Master Mode
  SL_HUMIDITY_NHM    = 0xF5, ///< Measure Relative Humidity, No Hold Master Mode
  SL_TEMPERATURE_HM  = 0xE3, ///< Measure Temperature, Hold Master Mode
  SL_TEMPERATURE_NHM = 0xF3, ///< Measure Temperature, No Hold Master Mode
  SL_TEMPERATURE_AH  = 0xE0, ///< Read Temperature Value from Previous RH Measurement
  SL_SI70XX_RESET    = 0xFE, ///< Si70XX Reset
  SL_W_RHT_U_REG     = 0xE6, ///< Write RH/T User Register 1
  SL_R_RHT_U_REG     = 0xE7, ///< Read RH/T User Register 1
  SL_W_HEATER_C_REG  = 0x51, ///< Write Heater Control Register
  SL_R_HEATER_C_REG  = 0x11, ///< Read Heater Control Register
  SL_EID_BYTEL1      = 0xFA, ///< Read Electronic ID 1st Byte, first part
  SL_EID_BYTEL2      = 0x0F, ///< Read Electronic ID 1st Byte, second part
  SL_EID_BYTE21      = 0xFC, ///< Read Electronic ID 2nd Byte, first part
  SL_EID_BYTE22      = 0xC9, ///< Read Electronic ID 2nd Byte, second part
  SL_FIRMWARE_REV1   = 0x84, ///< Read Firmware Revision, first part
  SL_FIRMWARE_REV2   = 0xB8  ///< Read Firmware Revision, second part
} sl_si70xx_commands_t;

/***************************************************************************/
/**
 * @brief Enum for Temperature and Humidity measurement types.
 * 
 * @details This enumeration defines the types of measurements that can be performed by the Si70xx sensor.
 *          It includes options for measuring humidity and temperature, as well as a validation enum.
 */
typedef enum sl_si70xx_measurement_type {
  SL_HUMIDITY,        ///< Enumerator for humidity selection
  SL_TEMPERATURE,     ///< Enumerator for temperature selection
  SL_LAST_MEASUREMENT ///< Last enum for validation
} sl_si70xx_measurement_type_t;

/***************************************************************************/
/**
 * @brief To specify the type of electronic ID byte to be sent.
 * 
 * @details This enumeration defines the types of electronic ID bytes that can be sent to the Si70xx sensor.
 *          It includes options for sending the first and second bytes of the electronic ID, as well as a validation enum.
 */
typedef enum sl_si70xx_eid_type {
  SL_EID_FIRST_BYTE,  ///< Enumerator for sending the first byte of the electronic ID
  SL_EID_SECOND_BYTE, ///< Enumerator for sending the second byte of the electronic ID
  SL_LAST_EID         ///< Last enum for validation
} sl_si70xx_eid_type_t;

/***************************************************************************/
/**
 * @brief To specify the RH/T (Relative Humidity & Temperature) User Register and Heater Control Registers.
 * 
 * @details This enumeration defines the register types used for configuring the Si70xx sensor. 
 *          It includes options for the RH/T user register and the heater control register, as well as a validation enum.
 */
typedef enum sl_si70xx_registers {
  SL_RH_T_USER_REG,      ///< Enumerator for RH/T user register selection
  SL_HEATER_CONTROL_REG, ///< Enumerator for heater control register selection
  SL_LAST_CONTROL_REG    ///< Last enum for validation
} sl_si70xx_registers_t;

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************/
/**
 * @brief To initialize the Si70xx sensor.
 * 
 * @details This API initializes the Si70xx sensor and must be called before using other functions.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_si70xx_reset()
 * 
 * @param[in] i2c_instance Select the I2C instance from sl_i2c_instance_t.
 * @param[in] addr I2C address for Si70xx.
 * @param[in] eid Electronic ID of type \ref sl_si70xx_eid_type_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INITIALIZATION - No Si70xx device present.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_si70xx_init(sl_i2c_instance_t i2c_instance, uint8_t addr, sl_si70xx_eid_type_t eid);

/***************************************************************************/
/**
 * @brief To check whether an Si7006/13/20/21 is present on the I2C bus.
 * 
 * @details This API checks whether an Si7006/13/20/21 sensor is present on the I2C bus based on the provided Electronic ID (EID).
 *          If `SL_EID_FIRST_BYTE` is selected, the first byte of the EID is considered.
 *          If `SL_EID_SECOND_BYTE` is selected, the second byte of the EID is considered.
 *          For details on EID commands, see the sensor's datasheet.
 *          The device ID from SNB_3 is written if the device responds. Pass NULL to discard.
 *          Expected values: 0x0D for Si7013, 0x14 for Si7020, or 0x15 for Si7021.
 * 
 * @param[in] i2c_instance Select the I2C instance from sl_i2c_instance_t.
 * @param[in] addr I2C device address for Si70xx.
 * @param[in] eid Electronic ID of type \ref sl_si70xx_eid_type_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_si70xx_is_present(sl_i2c_instance_t i2c_instance, uint8_t addr, sl_si70xx_eid_type_t eid);

/***************************************************************************/
/**
 * @brief To measure relative humidity and temperature from Si7006/13/20/21 sensor.
 * 
 * @details This API is used to measure relative humidity in percentage and temperature in degrees Celsius from the Si70xx sensor.
 *          The relative humidity and temperature values are obtained after conversion as per the formula mentioned in the datasheet.
 * 
 * @param[in] i2c_instance Select the I2C instance from sl_i2c_instance_t.
 * @param[in] addr I2C device address for Si70xx.
 * @param[out] humid_data The relative humidity in percentage.
 * @param[out] temp_data The temperature in degrees Celsius.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_si70xx_measure_rh_and_temp(sl_i2c_instance_t i2c_instance,
                                                uint8_t addr,
                                                uint32_t *humid_data,
                                                int32_t *temp_data);

/***************************************************************************/
/**
 * @brief To read the firmware revision from the Si7006/13/20/21 sensor.
 * 
 * @details This API reads the firmware revision from the Si70xx sensor. The firmware revision provides information about the internal firmware version of the sensor.
 * 
 * @param[in] i2c_instance Select the I2C instance from sl_i2c_instance_t.
 * @param[in] addr I2C device address for Si70xx.
 * @param[out] firmware_revision Pointer to store the internal firmware revision.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_si70xx_get_firmware_revision(sl_i2c_instance_t i2c_instance,
                                                  uint8_t addr,
                                                  uint8_t *firmware_revision);

/***************************************************************************/
/**
 * @brief To read the temperature value from the previous relative humidity measurement from the Si7006/13/20/21 sensor.
 * 
 * @details This API reads the temperature (in degrees Celsius) from the previous relative humidity measurement (in percent) from the Si70xx sensor.
 * 
 * @param[in] i2c_instance Select the I2C instance from sl_i2c_instance_t.
 * @param[in] addr I2C device address for Si70xx.
 * @param[out] humid_data The relative humidity in percent.
 * @param[out] temp_data The temperature in degrees Celsius.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_si70xx_read_temp_from_rh(sl_i2c_instance_t i2c_instance,
                                              uint8_t addr,
                                              uint32_t *humid_data,
                                              int32_t *temp_data);

/***************************************************************************/
/**
 * @brief To start a no-hold measurement of relative humidity or temperature from the Si7006/13/20/21 sensor.
 * 
 * @details This API starts a no-hold measurement (not acknowledging read requests) of relative humidity or temperature from the Si70xx sensor.
 *          The measurement type is specified by the `type` parameter, which can be either relative humidity or temperature.
 * 
 * @param[in] i2c_instance Select the I2C instance from sl_i2c_instance_t.
 * @param[in] addr I2C device address for Si70xx.
 * @param[in] type Measurement type of \ref sl_si70xx_measurement_type_t.
 * @param[out] data The data read from the sensor.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_si70xx_start_no_hold_measure_rh_or_temp(sl_i2c_instance_t i2c_instance,
                                                             uint8_t addr,
                                                             sl_si70xx_measurement_type_t type,
                                                             uint32_t *data);

/***************************************************************************/
/**
 * @brief To measure relative humidity from the Si7006/13/20/21 sensor.
 * 
 * @details This API is used to measure the relative humidity from the Si70xx sensor. The relative humidity value is returned in the `humid_data` parameter.
 * 
 * @param[in] i2c_instance Select the I2C instance from sl_i2c_instance_t.
 * @param[in] addr I2C device address for Si70xx.
 * @param[out] humid_data The relative humidity measurement.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_si70xx_measure_humidity(sl_i2c_instance_t i2c_instance, uint8_t addr, uint32_t *humid_data);

/***************************************************************************/
/**
 * @brief To measure temperature from the Si7006/13/20/21 sensor.
 * 
 * @details This API is used to measure the temperature from the Si70xx sensor. The temperature value is returned in the `temp_data` parameter.
 * 
 * @param[in] i2c_instance Select the I2C instance from sl_i2c_instance_t.
 * @param[in] addr I2C device address for Si70xx.
 * @param[out] temp_data The temperature measurement.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_si70xx_measure_temperature(sl_i2c_instance_t i2c_instance, uint8_t addr, int32_t *temp_data);

/***************************************************************************/
/**
 * @brief To initiate a software reset for the Si70xx sensor.
 * 
 * @details This API initiates a software reset for the Si70xx sensor using the appropriate command. 
 *          It is used to reset the sensor to its default state.
 * 
 * @param[in] i2c_instance Select the I2C instance from sl_i2c_instance_t.
 * @param[in] addr I2C device address for Si70xx.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_si70xx_reset(sl_i2c_instance_t i2c_instance, uint8_t addr);

/***************************************************************************/
/**
 * @brief To read the RH/T user register and heater control register data from the Si70xx sensor.
 * 
 * @details This API reads the RH/T user register and heater control register data from the Si70xx sensor.
 *          The register to be read is specified by the `reg` parameter, which can be either the RH/T user register or the heater control register.
 * 
 * @param[in] i2c_instance Select the I2C instance from sl_i2c_instance_t.
 * @param[in] addr I2C device address for Si70xx.
 * @param[in] reg Register of type \ref sl_si70xx_registers_t.
 * @param[out] data The data read from the sensor.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_si70xx_read_control_register(sl_i2c_instance_t i2c_instance,
                                                  uint8_t addr,
                                                  sl_si70xx_registers_t reg,
                                                  uint8_t *data);

/***************************************************************************/
/** 
 * @brief To write data to RH/T user register and heater control register.
 * 
 * @details This API is used to write data to the RH/T user register and heater control register of the Si70xx sensor.
 *          The register to be written is specified by the `reg` parameter, which can be either the RH/T user register or the heater control register.
 * 
 * @param[in] i2c_instance Select the I2C instance from sl_i2c_instance_t.
 * @param[in] addr I2C device address for Si70xx.
 * @param[in] reg Register of type \ref sl_si70xx_registers_t.
 * @param[in] value The value to be written into the register.
 * 
 * @return sl_status_t Status code indicating the result: 
 *         - SL_STATUS_OK  - Success.  
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument. 
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_si70xx_write_control_register(sl_i2c_instance_t i2c_instance,
                                                   uint8_t addr,
                                                   sl_si70xx_registers_t reg,
                                                   uint8_t value);

/// @} end group SI70XX ********************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/** @addtogroup SI70XX Si70XX Humidity and Temperature Sensor
* @{
*
* @details
*
* @section Si70xx Introduction
*
* The Si70xx sensor driver offers a set of functions for interacting with the Si70xx sensor series over the I2C bus. It facilitates
* the integration of the sensor by providing access to features such as temperature and humidity measurements,
* firmware revision retrieval, and device presence detection. All Si70xx functions are managed through the generic driver.
*
* @section Si70xx_Use Usage
*
* Once the I2C peripheral is initialized and configured, the Si70xx sensor will be ready for use. The common functions include the following:
*
* 1. *Initialize the Si70xx sensor:* @ref sl_si91x_si70xx_init
* 2. *Check whether an Si7006/13/20/21 is present on the I2C bus:* @ref sl_si91x_si70xx_is_present
* 3. *Measure relative humidity and temperature from the Si7006/13/20/21 sensor:* @ref sl_si91x_si70xx_measure_rh_and_temp
* 4. *Read firmware revision from the Si7006/13/20/21 sensor:* @ref sl_si91x_si70xx_get_firmware_revision
* 5. *Initiate a Si70xx software reset using the appropriate command:* @ref sl_si91x_si70xx_reset
*
* @li The following sequence should be followed when using the Si70xx sensor:
*   - *Initialize the Si70xx sensor:* Use @ref sl_si91x_si70xx_init before using the other functions.
*   - *Check Sensor Presence:* Optionally verify if a Si70xx sensor is present on the I2C bus using @ref sl_si91x_si70xx_is_present.
*   - *Measure Relative Humidity and Temperature:* Use @ref sl_si91x_si70xx_measure_rh_and_temp to obtain measurements from the sensor.
*   - *Read the firmware revision:* Retrieve the firmware version with @ref sl_si91x_si70xx_get_firmware_revision.
*   - *Initiate a software reset:* Perform a software reset on the sensor if needed using @ref sl_si91x_si70xx_reset.
*
* @li See the Function Documentation for the usage information of all the APIs in detail.
*
*
* @} (end addtogroup SI70XX Si70XX Sensor) */

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_SI70XX_H */
