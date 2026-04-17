/***************************************************************************//**
 * @file
 * @brief CLI for the MAC address filtering plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifdef PRO_COMPLIANCE
 #include "app/test/pro-compliance.h"
 #include "app/test/pro-compliance-printing.h"
#else // PRO_COMPLIANCE
 #include "app/framework/include/af.h"
 #include "app/util/serial/sl_zigbee_command_interpreter.h"
#endif /* PRO_COMPLIANCE */

#include "mac-address-filtering.h"

// *****************************************************************************
// Globals
#if (defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_DIAGNOSTICS_CLUSTER)) || (SL_ZIGBEE_DEBUG_CORE_GROUP_ENABLED == 1)

static const sli_802154_mac_address_filter_option_description_t optionsDescription[] = {
  { MAC_FILTER_OPTIONS_NO_ADDRESS_MASK, "No Address Packets  ", "Accept   ", "Drop     " },
  { MAC_FILTER_OPTIONS_SHORT_ADDRESS_LIST_MASK, "Short Address Filter", "Whitelist", "Blacklist" },
  { MAC_FILTER_OPTIONS_LONG_ADDRESS_LIST_MASK, "Long Address Filter ", "Whitelist", "Blacklist" },
  { MAC_FILTER_OPTIONS_DEST_PAN_ID_LIST_MASK, "Dest PAN ID Filter", "Whitelist", "Blacklist" },

  { 0, NULL, NULL, NULL }, // terminator
};
#endif

// // -----------------------------------------------------------------------------
// // Command definitions

// Command: plugin mac-address-filtering print-config
// Description: Prints the current filtering config and statistics.
void sl_zigbee_af_mac_address_filtering_print_config_command(sl_cli_command_arg_t * arguments)
{
  UNUSED_VAR(arguments);
  #if (defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_DIAGNOSTICS_CLUSTER)) || (SL_ZIGBEE_DEBUG_CORE_GROUP_ENABLED == 1)
  uint8_t i;
  sl_zigbee_af_core_println("Total MAC Packets received: %lu", macAddressFilterData.totalPackets);
  sl_zigbee_af_core_println("Options bitmask: 0x%02X", macAddressFilterData.options);
  for ( i = 0; i < MAC_FILTER_OPTIONS_COUNT; i++ ) {
    const char* enabledDisabledTextPointer = ( (macAddressFilterData.options & optionsDescription[i].bit)
                                               ? optionsDescription[i].enabledDescription
                                               : optionsDescription[i].disabledDescription);
    sl_zigbee_af_core_println("%s: %s  Packets Accepted %lu, Dropped: %lu",
                              optionsDescription[i].optionName,
                              enabledDisabledTextPointer,
                              macAddressFilterData.allowedPacketCount[i],
                              macAddressFilterData.droppedPacketCount[i]);
  }

  sl_zigbee_af_core_println("Short Address Filter List");

  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_MAC_ADDRESS_FILTERING_SHORT_ADDRESS_FILTER_LENGTH; i++ ) {
    sl_zigbee_af_core_print("%d: ", i);
    if (macAddressFilterData.macShortAddressList[i].shortAddress
        == SL_ZIGBEE_NULL_NODE_ID) {
      sl_zigbee_af_core_println("---");
    } else {
      sl_zigbee_af_core_println("0x%04X  Match Count: %lu",
                                macAddressFilterData.macShortAddressList[i].shortAddress,
                                macAddressFilterData.macShortAddressList[i].matchCount);
    }
  }

  sl_zigbee_af_core_println("\nLong Address Filter List");

  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_MAC_ADDRESS_FILTERING_LONG_ADDRESS_FILTER_LENGTH; i++ ) {
    sl_zigbee_af_core_print("%d: ", i);
    if (0 == memcmp(macAddressFilterData.macLongAddressList[i].longAddress,
                    zeroEui64,
                    EUI64_SIZE)) {
      sl_zigbee_af_core_println("---");
    } else {
      sl_zigbee_af_print_big_endian_eui64(macAddressFilterData.macLongAddressList[i].longAddress);
      sl_zigbee_af_core_println("  Match Count: %lu",
                                macAddressFilterData.macLongAddressList[i].matchCount);
    }
  }
  #endif
}

// Command: plugin mac-address-filtering clear-stats
// Description: Clears all packet counts for all modes.
void sl_zigbee_af_mac_address_filtering_clear_stats_command(sl_cli_command_arg_t * arguments)
{
  UNUSED_VAR(arguments);
  sl_zigbee_af_mac_address_filtering_clear_stats();
}

// Plugin CLI Description: Commands to manipulate the MAC address filtering tables.
// Command: plugin mac-address-filtering set-config no-address-filter <enable>
// Description: Enable/disable filtering of no-address messages.
// Arg: enable | bool | True means drop no-address messages, false means allow.
void sl_zigbee_af_mac_address_filtering_filter_no_address_command(sl_cli_command_arg_t * arguments)
{
  bool filterNoAddress = (bool)sl_cli_get_argument_uint32(arguments, 0);
  sl_zigbee_af_mac_address_filtering_filter_no_address(filterNoAddress);
}

// Command: plugin mac-address-filtering short-address-list add <node-id>
// Description: Adds an address to the short address list.
// Arg: node-id | sl_802154_short_addr_t | The node-id to add to the short list.
void sl_zigbee_af_mac_address_filtering_add_short_address_command(sl_cli_command_arg_t * arguments)
{
  sl_802154_short_addr_t nodeId = (uint16_t)sl_cli_get_argument_uint32(arguments, 0);
  sl_zigbee_af_mac_address_filtering_add_short_address(nodeId);
}

// Command: plugin mac-address-filtering short-address-list set-blacklist/set-whitelist
// Description:  Sets the short address list filter to a blacklist/whitelist.
void sl_zigbee_af_mac_address_filtering_set_short_address_list_type_command(sl_cli_command_arg_t * arguments)
{
  uint8_t* pointer
    = sl_zigbee_cli_get_argument_string_and_length(arguments, -1, // argument number
                                                   NULL); // returned length
  sl_zigbee_af_mac_address_filtering_set_short_address_list_type(pointer);
}

// Command: plugin mac-address-filtering long-address-list set-blacklist/set-whitelist
// Description: Sets the long address list filter to a blacklist/whitelist.
void sl_zigbee_af_mac_address_filtering_set_long_address_list_type_command(sl_cli_command_arg_t * arguments)
{
  uint8_t* pointer
    = sl_zigbee_cli_get_argument_string_and_length(arguments, -1, // argument number
                                                   NULL); // returned length
  sl_zigbee_af_mac_address_filtering_set_long_address_list_type(pointer);
}

// Command: plugin mac-address-filtering long-address-list add <eui64>
// Description: Adds a long address to the long address filter.
// Arg: eui64 | sl_802154_long_addr_t | The long address to add.
void sl_zigbee_af_mac_address_filtering_add_long_address_command(sl_cli_command_arg_t * arguments)
{
  sl_802154_long_addr_t eui64;
  sl_zigbee_copy_eui64_arg(arguments, 0, eui64, true);
  sl_zigbee_af_mac_address_filtering_add_long_address(eui64);
}

// Command: plugin mac-address-filtering short-address-list clear
// Description: Clears the short address list filter.
void sl_zigbee_af_mac_address_filtering_clear_short_address_list(sl_cli_command_arg_t * arguments)
{
  UNUSED_VAR(arguments);
  sli_zigbee_mac_address_filtering_clear_short_filter_list();
  sl_zigbee_af_core_println("Short address list cleared.");
}

// Command: plugin mac-address-filtering long-address-list clear
// Description: Clears the long address list filter.
void sl_zigbee_af_mac_address_filtering_clear_long_address_list(sl_cli_command_arg_t * arguments)
{
  UNUSED_VAR(arguments);
  sli_zigbee_mac_address_filtering_clear_long_filter_list();
  sl_zigbee_af_core_println("Long address list cleared.");
}

// Command: plugin mac-address-filtering pan-id-list add <pan-id>
// Description: Adds a PAN ID to the filter.
// Arg: pan-id | uint16_t | The PAN ID to add to the list.
void sl_zigbee_af_mac_address_filtering_add_pan_id_command(sl_cli_command_arg_t * arguments)
{
  sl_802154_pan_id_t panId = (sl_802154_pan_id_t)sl_cli_get_argument_uint32(arguments, 0);
  sl_zigbee_af_mac_address_filtering_add_pan_id(panId);
}

// Command: plugin mac-address-filtering pan-id-list delete <index>
// Description: Deletes an entry in the PAN ID list filter.
// Arg: index | uint8_t | The index of the entry to delete.
void sl_zigbee_af_mac_address_filtering_pan_id_delete_entry(sl_cli_command_arg_t * arguments)
{
  uint8_t index = sl_cli_get_argument_uint32(arguments, 0);
  if (index >= SL_ZIGBEE_AF_PLUGIN_MAC_ADDRESS_FILTERING_PAN_ID_FILTER_LENGTH) {
    sl_zigbee_af_core_println("Error: Index is greater than or equal to list length (%d)",
                              SL_ZIGBEE_AF_PLUGIN_MAC_ADDRESS_FILTERING_PAN_ID_FILTER_LENGTH);
    return;
  }

  macAddressFilterData.panIdFilterMatchList[index].panId = BROADCAST_PAN_ID;
  sl_zigbee_af_core_println("Entry deleted.");
}

// Command: plugin mac-address-filtering pan-id-list clear
// Description: Clears the PAN ID list filter.
void sl_zigbee_af_mac_address_filtering_clear_pan_id_list(sl_cli_command_arg_t * arguments)
{
  UNUSED_VAR(arguments);
  sli_zigbee_mac_address_filtering_clear_panid_filter_list();
}

// Command: plugin mac-address-filtering pan-id-list set-blacklist/set-whitelist
// Description:  Sets the PAN_ID list filter to a blacklist/whitelist.
void sl_zigbee_af_mac_address_filtering_set_pan_id_list_type_command(sl_cli_command_arg_t * arguments)
{
  uint8_t* pointer
    = sl_zigbee_cli_get_argument_string_and_length(arguments, -1, // argument number
                                                   NULL); // returned length
  sl_zigbee_af_mac_address_filtering_set_pan_id_list_type(pointer);
}

// Command: plugin mac-address-filtering reset
// Description: Resets the state of the plugin to the default of no filtering.
void sl_zigbee_af_mac_address_filtering_reset(sl_cli_command_arg_t * arguments)
{
  UNUSED_VAR(arguments);
  // TODO: update once we port the init callback
  UNUSED_VAR(arguments);
  sl_zigbee_af_mac_address_filtering_init_cb(SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA);
  sl_zigbee_af_core_println("All filter data cleared.");
}

// Command: plugin mac-address-filtering short-address-list delete <index>
// Description: Deletes an entry in the short address list filter.
// Arg: index | uint8_t | The index of the entry to delete.
void sl_zigbee_af_mac_address_filtering_short_address_delete_entry(sl_cli_command_arg_t * arguments)
{
  uint8_t index = sl_cli_get_argument_uint32(arguments, 0);
  if (index >= SL_ZIGBEE_AF_PLUGIN_MAC_ADDRESS_FILTERING_SHORT_ADDRESS_FILTER_LENGTH) {
    sl_zigbee_af_core_println("Error: Index is greater than or equal to list length (%d)",
                              SL_ZIGBEE_AF_PLUGIN_MAC_ADDRESS_FILTERING_SHORT_ADDRESS_FILTER_LENGTH);
    return;
  }

  macAddressFilterData.macShortAddressList[index].shortAddress = SL_ZIGBEE_NULL_NODE_ID;
  sl_zigbee_af_core_println("Entry deleted.");
}

// Command: plugin mac-address-filtering long-address-list delete <index>
// Description: Deletes an entry in the long address list filter.
// Arg: index | uint8_t | The entry in the long address list to delete.
void sl_zigbee_af_mac_address_filtering_long_address_delete_entry(sl_cli_command_arg_t * arguments)
{
  uint8_t index = sl_cli_get_argument_uint32(arguments, 0);
  if (index >= SL_ZIGBEE_AF_PLUGIN_MAC_ADDRESS_FILTERING_LONG_ADDRESS_FILTER_LENGTH) {
    sl_zigbee_af_core_println("Error: Index is greater than or equal to list length (%d)",
                              SL_ZIGBEE_AF_PLUGIN_MAC_ADDRESS_FILTERING_LONG_ADDRESS_FILTER_LENGTH);
    return;
  }

  memset(macAddressFilterData.macLongAddressList[index].longAddress,
         0,
         EUI64_SIZE);
  sl_zigbee_af_core_println("Entry deleted.");
}
