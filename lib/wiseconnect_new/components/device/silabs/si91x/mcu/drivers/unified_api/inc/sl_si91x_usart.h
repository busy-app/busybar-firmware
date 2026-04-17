/******************************************************************************
* @file sl_si91x_usart.h
* @brief USART API implementation
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

#ifndef SL_SI91X_USART_H
#define SL_SI91X_USART_H

#ifdef __cplusplus
extern "C" {
#endif

// Includes
#include "sl_status.h"
#include "USART.h"
#include "rsi_usart.h"
#if (SLI_SI91X_MCU_RS485_MODE == 1)
#include "rsi_rs485.h"
#include "sl_rs485_board.h"
#endif
#ifdef DEBUG_UART_UC
#include "sl_si91x_debug_uc_config.h"
#endif
#include "base_types.h"

/***************************************************************************/ /**
* @addtogroup USART USART
* @ingroup SI91X_PERIPHERAL_APIS
* @{
*
******************************************************************************/

/*******************************************************************************
**************************   DEFINES/MACROS     ********************************
******************************************************************************/
typedef ARM_USART_SignalEvent_t sl_usart_signal_event_t;  ///< Alias for ARM_USART_SignalEvent_t type
typedef ARM_USART_STATUS sl_usart_status_t;               ///< Alias for ARM_USART_Status type
typedef ARM_POWER_STATE sl_usart_power_state_t;           ///< Alias for ARM_POWER_STATE type
typedef ARM_USART_MODEM_CONTROL sl_usart_modem_control_t; ///< Alias for ARM_USART_MODEM_CONTROL type
typedef ARM_USART_MODEM_STATUS sl_usart_modem_status_t;   ///< Alias for ARM_USART_MODEM_STATUS type
typedef ARM_USART_CAPABILITIES sl_usart_capabilities_t;   ///< Alias for ARM_USART_CAPABILITIES type
typedef ARM_DRIVER_USART sl_usart_driver_t;               ///< Alias for ARM_DRIVER_USART type
typedef USART_RESOURCES usart_resources_t;                ///< Alias for USART_RESOURCES type
typedef const void *sl_usart_handle_t; ///< Alias for a constant void pointer type, used as a handle for USART resources

/*******************************************************************************
********************************   ENUM     ************************************
******************************************************************************/
/**
 * @brief Enumeration for USART/UART events.
 * 
 * This enumeration defines the various events that can occur in the USART/UART interface.
 * Each event corresponds to a specific condition or status change in the USART/UART operation.
 */
typedef enum {
  SL_USART_EVENT_SEND_COMPLETE     = ARM_USART_EVENT_SEND_COMPLETE,     ///< Send complete event.
  SL_USART_EVENT_RECEIVE_COMPLETE  = ARM_USART_EVENT_RECEIVE_COMPLETE,  ///< Receive complete event.
  SL_USART_EVENT_TRANSFER_COMPLETE = ARM_USART_EVENT_TRANSFER_COMPLETE, ///< Transfer complete event.
  SL_USART_EVENT_TX_COMPLETE       = ARM_USART_EVENT_TX_COMPLETE,       ///< TX complete event.
  SL_USART_EVENT_TX_UNDERFLOW      = ARM_USART_EVENT_TX_UNDERFLOW,      ///< TX underflow event.
  SL_USART_EVENT_RX_OVERFLOW       = ARM_USART_EVENT_RX_OVERFLOW,       ///< RX overflow event.
  SL_USART_EVENT_RX_TIMEOUT        = ARM_USART_EVENT_RX_TIMEOUT,        ///< RX timeout event.
  SL_USART_EVENT_RX_BREAK          = ARM_USART_EVENT_RX_BREAK,          ///< RX break event.
  SL_USART_EVENT_RX_FRAMING_ERROR  = ARM_USART_EVENT_RX_FRAMING_ERROR,  ///< RX framing error event.
  SL_USART_EVENT_RX_PARITY_ERROR   = ARM_USART_EVENT_RX_PARITY_ERROR,   ///< RX parity error event.
  SL_USART_EVENT_CTS               = ARM_USART_EVENT_CTS,               ///< CTS (Clear to Send) event.
  SL_USART_EVENT_DSR               = ARM_USART_EVENT_DSR,               ///< DSR (Data Set Ready) event.
  SL_USART_EVENT_DCD               = ARM_USART_EVENT_DCD,               ///< DCD (Data Carrier Detect) event.
  SL_USART_EVENT_RI                = ARM_USART_EVENT_RI                 ///< RI (Ring Indicator) event.
} usart_event_typedef_t;

/**
 * @brief Enumeration for general power states used in this peripheral.
 * 
 * This enumeration defines the various power states that can be used to control the power
 * mode of the peripheral. Each state corresponds to a specific power configuration.
 */
typedef enum {
  SL_POWER_OFF  = ARM_POWER_OFF,  ///< Power off state.
  SL_POWER_LOW  = ARM_POWER_LOW,  ///< Low power state.
  SL_POWER_FULL = ARM_POWER_FULL, ///< Full power state.
  SL_POWER_MODE_LAST              ///< Last power mode for validation purposes.
} power_mode_typedef_t;

/// @brief UART RS485 modes
typedef enum {
  SL_UART_FULL_DUPLEX_MODE,         ///< Full Duplex mode
  SL_UART_SW_CTRL_HALF_DUPLEX_MODE, ///< Software controlled Half duplex mode
  SL_UART_HW_CTRL_HALF_DUPLEX_MODE, ///< Hardware controlled Half duplex mode
  SL_UART_INVALID_TRANSFER_MODE,    ///< Invalid transfer mode
} uart_rs485_mode_typedef_t;

/// @brief UART RS485 DRIVER ENABLE polarity
typedef enum {
  SL_UART_DRIVER_EN_POL_LOW,  ///< Driver enable polarity low
  SL_UART_DRIVER_EN_POL_HIGH, ///< Driver enable polarity high
} uart_rs485_driver_en_pol_typedef_t;

/// @brief UART RS485 Receiver ENABLE polarity
typedef enum {
  SL_UART_RECEIVER_EN_POL_LOW,  ///< Receiver Enable polarity low
  SL_UART_RECEIVER_EN_POL_HIGH, ///< Receiver Enable polarity High
} uart_rs485_receiver_en_pol_typedef_t;

/// @brief UART RS485 RS485 Transmit mode
typedef enum {
  SL_UART_RS485_TX_MODE_0, ///< THR is 8-bit wide.The user needs to program the address into TAR and data into the THR register.SEND_ADDR bit is used as a control knob to send the address.
  SL_UART_RS485_TX_MODE_1, ///< THR is 9-bit wide.User must ensure THR register is written correctly for address/data. Address: 9th bit is set to 1, Data: 9th bit is set to 0.
} uart_rs485_transmit_mode_typedef_t;

/**
 * @brief Enumeration for USART data bits configuration.
 * 
 * This enumeration defines the possible data bit configurations for the USART interface.
 * The user can select the appropriate data bit setting based on their application requirements.
 */
typedef enum {
  SL_USART_DATA_BITS_5 = ARM_USART_DATA_BITS_5, ///< 5 data bits.
  SL_USART_DATA_BITS_6 = ARM_USART_DATA_BITS_6, ///< 6 data bits.
  SL_USART_DATA_BITS_7 = ARM_USART_DATA_BITS_7, ///< 7 data bits.
  SL_USART_DATA_BITS_8 = ARM_USART_DATA_BITS_8, ///< 8 data bits.
} usart_databits_typedef_t;

/**
 * @brief Enumeration for USART parity selection.
 * 
 * This enumeration defines the possible parity configurations for the USART interface.
 * The user can select the appropriate parity setting based on their application requirements.
 */
typedef enum {
  SL_USART_NO_PARITY   = ARM_USART_PARITY_NONE, ///< No parity.
  SL_USART_EVEN_PARITY = ARM_USART_PARITY_EVEN, ///< Even parity.
  SL_USART_ODD_PARITY  = ARM_USART_PARITY_ODD   ///< Odd parity.
} usart_parity_typedef_t;

/**
 * @brief Enumeration for USART modem control selection.
 * 
 * This enumeration defines the possible modem control configurations for the USART interface.
 * The user can select the appropriate modem control setting based on their application requirements.
 */
typedef enum {
  SL_USART_RTS_CLEAR = ARM_USART_RTS_CLEAR, ///< Clear RTS (Request to Send).
  SL_USART_RTS_SET   = ARM_USART_RTS_SET,   ///< Set RTS (Request to Send).
  SL_USART_DTR_CLEAR = ARM_USART_DTR_CLEAR, ///< Clear DTR (Data Terminal Ready).
  SL_USART_DTR_SET   = ARM_USART_DTR_SET,   ///< Set DTR (Data Terminal Ready).
  SL_USART_MODEM_CONTROL_LAST               ///< Last modem control option for validation purposes.
} usart_modem_control_typedef_t;

/**
 * @brief Enumeration for USART stop bits selection.
 * 
 * This enumeration defines the possible stop bit configurations for the USART interface,
 * used for asynchronous operation. The user can select the appropriate stop bit setting
 * based on their application requirements.
 */
typedef enum {
  SL_USART_STOP_BITS_1   = ARM_USART_STOP_BITS_1,   ///< 1 stop bit.
  SL_USART_STOP_BITS_1_5 = ARM_USART_STOP_BITS_1_5, ///< 1.5 stop bits.
  SL_USART_STOP_BITS_2   = ARM_USART_STOP_BITS_2    ///< 2 stop bits.
} usart_stopbit_typedef_t;

/**
 * @brief Enumeration for hardware flow control selection.
 * 
 * This enumeration defines the possible hardware flow control configurations for the USART interface.
 * The user can select the appropriate flow control setting based on their application requirements.
 */
typedef enum {
  SL_USART_FLOW_CONTROL_NONE = ARM_USART_FLOW_CONTROL_NONE, ///< No hardware flow control.
  SL_USART_FLOW_CONTROL_CTS  = ARM_USART_FLOW_CONTROL_CTS,  ///< CTS signal is enabled for TX flow control.
  SL_USART_FLOW_CONTROL_RTS  = ARM_USART_FLOW_CONTROL_RTS,  ///< RTS signal is enabled for RX flow control.
  SL_USART_FLOW_CONTROL_RTS_CTS =
    ARM_USART_FLOW_CONTROL_RTS_CTS ///< CTS and RTS signals are enabled for TX and RX flow control.
} usart_hwflowcontol_typedef_t;

/**
 * @brief Enumeration for USART mode selection.
 * 
 * This enumeration defines the possible modes of operation for the USART interface.
 * It is crucial to tailor the communication channel to the specific requirements of a system.
 */
typedef enum {
  SL_USART_MODE_ASYNCHRONOUS       = ARM_USART_MODE_ASYNCHRONOUS,       ///< Asynchronous mode.
  SL_USART_MODE_SYNCHRONOUS_MASTER = ARM_USART_MODE_SYNCHRONOUS_MASTER, ///< Synchronous mode master.
  SL_USART_MODE_SYNCHRONOUS_SLAVE  = ARM_USART_MODE_SYNCHRONOUS_SLAVE,  ///< Synchronous mode slave.
  SL_USART_MODE_SINGLE_WIRE        = ARM_USART_MODE_SINGLE_WIRE, ///< UART Single-wire (half-duplex); arg = Baudrate.
  SL_USART_MODE_IRDA               = ARM_USART_MODE_IRDA         ///< UART IrDA; arg = Baudrate.
} usart_mode_typedef_t;

/**
 * @note UART provides the underlying serial communication, while IrDA enables the wireless transmission of that data using infrared light.
 */

/**
 * @brief Enumeration for USART miscellaneous control selection.
 * 
 * This enumeration defines the various miscellaneous control options available for the USART interface.
 * These controls allow for fine-tuning and managing specific USART operations.
 */
typedef enum {
  SL_USART_MISC_CONTROL_NONE,                                     ///< No miscellaneous control.
  SL_USART_SET_DEFAULT_TX_VALUE = ARM_USART_SET_DEFAULT_TX_VALUE, ///< Set default TX value.
  SL_USART_CONTROL_TX           = ARM_USART_CONTROL_TX,           ///< Control the transfer line.
  SL_USART_CONTROL_RX           = ARM_USART_CONTROL_RX,           ///< Control the receive line.
  SL_USART_CONTROL_BREAK        = ARM_USART_CONTROL_BREAK,        ///< Set USART control break.
  SL_USART_ABORT_SEND           = ARM_USART_ABORT_SEND,           ///< Abort the send operation.
  SL_USART_ABORT_RECEIVE        = ARM_USART_ABORT_RECEIVE,        ///< Abort the receive operation.
  SL_USART_ABORT_TRANSFER       = ARM_USART_ABORT_TRANSFER        ///< Abort the transfer operation.
} usart_misc_control_typedef_t;

/**
 * @brief Enumeration of USART baud rate values.
 *
 * This enumeration defines the standard baud rates used for USART (Universal Synchronous/Asynchronous Receiver/Transmitter) communication.
 * Each member represents a specific baud rate value.
 */
typedef enum {
  BAUD_RATE_300    = 300,    ///< Set baud rate to 300.
  BAUD_RATE_600    = 600,    ///< Set baud rate to 600.
  BAUD_RATE_1200   = 1200,   ///< Set baud rate to 1200.
  BAUD_RATE_2400   = 2400,   ///< Set baud rate to 2400.
  BAUD_RATE_4800   = 4800,   ///< Set baud rate to 4800.
  BAUD_RATE_9600   = 9600,   ///< Set baud rate to 9600.
  BAUD_RATE_19200  = 19200,  ///< Set baud rate to 19200.
  BAUD_RATE_38400  = 38400,  ///< Set baud rate to 38400.
  BAUD_RATE_57600  = 57600,  ///< Set baud rate to 57600.
  BAUD_RATE_115200 = 115200, ///< Set baud rate to 115200.
  BAUD_RATE_230400 = 230400, ///< Set baud rate to 230400.
  BAUD_RATE_460800 = 460800, ///< Set baud rate to 460800.
  BAUD_RATE_921600 = 921600  ///< Set baud rate to 921600.
} usart_baud_rate_t;

/*******************************************************************************
   ***********************************   STRUCTS  *****************************
   ******************************************************************************/
/**
 * @brief USART configuration control structure.
 * 
 * This structure defines the configuration parameters for the USART interface, including
 * baud rate, mode of operation, parity, stop bits, hardware flow control, data bits, 
 * miscellaneous control options, and module-specific settings.
 */
typedef struct {
  uint32_t baudrate;                ///< Baud rate for USART communication.
  usart_mode_typedef_t mode;        ///< USART mode of operation. See @ref usart_mode_typedef_t.
  usart_parity_typedef_t parity;    ///< USART parity bit configuration. See @ref usart_parity_typedef_t.
  usart_stopbit_typedef_t stopbits; ///< USART stop bits configuration. See @ref usart_stopbit_typedef_t.
  usart_hwflowcontol_typedef_t
    hwflowcontrol; ///< USART hardware flow control configuration. See @ref usart_hwflowcontol_typedef_t.
  usart_databits_typedef_t databits; ///< USART data bits configuration. See @ref usart_databits_typedef_t.
  usart_misc_control_typedef_t
    misc_control;          ///< USART miscellaneous control options. See @ref usart_misc_control_typedef_t.
  uint32_t usart_module;   ///< USART module identifier.
  boolean_t config_enable; ///< Enable configuration for USART TX and RX.
  boolean_t synch_mode;    ///< Enable synchronous mode.
} sl_si91x_usart_control_config_t;

/**
 * @brief Structure to hold the version information of the USART peripheral API.
 * 
 * This structure defines the version numbers for the USART peripheral API, including
 * the release, major, and minor version numbers. These version numbers help in identifying
 * the specific version of the API being used.
 */
typedef struct {
  uint8_t release; ///< Release version number.
  uint8_t major;   ///< Major version number.
  uint8_t minor;   ///< Minor version number.
} sl_usart_version_t;

/**
 * @brief Structure to hold GPIO pin configuration parameters.
 *
 * @details This structure defines the configuration settings for a GPIO pin, including
 *          its port, pin number, multiplexing function, and pad configuration.
 *          It is used to configure GPIO pins for various functionalities in the system.
 */
typedef struct {
  uint8_t port; ///< GPIO port. Specifies the port number for the GPIO pin.
  uint8_t pin;  ///< GPIO pin. Specifies the pin number within the specified port.
  uint8_t mux;  ///< GPIO mux. Defines the multiplexing function for the pin.
  uint8_t pad;  ///< GPIO pad. Indicates the pad configuration for the pin.
} sl_rs485_gpio_pin_config_t;

/**
 * @brief Structure to hold the port and pin configuration for USART RS485.
 *
 * This structure defines the configuration parameters for the USART RS485 pins, including the port, pin,
 * mux, and pad settings for DE, RE and RS485_EN lines.
 */
typedef struct {
  sl_rs485_gpio_pin_config_t re;       ///< USART RS485 RE pin configs.
  sl_rs485_gpio_pin_config_t de;       ///< USART RS485 DE pin configs.
  sl_rs485_gpio_pin_config_t rs485_en; ///< USART RS485 RS485 EN pin configs.
} sl_uart_rs485_pin_init_t;

/*******************************************************************************
******************************  PROTOTYPES  ************************************
******************************************************************************/
/***************************************************************************/
/**
 * @brief To initialize the USART/UART module.
 * 
 * @details This function configures the clocks for the USART/UART module and also
 * initializes the DMA for UART/USART if DMA is enabled for data transfers.
 * 
 * @param[in] usart_instance The USART instance (USART_0, UART_1, ULPUART).
 * @param[in] usart_handle Pointer to the USART/UART driver.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, UART/USART initialization done properly.
 *         - SL_STATUS_FAIL  - Function failed, UART/USART initialization failed.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 *         - SL_STATUS_NOT_AVAILABLE  - The UART/USART instance is not available (see note below).
 * 
 * @note
 *   If the desired usart_instance is dedicated for debug output logs in Debug UC, it cannot be used for other communication. In this case, initialization will return SL_STATUS_NOT_AVAILABLE.
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_usart_init(usart_peripheral_t usart_instance, sl_usart_handle_t *usart_handle);

/***************************************************************************/ /**
 * @brief Uninitialize the USART/UART module.
 * @details This function will disable the clocks for USART/UART module and also
 * Deinit the DMA for UART/USART if DMA is enabled for data transfers.
 *
 * @param[in] usart_handle Pointer to the USART/UART driver
 *
 * @return status 0 if successful, else error code as follows:
 *         - SL_STATUS_FAIL (0x0001) - Fail, UART/USART Deinitialization failed 
 *         - SL_STATUS_OK (0x0000) - Success, UART/USART Deinitialization done properly 
 * @note
 *   When the USART/UART module is used in combination with other peripherals, while deinitializing in the application, see the notes below:
 *   1. Whenever sl_si91x_usart_deinit() gets called, it will disable the clock for the peripheral. To power off the peripheral, we have to power down the
 *      power domain (PERI_EFUSE) which contains the different peripherals mentioned below:
 *      i.e., USART, UART, I2C, SSI Master, SSI Slave, Generic-SPI Master, I2S Master, I2S Slave, Micro-DMA Controller, Config Timer,
 *      Random-Number Generator, CRC Accelerator, SIO, QEI, MCPWM, and EFUSE.
 *      Use the below API to power down the particular power domain if other peripherals are not being used:
 *      sl_si91x_peri_efuse_power_down(power_down); 
 * 
 *   2. A few peripherals (ULP Peripherals, UULP Peripherals, GPDMA, and SDIO-SPI) have separate domains; those can be powered down independently. For additional details, see the Power architecture section in the Hardware Reference Manual.
 *      Here, ULP_UART has a separate power domain ULPSS_PWRGATE_ULP_UART, which can be powered down independently. See the rsi_power_save.h file for all power gate definitions.
 * 
 ******************************************************************************/
sl_status_t sl_si91x_usart_deinit(sl_usart_handle_t usart_handle);

/***************************************************************************/
/**
 * @brief To register the user callback function for USART events.
 * 
 * @details This API registers a user-defined callback function for USART events.
 * If another callback is registered without unregistering the previous callback, 
 * it returns an error code. Therefore, it is mandatory to unregister the callback 
 * before registering another callback.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_usart_set_configuration must be called prior.
 * 
 * @param[in] callback_event Pointer to the function which needs to be called at the time of interrupt.
 * 
 * @return sl_status_t Status code indicating the result:
*         - SL_STATUS_OK  - Success.
*         - SL_STATUS_BUSY  - Driver is busy.
*         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_si91x_usart_multiple_instance_register_event_callback](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/usart#sl-si91x-usart-multiple-instance-register-event-callback) API. This is retained for backward compatibility.
 ******************************************************************************/
sl_status_t sl_si91x_usart_register_event_callback(sl_usart_signal_event_t callback_event)
  SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/
/**
 * @brief To unregister the user callback function for USART events.
 * 
 * @details This API unregisters a user-defined callback function for USART events.
 * It is mandatory to unregister the current callback before registering a new one.
 ******************************************************************************/
void sl_si91x_usart_unregister_event_callback(void);

/***************************************************************************/
/**
 * @brief To register the user callback function for multiple USART instances.
 * 
 * @details This function registers a user-defined callback function for multiple USART instances 
 * (e.g., USART_0, UART_1, ULP_UART). This API should be used to handle different USART instances.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_usart_set_configuration must be called prior.
 * 
 * @param[in] usart_instance The USART instance.
 * @param[in] callback_event Pointer to the function which needs to be called at the time of interrupt.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_BUSY  - Driver is busy.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_usart_multiple_instance_register_event_callback(usart_peripheral_t usart_instance,
                                                                     sl_usart_signal_event_t callback_event);

/***************************************************************************/
/**
 * @brief To unregister the user callback function for multiple USART instances.
 * 
 * @details This function unregisters a user-defined callback function for multiple USART instances 
 * (e.g., USART_0, UART_1, ULP_UART). This API should be used to handle different USART instances.
 * 
 * @param[in] usart_instance This is the USART instance.
 ******************************************************************************/
void sl_si91x_usart_multiple_instance_unregister_event_callback(usart_peripheral_t usart_instance);

/***************************************************************************/
/**
 * @brief To send the USART data when USART/UART is configured.
 * 
 * @details If DMA mode is set, this function will configure the DMA channel and enable the DMA channel,
 * then transfer the data pointed to it. Otherwise, it fills the data to the transfer FIFO and transfers the data.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_usart_init();
 *      - \ref sl_si91x_usart_set_configuration();
 * 
 * @param[in] usart_handle Pointer to the USART/UART driver.
 * @param[in] data Pointer to the variable which contains transfer data.
 * @param[in] data_length Length of the data to be sent.
 * 
 * @return sl_status_t Status code indicating the result:
*         - SL_STATUS_OK  - Success, data transfer completed.
*         - SL_STATUS_FAIL  - Function failed, data transfer failed.
*         - SL_STATUS_BUSY  - Busy, data transfer is already in progress.
*         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_usart_send_data(sl_usart_handle_t usart_handle, const void *data, uint32_t data_length);

/***************************************************************************/
/**
 * @brief To send data in async mode when USART/UART is configured.
 * 
 * @details This function returns immediately (non-blocking) and data transfer happens asynchronously.
 * Once the data transfer completes, the registered user callback gets invoked.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_usart_init();
 *      - \ref sl_si91x_usart_set_configuration();
 * 
 * @param[in] usart_handle Pointer to the USART/UART driver.
 * @param[in] data Pointer to the variable which contains the data to be transferred.
 * @param[in] data_length Length of the data to be transferred.
 * 
 * @return sl_status_t Status code indicating the result:
*         - SL_STATUS_OK  - Success, data transfer completed.
*         - SL_STATUS_FAIL  - Function failed, data transfer failed.
*         - SL_STATUS_BUSY  - Busy, data transfer is already in progress.
*         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_usart_async_send_data(sl_usart_handle_t usart_handle, const void *data, uint32_t data_length);
/***************************************************************************/
/**
 * @brief Internal function to send data in blocking mode byte-by-byte.
 *
 * @details This internal function sends data byte-by-byte via USART/UART and waits (blocks)
 * until each byte transmission is complete before sending the next byte. This ensures
 * true byte-by-byte transmission with hardware verification for each byte.
 * The function accesses UART hardware registers directly from the handle and works
 * with all UART instances: USART0, UART1, and ULP_UART.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_usart_init() must be called first
 *      - \ref sl_si91x_usart_set_configuration() must be called to configure the UART
 *
 * @param[in] usart_handle Pointer to the USART/UART driver (supports USART0, UART1, ULP_UART).
 * @param[in] data Pointer to the data buffer which contains the data to be transferred.
 * @param[in] data_length Length of the data to be transferred.
 *
 * @return sl_status_t Status code indicating the result:
*         - SL_STATUS_OK  - Success, data transfer completed.
*         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
*         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter.
 *
 * @note This is an internal blocking function for byte-by-byte transmission.
 *       - Supports all UART instances: USART_0, UART_1, and ULPUART
 *       - Each byte is transmitted and verified before sending the next byte
 *       - Do not call from interrupt context
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sli_si91x_usart_send_data_blocking(sl_usart_handle_t usart_handle, const void *data, uint32_t data_length);
/***************************************************************************/
/**
 * @brief To receive data when USART/UART is configured.
 * 
 * @details If DMA mode is set, this function configures the DMA channel, enables the DMA channel,
 * and receives data via DMA. If DMA mode is not set, it receives the data from FIFO.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_usart_init
 *      - \ref sl_si91x_usart_set_configuration
 * 
 * @param[in] usart_handle Pointer to the USART/UART driver.
 * @param[in] data Pointer to the variable which will store the received data.
 * @param[in] data_length Length of the data to be received.
 * 
 * @return sl_status_t Status code indicating the result:
*         - SL_STATUS_OK  - Success, data received successfully.
*         - SL_STATUS_FAIL  - Function failed, data transfer failed.
*         - SL_STATUS_BUSY  - Busy, data transfer is already in progress.
*         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_usart_receive_data(sl_usart_handle_t usart_handle, void *data, uint32_t data_length);

/***************************************************************************/
/**
 * @brief To receive data in async mode when USART/UART is configured.
 * 
 * @details This function returns immediately (non-blocking) and data reception happens asynchronously.
 * Once the data reception completes, the registered user callback gets invoked.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_usart_init();
 *      - \ref sl_si91x_usart_set_configuration();
 * 
 * @param[in] usart_handle Pointer to the USART/UART driver.
 * @param[in] data Pointer to the variable which will store the received data.
 * @param[in] data_length Length of the data to be received.
 * 
 * @return sl_status_t Status code indicating the result:
*         - SL_STATUS_OK  - Success, data received successfully.
*         - SL_STATUS_FAIL  - Function failed, data transfer failed.
*         - SL_STATUS_BUSY  - Busy, data transfer is already in progress.
*         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_usart_async_receive_data(sl_usart_handle_t usart_handle, void *data, uint32_t data_length);

/***************************************************************************/
/**
 * @brief To send and receive data when USART in Synchronous mode is enabled.
 * 
 * @details This function sends and receives data to and from the USART transmitter and
 * receiver in synchronous mode of operation. It configures the DMA channel and enables the DMA channel,
 * if DMA mode is set, and transfers the data pointed to it; otherwise, it transfers the data from FIFO.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_usart_init
 *      - \ref sl_si91x_usart_set_configuration
 * 
 * @param[in] usart_handle Pointer to the USART/UART driver.
 * @param[in] data_out Pointer to the variable which will store the data to be transferred.
 * @param[in] data_in Pointer to the variable which will store the received data.
 * @param[in] data_length Length of the data to be received.
 * 
 * @return sl_status_t Status code indicating the result:
*         - SL_STATUS_OK  - Success, data transfer completed.
*         - SL_STATUS_FAIL  - Function failed, data transfer failed.
*         - SL_STATUS_BUSY  - Busy, data transfer is already in progress.
*         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note This function is to be used in USART mode only (i.e., synchronous mode). In asynchronous
 * mode, use \ref sl_si91x_usart_receive_data() and \ref sl_si91x_usart_send_data().
 ******************************************************************************/
sl_status_t sl_si91x_usart_transfer_data(sl_usart_handle_t usart_handle,
                                         const void *data_out,
                                         void *data_in,
                                         uint32_t data_length);

/***************************************************************************/
/**
 * @brief To get the TX data count of USART/UART.
 * 
 * @details This function returns the number of bytes transferred by the USART/UART.
 * It can be used to check the progress of data transmission.
 * 
 * @param[in] usart_handle This is the pointer to the USART/UART driver.
 * 
 * @return uint32_t The number of bytes transferred.
 ******************************************************************************/
uint32_t sl_si91x_usart_get_tx_data_count(sl_usart_handle_t usart_handle);

/***************************************************************************/
/**
 * @brief To get the RX data count of USART/UART.
 * 
 * @details This function returns the number of bytes received by the USART/UART.
 * It can be used to check the progress of data reception.
 * 
 * @param[in] usart_handle This is the pointer to the USART/UART driver.
 * 
 * @return uint32_t The number of bytes received.
 ******************************************************************************/
uint32_t sl_si91x_usart_get_rx_data_count(sl_usart_handle_t usart_handle);

/***************************************************************************/
/**
 * @brief To control and configure the USART/UART module in different configurations.
 * 
 * @details This function configures the USART in different configurations such as USART mode,
 * data bits, parity, stop bits, flow control, and baud rate.
 * 
 * @param[in] usart_handle Pointer to the USART/UART driver.
 * @param[in] control_configuration Pointer to the USART configurations.
 * 
 * @note control_configuration - This parameter is used to pass the respective peripheral configuration when  
 * USART_UC, UART_UC, ULP_UART_UC macros are disabled for a particular instance. When these macros are enabled,
 * configurations are taken directly from the UC and this parameter is not considered.
 * 
 * @return sl_status_t Status code indicating the result:
*         - SL_STATUS_OK  - Success, UART/USART initialization done properly.
*         - SL_STATUS_BUSY  - Busy, data transfer is already in progress.
*         - SL_STATUS_NOT_SUPPORTED  - Feature not supported.
*         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
*         - SL_STATUS_INVALID_MODE  - USART invalid mode of operation.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_usart_set_configuration(sl_usart_handle_t usart_handle,
                                             sl_si91x_usart_control_config_t *control_configuration);

/***************************************************************************/
/**
 * @brief To configure the USART interface with non-UC configurations.
 * 
 * @details This internal function configures the USART interface with different configurations, such as 
 * USART mode, data bits, parity, stop bits, flow control, and baud rate. This API will not pick the 
 * configurations from USART UC.
 * 
 * @param[in] usart_handle Pointer to the USART/UART driver.
 * @param[in] control_configuration Pointer to the USART configurations.
 * 
 * @return sl_status_t Status code indicating the result:
*         - SL_STATUS_OK  - Success, UART/USART initialization done properly.
*         - SL_STATUS_BUSY  - Busy, data transfer is already in progress.
*         - SL_STATUS_NOT_SUPPORTED  - Feature not supported.
*         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
*         - SL_STATUS_INVALID_MODE  - USART invalid mode of operation.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sli_si91x_usart_set_non_uc_configuration(sl_usart_handle_t usart_handle,
                                                     sl_si91x_usart_control_config_t *control_configuration);

/***************************************************************************/
/**
 * @brief To get the USART/UART status.
 * 
 * @details This function returns the USART/UART transfer and receive status.
 * It provides information about the current state of the USART/UART, including:
 * - TX busy
 * - RX busy
 * - TX underflow
 * - RX overflow
 * - RX break
 * - RX framing error
 * - RX parity error
 * 
 * @param[in] usart_handle Pointer to the USART/UART driver.
 * 
 * @return sl_usart_status_t The status of the USART/UART.
 ******************************************************************************/
sl_usart_status_t sl_si91x_usart_get_status(sl_usart_handle_t usart_handle);

/***************************************************************************/
/**
 * @brief To set the USART modem control line.
 * 
 * @details This function sets the USART modem control line based on the provided control parameter.
 * 
 * @param[in] usart_handle Pointer to the USART/UART driver.
 * @param[in] control The USART modem control.
 * 
 * @return sl_status_t Status code indicating the result:
*         - SL_STATUS_OK  - Success, UART/USART initialization done properly.
*         - SL_STATUS_NOT_SUPPORTED  - Feature not supported.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_usart_set_modem_control(sl_usart_handle_t usart_handle, sl_usart_modem_control_t control);

/***************************************************************************/
/**
 * @brief To get the USART modem control status.
 * 
 * @details This function returns the status of the USART modem control lines, indicating whether the states are active or not.
 * 
 * @param[in] usart_handle Pointer to the USART/UART driver.
 * 
 * @return sl_usart_modem_status_t The status of the USART modem control lines.
 ******************************************************************************/
sl_usart_modem_status_t sl_si91x_usart_get_modem_status(sl_usart_handle_t usart_handle);

/***************************************************************************/
/**
 * @brief To get the USART version.
 * 
 * @details This function returns the version information of the USART driver.
 * It can be used to check the driver version for compatibility and debugging purposes.
 * 
 * @return sl_usart_version_t The version information of the USART driver.
 ******************************************************************************/
sl_usart_version_t sl_si91x_usart_get_version(void);

/***************************************************************************/
/**
 * @brief To get the USART configurations set in the module, such as baud rate, parity bit, stop bits, etc.
 * 
 * @details This function retrieves the current configurations of the specified USART module.
 * 
 * @param[in] usart_module USART peripheral type:
 *   -  0 - USART0
 *   -  1 - UART1
 *   -  2 - ULP_UART
 * @param[in] usart_config Pointer to the USART configurations structure.
 * 
 * @return sl_status_t Status code indicating the result:
*         - SL_STATUS_OK  - Success, UART/USART configurations retrieved successfully.
*         - SL_STATUS_NULL_POINTER  - Invalid NULL pointer received as argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_usart_get_configurations(uint8_t usart_module, sl_si91x_usart_control_config_t *usart_config);
#if (SLI_SI91X_MCU_RS485_MODE == 1)
/***************************************************************************/
/**
 * @brief To initialize the RS485 and configure the RS485 gpio pins.
 * 
 * @details This API configures the GPIO pins i.e DE, RE and EN pins and enables the RS485 module
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_usart_init and
 *      - \ref sl_si91x_usart_set_configuration must be called prior.
 * 
 * @param[in] uart_instance Uart instance
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Passed Invalid uart instance.
 *         - SL_STATUS_NOT_SUPPORTED  - UART INstance not supported.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_init(usart_peripheral_t uart_instance);

/***************************************************************************/
/**
 * @brief To configure the RS485 configurations.
 * 
 * @details This API configures the RS485 configurations such as transfer mode, Driver enable Assertion, Driver enable De-assertion time, DE pol, RE pol 
 * and Multidrop configurations such as data bit 9, addr match, send addr etc. 
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_usart_rs485_init must be called prior.
 * 
 * @param[in] uart_instance Uart instance
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Passed Invalid uart instance.
 *         - SL_STATUS_NOT_SUPPORTED  - UART INstance not supported.
  * @note 
 * To maintain backward compatibility, SL_UART1_RS485_TRANSMIT_MODE is initially defined as 0 in configuration file. However, the actual transmit mode will be determined and set internally according to the selected transfer mode.
 *  
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_set_configuration(usart_peripheral_t uart_instance);

/***************************************************************************/
/**
 * @brief Get the RS485 configurations.
 * 
 * @details This API gets the RS485 configurations such as transfer mode, Driver enable Assertion, Driver enable De-assertion time, DE pol, RE pol 
 * and Multidrop configurations such as data bit 9, addr match, send addr etc. 
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_usart_rs485_init must be called prior.
 * 
 * @param[in] uart_rs485_configs Pointer to \ref usart_rs485_config_t
 * 
 * @return None
 ******************************************************************************/
void sl_si91x_get_uart_rs485_configure(usart_rs485_config_t *uart_rs485_configs);
/***************************************************************************/
/**
 * @brief To Enable the Driver enable signal.
 * 
 * @details This API enable or disable the DE(Driver Enable) signal
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_usart_rs485_init and
 *      - \ref sl_si91x_usart_rs485_configure must be called prior.
 * 
 * @param[in] uart_instance Uart instance
 * @param[in] enable Enable/disable DE signal
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Passed Invalid uart instance.
 *         - SL_STATUS_NOT_SUPPORTED  - UART INstance not supported.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_de_enable(usart_peripheral_t uart_instance, bool enable);

/***************************************************************************/
/**
 * @brief To enable the Receiver Enable signal.
 * 
 * @details This API enable or disable the RE (Receiver Enable) signal
 * 
 * @param[in] uart_instance Uart instance
 * @param[in] enable Enable/disable RE signal
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Passed Invalid uart instance.
 *         - SL_STATUS_NOT_SUPPORTED  - UART INstance not supported.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_re_enable(usart_peripheral_t uart_instance, bool enable);

/***************************************************************************/
/**
 * @brief To transfer the address in transmit mode.
 * 
 * @details This API used to transfer the address during transmit mode
 * This is an address matching api during transmit mode. When 9-bit character 
 * is set to 1 and remaining 8-bit address will be sent using this API. 
 * 
 * @param[in] uart_instance Uart instance
 * @param[in] tx_address Pointer to the transmit address
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Passed Invalid uart instance.
 *         - SL_STATUS_NOT_SUPPORTED  - UART INstance not supported.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_transfer_hardware_address(usart_peripheral_t uart_instance, uint8_t *tx_address);

/***************************************************************************/
/**
 * @brief To wait till address received in case of receive mode.
 * 
 * @details This API wait till the address received in receive mode
 * 
 * @param[in] uart_instance Uart instance
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Passed Invalid uart instance.
 *         - SL_STATUS_NOT_SUPPORTED  - UART INstance not supported.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_address_received(usart_peripheral_t uart_instance);

/***************************************************************************/
/**
 * @brief To set the receive address in receive mode.
 * 
 * @details This API used to set the address to the module in receive mode
 * 
 * @param[in] uart_instance Uart instance
 * @param[in] rx_addr rx address to set to the module
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Passed Invalid uart instance.
 *         - SL_STATUS_NOT_SUPPORTED  - UART INstance not supported.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_rx_hardware_address_set(usart_peripheral_t uart_instance, uint8_t *rx_addr);

/***************************************************************************/
/**
 * @brief To halt or resume the transfer
 * 
 * @details This api is used when application is required to fill the data bytes in the TxFIFO before sending the address on 
 * the uart line.Then this api need to call with enable bit set to 1 so that uart does not start sending out the data in the 
 * TxFIFO as data byte, Once the TxFIFO is filled, then call the rsi_rs485_transfer_address() api  and call this api with 
 * enable bit set as 0.
 * 
 * @param[in] uart_instance Uart instance
 * @param[in] enable resume/halt the data transfer
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Passed Invalid uart instance.
 *         - SL_STATUS_NOT_SUPPORTED  - UART INstance not supported.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_resume_tx(usart_peripheral_t uart_instance, bool enable);

/***************************************************************************/
/**
 * @brief Enable/Disable send address bit while transferring address
 *
 * @details This api is used to Enable/Disable send address bit while transferring address
 *
 * @param[in] uart_instance Uart instance
 * @param[in] send_addr_ctrl  enable/disable send address control bit
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Passed Invalid uart instance.
 *         - SL_STATUS_NOT_SUPPORTED  - UART INstance not supported.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_control_send_address(usart_peripheral_t uart_instance, bool send_addr_ctrl);

/***************************************************************************/
/**
* @brief Enable/Disable address match bit while receiving.
*
* @details This api is used to Enable/Disable address match bit while receiving.
*
* @param[in] uart_instance Uart instance
* @param[in] addr_match_ctrl  enable/disable address match control bit
* @return sl_status_t Status code indicating the result:
*         - SL_STATUS_OK  - Success.
*         - SL_STATUS_INVALID_PARAMETER  - Passed Invalid uart instance.
*         - SL_STATUS_NOT_SUPPORTED  - UART INstance not supported.
*
* For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
******************************************************************************/
sl_status_t sl_si91x_uart_rs485_control_address_match(usart_peripheral_t uart_instance, bool addr_match_ctrl);

/***************************************************************************/
/**
 * @brief Sets the Driver Enable assertion time for RS-485.
 *
 * @details This API configures the duration for which the Driver Enable (DE) signal
 * is asserted before the start of data transmission for the specified UART instance.
 *
 * @pre Pre-condition:
 * - \ref sl_si91x_usart_init and \ref sl_si91x_uart_rs485_init must be called prior.
 *
 * @param[in] uart_instance Uart instance
 * @param[in] assert_time The Driver Enable assertion time value (in serial clock periods).
 * @return sl_status_t Status code indicating the result:
 * - SL_STATUS_OK              - Success.
 * - SL_STATUS_INVALID_PARAMETER - Passed Invalid uart instance.
 * - SL_STATUS_NOT_SUPPORTED   - UART Instance not supported.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_set_de_assert_time(usart_peripheral_t uart_instance, uint8_t assert_time);

/***************************************************************************/
/**
 * @brief Sets the Driver Enable de-assertion time for RS-485.
 *
 * @details This API configures the duration for which the Driver Enable (DE) signal
 * remains asserted after the completion of data transmission for the specified
 * UART instance.
 *
 * @pre Pre-condition:
 * - \ref sl_si91x_usart_rs485_init must be called prior.
 *
 * @param[in] uart_instance Uart instance
 * @param[in] deassert_time The Driver Enable de-assertion time value (in serial clock periods).
 * @return sl_status_t Status code indicating the result:
 * - SL_STATUS_OK              - Success.
 * - SL_STATUS_INVALID_PARAMETER - Passed Invalid uart instance.
 * - SL_STATUS_NOT_SUPPORTED   - UART Instance not supported.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_set_de_deassert_time(usart_peripheral_t uart_instance, uint8_t deassert_time);

/***************************************************************************/
/**
 * @brief Sets the Driver Enable to Receiver Enable turnaround time for RS-485.
 *
 * @details This API configures the delay between de-asserting the Driver Enable (DE)
 * signal and asserting the Receiver Enable (RE) signal when switching from
 * transmit to receive mode for the specified UART instance. This turnaround
 * time is crucial to avoid bus contention.
 *
 * @pre Pre-condition:
 * - \ref sl_si91x_usart_rs485_init must be called prior.
 *
 * @param[in] uart_instance Uart instance
 * @param[in] de_re_tat The DE to RE turnaround time value (in serial clock periods).
 * @return sl_status_t Status code indicating the result:
 * - SL_STATUS_OK              - Success.
 * - SL_STATUS_INVALID_PARAMETER - Passed Invalid uart instance.
 * - SL_STATUS_NOT_SUPPORTED   - UART Instance not supported.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_set_de_re_turnaround_time(usart_peripheral_t uart_instance, uint16_t de_re_tat);

/***************************************************************************/
/**
 * @brief Sets the Receiver Enable to Driver Enable turnaround time for RS-485.
 *
 * @details This API configures the delay between de-asserting the Receiver Enable (RE)
 * signal and asserting the Driver Enable (DE) signal when switching from
 * receive to transmit mode for the specified UART instance. This turnaround
 * time helps prevent bus conflicts.
 *
 * @pre Pre-condition:
 * - \ref sl_si91x_usart_rs485_init must be called prior.
 *
 * @param[in] uart_instance Uart instance
 * @param[in] re_de_tat The RE to DE turnaround time value (in serial clock periods).
 * @return sl_status_t Status code indicating the result:
 * - SL_STATUS_OK              - Success.
 * - SL_STATUS_INVALID_PARAMETER - Passed Invalid uart instance.
 * - SL_STATUS_NOT_SUPPORTED   - UART Instance not supported.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_set_re_de_turnaround_time(usart_peripheral_t uart_instance, uint16_t re_de_tat);

/***************************************************************************/
/**
 * @brief Sets the Driver Enable polarity for RS-485 for the specified UART instance.
 *
 * @details This API configures the active level of the Driver Enable (DE) signal.
 *
 * @pre Pre-condition:
 * - \ref sl_si91x_usart_rs485_init must be called prior.
 *
 * @param[in] uart_instance Uart instance
 * @param[in] de_polarity The Driver Enable polarity:
 * - `0`: DE signal is active low.
 * - `1`: DE signal is active high.
 * @return sl_status_t Status code indicating the result:
 * - SL_STATUS_OK              - Success.
 * - SL_STATUS_INVALID_PARAMETER - Passed Invalid uart instance.
 * - SL_STATUS_NOT_SUPPORTED   - UART Instance not supported.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_set_de_polarity(usart_peripheral_t uart_instance, uint8_t de_polarity);

/***************************************************************************/
/**
 * @brief Sets the Receiver Enable polarity for RS-485 for the specified UART instance.
 *
 * @details This API configures the active level of the Receiver Enable (RE) signal.
 *
 * @pre Pre-condition:
 * - \ref sl_si91x_usart_rs485_init must be called prior.
 *
 * @param[in] uart_instance Uart instance
 * @param[in] re_polarity The Receiver Enable polarity:
 * - `0`: RE signal is active low.
 * - `1`: RE signal is active high.
 * @return sl_status_t Status code indicating the result:
 * - SL_STATUS_OK              - Success.
 * - SL_STATUS_INVALID_PARAMETER - Passed Invalid uart instance.
 * - SL_STATUS_NOT_SUPPORTED   - UART Instance not supported.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_set_re_polarity(usart_peripheral_t uart_instance, uint8_t re_polarity);

/***************************************************************************/
/**
 * @brief To deinit the RS485 module
 * 
 * @details This api is used to un-initialize RS485 module
 * 
 * @param[in] uart_instance Uart instance
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Passed Invalid uart instance.
 *         - SL_STATUS_NOT_SUPPORTED  - UART INstance not supported.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_deinit(usart_peripheral_t uart_instance);
#endif
/// @} end group USART ********************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/**
* @addtogroup USART USART
* @{
*
* @details
* 
* @section USART_Intro Introduction
*
* USART is used in communication through wired medium in both synchronous and asynchronous fashion.
*
* In synchronous mode, both full duplex and half duplex (single wire) modes are supported.
*
* @image html uart_frame.png "UART Frame Format"
* 
* A USART frame consists of:
*
* - **Start bit**: Indicates the beginning of a data frame.
*
* - **Data bits**: The actual data to be transmitted (typically 7 or 8 bits).
*
* - **Parity bit**: Optional bit for error checking.
*
* - **Stop bit**: Indicates the end of a data frame.
*
* USART is widely used in embedded systems due to its simplicity and versatility. Some common applications include:
*
* - **Sensor data acquisition**: Reading data from sensors like temperature, humidity, pressure, etc.
* - **Actuator control**: Sending commands to motors, servos, and other actuators.
* - **Human-machine interface (HMI)**: Communicating with keyboards, displays, and other user interface devices.
* - **Debug and logging**: Sending diagnostic information to a computer for analysis.
* - **Communication with other devices**: Interfacing with modules like GPS, Bluetooth, Wi-Fi, etc.
*
* @section USART_Config Configuration
*
* USART can be configured using several parameters, including:
* - **Transmit Mode:**
*   1. Synchronous mode (@ref sl_si91x_usart_transfer_data).
*   2. Asynchronous mode (@ref sl_si91x_usart_async_send_data).
*
* - **Receive Mode:**
*   1. Synchronous mode (@ref sl_si91x_usart_transfer_data).
*   2. Asynchronous mode (@ref sl_si91x_usart_async_receive_data).
*   
* @note 
*
* In synchronous mode, USART signals are given out on the interface, and interface signals are given to the USART module, blocking inputs to UART.
*
* In asynchronous mode, UART signals are given out on the interface, and interface signals are given to the UART module, blocking inputs to USART.
*
* For more information on configuring available parameters, see the respective peripheral example readme document.
*
* @section USART_Usage Usage
* USART has to be initialized and configured first by calling @ref sl_si91x_usart_init. The DMA (Direct Memory Access) controller in the USART driver
* will enable efficient data transfer, thus reducing CPU overhead. Initialization of the DMA for UART/USART will also be done with the same USART init function,
* if DMA is enabled for data transfers. The other common functions include the following:
*
* You can use this in two modes:
*
* 1. Full Duplex Mode:
*
*    - Set synchronous mode.
*
*    - Write data to TX FIFO.
*
*    - Write count to RX_CNT.
*
*    - Set auto_flow_ctrl if controlled transfers on the interface.
*
* 2. Half Duplex Mode:
*
*    1. Transmit Mode:
*
*       - Reset full_duplex_mode.
*
*       - Set tx_mode.
*
*       - Reset rx_mode.
*
*       - Write data to TX_FIFO by polling the FIFO full/afull status.
*
*    2. Receive Mode:
*
*       - Reset full_duplex_mode.
*
*       - Set rx_mode.
*
*       - Reset tx_mode.
*
*       - Write RX_CNT with the number of bytes to be received.
*
*       - Read data from RX_FIFO by polling the RX FIFO empty/aempty status.
*
* The other common functions include the following:
*
* 1. @ref sl_si91x_usart_init
*
* 2. @ref sl_si91x_usart_set_configuration
*
* 3. @ref sl_si91x_usart_register_event_callback (deprecated)
*
* 4. @ref sl_si91x_usart_multiple_instance_register_event_callback
*
* 5. @ref sl_si91x_usart_async_send_data
*
* 6. @ref sl_si91x_usart_async_receive_data
*
* 7. @ref sl_si91x_usart_transfer_data
*
* 8. @ref sl_si91x_usart_get_status
*
* 9. @ref sl_si91x_usart_get_tx_data_count
*
* 10. @ref sl_si91x_usart_get_rx_data_count
*
* 11. @ref sl_si91x_usart_set_modem_control
*
* 12. @ref sl_si91x_usart_deinit
*
*
*/
/** @} (end addtogroup USART) */

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_USART_H */
