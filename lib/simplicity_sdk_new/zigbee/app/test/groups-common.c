/***************************************************************************//**
 * @file
 * @brief Functionality for manipulating APS/NWK groups that is common to all
 * platforms (250, 260, 2420).
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
#endif // EZSP_HOST
#include "hal/hal.h"
#include "serial/serial.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "app/util/common/common.h"

#include "app/test/groups-common.h"

//------------------------------------------------------------------------------
// Globals

const char * endPointZeroReserved = "Endpoint 0 is reserved.";

//------------------------------------------------------------------------------
// Forward Declarations

static bool getGroupAndPrintFailure(uint8_t index,
                                    sl_zigbee_multicast_table_entry_t* entry);
static bool endpointZeroErrorHandled(uint8_t endpoint);

//------------------------------------------------------------------------------
// Functions

void displayGroupsCommand(SL_CLI_COMMAND_ARG)
{
#ifdef SL_CATALOG_CLI_PRESENT
  UNUSED_VAR(arguments);
#endif // SL_CATALOG_CLI_PRESENT
  uint8_t i;
  bool groupsExist = false;
  uint8_t tableSize = getGroupTableSize();
  sl_zigbee_multicast_table_entry_t entry;
  if ( tableSize == 0xFF ) {
    return;
  }
  for (i = 0; i < tableSize; i++ ) {
    if ( !getGroupAndPrintFailure(i, &entry) ) {
      return;
    }
    if ( entry.endpoint != 0 ) {
      sl_zigbee_core_debug_println("Endpoint 0x%02X belongs to group 0x%02x",
                                   entry.endpoint,
                                   entry.multicastId);
      (void) sli_legacy_serial_wait_send(serialPort);
      groupsExist = true;
    }
  }

  if ( groupsExist == false ) {
    sl_zigbee_core_debug_println("No group associations exist.");
  }

  return;
}

//------------------------------------------------------------------------------

void clearGroupsCommand(SL_CLI_COMMAND_ARG)
{
#ifdef SL_CATALOG_CLI_PRESENT
  UNUSED_VAR(arguments);
#endif // SL_CATALOG_CLI_PRESENT
  printCommandStatus((initializeGroupsTable() == true
                      ? SL_STATUS_OK
                      : SL_STATUS_FAIL),
                     "Cleared",
                     "Failed.");
  (void) sli_legacy_serial_wait_send(serialPort);
}

//------------------------------------------------------------------------------
// Command arguments: Endpoint, Group

void addEndpointToGroupCommand(SL_CLI_COMMAND_ARG)
{
  uint8_t i;
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  uint16_t group = sl_cli_get_argument_uint16(arguments, 1);
  bool groupAdded = false;
  uint8_t tableSize = getGroupTableSize();

  if ( tableSize == 0xFF
       || endpointZeroErrorHandled(endpoint) ) {
    return;
  }

  for (i = 0; i < tableSize && groupAdded == false; i++ ) {
    sl_zigbee_multicast_table_entry_t entry;
    if ( !getGroupAndPrintFailure(i, &entry) ) {
      return;
    }
    if ( entry.endpoint != 0 ) {
      continue;
    }

    entry.endpoint = endpoint;
    entry.multicastId = group;
    entry.networkIndex = sl_zigbee_get_current_network();

    if ( !setGroup(i, &entry) ) {
      sl_zigbee_core_debug_println("Failed to %s group at index %d", "set", i);
      // keep going
    }
    groupAdded = true;
  }

  if ( groupAdded != true ) {
    printErrorMessage("No room in group table.");
  }

  (void) sli_legacy_serial_wait_send(serialPort);
  return;
}

//------------------------------------------------------------------------------
// Command Arguments: Endpoint, Group

void removeEndpointFromGroupCommand(SL_CLI_COMMAND_ARG)
{
  uint8_t i;
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  uint16_t group = sl_cli_get_argument_uint16(arguments, 1);
  bool groupRemoved = false;
  uint8_t tableSize = getGroupTableSize();

  if ( tableSize == 0xFF
       || endpointZeroErrorHandled(endpoint) ) {
    return;
  }

  for (i = 0; i < tableSize && groupRemoved == false; i++ ) {
    sl_zigbee_multicast_table_entry_t entry;
    if ( !getGroupAndPrintFailure(i, &entry) ) {
      return;
    }
    if ( entry.endpoint != endpoint
         || entry.multicastId != group ) {
      continue;
    }

    if ( !eraseGroup(i) ) {
      sl_zigbee_core_debug_println("Failed to %s group at index %d", "erase", i);
      // keep going
    }
    groupRemoved = true;
  }

  if ( groupRemoved != true ) {
    printErrorMessage("No endpoint-group association found.");
  }

  (void) sli_legacy_serial_wait_send(serialPort);
  return;
}

//------------------------------------------------------------------------------
// Lookup the endpoint for a multicast.

bool lookupGroupEndpoint(uint8_t endpoint, sl_zigbee_multicast_id_t groupId)
{
  uint8_t i;
  uint8_t tableSize = getGroupTableSize();
  if ( tableSize == 0xFF ) {
    return false;
  }

  for (i = 0; i < tableSize; i++) {
    sl_zigbee_multicast_table_entry_t entry;
    if ( !getGroup(i, &entry) ) {
      continue;  // error, but keep going
    }
    if (entry.multicastId == groupId
        && entry.endpoint == endpoint) {
      return true;
    }
  }
  return false;
}
//------------------------------------------------------------------------------

static bool getGroupAndPrintFailure(uint8_t index,
                                    sl_zigbee_multicast_table_entry_t* entry)
{
  if ( !getGroup(index, entry) ) {
    sl_zigbee_core_debug_println("Failed to get entry %d", index);
    return false;
  }
  return true;
}

//------------------------------------------------------------------------------

static bool endpointZeroErrorHandled(uint8_t endpoint)
{
  if ( endpoint == 0 ) {
    printErrorMessage(endPointZeroReserved);
    (void) sli_legacy_serial_wait_send(serialPort);
    return true;
  }
  return false;
}
