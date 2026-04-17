/*******************************************************************************
 * @file  rsi_ble_common_config.h
 * @brief : This file contains user configurable details to configure the device  
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

#ifndef RSI_BLE_COMMON_CONFIG_H
#define RSI_BLE_COMMON_CONFIG_H
/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */

#ifndef RSI_BLE_SET_RESOLVABLE_PRIV_ADDR_TOUT
#define RSI_BLE_SET_RESOLVABLE_PRIV_ADDR_TOUT 120 ///< BLE Resolvable Private Address timeout value (in seconds)
#endif
#ifndef NO_OF_ADV_REPORTS
#define NO_OF_ADV_REPORTS 10 ///< Maximum number of advertise reports to hold.
#endif
#ifndef BLE_CP_BUFF_SIZE_512
#define BLE_CP_BUFF_SIZE_512 0 ///< Enabled for BLE PER Test.
#endif
/*=======================================================================*/
// attribute properties
/*=======================================================================*/
#ifndef RSI_BLE_ATT_PROPERTY_READ
#define RSI_BLE_ATT_PROPERTY_READ 0x02 ///< Attribute property for read operation in BLE.
#endif
#ifndef RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE
#define RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE 0x04 ///< Attribute property allows writing without requiring a response.
#endif
#ifndef RSI_BLE_ATT_PROPERTY_WRITE
#define RSI_BLE_ATT_PROPERTY_WRITE 0x08 ///< Attribute property is for write operations.
#endif
#ifndef RSI_BLE_ATT_PROPERTY_NOTIFY
#define RSI_BLE_ATT_PROPERTY_NOTIFY 0x10 ///< Enables the BLE Attribute Property for Notification.
#endif
#ifndef RSI_BLE_ATT_PROPERTY_INDICATE
#define RSI_BLE_ATT_PROPERTY_INDICATE 0x20 ///< Attribute property for indication.
#endif
#ifndef BT_GLOBAL_BUFF_LEN
#define BT_GLOBAL_BUFF_LEN 10000 ///< Memory length for driver.
#endif
#ifndef BLE_PERIPHERAL_MODE_ENABLE
#define BLE_PERIPHERAL_MODE_ENABLE 1 ///< Enabling peripheral mode.
#endif
/*=======================================================================*/
// BLE Attribute Security Define
/*=======================================================================*/
#define ATT_REC_MAINTAIN_IN_HOST BIT(0) ///< Attribute record maintained in Host
#define SEC_MODE_1_LEVEL_1       BIT(1) ///< NO Auth and No Enc
#define SEC_MODE_1_LEVEL_2       BIT(2) ///< UnAUTH with Enc
#define SEC_MODE_1_LEVEL_3       BIT(3) ///< AUTH with Enc
#define SEC_MODE_1_LEVEL_4       BIT(4) ///< AUTH LE_SC Pairing with Enc
#define ON_BR_EDR_LINK_ONLY      BIT(5) ///< BR/EDR link-only mode
#define ON_LE_LINK_ONLY          BIT(6) ///< LE link-only mode
#define VARIABLE_ATT_CHAR_VAL    BIT(7) ///< Variable characteristic value length
/*=======================================================================*/
// Max Tx data length and Tx timeout values
/*=======================================================================*/
#ifndef TX_LEN
#define TX_LEN 0x001e ///< The transmission length.
#endif
#ifndef TX_TIME
#define TX_TIME 0x01f4 ///< Transmission time configuration.
#endif
#ifndef RSI_BLE_SMP_IO_CAPABILITY
#define RSI_BLE_SMP_IO_CAPABILITY 0x00 ///< The I/O capability for BLE Security Manager Protocol (SMP).
#endif
#ifndef RSI_BLE_APP_SMP_PASSKEY
#define RSI_BLE_APP_SMP_PASSKEY 0 ///< The default Security Manager Protocol (SMP) passkey for BLE applications.
#endif
/*=======================================================================*/
// PHY rate selections
/*=======================================================================*/
#ifndef TX_PHY_RATE
#define TX_PHY_RATE 0x02 ///< The PHY rate for transmission.
#endif
#ifndef RX_PHY_RATE
#define RX_PHY_RATE 0x02 ///< The PHY rate for RX (Receiver).
#endif
#ifndef CODDED_PHY_RATE
#define CODDED_PHY_RATE 0x00 ///< Specifies the PHY rate for coded PHY.
#endif

/*=======================================================================*/
// privacy mode 0-Network privacy mode 1-Device privacy mode
/*=======================================================================*/
#ifndef RSI_BLE_NETWORK_PRIVACY_MODE
#define RSI_BLE_NETWORK_PRIVACY_MODE 0 ///< This macro defines the value `0` for enabling Network Privacy Mode.
#endif
#ifndef RSI_BLE_DEVICE_PRIVACY_MODE
#define RSI_BLE_DEVICE_PRIVACY_MODE 1 ///< This macro defines the value `1` for enabling Device Privacy Mode.
#endif
#ifndef RSI_BLE_PRIVACY_MODE
#define RSI_BLE_PRIVACY_MODE RSI_BLE_DEVICE_PRIVACY_MODE ///< The BLE privacy mode is set to the device privacy mode.
#endif

#ifndef RSI_BLE_RESOLVING_LIST_SIZE
#define RSI_BLE_RESOLVING_LIST_SIZE 5 ///< Resovable list size
#endif
#ifndef RSI_BLE_MAX_DATA_LEN
#define RSI_BLE_MAX_DATA_LEN 20 ///< max data length
#endif
/*=======================================================================*/
// process type 1-add device to resolvlist, 2-remove device from resolvlist, 3-clear the resolvlist
/*=======================================================================*/
#ifndef RSI_BLE_ADD_TO_RESOLVE_LIST
#define RSI_BLE_ADD_TO_RESOLVE_LIST 1 ///< Enable or disable adding devices to the BLE resolve list.
#endif
#ifndef RSI_BLE_REMOVE_FROM_RESOLVE_LIST
#define RSI_BLE_REMOVE_FROM_RESOLVE_LIST 2 ///< Defines the value used to remove a device from the BLE resolve list.
#endif
#ifndef RSI_BLE_CLEAR_RESOLVE_LIST
#define RSI_BLE_CLEAR_RESOLVE_LIST 3 ///< Clears the resolving list in the BLE stack.
#endif
/*=======================================================================*/
// BLE attribute service types uuid values
/*=======================================================================*/
#ifndef RSI_BLE_BATTERY_SERVICE_UUID
#define RSI_BLE_BATTERY_SERVICE_UUID 0x180F ///< UUID for the Battery Service.
#endif
#ifndef RSI_BLE_CHAR_SERV_UUID
#define RSI_BLE_CHAR_SERV_UUID 0x2803 ///< UUID for BLE Characteristic Service.
#endif
#ifndef RSI_BLE_CLIENT_CHAR_UUID
#define RSI_BLE_CLIENT_CHAR_UUID 0x2902 ///< UUID for the BLE Client Characteristic Configuration Descriptor.
#endif
#ifndef RSI_BLE_CHAR_PRESENTATION_FORMATE_UUID
#define RSI_BLE_CHAR_PRESENTATION_FORMATE_UUID \
  0x2904 ///< UUID for the BLE Characteristic Presentation Format Descriptor.
#endif
/*=======================================================================*/
// BLE characteristic service and attribute uuid
/*=======================================================================*/
#ifndef RSI_BLE_NEW_SERVICE_UUID
#define RSI_BLE_NEW_SERVICE_UUID 0xAABB ///< Defines the UUID for a new BLE service.
#endif
#ifndef RSI_BLE_NEW_SERVICE_UUID2
#define RSI_BLE_NEW_SERVICE_UUID2 0xCCDD ///< Defines the UUID for a new BLE service.
#endif
#ifndef RSI_BLE_ATTRIBUTE_1_UUID
#define RSI_BLE_ATTRIBUTE_1_UUID 0x1AA1 ///< UUID for BLE Attribute 1.
#endif
#ifndef RSI_BLE_ATTRIBUTE_2_UUID
#define RSI_BLE_ATTRIBUTE_2_UUID 0x1BB1 ///< Defines the UUID for the second BLE attribute.
#endif
#ifndef RSI_BLE_ATTRIBUTE_3_UUID
#define RSI_BLE_ATTRIBUTE_3_UUID 0x1CC1 ///< UUID for the third BLE attribute.
#endif
#ifndef RSI_BLE_ATTRIBUTE_4_UUID
#define RSI_BLE_ATTRIBUTE_4_UUID 0x1DD1 ///< UUID for BLE Attribute 4.
#endif

#ifndef RSI_BLE_SET_RAND_ADDR
#define RSI_BLE_SET_RAND_ADDR "00:23:A7:12:34:56" ///< Defines the random Bluetooth address for BLE configuration.
#endif

#ifndef CLEAR_ACCEPTLIST
#define CLEAR_ACCEPTLIST 0x00 ///< Clears the accept list.
#endif
#ifndef ADD_DEVICE_TO_ACCEPTLIST
#define ADD_DEVICE_TO_ACCEPTLIST 0x01 ///< Adds a device to the accept list.
#endif
#ifndef DELETE_DEVICE_FROM_ACCEPTLIST
#define DELETE_DEVICE_FROM_ACCEPTLIST 0x02 ///< Operation code for deleting a device from the accept list.
#endif

#ifndef ALL_PHYS
#define ALL_PHYS 0x00 ///< Specify that all physical layer settings should be applied.
#endif

#ifndef RSI_BLE_DEV_ADDR_RESOLUTION_ENABLE
#define RSI_BLE_DEV_ADDR_RESOLUTION_ENABLE 0 ///< Enable or disable BLE device address resolution.
#endif

#ifndef RSI_OPERMODE_WLAN_BLE
#define RSI_OPERMODE_WLAN_BLE 13 ///< The operation mode for WLAN and BLE coexistence.
#endif

#ifndef RSI_BLE_MAX_NBR_ATT_REC
#define RSI_BLE_MAX_NBR_ATT_REC 80 ///< Maximum number of Attribute Records.
#endif
#ifndef RSI_BLE_MAX_NBR_ATT_SERV
#define RSI_BLE_MAX_NBR_ATT_SERV 10 ///< Defines the maximum number of Attribute Services supported by the BLE stack.
#endif

#ifndef RSI_BLE_MAX_NBR_PERIPHERALS
#define RSI_BLE_MAX_NBR_PERIPHERALS 3 ///< Defines the maximum number of BLE peripheral devices that can be connected.
#endif
#ifndef RSI_BLE_MAX_NBR_CENTRALS
#define RSI_BLE_MAX_NBR_CENTRALS \
  2 ///< Defines the maximum number of BLE central devices that can be connected simultaneously.
#endif
#ifndef RSI_BLE_NUM_CONN_EVENTS
#define RSI_BLE_NUM_CONN_EVENTS 20 ///< Defines the number of BLE connection events.
#endif

#ifndef RSI_BLE_PACKET_CHUNK_SIZE
#define RSI_BLE_PACKET_CHUNK_SIZE 320
#endif

#ifndef RSI_BLE_SMALL_BUFF_LEN
#define RSI_BLE_SMALL_BUFF_LEN 32
#endif

#ifndef RSI_BLE_SMALL_BUFF_COUNT_PER_PACKET_CHUNK
#define RSI_BLE_SMALL_BUFF_COUNT_PER_PACKET_CHUNK (RSI_BLE_PACKET_CHUNK_SIZE / RSI_BLE_SMALL_BUFF_LEN)
#endif

#ifndef RSI_BLE_PER_CONN_MIN_PACKET_BUFFS_REQUIRED
#define RSI_BLE_PER_CONN_MIN_PACKET_BUFFS_REQUIRED 2
#endif

#ifndef RSI_BLE_MIN_CE_BUFFS_REQUIRED_INCASE_SMALL_BUFF_MODE
#define RSI_BLE_MIN_CE_BUFFS_REQUIRED_INCASE_SMALL_BUFF_MODE 5
#endif

#ifndef RSI_BLE_MIN_CE_BUFFS_REQUIRED_INCASE_BIG_BUFF_MODE
#define RSI_BLE_MIN_CE_BUFFS_REQUIRED_INCASE_BIG_BUFF_MODE 4
#endif

#if !RSI_BLE_MAX_NBR_PERIPHERALS /* Wireless initialization is failing with error 
SL_STATUS_SI91X_INVALID_CONFIG_RANGE_PROVIDED(0x10063) for the 0P + 2C configuration. Therefore, for the 0P configuration, 
the number of peripherals has been updated to 1, and the number of connection events has been set to 8. */
#undef RSI_BLE_MAX_NBR_PERIPHERALS
#undef RSI_BLE_NUM_CONN_EVENTS
#define RSI_BLE_NUM_CONN_EVENTS     8
#define RSI_BLE_MAX_NBR_PERIPHERALS 1
#endif
/* Number of BLE GATT RECORD SIZE IN (n*16 BYTES), eg:(0x40*16) = 1024 bytes */
#ifndef RSI_BLE_NUM_REC_BYTES
#define RSI_BLE_NUM_REC_BYTES 0x40 ///< Defines the number of bytes to be received in a BLE operation.
#endif
/*=======================================================================*/
// Advertising command parameters
/*=======================================================================*/
#ifndef RSI_BLE_ADV_DIR_ADDR
#define RSI_BLE_ADV_DIR_ADDR "00:15:83:6A:64:17" ///< Defines the Bluetooth Low Energy (BLE) advertising direct address.
#endif

#ifndef RSI_BLE_ADV_CHANNEL_MAP
#define RSI_BLE_ADV_CHANNEL_MAP \
  0x07 ///< Defines the advertising channel map for BLE. Setting all bits to 1 (0x07) enables advertising on all three channels.
#endif

#ifndef RSI_BLE_PWR_INX
#define RSI_BLE_PWR_INX 30 ///< BLE Tx Power Index On Air
#endif

#ifndef RSI_BLE_PWR_INX_DBM
#define RSI_BLE_PWR_INX_DBM 0 ///< BLE Tx Power Index On Air per Gap role
#endif

#ifndef RSI_BLE_START_ADV
#define RSI_BLE_START_ADV 0x01 ///< Start the advertising process.
#endif
#ifndef RSI_BLE_STOP_ADV
#define RSI_BLE_STOP_ADV 0x00 ///< Stop the advertising process.
#endif
/*=======================================================================*/
// Advertise type
/*=======================================================================*/
/**
 * @def UNDIR_CONN
 * @brief Advertising will be visible(discoverable) to all the devices.
 * Scanning/Connection is also accepted from all devices
 */
#ifndef UNDIR_CONN
#define UNDIR_CONN 0x80
#endif
/**
 * @def DIR_CONN
 * @brief Advertising will be visible (discoverable) to the particular device 
 * mentioned in RSI_BLE_ADV_DIR_ADDR only. 
 * Scanning and Connection will be accepted from that device only.
 */
#ifndef DIR_CONN
#define DIR_CONN 0x81
#endif
/**
 * @def UNDIR_SCAN
 * @brief Advertising would be visible (discoverable) to all the devices.
 * Scanning will be accepted from all the devices.
 * Connection will be not be accepted from any device.
 */
#ifndef UNDIR_SCAN
#define UNDIR_SCAN 0x82
#endif
/**
 * @def UNDIR_NON_CONN
 * @brief Advertising would be visible (discoverable) to all the devices.
 * Scanning and Connection would not be accepted from any device.
 */
#ifndef UNDIR_NON_CONN
#define UNDIR_NON_CONN 0x83
#endif
/**
 * @def DIR_CONN_LOW_DUTY_CYCLE
 * @brief Advertising would be visible (discoverable) to the particular device 
 * mentioned in RSI_BLE_ADV_DIR_ADDR only. 
 * Scanning and Connection would be accepted from that device only.
 */
#ifndef DIR_CONN_LOW_DUTY_CYCLE
#define DIR_CONN_LOW_DUTY_CYCLE 0x84
#endif
/**
 * @def RSI_BLE_ADV_TYPE
 * @brief Defines the type of BLE advertisement.
 *
 * This macro sets the type of Bluetooth Low Energy (BLE) advertisement.
 * The value `UNDIR_CONN` indicates that the advertisement type is 
 * undirected connectable.
 *
 * Possible values:
 * - `UNDIR_CONN`: Undirected connectable advertisement.
 * - Other values might be defined based on the BLE stack being used.
 *
 * Example usage:
 * @code
 * #define RSI_BLE_ADV_TYPE UNDIR_CONN
 * @endcode
 */
#ifndef RSI_BLE_ADV_TYPE
#define RSI_BLE_ADV_TYPE UNDIR_CONN
#endif
/*=======================================================================*/
// Advertising flags
/*=======================================================================*/
/**
 * @def LE_LIMITED_DISCOVERABLE
 * @brief Limited Discoverable mode 
 *    
 * The Limited Discoverable mode is used to indicate that the device is discoverable for a limited period of time.
 * This mode is applicable for the devices that are not discoverable always, and becomes discoverable only for a short duration.
 * 
 */
#ifndef LE_LIMITED_DISCOVERABLE
#define LE_LIMITED_DISCOVERABLE 0x01
#endif
/**
 * @def LE_GENERAL_DISCOVERABLE
 * @brief LE General Discoverable Mode
 *
 * This macro defines the value for the LE General Discoverable mode.
 * It is used to set the device in a mode where it can be discovered
 * by other Bluetooth Low Energy (BLE) devices.
 *
 */
#ifndef LE_GENERAL_DISCOVERABLE
#define LE_GENERAL_DISCOVERABLE 0x02
#endif
/**
 * @def LE_BR_EDR_NOT_SUPPORTED
 * @brief BR/EDR (Basic Rate/Enhanced Data Rate) is not supported.
 * 
 * The value `0x04` is used to signify that the device does not support BR/EDR.
 * 
 * @note
 * Ensure that this macro is used in contexts where the absence of BR/EDR support is relevant.
 */
#ifndef LE_BR_EDR_NOT_SUPPORTED
#define LE_BR_EDR_NOT_SUPPORTED 0x04
#endif
/*=======================================================================*/
// Advertise filters
/*=======================================================================*/
/**
 * @def ALLOW_SCAN_REQ_ANY_CONN_REQ_ANY
 * @brief Allow any scan request and any connection request.
 *
 * Allow any device to send a scan request and any device to send a connection request.
 *
 * @note This setting might be used in scenarios where the device needs to be 
 *       discoverable and connectable by any other BLE device without restrictions.
 */
#ifndef ALLOW_SCAN_REQ_ANY_CONN_REQ_ANY
#define ALLOW_SCAN_REQ_ANY_CONN_REQ_ANY 0x00
#endif
/** @def ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ANY
 *  @brief Allows scan requests from devices in the accept list and connection requests from any device.
 *
 * Configures the device to:
 * - Accept scan requests only from devices in the accept list.
 * - Accept connection requests from any device, regardless of whether it is in the accept list.
 */
#ifndef ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ANY
#define ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ANY 0x01
#endif
/** @def ALLOW_SCAN_REQ_ANY_CONN_REQ_ACCEPT_LIST
 *  @brief Allows scan requests from any device and connection requests only from devices in the accept list.
 *
 * Configures the device to:
 * - Accept scan requests from any device, regardless of whether it is in the accept list.
 * - Accept connection requests only from devices in the accept list.
 */
#ifndef ALLOW_SCAN_REQ_ANY_CONN_REQ_ACCEPT_LIST
#define ALLOW_SCAN_REQ_ANY_CONN_REQ_ACCEPT_LIST 0x02
#endif
/** @def ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ACCEPT_LIST
 *  @brief Allows both scan and connection requests only from devices in the accept list.
 *
 * Configures the device to:
 * - Accept scan requests only from devices in the accept list.
 * - Accept connection requests only from devices in the accept list.
 */
#ifndef ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ACCEPT_LIST
#define ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ACCEPT_LIST 0x03
#endif
/** @def RSI_BLE_ADV_FILTER_TYPE
 *  @brief Sets the advertising filter policy to allow both scan and connection requests from any device.
 *
 * Configures the advertising filter policy to:
 * - Accept scan requests from any device, regardless of whether it is in the accept list.
 * - Accept connection requests from any device, regardless of whether it is in the accept list.
 *
 * Uses the `ALLOW_SCAN_REQ_ANY_CONN_REQ_ANY` setting.
 */
#ifndef RSI_BLE_ADV_FILTER_TYPE
#define RSI_BLE_ADV_FILTER_TYPE ALLOW_SCAN_REQ_ANY_CONN_REQ_ANY
#endif
/*=======================================================================*/
// Address types
/*=======================================================================*/
/**
 * @def LE_PUBLIC_ADDRESS
 * @brief Defines the public address type used in BLE communication.
 *
 * Represents the public address type with a value of 0x00.
 */
#ifndef LE_PUBLIC_ADDRESS
#define LE_PUBLIC_ADDRESS 0x00
#endif
/**
 * @def LE_RANDOM_ADDRESS
 * @brief Defines the type for a random address used in BLE communication.
 *
 * Specifies the value for a random address type with a value of 0x01.
 */
#ifndef LE_RANDOM_ADDRESS
#define LE_RANDOM_ADDRESS 0x01
#endif
/**
 * @def LE_RESOLVABLE_PUBLIC_ADDRESS
 * @brief Defines the resolvable public address type used in communication.
 *
 * Represents the resolvable public address type with a value of 0x02.
 */
#ifndef LE_RESOLVABLE_PUBLIC_ADDRESS
#define LE_RESOLVABLE_PUBLIC_ADDRESS 0x02
#endif
/**
 * @def LE_RESOLVABLE_RANDOM_ADDRESS
 * @brief Defines the resolvable random address type used in communication.
 *
 * Represents the resolvable random address type with a value of 0x03.
 */
#ifndef LE_RESOLVABLE_RANDOM_ADDRESS
#define LE_RESOLVABLE_RANDOM_ADDRESS 0x03
#endif
/**
 * @def RSI_BLE_ADV_DIR_ADDR_TYPE
 * @brief Defines the directed advertising address type used in communication.
 *
 * Represents the directed advertising address type, set to LE_PUBLIC_ADDRESS.
 */
#ifndef RSI_BLE_ADV_DIR_ADDR_TYPE
#define RSI_BLE_ADV_DIR_ADDR_TYPE LE_PUBLIC_ADDRESS
#endif
/*=======================================================================*/
// Connection parameters
/*=======================================================================*/
/**
 * @def CONNECTION_LATENCY
 * @brief Defines the peripheral latency used in communication.
 *
 * Represents the peripheral latency with a value of 0x0000.
 * 
 * Peripheral latency is the number of connection events the peripheral device
 * can skip. A value of 0x0000 means no events are skipped.
 */
#ifndef CONNECTION_LATENCY
#define CONNECTION_LATENCY 0x0000
#endif
/**
 * @def SUPERVISION_TIMEOUT
 * @brief Defines the supervision timeout used in BLE communication.
 *
 * Represents the supervision timeout with a value of 0x07D0 (2000).
 * 
 * The supervision timeout is the maximum time, in milliseconds, that a 
 * connection can remain idle before being considered lost and terminated.
 */
#ifndef SUPERVISION_TIMEOUT
#define SUPERVISION_TIMEOUT 0x07D0 //2000
#endif

/*=======================================================================*/
// Scan command parameters
/*=======================================================================*/
//Scan status
/**
 * @def RSI_BLE_START_SCAN
 * @brief Defines the command to start scanning for BLE devices.
 *
 * Represents the command to initiate scanning with a value of 0x01.
 */
#ifndef RSI_BLE_START_SCAN
#define RSI_BLE_START_SCAN 0x01
#endif
/**
 * @def RSI_BLE_STOP_SCAN
 * @brief Defines the command to stop scanning for BLE devices.
 *
 * Represents the command to stop scanning with a value of 0x00.
 */
#ifndef RSI_BLE_STOP_SCAN
#define RSI_BLE_STOP_SCAN 0x00
#endif
//Scan types
/**
 * @def SCAN_TYPE_ACTIVE
 * @brief Defines the active scan type for BLE communication.
 *
 * In active scanning, the scanner sends scan request packets to 
 * advertising devices to obtain additional information.
 */
#ifndef SCAN_TYPE_ACTIVE
#define SCAN_TYPE_ACTIVE 0x01
#endif
/**
 * @def SCAN_TYPE_PASSIVE
 * @brief Defines the passive scan type for BLE communication.
 *
 * In passive scanning, the scanner listens for advertising packets
 * without sending scan request packets to the advertising devices.
 */
#define SCAN_TYPE_PASSIVE 0x00

#ifndef SCAN_TYPE_PASSIVE
#define SCAN_TYPE_PASSIVE 0x00
#endif
//Scan filters
/**
 * @def SCAN_FILTER_TYPE_ALL
 * @brief Defines the scan filter type to accept all advertising packets.
 *
 * Configures the scanner to accept all advertising packets,
 * regardless of their address or data content.
 */
#ifndef SCAN_FILTER_TYPE_ALL
#define SCAN_FILTER_TYPE_ALL 0x00
#endif
/**
 * @def SCAN_FILTER_TYPE_ONLY_ACCEPT_LIST
 * @brief Defines the scan filter type to accept only advertising packets from devices in the accept list.
 *
 * Configures the scanner to accept advertising packets only from devices
 * that are included in the accept list, ignoring packets from other devices.
 */
#ifndef SCAN_FILTER_TYPE_ONLY_ACCEPT_LIST
#define SCAN_FILTER_TYPE_ONLY_ACCEPT_LIST 0x01
#endif
/**
 * @def RSI_BLE_SCAN_TYPE
 * @brief Configures the BLE scan type to active scanning.
 *
 * Sets the scan type to active, where the scanner sends scan request packets 
 * to advertising devices to obtain additional information.
 * 
 * Uses the `SCAN_TYPE_ACTIVE` setting.
 */
#ifndef RSI_BLE_SCAN_TYPE
#define RSI_BLE_SCAN_TYPE SCAN_TYPE_ACTIVE
#endif
/**
 * @def RSI_BLE_SCAN_FILTER_TYPE
 * @brief Configures the BLE scan filter to accept all advertising packets.
 *
 * Sets the scan filter type to accept all advertising packets,
 * regardless of their address or data content.
 *
 * Uses the `SCAN_FILTER_TYPE_ALL` setting.
 */
#ifndef RSI_BLE_SCAN_FILTER_TYPE
#define RSI_BLE_SCAN_FILTER_TYPE SCAN_FILTER_TYPE_ALL
#endif
/*-------------------------------------------------------------------------------------------*/
/**
 * @def BLE_DISABLE_DUTY_CYCLING
 * @brief Disables duty cycling for BLE operations.
 *
 * Configures the device to disable duty cycling, which means that BLE
 * operations would run continuously without the power-saving sleep cycles.
 */
#ifndef BLE_DISABLE_DUTY_CYCLING
#define BLE_DISABLE_DUTY_CYCLING 0
#endif
/**
 * @def BLE_DUTY_CYCLING
 * @brief Enables duty cycling for BLE operations.
 *
 * Configures the device to enable duty cycling, which means that BLE
 * operations would include power-saving sleep cycles to conserve energy.
 */
#ifndef BLE_DUTY_CYCLING
#define BLE_DUTY_CYCLING 1
#endif
/**
 * @def BLR_DUTY_CYCLING
 * @brief Enables duty cycling for BLE operations.
 *
 * Configures the device to enable duty cycling, which includes power-saving
 * sleep cycles to conserve energy. The value 2 indicates the specific duty cycling mode.
 */
#ifndef BLR_DUTY_CYCLING
#define BLR_DUTY_CYCLING 2
#endif
/**
 * @def BLE_4X_PWR_SAVE_MODE
 * @brief Enables power save mode for BLE 4.x operations.
 *
 */
#ifndef BLE_4X_PWR_SAVE_MODE
#define BLE_4X_PWR_SAVE_MODE 4
#endif
/**
 * @def RSI_BLE_PWR_SAVE_OPTIONS
 * @brief Configures power save options for BLE operations.
 *
 * Disables duty cycling to keep the device active and avoid power-saving sleep cycles.
 */
#ifndef RSI_BLE_PWR_SAVE_OPTIONS
#define RSI_BLE_PWR_SAVE_OPTIONS BLE_DISABLE_DUTY_CYCLING
#endif
/*-------------------------------------------------------------------------------------------*/
/**
 * @def RSI_SEL_INTERNAL_ANTENNA
 * @brief Selects the internal antenna for BLE operations.
 *
 * Configures the device to use the internal antenna. The value 0x00 indicates
 * the selection of the internal antenna.
 */
#ifndef RSI_SEL_INTERNAL_ANTENNA
#define RSI_SEL_INTERNAL_ANTENNA 0x00
#endif
/**
 * @def RSI_SEL_EXTERNAL_ANTENNA
 * @brief Selects the external antenna for BLE operations.
 *
 * Configures the device to use the external antenna. The value 0x01 indicates
 * the selection of the external antenna.
 */
#ifndef RSI_SEL_EXTERNAL_ANTENNA
#define RSI_SEL_EXTERNAL_ANTENNA 0x01
#endif
#endif

#ifndef RSI_SEL_ANTENNA
#define RSI_SEL_ANTENNA \
  RSI_SEL_INTERNAL_ANTENNA ///< Antenna Selection - RSI_SEL_EXTERNAL_ANTENNA / RSI_SEL_INTERNAL_ANTENNA
#endif
/*-------------------------------------------------------------------------------------------*/
/**
 * @def RSI_BLE_GATT_ASYNC_ENABLE
 * @brief Enables or disables asynchronous GATT operations.
 *
 * Configures the device to enable or disable asynchronous GATT operations.
 * The value 0 indicates that asynchronous GATT operations are disabled.
 */
#ifndef RSI_BLE_GATT_ASYNC_ENABLE
#define RSI_BLE_GATT_ASYNC_ENABLE 0
#endif
/**
 * @def RSI_BLE_GATT_INIT
 * @brief Initializes the GATT in Firmware for BLE operations.
 *
 * 0 - GATT Init in Firmware: Both the GAP service and GATT service would be maintained by Firmware. 
 * 1 - GATT Init in Host: GAP service and GATT service should be created by the APP/Host/User, 
 * and the ATT transactions like read, write, notify, and indicate should be handled by the 
 * APP/Host/User. Default: GATT Init in Firmware.
 */
#ifndef RSI_BLE_GATT_INIT
#define RSI_BLE_GATT_INIT 0
#endif
/**
 * @def RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST
 * @brief Configures indication confirmation from the host.
 *
 * When this macro is enabled, the host needs to provide confirmation for the indicatation.
 * If it is disabled, the firmware would send the confirmation automatically.
 */
#ifndef RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST
#define RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST 0
#endif
/**
 * @def RSI_BLE_MTU_EXCHANGE_FROM_HOST
 * @brief Configures whether the MTU exchange process is initiated by the host.
 *
 * If this macro is disabled, the firmware would initiate the MTU request to the remote device on the successful connection.
 * And if Peer initiates MTU exchange Request, then firmware will send Exchange MTU Response in reply to a received Exchange MTU Request. 
 * If this macro is enabled then APP/Host/User needs to initiate the MTU request by using the rsi_ble_mtu_exchange_event API. 
 * And if Peer initiates MTU exchange Request, then APP/Host/User should send Exchange MTU Response in reply to a received Exchange MTU Request using rsi_ble_mtu_exchange_resp API.
 */
#ifndef RSI_BLE_MTU_EXCHANGE_FROM_HOST
#define RSI_BLE_MTU_EXCHANGE_FROM_HOST 0
#endif
/**
 * @def RSI_BLE_SET_SCAN_RESP_DATA_FROM_HOST
 * @brief Configures scan response data from the host.
 *
 * Device would maintains some default scan response data and would be used in the scan_response controller frame. 
 * By enabling this bit we can make the default data as Null (empty) in the controller and set scan resp data from APP/Host/User.
 * 
 */
#ifndef RSI_BLE_SET_SCAN_RESP_DATA_FROM_HOST
#define RSI_BLE_SET_SCAN_RESP_DATA_FROM_HOST 0
#endif
/**
 * @def RSI_BLE_DISABLE_CODED_PHY_FROM_HOST
 * @brief Configures the disabling of coded PHY from the host.
 *
 * Device will support the LE-coded phy feature (that is, LR - 125 kbps and 500 kbps) by default. 
 * If this bit is enabled, the device will not the support the LE-coded phy rates.
 */
#ifndef RSI_BLE_DISABLE_CODED_PHY_FROM_HOST
#define RSI_BLE_DISABLE_CODED_PHY_FROM_HOST 0
#endif
/*-------------------------------------------------------------------------------------------*/
// Advertising command parameters
#ifndef RSI_BLE_ADV_INT_MIN
#define RSI_BLE_ADV_INT_MIN 0x100 ///< Minimum advertising interval
#endif
#ifndef RSI_BLE_ADV_INT_MAX
#define RSI_BLE_ADV_INT_MAX 0x200 ///< Maximum advertising interval
#endif

#ifndef CONNECTION_INTERVAL_MIN
#define CONNECTION_INTERVAL_MIN 0x00A0 ///< Minimum connection interval
#endif
#ifndef CONNECTION_INTERVAL_MAX
#define CONNECTION_INTERVAL_MAX 0x00A0 ///< Maximum connection interval
#endif

#ifndef LE_SCAN_INTERVAL
#define LE_SCAN_INTERVAL 0x0100 ///< Scan interval
#endif
#ifndef LE_SCAN_WINDOW
#define LE_SCAN_WINDOW 0x0050 ///< Scan window
#endif

/*=======================================================================*/
// Extended Advertising parameters
/*=======================================================================*/
#ifndef RSI_BLE_ENABLE_ADV_EXTN
#define RSI_BLE_ENABLE_ADV_EXTN 0 ///< disabled by default
#endif

#ifndef RSI_BLE_AE_MAX_ADV_SETS
#define RSI_BLE_AE_MAX_ADV_SETS 2 ///< default number of Advertising sets in extended advertising (Max value = 3)
#endif
/*=======================================================================*/
// Extended features
/*=======================================================================*/

#ifndef RSI_BT_BLE_STACK_BYPASS_ENABLE
#define RSI_BT_BLE_STACK_BYPASS_ENABLE \
  0 // 1 - enable BT stack bypass feature 0 - disable // by default it is 0 - disabled
#endif
/** @} */
