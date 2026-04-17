/***************************************************************************/
/**
 * @file sl_si91x_pcm.h
 * @brief 
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#ifndef __SL_SI91X_PCM_H__
#define __SL_SI91X_PCM_H__

#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"
#include "sl_si91x_i2s.h"

/***************************************************************************/
/**
 * @addtogroup PCM PCM
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 * 
 ***************************************************************************/
// --------------------------------------------------------------------------
/***************************************************************************/
/**
 * @brief PCM0 Instance Identifier
 *
 * @details Macro to select the PCM0 instance.
 ******************************************************************************/
#define SL_SI91X_PCM0_INSTANCE 0

/***************************************************************************/
/**
 * @brief ULP_PCM Instance Identifier
 *
 * @details Macro to select the ULP_PCM instance.
 ******************************************************************************/
#define SL_SI91X_ULP_PCM_INSTANCE 1

/***************************************************************************/
/**
 * @brief Enumeration for PCM audio data resolutions.
 * 
 * @details This enumeration defines the various audio data resolutions for the PCM PCM peripheral.
 *          It specifies the number of bits per audio sample that can be used.
 ******************************************************************************/

typedef enum {
  SL_PCM_RESOLUTION_12 = 12, ///< 12-bit resolution.
  SL_PCM_RESOLUTION_16 = 16, ///< 16-bit resolution.
  SL_PCM_RESOLUTION_20 = 20, ///< 20-bit resolution.
  SL_PCM_RESOLUTION_24 = 24, ///< 24-bit resolution.
  SL_PCM_RESOLUTION_32 = 32  ///< 32-bit resolution.
} sl_pcm_data_resolution_t;

/***************************************************************************/
/**
 * @brief Enumeration for PCM sample rates.
 *
 * @details This enumeration defines the various sample rates for the PCM PCM peripheral.
 *          It specifies the number of samples per second (in Hz) that can be used.
 ******************************************************************************/
typedef enum {
  SL_PCM_SAMPLING_RATE_8000  = 8000,  ///< 8 kHz
  SL_PCM_SAMPLING_RATE_11025 = 11025, ///< 11.025 kHz
  SL_PCM_SAMPLING_RATE_16000 = 16000, ///< 16 kHz
  SL_PCM_SAMPLING_RATE_22050 = 22050, ///< 22.05 kHz
  SL_PCM_SAMPLING_RATE_24000 = 24000, ///< 24 kHz
} sl_pcm_sampling_rate_t;

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/
/**
 * @brief To initialize the PCM peripheral.
 * 
 * @details This API enables the clock for the PCM peripheral and configures the SCLK, WSCLK, 
 *          DIN, and DOUT pins. It also requires the address of a pointer for storing the PCM handle,
 *          which can be used for other function calls.
 * 
 * @param[in] pcm_handle Double pointer to the I2S/PCM driver handle \ref sl_i2s_handle_t.
 * @param[in] pcm_instance  0 - PCM0,
 *                          1 - PCM1 (ULP_PCM)
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Operation successful.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as an argument.
 *         - SL_STATUS_BUSY  - Driver is busy.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_pcm_init(uint32_t pcm_instance, sl_i2s_handle_t *pcm_handle);

/***************************************************************************/
/**
 * @brief To uninitialize the PCM peripheral.This API is deprecated; please use sl_si91x_pcm_deinit_v2() instead.
 * 
 * @details This API disables the DMA instance used for PCM transfer and powers down the PCM peripheral
 *          by disabling the clock supply to PCM.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_pcm_init must be called before this function.
 * 
 * @param[in] pcm_handle Double pointer to the I2S/PCM driver handle \ref sl_i2s_handle_t.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Operation successful.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as an argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note
 * When the PCM module is used in combination with other peripherals, see the notes below while deinitializing in the application:
 * 1. Whenever sl_si91x_pcm_deinit() is called, it will disable the clock for the peripheral. To power off the peripheral, 
 *    you have to power down the power domain (PERI_EFUSE) which contains the following peripherals:
 *    USART, UART, I2C, SSI Master, SSI Slave, Generic-SPI Master, PCM Master, PCM Slave, Micro-DMA Controller, Config Timer,
 *    Random-Number Generator, CRC Accelerator, SIO, QEI, MCPWM, and EFUSE.
 *    Use the following API to power down the particular power domain if other peripherals are not being used:
 *    sl_si91x_peri_efuse_power_down()
 * 
 * 2. Some peripherals (ULP Peripherals, UULP Peripherals, GPDMA, and SDIO-SPI) have separate domains and can be powered down independently. 
 *    For additional details, see the Power Architecture section in the Hardware Reference Manual.
 *    The ULP_UART has a separate power domain, ULPSS_PWRGATE_ULP_UART, which can be powered down independently. 
 *    See the rsi_power_save.h file for all power gate definitions.
 ******************************************************************************/
sl_status_t sl_si91x_pcm_deinit(sl_i2s_handle_t *pcm_handle) SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/
/**
 * @brief To uninitialize the PCM peripheral,new version v2
 * 
 * @details This API disables the DMA instance used for PCM transfer and powers down the PCM peripheral
 *          by disabling the clock supply to PCM.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_pcm_init must be called before this function.
 * 
 * @param[in] pcm_handle pointer to the I2S/PCM driver handle \ref sl_i2s_handle_t.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Operation successful.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as an argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note
 * When the PCM module is used in combination with other peripherals, see the notes below while deinitializing in the application:
 * 1. Whenever sl_si91x_pcm_deinit() is called, it will disable the clock for the peripheral. To power off the peripheral, 
 *    you have to power down the power domain (PERI_EFUSE) which contains the following peripherals:
 *    USART, UART, I2C, SSI Master, SSI Slave, Generic-SPI Master, PCM Master, PCM Slave, Micro-DMA Controller, Config Timer,
 *    Random-Number Generator, CRC Accelerator, SIO, QEI, MCPWM, and EFUSE.
 *    Use the following API to power down the particular power domain if other peripherals are not being used:
 *    sl_si91x_peri_efuse_power_down()
 * 
 * 2. Some peripherals (ULP Peripherals, UULP Peripherals, GPDMA, and SDIO-SPI) have separate domains and can be powered down independently. 
 *    For additional details, see the Power Architecture section in the Hardware Reference Manual.
 *    The ULP_UART has a separate power domain, ULPSS_PWRGATE_ULP_UART, which can be powered down independently. 
 *    See the rsi_power_save.h file for all power gate definitions.
 ******************************************************************************/
sl_status_t sl_si91x_pcm_deinit_v2(sl_i2s_handle_t pcm_handle);

/***************************************************************************/
/**
 * @brief To configure the PCM module. Note that PCM supports only mono mode. 
 * 
 * @details This API configures sampling frequency and resolution of the PCM module.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_pcm_init must be called before this function.
 * 
 * @param[in] pcm_handle Pointer to the I2S/PCM driver handle \ref sl_i2s_handle_t.
 * @param[in] pcm_sampling_frequency The sampling frequency for sending or eceiving data.
 * @param[in] pcm_resolution Data resolution for the data being sent or received.
 * @param[in] mode Mode of operation (e.g., master or slave mode).
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_pcm_set_configuration(sl_i2s_handle_t pcm_handle,
                                           uint16_t pcm_sampling_frequency,
                                           uint16_t pcm_resolution,
                                           uint16_t mode);

/***************************************************************************/
/**
 * @brief To transmit PCM data.
 * 
 * @details This API configures the PCM Tx DMA channel descriptors and triggers 
 *          the DMA transfer.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_pcm_init must be called before this function.
 *      - \ref sl_si91x_pcm_set_configuration must be called before this function.
 * 
 * @param[in] pcm_handle Pointer to the I2S/PCM driver handle \ref sl_i2s_handle_t.
 * @param[in] data Address of the transmit data.
 * @param[in] size Size of the data to be transmitted.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as an argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_pcm_transmit_data(sl_i2s_handle_t pcm_handle, const void *data, uint32_t size);

/***************************************************************************/
/**
 * @brief To receive PCM data.
 * 
 * @details This API configures the PCM Rx DMA channel descriptors and triggers 
 *          the DMA transfer.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_pcm_init must be called before this function.
 *      - \ref sl_si91x_pcm_set_configuration must be called before this function.
 * 
 * @param[in] pcm_handle Pointer to the I2S/PCM driver handle \ref sl_i2s_handle_t.
 * @param[in] data Address of the receive data.
 * @param[in] size Size of the data to be received.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as an argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_pcm_receive_data(sl_i2s_handle_t pcm_handle, const void *data, uint32_t size);

/***************************************************************************/
/**
 * @brief To register the event callback function.
 * 
 * @details This API registers a callback function to notify the application of transfer complete events.
 *          Callbacks should be registered before starting the PCM transfer.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_pcm_init must be called before this function.
 * 
 * @param[in] pcm_handle Pointer to the I2S/PCM driver handle \ref sl_i2s_handle_t.
 * @param[in] callback_event Pointer to the function to be called at the time of interrupt.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_BUSY  - Driver is busy.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *  ******************************************************************************/
sl_status_t sl_si91x_pcm_register_event_callback(sl_i2s_handle_t pcm_handle, sl_i2s_signal_event_t callback_event);

/***************************************************************************/
/**
 * @brief To unregister the event callback function.
 * 
 * @details This function unregisters the previously registered callback function.
 *          It can be used only if callbacks are registered using sl_si91x_pcm_register_event_callback().
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_pcm_register_event_callback must be called before this function.
 * 
 * @param[in] pcm_handle Pointer to the I2S/PCM driver handle \ref sl_i2s_handle_t.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_pcm_unregister_event_callback(sl_i2s_handle_t pcm_handle);

/***************************************************************************/
/**
 * @brief To abort the PCM transfer.
 * 
 * @details This function disables the PCM clocks and DMA channel, stopping the frame-sync generation.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_pcm_init must be called before this function.
 *      - \ref sl_si91x_pcm_transmit_data or \ref sl_si91x_pcm_receive_data must be called before this function.
 * 
 * @param[in] pcm_handle Pointer to the I2S/PCM driver handle \ref sl_i2s_handle_t.
 * @param[in] abort_type Abort type, either ARM_SAI_ABORT_SEND or ARM_SAI_ABORT_RECEIVE.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as an argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_pcm_end_transfer(sl_i2s_handle_t pcm_handle, sl_i2s_xfer_type_t abort_type);

/***************************************************************************/
/**
 * @brief To get the transmitted data count of PCM.
 * 
 * @details This function retrieves the count of data bytes that have been transmitted by the PCM module.
 *          It can be used to monitor the progress of an ongoing PCM data transfer.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_pcm_init must be called before this function.
 *      - \ref sl_si91x_pcm_transmit_data must be called before this function.
 *
 * @param[in] pcm_handle Pointer to the I2S/PCM driver handle \ref sl_i2s_handle_t.
 * 
 * @return uint32_t The number of data bytes transmitted.
 ******************************************************************************/
uint32_t sl_si91x_pcm_get_transmit_data_count(sl_i2s_handle_t pcm_handle);

/***************************************************************************/
/**
 * @brief To get the received data count of PCM.
 * 
 * @details This function retrieves the count of data bytes that have been received by the PCM module.
 *          It can be used to monitor the progress of an ongoing PCM data transfer.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_pcm_init must be called before this function.
 *      - \ref sl_si91x_pcm_receive_data must be called before this function.
 *
 * @param[in] pcm_handle Pointer to the I2S/PCM driver handle \ref sl_i2s_handle_t.
 * 
 * @return uint32_t The number of data bytes received.
 ******************************************************************************/
uint32_t sl_si91x_pcm_get_receive_data_count(sl_i2s_handle_t pcm_handle);

/***************************************************************************/
/**
 * @brief To get the PCM peripheral status.
 * 
 * @details This function retrieves the current status of the PCM module, including:
 *          1. Frame error
 *          2. Rx busy
 *          3. Rx overflow
 *          4. Tx busy
 *          5. Tx underflow
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_pcm_init must be called before this function.
 *
 * @param[in] pcm_handle Pointer to the I2S/PCM driver handle \ref sl_i2s_handle_t.
 *
 * @return sl_pcm_status_t Structure containing the pcm status information.
 ******************************************************************************/
sl_i2s_status_t sl_si91x_pcm_get_status(sl_i2s_handle_t pcm_handle);

/// @} end of group PCM ****************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/** @addtogroup PCM PCM
* @{
*
* @details
*
* @section PCM_Intro Introduction
*
* PCM protocol is widely used in audio applications for transmitting and storing audio data.
* The PCM peripheral in the SI91x family supports mono audio data transmission and reception
* with configurable sampling rates and resolutions.
*
* The PCM peripheral provides the following features:
* 1. Programmable audio resolutions
* 2. Configurable sampling rates
* 3. Mono audio support
* 4. DMA-based data transfer
* 
* @section PCM_Config Configuration
* 
* The PCM module can be configured using the following parameters:
*  1. Audio resolutions supported: @ref sl_pcm_data_resolution_t  
*  2. Sampling rates supported: @ref sl_pcm_sampling_rate_t 
* 
* @li For more information on configuring available parameters, see the respective peripheral example readme document.
* 
* @section PCM_Use Usage
* 
* The PCM module can be initialized and configured to transmit or receive audio data.
* Below are the steps to use the PCM module:
* 
*  1. Initialize the PCM: @ref sl_si91x_pcm_init
*  2. Configure PCM settings: @ref sl_si91x_pcm_set_configuration
*  3. Perform data operations: 
*    - Transmit data: @ref sl_si91x_pcm_transmit_data
*    - Receive data: @ref sl_si91x_pcm_receive_data
*  4. Deinitialize the PCM: @ref sl_si91x_pcm_deinit
* 
* @li Additional PCM APIs include:
* 1. Register event callback: @ref sl_si91x_pcm_register_event_callback
* 2. Unregister event callback: @ref sl_si91x_pcm_unregister_event_callback
* 3. Get transmitted data count: @ref sl_si91x_pcm_get_transmit_data_count
* 4. Get received data count: @ref sl_si91x_pcm_get_receive_data_count
* 5. Abort PCM transfer: @ref sl_si91x_pcm_end_transfer
* 6. Get PCM status: @ref sl_si91x_pcm_get_status
* 
* (end addtogroup PCM)
*/
/** @} */
#ifdef __cplusplus

#endif

#endif
