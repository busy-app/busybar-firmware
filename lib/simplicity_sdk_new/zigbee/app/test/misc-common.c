/***************************************************************************//**
 * @file
 * @brief Miscellaneous Network Commands such as polling, leaving,
 * radio sleep, and setting the EUI64.
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

// Ember stack and related utilities.
#ifdef EZSP_HOST
// Includes needed for ember related functions for the EZSP host
  #include "stack/include/sl_zigbee_types.h"
  #include "app/util/ezsp/ezsp-protocol.h"
  #include "app/util/ezsp/ezsp.h"
  #include "app/util/ezsp/ezsp-utils.h"
  #include "app/util/ezsp/serial-interface.h"
#else
// Includes needed for ember related functions for the EM250
  #include "stack/include/sl_zigbee.h"
  #include "stack/include/zigbee-device-stack.h" // ZigBee Device Object.
#endif // EZSP_HOST

#include "stack/internal/inc/internal-defs-patch.h"

// HAL.
#include "hal/hal.h"

// Application utilities.
#include "serial/serial.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "app/util/common/common.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "app/util/zigbee-framework/zigbee-device-library.h"

#include "misc-common.h"

//------------------------------------------------------------------------------
// External Declarations

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------
// Globals

// A poll delay for each network
uint32_t pollDelay[4] = { 0, 0, 0, 0 };

// A poll event for each network
sl_zigbee_af_event_t realPollEvent[4];
// TODO: Once we fully port the zigbee_pro_compliance app to UC, we can
// get rid of the sl_zigbee_af_event_t pointer array.
sl_zigbee_af_event_t *pollEvent[4] = {
  &realPollEvent[0],
  &realPollEvent[1],
  &realPollEvent[2],
  &realPollEvent[3]
};

//------------------------------------------------------------------------------
// Functions

void printFailedToErrorMessage(const char * message)
{
  sl_zigbee_core_debug_println("Error: Failed to %s", message);
}

//------------------------------------------------------------------------------

// It is expected that onboard applications will add the poll event to their
// list of application events.
void pollCommand(SL_CLI_COMMAND_ARG)
{
  uint8_t nwkIndex = sl_zigbee_get_current_network();
  pollDelay[nwkIndex] = sl_cli_get_argument_uint32(arguments, 0);
  pollEventHandler(nwkIndex);
}

//------------------------------------------------------------------------------

void scanCommand(SL_CLI_COMMAND_ARG)
{
  sl_status_t status;
  uint8_t page;
  uint32_t channels;
  if (sl_cli_get_argument_count(arguments) == 2) {
    page = sl_cli_get_argument_uint8(arguments, 0);
    channels = sl_cli_get_argument_uint32(arguments, 1);
  } else {
    page = 0;
    channels = SL_ZIGBEE_ALL_802_15_4_CHANNELS_MASK;
  }
  status = sl_zigbee_start_scan(SL_ZIGBEE_ACTIVE_SCAN,
                                (page << 27) | channels,
                                page ? 4 : 3);

  printCommandStatus(status, "Scanning", "Scan failed");
}
