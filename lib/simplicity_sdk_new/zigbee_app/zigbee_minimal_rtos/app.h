/***************************************************************************//**
 * @brief Header file for zigbee minimal Rtos app.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef ZIGBEE_MINIMAL_RTOS_APP_HEADER
#define ZIGBEE_MINIMAL_RTOS_APP_HEADER

#define CLUSTER_BUFFER_TEST_REQUEST 0x001C
#define CLUSTER_BUFFER_TEST_RESPONSE 0x0054
#define TEST_PROFILE_ID 0x7F01

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
#endif // ZIGBEE_MINIMAL_RTOS_APP_HEADER
