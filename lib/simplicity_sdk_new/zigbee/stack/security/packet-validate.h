/***************************************************************************//**
 * @file
 * @brief Defines some sanity check functions for checking that packets
 *   at various level of the stack are well-formed and can be safetly processed.
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

#ifndef SL_ZIGBEE_PACKET_VALIDATE_H_
#define SL_ZIGBEE_PACKET_VALIDATE_H_

#include "stack/include/sl_zigbee_types_internal.h"

bool sli_zigbee_packet_validate_check(sli_zigbee_packet_validate_type_t type, sli_zigbee_packet_header_t header);

// It performs a preliminary sanity check of the network packet size by looking
// at the network frame and computing the expected (minimum) packet size
// associated with the network frame. The computed value also includes the size
// related to the MAC frame. It performs some consistency checks on some fields
// such as the rleayIndex and relayCount. It returns true if these consistency
// checks are passed and the message buffer length is greater or equal than the
// computed value, otherwise it returns false.
// At this check level we only want to verify that the network packet size meets
// the minimum requirements, therefore we validate packets with an empty
// payload. We leave the decision of discarding or accepting an empty payload
// network packet to the processZigbeePacket() function.
#define sli_zigbee_network_packetis_valid(header) \
  sli_zigbee_packet_validate_check(PACKET_VALIDATE_NWK, (header))

// It performs a preliminary check of the APS packet size by looking at the APS
// frame and computing the expected (minimum) packet size associated with the
// APS frame. The computed value also includes the size related to the MAC frame
// and the network frame. It returns true if the message buffer length is
// greater or equal than the computed value, otherwise it returns false
#define sli_zigbee_aps_packet_is_valid(header) \
  sli_zigbee_packet_validate_check(PACKET_VALIDATE_APS, (header))

// This function returns true if the MAC command has a valid size, i.e., the
// payload contains at least as many bytes as requested by the MAC command,
// otherwise it returns false.
#define sli_802154mac_command_packet_is_valid(header) \
  sli_zigbee_packet_validate_check(PACKET_VALIDATE_MAC_COMMAND, (header))

// This function returns true if the network command packet has a valid size,
// i.e., the payload contains at least as many bytes as requested by the network
// command, otherwise it returns false. If network security is ON, this function
// must be called after the packet has been decrypted. It also assumes that it
// is called only if the packet is network command packet.
#define sli_zigbee_network_command_packet_is_valid(header) \
  sli_zigbee_packet_validate_check(PACKET_VALIDATE_NWK_COMMAND, (header))

// This function returns true if the APS command packet has a valid size, i.e.,
// the payload contains at least as many bytes as requested by a specific APS
// command, otherwise it returns false. It assumes that it is called only for
// APS command packets.
#define sli_zigbee_aps_command_packet_is_valid(header) \
  sli_zigbee_packet_validate_check(PACKET_VALIDATE_APS_COMMAND, (header))

extern const sl_zigbee_library_status_t sli_zigbee_packet_validate_library;

uint8_t sli_zigbee_packet_validate_library_info(void);

sl_status_t sli_zigbee_stack_set_packet_validate_library_state(uint16_t state);

#endif // SL_ZIGBEE_PACKET_VALIDATE_H_
