/***************************************************************************//**
 * @file
 * @brief This file implements the autoack commands in sl_rail_test apps.
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

#include <stdio.h>
#include <string.h>

#include "response_print.h"

#include "sl_rail.h"
#include "sl_rail_zwave.h"
#include "app_common.h"

// Enable start to start TX repeats
static bool enableTxRepeatStartToStart = false;

#if SL_RAIL_SUPPORTS_CHANNEL_HOPPING

static sl_rail_rx_channel_hopping_config_entry_t channelHoppingEntries[MAX_NUMBER_CHANNELS];
static sl_rail_rx_channel_hopping_config_multi_mode_t multiModeParams[MAX_NUMBER_CHANNELS];
static sl_rail_rx_channel_hopping_config_t channelHoppingConfig = {
  .number_of_channels = MAX_NUMBER_CHANNELS,
  .buffer_words = CHANNEL_HOPPING_BUFFER_WORDS,
  .p_entries = channelHoppingEntries,
};
// Enable RX duty cycle with power manager schedule wakeup
static bool enableRxDutyCycleWithSchedWakeup = false;
// RX Duty cycle delay to schedule the periodic wakeup
static sl_rail_time_t rxDutyCycleSchedWakeupDelayUs = 0U;

void configRxChannelHopping(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }
  sl_rail_status_t status;
  uint8_t i = 0U;
  channelHoppingConfig.p_buffer = channelHoppingBuffer;

  if (sl_cli_get_argument_count(args) == 0) {
#if SL_RAIL_SUPPORTS_PROTOCOL_ZWAVE
    // If no arguments are provided, use the Z-Wave timings set by the calculator
    channelHoppingConfig.p_entries = channelHoppingEntries;
    channelHoppingConfig.buffer_words = CHANNEL_HOPPING_BUFFER_WORDS;
    channelHoppingConfig.number_of_channels = MAX_NUMBER_CHANNELS;
    status = sl_rail_zwave_config_rx_channel_hopping(railHandle, &channelHoppingConfig);
    responsePrint(sl_cli_get_command_string(args, 0),
                  "numberOfChannels:%d,paramCh0:%d,paramCh1:%d,paramCh2:%d,paramCh3:%d,buffer:0x%x,Success:%s",
                  channelHoppingConfig.number_of_channels,
                  channelHoppingConfig.p_entries[0].parameter,
                  channelHoppingConfig.p_entries[1].parameter,
                  channelHoppingConfig.p_entries[2].parameter,
                  channelHoppingConfig.p_entries[3].parameter,
                  channelHoppingConfig.p_buffer,
                  status == SL_RAIL_STATUS_NO_ERROR ? "True" : "False");
#else
    responsePrintError(sl_cli_get_command_string(args, 0), 0x12,
                       "Insufficient arguments. Must provide channel hopping sequence.");
#endif
    return;
  } else {
    for (i = 0U; (i + 1) * 4 < (sl_cli_get_argument_count(args) + 1); i++) {
      if (sl_cli_get_argument_uint32(args, ((i + 1) * 4) - 4) > (uint32_t)UINT16_MAX) {
        responsePrintError(sl_cli_get_command_string(args, 0), 0x16, "Channel must be a 16 bit value.");
        return;
      }
      channelHoppingEntries[i].channel   = sl_cli_get_argument_uint32(args, ((i + 1) * 4) - 4);
      channelHoppingEntries[i].mode      = sl_cli_get_argument_uint32(args, ((i + 1) * 4) - 3);
      if ((channelHoppingEntries[i].mode & ~SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE)
          == SL_RAIL_RX_CHANNEL_HOPPING_MODE_MULTI_SENSE) {
        channelHoppingEntries[i].parameter = (uint32_t)(void *)(&multiModeParams[i]);
      } else {
        channelHoppingEntries[i].parameter = sl_cli_get_argument_uint32(args, ((i + 1) * 4) - 2);
      }
      channelHoppingEntries[i].delay_us    = sl_cli_get_argument_uint32(args, ((i + 1) * 4) - 1);
    }
    channelHoppingConfig.number_of_channels = i;
    status = sl_rail_config_rx_channel_hopping(railHandle, &channelHoppingConfig);
    responsePrint(sl_cli_get_command_string(args, 0), "numberOfChannels:%d,buffer:0x%x,Success:%s",
                  i,
                  channelHoppingConfig.p_buffer,
                  status == SL_RAIL_STATUS_NO_ERROR ? "True" : "False");
  }
}

void configChannelHoppingOptions(sl_cli_command_arg_t *args)
{
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }

  uint8_t index = sl_cli_get_argument_uint8(args, 0);
  if (index >= COUNTOF(channelHoppingEntries)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x16,
                       "Index must be < %u", COUNTOF(channelHoppingEntries));
    return;
  }

  sl_rail_rx_channel_hopping_options_t options
    = (sl_rail_rx_channel_hopping_options_t)sl_cli_get_argument_uint8(args, 1);
  int8_t rssiThresholdDbm = channelHoppingEntries[index].rssi_threshold_dbm;

  channelHoppingEntries[index].options = options;

  if (sl_cli_get_argument_count(args) >= 3) {
    rssiThresholdDbm = sl_cli_get_argument_int8(args, 2);
    channelHoppingEntries[index].rssi_threshold_dbm = rssiThresholdDbm;
  }

  responsePrint(sl_cli_get_command_string(args, 0),
                "index:%u,options:%u,rssiThreshold:%d",
                index, options, rssiThresholdDbm);
}

void configChannelHoppingMulti(sl_cli_command_arg_t *args)
{
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }

  uint8_t index = sl_cli_get_argument_uint8(args, 0);
  if (index >= COUNTOF(channelHoppingEntries)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x17,
                       "Index must be < %u", COUNTOF(channelHoppingEntries));
    return;
  }

  multiModeParams[index].sync_detect_us     = sl_cli_get_argument_uint32(args, 1);
  multiModeParams[index].preamble_sense_us  = sl_cli_get_argument_uint32(args, 2);
  multiModeParams[index].timing_sense_us    = sl_cli_get_argument_uint32(args, 3);
  multiModeParams[index].timing_re_sense_us = sl_cli_get_argument_uint32(args, 4);
  multiModeParams[index].status             = 0U;

  responsePrint(sl_cli_get_command_string(args, 0),
                "index:%u,syncDetect:%u,preambleSense:%u,timingSense:%u,timingReSense:%u",
                index,
                multiModeParams[index].sync_detect_us,
                multiModeParams[index].preamble_sense_us,
                multiModeParams[index].timing_sense_us,
                multiModeParams[index].timing_re_sense_us);
}

void enableRxChannelHopping(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }
  bool enable = !!sl_cli_get_argument_uint8(args, 0);

  bool reset = false;
  if (sl_cli_get_argument_count(args) >= 2) {
    reset = !!sl_cli_get_argument_uint8(args, 1);
  }

  sl_rail_status_t status = sl_rail_enable_rx_channel_hopping(railHandle, enable, reset);
  responsePrint(sl_cli_get_command_string(args, 0), "Success:%s", status == SL_RAIL_STATUS_NO_ERROR ? "True" : "False");
}

void getChannelHoppingRssi(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  uint8_t channelIndex = sl_cli_get_argument_uint8(args, 0);

  int16_t result = sl_rail_get_channel_hopping_rssi(railHandle, channelIndex);

  char bufRssi[10];

  // The lowest negative value is used to indicate an error reading the RSSI
  if (result == SL_RAIL_RSSI_INVALID) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x08, "Could not read RSSI. Ensure channel hopping was configured and enabled.");
    return;
  }

  sprintfFloat(bufRssi, sizeof(bufRssi), ((float) result / 4), 2);

  responsePrint(sl_cli_get_command_string(args, 0), "rssi:%s", bufRssi);
}

void configRxDutyCycle(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }
  sl_rail_rx_channel_hopping_mode_t mode;
  int32_t parameters[4] = { 0, };
  int32_t delay;
  sl_rail_rx_channel_hopping_options_t options = SL_RAIL_RX_CHANNEL_HOPPING_OPTIONS_NONE;
  int8_t rssiThresholdDbm = SL_RAIL_RSSI_INVALID_DBM;
  uint32_t currentArg = 0U;
  uint32_t argCount = sl_cli_get_argument_count(args);
  uint32_t neededArgCount = 3U; // mode parameter delay minimally needed

  mode = (sl_rail_rx_channel_hopping_mode_t) sl_cli_get_argument_uint32(args, currentArg++);
  bool modeIsMulti = ((mode & ~SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE)
                      == SL_RAIL_RX_CHANNEL_HOPPING_MODE_MULTI_SENSE);
  if (modeIsMulti) {
    neededArgCount += 3U; // needs timingReSense preambleSense syncDetect params
  }
  if ((mode & SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE) != 0U) {
    neededArgCount += 1U; // needs options
  }
  if (argCount < neededArgCount) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x12,
                       "Insufficient arguments for specified mode");
    return;
  }
  parameters[0] = sl_cli_get_argument_int32(args, currentArg++);
  if (modeIsMulti) {
    parameters[1] = sl_cli_get_argument_int32(args, currentArg++);
    parameters[2] = sl_cli_get_argument_int32(args, currentArg++);
    parameters[3] = sl_cli_get_argument_int32(args, currentArg++);
  }
  delay = sl_cli_get_argument_int32(args, currentArg++);
  if ((mode & SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE) != 0U) {
    options = (sl_rail_rx_channel_hopping_options_t) sl_cli_get_argument_int32(args, currentArg++);
    if ((options & SL_RAIL_RX_CHANNEL_HOPPING_OPTION_RSSI_THRESHOLD)
        != SL_RAIL_RX_CHANNEL_HOPPING_OPTIONS_NONE) {
      if (currentArg >= argCount) {
        responsePrintError(sl_cli_get_command_string(args, 0), 0x13,
                           "Missing rssiThreshold argument for specified option");
        return;
      }
      rssiThresholdDbm = (int8_t) sl_cli_get_argument_int32(args, currentArg++);
    }
  }
  if (currentArg != argCount) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x14,
                       "Unexpected extra arguments for specified mode/options");
    return;
  }

  multiModeParams[0].sync_detect_us     = (uint32_t)parameters[0];
  multiModeParams[0].preamble_sense_us  = (uint32_t)parameters[1];
  multiModeParams[0].timing_sense_us    = (uint32_t)parameters[2];
  multiModeParams[0].timing_re_sense_us = (uint32_t)parameters[3];
  multiModeParams[0].status             = 0U;
  sl_rail_rx_duty_cycle_config_t config = {
    .mode = mode,
    .parameter = (modeIsMulti
                  ? ((uint32_t)(void *)&multiModeParams[0])
                  : ((uint32_t)parameters[0])),
    .delay_us = (uint32_t)delay,
    .options = options,
    .rssi_threshold_dbm = rssiThresholdDbm,
  };

  rxDutyCycleSchedWakeupDelayUs = delay;

  sl_rail_status_t status = sl_rail_config_rx_duty_cycle(railHandle, &config);

  responsePrint(sl_cli_get_command_string(args, 0),
                "Success:%s,Mode:0x%x,Parameter:%u,Delay:%d,Options:%u,rssiThreshold:%d",
                status == SL_RAIL_STATUS_NO_ERROR ? "True" : "False",
                config.mode,
                config.parameter,
                config.delay_us,
                config.options,
                config.rssi_threshold_dbm);
}

void getDefaultRxDutyCycleConfig(sl_cli_command_arg_t *args)
{
  sl_rail_rx_duty_cycle_config_t config = { 0 };

  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));

  sl_rail_status_t status = sl_rail_get_default_rx_duty_cycle_config(railHandle,
                                                                     &config);
  responsePrint(sl_cli_get_command_string(args, 0),
                "Success:%s,Mode:0x%x,Parameter:%u,Delay:%d,Options:%u,rssiThreshold:%d",
                status == SL_RAIL_STATUS_NO_ERROR ? "True" : "False",
                config.mode,
                config.parameter,
                config.delay_us,
                config.options,
                config.rssi_threshold_dbm);
}

void enableRxDutyCycle(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }

  enableRxDutyCycleWithSchedWakeup = false;
  serEvent = false;
  bool enable = !!sl_cli_get_argument_uint8(args, 0);

  if (sl_cli_get_argument_count(args) >= 2) {
    enableRxDutyCycleWithSchedWakeup = !!(sl_cli_get_argument_uint8(args, 1));
  }

  sl_rail_status_t status = sl_rail_enable_rx_duty_cycle(railHandle, enable);
  responsePrint(sl_cli_get_command_string(args, 0), "Success:%s",
                (status == SL_RAIL_STATUS_NO_ERROR) ? "True" : "False");
}

bool getRxDutyCycleSchedWakeupEnable(sl_rail_time_t *sleepInterval)
{
  bool schedWakeupEnable = false;

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  if (sleepInterval != NULL) {
    *sleepInterval = rxDutyCycleSchedWakeupDelayUs;
    schedWakeupEnable = enableRxDutyCycleWithSchedWakeup;
  }

  CORE_EXIT_CRITICAL();

  return schedWakeupEnable;
}

sl_rail_channel_metadata_t channelMetadata[MAX_NUMBER_CHANNELS];
#define TIME_PER_CHANNEL (10000UL)
void spectrumAnalyzer(sl_cli_command_arg_t *args)
{
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }

  uint16_t channelMetadataLength = MAX_NUMBER_CHANNELS;

  uint16_t minChannel = 0U;
  if (sl_cli_get_argument_count(args) >= 2) {
    minChannel = sl_cli_get_argument_uint16(args, 1);
  }

  uint16_t maxChannel = UINT16_MAX;
  if (sl_cli_get_argument_count(args) >= 3) {
    maxChannel = sl_cli_get_argument_uint16(args, 2);
  }

  sl_rail_status_t status = sl_rail_get_channel_metadata(railHandle,
                                                         channelMetadata,
                                                         &channelMetadataLength,
                                                         minChannel,
                                                         maxChannel);

  if (channelMetadataLength == 0) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x1, "No channels in range found");
    return;
  }
  bool graphics = false;
  if (sl_cli_get_argument_count(args) >= 1) {
    graphics = !!sl_cli_get_argument_uint8(args, 0);
  }

  if (status == SL_RAIL_STATUS_INVALID_STATE) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x20, "Error in sl_rail_get_channel_metadata, channel config invalid.");
    return;
  }

  uint8_t length = MAX_NUMBER_CHANNELS < channelMetadataLength
                   ? MAX_NUMBER_CHANNELS : channelMetadataLength;
  for (uint16_t i = 0; i < length; i++) {
    channelHoppingEntries[i].channel = channelMetadata[i].channel;
    channelHoppingEntries[i].mode = SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMEOUT;
    channelHoppingEntries[i].parameter = TIME_PER_CHANNEL;
    channelHoppingEntries[i].delay_us = 0;
  }
  channelHoppingConfig.number_of_channels = channelMetadataLength;
  channelHoppingConfig.p_buffer = channelHoppingBuffer;

  sl_rail_config_rx_channel_hopping(railHandle, &channelHoppingConfig);
  sl_rail_enable_rx_channel_hopping(railHandle, true, true);
  sl_rail_start_rx(railHandle, channelMetadata[0].channel, NULL);

  // Make sure we get into receive and then have enough time to
  // cycle through all the channels (time per channel * (num(channels) + safety buffer))
  while (sl_rail_get_radio_state(railHandle) != SL_RAIL_RF_STATE_RX) ;
  sl_rail_delay_us(railHandle, TIME_PER_CHANNEL * (channelMetadataLength + 5));

  responsePrintHeader(sl_cli_get_command_string(args, 0), "channelIndex:%u,channel:%u,frequency:%u,rssi:%s");
  int16_t min = INT16_MAX;
  int16_t max = INT16_MIN;
  int16_t result[MAX_NUMBER_CHANNELS];
  for (uint16_t i = 0; i < channelMetadataLength; i++) {
    result[i] = sl_rail_get_channel_hopping_rssi(railHandle, i);
    if (result[i] > max) {
      max = result[i];
    }
    if (result[i] < min) {
      min = result[i];
    }
    if (result[i] == SL_RAIL_RSSI_INVALID) {
      continue;
    }
    char bufRssi[10];
    sprintfFloat(bufRssi, sizeof(bufRssi), ((float) result[i] / 4), 2);
    responsePrintMulti("channelIndex:%u,channel:%u,frequency:%u,rssi:%s",
                       i, channelMetadata[i].channel,
                       channelMetadata[i].frequency_hz, bufRssi);
  }

  sl_rail_idle(railHandle, SL_RAIL_IDLE_FORCE_SHUTDOWN_CLEAR_FLAGS, true);

  if (!graphics) {
    return;
  }

  printf("\n");
  printf("+----------------------------+\n");
  printf("| RSSI dBm by Channel Number |\n");
  printf("+----------------------------+\n");
  char bufRssi[10];
  for (uint8_t i = 0; i <= 20; i++) {
    for (uint8_t j = 0; j < channelMetadataLength; j++) {
      if (i != 20) {
        for (uint8_t k = 0; k < 4; k++) {
          if (result[j] >= (max - (((max - min) / 20) * i))) {
            printf("#");
          } else {
            printf(".");
          }
        }
      } else {
        printf("%-4d", channelMetadata[j].channel);
      }
    }
    sprintfFloat(bufRssi, sizeof(bufRssi), ((float) (max - (((max - min) / 20) * i)) / 4), 2);
    printf(" %s dBm\n", bufRssi);
  }
  printf("Channel Number\n");
}

#else

static void channelHoppingNotSupported(sl_cli_command_arg_t *args)
{
  responsePrintError(sl_cli_get_command_string(args, 0), 0x17, "Channel hopping not suppported on this chip");
}

void configRxChannelHopping(sl_cli_command_arg_t *args)
{
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  channelHoppingNotSupported(args);
}

void enableRxChannelHopping(sl_cli_command_arg_t *args)
{
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  channelHoppingNotSupported(args);
}

void getChannelHoppingRssi(sl_cli_command_arg_t *args)
{
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  channelHoppingNotSupported(args);
}

void configRxDutyCycle(sl_cli_command_arg_t *args)
{
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  channelHoppingNotSupported(args);
}

void enableRxDutyCycle(sl_cli_command_arg_t *args)
{
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  channelHoppingNotSupported(args);
}

void getDefaultRxDutyCycleConfig(sl_cli_command_arg_t *args)
{
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  channelHoppingNotSupported(args);
}

void spectrumAnalyzer(sl_cli_command_arg_t *args)
{
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  channelHoppingNotSupported(args);
}

void configChannelHoppingOptions(sl_cli_command_arg_t *args)
{
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  channelHoppingNotSupported(args);
}

void configChannelHoppingMulti(sl_cli_command_arg_t *args)
{
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  channelHoppingNotSupported(args);
}
bool getRxDutyCycleSchedWakeupEnable(sl_rail_time_t *sleepInterval)
{
  (void)sleepInterval;
  return false;
}

#endif

void configTxRepeatStartToStart(sl_cli_command_arg_t *args)
{
  char *command = sl_cli_get_command_string(args, 0);
#if SL_RAIL_SUPPORTS_TX_REPEAT_START_TO_START
  if (sl_cli_get_argument_uint8(args, 0) > 1) {
    responsePrintError(command, 0x16, "Value must be 0 or 1");
  } else {
    enableTxRepeatStartToStart = !!sl_cli_get_argument_uint8(args, 0);
  }
  responsePrint(command, "TxRepeatStartToStart: %u", enableTxRepeatStartToStart);
#else
  responsePrintError(command, 0x16, "Start to start TX repeats not supported on this platform");
#endif
}

void setNextTxRepeat(sl_cli_command_arg_t *args)
{
  sl_rail_tx_repeat_config_t repeats = {
    .iterations = sl_cli_get_argument_uint16(args, 0),
    .repeat_options = SL_RAIL_TX_REPEAT_OPTIONS_NONE,
    .delay_or_hop.delay_us = SL_RAIL_TRANSITION_TIME_KEEP,
  };

  if (enableTxRepeatStartToStart) {
    repeats.repeat_options |= SL_RAIL_TX_REPEAT_OPTION_START_TO_START;
  }

  uint32_t argc = sl_cli_get_argument_count(args);
  char *command = sl_cli_get_command_string(args, 0);

  if (argc < 2U) {
    // repeats already setup appropriately
  } else if (argc <= 2U) {
    repeats.delay_or_hop.delay_us = sl_cli_get_argument_uint32(args, 1);
  } else { // argc > 2U
#if SL_RAIL_SUPPORTS_CHANNEL_HOPPING
    sl_rail_tx_channel_hopping_config_entry_t *txChannelHoppingEntries =
      (sl_rail_tx_channel_hopping_config_entry_t *) channelHoppingEntries;
    uint32_t i = 0U;
    for (uint32_t arg = 2U; arg < argc; arg += 2U) {
      uint32_t channel = sl_cli_get_argument_uint32(args, arg);
      if (channel > (uint32_t)UINT16_MAX) {
        responsePrintError(command, 0x16, "Channel must be a 16 bit value.");
        return;
      }
      txChannelHoppingEntries[i].delay_us = sl_cli_get_argument_uint32(args, arg - 1U);
      txChannelHoppingEntries[i].channel = (uint16_t)channel;
      i++;
    }
    repeats.repeat_options |= SL_RAIL_TX_REPEAT_OPTION_HOP;
    repeats.delay_or_hop.channel_hopping.number_of_channels = i;
    repeats.delay_or_hop.channel_hopping.p_buffer = channelHoppingBuffer;
    repeats.delay_or_hop.channel_hopping.buffer_words = CHANNEL_HOPPING_BUFFER_WORDS;
    repeats.delay_or_hop.channel_hopping.p_entries = txChannelHoppingEntries;
#else
    responsePrintError(command, 0x16, "Hopping not supported on this platform.");
    return;
#endif
  }

  sl_rail_status_t ret = sl_rail_set_next_tx_repeat(railHandle, &repeats);
  if (ret != SL_RAIL_STATUS_NO_ERROR) {
    repeats.iterations = 0;
  }
  txRepeatCount = repeats.iterations;
  responsePrint(command, "RepeatedTxCount:%u", repeats.iterations);
}

void setNextBleTxRepeat(sl_cli_command_arg_t *args)
{
  sl_rail_ble_tx_repeat_config_t repeats = {
    .iterations = sl_cli_get_argument_uint16(args, 0),
    .repeat_options = SL_RAIL_TX_REPEAT_OPTIONS_NONE,
    .delay_or_hop.delay_us = SL_RAIL_TRANSITION_TIME_KEEP,
  };

  if (enableTxRepeatStartToStart) {
    repeats.repeat_options |= SL_RAIL_TX_REPEAT_OPTION_START_TO_START;
  }

  uint32_t argc = sl_cli_get_argument_count(args);
  char *command = sl_cli_get_command_string(args, 0);

  if (argc < 2U) {
    // repeats already setup appropriately
  } else if (argc <= 2U) {
    repeats.delay_or_hop.delay_us = sl_cli_get_argument_uint32(args, 1);
  } else if (argc <= 5U) {
    responsePrintError(command, 0x17, "Need at least 6 parameters for BLE hop config.");
    return;
  } else { // argc > 5U
#if SL_RAIL_SUPPORTS_CHANNEL_HOPPING
    // Default parameters to not exceed CLI parameter limits
    uint32_t crcInit = 0x00555555UL;
    bool disableWhitening = false;

    sl_rail_ble_tx_channel_hopping_config_entry_t *txChannelHoppingEntries =
      (sl_rail_ble_tx_channel_hopping_config_entry_t *) channelHoppingEntries;
    uint32_t i = 0U;
    for (uint32_t arg = 2U; arg < argc; arg += 5U) {
      uint32_t railChannel    = sl_cli_get_argument_uint32(args, arg + 1U);
      uint32_t logicalChannel = sl_cli_get_argument_uint32(args, arg + 2U);
      if (railChannel >= 40U) {
        responsePrintError(command, 0x27, "RAIL channel must be a valid BLE channel.");
      }
      if (logicalChannel >= 40U) {
        responsePrintError(command, 0x28, "Logical channel must be a valid BLE channel.");
      }

      txChannelHoppingEntries[i].delay_us          = sl_cli_get_argument_uint32(args, arg - 1U);
      txChannelHoppingEntries[i].phy               = sl_cli_get_argument_uint32(args, arg);
      txChannelHoppingEntries[i].rail_channel      = (uint8_t)railChannel;
      txChannelHoppingEntries[i].logical_channel   = (uint8_t)logicalChannel;
      txChannelHoppingEntries[i].access_address    = sl_cli_get_argument_uint32(args, arg + 3U);
      txChannelHoppingEntries[i].disable_whitening = disableWhitening;
      txChannelHoppingEntries[i].crc_init          = crcInit;
      i++;
    }
    repeats.repeat_options |= SL_RAIL_TX_REPEAT_OPTION_HOP;
    repeats.delay_or_hop.channel_hopping.number_of_channels = i;
    repeats.delay_or_hop.channel_hopping.p_buffer = channelHoppingBuffer;
    repeats.delay_or_hop.channel_hopping.buffer_words = CHANNEL_HOPPING_BUFFER_WORDS;
    repeats.delay_or_hop.channel_hopping.p_entries = txChannelHoppingEntries;
#else
    responsePrintError(command, 0x16, "Hopping not supported on this platform.");
    return;
#endif
  }

  sl_rail_status_t ret = sl_rail_ble_set_next_tx_repeat(railHandle, &repeats);
  if (ret != SL_RAIL_STATUS_NO_ERROR) {
    repeats.iterations = 0;
  }
  txRepeatCount = repeats.iterations;
  responsePrint(command, "RepeatedTxCount:%u", repeats.iterations);
}

void triggerRxHop(sl_cli_command_arg_t *args)
{
  sl_rail_status_t ret = sl_rail_trigger_rx_channel_hop(railHandle);
  responsePrint(sl_cli_get_command_string(args, 0), "HopTriggerStatus:%u", ret);
}
