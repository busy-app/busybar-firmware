/***************************************************************************//**
 * @file
 * @brief This file implements the tx/rx commands for sl_rail_test applications.
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
#include <inttypes.h>

#if !defined(__ICCARM__)
// IAR doesn't have strings.h and puts those declarations in string.h
#include <strings.h>
#endif

#include "response_print.h"

#include "sl_rail.h"

#include "sl_hal_gpio.h"

#include "app_common.h"
#include "app_trx.h"

#include "sl_power_manager.h"
#include "../src/sleep_loop/sli_power_manager_private.h"

#if defined(SL_CATALOG_IOSTREAM_EUSART_PRESENT)
#include "sl_iostream_eusart_vcom_config.h"
#if defined(_SILICON_LABS_32B_SERIES_2)
#include "em_eusart.h"
#else
#include "sl_hal_eusart.h"
#endif
#endif

#if defined(SL_CATALOG_IOSTREAM_USART_PRESENT)
#include "sl_iostream_usart_vcom_config.h"
#endif

#if (defined(SL_CATALOG_IOSTREAM_USART_PRESENT) || defined(SL_CATALOG_IOSTREAM_EUSART_PRESENT)) && defined(SL_CATALOG_CLI_PRESENT)
  #if defined(SL_CLI_USE_STDIO)
    #include "sl_iostream.h"
    #define consumeChar()                               \
  do {                                                  \
    char ch;                                            \
    (void) sl_iostream_getchar(SL_IOSTREAM_STDIN, &ch); \
  } while (0)
  #else
    #define consumeChar() ((void) getchar())
  #endif // #if defined(SL_CLI_USE_STDIO)
#else
  #define consumeChar()
#endif // #if (defined(SL_CATALOG_IOSTREAM_USART_PRESENT)
// || defined(SL_CATALOG_IOSTREAM_EUSART_PRESENT)) && defined(SL_CATALOG_CLI_PRESENT)

static sl_rail_status_t identityTimestampTx(sl_rail_handle_t railHandle,
                                            sl_rail_tx_packet_details_t *pPacketDetails)
{
  (void) railHandle;
  (void) pPacketDetails;
  return SL_RAIL_STATUS_NO_ERROR;
}

static sl_rail_status_t identityTimestampRx(sl_rail_handle_t railHandle,
                                            sl_rail_rx_packet_details_t *pPacketDetails)
{
  (void) railHandle;
  (void) pPacketDetails;
  return SL_RAIL_STATUS_NO_ERROR;
}

TxTimestampFunc txTimePosition = &sl_rail_get_tx_time_frame_end;
RxTimestampFunc rxTimePosition = &sl_rail_get_rx_time_sync_word_end;

void tx(sl_cli_command_arg_t *args)
{
  uint32_t newTxCount = sl_cli_get_argument_uint32(args, 0);
  radioTransmit(newTxCount, sl_cli_get_command_string(args, 0));
}

static const char *configuredTxAntenna(sl_rail_tx_options_t txOptions)
{
  switch (txOptions & (SL_RAIL_TX_OPTION_ANTENNA_0 | SL_RAIL_TX_OPTION_ANTENNA_1)) {
    case (SL_RAIL_TX_OPTION_ANTENNA_0 | SL_RAIL_TX_OPTION_ANTENNA_1): {
      return "Any";
      break;
    }
    case (SL_RAIL_TX_OPTION_ANTENNA_0): {
      return "Antenna0";
      break;
    }
    case (SL_RAIL_TX_OPTION_ANTENNA_1): {
      return "Antenna1";
      break;
    }
    default: {
      return "Any";
      break;
    }
  }
}

void configTxOptions(sl_cli_command_arg_t *args)
{
  if (sl_cli_get_argument_count(args) >= 1) {
    txOptions = sl_cli_get_argument_uint32(args, 0);
  }

  responsePrint(sl_cli_get_command_string(args, 0), "waitForAck:%s,removeCrc:%s,syncWordId:%d,"
                                                    "txAntenna:%s,altPreambleLen:%s,"
                                                    "ccaOnly:%s,resend:%s,concurrentPhyId:%d",
                ((txOptions & SL_RAIL_TX_OPTION_WAIT_FOR_ACK) ? "True" : "False"),
                ((txOptions & SL_RAIL_TX_OPTION_REMOVE_CRC) ? "True" : "False"),
                ((txOptions & SL_RAIL_TX_OPTION_SYNC_WORD_ID) >> SL_RAIL_TX_OPTION_SYNC_WORD_ID_SHIFT),
                configuredTxAntenna(txOptions),
                ((txOptions & SL_RAIL_TX_OPTION_ALT_PREAMBLE_LEN) ? "True" : "False"),
                ((txOptions & SL_RAIL_TX_OPTION_CCA_ONLY) ? "True" : "False"),
                ((txOptions & SL_RAIL_TX_OPTION_RESEND) ? "True" : "False"),
                ((txOptions & SL_RAIL_TX_OPTION_CONCURRENT_PHY_ID) >> SL_RAIL_TX_OPTION_CONCURRENT_PHY_ID_SHIFT));
}

static void txAtTimeHelper(sl_cli_command_arg_t *args, uint8_t txCount)
{
  int8_t startArgPos = 0;
  if (txCount > 1U) {
    startArgPos = 1;
  }
  // DEFAULTS: 0 ms, absolute time, postponse tx during rx
  sl_rail_scheduled_tx_config_t scheduledTxOptions = { 0 };
  scheduledTxOptions.when = sl_cli_get_argument_uint32(args, startArgPos + 0);

  // Attempt to parse the time mode if specified
  int8_t optArgPos = startArgPos + 1;
  if (sl_cli_get_argument_count(args) > optArgPos) {
    if (!parseTimeModeFromString(sl_cli_get_argument_string(args, optArgPos), &scheduledTxOptions.mode)) {
      responsePrintError(sl_cli_get_command_string(args, 0), 28, "Invalid time mode");
      return;
    }
  }

  optArgPos = startArgPos + 2;
  scheduledTxOptions.tx_during_rx =
    ((sl_cli_get_argument_count(args) > optArgPos)
     && strcasecmp("abort", sl_cli_get_argument_string(args, optArgPos)) == 0)
    ? SL_RAIL_SCHEDULED_TX_DURING_RX_ABORT_TX
    : SL_RAIL_SCHEDULED_TX_DURING_RX_POSTPONE_TX;

  if (txCount > 1) {
    // For multi scheduled TX, convert to absolute time to generate Tx anchor time
    if (scheduledTxOptions.mode == SL_RAIL_TIME_DELAY) {
      scheduledTxOptions.when += sl_rail_get_time(railHandle);
      scheduledTxOptions.mode = SL_RAIL_TIME_ABSOLUTE;
    }
  }

  setNextPacketTime(&scheduledTxOptions);

  if (txCount > 1) {
    radioTransmitGeneric(txCount, true, sl_cli_get_command_string(args, 0));
  } else {
    setNextAppMode(TX_SCHEDULED, sl_cli_get_command_string(args, 0));
  }
}

void txAtTime(sl_cli_command_arg_t *args)
{
  txAtTimeHelper(args, 1);
}

void txAtTimeN(sl_cli_command_arg_t *args)
{
  uint32_t txSchedCount = sl_cli_get_argument_uint32(args, 0);
  if (txSchedCount == 0U) {
    responsePrintError(sl_cli_get_command_string(args, 0), 28, "Invalid tx count");
    return;
  }
  txAtTimeHelper(args, txSchedCount);
}

void txAfterRx(sl_cli_command_arg_t *args)
{
  uint32_t delay = sl_cli_get_argument_uint32(args, 0);
  txAfterRxDelay = delay;
  enableAppMode(SCHTX_AFTER_RX, (delay != 0), sl_cli_get_command_string(args, 0));
}

void getTxDelay(sl_cli_command_arg_t *args)
{
  responsePrint(sl_cli_get_command_string(args, 0), "txDelay:%d", continuousTransferPeriod);
}

void setTxDelay(sl_cli_command_arg_t *args)
{
  uint32_t delay = sl_cli_get_argument_uint32(args, 0);

  continuousTransferPeriod = delay;
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  getTxDelay(args);
}

void getRandomTxDelay(sl_cli_command_arg_t *args)
{
  responsePrint(sl_cli_get_argument_string(args, 0),
                "enableRandomTxDelay:%d", enableRandomTxDelay);
}

void setRandomTxDelay(sl_cli_command_arg_t *args)
{
  uint32_t enable = sl_cli_get_argument_uint32(args, 0);

  enableRandomTxDelay = enable;
  getRandomTxDelay(args);
}

void setTxHoldOff(sl_cli_command_arg_t *args)
{
  uint32_t holdOff = sl_cli_get_argument_uint32(args, 0);

  sl_rail_enable_tx_hold_off(railHandle, holdOff);
  responsePrint(sl_cli_get_command_string(args, 0), "TxHoldOff:%s",
                (sl_rail_is_tx_hold_off_enabled(railHandle) ? "Enabled" : "Disabled"));
}

void setTxAltPreambleLen(sl_cli_command_arg_t *args)
{
  uint16_t length = sl_cli_get_argument_uint16(args, 0);

  sl_rail_status_t status = sl_rail_set_tx_alt_preamble_length(railHandle, length);
  responsePrint(sl_cli_get_command_string(args, 0), "Result:%s",
                ((status == SL_RAIL_STATUS_NO_ERROR) ? "Success" : "Failure"));
}

void stopInfinitePreambleTx(sl_cli_command_arg_t *args)
{
  sl_rail_status_t status = sl_rail_stop_infinite_preamble_tx(railHandle);
  responsePrint(sl_cli_get_command_string(args, 0), "Result:%s", getStatusMessage(status));
}

void getSyncWords(sl_cli_command_arg_t *args)
{
  sl_rail_sync_word_config_t syncWordConfig = { 0, };
  sl_rail_status_t status = sl_rail_get_sync_words(railHandle, &syncWordConfig);
  responsePrint(sl_cli_get_command_string(args, 0),
                "Result:%s,bitlength:%u,syncWord1:%" PRIu32 ",syncWord2:%" PRIu32,
                status == SL_RAIL_STATUS_NO_ERROR ? "Success" : "Failure",
                syncWordConfig.sync_word_bits, syncWordConfig.sync_word_0,
                syncWordConfig.sync_word_1);
}

void configSyncWords(sl_cli_command_arg_t *args)
{
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }
  sl_rail_sync_word_config_t syncWordConfig = { 0, };
  syncWordConfig.sync_word_bits = sl_cli_get_argument_uint8(args, 0);
  syncWordConfig.sync_word_0 = sl_cli_get_argument_uint32(args, 1);
  if (sl_cli_get_argument_count(args) >= 3) {
    syncWordConfig.sync_word_1 = sl_cli_get_argument_uint32(args, 2);
  } else {
    syncWordConfig.sync_word_1 = syncWordConfig.sync_word_0;
  }
  sl_rail_status_t status = sl_rail_config_sync_words(railHandle, &syncWordConfig);
  responsePrint(sl_cli_get_command_string(args, 0), "Result:%s",
                ((status == SL_RAIL_STATUS_NO_ERROR) ? "Success" : "Failure"));
}

#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH && defined(WISUN_MODESWITCHPHRS_ARRAY_SIZE)
uint16_t modeSwitchNewChannel = 0xFFFFU;
uint16_t modeSwitchBaseChannel = 0xFFFFU;
ModeSwitchState_t modeSwitchState = IDLE;
#endif

void rx(sl_cli_command_arg_t *args)
{
  bool enable = !!sl_cli_get_argument_uint8(args, 0);
  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;

  // Don't allow Rx enable calls when scheduled Rx has been started
  if ((currentAppMode() == RX_SCHEDULED) && enable) {
    responsePrintError(sl_cli_get_command_string(args, 0), 29, "Can't turn on receive when in ScheduledRx");
    return;
  }

  // Only allow calls to Rx if we're in direct mode, no mode, or this is a
  // scheduled Rx disable call
  if ((currentAppMode() != DIRECT)
      && (currentAppMode() != RX_SCHEDULED)
      && (!inAppMode(NONE, sl_cli_get_command_string(args, 0)))) {
    return;
  }

  // Do the minimum amount of work to get into the correct state
  sl_rail_radio_state_t currentState = sl_rail_get_radio_state(railHandle);
  if ((enable && (currentState & SL_RAIL_RF_STATE_RX))
      || (!enable && (currentState <= SL_RAIL_RF_STATE_IDLE))) {
    // Do nothing since we're already in the right state
  } else if (enable) {
    status = sl_rail_start_rx(railHandle, channel, NULL);
  } else {
    sl_rail_idle(railHandle, SL_RAIL_IDLE_ABORT, false);
#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH && defined(WISUN_MODESWITCHPHRS_ARRAY_SIZE)
    // End mode switch operation if the radio is turned off
    modeSwitchBaseChannel = 0xFFFFU;
    modeSwitchNewChannel = 0xFFFFU;
    modeSwitchState = IDLE;
#endif
    // Turn off ScheduledRx if we were in it
    if (currentAppMode() == RX_SCHEDULED) {
      (void) enableAppModeSync(RX_SCHEDULED, false, NULL);
    }
  }
  if (status == SL_RAIL_STATUS_NO_ERROR) {
    receiveModeEnabled = enable;

    // Print out the current status of receive mode
    responsePrint(sl_cli_get_command_string(args, 0),
                  "Rx:%s,Idle:%s,Time:%u",
                  (enable ? "Enabled" : "Disabled"),
                  ((!enable) ? "Enabled" : "Disabled"),
                  sl_rail_get_time(railHandle));
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0),
                       40,
                       "Could not change receive state '%d'",
                       status);
  }
}

void rxAt(sl_cli_command_arg_t *args)
{
  uint32_t startTime, endTime;
  sl_rail_time_mode_t startMode, endMode;
  bool rxTransitionEndSchedule = false, hardEnd = false;
  bool scheduledRxUpdate = false;
  sl_rail_scheduled_rx_config_t rxCfg;

  // Only allow this when app is idle or to reconfigure an active scheduled Rx
  if (!inAppMode(NONE, NULL) && !inAppMode(RX_SCHEDULED, NULL)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 30, "Cannot enter ScheduledRx when not in Idle.");
    return;
  }

  // Parse out the command line parameters
  startTime = sl_cli_get_argument_uint32(args, 0);
  endTime   = sl_cli_get_argument_uint32(args, 2);

  if (!parseTimeModeFromString(sl_cli_get_argument_string(args, 1), &startMode)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 25, "Invalid startMode specified");
    return;
  }
  if (!parseTimeModeFromString(sl_cli_get_argument_string(args, 3), &endMode)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 26, "Invalid endMode specified");
    return;
  }

  // Read the rxTransitionEndSchedule option if available
  if (sl_cli_get_argument_count(args) >= 5) {
    rxTransitionEndSchedule = !!sl_cli_get_argument_uint8(args, 4);
  }
  // Read the hardEnd option if available
  if (sl_cli_get_argument_count(args) >= 6) {
    hardEnd = !!sl_cli_get_argument_uint8(args, 5);
  }

  // Configure scheduled receive as requested
  rxCfg.start = startTime;
  rxCfg.start_mode = startMode;
  rxCfg.end = endTime;
  rxCfg.end_mode = endMode;
  rxCfg.rx_transition_end_schedule = rxTransitionEndSchedule;
  rxCfg.hard_window_end = hardEnd;

  // Store whether this is an update or not
  if (inAppMode(RX_SCHEDULED, NULL)) {
    scheduledRxUpdate = true;
  }

  if (!scheduledRxUpdate) {
    // Attempt to put the app into scheduled receive mode
    if (!enableAppModeSync(RX_SCHEDULED, true, sl_cli_get_command_string(args, 0))) {
      return;
    }
  }

  // Enable scheduled receive mode
  sl_rail_status_t res = sl_rail_start_scheduled_rx(railHandle, channel, &rxCfg, NULL);
  if (res != SL_RAIL_STATUS_NO_ERROR) {
    responsePrintError(sl_cli_get_command_string(args, 0), 27, "Could not start scheduled receive %u", res);
    if (!scheduledRxUpdate) {
      (void) enableAppModeSync(RX_SCHEDULED, false, sl_cli_get_command_string(args, 0));
    }
    return;
  }
}

void setRxOptions(sl_cli_command_arg_t *args)
{
  // Only update the rxOptions if a parameter is given otherwise just print the
  // current settings
  if (sl_cli_get_argument_count(args) >= 1) {
    sl_rail_rx_options_t newRxOptions = sl_cli_get_argument_uint32(args, 0);
    sl_rail_status_t status = sl_rail_config_rx_options(railHandle,
                                                        SL_RAIL_RX_OPTIONS_ALL,
                                                        newRxOptions);

    // Make sure there was no error setting the new options
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      responsePrintError(sl_cli_get_command_string(args, 0), 31, "RxOptions:Failed");
      return;
    }
    // Update the global rxOptions
    rxOptions = newRxOptions;
  }

  responsePrint(sl_cli_get_command_string(args, 0),
                "storeCrc:%s,ignoreCrcErrors:%s,enableDualSync:%s,"
                "trackAborted:%s,removeAppendedInfo:%s,rxAntenna:%s,"
                "frameDet:%s,skipDCCal:%s,skipSynthCal:%s,rxChannelSwitching:%s,"
                "fastRx2Rx:%s,collisionDetection:%s",
                (rxOptions & SL_RAIL_RX_OPTION_STORE_CRC) ? "True" : "False",
                (rxOptions & SL_RAIL_RX_OPTION_IGNORE_CRC_ERRORS) ? "True" : "False",
                (rxOptions & SL_RAIL_RX_OPTION_ENABLE_DUAL_SYNC) ? "True" : "False",
                (rxOptions & SL_RAIL_RX_OPTION_TRACK_ABORTED_FRAMES) ? "True" : "False",
                (rxOptions & SL_RAIL_RX_OPTION_REMOVE_APPENDED_INFO) ? "True" : "False",
                configuredRxAntenna(rxOptions),
                (rxOptions & SL_RAIL_RX_OPTION_DISABLE_FRAME_DETECTION) ? "Off" : "On",
                (rxOptions & SL_RAIL_RX_OPTION_SKIP_DC_CAL) ? "True" : "False",
                (rxOptions & SL_RAIL_RX_OPTION_SKIP_SYNTH_CAL) ? "True" : "False",
                (rxOptions & SL_RAIL_RX_OPTION_CHANNEL_SWITCHING) ? "True" : "False",
#if SL_RAIL_SUPPORTS_FAST_RX_TO_RX
                (rxOptions & SL_RAIL_RX_OPTION_FAST_RX_TO_RX) ? "True" : "False",
#else
                "False",
#endif
                (rxOptions & SL_RAIL_RX_OPTION_ENABLE_COLLISION_DETECTION) ? "On" : "Off");
}

void setTxTone(sl_cli_command_arg_t *args)
{
  uint8_t enable = sl_cli_get_argument_uint8(args, 0);
  streamMode = SL_RAIL_STREAM_CARRIER_WAVE;
  antOptions = SL_RAIL_TX_OPTIONS_DEFAULT;
  if (sl_cli_get_argument_count(args) >= 2) {
    uint8_t antenna = sl_cli_get_argument_uint32(args, 1);
    //Choose any antenna by default
    if (antenna == 1) {
      antOptions = SL_RAIL_TX_OPTION_ANTENNA_1;
    } else {
      antOptions = SL_RAIL_TX_OPTION_ANTENNA_0;
    }
  }
  if (sl_cli_get_argument_count(args) >= 3) {
    if (sl_cli_get_argument_uint32(args, 2) > 0) {
      streamMode = SL_RAIL_STREAM_CARRIER_WAVE_PHASENOISE;
    }
  }
  enableAppMode(TX_STREAM, enable, sl_cli_get_command_string(args, 0));
}

void setTxStream(sl_cli_command_arg_t *args)
{
  uint8_t enable = sl_cli_get_argument_uint8(args, 0);
  sl_rail_stream_mode_t stream = SL_RAIL_STREAM_PN9_STREAM;
  antOptions = SL_RAIL_TX_OPTIONS_DEFAULT;
  if (sl_cli_get_argument_count(args) >= 3) {
    uint8_t antenna = sl_cli_get_argument_uint32(args, 2);
    //Choose any antenna by default
    if (antenna == 1) {
      antOptions = SL_RAIL_TX_OPTION_ANTENNA_1;
    } else {
      antOptions = SL_RAIL_TX_OPTION_ANTENNA_0;
    }
  }
  concPhyIdOptions = txOptions & SL_RAIL_TX_OPTION_CONCURRENT_PHY_ID;
  if (sl_cli_get_argument_count(args) >= 2) {
    stream = (sl_rail_stream_mode_t) sl_cli_get_argument_uint32(args, 1);
    if (stream >= SL_RAIL_STREAM_MODES_COUNT) {
      responsePrintError(sl_cli_get_command_string(args, 0), 1, "Invalid stream mode");
      return;
    }
  }
  //in ofdm case the PHR is read in the BUFC to provide MCS and packet length information
  //thus write it here again. It is not a problem to write it even for other modulations
  //as the TX fifo is reset
  if (railTxDataConfig.tx_method == SL_RAIL_DATA_METHOD_PACKET_MODE) {
    sl_rail_write_tx_fifo(railHandle, txData, 4, true);
  }
  streamMode = stream;
  enableAppMode(TX_STREAM, enable, sl_cli_get_command_string(args, 0));
}

void configDirectMode(sl_cli_command_arg_t *args)
{
  sl_rail_status_t status = SL_RAIL_STATUS_INVALID_CALL;
#if ((_SILICON_LABS_32B_SERIES == 1) || (_SILICON_LABS_32B_SERIES_2_CONFIG >= 3))
  sl_rail_direct_mode_config_t directModeConfig = { 0 };

  directModeConfig.sync_rx = sl_cli_get_argument_uint8(args, 0);
  directModeConfig.sync_tx = sl_cli_get_argument_uint8(args, 1);
  directModeConfig.dout_port = sl_cli_get_argument_uint8(args, 2);
  directModeConfig.dout_pin = sl_cli_get_argument_uint8(args, 3);
  directModeConfig.dclk_port = sl_cli_get_argument_uint8(args, 4);
  directModeConfig.dclk_pin = sl_cli_get_argument_uint8(args, 5);
  directModeConfig.din_port = sl_cli_get_argument_uint8(args, 6);
  directModeConfig.din_pin = sl_cli_get_argument_uint8(args, 7);

  status = sl_rail_config_direct_mode(railHandle, &directModeConfig);
#else
  (void)args;
#endif
  responsePrint(sl_cli_get_command_string(args, 0), "Result:%s", getStatusMessage(status));
}

void setDirectMode(sl_cli_command_arg_t *args)
{
#if ((_SILICON_LABS_32B_SERIES == 1) || (_SILICON_LABS_32B_SERIES_2_CONFIG >= 3))
  uint8_t enable = sl_cli_get_argument_uint8(args, 0);
  enableAppMode(DIRECT, enable, sl_cli_get_command_string(args, 0));
#else
  (void)args;
  responsePrint(sl_cli_get_command_string(args, 0), "Result:%s", "Invalid Call");
#endif
}

void setDirectTx(sl_cli_command_arg_t *args)
{
  uint8_t enable = sl_cli_get_argument_uint8(args, 0);
  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;

  // Make sure that direct mode is enabled to do a direct Tx
  if (currentAppMode() != DIRECT) {
    responsePrintError(sl_cli_get_command_string(args, 0), 7, "DirectMode not enabled");
    return;
  }

  // Stop whatever we were doing so we can go into Tx
  sl_rail_idle(railHandle, SL_RAIL_IDLE_ABORT, false);

  // Either enable or disable the transmitter
  if (enable) {
    // Turn on Tx
    status = sl_rail_start_tx(railHandle, channel, SL_RAIL_TX_OPTIONS_DEFAULT, NULL);
  } else {
    if (receiveModeEnabled) {
      status = sl_rail_start_rx(railHandle, channel, NULL);
    }
  }
  if (status == SL_RAIL_STATUS_NO_ERROR) {
    responsePrint(sl_cli_get_command_string(args, 0), "DirectTx:%s", (enable ? "Enabled" : "Disabled"));
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0), 8, "DirectMode Rx/Tx not enabled '%d'", status);
  }
}

#include <stdio.h>
#include "sl_core.h"

#ifndef DEBUG_SLEEP_LOOP
#define DEBUG_SLEEP_LOOP 0
#endif//DEBUG_SLEEP_LOOP

// Used for wakeup from sleep
extern volatile bool buttonWakeEvent;

#if defined(_SILICON_LABS_32B_SERIES_2)
static const char * const rfBands[] = { "Off", "GHz", "MHz", "Any", };
static sl_rail_rf_sense_band_t rfBand = SL_RAIL_RF_SENSE_OFF;
static uint32_t rfUs = 0;
static const char * const rfSensitivity[] = { "High", "Low" };
#endif

void sleep(sl_cli_command_arg_t *args)
{
#if defined(_SILICON_LABS_32B_SERIES_2) || defined(_SILICON_LABS_32B_SERIES_3)
  char* em4State = "";
  uint8_t emMode = (uint8_t)sl_cli_get_argument_string(args, 0)[0] - '0';
#if defined(_SILICON_LABS_32B_SERIES_2)
  void (*em4Function)(void) = &sl_power_manager_enter_em4;
#endif
#if (SL_RAIL_SUPPORTS_RF_SENSE_ENERGY_DETECTION || SL_RAIL_SUPPORTS_RF_SENSE_SELECTIVE_OOK)
  RailRfSenseMode_t mode = RFSENSE_MODE_OFF;
  uint32_t rfSenseSyncWord = 0U;
  uint8_t rfSenseSyncWordNumBytes = 0U;
#endif
  emMode &= ~0x80;

#if (_SILICON_LABS_32B_SERIES_3_CONFIG >= 300)
  uint8_t maxEmMode = 3;
#else
  uint8_t maxEmMode = 4;
#endif

  if (emMode <= maxEmMode) {
    if (emMode == 4) {
#if _SILICON_LABS_32B_SERIES_2_CONFIG >= 1
      em4State = "s"; // Only EM4 Shutoff supported
#endif
    }
    // Check for Selective(OOK) Mode
    if (sl_cli_get_argument_count(args) >= 4) {
#if SL_RAIL_SUPPORTS_RF_SENSE_SELECTIVE_OOK
      rfSenseSyncWordNumBytes = (sl_cli_get_argument_uint8(args, 1) > 4)
                                ? 0 : sl_cli_get_argument_uint32(args, 1);
      rfSenseSyncWord = sl_cli_get_argument_uint32(args, 2);
      rfBand = ((sl_rail_rf_sense_band_t) sl_cli_get_argument_uint32(args, 3)
                & SL_RAIL_RF_SENSE_ANY_LOW_SENSITIVITY); // mask off illegal values
      mode = ((rfBand != SL_RAIL_RF_SENSE_OFF) && (rfSenseSyncWordNumBytes > 0))
             ? RFSENSE_MODE_SELECTIVE_OOK : RFSENSE_MODE_OFF;
#else
      responsePrintError(sl_cli_get_command_string(args, 0), 0x15, "RF Sense Selective OOK Mode Unsupported");
      return;
#endif
    } else {
      // sleep is MODAL -- we'll block here in foreground.
      // If there are new RF Sense parameters, grab 'em:
      if (sl_cli_get_argument_count(args) >= 2) {
#if !SL_RAIL_SUPPORTS_RF_SENSE_ENERGY_DETECTION
        responsePrintError(sl_cli_get_command_string(args, 0), 0x15, "RF Sense Energy Detect Mode Unsupported");
        return;
#else
        rfUs = sl_cli_get_argument_uint32(args, 1);
        if (sl_cli_get_argument_count(args) >= 3) {
          rfBand = ((sl_rail_rf_sense_band_t) sl_cli_get_argument_uint32(args, 2)
                    & SL_RAIL_RF_SENSE_ANY_LOW_SENSITIVITY); // mask off illegal values
        }
        mode = ((rfBand != SL_RAIL_RF_SENSE_OFF) && (rfUs > 0))
               ? RFSENSE_MODE_ENERGY_DETECTION : RFSENSE_MODE_OFF;
#endif // _SILICON_LABS_32B_SERIES_2_CONFIG == 1
      }
    }

#if (SL_RAIL_SUPPORTS_RF_SENSE_ENERGY_DETECTION  || SL_RAIL_SUPPORTS_RF_SENSE_SELECTIVE_OOK)
    // Only call if we plan to enable RF Sense later.
    if (mode != RFSENSE_MODE_OFF) {
      if (!enableAppModeSync(RF_SENSE, true, sl_cli_get_command_string(args, 0))) {
        return;
      }
      // Enable event for RF sense since we do not use callback in RAIL 3
      sl_rail_config_events(railHandle, SL_RAIL_EVENT_RF_SENSED, SL_RAIL_EVENT_RF_SENSED);
    } else {
      rfBand = SL_RAIL_RF_SENSE_OFF; // rfUs == 0
    }
#endif

    // Shut down Peripherals for EM2+ sleep. If the radio is still enabled,
    // the chip will go into EM1P if supported
    if (emMode >= 2) {
      PeripheralDisable();
#if defined(_SILICON_LABS_32B_SERIES_2)
      GPIO_EM4SetPinRetention(true);
#endif
    }

    // We cannot configure UART RxD for EM4 wakeup on our EFR32's so the
    // *only* wakeup possible out of EM4 is RF Sense (or reset).
#if defined(_SILICON_LABS_32B_SERIES_3)
    responsePrint(sl_cli_get_command_string(args, 0), "EM:%u%s,SerialWakeup:%s,ButtonWakeup:%s",
#else // Series-2
    responsePrint(sl_cli_get_command_string(args, 0), "EM:%u%s,SerialWakeup:%s,RfSense:%s,RfSensitivity:%s,ButtonWakeup:%s",
#endif
                  emMode, em4State,
#if defined(_SILICON_LABS_32B_SERIES_2) && defined (VCOM_TX_PORT)
                  (VCOM_TX_PORT == SL_GPIO_PORT_C || VCOM_TX_PORT == SL_GPIO_PORT_D)
                  ? ((emMode < 2) ? "On" : "Off") :
#endif
                  (emMode < 4) ? "On" : "Off",
#if defined(_SILICON_LABS_32B_SERIES_2)
                  rfBands[rfBand & SL_RAIL_RF_SENSE_ANY],
                  rfSensitivity[(rfBand & 0x20U) >> 5U],
#endif
#if (defined(SL_CATALOG_BTN0_PRESENT) || defined(SL_CATALOG_BTN1_PRESENT))
                  (emMode >= 2) ? "On" : "Off"
#else
                  "Off"
#endif
                  );
    serialWaitForTxIdle();

    // Disable interrupts heading into sl_rail_start_rf_sense() so we don't miss
    // the event occurring before we try to sleep.
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_CRITICAL();

#if defined(_SILICON_LABS_32B_SERIES_2) && defined(VCOM_TX_PORT)
    // Sleep the USART Tx pin on series 2 devices to save energy
    if (emMode >= 2) {
      sl_gpio_set_pin_mode(&(sl_gpio_t){VCOM_TX_PORT, VCOM_TX_PIN }, SL_GPIO_MODE_DISABLED, true);
    }
#endif

#if (SL_RAIL_SUPPORTS_RF_SENSE_ENERGY_DETECTION || SL_RAIL_SUPPORTS_RF_SENSE_SELECTIVE_OOK)
    switch (mode) {
      case RFSENSE_MODE_SELECTIVE_OOK:
      {
        sl_rail_rf_sense_selective_ook_config_t config = {
          .band = rfBand,
          .sync_word_bytes = rfSenseSyncWordNumBytes,
          .sync_word = rfSenseSyncWord,
        };
        (void) sl_rail_start_selective_ook_rf_sense(railHandle, &config);
        break;
      }
      case RFSENSE_MODE_ENERGY_DETECTION:
        rfUs = sl_rail_start_rf_sense(railHandle, rfBand, rfUs, NULL);
        if (rfUs != 0) {
          break;
        }
      // Fall through
      case RFSENSE_MODE_OFF:
        if (currentAppMode() == RF_SENSE) {
          enableAppMode(RF_SENSE, false, NULL);
        }
        break;
    }
#endif

#if defined(_SILICON_LABS_32B_SERIES_3)
    // Configure the USART Rx pin as a GPIO interrupt for sleep-wake purposes,
    // falling-edge only
    int32_t int_no = VCOM_RX_PIN;
    sl_gpio_configure_external_interrupt(&(sl_gpio_t){.port = VCOM_RX_PORT, .pin = VCOM_RX_PIN },
                                         &int_no,
                                         SL_GPIO_INTERRUPT_FALLING_EDGE,
                                         NULL,
                                         NULL);
#else // Series-2
    // Configure the USART Rx pin as a GPIO interrupt for sleep-wake purposes,
    // falling-edge only
    GPIO_ExtIntConfig(VCOM_RX_PORT,
                      VCOM_RX_PIN,
                      VCOM_RX_PIN,
                      false, true, true);
#endif //_SILICON_LABS_32B_SERIES_3

    serEvent = false;
    rxPacketEvent = false;

    bool rfSensed = false;
   #if     DEBUG_SLEEP_LOOP
    uint64_t sleeps = 0;
    typedef struct wakeReasons{
      uint32_t scbInts;
      uint32_t nvicInts[2];
      uint32_t gpioInts;
    } wakeReasons_t;
    // Maintain the most recent two wakeup events in 'circular' list
    wakeReasons_t wakeReasons[2] = { { 0, 0, }, { 0, 0, }, };
   #endif//DEBUG_SLEEP_LOOP

#ifndef FPGA
    // Used for wakeup from sleep
    buttonWakeEvent = false;

    do { // Loop modally here until either RF Sense or Serial event occurs
         //@TODO Should we WDOG_Feed()??
      switch (emMode) {
        case 0:                      break;
#if defined(_SILICON_LABS_32B_SERIES_3)
        case 1:  sli_power_manager_apply_em(SL_POWER_MANAGER_EM1); break;
        case 2:  sli_power_manager_apply_em(SL_POWER_MANAGER_EM2); break;
        case 3:  sli_power_manager_apply_em(SL_POWER_MANAGER_EM3); break;
#else // Series-2
        case 1:  EMU_EnterEM1(    ); break;
        case 2:  EMU_EnterEM2(true); break;
        case 3:  EMU_EnterEM3(true); break;
        case 4:  em4Function(    );  break; // NOTREACHED
#endif
        default:                     break;
      }
      // After waking, interrupts need to be re-enabled so ISRs can run
     #if     DEBUG_SLEEP_LOOP
      // But grab why we woke first (of course this is meaningless for EM0):
      sleeps++;
      wakeReasons[sleeps & 1].gpioInts = GPIO_IntGetEnabled();
      // No NVIC_ API for this
      wakeReasons[sleeps & 1].nvicInts[0] = NVIC->ISPR[0] /*& NVIC->ISER[0]*/;
      wakeReasons[sleeps & 1].nvicInts[1] = NVIC->ISPR[1] /*& NVIC->ISER[1]*/;
      wakeReasons[sleeps & 1].scbInts = SCB->ICSR;
     #endif//DEBUG_SLEEP_LOOP
      CORE_EXIT_CRITICAL(); // Briefly enable IRQs to let them run
      CORE_ENTER_CRITICAL(); // but shut back off in case we loop
      rfSensed = sl_rail_is_rf_sensed(railHandle);
    } while (!rfSensed && !serEvent && !rxPacketEvent && !buttonWakeEvent);
#endif // not FPGA

#if defined(SL_CATALOG_IOSTREAM_EUSART_PRESENT)
    // Enable EUSART after waking up from sleep on platforms that use EUSART
#if defined(_SILICON_LABS_32B_SERIES_2)
    EUSART_Enable(SL_IOSTREAM_EUSART_VCOM_PERIPHERAL, eusartEnable);
#else
    sl_hal_eusart_enable(SL_IOSTREAM_EUSART_VCOM_PERIPHERAL);
    sl_hal_eusart_enable_rx(SL_IOSTREAM_EUSART_VCOM_PERIPHERAL);
    sl_hal_eusart_enable_tx(SL_IOSTREAM_EUSART_VCOM_PERIPHERAL);
#endif
#endif

#if defined(_SILICON_LABS_32B_SERIES_3)
    sl_gpio_disable_interrupts(1U << VCOM_RX_PIN);
    sl_hal_gpio_clear_interrupts(1U << VCOM_RX_PIN);
#else
    // Disable serial interrupt so it's not bothersome
    GPIO_IntDisable(1U << VCOM_RX_PIN);
    GPIO_IntClear(1U << VCOM_RX_PIN);
#endif

    CORE_EXIT_CRITICAL(); // Back on permanently

    // Here we've awoken for at least one of the desired events.
    // If we woke on serial, don't need RF Sense active any more.
    if (currentAppMode() == RF_SENSE) {
      enableAppMode(RF_SENSE, false, NULL);
    }

#if defined(VCOM_TX_PORT)
#if defined(_SILICON_LABS_32B_SERIES_3)
    // Wake the USART Tx pin back up
    if (emMode >= 2) {
      sl_gpio_set_pin_mode(&(sl_gpio_t){.port = VCOM_TX_PORT, .pin = VCOM_TX_PIN }, SL_GPIO_MODE_PUSH_PULL, true);
    }
#else
    // Wake the USART Tx pin back up
    if (emMode >= 2) {
      sl_gpio_set_pin_mode(&(sl_gpio_t){VCOM_TX_PORT, VCOM_TX_PIN }, SL_GPIO_MODE_PUSH_PULL, true);
    }
#endif
#endif

    // eliminate compiler warning (using multiple volatile variables below)
    bool serEventParam = serEvent;
    bool rxPacketEventParam = rxPacketEvent;

    responsePrint("sleepWoke",
                  "EM:%u%s,"
                  "SerialWakeup:%s,"
                  "RfSensed:%s,"
                  "PacketRx:%s,"
                  "ButtonWakeup:%s,"
                  "RfUs:%u"
                 #if     DEBUG_SLEEP_LOOP
                  ",\nSleeps:%llu,"
                  "\nScb[-]:0x%08x,"
                  "Nvic[-]:0x%x.%08x,Gpios[-]:0x%08x,"
                  "\nScb[0]:0x%08x,"
                  "Nvic[0]:0x%x.%08x,Gpios[0]:0x%08x"
                 #endif//DEBUG_SLEEP_LOOP
                  , emMode,
                  em4State,
                  serEventParam ? "Yes" : "No",
                  rfSensed ? "Yes" : "No",
                  rxPacketEventParam ? "Yes" : "No",
                  buttonWakeEvent ? "Yes" : "No",
#if defined(_SILICON_LABS_32B_SERIES_3)
                  0
#else // Series-2
                  rfUs
#endif
                 #if     DEBUG_SLEEP_LOOP
                  , sleeps,
                  wakeReasons[(sleeps - 1) & 1].scbInts,
                  wakeReasons[(sleeps - 1) & 1].nvicInts[1],
                  wakeReasons[(sleeps - 1) & 1].nvicInts[0],
                  wakeReasons[(sleeps - 1) & 1].gpioInts,
                  wakeReasons[(sleeps) & 1].scbInts,
                  wakeReasons[(sleeps) & 1].nvicInts[1],
                  wakeReasons[(sleeps) & 1].nvicInts[0],
                  wakeReasons[(sleeps) & 1].gpioInts
                 #endif//DEBUG_SLEEP_LOOP
                  );

    // Used for wakeup from sleep
    buttonWakeEvent = false;

    if (serEvent) { // Consume the character entered
      if (emMode >= 2) {
        // If UART was shut down, delay and gobble likely junk
        usDelay(250000); // Pause for 250 ms
        consumeChar();
        consumeChar();
        consumeChar();
        consumeChar();
      }
    }

    // Restart Rx if we're in Rx mode
    if (receiveModeEnabled) {
      if (sl_rail_start_rx(railHandle, channel, NULL) != SL_RAIL_STATUS_NO_ERROR) {
        responsePrintError(sl_cli_get_command_string(args, 0), 1, "Could not start receive after sleep");
      }
    }
    // Restart peripherals if they were active before sleeping
    if ((emMode >= 2) && (logLevel & PERIPHERAL_ENABLE)) {
      PeripheralEnable();
    }
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0), 1, "Invalid EM mode %u (valid 0-%u)", emMode, maxEmMode);
  }
#else
  responsePrintError(sl_cli_get_command_string(args, 0), 0x15, "Sleep Unsupported");
#endif
}

void rfSense(sl_cli_command_arg_t *args)
{
#if SL_RAIL_SUPPORTS_RF_SENSE_ENERGY_DETECTION
  RailRfSenseMode_t mode = RFSENSE_MODE_OFF;
  uint8_t rfSenseSyncWordNumBytes = 0U;
  uint32_t rfSenseSyncWord = 0U;
  // Check for Selective(OOK) Mode
  if (sl_cli_get_argument_count(args) >= 3) {
#if SL_RAIL_SUPPORTS_RF_SENSE_SELECTIVE_OOK
    rfSenseSyncWordNumBytes = (sl_cli_get_argument_uint8(args, 0) > 4)
                               ? 0 : sl_cli_get_argument_uint32(args, 0);
                               rfSenseSyncWord = sl_cli_get_argument_uint32(args, 1);
                               rfBand = ((sl_rail_rf_sense_band_t) sl_cli_get_argument_uint32(args, 2)
                                         & SL_RAIL_RF_SENSE_ANY_LOW_SENSITIVITY); // mask off illegal values
                               mode = ((rfBand != SL_RAIL_RF_SENSE_OFF) && (rfSenseSyncWordNumBytes > 0))
                                      ? RFSENSE_MODE_SELECTIVE_OOK : RFSENSE_MODE_OFF;
#else
    responsePrintError(sl_cli_get_command_string(args, 0), 0x15, "RF Sense Selective OOK Mode Unsupported");
    return;
#endif
  } else {
    // If there are RF Sense parameters, grab 'em; otherwise use prior settings
    if (sl_cli_get_argument_count(args) >= 1) {
      rfUs = sl_cli_get_argument_uint32(args, 0);
      if (sl_cli_get_argument_count(args) >= 2) {
        rfBand = ((sl_rail_rf_sense_band_t) sl_cli_get_argument_uint32(args, 1)
                  & SL_RAIL_RF_SENSE_ANY_LOW_SENSITIVITY); // mask off illegal values
      }
      mode = ((rfBand != SL_RAIL_RF_SENSE_OFF) && (rfUs > 0))
             ? RFSENSE_MODE_ENERGY_DETECTION : RFSENSE_MODE_OFF;
    }
  }

  if (!enableAppModeSync(RF_SENSE, mode != RFSENSE_MODE_OFF, sl_cli_get_command_string(args, 0))) {
    return;
  }

  switch (mode) {
    case RFSENSE_MODE_SELECTIVE_OOK:
      {
        sl_rail_rf_sense_selective_ook_config_t config = {
          .band = rfBand,
          .sync_word_bytes = rfSenseSyncWordNumBytes,
          .sync_word = rfSenseSyncWord,
        };
        (void) sl_rail_start_selective_ook_rf_sense(railHandle, &config);
        break;
      }
    case RFSENSE_MODE_ENERGY_DETECTION:
      rfUs = sl_rail_start_rf_sense(railHandle, rfBand, rfUs, NULL);
      if (rfUs != 0) {
        break;
      }
    // Fall through
    case RFSENSE_MODE_OFF:
      rfBand = SL_RAIL_RF_SENSE_OFF;
      if (currentAppMode() == RF_SENSE) {
        enableAppMode(RF_SENSE, false, NULL);
      }
      break;
  }

  responsePrint(sl_cli_get_command_string(args, 0), "RfBand:%s,RfUs:%u,RfSensitivity:%s",
                rfBands[rfBand & SL_RAIL_RF_SENSE_ANY],
                rfUs,
                rfSensitivity[(rfBand & 0x20U) >> 5U]);
#else//!SL_RAIL_SUPPORTS_RF_SENSE_ENERGY_DETECTION
  responsePrintError(sl_cli_get_command_string(args, 0), 0x15, "RF Sense Unsupported");
#endif//SL_RAIL_SUPPORTS_RF_SENSE_ENERGY_DETECTION
}

void rfSensedCheck(void)
{
#if defined(_SILICON_LABS_32B_SERIES_2)
  // If in Rf Sensing background mode, see if Rf got sensed
  if ((currentAppMode() == RF_SENSE) && sl_rail_is_rf_sensed(railHandle)) {
    // Yes, it did!  Terminate RF Sensing background mode
    enableAppMode(RF_SENSE, false, NULL);
    responsePrint("rfSensedCheck", "RfSensed:%s,RfUs:%u",
                  rfBands[rfBand & SL_RAIL_RF_SENSE_ANY], rfUs);
  }
#endif
}

/**
 * Routine to parse a RAIL timer mode from a string.
 *
 * @param str The string to parse. Must be one of the following:
 * - 'rel': SL_RAIL_TIME_DELAY
 * - 'abs': SL_RAIL_TIME_ABSOLUTE
 * - 'dis': SL_RAIL_TIME_DISABLED
 * @param mode The sl_rail_time_mode_t variable to fill in with the result of this
 * conversion.
 * @return Returns true if we were able to successfully convert the string into
 * a time mode and false if we could not.
 */
bool parseTimeModeFromString(char *str, sl_rail_time_mode_t *mode)
{
  if (strcasecmp("dis", str) == 0) {
    *mode = SL_RAIL_TIME_DISABLED;
  } else if (strcasecmp("rel", str) == 0) {
    *mode = SL_RAIL_TIME_DELAY;
  } else if (strcasecmp("abs", str) == 0) {
    *mode = SL_RAIL_TIME_ABSOLUTE;
  } else {
    *mode = SL_RAIL_TIME_DISABLED;
    return false;
  }
  return true;
}

// Must match sl_rail_time_position_t enum values as indices:
static const char * const timePosStrings[SL_RAIL_PACKET_TIME_COUNT] = {
  "INVALID",
  "DEFAULT",
  "PREAMBLE_START",
  "PREAMBLE_START_UT",
  "SYNC_END",
  "SYNC_END_UT",
  "PACKET_END",
  "PACKET_END_UT",
};

static const TxTimestampFunc txTimeFuncs[SL_RAIL_PACKET_TIME_COUNT / 2] = {
  &identityTimestampTx,
  &sl_rail_get_tx_time_preamble_start,
  &sl_rail_get_tx_time_sync_word_end,
  &sl_rail_get_tx_time_frame_end,
};

static const RxTimestampFunc rxTimeFuncs[SL_RAIL_PACKET_TIME_COUNT / 2] = {
  &identityTimestampRx,
  &sl_rail_get_rx_time_preamble_start,
  &sl_rail_get_rx_time_sync_word_end,
  &sl_rail_get_rx_time_frame_end,
};

void setTxTimePos(sl_cli_command_arg_t *args)
{
  uint32_t pos = sl_cli_get_argument_uint32(args, 0);
  if (pos < SL_RAIL_PACKET_TIME_COUNT) {
    txTimePosition = txTimeFuncs[pos / 2];
    responsePrint(sl_cli_get_command_string(args, 0), "txTimePos:%s", timePosStrings[pos]);
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0), 1, "Invalid TimePosition %u (valid 0-%u)",
                       pos, SL_RAIL_PACKET_TIME_COUNT - 1);
  }
}

void setRxTimePos(sl_cli_command_arg_t *args)
{
  uint32_t pos = sl_cli_get_argument_uint32(args, 0);
  if (pos < SL_RAIL_PACKET_TIME_COUNT) {
    rxTimePosition = rxTimeFuncs[pos / 2];
    responsePrint(sl_cli_get_command_string(args, 0), "rxTimePos:%s", timePosStrings[pos]);
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0), 1, "Invalid TimePosition %u (valid 0-%u)",
                       pos, SL_RAIL_PACKET_TIME_COUNT - 1);
  }
}

void holdRx(sl_cli_command_arg_t *args)
{
  rxHeld = !!sl_cli_get_argument_uint8(args, 0);
  rxProcessHeld = !rxHeld;
  responsePrint(sl_cli_get_command_string(args, 0), "HoldRx:%s",
                rxHeld ? "Enabled" : "Disabled");
}

void enableCacheSynthCal(sl_cli_command_arg_t *args)
{
  uint8_t enable = sl_cli_get_argument_uint8(args, 0);
  sl_rail_status_t status = sl_rail_enable_cache_synth_cal(railHandle, (bool)enable);

  responsePrint(sl_cli_get_command_string(args, 0), "Result:%s",
                ((status == SL_RAIL_STATUS_NO_ERROR) ? "Success" : "Failure"));
}

void enableAutoLnaBypass(sl_cli_command_arg_t *args)
{
#if !SL_RAIL_SUPPORTS_PRS_LNA_BYPASS
  responsePrintError(sl_cli_get_command_string(args, 0), 0x15, "Automatic LNA Bypass Unsupported");
  return;
#else
  bool enable = !!sl_cli_get_argument_uint8(args, 0);
  sl_gpio_set_pin_mode(&(sl_gpio_t){sl_cli_get_argument_uint8(args, 4), sl_cli_get_argument_uint8(args, 5) }, SL_GPIO_MODE_PUSH_PULL, enable);
  uint8_t prsLnaBypassChannel = PRS_GetFreeChannel(prsTypeAsync);
  PRS_PinOutput(prsLnaBypassChannel, prsTypeAsync, sl_cli_get_argument_uint8(args, 4), sl_cli_get_argument_uint8(args, 5));
  sl_rail_prs_lna_bypass_config_t prsLnaBypassConfig = {
    .timeout_us = (sl_rail_time_t)sl_cli_get_argument_uint32(args, 1),
    .threshold = sl_cli_get_argument_uint8(args, 2),
    .delta_rssi_db = sl_cli_get_argument_uint8(args, 3),
    .prs_channel = prsLnaBypassChannel,
    .polarity = (bool)sl_cli_get_argument_uint8(args, 6)
  };
  sl_rail_status_t status = sl_rail_enable_prs_lna_bypass(railHandle, enable, &prsLnaBypassConfig);
  responsePrint(sl_cli_get_command_string(args, 0), "Result:%s",
                ((status == SL_RAIL_STATUS_NO_ERROR) ? "Success" : "Failure"));
#endif
}

void cancelScheduledTrx(sl_cli_command_arg_t *args)
{
#ifdef SL_RAIL_SUPPORTS_HARDWARE_SCHEDULER
  // Turn off ScheduledRx if we were in it
  if (currentAppMode() == RX_SCHEDULED) {
    (void) enableAppModeSync(RX_SCHEDULED, false, NULL);
  }
  // Turn off ScheduledTx if we were in it
  if (currentAppMode() == TX_SCHEDULED) {
    (void) enableAppModeSync(TX_SCHEDULED, false, NULL);
  }
  sl_rail_status_t result = sl_rail_cancel_scheduled_trx(railHandle);
  responsePrint(sl_cli_get_command_string(args, 0), "Result:%d", result);
#else
  responsePrintError(sl_cli_get_command_string(args, 0), 0x11, "This command is not supported on this platform");
#endif
}
