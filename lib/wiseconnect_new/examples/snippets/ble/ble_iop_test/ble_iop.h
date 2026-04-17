/*******************************************************************************
* @file  ble_iop.h
* @brief
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
//! WDT constants

//! // WDT Interrupt Time
#define WDT_INTERRUPT_TIME 15
//! WDT System Reset Time
#define WDT_SYSTEM_RESET_TIME 17
//! WDT Window Time
#define WDT_WINDOW_TIME 0

//! local device name
#define RSI_BLE_DEVICE_NAME (void *)"IOP_Test_1"
//! local device IO capability
#define RSI_BLE_SMP_IO_CAPABILITY 0x00
//! error code
#define BT_HCI_COMMAND_DISALLOWED 0x4E0C
//! MITM status
#define MITM_REQ 0x01
//! MTU size
#define MAX_MTU_SIZE 240
//! Maximum data length
#define MAX_SEND_DATA_LEN 232
//! Length of the local device address
#define LOCAL_DEV_ADDR_LEN 18
//! PDU size
#define PDU_SIZE 251
//! DATA length for test case
#define DATA_LENGTH       255
#define WRITE_DATA_LENGTH 240
//! Device info UUID
#define DEVICE_INFO 0x180A
//! Device name UUID
#define DEVICE_NAME 0x2A29
//! Model number UUID
#define MODEL_NUMBER 0x2A24
//! Device name value
#define DEVICE_NAME_VALUE "Silicon Labs"
//! Model number value
#define MODEL_STRING "BRD4338A"
//! Notification data size
#define DATA_SIZE_NOTIFY 250
//! Stack version
#define STACK_VERSION "7.2.0-b1761"
//! Max LTK list size
#define RSI_MAX_LIST_SIZE 0x05
//! LTK length
#define LTK_LENGTH 16
//! Random number length
#define RAND_LENGTH 8

//! application event list
#define RSI_BLE_ADV_REPORT_EVENT            0x01
#define RSI_BLE_CONN_EVENT                  0x02
#define RSI_BLE_DISCONN_EVENT               0x03
#define RSI_BLE_SMP_REQ_EVENT               0x04
#define RSI_BLE_SMP_RESP_EVENT              0x05
#define RSI_BLE_SMP_PASSKEY_EVENT           0x06
#define RSI_BLE_SMP_FAILED_EVENT            0x07
#define RSI_BLE_ENCRYPT_STARTED_EVENT       0x08
#define RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT   0x09
#define RSI_BLE_SC_PASSKEY_EVENT            0x0A
#define RSI_BLE_LTK_REQ_EVENT               0x0B
#define RSI_BLE_RECEIVE_REMOTE_FEATURES     0x0C
#define RSI_APP_EVENT_DATA_LENGTH_CHANGE    0x0D
#define RSI_APP_EVENT_PHY_UPDATE_COMPLETE   0x0E
#define RSI_BLE_CONN_UPDATE_EVENT           0x0F
#define RSI_BLE_MTU_EVENT                   0x10
#define RSI_BLE_GATT_WRITE_EVENT            0x11
#define RSI_BLE_MORE_DATA_REQ_EVENT         0x12
#define RSI_DATA_TRANSMIT_EVENT             0x13
#define RSI_BLE_PREPARE_WRITE_REQUEST_EVENT 0x14
#define RSI_BLE_GATT_SEND_DATA              0x15
#define RSI_BLE_GATT_READ_EVENT             0x16
#define RSI_BLE_EXECUTE_WRITE_REQUEST_EVENT 0x17
#define RSI_BLE_SECURITY_KEYS_EVENT         0x18

//! Connection parameters value
#define CONN_INTERVAL_MIN_LSB   0xF7 // Minimum connection interval (LSB)
#define CONN_INTERVAL_MIN_MSB   0x00 // Minimum connection interval (MSB)
#define CONN_INTERVAL_MAX_LSB   0xFB // Maximum connection interval (LSB)
#define CONN_INTERVAL_MAX_MSB   0x00 // Maximum connection interval (MSB)
#define CONN_LATENCY_LSB        0x24 // Peripheral latency (LSB)
#define CONN_LATENCY_MSB        0x00 // Peripheral latency (MSB)
#define SUPERVISION_TIMEOUT_LSB 0x00 // Supervision timeout (LSB)
#define SUPERVISION_TIMEOUT_MSB 0x00 // Supervision timeout (MSB)
#define MIN_CE_LENGTH_LSB       0xF4 // Minimum CE (Connection Event) length (LSB)
#define MIN_CE_LENGTH_MSB       0x01 // Minimum CE length (MSB)
#define MAX_CE_LENGTH_LSB       0x01 // Maximum CE length (LSB)

//! LE resolvlist.
typedef struct rsi_ble_dev_ltk_list_s {
  uint8_t enc_enable;
  uint8_t sc_enable;
  uint8_t remote_dev_addr_type;
  uint8_t remote_dev_addr[6];
  uint8_t peer_irk[16];
  uint8_t local_irk[16];
  uint16_t remote_ediv;
  uint16_t local_ediv;
  uint8_t remote_rand[8];
  uint8_t localrand[8];
  uint8_t remote_ltk[16];
  uint8_t localltk[16];
  uint8_t Identity_addr_type;
  uint8_t Identity_addr[6];
  uint8_t used;
} rsi_ble_dev_ltk_list_t;

//! device information
typedef struct rsi_ble_resolve_key_s {
  uint8_t remote_dev_addr_type;
  uint8_t remote_dev_addr[18];
  uint8_t peer_irk[16];
  uint8_t local_irk[16];
  uint16_t remote_ediv;
  uint8_t remote_rand[16];
  uint8_t remote_ltk[16];
  uint8_t Identity_addr_type;
  uint8_t Identity_addr[18];
} rsi_ble_resolve_key_t;

//! LE resolvlist group.
typedef struct rsi_ble_resolvlist_group_s {
  uint8_t used;
  uint8_t remote_dev_addr_type;
  uint8_t remote_dev_addr[18];
  uint8_t peer_irk[16];
  uint8_t local_irk[16];
  uint8_t Identity_addr_type;
  uint8_t Identity_addr[18];
} rsi_ble_resolvlist_group_t;

//! Structure for storing handles for IOP Test Service and its characteristics
typedef struct {
  uint16_t iop_test_service1; //IOP test service 1 handle
  uint16_t stack_version;     //IOP stack version handle
  uint16_t conn_param;        //IOP connection parameters handle
  uint16_t hash_handle;
} test_iop_serv1;

//! Structure for storing connection parameters
typedef struct {
  uint8_t PHY;                  //value of PHY
  uint16_t supervision_timeout; //store the value of supervision timeout
  uint8_t responder_latency;    //value of responder latency
  uint16_t connection_interval; //value of connection interval
  uint16_t pdu_size;            //value of PDU size
  uint16_t mtu_size;            //value of MTU size
} connection_information;

//! Structure for Silicon Labs IOP Test Properties
typedef struct {
  uint16_t iop_test_prop_service;     //IOP test properties handle
  uint16_t read_only_len1;            //Read only length 1 handle
  uint16_t read_only_len255;          //Read only length 255 handle
  uint16_t write_only_len1;           //Write only length 1 handle
  uint16_t write_only_len255;         //Write only length 255 handle
  uint16_t write_without_resp_len1;   //Write without response length 1 handle
  uint16_t write_without_resp_len255; //Write without response length 255 handle
  uint16_t notify_len1;               //Notify length 1 handle
  uint16_t notify_len3;               //Notify length MTU-3 handle
  uint16_t indicate_len1;             //Indicate length 1 handle
  uint16_t indicate_len3;             //Indicate length MTU-3 handle
} test_prop;

//! Structure for storing IOP test characteristics types handles
typedef struct {
  uint16_t char_types_service; //characteristic type handle
  uint16_t char_len1;          //length 1 handle
  uint16_t char_len255;        //length 255 handle
  uint16_t char_len_var4;      //length variable 4 handle
  uint16_t char_const_len1;    //constant length 1 handle
  uint16_t char_const_len255;  //constant length 255 handle
  uint16_t char_usr_len1;      //user length 1 handle
  uint16_t char_usr_len255;    //user length 255 handle
  uint16_t char_usr_len_var4;  //user length 4 handle
} test_char_types;

//! Structure for storing IOP test phase3 service handles
typedef struct {
  uint16_t phase3_control_handle; //Phase3 control characteristic handle
  uint16_t tp_notify_handle;      //Throughput characteristic handle
  uint16_t handle_justworks;      //security justworks characteristic handle
  uint16_t handle_auth;           //security authentication characteristic handle
  uint16_t handle_bonding;        //security bonding characteristic handle
} iop_phase3_service_struct;

//! Structure to store device information
typedef struct {
  uint16_t device_name_handle;  //Device name char handle
  uint16_t model_number_handle; //model number char handle
} device_information;

//! Bluetooth Secuirty Mode
typedef enum {
  rsi_bt_connection_mode1_level1 = 0x0,
  /**< (0x0) No security */             //!< rsi_bt_connection_mode1_level1
  rsi_bt_connection_mode1_level2 = 0x1, /**< (0x1) Unauthenticated pairing with//!< rsi_bt_connection_mode1_level2
                                            encryption */
  rsi_bt_connection_mode1_level3 = 0x2, /**< (0x2) Authenticated pairing with  //!< rsi_bt_connection_mode1_level3
                                            encryption */
  rsi_bt_connection_mode1_level4 = 0x3, /**< (0x3) Authenticated Secure       //!< rsi_bt_connection_mode1_level4
                                            Connections pairing with encryption
                                            using a 128-bit strength encryption
                                            key */
  security_level_unchanged       = 0x4, //!< security_level_unchanged
  security_level_privacy         = 0x5  //!< security_level_privacy
} rsi_bt_connection_security_t;

//! Function Declarations
int8_t rsi_get_ltk_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list, rsi_bt_event_le_ltk_request_t *le_ltk_req);
uint32_t rsi_ble_iop_test_service(void);
uint32_t rsi_ble_iop_test_properties(void);
uint32_t rsi_ble_iop_test_characteristics(void);
void rsi_ble_add_device_info(void);
#if FWUP_UPGRADE
int32_t handle_ota_process(uint16_t handle_value);
#endif
