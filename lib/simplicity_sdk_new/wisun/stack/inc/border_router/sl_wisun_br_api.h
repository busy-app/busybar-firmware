/***************************************************************************//**
 * @file sl_wisun_br_api.h
 * @brief Wi-SUN Border Router API
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_WISUN_BR_API_H
#define SL_WISUN_BR_API_H

#include "sl_wisun_types.h"
#include "sl_wisun_br_msg_api.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * @addtogroup SL_WISUN_BR_API Border Router API
 * @ingroup SL_WISUN_API
 *
 * Wi-SUN Border Router API contains functions for Border Router specific
 * functionality.
 *
 * The API is an extension, not a replacement of @ref SL_WISUN_API.
 * An application implementing Border Router capability may utilize functions
 * from both APIs. See @ref API_AVAILABILITY for further information.
 *
 * @{
 *****************************************************************************/

/**************************************************************************//**
 * Start the Border Router.
 *
 * @param[in] name Name of the Wi-SUN network as a zero-terminated string
 * @param[in] phy_config Pointer to PHY configuration
 * @return SL_STATUS_OK if successful, an error code otherwise
 *****************************************************************************/
sl_status_t sl_wisun_br_start(const uint8_t *name, const sl_wisun_phy_config_t *phy_config);

/**************************************************************************//**
 * Stop the Border Router.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise
 *****************************************************************************/
sl_status_t sl_wisun_br_stop(void);

/**************************************************************************//**
 * Set Group Transient Key.
 *
 * @param[in] gtk Group Transient Key
 * @param[in] index Index of the key from 0 to 3 for GTKs and 4 to 6 for LGTKs
 * @note This API is provided for test and debug purposes: setting a specific
 *       key in production is not recommended.
 *       The keys set using this API are saved to NVM, and subsequent calls to
 *       this API are ignored until the NVM is cleared using
 *       @ref sl_wisun_clear_credential_cache.
 * @return SL_STATUS_OK if successful, SL_STATUS_ALREADY_EXISTS when
 *         ignored, an error code otherwise
 *****************************************************************************/
sl_status_t sl_wisun_br_set_gtk(const uint8_t *gtk, uint8_t index);

/**************************************************************************//**
 * Set the size of the Wi-SUN network.
 *
 * @param[in] size Size of the network
 *    <br/><b>SL_WISUN_NETWORK_SIZE_AUTOMATIC</b>: network size is managed automatically
 *    <br/><b>SL_WISUN_NETWORK_SIZE_SMALL</b>: less than 100 nodes
 *    <br/><b>SL_WISUN_NETWORK_SIZE_MEDIUM</b>: 100 to 800 nodes
 *    <br/><b>SL_WISUN_NETWORK_SIZE_LARGE</b>: 800 to 1500 nodes
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the size of the network. The size is used to set various
 * stack parameters, such as timing parameters to optimize device behavior
 * in regards to node count. The device will function with any setting but
 * may exhibit non-optimal behavior. Setting the size too large may cause
 * slow connection speed and increased latency. Conversely, a value too
 * small may cause increased network traffic.
 *****************************************************************************/
sl_status_t sl_wisun_br_set_network_size(sl_wisun_network_size_t size);

/**************************************************************************//**
 * Get the unicast IPv6 addresses configured in the Border Router.
 *
 * @param[out] addr_ll Pointer to Link-Local address
 * @param[out] addr_gua Pointer to Unique Local Address or Global Unicast Address
 * @param[out] addr_dodagid Pointer to DODAG ID
 * @return SL_STATUS_OK if successful, an error code otherwise
 *****************************************************************************/
sl_status_t sl_wisun_br_get_ip_addresses(uint8_t *addr_ll, uint8_t *addr_gua, uint8_t *addr_dodagid);

/**************************************************************************//**
 * Get Group AES Key.
 *
 * @param[in] index Index of the key from 0 to 3 for GAKs and 4 to 6 for LFN GAKs
 * @param[out] gak Pointer to Group AES Key
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function exports (L)GAK as a plaintext key.
 *
 * @note This functionality is only available when using the debug library.
 *****************************************************************************/
sl_status_t sl_wisun_br_get_gak(uint8_t index, uint8_t *gak);

/**************************************************************************//**
 * Get the Border Router state.
 *
 * @param[out] state Pointer to state
 *   - #SL_WISUN_BR_STATE_INITIALIZED: ready to start
 *   - #SL_WISUN_BR_STATE_OPERATIONAL: started
 * @return SL_STATUS_OK if successful, an error code otherwise
 *****************************************************************************/
sl_status_t sl_wisun_br_get_state(sl_wisun_br_state_t *state);

/**************************************************************************//**
 * Set the Border Router device certificate.
 *
 * @param[in] certificate_options Options for the certificate.
 *   <br/><b>SL_WISUN_CERTIFICATE_OPTION_APPEND</b>: Append the certificate to the list of server certificates
 *                                                   instead of replacing the previous entries
 *   <br/><b>SL_WISUN_CERTIFICATE_OPTION_IS_REF</b>: The application guarantees the certificate data will remain
 *                                                   in scope and can therefore be referenced instead of copied
 *   <br/><b>SL_WISUN_CERTIFICATE_OPTION_HAS_KEY</b>: The certificate has a private key
 * @param[in] certificate_length Size of the certificate data
 * @param[in] certificate Pointer to the certificate data
 * @return SL_STATUS_OK if successful, an error code otherwise
 *****************************************************************************/
sl_status_t sl_wisun_br_set_device_certificate(uint16_t certificate_options,
                                               uint16_t certificate_length,
                                               const uint8_t *certificate);

/**
 * Define sl_wisun_set_br_device_certificate for legacy.
 */
#define sl_wisun_set_br_device_certificate  sl_wisun_br_set_device_certificate

/**************************************************************************//**
 * Set broadcast settings.
 *
 * @param[in] interval_ms Broadcast Interval (BDI < BI <= 16777215 ms)
 *                        (default: 1020 ms)
 * @param[in] dwell_interval_ms Broadcast Dwell Interval (100-255 ms)
 *                              (default: 255 ms)
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the broadcast channel hopping schedule. The
 * Broadcast Interval is the duration, between Broadcast Dwell Intervals
 * within the advertising node’s broadcast schedule. Its resolution is
 * limited to 24 bits (16777215 ms). The Broadcast Dwell Interval specifies
 * the duration during which the node may transmit a broadcast message.
 * It is recommended a BI value of 4.0 times the maximum of (UDI, BDI).
 * These values affect the whole PAN, since they are propagated from
 * parent to children. This function must be called before starting the
 * Border Router.
 *****************************************************************************/
sl_status_t sl_wisun_br_set_broadcast_settings(uint32_t interval_ms,
                                               uint8_t dwell_interval_ms);

/**************************************************************************//**
 * Configure the Border Router parameter set.
 *
 * @param[in] params Parameter set to use
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This functions sets the Border Router parameter set. These parameters
 * impact connection time, bandwidth usage, and latency. Use of a predefined
 * parameter set is recommended (@ref SL_WISUN_BR_PARAMETER_SETS).
 * Small profile will be used by default for all missing configurations.
 * This function must be called before starting the Border Router.
 *****************************************************************************/
sl_status_t sl_wisun_br_set_connection_parameters(const sl_wisun_br_connection_params_t *params);

/**************************************************************************//**
 * Configure LFN parenting parameter set.
 *
 * @param[in] params Parameter set to use
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the LFN parenting parameter set. These parameters
 * impact battery life of all LFNs in the PAN. Use of a predefined parameter
 * set is recommended (@ref SL_WISUN_BR_LFN_PARAMETER_SETS). This function
 * must be called before starting the Border Router.
 *****************************************************************************/
sl_status_t sl_wisun_br_set_lfn_parameters(const sl_wisun_br_lfn_params_t *params);

/**************************************************************************//**
 * Configure LFN parenting support.
 *
 * @param[in] lfn_limit Maximum number of LFN children
 *   - **0**: LFN parenting is disabled in the Border Router
 *   - **> 0**: Maximum number of LFN children the Border Router can parent
 * @param[in] lfn_support_pan  LFN parenting support in the PAN
 *   - **true**: LFN parenting is enabled in the PAN
 *   - **false**: LFN parenting is disabled in the PAN
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function configures LFN parenting support in the Border Router
 * and in the PAN. Set @ref sl_wisun_config_neighbor_table accordingly.
 *****************************************************************************/
sl_status_t sl_wisun_br_set_lfn_support(uint8_t lfn_limit, bool lfn_support_pan);

/**************************************************************************//**
 * Start or stop PAN Defect advertisement.
 *
 * @param[in] defect_advertise True to start PAN Defect procedure,
 *                             false to stop it.
 * @param[in] min_scan_duration_s Minimum duration after which devices
 *                                are allowed to switch their PAN if an
 *                                alternative one is available.
 * @param[in] max_scan_duration_s Duration after which devices should
 *                                systemically switch to the alternative
 *                                PAN if one is available.
 * @return SL_STATUS_OK if successful, an error code otherwise.
 *
 * @note The values for `min_scan_duration_s` and `max_scan_duration_s` should
 *       be chosen based on the estimated performance of the backup power source.
 *       Specifically, `max_scan_duration_s` should be set to a value smaller than
 *       the estimated lifetime of the backup power source to ensure a seamless
 *       transition to the alternative PAN without long connection interruptions.
 *****************************************************************************/
sl_status_t sl_wisun_br_pan_defect_advertise(bool defect_advertise,
                                             uint32_t min_scan_duration_s,
                                             uint32_t max_scan_duration_s);

/**************************************************************************//**
 * Set IPv6 prefix for the Wi-SUN network.
 *
 * @param[in] ipv6_prefix IPv6 prefix
 * @param[in] prefix_length Prefix length. Currently, only 64 bit prefixes are
 *                          supported.
 * @note  Must not be called after the border router is started
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function sets the prefix used to generate IP addresses for RPL traffic
 * (DODAGID will derive from it). This function must be called before starting
 * the Border Router.
 *****************************************************************************/
sl_status_t sl_wisun_br_set_ipv6_prefix(const uint8_t *ipv6_prefix, uint8_t prefix_length);

/**************************************************************************//**
 * Register a function to handle IPv6 packets received from Wi-SUN network.
 *
 * @param[in] handler Pointer to handler function
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function registers a handler function that will be called when the
 * destination address of a received IPv6 packet is outside the DODAG prefix.
 * Setting the handler function to NULL disables the forwarding. This
 * function is used for handling uplink packets towards the backhaul
 * interface, see sl_wisun_br_ipv6_down() for downlink.
 *****************************************************************************/
sl_status_t sl_wisun_br_set_ipv6_up_handler(sl_wisun_ipv6_up_handler_t handler);

/**************************************************************************//**
 * Forward an IPv6 packet to Wi-SUN network.
 *
 * @param[in] data Pointer to IPv6 header
 * @param[out] data_length Length of IPv6 packet data in bytes
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function forwards an IPv6 packet to Wi-SUN network. The destination
 * address must belong to the DODAG prefix. This function is used for handling
 * downlink packets from the backhaul interface, see
 * sl_wisun_br_set_ipv6_up_handler() for uplink. It's recommended to use
 * sl_wisun_br_ipv6_route_exists() before forwarding a packet.
 *****************************************************************************/
sl_status_t sl_wisun_br_ipv6_down(const uint8_t* data, size_t data_length);

/**************************************************************************//**
 * Check if the IPv6 destination address is routable in the Wi-SUN network.
 *
 * @param[in] ipv6_address Pointer to IPv6 destination address
 * @param[out] exists Whether the address is routable
 *   - **true**: address is routable
 *   - **false**: address is not routable
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function checks if the given IPv6 address is a valid destination
 * in the Wi-SUN network. The destination address must belong to the
 * Border Router or one of the connected nodes.
 *****************************************************************************/
sl_status_t sl_wisun_br_ipv6_route_exists(const uint8_t *ipv6_address, bool *exists);

/**************************************************************************//**
 * Trigger a global repair process from the Border Router.
 *
 * @return SL_STATUS_OK if successful
 *         SL_STATUS_NOT_READY if the Border Router is not started
 *         SL_STATUS_FAIL if the DODAG Version is not incremented
 *
 * Increments the RPL DODAG Version to allow nodes in the network to choose
 * a new position whose rank is not constrained by their rank within
 * the old DODAG Version.
 * The new DODAG Version is propagated through DIOs in the entire network and will
 * cause all nodes to re-run a parent selection within the new DODAG Version,
 * effectively temporarily disconnecting them from the network.
 *
 * @note This operation will disrupt network communications as the
 *       repair process is carried out. It is not recommended to trigger a
 *       global repair more often than once every 24 hours.
 *****************************************************************************/
sl_status_t sl_wisun_br_trigger_global_repair(void);

/**************************************************************************//**
 * Get the Routing Table Size.
 *
 * @param[out] entry_count Number of entries in the routing table
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function retrieves the number of entries in the routing table. This allows
 * for allocation before calling sl_wisun_br_get_routing_table().
 * The allocated memory should be at least equal to:
 * sizeof(sl_wisun_br_routing_table_entry_t) * entry_count.
 *****************************************************************************/
sl_status_t sl_wisun_br_get_routing_table_entry_count(uint16_t *entry_count);

/**************************************************************************//**
 * Get the Routing Table.
 *
 * @param[in,out] entry_count Maximum number of entries to read on input,
 *                              number of entries read on output
 * @param[out] table Pointer to the application allocated memory where
 *                       the routing table is written
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * The routing table is unordered and only contains the parent-child relationship
 * between the nodes. The table is in the format:
 * | Node GUA/ULA 16 bytes   | parent GUA/ULA 16 bytes |
 *
 * @note This function should be called after sl_wisun_br_get_routing_table_entry_count()
 *      to get the number of entries in the routing table.
 *****************************************************************************/
sl_status_t sl_wisun_br_get_routing_table(uint16_t *entry_count, sl_wisun_br_routing_table_entry_t *table);

/**************************************************************************//**
 * Get the Routing Table starting from a specific index.
 *
 * @param[in] start_index Index to start reading the routing table from
 * @param[in,out] entry_count Maximum number of entries to read on input,
 *                              number of entries read on output
 * @param[out] entries Pointer to the application allocated memory where
 *                       the routing table is written
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function retrieves @a entry_count entries from the routing table,
 * starting from a specific index. This allows the caller to get any number of
 * entries from the routing table without having to allocate a large buffer.
 * The allocated memory should be at least equal to:
 * sizeof(sl_wisun_br_routing_table_entry_t) * entry_count.
 *
 * @note This function should be called after sl_wisun_br_get_routing_table_entry_count()
 *       to get the number of entries in the routing table.
 * @warning If the application does not retrieve the entire routing table in a single read,
 *          it may miss updates or changes in the network topology.
 *****************************************************************************/
sl_status_t sl_wisun_br_get_routing_table_from_index(uint16_t start_index,
                                                     uint16_t *entry_count,
                                                     sl_wisun_br_routing_table_entry_t *entries);

/**************************************************************************//**
 * Remove pairwise keys associated with a node.
 *
 * @param[in] address EUI-64 of the target node
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Remove the Pairwise Transient Key (PTK) and Pairwise Master Key (PMK) associated
 * with a node as described in the Wi-SUN FAN specification section 6.5.2.5.
 *
 * @note To permanently revoke access to the node, its certificate must be
 *       banned at the authentication authority (e.g. RADIUS server).
 *
 *****************************************************************************/
sl_status_t sl_wisun_br_revoke_pairwise_keys(const sl_wisun_mac_address_t *address);

/**************************************************************************//**
 * Revoke (LFN) Group Transient Keys and installs a new key.
 *
 * @param[in] revoke_lgtks True to revoke LGTKs, false to revoke GTKs
 * @param[in] gtk (LFN) Group Transient Key
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * Destroy all (L)GTKs except the current active one (and potentially the next
 * key), reduce the current (or next) key's lifetime, and add a new key, as
 * described in the Wi-SUN FAN specification section 6.5.2.5.
 *
 * @note The gtk parameter is provided for test and debug purposes: setting a
 *       specific key in production is not recommended. Set to NULL to
 *       generate a random key.
 *
 *****************************************************************************/
sl_status_t sl_wisun_br_revoke_gtks(bool revoke_lgtks, const uint8_t *gtk);

/** @} (end addtogroup SL_WISUN_BR_API) */

#ifdef __cplusplus
}
#endif

#endif  // SL_WISUN_BR_API_H
