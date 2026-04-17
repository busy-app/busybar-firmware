/***************************************************************************//**
 * @file
 * @brief Utility internal code that provides useful CLI commands and
 * application logic to perform dynamic multiprotocol testing between ZigBee and
 * BLE.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include PLATFORM_HEADER
#include "hal.h"
#include "sl_component_catalog.h"
#include "sl_cli.h"
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#include "sl_zigbee_debug_print.h"
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#ifdef SL_CATALOG_ZIGBEE_DEBUG_EXTENDED_PRESENT
#include "sl_zigbee_debug.h"
#endif //SL_CATALOG_ZIGBEE_DEBUG_EXTENDED_PRESENT
#ifdef SL_CATALOG_ZIGBEE_DEBUG_BASIC_PRESENT
#include "zigbee_debug_channel.h"
#endif //SL_CATALOG_ZIGBEE_DEBUG_BASIC_PRESENT
#include "network_test_config.h"

#include "sl_zigbee_system_common.h"
#include "app/framework/util/util.h"

#ifdef SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT
  #include "fragmentation.h"
#endif

//------------------------------------------------------------------------------
// Defines and variables.
#define MAX_ZIGBEE_TX_TEST_MESSAGE_LENGTH   70
#define ZIGBEE_TX_TEST_MAX_INFLIGHT         5
uint32_t ping_send_time_ms = 0;
typedef struct {
  bool in_use;
  uint32_t start_time;
  uint8_t seqn;
} sli_zigbee_in_flight_info_t;

#ifdef SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT
#define MAXIMUM_MESSAGE_LENGTH SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE
#else
#define MAXIMUM_MESSAGE_LENGTH 255
#endif

static struct {
  sl_zigbee_aps_option_t aps_options;
  sl_802154_short_addr_t destination;
  uint16_t message_length;
  uint16_t message_total_count;
  uint16_t message_running_count;
  uint16_t message_success_count;
  uint8_t max_in_flight;
  uint8_t current_in_flight;
  uint16_t tx_delay_ms;
  uint32_t start_time;
  uint32_t min_send_time_ms;
  uint32_t max_send_time_ms;
  uint32_t sum_send_time_ms;
  sli_zigbee_in_flight_info_t in_flight_info_table[ZIGBEE_TX_TEST_MAX_INFLIGHT];
  uint8_t message_payload[MAXIMUM_MESSAGE_LENGTH];
} zigbee_tx_test_info;

static sl_zigbee_af_event_t zigbee_tx_test_event;
static uint16_t sequence_counter;
static bool test_in_progress = false;

static sl_zigbee_af_event_t zigbee_large_network_event;
static void zigbee_large_network_event_handler(sl_zigbee_af_event_t *event);
static bool network_test_is_scheduled(void);
static uint8_t *data_ptr = NULL;
//------------------------------------------------------------------------------
// Extern and Forward declarations
extern sl_status_t sl_zigbee_af_send_unicast(sl_zigbee_outgoing_message_type_t type,
                                             uint16_t indexOrDestination,
                                             sl_zigbee_aps_frame_t *apsFrame,
                                             uint16_t messageLength,
                                             uint8_t* message);
extern sl_status_t sli_zigbee_af_send(sl_zigbee_outgoing_message_type_t type,
                                      uint16_t indexOrDestination,
                                      sl_zigbee_aps_frame_t *aps_frame,
                                      uint8_t messageLength,
                                      uint8_t *message,
                                      uint16_t *messageTag,
                                      sl_802154_short_addr_t alias,
                                      uint8_t sequence);
static void print_zigbee_tx_test_stats(void);
static void zigbee_tx_test_event_handler(sl_zigbee_af_event_t *event);

//------------------------------------------------------------------------------
// ZigBee callbacks and event handlers

void sli_zigbee_network_test_init(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
      sl_zigbee_af_event_init(&zigbee_tx_test_event, zigbee_tx_test_event_handler);
      sl_zigbee_af_event_init(&zigbee_large_network_event, zigbee_large_network_event_handler);
      break;
    }

    default:
    {
      break;
    }
  }
}

void sli_zigbee_network_test_message_sent_callback(sl_status_t status,
                                                   sl_zigbee_incoming_message_type_t type,
                                                   uint16_t index_or_destination,
                                                   sl_zigbee_aps_frame_t *aps_frame,
                                                   uint16_t messageTag,
                                                   uint8_t messageLength,
                                                   uint8_t *message)
{
  (void)type;
  (void)index_or_destination;
  (void)messageTag;
  (void)messageLength;
  (void)message;

  // This is a message sent out as part of the ZigBee TX test, and this isn't a different plugin's test
  if (aps_frame->profileId == 0x7F01
      && (aps_frame->clusterId == 0x0001 || aps_frame->clusterId == 0x0042)
      && (network_test_is_scheduled() || test_in_progress)) {
    uint32_t packet_send_time_ms = 0xFFFFFFFF;
    uint8_t i;

    for (i = 0; i < ZIGBEE_TX_TEST_MAX_INFLIGHT; i++) {
      if (zigbee_tx_test_info.in_flight_info_table[i].seqn == aps_frame->sequence
          || zigbee_tx_test_info.in_flight_info_table[i].seqn == messageTag) {
        zigbee_tx_test_info.in_flight_info_table[i].in_use = false;
        if (zigbee_tx_test_info.current_in_flight > 0) {
          zigbee_tx_test_info.current_in_flight--;
        }
        packet_send_time_ms =
          elapsedTimeInt32u(zigbee_tx_test_info.in_flight_info_table[i].start_time,
                            halCommonGetInt32uMillisecondTick());
        break;
      }
    }

    if (status == SL_STATUS_OK && packet_send_time_ms != 0xFFFFFFFF) {
      zigbee_tx_test_info.message_success_count++;
      zigbee_tx_test_info.sum_send_time_ms += packet_send_time_ms;

      if (zigbee_tx_test_info.min_send_time_ms > packet_send_time_ms) {
        zigbee_tx_test_info.min_send_time_ms = packet_send_time_ms;
      }
      if (zigbee_tx_test_info.max_send_time_ms < packet_send_time_ms) {
        zigbee_tx_test_info.max_send_time_ms = packet_send_time_ms;
      }
    }

    if (zigbee_tx_test_info.current_in_flight == 0
        && zigbee_tx_test_info.message_running_count
        >= zigbee_tx_test_info.message_total_count) {
      test_in_progress = false;
      print_zigbee_tx_test_stats();
    }
  } else if (aps_frame->clusterId == 0x0043 && data_ptr != NULL) { // ping
    // Free the allocated memory, safe to be called on NULL
    free(data_ptr);
    data_ptr = NULL;
  }
}

static void print_zigbee_tx_test_stats(void)
{
  uint32_t total_send_time_ms =
    elapsedTimeInt32u(zigbee_tx_test_info.start_time,
                      halCommonGetInt32uMillisecondTick());
  uint64_t throughput = (zigbee_tx_test_info.message_success_count
                         * zigbee_tx_test_info.message_length * 8);
  throughput = throughput * 1000;
  throughput = throughput / total_send_time_ms;

  sl_zigbee_app_debug_println("ZigBee TX done");
  sl_zigbee_app_debug_println("Total time %lums", total_send_time_ms);
  sl_zigbee_app_debug_println("Success messages: %d out of %d",
                              zigbee_tx_test_info.message_success_count,
                              zigbee_tx_test_info.message_total_count);
  sl_zigbee_app_debug_println("Throughput: %llu bits/s", throughput);
  sl_zigbee_app_debug_println("Min packet send time: %lu ms",
                              zigbee_tx_test_info.min_send_time_ms);
  sl_zigbee_app_debug_println("Max packet send time: %lu ms",
                              zigbee_tx_test_info.max_send_time_ms);

  if (zigbee_tx_test_info.message_success_count > 0) {
    sl_zigbee_app_debug_println("Avg packet send time: %lu ms",
                                (zigbee_tx_test_info.sum_send_time_ms
                                 / zigbee_tx_test_info.message_success_count));
  } else {
    sl_zigbee_app_debug_println("Avg packet send time: %lu ms",
                                0);
  }
}

void zigbee_tx_test_start_random(sl_cli_command_arg_t *arguments)
{
  // If a test is already in progress, do not corrupt the ongoing test data
  if (network_test_is_scheduled()) {
    sl_zigbee_app_debug_println("Test is in progress. Exiting");
    return;
  }

  zigbee_tx_test_info.message_length = sl_cli_get_argument_uint16(arguments, 0);
  zigbee_tx_test_info.message_total_count = sl_cli_get_argument_uint16(arguments, 1);
  zigbee_tx_test_info.max_in_flight = sl_cli_get_argument_uint8(arguments, 2);
  zigbee_tx_test_info.destination = sl_cli_get_argument_uint16(arguments, 3);
  zigbee_tx_test_info.tx_delay_ms = 0;
  sequence_counter = sl_cli_get_argument_uint16(arguments, 4);

  if (zigbee_tx_test_info.max_in_flight > ZIGBEE_TX_TEST_MAX_INFLIGHT) {
    sl_zigbee_app_debug_println("Error: max allowed in flight is %d",
                                ZIGBEE_TX_TEST_MAX_INFLIGHT);
    return;
  }

  if (zigbee_tx_test_info.message_total_count == 0
      || zigbee_tx_test_info.message_total_count == 0xFFFF) {
    sl_zigbee_app_debug_println("Error: invalid message count");
    return;
  }

  uint16_t count = 0;
  for (uint8_t j = 0; j < (zigbee_tx_test_info.message_length + 255) / 256; j++) {
    uint8_t limit = (j == (zigbee_tx_test_info.message_length / 256)) ? (zigbee_tx_test_info.message_length % 256) : 255;
    for (uint8_t i = 6; i < limit; i++) {
      zigbee_tx_test_info.message_payload[count] = i;
      count++;
    }
  }

  // First byte is the message length
  zigbee_tx_test_info.message_payload[0] = 0xFA;
  zigbee_tx_test_info.message_payload[1] = 0xDE;
  zigbee_tx_test_info.message_payload[2] = 0xFE;
  // Indices 3,4 are sequence number set in the event handler

  zigbee_tx_test_info.current_in_flight = 0;
  zigbee_tx_test_info.message_running_count = 0;
  zigbee_tx_test_info.message_success_count = 0;
  zigbee_tx_test_info.start_time = halCommonGetInt32uMillisecondTick();
  zigbee_tx_test_info.min_send_time_ms = 0xFFFFFFFF;
  zigbee_tx_test_info.max_send_time_ms = 0;
  zigbee_tx_test_info.sum_send_time_ms = 0;

  for (uint8_t i = 0; i < ZIGBEE_TX_TEST_MAX_INFLIGHT; i++) {
    zigbee_tx_test_info.in_flight_info_table[i].in_use = false;
  }

  sl_zigbee_app_debug_println("ZigBee TX test started");
  test_in_progress = true;

  // Set event to active - do not call handler from here since this is run
  // from the CLI task context.  sli_cli_post_cmd_hook will post a flag that
  // allows the Zigbee (app framework) task to run, at which point in time the
  // event handler will run and start the test.
  sl_zigbee_af_event_set_active(&zigbee_large_network_event);
}

static void zigbee_large_network_event_handler(sl_zigbee_af_event_t *event)
{
  (void)event;
  sl_zigbee_aps_frame_t apsf;
  apsf.sourceEndpoint = 0x01;
  apsf.destinationEndpoint = 0x01;
  apsf.options = (SL_ZIGBEE_APS_OPTION_RETRY | SL_ZIGBEE_APS_OPTION_ENABLE_ADDRESS_DISCOVERY);
  apsf.profileId = 0x7F01; // test profile ID
  apsf.clusterId = 0x0042; // counted packets cluster

  if (zigbee_tx_test_info.max_in_flight > 0
      && zigbee_tx_test_info.current_in_flight >= zigbee_tx_test_info.max_in_flight) {
    // Max in flight reached, wait and try again.
    sl_zigbee_af_event_set_delay_ms(&zigbee_large_network_event, 1);
    return;
  }

  // fourth and fifth bytes are sequence counters
  zigbee_tx_test_info.message_payload[3] = (sequence_counter >> 8);
  zigbee_tx_test_info.message_payload[4] = (sequence_counter);
  uint8_t outgoing_type = SL_ZIGBEE_OUTGOING_DIRECT;
  uint16_t messageTag = 0xFFFF;
  uint8_t out_seq;

  #ifndef SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT
  if (SL_STATUS_OK == sl_zigbee_send_unicast(outgoing_type,
                                             zigbee_tx_test_info.destination,
                                             &apsf,
                                             messageTag, // tag
                                             zigbee_tx_test_info.message_length,
                                             zigbee_tx_test_info.message_payload,
                                             &apsf.sequence)) {
    out_seq = apsf.sequence;
  #else
  if (SL_STATUS_OK == sli_zigbee_af_fragmentation_send_unicast(outgoing_type,
                                                               zigbee_tx_test_info.destination,
                                                               &apsf,
                                                               zigbee_tx_test_info.message_payload,
                                                               zigbee_tx_test_info.message_length,
                                                               &messageTag)) {
    out_seq = messageTag;
  #endif
    zigbee_tx_test_info.message_running_count++;
    zigbee_tx_test_info.current_in_flight++;
    sequence_counter++;
    uint8_t i;
    for (i = 0; i < ZIGBEE_TX_TEST_MAX_INFLIGHT; i++) {
      if (!zigbee_tx_test_info.in_flight_info_table[i].in_use) {
        zigbee_tx_test_info.in_flight_info_table[i].in_use = true;
        zigbee_tx_test_info.in_flight_info_table[i].seqn = out_seq;
        zigbee_tx_test_info.in_flight_info_table[i].start_time = halCommonGetInt32uMillisecondTick();
        break;
      }
    }
    // Make sure we have a free slot for next round when we increase current_in_flight - this should never happen
    assert(i < ZIGBEE_TX_TEST_MAX_INFLIGHT);
  }

  if (zigbee_tx_test_info.message_running_count
      >= zigbee_tx_test_info.message_total_count) {
    sl_zigbee_af_event_set_inactive(&zigbee_large_network_event);
  } else {
    sl_zigbee_af_event_set_active(&zigbee_large_network_event);
  }
}

static void zigbee_tx_test_event_handler(sl_zigbee_af_event_t *event)
{
  (void) event;

  if (zigbee_tx_test_info.max_in_flight > 0
      && zigbee_tx_test_info.current_in_flight >= zigbee_tx_test_info.max_in_flight) {
    // Max in flight reached, wait and try again.
    sl_zigbee_af_event_set_delay_ms(&zigbee_tx_test_event, 1);
    return;
  }

  sl_zigbee_aps_frame_t aps_frame;
  aps_frame.sourceEndpoint = 0xFF;
  aps_frame.destinationEndpoint = 0xFF;
  aps_frame.options = zigbee_tx_test_info.aps_options;
  aps_frame.profileId = 0x7F01; // test profile ID
  aps_frame.clusterId = 0x0001; // counted packets cluster

  // First byte is the message length
  zigbee_tx_test_info.message_payload[0] = zigbee_tx_test_info.message_length;
  // Second and third bytes are the message counter
  zigbee_tx_test_info.message_payload[1] = LOW_BYTE(zigbee_tx_test_info.message_running_count);
  zigbee_tx_test_info.message_payload[2] = HIGH_BYTE(zigbee_tx_test_info.message_running_count);
  // fourth and fifth bytes are sequence counters
  zigbee_tx_test_info.message_payload[3] = (sequence_counter >> 8);
  zigbee_tx_test_info.message_payload[4] = (sequence_counter);
  uint8_t outgoing_type = (zigbee_tx_test_info.destination == 0xFFFF) ? SL_ZIGBEE_OUTGOING_BROADCAST : SL_ZIGBEE_OUTGOING_DIRECT;

  if (SL_STATUS_OK == sl_zigbee_send_unicast(outgoing_type,
                                             zigbee_tx_test_info.destination,
                                             &aps_frame,
                                             0xFFFF,        // tag
                                             zigbee_tx_test_info.message_length,
                                             zigbee_tx_test_info.message_payload,
                                             &aps_frame.sequence)) {
    zigbee_tx_test_info.message_running_count++;
    zigbee_tx_test_info.current_in_flight++;
    sequence_counter++;
    uint8_t i;
    for (i = 0; i < ZIGBEE_TX_TEST_MAX_INFLIGHT; i++) {
      if (!zigbee_tx_test_info.in_flight_info_table[i].in_use) {
        zigbee_tx_test_info.in_flight_info_table[i].in_use = true;
        zigbee_tx_test_info.in_flight_info_table[i].seqn = aps_frame.sequence;
        zigbee_tx_test_info.in_flight_info_table[i].start_time = halCommonGetInt32uMillisecondTick();
        break;
      }
    }
    // Make sure we have a free slot for next round when we increase current_in_flight - this should never happen
    assert(i < ZIGBEE_TX_TEST_MAX_INFLIGHT);
  }

  if (zigbee_tx_test_info.message_running_count
      >= zigbee_tx_test_info.message_total_count) {
    sl_zigbee_af_event_set_inactive(&zigbee_tx_test_event);
  } else {
    sl_zigbee_af_event_set_delay_ms(&zigbee_tx_test_event,
                                    zigbee_tx_test_info.tx_delay_ms);
  }
}

// ZigBee related commands
void zigbee_tx_test_start_command(sl_cli_command_arg_t *arguments)
{
  uint8_t i;

  // If a test is already in progress, do not corrupt the ongoing test data
  if (network_test_is_scheduled()) {
    sl_zigbee_app_debug_println("Test is in progress. Exiting");
    return;
  }

  zigbee_tx_test_info.message_length = sl_cli_get_argument_uint16(arguments, 0);

  if (zigbee_tx_test_info.message_length > MAX_ZIGBEE_TX_TEST_MESSAGE_LENGTH) {
    sl_zigbee_app_debug_println("Error: max allowed message payload is %d",
                                MAX_ZIGBEE_TX_TEST_MESSAGE_LENGTH);
    return;
  }

  zigbee_tx_test_info.max_in_flight = sl_cli_get_argument_uint8(arguments, 3);

  if (zigbee_tx_test_info.max_in_flight > ZIGBEE_TX_TEST_MAX_INFLIGHT) {
    sl_zigbee_app_debug_println("Error: max allowed in flight is %d",
                                ZIGBEE_TX_TEST_MAX_INFLIGHT);
    return;
  }

  zigbee_tx_test_info.message_total_count = sl_cli_get_argument_uint16(arguments, 1);

  if (zigbee_tx_test_info.message_total_count == 0
      || zigbee_tx_test_info.message_total_count == 0xFFFF) {
    sl_zigbee_app_debug_println("Error: invalid message count");
    return;
  }

  zigbee_tx_test_info.tx_delay_ms = sl_cli_get_argument_uint16(arguments, 2);
  zigbee_tx_test_info.destination = sl_cli_get_argument_uint16(arguments, 4);
  zigbee_tx_test_info.aps_options = sl_cli_get_argument_uint16(arguments, 5);
  // Init the the rest of the message payload with progressive byte values
  for (i = 5; i < zigbee_tx_test_info.message_length; i++) {
    zigbee_tx_test_info.message_payload[i] = i - 5;
  }

  zigbee_tx_test_info.current_in_flight = 0;
  zigbee_tx_test_info.message_running_count = 0;
  zigbee_tx_test_info.message_success_count = 0;
  zigbee_tx_test_info.start_time = halCommonGetInt32uMillisecondTick();
  zigbee_tx_test_info.min_send_time_ms = 0xFFFFFFFF;
  zigbee_tx_test_info.max_send_time_ms = 0;
  zigbee_tx_test_info.sum_send_time_ms = 0;

  for (i = 0; i < ZIGBEE_TX_TEST_MAX_INFLIGHT; i++) {
    zigbee_tx_test_info.in_flight_info_table[i].in_use = false;
  }

  sl_zigbee_app_debug_println("ZigBee TX test started");
  test_in_progress = true;

  // Set event to active - do not call handler from here since this is run
  // from the CLI task context. sli_cli_post_cmd_hook will post a flag that
  // allows the Zigbee task to run at which point in time the event handler will
  // run and start the test. !!! ember APIs are not thread safe !!!
  sl_zigbee_af_event_set_active(&zigbee_tx_test_event);
}

void zigbee_tx_test_stop_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  sl_zigbee_af_event_set_inactive(&zigbee_tx_test_event);
  sl_zigbee_af_event_set_inactive(&zigbee_large_network_event);
  zigbee_tx_test_info.message_total_count = 0;
  test_in_progress = false;
  sl_zigbee_app_debug_println("ZigBee TX test stopped");
}

static bool network_test_is_scheduled(void)
{
  return (sl_zigbee_af_event_is_scheduled(&zigbee_tx_test_event) || sl_zigbee_af_event_is_scheduled(&zigbee_large_network_event));
}

#ifndef EZSP_HOST

void zigbee_set_passive_ack_config(sl_cli_command_arg_t *arguments)
{
  uint8_t config = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t min_acks_needed = sl_cli_get_argument_uint8(arguments, 1);

  if (SL_STATUS_OK != sl_zigbee_set_passive_ack_config((sl_passive_ack_config_enum_t)config,
                                                       min_acks_needed)) {
    sl_zigbee_app_debug_println("Error: Invalid value passive");
    return;
  }

  sl_zigbee_app_debug_println("Passive ACKs configured: config=%d, min_acks=%d",
                              config, min_acks_needed);
}

#endif // EZSP_HOST

void zigbee_frag_raw_tx_command(sl_cli_command_arg_t *arguments)
{
  uint16_t dest = sl_cli_get_argument_uint16(arguments, 0);
  uint16_t length = sl_cli_get_argument_uint16(arguments, 1);

  if (length > MAXIMUM_MESSAGE_LENGTH) {
    sl_zigbee_app_debug_println("Error: max message length is %d",
                                MAXIMUM_MESSAGE_LENGTH);
    return;
  }
  if (data_ptr != NULL) {
    sl_zigbee_app_debug_println("Another ping is in progress");
    return;
  }
  // Allocate memory for data_ptr
  data_ptr = (uint8_t *)malloc(length * sizeof(uint8_t));
  if (data_ptr == NULL) {
    sl_zigbee_app_debug_println("Error: Memory allocation failed");
    return;
  }
  uint16_t count = 0;
  for (uint8_t j = 0; j < (length + 255) / 256; j++) {
    uint8_t limit = (j == (length / 256)) ? (length % 256) : 255;
    for (uint8_t i = 0; i < limit; i++) {
      data_ptr[count] = i;
      count++;
    }
  }
  data_ptr[0] = 0xFA;
  data_ptr[1] = 0xDE;
  data_ptr[2] = 0xFE;
  data_ptr[3] = (sequence_counter >> 8);
  data_ptr[4] = (sequence_counter);

  sl_zigbee_aps_frame_t aps_frame;
  aps_frame.sourceEndpoint = 0x01;
  aps_frame.destinationEndpoint = 0x01;
  aps_frame.options = (SL_ZIGBEE_APS_OPTION_RETRY | SL_ZIGBEE_APS_OPTION_ENABLE_ADDRESS_DISCOVERY);
  aps_frame.profileId = 0x7F01; // test profile ID
  aps_frame.clusterId = 0x0043; // counted packets cluster

  sli_zigbee_af_apply_disable_default_response(&data_ptr[0]);
  sli_zigbee_af_apply_retry_override(&aps_frame.options);

  sl_status_t status = sl_zigbee_af_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                                 dest,
                                                 &aps_frame,
                                                 length,
                                                 data_ptr);
  if (status != SL_STATUS_OK) {
    sl_zigbee_app_debug_println("Error: %d", status);
  } else {
    ping_send_time_ms = halCommonGetInt32uMillisecondTick();
    sequence_counter++;
  }
}

#if (LARGE_NETWORK_TESTING == 1)

#include "sl_zigbee_pro_stack_config.h"
extern uint8_t sli_zigbee_new_broadcast_entry_threshold;

// raw 0x0042 { FA DE FE 00 01}
// send 0xffff 1 1

// For ping
// raw 0x0043 { FA DE FE 00 01}
// send 0x0000 1 1
bool sl_zigbee_af_pre_command_received_cb(sl_zigbee_af_cluster_command_t* cmd)
{
  // Print debug message for latency timing.
  if (cmd->apsFrame->clusterId == 0x0042 || cmd->apsFrame->clusterId == 0x0043) {
    uint8_t *msg_contents = cmd->buffer;
    if (msg_contents[0] == 0xFA && msg_contents[1] == 0xDE ) {
      sli_zigbee_debug_binary_format(EM_DEBUG_LATENCY, "BBBB",
                                     0x00, // frame control, top bit is start/stop
                                     2, //2 byte sequence number
                                     msg_contents[3],
                                     msg_contents[4]);
    }

    if (cmd->apsFrame->clusterId == 0x0043) {
      sl_zigbee_aps_frame_t apsFrame;
      memcpy(&apsFrame, cmd->apsFrame, sizeof(apsFrame));
      apsFrame.clusterId = 0x0044;
      (void)sl_zigbee_af_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                      cmd->source,
                                      &apsFrame,
                                      cmd->bufLen,
                                      cmd->buffer);
    }
    return true;
  } else if (cmd->apsFrame->clusterId == 0x0044) {
    #define INDEX_OF_SEQ_NUM_START 0x03
    #define INDEX_OF_SEQ_NUM_END   0x04
    uint32_t ping_duration_time_ms = elapsedTimeInt32u(ping_send_time_ms, halCommonGetInt32uMillisecondTick());
    sl_zigbee_app_debug_println("\nPing returned after %lu ms", ping_duration_time_ms);
    ping_send_time_ms = 0;

    return true;
  }
  return false;
}
// raw 0x0042 { FA DE FE 00 01}
// send 0xffff 1 1
bool sl_zigbee_af_pre_message_send_cb(sl_zigbee_af_message_struct_t* messageStruct,
                                      sl_status_t* status)
{
  // Print debug message for latency timing.
  if (messageStruct->apsFrame->clusterId == 0x0042 || messageStruct->apsFrame->clusterId == 0x0043) {
    uint8_t *msg_contents = messageStruct->message;
    if (msg_contents[0] == 0xFA && msg_contents[1] == 0xDE ) {
      sli_zigbee_debug_binary_format(EM_DEBUG_LATENCY, "BBBB",
                                     0x80, // frame control, top bit is start/stop
                                     2, //2 byte sequence number
                                     msg_contents[3],
                                     msg_contents[4]);
      *status = SL_STATUS_OK;
    }

    if (messageStruct->apsFrame->clusterId == 0x0043) {
      ping_send_time_ms = halCommonGetInt32uMillisecondTick();
    }
  }

  return false;
}

void sl_zigbee_af_main_init_cb(void)
{
  sli_zigbee_new_broadcast_entry_threshold = SL_ZIGBEE_BROADCAST_TABLE_SIZE;
}
#endif // #if (LARGE_NETWORK_TESTING == 1)
