/***************************************************************************//**
 * @file
 * @brief It implements and manages the ESI table. The ESI table is shared among
 * other plugins.
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

#include "app/framework/include/af.h"

#include "esi-management.h"

static sl_zigbee_af_plugin_esi_management_esi_entry_t esiTable[SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE];
static sl_zigbee_af_esi_management_deletion_callback_t deletionCallbackTable[SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_PLUGIN_CALLBACK_TABLE_SIZE];
static uint8_t deletionCallbackTableSize = 0;

static void performDeletionAnnouncement(uint8_t index)
{
  uint8_t i;

  // Notify all the subscribed plugins about this deletion.
  for (i = 0; i < deletionCallbackTableSize; i++) {
    (deletionCallbackTable[i])(index);
  }
}

sl_zigbee_af_plugin_esi_management_esi_entry_t* sl_zigbee_af_esi_management_get_free_entry(void)
{
  sl_zigbee_af_plugin_esi_management_esi_entry_t* entry = NULL;
  uint8_t networkIndex = sl_zigbee_get_current_network();
  uint8_t deletedEsiIndex = 0;
  uint8_t i;

  // Look for a free entry first.
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE; i++) {
    if (esiTable[i].nodeId == SL_ZIGBEE_NULL_NODE_ID) {
      return &(esiTable[i]);
    }
  }

  // No free entry found, we look for the oldest entry among those that
  // can be erased.
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE; i++) {
    if (esiTable[i].networkIndex == networkIndex
        && esiTable[i].age >= SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_MIN_ERASING_AGE
        && (entry == NULL || esiTable[i].age > entry->age)) {
      entry = &(esiTable[i]);
      deletedEsiIndex = i;
    }
  }

  if (entry != NULL) {
    performDeletionAnnouncement(deletedEsiIndex);
  }

  return entry;
}

sl_zigbee_af_plugin_esi_management_esi_entry_t* sl_zigbee_af_esi_management_esi_look_up_by_short_id_and_endpoint(sl_802154_short_addr_t shortId,
                                                                                                                 uint8_t endpoint)
{
  uint8_t index =
    sl_zigbee_af_esi_management_index_look_up_by_short_id_and_endpoint(shortId,
                                                                       endpoint);
  if (index < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE) {
    return &(esiTable[index]);
  } else {
    return NULL;
  }
}

sl_zigbee_af_plugin_esi_management_esi_entry_t* sl_zigbee_af_esi_management_esi_look_up_by_long_id_and_endpoint(sl_802154_long_addr_t longId,
                                                                                                                uint8_t endpoint)
{
  uint8_t index =
    sl_zigbee_af_esi_management_index_look_up_by_long_id_and_endpoint(longId,
                                                                      endpoint);
  if (index < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE) {
    return &(esiTable[index]);
  } else {
    return NULL;
  }
}

uint8_t sl_zigbee_af_esi_management_index_look_up_by_short_id_and_endpoint(sl_802154_short_addr_t shortId,
                                                                           uint8_t endpoint)
{
  uint8_t networkIndex = sl_zigbee_get_current_network();
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE; i++) {
    if (esiTable[i].networkIndex == networkIndex
        && esiTable[i].nodeId == shortId
        && esiTable[i].endpoint == endpoint) {
      return i;
    }
  }

  return 0xFF;
}

uint8_t sl_zigbee_af_esi_management_index_look_up_by_long_id_and_endpoint(sl_802154_long_addr_t longId,
                                                                          uint8_t endpoint)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE; i++) {
    if (memcmp(longId, esiTable[i].eui64, EUI64_SIZE) == 0
        && esiTable[i].endpoint == endpoint) {
      return i;
    }
  }

  return 0xFF;
}

sl_zigbee_af_plugin_esi_management_esi_entry_t* sl_zigbee_af_esi_management_esi_look_up_by_index(uint8_t index)
{
  if (index < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE
      && esiTable[index].nodeId != SL_ZIGBEE_NULL_NODE_ID) {
    return &(esiTable[index]);
  } else {
    return NULL;
  }
}

sl_zigbee_af_plugin_esi_management_esi_entry_t* sl_zigbee_af_esi_management_get_next_entry(sl_zigbee_af_plugin_esi_management_esi_entry_t* entry,
                                                                                           uint8_t age)
{
  uint8_t networkIndex = sl_zigbee_get_current_network();
  uint8_t i;
  bool entryFound = false;

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE; i++) {
    // If the passed entry is NULL we return the first active entry (if any).
    // If we already encountered the passed entry, we return the next active
    // entry (if any).
    if ((entry == NULL || entryFound)
        && esiTable[i].networkIndex == networkIndex
        && esiTable[i].nodeId != SL_ZIGBEE_NULL_NODE_ID
        && esiTable[i].age <= age) {
      return &(esiTable[i]);
    }
    // We found the passed entry in the table.
    if (&(esiTable[i]) == entry) {
      entryFound = true;
    }
  }

  return NULL;
}

void sl_zigbee_af_esi_management_delete_entry(uint8_t index)
{
  assert(index < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE);

  esiTable[index].nodeId = SL_ZIGBEE_NULL_NODE_ID;
  performDeletionAnnouncement(index);
}

void sl_zigbee_af_esi_management_age_all_entries(void)
{
  uint8_t networkIndex = sl_zigbee_get_current_network();
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE; i++) {
    if (esiTable[i].networkIndex == networkIndex
        && esiTable[i].nodeId != SL_ZIGBEE_NULL_NODE_ID
        && esiTable[i].age < 0xFF) {
      esiTable[i].age++;
    }
  }
}

void sl_zigbee_af_esi_management_clear_table(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE; i++) {
    sl_zigbee_af_esi_management_delete_entry(i);
  }
}

bool sl_zigbee_af_esi_management_subscribe_to_deletion_announcements(sl_zigbee_af_esi_management_deletion_callback_t callback)
{
  if (deletionCallbackTableSize
      < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_PLUGIN_CALLBACK_TABLE_SIZE) {
    deletionCallbackTable[deletionCallbackTableSize++] = callback;
    return true;
  } else {
    return false;
  }
}

uint8_t sl_zigbee_af_esi_management_update_esi_and_get_index(const sl_zigbee_af_cluster_command_t *cmd)
{
  sl_802154_long_addr_t esiEui64;
  sl_zigbee_af_plugin_esi_management_esi_entry_t *esiEntry;
  uint8_t index;

  // Replace assert with equivalent conditional to quiet CSTAT complaint about
  // possible dereference of null pointer.  The 'return 0' is never executed.
  // This approach satisfies all code lines where 'cmd' is dereferenced.
  // assert(cmd != NULL);
  if (cmd == NULL) {
    assert(false); return 0;
  }

  (void) sl_zigbee_af_push_network_index(cmd->networkIndex);

  sl_zigbee_lookup_eui64_by_node_id(cmd->source, esiEui64);
  esiEntry = sl_zigbee_af_esi_management_esi_look_up_by_long_id_and_endpoint(esiEui64,
                                                                             cmd->apsFrame->sourceEndpoint);
  // The source ESI is not in the ESI table.
  if (esiEntry == NULL) {
    sl_zigbee_af_debug_println("source ESI 0x%02X not found in table", cmd->source);
    // We add the ESI to the table.
    esiEntry = sl_zigbee_af_esi_management_get_free_entry();
    if (esiEntry != NULL) {
      esiEntry->nodeId = cmd->source;
      esiEntry->networkIndex = cmd->networkIndex;
      esiEntry->endpoint = cmd->apsFrame->sourceEndpoint;
      esiEntry->age = 0;
      memmove(esiEntry->eui64, esiEui64, EUI64_SIZE);
    } else {
      sl_zigbee_af_debug_println("No free entry available");
    }
  } else {
    // Check that the short ID is still the one we stored in the ESI table.
    // If not, update it.
    if (esiEntry->nodeId != cmd->source) {
      sl_zigbee_af_debug_println("ESI short ID changed, updating it");
      esiEntry->nodeId = cmd->source;
    }
  }

  index = sl_zigbee_af_esi_management_index_look_up_by_long_id_and_endpoint(esiEui64,
                                                                            cmd->apsFrame->sourceEndpoint);
  (void) sl_zigbee_af_pop_network_index();
  return index;
}

void sl_zigbee_af_esi_management_init_cb(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_esi_management_clear_table();
}
