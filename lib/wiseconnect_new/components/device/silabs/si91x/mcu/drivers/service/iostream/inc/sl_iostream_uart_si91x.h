/***************************************************************************/ /**
 * @file
 * @brief IO Stream UART Component.
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

#ifndef SL_IOSTREAM_UART_SI91X_H
#define SL_IOSTREAM_UART_SI91X_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif
#include "em_device.h"
#include "sl_iostream.h"
#include "sl_status.h"
/***************************************************************************/ /**
 * @addtogroup iostream
 * @{
 ******************************************************************************/

/***************************************************************************/ /**
 * @addtogroup iostream_uart I/O Stream UART
 * @brief I/O Stream UART
 * @details
 * ## Overview
 *
 *   UART layer provides a set of standard APIs that can be used with all type of
 *   UART interface.
 *
 * ## Initialization
 *
 *   Each UART stream type provides its initalization with parameters specific to them.
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Data Types

#define uartFlowControlNone     0
#define uartFlowControlSoftware 0xFFFF

// Iostream Instance in Si91x ,7 because using same sl_iostream.h
#define SL_IOSTREAM_TYPE_91X_UART 7

/// @brief I/O Stream UART stream object
typedef struct {
  sl_iostream_t stream;                           ///< stream
  sl_status_t (*deinit)(void *stream);            ///< uart deinit
  void (*set_auto_cr_lf)(void *context, bool on); ///< set_auto_cr_lf
  bool (*get_auto_cr_lf)(void *context);          ///< get_auto_cr_lf
} sl_iostream_uart_t;

/// @brief I/O Stream UART config
typedef struct {
  uint8_t *rx_buffer;      ///< UART Rx Buffer
  size_t rx_buffer_length; ///< UART Rx Buffer length
  bool lf_to_crlf;         ///< lf_to_crlf
} sl_iostream_uart_config_t;

/// @brief I/O Stream UART context
typedef struct {
  uint8_t *rx_buffer;                          ///< UART Rx Buffer
  size_t rx_buffer_len;                        ///< UART Rx Buffer length
  sl_status_t (*tx)(void *context, char c);    ///< Tx function pointer
  void (*set_next_byte_detect)(void *context); ///< Pointer to a function to enable detection of next
                                               ///< byte on stream
  sl_status_t (*deinit)(void *context);        ///< DeInit function pointer
  bool lf_to_crlf;                             ///< lf_to_crlf
} sl_iostream_uart_context_t;

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************/ /**
 * UART Stream De-init.
 *
 * @param[in] iostream_uart  UART stream object.
 *
 * @return Status result
 ******************************************************************************/
__STATIC_INLINE sl_status_t sl_iostream_uart_deinit(sl_iostream_uart_t *iostream_uart)
{
  return iostream_uart->deinit(iostream_uart);
}

/***************************************************************************/ /**
 * Configure Automatic line conversion.
 *
 * @param[in] iostream_uart  UART stream object.
 *
 * @param[in] on  If true, automatic LF to CRLF conversion will be enabled.
 ******************************************************************************/
__STATIC_INLINE void sl_iostream_uart_set_auto_cr_lf(sl_iostream_uart_t *iostream_uart, bool on)
{
  iostream_uart->set_auto_cr_lf(iostream_uart->stream.context, on);
}

/***************************************************************************/ /**
 * Get Automatic line conversion.
 *
 * @param[in] iostream_uart   UART stream object.
 *
 * @return Auto-conversion mode.
 ******************************************************************************/
__STATIC_INLINE bool sl_iostream_uart_get_auto_cr_lf(sl_iostream_uart_t *iostream_uart)
{
  return iostream_uart->get_auto_cr_lf(iostream_uart->stream.context);
}

/***************************************************************************/ /**
 * UART Set next byte detect IRQ.
 *
 * @param[in] iostream_uart  UART stream object.
 ******************************************************************************/
__STATIC_INLINE void sl_iostream_uart_prepare_for_sleep(sl_iostream_uart_t *iostream_uart)
{
  ((sl_iostream_uart_context_t *)iostream_uart->stream.context)->set_next_byte_detect(iostream_uart->stream.context);
}
/** @} (end addtogroup iostream_uart) */
/** @} (end addtogroup iostream) */

#ifdef __cplusplus
}
#endif

#endif // SL_IOSTREAM_UART_SI91X_H
