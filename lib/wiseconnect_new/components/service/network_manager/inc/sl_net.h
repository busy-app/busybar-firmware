/***************************************************************************/ /**
 * @file
 * @brief SL Network API
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_net_types.h"

/** 
 * \addtogroup NET_INTERFACE_FUNCTIONS Network Interface
 * 
 * @note To manage the network interface lifecycle correctly, it's important to use functions like sl_net_init(), sl_net_up(), sl_net_down(), and sl_net_deinit() as intended.
 *       Improper sequencing or combining sl_net_* and sl_wifi_* API functions without following the recommended order may lead to unpredictable or undefined behavior.
 * @note Stack overflows may occur if you invoke functions or use your own variables or data structures while handling callbacks.
 *       Please configure the stack size by modifying the pre-processor macro `SL_SI91X_EVENT_HANDLER_STACK_SIZE` as
 *       per your application's requirements. See [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-prog-preprocessor-build-settings/list-of-preprocessor-build-settings)
 *       for the instructions for modifying a pre-processor macro.
 * @note Event/Callback handlers must not contain function calls or code which can block or delay the execution of 
 *       the event/callback handler as it will cause all the other events to queue up and delay the execution of 
 *       other events since all the events are invoked and handled from a single thread.
 * @note Do not call any synchronous SDK APIs from within the Event/Callback handlers.
 * 
 * \ingroup SL_NET_FUNCTIONS
 * @{ */

/***************************************************************************/ /**
 * @brief Initialize the specified network interface.
 * 
 * This function initializes the specified network interface with the provided configuration,
 * network context, and event handler. It supports various network interfaces such as Wi-Fi client,
 * Wi-Fi access point. 
 * 
 * Once the user passes a function pointer to the event handler, the network context is passed in the callback,
 * and various events can be received through this callback.
 * 
 * @param[in] interface
 *   The network interface to initialize. One of the values from @ref sl_net_interface_t
 * @param[in] configuration
 *   Pointer to the configuration structure for the specified interface of type [sl_wifi_device_configuration_t](../wiseconnect-api-reference-guide-si91x-driver/sl-wifi-device-configuration-t).
 *   If NULL, then the following configuration is used internally by SDK:
 * | sl_net_interface_t           | Default configuration                 |
 * |:-----------------------------|:--------------------------------------|
 * | SL_NET_WIFI_CLIENT_INTERFACE | sl_wifi_default_client_configuration  |
 * | SL_NET_WIFI_AP_INTERFACE     | sl_wifi_default_ap_configuration      |
 * @param[in] network_context
 *   Runtime context specific to network interface.
 * @param[in] event_handler
 *   Function pointer to the network event handler callback of @ref sl_net_event_handler_t type
 * 
 * @return
 *  sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note 
 *  For Wi-Fi events, sl_net uses the wifi callback framework. Register the corresponding Wi-Fi event handlers using [sl_wifi_set_callback](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-callback) API.
 * @note
 *  The \p network_context parameter is used only when the module is acting as a station in external stack mode (lwIP).
 *  In this case, \p network_context should refer to a valid @ref sl_net_wifi_lwip_context_t variable.
 * @note
 *  In SoC mode, wireless initialization must be completed before using the NVM3 APIs in the common flash, as flash write and erase operations require communication between the NWP & M4.
 * @note
 * It is recommended not to access the interfaces once the @ref sl_net_deinit is performed to avoid unintented behavior.
 * It is recommended to avoid using other sl_net_interface_t during sl_net_init.
 ******************************************************************************/
sl_status_t sl_net_init(sl_net_interface_t interface,
                        const void *configuration,
                        void *network_context,
                        sl_net_event_handler_t event_handler);

/***************************************************************************/ /**
 * @brief
 *   De-initializes network interfaces.
 * 
 * This function de-initializes any network interfaces that were created, and releases any resources that were allocated during initialization.
 * 
 *  After this, the user will not receive callbacks related to events.
 * 
 * 
 * @pre Pre-conditions:
 * - @ref sl_net_init should be called before this API.
 * 
 * @param[in] interface
 *   This function de-initializes both Access Point and Station interfaces, regardless of the specified interface.
 *   The interface is identified by @ref sl_net_interface_t.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *
 * @note
 * This function should not be called after an M4 OTA firmware upgrade, as it only resets the TA while the M4 application continues to run, which could cause unexpected issues. Instead, use [sl_si91x_soc_nvic_reset()](../wiseconnect-api-reference-guide-common/soft-reset-functions#sl-si91x-soc-nvic-reset) to perform a complete reset of both the TA and M4.
 * @note
 * It is recommended not to access the interfaces once the sl_net_deinit is performed to avoid unintended behavior.
 * @note
 * It is recommended to call @ref sl_net_deinit for a particular interface from the same thread in which it was initialized to ensure thread safety and proper cleanup.
 ******************************************************************************/
sl_status_t sl_net_deinit(sl_net_interface_t interface);

/***************************************************************************/ /**
 * @brief
 *   Bring a network interface up.
 * 
 * @details
 *   This function brings the specified network interface up, making it ready for network communication.
 *   
 *   For `SL_NET_WIFI_CLIENT_INTERFACE`, the API fetches profile data from the profile ID, scans the network and connects to the network, configures the IP address, and updates the profile data.
 *   
 *   For `SL_NET_WIFI_AP_INTERFACE`, the API fetches profile data from the profile ID, configures the IP address, updates the profile data, and starts the Access Point (AP).
 * 
 * @pre Pre-conditions:
 * - 
 *   @ref sl_net_init should be called before this API.
 * 
 * @param[in] interface
 *   Interface identified by @ref sl_net_interface_t.
 * 
 * @param[in] profile_id
 *   Network profile identifier for the specific interface of type @ref sl_net_profile_id_t
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * 
 * @note
 * By default, profile and credential configurations in sl_net_defaults.h are used by SDK.
 * @note
 * To enable support for both IPv4 and IPv6, the ip.type in the profile should be set to (SL_IPV4|SL_IPV6).
 * @note
 * The user can define their profile and credential configurations for an interface by calling @ref sl_net_set_profile and @ref sl_net_set_credential APIs before calling @ref sl_net_up API.
 * @note
 * The user is advised to reset the NWP with @ref sl_net_deinit() if this API fails with error SL_STATUS_TIMEOUT.
 * @note
 * This API is not atomic. It performs multiple operations (network scanning, connection, IP configuration) sequentially. If any step fails after previous steps have completed, you may leave the interface in a partially configured state (e.g., connected to Wi-Fi but without an IP address). In such cases, call @ref sl_net_down to properly clean up before retrying.
 * ******************************************************************************/
sl_status_t sl_net_up(sl_net_interface_t interface, sl_net_profile_id_t profile_id);

/***************************************************************************/ /**
 * @brief
 *   Bring a network interface up asynchronously.
 * 
 * @details
 *   This function initiates bringing the specified network interface up in an asynchronous manner.
 *   The function returns immediately after posting the request to the network manager thread.
 *   Progress and completion are reported via the registered event handler set by @ref sl_net_init().
 * 
 * @pre Pre-conditions:
 * - @ref sl_net_init should be called before this API.
 * - An event handler must be registered via @ref sl_net_init() to receive completion notifications.
 * 
 * @param[in] interface
 *   Interface identified by @ref sl_net_interface_t.
 * 
 * @param[in] profile_id
 *   Network profile identifier for the specific interface of type @ref sl_net_profile_id_t
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *   - SL_STATUS_IN_PROGRESS: Request accepted and processing has started
 *   - SL_STATUS_BUSY: Another async operation is already in progress for this interface
 *   - SL_STATUS_INVALID_PARAMETER: Invalid interface or profile_id
 *   - SL_STATUS_NOT_INITIALIZED: Network manager not initialized
 *   - SL_STATUS_FAIL: Failed to post request to network manager queue
 * 
 * @note
 * By default, profile and credential configurations in sl_net_defaults.h are used by SDK.
 * @note
 * To enable support for both IPv4 and IPv6, the ip.type in the profile should be set to (SL_IPV4|SL_IPV6).
 * @note
 * The user can define their profile and credential configurations for an interface by calling @ref sl_net_set_profile and @ref sl_net_set_credential APIs before calling this API.
 * @note
 * Only one asynchronous bring-up operation is allowed per interface at a time. Calling this API while an async operation is in progress will return SL_STATUS_BUSY.
 * @note
 * The registered event handler will receive events indicating connection progress, success, or failure.
 * @note
 * This API does not support auto-join (SL_NET_AUTO_JOIN) profile. Use @ref sl_net_up with SL_NET_AUTO_JOIN for auto-join functionality.
 * ******************************************************************************/
sl_status_t sl_net_up_async(sl_net_interface_t interface, sl_net_profile_id_t profile_id);

/***************************************************************************/ /**
 * @brief
 *   Bring a network interface down.
 * 
 * @details
 *   This function deactivates the specified network interface, effectively 
 *   disconnecting it from the WLAN network. It should be called to properly 
 *   shut down the interface and release any associated resources.
 * 
 * @pre Pre-conditions:
 * - @ref sl_net_up should be called before this API to ensure the interface 
 *   is active before attempting to bring it down.
 * 
 * @param[in] interface
 *   Interface identified by @ref sl_net_interface_t. This parameter specifies 
 *   which network interface to bring down.
 * 
  * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) 
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
sl_status_t sl_net_down(sl_net_interface_t interface);

/***************************************************************************/ /**
 * @brief
 *   Gets network interface information for WLAN in AP mode or Client (Station) mode.
 * @pre Pre-conditions:
 *   - The device must be initialized by calling @ref sl_net_init before using this API.
 * @param[in] interface
 *   The network interface to query. Supported values are SL_NET_WIFI_CLIENT_INTERFACE (Station mode) and SL_NET_WIFI_AP_INTERFACE (AP mode).
 * @param[out] info
 *   Pointer to a `sl_net_interface_info_t` structure that will be populated with the interface information.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sl_net_get_interface_info(sl_net_interface_t interface, sl_net_interface_info_t *info);

/** @} */

/** 
 * \addtogroup NET_IP_MANAGEMENT_FUNCTIONS IP Management
 * \ingroup SL_NET_FUNCTIONS
 * @{ */

/***************************************************************************/ /**
 * @brief
 *   Configure IP address of given interface.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_net_up should be called before this API.
 * @param[in] interface
 *   Interface identified by @ref sl_net_interface_t
 * @param[in] ip_config
 *   Multicast IP address of type @ref sl_net_ip_configuration_t
 * @param[in] timeout
 *   Allowed range for finite timeout: [1,(1<<30)-1] milliseconds.
 *   If bit (1 << 31) is set and bit (1<<30) is not set, the value is treated as an indefinite timeout, regardless of the other bits.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sl_net_configure_ip(sl_net_interface_t interface,
                                const sl_net_ip_configuration_t *ip_config,
                                uint32_t timeout);

/***************************************************************************/ /**
 * @brief 
 *   This function retrieves the IP address of the specified network interface.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_net_up should be called before this API.
 * @param[in] interface
 *   Interface identified by @ref sl_net_interface_t
 * @param[in] ip_address
 *   IP address of type @ref sl_net_ip_address_t
 * @param[in] timeout
 *   The maximum time to wait for the IP address retrieval, in milliseconds.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
@note
 * - This API doesn't support async mode operation, so passing 0 in timeout parameter leads to an error.
 * - If the interface is setup in SL_IP_MANAGEMENT_STATIC_IP mode, this API only returns its mode and doesn't return ip address.
 ******************************************************************************/
sl_status_t sl_net_get_ip_address(sl_net_interface_t interface, sl_net_ip_address_t *ip_address, uint32_t timeout);

/***************************************************************************/ /**
 * @brief 
 * Enables Network Address Translation (NAT) on the specified network interface in concurrent mode.
 * This function enables NAT on the given network interface. This is a blocking API.
 * 
 * @pre Pre-conditions:
 *
 *   @ref sl_net_init should be called before this API on AP and STA interfaces.
 *   @ref sl_net_up should be called before this API on AP and STA interfaces.
 * 
 * @param[in] nat_config
 *   Pointer to the NAT configuration structure of type @ref sl_net_nat_config_t.
 *
 * @return status of the operation.
 *         - SL_STATUS_OK: On successful completion of the operation.
 *         - SL_STATUS_FAIL: If the operation fails.
 *         - SL_STATUS_INVALID_PARAMETER: If the nat_config parameter is NULL.
 ******************************************************************************/
sl_status_t sl_net_nat_enable(const sl_net_nat_config_t *nat_config);

/***************************************************************************/ /**
 * @brief 
 * Disables Network Address Translation (NAT) on the specified network interface in concurrent mode.
 * This function disables NAT on the given network interface. This is a blocking API.
 * 
 * @pre Pre-conditions:
 * 
 *   @ref sl_net_nat_enable should be called before this API.
 * 
 * @param[in] interface 
 *   The network interface on which to disable NAT. (This parameter is currently unused.)
 *
 * @return Status of the operation.
 *         - SL_STATUS_OK: On successful completion of the operation.
 *         - SL_STATUS_FAIL: If the operation fails.
 ******************************************************************************/
sl_status_t sl_net_nat_disable(const sl_net_interface_t interface);

/** @} */

/** 
 * \addtogroup NET_PROFILE_FUNCTIONS Network Profiles
 * \ingroup SL_NET_FUNCTIONS
 * @{ */

/***************************************************************************/ /**
 * @brief
 *   Store a network profile for a given interface.
 * 
 *  @details
 *   This function stores the network profile data such as WIFI Credentials and Network Credentials for the specified interface. 
 *   The profile can be used later to bring the interface up with the stored settings.
 * 
 *   The user can use the id to store multiple profiles for the same interface and pass the id to different APIs.
 * 
 *  @pre Pre-conditions:
 * - @ref sl_net_init should be called before this API.
 * 
 * @param[in] interface
 *   Interface identified by @ref sl_net_interface_t.
 * 
 * @param[in] id
 *   Profile storage index / identifier of type @ref sl_net_profile_id_t.
 * 
 * @param[in] profile
 *   Pointer to profile data of type @ref sl_net_profile_t.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * 
 * @note
 * When configuring Wi-Fi clients or Access Points with open security (SL_WIFI_OPEN), 
 * set the credential_id field to SL_WIFI_NO_CREDENTIAL_ID within the respective 
 * configuration structures (sl_wifi_client_configuration_t in sl_net_wifi_client_profile_t for clients 
 * and sl_wifi_ap_configuration_t in sl_net_wifi_ap_profile_t for APs). 
 * This ensures the stack does not attempt to fetch credentials that are not required for open security.
 ******************************************************************************/
sl_status_t sl_net_set_profile(sl_net_interface_t interface, sl_net_profile_id_t id, const sl_net_profile_t *profile);

/***************************************************************************/ /**
 * @brief
 *   Retrieve a stored network profile for a given interface.
 * 
 * @details
 *   This function retrieves the network profile data for the specified interface and profile ID.
 *   The retrieved profile data is stored in the provided profile object.
 * 
 * @pre Pre-conditions:
 * -
 *   @ref sl_net_init should be called before this API.
 * 
 * @param[in] interface
 *   Interface identified by @ref sl_net_interface_t.
 * 
 * @param[in] id
 *   Profile storage index / identifier of type @ref sl_net_profile_id_t.
 * 
 * @param[out] profile
 *   Pointer to @ref sl_net_profile_t object that will store the retrieved profile data.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
sl_status_t sl_net_get_profile(sl_net_interface_t interface, sl_net_profile_id_t id, sl_net_profile_t *profile);

/***************************************************************************/ /**
 * @brief
 *   Delete a stored network profile for a given interface.
 * 
 * @details
 *   This function deletes the network profile data for the specified interface and profile ID.
 *   Once deleted, the profile cannot be used to bring the interface up.
 * 
 * @pre Pre-conditions:
 * - @ref sl_net_init should be called before this API.
 * 
 * @param[in] interface
 *   Interface identified by @ref sl_net_interface_t.
 * 
 * @param[in] id
 *   Profile storage index / identifier of type @ref sl_net_profile_id_t.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
sl_status_t sl_net_delete_profile(sl_net_interface_t interface, sl_net_profile_id_t id);

/** @} */

/** 
 * \addtogroup NET_CREDENTIAL_FUNCTIONS Network Credential
 * \ingroup SL_NET_FUNCTIONS
 * @{ */

/***************************************************************************/ /**
 * @brief
 *   Set a network credential including client credentials, certificates, and keys.
 * 
 * @details
 *   This function sets the network credential type and data for the specified credential ID.
 *   The credential data can include client credentials, certificates, and keys.
 * 
 *   Repeatedly calling this API with the same ID will overwrite the existing credential type and data.
 *  
 * @pre Pre-conditions:
 * - @ref sl_net_init should be called before this API.
 * 
 * @param[in] id
 *   Network credential identifier as identified by @ref sl_net_credential_id_t.
 * 
 * @param[in] type
 *   Network credential type as identified by @ref sl_net_credential_type_t.
 * 
 * @param[in] credential
 *   Pointer to the credential data object.
 * 
 * @param[in] credential_length
 *   Length of the credential data object.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *   If the credential is NULL or the credential length is zero, this API will return an error `SL_STATUS_INVALID_PARAMETER`.
 *   If the credential_length exceeds NVM3_DEFAULT_MAX_OBJECT_SIZE when using NVM3 storage, this API will return an error `SL_STATUS_NVM3_WRITE_DATA_SIZE`.
 * @note
 * - Certificates should follow the standard array format.  
 * - After every 64 bytes, the special character `\n` should be used as a delimiter.
 * - The private key is prefixed with a header like "-----BEGIN PRIVATE KEY-----" line and postfixed with an footer like"-----END PRIVATE KEY-----". 
 * - Certificates are prefixed with a header like "-----BEGIN CERTIFICATE-----" line and postfixed with an footer like"-----END CERTIFICATE-----" line. 
 * - Text outside the prefix and postfix lines is ignored and can be used for metadata.
 * - The above mentioned Headers and Footers might vary 
 * - This API does not support the OPEN Security type for Wi-Fi client credentials.
 * - When configuring AP or client interfaces in open security mode, the credential ID must be set to `SL_NET_NO_CREDENTIAL_ID`.
 ******************************************************************************/
sl_status_t sl_net_set_credential(sl_net_credential_id_t id,
                                  sl_net_credential_type_t type,
                                  const void *credential,
                                  uint32_t credential_length);

/***************************************************************************/ /**
 * @brief
 *   Retrieve a stored network credential.
 * 
 * @details
 *   This function retrieves the network credential data for the specified credential ID.
 *   The retrieved credential data is stored in the provided credential object.
 * 
 * @pre Pre-conditions:
 * - @ref sl_net_init should be called before this API.
 * 
 * @param[in] id
 *   Network credential identifier as identified by @ref sl_net_credential_id_t.
 * 
 * @param[out] type
 *   Network credential type as identified by @ref sl_net_credential_type_t.
 * 
 * @param[out] credential
 *   Pointer to location where credential data is stored.
 * 
 * @param[in,out] credential_length
 *   in: Number of bytes available at credential,
 *   out: Number of bytes written.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * 
 * @note
 *   Currently, @ref SL_NET_CERTIFICATE and @ref SL_NET_SIGNING_CERTIFICATE are not supported for retrieval.
 ******************************************************************************/
sl_status_t sl_net_get_credential(sl_net_credential_id_t id,
                                  sl_net_credential_type_t *type,
                                  void *credential,
                                  uint32_t *credential_length);

/***************************************************************************/ /**
 * @brief
 *   Delete a stored network credential.
 * 
 * @details
 *   This function deletes the network credential data for the specified credential ID and type.
 *   Once deleted, the credential cannot be used for network operations.
 * 
 * @pre Pre-conditions:
 * -
 *   @ref sl_net_init should be called before this API.
 * 
 * @param[in] id
 *   Network credential identifier as identified by @ref sl_net_credential_id_t.
 * 
 * @param[out] type
 *   Network credential type as identified by @ref sl_net_credential_type_t.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
sl_status_t sl_net_delete_credential(sl_net_credential_id_t id, sl_net_credential_type_t type);

/** @} */

/***************************************************************************/ /**
 * @brief 
 * @param[in] id
 * @param[in] data
 * @param[in] data_length
 * @note
 *   This API is not yet implemented.
 ******************************************************************************/
sl_status_t sl_net_set_certificate(sl_net_certificate_id_t id, const void *data, uint32_t data_length);

/***************************************************************************/ /**
 * @brief 
 * @param[in]  id
 * @param[out] data
 * @param[in]  data_length
 * @note
 *   This API is not yet implemented.
 ******************************************************************************/
sl_status_t sl_net_get_certificate(sl_net_certificate_id_t id, const void *data, uint32_t data_length);

/***************************************************************************/ /**
 * @note
 *   This API is not yet implemented.
 ******************************************************************************/
sl_status_t sl_net_verify_certificate();

/***************************************************************************/ /**
 * @brief
 *   Convert an IPv4 address in string of from a.b.c.d to a binary uint32_t value
 * @param[in] addr
 *   IPV4 address.
 * @param[out] value
 *   Binary value of the given IP address.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sl_net_inet_addr(const char *addr, uint32_t *value);

/** 
 * \addtogroup NET_MULTICAST_FUNCTIONS Network Multicast
 * \ingroup SL_NET_FUNCTIONS
 * @{ */

/***************************************************************************/ /**
 * @brief
 *   Enable multicast for the given IP address.
 * 
 * @details
 *   This function enables multicast for the specified IP address on the given interface.
 *   It allows the interface to receive or send multicast packets to the specified IP address.
 *
 * @note
 *   IGMPv1 and IGMPv2 are supported.
 * 
 *   Users can use [sendto](../wiseconnect-api-reference-guide-sockets/bsd-socket-functions#sendto) and [recvfrom](../wiseconnect-api-reference-guide-sockets/bsd-socket-functions#recvfrom) socket APIs to send and receive data.  
 * 
 * @pre Pre-conditions:
 * - @ref sl_net_up should be called before this API.
 * 
 * @param[in] interface
 *   Interface identified by @ref sl_net_interface_t.
 * 
 * @param[in] ip_address
 *   Multicast IP address of type [sl_ip_address_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-ip-address-t).
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
sl_status_t sl_net_join_multicast_address(sl_net_interface_t interface, const sl_ip_address_t *ip_address);

/***************************************************************************/ /**
 * @brief
 *   Disable multicast for the given IP address.
 * 
 * @details
 *   This function disables multicast for the specified IP address on the given interface.
 *   It prevents the interface from receiving or sending multicast packets to the specified IP address.
 *
 * @note
 *   IGMPv1 and IGMPv2 are supported.
 * 
 * @pre Pre-conditions:
 * - @ref sl_net_up should be called before this API.
 * 
 * @param[in] interface
 *   Interface identified by @ref sl_net_interface_t.
 * 
 * @param[in] ip_address
 *   Multicast IP address of type [sl_ip_address_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-ip-address-t).
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
sl_status_t sl_net_leave_multicast_address(sl_net_interface_t interface, const sl_ip_address_t *ip_address);

/** @} */

// Helper functions
/**
 * @brief Initializes the Wi-Fi client interface.
 * 
 * This function initializes the Wi-Fi client with the specified configuration and event handler.
 * 
 * @param interface The network interface to initialize.
 * @param configuration Pointer to the configuration parameters.
 * @param context User-defined context passed to the event handler.
 * @param event_handler Callback function for network events.
 * @return sl_status_t Status of the operation.
 */
sl_status_t sl_net_wifi_client_init(sl_net_interface_t interface,
                                    const void *configuration,
                                    void *context,
                                    sl_net_event_handler_t event_handler);

/**
 * @brief Deinitializes the Wi-Fi client interface.
 * 
 * This function deinitializes the Wi-Fi client, freeing any resources allocated during initialization.
 * 
 * @param interface The network interface to deinitialize.
 * @return sl_status_t Status of the operation.
 */
sl_status_t sl_net_wifi_client_deinit(sl_net_interface_t interface);

/**
 * @brief Brings up the Wi-Fi client interface.
 * 
 * This function activates the Wi-Fi client interface using the specified profile.
 * 
 * @param interface The network interface to activate.
 * @param profile_id The profile ID to use for connection.
 * @return sl_status_t Status of the operation.
 */
sl_status_t sl_net_wifi_client_up(sl_net_interface_t interface, sl_net_profile_id_t profile_id);

/**
 * @brief Brings down the Wi-Fi client interface.
 * 
 * This function deactivates the Wi-Fi client interface.
 * 
 * @param interface The network interface to deactivate.
 * @return sl_status_t Status of the operation.
 */
sl_status_t sl_net_wifi_client_down(sl_net_interface_t interface);

/**
 * @brief Initializes the Wi-Fi AP (Access Point) interface.
 * 
 * This function initializes the Wi-Fi AP with the specified configuration and event handler.
 * 
 * @param interface The network interface to initialize.
 * @param configuration Pointer to the configuration parameters.
 * @param context User-defined context passed to the event handler.
 * @param event_handler Callback function for network events.
 * @return sl_status_t Status of the operation.
 */
sl_status_t sl_net_wifi_ap_init(sl_net_interface_t interface,
                                const void *configuration,
                                const void *context,
                                sl_net_event_handler_t event_handler);

/**
 * @brief Deinitializes the Wi-Fi AP interface.
 * 
 * This function deinitializes the Wi-Fi AP, freeing any resources allocated during initialization.
 * 
 * @param interface The network interface to deinitialize.
 * @return sl_status_t Status of the operation.
 */
sl_status_t sl_net_wifi_ap_deinit(sl_net_interface_t interface);

/**
 * @brief Brings up the Wi-Fi AP interface.
 * 
 * This function activates the Wi-Fi AP interface using the specified profile.
 * 
 * @param interface The network interface to activate.
 * @param profile_id The profile ID to use for the AP.
 * @return sl_status_t Status of the operation.
 */
sl_status_t sl_net_wifi_ap_up(sl_net_interface_t interface, sl_net_profile_id_t profile_id);

/**
 * @brief Brings down the Wi-Fi AP interface.
 * 
 * This function deactivates the Wi-Fi AP interface.
 * 
 * @param interface The network interface to deactivate.
 * @return sl_status_t Status of the operation.
 */
sl_status_t sl_net_wifi_ap_down(sl_net_interface_t interface);
