/***************************************************************************//**
 * @file
 * @brief Used for testing the counters library via a command line interface.
 * For documentation on the counters library see counters.h.
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

#include PLATFORM_HEADER

#if !defined(EZSP_HOST)
#include "stack/include/sl_zigbee.h"
#else
#include "stack/include/sl_zigbee_types.h"
#endif

#include "hal/hal.h"
#include "serial/serial.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "app/util/common/common.h"
#include "app/util/counters/counters.h"
#include "app/util/counters/counters-ota.h"

#if defined(EZSP_HOST)
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp.h"
#endif

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

void clearCountersCommand(void)
{
#if !defined(EZSP)
  sli_zigbee_stack_clear_counters();
#else
  sli_legacy_serial_print_line("Not supported.  Counters cleared automatically when retrieved by host.");
#endif
}

#if !defined(SL_CATALOG_ZIGBEE_COUNTERS_PRESENT)
const char * titleStrings[] = {
  SL_ZIGBEE_COUNTER_STRINGS
};

const char * unknownCounter = "???";
#else
extern const char * titleStrings[];
extern const char * unknownCounter;
#endif // SL_CATALOG_ZIGBEE_COUNTERS_PRESENT

void printCountersCommand(void)
{
  uint8_t i = 0;

#if defined(EZSP_HOST)
  sl_zigbee_ezsp_read_and_clear_counters(sli_zigbee_counters);
#else
  sl_zigbee_read_and_clear_counters(sli_zigbee_counters, SL_ZIGBEE_COUNTER_TYPE_COUNT);
#endif

  while ( i < SL_ZIGBEE_COUNTER_TYPE_COUNT ) {
    uint16_t data = sli_zigbee_counters[i];
    sl_zigbee_core_debug_println("%d) %s: %d",
                                 i,
                                 (titleStrings[i] == NULL
                                  ? unknownCounter
                                  : titleStrings[i]),
                                 data);
    (void) sli_legacy_serial_wait_send(serialPort);
    i++;
  }
}

// For applications short on const space.
void simplePrintCountersCommand(void)
{
  uint8_t i;
  for ( i = 0; i < SL_ZIGBEE_COUNTER_TYPE_COUNT; i++ ) {
    sl_zigbee_core_debug_println("%d: %d",
                                 i,
                                 sli_zigbee_counters[i]);
    (void) sli_legacy_serial_wait_send(serialPort);
  }
}

void sendCountersRequestCommand(SL_CLI_COMMAND_ARG)
{
  sl_zigbee_send_counters_request(sl_cli_get_argument_uint16(arguments, 0),
                                  (bool)sl_cli_get_argument_uint32(arguments, 1));
}

#ifdef SL_ZIGBEE_APPLICATION_HAS_COUNTER_ROLLOVER_HANDLER

void setCounterThreshold(void)
{
  uint16_t type = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t threshold = sl_cli_get_argument_uint8(arguments, 1);
  sli_zigbee_stack_set_counter_threshold(type, threshold);
}

void resetCounterThresholds(void)
{
  sli_zigbee_stack_reset_counters_thresholds();
}

void printCounterThresholdsCommand(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_COUNTER_TYPE_COUNT; i++) {
    sl_zigbee_core_debug_println("Counter %u Threshold %u\r\n", i, sli_zigbee_counters_thresholds[i]);
  }
}

#endif

#if !defined(EZSP_HOST)

void printCountersResponse(uint8_t messageLength, uint8_t *message)
{
  uint8_t i;
  for (i = 0; i < messageLength; i += 3) {
    sl_zigbee_core_debug_println("%d: %d",
                                 message[i],
                                 sl_util_fetch_low_high_int16u(message + i + 1));
    (void) sli_legacy_serial_wait_send(serialPort);
  }
}

#endif
