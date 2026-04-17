/***************************************************************************//**
 * @file
 * @brief This file provides functionality to test RAIL error rates.
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

#include "response_print.h"

#include "sl_rail.h"
#include "app_common.h"
#include "app_trx.h"
#include "sl_core.h"
#include "sl_rail_test_config.h"

// Variables for PER testing
uint32_t perCount;
uint32_t perDelay;

// Variables for BER testing
BerStatus_t berStats = { 0 };
bool berTestModeEnabled = false;
static uint32_t berBytesToTest = 0;

void startPerMode(sl_cli_command_arg_t *args)
{
#if defined(SL_RAIL_TEST_PER_PORT) && defined(SL_RAIL_TEST_PER_PIN)
  uint32_t packets = sl_cli_get_argument_uint32(args, 0);
  uint32_t delayUs = sl_cli_get_argument_uint32(args, 1);

  if (!enableAppModeSync(PER, packets != 0, sl_cli_get_command_string(args, 0))) {
    return;
  }
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  resetCounters(args);

  perCount = packets;
  perDelay = delayUs / 2;
  if (packets > 0) {
    sl_rail_set_timer(railHandle, perDelay, SL_RAIL_TIME_DELAY, &railtest_TimerExpired);
  }
#else
  responsePrintError(sl_cli_get_command_string(args, 0), 0, "To run PER commands, SL_RAIL_TEST_PER_PORT and SL_RAIL_TEST_PER_PIN must be configured for use.");
#endif //defined(SL_RAIL_TEST_PER_PORT) && defined(SL_RAIL_TEST_PER_PIN)
}

void updateStats(int32_t newValue, Stats_t *stats)
{
  stats->samples++;
  if (stats->samples == 1) {
    stats->min = newValue;
    stats->max = newValue;
    stats->mean = newValue;
    stats->varianceTimesSamples = 0;
  } else {
    stats->min = (newValue < stats->min) ? newValue : stats->min;
    stats->max = (newValue > stats->max) ? newValue : stats->max;

    float delta = newValue - stats->mean;
    stats->mean += delta / stats->samples;
    // wikipedia.org/wiki/Algorithms_for_calculating_variance#On-line_algorithm
    // Update by adding (newValue - oldMean) * (newValue - newMean)
    stats->varianceTimesSamples += delta * (newValue - stats->mean);
  }
}

float variance(const Stats_t stats)
{
  return stats.varianceTimesSamples / (stats.samples - 1);
}

void getPerStats(sl_cli_command_arg_t *args)
{
#if defined(SL_RAIL_TEST_PER_PORT) && defined(SL_RAIL_TEST_PER_PIN)
  char bufRssiMean[10];
  char bufRssiMin[10];
  char bufRssiMax[10];
  char bufRssiVariance[10];

  sprintfFloat(bufRssiMean, sizeof(bufRssiMean), counters.rssi.mean / 4, 0);
  sprintfFloat(bufRssiMin, sizeof(bufRssiMin), ((float) counters.rssi.min) / 4, 2);
  sprintfFloat(bufRssiMax, sizeof(bufRssiMax), ((float) counters.rssi.max) / 4, 2);
  sprintfFloat(bufRssiVariance, sizeof(bufRssiVariance), variance(counters.rssi) / 16, 0);

  responsePrint(sl_cli_get_command_string(args, 0),
                "PerTriggers:%u,"
                "RssiMean:%s,"
                "RssiMin:%s,"
                "RssiMax:%s,"
                "RssiVariance:%s",
                counters.perTriggers,
                bufRssiMean,
                bufRssiMin,
                bufRssiMax,
                bufRssiVariance);
#else
  responsePrintError(sl_cli_get_command_string(args, 0), 0, "To run PER commands, SL_RAIL_TEST_PER_PORT and SL_RAIL_TEST_PER_PIN must be configured for use.");
#endif //defined(SL_RAIL_TEST_PER_PORT) && defined(SL_RAIL_TEST_PER_PIN)
}

void berResetStats(uint32_t numBytes)
{
  // Reset test statistics
  memset(&berStats, 0, sizeof(BerStatus_t));

  // 0x1FFFFFFF bytes (0xFFFFFFF8 bits) is max number of bytes that can be
  // tested without uint32_t math rollover; numBytes = 0 is same as max
  if ((0 == numBytes) || (numBytes > 0x1FFFFFFF)) {
    numBytes = 0x1FFFFFFF;
  }
  berStats.bytesTotal = numBytes;
}

void berConfigSet(sl_cli_command_arg_t *args)
{
  sl_rail_status_t status;
  uint16_t rxThreshold, packetLength;

  sl_rail_idle(railHandle, SL_RAIL_IDLE_ABORT, true);
  sl_rail_reset_fifo(railHandle, true, true);

  // configure radio
  railTxDataConfig.tx_source = SL_RAIL_TX_DATA_SOURCE_PACKET_DATA;
  railRxDataConfig.rx_source = SL_RAIL_RX_DATA_SOURCE_PACKET_DATA;
  railTxDataConfig.tx_method = SL_RAIL_DATA_METHOD_FIFO_MODE;
  railRxDataConfig.rx_method = SL_RAIL_DATA_METHOD_FIFO_MODE;
  status = sl_rail_config_tx_data(railHandle, &railTxDataConfig);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    responsePrintError(sl_cli_get_command_string(args, 0), status, "Error calling sl_rail_config_tx_data().");
  }
  status = sl_rail_config_rx_data(railHandle, &railRxDataConfig);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    responsePrintError(sl_cli_get_command_string(args, 0), status, "Error calling sl_rail_config_rx_data().");
  }

  // configure RX FIFO
  rxThreshold = 100;
  rxThreshold = sl_rail_set_rx_fifo_threshold(railHandle, rxThreshold);

  // specify overall packet length info (infinite)
  packetLength = 0;
  packetLength = sl_rail_set_fixed_length(railHandle, packetLength);

  sl_rail_enable_pti(railHandle, false);

  berBytesToTest = sl_cli_get_argument_uint32(args, 0);
  berResetStats(berBytesToTest);

  responsePrint(sl_cli_get_command_string(args, 0), "NumBytes:%d", berBytesToTest);
}

void berRx(sl_cli_command_arg_t *args)
{
  bool enable = !!sl_cli_get_argument_uint8(args, 0);

  if (!enableAppModeSync(BER, enable, sl_cli_get_command_string(args, 0))) {
    return;
  }
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  resetCounters(args);

  sl_rail_idle(railHandle, SL_RAIL_IDLE_ABORT, true);
  sl_rail_reset_fifo(railHandle, true, true);
  if (enable) {
    sl_rail_enable_pti(railHandle, false);
    berResetStats(berBytesToTest);
    berTestModeEnabled = true;
    sl_rail_start_rx(railHandle, channel, NULL);
  }
}

void berStatusGet(sl_cli_command_arg_t *args)
{
  float percentDone;
  float percentBitError;
  uint32_t bytesTotal; /**< Number of bytes to receive */
  uint32_t bytesTested; /**< Number of bytes currently tested */
  uint32_t bitErrors; /**< Number of bits errors detected */
  int8_t rssi; /**< Current RSSI value during pattern acquisition */
  CORE_DECLARE_IRQ_STATE;

  // be sure we don't get half new, half stale data
  CORE_ENTER_CRITICAL();
  bytesTotal = berStats.bytesTotal;
  bytesTested = berStats.bytesTested;
  bitErrors = berStats.bitErrors;
  rssi = berStats.rssi;
  CORE_EXIT_CRITICAL();

  // don't divide by 0
  if (0 != bytesTotal) {
    percentDone = (float)((((double)bytesTested) / bytesTotal) * 100);
  } else {
    percentDone = 0.0;
  }
  // don't divide by 0
  if (0 != bytesTested) {
    percentBitError = (float)((((double)bitErrors) / (bytesTested * 8)) * 100);
  } else {
    percentBitError = 0.0;
  }

  // If rxOfEvent is > 0, then we're overflowing the incoming RX buffer
  // probably because the BER test isn't processing incoming bits fast
  // enough. The test will automatically try to re-synchronize and read in bits
  // from the stream, but the bits under test will not be continuous. Abort
  // testing and notify the user if this is the case.
  if (counters.rxOfEvent) {
    responsePrint(sl_cli_get_command_string(args, 0),
                  "BitsToTest:%u,"
                  "BitsTested:0,"
                  "PercentDone:0.00,"
                  "RSSI:%d,"
                  "BitErrors:0,"
                  "PercentBitError:0.00,"
                  "Status:TestAbortedRxOverflow",
                  bytesTotal * 8,
                  rssi);
  } else {
    char bufPercentDone[10];
    char bufPercentBitError[10];

    sprintfFloat(bufPercentDone, sizeof(bufPercentDone), percentDone, 2);
    sprintfFloat(bufPercentBitError, sizeof(bufPercentBitError), percentBitError, 2);

    responsePrint(sl_cli_get_command_string(args, 0),
                  "BitsToTest:%u,"
                  "BitsTested:%u,"
                  "PercentDone:%s,"
                  "RSSI:%d,"
                  "BitErrors:%u,"
                  "PercentBitError:%s",
                  bytesTotal * 8,
                  bytesTested * 8,
                  bufPercentDone,
                  rssi,
                  bitErrors,
                  bufPercentBitError);
  }
}

void throughput(sl_cli_command_arg_t *args)
{
  uint32_t numberOfPackets = sl_cli_get_argument_uint32(args, 0);
  sl_rail_status_t txStatus = SL_RAIL_STATUS_INVALID_STATE;
  uint32_t start = sl_rail_get_time(railHandle);
  if (sl_rail_write_tx_fifo(railHandle,
                            txData,
                            txDataLen,
                            true) != txDataLen) {
    responsePrint(sl_cli_get_command_string(args, 0), "sl_rail_write_tx_fifo Error");
    return;
  }
  for (uint32_t packets = 0; packets < numberOfPackets; packets++) {
    txStatus = SL_RAIL_STATUS_INVALID_STATE;
    while (txStatus != SL_RAIL_STATUS_NO_ERROR) {
      txStatus = sl_rail_start_tx(railHandle, channel, SL_RAIL_TX_OPTIONS_DEFAULT, NULL);
    }
    if (sl_rail_write_tx_fifo(railHandle,
                              txData,
                              txDataLen,
                              true) != txDataLen) {
      responsePrint(sl_cli_get_command_string(args, 0), "sl_rail_write_tx_fifo Error");
      return;
    }
  }
  uint32_t stop = sl_rail_get_time(railHandle);
  responsePrint(sl_cli_get_command_string(args, 0),
                "elapsedTime:%u", (stop - start));
}
