/*******************************************************************************
 * @file
 * @brief Development Kit debugging utilities
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

#ifndef __DEBUG_CHANNEL_H__
#define __DEBUG_CHANNEL_H__

#include <sl_iostream_swo_itm_8.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//******************************************************************************

/**
 * @addtogroup debug
 * @brief A set of utilities for printing to the debug backchannel.
 * @{
 */

/**
 * @brief Builds and prints a message on the debug channel using a SL-style format string
 *
 * @param[in]   debugType       the debug message type
 * @param[in]   formatString    SL-style format string
 */
void sl_debug_binary_format(sl_iostream_swo_itm_8_msg_type_t debugType, const char *formatString, ...);

/**
 * @brief Wrapper for sl_iostream_printf using the debug handle
 *
 * @param formatString
 * @param ...
 */
void sl_debug_printf(const char *formatString, ...);

/**
 * @brief Wrapper for sl_iostream_write using the debug handle
 *
 * @param[in]   debugType       the debug message type
 * @param[in]   buffer          buffer that contains the data to output
 * @param[in]   buffer_length   data length contained in the buffer.

 */
void sl_debug_binary_write(sl_iostream_swo_itm_8_msg_type_t debugType, const void *buffer, size_t buffer_length);

/**
 * @}
 *
 */

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__DEBUG_CHANNEL_H__
