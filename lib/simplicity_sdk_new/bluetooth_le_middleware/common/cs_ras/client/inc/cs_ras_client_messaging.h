/***************************************************************************//**
 * @file
 * @brief CS RAS Client - Messaging header
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

#ifndef CS_RAS_CLIENT_MESSAGING_H
#define CS_RAS_CLIENT_MESSAGING_H

#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"
#include "sl_slist.h"
#include "cs_ras_common.h"
#include "cs_ras_client.h"

/// RAS transfer configuration and storage type
typedef struct {
  sl_slist_node_t           node;               ///< List node
  struct {
    uint16_t                current;            ///< Current segment
    uint16_t                previous;           ///< Previous segment
    uint16_t                last;               ///< Last segment
    uint16_t                lost;               ///< Lost segment counter
  } counter;                                    ///< Internal counters
  struct {
    uint8_t                 first_received : 1; ///< First segment received
    uint8_t                 last_received  : 1; ///< Last segment received
    uint8_t                 overflow       : 1; ///< Rolling Segment Counter Overflow
    uint8_t                 started        : 1; ///< Reception started
    uint8_t                 stopped        : 1; ///< Reception stopped
  } segment_status;                             ///< Segment reception status
  uint64_t                  lost_segments;      ///< Lost segment bitmask
  sl_status_t               status;             ///< Reception status
  cs_ras_messaging_config_t config;             ///< Reception config
  uint32_t                  size;               ///< Size of data
  cs_ras_gattdb_handles_t   *handles;           ///< GATTDB handles
} cs_ras_client_messaging_reception_t;

/// RAS transfer completion type
typedef PACKSTRUCT (struct {
  sl_status_t                     sc;                 ///< Reception status
  uint8_t                         start_segment;      ///< Start segment
  uint8_t                         end_segment;        ///< End segment
  bool                            recoverable;        ///< Recoverable reception
  uint32_t                        size;               ///< Received size (offset)
  bool                            last_arrived;       ///< Last segment arrived
  uint8_t                         last_known_segment; ///< Counter of last segment
  uint64_t                        lost_segments;      ///< Lost segment bitmask
}) cs_ras_client_messaging_complete_t;

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * Initialize messaging
 *****************************************************************************/
void cs_ras_client_messaging_init(void);

/**************************************************************************//**
 * Bluetooth event handler of RAS Client messaging.
 * @param[in] evt Event coming from the Bluetooth stack.
 * @return true to send the event to the host in NCP case.
 *****************************************************************************/
bool cs_ras_client_messaging_on_bt_event(sl_bt_msg_t *evt);

/**************************************************************************//**
 * Receive segments of the data using the configuration specified in
 * @ref config.
 *
 * Data is reassembled into the data specified in @ref config structure.
 * Start and end segment specified in @ref config is omitted.
 * To receive all segments, the start segment used in reception is always 0 and
 * end segment is always equal to CS_RAS_MESSAGING_ALL_SEGMENTS.
 *
 * @param[in] rx      Pointer to the reception structure
 * @param[in] config  Pointer to the messaging configuration
 * @param[in] handles Pointer to the GATT database handles. The selected Ranging
 *                    Data characteristic is mandatory.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_messaging_receive(cs_ras_client_messaging_reception_t *rx,
                                            cs_ras_messaging_config_t           *config,
                                            cs_ras_gattdb_handles_t             *handles);

/**************************************************************************//**
 * Receive lost segments from the RAS Server.
 *
 * @param[in] rx            Pointer to the reception structure
 * @param[in] config        Pointer to the messaging configuration
 * @param[in] handles       Pointer to the GATT database handles. The selected
 *                          Ranging Data characteristic is mandatory.
 * @param[in] start_segment Counter value of the start segment to retransmit
 * @param[in] end_segment   Counter value of the end segment to retransmit.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_messaging_receive_lost(cs_ras_client_messaging_reception_t *rx,
                                                 cs_ras_messaging_config_t           *config,
                                                 cs_ras_gattdb_handles_t             *handles,
                                                 uint8_t                             start_segment,
                                                 uint8_t                             end_segment);

/**************************************************************************//**
 * Stop reception.
 *
 * @param[in] rx     Pointer to the reception structure
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_messaging_stop(cs_ras_client_messaging_reception_t *rx);

/**************************************************************************//**
 * Fill complete parameters (after finished)
 *
 * @param[in]  rx            Pointer to the reception structure
 * @param[out] complete      Complete parameters
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_messaging_get_complete(cs_ras_client_messaging_reception_t *rx,
                                                 cs_ras_client_messaging_complete_t  *complete);

/**************************************************************************//**
 * Reception finished callback
 *
 * @param[in] rx                  Pointer to the reception structure
 * @param[in] status              Reception status (cause of stop)
 * @param[in] sc                  Status code of the reception
 * @param[in] recoverable         Reception is recoverable
 * @param[in] size                Size of the received data.
 * @param[in] last_arrived        True if last segment has been arrived.
 * @param[in] last_known_segment  Last know segment index.
 * @param[in] lost_segments       Bitfield of lost segments
 *****************************************************************************/
void cs_ras_client_messaging_reception_stopped(cs_ras_client_messaging_reception_t *rx,
                                               cs_ras_messaging_status_t           status,
                                               sl_status_t                         sc,
                                               bool                                recoverable,
                                               uint32_t                            size,
                                               bool                                last_arrived,
                                               uint8_t                             last_known_segment,
                                               uint64_t                            lost_segments);

/**************************************************************************//**
 * Segment received callback.
 *
 * @param[in] rx      Pointer to the reception structure.
 * @param[in] counter Segment counter
 * @return Status of the operation.
 *****************************************************************************/
void cs_ras_client_messaging_segment_received(cs_ras_client_messaging_reception_t *rx,
                                              cs_ras_ranging_counter_t            counter);

#ifdef __cplusplus
};
#endif

#endif // CS_RAS_CLIENT_MESSAGING_H
