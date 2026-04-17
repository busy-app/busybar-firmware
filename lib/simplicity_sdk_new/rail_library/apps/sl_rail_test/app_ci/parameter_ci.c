/***************************************************************************//**
 * @file
 * @brief This file implements the parameter commands for sl_rail_test applications.
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
#if !defined(__ICCARM__)
// IAR doesn't have strings.h and puts those declarations in string.h
#include <strings.h>
#endif
#include <stdio.h>

#include "response_print.h"

#include "sl_rail.h"
#include "app_common.h"
#include "sl_core.h"
#include SL_RAIL_UTIL_PA_PLUGIN_HEADER

// This function gets the power mode index from a string.
// Returns 0xFF if the string is not found.
static uint8_t getPowerModeIndexFromString(char *modeString)
{
  // If modeString has the 'SL_' prefix, strip it
#define PREFIX "SL_"
  if (strncasecmp(modeString, PREFIX, sizeof(PREFIX) - 1) == 0) {
    modeString += sizeof(PREFIX) - 1;
  }
#undef  PREFIX
  // If modeString has the 'RAIL_TX_POWER_MODE_' prefix, strip it
#define PREFIX "RAIL_TX_POWER_MODE_"
  if (strncasecmp(modeString, PREFIX, sizeof(PREFIX) - 1) == 0) {
    modeString += sizeof(PREFIX) - 1;
  }
  for (uint8_t i = 0U; i <= SL_RAIL_TX_POWER_MODE_NONE; i++) {
    if (strcasecmp(modeString, paStrings[i]
                   + sizeof("SL_" PREFIX) - 1) == 0) {
      return i;
    }
    // RAIL 2.x backwards compatibility name support
    if (strcasecmp(modeString, paStrings2x[i]
                   + sizeof(PREFIX) - 1) == 0) {
      return i;
    }
  }
#undef  PREFIX
  return 0xFF;
}

void getChannel(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  if (sl_rail_get_debug_mode(railHandle) & SL_RAIL_DEBUG_MODE_FREQ_OVERRIDE) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x12, "Channels are not valid in Debug Mode");
  } else {
    responsePrint(sl_cli_get_command_string(args, 0), "channel:%d", getLikelyChannel());
  }
}

void setChannel(sl_cli_command_arg_t *args)
{
  uint16_t proposedChannel = sl_cli_get_argument_uint16(args, 0);
  bool success = false;

  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  // Make sure this is a valid channel
  if (sl_rail_is_valid_channel(railHandle, proposedChannel)
      == SL_RAIL_STATUS_NO_ERROR) {
    changeChannel(proposedChannel);
    success = true;
  }

  if (!success) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x11, "Invalid channel '%d'", proposedChannel);
    return;
  }

  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  getChannel(args);
}

void setPktOffsetInTxFifo(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  if (sl_cli_get_argument_count(args) == 1) {
    uint16_t proposedOffset = sl_cli_get_argument_uint16(args, 0);

    // Make sure this is a valid Offset for the current TX FIFO
    const sl_rail_config_t *railConfig = sl_rail_get_config(railHandle);
    if (proposedOffset < railConfig->tx_fifo_bytes) {
      offsetInTxFifo = proposedOffset;
    } else {
      responsePrintError(sl_cli_get_command_string(args, 0), 0x11,
                         "Invalid Offset '%u', must be strictly less than %u",
                         proposedOffset, railConfig->tx_fifo_bytes);
      return;
    }
  }
  responsePrint(sl_cli_get_command_string(args, 0),
                "Tx data Offset in Tx Fifo:%u", offsetInTxFifo);
}

void setFreqOffset(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  static sl_rail_frequency_offset_t currentFreqOffset = 0;
  if (sl_cli_get_argument_count(args) >= 1) {
    sl_rail_frequency_offset_t freqOffset = sl_cli_get_argument_int32(args, 0);

    if ((freqOffset < SL_RAIL_FREQUENCY_OFFSET_MIN)
        || (freqOffset > SL_RAIL_FREQUENCY_OFFSET_MAX)) {
      responsePrintError(sl_cli_get_command_string(args, 0), SL_RAIL_STATUS_INVALID_PARAMETER,
                         "Offset %d outside range of [%d, %d]",
                         freqOffset,
                         SL_RAIL_FREQUENCY_OFFSET_MIN,
                         SL_RAIL_FREQUENCY_OFFSET_MAX);
      return;
    }
    sl_rail_status_t status = sl_rail_set_freq_offset(railHandle, freqOffset);
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      responsePrintError(sl_cli_get_command_string(args, 0), status, "Could not set frequency offset");
      return;
    }
    currentFreqOffset = freqOffset;
  }
  responsePrint(sl_cli_get_command_string(args, 0), "freqOffset:%d", currentFreqOffset);
}

void getPowerConfig(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  sl_rail_tx_power_config_t config;
  sl_rail_status_t status = sli_rail_get_tx_power_config(railHandle, &config);

  responsePrint(sl_cli_get_command_string(args, 0), "success:%s,mode:%s,modeIndex:%d,voltage:%d,rampTime:%d",
                status == SL_RAIL_STATUS_NO_ERROR ? "true" : "false",
                paStrings[config.mode], config.mode, config.voltage_mv, config.ramp_time_us);
}

void getPowerLimits(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));

  sl_rail_tx_power_mode_t powerMode = SL_RAIL_TX_POWER_MODE_NONE;
  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;
  if (sl_cli_get_argument_count(args) >= 1) {
    powerMode = sl_cli_get_argument_uint8(args, 0);
  } else {
    sl_rail_tx_power_config_t config;
    status = sli_rail_get_tx_power_config(railHandle, &config);
    powerMode = config.mode;
  }

  if (powerMode >= SL_RAIL_TX_POWER_MODE_NONE || status != SL_RAIL_STATUS_NO_ERROR ) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x13,
                       "Invalid PA enum value selected: %d", powerMode);
    return;
  }
  sli_rail_tx_power_level_t maxPowerlevel = SLI_RAIL_TX_POWER_LEVEL_INVALID;
  sli_rail_tx_power_level_t minPowerlevel = SLI_RAIL_TX_POWER_LEVEL_INVALID;
  bool success = sli_rail_supports_tx_power_mode(railHandle, &powerMode,
                                                 &maxPowerlevel, &minPowerlevel);
  responsePrint(sl_cli_get_command_string(args, 0),
                "success:%s,powerMode:%s,minPowerLevel:%d,maxPowerLevel:%d",
                success ? "Success" : "Failure",
                paStrings[powerMode], minPowerlevel, maxPowerlevel);
  return;
}

void setPowerConfig(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  // Attempt to convert the power mode string to a sl_rail_tx_power_mode_t enum index.
  sl_rail_tx_power_mode_t mode = getPowerModeIndexFromString(sl_cli_get_argument_string(args, 0));
  if (mode == 0xFF) {
    // If the string doesn't match a powerMode, attempt to convert it to a sl_rail_tx_power_mode_t.
    char *strtoulEnd;
    mode = (sl_rail_tx_power_mode_t)strtoul(sl_cli_get_argument_string(args, 0), &strtoulEnd, 0);
    //check that strtoul didn't fail and that mode is valid sl_rail_tx_power_mode_t
    if (strtoulEnd == sl_cli_get_argument_string(args, 0) || mode >= SL_RAIL_TX_POWER_MODE_NONE) {
      responsePrintError(sl_cli_get_command_string(args, 0), 0x13, "Invalid PA enum value selected: %s", sl_cli_get_argument_string(args, 0));
      return;
    }
  }

  sl_rail_tx_power_config_t *txPowerConfigPtr;

#if _SILICON_LABS_EFR32_RADIO_TYPE == _SILICON_LABS_EFR32_RADIO_DUALBAND
  if (mode >= SL_RAIL_TX_POWER_MODE_SUB_GHZ_HP) {
    txPowerConfigPtr = (sl_rail_tx_power_config_t *)sl_rail_util_pa_get_tx_power_config_subghz();
  } else {
    txPowerConfigPtr = (sl_rail_tx_power_config_t *)sl_rail_util_pa_get_tx_power_config_2p4ghz();
  }
#elif _SILICON_LABS_EFR32_RADIO_TYPE == _SILICON_LABS_EFR32_RADIO_2G4HZ
  txPowerConfigPtr = (sl_rail_tx_power_config_t *)sl_rail_util_pa_get_tx_power_config_2p4ghz();
#elif _SILICON_LABS_EFR32_RADIO_TYPE == _SILICON_LABS_EFR32_RADIO_SUBGHZ
  txPowerConfigPtr = (sl_rail_tx_power_config_t *)sl_rail_util_pa_get_tx_power_config_subghz();
#endif

  // Make a backup of the TX Power Config before it's changed.
  sl_rail_tx_power_config_t txPowerConfigBackup = *txPowerConfigPtr; // struct copy

  uint16_t voltage = sl_cli_get_argument_uint16(args, 1);
  uint16_t rampTime = sl_cli_get_argument_uint16(args, 2);

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  txPowerConfigPtr->mode = mode;
  txPowerConfigPtr->voltage_mv = voltage;
  txPowerConfigPtr->ramp_time_us = rampTime;
  CORE_EXIT_CRITICAL();

  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  sl_rail_status_t status = sl_rail_config_tx_power(railHandle, txPowerConfigPtr);
  // Restore the backup TX Power Config on error.
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_CRITICAL();
    *txPowerConfigPtr = txPowerConfigBackup; // struct copy
    CORE_EXIT_CRITICAL();
  }

  if (status == SL_RAIL_STATUS_NO_ERROR) {
    args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
    getPowerConfig(args);
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x26, "Could not set power config");
  }
}

void getPower(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  responsePrint(sl_cli_get_command_string(args, 0),
                "powerLevel:%d,power:%d",
                sli_rail_get_tx_power(railHandle),
                sl_rail_get_tx_power_dbm(railHandle));
}

void setPower(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }
  bool setPowerError = false;

  if (sl_cli_get_argument_count(args) >= 2 && strcmp(sl_cli_get_argument_string(args, 1), "raw") == 0) {
    sli_rail_tx_power_level_t rawLevel = sl_cli_get_argument_uint8(args, 0);

#if SL_RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
    sl_rail_tx_power_config_t tempCfg;
    sli_rail_get_tx_power_config(railHandle, &tempCfg);
#ifdef _SILICON_LABS_32B_SERIES_2
    if (SL_RAIL_POWER_MODE_IS_ANY_DBM_POWERSETTING_MAPPING_TABLE(tempCfg.mode)) {
      // dBm-to-powerSetting mode does not support raw power setting
      responsePrintError(sl_cli_get_command_string(args, 0), SL_RAIL_STATUS_INVALID_PARAMETER, "%s does not support setting raw power.", paStrings[tempCfg.mode]);
      return;
    }
#endif
#endif

    // Set the power and update the RAW level global
    if (sli_rail_set_tx_power(railHandle, rawLevel) != SL_RAIL_STATUS_NO_ERROR) {
      setPowerError = true;
    }
  } else {
    sl_rail_tx_power_config_t tempCfg;
    sl_rail_tx_power_t powerDbm = sl_cli_get_argument_int16(args, 0);

    // Set the power in dBm and figure out what RAW level to store based on what
    // was requested NOT what is actually applied to the hardware after limits.
    if ((sl_rail_set_tx_power_dbm(railHandle, powerDbm)
         != SL_RAIL_STATUS_NO_ERROR)
        || (sli_rail_get_tx_power_config(railHandle, &tempCfg)
            != SL_RAIL_STATUS_NO_ERROR)) {
      setPowerError = true;
    }
  }

  if (setPowerError) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x23, "Could not set power.");
  } else {
    // Get and print out the actual applied power and power level
    args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
    getPower(args);
  }
}

void setPowerWithPaMode(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }

  sl_rail_tx_power_t powerDbm = sl_cli_get_argument_int16(args, 0);
  sl_rail_tx_pa_mode_t pa_mode = (sl_rail_tx_pa_mode_t)sl_cli_get_argument_uint8(args, 1);

  // Validate PA mode (check against invalid value)
  if (pa_mode >= SL_RAIL_TX_PA_MODE_INVALID) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x13, "Invalid PA mode: %d", pa_mode);
    return;
  }

  sl_rail_status_t status = sl_rail_set_tx_power_dbm_with_pa_mode(railHandle, powerDbm, pa_mode);

  if (status != SL_RAIL_STATUS_NO_ERROR) {
    responsePrintError(sl_cli_get_command_string(args, 0), status, "Could not set power %d with PA mode %d", powerDbm, pa_mode);
  } else {
    // Get and print out the actual applied power and power level
    args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
    responsePrint(sl_cli_get_command_string(args, 0),
                  "powerLevel:%d,power:%d,pa_mode:%d",
                  sli_rail_get_tx_power(railHandle),
                  sl_rail_get_tx_power_dbm(railHandle),
                  pa_mode);
  }
}

void sweepTxPower(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  responsePrint(sl_cli_get_command_string(args, 0), "Sweeping:Started,Instructions:'q' to quit or 'enter' to continue.");
  sl_rail_tx_power_config_t txPowerConfig;

  sli_rail_get_tx_power_config(railHandle, &txPowerConfig);

  sli_rail_tx_power_level_t start = 1;
  sli_rail_tx_power_level_t end = 255;

  switch (txPowerConfig.mode) {
#ifdef SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP
    case SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP:
      start = SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MIN;
      end = SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX;
      break;
#endif
#ifdef SL_RAIL_TX_POWER_MODE_2P4_GHZ_MP
    case SL_RAIL_TX_POWER_MODE_2P4_GHZ_MP:
      start = SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_MP_MIN;
      end = SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_MP_MAX;
      break;
#endif
#ifdef SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP
    case SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP:
      start = SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MIN;
      end = SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MAX;
      break;
#endif
#ifdef SL_RAIL_TX_POWER_MODE_2P4_GHZ_LLP
    case SL_RAIL_TX_POWER_MODE_2P4_GHZ_LLP:
      start = SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LLP_MIN;
      end = SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LLP_MAX;
      break;
#endif
#ifdef SL_RAIL_TX_POWER_MODE_SUB_GHZ_HP
    case SL_RAIL_TX_POWER_MODE_SUB_GHZ_HP:
      start = SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_HP_MIN;
      end = SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_HP_MAX;
      break;
#endif
#ifdef SL_RAIL_TX_POWER_MODE_SUB_GHZ_MP
    case SL_RAIL_TX_POWER_MODE_SUB_GHZ_MP:
      start = SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MP_MIN;
      end = SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MP_MAX;
      break;
#endif
#ifdef SL_RAIL_TX_POWER_MODE_SUB_GHZ_LP
    case SL_RAIL_TX_POWER_MODE_SUB_GHZ_LP:
      start = SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_LP_MIN;
      end = SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_LP_MAX;
      break;
#endif
#ifdef SL_RAIL_TX_POWER_MODE_SUB_GHZ_LLP
    case SL_RAIL_TX_POWER_MODE_SUB_GHZ_LLP:
      start = SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_LLP_MIN;
      end = SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_LLP_MAX;
      break;
#endif
#ifdef SL_RAIL_TX_POWER_MODE_OFDM_PA_POWERSETTING_TABLE
    case SL_RAIL_TX_POWER_MODE_OFDM_PA_POWERSETTING_TABLE:
      start = SLI_RAIL_TX_POWER_LEVEL_OFDM_PA_MIN;
      end = SLI_RAIL_TX_POWER_LEVEL_OFDM_PA_MAX;
      break;
#endif
#ifdef SL_RAIL_TX_POWER_MODE_SUB_GHZ_POWERSETTING_TABLE
    case SL_RAIL_TX_POWER_MODE_SUB_GHZ_POWERSETTING_TABLE:
      start = SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MIN;
      end = SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MAX;
      break;
#endif
    default:
      responsePrintError(sl_cli_get_command_string(args, 0), 0x21, "PA not configured.");
      return;
  }

  sli_rail_tx_power_level_t i;
  char input = ((args->handle->last_input_type == SL_CLI_INPUT_RETURN) ? '\r' : '\0');
  char lastChar;

  for (i = start; i <= end; i++) {
    responsePrint(sl_cli_get_command_string(args, 0), "PowerLevel:%u", i);
    sl_rail_idle(railHandle, SL_RAIL_IDLE_FORCE_SHUTDOWN_CLEAR_FLAGS, true);
    sli_rail_set_tx_power(railHandle, i);
    sl_rail_start_tx_stream(railHandle, channel, SL_RAIL_STREAM_CARRIER_WAVE,
                            SL_RAIL_TX_OPTIONS_DEFAULT);

    lastChar = input;
    input = getchar();

    while (1) {
      if ((input == '\r') || ((input == '\n') && (lastChar != '\r'))) {
        break;
      }

      if (input == 'q') {
        responsePrintError(sl_cli_get_command_string(args, 0), 0x20, "Sweep Aborted.");
        sl_rail_idle(railHandle, SL_RAIL_IDLE_FORCE_SHUTDOWN_CLEAR_FLAGS, true);
        return;
      }

      lastChar = input;
      input = getchar();
    }

    sl_rail_idle(railHandle, SL_RAIL_IDLE_FORCE_SHUTDOWN_CLEAR_FLAGS, true);
  }

  responsePrint(sl_cli_get_command_string(args, 0), "Sweeping:Complete");
}

void getCtune(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  uint32_t ctune = sl_rail_get_tune(railHandle);
  responsePrint(sl_cli_get_command_string(args, 0),
                "CTUNEXIANA:0x%.3x,CTUNEXOANA:0x%.3x",
                ctune,
                (ctune + sl_rail_get_tune_delta(railHandle)));
}

void setCtune(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }

  if (sl_rail_set_tune(railHandle, sl_cli_get_argument_uint32(args, 0))
      != SL_RAIL_STATUS_NO_ERROR) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x28, "Invalid CTUNE value provided");
    return;
  }

  // Read out and print the current CTUNE value
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  getCtune(args);
}

void getCtuneDelta(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  int32_t delta = sl_rail_get_tune_delta(railHandle);

  responsePrint(sl_cli_get_command_string(args, 0), "CTuneDelta:%d", delta);
}

void setCtuneDelta(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  sl_rail_set_tune_delta(railHandle, sl_cli_get_argument_uint32(args, 0));

  // Read out and print the current CTUNE delta value
  getCtuneDelta(args);
}

void enablePaCal(sl_cli_command_arg_t *args)
{
  uint8_t enable = sl_cli_get_argument_uint8(args, 0);
  sl_rail_enable_pa_cal(railHandle, enable);
  responsePrint(sl_cli_get_command_string(args, 0), "paCal:%s", (enable ? "Enabled" : "Disabled"));
}

// Helper to convert a string to a sl_rail_radio_state
static int8_t stringToState(char *string, sl_rail_radio_state_t *state)
{
  switch (string[0]) {
    case 'i': case 'I':
      *state = SL_RAIL_RF_STATE_IDLE;
      break;
    case 'r': case 'R':
      *state = SL_RAIL_RF_STATE_RX;
      break;
    case 't': case 'T':
      *state = SL_RAIL_RF_STATE_TX;
      break;
    default:
      return 1;
  }
  return 0;
}

void setTxTransitions(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  sl_rail_radio_state_t states[2];
  if (stringToState(sl_cli_get_argument_string(args, 0), &states[0])
      || stringToState(sl_cli_get_argument_string(args, 1), &states[1])) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x16, "Invalid states");
    return;
  }

  sl_rail_state_transitions_t transitions = {
    .success = states[0],
    .error = states[1]
  };

  sl_rail_status_t ret = sl_rail_set_tx_transitions(railHandle, &transitions);
  responsePrint(sl_cli_get_command_string(args, 0), "TxTransitions:%s", (ret ? "Error" : "Set"));
}

void setRxTransitions(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }
  sl_rail_radio_state_t states[2];
  if (stringToState(sl_cli_get_argument_string(args, 0), &states[0])
      || stringToState(sl_cli_get_argument_string(args, 1), &states[1])) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x16, "Invalid states");
    return;
  }
  sl_rail_state_transitions_t transitions = {
    .success = states[0],
    .error = states[1]
  };
  sl_rail_status_t ret = sl_rail_set_rx_transitions(railHandle, &transitions);
  if (ret == SL_RAIL_STATUS_NO_ERROR) {
    rxSuccessTransition = states[0];
  }
  responsePrint(sl_cli_get_command_string(args, 0), "RxTransitions:%s", (ret ? "Error" : "Set"));
}

void getRxTransitions(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  sl_rail_state_transitions_t transitions;
  sl_rail_status_t ret = sl_rail_get_rx_transitions(railHandle, &transitions);

  if (ret == SL_RAIL_STATUS_NO_ERROR) {
    const char *success = getRfStateName(transitions.success);
    const char *error = getRfStateName(transitions.error);
    responsePrint(sl_cli_get_command_string(args, 0), "Success:%s,Error:%s", success, error);
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x28, "Get rx transitions failed");
  }
}

void getTxTransitions(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  sl_rail_state_transitions_t transitions;
  sl_rail_status_t ret = sl_rail_get_tx_transitions(railHandle, &transitions);

  if (ret == SL_RAIL_STATUS_NO_ERROR) {
    const char *success = getRfStateName(transitions.success);
    const char *error = getRfStateName(transitions.error);
    responsePrint(sl_cli_get_command_string(args, 0), "Success:%s,Error:%s", success, error);
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x27, "Get tx transitions failed");
  }
}

void setTimings(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  sl_rail_state_timing_t timings = {
    .idle_to_rx = (sl_rail_transition_time_t)sl_cli_get_argument_int32(args, 0),
    .tx_to_rx   = (sl_rail_transition_time_t)sl_cli_get_argument_int32(args, 1),
    .idle_to_tx = (sl_rail_transition_time_t)sl_cli_get_argument_int32(args, 2),
  };
  switch (sl_cli_get_argument_count(args)) {
    default:
    case 7:
      timings.tx_to_tx = (sl_rail_transition_time_t)sl_cli_get_argument_int32(args, 6);
    // Fall through
    case 6:
      timings.tx_to_rxsearch_timeout = (sl_rail_transition_time_t)sl_cli_get_argument_int32(args, 5);
    // Fall through
    case 5:
      timings.rxsearch_timeout = (sl_rail_transition_time_t)sl_cli_get_argument_int32(args, 4);
    // Fall through
    case 4:
      timings.rx_to_tx = (sl_rail_transition_time_t)sl_cli_get_argument_int32(args, 3);
    // Fall through
    case 3: // .idle_to_tx not optional, already set above
    case 2: // .tx_to_rx   not optional, already set above
    case 1: // .idle_to_rx not optional, already set above
    case 0: // Should be impossible
      break;
  }
  if (!sl_rail_set_state_timing(railHandle, &timings)) {
    responsePrint(sl_cli_get_command_string(args, 0),
                  "IdleToRx:%u,RxToTx:%u,IdleToTx:%u,TxToRx:%u,"
                  "RxSearch:%u,Tx2RxSearch:%u,Tx2Tx:%u",
                  timings.idle_to_rx, timings.rx_to_tx, timings.idle_to_tx,
                  timings.tx_to_rx, timings.rxsearch_timeout,
                  timings.tx_to_rxsearch_timeout, timings.tx_to_tx);
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x18, "Setting timings failed");
  }
}

void setTxFifoThreshold(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  if (railTxDataConfig.tx_method != SL_RAIL_DATA_METHOD_FIFO_MODE) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x19, "Tx is not in FIFO mode");
    return;
  }

  uint16_t txFifoThreshold = sl_cli_get_argument_uint16(args, 0);
  txFifoThreshold = sl_rail_set_tx_fifo_threshold(railHandle, txFifoThreshold);
  responsePrint(sl_cli_get_command_string(args, 0), "TxFifoThreshold:%d", txFifoThreshold);
}

void setRxFifoThreshold(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  uint16_t rxFifoThreshold = sl_cli_get_argument_uint16(args, 0);
  rxFifoThreshold = sl_rail_set_rx_fifo_threshold(railHandle, rxFifoThreshold);
  responsePrint(sl_cli_get_command_string(args, 0), "RxFifoThreshold:%d", rxFifoThreshold);
}

void setEventConfig(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  sl_rail_events_t eventMask = sl_cli_get_argument_uint32(args, 0);
  sl_rail_events_t eventConfig = sl_cli_get_argument_uint32(args, 1);

  if (sl_cli_get_argument_count(args) >= 4) {
    eventMask |= (((sl_rail_events_t)sl_cli_get_argument_uint32(args, 2)) << 32U);
    eventConfig |= (((sl_rail_events_t)sl_cli_get_argument_uint32(args, 3)) << 32U);
  }

  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  sl_rail_config_events(railHandle, eventMask, eventConfig);
  // Avoid use of %ll long-long formats due to iffy printf library support
  if (sl_cli_get_argument_count(args) >= 4) {
    responsePrint(sl_cli_get_command_string(args, 0), "Mask:0x%x%08x,Values:0x%x%08x",
                  (uint32_t)(eventMask >> 32),
                  (uint32_t)eventMask,
                  (uint32_t)(eventConfig >> 32),
                  (uint32_t)eventConfig);
  } else {
    responsePrint(sl_cli_get_command_string(args, 0), "Mask:0x%x,Values:0x%x",
                  (uint32_t)eventMask,
                  (uint32_t)eventConfig);
  }
}

void delayUs(sl_cli_command_arg_t *args)
{
  uint32_t delayUs = sl_cli_get_argument_uint32(args, 0);

  // Do not measure any interrupt processing overhead during the delay.
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  // Measure the actual delay vs expected.
  uint32_t startTime = sl_rail_get_time(railHandle);
  sl_rail_status_t status = sl_rail_delay_us(railHandle, delayUs);
  uint32_t actualDelay = sl_rail_get_time(railHandle) - startTime;

  CORE_EXIT_CRITICAL();
  responsePrint(sl_cli_get_command_string(args, 0), "Success:%s,ActualDelay:%d",
                status == SL_RAIL_STATUS_NO_ERROR ? "True" : "False",
                actualDelay);
}

void configPaAutoMode(sl_cli_command_arg_t *args)
{
  // Fail command not supported in RAIL 3
  responsePrintError(sl_cli_get_command_string(args, 0), 0x01, "PA Auto Mode is built into RAIL 3, not configureable.");
}

void enablePaAutoMode(sl_cli_command_arg_t *args)
{
  // Fake 'success' for deprecated command not supported in RAIL 3
  responsePrint(sl_cli_get_command_string(args, 0), "enable:True");
}

void setRetimeOption(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  uint32_t option = sl_cli_get_argument_uint8(args, 0);
  sl_rail_retime_options_t finalRetimeOption;
  sl_rail_status_t status;

  status = sl_rail_config_retime_options(railHandle,
                                         SL_RAIL_RETIME_OPTIONS_ALL,
                                         option);

  // Report the current enabled status
  if (status == SL_RAIL_STATUS_NO_ERROR) {
    sl_rail_get_retime_options(railHandle,
                               &finalRetimeOption);
    responsePrint(sl_cli_get_command_string(args, 0),
#ifdef _SILICON_LABS_32B_SERIES_2
                  "LCD:%s,HFXO:%s,HFRCO:%s,DCDC:%s",
                  ((finalRetimeOption & SL_RAIL_RETIME_OPTION_LCD) != 0U) ? "Enabled" : "Disabled",
#else
                  "HFXO:%s,HFRCO:%s,DCDC: %s",
#endif
                  ((finalRetimeOption & SL_RAIL_RETIME_OPTION_HFXO) != 0U) ? "Enabled" : "Disabled",
                  ((finalRetimeOption & SL_RAIL_RETIME_OPTION_HFRCO) != 0U) ? "Enabled" : "Disabled",
                  ((finalRetimeOption & SL_RAIL_RETIME_OPTION_DCDC) != 0U) ? "Enabled" : "Disabled");
  } else {
    responsePrint(sl_cli_get_command_string(args, 0), "Status:%d", status);
  }
}

void getWhiteningInitVal(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  uint32_t whiteInit = sl_rail_get_whitening_init_val(railHandle);
  responsePrint(sl_cli_get_command_string(args, 0),
                "whiteInit:%u",
                whiteInit);
}

void getCrcInitVal(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  uint32_t crcInit = sl_rail_get_crc_init_val(railHandle);
  responsePrint(sl_cli_get_command_string(args, 0),
                "crcInit:%u",
                crcInit);
}

void setWhiteningInitVal(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  uint32_t whiteInit = sl_cli_get_argument_uint32(args, 0);
  sl_rail_status_t status = sl_rail_set_whitening_init_val(railHandle, whiteInit);
  responsePrint(sl_cli_get_command_string(args, 0),
                "status:%s",
                (status == SL_RAIL_STATUS_NO_ERROR) ? "Success" : "Fail");
}

void setCrcInitVal(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  uint32_t crcInit = sl_cli_get_argument_uint32(args, 0);
  sl_rail_status_t status = sl_rail_set_crc_init_val(railHandle, crcInit);
  responsePrint(sl_cli_get_command_string(args, 0),
                "status:%s",
                (status == SL_RAIL_STATUS_NO_ERROR) ? "Success" : "Fail");
}

void resetWhiteningInitVal(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  sl_rail_status_t status = sl_rail_reset_whitening_init_val(railHandle);
  responsePrint(sl_cli_get_command_string(args, 0),
                "status:%s",
                (status == SL_RAIL_STATUS_NO_ERROR) ? "Success" : "Fail");
}

void resetCrcInitVal(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  sl_rail_status_t status = sl_rail_reset_crc_init_val(railHandle);
  responsePrint(sl_cli_get_command_string(args, 0),
                "status:%s",
                (status == SL_RAIL_STATUS_NO_ERROR) ? "Success" : "Fail");
}

#if ((!defined(SL_CATALOG_SL_RAIL_UTIL_PA_PRESENT))  &&  SL_RAIL_UTIL_PA_NVM_ENABLED)

#include "sl_rail_util_pa_nvm_configs.h"
#include "nvm3_default.h"

extern RAIL_TxPowerCurvesConfigAlt_t powerCurvesState;

void printPowerCurves(sl_cli_command_arg_t *args)
{
  char where = sl_cli_get_argument_string(args, 0)[0];
  char format = 'u'; // Unserialized C
  if (sl_cli_get_argument_count(args) > 1) {
    format = sl_cli_get_argument_string(args, 1)[0];
  }
  RAIL_TxPowerCurvesConfigAlt_t *p_deserialized_curves = &powerCurvesState;
  sl_rail_nvm_pa_config_t pa_config;
  sl_status_t status;
  switch (where) {
    case 'c':
      break;
    case 'n':
      status = sl_rail_util_pa_nvm_read_config(&pa_config);
      if (status != SL_STATUS_OK) {
        responsePrintError(sl_cli_get_command_string(args, 0), status,
                           "Missing or invalid PA config in NVM");
        return;
      }
      // switch unserialized to deserialized for an NVM config
      if (format == 'u') {
        format = 'd';
      }
      break;
    default:
      responsePrintError(sl_cli_get_command_string(args, 0), 0x11,
                         "Unrecognized location %s, expected 'chip' or 'nvm'",
                         sl_cli_get_argument_string(args, 0));
      return;
      break;
  }
  switch (format) {
    case 'u': // unserialized C
      sli_rail_util_pa_nvm_print_deserialized(p_deserialized_curves);
      break;
    case 's': // serialized C
    case 'o': // serialized C with offsets
    case 'j': // serialized JSON
    case 'd': // deserialized C
      if (where == 'c') {
        status = sli_rail_util_pa_nvm_serialize_config(p_deserialized_curves, &pa_config);
        if (status != SL_STATUS_OK) {
          responsePrintError(sl_cli_get_command_string(args, 0), status,
                             "Chip power curves failed to serialize");
          return;
        }
      }
      if (format == 's') {
        sli_rail_util_pa_nvm_print_serialized(&pa_config, false);
      } else if (format == 'o') {
        sli_rail_util_pa_nvm_print_serialized(&pa_config, true);
      } else if (format == 'j') {
        sli_rail_util_pa_nvm_print_serialized_json(&pa_config);
      } else {
        p_deserialized_curves
          = sli_rail_util_pa_nvm_deserialize_config(&pa_config);
        if (p_deserialized_curves == NULL) {
          responsePrintError(sl_cli_get_command_string(args, 0), 0x11,
                             "Serialized power curves failed to deserialize");
          return;
        }
        sli_rail_util_pa_nvm_print_deserialized(p_deserialized_curves);
      }
      break;
    default:
      responsePrintError(sl_cli_get_command_string(args, 0), 0x11,
                         "Unrecognized format %s", sl_cli_get_argument_string(args, 1));
      break;
  }
  responsePrint(sl_cli_get_command_string(args, 0),
                "sizeof(sl_rail_nvm_pa_config_t):%u,status:Success",
                sizeof(sl_rail_nvm_pa_config_t));
}

void testNvmPaMode(sl_cli_command_arg_t *args)
{
  sl_status_t status;
  RAIL_TxPowerMode_t pa_mode = getPowerModeIndexFromString(sl_cli_get_argument_string(args, 0));
  if (pa_mode == 0xFFU) {
    // If the string doesn't match a powerMode, attempt to convert it to a RAIL_TxPowerMode_t.
    char *strtoulEnd;
    pa_mode = (RAIL_TxPowerMode_t)strtoul(sl_cli_get_argument_string(args, 0), &strtoulEnd, 0);
    //check that strtoul didn't fail but allow an invalid power mode
    if (strtoulEnd == sl_cli_get_argument_string(args, 0)) {
      responsePrintError(sl_cli_get_command_string(args, 0), 0x13, "Invalid PA enum value selected: %s", sl_cli_get_argument_string(args, 0));
      return;
    }
  }
  if (pa_mode == 0xFFU) {
    // No API for this, so use native NVM3 APIs
    status = nvm3_deleteObject(nvm3_defaultHandle,
                               SL_RAIL_UTIL_PA_NVM_MODE_TAG);
    responsePrint(sl_cli_get_command_string(args, 0), "nvmTag:0x%x,deleteNvmStatus:%u",
                  SL_RAIL_UTIL_PA_NVM_MODE_TAG, status);
    return;
  }
  status = sl_rail_util_pa_nvm_write_mode(pa_mode);
  if (status != SL_STATUS_OK) {
    responsePrintError(sl_cli_get_command_string(args, 0), status,
                       "Failed to write PA mode %u", pa_mode);
    return;
  }
  RAIL_TxPowerMode_t read_pa_mode = sl_rail_util_pa_nvm_read_mode();
  responsePrint(sl_cli_get_command_string(args, 0),
                "writePaMode:%u,readPaMode:%u,readPaString:%s",
                pa_mode, read_pa_mode, paStrings[read_pa_mode]);
}

void testNvmPowerCurves(sl_cli_command_arg_t *args)
{
  sl_rail_nvm_pa_config_t pa_config;
  char operation = 'w'; // Write a modified curve to NVM
  sl_status_t status;
  if (sl_cli_get_argument_count(args) > 0) {
    operation = sl_cli_get_argument_string(args, 0)[0];
  }
  switch (operation) {
    case 'w': // write modified curve to NVM
    {
      RAIL_TxPowerCurvesConfigAlt_t new_curves = powerCurvesState; // struct copy
      // Change it by swapping the curves
      new_curves.curves[0] = powerCurvesState.curves[1]; // struct copy
      new_curves.curves[1] = powerCurvesState.curves[0]; // struct copy
      status = sli_rail_util_pa_nvm_serialize_config(&new_curves, &pa_config);
      if (status != SL_STATUS_OK) {
        responsePrintError(sl_cli_get_command_string(args, 0), status,
                           "Failed to serialize modified curve");
        return;
      }
      status = sl_rail_util_pa_nvm_write_config(&pa_config);
      responsePrint(sl_cli_get_command_string(args, 0), "writeNvmStatus:%u", status);
      break;
    }
    case 'c': // corrupt curve in NVM (by changing its version)
      // Start by reading curve from NVM
      status = sl_rail_util_pa_nvm_read_config(&pa_config);
      if (status != SL_STATUS_OK) {
        responsePrintError(sl_cli_get_command_string(args, 0), status,
                           "Missing or invalid PA config in NVM");
        return;
      }
      pa_config.version = 0U;
      status = sl_rail_util_pa_nvm_write_config(&pa_config);
      responsePrint(sl_cli_get_command_string(args, 0), "writeNvmStatus:%u", status);
      break;
    case 'd': // delete curve in NVM
      // No API for this, so use native NVM3 APIs
      status = nvm3_deleteObject(nvm3_defaultHandle,
                                 SL_RAIL_UTIL_PA_NVM_CONFIG_TAG);
      responsePrint(sl_cli_get_command_string(args, 0), "nvmTag:0x%x,deleteNvmStatus:%u",
                    SL_RAIL_UTIL_PA_NVM_CONFIG_TAG, status);
      break;
    default:
      responsePrintError(sl_cli_get_command_string(args, 0), 0x11,
                         "Unrecognized operation %s", sl_cli_get_argument_string(args, 0));
      break;
  }
}

#else// !((!defined(SL_CATALOG_SL_RAIL_UTIL_PA_PRESENT))  &&  SL_RAIL_UTIL_PA_NVM_ENABLED)

void printPowerCurves(sl_cli_command_arg_t *args)
{
  responsePrintError(sl_cli_get_command_string(args, 0), 0x11, "This command is not supported on this platform");
}

void testNvmPaMode(sl_cli_command_arg_t *args)
{
  responsePrintError(sl_cli_get_command_string(args, 0), 0x11, "This command is not supported on this platform");
}

void testNvmPowerCurves(sl_cli_command_arg_t *args)
{
  responsePrintError(sl_cli_get_command_string(args, 0), 0x11, "This command is not supported on this platform");
}

#endif// ((!defined(SL_CATALOG_SL_RAIL_UTIL_PA_PRESENT))  &&  SL_RAIL_UTIL_PA_NVM_ENABLED)
