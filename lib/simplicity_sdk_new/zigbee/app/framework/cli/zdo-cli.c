/***************************************************************************//**
 * @file
 * @brief CLI commands for sending ZDO messages.
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

// common include file
#include "app/framework/util/common.h"
#include "app/framework/util/af-main.h"

#include "app/framework/include/af.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"

#include "app/util/zigbee-framework/zigbee-device-common.h"

#if defined(EZSP_HOST)
  #include "app/util/zigbee-framework/zigbee-device-host.h"
#else // SOC
  #include "app/util/zigbee-framework/zigbee-device-library.h"
#endif

#include "app/framework/cli/option-cli.h"

sl_status_t matchDescriptorsRequest(sl_802154_short_addr_t target,
                                    uint16_t profile,
                                    uint8_t inCount,
                                    uint8_t outCount,
                                    uint16_t *inClusters,
                                    uint16_t *outClusters,
                                    sl_zigbee_aps_option_t options);

#define MAX_CLUSTERS_CAN_MATCH 5
uint16_t sli_zigbee_af_cli_zdo_in_clusters[MAX_CLUSTERS_CAN_MATCH];
uint16_t sli_zigbee_af_cli_zdo_out_clusters[MAX_CLUSTERS_CAN_MATCH];
uint8_t sli_zigbee_af_cli_in_cl_count = 0;
uint8_t sli_zigbee_af_cli_out_cl_count = 0;

//------------------------------------------------------------------------------

// *****************************************
// zdoCommand
//
// zdo nwk <eui64:big-endian>
// zdo ieee <node-id>
// zdo simple <dest> <target ep>
// zdo node <dest>
// zdo match  <dest> <profile>
// zdo bind   <dest> <local ep> <remote ep> <cluster> <THEIR eui> <dest eui>
// zdo in-cl-list add <cluster IDs>
// zdo in-cl-list clear
// zdo out-cl-list add <clusters IDs>
// zdo out-cl-list clear
// zdo nwk-upd chan  <channel>
// zdo nwk-upd set   <nwk mgr id>  <chan mask>
// zdo nwk-upd scan  <target> <duration> <count>
// zdo active <target>
// *****************************************

void zdoNwkAddressRequestCommand(sl_cli_command_arg_t *args)
{
  sl_802154_long_addr_t eui64;
  sl_zigbee_copy_eui64_arg(args, 0, eui64, true);
  sl_zigbee_af_find_node_id(eui64,
                            sli_zigbee_af_cli_service_discovery_callback);
}

void zdoIeeeAddressRequestCommand(sl_cli_command_arg_t *args)
{
  sl_802154_short_addr_t id = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(args, 0);
  sl_zigbee_af_find_ieee_address(id,
                                 sli_zigbee_af_cli_service_discovery_callback);
}

void zdoSimpleCommand(sl_cli_command_arg_t *args)
{
  sl_802154_short_addr_t target = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(args, 0);
  uint8_t targetEndpoint = sl_cli_get_argument_uint8(args, 1);
  sl_status_t status = sl_zigbee_simple_descriptor_request(target,
                                                           targetEndpoint,
                                                           SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS);
  UNUSED_VAR(status);
  sl_zigbee_app_debug_println("ZDO simple desc req %02x", status);
}

void zdoNodeCommand(sl_cli_command_arg_t *args)
{
  sl_802154_short_addr_t target = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(args, 0);
  sl_status_t status = sl_zigbee_node_descriptor_request(target,
                                                         SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS);
  UNUSED_VAR(status);
  sl_zigbee_app_debug_println("ZDO node desc req %02x", status);
}

void zdoMatchCommand(sl_cli_command_arg_t *args)
{
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(args, 0);
  uint16_t profile = sl_cli_get_argument_uint16(args, 1);
  sl_status_t status = matchDescriptorsRequest(target,
                                               profile,
                                               sli_zigbee_af_cli_in_cl_count,
                                               sli_zigbee_af_cli_out_cl_count,
                                               sli_zigbee_af_cli_zdo_in_clusters,
                                               sli_zigbee_af_cli_zdo_out_clusters,
                                               SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS);
  UNUSED_VAR(status);
  sl_zigbee_app_debug_println("ZDO match desc req %02X", status);
}

static sl_status_t copyOrLookupEui64(sl_cli_command_arg_t *args,
                                     uint8_t argumentNumber,
                                     sl_802154_short_addr_t nodeId,
                                     sl_802154_long_addr_t returnEui64)
{
  sl_status_t status = SL_STATUS_OK;
  if (0 == sl_zigbee_copy_eui64_arg(args, argumentNumber, returnEui64, true)) {
    status = sl_zigbee_lookup_eui64_by_node_id(nodeId, returnEui64);
    if (status != SL_STATUS_OK) {
      sl_zigbee_app_debug_println("Error:  EUI64 argument is empty and lookup by node ID failed.");
    }
  }
  return status;
}

// For simple bind requests, just put {} as the last argument
//   zdo bind <dest> <local ep> <remote ep> <cluster> <THEIR EUI> {}
//
// More complex requests, you can actually specify the dest EUI64 of,
// the binding, which is NOT the same as the EUI64 of the destination
// of the device receiving the binding.  This allows for a user
// to set multiple bindings on the destination for different devices.
//   zdo bind <dest> <local ep> <remote ep> <cluster> <THEIR EUI> <dest EUI64>
void zdoBindCommand(sl_cli_command_arg_t *args)
{
  sl_status_t status;
  sl_802154_long_addr_t sourceEui, destEui;  // names relative to binding sent over-the-air
  sl_802154_short_addr_t target = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(args, 0);
  uint8_t sourceEndpoint = sl_cli_get_argument_uint8(args, 1);
  uint8_t destinationEndpoint = sl_cli_get_argument_uint8(args, 2);
  uint16_t clusterId = sl_cli_get_argument_uint16(args, 3);

  // NOTE:  The source/dest EUI is relative to the context.
  // In the case of the syntax of the CLI, we take "THEIR EUI64" to mean
  // the recipient of the binding request message.  However, when sending
  // the bind request that EUI64 used by the ZDO command is
  // actually the source for the binding because we are telling the remote
  // device (the destination) to create a binding with a source of itself.
  // And the destination for that binding will be this local device.
  // This is also not to be confused with the (short) destination of the ZDO
  // request itself.
  if (SL_STATUS_OK != copyOrLookupEui64(args, 4, target, sourceEui)) {
    return;
  }

  // If the last argument is empty, assume an EUI64 of the local device.
  // This allows for the simple case.  If an EUI64 is specified, it will
  // be used instead of the local EUI.  This is used for setting
  // multiple bindings on the same remote device.
  if (0 == sl_zigbee_copy_eui64_arg(args, 5, destEui, true)) {
    sl_zigbee_app_debug_println("Using my local EUI64 for dest EUI64 in binding");
    sl_zigbee_af_get_eui64(destEui);
  }

  status = sl_zigbee_bind_request(target,           // who gets the bind req
                                  sourceEui, // source eui IN the binding
                                  sourceEndpoint,
                                  clusterId,
                                  UNICAST_BINDING, // binding type
                                  destEui,   // destination eui IN the binding
                                  0,         // groupId for new binding
                                  destinationEndpoint,
                                  SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS);
  UNUSED_VAR(status);
  sl_zigbee_app_debug_println("ZDO bind req %02x", status);
}

void zdoAddClusterCommand(sl_cli_command_arg_t *args)
{
  uint16_t *clusters;
  uint8_t *clCount;

  if (memcmp(args->argv[args->arg_ofs - 2], "in", strlen("in")) == 0) {
    clusters = sli_zigbee_af_cli_zdo_in_clusters;
    clCount = &sli_zigbee_af_cli_in_cl_count;
  } else {
    clusters = sli_zigbee_af_cli_zdo_out_clusters;
    clCount = &sli_zigbee_af_cli_out_cl_count;
  }

  if (*clCount < MAX_CLUSTERS_CAN_MATCH) {
    clusters[*clCount] = sl_cli_get_argument_uint16(args, 0);
    (*clCount)++;
  } else {
    sl_zigbee_app_debug_println("cluster limit reached");
  }
}

void zdoClearClusterCommand(sl_cli_command_arg_t *args)
{
  if (memcmp(args->argv[args->arg_ofs - 2], "in", strlen("in")) == 0) {
    sli_zigbee_af_cli_in_cl_count = 0;
  } else {
    sli_zigbee_af_cli_out_cl_count = 0;
  }
}

void zdoNetworkUpdateChannelCommand(sl_cli_command_arg_t *args)
{
  uint8_t channel = sl_cli_get_argument_uint8(args, 0);
  uint8_t page = 0;
  sl_status_t status;

  if (sl_cli_get_argument_count(args) > 1) {
    page = sl_cli_get_argument_uint8(args, 1);
  } else {
    // page and channels are encoded in 8 bits as page 7..5 bits
    // channel 4...0 bits
    // 3rd and 4th bits are always 1's
    page = (uint8_t)(channel >> 5) | 0x18;
    channel = channel & 0x1F;
  }

  if (SL_STATUS_OK != sli_zigbee_af_validate_channel_pages(page, channel)) {
    sl_zigbee_app_debug_println("invalid page: %d or channel: %d", page, channel);
  } else {
    if (page == SL_ZIGBEE_NO_CHANNEL_PAGE_IN_USE) {
      // Request for 2.4Ghz network if page is zero
      status = sl_zigbee_channel_change_request(channel);
    } else {
      // 31..27 bits for page and 26..0 for channels
      uint32_t pageChannelBitMask = (page << 27 | BIT32(channel));
      status = sl_zigbee_energy_scan_request(SL_ZIGBEE_SLEEPY_BROADCAST_ADDRESS,
                                             pageChannelBitMask,
                                             0xFE,
                                             0);
    }
    UNUSED_VAR(status);
    sl_zigbee_app_debug_println("Change channel status: 0x%02x", status);
  }
}

void zdoNetworkUpdateScanCommand(sl_cli_command_arg_t *args)
{
  sl_802154_short_addr_t target = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(args, 0);
  uint8_t scanDuration = sl_cli_get_argument_uint8(args, 1);
  uint16_t scanCount = sl_cli_get_argument_uint16(args, 2);
  uint32_t scanChannelsMask = SL_ZIGBEE_ALL_802_15_4_CHANNELS_MASK;
  if (scanDuration > 5 || scanCount == 0 || scanCount > 8) {
    sl_zigbee_app_debug_println("duration must be in range 0 - 5");
    sl_zigbee_app_debug_println("count must be in range 1 - 8");
  } else {
    if (sl_cli_get_argument_count(args) > 3) {
      scanChannelsMask = sl_cli_get_argument_uint32(args, 3);
    }

    sl_status_t status = sl_zigbee_energy_scan_request(target,
                                                       scanChannelsMask,
                                                       scanDuration,
                                                       scanCount);
    sl_zigbee_af_app_print("scan status 0x%02X", status);
  }
}

void zdoNetworkUpdateSetCommand(sl_cli_command_arg_t *args)
{
  sl_802154_short_addr_t networkManager = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(args, 0);
  uint32_t activeChannels = sl_cli_get_argument_uint32(args, 1);
  sl_status_t status = sl_zigbee_set_network_manager_request(networkManager,
                                                             activeChannels);
  sl_zigbee_af_app_print("network update set status 0x%02X", status);
}

void zdoActiveEpCommand(sl_cli_command_arg_t *args)
{
  sl_802154_short_addr_t target = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(args, 0);
  sl_status_t status = sl_zigbee_active_endpoints_request(target,
                                                          SL_ZIGBEE_APS_OPTION_RETRY);
  sl_zigbee_af_app_print("Active EP request status: 0x%02X",
                         status);
}

void zdoMgmtLqiCommand(sl_cli_command_arg_t *args)
{
  sl_802154_short_addr_t target = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(args, 0);
  uint8_t index = sl_cli_get_argument_uint8(args, 1);
  sl_status_t status = sl_zigbee_lqi_table_request(target,
                                                   index,
                                                   SL_ZIGBEE_APS_OPTION_RETRY);
  sl_zigbee_af_app_print("LQI Table request: 0x%02X", status);
}

void zdoMgmtBindCommand(sl_cli_command_arg_t *args)
{
  sl_802154_short_addr_t target = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(args, 0);
  uint8_t index = sl_cli_get_argument_uint8(args, 1);
  sl_status_t status = sl_zigbee_binding_table_request(target,
                                                       index,
                                                       SL_ZIGBEE_APS_OPTION_RETRY);
  sl_zigbee_af_app_print("Binding Table request: 0x%02X", status);
}

void zdoLeaveRequestCommand(sl_cli_command_arg_t *args)
{
  sl_802154_short_addr_t target = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(args, 0);
  // CCB 2047
  // - CCB makes the first step to depracate the 'leave and remove children' functionality.
  // - We were proactive here and deprecated it right away.
  // bool removeChildren = (bool)sl_zigbee_unsigned_command_argument(1);
  bool rejoin = (bool)sl_cli_get_argument_uint8(args, 2);
  sl_802154_long_addr_t nullEui64 = { 0, 0, 0, 0, 0, 0, 0, 0 };
  sl_status_t status;

  uint8_t options = 0;
  if (rejoin) {
    options |= SL_ZIGBEE_ZIGBEE_LEAVE_AND_REJOIN;
  }

  status = sl_zigbee_leave_request(target,
                                   nullEui64,
                                   options,
                                   SL_ZIGBEE_APS_OPTION_RETRY);
  UNUSED_VAR(status);
  sl_zigbee_app_debug_println("Leave %s0x%02X", "Request: ", status);
}

void zdoPowerDescriptorRequestCommand(sl_cli_command_arg_t *args)
{
  sl_802154_short_addr_t target = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(args, 0);
  sl_status_t status = sl_zigbee_power_descriptor_request(target,
                                                          SL_ZIGBEE_APS_OPTION_RETRY);
  UNUSED_VAR(status);
  sl_zigbee_app_debug_println("Power Descriptor %s0x%02X", "Request: ", status);
}

static void unbindRequest(sl_cli_command_arg_t *args,
                          bool isGroupAddress,
                          void* destination,
                          uint8_t destinationEndpoint)
{
  sl_802154_short_addr_t target = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(args, 0);
  uint8_t sourceEndpoint = sl_cli_get_argument_uint8(args, 2);
  sl_zigbee_af_cluster_id_t clusterId = (sl_zigbee_af_cluster_id_t)sl_cli_get_argument_uint16(args, 3);
  sl_802154_long_addr_t sourceEui64;
  sl_802154_long_addr_t destinationEui64;
  sl_status_t status;
  sl_802154_short_addr_t groupAddress = SL_ZIGBEE_NULL_NODE_ID;

  if (SL_STATUS_OK != copyOrLookupEui64(args, 1, target, sourceEui64)) {
    return;
  }

  if (isGroupAddress) {
    groupAddress = *((sl_802154_short_addr_t*)destination);
  } else {
    groupAddress = 0;
    memmove(destinationEui64, destination, EUI64_SIZE);
  }

  status = sl_zigbee_unbind_request(target,
                                    sourceEui64,
                                    sourceEndpoint,
                                    clusterId,
                                    (isGroupAddress
                                     ? MULTICAST_BINDING
                                     : UNICAST_BINDING),
                                    destinationEui64,
                                    groupAddress,
                                    destinationEndpoint,
                                    SL_ZIGBEE_APS_OPTION_RETRY);
  UNUSED_VAR(status);
  sl_zigbee_app_debug_println("Unbind %s %s0x%02X",
                              (isGroupAddress
                               ? "Group"
                               : "Unicast"),
                              "Request: ",
                              status);
}

void zdoUnbindGroupCommand(sl_cli_command_arg_t *args)
{
  sl_802154_short_addr_t groupAddress = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(args, 4);
  unbindRequest(args, true,  // group addressed binding?
                &groupAddress,
                0);   // destination endpoint (unused)
}

void zdoUnbindUnicastCommand(sl_cli_command_arg_t *args)
{
  sl_802154_long_addr_t destinationEui64;
  uint8_t destinationEndpoint = sl_cli_get_argument_uint8(args, 5);

  // If the destination EUI64 of the binding (not the destination of the
  // actual message) is empty, use our local EUI64.
  if (0 == sl_zigbee_copy_eui64_arg(args, 4, destinationEui64, true)) {
    sl_zigbee_app_debug_println("Using my local EUI64 for dest EUI64 in unbinding");
    sl_zigbee_af_get_eui64(destinationEui64);
  }

  unbindRequest(args, false,   // group addressed binding?
                destinationEui64,
                destinationEndpoint);
}

void zdoRouteRequestCommand(sl_cli_command_arg_t *args)
{
  sl_802154_short_addr_t target = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(args, 0);
  uint8_t index = sl_cli_get_argument_uint8(args, 1);
  sl_status_t status = sl_zigbee_routing_table_request(target,
                                                       index,
                                                       SL_ZIGBEE_APS_OPTION_RETRY);
  UNUSED_VAR(status);
  sl_zigbee_app_debug_println("Route Table %s0x%02X", "Request: ", status);
}

//------------------------------------------------------------------------------
// Platform specific code

sl_status_t matchDescriptorsRequest(sl_802154_short_addr_t target,
                                    uint16_t profile,
                                    uint8_t inCount,
                                    uint8_t outCount,
                                    uint16_t *inClusters,
                                    uint16_t *outClusters,
                                    sl_zigbee_aps_option_t options)
{
#if defined(EZSP_HOST)
  return sl_zigbee_ezsp_match_descriptors_request(target,
                                                  profile,
                                                  inCount,
                                                  outCount,
                                                  inClusters,
                                                  outClusters,
                                                  options);
#else
  return sl_zigbee_match_descriptors_request(target,
                                             profile,
                                             inCount,
                                             inClusters,
                                             outCount,
                                             outClusters,
                                             options);
#endif // !defined(EZSP_HOST)
}
