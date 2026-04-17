/***************************************************************************//**
 * @file
 * @brief Header file of a sample of a custom EZSP protocol.
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

// This is set according to the ZigBee specification's manufacturer ID list
// Ember's manufacturer ID value is 0x1002.
#define XNCP_MANUFACTURER_ID  SL_ZIGBEE_MANUFACTURER_ID
#define XNCP_VERSION_NUMBER   0xBEEF

// For this sample app all the custom EZSP messages are formatted as follows:
//   <Custom command ID:1>
//   <parameters:0-118>

// -----------------------------------------------------------------------------
// Protocol specs:

// ----------------------
// SET_POWER_MODE
// Command: Command ID (1 byte) + power mode (1 byte)
// Response: Status (4 byte)
// ----------------------
// GET_POWER_MODE
// Command: Command ID (1 byte)
// Response: Status (4 byte) + power mode (1 byte)
// ----------------------
// ADD_CLUSTER_TO_FILTERING_LIST
// Command: Command ID (1 byte) + cluster ID (2 bytes)
// Response: Status (4 byte)
// ----------------------
// REMOVE_CLUSTER_TO_FILTERING_LIST
// Command: Command ID (1 byte) + cluster ID (2 bytes)
// Response: Status (4 byte)
// ----------------------
// GET_CLUSTER_TO_FILTERING_LIST
// Command: Command ID (1 byte)
// Response: Status (4 byte)  + num. entries (1 byte) + cluster IDs (variable)
// ----------------------
// ENABLE_PERIODIC_REPORTS
// Command: Command ID (1 byte) + time interval (secs.) (2 bytes)
// Response: Status (4 byte)
// ----------------------
// DISABLE_PERIODIC_REPORTS
// Command: Command ID (1 byte)
// Response: Status (4 byte)
// ----------------------
// SET_CUSTOM_TOKEN
// Command: Command ID (1 byte) + nodeType (1 byte) + nodeID (2 bytes) +
//          + panID (2 bytes)
// Response: Status (4 byte)
// ----------------------
// GET_CUSTOM_TOKEN
// Command: Command ID (1 byte)
// Response: Status (4 byte) + nodeType (1 byte) + nodeID (2 bytes) +
//           + panID (2 bytes)
// ----------------------
// CALLBACK_REPORT
// Command: Command ID (1 byte) + report count (2 bytes)
// Response: -

// -----------------------------------------------------------------------------

// Custom command IDs
enum {
  // HOST -> XNCP
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_SET_POWER_MODE                               = 0x00,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_GET_POWER_MODE                               = 0x01,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_ADD_CLUSTER_TO_FILTERING_LIST                = 0x02,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_REMOVE_CLUSTER_TO_FILTERING_LIST             = 0x03,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_GET_CLUSTER_FILTERING_LIST                   = 0x04,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_ENABLE_PERIODIC_REPORTS                      = 0x05,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_DISABLE_PERIODIC_REPORTS                     = 0x06,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_SET_CUSTOM_TOKEN                             = 0x07,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_GET_CUSTOM_TOKEN                             = 0x08,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_INIT_APP_GP_TX_QUEUE                         = 0x0E,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_SET_APP_GP_TX_QUEUE                          = 0x0F,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_SEND_APP_GP_RAW                              = 0x10,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_GET_APP_GP_TX_QUEUE                          = 0x11,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_SET_APP_GP_TX_QUEUE_MAX_SIZE                 = 0x12,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_GET_APP_GP_TX_QUEUE_MAX_SIZE                 = 0x13,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_GET_APP_GP_TX_QUEUE_COUNT                    = 0x14,
  SL_ZIGBEE_CUSTOM_EZSP_COMMAND_REMOVE_APP_GP_TX_QUEUE                       = 0x05,

  // XNCP -> HOST
  SL_ZIGBEE_CUSTOM_EZSP_CALLBACK_REPORT                                      = 0x80,
  SL_ZIGBEE_CUSTOM_EZSP_CALLBACK_APP_GP_TX_QUEUE_EVENT                       = 0x81,
};

// Power modes
enum {
  SL_ZIGBEE_XNCP_NORMAL_MODE,
  SL_ZIGBEE_XNCP_LOW_POWER_MODE,
  SL_ZIGBEE_XNCP_RESERVED // always last in the enum
};
