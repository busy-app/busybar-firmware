/***************************************************************************//**
 * @file
 * @brief EZSP Host user interface functions
 *
 * This includes command option parsing, trace and counter functions.
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
#include <string.h>
#define __USE_POSIX 1
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "stack/include/sl_zigbee_types.h"
#include "app/util/ezsp/ezsp-protocol.h"
#include "hal.h"
#include "app/util/ezsp/ezsp-enum-decode.h"
#include "app/ezsp-host/ezsp-host-common.h"
#include "app/ezsp-host/ezsp-host-priv.h"
#include "app/ezsp-host/ezsp-host-io.h"
#include "app/ezsp-host/ezsp-host-queues.h"
#include "app/ezsp-host/ezsp-host-ui.h"

//------------------------------------------------------------------------------
// Preprocessor definitions

#define ERR_LEN               128   // max length error message

//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// Command line option parsing

void sl_zigbee_ezsp_print_usage(char *name)
{
  char *shortName = strrchr(name, '/');
  shortName = shortName ? shortName + 1 : name;
  fprintf(stderr, "Usage: %s", shortName);
  fprintf(stderr, usage);
}

#ifdef __CYGWIN__
bool checkSerialPort(const char* portString, bool silent)
{
  // Not sure what we can do on Windows to check whether
  // the serial port designation is correct (RBA).
  return true;
}
#else  // POSIX OS
bool checkSerialPort(const char* portString, bool silent)
{
  struct stat buffer;
  if (0 != stat(portString, &buffer)) {
    if (!silent) {
      fprintf(stderr, "Error: No such device '%s'\n", portString);
    }
    return false;
  }

  if (!S_ISCHR(buffer.st_mode)) {
    if (!silent) {
      fprintf(stderr, "Error: '%s' is not a character device.\n", portString);
    }
    return false;
  }

  return true;
}
#endif

bool sl_zigbee_ezsp_process_command_options(int argc, char *argv[])
{
  char errStr[ERR_LEN] = "";

  if (!sl_zigbee_ezsp_internal_process_command_options(argc, argv, errStr)) {
    return false;
  }
  if (*errStr != '\0') {
    fprintf(stderr, "%s", errStr);
    sl_zigbee_ezsp_print_usage(argv[0]);
  }
  return (*errStr == '\0');
}

//------------------------------------------------------------------------------
// Error/status code to string conversion

const uint8_t* ezspErrorString(uint8_t error)
{
  switch (error) {
    case EM2XX_RESET_UNKNOWN:
      return (uint8_t *) "unknown reset";
    case EM2XX_RESET_EXTERNAL:
      return (uint8_t *) "external reset";
    case EM2XX_RESET_POWERON:
      return (uint8_t *) "power on reset";
    case EM2XX_RESET_WATCHDOG:
      return (uint8_t *) "watchdog reset";
    case EM2XX_RESET_ASSERT:
      return (uint8_t *) "assert reset";
    case EM2XX_RESET_BOOTLOADER:
      return (uint8_t *) "bootloader reset";
    case EM2XX_RESET_SOFTWARE:
      return (uint8_t *) "software reset";
    default:
      return (uint8_t *) decode_sl_zigbee_ezsp_status_t(error);
  }
} // end of ezspErrorString()

#define DATE_MAX 11 //10/10/2000 = 11 characters including NULL
#define TIME_MAX 13 //10:10:10.123 = 13 characters including NULL

void sl_zigbee_ezsp_print_elapsed_time(void)
{
  static bool first = true;
  static uint32_t base;
  uint32_t now;

  struct tm tmToday;
  time_t currentTime;
  char dateString[DATE_MAX];
  char timeString[TIME_MAX];
  time(&currentTime);
  localtime_r(&currentTime, &tmToday);
  strftime(dateString, DATE_MAX, "%m/%d/%Y", &tmToday);
  strftime(timeString, TIME_MAX, "%H:%M:%S", &tmToday);

  if (first) {
    base = halCommonGetInt32uMillisecondTick();
    first = false;
  }
  now = halCommonGetInt32uMillisecondTick() - base;
  ezspDebugPrintf("%s %s: %d.%03d ", dateString, timeString, now / 1000, now % 1000);
}

void sl_zigbee_ezsp_trace_event(const char *string)
{
  if (readConfig(traceFlags) & TRACE_EVENTS) {
    ezspDebugPrintf("%s", string);
    ezspDebugPrintf("\r\n");
  }
}

void sl_zigbee_ezsp_trace_ezsp_frame_id(const char *message, uint8_t *ezspFrame)
{
  if (readConfig(traceFlags) & TRACE_EZSP) {
    uint16_t frameId = ezspFrame[EZSP_FRAME_ID_INDEX];
    if ((ezspFrame[EZSP_EXTENDED_FRAME_CONTROL_HB_INDEX] & EZSP_EXTENDED_FRAME_FORMAT_VERSION_MASK)
        == EZSP_EXTENDED_FRAME_FORMAT_VERSION) {
      frameId |= (ezspFrame[EZSP_EXTENDED_FRAME_ID_HB_INDEX] << 8);
    }
    sl_zigbee_ezsp_print_elapsed_time();
    ezspDebugPrintf("EZSP: %s %s (%02X)\r\n",
                    message, decodeFrameId(frameId), frameId);
  }
}

void sl_zigbee_ezsp_trace_ezsp_verbose(char *format, ...)
{
  if (readConfig(traceFlags) & TRACE_EZSP_VERBOSE) {
    va_list argPointer;
    sl_zigbee_ezsp_print_elapsed_time();
    ezspDebugPrintf("EZSP: ");
    va_start(argPointer, format);
    sl_zigbee_ezsp_debug_vfprintf(format, argPointer);
    va_end(argPointer);
    ezspDebugPrintf("\r\n");
  }
}
