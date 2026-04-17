/***************************************************************************//**
 * @file
 * @brief This file implements commands for configuring ZWAVE RAIL options
 *   relevant to receiving packets
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
#include "sl_core.h"
#include "app_common.h"
#include "sl_rail_zwave.h"

// Macro to determine array size.
#define COMMON_UTILS_COUNTOF(a) (sizeof(a) / sizeof((a)[0]))

#if SL_RAIL_SUPPORTS_PROTOCOL_ZWAVE
sl_rail_zwave_config_t config = {
  .options = SL_RAIL_ZWAVE_OPTIONS_NONE,
  .ack_config = {
    .enable = false,
    .ack_timeout_us = SL_RAIL_ZWAVE_MAX_ACK_TIMEOUT_US,
    .rx_transitions = {
      .success = SL_RAIL_RF_STATE_RX,
      .error = SL_RAIL_RF_STATE_RX   // ignored
    },
    .tx_transitions = {
      .success = SL_RAIL_RF_STATE_RX,
      .error = SL_RAIL_RF_STATE_RX   // ignored
    }
  },
  .timings = {
    .idle_to_tx = SL_RAIL_ZWAVE_TIME_IDLE_TO_TX_US,
    .idle_to_rx = SL_RAIL_ZWAVE_TIME_IDLE_TO_RX_US,
    .rx_to_tx = SL_RAIL_ZWAVE_TIME_RX_TO_TX_US,
    .tx_to_rx = SL_RAIL_ZWAVE_TIME_TX_TO_RX_US,
    .rxsearch_timeout = 0,
    .tx_to_rxsearch_timeout = 0
  }
};

typedef struct ZWAVE_Region {
  const char *name;
  const sl_rail_zwave_region_config_t *config;
} ZWAVE_Region_t;

/******************* Local functions *******************/
static const char * const zwaveRegionNames[] = {
  "Invalid",
  "EU-European Union",
  "US-United States",
  "ANZ-Australia/New Zealand",
  "HK-Hong Kong",
  "MY-Malaysia",
  "IN-India",
  "JP-Japan",
  "RU-Russia",
  "IL-Israel",
  "KR-Korea",
  "CN-China",
  "USLR1-United States, Long Range 1",
  "USLR2-United States, Long Range 2",
  "USLR3-United States, Long Range 3",
  "EULR1-European Union, Long Range 1",
  "EULR2-European Union, Long Range 2",
  "EULR3-European Union, Long Range 3",
};

static uint8_t configuredRegion = SL_RAIL_ZWAVE_REGION_ID_UNKNOWN;

static const char* const baudrateNames[] = {
  "9600bps",
  "40Kbps",
  "100Kbps",
  "Long Range",
  "Energy Detect"
};

static sl_rail_zwave_ir_cal_val_t zwaveIrCalVal[2] = {
  {
    // For JP/KR regions
    .image_rejection[0] = SL_RAIL_IR_CAL_VALUES_UNINIT,
    .image_rejection[1] = SL_RAIL_IR_CAL_VALUES_UNINIT
  },
  {
    // For all other regions
    .image_rejection[0] = SL_RAIL_IR_CAL_VALUES_UNINIT,
    .image_rejection[1] = SL_RAIL_IR_CAL_VALUES_UNINIT
  }
};

void zwaveListRegions(sl_cli_command_arg_t *args)
{
  uint8_t i;
  responsePrintStart(sl_cli_get_command_string(args, 0));
  // For backwards compat, we skip Invalid and 0 is EU
  for (i = 1;
       i < SL_RAIL_ZWAVE_REGION_ID_COUNT - 1;
       ++i) {
    responsePrintContinue("%i:%s", i - 1, zwaveRegionNames[i]);
  }
  responsePrintEnd("%i:%s", i - 1, zwaveRegionNames[i]);
}

void zwaveStatus(sl_cli_command_arg_t *args)
{
  bool enabled = sl_rail_zwave_is_enabled(railHandle);

  // Report the current enabled status for ZWAVE
  responsePrint(sl_cli_get_command_string(args, 0),
                "ZWAVE:%s,"
                "Promiscuous:%s,"
                "BeamDetect:%s,"
                "PromiscuousBeam:%s",
                enabled ? "Enabled" : "Disabled",
                ((config.options & SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_MODE) != 0U)
                ? "Enabled" : "Disabled",
                ((config.options & SL_RAIL_ZWAVE_OPTION_DETECT_BEAM_FRAMES) != 0U)
                ? "Enabled" : "Disabled",
                ((config.options & SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_BEAM_MODE) != 0U)
                ? "Enabled" : "Disabled");
}

void zwaveEnable(sl_cli_command_arg_t *args)
{
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }

  if (sl_cli_get_argument_count(args) >= 1) {
    bool enable = !!sl_cli_get_argument_uint8(args, 0);
    if (sl_cli_get_argument_count(args) >= 2) {
      config.options = sl_cli_get_argument_uint32(args, 1);
    }
    if (sl_cli_get_argument_count(args) >= 3) {
      config.ack_config.enable = (bool)sl_cli_get_argument_uint32(args, 2);
    } else {
      config.ack_config.enable = false;
    }

    // Turn ZWAVE mode on or off as requested
    if (enable) {
      if (disableIncompatibleProtocols(SL_RAIL_PTI_PROTOCOL_ZWAVE) != SL_RAIL_STATUS_NO_ERROR) {
        responsePrintError(sl_cli_get_command_string(args, 0), 0x22, "Current protocol deinit failed");
        return;
      }
      sl_rail_zwave_init(railHandle, &config);
    } else {
      sl_rail_zwave_deinit(railHandle);
    }
  }
  // Report the current status of ZWAVE mode
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  zwaveStatus(args);
}

void zwaveConfigureOptions(sl_cli_command_arg_t *args)
{
  if (!sl_rail_zwave_is_enabled(railHandle)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x26,
                       "Need to enable Z-Wave for this command.");
    return;
  }

  if (sl_cli_get_argument_count(args) >= 1) {
    sl_rail_zwave_options_t options = (sl_rail_zwave_options_t)sl_cli_get_argument_uint32(args, 0);
    sl_rail_zwave_config_options(railHandle, SL_RAIL_ZWAVE_OPTIONS_ALL, options);
    config.options = options;
  }

  // Report the status for ZWAVE options.
  responsePrint(sl_cli_get_command_string(args, 0),
                "Promiscuous:%s,"
                "BeamDetect:%s,"
                "NodeIDFiltering:%s,"
                "AutoAck:%s,"
                "PromiscuousBeam:%s",
                ((config.options & SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_MODE) != 0U)
                ? "Enabled" : "Disabled",
                ((config.options & SL_RAIL_ZWAVE_OPTION_DETECT_BEAM_FRAMES) != 0U)
                ? "Enabled" : "Disabled",
                ((config.options & SL_RAIL_ZWAVE_OPTION_NODE_ID_FILTERING) != 0U)
                ? "Enabled" : "Disabled",
                (sl_rail_is_auto_ack_enabled(railHandle)
                 && (config.options & SL_RAIL_ZWAVE_OPTION_NODE_ID_FILTERING) != 0U)
                ? "Enabled" : "Disabled",
                ((config.options & SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_BEAM_MODE) != 0U)
                ? "Enabled" : "Disabled");
}

void zwaveGetRegion(sl_cli_command_arg_t *args)
{
  if (configuredRegion != SL_RAIL_ZWAVE_REGION_ID_UNKNOWN) {
    responsePrint(sl_cli_get_command_string(args, 0),
                  "ZWaveRegion:%s,ZWaveRegionIndex:%i",
                  zwaveRegionNames[configuredRegion],
                  configuredRegion - 1);
  } else {
    responsePrint(sl_cli_get_command_string(args, 0),
                  "ZWaveRegion:Undefined,ZWaveRegionIndex:%i",
                  SL_RAIL_ZWAVE_REGION_ID_COUNT - 1);
  }
}

void zwaveSetRegion(sl_cli_command_arg_t *args)
{
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }
  uint8_t region = sl_cli_get_argument_uint8(args, 0);
  if (region >= (SL_RAIL_ZWAVE_REGION_ID_COUNT - 1)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x25,
                       "Unsupported Z-Wave Region.");
    args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
    zwaveListRegions(args);
    return;
  }
  configuredRegion = region + 1;
  if (sl_rail_zwave_config_region(railHandle, sl_rail_zwave_region_cfg[configuredRegion]) != SL_RAIL_STATUS_NO_ERROR) {
    configuredRegion = SL_RAIL_ZWAVE_REGION_ID_UNKNOWN;
  }
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  zwaveGetRegion(args);
}

void zwavePerformIrcal(sl_cli_command_arg_t *args)
{
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }

  if (!sl_rail_zwave_is_enabled(railHandle)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x24,
                       "Z-Wave not enabled.");
    return;
  }

  if (configuredRegion == SL_RAIL_ZWAVE_REGION_ID_UNKNOWN) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x25,
                       "Z-Wave Region not set.");
    args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
    zwaveListRegions(args);
    return;
  }

  bool forceIrcal = false;
  if ((sl_cli_get_argument_count(args) >= 1)) {
    forceIrcal = !!sl_cli_get_argument_uint8(args, 0);
  }

  sl_rail_zwave_ir_cal_val_t *zwaveIrCalValLocal;
  if ((configuredRegion == SL_RAIL_ZWAVE_REGION_ID_KR)
#if SL_RAIL_ZWAVE_SUPPORTS_CONC_PHY
      || (configuredRegion == SL_RAIL_ZWAVE_REGION_ID_JP)
#endif
      ) {
    zwaveIrCalValLocal = &zwaveIrCalVal[0]; // Will hold the lowside IRCAL value for channel 0 and
                                            // highside IRCAL for channel 1 and others.
  } else {
    zwaveIrCalValLocal = &zwaveIrCalVal[1]; // Will only hold highside IRCAL value for all channels.
  }

  uint32_t start_time = sl_rail_get_time(railHandle);
  sl_rail_status_t status = sl_rail_zwave_perform_ir_cal(railHandle, zwaveIrCalValLocal, forceIrcal);
  uint32_t timing = sl_rail_get_time(railHandle) - start_time;
  responsePrint(sl_cli_get_command_string(args, 0),
                "Status:%s,"
                "elapsedTimeUs:%u",
                (status != SL_RAIL_STATUS_NO_ERROR) ? "Error" : "Complete",
                timing);
}

void zwaveSetNodeId(sl_cli_command_arg_t *args)
{
  sl_rail_zwave_node_id_t nodeId = sl_cli_get_argument_uint16(args, 0);
  sl_rail_status_t status = sl_rail_zwave_set_node_id(railHandle, nodeId);
  responsePrint(sl_cli_get_command_string(args, 0), "Status:%s",
                (status != SL_RAIL_STATUS_NO_ERROR) ? "Error" : "Set");
}

void zwaveSetHomeId(sl_cli_command_arg_t *args)
{
  sl_rail_zwave_home_id_t homeId = sl_cli_get_argument_uint32(args, 0);
  sl_rail_zwave_home_id_t homeIdHash = sl_cli_get_argument_uint32(args, 1);
  sl_rail_status_t status = sl_rail_zwave_set_home_id(railHandle, homeId, homeIdHash);
  responsePrint(sl_cli_get_command_string(args, 0), "Status:%s",
                (status != SL_RAIL_STATUS_NO_ERROR) ? "Error" : "Set");
}

void zwaveGetBaudRate(sl_cli_command_arg_t *args)
{
  uint16_t channel = -1;
  sl_rail_get_channel(railHandle, &channel);
  if (channel < SL_RAIL_NUM_ZWAVE_CHANNELS
      && configuredRegion != SL_RAIL_ZWAVE_REGION_ID_UNKNOWN) {
    responsePrint(sl_cli_get_command_string(args, 0),
                  "baudrate:%s",
                  baudrateNames[sl_rail_zwave_region_cfg[configuredRegion]->baud_rate[channel]]);
  } else {
    responsePrint(sl_cli_get_command_string(args, 0),
                  "baudrate:Undefined");
  }
}

void zwaveSetLowPowerLevel(sl_cli_command_arg_t *args)
{
  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;
  if (!sl_rail_zwave_is_enabled(railHandle)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x26,
                       "Need to enable Z-Wave for this command.");
    return;
  }
  sl_rail_tx_power_t powerLevelDbm = (sl_rail_tx_power_t)sl_cli_get_argument_int32(args, 0);
  status = sl_rail_zwave_set_tx_low_power_dbm(railHandle, powerLevelDbm);
  responsePrint(sl_cli_get_command_string(args, 0), "LowPowerLevel:%s", status ? "Error" : "Set");
}

void zwaveGetLowPowerLevel(sl_cli_command_arg_t *args)
{
  responsePrint(sl_cli_get_command_string(args, 0),
                "powerLeveldBm:%d",
                sl_rail_zwave_get_tx_low_power_dbm(railHandle));
}

void zwaveReceiveBeam(sl_cli_command_arg_t *args)
{
  uint8_t beamDetectIndex;
  sl_rail_status_t status = sl_rail_zwave_receive_beam(railHandle, &beamDetectIndex, NULL);
  responsePrint(sl_cli_get_command_string(args, 0),
                "status:%s",
                (status == SL_RAIL_STATUS_NO_ERROR) ? "Success" : "Error");
}

void railtest_ZWAVE_BeamFrame(sl_rail_handle_t railHandle)
{
  RailAppEvent_t *beamPacket
    = (RailAppEvent_t *)sl_malloc(sizeof(RailAppEvent_t));
  if (beamPacket == NULL) {
    eventsMissed++;
    return;
  }

  beamPacket->type = BEAM_PACKET;
  if ((sl_rail_zwave_get_rx_beam_details(railHandle, &beamPacket->beamPacket)
       != SL_RAIL_STATUS_NO_ERROR)) {
    return;
  }

  queueAdd(&railAppEventQueue, (void *)beamPacket);
}

void railtest_ZWAVE_LrAckData(sl_rail_handle_t railHandle)
{
  sl_rail_zwave_lr_ack_data_t lrAckData = {
    .noise_floor_dbm = (int8_t)(sl_rail_get_rssi(railHandle, false) / 4),
    .transmit_power_dbm = (int8_t)(sl_rail_get_tx_power_dbm(railHandle) / 10),
    .receive_rssi_dbm = (int8_t)(sl_rail_get_rssi(railHandle, false) / 4) // Ideally should be AGC_FRAMERSSI
  };
  sl_rail_zwave_set_lr_ack_data(railHandle, &lrAckData);
}
#else //!SL_RAIL_SUPPORTS_PROTOCOL_ZWAVE

void zwaveNotSupported(sl_cli_command_arg_t *args)
{
  (void)args;
  responsePrintError(sl_cli_get_command_string(args, 0), 0x56, "Z-Wave not suppported on this chip");
}

void zwaveListRegions(sl_cli_command_arg_t *args)
{
  zwaveNotSupported(args);
}

void zwaveStatus(sl_cli_command_arg_t *args)
{
  zwaveNotSupported(args);
}

void zwaveEnable(sl_cli_command_arg_t *args)
{
  zwaveNotSupported(args);
}

void zwaveConfigureOptions(sl_cli_command_arg_t *args)
{
  zwaveNotSupported(args);
}

void zwaveGetRegion(sl_cli_command_arg_t *args)
{
  zwaveNotSupported(args);
}

void zwaveSetRegion(sl_cli_command_arg_t *args)
{
  zwaveNotSupported(args);
}

void zwaveSetNodeId(sl_cli_command_arg_t *args)
{
  zwaveNotSupported(args);
}

void zwaveSetHomeId(sl_cli_command_arg_t *args)
{
  zwaveNotSupported(args);
}

void zwaveGetBaudRate(sl_cli_command_arg_t *args)
{
  zwaveNotSupported(args);
}

void zwaveSetLowPowerLevel(sl_cli_command_arg_t *args)
{
  zwaveNotSupported(args);
}

void zwaveGetLowPowerLevel(sl_cli_command_arg_t *args)
{
  zwaveNotSupported(args);
}

void zwaveReceiveBeam(sl_cli_command_arg_t *args)
{
  zwaveNotSupported(args);
}

void zwavePerformIrcal(sl_cli_command_arg_t *args)
{
  zwaveNotSupported(args);
}

#endif //SL_RAIL_SUPPORTS_PROTOCOL_ZWAVE
