/***************************************************************************//**
 * @file
 * @brief Bluetooth NCP transport layer header
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

#ifndef SL_BT_NCP_TRANSPORT_H
#define SL_BT_NCP_TRANSPORT_H

/***********************************************************************************************//**
 * @addtogroup bluetooth_ncp_transport
 * @{
 **************************************************************************************************/

#include <stdint.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Bluetooth NCP Transport error type
typedef enum {
  SL_BT_NCP_TRANSPORT_ERROR_INIT        = 0,
  SL_BT_NCP_TRANSPORT_ERROR_RUNTIME     = 1,
  SL_BT_NCP_TRANSPORT_ERROR_COM_HANDLE  = 2,
  SL_BT_NCP_TRANSPORT_ERROR_TX_OVERFLOW = 3,
  SL_BT_NCP_TRANSPORT_ERROR_CLOCK       = 4,
  SL_BT_NCP_TRANSPORT_ERROR_CLOSE       = 5,
  SL_BT_NCP_TRANSPORT_ERROR_RX          = 6
} sl_bt_ncp_transport_error_t;

/**************************************************************************//**
 * Transmit function
 * @param len  length of the data to transmit
 * @param data data to transmit
 *****************************************************************************/
void sl_bt_ncp_transport_transmit(uint32_t len, const uint8_t *data);

/**************************************************************************//**
 * Transmit completed callback
 *****************************************************************************/
void sl_bt_ncp_transport_on_transmit(sl_status_t status);

/**************************************************************************//**
 * Receive function
 *****************************************************************************/
void sl_bt_ncp_transport_receive(void);

/**************************************************************************//**
 * Receive completed callback
 * @param status status of the reception
 * @param len    length of the received data
 * @param data   received data
 *****************************************************************************/
void sl_bt_ncp_transport_on_receive(sl_status_t status,
                                    uint32_t    len,
                                    uint8_t     *data);

/**************************************************************************//**
 * Runtime error callback.
 * @note The error callback is called when an error has occurred in the runtime
 *       context. The weak implementation asserts. It can be overridden in user
 *       code by adding a strong implementation.
 * @param error  error type
 * @param status status code
 *****************************************************************************/
void sl_bt_ncp_transport_on_error(sl_bt_ncp_transport_error_t error,
                                  sl_status_t                 status);

#ifdef __cplusplus
}
#endif

/** @} (end addtogroup bluetooth_ncp_transport) */
#endif // SL_BT_NCP_TRANSPORT_H
