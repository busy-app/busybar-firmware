/***************************************************************************//**
 * @file
 * @brief CLI for the Network Steering plugin.
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

#include "app/util/serial/sl_zigbee_command_interpreter.h"

#include "network-steering.h"
#include "network-steering-internal.h"

// -----------------------------------------------------------------------------
// Helper macros and functions

static void addOrSubtractChannel(uint8_t maskToAddTo,
                                 uint8_t channelToAdd,
                                 bool operationIsAdd)
{
  if (channelToAdd < SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER
      || channelToAdd > SL_ZIGBEE_MAX_802_15_4_CHANNEL_NUMBER) {
    sl_zigbee_af_core_println("Channel not valid: %d", channelToAdd);
  } else if (maskToAddTo == 1) {
    if (operationIsAdd) {
      SETBIT(sli_zigbee_af_network_steering_primary_channel_mask, channelToAdd);
    } else {
      CLEARBIT(sli_zigbee_af_network_steering_primary_channel_mask, channelToAdd);
    }

    sl_zigbee_af_core_println("%s mask now 0x%08X",
                              "Primary",
                              sli_zigbee_af_network_steering_primary_channel_mask);
  } else if (maskToAddTo == 2) {
    if (operationIsAdd) {
      SETBIT(sli_zigbee_af_network_steering_secondary_channel_mask, channelToAdd);
    } else {
      CLEARBIT(sli_zigbee_af_network_steering_secondary_channel_mask, channelToAdd);
    }

    sl_zigbee_af_core_println("%s mask now 0x%08X",
                              "Secondary",
                              sli_zigbee_af_network_steering_secondary_channel_mask);
  } else {
    sl_zigbee_af_core_println("Mask not valid: %d", maskToAddTo);
  }
}

#include "app/util/serial/sl_zigbee_command_interpreter.h"

void sl_zigbee_af_network_steering_start_command(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_af_network_steering_options_mask = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_network_steering_start();
}
// // -----------------------------------------------------------------------------
// // Command definitions

// // plugin network-steering mask add <[1=primary|2=secondary]:1> <channel:1>
// // plugin network-steering mask subtract <[1=primary|2=secondary]:1> <channel:1>
void sl_zigbee_af_network_steering_channel_add_or_subtract_command(sl_cli_command_arg_t *arguments)
{
  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  bool operationIsAdd = memcmp(sl_cli_get_command_string(arguments, position), "add", strlen("add")) == 0;
  uint8_t maskToAddTo  = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t channelToAdd = sl_cli_get_argument_uint8(arguments, 1);

  addOrSubtractChannel(maskToAddTo,
                       channelToAdd,
                       operationIsAdd);
}

void sl_zigbee_af_network_steering_channel_set_command(sl_cli_command_arg_t *arguments)
{
  bool maskIsPrimary = (sl_cli_get_argument_uint8(arguments, 0) == 1);
  uint32_t value = sl_cli_get_argument_uint32(arguments, 1);
  uint32_t *mask = (maskIsPrimary
                    ? &sli_zigbee_af_network_steering_primary_channel_mask
                    : &sli_zigbee_af_network_steering_secondary_channel_mask);

  sl_zigbee_af_core_println("%s: Set %s mask to 0x%08X",
                            sli_zigbee_af_network_steering_plugin_name,
                            (maskIsPrimary ? "primary" : "secondary"),
                            (*mask = value));
}

// // plugin network-steering status
void sl_zigbee_af_network_steering_status_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_af_core_println("%s: %s:",
                            sli_zigbee_af_network_steering_plugin_name,
                            "Status");

  sl_zigbee_af_core_println("Channel mask:");
  sl_zigbee_af_core_print("    (1) 0x%08X [",
                          sli_zigbee_af_network_steering_primary_channel_mask);
  sl_zigbee_af_print_channel_list_from_mask(sli_zigbee_af_network_steering_primary_channel_mask);
  sl_zigbee_af_core_println("]");
  sl_zigbee_af_core_print("    (2) 0x%08X [",
                          sli_zigbee_af_network_steering_secondary_channel_mask);
  sl_zigbee_af_print_channel_list_from_mask(sli_zigbee_af_network_steering_secondary_channel_mask);
  sl_zigbee_af_core_println("]");

  sl_zigbee_af_core_println("State: 0x%02X (%s)",
                            sli_zigbee_af_network_steering_state,
                            sli_zigbee_af_network_steering_stateNames[sli_zigbee_af_network_steering_state]);
#ifndef OPTIMIZE_SCANS
  sl_zigbee_af_core_println("Pan ID index: %d",
                            sli_zigbee_af_network_steering_pan_id_index);
#endif //  OPTIMIZE_SCANS
  sl_zigbee_af_core_println("Current channel: %d",
                            sli_zigbee_af_network_steering_get_current_channel());
  sl_zigbee_af_core_println("Total beacons: %d",
                            sli_zigbee_af_network_steering_total_beacons);
  sl_zigbee_af_core_println("Join attempts: %d",
                            sli_zigbee_af_network_steering_join_attempts);
  sl_zigbee_af_core_println("Network state: 0x%02X",
                            sl_zigbee_af_network_state());
}

void sl_zigbee_af_network_steering_stop_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            sli_zigbee_af_network_steering_plugin_name,
                            "Stop",
                            sl_zigbee_af_network_steering_stop());
}

void sl_zigbee_af_network_steering_set_preconfigured_key_command(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_key_data_t keyData;
  size_t len = 16;
  uint8_t *ptr_string = sl_cli_get_argument_hex(arguments, 0, &len);
  memset(keyData.contents, 0, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  memmove(keyData.contents, ptr_string, SL_ZIGBEE_ENCRYPTION_KEY_SIZE); // Is the padding correct?

  sli_zigbee_af_network_steering_set_configured_key(keyData.contents, true);
}
