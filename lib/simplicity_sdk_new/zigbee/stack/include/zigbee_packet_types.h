/***************************************************************************//**
 * @file zigbee_packet_types.h
 * @brief ZigBee-specific header and application support sublayer definitions that cross the stack/application boundary.

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

#ifndef SILABS_ZIGBEE_PACKET_TYPES_H
#define SILABS_ZIGBEE_PACKET_TYPES_H

// Security Control bytes

#define ZIGBEE_SECURITY_CONTROL_LEVEL_MASK          0x07u

// Security frames are added for four different reasons.

#define ZIGBEE_SECURITY_CONTROL_KEY_IDENTIFIER_MASK 0x18u
#define ZIGBEE_SECURITY_CONTROL_LINK_KEY            0x00u
#define ZIGBEE_SECURITY_CONTROL_NETWORK_KEY         0x08u
#define ZIGBEE_SECURITY_CONTROL_KEY_TRANSPORT_KEY   0x10u
#define ZIGBEE_SECURITY_CONTROL_KEY_LOAD_KEY        0x18u

// This bit indicates whether or not there is a source address.
#define ZIGBEE_SECURITY_CONTROL_HAVE_EXTENDED_NONCE 0x20u

// This is the option bit that can be set in a sl_zigbee_aps_option_t variable
// by the app to indicate to the stack that APS Encryption should be used
// for sending a APS Data message.
#define SL_ZIGBEE_APS_OPTION_SECURITY  0x00020

// The various types of keys sent in APS_COMMAND_TRANSPORT_KEY messages.
// The messages start with
//  <frame control:1>
//  <key type:1>
// and then a key as formatted below.  For master keys and network keys
// the source is the trust center.
enum {
  // <network key:16> <key sequence:1> <destination:8> <source:8>
  KEY_TRANSPORT_RESIDENTIAL_NETWORK_KEY   = 1,

  // <link key:16> <partner address:8> <initiator flag:1>
  // The initiator flag is 1 if the message recipient requested this
  // key and 0 otherwise.
  KEY_TRANSPORT_APPLICATION_LINK_KEY      = 3,

  // <tc link key:16> <destination:8> <source:8>
  KEY_TRANSPORT_TRUST_CENTER_LINK_KEY     = 4,

  KEY_TRANSPORT_INVALID_KEY               = 6,

  // Zigbee Direct Specific Key Types
  KEY_TRANSPORT_EPHEMERAL_GLOBAL_LINK_KEY = 0xB0,
  KEY_TRANSPORT_EPHEMERAL_UNIQUE_LINK_KEY = 0xB1,
  KEY_TRANSPORT_BASIC_AUTHORIZATION_KEY = 0xB2,
  KEY_TRANSPORT_ADMINISTRATIVE_AUTHORIZATION_KEY = 0xB3,
};

// Key types in APS commands
enum {
  APS_COMMAND_TRANSPORT_KEY = 5,
  APS_COMMAND_UPDATE_DEVICE,
  APS_COMMAND_REMOVE_DEVICE,
  APS_COMMAND_REQUEST_KEY,
  APS_COMMAND_SWITCH_KEY,
  APS_COMMAND_EA_INIT_CHALLENGE,
  APS_COMMAND_EA_RESP_CHALLENGE,
  APS_COMMAND_EA_INIT_MAC_DATA,
  APS_COMMAND_EA_RESP_MAC_DATA,
  APS_COMMAND_TUNNEL_DATA,
  APS_COMMAND_VERIFY_KEY,
  APS_COMMAND_VERIFY_KEY_CONFIRM
};

// Options for sending APS commands and keys (sli_zigbee_send_aps_command_options_t)
enum {
  ENCRYPTION_NONE             = 0x00,
  ENCRYPTION_NETWORK          = 0x01,
  ENCRYPTION_APS              = 0x02,
  SEND_APS_COMMAND_TO_CHILD   = 0x04,
  SEND_APS_COMMAND_USE_TUNNEL = 0x08,
  SEND_APS_COMMAND_NULL_KEY   = 0x10,
  SEND_APS_COMMAND_ALT_NWK_KEY= 0x20,
  APP_KEY_INITIATOR           = 0x40
};

#define hasNwkEncryption(packetSecurity) \
  ((packetSecurity & ENCRYPTION_NETWORK) == ENCRYPTION_NETWORK)
#define hasApsEncryption(packetSecurity) \
  (((packetSecurity) & ENCRYPTION_APS)     == ENCRYPTION_APS)

#define STANDALONE_NONCE_SOURCE_ADDR_INDEX       0
#define STANDALONE_NONCE_FRAME_COUNTER_INDEX     8
#define STANDALONE_NONCE_SECURITY_CONTROL_INDEX 12

#define SL_ZIGBEE_NETWORK_COMMISSIONING_REQUEST  0x0E

#define SECURITY_CONTROL_SIZE 1
#define SECURITY_FRAME_COUNTER_SIZE 4

#endif //SILABS_ZIGBEE_PACKET_TYPES_H
