/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_status.h"
#include "sl_constants.h"

/** \addtogroup SL_SI91X_TYPES
 * @{
 * */
/**
 * @brief Si91x specific HTTP OTAF parameters.
 *
 * This structure contains the parameters required for HTTP Over-The-Air Firmware (OTAF) updates
 * specific to the Si91x series.
 * 
 * @note
 *   The following table lists the flags that can be used with this function:
 * 
 *   | Flags  | Description                                                            |
 *   |:-------|:-----------------------------------------------------------------------|
 *   | BIT(0) | Set this bit to enable HTTPS feature.                                  |
 *   | BIT(1) | Set this bit to enable IPv6. By default, it is configured to IPv4.     |
 *   | BIT(2) | Set this bit to support TLS Version 1.0 if HTTPS is enabled.           |
 *   | BIT(3) | Set this bit to support TLS Version 1.2 if HTTPS is enabled.           |
 *   | BIT(4) | Set this bit to support TLS Version 1.1 if HTTPS is enabled.           |
 *   | BIT(6) | Set this bit to use HTTP version 1.1                                   |
 *   | BIT(9) | Set this bit to specify index of SSL cert to be used for HTTPS.        |
 *   | BIT(10)| Set this bit to specify index of SSL cert to be used for HTTPS.        |
 */
typedef struct {
  uint16_t flags;           /**< Configuration flags. */
  uint8_t *ip_address;      /**< Server IP address. */
  uint16_t port;            /**< Port number. Default: 80 for HTTP, 443 for HTTPS. */
  uint8_t *resource;        /**< 
                      * @brief Requested resource URL in string format.
                      * @details 
                      * - The maximum supported HTTP URL is 2048 bytes when the SL_SI91X_FEAT_LONG_HTTP_URL bit is enabled in the feature_bit_map.
                      * - If the SL_SI91X_FEAT_LONG_HTTP_URL bit is disabled, the maximum supported length for the HTTP URL is (872 - (length of user_name + length of password) - length of hostname - length of IP address) bytes, excluding delimiters.
                      */
  uint8_t *host_name;       /**< Host name. */
  uint8_t *extended_header; /**< 
                             * @brief User-configurable header fields to append to the default HTTP/HTTPS header.
                             * @details The extended header can have multiple header fields, each ended by "\r\n" (0xD 0xA).
                             *          Example: key1:value1"\r\n"key2:value2"\r\n"
                             */
  uint8_t *user_name;       /**< Username for server authentication. */
  uint8_t *password;        /**< Password for server authentication. */
} sl_si91x_http_otaf_params_t;
/** @} */

/** \addtogroup SI91X_FIRMWARE_UPDATE_FROM_HOST_FUNCTIONS 
 * \ingroup SI91X_FIRMWARE_UPDATE_FUNCTIONS
 * @{ */

/***************************************************************************/ /**
 * @brief
 *   Send the RPS header content of the firmware file. 
 * 
 * @details
 *  This function sends the RPS (Remote Programming Service) header content of the firmware file to the Si91x device. 
 * 
 *  This is a blocking API.
 * 
 * @param[in] rps_header
 *   Pointer to the RPS header content.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *  
 * @note
 *  The following table summarizes the support for different modes and network stacks:
 * 
 *  | Mode      | Hosted Network Stack | Offload Network Stack |
 *  |-----------|----------------------|-----------------------|
 *  | SoC       | Supported            | Supported             |
 *  | NCP       | Supported            | Supported             |
 ******************************************************************************/
sl_status_t sl_si91x_fwup_start(const uint8_t *rps_header);

/***************************************************************************/ /**
 * @brief
 *   Send the firmware file content. 
 * 
 * @details
 *   This function sends the content of the firmware file to the Si91x device. 
 * 
 *   This is a blocking API.
 * 
 * @param[in] content
 *   Pointer to the firmware file content.
 * 
 * @param[in] length
 *   Length of the content in bytes.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *  
 * @note
 *  The following table summarizes the support for different modes and network stacks:
 * 
 *  | Mode      | Hosted Network Stack | Offload Network Stack |
 *  |-----------|----------------------|-----------------------|
 *  | SoC       | Supported            | Supported             |
 *  | NCP       | Supported            | Supported             |
 ******************************************************************************/
sl_status_t sl_si91x_fwup_load(const uint8_t *content, uint16_t length);

/***************************************************************************/ /**
 * @brief
 *   Abort firmware installation during firmware update process.
 * 
 * @details
 *   This function aborts the firmware installation phase during the firmware update process. It effectively halts any installation during next bootup.
 *
 *   This is a blocking API.
 *
 * @pre Pre-conditions:
 * - @ref sl_si91x_fwup_load should be called before this API.
 *
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *
 * @note
 *   Call this API before performing a soft or hard reset of the SiWx91x device in scenarios where the update needs to be canceled for any reason.
 *
 * @note
 *   Calling the API would be ineffective, if only partial firmware chunks have been received and the update hasn't been marked as complete.
 *
 * @note
 *  The following table summarizes the support for different modes and network stacks:
 * 
 *  | Mode      | Hosted Network Stack | Offload Network Stack |
 *  |-----------|----------------------|-----------------------|
 *  | SoC       | Supported            | Supported             |
 *  | NCP       | Supported            | Supported             |
 ******************************************************************************/
sl_status_t sl_si91x_fwup_abort(void);

/***************************************************************************/ /**
 * @brief
 *   Flash firmware to the Wi-Fi module via the bootloader. 
 * 
 * @details
 *   This function flashes the firmware to the Wi-Fi module using the bootloader. The firmware image, its size, and the position flags are provided as parameters.
 * 
 *  This is a blocking API.
 * 
 * @param[in] firmware_image
 *   Pointer to the firmware image.
 * 
 * @param[in] fw_image_size
 *   Size of the firmware image in bytes.
 * 
 * @param[in] flags 
 *   Flags indicating the chunk position in the file:
 *   - 0: Middle of the file
 *   - 1: Start of the file
 *   - 2: End of the file 
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * 
 * @note
 *  The following table summarizes the support for different modes and network stacks:
 * 
 *  | Mode      | Hosted Network Stack | Offload Network Stack |
 *  |-----------|----------------------|-----------------------|
 *  | SoC       | Not-Supported        | Not-Supported         |
 *  | NCP       | Supported            | Supported             |
 ******************************************************************************/
#if !defined(SLI_SI91X_MCU_INTERFACE) || defined(DOXYGEN)
sl_status_t sl_si91x_bl_upgrade_firmware(uint8_t *firmware_image, uint32_t fw_image_size, uint8_t flags);
#endif

/***************************************************************************/ /**
 * @brief
 *   Enable fast firmware upgrade mode.
 * 
 * @details
 *   This function enables the fast firmware upgrade mode on the Si91x device. It optimizes the firmware upgrade process for speed.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *  
 * @note
 *  The following table summarizes the support for different modes and network stacks:
 * 
 *  | Mode      | Hosted Network Stack | Offload Network Stack |
 *  |-----------|----------------------|-----------------------|
 *  | SoC       | Not-Supported        | Not-Supported         |
 *  | NCP       | Supported            | Supported             |
 ******************************************************************************/
#if !defined(SLI_SI91X_MCU_INTERFACE) || defined(DOXYGEN)
sl_status_t sl_si91x_set_fast_fw_up(void);
#endif

/** @} */

/** \addtogroup SI91X_FIRMWARE_UPDATE_FROM_MODULE_FUNCTIONS 
 * \ingroup SI91X_FIRMWARE_UPDATE_FUNCTIONS
 * @{ */
/***************************************************************************/ /**
 * @brief
 *   Create an OTAF client and initialize it with a given configuration.
 * 
 * @details
 *   This function creates an OTAF (Over-The-Air Firmware) client and initializes it with the provided configuration parameters. 
 * 
 *   It supports both synchronous and asynchronous firmware upgrades.
 * 
 *   In synchronous mode, the response is received via [sl_net_event_handler_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-net-types#sl-net-event-handler-t) with [SL_NET_OTA_FW_UPDATE_EVENT](../wiseconnect-api-reference-guide-nwk-mgmt/sl-net-constants#sl-net-event-t) as the event.
 *  
 * @pre Pre-conditions:
 * - [sl_net_up](../wiseconnect-api-reference-guide-nwk-mgmt/net-interface-functions#sl-net-up) API needs to be called before this API.
 * 
 * @param[in] server_ip
 *   OTAF server IP address of type [sl_ip_address_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-ip-address-t).
 * 
 * @param[in] server_port
 *   OTAF server port number.
 * 
 * @param[in] chunk_number 
 *   Firmware content request chunk number.
 * 
 * @param[in] timeout
 *   TCP receive packet timeout.
 * 
 * @param[in] tcp_retry_count
 *   TCP retransmissions count.
 * 
 * @param[in] asynchronous
 *   OTAF upgrade done asynchronously when this is set to true, else synchronous upgrade.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * 
 * @note
 *   For a safe firmware upgrade via TCP server, it will take approximately 65 seconds to upgrade the firmware of a 1.5 MB file.
 * 
 * @note
 *  The following table summarizes the support for different modes and network stacks:
 * 
 *  | Mode      | Hosted Network Stack | Offload Network Stack |
 *  |-----------|----------------------|-----------------------|
 *  | SoC       | Not-Supported        | Supported             |
 *  | NCP       | Not-Supported        | Supported             |
 *
 * @note
 *   This function uses a user-configurable timeout parameter that is not affected
 *   by the global timeout scaling factors (SL_WIFI_INTERNAL_COMMANDS_TIMEOUT_SF,
 *   SL_WIFI_MANAGEMENT_COMMANDS_TIMEOUT_SF, SL_WIFI_NETWORK_COMMANDS_TIMEOUT_SF)
 *   or the additional wait time configuration (SL_TX_ADDITIONAL_WAIT_TIME).
 ******************************************************************************/
#if defined(SLI_SI91X_OFFLOAD_NETWORK_STACK) || defined(DOXYGEN)
sl_status_t sl_si91x_ota_firmware_upgradation(sl_ip_address_t server_ip,
                                              uint16_t server_port,
                                              uint16_t chunk_number,
                                              uint16_t timeout,
                                              uint16_t tcp_retry_count,
                                              bool asynchronous);
#endif /* SLI_SI91X_OFFLOAD_NETWORK_STACK */

/***************************************************************************/
/**
 * @brief
 *  Sends an HTTP request to a specified server URL
 * 
 * @details      
 *  This function sends an HTTP request to a specified server URL with optional data and headers,
 *  and facilitates firmware downloads from the server. 
 * 
 *  This API is blocking API.
 * 
 *  By default, the following values are used:
 *  - SSL version: 1.0
 *  - HTTP version: 1.0
 *  - Certificate index: 0
 * 
 * @param[in] type
 *  (Not supported)
 * 
 * @param[in] flags 
 *   Configuration flags. See the table below for details.
 * 
 * @param[in] ip_address 
 *   Server IP address.
 * 
 * @param[in] port 
 *   Port number. Default: 80 - HTTP, 443 - HTTPS
 * 
 * @param[in] resource 
 *   Requested resource URL in string format. 
 *   - The maximum supported HTTP URL is 2048 bytes, when the SL_SI91X_FEAT_LONG_HTTP_URL Bit is enabled in the feature_bit_map. 
 *   - If the SL_SI91X_FEAT_LONG_HTTP_URL Bit is disabled, then the maximum supported length for HTTP URL is (872-(length of User_name + length of Password) - length of hostname - length of IP address) bytes excluding delimiters.
 * 
 * @param[in] host_name       
 *   Host name.
 * 
 * @param[in] extended_header 
 *   User-configurable header fields to append to the default HTTP/HTTPS header.
 *  - The extended header may contain multiple header fields, with each field terminated by "\r\n" (0x0D 0x0A).
 * 
 *    Example: key1:value1"\r\n"key2:value2"\r\n"
 * 
 * @param[in] user_name        
 *   Username for server authentication.
 * 
 * @param[in] password         
 *   Password for server authentication.
 * 
 * @param[in] post_data        
 *   HTTP data to be posted to the server. (Currently not supported)
 * 
 * @param[in] post_data_length 
 *   HTTP data length to be posted to the server. (Currently not supported)
 * 
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 * 
 * @note
 *   The following table lists the flags that can be used with this function:
 * 
 *   | Flags  | Description                                                            |
 *   |:-------|:-----------------------------------------------------------------------|
 *   | BIT(0) | Set this bit to enable HTTPS feature.                                  |
 *   | BIT(1) | Set this bit to enable IPv6. By default, it is configured to IPv4.     |
 *   | BIT(2) | Set this bit to support TLS Version 1.0 if HTTPS is enabled.           |
 *   | BIT(3) | Set this bit to support TLS Version 1.2 if HTTPS is enabled.           |
 *   | BIT(4) | Set this bit to support TLS Version 1.1 if HTTPS is enabled.           |
 *   | BIT(6) | Set this bit to use HTTP version 1.1                                   |
 *   | BIT(9) | Set this bit to specify index of SSL cert to be used for HTTPS.        |
 *   | BIT(10)| Set this bit to specify index of SSL cert to be used for HTTPS.        |
 *   | BIT(11)| Set this bit to enable server name indication (SNI) for HTTPS.         |
 * 
 * - Maximum supported length for user_name, password together is 278 bytes.
 * - Maximum length should be 872 bytes, which includes user_name, password, host_name, ip_address, resource, and extended_header.
 * - If username, password, hostname and extended http headers are not required, user should send empty string separated by delimiter.
 * - If content of any field contains a comma then NULL delimiter should be used.
 * - This API will wait until the response is received from NWP.
 * - When the SL_SI91X_FEAT_LONG_HTTP_URL feature is enabled, the maximum supported URL length for HTTP OTAF is 2048 bytes.
 * - To select certificate index 0, no additional flags are required to be configured explicitly.
 * 
 * - The following table summarizes the support for different modes and network stacks:
 * 
 *  | Mode      | Hosted Network Stack | Offload Network Stack |
 *  |-----------|----------------------|-----------------------|
 *  | SoC       | Not-Supported        | Supported             |
 *  | NCP       | Not-Supported        | Supported             |
 * 
 * - Moving forward, this structure will be deprecated. Instead, use the [sl_si91x_http_otaf_v2](../wiseconnect-api-reference-guide-si91x-driver/si91-x-firmware-update-from-module-functions#sl-si91x-http-otaf-v2) structure. This is retained for backward compatibility.
 ******************************************************************************/
#if defined(SLI_SI91X_OFFLOAD_NETWORK_STACK) || defined(DOXYGEN)
sl_status_t sl_si91x_http_otaf(uint8_t type,
                               uint16_t flags,
                               uint8_t *ip_address,
                               uint16_t port,
                               uint8_t *resource,
                               uint8_t *host_name,
                               uint8_t *extended_header,
                               uint8_t *user_name,
                               uint8_t *password,
                               const uint8_t *post_data,
                               uint32_t post_data_length) SL_DEPRECATED_API_WISECONNECT_3_5;
#endif /* SLI_SI91X_OFFLOAD_NETWORK_STACK */

/***************************************************************************/
/**
 * @brief
 *  Sends an HTTP request to a specified server URL
 * 
 * @details      
 *  This function sends an HTTP request to a specified server URL with optional data and headers,
 *  and facilitates firmware downloads from the server. 
 * 
 *  This API is blocking API.
 * 
 *  By default, the following values are used:
 *  - SSL version: 1.0
 *  - HTTP version: 1.0
 *  - Certificate index: 0
 *  
 * @param[in] http_otaf_params 
 *  HTTP OTAF parameters   
 * 
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 * 
 * @note
 * - Maximum supported length for user_name, password together is 278 bytes.
 * - Maximum length should be 872 bytes, which includes user_name, password, host_name, ip_address, resource, and extended_header.
 * - If username, password, hostname and extended http headers are not required, user should send empty string separated by delimiter.
 * - If content of any field contains a comma then NULL delimiter should be used.
 * - This API will wait until the response is received from NWP.
 * - When the SL_SI91X_FEAT_LONG_HTTP_URL feature is enabled, the maximum supported URL length for HTTP OTAF is 2048 bytes.
 * - To select certificate index 0, no additional flags need to be configured explicitly.
 ******************************************************************************/
#if defined(SLI_SI91X_OFFLOAD_NETWORK_STACK) || defined(DOXYGEN)
sl_status_t sl_si91x_http_otaf_v2(const sl_si91x_http_otaf_params_t *http_otaf_params);
#endif /* SLI_SI91X_OFFLOAD_NETWORK_STACK */
/** @} */
