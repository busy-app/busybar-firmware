/***************************************************************************//**
 * @file
 * @brief CS RAS Client Timeout API
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

#ifndef CS_RAS_CLIENT_TIMEOUT_H
#define CS_RAS_CLIENT_TIMEOUT_H

#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"
#include "cs_ras_common.h"

/// Type of the timeout
typedef enum {
  CS_RAS_CLIENT_TIMEOUT_REAL_TIME_START,           // CS_RAS_CLIENT_REAL_TIME_RANGING_DATA_TIMEOUT
  CS_RAS_CLIENT_TIMEOUT_REAL_TIME,                 // CS_RAS_CLIENT_INTER_EVENT_TIMEOUT
  CS_RAS_CLIENT_TIMEOUT_DATA_READY_INDICATION,     // CS_RAS_CLIENT_RANGING_DATA_READY_PROCEDURE_TIMEOUT
  CS_RAS_CLIENT_TIMEOUT_DATA_READY_NOTIFICATION,   // CS_RAS_CLIENT_RANGING_DATA_READY_PROCEDURE_TIMEOUT
  CS_RAS_CLIENT_TIMEOUT_ON_DEMAND_START,           // CS_RAS_CLIENT_GET_ON_DEMAND_RANGING_DATA_TIMEOUT
  CS_RAS_CLIENT_TIMEOUT_ON_DEMAND,                 // CS_RAS_CLIENT_INTER_EVENT_TIMEOUT
  CS_RAS_CLIENT_TIMEOUT_CONTROL_POINT              // CS_RAS_CLIENT_CONTROL_POINT_TIMEOUT
} cs_ras_client_timeout_t;

/// Action after timed out
typedef enum {
  CS_RAS_CLIENT_TIMEOUT_ACTION_NOTHING = 0,
  CS_RAS_CLIENT_TIMEOUT_ACTION_DISABLE_REAL_TIME_MODE,
  CS_RAS_CLIENT_TIMEOUT_ACTION_READ_DATA_READY,
  CS_RAS_CLIENT_TIMEOUT_ACTION_ABORT,
  CS_RAS_CLIENT_TIMEOUT_ACTION_CLOSE_CONNECTION
} cs_ras_client_timeout_action_t;

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * Pass procedure state to Ranging Profile timeout management.
 *
 * @param[in] connection Connection handle.
 * @param[in] enabled    True for procedure was enabled, false for disabled.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_procedure_enabled(uint8_t connection,
                                            bool    enabled);

/**************************************************************************//**
 * Callback for timeout.
 *
 * @param[in] connection Connection handle.
 * @param[in] timeout    Timeout type (origin).
 * @param[in] action     Action to be made..
 * @retval true the action was successful.
 *****************************************************************************/
bool cs_ras_client_on_timeout(uint8_t                        connection,
                              cs_ras_client_timeout_t        timeout,
                              cs_ras_client_timeout_action_t action);

#ifdef __cplusplus
};
#endif

#endif // CS_RAS_CLIENT_TIMEOUT_H
