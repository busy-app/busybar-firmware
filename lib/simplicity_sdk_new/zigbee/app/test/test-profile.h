/***************************************************************************//**
 * @file
 * @brief Common code for Application Test Profile #2.  This was originally part of
 * level-three.c but was extracted and put into a common file.
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
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
void msgCommand(SL_CLI_COMMAND_ARG);
void spoofedBroadcastMsgCommand(SL_CLI_COMMAND_ARG);
void sendPacketsCommand(SL_CLI_COMMAND_ARG);
void sendPacketCommand(SL_CLI_COMMAND_ARG);
void packetCountCommand(SL_CLI_COMMAND_ARG);
void setRadiusCommand(SL_CLI_COMMAND_ARG);
void setFragmentsCommand(SL_CLI_COMMAND_ARG);
void changeTxMode(SL_CLI_COMMAND_ARG);
void setProfileIdCommand(SL_CLI_COMMAND_ARG);
void enableRxPrintCommand(SL_CLI_COMMAND_ARG);
void sendAliasMsg(SL_CLI_COMMAND_ARG);
bool testProfileMessageHandler(sl_802154_short_addr_t sender,
                               sl_zigbee_aps_frame_t *apsFrame,
                               uint8_t messageLength,
                               uint8_t *message);
void testProfileMessageSentHandler(sl_zigbee_outgoing_message_type_t type,
                                   sl_zigbee_aps_frame_t *apsFrame,
                                   sl_status_t status);
void sendEventHandler0(sl_zigbee_af_event_t * event);
void sendEventHandler1(sl_zigbee_af_event_t * event);
void sendEventHandler2(sl_zigbee_af_event_t * event);
void sendEventHandler3(sl_zigbee_af_event_t * event);

sl_status_t transmitMessage(sl_802154_short_addr_t destination,
                            uint8_t  messageLength,
                            uint8_t* messageContents,
                            uint8_t sourceEndpoint,
                            uint8_t destEndpoint,
                            uint16_t clusterId,
                            uint8_t *apsSeqn,
                            sl_zigbee_aps_option_t options,
                            uint16_t nwkBroadcastDestination);

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
uint8_t sl_zigbee_af_maximum_aps_payload_length_pro_compliance(sl_zigbee_outgoing_message_type_t type,
                                                               uint16_t indexOrDestination,
                                                               sl_zigbee_aps_frame_t *apsFrame);
#else
uint8_t sl_zigbee_af_maximum_aps_payload_length(sl_zigbee_outgoing_message_type_t type,
                                                uint16_t indexOrDestination,
                                                sl_zigbee_aps_frame_t *apsFrame);
#endif

extern bool sli_zigbee_send_multicasts_to_sleepy_address;
#define DEFAULT_MULTICAST_NWK_BROADCAST_ADDRESS ((sli_zigbee_send_multicasts_to_sleepy_address) \
                                                 ? SL_ZIGBEE_SLEEPY_BROADCAST_ADDRESS           \
                                                 : SL_ZIGBEE_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS)

// Implementation specific code
sl_status_t initializeFragmentation(uint8_t maxTotalMessageLength,
                                    uint8_t windowSize,
                                    uint8_t fragmentDelayMs,
                                    uint8_t missedBlocks);

sl_status_t setTxMode(uint8_t signal);

// The ZigBee APS mode flags are used by the host to tell us if something
// is a multicast.
#define ZIGBEE_APS_FRAME_CONTROL_MODE_MASK      0x0Cu
#define ZIGBEE_APS_FRAME_CONTROL_MODE_MULTICAST 0x0Cu

//------------------------------------------------------------------------------

extern sl_zigbee_af_event_t *sendEvent[4];

extern const uint16_t testDriverInClusters[];
extern const uint16_t testDriverOutClusters[];

extern const uint16_t fullDeviceInClusters[];
extern const uint16_t fullDeviceOutClusters[];

extern sl_zigbee_endpoint_description_t const testDriverDescription;
extern sl_zigbee_endpoint_description_t const fullDeviceDescription;

#define TEST_PROFILE_ENDPOINT_COUNT 3

#define DONT_FRAGMENT 255
extern uint8_t maxMessageLength;      // for fragmentation only
extern uint8_t maxFragmentLength;

extern const uint16_t testProfileId;

#define SEND_PACKETS_EVENT_DEFINITION   \
  { &sendEvent[0], sendEventHandler0 }, \
  { &sendEvent[1], sendEventHandler1 }, \
  { &sendEvent[2], sendEventHandler2 }, \
  { &sendEvent[3], sendEventHandler3 },

//------------------------------------------------------------------------------

#define TEST_PROFILE_COMMANDS                                                                                               \
  /* sequence length, */                                                                                                    \
  /* source endpoint, destination (0xFFF[CDF] for broadcast, 0xFFFE for */                                                  \
  /* indirect), destination endpoint,  */                                                                                   \
  /* options (retry = 0x40, discovery = 0x80, force discovery = 0x20) */                                                    \
  sl_zigbee_command_entry_action("msg_buffer", msgCommand, "uuvuv",                                                         \
                                 "Sends a Msg buffer test request"),                                                        \
                                                                                                                            \
  /* request type, */                                                                                                       \
  /* source endpoint, destination (0xFFF[CDF] for broadcast, 0xFFFE for */                                                  \
  /* indirect), destination endpoint, */                                                                                    \
  /* options (retry = 0x40, discovery = 0x80, force discovery = 0x20) */                                                    \
  sl_zigbee_command_entry_action("msg_freeform", msgCommand, "uuvuv",                                                       \
                                 "Sends a Freeform MSG request"),                                                           \
                                                                                                                            \
  /* Buffer Test Group Request */                                                                                           \
  /* sequence length */                                                                                                     \
  /* source endpoint, group address, destination endpoint, */                                                               \
  /* options (retry = 0x40, discovery = 0x80, force discovery = 0x20) */                                                    \
  sl_zigbee_command_entry_action("msg_group", msgCommand, "uuvuv",                                                          \
                                 "Sends a Msg Buffer Test Group request"),                                                  \
                                                                                                                            \
  /* length, count (0 for no limit), interval (ms), max in flight (0 for no */                                              \
  /* limit) source endpoint, destination (0xFFF[CDF] for broadcast, 0xFFFE */                                               \
  /* for indirect), destination endpoint, */                                                                                \
  /* options (retry = 0x40, discovery = 0x80, force discovery = 0x20) */                                                    \
  sl_zigbee_command_entry_action("send_packets", sendPacketsCommand, "uvvuuvuv",                                            \
                                 "Sends continuous packets to destination"),                                                \
                                                                                                                            \
  /* length, source endpoint, destination endpoint, */                                                                      \
  /* destination (0xFFF[CDF] for broadcast, 0xFFFE for indirect), */                                                        \
  /* options (retry = 0x40, discovery = 0x80, force discovery = 0x20) */                                                    \
  { "send_packet", sendPacketCommand, "uuuvv" },                                                                            \
                                                                                                                            \
  /* source endpoint, destination (0xFFF[CDF] for broadcast, 0xFFFE for */                                                  \
  /* indirect), destination endpoint, */                                                                                    \
  /* options (retry = 0x40, discovery = 0x80, force discovery = 0x20) */                                                    \
  sl_zigbee_command_entry_action("reset_count", packetCountCommand, "uvuv",                                                 \
                                 "Sends a 'reset count' command to destination"),                                           \
  sl_zigbee_command_entry_action("get_count", packetCountCommand, "uvuv",                                                   \
                                 "Sends a 'get count' command to destination"),                                             \
                                                                                                                            \
  /* broadcast radius, non member radius */                                                                                 \
  sl_zigbee_command_entry_action("set_radius", setRadiusCommand, "uu",                                                      \
                                 "Sets default broadcast radius"),                                                          \
  /* max message size, max fragment size, window size, interframe delay (ms) */                                             \
  /* missed blocks bitmask (a debug value controlling which fragments were */                                               \
  /*   missed; the value in the APS Ack bitfield.  Set to 0xFF for normal */                                                \
  /*   operation.) */                                                                                                       \
  sl_zigbee_command_entry_action("set_fragment", setFragmentsCommand, "uuuuu",                                              \
                                 "Sets fragmentation parameters"),                                                          \
                                                                                                                            \
  sl_zigbee_command_entry_action("set_profile", setProfileIdCommand, "v",                                                   \
                                 "Set the profile ID for initiated Test Profile messages."),                                \
  sl_zigbee_command_entry_action("change_tx_mode", changeTxMode, "u",                                                       \
                                 "Change the mac TX mode between normal(0), request a grant(1), and complete hold off(2)"), \
  sl_zigbee_command_entry_action("enable_rx_print", enableRxPrintCommand, "u",                                              \
                                 "Enable/disable printing RX messages"),                                                    \
  sl_zigbee_command_entry_action("spoofed_broadcast_msg_buffer", spoofedBroadcastMsgCommand, "uvuvuvu",                     \
                                 "spoofed broadcast"),                                                                      \
  sl_zigbee_command_entry_action("alias_send", sendAliasMsg, "uv",                                                          \
                                 "Sends multicast frame to groupid 0xFABE with alias nwk address.                           \
                          argument 1 is sequence number, argument 2 is alias address"),                                     \

//------------------------------------------------------------------------------

#define ENDPOINT_TEST_DRIVER   0x01
#define ENDPOINT_FULL_DEVICE_1 0x02
#define ENDPOINT_FULL_DEVICE_2 0xF0

#define CLUSTER_MANAGE_NO_DATA        0x0000
#define CLUSTER_COUNTED_PACKETS       0x0001
#define CLUSTER_RESET_PACKET_COUNT    0x0002
#define CLUSTER_RETRIEVE_PACKET_COUNT 0x0003
#define CLUSTER_PACKET_COUNT_RESPONSE 0x0004
#define CLUSTER_BUFFER_TEST_REQUEST   0x001C
#define CLUSTER_BUFFER_GROUP_REQUEST  0x001D
#define CLUSTER_MANAGE_INT_8          0x38
#define CLUSTER_BUFFER_TEST_RESPONSE  0x0054
#define CLUSTER_BUFFER_GROUP_RESPONSE 0x0055
#define CLUSTER_MANAGE_INT_16         0x70
#define CLUSTER_MANAGE_SEMI_PRECISION 0x8C
#define CLUSTER_ROUTE_DISC_REQUEST    0x1000
#define CLUSTER_ROUTE_DISC_RESPONSE   0x1001
#define CLUSTER_BOARD_LED_CONTROL     0x2000
#define CLUSTER_FREEFORM_MSG_REQUEST  0xA0A8
#define CLUSTER_MANAGE_TIME           0xC4
#define CLUSTER_FREEFORM_MSG_RESPONSE 0xE000
#define CLUSTER_FREEFORM_NO_DATA_RESPONSE 0xE001
#define CLUSTER_BROADCAST_REQUEST     0xF000
#define CLUSTER_BROADCAST_RESPONSE    0xF001
#define CLUSTER_BROADCAST_COORD_ROUTER_REQUEST      0xF00A
#define CLUSTER_BROADCAST_COORD_ROUTER_RESPONSE     0xF00E
#define CLUSTER_GENERIC_MESSAGE       0xF00F
#define CLUSTER_MANAGE_STRING         0xFF

#define REQUEST_TYPE_8_BIT_INT        0x00
#define REQUEST_TYPE_CHAR_STRING      0x01
#define REQUEST_TYPE_COORDINATES      0x02
#define REQUEST_TYPE_16_BIT_INT       0x03
#define REQUEST_TYPE_NO_DATA          0x04
#define REQUEST_TYPE_RELATIVE_TIME    0x05
#define REQUEST_TYPE_ABSOLUTE_TIME    0x06

#define KVP_CLUSTERS             \
  CLUSTER_MANAGE_NO_DATA,        \
  CLUSTER_MANAGE_INT_8,          \
  CLUSTER_MANAGE_INT_16,         \
  CLUSTER_MANAGE_SEMI_PRECISION, \
  CLUSTER_MANAGE_TIME,           \
  CLUSTER_MANAGE_STRING

#define MSG_REQUEST_CLUSTERS    \
  CLUSTER_BUFFER_TEST_REQUEST,  \
  CLUSTER_FREEFORM_MSG_REQUEST, \
  CLUSTER_BUFFER_GROUP_REQUEST, \
  CLUSTER_ROUTE_DISC_REQUEST

#define MSG_RESPONSE_CLUSTERS    \
  CLUSTER_BUFFER_TEST_RESPONSE,  \
  CLUSTER_FREEFORM_MSG_RESPONSE, \
  CLUSTER_BUFFER_GROUP_RESPONSE, \
  CLUSTER_ROUTE_DISC_RESPONSE

#define MSG_TX_PACKET_CLUSTERS \
  CLUSTER_COUNTED_PACKETS,     \
  CLUSTER_RESET_PACKET_COUNT,  \
  CLUSTER_RETRIEVE_PACKET_COUNT

#define MSG_RX_PACKET_CLUSTERS \
  CLUSTER_PACKET_COUNT_RESPONSE

#define TEST_DRIVER_IN_CLUSTERS \
  MSG_RESPONSE_CLUSTERS,        \
  MSG_RX_PACKET_CLUSTERS

#define TEST_DRIVER_OUT_CLUSTERS \
  MSG_REQUEST_CLUSTERS,          \
  MSG_TX_PACKET_CLUSTERS

#define KVP_DEVICE_IN_CLUSTERS \
  KVP_CLUSTERS

#define KVP_DEVICE_OUT_CLUSTERS \
  KVP_CLUSTERS

#define MSG_DEVICE_IN_CLUSTERS \
  MSG_REQUEST_CLUSTERS,        \
  MSG_TX_PACKET_CLUSTERS

#define MSG_DEVICE_OUT_CLUSTERS \
  MSG_RESPONSE_CLUSTERS,        \
  MSG_RX_PACKET_CLUSTERS

#define FULL_DEVICE_IN_CLUSTER_COUNT 7
#define FULL_DEVICE_IN_CLUSTERS \
  MSG_DEVICE_IN_CLUSTERS

#define FULL_DEVICE_OUT_CLUSTER_COUNT 5
#define FULL_DEVICE_OUT_CLUSTERS \
  MSG_DEVICE_OUT_CLUSTERS

#define MAX_CLUSTER_LIST_LENGTH FULL_DEVICE_IN_CLUSTER_COUNT

#define ENDPOINT_TEST_DRIVER_DEFINITION           \
  { ENDPOINT_TEST_DRIVER, &testDriverDescription, \
    testDriverInClusters, testDriverOutClusters }

#define ENDPOINT_FULL_DEVICE_1_DEFINITION           \
  { ENDPOINT_FULL_DEVICE_1, &fullDeviceDescription, \
    fullDeviceInClusters, fullDeviceOutClusters }

#define ENDPOINT_FULL_DEVICE_2_DEFINITION           \
  { ENDPOINT_FULL_DEVICE_2, &fullDeviceDescription, \
    fullDeviceInClusters, fullDeviceOutClusters }
