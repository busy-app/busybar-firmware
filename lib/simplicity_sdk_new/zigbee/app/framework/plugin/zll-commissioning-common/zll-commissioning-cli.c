/***************************************************************************//**
 * @file
 * @brief CLI for the ZLL Commissioning Common plugin.
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
#endif
// TODO: EMZIGBEE-6719
#include "zll-commissioning-common.h"

#ifdef SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_CLIENT_PRESENT
#include "zll-commissioning-client.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_SERVER_PRESENT
#include "zll-commissioning-server.h"
#endif

//---Forward Decleration
static void printChannels(sl_cli_command_arg_t *arguments);
static uint32_t channelMasks[] = {
  0x02108800UL, // standard (11, 15, 20, 25)
  0x04211000UL, // +1 (12, 16, 21, 26)
  0x004A2000UL, // +2 (13, 17, 22, 19)
  0x01844000UL, // +3 (14, 18, 23, 24)
  0x07FFF800UL, // all (11--26)
};

// Internal APIs.
extern sl_status_t sli_zigbee_af_zll_form_network(uint8_t channel, int8_t power, sl_802154_pan_id_t panId);
extern sl_zigbee_zll_policy_t sli_zigbee_af_zll_get_policy(void);

// plugin zll-commissioning form <channel:1> <power:1> <pan id:2>
void formNetwork(sl_cli_command_arg_t *arguments)
{
  sl_status_t status = sli_zigbee_af_zll_form_network(sl_cli_get_argument_uint8(arguments, 0),       // channel
                                                      sl_cli_get_argument_uint16(arguments, 1), // power
                                                      (sl_802154_pan_id_t)sl_cli_get_argument_uint16(arguments, 2)); // pan id
  sl_zigbee_af_app_println("%s 0x%02X", "form", status);
}

// Leaving intact for legacy CLI purposes, but for generated CLI, the options have been moved to
// scanTouchLink().

// plugin zll-commissioning link [device|identify|reset]
void initiateTouchLink(sl_cli_command_arg_t *arguments)
{
#ifdef SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
  sl_status_t status;
  switch (sl_cli_get_argument_count(arguments)) {
    case 0:
      status = sl_zigbee_af_zll_initiate_touch_link();
      sl_zigbee_af_app_println("%s 0x%02X", "touch link", status);
      return;
    case 1:
      if (memcmp(arguments->argv[arguments->arg_ofs - 1], "device", strlen("device")) == 0 ) {
        status = sl_zigbee_af_zll_device_information_request();
        sl_zigbee_af_app_println("%s 0x%02X", "device information", status);
        return;
      } else if (memcmp(arguments->argv[arguments->arg_ofs - 1], "identify", strlen("identify")) == 0 ) {
        status = sl_zigbee_af_zll_identify_request();
        sl_zigbee_af_app_println("%s 0x%02X", "identify", status);
        return;
      }
      if (memcmp(arguments->argv[arguments->arg_ofs - 1], "reset", strlen("reset")) == 0 ) {
        status = sl_zigbee_af_zll_reset_to_factory_new_request();
        sl_zigbee_af_app_println("%s 0x%02X", "reset to factory new", status);
        return;
      }
      // -1 because we're smarter than command-interpreter2.
      // switch (*arguments->argv[arguments->arg_ofs - 1]) {
      //   case 'd':
      //     status = sl_zigbee_af_zll_device_information_request();
      //     sl_zigbee_af_app_println("%s 0x%02X", "device information", status);
      //     return;
      //   case 'i':
      //     status = sl_zigbee_af_zll_identify_request();
      //     sl_zigbee_af_app_println("%s 0x%02X", "identify", status);
      //     return;
      //   case 'r':
      //     status = sl_zigbee_af_zll_reset_to_factory_new_request();
      //     sl_zigbee_af_app_println("%s 0x%02X", "reset to factory new", status);
      //     return;
      //   default:
      //     return;
      //}
  }
  sl_zigbee_af_app_println("Usage:");
  sl_zigbee_af_app_println("plugin zll-commissioning link");
  sl_zigbee_af_app_println("plugin zll-commissioning link device");
  sl_zigbee_af_app_println("plugin zll-commissioning link identify");
  sl_zigbee_af_app_println("plugin zll-commissioning link reset");
#else
  (void)arguments;
  sl_zigbee_af_app_println("zll commissioning client plugin not found");
#endif
}

// Generated CLI version of "link" commands

// plugin zll-commissioning scan [device|identify|reset]
void scanTouchLink(sl_cli_command_arg_t *arguments)
{
#ifdef SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
  sl_status_t status;
  if (memcmp(arguments->argv[arguments->arg_ofs - 1], "device", strlen("device")) == 0 ) {
    status = sl_zigbee_af_zll_device_information_request();
    sl_zigbee_af_app_println("%s 0x%02X", "device information", status);
    return;
  } else if (memcmp(arguments->argv[arguments->arg_ofs - 1], "identify", strlen("identify")) == 0 ) {
    status = sl_zigbee_af_zll_identify_request();
    sl_zigbee_af_app_println("%s 0x%02X", "identify", status);
    return;
  }
  if (memcmp(arguments->argv[arguments->arg_ofs - 1], "reset", strlen("reset")) == 0 ) {
    status = sl_zigbee_af_zll_reset_to_factory_new_request();
    sl_zigbee_af_app_println("%s 0x%02X", "reset to factory new", status);
    return;
  }
  // switch (*(arguments->argv[arguments->arg_ofs - 1])) {
  //   case 'd':
  //     status = sl_zigbee_af_zll_device_information_request();
  //     sl_zigbee_af_app_println("%s 0x%02X", "device information", status);
  //     return;
  //   case 'i':
  //     status = sl_zigbee_af_zll_identify_request();
  //     sl_zigbee_af_app_println("%s 0x%02X", "identify", status);
  //     return;
  //   case 'r':
  //     status = sl_zigbee_af_zll_reset_to_factory_new_request();
  //     sl_zigbee_af_app_println("%s 0x%02X", "reset to factory new", status);
  //     return;
  // }
#else
  (void)arguments;
  sl_zigbee_af_app_println("zll commissioning client plugin not found");
#endif
}

// plugin zll-commissioning abort
void abortTouchLink(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
#ifdef SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
  sl_zigbee_af_zll_abort_touch_link();
  sl_zigbee_af_app_println("%s", "touch link aborted");
#else
  sl_zigbee_af_app_println("zll commissioning client plugin not found");
#endif
}

// plugin zll-commissioning info <destination:2> <src endpoint:1> <dst endpoint:1>
void endpointInformation(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t eui64;
  sl_status_t status = SL_STATUS_INVALID_PARAMETER;
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t index = sl_zigbee_af_index_from_endpoint(endpoint);
  if (index != 0xFF
#if ZCL_FIXED_ENDPOINT_COUNT > 0
      && index < ZCL_FIXED_ENDPOINT_COUNT
#endif
      ) {
    // If the profile interop bit in the ZllInformation bitmask is cleared,
    // then we set the profile ID in our response to the ZLL profile ID.
    // If the bit is set, then we use the profile ID specified for the endpoint.
    uint8_t deviceVersion = sl_zigbee_af_device_version_from_index(index);
    sl_zigbee_tok_type_stack_zll_data_t token;
    sl_zigbee_zll_get_token_stack_zll_data(&token);
    sl_zigbee_af_profile_id_t profileId = (token.bitmask & SL_ZIGBEE_ZLL_STATE_PROFILE_INTEROP
                                           ? sl_zigbee_af_profile_id_from_index(index)
                                           : SL_ZIGBEE_ZLL_PROFILE_ID);
    sl_zigbee_af_get_eui64(eui64);
    sl_zigbee_af_fill_command_zll_commissioning_cluster_endpoint_information(eui64,
                                                                             sl_zigbee_af_get_node_id(),
                                                                             endpoint,
                                                                             profileId,
                                                                             sl_zigbee_af_device_id_from_index(index),
                                                                             deviceVersion);
    sl_zigbee_af_set_command_endpoints(endpoint,
                                       sl_cli_get_argument_uint8(arguments, 2));
    status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                               sl_cli_get_argument_uint16(arguments, 0));
  }
  sl_zigbee_af_app_println("%s 0x%02X", "endpoint information", status);
}

// plugin zll-commissioning groups <destination:2> <src endpoint:1> <dst endpoint:1> <startIndex:1>
void getGroupIdentifiersRequest(sl_cli_command_arg_t *arguments)
{
  sl_status_t status = SL_STATUS_INVALID_PARAMETER;
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t index = sl_zigbee_af_index_from_endpoint(endpoint);
  if (index != 0xFF) {
    sl_zigbee_af_fill_command_zll_commissioning_cluster_get_group_identifiers_request(sl_cli_get_argument_uint8(arguments, 3));
    sl_zigbee_af_set_command_endpoints(endpoint,
                                       sl_cli_get_argument_uint8(arguments, 2));
    status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                               sl_cli_get_argument_uint16(arguments, 0));
  }
  sl_zigbee_af_app_println("%s 0x%02X", "get group identifiers", status);
}

// plugin zll-commissioning endpoints <destination:2> <src endpoint:1> <dst endpoint:1> <startIndex:1>
void getEndpointListRequest(sl_cli_command_arg_t *arguments)
{
  sl_status_t status = SL_STATUS_INVALID_PARAMETER;
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t index = sl_zigbee_af_index_from_endpoint(endpoint);
  if (index != 0xFF) {
    sl_zigbee_af_fill_command_zll_commissioning_cluster_get_endpoint_list_request(sl_cli_get_argument_uint8(arguments, 3));
    sl_zigbee_af_set_command_endpoints(endpoint,
                                       sl_cli_get_argument_uint8(arguments, 2));
    status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                               sl_cli_get_argument_uint16(arguments, 0));
  }
  sl_zigbee_af_app_println("%s 0x%02X", "get endpoint list", status);
}

// plugin zll-commissioning set-rx-on <duration:4>
void setRxOn(sl_cli_command_arg_t *arguments)
{
  uint32_t duration = (uint32_t)sl_cli_get_argument_uint32(arguments, 0);
  sl_zigbee_zll_set_rx_on_when_idle(duration);
  sl_zigbee_af_app_println("%s %s %d %s", "Rx-On-When-Idle", "set for", duration, "ms");
}

// plugin zll-commissioning cancel-rx-on
void cancelRxOn(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_zll_cancel_rx_on_when_idle(); // cancel rx on
  sl_zigbee_af_app_println("%s %s", "Rx-On-When-Idle", "cancelled");
}

// plugin zll-commissioning rx-on-active
void rxOnStatus(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  bool status = sl_zigbee_zll_rx_on_when_idle_get_active();
  sl_zigbee_af_app_println("%s %s", "Rx-On-When-Idle", status ? "active" : "inactive");
}

void printZllTokens(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_tok_type_stack_zll_data_t token;
  sl_zigbee_tok_type_stack_zll_security_t security;
  sl_zigbee_zll_get_token_stack_zll_data(&token);
  sl_zigbee_zll_get_token_stack_zll_security(&security);

  sl_zigbee_af_app_flush();
  sl_zigbee_af_app_println("ZLL Tokens");
  sl_zigbee_af_app_println("Bitmask: 0x%08X", token.bitmask);
  sl_zigbee_af_app_println("Free Node IDs,  Min: 0x%04X, Max: 0x%04X",
                           token.freeNodeIdMin,
                           token.freeNodeIdMax);
  sl_zigbee_af_app_flush();
  sl_zigbee_af_app_println("Free Group IDs, Min: 0x%04X, Max: 0x%04X",
                           token.freeGroupIdMin,
                           token.freeGroupIdMax);
  sl_zigbee_af_app_flush();
  sl_zigbee_af_app_println("My Group ID Min: 0x%04X",
                           token.myGroupIdMin);
  sl_zigbee_af_app_flush();
  sl_zigbee_af_app_println("RSSI Correction: %d",
                           token.rssiCorrection);
  sl_zigbee_af_app_flush();

  sl_zigbee_af_app_println("Security Bitmask: 0x%08X", security.bitmask);
  sl_zigbee_af_app_flush();
  sl_zigbee_af_app_println("Security Key Index: %d", security.keyIndex);
  sl_zigbee_af_app_flush();
  sl_zigbee_af_app_print("Security Encryption Key: ");
  sl_zigbee_af_app_debug_exec(sl_zigbee_af_print_zigbee_key(security.encryptionKey));
  sl_zigbee_af_app_println("");
  sl_zigbee_af_app_flush();
}

void setScanChannel(sl_cli_command_arg_t *arguments)
{
  uint8_t channel = sl_cli_get_argument_uint8(arguments, 0);

  if (channel == 0) {
    sl_zigbee_set_zll_primary_channel_mask(0);
  } else {
    if (channel > SL_ZIGBEE_MAX_802_15_4_CHANNEL_NUMBER
        || channel < SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER) {
      sl_zigbee_af_app_println("Invalid channel %d", channel);
      return;
    }
    sl_zigbee_set_zll_primary_channel_mask(1 << channel);

#ifdef SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_SERVER
    if (sl_zigbee_af_network_state() == SL_ZIGBEE_NO_NETWORK) {
      // If we are not joined, then we also change the default channel.
      sl_zigbee_set_logical_and_radio_channel(channel);
      sl_zigbee_af_app_println("Setting default channel to %d", channel);
    }
#endif
  }

  printChannels(arguments);
}

void setSecondaryScanChannel(sl_cli_command_arg_t *arguments)
{
  uint8_t channel = sl_cli_get_argument_uint8(arguments, 0);

  if (channel == 0) {
    sl_zigbee_set_zll_secondary_channel_mask(0);
  } else {
    if (channel > SL_ZIGBEE_MAX_802_15_4_CHANNEL_NUMBER
        || channel < SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER) {
      sl_zigbee_af_app_println("Invalid channel %d", channel);
      return;
    }
    sl_zigbee_set_zll_secondary_channel_mask(1 << channel);
  }
  printChannels(arguments);
}

void setScanMask(sl_cli_command_arg_t *arguments)
{
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  if (COUNTOF(channelMasks) <= index) {
    sl_zigbee_af_app_println("Invalid channel mask index %d", index);
    return;
  }
  sl_zigbee_set_zll_primary_channel_mask(channelMasks[index]);
  printChannels(arguments);
}

void setIdentifyDuration(sl_cli_command_arg_t *arguments)
{
#ifdef SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
  sli_zigbee_af_zll_identify_duration_sec = sl_cli_get_argument_uint16(arguments, 0);
  sl_zigbee_af_app_println("%s %d", "Identify duration set to", sli_zigbee_af_zll_identify_duration_sec);
#else // !SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
  (void)arguments;
#endif // SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
}

void statusCommand(sl_cli_command_arg_t *arguments)
{
  printChannels(arguments);
  printZllTokens(arguments);

  // Use the Ember API, not the framework API, for host-side
  sl_zigbee_af_app_println("%s %d", "Current channel:", sl_zigbee_get_radio_channel());
  sl_zigbee_af_app_println("%s %d", "Touchlinking policy:", sli_zigbee_af_zll_get_policy());
}

static void printChannels(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_af_app_print("%s channels: ", "Primary");
  sl_zigbee_af_app_debug_exec(sl_zigbee_af_print_channel_list_from_mask(sl_zigbee_get_zll_primary_channel_mask()));
  sl_zigbee_af_app_println(" (0x%08X)", sl_zigbee_get_zll_primary_channel_mask());
#if (SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_SCAN_SECONDARY_CHANNELS == 1)
  sl_zigbee_af_app_print("%s channels: ", "Secondary");
  sl_zigbee_af_app_debug_exec(sl_zigbee_af_print_channel_list_from_mask(sl_zigbee_get_zll_secondary_channel_mask()));
  sl_zigbee_af_app_println(" (0x%08X)", sl_zigbee_get_zll_secondary_channel_mask());
#endif
}

void joinable(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
#if (defined SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_NETWORK) || (defined SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_NETWORK_PRESENT)
  sl_status_t status = sl_zigbee_af_start_search_for_joinable_network_cb();
  sl_zigbee_af_app_println("%s 0x%02X", "joinable", status);
#else
  sl_zigbee_af_app_println("zll commissioning network plugin not found");
#endif
}

void unused(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
#if (defined SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_NETWORK) || (defined SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_NETWORK_PRESENT)
  sl_status_t status = sl_zigbee_af_find_unused_pan_id_and_form_cb();
  sl_zigbee_af_app_println("%s 0x%02X", "unused", status);
#else
  sl_zigbee_af_app_println("zll commissioning network plugin not found");
#endif
}

void resetToFactoryNew(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  sl_zigbee_af_zll_reset_to_factory_new();
}

void noTouchlinkForNFN(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
#ifdef SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_SERVER
  sl_zigbee_af_zll_no_touchlink_for_nfn();
  sl_zigbee_af_app_println("Touchlink stealing disabled");
#else
  sl_zigbee_af_app_println("zll commissioning server plugin not found");
#endif
}

void noResetForNFN(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
#ifdef SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_SERVER
  sl_zigbee_af_zll_no_reset_for_nfn();
  sl_zigbee_af_app_println("Touchlink remote reset disabled");
#else
  sl_zigbee_af_app_println("zll commissioning server plugin not found");
#endif
}

void enable(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_af_zll_enable();
  sl_zigbee_af_app_println("Touchlinking enabled");
}

void disable(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_af_zll_disable();
  sl_zigbee_af_app_println("Touchlinking disabled");
}
