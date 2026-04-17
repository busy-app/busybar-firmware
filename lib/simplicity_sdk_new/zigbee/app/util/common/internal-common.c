/***************************************************************************//**
 * @file
 * @brief common code for internal apps.
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
#include "stack/include/sl_zigbee.h"
#include "stack/core/sl_zigbee_stack.h"     // we use the events
#include "stack/core/sl_zigbee_multi_network.h"
#include "hal/hal.h"
#include "mac-child.h" // unified-mac

#include "serial/serial.h"
#include "stack/include/zigbee-device-stack.h"
#include "stack/routing/zigbee/child-handling.h"
#include "app/util/zigbee-framework/zigbee-device-library.h"

#include "app/util/common/common.h"
#include "app/util/common/internal-common.h"

#include "stack/framework/debug.h"
#include "stack/zigbee/zigbee-device.h"
#include "stack/routing/zigbee/route-table.h"
#include "stack/routing/zigbee/child.h"
#include "stack/routing/zigbee/id-assign.h"
#include "stack/routing/zigbee/association.h"
#include "stack/routing/zigbee/network.h"
#include "stack/routing/neighbor/neighbor.h"
#include "stack/routing/util/broadcast.h"
#include "stack/mac/command.h"
#include "stack/zigbee/stack-address-table.h"

#include "stack/framework/zigbee-packet-header.h"

#ifdef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
// This function prints out info that is used to verify
// multi-PAN functionality in tests, we strip it out for
// SL_ZIGBEE_MULTI_NETWORK_STRIPPED to save more code space.
  #define sli_zigbee_multi_pan_println(...)
#else
  #define sli_zigbee_multi_pan_println(...) sl_zigbee_core_debug_println(__VA_ARGS__);
#endif // SL_ZIGBEE_MULTI_NETWORK_STRIPPED

static const char yesChar = 'y';
static const char noChar  = 'n';

extern void emRadioEnableAddressMatching(uint8_t enable);
extern void sli_802154mac_radio_enable_auto_ack(bool enable);

void sniffCommand(SL_CLI_COMMAND_ARG)
{
  bool enable = (bool)sl_cli_get_argument_uint32(arguments, 0);
  emRadioEnableAddressMatching(!enable);
  sli_802154mac_radio_enable_auto_ack(!enable);
  if (enable) {
    sl_zigbee_core_debug_println("sniff on");
  } else {
    sl_zigbee_core_debug_println("sniff off");
  }
}

void statsCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  sli_zigbee_debug_print_stats();
}

void freeBuffersCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  sl_zigbee_core_debug_println("%d buffers",
                               sli_legacy_buffer_manager_buffer_bytes_remaining() / PACKET_BUFFER_SIZE);
}

void neighborTableCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  uint8_t i;
  for (i = 0; i < sli_zigbee_neighbor_count; i++) {
    uint32_t frameCounter = *(sli_zigbee_frame_counters + i);
    sl_zigbee_neighbor_table_entry_info_t *entry = sli_zigbee_neighbor_table + i;
    sl_zigbee_core_debug_println("%04X lqi:%d in:%d out:%d lap:%d age:%d used:%c my-key:%c fc:%4lx",
                                 entry->id,
                                 entry->incoming >> 8,
                                 sli_zigbee_neighbor_entry_incoming_cost(entry),
                                 sli_zigbee_neighbor_entry_outgoing_cost(entry),
                                 sli_zigbee_neighbor_entry_overlap_count(entry),
                                 sli_zigbee_neighbor_entry_age(entry),
                                 (sli_zigbee_in_use_as_next_hop(entry->id)
                                  ? yesChar
                                  : noChar),
                                 (sli_zigbee_neighbor_index_using_active_network_key(i)
                                  ? yesChar
                                  : noChar),
                                 (unsigned long)frameCounter);
    (void) sli_legacy_serial_wait_send(serialPort);
  }

  sl_zigbee_core_debug_println("%d / %d total neighbors",
                               sli_zigbee_neighbor_count,
                               SL_ZIGBEE_NEIGHBOR_TABLE_SIZE);
  (void) sli_legacy_serial_wait_send(serialPort);
}

void reomveNeighborByNodeIdCommand(SL_CLI_COMMAND_ARG)
{
  sl_802154_short_addr_t neighborId = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t index = sli_zigbee_neighbor_entry_index(neighborId);

  if (index != 0xFF) {
    // to remove from neighbor table
    sli_zigbee_remove_neighbor(index);
    // to remove from address table

    sli_zigbee_forget_short_id(neighborId);
    sl_zigbee_core_debug_println("Removed neighbor 0x%04X",
                                 neighborId);
  } else {
    sl_zigbee_core_debug_println("Neighbor 0x%04X not found",
                                 neighborId);
  }
}

void broadcastTableCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  uint8_t i;
  uint8_t used = 0;
  (void)used; // workaround clang's [-Werror,-Wunused-but-set-variable]
  for (i = 0; i < BROADCAST_TABLE_SIZE; i++) {
    sl_zigbee_broadcast_table_entry_t *b =
      sli_zigbee_broadcast_table
      + ((sli_zigbee_broadcast_head - i + BROADCAST_TABLE_SIZE) % BROADCAST_TABLE_SIZE);
    sl_zigbee_core_debug_println("%04X seq:%02X mask:%04lX numAcks:%d",
                                 b->source,
                                 b->sequence,
                                 (unsigned long)b->neighborBitmask,
                                 b->numAcks);
    if (b->source != SL_ZIGBEE_NULL_NODE_ID) {
      used++;
    }
  }
  sli_zigbee_multi_pan_println("In use: %d/%d, Bcast Cutoff: %d",
                               used,
                               BROADCAST_TABLE_SIZE,
                               sli_zigbee_broadcast_age_cutoff_indices);
}

void nodeIdCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  sl_zigbee_core_debug_println("NodeId: %04X", sl_zigbee_get_node_id());
}

const char * internalRouteStatusNames[] = {
  "active", "discov", "failed", "unused"
};

void routeTableCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  uint8_t i;
  uint8_t used = 0;
  (void)used; // workaround clang's [-Werror,-Wunused-but-set-variable]
  for (i = 0; i < sl_zigbee_get_route_table_size(); i++) {
    if (sli_zigbee_route_table[i].destination != SL_ZIGBEE_NULL_NODE_ID
        && sli_zigbee_route_table[i].networkIndex == sli_zigbee_get_current_network_index()) {
      sl_zigbee_core_debug_println("%s dest:%04X next:%04X Mto1:%c age:%dmin",
                                   internalRouteStatusNames[sli_zigbee_route_status(&sli_zigbee_route_table[i])],
                                   sli_zigbee_route_table[i].destination,
                                   sli_zigbee_route_table[i].nextHop,
                                   ((sli_zigbee_route_aggregator_type(&sli_zigbee_route_table[i])
                                     == HIGH_RAM_AGGREGATOR)
                                    ? 'H'
                                    : ((sli_zigbee_route_aggregator_type(&sli_zigbee_route_table[i])
                                        == LOW_RAM_AGGREGATOR)
                                       ? 'L'
                                       : noChar)),
                                   sli_zigbee_route_age(&sli_zigbee_route_table[i]));
      (void) sli_legacy_serial_wait_send(serialPort);
      used++;
    }
  }
  sl_zigbee_core_debug_println("In use: %d", used);
}

void discoveryTableCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  uint8_t i;
  uint8_t used = 0;
  (void)used; // workaround clang's [-Werror,-Wunused-but-set-variable]
  for (i = 0; i < sli_zigbee_discovery_table_size; i++) {
    sli_zigbee_discovery_table_entry_t *e = sli_zigbee_discovery_table + i;
    if (e->source != SL_ZIGBEE_NULL_NODE_ID
        && e->networkIndex == sli_zigbee_get_current_network_index()) {
      sl_zigbee_core_debug_println("src:%04X id:%d sender:%04X "
                                   "fwd:%d ttl:%d index:%d",
                                   e->source,
                                   e->id,
                                   e->sender,
                                   e->forwardRoutingCost,
                                   e->quarterSecondsToLive,
                                   e->routeTableIndex);
      used++;
    }
  }
  sli_zigbee_multi_pan_println("In use: %d", used);
}

void eraseChildCommand(SL_CLI_COMMAND_ARG)
{
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  if (SL_ZIGBEE_NULL_NODE_ID == sli_mac_child_short_id(index)) {
    sl_zigbee_core_debug_println("No child at index %d", index);
  } else {
    sli_zigbee_erase_child(index);
    sl_zigbee_core_debug_println("Erased child.");
  }
}

// This is mainly require for test case 10.28 if we ever decide to
// run this. We need to erase child table when it
// reaches SL_ZIGBEE_MAX_END_DEVICE_CHILDREN to join 100 devices.
void eraseChildTableCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  sli_zigbee_erase_child_table();
  sl_zigbee_core_debug_println("Erased child table.");
}

void childTableCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  uint8_t i;
  uint8_t inUseCount = 0;
  uint8_t maxChildren = sl_zigbee_get_max_end_device_children();

  sl_zigbee_core_debug_println("Child Table:");

  for (i = 0; i < maxChildren; i++) {
    if (sli_mac_have_child(i)) {
      sl_zigbee_child_data_t childData;
      sl_zigbee_get_child_data(i, &childData);

      uint8_t capabilities = sli_zigbee_get_child_capabilities(i);
      uint32_t frameCounter = *(sli_zigbee_frame_counters + sli_zigbee_neighbor_table_size + i);
      uint8_t actualTimeoutIndex = childData.timeout & ~(ZIGBEE_END_DEVICE_KEEP_ALIVE_RECEIVED);
      sl_zigbee_core_debug_print("%02X: %04X ",
                                 i, childData.id);
      printLittleEndianEui64(serialPort, childData.eui64);
      sl_zigbee_core_debug_print(" pwr: %d", childData.power);
      sl_zigbee_core_debug_println(" %sFD rx%s cap:%02X my-key:%c fc:%4lx auth:%c timeoutIndex: %u timeout: %u",
                                   (capabilities & CAPABILITY_DEVICE_TYPE ? "F" : "R"),
                                   (capabilities & CAPABILITY_RECEIVER_ON_WHEN_IDLE
                                    ? "ON" : "OFF"),
                                   capabilities,
                                   (sli_zigbee_child_is_using_active_network_key(i)
                                    ? yesChar
                                    : noChar),
                                   (unsigned long)frameCounter,
                                   (sli_zigbee_child_is_authenticated(i)
                                    ? yesChar
                                    : noChar),
                                   actualTimeoutIndex,
                                   (actualTimeoutIndex != SECONDS_10)
                                   ? (2 << (actualTimeoutIndex - 1)) * 60
                                   : 10);
      (void) sli_legacy_serial_wait_send(serialPort);
      inUseCount++;
    }
  }
  sl_zigbee_core_debug_println("Entries in use: %d / %d",
                               inUseCount,
                               maxChildren);
}

void addressTableCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  uint8_t i;
  sl_zigbee_core_debug_println("Flags  NodeId  EUI64");

  for (i = 0; i < sli_zigbee_address_table_size; i++) {
    sl_zigbee_core_debug_println("0x%02X   0x%04X   ",
                                 addressTable[i].flags,
                                 addressTable[i].shortId);
    printLittleEndianEui64(serialPort, addressTable[i].longId);
    printCarriageReturn();
    (void) sli_legacy_serial_wait_send(serialPort);
  }
}
