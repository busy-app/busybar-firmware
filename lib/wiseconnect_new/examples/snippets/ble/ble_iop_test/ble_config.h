/*******************************************************************************
* @file  ble_config.h
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

#ifndef BLE_CONFIG_H
#define BLE_CONFIG_H

/******************************************************
 * *                      Macros
 * ******************************************************/
#ifndef FWUP_UPGRADE
#define FWUP_UPGRADE 0
#endif

#define RSI_BLE_SET_RAND_ADDR "00:23:A7:12:34:56"

#define CLEAR_ACCEPTLIST              0x00
#define ADD_DEVICE_TO_ACCEPTLIST      0x01
#define DELETE_DEVICE_FROM_ACCEPTLIST 0x02

#define ALL_PHYS 0x00

#define RSI_BLE_DEV_ADDR_RESOLUTION_ENABLE 1
#define RSI_BLE_MAX_NBR_PERIPHERALS        1

#define RSI_BLE_MAX_NBR_ATT_REC  100
#define RSI_BLE_NUM_CONN_EVENTS  30
#define RSI_BLE_MAX_NBR_ATT_SERV 10

#define RSI_BLE_MAX_NBR_CENTRALS  1
#define RSI_BLE_GATT_ASYNC_ENABLE 0
#define RSI_BLE_GATT_INIT         0

#define BD_ADDR_STRING_SIZE   18
#define FIRMWARE_VERSION_SIZE 18
//!UUID size
#define UUID_SIZE     16
#define FW_UP_SUCCESS 0x1dd03

//!ERROR CODE
#define WRITE_NOT_PERIMITTED 0x03

// Mobile OS types
#define ANDROID 0x01
#define IOS     0x02

#define TA_FW_UP 0
#define M4_FW_UP 1

#define FW_UPGRADE_TYPE M4_FW_UP

/* Number of BLE GATT RECORD SIZE IN (n*16 BYTES), eg:(0x40*16)=1024 bytes */
#define RSI_BLE_NUM_REC_BYTES 0x40

#define RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST 0

/*=======================================================================*/
//! Advertising command parameters
/*=======================================================================*/

#define RSI_BLE_ADV_TYPE          UNDIR_CONN
#define RSI_BLE_ADV_FILTER_TYPE   ALLOW_SCAN_REQ_ANY_CONN_REQ_ANY
#define RSI_BLE_ADV_DIR_ADDR_TYPE LE_PUBLIC_ADDRESS
#define RSI_BLE_ADV_DIR_ADDR      "00:15:83:6A:64:17"

#define RSI_BLE_ADV_INT_MIN     0x20
#define RSI_BLE_ADV_INT_MAX     0x20
#define RSI_BLE_ADV_CHANNEL_MAP 0x07

//!Advertise status
//! Start the advertising process
#define RSI_BLE_START_ADV 0x01
//! Stop the advertising process
#define RSI_BLE_STOP_ADV 0x00

//! BLE Tx Power Index On Air
#define RSI_BLE_PWR_INX 30

//! BLE Active H/w Pwr Features
#define BLE_DISABLE_DUTY_CYCLING 0
#define BLE_DUTY_CYCLING         1
#define BLR_DUTY_CYCLING         2
#define BLE_4X_PWR_SAVE_MODE     4
#define RSI_BLE_PWR_SAVE_OPTIONS BLE_DISABLE_DUTY_CYCLING

//!Advertise types

/* Advertising will be visible(discoverable) to all the devices.
 * Scanning/Connection is also accepted from all devices
 * */
#define UNDIR_CONN 0x80

/* Advertising will be visible(discoverable) to the particular device 
 * mentioned in RSI_BLE_ADV_DIR_ADDR only. 
 * Scanning and Connection will be accepted from that device only.
 * */
#define DIR_CONN 0x81

/* Advertising will be visible(discoverable) to all the devices.
 * Scanning will be accepted from all the devices.
 * Connection will be not be accepted from any device.
 * */
#define UNDIR_SCAN 0x82

/* Advertising will be visible(discoverable) to all the devices.
 * Scanning and Connection will not be accepted from any device
 * */
#define UNDIR_NON_CONN 0x83

/* Advertising will be visible(discoverable) to the particular device 
 * mentioned in RSI_BLE_ADV_DIR_ADDR only. 
 * Scanning and Connection will be accepted from that device only.
 * */
#define DIR_CONN_LOW_DUTY_CYCLE 0x84

//!Advertising flags
#define LE_LIMITED_DISCOVERABLE 0x01
#define LE_GENERAL_DISCOVERABLE 0x02
#define LE_BR_EDR_NOT_SUPPORTED 0x04

//!Advertise filters
#define ALLOW_SCAN_REQ_ANY_CONN_REQ_ANY                 0x00
#define ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ANY         0x01
#define ALLOW_SCAN_REQ_ANY_CONN_REQ_ACCEPT_LIST         0x02
#define ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ACCEPT_LIST 0x03

//! Address types
#define LE_PUBLIC_ADDRESS            0x00
#define LE_RANDOM_ADDRESS            0x01
#define LE_RESOLVABLE_PUBLIC_ADDRESS 0x02
#define LE_RESOLVABLE_RANDOM_ADDRESS 0x03

/*=======================================================================*/

/*=======================================================================*/
//! Connection parameters
/*=======================================================================*/
#define LE_SCAN_INTERVAL 0x0100
#define LE_SCAN_WINDOW   0x0050

#define CONNECTION_INTERVAL_MIN 0x00A0
#define CONNECTION_INTERVAL_MAX 0x00A0

#define CONNECTION_LATENCY  0x0000
#define SUPERVISION_TIMEOUT 800

/*=======================================================================*/

/*=======================================================================*/
//! Scan command parameters
/*=======================================================================*/

#define RSI_BLE_SCAN_TYPE        SCAN_TYPE_ACTIVE
#define RSI_BLE_SCAN_FILTER_TYPE SCAN_FILTER_TYPE_ALL

//!Scan status
#define RSI_BLE_START_SCAN 0x01
#define RSI_BLE_STOP_SCAN  0x00

//!Scan types
#define SCAN_TYPE_ACTIVE  0x01
#define SCAN_TYPE_PASSIVE 0x00

//!Scan filters
#define SCAN_FILTER_TYPE_ALL              0x00
#define SCAN_FILTER_TYPE_ONLY_ACCEPT_LIST 0x01

#define RSI_SEL_INTERNAL_ANTENNA 0x00
#define RSI_SEL_EXTERNAL_ANTENNA 0x01
/*=======================================================================*/

//! Remote device bd address
#define RSI_BLE_REMOTE_DEV_ADDR "00:12:45:AB:1D:32"
//! Remote device name
#define RSI_REMOTE_DEVICE_NAME "Note10"

#define CONN_BY_ADDR 1
#define CONN_BY_NAME 2

#define CONNECT_OPTION CONN_BY_NAME

#define CENTERAL_ROLE   1
#define PERIPHERAL_ROLE 2
#define CONNECTION_ROLE PERIPHERAL_ROLE

//! connection update params
#define CONN_INTERVAL_MIN 0x20
#define CONN_INTERVAL_MAX 0x20
#define CONN_LATENCY      0

//! enabling the security
#define SMP_ENABLE 1

#define LOCAL_OOB_DATA_FLAG_NOT_PRESENT (0x00)

#define AUTH_REQ_BONDING_BITS   ((1 << 0))
#define AUTH_REQ_MITM_BIT       (1 << 2)
#define AUTH_REQ_SC_BIT         (1 << 3)
#define AUTH_REQ_CT2_BIT        (1 << 5)
#define AUTH_REQ_BITS           (AUTH_REQ_BONDING_BITS | AUTH_REQ_MITM_BIT | AUTH_REQ_SC_BIT) //| AUTH_REQ_CT2_BIT)
#define MAXIMUM_ENC_KEY_SIZE_16 (16)

// BLUETOOTH SPECIFICATION Version 5.0 | Vol 3, Part H 3.6.1 Key Distribution and Generation
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
/*=======================================================================*/
// BLE Attribute Security Define
/*=======================================================================*/
#define ATT_REC_MAINTAIN_IN_HOST BIT(0) ///< Att record maintained by the stack
#define SEC_MODE_1_LEVEL_1       BIT(1) ///< NO Auth and No Enc
#define SEC_MODE_1_LEVEL_2       BIT(2) ///< UnAUTH with Enc
#define SEC_MODE_1_LEVEL_3       BIT(3) ///< AUTH with Enc
#define SEC_MODE_1_LEVEL_4       BIT(4) ///< AUTH LE_SC Pairing with Enc
#define ON_BR_EDR_LINK_ONLY      BIT(5) ///< BR/EDR link-only mode
#define ON_LE_LINK_ONLY          BIT(6) ///< LE link-only mode

#define ENC_KEY_DIST  (1 << 0)
#define ID_KEY_DIST   (1 << 1)
#define SIGN_KEY_DIST (0 << 2)
#define LINK_KEY_DIST (0 << 3)
#if RESOLVE_ENABLE
#define RESPONDER_KEYS_TO_DIST (ENC_KEY_DIST | SIGN_KEY_DIST | ID_KEY_DIST)
#else
#define RESPONDER_KEYS_TO_DIST (ENC_KEY_DIST | SIGN_KEY_DIST | ID_KEY_DIST | LINK_KEY_DIST)
#endif
#if RESOLVE_ENABLE
#define INITIATOR_KEYS_TO_DIST (ID_KEY_DIST | ENC_KEY_DIST | SIGN_KEY_DIST)
#else
#define INITIATOR_KEYS_TO_DIST (ENC_KEY_DIST | SIGN_KEY_DIST | ID_KEY_DIST | LINK_KEY_DIST)
#endif

//! Tx Data length parameters
#define TX_LEN  0xFB
#define TX_TIME 0x4290

// DATA RATE
//  0x02 - 2Mbps
//  0x01 - 1Mbps
//  0x04 - Coded PHY (set desired CODDED_PHY_RATE)
//! Phy parameter
#define TX_PHY_RATE 0x01
#define RX_PHY_RATE 0x01
// CODED_PHY_RATE: 0x01 - 500Kbps
// CODED_PHY_RATE: 0x02 - 125Kbps
#define CODDED_PHY_RATE 0x00

//! Notify status
#define NOTIFY_DISABLE 0x00
#define NOTIFY_ENABLE  0x01

#define DLE_ON 1

#if DLE_ON
#define DLE_BUFFER_MODE      1
#define DLE_BUFFER_COUNT     25 // Should be less than RSI_BLE_NUM_CONN_EVENTS
#define RSI_BLE_MAX_DATA_LEN 230
#else
#define DLE_BUFFER_MODE      0
#define DLE_BUFFER_COUNT     2 // Should be less than RSI_BLE_NUM_CONN_EVENTS
#define RSI_BLE_MAX_DATA_LEN 20
#endif

#endif