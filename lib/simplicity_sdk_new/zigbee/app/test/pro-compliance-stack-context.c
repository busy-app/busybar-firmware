/***************************************************************************//**
 * @file
 * @brief Application implementing Zigbee Pro
 * for compliance testing.  This implements the stack-side of IPC-enabled
 * pro-compliance APIs, in order to have internal functions run in the stack
 * context as expected rather than run in the same context as CLI.
 * It is designed to be a Golden Unit and therefore utilizes certain Ember
 * internal functions for testing special behavior (i.e. changing its own
 * address to induce address conflicts).
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

//includes split out from pro-compliance.c; not all of them may actually be needed
#include "pro-compliance.h"
#include "pro-compliance-printing.h"
#include "app/util/common/internal-common.h"

#include "app/test/security-common.h"

#include "app/test/test-profile.h"
#include "app/test/zdo-common.h"
#include "app/test/groups-common.h"
#include "app/test/misc-common.h"
#include "app/test/pro-compliance-common.h"
#include "app/test/multiprotocol-sim.h"
#include "app/util/security/security.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "include/zigbee-device-stack.h"
#include "stack/include/source-route.h"
#include "app/test/mac-test-commands.h"

// Included for not yet released stack features, and
// Golden Node special functionality (e.g. forcing address conflicts)
#include "stack/core/sl_zigbee_stack.h"
#include "core/sl_zigbee_multi_network.h"
#include "stack/framework/packet-header.h"
#include "stack/framework/zigbee-packet-header.h"
#include "stack/routing/zigbee/management.h"
#include "stack/routing/zigbee/route-error.h"
#include "stack/routing/zigbee/route-table.h"
#include "stack/routing/zigbee/zigbee.h"
#include "stack/routing/zigbee/leave.h"
#include "stack/routing/zigbee/network.h"
#include "stack/routing/zigbee/association.h"
#include "stack/routing/util/retry.h"
#include "stack/zigbee/aps-security.h"
#include "stack/zigbee/aps-security-policy.h"
#include "stack/zigbee/aps-keys.h"
#include "stack/zigbee/aps-keys-full.h"
#include "stack/security/aps-keys-transient.h"
#include "stack/zigbee/trust-center-util.h"
#include "stack/zigbee/zigbee-device.h"
#include "stack/zigbee/sli_zigbee_zdo_dlk_negotiation.h"
#include "stack/zigbee/zigbee-dispatch.h"
#include "stack/zigbee/application-support.h"
#include "stack/include/zigbee-security-manager.h"
#include "stack/config/token-cache.h"
#include "stack/config/sl_zigbee_token_defines.h"
#include "stack/mac/mac-info-element-parsing.h"
#include "stack/routing/zigbee/enhanced-beacon-request.h"
#include "stack/mac/lower-mac-multi-phy.h"
#include "stack/framework/eui64.h"
#include "mac/mac.h"  // sli_802154mac_set_radio_idle_mode
#include "mac/command.h" // sli_802154mac_associate_request
#include "lower-mac.h"  // unified-mac
#include "mac-child.h"  // unified-mac
#include "mac/zigbee-upper-mac.h"  // sli_802154mac_set_radio_idle_mode
#include "stack/mac/mac-dispatch.h"
#include "indirect-queue.h"

#include "app/util/counters/counters-ota.h"
#include "app/util/counters/counters-cli.h"
#include "app/util/common/library.h"
#include "app/framework/plugin/fragmentation/fragmentation.h"
#include "stack/include/zigbee_packet_types.h"
#if defined (CORTEXM3)
    #include "cortexm3/diagnostic.h"
#endif

#if !defined(SL_ZIGBEE_TEST)
#include "sl_mbedtls.h"
#endif // SL_ZIGBEE_TEST

#include "stack/security/packet-validate.h"

#include "stack/security/security.h"
#include "stack/security/crypto.h"
#include "stack/routing/zigbee/child-handling.h"
#include "stack/routing/zigbee/child.h"
#if (defined(PHY_DUAL) || defined(PHY_SIMULATION_DUAL) || defined(PHY_SIMULATION_GB) || defined(PHY_RAILGB))
//#include "phy/plugin/duty-cycle/duty-cycle.h"
#include "duty-cycle.h"
#endif
#include "include/sl_zigbee_duty_cycle.h"
#include "stack/zigbee/source-route-table-update.h"
#include "stack/core/multi-pan.h"
#include "sl_zigbee_types_internal.h"
#include "stack/include/zigbee-security-manager.h"

// NOTE for over the air key dumps
#include "stack/framework/slx_zigbee_insecure_debug_key_trace.h"

#include "stack/include/pro_compliance_stack_interface.h"
#include "stack/internal/inc/pro_compliance_stack_interface_internal_def.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_KERNEL_PRESENT
#include "sl_zigbee_system_common.h"
#include "sl_zigbee_rtos_task_config.h"

#include "sl_event_system.h"
#if defined(SL_CATALOG_BLUETOOTH_PRESENT)
#include "sl_bluetooth.h"
#endif

#include "stack/internal/src/ipc/zigbee_ipc_callback_events.h"
#endif //SL_CATALOG_KERNEL_PRESENT

//------------------------------------------------------------------------------
// Globals

// For CLI commands that accept buffers
#define UINT8_BUFFER_PAYLOAD_SIZE   255

// Defined here to avoid including command.h
#define ASSOCIATION_SUCCESSFUL 0x00

#define RESET_NWK_FC_MASK 0x01
#define RESET_APS_FC_MASK 0x02

// Duplicated to avoid including stack/framework/zigbee-packet-header.h
#define NETWORK_REPORT_COMMAND 0x09

// buffer for organizing data before we send a datagram
#define PROFILE_ID 0xC00F
#define ENDPOINT 1
#define MSG_MULTICAST_HELLO 100
#define MULTICAST_ID 0x1111
#define HELLO_MSG_SIZE 5

// Buffer for sending network command
#define NWK_CMD_OVERHEAD    1   // to store the NWK command ID
#define NWK_CMD_MAX_PAYLOAD (UINT8_BUFFER_PAYLOAD_SIZE - NWK_CMD_OVERHEAD)

#if defined(__CYGWIN__)
// This is necessary when running simulation on Windows.
const sl_zigbee_library_status_t sli_zigbee_af_ncp_library_status = SL_ZIGBEE_LIBRARY_IS_STUB;
#endif

//------------------------------------------------------------------------------
// External Declarations

// route-discovery.c
extern bool sli_zigbee_mtorr_flow_control;
extern bool sli_zigbee_really_send_route_request(sl_802154_short_addr_t destination,
                                                 uint8_t radius,
                                                 sl_802154_short_addr_t source,
                                                 bool add_tlvs);

// Declared in lower-mac-efr32.c
extern bool useNegotiatedPowerbyLinkPowerDelta;

// Broadcast.c
extern uint8_t sli_zigbee_new_broadcast_entry_threshold;

sl_status_t sl_zigbee_add_child(sl_802154_short_addr_t shortId,
                                sl_802154_long_addr_t longId,
                                sl_zigbee_node_type_t nodeType);

extern bool slxi_zigbee_stack_network_send_command(sl_802154_short_addr_t destination,
                                                   uint8_t *commandFrame,
                                                   uint8_t length,
                                                   bool tryToInsertLongDest,
                                                   sl_802154_long_addr_t destinationEui);

//stack/mac/command.c
extern bool sli_zigbee_use_parent_long_id;

//stack/routing/child-handling.c
extern void sli_zigbee_lpd_event_handler(sli_zigbee_event_t *event);
extern bool useLegacyTimeoutMethod;

// assocation.c
extern uint16_t sli_zigbee_allow_tc_rejoins_using_well_known_key_timeout_sec;

// mac-address-filtering.c
extern void sl_zigbee_af_mac_address_filtering_init_cb(uint8_t init_level);
extern void sl_zigbee_af_mac_address_filtering_add_long_address_command(SL_CLI_COMMAND_ARG);
extern void sl_zigbee_af_mac_address_filtering_add_pan_id_command(SL_CLI_COMMAND_ARG);
extern void sl_zigbee_af_mac_address_filtering_add_short_address_command(SL_CLI_COMMAND_ARG);
extern void sl_zigbee_af_mac_address_filtering_clear_long_address_list(SL_CLI_COMMAND_ARG);
extern void sl_zigbee_af_mac_address_filtering_clear_pan_id_list(SL_CLI_COMMAND_ARG);
extern void sl_zigbee_af_mac_address_filtering_clear_short_address_list(SL_CLI_COMMAND_ARG);
extern void sl_zigbee_af_mac_address_filtering_clear_stats_command(SL_CLI_COMMAND_ARG);
extern void sl_zigbee_af_mac_address_filtering_filter_no_address_command(SL_CLI_COMMAND_ARG);
extern void sl_zigbee_af_mac_address_filtering_long_address_delete_entry(SL_CLI_COMMAND_ARG);
extern void sl_zigbee_af_mac_address_filtering_pan_id_delete_entry(SL_CLI_COMMAND_ARG);
extern void sl_zigbee_af_mac_address_filtering_print_config_command(SL_CLI_COMMAND_ARG);
extern void sl_zigbee_af_mac_address_filtering_reset(SL_CLI_COMMAND_ARG);
extern void sl_zigbee_af_mac_address_filtering_set_long_address_list_type(SL_CLI_COMMAND_ARG);
extern void sl_zigbee_af_mac_address_filtering_set_pan_id_list_type(SL_CLI_COMMAND_ARG);
extern void sl_zigbee_af_mac_address_filtering_set_short_address_list_type(SL_CLI_COMMAND_ARG);
extern void sl_zigbee_af_mac_address_filtering_short_address_delete_entry(SL_CLI_COMMAND_ARG);

extern bool sli_zigbee_add_route_entry(sl_802154_short_addr_t destination,
                                       sl_802154_short_addr_t nextHop,
                                       sli_route_table_status_t status,
                                       uint8_t age);
extern void sli_zigbee_deactivate_or_delete_active_route(sl_802154_short_addr_t destination);
extern void sli_zigbee_initialize_table_routing(uint8_t nwkIndex);
extern void sli_zigbee_source_route_clear_table(void);
extern void sli_zigbee_neighbor_process_quality(sl_802154_short_addr_t neighbor);
extern void sli_zigbee_set_end_device_configuration(uint8_t end_device_configuration);

sl_mac_scan_request_result_t orphan_scan_request_callback(uint8_t channel, sli_buffer_manager_buffer_t *scan_packet);
extern void orphanScanCompleteHandler(uint32_t channelMask);

//------------------------------------------------------------------------------
// Forward Declarations

void changeAddressCommand(SL_CLI_COMMAND_ARG);
void applicationKeyTimeoutCommand(SL_CLI_COMMAND_ARG);
void setAddressCommand(SL_CLI_COMMAND_ARG);
void sendNetworkStatusAddressConflict(SL_CLI_COMMAND_ARG);
void sendKeyCommand(SL_CLI_COMMAND_ARG);

void routeErrorCommand(SL_CLI_COMMAND_ARG);

void changePanCommand(SL_CLI_COMMAND_ARG);
void setPendingNetworkUpdatePanId(SL_CLI_COMMAND_ARG);
void setPendingNetworkUpdateChannel(SL_CLI_COMMAND_ARG);
void panConflictCommand(SL_CLI_COMMAND_ARG);
void setNetworkTokensCommand(SL_CLI_COMMAND_ARG);
void writeSecurityToken(SL_CLI_COMMAND_ARG);

#if defined SL_ZIGBEE_TEST
void hashKey(SL_CLI_COMMAND_ARG);
#endif

void joinQuietlyCommand(SL_CLI_COMMAND_ARG);
void resetFrameCounterCommand(SL_CLI_COMMAND_ARG);
void setFrameCounterCommand(SL_CLI_COMMAND_ARG);
void checkOutgoingFrameCounter(SL_CLI_COMMAND_ARG);
void tokenWritingCommand(SL_CLI_COMMAND_ARG);

void sendLeaveRequestCommand(SL_CLI_COMMAND_ARG);

void sendUpdateDeviceCommand(SL_CLI_COMMAND_ARG);

#if (defined SL_ZIGBEE_TEST || defined SL_ZIGBEE_GOLDEN_UNIT)
void sendRemoveDeviceCommand(SL_CLI_COMMAND_ARG);
void acceptOnlyNonEncryptedUpdateDeviceCommand(SL_CLI_COMMAND_ARG);
void acceptOnlyEncryptedUpdateDeviceCommand(SL_CLI_COMMAND_ARG);
void sendOnlyNonEncryptedUpdateDeviceCommand(SL_CLI_COMMAND_ARG);
#endif //(defined SL_ZIGBEE_TEST || defined SL_ZIGBEE_GOLDEN_UNIT)

void eraseKeyTableEntryZCP(uint8_t i);

void sendRawPacketCommand(SL_CLI_COMMAND_ARG);
void printExtPanIdCommand(SL_CLI_COMMAND_ARG);
void printStackStatusCommand(SL_CLI_COMMAND_ARG);

void resetNodeRebootFlag(SL_CLI_COMMAND_ARG);
void printNodeRebootFlag(SL_CLI_COMMAND_ARG);

void requestPolicyCommand(SL_CLI_COMMAND_ARG);
#if defined(SL_ZIGBEE_TEST)

void sleepOkCommand(SL_CLI_COMMAND_ARG);
#endif // SL_ZIGBEE_TEST
void nwkInitCommand(SL_CLI_COMMAND_ARG);

void addChildCommand(SL_CLI_COMMAND_ARG);
void pollOnceCommand(SL_CLI_COMMAND_ARG);
void pollLongOnceCommand(SL_CLI_COMMAND_ARG);
void pollAllNetworksOnceCommand(SL_CLI_COMMAND_ARG);
void getLastLeaveReasonCommand(SL_CLI_COMMAND_ARG);
void optionBindingTableSetCommandZCP(SL_CLI_COMMAND_ARG);
void getBindingShortId(SL_CLI_COMMAND_ARG);
void packetValidateCommand(SL_CLI_COMMAND_ARG);
void parentInfoCommand(SL_CLI_COMMAND_ARG);

// This can be turned on for XAP, we just don't have the flash / ram normally.
// The corresponding #define must be turned on in the stack as well.
// stack/security/crypto.c
//#define CRYPTO_TIMING_INFO
#if defined(SL_ZIGBEE_TEST)
  #define CRYPTO_TIMING_INFO
#endif

void resetCryptoTimingCommand(SL_CLI_COMMAND_ARG);
void enableCryptoTimingCommand(SL_CLI_COMMAND_ARG);
void printCryptoTimingCommand(SL_CLI_COMMAND_ARG);

//Sets Child NetworkTimeout
void sendNetworkTimeoutRequest(SL_CLI_COMMAND_ARG);
//modify Child NetworkTimeout locally
void modifyNetworkTimeoutInternally(SL_CLI_COMMAND_ARG);
//Sets end device timeout values
void setEndDeviceTimeout(SL_CLI_COMMAND_ARG);
//sets the end device tiemout without sending the timeout request over the air
void setSilentEndDeviceTiemout(sl_cli_command_arg_t *arguments);
//Sets keep alive mechanism
void setKeepAliveSupport(SL_CLI_COMMAND_ARG);
//Use legacy end device timeout method
void useLegacyEndDeviceTimeoutMethod(SL_CLI_COMMAND_ARG);
//Toggle sleepy end device mode
void makeEndDeviceSleepy(SL_CLI_COMMAND_ARG);
//Send a permit join request.
void sendPermitJoinRequest(SL_CLI_COMMAND_ARG);
//Print total MTORR's received.
void printMtorrReceived(SL_CLI_COMMAND_ARG);
//Send a multicast hello.
void sendMulticastHello(SL_CLI_COMMAND_ARG);

//Toggles the MTORR flow control done by the stack.
void toggleMtorrFlowControl(SL_CLI_COMMAND_ARG);
void setNewBroadcastEntryThreshold(SL_CLI_COMMAND_ARG);
void setPassiveAckConfig(SL_CLI_COMMAND_ARG);
//Attempts rejoining the network with a different device type.
void rejoinDiffDeviceType(SL_CLI_COMMAND_ARG);

//Upate Link Key after joining.
void updateLinkKey(SL_CLI_COMMAND_ARG);

//Sends a device announce.
void sendDeviceAnnounce(SL_CLI_COMMAND_ARG);

//Sends a parent announce
#ifndef SL_ZIGBEE_LEAF_STACK
void sendParentAnnounce(SL_CLI_COMMAND_ARG);
void setChildTableSize(SL_CLI_COMMAND_ARG);
#endif //SL_ZIGBEE_LEAF_STACK

//Add a Transient Key for a particular EUI.
void addTransientKeys(SL_CLI_COMMAND_ARG);

void printDcStats(SL_CLI_COMMAND_ARG);

void sendRandomPacketCommand(SL_CLI_COMMAND_ARG);
void setChannelCommand(SL_CLI_COMMAND_ARG);
void joinListAddCommand(SL_CLI_COMMAND_ARG);
void joinListRequestCommand(SL_CLI_COMMAND_ARG);
void ieeeAddressRequestCommand(SL_CLI_COMMAND_ARG);

#if     (defined(PHY_DUAL) || defined(PHY_SIMULATION_DUAL) || defined(PHY_SIMULATION_GB) || defined(PHY_RAILGB))
void makeUseofNegotiatedPower(SL_CLI_COMMAND_ARG);
void runDutyCycleTest(SL_CLI_COMMAND_ARG);
void ccaThresh(SL_CLI_COMMAND_ARG);
#endif

// Set the allowRejoins variable
// 4.7.3 Trust Center Policy Values, value 0xb6
// This value indicates if the trust center allows rejoins using well known or
// default keys. A setting of FALSE means rejoins are only allowed with trust
// center link keys where the KeyAttributes of the apsDeviceKeyPairSet entry
// indicates VERIFIED_KEY.
void setAllowRejoins(SL_CLI_COMMAND_ARG);

void concentratorActionCommand(SL_CLI_COMMAND_ARG);

void versionCommandZCP(SL_CLI_COMMAND_ARG);

void ebrAddEuiandPower(SL_CLI_COMMAND_ARG);
void ebrFindOrRemovePowerByEui64(SL_CLI_COMMAND_ARG);
void printEbrPowerAndEuiList(SL_CLI_COMMAND_ARG);
void lpdEventCommand(SL_CLI_COMMAND_ARG);
void printLinkPowerByNodeId(SL_CLI_COMMAND_ARG);

void sendNetworkCommand(SL_CLI_COMMAND_ARG);

void setDutyCycleLimitsInStack(SL_CLI_COMMAND_ARG);
void getDutyCycleLimits(SL_CLI_COMMAND_ARG);
void getCurrentDutyCycle(SL_CLI_COMMAND_ARG);
void getDutyCycleState(SL_CLI_COMMAND_ARG);

void setCorePrintingEnable(SL_CLI_COMMAND_ARG);

void addRouteCommand(SL_CLI_COMMAND_ARG);
void deleteRouteCommand(SL_CLI_COMMAND_ARG);
void clearRouteTableCommand(SL_CLI_COMMAND_ARG);
void showSourceRouteTableCommand(SL_CLI_COMMAND_ARG);
void clearSourceTableCommand(SL_CLI_COMMAND_ARG);
void assumeTcConcentratorTypeCommand(SL_CLI_COMMAND_ARG);
void routeRecordPolicyCommand(SL_CLI_COMMAND_ARG);
//------------------------------------------------------------------------------
// Command Line Stuff
void sendLPD(SL_CLI_COMMAND_ARG);
void sendMalformedLPD(SL_CLI_COMMAND_ARG);

extern sli_zigbee_packet_header_t sli_zigbee_make_network_status_message_no_network_encryption(uint8_t errorCode,
                                                                                               sl_802154_short_addr_t destination,
                                                                                               sl_802154_short_addr_t target);

extern uint8_t joinListUpdateId;

void sli_zigbee_stack_test_send_network_rejoin_command(uint8_t cmd_id,
                                                       sl_802154_long_addr_t longId,
                                                       sl_802154_short_addr_t oldShortId,
                                                       sl_802154_short_addr_t newShortId,
                                                       bool useNwkSecurity,
                                                       uint8_t status,
                                                       bool reallySend)
{
  (void) sli_zigbee_send_network_rejoin_command(cmd_id,
                                                longId,
                                                oldShortId,
                                                newShortId,
                                                useNwkSecurity,
                                                status,
                                                reallySend);
}

void sli_zigbee_stack_set_pan_id(uint16_t panId)
{
  sli_zigbee_set_pan_id(panId);
}

void sli_zigbee_stack_test_send_our_end_device_announcement(void)
{
  sli_zigbee_send_our_end_device_announcement();
}

void sli_zigbee_stack_test_send_route_error_payload_no_network_encryption(sl_802154_short_addr_t destination,
                                                                          sl_802154_short_addr_t target,
                                                                          uint8_t errorCode,
                                                                          uint8_t *payload,
                                                                          uint8_t payload_len)
{
  sli_zigbee_packet_header_t header;

  header = sli_zigbee_make_network_status_message_no_network_encryption(errorCode, destination, target);
  if (header == SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    return;
  }
  if (errorCode == SL_ZIGBEE_ROUTE_ERROR_ADDRESS_CONFLICT) {
    sli_zigbee_build_and_send_counter_info(SL_ZIGBEE_COUNTER_ADDRESS_CONFLICT_SENT, SL_ZIGBEE_NULL_NODE_ID, 0);
  }
  if (payload != NULL) {
    sl_legacy_buffer_manager_append_to_linked_buffers(header, payload, payload_len);
    if (header == SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
      return;
    }
  }
  sli_zigbee_network_send(header);

  sl_legacy_buffer_manager_release_message_buffer(header);
}

bool sli_zigbee_stack_test_send_link_key(sl_802154_short_addr_t targetNodeId,
                                         sl_802154_long_addr_t targetEui64,
                                         uint8_t keyType,
                                         sl_zigbee_key_data_t* key,
                                         bool useApsEncryption)
{
  return sli_zigbee_send_link_key(targetNodeId,
                                  targetEui64,
                                  (sli_zigbee_key_type_t) keyType,
                                  key,
                                  useApsEncryption);
}

void sli_zigbee_stack_test_send_route_error_payload(sl_802154_short_addr_t destination,
                                                    sl_802154_short_addr_t target,
                                                    uint8_t errorCode,
                                                    uint8_t *payload,
                                                    uint8_t payload_len)
{
  sli_zigbee_send_route_error_payload(destination, target, errorCode, payload, payload_len);
}

sl_status_t sli_zigbee_stack_test_send_route_request_with_tlv(sl_802154_short_addr_t target)
{
  bool success = sli_zigbee_really_send_route_request(target,
                                                      0,  // radius
                                                      sli_zigbee_stack_get_node_id(),
                                                      true); // add TLVs
  return success ? SL_STATUS_OK : SL_STATUS_FAIL;
}

bool sli_zigbee_stack_test_send_report_or_update(uint8_t command,
                                                 uint8_t updateId,
                                                 uint16_t panId)
{
  return sli_zigbee_send_report_or_update(command, updateId, panId);
}

void sli_zigbee_stack_test_set_network_tokens(uint8_t stackProfile,
                                              uint8_t nodeType,
                                              uint8_t channel,
                                              int8_t power,
                                              uint16_t nodeId,
                                              uint16_t panId,
                                              sl_802154_long_addr_t extendedPanId)
{
  tokTypeStackNodeData tokNode;

  tokNode.stackProfile     = stackProfile;
  tokNode.nodeType         = nodeType;
  tokNode.radioFreqChannel = channel;
  tokNode.radioTxPower     = power;
  tokNode.zigbeeNodeId     = nodeId;
  tokNode.panId            = panId;
  memmove(tokNode.extendedPanId, extendedPanId, EUI64_SIZE);

  sli_zigbee_set_stack_token(COMMON_TOKEN_STACK_NODE_DATA, &tokNode, sizeof(tokNode));
}

void sli_zigbee_stack_test_reset_frame_counter(uint8_t mask)
{
  if (mask & RESET_NWK_FC_MASK) {
    sli_zigbee_reset_nwk_outgoing_frame_counter();
  }
  if (mask & RESET_APS_FC_MASK) {
    sli_zigbee_reset_aps_frame_counter();
  }
}

sl_status_t sli_zigbee_stack_test_send_leave_request_command(uint16_t destId,
                                                             sl_802154_long_addr_t destEui)
{
  sli_zigbee_packet_header_t header;
  uint8_t frame[2];
  frame[0] = ZIGBEE_LEAVE_COMMAND;
  // Bit "Request" set.
  frame[1] = 0x40;
  sl_status_t status = SL_STATUS_FAIL;

  header = sli_zigbee_make_zigbee_command_header(destId,
                                                 1, // radius
                                                 frame, // payload
                                                 2, // length
                                                 true, // has long id destination.
                                                 destEui);

  if (header == SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    sl_zigbee_core_debug_println("NWK Leave sending failed, no buffers");
    return SL_STATUS_ALLOCATION_FAILED;
  }

  status = ((sl_zigbee_is_zigbee_broadcast_address(destId)
             ? sli_zigbee_retry_submit(header,
                                       ZIGBEE_MAX_BROADCAST_RETRIES,
                                       0,
                                       SLI_ZIGBEE_RETRY_FLAG_LOCAL_ORIGIN)
             : sli_zigbee_network_submit_unicast(header, destId))
            ? SL_STATUS_OK
            : SL_STATUS_FAIL);

  sl_legacy_buffer_manager_release_message_buffer(header);
  return status;
}

bool sli_zigbee_stack_test_send_device_update(uint16_t newShortId,
                                              sl_802154_long_addr_t newLongId,
                                              bool apsEncryption,
                                              uint8_t deviceStatus)
{
  sli_buffer_manager_buffer_t message;
  bool status = false;

  if (!sli_zigbee_distributed_trust_center_mode_enabled() ) {
    message = sli_legacy_packet_buffer_make_message(0,
                                                    "1821",
                                                    APS_COMMAND_UPDATE_DEVICE,
                                                    newLongId,
                                                    newShortId,
                                                    deviceStatus);

    if (message != SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
      status = sli_zigbee_send_aps_command(sli_zigbee_get_trust_center_node_id(),
                                           NULL,
                                           message,
                                           (apsEncryption
                                            ? (ENCRYPTION_NETWORK | ENCRYPTION_APS)
                                            : ENCRYPTION_NETWORK));
      sl_legacy_buffer_manager_release_message_buffer(message);
    }
  }
  return status;
}

sl_status_t sli_zigbee_stack_test_send_remove_device_command(uint16_t destId,
                                                             sl_802154_long_addr_t destEui,
                                                             sl_802154_long_addr_t deviceToRemoveEui,
                                                             bool sendNonEncrypted)
{
  sl_status_t status = SL_STATUS_OK;
  sli_buffer_manager_buffer_t message;
  sli_zigbee_send_aps_command_options_t options = ENCRYPTION_NETWORK;
  uint8_t contents[REMOVE_DEVICE_FRAME_SIZE];
  if (sendNonEncrypted) {
    sli_zigbee_send_non_encrypted_remove_device = true;
  }

  contents[0] = APS_COMMAND_REMOVE_DEVICE;
  memmove(contents + 1, deviceToRemoveEui, EUI64_SIZE);

  message = sl_legacy_buffer_manager_fill_linked_buffers(contents, REMOVE_DEVICE_FRAME_SIZE);

  if ( message == SL_ZIGBEE_NULL_MESSAGE_BUFFER ) {
    status = SL_STATUS_ALLOCATION_FAILED;
  }

  if (!sli_zigbee_send_aps_command(destId,
                                   destEui,
                                   message,
                                   options)) {
    status = SL_STATUS_INVALID_STATE;
  }
  sl_legacy_buffer_manager_release_message_buffer(message);

  sli_zigbee_send_non_encrypted_remove_device = false;
  return status;
}

void sli_zigbee_stack_test_send_network_timeout_request(uint8_t requestedTimeoutValue)
{
  sli_zigbee_send_network_timeout_request(requestedTimeoutValue);
}

void sli_zigbee_stack_set_end_device_poll_timeout(uint8_t timeout)
{
  sli_zigbee_stack_end_device_poll_timeout = timeout;
}

void sli_zigbee_stack_test_send_timeout_request(void)
{
  sli_zigbee_send_timeout_request();
}

void sli_zigbee_stack_test_spoof_device_announcement(uint16_t shortId,
                                                     uint8_t *sourceEUI64,
                                                     sl_802154_long_addr_t deviceAnnounceEui,
                                                     uint8_t capabilities)
{
  sli_zigbee_spoof_device_announcement(shortId,
                                       sourceEUI64,
                                       deviceAnnounceEui,
                                       capabilities);
}

void sli_zigbee_stack_test_join_list_request(uint8_t startIndex)
{
  sl_zigbee_aps_frame_t apsFrame;

  sli_zigbee_zig_dev_prepare_zdo_request(&apsFrame,
                                         NWK_UPDATE_IEEE_JOINING_LIST_REQUEST,
                                         0); // aps options
  apsFrame.sourceEndpoint = SL_ZIGBEE_ZDO_ENDPOINT;
  apsFrame.options |= SL_ZIGBEE_APS_OPTION_RETRY;

  sli_zigbee_send_zig_dev_message(0x0000,
                                  &apsFrame,
                                  "1",
                                  startIndex);
}

void sli_zigbee_stack_test_join_list_add(uint8_t command,
                                         uint8_t* eui64List,
                                         uint8_t counts)
{
  uint8_t index = 0;

  sl_zigbee_aps_frame_t apsFrame;
  sl_802154_long_addr_t* longIdList = (sl_802154_long_addr_t*) eui64List;
  if (command == 1) {
    for (uint8_t i = 0; i < counts; i++) {
      sli_zigbee_join_list_add(longIdList[i]);
    }

    joinListUpdateId++;
    sli_zigbee_zig_dev_prepare_zdo_response(&apsFrame,
                                            NWK_UPDATE_IEEE_JOINING_LIST_REPONSE,
                                            0,
                                            0);
    apsFrame.sourceEndpoint = SL_ZIGBEE_ZDO_ENDPOINT;
    apsFrame.options |= SL_ZIGBEE_APS_OPTION_RETRY;
    //broadcast the list from index zero
    sli_zigbee_handle_ieee_joining_list_request(SL_ZIGBEE_BROADCAST_ADDRESS, &apsFrame, 0, true, &index);
  } else if (command == 0) {
    sli_zigbee_join_list_delete(longIdList[0]);
    joinListUpdateId++;
  } else if (command == 10) {
    sli_zigbee_join_list_clear();
    joinListUpdateId++;
  } else if (command == 20) {
    sli_buffer_manager_buffer_t ieeeListPrt = sli_zigbee_get_join_list_pointer();
    if (ieeeListPrt != SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
      sl_zigbee_core_debug_println("BEGIN IEEE LIST");
      for (uint8_t i = 0; i < (sl_legacy_buffer_manager_message_buffer_length(ieeeListPrt) / EUI64_SIZE); i++) {
        uint8_t *bp = sl_legacy_buffer_manager_get_linked_buffers_pointer(ieeeListPrt, i * EUI64_SIZE);
        printBigEndianEui64(serialPort, bp);
        sl_zigbee_core_debug_println("");
      }
      sl_zigbee_core_debug_println("END IEEE LIST");
    } else {
      sl_zigbee_core_debug_println("IEEE join list is empty");
    }
  }
}

sl_status_t sli_zigbee_stack_test_ieee_address_request_to_target(sl_802154_short_addr_t discoveryNodeId,
                                                                 bool reportKids,
                                                                 uint8_t childStartIndex,
                                                                 uint8_t sourceEndpoint,
                                                                 sl_zigbee_aps_option_t options,
                                                                 sl_802154_short_addr_t targetNodeIdOfRequest)
{
  return sli_zigbee_ieee_address_request_to_target(discoveryNodeId,
                                                   reportKids,
                                                   childStartIndex,
                                                   sourceEndpoint,
                                                   options,
                                                   targetNodeIdOfRequest);
}

bool sli_zigbee_stack_test_network_send_command(sl_802154_short_addr_t destination,
                                                uint8_t *commandFrame,
                                                uint8_t length,
                                                bool tryToInsertLongDest,
                                                sl_802154_long_addr_t destinationEui)
{
  return slxi_zigbee_stack_network_send_command(destination,
                                                commandFrame,
                                                length,
                                                tryToInsertLongDest,
                                                destinationEui);
}

bool sli_mac_stack_lower_mac_radio_is_on(uint8_t mac_index)
{
  return sli_mac_lower_mac_radio_is_on(mac_index);
}

sl_status_t sli_mac_stack_lower_mac_set_radio_channel(uint8_t mac_index, uint8_t channel)
{
  return sli_mac_lower_mac_set_radio_channel(mac_index, channel);
}

sl_status_t sli_mac_stack_lower_mac_set_radio_power(uint8_t mac_index, int8_t power)
{
  return sli_mac_lower_mac_set_radio_power(mac_index, power);
}

sl_status_t sli_mac_stack_lower_mac_set_radio_idle_mode(uint8_t mac_index, uint8_t mode)
{
  return sli_mac_lower_mac_set_radio_idle_mode(mac_index, mode);
}

void sli_mac_stack_lower_mac_radio_sleep(void)
{
  sli_mac_lower_mac_radio_sleep();
}

void sli_mac_stack_lower_mac_radio_wakeup(void)
{
  sli_mac_lower_mac_radio_wakeup();
}

//consolidate with sl_zigbee_lookup_node_id_by_eui64?
sl_802154_short_addr_t sli_mac_stack_find_child_short_id(sl_802154_long_addr_t eui64)
{
  return sli_mac_find_child_short_id(eui64);
}

void sli_mac_stack_set_coordinator(bool isCoordinator)
{
  sli_mac_set_coordinator(isCoordinator);
}

//sl_mac_child_status_flags_t is 64-bit, but not visible to IPC generator
uint64_t sli_mac_stack_get_child_info_flags(uint8_t childIndex)
{
  return sli_mac_get_child_info_flags(childIndex);
}

void sli_mac_stack_test_send_mac_command(uint8_t macContentsLength, uint8_t* macContents)
{
  uint8_t *packetContents;
  uint8_t macHeaderLength = 0;

  sli_zigbee_packet_header_t header = sli_zigbee_make_raw_packet_header(SL_802154_INFO_TYPE_PASSTHROUGH, SL_ZIGBEE_NULL_MESSAGE_BUFFER);

  if (header == SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    sl_zigbee_core_debug_println("error: no buffers");
    return;
  }

  // Append a dummy PHY byte. This is so that emMac calls work later (they
  // operate on buffers having PHY pointers, not MAC pointers)
  packetContents = sli_zigbee_packet_header_contents(header);
  sl_legacy_buffer_manager_append_to_linked_buffers(header,
                                                    macContents,
                                                    macContentsLength);
  //(void) sli_legacy_serial_printf_line(1,"message buffer length %d", sl_legacy_buffer_manager_message_buffer_length(header));

  // Skip mac header verification if it is reserved frameType.
  // This is to run reserved/invalid FCS (frame control sub field)
  // test cases such as Frame-Validation-01 & -02.
  if (sli_mac_header_mac_info_frame_type(header) & MAC_FRAME_TYPE_RESERVED_MASK) {
    macHeaderLength = 1; // Give any value such that it pass validation check.
  } else {
    // Verify mac header
    macHeaderLength = sli_mac_flat_mac_header_length(packetContents, false);
  }

  if (macHeaderLength == 0 || macContentsLength < macHeaderLength) {
    sl_zigbee_core_debug_println("error: invalid MAC header macHeaderLength %d", macHeaderLength);
  } else {
    if ((sli_802154mac_destination_mode(header)
         == MAC_FRAME_DESTINATION_MODE_SHORT)
        && sli_802154mac_short_destination(header) == 0xFFFF) {
      sl_zigbee_core_debug_println("broadcast: tx direct, size %d", macContentsLength);
      sli_802154mac_submit(header, SL_802154_TRANSMIT_PRIORITY_NORMAL);
    } else {
      sl_802154_short_addr_t nodeId = 0xFFFF;
      switch (sli_802154mac_destination_mode(header)) {
        case MAC_FRAME_DESTINATION_MODE_SHORT:
          nodeId = sli_802154mac_short_destination(header);
          break;
        case MAC_FRAME_DESTINATION_MODE_LONG:
          nodeId = sl_mac_find_child_short_id(sli_802154mac_destination_pointer(header));
          break;
        default:
          sl_zigbee_core_debug_println("warning: invalid MAC frame destination mode %d", sli_802154mac_destination_mode(header));
          break;
      }

      if (sli_mac_child_id_is_sleepy(nodeId)) {
        sl_zigbee_core_debug_println("tx indirect, size %d", macContentsLength);
        sli_mac_indirect_submit(header);
      } else {
        sl_zigbee_core_debug_println("tx direct, size %d", macContentsLength);
        sli_802154mac_submit(header, SL_802154_TRANSMIT_PRIORITY_NORMAL);
      }
    }
  }
  sl_legacy_buffer_manager_release_message_buffer(header);
}

void sli_mac_stack_kickstart(uint8_t mac_index)
{
  sli_mac_kickstart(mac_index);
}

void sli_mac_stack_indirect_purge(uint8_t nwk_index)
{
  sli_mac_indirect_purge(nwk_index);
}

void sli_zigbee_stack_test_perform_raw_active_scan(uint32_t scanChannels, uint8_t scanDuration)
{
  // For MAC certification SCANNING -7/8 tests - Use the mac api to perform the orphan scan
  // and handle the responses.
  sli_mac_raw_active_scan(scanChannels,
                          scanDuration,
                          &orphan_scan_request_callback, //equivalent to sendorphanNotification
                          sl_zigbee_get_radio_power(),
                          &sli_802154phy_radio_receive_complete_callback,
                          &orphanScanCompleteHandler,
                          true);                       // set_pan_id_to_broadcast_pan
}

sl_status_t sli_mac_stack_test_set_tx_power(int8_t power)
{
  sl_mac_radio_parameters_t radio_parameters;
  uint8_t network_index = sli_zigbee_stack_get_current_network();
  sli_mac_get_nwk_radio_parameters(PHY_INDEX_NATIVE, network_index, &radio_parameters);
  radio_parameters.tx_power = power;
  return sli_mac_set_nwk_radio_parameters(PHY_INDEX_NATIVE, network_index, &radio_parameters);
}

sl_status_t sli_mac_stack_test_set_nwk_radio_params_channel(uint8_t channel)
{
  sl_mac_radio_parameters_t radio_parameters;
  uint8_t network_index = sli_zigbee_stack_get_current_network();
  sli_mac_get_nwk_radio_parameters(PHY_INDEX_NATIVE, network_index, &radio_parameters);
  radio_parameters.channel = channel;
  return sli_mac_set_nwk_radio_parameters(PHY_INDEX_NATIVE, network_index, &radio_parameters);
}

sl_status_t sli_mac_stack_test_set_nwk_radio_params_eui(uint8_t network_index, sl_802154_long_addr_t eui)
{
  (void)eui;
  sl_mac_radio_parameters_t radio_parameters;
  sli_mac_get_nwk_radio_parameters(PHY_INDEX_NATIVE, network_index, &radio_parameters);
  memcpy(radio_parameters.local_eui, sli_zigbee_stack_get_eui64(), EUI64_SIZE);
  return sli_mac_set_nwk_radio_parameters(PHY_INDEX_NATIVE, network_index, &radio_parameters);
}

sl_status_t sli_mac_stack_test_associate_command(sl_802154_short_addr_t parentId, uint16_t panId)
{
  uint8_t capabilities   = sli_zigbee_get_local_capabilities();

  sl_status_t status = sli_802154mac_associate_request(0, parentId, panId, capabilities);

  if (status == SL_STATUS_OK) {
    sl_zigbee_set_node_id(EM_USE_LONG_ADDRESS);
    sl_zigbee_set_pan_id(panId);
    sli_zigbee_parent_id = parentId;
    sli_zigbee_set_join_method(SL_ZIGBEE_USE_MAC_ASSOCIATION);
    sli_zigbee_state = NETWORK_JOINING;
    sli_zigbee_set_join_poll_attemps_remaining(3);
    sli_zigbee_set_zigbee_event_network_index(ZIGBEE_NWK_INDEX_OFFSET_ASSOCIATION_EVENT);
    sli_zigbee_event_set_delay_ms(&sli_zigbee_association_event, 200);
  }
  return status;
}

void sli_zigbee_stack_set_eui64(sl_802154_long_addr_t eui64)
{
  sli_zigbee_set_eui64(eui64);
}

extern bool slx_zigbee_zdo_dlk_save_derived_key;
void sli_zigbee_stack_set_zdo_dlk_save_derived_key(bool value)
{
  slx_zigbee_zdo_dlk_save_derived_key = value;
}
extern void sli_zigbee_set_ignore_aps_acks(bool ignore);
void sli_zigbee_stack_set_ignore_aps_acks(bool ignore)
{
  sli_zigbee_set_ignore_aps_acks(ignore);
}

extern uint8_t sli_bdb_tclk_max_exchange_attempts(void);
uint8_t sli_zigbee_stack_bdb_tclk_max_exchange_attempts(void)
{
  return sli_bdb_tclk_max_exchange_attempts();
}

extern uint8_t sli_zigbee_stack_compliance_revision;
void sli_zigbee_stack_set_stack_compliance_revision(uint8_t revision)
{
  sli_zigbee_stack_compliance_revision = revision;
}

sl_status_t sli_zigbee_stack_request_link_key_with_option_encrypt(sl_802154_long_addr_t partner, uint8_t option)
{
  uint8_t frame[REQUEST_KEY_FRAME_SIZE];
  uint8_t size;
  sli_buffer_manager_buffer_t payload;
  sl_802154_long_addr_t trustCenterAddress;
  sl_802154_short_addr_t tcNodeId = 0x0000;
  bool success;
  bool isTrustCenterLinkKeyRequest = false;

  if (sli_zigbee_am_trust_center
      || sli_zigbee_stack_security_level() == 0
      || !sli_zigbee_get_trust_center_eui64(trustCenterAddress)
      || (partner != NULL
          && sli_zigbee_stack_is_local_eui64(partner))) {
    return SL_STATUS_INVALID_STATE;
  }

  // Only Requests for the TC Link Key are allowed in
  // No TC mode and only while joining.  Requests after
  // the fact means that devices may not be getting a Link
  // during joining which means the Link Key is not the same
  // throughout the network.
  if ( sli_zigbee_distributed_trust_center_mode_enabled() ) {
    if ( sli_zigbee_state != NETWORK_JOINED_UNAUTHENTICATED ) {
      return SL_STATUS_INVALID_STATE;
    }
    tcNodeId = sli_zigbee_parent_id;
  }

  if (partner == NULL
      || (0 == memcmp(trustCenterAddress, partner, EUI64_SIZE))) {
    isTrustCenterLinkKeyRequest = true;
  }

  frame[0] = APS_COMMAND_REQUEST_KEY;

  if (isTrustCenterLinkKeyRequest) {
    frame[1] = REQUEST_TRUST_CENTER_LINK_KEY;
    size = 2;
  } else {
    frame[1] = REQUEST_APP_LINK_KEY;
    memmove(frame + 2, partner, EUI64_SIZE);
    size = 10;
  }

  payload = sl_legacy_buffer_manager_fill_linked_buffers(frame, size);

  if (payload == SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  success = sli_zigbee_send_aps_command(tcNodeId,
                                        NULL,
                                        payload,
                                        (option
                                         ? ENCRYPTION_NETWORK
                                         : ENCRYPTION_NONE)
                                        | ENCRYPTION_APS
                                        | (sli_zigbee_distributed_trust_center_mode_enabled()
                                           ? SEND_APS_COMMAND_TO_CHILD
                                           : 0));
  if (success) {
    sli_zigbee_start_request_key_timeout(false);  // do not use BDB timeout values
    if (isTrustCenterLinkKeyRequest) {
      sli_zigbee_set_update_link_key_request(true); // request has been sent
    }
  }
  sl_legacy_buffer_manager_release_message_buffer(payload);
  return (success ? SL_STATUS_OK : SL_STATUS_ALLOCATION_FAILED);
}

sl_status_t sli_zigbee_stack_send_aps_ack(sl_zigbee_aps_frame_t apsStruct, sl_802154_short_addr_t dest)
{
  if (apsStruct.sequence == 0) {
    apsStruct.sequence = sli_zigbee_current_aps_struct.sequence;
  }
  apsStruct.radius = sli_zigbee_current_aps_struct.radius;

  return sli_zigbee_send_aps_message(SL_ZIGBEE_OUTGOING_UNICAST_REPLY,
                                     dest,
                                     &apsStruct,
                                     0xFF, // use maximum radius
                                     SL_ZIGBEE_NULL_MESSAGE_BUFFER,
                                     NULL);
}
