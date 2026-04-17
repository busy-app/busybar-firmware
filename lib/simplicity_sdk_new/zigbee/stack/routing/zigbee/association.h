/***************************************************************************//**
 * @file
 * @brief implementation of the ZigBee association and network formation.
 * Based on ZigBee Network Specification Draft Version 0.92.
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

#ifndef SILABS_ASSOCIATION_H
#define SILABS_ASSOCIATION_H

#include "stack/mac/multi-mac.h"
#include "event_queue/event-queue.h"

#define MAC_INDEX_FROM_HEADER sli_mac_header_mac_index(header),
#define MAC_INDEX_FROM_HEADER_SOLO sli_mac_header_mac_index(header)

#define ZIGBEE_RECOMMENDED_SCAN_DURATION 3  // R22, D.7

#define sli_zigbee_external_zigbee_state() (sli_zigbee_state & 0x0F)

// extern sl_802154_long_addr_t sli_zigbee_parent_eui64;             // moved to sl_zigbee.h
// extern sl_zigbee_node_type_t sli_zigbee_node_type;             // moved to sl_zigbee_stack.h

#define MAC_COORDINATOR_CAPABILITIES                                        \
  (CAPABILITY_ALTERNATE_PAN_COORDINATOR  /* we are a coordinator         */ \
   | MAC_ROUTER_CAPABILITIES)            /* plus router capabilities     */

#define MAC_ROUTER_CAPABILITIES                                             \
  (CAPABILITY_DEVICE_TYPE                /* we are a router              */ \
   | MAC_END_DEVICE_CAPABILITIES)        /* plus end device capabilities */

#define MAC_END_DEVICE_CAPABILITIES                                         \
  (CAPABILITY_POWER_SOURCE               /* we are mains-powered         */ \
   | CAPABILITY_RECEIVER_ON_WHEN_IDLE    /* our radio is on at all times */ \
   | MAC_SLEEPY_CAPABILITIES)            /* plus sleepy capabilities     */

#define MAC_SLEEPY_CAPABILITIES \
  (CAPABILITY_ALLOCATE_ADDRESS)          /* we want a two-byte address   */

#define sli_zigbee_send_rejoin_response(...) \
  sli_zigbee_send_network_rejoin_command(ZIGBEE_REJOIN_RESPONSE, __VA_ARGS__)

// Translating the node type into a capability byte.
// This array is actually in core/zigbee-stack.c to make it
// easier to link test programs.
extern uint8_t const sli_zigbee_capability_bytes[];
extern uint8_t sli_zigbee_tree_depth;

#ifdef MAC_TEST_COMMANDS_SUPPORT
extern bool sli_zigbee_enable_mac_certification_test_mode;
#endif  // MAC_TEST_COMMANDS_SUPPORT

uint8_t sli_zigbee_get_local_capabilities(void);

void sli_zigbee_process_incoming_beacon(sli_zigbee_packet_header_t header);
void sli_zigbee_association_handler(uint8_t mac_index, sli_zigbee_packet_header_t header);

void sli_zigbee_note_joining_change(void);

void sli_zigbee_note_join_authenticated(void);

// The security extends the timeout for joining if authentication is
// expected to take longer than the default.
void sli_zigbee_extend_join_timeout(void);
void sli_zigbee_extend_join_timeout_ms(uint32_t delayMs);

// If the MAC fails send the association request we time out immediately.
void sli_zigbee_association_request_not_sent(void);

// This is called with a non-NULL argument when forming or joining and
// with a NULL argument when the node or PAN id or the radio settings have
// changed.
void sli_zigbee_write_radio_and_network_tokens(uint8_t *extendedPanId, uint8_t nwkUpdateId);

void sli_zigbee_write_node_type_token(sl_zigbee_node_type_t nodeType);

void sli_zigbee_process_legacy_join_request(uint8_t mac_index,
                                            bool rejoin,
                                            sli_zigbee_packet_header_t header,
                                            uint8_t *networkFrame,
                                            uint8_t commandFrameIndex);

void sli_zigbee_process_join_response(sl_zigbee_join_method_t joinMethod,
                                      sli_zigbee_packet_header_t header,
                                      uint8_t commandFrameIndex);

// Returns a header only if 'reallySend' is false.  In other cases
// it sends the header to 'oldShortId'.

sli_zigbee_packet_header_t sli_zigbee_send_network_rejoin_command(uint8_t   cmd_id,
                                                                  sl_802154_long_addr_t longId,
                                                                  sl_802154_short_addr_t oldShortId,
                                                                  sl_802154_short_addr_t newShortId,
                                                                  bool useNwkSecurity,
                                                                  uint8_t status,
                                                                  bool reallySend);

void sli_zigbee_set_node_type(sl_zigbee_node_type_t nodeType);

void sli_zigbee_set_min_rssi_for_receiving_pkts(int8_t minRSSI);

// Used by the random id code after picking a new id.
void sli_zigbee_write_node_data(bool erase);
#define sli_zigbee_write_node_id_token() (sli_zigbee_write_node_data(false))
#define sli_zigbee_clear_radio_and_network_tokens() (sli_zigbee_write_node_data(true))

sl_status_t sli_zigbee_read_radio_tokens(void);

void sli_zigbee_finish_join_failure(void);
void sli_zigbee_note_join_failure_for_timeout(sl_status_t reason);
void sli_zigbee_set_join_poll_attemps_remaining(uint8_t attempts);
void sli_zigbee_set_join_method(sl_zigbee_join_method_t joinMethod);
sl_zigbee_join_method_t sli_zigbee_get_join_method(void);
void sli_zigbee_note_link_key_request(void);

extern sli_zigbee_event_t sli_zigbee_association_event;

// Callback for the ZDO for when it has initiated an energy scan.
void sli_zigbee_stack_energy_scan_handler(sli_buffer_manager_buffer_t results);

void sli_zigbee_set_parent(sl_802154_short_addr_t shortId, const sl_802154_long_addr_t longId);

sl_status_t sli_zigbee_join_network_internal(bool checkSecurityState,
                                             sl_802154_short_addr_t id,
                                             sl_zigbee_node_type_t nodeType,
                                             sl_zigbee_network_parameters_t *parameters);
void sli_zigbee_set_network_parameters(sl_802154_short_addr_t networkId,
                                       sl_802154_short_addr_t parentShort,
                                       const sl_802154_long_addr_t parentLong);
// update parrentID token
void sli_zigbee_update_parent_token(void);

void sli_zigbee_common_join_handler(uint8_t mac_index,
                                    bool rejoin,
                                    uint8_t network_command_id,
                                    uint8_t capabilities,
                                    sli_zigbee_packet_header_t header,
                                    uint8_t *network_frame,
                                    sl_zigbee_join_method_t join_method,
                                    void *auxJoinerTlvData);

#endif // SILABS_ASSOCIATION_H
