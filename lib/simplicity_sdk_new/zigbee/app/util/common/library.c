/***************************************************************************//**
 * @file
 * @brief Code to display or retrieve the presence or absence of
 * Ember stack libraries on the device.
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

#if defined EZSP_HOST
  #include "stack/include/sl_zigbee_types.h"
  #include "hal/hal.h"
  #include "app/util/ezsp/ezsp-protocol.h"
  #include "app/util/ezsp/ezsp.h"
  #include "stack/include/library.h"
#else
  #include "stack/include/sl_zigbee.h"
  #include "hal/hal.h"
#endif

#include "serial/serial.h"
#include "app/util/common/common.h"
#include "sl_component_catalog.h"

#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
static const char * libraryNames[] = {
  SL_ZIGBEE_LIBRARY_NAMES
};
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

#if !defined(PRO_COMPLIANCE)
uint8_t serialPort = APP_SERIAL;
#endif

void printAllLibraryStatus(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  uint8_t i = SL_ZIGBEE_FIRST_LIBRARY_ID;
  while (i < SL_ZIGBEE_NUMBER_OF_LIBRARIES) {
    sl_zigbee_library_status_t status = sl_zigbee_get_library_status(i);
    if (status == SL_ZIGBEE_LIBRARY_ERROR) {
      sl_zigbee_core_debug_println("Error retrieving info for library ID %d",
                                   i);
    } else {
      sl_zigbee_core_debug_println("%s library%s present",
                                   libraryNames[i],
                                   ((status & SL_ZIGBEE_LIBRARY_PRESENT_MASK)
                                    ? ""
                                    : " NOT"));
      if ((status & SL_ZIGBEE_LIBRARY_PRESENT_MASK) != 0U) {
        if (i == SL_ZIGBEE_ZIGBEE_PRO_LIBRARY_ID
            || i == SL_ZIGBEE_SECURITY_CORE_LIBRARY_ID) {
          sl_zigbee_core_debug_println(((status
                                         & SL_ZIGBEE_ZIGBEE_PRO_LIBRARY_HAVE_ROUTER_CAPABILITY)
                                        ? "  Have Router Support"
                                        : "  End Device Only"));
        }
        if (i == SL_ZIGBEE_PACKET_VALIDATE_LIBRARY_ID
            && (status
                & SL_ZIGBEE_LIBRARY_PRESENT_MASK)) {
          sl_zigbee_core_debug_println(((status & SL_ZIGBEE_PACKET_VALIDATE_LIBRARY_ENABLED)
                                        ? "  Enabled"
                                        : "  Disabled"));
        }
      }
    }
    (void) sli_legacy_serial_wait_send(serialPort);
    i++;
  }
}

bool isLibraryPresent(uint8_t libraryId)
{
  sl_zigbee_library_status_t status = sl_zigbee_get_library_status(libraryId);
  return (status != SL_ZIGBEE_LIBRARY_ERROR
          && (status & SL_ZIGBEE_LIBRARY_PRESENT_MASK));
}
