/***************************************************************************/ /**
 * @file sl_iostream_usart_si91x.h
 * @brief IO Stream USART Component.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_IOSTREAM_USART_SI91X_H
#define SL_IOSTREAM_USART_SI91X_H

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif
#include "sl_iostream.h"
#include "sl_iostream_uart_si91x.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/ /**
 * @addtogroup iostream
 * @{
 ******************************************************************************/

/***************************************************************************/ /**
 * @addtogroup iostream_usart I/O Stream USART
 * @brief I/O Stream USART
 * @details
 * ## Overview
 *
 *   The Universal Synchronous / Asynchronous Receiver / Transmitter controller(s) (USART)
 *   can be used as a UART and can, therefore, be connected to an external transceiver
 *   to communicate with another host using the serial link. Multiple instances can be
 *   instantiated as long as they are bound to a unique USART peripheral. The flow control
 *   is supported as well.
 *
 * ## Initialization
 *
 *   The stream sets itself as the default stream at the end of the initialization
 *   function.You must reconfigure the default interface if you have multiple streams
 *   in your project else the last stream initialized will be set as the system default
 *   stream.
 *
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Data Types

#include "sl_si91x_usart.h"

static const sl_si91x_usart_control_config_t usart_config;
#define USART_InitAsync_TypeDef sl_si91x_usart_control_config_t

/// @brief I/O Stream USART config
typedef struct {
  uint8_t tx_pin;     ///< Transmit pin
  uint8_t rx_pin;     ///< Receive pin
  uint8_t cts_pin;    ///< Flow control, CTS pin
  uint8_t rts_pin;    ///< Flow control, RTS pin
  uint8_t rx_timeout; ///< Timeout delay between two Rx events
} sl_iostream_usart_config_t;

/// @brief I/O Stream USART context
typedef struct {
  sl_iostream_uart_context_t context; ///< usart_location
  uint8_t tx_pin;                     ///< Transmit pin
  uint8_t rx_pin;                     ///< Receive pin
} sl_iostream_usart_context_t;

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************/ /**
 * USART Stream init.
 *
 * @param[in] iostream_uart  I/O Stream UART handle.
 *
 * @param[in] uart_config  I/O Stream UART config.
 *
 * @param[in] init  USART initialization modes.
 *
 * @param[in] usart_config  USART configuration.
 *
 * @param[in] usart_context  USART Instance context.
 *
 * @return  Status result
 ******************************************************************************/
sl_status_t sl_iostream_usart_init(sl_iostream_uart_t *iostream_uart,
                                   sl_iostream_uart_config_t *uart_config,
                                   USART_InitAsync_TypeDef *init,
                                   sl_iostream_usart_config_t *usart_config,
                                   sl_iostream_usart_context_t *usart_context);

/*******************************************************************************
 * Usart interrupt handler.
 * @param[in] iostream_uart   I/O Stream UART handle.
 ******************************************************************************/
void sl_iostream_usart_irq_handler(sl_iostream_uart_t *iostream_uart);

/** @} (end addtogroup iostream_usart) */
/** @} (end addtogroup iostream) */

#ifdef __cplusplus
}
#endif

#endif // SL_IOSTREAM_USART_H