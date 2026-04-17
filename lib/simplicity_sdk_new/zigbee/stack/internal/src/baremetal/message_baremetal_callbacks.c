/***************************************************************************//**
 * @file message_baremetal_callbacks.c
 * @brief internal dispatch for 'message' callbacks as a thin-wrapper
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from message.h.  Do not manually edit
#include "stack/include/message.h"
#include "stack/internal/inc/message_internal_def.h"

void sli_zigbee_stack_gpep_incoming_message_handler(sl_zigbee_gp_params_t *params)
{
  sl_zigbee_gpep_incoming_message_handler(params);
}

void sli_zigbee_stack_id_conflict_handler(sl_802154_short_addr_t conflictingId)
{
  sl_zigbee_id_conflict_handler(conflictingId);
}

void sli_zigbee_stack_incoming_many_to_one_route_request_handler(sl_802154_short_addr_t source,
                                                                 sl_802154_long_addr_t longId,
                                                                 uint8_t cost)
{
  sl_zigbee_incoming_many_to_one_route_request_handler(source,
                                                       longId,
                                                       cost);
}

void sli_zigbee_stack_incoming_message_handler(sl_zigbee_incoming_message_type_t type,
                                               sl_zigbee_aps_frame_t *apsFrame,
                                               sl_zigbee_rx_packet_info_t *packetInfo,
                                               uint8_t messageLength,
                                               uint8_t *message)
{
  sl_zigbee_incoming_message_handler(type,
                                     apsFrame,
                                     packetInfo,
                                     messageLength,
                                     message);
}

void sli_zigbee_stack_incoming_network_status_handler(uint8_t errorCode,
                                                      sl_802154_short_addr_t target)
{
  sl_zigbee_incoming_network_status_handler(errorCode,
                                            target);
}

void sli_zigbee_stack_incoming_route_error_handler(sl_status_t status,
                                                   sl_802154_short_addr_t target)
{
  sl_zigbee_incoming_route_error_handler(status,
                                         target);
}

void sli_zigbee_stack_incoming_route_record_handler(sl_zigbee_rx_packet_info_t *packetInfo,
                                                    uint8_t relayCount,
                                                    uint8_t *relayList)
{
  sl_zigbee_incoming_route_record_handler(packetInfo,
                                          relayCount,
                                          relayList);
}

void sli_zigbee_stack_message_sent_handler(sl_status_t status,
                                           sl_zigbee_outgoing_message_type_t type,
                                           uint16_t indexOrDestination,
                                           sl_zigbee_aps_frame_t *apsFrame,
                                           uint16_t messageTag,
                                           uint8_t messageLength,
                                           uint8_t *message)
{
  sl_zigbee_message_sent_handler(status,
                                 type,
                                 indexOrDestination,
                                 apsFrame,
                                 messageTag,
                                 messageLength,
                                 message);
}

void sli_zigbee_stack_override_incoming_route_record_handler(sl_zigbee_rx_packet_info_t *packetInfo,
                                                             uint8_t relayCount,
                                                             uint8_t *relayList,
                                                             bool *consumed)
{
  sl_zigbee_override_incoming_route_record_handler(packetInfo,
                                                   relayCount,
                                                   relayList,
                                                   consumed);
}

void sli_zigbee_stack_post_incoming_packet_filter_cb(sl_zigbee_zigbee_packet_type_t packetType,
                                                     uint8_t *packetData,
                                                     uint8_t size_p,
                                                     uint8_t *data,
                                                     uint8_t size_d,
                                                     sl_zigbee_packet_action_t action)
{
  sl_zigbee_post_incoming_packet_filter_cb(packetType,
                                           packetData,
                                           size_p,
                                           data,
                                           size_d,
                                           action);
}

void sli_zigbee_stack_post_outgoing_packet_filter_cb(sl_zigbee_zigbee_packet_type_t packetType,
                                                     uint8_t *packetData,
                                                     uint8_t size_p,
                                                     uint8_t *data,
                                                     uint8_t size_d,
                                                     sl_zigbee_packet_action_t action)
{
  sl_zigbee_post_outgoing_packet_filter_cb(packetType,
                                           packetData,
                                           size_p,
                                           data,
                                           size_d,
                                           action);
}

void sli_zigbee_stack_redirect_outgoing_message_handler(uint8_t mac_index,
                                                        uint8_t packet_length,
                                                        uint8_t *packet_contents,
                                                        uint8_t priority)
{
  sl_zigbee_redirect_outgoing_message_handler(mac_index,
                                              packet_length,
                                              packet_contents,
                                              priority);
}
