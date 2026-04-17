/***************************************************************************//**
 * @file
 * @brief ESL Access Point Connection header.
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

#ifndef ESL_LIB_CONNECTION_H
#define ESL_LIB_CONNECTION_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "sl_bt_api.h"
#include "esl_lib.h"
#include "esl_lib_image_transfer.h"
#include "esl_lib_command_list.h"
#include "app_timer.h"
#include "esl_lib_storage.h"
#include "esl_lib_log.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Definitions

#define ESL_LIB_CONNECTION_RETRY_COUNT_MAX          3

/// Tag info storage type
typedef struct {
  sl_slist_node_t          node;    ///< Node for the list
  esl_lib_data_type_t      type;    ///< Type of the data
  esl_lib_storage_handle_t storage; ///< Data storage
} esl_lib_connection_tag_info_storage_t;

/// Connection list item type
typedef struct {
  sl_slist_node_t                 node;              ///< List node pointer
  esl_lib_command_list_cmd_t      *command;          ///< Command in progress
  sl_slist_node_t                 *command_list;     ///< Command list
  sl_slist_node_t                 *tag_info_list;    ///< Tag info storage list
  bool                            command_complete;  ///< Finished command
  bool                            established;       ///< Connection opened & established
  uint8_t                         connection_handle; ///< Connection handle
  uint8_t                         max_payload;       ///< Max payload
  uint8_t                         security;          ///< Security level
  uint8_t                         config_index;      ///< Current config index
  uint8_t                         address_type;      ///< Address type
  bd_addr                         address;           ///< Address
  esl_lib_connection_state_t      state;             ///< State of the connection
  esl_lib_image_transfer_handle_t ots_handle;        ///< OTS handle
  aes_key_128                     ltk;               ///< LTK for the connection
  app_timer_t                     timer;             ///< Connection timer
  app_timer_t                     gatt_timer;        ///< GATT timer
  sl_status_t                     last_error;        ///< Last error
  esl_lib_bool_t                  gattdb_known;      ///< Predefined GATT database
  esl_lib_gattdb_handles_t        gattdb_handles;    ///< List of UUIDs
  esl_lib_data_type_t             tag_info_type;     ///< Current tag info type
  esl_lib_storage_handle_t        tag_info_data;     ///< Current tag info data
  esl_lib_data_type_t             config_type;       ///< Current config type
} esl_lib_connection_t;

// -----------------------------------------------------------------------------
// Public functions

/**************************************************************************//**
 * Check current connection mode and core status.
 * @param[out] core_state Pointer to status response output. Can be NULL if
 *                        the caller is not interested in core status.
 * @param[out] connections Pointer to active connection handle count variable.
 *                         Can be NULL if the caller is not interested in
 *                         active connection handles count
 * @param[out] initiator_busy Pointer to a bool variable to indicate if the
 *                            initiator list is busy at Link Layer level
 *                            (i.e. it's currently unable to accept new items).
 *                            Can be NULL if the caller is not interested in
 *                            the LL initiator list state.
 * @return ESL library connection mode
 *****************************************************************************/
esl_lib_connection_mode_t esl_lib_get_connection_mode_and_status(esl_lib_core_state_t *core_state,
                                                                 uint8_t *connections,
                                                                 bool *initiator_busy);

/**************************************************************************//**
 * Change connection mode.
 * @param[in] mode The connection mode to be set: either @ref
 *                 ESL_LIB_CONNECTION_MODE_SINGLE for manual, or @ref
 *                 ESL_LIB_CONNECTION_MODE_LIST for automatic connection
 *                 initiation based on the Initiator Filter Policy using
 *                 the Filter Accept List stack feature.
 *
 * @return Status code
 *****************************************************************************/
sl_status_t esl_lib_change_connection_mode(esl_lib_connection_mode_t mode);

/**************************************************************************//**
 * Get current list size of Filter Accept List for auto connection initiator.
 *
 * @return ESL library initiator list (Filter Accept List) size
 *****************************************************************************/
size_t esl_lib_get_initiator_filter_size(void);

/**************************************************************************//**
 * Add a connection to the list.
 * @param[in]  conn    Connection handle.
 * @param[out] ptr_out Pointer output.
 *
 * @return Status code.
 *****************************************************************************/
sl_status_t esl_lib_connection_add(uint8_t              conn,
                                   esl_lib_connection_t **ptr_out);

/**************************************************************************//**
 * Find a connection in the list.
 * @param[in]  conn    Connection handle.
 * @param[out] ptr_out Pointer output.
 *
 * @return Status code.
 *****************************************************************************/
sl_status_t esl_lib_connection_find(uint8_t              conn,
                                    esl_lib_connection_t **ptr_out);

/**************************************************************************//**
 * Remove a connection from the list.
 * @param[in] ptr Connection pointer.
 *
 * @return Status code.
 *****************************************************************************/
sl_status_t esl_lib_connection_remove_ptr(esl_lib_connection_t *ptr);

/**************************************************************************//**
 * Check connection list for the given item.
 * @param[in] ptr Pointer to the connection list item.
 *
 * @return true if the list contains the item.
 *****************************************************************************/
bool esl_lib_connection_contains(esl_lib_connection_t *ptr);

/**************************************************************************//**
 * Close any ongoing connection and clean all related data.
 *****************************************************************************/
void esl_lib_connection_cleanup(void);

/**************************************************************************//**
 * Clear all Filter Accept List data for the Initiator Filter policy.
 *
 * @return Status code from stack.
 *****************************************************************************/
sl_status_t esl_lib_initiator_filter_cleanup(void);

/**************************************************************************//**
 * Clear all Filter Accept List data for the Initiator Filter policy and
 * release its resources.
 *****************************************************************************/
void esl_lib_auto_initiator_deinit(void);

/**************************************************************************//**
 * Add a command to the command list of the connection.
 *
 * @param[in] conn Pointer to the connection list item.
 * @param[in] cmd  Pointer to the command to add.
 *
 * @return true if the list contains the item.
 *****************************************************************************/
sl_status_t esl_lib_connection_add_command(esl_lib_connection_t       *conn,
                                           esl_lib_command_list_cmd_t *cmd);

/**************************************************************************//**
 * Connection step.
 *****************************************************************************/
void esl_lib_connection_step(void);

/**************************************************************************//**
 * Bluetooth event handler.
 *
 * @param[in] evt Bluetooth event.
 *
 *****************************************************************************/
void esl_lib_connection_on_bt_event(sl_bt_msg_t *evt);

/**************************************************************************//**
 * Check for the validity of the GATT database handles.
 *
 * @param[in] conn Pointer to the GATT database handles.
 *
 * @return SL_STATUS_OK if the configuration is valid.
 *****************************************************************************/
sl_status_t esl_lib_connection_check_gattdb_handles(esl_lib_gattdb_handles_t *gattdb_handles);

/**************************************************************************//**
 * Request connection initiation.
 *
 * @param[in] cmd Connection open library command.
 *
 * @return SL_STATUS_OK if initiating the connection was successful.
 *
 * @note In manual connection mode or in the case of a connection request via
 *       PAwR, the initiation occurs immediately upon success, but in automatic
 *       initiator mode, the action is slightly delayed by Initiator Filter
 *       Policy actions performed at the library and then at the Link Layer.
 *       Furthermore, even in auto mode, an immediate connection request will
 *       be issued if the command is associated with a foreign identity address
 *       and/or passkey TLV(s). In the latter case, however, there is an
 *       increased chance of the request being rejected, since in auto mode
 *       the stack may be busy at the moment the request is processed.
 *****************************************************************************/
sl_status_t esl_lib_initiate_connection(esl_lib_command_list_cmd_t *cmd);

#ifdef __cplusplus
};
#endif

#endif // ESL_LIB_CONNECTION_H
