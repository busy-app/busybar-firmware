/******************************************************************************
* @file sl_si91x_adc.h
* @brief ADC API implementation
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

#ifndef SL_SI91X_ADC_H_
#define SL_SI91X_ADC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"
#include "rsi_adc.h"
#include "base_types.h"

/***************************************************************************/
/**
 * @addtogroup ADC Analog to Digital Converter
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 ******************************************************************************/
// -----------------------------------------------------------------------------
// Data Types

/**
 * @brief Renamed ADC channel configuration.
 * 
 * @details This contains channel-specific configuration parameters including
 * Ping/Pong DMA buffer addresses for FIFO mode operation. Key fields for FIFO + DMA:
 * - chnl_ping_address[channel] : Ping buffer base address for DMA
 * - chnl_pong_address[channel] : Pong buffer base address for DMA  
 * - rx_buf[channel]            : CPU-accessible buffer pointer
 * - num_of_samples[channel]    : Number of samples per conversion
 * - sampling_rate[channel]     : Sampling rate for the channel
 * - input_type[channel]        : Input type (single-ended/differential)
 */
typedef adc_ch_config_t sl_adc_channel_config_t;

/**
 * @brief Renamed ADC configuration.
 * 
 * @details This contains global ADC operation parameters:
 * - operation_mode         : Static mode or FIFO mode
 * - num_of_channel_enable  : Number of channels to enable
 */
typedef adc_config_t sl_adc_config_t;

/**
 * @brief Renamed ADC internal configuration.
 * 
 * @details This is used specifically with sl_si91x_adc_configure_ping_pong_memory_address()
 * for detailed Ping/Pong buffer configuration in FIFO mode. Key fields:
 * - ping_addr[channel]     : Ping buffer base address
 * - pong_addr[channel]     : Pong buffer base address
 * - ping_length[channel]   : Ping buffer length in samples
 * - pong_length[channel]   : Pong buffer length in samples
 * - input_type[channel]    : Input type for the channel
 * - num_of_samples[channel]: Number of samples per conversion
 */
typedef adc_inter_config_t sl_adc_internal_config_t;

/***************************************************************************/
/**
 * @brief Typedef for a user-provided callback function that is invoked upon the completion of an ADC sample.
 * 
 * @param[in] channel ADC channel number @ref sl_adc_channel_id_t.
 * @param[in] event   ADC event for different interrupts.
 ******************************************************************************/
typedef void (*sl_adc_callback_t)(uint8_t channel, uint8_t event);

// -----------------------------------------------------------------------------
// Macros for events
#define SL_INTERNAL_DMA          INTERNAL_DMA             ///< Internal DMA event
#define SL_ADC_STATIC_MODE_EVENT ADC_STATIC_MODE_CALLBACK ///< Static mode event
#define SIGN_BIT                 BIT(11)                  ///< Sign/magnitude bit
#define ADC_DIFFERENTIAL_INPUT   1 ///< To check condition in config file, this macro value '1' should constant.

/**
 * @brief Enumeration for ADC input types.
 */
typedef enum {
  SL_ADC_SINGLE_ENDED,    ///< Single-ended input: measures the voltage between the input and ground.
  SL_ADC_DIFFERENTIAL,    ///< Differential input: measures the voltage difference between two inputs.
  SL_ADC_INPUT_TYPE_LAST, ///< Marks the last enumeration value for validation checks.
} sl_adc_input_type_typedef_t;

/**
 * @brief Enumeration for ADC operation mode.
 */
typedef enum {
  SL_ADC_FIFO_MODE   = ADC_FIFOMODE_ENABLE,   ///< Operation mode as FIFO mode.
  SL_ADC_STATIC_MODE = ADC_STATICMODE_ENABLE, ///< Operation mode as static mode.
  SL_ADC_OPERATION_MODE_LAST,                 ///< Last member of enum for validation.
} sl_adc_operation_mode_typedef_t;

/**
 * @brief Enumeration for ADC DMA type.
 */
typedef enum {
  SL_ADC_INTERNAL_DMA = INTERNAL_DMA_EN, ///< Internal DMA type.
  SL_ADC_EXTERNAL_DMA = EXTERNAL_DMA_EN, ///< External DMA type.
  SL_ADC_DMA_TYPE_LAST,                  ///< Last member of enum for validation.
} sl_adc_dma_type_typedef_t;

/**
 * @brief Enumeration for ADC channel selection modes.
 */
typedef enum {
  SL_ADC_SINGLE_CHNL = DYNAMIC_MODE_DI, ///< Selects a single ADC channel (dynamic mode disabled).
  SL_ADC_MULTI_CHNL  = DYNAMIC_MODE_EN, ///< Selects multiple ADC channels (dynamic mode enabled).
  SL_ADC_CHANNEL_TYPE_LAST,             ///< Last member of enum for validation.
} sl_adc_channel_type_typedef_t;

/**
 * @brief Enumeration for ADC channels.
 */
typedef enum {
  SL_ADC_CHANNEL_1,  ///< ADC channel 1
  SL_ADC_CHANNEL_2,  ///< ADC channel 2
  SL_ADC_CHANNEL_3,  ///< ADC channel 3
  SL_ADC_CHANNEL_4,  ///< ADC channel 4
  SL_ADC_CHANNEL_5,  ///< ADC channel 5
  SL_ADC_CHANNEL_6,  ///< ADC channel 6
  SL_ADC_CHANNEL_7,  ///< ADC channel 7
  SL_ADC_CHANNEL_8,  ///< ADC channel 8
  SL_ADC_CHANNEL_9,  ///< ADC channel 9
  SL_ADC_CHANNEL_10, ///< ADC channel 10
  SL_ADC_CHANNEL_11, ///< ADC channel 11
  SL_ADC_CHANNEL_12, ///< ADC channel 12
  SL_ADC_CHANNEL_13, ///< ADC channel 13
  SL_ADC_CHANNEL_14, ///< ADC channel 14
  SL_ADC_CHANNEL_15, ///< ADC channel 15
  SL_ADC_CHANNEL_16, ///< ADC channel 16
} sl_adc_channel_id_t;

/**
 * @brief Structure to hold the threshold detection configuration parameters.
 */
typedef struct {
  uint16_t threshold1;      ///< Threshold 1 value.
  uint16_t threshold2;      ///< Threshold 2 value.
  uint8_t *threshold1_cond; ///< Condition for Threshold 1 (for example, Equal, Greater, Lesser).
  uint8_t *threshold2_cond; ///< Condition for Threshold 2 (for example, Equal, Greater, Lesser).
  uint8_t range;            ///< Range of the threshold.
} sl_adc_threshold_config_t;

/**
 * @brief Structure to hold the FIFO threshold configuration parameters.
 */
typedef struct {
  uint8_t num_of_channel_en; ///< Number of channels enabled.
  uint8_t a_empty_threshold; ///< AEMPTY (almost empty) threshold.
  uint8_t a_full_threshold;  ///< AFULL (almost full) threshold.
  uint8_t dma_type;          ///< Internal (0) or external (1) DMA type.
} sl_adc_fifo_thrld_config_t;

/**
 * @brief Structure to hold the clock configuration parameters.
 */
typedef struct {
  uint16_t division_factor;         ///< Division factor (0 to 1023).
  uint32_t soc_pll_clock;           ///< SoC PLL clock frequency.
  uint32_t soc_pll_reference_clock; ///< SoC PLL reference clock frequency.
} sl_adc_clock_config_t;

/**
 * @brief Structure to hold the version numbers of the peripheral API.
 */
typedef struct {
  uint8_t release; ///< Release version number
  uint8_t major;   ///< Major version number
  uint8_t minor;   ///< Minor version number
} sl_adc_version_t;

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************/
/**
 * @brief To set the ADC peripheral clock and configure the PLL clock based on user-specified 
 * values in the clock configuration structure.
 * 
 * @details This API configures the ADC clock. The ADC clock configurations include:
 *         - `soc_pll_clock` (Frequency range from 1 to 180 MHz)
 *         - `soc_pll_reference_clock` (Frequency range from 15 to 65 MHz)
 *         - `division_factor` (0 to 1023)
 * 
 * @param[in] clock_configuration Pointer to the clock structure variables ( \ref sl_adc_clock_config_t)
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully configured the clock.
 *         - SL_STATUS_FAIL               - Function failed.
 *         - SL_STATUS_NOT_INITIALIZED    - Clock is not initialized.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER       - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_configure_clock(sl_adc_clock_config_t *clock_configuration) SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/
/**
 * @brief To initialize the ADC peripheral.
 * 
 * @details This API initializes the ADC by:
 *   - Setting the clock source.
 *   - Configuring the reference voltage.
 *   - Powering on the ADC block.
 *   - Calibrating the ADC offset and gain.
 * 
 * @note For FIFO mode with DMA, Ping/Pong buffer addresses are NOT configured by this function.
 *       Configure them separately using \ref sl_si91x_adc_configure_ping_pong_memory_address() 
 *       after initialization, or pre-populate the sl_adc_channel_config_t.chnl_ping_address[] 
 *       and sl_adc_channel_config_t.chnl_pong_address[] fields before calling 
 *       \ref sl_si91x_adc_set_channel_configuration().
 *       
 * @param[in] adc_channel_config ADC channels configuration structure variable, see \ref sl_adc_channel_config_t.
 * @param[in] adc_config ADC operation configuration structure variable, see \ref sl_adc_config_t.
 * @param[in] vref_value Reference voltage (range from 1.8 to 3.6 V).
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                    - Successfully initialized the ADC.
 *         - SL_STATUS_BUSY                  - The function is already active.
 *         - SL_STATUS_INVALID_PARAMETER     - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER          - The parameter is a null pointer.
 *         - SL_STATUS_INVALID_RANGE         - Mismatch range.
 *         - SL_STATUS_INVALID_COUNT         - Mismatch count.
 *         - SL_STATUS_INVALID_CONFIGURATION - Invalid configuration.(please refer below note)
 *         - SL_STATUS_NOT_INITIALIZED       - Module is not initialized. 
 * 
 * @note - Add UC channels sequentially starting from 1, and ensure that the number of instances added 
 * matches the number of channels configured in the UC. 
 * Otherwise, it will return the error code SL_STATUS_INVALID_CONFIGURATION.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_init(sl_adc_channel_config_t adc_channel_config, sl_adc_config_t adc_config, float vref_value);

/***************************************************************************/
/**
 * @brief To configure the ADC channel parameters.
 * 
 * @details This API configures the following parameters:
 *  - Number of channels
 *  - ADC operation mode (Static mode/FIFO mode)
 *  - Sample length
 *  - Input type (Single-ended/differential type)
 *  - Sampling rate (1 - 2.5 Msps (megasamples per second))
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 * 
 * @param[in] adc_channel_config ADC channels configuration structure variable, see \ref sl_adc_channel_config_t.
 * @param[in] adc_config ADC configuration structure variable, see \ref sl_adc_config_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully configured the ADC channel.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER       - The parameter is a null pointer.
 *         - SL_STATUS_INVALID_RANGE      - Mismatch range.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note If Static mode is configured, the sample length should be '1'.
 ******************************************************************************/
sl_status_t sl_si91x_adc_set_channel_configuration(sl_adc_channel_config_t adc_channel_config,
                                                   sl_adc_config_t adc_config);

/***************************************************************************/
/**
 * @brief To register the user callback function.
 * 
 * @details This API registers a user-provided callback function that is invoked when an event occurs.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 * 
 * @param[in] callback_event Pointer to the function that should be called during an interrupt event, see \ref sl_adc_callback_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK            - Successfully registered the callback.
 *         - SL_STATUS_BUSY          - Driver is busy.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note Before calling this function again, it is mandatory to call the \ref sl_si91x_adc_unregister_event_callback
 * function to unregister the callback; otherwise, it returns the SL_STATUS_BUSY error code.
 ******************************************************************************/
sl_status_t sl_si91x_adc_register_event_callback(sl_adc_callback_t callback_event);

/***************************************************************************/
/**
 * @brief To unregister the user callback function.
 * 
 * @details This API unregisters the callback function registered using \ref sl_si91x_adc_register_event_callback.
 * This function must be called before registering the callback again.
 * 
 * @pre Pre-condition:
 *  - \ref sl_si91x_adc_register_event_callback
 ******************************************************************************/
void sl_si91x_adc_unregister_event_callback(void);

/***************************************************************************/
/**
 * @brief To configure the ADC sampling rate for the ADC channels.
 * 
 * @details This API adjusts the channel offset and frequency for each channel to determine the sample rate.
 * This API ensures that the Nyquist sampling rate criteria are met by setting the channel swallow factor to no less than '3'.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 * 
 * @param[in] adc_internal_config ADC internal configuration structure for setting the sampling rate, see \ref sl_adc_internal_config_t.
 * @param[in] channel_num Channel number.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully configured the sampling rate.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_configure_channel_sampling_rate(sl_adc_internal_config_t adc_internal_config,
                                                         uint8_t channel_num);

/***************************************************************************/
/**
 * @brief To configure the ADC ping and pong memory location and length.
 * 
 * @details This API configures the ping and pong memory locations, 
 * as well as the ping and pong memory lengths. Applicable only for FIFO mode 
 * of ADC operation. FIFO mode has a dual-buffer cyclic mode that prevents data loss 
 * when the buffer is full. In dual-buffer cyclic mode, if the Ping buffer for a given 
 * channel is full, incoming sampled data is written into the Pong buffer, allowing the 
 * controller to read back samples from the Ping buffer during this period.
 * 
 * Required fields in sl_adc_internal_config_t structure (for channel index 'channel_num'):
 *  - ping_addr[channel_num]   : Ping buffer base address (ULP SRAM absolute or offset address)
 *  - pong_addr[channel_num]   : Pong buffer base address (same addressing scheme as ping_addr)
 *  - ping_length[channel_num] : Ping buffer length in samples
 *  - pong_length[channel_num] : Pong buffer length in samples  
 *  - input_type[channel_num]  : Input type for the channel (single-ended/differential)
 *  - num_of_samples[channel_num] : Number of samples per conversion
 *
 * Address ranges:
 *  - Absolute ULP SRAM addresses must lie within [0x24060000 .. 0x24061B00]
 *  - If using relative/offset addressing, offsets must not exceed the valid range limits
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 * 
 * Typical usage sequence after this function:
 *  - \ref sl_si91x_adc_enable_ping_pong()
 *  - \ref sl_si91x_adc_channel_enable()
 *  - \ref sl_si91x_adc_internal_per_channel_dma_enable()
 *  - \ref sl_si91x_adc_configure_fifo_mode()
 * 
 * @param[in] adc_internal_config ADC internal trigger configuration @ref sl_adc_internal_config_t structure variable.
 * @param[in] channel_num Channel number.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully configured ping and pong memory address and length.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_configure_ping_pong_memory_address(sl_adc_internal_config_t adc_internal_config,
                                                            uint8_t channel_num);

/***************************************************************************/
/**
 * @brief To enable ping-pong for the corresponding ADC channels.
 * 
 * @details This API enables the ping-pong mode for the specified ADC channel.
 * Ping-pong mode is applicable only for FIFO mode of ADC operation.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 *  - \ref sl_si91x_adc_configure_ping_pong_memory_address
 * 
 * @param[in] channel_num Channel number.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully enabled ping-pong for the given channel.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_enable_ping_pong(uint8_t channel_num);

/***************************************************************************/
/**
 * @brief To disable ping-pong for the corresponding ADC channels.
 * 
 * @details This API disables the ping-pong mode for the specified ADC channel.
 * Ping-pong mode is applicable only for FIFO mode of ADC operation.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 *  - \ref sl_si91x_adc_configure_ping_pong_memory_address
 *  - \ref sl_si91x_adc_enable_ping_pong
 *  - \ref sl_si91x_adc_start
 * 
 * @param[in] channel_num Channel number.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully disabled the ping-pong for the given channel.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_disable_ping_pong(uint8_t channel_num);

/***************************************************************************/
/**
 * @brief To enable DMA for the corresponding ADC channels.
 * 
 * @details This API enables DMA for the specified ADC channel. 
 * This is applicable only for FIFO mode of ADC operation.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_channel_enable
 * 
 * @param[in] channel_num Channel number.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully enabled DMA for the given channel.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_internal_per_channel_dma_enable(uint8_t channel_num);

/***************************************************************************/
/**
 * @brief To disable DMA for the corresponding ADC channels.
 * 
 * @details This API disables DMA for the specified ADC channel. 
 * This is applicable only for FIFO mode of ADC operation.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 *  - \ref sl_si91x_adc_channel_enable
 *  - \ref sl_si91x_adc_internal_per_channel_dma_enable
 *  - \ref sl_si91x_adc_start
 * 
 * @param[in] channel_num Channel number.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully disabled DMA for the given channel.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_internal_per_channel_dma_disable(uint8_t channel_num);

/***************************************************************************/
/**
 * @brief To configure the ADC in Static Mode.
 * 
 * @details This API configures the ADC to static mode with the specified positive and negative input channels and input type.
 * 
 * @note Applicable only for static mode of ADC operation.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 * 
 * @param[in] adc_channel_config ADC channels configuration structure variable, see \ref sl_adc_channel_config_t.
 * @param[in] channel_num Channel number.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully configured to static mode.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_INVALID_RANGE      - Mismatch range.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_configure_static_mode(sl_adc_channel_config_t adc_channel_config, uint8_t channel_num);

/***************************************************************************/
/**
 * @brief To configure the ADC in FIFO Mode.
 * 
 * @details This API configures the ADC for FIFO mode, where it samples data from the ADC input and writes it to the FIFO buffer.
 * 
 * @note Applicable only for FIFO mode of ADC operation.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 *  - Configure Ping/Pong DMA buffers using EITHER:
 *      - Method 1 (Recommended): \ref sl_si91x_adc_configure_ping_pong_memory_address() 
 *        followed by \ref sl_si91x_adc_enable_ping_pong(), OR
 *      - Method 2: Pre-populate sl_adc_channel_config_t.chnl_ping_address[] and 
 *        sl_adc_channel_config_t.chnl_pong_address[] before calling sl_si91x_adc_set_channel_configuration()
 *        followed by \ref sl_si91x_adc_enable_ping_pong()
 *  - \ref sl_si91x_adc_channel_enable
 *  - \ref sl_si91x_adc_internal_per_channel_dma_enable
 * 
 * @param[in] adc_channel_config ADC channels configuration structure variable, see \ref sl_adc_channel_config_t.
 * @param[in] channel_num Channel number.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully configured to FIFO mode.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_INVALID_RANGE      - Mismatch range.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_configure_fifo_mode(sl_adc_channel_config_t adc_channel_config, uint8_t channel_num);

/***************************************************************************/
/**
 * @brief To enable the ADC channel.
 * 
 * @details This API enables the ADC channel. Data will be sampled from the ADC only when the corresponding channel is enabled.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 * 
 * @param[in] channel_num Channel number.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully enabled the given channel.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_channel_enable(uint8_t channel_num);

/***************************************************************************/
/**
 * @brief To disable the ADC channel.
 * 
 * @details This API stops data sampling when the corresponding channel is disabled.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 *  - \ref sl_si91x_adc_channel_enable
 *  - \ref sl_si91x_adc_start
 * 
 * @param[in] channel_num Channel number.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully disabled the given channel.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_channel_disable(uint8_t channel_num);

/***************************************************************************/
/**
 * @brief To power on or off the ADC.
 * 
 * @details This API powers up or powers down the ADC power gates based on the provided state.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 * 
 * @param[in] state POWER_STATE type indicating whether to power on or off the ADC.
 *  - ADC_POWER_ON  - Power up ADC power gates.
 *  - ADC_POWER_OFF - Power down ADC power gates.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully set the power mode.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_set_power_mode(POWER_STATE state);

/***************************************************************************/
/**
 * @brief To enable or disable the noise averaging mode for ADC.
 * 
 * @details This API enables or disables the noise averaging mode for the ADC. 
 * If bypass noise averaging is enabled, the sampled data will be more accurate. 
 * Disabling bypass noise averaging may cause an additional clock cycle latency.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 * 
 * @param[in] state 1 - To enable bypass noise averaging mode,
 *                  0 - To disable bypass noise averaging mode.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_set_noise_average_mode(boolean_t state);

/***************************************************************************/
/**
 * @brief To enable the temperature sensor.
 * 
 * @details This API enables the BJT-based temperature sensor as an input to the ADC.
 * 
 * @note The operation mode of the ADC should be Static for this input selection.
 *       If the ADC input sample is obtained from a temperature sensor, the positive input selection value must be '23'.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully enabled the ADC temperature sensor.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_temperature_sensor_enable(void);

/***************************************************************************/
/**
 * @brief To configure ADC FIFO Threshold.
 * 
 * @details This API configures the ADC FIFO threshold. The maximum FIFO depth is 16, and the threshold value can be configured to 
 * indicate when the FIFO is almost full or almost empty for ADC operations. 
 * This allows for setting either a "FIFO almost full" or "FIFO almost empty" threshold, 
 * depending on the specific requirements of ADC configuration.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 * 
 * @param[in] adc_config ADC operation configuration structure variable, see \ref sl_adc_config_t.
 * @param[in] adc_fifo_threshold ADC FIFO structure variable like an empty FIFO, a full FIFO threshold level, see \ref sl_adc_fifo_thrld_config_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully configured FIFO threshold.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_INVALID_RANGE      - Mismatch range.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_fifo_threshold_configuration(sl_adc_config_t adc_config,
                                                      sl_adc_fifo_thrld_config_t adc_fifo_threshold);

/***************************************************************************/
/**
 * @brief To configure the ADC threshold to compare threshold value with ADC data.
 * 
 * @details This API configures the ADC threshold values for comparison with ADC data.
 * This API applies when the ADC mode of operation is static.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 * 
 * @param[in] adc_threshold ADC threshold configuration structure variable, see \ref sl_adc_threshold_config_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully configured ADC threshold.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_threshold_configuration(sl_adc_threshold_config_t adc_threshold);

/***************************************************************************/
/**
 * @brief To read the ADC sample data for FIFO mode of operation.
 * 
 * @details This API reads the ADC sample data when the ADC is configured in FIFO mode.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 *  - \ref sl_si91x_adc_register_event_callback
 *  - \ref sl_si91x_adc_start
 * 
 * @param[in] adcchconfig ADC channels configuration structure variable, see \ref sl_adc_channel_config_t.
 * @param[in] channel_num Channel number.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully read the data.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_INVALID_RANGE      - Mismatch range.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_read_data(sl_adc_channel_config_t adcchconfig, uint8_t channel_num);

/***************************************************************************/
/**
 * @brief To read the ADC sampled data for static mode of operation.
 * 
 * @details This API reads the ADC sampled data when the ADC is configured in static mode.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init 
 *  - \ref sl_si91x_adc_set_channel_configuration
 *  - \ref sl_si91x_adc_register_event_callback
 *  - \ref sl_si91x_adc_start
 * 
 * @param[in] adc_channel_config ADC channels configuration structure variable, see \ref sl_adc_channel_config_t.
 * @param[in] adc_config ADC operation configuration structure variable, see \ref sl_adc_config_t.
 * @param[out] adc_value Store the reading data in adc_value.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully read the data.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_INVALID_RANGE      - Mismatch range.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_read_data_static(sl_adc_channel_config_t adc_channel_config,
                                          sl_adc_config_t adc_config,
                                          uint16_t *adc_value);

/***************************************************************************/
/**
 * @brief To get the channel sampling rate value configured to ADC.
 * 
 * @details This API retrieves the sampling rate value that is configured for the specified ADC channel.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 *  - \ref sl_si91x_adc_start
 * 
 * @param[in] channel_num Channel number.
 * 
 * @return uint32_t Sampling rate retrieved from the register.
 ******************************************************************************/
uint32_t sl_si91x_adc_get_sampling_rate(uint8_t channel_num);

/***************************************************************************/
/**
 * @brief To deinitialize the ADC.
 * 
 * @details This API deinitializes the ADC. If the DMA (Direct Memory Access) is enabled, it also deinitializes the
 * DMA. It will power down the ADC block and disable the dynamic mode
 * if FIFO mode of ADC operation is used.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init 
 * 
 * @param[in] adc_config ADC operation configuration structure variable, see \ref sl_adc_config_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully deinitialized the ADC.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_deinit(sl_adc_config_t adc_config);

/***************************************************************************/
/**
 * @brief To start the ADC operation.
 * 
 * @details This API starts the signal to the ADC controller and enables dynamic mode if FIFO mode of ADC operation is configured.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 *  - \ref sl_si91x_adc_register_event_callback
 * 
 * @param[in] adc_config ADC operation configuration structure variable, see \ref sl_adc_config_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully started the ADC.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_start(sl_adc_config_t adc_config);

/***************************************************************************/
/**
 * @brief To stop the ADC operation.
 * 
 * @details This API stops the signal to the ADC controller and disables dynamic mode if FIFO mode of ADC operation is configured.
 * 
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_adc_init
 *  - \ref sl_si91x_adc_set_channel_configuration
 *  - \ref sl_si91x_adc_register_event_callback
 *  - \ref sl_si91x_adc_start
 * 
 * @param[in] adc_config ADC operation configuration structure variable, see \ref sl_adc_config_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully stopped the ADC operation.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_adc_stop(sl_adc_config_t adc_config);

/***************************************************************************/
/**
 * @brief To get the ADC version.
 * 
 * @details This API returns the release, major, and minor version of the ADC.
 * 
 * @note Returns the API version of the ADC.
 * 
 * @return sl_adc_version_t Structure containing the version information, see \ref sl_adc_version_t.
 ******************************************************************************/
sl_adc_version_t sl_si91x_adc_get_version(void);

/***************************************************************************/
/**
 * @brief To get chip operating voltage (VBATT).
 * 
 * @details  1. This API reads the operating voltage of chip i.e vbatt voltage.
 *           2. If the operating voltage is less than 2.4V then this api switches the input voltage supply
 *              from SCDC to HPLDO.
 *           3. then returns the chip operating voltage (VBATT) in volts.
 *
 * @return chip operating voltage(VBATT) in volts.
 ***************************************************************************************/
float sl_si91x_adc_get_chip_voltage(void);

/// @} end group ADC ********************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY! ***********************
/** 
 * @addtogroup ADC Analog to Digital Converter
 * @{
 *
 * @details
 *
 * @section ADC_Intro Introduction
 *
 * The Analog-to-Digital Converter (ADC) is a crucial peripheral in microcontrollers, enabling the conversion of analog signals (continuous voltage levels) into digital data (discrete binary values). This conversion allows microcontrollers to interface with analog sensors, such as temperature sensors, light sensors, or joysticks, effectively translating real-world analog signals into data that can be processed by digital systems.
 *
 * @image html adc_blockDiagram.png "ADC Block Diagram"
 * 
 * ADCs are essential in a wide range of applications, including:
 *
 * 1. **Sensor Data Acquisition**: Converting sensor outputs into digital form for processing.
 * 2. **Audio Processing**: Digitizing analog audio signals for further manipulation.
 * 3. **Instrumentation and Measurement**: Precise measurement of analog signals.
 * 4. **Motor Control**: Monitoring and controlling motor performance.
 * 5. **Battery Monitoring and Management**: Ensuring optimal battery usage.
 * 6. **Feedback Control Systems**: Providing digital feedback for control systems.
 *
 * @section ADC_Config Configuration
 *
 * Configuring the ADC involves several key steps to ensure accurate and efficient operation. These steps vary based on the specific hardware and application requirements. Below are some essential configuration tasks:
 *
 * - **Channel and Mode Configuration**: Set the number of ADC channels and the operating mode by configuring the `sl_adc_config_t` structure. Use this structure with the API `sl_si91x_adc_set_channel_configuration()` to apply the settings.
 *
 * - **Channel-Specific Settings**: Define input type, sample count (1-1023), and sampling rate (1-2,500,000 samples per second) in the `sl_adc_channel_config_t` structure. After configuring this structure, call `sl_si91x_adc_set_channel_configuration()` to set the channel parameters.
 *
 * For more detailed configuration information, see the specific peripheral example README documents.
 *
 * After defining the ADC configuration structures, use the following functions to initialize and control the ADC module:
 *
 * **Static Mode (non-DMA) typical sequence:**
 * 1. **Initialize ADC**: @ref sl_si91x_adc_init
 * 2. **Set Channel Configuration**: @ref sl_si91x_adc_set_channel_configuration
 * 3. **Register Event Callback**: @ref sl_si91x_adc_register_event_callback
 * 4. **Start ADC**: @ref sl_si91x_adc_start
 * 5. **Read Data**: @ref sl_si91x_adc_read_data_static
 * 6. **Deinitialize ADC**: @ref sl_si91x_adc_deinit
 *
 * **FIFO Mode with DMA typical sequence:**
 * 
 * 1. **Initialize ADC**: @ref sl_si91x_adc_init
 * 2. **Set Channel Configuration**: @ref sl_si91x_adc_set_channel_configuration
 * 3. **Configure Ping/Pong Memory**: @ref sl_si91x_adc_configure_ping_pong_memory_address
 * 4. **Enable Ping/Pong**: @ref sl_si91x_adc_enable_ping_pong
 * 5. **Enable Channel**: @ref sl_si91x_adc_channel_enable
 * 6. **Enable Internal DMA**: @ref sl_si91x_adc_internal_per_channel_dma_enable
 * 7. **Configure FIFO Mode**: @ref sl_si91x_adc_configure_fifo_mode
 * 8. **Register Event Callback**: @ref sl_si91x_adc_register_event_callback
 * 9. **Start ADC**: @ref sl_si91x_adc_start
 * 10. **Read Data**: @ref sl_si91x_adc_read_data
 * 11. **Deinitialize ADC**: @ref sl_si91x_adc_deinit
 *
 * @section ADC_PingPong Ping/Pong Buffer Configuration for FIFO Mode
 *
 * When using FIFO mode with internal DMA, you must configure Ping/Pong buffers for dual-buffer cyclic operation:
 *
 * **Structure fields for Ping/Pong addresses:**
 * - **sl_adc_channel_config_t** (used during channel configuration):
 *   - `chnl_ping_address[channel]` : DMA Ping buffer base address
 *   - `chnl_pong_address[channel]` : DMA Pong buffer base address  
 *   - `rx_buf[channel]`            : CPU-accessible buffer for reading samples
 *
 * - **sl_adc_internal_config_t** (used with configure_ping_pong_memory_address API):
 *   - `ping_addr[channel]`   : Ping buffer base address
 *   - `pong_addr[channel]`   : Pong buffer base address
 *   - `ping_length[channel]` : Ping buffer length in samples
 *   - `pong_length[channel]` : Pong buffer length in samples
 *   - `input_type[channel]`  : Input type (single-ended/differential)
 *   - `num_of_samples[channel]` : Number of samples per conversion
 *
 * **Configuration Methods:**
 *
 * *Method 1 (Recommended):* Use the dedicated ping/pong configuration API:
 * @code
 * sl_adc_internal_config_t internal_config = {0};
 * uint8_t channel = 0;
 * 
 *  Configure ping/pong parameters
 * internal_config.ping_addr[channel]   = 0x24060000;
 * internal_config.pong_addr[channel]   = 0x24060400;
 * internal_config.ping_length[channel] = 256;
 * internal_config.pong_length[channel] = 256; 
 * internal_config.input_type[channel]  = SL_ADC_SINGLE_ENDED;
 * internal_config.num_of_samples[channel] = 256;
 * 
 * sl_si91x_adc_configure_ping_pong_memory_address(internal_config, channel);
 * sl_si91x_adc_enable_ping_pong(channel);
 * @endcode
 *
 * *Method 2:* Pre-populate channel config before set_channel_configuration:
 * @code
 * sl_adc_channel_config_t channel_config = {0};
 * uint8_t channel = 0;
 * 
 *  Pre-populate ping/pong addresses  
 * channel_config.chnl_ping_address[channel] = 0x24060000;
 * channel_config.chnl_pong_address[channel] = 0x24060400;
 * channel_config.rx_buf[channel] = my_cpu_buffer;
 * 
 * sl_si91x_adc_set_channel_configuration(channel_config, adc_config);
 * sl_si91x_adc_enable_ping_pong(channel);  
 * @endcode
 *
 * **Important Notes:**
 * - Ping/Pong buffers are NOT used in Static mode (non-DMA operation)
 * - Address ranges must be valid ULP SRAM locations (typically 0x24060000 - 0x24061B00)
 * - Buffer sizes should accommodate the expected number of samples per channel
 * - Always enable ping/pong mode after configuring the addresses
 *
 * @section ADC_Usage Usage
 *
 * After defining the ADC configuration structures, use the following functions to initialize and control the ADC module:
 *
 * 1. **Initialize ADC**: @ref sl_si91x_adc_init
 * 2. **Set Channel Configuration**: @ref sl_si91x_adc_set_channel_configuration
 * 3. **Register Event Callback**: @ref sl_si91x_adc_register_event_callback
 * 4. **Start ADC**: @ref sl_si91x_adc_start
 * 5. **Read Data**: @ref sl_si91x_adc_read_data
 * 6. **Deinitialize ADC**: @ref sl_si91x_adc_deinit
 *
 * @} (end addtogroup ADC)
 */

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_ADC_H_ */
