/***************************************************************************//**
 * @file
 * @brief Debug trace.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef TRACE_H
#define TRACE_H

#include "sl_component_catalog.h"
#include "app_log.h"
#include "app_config.h"

#if defined(SL_CATALOG_BGAPI_TRACE_PRESENT) && CS_INITIATOR_UART_LOG
#include "sl_iostream.h"
#include "sl_iostream_handles.h"
// Forward messages to 2 iostream instances.
#define log_debug(...)                                                       \
  do {                                                                       \
    app_log_debug(__VA_ARGS__);                                              \
    sl_iostream_printf(sl_iostream_recommended_console_stream, __VA_ARGS__); \
  } while (0)

#define log_info(...)                                                        \
  do {                                                                       \
    app_log_info(__VA_ARGS__);                                               \
    sl_iostream_printf(sl_iostream_recommended_console_stream, __VA_ARGS__); \
  } while (0)

#define log_append(...)                                                      \
  do {                                                                       \
    app_log_append(__VA_ARGS__);                                             \
    sl_iostream_printf(sl_iostream_recommended_console_stream, __VA_ARGS__); \
  } while (0)

#define log_error(...)                                                       \
  do {                                                                       \
    app_log_error(__VA_ARGS__);                                              \
    sl_iostream_printf(sl_iostream_recommended_console_stream, __VA_ARGS__); \
  } while (0)
#else
#define log_debug(...)   app_log_debug(__VA_ARGS__)
#define log_info(...)    app_log_info(__VA_ARGS__)
#define log_error(...)   app_log_error(__VA_ARGS__)
#define log_append(...)  app_log_append_info(__VA_ARGS__)
#endif

/**************************************************************************//**
 * Initialize debug trace.
 *****************************************************************************/
void trace_init(void);

#endif // TRACE_H
