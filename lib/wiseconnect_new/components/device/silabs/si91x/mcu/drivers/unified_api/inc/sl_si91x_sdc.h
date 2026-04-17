/******************************************************************************
* @file sl_si91x_sdc.h
* @brief SDC API implementation
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

#ifndef SL_SI91X_SDC_H
#define SL_SI91X_SDC_H

#include "sl_si91x_peripheral_sdc.h"
#include "sl_si91x_sdc_common_config.h"

/***************************************************************************/
/**
 * @addtogroup SDC Sensor Data Collector
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************************************************************************
**********************************************************Macros*****************************************************************
********************************************************************************************************************************* */

#define SDC_TRANFER_COMPLETE_MASK      0xF                                     ///< Mask for transfer complete
#define SDC_TRANSFER_COMPLETE_POS      0                                       ///<  Position for transfer complete
#define SDC_CHANNEL1_TRANSFER_COMPLETE (0X1 << 0) << SDC_TRANSFER_COMPLETE_POS ///< Channel 1 transfer complete flag
#define SDC_CHANNEL2_TRANSFER_COMPLETE (0X1 << 1) << SDC_TRANSFER_COMPLETE_POS ///< Channel 2 transfer complete flag
#define SDC_CHANNEL3_TRANSFER_COMPLETE (0X1 << 2) << SDC_TRANSFER_COMPLETE_POS ///< Channel 3 transfer complete flag
#define SDC_CHANNEL4_TRANSFER_COMPLETE (0X1 << 3) << SDC_TRANSFER_COMPLETE_POS ///< Channel 4 transfer complete flag

#define SDC_AUTO_BUFFER_RESET_MASK 0xF00 ///< Mask for buffer auto reset
#define SDC_AUTO_BUFFER_RESET_POS  8     ///< Position for buffer auto reset
#define SDC_CHANNEL1_AUTO_BUFFER_RESET \
  (0X1 << 0) << SDC_AUTO_BUFFER_RESET_POS ///< Channel 1 buffer auto reset enable flag
#define SDC_CHANNEL2_AUTO_BUFFER_RESET \
  (0X1 << 1) << SDC_AUTO_BUFFER_RESET_POS ///< Channel 2 buffer auto reset enable flag
#define SDC_CHANNEL3_AUTO_BUFFER_RESET \
  (0X1 << 2) << SDC_AUTO_BUFFER_RESET_POS ///< Channel 3 buffer auto reset enable flag
#define SDC_CHANNEL4_AUTO_BUFFER_RESET \
  (0X1 << 3) << SDC_AUTO_BUFFER_RESET_POS ///< Channel 4 buffer auto reset enable flag

#define SDC_DIFFERENTIAL_MODE_MASK     0xF0000                                 ///< Mask for differential mode
#define SDC_DIFFERENTIAL_MODE_POS      16                                      ///<  Position for differential mode
#define SDC_CHANNEL1_DIFFERENTIAL_MODE (0X1 << 0) << SDC_DIFFERENTIAL_MODE_POS ///< Channel 1 differential mode flag
#define SDC_CHANNEL2_DIFFERENTIAL_MODE (0X1 << 1) << SDC_DIFFERENTIAL_MODE_POS ///< Channel 2 differential mode flag
#define SDC_CHANNEL3_DIFFERENTIAL_MODE (0X1 << 2) << SDC_DIFFERENTIAL_MODE_POS ///< Channel 3 differential mode flag
#define SDC_CHANNEL4_DIFFERENTIAL_MODE (0X1 << 3) << SDC_DIFFERENTIAL_MODE_POS ///< Channel 4 differential mode flag

// Status codes for SDC operations

#define SL_SDC_BUFFER_RESET \
  0x75 ///< Status code indicating that buffer has been reset after successfully reading the data
#define SL_SDC_BUFFER_FULL 0x76 ///< Status code indicating buffer is full

// SDC Configuration Macros

#define SL_SDC_SINGLE_ENDED_MODE 0 ///< Single-ended Mode
#define SL_SDC_DIFFERENTIAL_MODE 1 ///< Differential Mode

#define SDC_MAX_VALUE 4095 ///< Maximum value of the SDC in single-ended mode
#define SDC_MIN_VALUE 0    ///< Minimum value of the SDC
#define SDC_MID_VALUE 2048 ///< Mid value of the SDC

#define SDC_ONE_CH_MAX_FIFO_TH 15 ///< Maximum FIFO threshold per channel when only using one channel

#define SDC_TWO_CH_MAX_FIFO_TH 8 ///< Maximum FIFO threshold per channel for SDC with two channels

#define SDC_FOUR_CH_MAX_FIFO_TH 4 ///< Maximum FIFO threshold per channel for SDC with four channels

/**********************************************************************************************************************************
**********************************************************Typedefs Enums*******************************************************
********************************************************************************************************************************* */
/**
 * @brief Enumeration for SDC Events
 */
typedef enum {
  SDC_EVENT_SAMPLE_COMPLETE, ///< Sample complete event
  SDC_EVENT_ERROR,           ///< Error event
  SDC_EVENT_DATA_READY,      ///< SDC data ready
  SDC_EVENT_BUFFER_FULL,     ///< Buffer is full
  SDC_EVENT_BUFFER_RESET     ///< Buffer reset
} sl_sdc_event_t;

/**********************************************************************************************************************************
**********************************************************Callback function*******************************************************
********************************************************************************************************************************* */

/**
   * @brief Callback function type for Sensor Data Controller (SDC) events
   *
   * This typedef defines a function pointer for SDC event callbacks. The callback function
   * is invoked when SDC events occur.
   *
   * @param[in] channel The SDC channel on which the event occurred
   * @param[in] event   The type of event that occurred
   *
   * @note The application must register this callback to receive notifications about SDC events
   */
typedef void (*sl_sdc_callback_t)(uint8_t channel, sl_sdc_event_t event);

/**********************************************************************************************************************************
**********************************************************Typedefs Structs*******************************************************
********************************************************************************************************************************* */

/**
* @brief Structure for holding the channel info
*/

typedef struct {
  sl_sdc_callback_t callback_event;                ///< Callback function for SDC events
  uint32_t channel_info_bit_field;                 ///<Bit field for tracking transfer complete and auto buffer reset
  uint32_t transfer_count[SDC_NUMBER_OF_CHANNELS]; ///< Transfer count for each channel
  uint32_t sample_length[SDC_NUMBER_OF_CHANNELS];  ///< Transfer length for each channel
  int16_t *recieve_buffer[SDC_NUMBER_OF_CHANNELS]; ///< Pointer to the receive buffer for SDC data
} sl_si91x_sdc_channel_info_t;

/**
 * @brief Structure for holding channel misc configurations
 * 
 */

typedef struct {
  uint32_t buffer_auto_reset; ///< Start storing data from start of buffer after it is filled
} sl_si91x_sdc_channel_misc_config_t;

/***********************************************************************************************************************************
**********************************************************Function prototypes*****************************************************
************************************************************************************************************************************ */

/***************************************************************************/
/**
 * @brief Initialize the SDC driver.
 *
 * @details   * This function initializes the SDC driver by performing the following steps:
 *             1. Initializes the RTC block.
 *             2. Starts the RTC.   
 *             3. Initializes the SDC peripheral.
 *             4. Gets the chip voltage and update input voltage if required.
 *             5. Calibrates the ADC.
 *             6. Calculates the offset and gain values and stores them.
 *             7. Configures the SDC LDO voltage based on the reference voltage.
 *
 * @param[in] vref Reference voltage for SDC.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully initialized the SDC.
 *        
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sdc_driver_init(float vref);

/***************************************************************************/

/**
 * @brief Deinitialize the SDC driver.
 *
 * @details  * This function performs the following steps:
 *            1. Stops the RTC to ensure no further operations are performed on it.
 *            2. Disables the SDC NVIC interrupt to prevent further interrupts.
 *            3. Deinitializes the SDC peripheral.
 *            4. Disables the SDC LDO to save power.
 *            5. Resets the global variable sdc_channel_info to zero.
 *            6. Resets the global variable sdc_calibration_data to zero.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_sdc_driver_init
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully deinitialized the SDC.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sdc_driver_deinit(void);

/***************************************************************************/

/**
 * @brief Configure the SDC driver with channel and pin configuration.
 *
 * @details  * This function performs the following steps:
 *           1. Checks if the provided configuration pointer is NULL and returns an error if it is.
 *           2. Decrements the sample threshold and trigger event select values in the configuration to match the expected values in the SDC register.
 *           3. Configures the SDC with the provided configuration using the `sl_si91x_sdc_config` function.
 *           4. Returns the status of the configuration operation.
 *
 * @param[in] sl_sdc_config Pointer to SDC configuration structure.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_sdc_driver_init
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully configured the SDC.
 *         - SL_STATUS_NULL_POINTER       - The parameter is a null pointer.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/

sl_status_t sl_si91x_sdc_driver_config(sl_si91x_sdc_config_t *sl_sdc_config);

/***************************************************************************/

/**
 * @brief Configures an SDC channel with the specified settings.
 *
 * @details   * This function performs the following steps:
 *             1. Checks if the provided pin configuration and channel configuration pointers are NULL and returns an error if they are.
 *             2. Configures the SDC ADC pins using the `sl_si91x_sdc_adc_pin_config` function with the provided pin configuration and the number of channels.
 *             3. Iterates through each channel and initializes the channel information structure with the provided settings:
 *                - Sets the sample length for each channel.
 *                - Sets the receive buffer for each channel.
 *                - Sets the differential mode and auto buffer reset bits in the channel info bit field.
 *             4. Returns the status of the configuration operation.
 *
 * @param[in] pin_config Pointer to a structure containing the ADC pin configuration parameters.
 * @param[in] sdc_channel_config Pointer to a structure containing the SDC channel configuration parameters.
 * @param[in] misc_config Pointer to a structure containing misc config of SDC channel
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_sdc_driver_init
 * @note 
      1. pin_config structure is auto-generated when instance is created in the application user can directly use that structure.
 * @return
 *   - SL_STATUS_OK on success.
 *   - SL_STATUS_NULL_POINTER  - The parameter is a null pointer..
 * * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/

sl_status_t sl_si91x_sdc_driver_channel_config(sl_si91x_sdc_adc_config_t *pin_config,
                                               sl_si91x_sdc_channel_info_t *sdc_channel_config,
                                               sl_si91x_sdc_channel_misc_config_t *misc_config);

/***************************************************************************/

/**
 * @brief Register a callback for SDC events.
 *
 * @details This API registers a user-provided callback function that is invoked when an SDC event occurs.
 *
 * @param[in] callback_event Callback function to register.
 * @pre Pre-conditions:
 *      - \ref sl_si91x_sdc_driver_init
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK            - Successfully registered the callback.
 *         - SL_STATUS_INVALID_PARAMETER - callback event is NULL
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sdc_driver_register_callback(sl_sdc_callback_t callback_event);

/***************************************************************************/

/**
 * @brief Unregister the SDC callback.
 *
 * @details This API unregisters the callback function registered for SDC events.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_sdc_driver_init
 *      - \ref sl_si91x_sdc_driver_register_callback
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK            - Successfully unregistered the callback.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sdc_driver_unregister_callback(void);

/***************************************************************************/

/**
 * @brief Start reading data from SDC.
 *
 * @details This API starts the SDC data acquisition process.
 *        1. It prepares the SDC to read data and provides a buffer to store the received data.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_sdc_driver_init
 *      - \ref sl_si91x_sdc_driver_config
 *      - \ref sl_si91x_sdc_driver_channel_config
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully started data read.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sdc_driver_read_data_start(void);

/***************************************************************************/

/**
 * @brief Stop the SDC driver.
 *
 * @details This API stops the SDC data acquisition process.
 *        1. It disables the SDC interrupt and stops the SDC sampling process.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_sdc_driver_init
 *      - \ref sl_si91x_sdc_driver_config
 *      - \ref sl_si91x_sdc_driver_channel_config 
 *      - \ref sl_si91x_sdc_driver_read_data_start
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully stopped the SDC.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sdc_driver_stop(void);

/***************************************************************************/

/**
 * @brief Store the data from SDC FIFO registers.
 * 
 * @details This API performs the following steps:
 *             1. Reads the data from the SDC FIFO.
 *             2. channel_id is decremented by 1 to match the zero-based index.
 *             3. Checks if the channel is in differential mode and processes the data accordingly.
 *             4. Stores the processed data in the channel's receive buffer.
 *             5. Increments the transfer count for the channel.
 *             6. Checks if the transfer count has reached the sample length for the channel.
 *                If it has, it calls the callback function with the SDC_EVENT_SAMPLE_COMPLETE event for that channel.
 *             7. If the channel is set to auto buffer reset, it resets the transfer count for the channel and clears the transfer complete bit in the channel info bit field.

 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_sdc_driver_init
 *      - \ref sl_si91x_sdc_driver_config
 *      - \ref sl_si91x_sdc_driver_channel_config 
 *      - \ref sl_si91x_sdc_driver_read_data_start
 *      - \ref sl_si91x_sdc_driver_register_callback
 *
 * @note   1. This API should be called only after the SDC is started.
 *         2. If auto buffer reset is enabled, the write pointer wraps around to the start of the buffer when it reaches the end,
 *            allowing new data to overwrite the oldest data.
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully reads the SDC data.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sdc_driver_store_data(void);

/***************************************************************************/

/**
 * @brief Reads data from the SDC driver for the specified channel.
 *
 * This function reads data of specified length from the given SDC channel and stores it in the provided buffer.
 *
 * @param[in]  channel  The SDC channel number to read data from.
 * @param[out] buffer   Pointer to the buffer where the read data will be stored.
 * @param[in]  length   The number of samples to read from the SDC channel.
 * @pre Pre-conditions:
 *      - \ref sl_si91x_sdc_driver_init
 *      - \ref sl_si91x_sdc_driver_config
 *      - \ref sl_si91x_sdc_driver_channel_config
 *      - \ref sl_si91x_sdc_driver_read_data_start
 *
 *
 * @return
 *   - SL_STATUS_OK if the data was read successfully.
 *   - SL_INVALID_PARAMETER if the provided parameters are invalid (e.g., channel number out of range, buffer is NULL, or length > threshold).
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sdc_driver_read_data(uint32_t channel, int16_t *buffer, uint8_t length);

/***************************************************************************/

/// @} end group SDC ********************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY! ***********************

/** 
 * @addtogroup SDC Sensor Data Collector
 * @{
 *
 * @details
 *
 * @section SDC_Intro Introduction
 *
 * The Sensor Data Collector (SDC) is a low-power peripheral that internally uses ADC, enabling the conversion of analog signals (continuous voltage levels) into digital data (discrete binary values). This conversion allows microcontrollers to interface with analog sensors, such as temperature sensors, light sensors, or joysticks, effectively translating real-world analog signals into data that can be processed by digital systems.
 *
 * @image html sdc_blockDiagram.png "SDC Block Diagram"
 * 
 * **Features supported by the Watchdog Timer**
 * @li Low power operation: SDC works in PS1 mode.
 * @li Multiple channels: SDC supports upto 4 channels.
 * @li Sampling Interval: The SDC can be configured to sample data at different intervals from 1 to 1024 units of seconds or milli seconds.
 * @li Sampling threshold: The SDC can be configured with a sampling threshold ranging from 1 to 16. device wakeups after reaching the threshold.
 * @li Single Ended and Differential Mode: SDC supports both single-ended and differential modes of operation.
 *
 * @section SDC_Config Configuration
 *
 * Configuring the SDC involves several key steps to ensure accurate and efficient operation. These steps vary based on the specific hardware and application requirements. Below are some essential configuration tasks:
 *
 * - **SDC Common Configurations**: Define number of samples, sampling interval (1-1024), sampling threshold (1-16), clock division factor, and millisecond or second selection in the `sl_si91x_sdc_config_t` structure. After configuring this structure, call `sl_si91x_sdc_driver_config()` to set the SDC parameters.
 *
 * - **Channel-Specific Settings**: Define transfer count, buffer in the `sl_si91x_sdc_channel_info_t` structure, Auto buffer reset in `sl_si91x_sdc_channel_misc_config_t`, Input type and input pins in `sl_si91x_sdc_adc_config_t`. After configuring this structure, call `sl_sdc_driver_channel_config()` to set the channel parameters.
 *
 * For more detailed configuration information, see the specific peripheral example README documents.
 *
 * @section SDC_Usage Usage
 *
 * After defining the ADC configuration structures, use the following functions to initialize and control the ADC module:
 *
 * 1. **Initialize SDC**: @ref sl_si91x_sdc_driver_init
 * 2. **Set SDC Configuration**: @ref sl_si91x_sdc_driver_config
 * 3. **Set Channel Configuration**: @ref sl_si91x_sdc_driver_channel_config
 * 4. **Register Callback**: @ref sl_si91x_sdc_driver_register_callback
 * 5. **Start Reading Data**: @ref sl_si91x_sdc_driver_read_data_start
 * 6. **Stop Reading Data**: @ref sl_si91x_sdc_driver_stop
 * 7. **Store Data**: @ref sl_si91x_sdc_driver_store_data
 * 8. **Read Data**: @ref sl_si91x_sdc_driver_read_data
 *
 * @} (end addtogroup SDC)
 */

#ifdef __cplusplus
}
#endif

#endif
