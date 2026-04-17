/***************************************************************************//**
 * @file
 * @brief This file implements the radio config commands for sl_rail_test
 *   applications.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include <string.h>
#include <stdio.h>

#include "response_print.h"

#include "sl_rail.h"

#include "sl_rail_ble.h"
#include "sl_rail_ieee802154.h"
#include "sl_rail_mfm.h"
#include "sl_rail_zwave.h"
#if RAIL_SUPPORTS_PROTOCOL_BTC
#include "sl_rail_btc.h"
#endif
#if SL_RAIL_SUPPORTS_PROTOCOL_ANT
#include "sl_rail_ant.h"
#endif

#include "app_common.h"
#include "sl_rail_util_pa_config.h"
#include "sl_rail_util_init.h"
#include "sli_rail_util_callbacks.h"

#if SL_RAIL_UTIL_INIT_RADIO_CONFIG_SUPPORT_INST0_ENABLE
  #include "rail_config.h"
#endif // SL_RAIL_UTIL_INIT_RADIO_CONFIG_SUPPORT_INST0_ENABLE

// Channel Config Selection Variable
uint8_t configIndex = 0;

const char * const paStrings[] = SL_RAIL_TX_POWER_MODE_NAMES;
const char * const paStrings2x[] = RAIL_TX_POWER_MODE_NAMES; // For RAIL 2.x compat

/******************************************************************************
 * Command Line Interface functions
 *****************************************************************************/
void getConfigIndex(sl_cli_command_arg_t *args)
{
#if SL_RAIL_UTIL_INIT_RADIO_CONFIG_SUPPORT_INST0_ENABLE
  responsePrint(sl_cli_get_command_string(args, 0), "configIndex:%d", configIndex);
#else
  responsePrintError(sl_cli_get_command_string(args, 0), 0x22, "External radio config support not enabled");
#endif // SL_RAIL_UTIL_INIT_RADIO_CONFIG_SUPPORT_INST0_ENABLE
}

void setConfigIndex(sl_cli_command_arg_t *args)
{
#if SL_RAIL_UTIL_INIT_RADIO_CONFIG_SUPPORT_INST0_ENABLE
  uint8_t proposedIndex = sl_cli_get_argument_uint8(args, 0);

  // Be sure that the proposed index is valid. Scan through all possible
  // indexes and check for the last NULL parameter since you can't
  // use sizeof on an extern-ed array without an explicit index.
  for (uint8_t i = 0; i <= proposedIndex; i++) {
    if (channelConfigs[i] == NULL) {
      responsePrintError(sl_cli_get_command_string(args, 0), 0x11,
                         "Invalid radio config index '%d'",
                         proposedIndex);
      return;
    }
  }

  sl_rail_idle(railHandle, SL_RAIL_IDLE_ABORT, true);
  // Load the channel configuration for the specified index.
  sl_rail_status_t status
    = sl_rail_config_channels(railHandle,
                              (const sl_rail_channel_config_t *)channelConfigs[proposedIndex],
                              (sl_rail_radio_config_changed_callback_t)(void *)
                              &sli_rail_util_on_channel_config_change);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    responsePrintError(sl_cli_get_command_string(args, 0), status,
                       "Failed to configure configIndex:%d", proposedIndex);
    return;
  }
  if (sl_cli_get_argument_count(args) >= 2) {
    // A channel is provided, try to use it.
    uint16_t proposedChannel = sl_cli_get_argument_uint16(args, 1);
    // Make sure this is a valid channel. if not, reset channel configuration.
    if (sl_rail_is_valid_channel(railHandle, proposedChannel)
        != SL_RAIL_STATUS_NO_ERROR) {
      (void) sl_rail_config_channels((sl_rail_handle_t)railHandle,
                                     (const sl_rail_channel_config_t *)channelConfigs[configIndex],
                                     (sl_rail_radio_config_changed_callback_t)(void *)
                                     &sli_rail_util_on_channel_config_change);
      responsePrintError(sl_cli_get_command_string(args, 0), 0x11, "Invalid channel '%d'", proposedChannel);
      return;
    }
    channel = proposedChannel;
  } else {
    // No channel is provided, use the first available one.
    channel = sl_rail_get_first_channel(railHandle,
                                        (const sl_rail_channel_config_t *)channelConfigs[proposedIndex]);
  }
  configIndex = proposedIndex;
  if (channel != SL_RAIL_CHANNEL_INVALID) {
    if (receiveModeEnabled) {
      (void) sl_rail_start_rx(railHandle, channel, NULL);
    } else {
      // Prepare the channel because some commands expect it to be in place
      (void) sl_rail_prepare_channel(railHandle, channel);
    }
  }
  responsePrint(sl_cli_get_command_string(args, 0), "configIndex:%d,channel:%d",
                configIndex,
                channel);
#else // !SL_RAIL_UTIL_INIT_RADIO_CONFIG_SUPPORT_INST0_ENABLE
  responsePrintError(sl_cli_get_command_string(args, 0), 0x22, "External radio config support not enabled");
#endif // SL_RAIL_UTIL_INIT_RADIO_CONFIG_SUPPORT_INST0_ENABLE
}

void setTransitionTime(sl_cli_command_arg_t *args)
{
  (void) sl_rail_set_transition_time(SL_RAIL_EFR32_HANDLE,
                                     (sl_rail_time_t)sl_cli_get_argument_uint32(args, 0));
  responsePrint(sl_cli_get_command_string(args, 0),
                "DMP transition time:%d,"
                "Status:Success",
                sl_rail_get_transition_time(railHandle));
}

void getTransitionTime(sl_cli_command_arg_t *args)
{
  responsePrint(sl_cli_get_command_string(args, 0),
                "DMP transition time:%d",
                sl_rail_get_transition_time(railHandle));
}

void getChannelConfigEntry(sl_cli_command_arg_t *args)
{
#if SL_RAIL_UTIL_INIT_RADIO_CONFIG_SUPPORT_INST0_ENABLE
  uint8_t channelConfigIndex = sl_cli_get_argument_uint8(args, 0);
  uint8_t channelConfigEntryIndex = sl_cli_get_argument_uint8(args, 1);

  // Check if channelConfigs is empty
  if (channelConfigs[0] == NULL) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x11,
                       "Empty channelConfigs");
    return;
  }

  // Get number of channelConfig
  uint8_t maxChannelConfigIndex;
  for (maxChannelConfigIndex = 1; maxChannelConfigIndex < 0xFFU; maxChannelConfigIndex++) {
    if (channelConfigs[maxChannelConfigIndex] == NULL) {
      maxChannelConfigIndex--;
      break;
    }
  }

  // Get number of channelConfigEntry if valid channelConfigIndex
  if (channelConfigIndex > maxChannelConfigIndex) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x11,
                       "Invalid channelConfigIndex '%u', maxChannelConfigIndex:%u",
                       channelConfigIndex, maxChannelConfigIndex);
    return;
  }
  uint8_t maxChannelConfigEntryIndex = channelConfigs[channelConfigIndex]->length - 1;

  // Get parameters if valid channelConfigIndex & channelConfigEntryIndex
  if (channelConfigEntryIndex > maxChannelConfigEntryIndex) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x11,
                       "Invalid channelConfigEntryIndex '%u', maxChannelConfigEntryIndex:%u",
                       channelConfigEntryIndex, maxChannelConfigEntryIndex);
    return;
  }
  const sl_rail_channel_config_entry_t* entry = (sl_rail_channel_config_entry_t *)&(channelConfigs[channelConfigIndex]->configs[channelConfigEntryIndex]);
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
  const uint8_t *stackInfo = entry->p_stack_info;
  if (stackInfo == NULL) {
    responsePrint(sl_cli_get_command_string(args, 0),
                  "maxChannelConfigIndex:%d,maxChannelConfigEntryIndex:%d,baseFrequency:%d,channelSpacing:%d,"
                  "physicalChannelOffset:%d,channelNumberStart:%d,channelNumberEnd:%d,maxPower:%d,protocolId:%s,phyModeId:%s",
                  maxChannelConfigIndex, maxChannelConfigEntryIndex, entry->base_frequency_hz, entry->channel_spacing_hz,
                  entry->physical_channel_offset, entry->channel_number_start, entry->channel_number_end, entry->max_power_ddbm, "N/A", "N/A");
  } else {
    responsePrint(sl_cli_get_command_string(args, 0),
                  "maxChannelConfigIndex:%d,maxChannelConfigEntryIndex:%d,baseFrequency:%d,channelSpacing:%d,"
                  "physicalChannelOffset:%d,channelNumberStart:%d,channelNumberEnd:%d,maxPower:%d,protocolId:%d,phyModeId:%d",
                  maxChannelConfigIndex, maxChannelConfigEntryIndex, entry->base_frequency_hz, entry->channel_spacing_hz,
                  entry->physical_channel_offset, entry->channel_number_start, entry->channel_number_end, entry->max_power_ddbm, stackInfo[0], stackInfo[1]);
  }
#else // RADIO_CONFIG_ENABLE_STACK_INFO
  responsePrint(sl_cli_get_command_string(args, 0),
                "maxChannelConfigIndex:%d,maxChannelConfigEntryIndex:%d,baseFrequency:%d,channelSpacing:%d,"
                "physicalChannelOffset:%d,channelNumberStart:%d,channelNumberEnd:%d,maxPower:%d",
                maxChannelConfigIndex, maxChannelConfigEntryIndex, entry->base_frequency_hz, entry->channel_spacing_hz,
                entry->physical_channel_offset, entry->channel_number_start, entry->channel_number_end, entry->max_power_ddbm);
#endif // RADIO_CONFIG_ENABLE_STACK_INFO
#else // SL_RAIL_UTIL_INIT_RADIO_CONFIG_SUPPORT_INST0_ENABLE
  responsePrintError(sl_cli_get_command_string(args, 0), 0x22, "External radio config support not enabled");
#endif // SL_RAIL_UTIL_INIT_RADIO_CONFIG_SUPPORT_INST0_ENABLE
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
// Override weak function called by callback sli_rail_util_on_channel_config_change.
void sl_rail_util_on_channel_config_change(sl_rail_handle_t rail_handle,
                                           const sl_rail_channel_config_entry_t *p_entry)
{
  (void)rail_handle;
  (void)p_entry;
  counters.radioConfigChanged++;
}

sl_rail_status_t disableIncompatibleProtocols(sl_rail_pti_protocol_t newProtocol)
{
  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;
  if ((newProtocol != SL_RAIL_PTI_PROTOCOL_BLE)
      && sl_rail_ble_is_enabled(railHandle)) {
    (void) sl_rail_ble_deinit(railHandle);
  }
  if ((newProtocol != SL_RAIL_PTI_PROTOCOL_802154)
      && sl_rail_ieee802154_is_enabled(railHandle)) {
    status = sl_rail_ieee802154_deinit(railHandle);
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      return status;
    }
  }
  if ((newProtocol != SL_RAIL_PTI_PROTOCOL_ZWAVE)
      && sl_rail_zwave_is_enabled(railHandle)) {
    status = sl_rail_zwave_deinit(railHandle);
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      return status;
    }
  }
#if RAIL_SUPPORTS_PROTOCOL_BTC
  if ((newProtocol != SL_RAIL_PTI_PROTOCOL_BTC)
      && sl_rail_btc_is_enabled(railHandle)) {
    status = sl_rail_btc_deinit(railHandle);
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      return status;
    }
  }
#endif //#if RAIL_SUPPORTS_PROTOCOL_BTC
#if SL_RAIL_SUPPORTS_PROTOCOL_ANT
  if ((newProtocol != SL_RAIL_PTI_PROTOCOL_ANT)
      && sl_rail_ant_is_enabled(railHandle)) {
    status = sl_rail_ant_deinit(railHandle);
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      return status;
    }
  }
#endif //#if SL_RAIL_SUPPORTS_PROTOCOL_ANT
  return sl_rail_set_pti_protocol(railHandle, newProtocol);
}

void configRfSenseSelectiveOokWakeupPhy(sl_cli_command_arg_t *args)
{
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }

  sl_rail_status_t status = sl_rail_config_rf_sense_selective_ook_wakeup_phy(railHandle);
  if (status == SL_RAIL_STATUS_NO_ERROR) {
    changeChannel(0);
  }
  responsePrint(sl_cli_get_command_string(args, 0), "RFSense Wakeup PHY:%s", status ? "Disabled" : "Enabled");
}

void deinitProtocol(sl_cli_command_arg_t *args)
{
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }

  sl_rail_status_t status = disableIncompatibleProtocols(SL_RAIL_PTI_PROTOCOL_CUSTOM);
  responsePrint(sl_cli_get_command_string(args, 0), status
                ? "Current protocol deinit failed"
                : "BLE:Disabled,802.15.4:Disabled,Z-Wave:Disabled");
}
