/***************************************************************************//**
 * @file
 * @brief RAILtest transmit and receive events
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
#include <stdlib.h>
#include <string.h>

#include "app_common.h"
#include "sl_rail_ble.h"
#include "sl_rail_ieee802154.h"

#include "app_trx.h"
#include "response_print.h"
#ifdef SL_CATALOG_CS_CLI_PRESENT
#include "railapp_cs.h"
#endif
/******************************************************************************
 * Variables
 *****************************************************************************/
sl_rail_tx_packet_details_t previousTxAppendedInfo = { .is_ack = false, };
sl_rail_tx_packet_details_t previousTxAckAppendedInfo = { .is_ack = true, };

static uint16_t dataLeft = 0;
static uint8_t *dataLeftPtr = NULL;

// Variables to keep track of the receive packet
static uint16_t rxLengthTarget = SL_RAIL_SET_FIXED_LENGTH_INVALID;
static uint16_t rxLengthCount;
static RailAppEvent_t *rxFifoPacketData;
static uint8_t *currentRxFifoPacketPtr;

int32_t abortRxDelay = 0;

// Used for wakeup from sleep
volatile bool rxPacketEvent = false;

// fifo mode test variables
/*
 * This test bit is used to prevent sl_rail_test from automatically loading packet
 * data into transmit fifo
 */
bool txFifoManual = false;
/*
 * This test bit is used to prevent sl_rail_test from automatically reading out the
 * packet from the fifo
 */
bool rxFifoManual = false;

// To enable power manager sleep from the main loop.
extern volatile bool allowPowerManagerSleep;

/******************************************************************************
 * Static
 *****************************************************************************/
static void packetMode_RxPacketAborted(sl_rail_handle_t railHandle)
{
  if (!printRxErrorPackets) {
    return;
  }
  sl_rail_rx_packet_info_t packetInfo;
  sl_rail_rx_packet_handle_t packetHandle
    = sl_rail_get_rx_packet_info(railHandle, SL_RAIL_RX_PACKET_HANDLE_NEWEST,
                                 &packetInfo);
  if (packetHandle == SL_RAIL_RX_PACKET_HANDLE_INVALID) {
    return;
  }
  if (logLevel & ASYNC_RESPONSE) {
    // Get a memory buffer for the received packet details
    uint16_t length = packetInfo.packet_bytes;
    RailAppEvent_t *rxPacket
      = (RailAppEvent_t *)sl_malloc(sizeof(RailAppEvent_t) + length);
    if (rxPacket != NULL) {
      rxPacket->type = RX_PACKET;
      if (length > 0U) {
        // Read any non-rolled-back packet data into our packet structure
        uint8_t *rxPacketData = (uint8_t *)&rxPacket[1]; // Point beyond RailAppEvent_t
        sl_rail_copy_rx_packet(railHandle, rxPacketData, &packetInfo);
        rxPacket->rxPacket.dataPtr =  rxPacketData;
      } else {
        rxPacket->rxPacket.dataPtr = NULL;
      }
      rxPacket->rxPacket.packetStatus = packetInfo.packet_status;
      rxPacket->rxPacket.dataLength = length;
      rxPacket->rxPacket.freqOffset = getRxFreqOffset();
      rxPacket->rxPacket.filterMask = packetInfo.filter_mask;
      rxPacket->rxPacket.packetHeld = false;
      // Read what packet details are available into our packet structure
      if (sl_rail_get_rx_packet_details(railHandle, packetHandle,
                                        &rxPacket->rxPacket.appendedInfo)
          != SL_RAIL_STATUS_NO_ERROR) {
        // assert(false);
        memset(&rxPacket->rxPacket.appendedInfo, 0, sizeof(rxPacket->rxPacket.appendedInfo));
      }
      // Copy this received packet into our circular queue
      queueAdd(&railAppEventQueue, (void *)rxPacket);
      // Do not include Rx Error packets in PER's RSSI stats:
      // updateStats(rxPacket->appendedInfo.rssi, &counters.rssi);
    } else {
      counters.noRxBuffer++;
      eventsMissed++;
    }
  }
}

static void packetMode_RxPacketReceived(sl_rail_handle_t railHandle)
{
  if (((rxSuccessTransition == SL_RAIL_RF_STATE_TX) && fillTxPacketOnRx)
      || (sl_rail_is_auto_ack_enabled(railHandle) && afterRxUseTxBufferForAck)) {
    // Load packet for either the non-AutoACK RXSUCCESS => TX transition,
    // or for the ACK transition when we intend to use the TX buffer
    // We don't do this in other circumstances in case of an RX2TX
    // transition to send a packet that's already been loaded,
    // which could cause TX underflow if we were to disturb it.
    sl_rail_write_tx_fifo(railHandle, txData, txDataLen, true);
  }

  if (rxHeld
      && (sl_rail_hold_rx_packet(railHandle) != SL_RAIL_RX_PACKET_HANDLE_INVALID)) {
    packetsHeld++;
  } else {
    (void)processRxPacket(railHandle, SL_RAIL_RX_PACKET_HANDLE_NEWEST, false);
  }
}

sl_rail_rx_packet_handle_t processRxPacket(sl_rail_handle_t railHandle,
                                           sl_rail_rx_packet_handle_t packetHandle,
                                           bool heldPacket)
{
  sl_rail_status_t status;
  sl_rail_rx_packet_details_t details;
  sl_rail_rx_packet_info_t packetInfo;
  packetHandle = sl_rail_get_rx_packet_info(railHandle, packetHandle, &packetInfo);
  if (packetHandle == SL_RAIL_RX_PACKET_HANDLE_INVALID) {
    return packetHandle;
  }
  if (packetInfo.packet_status == SL_RAIL_RX_PACKET_READY_CRC_ERROR) {
    counters.receiveCrcErrDrop++; // counters.receive still counts such too
  }
  uint16_t length = packetInfo.packet_bytes;

  status = sl_rail_get_rx_packet_details(railHandle, packetHandle, &details);
  if (status == SL_RAIL_STATUS_NO_ERROR) {
    if (details.sub_phy_id < SUBPHYID_COUNT) {
      counters.subPhyCount[details.sub_phy_id]++;
    }
    // Note that this does not take into account CRC bytes unless
    // SL_RAIL_RX_OPTION_STORE_CRC is used
    details.time_received.total_packet_bytes = length;
    (void) (*rxTimePosition)(railHandle, &details);

    if (phySwitchToRx.iterations > 0) {
      // Be careful when setting timeDelta, because it's the delta from
      // whatever timestamp position is currently configured
      uint32_t timestamp = details.time_received.packet_time;
      if (phySwitchToRx.extraDelayUs > 0) {
        sl_rail_delay_us(railHandle, phySwitchToRx.extraDelayUs);
      }
      (void) sl_rail_ble_phy_switch_to_rx(railHandle,
                                          phySwitchToRx.phy,
                                          phySwitchToRx.physicalChannel,
                                          phySwitchToRx.timeDelta + timestamp,
                                          &phySwitchToRx.params);
      phySwitchToRx.iterations--;
    }
  } else {
    memset(&details, 0, sizeof(details));
  }

  RailAppEvent_t *rxPacket
    = (RailAppEvent_t *)sl_malloc(sizeof(RailAppEvent_t) + length);

  // Count packets that we received but had no memory to store
  if (rxPacket == NULL) {
    counters.noRxBuffer++;
  } else {
    uint8_t *rxPacketData = (uint8_t *)&rxPacket[1]; // Point beyond RailAppEvent_t
    rxPacket->type = RX_PACKET;
    rxPacket->rxPacket.dataPtr = rxPacketData;
    rxPacket->rxPacket.packetStatus = packetInfo.packet_status;
    // Read packet data into our packet structure
    sl_rail_copy_rx_packet(railHandle, rxPacketData, &packetInfo);
    rxPacket->rxPacket.filterMask = packetInfo.filter_mask;
    rxPacket->rxPacket.dataLength = length;
    rxPacket->rxPacket.freqOffset = getRxFreqOffset();
    rxPacket->rxPacket.appendedInfo = details;
    rxPacket->rxPacket.railHandle = railHandle;
    rxPacket->rxPacket.packetHeld = heldPacket;

    // If we have just received an ACK, don't respond with an ACK
    if (rxPacketData[2] == 0xF1) {
      sl_rail_cancel_auto_ack(railHandle);
    }

    // Cancel ack if user requested
    if (afterRxCancelAck) {
      afterRxCancelAck = false;
      sl_rail_cancel_auto_ack(railHandle);
    }

    // Use Tx Buffer for Ack if user requested
    if (afterRxUseTxBufferForAck) {
      afterRxUseTxBufferForAck = false;
      sl_rail_use_tx_fifo_for_auto_ack(railHandle);
    }

    if (currentAppMode() == SCHTX_AFTER_RX) {
      // Schedule the next transmit after this receive
      sl_rail_scheduled_tx_config_t scheduledTxOptions = {
        //TODO: packetTime depends on rxTimePosition;
        //      this code assumes default position (SYNC_END).
        .when = rxPacket->rxPacket.appendedInfo.time_received.packet_time
                + txAfterRxDelay,
        .mode = SL_RAIL_TIME_ABSOLUTE,
        .tx_during_rx = SL_RAIL_SCHEDULED_TX_DURING_RX_POSTPONE_TX
      };

      setNextPacketTime(&scheduledTxOptions);

      txCount = 1;
      pendPacketTx();
    }

    updateStats(rxPacket->rxPacket.appendedInfo.rssi_dbm, &counters.rssi);
    if (logLevel & ASYNC_RESPONSE) {
      updateGraphics();
      // Copy this received packet into our circular queue
      queueAdd(&railAppEventQueue, (void *)rxPacket);
    } else {
      sl_free((void *)rxPacket);
    }
  }

  // In Rx overflow test mode hang in this ISR to prevent processing new
  // packets to force an overflow
  if ((currentAppMode() == RX_OVERFLOW)) {
    enableAppMode(RX_OVERFLOW, false, NULL); // Switch back after the overflow
    changeAppModeIfPending();
    // Trigger an overflow by waiting in the interrupt handler
    usDelay(rxOverflowDelay);
  }

  return packetHandle;
}

// Only support fixed length
static void fifoMode_RxPacketReceived(void)
{
  uint16_t bytesRead;
  // Discard incoming data stream in BER mode.
  if (currentAppMode() == BER) {
    sl_rail_reset_fifo(railHandle, true, true);
  } else {
    // Note the packet's status
    sl_rail_rx_packet_info_t packetInfo;
    sl_rail_rx_packet_handle_t packetHandle
      = sl_rail_get_rx_packet_info(railHandle, SL_RAIL_RX_PACKET_HANDLE_OLDEST,
                                   &packetInfo);
    // assert(packetHandle != NULL);
    if (packetInfo.packet_status == SL_RAIL_RX_PACKET_READY_CRC_ERROR) {
      counters.receiveCrcErrDrop++; // counters.receive still counts such too
    }
    if (printRxErrorPackets
        || (packetInfo.packet_status == SL_RAIL_RX_PACKET_READY_CRC_ERROR)
        || (packetInfo.packet_status == SL_RAIL_RX_PACKET_READY_SUCCESS)) {
      // Read the rest of the bytes out of the fifo
      bytesRead = sl_rail_read_rx_fifo(railHandle, currentRxFifoPacketPtr, rxLengthCount);
      rxLengthCount -= bytesRead;
      if (currentRxFifoPacketPtr != NULL) {
        currentRxFifoPacketPtr += bytesRead;
      }

      if (rxFifoPacketData != NULL) {
        // Keep and display this frame
        rxFifoPacketData->rxPacket.packetStatus = packetInfo.packet_status;
        rxFifoPacketData->rxPacket.filterMask = packetInfo.filter_mask;

        // Configure how many bytes were received
        rxFifoPacketData->rxPacket.dataLength = rxLengthTarget;

        // Get the appended info details
        if (sl_rail_get_rx_packet_details(railHandle, packetHandle,
                                          &rxFifoPacketData->rxPacket.appendedInfo)
            != SL_RAIL_STATUS_NO_ERROR) {
          // assert(false);
          memset(&rxFifoPacketData->rxPacket.appendedInfo, 0, sizeof(rxFifoPacketData->rxPacket.appendedInfo));
        } else {
          if (rxFifoPacketData->rxPacket.appendedInfo.sub_phy_id < SL_RAIL_BLE_RX_SUBPHY_COUNT) {
            counters.subPhyCount[rxFifoPacketData->rxPacket.appendedInfo.sub_phy_id]++;
          }
        }
        // Note that this does not take into account CRC bytes unless
        // SL_RAIL_RX_OPTION_STORE_CRC is used
        rxFifoPacketData->rxPacket.appendedInfo.time_received.total_packet_bytes = rxLengthTarget;
        sl_rail_get_rx_time_sync_word_end(railHandle, &rxFifoPacketData->rxPacket.appendedInfo);
        queueAdd(&railAppEventQueue, (void *)rxFifoPacketData);
      }
    } else {
      // Toss this frame and any of its data accumlated so far
      sl_free((void *)rxFifoPacketData);
    }
  }
}

/**
 * In Fifo mode, prepare for the upcoming rx
 *
 * Grab a buffer to store rx data
 * Keep track of writing data to this buffer
 */
void rxFifoPrep(void)
{
  // Don't allocate memory to save incoming data in BER mode
  // or non-packet-data sources
  if ((railRxDataConfig.rx_method == SL_RAIL_DATA_METHOD_FIFO_MODE)
      && (railRxDataConfig.rx_source == SL_RAIL_RX_DATA_SOURCE_PACKET_DATA)
      && (currentAppMode() != BER)
      && (rxLengthTarget != SL_RAIL_SET_FIXED_LENGTH_INVALID)
      && !rxFifoManual) {
    rxLengthCount = rxLengthTarget;
    rxFifoPacketData
      = (RailAppEvent_t *)sl_malloc(sizeof(RailAppEvent_t) + rxLengthTarget);
    if (rxFifoPacketData == NULL) {
      currentRxFifoPacketPtr = NULL;
      counters.noRxBuffer++;
    } else {
      uint8_t *rxPacketData = (uint8_t *)&rxFifoPacketData[1]; // Point beyond RailAppEvent_t
      rxFifoPacketData->type = RX_PACKET;
      rxFifoPacketData->rxPacket.dataPtr = rxPacketData;
      rxFifoPacketData->rxPacket.freqOffset = getRxFreqOffset();
      rxFifoPacketData->rxPacket.filterMask = 0U;
      rxFifoPacketData->rxPacket.packetHeld = false;
      currentRxFifoPacketPtr = rxPacketData;
    }
  }
}

/******************************************************************************
 * Public
 *****************************************************************************/
void loadTxData(uint8_t *data, uint16_t dataLen)
{
  setTxRandomHelper();
  uint16_t dataWritten;

  if (railTxDataConfig.tx_method == SL_RAIL_DATA_METHOD_PACKET_MODE) {
    if (offsetInTxFifo == 0U) {
      sl_rail_write_tx_fifo(railHandle, data, dataLen, true);
    } else {
      const sl_rail_config_t *config = sl_rail_get_config(railHandle);
      uint16_t fifo_bytes = config->tx_fifo_bytes;
      if (offsetInTxFifo >= fifo_bytes) {
        return; // Cannot write anyting
      }
      if ((dataLen + offsetInTxFifo) > fifo_bytes) {
        dataLen = fifo_bytes - offsetInTxFifo;
      }
      // Copy prepared data buffer directly in TxFifo regardless of alignment
      // Need to use a uint8_t pointer instead of a uint32_t pointer as memcpy parameter
      // to prevent from incrementing destination address by 4 bytes on each offsetInTxFifo step.
      uint8_t *p_byte = (uint8_t *)(config->p_tx_fifo_buffer);
      (void)memcpy(&p_byte[offsetInTxFifo], data, dataLen);
      // Insert dummy 'x' char before Tx Data for testing purpose
      (void)memset(config->p_tx_fifo_buffer, 'x', offsetInTxFifo);
      (void)sl_rail_set_tx_fifo(railHandle, config->p_tx_fifo_buffer, fifo_bytes,
                                dataLen, offsetInTxFifo);
    }
  } else {
    dataWritten = sl_rail_write_tx_fifo(railHandle, data, dataLen, false);
    dataLeft = dataLen - dataWritten;
    dataLeftPtr = data + dataWritten;
  }
}

void configRxLengthSetting(uint16_t rxLength)
{
  if (railRxDataConfig.rx_method == SL_RAIL_DATA_METHOD_FIFO_MODE) {
    rxLengthTarget = rxLength;
  }
}

/******************************************************************************
 * RAIL Callback Implementation
 *****************************************************************************/
void railtest_TxPacketSent(sl_rail_handle_t railHandle, bool isAck)
{
#ifdef SL_CATALOG_CS_CLI_PRESENT
  railappcb_TxPacketSentCs(railHandle);
#endif
#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH && defined(WISUN_MODESWITCHPHRS_ARRAY_SIZE)
  if ((modeSwitchState == TX_MS_PACKET) || (modeSwitchState == TX_ON_NEW_PHY)) { // Packet has been sent in a MS context
    scheduleNextModeSwitchTx(true);
  } else
#endif
  {
    // Store the previous tx time for printing later
    if (isAck) {
      sentAckPackets++;
      // previousTxAckAppendedInfo.isAck already initialized true
      previousTxAckAppendedInfo.time_sent.total_packet_bytes
        = sl_rail_ieee802154_is_enabled(railHandle) ? 4U : ackDataLen;
      (void) sl_rail_get_tx_packet_details(railHandle, &previousTxAckAppendedInfo);
      (void) (*txTimePosition)(railHandle, &previousTxAckAppendedInfo);
      pendFinishTxAckSequence();
    } else {
      internalTransmitCounter++;
      // previousTxAppendedInfo.isAck already initialized false
      previousTxAppendedInfo.time_sent.total_packet_bytes = txDataLen;
      (void) sl_rail_get_tx_packet_details(railHandle, &previousTxAppendedInfo);
      (void) (*txTimePosition)(railHandle, &previousTxAppendedInfo);
      scheduleNextTx();
    }
  }

  // Move visualization update in order not to delay mode switching
  LedToggle(1);
  updateGraphics();
}

void railtest_RxPacketAborted(sl_rail_handle_t railHandle)
{
  if (railRxDataConfig.rx_method == SL_RAIL_DATA_METHOD_PACKET_MODE) {
    packetMode_RxPacketAborted(railHandle);
  } else if (!rxFifoManual) {
    if (rxLengthTarget == SL_RAIL_SET_FIXED_LENGTH_INVALID) {
      packetMode_RxPacketAborted(railHandle);
    } else {
      fifoMode_RxPacketReceived();
    }
  }
}

void railtest_RxTimeout(sl_rail_handle_t railHandle)
{
#ifdef SL_CATALOG_CS_CLI_PRESENT
  railappcb_RxTimeoutCs(railHandle);
#else
  (void)railHandle;
#endif
}

void railtest_RxBlocked(sl_rail_handle_t railHandle)
{
#ifdef SL_CATALOG_CS_CLI_PRESENT
  railappcb_RxTimeoutCs(railHandle);
#else
  (void)railHandle;
#endif
}

void railtest_RxPacketReceived(sl_rail_handle_t railHandle)
{
#ifdef SL_CATALOG_CS_CLI_PRESENT
  railappcb_RxPacketReceivedCs(railHandle);
#endif

  counters.receive++;
  LedToggle(0);
  rxPacketEvent = true;

  if (railRxDataConfig.rx_method == SL_RAIL_DATA_METHOD_PACKET_MODE) {
    packetMode_RxPacketReceived(railHandle);
  } else if (!rxFifoManual) {
    if (rxLengthTarget == SL_RAIL_SET_FIXED_LENGTH_INVALID) {
      packetMode_RxPacketAborted(railHandle);
    } else {
      fifoMode_RxPacketReceived();
    }
  }
}

void railtest_RxChannelHoppingComplete(sl_rail_handle_t railHandle)
{
  sl_rail_time_t periodicWakeupUs = 0U;

  // Only schedule the next wakeup if:
  // 1. RX Duty Cycle Schedule Wakeup is enabled
  // 2. Sleep duration > 0
  // 3. Sleep has not been interrupted via a serial wakeup.
  if ((getRxDutyCycleSchedWakeupEnable(&periodicWakeupUs))
      && (periodicWakeupUs > 0U) && (!serEvent)) {
    // Configure the next scheduled RX
    sl_rail_scheduled_rx_config_t rxCfg = {
      .start = periodicWakeupUs,
      .start_mode = SL_RAIL_TIME_DELAY,
      .end = 0U,
      .end_mode = SL_RAIL_TIME_DISABLED,
      .rx_transition_end_schedule = 0U,
      .hard_window_end = 0U
    };

    sl_rail_idle(railHandle, SL_RAIL_IDLE_ABORT, true);
    (void) sl_rail_start_scheduled_rx(railHandle, channel, &rxCfg, NULL);
    allowPowerManagerSleep = true;
  }
}

void railtest_TxFifoAlmostEmpty(sl_rail_handle_t railHandle)
{
  uint16_t dataWritten;
  counters.txFifoAlmostEmpty++;

  if (dataLeft) {
    dataWritten = sl_rail_write_tx_fifo(railHandle, dataLeftPtr, dataLeft, false);
    dataLeft -= dataWritten;
    dataLeftPtr += dataWritten;
  }
}

// count number of 1s in a byte without a loop
static uint8_t countBits(uint8_t num)
{
  uint8_t count = 0;
  static const uint8_t nibblebits[] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
  count += nibblebits[num & 0x0F];
  count += nibblebits[num >> 4];
  return count;
}

static void berSource_RxFifoAlmostFull(uint16_t bytesAvailable)
{
  (void)bytesAvailable;
  // All incoming bytes are received and validated here.
  uint16_t numBytes;
  bool stopBerRx = false;

  // If rxOfEvent is > 0, then we're overflowing the incoming RX buffer
  // probably because the BER test isn't processing incoming bits fast
  // enough. The test will automatically try to re-synchronize and read in bits
  // from the stream, but the bits under test will not be continuous. Abort
  // testing and notify the user if this is the case.
  if (counters.rxOfEvent) {
    stopBerRx = true;
  }

  while ((sl_rail_get_rx_fifo_bytes_available(railHandle) > sl_rail_get_rx_fifo_threshold(railHandle))
         && !stopBerRx) {
    // Read multiple bytes in if they're available.
    // Reuse the txData[SL_RAIL_TEST_MAX_PACKET_LENGTH] array since we won't be
    // transmitting in BER Test mode anyway.
    numBytes = sl_rail_read_rx_fifo(railHandle, txData, SL_RAIL_TEST_MAX_PACKET_LENGTH);

    for (uint16_t x = 0; x < numBytes && !stopBerRx; x++) {
      // Update BER statistics
      if (berStats.bytesTested < berStats.bytesTotal) {
        // Counters will not overflow since bytesTotal max value is capped.
        berStats.bitErrors += countBits(txData[x]);
        berStats.bytesTested++;
      } else {
        stopBerRx = true; // statistics are all gathered - stop now
      }
    }
  }
  // disregard decimal point
  berStats.rssi = (int8_t)(sl_rail_get_rssi(railHandle, SL_RAIL_GET_RSSI_WAIT_WITHOUT_TIMEOUT) / 4);

  // stop RXing when enough bits are acquired or an error (i.e. RX overflow)
  if (stopBerRx) {
    sl_rail_idle(railHandle, SL_RAIL_IDLE_FORCE_SHUTDOWN, true);
    sl_rail_reset_fifo(railHandle, true, true);
    berTestModeEnabled = false;
  }
}

// @TODO It would be better if we could 'register' callback contents
static void packetSource_RxFifoAlmostFull(uint16_t bytesAvailable)
{
  uint16_t bytesRead;
  if (rxLengthCount > 0) {
    // Amount to read is either bytes avilable or number of bytes remaining in packet
    bytesRead = (rxLengthCount > bytesAvailable) ? bytesAvailable : rxLengthCount;
    bytesRead = sl_rail_read_rx_fifo(railHandle, currentRxFifoPacketPtr, bytesRead);
    rxLengthCount -= bytesRead;
    if (currentRxFifoPacketPtr != NULL) {
      currentRxFifoPacketPtr += bytesRead;
    }
  }
}

bool reproFifoAlmostFullBug = false;

void railtest_RxFifoAlmostFull(sl_rail_handle_t railHandle)
{
  uint16_t bytesAvailable = sl_rail_get_rx_fifo_bytes_available(railHandle);

  if (berTestModeEnabled) {
    berSource_RxFifoAlmostFull(bytesAvailable);
  } else if (sl_rail_ble_is_enabled(railHandle) || reproFifoAlmostFullBug) {
    sl_rail_config_events(railHandle, SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL,
                          SL_RAIL_EVENTS_NONE); // Disable this event
  } else if (railRxDataConfig.rx_source != SL_RAIL_RX_DATA_SOURCE_PACKET_DATA) {
    // Just consume (flush) the data
    counters.rxRawSourceBytes += sl_rail_read_rx_fifo(railHandle, NULL, bytesAvailable);
    rxDataSourceEventState++;
    if (rxDataSourceEventState >= RX_DATA_SOURCE_EVENT_STATE_SUSPENDED) {
      // Temporarily disable RX_FIFO events to let main-line code run
      // in case these events are so frequent nothing gets done.
      sl_rail_config_events(railHandle, SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL,
                            SL_RAIL_EVENTS_NONE);
      // Separate calls each of which uses fast-path code in library
      sl_rail_config_events(railHandle, (SL_RAIL_EVENT_RX_FIFO_OVERFLOW
                                         | SL_RAIL_EVENT_RX_FIFO_FULL),
                            SL_RAIL_EVENTS_NONE);
    }
  } else {
    packetSource_RxFifoAlmostFull(bytesAvailable);
  }
}

void setNextPacketTime(sl_rail_scheduled_tx_config_t *scheduledTxOptions)
{
  if (scheduledTxOptions->mode == SL_RAIL_TIME_ABSOLUTE) {
    txScheduledTime = scheduledTxOptions->when;
  } else {
    txScheduledTime = 0U;
  }
  (void) memcpy(&nextPacketTxTime, scheduledTxOptions, sizeof(sl_rail_scheduled_tx_config_t));
}
