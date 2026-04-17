/***************************************************************************//**
 * @file
 * @brief CLI for the Network Creator plugin.
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

#include "network-creator.h"

#include "app/util/serial/sl_zigbee_command_interpreter.h"
// -----------------------------------------------------------------------------
// CLI Command Definitions

// plugin network-creator start <centralized:1>
void sl_zigbee_af_network_creator_start_command(sl_cli_command_arg_t *args)
{
  bool centralizedSecurity = sl_cli_get_argument_uint8(args, 0);
  sl_status_t status = sl_zigbee_af_network_creator_start(centralizedSecurity);

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                            "Form",
                            status);
}

// plugin network-creator form <centralized:1> <panId:1> <radioTxPower:1> <channel:1>
void sl_zigbee_af_network_creator_form_command(sl_cli_command_arg_t *args)
{
  bool centralizedSecurity = sl_cli_get_argument_uint8(args, 0);
  sl_802154_pan_id_t panId = (sl_802154_pan_id_t)(sl_cli_get_argument_uint16(args, 1));
  int8_t radioTxPower = sl_cli_get_argument_int8(args, 2);
  uint8_t channel = sl_cli_get_argument_uint8(args, 3);

  sl_status_t status = sl_zigbee_af_network_creator_network_form(centralizedSecurity,
                                                                 panId,
                                                                 radioTxPower,
                                                                 channel);

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                            "Form",
                            status);
}

// plugin network-creator mask add <mask:1> <channel:1>
// plugin network-creator mask subtract <mask:1> <channel:1>
// plugin network-creator mask set <mask:1> <new:1>
void sl_zigbee_af_network_creator_channel_mask_command(sl_cli_command_arg_t *args)
{
  bool channelMaskIsPrimary = (sl_cli_get_argument_uint8(args, 0) == 1);
  uint32_t channelOrNewMask = sl_cli_get_argument_uint32(args, 1);
  uint32_t *channelMask = (channelMaskIsPrimary
                           ? &sli_zigbee_af_network_creator_primary_channel_mask
                           : &sli_zigbee_af_network_creator_secondary_channel_mask);

  // Check if operation is add or subtract first.
  if (memcmp(args->argv[args->arg_ofs - 1], "set", strlen("set")) != 0) {
    if (channelOrNewMask < SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER
        || channelOrNewMask > SL_ZIGBEE_MAX_802_15_4_CHANNEL_NUMBER) {
      sl_zigbee_af_core_println("%s: %s: 0x%02X",
                                SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                                "Illegal 802.15.4 channel",
                                channelOrNewMask);
    } else if (memcmp(args->argv[args->arg_ofs - 1], "add", strlen("add")) == 0) {
      *channelMask |= (1 << channelOrNewMask);
    } else {
      *channelMask &= ~(1 << channelOrNewMask);
    }
  } else {
    *channelMask = channelOrNewMask;
  }

  sl_zigbee_af_core_print("%s channel mask now: 0x%08X [",
                          (channelMaskIsPrimary ? "Primary" : "Secondary"),
                          *channelMask);
  sl_zigbee_af_print_channel_list_from_mask(*channelMask);
  sl_zigbee_af_core_println("]");
}

// plugin network-creator status
void sl_zigbee_af_network_creator_status_command(sl_cli_command_arg_t *args)
{
  (void)args;
  uint8_t i;
  uint32_t masks[] = { 0,
                       sli_zigbee_af_network_creator_primary_channel_mask,
                       sli_zigbee_af_network_creator_secondary_channel_mask };

  sl_zigbee_af_core_println("%s: %s:",
                            SL_ZIGBEE_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                            "Channel mask");
  for (i = 1; i <= 2; i++) {
    sl_zigbee_af_core_print("    (%d) 0x%08X [",
                            i,
                            masks[i]);
    sl_zigbee_af_print_channel_list_from_mask(masks[i]);
    sl_zigbee_af_core_println("]");
  }
}

// plugin network-creator stop
void sl_zigbee_af_network_creator_stop_command(sl_cli_command_arg_t *args)
{
  (void) args;
  sl_zigbee_af_network_creator_stop();
}
