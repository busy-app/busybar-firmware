/***************************************************************************//**
 * @file
 * @brief This file implements the packet commands for sl_rail_test applications.
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
#include "app_trx.h"

static uint16_t start;
static uint16_t end;
static bool isRandom = false; // Toggle flag for randomizing payload

void setTxRandomHelper(void)   // Randomizes payload
{
  if (isRandom) {
    // This check modifies end to txDataLen if txDataLen becomes smaller than the end index
    if (end > txDataLen) {
      end = txDataLen;
    }
    for (int index = start; index < end; index++) {
      txData[index] = rand() & 0xFFU;
    }
  }
}

void printTxPacket(sl_cli_command_arg_t *args)
{
  // Use the packet print helper to print out the transmit payload
  printPacket(sl_cli_get_command_string(args, 0),
              txData,
              txDataLen,
              NULL);
}

void setTxRandom(sl_cli_command_arg_t *args)
{
  isRandom = !isRandom;
  if (sl_cli_get_argument_count(args) > 0) {
    // If one argument given, assumed as start index and assigns end index as TxDataLen
    if (sl_cli_get_argument_count(args) == 1) {
      start = sl_cli_get_argument_uint16(args, 0);
      end = txDataLen;
    } else {
      // If 2 arguments are given, assign start and end indices to args
      start = sl_cli_get_argument_uint16(args, 0);
      end = sl_cli_get_argument_uint16(args, 1);
    }
    if (end > txDataLen) {
      // Initial check for start and end indeces. The setTxRandomHelper() calls another check if txDataLen changes
      responsePrintError(sl_cli_get_command_string(args, 0), 0x06, "Invalid Payload Range (Payload Range 0-%d)", txDataLen);
      return;
    }
    if (start > end) {
      responsePrintError(sl_cli_get_command_string(args, 0),
                         0x07,
                         "Starting Index %u cannot be greater than the Ending Index %u",
                         start,
                         end);
      return;
    }
    setTxRandomHelper();
    printTxPacket(args);
  }
  responsePrint(sl_cli_get_command_string(args, 0),
                "Randomization:%s,StartIndex:%u,EndIndex:%u",
                (isRandom ? "On" : "Off"),
                start,
                end);
}

/*
 * The sl_cli_get_argument_count() can include the start index then
 * payload. Remove 1 from the count when checking "> sizeof(txData)" to
 * compensate for the index and leave just the size of the payload.
 * The test test_payloadMultipleFifoSize checks for
 * exactly MAX_PAYLOAD = 1024 and if the offset count is included the
 * check is throws the check off by 1.
 * Specifically, the test being run is:
 *  "setTxPayloadQuiet 1008 240 241 242 243 244 245 246 247 248 249 250 251 252 253 254 255"
 * which results in offset = 1008 and count = 17 so 1 must be removed
 * from the count to accommodate the precise MAX_PAYLOAD = 1024.
 * Reducing the count during the check in this way does not affect
 * any other tests.
 */
static bool setTxPayloadHelper(sl_cli_command_arg_t *args)
{
  uint16_t offset = sl_cli_get_argument_uint16(args, 0);
  // Make sure this fits in the buffer
  uint8_t count = (uint8_t)sl_cli_get_argument_count(args);
  if ((offset + count - 1U) > sizeof(txData)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 5, "Data overflows txData buffer");
    return false;
  }
  // Read as many bytes as have been supplied and set them
  for (int i = 1; i < count; i++) {
    uint32_t index = offset + i - 1;
    uint8_t value = sl_cli_get_argument_uint8(args, i);
    txData[index] = value;
  }
  if (railTxDataConfig.tx_method == SL_RAIL_DATA_METHOD_PACKET_MODE) {
    sl_rail_write_tx_fifo(railHandle, txData, txDataLen, true);
  }
  return true;
}

void setTxPayload(sl_cli_command_arg_t *args)
{
  if (setTxPayloadHelper(args)) {
    args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
    printTxPacket(args);
  }
}

void setTxPayloadQuiet(sl_cli_command_arg_t *args)
{
  if (setTxPayloadHelper(args)) {
    uint16_t start = sl_cli_get_argument_uint16(args, 0);
    // Ignore first two arguments in end (command name, start index)
    responsePrint(sl_cli_get_command_string(args, 0), "start:%u,end:%u", start, start + sl_cli_get_argument_count(args) - 1);
  }
}

void setRfSenseTxPayload(sl_cli_command_arg_t *args)
{
  sl_rail_status_t status = SL_RAIL_STATUS_INVALID_CALL;
  if (sl_cli_get_argument_count(args) >= 2) {
    uint8_t syncwordNumBytes = sl_cli_get_argument_uint8(args, 0);
    uint32_t syncword = sl_cli_get_argument_uint32(args, 1);
    if (railTxDataConfig.tx_method == SL_RAIL_DATA_METHOD_PACKET_MODE) {
      status = sl_rail_set_rf_sense_selective_ook_wakeup_payload(railHandle,
                                                                 syncwordNumBytes,
                                                                 syncword);
    }
    responsePrint(sl_cli_get_command_string(args, 0), "RFSense Payload:%s",
                  (status == SL_RAIL_STATUS_NO_ERROR) ? "Set" : "Not Set");
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x5, "Invalid inputs");
  }
}

void setTxLength(sl_cli_command_arg_t *args)
{
  uint32_t length = sl_cli_get_argument_uint32(args, 0);

  if (length > sizeof(txData)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 6, "Invalid length %d", length);
    return;
  }

  txDataLen = length;
  if (railTxDataConfig.tx_method == SL_RAIL_DATA_METHOD_PACKET_MODE) {
    length = sl_rail_write_tx_fifo(railHandle, txData, txDataLen, true);
  }
  responsePrint(sl_cli_get_command_string(args, 0), "TxLength:%d,TxLength Written:%d", txDataLen, length);
}

void printAckPacket(sl_cli_command_arg_t *args)
{
  // Use the packet print helper to print out the transmit payload
  printPacket(sl_cli_get_command_string(args, 0),
              ackData,
              ackDataLen,
              NULL);
}

static void updateAckPayload(sl_cli_command_arg_t *args)
{
  uint8_t *pPkt = ackData;
  sl_rail_status_t status;
  if (txAckDirect) {
    status = sl_rail_get_auto_ack_fifo(railHandle, &pPkt, NULL);
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      responsePrintError(sl_cli_get_command_string(args, 0), status, "sl_rail_get_auto_ack_fifo failed: %d", status);
      return;
    }
    (void) memcpy(pPkt, ackData, ackDataLen);
    pPkt = NULL; // Not strictly necessary
  }
  status = sl_rail_write_auto_ack_fifo(railHandle, pPkt, ackDataLen);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    responsePrintError(sl_cli_get_command_string(args, 0), status, "sl_rail_write_auto_ack_fifo failed: %d", status);
    return;
  }
  printAckPacket(args);
}

void setAckPayload(sl_cli_command_arg_t *args)
{
  uint16_t offset = sl_cli_get_argument_uint16(args, 0);

  // Read as many bytes as have been supplied and set them
  for (int i = 2; i < sl_cli_get_argument_count(args) + 1; i++) {
    uint32_t index = offset + i - 2;
    uint8_t value = sl_cli_get_argument_uint8(args, i - 1);

    // Make sure this fits in the txData buffer
    if (index >= sizeof(ackData)) {
      responsePrintError(sl_cli_get_command_string(args, 0), 5, "Data overflows ackData buffer");
      return;
    }
    ackData[index] = value;
  }
  updateAckPayload(args);
}

void setAckLength(sl_cli_command_arg_t *args)
{
  uint32_t length = sl_cli_get_argument_uint32(args, 0);

  if (length > sizeof(ackData)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 6, "Invalid length %d", length);
    return;
  }

  // Make sure we're using the txData array and set the length
  ackDataLen = length;
  updateAckPayload(args);
}

void setFixedLength(sl_cli_command_arg_t *args)
{
  if ((!inAppMode(NONE, sl_cli_get_command_string(args, 0)))
      || (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0)))) {
    return;
  }
  uint16_t fixedLength = sl_cli_get_argument_uint16(args, 0);
  fixedLength = sl_rail_set_fixed_length(railHandle, fixedLength);
  configRxLengthSetting(fixedLength);

  // Print configured length
  responsePrint(sl_cli_get_command_string(args, 0), "FixedLength:%d", fixedLength);
}

void dataConfig(sl_cli_command_arg_t *args)
{
  if ((!inAppMode(NONE, sl_cli_get_command_string(args, 0)))
      || (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0)))) {
    return;
  }

  sl_rail_rx_data_config_t newRxConfig = railRxDataConfig; // local struct copy
  sl_rail_tx_data_config_t newTxConfig = railTxDataConfig; // local struct copy

  if (memcmp(sl_cli_get_argument_string(args, 0), "pkt", 3) == 0) {
    newTxConfig.tx_method = SL_RAIL_DATA_METHOD_PACKET_MODE;
  } else if (memcmp(sl_cli_get_argument_string(args, 0), "fifo", 4) == 0) {
    newTxConfig.tx_method = SL_RAIL_DATA_METHOD_FIFO_MODE;
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x50, "Invalid Tx Data Method selection.");
  }

  if (memcmp(sl_cli_get_argument_string(args, 1), "pkt", 3) == 0) {
    newRxConfig.rx_method = SL_RAIL_DATA_METHOD_PACKET_MODE;
  } else if (memcmp(sl_cli_get_argument_string(args, 1), "fifo", 4) == 0) {
    newRxConfig.rx_method = SL_RAIL_DATA_METHOD_FIFO_MODE;
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x50, "Invalid Rx Data Method selection.");
  }

  newRxConfig.rx_source = SL_RAIL_RX_DATA_SOURCE_PACKET_DATA; // Default
  if (sl_cli_get_argument_count(args) >= 3) {
    newRxConfig.rx_source = (sl_rail_rx_data_source_t)sl_cli_get_argument_uint8(args, 2);
  }

#if SL_RAIL_SUPPORTS_MFM
  newTxConfig.tx_source = SL_RAIL_TX_DATA_SOURCE_PACKET_DATA; // Default
  if (sl_cli_get_argument_count(args) >= 4) {
    newTxConfig.tx_source = (sl_rail_tx_data_source_t)sl_cli_get_argument_uint8(args, 3);
  }
#else
  // newTxConfig.tx_source is unchanged
#endif

  sl_rail_status_t statusRx = sl_rail_config_rx_data(railHandle, &newRxConfig);
  sl_rail_status_t statusTx = sl_rail_config_tx_data(railHandle, &newTxConfig);
  if ((statusRx != SL_RAIL_STATUS_NO_ERROR) || (statusTx != SL_RAIL_STATUS_NO_ERROR)) {
    // In case one worked and the other didn't, restore both to original state
    (void) sl_rail_config_rx_data(railHandle, &railRxDataConfig);
    (void) sl_rail_config_tx_data(railHandle, &railTxDataConfig);
    if (statusRx != SL_RAIL_STATUS_NO_ERROR) {
      responsePrintError(sl_cli_get_command_string(args, 0), 0x50, "Failed to successfully call sl_rail_config_rx_data: %d", statusRx);
    }
    if (statusTx != SL_RAIL_STATUS_NO_ERROR) {
      responsePrintError(sl_cli_get_command_string(args, 0), 0x50, "Failed to successfully call sl_rail_config_tx_data: %d", statusTx);
    }
    return;
  }

  // Reset RX FIFO and rxRawSourceBytes counter for a new rxSource
  if (railRxDataConfig.rx_source != newRxConfig.rx_source) {
    counters.rxRawSourceBytes = 0;
    sl_rail_reset_fifo(railHandle, false, true);
  }

  railRxDataConfig = newRxConfig; // global struct copy
  railTxDataConfig = newTxConfig; // global struct copy

  responsePrint(sl_cli_get_command_string(args, 0),
                "TxMethod:%s,"
                "TxSource:%u,"
                "RxMethod:%s,"
                "RxSource:%u",
                (newTxConfig.tx_method == SL_RAIL_DATA_METHOD_PACKET_MODE
                 ? "pkt" : "fifo"),
                newTxConfig.tx_source,
                (newRxConfig.rx_method == SL_RAIL_DATA_METHOD_PACKET_MODE
                 ? "pkt" : "fifo"),
                newRxConfig.rx_source);
}

static uint16_t origRxFifoBytes = 0U;
static sl_rail_fifo_buffer_align_t *origRxFifoBuffer = NULL;

void setRxFifo(sl_cli_command_arg_t *args)
{
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }
  if (origRxFifoBytes == 0U) {
    const sl_rail_config_t *config = sl_rail_get_config(railHandle);
    origRxFifoBytes = config->rx_fifo_bytes;
    origRxFifoBuffer = config->p_rx_fifo_buffer;
  }
  uint16_t reqFifoBytes = sl_cli_get_argument_uint16(args, 0);
  if (reqFifoBytes > origRxFifoBytes) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x54, "Max RX FIFO size in this build is %u bytes",
                       origRxFifoBytes);
    return;
  }
  if (reqFifoBytes == 0U) { // Restore to max size
    reqFifoBytes = origRxFifoBytes;
  }
  sl_rail_status_t status = sl_rail_set_rx_fifo(railHandle, origRxFifoBuffer,
                                                &reqFifoBytes);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x55, "Setting RX FIFO failed; Min RX FIFO size is 64 bytes");
    return;
  }
  responsePrint(sl_cli_get_command_string(args, 0),
                "RxFifoSize:%u", reqFifoBytes);
}

static uint16_t origTxFifoBytes = 0U;
static sl_rail_fifo_buffer_align_t *origTxFifoBuffer = NULL;

void setTxFifo(sl_cli_command_arg_t *args)
{
  // args is NULL when called from app_main.c::sl_rail_test_internal_app_init()
  // to record the orig TX FIFO established by the sl_rail_util_init component.
  if ((args == NULL) || (origTxFifoBytes == 0U)) {
    const sl_rail_config_t *config = sl_rail_get_config(railHandle);
    origTxFifoBytes = config->tx_fifo_bytes;
    origTxFifoBuffer = config->p_tx_fifo_buffer;
    if (args == NULL) {
      return; // Our job of recording original TX FIFO is done
    }
  }
  uint16_t reqFifoBytes = sl_cli_get_argument_uint16(args, 0);
  if (reqFifoBytes > origTxFifoBytes) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x54, "Max TX FIFO size in this build is %u bytes",
                       origTxFifoBytes);
    return;
  }
  if (reqFifoBytes == 0U) { // Restore to max size
    reqFifoBytes = origTxFifoBytes;
  }
  sl_rail_status_t status = sl_rail_set_tx_fifo(railHandle, origTxFifoBuffer,
                                                reqFifoBytes, 0U, 0U);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    responsePrintError(sl_cli_get_command_string(args, 0), status,
                       "Setting TX FIFO failed");
    return;
  }
  responsePrint(sl_cli_get_command_string(args, 0),
                "TxFifoBytes:%u,Success:True",
                reqFifoBytes);
}

uint8_t *getTxFifoBaseAddress(void)
{
  return (uint8_t *)origTxFifoBuffer;
}

void fifoModeTestOptions(sl_cli_command_arg_t *args)
{
  char *outputStr[] = { "Disabled", "Enabled" };
  txFifoManual = !!sl_cli_get_argument_uint8(args, 0);
  rxFifoManual = !!sl_cli_get_argument_uint8(args, 1);

  responsePrint(sl_cli_get_command_string(args, 0),
                "TxFifoManual:%s,"
                "RxFifoManual:%s",
                outputStr[txFifoManual],
                outputStr[rxFifoManual]);
}

/**
 * Specify how many bytes to read out of the fifo and print to terminal
 */
void rxFifoManualRead(sl_cli_command_arg_t *args)
{
  if (!rxFifoManual) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x51, "Must be in rx fifo manual mode (fifoModeTestOptions).");
  } else {
    bool readAppendedInfo = !!sl_cli_get_argument_uint8(args, 0);
    uint16_t bytesToRead = sl_cli_get_argument_uint16(args, 1);
    bool printTimingInfo = (sl_cli_get_argument_count(args) >= 3) ? !!sl_cli_get_argument_uint8(args, 2) : false;
    RailAppEvent_t *packetData
      = (RailAppEvent_t *)sl_malloc(sizeof(RailAppEvent_t) + bytesToRead);

    if (packetData == NULL) {
      sl_rail_release_rx_packet(railHandle, SL_RAIL_RX_PACKET_HANDLE_OLDEST);
      responsePrintError(sl_cli_get_command_string(args, 0),
                         SL_STATUS_ALLOCATION_FAILED,
                         "Unable to allocate memory to read up to %u bytes, try "
                         "fewer bytes.", bytesToRead
                         );
      return;
    }
    sl_rail_rx_packet_details_t *appendedInfo = &packetData->rxPacket.appendedInfo;

    // dataLength is number of bytes read from the fifo
    packetData->type = RX_PACKET;
    packetData->rxPacket.dataPtr = (uint8_t *)&packetData[1]; // Point beyond RailAppEvent_t
    packetData->rxPacket.dataLength = sl_rail_read_rx_fifo(railHandle, packetData->rxPacket.dataPtr,
                                                           bytesToRead);
    packetData->rxPacket.freqOffset = getRxFreqOffset();
    if (readAppendedInfo) {
      sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;
      // Note the packet's status
      sl_rail_rx_packet_info_t packetInfo;
      sl_rail_rx_packet_handle_t packetHandle
        = sl_rail_get_rx_packet_info(railHandle, SL_RAIL_RX_PACKET_HANDLE_OLDEST,
                                     &packetInfo);
      // assert(packetHandle != NULL);
      packetData->rxPacket.packetStatus = packetInfo.packet_status;

      // Get the appended info details and release this packet
      status = sl_rail_get_rx_packet_details(railHandle, packetHandle, appendedInfo);
      sl_rail_release_rx_packet(railHandle, packetHandle);

      if (status == SL_RAIL_STATUS_NO_ERROR) {
        appendedInfo->time_received.total_packet_bytes = packetData->rxPacket.dataLength;
        if (printTimingInfo) {
          typedef sl_rail_status_t (*Adjustment)(sl_rail_handle_t, sl_rail_rx_packet_details_t *);
          Adjustment positionFuncs[] = {
            &sl_rail_get_rx_time_preamble_start,
            &sl_rail_get_rx_time_sync_word_end,
            &sl_rail_get_rx_time_frame_end,
          };
          #define NUM_POSITIONS (sizeof(positionFuncs) / sizeof(positionFuncs[0]))
          sl_rail_time_t times[NUM_POSITIONS];
          for (uint8_t i = 0; i < NUM_POSITIONS; i++) {
            sl_rail_rx_packet_details_t detailsCopy = *appendedInfo;
            (void) positionFuncs[i](railHandle, &detailsCopy);
            times[i] = detailsCopy.time_received.packet_time;
          }
          responsePrint(sl_cli_get_command_string(args, 0),
                        "Pre:%u,Sync:%u,End:%u,PreAlt:%u,SyncAlt:%u,EndAlt:%u",
                        times[0], times[1], times[2],
                        times[0], times[1], times[2]);
        }
        (void) sl_rail_get_rx_time_sync_word_end(railHandle, appendedInfo);
      } else {
        memset(&packetData->rxPacket.appendedInfo, 0, sizeof(sl_rail_rx_packet_details_t));
        packetData->rxPacket.appendedInfo.rssi_dbm = SL_RAIL_RSSI_INVALID_DBM;
        if (packetData->rxPacket.dataLength == 0) {
          responsePrintError(sl_cli_get_command_string(args, 0), 0x52, "No packet found in rx fifo!");
          sl_free((void *)packetData);
          return;
        }
      }
    }

    queueAdd(&railAppEventQueue, (void *)packetData);
  }
}

void fifoReset(sl_cli_command_arg_t *args)
{
  char *outputStr[] = { "DoNothing", "Reset" };
  bool txReset = !!sl_cli_get_argument_uint8(args, 0);
  bool rxReset = !!sl_cli_get_argument_uint8(args, 1);

  sl_rail_reset_fifo(railHandle, txReset, rxReset);

  responsePrint(sl_cli_get_command_string(args, 0),
                "TxFifo:%s,"
                "RxFifo:%s",
                outputStr[txReset],
                outputStr[rxReset]);
}

void txFifoManualLoad(sl_cli_command_arg_t *args)
{
  if (!txFifoManual) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x51, "Must be in tx fifo manual mode (fifoModeTestOptions).");
  } else {
    // Test loading unaligned data
    loadTxData(txData, 1U);
    loadTxData(&txData[1], txDataLen - 1U);
    responsePrint(sl_cli_get_command_string(args, 0), "Status:Fifo Written");
  }
}

void abortRxPacket(sl_cli_command_arg_t *args)
{
  abortRxDelay = sl_cli_get_argument_int32(args, 0);

  responsePrint(sl_cli_get_command_string(args, 0), "TimeAfterSyncToAbort:%d", abortRxDelay);
}

void peekRx(sl_cli_command_arg_t *args)
{
  uint16_t offset = 0;
  uint16_t bytesToPeek = sl_cli_get_argument_uint16(args, 0);
  uint8_t pDst[10];
  if (bytesToPeek > 10) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x52, "Can't peek more than 10 bytes");
    return;
  }
  if (sl_cli_get_argument_count(args) >= 2) {
    offset = sl_cli_get_argument_uint16(args, 1);
  }
  if (sl_rail_peek_rx_packet(railHandle, SL_RAIL_RX_PACKET_HANDLE_OLDEST,
                             pDst, bytesToPeek, offset) != bytesToPeek) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x53, "Requested bytes not in receive buffer");
    return;
  }
  printPacket(sl_cli_get_command_string(args, 0), pDst, bytesToPeek, NULL);
}
