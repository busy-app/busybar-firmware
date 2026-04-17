/***************************************************************************/ /**
 * @file
 * @brief This code provides support for managing the address table.
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

#ifndef SILABS_PLUGIN_ADDRESS_TABLE_H
#define SILABS_PLUGIN_ADDRESS_TABLE_H

/**
 * @defgroup address-table Address Table
 * @ingroup component
 * @brief API and Callbacks for the Address Table Component
 *
 * This component supports managing the address table. On the SoC, all
 * actions are directly performed on the stack address table. The host
 * maintains a copy of the address table and ensures that it is always in
 * sync with the NCP's address table. When building the Host, make sure that the
 * NCP can support the address table size in the component configuration.
 * The optimal size varies based on the platform architecture and size of other
 * dynamically-allocated data for this application. If errors occur upon
 * reseting the NCP, the NCP may not have enough room to store a large address
 * table size. If this is not the case, the NCP may not be able to dynamically
 * increase the address table size based on the host's request. To work around
 * this issue, statically allocate the desired address table size on
 * the Zigbee Pro Stack\Pro Leaf Stack components on the NCP.
 *
 */

/**
 * @addtogroup address-table
 * @{
 */

/**
 * @name API
 * @{
 */

/**
 * @brief Add a new entry to the address table.
 *
 * If the entry already exists, no
 * new entry is created and the index of the entry already in the table is
 * returned. If a free entry is found, the entry is added to the table and the
 * corresponding index is returned. If no free entry is available in the address
 * table, the value SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX is returned.
 */
#define sl_zigbee_af_address_table_add_entry(longId) \
  sl_zigbee_af_add_address_table_entry((longId), SL_ZIGBEE_UNKNOWN_NODE_ID);

/**
 * @brief Remove an entry from the address table.
 *
 * If the entry exists, the
 * entry is cleared and the SL_STATUS_OK status is returned. Otherwise, the
 * status SL_STATUS_INVALID_STATE is returned.
 */
sl_status_t sl_zigbee_af_address_table_remove_entry(sl_802154_long_addr_t entry);

/**
 * @brief Remove an entry from the address table by index.
 *
 * If the index is valid,
 * the entry is cleared and the SL_STATUS_OK status is returned. Otherwise, the
 * status SL_STATUS_INVALID_STATE is returned.
 */
#define sl_zigbee_af_address_table_remove_entry_by_index(index) \
  sl_zigbee_af_remove_address_table_entry((index))

/**
 * @brief Search for an EUI64 in the address table.
 *
 * If an entry with a
 * matching EUI64 is found, the function returns the corresponding index.
 * Otherwise, the value SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX is returned.
 */
#define sl_zigbee_af_address_table_lookup_by_eui64(longId) \
  sl_zigbee_af_lookup_address_table_entry_by_eui64((longId))
uint8_t sl_zigbee_af_lookup_address_table_entry_by_eui64(sl_802154_long_addr_t longId);

/**
 * @brief Copy the entry stored at the passed
 * index to the passed variable and return SL_STATUS_OK if the
 * passed index is valid.
 *
 * Otherwise, the status
 * SL_STATUS_INVALID_STATE is returned.
 */
sl_status_t sl_zigbee_af_address_table_lookup_by_index(uint8_t index,
                                                       sl_802154_long_addr_t entry);

/**
 * @brief Copy the entry stored at the passed
 * index to the passed variable and return SL_STATUS_OK
 * if the passed index is valid.
 *
 * Otherwise, the status
 * SL_STATUS_INVALID_STATE is returned.
 */
sl_802154_short_addr_t sl_zigbee_af_address_table_lookup_node_id_by_index(uint8_t index);

/**
 * @brief Allow sending a unicast message specifying the EUI64 of the
 * destination node.
 *
 * Note: the specified EUI64 must be present in the address
 * table. Otherwise, the status SL_STATUS_INVALID_STATE is returned.
 */
sl_status_t sl_zigbee_af_send_unicast_to_eui64(sl_802154_long_addr_t destination,
                                               sl_zigbee_aps_frame_t *apsFrame,
                                               uint16_t messageLength,
                                               uint8_t *message);

/**
 * @brief Send the command prepared with sl_zigbee_af_fill.... macro.
 *
 * This function sends a command that was previously prepared
 * using the sl_zigbee_af_fill... macros from the client command API.
 * It will be sent as unicast to the node that corresponds in the address table
 * to the passed EUI64.
 */
sl_status_t sl_zigbee_af_send_command_unicast_to_eui64(sl_802154_long_addr_t destination);

/**
 * @brief Copy the sender EUI64, if available, to the address parameter
 * and return SL_STATUS_OK. Otherwise, return SL_STATUS_INVALID_STATE.
 */
sl_status_t sl_zigbee_af_get_current_sender_eui64(sl_802154_long_addr_t address);

/** @} */ // end of name APIs
/** @} */ // end of group address-table

#endif // SILABS_PLUGIN_ADDRESS_TABLE_H
