/***************************************************************************//**
 * @file message_ipc_command_messages.c
 * @brief internal wrappers for 'message' ipc commands
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
#include "stack/internal/src/ipc/message_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_address_table_entry_is_active_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.address_table_entry_is_active.response.result = sli_zigbee_stack_address_table_entry_is_active(msg->data.address_table_entry_is_active.request.addressTableIndex);
}

void sli_zigbee_stack_get_address_table_info_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_address_table_info.response.result = sli_zigbee_stack_get_address_table_info(msg->data.get_address_table_info.request.addressTableIndex,
                                                                                             &msg->data.get_address_table_info.request.nodeId,
                                                                                             msg->data.get_address_table_info.request.eui64);
}

void sli_zigbee_stack_get_aps_ack_timeout_ms_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_aps_ack_timeout_ms.response.result = sli_zigbee_stack_get_aps_ack_timeout_ms();
}

void sli_zigbee_stack_get_extended_timeout_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_extended_timeout.response.result = sli_zigbee_stack_get_extended_timeout(msg->data.get_extended_timeout.request.remoteEui64);
}

void sli_zigbee_stack_get_multicast_table_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_multicast_table.response.result = sli_zigbee_stack_get_multicast_table();
}

void sli_zigbee_stack_get_multicast_table_size_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_multicast_table_size.response.result = sli_zigbee_stack_get_multicast_table_size();
}

void sli_zigbee_stack_maximum_aps_payload_length_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.maximum_aps_payload_length.response.result = sli_zigbee_stack_maximum_aps_payload_length();
}

void sli_zigbee_stack_pending_acked_messages_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.pending_acked_messages.response.result = sli_zigbee_stack_pending_acked_messages();
}

void sli_zigbee_stack_proxy_next_broadcast_from_long_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.proxy_next_broadcast_from_long.response.result = sli_zigbee_stack_proxy_next_broadcast_from_long(msg->data.proxy_next_broadcast_from_long.request.euiSource);
}

void sli_zigbee_stack_send_broadcast_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_broadcast.response.result = sli_zigbee_stack_send_broadcast(msg->data.send_broadcast.request.alias,
                                                                             msg->data.send_broadcast.request.destination,
                                                                             msg->data.send_broadcast.request.nwkSequence,
                                                                             &msg->data.send_broadcast.request.apsFrame,
                                                                             msg->data.send_broadcast.request.radius,
                                                                             msg->data.send_broadcast.request.messageTag,
                                                                             msg->data.send_broadcast.request.messageLength,
                                                                             msg->data.send_broadcast.request.message,
                                                                             &msg->data.send_broadcast.request.apsSequence);
}

void sli_zigbee_stack_send_multicast_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_multicast.response.result = sli_zigbee_stack_send_multicast(&msg->data.send_multicast.request.apsFrame,
                                                                             msg->data.send_multicast.request.radius,
                                                                             msg->data.send_multicast.request.broadcastAddr,
                                                                             msg->data.send_multicast.request.alias,
                                                                             msg->data.send_multicast.request.nwkSequence,
                                                                             msg->data.send_multicast.request.messageTag,
                                                                             msg->data.send_multicast.request.messageLength,
                                                                             msg->data.send_multicast.request.message,
                                                                             &msg->data.send_multicast.request.apsSequence);
}

void sli_zigbee_stack_send_reply_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_reply.response.result = sli_zigbee_stack_send_reply(msg->data.send_reply.request.destination,
                                                                     &msg->data.send_reply.request.apsFrame,
                                                                     msg->data.send_reply.request.messageLength,
                                                                     msg->data.send_reply.request.messageContents);
}

void sli_zigbee_stack_send_unicast_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_unicast.response.result = sli_zigbee_stack_send_unicast(msg->data.send_unicast.request.type,
                                                                         msg->data.send_unicast.request.indexOrDestination,
                                                                         &msg->data.send_unicast.request.apsFrame,
                                                                         msg->data.send_unicast.request.messageTag,
                                                                         msg->data.send_unicast.request.messageLength,
                                                                         msg->data.send_unicast.request.message,
                                                                         &msg->data.send_unicast.request.apsSequence);
}

void sli_zigbee_stack_set_address_table_info_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_address_table_info.response.result = sli_zigbee_stack_set_address_table_info(msg->data.set_address_table_info.request.addressTableIndex,
                                                                                             msg->data.set_address_table_info.request.eui64,
                                                                                             msg->data.set_address_table_info.request.id);
}

void sli_zigbee_stack_set_aps_ack_timeout_ms_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_aps_ack_timeout_ms(msg->data.set_aps_ack_timeout_ms.request.timeout);
}

void sli_zigbee_stack_set_extended_timeout_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_extended_timeout.response.result = sli_zigbee_stack_set_extended_timeout(msg->data.set_extended_timeout.request.remoteEui64,
                                                                                         msg->data.set_extended_timeout.request.extendedTimeout);
}

void sli_zigbee_stack_set_multicast_table_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_multicast_table.response.result = sli_zigbee_stack_set_multicast_table(&msg->data.set_multicast_table.request.entry);
}

void sli_zigbee_stack_set_multicast_table_size_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_multicast_table_size.response.result = sli_zigbee_stack_set_multicast_table_size(msg->data.set_multicast_table_size.request.size);
}

void sli_zigbee_stack_set_passive_ack_config_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_passive_ack_config.response.result = sli_zigbee_stack_set_passive_ack_config(msg->data.set_passive_ack_config.request.config,
                                                                                             msg->data.set_passive_ack_config.request.minAcksNeeded);
}

void sli_zigbee_stack_set_reply_fragment_data_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_reply_fragment_data(msg->data.set_reply_fragment_data.request.fragmentData);
}

void slxi_zigbee_stack_add_to_incoming_network_queue_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.add_to_incoming_network_queue.response.result = slxi_zigbee_stack_add_to_incoming_network_queue(msg->data.add_to_incoming_network_queue.request.rssi,
                                                                                                            msg->data.add_to_incoming_network_queue.request.lqi,
                                                                                                            &msg->data.add_to_incoming_network_queue.request.nwk_packet);
}

void slxi_zigbee_stack_network_send_command_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.network_send_command.response.result = slxi_zigbee_stack_network_send_command(msg->data.network_send_command.request.destination,
                                                                                          &msg->data.network_send_command.request.commandFrame,
                                                                                          msg->data.network_send_command.request.length,
                                                                                          msg->data.network_send_command.request.tryToInsertLongDest,
                                                                                          msg->data.network_send_command.request.destinationEui);
}

// public entrypoints

bool sl_zigbee_address_table_entry_is_active(uint8_t addressTableIndex)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.address_table_entry_is_active.request.addressTableIndex = addressTableIndex;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_address_table_entry_is_active_process_ipc_command, &msg);

  return msg.data.address_table_entry_is_active.response.result;
}

sl_status_t sl_zigbee_get_address_table_info(uint8_t addressTableIndex,
                                             sl_802154_short_addr_t *nodeId,
                                             sl_802154_long_addr_t eui64)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_address_table_info.request.addressTableIndex = addressTableIndex;

  if (nodeId != NULL) {
    msg.data.get_address_table_info.request.nodeId = *nodeId;
  }

  if (eui64 != NULL) {
    memmove(msg.data.get_address_table_info.request.eui64, eui64, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_address_table_info_process_ipc_command, &msg);

  if (nodeId != NULL) {
    *nodeId = msg.data.get_address_table_info.request.nodeId;
  }

  if (eui64 != NULL) {
    memmove(eui64, msg.data.get_address_table_info.request.eui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.get_address_table_info.response.result;
}

uint16_t sl_zigbee_get_aps_ack_timeout_ms(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_aps_ack_timeout_ms_process_ipc_command, &msg);

  return msg.data.get_aps_ack_timeout_ms.response.result;
}

sl_status_t sl_zigbee_get_extended_timeout(sl_802154_long_addr_t remoteEui64)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (remoteEui64 != NULL) {
    memmove(msg.data.get_extended_timeout.request.remoteEui64, remoteEui64, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_extended_timeout_process_ipc_command, &msg);

  if (remoteEui64 != NULL) {
    memmove(remoteEui64, msg.data.get_extended_timeout.request.remoteEui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.get_extended_timeout.response.result;
}

sl_zigbee_multicast_table_entry_t * sl_zigbee_get_multicast_table(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_multicast_table_process_ipc_command, &msg);

  return msg.data.get_multicast_table.response.result;
}

uint8_t sl_zigbee_get_multicast_table_size(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_multicast_table_size_process_ipc_command, &msg);

  return msg.data.get_multicast_table_size.response.result;
}

uint8_t sl_zigbee_maximum_aps_payload_length(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_maximum_aps_payload_length_process_ipc_command, &msg);

  return msg.data.maximum_aps_payload_length.response.result;
}

bool sl_zigbee_pending_acked_messages(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_pending_acked_messages_process_ipc_command, &msg);

  return msg.data.pending_acked_messages.response.result;
}

sl_status_t sl_zigbee_proxy_next_broadcast_from_long(const uint8_t *euiSource)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (euiSource != NULL) {
    memmove(msg.data.proxy_next_broadcast_from_long.request.euiSource, euiSource, sizeof(uint8_t) * EUI64_SIZE);
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_proxy_next_broadcast_from_long_process_ipc_command, &msg);

  return msg.data.proxy_next_broadcast_from_long.response.result;
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
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.send_broadcast.request.alias = alias;
  msg.data.send_broadcast.request.destination = destination;
  msg.data.send_broadcast.request.nwkSequence = nwkSequence;

  if (apsFrame != NULL) {
    msg.data.send_broadcast.request.apsFrame = *apsFrame;
  }

  msg.data.send_broadcast.request.radius = radius;
  msg.data.send_broadcast.request.messageTag = messageTag;
  msg.data.send_broadcast.request.messageLength = messageLength;

  if (messageLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector message length exceeds expected maximum
  }

  memmove(msg.data.send_broadcast.request.message, message, sizeof(uint8_t) * messageLength);

  if (apsSequence != NULL) {
    msg.data.send_broadcast.request.apsSequence = *apsSequence;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_broadcast_process_ipc_command, &msg);

  if (apsFrame != NULL) {
    *apsFrame = msg.data.send_broadcast.request.apsFrame;
  }

  if (apsSequence != NULL) {
    *apsSequence = msg.data.send_broadcast.request.apsSequence;
  }

  return msg.data.send_broadcast.response.result;
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
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (apsFrame != NULL) {
    msg.data.send_multicast.request.apsFrame = *apsFrame;
  }

  msg.data.send_multicast.request.radius = radius;
  msg.data.send_multicast.request.broadcastAddr = broadcastAddr;
  msg.data.send_multicast.request.alias = alias;
  msg.data.send_multicast.request.nwkSequence = nwkSequence;
  msg.data.send_multicast.request.messageTag = messageTag;
  msg.data.send_multicast.request.messageLength = messageLength;

  if (messageLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector message length exceeds expected maximum
  }

  memmove(msg.data.send_multicast.request.message, message, sizeof(uint8_t) * messageLength);

  if (apsSequence != NULL) {
    msg.data.send_multicast.request.apsSequence = *apsSequence;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_multicast_process_ipc_command, &msg);

  if (apsFrame != NULL) {
    *apsFrame = msg.data.send_multicast.request.apsFrame;
  }

  if (apsSequence != NULL) {
    *apsSequence = msg.data.send_multicast.request.apsSequence;
  }

  return msg.data.send_multicast.response.result;
}

sl_status_t sl_zigbee_send_reply(sl_802154_short_addr_t destination,
                                 sl_zigbee_aps_frame_t *apsFrame,
                                 uint8_t messageLength,
                                 uint8_t *messageContents)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.send_reply.request.destination = destination;

  if (apsFrame != NULL) {
    msg.data.send_reply.request.apsFrame = *apsFrame;
  }

  msg.data.send_reply.request.messageLength = messageLength;

  if (messageLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector messageContents length exceeds expected maximum
  }

  memmove(msg.data.send_reply.request.messageContents, messageContents, sizeof(uint8_t) * messageLength);
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_reply_process_ipc_command, &msg);

  if (apsFrame != NULL) {
    *apsFrame = msg.data.send_reply.request.apsFrame;
  }

  if (messageLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector messageContents length exceeds expected maximum
  }

  memmove(messageContents, msg.data.send_reply.request.messageContents, sizeof(uint8_t) * messageLength);
  return msg.data.send_reply.response.result;
}

sl_status_t sl_zigbee_send_unicast(sl_zigbee_outgoing_message_type_t type,
                                   uint16_t indexOrDestination,
                                   sl_zigbee_aps_frame_t *apsFrame,
                                   uint16_t messageTag,
                                   uint8_t messageLength,
                                   const uint8_t *message,
                                   uint8_t *apsSequence)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.send_unicast.request.type = type;
  msg.data.send_unicast.request.indexOrDestination = indexOrDestination;

  if (apsFrame != NULL) {
    msg.data.send_unicast.request.apsFrame = *apsFrame;
  }

  msg.data.send_unicast.request.messageTag = messageTag;
  msg.data.send_unicast.request.messageLength = messageLength;

  if (messageLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector message length exceeds expected maximum
  }

  memmove(msg.data.send_unicast.request.message, message, sizeof(uint8_t) * messageLength);

  if (apsSequence != NULL) {
    msg.data.send_unicast.request.apsSequence = *apsSequence;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_unicast_process_ipc_command, &msg);

  if (apsFrame != NULL) {
    *apsFrame = msg.data.send_unicast.request.apsFrame;
  }

  if (apsSequence != NULL) {
    *apsSequence = msg.data.send_unicast.request.apsSequence;
  }

  return msg.data.send_unicast.response.result;
}

sl_status_t sl_zigbee_set_address_table_info(uint8_t addressTableIndex,
                                             sl_802154_long_addr_t eui64,
                                             sl_802154_short_addr_t id)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_address_table_info.request.addressTableIndex = addressTableIndex;

  if (eui64 != NULL) {
    memmove(msg.data.set_address_table_info.request.eui64, eui64, sizeof(sl_802154_long_addr_t));
  }

  msg.data.set_address_table_info.request.id = id;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_address_table_info_process_ipc_command, &msg);

  if (eui64 != NULL) {
    memmove(eui64, msg.data.set_address_table_info.request.eui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.set_address_table_info.response.result;
}

void sl_zigbee_set_aps_ack_timeout_ms(uint16_t timeout)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_aps_ack_timeout_ms.request.timeout = timeout;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_aps_ack_timeout_ms_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_set_extended_timeout(sl_802154_long_addr_t remoteEui64,
                                           bool extendedTimeout)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (remoteEui64 != NULL) {
    memmove(msg.data.set_extended_timeout.request.remoteEui64, remoteEui64, sizeof(sl_802154_long_addr_t));
  }

  msg.data.set_extended_timeout.request.extendedTimeout = extendedTimeout;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_extended_timeout_process_ipc_command, &msg);

  if (remoteEui64 != NULL) {
    memmove(remoteEui64, msg.data.set_extended_timeout.request.remoteEui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.set_extended_timeout.response.result;
}

sl_status_t sl_zigbee_set_multicast_table(const sl_zigbee_multicast_table_entry_t *entry)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (entry != NULL) {
    msg.data.set_multicast_table.request.entry = *entry;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_multicast_table_process_ipc_command, &msg);

  return msg.data.set_multicast_table.response.result;
}

sl_status_t sl_zigbee_set_multicast_table_size(uint8_t size)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_multicast_table_size.request.size = size;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_multicast_table_size_process_ipc_command, &msg);

  return msg.data.set_multicast_table_size.response.result;
}

sl_status_t sl_zigbee_set_passive_ack_config(sl_passive_ack_config_enum_t config,
                                             uint8_t minAcksNeeded)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_passive_ack_config.request.config = config;
  msg.data.set_passive_ack_config.request.minAcksNeeded = minAcksNeeded;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_passive_ack_config_process_ipc_command, &msg);

  return msg.data.set_passive_ack_config.response.result;
}

void sl_zigbee_set_reply_fragment_data(uint16_t fragmentData)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_reply_fragment_data.request.fragmentData = fragmentData;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_reply_fragment_data_process_ipc_command, &msg);
}

sl_status_t slx_zigbee_add_to_incoming_network_queue(int8_t rssi,
                                                     uint8_t lqi,
                                                     const sl_nwk_packet_exchange_t *nwk_packet)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.add_to_incoming_network_queue.request.rssi = rssi;
  msg.data.add_to_incoming_network_queue.request.lqi = lqi;

  if (nwk_packet != NULL) {
    msg.data.add_to_incoming_network_queue.request.nwk_packet = *nwk_packet;
  }

  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_add_to_incoming_network_queue_process_ipc_command, &msg);

  return msg.data.add_to_incoming_network_queue.response.result;
}

bool slx_zigbee_network_send_command(sl_802154_short_addr_t destination,
                                     uint8_t *commandFrame,
                                     uint8_t length,
                                     bool tryToInsertLongDest,
                                     sl_802154_long_addr_t destinationEui)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.network_send_command.request.destination = destination;

  if (commandFrame != NULL) {
    msg.data.network_send_command.request.commandFrame = *commandFrame;
  }

  msg.data.network_send_command.request.length = length;
  msg.data.network_send_command.request.tryToInsertLongDest = tryToInsertLongDest;

  if (destinationEui != NULL) {
    memmove(msg.data.network_send_command.request.destinationEui, destinationEui, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_network_send_command_process_ipc_command, &msg);

  if (commandFrame != NULL) {
    *commandFrame = msg.data.network_send_command.request.commandFrame;
  }

  if (destinationEui != NULL) {
    memmove(destinationEui, msg.data.network_send_command.request.destinationEui, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.network_send_command.response.result;
}
