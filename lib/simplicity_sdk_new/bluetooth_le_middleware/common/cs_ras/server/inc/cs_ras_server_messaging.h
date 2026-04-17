/***************************************************************************//**
 * @file
 * @brief CS RAS Server - Messaging header
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

#ifndef CS_RAS_SERVER_MESSAGING_H
#define CS_RAS_SERVER_MESSAGING_H

#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"
#include "cs_ras_common.h"
#include "sl_slist.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif // SL_CATALOG_POWER_MANAGER_PRESENT

/// RAS transfer configuration and storage type
typedef struct {
  sl_slist_node_t           node;            ///< List node
  uint16_t                  segment_counter; ///< Rolling segment counter of last segment
  struct {
    uint8_t                 created   : 1;   ///< Message segment has been created
    uint8_t                 sent      : 1;   ///< Message segment has been sent
    uint8_t                 confirmed : 1;   ///< Message segment indication was confirmed
    uint8_t                 stopped   : 1;   ///< Stop transmission
  } message_status;                          ///< Message status bitfield
  sl_status_t               status;          ///< Transmission status
  cs_ras_messaging_config_t config;          ///< Transmission config
  uint16_t                  segment_size;    ///< Size of the segment
  cs_ras_segment_storage_t  storage;         ///< Internal storage for a segment
} cs_ras_server_messaging_transmit_t;

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * Send all segments of the data using the configuration specified in
 * @ref config.
 *
 * @note The @ref start_segment and @ref end_segment fields pointed by
 * @ref config are omitted.
 *
 * @param[in] tx     Pointer to the transmit structure
 * @param[in] config Pointer to the transmit configuration
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_server_messaging_transmit_all(cs_ras_server_messaging_transmit_t *tx,
                                                 cs_ras_messaging_config_t          *config);

/**************************************************************************//**
 * Send segments of the data using the configuration specified in
 * @ref config.
 *
 * @param[in] tx     Pointer to the transmit structure
 * @param[in] config Pointer to the transmit configuration
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_server_messaging_transmit(cs_ras_server_messaging_transmit_t *tx,
                                             cs_ras_messaging_config_t          *config);

/**************************************************************************//**
 * Cancel an active transmission
 *
 * @param[in] tx Pointer to the transmit structure
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_server_messaging_cancel(cs_ras_server_messaging_transmit_t *tx);

#ifdef __cplusplus
};
#endif

#endif // CS_RAS_SERVER_MESSAGING_H
