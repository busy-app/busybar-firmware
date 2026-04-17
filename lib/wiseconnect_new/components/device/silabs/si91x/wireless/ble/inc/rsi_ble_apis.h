/*******************************************************************************
 * @file  rsi_ble_apis.h
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

#ifndef RSI_BLE_APIS_H
#define RSI_BLE_APIS_H

#include "rsi_bt_common.h"

/******************************************************
 * *                      Macros
 * ******************************************************/

/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */
/// Maximum number of response list for Bluetooth Low Energy (BLE).
#define RSI_BLE_MAX_RESP_LIST 0x05
/// Maximum size of an advertising report.
#define RSI_MAX_ADV_REPORT_SIZE 31
/// Size of the passkey.
#define BLE_PASSKEY_SIZE 6

/// Defines the output power front end loss.
#ifndef BLE_OUTPUT_POWER_FRONT_END_LOSS
#define BLE_OUTPUT_POWER_FRONT_END_LOSS 0 /* db */
#endif

/// Host descriptor length
#define RSI_HOST_DESC_LENGTH 16

/** @} */

/******************************************************
 * *                    Constants
 * ******************************************************/
/******************************************************
 * *                   Enumerations
 * ******************************************************/

/******************************************************
 * *                 Type Definitions
 * ******************************************************/

/******************************************************
 * *                    Structures
******************************************************/

/** @addtogroup BT_BLE_EVENT_TYPES Event Types
  * @{ */

// GAP Event structures

//Advertise report event structure
/**
 * @brief Structure representing an advertising report event.
 */
typedef struct rsi_ble_event_adv_report_s {
  /**Address type of the advertising device */
  uint8_t dev_addr_type;
  /**Address of the advertising device*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**Raw advertisement data length*/
  uint8_t adv_data_len;
  /**advertisement data*/
  uint8_t adv_data[RSI_MAX_ADV_REPORT_SIZE];
  /**Signal strength*/
  int8_t rssi;
  /** Report type 
-
       0x00	Connectable and scannable undirected advertising (ADV_IND) 
-
       0x01	Connectable directed advertising (ADV_DIRECT_IND) 
-
       0x02	Scannable undirected advertising (ADV_SCAN_IND)  
-
       0x03	Non connectable undirected advertising (ADV_NONCONN_IND) 
-
       0x04	Scan Response (SCAN_RSP) 
-
       All other values	Reserved for future use*/
  uint8_t report_type;
} rsi_ble_event_adv_report_t;

//Connection status event structure
/**
 * @brief Structure representing the connection status event in the module.
 */
typedef struct rsi_ble_event_conn_status_s {
  /** Address type of the remote device */
  uint8_t dev_addr_type;
  /** Address of the remote device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Status of the connection - success/failure */
  uint16_t status;
} rsi_ble_event_conn_status_t;

//enhance connection status event structure
/**
 * @brief Structure representing the enhanced connection status event in the module.
 */
typedef struct rsi_ble_event_enhnace_conn_status_s {

  /**Device address type of the Connected Remote Device*/
  uint8_t dev_addr_type;
  /**Device address of the remote device.*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**Local Device resolvable address*/
  uint8_t local_resolvlable_addr[RSI_DEV_ADDR_LEN];
  /**Remote Device resolvable address*/
  uint8_t peer_resolvlable_addr[RSI_DEV_ADDR_LEN];
  /**The role of the device - central/ peripheral*/
  uint8_t role;
  /**Connection interval used on this connection. Range: 0x0006 to 0x0C80*/
  uint16_t conn_interval;
  /**Peripheral latency for the connection in number of connection events. Range: 0x0000 to 0x01F3*/
  uint16_t conn_latency;
  /**Connection supervision timeout. Range: 0x000A to 0x0C80*/
  uint16_t supervision_timeout;
  /**Only applicable for peripheral, for central this value is set to 0x00*/
  uint8_t master_clock_accuracy;
  /**Status of the Connection - success/failure*/
  uint16_t status;
} rsi_ble_event_enhance_conn_status_t;

//Disconnect event structure
/**
 * @brief Disconnection event structure.
 *
 * This structure contains information about the disconnection event.
 */
typedef struct rsi_ble_event_disconnect_s {
  /** Device address of the remote device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** The type of the remote device (LE) */
  uint8_t dev_type;
} rsi_ble_event_disconnect_t;

//le ping timeout expired event structure
/**
 * @brief The structure represents the LE ping timeout expired event.
 */
typedef struct rsi_ble_event_le_ping_time_expired_s {
  /**Device address of the remote device*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];

} rsi_ble_event_le_ping_time_expired_t;

//le ltk request event Structure
/**
 * @brief The structure represents the Long Term Key (LTK) request event.
 */
typedef struct rsi_bt_event_le_ltk_request_s {
  /**BD Address of the remote device*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**ediv of local device*/
  uint16_t localediv;
  /**rand of local device*/
  uint8_t localrand[8];
  /**Address type of remote device*/
  uint8_t dev_addr_type;
} rsi_bt_event_le_ltk_request_t;

//le security keys event Structure
/**
 * @brief The structure represents security keys event.
 */
typedef struct rsi_bt_event_le_security_keys_s {
  /**BD Address of the remote device*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**16 byte irk of the local device*/
  uint8_t local_irk[16];
  /**16 byte irk of the remote device*/
  uint8_t remote_irk[16];
  /**remote device ediv value*/
  uint16_t remote_ediv;
  /**remote device rand value*/
  uint8_t remote_rand[16];
  /**remote device ltk value*/
  uint8_t remote_ltk[16];
  /**Identity address type - public/random 
-
      0x00 --> Public Identity Address 
-
      0x01 --> Random (static) Identity Address 
-
      All other values Reserved for future use*/
  uint8_t Identity_addr_type;
  /**Identity address which is resolved after security keys exchange*/
  uint8_t Identity_addr[6];
  /**Device address type*/
  uint8_t dev_addr_type;
} rsi_bt_event_le_security_keys_t;

//encryption enabled structure
/**
 * @brief Structure to hold the event data for encryption enabled event.
 */
typedef struct rsi_bt_event_encryption_enabled_s {
  /**Remote device Address*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**Status of the Encryption 
-
      ENCRYPT_ENABLED BIT(0) --> To indicate or set encryption is enabled AUTH_LTK_OR_STK_ENC 
-
	  BIT(1) --> To indicate or set Authenticated Pairing and Encryption UN_AUTH_LTK_OR_STK_ENC 
-
	  BIT(2) --> To indicate or set UnAuthenticated Pairing and Encryption AUTH_LTK_WITH_LE_SC_ENC 
-
	  BIT(3) --> To indicate or set Authenticated Pairing and Encryption with LE SC */
  uint8_t enabled;
  /**BLE Secure Connections Enable/Disable indication 
-
      0 --> Disable 
-
      1 --> Enable */
  uint8_t sc_enable;
  /**Local device EDIV*/
  uint16_t localediv;
  /**Local RAND*/
  uint8_t localrand[8];
  /**Local Long term Key*/
  uint8_t localltk[16];
  /**Remote Device Address type*/
  uint8_t dev_addr_type;
} rsi_bt_event_encryption_enabled_t;

//SMP protocol structure
//SMP Request event structure
/**
 * @brief Structure to hold the Security Manager Protocol (SMP) request event.
 */
typedef struct rsi_bt_event_smp_req_s {
  /**address of remote device*/
  uint8_t dev_addr[6];
  /**auth req of remote device*/
  uint8_t auth_req;
} rsi_bt_event_smp_req_t;

//SMP Response event structure
/**
 * @brief Structure to hold the SMP response event data.
 */
typedef struct rsi_bt_event_smp_resp_s {
  /**address of remote device*/
  uint8_t dev_addr[6];
  /**Device input output capability 
-
      0x00 - Display Only 
-
      0x01 - Display Yes/No 
-
      0x02 - Keyboard Only  
-
      0x03 - No Input No Output
      0x04 - Keyboard Display*/
  uint8_t io_cap;
  /**Out Of the Band data*/
  uint8_t oob_data;
  /**Authentication Request contains bonding type 
-
      MITM Request - BIT(2) 
-
      Secure Connections - BIT(3) 
-
      Keypress - BIT(4) 
-
      CT2 - BIT(5)*/
  uint8_t auth_req;
  /**Minimum required key size*/
  uint8_t min_req_key_size;
  /** Initiator generates/requires the no .of keys after successful paring 
-
      BIT(0) - EncKey: Initiator distributes the Long-Term Key (LTK) followed by Encrypted Diversifier (EDIV) and Random Number (Rand) 
-
      BIT(1) - IdKey : Initiator distributes the Identity Resolving Key (IRK) followed by its address 
-
      BIT(2) - Sign  : Initiator distributes the Connection Signature Resolving Key (CSRK) 
-
      BIT(3) - BIT(7): Reserved for future use */
  uint8_t ini_key_distrb;
  /** Responder generates/requires the no .of keys after successful paring 
-
      BIT(0) - EncKey: Responder distributes the LTK followed by EDIV and Rand 
-
      BIT(1) - IdKey : Responder distributes the IRK followed by its address 
-
      BIT(2) - Sign  : Responder distributes the CSRK 
-
      BIT(3) - BIT(7): Reserved for future use */
  uint8_t resp_key_distrb;
} rsi_bt_event_smp_resp_t;

//SMP passkey event structure
/**
 * @brief Structure to hold the SMP passkey event data.
 */
typedef struct rsi_bt_event_smp_passkey_s {
  /**address of remote device*/
  uint8_t dev_addr[6];
} rsi_bt_event_smp_passkey_t;

//SMP passkey display event structure
/**
 * @brief Structure to hold the SMP passkey display event.
 */
typedef struct rsi_bt_event_smp_passkey_display_s {
  /**address of remote device*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**This is the key required in pairing process( 6 bytes)*/
  uint8_t passkey[BLE_PASSKEY_SIZE];
} rsi_bt_event_smp_passkey_display_t;

//SC passkey display event structure
/**
 * @brief Structure to hold the Secure Connections (SC) passkey event.
 */
typedef struct rsi_bt_event_sc_passkey_s {
  /**address of remote device*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**Reserved for future use*/
  uint8_t reserved[2];
  /**This is the key required in pairing process*/
  uint32_t passkey;
} rsi_bt_event_sc_passkey_t;

//SMP failed event structure
/**
 * @brief Structure to hold the information related to Security Manager Protocol (SMP) failed event.
 */
typedef struct rsi_bt_event_smp_failed_s {
  /**device address of the remote device*/
  uint8_t dev_addr[6];
} rsi_bt_event_smp_failed_t;

//Security Methods event structure
/**
 * @brief Structure to define the Bluetooth event security method.
 */
typedef struct rsi_bt_event_sc_method_s {
  /**Security Method --> Justworks or Passkey 
-
     RSI_BT_LE_SC_JUST_WORKS		0x01 
-
     RSI_BT_LE_SC_PASSKEY   		0x02 */
  uint8_t sc_method;
} rsi_bt_event_sc_method_t;

/**
 * @brief Structure representing the event for Cross Transport Key Derivation (CTKD).
 *
 * This structure is used to define the parameters for the events related to CTKD,
 * that includes the remote device address, and the derived key.
 */
typedef struct rsi_bt_event_ctkd_s {
  /** Address of the remote device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Derived key */
  uint8_t key[16];
} rsi_ble_event_ctkd_t;

// phy update complete event
/**
 * @brief The structure holds PHY update event information.
 */
typedef struct rsi_ble_event_phy_update_s {

  /**Device address of the remote device.*/
  uint8_t dev_addr[6];
  /**Transmission PHY rate(1 byte) 
-
     BIT(0) - The Host prefers to use the LE 1M transmitter PHY (possible among others) 
-
     BIT(1) - The Host prefers to use the LE 2M transmitter PHY (possible among others) 
-
     BIT(2) - The Host prefers to use the LE Coded transmitter PHY (possible among others) 
-
     BIT(3) - BIT(7) Reserved for future use */
  uint8_t TxPhy;
  /**Reception PHY rate(1 byte) 
-
     BIT(0) - The Host prefers to use the LE 1M reception PHY (possible among others) 
-
     BIT(1) - The Host prefers to use the LE 2M reception PHY (possible among others) 
-
     BIT(2) - The Host prefers to use the LE Coded reception PHY (possible among others) 
-
     BIT(3) - BIT(7) Reserved for future use */
  uint8_t RxPhy;
} rsi_ble_event_phy_update_t;

// connection parameters complete event

/**
 * @brief Structure holds the data related to connection update event.
 * 
 */
typedef struct rsi_ble_event_conn_update_s {
  /**Device address of the remote device*/
  uint8_t dev_addr[6];
  /**Connection Interval*/
  uint16_t conn_interval;
  /**Slave Latency*/
  uint16_t conn_latency;
  /**Supervision Timeout*/
  uint16_t timeout;
} rsi_ble_event_conn_update_t;

// remote connection params request event  //event_id : 0x152E

/**
 * @brief The structure represents the parameters of a remote connection parameter request event.
 */
typedef struct rsi_ble_event_remote_conn_param_req_s {
  /** Device address of the remote device */
  uint8_t dev_addr[6];
  /** Minimum connection interval */
  uint16_t conn_interval_min;
  /** Maximum connection interval */
  uint16_t conn_interval_max;
  /** Slave Latency */
  uint16_t conn_latency;
  /** Supervision Timeout */
  uint16_t timeout;
} rsi_ble_event_remote_conn_param_req_t;

/**
 * @brief Structure holds the data of the remote features event.
 */
typedef struct rsi_ble_event_remote_features_s {
  /**Remote device address*/
  uint8_t dev_addr[6];
  /**Remote device supported features 
-
     @note Refer to spec for the supported features list. */
  uint8_t remote_features[8];
} rsi_ble_event_remote_features_t;

/**
 * @brief Structure holds the LE device buffer indication event.
 */
typedef struct rsi_ble_event_le_dev_buf_ind_s {

  /**Remote device address*/
  uint8_t remote_dev_bd_addr[RSI_DEV_ADDR_LEN];
  /**No. of Available buffer*/
  uint8_t avail_buf_cnt;
  /** total number of available data buffers*/
  uint8_t avail_buff_total_cnt;
} rsi_ble_event_le_dev_buf_ind_t;

/**
 * @brief Structure representing the data for the length update event.
 * 
 */
typedef struct rsi_ble_event_data_length_update_s {

  /**Device address of the remote device.*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**Maximum TX Octets to be transmitted*/
  uint16_t MaxTxOctets;
  /**Maximum TX time to transmit the MaxTxOctets*/
  uint16_t MaxTxTime;
  /**Maximum Rx Octets to be received*/
  uint16_t MaxRxOctets;
  /**Maximum Rx time to receive the MaxRxOctets*/
  uint16_t MaxRxTime;
} rsi_ble_event_data_length_update_t;

/**
 * @brief Structure representing a 128-bit Universal Unique IDentifier (UUID).
 */
typedef struct uuid128_s {
  /** Holds the value of data1 */
  uint32_t data1;
  /** Holds the value of data2 */
  uint16_t data2;
  /**
   * @brief Holds the value of data3.
   */
  uint16_t data3;
  /**
   * @brief Array to store 8 bytes of data.
   */
  uint8_t data4[8];
} uuid128_t;

/// 16 bit UUID format structure
typedef uint16_t uuid16_t;

/// 32 bit UUID format structure
typedef uint32_t uuid32_t;

// UUID format structure

/**
 * @brief Structure representing a Universally Unique Identifier (UUID).
 *
 * This structure encapsulates a UUID, which can be of varying sizes— 16, 32, or 128-bits.
 * It includes the size of the UUID, a reserved field, and a union to hold the value 
 * of one of the three types of UUIDs.
 */
typedef struct uuid_s {
  /** 
   * Size of the UUID.
   * Indicates the size of the UUID in bits. It can be 16, 32, or 128, depending on the type of UUID used.
   */
  uint8_t size;

  /** 
   * Reserved field.
   * This field is reserved for future use.
   */
  uint8_t reserved[3];

  /** 
   * @brief Union representing the value of the UUID.
   * This union stores the value of the UUID, which can be of three different sizes:
   * 128-bit, 32-bit, or 16-bit. The appropriate field in the union should be used 
   * depending on the size of the UUID.
   */
  union uuid_t {
    /** 
  * @brief 128-bit UUID value. \ref uuid128_t
  *
  * This member holds the value of a 128-bit UUID, which is divided into four segments:
  * - The first 32 bits (`data1`)
  * - The next 16 bits (`data2`)
  * - The following 16 bits (`data3`)
  * - The final 64 bits (`data4`), stored as an array of 8 bytes
  *
  * These segments together represent a full 128-bit UUID, compliant with the standard UUID format.
  */
    uuid128_t val128;

    /** 
     * 32-bit UUID value.
     * This field holds the value of a 32-bit UUID, stored as an array of 4 bytes.
     */
    uuid32_t val32;

    /** 
     * 16-bit UUID value.
     * This field holds the value of a 16-bit UUID, stored as an array of 2 bytes.
     */
    uuid16_t val16;
  } val; ///< Value of one of the 3 types (128-bit, 32-bit, or 16-bit) of UUIDs
} uuid_t;

/** @} */
/** @addtogroup BT_BLE_TYPES
  * @{ */
// profile descriptor/primary service structure
/**
 * @brief Represents the data of the particular profile descriptor.
 */
typedef struct profile_descriptor_s {
  /** Start handle. */
  uint8_t start_handle[2];
  /** End handle. */
  uint8_t end_handle[2];
  /** profile uuid. */
  uuid_t profile_uuid;
} profile_descriptors_t;
/** @} */

// GATT attribute descriptor structure
/** @addtogroup BT_BLE_TYPES
  * @{ */
/**
 * @brief Structure representing an attribute descriptor.
 *
 * This structure is used to define the parameters for an attribute descriptor,
 * including the attribute handle, reserved field, and attribute UUID.
 */
typedef struct att_desc_s {
  /** Attribute handle */
  uint8_t handle[2];
  /** Reserved */
  uint8_t reserved[2];
  /** Attribute UUID (attribute type) */
  uuid_t att_type_uuid;
} att_desc_t;

//characteristic service attribute value structure

/**
 * @brief Structure representing characteristic service data.
 *
 * This structure is used to define the parameters for characteristic service data,
 * including the characteristic value property, handle, and UUID.
 */
typedef struct char_serv_data_s {
  /** Characteristic value property */
  uint8_t char_property;
  /** Reserved */
  uint8_t reserved;
  /** Characteristic value handle */
  uint16_t char_handle;
  /** Characteristic value attributes UUID */
  uuid_t char_uuid;
} char_serv_data_t;

//characteristic service attribute structure

/**
 * @brief Structure representing a characteristic service.
 *
 * This structure is used to define the parameters for a characteristic service,
 * including the attribute handle, reserved space for future use, and the characteristic service attribute value.
 */
typedef struct char_serv_s {
  /** Characteristic service attribute handle */
  uint16_t handle;
  /** Reserved for future use */
  uint8_t reserved[2];
  /** Characteristic service attribute value */
  char_serv_data_t char_data;
} char_serv_t;

//include service attribute value structure
/**
 * @brief Structure representing the included service data.
 */
typedef struct inc_serv_data_s {

  /**include service start handle*/
  uint16_t start_handle;
  /**include service end handle*/
  uint16_t end_handle;
  /**UUID value of the included service*/
  uuid_t uuid;
} inc_serv_data_t;

// include service attribute structure
/**
 * @brief Structure representing an included service attribute in a GATT server.
 *
 */
typedef struct inc_serv_s {
  /**Include service attribute handle*/
  uint16_t handle;
  /**Reserved field */
  uint8_t reserved[2];
  /**Include service attribute data structure */
  inc_serv_data_t inc_serv;
} inc_serv_t;

/** @} */
/** @addtogroup BT_BLE_TYPES
  * @{ */
// GATT Request structures
// add new attributes to service request structure
/**
 * @brief Structure representing a request to add an attribute in the BLE stack.
 */
typedef struct rsi_ble_req_add_att_s {
  /** service handler */
  void *serv_handler;
  /** Attribute handle */
  uint16_t handle;
  /** If this variable is 1, Host has to maintain attributes and records in the application. 
-
      If 0, Stack will maintain the attributes and records
   */
  uint16_t config_bitmap;
  /** Attribute type UUID */
  uuid_t att_uuid;
  /** Attribute property */
  uint8_t property;
  /** Attribute data length */
  uint16_t data_len;
  /** Attribute data. The maximum value is 240, refer to RSI_DEV_ATT_LEN Macro*/
  uint8_t data[RSI_DEV_ATT_LEN];
} rsi_ble_req_add_att_t;
/** @} */

// GATT Response structures

//Presentation Format descriptor structure
// actual value =  char value * 10 ^ exponent;
/**
 * @brief The Structure represents the presentation format of a service characteristic.
 */
typedef struct rsi_ble_presentation_format {
  /**Format of the data*/
  uint8_t format;
  /**Exponent of the data*/
  uint8_t exponent;
  /**uints of the data*/
  uint16_t unit;
  /**name space of the data*/
  uint8_t name_space;
  /**Description for data*/
  uint16_t description;
} rsi_ble_pesentation_format_t;

/** @addtogroup BT_BLE_EVENT_TYPES Event Types
  * @{ */

//RSI_BLE_EVENT_GATT_ERROR_RESP, event_id: 0x1500
/**
 * @brief The Structure represents the error response event of GATT.
 */
typedef struct rsi_ble_event_error_resp_s {

  /**remote device address*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**attribute handle*/
  uint8_t handle[2];
  /**Error indicates the type of Gatt Error*/
  uint8_t error[2];
} rsi_ble_event_error_resp_t;

//RSI_BLE_EVENT_GATT_CHAR_DESC - event_ix = 1501

/**
 * @brief Structure representing a GATT descriptor event.
 */
typedef struct rsi_ble_event_gatt_desc_s {
  /**remote device address*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**number of descriptors found*/
  uint8_t num_of_att;
  /**Reserved for future use*/
  uint8_t reserved;
  /**Attribute descriptors list. The maximum value is 5*/
  att_desc_t att_desc[RSI_BLE_MAX_RESP_LIST];
} rsi_ble_event_gatt_desc_t;

//RSI_BLE_EVENT_GATT_PRIMARY_SERVICE_LIST, event_id: 0x1509

/**
 * @brief Structure representing the BLE event profiles list.
 *
 * This structure is used to define the parameters for the BLE event profiles list,
 * including the remote device address, number of profiles found, and the list of found profiles.
 */
typedef struct rsi_ble_event_profiles_list_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Number of profiles found */
  uint8_t number_of_profiles;
  /** Reserved for future use */
  uint8_t reserved;
  /** List of found profiles. The maximum value is 5 */
  profile_descriptors_t profile_desc[RSI_BLE_MAX_RESP_LIST];
} rsi_ble_event_profiles_list_t;

//RSI_BLE_EVENT_GATT_PRIMARY_SERVICE_BY_UUID, event_id = 0x1502
/**
 * @brief Structure to hold the BLE event profile by UUID.
 */
typedef struct rsi_ble_event_profile_by_uuid_s {

  /**remote device address*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**profile start handle*/
  uint8_t start_handle[2];
  /**profile end handle*/
  uint8_t end_handle[2];
} rsi_ble_event_profile_by_uuid_t;

//RSI_BLE_EVENT_GATT_READ_CHAR_SERVS, event_id = 0x1503
/**
 * @brief Structure representing the BLE event for reading characteristic services by type.
 *
 * This structure is used to represent the BLE event `RSI_BLE_EVENT_GATT_READ_CHAR_SERVS` 
 * with event ID `0x1503`. It provides information related to the read operation 
 * of characteristic services.
 */
typedef struct rsi_ble_event_read_by_type1_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Number of characteristic services found */
  uint8_t num_of_services;
  /** Reserved for future use */
  uint8_t reserved;
  /** It contains the characteristic service list. The maximum value is 5. */
  char_serv_t char_services[RSI_BLE_MAX_RESP_LIST];
} rsi_ble_event_read_by_type1_t;

//RSI_BLE_EVENT_GATT_READ_INC_SERVS, event_id = 0x1504
/**
 * @brief Structure contains the service information received during the GATT read included services event.
 */
typedef struct rsi_ble_event_read_by_type2_s {

  /**remote device address*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**number of services found*/
  uint8_t num_of_services;
  /**Reserved for future use*/
  uint8_t reserved;
  /**list of included services. The maximum value is 5*/
  inc_serv_t services[RSI_BLE_MAX_RESP_LIST];
} rsi_ble_event_read_by_type2_t;

//RSI_BLE_EVENT_GATT_READ_VAL_BY_UUID, event_id = 0x1505
/**
 * @brief Structure contains the service information received during the GATT read value by UUID event.
 */
typedef struct rsi_ble_event_read_by_type3_s {

  /**Remote device address*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**Connection handle*/
  uint8_t handle[2];
  /**Total length of the att_value list*/
  uint16_t length;
  /**This contains the attribute handle-value pairs. The maximum value is 240. For more information, see the RSI_DEV_ATT_LEN macro.*/
  /**Index 0 of att_value contains the length of each attribute handle-value pair. **/
  uint8_t att_value[RSI_DEV_ATT_LEN];
} rsi_ble_event_read_by_type3_t;

//RSI_BLE_EVENT_GATT_READ_RESP , evet_id = 0x1506,0x1507,0x1508
/**
 * @brief Structure representing the ATT value on GATT read response event.
 */
typedef struct rsi_ble_event_att_value_s {

  /**remote device address*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**length of attribute value*/
  uint16_t length;
  /**This contains the attribute value. The maximum value is 240, refer to RSI_DEV_ATT_LEN Macro*/
  uint8_t att_value[RSI_DEV_ATT_LEN];
} rsi_ble_event_att_value_t;

//RSI_BLE_EVENT_GATT_WRITE_RESP, event_id: 0x150A,0x150C
/**
 * @brief The structure represents the set attribute response.
 */
typedef struct rsi_ble_set_att_resp_s {
  /**remote device address*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_ble_set_att_resp_t;

//RSI_BLE_EVENT_GATT_PREPARE_WRITE_RESP, event_id: 0x150B
/**
 * @brief The structure represents the prepare write response.
 */
typedef struct rsi_ble_prepare_write_resp_s {

  /**remote device address*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**attribute handle*/
  uint8_t handle[2];
  /**attribute value offset*/
  uint8_t offset[2];
  /**length of attribute value*/
  uint8_t length;
  /**This contains the attribute value. The maximum value is 240, refer to RSI_DEV_ATT_LEN Macro*/
  uint8_t att_value[RSI_DEV_ATT_LEN];
} rsi_ble_prepare_write_resp_t;

// GATT Profiles list response structure
/**
 * @brief The structure shows the list of received profiles. The maximum supported number of received profiles is 5.
 */
typedef struct rsi_ble_resp_profiles_list_s {
  /** Number of profiles received */
  uint8_t number_of_profiles;
  /** Reserved */
  uint8_t reserved[3];
  /** List of received profile descriptors.
-
      The maximum value is 5. */
  profile_descriptors_t profile_desc[RSI_BLE_MAX_RESP_LIST];
} rsi_ble_resp_profiles_list_t;

/**
 * @brief The Structure represents the list of received profile descriptors of a particular profile. The maximum supported number of received profile descriptors is 5.
 */
typedef struct rsi_ble_resp_query_profile_descriptor_s {
  /**remote device address*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**List of recieved profile descriptors.
-
      The maximum value is 5. */
  profile_descriptors_t profile_desc[RSI_BLE_MAX_RESP_LIST];
} rsi_ble_resp_query_profile_descriptor_t;

// GATT characteristic query service response structure

#ifndef RSI_BLE_APIS_H
#define RSI_BLE_APIS_H

#include <stdint.h>

#endif /* RSI_BLE_APIS_H */
/**
 * @brief Structure to hold the response of characteristic services.
 */
typedef struct rsi_ble_resp_char_serv_s {
  /** The number of profiles found */
  uint8_t num_of_services;
  /** Reserved */
  uint8_t reserved[3];
  /** Characteristic service array. 
-
      The maximum value is 5. */
  char_serv_t char_services[RSI_BLE_MAX_RESP_LIST];
} rsi_ble_resp_char_services_t;

// GATT include service response structure
/**
 * @brief The structure represents the list of received included service attributes of a particular included service. The maximum supported number of received included service attributes is 5.
 *        
 */
typedef struct rsi_ble_resp_inc_serv {
  /** Number of profiles found */
  uint8_t num_of_services;
  /** Reserved */
  uint8_t reserved[3];
  /** Included services list. 
-
      The maximum value is 5. */
  inc_serv_t services[RSI_BLE_MAX_RESP_LIST];
} rsi_ble_resp_inc_services_t;

// GATT attribute value response structure
/**
 * @brief Structure represents the response for a Attribute Protocol (ATT) value.
 */
typedef struct rsi_ble_resp_att_value_s {
  /** Length of attribute value */
  uint8_t len;
  /** Attribute values list. 
- 
      Each attribute value is maximum of size 240, refer to RSI_DEV_ATT_LEN Macro */
  uint8_t att_value[RSI_DEV_ATT_LEN];
} rsi_ble_resp_att_value_t;

// GATT attribute descriptors response structure
/**
 * @brief Structure represents the list of received attribute descriptors. The maximum supported number of received attribute descriptors is 5
 */
typedef struct rsi_ble_resp_att_descs_s {
  /** Number of descriptors found */
  uint8_t num_of_att;
  /** Reserved */
  uint8_t reserved[3];
  /** Attribute descriptors list. 
-
      The maximum value is 5. */
  att_desc_t att_desc[RSI_BLE_MAX_RESP_LIST];
} rsi_ble_resp_att_descs_t;

// add new service response structure
/**
 * @brief Structure represents the add new service response.
 */
typedef struct rsi_ble_resp_add_serv_s {
  /** Contains the address of the service record stored in the Silicon Labs stack. */
  void *serv_handler;
  /** The handle from where the service starts. */
  uint16_t start_handle;
} rsi_ble_resp_add_serv_t;

// read local attribute value response structure
/**
 * @brief Structure represents the response for local attribute value.
 */
typedef struct rsi_ble_resp_local_att_value_s {
  /** Attribute handle */
  uint16_t handle;
  /** Attribute value length */
  uint16_t data_len;
  /** Attribute value (data). The maximum value is 240, refer to RSI_DEV_ATT_LEN Macro */
  uint8_t data[RSI_DEV_ATT_LEN];
} rsi_ble_resp_local_att_value_t;

/**
 * @brief Structure represents the remote device information event.
 *
 * This structure is used to define the parameters for a BLE event
 * that provides information about a remote device.
 */
typedef struct rsi_ble_event_remote_device_info_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Remote device version */
  uint8_t remote_version;
  /** Remote device company ID */
  uint16_t remote_company_id;
  /** Remote device sub-version */
  uint16_t remote_sub_version;
} rsi_ble_event_remote_device_info_t;

/**
 * @brief Structure represents the received Remote Control Protocol (RCP) information event.
 *
 * This structure is used to define the parameters for a BLE event
 * that involves receiving RCP information.
 */
typedef struct rsi_ble_event_rcp_rcvd_info_s {
  /** Received RCP data */
  uint8_t data[1024];
} rsi_ble_event_rcp_rcvd_info_t;
// GATT Event structures

// GATT Write event structure
/**
 * @brief Structure indicates the received write information from the remote device.
 */
typedef struct rsi_ble_event_write_s {
  /** remote device address  */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Reserved for future use */
  uint8_t reserved;

/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */

/// BLE write command event
#define RSI_BLE_WRITE_CMD_EVENT 0x01
/// BLE write request event
#define RSI_BLE_WRITE_REQUEST_EVENT 0x02
/// BLE notification event
#define RSI_BLE_NOTIFICATION_EVENT 0x03
/// BLE indication event
#define RSI_BLE_INDICATION_EVENT 0x04
  /** @} */
  /**Type of the event received from the remote device 
-
     RSI_BLE_WRITE_CMD_EVENT     0x01 
-
     RSI_BLE_WRITE_REQUEST_EVENT 0x02 
-
     RSI_BLE_NOTIFICATION_EVENT  0x03 
-
     RSI_BLE_INDICATION_EVENT    0x04 */
  uint8_t pkt_type;
  /**attribute handle*/
  uint8_t handle[2];
  /**length of attribute value*/
  uint8_t length;
  /**This contains the attribute value. The maximum value is 240, refer to RSI_DEV_ATT_LEN Macro*/
  uint8_t att_value[RSI_DEV_ATT_LEN];
} rsi_ble_event_write_t;

// GATT prepare write event structure
/**
 * @brief Structure represents the GATT prepare write event.
 */
typedef struct rsi_ble_event_prepare_write_s {

  /**remote device address*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**attribute handle*/
  uint8_t handle[2];
  /**attribute value offset*/
  uint8_t offset[2];
  /**length of attribute value*/
  uint16_t length;
  /**This contains the attribute value. The maximum value is 240, refer to RSI_DEV_ATT_LEN Macro*/
  uint8_t att_value[RSI_DEV_ATT_LEN];
} rsi_ble_event_prepare_write_t;

/**
 * @brief Structure representing the BLE execute write request.
 *
 * This structure is used to define the parameters for an execute write request
 * to a remote BLE device.
 */
typedef struct rsi_ble_execute_write_s {
  /** Remote device address (6 bytes) */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Execute write flag */
  uint8_t exeflag;
} rsi_ble_execute_write_t;

// GATT Read event structure

/**
 * @brief Structure representing the BLE read request.
 *
 * This structure is used to define the parameters for a BLE read request,
 * including the remote device address, attribute handle, request type, and offset.
 */
typedef struct rsi_ble_read_req_s {
  /** Remote device address */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Attribute handle */
  uint16_t handle;
  /** Request type: 0 - Read request, 1 - Read Blob request */
  uint8_t type;
  /** Reserved for future use */
  uint8_t reserved;
  /** Offset of attribute value to be read */
  uint16_t offset;
} rsi_ble_read_req_t;

// GATT MTU event structure
/**
 * @brief Structure to hold the BLE MTU event information.
 */
typedef struct rsi_ble_event_mtu_s {

  /**remote device address*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**MTU size*/
  uint16_t mtu_size;
} rsi_ble_event_mtu_t;
/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */
#define PEER_DEVICE_INITATED_MTU_EXCHANGE \
  0x1 ///< Indicates that the Maximum Transmission Unit (MTU) exchange was initiated by the peer device.
#define LOCAL_DEVICE_INITATED_MTU_EXCHANGE 0x2 ///< Indicates that the MTU exchange was initiated by the local device.
/** @} */
//MTU Exchange Information event structure
/**
 * @brief Structure to hold the MTU exchange information event.
 */
typedef struct rsi_ble_event_mtu_exchange_information_s {
  /**uint8_t[6], remote device address*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**uint8_t[2], Remote MTU Size*/
  uint16_t remote_mtu_size;
  /**uint8_t[2], Local MTU Size*/
  uint16_t local_mtu_size;
  /**uint8_t Initiated role, who initiated MTU exchange 
-
     PEER_DEVICE_INITATED_MTU_EXCHANGE     0x01 
-
     LOCAL_DEVICE_INITATED_MTU_EXCHANGE    0x02  */
  uint8_t initiated_role;
} rsi_ble_event_mtu_exchange_information_t;

/**
 * @brief Structure to hold the information of a directed advertising report event.
 */
typedef struct rsi_ble_event_directedadv_report_s {
  /**Event type 
-
      0x01	Connectable directed advertising (ADV_DIRECT_IND) */
  uint16_t event_type;
  /**Address type of remote device*/
  uint8_t dev_addr_type;
  /**Address of the remote device*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /**Directed address type*/
  uint8_t directed_addr_type;
  /**Directed device address*/
  uint8_t directed_dev_addr[RSI_DEV_ADDR_LEN];
  /**rssi value*/
  int8_t rssi;
} rsi_ble_event_directedadv_report_t;

/**
 * @brief Structure representing the CBFC connection request event.
 *
 * This structure is used to define the parameters for a BLE event
 * that involves a Circular Buffer Flow Control (CBFC) connection request from a remote device.
 */
typedef struct rsi_ble_event_cbfc_conn_req_s {
  /** Address of the remote device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Protocol/Service Multiplexer (PSM) value */
  uint16_t psm;
  /** Local Channel Identifier (LCID) */
  uint16_t lcid;
} rsi_ble_event_cbfc_conn_req_t;

/**
 * @brief Structure representing the CBFC connection complete event.
 *
 * This structure is used to define the parameters for a BLE event
 * that indicates the completion of a Circular Buffer Flow Control (CBFC) connection with a remote device.
 */
typedef struct rsi_ble_event_cbfc_conn_complete_s {
  /** Address of the remote device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Protocol/Service Multiplexer (PSM) value */
  uint16_t psm;
  /** Maximum Transmission Unit (MTU) size */
  uint16_t mtu;
  /** Maximum PDU Size (MPS) */
  uint16_t mps;
  /** Local Channel Identifier (LCID) */
  uint16_t lcid;
} rsi_ble_event_cbfc_conn_complete_t;

/**
 * @brief Structure representing the CBFC received data event.
 *
 * This structure is used to define the parameters for a BLE event
 * that involves receiving data over a Circular Buffer Flow Control (CBFC) connection from a remote device.
 */
typedef struct rsi_ble_event_cbfc_rx_data_s {
  /** Address of the remote device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Local Channel Identifier (LCID) */
  uint16_t lcid;
  /** Length of the received data */
  uint16_t len;
  /** Received data */
  uint8_t data[RSI_DEV_ATT_LEN];
} rsi_ble_event_cbfc_rx_data_t;

/**
 * @brief Structure representing the CBFC disconnection event.
 *
 * This structure is used to define the parameters for a BLE event
 * that involves the disconnection of a Circular Buffer Flow Control (CBFC) connection with a remote device.
 */
typedef struct rsi_ble_event_cbfc_disconn_s {
  /** Address of the remote device */
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** Local Channel Identifier (LCID) */
  uint16_t lcid;
} rsi_ble_event_cbfc_disconn_t;

/** @addtogroup BT-LOW-ENERGY1 GAP
  * @{ */
// SMP pairing capabilities    cmd_ix = 0x00FE
/**
 * @brief Structure for setting SMP pairing capability data.
 */
typedef struct rsi_ble_set_smp_pairing_capabilty_data {

  /**Device input output capability 
-
      0x00 - Display Only 
-
      0x01 - Display Yes/No 
-
      0x02 - Keyboard Only  
-
      0x03 - No Input No Output
      0x04 - Keyboard Display*/
  uint8_t io_capability;
  /** oob_data_flag 
-
      0 - disable 
- 1 - enable */
  uint8_t oob_data_flag;
  /** Authentication Request contains bonding type 
-
      MITM Request - BIT(2) 
-
      Secure Connections - BIT(3) 
-
      Keypress - BIT(4) 
-
      CT2 - BIT(5) */
  uint8_t auth_req;
  /** Supported Encryption key size 7 to 16 bytes */
  uint8_t enc_key_size;
  /** Initiator generates/requires the no .of keys after successful paring 
-
      BIT(0) - EncKey: Initiator distributes the LTK followed by EDIV and Rand 
-
      BIT(1) - IdKey : Initiator distributes the IRK followed by its address 
-
      BIT(2) - Sign  : Initiator distributes the CSRK 
-
      BIT(3) - BIT(7): Reserved for future use */
  uint8_t ini_key_distribution;
  /** Responder generates/requires the no .of keys after successful paring 
-
      BIT(0) - EncKey: Responder distributes the LTK followed by EDIV and Rand 
-
      BIT(1) - IdKey : Responder distributes the IRK followed by its address 
-
      BIT(2) - Sign  : Responder distributes the CSRK 
-
      BIT(3) - BIT(7): Reserved for future use */
  uint8_t rsp_key_distribution;

} rsi_ble_set_smp_pairing_capabilty_data_t;

//LE read PHY request command response structure, cmd_ix - 0x00B0

/**
 * @brief Structure to hold the response for the BLE read PHY request command.
 */
typedef struct rsi_ble_resp_read_phy_s {
  /** Remote device Bluetooth Address*/
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  /** TX PHY Rate 
-
       0x01 The transmitter PHY for the connection is LE 1M 
-
       0x02 The transmitter PHY for the connection is LE 2M 
-
       0x03 The transmitter PHY for the connection is LE Coded 
-
       All other values Reserved for future use*/
  uint8_t tx_phy;
  /** RX PHY Rate 
-
       0x01 The receiver PHY for the connection is LE 1M 
-
       0x02 The receiver PHY for the connection is LE 2M 
-
       0x03 The receiver PHY for the connection is LE Coded 
-
       All other values Reserved for future use*/
  uint8_t rx_phy;
} rsi_ble_resp_read_phy_t;

//LE read max data length command response structure, cmd_ix - 0x00B3
/**
 * @brief Structure representing the response for reading the maximum data length in BLE.
 */
typedef struct rsi_ble_resp_read_max_data_length_s {
  /** maxtxoctets 
- 
       	Preferred maximum number of payload octets that the local Controller should
        include in a single Link Layer packet on this connection. */
  uint16_t maxtxoctets;
  /** maxtxtime 
- 
	    Preferred maximum number of microseconds that the local Controller
        should use to transmit a single Link Layer packet on this connection */
  uint16_t maxtxtime;
  /** maxrxoctets 
- 
	    Maximum number of payload octets that the local Controller supports
        for reception of a single Link Layer packet on a data connection */
  uint16_t maxrxoctets;
  /** maxrxtime 
- 
	    Maximum time, in microseconds, that the local Controller supports for
        reception of a single Link Layer packet on a data connection. */
  uint16_t maxrxtime;
} rsi_ble_read_max_data_length_t;

/** @} */

/** @addtogroup BT-LOW-ENERGY6 Test Mode
  * @{ */
//LE PER Transmit mode, cmd_ix = 0x00BE

/**
 * @brief Structure to hold the parameters for BLE transmit PER mode.
 */
typedef struct rsi_ble_per_transmit_s {
  /** Command ID 
-
   Takes PER BLE_TRANSMIT_CMD_ID of value 0x13*/
  uint8_t cmd_ix;
  /** Enables/disables the BLE transmit PER mode 
-
     1  PER Transmit Enable 
-
     0  PER Transmit Disable */
  uint8_t transmit_enable;
  /** Access address with which packets are transmitted */
  uint8_t access_addr[4];
  /** Phy rate at which packets are transmitted 
-
       1  1Mbps  
-
       2  2 Mbps 
-
       4  125 Kbps Coded 
-
       8  500 Kbps Coded */
  uint8_t phy_rate;
  /** Rx channel number (0 - 39) 
     @note Removed the BLE packet transmission on channel-39 (2480 MHz) at a 2 Mbps data rate */
  uint8_t rx_chnl_num;
  /** Tx channel number (0 - 39) 
     @note Removed the BLE packet transmission on channel-39 (2480 MHz) at a 2 Mbps data rate */
  uint8_t tx_chnl_num;
  /** Initial seed to be used for whitening. It should be set to 0 in order to disable whitening. 
-
      In order to enable, one should give the scrambler seed value which is used on the receive side.

      @note Set value to 5 for the continuous mode
       */
  uint8_t scrambler_seed;
  /** LE channel type (data or advertise channel) 
-
       0x00  Advertise Channel 
-
       0x01  Data Channel (to be used by Default) */
  uint8_t le_chnl_type;
  /** Frequency hopping type to be used 
-
       0  No Hopping 
-
       1  Fixed Hopping 
-
       2  Random Hopping (rx_chnl_num, tx_chnl_num parameters are unused in this mode) */
  uint8_t freq_hop_en;
  /** Select the antenna to be used. Refer to the data sheet for your hardware to check whether or not it contains an onboard antenna. 
-
       2  ONBOARD_ANT_SEL 
-
       3  EXT_ANT_SEL */
  uint8_t ant_sel;
  /** pll_mode type to be used 
- 
       0  PLL_MODE0 (to be used by Default) 
- 
       1  PLL_MODE1 */
  uint8_t pll_mode;
  /** Selection of RF type (internal/external) 
-
       0  BT_EXTERNAL_RF 
-
       1  BT_INTERNAL_RF (to be used by Default) 
      @note  The above macros are applicable for both BT and BLE */
  uint8_t rf_type;
  /** Selection of RF Chain (HP/LP) to be used 
-
       2  BT_HP_CHAIN 
-
       3  BT_LP_CHAIN
      @note  The above macros are applicable for both BT and BLE */
  uint8_t rf_chain;
  /** Length of the packet to be transmitted*/
  uint8_t pkt_len[2];
  /** Type of payload data sequence 
-
       0x00  PRBS9 sequence �11111111100000111101... 
-
       0x01  Repeated �11110000� 
-
       0x02  Repeated �10101010� 
-
       0x03  PRBS15 
-
       0x04  Repeated �11111111� 
-
       0x05  Repeated �00000000� 
-
       0x06  Repeated '00001111' 
-
       0x07  Repeated '01010101' */
  uint8_t payload_type;
  /** Transmit Power 
- 
       Default Value for BLE TX Power Index is 31, The range for the BLE TX Power Index is 1 to 127 (0, 32 indexes are invalid). 
-
       TX power index for the BLE LP Chain : 1 to 31 (0 dBm Mode), 33 to 63 ( 10 dBm Mode) 
-
       TX power index for the BLE HP Chain : 1 to 127 
-
     @note  For the LP Chain - Power index vs Outpt power in dBm.

 *    -----------------------------------------------------------------------------------------
 *   |       Power Index   |                  Output Power in dBm                              |
 *   ----------------------|-------------------------------------------------------------------
 *   |      1              |      		 -22.3054959      			       |
 *   |      2              |      		 -16.59332574     			       |
 *   |      3              |      		 -13.38278365     			       |
 *   |      4              |      		 -11.19804718                                  |
 *   |      5              |      		 -9.576522466                                  |
 *   |      6              |      		 -8.312070432                                  |
 *   |      7              |      		 -7.294640362                                  |
 *   |      8              |      		 -6.458448154                                  |
 *   |      9              |      		 -5.760963318                                  |
 *   |      10             |      		 -5.173042366                                  |
 *   |      11             |      		 -4.673788189                                  |
 *   |      12             |      		 -4.247653993                                  |
 *   |      13             |      		 -3.882708784                                  |
 *   |      14             |      		 -3.569545894                                  |
 *   |      15             |      		 -3.300567503                                  |
 *   |      16             |      		 -3.069499167                                  |
 *   |      17             |      		 -2.871050592                                  |
 *   |      18             |      		 -2.700672503                                  |
 *   |      19             |      		 -2.554378603                                  |
 *   |      20             |      		 -2.428612817                                  |
 *   |      21             |      		 -2.32014891                                   |
 *   |      22             |      		 -2.226013876                                  |
 *   |      23             |      		 -2.143429275                                  |
 *   |      24             |      		 -2.069766557                                  |
 *   |      25             |      		 -2.002513642                                  |
 *   |      26             |      		 -1.939250859                                  |
 *   |      27             |      		 -1.87763493                                   |
 *   |      28             |      		 -1.815390046                                  |
 *   |      29             |      		 -1.750305305                                  |
 *   |      30             |      		 -1.680237892                                  |
 *   |      31             |      		 -1.603121401                                  |
 *   |      32             |                     NA		  		               |
 *   |      33             |                     -10.4822997	  		               |
 *   |      34             |                     -4.9531679	  		               |
 *   |      35             |                     -1.931961022	  		               |
 *   |      36             |                     0.057132993	  		               |
 *   |      37             |                     1.476764101	  		               |
 *   |      38             |                     2.5332116	  		               |
 *   |      39             |                     3.336771823	  		               |
 *   |      40             |                     3.953605265	  		               |
 *   |      41             |                     4.426779615	  		               |
 *   |      42             |                     4.786171523	  		               |
 *   |      43             |                     5.053647759	  		               |
 *   |      44             |                     5.246007208	  		               |
 *   |      45             |                     5.37676618	  		               |
 *   |      46             |                     5.457304255	  		               |
 *   |      47             |                     5.497635316	  		               |
 *   |      48             |                     5.506945838	  		               |
 *   |      49             |                     5.493978354	  		               |
 *   |      50             |                     5.467302132	  		               |
 *   |      51             |                     5.435491631	  		               |
 *   |      52             |                     5.407220119	  		               |
 *   |      53             |                     5.391268248	  		               |
 *   |      54             |                     5.396444507	  		               |
 *   |      55             |                     5.431416481	  		               |
 *   |      56             |                     5.504458826	  		               |
 *   |      57             |                     5.62313521	  		               |
 *   |      58             |                     5.793945208	  		               |
 *   |      59             |                     6.02197959	  		               |
 *   |      60             |                     6.310634089	  		               |
 *   |      61             |                     6.661428559	  		               |
 *   |      62             |                     7.073964236	  		               |
 *   |      63             |                     7.546029076	  		               |
 *   -----------------------------------------------------------------------------------------
-
     @note  For the HP Chain - Power index vs Outpt power in dBm.

 *    -----------------------------------------------------------------------------------------
 *   |       Power Index   |                  Output Power in dBm                              |
 *   ----------------------|-------------------------------------------------------------------
 *   |         1           |      		    1	 	  			       |
 *   |         2           |      		    2	   				       |
 *   |         3           |      		    3	   				       |
 *   |         4           |      		    4	                        	       |
 *   |         5           |      		    5					       |
 *   |         6           |      		    6	                                       |
 *   |         7           |      		    7	                                       |
 *   |         8           |      		    8	                                       |
 *   |         9           |      		    9	                                       |
 *   |         10          |      		    10	                                       |
 *   |         -           |      		    -	                                       |
 *   |         -           |      		    -	                                       |
 *   |         -           |      		    -	                                       |
 *   |         -           |      		    -	                                       |
 *   |         41          |      		    0.5                                        |
 *   |         42          |      		    1	                                       |
 *   |         43          |      		    1.5                                        |
 *   |         44          |      		    2	                                       |
 *   |         45          |      		    2.5                                        |
 *   |         46          |      		    3	                                       |
 *   |         47          |      		    3.5                                        |
 *   |         48          |      		    4	                                       |
 *   |         49          |      		    4.5                                        |
 *   |         50          |      		    5	                                       |
 *   |         -           |      		    -	                                       |
 *   |         -           |      		    -	                                       |
 *   |         127         |   		(Max Power Supported by Country region)                |
 *   ----------------------|-------------------------------------------------------------------
 */

  uint8_t tx_power;
  /** Transmit mode to be used either Burst/Continuous 
-
       0  BURST_MODE 
-
       1  CONTINUOUS_MODE 
-
       2  CONTINUOUS_WAVE_MODE (CW_MODE) */
  uint8_t transmit_mode;
  /** This field takes the value of inter packet gap. 
-
      Number of slots to be skipped between two packets - Each slot will be 1250 usec */
  uint8_t inter_pkt_gap;
  /** This field defines the number of packets to be transmitted, default to zero for continuous transmission */
  uint8_t num_pkts[4];
} rsi_ble_per_transmit_t;

//LE Per Receive mode, cmd_ix = 0x00BF
/**
 * @brief Structure representing the parameters for enabling/disabling Rx data stream in PER mode.
 */
typedef struct rsi_ble_per_receive_s {
  /** Command ID 
-
   Takes per BLE_RECEIVE_CMD_ID of value 0x14*/
  uint8_t cmd_ix;
  /** Enables/disables the BLE receive PER mode.
-
     1  PER Receive Enable 
-
     0  PER Receive Disable */
  uint8_t receive_enable;
  /** Access address with which packets are received */
  uint8_t access_addr[4];
  /** Phy rate at which packets are received 
-
       1  1Mbps  
-
       2  2 Mbps 
-
       4  125 Kbps Coded 
-
       8  500 Kbps Coded */
  uint8_t phy_rate;
  /** Rx channel number (0 - 39) */
  uint8_t rx_chnl_num;
  /** Tx channel number (0 - 39) */
  uint8_t tx_chnl_num;
  /** Initial seed to be used for whitening. It should be set to 0 in order to disable whitening. 
-
      In order to enable, one should give the scrambler seed value which is used on the transmit side */
  uint8_t scrambler_seed;
  /** LE channel type (data or advertise channel) 
-
       0x00  Advertise Channel 
-
       0x01  Data Channel (to be used by Default) */
  uint8_t le_chnl_type;
  /** Frequency hopping type to be used 
-
       0  No Hopping 
-
       1  Fixed Hopping 
-
       2  Random Hopping (rx_chnl_num, tx_chnl_num parameters are unused in this mode) */
  uint8_t freq_hop_en;
  /** Select the antenna to be used. Refer to the datasheet for your hardware to check whether or not it contains an onboard antenna. 
- 
       2  ONBOARD_ANT_SEL 
-
       3  EXT_ANT_SEL */
  uint8_t ant_sel;
  /** pll_mode type to be used 
- 
       0  PLL_MODE0 (to be used by Default) 
- 
       1  PLL_MODE1 */
  uint8_t pll_mode;
  /** Selection of RF type (internal/external) 
-
       0  BT_EXTERNAL_RF 
-
       1  BT_INTERNAL_RF (to be used by Default) 
      @note  The above macros are applicable for both BT and BLE */
  uint8_t rf_type;
  /** Selection of RF Chain (HP/LP) to be used 
-
       2  BT_HP_CHAIN 
-
       3  BT_LP_CHAIN
      @note  The above macros are applicable for both BT and BLE */
  uint8_t rf_chain;
  /** This field enables/disables the extended data length 
-
       0  Extended Data length disabled 
-
       1  Extended Data length enabled */
  uint8_t ext_data_len_indication;
  /** This field defines the loopback to be enable or disable 
-
       0  LOOP_BACK_MODE_DISABLE 
-
       1  LOOP_BACK_MODE_ENABLE*/
  uint8_t loop_back_mode;
  /** This field enables/disables the duty cycling 
-
       0  Duty Cycling Disabled (to be used by Default) 
-
       1  Duty Cycling Enabled */
  uint8_t duty_cycling_en;
} rsi_ble_per_receive_t;
/** @} */
/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */

///The maximum length of advertising data.
#define ADV_DATA_LEN 210
///Length of the device address in bytes.
#define DEVICE_ADDR_LEN 6
/** @} */
//! ae adv report event
typedef struct rsi_ble_ae_adv_report_s {
  /**
 *    -----------------------------------------------------------------------------------------
 *   |       Bit Number    |                    Parameter Description                          |
 *   ----------------------|-------------------------------------------------------------------
 *   |      0              |                   Connectable Advertising                         |
 *   |      1              |                   Scannable Advertising                           |
 *   |      2              |                   Direct Advertising                              |
 *   |      3              |                    Scan Response                                  |
 *   |      4              |                 Legacy Advertising PDUs used                      |
 *   |    5 to 6           |                       Data status : 
 *                            |
 *   |                     |             0b00  = complete 
 *                                   |
 *   |                     |             0b01  = Incomplete, more data to come 
 *              |
 *   |                     |             0b10  = Incomplete, data truncated, no more to come 
 *|
 *   |                     |             0b11  = Reserved for future use                       |
 */
  uint16_t event_type;
  /**
    uint8_t Remote Address Type, Indicates the type of the Address
         0x00 - Public Device Address
         0x01 - Random Device Address
         0x02 - Public Identity Address (corresponds to Resolved Private Address)
         0x03 - Random (static) Identity Address (corresponds to Resolved Private Address)
   */
  uint8_t remote_addr_type;
  /**  uint8[6] remote_Address : Address of the remote address Type */
  uint8_t remote_addr[DEVICE_ADDR_LEN];
  /**
       uint8_t Primary PHY
        0x01 - Advertiser PHY is LE 1M
        0x03 - Advertiser PHY is LE Coded
  */
  uint8_t pri_phy;
  /**
       uint8_t Secondary PHY
          0x00 - No packets on the secondary advertising physical channel
          0x01 - Advertiser PHY is LE 1M
          0x02 - Advertiser PHY is LE 2M
          0x03 - Advertiser PHY is LE Coded
  */
  uint8_t sec_phy;
  /** uint8_t Advertising_SID
   *    ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
   *   |           Value     |                    Parameter Description                                                                                                                                         |
   *   ----------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
   *   |       0x00 to 0x0F  |  Value of the Advertising SID subfield in the ADI field of the PDU or, 
 *  for  scan responses, in the ADI field of the original scannable advertisement                         |
   *   |           0xFF      |                   No ADI field provided                                                                                                                                          |
   *
   */
  uint8_t SID;
  /**
    uint8_t TX_Power, It shall be set based on the AUX_SYNC_IND PDU
     TX_Power ranges from -127 to +20 and it's units is in dBm
  */
  uint8_t tx_power;
  /**
    uint8_t RSSI , this parameter contains the RSSI value, excluding any constant tone Extension.
     RSSI ranges from -127 to +20 and it's units is in dBm
  */
  uint8_t RSSI;
  /** uint16_t Periodic_Advertising_Interval. This parameter specifies the interval between the periodic advertising events */
  uint16_t per_adv_interval;
  /**
   uint8_t Direct Address Type ,Indicates the type of the Address
         0x00 - Public Device Address
         0x01 - Random Device Address
         0x02 - Public Identity Address (corresponds to Resolved Private Address)
         0x03 - Random (static) Identity Address (corresponds to Resolved Private Address)
         0xFE - Resolves Private Address
  */
  uint8_t direct_addr_type;
  /** uint8[6] Direct_Address, Direct_Address of the Advertiser type */
  uint8_t direct_addr[DEVICE_ADDR_LEN];
  /** uint8_t Data _length , Length of the Data field for each device which responded , ranges from 0 to 229 */
  uint8_t data_len;
  /** uint8[256] Data */
  uint8_t data[ADV_DATA_LEN];
} rsi_ble_ae_adv_report_t;

//! ae periodic sync establishment report event
typedef struct rsi_ble_per_adv_sync_estbl_s {
  /**
     uint8_t status , It indicates whether Periodic Advertising is successful or not
              0 - Periodic advertising sync successful
             !0 - Periodic advertising sync failed
  */
  uint8_t status;
  /** uint16_t Sync_Handle, It identifies the periodic Advertising train. Range : 0x0000 to 0x0EFF */
  uint16_t sync_handle;
  /** uint8_t Advertising_SID,  Value of the Advertising SID subfield in the ADI field of the PDU, Range : 0x00 to 0x0F */
  uint8_t adv_sid;
  /**
    uint8_t Advertiser_Address_Type : Indicates the type of the Address
         0x00 - Public Device Address
         0x01 - Random Device Address
         0x02 - Public Identity Address (corresponds to Resolved Private Address)
         0x03 - Random (static) Identity Address (corresponds to Resolved Private Address)
  */
  uint8_t advertiser_addr_type;
  /** uint8[6], Advertiser_Address of the Advertiser type */
  uint8_t advertiser_addr[DEVICE_ADDR_LEN];
  /**
     uint8_t Advertiser_PHY. This parameter specifies the PHY used for the periodic advertising.
                 0x01 - Advertiser PHY is LE 1M
                 0x02 - Advertiser PHY is LE 2M
                 0x03 - Advertiser PHY is LE Coded
  */
  uint8_t advertiser_phy;
  /** uint16_t Periodic_Advertising_Interval. This parameter specifies the interval between the periodic advertising events. */
  uint16_t per_adv_interval;
  /**
    uint16_t Advertiser_Clock_Accuracy. This parameter specifies the accuracy of the periodic advertiser's clock.
                  0x00 - 500 ppm
                  0x01 - 250 ppm
                  0x02 - 150 ppm
                  0x03 - 100 ppm
                  0x04 - 75 ppm
                  0x05 - 50 ppm
                  0x06 - 30 ppm
                  0x07 - 20 ppm
  */
  uint16_t advertiser_clock_accuracy;
} rsi_ble_per_adv_sync_estbl_t;

//! ae periodic adv report event
typedef struct rsi_ble_per_adv_report_s {
  /** uint16_t Sync_Handle, It identifies the periodic Advertising train. Range : 0x0000 to 0x0EFF */
  uint16_t sync_handle;
  /** int8_t TX_Power, It shall be set based on the AUX_SYNC_IND PDU
     TX_Power ranges from -127 to +20 and it's units is in dBm
  */
  int8_t tx_power;
  /** int8_t RSSI , this parameter contains the RSSI value, excluding any constant tone Extension.
     RSSI ranges from -127 to +20 and it's units is in dBm
  */
  int8_t RSSI;
  /** Unused byte for alignment */
  uint8_t unused;
  /**
     uint8_t Data_Status, It specifies about the status of the data sent
                 0x00 - Data Complete
                 0x01 - Data Incomplete, more Data to come
  */
  uint8_t data_status;
  /** uint8_t Data_Length ,Length of the Data Field, Ranges from 0 to 247 */
  uint8_t data_len;
  /** uint8[256] Data, Data received from a Periodic Advertising Packet */
  uint8_t data[ADV_DATA_LEN];
} rsi_ble_per_adv_report_t;

//! ae periodic sync lost report event
typedef struct rsi_ble_per_adv_sync_lost_s {
  /** uint16_t Sync_Handle, It identifies the periodic Advertising train. Range : 0x0000 to 0x0EFF */
  uint16_t sync_handle;
} rsi_ble_per_adv_sync_lost_t;

//! ae scan timeout report event
typedef struct rsi_ble_scan_timeout_s {
  /** uint8_t status , Status indicates that scanning has ended because the duration has expired */
  uint8_t status;
} rsi_ble_scan_timeout_t;

//! ae adv set terminated report event
typedef struct rsi_ble_adv_set_terminated_s {
  /** uint8_t status : Status shows the status on how the Advertising ended
        0 - Advertising successfully ended with a connection being created
       !0 - Advertising ended for another reason and usually error codes would be listed
  */
  uint8_t status;
  /** uint8_t Advertising_Handle : Advertising_Handle in which Advertising has ended, Range : 0x00 to 0xEF */
  uint8_t adv_handle;
  /** uint16_t Connection_Handle : It is the Connection Handle of the connection whose creation ended the advertising, Range : 0x00 to 0xEF */
  uint16_t conn_handle;
  /**
     uint8_t Num_Completed_Extended_Advertising_Events
     Number of completed extended advertising events transmitted by the Controller
  */
  uint8_t num_completed_ae_events;
} rsi_ble_adv_set_terminated_t;

//! ae scan request recvd report event
typedef struct rsi_ble_scan_req_recvd_s {
  /** uint8_t Advertising_Handle : Used to identify an Advertising set , Range : 0x00 to 0xEF */
  uint8_t adv_handle;
  /**
      uint8_t Scanner_Address_Type : Indicates the type of the Address
         0x00 - Public Device Address
         0x01 - Random Device Address
         0x02 - Public Identity Address (corresponds to Resolved Private Address)
         0x03 - Random (static) Identity Address (corresponds to Resolved Private Address)
  */
  uint8_t scanner_addr_type;
  /** uint8[6] Scanner_Address : Address of the Advertising Type */
  uint8_t scanner_addr[DEVICE_ADDR_LEN];
} rsi_ble_scan_req_recvd_t;

/******************************************************
 * *                 Global Variables
 * ******************************************************/
/**
 * @brief Structure representing the BLE buffer statistics.
 *
 * This structure is used to define the parameters for BLE buffer statistics,
 * including the utilization of Asynchronous Connection-Less (ACL) buffers and command packet buffers.
 */
typedef struct chip_ble_buffers_stats_s {
  /** Utilization of ACL (Asynchronous Connection-Less) buffers*/
  uint8_t acl_buffer_utilization;
  /** Utilization of command packet buffers */
  uint8_t cp_buffer_utilization;
} chip_ble_buffers_stats_t;

typedef struct rsi_bt_event_controller_logs_s {
  uint8_t ring_buffer[240];
  uint8_t ring_buffer_len;
} rsi_bt_event_controller_logs_t;

/******************************************************
 * *              GAP API's Declarations
 * ******************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @fn uint8_t rsi_convert_db_to_powindex(int8_t tx_power_in_dBm)
 * @brief Converts the given transmit power in dBm to power index.
 *
 * This function takes a transmit power value in dBm and converts it to a power index value.
 * The power index is used to set the transmit power level in the BLE module.
 *
 * @param tx_power_in_dBm The transmit power in dBm.
 * @return The power index corresponding to the given transmit power.
 */
uint8_t rsi_convert_db_to_powindex(int8_t tx_power_in_dBm);

/** @addtogroup BT-LOW-ENERGY1
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_random_address(void)
 * @brief      Request the local device to set a random address. This is a blocking API.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @return The following values are returned:
 *     *             - 0 - Success 
 *             Non-Zero Value - Failure 
 *             If the return value is less than 0 
 *             -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_set_random_address(void);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_random_address_with_value(uint8_t *random_addr)
 * @brief      Request the local device to set a given random address. This is a blocking API.
 * @pre Pre-condition:
 *      - Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  random_addr - random address of the device to be set
 * @return The following values are returned:
 *           *             - 0 - Success 
 *           Non-Zero Value - Failure 
 *           If the return value is less than 0 
 *           -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_set_random_address_with_value(uint8_t *random_addr);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_advertising(void)
 * @brief      Request the local device to start advertising. This is a blocking API.
 *             A received event \ref rsi_ble_on_enhance_connect_t/ \ref rsi_ble_on_connect_t indicates remote device given BLE connect command and got connected.
 * @pre Pre-condition:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @return The following values are returned:
 *     *             - 0 - Success 
 *     Non-Zero Value - Failure 
 *     If the return value is less than 0 
 *     -4 - Buffer not available to serve the command 
 *     0x4E0C - Command disallowed 
 *     0x4046 - Invalid Arguments
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */

int32_t rsi_ble_start_advertising(void);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_advertising_with_values(const void *rsi_ble_adv)
 * @brief      Request the local device to start advertising with specified values. This is a blocking API.
 *     A received event \ref rsi_ble_on_enhance_connect_t/ \ref rsi_ble_on_connect_t indicates remote device given BLE connect command and got connected.
 * @pre Pre-condition:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API, this is a blocking API.
 * @param[in]  rsi_ble_adv - This structure pointer holds the information of advertising values. This variable is the pointer of the \ref rsi_ble_req_adv_s structure.
 * @return The following values are returned:
 *     *             - 0 - Success 
 *     Non-Zero Value - Failure 
 *     If the return value is less than 0 
 *     -4 - Buffer not available to serve the command 
 *     0x4E0C - Command disallowed 
 *     0x4046 - Invalid Arguments
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_start_advertising_with_values(const void *rsi_ble_adv);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_encrypt(const uint8_t *key, const uint8_t *data, uint8_t *resp)
 * @brief      Encrypt the plain text data fed by the user using the key provided, it uses the AES-128 bit block cypher a logo to generate encrypted data, refer to Bluetooth Spec 5.4 for further details.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API, this is a blocking API.
 * @param[in]  key - 16 Bytes key for Encryption of data.
 * @param[in]  data - 16 Bytes of Data request to encrypt.
 * @param[out] resp - Encrypted data
 * @return The following values are returned:
 *     *             - 0 - Success 
 * Non-Zero Value - Failure 
 * If the return value is less than 0 
 * -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_encrypt(const uint8_t *key, const uint8_t *data, uint8_t *resp);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_stop_advertising(void)
 * @brief      Stop advertising, this is a Blocking API
 * @pre Pre-conditions:
 *        Call rsi_ble_start_advertising() or rsi_ble_start_advertising_with_values() before calling this API.
 * @return The following values are returned:
 *     *             - 0 - Success 
 *     Non-Zero Value - Failure 
 *     If the return value is less than 0 
 *     -4 - Buffer not available to serve the command 
 *     0x4E0C - Command disallowed 
 *     0x4046 - Invalid Arguments
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_stop_advertising(void);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_advertise_data(const uint8_t *data, uint16_t data_len)
 * @brief      Set the advertising data. This is a blocking API.
 * @pre Pre-condition:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  data - Advertising data.
 * @param[in]  data_len - Total length of advertising data.
 * @return     The following values are returned:
 *     *             - 0 - Success 
 *                     Non-Zero Value - Failure 
 *     If the return value is less than 0 
 *     -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 * @note       The maximum length of advertising data payload is 31 bytes. 
 * @note       The basic format of advertising payload record contains length and data.
 * @note       Refer to " ADVERTISING AND SCAN RESPONSE DATA FORMAT " in vol 3-part c in core spec 5.4 for the data structure information.
 */
int32_t rsi_ble_set_advertise_data(const uint8_t *data, uint16_t data_len);

/*========================================================*/
/**
 * @fn         int32_t rsi_ble_set_scan_response_data(const uint8_t *data, uint16_t data_len);
 * @brief      Request the local device to set the scan response data, this is a Blocking API
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API, this is a Blocking API.
 * @param[in]  data - Data about to be sent
 * @param[in]  data_len - Length of data, which is about to be sent
 * @return The following values are returned:
 *     *             - 0 - Success 
 *     Non-Zero Value - Failure 
 *     If the return value is less than 0 
 *     -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 * @note       Refer to " ADVERTISING AND SCAN RESPONSE DATA FORMAT " in vol 3-part c in core spec 5.4 for the data structure information.
 */
int32_t rsi_ble_set_scan_response_data(const uint8_t *data, uint16_t data_len);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_scanning(void)
 * @brief      Start scanning, this is a Blocking API
 *             A received event \ref rsi_ble_on_adv_report_event_t indicates advertise report of remote device received.
 * @pre Pre-condition:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API, this is a Blocking API.
 * @return The following values are returned:
 *     *             - 0 - Success 
 *     Non-Zero Value - Failure 
 *     If the return value is less than 0 
 *     -4 - Buffer not available to serve the command 
 *     0x4E0C - Command disallowed 
 *     0x4046 - Invalid Arguments
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_start_scanning(void);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_scanning_with_values(void *rsi_ble_scan_params)
 * @brief      Start scanning with values. This is a blocking API. A received event \ref rsi_ble_on_adv_report_event_t indicates the advertise report of a remote device received.
 * @pre Pre-condition:
 *      - Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  rsi_ble_scan_params - BLE scan parameters structure. This is a variable of type \ref rsi_ble_req_scan_t, 
 *             which contains information about the scanning status, scan type, filter type, own address type, 
 *             scan interval, and scan window. Refer to the \ref rsi_ble_req_scan_s structure for more details.
 * @return The following values are returned:
 *     *             - 0 - Success 
 *     Non-Zero Value - Failure 
 *     0x4E0C - Command disallowed 
 *     0x4046 - Invalid Arguments
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_start_scanning_with_values(void *rsi_ble_scan_params);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_stop_scanning(void)
 * @brief      Stop scanning. This is a blocking API
 * @pre Pre-conditions:
 *      - \ref rsi_ble_start_scanning() or rsi_ble_start_scanning_with_values() API needs to be called before this API.
 * @return The following values are returned:
 *     *             - 0 - Success 
 *     Non-Zero Value - Failure 
 *     If the return value is less than 0 
 *     -4 - Buffer not available to serve the command
 *     0x4E0C - Command disallowed
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_stop_scanning(void);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_connect_with_params(uint8_t remote_dev_addr_type,
 *                                              const int8_t *remote_dev_addr,
 *                                              uint16_t scan_interval,
 *                                              uint16_t scan_window,
 *                                              uint16_t conn_interval_max,
 *                                              uint16_t conn_interval_min,
 *                                              uint16_t conn_latency,
 *                                              uint16_t supervision_tout)
 * @brief      Connect to the remote BLE device with the user configured parameters. This is a blocking API,
 * a received event \ref rsi_ble_on_enhance_connect_t / \ref rsi_ble_on_connect_t indicates that the connection successful and 
 * a received event \ref rsi_ble_on_disconnect_t indicates that connection failures have occurred.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  remote_dev_addr_type - AddressType - Specifies the type of the address mentioned in BD Address 
 *                                  - 0 - Public Address 
 *                                  - 1 - Random Address
 * @param[in]  remote_dev_addr - This parameter describes the device address of remote device
 * @param[in]  scan_interval - LE Scan Interval : N=0xXXXX 
 *                           - It is defined as the time interval from when the Controller started its last LE scan until it 
 *                               begins the subsequent LE scan. 
 *                           - Range: 0x0004 to 0x4000 
 *                           - Time = N * 0.625 msec 
 *                           - Time Range: 2.5 msec to 10 . 24 seconds
 * @param[in]  scan_window - LE Scan Window : N=0xXXXX 
 *                         - Amount of time for the duration of the LE scan. LE_Scan_Window
 *                           must be less than or equal to LE_Scan_Interval 
 *                         - Range: 0x0004 to 0x4000 
 *                         - Time = N * 0.625 msec 
 *                         - Time Range: 2.5 msec to 10 . 24 seconds
 * @param[in]  conn_interval_max - Max Connection Interval : N=0xXXXX 
 *                               - Minimum value for the connection event interval, which must 
 *                                 be greater than or equal to Conn_Interval_Min. 
 *                               - Range: 0x0006 to 0x0C80 
 *                               - Time = N * 1.25 msec 
 *                               - Time Range: 7.5 msec to 4 seconds. 
 *                               - 0x0000 - 0x0005 and 0x0C81 - 0xFFFF - Reserved for future use
 * @param[in]  conn_interval_min - Min Connection Interval : N=0xXXXX 
 *                               - Minimum value for the connection event interval, which must
 *                                 be greater than or equal to Conn_Interval_Max. 
 *                               - Range: 0x0006 to 0x0C80 
 *                               - Time = N * 1.25 msec 
 *                               - Time Range: 7.5 msec to 4 seconds. 
 *                               - 0x0000 - 0x0005 and 0x0C81 - 0xFFFF - Reserved for future use
 * @param[in]  conn_latency - Connection Latency : N = 0xXXXX 
 *                          - Peripheral latency for the connection in number of connection events. 
 *                          - Range: 0x0000 to 0x01F4
 * @param[in]  supervision_tout - Supervision Timeout : N = 0xXXXX 
 *                              - Supervision timeout for the LE Link. 
 *                              - Range: 0x000A to 0x0C80 
 *                              - Time = N * 10 msec 
 *                              - Time Range: 100 msec to 32 seconds 
 *                              - 0x0000 - 0x0009 and 0x0C81 - 0xFFFF - Reserved for future use
 * @return The following values are returned:
 *                              -     0 - Success 
 *                              -     Non-Zero Value - Failure 
 *                              -     0x4E0C - Command disallowed 
 *                              -     0x4046 - Invalid Arguments
 * @note       If a connection can't be established, for example, the remote device has gone out of range, has entered into deep sleep, or is not advertising, 
 *             the stack will try to connect forever. In this case, the application will not get an event related to the connection request. 
 * @note       To recover from this situation, the application can implement a timeout and call rsi_ble_connect_cancel() to cancel the connection request. 
 *             Subsequent calls of this command have to wait for the ongoing command to complete.
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_connect_with_params(uint8_t remote_dev_addr_type,
                                    const int8_t *remote_dev_addr,
                                    uint16_t scan_interval,
                                    uint16_t scan_window,
                                    uint16_t conn_interval_max,
                                    uint16_t conn_interval_min,
                                    uint16_t conn_latency,
                                    uint16_t supervision_tout);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_connect(uint8_t remote_dev_addr_type, const int8_t *remote_dev_addr)
 * @brief      Connect to the remote BLE device. This is a blocking API,
 * a received event \ref rsi_ble_on_enhance_connect_t/ \ref rsi_ble_on_connect_t indicates that the connection successful and 
 * a received event \ref rsi_ble_on_disconnect_t indicates that connection failures have occurred.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  remote_dev_addr_type - This parameter describes the address type of the remote device
 * @param[in]  remote_dev_addr - This parameter describes the device address of the remote device
 * @return The following values are returned:
 * *             - 0 - Success 
 * Non-Zero Value - Failure 
 * 0x4E0C - Command disallowed 
 * 0x4046 - Invalid Arguments
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 * @note If a connection can't be established, for example, the remote device has gone out of range, has entered into deep sleep, or is not advertising, 
 * @note The stack will try to connect forever. In this case, the application will not get an event related to the connection request. 
 * @note To recover from this situation, the application can implement a timeout and call rsi_ble_connect_cancel() to cancel the connection request. 
 * @note Subsequent calls of this command have to wait for the ongoing command to complete.
 */
int32_t rsi_ble_connect(uint8_t remote_dev_addr_type, const int8_t *remote_dev_addr);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_enhance_connect_with_params(void* ble_enhance_conn_params)
 * @brief      Connect to the remote BLE device with the user configured parameters.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  ble_enhance_conn_params - BLE enhance connection parameter structure. Refer to notes for the fields in this structure.
 * @return The following values are returned:
 * *             - 0 - Success 
 * Non-Zero Value - Failure 
 * If the return value is less than 0 
 * -4 - Buffer not available to serve the command 
 * 0x4E0C - Command disallowed 
 * 0x4046 - Invalid Arguments 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 * @note       The following fields are included in the ble_enhance_conn_params parameter structure:
 *               - dev_addr_type - Address type of the device to connect.
 *                 - 0 - Public Address 
 *                 - 1 - Random Address  
 *               - dev_addr - Address of the device to connect.  
 *               - filter_policy - Policy used to determine whether the filter accept list is used.
 *                 - 0 - Filter accept list is not used to determine which advertiser to connect to. 
 *                 - 1 - Filter accept list is used to determine which advertiser to connect to.  
 *               - own_addr_type - Own address type
 *               - le_scan_interval - The time interval from when the Controller started its last LE scan until it begins the subsequent LE scan. 
 *                 - Range: 0x0004 to 0x4000 
 *                 - Time = le_scan_interval * 0.625 msec
 *                 - Time Range: 2.5 msec to 10 . 24 seconds  
 *               - le_scan_window - Amount of time for the duration of the LE scan. This must be less than or equal to le_scan_interval. 
 *                 - Range: 0x0004 to 0x4000 
 *                 - Time = le_scan_window * 0.625 msec 
 *                 - Time Range: 2.5 msec to 10.24 seconds  
 *               - conn_interval_min - Minimum value for the connection event interval. This must be greater than or equal to conn_interval_max. 
 *                 - Range: 0x0006 to 0x0C80 
 *                 - Time = conn_interval_min * 1.25 msec 
 *                 - Time Range: 7.5 msec to 4 seconds. 
 *                 - 0x0000 - 0x0005 and 0x0C81 - 0xFFFF - Reserved for future use  
 *               - conn_interval_max - Maximum value for the connection event interval. This must be greater than or equal to conn_interval_min.
 *                 - Range: 0x0006 to 0x0C80 
 *                 - Time = conn_interval_max * 1.25 msec 
 *                 - Time Range: 7.5 msec to 4 seconds. 
 *                 - 0x0000 - 0x0005 and 0x0C81 - 0xFFFF - Reserved for future use  
 *               - conn_latency - Peripheral latency for the connection in number of connection events. 
 *                 - Range: 0x0000 to 0x01F4  
 *               - supervision_tout - Supervision timeout for the LE Link.
 *                 - Range: 0x000A to 0x0C80 
 *                 - Time = N * 10 msec 
 *                 - Time Range: 100 msec to 32 seconds 
 *                 - 0x0000 - 0x0009 and 0x0C81 - 0xFFFF - Reserved for future use  
 *               - min_ce_length - Minimum length of connection event recommended for this LE connection. 
 *                 - Range: 0x0000 to 0xFFFF 
 *                 - Time = N * 0.625 msec  
 *               - max_ce_length - Maximum length of connection event recommended for this LE connection. 
 *                 - Range: 0x0000 to 0xFFFF 
 *                 - Time = N * 0.625 msec  
 */
int32_t rsi_ble_enhance_connect_with_params(void *ble_enhance_conn_params);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_connect_cancel(const int8_t *remote_dev_address)
 * @brief      Cancel the connection to the remote BLE device. This is a blocking API,
 * A received event \ref rsi_ble_on_disconnect_t indicates disconnect complete.
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - This parameter describes the device address of the remote device
 * @return The following values are returned:
 *                             - 0 - Success 
 *                             - Non-Zero Value - Failure 
 *                             - 0x4E0C - Command disallowed 
 *                             - 0x4046 - Invalid Arguments 
 *                             - 0x4E02 - Unknown Connection Identifier 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_connect_cancel(const int8_t *remote_dev_address);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_disconnect(const int8_t *remote_dev_address)
 * @brief      Disconnect with the remote BLE device. This is a blocking API.
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - This parameter describes the device address of the remote device
 * @return The following values are returned:
 * 0 - Success 
 * Non-Zero Value - Failure 
 * 0x4E0C - Command disallowed 
 * 0x4D05 - BLE socket not available 
 * 0x4E62 - Invalid Parameters 
 * 0x4D04-  BLE not connected 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_disconnect(const int8_t *remote_dev_address);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_device_state(uint8_t *resp)
 * @brief      Get the local device state. This is a blocking API. The state value is filled in "resp".
 * @pre Pre-conditions:
 *        - Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[out] resp - This is an output parameter which consists of local device state. 
 * This is a 1-byte value. The possible states are described below: 
 * BIT(0)    Advertising state 
 * BIT(1)    Scanning state 
 * BIT(2)    Initiating state 
 * BIT(3)    Connected state 
 * BIT(4)    Extended Advertising state 
 * BIT(5)    Extended Scanning state
 * BIT(6)    Extended Initiating state         
 * @return The following values are returned:
 *                                   - 0 - Success 
 *                                   - Non-Zero Value - Failure 
 *                                   - If the return value is less than 0 
 *                                   - -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_get_device_state(uint8_t *resp);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_smp_pairing_cap_data(rsi_ble_set_smp_pairing_capabilty_data_t *smp_pair_cap_data)
 * @brief      Set the SMP Pairing Capability of local device. This is a blocking API.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  smp_pair_cap_data - This structure pointer holds the information of the SMP capability data values 
 * Refer to \ref rsi_ble_set_smp_pairing_capabilty_data structure for more details.
 * @return The following values are returned:
 * - 0 - Success 
 * - Non-Zero Value - Failure 
 * - If the return value is less than 0 
 * - -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_set_smp_pairing_cap_data(rsi_ble_set_smp_pairing_capabilty_data_t *smp_pair_cap_data);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_local_irk_value(const uint8_t *l_irk)
 * @brief      Set the IRK value to the local device. This is a blocking API.
 * @pre Pre-conditions:
 *        - Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  l_irk -  l_irk  Pointer to local_irk
 * @return The following values are returned:
 * - 0 - Success 
 * - Non-Zero Value - Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_set_local_irk_value(const uint8_t *l_irk);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_conn_param_resp(const uint8_t *remote_dev_address, uint8_t status)
 * @brief      Give the response for the remote device connection parameter request. This is a blocking API.
 * A received event \ref rsi_ble_on_conn_update_complete_t indicates connection update procedure is successful.
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 *             rsi_ble_on_remote_conn_params_request_event should be called by the application.
 * @param[in]  remote_dev_address 	- remote device address
 * @param[in]  status 			- accept or reject the connection parameters update request 
 *                          -	0 - ACCEPT, 
 *                          - 1 - REJECT 
 * @return The following values are returned:
 * - 0 - Success 
 * - Non-Zero Value - Failure
 * - 0x4E0C - Command disallowed 
 * - 0x4046 - Invalid Arguments 
 * - 0x4E02 - Unknown Connection Identifier
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_conn_param_resp(const uint8_t *remote_dev_address, uint8_t status);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_smp_pair_request(uint8_t *remote_dev_address, uint8_t io_capability, uint8_t mitm_req)
 * @brief      Request the SMP pairing process with the remote device. This is a blocking API. 
 * - A received event \ref rsi_ble_on_smp_request_t indicated remote device is given Security Request and need to respond back with \ref rsi_ble_smp_pair_request 
 * - A received event \ref rsi_ble_on_smp_response_t indicated remote device is given SMP Pair Request and need to respond back with \ref rsi_ble_smp_pair_response 
 * - A received event \ref rsi_ble_on_smp_failed_t indicated SMP procedure have failed
 * @pre Pre-conditions:
 *        - \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address -  This is the remote device address
 * @param[in]  io_capability - This is the device input output capability 
 * - 0x00 - Display Only 
 * - 0x01 - Display Yes/No 
 * - 0x02 - Keyboard Only 
 * - 0x03 - No Input No Output
 * @param[in]  mitm_req - MITM enable/disable 
 *                      - 0 - Disable 
 *                      - 1 - Enable
 * @return The following values are returned:
 * - 0 - Success 
 * - Non-Zero Value - Failure 
 * - If the return value is less than 0 
 * - -4 - Buffer not available to serve the command 
 * - 0x4D05  BLE socket not available 
 * - 0x4E62 	Invalid Parameters 
 * - 0x4D04	BLE not connected 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_smp_pair_request(uint8_t *remote_dev_address, uint8_t io_capability, uint8_t mitm_req);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_smp_pair_failed(uint8_t *remote_dev_address, uint8_t reason)
 * @brief      Send SMP pairing failure reason to the remote device.
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address -  This is the remote device address
 * @param[in]  reason - This is the reason for SMP Pairing Failure 
 * - 0x05 - Pairing Not Supported 
 * - 0x08 - Unspecified Reason 
 * - 0x09 - Repeated Attempts 
 * @return The following values are returned:
 * - 0 - Success 
 * - Non-Zero Value - Failure 
 * - If the return value is less than 0 
 * - -4 - Buffer not available to serve the command
 */
int32_t rsi_ble_smp_pair_failed(uint8_t *remote_dev_address, uint8_t reason);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_ltk_req_reply(uint8_t *remote_dev_address,
 *                                           uint8_t reply_type, const uint8_t *ltk)
 * @brief      Send the local long term key of its associated local EDIV and local Rand. This is a blocking API. 
 * - A received event \ref rsi_ble_on_encrypt_started_t indicated encrypted event is received from module 
 * - A received event \ref rsi_ble_on_smp_failed_t indicated SMP procedure have failed
 * @param[in]  remote_dev_address - remote device address
 * @param[in]  reply_type  - 0 - Negative reply 
 *                         - BIT(0) - Positive Reply (Encryption Enabled)
 *                         - BIT(1) - Un authenticated LTK or STK-based Encryption Enabled 
 *                         - BIT(2) - Authenticated LTK or STK-based Encryption Enabled 
 *                         - BIT(3) - Authenticated LTK with LE Secure Connections based Encryption Enabled 
 *                         - BIT(4) to BIT(6) - Reserved for Future use 
 *                         - BIT(7) - LE Secure Connection Enabled 
 * @param[in]  ltk - Long Term Key 16 bytes
 * @return The following values are returned:
 * - 0 - Success 
 * - Non-Zero Value - Failure 
 * - If the return value is less than 0 
 * - -4 - Buffer not available to serve the command 
 * - 0x4D05  BLE socket not available 
 * - 0x4E62 	Invalid Parameters 
 * - 0x4D04	BLE not connected 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_ltk_req_reply(uint8_t *remote_dev_address, uint8_t reply_type, const uint8_t *ltk);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_smp_pair_response(uint8_t *remote_dev_address, uint8_t io_capability, uint8_t mitm_req)
 * @brief      Send SMP pairing response during the process of pairing with the remote device. This is a blocking API.
 * - A received event \ref rsi_ble_on_smp_passkey_t indicated Legacy SMP passkey is received and need to respond back with \ref rsi_ble_smp_passkey() 
 * - A received event \ref rsi_ble_on_sc_passkey_t indicated BLE SC passkey is received and need to respond back with \ref rsi_ble_smp_passkey() 
 * - A received event \ref rsi_ble_on_smp_passkey_display_t indicates SMP passkey display is received from the module 
 * - A received event \ref rsi_ble_on_smp_failed_t indicated SMP Failed event is received
 * @pre Pre-condition:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address -  This is the remote device address
 * @param[in]  io_capability - This is the device input output capability 
 * 0x00 - Display Only 
 * 0x01 - Display Yes/No 
 * 0x02 - Keyboard Only 
 * 0x03 - No Input No Output
 * @param[in]  mitm_req -  MITM Request information 
 *                      - 0 - Disable 
 *                      - 1 - Enable
 * @return The following values are returned:
 *  - 0 - Success 
 *  - Non-Zero Value - Failure 
 *  - If the return value is less than 0 
 *  - -4 - Buffer not available to serve the command 
 *  - 0x4D05  BLE socket not available 
 *  - 0x4E62 	Invalid Parameters 
 *  - 0x4D04	BLE not connected 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_smp_pair_response(uint8_t *remote_dev_address, uint8_t io_capability, uint8_t mitm_req);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_smp_passkey(uint8_t *remote_dev_address, uint32_t passkey)
 * @brief      Send SMP passkey during SMP pairing process with the remote device. This is a Blocking API 
 * A received event \ref rsi_ble_on_encrypt_started_t indicated encrypted event is received from module 
 * A received event \ref rsi_ble_on_le_security_keys_t indicates exchange of security keys completed after encryption 
 * A received event \ref rsi_ble_on_smp_failed_t indicated SMP procedure have failed
 * @pre Pre-conditions:
 *        Call \ref rsi_ble_smp_pair_request and \ref rsi_ble_smp_pair_response
 *             before calling this API.
 * @param[in]  remote_dev_address -  This is the remote device address
 * @param[in]  passkey - This is the key required in pairing process
 * @return The following values are returned:
 * -  0 - Success 
 * - Non-Zero Value - Failure 
 * - If the return value is less than 0 
 * - -4 - Buffer not available to serve the command 
 * - 0x4D05  BLE socket not available 
 * - 0x4E62 	Invalid Parameters 
 * - 0x4D04	BLE not connected 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_smp_passkey(uint8_t *remote_dev_address, uint32_t passkey);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_le_ping_timeout(uint8_t *remote_dev_address, uint16_t *time_out)
 * @brief      Get the timeout value of the LE ping. This is a blocking API.
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - This is the remote device address
 * @param[out] time_out - This a response parameter which holds timeout value for 
 *                        authentication payload command.
 * @return The following values are returned:
 * - 0 - Success 
 * - Non-Zero Value - Failure 
 * - If the return value is less than 0 
 * - -4 - Buffer not available to serve the command 
 * - 0x4D05  BLE socket not available 
 * - 0x4E62 	Invalid Parameters 
 * - 0x4D04	BLE not connected 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 * @note       Currently Get ping is not supported.
 */
int32_t rsi_ble_get_le_ping_timeout(uint8_t *remote_dev_address, uint16_t *time_out);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_le_ping_timeout(uint8_t *remote_dev_address, uint16_t time_out)
 * @brief      Set the timeout value of the LE ping. This is a blocking API.
 * A received event of \ref rsi_ble_on_le_ping_payload_timeout_t indicates LE ping payload timeout expired
 * @pre Pre-conditions:
 *       - \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address -  This is the remote device address
 * @param[out] time_out - This input parameter sets timeout value for authentication 
 *                       payload command.(in milliseconds)
 * @return The following values are returned:
 * - 0 - Success 
 * - Non-Zero Value - Failure 
 * - If the return value is less than 0 
 * - -4 - Buffer not available to serve the command 
 * - 0x4D05  BLE socket not available 
 * - 0x4E62 	Invalid Parameters 
 * - 0x4D04	BLE not connected 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_set_le_ping_timeout(uint8_t *remote_dev_address, uint16_t time_out);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_clear_acceptlist(void)
 * @brief      Clear all the BD address present in accept list. This is a blocking API.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @return The following values are returned:
 * - 0 - Success 
 * - Non-Zero Value - Failure 
 * - If the return value is less than 0 
 * - -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_clear_acceptlist(void);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_addto_acceptlist(const int8_t *dev_address, uint8_t dev_addr_type)
 * @brief      Add BD address to accept list. This is a blocking API.
 * @pre Pre-conditions:
 *        - Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  dev_address - Address of the device which is going to add in accept list
 * @param[in]  dev_addr_type - address type of BD address
 * @return The following values are returned:
 * - 0 - Success 
 * - Non-Zero Value - Failure 
 * - If the return value is less than 0 
 * - -4 - Buffer not available to serve the command
 * @note       Maximum number of device address that firmware can store is 10. 
 * Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_addto_acceptlist(const int8_t *dev_address, uint8_t dev_addr_type);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_deletefrom_acceptlist(const int8_t *dev_address, uint8_t dev_addr_type)
 * @brief      Delete particular BD address from accept list. This is a blocking API.
 * @pre Pre-conditions:
 *        - \ref rsi_ble_addto_acceptlist() API needs to be called before this API.
 * @param[in]  dev_address - Address of the device which is going to delete from accept list
 * @param[in]  dev_addr_type - address type of BD address
 * @return The following values are returned:
 * - 0 - Success 
 * - Non-Zero Value - Failure 
 * - If the return value is less than 0 
 * - -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_deletefrom_acceptlist(const int8_t *dev_address, uint8_t dev_addr_type);
/*==============================================*/
/**
 * @fn        int32_t rsi_ble_resolvlist(uint8_t process_type,
 *                                      uint8_t remote_dev_addr_type,
 *                                      uint8_t *remote_dev_address,
 *                                      const uint8_t *peer_irk,
 *                                      const uint8_t *local_irk)
 * @brief     Resolvlist API used for multiple purposes based on the process type. It will be used to add/remove/clear a device to/from the list. This is a blocking API.
 * @pre Pre-conditions:
 *       Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in] process_type - Indicates which type of process this is, as follows: 
 *                          1 - add a device to the resolve list 
 *                          2 - remove a device from the resolve list 
 *                          3 - clear the entire resolve list
 * @param[in] remote_dev_addr_type 	- typr of the remote device address
 * @param[in] remote_dev_address 	- remote device address 
 *				0 - Public identity address 
 * 				1 - Random (static) identity address 
 * @param[in] peer_irk 			- 16-byte IRK of the peer device
 * @param[in] local_irk 		- 16-byte IRK of the local device
 * @return The following values are returned:
 *             - 0 - Success 
 *             - Non-Zero Value - Failure 
 *             - If the return value is less than 0 
 *             - -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_resolvlist(uint8_t process_type,
                           uint8_t remote_dev_addr_type,
                           uint8_t *remote_dev_address,
                           const uint8_t *peer_irk,
                           const uint8_t *local_irk);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_resolving_list_size(uint8_t *resp)
 * @brief      Request to get resolving list size. This is a blocking API.
 * @pre Pre-conditions:
 *       - Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[out] resp - output parameter which consists of supported resolving the list size.
 * @return The following values are returned:
 *            - 0 - Success 
 *            - Non-Zero Value - Failure 
 *            - If the return value is less than 0 
 *            - -4 : Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_get_resolving_list_size(uint8_t *resp);
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_addr_resolution_enable(uint8_t enable, uint16_t tout)
 * @brief      Request to enable address resolution, and to set resolvable private address timeout. This is a blocking API.
 * @pre Pre-conditions:
 *        - Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  enable - value to enable/disable address resolution 
 *		- 1 - enables address resolution 
 *    - 0 - disables address resolution
 * @param[in]  tout -  the period for changing address of our local device in seconds 
 *		Value ranges from 0x0001 to 0xA1B8 (1 s to approximately 11.5 hours)
 * @return The following values are returned:
 *             - 0 - Success 
 *             - Non-Zero Value - Failure 
 *             - If the return value is less than 0 
 *             - -4 : Buffer not available to serve the command
 */
int32_t rsi_ble_set_addr_resolution_enable(uint8_t enable, uint16_t tout);
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_privacy_mode(uint8_t remote_dev_addr_type,
 *                                              uint8_t *remote_dev_address, uint8_t privacy_mode)
 * @brief      Request to set privacy mode for particular device, this is a Blocking API
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  remote_dev_addr_type - type of the remote device address 
 *                                    0 - Public Identity Address 
 *                                    1 - Random (static) Identity Address
 * @param[in]  remote_dev_address   - remote device address
 * @param[in]  privacy_mode 	    - type of the privacy mode 
 *				0 - Network privacy mode 
 *        1 - Device privacy mode
 * @return The following values are returned:
 *             0 - Success 
 *             Non-Zero Value - Failure 
 *             If the return value is less than 0 
 *             -4 : Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_set_privacy_mode(uint8_t remote_dev_addr_type, uint8_t *remote_dev_address, uint8_t privacy_mode);
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_readphy(const int8_t *remote_dev_address, rsi_ble_resp_read_phy_t *resp)
 * @brief      Reads the TX and RX PHY rates of the Connection. This is a blocking API.
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - remote device address
 * @param[out] resp - pointer to store the response
	 Refer to \ref rsi_ble_resp_read_phy_s structure for more details.
 * @return The following values are returned:
 *             0 - Success 
 *             Non-Zero Value - Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_readphy(const int8_t *remote_dev_address, rsi_ble_resp_read_phy_t *resp);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_setphy(const int8_t *remote_dev_address, uint8_t tx_phy, uint8_t rx_phy,
 *                                    uint16_t coded_phy)
 * @brief      Set TX and RX PHY. This is a blocking API.
 *             A received event \ref rsi_ble_on_phy_update_complete_t indicates PHY rate update complete.
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - remote device address 
 * @param[in]  tx_phy - transmit PHY rate 
 *		BIT(0) - Host prefers to use the LE 1M transmitter PHY (possibly among others)
 *                      - BIT(1) - Host prefers to use the LE 2M transmitter PHY (possibly among others) 
 *                      - BIT(2) - Host prefers to use the LE Coded transmitter PHY (possibly among others) 
 *                      - BIT(3) - BIT(7) Reserved for future use 
 * @param[in]  rx_phy - receive PHY rate 
 *		BIT(0) - Host prefers to use the LE 1M receiver PHY (possibly among others) 
 *                      - BIT(1) - Host prefers to use the LE 2M receiver PHY (possibly among others) 
 *                      - BIT(2) - Host prefers to use the LE Coded receiver PHY (possibly among others) 
 *                      - BIT(3) - BIT(7) Reserved for future use 
 * @param[in]  coded_phy - TX/RX coded PHY rate 
 *		                     - 0 = Host has no preferred coding when transmitting on the LE Coded PHY 
 *                         - 1 = Host prefers that S=2 coding be used when transmitting on the LE Coded PHY 
 *                         - 2 = Host prefers that S=8 coding be used when transmitting on the LE Coded PHY 
 *                         - 3 = Reserved for future use 
 * @return The following values are returned:
 *              0   -  Success 
 *              Non-Zero Value - Failure 
 *              0x4D05  BLE socket not available 
 *              0x4E62 	Invalid Parameters 
 *              0x4D04	BLE not connected 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_setphy(const int8_t *remote_dev_address, uint8_t tx_phy, uint8_t rx_phy, uint16_t coded_phy);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_conn_params_update(const uint8_t *remote_dev_address,
 *                                             uint16_t min_int,
 *                                             uint16_t max_int,
 *                                             uint16_t latency,
 *                                             uint16_t timeout)
 * @brief      	Requests the connection parameters change with the remote device,
 *				      When the Silicon Labs device acts as a central, this API is used to update the connection parameters.
 * 				      When the Silicon Labs device acts as a peripheral, this API is used to request the central to initiate the connection update procedure. This is a blocking API.
 *              A received event \ref rsi_ble_on_conn_update_complete_t indicates connection parameters update complete.
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address 	- remote device address
 * @param[in]  min_int 			- Minimum value for the connection interval. 
 *                       		  This value shall be less than or equal to max_int.
 *                       		  The value is represented in units of 1.25 ms.
 *                       		  Actual Time = min_int * 1.25 ms.
 *                       		  Range: 6 to 3200 (7.5 ms to 4 s).
 * @param[in]  max_int 			- Maximum value for the connection interval. 
 *                       		  This value shall be greater than or equal to min_int.
 *                       		  The value is represented in units of 1.25 ms.
 *                       		  Actual Time = max_int * 1.25 ms.
 *                       		  Range: 6 to 3200 (7.5 ms to 4 s).
 * @param[in]  latency 			- Peripheral latency for the connection in number of connection events.
 *                       		  Range: 0 to 499.
 *                       		  When the device is in the peripheral role, if the latency value is greater than 32,
 *                       		  the peripheral latency is limited to a maximum value of 32.
 * @param[in]  timeout 			- supervision timeout for the LE Link. 
 *                       		  Ranges from 10 to 3200 (Time = N * 10 ms, Time Range: 100 ms to 32 s)
 * @return The following values are returned:
 *              - 0   -  Success 
 *              - Non-Zero Value - Failure 
 *              - 0x4D05  BLE socket not available 
 *              - 0x4E62 	Invalid Parameters 
 *              - 0x4D04	BLE not connected 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 * @note       min_int and max_int values ranges from 6 to 3200 (Time = N * 1.25 ms, Time Range: 7.5 ms to 4 s)
		latency : If latency value is greater than 32 ,Limiting the peripheral latency value to 32
		Max supported peripheral latency is 32 when Device is in peripheral Role.
 *
 */
int32_t rsi_ble_conn_params_update(const uint8_t *remote_dev_address,
                                   uint16_t min_int,
                                   uint16_t max_int,
                                   uint16_t latency,
                                   uint16_t timeout);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_data_len(uint8_t *remote_dev_address, uint16_t tx_octets, uint16_t tx_time)
 * @brief      Sets the TX octets and the TX time of specified link (remote device connection), this is a Blocking API. 
 *             A received event \ref rsi_ble_on_data_length_update_t indicates data length update complete.
 * @pre Pre-conditions:
 *        - \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address 	- remote device device 
 * @param[in]  tx_octets 		- preferred maximum number of payload octets that the local Controller 
 *                         		  should include in a single Link Layer packet on this connection.
 * @param[in]  tx_time 			- preferred maximum number of microseconds that the local Controller 
 *                       		  should use to transmit a single Link Layer packet on this connection.
 * @return The following values are returned:
 *              - 0 - LE_Set_Data_Length command succeeded. 
 *              - Non-Zero Value - Failure 
 *              - 0x4D05  BLE socket not available 
 *              - 0x4E62 	Invalid Parameters 
 *              - 0x4D04	BLE not connected 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_set_data_len(uint8_t *remote_dev_address, uint16_t tx_octets, uint16_t tx_time);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_read_max_data_len(rsi_ble_read_max_data_length_t *blereaddatalen)
 * @brief      reads the maximum supported values of TX octets, TX time, RX octets and Rx time. This is a blocking API.
 * @pre Pre-conditions:
 *        - \ref rsi_ble_connect() API needs to be called before this API.
 * @param[out]  blereaddatalen - pointer to structure variable,
		 Refer to rsi_ble_resp_read_max_data_length_s structure for more details.
 * @return The following values are returned:
 *             - 0 - command success 
 *             - Non-Zero Value - Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_read_max_data_len(rsi_ble_read_max_data_length_t *blereaddatalen);
/** @} */

/** @addtogroup BT-LOW-ENERGY6
* @{
*/

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_rx_test_mode(uint8_t rx_channel, uint8_t phy, uint8_t modulation)
 * @brief      Start the BLE RX test mode in controller. This is a blocking API.
 * @param[in]  rx_channel - Channel in which packet have to be received (0 - 39)
 * @param[in]  phy - 0x00  Reserved for future use 
 *                   0x01  Receiver set to use the LE 1M PHY 
 *                   0x02  Receiver set to use the LE 2M PHY 
 *                   0x03  Receiver set to use the LE Coded PHY 
 *                   (0x04 - 0xFF)   Reserved for future use.
 * @param[in]  modulation - 0x00  Assume transmitter would have a standard standard modulation index 
 *                          0x01  Assume transmitter would have a stable modulation index 
 *                          (0x02 - 0xFF)  Reserved for future use
 * @return The following values are returned:
 *             - 0 - Success 
 *             - Non-Zero Value - Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_rx_test_mode(uint8_t rx_channel, uint8_t phy, uint8_t modulation);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_tx_test_mode(uint8_t tx_channel, uint8_t phy,
 *                                          uint8_t tx_len, uint8_t mode)
 * @brief      Start the BLE TX test mode in controller. This is a blocking API.
 * @param[in]  tx_channel -  RF Channel (0-39). 
 * @param[in]  phy - 0x00  Reserved for future use 
 *                  - 0x01  Transmitter set to use the LE 1M PHY 
 *                  - 0x02  Transmitter set to use the LE 2M PHY 
 *                  - 0x03  Transmitter set to use the LE Coded PHY with S = 8 data coding 
 *                  - 0x04  Transmitter set to use the LE Coded PHY with S = 2 data coding 
 *                  - (0x05 - 0xFF)  Reserved for future use.
 * @param[in] tx_len - Length in bytes of payload data in each packet ( 1 - 251 bytes).
 * @param[in] mode - 0x00  PRBS9 sequence '11111111100000111101...' 
 *                 - 0x01  Repeated '11110000' 
 *                 - 0x02  Repeated '10101010' 
 *                 - 0x03  PRBS15 
 *                 - 0x04  Repeated '11111111' 
 *                 - 0x05  Repeated '00000000' 
 *		             - 0x06  Repeated '00001111' 
 *		             - 0x07  Repeated '01010101' 
 *                 - 0x08 - 0xFF Reserved for future use 
 * @return The following values are returned:
 *               - 0 - Success 
 *               - Non-Zero Value - Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_tx_test_mode(uint8_t tx_channel, uint8_t phy, uint8_t tx_len, uint8_t mode);

/*==============================================*/
/**
 * @fn        int32_t rsi_ble_end_test_mode(uint16_t *num_of_pkts)
 * @brief     Stop the BLE TX / RX test mode in controller. This is a blocking API.
 * @param[out] num_of_pkts - Number of RX packets received are displayed when RX test is stopped 
 * @return The following values are returned:
 *             - 0 - Success 
 *             - Non-Zero Value - Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_end_test_mode(uint16_t *num_of_pkts);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_per_transmit(struct rsi_ble_per_transmit_s *rsi_ble_per_tx)
 * @brief      Enables/disables the BLE transmit PER mode in the controller. This is a blocking API.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  rsi_ble_per_tx - This parameter is the buffer to hold the structure values 
 *                              This is a structure variable of type struct \ref rsi_ble_per_transmit_s.
 * @return The following values are returned:
 *             - 0 - Success 
 *             - Non-Zero Value - Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_per_transmit(struct rsi_ble_per_transmit_s *rsi_ble_per_tx);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_per_receive(struct rsi_ble_per_receive_s *rsi_ble_per_rx)
 * @brief      Enables/disables the BLE receive PER mode in the controller. This is a blocking API.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  rsi_ble_per_rx - This parameter is the buffer to hold the structure values. 
 *             This is a structure variable of type struct \ref rsi_ble_per_receive_s
 * @return The following values are returned:
 *             - 0 - Success 
 *             - Non-Zero Value - Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_per_receive(struct rsi_ble_per_receive_s *rsi_ble_per_rx);
/** @} */

/** @addtogroup BT-LOW-ENERGY1
* @{
*/
/**
 * @fn         int32_t rsi_ble_acceptlist_on_type(uint8_t enable,
 *                                                uint8_t ad_type,
 *                                                uint8_t value_length,
 *                                                const uint8_t *value)
 * @brief      Vendor-specific command to set the acceptlist filter based on
 *             advertising data type, length, and value. This is a blocking API.
 * @param[in]  enable - enable/disable
 * @param[in]  ad_type - Advertising data type to filter \n
 *                       Currently only supports: \n
 *                       0x08 - Shortened Local Name \n
 *                       0x09 - Complete Local Name
 * @param[in]  value_length - Length of the value to compare
 * @param[in]  value - Value to compare
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 * @note       This API currently supports filtering only on device name advertising data types (0x08 and 0x09). \n
 *             Other advertising data types are not supported by this vendor-specific command.
 */
int32_t rsi_ble_acceptlist_on_type(uint8_t enable, uint8_t ad_type, uint8_t value_length, const uint8_t *value);
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_accept_list_using_adv_data(uint8_t enable,
 *                                                    uint8_t data_compare_index,
 *                                                    uint8_t len_for_compare_data,
 *                                                    const uint8_t *payload)
 * @brief      Give vendor-specific command to set the acceptlist feature based on
 *             the advertisers advertising payload, this is a Blocking API
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  enable - enable/disable
 * @param[in]  data_compare_index - the starting index of the data to compare
 * @param[in]  len_for_compare_data - total length of data to compare
 * @param[in]  payload - Payload
 * @return The following values are returned:
 *             - 0 - Success 
 *             - Non-Zero Value - Failure 
 *             - If the return value is less than 0 
 *             - -4 - Buffer not available to serve the command
 *             - 0x4E62 	Invalid Parameters 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_accept_list_using_adv_data(uint8_t enable,
                                           uint8_t data_compare_index,
                                           uint8_t len_for_compare_data,
                                           const uint8_t *payload);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_coex_roles_priority(const uint8_t *payload)
 * @brief      Give vendor-specific command to set the priorities for the various coex roles.
 *             The payload contains role id, followed by min, max priorities of the specified role..
 *             This is a Blocking API.
 * @pre        Device should be initialized before calling this API.
 * @param[in]  payload - Payload (There are 7 coex roles refer rsi_ble_coex_role_id_t, each having min and max priority followed by the role id in the role_priority_payload.)
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *               -4 - Buffer not available to serve the command
 *              0x4E62  Invalid Parameters \n
 * @note       Refer to the Status Codes section for the above error codes at [additional-status-codes](../wiseconnect-api-reference-guide-err-codes/sl-additional-status-errors).
 */

/*==============================================*/
int32_t rsi_ble_set_coex_roles_priority(const uint8_t *payload);

/**
 * @fn         void BT_LE_ADPacketExtract(uint8_t *remote_name, const uint8_t *pbuf, uint8_t buf_len);
 * @brief      Used to extract remote Bluetooth device name from the received advertising report.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  remote_name - device name
 * @param[in]  pbuf -  advertise data packet buffer pointer
 * @param[in]  buf_len - buffer length
 * 
 */
void BT_LE_ADPacketExtract(uint8_t *remote_name, const uint8_t *pbuf, uint8_t buf_len);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_encryption(uint8_t *remote_dev_address, uint16_t ediv,
 *                                              const uint8_t *rand, const uint8_t *ltk)
 * @brief      Start the encryption process with the remote device. This is a blocking API.
 *             A received event \ref rsi_ble_on_encrypt_started_t indicated encrypted event is received from module.
 *             A received event \ref rsi_ble_on_le_security_keys_t indicates exchange of security keys completed after encryption.
 *             A received event \ref rsi_ble_on_smp_failed_t indicated SMP procedure have failed.
 * @pre Pre-conditions:
 *       - Encryption enabled event should come before calling this API for second time SMP connection.
 * @param[in]  remote_dev_address - Remote BD address in string format
 * @param[in]  ediv - remote device ediv value.
 * @param[in]  rand - remote device rand value.
 * @param[in]  ltk  - remote device ltk value.
 * @return The following values are returned:
 *             - 0 - Success 
 *             - Non-Zero Value - Failure 
 *             - If the return value is less than 0 
 *             - -4 - Buffer not available to serve the command
 *             - 0x4D05  BLE socket not available 
 *             - 0x4E62 	Invalid Parameters 
 *             - 0x4D04	BLE not connected 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_start_encryption(uint8_t *remote_dev_address, uint16_t ediv, const uint8_t *rand, const uint8_t *ltk);

/** @} */

/** @addtogroup BT-BLE
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_ble_set_ble_tx_power(int8_t tx_power)
 * @brief       Set TX power
 * @param[in]   tx_power Power value
 * @return      The following values are returned:
 *                - 0 - Success 
 *                - Non-zero value - Failure
 *                - 0x4E02 - Unknown connection identifier
 *                - 0x4E01	- Unknown HCI command
 *                - 0x4E0C	- Command disallowed 
 *                - 0x4046 - Invalid arguments
 *                - 0x4D04	- BLE not connected 
 *                - 0x4D14	- BLE parameter out of mandatory range
 * @note        This is a Blocking API. 
 * @note        Refer to the Status Codes section for the above error codes at 
 *              [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 * @note        The higher power will be backed off based on country region.
 * @note        This API currently supports controlling the TX power in the configured chain during wireless initialization. However, changing the chain from LP to HP or vice versa is not supported.
 * @note        Use the following setting to indicate tx_power as an index: 
 *              `#define RSI_BLE_PWR_INX 30`
 *              Default value for power index is 31. 
 *              Valid values for power index range from 1 to 31 and 33 to 127:
 *                - 1 to 31  : BLE - 0 dBm mode.  
 *                - 33 to 63 : BLE - 10 dBm mode. 
 *                - 64 to 82 : BLE - 1 dBm - 18 dBm HP mode in the resolution of 1 dBm.
 *                - 104 to 126: BLE - 0.5 dBm - 11 dBm HP mode in the resolution of 0.5 dBm.
 *                - 127      : BLE HP Mode, maximum power supported.

 * @note        Use the following setting to indicate tx_power in dBm (-8 to 18 dBm): `#define RSI_BLE_PWR_INX_DBM  1`
 *   
 * @note        For the LP Chain - Power index vs output power in dBm in E2E mode:
 *   |       Power Index   |                  Output Power in dBm                              |
 *   ----------------------|-------------------------------------------------------------------|
 *   |      1              |      		 -22.3054959      			                                 |
 *   |      2              |      		 -16.59332574     			                                 |
 *   |      3              |      		 -13.38278365     			                                 |
 *   |      4              |      		 -11.19804718                                            |
 *   |      5              |      		 -9.576522466                                            |
 *   |      6              |      		 -8.312070432                                            |
 *   |      7              |      		 -7.294640362                                            |
 *   |      8              |      		 -6.458448154                                            |
 *   |      9              |      		 -5.760963318                                            |
 *   |      10             |      		 -5.173042366                                            |
 *   |      11             |      		 -4.673788189                                            |
 *   |      12             |      		 -4.247653993                                            |
 *   |      13             |      		 -3.882708784                                            |
 *   |      14             |      		 -3.569545894                                            |
 *   |      15             |      		 -3.300567503                                            |
 *   |      16             |      		 -3.069499167                                            |
 *   |      17             |      		 -2.871050592                                            |
 *   |      18             |      		 -2.700672503                                            |
 *   |      19             |      		 -2.554378603                                            |
 *   |      20             |      		 -2.428612817                                            |
 *   |      21             |      		 -2.32014891                                             |
 *   |      22             |      		 -2.226013876                                            |
 *   |      23             |      		 -2.143429275                                            |
 *   |      24             |      		 -2.069766557                                            |
 *   |      25             |      		 -2.002513642                                            |
 *   |      26             |      		 -1.939250859                                            |
 *   |      27             |      		 -1.87763493                                             |
 *   |      28             |      		 -1.815390046                                            |
 *   |      29             |      		 -1.750305305                                            |
 *   |      30             |      		 -1.680237892                                            |
 *   |      31             |      		 -1.603121401                                            |
 *   |      32             |            N/A		  		                                           |
 *   |      33             |           -10.4822997	  		                                     |
 *   |      34             |           -4.9531679	  		                                       |
 *   |      35             |           -1.931961022	  		                                     |
 *   |      36             |            0.057132993	  		                                     |
 *   |      37             |            1.476764101	  		                                     |
 *   |      38             |            2.5332116	  		                                       |
 *   |      39             |            3.336771823	  		                                     |
 *   |      40             |            3.953605265	  		                                     |
 *   |      41             |            4.426779615	  		                                     |
 *   |      42             |            4.786171523	  		                                     |
 *   |      43             |            5.053647759	  		                                     |
 *   |      44             |            5.246007208	  		                                     |
 *   |      45             |            5.37676618	  		                                     |
 *   |      46             |            5.457304255	  		                                     |
 *   |      47             |            5.497635316	  		                                     |
 *   |      48             |            5.506945838	  		                                     |
 *   |      49             |            5.493978354	  		                                     |
 *   |      50             |            5.467302132	  		                                     |
 *   |      51             |            5.435491631	  		                                     |
 *   |      52             |            5.407220119	  		                                     |
 *   |      53             |            5.391268248	  		                                     |
 *   |      54             |            5.396444507	  		                                     |
 *   |      55             |            5.431416481	  		                                     |
 *   |      56             |            5.504458826	  		                                     |
 *   |      57             |            5.62313521	  		                                     |
 *   |      58             |            5.793945208	  		                                     |
 *   |      59             |            6.02197959	  		                                     |
 *   |      60             |            6.310634089	  		                                     |
 *   |      61             |            6.661428559	  		                                     |
 *   |      62             |            7.073964236	  		                                     |
 *   |      63             |            7.546029076	  		                                     |
 * @note For the HP Chain - Power index vs output power in dBm in E2E mode:                    
 *   |       Power Index   |                  Output Power in dBm                              |
 *   |---------------------|-------------------------------------------------------------------|
 *   |      64	           |      		  1		  			                                           |
 *   |      65	           |      		  2	  				                                           |
 *   |      66	           |      		  3	  				                                           |
 *   |      67	           |      		  4	       	                	                           |
 *   |      68	           |      		  5	       				                                       |
 *   |      69	           |      		  6	               	                                     | 
 *   |      70	           |      		  7	               	                                     |
 *   |      71	           |      		  8	               	                                     |
 *   |      72	           |            9	               	                                     |
 *   |      73	           |      	    10	               	                                   |
 *   |      74	           |      		  11	               	                                   |
 *   |      75	           |      		  12	               	                                   |
 *   |      76	           |      		  13	               	                                   |
 *   |      77	           |      		  14	               	                                   |
 *   |      78	           |      		  15	               	                                   |
 *   |      -	             |      		  -	               	                                     |
 *   |      -	             |      		  -	               	                                     |
 *   |      104	           |      		  0.5	               	                                   |
 *   |      105	           |      		  1	               	                                     |
 *   |      106	           |      		  1.5	               	                                   |
 *   |      107	           |      		  2	               	                                     |
 *   |      108	           |      		  2.5	               	                                   |
 *   |      109	           |      		  3	               	                                     |
 *   |      110	           |      		  3.5	               	                                   |
 *   |      111	           |      		  4	               	                                     |
 *   |      -	             |      		  -	                                                     |
 *   |      126	           |      		  10.5	                                                 |
 *   |      127	           | Max Power Supported by Country region			                       |
 */
int32_t rsi_ble_set_ble_tx_power(int8_t tx_power);

/** @} */

/******************************************************
 * *        GATT Client API's Declarations
 * ******************************************************/

/** @addtogroup BT-LOW-ENERGY3
* @{
*/

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_profiles(uint8_t *dev_addr,
 *                                          uint16_t start_handle,
 *                                          uint16_t end_handle,
 *                                          rsi_ble_resp_profiles_list_t *p_prof_list)
 * @brief      This API initiates a command to discover the GATT profiles or services supported by a remote BLE device.
 *
 *             The discovery results are provided through callback functions:
 *               - \ref rsi_ble_on_profiles_list_resp_t    - Invoked upon successful retrieval of the profile list.
 *               - \ref rsi_ble_on_gatt_error_resp_t       - Invoked when an error response is received.
 *
 * @note       This is a non-blocking API, still you need to wait until the callback \ref rsi_ble_on_profiles_list_resp_t is received from the device,
 *             to initiate further attribute related transactions on this remote device address.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr      remote device address
 * @param[in]  start_handle  start handle (index) of the remote device's service records
 * @param[in]  end_handle    end handle (index) of the remote device's service records
 * @param[out] p_prof_list   NULL for p_prof_list because the profiles/services information will be provided through the callback event.
 *                           Refer to \ref rsi_ble_resp_profiles_list_s structure for more details.
 *
 * @note       p_prof_list structure should be set to NULL.
 *
 * @return     The following values are returned:
 *               - Zero Value             - Success
 *               - Non-Zero Value         - Failure
 *               - 0x4E62                 - Invalid Parameters
 *               - 0x4D04                 - BLE not connected
 *               - 0x4D05                 - BLE Socket not available
 *
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_get_profiles(uint8_t *dev_addr,
                             uint16_t start_handle,
                             uint16_t end_handle,
                             rsi_ble_resp_profiles_list_t *p_prof_list);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_profile(uint8_t *dev_addr, uuid_t profile_uuid,
 *                                         profile_descriptors_t *p_profile)
 * @brief      This API initiates a command to discover the specific GATT profile/service of the connected remote BLE device.
 *
 *             The discovery results are provided through callback functions:
 *               - \ref rsi_ble_on_profile_resp_t    - Invoked upon successful retrieval of the specified profile/service.
 *               - \ref rsi_ble_on_gatt_error_resp_t - Invoked when an error response is received.
 *
 * @note       This is a non-blocking API, still you need to wait until the callback \ref rsi_ble_on_profile_resp_t is received from the device,
 *             to initiate further attribute related transactions on this remote device address.
 *
 * @pre        Pre-conditions:
 *               - \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr      remote device address
 * @param[in]  profile_uuid  services/profiles which are searched using profile_uuid
 * @param[out] p_profile     NULL for p_profile because the specified profile/service information will be provided through the callback event.
 *                           Refer to \ref profile_descriptor_s structure for more details.
 *
 * @note       p_profile structure should be set to NULL.
 * 
 * @return     The following values are returned:
 *               - Zero              - Success
 *               - Non-Zero Value    - Failure
 *               - 0x4E62            - Invalid Parameters
 *               - 0x4D04            - BLE not connected
 *               - 0x4D05            - BLE Socket not available
 *
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_get_profile(uint8_t *dev_addr, uuid_t profile_uuid, profile_descriptors_t *p_profile);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_char_services(uint8_t *dev_addr,
 *                                               uint16_t start_handle,
 *                                               uint16_t end_handle,
 *                                               rsi_ble_resp_char_services_t *p_char_serv_list)
 * @brief      This API initiates a command to get the service characteristics of the connected remote BLE device.
 *
 *             The discovery results are provided through callback functions:
 *               - \ref rsi_ble_on_char_services_resp_t    - Invoked upon successful retrieval of the service characteristics.
 *               - \ref rsi_ble_on_gatt_error_resp_t       - Invoked when an error response is received.
 *
 * @note       This is a non-blocking API.
 *             Still you need to wait until the callback \ref rsi_ble_on_char_services_resp_t is received from the device,
 *             to initiate further attribute related transactions on this remote device address.
 *
 * @pre        Pre-conditions:
 *               - \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr         remote device address
 * @param[in]  start_handle     start handle (index) of the remote device's service records
 * @param[in]  end_handle       end handle (index) of the remote device's service records
 * @param[out] p_char_serv_list NULL for p_char_serv_list because the service characteristics information will be provided through the callback event.
 *                              Refer to \ref rsi_ble_resp_char_serv_s structure for more details.
 * 
 * @note       p_char_serv_list structure should be set to NULL.
 *
 * @return     The following values are returned:
 *               - 0                  - Success
 *               - Non-Zero Value     - Failure
 *               - 0x4E60             - Invalid Handle range
 *               - 0x4E62             - Invalid Parameters
 *               - 0x4D04             - BLE not connected
 *               - 0x4D05             - BLE Socket not available
 *
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_get_char_services(uint8_t *dev_addr,
                                  uint16_t start_handle,
                                  uint16_t end_handle,
                                  rsi_ble_resp_char_services_t *p_char_serv_list);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_inc_services(uint8_t *dev_addr,
 *                                              uint16_t start_handle,
 *                                              uint16_t end_handle,
 *                                              rsi_ble_resp_inc_services_t *p_inc_serv_list)
 * @brief      This API initiates a command to get the supported include services of the connected remote device.
 *
 *             The discovery results are provided through callback functions:
 *               - \ref rsi_ble_on_inc_services_resp_t    - Invoked upon successful retrieval of the supported include services.
 *               - \ref rsi_ble_on_gatt_error_resp_t      - Invoked when an error response is received.
 *
 * @note       This is a non-blocking API.
 *             Still you need to wait until the callback \ref rsi_ble_on_inc_services_resp_t is received from the device,
 *             to initiate further attribute related transactions on this remote device address.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr         remote device address
 * @param[in]  start_handle     start handle (index) of the remote device's service records
 * @param[in]  end_handle       end handle (index) of the remote device's service records
 * @param[out] p_inc_serv_list  NULL for p_inc_serv_list because the supported include services information will be provided through the callback event.
 *                              Refer to \ref rsi_ble_resp_inc_serv structure for more details.
 * 
 * @note       p_inc_serv_list structure should be set to NULL.
 * 
 * @return     The following values are returned:
 *               - 0              - Success
 *               - Non-Zero Value - Failure
 *               - 0x4E60         - Invalid Handle range
 *               - 0x4E62         - Invalid Parameters
 *               - 0x4D04         - BLE not connected
 *               - 0x4D05         - BLE Socket not available
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_get_inc_services(uint8_t *dev_addr,
                                 uint16_t start_handle,
                                 uint16_t end_handle,
                                 rsi_ble_resp_inc_services_t *p_inc_serv_list);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_char_value_by_uuid(uint8_t *dev_addr,
 *                                                    uint16_t start_handle,
 *                                                    uint16_t end_handle,
 *                                                    uuid_t char_uuid,
 *                                                    rsi_ble_resp_att_value_t *p_char_val)
 *
 * @brief      This API initiates a command to get the characteristic value by UUID (char_uuid).
 *
 *             The discovery results are provided through callback functions:
 *               - \ref rsi_ble_on_read_resp_t       - Invoked upon successful retrieval of a characteristic value from a specified characteristic service.
 *               - \ref rsi_ble_on_gatt_error_resp_t - Invoked when an error response is received.
 *
 * @note       This is a non-blocking API.
 *             Still you need to wait until the callback \ref rsi_ble_on_read_resp_t is received to initiate further attribute related transactions such as read operation on the remote device address.
 *
 * @pre        Pre-conditions:
 *             - \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr      remote device address
 * @param[in]  start_handle  start handle (index) of the remote device's service records
 * @param[in]  end_handle    end handle (index) of the remote device's service records
 * @param[in]  char_uuid     UUID of the characteristic which can be of sizes 128-bit, 32-bit, or 16-bit.
 * @param[out] p_char_val    NULL for p_char_val because the characteristic value from a specified characteristic service information will be provided through the callback event. 
 *                           Refer to \ref rsi_ble_resp_att_value_s structure for more details.
 * @note       p_char_val structure should be set to NULL.
 *          
 * @return     The following values are returned:
 *               - 0              - Success
 *               - Non-Zero Value - Failure
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_get_char_value_by_uuid(uint8_t *dev_addr,
                                       uint16_t start_handle,
                                       uint16_t end_handle,
                                       uuid_t char_uuid,
                                       rsi_ble_resp_att_value_t *p_char_val);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_att_descriptors(uint8_t *dev_addr,
 *                                                 uint16_t start_handle,
 *                                                 uint16_t end_handle,
 *                                                 rsi_ble_resp_att_descs_t *p_att_desc)
 *
 * @brief      This API initiates a command to get the characteristic descriptors list from the remote device.
 *
 *             The discovery results are provided through callback functions:
 *               - \ref rsi_ble_on_att_desc_resp_t   - Invoked upon successful retrieval of the characteristic descriptors list.
 *               - \ref rsi_ble_on_gatt_error_resp_t - Invoked when an error response is received.
 *
 * @note       This is a non-blocking API.
 *             Still you need to wait until the callback \ref rsi_ble_on_att_desc_resp_t is received from the device,
 *             to initiate further attribute related transactions on this remote device address.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr     remote device address
 * @param[in]  start_handle start handle (index) of the remote device's service records
 * @param[in]  end_handle   end handle (index) of the remote device's service records
 * @param[out] p_att_desc   NULL for p_att_desc because the characteristic descriptors list information will be provided through the callback event. 
 *                          Refer to \ref rsi_ble_resp_att_descs_s structure for more details.
 * @note       p_att_desc structure should be set to NULL.
 * 
 * @return     The following values are returned:
 *               - 0              - Success
 *               - Non-Zero Value - Failure
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_get_att_descriptors(uint8_t *dev_addr,
                                    uint16_t start_handle,
                                    uint16_t end_handle,
                                    rsi_ble_resp_att_descs_t *p_att_desc);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_att_value(uint8_t *dev_addr, uint16_t handle,
 *                                           rsi_ble_resp_att_value_t *p_att_val)
 *
 * @brief      This API initiates a command to get the attribute for a specific handle.
 *
 *             The discovery results are provided through callback functions:
 *               - \ref rsi_ble_on_read_resp_t       - Invoked upon successful retrieval of the attribute for the specified handle.
 *               - \ref rsi_ble_on_gatt_error_resp_t - Invoked when an error response is received.
 *
 * @note       This is a non-blocking API.
 *             Still you need to wait until the callback \ref rsi_ble_on_read_resp_t is received from the device,
 *             to initiate further attribute related transactions on this remote device address.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr   remote device address
 * @param[in]  handle     handle value of the attribute
 * @param[out] p_att_val  NULL for p_att_val because the attribute for the specified handle information will be provided through the callback event. 
 *                        Refer to \ref rsi_ble_resp_att_value_s structure for more details.
 * @note       p_att_val structure should be set to NULL.
 *  
 * @return     The following values are returned:
 *               - 0              - Success
 *               - Non-Zero Value - Failure
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_get_att_value(uint8_t *dev_addr, uint16_t handle, rsi_ble_resp_att_value_t *p_att_val);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_multiple_att_values(uint8_t *dev_addr,
 *                                                    uint8_t num_of_handlers,
 *                                                    const uint16_t *handles,
 *                                                    rsi_ble_resp_att_value_t *p_att_vals)
 *
 * @brief      This API initiates a command to get the multiple attribute values by using multiple handles.
 *
 *             The discovery results are provided through callback functions:
 *               - \ref rsi_ble_on_read_resp_t       - Invoked upon successful retrieval of the multiple attribute values for the specified multiple handles.
 *               - \ref rsi_ble_on_gatt_error_resp_t - Invoked when an error response is received.
 *
 * @note       This is a non-blocking API,
 *             Still you need to wait until the callback \ref rsi_ble_on_read_resp_t is received from the device,
 *             to initiate further attribute related transactions on this remote device address.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr        remote device address
 * @param[in]  num_of_handlers number of handles in the list
 * @param[in]  handles         list of attribute handles
 * @param[out] p_att_vals      NULL for p_att_vals because the multiple attribute values for the specified multiple handles information will be provided through the callback event. 
 *                             Refer to \ref rsi_ble_resp_att_value_s structure for more details.
 * @note       p_att_vals structure should be set to NULL.
 *  
 * @return     The following values are returned:
 *               - 0              - Success
 *               - Non-Zero Value - Failure
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_get_multiple_att_values(uint8_t *dev_addr,
                                        uint8_t num_of_handlers,
                                        const uint16_t *handles,
                                        rsi_ble_resp_att_value_t *p_att_vals);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_long_att_value(uint8_t *dev_addr,
 *                                                uint16_t handle,
 *                                                uint16_t offset,
 *                                                rsi_ble_resp_att_value_t *p_att_vals)
 * @brief      Initiates a command to get the long attribute value using handle and offset.
 *
 *             The discovery results are provided through callback functions:
 *               - \ref rsi_ble_on_read_resp_t         – Invoked upon successful retrieval of the long attribute value.
 *               - \ref rsi_ble_on_gatt_error_resp_t   – Invoked when an error response is received.
 *
 * @note       This is a non-blocking API.
 *             Still you need to wait until the callback \ref rsi_ble_on_read_resp_t is received from the device,
 *             to initiate further attribute related transactions on this remote device address.
 *
 * @pre        \ref rsi_ble_connect() API must be called before this API.
 *
 * @param[in]  dev_addr    Remote device address.
 * @param[in]  handle      Attribute handle.
 * @param[in]  offset      Offset within the attribute value.
 * @param[out] p_att_vals  NULL for p_att_vals because the long attribute value information will be provided through the callback event. 
 *                          Refer to \ref rsi_ble_resp_att_value_s structure for more details. 
 * @note       p_att_vals structure should be set to NULL.
 *  
 * @return     The following values are returned:
 *               - 0              - Success
 *               - Non-Zero Value - Failure
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_get_long_att_value(uint8_t *dev_addr,
                                   uint16_t handle,
                                   uint16_t offset,
                                   rsi_ble_resp_att_value_t *p_att_vals);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_att_value(uint8_t *dev_addr,
 *                                           uint16_t handle,
 *                                           uint8_t data_len,
 *                                           const uint8_t *p_data)
 * @brief      Initiates a command to set the attribute value of the remote device.
 *
 *             The result is provided through callback functions:
 *               - \ref rsi_ble_on_write_resp_t      – Invoked upon the attribute write action is completed.
 *               - \ref rsi_ble_on_gatt_error_resp_t – Invoked when an error response is received.
 *
 * @note       This is a non-blocking API. However, you need to wait until the \ref rsi_ble_on_write_resp_t callback is received from the device
 *             to initiate further attribute-related transactions on this remote device address.
 *
 * @pre        \ref rsi_ble_connect() API must be called before this API.
 *
 * @param[in]  dev_addr   Remote device address.
 * @param[in]  handle     Attribute value handle.
 * @param[in]  data_len   Attribute value length.
 * @param[in]  p_data     Attribute value.
 *
 * @return     The following values are returned:
 *               - 0              - Success
 *               - Non-Zero Value - Failure
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_set_att_value(uint8_t *dev_addr, uint16_t handle, uint8_t data_len, const uint8_t *p_data);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_att_cmd(uint8_t *dev_addr, uint16_t handle,
 *                                         uint8_t data_len, const uint8_t *p_data)
 * @brief      This API initiates a command to set the attribute value without waiting for an ACK from the remote device. This is a non-blocking API.
 *
 *             If the API returns RSI_ERROR_BLE_DEV_BUF_FULL (-31) error then wait until the \ref rsi_ble_on_le_more_data_req_t event gets received from the module.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr remote device address
 * @param[in]  handle   attribute value handle
 * @param[in]  data_len attribute value length
 * @param[in]  p_data   attribute value
 *
 * @return     The following values are returned:
 *               - 0              - Success
 *               - Non-Zero Value - Failure
 *               - 0x4E60         - Invalid Handle range
 *               - 0x4E62         - Invalid Parameters
 *               - 0x4D04         - BLE not connected
 *               - 0x4D05         - BLE Socket not available
 *               - 0x4E65         - Invalid Attribute Length When Small Buffer Mode is Configured
 *
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_set_att_cmd(uint8_t *dev_addr, uint16_t handle, uint8_t data_len, const uint8_t *p_data);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_att_cmd_async(uint8_t *dev_addr, uint16_t handle,
 *                                         uint8_t data_len, const uint8_t *p_data)
 * @brief      Set the attribute value without waiting for an ACK from the remote device. This is a blocking API.
 *             If the API returns an RSI_ERROR_BLE_DEV_BUF_FULL  (-31) error, wait until the \ref rsi_ble_on_le_more_data_req_t event is received from the module.
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  dev_addr - remote device address
 * @param[in]  handle 	- attribute value handle
 * @param[in]  data_len - attribute value length
 * @param[in]  p_data 	- attribute value
 * @return The following values are returned:
 *     - 0		-	Success 
 *     - Non-Zero Value	-	Failure 
 *     - 0x4E60  -  Invalid Handle range 
 *     - 0x4E62  -  Invalid Parameters 
 *     - 0x4D04  -  BLE not connected 
 *     - 0x4D05  -  BLE Socket not available 
 *     - 0x4E65  -  Invalid Attribute Length When Small Buffer Mode is Configured 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_set_att_cmd_async(uint8_t *dev_addr, uint16_t handle, uint8_t data_len, const uint8_t *p_data);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_long_att_value(uint8_t *dev_addr,
 *                                                uint16_t handle,
 *                                                uint16_t offset,
 *                                                uint8_t data_len,
 *                                                const uint8_t *p_data)
 * @brief      This API initiates a command to set the long attribute value of the remote device.
 *
 *             The discovery results are provided through callback functions:
 *               - \ref rsi_ble_on_write_resp_t      - Invoked upon the attribute write action is completed
 *               - \ref rsi_ble_on_gatt_error_resp_t - Invoked when an error response is received.
 *
 * @note       This is a non-blocking API.
 *             Still you need to wait until the callback \ref rsi_ble_on_write_resp_t is received from the device,
 *             to initiate further attribute related transactions on this remote device address.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr remote device address
 * @param[in]  handle   attribute handle
 * @param[in]  offset   attribute value offset
 * @param[in]  data_len attribute value length
 * @param[in]  p_data   attribute value
 *
 * @return     The following values are returned:
 *               - 0              - Success
 *               - Non-Zero Value - Failure
 *
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_set_long_att_value(uint8_t *dev_addr,
                                   uint16_t handle,
                                   uint16_t offset,
                                   uint8_t data_len,
                                   const uint8_t *p_data);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_prepare_write(uint8_t *dev_addr,
 *                                           uint16_t handle,
 *                                           uint16_t offset,
 *                                           uint8_t data_len,
 *                                           const uint8_t *p_data)
 * @brief      This API initiates a command to prepare the attribute value.
 *
 *             The discovery results are provided through callback functions:
 *               - \ref rsi_ble_on_write_resp_t      - Invoked upon successful prepare write operation.
 *               - \ref rsi_ble_on_gatt_error_resp_t - Invoked when an error response is received.
 *
 * @note       This is a non-blocking API.
 *             Still you need to wait until the callback \ref rsi_ble_on_write_resp_t is received from the device,
 *             to initiate further attribute related transactions on this remote device address.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr   Remote device address
 * @param[in]  handle     Attribute handle
 * @param[in]  offset     Attribute value offset
 * @param[in]  data_len   Attribute value length
 * @param[in]  p_data     Attribute value
 *
 * @return     The following values are returned:
 *               - 0              - Success
 *               - Non-Zero Value - Failure
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_prepare_write(uint8_t *dev_addr,
                              uint16_t handle,
                              uint16_t offset,
                              uint8_t data_len,
                              const uint8_t *p_data);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_execute_write(uint8_t *dev_addr, uint8_t exe_flag)
 *
 * @brief      This API initiates a command to execute the prepared attribute values.
 *
 *             The discovery results are provided through callback functions:
 *               - \ref rsi_ble_on_write_resp_t     - Invoked upon successful execute write operation.
 *               - \ref rsi_ble_on_gatt_error_resp_t - Invoked when an error response is received.
 *
 * @note       This is a non-blocking API.
 *             Still you need to wait until the callback \ref rsi_ble_on_write_resp_t is received from the device,
 *             to initiate further attribute related transactions on this remote device address.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr Remote device address
 * @param[in]  exe_flag Execute flag to write, possible values mentioned below:
 *                        - 0 - BLE_ATT_EXECUTE_WRITE_CANCEL
 *                        - 1 - BLE_ATT_EXECUTE_PENDING_WRITES_IMMEDIATELY
 *
 * @return     The following values are returned:
 *               - 0              - Success
 *               - Non-Zero Value - Failure
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_execute_write(uint8_t *dev_addr, uint8_t exe_flag);
/** @} */

/********************************************************
 * *        GATT Server API's Declarations
 * ******************************************************/
/** @addtogroup BT-LOW-ENERGY5
* @{
*/

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_add_service(uuid_t service_uuid, rsi_ble_resp_add_serv_t *p_resp_serv)
 * @brief      Add a new service to the local GATT Server. This is a blocking API.
 * @pre Pre-conditions:
 *        - Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  service_uuid 	- new service UUID value, refer to uuid_s structure for more details.
 * @param[out] p_resp_serv 	- new service handler filled in this structure, refer to rsi_ble_resp_add_serv_s structure for more details.
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure 
 *             - 0x4046  -  Invalid Arguments 
 *             - 0x4D08  -  Profile record full 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_add_service(uuid_t service_uuid, rsi_ble_resp_add_serv_t *p_resp_serv);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_add_attribute(rsi_ble_req_add_att_t *p_attribute)
 * @brief      Add a new attribute to a specific service. This is a blocking API.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  p_attribute - add a new attribute to the service, refer to rsi_ble_req_add_att_s structure for more details. 
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure 
 *             - 0x4046  -  Invalid Arguments 
 *             - 0x4D09  -  Attribute record full  
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_add_attribute(rsi_ble_req_add_att_t *p_attribute);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_local_att_value(uint16_t handle, uint16_t data_len,
 *                                                 const uint8_t *p_data)
 * @brief      Change the local attribute value. This is a blocking API.
 * When the gatt_database is present, the value can be altered using the "rsi_ble_set_local_att_value" API. 
 * This means the gatt_database value can be modified at any point following its creation. 
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  handle 	- attribute value handle
 * @param[in]  data_len - attribute value length
 * @param[in]  p_data 	- attribute value
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure 
 *             - 0x4046  -  Invalid Arguments 
 *             - 0x4D06  -  Attribute record not found 
 *             - 0x4E60  -  Invalid Handle Range 
 * @note      Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 * @note      This API can only be used if the service is maintained inside the firmware. 
 * @note      The services which are maintained by firmware must follow the below rules. 
 * @note      Rule 1: The attribute_data_size is less than 20 bytes during the service_creation 
 * @note      Rule 2: while creating the service, don't use the RSI_BLE_ATT_MAINTAIN_IN_HOST bit 
 *                    in the RSI_BLE_ATT_CONFIG_BITMAP macro. 
 * @note      Rule 3: The data_len must be less than or equal to the dat_length mentioned while 
 *                    creating the service/attribute 
 * @note		  Rule 4: The \ref rsi_ble_notify_value() API should be used instead of \ref rsi_ble_set_local_att_value() to update the SiWx91x local database when the services are maintained in the host itself.
 */
int32_t rsi_ble_set_local_att_value(uint16_t handle, uint16_t data_len, const uint8_t *p_data);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_wo_resp_notify_buf_info(const uint8_t *dev_addr, uint8_t buf_mode,
 *                                                         uint8_t buf_cnt)
 * @brief      Configure the buf mode for Notify and WO response commands for the remote device. This is a blocking API.
 * @param[in]  dev_addr - remote device address
 * @param[in]  buf_mode - buffer mode configuration
 *     - 0 - BLE_SMALL_BUFF_MODE 
 *     - 1 - BLE_BIG_BUFF_MODE 
 * @param[in]  buf_cnt - no of buffers to be configured 
 *    only value 1 and 2 are supported in BLE_SMALL_BUFF_MODE  
			in BLE_BIG_BUFF_MODE, buffers allocated based on the below notations.
			intial available_buf_cnt = RSI_BLE_NUM_CONN_EVENTS,
			a) When connection 1 is formed, the possible range of buffers is (available_buf_cnt - remaining possible number of connections)
			b) After allocating X buffers using \ref rsi_ble_set_wo_resp_notify_buf_info to the 1st connection remaining available_buf_cnt = (available_buf_cnt - X ) 
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure
 *             - 0x4046  -  Invalid Arguments 
 *             - 0x4D05  -  BLE socket not available 
 *             - 0x4D06  -  Attribute record not found 
 *             - 0x4E60  -  Invalid Handle Range 
 *             - 0x4E63  -  BLE Buffer Count Exceeded 
 *             - 0x4E64  -  BLE Buffer already in use  
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_set_wo_resp_notify_buf_info(const uint8_t *dev_addr, uint8_t buf_mode, uint8_t buf_cnt);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_notify_value(const uint8_t *dev_addr, uint16_t handle,
 *                                          uint16_t data_len, const uint8_t *p_data)
 * @brief      Notify the local value to the remote device. This is a blocking API.
 *             If the API returns RSI_ERROR_BLE_DEV_BUF_FULL  (-31) error then wait until the \ref rsi_ble_on_le_more_data_req_t event gets received from the module.
 * @pre Pre-conditions:
 *        - \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  dev_addr - remote device address
 * @param[in]  handle 	- local attribute handle
 * @param[in]  data_len - attribute value length
 * @param[in]  p_data 	- attribute value
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure 
 *             - 0x4046  -  Invalid Arguments 
 *             - 0x4A0D  -  Invalid attribute value length 
 *             - 0x4D05  -  BLE socket not available 
 *             - 0x4D06  -  Attribute record not found 
 *             - 0x4E60  -  Invalid Handle Range 
 *             - 0x4E65  -  Invalid Attribute Length When Small Buffer Mode is Configured  
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) 
 * @note       The rsi_ble_notify_value() API should be used instead of rsi_ble_set_local_att_value() to update the SiWx91x local database when the services are maintained in the host itself.
 */
int32_t rsi_ble_notify_value(const uint8_t *dev_addr, uint16_t handle, uint16_t data_len, const uint8_t *p_data);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_indicate_value(const uint8_t *dev_addr, uint16_t handle,
 *                                            uint16_t data_len, const uint8_t *p_data)
 * @brief      Indicate the local value to the remote device. This is a blocking API and can unblock the application
 *             on the reception of the callback functions either \ref rsi_ble_on_event_indicate_confirmation_t.
 * @pre Pre-conditions:
 *        - \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  dev_addr - remote device address
 * @param[in]  handle 	- local attribute handle
 * @param[in]  data_len - attribute value length
 * @param[in]  p_data 	- attribute value
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure 
 *             - 0x4D05  -  BLE socket not available 
 *             - 0x4E60  -  Invalid Handle Range 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) 
 */
int32_t rsi_ble_indicate_value(const uint8_t *dev_addr, uint16_t handle, uint16_t data_len, const uint8_t *p_data);
/** @} */

/** @addtogroup BT-LOW-ENERGY4
 * @{
 */

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_indicate_value_sync(const uint8_t *dev_addr, uint16_t handle,
 *                                            uint16_t data_len, const uint8_t *p_data)
 * @brief      Indicate the local value to the remote device. This is a blocking API.
 *
 *             This will not send any confirmation event to the application instead 
 *
 *             send the status as success on receiving confirmation from remote side.
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  dev_addr - remote device address
 * @param[in]  handle 	- local attribute handle
 * @param[in]  data_len - attribute value length
 * @param[in]  p_data 	- attribute value
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure 
 *             - 0x4D05  -  BLE socket not available 
 *             - 0x4E60  -  Invalid Handle Range 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) 
 */
int32_t rsi_ble_indicate_value_sync(const uint8_t *dev_addr, uint16_t handle, uint16_t data_len, const uint8_t *p_data);

/*==============================================*/
/**
 * @fn        int32_t rsi_ble_indicate_confirm(const uint8_t *dev_addr)
 * @brief     Send indicate confirmation to the remote device. This is a blocking API.
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in] dev_addr - remote device address
 * @return The following values are returned:
 *            - 0			-	Success 
 *            - Non-Zero Value	-	Failure
 *            - 0x4D05  -  BLE socket not available 
 * @note      Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) 
 * @note      API is required to be called from the application when the "RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST" is enabled in the opermode configurations.
 */
int32_t rsi_ble_indicate_confirm(const uint8_t *dev_addr);
/** @} */

/** @addtogroup BT-LOW-ENERGY5
* @{
*/

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_local_att_value(uint16_t handle,
 *                                                 rsi_ble_resp_local_att_value_t *p_resp_local_att_val)
 * @brief      Get the local attribute value. This is a blocking API.
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  handle 			- local attribute handle
 * @param[out] p_resp_local_att_val 	- local attribute value filled in this structure, refer to rsi_ble_resp_local_att_value_s structure for more details.
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure 
 *             - 0x4046  -  Invalid Arguments 
 *             - 0x4D06  -  Attribute record not found 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) 
 * @note       This API can only be used if the service is maintained inside the firmware. The services which are maintained by firmware must
 *             follow the below rules.
 * @note       Rule 1: The attribute_data_size is less than 20 bytes during the service_creation 
 * @note       Rule 2: While creating the service, don't use the RSI_BLE_ATT_MAINTAIN_IN_HOST bit in the RSI_BLE_ATT_CONFIG_BITMAP macro.
 */
int32_t rsi_ble_get_local_att_value(uint16_t handle, rsi_ble_resp_local_att_value_t *p_resp_local_att_val);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_gatt_read_response(uint8_t *dev_addr,
 *                                                uint8_t read_type,
 *                                                uint16_t handle,
 *                                                uint16_t offset,
 *                                                uint16_t length,
 *                                                const uint8_t *p_data)
 * @brief      Send the response for the read request received from the remote device This is a blocking API.
 * @pre Pre-conditions:
 *        - \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  dev_addr 	- remote device Address
 * @param[in]  read_type 	- read value type 
 *        - 0 - Read response 
 *        - 1 - Read blob response
 * @param[in]  handle 		- attribute value handle
 * @param[in]  offset 		- attribute value offset
 * @param[in]  length 		- attribute value length
 * @param[in]  p_data 		- attribute value
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure 
 *             - 0x4D04  -  BLE not connected 
 * @note       It's necessary to use the "rsi_ble_gatt_read_response" API when the services are managed by the Application/Host itself.
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) 
 */
int32_t rsi_ble_gatt_read_response(uint8_t *dev_addr,
                                   uint8_t read_type,
                                   uint16_t handle,
                                   uint16_t offset,
                                   uint16_t length,
                                   const uint8_t *p_data);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_remove_gatt_service(uint32_t service_handler)
 * @brief      Remove the GATT service record. This is a blocking API.
 * @pre Pre-conditions:
 *             GATT database needs to be present, GATT Database value can be modified at any point following its creation.
 * @param[in]  service_handler - GATT service record handle
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure 
 *             - 0x4D0A  -  BLE profile not found (profile handler invalid) 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) 
 */
int32_t rsi_ble_remove_gatt_service(uint32_t service_handler);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_remove_gatt_attibute(uint32_t service_handler, uint16_t att_hndl)
 * @brief      Remove the GATT attribute record. This is a blocking API.
 * @pre Pre-conditions:
 *             GATT database needs to be present, GATT Database value can be modified at any point following its creation.
 * @param[in]  service_handler - GATT service record handle
 * @param[in]  att_hndl - attribute handle
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure 
 *             - 0x4D06  -  Attribute record not found  
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) 
 */
int32_t rsi_ble_remove_gatt_attibute(uint32_t service_handler, uint16_t att_hndl);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_att_error_response(uint8_t *dev_addr, uint16_t handle,
 *												  uint8_t opcode, uint8_t err)
 * @brief      Send attribute error response for any of the att request. This is a blocking API.
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  dev_addr - remote device address
 * @param[in]  handle 	- attribute handle
 * @param[in]  opcode 	- error response opcode
 * @param[in]  err - specific error related Gatt
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure
 *             - 0x4D04  -  BLE not Connected  
 *             - 0x4E62  -  Invalid Parameters 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) 
 */
int32_t rsi_ble_att_error_response(uint8_t *dev_addr, uint16_t handle, uint8_t opcode, uint8_t err);
/** @} */

/** @addtogroup BT-LOW-ENERGY3
* @{
*/

/*==============================================*/
/** @addtogroup BT-LOW-ENERGY9
 * @{
 */
/**
 * @fn         int32_t rsi_ble_mtu_exchange_event(uint8_t *dev_addr, uint8_t mtu_size)
 * @brief      Initiates the MTU exchange request with the remote device.  
 *             This is a blocking API and will receive a callback event \ref rsi_ble_on_mtu_event_t as the response for this API.
 * @pre Pre-conditions:
 *        - \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  dev_addr - remote device address
 * @param[in]  mtu_size - requested MTU value
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure
 *             - 0x4D04  -  BLE not Connected  
 *             - 0x4E62  -  Invalid Parameters 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes)
 * @note       When services are maintained on the host, this API need to be triggered by the application.
 */
int32_t rsi_ble_mtu_exchange_event(uint8_t *dev_addr, uint8_t mtu_size);
/** @} */ // End of ATT_MAINTAIN_IN_HOST Group
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_mtu_exchange_resp(uint8_t *dev_addr, uint8_t mtu_size)
 * @brief      This function (Exchange MTU Response) is sent in reply to a received Exchange MTU Request.
 * @pre Pre-conditions:
 *        - \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  dev_addr - Remote Device Address
 * @param[in]  mtu_size - requested MTU value
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - 0x4D0C   -   When RSI_BLE_MTU_EXCHANGE_FROM_HOST BIT is not SET.
 *             - 0x4D05   -   BLE Socket Not Available.
 *             - Non-Zero Value	-	Failure
 * @note   Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes)
 */
int32_t rsi_ble_mtu_exchange_resp(uint8_t *dev_addr, uint8_t mtu_size);
/** @} */

/** @addtogroup BT-LOW-ENERGY5
* @{
*/

/*==============================================*/
/** @addtogroup BT-LOW-ENERGY9
 * @{
 */
/**
 * @fn         int32_t rsi_ble_gatt_write_response(uint8_t *dev_addr, uint8_t type)
 * @brief      Send the response to the write request received from the remote device. This is a blocking API.
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  dev_addr - remote device address
 * @param[in]  type 	- response type 
 *	- 0 - write response, 
 *  - 1 - execute write response.
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure
 *             - 0x4046  -  Invalid Arguments  
 *             - 0x4D04  -  BLE not Connected  
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes)
 * @note       When services are maintained on the host, this API need to be triggered by the application.
 */
int32_t rsi_ble_gatt_write_response(uint8_t *dev_addr, uint8_t type);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_gatt_prepare_write_response(uint8_t *dev_addr,
 *                                                         uint16_t handle,
 *                                                         uint16_t offset,
 *                                                         uint16_t length,
 *                                                         const uint8_t *data)
 * @brief      Send the response for the prepare write requests received from the remote device. This is a blocking API.
 * @pre Pre-conditions:
 *        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  dev_addr - remote device address
 * @param[in]  handle 	- attribute value handle
 * @param[in]  offset 	- attribute value offset
 * @param[in]  length - attribute value length
 * @param[in]  data 	- attribute value
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure
 *             - 0x4046  -  Invalid Arguments  
 *             - 0x4D04  -  BLE not Connected  
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) 
 * @note       When services are maintained on the host, this API need to be triggered by the application.
 */
int32_t rsi_ble_gatt_prepare_write_response(uint8_t *dev_addr,
                                            uint16_t handle,
                                            uint16_t offset,
                                            uint16_t length,
                                            const uint8_t *data);
/** @} */ // End of ATT_MAINTAIN_IN_HOST Group
/** @} */

/** @addtogroup BT-LOW-ENERGY1
* @{ */
/*========================================================*/
/**
 * @fn         int32_t rsi_ble_get_max_adv_data_len(uint8_t *resp)
 * @brief      Get maximum advertising data length
 * @param[out] resp Maximum supported advertising data length returned by the controller. Possible values range from 0x001F to 0x0672.
 * @return The following values are returned:
 *     - 0   =  success
 * @return The following values are returned:
 *     - !0  = failure
 * @note  This function requests the controller to return the maximum supported advertising data length.
 */
int32_t rsi_ble_get_max_adv_data_len(uint8_t *resp);

/*========================================================*/
/**
 * @fn         int32_t rsi_ble_get_max_no_of_supp_adv_sets(uint8_t *resp)
 * @brief      Get maximum number of advertising sets
 * @param[out] resp Number of supported advertising sets returned by the controller. Possible values range from 0x01 to 0xF0.
 * @return The following values are returned:
 *     0   =  success
 * @return The following values are returned:
 *     !0 = failure
 * @note This function requests the controller to return the maximum number of supporting advertising sets.
 * @note The number of supported advertising sets can be configured through the operating modes.
 */
int32_t rsi_ble_get_max_no_of_supp_adv_sets(uint8_t *resp);

/*==============================================*/
/**
 * @fn         rsi_ble_set_ae_set_random_address(uint8_t handle, const uint8_t *rand_addr)
 * @brief      Update AE random address
 * @param[in]  handle  The advertising handle used to identify an advertising set
 * @param[in]  rand_addr Random device address set to either a static or private address
 * @return The following values are returned:
 *     - 0   =  success
 * @return The following values are returned:
 *     - !0  = failure
 */
int32_t rsi_ble_set_ae_set_random_address(uint8_t handle, const uint8_t *rand_addr);

/*========================================================*/
/**
 * @fn         int32_t rsi_ble_set_ae_data(void *ble_ae_data)
 * @brief      Set the AE advertising data.
 * @param[in]  ble_ae_data Extended Advertising data to be updated
 * @return The following values are returned:
 *     0  =  success
 * @return The following values are returned:
 *     !0 = failure
 * @note  This function sets the AE advertiser data used in advertising PDUs.
 * @note  Refer to Bluetooth specification 5.4 for possible combinations in which ae_adv/scanresp data can be set.
 */
int32_t rsi_ble_set_ae_data(void *ble_ae_data);

/*========================================================*/
/**
 * @fn         int32_t rsi_ble_set_ae_params(void *ble_ae_params, int8_t *sel_tx_pwr)
 * @brief      Set the AE advertising parameters.
 * @param[in]  ble_ae_params Extended Advertising parameters to be updated
 * @param[out] sel_tx_pwr Output transmit power in dBm, ranging from -127 to +20.
 * @return The following values are returned:
 *     0  =  success
 * @return The following values are returned:
 *     !0 = failure
 */
int32_t rsi_ble_set_ae_params(void *ble_ae_params, int8_t *sel_tx_pwr);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_ae_advertising(void *adv_enable)
 * @brief      Enable or disable AE advertising
 * A received event \ref rsi_ble_adv_set_terminated_s,indicates that an AE advertising set has been terminated.
 * Note: This callback may be triggered due to duration timeout, event count completion, or manual stop, even after a connection is established.
 * @param[in]  adv_enable Parameters to enable or disable specific advertising sets identified by advertising handle
 * @return The following values are returned:
 *     0  =  success
 * @return The following values are returned:
 *     !0  = failure
 */
int32_t rsi_ble_start_ae_advertising(void *adv_enable);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_app_adv_set_clear_or_remove(uint8_t type, uint8_t handle)
 * @brief      request the local device to clear or remove the advertising sets based on the type specified
 *             This function requests the local device to clear or remove an advertising set from the controller based on the type specified
 * @param[in]  type - Specifies whether to remove or clear the advertising sets.
 *             {1}  - clear
 *             {2}  - remove
 * @param[in]  handle - Advertising_Handle - Used to identify an Advertising set. Possible Values : 0x00 to 0xEF
 * @return     0  =  success \n
 *            !0 = failure \n
 */
int32_t rsi_ble_app_adv_set_clear_or_remove(uint8_t type, uint8_t handle);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_app_set_periodic_ae_params(void *periodic_adv_params)
 * @brief      Set periodic advertising parameters.
 * @param[in]  periodic_adv_params Periodic advertising parameters to be updated
 * @return The following values are returned:
 *     0  =  success
 * @return The following values are returned:
 *     !0  = failure
 */
int32_t rsi_ble_app_set_periodic_ae_params(void *periodic_adv_params);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_app_set_periodic_ae_enable(uint8_t enable, uint8_t handle)
 * @brief      Enable periodic advertising
 * @param[in]  enable Configure the "enable" parameter to 0 to enable ae periodic advertisement on specified set, or 1 to include the ADI field in AUX_SYNC_IND PDUs
 * @param[in]  handle Advertising handle of the advertising set to enable or disable
 * @return The following values are returned:
 *     0  =  success:
 *     !0 =  failure
 * @note
 * This function requests the controller to enable or disable periodic advertising for the specified advertising set.
 */
int32_t rsi_ble_app_set_periodic_ae_enable(uint8_t enable, uint8_t handle);

/*========================================================*/
/**
 * @fn         int32_t rsi_ble_ae_set_scan_params(void *ae_scan_params)
 * @brief      Set AE scan parameters.
 * @param[in]  ae_scan_params Extended scan parameters to be updated
 * @return The following values are returned:
 *     0  =  success
 * @return The following values are returned:
 *     !0 = failure
 * @note
 * This function sets the extended scan parameters to be used on the physical advertising channels.
 */
int32_t rsi_ble_ae_set_scan_params(void *ae_scan_params);

/*==============================================*/
/**
* @fn         int32_t rsi_ble_ae_set_scan_enable(void *ae_scan_enable)
* @brief      request the local device to enable scanning for both legacy and extended advertising PDUs
*             A received event \ref rsi_ble_on_adv_report_event_t indicates advertise report of remote device received.
* @param[in]  ae_scan_enable - Extended Scan Enable command Parameters would be filled here 
* @return     0  =  success \n
*            !0  = failure \n
*/
int32_t rsi_ble_ae_set_scan_enable(void *ae_scan_enable);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_ae_set_periodic_sync(uint8_t type, void *periodic_sync_data)
 * @brief      Synchronize with a periodic advertising train from an advertiser and begin receiving periodic advertising packets.
 *             The callback \ref rsi_ble_ae_per_adv_sync_estbl_t will be received when an AE periodic sync has been established.
 * @param[in]  type - Configure the "type" parameter to values 1 to begin, 2 to cancel and 3 to terminate the periodic advertising sync
 * @param[in]  periodic_sync_data Parameters for starting a periodic advertising sync operation
 * @return The following values are returned:
 *     0  =  success
 *     0x4E42 = unknown advertising identifier
 *     0x4E0C = command not permitted 
 * @note
 * The operation is either started, cancelled or terminated depending on the type parameter.
 * @note
 * "Sync Terminate" to be used when synchronization has already been established, and operation needs to be stopped.
 * "Sync Cancel" to be used when synchronization is still in progress, and process is no longer intended to be completed.
 * @note
 * 
 */
int32_t rsi_ble_ae_set_periodic_sync(uint8_t type, void *periodic_sync_data);
int32_t rsi_ble_ae_set_periodic_sync(uint8_t type, void *periodic_sync_data);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_ae_dev_to_periodic_list(void *dev_to_list)
 * @brief      Manage a device in the periodic advertiser list
 *             This function adds a device to periodic advertiser list stored in the controller.
 * @param[in]  dev_to_list Details of a device to be added to the periodic advertiser list
 * @return The following values are returned:
 *     0  =  success
 *    !0  = failure
 */
int32_t rsi_ble_ae_dev_to_periodic_list(void *dev_to_list);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_ae_read_periodic_adv_list_size(uint8_t *resp)
 * @brief      Get periodic advertiser list size
 * @param[out] resp Periodic advertiser list size returned by the controller
 * @return The following values are returned:
 *     0  =  success
 * @return The following values are returned:
 *     !0 = failure
 */
int32_t rsi_ble_ae_read_periodic_adv_list_size(uint8_t *resp);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_extended_connect_with_params(void *ext_create_conn)
 * @brief      This function is used to create an ACL connection, with the local device in the central role, to a connectable advertiser.
 * 
 * @param[in]  ext_create_conn  A pointer to a structure of type \ref rsi_ble_ae_extended_create_connect_s, 
 *                              which contains the extended connection parameters.
 * 
 * @return     int32_t 
 *             - 0  : Success.
 *             - !0 : Failure (an error code indicating the reason for failure).
 * 
 * @note       This function establishes a BLE connection with a remote advertiser, utilizing extended
 *             connection parameters for enhanced flexibility. The local device operates in the Central role.
 *             Key fields in the structure include:
 * 
 *             - **initiator_filter_policy**: Determines the use of the Filter Accept List:
 *                 - `0x00`: Do not use the Filter Accept List; connect to the specified peer address.
 *                 - `0x01`: Use the Filter Accept List; ignore the peer address.
 * 
 *             - **own_addr_type**: Specifies the local device's address type:
 *                 - `0x00`: Public Device Address.
 *                 - `0x01`: Random Device Address.
 *                 - `0x02`: Resolvable Private Address (local IRK).
 *                 - `0x03`: Resolvable Private Address (local IRK or random address).
 * 
 *             - **remote_addr_type**: Specifies the peer's address type:
 *                 - `0x00`: Public Device Address or Public Identity Address.
 *                 - `0x01`: Random Device Address or Random (static) Identity Address.
 * 
 *             - **remote_addr**: The peer's address (Public, Random Static, Non-Resolvable Private, or Resolvable Private).
 * 
 *             - **init_phys**: Indicates the PHY(s) for initiating connections:
 *                 - Bit 0: LE 1M PHY.
 *                 - Bit 1: LE 2M PHY.
 *                 - Bit 2: LE Coded PHY.
 * 
 *             - **init_params**: Array of structures contains initiation parameters for each PHY.
 */
int32_t rsi_ble_extended_connect_with_params(void *ext_create_conn);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_read_transmit_power(void *resp)
 * @brief      Get supported transmit power range
 * @param[out] resp Minimum and maximum supported transmit power, returned by the controller based on the country region. Power ranges from -127 dBm to +20 dBm.
 * @return The following values are returned:
 *     0  =  success
 * @return The following values are returned:
 *     !0 = failure
 * @note
 * This function requests the controller to return the minimum and maximum supported transmit power based on the country region.
 */
int32_t rsi_ble_read_transmit_power(void *resp);

/** @} */

// GATT Client Functions
/**********************************************************************
 * In all GAT client APIs, all actions (get/set/write) are referred to
 * the GATT server running on the remote / connected device.
 *********************************************************************/

/** @addtogroup BT-LOW-ENERGY4
* @{
*/

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_profiles_async(uint8_t *dev_addr,
 *                                               uint16_t start_handle,
 *                                               uint16_t end_handle,
 *                                               rsi_ble_resp_profiles_list_t *p_prof_list)
 * @brief      This API initiates a command to discover the GATT profiles or services supported by a remote BLE device.
 *             The discovery results are provided asynchronously through callback functions:
 *               - \ref rsi_ble_on_event_profiles_list_t    - Invoked upon successful retrieval of the profile list.
 *               - \ref rsi_ble_on_gatt_error_resp_t        - Invoked when an error response is received.
 * @note       Although the results are delivered asynchronously via callbacks, this API is blocking. It waits for the
 *             command response before returning control to the application.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  dev_addr      - Remote device address
 * @param[in]  start_handle  - Start handle (index) of the remote device's service records
 * @param[in]  end_handle    - End handle (index) of the remote device's service records
 * @param[out] p_prof_list   - NULL for p_prof_list because the profiles/services information will be provided asynchronously through the callback event.
 *
 * @note       The p_prof_list structure should be set to NULL for asynchronous APIs.
 *
 * @return     The following values are returned:
 *               - Zero Value - Success
 *               - Non-Zero Value - Failure
 *               - 0x4E62  - Invalid Parameters
 *               - 0x4D04  - BLE not connected
 *               - 0x4D05  - BLE Socket not available
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_get_profiles_async(uint8_t *dev_addr,
                                   uint16_t start_handle,
                                   uint16_t end_handle,
                                   rsi_ble_resp_profiles_list_t *p_prof_list);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_profile_async(uint8_t *dev_addr, uuid_t profile_uuid,
 *                                               profile_descriptors_t *p_profile)
 * @brief      This API initiates a command to discover the specific GATT profile/service of the connected remote BLE device.
 *             The discovery results are provided asynchronously through callback functions:
 *               - \ref rsi_ble_on_event_profile_by_uuid_t  - Invoked upon successful retrieval of the specified profile/service.
 *               - \ref rsi_ble_on_gatt_error_resp_t        - Invoked when an error response is received.
 * @note       Although the results are delivered asynchronously via callbacks, this API is blocking. It waits for the command response before returning control to the application.
 *
 * @pre        Pre-conditions:
 *               - \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr       - remote device address
 * @param[in]  profile_uuid   - services/profiles which are searched using profile_uuid
 * @param[out] p_profile      - NULL for p_profile because the specified profile/service information will be provided asynchronously through the callback event.
 *                              
 * @note       p_profile structure should be set to NULL for Asynchronous APIs.
 *
 * @return     The following values are returned:
 *               - Zero              - Success
 *               - Non-Zero Value    - Failure
 *               - 0x4E62            - Invalid Parameters
 *               - 0x4D04            - BLE not connected
 *               - 0x4D05            - BLE Socket not available
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_get_profile_async(uint8_t *dev_addr, uuid_t profile_uuid, profile_descriptors_t *p_profile);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_char_services_async(uint8_t *dev_addr,
 *                                                     uint16_t start_handle,
 *                                                     uint16_t end_handle,
 *                                                     rsi_ble_resp_char_services_t *p_char_serv_list)
 * @brief      This API initiates a command to get the service characteristics of the connected remote BLE device.
 *             The discovery results are provided asynchronously through callback functions:
 *               - \ref rsi_ble_on_event_read_by_inc_services_t   - Invoked upon successful retrieval of the service characteristics.
 *               - \ref rsi_ble_on_gatt_error_resp_t              - Invoked when an error response is received.
 * @note       Although the results are delivered asynchronously via callbacks, this API is blocking. It waits for the command response before returning control to the application.
 *
 * @pre        Pre-conditions:
 *               - \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr         - remote device address
 * @param[in]  start_handle     - start handle (index) of the remote device's service records
 * @param[in]  end_handle       - end handle (index) of the remote device's service records
 * @param[out] p_char_serv_list - NULL for p_char_serv_list because the service characteristics information will be provided asynchronously through the callback event.
 *
 * @note       p_char_serv_list structure should be set to NULL for Asynchronous APIs.
 *
 * @return     The following values are returned:
 *               - 0                  - Success
 *               - Non-Zero Value     - Failure
 *               - 0x4E60             - Invalid Handle range
 *               - 0x4E62             - Invalid Parameters
 *               - 0x4D04             - BLE not connected
 *               - 0x4D05             - BLE Socket not available
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_get_char_services_async(uint8_t *dev_addr,
                                        uint16_t start_handle,
                                        uint16_t end_handle,
                                        rsi_ble_resp_char_services_t *p_char_serv_list);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_inc_services_async(uint8_t *dev_addr,
 *                                                    uint16_t start_handle,
 *                                                    uint16_t end_handle,
 *                                                    rsi_ble_resp_inc_services_t *p_inc_serv_list)
 * @brief      This API initiates a command to get the supported include services of the connected remote device. 
 *             The discovery results are provided asynchronously through callback functions:
 *               - \ref rsi_ble_on_event_read_att_value_t  - Invoked upon successful retrieval of the supported include services.
 *               - \ref rsi_ble_on_gatt_error_resp_t       - Invoked when an error response is received.
 *
 * @note       Although the results are delivered asynchronously via callbacks, this API is blocking. 
 *             It waits for the command response before returning control to the application.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr         - remote device address
 * @param[in]  start_handle     - start handle (index) of the remote device's service records
 * @param[in]  end_handle       - end handle (index) of the remote device's service records
 * @param[out] p_inc_serv_list  - NULL for p_inc_serv_list because the supported include services information will be provided asynchronously through the callback event. 
 *                                
 *
 * @note       p_inc_serv_list structure should be set to NULL for Asynchronous APIs.
 *
 * @return     The following values are returned:
 *               - 0              - Success 
 *               - Non-Zero Value - Failure 
 *               - 0x4E60         - Invalid Handle range 
 *               - 0x4E62         - Invalid Parameters 
 *               - 0x4D04         - BLE not connected 
 *               - 0x4D05         - BLE Socket not available 
 *
 * @note       Refer to the Status Codes section for the above error codes at 
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_get_inc_services_async(uint8_t *dev_addr,
                                       uint16_t start_handle,
                                       uint16_t end_handle,
                                       rsi_ble_resp_inc_services_t *p_inc_serv_list);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_char_value_by_uuid_async(uint8_t *dev_addr,
 *                                                          uint16_t start_handle,
 *                                                          uint16_t end_handle,
 *                                                          uuid_t char_uuid,
 *                                                          rsi_ble_resp_att_value_t *p_char_val)
 *
 * @brief      This API initiates a command to get the characteristic value by UUID (char_uuid).
 *             The discovery results are provided asynchronously through callback functions:
 *               - \ref rsi_ble_on_event_read_att_value_t - Invoked upon successful retrieval of a characteristic value from a specified characteristic service.
 *               - \ref rsi_ble_on_gatt_error_resp_t      - Invoked when an error response is received.
 *
 * @note       Although the results are delivered asynchronously via callbacks, this API is blocking.
 *             It waits for the command response before returning control to the application.
 *
 * @pre        Pre-conditions:
 *             - \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr     - Remote device address
 * @param[in]  start_handle - Start handle (index) of the remote device's service records
 * @param[in]  end_handle   - End handle (index) of the remote device's service records
 * @param[in]  char_uuid    - UUID of the characteristic
 * @param[out] p_char_val   - NULL for p_char_val because the characteristic value from a specified characteristic service information will be provided asynchronously through the callback event. 
 *                            
 *
 * @note       The p_char_val structure should be set to NULL for Asynchronous APIs.
 *
 * @return     The following values are returned:
 *               - 0              - Success 
 *               - Non-Zero Value - Failure 
 *               - 0x4E60         - Invalid Handle range 
 *               - 0x4E62         - Invalid Parameters 
 *               - 0x4D04         - BLE not connected 
 *               - 0x4D05         - BLE Socket not available 
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_get_char_value_by_uuid_async(uint8_t *dev_addr,
                                             uint16_t start_handle,
                                             uint16_t end_handle,
                                             uuid_t char_uuid,
                                             rsi_ble_resp_att_value_t *p_char_val);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_att_descriptors_async(uint8_t *dev_addr,
 *                                                       uint16_t start_handle,
 *                                                       uint16_t end_handle,
 *                                                       rsi_ble_resp_att_descs_t *p_att_desc)
 *
 * @brief      This API initiates a command to get the characteristic descriptors list from the remote device.
 *             The discovery results are provided asynchronously through callback functions:
 *               - \ref rsi_ble_on_gatt_desc_val_event_t - Invoked upon successful retrieval of the characteristic descriptors list.
 *               - \ref rsi_ble_on_gatt_error_resp_t     - Invoked when an error response is received.
 *
 * @note       Although the results are delivered asynchronously via callbacks, this API is blocking.
 *             It waits for the command response before returning control to the application.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr     - Remote device address
 * @param[in]  start_handle - Start handle (index) of the remote device's service records
 * @param[in]  end_handle   - End handle (index) of the remote device's service records
 * @param[out] p_att_desc   - NULL for p_att_desc because the characteristic descriptors list information will be provided asynchronously through the callback event. 
 *                            
 *
 * @note       The p_att_desc structure should be set to NULL for Asynchronous APIs.
 *
 * @return     The following values are returned:
 *               - 0              - Success 
 *               - Non-Zero Value - Failure
 *               - 0x4E60         - Invalid Handle               
 *               - 0x4E62         - Invalid Parameters 
 *               - 0x4D04         - BLE not connected 
 *               - 0x4D05         - BLE Socket not available
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_get_att_descriptors_async(uint8_t *dev_addr,
                                          uint16_t start_handle,
                                          uint16_t end_handle,
                                          rsi_ble_resp_att_descs_t *p_att_desc);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_att_value_async(uint8_t *dev_addr, uint16_t handle,
 *                                                 rsi_ble_resp_att_value_t *p_att_val)
 *
 * @brief      This API initiates a command to get the attribute for a specific handle.
 *             The discovery results are provided asynchronously through callback functions:
 *               - \ref rsi_ble_on_event_read_resp_t - Invoked upon successful retrieval of the attribute for the specified handle.
 *               - \ref rsi_ble_on_gatt_error_resp_t - Invoked when an error response is received.
 *
 * @note       Although the results are delivered asynchronously via callbacks, this API is blocking.
 *             It waits for the command response before returning control to the application.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr   - Remote device address
 * @param[in]  handle     - Handle value of the attribute
 * @param[out] p_att_val  - NULL for p_att_val because the attribute for the specified handle information will be provided asynchronously through the callback event. 
 *                          
 *
 * @note       The p_att_val structure should be set to NULL for Asynchronous APIs.
 *
 * @return     The following values are returned:
 *               - 0              - Success 
 *               - Non-Zero Value - Failure 
 *               - 0x4E60         - Invalid Handle range 
 *               - 0x4E62         - Invalid Parameters 
 *               - 0x4D04         - BLE not connected 
 *               - 0x4D05         - BLE Socket not available 
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_get_att_value_async(uint8_t *dev_addr, uint16_t handle, rsi_ble_resp_att_value_t *p_att_val);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_multiple_att_values_async(uint8_t *dev_addr,
 *                                                           uint8_t num_of_handlers,
 *                                                           const uint16_t *handles,
 *                                                           rsi_ble_resp_att_value_t *p_att_vals)
 *
 * @brief      This API initiates a command to get the multiple attribute values by using multiple handles. 
 *             The discovery results are provided asynchronously through callback functions:
 *               - \ref rsi_ble_on_event_read_resp_t - Invoked upon successful retrieval of the multiple attribute values for the specified multiple handles.
 *               - \ref rsi_ble_on_gatt_error_resp_t - Invoked when an error response is received.
 *
 * @note       Although the results are delivered asynchronously via callbacks, this API is blocking.
 *             It waits for the command response before returning control to the application.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr        - Remote device address
 * @param[in]  num_of_handlers - Number of handles in the list
 * @param[in]  handles         - List of attribute handles
 * @param[out] p_att_vals      - NULL for p_att_vals because the multiple attribute values for the specified multiple handles information will be provided asynchronously through the callback event. 
 *                               
 *
 * @note       The p_att_vals structure should be set to NULL for asynchronous APIs.
 *
 * @return     The following values are returned:
 *               - 0              - Success 
 *               - 0x4E60         - Invalid Handle range 
 *               - 0x4E62         - Invalid Parameters 
 *               - 0x4D04         - BLE not connected 
 *               - 0x4D05         - BLE Socket not available 
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_get_multiple_att_values_async(uint8_t *dev_addr,
                                              uint8_t num_of_handlers,
                                              const uint16_t *handles,
                                              rsi_ble_resp_att_value_t *p_att_vals);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_long_att_value_async(uint8_t *dev_addr,
 *                                                      uint16_t handle,
 *                                                      uint16_t offset,
 *                                                      rsi_ble_resp_att_value_t *p_att_vals)
 * @brief      Initiates a command to get the long attribute value using handle and offset.
 *             The discovery results are provided asynchronously through callback functions:
 *             - \ref rsi_ble_on_event_read_resp_t – Invoked upon successful retrieval of the long attribute value.
 *             - \ref rsi_ble_on_gatt_error_resp_t – Invoked when an error response is received.
 *
 * @note       Although the results are delivered asynchronously via callbacks, this API is blocking.
 *             It waits for the command response before returning control to the application.
 *
 * @pre        \ref rsi_ble_connect() API must be called before this API.
 *
 * @param[in]  dev_addr    - Remote device address.
 * @param[in]  handle      - Attribute handle.
 * @param[in]  offset      - Offset within the attribute value.
 * @param[out] p_att_vals  - NULL for p_att_vals because the long attribute value information will be provided asynchronously through the callback event. 
 *                          
 *
 * @note       The p_att_vals structure should be set to NULL for asynchronous APIs.
 *
 * @return     Possible return values:
 *             - 0              Success
 *             - Non-Zero       Failure
 *             - 0x4E60         Invalid Handle range
 *             - 0x4E62         Invalid Parameters
 *             - 0x4D04         BLE not connected
 *             - 0x4D05         BLE Socket not available
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_get_long_att_value_async(uint8_t *dev_addr,
                                         uint16_t handle,
                                         uint16_t offset,
                                         rsi_ble_resp_att_value_t *p_att_vals);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_att_value_async(uint8_t *dev_addr,
 *                                                 uint16_t handle,
 *                                                 uint8_t data_len,
 *                                                 const uint8_t *p_data)
 * @brief      Initiates a command to set the attribute value of the remote device.
 *             The result is provided asynchronously through callback functions:
 *             - \ref rsi_ble_on_event_write_resp_t – Invoked upon the attribute write action is completed.
 *             - \ref rsi_ble_on_gatt_error_resp_t – Invoked when an error response is received.
 *
 * @note       Although the results are delivered asynchronously via callbacks, this API is blocking.
 *             It waits for the command response before returning control to the application.
 *
 * @pre        \ref rsi_ble_connect() API must be called before this API.
 *
 * @param[in]  dev_addr   Remote device address.
 * @param[in]  handle     Attribute value handle.
 * @param[in]  data_len   Attribute value length.
 * @param[in]  p_data     Attribute value.
 *
 * @return     Possible return values:
 *             - 0              Success
 *             - Non-Zero       Failure
 *             - 0x4E60         Invalid Handle range
 *             - 0x4E62         Invalid Parameters
 *             - 0x4D04         BLE not connected
 *             - 0x4D05         BLE Socket not available
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_set_att_value_async(uint8_t *dev_addr, uint16_t handle, uint8_t data_len, const uint8_t *p_data);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_prepare_write_async(uint8_t *dev_addr,
 *                                                 uint16_t handle,
 *                                                 uint16_t offset,
 *                                                 uint8_t data_len,
 *                                                 const uint8_t *p_data)
 * @brief      This API initiates a command to prepare the attribute value.
 *             The discovery results are provided asynchronously through callback functions:
 *               - \ref rsi_ble_on_event_prepare_write_resp_t - Invoked upon successful prepare write operation.
 *               - \ref rsi_ble_on_gatt_error_resp_t          - Invoked when an error response is received.
 *
 * @note       Although the results are delivered asynchronously via callbacks,
 *             this API is blocking. It waits for the command response before
 *             returning control to the application.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr   - Remote device address
 * @param[in]  handle     - Attribute handle
 * @param[in]  offset     - Attribute value offset
 * @param[in]  data_len   - Attribute value length
 * @param[in]  p_data     - Attribute value
 *
 * @return     The following values are returned:
 *               - 0       - Success 
 *               - 0x4E60  - Invalid Handle range 
 *               - 0x4E62  - Invalid Parameters 
 *               - 0x4D04  - BLE not connected 
 *               - 0x4D05  - BLE Socket not available
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_prepare_write_async(uint8_t *dev_addr,
                                    uint16_t handle,
                                    uint16_t offset,
                                    uint8_t data_len,
                                    const uint8_t *p_data);

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_execute_write_async(uint8_t *dev_addr, uint8_t exe_flag)
 *
 * @brief      This API initiates a command to execute the prepared attribute values.
 *             The discovery results are provided asynchronously through callback functions:
 *               - \ref rsi_ble_on_event_write_resp_t - Invoked upon successful execute write operation.
 *               - \ref rsi_ble_on_gatt_error_resp_t  - Invoked when an error response is received.
 *
 * @note       Although the results are delivered asynchronously via callbacks,
 *             this API is blocking. It waits for the command response before
 *             returning control to the application.
 *
 * @pre        Pre-conditions:
 *             \ref rsi_ble_connect() API needs to be called before this API.
 *
 * @param[in]  dev_addr - Remote device address
 * @param[in]  exe_flag - Execute flag to write. The possible values are listed below:
 *                          - 0 - BLE_ATT_EXECUTE_WRITE_CANCEL
 *                          - 1 - BLE_ATT_EXECUTE_PENDING_WRITES_IMMEDIATELY
 *
 * @return     The following values are returned:
 *               - 0       - Success
 *               - Non-Zero Value - Failure
 *               - 0x4D05  - BLE Socket not available
 *
 * @note       Refer to the Status Codes section for the above error codes at
 *             [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_execute_write_async(uint8_t *dev_addr, uint8_t exe_flag);
/** @} */

/*==============================================*/

/**
 * @fn uint32_t rsi_ble_cbfc_connreq(uint8_t *dev_addr, uint16_t psm)
 * @brief Sends a connection request to a remote device using the Credit Based Flow Control (CBFC) mechanism.
 *
 * This function sends a connection request to a remote device using the CBFC mechanism. It takes the device address and the Protocol Service Multiplexer (PSM) as input parameters.
 *
 * @param dev_addr Pointer to the device address of the remote device.
 * @param psm The Protocol Service Multiplexer (PSM) value.
 *
 * @return Returns a 32-bit unsigned integer indicating the status of the connection request.
 *         - 0: If the connection request is successful.
 *         - Non-zero: If there is an error in sending the connection request.
 */
uint32_t rsi_ble_cbfc_connreq(uint8_t *dev_addr, uint16_t psm);

/**
 * @fn uint32_t rsi_ble_cbfc_connresp(uint8_t *dev_addr, uint16_t lcid, uint8_t result)
 * @brief Sends a connection response for a credit-based flow control (CBFC) connection.
 *
 * This function is used to send a connection response for a CBFC connection in the BLE module.
 *
 * @param dev_addr The device address of the remote device.
 * @param lcid The logical channel ID of the connection.
 * @param result The result of the connection response.
 *
 * @return The status of the operation. Returns a 32-bit unsigned integer.
 */
uint32_t rsi_ble_cbfc_connresp(uint8_t *dev_addr, uint16_t lcid, uint8_t result);

/**
 * @fn uint32_t rsi_ble_cbfc_data_tx(uint8_t *dev_addr, uint16_t lcid, uint16_t len, uint8_t *p_data)
 * @brief Transmits data over a CBFC channel in the BLE module.
 *
 * This function is used to transmit data over a CBFC channel in the BLE module. It takes the device address, logical channel ID (LCID),
 * length of the data, and a pointer to the data as input parameters. The function returns a 32-bit unsigned integer as the status of the transmission.
 *
 * @param dev_addr Pointer to the device address of the BLE module.
 * @param lcid Logical channel ID of the CBFC channel.
 * @param len Length of the data to be transmitted.
 * @param p_data Pointer to the data to be transmitted.
 *
 * @return 32-bit unsigned integer representing the status of the transmission.
 */
uint32_t rsi_ble_cbfc_data_tx(uint8_t *dev_addr, uint16_t lcid, uint16_t len, uint8_t *p_data);

/**
 * @fn uint32_t rsi_ble_cbfc_disconnect(uint8_t *dev_addr, uint16_t lcid)
 * @brief Disconnects a Connection-Based Flow Control (CBFC) connection with the specified device address and logical channel ID.
 *
 * This function is used to disconnect a CBFC connection with the specified device address and logical channel ID.
 *
 * @param dev_addr The device address of the remote device.
 * @param lcid The logical channel ID of the CBFC connection.
 *
 * @return The status code indicating the success or failure of the operation.
 *         - Returns 0 on success.
 *         - Returns a non-zero value on failure.
 */
uint32_t rsi_ble_cbfc_disconnect(uint8_t *dev_addr, uint16_t lcid);
/******************************************************
 * *       BLE GAP Callbacks Declarations
 * ******************************************************/
/** @addtogroup BT-LOW-ENERGY8
* @{
*/
/**
 * @typedef    void (*rsi_ble_on_adv_report_event_t)(rsi_ble_event_adv_report_t *rsi_ble_event_adv);
 * @brief      Callback function advertise event report from the module.
 *             This  callback function is called whenever an advertise event report is received from the module.
 *             It has to registered using the `rsi_ble_gap_register_callbacks` API.               
 * @param[out]  rsi_ble_event_adv includes the advertise report information. Refer to rsi_ble_event_adv_report_s for more details.
 * @return The following values are returned:
 *      void
 */
typedef void (*rsi_ble_on_adv_report_event_t)(rsi_ble_event_adv_report_t *rsi_ble_event_adv);

/**
 * @typedef   void (*rsi_ble_on_connect_t)(rsi_ble_event_conn_status_t *rsi_ble_event_conn);
 * @brief Callback function for the BLE connection status from the module.
 *
 * This callback function is called whenever the BLE connection status is received from the module.
 * For Bluetooth 4.1 and lower versions, this callback is received.
 * It has to be registered using the `rsi_ble_gap_register_callbacks` API.
 * @param[out]  rsi_ble_event_conn includes the connection status. Refer \ref rsi_ble_event_conn_status_s for more details.
 * @return The following values are returned:
 *      void
 */
typedef void (*rsi_ble_on_connect_t)(rsi_ble_event_conn_status_t *rsi_ble_event_conn);

/**
 * @typedef void (*rsi_ble_on_conn_update_complete_t)(rsi_ble_event_conn_update_t *rsi_ble_event_conn_update_complete,
                                                  uint16_t resp_status);
 * @brief Callback function for a connection update complete event from the module.
 *        This callback function is called whenever the connection update complete event is received. 
 *        \ref rsi_ble_event_conn_update_s contains connection information.
 *        It has to be registered using the `rsi_ble_gap_register_callbacks` API.                                                
 * @param[out] rsi_ble_event_conn_update_complete contains the connection information. Refer to \ref rsi_ble_event_conn_update_s for more details.
 * @param[out] resp_status contains the response status (Success or Error code)
 */
typedef void (*rsi_ble_on_conn_update_complete_t)(rsi_ble_event_conn_update_t *rsi_ble_event_conn_update_complete,
                                                  uint16_t resp_status);
/**
 * @typedef    void (*rsi_ble_on_enhance_connect_t)(rsi_ble_event_enhance_conn_status_t *rsi_ble_event_enhance_conn);
 * @brief Callback function for the BLE enhanced connection status from the module.
 *
 * This callback function is called whenever the connection status is received from the module.
 * For Bluetooth 4.2 and later versions, this callback will be triggered.
 * It has to be registered using the `rsi_ble_gap_register_callbacks` API.
 * @param[out]  rsi_ble_event_enhance_conn Contains the connection status and connection parameters, such as connection interval, latency, and supervision timeout, and more. Refer to \ref rsi_ble_event_enhance_conn_status_s for more details.
 * @return The following values are returned:
 *      void
 */
typedef void (*rsi_ble_on_enhance_connect_t)(rsi_ble_event_enhance_conn_status_t *rsi_ble_event_enhance_conn);

/**
 * @typedef    void (*rsi_ble_on_data_length_update_t)(rsi_ble_event_data_length_update_t *remote_dev_address);
 * @brief Callback function for the data length update complete event.
 *        This callback function is called whenever the data length update complete event is received. \ref rsi_ble_event_data_length_update_s type variable remote_dev_address
 *        contains information about dev_addr - Device address of the remote device , MaxTxOctets - Maximum TX Octets to be transmitted , MaxTxTime - Maximum TX time to transmit the MaxTxOctets ,
 *        MaxRxOctets - Maximum Rx Octets to be received , MaxRxTime - Maximum Rx time to receive the MaxRxOctets
 *        It has to be registered using the `rsi_ble_gap_register_callbacks` API.
 * @param[out] remote_dev_address contains the controller support TX and RX length, and transmission time information. Refer to \ref rsi_ble_event_data_length_update_s for more details.
 */
typedef void (*rsi_ble_on_data_length_update_t)(rsi_ble_event_data_length_update_t *remote_dev_address);

/**
 * @typedef    void (*rsi_ble_on_directed_adv_report_event_t)(rsi_ble_event_directedadv_report_t *rsi_ble_event_directed);
 * @brief Callback function for a directed advertise report event from the module.
 * This callback function is called whenever a directed advertise report event is received from the module.
 * It has to be registered using the `rsi_ble_gap_register_callbacks` API.
 * @param[in]  rsi_ble_event_directed contains the advertise report information
 * 
 */
typedef void (*rsi_ble_on_directed_adv_report_event_t)(rsi_ble_event_directedadv_report_t *rsi_ble_event_directed);

/**
 * @typedef void (*rsi_ble_on_disconnect_t)(rsi_ble_event_disconnect_t *rsi_ble_event_disconnect, uint16_t reason);
 * @brief Callback function for the disconnect status event from the module.
 *
 * This callback function is called whenever the disconnect status event is received from the module.
 * It has to be registered using the `rsi_ble_gap_register_callbacks` API.
 * @param[out]  rsi_ble_event_disconnect contains the disconnect status. Refer to rsi_ble_event_disconnect_s for more details.
 * @param[out]  reason contains reason for failure. 
 * @note        Few reason for failure are given below 
 *              0x4E13	Remote user terminated connection 
 *              0x4E14	Remote device terminated connection due to low resources 
 *              0x4E15	Remote device terminated connection due to power off 
 *              0x4E3D	Connection terminated due to MIC failure 
 *              0x4E3E  Connection Failed to be Established 
 *              0x4E08  Connection Timeout
 *              0x4E60  Invalid Handle Range
 *              
 * @return The following values are returned:
 *      void
 * 
 */
typedef void (*rsi_ble_on_disconnect_t)(rsi_ble_event_disconnect_t *rsi_ble_event_disconnect, uint16_t reason);

/**
 * @typedef    void (*rsi_ble_on_phy_update_complete_t)(rsi_ble_event_phy_update_t *rsi_ble_event_phy_update_complete);
 * @brief Callback function for receiving the PHY update complete event.
 *
 * This callback function is called whenever the PHY update complete event is received.
 * It has to be registered using the `rsi_ble_gap_register_callbacks` API.
 * @param[out] rsi_ble_event_phy_update_complete  contains the controller support PHY information
 *                                                BIT(0) - The Host prefers to use the LE 1M transmitter PHY (possibly among others) 
 *                                                -
 *                                                BIT(1) - The Host prefers to use the LE 2M transmitter PHY (possibly among others) 
 *                                                -
 *                                                BIT(2) - The Host prefers to use the LE Coded transmitter PHY (possibly among others) 
 *                                                Refer to \ref rsi_ble_event_phy_update_s for more details.
 * 
 */
typedef void (*rsi_ble_on_phy_update_complete_t)(rsi_ble_event_phy_update_t *rsi_ble_event_phy_update_complete);

/**
 * @typedef void (*rsi_ble_on_le_ping_payload_timeout_t)(rsi_ble_event_le_ping_time_expired_t *rsi_ble_event_timeout_expired);
 * @brief Callback function for the LE ping time expired event from the module.
 *
 * This callback function is called whenever the LE ping time expired event is received from the module.
 * It has to be registered using the `rsi_ble_gap_register_callbacks` API.
 * @param[out]  rsi_ble_event_timeout_expired contains the disconnect status. Refer to \ref rsi_ble_event_le_ping_time_expired_s for more details.
 * @return The following values are returned:
 *      void
 */
typedef void (*rsi_ble_on_le_ping_payload_timeout_t)(
  rsi_ble_event_le_ping_time_expired_t *rsi_ble_event_timeout_expired);

/**
 * @typedef void (*rsi_ble_on_remote_conn_params_request_t)(
 *                              rsi_ble_event_remote_conn_param_req_t *rsi_ble_event_remote_conn_param,
 *                              uint16_t resp_status);
 * @brief Callback function for remote connection parameters request.
 *
 * This callback function is called whenever a remote connection parameters request is received.
 * It has to be registered using the `rsi_ble_gap_register_callbacks` API.
 * @param[out] resp_status contains the response status (Success or Error code)
 * @param[out] rsi_ble_event_remote_conn_param contains the remote device connection parameters. Refer to \ref rsi_ble_event_remote_conn_param_req_s for more details.
 * @return The following values are returned:
 *      void
 * 
 * 
 */
typedef void (*rsi_ble_on_remote_conn_params_request_t)(
  rsi_ble_event_remote_conn_param_req_t *rsi_ble_event_remote_conn_param,
  uint16_t resp_status);

/******************************************************
 * *       BLE GAP EXTENDED Callbacks Declarations
 * ******************************************************/
/**
 * @typedef void (*rsi_ble_on_remote_features_t)(rsi_ble_event_remote_features_t *rsi_ble_event_remote_features);
 * @brief      Callback function for supported features of remote device
 *             This callback function will be called when LE remote features event is received.
 *             it has to be registered using rsi_ble_gap_extended_register_callbacks API.
 * @param[out] rsi_ble_event_remote_features contains the remote device supported features. Refer to \ref rsi_ble_event_remote_features_s for more details.
 */
typedef void (*rsi_ble_on_remote_features_t)(rsi_ble_event_remote_features_t *rsi_ble_event_remote_features);

/**
 * @typedef    void (*rsi_ble_on_le_more_data_req_t)(rsi_ble_event_le_dev_buf_ind_t *rsi_ble_more_data_evt);
 * @brief Callback function for the LE more data event.
 *
 * This callback function is called whenever an LE more data event is received.
 * It has to be registered using the `rsi_ble_gap_extended_register_callbacks` API.
 * @param[out] rsi_ble_more_data_evt contains the LE Device Buffer Indication information. Refer to \ref rsi_ble_event_le_dev_buf_ind_s for details.
 * @return The following values are returned:
 *     void
 * 
 * 
 * 
 */
typedef void (*rsi_ble_on_le_more_data_req_t)(rsi_ble_event_le_dev_buf_ind_t *rsi_ble_more_data_evt);
/** @} */

/******************************************************
 * *       BLE SMP EVENT Callbacks Declarations
 * ******************************************************/
/** @addtogroup BT-LOW-ENERGY8
* @{
*/
/**
 * @typedef    void (*rsi_ble_on_le_ltk_req_event_t)(rsi_bt_event_le_ltk_request_t *rsi_ble_event_le_ltk_request);
 * @brief Callback function for the LE LTK request event from the module.
 *
 * This callback function is called whenever the LE LTK request event is received from the module.
 * It has to be registered using the `rsi_ble_smp_register_callbacks` API.
 * @param[out]  rsi_ble_event_le_ltk_request contains the LTK request information. Refer to \ref rsi_bt_event_le_ltk_request_s for more details.
 * @return The following values are returned:
 *      void
 */
typedef void (*rsi_ble_on_le_ltk_req_event_t)(rsi_bt_event_le_ltk_request_t *rsi_ble_event_le_ltk_request);

/**
 * @typedef    void (*rsi_ble_on_le_security_keys_t)(rsi_bt_event_le_security_keys_t *rsi_ble_event_le_security_keys);
 * @brief Callback function for the LE security keys event from the module.
 *        This callback function is called whenever the LE security keys event is received from the module.
 *        It has to be registered using the `rsi_ble_smp_register_callbacks` API.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[out]  rsi_ble_event_le_security_keys contains security keys. Refer to \ref rsi_bt_event_le_security_keys_s for more details.
 */
typedef void (*rsi_ble_on_le_security_keys_t)(rsi_bt_event_le_security_keys_t *rsi_ble_event_le_security_keys);

/**
 * @typedef    void (*rsi_ble_on_smp_request_t)(rsi_bt_event_smp_req_t *remote_dev_address);
 * @brief Callback function for the SMP request in central mode from the remote device.
 *        This callback function is called whenever an SMP request is received in central mode from the remote device.
 *        When the central device initiates the bonding process, the application receives the smp_req from the module.
 *        It has to be registered using the `rsi_ble_smp_register_callbacks` API.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[out] remote_dev_address contains the SMP request information. Refer to \ref rsi_bt_event_smp_req_s for more details.
 * @return The following values are returned:
 *     void  
 */
typedef void (*rsi_ble_on_smp_request_t)(rsi_bt_event_smp_req_t *remote_dev_address);

/*==============================================*/
/**
 * @typedef    void (*rsi_ble_on_smp_response_t)(rsi_bt_event_smp_resp_t *remote_dev_address);
 * @brief Callback function for the SMP response in peripheral mode from the remote device.
 *        This callback function is called whenever an SMP response is received in peripheral mode from the remote device.
 *        When the central device initiates the bonding process, the application receives the smp_resp from the module.
 *        It has to be registered using the `rsi_ble_smp_register_callbacks` API.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[out] remote_dev_address contains the SMP response information. Refer to \ref rsi_bt_event_smp_resp_s for more details.
 * @return The following values are returned:
 *     void  
 * 
 */
typedef void (*rsi_ble_on_smp_response_t)(rsi_bt_event_smp_resp_t *remote_dev_address);

/*==============================================*/
/**
 * @typedef    void (*rsi_ble_on_smp_passkey_t)(rsi_bt_event_smp_passkey_t *remote_dev_address);
 * @brief Callback function for SMP passkey event from the module.
 *        This callback function is called whenever an SMP passkey event is received from the module.
 *        It has to be registered using the `rsi_ble_smp_register_callbacks` API.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[out] remote_dev_address contains the remote device address. Refer to \ref rsi_bt_event_smp_passkey_s for more details.
 */
typedef void (*rsi_ble_on_smp_passkey_t)(rsi_bt_event_smp_passkey_t *remote_dev_address);

/*==============================================*/
/**
 * @typedef    void (*rsi_ble_on_smp_passkey_display_t)(rsi_bt_event_smp_passkey_display_t *smp_passkey_display);
 * @brief Callback function for SMP passkey display event from the module.
 *        This callback function is called whenever an SMP passkey display event is received from the module.
 *        It has to be registered using the `rsi_ble_smp_register_callbacks` API.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[out] smp_passkey_display contains the smp passkey display information. Refer to \ref rsi_bt_event_smp_passkey_display_s for more details.
 */
typedef void (*rsi_ble_on_smp_passkey_display_t)(rsi_bt_event_smp_passkey_display_t *smp_passkey_display);

/*==============================================*/
/**
 * @typedef    void (*rsi_ble_on_smp_failed_t)(uint16_t resp_status, rsi_bt_event_smp_failed_t *remote_dev_address);
 * @brief Callback function for SMP failed event from the module.
 *        This callback function will be called if the  smp process is failed with remote device.
 *        It has to be registered using the `rsi_ble_smp_register_callbacks` API.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[out] resp_status contains the response status (Success or Error code) 
 * @note       Error codes for SMP FAILED are given below: 
 *              0x4B01	SMP Passkey entry failed  
 *              0x4B02	SMP OOB not available 
 *              0x4B03	SMP Authentication Requirements 
 *              0x4B04	SMP confirm value failed 
 *              0x4B05	SMP Pairing not supported 
 *              0x4B06	SMP Encryption key size insufficient 
 *              0x4B07	SMP command not supported 
 *              0x4B08	SMP Unspecified Reason 
 *              0x4B09	SMP repeated attempts 
 *              0x4B0C	SMP Numeric Comparison Failed 
 *              0x4B0B DHKEY Check Failed
 * @param[out] remote_dev_address contains the remote device address. Refer to \ref rsi_bt_event_smp_failed_s for more details.
 */
typedef void (*rsi_ble_on_smp_failed_t)(uint16_t resp_status, rsi_bt_event_smp_failed_t *remote_dev_address);

/*==============================================*/
/**
 * @typedef   void (*rsi_ble_on_sc_method_t)(rsi_bt_event_sc_method_t *scmethod);
 * @brief Callback function for a secure connection method event from the module.
 *        This callback function will be called if the secure connection method is done with remote device.
 *        It has to be registered using the `rsi_ble_smp_register_callbacks` API.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[out] scmethod Pointer to a structure of type `rsi_bt_event_sc_method_t` containing details about
 *                      the Security Method used. Possible values:
 *                      - `1`: **Just Works** – No user interaction required, suitable for devices without input/output capabilities.
 *                      - `2`: **Passkey**    – Requires to enter or confirm a 6-digit passkey, which provides strong security.
 *                      Refer to \ref rsi_bt_event_sc_method_s for more details.
 */
typedef void (*rsi_ble_on_sc_method_t)(rsi_bt_event_sc_method_t *scmethod);

/*==============================================*/
/**
 * @typedef    void (*rsi_ble_on_encrypt_started_t)(uint16_t resp_status, rsi_bt_event_encryption_enabled_t *enc_enabled);
 * @brief Callback function an encrypted event from the module.
 *        This callback function will be called if the encryption process is started with remote device.
 *        It has to be registered using the `rsi_ble_smp_register_callbacks` API.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[out] resp_status contains the response status (Success or Error code)
 * @param[out] enc_enabled contains encryption information. Refer to \ref rsi_bt_event_encryption_enabled_s for more details. 
 */
typedef void (*rsi_ble_on_encrypt_started_t)(uint16_t resp_status, rsi_bt_event_encryption_enabled_t *enc_enabled);

/*==============================================*/
/**
 * @typedef void (*rsi_ble_on_sc_passkey_t)(rsi_bt_event_sc_passkey_t *sc_passkey);
 * @brief Callback function for a BLE Secure Connection passkey event from the module.
 *        This callback function is called whenever a BLE Secure Connection passkey event is received from the module.
 *        It has to be registered using the `rsi_ble_smp_register_callbacks` API.
 * 
 *         the Security Method used. Possible values:
 *            - `1`: **Just Works** – No user interaction required, suitable for devices without input/output capabilities.
 *            - `2`: **Passkey**    – Requires to enter or confirm a 6-digit passkey, which provides strong security.
 * 
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[out] sc_passkey contains LE SC Passkey information such as address of device involved in SMP process and the key required in pairing process. Refer to \ref rsi_bt_event_encryption_enabled_s for more details.
 */
typedef void (*rsi_ble_on_sc_passkey_t)(rsi_bt_event_sc_passkey_t *sc_passkey);

/** @} */
/**
 * @fn          rsi_bt_on_controller_logs_t
 * @brief       Callback function to be called when controller stats events are received from the module
 * @param[in]   resp_status, contains the response status (Success or Error code)
 * @param[in]   controller_logs, contains the controller stats parameters
 * @return      void
 * @section description
 * This callback function will be called when controller stats events are received from the module.
 * This callback has to be registered using rsi_bt_register_controller_stats_callbacks API.
 */
typedef void (*rsi_bt_on_controller_logs_t)(uint16_t resp_status, rsi_bt_event_controller_logs_t *controller_logs);

/** @addtogroup BT-LOW-ENERGY7
* @{
*/
/*==============================================*/
/**
 * @brief      Register GAP callbacks.
 * @param[in]  ble_on_adv_report_event                  - Callback function for Advertise events
 * @param[in]  ble_on_conn_status_event                 - Callback function for Connect events
 * @param[in]  ble_on_disconnect_event                  - Callback function for Disconnect events
 * @param[in]  timeout_expired_event                    - Callback function for LE ping timeout events
 * @param[in]  ble_on_phy_update_complete_event         - Callback function for PHY update complete events
 * @param[in]  ble_on_data_length_update_complete_event - Callback function for data length update events
 * @param[in]  ble_on_enhance_conn_status_event         - Callback function for enhanced connection status events
 * @param[in]  ble_on_directed_adv_report_event         - Callback function for directed advertising report events
 * @param[in]  ble_on_conn_update_complete_event        - Callback function for connection update complete events
 * @param[in]  ble_on_remote_conn_params_request_event  - Callback function to remote connection params request events
 * 
 */
void rsi_ble_gap_register_callbacks(rsi_ble_on_adv_report_event_t ble_on_adv_report_event,
                                    rsi_ble_on_connect_t ble_on_conn_status_event,
                                    rsi_ble_on_disconnect_t ble_on_disconnect_event,
                                    rsi_ble_on_le_ping_payload_timeout_t timeout_expired_event,
                                    rsi_ble_on_phy_update_complete_t ble_on_phy_update_complete_event,
                                    rsi_ble_on_data_length_update_t ble_on_data_length_update_complete_event,
                                    rsi_ble_on_enhance_connect_t ble_on_enhance_conn_status_event,
                                    rsi_ble_on_directed_adv_report_event_t ble_on_directed_adv_report_event,
                                    rsi_ble_on_conn_update_complete_t ble_on_conn_update_complete_event,
                                    rsi_ble_on_remote_conn_params_request_t ble_on_remote_conn_params_request_event);

/*==============================================*/
/**
 * @brief      Register GAP Extended responses/events callbacks.
 * @pre Pre-conditions:
 *        Call [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) before calling this API.
 * @param[in]  ble_on_remote_features_event  - Call back function for Remote feature request.
 * @param[in]  ble_on_le_more_data_req_event - Call back function for LE More data request.
 * @note        For details about each callback, refer to the GAP Extended callbacks description section.
 * 
 */

void rsi_ble_gap_extended_register_callbacks(rsi_ble_on_remote_features_t ble_on_remote_features_event,
                                             rsi_ble_on_le_more_data_req_t ble_on_le_more_data_req_event);

/**
 * @fn uint32_t rsi_ble_enhanced_gap_extended_register_callbacks(uint16_t callback_id,
 *                                   void (*callback_handler_ptr)(uint16_t status,
 *                                                  uint8_t *buffer));
 * @brief Registers a callback function for the Enhanced GAP Extended feature in the RSI BLE module.
 *
 * This function allows you to register a callback function to handle events related to the Enhanced GAP Extended feature.
 * The callback function will be called when an event occurs, providing the status and a buffer containing additional data.
 *
 * @param callback_id The ID of the callback to register.
 * @param callback_handler_ptr A pointer to the callback function that will handle the events.
 *
 * @return The status of the registration process. Possible values are defined by the uint16_t data type.
 * 
 * @note Ensure that the callback function is implemented to handle the specific events you are interested in.
 *       The callback function should match the signature specified in the function parameters.
 *       This function should be called during the initialization phase of your application to ensure that
 *       the callbacks are registered before any events occur.
 */
uint32_t rsi_ble_enhanced_gap_extended_register_callbacks(uint16_t callback_id,
                                                          void (*callback_handler_ptr)(uint16_t status,
                                                                                       uint8_t *buffer));

/*==============================================*/
/**
 * 
 */
/**
 * @fn         int32_t rsi_ble_adv_ext_events_register_callbacks(uint16_t callback_id,
 *                                       void (*callback_handler_ptr)(uint16_t status, uint8_t *buffer));
 * @brief Registers callback functions for extended advertising events in the BLE module.
 *
 * This function allows the application to register callback functions to handle extended advertising events
 * in the module. The callback is received when an extended advertising event occurs.
 *
 * @param callback_id The ID of the callback function to register.
 * @param callback_handler_ptr A pointer to the callback function that will handle the extended advertising events.
 *                             The callback function should have the following signature:
 *                             void callback_handler(uint16_t status, uint8_t *buffer)
 *                             - status: The status of the extended advertising event.
 *                             - buffer: A pointer to the buffer containing the extended advertising event data.
 *
 * @return Returns 0 on success, or a negative error code on failure.
 */
int32_t rsi_ble_adv_ext_events_register_callbacks(uint16_t callback_id,
                                                  void (*callback_handler_ptr)(uint16_t status, uint8_t *buffer));

/*==============================================*/
/**
 * @brief         Register the SMP callbacks
 * @param[in]     ble_on_smp_request_event           - smp request callback
 * @param[in]     ble_on_smp_response_event          - smp response callback
 * @param[in]	  ble_on_smp_passkey_event           - smp passkey callback
 * @param[in]	  ble_on_smp_fail_event            - smp failed callback
 * @param[in]	  rsi_ble_on_encrypt_started_event               - encryption enabled callback
 * @param[in]	  ble_on_smp_passkey_display_event   - smp passkey display callback
 * @param[in]	  ble_sc_passkey_event               - sc passkey display callback
 * @param[in]     ble_on_le_ltk_req_event            - This is the SMP LTK request callback
 * @param[in]     ble_on_le_security_keys_event      -  This is the SMP security keys callback
 * @param[in]	  ble_on_cli_smp_response_event        -  This is the client smp response callback    - 
 * @param[in]   ble_on_sc_method_event                    - sc method display callback
 * 
 */
void rsi_ble_smp_register_callbacks(rsi_ble_on_smp_request_t ble_on_smp_request_event,
                                    rsi_ble_on_smp_response_t ble_on_smp_response_event,
                                    rsi_ble_on_smp_passkey_t ble_on_smp_passkey_event,
                                    rsi_ble_on_smp_failed_t ble_on_smp_fail_event,
                                    rsi_ble_on_encrypt_started_t rsi_ble_on_encrypt_started_event,
                                    rsi_ble_on_smp_passkey_display_t ble_on_smp_passkey_display_event,
                                    rsi_ble_on_sc_passkey_t ble_sc_passkey_event,
                                    rsi_ble_on_le_ltk_req_event_t ble_on_le_ltk_req_event,
                                    rsi_ble_on_le_security_keys_t ble_on_le_security_keys_event,
                                    rsi_ble_on_smp_response_t ble_on_cli_smp_response_event,
                                    rsi_ble_on_sc_method_t ble_on_sc_method_event);

/** @} */

/******************************************************
 * *       BLE GATT Response Callbacks Declarations
 * ******************************************************/
/** @addtogroup BT-LOW-ENERGY8
* @{
*/
/**
 * @typedef    void (*rsi_ble_on_gatt_error_resp_t)(uint16_t event_status, rsi_ble_event_error_resp_t *rsi_ble_gatt_error);
 * @brief Callback function for a GATT error event from the module.
 *
 * This callback function is called whenever a GATT error event is received from the module.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  event_status contains the error response  
 *              Non-Zero Value	-	Failure 
 *              Attribute protocol error codes 
 *              0x4A01  -  Invalid Handle 
 *              0x4A06  -  Request not supported 
 *              0x4A0A  -  Attribute not found 
 *              0x4A05  -  Insufficient authentication 
 *              0x4A08  -  Insufficient authorization 
 *              0x4A0C  -  Insufficient encryption key size 
 *              0x4A0F  -  Insufficient encryption 
 * 				      0x4A02  -  Read not permitted 
 *              0x4A03  -  Write not permitted 
 *              0x4A07  -  Invalid offset 
 *              0x4A0B  -  Attribute not Long (Attribute value has a fixed length that is less than or equal to (ATT_MTU - 1) octets in length , refer to Bluetooth Spec 5.4 for further details.)
 * @param[out]  rsi_ble_gatt_error contains the GATT error information. Refer to rsi_ble_event_error_resp_s for more details.
 * @return The following values are returned:
 *      void
 * 
 */
typedef void (*rsi_ble_on_gatt_error_resp_t)(uint16_t event_status, rsi_ble_event_error_resp_t *rsi_ble_gatt_error);
/**
 * @typedef void (*rsi_ble_on_gatt_desc_val_event_t)(uint16_t event_status,
                                                 rsi_ble_event_gatt_desc_t *rsi_ble_gatt_desc_val);
 * @brief Callback function for an attribute descriptors event from the module.
 *
 * This callback function is called whenever an attribute descriptors event is received from the module.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  event_status contains the response status 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 *
 * @param[out]  rsi_ble_gatt_desc_val contains the profiles list event information. Refer to rsi_ble_event_gatt_desc_s for more details.
 * @return The following values are returned:
 *      void
 * 
 */
typedef void (*rsi_ble_on_gatt_desc_val_event_t)(uint16_t event_status,
                                                 rsi_ble_event_gatt_desc_t *rsi_ble_gatt_desc_val);

/**
 * @typedef void (*rsi_ble_on_event_profiles_list_t)(uint16_t event_status,
                                                 rsi_ble_event_profiles_list_t *rsi_ble_event_profiles);
 * @brief Callback function for a profiles list response from the module.
 *
 * This callback function is called whenever a profiles list response is received from the module.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  event_status contains the response status 
 * 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 *
 * @param[out]  rsi_ble_event_profiles contains the profiles list event information. Refer to rsi_ble_event_profiles_list_s for more details.
 * @return The following values are returned:
 *      void
 * 
 *  
 * 
 */
typedef void (*rsi_ble_on_event_profiles_list_t)(uint16_t event_status,
                                                 rsi_ble_event_profiles_list_t *rsi_ble_event_profiles);

/**
 * @typedef void (*rsi_ble_on_event_profile_by_uuid_t)(uint16_t event_status,
                                                   rsi_ble_event_profile_by_uuid_t *rsi_ble_event_profile);
 * @brief Callback function for a profile response from the module.
 *
 * This callback function is called whenever a profile response is received from the module.
 * It provides the particular profile information.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  event_status contains the response status 
 * 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 *
 * @param[out]  rsi_ble_event_profile contains the profile response information. Refer to \ref rsi_ble_event_profile_by_uuid_s for more details.
 * @return The following values are returned:
 *      void
 * 
 *  
 * 
 */
typedef void (*rsi_ble_on_event_profile_by_uuid_t)(uint16_t event_status,
                                                   rsi_ble_event_profile_by_uuid_t *rsi_ble_event_profile);
/**
 * @typedef void (*rsi_ble_on_event_read_by_char_services_t)(uint16_t event_status,
                                                         rsi_ble_event_read_by_type1_t *rsi_ble_event_read_type1);
 * @brief      This callback function will be called if the characteristic services list response is received from the module.
 * @param[out]  event_status contains the response status 
 * 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 * @param[out]  rsi_ble_event_read_type1 contains the char services event information. Refer to rsi_ble_event_read_by_type1_s for more details.
 * @return The following values are returned:
 *      void
 * 
 * 
 */
typedef void (*rsi_ble_on_event_read_by_char_services_t)(uint16_t event_status,
                                                         rsi_ble_event_read_by_type1_t *rsi_ble_event_read_type1);

/**
 * @typedef void (*rsi_ble_on_event_read_by_inc_services_t)(uint16_t event_status,
                                                        rsi_ble_event_read_by_type2_t *rsi_ble_event_read_type2);
 * @brief Callback function for an included services list response from the module.
 *
 * This callback function is called whenever an included services list response is received from the module.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  event_status contains the response status 
 * 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 * @param[out]  rsi_ble_event_read_type2 includes the services event information. Refer to \ref rsi_ble_event_read_by_type2_s for details.
 * @return The following values are returned:
 *      void
 * 
 *  
 */
typedef void (*rsi_ble_on_event_read_by_inc_services_t)(uint16_t event_status,
                                                        rsi_ble_event_read_by_type2_t *rsi_ble_event_read_type2);
/**
 * @typedef void (*rsi_ble_on_event_read_att_value_t)(uint16_t event_status,
                                                  rsi_ble_event_read_by_type3_t *rsi_ble_event_read_type3);
 * @brief Callback function for an attribute value response from the module.
 *
 * This callback function is called whenever an attribute value response is received from the module.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  event_status contains the response status 
 * 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure
 * @param[out]  rsi_ble_event_read_type3 includes the attribute value information. Refer to \ref rsi_ble_event_read_by_type3_s for details.
 * @return The following values are returned:
 *      void
 * 
 */
typedef void (*rsi_ble_on_event_read_att_value_t)(uint16_t event_status,
                                                  rsi_ble_event_read_by_type3_t *rsi_ble_event_read_type3);

/**
 * @typedef void (*rsi_ble_on_event_read_resp_t)(uint16_t event_status, rsi_ble_event_att_value_t *rsi_ble_event_att_val);
 * @brief Callback function for an attribute value from the module.
 *
 * This callback function is called whenever an attribute value is received from the module.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  event_status contains the response status 
 * 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 * @param[out]  rsi_ble_event_att_val contains the profile response information. Refer to rsi_ble_event_att_value_s for more details.
 * @return The following values are returned:
 *      void
 * 
 */
typedef void (*rsi_ble_on_event_read_resp_t)(uint16_t event_status, rsi_ble_event_att_value_t *rsi_ble_event_att_val);

/**
 * @typedef    void (*rsi_ble_on_event_write_resp_t)(uint16_t event_status, rsi_ble_set_att_resp_t *rsi_ble_event_set_att_rsp);
 * @brief Callback function for a GATT write response from the module.
 *
 * \ref rsi_ble_set_att_resp_t holds the response for the set attribute request that is, it specifically includes the remote device address which intiates the write request.
 * This callback function is called whenever a GATT write response is received from the module.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  event_status contains the response status 
 * 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 *
 * @param[out]  rsi_ble_event_set_att_rsp includes the response of the set attribute request information. Refer to \ref rsi_ble_set_att_resp_t for details.
 * @return The following values are returned:
 *      void
 * 
 */
typedef void (*rsi_ble_on_event_write_resp_t)(uint16_t event_status, rsi_ble_set_att_resp_t *rsi_ble_event_set_att_rsp);
/**
 * @typedef void (*rsi_ble_on_event_indicate_confirmation_t)(uint16_t event_status,
                                                         rsi_ble_set_att_resp_t *rsi_ble_event_set_att_rsp);
 * @brief      Callback function for indication confirmation event
 *
 * \ref rsi_ble_set_att_resp_t holds the response for the set attribute request, specifically includes the remote device address which intiates the write request.
 * This callback function is called whenever an indication confirmation response is received from the module.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  event_status contains the response status 
 * 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 * @param[out]  rsi_ble_event_set_att_rsp includes the response for the set attribute request information. Refer to \ref rsi_ble_set_att_resp_s for details.
 * @return The following values are returned:
 *      void
 * 
 */
typedef void (*rsi_ble_on_event_indicate_confirmation_t)(uint16_t event_status,
                                                         rsi_ble_set_att_resp_t *rsi_ble_event_set_att_rsp);
/**
 * @typedef void (*rsi_ble_on_event_prepare_write_resp_t)(uint16_t event_status,
                                                      rsi_ble_prepare_write_resp_t *rsi_ble_event_prepare_write);
 * @brief Callback function for a GATT prepare write response from the module.
 *
 * This callback function is called whenever a GATT prepare response is received from the module.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  event_status contains the response status 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 * @param[out]  rsi_ble_event_prepare_write Pointer to structure \ref rsi_ble_prepare_write_resp_s includes write response information such as:  attribute value, length of attribute, and  attribute handle.
 * @return The following values are returned:
 *      void
 * 
 */
typedef void (*rsi_ble_on_event_prepare_write_resp_t)(uint16_t event_status,
                                                      rsi_ble_prepare_write_resp_t *rsi_ble_event_prepare_write);

/**
 * @typedef void (*rsi_ble_on_profiles_list_resp_t)(uint16_t resp_status,
                                                rsi_ble_resp_profiles_list_t *rsi_ble_resp_profiles);
 * @brief Callback function for a profiles list response from the module.
 *
 * This callback function is called whenever a profiles list response is received from the module.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  resp_status contains the response status 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 * @note        Attribute protocol error codes 
 *              0x4A01  -  Invalid Handle 
 *              0x4A0A  -  Attribute not found 
 * @param[out]  rsi_ble_resp_profiles contains the profiles list response information. Refer to rsi_ble_resp_profiles_list_s for more details.
 * @return The following values are returned:
 *      void
 * 
 */
typedef void (*rsi_ble_on_profiles_list_resp_t)(uint16_t resp_status,
                                                rsi_ble_resp_profiles_list_t *rsi_ble_resp_profiles);

/**
 * @typedef void (*rsi_ble_on_profile_resp_t)(uint16_t resp_status, profile_descriptors_t *rsi_ble_resp_profile);
 * @brief Callback function for a profile response from the module.
 *
 * This callback function is called whenever a profile response is received from the module.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  resp_status contains the response status 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 * @note        Attribute protocol error codes 
 *              0x4A01  -  Invalid Handle 
 *              0x4A06  -  Request not supported 
 *              0x4A0A  -  Attribute not found 
 * @param[out]  rsi_ble_resp_profile contains the profile response information. Refer to profile_descriptors_s for more details.
 * @return The following values are returned:
 *      void
 * 
 */
typedef void (*rsi_ble_on_profile_resp_t)(uint16_t resp_status, profile_descriptors_t *rsi_ble_resp_profile);

/**
 * @typedef void (*rsi_ble_on_char_services_resp_t)(uint16_t resp_status,
                                                rsi_ble_resp_char_services_t *rsi_ble_resp_char_serv);
 * @brief Callback function for a service characteristics response from the module.
 *
 * This callback function is called whenever a service characteristics response is received from the module.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  resp_status contains the response status 
 * 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 * @note        Attribute protocol error codes 
 *              0x4A01  -  Invalid Handle 
 *              0x4A06  -  Request not supported 
 *              0x4A0A  -  Attribute not found 
 *              0x4A05  -  Insufficient authentication 
 *              0x4A08  -  Insufficient authorization 
 *              0x4A0C  -  Insufficient encryption key size 
 *              0x4A0F  -  Insufficient encryption 
 * 				      0x4A02  -  Read not permitted 
 * @param[out]  rsi_ble_resp_char_serv contains the service characteristics response information. Refer to \ref rsi_ble_resp_char_services_s for more details.
 * @return The following values are returned:
 *      void
 */
typedef void (*rsi_ble_on_char_services_resp_t)(uint16_t resp_status,
                                                rsi_ble_resp_char_services_t *rsi_ble_resp_char_serv);

/**
 * @typedef void (*rsi_ble_on_inc_services_resp_t)(uint16_t resp_status,
                                               rsi_ble_resp_inc_services_t *rsi_ble_resp_inc_serv);
 * @brief Callback function for included services response from the module.
 *
 * This callback function is triggered whenever an included services response is received from the module.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  resp_status contains the response status 
 * 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 * @note        Attribute protocol error codes 
 *              0x4A01  -  Invalid Handle 
 *              0x4A06  -  Request not supported 
 *              0x4A0A  -  Attribute not found 
 *              0x4A05  -  Insufficient authentication 
 *              0x4A08  -  Insufficient authorization 
 *              0x4A0C  -  Insufficient encryption key size 
 *              0x4A0F  -  Insufficient encryption 
 * 				      0x4A02  -  Read not permitted 
 * @param[out]  rsi_ble_resp_inc_serv includes the services response information. Refer to \ref rsi_ble_resp_inc_services_s for details.
 * @return The following values are returned:
 *      void
 * 
 *  
 *
 */
typedef void (*rsi_ble_on_inc_services_resp_t)(uint16_t resp_status,
                                               rsi_ble_resp_inc_services_t *rsi_ble_resp_inc_serv);

/**
 * @typedef     void (*rsi_ble_on_att_desc_resp_t)(uint16_t resp_status, rsi_ble_resp_att_descs_t *rsi_ble_resp_att_desc);
 * @brief Callback function for an attribute descriptors response from the module.
 *
 * This callback function is called whenever an attribute descriptors response is received from the module.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  resp_status contains the response status 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 * @note        Attribute protocol error codes 
 *              0x4A01  -  Invalid Handle 
 *              0x4A0A  -  Attribute not found 
 *              0x4A05  -  Insufficient authentication 
 *              0x4A08  -  Insufficient authorization 
 *              0x4A0C  -  Insufficient encryption key size 
 *              0x4A0F  -  Insufficient encryption 
 * 				      0x4A02  -  Read not permitted 
 * @param[out]  rsi_ble_resp_att_desc contains the attribute descriptors response information. Refer to rsi_ble_resp_att_descs_s for more details. 
 * @return The following values are returned:
 *      void
 * 
 */
typedef void (*rsi_ble_on_att_desc_resp_t)(uint16_t resp_status, rsi_ble_resp_att_descs_t *rsi_ble_resp_att_desc);

/**
 * @typedef void (*rsi_ble_on_read_resp_t)(uint16_t resp_status,
                                       uint16_t resp_id,
                                       rsi_ble_resp_att_value_t *rsi_ble_resp_att_val);
 * @brief Callback function for attribute read response from the module.
 *
 * This callback function is called upon receiving an attribute value from the module.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  resp_status contains the response status 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 * @note        Attribute protocol error codes 
 *              0x4A01  -  Invalid Handle 
 *              0x4A0A  -  Attribute not found 
 *              0x4A05  -  Insufficient authentication 
 *              0x4A08  -  Insufficient authorization 
 *              0x4A0C  -  Insufficient encryption key size 
 *              0x4A0F  -  Insufficient encryption 
 * 				      0x4A02  -  Read not permitted 
 *              0x4A06  -  Request not supported 
 *              0x4A07  -  Invalid offset 
 *              0x4A0B  -  Attribute not Long - attribute value has a fixed length that is less than or equal to (ATT_MTU - 1) octets in length , refer to Bluetooth Spec 5.4 for further details.
 * @param[out]  resp_id contains the response id because of which, this callback is called
 *                      response ids: (RSI_BLE_RSP_READ_VAL, RSI_BLE_RSP_READ_BY_UUID, RSI_BLE_RSP_LONG_READ, RSI_BLE_RSP_MULTIPLE_READ)
 * @param[out]  rsi_ble_resp_att_val contains the attribute value on attribute read response. Refer to \ref rsi_ble_resp_att_value_s for more details.
 * @return The following values are returned:
 *      void
 * 
 * 
 */
typedef void (*rsi_ble_on_read_resp_t)(uint16_t resp_status,
                                       uint16_t resp_id,
                                       rsi_ble_resp_att_value_t *rsi_ble_resp_att_val);

/**
 * @typedef void (*rsi_ble_on_write_resp_t)(uint16_t resp_status, uint16_t resp_id);
 * @brief Callback function for attribute write response such as set/prepare/execute action completion.
 *
 * This callback function is called when an attribute set, prepare, or execute action is completed.
 * It has to be registered using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out]  resp_status contains the response status 
 * 
 *              - 0 - Success 
 *              - Non-Zero Value	-	Failure 
 * @note        Attribute protocol error codes 
 *              0x4A01  -  Invalid Handle
 *              0x4A0A  -  Attribute not found 
 *              0x4A05  -  Insufficient authentication 
 *              0x4A08  -  Insufficient authorization 
 *              0x4A0C  -  Insufficient encryption key size 
 *              0x4A0F  -  Insufficient encryption 
 * 				      0x4A03  -  Write not permitted 
 *              0x4A07  -  Invalid offset  
 *              0x4A0D  -  Invalid attribute value length 
 * @param[out]  resp_id contains the response id because of which, this callback is called
 *                      response ids: (RSI_BLE_RSP_WRITE, RSI_BLE_RSP_WRITE_NO_ACK, RSI_BLE_RSP_LONG_WRITE, RSI_BLE_RSP_EXECUTE_WRITE)
 * 
 */
typedef void (*rsi_ble_on_write_resp_t)(uint16_t resp_status, uint16_t resp_id);
/** @} */

/******************************************************
 * *       BLE GATT Event Callbacks Declarations
 * ******************************************************/
/** @addtogroup BT-LOW-ENERGY8
* @{
*/
/**
 * @typedef void (*rsi_ble_on_gatt_write_event_t)(uint16_t event_id, rsi_ble_event_write_t *rsi_ble_write);
 * @brief      Callback function are triggered for GATT write command, write request, notification, and event indication from the module.
 *             It is registered by using the `rsi_ble_gatt_register_callbacks` API.
 * @param[out] event_id includes the event id (RSI_BLE_EVENT_GATT_NOTIFICATION,  RSI_BLE_EVENT_GATT_INDICATION, RSI_BLE_EVENT_GATT_WRITE)
 * @param[out] rsi_ble_write Pointer to a structure of type `rsi_ble_event_write_t` includes the detailed event information such as
 *                           dev_addr (Address of the remote device) , dev_addr (Type of the event received) , handle (Attribute handle associated with the event)
 *                           length (length of attribute value) , att_value (the attribute value). Refer to \ref rsi_ble_event_write_s for more details.
 * 
 */
typedef void (*rsi_ble_on_gatt_write_event_t)(uint16_t event_id, rsi_ble_event_write_t *rsi_ble_write);

/**
 * @typedef void (*rsi_ble_on_gatt_prepare_write_event_t)(uint16_t event_id, rsi_ble_event_prepare_write_t *rsi_ble_write);
 * @brief      This callback function will be called if the GATT write/notify/indicate events are received
 *             This callback has to be registered using rsi_ble_gatt_register_callbacks API.
 * @param[in]  event_id contains the event id (RSI_BLE_EVENT_GATT_NOTIFICATION,  RSI_BLE_EVENT_GATT_INDICATION, RSI_BLE_EVENT_GATT_WRITE)
 * @param[in]  rsi_ble_write  Pointer to a \ref rsi_ble_event_prepare_write_s structure containing detailed information
 *                            about the GATT prepare write event. This includes the remote device address, attribute handle, 
 *                            value offset, length of the value, and the attribute value itself. Refer to \ref rsi_ble_event_prepare_write_s for more details.
 * 
 */
typedef void (*rsi_ble_on_gatt_prepare_write_event_t)(uint16_t event_id, rsi_ble_event_prepare_write_t *rsi_ble_write);

/**
 * @typedef void (*rsi_ble_on_execute_write_event_t)(uint16_t event_id, rsi_ble_execute_write_t *rsi_ble_execute_write);
 * @brief      The callback function will be called if the GATT write execute events are received.
 *             This callback has to be registered using rsi_ble_gatt_register_callbacks API.
 * @param[out]  event_id contains the gatt_execute_write event id (RSI_BLE_EVENT_EXECUTE_WRITE) 
 *
 * @param[out]  rsi_ble_execute_write Pointer to a \ref rsi_ble_execute_write_s structure containing details
 *                                    about the execute write request. This includes the remote device address 
 *                                    and the execute write flag.
 */
typedef void (*rsi_ble_on_execute_write_event_t)(uint16_t event_id, rsi_ble_execute_write_t *rsi_ble_execute_write);

/**
 * @typedef    void (*rsi_ble_on_read_req_event_t)(uint16_t event_id, rsi_ble_read_req_t *rsi_ble_read_req);
 * @brief      The callback function will be called if the GATT read request events are received.
 *             This callback has to be registered using rsi_ble_gatt_register_callbacks API.
 * @param[out]  event_id contains the gatt_read_req_event id (RSI_BLE_EVENT_READ_REQ) 
 * @param[out]  rsi_ble_read_req Pointer to a \ref rsi_ble_read_req_s structure containing details about the
 *                               GATT read request. This includes the remote device address, attribute handle, 
 *                               request type, and the offset of the attribute value.
 * 
 */
typedef void (*rsi_ble_on_read_req_event_t)(uint16_t event_id, rsi_ble_read_req_t *rsi_ble_read_req);

/**
 * @typedef void (*rsi_ble_on_mtu_event_t)(rsi_ble_event_mtu_t *rsi_ble_event_mtu);
 * @brief      The callback function will be called if MTU size request is received. 
 *             The MTU size negotiation occurs after establishing a connection, and the callback provides
 *             the updated MTU size information.
 * @param[out]  rsi_ble_event_mtu  contains the MTU size information. Refer to \ref rsi_ble_event_mtu_s for more details.
 * 
 */
typedef void (*rsi_ble_on_mtu_event_t)(rsi_ble_event_mtu_t *rsi_ble_event_mtu);
/** @addtogroup BT-LOW-ENERGY9
 * @{
 */
/**
 * @typedef    void (*rsi_ble_on_mtu_exchange_info_t)(rsi_ble_event_mtu_exchange_information_t *rsi_ble_event_mtu_exchange_info);
 * @brief      Callback function called to indicate MTU exchange details.
 *             This callback provides information about the MTU size negotiation and indicates
 *             the initiator of the MTU exchange process (local or remote device).
 *             It is triggered during the MTU exchange process after establishing a BLE connection.
 *             This callback has to be registered using rsi_ble_gatt_extended_register_callbacks API.
 * @param[out]  rsi_ble_event_mtu_exchange_info contains the MTU exchange information. Refer to \ref rsi_ble_event_mtu_exchange_information_s for more details.
 * @note        When services are maintained on the host, this API need to be triggered by the application.
 * 
 */
typedef void (*rsi_ble_on_mtu_exchange_info_t)(
  rsi_ble_event_mtu_exchange_information_t *rsi_ble_event_mtu_exchange_info);
/** @} */ // End of ATT_MAINTAIN_IN_HOST Group
/**
 * @typedef    void (*rsi_ble_on_remote_device_info_t)(uint16_t status, rsi_ble_event_remote_device_info_t *resp_buffer);
 * @brief      Callback function to retrieve remote device information.it provides the version and manufacturer-specific details of a remote device.
 *             This callback has to be registered using rsi_ble_enhanced_gap_extended_register_callbacks API.
 * @param[out]  status contains the response status (Success or Error code) 
 *               - 0 - SUCCESS 
 *               - Non-Zero Value - ErrorCodes
 * @note        Refer Bluetooth Generic Error Codes section  up to 0x4FF8 from error-codes.
 * @param[out] resp_buffer Pointer to a \ref rsi_ble_event_remote_device_info_s structure containing 
 *                         remote device version information, including the version, company ID, 
 *                         and sub-version.
 *
 */
typedef void (*rsi_ble_on_remote_device_info_t)(uint16_t status, rsi_ble_event_remote_device_info_t *resp_buffer);
/**
 * @file rsi_ble_apis.h
 * @brief This file contains the declarations of the BLE API functions.
 */

/**
 * @typedef void (*rsi_ble_on_rcp_resp_rcvd_t)(uint16_t status, rsi_ble_event_rcp_rcvd_info_t *resp_buffer);
 * @brief Callback function type for receiving RCP response events.
 * @param status The status of the RCP response event.
 * @param resp_buffer Pointer to the buffer containing the RCP response information.
 * 
 */
typedef void (*rsi_ble_on_rcp_resp_rcvd_t)(uint16_t status, rsi_ble_event_rcp_rcvd_info_t *resp_buffer);

/*********************************************************************************
 * * BLE L2CAP Credit based flow control(CBFC) Callbacks register function Declarations
 * *******************************************************************************/

/**
 * @typedef void (*rsi_ble_on_cbfc_conn_req_event_t)(rsi_ble_event_cbfc_conn_req_t *rsi_ble_cbfc_conn_req);
 * @brief      Callback function to handle an L2CAP CBFC connection request event.
 *             This callback function is called when a Circular Buffer Flow Control (CBFC) 
 *             connection request is received from a remote BLE device. It provides the 
 *             necessary connection parameters to process the request.
 *             This callback has to be registered using rsi_ble_l2cap_cbfc_callbacks API.
 * @param[in]  rsi_ble_cbfc_conn_req  contains the connection request information , refer to \ref rsi_ble_event_cbfc_conn_req_s for more details.
 *
 */
typedef void (*rsi_ble_on_cbfc_conn_req_event_t)(rsi_ble_event_cbfc_conn_req_t *rsi_ble_cbfc_conn_req);

/**
 * @typedef void (*rsi_ble_on_cbfc_conn_complete_event_t)(rsi_ble_event_cbfc_conn_complete_t *rsi_ble_cbfc_conn_complete,
 *                                                     uint16_t status);
 * @brief      Callback function to indicate L2CAP CBFC connection complete status. 
 *             This callback function will be called when Circular Buffer Flow Control (CBFC) connection
 *             has been successfully established or when the connection attempt fails. The function
 *             provides detailed information about the connection parameters and the status of the operation. 
 *             This callback has to be registered using rsi_ble_l2cap_cbfc_callbacks API.
 * @param[in]  status The status of the connection operation:
 *                     - **0**: SUCCESS
 *                     - **Non-Zero Value**: Error code indicating the reason for the failure.
 * @param[in]  rsi_ble_cbfc_conn_complete  contains the connection completed information such as psm , mtu , mps , lcid. Refer to \ref rsi_ble_event_cbfc_conn_complete_s for more details.
 * 
 * 
 * 
 */
typedef void (*rsi_ble_on_cbfc_conn_complete_event_t)(rsi_ble_event_cbfc_conn_complete_t *rsi_ble_cbfc_conn_complete,
                                                      uint16_t status);

/**
 * @typedef void (*rsi_ble_on_cbfc_rx_data_event_t)(rsi_ble_event_cbfc_rx_data_t *rsi_ble_cbfc_rx_data);
 * @brief      Callback function to indicate L2CAP CBFC RX data event. 
 *             This callback function will be called when data is received over a Circular Buffer Flow Control (CBFC)
 *             connection from a remote device.
 *             This callback has to be registered using rsi_ble_l2cap_cbfc_callbacks API.
 * @param[in]  rsi_ble_cbfc_rx_data  contains the received data information , refer to \ref rsi_ble_event_cbfc_rx_data_s
 * 
 */
typedef void (*rsi_ble_on_cbfc_rx_data_event_t)(rsi_ble_event_cbfc_rx_data_t *rsi_ble_cbfc_rx_data);

/**
 * @typedef void (*rsi_ble_on_cbfc_disconn_event_t)(rsi_ble_event_cbfc_disconn_t *rsi_ble_cbfc_disconn);
 * @brief      Callback function to indicate L2CAP CBFC disconnection event.
 *             This callback function will be called whenever a Circular Buffer Flow Control (CBFC) 
 *             disconnection event occurs.
 *             This callback has to be registered using rsi_ble_l2cap_cbfc_callbacks API.
 * @param[in]  rsi_ble_cbfc_disconn  contains the disconnect device information , refer to \ref rsi_ble_event_cbfc_disconn_s.
 */
typedef void (*rsi_ble_on_cbfc_disconn_event_t)(rsi_ble_event_cbfc_disconn_t *rsi_ble_cbfc_disconn);

/**
 * @fn void rsi_ble_l2cap_cbsc_register_callbacks(rsi_ble_on_cbfc_conn_req_event_t ble_on_cbsc_conn_req,
 *                       rsi_ble_on_cbfc_conn_complete_event_t ble_on_cbsc_conn_complete,
 *                       rsi_ble_on_cbfc_rx_data_event_t ble_on_cbsc_rx_data,
 *                       rsi_ble_on_cbfc_disconn_event_t ble_on_cbsc_disconn);
 * @brief Register callbacks for BLE L2CAP Credit Based Flow Control (CBFC) events.
 *
 * This function is used to register callbacks for various CBFC events in BLE L2CAP.
 * The registered callbacks will be invoked when the corresponding events occur.
 *
 * @param ble_on_cbsc_conn_req Callback function to handle CBFC connection request event.
 * @param ble_on_cbsc_conn_complete Callback function to handle CBFC connection complete event.
 * @param ble_on_cbsc_rx_data Callback function to handle CBFC receive data event.
 * @param ble_on_cbsc_disconn Callback function to handle CBFC disconnection event.
 */
void rsi_ble_l2cap_cbsc_register_callbacks(rsi_ble_on_cbfc_conn_req_event_t ble_on_cbsc_conn_req,
                                           rsi_ble_on_cbfc_conn_complete_event_t ble_on_cbsc_conn_complete,
                                           rsi_ble_on_cbfc_rx_data_event_t ble_on_cbsc_rx_data,
                                           rsi_ble_on_cbfc_disconn_event_t ble_on_cbsc_disconn);

/**
 * @typedef void (*chip_ble_buffers_stats_handler_t)(chip_ble_buffers_stats_t *chip_ble_buffers_stats);
 * @brief Typedef for the chip_ble_buffers_stats_handler_t function pointer.
 *
 * This function pointer type is used to define a callback function that handles
 * the statistics of the BLE buffers in the chip.
 *
 * @param chip_ble_buffers_stats Pointer to the chip_ble_buffers_stats_t structure
 *                               that contains the statistics of the BLE buffers.
 */
typedef void (*chip_ble_buffers_stats_handler_t)(chip_ble_buffers_stats_t *chip_ble_buffers_stats);

/*********************************************************************************
 * * BLE AE EVENTS Callbacks register function Declarations
 * *******************************************************************************/
/**
 * @typedef    void (*rsi_ble_ae_report_complete_t)(uint16_t resp_status, rsi_ble_ae_adv_report_t *rsi_ble_event_ae_report);
 * @brief      Callback function to report the AE Advertisements.
 *             This callback function will be called when AE adv report event is received.
 *             This callback has to be registered using rsi_ble_ae_events_register_callbacks API.
 * @param[out] rsi_ble_event_ae_report  A pointer to the structure containing AE Adv packets information.
 *                                      It contains important information such PHY rate, Set ID (SID) , event type etc. Refer to \ref rsi_ble_ae_adv_report_s for more details.
 * @param[out] resp_status Status of the event response:
 *                         - 0: SUCCESS
 *                         - Non-zero: Error code
 */
typedef void (*rsi_ble_ae_report_complete_t)(uint16_t resp_status, rsi_ble_ae_adv_report_t *rsi_ble_event_ae_report);

/**
 * @typedef    void (*rsi_ble_ae_per_adv_sync_estbl_t)(uint16_t resp_status,
 *                                                     rsi_ble_per_adv_sync_estbl_t *rsi_ble_event_per_adv_sync_estbl);
 * @brief      Callback function to report the AE periodic sync established event.
 *             This callback function will be called when AE periodic sync established event is received.
 *             This callback has to be registered using rsi_ble_ae_events_register_callbacks API.
 * @param[out] rsi_ble_event_per_adv_sync_estbl  A pointer to the structure containing AE periodic sync established information.
 *                                               Such as status of sync indicating whether Periodic Advertising is successful or not. Refer to \ref rsi_ble_per_adv_sync_estbl_s for more details.
 * @param[out] resp_status Status of the event response:
 *                         - 0: SUCCESS
 *                         - Non-zero: Error code
 */
typedef void (*rsi_ble_ae_per_adv_sync_estbl_t)(uint16_t resp_status,
                                                rsi_ble_per_adv_sync_estbl_t *rsi_ble_event_per_adv_sync_estbl);

/**
 * @typedef   void (*rsi_ble_ae_per_adv_report_t)(uint16_t resp_status,
 *                                          rsi_ble_per_adv_report_t *rsi_ble_event_per_adv_report);
 * @brief      Callback function to report the AE periodic advertisement event
 *             This callback function will be called when AE periodic advertisement event is received
 *             This callback has to be registered using rsi_ble_ae_events_register_callbacks API
 * @param[out] rsi_ble_event_per_adv_report A pointer to the structure containing AE periodic advertisement, report information such as sync_handle, tx_power, RSSI, data etc. Refer to \ref rsi_ble_per_adv_report_s for more details.
 * @param[out] resp_status Status of the event response:
 *                         - 0: SUCCESS
 *                         - Non-zero: Error code
 */
typedef void (*rsi_ble_ae_per_adv_report_t)(uint16_t resp_status,
                                            rsi_ble_per_adv_report_t *rsi_ble_event_per_adv_report);

/**
 * @typedef   void (*rsi_ble_ae_per_adv_sync_lost_t)(uint16_t resp_status,
 *                                                   rsi_ble_per_adv_sync_lost_t *rsi_ble_event_per_adv_sync_lost);
 * @brief      Callback function to report the AE periodic sync lost event
 *             This callback function will be called when AE periodic sync lost event is received
 *             This callback has to be registered using rsi_ble_ae_events_register_callbacks API
 * @param[out] rsi_ble_event_per_adv_sync_lost  A pointer to the structure containing AE periodic sync lost information of sync handle on which adv sync is lost , refer to  \ref rsi_ble_per_adv_sync_lost_s for more details.
 * @param[out] resp_status Status of the event response:
 *                         - 0: SUCCESS
 *                         - Non-zero: Error code
 */
typedef void (*rsi_ble_ae_per_adv_sync_lost_t)(uint16_t resp_status,
                                               rsi_ble_per_adv_sync_lost_t *rsi_ble_event_per_adv_sync_lost);

/**
 * @typedef    void (*rsi_ble_ae_scan_timeout_t)(uint16_t resp_status, rsi_ble_scan_timeout_t *rsi_ble_event_scan_timeout);
 * @brief      Callback function to report the AE scan timeout event
 *             This callback function will be called when AE scan timeout event is received
 *             This callback has to be registered using rsi_ble_ae_events_register_callbacks API
 * @param[out] rsi_ble_event_scan_timeout  A pointer to the structure containing AE scan timeout information.
 *                                         Such as status indicating that scanning has ended because the duration has expired, refer to \ref rsi_ble_scan_timeout_s for more details.
 * @param[out] resp_status Status of the event response:
 *                         - 0: SUCCESS
 *                         - Non-zero: Error code
 */
typedef void (*rsi_ble_ae_scan_timeout_t)(uint16_t resp_status, rsi_ble_scan_timeout_t *rsi_ble_event_scan_timeout);

/**
 * @typedef    void (*rsi_ble_ae_adv_set_terminated_t)(uint16_t resp_status,
 *                                                     rsi_ble_adv_set_terminated_t *rsi_ble_event_adv_set_terminated);
 * @brief      Callback function to report the AE advertising set terminated event
 *             This callback function will be called when AE advertising set terminated is received.
 *             This callback has to be registered using rsi_ble_ae_events_register_callbacks API.
 * @param[out] rsi_ble_event_adv_set_terminated  A pointer to the structure containing the AE advertising set terminated information about status on advertisement termination,
 *                                               adv handle, connection handle, number of AE events. Refer to \ref rsi_ble_adv_set_terminated_s for more details.
 * @param[out] resp_status Status of the event response:
 *                         - 0: SUCCESS
 *                         - Non-zero: Error code 
 */
typedef void (*rsi_ble_ae_adv_set_terminated_t)(uint16_t resp_status,
                                                rsi_ble_adv_set_terminated_t *rsi_ble_event_adv_set_terminated);

/**
 * @typedef    void (*rsi_ble_ae_scan_req_recvd_t)(uint16_t resp_status,
 *                                                rsi_ble_scan_req_recvd_t *rsi_ble_event_scan_req_recvd);
 * @brief      Callback function to report the AE scan request received event
 *             This callback function will be called when AE scan request is received. it provides AE scan request information.
 *             This callback has to be registered using rsi_ble_ae_events_register_callbacks API
 * @param[out] rsi_ble_event_scan_req_recvd A pointer to the structure containing AE scan request information about the received scan request, including the advertising set
 *             handle and scanner details. Refer to \ref rsi_ble_scan_req_recvd_s for detailed information
 *                                          
 * @param[out] resp_status Status of the event response:
 *                         - 0: SUCCESS
 *                         - Non-zero: Error code
 */
typedef void (*rsi_ble_ae_scan_req_recvd_t)(uint16_t resp_status,
                                            rsi_ble_scan_req_recvd_t *rsi_ble_event_scan_req_recvd);
/** @} */

/******************************************************
 * * BLE GATT Callbacks register function Declarations
 * ******************************************************/
/** @addtogroup BT-LOW-ENERGY7
* @{
*/
/*==============================================*/
/**
 * @brief       Register the GATT callbacks.
 * @param[in]   ble_on_profiles_list_resp          - Callback for rsi_ble_get_profiles command.
 * @param[in]   ble_on_profile_resp                - Callback for rsi_ble_get_profile command.
 * @param[in]   ble_on_char_services_resp          - Callback for rsi_ble_get_char_services command.
 * @param[in]   ble_on_inc_services_resp           - Callback for rsi_ble_get_inc_services command.
 * @param[in]   ble_on_att_desc_resp               - Callback for rsi_ble_get_att_descriptors command.
 * @param[in]   ble_on_read_resp                   - Callback for all read requests command.
 * @param[in]   ble_on_write_resp                  - Callback for all write commands.
 * @param[in]   ble_on_gatt_event                  - Callback for all GATT events.
 * @param[in]   ble_on_gatt_prepare_write_event    - Callback for GATT prepare write events.
 * @param[in]   ble_on_execute_write_event         - Callback for GATT execute write events.
 * @param[in]   ble_on_read_req_event              - Callback for read request events.
 * @param[in]   ble_on_mtu_event                   - Callback for MTU events.
 * @param[in]   ble_on_gatt_error_resp_event       - Callback for GATT error events.
 * @param[in]   ble_on_gatt_desc_val_resp_event    - Callback for GATT descriptor value events.
 * @param[in]   ble_on_profiles_list_event         - Callback for profiles list events.
 * @param[in]   ble_on_profile_by_uuid_event       - Callback for profile by UUID events.
 * @param[in]   ble_on_read_by_char_services_event - Callback for read by characteristic services events.
 * @param[in]   ble_on_read_by_inc_services_event  - Callback for read by included services events.
 * @param[in]   ble_on_read_att_value_event        - Callback for read attribute value events.
 * @param[in]   ble_on_read_resp_event             - Callback for read response events.
 * @param[in]   ble_on_write_resp_event            - Callback for write response events.
 * @param[in]   ble_on_indicate_confirmation_event - Callback for indication confirmation events.
 * @param[in]   ble_on_prepare_write_resp_event    - Callback for prepare write response events.
 * 
 */
void rsi_ble_gatt_register_callbacks(rsi_ble_on_profiles_list_resp_t ble_on_profiles_list_resp,
                                     rsi_ble_on_profile_resp_t ble_on_profile_resp,
                                     rsi_ble_on_char_services_resp_t ble_on_char_services_resp,
                                     rsi_ble_on_inc_services_resp_t ble_on_inc_services_resp,
                                     rsi_ble_on_att_desc_resp_t ble_on_att_desc_resp,
                                     rsi_ble_on_read_resp_t ble_on_read_resp,
                                     rsi_ble_on_write_resp_t ble_on_write_resp,
                                     rsi_ble_on_gatt_write_event_t ble_on_gatt_event,
                                     rsi_ble_on_gatt_prepare_write_event_t ble_on_gatt_prepare_write_event,
                                     rsi_ble_on_execute_write_event_t ble_on_execute_write_event,
                                     rsi_ble_on_read_req_event_t ble_on_read_req_event,
                                     rsi_ble_on_mtu_event_t ble_on_mtu_event,
                                     rsi_ble_on_gatt_error_resp_t ble_on_gatt_error_resp_event,
                                     rsi_ble_on_gatt_desc_val_event_t ble_on_gatt_desc_val_resp_event,
                                     rsi_ble_on_event_profiles_list_t ble_on_profiles_list_event,
                                     rsi_ble_on_event_profile_by_uuid_t ble_on_profile_by_uuid_event,
                                     rsi_ble_on_event_read_by_char_services_t ble_on_read_by_char_services_event,
                                     rsi_ble_on_event_read_by_inc_services_t ble_on_read_by_inc_services_event,
                                     rsi_ble_on_event_read_att_value_t ble_on_read_att_value_event,
                                     rsi_ble_on_event_read_resp_t ble_on_read_resp_event,
                                     rsi_ble_on_event_write_resp_t ble_on_write_resp_event,
                                     rsi_ble_on_event_indicate_confirmation_t ble_on_indicate_confirmation_event,
                                     rsi_ble_on_event_prepare_write_resp_t ble_on_prepare_write_resp_event);

/*==============================================*/
/**
 * @brief       Register the GATT Extended responses/events callbacks.
 * @param[in]   ble_on_mtu_exchange_info_event         ble_on_mtu_exchange_info_event         - Call back function for MTU Exchange information Event
 * 
 *
 */
void rsi_ble_gatt_extended_register_callbacks(rsi_ble_on_mtu_exchange_info_t ble_on_mtu_exchange_info_event);
/** @} */
/** @} */

/**
 * @fn       uint32_t rsi_bt_debug_logs_register_callbacks
 * 
 */
uint32_t rsi_bt_debug_logs_register_callbacks(uint16_t callback_id,
                                              void (*callback_handler_ptr)(uint16_t status, uint8_t *buffer));

#ifdef __cplusplus
}
#endif
#endif