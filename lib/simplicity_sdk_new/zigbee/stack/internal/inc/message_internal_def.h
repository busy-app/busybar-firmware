/***************************************************************************//**
 * @file message_internal_def.h
 * @brief internal names for 'message' declarations
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
#ifndef MESSAGE_INTERNAL_DEF_H
#define MESSAGE_INTERNAL_DEF_H

#include "stack/include/message.h"

// Command Indirection

bool sli_zigbee_stack_address_table_entry_is_active(uint8_t addressTableIndex);

sl_status_t sli_zigbee_stack_get_address_table_info(uint8_t addressTableIndex,
                                                    sl_802154_short_addr_t *nodeId,
                                                    sl_802154_long_addr_t eui64);

uint16_t sli_zigbee_stack_get_aps_ack_timeout_ms(void);

sl_status_t sli_zigbee_stack_get_extended_timeout(sl_802154_long_addr_t remoteEui64);

sl_zigbee_multicast_table_entry_t * sli_zigbee_stack_get_multicast_table(void);

uint8_t sli_zigbee_stack_get_multicast_table_size(void);

uint8_t sli_zigbee_stack_maximum_aps_payload_length(void);

bool sli_zigbee_stack_pending_acked_messages(void);

sl_status_t sli_zigbee_stack_proxy_next_broadcast_from_long(const uint8_t *euiSource);

sl_status_t sli_zigbee_stack_send_broadcast(sl_802154_short_addr_t alias,
                                            sl_802154_short_addr_t destination,
                                            uint8_t nwkSequence,
                                            sl_zigbee_aps_frame_t *apsFrame,
                                            uint8_t radius,
                                            uint16_t messageTag,
                                            uint8_t messageLength,
                                            const uint8_t *message,
                                            uint8_t *apsSequence);

sl_status_t sli_zigbee_stack_send_multicast(sl_zigbee_aps_frame_t *apsFrame,
                                            uint8_t radius,
                                            uint16_t broadcastAddr,
                                            sl_802154_short_addr_t alias,
                                            uint8_t nwkSequence,
                                            uint16_t messageTag,
                                            uint8_t messageLength,
                                            const uint8_t *message,
                                            uint8_t *apsSequence);

sl_status_t sli_zigbee_stack_send_reply(sl_802154_short_addr_t destination,
                                        sl_zigbee_aps_frame_t *apsFrame,
                                        uint8_t messageLength,
                                        uint8_t *messageContents);

sl_status_t sli_zigbee_stack_send_unicast(sl_zigbee_outgoing_message_type_t type,
                                          uint16_t indexOrDestination,
                                          sl_zigbee_aps_frame_t *apsFrame,
                                          uint16_t messageTag,
                                          uint8_t messageLength,
                                          const uint8_t *message,
                                          uint8_t *apsSequence);

sl_status_t sli_zigbee_stack_set_address_table_info(uint8_t addressTableIndex,
                                                    sl_802154_long_addr_t eui64,
                                                    sl_802154_short_addr_t id);

void sli_zigbee_stack_set_aps_ack_timeout_ms(uint16_t timeout);

sl_status_t sli_zigbee_stack_set_extended_timeout(sl_802154_long_addr_t remoteEui64,
                                                  bool extendedTimeout);

sl_status_t sli_zigbee_stack_set_multicast_table(const sl_zigbee_multicast_table_entry_t *entry);

sl_status_t sli_zigbee_stack_set_multicast_table_size(uint8_t size);

sl_status_t sli_zigbee_stack_set_passive_ack_config(sl_passive_ack_config_enum_t config,
                                                    uint8_t minAcksNeeded);

void sli_zigbee_stack_set_reply_fragment_data(uint16_t fragmentData);

sl_status_t slxi_zigbee_stack_add_to_incoming_network_queue(int8_t rssi,
                                                            uint8_t lqi,
                                                            const sl_nwk_packet_exchange_t *nwk_packet);

bool slxi_zigbee_stack_network_send_command(sl_802154_short_addr_t destination,
                                            uint8_t *commandFrame,
                                            uint8_t length,
                                            bool tryToInsertLongDest,
                                            sl_802154_long_addr_t destinationEui);

// Callback Indirection

void sli_zigbee_stack_gpep_incoming_message_handler(sl_zigbee_gp_params_t *params);

void sli_zigbee_stack_id_conflict_handler(sl_802154_short_addr_t conflictingId);

void sli_zigbee_stack_incoming_many_to_one_route_request_handler(sl_802154_short_addr_t source,
                                                                 sl_802154_long_addr_t longId,
                                                                 uint8_t cost);

void sli_zigbee_stack_incoming_message_handler(sl_zigbee_incoming_message_type_t type,
                                               sl_zigbee_aps_frame_t *apsFrame,
                                               sl_zigbee_rx_packet_info_t *packetInfo,
                                               uint8_t messageLength,
                                               uint8_t *message);

void sli_zigbee_stack_incoming_network_status_handler(uint8_t errorCode,
                                                      sl_802154_short_addr_t target);

void sli_zigbee_stack_incoming_route_error_handler(sl_status_t status,
                                                   sl_802154_short_addr_t target);

void sli_zigbee_stack_incoming_route_record_handler(sl_zigbee_rx_packet_info_t *packetInfo,
                                                    uint8_t relayCount,
                                                    uint8_t *relayList);

void sli_zigbee_stack_message_sent_handler(sl_status_t status,
                                           sl_zigbee_outgoing_message_type_t type,
                                           uint16_t indexOrDestination,
                                           sl_zigbee_aps_frame_t *apsFrame,
                                           uint16_t messageTag,
                                           uint8_t messageLength,
                                           uint8_t *message);

void sli_zigbee_stack_override_incoming_route_record_handler(sl_zigbee_rx_packet_info_t *packetInfo,
                                                             uint8_t relayCount,
                                                             uint8_t *relayList,
                                                             bool *consumed);

void sli_zigbee_stack_post_incoming_packet_filter_cb(sl_zigbee_zigbee_packet_type_t packetType,
                                                     uint8_t *packetData,
                                                     uint8_t size_p,
                                                     uint8_t *data,
                                                     uint8_t size_d,
                                                     sl_zigbee_packet_action_t action);

void sli_zigbee_stack_post_outgoing_packet_filter_cb(sl_zigbee_zigbee_packet_type_t packetType,
                                                     uint8_t *packetData,
                                                     uint8_t size_p,
                                                     uint8_t *data,
                                                     uint8_t size_d,
                                                     sl_zigbee_packet_action_t action);

void sli_zigbee_stack_redirect_outgoing_message_handler(uint8_t mac_index,
                                                        uint8_t packet_length,
                                                        uint8_t *packet_contents,
                                                        uint8_t priority);

#endif // MESSAGE_INTERNAL_DEF_H
