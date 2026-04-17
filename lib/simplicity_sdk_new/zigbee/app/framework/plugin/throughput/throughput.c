/***************************************************************************//**
 * @file
 * @brief Routines for the Throughput plugin, which provides a mechanism for
 *        measuring network throughput.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <stdint.h>
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "math.h"
#include "app/framework/plugin/counters/counters.h"

#ifdef UNIX_HOST
#include <time.h>
#endif // UNIX_HOST
#define MAX_PACKET_LENGTH 127
#define ZIGBEE_TX_TEST_MAX_INFLIGHT         5

typedef struct {
  bool inUse;
  uint32_t startTime;
  uint8_t seqn;
} sli_zigbee_inflight_info_t;

static struct {
  sl_zigbee_aps_option_t apsOptions;
  sl_802154_short_addr_t destination;
  uint8_t payloadLength;
  uint8_t headerLength;
  uint8_t packetLength;
  uint16_t messageTotalCount;
  uint16_t messageRunningCount;
  uint16_t messageSuccessCount;
  uint8_t maxInFlight;
  uint8_t currentInFlight;
  uint16_t txIntervalMs;
  uint32_t startTime;
  uint32_t runTime;
  uint32_t testTimeout;
  uint32_t minSendTimeMs;
  uint32_t maxSendTimeMs;
  uint32_t sumSendTimeMs;
  uint32_t varSendTimeMs;
  sli_zigbee_inflight_info_t inflightInfoTable[ZIGBEE_TX_TEST_MAX_INFLIGHT];
} testParams;

#include "sl_cli.h"
//stack events and handlers
static sl_zigbee_af_event_t packetSendEvent;
static void packetSendEventHandler(sl_zigbee_af_event_t * event);

//Other declarations
static uint8_t isRunning = 0;
static bool test_in_progress = false;
extern uint16_t sl_zigbee_counters[SL_ZIGBEE_COUNTER_TYPE_COUNT];
extern const char * titleStrings[];
static void getHeaderLen(void);
static void printParameter(uint8_t printSelection);
static void printCounter(uint8_t);
static void printAllParameters(void);
static void printResult(void);
static void startTest(bool doClearCounters);
static void stopTest(void);
static void clearCounters(void);
enum printSelection {
  DESTINATION = 0,
  COUNT,
  INTERVAL,
  HEADER,
  PAYLOAD,
  PACKET,
  INFLIGHT,
  APSOPTIONS,
  TESTTIMEOUT
};

//---internal callback

void sli_zigbee_af_throughput_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_event_init(&packetSendEvent,
                          packetSendEventHandler);
}

//-------------------------------------------------

static void printParameter(uint8_t printSelection)
{
  switch (printSelection) {
    case DESTINATION:
      sl_zigbee_af_core_println("Destination nodeID: 0x%04X ", testParams.destination);
      break;
    case COUNT:
      sl_zigbee_af_core_println("Packets to send: %d ", testParams.messageTotalCount);
      break;
    case INTERVAL:
      sl_zigbee_af_core_println("Transmit interval: %d ms", testParams.txIntervalMs);
      break;
    case HEADER:
      sl_zigbee_af_core_println("Header size: %dB", testParams.headerLength);
      break;
    case PAYLOAD:
      sl_zigbee_af_core_println("Payload size: %dB", testParams.payloadLength);
      break;
    case PACKET:
      sl_zigbee_af_core_println("Packet size: %dB", testParams.packetLength);
      break;
    case INFLIGHT:
      sl_zigbee_af_core_println("Packets in flight: %d", testParams.maxInFlight);
      break;
    case APSOPTIONS:
      sl_zigbee_af_core_println("APS Options = 0x%04X", testParams.apsOptions);
      break;
    case TESTTIMEOUT:
      sl_zigbee_af_core_println("Timeout: %d ms", testParams.testTimeout);
      break;
    default:
      sl_zigbee_af_core_println("Err");
  }
}

static void printAllParameters(void)
{
  sl_zigbee_af_core_println(" ");
  sl_zigbee_af_core_println("TEST PARAMETERS");
  printParameter(DESTINATION);
  printParameter(COUNT);
  printParameter(INTERVAL);
  printParameter(PAYLOAD);
  printParameter(PACKET);
  printParameter(INFLIGHT);
  printParameter(APSOPTIONS);
  printParameter(TESTTIMEOUT);
}

static void printResult(void)
{
  uint32_t mean, var, std;

  if (isRunning) {
    sl_zigbee_af_core_println("Test still in progress");
    testParams.runTime = elapsedTimeInt32u(testParams.startTime, halCommonGetInt32uMillisecondTick());
  }
  uint64_t appThroughput = (testParams.messageSuccessCount
                            * testParams.payloadLength);
  appThroughput = appThroughput * 8 * 1000 / testParams.runTime;

  uint64_t phyThroughput = (testParams.messageSuccessCount
                            * testParams.packetLength);
  phyThroughput = phyThroughput * 8 * 1000 / testParams.runTime;

  sl_zigbee_af_core_println(" ");
  sl_zigbee_af_core_println("THROUGHPUT RESULTS");
  sl_zigbee_af_core_println("Total time %u ms", testParams.runTime);
  sl_zigbee_af_core_println("Success messages: %d out of %d",
                            testParams.messageSuccessCount,
                            testParams.messageTotalCount);
  sl_zigbee_af_core_println("Payload Throughput: %u bits/s", appThroughput);
  sl_zigbee_af_core_println("Phy Throughput: %u bits/s", phyThroughput);
  if (testParams.messageSuccessCount > 0) {
    mean = testParams.sumSendTimeMs / testParams.messageSuccessCount;
    //10X scaling factor makes large number less likely to overflow
    var = testParams.varSendTimeMs * 10 / testParams.messageSuccessCount;
    if (var > (mean * mean) ) {
      var -= (mean * mean);
      std = (uint32_t)sqrt((double)var);
    } else {
      std = 0;
    }

    sl_zigbee_af_core_println("Min packet send time: %u ms",
                              testParams.minSendTimeMs);
    sl_zigbee_af_core_println("Max packet send time: %u ms",
                              testParams.maxSendTimeMs);
    sl_zigbee_af_core_println("Avg packet send time: %u ms", mean);
    sl_zigbee_af_core_println("STD packet send time: %u ms", std);
  }
}

static void startTest(bool doClearCounters)
{
  uint8_t i;

  if (isRunning) {
    sl_zigbee_af_core_println("Test already in progress");
    return;
  }

  if (testParams.packetLength <= 50) {
    sl_zigbee_af_core_println("Err: Invalid Packet Size");
    printAllParameters();
    return;
  }

  if (testParams.messageTotalCount == 0) {
    sl_zigbee_af_core_println("Err: Invalid Count");
    printAllParameters();
    return;
  }

  isRunning = 1;
  testParams.currentInFlight = 0;
  testParams.messageRunningCount = 0;
  testParams.messageSuccessCount = 0;
  testParams.startTime = halCommonGetInt32uMillisecondTick();
  testParams.runTime = 0;
  testParams.minSendTimeMs = 0xFFFFFFFF;
  testParams.maxSendTimeMs = 0;
  testParams.sumSendTimeMs = 0;
  testParams.varSendTimeMs = 0;
  for (i = 0; i < ZIGBEE_TX_TEST_MAX_INFLIGHT; i++) {
    testParams.inflightInfoTable[i].inUse = false;
  }
  if (doClearCounters) {
    clearCounters();
  }
  test_in_progress = true;
  sl_zigbee_af_core_println("Starting Test");
  packetSendEventHandler(&packetSendEvent);
}

static void stopTest(void)
{
  sl_zigbee_af_event_set_inactive(&packetSendEvent);
  testParams.messageTotalCount = 0;
  isRunning = 0;
  test_in_progress = false;
  sl_zigbee_af_core_println("Test Aborted");
}

static void clearCounters(void)
{
  sl_zigbee_af_core_println("Clearing counters");
  sl_zigbee_af_counters_clear();
}

static bool messageSentHandler(sl_status_t status,
                               sl_zigbee_outgoing_message_type_t type,
                               uint16_t indexOrDestination,
                               sl_zigbee_aps_frame_t* apsFrame,
                               uint8_t msgLen,
                               uint8_t* message)
{
  (void)type;
  (void)indexOrDestination;
  (void)msgLen;
  (void)message;

  // Is this is a message sent out as part of the throughput test?
  if (apsFrame->profileId == 0x7F01
      && apsFrame->clusterId == 0x0001
      && (sl_zigbee_af_event_is_scheduled(&packetSendEvent) || test_in_progress)) {
    uint32_t packetSendTimeMs = 0xFFFFFFFF;
    uint8_t i;

    for (i = 0; i < ZIGBEE_TX_TEST_MAX_INFLIGHT; i++) {
      if (testParams.inflightInfoTable[i].seqn == apsFrame->sequence) {
        testParams.currentInFlight--;
        testParams.inflightInfoTable[i].inUse = false;
        packetSendTimeMs = elapsedTimeInt32u(testParams.inflightInfoTable[i].startTime, halCommonGetInt32uMillisecondTick());
        break;
      }
    }

    #ifdef SL_ZIGBEE_TEST
    assert(packetSendTimeMs != 0xFFFFFFFF);
    #endif

    if ((status == SL_STATUS_OK) && (packetSendTimeMs != 0xFFFFFFFF)) {
      testParams.messageSuccessCount++;
      testParams.sumSendTimeMs += packetSendTimeMs;

      if (testParams.minSendTimeMs > packetSendTimeMs) {
        testParams.minSendTimeMs = packetSendTimeMs;
      }
      if (testParams.maxSendTimeMs < packetSendTimeMs) {
        testParams.maxSendTimeMs = packetSendTimeMs;
      }

      //10X scaling factor makes large number less likely to overflow
      testParams.varSendTimeMs += packetSendTimeMs * packetSendTimeMs / 10;
    }

    if (testParams.currentInFlight == 0
        && testParams.messageRunningCount
        == testParams.messageTotalCount) {
      sl_zigbee_af_core_println("Test Complete");
      test_in_progress = false;
      testParams.runTime = elapsedTimeInt32u(testParams.startTime, halCommonGetInt32uMillisecondTick());
      printResult();
    }
    return true;
  }
  return false;
}

void sli_zigbee_af_throughput_message_sent_callback(sl_status_t status,
                                                    sl_zigbee_outgoing_message_type_t type,
                                                    uint16_t indexOrDestination,
                                                    sl_zigbee_aps_frame_t *apsFrame,
                                                    uint16_t messageTag,
                                                    uint8_t messageLength,
                                                    uint8_t *messageContents)
{
  (void)messageTag;

  messageSentHandler(status,
                     type,
                     indexOrDestination,
                     apsFrame,
                     messageLength,
                     messageContents);
}

static void getHeaderLen(void)
{
  sl_zigbee_aps_frame_t* testFrame;
  testFrame = sl_zigbee_af_get_command_aps_frame();
  uint8_t maxPayloadLen = sl_zigbee_af_maximum_aps_payload_length(
    SL_ZIGBEE_OUTGOING_DIRECT,
    testParams.destination,
    testFrame);
  testParams.headerLength = MAX_PACKET_LENGTH - maxPayloadLen;
}

static void printCounter(uint8_t id)
{
  sl_zigbee_af_core_println("%s: %u ", titleStrings[id], sl_zigbee_counters[id]);
}

static void packetSendEventHandler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_zigbee_aps_frame_t apsFrame;
  uint8_t messagePayload[SL_ZIGBEE_AF_MAXIMUM_APS_PAYLOAD_LENGTH];
  uint16_t messageTag[1] = { 0 };
  uint8_t i;
  uint32_t txIntervalAdjustmentMs;
  uint32_t adjustedTxIntervalMs;

  uint32_t currentTimeMs = halCommonGetInt32uMillisecondTick();
  uint32_t totalRunTimeMs = elapsedTimeInt32u(testParams.startTime, currentTimeMs);
  if ((totalRunTimeMs >= testParams.testTimeout)
      && (testParams.testTimeout > 0)) {
    sl_zigbee_af_core_println("Timeout Exceeded");
    stopTest();
    return;
  }
  messagePayload[0] = testParams.payloadLength;
  messagePayload[1] = LOW_BYTE(testParams.messageRunningCount);
  messagePayload[2] = HIGH_BYTE(testParams.messageRunningCount);
  for (i = 3; i < testParams.payloadLength; i++) {
    messagePayload[i] = i - 3;
  }
  if (testParams.maxInFlight > 0
      && testParams.currentInFlight >= testParams.maxInFlight) {
    sl_zigbee_af_event_set_delay_ms(&packetSendEvent, 1);
    return;
  }
  apsFrame.sourceEndpoint = 0xFF;
  apsFrame.destinationEndpoint = 0xFF;
  apsFrame.options = testParams.apsOptions;
  apsFrame.profileId = 0x7F01; // test profile ID
  apsFrame.clusterId = 0x0001; // counted packets cluster

  if (sli_zigbee_af_send(SL_ZIGBEE_OUTGOING_DIRECT,
                         testParams.destination,
                         &apsFrame,
                         testParams.payloadLength,
                         messagePayload,
                         messageTag,
                         0xFFFF,
                         0) == SL_STATUS_OK) {
    testParams.messageRunningCount++;
    testParams.currentInFlight++;

    for (i = 0; i < ZIGBEE_TX_TEST_MAX_INFLIGHT; i++) {
      if (!testParams.inflightInfoTable[i].inUse) {
        testParams.inflightInfoTable[i].inUse = true;
        testParams.inflightInfoTable[i].seqn = apsFrame.sequence;
        testParams.inflightInfoTable[i].startTime =
          halCommonGetInt32uMillisecondTick();
        break;
      }
    }
    assert(i < ZIGBEE_TX_TEST_MAX_INFLIGHT);
  }

  if (testParams.messageRunningCount
      >= testParams.messageTotalCount) {
    sl_zigbee_af_event_set_inactive(&packetSendEvent);
    isRunning = 0;
  } else {
    // txIntervalAdjustment subtracts out time spent in this function from the send loop timer,
    // which makes a significant difference for host applications using ncp-uart
    txIntervalAdjustmentMs = elapsedTimeInt32u(currentTimeMs, halCommonGetInt32uMillisecondTick());
    if (txIntervalAdjustmentMs >= testParams.txIntervalMs) {
      adjustedTxIntervalMs = 0;
    } else {
      adjustedTxIntervalMs = testParams.txIntervalMs - txIntervalAdjustmentMs;
    }
    sl_zigbee_af_event_set_delay_ms(&packetSendEvent,
                                    adjustedTxIntervalMs);
  }
}

void sli_zigbee_af_throughput_cli_print_parameters(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  printAllParameters();
}

void sli_zigbee_af_throughput_cli_set_all_parameters(sl_cli_command_arg_t *arguments)
{
  static uint8_t minPacketLen;

  testParams.destination = sl_cli_get_argument_uint16(arguments, 0);
  testParams.messageTotalCount = sl_cli_get_argument_uint16(arguments, 1);
  testParams.txIntervalMs = sl_cli_get_argument_uint16(arguments, 2);
  testParams.packetLength = sl_cli_get_argument_uint8(arguments, 3);
  testParams.maxInFlight = sl_cli_get_argument_uint8(arguments, 4);
  testParams.apsOptions = sl_cli_get_argument_uint16(arguments, 5);
  testParams.testTimeout = sl_cli_get_argument_uint32(arguments, 6);

  // Check packet length parameter
  getHeaderLen();
  minPacketLen = testParams.headerLength + 4;
  if (testParams.packetLength < minPacketLen) {
    testParams.packetLength = minPacketLen;
  } else if (testParams.packetLength > MAX_PACKET_LENGTH) {
    testParams.packetLength = MAX_PACKET_LENGTH;
  }
  testParams.payloadLength = testParams.packetLength - testParams.headerLength;

  // Check in-flight parameter
  if (testParams.maxInFlight > ZIGBEE_TX_TEST_MAX_INFLIGHT) {
    testParams.maxInFlight = ZIGBEE_TX_TEST_MAX_INFLIGHT;
  } else if (testParams.maxInFlight <= 0) {
    testParams.maxInFlight = 1;
  }
  printAllParameters();
}

void sli_zigbee_af_throughput_cli_set_destination(sl_cli_command_arg_t *arguments)
{
  testParams.destination = sl_cli_get_argument_uint16(arguments, 0);
  getHeaderLen();
  testParams.packetLength = testParams.payloadLength + testParams.headerLength;
  if (testParams.packetLength > MAX_PACKET_LENGTH) {
    testParams.payloadLength = MAX_PACKET_LENGTH - testParams.headerLength;
  }
  printParameter(DESTINATION);
  printParameter(PACKET);
}

void sli_zigbee_af_throughput_cli_set_test_count(sl_cli_command_arg_t *arguments)
{
  testParams.messageTotalCount = sl_cli_get_argument_uint16(arguments, 0);
  printParameter(COUNT);
}

void sli_zigbee_af_throughput_cli_set_interval(sl_cli_command_arg_t *arguments)
{
  testParams.txIntervalMs = sl_cli_get_argument_uint16(arguments, 0);
  printParameter(INTERVAL);
}

void sli_zigbee_af_throughput_cli_set_packet_size(sl_cli_command_arg_t *arguments)
{
  static uint8_t minPacketLen;

  testParams.packetLength = sl_cli_get_argument_uint8(arguments, 0);
  getHeaderLen();
  minPacketLen = testParams.headerLength + 4;

  if (testParams.packetLength < minPacketLen) {
    testParams.packetLength = minPacketLen;
  } else if (testParams.packetLength > MAX_PACKET_LENGTH) {
    testParams.packetLength = MAX_PACKET_LENGTH;
  }
  testParams.payloadLength = testParams.packetLength - testParams.headerLength;

  sl_zigbee_af_core_println("Max packet: 127");
  printParameter(HEADER);
  printParameter(PAYLOAD);
  printParameter(PACKET);
}

void sli_zigbee_af_throughput_cli_set_in_flight_count(sl_cli_command_arg_t *arguments)
{
  testParams.maxInFlight = sl_cli_get_argument_uint8(arguments, 0);
  if (testParams.maxInFlight > ZIGBEE_TX_TEST_MAX_INFLIGHT) {
    testParams.maxInFlight = ZIGBEE_TX_TEST_MAX_INFLIGHT;
  } else if (testParams.maxInFlight <= 0) {
    testParams.maxInFlight = 1;
  }
  printParameter(INFLIGHT);
}

void sli_zigbee_af_throughput_cli_set_aps_ack_on(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  testParams.apsOptions |= SL_ZIGBEE_APS_OPTION_RETRY;
  printParameter(APSOPTIONS);
}

void sli_zigbee_af_throughput_cli_set_aps_ack_off(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  testParams.apsOptions &= ~SL_ZIGBEE_APS_OPTION_RETRY;
  printParameter(APSOPTIONS);
}

void sli_zigbee_af_throughput_cli_set_test_timeout(sl_cli_command_arg_t *arguments)
{
  testParams.testTimeout = sl_cli_get_argument_uint32(arguments, 0);
  printParameter(TESTTIMEOUT);
}

void sli_zigbee_af_throughput_cli_clear_counters(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  clearCounters();
}

void sli_zigbee_af_throughput_cli_stop_test(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  stopTest();
}

void sli_zigbee_af_throughput_cli_start_test(sl_cli_command_arg_t *arguments)
{
  bool clear_counters = true;
  if (sl_cli_get_argument_count(arguments) >= 1) {
    clear_counters = sl_cli_get_argument_uint8(arguments, 0);
  }

  startTest(clear_counters);
}

void sli_zigbee_af_throughput_cli_print_result(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  printResult();
}

void sli_zigbee_af_throughput_cli_print_counters(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
#ifdef EZSP_HOST
  sl_zigbee_ezsp_read_counters(sl_zigbee_counters);
#else
  sl_zigbee_read_counters(sl_zigbee_counters, SL_ZIGBEE_COUNTER_TYPE_COUNT);
#endif
  sl_zigbee_af_core_println(" ");
  sl_zigbee_af_core_println("COUNTERS");
  printCounter(SL_ZIGBEE_COUNTER_PHY_CCA_FAIL_COUNT);
  printCounter(SL_ZIGBEE_COUNTER_MAC_TX_UNICAST_SUCCESS);
  printCounter(SL_ZIGBEE_COUNTER_MAC_TX_UNICAST_RETRY);
  printCounter(SL_ZIGBEE_COUNTER_MAC_TX_UNICAST_FAILED);
  printCounter(SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_SUCCESS);
  printCounter(SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_RETRY);
  printCounter(SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_FAILED);
}
