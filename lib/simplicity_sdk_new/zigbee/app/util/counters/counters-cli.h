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

#ifndef SILABS_APP_UTIL_COUNTERS_CLI_H
#define SILABS_APP_UTIL_COUNTERS_CLI_H

void printCountersCommand(void);
void simplePrintCountersCommand(void);
void clearCountersCommand(void);
void setCounterThreshold(void);
void resetCounterThresholds(void);
void printCounterThresholdsCommand(void);
/** Args: destination id, clearCounters (bool) */
void sendCountersRequestCommand(SL_CLI_COMMAND_ARG);

/** Utility function for printing out the OTA counters response. */
void printCountersResponse(uint8_t messageLength, uint8_t *message);

/** Use this macro in the sl_zigbee_command_table for convenience.
 * This command requests counters over the air from a remote node.
 */
#define OTA_COUNTER_COMMANDS                                                  \
  sl_zigbee_command_entry_action("cnt_req", sendCountersRequestCommand, "vu", \
                                 "Request stack counters from remote device"),

#endif // SILABS_APP_UTIL_COUNTERS_CLI_H
