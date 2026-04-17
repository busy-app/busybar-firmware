/***************************************************************************//**
 * @file
 * @brief
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
#include "stack/include/sl_zigbee_types.h"
#include "serial/serial.h"
#include "sl_zigbee_debug_print.h"
#include "pro-compliance-printing.h"

extern uint8_t serialPort;
extern bool proComplianceCorePrintingEnabled;
static void proComplianceCorePrintInternalVarArg(bool newLine, const char * formatString, va_list ap) __attribute__((format(printf, 2, 0)));

static void proComplianceCorePrintInternalVarArg(bool newLine,
                                                 const char * formatString,
                                                 va_list ap)
{
  if (false == proComplianceCorePrintingEnabled) {
    return;
  }
  (void) local_vprintf(formatString, ap);

  if (newLine) {
    sl_zigbee_core_debug_print("\n");
  }
}

void sl_zigbee_af_core_print(const char * formatString, ...)
{
  va_list ap = { 0 };
  va_start(ap, formatString);
  proComplianceCorePrintInternalVarArg(false, formatString, ap);
  va_end(ap);
}

void sl_zigbee_af_core_println(const char * formatString, ...)
{
  va_list ap = { 0 };
  va_start(ap, formatString);
  proComplianceCorePrintInternalVarArg(true, formatString, ap);
  va_end(ap);
}

#if !defined (SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)
void sl_zigbee_af_print_big_endian_eui64(const sl_802154_long_addr_t eui64)
{
  sl_zigbee_core_debug_print("(%c)%02X%02X%02X%02X%02X%02X%02X%02X",
                             '>',
                             eui64[7],
                             eui64[6],
                             eui64[5],
                             eui64[4],
                             eui64[3],
                             eui64[2],
                             eui64[1],
                             eui64[0]);
}
#endif
