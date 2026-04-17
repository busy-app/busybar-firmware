/***************************************************************************//**
 * @file
 * @brief Code to manage the mirrors via the CLI.
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
#include "app/framework/util/util.h"
#include "meter-mirror.h"

uint16_t sl_zigbee_af_simple_metering_client_remove_mirror_cb(sl_802154_long_addr_t requestingDeviceIeeeAddress);

//-----------------------------------------------------------------------------
// Globals

#include "app/util/serial/sl_zigbee_command_interpreter.h"
void sli_zigbee_af_meter_mirror_status_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  uint8_t meteringDeviceTypeAttribute = 0xFF;
  uint16_t endpoint;
  uint8_t mirrorEui[EUI64_SIZE];

  sl_zigbee_af_simple_metering_cluster_println("Known meters");
  // Go over our endpoints and find an unallocated meter mirror
  for (endpoint = SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_ENDPOINT_START;
       endpoint < SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_ENDPOINT_START + SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS;
       endpoint++) {
    bool validEndpoint;

    // Reset these every run
    memset(mirrorEui, 0xFF, EUI64_SIZE);
    meteringDeviceTypeAttribute = 0xFF;
    validEndpoint = sl_zigbee_af_meter_mirror_get_eui64_by_endpoint(endpoint,
                                                                    mirrorEui);

    if (!validEndpoint) {
      continue;
    }
    if (sl_zigbee_af_meter_mirror_is_mirror_used(endpoint)) {
      // Retrieve the meter device type, this will tell us what
      // type of meter we are looking at on this endpoint
      sl_zigbee_af_read_server_attribute(endpoint,
                                         ZCL_SIMPLE_METERING_CLUSTER_ID,
                                         ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID,
                                         &meteringDeviceTypeAttribute,
                                         sizeof(meteringDeviceTypeAttribute));
    }

    sl_zigbee_af_simple_metering_cluster_print("ep: %d  ieee:", endpoint);
    sl_zigbee_af_print_big_endian_eui64(mirrorEui);
    sl_zigbee_af_simple_metering_cluster_println("  type: 0x%02X", meteringDeviceTypeAttribute);
  }
  // Explicitly usea  function call to find the number of mirrors allocated instead
  // of just calculating it.  In case there is ever a discrepancy,
  // this will help track down any bugs.

  sl_zigbee_af_simple_metering_cluster_println("%d of %d mirrors used.",
                                               sli_zigbee_af_meter_mirror_get_mirrors_allocated(),
                                               SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS);
}

void sli_zigbee_af_meter_mirror_remove_command(sl_cli_command_arg_t *arguments)
{
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  sl_802154_long_addr_t mirrorEui64;
  bool success = sl_zigbee_af_meter_mirror_get_eui64_by_endpoint(endpoint, mirrorEui64);
  if (!success) {
    sl_zigbee_af_simple_metering_cluster_println("Error: Failed to obtain EUI64 for endpoint %d", endpoint);
    return;
  }
  sl_zigbee_af_simple_metering_client_remove_mirror_cb(mirrorEui64);
}
