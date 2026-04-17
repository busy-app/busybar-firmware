/***************************************************************************//**
 * @file sl_wisun_api.h
 * @brief Wi-SUN API
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_WISUN_API_H
#define SL_WISUN_API_H

#include "sl_wisun_types.h"
#include "sl_wisun_events.h"
#include "sl_status.h"
#include "sl_wisun_connection_params_api.h"
#include "sl_wisun_lfn_params_api.h"
#include "sl_wisun_regulation_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * @addtogroup SL_WISUN_API Wi-SUN Stack API
 *
 * Wi-SUN Stack API provides functions for managing and configuring the Wi-SUN
 * protocol stack.
 *
 * The API allows applications to initiate and manage network connections,
 * set security credentials, configure PHY and MAC parameters, and retrieve
 * network statistics and information. It's based on requests from the
 * application to the stack and events from the stack to the application.
 * Requests are made using function calls, where a function call either
 * performs the required action immediately or initiates an internal operation
 * within the stack, which terminates with an event. All events contain
 * a status code, indicating the result of the requested operation. Events
 * are also used by the stack to notify the application of any important
 * information, such as the state of the connection.
 *
 * The application is expected to override sl_wisun_on_event() to handle events
 * from the stack. Because all events share a common header, the function may be
 * implemented as a switch statement. The event-specific data can be accessed
 * through the #sl_wisun_evt_t::evt union.
 *
 * @code
 *     void sl_wisun_on_event(sl_wisun_evt_t *evt)
 *     {
 *       switch (evt->header.id) {
 *         case SL_WISUN_MSG_CONNECTED_IND_ID:
 *           handle_connected_event(evt->evt.connected);
 *           break;
 *         default:
 *           break;
 *       }
 *     }
 * @endcode
 *
 * The API is thread-safe, which means can be called from multiple RTOS tasks. The
 * stack guarantees that only a single request is executed at a time and that requests
 * are handled in the order they were made. Event callback is executed in a
 * different context than the request, so the API functions may be called from
 * the event callback.
 *
 * @{
 *****************************************************************************/

/**
 * @name Callbacks
 * @{
 */

/**************************************************************************//**
 * Callback handler for a single event.
 *
 * @param[out] evt The event to be handled
 *
 * This function is called when the stack sends an event to the application.
 * The application can declare its own version this function to customize
 * event handling. The default implementation discards all events.
 *
 * @see #SL_WISUN_EVT
 *****************************************************************************/
void sl_wisun_on_event(sl_wisun_evt_t *evt);

/**
 * @}
 */

/**************************************************************************//**
 * Initiate a connection to a Wi-SUN network.
 *
 * @param[in] name Name of the Wi-SUN network as a zero-terminated string
 * @param[in] phy_config Pointer to PHY configuration
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function initiates connection to a Wi-SUN network. Completion of the
 * request is indicated with a #SL_WISUN_MSG_CONNECTED_IND_ID event.
 *
 * Available in libraries: Full, FFN, LFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_join(const uint8_t *name, sl_wisun_phy_config_t *phy_config);

/**************************************************************************//**
 * Read an IP address.
 *
 * @param[in] address_type Type of the IP address to read
 *   - #SL_WISUN_IP_ADDRESS_TYPE_LINK_LOCAL: Link-local IPv6 address of the device
 *   - #SL_WISUN_IP_ADDRESS_TYPE_GLOBAL: Global unicast IPv6 address of the device
 *   - #SL_WISUN_IP_ADDRESS_TYPE_BORDER_ROUTER: Global unicast IPv6 address of the border router
 *   - #SL_WISUN_IP_ADDRESS_TYPE_PRIMARY_PARENT: Link-local IPv6 address of the primary parent
 *   - #SL_WISUN_IP_ADDRESS_TYPE_SECONDARY_PARENT: Link-local IPv6 address of the secondary parent
 * @param[out] address IP address to read
 * @return SL_STATUS_OK if successful, an error code otherwise.
 *
 * Available in libraries: Full, FFN, LFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_get_ip_address(sl_wisun_ip_address_type_t address_type,
                                    in6_addr_t *address);

/**************************************************************************//**
 * Disconnect from the Wi-SUN network.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function disconnects an active connection or cancels an ongoing
 * connection attempt.
 *
 * Available in libraries: Full, FFN, LFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_disconnect();

/**************************************************************************//**
 * Set a trusted CA certificate used to validate the authentication server
 * certificate.
 *
 * @param[in] certificate_options Options for the certificate
 *   - #SL_WISUN_CERTIFICATE_OPTION_APPEND: Append the certificate to the list of trusted certificates
 *                                          instead of replacing the previous entries
 *   - #SL_WISUN_CERTIFICATE_OPTION_IS_REF: The application guarantees the certificate data will remain
 *                                          in scope and can therefore be referenced instead of copied
 * @param[in] certificate_length Size of the certificate data
 * @param[in] certificate Pointer to the certificate data
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets a trusted CA certificate for validiting the
 * authentication server certificate during the authentication process.
 * The supported certificate formats depend on application's Mbed TLS
 * configuration. The default configuration supports PEM and DER formats.
 * When PEM is used, the certificate data may contain multiple concatenated
 * certificates.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_trusted_certificate(uint16_t certificate_options,
                                             uint16_t certificate_length,
                                             const uint8_t *certificate);

/**************************************************************************//**
 * Set the device certificate used to authenticate to the authentication server.
 *
 * @param[in] certificate_options Options for the certificate.
 *   - #SL_WISUN_CERTIFICATE_OPTION_APPEND: Append the certificate to the list of device certificates
 *                                          instead of replacing the previous entries
 *   - #SL_WISUN_CERTIFICATE_OPTION_IS_REF: The application guarantees the certificate data will remain
 *                                          in scope and can therefore be referenced instead of copied
 *   - #SL_WISUN_CERTIFICATE_OPTION_HAS_KEY: The certificate has a corresponding private key
 * @param[in] certificate_length Size of the certificate data
 * @param[in] certificate Pointer to the certificate data
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the device certificate used during the authentication
 * process. The corresponding device private key must be set using
 * sl_wisun_set_device_private_key() or sl_wisun_set_device_private_key_id().
 * The supported certificate formats depend on application's Mbed TLS
 * configuration. The default configuration supports PEM and DER formats.
 *
 * Available in libraries: Full, FFN, LFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_device_certificate(uint16_t certificate_options,
                                            uint16_t certificate_length,
                                            const uint8_t *certificate);

/**************************************************************************//**
 * Set the device private key used to authenticate to the authentication server.
 *
 * @param[in] key_options Options for the private key
 *   - #SL_WISUN_PRIVATE_KEY_OPTION_IS_REF: The application guarantees the private key data will remain
 *                                          in scope and can therefore be referenced instead of copied
 * @param[in] key_length Size of the private key data
 * @param[in] key Pointer to the private key data
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the device private key used during the authentication
 * process. The corresponding device certificate must be set using
 * sl_wisun_set_device_certificate(). The supported key formats depend on
 * application's Mbed TLS configuration. The default configuration supports
 * unencrypted PKCS#8 and SEC1 keys in PEM and DER formats.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_device_private_key(uint16_t key_options,
                                            uint16_t key_length,
                                            const uint8_t *key);

/**************************************************************************//**
 * Read a set of statistics.
 *
 * @param[in] statistics_type Type of statistics to read
 *   - #SL_WISUN_STATISTICS_TYPE_PHY: PHY/RF statistics
 *   - #SL_WISUN_STATISTICS_TYPE_MAC: MAC statistics
 *   - #SL_WISUN_STATISTICS_TYPE_FHSS: Frequency hopping statistics
 *   - #SL_WISUN_STATISTICS_TYPE_WISUN: Wi-SUN statistics
 *   - #SL_WISUN_STATISTICS_TYPE_NETWORK: 6LoWPAN/IP stack statistics
 *   - #SL_WISUN_STATISTICS_TYPE_REGULATION: Regional regulation statistics
 *   - #SL_WISUN_STATISTICS_TYPE_HEAP: Heap usage statistics
 * @param[out] statistics Set of statistics read
 * @return SL_STATUS_OK if successful, an error code otherwise.
 *
 * This function reads a set of statistics from the stack. Statistics are
 * cumulative and reset when a connection is initiated or by calling
 * sl_wisun_reset_statistics().
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_get_statistics(sl_wisun_statistics_type_t statistics_type,
                                    sl_wisun_statistics_t *statistics);

/**************************************************************************//**
 * Set the maximum TX power.
 *
 * @param[in] tx_power TX power in dBm
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the maximum TX power. The device may use
 * a lower value based on internal decision making or hardware limitations but
 * will never exceed the given value.
 * The function must be called before initiating a connection.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 * @deprecated This function will be removed in the future versions of the
 *             Wi-SUN stack, use sl_wisun_set_tx_power_ddbm() instead.
 *****************************************************************************/
sl_status_t sl_wisun_set_tx_power(int8_t tx_power) SL_DEPRECATED_API_SDK_2024_6;

/**************************************************************************//**
 * Restrict channel use to a subset of allowed channels in the current channel plan.
 *
 * @param[in] channel_mask Mask of operating channels
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets a mask of channels the device is allowed to operate in
 * for unicast frequency hopping. By default, all channels in the channel plan
 * are allowed. The mask can only be used to further restrict the channels.
 * Channels outside the channel plan or channels internally excluded are
 * ignored. This mask will be used in the following connections.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *
 * @warning By comparison to the Wi-SUN FAN specification, the channel mask
 *          logic is inverted. The specification references a mask of excluded
 *          channels.
 *****************************************************************************/
sl_status_t sl_wisun_set_allowed_channel_mask(const sl_wisun_channel_mask_t *channel_mask);

/**************************************************************************//**
 * Restrict channel use to a subset of allowed channels in the current channel plan.
*
 * @param[in] channel_mask Mask of operating channels
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets a mask of channels the device is allowed to operate in
 * for unicast frequency hopping. By default, all channels in the channel plan
 * are allowed. The mask can only be used to further restrict the channels.
 * Channels outside the channel plan or channels internally excluded are
 * ignored. This mask will be used in the following connections.
 *
 * @deprecated This function is an alias for sl_wisun_set_allowed_channel_mask().
 *             It is maintained for backward compatibility.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_channel_mask(const sl_wisun_channel_mask_t *channel_mask);

/**************************************************************************//**
 * Add a MAC address to the list of allowed addresses.
 *
 * @param[in] address MAC address
 *   - **sl_wisun_broadcast_mac**: allow all MAC addresses
 *   - **unicast address**: allow the given MAC address
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function adds a MAC address to the list of allowed addresses. When the
 * first address is added to the list, the list of denied addresses is cleared
 * and the device will start preventing communication with any device whose MAC
 * address does not match any of addresses on the list. By default, all MAC
 * addresses are allowed. Up to 10 MAC addresses may be added to the list.
 * The access list affects only directly connected nodes such as parents,
 * children, and neighbors. The list is persistent when the device disconnects
 * or stops.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_allow_mac_address(const sl_wisun_mac_address_t *address);

/**************************************************************************//**
 * Add a MAC address to the list of denied addresses.
 *
 * @param[in] address MAC address
 *   - **sl_wisun_broadcast_mac**: deny all MAC addresses
 *   - **unicast address**: deny the given MAC address
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function adds a MAC address to the list of denied addresses. When the
 * first address is added to the list, the list of allowed addresses is cleared
 * and the device will start preventing communication with any device whose MAC
 * address matches any of the addresses on the list. By default, all MAC
 * addresses are allowed. Up to 10 MAC addresses may be added to the list.
 * The access list affects only directly connected nodes such as parents,
 * children, and neighbors. The list is persistent when the device disconnects
 * os stops.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_deny_mac_address(const sl_wisun_mac_address_t *address);

/**************************************************************************//**
 * Get the current join state.
 *
 * @param[out] join_state Join state
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function retrieves the current state of the connection process. The
 * function can only be used once a connection has been initiated.
 *
 * Available in libraries: Full, FFN, LFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_get_join_state(sl_wisun_join_state_t *join_state);

/**************************************************************************//**
 * Clear the credential cache.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function clears the cached authentication credentials stored in
 * non-volatile storage. The function is intended for test purposes. Note that
 * clearing the credential cache may prevent the node from reconnecting to the
 * same parent until the corresponding cache entry has expired on the parent.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_clear_credential_cache();

/**************************************************************************//**
 * Get the current device MAC address in use.
 *
 * @param[out] address MAC address
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_get_mac_address(sl_wisun_mac_address_t *address);

/**************************************************************************//**
 * Set the device MAC address to be used.
 *
 * @param[in] address MAC address
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the MAC address for use in the following connections.
 * By default, the device will use the built-in unique device MAC address.
 * The address is reset to the built-in value upon power up.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_mac_address(const sl_wisun_mac_address_t *address);

/**************************************************************************//**
 * Reset a set of statistics in the stack.
 *
 * @param[in] statistics_type Type of statistics to reset
 *   - #SL_WISUN_STATISTICS_TYPE_PHY: PHY/RF statistics
 *   - #SL_WISUN_STATISTICS_TYPE_MAC: MAC statistics
 *   - #SL_WISUN_STATISTICS_TYPE_FHSS: Frequency hopping statistics
 *   - #SL_WISUN_STATISTICS_TYPE_WISUN: Wi-SUN statistics
 *   - #SL_WISUN_STATISTICS_TYPE_NETWORK: 6LoWPAN/IP stack statistics
 * @return SL_STATUS_OK if successful, an error code otherwise.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_reset_statistics(sl_wisun_statistics_type_t statistics_type);

/**************************************************************************//**
 * Get the number of RPL neighbors (parents and children).
 *
 * @param[out] neighbor_count Number of neighbors
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_get_neighbor_count(uint8_t *neighbor_count);

/**************************************************************************//**
 * Get a list of RPL neighbor (parents and children) MAC addresses.
 *
 * @param[in,out] neighbor_count Maximum number of neighbors to read on input,
 *                               number of neighbors read on output
 * @param[out] neighbor_mac_addresses Pointer to memory where to store neighbor
 *                                    MAC addresses
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_get_neighbors(uint8_t *neighbor_count,
                                   sl_wisun_mac_address_t *neighbor_mac_addresses);

/**************************************************************************//**
 * Get information about a RPL neighbor (parent or child).
 *
 * @param[in] neighbor_mac_address Pointer to neighbor MAC address
 * @param[out] neighbor_info Pointer to where the read information is stored
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_get_neighbor_info(const sl_wisun_mac_address_t *neighbor_mac_address,
                                       sl_wisun_neighbor_info_t *neighbor_info);

/**************************************************************************//**
 * Set the unicast dwell interval (ms) used for channel hopping.
 *
 * @param[in] dwell_interval_ms Unicast Dwell Interval (15-255 ms)
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the parameters for unicast channel hopping to be used in
 * the following connections. The Unicast Dwell Interval specifies the duration
 * which the node will listen to a channel within its listening schedule.
 * The default value is 255 ms.
 *
 * Available in libraries: Full, FFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_unicast_settings(uint8_t dwell_interval_ms);

/**************************************************************************//**
 * Set the device private key used to authenticate to the authentication server.
 *
 * @param[in] key_id Key ID of the private key
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the device private key using a key identifier,
 * referencing a key stored in PSA cryptography module. The corresponding
 * device certificate must be set using sl_wisun_set_device_certificate().
 * The stored key must have correct PSA key attributes, see the
 * Wi-SUN FAN Security Concepts and Design Considerations document for
 * details.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_device_private_key_id(uint32_t key_id);

/**************************************************************************//**
 * Set the regional regulation.
 *
 * @param[in] regulation Regional regulation
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function applies the appropriate parameter set from
 * @ref SL_WISUN_REGULATION_PARAMETER_SETS to be used for upcoming connections.
 * The selected regional regulation will impact both the Wi-SUN
 * stack performance and its behavior. See regulation standards for details.
 * No regulation is set by default.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *
 * @note This function uses a copy of the regulation parameter sets, modifying
 *       a predefined set in sl_wisun_regulation_api.h will not affect the
 *       regulation set used by this function.
 *       To change the regulation parameters, use sl_wisun_set_regulation_parameters().
 * @deprecated This function will be removed in the future versions of the
 *             Wi-SUN stack, use sl_wisun_set_regulation_parameters() instead.
 *****************************************************************************/
sl_status_t sl_wisun_set_regulation(sl_wisun_regulation_t regulation) SL_DEPRECATED_API_SDK_2025_12;

/**************************************************************************//**
 * Configure the regional regulation parameter set.
 *
 * @param[in] params Parameter set to use
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_regulation_parameters(const sl_wisun_regulation_params_t *params);

/**************************************************************************//**
 * Reset all the past hour transmissions counters.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_reset_regulation_duty_cycle(void);

/**************************************************************************//**
 * Configure neighbor table size.
 *
 * @param[in] max_child_count Maximum number of RPL children
 * Increasing this parameter means a higher number of potential neighbors
 * while lowering it means reduced RAM consumption.
 * The default value is 22.
 * @param[in] max_neighbor_count Maximum number of neighbors including children, parent, and temporary neighbors
 * The default value is 32
 * @param[in] max_security_neighbor_count Maximum number of neigbors in the security table. Entries in the security
 * table are removed on key expirations.
 * Default value is 300
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * max_neighbor_count must be greater than max_child_count. max_neighbor_count - max_child_count represents
 * the available neighbors for RPL parents and temporary neighbors (neighbors not yet registered, parent candidate
 * or neighbors sending multicast). max_security_neighbor_count must be greater or equal than max_neighbor_count
 * Each entry in the neighbor table consumes about 450 bytes of RAM.
 * Each entry in the security neighbor table consumes about 50 bytes of RAM.
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_config_neighbor_table(uint8_t max_child_count, uint8_t max_neighbor_count, uint16_t max_security_neighbor_count);

/**************************************************************************//**
 * Set neighbor table size.
 *
 * @param[in] size Size of the neighbor table
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the limit of regular neighbors supported by the
 * node [1, 245], without considering temporary or RPL parents.
 * Increasing this parameter means a higher number of potential neighbors
 * while lowering it means reduced RAM consumption.
 * The default value is 22.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 * @deprecated This function will be removed in the future versions of the
 *             Wi-SUN stack, use sl_wisun_config_neighbor_table() instead.
 *****************************************************************************/
sl_status_t sl_wisun_set_neighbor_table_size(uint8_t size) SL_DEPRECATED_API_SDK_2025_6;

/**************************************************************************//**
 * Set the thresholds for transmission duration level event.
 *
 * @param[in] warning_threshold Warning threshold in percent or -1 to disable
 * @param[in] alert_threshold Alert threshold in percent or -1 to disable
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the thresholds for transmission duration level event.
 * When set and when a regional regulation is enabled using
 * sl_wisun_set_regulation_parameters(), a
 * #SL_WISUN_MSG_REGULATION_TX_LEVEL_IND_ID event is sent when one of the
 * configured thresholds is exceeded. This can be used by the application to
 * prevent exceeding the total transmission duration allowed in the regional
 * regulation. Thresholds are defined as a percentage of the maximum
 * transmission duration permitted by the regional regulation.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_regulation_tx_thresholds(int8_t warning_threshold,
                                                  int8_t alert_threshold);

/**************************************************************************//**
 * Set the async transmission fragmentation parameters.
 *
 * @param[in] fragment_duration_ms Max duration of a fragment in ms (min 500 ms)
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Async transmissions, such as Wi-SUN PAN advertisement packets, are sent
 * to every allowed operating channel and may therefore block broadcast and
 * unicast traffic. This impact can be reduced by splitting the channel list
 * into fragments based on the maximum transmission duration and by forcing a
 * delay between the fragments, allowing other traffic to occur. This function
 * sets the maximum duration of a PA, PAS, PC, and PCS advertisement period
 * fragments. A small value trades off longer connection times for shorter
 * latencies. Setting the duration to #SL_WISUN_ADVERT_FRAGMENT_DISABLE disables
 * async transmission fragmentation.
 *
 * By default, the maximum fragment duration is set to 500 ms.
 *
 * Available in libraries: Full, FFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_advert_fragment_duration(uint32_t fragment_duration_ms);

/**************************************************************************//**
 * Set the device type (FFN/LFN/BR).
 *
 * @param[in] device_type Type of the device
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the operational mode of the node.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *
 * @note This is a no-op with BR, FFN and LFN libraries.
 *****************************************************************************/
sl_status_t sl_wisun_set_device_type(sl_wisun_device_type_t device_type);

/**************************************************************************//**
 * Configure per‑neighbor mode switch behavior.
 *
 * @param[in] mode Mode switch configuration of the neighbor. If set to
 *                 #SL_WISUN_MODE_SWITCH_DEFAULT, the configuration of
 *                 the neighbor is reset back to the default mode switch
 *                 behavior.
 * @param[in] phy_mode_id PhyModeId to use when mode is set to
 *                        #SL_WISUN_MODE_SWITCH_ENABLED, ignored otherwise.
 * @param[in] neighbor_address MAC address of the neighbor to configure.
 *                             If set to #sl_wisun_broadcast_mac, configures
 *                             the default mode switch behavior for all
 *                             non-configured neighbors.
 * @param[in] reserved Reserved for future use, set to false.
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Available in libraries: Full, FFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_config_mode_switch(uint8_t mode,
                                        uint8_t phy_mode_id,
                                        const sl_wisun_mac_address_t *neighbor_address,
                                        bool reserved);

/**************************************************************************//**
 * Set the PHY mode switch configuration.
 *
 * @param[in] mode Mode switch configuration of the neighbor. If set to
 *                 #SL_WISUN_MODE_SWITCH_DEFAULT, the configuration of
 *                 the neighbor is reset back to the default mode switch
 *                 behavior.
 * @param[in] phy_mode_id PhyModeId to use when mode is set to
 *                        #SL_WISUN_MODE_SWITCH_ENABLED, ignored otherwise.
 * @param[in] neighbor_address MAC address of the neighbor to configure.
 *                             If set to #sl_wisun_broadcast_mac, configures
 *                             the default mode switch behavior for all
 *                             non-configured neighbors.
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Available in libraries: Full, FFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_mode_switch(uint8_t mode,
                                     uint8_t phy_mode_id,
                                     const sl_wisun_mac_address_t *neighbor_address);

/**************************************************************************//**
 * Configure the FFN parameter set.
 *
 * @param[in] params Parameter set to use
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the FFN parameter set. These parameters impact
 * connection time, bandwidth usage, and latency. Use of a predefined
 * parameter set is recommended (@ref SL_WISUN_FFN_PARAMETER_SETS).
 * Small profile will be used by default for all missing configurations.
 * The function must be called before initiating a connection.
 *
 * Available in libraries: Full, FFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_connection_parameters(const sl_wisun_connection_params_t *params);

/**************************************************************************//**
 * Configure the list of PHY operating modes the device will use for mode switch operations.
 *
 * @param[in] phy_mode_id_count Number of PhyModeId to configure. If set to 0,
 *                              it removes POM-IE and disables mode switch feature.
 *
 * @param[in] phy_mode_ids List of phy_mode_id_count PhyModeId. It must not
 *                         contain the base operating mode.
 * @param[in] is_mdr_command_capable Indicate if the device supports MAC mode switch.
 *                                   Feature currently unsupported, must be set to 0.
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the PHY operating mode information advertised to neighboring
 * nodes. By default the PhyModeId list contains the first fifteen PhyModeId listed
 * in radio multi-PHY configuration, MAC mode switch is disabled.
 *
 * Available in libraries: Full, FFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_pom_ie(uint8_t phy_mode_id_count,
                                uint8_t phy_mode_ids[SL_WISUN_MAX_PHY_MODE_ID_COUNT],
                                uint8_t is_mdr_command_capable);

/**************************************************************************//**
 * Get the list of PHY operating modes the device will use for mode switch operations.
 *
 * @param[out] phy_mode_id_count Number of PhyModeId retrieved
 * @param[out] phy_mode_ids List of phy_mode_id_count PhyModeId. Caller must allocate
 *                          space for at least #SL_WISUN_MAX_PHY_MODE_ID_COUNT entries.
 * @param[out] is_mdr_command_capable Set to 1 if the device supports MAC mode switch,
 *                                    0 otherwise
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function retrieves the PHY operating mode information advertised
 * to neighboring nodes.
 *
 * Available in libraries: Full, FFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_get_pom_ie(uint8_t *phy_mode_id_count,
                                uint8_t *phy_mode_ids,
                                uint8_t *is_mdr_command_capable);

/**************************************************************************//**
 * Get the Wi-SUN stack version.
 *
 * @param[out] major Wi-SUN stack version major
 * @param[out] minor Wi-SUN stack version minor
 * @param[out] patch Wi-SUN stack version patch
 * @param[out] build Build number, set to 0 in public versions
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_get_stack_version(uint8_t *major,
                                       uint8_t *minor,
                                       uint8_t *patch,
                                       uint16_t *build);

/**************************************************************************//**
 * Configure the LFN parameter set.
 *
 * @param[in] params Parameter set to use
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the LFN parameter set. These parameters impact
 * connection time, bandwidth usage, power consumption, and latency.
 * Use of a predefined parameter set is recommended
 * (@ref SL_WISUN_LFN_PARAMETER_SETS).
 * Test profile will be used by default for all missing configurations.
 * The function must be called before initiating a connection.
 *
 * Available in libraries: Full, LFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_lfn_parameters(const sl_wisun_lfn_params_t *params);

/**************************************************************************//**
 * Configure LFN parenting support.
 *
 * @param[in] lfn_limit Maximum number of LFN children
 *   - **0**: LFN parenting is disabled in the node
 *   - **> 0**: Maximum number of LFN children the node can parent
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the maximum number of LFN children this node can
 * parent. Set @ref sl_wisun_config_neighbor_table accordingly.
 *
 * Available in libraries: Full, FFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_lfn_support(uint8_t lfn_limit);

/**************************************************************************//**
 * Enable or disable the Packet Trace Interface (PTI).
 *
 * @param[in] pti_state PTI state
 *   - **true**: PTI is enabled
 *   - **false**: PTI is disabled
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets Packet Trace Interface (PTI) state. PTI is enabled
 * by default.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_pti_state(bool pti_state);

/**************************************************************************//**
 * Trigger a transmission of a periodic frame (FAN Discovery, RPL).
 *
 * @param[in] frame_type Type of frame to transmit
 *   - #SL_WISUN_FRAME_TYPE_PAS: Transmit a PAN Advertisement Solicit frame
 *   - #SL_WISUN_FRAME_TYPE_PA: Transmit a PAN Advertisement frame
 *   - #SL_WISUN_FRAME_TYPE_PCS: Transmit a PAN Configuration Solicit frame
 *   - #SL_WISUN_FRAME_TYPE_PC: Transmit a PAN Configuration frame
 *   - #SL_WISUN_FRAME_TYPE_DIS: Transmit a DODAG Information Solicitation frame
 *   - #SL_WISUN_FRAME_TYPE_DIO: Transmit a DODAG Information Object frame
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function causes a periodic frame (FAN Discovery, RPL) to be transmitted
 * immediately. The frame can only be transmitted in the appropriate connection
 * state, i.e. the associated trickle timer must be running. The function is
 * intended for test purposes.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_trigger_frame(sl_wisun_frame_type_t frame_type);

/**************************************************************************//**
 * Enable or disable authentication and link encryption.
 *
 * @param[in] security_state Connection security state
 *   - **0**: Authentication and link encryption are disabled
 *   - **1**: Authentication and link encryption are enabled
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the connection security state. By default, authentication
 * and link encryption are enabled. The function is intended for test purposes.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *
 * @note Authentication and link encryption can only be disabled when the
 *       application is linked against a library supporting this option.
 *****************************************************************************/
sl_status_t sl_wisun_set_security_state(uint32_t security_state);

/**************************************************************************//**
 * Get Wi-SUN network information.
 *
 * @param[out] network_info Pointer where network information is written
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_get_network_info(sl_wisun_network_info_t *network_info);

/**************************************************************************//**
 * Get RPL information.
 *
 * @param[out] rpl_info Pointer where RPL information is written
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Available in libraries: Full, FFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_get_rpl_info(sl_wisun_rpl_info_t *rpl_info);

/**************************************************************************//**
 * Get the mask of channels excluded from channel plan
 *
 * @param[in] type Type of channel mask to retrieve
 * @param[out] channel_mask Pointer where channel mask is written
 * @param[out] channel_count Number of channels in the mask
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_get_excluded_channel_mask(sl_wisun_channel_mask_type_t type,
                                               sl_wisun_channel_mask_t *channel_mask,
                                               uint32_t *channel_count);

/**************************************************************************//**
 * Set the maximum TX power.
 *
 * @param[in] tx_power_ddbm TX power in deci-dBm
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the maximum TX power. The device may use
 * a lower value based on internal decision making or hardware limitations but
 * will never exceed the given value.
 * The function must be called before initiating a connection.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *
 * @note When replacing sl_wisun_set_tx_power() function calls with this
 *       function, note the change from dBm units to deci-dBm units.
 *****************************************************************************/
sl_status_t sl_wisun_set_tx_power_ddbm(int16_t tx_power_ddbm);

/**************************************************************************//**
 * Allow or prevent the node from parenting other nodes.
 *
 * @param[in] is_leaf RPL leaf mode
 *   - **true**: RPL leaf mode is enabled
 *   - **false**: RPL leaf mode is disabled
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function configures RPL leaf mode on FFNs. When enabled, it prevents
 * the FFN from parenting other nodes (FFN or LFN).
 *
 * Available in libraries: Full, FFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_leaf(bool is_leaf);

/**************************************************************************//**
 * Enable/disable Direct Connect.
 *
 * @param[in] is_enabled Direct Connect status
 *   - **true**: Direct Connect is enabled
 *   - **false**: Direct Connect is disabled
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Available in libraries: Full, FFN (see @ref API_AVAILABILITY)
 *
 * @note Enabling this feature may increase connection time.
 *****************************************************************************/
sl_status_t sl_wisun_set_direct_connect_state(bool is_enabled);

/**************************************************************************//**
 * Accept a Direct Connect link.
 *
 * @param[in] link_local_ipv6 Link-Local IPv6 address of the client
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Available in libraries: Full, FFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_accept_direct_connect_link(in6_addr_t *link_local_ipv6);

/**************************************************************************//**
 * Set the radio sensitivity for the given PHY.
 *
 * @param[in] phy_mode_id PHY mode ID
 * @param[in] sensitivity_dbm Sensitivity in dBm
 * @return SL_STATUS_OK if successful,
 *         SL_STATUS_INVALID_PARAMETER if the PHY mode ID is invalid,
 *         SL_STATUS_FULL if you a have already set SL_WISUN_MAX_PHY_MODE_ID_COUNT different entries
 *
 * This function sets the radio sensitivity for RX state. It allows to fine tune it
 * according to the user's hardware and needs. The stack possesses a set of default
 * values for each PHY mode, but the user can override them with this function.
 * User's modifications generated by this API are not saved in the NVM.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *
 * @warning Changing the RX sensitivity can have a significant impact on the network's
 *          behavior and performance and should be done with caution.
 *          The stack follows the Wi-SUN specification, and as such, takes a margin around
 *          the given sensitivity (hysteresis).
 *          It is strongly advised to only use this API when using a Low-noise amplifier
 *          with an FEM.
 *****************************************************************************/
sl_status_t sl_wisun_set_phy_sensitivity(uint8_t phy_mode_id, int16_t sensitivity_dbm);

/**************************************************************************//**
 * Set the Pairwise Master Key (PMK) to use for Direct Connect.
 *
 * @param[in] pmk_key_id The Pairwise Master Key (PMK) ID to use.
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the Direct Connect PMK using a key identifier,
 * referencing a key stored in PSA cryptography module.
 * The stored key must have correct PSA key attributes, see the
 * Wi-SUN FAN Security Concepts and Design Considerations document for
 * details.
 *
 * Available in libraries: Full, FFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_direct_connect_pmk(const uint32_t pmk_key_id);

/**************************************************************************//**
 * Set the preferred PAN for this device.
 *
 * @param[in] pan_id Preferred PAN ID. Set to 0xFFFF to disable.
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets a preferred PAN ID that the device will always select
 * during connection if available, or seek to roam to while connected to
 * another PAN.
 *
 * @note Available in libraries: Full, FFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_preferred_pan(uint16_t pan_id);

/**************************************************************************//**
 * Configure concurrent detection.
 *
 * @param[in] enable_tx TX state
 *   - **true**: TX is allowed with an alternate PHY
 *   - **false**: TX is not allowed with an alternate PHY
 * @param[in] reserved Reserved for future use, set to zero.
 * @return SL_STATUS_OK if the operation is successful, an error code otherwise.
 *
 * This function configures whether an alternate PHY can be used concurrently
 * with the base PHY. The alternate PHY must be enabled in the radio
 * configuration to utilize this feature. When enabled, the alternate PHY is
 * always accepted on receive. When TX state is set to true, the node will
 * transmit to nodes that support the feature using the alternate PHY, instead
 * of using the standard mode switch. TX is disabled by default.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_config_concurrent_detection(bool enable_tx, uint8_t reserved);

/**************************************************************************//**
 * Set EAP identity for authentication.
 *
 * @param[in] identity_length Length of EAP identity in bytes
 * @param[in] identity EAP identity to use
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the identity used in the EAP Identity Response during
 * EAP authentication. The function must be called before initiating a
 * connection. No checks are performed on the identity, the caller is
 * responsible for formatting it correctly. In particular, RFC3748 requires
 * that the identity must not be NULL terminated. If not set, the identity
 * defaults to "Anonymous".
 *
 * Available in libraries: Full, FFN, LFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_eap_identity(uint8_t identity_length,
                                      const uint8_t *identity);

/**************************************************************************//**
 * Set filters for event logger.
 *
 * @param[in] address MAC address to filter events
 *   - **unicast address**: set the filter for the given MAC address
 *   - **sl_wisun_broadcast_mac**: set the filter for all MAC addresses
 * @param[in] events Bitmask of accepted events.
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function configures a filter for the event logger based on the specified
 * MAC address and a bitmask of `sl_wisun_logger_event_type_t` that defines the filtered
 * events. The events are delivered through Silicon Labs’ Event System API[1]. To
 * receive them, user must call `sl_event_subscribe` with `SL_EVENT_CLASS_WISUN`.
 * To disable a filter, set the events bitmask to 0.
 * This function can be called at any time.
 *
 * [1]: https://docs.silabs.com/gecko-platform/5.2.1/platform-common/event-system
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_event_filter(const sl_wisun_mac_address_t *address,
                                      uint64_t events);

/**************************************************************************//**
 * Clear filters for event logger.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function clears all event logger filters set by `sl_wisun_set_event_filter`
 * for all event types and addresses.
 * This function can be called at any time.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_clear_event_filters(void);

/**************************************************************************//**
 * Set the FAN TPS Version.
 *
 * @param[in] fan_tps_version FAN TPS Version to set
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the FAN TPS Version advertised in PAN Information Element.
 * - #SL_WISUN_FAN_VERSION_DEFAULT: to keep the stack's default FAN TPS version
 * - #SL_WISUN_FAN_VERSION_1_0: to force the stack to behave like a FAN 1.0 certified node
 * - #SL_WISUN_FAN_VERSION_1_1: to force the stack to behave like a FAN 1.1 certified node
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_fan_tps_version(uint8_t fan_tps_version);

/**************************************************************************//**
 * Set the radio RX FIFO size.
 *
 * @param[in] size Size of the RX FIFO in bytes.
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the size of the RX FIFO used by the radio driver.
 * The value must be a power of two between 64 and 4096 bytes.
 *
 * @note In practice the minimum size for a device to perform authentication
 *       is 1024 bytes.
 *
 * Available in libraries: Full, FFN, LFN, BR (see @ref API_AVAILABILITY)
 *****************************************************************************/
sl_status_t sl_wisun_set_rx_fifo_size(uint16_t size);

/** @} (end SL_WISUN_API) */

#ifdef __cplusplus
}
#endif

#endif  // SL_WISUN_API_H
