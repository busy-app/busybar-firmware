/***************************************************************************//**
 * @file
 * @brief Prints various information abouth the stack.
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

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"

#include "app/util/serial/sl_zigbee_command_interpreter.h"

#include "stack-diagnostics-config.h"
#if ((SL_ZIGBEE_AF_PLUGIN_STACK_DIAGNOSTICS_RECORD_LQI_RSSI == 1) && !defined(EZSP_HOST))
#define RECORD_LQI_RSSI
#endif

#define UNKNOWN_LEAVE_REASON_INDEX     5
#define APP_EVENT_1_LEAVE_REASON_INDEX 6
static const char * leaveReasonStrings[] = {
  "-",                 // None
  "NWK Leave message",
  "APS Remove message",
  "ZDO Leave message",
  "ZLL Touchlink",
  "???",
  "App Event 1",
};

#define UNKNOWN_REJOIN_REASON_INDEX       5
#define APP_FRAMEWORK_REJOIN_REASON_INDEX 6
#define CUSTOM_APP_EVENTS_INDEX           9
static const char * rejoinReasonStrings[] = {
  "-",
  "NWK Key Update",
  "Leave & Rejoin Message",
  "No Parent",
  "ZLL Touchlink",
  "???",

  // General App. Framework events defined in 'af.h'
  "End Device Move",
  "TC Keepalive failure",
  "CLI Command",

  // Non App. Framework, Application specific custom events
  "App Event 5",
  "App Event 4",
  "App Event 3",
  "App Event 2",
  "App Event 1",
};

static const char * nodeTypeStrings[] = {
  "Unknown",
  "Coordinator",
  "Router",
  "End Device",
  "Sleep End Device",
};

#ifdef RECORD_LQI_RSSI
#define DEFAULT_LQI    0
#define DEFAULT_RSSI  -127
static uint8_t neighborLqiTable[SL_ZIGBEE_MAX_NEIGHBOR_TABLE_SIZE] = { DEFAULT_LQI };
static int8_t neighborRssiTable[SL_ZIGBEE_MAX_NEIGHBOR_TABLE_SIZE] = { DEFAULT_RSSI };
#endif // RECORD_LQI_RSSI

//------------------------------------------------------------------------------
// Externs

#ifdef RECORD_LQI_RSSI
// TODO: once we add an sl_zigbee_rx_packet_info_t struct to the
// sl_zigbee_internal_packet_handoff_incoming_handler() we will use that instead
// of externing these globals.
extern sl_802154_short_addr_t sli_zigbee_current_sender;
extern uint8_t sli_zigbee_current_lqi;
extern int8_t sli_zigbee_current_rssi;

extern uint8_t sli_zigbee_neighbor_entry_index(sl_802154_short_addr_t neighbor);
#endif // RECORD_LQI_RSSI

//------------------------------------------------------------------------------
// Functions

void printChildTable(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  uint8_t size = sl_zigbee_af_get_child_table_size();
  uint8_t i;
  const char * types[] = {
    "Unknown",
    "Coordin",
    "Router ",
    "RxOn   ",
    "Sleepy ",
    "???    ",
  };
  uint8_t used = 0;
  uint32_t returnFrameCounter = 0;

  sl_zigbee_af_app_println("");
  sl_zigbee_af_app_println("#  type    id     eui64               timeout         remaining         framecounter");
  for (i = 0; i < size; i++) {
    sl_zigbee_child_data_t childData;
    sl_status_t status = sl_zigbee_af_get_child_data(i, &childData);
    if (status != SL_STATUS_OK) {
      continue;
    }
    if (childData.type > SL_ZIGBEE_SLEEPY_END_DEVICE) {
      childData.type = SL_ZIGBEE_SLEEPY_END_DEVICE + 1;
    }
    used++;
    sl_zigbee_af_app_print("%d: %s 0x%04X ", i, types[childData.type], childData.id);
    sl_zigbee_af_app_debug_exec(sl_zigbee_af_print_big_endian_eui64(childData.eui64));
    sl_zigbee_af_app_print(" ");
    if (childData.timeout == SECONDS_10) {
      sl_zigbee_af_app_print("10 sec");
    } else {
      sl_zigbee_af_app_print("%d min", (2 << (childData.timeout - 1)));
    }
    if (childData.remainingTimeout >= 60) {
      sl_zigbee_af_app_print("         %d min", (childData.remainingTimeout / 60));
    } else {
      sl_zigbee_af_app_print("         %d sec", childData.remainingTimeout);
    }

    //Add in the Frame Counter information
    status = sl_zigbee_get_neighbor_frame_counter(childData.eui64, &returnFrameCounter);
    if (status != SL_STATUS_OK) {
      returnFrameCounter = 0xFFFFFFFF;
    }

    sl_zigbee_af_app_print(" 0x%08X ", returnFrameCounter);
    sl_zigbee_af_app_println("");
  }
  sl_zigbee_af_app_println("\n%d of %d entries used.",
                           used,
                           sl_zigbee_af_get_child_table_size());
}

void printNeighborTable(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  uint8_t used = 0;
  uint8_t i;
  sl_zigbee_neighbor_table_entry_t n;
  uint32_t returnFrameCounter = 0;

  sl_zigbee_af_app_println("\n#  id     lqi  in  out  age  "
                    #ifdef RECORD_LQI_RSSI
                           "lastRssi lastLqi "
                    #endif // RECORD_LQI_RSSI
                           "framecounter eui");
  for (i = 0; i < sl_zigbee_af_get_neighbor_table_size(); i++) {
    sl_status_t status = sl_zigbee_get_neighbor(i, &n);
    if ((status != SL_STATUS_OK)
        || (n.shortId == SL_ZIGBEE_NULL_NODE_ID)) {
      continue;
    }
    used++;

    //Add in Frame Counter information
    status = sl_zigbee_get_neighbor_frame_counter(n.longId, &returnFrameCounter);
    if (status != SL_STATUS_OK) {
      returnFrameCounter = 0xFFFFFFFF;
    }

    sl_zigbee_af_app_print("%d: 0x%04X %d  %d   %d    %d    "
                    #ifdef RECORD_LQI_RSSI
                           "%d      %d     "
                    #endif // RECORD_LQI_RSSI
                           "0x%08X  "
                           ,
                           i,
                           n.shortId,
                           n.averageLqi,
                           n.inCost,
                           n.outCost,
                           n.age,
                    #ifdef RECORD_LQI_RSSI
                           neighborRssiTable[i],
                           neighborLqiTable[i],
                    #endif // RECORD_LQI_RSSI
                           returnFrameCounter
                           );
    sl_zigbee_af_app_debug_exec(sl_zigbee_af_print_big_endian_eui64(n.longId));
    sl_zigbee_af_app_println("");
    sl_zigbee_af_app_flush();
  }
  sl_zigbee_af_app_println("\n%d of %d entries used.", used, sl_zigbee_af_get_neighbor_table_size());
}

void printRouteTable(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  const char * zigbeeRouteStatusNames[] = {
    "active",
    "discovering",
    "??",
    "unused",
  };
  const char * concentratorNames[] = {
    "-    ",
    "low  ",
    "high ",
  };
  uint8_t used = 0;
  uint8_t i;
  sl_zigbee_route_table_entry_t entry;

  sl_zigbee_af_app_println("#  id      next    age  conc    status");
  for (i = 0; i < sl_zigbee_af_get_route_table_size(); i++) {
    if (SL_STATUS_OK !=  sl_zigbee_get_route_table_entry(i, &entry)
        || entry.destination == SL_ZIGBEE_NULL_NODE_ID) {
      continue;
    }
    used++;
    sl_zigbee_af_app_println("%d: 0x%04X  0x%04X  %d   %s  %s",
                             i,
                             entry.destination,
                             entry.nextHop,
                             entry.age,
                             concentratorNames[entry.concentratorType],
                             zigbeeRouteStatusNames[entry.status]);
  }
  sl_zigbee_af_app_println("\n%d of %d entries used.", used, sl_zigbee_af_get_route_table_size());
}

void printInfo(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_802154_short_addr_t nodeThatSentLeave;
  sl_zigbee_rejoin_reason_t rejoinReason = sl_zigbee_get_last_rejoin_reason();
  sl_zigbee_leave_reason_t  leaveReason =  sl_zigbee_get_last_leave_reason(&nodeThatSentLeave);

  uint8_t index;

  sl_802154_short_addr_t id = sl_zigbee_af_get_node_id();
  sl_zigbee_node_type_t type = SL_ZIGBEE_UNKNOWN_DEVICE;
  sl_zigbee_network_parameters_t parameters;

  if (!sli_zigbee_af_pro_is_current_network()) {
    return;
  }

  sl_status_t status = sl_zigbee_af_get_network_parameters(&type,
                                                           &parameters);

  sl_zigbee_af_app_println("Stack Profile: %d", sl_zigbee_af_get_stack_profile());
  sl_zigbee_af_app_println("Configured Node Type (%d): %s",
                           sli_zigbee_af_current_zigbee_pro_network->nodeType,
                           nodeTypeStrings[sli_zigbee_af_current_zigbee_pro_network->nodeType]);
  sl_zigbee_af_app_println("Running Node Type    (%d): %s\n",
                           type,
                           nodeTypeStrings[type]);

  if (status == SL_STATUS_NOT_JOINED) {
    sl_zigbee_af_app_println("Node is not joined - info is not available");
    return;
  }

  sl_zigbee_af_app_println("Channel:       %d", parameters.radioChannel);
  sl_zigbee_af_app_println("Node ID:       0x%04X", id);
  sl_zigbee_af_app_println("PAN ID:        0x%04X", parameters.panId);
  sl_zigbee_af_app_print("Extended PAN:  ");
  sl_zigbee_af_print_big_endian_eui64(parameters.extendedPanId);
  sl_zigbee_af_app_println("");
  sl_zigbee_af_app_println("\nNWK Update ID: 0x%02X\n", parameters.nwkUpdateId);

  sl_zigbee_af_app_println("NWK Manager ID: 0x%04X", parameters.nwkManagerId);
  sl_zigbee_af_app_print("NWK Manager Channels: ");
  sl_zigbee_af_print_channel_list_from_mask(parameters.channels);
  sl_zigbee_af_app_println("\n");

  sl_zigbee_af_app_println("Send Multicasts to sleepies: %s\n",
                           (sl_zigbee_af_get_sleepy_multicast_config()
                            ? "yes"
                            : "no"));

  index = rejoinReason;
  if (rejoinReason >= SL_ZIGBEE_REJOIN_DUE_TO_APP_EVENT_5) {
    index = rejoinReason - SL_ZIGBEE_REJOIN_DUE_TO_APP_EVENT_5 + CUSTOM_APP_EVENTS_INDEX;
  } else if (rejoinReason >= SL_ZIGBEE_AF_REJOIN_FIRST_REASON
             && rejoinReason <= SL_ZIGBEE_AF_REJOIN_LAST_REASON) {
    index = rejoinReason - SL_ZIGBEE_AF_REJOIN_FIRST_REASON + APP_FRAMEWORK_REJOIN_REASON_INDEX;
  } else if (rejoinReason >= SL_ZIGBEE_REJOIN_DUE_TO_ZLL_TOUCHLINK) {
    index = UNKNOWN_REJOIN_REASON_INDEX;
  }
  sl_zigbee_af_app_println("Last Rejoin Reason (%d): %s",
                           rejoinReason,
                           rejoinReasonStrings[index]);

  index = leaveReason;
  if (leaveReason == 0xFF) {
    index = APP_EVENT_1_LEAVE_REASON_INDEX;
  } else if (leaveReason > SL_ZIGBEE_LEAVE_DUE_TO_ZLL_TOUCHLINK) {
    index = UNKNOWN_LEAVE_REASON_INDEX;
  }
  sl_zigbee_af_app_println("Last Leave Reason  (%d): %s",
                           leaveReason,
                           leaveReasonStrings[index]);
  sl_zigbee_af_app_println("Node that sent leave: 0x%04X", nodeThatSentLeave);
}

#ifdef RECORD_LQI_RSSI
// Provided by the "zigbee_packet_handoff" component
sl_zigbee_packet_action_t sl_zigbee_pre_incoming_packet_filter_cb(sl_zigbee_zigbee_packet_type_t packetType,
                                                                  uint8_t* packetData,
                                                                  uint8_t* size_p,
                                                                  void *data,
                                                                  uint8_t size_d)
{
  sl_802154_short_addr_t macSource = SL_ZIGBEE_NULL_NODE_ID;
  UNUSED_VAR(packetData);
  UNUSED_VAR(size_p);
  UNUSED_VAR(data);
  UNUSED_VAR(size_d);

  switch (packetType) {
    case SL_ZIGBEE_ZIGBEE_PACKET_TYPE_RAW_MAC:
      // unprocessed by stack, ignore
      break;
    case SL_ZIGBEE_ZIGBEE_PACKET_TYPE_MAC_COMMAND:
      // only called when receiving beacon request, which lacks source info
      break;
    case SL_ZIGBEE_ZIGBEE_PACKET_TYPE_NWK_DATA:
    case SL_ZIGBEE_ZIGBEE_PACKET_TYPE_NWK_COMMAND:
      macSource = sli_zigbee_current_sender;
      break;
    case SL_ZIGBEE_ZIGBEE_PACKET_TYPE_APS_DATA:
    case SL_ZIGBEE_ZIGBEE_PACKET_TYPE_APS_COMMAND:
    case SL_ZIGBEE_ZIGBEE_PACKET_TYPE_ZDO:
      // APS frames will have been called as SL_ZIGBEE_ZIGBEE_PACKET_TYPE_NWK_DATA
      // as well, so no need to double record them
      break;
    case SL_ZIGBEE_ZIGBEE_PACKET_TYPE_INTERPAN_ZLL:
      // Only called for Interpan messages, which lack short source
      break;
    case SL_ZIGBEE_ZIGBEE_PACKET_TYPE_BEACON:
      // We'll only look at nwk+aps messages
      break;
    default:
      break;
  }

  if (macSource != SL_ZIGBEE_NULL_NODE_ID) {
    uint8_t index = sli_zigbee_neighbor_entry_index(macSource);

    if (index != 0xFF) {
      neighborLqiTable[index] = sli_zigbee_current_lqi;
      neighborRssiTable[index] = sli_zigbee_current_rssi;
    }
  }

  return SL_ZIGBEE_ACCEPT_PACKET;
}
#endif // RECORD_LQI_RSSI
