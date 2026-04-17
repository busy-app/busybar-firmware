/***************************************************************************//**
 * @file
 * @brief ZDO commands for the Compliance application.  Common to both the 260 host
 * and onboard (250/2420) versions of the application.
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
  #include "stack/include/message.h"
  #include "app/util/ezsp/ezsp-protocol.h"
  #include "app/util/ezsp/ezsp.h"
  #include "app/util/ezsp/ezsp-utils.h"
  #include "app/util/ezsp/serial-interface.h"
#else
// Includes needed for ember related functions for the EM250
  #include "stack/include/sl_zigbee.h"
#endif // EZSP_HOST
#include "stack/internal/inc/internal-defs-patch.h"
#include "hal/hal.h"
#include "serial/serial.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "app/util/common/common.h"

#include "app/test/pro-compliance-common.h"
#include "app/test/test-profile.h"

#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "app/util/zigbee-framework/zigbee-device-library.h"

#define ZDO_REQUEST_PAYLOAD_LEN   2   // target short ID
#define ZDO_MAX_PAYLOAD_LEN       (255                    \
                                   - ZDO_MESSAGE_OVERHEAD \
                                   - ZDO_REQUEST_PAYLOAD_LEN)
//------------------------------------------------------------------------------
// Globals
bool debugPrintLqiTableResponse = TRUE;
//------------------------------------------------------------------------------
// Forward Declarations

static void printBindingTableEntry(sl_zigbee_binding_table_entry_t* entry,
                                   sl_802154_long_addr_t sourceEui64,
                                   uint8_t index,
                                   uint8_t totalEntries);

//------------------------------------------------------------------------------
// Functions

void sendBindUnbindCommand(sl_cli_command_arg_t *arguments)
{
  // target, source EUI64, source endpoint, dest EUI64,
  // dest endpoint OR group address, clusterID
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_long_addr_t sourceEui64;
  uint8_t sourceEndpoint      = sl_cli_get_argument_uint8(arguments, 2);
  sl_802154_long_addr_t destinationEui64;
  uint16_t destEpOrGroupAddr = sl_cli_get_argument_uint16(arguments, 4);
  uint16_t clusterId          = sl_cli_get_argument_uint16(arguments, 5);
  uint8_t type = UNICAST_BINDING;
  uint8_t length;
  uint8_t *command = sl_zigbee_cli_get_argument_string_and_length(arguments, -1, &length);

  bool useEncryption = false;
  if (sl_cli_get_argument_count(arguments) > 6) {
    useEncryption = (bool) sl_cli_get_argument_uint8(arguments, 6);
  }

  sl_zigbee_copy_eui64_arg(arguments, 1, sourceEui64, false);
  sl_zigbee_copy_eui64_arg(arguments, 3, destinationEui64, false);

  if (command[5] == 'g') {
    // A.k.a. Group Bind/Unbind request
    type = MULTICAST_BINDING;
  }
  if (command[5] == 'b'
      || (command[5] == 'g' && command[7] == 'b')) {
    sl_zigbee_bind_request(target,
                           sourceEui64,
                           sourceEndpoint,
                           clusterId,
                           type,
                           destinationEui64,
                           (type == MULTICAST_BINDING
                            ? destEpOrGroupAddr
                            : 0),
                           (type == UNICAST_BINDING
                            ? ((uint8_t)destEpOrGroupAddr)
                            : 0),
                           SL_ZIGBEE_APS_OPTION_RETRY
                           | (useEncryption ? SL_ZIGBEE_APS_OPTION_ENCRYPTION : SL_ZIGBEE_APS_OPTION_NONE));
  } else {
    sl_zigbee_unbind_request(target,
                             sourceEui64,
                             sourceEndpoint,
                             clusterId,
                             type,
                             destinationEui64,
                             (type == MULTICAST_BINDING
                              ? destEpOrGroupAddr
                              : 0),
                             (type == UNICAST_BINDING
                              ? ((uint8_t)destEpOrGroupAddr)
                              : 0),
                             SL_ZIGBEE_APS_OPTION_RETRY
                             | (useEncryption ? SL_ZIGBEE_APS_OPTION_ENCRYPTION : SL_ZIGBEE_APS_OPTION_NONE));
  }
}

//------------------------------------------------------------------------------

void eraseBindingsCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_status_t status = sl_zigbee_clear_binding_table();
  printCommandStatusWithPrefix(status,
                               "Binding table",
                               "erased",
                               "Erase failed");
}

//------------------------------------------------------------------------------
// Send a Bind Table Request to ourselves that our incoming message handler
// should handle by printing each entry.

static void printBindingTableHeader(void)
{
  sl_zigbee_core_debug_println("Binding table");
  sl_zigbee_core_debug_println("        Source           EP        Dest             EP");
  (void) sli_legacy_serial_wait_send(serialPort);
}

void remoteBindingsCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t id = sl_cli_get_argument_uint16(arguments, 0);
  sl_status_t status;
  printBindingTableHeader();
  status = sl_zigbee_binding_table_request(id,
                                           0,
                                           SL_ZIGBEE_APS_OPTION_NONE);
  printCommandStatusWithPrefix(status,
                               "Binding table",   // prefix
                               NULL,              // success text
                               "request failed"); // fail text
}

extern sl_802154_long_addr_t sli_802154mac_local_eui64;

void printBindingsCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  uint8_t i;
  sl_zigbee_binding_table_entry_t entry;
  uint8_t entries = 0;
  printBindingTableHeader();
  for (i = 0; i < sl_zigbee_get_binding_table_size(); i++) {
    if (SL_STATUS_OK == sl_zigbee_get_binding(i, &entry)
        && entry.type != SL_ZIGBEE_UNUSED_BINDING) {
      entries++;
      printBindingTableEntry(&entry, sli_802154mac_local_eui64, i, sl_zigbee_get_binding_table_size());
    }
  }
  sl_zigbee_core_debug_println("%d/%d entries",
                               entries,
                               sl_zigbee_get_binding_table_size());
}

//------------------------------------------------------------------------------
// Send arbitrary ZDO messages that do not have specific CLI commands to do so.
// Messages may not be formatted correctly because we assume that the ZDO
// message needs only a clusterId and target.

void zdoRequestCommand(sl_cli_command_arg_t *arguments)
{
  uint16_t clusterId = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 1);
  uint8_t contents[ZDO_MESSAGE_OVERHEAD
                   + ZDO_REQUEST_PAYLOAD_LEN
                   + ZDO_MAX_PAYLOAD_LEN];

  // payload[0] (sequence number) is written by sl_zigbee_send_zig_dev_request
  uint8_t contentsLen = ZDO_MESSAGE_OVERHEAD;

  // Write the target in the ZDO request payload
  uint8_t *payload = contents + ZDO_MESSAGE_OVERHEAD;
  payload[0] = LOW_BYTE(target);
  payload[1] = HIGH_BYTE(target);
  payload += ZDO_REQUEST_PAYLOAD_LEN;
  contentsLen += ZDO_REQUEST_PAYLOAD_LEN;

  // Write any additional, optional payload
  if (sl_cli_get_argument_count(arguments) >= 3) {
    contentsLen += sl_zigbee_copy_hex_arg(arguments, 2, payload, ZDO_MAX_PAYLOAD_LEN, false);
  }

  // Send the ZDO request
  sl_zigbee_send_zig_dev_request(target,
                                 clusterId,
                                 0,
                                 contents,
                                 contentsLen);
}

//------------------------------------------------------------------------------

#define ZDO_LEAVE_LENGTH 9

void sendZdoLeave(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 0);
  // CCB 2047
  // - CCB makes the first step to depracate the 'leave and remove children' functionality.
  // - We were proactive here and deprecated it right away.
  // bool removeChildren = (bool)sl_cli_get_argument_uint32(arguments, 1);
  bool rejoinNetwork = (bool)sl_cli_get_argument_uint32(arguments, 2);
  sl_zigbee_aps_option_t options = sl_cli_get_argument_uint16(arguments, 3);
  sl_802154_long_addr_t eui64;
  uint8_t flags = 0;

  // Per the spec, the EUI64 should be set to zero when this is a request.
  // It makes no sense, I know.
  memset(eui64, 0, EUI64_SIZE);

  if (rejoinNetwork) {
    flags |= LEAVE_REQUEST_REJOIN_FLAG;
  }
  sl_zigbee_leave_request(target, eui64, flags, options);
}

void sendZdoLeaveRequest(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_long_addr_t targetEui;
  uint8_t flags = sl_cli_get_argument_uint8(arguments, 2);
  sl_zigbee_aps_option_t options = sl_cli_get_argument_uint16(arguments, 3);
  sl_zigbee_copy_eui64_arg(arguments, 1, targetEui, false);

  sl_zigbee_leave_request(target, targetEui, flags, options);
}

//------------------------------------------------------------------------------
void serverRequestCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t mask[3];  // 1 byte for Sequence Number, which will be filled in
                    //   by sl_zigbee_send_zig_dev_request()
                    // 2 bytes for Server Mask
  sl_util_store_low_high_int16u(mask + 1, sl_cli_get_argument_uint16(arguments, 1));
  sl_zigbee_send_zig_dev_request(target,
                                 SYSTEM_SERVER_DISCOVERY_REQUEST,
                                 0, // APS options
                                 mask,
                                 3); // length
}

//------------------------------------------------------------------------------

void sendSimpleRequestCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 1);
  sl_zigbee_simple_descriptor_request(target, endpoint, 0);
}

//------------------------------------------------------------------------------

static void printBindingTableEntry(sl_zigbee_binding_table_entry_t* entry,
                                   sl_802154_long_addr_t sourceEui64,
                                   uint8_t index,
                                   uint8_t totalEntries)
{
  sl_zigbee_binding_type_t type = (entry->type & 0x1F);

  sl_zigbee_core_debug_print("%d/%d: ", index + 1, totalEntries);
  printLittleEndianEui64(serialPort, sourceEui64);

  // Source Endpoint
  sl_zigbee_core_debug_print(" %d -> ", entry->local);

  if ( type == SL_ZIGBEE_UNICAST_BINDING ) {
    // Destination has IEEE Address and Dest Endpoint
    printLittleEndianEui64(serialPort, entry->identifier);
    sl_zigbee_core_debug_print(" %d ", entry->remote);
  } else if (type == SL_ZIGBEE_MULTICAST_BINDING) {
    // Destination has Group Address and no Dest Endpoint
    sl_zigbee_core_debug_print(" 0x%02X%02X             ",
                               entry->identifier[1],
                               entry->identifier[0]);
    sl_zigbee_core_debug_print(" - ");
  } else {
    sl_zigbee_core_debug_println("Unknown binding type %d", entry->type);
    return;
  }
  sl_zigbee_core_debug_println(" (cluster 0x%04X)",
                               entry->clusterId);

  (void) sli_legacy_serial_wait_send(serialPort);
}

//------------------------------------------------------------------------------
// This takes care of any resposes to ZDO binding table requests sent
// by the app.
// The function returns true if the cluster ID was recognized and handled,
// false otherwise.

bool handleZdoClusterMessage(sl_zigbee_aps_frame_t *apsFrame,
                             sl_zigbee_rx_packet_info_t *packetInfo,
                             uint8_t messageLength,
                             uint8_t *message)
{
  if (apsFrame->clusterId >= CLUSTER_ID_RESPONSE_MINIMUM) {
    // ZDO responses start with a status byte
    uint8_t status = message[1];
    sl_zigbee_core_debug_println("ZDO response 0x%04x (status 0x%02x)", apsFrame->clusterId, status);
  }
  switch (apsFrame->clusterId) {
    case BINDING_TABLE_RESPONSE: {
      uint8_t status = message[1];
      uint8_t i;
      uint8_t finger = 5;
      if (status == SL_ZIGBEE_ZDP_SUCCESS) {
        uint8_t entries = message[2];
        uint8_t start = message[3];
        uint8_t count = message[4];
        // Maximum size for one entry is 21 bytes for Long Address mode (0x03)
        // IEEE Dest Address + Dest Endpoint present
        uint8_t entry[21];
        sl_zigbee_binding_table_entry_t binding;
        sl_802154_long_addr_t sourceEui64;
        if (count == 0) {
          printErrorMessage2("Binding table", "response contained no entries.");
          return true;
        }

        for (i = 0; i < count; i++) {
          uint8_t dstAddrMode = message[finger + 11];
          uint8_t entrySize = (dstAddrMode == 0x1) ? 14 : 21;

          memmove(entry, message + finger, entrySize);
          memmove(sourceEui64, entry, EUI64_SIZE);
          binding.type = (entry[11] == 0x03
                          ? SL_ZIGBEE_UNICAST_BINDING
                          : SL_ZIGBEE_MULTICAST_BINDING);
          binding.local = entry[8];
          binding.clusterId = (((uint16_t)entry[10]) << 8) + entry[9];
          binding.remote = entry[20];
          memmove(binding.identifier, entry + 12, EUI64_SIZE);
          printBindingTableEntry(&binding, sourceEui64, start + i, entries);
          finger += entrySize;
        }
        if (start + count < entries) {
          sl_zigbee_binding_table_request(packetInfo->sender_short_id,
                                          start + count,
                                          SL_ZIGBEE_APS_OPTION_NONE);
        } else {
          sl_zigbee_core_debug_println("Entries in use: %d / %d",
                                       entries,
                                       sl_zigbee_get_binding_table_size());
        }
      }
      break;
    }

    case NETWORK_ADDRESS_RESPONSE:
    case IEEE_ADDRESS_RESPONSE: {
      sl_802154_long_addr_t eui64;
      sl_802154_short_addr_t nodeId
        = sl_zigbee_decode_address_response(messageLength, message, eui64);
      sl_zigbee_core_debug_print("Address response from %04X: %04X = ",
                                 packetInfo->sender_short_id,
                                 nodeId);
      printLittleEndianEui64(serialPort, eui64);
      printCarriageReturn();
      break;
    }
    case LQI_TABLE_RESPONSE: {
      sl_zigbee_core_debug_print("LQI table response from %04X: ",
                                 packetInfo->sender_short_id);
      // switch on status value
      uint8_t status = message[1];
      if (status == SL_ZIGBEE_ZDP_NOT_SUPPORTED) {
        sl_zigbee_core_debug_println("(NOT_SUPPORTED)");
      } else if (status == SL_ZIGBEE_ZDP_SUCCESS) {
        uint8_t tableCount, startIndex, listCount;
        tableCount = message[2];
        startIndex = message[3];
        listCount  = message[4];
        sl_zigbee_core_debug_println("(SUCCESS)");
        sl_zigbee_core_debug_print("Neighbor table entries: %d, ",
                                   tableCount);
        sl_zigbee_core_debug_println("msg start index: %d, msg entry count: %d",
                                     startIndex, listCount);
        if (debugPrintLqiTableResponse) {
          sl_zigbee_core_debug_println("Device/OutOf  Address  NodeType  Relation");
          sl_zigbee_core_debug_println("-----------------------------------------");
          uint16_t nwkAddress;
          uint8_t nodeTypeBits, relationshipBits;
          uint8_t byte16, byte17, byte18;
          int offset;   // offset to first entry in list
          char *type, *relation;  // printed chars
          for (int i = 0; i < listCount; i++) {
            offset = 5 + (22 * i);   // Each table entry is 22 bytes
            byte16 = message[offset + 16];
            byte17 = message[offset + 17];
            byte18 = message[offset + 18];
            nwkAddress = (byte17 << 8) | byte16; // combining two bytes into one int
            nodeTypeBits = (byte18 & 0x03); // mask byte18 to get type field
            relationshipBits = (byte18 & 0x70) >> 4; // same as above for relationship
            // determining type field value
            switch (nodeTypeBits) {
              case 0x00:    // ZC
                type = "ZC";
                break;
              case 0x01:    // ZR
                type = "ZR";
                break;
              case 0x02:    // ZED
                type = "ZE";
                break;
              case 0x03:    // Unknown
              default:
                type = "?";
                break;
            }
            // determining the relationship field value
            switch (relationshipBits) {
              case 0x00:
                relation = "P";  // Parent
                break;
              case 0x01:
                relation = "C";  // Child
                break;
              case 0x02:
                relation = "S";  // Sibling
                break;
              case 0x04:
                relation = "PC"; // Previous Child
                break;
              case 0x03:        // None of the above
              default:
                relation = "-";
                break;
            }
            sl_zigbee_core_debug_println("(%d/%d)      %04X,   %s,   %s",
                                         i + startIndex + 1,
                                         tableCount,
                                         nwkAddress,
                                         type,
                                         relation);
          }
        }
      } else {
        sl_zigbee_core_debug_println("(ERROR - status code %02X)",
                                     status);
      }
      break;
    }
    case PARENT_ANNOUNCE: {
      uint8_t children = message[1];
      sl_zigbee_core_debug_println("Parent Announce from 0x%04X: children %d",
                                   packetInfo->sender_short_id,
                                   children);
      break;
    }
    default:
      return false;
  }

  return true;
}

//------------------------------------------------------------------------------
// 'list' has a string of big endian binary cluster IDs:  0x00 0x54 0xE0 0x00.
// Modifies list to contain little endian cluster IDs: 0x54 0x00 0x00 x0E0.

static void prepareClusterList(uint8_t *list, uint8_t length)
{
  for (uint8_t i = 0; i < length; i += 2) {
    uint8_t temp = list[i];
    list[i] = list[i + 1];
    list[i + 1] = temp;
  }
}

//------------------------------------------------------------------------------

void sendMatchRequestCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t inLength, outLength;
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 0);
  uint16_t profile = sl_cli_get_argument_uint16(arguments, 1);
  uint8_t inClusterList[ZDO_MAX_PAYLOAD_LEN] = { 0 };
  uint8_t outClusterList[ZDO_MAX_PAYLOAD_LEN] = { 0 };
  inLength = sl_zigbee_copy_hex_arg(arguments, 2, inClusterList, ZDO_MAX_PAYLOAD_LEN, false);
  outLength = sl_zigbee_copy_hex_arg(arguments, 3, outClusterList, ZDO_MAX_PAYLOAD_LEN, false);
  sl_zigbee_aps_option_t opts = 0;
  if (sl_cli_get_argument_count(arguments) > 4) {
    opts = sl_cli_get_argument_uint16(arguments, 4);
  }
  uint8_t inCount = inLength >> 1;
  uint8_t outCount = outLength >> 1;
  prepareClusterList(inClusterList, inLength);
  prepareClusterList(outClusterList, outLength);

  sl_zigbee_match_descriptors_request(target,
                                      profile,
                                      inCount,
                                      (uint16_t*)inClusterList,
                                      outCount,
                                      (uint16_t*)outClusterList,
                                      opts);
}

void rtgRequestCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t startIndex = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t contents[ZDO_MESSAGE_OVERHEAD + 1];
  contents[ZDO_MESSAGE_OVERHEAD] = startIndex;
  sl_zigbee_send_zig_dev_request(target,
                                 ROUTING_TABLE_REQUEST,
                                 0,              // APS options
                                 contents,
                                 sizeof(contents));
}

//------------------------------------------------------------------------------

void sendNodeDescriptorRequest(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 0);
  uint16_t options = sl_cli_get_argument_uint16(arguments, 1);
  sl_status_t status = sl_zigbee_node_descriptor_request(target,
                                                         options);
  sl_zigbee_core_debug_println("node Desc Request: status 0x%02X",
                               status);
}

void zdoMgmtLqiCommandZCP(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t index = sl_cli_get_argument_uint8(arguments, 1);
  sl_status_t status = sl_zigbee_lqi_table_request(target,
                                                   index,
                                                   SL_ZIGBEE_APS_OPTION_RETRY);
  sl_zigbee_core_debug_println("LQI Table request: 0x%02X",
                               status);
}
