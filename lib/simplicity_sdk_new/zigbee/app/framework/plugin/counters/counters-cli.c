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

#include "app/framework/include/af.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "app/util/common/common.h"
#include "counters.h"
#include "counters-ota.h"
#include "counters-cli.h"

#include "app/util/serial/sl_zigbee_command_interpreter.h"

const char * titleStrings[] = {
  SL_ZIGBEE_COUNTER_STRINGS
};
const char * unknownCounter = "???";
static void sl_zigbee_af_counters_print(void);

void sl_zigbee_af_counters_print_command(sl_cli_command_arg_t *args)
{
  UNUSED_VAR(args);
#if defined(EZSP_HOST)
  sl_zigbee_ezsp_read_and_clear_counters(sl_zigbee_counters);
#else
  sl_zigbee_read_and_clear_counters(sl_zigbee_counters, SL_ZIGBEE_COUNTER_TYPE_COUNT);
#endif

  sl_zigbee_af_counters_print();
}

void sl_zigbee_af_counters_print_counters_type_command(sl_cli_command_arg_t *args)
{
#if !defined(EZSP_HOST)
  uint8_t counterType = sl_cli_get_argument_uint8(args, 0);
  if (counterType < SL_ZIGBEE_COUNTER_TYPE_COUNT) {
    sl_zigbee_af_core_println("%u) %s: %u",
                              counterType,
                              (titleStrings[counterType] == NULL
                               ? unknownCounter
                               : titleStrings[counterType]),
                              sl_zigbee_counters[counterType]);
  }
#else
  UNUSED_VAR(args);
#endif
}

void sl_zigbee_af_counters_simple_print_command(sl_cli_command_arg_t *args)
{
  UNUSED_VAR(args);
#if defined(EZSP_HOST)
  sl_zigbee_ezsp_read_counters(sl_zigbee_counters);
#else
  sl_zigbee_read_counters(sl_zigbee_counters, SL_ZIGBEE_COUNTER_TYPE_COUNT);
#endif

  sl_zigbee_af_counters_print();
}

void sl_zigbee_af_counters_send_request_command(sl_cli_command_arg_t *args)
{
#if defined(OTA_COUNTERS_ENABLED)
  sl_zigbee_af_counters_send_request(sl_cli_get_argument_uint16(args, 0),
                                     sl_cli_get_argument_uint8(args, 1));
#else
  UNUSED_VAR(args);
#endif
}

void sl_zigbee_af_counters_set_threshold_command(sl_cli_command_arg_t *args)
{
  sl_zigbee_counter_type_t type = sl_cli_get_argument_uint8(args, 0);
  uint16_t threshold = sl_cli_get_argument_uint16(args, 1);
  sl_zigbee_af_core_println("Setting Threshold command");
  sl_zigbee_af_counters_set_threshold(type, threshold);
}

void sl_zigbee_af_counters_print_thresholds_command(sl_cli_command_arg_t *args)
{
  UNUSED_VAR(args);
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_COUNTER_TYPE_COUNT; i++) {
    sl_zigbee_af_core_println("%u) %s: %u",
                              i,
                              (titleStrings[i] == NULL
                               ? unknownCounter
                               : titleStrings[i]),
                              sl_zigbee_counters_thresholds[i]);
  }
}

static void sl_zigbee_af_counters_print(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_COUNTER_TYPE_COUNT; i++) {
    sl_zigbee_af_core_println("%u) %s: %u",
                              i,
                              (titleStrings[i] == NULL
                               ? unknownCounter
                               : titleStrings[i]),
                              sl_zigbee_counters[i]);
  }
}

void sl_zigbee_af_counters_clear_command(sl_cli_command_arg_t *args)
{
  UNUSED_VAR(args);
  sl_zigbee_af_counters_clear();
}

void sl_zigbee_af_counters_reset_thresholds_command(sl_cli_command_arg_t *args)
{
  UNUSED_VAR(args);
  sl_zigbee_af_counters_reset_thresholds();
}
