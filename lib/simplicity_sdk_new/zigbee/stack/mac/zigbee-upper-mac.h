/***************************************************************************//**
 * @file
 * @brief Upper MAC macros and prototypes. It provides the Zigbee MAC layer
 * interface to the unified-mac  upper layers.
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

#ifndef ZIGBEE_UPPER_MAC_H
#define ZIGBEE_UPPER_MAC_H

#include "multi-mac.h"
#include "upper-mac.h"

void sli_802154mac_transmit_complete_callback(sli_zigbee_packet_header_t header, sl_status_t status, uint8_t messageTag);
void sli_802154mac_packet_send_complete_callback(uint8_t mac_index, sl_status_t status, sli_zigbee_packet_header_t packet, uint8_t tag);
void sli_zigbee_set_mac_rx_on_when_idle_state(uint8_t macIndex,
                                              uint8_t networkIndex,
                                              sl_mac_rx_state_t rxOnWhenIdle);
sl_mac_rx_state_t sli_zigbee_get_mac_rx_on_when_idle_state(uint8_t macIndex, uint8_t networkIndex);

#ifdef MULTI_MAC_PRESENT
bool sli_zigbee_is_native_radio_mac(MAC_INDEX_PARAMETER_SOLO);
#else // !MULTI_MAC_PRESENT
#define sli_zigbee_is_native_radio_mac(MAC_INDEX_PARAMETER_SOLO) true
#endif  // MULTI_MAC_PRESENT

void sli_802154phy_radio_receive_complete_callback(sli_buffer_manager_buffer_t header);
bool sli_zigbee_packet_handoff_incoming_callback(sli_buffer_manager_buffer_t rawPacket, uint8_t index, void *data, uint8_t data_len);
#endif
