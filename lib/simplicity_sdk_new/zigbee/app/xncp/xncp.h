/***************************************************************************//**
 * @file
 * @brief Programmable NCP code.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SILABS_XNCP_H
#define SILABS_XNCP_H

#include PLATFORM_HEADER
#include "stack/include/sl_zigbee.h"
#include "app/util/ezsp/ezsp-protocol.h"

#include "hal/hal.h"

// A legacy EZSP frame is defined as follows:
//   <sequence:1>
//   <frame control:1>
//   <frame ID:1>
//   <parameters:0-120>

// An extended EZSP frame (EZSP Version 8) is defined as follows:
//   <sequence:1>
//   <frame control:2>
//   <frame ID:2>
//   <parameters:0-120>

// Since we use the first byte of the parameters for the length,
// the largest possible custom EZSP frame payload is 119 bytes.
#define SL_ZIGBEE_MAX_CUSTOM_EZSP_MESSAGE_PAYLOAD 119

//------------------------------------------------------------------------------
// Callbacks

sl_status_t sl_zigbee_af_xncp_incoming_custom_frame_cb(uint8_t messageLength,
                                                       uint8_t *messagePayload,
                                                       uint8_t *replyPayloadLength,
                                                       uint8_t *replyPayload);

bool sl_zigbee_af_xncp_incoming_message_cb(sl_zigbee_incoming_message_type_t type,
                                           sl_zigbee_aps_frame_t *apsFrame,
                                           sl_zigbee_rx_packet_info_t *packetInfo,
                                           uint8_t messageLength,
                                           uint8_t *message);

bool sl_zigbee_af_xncp_permit_ncp_to_host_frame_cb(uint16_t frameId,
                                                   uint8_t payloadLength,
                                                   uint8_t *payload);

bool sl_zigbee_af_xncp_permit_host_to_ncp_frame_cb(uint16_t frameId,
                                                   uint8_t payloadLength,
                                                   uint8_t *payload);

//------------------------------------------------------------------------------
// Public APIs

sl_status_t sl_zigbee_af_xncp_send_custom_ezsp_message(uint8_t length, uint8_t *payload);

//------------------------------------------------------------------------------
// Private APIs

sl_status_t sli_zigbee_xncp_init(void);
bool sli_zigbee_xncp_allow_incoming_ezsp_command(void);
bool sli_zigbee_xncp_allow_outgoing_ezsp_callback(uint8_t *callback,
                                                  uint8_t callbackLength);
bool sli_zigbee_af_xncp_incoming_message_callback(sl_zigbee_incoming_message_type_t type,
                                                  sl_zigbee_aps_frame_t *apsFrame,
                                                  sl_zigbee_rx_packet_info_t *packetInfo,
                                                  uint8_t messageLength,
                                                  uint8_t *message);
void sli_zigbee_xncp_handle_incoming_custom_ezsp_message(void);
void sli_zigbee_xncp_handle_get_info_command(void);

#endif // SILABS_XNCP_H
