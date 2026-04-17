/***************************************************************************/ /**
 * @file
 * @brief SI91x IO Stream SWO ITM Component header.
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
#ifndef SL_SI91X_IOSTREAM_SWO_ITM_8_H
#define SL_SI91X_IOSTREAM_SWO_ITM_8_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/**
 * @brief Enumeration for different Message types.
 */
typedef enum {
  SI91X_DEBUG_PRINTF          = 0x0002,
  SI91X_DEBUG_VIRTUAL_UART_TX = 0x0011,
  SI91X_DEBUG_VIRTUAL_UART_RX = 0x0012
} sl_si91x_iostream_swo_itm_8_msg_type_t;

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_IOSTREAM_SWO_ITM_8_H */
