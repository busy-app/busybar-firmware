#include "stack/include/sl_zigbee.h"
#include "hal.h"
#include <stdio.h>

//----------------------------------------------------------------
// From tool/simulator/child/child-main.c

// debugPrintTextAndHex, vSimPrint, simPrint, scriptTestCheckpoint and
// debugSimPrint are from child-main.c. However, they are referenced
// from many other zigbee stack source files. Once SL_ZIGBEE_TEST is removed,
// sl_zigbee_stack.h will define an empty macro to allow compilation.
// sendLogEvent will go away after SL_ZIGBEE_TEST is removed

#ifndef UNUSED_VAR
#define UNUSED_VAR(x) (void)(x)
#endif

#ifdef SL_ZIGBEE_TEST
uint16_t simulatorId;
uint16_t rebootCount = 0;
bool quiet = true;

void debugPrintTextAndHex(const char* text,
                          const uint8_t* hexData,
                          uint8_t length,
                          uint8_t spaceEveryXChars,
                          bool finalCr)
{
  UNUSED_VAR(text);
  UNUSED_VAR(hexData);
  UNUSED_VAR(length);
  UNUSED_VAR(spaceEveryXChars);
  UNUSED_VAR(finalCr);
}

void vSimPrint(char *format, va_list argPointer)
{
  if (quiet) {
    return;
  }
  fprintf(stderr, "[%f ", halCommonGetInt32uMillisecondTick() / 1000.0);
  if (rebootCount == 0) {
    fprintf(stderr, "%02X: ", simulatorId);
  } else {
    fprintf(stderr, "%02X.%d: ", simulatorId, rebootCount);
  }
  vfprintf(stderr, format, argPointer);
  putc(']', stderr);
  putc('\n', stderr);
}

bool usingScript = true;

void simPrint(char* format, ...)
{
  va_list argPointer;
  va_start(argPointer, format);
  vSimPrint(format, argPointer);
  va_end(argPointer);
}

void debugSimPrint(char* format, ...)
{
  if (usingScript) {
    va_list argPointer;
    va_start(argPointer, format);
    vSimPrint(format, argPointer);
    va_end(argPointer);
  }
}

void scriptTestCheckpoint(char* string, ...)
{
  UNUSED_VAR(string);
}

// Referenced from sli_zigbee_report_link_costs in neighbor.c
void sendLogEvent(char *types, char *data)
{
  UNUSED_VAR(types);
  UNUSED_VAR(data);
}

#endif // SL_ZIGBEE_TEST

#ifdef SL_ZIGBEE_TEST

void simulatedTimePasses(void)
{
}

#endif

#include "sl_status.h"
#include "sl_iostream.h"

// Referenced from sli_zigbee_source_route_update_event_handler in source-route-update.c
sl_status_t sli_legacy_serial_printf_line(uint8_t port, const char *formatString, ...)
{
  UNUSED_VAR(port);
  UNUSED_VAR(formatString);
  return 0;
}

void sl_zigbee_set_or_get_ezsp_token_command_handler(bool isSet)
{
  UNUSED_VAR(isSet);
}
// Referenced from stream_putchar in iostream_printf.o
//   from putchar in iostream_printf.o
WEAK(sl_status_t sl_iostream_putchar(sl_iostream_t *stream, char c))
{
  UNUSED_VAR(stream);
  UNUSED_VAR(c);
  //sl_status_t status = sli_legacy_serial_write_byte(APP_SERIAL, c);
  sl_status_t status = SL_STATUS_OK;
  if (status == SL_STATUS_OK) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_FAIL;
  }
}
