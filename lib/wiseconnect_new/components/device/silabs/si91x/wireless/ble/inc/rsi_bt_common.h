/*******************************************************************************
 * @file  rsi_bt_common.h
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

#ifndef RSI_BT_COMMON_H
#define RSI_BT_COMMON_H

#include "sl_si91x_types.h"

#include "cmsis_os2.h"

/******************************************************
 * *                      Macros
 * ******************************************************/
/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */

/// Specifies the packet type is Bluetooth HCI command packet.
#define RSI_BT_HCI_CMD_PKT 0x1
/// Represents the HCI ACL packet type.
#define RSI_BT_HCI_ACL_PKT 0x2
/// Specifies the number of packets that can be sent per HCI command.
#define RSI_BT_HCI_PER_CMD_PKT 0x3
/// Indicates dual mode operation in Bluetooth.
#define RSI_BT_DUAL_MODE 0x8
/// This macro is used to enable the Bluetooth Low Energy (BLE) protocol.
#define BLE_PROTO_ENABLE BIT(2)
/// This macro is used to enable the Bluetooth Classic protocol.
#define BT_CLASSIC_PROTO_ENABLE BIT(3)
/// This macro is used to enable the proprietary protocol. It sets the 8th bit of the property.
#define PROP_PROTO_ENABLE (BIT(8))
/// Enable both BLE and Classic Bluetooth protocols.
#define BT_DUAL_MODE_PROTO_ENABLE (BLE_PROTO_ENABLE | BT_CLASSIC_PROTO_ENABLE)
/// Define the bits for Bluetooth Low Energy (BLE) mode.
#define RSI_BT_BLE_MODE_BITS (BIT(2) | BIT(3))
/// Offset of the BT status in the Bluetooth module.
#define RSI_BT_STATUS_OFFSET 0x0C
/// Offset value for the response type in the Bluetooth module.
#define RSI_BT_RSP_TYPE_OFFSET 0x02
///Offset value for the length field in the Bluetooth response.
#define RSI_BT_RSP_LEN_OFFSET 0x00
///Mask used to extract the response length from a Bluetooth response.
#define RSI_BT_RSP_LEN_MASK 0x0FFF
///Protocol identifier for Bluetooth Common.
#define RSI_PROTO_BT_COMMON 0x01
///Bluetooth protocol type for classic Bluetooth.
#define RSI_PROTO_BT_CLASSIC 0x02
///Defines the protocol value for Bluetooth Low Energy (BLE).
#define RSI_PROTO_BLE 0x03
///Definition of the Bluetooth Low Energy (BLE) stack protocol.
#define RSI_PROTO_BT_BLE_STACK 0x05
///The maximum length of the device name.
#define RSI_DEV_NAME_LEN 50
///Length of the device address in bytes.
#define RSI_DEV_ADDR_LEN 6
///Length of the Attribute Protocol (ATT) buffer for the device.
#define RSI_DEV_ATT_LEN 240
///Defines the Bluetooth classic device type.
#define RSI_BT_CLASSIC_DEVICE 0x00
///Defines the Bluetooth Low Energy (LE) device type.
#define RSI_BT_LE_DEVICE 0x01
///Macro definition for an unknown Bluetooth device. The value of this macro is 0xFF.
#define RSI_BT_UNKWON_DEVICE 0xFF
///Defines the stack mode for Bluetooth.
#define STACK_BT_MODE 0x01
///Defines the stack mode for Bluetooth Low Energy (BLE).
#define STACK_BTLE_MODE 0x02
//RF types
/// Specify the external RF mode.
#define RSI_EXTERNAL_RF 0x00
/// Specify the internal RF mode.
#define RSI_INTERNAL_RF 0x01
/**
 * @brief Defines the RF type for the Bluetooth module.
 * 
 * This macro is used to specify the RF type for the Bluetooth module.
 * The value `RSI_INTERNAL_RF` indicates that the module uses an internal RF.
 * 
 * @note This macro should be defined before including the header file.
 */
#define RSI_RF_TYPE RSI_INTERNAL_RF
///Maximum number of attributes supported by the Bluetooth module.
#define RSI_MAX_ATT 5
/**
 * @brief Defines the operating mode for WLAN and Bluetooth dual mode.
 * 
 * This macro is used to specify the operating mode for WLAN and Bluetooth dual mode.
 * The value 9 represents the dual mode operation.
 */
#define RSI_OPERMODE_WLAN_BT_DUAL_MODE 9
/// Bluetooth state indicating none.
#define RSI_BT_STATE_NONE 0
///Defines the state value which indicates that the Bluetooth operation mode is done.
#define RSI_BT_STATE_OPERMODE_DONE 1
///Mask to extract the lower nibble of a byte.
#define LOWERNIBBLE 0x0F
/** @} */
/******************************************************
 * *                    Constants
 * ******************************************************/

/******************************************************
 * *                 Type Definitions
 * ******************************************************/
/** @addtogroup BT_BLE_TYPES
  * @{ */
typedef struct rsi_ble_cb_s
  rsi_ble_cb_t; ///< Typedef for the BLE control block structure refer \ref rsi_ble_cb_s_group  for complete information of Driver BLE control block.
typedef struct rsi_bt_classic_cb_s rsi_bt_classic_cb_t; ///< Typedef for the Bluetooth Classic control block structure.
typedef struct rsi_bt_common_specific_cb_s
  rsi_bt_common_specific_cb_t; ///< Typedef for the Bluetooth common specific control block structure.
typedef int32_t (*rsi_bt_get_ber_pkt_t)(uint8_t *pkt,
                                        uint16_t pkt_len); ///<  Typedef for a function pointer to get BER packet.
/** @} */
/******************************************************
 * *                   Enumerations
 * ******************************************************/

/******************************************************
 * *                   Enumerations
 * ******************************************************/
/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */

/**
 * 
 * @brief Enumeration of Bluetooth common command requests.
 *
 * Defines various command requests for Bluetooth operations, which includes setting
 * and getting local device information, initializing and deinitializing the
 * device, and configuring antenna and power settings.
 */
typedef enum rsi_bt_common_cmd_request_e {
  RSI_BT_SET_LOCAL_NAME                            = 0x0001, ///< Set the local device name.
  RSI_BT_GET_LOCAL_NAME                            = 0x0002, ///< Get the local device name.
  RSI_BT_GET_RSSI                                  = 0x0005, ///< Get the Received Signal Strength Indicator (RSSI).
  RSI_BT_GET_LOCAL_DEV_ADDR                        = 0x0007, ///< Get the local device address.
  RSI_BT_REQ_INIT                                  = 0x008D, ///< Initialize the BT classic protocol.
  RSI_BT_REQ_DEINIT                                = 0x008E, ///< De-Initialize the BT classic protocol.
  RSI_BT_SET_ANTENNA_SELECT                        = 0x008F, ///< Select the antenna.
  RSI_BT_REQ_PER_CMD                               = 0x009A, ///< Request Packet Error Rate (PER) command.
  RSI_BT_SET_FEATURES_BITMAP                       = 0x00A6, ///< Set the features bitmap.
  RSI_BT_VENDOR_SPECIFIC                           = 0x00BE, ///< Vendor-specific command.
  RSI_BT_SET_ANTENNA_TX_POWER_LEVEL                = 0x00A7, ///< Set the antenna transmission power level.
  RSI_BT_SET_GAIN_TABLE_OFFSET_OR_MAX_POWER_UPDATE = 0x012C, ///< Set gain table offset or update maximum power.
  RSI_BT_SET_BD_ADDR_REQ                           = 0x012E, ///< Set the Bluetooth device address.
  RSI_BT_GET_BT_STACK_VERSION                      = 0x012F, ///< Get the Bluetooth stack version.

  RSI_BLE_ONLY_OPER_MODE = 0x8010, ///< Set BLE-only operation mode.
  RSI_BLE_REQ_PWRMODE    = 0x8015, ///< Request BLE power mode.
  RSI_BLE_REQ_SOFTRESET  = 0x801C  ///< Request BLE soft reset.
} rsi_bt_common_cmd_request_t;

/**
 * 
 * @brief Enumeration of Bluetooth common events.
 *
 * Defines various events for Bluetooth operations, which includes the event
 * that indicates the Bluetooth card is ready.
 */
typedef enum rsi_bt_common_event_e {
  RSI_BT_EVENT_CARD_READY = 0x0505, ///< Event indicates that the Bluetooth card is ready.
} rsi_bt_common_event_t;

/**
 * 
 * @brief Enumeration of Bluetooth common command responses.
 *
 * Defines various command responses for Bluetooth operations, which includes responses
 * for setting and querying local device information, initializing and deinitializing
 * the device, and configuring antenna and power settings.
 */
typedef enum rsi_bt_common_cmd_resp_e {
  RSI_BT_RSP_SET_LOCAL_NAME         = 0x0001, ///< Response for setting the local device name.
  RSI_BT_RSP_QUERY_LOCAL_NAME       = 0x0002, ///< Response for querying the local device name.
  RSI_BT_RSP_QUERY_RSSI             = 0x0005, ///< Response for querying the Received Signal Strength Indicator (RSSI).
  RSI_BT_RSP_QUERY_LOCAL_BD_ADDRESS = 0x0007, ///< Response for querying the local device address.
  RSI_BT_RSP_INIT                   = 0x008D, ///< Response for initializing the BT classic protocol.
  RSI_BT_RSP_DEINIT                 = 0x008E, ///< Response for deinitializing the BT classic protocol.
  RSI_BT_RSP_ANTENNA_SELECT         = 0x008F, ///< Response for selecting the antenna.
  RSI_BT_RSP_SET_FEATURES_BITMAP    = 0x00A6, ///< Response for setting the features bitmap.
  RSI_BT_RSP_ANTENNA_TX_POWER_LEVEL = 0x00A7, ///< Response for setting the antenna transmission power level.
  RSI_BT_RSP_SET_GAIN_TABLE_OFFSET_OR_MAX_POWER_UPDATE =
    0x012C,                        ///< Response for setting gain table offset or updating maximum power.
  RSI_BT_RSP_SET_BD_ADDR = 0x012E, ///< Response for setting the Bluetooth device address.

  RSI_BLE_RSP_ONLY_OPER_MODE = 0x8010, ///< Response for setting BLE-only operation mode.
  RSI_BLE_RSP_PWRMODE        = 0x8015, ///< Response for requesting BLE power mode.
  RSI_BLE_RSP_SOFTRESET      = 0x801C  ///< Response for requesting BLE soft reset.
} rsi_bt_common_cmd_resp_t;
/** @} */
/******************************************************
 * *                    Structures
 * ******************************************************/
/** @addtogroup BT_BLE_TYPES
  * @{ */
/// Driver control block
struct rsi_driver_cb_s;
// Driver BT Common control block
/**
 * @brief Structure represents the Bluetooth common specific callback.
 *
 * This structure is used to define the parameters for Bluetooth common specific callbacks,
 * which includes the PER Bit Error Rate (BER) callback.
 */
struct rsi_bt_common_specific_cb_s {
  /** PER BER Callbacks */
  rsi_bt_get_ber_pkt_t rsi_bt_get_ber_pkt;
};
// Specific BT, BLE blocks
/**
 * @brief Structure representing the Bluetooth global callback.
 *
 * This structure is used to define the parameters for Bluetooth global callbacks,
 * which includes common specific callbacks, BLE specific callbacks, and BT Classic specific callbacks.
 */
typedef struct rsi_bt_global_cb_s {
  /** BT Common specific callback */
  rsi_bt_common_specific_cb_t *bt_common_specific_cb;
  /** BLE specific callback */
  rsi_ble_cb_t *ble_specific_cb;
  /** BT Classic specific callback */
  rsi_bt_classic_cb_t *bt_specific_cb;
} rsi_bt_global_cb_t;

// Remote LE Device info structure

/**
 * @brief Structure represents the remote BLE device information.
 *
 * This structure is used to define the parameters for storing information about a remote BLE device,
 * which includes its address, buffer counts, and synchronization mechanisms.
 */
typedef struct rsi_remote_ble_info_s {
  /** BD Address of the remote LE device */
  uint8_t remote_dev_bd_addr[RSI_DEV_ADDR_LEN];
  /** Address type of the remote LE device */
  uint8_t remote_dev_addr_type;
  /** Available Buffer Count */
  uint8_t avail_buf_cnt;
  /** Max Buffer Count */
  uint8_t max_buf_cnt;
  /** Max Buffer Length */
  uint16_t max_buf_len;
  /** Flag for dev info used or not */
  uint8_t used;
  /** Flag for checking command in use */
  uint8_t cmd_in_use;
  /** Flag for checking expected remote response for each procedure */
  uint16_t expected_resp;

#define SMALL_BUFF_MODE 0
#define BIG_BUFF_MODE   1
  /** Buffer config mode */
  uint8_t mode;
  /** Mutex handle for avail_buf_info update */
  osMutexId_t ble_buff_mutex;
} rsi_remote_ble_info_t;
// Driver BT/BLE/PROP_PROTOCOL control block
/**
 * @brief Structure represents the Bluetooth control block.
 *
 * This structure is used to define the parameters for the Bluetooth control block,
 * which includes status, state, synchronization mechanisms, and remote BLE device information.
 */
typedef struct rsi_bt_cb_s {
  /** Driver BT control block status */
  volatile int32_t status;
  /** Driver BT control block state */
  uint16_t state;
  /** Driver BT control block mutex */
  osMutexId_t bt_mutex;
  /** Driver BT control block expected command response */
  void *expected_response_buffer;
  /** Expected command response type */
  uint16_t expected_response_type;
  /** Sync command flag to identify that the command is blocking / sync type */
  uint8_t sync_rsp;
  /** BT device type at disconnect */
  uint8_t dev_type;
  /** Driver BT control block semaphore for commands */
  osSemaphoreId_t bt_cmd_sem;
  /** Driver BT control block semaphore */
  osSemaphoreId_t bt_sem;
  /** Buffer pointer given by application to driver */
  uint8_t *app_buffer;
  /** Buffer length given by application to driver */
  uint32_t app_buffer_length;
  /** Pointer to global Bluetooth callback structure */
  rsi_bt_global_cb_t *bt_global_cb;
  /** Address of the device to which directed advertising has to be done in ll privacy mode */
  uint8_t directed_addr[RSI_DEV_ADDR_LEN];
/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */

/** Maximum number of remote BLE devices */
#define MAX_REMOTE_BLE_DEVICES 10
  /** @} */
  /** Structure holds remote LE device information (BD address and controller buffer availability) */
  rsi_remote_ble_info_t remote_ble_info[MAX_REMOTE_BLE_DEVICES];
  /** Variable indicates buffer full/empty status --> 0 -> Empty, 1 -> Full */
  uint8_t buf_status;
  /** Variable indicates command in use status --> 0 -> Not In Use, 1 -> In Use */
  uint8_t cmd_status;
  /** Variable to save remote information index */
  uint8_t remote_ble_index;
  /** Driver BT control block asynchronous status */
  volatile int32_t async_status;
  /**to track total number of available data buffers not based on individual connections*/
  uint8_t ble_buff_total_avail_cnt;
  /** Mutex handle for avail_buf_info update */
  osMutexId_t ble_buff_total_mutex;
} rsi_bt_cb_t;

// Set local name command structure

/**
 * @brief Structure represents the Bluetooth request to set the local device name.
 *
 * This structure is used to define the parameters for setting the local Bluetooth device name,
 * which includes the name length, and the name itself.
 */
typedef struct rsi_bt_req_set_local_name_s {
  /** Length of the required name to be set */
  uint8_t name_len;
  /** Required name */
  int8_t name[RSI_DEV_NAME_LEN];
} rsi_bt_req_set_local_name_t;

// Get RSSI command structure

/**
 * @brief Structure represents the Bluetooth request to get the Received Signal Strength Indicator (RSSI).
 *
 * This structure is used to define the parameters for requesting the RSSI of a remote Bluetooth device,
 * that includes the device address.
 */
typedef struct rsi_bt_get_rssi_s {
  /** Device address (6 bytes) */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_get_rssi_t;

// Response structures

// Get local name response structures

/**
 * @brief Structure represents the Bluetooth response to get the local device name.
 *
 * This structure is used to define the parameters for the response that includes the local Bluetooth device name,
 * that includes the name length, the name itself, and a reserved variable.
 */
typedef struct rsi_bt_resp_get_local_name_s {
  /** Name length */
  uint8_t name_len;
  /** Array which consists name of the local device. The maximum size of this array is 50 */
  int8_t name[RSI_DEV_NAME_LEN];
  /** Reserved variable */
  uint8_t _reserved;
} rsi_bt_resp_get_local_name_t;
/** @} */

/** @addtogroup BT-BLE Common
  * @{ */
// Get Stack Version
/**
 * @brief Structure represents the Bluetooth response to get the BT stack version.
 *
 * This structure is used to define the parameters for the response that includes the Bluetooth stack version.
 */
typedef struct rsi_bt_resp_get_bt_stack_version_s {
  /** Bluetooth stack version */
  int8_t stack_version[10];
} rsi_bt_resp_get_bt_stack_version_t;

// Set antenna structure
/**
 * @brief Structure represents the BLE set antenna request.
 *
 * This structure is used to define the parameters for setting the BLE antenna,
 * that includes the antenna value which, can be internal or external.
 */
typedef struct rsi_ble_set_antenna_s {
  /** Antenna value - internal/external */
  uint8_t value;
} rsi_ble_set_antenna_t;

/**
 * @brief Structure represents the Bluetooth set feature bitmap.
 *
 * This structure is used to define the parameters for setting the Bluetooth feature bitmap,
 * which includes feature bits in a bit map format.
 */
typedef struct rsi_bt_set_feature_bitmap_s {
  /** Features bits (bit map) */
  uint32_t bit_map;
} rsi_bt_set_feature_bitmap_t;

/**
 * @brief Structure represents the BLE operation mode.
 *
 * This structure is used to define the parameters for the BLE operation mode,
 * that includes Bluetooth features, and the module type.
 */
typedef struct rsi_ble_oper_mode_s {
  /** Bluetooth features */
  uint32_t bt_features;
  /** Module type */
  uint8_t module_type;
} rsi_ble_oper_mode_t;

/**
 * @brief Structure represents the BLE power mode.
 *
 * This structure is used to define the parameters for setting the BLE power mode,
 * that includes the power mode and Ultra-Low-Power (ULP) mode enable settings.
 */
typedef struct rsi_ble_power_mode_s {
  /** Power mode to set */
  uint8_t power_mode;
  /** Set LP/ULP/ULP-without RAM retention */
  uint8_t ulp_mode_enable;
} rsi_ble_power_mode_t;

// Set antenna tx power level structure

/**
 * @brief Structure represents the Bluetooth set antenna transmit power level.
 *
 * This structure is used to define the parameters for setting the antenna transmit power level,
 * which includes the protocol mode and transmit power.
 */
typedef struct rsi_bt_set_antenna_tx_power_level_s {
  /** Protocol mode: 1-BT, 2-LE */
  uint8_t protocol_mode;
  /** Transmit power */
  int8_t tx_power;
} rsi_bt_set_antenna_tx_power_level_t;

// BT PER Stats
/**
 * @brief Structure represents the Bluetooth Packet Error Rate (PER) statistics.
 *
 * This structure is used to define the parameters for collecting Bluetooth PER statistics,
 * that includes counts of CRC fails, successful transmissions, and other relevant metrics.
 */
typedef struct rsi_bt_per_stats_s {
  /** Packet count of CRC fails (Cyclic Redundancy Check (CRC)) */
  uint16_t crc_fail_cnt;
  /** Packet count of CRC passes */
  uint16_t crc_pass_cnt;
  /** Packet count of aborted transmissions */
  uint16_t tx_abort_cnt;
  /** Packet count of dropped receptions */
  uint16_t rx_drop_cnt;
  /** Packet count of CCA Idle (Clear Channel Assessment (CCA)) */
  uint16_t rx_cca_idle_cnt;
  /** Packet count of Rx start */
  uint16_t rx_start_idle_cnt;
  /** Packet count of aborted receptions */
  uint16_t rx_abrt_cnt;
  /** Packet count of successful transmissions */
  uint16_t tx_dones;
  /** Received Signal Strength Indicator of the packet */
  int8_t rssi;
  /** Packet count of ID packets received */
  uint16_t id_pkts_rcvd;
  /** Dummy array of length 5 */
  uint16_t dummy[5];
} rsi_bt_per_stats_t;

/**
 * @brief Structure represents the Bluetooth set local Bluetooth Device (BD) address.
 *
 * This structure is used to define the parameters for setting the local Bluetooth device address.
 */
typedef struct rsi_bt_set_local_bd_addr_s {
  /** Device address (6 bytes) */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_set_local_bd_addr_t;

/**
 * @brief Structure for updating gain table offset or max power in Bluetooth commands.
 */
typedef struct rsi_bt_cmd_update_gain_table_offset_or_maxpower_s {
  /** Node id 
   *  * 0 - BLE
  */
  uint8_t node_id;
  /** Request type to select appropriate gain table
   *  * 0 - Update gain table with max power values
   *  * 1 - Update gain table with offset values
  */
  uint8_t update_gain_table_type;
  /** 
  * * Gain table payload length 
  */
  uint8_t payload_len;
  /** 
  *   * The Gain table payload data consists of channel Tx power values for different regions in the format mentioned below. 
  *   * The Gain table holding maximum Tx power values: (Each entry in the table is 1 byte.)
  *       @code
  *        <TABLE NAME>[] = {
  *            <REGION NAME 1>, <MAX POWER>,
  *            <REGION NAME 2>, <MAX POWER>,
  *            .
  *            .
  *            <REGION NAME N>, <MAX POWER>
  *        };
  *       @endcode
  * 
  *   * The Gain table with offset values: (Each entry in the table is 1 byte.)
  *      @code
  *       <TABLE NAME>[] = {
  *         <Number Of Regions - 'r'>,
  *           <REGION NAME 1>, <Number Of Channels - 'n'>,
  *             <cha_num_1>,	<1Mbps_offset>,	<2Mbps_offset>, <125kbps_offset>, <500kbps_offset>
  *             <cha_num_2>,	<1Mbps_offset>,	<2Mbps_offset>, <125kbps_offset>, <500kbps_offset>
  *             .
  *             .
  *             <cha_num_n>,	<1Mbps_offset>,	<2Mbps_offset>, <125kbps_offset>, <500kbps_offset>
  *           <REGION NAME 2>, <Number Of Channels - 'n'>,
  *             <cha_num_1>,	<1Mbps_offset>,	<2Mbps_offset>, <125kbps_offset>, <500kbps_offset>
  *             <cha_num_2>,	<1Mbps_offset>,	<2Mbps_offset>, <125kbps_offset>, <500kbps_offset>
  *             .
  *             .
  *             <cha_num_n>,	<1Mbps_offset>,	<2Mbps_offset>, <125kbps_offset>, <500kbps_offset>
  *           .
  *           .
  *           <REGION NAME r>, <Number Of Channels - 'n'>,
  *             <cha_num_1>,	<1Mbps_offset>,	<2Mbps_offset>, <125kbps_offset>, <500kbps_offset>
  *             <cha_num_2>,	<1Mbps_offset>,	<2Mbps_offset>, <125kbps_offset>, <500kbps_offset>
  *             .
  *             .
  *             <cha_num_n>,	<1Mbps_offset>,	<2Mbps_offset>, <125kbps_offset>, <500kbps_offset>
  *       };
  *      @endcode
  * 
  *   * Supported Region names: FCC, ETSI, TELEC, WORLDWIDE, and KCC.
  *   * The following table lists the regions and their corresponding values to pass:
  * 
  *      | Value | Region     |
  *      |-------|------------|
  *      | 0     | FCC        |
  *      | 1     | ETSI       |
  *      | 2     | TELEC      |
  *      | 3     | WORLDWIDE  |
  *      | 4     | KCC        |
  * 
  *   * If Tx power varies by channel, specify the number of applicable channels and provide the Tx power value for each channel. For channels that do not require a change, use channel 255 with a Tx power offset of 0.
  * 
  * @note When using user gain tables, define gain tables for both the high-power (HP) chain and the low-power (LP) chain at **0 dBm** and **8 dBm**. Use the **ble_per** application as a reference implementation.
  */
  uint8_t payload[128];
} rsi_bt_cmd_update_gain_table_offset_or_maxpower_t;
/** @} */
/******************************************************
 * * BT/BLE common function declarations
 * ******************************************************/
void rsi_bt_set_status(rsi_bt_cb_t *bt_cb, int32_t status);
void rsi_bt_common_tx_done(const sl_wifi_system_packet_t *pkt, int32_t status);
int8_t rsi_bt_cb_init(rsi_bt_cb_t *bt_cb, uint16_t protocol_type);
int32_t rsi_bt_driver_send_cmd(uint16_t cmd, void *cmd_struct, void *resp);
uint16_t rsi_bt_global_cb_init(struct rsi_driver_cb_s *driver_cb, uint8_t *buffer);
uint16_t rsi_driver_process_bt_resp_handler(void *rx_pkt);
uint16_t rsi_bt_get_proto_type(uint16_t rsp_type, rsi_bt_cb_t **bt_cb);

/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */
#define HCI_BT_PER_STATS_CMD_ID 0x08 ///< HCI command ID for Bluetooth Packet Error Rate (PER) statistics.
#define HCI_BLE_TRANSMIT_CMD_ID 0x13 ///< HCI command ID for BLE transmit command.
#define HCI_BLE_RECEIVE_CMD_ID  0x14 ///< HCI command ID for BLE receive command.
#define HCI_BT_TRANSMIT_CMD_ID  0x15 ///< HCI command ID for Bluetooth transmit command.
#define HCI_BT_RECEIVE_CMD_ID   0x16 ///< HCI command ID for Bluetooth receive command.
/** @} */
#endif
