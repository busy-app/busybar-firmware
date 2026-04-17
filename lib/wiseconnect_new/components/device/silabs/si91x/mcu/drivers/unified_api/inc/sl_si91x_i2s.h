/******************************************************************************
* @file sl_si91x_i2s.h
* @brief I2S API implementation
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

#ifndef SL_SI91X_I2S_H
#define SL_SI91X_I2S_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"
#include "SAI.h"
#include "base_types.h"

/***************************************************************************/
/**
 * @addtogroup I2S I2S
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 *
 ******************************************************************************/
// -----------------------------------------------------------------------------
// Data Types

typedef ARM_SAI_SignalEvent_t sl_i2s_signal_event_t; ///< Renamed signal event structure
typedef ARM_SAI_STATUS sl_i2s_status_t;              ///< Renamed status structure
typedef ARM_DRIVER_SAI sl_i2s_driver_t;              ///< Renamed I2S driver structure
typedef const void *sl_i2s_handle_t;                 ///< Created I2S handle type

/***************************************************************************/
/**
 * @brief Enumeration for different I2S callback events.
 * 
 * @details This enumeration defines the various events that can trigger I2S callbacks.
 *          These events are used to notify the application of different I2S operation statuses.
 ******************************************************************************/
typedef enum {
  SL_I2S_SEND_COMPLETE    = ARM_SAI_EVENT_SEND_COMPLETE,    ///< Send complete event.
  SL_I2S_RECEIVE_COMPLETE = ARM_SAI_EVENT_RECEIVE_COMPLETE, ///< Receive complete event.
  SL_I2S_TX_UNDERFLOW     = ARM_SAI_EVENT_TX_UNDERFLOW,     ///< TX underflow event.
  SL_I2S_RX_OVERFLOW      = ARM_SAI_EVENT_RX_OVERFLOW,      ///< RX overflow event.
  SL_I2S_FRAME_ERROR      = ARM_SAI_EVENT_FRAME_ERROR       ///< Frame error event.
} i2s_event_typedef_t;

/***************************************************************************/
/**
 * @brief Enumeration for I2S power states.
 * 
 * @details This enumeration defines the different power states for the I2S peripheral.
 *          These states are used to manage the power consumption of the I2S module.
 ******************************************************************************/
typedef enum {
  SL_I2S_POWER_OFF  = ARM_POWER_OFF, ///< Power mode OFF.
  SL_I2S_FULL_POWER = ARM_POWER_FULL ///< Full power mode.
} sl_i2s_power_state_t;

/***************************************************************************/
/**
 * @brief Enumeration for I2S primary and secondary modes.
 * 
 * @details This enumeration defines the modes for the I2S peripheral, specifying whether it operates
 *          as a master or a slave. The master mode controls the clock generation, while the slave mode
 *          relies on an external clock.
 ******************************************************************************/
typedef enum {
  SL_I2S_MASTER = ARM_SAI_MODE_MASTER, ///< I2S primary (master) mode.
  SL_I2S_SLAVE  = ARM_SAI_MODE_SLAVE   ///< I2S secondary (slave) mode.
} sl_i2s_mode_t;

/***************************************************************************/
/**
 * @brief Enumeration for SAI protocol type.
 * 
 * @details This enumeration defines the protocol types for the SAI (Serial Audio Interface).
 *          It specifies whether the I2S or PCM protocol is used. Note that the PCM protocol
 *          is currently not supported in the driver.
 ******************************************************************************/
typedef enum {
  SL_I2S_PROTOCOL = ARM_SAI_PROTOCOL_I2S,      ///< I2S protocol.
  SL_PCM_PROTOCOL = ARM_SAI_PROTOCOL_PCM_SHORT ///< PCM protocol.
} sl_sai_protocol_t;

/***************************************************************************/
/**
 * @brief Enumeration for I2S SYNC/ASYNC modes.
 * 
 * @details This enumeration defines the synchronization modes for the I2S peripheral.
 *          It specifies whether the I2S operates in synchronous or asynchronous mode.
 ******************************************************************************/
typedef enum {
  SL_I2S_SYNC  = ARM_SAI_SYNCHRONOUS, ///< I2S synchronous mode.
  SL_I2S_ASYNC = ARM_SAI_ASYNCHRONOUS ///< I2S asynchronous mode.
} sl_i2s_sync_t;

/***************************************************************************/
/**
 * @brief Enumeration for I2S transfer types.
 * 
 * @details This enumeration defines the various transfer types for the I2S peripheral.
 *          It specifies whether the I2S is configured for transmitting, receiving, or aborting
 *          ongoing transfers.
 ******************************************************************************/
typedef enum {
  SL_I2S_TRANSMIT         = ARM_SAI_CONFIGURE_TX,  ///< I2S transmit.
  SL_I2S_RECEIVE          = ARM_SAI_CONFIGURE_RX,  ///< I2S receive.
  SL_I2S_SEND_ABORT       = ARM_SAI_ABORT_SEND,    ///< I2S abort transmit.
  SL_I2S_RECEIVE_ABORT    = ARM_SAI_ABORT_RECEIVE, ///< I2S abort receive.
  SL_MIC_ICS43434_RECEIVE = ARM_SAI_CONFIGURE_MIC  ///< I2S MIC ICS43434 Configuration.
} sl_i2s_xfer_type_t;

/***************************************************************************/
/**
 * @brief Enumeration for I2S transfer data size.
 * 
 * @details This enumeration defines the data sizes for I2S transfers.
 *          It specifies the number of bits per data word that can be transferred.
 ******************************************************************************/
typedef enum {
  SL_I2S_DATA_SIZE8  = 8,  ///< 8 bits per data word.
  SL_I2S_DATA_SIZE16 = 16, ///< 16 bits per data word.
  SL_I2S_DATA_SIZE32 = 32  ///< 32 bits per data word.
} sl_i2s_xfer_size_t;

/***************************************************************************/
/**
 * @brief Enumeration for I2S audio data resolutions.
 * 
 * @details This enumeration defines the various audio data resolutions for the I2S peripheral.
 *          It specifies the number of bits per audio sample that can be used.
 ******************************************************************************/
typedef enum {
  SL_I2S_RESOLUTION_16 = 16, ///< 16-bit resolution.
  SL_I2S_RESOLUTION_24 = 24, ///< 24-bit resolution.
  SL_I2S_RESOLUTION_32 = 32  ///< 32-bit resolution.
} sl_i2s_data_resolution_t;

/***************************************************************************/
/**
 * @brief Enumeration for I2S sample rates.
 * 
 * @details This enumeration defines the various sample rates for the I2S peripheral.
 *          It specifies the number of samples per second (in Hz) that can be used.
 ******************************************************************************/
typedef enum {
  SL_I2S_SAMPLING_RATE_8000   = 8000,  ///< 8 kHz
  SL_I2S_SAMPLING_RATE_11025  = 11025, ///< 11.025 kHz
  SL_I2S_SAMPLING_RATE_16000  = 16000, ///< 16 kHz
  SL_I2S_SAMPLING_RATE_22050  = 22050, ///< 22.05 kHz
  SL_I2S_SAMPLING_RATE_24000  = 24000, ///< 24 kHz
  SL_I2S_SAMPLING_RATE_32000  = 32000, ///< 32 kHz
  SL_I2S_SAMPLING_RATE_44100  = 44100, ///< 44.1 kHz
  SL_I2S_SAMPLING_RATE_48000  = 48000, ///< 48 kHz
  SL_I2S_SAMPLING_RATE_88200  = 88200, ///< 88.2 kHz
  SL_I2S_SAMPLING_RATE_96000  = 96000, ///< 96 kHz
  SL_I2S_SAMPLING_RATE_192000 = 192000 ///< 192 kHz
} sl_i2s_sampling_rate_t;

/***************************************************************************/
/**
 * @brief Structure to hold the version numbers of the peripheral API.
 * 
 * @details This structure contains the version information for the I2S peripheral API,
 *          including the release, major, and minor version numbers.
 ******************************************************************************/
typedef struct {
  uint8_t release; ///< Release version number.
  uint8_t major;   ///< Major version number.
  uint8_t minor;   ///< Minor version number.
} sl_i2s_version_t;

/***************************************************************************/
/**
 * @brief Structure to hold transmit and receive configuration parameters.
 * 
 * @details This structure contains the configuration parameters for I2S transmit and receive operations.
 *          It includes settings for mode, synchronization, protocol, resolution, data size, sampling rate, and transfer type.
 ******************************************************************************/
typedef struct {
  uint16_t mode;          ///< Primary/Secondary mode (Master/Slave).
  uint16_t sync;          ///< SYNC/ASYNC mode (Synchronous/Asynchronous).
  uint16_t protocol;      ///< I2S/PCM protocol (currently only I2S is supported).
  uint16_t resolution;    ///< Audio data resolution.
  uint32_t data_size;     ///< Data size in bits.
  uint32_t sampling_rate; ///< Audio sampling rate in Hz.
  uint32_t transfer_type; ///< Transfer type (Transmit/Receive).
} sl_i2s_xfer_config_t;

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/
/**
 * @brief To initialize the I2S peripheral.
 * 
 * @details This API enables the clock for the I2S peripheral and configures the SCLK, WSCLK, 
 *          DIN, and DOUT pins. It also requires the address of a pointer for storing the I2S handle,
 *          which can be used for other function calls.
 * 
 * @param[in] i2s_instance  0 - I2S0,
 *                          1 - I2S1
 * @param[in] i2s_handle    Double pointer to the I2S driver handle.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Operation successful.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as an argument.
 *         - SL_STATUS_BUSY  - Driver is busy.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_i2s_init(uint32_t i2s_instance, sl_i2s_handle_t *i2s_handle);

/***************************************************************************/
/**
 * @brief To uninitialize the I2S peripheral. This API is deprecated; please use sl_si91x_i2s_deinit_v2() instead.
 * 
 * @details This API disables the DMA instance used for I2S transfer and powers down the I2S peripheral
 *          by disabling the clock supply to I2S.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_i2s_init must be called before this function.
 * 
 * @param[in] i2s_handle Pointer to the I2S driver handle.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Operation successful.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as an argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note
 * When the I2S module is used in combination with other peripherals, see the notes below while deinitializing in the application:
 * 1. Whenever sl_si91x_i2s_deinit() is called, it will disable the clock for the peripheral. To power off the peripheral, 
 *    you have to power down the power domain (PERI_EFUSE) which contains the following peripherals:
 *    USART, UART, I2C, SSI Master, SSI Slave, Generic-SPI Master, I2S Master, I2S Slave, Micro-DMA Controller, Config Timer,
 *    Random-Number Generator, CRC Accelerator, SIO, QEI, MCPWM, and EFUSE.
 *    Use the following API to power down the particular power domain if other peripherals are not being used:
 *    sl_si91x_peri_efuse_power_down()
 * 
 * 2. Some peripherals (ULP Peripherals, UULP Peripherals, GPDMA, and SDIO-SPI) have separate domains and can be powered down independently. 
 *    For additional details, see the Power Architecture section in the Hardware Reference Manual.
 *    The ULP_UART has a separate power domain, ULPSS_PWRGATE_ULP_UART, which can be powered down independently. 
 *    See the rsi_power_save.h file for all power gate definitions.
 ******************************************************************************/
sl_status_t sl_si91x_i2s_deinit(sl_i2s_handle_t *i2s_handle) SL_DEPRECATED_API_WISECONNECT_4_0;
/***************************************************************************/
/**
 * @brief To uninitialize the I2S peripheral,new verison v2.
 *
 * @details This API disables the DMA instance used for I2S transfer and powers down the I2S peripheral
 *          by disabling the clock supply to I2S.
 *
 * @pre Pre-condition:
 *      - \ref sl_si91x_i2s_init must be called before this function.
 *
 * @param[in] i2s_handle Pointer to the I2S driver handle.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Operation successful.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as an argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *
 * @note
 * When the I2S module is used in combination with other peripherals, see the notes below while deinitializing in the application:
 * 1. Whenever sl_si91x_i2s_deinit_v2() is called, it will disable the clock for the peripheral. To power off the peripheral,
 *    you have to power down the power domain (PERI_EFUSE) which contains the following peripherals:
 *    USART, UART, I2C, SSI Master, SSI Slave, Generic-SPI Master, I2S Master, I2S Slave, Micro-DMA Controller, Config Timer,
 *    Random-Number Generator, CRC Accelerator, SIO, QEI, MCPWM, and EFUSE.
 *    Use the following API to power down the particular power domain if other peripherals are not being used:
 *    sl_si91x_peri_efuse_power_down()
 *
 * 2. Some peripherals (ULP Peripherals, UULP Peripherals, GPDMA, and SDIO-SPI) have separate domains and can be powered down independently.
 *    For additional details, see the Power Architecture section in the Hardware Reference Manual.
 *    The ULP_UART has a separate power domain, ULPSS_PWRGATE_ULP_UART, which can be powered down independently.
 *    See the rsi_power_save.h file for all power gate definitions.
 ******************************************************************************/
sl_status_t sl_si91x_i2s_deinit_v2(sl_i2s_handle_t i2s_handle);

/***************************************************************************/
/**
 * @brief To configure the I2S power mode.
 * 
 * @details This API configures the power mode of the I2S peripheral. The supported modes are:
 *          - SL_I2S_POWER_OFF: I2S peripheral clocks and I2S DMA channel are disabled.
 *          - SL_I2S_FULL_POWER: Enables I2S peripheral clocks and configures the I2S DMA channel.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_i2s_init must be called before this function.
 * 
 * @param[in] i2s_handle Pointer to the I2S driver handle.
 * @param[in] state Power state, either SL_I2S_POWER_OFF or SL_I2S_FULL_POWER.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as an argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_i2s_configure_power_mode(sl_i2s_handle_t i2s_handle, sl_i2s_power_state_t state);

/***************************************************************************/
/**
 * @brief To configure transmit/receive parameters for I2S transfer.
 * 
 * @details This API configures the I2S DMA transmit and receive channels. 
 *          Setting the transfer_type to SL_I2S_TRANSMIT will configure the Tx channel, 
 *          while setting it to SL_I2S_RECEIVE will configure the Rx channel.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_i2s_init must be called before this function.
 *      - \ref sl_si91x_i2s_configure_power_mode must be called before this function.
 * 
 * @param[in] i2s_handle Pointer to the I2S driver handle.
 * @param[in] xfer_config Pointer to the structure that stores transfer parameters.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Operation successful.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as an argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_i2s_config_transmit_receive(sl_i2s_handle_t i2s_handle, sl_i2s_xfer_config_t *xfer_config);

/***************************************************************************/
/**
 * @brief To transmit I2S data.
 * 
 * @details This API configures the I2S Tx DMA channel descriptors and triggers 
 *          the DMA transfer. The function sl_si91x_i2s_config_transmit_receive() 
 *          should be called with transfer_type set to SL_I2S_TRANSMIT before sending data.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_i2s_init must be called before this function.
 *      - \ref sl_si91x_i2s_configure_power_mode must be called before this function.
 *      - \ref sl_si91x_i2s_config_transmit_receive must be called before this function.
 * 
 * @param[in] i2s_handle Pointer to the I2S driver handle.
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
sl_status_t sl_si91x_i2s_transmit_data(sl_i2s_handle_t i2s_handle, const void *data, uint32_t size);

/***************************************************************************/
/**
 * @brief To receive I2S data.
 * 
 * @details This API configures the I2S Rx DMA channel descriptors and enables the DMA
 *          channel to receive data. The function sl_si91x_i2s_config_transmit_receive()
 *          should be called with transfer_type set to SL_I2S_RECEIVE before receiving data.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_i2s_init must be called before this function.
 *      - \ref sl_si91x_i2s_configure_power_mode must be called before this function.
 *      - \ref sl_si91x_i2s_config_transmit_receive must be called before this function.
 * 
 * @param[in] i2s_handle Pointer to the I2S driver handle.
 * @param[in] data Address of the receive data buffer.
 * @param[in] size Size of the data to be received.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as an argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_i2s_receive_data(sl_i2s_handle_t i2s_handle, const void *data, uint32_t size);

/***************************************************************************/
/**
 * @brief To register the event callback function.
 * 
 * @details This API registers a callback function to notify the application of transfer complete events.
 *          Callbacks should be registered before starting the I2S transfer.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_i2s_init must be called before this function.
 *      - \ref sl_si91x_i2s_configure_power_mode must be called before this function.
 * 
 * @param[in] i2s_handle Pointer to the I2S driver handle.
 * @param[in] callback_event Pointer to the function to be called at the time of interrupt.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_BUSY  - Driver is busy.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_i2s_register_event_callback(sl_i2s_handle_t i2s_handle, sl_i2s_signal_event_t callback_event);

/***************************************************************************/
/**
 * @brief To unregister the event callback function.
 * 
 * @details This function unregisters the previously registered callback function.
 *          It can be used only if callbacks are registered using sl_si91x_i2s_register_event_callback().
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_i2s_register_event_callback must be called before this function.
 * 
 * @param[in] i2s_handle Pointer to the I2S driver handle.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_i2s_unregister_event_callback(sl_i2s_handle_t i2s_handle);

/***************************************************************************/
/**
 * @brief To get the transmitted data count of I2S.
 * 
 * @details This function retrieves the count of data bytes that have been transmitted by the I2S peripheral.
 *          It can be used to monitor the progress of an ongoing I2S data transfer.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_i2s_init must be called before this function.
 *      - \ref sl_si91x_i2s_configure_power_mode must be called before this function.
 *      - \ref sl_si91x_i2s_config_transmit_receive must be called before this function.
 *      - \ref sl_si91x_i2s_transmit_data must be called before this function.
 * 
 * @param[in] i2s_handle Pointer to the I2S driver handle.
 * 
 * @return uint32_t The number of data bytes transmitted.
 ******************************************************************************/
uint32_t sl_si91x_i2s_get_transmit_data_count(sl_i2s_handle_t i2s_handle);

/***************************************************************************/
/**
 * @brief To get the received data count of I2S.
 * 
 * @details This function retrieves the count of data bytes that have been received by the I2S peripheral.
 *          It can be used to monitor the progress of an ongoing I2S data reception.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_i2s_init must be called before this function.
 *      - \ref sl_si91x_i2s_configure_power_mode must be called before this function.
 *      - \ref sl_si91x_i2s_config_transmit_receive must be called before this function.
 *      - \ref sl_si91x_i2s_receive_data must be called before this function.
 * 
 * @param[in] i2s_handle Pointer to the I2S driver handle.
 * 
 * @return uint32_t The number of data bytes received.
 ******************************************************************************/
uint32_t sl_si91x_i2s_get_receive_data_count(sl_i2s_handle_t i2s_handle);

/***************************************************************************/
/**
 * @brief To get the release, major, and minor version of the I2S peripheral.
 * 
 * @details This function retrieves the version information of the I2S peripheral, including
 *          the release, major, and minor version numbers.
 * 
 * @return sl_i2s_version_t Structure containing the version information:
 *         - release: Release version number.
 *         - major: Major version number.
 *         - minor: Minor version number.
 ******************************************************************************/
sl_i2s_version_t sl_si91x_i2s_get_version(void);

/***************************************************************************/
/**
 * @brief To get the I2S peripheral status.
 * 
 * @details This function retrieves the current status of the I2S peripheral, including:
 *          1. Frame error
 *          2. Rx busy
 *          3. Rx overflow
 *          4. Tx busy
 *          5. Tx underflow
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_i2s_init must be called before this function.
 *      - \ref sl_si91x_i2s_configure_power_mode must be called before this function.
 *      - \ref sl_si91x_i2s_config_transmit_receive must be called before this function.
 * 
 * @param[in] i2s_handle Pointer to the I2S driver handle.
 * 
 * @return sl_i2s_status_t Structure containing the I2S status information.
 ******************************************************************************/
sl_i2s_status_t sl_si91x_i2s_get_status(sl_i2s_handle_t i2s_handle);

/***************************************************************************/
/**
 * @brief To abort the I2S transfer.
 * 
 * @details This function disables the I2S clocks and DMA channel, stopping the WSCLK generation.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_i2s_init must be called before this function.
 *      - \ref sl_si91x_i2s_configure_power_mode must be called before this function.
 *      - \ref sl_si91x_i2s_config_transmit_receive must be called before this function.
 *      - \ref sl_si91x_i2s_transmit_data or \ref sl_si91x_i2s_receive_data must be called before this function.
 * 
 * @param[in] i2s_handle Pointer to the I2S driver handle.
 * @param[in] abort_type Abort type, either ARM_SAI_ABORT_SEND or ARM_SAI_ABORT_RECEIVE.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as an argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_i2s_end_transfer(sl_i2s_handle_t i2s_handle, sl_i2s_xfer_type_t abort_type);

/// @} end group I2S ********************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/** @addtogroup I2S I2S
* @{
*
* @details
*
* @section I2S_Intro Introduction
*
* Inter-IC Sound (I2S) is a serial communication protocol intended to
* convey digital audio data between integrated circuits (ICs) in electronic devices.
* It is commonly used for connecting audio components like digital signal processors,
* digital-to-analog converters, and microphones.
*
* @image html i2s_blockdiagram.png "I2S Block Diagram"
* @image html i2s_frame_format.png "I2S Frame Format"
*
* In an I2S connection, one device typically serves as the master, generating the clock signals,
* while the other device operates as the slave, synchronizing its data transmission to the master's clock.
* The following is a brief discussion of the I2S peripheral's capabilities:
* 1. Programmable audio resolutions
* 2. Sampling rates
* 3. Full duplex communication
* 
* @section I2S_Config Configuration
* 
* I2S can be configured using several parameters:
*  1. The I2S module supports 3 programmable audio resolutions @ref sl_i2s_data_resolution_t  
*  2. I2S also supports 11 audio sampling rates @ref sl_i2s_sampling_rate_t 
* 
* @li For more information on configuring available parameters, see the respective peripheral example readme document.
* 
* @section I2S_Use Usage
* 
* The default I2S is initialized and configured with a 16-bit resolution and an 8 kHz sampling rate.
* Just after calling the register callback, it will go into transmit-receive mode and communicate over
* the given interrupt.
* 
*  1. Initialize the I2S: @ref sl_si91x_i2s_init
*  2. Register event callback: @ref sl_si91x_i2s_register_event_callback
*  3. Perform data operations: 
*    - Transmit data: @ref sl_si91x_i2s_transmit_data
*    - Receive data: @ref sl_si91x_i2s_receive_data
*  4. Deinitialize the I2S: @ref sl_si91x_i2s_deinit
* 
* @li Some I2S APIs to configure the power modes, unregister the callback, get the data count,
* and abort the transfer include:
* 1. Configure power mode: @ref sl_si91x_i2s_configure_power_mode
* 2. Unregister event callback: @ref sl_si91x_i2s_unregister_event_callback
* 3. Get received data count: @ref sl_si91x_i2s_get_receive_data_count
* 4. Abort I2S transfer: @ref sl_si91x_i2s_end_transfer
* 
* @} (end addtogroup I2S)
*/

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_I2S_H */
