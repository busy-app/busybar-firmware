/***************************************************************************/ /**
 * @file
 * @brief IO Stream UART internal APIs.
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

#ifndef SLI_IOSTREAM_UART_H
#define SLI_IOSTREAM_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#include "sl_status.h"
#include "sl_iostream.h"
#include "sl_iostream_uart.h"

#define SLI_IOSTREAM_UART_FLAG_CTS            0x01
#define SLI_IOSTREAM_UART_FLAG_RTS            0x02
#define SLI_IOSTREAM_UART_FLAG_HIGH_FREQUENCY 0x04

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

sl_status_t sli_iostream_uart_context_init(sl_iostream_uart_t *uart,
                                           sl_iostream_uart_context_t *context,
                                           sl_iostream_uart_config_t *config);

/***************************************************************************/ /**
 * @brief Subscribe to receive a callback when new data is received.
 *
 * @note  The callback is called from an IRQ context
 *
 * @details This function provides a notification mechanism when data becomes
 *          available in the UART receive buffer. The callback is triggered once
 *          when the buffer transitions from empty to containing one or more bytes
 *          of data. Note that notifications are not sent for each individual byte
 *          received; a single notification is sent regardless of how many bytes
 *          arrive. To receive subsequent notifications, the user must drain the
 *          buffer by calling sl_iostream_read() before more data arrives.
 *
 * @warning The callback must be non-blocking and execute quickly. Do not call
 *          any IOstream API functions from within the callback. This callback
 *          serves only as a notification mechanism to signal another process
 *          (e.g. a task) to proceed with reading data.
 *
 * @note    Usage guidelines:
 *          - Use this callback as a notification only
 *          - Move actual data handling (printing, processing) outside of the callback
 *          - Drain the buffer promptly so interrupts continue firing
 *
 * @param[in] iostream_uart  Pointer to the UART stream object
 * @param[in] callback       Function pointer to the callback
 * @param[in] callback_data  Pointer to the user data to be passed to the callback
 *
 * @return sl_status_t       Status code
 ******************************************************************************/
sl_status_t sli_iostream_uart_subscribe_to_new_data(sl_iostream_uart_t *iostream_uart,
                                                    sl_iostream_uart_new_data_callback_t callback,
                                                    void *callback_data);

/***************************************************************************/ /**
 * @brief Unsubscribe from receiving new data notifications.
 *
 * This function allows you to unsubscribe from receiving new data notifications
 * on the specified UART stream.
 *
 * @param[in] iostream_uart  Pointer to the UART stream object
 *
 * @return sl_status_t       Status code
 ******************************************************************************/
sl_status_t sli_iostream_uart_unsubscribe_to_new_data(sl_iostream_uart_t *iostream_uart);

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && !defined(SL_IOSTREAM_UART_FLUSH_TX_BUFFER)
void sli_uart_txc(void *context);
#endif

#ifdef __cplusplus
}
#endif

#endif /* SLI_IOSTREAM_UART_H */
