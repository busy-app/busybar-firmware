/*******************************************************************************
 * @file  rsi_ble.h
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef RSI_BLE_H
#define RSI_BLE_H

#include "rsi_ble_apis.h"
#include "sl_common.h"

/******************************************************
 * *                      Macros
 * ******************************************************/
/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */

/// Represents the BLE state when there is no active connection.
#define RSI_BLE_STATE_NONE 0x00
/// Represents the BLE state when the device is connected.
#define RSI_BLE_STATE_CONNECTION 0x01
/// Represents the BLE state when the device is disconnected
#define RSI_BLE_STATE_DSICONNECT 0x02
/// Defines the maximum number of request list entries in BLE.
#define RSI_BLE_MAX_REQ_LIST 0x05
/// Indicates that legacy BLE commands are used (BIT 0).
#define RSI_BLE_LEGACY_CMD_USED (1 << 0) //BIT(0)
/// Indicates that advertising extension commands are used (BIT 1).
#define RSI_BLE_ADV_EXT_CMD_USED (1 << 1) //BIT(1)
/// Command to check BLE status (BIT 15).
#define RSI_BLE_CHECK_CMD (1 << 15) //BIT(15)
/// Indicates that the buffer is available.
#define SI_LE_BUFFER_AVL 0
/// Indicates that the buffer is full.
#define SI_LE_BUFFER_FULL 1
/// Indicates that the buffer operation is in progress.
#define SI_LE_BUFFER_IN_PROGRESS 2
/// Vendor-specific RF type command opcode.
#define BLE_VENDOR_RF_TYPE_CMD_OPCODE 0xFC14
/// BLE_VENDOR_ACCEPTLIST_USING_ADV_DATA_PAYLOAD.
#define BLE_VENDOR_ACCEPTLIST_USING_ADV_DATA_PAYLOAD 0xFC1B
/// BLE_VENDOR_SET_COEX_ROLE_PRIORITY.
#define BLE_VENDOR_SET_COEX_ROLE_PRIORITY 0xFC31
/// Defines the maximum number of GAP extension callbacks.
#define RSI_BLE_MAX_NUM_GAP_EXT_CALLBACKS 2
/// Defines the maximum number of advertising extension event callbacks.
#define RSI_BLE_MAX_NUM_ADV_EXT_EVENT_CALLBACKS 0x08
/// Advertising extension reporting is disabled.
#define BLE_AE_REPORTING_DISABLED 0x01
/// Advertising extension reporting is enabled.
#define BLE_AE_REPORTING_ENABLED 0x00
/// Enables periodic duplicate filtering.
#define BLE_AE_PERODIC_DUPLICATE_FILTERING_ENABLED 0x01
/// Disables periodic duplicate filtering.
#define BLE_AE_PERODIC_DUPLICATE_FILTERING_DISABLED 0x00
/// Periodic list usage flag.
#define BLE_AE_PERIODIC_LIST_USED 0x01
// BLE_VENDOR_ACCEPTLIST_ON_TYPE
#define BLE_VENDOR_ACCEPTLIST_ON_TYPE 0xFC27

/******************************************************
 * *                    Constants
 * ******************************************************/

/******************************************************
 * *                   Enumerations
 * ******************************************************/
/// Enumeration for BLE advertising extension command request codes
typedef enum RSI_BLE_CMD_AE_opcode_e {
  RSI_BLE_GET_AE_MAX_NO_OF_ADV_SETS  = 0x0001,
  RSI_BLE_GET_AE_MAX_ADV_DATA_LEN    = 0x0002,
  RSI_BLE_SET_AE_SET_RANDOM_ADDR     = 0x0003,
  RSI_BLE_SET_AE_PARAMS              = 0x0004,
  RSI_BLE_SET_AE_DATA                = 0x0005,
  RSI_BLE_SET_AE_ENABLE              = 0x006,
  RSI_BLE_ADV_SET_CLEAR_OR_REMOVE    = 0x0007,
  RSI_BLE_SET_AE_PERIODIC_ADV_PARAMS = 0x0008,
  RSI_BLE_SET_AE_PERIODIC_ADV_ENABLE = 0x0009,
  RSI_BLE_SET_AE_SCAN_PARAMS         = 0x000A,
  RSI_BLE_SET_AE_SCAN_ENABLE         = 0x000B,
  RSI_BLE_SET_AE_PERIODIC_SYNC       = 0x000C,
  RSI_BLE_AE_DEV_TO_PERIODIC_LIST    = 0x000D,
  RSI_BLE_AE_READ_PERIODIC_LIST_SIZE = 0x000E,
  RSI_BLE_AE_EXTENDED_CREATE_CONNECT = 0x000F,
} RSI_BLE_CMD_AE_opcode_t;

/// Enumeration for BLE command request codes
typedef enum rsi_ble_cmd_request_e {
  RSI_BLE_REQ_HCI_RAW                                = 0x0050,
  RSI_BLE_REQ_ADV                                    = 0x0075,
  RSI_BLE_REQ_SCAN                                   = 0x0076,
  RSI_BLE_REQ_CONN                                   = 0x0077,
  RSI_BLE_REQ_DISCONNECT                             = 0x0078,
  RSI_BLE_GET_DEV_STATE                              = 0x0079,
  RSI_BLE_CMD_CONN_PARAMS_UPDATE                     = 0x007A,
  RSI_BLE_REQ_START_ENCRYPTION                       = 0x007B,
  RSI_BLE_REQ_SMP_PAIR                               = 0x007C,
  RSI_BLE_SMP_PAIR_RESPONSE                          = 0x007D,
  RSI_BLE_SMP_PASSKEY                                = 0x007E,
  RSI_BLE_REQ_PROFILES                               = 0x007F,
  RSI_BLE_REQ_PROFILE                                = 0x0080,
  RSI_BLE_REQ_CHAR_SERVICES                          = 0x0081,
  RSI_BLE_REQ_INC_SERVICES                           = 0x0082,
  RSI_BLE_REQ_READ_BY_UUID                           = 0x0083,
  RSI_BLE_REQ_DESC                                   = 0x0084,
  RSI_BLE_REQ_READ_VAL                               = 0x0085,
  RSI_BLE_REQ_MULTIPLE_READ                          = 0x0086,
  RSI_BLE_REQ_LONG_READ                              = 0x0087,
  RSI_BLE_REQ_WRITE                                  = 0x0088,
  RSI_BLE_REQ_WRITE_NO_ACK                           = 0x0089,
  RSI_BLE_REQ_LONG_WRITE                             = 0x008A,
  RSI_BLE_REQ_PREPARE_WRITE                          = 0x008B,
  RSI_BLE_REQ_EXECUTE_WRITE                          = 0x008C,
  RSI_BLE_ADD_SERVICE                                = 0x0092,
  RSI_BLE_ADD_ATTRIBUTE                              = 0x0093,
  RSI_BLE_SET_LOCAL_ATT_VALUE                        = 0x0094,
  RSI_BLE_GET_LOCAL_ATT_VALUE                        = 0x0095,
  RSI_BLE_CMD_NOTIFY                                 = 0x0096,
  RSI_BLE_CMD_INDICATE                               = 0x0097,
  RSI_BLE_SET_ADVERTISE_DATA                         = 0x009C,
  RSI_BLE_GET_LE_PING                                = 0x00A1,
  RSI_BLE_SET_LE_PING                                = 0x00A2,
  RSI_BLE_SET_RANDOM_ADDRESS                         = 0x00A3,
  RSI_BLE_ENCRYPT                                    = 0x00A4,
  RSI_BLE_CMD_READ_RESP                              = 0x00A5,
  RSI_BLE_SET_SCAN_RESPONSE_DATA                     = 0x00A8,
  RSI_BLE_LE_ACCEPT_LIST                             = 0x00AA,
  RSI_BLE_CMD_REMOVE_SERVICE                         = 0x00AB,
  RSI_BLE_CMD_REMOVE_ATTRIBUTE                       = 0x00AC,
  RSI_BLE_PROCESS_RESOLV_LIST                        = 0x00AD,
  RSI_BLE_GET_RESOLVING_LIST_SIZE                    = 0x00AE,
  RSI_BLE_SET_ADDRESS_RESOLUTION_ENABLE              = 0x00AF,
  RSI_BLE_REQ_READ_PHY                               = 0x00B0,
  RSI_BLE_REQ_SET_PHY                                = 0x00B1,
  RSI_BLE_SET_DATA_LEN                               = 0x00B2,
  RSI_BLE_READ_MAX_DATA_LEN                          = 0X00B3,
  RSI_BLE_SET_PRIVACY_MODE                           = 0x00B4,
  RSI_BLE_CBFC_CONN_REQ                              = 0x00B5,
  RSI_BLE_CBFC_CONN_RESP                             = 0x00B6,
  RSI_BLE_CBFC_TX_DATA                               = 0x00B7,
  RSI_BLE_CBFC_DISCONN                               = 0x00B8,
  RSI_BLE_LE_LTK_REQ_REPLY                           = 0X00BA,
  RSI_BLE_RX_TEST_MODE                               = 0x00BB,
  RSI_BLE_TX_TEST_MODE                               = 0x00BC,
  RSI_BLE_END_TEST_MODE                              = 0x00BD,
  RSI_BLE_PER_TX_MODE                                = 0x00BF,
  RSI_BLE_PER_RX_MODE                                = 0x00C0,
  RSI_BLE_CMD_ATT_ERROR                              = 0x00C1,
  RSI_BLE_REQ_PROFILES_ASYNC                         = 0x00F2,
  RSI_BLE_REQ_PROFILE_ASYNC                          = 0x00F3,
  RSI_BLE_GET_CHARSERVICES_ASYNC                     = 0x00F4,
  RSI_BLE_GET_INCLUDESERVICES_ASYNC                  = 0x00F5,
  RSI_BLE_READCHARVALUEBYUUID_ASYNC                  = 0x00F6,
  RSI_BLE_GET_ATTRIBUTE_ASYNC                        = 0x00F7,
  RSI_BLE_GET_DESCRIPTORVALUE_ASYNC                  = 0x00F8,
  RSI_BLE_GET_MULTIPLEVALUES_ASYNC                   = 0x00F9,
  RSI_BLE_GET_LONGDESCVALUES_ASYNC                   = 0x00FA,
  RSI_BLE_SET_DESCVALUE_ASYNC                        = 0x00FB,
  RSI_BLE_SET_PREPAREWRITE_ASYNC                     = 0x00FC,
  RSI_BLE_EXECUTE_LONGDESCWRITE_ASYNC                = 0x00FD,
  RSI_BLE_SET_SMP_PAIRING_CAPABILITY_DATA            = 0x0101,
  RSI_BLE_CONN_PARAM_RESP_CMD                        = 0x0105,
  RSI_BLE_CMD_INDICATE_CONFIRMATION                  = 0x0106,
  RSI_BLE_MTU_EXCHANGE_REQUEST                       = 0x0107,
  RSI_BLE_CMD_SET_WWO_RESP_NOTIFY_BUF_INFO           = 0x0108,
  RSI_BLE_CMD_WRITE_RESP                             = 0x010A,
  RSI_BLE_CMD_PREPARE_WRITE_RESP                     = 0x010B,
  RSI_BLE_CMD_SET_LOCAL_IRK                          = 0x010C,
  RSI_BLE_REQ_SMP_PAIRING_FAILED                     = 0x0111,
  RSI_BLE_CMD_SET_PROP_PROTOCOL_BLE_BANDEDGE_TXPOWER = 0x012A,
  RSI_BLE_CMD_MTU_EXCHANGE_RESP                      = 0x012B,
  RSI_BLE_CMD_SET_BLE_TX_POWER                       = 0x012D,
  RSI_BLE_CMD_INDICATE_SYNC                          = 0x016F,
  RSI_BLE_CMD_AE                                     = 0x0171,
  RSI_BLE_CMD_READ_TRANSMIT_POWER                    = 0x0172,
  RSI_BLE_CMD_READ_RF_PATH_COMP                      = 0x0173,
  RSI_BLE_CMD_WRITE_RF_PATH_COMP                     = 0x0174,
  RSI_BLE_REQ_CONN_ENHANCE                           = 0x1FFF, // Add new cmd ids above this cmd id.
} rsi_ble_cmd_request_t;

/// Enumeration for BLE command response codes
typedef enum rsi_ble_cmd_resp_e {
  RSI_BLE_RSP_ADVERTISE                     = 0x0075,
  RSI_BLE_RSP_SCAN                          = 0x0076,
  RSI_BLE_RSP_CONNECT                       = 0x0077,
  RSI_BLE_RSP_DISCONNECT                    = 0x0078,
  RSI_BLE_RSP_QUERY_DEVICE_STATE            = 0x0079,
  RSI_BLE_RSP_CONN_PARAMS_UPDATE            = 0x007A,
  RSI_BLE_RSP_START_ENCRYPTION              = 0x007B,
  RSI_BLE_RSP_SMP_PAIR                      = 0x007C,
  RSI_BLE_RSP_SMP_PAIR_RESPONSE             = 0x007D,
  RSI_BLE_RSP_SMP_PASSKEY                   = 0x007E,
  RSI_BLE_RSP_PROFILES                      = 0x007F,
  RSI_BLE_RSP_PROFILE                       = 0x0080,
  RSI_BLE_RSP_CHAR_SERVICES                 = 0x0081,
  RSI_BLE_RSP_INC_SERVICES                  = 0x0082,
  RSI_BLE_RSP_READ_BY_UUID                  = 0x0083,
  RSI_BLE_RSP_DESC                          = 0x0084,
  RSI_BLE_RSP_READ_VAL                      = 0x0085,
  RSI_BLE_RSP_MULTIPLE_READ                 = 0x0086,
  RSI_BLE_RSP_LONG_READ                     = 0x0087,
  RSI_BLE_RSP_WRITE                         = 0x0088,
  RSI_BLE_RSP_WRITE_NO_ACK                  = 0x0089,
  RSI_BLE_RSP_LONG_WRITE                    = 0x008A,
  RSI_BLE_RSP_PREPARE_WRITE                 = 0x008B,
  RSI_BLE_RSP_EXECUTE_WRITE                 = 0x008C,
  RSI_BLE_RSP_INIT                          = 0x008D,
  RSI_BLE_RSP_DEINIT                        = 0x008E,
  RSI_BLE_RSP_SET_ANTENNA                   = 0x008F,
  RSI_BLE_RSP_ADD_SERVICE                   = 0x0092,
  RSI_BLE_RSP_ADD_ATTRIBUTE                 = 0x0093,
  RSI_BLE_RSP_SET_LOCAL_ATT_VALUE           = 0x0094,
  RSI_BLE_RSP_GET_LOCAL_ATT_VALUE           = 0x0095,
  RSI_BLE_RSP_NOTIFY                        = 0x0096,
  RSI_BLE_RSP_GET_LE_PING                   = 0x00A1,
  RSI_BLE_RSP_SET_LE_PING                   = 0x00A2,
  RSI_BLE_RSP_SET_RANDOM_ADDRESS            = 0x00A3,
  RSI_BLE_RSP_ENCRYPT                       = 0x00A4,
  RSI_BLE_RSP_READ_RESP                     = 0x00A5,
  RSI_BLE_RSP_LE_ACCEPT_LIST                = 0x00AA,
  RSI_BLE_RSP_REMOVE_SERVICE                = 0x00AB,
  RSI_BLE_RSP_REMOVE_ATTRIBUTE              = 0x00AC,
  RSI_BLE_RSP_PROCESS_RESOLV_LIST           = 0x00AD,
  RSI_BLE_RSP_GET_RESOLVING_LIST_SIZE       = 0x00AE,
  RSI_BLE_RSP_SET_ADDRESS_RESOLUTION_ENABLE = 0x00AF,
  RSI_BLE_RSP_READ_PHY                      = 0x00B0,
  RSI_BLE_RSP_SET_PHY                       = 0x00B1,
  RSI_BLE_RSP_SET_DATA_LEN                  = 0x00B2,
  RSI_BLE_RSP_READ_MAX_DATA_LEN             = 0X00B3,
  RSI_BLE_RSP_PRIVACY_MODE                  = 0x00B4,
  RSI_BLE_RSP_CBFC_CONN_REQ                 = 0x00B5,
  RSI_BLE_RSP_CBFC_CONN_RESP                = 0x00B6,
  RSI_BLE_RSP_CBFC_TX_DATA                  = 0x00B7,
  RSI_BLE_RSP_CBFC_DISCONN                  = 0x00B8,

  RSI_BLE_RSP_LE_LTK_REQ_REPLY = 0X00BA,
  RSI_BLE_RSP_RX_TEST_MODE     = 0x00BB,
  RSI_BLE_RSP_TX_TEST_MODE     = 0x00BC,
  RSI_BLE_RSP_END_TEST_MODE    = 0x00BD,
  RSI_BLE_RSP_PER_TX_MODE      = 0x00BE,
  RSI_BLE_RSP_PER_RX_MODE      = 0x00BF,

  RSI_BLE_RSP_ATT_ERROR = 0x00C1,

  RSI_BLE_RSP_PROFILES_ASYNC              = 0x00F2,
  RSI_BLE_RSP_PROFILE_ASYNC               = 0x00F3,
  RSI_BLE_RSP_GET_CHARSERVICES_ASYNC      = 0x00F4,
  RSI_BLE_RSP_GET_INCLUDESERVICES_ASYNC   = 0x00F5,
  RSI_BLE_RSP_READCHARVALUEBYUUID_ASYNC   = 0x00F6,
  RSI_BLE_RSP_GET_ATTRIBUTE_ASYNC         = 0x00F7,
  RSI_BLE_RSP_GET_DESCRIPTORVALUE_ASYNC   = 0x00F8,
  RSI_BLE_RSP_GET_MULTIPLEVALUES_ASYNC    = 0x00F9,
  RSI_BLE_RSP_GET_LONGDESCVALUES_ASYNC    = 0x00FA,
  RSI_BLE_RSP_SET_DESCVALUE_ASYNC         = 0x00FB,
  RSI_BLE_RSP_SET_PREPAREWRITE_ASYNC      = 0x00FC,
  RSI_BLE_RSP_EXECUTE_LONGDESCWRITE_ASYNC = 0x00FD,

  RSI_BLE_RSP_SET_SMP_PAIRING_CAPABILITY_DATA        = 0x0101,
  RSI_BLE_RSP_CONN_PARAM_RESP                        = 0x0105,
  RSI_BLE_RSP_INDICATE_CONFIRMATION                  = 0x0106,
  RSI_BLE_RSP_MTU_EXCHANGE_REQUEST                   = 0x0107,
  RSI_BLE_RSP_SET_WWO_RESP_NOTIFY_BUF_INFO           = 0x0108,
  RSI_BLE_RSP_SET_LOCAL_IRK                          = 0x010C,
  RSI_BLE_RSP_SMP_PAIRING_FAILED                     = 0x0111,
  RSI_BLE_RSP_SET_PROP_PROTOCOL_BLE_BANDEDGE_TXPOWER = 0x012A,
  RSI_BLE_RSP_MTU_EXCHANGE_RESP                      = 0x012B,
  RSI_BLE_RSP_SET_BLE_TX_POWER                       = 0x012D,

  RSI_BLE_RSP_AE                  = 0x0171,
  RSI_BLE_RSP_READ_TRANSMIT_POWER = 0x0172,
  RSI_BLE_RSP_READ_RF_PATH_COMP   = 0x0173,
  RSI_BLE_RSP_WRITE_RF_PATH_COMP  = 0x0174,
} rsi_ble_cmd_resp_t;

/// Enumeration for BLE event codes
typedef enum rsi_ble_event_e {
  RSI_BLE_EVENT_DISCONNECT                   = 0x1006,
  RSI_BLE_EVENT_GATT_ERROR_RESPONSE          = 0x1500,
  RSI_BLE_EVENT_GATT_DESC_VAL_RESPONSE       = 0x1501,
  RSI_BLE_EVENT_GATT_PRIMARY_SERVICE_BY_UUID = 0x1502,
  RSI_BLE_EVENT_GATT_READ_CHAR_SERVS =
    0x1503, //read by type: read char, include serivces and read value by UUID (Universal Unique Identifier).
  RSI_BLE_EVENT_GATT_READ_INC_SERVS        = 0x1504, //read by type: read char, include serivces and read value by UUID.
  RSI_BLE_EVENT_GATT_READ_VAL_BY_UUID      = 0x1505, //read by type: read char, include serivces and read value by UUID.
  RSI_BLE_EVENT_GATT_READ_RESP             = 0x1506,
  RSI_BLE_EVENT_GATT_READ_BLOB_RESP        = 0x1507,
  RSI_BLE_EVENT_GATT_READ_MULTIPLE_RESP    = 0x1508,
  RSI_BLE_EVENT_GATT_PRIMARY_SERVICE_LIST  = 0x1509,
  RSI_BLE_EVENT_GATT_WRITE_RESP            = 0x150A,
  RSI_BLE_EVENT_GATT_PREPARE_WRITE_RESP    = 0x150B,
  RSI_BLE_EVENT_GATT_EXECUTE_WRITE_RESP    = 0x150C,
  RSI_BLE_EVENT_GATT_INDICATE_CONFIRMATION = 0x150D,
  RSI_BLE_EVENT_ADV_REPORT                 = 0x150E,
  RSI_BLE_EVENT_CONN_STATUS                = 0x150F,
  RSI_BLE_EVENT_SMP_REQUEST                = 0x1510,
  RSI_BLE_EVENT_SMP_RESPONSE               = 0x1511,
  RSI_BLE_EVENT_SMP_PASSKEY                = 0x1512,
  RSI_BLE_EVENT_SMP_FAILED                 = 0x1513,
  RSI_BLE_EVENT_GATT_NOTIFICATION          = 0x1514,
  RSI_BLE_EVENT_GATT_INDICATION            = 0x1515,
  RSI_BLE_EVENT_ENCRYPT_STARTED            = 0x1516,
  RSI_BLE_EVENT_GATT_WRITE                 = 0x1517,
  RSI_BLE_EVENT_LE_PING_TIME_EXPIRED       = 0x1518,
  RSI_BLE_EVENT_PREPARE_WRITE              = 0x1519,
  RSI_BLE_EVENT_EXECUTE_WRITE              = 0x151A,
  RSI_BLE_EVENT_READ_REQ                   = 0x151B,
  RSI_BLE_EVENT_MTU                        = 0x151C,
  RSI_BLE_EVENT_SMP_PASSKEY_DISPLAY_EVENT  = 0x151D,
  RSI_BLE_EVENT_PHY_UPDATE_COMPLETE        = 0x151E,
  RSI_BLE_EVENT_DATA_LENGTH_UPDATE_COMPLETE = 0x151F,
  RSI_BLE_EVENT_SC_PASSKEY                  = 0x1520,
  RSI_BLE_EVENT_ENHANCE_CONN_STATUS         = 0x1521,
  RSI_BLE_EVENT_DIRECTED_ADV_REPORT         = 0x1522,
  RSI_BLE_EVENT_SECURITY_KEYS               = 0x1523,
  RSI_BLE_EVENT_PSM_CONN_REQ                = 0x1524,
  RSI_BLE_EVENT_PSM_CONN_COMPLETE           = 0x1525,
  RSI_BLE_EVENT_PSM_RX_DATA                 = 0x1526,
  RSI_BLE_EVENT_PSM_DISCONNECT              = 0x1527,
  RSI_BLE_EVENT_LE_LTK_REQUEST              = 0x152A,
  RSI_BLE_EVENT_CONN_UPDATE_COMPLETE        = 0x152B,
  RSI_BLE_EVENT_REMOTE_FEATURES             = 0x152C,
  RSI_BLE_EVENT_LE_MORE_DATA_REQ            = 0x152D,
  RSI_BLE_EVENT_REMOTE_CONN_PARAMS_REQUEST  = 0x153C,
  RSI_BLE_EVENT_CLI_SMP_RESPONSE            = 0x153D,
  RSI_BLE_EVENT_CHIP_MEMORY_STATS           = 0x1530,
  RSI_BLE_EVENT_SC_METHOD                   = 0x1540,
  RSI_BLE_EVENT_MTU_EXCHANGE_INFORMATION    = 0x1541,
  RSI_BLE_EVENT_CTKD                        = 0x1542,
  RSI_BLE_EVENT_REMOTE_DEVICE_INFORMATION   = 0x1543,
  RSI_BLE_EVENT_AE_ADVERTISING_REPORT       = 0x1544,
  RSI_BLE_EVENT_PER_ADV_SYNC_ESTBL          = 0x1545,
  RSI_BLE_EVENT_PER_ADV_REPORT              = 0x1546,
  RSI_BLE_EVENT_PER_ADV_SYNC_LOST           = 0x1547,
  RSI_BLE_EVENT_SCAN_TIMEOUT                = 0x1548,
  RSI_BLE_EVENT_ADV_SET_TERMINATED          = 0x1549,
  RSI_BLE_EVENT_SCAN_REQ_RECVD              = 0x154a,
  RSI_BLE_EVENT_RCP_DATA_RCVD               = 0x15FF,
  RSI_BT_EVT_CONTROLLER_LOGS                = 0x1020,
} rsi_ble_event_t;

/// Enumerations for smp failure error
typedef enum {
  RSI_SMP_PAIRING_NOT_SUPPORTED = 0x05,
  RSI_SMP_UNSPECIFIED_REASON    = 0x08,
  RSI_SMP_REPEATED_ATTEMPTS     = 0x09,
} smp_failure_error;

/// Enumerations for call back types
typedef enum rsi_ble_callback_id_e {
  RSI_BLE_ON_CTKD                                    = 1,
  RSI_BLE_ON_ADV_EXT_ADVERTISE_REPORT_EVENT          = 2,
  RSI_BLE_ON_ADV_EXT_PERIODIC_ADV_SYNC_ESTBL_EVENT   = 3,
  RSI_BLE_ON_ADV_EXT_PERIODIC_ADVERTISE_REPORT_EVENT = 4,
  RSI_BLE_ON_ADV_EXT_PERIODIC_ADV_SYNC_LOST_EVENT    = 5,
  RSI_BLE_ON_ADV_EXT_SCAN_TIMEOUT_EVENT              = 6,
  RSI_BLE_ON_ADV_EXT_ADVERTISE_SET_TERMINATED_EVENT  = 7,
  RSI_BLE_ON_ADV_EXT_SCAN_REQUEST_RECEIVED_EVENT     = 8,
} rsi_ble_callback_id_t;

/// Enumerations for coex roles
typedef enum rsi_ble_coex_role_id_e {
  RSI_COEX_ROLE_BLE_SCAN           = 0,
  RSI_COEX_ROLE_BLE_INIT           = 1,
  RSI_COEX_ROLE_BLE_CONNECTION     = 2,
  RSI_COEX_ROLE_BLE_ADVERTISE      = 3,
  RSI_COEX_ROLE_BLE_EXT_ADVERTISE  = 4,
  RSI_COEX_ROLE_BLE_EXT_SCAN       = 5,
  RSI_COEX_ROLE_BLE_EXT_CONNECTION = 6,
} rsi_ble_coex_role_id_t;
/** @} */
/********************************************************
 * *                 Structure Definitions
 * ******************************************************/
/** @addtogroup BT_BLE_TYPES
  * @{ */
// enumerations for call back types
enum rsi_bt_debug_logs_callback_id_e { RSI_BT_ON_CONTROLLER_LOGS = 1, RSI_BT_MAX_NUM_DEBUG_LOGS_CALLBACKS };

// GAP command structures

/**
 * @brief Structure representing the BLE request to set a random address.
 *
 * This structure is used to define the parameters for the BLE request to set a random address,
 * which includes the random address of the device to be set.
 */
typedef struct rsi_ble_req_rand_s {
  /** Random address of the device to be set */
  uint8_t rand_addr[RSI_DEV_ADDR_LEN];
} rsi_ble_req_rand_t;

// Advertising command structure
/**
 * @brief Structure representing a BLE advertising request.
 */
typedef struct rsi_ble_req_adv_s {
  /** Advertising Status 
-
 
      0 - disable 
-
 1 - enable*/
  uint8_t status;
  /** Advertising type used during advertising 
-

      1. Advertising would visible (discoverable) to all the devices. Scanning/Connection is also accepted from all devices. 
-

      #define UNDIR_CONN     0x80 
-

      2. Advertising would be visible (discoverable) to the particular device mentioned in RSI_BLE_ADV_DIR_ADDR only. 
-

      Scanning and Connection would be accepted from that device only. 
-

      #define DIR_CONN           0x81 
-

      3. Advertising would be visible (discoverable) to all the devices. Scanning would be accepted from all the devices. 
-

      Connection would be not be accepted from any device. 
-

      #define UNDIR_SCAN         0x82 
-

      4. Advertising would be visible(discoverable) to all the devices. Scanning and Connection would not be accepted from any device. 
-

      #define UNDIR_NON_CONN       0x83 
-

      5. Advertising would be visible(discoverable) to the particular device mentioned in RSI_BLE_ADV_DIR_ADDR only. 
-

      Scanning and Connection would be accepted from that device only. 
-

      #define DIR_CONN_LOW_DUTY_CYCLE      0x84  */
  uint8_t adv_type;
  /** Advertising filter type 
-

      #define ALLOW_SCAN_REQ_ANY_CONN_REQ_ANY                        0x00 
-

      #define ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ANY                 0x01 
-

      #define ALLOW_SCAN_REQ_ANY_CONN_REQ_ACCEPT_LIST                 0x02 
-

      #define ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ACCEPT_LIST          0x03 */
  uint8_t filter_type;
  /** Address type of the device to which directed advertising has to be done 
-

      #define LE_PUBLIC_ADDRESS                                     0x00 
-

      #define LE_RANDOM_ADDRESS                                     0x01 
-

      #define LE_RESOLVABLE_PUBLIC_ADDRESS                          0x02 
-

      #define LE_RESOLVABLE_RANDOM_ADDRESS                          0x03 */
  uint8_t direct_addr_type;
  /** Address of the device to which directed advertising has to be done */
  uint8_t direct_addr[RSI_DEV_ADDR_LEN];

  /** Advertising interval min 0x0020 to 0x4000 */
  uint16_t adv_int_min;

  /** Advertising interval max 0x0020 to 0x4000 */
  uint16_t adv_int_max;

  /** Address of the local device. 
-

      #define LE_PUBLIC_ADDRESS                                     0x00 
-

      #define LE_RANDOM_ADDRESS                                     0x01 
-

      #define LE_RESOLVABLE_PUBLIC_ADDRESS                          0x02 
-

      #define LE_RESOLVABLE_RANDOM_ADDRESS                          0x03 */
  uint8_t own_addr_type;

  /** Advertising channel map. 
-

      #define  RSI_BLE_ADV_CHANNEL_MAP    0x01 or 0x03 or 0x07 */
  uint8_t adv_channel_map;
} rsi_ble_req_adv_t;

// Advertising data command structure

/**
 * @brief Structure represents the BLE request to set advertising data.
 *
 * This structure is used to define the parameters for the BLE request to set advertising data,
 * including the advertising data length and the advertising data itself.
 */
typedef struct rsi_ble_req_adv_data_s {
  /** Advertising data length */
  uint8_t data_len;
  /** Advertising data */
  uint8_t adv_data[31];
} rsi_ble_req_adv_data_t;

/**
 * @brief Structure representing the BLE request to manage the accept list using AD type, length, and value.
 */
typedef struct rsi_ble_req_acceptlist_on_type_s {
  uint8_t opcode[2];    // Operation code for the request
  uint8_t enable;       // Enable or disable the accept list
  uint8_t ad_type;      // Advertising data type to filter
  uint8_t value_length; // Length of the value to compare
  uint8_t value[31];    // Value to compare (payload)
} rsi_ble_req_acceptlist_on_type_t;
/**
 * @brief Structure representing the BLE request to manage the accept list using a payload.
 *
 * This structure is used to define the parameters for the BLE request to manage the accept list,
 * including the operation code, enable flag, total length of the payload, data compare index,
 * length of the data to compare, and the advertising data payload.
 */
typedef struct rsi_ble_req_acceptlist_using_payload_s {
  /** Operation code for the request */
  uint8_t opcode[2];
  /** Enable or disable the accept list */
  uint8_t enable;
  /** Total length of the payload */
  uint8_t total_len;
  /** Index for comparing data */
  uint8_t data_compare_index;
  /** Length of the data to compare */
  uint8_t len_for_compare_data;
  /** Advertising data payload */
  uint8_t adv_data_payload[31];
} rsi_ble_req_acceptlist_using_payload_t;

/**
 * @brief Structure representing the BLE request to manage the priorities of coex roles.
 *
 * This structure is used to define the priorities for the various coex roles,
 * including the role id, followed by min, max priorities of the specified role.
 * There are 7 coex roles refer rsi_ble_coex_role_id_t, each having min and max priority followed by the role id in the role_priority_payload.
 */
typedef struct rsi_ble_set_coex_roles_priority_s {
  /** Operation code for the request */
  uint8_t opcode[2];
  /** Role priority payload
  *    ---------------------------------------------------------------------------------------------------------
  *   |       Role ID       |                   Role Description          |   Min Priority |   Max Priority
  *   ----------------------|-----------------------------------------------------------------------------------
  *   |      0              |                   BLE Scan                  |      X         |      Y
  *   |      1              |                   BLE Init                  |      X         |      Y
  *   |      2              |                   BLE Connection            |      X         |      Y
  *   |      3              |                   BLE Advertise             |      X         |      Y
  *   |      4              |                   EXT Advertise             |      X         |      Y
  *   |      5              |                   EXT Scan                  |      X         |      Y
  *   |      6              |                   EXT Connection            |      X         |      Y
  * */
  uint8_t role_priority_payload[21];
} rsi_ble_set_coex_roles_priority_t;
/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */
/// BLE protocol identifier.
#define BLE_PROTOCOL 0x01
/// Proprietary protocol identifier.
#define PROP_PROTOCOL 0x02
/// Advertising role identifier.
#define ADV_ROLE 0x01
/// Scanning and central role identifier.
#define SCAN_AND_CENTRAL_ROLE 0x02
#ifndef PERIPHERAL_ROLE
/// Peripheral role identifier.
#define PERIPHERAL_ROLE 0x03
#endif
/// Connection role identifier.
#define CONN_ROLE 0x04
/** @} */

// Set BLE tx power cmd_ix=0x012D

/**
 * @brief Structure represents the BLE request to set the transmission power.
 *
 * This structure is used to define the parameters for the BLE request to set the transmission power,
 * which includes the transmission power value.
 */
typedef struct rsi_ble_set_ble_tx_power_s {
  /** Transmission power value */
  int8_t tx_power;
} rsi_ble_set_ble_tx_power_t;

//Scan response data command structure

/**
 * @brief Structure represents the set scan response data for the BLE scan request.
 *
 * This structure is used to define the parameters to set scan response data for the BLE scan request,
 * which includes the scan response data length.
 */
typedef struct rsi_ble_req_scanrsp_data_s {
  /** Scan response data length */
  uint8_t data_len;
  /** Scan response data */
  uint8_t scanrsp_data[31];
} rsi_ble_req_scanrsp_data_t;

//Scan command structure
/**
 * @brief Structure represents a BLE scan request.
 */
typedef struct rsi_ble_req_scan_s {

  /** Scanning Status 
-
 
      0 - disable 
-
 1 - enable*/
  uint8_t status;

  /** Scanning type 
-

  SCAN_TYPE_ACTIVE                              0x01 
-

  SCAN_TYPE_PASSIVE                             0x00 */
  uint8_t scan_type;

  /** To filter incoming advertising reports 
-

   FILTERING_DISABLED = 0 (default) 
-

   ACCEPTLIST_FILTERING = 1
   @note     In order to allow only acceptlisted devices, need to add bd_addr
   into acceptlist by calling @ref rsi_ble_addto_acceptlist() API  */
  uint8_t filter_type;

  /** Address type of the local device 
-

      #define LE_PUBLIC_ADDRESS                  0x00 
-

      #define LE_RANDOM_ADDRESS                  0x01 
-

      #define LE_RESOLVABLE_PUBLIC_ADDRESS       0x02 
-

      #define LE_RESOLVABLE_RANDOM_ADDRESS       0x03  */
  uint8_t own_addr_type;

  /** Scan interval 
-

     This is defined as the time interval from when the Controller started
	 its last LE scan until it begins the subsequent LE scan. 
-

     Range: 0x0004 to 0x4000 */
  uint16_t scan_int;

  /** Scan window 
-

      The duration of the LE scan. LE_Scan_Window should be less than or equal to LE_Scan_Interval 
-

      Range: 0x0004 to 0x4000 */
  uint16_t scan_win;

} rsi_ble_req_scan_t;

//ENCRYPT COMMAND STRUCTURE

/**
 * @brief Structure representing the BLE encryption request.
 *
 * This structure is used to define the parameters for the BLE encryption request,
 * which includes the encryption key and the data to be encrypted.
 */
typedef struct rsi_ble_encrypt_s {
  /** Encryption key (16 bytes) */
  uint8_t key[16];
  /** Data to be encrypted (16 bytes) */
  uint8_t data[16];
} rsi_ble_encrypt_t;

/**
 * @brief Structure representing a data packet.
 *
 * This structure is used to define the parameters for a data packet,
 * which includes an array to hold the data packet, up to 1024 bytes.
 */
typedef struct rsi_data_packet_s {
  /** Array to hold the data packet, up to 1024 bytes */
  uint8_t data[1024];
} rsi_data_packet_t;

//accept list structure

/**
 * @brief Structure representing the BLE accept list.
 *
 * This structure is used to define the parameters for the BLE accept list,
 * which includes the operation to add or delete the address, the device address, and the address type.
 */
typedef struct rsi_ble_accept_list_s {
  /** Bit used to add or delete the address from/to the allow list */
  uint8_t addordeltowhitlist;
  /** Address of the device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Address type */
  uint8_t bdaddressType;
} rsi_ble_accept_list_t;

//Connect command structure

/**
 * @brief Structure representing the BLE connection request.
 *
 * This structure is used to define the parameters for a BLE connection request,
 * which includes the address type and address of the device to connect, scan interval, scan window,
 * connection interval, connection latency, and supervision timeout.
 */
typedef struct rsi_ble_req_conn_s {
  /** Address type of the device to connect */
  uint8_t dev_addr_type;
  /** Address of the device to connect */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Scan interval */
  uint16_t le_scan_interval;
  /** Scan window */
  uint16_t le_scan_window;
  /** Minimum connection interval */
  uint16_t conn_interval_min;
  /** Maximum connection interval */
  uint16_t conn_interval_max;
  /** Connection latency */
  uint16_t conn_latency;
  /** Supervision timeout */
  uint16_t supervision_tout;
} rsi_ble_req_conn_t;

/**
 * @brief Structure representing the BLE enhanced connection request.
 *
 * This structure is used to define the parameters for a BLE enhanced connection request,
 * which includes the address type and address of the device to connect, filter policy, own address type,
 * scan interval, scan window, connection interval, connection latency, supervision timeout,
 * and connection event length.
 */
typedef struct rsi_ble_req_enhance_conn_s {
  /** Address type of the device to connect */
  uint8_t dev_addr_type;
  /** Address of the device to connect */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Filter policy */
  uint8_t filter_policy;
  /** Own address type */
  uint8_t own_addr_type;
  /** Scan interval */
  uint16_t le_scan_interval;
  /** Scan window */
  uint16_t le_scan_window;
  /** Minimum connection interval */
  uint16_t conn_interval_min;
  /** Maximum connection interval */
  uint16_t conn_interval_max;
  /** Connection latency */
  uint16_t conn_latency;
  /** Supervision timeout */
  uint16_t supervision_tout;
  /** Minimum connection event length */
  uint16_t min_ce_length;
  /** Maximum connection event length */
  uint16_t max_ce_length;
} rsi_ble_req_enhance_conn_t;

//Disconnect command structure

/**
 * @brief Structure represents the BLE request to disconnect.
 *
 * This structure is used to define the parameters for the BLE request to disconnect,
 * which includes the device address, and the type of disconnect operation.
 */
typedef struct rsi_ble_req_disconnect_s {
  /** Address of the device to disconnect */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */

/// Indicates compatibility mode for BLE operations.
#define COMPATABILITY 0
/// Command to cancel an ongoing BLE connection attempt.
#define BLE_CONNECT_CANCEL 1
/// Command to disconnect an established BLE connection.
#define BLE_DISCONNECT 2
  /** @} */
  /** Type of the disconnect operation */
  uint8_t type;
} rsi_ble_req_disconnect_t;

//SMP protocol structures

//start encryption cmd structures

/**
 * @brief Structure representing the BLE start encryption request.
 *
 * This structure is used to define the parameters for a BLE start encryption request,
 * which includes the address of the connected device, the remote device's Encrypted Diversifier (EDIV) value,
 * Random (RAND)  value, and Long Term Key (LTK) value.
 */
typedef struct rsi_ble_start_encryption_s {
  /** Address of the connected device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Remote device EDIV value */
  uint16_t ediv;
  /** Remote device RAND value  */
  uint8_t rand[8];
  /** Remote device LTK value */
  uint8_t ltk[16];
} rsi_ble_strat_encryption_t;

//SMP Pair Request command structure = 0x007C

/**
 * @brief Structure represents the BLE Security Manager Protocol (SMP) pairing request.
 *
 * This structure is used to define the parameters for a BLE SMP pairing request,
 * which includes the device address, IO capability, and MITM (Man-In-The-Middle) protection requirement.
 */
typedef struct rsi_ble_req_smp_pair_s {
  /** Device address for the pairing request */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** IO capability of the device */
  uint8_t io_capability;
  /** MITM (Man-In-The-Middle) protection requirement */
  uint8_t mitm_req;
} rsi_ble_req_smp_pair_t;

//SMP Response command structure = 0x007D

/**
 * @brief Structure represents the BLE Security Manager Protocol (SMP) response.
 *
 * This structure is used to define the parameters for a BLE SMP response,
 * which includes the device address, IO capability, and Man-In-The-Middle (MITM) protection requirement.
 */
typedef struct rsi_ble_smp_response_s {
  /** Device address for the SMP response */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** IO capability of the device */
  uint8_t io_capability;
  /** MITM protection requirement */
  uint8_t mitm_req;
} rsi_ble_smp_response_t;

//SMP Passkey command structure, cmd_ix - 0x007E

/**
 * @brief Structure representing the BLE SMP (Security Manager Protocol) passkey.
 *
 * This structure is used to define the parameters for a BLE SMP passkey,
 * including the device address, reserved bytes for future use, and the passkey used for pairing.
 */
typedef struct rsi_ble_smp_passkey_s {
  /** Address of the device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Reserved for future use (2 bytes) */
  uint8_t reserved[2];
  /** Passkey used for pairing */
  uint32_t passkey;
} rsi_ble_smp_passkey_t;

//LE ping get auth payload timeout command structure, cmd_ix - 0x00A1

/**
 * @brief Structure represents the BLE command to get the LE ping timeout.
 *
 * This structure is used to define the parameters for a BLE command to get the LE ping timeout,
 * which includes the remote device address.
 */
typedef struct rsi_ble_get_le_ping_timeout_s {
  /** Remote device address (6 bytes) */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_ble_get_le_ping_timeout_t;

//LE ping get auth payload timeout command structure, cmd_ix - 0x00A2
/**
 * @brief Structure represents the BLE response for getting LE ping timeout.
 *
 * This structure is used to define the parameters for the BLE response to get the LE ping timeout,
 * which includes the address of the connected device, and the LE ping timeout value.
 */
typedef struct rsi_ble_rsp_get_le_ping_timeout_s {
  /** Address of the connected device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** LE ping timeout value */
  uint16_t time_out;
} rsi_ble_rsp_get_le_ping_timeout_t;

//LE ping get auth payload timeout command structure, cmd_ix - 0x00A2
/**
 * @brief Structure representing the BLE request to set LE ping timeout.
 *
 * This structure is used to define the parameters for a BLE request to set the LE ping timeout,
 * which includes the address of the connected device, and the LE ping timeout value to be set.
 */
typedef struct rsi_ble_set_le_ping_timeout_s {
  /** Address of the connected device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** LE ping timeout value to be set */
  uint16_t time_out;
} rsi_ble_set_le_ping_timeout_t;

/**
 * @brief Structure representing the BLE resolving list entry.
 *
 * This structure is used to define the parameters for a BLE resolving list entry,
 * which includes the type of process (for example, add, remove, clear), the address type and address of the remote device,
 * and the Identity Resolving Keys (IRKs) of both the peer and local devices.
 */
typedef struct rsi_ble_resolvlist_s {
  /** Type of process (for example, add, remove, clear) */
  uint8_t process_type;
  /** Address type of the remote device */
  uint8_t remote_dev_addr_type;
  /** Address of the remote device */
  uint8_t remote_dev_addr[RSI_DEV_ADDR_LEN];
  /** Identity Resolving Key (IRK) of the peer device */
  uint8_t peer_irk[16];
  /** Identity Resolving Key (IRK) of the local device */
  uint8_t local_irk[16];
} rsi_ble_resolvlist_t;

//LE Get resolvlist size command structure, cmd_ix - 0x00AE

/**
 * @brief Structure represents the BLE request to get the size of the resolving list.
 *
 * This structure is used to define the parameter for a BLE request to get the size of the resolving list.
 */
typedef struct rsi_ble_get_resolving_list_size_s {
  /** Size of the resolving list */
  uint8_t size;
} rsi_ble_get_resolving_list_size_t;

/**
 * @brief Structure represents the BLE request to enable or disable address resolution.
 *
 * This structure is used to define the parameters for a BLE request to enable or disable address resolution,
 * which includes the enable flag, reserved field for future use, and the timeout value for address resolution.
 */
typedef struct rsi_ble_set_addr_resolution_enable_s {
  /** Enable or disable address resolution */
  uint8_t enable;
  /** Reserved for future use */
  uint8_t reserved;
  /** Timeout value for address resolution */
  uint16_t tout;
} rsi_ble_set_addr_resolution_enable_t;

//LE conn params update command structure, cmd_ix - 0x007A

/**
 * @brief Structure represents the BLE command to update connection parameters.
 *
 * This structure is used to define the parameters for a BLE command to update connection parameters,
 * which includes the address of the connected device, minimum and maximum connection intervals,
 * peripheral latency, and supervision timeout.
 */
typedef struct rsi_ble_cmd_conn_params_update_s {
  /** Address of the connected device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Minimum connection interval */
  uint16_t min_interval;
  /** Maximum connection interval */
  uint16_t max_interval;
  /** Peripheral latency */
  uint16_t latency;
  /** Supervision timeout */
  uint16_t timeout;
} rsi_ble_cmd_conn_params_update_t;

//LE read phy request command structure, cmd_ix - 0x00B0

/**
 * @brief Structure represents the BLE request to read PHY.
 *
 * This structure is used to define the parameter for a BLE request to read the PHY,
 * which includes the address of the connected device.
 */
typedef struct rsi_ble_req_read_phy_s {
  /** Address of the connected device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_ble_req_read_phy_t;

//LE set phy command response structure, cmd_ix - 0x00B1

/**
 * @brief Structure represents the BLE request to set PHY.
 *
 * This structure is used to define the parameters for a BLE request to set the PHY,
 * which includes the address of the connected device, all PHYs preference, preferred TX PHY,
 * preferred RX PHY, reserved field for future use, and PHY options.
 */
typedef struct rsi_ble_set_phy_s {
  /** Address of the connected device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** All PHYs preference */
  uint8_t all_phy;
  /** Preferred TX PHY */
  uint8_t tx_phy;
  /** Preferred RX PHY */
  uint8_t rx_phy;
  /** Reserved for future use */
  uint8_t reserved;
  /** PHY options */
  uint16_t phy_options;
} rsi_ble_set_phy_t;

//LE set data length command response structure, cmd_ix - 0x00B2

/**
 * @brief Structure represents the BLE request to set data length.
 *
 * This structure is used to define the parameters for a BLE request to set the data length,
 * which includes the address of the connected device, the maximum number of payload octets that the local device would send,
 * and the maximum time that the local device would take to send the payload.
 */
typedef struct rsi_ble_setdatalength_s {
  /** Address of the connected device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Maximum number of payload octets that the local device will send */
  uint16_t txoctets;
  /** Maximum time that the local device will take to send the payload */
  uint16_t txtime;
} rsi_ble_setdatalength_t;

//LE set privacy mode command structure, cmd_ix - 0x00B4

/**
 * @brief Structure represents the BLE request to set privacy mode.
 *
 * This structure is used to define the parameters for a BLE request to set the privacy mode,
 * which includes the address type and address of the remote device, and the privacy mode to be set.
 */
typedef struct rsi_ble_set_privacy_mode_s {
  /** Address type of the remote device */
  uint8_t remote_dev_addr_type;
  /** Address of the remote device */
  uint8_t remote_dev_addr[RSI_DEV_ADDR_LEN];
  /** Privacy mode to be set */
  uint8_t privacy_mode;
} rsi_ble_set_privacy_mode_t;

//LE cbfc connection req command structure, cmd_ix - 0x00B5

/**
 * @brief Structure represents the BLE Credit Based Flow Control (CBFC) connection request.
 *
 * This structure is used to define the parameters for a BLE CBFC connection request,
 * which includes the address of the remote device and the Protocol/Service Multiplexer (PSM) value.
 */
typedef struct rsi_ble_cbfc_conn_req_s {
  /** Address of the remote device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Protocol/Service Multiplexer (PSM) value */
  uint8_t psm;
} rsi_ble_cbfc_conn_req_t;

//LE cbfc connection resp command structure, cmd_ix - 0x00B6

/**
 * @brief Structure represents the BLE Credit Based Flow Control (CBFC) connection response.
 *
 * This structure is used to define the parameters for a BLE CBFC connection response,
 * which includes the address of the remote device, the Local Channel Identifier (LCID), and the result of the connection request.
 */
typedef struct rsi_ble_cbfc_conn_resp_s {
  /** Address of the remote device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Local Channel Identifier (LCID) */
  uint16_t lcid;
  /** Result of the connection request */
  uint8_t result;
} rsi_ble_cbfc_conn_resp_t;

//LE cbfc data TX command structure, cmd_ix - 0x00B7

/**
 * @brief Structure represents the BLE Credit Based Flow Control (CBFC) data transmission.
 *
 * This structure is used to define the parameters for a BLE CBFC data transmission,
 * which includes the address of the remote device, the Local Channel Identifier (LCID),
 * the length of the data to be transmitted, and the data to be transmitted.
 */
typedef struct rsi_ble_cbfc_data_tx_s {
  /** Address of the remote device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Local Channel Identifier (LCID) */
  uint16_t lcid;
  /** Length of the data to be transmitted */
  uint16_t len;
  /** Data to be transmitted */
  uint8_t data[RSI_DEV_ATT_LEN];
} rsi_ble_cbfc_data_tx_t;

//LE cbfc disconn command structure, cmd_ix - 0x00B8

/**
 * @brief Structure represents the BLE CBFC (Credit Based Flow Control) disconnection.
 *
 * This structure is used to define the parameters for a BLE CBFC disconnection,
 * which includes the address of the remote device and the Local Channel Identifier (LCID).
 */
typedef struct rsi_ble_cbfc_disconn_s {
  /** Address of the remote device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Local Channel Identifier (LCID) */
  uint16_t lcid;
} rsi_ble_cbfc_disconn_t;

//LE RX text mode (cmd), cmd_ix = 0x00BB

/**
 * @brief Structure represents the BLE RX test mode parameters.
 *
 * This structure is used to define the parameters for a BLE RX test mode,
 * which includes the RX channel, PHY, and modulation type to be used for the test.
 */
typedef struct rsi_ble_rx_test_mode_s {
  /** RX channel to be used for the test */
  uint8_t rx_channel;
  /** PHY to be used for the test */
  uint8_t phy;
  /** Modulation type to be used for the test */
  uint8_t modulation;
} rsi_ble_rx_test_mode_t;

//LE TX test mode (cmd), cmd_ix = 0x00BC
/**
 * @brief Structure represents the BLE TX test mode parameters.
 *
 * This structure is used to define the parameters for a BLE TX test mode,
 * which includes the TX channel, PHY, length of the data to be transmitted, and data mode to be used for the test.
 */
typedef struct rsi_ble_tx_test_mode_s {
  /** TX channel to be used for the test */
  uint8_t tx_channel;
  /** PHY to be used for the test */
  uint8_t phy;
  /** Length of the data to be transmitted */
  uint8_t tx_len;
  /** Data mode to be used for the test */
  uint8_t tx_data_mode;
} rsi_ble_tx_test_mode_t;

//LE End test mode (cmd), cmd_ix = 0x00BD

/**
 * @brief Structure represents the BLE end test mode parameters.
 *
 * This structure is used to define the parameter for a BLE end test mode.
 * Number of TX / RX packets received are displayed when test is stopped.
 */
typedef struct rsi_ble_end_test_mode_s {
  /** Number of TX / RX packets received when test is stopped */
  uint16_t num_of_pkts;
} rsi_ble_end_test_mode_t;

/**
 * @brief Structure represents the BLE request to set LE Long Term Key (LTK) request reply.
 *
 * This structure is used to define the parameters for a BLE request to set the LE LTK request reply,
 * which includes the address of the remote device, the type of reply (for example, positive or negative), 
 * and the local Long Term Key (LTK).
 */
typedef struct rsi_ble_set_le_ltkreqreply_s {
  /** Address of the remote device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Type of reply (e.g., positive or negative) */
  uint8_t replytype;
  /** Local Long Term Key (LTK) (16 bytes) */
  uint8_t localltk[16];
} rsi_ble_set_le_ltkreqreply_t;

//SMP Pairing Failed (cmd), cmd_ix = 0x0111

/**
 * @brief Structure represents the BLE Security Manager Protocol (SMP) pairing failure request.
 *
 * This structure is used to define the parameters for a BLE SMP pairing failure request,
 * which includes the address of the remote device and the reason for the pairing failure.
 */
typedef struct rsi_ble_req_smp_pair_failed_s {
  /** Address of the remote device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Reason for the pairing failure */
  uint8_t reason;
} rsi_ble_req_smp_pair_failed_t;

// GATT structures

// GATT Profiles list request structure

/**
 * @brief Structure representing the BLE request for profiles list.
 *
 * This structure is used to define the parameters for a BLE request to get the profiles list,
 * which includes the remote device address, the handle from which the profiles search would start,
 * and the handle at which the profiles search will stop.
 */
typedef struct rsi_ble_req_profiles_list_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Profiles search will start from this handle */
  uint16_t start_handle;
  /** Profiles search will stop at this handle */
  uint16_t end_handle;
} rsi_ble_req_profiles_list_t;

// GATT Profile request structure

/**
 * @brief Structure represents the BLE request for a specific profile.
 *
 * This structure is used to define the parameters for a BLE request to get a specific profile,
 * which includes the remote device address, reserved bytes for future use, and the profile UUID.
 */
typedef struct rsi_ble_req_profile_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Reserved for future use */
  uint8_t reserved[2];
  /** Profile UUID (20 bytes) */
  uuid_t profile_uuid;
} rsi_ble_req_profile_t;

// GATT Profile response  structure
//profile_descriptors_t;

// GATT multiple characteristic services request structure

/**
 * @brief Structure represents the BLE request for characteristic services.
 *
 * This structure is used to define the parameters for a BLE request to get characteristic services,
 * which includes the remote device address, the handle from which the search will start, 
 * and the handle at which the search will end.
 */
typedef struct rsi_ble_req_char_services_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Search will start from this handle */
  uint8_t start_handle[2];
  /** Search will end at this handle */
  uint8_t end_handle[2];
} rsi_ble_req_char_services_t;

// GATT include service query request structure

/**
 * @brief Structure represents the BLE request for included services.
 *
 * This structure is used to define the parameters for a BLE request to get included services,
 * includes the remote device address, the handle from which the search would start,
 * and the handle at which the search would end.
 */
typedef struct rsi_ble_req_inc_services_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Search will start from this handle */
  uint8_t start_handle[2];
  /** Search will end at this handle */
  uint8_t end_handle[2];
} rsi_ble_req_inc_services_t;

// GATT read value by UUID request structure

/**
 * @brief Structure represents the BLE request for characteristic value by UUID.
 *
 * This structure is used to define the parameters for a BLE request to get a characteristic value by UUID,
 * which includes the remote device address, the handle from which the search would start,
 * the handle at which the search would end, reserved bytes for future use, and the search UUID value.
 */
typedef struct rsi_ble_req_char_val_by_uuid_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Search will start from this handle */
  uint8_t start_handle[2];
  /** Search will end at this handle */
  uint8_t end_handle[2];
  /** Reserved for future use */
  uint8_t reserved[2];
  /** Search UUID value (20 bytes) */
  uuid_t char_uuid;
} rsi_ble_req_char_val_by_uuid_t;

// GATT read value by UUID response structure
//rsi_ble_resp_att_value_t

// GATT multiple attribute descriptors request structure

/**
 * @brief Structure represents the BLE request for attribute descriptors.
 *
 * This structure is used to define the parameters for a BLE request to get attribute descriptors,
 * which includes the remote device address, the handle from which the search would start,
 * and the handle at which the search would end.
 */
typedef struct rsi_ble_req_att_descs_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Search will start from this handle */
  uint8_t start_handle[2];
  /** Search will end at this handle */
  uint8_t end_handle[2];
} rsi_ble_req_att_descs_t;

// GATT attribute value request structure
/**
 * @brief Structure represents the BLE request for attribute value.
 *
 * This structure is used to define the parameters for a BLE request to get an attribute value,
 * which includes the remote device address and the attribute handle.
 */
typedef struct rsi_ble_req_att_value_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Attribute handle */
  uint8_t handle[2];
} rsi_ble_req_att_value_t;

// GATT multiple attribute values request structure

/**
 * @brief Structure represents the BLE request for multiple attribute values.
 *
 * This structure is used to define the parameters for a BLE request to get multiple attribute values,
 * which includes the remote device address, the number of attribute handles, reserved bytes for future use,
 * and the list of attribute handles.
 */
typedef struct rsi_ble_req_multiple_att_val_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Number of attribute handles */
  uint8_t num_of_handles;
  /** Reserved for future use */
  uint8_t reserved;
  /** List of attribute handles */
  uint16_t handles[RSI_BLE_MAX_RESP_LIST];
} rsi_ble_req_multi_att_values_t;

// GATT multiple attribute values response structure
//rsi_ble_resp_att_value_t

// GATT long attribute value request structure

/**
 * @brief Structure represents the BLE request for a long attribute value.
 *
 * This structure is used to define the parameters for a BLE request to get a long attribute value,
 * which includes the remote device address, the attribute handle, and the attribute value offset.
 */
typedef struct rsi_ble_req_long_att_value_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Attribute handle */
  uint16_t handle;
  /** Attribute value offset */
  uint16_t offset;
} rsi_ble_req_long_att_value_t;

// GATT long attribute value response structure
//rsi_ble_resp_att_value_t

// GATT write attribute value request structure

/**
 * @brief Structure represents the BLE request to set an attribute value.
 *
 * This structure is used to define the parameters for a BLE request to set an attribute value,
 * which includes the remote device address, the attribute handle, the length of the attribute value,
 * and the attribute value itself. The module would receive the acknowledgement from the remote device.
 * 
 */
typedef struct rsi_ble_set_att_val_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Attribute handle */
  uint8_t handle[2];
  /** Length of the attribute value */
  uint8_t length;
  /** Attribute value */
  uint8_t att_value[RSI_DEV_ATT_LEN];
} rsi_ble_set_att_value_t;
// GATT write attribute value without ack request structure

/**
 * @brief Structure represents the BLE command to set an attribute value.
 *
 * This structure is used to define the parameters for a BLE command to set an attribute value,
 * which includes the remote device address, the attribute handle, the length of the attribute value,
 * and the attribute value itself. The module won't receive the acknowledgement from the remote device.
 */
typedef struct rsi_ble_set_att_cmd_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Attribute handle */
  uint8_t handle[2];
  /** Length of the attribute value */
  uint8_t length;
  /** Attribute value */
  uint8_t att_value[RSI_DEV_ATT_LEN];
} rsi_ble_set_att_cmd_t;

// GATT write long attribute value request structure

/**
 * @brief Structure represents the BLE command to set a long attribute value.
 *
 * This structure is used to define the parameters for a BLE command to set a long attribute value,
 * which includes the remote device address, the attribute handle, the attribute value offset,
 * the length of the attribute value, and the attribute value itself.
 */
typedef struct rsi_ble_set_long_att_val_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Attribute handle */
  uint8_t handle[2];
  /** Attribute value offset */
  uint8_t offset[2];
  /** Length of the attribute value */
  uint8_t length;
  /** Attribute value */
  uint8_t att_value[RSI_DEV_ATT_LEN];
} rsi_ble_set_long_att_value_t;

// GATT prepare write value request structure

/**
 * @brief Structure represents the BLE request to prepare a write operation.
 *
 * This structure is used to define the parameters for a BLE request to prepare a write operation,
 * which includes the remote device address, the attribute handle, the attribute value offset,
 * the length of the attribute value, and the attribute value itself.
 */
typedef struct rsi_ble_req_prepare_write_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Attribute handle */
  uint8_t handle[2];
  /** Attribute value offset */
  uint8_t offset[2];
  /** Length of the attribute value */
  uint8_t length;
  /** Attribute value */
  uint8_t att_value[RSI_DEV_ATT_LEN];
} rsi_ble_req_prepare_write_t;

// GATT execute write request structure

/**
 * @brief Structure represents the BLE request to execute a write operation.
 *
 * This structure is used to define the parameters for a BLE request to execute a write operation,
 * which includes the remote device address and the execute flag indicating whether to write or not.
 */
typedef struct rsi_ble_req_execute_write_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Execute flag (write/don't write) */
  uint8_t flag;
} rsi_ble_req_execute_write_t;

//RSI_BLE_CONN_PARAM_RESP_CMD, cmd_id: 0x0105

/**
 * @brief Structure represents the BLE command response for connection parameters.
 *
 * This structure is used to define the parameters for a BLE command response to a connection parameter request,
 * which includes the remote device address and the status indicating whether to accept or reject the request.
 */
typedef struct rsi_ble_cmd_conn_param_resp {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Accept or reject the remote connection parameter request */
  uint8_t status;
} rsi_ble_cmd_conn_param_resp_t;

//GATT Events

//GATT Service
// add new GATT service request structure

/**
 * @brief Structure represents the BLE request to add a service.
 *
 * This structure is used to define the parameters for a BLE request to add a service,
 * which includes the service UUID, the number of attributes in the service, 
 * and the total size of the attributes' values (data).
 */
typedef struct rsi_ble_req_add_serv_s {
  /** Service UUID */
  uuid_t service_uuid;
  /** Number of attributes in the service */
  uint8_t num_of_attributes;
  /** Total size of the attributes' values (data) */
  uint8_t total_att_datasize;
} rsi_ble_req_add_serv_t;

// write or change local attribute value request structure

/**
 * @brief Structure represents the BLE command to set a local attribute value.
 *
 * This structure is used to define the parameters for a BLE command to set a local attribute value,
 * which includes the attribute handle, the length of the attribute value, and the attribute value itself.
 */
typedef struct rsi_ble_set_local_att_value_s {
  /** Attribute handle */
  uint16_t handle;
  /** Attribute value length */
  uint16_t data_len;
  /** Attribute value (data) */
  uint8_t data[RSI_DEV_ATT_LEN];
} rsi_ble_set_local_att_value_t;

// write or change local attribute value request structure

/**
 * @brief Structure represents the BLE notification for an attribute value.
 *
 * This structure is used to define the parameters for a BLE notification of an attribute value,
 * which includes the remote device address, the attribute handle, the length of the attribute value,
 * and the attribute value itself.
 */
typedef struct rsi_ble_notify_att_value_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Attribute handle */
  uint16_t handle;
  /** Attribute value length */
  uint16_t data_len;
  /** Attribute value (data) */
  uint8_t data[RSI_DEV_ATT_LEN];
} rsi_ble_notify_att_value_t;

// set wo_resp and notify buffer info

/**
 * @brief Structure represents the BLE configuration for write without response notification buffer.
 *
 * This structure is used to define the parameters for configuring the buffer for write without response notifications,
 * which includes the remote device address, the buffer configuration mode, and the buffer count.
 */
typedef struct rsi_ble_set_wo_resp_notify_buf_info_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Buffer config mode: 0 for small buffer mode, 1 for big buffer mode */
  uint8_t buf_mode;
  /** Buffer count */
  uint8_t buf_count;
} rsi_ble_set_wo_resp_notify_buf_info_t;

// indicate confirmation structure

/**
 * @brief Structure represents the BLE indication confirmation.
 *
 * This structure is used to define the parameters for a BLE indication confirmation,
 * which includes the remote device address.
 */
typedef struct rsi_ble_indicate_confirm_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_ble_indicate_confirm_t;

// read local attribute value request structure

/**
 * @brief Structure represents the BLE request to get a local attribute value.
 *
 * This structure is used to define the parameters for a BLE request to get a local attribute value,
 * which includes the attribute handle.
 */
typedef struct rsi_ble_get_local_att_value_s {
  /** Attribute handle */
  uint16_t handle;
} rsi_ble_get_local_att_value_t;

/**
 * @brief Structure represents the BLE GATT read response.
 *
 * This structure is used to define the parameters for a BLE GATT read response,
 * which includes the remote device address, the type of the read response, reserved field for future use,
 * the length of the attribute value, and the attribute value itself.
 */
typedef struct rsi_ble_gatt_read_response_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Type of the read response */
  uint8_t type;
  /** Reserved for future use */
  uint8_t reserved;
  /** Attribute value length */
  uint16_t data_len;
  /** Attribute value (data) */
  uint8_t data[RSI_DEV_ATT_LEN];
} rsi_ble_gatt_read_response_t;

// Att write/ execute write response cmd = 0x010A

/**
 * @brief Structure represents the BLE GATT write response.
 *
 * This structure is used to define the parameters for a BLE GATT write response,
 * which includes the remote device address and the response type.
 */
typedef struct rsi_ble_gatt_write_response_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Response type */
  uint8_t type;
} rsi_ble_gatt_write_response_t;

// Att prepare write response cmd = 0x010B

/**
 * @brief Structure represents the BLE GATT prepare write response.
 *
 * This structure is used to define the parameters for a BLE GATT prepare write response,
 * which includes the remote device address, the attribute handle, the attribute value offset,
 * the length of the attribute value, and the attribute value itself.
 */
typedef struct rsi_ble_gatt_prepare_write_response_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Attribute handle */
  uint16_t handle;
  /** Attribute value offset */
  uint16_t offset;
  /** Attribute value length */
  uint16_t data_len;
  /** Attribute value (data) */
  uint8_t data[RSI_DEV_ATT_LEN];
} rsi_ble_gatt_prepare_write_response_t;

/**
 * @brief Structure representing the BLE command to set the local Identity Resolving Key (IRK).
 *
 * This structure is used to define the parameters for a BLE command to set the local IRK,
 * including the local device IRK.
 */
typedef struct rsi_ble_set_local_irk_s {
  /** Local device IRK (16 bytes) */
  uint8_t irk[16];
} rsi_ble_set_local_irk_t;

// BLE GAP extended callback ids
/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */

/**
 * 
 * @brief Enumeration represents the BLE GAP extended callback types.
 *
 * This enumeration defines the possible callback types for the BLE GAP extended feature.
 *
 */
typedef enum rsi_ble_gap_extended_callbacks_s {
  /** Callback type for remote device information events */
  RSI_BLE_ON_REMOTE_DEVICE_INFORMATION = 1,
  /** Callback type for RCP (Remote Control Protocol) events */
  RSI_BLE_ON_RCP_EVENT = 2,
} rsi_ble_gap_extended_callbacks_t;
//attribute codes
/// Attribute Protocol (ATT) Exchange MTU request.
#define RSI_BLE_ATT_EXCHANGE_MTU_REQUEST 0x02
/// Attribute Protocol (ATT) Find Information request.
#define RSI_BLE_ATT_FIND_INFORMATION_REQUEST 0x04
/// Attribute Protocol (ATT) Find By Type Value request.
#define RSI_BLE_ATT_FIND_BY_TYPE_VALUE_REQUEST 0x06
/// Attribute Protocol (ATT) Read By Type request.
#define RSI_BLE_ATT_READ_BY_TYPE_REQUEST 0x08
/// Attribute Protocol (ATT) Read request.
#define RSI_BLE_ATT_READ_REQUEST 0x0A
/// Attribute Protocol (ATT) Read Blob request.
#define RSI_BLE_ATT_READ_BLOB_REQUEST 0x0C
/// Attribute Protocol (ATT) Read Multiple request.
#define RSI_BLE_ATT_READ_MULTIPLE_REQUEST 0x0E
/// Attribute Protocol (ATT) Read By Group Type request.
#define RSI_BLE_ATT_READ_BY_GROUP_TYPE_REQUEST 0x10
/// Attribute Protocol (ATT) Write request.
#define RSI_BLE_ATT_WRITE_REQUEST 0x12
/// Attribute Protocol (ATT) Prepare Write request.
#define RSI_BLE_ATT_PREPARE_WRITE_REQUEST 0x16
/// Attribute Protocol (ATT) Execute Write request.
#define RSI_BLE_ATT_EXECUTE_WRITE_REQUEST 0x18
/** @} */

// Att error response cmd = 0x00C1

/**
 * @brief Structure represents the BLE attribute error response.
 *
 * This structure is used to define the parameters for a BLE event
 * that involves an error response for an attribute operation from a remote device.
 */
typedef struct rsi_ble_att_error_response_s {
  /** Address of the remote device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** The request opcode for the BLE operation */
  uint8_t req_opcode;
  /** The attribute handle for the Bluetooth Low Energy (BLE) device */
  uint16_t att_handle;
  /** The error code for the operation */
  uint8_t err_code;
} rsi_ble_att_error_response_t;

/**
 * @brief Structure represents the BLE GATT remove service request.
 *
 * This structure is used to define the parameters for a BLE event
 * that involves removing a GATT service from a remote device.
 */
typedef struct rsi_ble_gatt_remove_serv_s {
  /** Service handler */
  uint32_t serv_hndler;
} rsi_ble_gatt_remove_serv_t;

/**
 * @brief Structure represents the BLE GATT command to remove an attribute.
 *
 * This structure is used to define the parameters for a BLE GATT command to remove an attribute,
 * which includes the service handler and the attribute handle.
 */
typedef struct rsi_ble_gatt_remove_att_s {
  /** Service handler */
  uint32_t serv_hndler;
  /** Attribute handle */
  uint16_t att_hndl;
} rsi_ble_gatt_remove_att_t;

// rf type command structure

/**
 * @brief Structure represents the BLE vendor-specific RF type command.
 *
 * This structure is used to define the parameters for a BLE vendor-specific RF type command,
 * which includes the opcode and the BLE power index.
 */
typedef struct rsi_ble_vendor_rf_type_s {
  /** Opcode */
  uint8_t opcode[2];
  /** BLE power index */
  uint8_t ble_power_index;
} rsi_ble_vendor_rf_type_t;

// rf type command structure

/**
 * @brief Structure represents the BLE MTU exchange request.
 *
 * This structure is used to define the parameters for a BLE MTU exchange request,
 * which includes the remote device address and the requested MTU size.
 */
typedef struct rsi_ble_mtu_exchange_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Requested MTU size */
  uint16_t req_mtu_size;
} rsi_ble_mtu_exchange_t;

// mtu exchange resp command structure

/**
 * @brief Structure represents the BLE MTU exchange response.
 *
 * This structure is used to define the parameters for a BLE MTU exchange response,
 * which includes the Bluetooth device address of the peer device and the requested MTU size from the peer device.
 */
typedef struct rsi_ble_mtu_exchange_resp_s {
  /** The Bluetooth device address of the peer device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** The requested MTU size from the peer device */
  uint16_t req_mtu_size;
} rsi_ble_mtu_exchange_resp_t;

/**
 * @brief Structure represents the BLE Advertising Extensions (AE) command to get the supported number of advertising sets.
 *
 * This structure is used to define the parameters for a BLE AE command to get the supported number of advertising sets,
 * which includes a reserved field.
 */
typedef struct rsi_ble_ae_get_supported_no_of_adv_sets_s {
  /** Reserved field */
  uint16_t reserved;
} SL_ATTRIBUTE_PACKED rsi_ble_ae_get_supported_no_of_adv_sets_t;

/**
 * @brief Structure represents the BLE Advertising Extensions (AE) command to read the supported maximum advertising data.
 *
 * This structure is used to define the parameters for a BLE AE command to read the supported maximum advertising data,
 * which includes a reserved field for future use or alignment.
 */
typedef struct rsi_ble_ae_read_supported_max_adv_data_s {
  /** Reserved field for future use or alignment */
  uint16_t reserved;
} SL_ATTRIBUTE_PACKED rsi_ble_ae_read_supported_max_adv_data_t;

// AE Set Random Address (cmd), cmd_ix =

/**
 * @brief Structure represents the BLE Advertising Extensions (AE) command to set a random address.
 *
 * This structure is used to define the parameters for a BLE AE command to set a random address,
 * which includes the advertising handle and the random address.
 */
typedef struct rsi_ble_ae_set_random_address_s {
  /** Advertising_Handle, used to identify an advertising set, Range: 0x00 to 0xEF */
  uint8_t adv_handle;
  /** Random_Address, The Random Address may be of either Static Address or Private Address */
  uint8_t addr[RSI_DEV_ADDR_LEN];
} SL_ATTRIBUTE_PACKED rsi_ble_ae_set_random_address_t;

//! AE Advertising Params
typedef struct ae_adv_params_s {
  /** uint8_t, Advertising Handle,  Used to identify an Advertising set , Range : 0x00 to 0xEF */
  uint8_t adv_handle;
  /**
  *  uint16_t, Advertising Event Properties, indicates the properties of Advertising Event
 *    ---------------------------------------------------------------------------------------------------------
 *   |       Bit Number    |                    Parameter Description                                          |
 *   ----------------------|-----------------------------------------------------------------------------------
 *   |      0              |                   Connectable Advertising                                         |
 *   |      1              |                   Scannable Advertising                                           |
 *   |      2              |                   Direct Advertising                                              |
 *   |      3              |  High Duty cycle Directed Connectable advertising (≤ 3.75 ms Advertising interval)|
 *   |      4              |                 Use legacy advertising PDUs                                       |
 *   |      5              |         Omit advertiser's address from all PDUs("anonymous advertising")          |
 *   |      6              |         Include Tx Power in the extended header of at least one advertising PDU   |
 * */
  uint16_t adv_event_prop;
  /** uint32_t, Primary Advertising Interval Minimum, Minimum advertising interval for undirected and low duty cycle directed advertising */
  uint32_t primary_adv_intterval_min : 24;
  /** uint32_t, Primary Advertising Interval Maximum, Maximum advertising interval for undirected and low duty cycle directed advertising
   primary_adv_intterval_min <= primary_adv_intterval_max */
  uint32_t primary_adv_intterval_max : 24;
  /** uint8_t, Primary Advertising Channel Map, It specifies on which channel it shall advertise
                    Bit Number          Parameter Description
                       0                   Channel 37 shall be used
                       1                   Channel 38 Shall be used
                       2                   Channel 39 shall be used
  */
  uint8_t primary_adv_chnl_map;
  /**
     uint8_t, Own_Address_type,  Indicates the type of the Address
                  0x00 - Public Device Address
                  0x01 - Random Device Address
                  0x02 - Controller generates the Resolvable Private Address based on the local
                         IRK from the resolving list. If the resolving list contains no matching
                         entry, use the public address.
                  0x03 - Controller generates the Resolvable Private Address based on the local
                         IRK from the resolving list. If the resolving list contains no matching
                         entry, use the random address from LE_Set_Advertising_Set_Random_Address.
  */
  uint8_t own_addr_type;
  /** uint8_t, Peer_Address_Type, Specifies Peer Address Type
                  0x00 - Public Device Address or Public Identity Address
                  0x01 - Random Device Address or Random (static) Identity Address
  */
  uint8_t peer_addr_type;
  /** uint8[6], Peer_Device_Address, Address of the Peer_Address_Type */
  uint8_t peer_dev_addr[RSI_DEV_ADDR_LEN];
  /** uint8_t, Advertising_Filter_Policy
              0x00 - Process scan and connection requests from all devices (that is , the Filter Accept List is not in use).
              0x01 - Process connection requests from all devices and scan requests only from devices that are in the Filter Accept List.
              0x02 - Process scan requests from all devices and connection requests only from devices that are in the Filter Accept List.
              0x03 - Process scan and connection requests only from devices in the Filter Accept List.
  */
  uint8_t adv_filter_policy;
  /** uint8_t Advertising_TX_Power, Advertising TX Power ranges from -127 to +20 and units are in dBm */
  uint8_t adv_tx_power;
  /** uint8_t Primary_Advertising_PHY, This  parameter specifies the PHY used for the periodic advertising.
                 0x01 - Advertiser PHY is LE 1M.
                 0x03 - Advertiser PHY is LE Coded
  */
  uint8_t primary_adv_phy;
  /** uint8_t Secondary_Advertising_Max_Skip
                  0x00  -     AUX_ADV_IND shall be sent prior to the next advertising event.
               0x01 to 0xFF - Maximum advertising events the Controller can skip before sending the AUX_ADV_IND packets on the secondary advertising physical channel.
  */
  uint8_t sec_adv_max_skip;
  /** uint8_t Secondary_Advertising_PHY,  This  parameter specifies the PHY used for the periodic advertising.
                 0x01 - Advertiser PHY is LE 1M.
                 0x02 - Advertiser PHY is LE 2M.
                 0x03 - Advertiser PHY is LE Coded.
  */
  uint8_t sec_adv_phy;
  /** uint8_t Advertising_Sid, Value of the Advertising SID subfield in the ADI field of the PDU, Range : 0x00 to 0x0F */
  uint8_t adv_sid;
  /** uint8_t Scan Request Notification Enable
               0x00 - Scan request notifications disabled.
               0x01 - Scan request notifications enabled.
  */
  uint8_t scan_req_notify_enable;

} SL_ATTRIBUTE_PACKED rsi_ble_ae_adv_params_t;

// AE adv,scan_rsp and periodic data

/**
 * @brief Structure representing the AE (Application Event) data for BLE.
 */
typedef struct rsi_ble_ae_data_s {

  /** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */

  /// Advertising Extension (AE) advertising data.
#define AE_ADV_DATA 0x01
  /// Advertising Extension (AE) scan response data.
#define AE_SCAN_RSP_DATA 0x02
  /** @} */
  /** uint8_t AE_ADV_DATA_TYPE 1, AE_PERIODIC_ADV_DATA_TYPE 2, AE_SCAN_RSP_DATA_TYPE 3 */
  uint8_t type;
  /** uint8_t Advertising Handle, used to identify an Advertising set, Ranges from 0x00 to 0xEF */
  uint8_t adv_handle;
  /** uint8_t Operation
     0x00 - Intermediate fragment of fragmented extended advertising data.
     0x01 - First fragment of fragmented extended advertising data.
     0x02 - Last fragment of fragmented extended advertising data.
     0x03 - Complete extended advertising data.
     0x04 - Unchanged data (just update the Advertising DID).
  */
  uint8_t operation;
  /**
    uint8_t Fragment_Preference, Specifies the controller on where to fragment the Host advertising Data
              0x00 - The Controller may fragment all Host advertising data.
              0x01 - The Controller should not fragment or should minimize fragmentation of Host advertising data.
  */
  uint8_t frag_pref;
  /** uint8_t Data Length, Specifies Advertising_Data_Length , This parameter ranges from 0 to 251 */
  uint8_t data_len;
  /** uint8_t Data ,Specifies Advertising_Data. */
  uint8_t data[0xC8]; //FIXME
} SL_ATTRIBUTE_PACKED rsi_ble_ae_data_t;

//! AE Advertising enable
typedef struct rsi_ble_ae_adv_enabel_s {
  /**
      uint8_t Enable, This parameter specifies whether to disable or Enable Advertising
               0x00 - Advertising is disabled.
               0x01 - Advertising is enabled.
  */
  uint8_t enable;
  /**
     uint8_t Num_of_Sets , Indicates the number of Advertising sets to be disabled or enabled for Advertising
              0x00         - Disable all advertising sets.
              0x01 to 0x3F - Number of advertising sets to enable or disable.
  */
  uint8_t no_of_sets;
  /** uint8_t Advertising_Handle, used to identify Advertising set, Ranges from 0x00 to 0xEF */
  uint8_t adv_handle;
  /**
     uint16_t Duration, specifies the duration to continue advertising
      0x00 - No Advertising.
      0x0001 to 0xFFFF , Advertising Duration.
  */
  uint16_t duration;
  /** uint8_t Maximum Extended Advertising Events, It specifies the Maximum number of extended advertising events the Controller should
     attempt to send prior to terminating the extended advertising */
  uint8_t max_ae_events;
} SL_ATTRIBUTE_PACKED rsi_ble_ae_adv_enable_t;

//AE adv set clear/remove

/**
 * @brief Structure for Advertising Event (AE) Advertisement (ADV) Set, Clear, or Remove.
 */
typedef struct rsi_ble_ae_adv_set_clear_or_remove_s {

  /**
     type - Specifies whether to remove or clear the advertising sets.
      {1} - clear
      {2} - remove
  */
  uint8_t type;
  /** uint8_t Advertising_Handle, used to identify Advertising set, Ranges from 0x00 to 0xEF */
  uint8_t adv_handle;
} SL_ATTRIBUTE_PACKED rsi_ble_ae_adv_set_clear_or_remove_t;

//AE periodic adv params

/**
 * @brief Structure representing the parameters for periodic advertising in BLE.
 */
typedef struct ae_periodic_adv_params {

  /** uint8_t, Advertising Handle , this parameter identifies the advertising set whose periodic advertising parameters are being configured
  * Rang : 0x00 to 0xEF */
  uint8_t adv_handle;
  /** uint16_t, Minimum Interval,Minimum advertising interval for periodic advertising.Range: 0x0006 to 0xFFFF */
  uint16_t min_interval;
  /**uint16_t, Maximum Interval,Maximum advertising interval for periodic advertising.Range: 0x0006 to 0xFFFF */
  uint16_t max_interval;
  /** uint16_t, Periodic Advertising Properties, this parameter indicates which fields should be included in the advertising packet
   *  Bit Number, 6: Include TxPower in the advertising PDU
   *  All other Values - Reserved For future use */
  uint16_t properties;
} SL_ATTRIBUTE_PACKED rsi_ble_ae_periodic_adv_params_t;

//AE periodic adv enable

/**
 * @brief Structure represents the enable/disable state of periodic advertising.
 */
typedef struct ae_periodic_adv_enable {

  /** uint8_t, enable, If this parameter is set Periodic Advertising starts
   *           0 - Enable Periodic Advertising
   *           1 - Include the ADI field in AUX_SYNC_IND PDUs
*/
  uint8_t enable;
  /** uint8_t, Advertising Handle, Used to identify an advertising set
   *  Range : 0x00 to 0xEF
*/
  uint8_t adv_handle;
} SL_ATTRIBUTE_PACKED rsi_ble_ae_periodic_adv_enable_t;

/**
 * @brief Structure represents the scan parameters for Active Energy (AE) scanning.
 */
typedef struct ae_scan_params_s {
  /** uint8_t, Scan Type, this parameter specifies the type of scan to perform
   *              0x00 - Passive Scanning. No scan request PDUs should be sent.
   *              0x01 - Active Scanning. Scan request PDUs might be sent.
*/
  uint8_t ScanType;
  /** uint16_t, Scan Interval, this parameter is a recommendation from the Host on how frequently the Controller should scan
  *  Range : 0x0004 to 0xFFFF */
  uint16_t ScanInterval;
  /** uint16_t, Scan Window, this parameter is a recommendation from the Host on how long the Controller should scan
 *   Range : 0x0004 to 0xFFFF */
  uint16_t ScanWindow;
} SL_ATTRIBUTE_PACKED ae_scan_params_t;

//AE set sacn params
/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */
/// Indicates the number of supported scanning physical channels.
#define SUPPORTED_SCNNING_PHYS 2
/** @} */
/**
 * @brief Structure to set the scan parameters for BLE Active Scanning.
 */
typedef struct rsi_ble_ae_set_scan_params_s {

  /** unit8_t,The Own Address Type parameter indicates the type of address being used in the scan request packets
  *            Value             Parameter Description
  *             0x00             Public Device Address
  *             0x01             Random Device Address
  *             0x02        Controller generates the Resolvable Private Address based on the local IRK from the resolving list.
  *                         If the resolving list contains no matching entry, then use the public address.
  *             0x03        Controller generates the Resolvable Private Address based on the local IRK from the resolving list.
  *                        If the resolving list contains no matching entry, then use the random address from LE_Set_Random_Address.
  *    All other values           Reserved for future use
  *
 */
  uint8_t own_addr_type;
  /** uint8_t, It is used to determine whether the Filter Accept List is used
   *
   *                 0x00            Basic unfiltered scanning filter policy
   *                 0x01            Basic filtered scanning filter policy
   *                 0x02            Extended unfiltered scanning filter policy
   *                 0x03            Extended filtered scanning filter policy
   *           All other values      Reserved for future use
    */
  uint8_t scanning_filter_policy;
  /** uint8_t, The Scanning_PHYs parameter indicates the PHY(s) on which the advertising packets should be
   received on the primary advertising physical channel.
  *
  *           0             Scan advertisements on the LE 1M PHY
  *           2             Scan advertisements on the LE Coded PHY
  *     All other bits      Reserved for future use
  */
  uint8_t scanning_phys;
  /** ScanParams is an array of variable of structure ae_scan_params_s */
  ae_scan_params_t ScanParams[SUPPORTED_SCNNING_PHYS];
} SL_ATTRIBUTE_PACKED rsi_ble_ae_set_scan_params_t;

//AE set scan enable

/**
 * @brief Structure to configure BLE scanning parameters.
 *
 * This structure is used to set various parameters for enabling or disabling scanning 
 * in the module. It allows the configuration of scan enablement, 
 * duplicate filtering, scan duration, and scan period.
 */
typedef struct rsi_ble_ae_set_scan_enable_s {

  /** uint8_t, Enable, this Parameter determines whether scanning is enabled or disabled
 *
 *             0x00         Scanning disabled
 *             0x01         Scanning enabled
 *       All other values  Reserved for future use
 */
  uint8_t enable;
  /** uint8_t, Filter duplicates, this parameter controls whether the Link Layer should filter out duplicate advertising reports
   *           to the Host or if the Link Layer should generate advertising reports for each packet received
   *
   *         0x00          Duplicate Filtering Disabled
   *         0x01          Duplicate Filtering Enabled
   *         0x02          Duplicate filtering enabled, reset for each scan period
   *    All other Values   Reserved for future use
 */
  uint8_t filter_duplicates;
  /** uint16_t, Duration, The duration of a scan period refers to the time spent scanning on both the primary and secondary advertising physical channels
 *  Range : 0x0001 to 0xFFFF
 */
  uint16_t duration;
  /** uint16_t, Period , Time interval from when the Controller started its last Scan_Duration until it begins the subsequent Scan_Duration
 *  Range : 0x0001 to 0xFFFF
*/
  uint16_t period;
} SL_ATTRIBUTE_PACKED rsi_ble_ae_set_scan_enable_t;

/**
 * 
 * @brief Structure representing the parameters for setting periodic advertising and creating synchronization.
 * This structure is used in BLE operations to configure periodic advertising and create synchronization.
 */
typedef struct rsi_ble_ae_set_periodic_adv_create_sync_s {

  /** uint8_t, Options field, The Options parameter is used to determine whether the Periodic Advertiser List is used
   
     Bit_NUmber    parameter description
	 
   * 0 	           0: Use the Advertising_SID, Advertiser_Address_Type, and Advertiser_Address parameters to determine which advertiser to listen to
		           1: Use the Periodic Advertiser List to determine which advertiser to listen to.
				   
   * 1 			   0: Reporting initially enabled
                   1: Reporting initially disabled
				   
   * 2             0: Duplicate filtering initially disabled
                   1: Duplicate filtering initially enabled
				   
    All other bits Reserved for future use   **/
  uint8_t options;
  /** uint8_t, Advertising SID subfield in the ADI field used to identify the Periodic Advertising.
 *  Range : 0x00 to 0x0F, All other bits - Reserved for future use
*/
  uint8_t adv_sid;
  /** uint8_t, Advertiser Address Type, this parameter indicates the type of address being used in the connection request packets
 *
 *              0x00          Public Device Address or Public Identity Address
 *              0x01          Random Device Address or Random (static) Identity Address
 *       All other values     Reserved for future use
 */
  uint8_t adv_addr_type;
  /** uint8_t, Advertiser Address[6]*/
  uint8_t adv_addr[RSI_DEV_ADDR_LEN];
  /** uint16_t, Skip,The maximum number of periodic advertising events that can be skipped after a successful receive
 *  Range : 0x0000 to 0x01F3 */
  uint16_t skip;
  /** uint16_t, Sync Timeout, Synchronization timeout for the periodic advertising train
 *  Range : 0x000A to 0x4000 */
  uint16_t sync_timeout;
  /** Reserved for future use */
  uint8_t reserved;
} SL_ATTRIBUTE_PACKED rsi_ble_ae_set_periodic_adv_create_sync_t;
/**
 * 
 * @brief Structure to set periodic advertising and terminate synchronization.
 */
typedef struct rsi_ble_ae_set_periodic_adv_terminate_sync_s {

  /** uint16_t, Sync Handle, identifies the periodic Advertising Train
 *  Range : 0x0000 to 0x0EFF*/
  uint16_t sync_handle;
} SL_ATTRIBUTE_PACKED rsi_ble_ae_set_periodic_adv_terminate_sync_t;

//AE set periodic sync(create/terminate or cancel) params
/**
 * @brief Structure representing the BLE Advertising Extension(AE) set periodic synchronization.
 *
 * This structure is used to define the parameters for setting periodic synchronization
 * for Advertising Extension in a BLE device.
 */
typedef struct rsi_ble_ae_set_periodic_sync_s {
  /** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */

  /// Command to create a periodic synchronization for Advertising Extension.
#define BLE_AE_PERIODIC_SYNC_CREATE 0x01
  /// Command to cancel the creation of a periodic synchronization for Advertising Extension.
#define BLE_AE_PERIODIC_SYNC_CREATE_CANCEL 0x02
  /// Command to terminate a periodic synchronization for Advertising Extension.
#define BLE_AE_PERIODIC_SYNC_TERMINATE 0x03
  /** @} */
  uint8_t type; ///< Type of the periodic synchronization command.
  union {
    rsi_ble_ae_set_periodic_adv_create_sync_t create_sync; ///< Parameters for creating periodic synchronization.
    rsi_ble_ae_set_periodic_adv_terminate_sync_t
      terminate_sync;              ///< Parameters for terminating periodic synchronization.
  } SL_ATTRIBUTE_PACKED sync_type; ///< Union for periodic synchronization parameters.
} SL_ATTRIBUTE_PACKED rsi_ble_ae_set_periodic_sync_t;
//#pragma pack(pop)
// AE add/remove/clear dev to/from periodic adv list

/**
 * @brief Structure representing the AE (Advertising extension) device to periodic list.
 */
typedef struct rsi_ble_ae_dev_to_periodic_list_s {

  /** uint8_t, Type
 *          Type Values                    Description
 *              1              Adding Device to Periodic Advertising list
 *              2              Removing Device from Periodic Advertising list
 *              3              Clearing Periodic Advertising List
*/
  uint8_t type;
  /** uint8_t, Advertiser Address Type, this parameter indicates the type of address being used in the connection request packets
 *
 *              0x00          Public Device Address or Public Identity Address
 *              0x01          Random Device Address or Random (static) Identity Address
 *       All other values     Reserved for future use
*/
  uint8_t adv_addr_type;
  /** uint8_t, Advertiser Address[6]*/
  uint8_t adv_addr[RSI_DEV_ADDR_LEN];
  /** uint8_t, Advertising_Sid, Value of the Advertising SID subfield in the ADI field of the PDU, Range : 0x00 to 0x0F*/
  uint8_t adv_sid;
} SL_ATTRIBUTE_PACKED rsi_ble_ae_dev_to_periodic_list_t;

/**
 * @brief Structure representing connection initiation parameters.
 *
 * This structure is used to define various parameters required for initiating a BLE 
 * connection. It includes parameters for scan intervals, scan window, connection 
 * intervals, peripheral latency, supervision timeout, and minimum and maximum length of the connection events.
 */
typedef struct rsi_ble_initiation_params_s {

  /** uint16_t, ScanInterval, It is the Time interval from when the Controller started its last scan until it begins the subsequent scan on the primary
 * advertising physical channel. Range : 0x0004 to 0xFFFF  */
  uint16_t ScanInterval;
  /** uint16_t, Scan Window parameter is a recommendation from the host on how long the controller should scan.
 *  Range : 0x0004 to 0xFFFF */
  uint16_t ScanWindow;
  /** uint16_t, Connection interval minimum parameter defines the minimum allowed connection interval.
 *  Range: 0x0006 to 0x0C80 */
  uint16_t ConnIntervalMin;
  /** uint16_t, Connection interval maximum parameter defines the maximum allowed connection interval.
 *  Range: 0x0006 to 0x0C80*/
  uint16_t ConnIntervalMax;
  /** uint16_t, Peripheral Latency or Maximum Latency parameter defines the maximum allowed Peripheral latency.
 *  Range: 0x0000 to 0x01F3 */
  uint16_t ConnLatency;
  /** uint16_t, Connection Timeout or Supervision Timeout parameter defines the link supervision timeout for the connection.
  *  Range: 0x000A to 0x0C80*/
  uint16_t ConnSTO; //SuperVisionTimeout
  /** uint16_t,The Min CE Length parameter provide the Controller with the expected minimum length of the connection events.
  *  Range: 0x0000 to 0xFFFF */
  uint16_t MinCELen;
  /** uint16_t,The Max CE Length parameter provide the controller with the expected maximum length of the connection events.
  *  Range: 0x0000 to 0xFFFF */
  uint16_t MaxCELen;
} SL_ATTRIBUTE_PACKED rsi_ble_initiation_params_t;

// AE extended create connect

/**
 * @brief Structure representing the extended create connect command for BLE AE.
 */
typedef struct rsi_ble_ae_extended_create_connect_s {

  /** uint8_t, Initiator Filter Policy,It is used to determine whether the Filter Accept List is used
   *        Value                                      Parameter Description
   *        0x00             Filter Accept List is not used to determine which advertiser to connect to Peer_Address_Type and Peer_Address should be used.
   *        0x01             Filter Accept List is used to determine which advertiser to connect to Peer_Address_Type and Peer_Address should be ignored.
   *   All other values      Reserved for future use
  */
  uint8_t initiator_filter_policy;
  /** uint8_t, Own Address Type, this parameter indicates the type of address being used in the connection request packets
   *
   *  Value                                          Parameter Description
   *  0x00                                           Public Device Address
   *  0x01                                           Random Device Address
   *  0x02                         Controller generates the Resolvable Private Address based on the local IRK from the resolving list.
   *                               If the resolving list contains no matching entry, then use the public address.
   *  0x03                         Controller generates the Resolvable Private Address based on the local IRK from the resolving list.
   *                               If the resolving list contains no matching entry, then use the random address from the most recent successful
   *                               HCI_LE_Set_Random_Address command.
   * All other values                               Reserved for future use
  */
  uint8_t own_addr_type;
  /** uint8_t, Remote Address Type or Peer Address Type, this parameter indicates the type of address used in the
  connectable advertisement sent by the peer
  *   Value                                          Parameter Description
  *   0x00                           Public Device Address or Public Identity Address
  *   0x01                         Random Device Address or Random (static) Identity Address
  * All other values                              Reserved for future use
  */
  uint8_t remote_addr_type;
  /** uint8_t, Remote Address or Peer Address, this parameter indicates the Peer’s Public Device Address,
  Random (static) Device Address, Non-Resolvable Private Address, or Resolvable Private Address depending on the Peer_Address_Type parameter */
  uint8_t remote_addr[RSI_DEV_ADDR_LEN];
  /** uint8_t, Initiating PHYs, this parameter indicates the PHY(s) on which the advertising packets should be received on the
   primary advertising physical channel and the PHYs for which connection parameters have been specified
   *         Bit number                         Parameter Description
   *            0                   Scan connectable advertisements on the LE 1M PHY. Connection parameters for the LE 1M PHY are provided.
   *            1                  Connection parameters for the LE 2M PHY are provided.
   *            2                  Scan connectable advertisements on the LE Coded PHY. Connection parameters for the LE Coded PHY are provided.
   *      All other bits                       Reserved for future use
   */
  uint8_t init_phys;
  /** init_params is an array of Variable of Structure rsi_ble_initiation_params_s */
  rsi_ble_initiation_params_t init_params[3];
} SL_ATTRIBUTE_PACKED rsi_ble_ae_extended_create_connect_t;

// LE Read Transmit Power
/**
 * @brief Structure representing the BLE transmit power.
 */
typedef struct rsi_ble_tx_pwr_s {
  /** int8_t, Minimum TX Power, Range: -127 to +20 */
  int8_t min_tx_pwr;
  /** int8_t, Maximum TX Power, Range: -127 to +20 */
  int8_t max_tx_pwr;
} SL_ATTRIBUTE_PACKED rsi_ble_tx_pwr_t;

// Query Rf Path Compensation

/**
 * @brief Structure to hold the information for querying RF path compensation.
 */
typedef struct rsi_ble_query_rf_path_comp_s {

  /** int16_t, RF TX Path Compensation Value, Range: -128.0 dB (0xFB00) to 128.0 dB (0x0500) */
  int16_t tx_path_value;
  /** int16_t, RF RX Path Compensation Value, Range: -128.0 dB (0xFB00) to 128.0 dB (0x0500) */
  int16_t rx_path_value;
} SL_ATTRIBUTE_PACKED rsi_ble_query_rf_path_comp_t;

// write Rf Path Compensation
/**
 * @brief Structure representing the parameters for writing RF path compensation values.
 */
typedef struct rsi_ble_write_rf_path_comp_s {
  /** int16_t, RF TX Path Compensation Value, Range: -128.0 dB (0xFB00) to 128.0 dB (0x0500) */
  int16_t tx_path_value;
  /** int16_t, RF RX Path Compensation Value, Range: -128.0 dB (0xFB00) to 128.0 dB (0x0500)*/
  int16_t rx_path_value;
} SL_ATTRIBUTE_PACKED rsi_ble_write_rf_path_comp_t;

/**
 * @brief Structure representing the BLE Advertising Extensions (AE) Protocol Data Unit (PDU).
 *
 * This structure is used to define the BLE Advertising Extensions PDU.
 * It contains a command sub-opcode and a union of various possible PDU types related to
 * advertising, scanning, and connection parameters.
 *
 */
typedef struct rsi_ble_ae_pdu {
  /** Command sub-opcode */
  uint16_t cmd_sub_opcode;

  /** Union of various possible PDU types */
  union {
    /** Supported number of advertising sets */
    rsi_ble_ae_get_supported_no_of_adv_sets_t ae_supported_no_of_sets;
    /** Supported maximum advertising data */
    rsi_ble_ae_read_supported_max_adv_data_t ae_supported_max_data;
    /** Set random address */
    rsi_ble_ae_set_random_address_t ae_random_address;
    /** Advertising parameters */
    rsi_ble_ae_adv_params_t ae_adv_params;
    /** Advertising or scan response data */
    rsi_ble_ae_data_t ae_adv_or_scn_rsp_data;
    /** Advertising enable */
    rsi_ble_ae_adv_enable_t ae_adv_enable;
    /** Advertising set clear or remove */
    rsi_ble_ae_adv_set_clear_or_remove_t ae_adv_set_clear_or_remove;
    /** Periodic advertising parameters */
    rsi_ble_ae_periodic_adv_params_t ae_periodic_adv_params;
    /** Periodic advertising enable */
    rsi_ble_ae_periodic_adv_enable_t ae_periodic_adv_enable;
    /** Scan parameters */
    rsi_ble_ae_set_scan_params_t ae_scan_params;
    /** Scan enable */
    rsi_ble_ae_set_scan_enable_t ae_scan_enable;
    /** Periodic sync settings */
    rsi_ble_ae_set_periodic_sync_t ae_periodic_sync;
    /** Device to periodic list */
    rsi_ble_ae_dev_to_periodic_list_t dev_to_periodic_list;
    /** Extended create connection */
    rsi_ble_ae_extended_create_connect_t extended_create_conn;
  } SL_ATTRIBUTE_PACKED pdu_type;
} SL_ATTRIBUTE_PACKED rsi_ble_ae_pdu_t;

/** @} */

/**
 * \addtogroup rsi_ble_cb_s_group rsi_ble_cb_s Driver BLE control block
 * @brief Driver BLE control block group
 * 
 * @ingroup BT_BLE_TYPES
 * @{
 */

/**
 * @brief Structure representing the BLE control block.
 *
 * This structure contains various callback functions for handling BLE events,
 * including GAP, SMP, GATT, L2CAP, and AE events.
 */
struct rsi_ble_cb_s {

  /**
     * \addtogroup gap_callbacks_group GAP Callbacks Group
     * @brief Group for GAP callbacks.
     * @ingroup rsi_ble_cb_s_group
     * @{
     */

  /**
   * @brief Advertising report event callback.
   */
  rsi_ble_on_adv_report_event_t ble_on_adv_report_event;

  /**
   * @brief Connection status event callback.
   */
  rsi_ble_on_connect_t ble_on_conn_status_event;

  /**
   * @brief Disconnect event callback.
   */
  rsi_ble_on_disconnect_t ble_on_disconnect_event;

  /**
   * @brief LE ping payload timeout event callback.
   */
  rsi_ble_on_le_ping_payload_timeout_t ble_on_le_ping_time_expired_event;

  /**
   * @brief PHY update complete event callback.
   */
  rsi_ble_on_phy_update_complete_t ble_on_phy_update_complete_event;

  /**
   * @brief Data length update event callback.
   */
  rsi_ble_on_data_length_update_t rsi_ble_on_data_length_update_event;

  /**
   * @brief Enhanced connection status event callback.
   */
  rsi_ble_on_enhance_connect_t ble_on_enhance_conn_status_event;

  /**
   * @brief Directed advertising report event callback.
   */
  rsi_ble_on_directed_adv_report_event_t ble_on_directed_adv_report_event;

  /**
   * @brief Connection update complete event callback.
   */
  rsi_ble_on_conn_update_complete_t ble_on_conn_update_complete_event;

  /**
   * @brief Remote connection parameters request event callback.
   */
  rsi_ble_on_remote_conn_params_request_t ble_on_remote_conn_params_request_event;

  /** @} */ // end of gap_callbacks_group

  /**
     * \addtogroup gap_extended_callbacks_group GAP Extended Callbacks Group 
     * @brief Group for GAP Extended callbacks.
     * @ingroup rsi_ble_cb_s_group
     * @{
     */
  /**
   * @brief Remote device info event callback.
   */
  rsi_ble_on_remote_device_info_t ble_on_remote_device_info_event;

  /**
   * @brief Remote features event callback.
   */
  rsi_ble_on_remote_features_t ble_on_remote_features_event;

  /**
   * @brief LE more data request event callback.
   */
  rsi_ble_on_le_more_data_req_t ble_on_le_more_data_req_event;

  /** @} */ // end of gap_extended_callbacks_group

  /**
     * \addtogroup smp_callbacks_group SMP Callbacks Group
     * @brief Group for SMP callbacks.
     * @ingroup rsi_ble_cb_s_group
     * @{
     */
  /**
   * @brief SMP request event callback.
   */
  rsi_ble_on_smp_request_t ble_on_smp_request_event;

  /**
   * @brief SMP response event callback.
   */
  rsi_ble_on_smp_response_t ble_on_smp_response_event;

  /**
   * @brief SMP passkey event callback.
   */
  rsi_ble_on_smp_passkey_t ble_on_smp_passkey_event;

  /**
   * @brief SMP failed event callback.
   */
  rsi_ble_on_smp_failed_t ble_on_smp_fail_event;

  /**
   * @brief SMP encryption started event callback.
   */
  rsi_ble_on_encrypt_started_t ble_on_smp_encrypt_started;

  /**
   * @brief SMP passkey display event callback.
   */
  rsi_ble_on_smp_passkey_display_t ble_on_smp_passkey_display;

  /**
   * @brief Secure connections passkey event callback.
   */
  rsi_ble_on_sc_passkey_t ble_on_sc_passkey;

  /**
   * @brief LE LTK request event callback.
   */
  rsi_ble_on_le_ltk_req_event_t ble_on_le_ltk_req_event;

  /**
   * @brief LE security keys event callback.
   */
  rsi_ble_on_le_security_keys_t ble_on_le_security_keys_event;

  /**
   * @brief Client SMP response event callback.
   */
  rsi_ble_on_smp_response_t ble_on_cli_smp_response_event;

  /**
   * @brief Secure connections method event callback.
   */
  rsi_ble_on_sc_method_t ble_on_sc_method_event;

  /** @} */ // end of smp_callbacks_group

  /**
     * \addtogroup gatt_callbacks_group GATT Callbacks Group
     * @brief Group for GATT callbacks.
     * @ingroup rsi_ble_cb_s_group
     * @{
     */
  /**
   * @brief Profiles list response callback.
   */
  rsi_ble_on_profiles_list_resp_t ble_on_profiles_list_resp;

  /**
   * @brief Profile response callback.
   */
  rsi_ble_on_profile_resp_t ble_on_profile_resp;

  /**
   * @brief Characteristic services response callback.
   */
  rsi_ble_on_char_services_resp_t ble_on_char_services_resp;

  /**
   * @brief Included services response callback.
   */
  rsi_ble_on_inc_services_resp_t ble_on_inc_services_resp;

  /**
   * @brief Attribute description response callback.
   */
  rsi_ble_on_att_desc_resp_t ble_on_att_desc_resp;

  /**
   * @brief Read response callback.
   */
  rsi_ble_on_read_resp_t ble_on_read_resp;

  /**
   * @brief Write response callback.
   */
  rsi_ble_on_write_resp_t ble_on_write_resp;

  /**
   * @brief GATT write event callback.
   */
  rsi_ble_on_gatt_write_event_t ble_on_gatt_events;

  /**
   * @brief Prepare write event callback.
   */
  rsi_ble_on_gatt_prepare_write_event_t ble_on_prepare_write_event;

  /**
   * @brief Execute write event callback.
   */
  rsi_ble_on_execute_write_event_t ble_on_execute_write_event;

  /**
   * @brief Read request event callback.
   */
  rsi_ble_on_read_req_event_t ble_on_read_req_event;

  /**
   * @brief MTU event callback.
   */
  rsi_ble_on_mtu_event_t ble_on_mtu_event;

  /**
   * @brief GATT error response event callback.
   */
  rsi_ble_on_gatt_error_resp_t ble_on_gatt_error_resp_event;

  /**
   * @brief GATT descriptor value response event callback.
   */
  rsi_ble_on_gatt_desc_val_event_t ble_on_gatt_desc_val_resp_event;

  /**
   * @brief Profiles list event callback.
   */
  rsi_ble_on_event_profiles_list_t ble_on_profiles_list_event;

  /**
   * @brief Profile by UUID event callback.
   */
  rsi_ble_on_event_profile_by_uuid_t ble_on_profile_by_uuid_event;

  /**
   * @brief Read by characteristic services event callback.
   */
  rsi_ble_on_event_read_by_char_services_t ble_on_read_by_char_services_event;

  /**
   * @brief Read by included services event callback.
   */
  rsi_ble_on_event_read_by_inc_services_t ble_on_read_by_inc_services_event;

  /**
   * @brief Read attribute value event callback.
   */
  rsi_ble_on_event_read_att_value_t ble_on_read_att_value_event;

  /**
   * @brief Read response event callback.
   */
  rsi_ble_on_event_read_resp_t ble_on_read_resp_event;

  /**
   * @brief Write response event callback.
   */
  rsi_ble_on_event_write_resp_t ble_on_write_resp_event;

  /**
   * @brief Indicate confirmation event callback.
   */
  rsi_ble_on_event_indicate_confirmation_t ble_on_indicate_confirmation_event;

  /**
   * @brief Prepare write response event callback.
   */
  rsi_ble_on_event_prepare_write_resp_t ble_on_prepare_write_resp_event;

  /** @} */ // end of gatt_callbacks_group

  /**
     * \addtogroup gatt_extended_callbacks_group GATT Extended Callbacks Group
     * @brief Group for GATT extended callbacks.
     * @ingroup rsi_ble_cb_s_group
     * @{
     */

  /**
   * @brief MTU exchange info event callback.
   * @note extended
   */
  rsi_ble_on_mtu_exchange_info_t ble_on_mtu_exchange_info_event;

  /** @} */ // end of gatt_extended_callbacks_group

  /**
     * \addtogroup l2cap_callbacks_group L2CAP Callbacks Group
     * @brief Group for L2CAP callbacks.
     * @ingroup rsi_ble_cb_s_group
     * @{
     */
  /**
   * @brief CBFC connection request event callback.
   */
  rsi_ble_on_cbfc_conn_req_event_t ble_on_cbfc_conn_req_event;

  /**
   * @brief CBFC connection complete event callback.
   */
  rsi_ble_on_cbfc_conn_complete_event_t ble_on_cbfc_conn_complete_event;

  /**
   * @brief CBFC receive data event callback.
   */
  rsi_ble_on_cbfc_rx_data_event_t ble_on_cbfc_rx_data_event;

  /**
   * @brief CBFC disconnect event callback.
   */
  rsi_ble_on_cbfc_disconn_event_t ble_on_cbfc_disconn_event;

  /** @} */ // end of l2cap_callbacks_group

  /**
     * \addtogroup chip_memory_status_callbacks_group Chip Memory Status Callbacks Group
     * @brief Group for Chip memory status callbacks.
     * @ingroup rsi_ble_cb_s_group
     * @{
     */

  /**
   * @brief Chip memory status event callback.
   * @note rsi_ble_on_chip_memory_status_callbacks_register
   */
  chip_ble_buffers_stats_handler_t ble_on_chip_memory_status_event;

  /** @} */ // end of chip_memory_status_callbacks_group

  /**
     * \addtogroup ae_callbacks_group AE Callbacks Group
     * @brief Group for AE callbacks.
     * @ingroup rsi_ble_cb_s_group
     * @{
     */
  /**
   * @brief AE report complete event callback.
   */
  rsi_ble_ae_report_complete_t ble_ae_report_complete_event;

  /**
   * @brief AE periodic advertising sync established event callback.
   */
  rsi_ble_ae_per_adv_sync_estbl_t ble_ae_per_adv_sync_estbl_event;

  /**
   * @brief AE periodic advertising report event callback.
   */
  rsi_ble_ae_per_adv_report_t ble_ae_per_adv_report_event;

  /**
   * @brief AE periodic advertising sync lost event callback.
   */
  rsi_ble_ae_per_adv_sync_lost_t ble_ae_per_adv_sync_lost_event;

  /**
   * @brief AE scan timeout event callback.
   */
  rsi_ble_ae_scan_timeout_t ble_ae_scan_timeout_event;

  /**
   * @brief AE advertising set terminated event callback.
   */
  rsi_ble_ae_adv_set_terminated_t ble_ae_adv_set_terminated_event;

  /**
   * @brief AE scan request received event callback.
   */
  rsi_ble_ae_scan_req_recvd_t ble_ae_scan_req_recvd_event;

  /**
   * @brief RCP response received event callback.
   * 
   */
  rsi_ble_on_rcp_resp_rcvd_t ble_on_rcp_resp_rcvd_event;

  /** @} */ // end of ae_callbacks_group

  rsi_bt_on_controller_logs_t bt_on_controller_logs_event;
};

/** @} */ // end of rsi_ble_cb_s_group

/******************************************************
 * * BLE internal function declarations
 * ******************************************************/
void rsi_ble_callbacks_handler(rsi_bt_cb_t *ble_cb, uint16_t rsp_type, uint8_t *payload, uint16_t payload_length);

#endif
