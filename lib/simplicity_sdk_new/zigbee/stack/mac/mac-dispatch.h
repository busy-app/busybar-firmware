/***************************************************************************//**
 * @file
 * @brief MAC dispatch macros and prototypes. It provides the MAC layer
 * interface to the upper layers.
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

#ifndef MAC_DISPATCH_H
#define MAC_DISPATCH_H

#include "upper-mac.h"

bool sli_802154mac_is_empty(void);
void sli_802154mac_init(void);
bool sli_802154mac_submit(sli_zigbee_packet_header_t header, sl_zigbee_transmit_priority_t priority);
sl_status_t sli_802154mac_upper_mac_submit(uint8_t mac_index,
                                           sli_zigbee_packet_header_t header,
                                           sl_zigbee_transmit_priority_t priority);

bool sli_802154mac_passthrough_handler(uint8_t* macHeader, uint8_t macPayloadLength);
sl_zigbee_mac_passthrough_type_t sli_802154mac_passthrough_message_type(uint8_t *macHeader, uint8_t macPayloadLength);
bool sli_802154mac_filter_match_check(sl_zigbee_mac_passthrough_type_t passthroughType,
                                      sl_zigbee_mac_filter_match_data_t* filterValueMatch,
                                      uint8_t* macHeader);
bool sli_802154mac_incoming_queue_is_empty(void);
void sli_802154_stack_cancel_polls(void);
void sli_802154_stack_purge_transmit_queue(void);
void sli_802154_stack_purge_incoming_queue(void);
bool sli_802154_stack_lower_macs_are_idle(void);
sl_mac_tx_options_bitmask_t sli_802154mac_prepare_tx_handler(sli_zigbee_packet_header_t packet,
                                                             uint8_t *flat_packet_buffer,
                                                             uint8_t mac_payload_offset,
                                                             uint8_t mac_index,
                                                             uint8_t nwk_index,
                                                             int8_t *tx_power);
#endif
