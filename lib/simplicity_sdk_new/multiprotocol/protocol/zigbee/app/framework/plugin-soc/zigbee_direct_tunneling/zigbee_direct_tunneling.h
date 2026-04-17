/***************************************************************************//**
 * @file zigbee_direct_tunneling.h
 * @brief Zigbee Direct - Zigbee Direct tunneling code
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SILABS_ZIGBEE_DIRECT_TUNNELING_H
#define SILABS_ZIGBEE_DIRECT_TUNNELING_H

#include "sl_bluetooth.h"

#define LONG_SOURCE_NOT_INCLUDED 0x10
#define NPDU_INDEX_FLAGS 2
#define PACKET_LENGTH_INDEX 3
#define NWK_PACKET_INDEX 4
#define DESTINATION_NODE_ID_INDEX 6
#define NODE_ID_INDEX 8
#define INCOMING_EUI64_INDEX 12
#define INCOMING_COMMAND_ID_INDEX 20
#define ATTACHED_TLVS_INDEX 23
#define MAX_MTU 254 //maximum usable MTU size
#define NPDU_TLV_OVERHEAD 4
#define OUTGOING_NPDU_QUEUE_SIZE 1024
#define MAX_FLAT_PACKET_SIZE 127
#define TEMP_BUFFER_SIZE 54
#define TUNNEL_DATA_COMMAND_FRAME_SIZE 9
#define TUNNEL_DATA_AUX_FRAME_HEADER_SIZE 2
#define TUNNEL_DATA_FRAME_SIZE 35
#define ZIGBEE_APS_FRAME_CONTROL_TYPE_ACK         0x02u
#define ZIGBEE_APS_FRAME_CONTROL_TYPE_MASK        0x03u
#define BLE_CONNECTION_INDEX                   0
#define BLE_INDICATION_SLACK_MS                2
#define BLE_INDICATION_HEADER_SIZE             8
#define OUTGOING_INDICATION_OVERHEAD           12
#define COMMISSIONING_TLVS_OFFSET              22
#define SIGNAL_STRENGTH_DEFAULT                -40
#define LQI_DEFAULT                            255
#define MIN_OUTGOING_INDICATION_LENGTH         5
#define OUTGOING_INDICATION_START_INDEX        NPDU_TLV_OVERHEAD
#define MIN_TLV_TOTAL_LENGTH                 4
#define NWK_HEADER_SRC_EXT_PRESENT_OFFSET    5
#define APS_SEC_CTRL_OFFSET_FROM_LEN         11
#define APS_FRAME_CTRL_OFFSET_FROM_LEN       9
#define TLV_WRAPPER_OVERHEAD                 NPDU_TLV_OVERHEAD
#define OUTGOING_QUEUE_ENTRY_OVERHEAD        6
#define NWK_HEADER_SECURITY_BIT_MASK         0x02
#define NPDU_FLAGS_SECURITY_ENABLED          0x01
#define NPDU_FLAGS_SECURITY_DISABLED         0x00
#define NWK_HEADER_SECURITY_CLEAR_MASK       0xFD
#define TLV_LENGTH_FIELD_INDEX               1
#define TLV_HEADER_SIZE                      3

void sli_zigbee_direct_tunnel_write(uint8_t connection, byte_array *writeValue);

#endif // SILABS_ZIGBEE_DIRECT_TUNNELING_H
