/***************************************************************************//**
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

#include "app/framework/include/af.h"
#include "address-table.h"

#include "address-table-config.h"

#define FREE_EUI64 { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }

#ifdef EZSP_HOST
  #define SET_CACHE_ADDRESS(index, longId)   memcpy(addressTable[(index)], (longId), EUI64_SIZE)
  #define CLEAR_CACHE_ADDRESS(index)         memset(&addressTable[(index)], 0xFF, EUI64_SIZE)
  #define GET_ADDRESS(index, nodeId, dstEntry)       memmove((dstEntry), addressTable[(index)], EUI64_SIZE);
#else
  #define SET_CACHE_ADDRESS(index, longId)
  #define CLEAR_CACHE_ADDRESS(index)
  #define GET_ADDRESS(index, nodeId, dstEntry)       (void)sl_zigbee_get_address_table_info((index), (nodeId), (dstEntry))
#endif

static uint8_t addressTableReferenceCounts[SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE];
#ifdef EZSP_HOST
static sl_802154_long_addr_t addressTable[SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE];
static bool initPending = true;
#endif  // EZSP_HOST

#ifdef EZSP_HOST
void sl_zigbee_af_address_table_ncp_init_cb(void)
{
  uint8_t index;
  sl_802154_long_addr_t freeEui = FREE_EUI64;
  uint16_t addressTableSize;

  // If the host and the ncp disagree on the address table size, explode.
  sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_ADDRESS_TABLE_SIZE, &addressTableSize);
  // After the change of ncp memory model in UC, we can not increase the default NCP table sizes anymore.
  // Therefore, checking for SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE == (ncp)addressTableSize might not be always true anymore
  assert(SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE <= addressTableSize);

  if (initPending) {
    // Initialize all the entries to all 0xFFs. All 0xFFs means that the entry
    // is unused.
    memset(addressTable, 0xFF, EUI64_SIZE * SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE);
    initPending = false;
    return;
  }

  // When the NCP resets the address table at the NCP is empty (it is only
  // stored in RAM). We re-add all the non-empty entries at the NCP.
  for (index = 0; index < SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE; index++) {
    if (memcmp(addressTable[index], freeEui, EUI64_SIZE) != 0) {
      sl_status_t status = sl_zigbee_set_address_table_info(index, addressTable[index], SL_ZIGBEE_UNKNOWN_NODE_ID);
      if (status != SL_STATUS_OK) {
      #ifdef EZSP_HOST
        sl_zigbee_ezsp_error_handler(status);
        return;
      #else // !EZSP_HOST
        assert(0);
      #endif //EZSP_HOST
      }
      // we should always be able to add an entry at the NCP here.
    }
  }
}
#endif

// Non Plugin Specific functions.
sl_status_t sl_zigbee_af_send_command_unicast_to_eui64(sl_802154_long_addr_t destination)
{
  return sl_zigbee_af_send_unicast_to_eui64(destination,
                                            sli_zigbee_af_command_aps_frame,
                                            *sli_zigbee_af_response_length_ptr,
                                            sli_zigbee_af_zcl_buffer);
}

uint8_t sl_zigbee_af_lookup_address_table_entry_by_eui64(sl_802154_long_addr_t longId)
{
  uint8_t i;
  if (longId == NULL) {
    goto kickout;
  }
#ifdef EZSP_HOST
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE; i++) {
    if (memcmp(addressTable[i], longId, EUI64_SIZE) == 0) {
      return i;
    }
  }
#else
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE; i++) {
    sl_802154_long_addr_t eui64;
    sl_802154_short_addr_t nodeId;
    sl_zigbee_get_address_table_info(i, &nodeId, eui64);
    if ((nodeId != SL_ZIGBEE_TABLE_ENTRY_UNUSED_NODE_ID)
        && (memcmp(longId, eui64, EUI64_SIZE) == 0)) {
      return i;
    }
  }
#endif
  kickout:
  return SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX;
}

sl_status_t sl_zigbee_af_send_unicast_to_eui64(sl_802154_long_addr_t destination,
                                               sl_zigbee_aps_frame_t *apsFrame,
                                               uint16_t messageLength,
                                               uint8_t *message)
{
  uint8_t index = sl_zigbee_af_address_table_lookup_by_eui64(destination);

  if (index == SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX) {
    return SL_STATUS_INVALID_STATE;
  }

  return sl_zigbee_af_send_unicast(SL_ZIGBEE_OUTGOING_VIA_ADDRESS_TABLE,
                                   index,
                                   apsFrame,
                                   messageLength,
                                   message);
}

uint8_t sl_zigbee_af_add_address_table_entry(sl_802154_long_addr_t longId, sl_802154_short_addr_t shortId)
{
  uint8_t i, index = SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX;
  // search for unused node entry / finding existing entry with matching longId
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE; i++) {
    sl_802154_long_addr_t eui64;
    sl_802154_short_addr_t nodeId;
    sl_zigbee_get_address_table_info(i, &nodeId, eui64);
    if (nodeId != SL_ZIGBEE_TABLE_ENTRY_UNUSED_NODE_ID) {
      if (memcmp(longId, eui64, EUI64_SIZE) == 0) {
        index = i;
        goto kickout;
      }
    } else if (index == SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX) {
      index = i;
    } else {
      // MISRA requires ..else if.. to have terminating else.
    }
  }

  if (index == SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX) {
    return index;
  }

  sl_zigbee_set_address_table_info(index, longId, SL_ZIGBEE_UNKNOWN_NODE_ID);
  SET_CACHE_ADDRESS(index, longId);

  kickout:
  if (shortId != SL_ZIGBEE_UNKNOWN_NODE_ID) {
    sl_zigbee_set_address_table_info(index, longId, shortId);
  }
  if (index < SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE) {
    addressTableReferenceCounts[index]++;
  }
  return index;
}

sl_status_t sl_zigbee_af_set_address_table_entry(uint8_t index,
                                                 sl_802154_long_addr_t longId,
                                                 sl_802154_short_addr_t shortId)
{
  sl_status_t status = SL_STATUS_INVALID_RANGE;
  if (index < SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE) {
    if (addressTableReferenceCounts[index] == 0) {
      status = sl_zigbee_set_address_table_info(index, longId, shortId);
      SET_CACHE_ADDRESS(index, longId);
      addressTableReferenceCounts[index] = 1;
    } else {
      status = SL_STATUS_ZIGBEE_ADDRESS_TABLE_ENTRY_IS_ACTIVE;
    }
  }
  return status;
}

sl_status_t sl_zigbee_af_remove_address_table_entry(uint8_t index)
{
  sl_status_t status = SL_STATUS_INVALID_RANGE;
  if (index < SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE) {
    if (0 < addressTableReferenceCounts[index]) {
      addressTableReferenceCounts[index]--;
    }
    if (addressTableReferenceCounts[index] == 0) {
      sl_802154_short_addr_t nodeId;
      sl_802154_long_addr_t eui64;
      sl_zigbee_get_address_table_info(index, &nodeId, eui64);
      status = sl_zigbee_set_address_table_info(index,
                                                eui64,
                                                SL_ZIGBEE_TABLE_ENTRY_UNUSED_NODE_ID);
      CLEAR_CACHE_ADDRESS(index);
    }
    status = SL_STATUS_OK;
  }
  return status;
}

sl_status_t sl_zigbee_af_get_current_sender_eui64(sl_802154_long_addr_t address)
{
  uint8_t index = sli_zigbee_af_get_address_index();
  if (index == SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX) {
    return SL_STATUS_INVALID_STATE;
  } else {
    return sl_zigbee_af_address_table_lookup_by_index(index, address);
  }
}

// Plugin functions.

sl_802154_short_addr_t sl_zigbee_af_address_table_lookup_node_id_by_index(uint8_t index)
{
  sl_802154_long_addr_t eui64;
  if (sl_zigbee_af_address_table_lookup_by_index(index, eui64) == SL_STATUS_OK) {
    UNUSED sl_802154_short_addr_t nodeId; //only used on SoC
    (void) sl_zigbee_lookup_node_id_by_eui64(eui64, &nodeId);
    return nodeId;
  }
  return SL_ZIGBEE_NULL_NODE_ID;
}

sl_status_t sl_zigbee_af_address_table_remove_entry(sl_802154_long_addr_t entry)
{
  uint8_t index = sl_zigbee_af_address_table_lookup_by_eui64(entry);
  return sl_zigbee_af_address_table_remove_entry_by_index(index);
}

sl_status_t sl_zigbee_af_address_table_lookup_by_index(uint8_t index,
                                                       sl_802154_long_addr_t entry)
{
  if (index < SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE) {
    sl_802154_short_addr_t nodeId = SL_ZIGBEE_NULL_NODE_ID;
    UNUSED_VAR(nodeId);
    GET_ADDRESS(index, &nodeId, entry);
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_INVALID_STATE;
  }
}
