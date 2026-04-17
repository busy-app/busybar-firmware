/******************************************************************************
* @file sl_si91x_bjt_temperature_sensor.h
* @brief BJT temperature sensor API implementation
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

#ifndef SL_SI91X_BJT_TEMPERATURE_SENSOR_H_
#define SL_SI91X_BJT_TEMPERATURE_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_adc.h"
#include "sl_status.h"

/***************************************************************************/
/**
 * @addtogroup BJTTEMPSENSOR BJT Temperature Sensor
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 ******************************************************************************/

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/
/**
 * @brief Enumeration for BJT temperature sensor types.
 */
typedef enum {
  SL_BJT_DEGREE_CELSIUS, ///< Temperature in degree Celsius
  SL_BJT_FAHRENHEIT,     ///< Temperature in Fahrenheit
  SL_BJT_KELVIN,         ///< Temperature in Kelvin
} sl_bjt_temperature_sensor_enum_t;

/**
 * @brief Enumeration for BJT temperature sensor state.
 */
typedef enum {
  SL_BJT_TEMPERATURE_SENSOR_DISABLE, ///< Temperature sensor state is disabled
  SL_BJT_TEMPERATURE_SENSOR_ENABLE,  ///< Temperature sensor state is enabled
  SL_BJT_TEMPERATURE_SENSOR_LAST,    ///< Last member of enum is used to validate the sensor state
} sl_bjt_temperature_sensor_state_t;

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************/
/**
 * @brief To initialize the BJT temperature sensor by configuring ADC channels and settings.
 * 
 * @details This API performs ADC initialization and configuration, sets the register callback, and starts the ADC.
 * 
 * @pre Pre-conditions:
 *      - Ensure that the ADC peripheral clock is enabled.
 *      - Ensure that the necessary GPIOs for the ADC channels are configured.
 * 
 * @param[in] sl_bjt_temperature_sensor_channel_config BJT channels configuration structure containing channel number, sampling rate, and sample length.
 * @param[in] sl_bjt_temperature_sensor_config BJT configuration structure containing the operation mode and number of channels.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Fail.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter.
 *         - SL_STATUS_INVALID_RANGE  - Invalid range.
 *         - SL_STATUS_INVALID_COUNT  - Invalid count.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_bjt_temperature_sensor_init(adc_ch_config_t sl_bjt_temperature_sensor_channel_config,
                                                 adc_config_t sl_bjt_temperature_sensor_config);

/***************************************************************************/
/**
 * @brief To read data from the BJT temperature sensor and convert it to temperature.
 * 
 * @details The API is used to read raw data from the BJT temperature sensor and convert it to a temperature value.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_bjt_temperature_sensor_init()
 * 
 * @param[out] temp_data Pointer to the variable where the converted temperature data will be stored.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Fail.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_INVALID_RANGE  - Mismatch Range.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_bjt_temperature_sensor_read_data(double *temp_data);

/***************************************************************************/
/**
 * @brief To enable or disable the BJT temperature sensor.
 * 
 * @details The API is used to enable or disable the BJT temperature sensor.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_bjt_temperature_sensor_init()
 *      - \ref sl_si91x_bjt_temperature_sensor_set_channel_configuration()
 * 
 * @param[in] state Value indicating whether to enable or disable the BJT temperature sensor:
 *                  - SL_BJT_TEMPERATURE_SENSOR_ENABLE (1) - Enable
 *                  - SL_BJT_TEMPERATURE_SENSOR_DISABLE (0) - Disable
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_bjt_temperature_sensor_state(sl_bjt_temperature_sensor_state_t state);

/***************************************************************************/
/**
 * @brief To configure the BJT temperature sensor with the specified channel.
 * 
 * @details The API is used to configure the BJT temperature sensor with the specified channel.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_bjt_temperature_sensor_init()
 * 
 * @param[in] sl_bjt_temperature_sensor_channel_config BJT channel configuration structure containing the positive input to BJT value.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_INVALID_RANGE  - Mismatch range.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_bjt_temperature_sensor_set_channel_configuration(
  adc_ch_config_t sl_bjt_temperature_sensor_channel_config);

/*******************************************************************************/
/**
 * @brief To configure the band gap for the BJT temperature sensor.
 * 
 * @details The API is used to configure the band gap for the BJT temperature sensor.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_bjt_temperature_sensor_init()
 * 
 * @param[in] sl_bjt_temperature_sensor_channel_config Band gap channel configuration structure containing the positive input to OPAMP value.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_INVALID_RANGE  - Mismatch range.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_bjt_temperature_sensor_bg_set_channel_configuration(
  adc_ch_config_t sl_bjt_temperature_sensor_channel_config);

/******************************************************************************/
/**
 * @brief Converts temperature from degrees Celsius to Fahrenheit or Kelvin.
 * 
 * @details This API takes a temperature measurement in degrees Celsius and converts it
 * to the temperature scale specified by current_temperature_mode parameter.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_bjt_temperature_sensor_init() must be called before using this API.
 *      - \ref sl_si91x_bjt_temperature_sensor_read_data() must be called to get the temperature in Celsius.
 * 
 * @param[in,out] temp_data Pointer to the variable containing the temperature in Celsius;
 *                         will store the converted temperature after function execution.
 * @param[in] current_temperature_mode Desired temperature scale for conversion @ref sl_bjt_temperature_sensor_enum_t
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Fail.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *****************************************************************************/
sl_status_t sl_si91x_get_bjt_temperature_sensor_conversion(double *temp_data,
                                                           sl_bjt_temperature_sensor_enum_t current_temperature_mode);

/***************************************************************************/
/**
 * @brief To de-initialize the BJT temperature sensor and release resources.
 * 
 * @details The API is used to de-initialize the BJT temperature sensor and release any allocated resources.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_bjt_temperature_sensor_init()
 * 
 * @param[in] sl_bjt_temperature_sensor_config Configuration structure used for de-initializing the BJT temperature sensor.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Fail.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_bjt_temperature_sensor_deinit(adc_config_t sl_bjt_temperature_sensor_config);

/** @} end group BJTTEMPSENSOR */

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/** @addtogroup BJTTEMPSENSOR BJT Temperature Sensor
 * @{
 * 
 * @details
 *
 * @section BJTTEMPSENSOR_intro Introduction
 *
 * Bipolar Junction Transistor (BJT) can be utilized as a temperature sensor and its temperature-dependent characteristics.
 * The BJT temperature sensor peripheral converts a digital output into a temperature in degrees Celsius.
 * The process involves:
 *  - *Reading the Band-gap Reference:* The sensor first measures the band-gap voltage.
 *  - *Reading the BJT Output:* After successful calibration, the BJT output is read.
 *  - *Computing the Temperature:* The temperature is calculated by taking the difference between the BJT output and the band-gap reference value.
 *
 * Additionally, by switching the current temperature mode, you can also obtain the temperature value in Kelvin and Fahrenheit.
 *
 * @section BJTTEMPSENSOR_Config Configuration
 *
 * The configurable parameters by using structured macros are:
 * - *Temperature Units:* By altering the macros in the mode, you can change the temperature from
 *   Celsius to Kelvin and from Celsius to Fahrenheit: @ref sl_bjt_temperature_sensor_enum_t
 * - *Sensor State:* Structured macros are used to enable and disable the BJT temperature sensor: @ref sl_bjt_temperature_sensor_state_t
 * 
 * @section BJTTEMPSENSOR_Use Usage
 *
 * The BJT temperature sensor will first call the callback event after initializing
 * and configuring the channel settings and operating mode. It will enable, configure,
 * and read the BJT temperature value in the read data along with configuring and reading the band-gap value.
 * Once the values are successfully read, the temperature in Celsius will be computed.
 *  1. *Initialize the BJT temperature sensor:* @ref sl_si91x_bjt_temperature_sensor_init
 *  2. *Read data from the BJT temperature sensor:* @ref sl_si91x_bjt_temperature_sensor_read_data
 *  3. *Enable or disable the BJT temperature sensor:* @ref sl_si91x_bjt_temperature_sensor_state
 *  4. *Configure the channel of BJT:* @ref sl_si91x_bjt_temperature_sensor_set_channel_configuration
 *  5. *Configure band gap channel for BJT temperature sensor:* @ref sl_si91x_bjt_temperature_sensor_bg_set_channel_configuration
 * 
 * @li Additionally, APIs to modify the temperature mode and deinitialize the callback event 
 * using the conversion and deinit APIs:
 *  1. *De-initialize the BJT temperature sensor:* @ref sl_si91x_bjt_temperature_sensor_deinit
 *  2. *Convert the sensor reading to temperature in Celsius/Fahrenheit/Kelvin:* @ref sl_si91x_get_bjt_temperature_sensor_conversion
 *
 */
/** @} end group BJTTEMPSENSOR */

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_BJT_TEMPERATURE_SENSOR_H_ */
