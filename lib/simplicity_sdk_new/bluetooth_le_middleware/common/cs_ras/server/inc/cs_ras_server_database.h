/***************************************************************************//**
 * @file
 * @brief CS RAS Server - Ranging Database
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

#ifndef CS_RAS_SERVER_DATABASE_H
#define CS_RAS_SERVER_DATABASE_H

#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"
#include "cs_ras_common.h"
#include "sl_bt_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// RAS Server Ranging Database internal API

// -----------------------------------------------------------------------------
// Macros

// -------------------------------
// Enums, structs, typedefs

// Type for storing procedure data in the ranging database
typedef struct {
  uint8_t *data;                            // Procedure data
  uint32_t data_len;                        // Length of the procedure data
  cs_ras_ranging_counter_t ranging_counter; // Ranging counter. This is saved separately. When overwrite occurs,
                                            // the logic must use this field to indicate which subevent was overwritten
  cs_ras_ranging_header_t ras_header;
  cs_ras_subevent_header_t ras_subevent_header;
  bool empty;                               // Show whether it's an occupied or free slot which can be overwritten
  bool overwritten;                         // True if the slot is overwritten
  bool sent;                                // Indicates that the data has been sent
} cs_procedure_data_t;

/**************************************************************************//**
 * Initialize RAS database.
 *
 *****************************************************************************/
void cs_ras_server_database_init(void);

/**************************************************************************//**
 * Set RAS filter for the connection set by conn_handle.
 *
 * @param[in] conn_handle The connection handle.
 * @param[in] filter      Filter pattern.
 * @return status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_server_database_set_filter(uint8_t conn_handle,
                                              cs_ras_filter_t filter);

/**************************************************************************//**
 * Store a CS result to the database.
 *
 * @param[in]  cs_res         A pointer to a BLE message.
 * @param[in]  mode           RAS mode
 * @return status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_server_database_store_result(sl_bt_msg_t *cs_res,
                                                cs_ras_mode_t mode);

/**************************************************************************//**
 * Mark a procedure sent.
 * @param[in]  conn_handle      The connection handle.
 * @param[in]  ranging_counter  Ranging counter.
 *****************************************************************************/
void cs_ras_server_database_sent(uint8_t conn_handle,
                                 cs_ras_ranging_counter_t ranging_counter);

/**************************************************************************//**
 * Purge any data for a given ranging counter.
 * Useful when all data has been sent to the client.
 *
 * @param[in] conn_handle The connection handle.
 * @param[in] ranging_counter Ranging counter
 *****************************************************************************/
void cs_ras_server_database_clear(uint8_t conn_handle,
                                  cs_ras_ranging_counter_t ranging_counter);

/**************************************************************************//**
 * Get next slot to be transmitted for a given ranging counter and connection.
 *
 * @param[in] conn_handle The connection handle.
 * @param[in] ranging_counter Ranging counter.
 * @return Pointer to the slot or NULL if not found.
 *****************************************************************************/
cs_procedure_data_t *cs_ras_server_database_find_next_slot(const uint8_t conn_handle,
                                                           cs_ras_ranging_counter_t ranging_counter);

/**************************************************************************//**
 * Purge any data from the connection data.
 *
 * @param[in] conn_handle the connection handle to erase all data for
 *****************************************************************************/
void cs_ras_server_database_clear_connection(const uint8_t conn_handle);

#ifdef __cplusplus
};
#endif

#endif // CS_RAS_SERVER_DATABASE_H
