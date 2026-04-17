/***************************************************************************//**
 * @file
 * @brief This file implements commands for configuring BLE RAIL options
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

#include "response_print.h"

#include "sl_rail.h"
#include "sl_rail_ble.h"

#include "app_common.h"

static const uint8_t blePacket[] = {
  0x02, 0x1A, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x02,
  0x01, 0x06, 0x10, 0x08, 'S', 'i', 'l', 'a', 'b', 's',
  ' ', 'R', 'A', 'I', 'L', 'T', 'E', 'S', 'T',
};
PhySwitchToRx_t phySwitchToRx;

void bleStatus(sl_cli_command_arg_t *args);

void bleEnable(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  bool enable = !!sl_cli_get_argument_uint8(args, 0);

  // Turn BLE mode on or off as requested
  if (enable) {
    if (disableIncompatibleProtocols(SL_RAIL_PTI_PROTOCOL_BLE) != SL_RAIL_STATUS_NO_ERROR) {
      responsePrintError(sl_cli_get_command_string(args, 0), 0x22, "Current protocol deinit failed");
      return;
    }
    sl_rail_ble_init(railHandle);
  } else {
    sl_rail_ble_deinit(railHandle);
  }

  // Report the current status of BLE mode
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  bleStatus(args);
}

void bleStatus(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  bool enabled = sl_rail_ble_is_enabled(railHandle);

  // Report the current enabled status for BLE
  responsePrint(sl_cli_get_command_string(args, 0), "BLE:%s", enabled ? "Enabled" : "Disabled");
}

void bleSet1MbpsPhy(sl_cli_command_arg_t *args)
{
  sl_rail_status_t status;
  // No longer support viterbi vs. legacy so ignore parameter
  status = sl_rail_ble_config_phy_1_mbps(railHandle);
  responsePrint(sl_cli_get_command_string(args, 0), "Status:%s", getStatusMessage(status));
}

void bleSet2MbpsPhy(sl_cli_command_arg_t *args)
{
  sl_rail_status_t status;
  // No longer support viterbi vs. legacy so ignore parameter
  status = sl_rail_ble_config_phy_2_mbps(railHandle);
  responsePrint(sl_cli_get_command_string(args, 0), "Status:%s", getStatusMessage(status));
}

void bleSetCoding(sl_cli_command_arg_t *args)
{
  // Make sure BLE mode is enabled so that the call below can succeed
  if (!sl_rail_ble_is_enabled(railHandle)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x31, "BLE mode not enabled");
    return;
  }

  sl_rail_ble_coding_t coding = (sl_rail_ble_coding_t) sl_cli_get_argument_uint8(args, 0);
  sl_rail_status_t status = sl_rail_ble_config_phy_coded(railHandle, coding);
  responsePrint(sl_cli_get_command_string(args, 0), "Status:%s", getStatusMessage(status));
}

void bleSetSimulscan(sl_cli_command_arg_t *args)
{
  // Make sure BLE mode is enabled so that the call below can succeed
  if (!sl_rail_ble_is_enabled(railHandle)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x31, "BLE mode not enabled");
    return;
  }

  sl_rail_status_t status = sl_rail_ble_config_phy_simulscan(railHandle);
  responsePrint(sl_cli_get_command_string(args, 0), "Status:%s", getStatusMessage(status));
}

// channel, accessAddress, crcInit, whitening
void bleSetChannelParams(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  // Default to the parameters for advertising channels
  sl_rail_ble_state_t params = {
    .crc_init = 0x00555555UL,
    .access_address = 0x8E89BED6UL,
    .logical_channel = 37,
    .disable_whitening = false,
  };
  sl_rail_status_t res;

  // Make sure BLE mode is enabled so that the call below can succeed
  if (!sl_rail_ble_is_enabled(railHandle)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x31, "BLE mode not enabled");
    return;
  }

  if (sl_cli_get_argument_count(args) >= 1) {
    params.logical_channel = sl_cli_get_argument_uint8(args, 0);
  }
  if (sl_cli_get_argument_count(args) >= 2) {
    params.access_address = sl_cli_get_argument_uint32(args, 1);
  }
  if (sl_cli_get_argument_count(args) >= 3) {
    params.crc_init = sl_cli_get_argument_uint32(args, 2);
  }
  if (sl_cli_get_argument_count(args) >= 4) {
    params.disable_whitening = !!sl_cli_get_argument_uint8(args, 3);
  }

  res = sl_rail_ble_config_channel_radio_params(railHandle, &params);
  if (res == SL_RAIL_STATUS_NO_ERROR) {
    responsePrint(sl_cli_get_command_string(args, 0),
                  "LogicalChannel:%d,"
                  "AccessAddress:0x%0.8X,"
                  "CrcInit:0x%0.8X,"
                  "Whitening:%s",
                  params.logical_channel,
                  params.access_address,
                  params.crc_init,
                  params.disable_whitening ? "Disabled" : "Enabled");
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x32, "Setting channel parameters failed");
  }
}

void bleAdvertisingConfig(sl_cli_command_arg_t *args)
{
  uint8_t advChannel = sl_cli_get_argument_uint8(args, 0);
  sl_rail_status_t res;

  // Make sure BLE mode is enabled so that the call below can succeed
  if (!sl_rail_ble_is_enabled(railHandle)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x31, "BLE mode not enabled");
    return;
  }

  if ((advChannel < 37) || (advChannel > 39)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x30, "Invalid advertising channel");
    return;
  }

  sl_rail_ble_state_t params = {
    .crc_init = 0x00555555UL,
    .access_address = 0x8E89BED6UL,
    .disable_whitening = false,
    .logical_channel = advChannel,
  };
  // Configure RAIL for BLE advertising on channel 37
  res = sl_rail_ble_config_channel_radio_params(railHandle, &params);
  if (res != SL_RAIL_STATUS_NO_ERROR) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x32, "Setting channel parameters failed");
    return;
  }

  if (advChannel == 37) {
    changeChannel(0); // Logical channel 37 is physical channel 0
  } else if (advChannel == 38) {
    changeChannel(12); // Logical channel 38 is physical channel 12
  } else if (advChannel == 39) {
    changeChannel(39); // Logical channel 39 is physical channel 39
  }

  responsePrint(sl_cli_get_command_string(args, 0), "AdvertisingChannel:%d", advChannel);

  // Load up a suitable advertising packet
  (void) memcpy(txData, blePacket, sizeof(blePacket));
  txDataLen = sizeof(blePacket);
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  printTxPacket(args);
}

void blePhySwitchToRx(sl_cli_command_arg_t *args)
{
  phySwitchToRx.iterations = sl_cli_get_argument_uint8(args, 0);
  if (phySwitchToRx.iterations > 0) {
    phySwitchToRx.phy = SL_RAIL_BLE_PHY_1_MBPS;
    phySwitchToRx.physicalChannel = 0U;
    phySwitchToRx.timeDelta = 1000U;
    phySwitchToRx.params.crc_init = 0x00555555UL;
    phySwitchToRx.params.access_address = 0x8E89BED6UL;
    phySwitchToRx.params.logical_channel = 37U;
    phySwitchToRx.params.disable_whitening = false;
    phySwitchToRx.extraDelayUs = 0U;

    if (sl_cli_get_argument_count(args) >= 2) {
      phySwitchToRx.phy = sl_cli_get_argument_uint8(args, 1);
    }
    if (sl_cli_get_argument_count(args) >= 3) {
      phySwitchToRx.timeDelta = sl_cli_get_argument_uint32(args, 2);
    }
    if (sl_cli_get_argument_count(args) >= 4) {
      phySwitchToRx.physicalChannel = sl_cli_get_argument_uint16(args, 3);
    }
    if (sl_cli_get_argument_count(args) >= 5) {
      phySwitchToRx.params.logical_channel = sl_cli_get_argument_uint16(args, 4);
    }
    if (sl_cli_get_argument_count(args) >= 6) {
      phySwitchToRx.params.access_address = sl_cli_get_argument_uint32(args, 5);
    }
    if (sl_cli_get_argument_count(args) >= 7) {
      phySwitchToRx.params.crc_init = sl_cli_get_argument_uint32(args, 6);
    }
    if (sl_cli_get_argument_count(args) >= 8) {
      phySwitchToRx.params.disable_whitening = !!sl_cli_get_argument_uint8(args, 7);
    }
    if (sl_cli_get_argument_count(args) >= 9) {
      phySwitchToRx.extraDelayUs = sl_cli_get_argument_uint8(args, 8);
    }
  }

  char *enabled = (phySwitchToRx.iterations > 0) ? "Enabled" : "Disabled";
  responsePrint(sl_cli_get_command_string(args, 0), "PhySwitchToRx:%s", enabled);
}

void enableBleSignalIdentifier(sl_cli_command_arg_t *args)
{
#if SL_RAIL_BLE_SUPPORTS_SIGNAL_IDENTIFIER
  sl_rail_status_t status;
  sl_rail_ble_signal_identifier_mode_t siMode = (sl_rail_ble_signal_identifier_mode_t)sl_cli_get_argument_uint8(args, 0);
  bool enable = (siMode != SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_DISABLE);

  if (sl_rail_ble_is_enabled(railHandle)) {
    status = sl_rail_ble_config_signal_identifier(railHandle, siMode);
    if (status == SL_RAIL_STATUS_NO_ERROR) {
      status = sl_rail_ble_enable_signal_detection(railHandle, enable);
    }
  } else {
    // Disable the signal identifier just to be safe. Ignore the return type
    // as we know it would be invalid call if it was not configured.
    (void) sl_rail_ble_config_signal_identifier(railHandle, SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_DISABLE);
    status = SL_RAIL_STATUS_INVALID_CALL;
  }
  responsePrint(sl_cli_get_command_string(args, 0), "Result:%s", getStatusMessage(status));
#else
  responsePrint(sl_cli_get_command_string(args, 0),
                "Signal identifier unsupported");
#endif
}
