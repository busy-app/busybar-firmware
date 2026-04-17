/***************************************************************************//**
 * @file
 * @brief It implements and manages the ESI table. The ESI table is shared among
 *   other plugins.
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
 * @defgroup esi-management  ESI Management
 * @ingroup component
 * @brief API and Callbacks for the ESI Management Component
 *
 * This component maintains a table of ESIs that can be queried by other components.
 * This is used by all Smart Energy client devices.
 *
 */

/**
 * @addtogroup esi-management
 * @{
 */
#ifdef SL_ZIGBEE_SCRIPTED_TEST
#include "config/esi-management-config.h"
#else
#include "esi-management-config.h"
#endif

#ifndef SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_MIN_ERASING_AGE
#define SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_MIN_ERASING_AGE 3
#endif //SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_MIN_ERASING_AGE

#ifndef SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_PLUGIN_CALLBACK_TABLE_SIZE
#define SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_PLUGIN_CALLBACK_TABLE_SIZE 5
#endif // SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_PLUGIN_CALLBACK_TABLE_SIZE

typedef struct {
  sl_802154_long_addr_t eui64;
  sl_802154_short_addr_t nodeId;
  uint8_t networkIndex;
  uint8_t endpoint;
  uint8_t age; // The number of discovery cycles the ESI has not been discovered.
} sl_zigbee_af_plugin_esi_management_esi_entry_t;

typedef void(*sl_zigbee_af_esi_management_deletion_callback_t)(uint8_t);

/**
 * @name API
 * @{
 */

/**
 * Search in the ESI table by the pair node (short ID, endopoint).
 *
 * Returns a pointer to the entry if a matching entry was found. Otherwise, it
 * returns NULL.
 */
sl_zigbee_af_plugin_esi_management_esi_entry_t* sl_zigbee_af_esi_management_esi_look_up_by_short_id_and_endpoint(sl_802154_short_addr_t shortId,
                                                                                                                 uint8_t endpoint);

/**
 * Search in the ESI table by the pair node (long ID, endopoint).
 *
 * Returns a pointer to the entry if a matching entry was found. Otherwise, it
 * returns NULL.
 */
sl_zigbee_af_plugin_esi_management_esi_entry_t* sl_zigbee_af_esi_management_esi_look_up_by_long_id_and_endpoint(sl_802154_long_addr_t longId,
                                                                                                                uint8_t endpoint);

/**
 * Allow retrieving the index of an entry that matches the passed short ID
 * and endpoint.
 *
 * Returns the index of the matching entry if a matching entry was found.
 * Otherwise, it returns 0xFF.
 */
uint8_t sl_zigbee_af_esi_management_index_look_up_by_short_id_and_endpoint(sl_802154_short_addr_t shortId,
                                                                           uint8_t endpoint);

/**
 * Allow retrieving the index of an entry that matches the passed long ID
 * and endpoint.
 *
 * Returns the index of the matching entry if a matching entry was found.
 * Otherwise, it returns 0xFF.
 */
uint8_t sl_zigbee_af_esi_management_index_look_up_by_long_id_and_endpoint(sl_802154_long_addr_t longId,
                                                                          uint8_t endpoint);

/**
 * Search in the ESI table by the table index.
 *
 * Returns a pointer to the ESI entry stored at the index passed as
 * parameter.
 */
sl_zigbee_af_plugin_esi_management_esi_entry_t* sl_zigbee_af_esi_management_esi_look_up_by_index(uint8_t index);

/**
 * Iterate through the entries in the
 * table that are within a certain age threshold.
 *
 * If the passed pointer is NULL, it returns the first active entry with age
 * lesser or equal than the passed age parameter (if any). Otherwise, it returns
 * the next active entry that satisfy the age requirement. If the are no entries
 * after the passed entry that satisfy the age requirement, it returns NULL.
 */
sl_zigbee_af_plugin_esi_management_esi_entry_t* sl_zigbee_af_esi_management_get_next_entry(sl_zigbee_af_plugin_esi_management_esi_entry_t* entry,
                                                                                           uint8_t age);

/**
 * Allow obtaining a free entry in the ESI table. It is the
 * requester responsibility to properly set all the fields in the obtained free
 * entry such as nodeId, age, and so on to avoid inconsistencies in the
 * table.
 *
 * Returns a free entry (if any), otherwise it clears the oldest entry whose age
 * is at least SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_MIN_ERASING_AGE (if any) and
 * returns it, otherwise it returns NULL.
 */
sl_zigbee_af_plugin_esi_management_esi_entry_t* sl_zigbee_af_esi_management_get_free_entry(void);

/**
 * Delete the entry indicated by the parameter 'index' from the
 * ESI table.
 */
void sl_zigbee_af_esi_management_delete_entry(uint8_t index);

/**
 * Increase the age of all the active entries in the table. A
 * non-active entry is an entry whose short ID is set to SL_ZIGBEE_NULL_NODE_ID.
 */
void sl_zigbee_af_esi_management_age_all_entries(void);

/**
 * Clear the ESI table, i.e., set the short ID of each entry
 * to SL_ZIGBEE_NULL_NODE_ID.
 */
void sl_zigbee_af_esi_management_clear_table(void);

/**
 * Allow a plugin to subscribe to ESI entries deletion
 * announcements by passing its own deletion callback function. Upon an entry
 * deletion, all the deletion callback function are called passing the index
 * of the deleted entry.
 *
 * It returns true if the subscription was successful, false otherwise.
 */
bool sl_zigbee_af_esi_management_subscribe_to_deletion_announcements(sl_zigbee_af_esi_management_deletion_callback_t callback);

/**
 * Perform the following steps:
 *  - Search for the source node of the passed command in the ESI table.
 *  - Add a new entry in the ESI table if the source node is not present in the
 *    ESI table yet, or update the current entry if needed.
 *
 *  @return The index of the source node of the passed command in the ESI
 *  table, or it returns 0xFF if the ESI was not present in the table and a new
 *  entry could not be added since the table was full.
 **/
uint8_t sl_zigbee_af_esi_management_update_esi_and_get_index(const sl_zigbee_af_cluster_command_t *cmd);

/** @} */ // end of name API
/** @} */ // end of esi-management
