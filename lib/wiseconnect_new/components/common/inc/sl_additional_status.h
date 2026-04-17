/********************************************************************************
 * @file  sl_additional_status.h
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
#pragma once

// To do: Need to reserve the space in sl_status.h. Ranging from 0x10000 to 0x1FFFF.
#define SL_STATUS_SI91X_SUBSPACE ((sl_status_t)0x00010000)

/**
 * @addtogroup WISECONNECT_STATUS_CODES
 * @{ 
 */

// Additional generic errors
// Reserving the space in between SL_STATUS_SI91X_SUBSPACE to write the overlapping status codes.
// Range of these status codes will be from 0x16000 to 0x16FFF.
#define SL_STATUS_OS_OPERATION_FAILURE            ((sl_status_t)0x16051) ///< OS operation failed.
#define SL_STATUS_BOOTUP_OPTIONS_NOT_SAVED        ((sl_status_t)0x16052) ///< Bootup options were not saved.
#define SL_STATUS_BOOTUP_OPTIONS_CHECKSUM_FAILURE ((sl_status_t)0x16053) ///< Bootup options checksum validation failed.
#define SL_STATUS_BOOTLOADER_VERSION_MISMATCH \
  ((sl_status_t)0x16054) ///< Bootloader version does not match expected version.
#define SL_STATUS_WAITING_FOR_BOARD_READY    ((sl_status_t)0x16055) ///< Waiting for the board to be ready.
#define SL_STATUS_VALID_FIRMWARE_NOT_PRESENT ((sl_status_t)0x16056) ///< No valid firmware present.
#define SL_STATUS_INVALID_OPTION             ((sl_status_t)0x16057) ///< Provided option is invalid.
#define SL_STATUS_SPI_BUSY                   ((sl_status_t)0x16058) ///< SPI is currently busy.
#define SL_STATUS_CARD_READY_TIMEOUT         ((sl_status_t)0x16059) ///< Timeout waiting for card to be ready.
#define SL_STATUS_FW_LOAD_OR_UPGRADE_TIMEOUT ((sl_status_t)0x1605A) ///< Firmware load or upgrade operation timed out.

// Additional Wi-Fi errors
#define SL_STATUS_WIFI_DOES_NOT_EXIST    ((sl_status_t)0x0B21) ///< WiFi network does not exist.
#define SL_STATUS_WIFI_NOT_AUTHENTICATED ((sl_status_t)0x0B22) ///< WiFi network is not authenticated.
#define SL_STATUS_WIFI_NOT_KEYED \
  ((sl_status_t)0x0B23) ///< Device attempts to connect to a secured WiFi network without providing the correct security key or credentials.
#define SL_STATUS_WIFI_IOCTL_FAIL                   ((sl_status_t)0x0B24) ///< Input/Output Control operation failed.
#define SL_STATUS_WIFI_BUFFER_UNAVAILABLE_TEMPORARY ((sl_status_t)0x0B25) ///< WiFi buffer temporarily unavailable.
#define SL_STATUS_WIFI_BUFFER_UNAVAILABLE_PERMANENT ((sl_status_t)0x0B26) ///< WiFi buffer permanently unavailable.
#define SL_STATUS_WIFI_WPS_PBC_OVERLAP              ((sl_status_t)0x0B27) ///< WPS PBC overlap detected.
#define SL_STATUS_WIFI_OUT_OF_EVENT_HANDLER_SPACE   ((sl_status_t)0x0B29) ///< No space for additional event handlers.
#define SL_STATUS_WIFI_SEMAPHORE_ERROR              ((sl_status_t)0x0B2A) ///< Semaphore manipulation error.
#define SL_STATUS_WIFI_FLOW_CONTROLLED              ((sl_status_t)0x0B2B) ///< Packet retrieval cancelled due to flow control.
#define SL_STATUS_WIFI_NO_CREDITS                   ((sl_status_t)0x0B2C) ///< Packet retrieval cancelled due to lack of bus credits.
#define SL_STATUS_WIFI_NO_PACKET_TO_SEND \
  ((sl_status_t)0x0B2D) ///< Packet retrieval cancelled due to no pending packets.
#define SL_STATUS_WIFI_CORE_CLOCK_NOT_ENABLED ((sl_status_t)0x0B2E) ///< WiFi core disabled due to no clock.
#define SL_STATUS_WIFI_CORE_IN_RESET          ((sl_status_t)0x0B2F) ///< WiFi core disabled because it is in reset state.
#define SL_STATUS_WIFI_UNSUPPORTED            ((sl_status_t)0x0B30) ///< Unsupported WiFi function.
#define SL_STATUS_WIFI_BUS_WRITE_REGISTER_ERROR \
  ((sl_status_t)0x0B31)                                          ///< Error occurred while writing to the WLAN register.
#define SL_STATUS_WIFI_SDIO_BUS_UP_FAIL    ((sl_status_t)0x0B32) ///< SDIO bus failed to initialize.
#define SL_STATUS_WIFI_JOIN_IN_PROGRESS    ((sl_status_t)0x0B33) ///< WiFi join operation is still in progress.
#define SL_STATUS_WIFI_NETWORK_NOT_FOUND   ((sl_status_t)0x0B34) ///< Specified WiFi network was not found.
#define SL_STATUS_WIFI_INVALID_JOIN_STATUS ((sl_status_t)0x0B35) ///< Invalid status encountered during WiFi join.
#define SL_STATUS_WIFI_UNKNOWN_INTERFACE   ((sl_status_t)0x0B36) ///< Unknown WiFi interface specified.
#define SL_STATUS_WIFI_SDIO_RX_FAIL        ((sl_status_t)0x0B37) ///< Error occurred during SDIO receive operation.
#define SL_STATUS_WIFI_HWTAG_MISMATCH      ((sl_status_t)0x0B38) ///< Hardware tag header is corrupt.
#define SL_STATUS_WIFI_RX_BUFFER_ALLOC_FAIL \
  ((sl_status_t)0x0B39) ///< Failed to allocate a buffer for WiFi receive operation.
#define SL_STATUS_WIFI_BUS_READ_REGISTER_ERROR \
  ((sl_status_t)0x0B3A) ///< Error occurred while reading a bus hardware register.
#define SL_STATUS_WIFI_THREAD_CREATE_FAILED ((sl_status_t)0x0B3B) ///< Failed to create a new WiFi thread.
#define SL_STATUS_WIFI_QUEUE_ERROR          ((sl_status_t)0x0B3C) ///< Error occurred while manipulating a WiFi queue.
#define SL_STATUS_WIFI_BUFFER_POINTER_MOVE_ERROR \
  ((sl_status_t)0x0B3D) ///< Error occurred while moving the current pointer of a buffer.

#define SL_STATUS_WIFI_BUFFER_SIZE_SET_ERROR \
  ((sl_status_t)0x0B3E) ///< Error occurred while setting the size of the packet buffer.

#define SL_STATUS_WIFI_THREAD_STACK_NULL \
  ((sl_status_t)0x0B3F) ///< Null stack pointer passed when a non-null pointer was required.
#define SL_STATUS_WIFI_THREAD_DELETE_FAIL ((sl_status_t)0x0B40) ///< Error occurred while deleting a WiFi thread.
#define SL_STATUS_WIFI_SLEEP_ERROR        ((sl_status_t)0x0B41) ///< Failed to put a WiFi thread to sleep.
#define SL_STATUS_WIFI_BUFFER_ALLOC_FAIL  ((sl_status_t)0x0B42) ///< Failed to allocate a WiFi packet buffer.
#define SL_STATUS_WIFI_INTERFACE_NOT_UP   ((sl_status_t)0x0B44) ///< The requested WiFi interface is not active.
#define SL_STATUS_WIFI_DELAY_TOO_LONG     ((sl_status_t)0x0B45) ///< The requested delay for WiFi operation is too long.
#define SL_STATUS_WIFI_INVALID_DUTY_CYCLE \
  ((sl_status_t)0x0B46) ///< The specified WiFi duty cycle is outside the valid range.
#define SL_STATUS_WIFI_PMK_WRONG_LENGTH \
  ((sl_status_t)0x0B47) ///< The returned Pairwise Master Key (PMK) length is incorrect.
#define SL_STATUS_WIFI_UNKNOWN_SECURITY_TYPE \
  ((sl_status_t)0x0B48) ///< The security type of the WiFi Access Point is unknown.
#define SL_STATUS_WIFI_WEP_NOT_ALLOWED \
  ((sl_status_t)0x0B49) ///< WEP security is not allowed; use Open security instead.
#define SL_STATUS_WIFI_WPA_KEYLEN_BAD    ((sl_status_t)0x0B4A) ///< WPA/WPA2 key length must be between 8 and 64 bytes.
#define SL_STATUS_WIFI_FILTER_NOT_FOUND  ((sl_status_t)0x0B4B) ///< The specified WiFi filter ID was not found.
#define SL_STATUS_WIFI_SPI_ID_READ_FAIL  ((sl_status_t)0x0B4C) ///< Failed to read the SPI ID (0xfeedbead) from the chip.
#define SL_STATUS_WIFI_SPI_SIZE_MISMATCH ((sl_status_t)0x0B4D) ///< Size mismatch between SPI and SDPCM header.
#define SL_STATUS_WIFI_ADDRESS_ALREADY_REGISTERED \
  ((sl_status_t)0x0B4E) ///< Attempted to register a multicast address that is already registered.
#define SL_STATUS_WIFI_SDIO_RETRIES_EXCEEDED \
  ((sl_status_t)0x0B4F) ///< SDIO transfer failed after exceeding the maximum number of retries.
#define SL_STATUS_WIFI_NULL_PTR_ARG            ((sl_status_t)0x0B50) ///< Null pointer argument passed to a function.
#define SL_STATUS_WIFI_THREAD_FINISH_FAIL      ((sl_status_t)0x0B51) ///< Error occurred while attempting to delete a thread.
#define SL_STATUS_WIFI_WAIT_ABORTED            ((sl_status_t)0x0B52) ///< Semaphore or mutex wait operation was aborted.
#define SL_STATUS_WIFI_QUEUE_MESSAGE_UNALIGNED ((sl_status_t)0x0B53) ///< Unaligned message found in the queue.
#define SL_STATUS_WIFI_MUTEX_ERROR             ((sl_status_t)0x0B54) ///< Error occurred during a mutex operation.
#define SL_STATUS_WIFI_SECURE_LINK_DECRYPT_ERROR \
  ((sl_status_t)0x0B57) ///< Error occurred during decryption over a secure link.
#define SL_STATUS_WIFI_SECURE_LINK_KEY_RENEGOTIATION_ERROR \
  ((sl_status_t)0x0B59) ///< Error occurred during key renegotiation over a secure link.
#define SL_STATUS_WIFI_INVALID_OPERMODE          ((sl_status_t)0x0B60) ///< Invalid operation mode provided.
#define SL_STATUS_WIFI_INVALID_ENCRYPTION_METHOD ((sl_status_t)0x0B61) ///< Invalid encryption method provided.

// Si91X Crypto errors
#define SL_STATUS_TRNG_DUPLICATE_ENTROPY ((sl_status_t)0x0B62) ///< Error due to duplicate entropy elements in TRNG.

// Si91X Wi-Fi transceiver error codes
#define SL_STATUS_TRANSCEIVER_INVALID_MAC_ADDRESS  ((sl_status_t)0x0B63) ///< Provided MAC address is invalid.
#define SL_STATUS_TRANSCEIVER_INVALID_QOS_PRIORITY ((sl_status_t)0x0B64) ///< Provided QoS priority is invalid.
#define SL_STATUS_TRANSCEIVER_INVALID_CHANNEL      ((sl_status_t)0x0B65) ///< Provided channel is invalid.
#define SL_STATUS_TRANSCEIVER_INVALID_DATA_RATE    ((sl_status_t)0x0B66) ///< Provided data rate is invalid.
#define SL_STATUS_TRANSCEIVER_INVALID_CONFIG \
  ((sl_status_t)0x0B67) ///< Provided transceiver configuration parameters are invalid.

// Si91X Crypto Firmware Errors
#define SL_STATUS_CRYPTO_INVALID_PARAMETER \
  ((sl_status_t)0x1CCFE) ///< Return when parameter passed to Crypto SAPI is invalid.
#define SL_STATUS_CRYPTO_INVALID_SIGNATURE \
  ((sl_status_t)0x1CC9A) ///< Return in AEAD (CCM, GCM, Chachapoly) decryption function, when MAC generated during decryption does not match the MAC passed.

// Si91X Wi-Fi Firmware errors
#define SL_STATUS_WIFI_CONNECTION_LOST ((sl_status_t)0x10B28) ///< WiFi connection lost.
#define SL_STATUS_SI91X_SCAN_ISSUED_IN_ASSOCIATED_STATE \
  ((sl_status_t)0x10002) ///< Scan command issued while device is already associated with an access point.
#define SL_STATUS_SI91X_NO_AP_FOUND ((sl_status_t)0x10003) ///< No access point found.
#define SL_STATUS_SI91X_INVALID_PSK_IN_WEP_SECURITY \
  ((sl_status_t)0x10004) ///< Incorrect PSK provided while trying to join an access point with WEP security enabled.
#define SL_STATUS_SI91X_INVALID_BAND ((sl_status_t)0x10005) ///< Invalid band specified.
#define SL_STATUS_SI91X_UNASSOCIATED \
  ((sl_status_t)0x10006) ///< Device is not associated or is in an unassociated state.
#define SL_STATUS_SI91X_DEAUTHENTICATION_RECEIVED_FROM_AP \
  ((sl_status_t)0x10008) ///< De-authentication received from access point.
#define SL_STATUS_SI91X_ASSOCIATION_FAILED \
  ((sl_status_t)0x10009) ///< Failed to associate with the access point during "Join".
#define SL_STATUS_SI91X_INVALID_CHANNEL ((sl_status_t)0x1000A) ///< Invalid channel specified.
#define SL_STATUS_SI91X_JOIN_AUTHENTICATION_FAILED \
  ((sl_status_t)0x1000E) ///< Authentication failure during "Join". Unable to find access point during join which was found during scan.
#define SL_STATUS_SI91X_BEACON_MISSED_FROM_AP_DURING_JOIN \
  ((sl_status_t)0x1000F) ///< Missed beacon from access point during join.
#define SL_STATUS_SI91X_INVALID_MAC_SUPPLIED \
  ((sl_status_t)0x10013) ///< Non-existent MAC address supplied in "Disassociate" command.
#define SL_STATUS_SI91X_EAP_CONFIG_NOT_DONE ((sl_status_t)0x10014) ///< EAP configuration is not completed.
#define SL_STATUS_SI91X_MEMORY_FAILED_FROM_MODULE \
  ((sl_status_t)0x10015) ///< Memory allocation failed or store configuration checksum failed.
#define SL_STATUS_SI91X_INSUFFICIENT_INFO \
  ((sl_status_t)0x10016) ///< Insufficient or incorrect information provided in join command.
#define SL_STATUS_SI91X_NOT_AP_INTERFACE ((sl_status_t)0x10017) ///< Not an access point interface.
#define SL_STATUS_SI91X_INVALID_PUSH_BUTTON_SEQUENCE \
  ((sl_status_t)0x10018) ///< Push button command issued before the expiry of the previous push button command.
#define SL_STATUS_SI91X_REJOIN_FAILURE          ((sl_status_t)0x10019) ///< Rejoin failure: Access point not found.
#define SL_STATUS_SI91X_FREQUENCY_NOT_SUPPORTED ((sl_status_t)0x1001A) ///< Operation failed: Frequency not supported.
#define SL_STATUS_SI91X_INVALID_OPERMODE        ((sl_status_t)0x1001B) ///< Operation failed: Invalid operation mode.
#define SL_STATUS_SI91X_EAP_CONFIG_FAILED       ((sl_status_t)0x1001C) ///< EAP configuration failed.
#define SL_STATUS_SI91X_P2P_CONFIG_FAILED       ((sl_status_t)0x1001D) ///< P2P configuration failed.
#define SL_STATUS_SI91X_GROUP_OWNER_NEGOTIATION_FAILED \
  ((sl_status_t)0x1001E) ///< Failed to start Group Owner negotiation.
#define SL_STATUS_SI91X_JOIN_TIMEOUT                   ((sl_status_t)0x10020) ///< Join operation timed out.
#define SL_STATUS_SI91X_COMMAND_GIVEN_IN_INVALID_STATE ((sl_status_t)0x10021) ///< Command issued in an invalid state.
#define SL_STATUS_SI91X_INVALID_QUERY_GO_PARAMS \
  ((sl_status_t)0x10022) ///< Invalid parameters for Query GO in the current operating mode.
#define SL_STATUS_SI91X_ACCESS_POINT_FAILED ((sl_status_t)0x10023) ///< Failed to form access point.
#define SL_STATUS_SI91X_INVALID_SCAN_INFO   ((sl_status_t)0x10024) ///< Invalid scan input parameters.
#define SL_STATUS_SI91X_COMMAND_ISSUED_IN_REJOIN_STATE \
  ((sl_status_t)0x10025)                                        ///< Command issued during re-join operation.
#define SL_STATUS_SI91X_WRONG_PARAMETERS ((sl_status_t)0x10026) ///< Incorrect parameters provided for the command.
#define SL_STATUS_SI91X_PROVISION_DISCOVERY_FAILED_IN_P2P \
  ((sl_status_t)0x10027) ///< Provision discovery failed in P2P mode.
#define SL_STATUS_SI91X_INVALID_PSK_LENGTH \
  ((sl_status_t)0x10028) ///< PSK length less than 8 bytes or more than 63 bytes.
#define SL_STATUS_SI91X_FAILED_TO_CLEAR_OR_SET_EAP_CERTIFICATE \
  ((sl_status_t)0x10029) ///< Failed to clear or to set the Enterprise Certificate (Set Certificate).
#define SL_STATUS_SI91X_P2P_GO_NEGOTIATED_FAILED ((sl_status_t)0x1002A) ///< P2P Group Owner negotiation failed.
#define SL_STATUS_SI91X_ASSOCIATION_TIMEOUT_IN_P2P_WPS_MODE \
  ((sl_status_t)0x1002B) ///< Association between nodes failed in P2P WPS mode due to timeout.
#define SL_STATUS_SI91X_COMMAND_ISSUED_WHILE_INTERNAL_OPERATION \
  ((sl_status_t)0x1002C) ///< Command issued by the Host while the device is executing an internal operation (for example, auto-join or auto-create).
#define SL_STATUS_SI91X_INVALID_WEP_KEY_LEN        ((sl_status_t)0x1002D) ///< Invalid WEP key length.
#define SL_STATUS_SI91X_ICMP_REQUEST_TIMEOUT_ERROR ((sl_status_t)0x1002E) ///< ICMP request timed out.
#define SL_STATUS_SI91X_ICMP_DATA_SIZE_EXCEED_MAX_LIMIT \
  ((sl_status_t)0x1002F) ///< ICMP data size exceeds the maximum limit.
#define SL_STATUS_SI91X_SEND_DATA_PACKET_EXCEED_LIMIT \
  ((sl_status_t)0x10030) ///< Data packet size exceeded the limit, MQTT published the data and length mismatched, or MQTT send data packet exceeded the limit.
#define SL_STATUS_SI91X_ARP_CACHE_NOT_FOUND  ((sl_status_t)0x10031) ///< ARP cache entry not found.
#define SL_STATUS_SI91X_UART_COMMAND_TIMEOUT ((sl_status_t)0x10032) ///< UART command timed out.
#define SL_STATUS_SI91X_FIXED_DATA_RATE_NOT_SUPPORTED_BY_AP \
  ((sl_status_t)0x10033) ///< Fixed data rate not supported by the connected AP.
#define SL_STATUS_SI91X_USERNAME_PASSWORD_CLIENTID_TOPIC_MAX_LEN \
  ((sl_status_t)0x10036) ///< Maximum length exceeded for username, password, client ID, or topic in MQTT.
#define SL_STATUS_SI91X_INVALID_WPS_PIN     ((sl_status_t)0x10037) ///< Invalid WPS PIN.
#define SL_STATUS_SI91X_INVALID_WPS_PIN_LEN ((sl_status_t)0x10038) ///< Invalid WPS PIN length.
#define SL_STATUS_SI91X_INVALID_PMK_LEN     ((sl_status_t)0x10039) ///< Invalid PMK length.
#define SL_STATUS_SI91X_SSID_NOT_PRESENT_FOR_PMK_GENERATION \
  ((sl_status_t)0x1003A) ///< SSID not present for PMK generation.
#define SL_STATUS_SI91X_SSID_INCORRECT_PMK_GENERATION \
  ((sl_status_t)0x1003B) ///< SSID incorrect for PMK generation(more than 32 bytes).
#define SL_STATUS_SI91X_BAND_NOT_SUPPORTED ((sl_status_t)0x1003C) ///< Band not supported.
#define SL_STATUS_SI91X_INVALID_USR_STORE_CONFIGURATION_LEN \
  ((sl_status_t)0x1003D) ///< Invalid user store configuration length.
#define SL_STATUS_SI91X_INVALID_COMMAND_LEN \
  ((sl_status_t)0x1003E) ///< Invalid command length (exceeds the number of characters specified in the PRM).
#define SL_STATUS_SI91X_DATA_PACKET_DROPPED          ((sl_status_t)0x1003F) ///< Data packet dropped.
#define SL_STATUS_SI91X_WEP_KEY_NOT_GIVEN            ((sl_status_t)0x10040) ///< WEP key not provided.
#define SL_STATUS_SI91X_INVALID_STORE_CONFIG_PROFILE ((sl_status_t)0x10041) ///< Store config profile length error.
#define SL_STATUS_SI91X_MISSING_PSK_OR_PMK           ((sl_status_t)0x10042) ///< PSK or PMK not provided.
#define SL_STATUS_SI91X_INVALID_SECURITY_MODE_IN_JOIN_COMMAND \
  ((sl_status_t)0x10043) ///< Invalid security mode in join command.
#define SL_STATUS_SI91X_MAX_BEACON_MISCOUNT \
  ((sl_status_t)0x10044) ///< Maximum beacon miscount reached: De-authentication due to beacon miss.
#define SL_STATUS_SI91X_DEAUTH_REQUEST_FROM_SUPPLICANT \
  ((sl_status_t)0x10045) ///< De-authentication request received from supplicant.
#define SL_STATUS_SI91X_DEAUTH_REQUEST_FROM_FROM_AP \
  ((sl_status_t)0x10046) ///< De-authentication request received from AP after channel switching.
#define SL_STATUS_SI91X_MISSED_SYNCHRONIZATION ((sl_status_t)0x10047) ///< Synchronization missed.
#define SL_STATUS_SI91X_AUTHENTICATION_TIMEOUT ((sl_status_t)0x10048) ///< Authentication timeout occurred.
#define SL_STATUS_SI91X_ASSOCIATION_TIMEOUT    ((sl_status_t)0x10049) ///< Association timeout.
#define SL_STATUS_SI91X_BG_SCAN_NOT_ALLOWED    ((sl_status_t)0x1004A) ///< BG scan in given channels is not allowed.
#define SL_STATUS_SI91X_SSID_MISMATCH          ((sl_status_t)0x1004B) ///< Scanned SSID and SSID given in join are not matching.
#define SL_STATUS_SI91X_CLIENT_MAX_SUPPORTED_EXCEEDED \
  ((sl_status_t)0x1004C) ///< The number of clients exceeds the maximum number of supported stations.
#define SL_STATUS_SI91X_HT_CAPABILITIES_NOT_SUPPORTED \
  ((sl_status_t)0x1004D) ///< The specified HT capabilities are not supported.
#define SL_STATUS_SI91X_UART_FLOW_NOT_SUPPORTED ((sl_status_t)0x1004E) ///< UART flow control is not supported.
#define SL_STATUS_SI91X_ZB_BT_BLE_PKT_RECEIVED \
  ((sl_status_t)0x1004F) ///< ZB/BT/BLE packet received, but the protocol is not enabled.
#define SL_STATUS_SI91X_MGMT_PKT_DROPPED        ((sl_status_t)0x10050) ///< Management packet dropped.
#define SL_STATUS_SI91X_INVALID_RF_CURRENT_MODE ((sl_status_t)0x10051) ///< The current RF mode is invalid.
#define SL_STATUS_SI91X_POWER_SAVE_NOT_SUPPORTED \
  ((sl_status_t)0x10052) ///< Power save is not supported for the specified interface.
#define SL_STATUS_SI91X_CONCURRENT_AP_IN_CONNECTED_STATE \
  ((sl_status_t)0x10053) ///< Concurrent AP is in a connected state.
#define SL_STATUS_SI91X_CONNECTED_AP_OR_STATION_CHANNEL_MISMATCH \
  ((sl_status_t)0x10054) ///< Channel mismatch between connected AP and station.
#define SL_STATUS_SI91X_IAP_COPROCESSOR_ERROR ((sl_status_t)0x10055) ///< IAP coprocessor error occurred.
#define SL_STATUS_SI91X_WPS_NOT_SUPPORTED \
  ((sl_status_t)0x10056) ///< WPS is not supported in the current operating mode.
#define SL_STATUS_SI91X_CONCURRENT_AP_CHANNEL_MISMATCH \
  ((sl_status_t)0x10057) ///< Concurrent AP channel does not match the connected station channel.
#define SL_STATUS_SI91X_PBC_SESSION_OVERLAP       ((sl_status_t)0x10058) ///< PBC session overlap error.
#define SL_STATUS_SI91X_BT_FEATURE_BITMAP_INVALID ((sl_status_t)0x10059) ///< The Bluetooth feature bitmap is invalid.
#define SL_STATUS_SI91X_FOUR_WAY_HANDSHAKE_FAILED \
  ((sl_status_t)0x1005A) ///< The 4/4 confirmation of the four-way handshake failed.
#define SL_STATUS_SI91X_MAC_ADDRESS_NOT_PRESENT_IN_MAC_JOIN \
  ((sl_status_t)0x1005B) ///< MAC address is not present in the MAC-based join.
#define SL_STATUS_SI91X_CONCURRENT_MODE_DOWN \
  ((sl_status_t)0x1005C) ///< Concurrent mode requires both AP and client to be up for configuration.
#define SL_STATUS_SI91X_CERTIFICATE_LOAD_NOT_ALLOWED_IN_FLASH \
  ((sl_status_t)0x1005D) ///< Certificate load operation is not allowed in flash memory.
#define SL_STATUS_SI91X_CERTIFICATE_LOAD_NOT_ALLOWED_IN_RAM \
  ((sl_status_t)0x1005E) ///< Certificate load operation is not allowed in RAM.
#define SL_STATUS_SI91X_WRONG_CERTIFICATE_LOAD_INDEX \
  ((sl_status_t)0x1005F) ///< Certificate load failed due to an incorrect index.
#define SL_STATUS_SI91X_AP_HT_CAPS_NOT_ENABLED ((sl_status_t)0x10060) ///< Access Point HT capabilities are not enabled.
#define SL_STATUS_SI91X_ADDRESS_FAMILY_NOT_SUPPORTED \
  ((sl_status_t)0x10061) ///< The specified address family is not supported by the protocol.
#define SL_STATUS_SI91X_INVALID_BEACON_INTERVAL_OR_DTM_PERIOD \
  ((sl_status_t)0x10062) ///< The provided beacon interval or DTIM period is invalid.
#define SL_STATUS_SI91X_INVALID_CONFIG_RANGE_PROVIDED \
  ((sl_status_t)0x10063) ///< The provided configuration range is invalid.
#define SL_STATUS_SI91X_INVALID_CONFIG_TYPE \
  ((sl_status_t)0x10064) ///< The provided configuration type (for example, RTS threshold) is invalid.
#define SL_STATUS_SI91X_ERROR_WITH_MQTT_COMMAND ((sl_status_t)0x10065) ///< An error occurred with the MQTT command.
#define SL_STATUS_SI91X_HIGHER_LISTEN_INTERVAL \
  ((sl_status_t)0x10066) ///< The listen interval in power save mode is greater than that specified in the join command.
#define SL_STATUS_SI91X_WLAN_RADIO_DEREGISTERED ((sl_status_t)0x10067) ///< The WLAN radio has been deregistered.
#define SL_STATUS_SI91X_SAE_FAILURE_DUE_TO_MULTIPLE_CONFIRM_FRAMES_FROM_AP \
  ((sl_status_t)0x10069) ///< SAE authentication failed due to multiple confirm frames received from the Access Point.
#define SL_STATUS_SI91X_EC_GROUP_STATION_UNSUPPORTED_BY_AP \
  ((sl_status_t)0x1006A) ///< The Access Point does not support the EC-group set by the station.
#define SL_STATUS_SI91X_WPS_CREDENTIALS_RECEIVED_WITHOUT_JOIN_COMMAND \
  ((sl_status_t)0x1006B) ///< WPS credentials have been received, but the device did not join the network.
#define SL_STATUS_SI91X_NO_11AX_SUPPORT_IN_AP \
  ((sl_status_t)0x1006C) ///< 802.11ax (Wi-Fi 6) is not supported in Access Point mode.
#define SL_STATUS_SI91X_NON_PREF_CHAN_CONFIG_FAILED \
  ((sl_status_t)0x1006D) ///< Non-preferred channel configuration failed.
#define SL_STATUS_TWT_SUPPORT_NOT_ENABLED_ERR \
  ((sl_status_t)0x10070) ///< Error occurs when HE_PARAMS_SUPPORT and SLI_SI91X_ENABLE_TWT_FEATURE macros are not enabled.
#define SL_STATUS_TWT_SETUP_ERR_SESSION_ACTIVE \
  ((sl_status_t)0x10071) ///< Error occurs when a TWT config command is issued while there is already an active TWT session.
#define SL_STATUS_TWT_TEARDOWN_ERR_FLOWID_NOT_MATCHED \
  ((sl_status_t)0x10072) ///< Error occurs when a TWT teardown command is issued with a flow ID that does not match the existing session flow ID.
#define SL_STATUS_TWT_TEARDOWN_ERR_NOACTIVE_SESS \
  ((sl_status_t)0x10073) ///< Error occurs when a teardown command is issued while there is no active TWT session.
#define SL_STATUS_TWT_SESSION_NOT_FEASIBLE \
  ((sl_status_t)0x10074) ///< Indicates that a TWT session is not feasible. This error is thrown only when the TWT Auto Selection API is used.
#define SL_STATUS_SI91X_RESCHEDULE_TWT_NOT_SUPPORTED \
  ((sl_status_t)0x10075) ///< Error occurs when the AP does not support TWT information frame reception.
#define SL_STATUS_SI91X_RESCHEDULE_TWT_ERR_NOACTIVE_SESS \
  ((sl_status_t)0x10076) ///< Error occurs when there is no active TWT agreement corresponding to the given flow ID.
#define SL_STATUS_SI91X_TWT_RESCHEDULING_IN_PROGRESS \
  ((sl_status_t)0x10077) ///< Indicates that a suspend or resume TWT action is currently in progress.
#define SL_STATUS_SI91X_RESCHEDULE_TWT_PACKET_CREATION_FAILED \
  ((sl_status_t)0x10078) ///< Error occurs when TWT information frame packet creation fails in firmware.
#define SL_STATUS_SI91X_INVALID_STATION_TSF \
  ((sl_status_t)0x10079) ///< Error occurs when the station TSF is invalid, typically when the station is not connected or has not received at least one beacon.
#define SL_STATUS_SI91X_MQTT_ERROR_UNACCEPTABLE_PROTOCOL \
  ((sl_status_t)0x10081) ///< Error occurs when the server does not support the level of the MQTT protocol requested by the client.
#define SL_STATUS_SI91X_MQTT_ERROR_IDENTIFIER_REJECTED \
  ((sl_status_t)0x10082) ///< Error occurs when the client identifier is correct UTF-8 but is not allowed by the server.
#define SL_STATUS_SI91X_MQTT_ERROR_SERVER_UNAVAILABLE \
  ((sl_status_t)0x10083) ///< Error occurs when the network connection has been made but the MQTT service is unavailable.
#define SL_STATUS_SI91X_MQTT_ERROR_BAD_USERNAME_PASSWORD \
  ((sl_status_t)0x10084) ///< The data in the username or password is malformed.
#define SL_STATUS_SI91X_MQTT_ERROR_NOT_AUTHORIZED ((sl_status_t)0x10085) ///< The client is not authorized to connect.
#define SL_STATUS_SI91X_SA_QUERY_TIMEOUT          ((sl_status_t)0x10086) ///< Disconnection due to SA Query Timeout.
#define SL_STATUS_SI91X_TRANSCEIVER_PEER_DS_FEAT_DISABLED \
  ((sl_status_t)0x10096) ///< Feature to add peers in MAC layer is disabled.
#define SL_STATUS_SI91X_TRANSCEIVER_PEER_ALREADY_EXISTS ((sl_status_t)0x10097) ///< Peer already exists in MAC layer.
#define SL_STATUS_SI91X_TRANSCEIVER_MAX_PEER_LIMIT_REACHED \
  ((sl_status_t)0x10098)                                                  ///< Max peer limit reached in MAC layer.
#define SL_STATUS_SI91X_TRANSCEIVER_PEER_NOT_FOUND ((sl_status_t)0x10099) ///< Peer not found in MAC layer.
#define SL_STATUS_SI91X_DUPLICATE_ENTRY_EXISTS_IN_DNS_SERVER_TABLE \
  ((sl_status_t)0x100AF) ///< Duplicate entry exists in DNS server table.
#define SL_STATUS_SI91X_NO_MEM_AVAILABLE                  ((sl_status_t)0x100B1) ///< Memory error: No memory available.
#define SL_STATUS_SI91X_INVALID_CHARACTERS_IN_JSON_OBJECT ((sl_status_t)0x100B2) ///< Invalid characters in JSON object.
#define SL_STATUS_SI91X_NO_KEY_FOUND                      ((sl_status_t)0x100B3) ///< Update commands: No such key found.
#define SL_STATUS_SI91X_NO_FILE_FOUND                     ((sl_status_t)0x100B4) ///< No such file found: Re-check file name.
#define SL_STATUS_SI91X_NO_WEB_PAGE_EXISTS_WITH_SAME_FILENAME \
  ((sl_status_t)0x100B5) ///< No corresponding web page exists with the same filename.
#define SL_STATUS_SI91X_SPACE_UNAVAILABLE_FOR_NEW_FILE ((sl_status_t)0x100B6) ///< Space unavailable for new file.
#define SL_STATUS_SI91X_INVALID_INPUT_DATA \
  ((sl_status_t)0x100C1) ///< Invalid input data, re-check filename, lengths, etc.
#define SL_STATUS_SI91X_NO_SPACE_AVAILABLE_FOR_NEW_FILE ((sl_status_t)0x100C2) ///< Space unavailable for new file.
#define SL_STATUS_SI91X_EXISTING_FILE_OVERWRITE \
  ((sl_status_t)0x100C3) ///< Existing file overwrite: Exceeds size of previous file. Use erase and try again.
#define SL_STATUS_SI91X_NO_SUCH_FILE_FOUND ((sl_status_t)0x100C4) ///< No such file found. Re-check filename.
#define SL_STATUS_SI91X_MEMORY_ERROR       ((sl_status_t)0x100C5) ///< Memory Error: No memory available.
#define SL_STATUS_SI91X_RECEIVED_MORE_WEB_PAGE_DATA \
  ((sl_status_t)0x100C6) ///< Received more web page data than the total length initially specified.
#define SL_STATUS_SI91X_SET_REGION_ERROR ((sl_status_t)0x100C7) ///< Error in set region command.
#define SL_STATUS_SI91X_INVALID_WEBPAGE_CURRENT_CHUNK_LEN \
  ((sl_status_t)0x100C8) ///< Web page current chunk length is incorrect.
#define SL_STATUS_SI91X_AP_SET_REGION_COMMAND_ERROR ((sl_status_t)0x100CA) ///< Error in AP set region command.
#define SL_STATUS_SI91X_AP_SET_REGION_COMMAND_PARAMETERS_ERROR \
  ((sl_status_t)0x100CB)                                                 ///< Error in AP set region command parameters.
#define SL_STATUS_SI91X_REGION_CODE_NOT_SUPPORTED ((sl_status_t)0x100CC) ///< Region code not supported.
#define SL_STATUS_SI91X_EXTRACTING_COUNTRY_REGION_FROM_BEACON_FAILED \
  ((sl_status_t)0x100CD) ///< Error in extracting country region from beacon.
#define SL_STATUS_SI91X_SELECTED_REGION_NOT_SUPPORTED \
  ((sl_status_t)0x100CE) ///< Device does not have selected region support.
#define SL_STATUS_SI91X_HOST_VOLTAGE_OUT_OF_RANGE       ((sl_status_t)0x100D0) ///< Voltage out of range.
#define SL_STATUS_SI91X_SSL_TLS_CONTEXT_CREATION_FAILED ((sl_status_t)0x100D1) ///< SSL/TLS context create failed.
#define SL_STATUS_SI91X_SSL_TLS_HANDSHAKE_FAIL \
  ((sl_status_t)0x100D2) ///< SSL/TLS handshake failed. Socket will be closed.
#define SL_STATUS_SI91X_SSL_TLS_MAX_SOCKETS_REACHED ((sl_status_t)0x100D3) ///< SSL/TLS max sockets reached.
#define SL_STATUS_SI91X_FTP_CLIENT_NOT_CONNECTED    ((sl_status_t)0x100D3) ///< FTP client is not connected.
#define SL_STATUS_SI91X_CIPHER_SET_FAILED           ((sl_status_t)0x100D4) ///< Cipher set failure.
#define SL_STATUS_SI91X_HTTP_CREDENTIALS_MAX_LEN_EXCEEDED \
  ((sl_status_t)0x100F1)                                             ///< HTTP credentials maximum length exceeded.
#define SL_STATUS_SI91X_FEATURE_NOT_SUPPORTED ((sl_status_t)0x100F7) ///< Feature not supported.
#define SL_STATUS_SI91X_FLASH_WRITE_OR_FLASH_DATA_VERIFICATION_FAILED \
  ((sl_status_t)0x100F8) ///< Unable to write to flash OR flash data verification failed.
#define SL_STATUS_SI91X_CALIBRATION_DATA_VERIFICATION_FAILED \
  ((sl_status_t)0x100F9)                                               ///< Calibration data verification failed.
#define SL_STATUS_SI91X_EVM_INDEX_LIMIT_REACHED ((sl_status_t)0x100FD) ///< EVM index limit reached.
#define SL_STATUS_SI91X_EFUSE_ALREADY_POPULATED \
  ((sl_status_t)0x100FA) ///< Efuse data is already populated. Write prohibited.
#define SL_STATUS_SI91X_SNMP_INTERNAL_ERROR      ((sl_status_t)0x10100) ///< SNMP internal error.
#define SL_STATUS_SI91X_SNMP_INVALID_IP_PROTOCOL ((sl_status_t)0x10104) ///< SNMP invalid IP protocol error.
#define SL_STATUS_SI91X_EFUSE_DATA_INVALID       ((sl_status_t)0x10106) ///< Efuse data is invalid.
#define SL_STATUS_SI91X_VENDOR_IE_INVALID_IE_LENGTH \
  ((sl_status_t)0x1010E) ///< Vendor IE buffer length is invalid or exceeds maximum allowed size.
#define SL_STATUS_SI91X_VENDOR_IE_INVALID_UNIQUE_ID \
  ((sl_status_t)0x1010F) ///< Vendor IE unique ID is out of valid range or not found.
#define SL_STATUS_SI91X_VENDOR_IE_INVALID_ACTION \
  ((sl_status_t)0x10110) ///< Vendor IE operation is not supported or invalid action specified.
#define SL_STATUS_SI91X_VENDOR_IE_INVALID_PACKET_BITMAP \
  ((sl_status_t)0x10111) ///< Vendor IE management frame bitmap contains invalid or unsupported frame types.
#define SL_STATUS_SI91X_VENDOR_IE_NO_SLOTS_AVAILABLE \
  ((sl_status_t)0x10112) ///< No firmware memory slots available for adding new vendor IE.
#define SL_STATUS_SI91X_NO_DATA_RECEIVED_OR_RECEIVE_TIMEOUT \
  ((sl_status_t)0x1BB01) ///< No data received or receive timeout.
#define SL_STATUS_SI91X_INSUFFICIENT_DATA_FOR_TIME_CONVERSION \
  ((sl_status_t)0x1BB08) ///< Insufficient data for converting NTP time to mm-dd-yy time format.
#define SL_STATUS_SI91X_INVALID_SNTP_SERVER_ADDRESS ((sl_status_t)0x1BB0A) ///< Invalid SNTP server address.
#define SL_STATUS_SI91X_SNTP_CLIENT_NOT_STARTED     ((sl_status_t)0x1BB0B) ///< SNTP client not started.
#define SL_STATUS_SI91X_SNTP_SERVER_UNAVAILABLE \
  ((sl_status_t)0x1BB10) ///< SNTP server not available. Client will not get any time update service from current server.
#define SL_STATUS_SI91X_SNTP_SERVER_AUTHENTICATION_FAILED ((sl_status_t)0x1BB15) ///< SNTP server authentication failed.
#define SL_STATUS_SI91X_INTERNAL_ERROR                    ((sl_status_t)0x1BB0E) ///< Internal error.
#define SL_STATUS_SI91X_MULTICAST_IP_ADDRESS_ENTRY_NOT_FOUND \
  ((sl_status_t)0x1BB16)                                                  ///< Entry not found for multicast IP address.
#define SL_STATUS_SI91X_MULTICAST_NO_ENTRIES_FOUND ((sl_status_t)0x1BB17) ///< No more entries found for multicast.
#define SL_STATUS_SI91X_IP_ADDRESS_ERROR           ((sl_status_t)0x1BB21) ///< IP address error.
#define SL_STATUS_SI91X_SOCKET_ALREADY_BOUND       ((sl_status_t)0x1BB22) ///< Socket already bound.
#define SL_STATUS_SI91X_PORT_UNAVAILABLE           ((sl_status_t)0x1BB23) ///< Port not available.
#define SL_STATUS_SI91X_SOCKET_NOT_CREATED         ((sl_status_t)0x1BB27) ///< Socket is not created.
#define SL_STATUS_SI91X_ICMP_REQUEST_FAILED        ((sl_status_t)0x1BB29) ///< ICMP request failed.
#define SL_STATUS_SI91X_MAX_LISTEN_SOCKETS_REACHED ((sl_status_t)0x1BB33) ///< Maximum listen sockets reached.
#define SL_STATUS_SI91X_DHCP_DUPLICATE_LISTEN      ((sl_status_t)0x1BB34) ///< DHCP duplicate listen.
#define SL_STATUS_SI91X_PORT_NOT_IN_CLOSE_STATE    ((sl_status_t)0x1BB35) ///< Port not in closed state.
#define SL_STATUS_SI91X_SOCKET_CLOSED              ((sl_status_t)0x1BB36) ///< Socket is closed or in process of closing.
#define SL_STATUS_SI91X_PROCESS_IN_PROGRESS        ((sl_status_t)0x1BB37) ///< Process in progress.
#define SL_STATUS_SI91X_CONNECT_TO_NON_EXISTING_TCP_SERVER_SOCKET \
  ((sl_status_t)0x1BB38) ///< Trying to connect to a non-existing TCP server socket.
#define SL_STATUS_SI91X_ERROR_IN_LEN_OF_THE_COMMAND \
  ((sl_status_t)0x1BB3E) ///< Error in the length of the command (exceeds the number of characters mentioned in the PRM).
#define SL_STATUS_SI91X_WRONG_PACKET_INFO  ((sl_status_t)0x1BB40) ///< Wrong packet information.
#define SL_STATUS_SI91X_CORRUPTED_RPS      ((sl_status_t)0x1BB41) ///< Corrupted RPS encountered during firmware update.
#define SL_STATUS_SI91X_SOCKET_STILL_BOUND ((sl_status_t)0x1BB42) ///< Socket is still bound.
#define SL_STATUS_SI91X_NO_FREE_PORT       ((sl_status_t)0x1BB45) ///< No free port available.
#define SL_STATUS_SI91X_INVALID_PORT       ((sl_status_t)0x1BB46) ///< Invalid port specified.
#define SL_STATUS_SI91X_CORRUPTED_RPS_HEADER \
  ((sl_status_t)0x1BB49) ///< Corrupted RPS header encountered or Received empty RPS file(no data) during firmware update.
#define SL_STATUS_SI91X_FEATURE_UNSUPPORTED ((sl_status_t)0x1BB4B) ///< Feature not supported.
#define SL_STATUS_SI91X_SOCKET_IN_UNCONNECTED_STATE \
  ((sl_status_t)0x1BB50) ///< Socket is not in a connected state. Disconnected from the server. In the case of FTP, the user needs to issue a destroy command after receiving this error.
#define SL_STATUS_SI91X_POP3_SESSION_CREATION_FAILED \
  ((sl_status_t)0x1BB87) ///< POP3 session creation failed or POP3 session got terminated.
#define SL_STATUS_SI91X_DHCPV6_HANDSHAKE_FAIL     ((sl_status_t)0x1BB9C) ///< DHCPv6 handshake failure.
#define SL_STATUS_SI91X_DHCP_INVALID_IP_RESPONSE  ((sl_status_t)0x1BB9D) ///< DHCP invalid IP response.
#define SL_STATUS_SI91X_SMTP_AUTHENTICATION_ERROR ((sl_status_t)0x1BBA0) ///< SMTP authentication error.
#define SL_STATUS_SI91X_SMTP_OVER_SIZE_MAIL_DATA \
  ((sl_status_t)0x1BBA1) ///< No DNS server was specified, SMTP over size mail data.
#define SL_STATUS_SI91X_SMTP_INVALID_SERVER_REPLY ((sl_status_t)0x1BBA2) ///< SMTP invalid server reply.
#define SL_STATUS_SI91X_SMTP_DNS_QUERY_FAILED     ((sl_status_t)0x1BBA3) ///< DNS query failed, SMTP internal error.
#define SL_STATUS_SI91X_SMTP_BAD_DNS_ADDRESS \
  ((sl_status_t)0x1BBA4) ///< Bad DNS address, SMTP server error code received.
#define SL_STATUS_SI91X_SMTP_INVALID_PARAMETERS       ((sl_status_t)0x1BBA5) ///< SMTP invalid parameters.
#define SL_STATUS_SI91X_SMTP_PACKET_ALLOCATION_FAILED ((sl_status_t)0x1BBA6) ///< SMTP packet allocation failed.
#define SL_STATUS_SI91X_SMTP_GREET_REPLY_FAILED       ((sl_status_t)0x1BBA7) ///< SMTP Greet reply failed.
#define SL_STATUS_SI91X_SMTP_PARAMETER_ERROR          ((sl_status_t)0x1BBA8) ///< Parameter error, SMTP hello reply error.
#define SL_STATUS_SI91X_SMTP_MAIL_REPLY_ERROR         ((sl_status_t)0x1BBA9) ///< SMTP mail reply error.
#define SL_STATUS_SI91X_SMTP_RCPT_REPLY_ERROR         ((sl_status_t)0x1BBAA) ///< SMTP RCPT reply error.
#define SL_STATUS_SI91X_SMTP_MESSAGE_REPLY_ERROR      ((sl_status_t)0x1BBAB) ///< SMTP message reply error.
#define SL_STATUS_SI91X_SMTP_DATA_REPLY_ERROR         ((sl_status_t)0x1BBAC) ///< SMTP data reply error.
#define SL_STATUS_SI91X_SMTP_AUTH_REPLY_ERROR         ((sl_status_t)0x1BBAD) ///< SMTP authentication reply error.
#define SL_STATUS_SI91X_SMTP_SERVER_REPLY_ERROR       ((sl_status_t)0x1BBAE) ///< SMTP server error reply.
#define SL_STATUS_SI91X_DNS_DUPLICATE_ENTRY           ((sl_status_t)0x1BBAF) ///< DNS duplicate entry.
#define SL_STATUS_SI91X_SMTP_OVERSIZE_SERVER_REPLY    ((sl_status_t)0x1BBB1) ///< SMTP oversize server reply.
#define SL_STATUS_SI91X_SMTP_CLIENT_NOT_INITIALIZED   ((sl_status_t)0x1BBB2) ///< SMTP client not initialized.
#define SL_STATUS_SI91X_DNS_IPV6_NOT_SUPPORTED        ((sl_status_t)0x1BBB3) ///< DNS IPv6 not supported.
#define SL_STATUS_SI91X_INVALID_MAIL_INDEX_FOR_POP3_MAIL_RETRIEVE_COMMAND \
  ((sl_status_t)0x1BBC5) ///< Invalid mail index specified for the POP3 mail retrieve command.
#define SL_STATUS_SI91X_SSL_TLS_HANDSHAKE_FAILED ((sl_status_t)0x1BBD2) ///< SSL/TLS handshake process failed.
#define SL_STATUS_SI91X_FTP_CLIENT_DISCONNECTED \
  ((sl_status_t)0x1BBD3) ///< FTP client is either not connected or has been disconnected from the FTP server.
#define SL_STATUS_SI91X_FTP_CLIENT_NOT_DISCONNECTED \
  ((sl_status_t)0x1BBD4) ///< FTP client is still connected and has not been disconnected.
#define SL_STATUS_SI91X_TLS_RX_LEN_ERR_OR_FTP_FILE_NOT_OPEN \
  ((sl_status_t)0x1BBD5) ///< TLS received data length exceeded or FTP file could not be opened.
#define SL_STATUS_SI91X_SSL_TLS_HANDSHAKE_TIMEOUT_OR_FTP_FILE_NOT_CLOSED \
  ((sl_status_t)0x1BBD6) ///< SSL/TLS handshake timed out or the FTP file was not closed properly.
#define SL_STATUS_SI91X_FTP_EXPECTED_1XX_RESPONSE_NOT_RECEIVED \
  ((sl_status_t)0x1BBD9) ///< Expected 1XX response from the FTP server was not received.
#define SL_STATUS_SI91X_FTP_EXPECTED_2XX_RESPONSE_NOT_RECEIVED \
  ((sl_status_t)0x1BBDA) ///< Expected 2XX response from the FTP server was not received.
#define SL_STATUS_SI91X_FTP_EXPECTED_22X_RESPONSE_NOT_RECEIVED \
  ((sl_status_t)0x1BBDB) ///< Expected 22X response from the FTP server was not received.
#define SL_STATUS_SI91X_FTP_EXPECTED_23X_RESPONSE_NOT_RECEIVED \
  ((sl_status_t)0x1BBDC) ///< Expected 23X response from the FTP server was not received.
#define SL_STATUS_SI91X_FTP_EXPECTED_3XX_RESPONSE_NOT_RECEIVED \
  ((sl_status_t)0x1BBDD) ///< Expected 3XX response from the FTP server was not received.
#define SL_STATUS_SI91X_FTP_EXPECTED_33X_RESPONSE_NOT_RECEIVED \
  ((sl_status_t)0x1BBDE) ///< Expected 33X response from the FTP server was not received.
#define SL_STATUS_SI91X_HTTP_TIMEOUT            ((sl_status_t)0x1BBE1) ///< HTTP request timed out.
#define SL_STATUS_SI91X_HTTP_FAILED             ((sl_status_t)0x1BBE2) ///< HTTP request failed.
#define SL_STATUS_SI91X_HTTP_PUT_CLIENT_TIMEOUT ((sl_status_t)0x1BBE7) ///< HTTP PUT client request timed out.
#define SL_STATUS_SI91X_AUTHENTICATION_ERROR    ((sl_status_t)0x1BBEB) ///< Authentication process failed.
#define SL_STATUS_SI91X_INVALID_PACKET_LENGTH \
  ((sl_status_t)0x1BBED) ///< Invalid packet length. Content length and received data length mismatch.
#define SL_STATUS_SI91X_SERVER_RESPONDS_BEFORE_REQUEST_COMPLETE \
  ((sl_status_t)0x1BBEF) ///< Server responds before HTTP client request is complete.
#define SL_STATUS_SI91X_HTTP_PASSWORD_TOO_LONG ((sl_status_t)0x1BBF0) ///< HTTP/HTTPS password is too long.
#define SL_STATUS_SI91X_MQTT_PING_TIMEOUT      ((sl_status_t)0x1BBF1) ///< MQTT ping timeout error.
#define SL_STATUS_SI91X_MQTT_COMMAND_SENT_IN_INCORRECT_STATE \
  ((sl_status_t)0x1BBF2)                                               ///< MQTT command sent in incorrect state.
#define SL_STATUS_SI91X_MQTT_ACK_TIMEOUT        ((sl_status_t)0x1BBF3) ///< MQTT ACK timeout error.
#define SL_STATUS_SI91X_POP3_INVALID_MAIL_INDEX ((sl_status_t)0x1BBFF) ///< POP3 error for invalid mail index.
#define SL_STATUS_SI91X_FW_UPDATE_DONE          ((sl_status_t)0x1DD03) ///< Firmware update successful.
#define SL_STATUS_SI91X_FW_UPDATE_FAILED        ((sl_status_t)0x1DD04) ///< Firmware update failed.
#define SL_STATUS_SI91X_ALLOCATION_FAILED \
  ((sl_status_t)0x1DD3D) ///< Memory allocation failed in NWP during firmware upgradation.
#define SL_STATUS_SI91X_INSUFFICIENT_FLASH_MEMORY \
  ((sl_status_t)0x1DD3E) ///< Insufficient space in NWP flash memory during firmware upgradation.
#define SL_STATUS_SI91X_FW_UP_WRONG_PACKET_INFO ((sl_status_t)0x1DD40) ///< Wrong packet info.
#define SL_STATUS_SI91X_INVALID_LENGTH \
  ((sl_status_t)0x1DD41) ///< All payload chunks length together can't be greater than total image size in header, during firmware upgradation.

//FW fallback error codes
#define SL_STATUS_SI91X_AB_FALLBACK_NOT_ENABLED ((sl_status_t)0x1DD0A) ///< AB fall-back feature is not enabled
#define SL_STATUS_SI91X_LEGACY_OTA_FEATURE_NOT_SUPPORTED \
  ((sl_status_t)0x1DD0B) ///< Legacy OTA feature is not supported for fw fallback
#define SL_STATUS_SI91X_INVALID_ERASE_LENGTH_OR_ADDRESS \
  ((sl_status_t)0x1DD0C) ///< Invalid erase length or address specified for the operation for fw fallback
#define SL_STATUS_SI91X_INVALID_READ_OR_WRITE_REQUEST \
  ((sl_status_t)0x1DD0D) ///< The read or write request is invalid or not supported for fw fallback
#define SL_STATUS_SI91X_INVALID_FLASH_RANGE \
  ((sl_status_t)0x1DD0E) ///< The specified flash range is invalid or out of bounds for fw fallback
#define SL_STATUS_SI91X_FLASH_ADDRESS_OUT_OF_BOUNDARY \
  ((sl_status_t)0x1DD0F) ///< The flash address is out of the allowable boundary for fw fallback
#define SL_STATUS_SI91X_INVALID_OPERATION_ON_NWP_ACTIVE_IMAGE \
  ((sl_status_t)0x1DD10) ///< Operation attempted on an active NWP image which is not allowed for fw fallback
#define SL_STATUS_SI91X_MAX_INPUT_LENGTH_EXCEED_1K_SIZE \
  ((sl_status_t)0x1DD11) ///< The input length exceeds the maximum allowed size of 1KB for fw fallback
#define SL_STATUS_SI91X_INVALID_FW_ROLLBACK_SUBCOMMAND \
  ((sl_status_t)0x1DD12) ///< The sub-command for firmware fallback is invalid
#define SL_STATUS_SI91X_INVALID_CONFIG_FOR_FW_ROLLBACK \
  ((sl_status_t)0x1DD13) ///< The configuration for firmware fallback is invalid, applicable for 8MB SoC common-flash
#define SL_STATUS_SI91X_FW_DOWNGRADE_NOT_ALLOWED \
  ((sl_status_t)0x1DD14) ///< The NWP firmware downgrade is not allowed for fw fallback
#define SL_STATUS_SI91X_ANTIROLLBACK_IS_NOT_ENABLED \
  ((sl_status_t)0x1DD15) ///< Anti-rollback feature is not enabled for firmware fallback
#define SL_STATUS_SI91X_INVALID_REQUEST_TO_LOAD_QSPI_KEYS \
  ((sl_status_t)0x1DD16) ///< Invalid request to load QSPI keys for fw fallback
#define SL_STATUS_SI91X_INVALID_OPERATION_ON_M4_UPDATER_IMAGE \
  ((sl_status_t)0x1DD17) ///< Invalid operation on M4 updater image for fw fallback
#define SL_SI91X_DEVICE_AND_OTA_IMAGE_SECURITY_MISMATCH \
  ((sl_status_t)0x1DD18) ///< Device security and OTA image security mismatch for fw fallback
//Fw fallback error codes end

#define SL_STATUS_SI91X_FW_UP_CORRUPTED_RPS_HEADER \
  ((sl_status_t)0x1DD49) ///< Corrupted RPS header encountered or Received empty RPS file(no data) during firmware update.
#define SL_STATUS_SI91X_SOCKET_NOT_CONNECTED \
  ((sl_status_t)0x1FFFF) ///< Listening TCP socket in device is not connected to the remote peer, or the LTCP socket is not yet opened in the device.
#define SL_STATUS_SI91X_SOCKET_LIMIT_EXCEEDED \
  ((sl_status_t)0x1FFFE) ///< Socket limit exceeded. More than 10 sockets opened.
#define SL_STATUS_SI91X_HTTP_OTAF_INVALID_PACKET ((sl_status_t)0x1FFFD) ///< HTTP OTAF invalid packet.
#define SL_STATUS_SI91X_TCP_IP_INIT_FAILED       ((sl_status_t)0x1FFFC) ///< TCP/IP initialization failed.
#define SL_STATUS_SI91X_CONCURRENT_IP_CREATION_ERROR \
  ((sl_status_t)0x1FFFB) ///< Cannot create IP in same interface in concurrent mode.
#define SL_STATUS_SI91X_HTTP_OTAF_INCOMPLETE_PACKET ((sl_status_t)0x1FFF4) ///< HTTP OTAF incomplete packet.
#define SL_STATUS_SI91X_INVALID_STORE_CONFIGURATION_PROFILE \
  ((sl_status_t)0x1FFF5) ///< Invalid or mismatched store configuration profile type.
#define SL_STATUS_SI91X_MQTT_REMOTE_TERMINATE_ERROR ((sl_status_t)0x1FFF6) ///< MQTT remote terminate error.
#define SL_STATUS_SI91X_MQTT_KEEP_ALIVE_TERMINATE_ERROR \
  ((sl_status_t)0x1BBF1) ///< MQTT remote terminate error due to keep-alive response timeout.
#define SL_STATUS_SI91X_BYTE_STUFFING_ERROR_IN_AT_MODE ((sl_status_t)0x1FFF7) ///< Byte stuffing error in AT mode.
#define SL_STATUS_SI91X_INVALID_COMMAND_OR_OPERATION \
  ((sl_status_t)0x1FFF8) ///< Invalid command (e.g. parameters insufficient or invalid in the command). Invalid operation (for example, power save command with the same mode given twice, accessing wrong socket, creating more than allowed sockets ).
#define SL_STATUS_SI91X_HTTP_OTAF_NO_PACKET      ((sl_status_t)0x1FFF9) ///< HTTP OTAF no packet received.
#define SL_STATUS_SI91X_TCP_SOCKET_NOT_CONNECTED ((sl_status_t)0x1FFFA) ///< TCP socket is not connected.
#define SL_STATUS_SI91X_MAX_STATION_COUNT_EXCEEDED \
  ((sl_status_t)0x1FFC5) ///< Station count exceeded the maximum supported stations.
#define SL_STATUS_SI91X_UNABLE_TO_SEND_TCP_DATA ((sl_status_t)0x1FFC4) ///< Unable to send TCP data.
#define SL_STATUS_SI91X_SOCKET_BUFFER_TOO_SMALL ((sl_status_t)0x1FFBC) ///< Socket buffer too small.
#define SL_STATUS_SI91X_INVALID_CONTENT_IN_DNS_RESPONSE \
  ((sl_status_t)0x1FFBB) ///< Invalid content in the DNS response to the DNS resolution query.
#define SL_STATUS_SI91X_DNS_CLASS_ERROR_IN_DNS_RESPONSE \
  ((sl_status_t)0x1FFBA) ///< DNS class error in response to the DNS resolution query.
#define SL_STATUS_SI91X_DNS_COUNT_ERROR_IN_DNS_RESPONSE \
  ((sl_status_t)0x1FFB8) ///< DNS count error in response to the DNS resolution query.
#define SL_STATUS_SI91X_DNS_RETURN_CODE_ERROR_IN_DNS_RESPONSE \
  ((sl_status_t)0x1FFB7) ///< DNS return code error in the response to the DNS resolution query.
#define SL_STATUS_SI91X_DNS_OPCODE_ERROR_IN_DNS_RESPONSE \
  ((sl_status_t)0x1FFB6) ///< DNS Opcode error in the response to the DNS resolution query.
#define SL_STATUS_SI91X_DNS_ID_MISMATCH \
  ((sl_status_t)0x1FFB5) ///< DNS id mismatch between the DNS resolution request and response.
#define SL_STATUS_SI91X_INVALID_INPUT_IN_DNS_QUERY \
  ((sl_status_t)0x1FFAB) ///< Invalid input provided in the DNS resolution query.
#define SL_STATUS_SI91X_DNS_RESPONSE_TIMEOUT        ((sl_status_t)0x1FF42) ///< DNS response was timed out.
#define SL_STATUS_SI91X_ARP_REQUEST_FAILURE         ((sl_status_t)0x1FFA1) ///< ARP request failure.
#define SL_STATUS_SI91X_UNABLE_TO_UPDATE_TCP_WINDOW ((sl_status_t)0x1FF91) ///< Failed to update TCP window.
#define SL_STATUS_SI91X_DHCP_LEASE_EXPIRED          ((sl_status_t)0x1FF9D) ///< DHCP lease time expired.
#define SL_STATUS_SI91X_DHCP_HANDSHAKE_FAILURE      ((sl_status_t)0x1FF9C) ///< DHCP handshake failure.
#define SL_STATUS_SI91X_WEBSOCKET_CREATION_FAILED   ((sl_status_t)0x1FF88) ///< WebSocket creation failed.
#define SL_STATUS_SI91X_TRYING_TO_CONNECT_NON_EXISTENT_TCP_SERVER_SOCKET \
  ((sl_status_t)0x1FF87) ///< Attempted to connect to a non-existent TCP server socket on the remote side.
#define SL_STATUS_SI91X_TRYING_TO_CLOSE_NON_EXISTENT_SOCKET \
  ((sl_status_t)0x1FF86) ///< Attempted to close a non-existent or invalid socket descriptor.
#define SL_STATUS_SI91X_INVALID_SOCKET_PARAMETERS \
  ((sl_status_t)0x1FF85)                                             ///< The provided socket parameters are invalid.
#define SL_STATUS_SI91X_FEATURE_NOT_AVAILABLE ((sl_status_t)0x1FF82) ///< The requested feature is not supported.
#define SL_STATUS_SI91X_SOCKET_ALREADY_OPEN   ((sl_status_t)0x1FF81) ///< The socket is already open.
#define SL_STATUS_SI91X_MAX_SOCKETS_EXCEEDED \
  ((sl_status_t)0x1FF80) ///< Attempted to open more than the maximum allowed number of sockets.
#define SL_STATUS_SI91X_DATA_LENGTH_EXCEEDS_MSS \
  ((sl_status_t)0x1FF7E) ///< Data length exceeds the Maximum Segment Size (MSS).
#define SL_STATUS_SI91X_IP_CONFLICT_ERROR \
  ((sl_status_t)0x1FF75) ///< Device unable to configure IP address due to an IP conflict.
#define SL_STATUS_SI91X_FEATURE_NOT_ENABLED ((sl_status_t)0x1FF74) ///< The requested feature is not enabled.
#define SL_STATUS_SI91X_DHCP_SERVER_NOT_SET ((sl_status_t)0x1FF73) ///< DHCP server is not configured in AP mode.
#define SL_STATUS_SI91X_AP_SET_REGION_PARAM_ERROR \
  ((sl_status_t)0x1FF71) ///< Invalid parameters provided for the AP set region command.
#define SL_STATUS_SI91X_SSL_TLS_NOT_SUPPORTED  ((sl_status_t)0x1FF70) ///< SSL/TLS is not supported.
#define SL_STATUS_SI91X_JSON_NOT_SUPPORTED     ((sl_status_t)0x1FF6F) ///< JSON format is not supported.
#define SL_STATUS_SI91X_INVALID_OPERATING_MODE ((sl_status_t)0x1FF6E) ///< The specified operating mode is invalid.
#define SL_STATUS_SI91X_INVALID_SOCKET_CONFIG_PARAMS \
  ((sl_status_t)0x1FF6D)                                                  ///< Invalid socket configuration parameters.
#define SL_STATUS_SI91X_WEBSOCKET_CREATION_TIMEOUT ((sl_status_t)0x1FF6C) ///< Web socket creation timeout.
#define SL_STATUS_SI91X_PARAM_MAX_VALUE_EXCEEDED \
  ((sl_status_t)0x1FF6B) ///< Parameter exceeds the maximum allowed value.
#define SL_STATUS_SI91X_SOCKET_READ_TIMEOUT         ((sl_status_t)0x1FF6A) ///< The socket read operation timed out.
#define SL_STATUS_SI91X_INVALID_COMMAND_SEQUENCE    ((sl_status_t)0x1FF69) ///< The command sequence is invalid.
#define SL_STATUS_SI91X_DNS_RESPONSE_TIMEOUT_ERROR  ((sl_status_t)0x1FF42) ///< The DNS response timed out.
#define SL_STATUS_SI91X_HTTP_SOCKET_CREATION_FAILED ((sl_status_t)0x1FF41) ///< Failed to create an HTTP socket.
#define SL_STATUS_SI91X_HTTP_GET_CMD_IN_PROGRESS \
  ((sl_status_t)0x10005) ///< An HTTP GET command is currently in progress.
#define SL_STATUS_SI91X_TCP_CLOSE_BEFORE_RESPONSE_ERROR \
  ((sl_status_t)0x1FF40) ///< TCP socket close command issued before receiving the response of the previous close command.
#define SL_STATUS_SI91X_WAIT_ON_HOST_FEATURE_NOT_ENABLED \
  ((sl_status_t)0x1FF36) ///< 'Wait On Host' feature is not enabled.
#define SL_STATUS_SI91X_STORE_CONFIG_CHECKSUM_INVALID \
  ((sl_status_t)0x1FF35) ///< Store configuration checksum validation failed.
#define SL_STATUS_SI91X_TCP_KEEP_ALIVE_TIMEOUT     ((sl_status_t)0x1FF33) ///< TCP keep-alive timed out.
#define SL_STATUS_SI91X_TCP_ACK_FAILED_FOR_SYN_ACK ((sl_status_t)0x1FF2D) ///< TCP ACK failed for TCP SYN-ACK.
#define SL_STATUS_SI91X_MEMORY_LIMIT_EXCEEDED \
  ((sl_status_t)0x1FF2C) ///< Memory limit exceeded in the given operating mode.
#define SL_STATUS_SI91X_MEMORY_LIMIT_EXCEEDED_DURING_AUTO_JOIN \
  ((sl_status_t)0x1FF2A) ///< Memory limit exceeded in the operating mode during auto join/create.
#define SL_STATUS_SI91X_MDNS_COMMAND_NOT_SUPPORTED ((sl_status_t)0x1FF2B) ///< The MDNS command type is invalid.
#define SL_STATUS_SI91X_CRYPTO_INVALID_SECURITY_MODE \
  ((sl_status_t)0x1CC43) ///< Invalid security mode in SI91X crypto operations.
#define SL_STATUS_SI91X_CRYPTO_ERR_DATA_ALIGNMENT_16BYTE \
  ((sl_status_t)0x1CC92) ///< Input data is not 16-byte aligned (required for AES crypto operation).
#define SL_STATUS_SI91X_CRYPTO_CMAC_DECRYPTION_NOT_SUPPORTED \
  ((sl_status_t)0x1CC93) ///< CMAC decryption is not supported.
#define SL_STATUS_SI91X_CRYPTO_INVALID_KEY_LENGTH \
  ((sl_status_t)0x1CC94) ///< Invalid key length provided to a crypto operation.
#define SL_STATUS_SI91X_CRYPTO_INVALID_LENGTH \
  ((sl_status_t)0x1CC95) ///< Invalid input parameter length (e.g., for CCM operation).
#define SL_STATUS_SI91X_CRYPTO_SHA_MODE_NOT_SUPPORTED      ((sl_status_t)0x1CC96) ///< Unsupported SHA mode.
#define SL_STATUS_SI91X_CRYPTO_INVALID_ECDSA_CURVE_ID      ((sl_status_t)0x1CC97) ///< Invalid ECDSA curve identifier.
#define SL_STATUS_SI91X_CRYPTO_INVALID_ECDH_TYPE           ((sl_status_t)0x1CC98) ///< Invalid ECDH type.
#define SL_STATUS_SI91X_CRYPTO_INVALID_ECDSA_SUB_ALGORITHM ((sl_status_t)0x1CC99) ///< Invalid ECDSA sub-algorithm.
#define SL_STATUS_SI91X_CRYPTO_OPERATION_FAILED            ((sl_status_t)0x1CC9B) ///< Crypto operation failure.
#define SL_STATUS_SI91X_CRYPTO_INVALID_SUB_ALGORITHM       ((sl_status_t)0x1CC9C) ///< Invalid sub-algorithm selection.
#define SL_STATUS_SI91X_CRYPTO_INVALID_ALGORITHM           ((sl_status_t)0x1CC9D) ///< Invalid algorithm selection.
#define SL_STATUS_SI91X_CRYPTO_INVALID_ECDH_MODE           ((sl_status_t)0x1CC9E) ///< Invalid ECDH mode.
#define SL_STATUS_SI91X_CRYPTO_INPUT_MSG_LENGTH_EXCEEDED \
  ((sl_status_t)0x1CC9F) ///< Input message length exceeding the allowed maximum.
#define SL_STATUS_SI91X_CRYPTO_DEVICE_SECURITY_IS_DISABLED ((sl_status_t)0x1CCA1) ///< Device security is disabled.
#define SL_STATUS_SI91X_CRYPTO_INVALID_SIGNATURE \
  ((sl_status_t)0x1CCC6) ///< Invalid signature or tag mismatch (e.g., authentication failure).
#define SL_STATUS_SI91X_AUTO_JOIN_IN_PROGRESS \
  ((sl_status_t)0x15A5A) ///< Auto join or user store configuration is in progress.
#define SL_STATUS_SI91X_RSNIE_FROM_AP_INVALID \
  ((sl_status_t)0x1FFE1) ///< Received an invalid RSNIE from the AP to the station.
#define SL_STATUS_SI91X_SNTP_MAX_ATTEMPTS_REACHED \
  ((sl_status_t)0x1FF5F) ///< Maximum number of invalid SNTP attempts reached.
#define SL_STATUS_SI91X_FREQUENCY_OFFSET_ZER0 ((sl_status_t)0x100FC) ///< The frequency offset sent is zero.
#define SL_STATUS_SI91X_FREQUENCY_OFFSET_OUT_OF_LIMITS \
  ((sl_status_t)0x100FB) ///< Frequency offset specified goes beyond upper or lower limits and indicates that frequency offset cannot be changed further.

//Bluetooth Generic Error Codes
#define SL_STATUS_SI91X_UNKNOWN_HCI_COMMAND           (0x4E01) ///< Unknown HCI command.
#define SL_STATUS_SI91X_UNKNOWN_CONNECTION_IDENTIFIER (0x4E02) ///< Unknown Connection Identifier.
#define SL_STATUS_SI91X_HARDWARE_FAILURE              (0x4E03) ///< Hardware failure.
#define SL_STATUS_SI91X_PAGE_TIMEOUT                  (0x4E04) ///< Page timeout.
#define SL_STATUS_SI91X_AUTHENTICATION_FAILURE        (0x4E05) ///< Authentication failure.
#define SL_STATUS_SI91X_PIN_MISSING                   (0x4E06) ///< Pin missing.
#define SL_STATUS_SI91X_MEMORY_CAPACITY_EXCEED        (0x4E07) ///< Memory capacity exceeded.
#define SL_STATUS_SI91X_CONNECTION_TIMEOUT            (0x4E08) ///< Connection timeout.
#define SL_STATUS_SI91X_CONNECTION_LIMIT_EXCEED       (0x4E09) ///< Connection limit exceeded.
#define SL_STATUS_SI91X_SCO_LIMIT_EXCEED              (0x4E0A) ///< SCO limit exceeded.
#define SL_STATUS_SI91X_ACL_CONNECTION_ALREADY_EXIST  (0x4E0B) ///< ACL Connection already exists.
#define SL_STATUS_SI91X_COMMAND_DISALLOWED            (0x4E0C) ///< Command disallowed.
#define SL_STATUS_SI91X_CONNECTION_REJECTED_LIMITED_RESOURCES \
  (0x4E0D) ///< Connection rejected due to limited resources .
#define SL_STATUS_SI91X_CONNECTION_REJECTED_SECURITY_REASONS (0x4E0E) ///< Connection rejected due to security reasons.
#define SL_STATUS_SI91X_CONNECTION_REJECTED_FOR_BD_ADDR      (0x4E0F) ///< Connection rejected for BD address.
#define SL_STATUS_SI91X_CONNECTION_ACCEPT_TIMEOUT            (0x4E10) ///< Connection accept timeout.
#define SL_STATUS_SI91X_UNSUPPORTED_FEATURE_OR_PARAMETER     (0x4E11) ///< Unsupported feature or parameter.
#define SL_STATUS_SI91X_INVALID_HCI_COMMAND_PARAMETER        (0x4E12) ///< Invalid HCI command parameter.
#define SL_STATUS_SI91X_REMOTE_USER_TERMINATED_CONNECTION    (0x4E13) ///< Remote user terminated connection.
#define SL_STATUS_SI91X_REMOTE_DEVICE_TERMINATED_CONNECTION_LOW_RESOURCES \
  (0x4E14) ///< Remote device terminated connection due to low resources .
#define SL_STATUS_SI91X_REMOTE_DEVICE_TERMINATED_CONNECTION_POWER_OFF \
  (0x4E15) ///< Remote device terminated connection due to power off .
#define SL_STATUS_SI91X_LOCAL_DEVICE_TERMINATED_CONNECTION           (0x4E16) ///< Local device terminated connection.
#define SL_STATUS_SI91X_REPEATED_ATTEMPTS                            (0x4E17) ///< Repeated attempts.
#define SL_STATUS_SI91X_PAIRING_NOT_ALLOWED                          (0x4E18) ///< Pairing not allowed.
#define SL_STATUS_SI91X_UNKNOWN_LMP_PDU                              (0x4E19) ///< Unknown LMP PDU.
#define SL_STATUS_SI91X_UNSUPPORTED_REMOTE_FEATURE                   (0x4E1A) ///< Unsupported remote feature.
#define SL_STATUS_SI91X_SCO_OFFSET_REJECTED                          (0x4E1B) ///< SCO offset rejected.
#define SL_STATUS_SI91X_SCO_INTERVAL_REJECTED                        (0x4E1C) ///< SCO interval rejected.
#define SL_STATUS_SI91X_SCO_AIR_MODE_REJECTED                        (0x4E1D) ///< SCO Air mode rejected.
#define SL_STATUS_SI91X_INVALID_LMP_PARAMETERS                       (0x4E1E) ///< Invalid LMP parameters.
#define SL_STATUS_SI91X_UNSPECIFIED                                  (0x4E1F) ///< Unspecified.
#define SL_STATUS_SI91X_UNSUPPORTED_LMP_PARAMETER                    (0x4E20) ///< Unsupported LMP Parameter.
#define SL_STATUS_SI91X_ROLE_CHANGE_NOT_ALLOWED                      (0x4E21) ///< Role change not allowed.
#define SL_STATUS_SI91X_LMP_RESPONSE_TIMEOUT                         (0x4E22) ///< LMP response timeout.
#define SL_STATUS_SI91X_LMP_TRANSACTION_COLLISION                    (0x4E23) ///< LMP transaction collision.
#define SL_STATUS_SI91X_LMP_PDU_NOT_ALLOWED                          (0x4E24) ///< LMP PDU not allowed.
#define SL_STATUS_SI91X_ENCRYPTION_MODE_NOT_ACCEPTABLE               (0x4E25) ///< Encryption mode not acceptable.
#define SL_STATUS_SI91X_LINK_KEY_CANNOT_CHANGE                       (0x4E26) ///< Link key cannot change.
#define SL_STATUS_SI91X_REQUESTED_QOS_NOT_SUPPORTED                  (0x4E27) ///< Requested QOS not supported.
#define SL_STATUS_SI91X_INSTANT_PASSED                               (0x4E28) ///< Instant passed.
#define SL_STATUS_SI91X_PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED          (0x4E29) ///< Pairing with unit key not supported.
#define SL_STATUS_SI91X_DIFFERENT_TRANSACTION_COLLISION              (0x4E2A) ///< Different transaction collision.
#define SL_STATUS_SI91X_RESERVED_1                                   (0x4E2B) ///< Reserved 1.
#define SL_STATUS_SI91X_QOS_PARAMETER_NOT_ACCEPTABLE                 (0x4E2C) ///< QOS parameter not acceptable.
#define SL_STATUS_SI91X_QOS_REJECTED                                 (0x4E2D) ///< QOS rejected.
#define SL_STATUS_SI91X_CHANNEL_CLASSIFICATION_NOT_SUPPORTED         (0x4E2E) ///< Channel classification not supported.
#define SL_STATUS_SI91X_INSUFFICIENT_SECURITY                        (0x4E2F) ///< Insufficient security.
#define SL_STATUS_SI91X_PARAMETER_OUT_OF_MANDATORY_RANGE             (0x4E30) ///< Parameter out of mandatory range.
#define SL_STATUS_SI91X_RESERVED_2                                   (0x4E31) ///< Reserved 2.
#define SL_STATUS_SI91X_ROLE_SWITCH_PENDING                          (0x4E32) ///< Role switch pending.
#define SL_STATUS_SI91X_RESERVED_3                                   (0x4E33) ///< Reserved 3.
#define SL_STATUS_SI91X_RESERVED_SLOT_VIOLATION                      (0x4E34) ///< Reserved slot violation.
#define SL_STATUS_SI91X_ROLE_SWITCH_FAILED                           (0x4E35) ///< Role switch failed.
#define SL_STATUS_SI91X_EXTENDED_INQUIRY_RESPONSE_TOO_LARGE          (0x4E36) ///< Extended Inquiry Response too large.
#define SL_STATUS_SI91X_EXTENDED_SECURE_SIMPLE_PAIRING_NOT_SUPPORTED (0x4E37) ///< Extended SSP not supported.
#define SL_STATUS_SI91X_HOST_BUSY_PAIRING                            (0X4E38) ///< Host busy pairing.
#define SL_STATUS_SI91X_PINCODE_REPLY_FOR_WRONG_BD_ADDRESS           (0x4E39) ///< Wrong BD Address.
#define SL_STATUS_SI91X_CONNECTION_FAILED_ESTABLISHED                (0x4E3E) ///< Connection Failed to be Establish.
#define SL_STATUS_SI91X_INVALID_GAIN_TABLE_PAYLOAD_LENGTH            (0x4F01) ///< Invalid Gain table payload length.
#define SL_STATUS_SI91X_INVALID_REGION                               (0x4F02) ///< Invalid Region.
#define SL_STATUS_SI91X_INVALID_GAIN_TABLE_OFFSET_REQUEST_TYPE       (0x4F03) ///< Invalid Gain Table offset request type.
#define SL_STATUS_SI91X_INVALID_NODE_ID                              (0x4F04) ///< Invalid Node ID.
#define SL_STATUS_SI91X_BT_INVALID_COMMAND                           (0x4FF8) ///< BT Invalid Command.
#define SL_STATUS_SI91X_INVALID_ARGS                                 (0x0101) ///< Invalid Args.
#define SL_STATUS_SI91X_UNKNOWN                                      (0x0102) ///< Unknown.
#define SL_STATUS_SI91X_FIRMWARE_TIMEOUT                             (0x0103) ///< Firmware Timeout.
#define SL_STATUS_SI91X_MEMORY_ALLOC_FAIL                            (0x0104) ///< Memory alloc fail.
#define SL_STATUS_SI91X_IO_FAIL                                      (0x0106) ///< I/O fail.
#define SL_STATUS_SI91X_UNSUPPORTED                                  (0x0108) ///< Unsupported.
#define SL_STATUS_SI91X_SHORT_BUF                                    (0x0109) ///< Short buffer.
#define SL_STATUS_SI91X_BUF_OVERFLOW                                 (0x010A) ///< Buf overflow .
#define SL_STATUS_SI91X_TOO_LARGE_BUF                                (0x010B) ///< Too large buffer.
#define SL_STATUS_SI91X_I_O_ABORT                                    (0x010C) ///< I/O abort.
#define SL_STATUS_SI91X_OPEN_FAIL                                    (0x010D) ///< File open fail.
#define SL_STATUS_SI91X_OS_TASK_INVALID_PRIORITY                     (0x1010) ///< OS task invalid priority.
#define SL_STATUS_SI91X_OS_TASK_PRIO_EXISTS                          (0x1011) ///< OS task prio exists.
#define SL_STATUS_SI91X_OS_TASK_NOT_STOPPED                          (0x1012) ///< OS task not stopped.
#define SL_STATUS_SI91X_OS_SEM_MAX_VALUE                             (0x1020) ///< OS sem max value.
#define SL_STATUS_SI91X_OS_SEM_NOT_AVAILABLE                         (0x1021) ///< OS sem not available.
#define SL_STATUS_SI91X_OS_SEM_RESET                                 (0x1022) ///< OS sem reset.
#define SL_STATUS_SI91X_OS_MUTEX_NOT_OWNER                           (0x1030) ///< OS mutex not owner.
#define SL_STATUS_SI91X_OS_MUTEX_NOT_LOCKED                          (0x1031) ///< OS mutex not locked.
#define SL_STATUS_SI91X_OS_MUTEX_LOCK_FAILED                         (0x1032) ///< OS mutex lock failed.
#define SL_STATUS_SI91X_OS_MUTEX_TRY_LOCK_FAILED                     (0x1033) ///< OS mutex try lock failed.
#define SL_STATUS_SI91X_OS_MSG_QUEUE_FULL                            (0x1040) ///< OS msg queue full.
#define SL_STATUS_SI91X_OS_MESSAGE_QUEUE_EMPTY                       (0x1041) ///< OS message queue empty.
#define SL_STATUS_SI91X_PIPE_EMPTY                                   (0x1050) ///< Pipe empty.
#define SL_STATUS_SI91X_PIPE_FULL                                    (0x1051) ///< Pipe full.
#define SL_STATUS_SI91X_INVALID_LEN                                  (0x1052) ///< Invalid len.
#define SL_STATUS_SI91X_PIPE_READ_IN_USE                             (0x1053) ///< Pipe read in use.
#define SL_STATUS_SI91X_PIPE_WRITE_IN_USE                            (0x1054) ///< Pipe write in use.
#define SL_STATUS_SI91X_OS_TIMER_EXPIRED                             (0x1060) ///< OS timer expired.
#define SL_STATUS_SI91X_OS_TIMER_STATE_RUNNING                       (0x1061) ///< OS timer state running.
#define SL_STATUS_SI91X_OS_CANNOT_WAIT                               (0x1070) ///< OS cannot wait.
#define SL_STATUS_SI91X_OS_MEM_POOL_EMPTY                            (0x1080) ///< OS mem pool empty.
#define SL_STATUS_SI91X_OS_MEM_POOL_SIZE_SHORT                       (0x1081) ///< OS mem pool size short.
#define SL_STATUS_SI91X_ERR_BT_INVALID_ARGS                          (0x4046) ///< Invalid Arguments.

//BLE Generic Error Codes
#define SL_STATUS_SI91X_DIRECTED_ADVERTISING_TIMEOUT (0x4E3C) ///<  Directed Advertising Timeout.
#define SL_STATUS_SI91X_CONNECTION_TERMINATED_DUE_TO_MIC_FAILURE \
  (0x4E3D)                                                 ///<  Connection terminated due to MIC failure.
#define SL_STATUS_SI91X_INVALID_HANDLE_RANGE      (0x4E60) ///<  Invalid Handle Range.
#define SL_STATUS_SI91X_MEMORY_IS_NOT_SUFFICIENT  (0x4E61) ///<  Memory is not sufficient.
#define SL_STATUS_SI91X_INVALID_PARAMETERS        (0x4E62) ///<  Invalid Parameters.
#define SL_STATUS_SI91X_BLE_BUFFER_COUNT_EXCEEDED (0x4E63) ///<  BLE Buffer Count Exceeded.
#define SL_STATUS_SI91X_BLE_BUFFER_ALREADY_IN_USE (0x4E64) ///<  BLE Buffer already in use.
#define SL_STATUS_SI91X_INVALID_ATTRIBUTE_LENGTH \
  (0x4E65) ///<  Invalid Attribute Length When Small Buffer Mode is Configured.
#define SL_STATUS_SI91X_INVALID_NAME_LENGTH (0x4E66) ///<  Invalid Name length when set to more than 16 bytes.

//BLE Mode Error Codes
#define SL_STATUS_SI91X_INVALID_HANDLE                       (0x4A01) ///< Invalid Handle.
#define SL_STATUS_SI91X_READ_NOT_PERMITTED                   (0x4A02) ///< Read not permitted.
#define SL_STATUS_SI91X_WRITE_NOT_PERMITTED                  (0x4A03) ///< Write not permitted.
#define SL_STATUS_SI91X_INVALID_PDU                          (0x4A04) ///< Invalid PDU.
#define SL_STATUS_SI91X_INSUFFICIENT_AUTHENTICATION          (0x4A05) ///< Insufficient authentication.
#define SL_STATUS_SI91X_REQUEST_NOT_SUPPORTED                (0x4A06) ///< Request not supported .
#define SL_STATUS_SI91X_INVALID_OFFSET                       (0x4A07) ///< Invalid offset.
#define SL_STATUS_SI91X_INSUFFICIENTA_UTHORIZATION           (0x4A08) ///< Insufficient authorization.
#define SL_STATUS_SI91X_PREPARE_QUEUE_FULL                   (0x4A09) ///< Prepare queue full.
#define SL_STATUS_SI91X_ATTRIBUTE_NOT_FOUND                  (0x4A0A) ///< Attribute not found.
#define SL_STATUS_SI91X_ATTRIBUTE_NOT_LONG                   (0x4A0B) ///< Attribute not Long.
#define SL_STATUS_SI91X_INSUFFICIENT_ENCRYPTION_KEY_SIZE     (0x4A0C) ///< Insufficient encryption key size.
#define SL_STATUS_SI91X_INVALID_ATTRIBUTE_VALUE_LENGTH       (0x4A0D) ///< Invalid attribute value length.
#define SL_STATUS_SI91X_UNLIKELY_ERROR                       (0x4A0E) ///< Unlikely error.
#define SL_STATUS_SI91X_INSUFFICIENT_ENCRYPTION              (0x4A0F) ///< Insufficient encryption.
#define SL_STATUS_SI91X_UNSUPPORTED_GROUP_TYPE               (0x4A10) ///< Unsupported group type.
#define SL_STATUS_SI91X_INSUFFICIENT_RESOURCES               (0x4A11) ///< Insufficient resources.
#define SL_STATUS_SI91X_GATT_TIMEOUT                         (0x4A80) ///< GATT Timeout.
#define SL_STATUS_SI91X_DATA_MAINTAINED_IN_HOST              (0x4AFF) ///< Data Maintained in Host.
#define SL_STATUS_SI91X_SMP_PASSKEY_ENTRY_FAILED             (0x4B01) ///< SMP Passkey entry failed.
#define SL_STATUS_SI91X_SMP_OOB_NOT_AVAILABLE                (0x4B02) ///< SMP OOB not available.
#define SL_STATUS_SI91X_SMP_AUTHENTICATION_REQUIREMENTS      (0x4B03) ///< SMP Authentication Requirements.
#define SL_STATUS_SI91X_SMP_CONFIRM_VALUE_FAILED             (0x4B04) ///< SMP confirm value failed.
#define SL_STATUS_SI91X_SMP_PAIRING_NOT_SUPPORTED            (0x4B05) ///< SMP Pairing not supported.
#define SL_STATUS_SI91X_SMP_ENCRYPTION_KEY_SIZE_INSUFFICIENT (0x4B06) ///< SMP Encryption key size insufficient.
#define SL_STATUS_SI91X_SMP_COMMAND_NOT_SUPPORTED            (0x4B07) ///< SMP command not supported.
#define SL_STATUS_SI91X_SMP_PAIRING_FAILED                   (0x4B08) ///< SMP pairing failed.
#define SL_STATUS_SI91X_SMP_REPEATED_ATTEMPTS                (0x4B09) ///< SMP repeated attempts.
#define SL_STATUS_SI91X_SMP_INVALID_PARAMETERS               (0x4B0A) ///< SMP Invalid parameters.
#define SL_STATUS_SI91X_SMP_DH_KEY_CHECK_FAILED              (0x4B0B) ///< SMP DH Key check failed.
#define SL_STATUS_SI91X_SMP_NUMERIC_COMPARISON_FAILED        (0x4B0C) ///< SMP Numeric Comparison Failed.
#define SL_STATUS_SI91X_SMP_INVALID_KEYS_GENERATED           (0x4BFD) ///< SMP Invalid Keys Generated.
#define SL_STATUS_SI91X_SMP_IDENTICAL_PUBLIC_KEY             (0x4BFE) ///< SMP Identical Public Key.
#define SL_STATUS_SI91X_PSM_CONN_FAILED                      (0x4C02) ///< PSM Conn Failed.
#define SL_STATUS_SI91X_BLE_REMOTE_DEVICE_FOUND              (0x4D00) ///< BLE Remote device found.
#define SL_STATUS_SI91X_BLE_REMOTE_DEVICE_NOT_FOUND          (0x4D01) ///< BLE Remote device not found.
#define SL_STATUS_SI91X_BLE_REMOTE_DEVICE_STRUCTURE_FULL     (0x4D02) ///< BLE Remote device structure full.
#define SL_STATUS_SI91X_UNABLE_TO_CHANGE_STATE               (0x4D03) ///< Unable to change state.
#define SL_STATUS_SI91X_BLE_NOT_CONNECTED                    (0x4D04) ///< BLE not Connected.
#define SL_STATUS_SI91X_BLE_SOCKET_NOT_AVAILABLE             (0x4D05) ///< BLE socket not available.
#define SL_STATUS_SI91X_ATTRIBUTE_RECORD_NOT_FOUND           (0x4D06) ///< Attribute record not found.
#define SL_STATUS_SI91X_ATTRIBUTE_ENTRY_NOT_FOUND            (0x4D07) ///< Attribute entry not found.
#define SL_STATUS_SI91X_PROFILE_RECORD_FULL                  (0x4D08) ///< Profile record full.
#define SL_STATUS_SI91X_ATTRIBUTE_RECORD_FULL                (0x4D09) ///< Attribute record full.
#define SL_STATUS_SI91X_BLE_PROFILE_HANDLER_INVALID          (0x4D0A) ///< BLE profile not found(profile handler invalid).
#define SL_STATUS_SI91X_BLE_ATTRIBUTE_BUFFER_FULL            (0x4D0B) ///< BLE Attribute Buffer Full .
#define SL_STATUS_SI91X_BLE_CONNECTION_SOCK_NOT_AVAILABLE    (0x4D10) ///< BLE Connection Sock not Available.
#define SL_STATUS_SI91X_BLE_REMOTE_CREDITS_NOT_AVAILABLE     (0x4D11) ///< BLE Remote Credits not Available.
#define SL_STATUS_SI91X_PARAMETER_OUTOFF_MANADATORY_RANGE    (0x4D14) ///< Parameter is outoff the manadatory range.
#define SL_STATUS_SI91X_ERROR_BLE_HW_BUF_OVERFLOW            (0x4D16) ///< Sufficient buffers are not available in TA.

/** @} */
