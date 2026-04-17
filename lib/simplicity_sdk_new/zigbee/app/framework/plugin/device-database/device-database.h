/***************************************************************************//**
 * @file
 * @brief APIs and defines for the Device Database plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

/**
 * @defgroup device-database Device Database
 * @ingroup component
 * @brief API and Callbacks for the Device Database Component
 *
 * This component provides an API to add/remove a device
 * from a list of known devices, and to record their list of endpoints and clusters.
 *
 */

/**
 * @addtogroup device-database
 * @{
 */

/**
 * @name API
 * @{
 */
/** @brief Get device by index.
 *
 * @param[in] index The index into the database.
 *
 * @return NULL if no device exists at that index, else a valid pointer to the entry.
 *
 */
const sl_zigbee_af_device_info_t* sl_zigbee_af_device_database_get_device_by_index(uint16_t index);

/** @brief Find the first device that matches input status.
 *
 * @param[in] status The status to look for.
 *
 * @return NULL if no device matches status, else a valid pointer to the entry.
 *
 */
const sl_zigbee_af_device_info_t* sl_zigbee_af_device_database_find_device_by_status(sl_zigbee_af_device_discovery_status_t status);

/** @brief Find device by EUI64.
 *
 * @param[in] eui64 The EUI to search for.
 *
 * @return NULL if device is not found, else a valid pointer to the entry.
 *
 */
const sl_zigbee_af_device_info_t* sl_zigbee_af_device_database_find_device_by_eui64(sl_802154_long_addr_t eui64);

/** @brief Add to the device database.
 *
 * @param[in] eui64 The EUI to add.
 * @param[in] macCapabilities The MAC capabilities for the device.
 *
 * @return SL_STATUS_OK upon success, SL_STATUS_ALREADY_EXISTS if device is already in table,
 * else SL_STATUS_FULL if table is full.
 *
 */
sl_status_t sl_zigbee_af_device_database_add(sl_802154_long_addr_t eui64, uint8_t macCapabilities);

/** @brief Erase device from device database.
 *
 * @param[in] eui64 The EUI to search for.
 *
 * @return True if the device is found, else false.
 *
 */
bool sl_zigbee_af_device_database_erase_device(sl_802154_long_addr_t eui64);

/** @brief Set device endpoints.
 *
 * @param[in] eui64 The EUI to search for.
 * @param[in] endpointList A list of endpoints to add in the database entry for ::eui64.
 * @param[in] endpointCount The length of endpointList.
 *
 * @return True if the device is found, else false.
 *
 */
bool sl_zigbee_af_device_database_set_endpoints(const sl_802154_long_addr_t eui64,
                                                const uint8_t* endpointList,
                                                uint8_t endpointCount);

/** @brief Get device endpoint from index.
 *
 * @param[in] eui64 The EUI to search for.
 * @param[in] index The endpoint index to retrieve.
 *
 * @return The endpoint value from the entry at the given index.
 *
 */
uint8_t sl_zigbee_af_device_database_get_device_endpoint_from_index(const sl_802154_long_addr_t eui64,
                                                                    uint8_t index);

/** @brief Get endpoint index for a given endpoint value on the specified device.
 *
 * @param[in] endpoint The endpoint relevant to ::eui64.
 * @param[in] eui64 The EUI to search for.
 *
 * @return The index at which ::endpoint is stored in the entry for ::eui64, else 0xFF if
 * the device or endpoint are not found.
 */
uint8_t sl_zigbee_af_device_database_get_index_from_endpoint(uint8_t endpoint,
                                                             const sl_802154_long_addr_t eui64);

/** @brief Set clusters for device endpoint.
 *
 * @param[in] eui64 The EUI to search for.
 * @param[in] clusterList Cluster and endpoint information from ::eui64.
 *
 * @return True upon successful update, else false if device or endpoint on device is not found.
 *
 * @note The endpoint specified in ::clusterList must previously be written to the entry using
 * ::sl_zigbee_af_device_database_set_endpoints.
 */
bool sl_zigbee_af_device_database_set_clusters_for_endpoint(const sl_802154_long_addr_t eui64,
                                                            const sl_zigbee_af_cluster_list_t* clusterList);

/** @brief Searches for any entries that have a failed discovery state (SL_ZIGBEE_AF_DEVICE_DISCOVERY_STATUS_FAILED) and
 * whose number of failures is less than ::maxFailureCount. Clears those entries' statuses by setting them
 * to SL_ZIGBEE_AF_DEVICE_DISCOVERY_STATUS_NEW.
 *
 * @param[in] maxFailureCount The maximum number of failures to filter for when looking for entries
 * in the SL_ZIGBEE_AF_DEVICE_DISCOVERY_STATUS_FAILED state. If an entry
 * has a failure count greater than or equal to ::maxFailureCount, then its status won't be cleared.
 *
 * @return True if at least one entry was cleared, false otherwise.
 *
 * @note This component works alongside the Device Query Service component, which manages device status in this Device
 * Database component. The user is not expected to call these APIs to update device status.
 */
bool sl_zigbee_af_device_database_clear_all_failed_discovery_status(uint8_t maxFailureCount);

/** @brief Set device database status for a given device. This API is used by other components to handle device state
 * when probing the device, for instance. The generic device state is stored in this Device Database component, but this
 * component performs no action based on that state.
 *
 * @param[in] deviceEui64 The EUI to search for.
 * @param[in] newStatus The status to set.
 *
 * @return True if the device is found, else false.
 *
 * @note This component works alongside the Device Query Service component, which manages device status in this Device
 * Database component. The user is not expected to call these APIs to update device status.
 *
 */
bool sl_zigbee_af_device_database_set_status(const sl_802154_long_addr_t deviceEui64,
                                             sl_zigbee_af_device_discovery_status_t newStatus);

/** @brief Add device to database with all information filled out.
 *
 * @param[in] newDevice Pointer with complete device entry information.
 *
 * @return SL_STATUS_OK upon success, SL_STATUS_ALREADY_EXISTS if device is already in table,
 * else SL_STATUS_FULL if table is full.
 */
sl_status_t sl_zigbee_af_device_database_add_device_with_all_info(const sl_zigbee_af_device_info_t* newDevice);

/** @brief Check if a device has cluster.
 *
 * @param[in] deviceEui64 The EUI to search for.
 * @param[in] clusterToFind Cluster to search for.
 * @param[in] server True to find server-side cluster, false for client-side.
 * @param[out] returnEndpoint Upon success, filled in with the endpoint matching the input cluster.
 *
 * @return SL_STATUS_OK if found, SL_STATUS_INVALID_PARAMETER if device not in database,
 * else SL_STATUS_NOT_FOUND if device is found but no match on cluster.
 *
 */
sl_status_t sl_zigbee_af_device_database_does_device_have_cluster(sl_802154_long_addr_t deviceEui64,
                                                                  sl_zigbee_af_cluster_id_t clusterToFind,
                                                                  bool server,
                                                                  uint8_t* returnEndpoint);

/** @brief Create a new search. Resets iterator starting index to 0.
 *
 * @param[out] iterator Param to update. Essentially has its deviceIndex set to 0.
 *
 * @note This function is typically called before calling ::sl_zigbee_af_device_database_find_device_supporting_cluster,
 * which is done to find the entry index of a device matching an input cluster.
 *
 * @return None
 *
 * @note This function is used when a caller wants to know which index in the database matches
 * a given search criterion.
 *
 */
void sl_zigbee_af_device_database_create_new_search(sl_zigbee_af_device_database_iterator_t* iterator);

/** @brief Finds the next device that supports the given cluster.
 *
 * @param[in,out] iterator Upon success, deviceIndex field is updated with index of device matching cluster.
 * @param[in] clusterToFind Cluster to search for.
 * @param[in] server True to find server-side cluster, false for client-side.
 * @param[out] returnEndpoint Upon success, filled in with the endpoint matching the input cluster.
 *
 * @return SL_STATUS_OK if found, SL_STATUS_INVALID_INDEX if iterator index is beyond max table size,
 * else SL_STATUS_NOT_FOUND if no device in the database supports the desired cluster.
 */
sl_status_t sl_zigbee_af_device_database_find_device_supporting_cluster(sl_zigbee_af_device_database_iterator_t* iterator,
                                                                        sl_zigbee_af_cluster_id_t clusterToFind,
                                                                        bool server,
                                                                        uint8_t* returnEndpoint);

/** @} */ // end of name API
/** @} */ // end of device-database

void sli_zigbee_af_device_database_update_node_stack_revision(sl_802154_long_addr_t eui64,
                                                              uint8_t stackRevision,
                                                              uint8_t macCapabilities); // 0xFF doesn't update the device's capabilities
