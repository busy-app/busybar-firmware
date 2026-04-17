/***************************************************************************//**
 * @file
 * @brief This file implements commands for configuring MFM RAIL options
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
#include "app_common.h"
#include "sl_core.h"
#include "sl_rail_mfm.h"

#if SL_RAIL_SUPPORTS_MFM
#define MFM_RAW_BUF_SZ_BYTES 512

typedef struct MFM_Config_App {
  sl_rail_mfm_ping_pong_buffer_config_t buffer;
  sl_rail_state_timing_t timings;
} MFM_Config_App_t;

MFM_Config_App_t mfmConfig = {
  .buffer = {
    .p_buffer_0 = (&channelHoppingBufferSpace[0]),
    .p_buffer_1 = (&channelHoppingBufferSpace[MFM_RAW_BUF_SZ_BYTES / 4]),
    .buffer_words = (MFM_RAW_BUF_SZ_BYTES / 4)
  },
  .timings = {
    .idle_to_tx = 100,
    .idle_to_rx = 0,
    .rx_to_tx = 0,
    .tx_to_rx = 0,
    .rxsearch_timeout = 0,
    .tx_to_rxsearch_timeout = 0
  }
};

/******************* Local functions *******************/
static bool mfm_is_enabled(sl_rail_handle_t railHandle)
{
  (void) railHandle;
  return (railTxDataConfig.tx_source == SL_RAIL_TX_DATA_SOURCE_MFM_DATA);
}

static sl_rail_status_t mfm_init(sl_rail_handle_t railHandle,
                                 MFM_Config_App_t *config)
{
  sl_rail_status_t status;

  railTxDataConfig.tx_source = SL_RAIL_TX_DATA_SOURCE_MFM_DATA;
  status = sl_rail_set_mfm_ping_pong_fifo(railHandle,
                                          &(config->buffer));
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    return (status);
  }

  status = sl_rail_config_tx_data(railHandle, &railTxDataConfig);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    return (status);
  }

  return (sl_rail_set_state_timing(railHandle, &(config->timings)));
}

static sl_rail_status_t mfm_deinit(sl_rail_handle_t railHandle)
{
  if (!mfm_is_enabled(railHandle)) {
    return SL_RAIL_STATUS_INVALID_STATE;
  }

  sl_rail_status_t status;
  status = sl_rail_stop_tx(railHandle, SL_RAIL_STOP_MODES_ALL);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    return (status);
  }

  railTxDataConfig.tx_source = SL_RAIL_TX_DATA_SOURCE_PACKET_DATA;
  return (sl_rail_config_tx_data(railHandle, &railTxDataConfig));
}

void mfmStatus(sl_cli_command_arg_t *args)
{
  bool enabled = mfm_is_enabled(railHandle);

  // Report the current enabled status for MFM
  responsePrint(sl_cli_get_command_string(args, 0),
                "MFM:%s",
                enabled ? "Enabled" : "Disabled");
}

void mfmEnable(sl_cli_command_arg_t *args)
{
  if (sl_cli_get_argument_count(args) >= 1) {
    bool enable = !!sl_cli_get_argument_uint8(args, 0);

    // Turn MFM mode on or off as requested
    if (enable) {
      uint32_t idx;
      uint32_t *pDst0 = mfmConfig.buffer.p_buffer_0;
      uint32_t *pDst1 = mfmConfig.buffer.p_buffer_1;

      if (disableIncompatibleProtocols(SL_RAIL_PTI_PROTOCOL_CUSTOM) != SL_RAIL_STATUS_NO_ERROR) {
        responsePrintError(sl_cli_get_command_string(args, 0), 0x22, "Current protocol deinit failed");
        return;
      }
      mfm_init(railHandle, &mfmConfig);

      for (idx = 0; idx < (MFM_RAW_BUF_SZ_BYTES / 16); idx++) {
        pDst0[4 * idx + 0] = 0x755A3100;
        pDst1[4 * idx + 0] = 0x755A3100;
        pDst0[4 * idx + 1] = 0x315A757F;
        pDst1[4 * idx + 1] = 0x315A757F;
        pDst0[4 * idx + 2] = 0x8BA6CF00;
        pDst1[4 * idx + 2] = 0x8BA6CF00;
        pDst0[4 * idx + 3] = 0xCFA68B81;
        pDst1[4 * idx + 3] = 0xCFA68B81;
      }
    } else {
      mfm_deinit(railHandle);
    }
  }
  // Report the current status of MFM mode
  mfmStatus(args);
}

#else //!SL_RAIL_SUPPORTS_MFM

static void mfmNotSupported(int argc, char **argv)
{
  (void)argc;
  responsePrintError(argv[0], 0x56, "MFM is not suppported on this chip");
}

void mfmStatus(int argc, char **argv)
{
  mfmNotSupported(argc, argv);
}

void mfmEnable(int argc, char **argv)
{
  mfmNotSupported(argc, argv);
}

#endif //SL_RAIL_SUPPORTS_MFM
