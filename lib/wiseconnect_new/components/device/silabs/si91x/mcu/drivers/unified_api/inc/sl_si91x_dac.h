/******************************************************************************
* @file sl_si91x_dac.h
* @brief DAC API implementation
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

#ifndef SL_SI91X_DAC_H_
#define SL_SI91X_DAC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"
#include "rsi_dac.h"

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif // UNUSED_PARAMETER
/** @endcond */

/***************************************************************************/
/**
 * @addtogroup DAC Digital to Analog Converter
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 *****************************************************************************/

/***************************************************************************/
/**
 * @brief Typedef for a user-defined callback function, triggered upon the completion of a DAC sample.
 *
 * @param[in] event DAC event identifier for various interrupts.
 ******************************************************************************/
typedef void (*sl_dac_callback_t)(uint8_t event); // Callback function pointer

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/
/***************************************************************************/
/**
 * @brief Enumeration for DAC operation modes.
 */
typedef enum {
  SL_DAC_FIFO_MODE,   ///< Operation mode as FIFO mode.
  SL_DAC_STATIC_MODE, ///< Operation mode as static mode.
  SL_DAC_OUTPUT_REF_VOLTAGE_FOR_ADC, ///< This mode is currently not supported. It is intended to route DAC output directly to ADC input for processing.
  SL_DAC_OPERATION_MODE_LAST, ///< Last member of enum for validation.
} sl_dac_operation_mode_t;

/**
 * @brief Enumeration for DAC callback events.
 */
typedef enum {
  SL_DAC_FIFO_MODE_EVENT = 1, ///< Callback event for FIFO mode.
  SL_DAC_STATIC_MODE_EVENT,   ///< Callback event for static mode.
} sl_dac_callback_event_t;

/**
 * @brief Enumeration for ADC channels. This enumeration is only used in the Reference voltage for ADC mode of DAC.
 * @note These enums are not used as SL_DAC_OUTPUT_REF_VOLTAGE_FOR_ADC mode is not supported currently.
 */
typedef enum {
  SL_DAC_ADC_CHANNEL_0,  ///< ADC channel 0
  SL_DAC_ADC_CHANNEL_1,  ///< ADC channel 1
  SL_DAC_ADC_CHANNEL_2,  ///< ADC channel 2
  SL_DAC_ADC_CHANNEL_3,  ///< ADC channel 3
  SL_DAC_ADC_CHANNEL_4,  ///< ADC channel 4
  SL_DAC_ADC_CHANNEL_5,  ///< ADC channel 5
  SL_DAC_ADC_CHANNEL_6,  ///< ADC channel 6
  SL_DAC_ADC_CHANNEL_7,  ///< ADC channel 7
  SL_DAC_ADC_CHANNEL_8,  ///< ADC channel 8
  SL_DAC_ADC_CHANNEL_9,  ///< ADC channel 9
  SL_DAC_ADC_CHANNEL_10, ///< ADC channel 10
  SL_DAC_ADC_CHANNEL_11, ///< ADC channel 11
  SL_DAC_ADC_CHANNEL_12, ///< ADC channel 12
  SL_DAC_ADC_CHANNEL_13, ///< ADC channel 13
  SL_DAC_ADC_CHANNEL_14, ///< ADC channel 14
  SL_DAC_ADC_CHANNEL_15, ///< ADC channel 15
} sl_channel_id_for_adc_t;

/**
 * @brief Structure to hold the version number of the peripheral API.
 */
typedef struct {
  uint8_t release; ///< Release version number.
  uint8_t major;   ///< Major version number.
  uint8_t minor;   ///< Minor version number.
} sl_dac_version_t;

/**
 * @brief Structure to hold the clock configuration parameters.
 */
typedef struct {
  uint16_t division_factor;         ///< Division factor (0 to 1023).
  uint32_t soc_pll_clock;           ///< SoC PLL clock frequency.
  uint32_t soc_pll_reference_clock; ///< SoC PLL reference clock frequency.
} sl_dac_clock_config_t;

/**
 * @brief Structure to hold the DAC configuration parameters.
 */
typedef struct {
  uint8_t operating_mode;     ///< Operating mode of the DAC.
  uint8_t dac_fifo_threshold; ///< DAC FIFO threshold level.
  uint8_t adc_channel;        ///< ADC channel to be used for the DAC.
  uint32_t dac_sample_rate;   ///< DAC sample rate.
  uint8_t dac_pin;            ///< DAC pin to be used for the DAC.
  uint8_t dac_port;           ///< DAC port to be used for the DAC.
} sl_dac_config_t;

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/
/**
 * @brief To initialize the DAC (Digital to Analog Converter).
 * 
 * @details This API enables the DAC peripheral and sets the clock for the DAC controller.
 *
 * @param[in] dac_clock Pointer to the DAC clock configuration structure (\ref sl_dac_clock_config_t).
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully initialized the DAC.
 *         - SL_STATUS_FAIL               - The function failed.
 *         - SL_STATUS_NOT_INITIALIZED    - Clock is not initialized.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER       - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *
 * @note Default value is recommended for DAC clock configurations.
 ******************************************************************************/
sl_status_t sl_si91x_dac_init(sl_dac_clock_config_t *dac_clock);

/***************************************************************************/
/**
 * @brief To configure DAC control parameters.
 * 
 * @details This API configures the DAC control parameters, including:
 *          - Operation mode (Static/FIFO/Reference voltage for ADC)
 *          - Sample rate (63 sps to 5 Msps)
 *          - DAC FIFO Threshold value (0 to 7)
 *          - ADC Channel (0 to 15)
 * 
 * @pre Pre-condition:
 *          - \ref sl_si91x_dac_init
 * 
 * @param[in] dac_config DAC configuration structure variable, see \ref sl_dac_config_t.
 * @param[in] vref_value Reference voltage in the range of 1.8 to 3.6 V.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully configured the DAC.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_INVALID_RANGE      - Mismatch range.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note In DAC, if "Reference voltage for ADC" is the operation mode, choose the ADC Channel.
 ******************************************************************************/
sl_status_t sl_si91x_dac_set_configuration(sl_dac_config_t dac_config, float vref_value);

/***************************************************************************/
/**
 * @brief To write digital input samples to the DAC.
 * 
 * @details This API writes the digital sample to both dynamic and static modes of the DAC.
 *          - When Static mode is enabled, any data written to the Data Register (DR) will bypass the FIFO and be sent directly to the DAC for playback.
 *            Only a single sample can be held at a time.
 *          - If FIFO mode is enabled, data is written to the FIFO.
 * 
 * @note For static mode, the sample length is 1.
 * @note For FIFO mode, the sample length is 1 - 1024.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_dac_init
 *      - \ref sl_si91x_dac_set_configuration
 * 
 * @param[in] data Pointer to the input samples buffer.
 * @param[in] length Number of samples to play in the DAC in the range of 1 - 1024 bits.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully wrote the data to the DAC.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_INVALID_RANGE      - Mismatch range.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dac_write_data(int16_t *data, uint16_t length);

/***************************************************************************/
/**
 * @brief To read the DAC sampled output data.
 * 
 * @details This API reads the sampled output data of the DAC from the data register for dynamic 
 *          and static modes, returning the last data played on the DAC.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_dac_init
 *      - \ref sl_si91x_dac_set_configuration
 *      - \ref sl_si91x_dac_write_data
 *      - \ref sl_si91x_dac_start
 * 
 * @param[out] dac_output_data Pointer to store the DAC data read from the data register.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully read the data from the DAC.
 *         - SL_STATUS_FAIL  - Function failed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dac_read_data(uint16_t *dac_output_data);

/***************************************************************************/
/**
 * @brief To get the DAC sample clock.
 * 
 * @details This API reads the AUX clock and returns the achieved sample clock.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_dac_init
 *      - \ref sl_si91x_dac_set_configuration
 * 
 * @param[in] sample_rate Sample rate for input data per second in the range of (63-5000000).
 * @param[out] sample_clock Pointer to store the achieved sample frequency value.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully got the achieved sample clock.
 *         - SL_STATUS_INVALID_RANGE      - Mismatch range.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dac_get_achieved_sample_clock(uint32_t sample_rate, uint32_t *sample_clock);

/***************************************************************************/
/**
 * @brief To rewrite the DAC sample data.
 * 
 * @details This API rewrites the input sample data in DAC FIFO operating mode, continuously rewriting the sample data in the DAC.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_dac_init
 *      - \ref sl_si91x_dac_set_configuration
 *      - \ref sl_si91x_dac_write_data
 *      - \ref sl_si91x_dac_start
 * 
 * @param[in] data Pointer to the input samples buffer.
 * @param[in] length Number of samples to play in the DAC in the range of (1-1024).
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully rewrote the sample data.
 *         - SL_STATUS_NULL_POINTER       - The parameter is a null pointer.
 *         - SL_STATUS_INVALID_RANGE      - Mismatch range.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note The rewrite is only applicable for FIFO mode.
 ******************************************************************************/
sl_status_t sl_si91x_dac_rewrite_data(int16_t *data, uint16_t length);

/***************************************************************************/
/**
 * @brief To register the user callback function.
 * 
 * @details This API registers a user-provided callback function that is invoked when an event occurs.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_dac_init
 *      - \ref sl_si91x_dac_set_configuration
 *
 * @param[in] callback_event Pointer to the function which needs to be called at the time of interrupt, see \ref sl_dac_callback_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully registered the callback.
 *         - SL_STATUS_BUSY               - Driver is busy.
 *         - SL_STATUS_NULL_POINTER       - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note Before calling this function again, it is mandatory to call the \ref sl_si91x_dac_unregister_event_callback
 *       function to unregister the callback; otherwise, it returns the SL_STATUS_BUSY error code.
 ******************************************************************************/
sl_status_t sl_si91x_dac_register_event_callback(sl_dac_callback_t callback_event);

/***************************************************************************/
/**
 * @brief To unregister the user callback function.
 * 
 * @details This API unregisters the previously registered user callback function.
 * 
 * @note It is mandatory to call this function before registering the callback again.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_dac_register_event_callback
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully deregistered the callback.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dac_unregister_event_callback(void);

/***************************************************************************/
/**
 * @brief To start the DAC operation.
 * 
 * @details This API starts writing the samples in static/FIFO modes.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_dac_init
 *      - \ref sl_si91x_dac_set_configuration
 *      - \ref sl_si91x_dac_register_event_callback
 *      - \ref sl_si91x_dac_write_data
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully started the DAC operation.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dac_start(void);

/***************************************************************************/
/**
 * @brief To stop the DAC operation.
 * 
 * @details This API stops writing the samples in static/FIFO modes.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_dac_init
 *      - \ref sl_si91x_dac_set_configuration
 *      - \ref sl_si91x_dac_register_event_callback
 *      - \ref sl_si91x_dac_write_data
 *      - \ref sl_si91x_dac_start
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully stopped the DAC operation.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dac_stop(void);

/***************************************************************************/
/**
 * @brief To deinitialize the DAC module.
 * 
 * @details This API deinitializes the DAC module, releases the hardware resources, and resets the DAC parameters.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_dac_init
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully deinitialized the DAC module.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER       - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dac_deinit(void);

/***************************************************************************/
/**
 * @brief To get the DAC version.
 * 
 * @details This API returns the release, major, and minor versions of the DAC.
 * 
 * @return sl_dac_version_t Structure containing the DAC version information, see \ref sl_dac_version_t.
 ******************************************************************************/
sl_dac_version_t sl_si91x_dac_get_version(void);

/// @} end group DAC ********************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/**
 * @addtogroup DAC
 * @{
 *
 * @details
 *
 * @section DAC_Intro Introduction
 *
 * The Digital-to-Analog Converter (DAC) is a crucial component used to convert digital signals (binary data) into analog signals (continuous voltage or current). 
 * DACs are widely used in various applications, including audio systems, telecommunications, and control systems, where precise analog output is required.
 * 
 * @image html dac_blockDiagram.png "DAC Block Diagram"
 * 
 * **Common use cases include:**
 * - **Audio Systems**: Converting digital audio signals for playback.
 * - **Telecommunications**: Modulating digital data onto an analog carrier.
 * - **Control Systems**: Generating analog control signals for process control.
 * - **Signal Processing**: Handling analog signals in various applications.
 *
 * @section DAC_Config Configuration
 *
 * DAC configuration involves setting parameters such as the operation mode, sampling rate, and FIFO threshold. The configuration structure 
 * @ref sl_dac_config_t is used to set these parameters, including operation mode (@ref sl_dac_operation_mode_t), sampling rate, and more. 
 * The configuration is then applied using the @ref sl_si91x_dac_set_configuration() API.
 *
 * For detailed information on available configuration parameters, see the specific peripheral example README document.
 *
 * @section DAC_Use Usage
 *
 * After defining the DAC configuration structures and passing an instance of @ref sl_dac_config_t, the common DAC functions can be used to initiate and configure the DAC. 
 * The typical flow for implementation is as follows:
 *
 * 1. Initialize the DAC: @ref sl_si91x_dac_init
 * 2. Set the DAC configuration: @ref sl_si91x_dac_set_configuration
 * 3. Register the event callback (if required): @ref sl_si91x_dac_register_event_callback
 * 4. Write data to DAC: @ref sl_si91x_dac_write_data
 * 5. Start DAC operation: @ref sl_si91x_dac_start
 * 6. (Optional) Rewrite data for FIFO mode: @ref sl_si91x_dac_rewrite_data
 * 7. Deinitialize the DAC: @ref sl_si91x_dac_deinit
 *
 * @} (end addtogroup DAC)
 */

#ifdef __cplusplus
}
#endif
#endif /* SL_SI91X_DAC_H_ */
