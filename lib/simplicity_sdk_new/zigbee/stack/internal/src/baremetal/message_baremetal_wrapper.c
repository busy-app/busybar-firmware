/***************************************************************************//**
 * @file message_baremetal_wrapper.c
 * @brief internal implementations for 'message' as a thin-wrapper
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

bool sl_zigbee_address_table_entry_is_active(uint8_t addressTableIndex)
{
  return sli_zigbee_stack_address_table_entry_is_active(addressTableIndex);
}

sl_status_t sl_zigbee_get_address_table_info(uint8_t addressTableIndex,
                                             sl_802154_short_addr_t *nodeId,
                                             sl_802154_long_addr_t eui64)
{
  return sli_zigbee_stack_get_address_table_info(addressTableIndex,
                                                 nodeId,
                                                 eui64);
}

uint16_t sl_zigbee_get_aps_ack_timeout_ms(void)
{
  return sli_zigbee_stack_get_aps_ack_timeout_ms();
}

sl_status_t sl_zigbee_get_extended_timeout(sl_802154_long_addr_t remoteEui64)
{
  return sli_zigbee_stack_get_extended_timeout(remoteEui64);
}

sl_zigbee_multicast_table_entry_t * sl_zigbee_get_multicast_table(void)
{
  return sli_zigbee_stack_get_multicast_table();
}

uint8_t sl_zigbee_get_multicast_table_size(void)
{
  return sli_zigbee_stack_get_multicast_table_size();
}

uint8_t sl_zigbee_maximum_aps_payload_length(void)
{
  return sli_zigbee_stack_maximum_aps_payload_length();
}

bool sl_zigbee_pending_acked_messages(void)
{
  return sli_zigbee_stack_pending_acked_messages();
}

sl_status_t sl_zigbee_proxy_next_broadcast_from_long(const uint8_t *euiSource)
{
  return sli_zigbee_stack_proxy_next_broadcast_from_long(euiSource);
}

sl_status_t sl_zigbee_send_broadcast(sl_802154_short_addr_t alias,
                                     sl_802154_short_addr_t destination,
                                     uint8_t nwkSequence,
                                     sl_zigbee_aps_frame_t *apsFrame,
                                     uint8_t radius,
                                     uint16_t messageTag,
                                     uint8_t messageLength,
                                     const uint8_t *message,
                                     uint8_t *apsSequence)
{
  return sli_zigbee_stack_send_broadcast(alias,
                                         destination,
                                         nwkSequence,
                                         apsFrame,
                                         radius,
                                         messageTag,
                                         messageLength,
                                         message,
                                         apsSequence);
}

sl_status_t sl_zigbee_send_multicast(sl_zigbee_aps_frame_t *apsFrame,
                                     uint8_t radius,
                                     uint16_t broadcastAddr,
                                     sl_802154_short_addr_t alias,
                                     uint8_t nwkSequence,
                                     uint16_t messageTag,
                                     uint8_t messageLength,
                                     const uint8_t *message,
                                     uint8_t *apsSequence)
{
  return sli_zigbee_stack_send_multicast(apsFrame,
                                         radius,
                                         broadcastAddr,
                                         alias,
                                         nwkSequence,
                                         messageTag,
                                         messageLength,
                                         message,
                                         apsSequence);
}

sl_status_t sl_zigbee_send_reply(sl_802154_short_addr_t destination,
                                 sl_zigbee_aps_frame_t *apsFrame,
                                 uint8_t messageLength,
                                 uint8_t *messageContents)
{
  return sli_zigbee_stack_send_reply(destination,
                                     apsFrame,
                                     messageLength,
                                     messageContents);
}

sl_status_t sl_zigbee_send_unicast(sl_zigbee_outgoing_message_type_t type,
                                   uint16_t indexOrDestination,
                                   sl_zigbee_aps_frame_t *apsFrame,
                                   uint16_t messageTag,
                                   uint8_t messageLength,
                                   const uint8_t *message,
                                   uint8_t *apsSequence)
{
  return sli_zigbee_stack_send_unicast(type,
                                       indexOrDestination,
                                       apsFrame,
                                       messageTag,
                                       messageLength,
                                       message,
                                       apsSequence);
}

sl_status_t sl_zigbee_set_address_table_info(uint8_t addressTableIndex,
                                             sl_802154_long_addr_t eui64,
                                             sl_802154_short_addr_t id)
{
  return sli_zigbee_stack_set_address_table_info(addressTableIndex,
                                                 eui64,
                                                 id);
}

void sl_zigbee_set_aps_ack_timeout_ms(uint16_t timeout)
{
  sli_zigbee_stack_set_aps_ack_timeout_ms(timeout);
}

sl_status_t sl_zigbee_set_extended_timeout(sl_802154_long_addr_t remoteEui64,
                                           bool extendedTimeout)
{
  return sli_zigbee_stack_set_extended_timeout(remoteEui64,
                                               extendedTimeout);
}

sl_status_t sl_zigbee_set_multicast_table(const sl_zigbee_multicast_table_entry_t *entry)
{
  return sli_zigbee_stack_set_multicast_table(entry);
}

sl_status_t sl_zigbee_set_multicast_table_size(uint8_t size)
{
  return sli_zigbee_stack_set_multicast_table_size(size);
}

sl_status_t sl_zigbee_set_passive_ack_config(sl_passive_ack_config_enum_t config,
                                             uint8_t minAcksNeeded)
{
  return sli_zigbee_stack_set_passive_ack_config(config,
                                                 minAcksNeeded);
}

void sl_zigbee_set_reply_fragment_data(uint16_t fragmentData)
{
  sli_zigbee_stack_set_reply_fragment_data(fragmentData);
}

sl_status_t slx_zigbee_add_to_incoming_network_queue(int8_t rssi,
                                                     uint8_t lqi,
                                                     const sl_nwk_packet_exchange_t *nwk_packet)
{
  return slxi_zigbee_stack_add_to_incoming_network_queue(rssi,
                                                         lqi,
                                                         nwk_packet);
}

bool slx_zigbee_network_send_command(sl_802154_short_addr_t destination,
                                     uint8_t *commandFrame,
                                     uint8_t length,
                                     bool tryToInsertLongDest,
                                     sl_802154_long_addr_t destinationEui)
{
  return slxi_zigbee_stack_network_send_command(destination,
                                                commandFrame,
                                                length,
                                                tryToInsertLongDest,
                                                destinationEui);
}
