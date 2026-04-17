/***************************************************************************//**
 * @file
 * @brief User-configurable stack memory allocation.
 *
 *
 * \b Note: Application developers should \b not modify any portion
 * of this file. Doing so may lead to mysterious bugs. Allocations should be
 * adjusted only with macros in a custom CONFIGURATION_HEADER.
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
#include "hal.h"
#include "sl_common.h" // SL_ALIGN
#include "stack/include/sl_zigbee.h"
#include "stack/include/message.h" // Required for packetHandlers
#include "mac-child.h" // unified-mac
#include "scan.h" // Required for PG_CH_BITMASK32
#include "stack/include/sl_zigbee_types_internal.h"
#include "stack/internal/inc/raw-message-internal-def.h"

#include <stdlib.h>

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#ifdef SL_CATALOG_RADIO_PRIORITY_15_4_PRESENT
#include "sl_802154_radio_priority_config.h"
#endif // SL_CATALOG_RADIO_PRIORITY_15_4_PRESENT

#ifdef SL_CATALOG_HIGH_DATARATE_PHY_PRESENT
#include "high_datarate_phy_config.h"
#endif
// *****************************************
// Memory Allocations & declarations
// *****************************************

#if defined(CORTEXM3)
  #define align(value) (((value) + 3) & ~0x03)
#else
  #define align(value) (value)
#endif

//------------------------------------------------------------------------------
// API Version

const uint8_t sli_zigbee_api_version
  = (SL_ZIGBEE_API_MAJOR_VERSION << 4) + SL_ZIGBEE_API_MINOR_VERSION;

//------------------------------------------------------------------------------
// Multi PAN globals

#define NUM_MULTI_PAN_FORKS    (SL_ZIGBEE_ZC_AND_ZR_DEVICE_COUNT == 0 \
                                ? 1                                   \
                                : SL_ZIGBEE_ZC_AND_ZR_DEVICE_COUNT)
uint8_t sli_zigbee_num_multi_pan_forks = NUM_MULTI_PAN_FORKS;

uint8_t blackListedIdByteArray[NUM_MULTI_PAN_FORKS] = { 0 };
sli_buffer_manager_buffer_t blacklistedIdsArray[NUM_MULTI_PAN_FORKS] = { SL_ZIGBEE_NULL_MESSAGE_BUFFER };
uint8_t panConflictMeters[NUM_MULTI_PAN_FORKS];
uint16_t trustCenterPolicies[NUM_MULTI_PAN_FORKS];
bool sl_permit_panid_conflict_reports[NUM_MULTI_PAN_FORKS] = { true };
uint16_t sl_zigbee_local_panid_conflict_count[NUM_MULTI_PAN_FORKS];
uint16_t sl_local_panid_conflict_last_reset_sec[NUM_MULTI_PAN_FORKS];
//------------------------------------------------------------------------------
// Multi PAN events

// In multi-PAN, we fork events that are required for both ZC and ZR networks.
// Each index of the event array maps 1-to-1 to the network index.
// For multi-network and single network, we only allocate 1 copy
// of the event (i.e. an array of element size 1).

// The following are not resizable, and we especially don't want their addresses
// to change, so we don't allocate them from emAvailableMemory

sli_zigbee_event_t sli_zigbee_beacon_events[NUM_MULTI_PAN_FORKS];
sli_zigbee_event_t sli_zigbee_permit_joining_events[NUM_MULTI_PAN_FORKS];
sli_zigbee_event_t sli_zigbee_network_management_events[NUM_MULTI_PAN_FORKS];
sli_zigbee_event_t sli_zigbee_neighbor_exchange_events[NUM_MULTI_PAN_FORKS];
sli_zigbee_event_t sli_zigbee_request_key_events[SL_ZIGBEE_SUPPORTED_NETWORKS];
sli_zigbee_event_t sli_zigbee_partner_key_update_event;

#if !defined(SL_ZIGBEE_LEAF_STACK) || defined (CSL_SUPPORT)
sli_zigbee_event_t sli_zigbee_send_parent_announce_events[NUM_MULTI_PAN_FORKS];
#endif
sli_zigbee_event_t sli_zigbee_transient_link_key_events[NUM_MULTI_PAN_FORKS];
sli_zigbee_event_t sli_zigbee_gp_tx_events[NUM_MULTI_PAN_FORKS];
sli_zigbee_event_t sli_zigbee_source_route_update_events[NUM_MULTI_PAN_FORKS];
#if !(defined(SL_ZIGBEE_ROUTER_STACK)) && !(defined(SL_ZIGBEE_LEAF_STACK))
sli_zigbee_event_t sli_zigbee_allow_tc_rejoins_using_well_known_key_events[NUM_MULTI_PAN_FORKS];
#endif

//------------------------------------------------------------------------------

// Timeout in milliseconds
// before the entry in the transient table will be purged or
// copied into the relevant table (child / neighbor)
// It is also the proposed apsSecurityTimeoutPeriod
uint16_t sli_zigbee_transient_device_timeout = SL_ZIGBEE_TRANSIENT_DEVICE_DEFAULT_TIMEOUT_MS;

//Allows the aps acks for APSME commands to be turned off or on
//This should be "true" by default but to keep it consistend with the current stack behaviour this is "false" by default.
bool sl_zigbee_enable_aps_ack_for_cmds = SL_ZIGBEE_ENABLE_APS_ACKS_FOR_COMMANDS;
//------------------------------------------------------------------------------
// MAC Layer

//This array's length is stored just below its declaration
//as zigbee_mac_filter_list_length.
const sl_zigbee_mac_filter_match_data_t zigbeeMacFilterList[] = {
#if (defined(SL_ZIGBEE_AF_PLUGIN_GP_LIBRARY) || defined(SL_CATALOG_ZIGBEE_GREEN_POWER_PRESENT))
  (SL_802154_FILTER_MATCH_ON_PAN_DEST_BROADCAST
   | SL_802154_FILTER_MATCH_ON_PAN_SOURCE_NONE
   | SL_802154_FILTER_MATCH_ON_DEST_BROADCAST_SHORT
   | SL_802154_FILTER_MATCH_ON_SOURCE_NONE),
  (SL_802154_FILTER_MATCH_ON_PAN_DEST_BROADCAST
   | SL_802154_FILTER_MATCH_ON_PAN_SOURCE_LOCAL
   | SL_802154_FILTER_MATCH_ON_DEST_BROADCAST_SHORT
   | SL_802154_FILTER_MATCH_ON_SOURCE_LONG),
#endif // SL_ZIGBEE_AF_PLUGIN_GP_LIBRARY
#if (defined(SL_ZIGBEE_AF_PLUGIN_ZLL_LIBRARY) || defined(SL_CATALOG_ZIGBEE_LIGHT_LINK_PRESENT))
  // ZLL Scan requests
  (SL_802154_FILTER_MATCH_ON_PAN_DEST_BROADCAST
   | SL_802154_FILTER_MATCH_ON_PAN_SOURCE_NON_LOCAL
   | SL_802154_FILTER_MATCH_ON_DEST_BROADCAST_SHORT
   | SL_802154_FILTER_MATCH_ON_SOURCE_LONG),
  (SL_802154_FILTER_MATCH_ON_PAN_DEST_BROADCAST
   | SL_802154_FILTER_MATCH_ON_PAN_SOURCE_LOCAL
   | SL_802154_FILTER_MATCH_ON_DEST_BROADCAST_SHORT
   | SL_802154_FILTER_MATCH_ON_SOURCE_LONG),

  // All other incoming inter-pan ZLL messages
  (SL_802154_FILTER_MATCH_ON_PAN_DEST_BROADCAST
   | SL_802154_FILTER_MATCH_ON_PAN_SOURCE_NON_LOCAL
   | SL_802154_FILTER_MATCH_ON_DEST_UNICAST_LONG
   | SL_802154_FILTER_MATCH_ON_SOURCE_LONG),
  (SL_802154_FILTER_MATCH_ON_PAN_DEST_BROADCAST
   | SL_802154_FILTER_MATCH_ON_PAN_SOURCE_LOCAL
   | SL_802154_FILTER_MATCH_ON_DEST_UNICAST_LONG
   | SL_802154_FILTER_MATCH_ON_SOURCE_LONG),
#endif // SL_ZIGBEE_AF_PLUGIN_ZLL_LIBRARY
  SL_802154_FILTER_MATCH_END
};

uint8_t zigbee_mac_filter_list_length = COUNTOF(zigbeeMacFilterList);

// Pass a pointer to child table in memory to the unified mac layer.
//sl_mac_child_entry_t *sli_zigbee_child_table_data = (sl_mac_child_entry_t *) &emAvailableMemory[0];
sl_mac_child_entry_t sli_zigbee_child_table_data[1 + SL_ZIGBEE_CHILD_TABLE_SIZE * SL_ZIGBEE_ZC_AND_ZR_DEVICE_COUNT];
#if defined(SL_ZIGBEE_LEAF_STACK) && defined(SL_ZIGBEE_AF_NCP)
uint8_t sl_zigbee_child_table_size = 0;
#else // !(defined(SL_ZIGBEE_LEAF_STACK) && defined(SL_ZIGBEE_AF_NCP))
uint8_t sl_zigbee_child_table_size = SL_ZIGBEE_CHILD_TABLE_SIZE;
#endif // defined(SL_ZIGBEE_LEAF_STACK) && defined(SL_ZIGBEE_AF_NCP)

#if defined(SL_HDR_PHY_MAX_PACKET_SIZE)
#define MAX_FLAT_PACKET_SIZE SL_HDR_PHY_MAX_PACKET_SIZE + 4 // + 4 for timestamp addition in mac layer in case of scheduled tx
#else
#define MAX_FLAT_PACKET_SIZE 127
#endif

uint16_t sli_mac_max_rx_flat_pkt_size = MAX_FLAT_PACKET_SIZE + 16 /* + 4 crc + 12 RAIL appendend info +*/;

// TX buffer needs to be 32 bit aligned
// Manually adding padding byte at the end of each major index array to ensure they are properly aligned
#define ALIGNED_MAX_FLAT_PACKET_SIZE (((MAX_FLAT_PACKET_SIZE + 3) / 4) * 4)
uint16_t sli_mac_max_tx_flat_pkt_size = ALIGNED_MAX_FLAT_PACKET_SIZE;
SL_ALIGN(4) static uint8_t outgoing_flat_packet[MAX_MAC_INDEX][ALIGNED_MAX_FLAT_PACKET_SIZE] SL_ATTRIBUTE_ALIGN(4);
uint8_t *sli_mac_upper_mac_outgoing_flat_packet_ptr = (uint8_t *)outgoing_flat_packet;

//------------------------------------------------------------------------------
// NWK Layer

#ifdef SL_ZIGBEE_DISABLE_RELAY
uint8_t sli_zigbee_allow_relay = false;
#else
uint8_t sli_zigbee_allow_relay = true;
#endif

uint16_t sli_zigbee_child_status_data[1 + SL_ZIGBEE_CHILD_TABLE_SIZE * SL_ZIGBEE_ZC_AND_ZR_DEVICE_COUNT];

uint32_t sli_zigbee_child_timers_data[1 + SL_ZIGBEE_CHILD_TABLE_SIZE * SL_ZIGBEE_ZC_AND_ZR_DEVICE_COUNT];

int8_t sli_zigbee_child_power[1 + SL_ZIGBEE_CHILD_TABLE_SIZE];

// sli_zigbee_child_lqi >> 8 keeps the average Lqi values mapped from 0-255 to 1-255 after initialization
// LQI data (0-255) is stored in a 16 bit variable so we end up with greater
// precision after the weighted averaging operation
uint16_t sli_zigbee_child_lqi_data[1 + SL_ZIGBEE_CHILD_TABLE_SIZE * SL_ZIGBEE_ZC_AND_ZR_DEVICE_COUNT];

sli_zigbee_route_table_entry_t sli_zigbee_route_table[1 + SL_ZIGBEE_ROUTE_TABLE_SIZE];
uint8_t sli_zigbee_route_table_size = SL_ZIGBEE_ROUTE_TABLE_SIZE;

// Duplicate the table for multi PAN mode depending on how many ZC and ZR devices are present.
uint8_t sli_zigbee_route_record_table_data[1 + (((SL_ZIGBEE_CHILD_TABLE_SIZE + 7) >> 3) * SL_ZIGBEE_ROUTE_TABLE_SIZE * SL_ZIGBEE_ZC_AND_ZR_DEVICE_COUNT)];

sli_zigbee_discovery_table_entry_t sli_zigbee_discovery_table[SL_ZIGBEE_DISCOVERY_TABLE_SIZE];
uint8_t sli_zigbee_discovery_table_size = SL_ZIGBEE_DISCOVERY_TABLE_SIZE;

sl_zigbee_multicast_table_entry_t sli_zigbee_multicast_table[SL_ZIGBEE_MULTICAST_TABLE_SIZE];
uint8_t sli_zigbee_multicast_table_size = SL_ZIGBEE_MULTICAST_TABLE_SIZE;

// Broadcast table exists in all device types so we allocate memory to it even if no ZC and ZR device present.
// Duplicate the table if more than one ZC and ZR devices present.
sl_zigbee_broadcast_table_entry_t sli_zigbee_broadcast_table_data[SL_ZIGBEE_BROADCAST_TABLE_SIZE * NUM_MULTI_PAN_FORKS];
uint8_t sli_zigbee_broadcast_table_size = SL_ZIGBEE_BROADCAST_TABLE_SIZE;

//------------------------------------------------------------------------------
// Network descriptor (multi-network support)

#if !defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)
sl_zigbee_network_info_t sli_zigbee_network_descriptor[SL_ZIGBEE_SUPPORTED_NETWORKS];
uint8_t sli_zigbee_supported_networks = SL_ZIGBEE_SUPPORTED_NETWORKS;

// PAN info exists in all device types so we allocate memory to it even if no ZC and ZR device present.
// Duplicate it if more than one ZC and ZR devices present.
sl_zigbee_pan_info_t sl_zigbee_pan_info_table[NUM_MULTI_PAN_FORKS];
#else
uint8_t sli_zigbee_supported_networks = 1;
#define END_sli_zigbee_pan_info_data END_emberBroadcastTable
#endif // !defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)

//------------------------------------------------------------------------------
// Neighbor Table
// Neighbor and frame counter table exist on end device as well but it contains only one entry.
// Duplicate neighbor and frame counter tables per ZC and ZR device.
// If there is no ZC and ZR device present, then allocate one entry for each end device.
#ifdef CSL_SUPPORT
// For a CSL device, we reserve SL_ZIGBEE_NEIGHBOR_TABLE_SIZE entries for inbound reception of messages from SL_ZIGBEE_NEIGHBOR_TABLE_SIZE (number S2S initiators).
sl_zigbee_neighbor_table_entry_info_t sli_zigbee_neighbor_data[(SL_ZIGBEE_NEIGHBOR_TABLE_SIZE * SL_ZIGBEE_ZC_AND_ZR_DEVICE_COUNT) \
                                                               + SL_ZIGBEE_NEIGHBOR_TABLE_SIZE + SL_ZIGBEE_SUPPORTED_NETWORKS];
uint32_t sli_zigbee_frame_counters_table[((SL_ZIGBEE_NEIGHBOR_TABLE_SIZE + SL_ZIGBEE_CHILD_TABLE_SIZE) * SL_ZIGBEE_ZC_AND_ZR_DEVICE_COUNT) \
                                         + (SL_ZIGBEE_NEIGHBOR_TABLE_SIZE + SL_ZIGBEE_CHILD_TABLE_SIZE)                                    \
                                         + SL_ZIGBEE_SUPPORTED_NETWORKS];
#else // !CSL_SUPPORT
sl_zigbee_neighbor_table_entry_info_t sli_zigbee_neighbor_data[((SL_ZIGBEE_NEIGHBOR_TABLE_SIZE * SL_ZIGBEE_ZC_AND_ZR_DEVICE_COUNT) + SL_ZIGBEE_SUPPORTED_NETWORKS)];
uint32_t sli_zigbee_frame_counters_table[((SL_ZIGBEE_NEIGHBOR_TABLE_SIZE + SL_ZIGBEE_CHILD_TABLE_SIZE) * SL_ZIGBEE_ZC_AND_ZR_DEVICE_COUNT + SL_ZIGBEE_SUPPORTED_NETWORKS)];
#endif // CSL_SUPPORT
uint8_t sli_zigbee_router_neighbor_table_size = SL_ZIGBEE_NEIGHBOR_TABLE_SIZE;

//------------------------------------------------------------------------------
// NWK Retry Queue

sli_zigbee_retry_queue_entry_t sli_zigbee_retry_queue[SL_ZIGBEE_RETRY_QUEUE_SIZE];
uint8_t sli_zigbee_retry_queue_size = SL_ZIGBEE_RETRY_QUEUE_SIZE;

// NWK Store And Forward Queue

sli_zigbee_store_and_forward_queue_entry_t sli_zigbee_store_and_forward_queue[SL_ZIGBEE_STORE_AND_FORWARD_QUEUE_SIZE];
uint8_t sli_zigbee_store_and_forward_queue_size = SL_ZIGBEE_STORE_AND_FORWARD_QUEUE_SIZE;

//------------------------------------------------------------------------------
// Green Power stack tables
uint8_t sli_zigbee_gp_incoming_fc_token_table_size = SL_ZIGBEE_GP_INCOMING_FC_TOKEN_TABLE_SIZE;
uint32_t sli_zigbee_gp_incoming_fc_token_timeout = SL_ZIGBEE_GP_INCOMING_FC_TOKEN_TIMEOUT;

sl_zigbee_gp_proxy_table_entry_t sli_zigbee_gp_proxy_table[SL_ZIGBEE_GP_PROXY_TABLE_SIZE];
uint8_t sli_zigbee_gp_proxy_table_size = SL_ZIGBEE_GP_PROXY_TABLE_SIZE;

uint8_t sli_zigbee_gp_incoming_fc_in_sink_token_table_size = SL_ZIGBEE_GP_INCOMING_FC_IN_SINK_TOKEN_TABLE_SIZE;
uint32_t sli_zigbee_gp_incoming_fc_in_sink_token_timeout = SL_ZIGBEE_GP_INCOMING_FC_IN_SINK_TOKEN_TIMEOUT;

#if (SL_ZIGBEE_GP_SINK_TABLE_SIZE > 0)
sl_zigbee_gp_sink_table_entry_t sli_zigbee_gp_sink_table[SL_ZIGBEE_GP_SINK_TABLE_SIZE];
#else
// Provide a dummy entry to avoid 0 length array
sl_zigbee_gp_sink_table_entry_t sli_zigbee_gp_sink_table[1];
#endif
uint8_t sli_zigbee_gp_sink_table_size = SL_ZIGBEE_GP_SINK_TABLE_SIZE;

//------------------------------------------------------------------------------
// Source routing

#if (defined(SL_ZIGBEE_AF_PLUGIN_SOURCE_ROUTE_LIBRARY) || defined(SL_CATALOG_ZIGBEE_SOURCE_ROUTE_PRESENT))

// Source Route info
uint8_t sourceRouteTableEntryCount[NUM_MULTI_PAN_FORKS];
uint8_t sourceRouteTableNewestIndex[NUM_MULTI_PAN_FORKS];
uint16_t sourceRouteUpdateConcentratorType[NUM_MULTI_PAN_FORKS];
uint16_t sourceRouteUpdateMinQS[NUM_MULTI_PAN_FORKS];
uint16_t sourceRouteUpdateMaxQS[NUM_MULTI_PAN_FORKS];
uint8_t sourceRouteUpdateRouteErrorThreshold[NUM_MULTI_PAN_FORKS];
uint8_t sourceRouteUpdateDeliveryFailureThreshold[NUM_MULTI_PAN_FORKS];
uint8_t sourceRouteUpdateMaxHops[NUM_MULTI_PAN_FORKS];
uint8_t sourceRouteUpdateRouteErrorCount[NUM_MULTI_PAN_FORKS];
uint8_t sourceRouteUpdateDeliveryFailureCount[NUM_MULTI_PAN_FORKS];
sl_802154_short_addr_t sourceRouteUpdateNodeIdWithBrokenRoute[NUM_MULTI_PAN_FORKS];
bool sourceRouteUpdateSendAddressDiscoveryNextEvent[NUM_MULTI_PAN_FORKS];
bool sourceRouteUpdateActiveEvent[NUM_MULTI_PAN_FORKS];

sl_zigbee_source_route_table_entry_t sli_zigbee_source_route_table_data[SL_ZIGBEE_SOURCE_ROUTE_TABLE_SIZE * NUM_MULTI_PAN_FORKS];
uint8_t sli_zigbee_source_route_table_size = SL_ZIGBEE_SOURCE_ROUTE_TABLE_SIZE;
#else
sl_zigbee_source_route_table_entry_t *sli_zigbee_source_route_table_data = NULL;
uint8_t sli_zigbee_source_route_table_size = 0;
#define END_sli_zigbee_source_route_table END_sli_zigbee_gp_sink_table
#endif // SL_ZIGBEE_AF_PLUGIN_SOURCE_ROUTE_LIBRARY || SL_CATALOG_ZIGBEE_SOURCE_ROUTE_PRESENT

//------------------------------------------------------------------------------
// Binding Table

uint16_t sli_zigbee_binding_remote_node[1 + SL_ZIGBEE_BINDING_TABLE_SIZE];
uint8_t sli_zigbee_binding_table_size = SL_ZIGBEE_BINDING_TABLE_SIZE;

uint8_t sli_zigbee_binding_flags[1 + SL_ZIGBEE_BINDING_TABLE_SIZE];

//------------------------------------------------------------------------------
//End Device Timeouts

uint8_t sli_zigbee_end_device_timeout_data[1 + SL_ZIGBEE_CHILD_TABLE_SIZE * SL_ZIGBEE_ZC_AND_ZR_DEVICE_COUNT];

//------------------------------------------------------------------------------
// APS Layer

uint8_t  sli_zigbee_aps_duplicate_rejection_max_size = SL_ZIGBEE_APS_DUPLICATE_REJECTION_MAX_ENTRIES;
sl_zigbee_aps_duplicate_msg_entry_t sli_zigbee_aps_duplicate_rejection_entries[SL_ZIGBEE_APS_DUPLICATE_REJECTION_MAX_ENTRIES];

uint8_t sli_zigbee_address_table_size = SL_ZIGBEE_ADDRESS_TABLE_SIZE;
uint8_t sli_zigbee_address_table_max_size = SL_ZIGBEE_ADDRESS_TABLE_SIZE + 4;
sli_zigbee_address_table_entry_t sli_zigbee_address_table[SL_ZIGBEE_ADDRESS_TABLE_SIZE + 4];

uint8_t sli_zigbee_max_aps_unicast_messages = SL_ZIGBEE_APS_UNICAST_MESSAGE_COUNT;
sli_zigbee_aps_unicast_message_data_t sli_zigbee_aps_unicast_message_data[SL_ZIGBEE_APS_UNICAST_MESSAGE_COUNT];

uint16_t sli_zigbee_aps_ack_timeout_ms =
  ((SL_ZIGBEE_APSC_MAX_ACK_WAIT_HOPS_MULTIPLIER_MS
    * SL_ZIGBEE_MAX_HOPS)
   + SL_ZIGBEE_APSC_MAX_ACK_WAIT_TERMINAL_SECURITY_MS);

uint8_t sli_zigbee_fragment_delay_ms = SL_ZIGBEE_FRAGMENT_DELAY_MS;
uint8_t sl_zigbee_fragment_window_size = SL_ZIGBEE_FRAGMENT_WINDOW_SIZE;

uint8_t sl_zigbee_key_table_size = SL_ZIGBEE_KEY_TABLE_SIZE;
uint32_t sli_zigbee_incoming_aps_frame_counters[1 + SL_ZIGBEE_KEY_TABLE_SIZE];

sl_zigbee_tc_link_key_request_policy_t  sli_zigbee_trust_center_link_key_request_policies[NUM_MULTI_PAN_FORKS];
sl_zigbee_app_link_key_request_policy_t sli_zigbee_app_link_key_request_policies[NUM_MULTI_PAN_FORKS];

uint8_t sli_zigbee_certificate_table_size = SL_ZIGBEE_CERTIFICATE_TABLE_SIZE;

uint8_t sli_zigbee_zdo_beacon_survey_unique_nwks = SL_ZIGBEE_ZDO_BEACON_SURVEY_MAX_UNIQUE_NETWORKS;

uint8_t sli_zigbee_app_zdo_configuration_flags =
  0

// Define this in order to receive supported ZDO request messages via
// the incomingMessageHandler callback.  A supported ZDO request is one that
// is handled by the EmberZNet stack.  The stack will continue to handle the
// request and send the appropriate ZDO response even if this configuration
// option is enabled.
#ifdef SL_ZIGBEE_APPLICATION_RECEIVES_SUPPORTED_ZDO_REQUESTS
  | SL_ZIGBEE_APP_RECEIVES_SUPPORTED_ZDO_REQUESTS
#endif

// Define this in order to receive unsupported ZDO request messages via
// the incomingMessageHandler callback.  An unsupported ZDO request is one that
// is not handled by the EmberZNet stack, other than to send a 'not supported'
// ZDO response.  If this configuration option is enabled, the stack will no
// longer send any ZDO response, and it is the application's responsibility
// to do so.  To see if a response is required, the application must check
// the APS options bitfield within the sli_zigbee_stack_incoming_message_handler callback to see
// if the SL_ZIGBEE_APS_OPTION_ZDO_RESPONSE_REQUIRED flag is set.
#ifdef SL_ZIGBEE_APPLICATION_HANDLES_UNSUPPORTED_ZDO_REQUESTS
  | SL_ZIGBEE_APP_HANDLES_UNSUPPORTED_ZDO_REQUESTS
#endif

// Define this in order to receive the following ZDO request
// messages via the sli_zigbee_stack_incoming_message_handler callback: SIMPLE_DESCRIPTOR_REQUEST,
// MATCH_DESCRIPTORS_REQUEST, and ACTIVE_ENDPOINTS_REQUEST.  If this
// configuration option is enabled, the stack will no longer send any ZDO
// response, and it is the application's responsibility to do so.
// To see if a response is required, the application must check
// the APS options bitfield within the sli_zigbee_stack_incoming_message_handler callback to see
// if the SL_ZIGBEE_APS_OPTION_ZDO_RESPONSE_REQUIRED flag is set.
#ifdef SL_ZIGBEE_APPLICATION_HANDLES_ENDPOINT_ZDO_REQUESTS
  | SL_ZIGBEE_APP_HANDLES_ZDO_ENDPOINT_REQUESTS
#endif

// Define this in order to receive the following ZDO request
// messages via the sli_zigbee_stack_incoming_message_handler callback: BINDING_TABLE_REQUEST,
// BIND_REQUEST, and UNBIND_REQUEST.  If this
// configuration option is enabled, the stack will no longer send any ZDO
// response, and it is the application's responsibility to do so.
// To see if a response is required, the application must check
// the APS options bitfield within the sli_zigbee_stack_incoming_message_handler callback
// to see if the SL_ZIGBEE_APS_OPTION_ZDO_RESPONSE_REQUIRED flag is set.
#ifdef SL_ZIGBEE_APPLICATION_HANDLES_BINDING_ZDO_REQUESTS
  | SL_ZIGBEE_APP_HANDLES_ZDO_BINDING_REQUESTS
#endif
;

uint16_t sli_zigbee_transient_key_timeout_s = SL_ZIGBEE_TRANSIENT_KEY_TIMEOUT_S;

// This configuration is for non trust center node to assume a
// concentrator type of the trust center it join to, until it receive
// many-to-one route request from the trust center. For the trust center
// node, concentrator type is configured from the concentrator plugin.
// The stack by default assumes trust center be a low RAM concentrator.
// By setting to a low RAM concentrator, other devices send route record
// to the trust center even without receiving many-to-one route request.
sl_zigbee_assume_trust_center_concentrator_type_t sli_zigbee_assumed_trust_center_concentrator_type =
  SL_ZIGBEE_ASSUME_TRUST_CENTER_IS_LOW_RAM_CONCENTRATOR;

// Packet Buffers
const uint32_t sli_zigbee_buffer_heap_config_size = SL_ZIGBEE_PACKET_BUFFER_HEAP_SIZE;
// The actual memory for buffers will be allocated on initialization
void sli_legacy_buffer_manager_acquire_heap(void **heapMemory, size_t *heapMemorySize)
{
  *heapMemory = malloc(sli_zigbee_buffer_heap_config_size);
  *heapMemorySize = sli_zigbee_buffer_heap_config_size;
  assert(*heapMemory != NULL); /* buffer heap allocation failed, check `sl_memory_manager` heap usage*/
}

// *****************************************
// Non-dynamically configurable structures
// *****************************************
//const uint8_t sli_event_control_task_count = SL_ZIGBEE_TASK_COUNT;
//sl_zigbee_task_control_t sli_event_control_tasks[SL_ZIGBEE_TASK_COUNT];

// *****************************************
// Stack Profile Parameters
// *****************************************

const uint8_t sl_zigbee_stack_profile_id[8] = { 0, };

uint8_t sli_zigbee_stack_compliance_revision = SL_ZIGBEE_STACK_COMPLIANCE_REVISION;

uint8_t sli_zigbee_default_stack_profile = SL_ZIGBEE_STACK_PROFILE;
uint8_t sli_zigbee_default_security_level = SL_ZIGBEE_SECURITY_LEVEL;
uint8_t sli_zigbee_max_end_device_children = SL_ZIGBEE_MAX_END_DEVICE_CHILDREN;
uint8_t sli_zigbee_max_hops = SL_ZIGBEE_MAX_HOPS;
uint16_t sl_zigbee_mac_indirect_timeout = SL_ZIGBEE_INDIRECT_TRANSMISSION_TIMEOUT;
uint8_t sl_zigbee_end_devicekeep_alive_support_mode = SL_ZIGBEE_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE;
uint8_t sli_zigbee_stack_end_device_poll_timeout = SL_ZIGBEE_END_DEVICE_POLL_TIMEOUT;
//STATIC_ASSERT(SL_ZIGBEE_END_DEVICE_POLL_TIMEOUT <= MINUTES_16384, "End device timeout out of range");
uint16_t sl_zigbee_link_power_delta_interval = SL_ZIGBEE_LINK_POWER_DELTA_INTERVAL;
uint8_t sli_zigbee_request_key_timeout = SL_ZIGBEE_REQUEST_KEY_TIMEOUT;
uint8_t sli_zigbee_pan_id_conflict_report_threshold = SL_ZIGBEE_PAN_ID_CONFLICT_REPORT_THRESHOLD;
uint8_t sli_zigbee_zc_and_zr_count = SL_ZIGBEE_ZC_AND_ZR_DEVICE_COUNT;
uint8_t sli_zigbee_max_network_retries = SL_ZIGBEE_ZIGBEE_NUM_NETWORK_RETRIES_DEFAULT;

#ifndef SL_ZIGBEE_NO_STACK
uint8_t sli_zigbee_end_device_configuration = SL_ZIGBEE_END_DEVICE_CONFIG_PERSIST_DATA_ON_PARENT;
#endif

// Normally multicasts do NOT go to the sleepy address (0xFFFF), they go to
// RxOnWhenIdle=true (0xFFFD).  This can be changed, but doing so is not
// ZigBee Pro Compliant and is possibly NOT interoperable.
bool sli_zigbee_send_multicasts_to_sleepy_address = SL_ZIGBEE_SEND_MULTICASTS_TO_SLEEPY_ADDRESS;

// *****************************************
// ZigBee Light Link
// *****************************************

sl_zigbee_zll_policy_t sli_zigbee_zll_policy = SL_ZIGBEE_ZLL_POLICY_DISABLED;

// The number of groups required by the ZLL application.
uint8_t sli_zigbee_zll_group_addresses_needed = SL_ZIGBEE_ZLL_GROUP_ADDRESSES;
int8_t sli_zigbee_zll_rssi_threshold = SL_ZIGBEE_ZLL_RSSI_THRESHOLD;
#ifdef SL_ZIGBEE_ZLL_APPLY_THRESHOLD_TO_ALL_INTERPANS
bool sli_zigbee_zll_apply_threshold_to_all_interpans = true;
#else
bool sli_zigbee_zll_apply_threshold_to_all_interpans = false;
#endif

#if (defined(SL_ZIGBEE_AF_NCP) || defined(SL_CATALOG_ZIGBEE_NCP_FRAMEWORK_PRESENT) || defined(SL_ZIGBEE_TEST))

uint8_t customMacFilterTableSize = SL_ZIGBEE_CUSTOM_MAC_FILTER_TABLE_SIZE;
sl_zigbee_mac_filter_match_data_t sli_zigbee_custom_mac_filter_match_list_data[SL_ZIGBEE_CUSTOM_MAC_FILTER_TABLE_SIZE];

#endif // SL_ZIGBEE_AF_NCP || SL_CATALOG_ZIGBEE_NCP_FRAMEWORK_PRESENT || SL_ZIGBEE_TEST

#ifdef SL_CATALOG_RADIO_PRIORITY_15_4_PRESENT
sl_802154_radio_priorities_t radioSchedulerPriorityTable = {
  .background_rx = SL_802154_RADIO_PRIO_BACKGROUND_RX_VALUE,
  .min_tx_priority = SL_802154_RADIO_PRIO_TX_MIN,
  .tx_step = SL_802154_RADIO_PRIO_TX_STEP,
  .max_tx_priority = SL_802154_RADIO_PRIO_TX_MAX,
  .active_rx = SL_802154_RADIO_PRIO_ACTIVE_RX_VALUE,
};
#endif //SL_CATALOG_RADIO_PRIORITY_15_4_PRESENT

// Stubs that apply to both SoC and NCP
