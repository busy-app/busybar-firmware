/***************************************************************************//**
 * @file
 * @brief Implementations for network related CLI commands.
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
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT
// This is to bring in sli_zigbee_af_permit_join() prototype.
#include "app/framework/util/af-main.h"
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
#include "test-harness-config.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
  #include "app/framework/plugin/zll-commissioning-common/zll-commissioning-common.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_PHY_2_4_SUBGHZ_JOINING_END_DEVICE_PRESENT
  #include "stack/include/stack-info.h"
#endif // SL_CATALOG_ZIGBEE_PHY_2_4_SUBGHZ_JOINING_END_DEVICE_PRESENT

uint8_t sli_zigbee_af_cli_network_index = SL_ZIGBEE_AF_DEFAULT_NETWORK_INDEX;
extern uint8_t sli_zigbee_af_extended_pan_id[];
static void initNetworkParams(sl_zigbee_network_parameters_t *networkParams,
                              sl_cli_command_arg_t *arguments)
{
  memset(networkParams, 0, sizeof(sl_zigbee_network_parameters_t));
  sl_zigbee_af_get_form_and_join_extended_pan_id_cb(networkParams->extendedPanId);
  networkParams->radioChannel = sl_cli_get_argument_uint8(arguments, 0);
  networkParams->radioTxPower = sl_cli_get_argument_int8(arguments, 1);
  networkParams->panId = sl_cli_get_argument_uint16(arguments, 2);
  if (sl_cli_get_argument_count(arguments) > 3) {
    // Map the channel page into the channel, if present.
    // Page bits: 7, 6, 5 channel bits: 4...0
    uint8_t page = sl_cli_get_argument_uint8(arguments, 3);
    networkParams->radioChannel = (uint8_t)((page << 5) | (networkParams->radioChannel & 0x1F));
  }
}

// network form <channel> <power> <panid>
void networkFormCommand(sl_cli_command_arg_t *arguments)
{
#ifdef SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK
  sl_status_t status;
  sl_zigbee_network_parameters_t networkParams;
  initNetworkParams(&networkParams, arguments);
  status = sl_zigbee_af_form_network(&networkParams);
  sl_zigbee_core_debug_print("form 0x%08X\n", status);
  sl_zigbee_af_app_flush();
#else
  (void) arguments;
  sl_zigbee_core_debug_print("only coordinators can form\n");
#endif
}

// net_join <channel> <power> <panid>
void networkJoinCommand(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  sl_zigbee_network_parameters_t networkParams = { 0 };
  initNetworkParams(&networkParams, arguments);
  status = sl_zigbee_af_join_network(&networkParams);
  UNUSED_VAR(status);
  sl_zigbee_core_debug_print("join 0x%08X\n", status);
}

// TODO: full port of this CLI command (which includes "network broad-pjoin")
// net_pjoin <time>
#if !(defined(SL_ZIGBEE_LEAF_STACK))
void networkPermitJoinCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t duration = sl_cli_get_argument_uint8(arguments, 0);
  sli_zigbee_af_permit_join(duration, NULL);
}
#endif

// network leave
void networkLeaveCommand(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  sl_status_t status = SL_STATUS_OK;
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
  // The ZTT scripts just do 'network leave', which means that ZLL tokens
  // never get cleared if the leave fails. This way will ensure that they
  // do, and that other NV data (attributes, scenes, etc.) gets reset.
  sl_zigbee_af_zll_reset_to_factory_new();
#else
  status = sl_zigbee_leave_network(SL_ZIGBEE_LEAVE_NWK_WITH_NO_OPTION);
#endif
  UNUSED_VAR(status);
  sl_zigbee_core_debug_print("%s 0x%02X\n", "leave", status);
}

// network rejoin <haveCurrentNetworkKey:1> <channelMask:4>
void networkRejoinCommand(sl_cli_command_arg_t *arguments)
{
  bool haveCurrentNetworkKey = (bool)sl_cli_get_argument_uint8(arguments, 0);
  uint32_t channelMask = sl_cli_get_argument_uint32(arguments, 1);
  if (channelMask == 0) {
    channelMask = SL_ZIGBEE_ALL_802_15_4_CHANNELS_MASK;
    // Allow the upper layer to update the rejoin mask incase needed in this callback.
    // Why do we need this callback - in SE 1.4 CCB 2637 introduced a device type that is slightly
    // different Multi-MAC Selection device called as the Multi-MAC Joining device.
    // The Joining Device shall not change the interface during rejoin but the selection device can.
    // Since this code is in library of the leaf node with the folloiwng callback it would be able
    // update mask for rejoining based on the above device types.
    #ifdef SL_CATALOG_ZIGBEE_PHY_2_4_SUBGHZ_JOINING_END_DEVICE_PRESENT
    channelMask = sl_zigbee_internal_update_multi_mac_rejoin_channel_mask_for_selection_or_joining_device_handler(channelMask);
    #endif // SL_CATALOG_ZIGBEE_PHY_2_4_SUBGHZ_JOINING_END_DEVICE_PRESENT
  }
  sl_status_t status = sl_zigbee_find_and_rejoin_network(haveCurrentNetworkKey,
                                                         channelMask,
                                                         SL_ZIGBEE_AF_REJOIN_DUE_TO_CLI_COMMAND,
                                                         SL_ZIGBEE_DEVICE_TYPE_UNCHANGED);
  UNUSED_VAR(status);
  sl_zigbee_app_debug_println("%s 0x%02X", "rejoin", status);
}

void networkRejoinDiffDeviceTypeCommand(sl_cli_command_arg_t *arguments)
{
  bool haveCurrentNetworkKey = sl_cli_get_argument_uint8(arguments, 0);
  uint32_t channelMask = sl_cli_get_argument_uint32(arguments, 1);
  uint8_t sl_zigbee_node_type = sl_cli_get_argument_uint8(arguments, 2);
  sl_status_t status = sl_zigbee_find_and_rejoin_network(haveCurrentNetworkKey,
                                                         channelMask,
                                                         SL_ZIGBEE_REJOIN_DUE_TO_APP_EVENT_1,
                                                         sl_zigbee_node_type);
  UNUSED_VAR(status);
  sl_zigbee_app_debug_println("%s 0x%02X", "rejoinDiffDeviceType", status);
}

// network extpanid <8 BYTES>
void networkExtendedPanIdCommand(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_copy_eui64_arg(arguments, 0, sli_zigbee_af_extended_pan_id, true);
  sl_zigbee_af_set_form_and_join_extended_pan_id_cb(sli_zigbee_af_extended_pan_id);
  sl_zigbee_af_app_print("ext. PAN ID: ");
  sl_zigbee_af_app_debug_exec(sl_zigbee_af_print_big_endian_eui64(sli_zigbee_af_extended_pan_id));
  sl_zigbee_app_debug_println("");
}

 #if !(defined(SL_ZIGBEE_LEAF_STACK))
// check pjoin status
void networkCheckPjoinCommand(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  uint8_t open_duration = sl_zigbee_af_get_open_network_duration_sec();
  if (open_duration > 0) {
    sl_zigbee_app_debug_println("NWK open: %u sec", open_duration);
  } else {
    sl_zigbee_app_debug_println("NWK closed");
  }
}
#endif

void findJoinableNetworkCommand(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  sl_status_t status = sl_zigbee_af_start_search_for_joinable_network();
  sl_zigbee_core_debug_println("find joinable: 0x%02X", status);
}

void findUnusedPanIdCommand(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  sl_status_t status = sl_zigbee_af_find_unused_pan_id_and_form();
  sl_zigbee_core_debug_println("find unused: 0x%02X", status);
}

void networkChangeChannelCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t channel = sl_cli_get_argument_uint8(arguments, 0);
  sl_status_t status = sl_zigbee_channel_change_request(channel);
  UNUSED_VAR(status);
  sl_zigbee_app_debug_println("Changing to channel %d: 0x%02X",
                              channel,
                              status);
}

// is defined to either 1 or 0 as a UC configuration.
#if (SL_ZIGBEE_AF_TC_SWAP_OUT_TEST == 1)
void networkInitCommand(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  sl_zigbee_network_init_struct_t networkInitStruct = {
    SL_ZIGBEE_AF_CUSTOM_NETWORK_INIT_OPTIONS   // sl_zigbee_network_init_bitmask_t value
  };
  sl_status_t status = sl_zigbee_network_init(&networkInitStruct);
  sl_zigbee_app_debug_println("Network Init returned: 0x%02X", status);
}
#else
void networkInitCommand(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
}
#endif

void networkSetCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  if (SL_ZIGBEE_SUPPORTED_NETWORKS <= index) {
    sl_zigbee_core_debug_println("invalid network index");
    return;
  }
  sli_zigbee_af_cli_network_index = index;
}

void networkIdCommand(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  sl_802154_long_addr_t eui64;
  sl_zigbee_af_get_eui64(eui64);
  sl_zigbee_core_debug_println("Short ID: 0x%04X, EUI64: ", sl_zigbee_af_get_node_id());
  sl_zigbee_af_print_big_endian_eui64(eui64);
  sl_zigbee_core_debug_println(", Pan ID: 0x%04X", sl_zigbee_af_get_pan_id());
}

void networkMultiPhyStartCommand(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  uint8_t page = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t channel = sl_cli_get_argument_uint8(arguments, 1);
  int8_t power = sl_cli_get_argument_int8(arguments, 2);
  uint8_t optionsMask = 0;

  if (sl_cli_get_argument_count(arguments) > 3) {
    optionsMask = sl_cli_get_argument_uint8(arguments, 3);
  }

  status = sl_zigbee_multi_phy_start(PHY_INDEX_PRO2PLUS, page, channel, power, optionsMask);

  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_println("Started multi-phy interface");
  } else {
    sl_zigbee_core_debug_println("Failed to %s %s 0x%02X",
                                 "start",
                                 "multi-phy interface",
                                 status);
  }
}

void networkMultiPhyStopCommand(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  sl_status_t status = sl_zigbee_multi_phy_stop(PHY_INDEX_PRO2PLUS);

  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_println("Terminated %s 0x%02X",
                                 "multi-phy interface",
                                 status);
  } else {
    sl_zigbee_core_debug_println("Failed to %s %s 0x%02X",
                                 "stop",
                                 "multi-phy interface",
                                 status);
  }
}

/* ToDo: keep or remove? needed for testing. no ezsp support. */
void changeKeepAliveModeCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t keepAliveMode = sl_cli_get_argument_uint8(arguments, 0);
  if (!sl_zigbee_set_keep_alive_mode(keepAliveMode)) {
    sl_zigbee_app_debug_println("Keep alive support enabled.");
  } else {
    sl_zigbee_app_debug_println("failed to set keep alive mode.");
  }
}

void networkChangeChildTimeoutOptionMaskCommand(sl_cli_command_arg_t *arguments)
{
  uint16_t mask = sl_cli_get_argument_uint16(arguments, 0);
  if (!sl_zigbee_set_child_timeout_option_mask(mask)) {
    sl_zigbee_app_debug_println("successfully set the child timeout option mask.");
  } else {
    sl_zigbee_app_debug_println("failed to set the child timeout option mask.");
  }
}
