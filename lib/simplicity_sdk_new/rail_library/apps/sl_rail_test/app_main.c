/***************************************************************************//**
 * @file
 * @brief This is the base test application. It handles basic RAIL configuration
 *   as well as transmit, receive, and various debug modes.
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

#include "sl_rail_ieee802154.h"
#include "sl_rail_mfm.h"
#include "sl_rail_zwave.h"
#include "sl_rail_util_init.h"
#include "sl_rail_util_protocol.h"
#if defined(SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT)
  #include "sl_rail_util_ant_div.h"
#endif

#include "em_device.h"
#include "sl_core.h"
#ifdef _SILICON_LABS_32B_SERIES_2
#include "em_rmu.h"
#endif

#include "app_trx.h"

#ifdef RAILAPP_RMR
#include "railapp_rmr.h"
#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  #include "sl_power_manager.h"
#endif

#include "sl_rail_test_config.h"

#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT
#include "coexistence-802154.h"
#endif

#ifdef SL_CATALOG_TIMING_TEST_PRESENT
#include "sl_rail_util_timing_test.h"
#endif

#ifdef SL_CATALOG_CS_CLI_PRESENT
#include "railapp_cs.h"
#endif

// Includes for Silicon Labs-only, internal testing
#ifdef RPC_TESTING
#include "vs_rpc.h"
#else
#define RPC_Server_Init()
#define RPC_Server_Tick()
#endif

// Add a way to override the default setting for printingEnabled
#if defined(SL_RAIL_UTIL_PRINTING_DEFAULT) && !(SL_RAIL_UTIL_PRINTING_DEFAULT)
#define RAILTEST_PRINTING_DEFAULT_BOOL  false
#else
#define RAILTEST_PRINTING_DEFAULT_BOOL  true
#endif

// Add a way to override the default setting for skipCalibrations
#if defined(SL_RAIL_UTIL_SKIP_CALIBRATIONS_DEFAULT) && (SL_RAIL_UTIL_SKIP_CALIBRATIONS_DEFAULT)
#define RAILTEST_SKIP_CALIBRATIONS_BOOL true
#else
#define RAILTEST_SKIP_CALIBRATIONS_BOOL false
#endif

#ifdef SL_CATALOG_SL_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
extern void sl_rail_util_ieee801254_on_rail_event(sl_rail_handle_t railHandle, sl_rail_events_t events);
#endif // SL_CATALOG_SL_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT
extern void sl_bt_ll_coex_handle_events(sl_rail_events_t events);
#endif //SL_CATALOG_RAIL_UTIL_COEX_PRESENT

#ifndef SL_BOARD_NAME
#define SL_BOARD_NAME "?"
#endif//SL_BOARD_NAME

// External control and status variables
Counters_t counters = { 0 };
bool receiveModeEnabled = false;
#if SL_RAIL_UTIL_INIT_INST0_ENABLE && SL_RAIL_UTIL_INIT_TRANSITIONS_INST0_ENABLE
sl_rail_radio_state_t rxSuccessTransition = SL_RAIL_UTIL_INIT_TRANSITION_INST0_RX_SUCCESS;
#else
sl_rail_radio_state_t rxSuccessTransition = SL_RAIL_RF_STATE_IDLE;
#endif
bool fillTxPacketOnRx = true;
uint8_t logLevel = PERIPHERAL_ENABLE | ASYNC_RESPONSE;
int32_t txCount = 0;
int32_t txRepeatCount = 0;
int32_t txRemainingCount = 0;
uint32_t continuousTransferPeriod = SL_RAIL_TEST_CONTINUOUS_TRANSFER_PERIOD;
bool enableRandomTxDelay = false;
uint32_t txAfterRxDelay = 0;
int32_t txCancelDelay = -1;
sl_rail_stop_mode_t txCancelMode = SL_RAIL_STOP_MODES_NONE; // Use sl_rail_idle()
bool skipCalibrations = RAILTEST_SKIP_CALIBRATIONS_BOOL;
bool afterRxCancelAck = false;
bool afterRxUseTxBufferForAck = false;
uint32_t rssiDoneCount = 0; // HW rssi averaging
float averageRssi = -128;
bool printTxAck = false;
const char buildDateTime[] = __DATE__ " " __TIME__;
bool rxHeld = false;
volatile bool rxProcessHeld = false;
volatile uint32_t packetsHeld = 0U;

// Internal app state variables
static uint32_t startTransmitCounter = 0;
uint32_t internalTransmitCounter = 0;
volatile uint32_t ccaSuccesses = 0;
volatile uint32_t sentAckPackets = 0;

// Globals accessed in an interrupt context declared volatile.
volatile sl_rail_events_t lastTxStatus = 0;
volatile sl_rail_events_t lastTxAckStatus = 0;
volatile uint32_t failPackets = 0;
volatile uint32_t failAckPackets = 0;
static volatile bool packetTx = false;
static volatile bool finishTxSequence = false;
static volatile bool finishTxAckSequence = false;

sl_rail_scheduled_tx_config_t nextPacketTxTime = {
  .when = 0,
  .mode = SL_RAIL_TIME_ABSOLUTE,
  .tx_during_rx = SL_RAIL_SCHEDULED_TX_DURING_RX_POSTPONE_TX
};
volatile uint32_t eventsMissed = 0U;
sl_rail_time_t txStartTime = 0U;
sl_rail_time_t txScheduledTime = 0U;

// Variable which holds the receive frequency offset for the period of time
// between when the frequency offset is measured (in either the
// SL_RAIL_EVENT_RX_SYNC0_DETECT event or the SL_RAIL_EVENT_RX_SYNC1_DETECT event)
// until reception of the packet is completed or aborted.
sl_rail_frequency_offset_t rxFreqOffset = SL_RAIL_FREQUENCY_OFFSET_INVALID;

volatile bool beamReceived = false;
static volatile uint32_t railTimerExpireTime = 0;
static volatile uint32_t railTimerConfigExpireTime = 0;
static volatile bool     railTimerExpired = false;
static volatile bool     calibrateRadio = false;
bool volatile newTxError = false;
static volatile bool     rxAckTimeout = false;
static volatile uint32_t ackTimeoutDuration = 0;
sl_rail_events_t enablePrintEvents = SL_RAIL_EVENTS_NONE;
bool printRxErrorPackets = false;
bool printRxFreqOffsetData = false;
bool printingEnabled = RAILTEST_PRINTING_DEFAULT_BOOL;

// Names of SL_RAIL_EVENT defines. This should align with sl_rail_types.h
const char * const eventNames[] = RAILTEST_EVENT_STRINGS;
const uint8_t numRailEvents = COUNTOF(eventNames);

// Channel Hopping configuration structures
#if SL_RAIL_SUPPORTS_CHANNEL_HOPPING
uint32_t channelHoppingBufferSpace[CHANNEL_HOPPING_BUFFER_WORDS];
uint32_t *channelHoppingBuffer = channelHoppingBufferSpace;
#endif

#ifdef SL_CATALOG_TIMING_TEST_PRESENT
extern volatile bool enableRxPacketEventTimeCapture;
#endif

// Channel Variable
uint16_t channel = 0;

// Generic
sl_rail_handle_t railHandle = SL_RAIL_EFR32_HANDLE;

// Make default packet work with the length-decoding schemes of 802.15.4, BLE,
// and Z-Wave
uint8_t txData[SL_RAIL_TEST_MAX_PACKET_LENGTH] = {
  0x0F, 0x0E, 0x11, 0x22, 0x33, 0x44, 0x55, 0x0F,
  0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE,
};
uint16_t txDataLen = 16;
uint16_t offsetInTxFifo = 0;

uint8_t ackData[SL_RAIL_DEFAULT_AUTO_ACK_FIFO_BYTES] = {
  0x0F, 0x0E, 0xF1, 0xE2, 0xD3, 0xC4, 0xB5, 0x0F,
  0x97, 0x88, 0x79, 0x6A, 0x5B, 0x4C, 0x3D, 0x2E,
};
uint8_t ackDataLen = 16;

// Static RAIL callbacks
static void railtest_RssiAverageDone(sl_rail_handle_t railHandle);

// Structures that hold default TX & RX Options
sl_rail_tx_options_t txOptions = SL_RAIL_TX_OPTIONS_DEFAULT;
sl_rail_rx_options_t rxOptions = SL_RAIL_RX_OPTIONS_DEFAULT;

// Data Management
sl_rail_tx_data_config_t railTxDataConfig = {
  .tx_source = SL_RAIL_UTIL_INIT_TX_DATA_FORMAT_INST0_SOURCE,
  .tx_method = SL_RAIL_UTIL_INIT_TX_DATA_FORMAT_INST0_MODE,
};
sl_rail_rx_data_config_t railRxDataConfig = {
  .rx_source = SL_RAIL_UTIL_INIT_RX_DATA_FORMAT_INST0_SOURCE,
  .rx_method = SL_RAIL_UTIL_INIT_RX_DATA_FORMAT_INST0_MODE,
};

#if defined(SL_CATALOG_SL_RAIL_UTIL_THERMISTOR_PRESENT)
sl_rail_hfxo_compensation_config_t compensationConfig = {
  .enable_compensation = false,
  .zone_temperature_celsius = 85,
  .delta_nominal_celsius = 15,
  .delta_critical_celsius = 5
};
#endif

// Called during main loop
void sendPacketIfPending(void);
void finishTxSequenceIfPending(void);
void changeAppModeIfPending(void);
void printNewTxError(void);
void checkTimerExpiration(void);
void processPendingCalibrations(void);
void printAckTimeout(void);

static void heldRxProcess(void)
{
  if (!rxProcessHeld) {
    return;
  }

  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  uint32_t packetsHeldLocal = packetsHeld;
  if (packetsHeldLocal == 0) {
    rxProcessHeld = false;
  }
  sl_rail_rx_packet_handle_t packetHandle
    = processRxPacket(railHandle, SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, true);
  CORE_EXIT_CRITICAL();

  if (packetHandle == SL_RAIL_RX_PACKET_HANDLE_INVALID) {
    if (packetsHeldLocal > 0) {
      responsePrint("heldRxProcess",
                    "heldPkts:%u,error:unexpected invalid packetHandle",
                    packetsHeldLocal);
      // Recover from error
      CORE_ENTER_CRITICAL();
      packetsHeld -= packetsHeldLocal;
      CORE_EXIT_CRITICAL();
    }
  } else {
    sl_rail_release_rx_packet(railHandle, packetHandle);
    CORE_ENTER_CRITICAL();
    packetsHeld--;
    CORE_EXIT_CRITICAL();
    if (packetsHeldLocal == 0) {
      responsePrint("heldRxProcess",
                    "heldPkts:0,error:unexpected valid packetHandle,");
    }
  }
}

// Copy src bytes into txData array starting at offset, and update txDataLength.
static void changeTxPayload(uint32_t offset,
                            uint8_t *src,
                            uint16_t srcLen,
                            uint16_t pktLen)
{
  if (offset + srcLen <= sizeof(txData)) {   // be sure it fits
    (void) memcpy(&txData[offset], &src[0], srcLen);
    txDataLen = pktLen;
  } else {
    responsePrintError("changeTxPayload",
                       0xF0,
                       "New packet data overflows the txData buffer size.");
    while (1) ;
  }
}

#ifdef _SILICON_LABS_32B_SERIES_2
#define sl_hal_emu_get_reset_cause RMU_ResetCauseGet
#define sl_hal_emu_clear_reset_cause RMU_ResetCauseClear
#define sl_power_manager_em4_unlatch_pin_retention EMU_UnlatchPinRetention
#endif

#if defined(_SILICON_LABS_IP_PROJ_IS_LPWH74000) || (_SILICON_LABS_32B_SERIES_3_CONFIG == 353)
// Not 100% sure why this works on no 74000 builds.  It has to do with us forcing SL_POWER_MANAGER_DEBUG to 1 in
// rail_power_manager.c, but not having SL_POWER_MANAGER_DEBUG set to 1 in the fpga power_manager component.  Maybe.
#undef sli_power_manager_debug_log_em_requirement
static inline void sli_power_manager_debug_log_em_requirement(sl_power_manager_em_t em,
                                                              bool                  add,
                                                              const char            *name)
{
  (void) em;
  (void) add;
  (void) name;
}
#endif

// Function called from sl_main_init before the main super loop.
void sl_rail_test_internal_app_init(void)
{
  // Get RAIL handle.
  railHandle = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);

  // Get proprietary channel config index, if support enabled.
  if (SL_RAIL_UTIL_INIT_RADIO_CONFIG_SUPPORT_INST0_ENABLE) {
    configIndex = SL_RAIL_UTIL_INIT_PROTOCOL_PROPRIETARY_INST0_INDEX;
  }

  // Initialize the RPC Server
  RPC_Server_Init();

  // Grab the reset cause
  uint32_t resetCause = sl_hal_emu_get_reset_cause();
  sl_hal_emu_clear_reset_cause(); // So resetCause is rational and not an accumulated mess
  // Release GPIOs that were held by EM4h to ensure proper startup
  sl_power_manager_em4_unlatch_pin_retention();

  // Initialize hardware for application
  appHalInit();

  // Initialize txOptions & rxOptions
#if defined(SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT)
  sl_rail_util_ant_div_init_tx_options(&txOptions);
  sl_rail_util_ant_div_init_rx_options(&rxOptions);
#endif

  // Make sure the response printer mirrors the default printingEnabled state
  responsePrintEnable(printingEnabled);
  // Print app initialization information.
  RAILTEST_PRINTF("\n");
  responsePrint("reset", "App:%s,Built:%s,Cause:0x%x,Brd:%s",
                SL_RAIL_TEST_APP_NAME, buildDateTime, resetCause, SL_BOARD_NAME);
  printChipInfo();
  getPti(NULL);

  (void) sl_rail_get_channel(railHandle, &channel);

  sl_rail_config_rx_options(railHandle, SL_RAIL_RX_OPTIONS_ALL, rxOptions);

#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9))
  if (resetCause & EMU_RSTCAUSE_EM4) {
    responsePrint("sleepWoke", "EM:4s,SerialWakeup:No,RfSensed:%s",
                  sl_rail_is_rf_sensed(railHandle) ? "Yes" : "No");
    // Always turn off RfSense when waking back up from EM4
    (void) sl_rail_start_rf_sense(railHandle, SL_RAIL_RF_SENSE_OFF, 0, NULL);
  }
#else
  (void) resetCause;
#endif

  // Fill out the default TX packet with a useful pattern so it's not
  // all zeros if user extends the TX length.
  for (unsigned int i = txDataLen; i < sizeof(txData); i++) {
    txData[i] = i;
  }

#if defined(SL_CATALOG_SL_RAIL_UTIL_THERMISTOR_PRESENT)
  sl_rail_config_hfxo_compensation(railHandle, &compensationConfig);
#endif

  // Change the default TX packet payload to be protocol-specific.
  if (SL_RAIL_UTIL_PROTOCOL_IS_IEEE802154_2G4(SL_RAIL_UTIL_INIT_PROTOCOL_INST0_DEFAULT)) {
    // Default 802.15.4 2.4 GHz packet.
    uint8_t changes[] = { 0x0c, 0x63, 0x88, 0xbe, 0xff, 0xff, 0xff, 0xff,
                          0x11, 0x22, 0x04 };
    // Change 11 bytes at offset 0 of txData, and new packet length is 11.
    changeTxPayload(0, &changes[0], sizeof(changes), 11);
  } else if (SL_RAIL_UTIL_PROTOCOL_IS_IEEE802154_GB868(SL_RAIL_UTIL_INIT_PROTOCOL_INST0_DEFAULT)) {
    // Default 802.15.4 2.4 GB868 packet.
    uint8_t changes[] = { 0x18, 0x28 };
    // Change 2 bytes at offset 0 of txData, and new packet length is 20.
    changeTxPayload(0, &changes[0], sizeof(changes), 20);
  } else if (SL_RAIL_UTIL_PROTOCOL_IS_ZWAVE(SL_RAIL_UTIL_INIT_PROTOCOL_INST0_DEFAULT)) {
    // Default Z-Wave packet.
    uint8_t changes[] = { 0x20 };
    // Change 1 byte at offset 7 of txData, and new packet length is 200.
    changeTxPayload(7, &changes[0], sizeof(changes), 200);
  } else {
    // Use the default packet that already has length-decoding schemes for
    // 802.15.4, BLE, and Z-Wave.
  }

  // Record initial host-side TX FIFO
  setTxFifo(NULL);
  // Initialize autoack data
  sl_rail_write_auto_ack_fifo(railHandle, ackData, ackDataLen);

  // RX isn't validated yet so lets not go into receive just yet
  sl_rail_start_rx(railHandle, channel, NULL); // Start in receive mode
  receiveModeEnabled = true;

  unsigned int seed;
  (void) sl_rail_get_radio_entropy(railHandle, (uint8_t *) &seed, sizeof(seed));
  srand(seed);
}

volatile uint16_t rxDataSourceEventState = RX_DATA_SOURCE_EVENT_STATE_CHECKED;

static void checkRxDataSource(void)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  if (rxDataSourceEventState >= RX_DATA_SOURCE_EVENT_STATE_SUSPENDED) {
    sl_rail_config_events(railHandle, SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL,
                          SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL);
    // Separate calls each of which uses fast-path code in library
    sl_rail_config_events(railHandle, (SL_RAIL_EVENT_RX_FIFO_OVERFLOW
                                       | SL_RAIL_EVENT_RX_FIFO_FULL),
                          (SL_RAIL_EVENT_RX_FIFO_OVERFLOW | SL_RAIL_EVENT_RX_FIFO_FULL));
  }
  rxDataSourceEventState = RX_DATA_SOURCE_EVENT_STATE_CHECKED;
  CORE_EXIT_CRITICAL();
}

static volatile sl_rail_status_t abortRxDelayStatus;

static void printAbortRxDelayStatus(void)
{
  if (abortRxDelayStatus != SL_RAIL_STATUS_NO_ERROR) {
    responsePrint("abortRxDelay", "Failed:%s",
                  getStatusMessage(abortRxDelayStatus));
    abortRxDelayStatus = SL_RAIL_STATUS_NO_ERROR;
  }
}

// Function called from sl_main_process_action within the main super loop.
void sl_rail_test_internal_app_process_action(void)
{
  RPC_Server_Tick();

  // Change app mode first so that any new actions can take effect this loop
  changeAppModeIfPending();

  rfSensedCheck();

  sendPacketIfPending();

  printNewTxError();

  finishTxSequenceIfPending();

  printRailAppEvents();

  checkTimerExpiration();

  processPendingCalibrations();

  printAckTimeout();

  heldRxProcess();

  checkRxDataSource();

  printAbortRxDelayStatus();
}

/******************************************************************************
 * RAIL Callback Implementation
 *****************************************************************************/
// Override weak function called by callback sli_rail_util_on_rf_ready.
void sl_rail_util_on_rf_ready(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  LedSet(0);
  LedSet(1);
}

void railtest_SwTimerExpired(sl_rail_handle_t railHandle)
{
  railTimerExpireTime = sl_rail_get_time(railHandle);
  railTimerConfigExpireTime = sl_rail_get_timer(railHandle);
  railTimerExpired = true;
}

#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH && defined(WISUN_MODESWITCHPHRS_ARRAY_SIZE)
void railtest_ModeSwitchMultiTimerExpired(sl_rail_multi_timer_t *tmr,
                                          sl_rail_time_t expectedTimeOfEvent,
                                          void *cbArg)
{
  (void)tmr;
  (void)expectedTimeOfEvent;
  (void)cbArg;

  if ((modeSwitchState == TX_MS_PACKET) || (modeSwitchState == TX_ON_NEW_PHY)) {
    restartModeSwitchSequence(false);
  }
  if (modeSwitchState == RX_ON_NEW_PHY) {
    if (sl_rail_is_valid_channel(railHandle, modeSwitchBaseChannel)
        == SL_RAIL_STATUS_NO_ERROR) {
      changeChannel(modeSwitchBaseChannel);
      modeSwitchBaseChannel = 0xFFFFU;
      modeSwitchNewChannel = 0xFFFFU;
    }
  }
}

void restartModeSwitchSequence(bool applyDelay)
{
  txCountAfterModeSwitchId = 0;
  // Start timer if needed
  if ((modeSwitchDelayUs > 0) && applyDelay) {
    sl_rail_set_multi_timer(railHandle,
                            &modeSwitchMultiTimer,
                            modeSwitchDelayUs,
                            SL_RAIL_TIME_DELAY,
                            &railtest_ModeSwitchMultiTimerExpired,
                            NULL);
  } else {
    // Re-start sequence: switch back on base channel
    if (sl_rail_is_valid_channel(railHandle, modeSwitchBaseChannel)
        == SL_RAIL_STATUS_NO_ERROR) {
      changeChannel(modeSwitchBaseChannel);
      // Write MS PHR in txData
      txData[0] = MSphr[0];
      txData[1] = MSphr[1];
      modeSwitchState = TX_MS_PACKET;
      // Send MS packet
      txCount = 1;
      pendPacketTx();
      sendPacketIfPending(); // txCount is decremented in this function
    }
  }
}

void endModeSwitchSequence(void)
{
  if (modeSwitchLifeReturn) {
    changeChannel(modeSwitchBaseChannel);
  } else {
    modeSwitchState = TX_STAY_ON_NEW_PHY;
  }
  modeSwitchDelayUs = 0;
  modeSwitchBaseChannel = 0xFFFFU;
  modeSwitchNewChannel = 0xFFFFU;
  setNextAppMode(NONE, NULL);
}

void scheduleNextModeSwitchTx(bool MSPktSent)
{
  if (MSPktSent) {
    internalTransmitCounter++;
    // previousTxAppendedInfo.isAck already initialized false
    previousTxAppendedInfo.time_sent.total_packet_bytes = txDataLen;
    (void) sl_rail_get_tx_packet_details(railHandle, &previousTxAppendedInfo);
    (void) (*txTimePosition)(railHandle, &previousTxAppendedInfo);
  }
  if (txCountAfterModeSwitchId == 0) {     // Scheduled packet was MS packet
    if (MSPktSent && (sl_rail_is_valid_channel(railHandle, modeSwitchNewChannel)
                      == SL_RAIL_STATUS_NO_ERROR)) {
      changeChannel(modeSwitchNewChannel);
      modeSwitchState = TX_ON_NEW_PHY;
    }

    // Restore first 2 bytes overwritten by Mode Switch PHR
    txData[0] = txData_2B[0];
    txData[1] = txData_2B[1];

    if (MSPktSent && (txCountAfterModeSwitch != 0)) {
      // Send next packets asap
      txCount = txCountAfterModeSwitch;
      pendPacketTx();
      sendPacketIfPending();   // txCount is decremented in this function
      txCountAfterModeSwitchId++;
    } else {
      modeSwitchSequenceId++;
      if (modeSwitchSequenceId < modeSwitchSequenceIterations) {
        restartModeSwitchSequence(true);
      } else {
        endModeSwitchSequence();
      }
    }
  } else {
    if (txCountAfterModeSwitchId < txCountAfterModeSwitch) {   // Scheduled packet was not the last data packet to be tx
      txCountAfterModeSwitchId++;
      scheduleNextTx();
    } else {
      modeSwitchSequenceId++;
      if (modeSwitchSequenceId < modeSwitchSequenceIterations) {
        restartModeSwitchSequence(true);
      } else {
        endModeSwitchSequence();
      }
    }
  }
}
#endif

void railtest_TimerExpired(sl_rail_handle_t railHandle)
{
  if (inAppMode(NONE, NULL)) {
    if (abortRxDelay != 0) {
      if (abortRxDelay < 0) {
        abortRxDelayStatus = sl_rail_abort_rx_incoming_packet(railHandle);
      } else {
        abortRxDelayStatus = sl_rail_idle(railHandle, SL_RAIL_IDLE_ABORT, true);
      }
    } else {
      railTimerExpireTime = sl_rail_get_time(railHandle);
      railTimerConfigExpireTime = sl_rail_get_timer(railHandle);
      railTimerExpired = true;
    }
  } else if (currentAppMode() == PER) {
#if defined (SL_RAIL_TEST_PER_PORT) && defined(SL_RAIL_TEST_PER_PIN)
    sl_gpio_toggle_pin(&(sl_gpio_t){SL_RAIL_TEST_PER_PORT, SL_RAIL_TEST_PER_PIN });
    bool pinState;
    sl_gpio_get_pin_output(&(sl_gpio_t){SL_RAIL_TEST_PER_PORT, SL_RAIL_TEST_PER_PIN }, &pinState);
    counters.perTriggers += pinState;
    perCount -= pinState;
#endif // SL_RAIL_TEST_PER_PORT && SL_RAIL_TEST_PER_PIN
    if (perCount < 1) {
#if defined (SL_RAIL_TEST_PER_PORT) && defined(SL_RAIL_TEST_PER_PIN)
      sl_gpio_clear_pin(&(sl_gpio_t){SL_RAIL_TEST_PER_PORT, SL_RAIL_TEST_PER_PIN });
#endif // SL_RAIL_TEST_PER_PORT && SL_RAIL_TEST_PER_PIN
      enableAppMode(PER, false, NULL);
    } else {
      sl_rail_set_timer(railHandle, perDelay, SL_RAIL_TIME_DELAY, &railtest_TimerExpired);
    }
  } else {
    pendPacketTx();
  }
}

static void railtest_RssiAverageDone(sl_rail_handle_t railHandle)
{
  RailAppEvent_t *rssi
    = (RailAppEvent_t *)sl_malloc(sizeof(RailAppEvent_t));
  if (rssi == NULL) {
    eventsMissed++;
    return;
  }
  rssi->type = AVERAGE_RSSI;
  rssi->rssi.rssi = sl_rail_get_average_rssi(railHandle);
  queueAdd(&railAppEventQueue, (void *)rssi);
  rssiDoneCount++;
}

// Override weak function called by callback sl_rail_assert_failed.
void sl_rail_util_on_assert_failed(sl_rail_handle_t railHandle, uint32_t errorCode)
{
  (void)railHandle;
  static const char* railErrorMessages[] = SL_RAIL_ASSERT_ERROR_MESSAGES;
  const char *errorMessage = "Unknown";

  // If this error code is within the range of known error messages then use
  // the appropriate error message.
  if (errorCode < (sizeof(railErrorMessages) / sizeof(char*))) {
    errorMessage = railErrorMessages[errorCode];
  }
  // Print a message about the assert that triggered
  extern volatile int sli_rail_assert_line_number;
  responsePrint("assert",
                "code:%d,line:%d,message:%s",
                errorCode,
                sli_rail_assert_line_number,
                errorMessage);
  serialWaitForTxIdle();
  // Reset the chip since an assert is a fatal error
  NVIC_SystemReset();
}

// Override weak function called by callback sli_rail_util_on_event.
void sl_rail_util_on_event(sl_rail_handle_t railHandle, sl_rail_events_t events)
{
#ifdef SL_CATALOG_TIMING_TEST_PRESENT
  if (enableRxPacketEventTimeCapture
      && (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED)) {
    sl_rac_info_start.radioStateTimerTick = sl_rail_get_timer_tick(SL_RAIL_TIMER_TICK_DEFAULT);
    enableRxPacketEventTimeCapture = false;
  }
#endif //SL_CATALOG_TIMING_TEST_PRESENT
  enqueueEvents(events);
  if (events & SL_RAIL_EVENT_CAL_NEEDED) {
    calibrateRadio = true;
  }
  if (events & SL_RAIL_EVENT_RSSI_AVERAGE_DONE) {
    railtest_RssiAverageDone(railHandle);
  }

  // RX Events
  if (events & SL_RAIL_EVENT_RX_TIMING_DETECT) {
    counters.timingDetect++;
  }
  if (events & SL_RAIL_EVENT_RX_TIMING_LOST) {
    counters.timingLost++;
  }
  if (events & SL_RAIL_EVENT_RX_PREAMBLE_LOST) {
    counters.preambleLost++;
  }
  if (events & SL_RAIL_EVENT_RX_PREAMBLE_DETECT) {
    counters.preambleDetect++;
  }
  if (events & (SL_RAIL_EVENT_RX_SYNC_0_DETECT | SL_RAIL_EVENT_RX_SYNC_1_DETECT)) {
    counters.syncDetect++;
    if (events & SL_RAIL_EVENT_RX_SYNC_0_DETECT) {
      counters.syncDetect0++;
    }
    if (events & SL_RAIL_EVENT_RX_SYNC_1_DETECT) {
      counters.syncDetect1++;
    }
    rxFifoPrep();
    if (printRxFreqOffsetData) {
      rxFreqOffset = sl_rail_get_rx_freq_offset(railHandle);
    }
    if (abortRxDelay != 0) {
      uint32_t delay = (uint32_t)((abortRxDelay > 0) ? abortRxDelay : -abortRxDelay);
      sl_rail_set_timer(railHandle, delay, SL_RAIL_TIME_DELAY, &railtest_TimerExpired);
    }
  }
  if (events & SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND) {
    if (sl_rail_ieee802154_is_enabled(railHandle)) {
      counters.dataRequests++;
      railtest_IEEE802154_DataRequestCommand(railHandle);
    }
#if SL_RAIL_SUPPORTS_PROTOCOL_ZWAVE
    else if (sl_rail_zwave_is_enabled(railHandle)) {
      railtest_ZWAVE_LrAckData(railHandle);
    }
#endif //SL_RAIL_SUPPORTS_PROTOCOL_ZWAVE
    else {
      // Other protocols ignore this event
    }
  }
#if SL_RAIL_SUPPORTS_PROTOCOL_ZWAVE
  if (events & SL_RAIL_EVENT_ZWAVE_BEAM) {
    if (sl_rail_zwave_is_enabled(railHandle)) {
      counters.rxBeams++;
      railtest_ZWAVE_BeamFrame(railHandle);
    }
  }
#endif //SL_RAIL_SUPPORTS_PROTOCOL_ZWAVE
  if (events & SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL) {
    counters.rxFifoAlmostFull++;
    railtest_RxFifoAlmostFull(railHandle);
  }
  if (events & SL_RAIL_EVENT_RX_FIFO_FULL) {
    if (rxHeld) {
      rxProcessHeld = true; // Try to avoid overflow by processing held packets
    }
    counters.rxFifoFull++;
  }
  if (events & (SL_RAIL_EVENT_RX_FIFO_OVERFLOW
                | SL_RAIL_EVENT_RX_ADDRESS_FILTERED
                | SL_RAIL_EVENT_RX_PACKET_ABORTED
                | SL_RAIL_EVENT_RX_FRAME_ERROR
                | SL_RAIL_EVENT_RX_PACKET_RECEIVED)) {
    // All of the above events cause a packet to not be received
    if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
      railtest_RxPacketReceived(railHandle);
#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH && defined(WISUN_MODESWITCHPHRS_ARRAY_SIZE)
      if (modeSwitchState == RX_ON_NEW_PHY && modeSwitchLifeReturn) {
        sl_rail_set_multi_timer(railHandle,
                                &modeSwitchMultiTimer,
                                RX_MODE_SWITCH_DELAY_US,
                                SL_RAIL_TIME_DELAY,
                                &railtest_ModeSwitchMultiTimerExpired,
                                NULL);
      }
 #endif
    }
    if (rxFifoManual && (railRxDataConfig.rx_method != SL_RAIL_DATA_METHOD_PACKET_MODE)) {
      (void)sl_rail_hold_rx_packet(railHandle);
    }
    if (events & SL_RAIL_EVENT_RX_FIFO_OVERFLOW) {
      if (rxHeld) {
        rxProcessHeld = true; // Process held packets
      }
      counters.rxOfEvent++;
      if (railRxDataConfig.rx_source == SL_RAIL_RX_DATA_SOURCE_PACKET_DATA) {
        railtest_RxPacketAborted(railHandle);
      } else {
        // Treat similar to RX_FIFO_ALMOST_FULL: consume RX data
        railtest_RxFifoAlmostFull(railHandle);
        // Since we disable RX after a overflow, go ahead and
        // turn RX back on to continue collecting data.
        if (receiveModeEnabled) {
          sl_rail_start_rx(railHandle, channel, NULL);
        }
      }
    }
    if (events & SL_RAIL_EVENT_RX_ADDRESS_FILTERED) {
      counters.addrFilterEvent++;
      railtest_RxPacketAborted(railHandle);
    }
    if (events & SL_RAIL_EVENT_RX_PACKET_ABORTED) {
      counters.rxFail++;
      railtest_RxPacketAborted(railHandle);
    }
    if (events & SL_RAIL_EVENT_RX_FRAME_ERROR) {
      counters.frameError++;
      railtest_RxPacketAborted(railHandle);
    }
  }
  if (events & SL_RAIL_EVENT_RX_BLOCKED) {
    counters.rxBlocked++;
    railtest_RxBlocked(railHandle);
  }
  if (events & SL_RAIL_EVENT_RX_TIMEOUT) {
    counters.rxTimeout++;
    railtest_RxTimeout(railHandle);
  }
  if (events & SL_RAIL_EVENT_RX_ACK_TIMEOUT) {
    counters.ackTimeout++;
    rxAckTimeout = true;
    //TODO: packetTime depends on txTimePosition;
    //      this code assumes default position (PACKET_END).
    ackTimeoutDuration = sl_rail_get_time(railHandle)
                         - previousTxAppendedInfo.time_sent.packet_time;
  }
  // End scheduled receive mode if an appropriate end or error event is received
  if ((events & (SL_RAIL_EVENT_RX_SCHEDULED_RX_END
                 | SL_RAIL_EVENT_RX_SCHEDULED_RX_MISSED))
      && inAppMode(RX_SCHEDULED, NULL)) {
    enableAppMode(RX_SCHEDULED, false, NULL);
  }

  // TX Events
  if (events & SL_RAIL_EVENT_TX_START_CCA) {
    counters.lbtStartCca++;
  }
  if (events & SL_RAIL_EVENT_TX_CCA_ACTIVATED) {
    counters.lbtCcaActivated++;
  }
  if (events & SL_RAIL_EVENT_TX_CCA_RETRY) {
    if (sl_rail_is_next_cca_now(railHandle)) {
      counters.lbtNextCcaNow++;
    }
    counters.lbtRetry++;
  }
  if (events & SL_RAIL_EVENT_TX_CHANNEL_CLEAR) {
    counters.lbtSuccess++;
    ccaSuccesses++;
    if ((txOptions & SL_RAIL_TX_OPTION_CCA_ONLY) != 0U) {
      lastTxStatus = events;
      newTxError = true; // This is a 'pretend error'; see printNewTxError()
      // This doesn't counters.userTx++;
      //@TODO: Should we instead initiate an immediate transmit here?
      scheduleNextTx();
    }
  }
#if SL_RAIL_SUPPORTS_MFM
  if (events & SL_RAIL_EVENT_MFM_TX_BUFFER_DONE) {
    if (railTxDataConfig.tx_source == SL_RAIL_TX_DATA_SOURCE_MFM_DATA) {
      counters.userTx++;
    }
  }
#endif
  if (events & SL_RAIL_EVENT_TX_STARTED) {
    counters.userTxStarted++;
    sl_rail_tx_packet_details_t packetDetails = {
      .time_sent.packet_time = txStartTime,
      .time_sent.total_packet_bytes = SL_RAIL_TX_STARTED_BYTES,
    };
    (void) sl_rail_get_tx_time_preamble_start(railHandle, &packetDetails);
    txStartTime = packetDetails.time_sent.packet_time;
  }
  if (events & SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY) {
    railtest_TxFifoAlmostEmpty(railHandle);
  }
  // Process TX success before any failures in case an auto-repeat fails
  if (events & SL_RAIL_EVENT_TX_PACKET_SENT) {
    counters.userTx++;
    txRemainingCount = sl_rail_get_tx_packets_remaining(railHandle);
    if (txRemainingCount != txRepeatCount) {
      counters.userTxRemainingErrors++;
    }
    if ((txRemainingCount > 0) && (txRepeatCount > 0)) {
      // Defer calling railtest_TxPacketSent() to last of auto-repeat transmits
      internalTransmitCounter++;
      if (txRepeatCount != SL_RAIL_TX_REPEAT_INFINITE_ITERATIONS) {
        txRepeatCount--;
      }
    } else {
      txRepeatCount = 0;
      railtest_TxPacketSent(railHandle, false);
    }
  }
  if (events & (SL_RAIL_EVENT_TX_ABORTED
                | SL_RAIL_EVENT_TX_BLOCKED
                | SL_RAIL_EVENT_TX_UNDERFLOW
                | SL_RAIL_EVENT_TX_CHANNEL_BUSY
                | SL_RAIL_EVENT_TX_SCHEDULED_TX_MISSED)) {
    if (currentAppMode() == TX_STREAM) {
      lastTxStatus = events;
      scheduleNextTx();
    } else {
      lastTxStatus = events;
      txRemainingCount = sl_rail_get_tx_packets_remaining(railHandle);
      if ((txRepeatCount != SL_RAIL_TX_REPEAT_INFINITE_ITERATIONS)
          && (txRepeatCount > 0)
          && ((events & (SL_RAIL_EVENT_TX_ABORTED | SL_RAIL_EVENT_TX_UNDERFLOW)) == 0U)) {
        txRepeatCount++; // A transmit never happened
      }
      if (txRemainingCount != txRepeatCount) {
        counters.userTxRemainingErrors++;
      }
      txRepeatCount = 0;
      newTxError = true;
      failPackets++;
#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH && defined(WISUN_MODESWITCHPHRS_ARRAY_SIZE)
      if ((modeSwitchState == TX_MS_PACKET) || (modeSwitchState == TX_ON_NEW_PHY)) { // Packet has been sent in a MS context
        scheduleNextModeSwitchTx(false);
      } else
#endif
      {
        scheduleNextTx();
      }

      // Increment counters for TX events
      if (events & SL_RAIL_EVENT_TX_ABORTED) {
        counters.userTxAborted++;
      }
      if (events & SL_RAIL_EVENT_TX_BLOCKED) {
#ifdef SL_CATALOG_CS_CLI_PRESENT
        railappcb_TxBlockedCs(railHandle);
#endif
        counters.userTxBlocked++;
      }
      if (events & SL_RAIL_EVENT_TX_UNDERFLOW) {
        counters.userTxUnderflow++;
      }
      if (events & SL_RAIL_EVENT_TX_CHANNEL_BUSY) {
        counters.txChannelBusy++;
      }
    }
  }
  // Put this here too so that we do these things twice
  // in the case that an ack and a non ack have completed
  if (events & SL_RAIL_EVENT_TXACK_PACKET_SENT) {
    counters.ackTx++;
    railtest_TxPacketSent(railHandle, true);
  }
  if (events & (SL_RAIL_EVENT_TXACK_ABORTED
                | SL_RAIL_EVENT_TXACK_BLOCKED
                | SL_RAIL_EVENT_TXACK_UNDERFLOW)) {
    lastTxAckStatus = events;
    failAckPackets++;
    pendFinishTxAckSequence();

    // Increment counters for TXACK events
    if (events & SL_RAIL_EVENT_TXACK_ABORTED) {
      counters.ackTxAborted++;
    }
    if (events & SL_RAIL_EVENT_TXACK_BLOCKED) {
      counters.ackTxBlocked++;
    }
    if (events & SL_RAIL_EVENT_TXACK_UNDERFLOW) {
      counters.ackTxUnderflow++;
    }
  }
  if (events & SL_RAIL_EVENT_RX_CHANNEL_HOPPING_COMPLETE) {
    railtest_RxChannelHoppingComplete(railHandle);
  }
  if (events & SL_RAIL_EVENT_RF_SENSED) {
    counters.rfSensedEvent++;
    if (counters.rfSensedEvent == 0) { // Wrap it to 1 not 0
      counters.rfSensedEvent = 1;
    }
  }
  if (events & SL_RAIL_EVENT_PA_PROTECTION) {
    counters.paProtect++;
  }
#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH && defined(WISUN_MODESWITCHPHRS_ARRAY_SIZE)
  if (events & SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_START) {
    modeSwitchState = RX_ON_NEW_PHY;
    modeSwitchBaseChannel = channel;
    channel = getLikelyChannel();
    modeSwitchNewChannel = channel;
  }
  if (events & SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_END) {
    modeSwitchState = IDLE;
    channel = getLikelyChannel();
    modeSwitchBaseChannel = 0xFFFFU;
    modeSwitchNewChannel = 0xFFFFU;
  }
#endif
#ifdef SL_CATALOG_SL_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
  if (sl_rail_ieee802154_is_enabled(railHandle)) {
    sl_rail_util_ieee801254_on_rail_event(railHandle, events);
  }
#endif //SL_CATALOG_SL_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT
  if (sl_rail_ble_is_enabled(railHandle)) {
    sl_bt_ll_coex_handle_events(events);
  }
#endif //SL_CATALOG_RAIL_UTIL_COEX_PRESENT
}

volatile bool allowPowerManagerSleep = false;
// This app-level function overrides a weak power manager implementation and
// gets called in sl_power_manager_handler.c.
// By default prevent the sl_rail_test App from sleeping and
// keep CLI responsive.
bool app_is_ok_to_sleep(void)
{
  return allowPowerManagerSleep;
}

/******************************************************************************
 * Application Helper Functions
 *****************************************************************************/
void processPendingCalibrations(void)
{
  // Only calibrate the radio when not currently receiving a packet
  bool calsInMode = inAppMode(NONE, NULL);
  bool calsInTxMode = inAppMode(TX_N_PACKETS, NULL)
                      || inAppMode(TX_CONTINUOUS, NULL)
                      || inAppMode(TX_STREAM, NULL);
  bool receivingPacket = sl_rail_get_radio_state(railHandle) == SL_RAIL_RF_STATE_RX_ACTIVE;

  if (calibrateRadio && (calsInMode | calsInTxMode) && !skipCalibrations && !receivingPacket) {
    sl_rail_cal_mask_t pendingCals = sl_rail_get_pending_cal(railHandle);
    sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;

    calibrateRadio = false;

    if ((pendingCals & SL_RAIL_CAL_TEMP_HFXO)
        && !isHFXOCompensationSystematic) {
      // Compensation step 1: wait for thermistor measurement
      status = sl_rail_start_thermistor_measurement(railHandle);
    }

    if (status == SL_RAIL_STATUS_NO_ERROR) {
      // Avoid double increment in case of compensation
      if ((pendingCals & SL_RAIL_CAL_COMPENSATE_HFXO) == 0U) {
        counters.calibrations++;
      }
    } else {
      calibrateRadio = true;
    }

    if (pendingCals & SL_RAIL_CAL_COMPENSATE_HFXO) {
      bool isTxStream = inAppMode(TX_STREAM, NULL);
      if (isTxStream) {
        sl_rail_stop_tx_stream(railHandle);
      }

      // Compensation step 2: compute and apply ppm correction
      sl_rail_radio_state_t radioState = sl_rail_get_radio_state(railHandle);
      if ((radioState == SL_RAIL_RF_STATE_RX) || (radioState == SL_RAIL_RF_STATE_IDLE)) {
        int8_t crystalPPMError;
        // Stop the radio and compensate
        sl_rail_calibrate_hfxo(railHandle, &crystalPPMError);

        if (isTxStream) {
          sl_rail_start_tx_stream(railHandle, channel, streamMode,
                                  concPhyIdOptions | antOptions);
        } else if (receiveModeEnabled && !calsInTxMode) {
          sl_rail_start_rx(railHandle, getLikelyChannel(), NULL);
        }
      } else {
        calibrateRadio = true;
      }
    }

    // Only execute these calibrations when the radio is not currently transmitting
    // or in a transmit mode.
    if (calsInMode) {
      // Perform the necessary calibrations and don't save the results
      if (pendingCals & SL_RAIL_CAL_TEMP_VCO) {
        sl_rail_calibrate_temp(railHandle);
      }

      if (pendingCals & SL_RAIL_CAL_ONETIME_IR_CAL) {
        sl_rail_antenna_sel_t rfPath = SL_RAIL_ANTENNA_AUTO;
        sl_rail_status_t retVal = sl_rail_get_rf_path(railHandle, &rfPath);

        if (retVal == SL_RAIL_STATUS_NO_ERROR) {
          // Disable the radio if we have to do IR Cal
          sl_rail_idle(railHandle, SL_RAIL_IDLE_ABORT, false);
          sl_rail_calibrate_ir(railHandle, NULL, rfPath);
          if (receiveModeEnabled) {
            sl_rail_start_rx(railHandle, channel, NULL);
          }
        }
      }
    }
  }
}

sl_rail_frequency_offset_t getRxFreqOffset(void)
{
  sl_rail_frequency_offset_t retVal = rxFreqOffset;
  rxFreqOffset = SL_RAIL_FREQUENCY_OFFSET_INVALID;
  return retVal;
}

void checkTimerExpiration(void)
{
  if (railTimerExpired) {
    railTimerExpired = false;
    uint32_t paramRailTimerExpireTime = railTimerExpireTime;

    responsePrint("timerCb",
                  "TimerExpiredCallback:%u,ConfiguredExpireTime:%u",
                  paramRailTimerExpireTime,
                  railTimerConfigExpireTime);
  }
}

void printNewTxError(void)
{
  sl_rail_events_t paramLastTxStatus;

  if (newTxError) {
    newTxError = false;
    if (lastTxStatus & SL_RAIL_EVENT_TX_UNDERFLOW) {
      if (logLevel & ASYNC_RESPONSE) {
        paramLastTxStatus = lastTxStatus;
        responsePrint("txPacket",
                      "txStatus:Error,"
                      "errorReason:Tx underflow,"
                      "errorCode:0x%x%08x",
                      (uint32_t)(paramLastTxStatus >> 32),
                      (uint32_t)(paramLastTxStatus));
      }
    }
    if (lastTxStatus & SL_RAIL_EVENT_TX_ABORTED) {
      if (logLevel & ASYNC_RESPONSE) {
        paramLastTxStatus = lastTxStatus;
        responsePrint("txPacket",
                      "txStatus:Error,"
                      "errorReason:Tx abort,"
                      "errorCode:0x%x%08x",
                      (uint32_t)(paramLastTxStatus >> 32),
                      (uint32_t)(paramLastTxStatus));
      }
    }
    if (lastTxStatus & SL_RAIL_EVENT_TX_BLOCKED) {
      if (logLevel & ASYNC_RESPONSE) {
        paramLastTxStatus = lastTxStatus;
        responsePrint("txPacket",
                      "txStatus:Error,"
                      "errorReason:Tx blocked,"
                      "errorCode:0x%x%08x",
                      (uint32_t)(paramLastTxStatus >> 32),
                      (uint32_t)(paramLastTxStatus));
      }
    }
    if (lastTxStatus & SL_RAIL_EVENT_TX_SCHEDULED_TX_MISSED) {
      if (logLevel & ASYNC_RESPONSE) {
        paramLastTxStatus = lastTxStatus;
        responsePrint("txPacket",
                      "txStatus:Error,"
                      "errorReason:SchedTx missed,"
                      "errorCode:0x%x%08x",
                      (uint32_t)(paramLastTxStatus >> 32),
                      (uint32_t)(paramLastTxStatus));
      }
    }
    if (lastTxStatus & SL_RAIL_EVENT_TX_CHANNEL_BUSY) {
      if (logLevel & ASYNC_RESPONSE) {
        paramLastTxStatus = lastTxStatus;
        responsePrint("txPacket",
                      "txStatus:Error,"
                      "errorReason:Tx channel busy,"
                      "errorCode:0x%x%08x",
                      (uint32_t)(paramLastTxStatus >> 32),
                      (uint32_t)(paramLastTxStatus));
      }
    }
    if (lastTxStatus & SL_RAIL_EVENT_TX_CHANNEL_CLEAR) {
      if (logLevel & ASYNC_RESPONSE) {
        responsePrint("txPacket",
                      "txStatus:ChannelClear");
      }
    }
  }
}

void printAckTimeout(void)
{
  if (rxAckTimeout) {
    rxAckTimeout = false;
    responsePrint("rxAckTimeout",
                  "ackTimeoutDuration:%d",
                  ackTimeoutDuration);
  }
}

void changeChannel(uint32_t i)
{
  channel = i;
  updateGraphics();
  // Apply the new channel immediately if you are in receive already
  if (receiveModeEnabled
      || ((sl_rail_get_radio_state(railHandle) & SL_RAIL_RF_STATE_RX) != 0U)) {
    sl_rail_status_t status = sl_rail_start_rx(railHandle, channel, NULL);

    // Lock up if changing the channel failed since calls to this are supposed
    // to be checked for errors
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      responsePrintError("changeChannel",
                         0xF0,
                         "FATAL, call to sl_rail_start_rx() failed (%u)", status);
      while (1) ; // as desired channel has been validated by sl_rail_is_valid_channel(), this is a FATAL error
    }
  } else {
    // If we are not in receive mode, Radio configuration related to selected channel needs to be done
    // in case of following 802.15.4g option configuration. In that case, FRC dynamic frame length mode is checked
    // to see if we have loaded a valid PHY supporting dynamic FEC.
    sl_rail_status_t status = sl_rail_prepare_channel(railHandle, channel);
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      responsePrintError("changeChannel",
                         0xF0,
                         "FATAL, call to sl_rail_prepare_channel() failed (%u)", status);
      while (1) ; // as desired channel has been validated by sl_rail_is_valid_channel(), this is a FATAL error
    }
  }
#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH && defined(WISUN_MODESWITCHPHRS_ARRAY_SIZE)
  if (modeSwitchState != IDLE) {
    RailAppEvent_t *modeSwitchChangeChannel
      = (RailAppEvent_t *)sl_malloc(sizeof(RailAppEvent_t));
    if (modeSwitchChangeChannel == NULL) {
      eventsMissed++;
      return;
    }
    modeSwitchChangeChannel->type = MODE_SWITCH_CHANGE_CHANNEL;
    modeSwitchChangeChannel->modeSwitchChangeChannel.channel = channel;
    queueAdd(&railAppEventQueue, (void *)modeSwitchChangeChannel);
  }
  if (i != modeSwitchNewChannel) {
    modeSwitchState = IDLE;
  }
#endif
}

void pendPacketTx(void)
{
  packetTx = true;
}

sl_rail_status_t chooseTxType(void)
{
  // Invalidate the previous TX's start time
  txStartTime = 0U;
  if (currentAppMode() == TX_SCHEDULED || currentAppMode() == SCHTX_AFTER_RX
      || currentAppMode() == TX_SCHEDULED_N_PACKETS) {
    if (txType == TX_TYPE_CSMA) {
      return sl_rail_start_scheduled_cca_csma_tx(railHandle, channel, txOptions, &nextPacketTxTime,
                                                 csmaConfig, NULL);
    } else if (txType == TX_TYPE_LBT) {
      return sl_rail_start_scheduled_cca_lbt_tx(railHandle, channel, txOptions, &nextPacketTxTime,
                                                lbtConfig, NULL);
    } else {
      return sl_rail_start_scheduled_tx(railHandle, channel, txOptions, &nextPacketTxTime, NULL);
    }
  } else if (txType == TX_TYPE_LBT) {
    return sl_rail_start_cca_lbt_tx(railHandle, channel, txOptions, lbtConfig, NULL);
  } else if (txType == TX_TYPE_CSMA) {
    return sl_rail_start_cca_csma_tx(railHandle, channel, txOptions, csmaConfig, NULL);
  } else {
    return sl_rail_start_tx(railHandle, channel, txOptions, NULL);
  }
}

void sendPacketIfPending(void)
{
  if (packetTx) {
    packetTx = false;
    uint8_t txStatus;

    // Don't decrement in continuous mode
    if (currentAppMode() != TX_CONTINUOUS) {
      txCount--;
    }

    // Generate the payload and start transmitting
    if (currentAppMode() != TX_UNDERFLOW) { // Force underflows in this mode
      // Load packet data before transmitting if manual loading is not enabled
      if (!txFifoManual) {
        loadTxData(txData, txDataLen);
      }
    }

    txStatus = chooseTxType();

    if (txStatus != 0) {
      lastTxStatus = txStatus;
      failPackets++;
      scheduleNextTx(); // No callback will fire, so fake it
    } else if (currentAppMode() == TX_CANCEL) {
      usDelay(txCancelDelay);
      txRepeatCount = 0;
      if (txCancelMode == SL_RAIL_STOP_MODES_NONE) {
        sl_rail_idle(railHandle, SL_RAIL_IDLE_ABORT, false);
      } else if (txCancelMode >= 4U) {
        sl_rail_idle_mode_t idleMode = (sl_rail_idle_mode_t) (txCancelMode - 4U);
        sl_rail_idle(railHandle, idleMode, true);
      } else {
        sl_rail_stop_tx(railHandle, txCancelMode);
      }
      scheduleNextTx(); // callback may not fire, so fake it
    }
  }
}

void pendFinishTxSequence(void)
{
  finishTxSequence = true;
}

void pendFinishTxAckSequence(void)
{
  finishTxAckSequence = true;
}

void finishTxSequenceIfPending(void)
{
  if (finishTxSequence) {
    // Defer finishing to next main-loop iteration if Tx completion
    // event snuck in after printNewTxError() but before this call.
    if (newTxError) {
      return;
    }
    finishTxSequence = false;

    if (currentAppMode() == TX_STREAM) {
      setNextAppMode(NONE, NULL);
      // Print the number of sent and failed packets
      if (logLevel & ASYNC_RESPONSE) {
        responsePrint("txStreamEnd",
                      "txStatus:%s",
                      ((lastTxStatus & RAIL_EVENT_TX_ABORTED) ? "Aborted" : "Complete"));
        lastTxStatus = 0;
      }
    } else {
      // Compute the number of packets sent
      uint32_t sentPackets = internalTransmitCounter - startTransmitCounter;
      // Don't log if we didn't send any packets
      if ((logLevel & ASYNC_RESPONSE)
          && ((failPackets != 0) || (sentPackets != 0) || (ccaSuccesses != 0))) {
        sl_rail_events_t paramLastTxStatus = lastTxStatus;
        uint32_t paramFailPackets = failPackets;
        // Print the number of sent and failed packets
        responsePrint("txEnd",
                      "txStatus:%s,"
                      "transmitted:%u,"
                      "lastTxTime:%u,"
                      "timePos:%u,"
                      "durationUs:%u,"
                      "lastTxStart:%u,"
                      "ccaSuccess:%u,"
                      "failed:%u,"
                      "lastTxStatus:0x%x%08x,"
                      "txRemain:%d,"
                      "isAck:False",
                      (paramFailPackets == 0
                       ? "Complete"
                       : (sentPackets == 0 ? "Error" : "Partial")),
                      sentPackets,
                      previousTxAppendedInfo.time_sent.packet_time,
                      previousTxAppendedInfo.time_sent.time_position,
                      previousTxAppendedInfo.time_sent.packet_duration_us,
                      txStartTime,
                      ccaSuccesses,
                      paramFailPackets,
                      (uint32_t)(paramLastTxStatus >> 32),
                      (uint32_t)(paramLastTxStatus),
                      txRemainingCount);
      }
      startTransmitCounter = internalTransmitCounter;
      failPackets = 0;
      ccaSuccesses = 0;
      lastTxStatus = 0;
    }
  }
  if (finishTxAckSequence) {
    finishTxAckSequence = false;

    if ((logLevel & ASYNC_RESPONSE) && printTxAck) {
      sl_rail_events_t paramLastTxAckStatus = lastTxAckStatus;
      uint32_t paramFailAckPackets = failAckPackets;
      uint32_t paramSentAckPackets = sentAckPackets;
      // Print the number of sent and failed packets
      responsePrint("txEnd",
                    "txStatus:%s,"
                    "transmitted:%u,"
                    "lastTxTime:%u,"
                    "timePos:%u,"
                    "failed:%u,"
                    "lastTxStatus:0x%x%08x,"
                    "isAck:True",
                    paramFailAckPackets == 0
                    ? "Complete"
                    : (paramSentAckPackets == 0 ? "Error" : "Partial"),
                    paramSentAckPackets,
                    previousTxAckAppendedInfo.time_sent.packet_time,
                    previousTxAckAppendedInfo.time_sent.time_position,
                    paramFailAckPackets,
                    (uint32_t)(paramLastTxAckStatus >> 32),
                    (uint32_t)(paramLastTxAckStatus));
    }
    sentAckPackets = 0;
    failAckPackets = 0;
    lastTxAckStatus = 0;
  }
}

void printPacket(char *cmdName,
                 uint8_t *data,
                 uint16_t dataLength,
                 RxPacketData_t *packetData)
{
  // If this is an Rx packet print the appended info
  responsePrintStart(cmdName);
  if (packetData != NULL) {
    responsePrintContinue(
      "len:%d,timeUs:%u,timePos:%u,durationUs:%u,crc:%s,filterMask:0x%x,rssi:%d,lqi:%d,phy:%d",
      packetData->dataLength,
      packetData->appendedInfo.time_received.packet_time,
      packetData->appendedInfo.time_received.time_position,
      packetData->appendedInfo.time_received.packet_duration_us,
      (packetData->appendedInfo.crc_passed) ? "Pass" : "Fail",
      packetData->filterMask,
      packetData->appendedInfo.rssi_dbm,
      packetData->appendedInfo.lqi,
      packetData->appendedInfo.sub_phy_id);

    responsePrintContinue(
      "isAck:%s,syncWordId:%d,antenna:%d,channelHopIdx:%d,channel:%u,held:%u",
      packetData->appendedInfo.is_ack ? "True" : "False",
      packetData->appendedInfo.sync_word_id,
      packetData->appendedInfo.antenna_id,
      packetData->appendedInfo.channel_hopping_channel_index,
      packetData->appendedInfo.channel,
      (unsigned int)packetData->packetHeld);
    if (sl_rail_ieee802154_is_enabled(railHandle)) {
      responsePrintContinue(
        "ed154:%u,lqi154:%u",
        sl_rail_ieee802154_convert_rssi_to_ed(packetData->appendedInfo.rssi_dbm),
        sl_rail_ieee802154_convert_rssi_to_lqi(packetData->appendedInfo.lqi,
                                               packetData->appendedInfo.rssi_dbm));
    }
    if (printRxFreqOffsetData) {
      if (packetData->freqOffset == SL_RAIL_FREQUENCY_OFFSET_INVALID) {
        responsePrintContinue("freqOffset:Invalid");
      } else {
        responsePrintContinue("freqOffset:%d", packetData->freqOffset);
      }
    }
  } else {
    responsePrintContinue("len:%d", dataLength);
  }
  if ((data != NULL) && (dataLength > 0U)) {
    // Manually print out payload bytes iteratively, so that we don't need to
    // reserve a RAM buffer. Finish the response here.
    RAILTEST_PRINTF("{payload:");
    for (int i = 0; i < dataLength; i++) {
      RAILTEST_PRINTF(" 0x%.2x", data[i]);
    }
    RAILTEST_PRINTF("}");
  }
  responsePrintEnd("}");
}

void enqueueEvents(sl_rail_events_t events)
{
  events &= enablePrintEvents;
  if (events != SL_RAIL_EVENTS_NONE) {
    // Capture event time before sl_malloc()
    sl_rail_time_t eventTime = sl_rail_get_time(railHandle);
    RailAppEvent_t *railEvent
      = (RailAppEvent_t *)sl_malloc(sizeof(RailAppEvent_t));
    if (railEvent == NULL) {
      eventsMissed++;
      return;
    }

    railEvent->type = RAIL_EVENT;

    // No need to disable interrupts; this is only called from interrupt
    // context and RAIL doesn't support nested interrupts/events.
    railEvent->railEvent.timestamp = eventTime;
    railEvent->railEvent.events[1] = (uint32_t)(events >> 32);
    railEvent->railEvent.events[0] = (uint32_t)(events);

    queueAdd(&railAppEventQueue, (void *)railEvent);
  }
}

void printRailEvents(RailEvent_t *railEvent)
{
  sl_rail_events_t events = (((sl_rail_events_t) railEvent->events[1] << 32)
                             | railEvent->events[0]);
  if ((events >> numRailEvents) != SL_RAIL_EVENTS_NONE) {
    responsePrintError("printRailEvents", 0x2,
                       "Unknown RAIL Events:0x%x%08x",
                       (uint32_t)(events >> 32),
                       (uint32_t)(events));
    return;
  }
  for (unsigned int i = 0U; events != SL_RAIL_EVENTS_NONE; i++, events >>= 1) {
    if ((events & 1U) != SL_RAIL_EVENTS_NONE) {
      responsePrint("event",
                    "timestamp:%u,eventName:SL_RAIL_EVENT_%s",
                    railEvent->timestamp,
                    eventNames[i]);
    }
  }
}

char *handleToString(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  return "r";//for sl_rail_test vs MP configuration
}
