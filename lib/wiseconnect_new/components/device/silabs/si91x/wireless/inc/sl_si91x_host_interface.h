/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#pragma once

#include "sl_status.h"
#include "sl_wifi_types.h"
#include "sl_wifi_host_interface.h"
#include "sl_si91x_types.h"
#include "sl_wifi_device.h"
#include <stdbool.h>
#include <stdint.h>

/** \addtogroup EXTERNAL_HOST_INTERFACE_FUNCTIONS
 * \ingroup EXTERNAL_HOST_INTERFACE
 * @{ */

/***************************************************************************/ /**
 * @typedef sl_si91x_host_rx_irq_handler
 *
 * @brief
 *   Function pointer type for the host RX interrupt handler.
 *
 * @details
 *   This function pointer is used to define a callback for handling host receive (RX) interrupts.
 *
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *
 *****************************************************************************/
typedef sl_status_t (*sl_si91x_host_rx_irq_handler)(void);

/***************************************************************************/ /**
 * @typedef sl_si91x_host_rx_done_handler
 * 
 * @brief
 *   Function pointer type for the host receive done handler.
 *
 * @details
 *   This function pointer is used to define a callback that is invoked when the host interface completes a receive operation.
 * 
 *****************************************************************************/
typedef void (*sl_si91x_host_rx_done_handler)(void);

/// Configure and initialize the SI91x host interface.
typedef struct {
  sl_si91x_host_rx_irq_handler rx_irq;   ///< Interrupt handler for receiving data
  sl_si91x_host_rx_done_handler rx_done; ///< Callback handler for completed data reception.
  uint8_t boot_option;                   ///< Boot option configuration for the host interface
} sl_si91x_host_init_configuration_t;

/***************************************************************************/ /**
 * @brief 
 *   Holds the SI91x host in a reset state.
 *
 * @details
 *   This function drives the reset pin of the SI91x host to a low state, effectively holding the device in a reset condition. 
 *
 *****************************************************************************/
void sl_si91x_host_hold_in_reset(void);

/***************************************************************************/ /**
 * @brief 
 *   Releases the SI91x host from reset state.
 *
 * @details
 *   This function sets the specified GPIO pin high to release the SI91x host from its reset state. 
 *
 *****************************************************************************/
void sl_si91x_host_release_from_reset(void);

/***************************************************************************/ /**
 * @brief
 *   Initializes the host interface for the Si91x device.
 *
 * @details
 *   This function configures the necessary hardware and software components required for communication with the Si91x device over the SPI interface.
 *
 *   It sets up GPIO pins, SPI settings, DMA channels, and interrupt handlers.
 *
 * @param[in] config 
 *   Pointer to the host initialization configuration structure @ref sl_si91x_host_init_configuration_t (Currently unused).
 *
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *
 * @note 
 *   This function must be called before any communication with the Si91x device.
 *
 *****************************************************************************/
sl_status_t sl_si91x_host_init(const sl_si91x_host_init_configuration_t *config);

/***************************************************************************/ /**
 * @brief
 *   Deinitializes the SI91x host by releasing resources.
 *
 * @details
 *   This function is responsible for cleaning up resources used by the SI91x host.
 *
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * 
 *****************************************************************************/
sl_status_t sl_si91x_host_deinit(void);

/***************************************************************************/ /**
 * @brief
 *   Enables the high-speed bus for the SI91x host interface.
 *
 * @details
 *   This function configures and activates the high-speed bus interface for communication with the SI91x device. 
 *
 *****************************************************************************/
void sl_si91x_host_enable_high_speed_bus();

/***************************************************************************/ /**
 * @brief
 *   Processes a received data frame from the Wi-Fi interface
 * 
 * @details
 *   This function retrieves the raw data frame from the provided buffer, extracts the packet information.
 *
 *   The received data packet can then be forwarded to an external network stack for further processing.
 * 
 * @param[in] interface
 *   The Wi-Fi interface on which the data frame is received. Interfaces are defined in [sl_wifi_interface_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-constants#sl-wifi-interface-t).
 *
 * @param[in] buffer
 *   A pointer to an [sl_wifi_buffer_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-buffer-t) structure containing the data frame to be processed.
 *
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * 
 * @note
 *   This is a weak implementation, and by default, an implementation is provided by the SDK.
 * 
 *   Users can choose to overwrite it with their own custom implementation if needed.
 *
 ******************************************************************************/
sl_status_t sl_si91x_host_process_data_frame(sl_wifi_interface_t interface, sl_wifi_buffer_t *buffer);

/***************************************************************************/ /**
 * @brief
 *   Enables the bus interrupt for the SI91x host interface.
 *
 * @details
 *   This function is used to enable the bus interrupt mechanism, allowing the host interface to handle interrupt-driven communication with the SI91x device.
 *
 *   It ensures that the host is notified of events or data availability through the configured interrupt line.
 * 
 ******************************************************************************/
void sl_si91x_host_enable_bus_interrupt(void);

/***************************************************************************/ /**
 * @brief
 *   Disables the bus interrupt for the host interface.
 *
 * @details
 *   This function is used to disable the bus interrupt mechanism in the host interface, preventing the host from receiving further interrupt signals from the device.
 * 
 ******************************************************************************/
void sl_si91x_host_disable_bus_interrupt(void);

/***************************************************************************/ /**
 * @brief
 *   Sets the sleep indicator for the SI91x host.
 *
 * @details
 *   This function sets the sleep confirmation pin to indicate that the host is ready to enter a low-power sleep state.
 * 
 ******************************************************************************/
void sl_si91x_host_set_sleep_indicator(void);

/***************************************************************************/ /**
 * @brief
 *   Clears the sleep indicator signal for the host.
 *
 * @details
 *   This function clears the sleep indicator pin, signaling that the host is no longer in a sleep state. 
 * 
 ******************************************************************************/
void sl_si91x_host_clear_sleep_indicator(void);

/***************************************************************************/ /**
 * @brief
 *   Retrieves the current state of the wake indicator pin.
 *
 * @details
 *   This function reads the state of the wake indicator pin using the HAL GPIO interface.
 *
 * @return uint32_t
 *   The state of the wake indicator pin.
 * 
 ******************************************************************************/
uint32_t sl_si91x_host_get_wake_indicator(void);

/***************************************************************************/ /**
 * @brief
 *   Perform an SPI transfer.
 *
 * @details
 *   This function handles SPI data transfer between a transmit buffer and a receive buffer. 
 *
 * @param[in] tx_buffer 
 *   Pointer to the transmit buffer.
 * 
 * @param[out] rx_buffer
 *   Pointer to the receive buffer.
 * 
 * @param[in] buffer_length
 *   Length of the data to be transferred in bytes.
 *
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * 
 ******************************************************************************/
sl_status_t sl_si91x_host_spi_transfer(const void *tx_buffer, void *rx_buffer, uint16_t buffer_length);

/**
 * @brief
 * Asserts the SPI bus chip select
 * 
 * @note 
 * This is a weak implementation, and by default, an implementation is provided by the SDK.
 * Users can choose to overwrite it with their own custom implementation if needed.
 */
void sl_si91x_host_spi_cs_assert(void);

/**
 * @brief
 * De-asserts the SPI bus chip select
 * 
 * @note 
 * This is a weak implementation provided by default by the SDK.
 * Users can choose to overwrite it with their own custom implementation if needed.
 */
void sl_si91x_host_spi_cs_deassert(void);

/***************************************************************************/ /**
 * @brief
 *   Transfers data over the UART interface.
 *
 * @details
 *   This function performs UART data transfer for both transmission and reception.
 *
 *   It supports both direct byte-by-byte transfer and DMA-based transfer depending on the buffer length and initialization status of the NCP.
 *
 * @param[in]  tx_buffer
 *   Pointer to the data buffer to be transmitted.
 *
 * @param[out] rx_buffer
 *   Pointer to the buffer where received data will be stored.
 *
 * @param[in]  buffer_length
 *   Length of the data buffer to be transmitted or received.
 *
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *
 * @note
 *   If the buffer length is less than or equal to 16 bytes, or if the NCP is not initialized, the function performs a direct byte-by-byte transfer.
 * 
 ******************************************************************************/
sl_status_t sl_si91x_host_uart_transfer(const void *tx_buffer, void *rx_buffer, uint16_t buffer_length);

/***************************************************************************/ /**
 * @brief
 *   Flushes the UART RX buffer by reading and discarding all incoming data.
 *
 * @details
 *   This function continuously checks for data availability in the UART RX buffer and reads the data to clear the buffer. 
 *
 *   It exits if no data is received within a timeout period of 2000 ticks.
 * 
 ******************************************************************************/
void sl_si91x_host_flush_uart_rx(void);

/***************************************************************************/ /**
 * @brief
 *   Enables hardware flow control for the UART interface on the host.
 *
 * @details
 *   This function initializes the UART interface with hardware flow control enabled.
 * 
 ******************************************************************************/
void sl_si91x_host_uart_enable_hardware_flow_control(void);

/***************************************************************************/ /**
 * @brief
 *   Checks if the host MCU is currently in an interrupt context.
 *
 * @details
 *   This function determines whether the host MCU is executing within an ISR by checking the Interrupt Control and State Register (ICSR) of the System Control Block (SCB). 
 *
 * @return
 *   True if the MCU is in an interrupt context and false if the MCU is not in an interrupt context.
 * 
 ******************************************************************************/
bool sl_si91x_host_is_in_irq_context(void);

/** @} */
