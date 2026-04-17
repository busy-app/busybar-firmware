/***************************************************************************//**
 * @file raw-message-baremetal-callbacks.c
 * @brief internal dispatch for 'raw-message' callbacks as a thin-wrapper
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
// automatically generated from raw-message.h.  Do not manually edit
#include "stack/include/raw-message.h"
#include "stack/internal/inc/raw-message-internal-def.h"

void sli_802154_stack_filter_match_message_handler(sl_zigbee_mac_filter_match_data_t filterValueMatch,
                                                   uint8_t legacyPassthroughType,
                                                   sl_zigbee_rx_packet_info_t *packetInfo,
                                                   uint8_t messageLength,
                                                   uint8_t *messageContents)
{
  sl_802154_filter_match_message_handler(filterValueMatch,
                                         legacyPassthroughType,
                                         packetInfo,
                                         messageLength,
                                         messageContents);
}

void sli_802154_stack_passthrough_message_handler(sl_zigbee_mac_passthrough_type_t messageType,
                                                  sl_zigbee_rx_packet_info_t *packetInfo,
                                                  uint8_t messageLength,
                                                  uint8_t *messageContents)
{
  sl_802154_passthrough_message_handler(messageType,
                                        packetInfo,
                                        messageLength,
                                        messageContents);
}

void sli_zigbee_stack_raw_transmit_complete_handler(uint8_t messageLength,
                                                    uint8_t *messageContents,
                                                    sl_status_t status,
                                                    uint8_t messageTag)
{
  sl_zigbee_raw_transmit_complete_handler(messageLength,
                                          messageContents,
                                          status,
                                          messageTag);
}
