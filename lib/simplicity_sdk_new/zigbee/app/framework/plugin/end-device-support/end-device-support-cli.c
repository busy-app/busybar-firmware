/***************************************************************************//**
 * @file
 * @brief CLI for the Polling plugin.
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
#include "app/framework/util/common.h"
#include "end-device-support.h"

const char * pollControlStrings[] = {
  "SL_ZIGBEE_AF_LONG_POLL",
  "SL_ZIGBEE_AF_SHORT_POLL",
};

#include "app/util/serial/sl_zigbee_command_interpreter.h"

void sl_zigbee_af_end_device_support_status_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  const char * names[] = {
    SL_ZIGBEE_AF_GENERATED_NETWORK_STRINGS
  };
  uint8_t i;

  sl_zigbee_af_core_println("End Device Poll Information");
  sl_zigbee_af_core_println("EMBER_END_DEVICE_TIMEOUT:       %d", SL_ZIGBEE_END_DEVICE_POLL_TIMEOUT);
  sl_zigbee_af_core_println("Poll completed callback: %s",
                            (sli_zigbee_af_enable_poll_completed_callback
                             ? "yes"
                             : "no"));
  sl_zigbee_af_core_flush();

  for (i = 0; i < SL_ZIGBEE_SUPPORTED_NETWORKS; i++) {
    (void) sl_zigbee_af_push_network_index(i);
    if (sli_zigbee_af_pro_is_current_network()
        && SL_ZIGBEE_END_DEVICE <= sli_zigbee_af_current_zigbee_pro_network->nodeType) {
      sl_zigbee_af_core_println("nwk %d [%s]", i, names[i]);
      sl_zigbee_af_core_println("  Current Poll Interval (qs):   %ld",
                                sl_zigbee_af_get_current_poll_interval_qs_cb());
      sl_zigbee_af_core_println("  Long Poll Interval (qs):      %ld",
                                sl_zigbee_af_get_long_poll_interval_qs_cb());
      if (SL_ZIGBEE_SLEEPY_END_DEVICE <= sli_zigbee_af_current_zigbee_pro_network->nodeType) {
        sl_zigbee_af_core_println("  Short Poll Interval (qs):     %ld",
                                  sl_zigbee_af_get_short_poll_interval_qs_cb());
        sl_zigbee_af_core_flush();
        sl_zigbee_af_core_println("  Wake Timeout (qs):            %ld",
                                  sl_zigbee_af_get_wake_timeout_qs_cb());
        sl_zigbee_af_core_flush();
        sl_zigbee_af_core_println("  Wake Timeout Bitmask:         0x%08X",
                                  sl_zigbee_af_get_wake_timeout_bitmask_cb());
        sl_zigbee_af_core_flush();
        sl_zigbee_af_core_println("  Current App Tasks:            0x%08X",
                                  sl_zigbee_af_get_current_app_tasks_cb());
        sl_zigbee_af_core_println("  Current Poll Control          %s",
                                  pollControlStrings[sl_zigbee_af_get_current_poll_control_cb()]);
        sl_zigbee_af_core_flush();
        sl_zigbee_af_core_println("  Default Poll Control          %s",
                                  pollControlStrings[sl_zigbee_af_get_default_poll_control_cb()]);
        sl_zigbee_af_core_flush();
      }
    }
    (void) sl_zigbee_af_pop_network_index();
  }
}

void sl_zigbee_af_end_device_support_force_short_poll_command(sl_cli_command_arg_t *arguments)
{
  bool shortPoll = sl_cli_get_argument_uint8(arguments, 0);
  if (shortPoll) {
    sl_zigbee_af_core_println("Forcing device to short poll");
    sl_zigbee_af_add_to_current_app_tasks_cb(SL_ZIGBEE_AF_FORCE_SHORT_POLL);
  } else {
    sl_zigbee_af_core_println("Allowing device to long poll");
    sl_zigbee_af_remove_from_current_app_tasks_cb(SL_ZIGBEE_AF_FORCE_SHORT_POLL);
  }
}

void sl_zigbee_af_end_device_support_poll_completed_callback_command(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_af_enable_poll_completed_callback = sl_cli_get_argument_uint8(arguments, 0);
}
