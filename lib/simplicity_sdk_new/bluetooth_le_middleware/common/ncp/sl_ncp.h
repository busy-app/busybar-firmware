/***************************************************************************//**
 * @file
 * @brief Bluetooth Network Co-Processor (NCP) Interface
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_NCP_H
#define SL_NCP_H

/***********************************************************************************************//**
 * @addtogroup ncp
 * @{
 **************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"
#include "sl_ncp_config.h"
#include "sl_bt_api.h"
#include "sl_power_manager.h"
#include "sl_component_catalog.h"
#if defined(SL_CATALOG_BTMESH_PRESENT)
#include "sl_btmesh_api.h"
#endif // SL_CATALOG_BTMESH_PRESENT

#ifdef __cplusplus
extern "C" {
#endif

/// NCP error type
typedef enum {
  SL_NCP_ERROR_RUNTIME    = 0,
  SL_NCP_ERROR_RECEIVE    = 1,
  SL_NCP_ERROR_TIMER      = 2,
  SL_NCP_ERROR_ENCRYPT    = 3,
  SL_NCP_ERROR_RSP_BUFFER = 4,
  SL_NCP_ERROR_EVT_ENQUE  = 5,
  SL_NCP_ERROR_EVT_DEQUE  = 6,
} sl_ncp_error_t;

/**************************************************************************//**
 * NCP initialization function.
 *****************************************************************************/
void sl_ncp_init(void);

/**************************************************************************//**
 * NCP runtime ready.
 *****************************************************************************/
void sl_ncp_rta_ready(void);

/**************************************************************************//**
 * User command (message_to_target) handler callback.
 *
 * Handle user defined commands received from NCP-host.
 *
 * @param[in] data Data received from NCP through UART.
 *****************************************************************************/
void sl_ncp_user_cmd_message_to_target_cb(void *data);

/**************************************************************************//**
 * CS user command (message_to_target) handler callback.
 *
 * Handle CS defined user commands received from NCP-host.
 *
 * @param[in] data Data received from NCP through UART.
 *****************************************************************************/
void sl_ncp_user_cs_cmd_message_to_target_cb(const void *data);

/**************************************************************************//**
 * Send user command (message_to_target) response.
 *
 * Send response to user defined (message_to_target) command to NCP-host.
 *
 * @param[out] result Result of the response to the command received.
 * @param[out] len Message length.
 * @param[out] data Data to send to NCP.
 *****************************************************************************/
void sl_ncp_user_cmd_message_to_target_rsp(sl_status_t result,
                                           uint8_t len,
                                           uint8_t *data);

/**************************************************************************//**
 * Send user event (message_to_host).
 *
 * Send user defined (message_to_host) event to NCP-host.
 *
 * @param[out] len Message length.
 * @param[out] data Data to send to NCP.
 *****************************************************************************/
void sl_ncp_user_evt_message_to_host(uint8_t len, uint8_t *data);

/**************************************************************************//**
 * Local event processor for handling Bluetooth events in the application.
 *
 * @note This function can be implemented in the application.
 *
 * @param[in] evt Bluetooth event.
 *
 * @retval true if event needs to be passed to the host.
 *****************************************************************************/
bool sl_ncp_local_evt_process(sl_bt_msg_t *evt);

/**************************************************************************//**
 * Process Bluetooth events locally using template contribution.
 *
 * @note Automatically calls @ref sl_ncp_local_evt_process after all component
 *       callback is finished.
 *
 * @param[in] evt Bluetooth event.
 *
 * @retval true if event needs to be passed to the host.
 *****************************************************************************/
bool sl_ncp_local_common_evt_process(sl_bt_msg_t *evt);

#if defined(SL_CATALOG_BTMESH_PRESENT)

/**************************************************************************//**
 * Local event processor for handling Bluetooth Mesh events in the application.
 *
 * @note This function can be implemented in the application.
 *
 * @param[in] evt Bluetooth Mesh event.
 *
 * @retval true if event needs to be passed to the host.
 *****************************************************************************/
bool sl_ncp_local_btmesh_evt_process(sl_btmesh_msg_t *evt);

/**************************************************************************//**
 * Process Bluetooth Mesh events locally using template contribution.
 *
 * @note Automatically calls @ref sl_ncp_local_btmesh_evt_process after all
 *       component callback is finished.
 *
 * @param[in] evt Bluetooth Mesh event.
 *
 * @retval true if event needs to be passed to the host.
 *****************************************************************************/
bool sl_ncp_local_common_btmesh_evt_process(sl_btmesh_msg_t *evt);

#endif // SL_CATALOG_BTMESH_PRESENT

/**************************************************************************//**
 * NCP runtime error callback.
 *
 * @note The error callback is called when an error has occurred in the runtime
 *       context. The weak implementation asserts. It can be overridden in user
 *       code by adding a strong implementation.
 *
 * @param error  error type
 * @param status status code
 *
 *****************************************************************************/
void sl_ncp_on_error(sl_ncp_error_t error, sl_status_t status);

#ifdef __cplusplus
}
#endif

/** @} (end addtogroup ncp) */
#endif // SL_NCP_H
