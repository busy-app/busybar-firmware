/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include PLATFORM_HEADER
#include "stack/include/sl_zigbee_types.h"

/** @brief A protocol layer packet has been received by the stack.
 *
 * This is called when the stack receives a packet that is meant
 * for one of the protocol layers specified in ::sl_zigbee_zigbee_packet_type_t.
 *
 * The packetType argument is one of the values of the
 * ::sl_zigbee_zigbee_packet_type_t enum. If the stack receives an 802.15.4
 * MAC beacon, it will call this function with the packetType argument
 * set to ::SL_ZIGBEE_ZIGBEE_PACKET_TYPE_BEACON.
 *
 * The implementation of this callback may alter the data contained in
 * packetData, modify options and flags in the auxillary data, or
 * consume the packet itself, either sending the message, or discarding
 * it as it sees fit.
 *
 * @param packetType The type of packet received. See
 * ::sl_zigbee_zigbee_packet_type_t.
 * @param packetData A flat buffer containing the packet contents
 * the buffer starts with the command id followed by the payload
 * @param size_p a pointer to the size of the packet data
 * @param data This is a pointer to auxiliary data for the command.
 * @param size_d The size of the auxiliary data.
 *
 * @return a ::sl_zigbee_packet_action_t indicating what action should be taken for
 * the packet, SL_ZIGBEE_ACCEPT_PACKET, SL_ZIGBEE_DROP_PACKET, or SL_ZIGBEE_MANGLE_PACKET
 */
WEAK(sl_zigbee_packet_action_t sl_zigbee_pre_incoming_packet_filter_cb(sl_zigbee_zigbee_packet_type_t packetType,
                                                                       uint8_t* packetData,
                                                                       uint8_t* size_p,
                                                                       void *data,
                                                                       uint8_t size_d))
{
  (void)packetType;
  (void)packetData;
  (void)size_p;
  (void)data;
  (void)size_d;
  return SL_ZIGBEE_ACCEPT_PACKET;
}

/** @brief The stack is preparing to send a protocol layer packet.
 *
 * This is called when the stack is preparing to send a packet from one
 * of the protocol layers specified in ::sl_zigbee_zigbee_packet_type_t.
 *
 * The packetType argument is one of the values of the
 * ::sl_zigbee_zigbee_packet_type_t enum. If the stack is preparing to send an
 * 802.15.4 MAC beacon, it will call this function with the packetType
 * argument set to ::SL_ZIGBEE_ZIGBEE_PACKET_TYPE_BEACON.
 *
 * The implementation of this callback may alter the data contained in
 * packetData, modify options and flags in the auxillary data, or
 * consume the packet itself, either sending the message, or discarding
 * it as it sees fit.
 *
 * @param packetType The type of packet received. See
 * ::sl_zigbee_zigbee_packet_type_t.
 * @param packetData A flat buffer containing the packet contents
 * the buffer starts with the command id followed by the payload
 * @param size_p a pointer to the size of the packet data
 * @param data This is a pointer to auxiliary data for the command. ZDO
 * commands pass the ::sl_zigbee_aps_frame_t associated with the packet here. Otherwise,
 * this value is NULL.
 * @param size_d The size of the auxiliary data.
 *
 * @return a ::sl_zigbee_packet_action_t indicating what action should be taken for
 * the packet, SL_ZIGBEE_ACCEPT_PACKET, SL_ZIGBEE_DROP_PACKET, or SL_ZIGBEE_MANGLE_PACKET
 */
WEAK(sl_zigbee_packet_action_t sl_zigbee_pre_outgoing_packet_filter_cb(sl_zigbee_zigbee_packet_type_t packetType,
                                                                       uint8_t* packetData,
                                                                       uint8_t* size_p,
                                                                       void* data,
                                                                       uint8_t size_d))
{
  (void)packetType;
  (void)packetData;
  (void)size_p;
  (void)data;
  (void)size_d;
  return SL_ZIGBEE_ACCEPT_PACKET;
}
