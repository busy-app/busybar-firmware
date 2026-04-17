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
#include "app_common.h"

static sl_rail_auto_ack_config_t config;

void autoAckConfig(sl_cli_command_arg_t *args)
{
  config.enable = true;

  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }

  if (memcmp(sl_cli_get_argument_string(args, 0), "idle", 4) == 0) {
    config.rx_transitions.success = SL_RAIL_RF_STATE_IDLE;
    config.tx_transitions.success = SL_RAIL_RF_STATE_IDLE;
  } else if (memcmp(sl_cli_get_argument_string(args, 0), "rx", 2) == 0) {
    config.rx_transitions.success = SL_RAIL_RF_STATE_RX;
    config.tx_transitions.success = SL_RAIL_RF_STATE_RX;
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x20, "Unknown auto ack default state.");
    return;
  }

  sl_rail_status_t status;
  sl_rail_transition_time_t timing;
  sl_rail_state_timing_t timings = { 0U, };

  timing = (sl_rail_transition_time_t)sl_cli_get_argument_uint32(args, 1);
  if (timing > SL_RAIL_MAXIMUM_TRANSITION_US) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x21, "Invalid idle timing.");
    return;
  } else {
    timings.idle_to_tx = timing;
    timings.idle_to_rx = timing;
  }

  timing = (sl_rail_transition_time_t)sl_cli_get_argument_uint32(args, 2);
  if (timing > SL_RAIL_MAXIMUM_TRANSITION_US) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x22, "Invalid turnaround timing");
    return;
  } else {
    timings.rx_to_tx = timing;
    // Make tx_to_rx a little lower than desired. See documentation
    // on sl_rail_config_auto_ack.
    if (timing > 10) {
      timings.tx_to_rx = timing - 10U;
    } else {
      timings.tx_to_rx = 0U;
    }
  }

  timing = sl_cli_get_argument_uint16(args, 3);
  config.ack_timeout_us = timing;

  sl_rail_set_state_timing(railHandle, &timings);
  status = sl_rail_config_auto_ack(railHandle, &config);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    responsePrintError(sl_cli_get_command_string(args, 0), status, "Call to sl_rail_config_auto_ack returned an error");
  } else {
    responsePrint(sl_cli_get_command_string(args, 0),
                  "rxDefaultState:%s,"
                  "txDefaultState:%s,"
                  "idleTiming:%u,"
                  "turnaroundTime:%u,"
                  "ackTimeout:%u",
                  getRfStateName(config.rx_transitions.success),
                  getRfStateName(config.tx_transitions.success),
                  timings.idle_to_tx,
                  timings.rx_to_tx,
                  config.ack_timeout_us);
  }
}

void getAutoAck(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  bool filteringEnabled = sl_rail_is_auto_ack_enabled(railHandle);
  bool rxPaused = sl_rail_is_rx_auto_ack_paused(railHandle);
  bool txPaused = sl_rail_is_tx_auto_ack_paused(railHandle);

  responsePrint(sl_cli_get_command_string(args, 0),
                "AutoAck:%s,"
                "RxAutoAckStatus:%s,"
                "TxAutoAckStatus:%s",
                filteringEnabled ? "Enabled" : "Disabled",
                rxPaused ? "Paused" : "Unpaused",
                txPaused ? "Paused" : "Unpaused");
}

void autoAckDisable(sl_cli_command_arg_t *args)
{
  sl_rail_state_transitions_t transitions = {
    .success = SL_RAIL_RF_STATE_IDLE,
    .error = SL_RAIL_RF_STATE_IDLE
  };
  sl_rail_auto_ack_config_t autoAckConfig = {
    .enable = false,
    .ack_timeout_us = 0,
    .rx_transitions = transitions,
    .tx_transitions = transitions,
  };

  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  sl_rail_status_t status = sl_rail_config_auto_ack(railHandle, &autoAckConfig);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    responsePrintError(sl_cli_get_command_string(args, 0), status, "Error disabling AutoAck");
  }
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  getAutoAck(args);
}

void autoAckPause(sl_cli_command_arg_t *args)
{
  uint8_t rxPause = sl_cli_get_argument_uint8(args, 0);
  uint8_t txPause = sl_cli_get_argument_uint8(args, 1);

  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));

  sl_rail_pause_rx_auto_ack(railHandle, !!rxPause);

  sl_rail_pause_tx_auto_ack(railHandle, !!txPause);

  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  getAutoAck(args);
}

void setTxAckOptions(sl_cli_command_arg_t *args)
{
  uint8_t cancelAck = sl_cli_get_argument_uint8(args, 0);
  uint8_t useTxBuffer = sl_cli_get_argument_uint8(args, 1);

  afterRxCancelAck = ((cancelAck != 0) ? true : false);
  afterRxUseTxBufferForAck = ((useTxBuffer != 0) ? true : false);

  responsePrint(sl_cli_get_command_string(args, 0),
                "CancelAck:%s,"
                "UseTxBuffer:%s",
                afterRxCancelAck ? "True" : "False",
                afterRxUseTxBufferForAck ? "True" : "False");
}

bool txAckDirect;

void setTxAckDirect(sl_cli_command_arg_t *args)
{
  txAckDirect = (bool) sl_cli_get_argument_uint8(args, 0);
  responsePrint(sl_cli_get_command_string(args, 0),
                "TxAckDirect:%s",
                txAckDirect ? "Enabled" : "Disabled");
}
