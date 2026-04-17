/***************************************************************************//**
 * @file
 * @brief ESL Host Library core component's header.
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

#ifndef ESL_LIB_CORE_H
#define ESL_LIB_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include "esl_lib.h"
#include "esl_lib_event_list.h"
#include "esl_lib_command_list.h"
#include "esl_lib_connection.h"
#include "esl_lib_pawr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CHECK_NULL(x)              \
  if ((x) == NULL) {               \
    return SL_STATUS_NULL_POINTER; \
  }

#define ESL_SERVICE_UUID { 0x57, 0x18 }

/// Scan status type
typedef esl_lib_evt_scan_status_t esl_lib_scan_status_t;

/// Access Point state data
typedef struct {
  esl_lib_scan_status_t      scan;             ///< Scan status
  esl_lib_bool_t             scanner_suspended;///< Scanner - lib internal state
  esl_lib_command_list_cmd_t *command;         ///< Command in progress
  bool                       command_complete; ///< Finished command
  sl_slist_node_t            *command_list;    ///< Command list
  esl_lib_core_state_t       core_state;       ///< Core state
} esl_lib_ap_state_t;

/**************************************************************************//**
 * Library Init.
 *
 * @param[in] config Configuration string.
 *
 *****************************************************************************/
void esl_lib_init(char *config);

/**************************************************************************//**
 * Library Process Action.
 *****************************************************************************/
void esl_lib_process_action(void);

/**************************************************************************//**
 * Library Deinit.
 *****************************************************************************/
void esl_lib_deinit(void);

/**************************************************************************//**
 * Add common command.
 *
 * @param[in] cmd Command to add.
 *
 *****************************************************************************/
sl_status_t esl_lib_core_add_command(esl_lib_command_list_cmd_t *cmd);

/**************************************************************************//**
 * Connection request complete callback.
 *****************************************************************************/
void esl_lib_core_connection_complete(void);

/***************************************************************************//**
 *
 * Read the Bluetooth identity address used by the ESL AP, which can be a public
 * or random static device address.
 *
 * @param[out] address Bluetooth identity address in little endian format
 * @param[out] type Enum @ref sl_bt_gap_address_type_t. Identity address type.
 *   Values:
 *     - <b>sl_bt_gap_public_address (0x0):</b> Public device address
 *     - <b>sl_bt_gap_static_address (0x1):</b> Static device address
 *
 * @return SL_STATUS_OK if successful or SL_STATUS_INITIALIZATION if called
 *       before the sl_bt_evt_system_boot_id event from the radio has arrived
 *       and been processed. Returns SL_STATUS_NULL_POINTER in case of NULL
 *       argument(s).
 ******************************************************************************/
sl_status_t esl_lib_core_get_identity_address(bd_addr *address, uint8_t *type);

#ifdef __cplusplus
};
#endif

#endif // ESL_LIB_CORE_H
