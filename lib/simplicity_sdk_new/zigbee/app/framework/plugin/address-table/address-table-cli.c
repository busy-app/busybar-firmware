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
#include "app/framework/util/af-main.h"
#include "address-table.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"

#include "address-table-config.h"

//[plugin address-table set 0 {0200000000000000} 0xA3D1
void sl_zigbee_af_address_table_add_command(sl_cli_command_arg_t *arguments)
{
  uint8_t index;
  sl_802154_long_addr_t entry;
  sl_zigbee_copy_eui64_arg(arguments, 0, entry, false);// assuming there is left padding as before
  index = sl_zigbee_af_address_table_add_entry(entry);

  if (index == SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX) {
    sl_zigbee_af_core_println("Table full, entry not added");
  } else {
    sl_zigbee_af_core_println("Entry added at position 0x%02X", index);
  }
}

void sl_zigbee_af_address_table_remove_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  sl_802154_long_addr_t entry;
  sl_zigbee_copy_eui64_arg(arguments, 0, entry, false);

  status = sl_zigbee_af_address_table_remove_entry(entry);

  if (status == SL_STATUS_OK) {
    sl_zigbee_af_core_println("Entry removed");
  } else {
    sl_zigbee_af_core_println("Entry removal failed");
  }
}

void sl_zigbee_af_address_table_lookup_command(sl_cli_command_arg_t *arguments)
{
  uint8_t index;
  sl_802154_long_addr_t entry;
  sl_zigbee_copy_eui64_arg(arguments, 0, entry, false);
  index = sl_zigbee_af_address_table_lookup_by_eui64(entry);

  if (index == SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX) {
    sl_zigbee_af_core_println("Entry not found");
  } else {
    sl_zigbee_af_core_println("Found entry at position 0x%02X", index);
  }
}

// plugin address-table print
void sl_zigbee_af_address_table_print_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  uint8_t i;
  uint8_t used = 0;
  sl_zigbee_core_debug_println("#  node   eui");
  for (i = 0; i < sl_zigbee_af_get_address_table_size(); i++) {
    sl_802154_long_addr_t eui64;
    sl_802154_short_addr_t nodeId;
    sl_zigbee_get_address_table_info(i, &nodeId, eui64);
    if (nodeId != SL_ZIGBEE_TABLE_ENTRY_UNUSED_NODE_ID) {
      used++;
      sl_zigbee_core_debug_print("%d: 0x%04X ", i, nodeId);
      sl_zigbee_af_app_debug_exec(sl_zigbee_af_print_big_endian_eui64(eui64));
      sl_zigbee_core_debug_println("");
      sl_zigbee_af_app_flush();
    }
  }
  sl_zigbee_core_debug_println("%d of %d entries used.",
                               used,
                               sl_zigbee_af_get_address_table_size());
}

void sl_zigbee_af_address_table_set_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t eui64;
  sl_status_t status;
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  sl_802154_short_addr_t nodeId = sl_cli_get_argument_uint16(arguments, 2);
  sl_zigbee_copy_eui64_arg(arguments, 1, eui64, true);
  status = sl_zigbee_af_set_address_table_entry(index, eui64, nodeId);
  UNUSED_VAR(status);
  sl_zigbee_core_debug_println("set address %d: 0x%08X", index, status);
}
