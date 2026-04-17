/***************************************************************************//**
 * @file
 * @brief PC host files for the HAL.
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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include PLATFORM_HEADER
#include "stack/include/sl_zigbee_types.h"
#include "hal/hal.h"
#include "micro-common.h"

#if defined(EZSP_HOST) && !defined(EZSP_ASH) && !defined(EZSP_USB) && !defined(EZSP_CPC)
  #include "spi-protocol-common.h"
  #define initSpiNcp() halNcpSerialInit()
#else
  #define initSpiNcp()
#endif

static void (*microRebootHandler)(void) = NULL;

void halInit(void)
{
  initSpiNcp();
}

void halInternalAssertFailed(const char * filename, int linenumber)
{
  printf("\nFailed assert on line %d of %s, rebooting.\n", linenumber, filename);
  halReboot();
}

void setMicroRebootHandler(void (*handler)(void))
{
  microRebootHandler = handler;
}

void halReboot(void)
{
  if (microRebootHandler) {
    microRebootHandler();
  }
  printf("\nReboot not supported.  Exiting instead.\n");
  exit(0);
}

uint8_t halGetResetInfo(void)
{
  return RESET_SOFTWARE;
}

const char *halGetResetString(void)
{
  static const char *resetString = "SOFTWARE";
  return (resetString);
}

void halCommonDelayMicroseconds(uint16_t usec)
{
  if (usec == 0) {
    return;
  }

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = usec;
  assert(select(0, NULL, NULL, NULL, &timeout) >= 0);
}

void halCommonDelayMilliseconds(uint16_t msec)
{
  uint16_t cnt = msec;

  if (msec == 0) {
    return;
  }

  while (cnt-- > 0)
    halCommonDelayMicroseconds(1000);
}
