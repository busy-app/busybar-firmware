/***************************************************************************//**
 * @file
 * @brief CLI for the Device Query Service plugin.
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
#include "app/framework/plugin/device-query-service/device-query-service.h"
#include "app/framework/plugin/device-database/device-database.h"

//============================================================================
// Globals

//============================================================================
// Forward declarations

extern const char* device_database_get_status_string(sl_zigbee_af_device_discovery_status_t status);

//============================================================================
void sli_zigbee_af_device_query_service_enable_disable_command(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_device_query_service_enable_disable(memcmp(arguments->argv[arguments->arg_ofs - 1], "enable", strlen("enable")) == 0);
}

void sli_zigbee_af_device_query_service_status_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  sl_802154_long_addr_t currentEui64;
  bool enabled = sl_zigbee_af_device_query_service_get_enabled_state();
  sl_zigbee_af_device_query_service_get_current_discovery_target_eui64(currentEui64);
  sl_zigbee_af_core_println("Enabled: %s", enabled ? "yes" : "no");
  sl_zigbee_af_core_print("Current Discovery Target: ");
  sl_zigbee_af_print_big_endian_eui64(currentEui64);
  sl_zigbee_af_core_println("");
  const sl_zigbee_af_device_info_t* device = sl_zigbee_af_device_database_find_device_by_eui64(currentEui64);
  sl_zigbee_af_core_println("Status: %s",
                            (device == NULL
                             ? ""
                             : device_database_get_status_string(device->status)));
}

void sli_zigbee_af_device_query_service_discover_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t eui64;
  sl_zigbee_copy_eui64_arg(arguments, 0, eui64, true);
  (void)sl_zigbee_af_device_database_erase_device(eui64);
  sl_status_t status = sl_zigbee_af_device_query_service_discover_target(eui64, 0xFF); // Node Descriptor will update device capabilities
  sl_zigbee_af_core_println("Discover device: 0x%02X", status);
}
