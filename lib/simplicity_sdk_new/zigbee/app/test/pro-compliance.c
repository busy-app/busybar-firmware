/***************************************************************************//**
 * @file
 * @brief Application implementing Zigbee Pro
 * for compliance testing.  This implements the onboard version (non-host).
 * It is designed to be a Golden Unit and therefore utilizes certain Ember
 * internal functions for testing special behavior (i.e. changing its own
 * address to induce address conflicts).
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
#include "stack/include/zigbee-security-manager.h"
#include "stack/include/zigbee_packet_types.h"
#include "stack/include/message.h"
#include "stack/config/token-cache.h"
#include "stack/config/sl_zigbee_token_defines.h"
#include "stack/mac/mac-info-element-parsing.h"
#include "stack/routing/zigbee/enhanced-beacon-request.h"
#include "stack/mac/lower-mac-multi-phy.h"
#include "mac/mac.h"  // sli_802154mac_set_radio_idle_mode
#include "lower-mac.h"  // unified-mac
#include "mac-child.h"  // unified-mac
#include "mac/zigbee-upper-mac.h"  // sli_802154mac_set_radio_idle_mode

#include "app/util/counters/counters-ota.h"
#include "app/util/counters/counters-cli.h"
#include "app/util/common/library.h"
#include "app/framework/plugin/fragmentation/fragmentation.h"
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
#include "stack/include/sl_zigbee_types_internal.h"
#include "stack/include/zigbee-security-manager.h"

// NOTE for over the air key dumps
#include "stack/framework/slx_zigbee_insecure_debug_key_trace.h"

#include "stack/include/pro_compliance_stack_interface.h"

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

#if defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)
#include "af.h"
#endif

#include "stack/internal/src/ipc/zigbee_ipc_callback_events.h"
#endif //SL_CATALOG_KERNEL_PRESENT

//------------------------------------------------------------------------------
// Globals
static uint8_t MtorrReceived = 0;
static bool use_network_security = true;
uint8_t joinDecision = SL_ZIGBEE_SEND_KEY_IN_THE_CLEAR;

// Added for monitoring whether a node has rebooted during its execution.
bool nodeRebootFlag = true;

const char * applicationString = "ZigBee Pro Compliance Test App";

uint8_t sl_zigbee_endpoint_count = TEST_PROFILE_ENDPOINT_COUNT;

sl_zigbee_endpoint_t sl_zigbee_endpoints[] = {
  ENDPOINT_TEST_DRIVER_DEFINITION,
  ENDPOINT_FULL_DEVICE_1_DEFINITION,
  ENDPOINT_FULL_DEVICE_2_DEFINITION
};

sl_zigbee_key_data_t zigbeeAlliance09Key = { { 0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C,
                                               0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 } };

#if     (defined(PHY_DUAL) || defined(PHY_SIMULATION_DUAL) || defined(PHY_SIMULATION_GB) || defined(PHY_RAILGB))
static void dcTestEventHandler(sl_zigbee_af_event_t * event);

// TODO: Once we fully port the zigbee_pro_compliance app to UC, we can
// get rid of the realDcTestEvent and just use dcTestEvent.
sl_zigbee_af_event_t realDcTestEvent;
#define dcTestEvent (&realDcTestEvent)
#endif // (defined(PHY_DUAL) || defined(PHY_SIMULATION_DUAL) || defined(PHY_SIMULATION_GB) || defined(PHY_RAILGB))

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

// This controls whether to print emberAfCorePrints, which are found in the
// zigbee_pro_compliance binary (such as the mac-address-filtering files)
bool proComplianceCorePrintingEnabled = false;

//------------------------------------------------------------------------------
// External Declarations

// route-discovery.c
extern bool sli_zigbee_mtorr_flow_control;

// Declared in lower-mac-efr32.c
extern bool useNegotiatedPowerbyLinkPowerDelta;

// Broadcast.c
extern uint8_t sli_zigbee_new_broadcast_entry_threshold;

sl_status_t sl_zigbee_add_child(sl_802154_short_addr_t shortId,
                                sl_802154_long_addr_t longId,
                                sl_zigbee_node_type_t nodeType);

//stack/mac/command.c
extern bool sli_zigbee_use_parent_long_id;

//stack/routing/child-handling.c
extern void sli_zigbee_lpd_event_handler(sli_zigbee_event_t *event);
extern bool useLegacyTimeoutMethod;

// assocation.c
extern uint16_t sli_zigbee_allow_tc_rejoins_using_well_known_key_timeout_sec;

// mac-address-filtering.c
extern void sl_zigbee_af_mac_address_filtering_init_cb(uint8_t init_level);
extern void sl_zigbee_af_mac_address_filtering_add_long_address_command(sl_cli_command_arg_t *arguments);
extern void sl_zigbee_af_mac_address_filtering_add_pan_id_command(sl_cli_command_arg_t *arguments);
extern void sl_zigbee_af_mac_address_filtering_add_short_address_command(sl_cli_command_arg_t *arguments);
extern void sl_zigbee_af_mac_address_filtering_clear_long_address_list(sl_cli_command_arg_t *arguments);
extern void sl_zigbee_af_mac_address_filtering_clear_pan_id_list(sl_cli_command_arg_t *arguments);
extern void sl_zigbee_af_mac_address_filtering_clear_short_address_list(sl_cli_command_arg_t *arguments);
extern void sl_zigbee_af_mac_address_filtering_clear_stats_command(sl_cli_command_arg_t *arguments);
extern void sl_zigbee_af_mac_address_filtering_filter_no_address_command(sl_cli_command_arg_t *arguments);
extern void sl_zigbee_af_mac_address_filtering_long_address_delete_entry(sl_cli_command_arg_t *arguments);
extern void sl_zigbee_af_mac_address_filtering_pan_id_delete_entry(sl_cli_command_arg_t *arguments);
extern void sl_zigbee_af_mac_address_filtering_print_config_command(sl_cli_command_arg_t *arguments);
extern void sl_zigbee_af_mac_address_filtering_reset(sl_cli_command_arg_t *arguments);
extern void sl_zigbee_af_mac_address_filtering_set_long_address_list_type(sl_cli_command_arg_t *arguments);
extern void sl_zigbee_af_mac_address_filtering_set_pan_id_list_type(sl_cli_command_arg_t *arguments);
extern void sl_zigbee_af_mac_address_filtering_set_short_address_list_type(sl_cli_command_arg_t *arguments);
extern void sl_zigbee_af_mac_address_filtering_short_address_delete_entry(sl_cli_command_arg_t *arguments);

extern bool sli_zigbee_add_route_entry(sl_802154_short_addr_t destination,
                                       sl_802154_short_addr_t nextHop,
                                       sli_route_table_status_t status,
                                       uint8_t age);
extern void sli_zigbee_deactivate_or_delete_active_route(sl_802154_short_addr_t destination);
extern void sli_zigbee_initialize_table_routing(uint8_t nwkIndex);
extern void sli_zigbee_source_route_clear_table(void);
extern void sli_zigbee_neighbor_process_quality(sl_802154_short_addr_t neighbor);
extern void sli_zigbee_set_end_device_configuration(uint8_t end_device_configuration);

//------------------------------------------------------------------------------
// Forward Declarations

void changeAddressCommand(sl_cli_command_arg_t *arguments);
void applicationKeyTimeoutCommand(sl_cli_command_arg_t *arguments);
void setAddressCommand(sl_cli_command_arg_t *arguments);
void sendNetworkStatusAddressConflict(sl_cli_command_arg_t *arguments);
void sendKeyCommand(sl_cli_command_arg_t *arguments);

//static void derivedKeysCommand(void);
void routeErrorCommand(sl_cli_command_arg_t *arguments);

void changePanCommand(sl_cli_command_arg_t *arguments);
void setPendingNetworkUpdatePanId(sl_cli_command_arg_t *arguments);
void setPendingNetworkUpdateChannel(sl_cli_command_arg_t *arguments);
void panConflictCommand(sl_cli_command_arg_t *arguments);
void setNetworkTokensCommand(sl_cli_command_arg_t *arguments);
void writeSecurityToken(sl_cli_command_arg_t *arguments);

#if defined SL_ZIGBEE_TEST
void hashKey(sl_cli_command_arg_t *arguments);
#endif

void joinQuietlyCommand(sl_cli_command_arg_t *arguments);
void resetFrameCounterCommand(sl_cli_command_arg_t *arguments);
void setFrameCounterCommand(sl_cli_command_arg_t *arguments);
void checkOutgoingFrameCounter(sl_cli_command_arg_t *arguments);
void tokenWritingCommand(sl_cli_command_arg_t *arguments);

void sendLeaveRequestCommand(sl_cli_command_arg_t *arguments);

void sendUpdateDeviceCommand(sl_cli_command_arg_t *arguments);

#if (defined SL_ZIGBEE_TEST || defined SL_ZIGBEE_GOLDEN_UNIT)
void sendRemoveDeviceCommand(sl_cli_command_arg_t *arguments);
void acceptOnlyNonEncryptedUpdateDeviceCommand(sl_cli_command_arg_t *arguments);
void acceptOnlyEncryptedUpdateDeviceCommand(sl_cli_command_arg_t *arguments);
void sendOnlyNonEncryptedUpdateDeviceCommand(sl_cli_command_arg_t *arguments);
#endif //(defined SL_ZIGBEE_TEST || defined SL_ZIGBEE_GOLDEN_UNIT)

void eraseKeyTableEntry(uint8_t i);

void sendRawPacketCommand(sl_cli_command_arg_t *arguments);
void sendRawPacketCommandWithTag(sl_cli_command_arg_t *arguments);
void printExtPanIdCommand(sl_cli_command_arg_t *arguments);
void printStackStatusCommand(sl_cli_command_arg_t *arguments);

void resetNodeRebootFlag(sl_cli_command_arg_t *arguments);
void printNodeRebootFlag(sl_cli_command_arg_t *arguments);

void requestPolicyCommand(sl_cli_command_arg_t *arguments);
#if defined(SL_ZIGBEE_TEST)

void sleepOkCommand(sl_cli_command_arg_t *arguments);
#endif // SL_ZIGBEE_TEST
void nwkInitCommand(sl_cli_command_arg_t *arguments);

void addChildCommand(sl_cli_command_arg_t *arguments);
void pollOnceCommand(sl_cli_command_arg_t *arguments);
void pollLongOnceCommand(sl_cli_command_arg_t *arguments);
void pollAllNetworksOnceCommand(sl_cli_command_arg_t *arguments);
void getLastLeaveReasonCommand(sl_cli_command_arg_t *arguments);
void optionBindingTableSetCommand(sl_cli_command_arg_t *arguments);
void getBindingShortId(sl_cli_command_arg_t *arguments);
void packetValidateCommand(sl_cli_command_arg_t *arguments);
void parentInfoCommand(sl_cli_command_arg_t *arguments);

// This can be turned on for XAP, we just don't have the flash / ram normally.
// The corresponding #define must be turned on in the stack as well.
// stack/security/crypto.c
//#define CRYPTO_TIMING_INFO
#if defined(SL_ZIGBEE_TEST)
  #define CRYPTO_TIMING_INFO
#endif

void resetCryptoTimingCommand(sl_cli_command_arg_t *arguments);
void enableCryptoTimingCommand(sl_cli_command_arg_t *arguments);
void printCryptoTimingCommand(sl_cli_command_arg_t *arguments);

//Sets Child NetworkTimeout
void sendNetworkTimeoutRequest(sl_cli_command_arg_t *arguments);
//modify Child NetworkTimeout locally
void modifyNetworkTimeoutInternally(sl_cli_command_arg_t *arguments);
//Sets end device timeout values
void setEndDeviceTimeout(sl_cli_command_arg_t *arguments);

//sets the end device tiemout without sending the timeout request over the air
void setSilentEndDeviceTiemout(sl_cli_command_arg_t *arguments);

//Sets keep alive mechanism
void setKeepAliveSupport(sl_cli_command_arg_t *arguments);
//Use legacy end device timeout method
void useLegacyEndDeviceTimeoutMethod(sl_cli_command_arg_t *arguments);
//Toggle sleepy end device mode
void makeEndDeviceSleepy(sl_cli_command_arg_t *arguments);
//Send a permit join request.
void sendPermitJoinRequest(sl_cli_command_arg_t *arguments);
//Print total MTORR's received.
void printMtorrReceived(sl_cli_command_arg_t *arguments);
//Send a multicast hello.
void sendMulticastHello(sl_cli_command_arg_t *arguments);

//Toggles the MTORR flow control done by the stack.
void toggleMtorrFlowControl(sl_cli_command_arg_t *arguments);
void setNewBroadcastEntryThreshold(sl_cli_command_arg_t *arguments);
void setPassiveAckConfig(sl_cli_command_arg_t *arguments);
//Attempts rejoining the network with a different device type.
void rejoinDiffDeviceType(sl_cli_command_arg_t *arguments);

//Upate Link Key after joining.
void updateLinkKey(sl_cli_command_arg_t *arguments);

//Sends a device announce.
void sendDeviceAnnounce(sl_cli_command_arg_t *arguments);

//Sends a parent announce
#ifndef SL_ZIGBEE_LEAF_STACK
void sendParentAnnounce(sl_cli_command_arg_t *arguments);
void setChildTableSize(sl_cli_command_arg_t *arguments);
#endif //SL_ZIGBEE_LEAF_STACK

//Add a Transient Key for a particular EUI.
void addTransientKeys(sl_cli_command_arg_t *arguments);

void printDcStats(sl_cli_command_arg_t *arguments);

void sendRandomPacketCommand(sl_cli_command_arg_t *arguments);
void setChannelCommand(sl_cli_command_arg_t *arguments);
void joinListAddCommand(sl_cli_command_arg_t *arguments);
void joinListRequestCommand(sl_cli_command_arg_t *arguments);
void ieeeAddressRequestCommand(sl_cli_command_arg_t *arguments);

#if     (defined(PHY_DUAL) || defined(PHY_SIMULATION_DUAL) || defined(PHY_SIMULATION_GB) || defined(PHY_RAILGB))
void makeUseofNegotiatedPower(sl_cli_command_arg_t *arguments);
void runDutyCycleTest(sl_cli_command_arg_t *arguments);
void ccaThresh(sl_cli_command_arg_t *arguments);
#endif

// Set the allowRejoins variable
// 4.7.3 Trust Center Policy Values, value 0xb6
// This value indicates if the trust center allows rejoins using well known or
// default keys. A setting of FALSE means rejoins are only allowed with trust
// center link keys where the KeyAttributes of the apsDeviceKeyPairSet entry
// indicates VERIFIED_KEY.
void setAllowRejoins(sl_cli_command_arg_t *arguments);

void concentratorActionCommand(sl_cli_command_arg_t *arguments);

void versionCommandZCP(sl_cli_command_arg_t *arguments);

void ebrAddEuiandPower(sl_cli_command_arg_t *arguments);
void ebrFindOrRemovePowerByEui64(sl_cli_command_arg_t *arguments);
void printEbrPowerAndEuiList(sl_cli_command_arg_t *arguments);
void lpdEventCommand(sl_cli_command_arg_t *arguments);
void printLinkPowerByNodeId(sl_cli_command_arg_t *arguments);

void sendNetworkCommand(sl_cli_command_arg_t *arguments);

void setDutyCycleLimitsInStack(sl_cli_command_arg_t *arguments);
void getDutyCycleLimits(sl_cli_command_arg_t *arguments);
void getCurrentDutyCycle(sl_cli_command_arg_t *arguments);
void getDutyCycleState(sl_cli_command_arg_t *arguments);

void setCorePrintingEnable(sl_cli_command_arg_t *arguments);

void addRouteCommand(sl_cli_command_arg_t *arguments);
void deleteRouteCommand(sl_cli_command_arg_t *arguments);
void clearRouteTableCommand(sl_cli_command_arg_t *arguments);
void showSourceRouteTableCommand(sl_cli_command_arg_t *arguments);
void clearSourceTableCommand(sl_cli_command_arg_t *arguments);
void assumeTcConcentratorTypeCommand(sl_cli_command_arg_t *arguments);
void routeRecordPolicyCommand(sl_cli_command_arg_t *arguments);
//------------------------------------------------------------------------------
// Command Line Stuff
void sendLPD(sl_cli_command_arg_t *arguments);
void sendMalformedLPD(sl_cli_command_arg_t *arguments);

//------------------------------------------------------------------------------
// User Defined Stack Handlers
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_scan_complete_handler_pro_compliance(uint8_t channel, sl_status_t status)
#else
void sl_zigbee_scan_complete_handler(uint8_t channel, sl_status_t status)
#endif
{
  scanCompleteHandler(channel, status);
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_stack_status_handler_pro_compliance(sl_status_t status)
#else
void sl_zigbee_stack_status_handler(sl_status_t status)
#endif
{
  stackStatusHandler(status);
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_network_found_handler_pro_compliance(sl_zigbee_zigbee_network_t *networkFound,
                                                    uint8_t lqi,
                                                    int8_t rssi)
#else
void sl_zigbee_network_found_handler(sl_zigbee_zigbee_network_t *networkFound,
                                     uint8_t lqi,
                                     int8_t rssi)
#endif
{
  UNUSED_VAR(lqi);
  UNUSED_VAR(rssi);
  networkFoundHandler(networkFound);
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_message_sent_handler_pro_compliance(sl_status_t status,
                                                   sl_zigbee_outgoing_message_type_t type,
                                                   uint16_t indexOrDestination,
                                                   sl_zigbee_aps_frame_t *apsFrame,
                                                   uint16_t messageTag,
                                                   uint8_t messageLength,
                                                   uint8_t *message)
#else
void sl_zigbee_message_sent_handler(sl_status_t status,
                                    sl_zigbee_outgoing_message_type_t type,
                                    uint16_t indexOrDestination,
                                    sl_zigbee_aps_frame_t *apsFrame,
                                    uint16_t messageTag,
                                    uint8_t messageLength,
                                    uint8_t *message)
#endif
{
  (void)indexOrDestination;
  (void)messageTag;
  (void)messageLength;
  (void)message;
  sl_zigbee_concentrator_note_delivery_failure(type, status);
  testProfileMessageSentHandler(type, apsFrame, status);
  if (!sli_zigbee_af_fragmentation_message_sent(apsFrame, status)
      && !sl_zigbee_is_outgoing_counters_response(apsFrame, status)
      && status != SL_STATUS_OK) {
    printErrorMessage("Message send failed.");
    sl_zigbee_core_debug_print("Status 0x%0x\r\n", status);
  }
}
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sli_zigbee_af_fragmentation_message_sent_handler_pro_compliance(sl_status_t status,
                                                                     sl_zigbee_outgoing_message_type_t type,
                                                                     uint16_t indexOrDestination,
                                                                     sl_zigbee_aps_frame_t *apsFrame,
                                                                     uint16_t messageTag,
                                                                     uint8_t *buffer,
                                                                     uint16_t bufLen)
#else
void sli_zigbee_af_fragmentation_message_sent_handler(sl_status_t status,
                                                      sl_zigbee_outgoing_message_type_t type,
                                                      uint16_t indexOrDestination,
                                                      sl_zigbee_aps_frame_t *apsFrame,
                                                      uint16_t messageTag,
                                                      uint8_t *buffer,
                                                      uint16_t bufLen)
#endif
{
  (void)type;
  (void)indexOrDestination;
  (void)apsFrame;
  (void)buffer;
  (void)bufLen;
  (void)messageTag;

  if (status != SL_STATUS_OK) {
    printErrorMessage("Fragment send failed.");
  }
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_child_join_handler_pro_compliance(
  // The index of the child of interest.
  uint8_t index,
  // True if the child is joining. False the child is leaving.
  bool joining,
  // The node ID of the child.
  sl_802154_short_addr_t childId,
  // The EUI64 of the child.
  sl_802154_long_addr_t childEui64,
  // The node type of the child.
  sl_zigbee_node_type_t childType)
#else
void sl_zigbee_child_join_handler(
  // The index of the child of interest.
  uint8_t index,
  // True if the child is joining. False the child is leaving.
  bool joining,
  // The node ID of the child.
  sl_802154_short_addr_t childId,
  // The EUI64 of the child.
  sl_802154_long_addr_t childEui64,
  // The node type of the child.
  sl_zigbee_node_type_t childType)
#endif
{
  UNUSED_VAR(childId);
  UNUSED_VAR(childEui64);
  UNUSED_VAR(childType);

  sli_zigbee_set_current_network(sl_zigbee_get_callback_network());
  sl_zigbee_child_data_t childData;
  sl_zigbee_get_child_data(index, &childData);
  childJoinHandler(index,
                   joining,
                   childData.id,
                   childData.eui64,
                   childData.type);
  sli_zigbee_restore_current_network();
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
sl_zigbee_join_decision_t sl_zigbee_internal_trust_center_join_handler_pro_compliance(sl_802154_short_addr_t newNodeId,
                                                                                      sl_802154_long_addr_t newNodeEui64,
                                                                                      sl_zigbee_device_update_t status,
                                                                                      sl_802154_short_addr_t parentOfNewNode)
#else
sl_zigbee_join_decision_t sl_zigbee_internal_trust_center_join_handler(sl_802154_short_addr_t newNodeId,
                                                                       sl_802154_long_addr_t newNodeEui64,
                                                                       sl_zigbee_device_update_t status,
                                                                       sl_802154_short_addr_t parentOfNewNode)
#endif
{
  UNUSED_VAR(parentOfNewNode);
  sli_zigbee_set_current_network(sli_zigbee_stack_get_callback_network());
  bool secured = (status == SL_ZIGBEE_STANDARD_SECURITY_SECURED_REJOIN);
  bool rejoin = (status == SL_ZIGBEE_STANDARD_SECURITY_SECURED_REJOIN
                 || status == SL_ZIGBEE_STANDARD_SECURITY_UNSECURED_REJOIN);
  sl_zigbee_join_decision_t handlerDecision = joinDecision;
  char * decisionText[] = { "USE_PRE", "SEND_KEY", "DENY", "NADA" };

  if ((status == SL_ZIGBEE_DEVICE_LEFT)
      || (secured && rejoin)
      // 4.6.3.3.2 - TC rejoins rejected in distributed TC mode
      || (sli_zigbee_get_security_state(SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE)
          && !secured && rejoin)) {
    handlerDecision = SL_ZIGBEE_NO_ACTION;
  }

  if (status == SL_ZIGBEE_DEVICE_LEFT) {
    sl_zigbee_core_debug_print("Device leave: 0x%04X, ",
                               newNodeId);
  } else {
    // Note that the decision here is not final
    // If the device is using the well-known link key, even if the status here
    // is SL_ZIGBEE_USE_PRECONFIGURED_KEY, the device will not be sent the Network
    // Key if doing a rejoin unless the rejoin policy is updated (see
    // setAllowRejoins)
    sl_zigbee_core_debug_print("Join decision %s for device ",
                               decisionText[handlerDecision]);
  }
  printLittleEndianEui64(serialPort, newNodeEui64);

  if ( status != SL_ZIGBEE_DEVICE_LEFT ) {
    sl_zigbee_core_debug_print(" (%ssecure %sjoin)",
                               (secured
                                ? ""
                                : "un"),
                               (rejoin
                                ? "re"
                                : ""));
    (void) sli_legacy_serial_wait_send(serialPort);
  }
  printCarriageReturn();
  sli_zigbee_restore_current_network();

  return handlerDecision;
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
sl_zigbee_zdo_status_t sl_zigbee_internal_remote_set_binding_handler_pro_compliance(sl_zigbee_binding_table_entry_t *entry)
#else
sl_zigbee_zdo_status_t sl_zigbee_internal_remote_set_binding_handler(sl_zigbee_binding_table_entry_t *entry)
#endif
{
  uint8_t i;
  sl_status_t status;
  sli_zigbee_set_current_network(sli_zigbee_stack_get_callback_network());
  for (i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
    sl_zigbee_binding_table_entry_t temp;
    sli_zigbee_stack_get_binding(i, &temp);
    if (temp.type == SL_ZIGBEE_UNUSED_BINDING) {
      status = sli_zigbee_stack_set_binding(i, entry);
      sli_zigbee_restore_current_network();
      return status == SL_STATUS_OK ? SL_ZIGBEE_ZDP_SUCCESS // binding set
             : (status == SL_STATUS_ZIGBEE_BINDING_IS_ACTIVE ? SL_ZIGBEE_ZDP_NOT_AUTHORIZED   // selected index is active
                : SL_ZIGBEE_ZDP_TABLE_FULL);   // report full for any other failure
    }
  }
  sli_zigbee_restore_current_network();
  return SL_ZIGBEE_ZDP_TABLE_FULL;
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
sl_zigbee_zdo_status_t sl_zigbee_internal_remote_delete_binding_handler_pro_compliance(uint8_t index)
#else
sl_zigbee_zdo_status_t sl_zigbee_internal_remote_delete_binding_handler(uint8_t index)
#endif
{
  sl_status_t status = sli_zigbee_stack_delete_binding(index);
  return status == SL_STATUS_OK ? SL_ZIGBEE_ZDP_SUCCESS // binding deleted
         : (status == SL_STATUS_ZIGBEE_BINDING_IS_ACTIVE ? SL_ZIGBEE_ZDP_NOT_AUTHORIZED   // selected index is active
            : SL_ZIGBEE_ZDP_NO_ENTRY);   // report no entry for any other failure
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_incoming_message_handler_pro_compliance(sl_zigbee_incoming_message_type_t type,
                                                       sl_zigbee_aps_frame_t *apsFrame,
                                                       sl_zigbee_rx_packet_info_t *packetInfo,
                                                       uint8_t messageLength,
                                                       uint8_t *message)
#else
void sl_zigbee_incoming_message_handler(sl_zigbee_incoming_message_type_t type,
                                        sl_zigbee_aps_frame_t *apsFrame,
                                        sl_zigbee_rx_packet_info_t *packetInfo,
                                        uint8_t messageLength,
                                        uint8_t *message)
#endif
{
  uint16_t length = messageLength;
  if (type == SL_ZIGBEE_INCOMING_UNICAST
      && sli_zigbee_af_fragmentation_incoming_message(SL_ZIGBEE_INCOMING_UNICAST,
                                                      apsFrame,
                                                      packetInfo->sender_short_id,
                                                      &message,
                                                      &length)) {
    // handled by the fragmentation code
  } else if (sl_zigbee_is_incoming_counters_request(apsFrame, packetInfo->sender_short_id)) {
    // handled by the counters code
  } else if (sl_zigbee_is_incoming_counters_response(apsFrame)) {
    printCountersResponse(messageLength, message);
  } else if (type == SL_ZIGBEE_INCOMING_UNICAST
             || type == SL_ZIGBEE_INCOMING_BROADCAST
             || type == SL_ZIGBEE_INCOMING_BROADCAST_LOOPBACK
             || type == SL_ZIGBEE_INCOMING_MULTICAST
             || type == SL_ZIGBEE_INCOMING_MULTICAST_LOOPBACK) {
    if (apsFrame->sourceEndpoint == 0) {
      if ( !handleZdoClusterMessage(apsFrame, packetInfo, (uint8_t)length, message) ) {
        noteIncoming(type, apsFrame, packetInfo, (uint8_t)length, message);
      }
    } else {
      incomingMessageHandler(type, apsFrame, packetInfo, (uint8_t)length, message);
    }
  }
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_incoming_route_error_handler_pro_compliance(sl_status_t status,
                                                           sl_802154_short_addr_t target)
#else
void sl_zigbee_incoming_route_error_handler(sl_status_t status,
                                            sl_802154_short_addr_t target)
#endif
{
  incomingRouteErrorHandler(status, target);
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_zigbee_key_establishment_handler_pro_compliance(sl_802154_long_addr_t partner, sl_zigbee_key_status_t status)
#else
void sl_zigbee_zigbee_key_establishment_handler(sl_802154_long_addr_t partner, sl_zigbee_key_status_t status)
#endif
{
  keyEstablishmentHandler(partner, status);
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_incoming_many_to_one_route_request_handler_pro_compliance(sl_802154_short_addr_t source,
                                                                         sl_802154_long_addr_t longId,
                                                                         uint8_t cost)
#else
void sl_zigbee_incoming_many_to_one_route_request_handler(sl_802154_short_addr_t source,
                                                          sl_802154_long_addr_t longId,
                                                          uint8_t cost)
#endif
{
  UNUSED_VAR(longId);
  UNUSED_VAR(cost);
  MtorrReceived++;
  sl_zigbee_core_debug_println("MTORR Request: source %u", source);
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_override_incoming_route_record_handler_pro_compliance(sl_zigbee_rx_packet_info_t *packetInfo,
                                                                     uint8_t relayCount,
                                                                     uint8_t *relayList,
                                                                     bool* consumed)
#else
void sl_zigbee_override_incoming_route_record_handler(sl_zigbee_rx_packet_info_t *packetInfo,
                                                      uint8_t relayCount,
                                                      uint8_t *relayList,
                                                      bool* consumed)
#endif
{
  UNUSED_VAR(relayCount);
  UNUSED_VAR(relayList);
  UNUSED_VAR(consumed);
  sl_zigbee_core_debug_println("Incoming route record: source 0x%04X",
                               packetInfo->sender_short_id);
}

//------------------------------------------------------------------------------
// Application functions
void setNetworkSecurity(sl_cli_command_arg_t *arguments)
{
  use_network_security = sl_cli_get_argument_uint8(arguments, 0) ? true : false;
}
void applicationKeyTimeoutCommand(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_request_key_timeout = sl_cli_get_argument_uint8(arguments, 0);
}

void changeAddressCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t oldAddress = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_short_addr_t newAddress = sl_cli_get_argument_uint16(arguments, 1);
  sl_zigbee_child_data_t childData;

  uint8_t childIndex = sl_zigbee_child_index(oldAddress);
  if ( SL_MAC_CHILD_INVALID_INDEX == childIndex
       || SL_STATUS_OK != sl_zigbee_get_child_data(childIndex,
                                                   &childData) ) {
    printErrorMessage("No such child");
    (void) sli_legacy_serial_wait_send(serialPort);
    return;
  }

  sl_zigbee_test_send_network_rejoin_command(ZIGBEE_REJOIN_RESPONSE,
                                             childData.eui64,
                                             oldAddress,
                                             newAddress,
                                             use_network_security && (sl_zigbee_security_level() != 0),   // use nwk security
                                             ASSOCIATION_SUCCESSFUL,
                                             true);
}

void setAddressCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t newAddress = sl_cli_get_argument_uint16(arguments, 0);
  bool sendDeviceAnnouncement = (bool)sl_cli_get_argument_uint32(arguments, 1);
  sl_zigbee_set_node_id(newAddress);
  if ( sendDeviceAnnouncement ) {
    sl_zigbee_test_send_our_end_device_announcement();
  }
}

sli_zigbee_packet_header_t sli_zigbee_make_zigbee_command_header_no_security(sl_802154_short_addr_t destination,
                                                                             uint8_t radius,
                                                                             uint8_t *commandFrame,
                                                                             uint8_t length,
                                                                             bool tryToInsertLongDest,
                                                                             sl_802154_long_addr_t destinationEui)
{
  sli_zigbee_packet_header_t header;
  uint16_t nwkFrameControl
    = (ZIGBEE_FRAME_CONTROL_FRAME_TYPE_COMMAND
       | ZIGBEE_FRAME_CONTROL_PROTOCOL_VERSION
       | ZIGBEE_FRAME_CONTROL_SOURCE_IEEE_ADDRESS);

  if (tryToInsertLongDest) {
    nwkFrameControl |= ZIGBEE_FRAME_CONTROL_DESTINATION_IEEE_ADDRESS;
  }

  header = sli_zigbee_make_zigbee_header(SL_ZIGBEE_NULL_MESSAGE_BUFFER,
                                         nwkFrameControl,
                                         destination,
                                         destinationEui,
                                         radius);

  if (header == SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    return SL_ZIGBEE_NULL_MESSAGE_BUFFER;
  }

  if (sl_legacy_buffer_manager_append_to_linked_buffers(header, commandFrame, length)
      != SL_STATUS_OK) {
    sl_legacy_buffer_manager_release_message_buffer(header);
    return SL_ZIGBEE_NULL_MESSAGE_BUFFER;
  }
  return header;
}

sli_zigbee_packet_header_t sli_zigbee_make_network_status_message_no_network_encryption(uint8_t errorCode,
                                                                                        sl_802154_short_addr_t destination,
                                                                                        sl_802154_short_addr_t target)
{
  uint8_t frame[ZIGBEE_ROUTE_ERROR_FRAME_SIZE];

  frame[0] = ZIGBEE_ROUTE_ERROR;
  frame[ZIGBEE_ROUTE_ERROR_CODE_INDEX] = errorCode;
  frame[ZIGBEE_ROUTE_ERROR_DESTINATION_INDEX] = LOW_BYTE(target);
  frame[ZIGBEE_ROUTE_ERROR_DESTINATION_INDEX + 1] = HIGH_BYTE(target);
  return sli_zigbee_make_zigbee_command_header_no_security(destination,
                                                           0,
                                                           frame,
                                                           ZIGBEE_ROUTE_ERROR_FRAME_SIZE,
                                                           true,
                                                           NULL);
}

void sendNetworkStatusAddressConflict(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t dest = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_short_addr_t conflictAddr = sl_cli_get_argument_uint16(arguments, 1);

  sl_zigbee_test_send_route_error_payload_no_network_encryption(dest,
                                                                conflictAddr,
                                                                SL_ZIGBEE_ROUTE_ERROR_ADDRESS_CONFLICT,
                                                                NULL,
                                                                0);
}

#if defined SL_ZIGBEE_TEST
void hashKey(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t eui64;
  sl_zigbee_key_data_t key;
  sl_zigbee_key_data_t newKey;

  sl_zigbee_copy_eui64_arg(arguments, 0, eui64, false);
  sl_zigbee_copy_hex_arg(arguments,
                         1,
                         sl_zigbee_key_contents(&key),
                         SL_ZIGBEE_ENCRYPTION_KEY_SIZE,
                         true);

  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_init_context(&context);
  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_INTERNAL;
  sl_zigbee_sec_man_import_key(&context, (sl_zigbee_sec_man_key_t *)&key);
  sl_zigbee_sec_man_load_key_context(&context);

  sl_zigbee_sec_man_hmac_aes_mmo(eui64,
                                 EUI64_SIZE,
                                 sl_zigbee_key_contents(&newKey));
  printEncryptionKey2(serialPort, sl_zigbee_key_contents(&newKey));
}
#endif // defined SL_ZIGBEE_TEST

#if 0
static void derivedKeysCommand(void)
{
  uint8_t i;
  for (i = 0; i < 2; i++) {
    sl_zigbee_core_debug_println((i == 0
                                  ? "Transport Key Data"
                                  : "Load Key Data"));
    printKeyTable(i == 0
                  ? DERIVE_TRANSPORT_KEY_TAG
                  : DERIVE_LOAD_KEY_TAG,
                  false);
  }
}
#endif

void sendKeyCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t partnerEui64;
  sl_802154_short_addr_t partnerNodeId;
  sl_zigbee_key_data_t newKey;
  bool success;

  sl_zigbee_copy_eui64_arg(arguments, 0, partnerEui64, false);
  if (!sl_zigbee_copy_hex_arg(arguments, 1, sl_zigbee_key_contents(&newKey), SL_ZIGBEE_ENCRYPTION_KEY_SIZE, true)) {
    sl_zigbee_generate_random_key(&newKey);
  }
  sl_status_t status = sl_zigbee_lookup_node_id_by_eui64(partnerEui64, &partnerNodeId);
  success = isSecurityStateValid()
            && status == SL_STATUS_OK
            && sl_zigbee_test_send_link_key(partnerNodeId,
                                            partnerEui64,
                                            (uint8_t) KEY_TRANSPORT_TRUST_CENTER_LINK_KEY,
                                            &newKey,
                                            true);
  printOperationStatus(!success,  // convert to sl_status_t
                       "Send Key");
}

void requestPolicyCommand(sl_cli_command_arg_t *arguments)
{
  // In 6.3.1 (a WWAH release) we changed the trust center and application link
  // key policies to be separate and clearer. The change went from
  //   SL_ZIGBEE_DENY_KEY_REQUESTS  = 0x00,
  //   SL_ZIGBEE_ALLOW_KEY_REQUESTS = 0x01,
  //   SL_ZIGBEE_GENERATE_NEW_TC_LINK_KEY = 0x02,
  // for both policies to
  //   SL_ZIGBEE_DENY_TC_LINK_KEY_REQUESTS  = 0x00,
  //   SL_ZIGBEE_ALLOW_TC_LINK_KEY_REQUEST_AND_SEND_CURRENT_KEY = 0x01,
  //   SL_ZIGBEE_ALLOW_TC_LINK_KEY_REQUEST_AND_GENERATE_NEW_KEY = 0x02
  // for the emberTrustCenterLinkKeyRequestPolicy field and
  //   SL_ZIGBEE_DENY_APP_LINK_KEY_REQUESTS  = 0x00,
  //   SL_ZIGBEE_ALLOW_APP_LINK_KEY_REQUEST = 0x01
  // for the emberAppLinkKeyRequestPolicy field. In doing so, we had to change
  // this CLI command to map the old values to the new ones, otherwise a test
  // house with a new ZCP java binary and a mix of new and old zigbee_pro_compliance
  // images would not work. The old ones would want key policy 2
  // (SL_ZIGBEE_GENERATE_NEW_TC_LINK_KEY) while the new zigbee_pro_compliances would want
  // policy 1 (SL_ZIGBEE_ALLOW_TC_LINK_KEY_REQUEST_AND_SEND_CURRENT_KEY) (the issue
  // being that the two policies ended up doing the same thing in their
  // respective versions of the stack, hence the rework of the policies. The
  // original implementation was flawed in that the distinction between
  // SL_ZIGBEE_ALLOW_KEY_REQUESTS and SL_ZIGBEE_GENERATE_NEW_TC_LINK_KEY was unclear).
  //
  // To mitigate this change and provide for a smooth certification testing
  // process, we simply have the new zigbee_pro_compliance images treat value 1 as
  // 2 and vice-versa. Ooof.
  uint8_t tcLinkKeyPolicy = sl_cli_get_argument_uint8(arguments, 0);
  switch (tcLinkKeyPolicy) {
    case SL_ZIGBEE_DENY_TC_LINK_KEY_REQUESTS:
      break;
    case SL_ZIGBEE_ALLOW_TC_LINK_KEY_REQUEST_AND_SEND_CURRENT_KEY:
      tcLinkKeyPolicy = SL_ZIGBEE_ALLOW_TC_LINK_KEY_REQUEST_AND_GENERATE_NEW_KEY;
      break;
    case SL_ZIGBEE_ALLOW_TC_LINK_KEY_REQUEST_AND_GENERATE_NEW_KEY:
      tcLinkKeyPolicy = SL_ZIGBEE_ALLOW_TC_LINK_KEY_REQUEST_AND_SEND_CURRENT_KEY;
      break;
    default:
      break;
  }
  sl_zigbee_set_trust_center_link_key_request_policy(tcLinkKeyPolicy);

  uint8_t appLinkKeyPolicy = sl_cli_get_argument_uint8(arguments, 1);
  if (appLinkKeyPolicy > SL_ZIGBEE_ALLOW_APP_LINK_KEY_REQUEST) {
    appLinkKeyPolicy = SL_ZIGBEE_ALLOW_APP_LINK_KEY_REQUEST;
  }
  sl_zigbee_set_app_link_key_request_policy(appLinkKeyPolicy);
}

#if defined(SL_ZIGBEE_TEST)
void sleepOkCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_zigbee_core_debug_print("Nap:       ");
  printYesOrNo(sl_zigbee_ok_to_nap());
  sl_zigbee_core_debug_print("Hibernate: ");
  printYesOrNo(sl_zigbee_ok_to_hibernate());
  sl_zigbee_core_debug_println("Tasks:     0x%02X",
                               sl_zigbee_current_stack_tasks());
  sl_zigbee_core_debug_print("Radio Rx On:");
  printYesOrNo(sl_mac_lower_mac_radio_is_on(0));
}
#endif // SL_ZIGBEE_TEST

void nwkInitCommand(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_network_init_struct_t nwkInitStruct;
  sl_status_t status;
  nwkInitStruct.bitmask = sl_cli_get_argument_uint16(arguments, 0);
  status = sl_zigbee_network_init(&nwkInitStruct);
  printOperationStatus(status, "Re-initializing network with bitmask");
}

void routeErrorCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t errorCode = sl_cli_get_argument_uint8(arguments, 0);
  sl_802154_short_addr_t destination = sl_cli_get_argument_uint16(arguments, 1);
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 2);
  uint8_t *optional_payload = NULL;
  size_t payload_length = 0;
  if (sl_cli_get_argument_count(arguments) >= 4) {
    optional_payload = sl_cli_get_argument_hex(arguments, 3, &payload_length);
  }

  if (payload_length > 128) {
    sl_zigbee_core_debug_println("error: payload_length=%d is longer than 128 bytes allowed", payload_length);
    return;
  }

  sl_zigbee_test_send_route_error_payload(destination, target, errorCode, optional_payload, (uint8_t)payload_length);
}

void changePanCommand(sl_cli_command_arg_t *arguments)
{
  if (sl_zigbee_send_pan_id_update(sl_cli_get_argument_uint16(arguments, 0))) {
    // Success
    return;
  }

  printErrorMessage("Not NWK manager.");
  (void) sli_legacy_serial_wait_send(serialPort);
}

void setPendingNetworkUpdatePanId(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_set_pending_network_update_pan_id(sl_cli_get_argument_uint16(arguments, 0));
}

void setPendingNetworkUpdateChannel(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_set_pending_network_update_channel(sl_cli_get_argument_uint8(arguments, 0));
}

void panConflictCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_zigbee_network_parameters_t params;
  sl_status_t status = sl_zigbee_get_network_parameters(NULL, &params);
  if (status == SL_STATUS_OK
      && sl_zigbee_test_send_report_or_update(NETWORK_REPORT_COMMAND,
                                              0,    // updateId should be ignored
                                              params.panId)) {
    sl_zigbee_core_debug_println("Sent PAN ID conflict for 0x%02x.", params.panId);
    return;
  }
  sl_zigbee_core_debug_println("PAN conflict send fail.");
}

void setNetworkTokensCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t temp;
  sl_zigbee_copy_eui64_arg(arguments, 6, temp, false);
  sl_zigbee_test_set_network_tokens(sl_cli_get_argument_uint8(arguments, 0),
                                    sl_cli_get_argument_uint8(arguments, 1),
                                    sl_cli_get_argument_uint8(arguments, 2),
                                    sl_cli_get_argument_int8(arguments, 3),
                                    sl_cli_get_argument_uint16(arguments, 4),
                                    sl_cli_get_argument_uint16(arguments, 5),
                                    temp);
}

// This should typically call after "set_security" if test case
// require to persist security settings.
void writeSecurityToken(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sli_zigbee_write_security_token();
}
//Write Parent Token
void sli_zigbee_update_parent_token(void);
void writeParentToken(sl_cli_command_arg_t *arguments)
{
  tokTypeStackParentInfo tok;
  tok.parentNodeId = sl_cli_get_argument_uint16(arguments, 0);
  sli_zigbee_parent_id = sl_cli_get_argument_uint16(arguments, 0);
  sl_zigbee_copy_eui64_arg(arguments, 1, sli_zigbee_parent_eui64, false);
  sl_zigbee_copy_eui64_arg(arguments, 1, tok.parentEui, false);
  sli_zigbee_set_stack_token(COMMON_TOKEN_STACK_PARENT_INFO,
                             &tok,
                             sizeof(tok));
  sli_zigbee_update_parent_token();
}

// Parameters: Channel, Power, Extended PAN ID, PanID, shortId (optional)
void joinQuietlyCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t id = (sl_cli_get_argument_count(arguments) > 5)
                              ? sl_cli_get_argument_uint16(arguments, 5)
                              : sl_zigbee_get_pseudo_random_number();
  sl_802154_long_addr_t temp;
  sl_zigbee_copy_eui64_arg(arguments, 2, temp, false);
  sl_zigbee_test_set_network_tokens(2,                                // stack profile
                                    sl_cli_get_argument_uint8(arguments, 4),  // node type
                                    sl_cli_get_argument_uint8(arguments, 0),  // channel
                                    sl_cli_get_argument_int8(arguments, 1),  // power
                                    id,                               // node Id
                                    sl_cli_get_argument_uint16(arguments, 3),  // pan Id
                                    temp);                            // extended pan Id

  // sl_zigbee_network_init() will restore the security state bitmask back to what's
  // in the tokens, which has a bad consequence of removing the
  // SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE bit if it's on.
  // We atone for this by reading off its value and restoring it later if needed
  bool distributedTcMode =
    sli_zigbee_get_security_state(SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE);

  sl_zigbee_network_init_struct_t nwkInitStruct;

  nwkInitStruct.bitmask = SL_ZIGBEE_NETWORK_INIT_PARENT_INFO_IN_TOKEN;
  sl_zigbee_network_init(&nwkInitStruct);

  if (distributedTcMode) {
    sli_zigbee_set_security_state(SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE);
  }
}

void set_tx_power(sl_cli_command_arg_t *arguments)
{
  int8_t power = sl_cli_get_argument_int8(arguments, 0);
  sl_status_t status = sl_mac_test_set_tx_power(power);
  sl_zigbee_core_debug_println("Set radio power status = 0x%0x", status);
}

void set_relative_tx_power(sl_cli_command_arg_t *arguments)
{
  int8_t power_delta = sl_cli_get_argument_int8(arguments, 0);
  int8_t power = sl_zigbee_get_radio_power();
  power += power_delta;
  sl_status_t status = sl_mac_test_set_tx_power(power);
  sl_zigbee_core_debug_println("Set radio power status = 0x%0x", status);
}

void setChannelCommand(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  uint8_t channel = sl_cli_get_argument_uint8(arguments, 0);
  int8_t power = 0;

#ifdef MAC_DUAL_PRESENT
  // Assume subghz channel if page is not zero.

  if (sli_802154mac_pg_chan_pg(channel) != SL_ZIGBEE_NO_CHANNEL_PAGE_IN_USE ) {
    status = sli_802154phy_multi_phy_set_radio_channel(PHY_INDEX_PRO2PLUS, channel);
    sli_zigbee_sub_ghz_radio_channel = (status ==  SL_STATUS_OK) ? channel : sli_zigbee_sub_ghz_radio_channel;
    if (sl_cli_get_argument_count(arguments)  > 1) {  // set power
      power = sl_cli_get_argument_int8(arguments, 1);
      status = (status == SL_STATUS_OK)  ? sli_802154phy_multi_phy_set_radio_power(PHY_INDEX_PRO2PLUS, power)
               : status;
      sli_zigbee_sub_ghz_radio_power = (status ==  SL_STATUS_OK) ? power : sli_zigbee_sub_ghz_radio_power;
    }
  } else {  // 2.4Ghz channel
#endif // MAC_DUAL_PRESENT
  // upper mac API?
  status = sl_mac_lower_mac_set_radio_channel(PHY_INDEX_NATIVE, channel);
  if (status == SL_STATUS_OK) {
    (void) sl_mac_test_set_nwk_radio_params_channel(channel);
  }
  if (sl_cli_get_argument_count(arguments) > 1) {
    power =  sl_cli_get_argument_int8(arguments, 1);
    if (status == SL_STATUS_OK) {
      status = sl_mac_lower_mac_set_radio_power(PHY_INDEX_NATIVE, power);
      sl_mac_test_set_tx_power(power);
    }
  }
#ifdef MAC_DUAL_PRESENT
}
#ifdef MAC_TEST_COMMANDS_SUPPORT
// Turn off the other radio to avoid spurious rx/tx packets,
// required for mac certification tests.
if ( sli_zigbee_enable_mac_certification_test_mode ) {
  sl_mac_lower_mac_set_radio_idle_mode(sli_802154mac_pg_chan_pg(channel) ? PHY_INDEX_NATIVE : PHY_INDEX_PRO2PLUS,
                                       SL_ZIGBEE_RADIO_POWER_MODE_OFF);
  // Need this when device acts as a coordinator to keep active interface index.
  sli_zigbee_association_mac_index = sli_802154mac_pg_chan_pg(channel) ? PHY_INDEX_PRO2PLUS : PHY_INDEX_NATIVE;
}
#endif // MAC_TEST_COMMANDS_SUPPORT
#endif // MAC_DUAL_PRESENT

  sl_zigbee_core_debug_print(
    "status 0x%02X set channel %d",
    status, channel);
  if (sl_cli_get_argument_count(arguments) > 1) {
    sl_zigbee_core_debug_println(" power %d", power);
  } else {
    sl_zigbee_core_debug_println("");
  }
}

void resetFrameCounterCommand(sl_cli_command_arg_t *arguments)
{
  const char * fcMessage = "Reset %s frame counter.";
  uint8_t mask =  sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_test_reset_frame_counter(mask);
  if (mask & RESET_NWK_FC_MASK) {
    sl_zigbee_core_debug_println(fcMessage, "NWK");
  }
  if (mask & RESET_APS_FC_MASK) {
    sl_zigbee_core_debug_println(fcMessage, "APS");
  }
}

void setFrameCounterCommand(sl_cli_command_arg_t *arguments)
{
  uint32_t newFrameCounterValue = sl_cli_get_argument_uint32(arguments, 0);
  sli_zigbee_next_nwk_frame_counter = newFrameCounterValue;
}

void checkOutgoingFrameCounter(sl_cli_command_arg_t *arguments)
{
  uint32_t minValue = sl_cli_get_argument_uint32(arguments, 0);
  uint32_t maxValue = sl_cli_get_argument_uint32(arguments, 1);
  uint32_t actualValue = sl_zigbee_get_security_frame_counter();
  if (actualValue <= maxValue && actualValue >= minValue) {
    sl_zigbee_core_debug_println("Expected outgoing frame counter (0x%04lX)", (unsigned long)actualValue);
  } else {
    sl_zigbee_core_debug_println("Unexpected frame Counter 0x%04lx", (unsigned long)actualValue);
  }
}

void tokenWritingCommand(sl_cli_command_arg_t *arguments)
{
  bool writeTokens = (bool)sl_cli_get_argument_uint32(arguments, 0);
  printCommandStatus((writeTokens
                      ? sl_zigbee_start_writing_stack_tokens()
                      : sl_zigbee_stop_writing_stack_tokens()),
                     NULL,
                     "Token writing command failed.");
}

// Creates a message buffer, copies the string of hex values passed through the
// the command 'send_raw_packet' into the buffer and sends it to the MAC
// layer using the sl_zigbee_send_raw_message() function.
void sendRawPacketCommand(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  uint8_t i;
  size_t byteCount;
  uint8_t *bytes = sl_cli_get_argument_hex(arguments, 0, &byteCount);

  if (byteCount > 128) {
    sl_zigbee_core_debug_println("raw packet too long");
    return;
  }

  uint8_t rawPacketBuffer[128] = { 0 };
  for (i = 0; i < byteCount; i++) {
    rawPacketBuffer[i] = bytes[i];
  }

  status = sl_zigbee_send_raw_message(rawPacketBuffer, (uint8_t)byteCount, SL_802154_TRANSMIT_PRIORITY_NORMAL, true);

  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_println("raw packet sent");
  } else {
    sl_zigbee_core_debug_println("send failed");
  }

  (void) sli_legacy_serial_wait_send(serialPort);
}

void sendRawPacketCommandWithTag(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  size_t byteCount;
  const uint8_t *bytes = sl_cli_get_argument_hex(arguments, 0, &byteCount);
  uint8_t messageTag = sl_cli_get_argument_uint8(arguments, 1);

  if (byteCount > 128) {
    sl_zigbee_core_debug_println("raw packet too long");
    return;
  }

  uint8_t rawPacketBuffer[128] = { 0 };
  for (size_t i = 0; i < byteCount; i++) {
    rawPacketBuffer[i] = bytes[i];
  }

  status = sl_zigbee_send_raw_message_with_tag(rawPacketBuffer, (uint8_t)byteCount, SL_802154_TRANSMIT_PRIORITY_NORMAL, true, messageTag);

  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_println("raw packet sent");
  } else {
    sl_zigbee_core_debug_println("send failed");
  }

  (void) sli_legacy_serial_wait_send(serialPort);
}

void printExtPanIdCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  uint8_t extPanId[EXTENDED_PAN_ID_SIZE];
  sl_zigbee_get_extended_pan_id(extPanId);

  sl_zigbee_core_debug_println("EPID=0x%0x%0x%0x%0x%0x%0x%0x%0x",
                               extPanId[0], extPanId[1], extPanId[2], extPanId[3],
                               extPanId[4], extPanId[5], extPanId[6], extPanId[7]);

  (void) sli_legacy_serial_wait_send(serialPort);
}

void printStackStatusCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  if (sl_zigbee_stack_is_up()) {
    sl_zigbee_core_debug_println("Stack up");
  } else {
    sl_zigbee_core_debug_println("Stack down");
  }
}

void resetNodeRebootFlag(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  nodeRebootFlag = false;

  sl_zigbee_core_debug_println("Reboot flag reset");
  (void) sli_legacy_serial_wait_send(serialPort);
}

void printNodeRebootFlag(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  if (nodeRebootFlag) {
    sl_zigbee_core_debug_println("Node has rebooted");
  } else {
    sl_zigbee_core_debug_println("Node has not rebooted");
  }

  (void) sli_legacy_serial_wait_send(serialPort);
}

void sendLeaveRequestCommand(sl_cli_command_arg_t *arguments)
{
  uint16_t destId = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_long_addr_t destEui;
  sl_status_t status;

  sl_zigbee_copy_eui64_arg(arguments, 1, destEui, false);

  status = sl_zigbee_test_send_leave_request_command(destId, destEui);

  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_println("NWK Leave sent");
  } else {
    sl_zigbee_core_debug_println("NWK Leave sending failed");
  }
}

void sendUpdateDeviceCommand(sl_cli_command_arg_t *arguments)
{
  bool status = false;
  uint16_t newDeviceShortId = sl_cli_get_argument_uint16(arguments, 0);
  bool apsEncryption = (sl_cli_get_argument_uint32(arguments, 2) > 0);
  uint8_t deviceStatus = sl_cli_get_argument_uint8(arguments, 3);
  sl_802154_long_addr_t newDeviceLongId;

  sl_zigbee_copy_eui64_arg(arguments, 1, newDeviceLongId, false);

  status = sl_zigbee_test_send_device_update(newDeviceShortId,
                                             newDeviceLongId,
                                             apsEncryption,
                                             deviceStatus);

  if (status) {
    sl_zigbee_core_debug_println("Update device sent");
  } else {
    sl_zigbee_core_debug_println("Update device failed");
  }
}

void addChildCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t shortId;
  sl_802154_long_addr_t longId;
  uint8_t nodeType;
  sl_status_t status;
  shortId = sl_cli_get_argument_uint16(arguments, 0);
  sl_zigbee_copy_eui64_arg(arguments, 1, longId, false);
  nodeType = sl_cli_get_argument_uint8(arguments, 2);

  status = sl_zigbee_add_child(shortId, longId, nodeType);

  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_println("Child added");
  } else {
    sl_zigbee_core_debug_println("Error 0x%0x", status);
  }
}

void pollOnceCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_status_t status;

#ifdef MAC_TEST_COMMANDS_SUPPORT
  sli_zigbee_use_parent_long_id = false;
#endif // MAC_TEST_COMMANDS_SUPPORT

  status = sl_zigbee_poll_for_data();

  if (status != SL_STATUS_OK) {
    sl_zigbee_core_debug_println("Poll failed 0x%02x", status);
  }
}

void pollLongOnceCommand(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;

#ifdef MAC_TEST_COMMANDS_SUPPORT
  sli_zigbee_use_parent_long_id = true;
#endif // MAC_TEST_COMMANDS_SUPPORT

  // You can change parent long id if require
  if (sl_cli_get_argument_count(arguments) > 0) {
    sl_zigbee_copy_eui64_arg(arguments, 0, sli_zigbee_parent_eui64, false);
  }

  status = sl_zigbee_poll_for_data();

  if (status != SL_STATUS_OK) {
    sl_zigbee_core_debug_println("Long poll failed 0x%02x", status);
  }
}

void pollAllNetworksOnceCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  UNUSED uint8_t savedNwkIndex = sl_zigbee_get_current_network();

  uint8_t i;
  for (i = 0; i < sli_zigbee_supported_networks; i++) {
    sl_status_t status;

    (void) sl_zigbee_set_current_network(i);

    if (!sli_zigbee_node_type_is_end_device()) {
      continue;
    }

    status = sl_zigbee_poll_for_data();

    if (status != SL_STATUS_OK) {
      sl_zigbee_core_debug_println("Poll failed 0x%02x", status);
    }
  }

  (void) sl_zigbee_set_current_network(savedNwkIndex);
}

void getLastLeaveReasonCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_802154_short_addr_t nodeIdThatSentLeave;
  uint8_t reason = sl_zigbee_get_last_leave_reason(&nodeIdThatSentLeave);
  sl_zigbee_core_debug_println("Last Reason %u", reason);
}

void optionBindingTableSetCommandZCP(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_binding_table_entry_t entry;
  sl_status_t status = SL_STATUS_FAIL;
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 2);
  entry.type = SL_ZIGBEE_UNICAST_BINDING;
  entry.clusterId = sl_cli_get_argument_uint16(arguments, 1);
  entry.local = endpoint;
  entry.remote = sl_cli_get_argument_uint8(arguments, 3);
  sl_zigbee_copy_eui64_arg(arguments, 4, entry.identifier, false);
  status = sl_zigbee_set_binding(index, &entry);
  sl_zigbee_core_debug_println("set bind %d: 0x%02x", index, (uint32_t)status);
}

void getBindingShortId(sl_cli_command_arg_t *arguments)
{
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  uint16_t shortId = sl_zigbee_get_binding_remote_node_id(index);
  sl_zigbee_core_debug_println("ShortId %u", shortId);
}
#if (defined SL_ZIGBEE_TEST || defined SL_ZIGBEE_GOLDEN_UNIT)

void sendRemoveDeviceCommand(sl_cli_command_arg_t *arguments)
{
  uint16_t destId = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_long_addr_t destEui;
  sl_802154_long_addr_t deviceToRemoveEui;
  sl_status_t status = SL_STATUS_OK;

  sl_zigbee_copy_eui64_arg(arguments, 1, destEui, false);
  sl_zigbee_copy_eui64_arg(arguments, 2, deviceToRemoveEui, false);
  bool sendNonEncrypted = false;

  if (sl_cli_get_argument_uint32(arguments, 3) == 0) {
    sendNonEncrypted = true;
  }

  status = sl_zigbee_test_send_remove_device_command(destId,
                                                     destEui,
                                                     deviceToRemoveEui,
                                                     sendNonEncrypted);

  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_println("Remove device sent");
  } else {
    sl_zigbee_core_debug_println("Failed, status=0x%0x", status);
  }
}

void acceptOnlyNonEncryptedUpdateDeviceCommand(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_accept_only_non_encrypted_update_device_messages =
    (sl_cli_get_argument_uint32(arguments, 0) > 0);
}

void acceptOnlyEncryptedUpdateDeviceCommand(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_accept_only_encrypted_update_device_messages =
    (sl_cli_get_argument_uint32(arguments, 0) > 0);
}

void sendOnlyNonEncryptedUpdateDeviceCommand(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_send_only_non_encrypted_update_device =
    (sl_cli_get_argument_uint32(arguments, 0) > 0);
}

#endif //(defined SL_ZIGBEE_TEST || defined SL_ZIGBEE_GOLDEN_UNIT)

// The sl_zigbee_erase_key_table_entry(x) is a #define in the onboard
// version, and a real function in the EZSP version.  This makes compiling
// the same object code for the simulator impossible.
void eraseKeyTableEntryZCP(uint8_t i)
{
  sl_zigbee_erase_key_table_entry(i);
}

uint32_t getApsFrameCounter(void)
{
  return sl_zigbee_get_aps_frame_counter();
}

void packetValidateCommand(sl_cli_command_arg_t *arguments)
{
  bool enabled = (bool)sl_cli_get_argument_uint32(arguments, 0);
  sl_status_t status =
    sl_zigbee_set_packet_validate_library_state(enabled
                                                ? SL_ZIGBEE_PACKET_VALIDATE_LIBRARY_ENABLED
                                                : SL_ZIGBEE_PACKET_VALIDATE_LIBRARY_DISABLED);
  sl_zigbee_core_debug_println("Packet validate set %s: 0x%02X",
                               (enabled
                                ? "Enabled"
                                : "Disabled"),
                               status);
}

void parentInfoCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  tokTypeStackParentInfo tok;

  sl_zigbee_core_debug_print("RAM - ID: 0x%02X, EUI64: ", sli_zigbee_parent_id);
  printBigEndianEui64(serialPort, sli_zigbee_parent_eui64);

  sli_zigbee_get_stack_token(&tok, COMMON_TOKEN_STACK_PARENT_INFO, sizeof(tok));
  sl_zigbee_core_debug_print(" | Token - ID: 0x%02X, EUI64: ", tok.parentNodeId);
  printBigEndianEui64(serialPort, tok.parentEui);
  sl_zigbee_core_debug_println("");
  sl_zigbee_core_debug_println("Parent additional info: 0x%0X", sli_zigbee_get_parent_nwk_information());
}

void resetCryptoTimingCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  #if defined(CRYPTO_TIMING_INFO)
  sli_zigbee_crypto_timing_reset();
  #endif
}

void enableCryptoTimingCommand(sl_cli_command_arg_t *arguments)
{
  #if defined(CRYPTO_TIMING_INFO)
  sli_zigbee_crypto_timing_record_enable((bool)sl_cli_get_argument_uint32(arguments, 0));
  #else
  (void)arguments;
  #endif
}

static void printCryptoTiming(bool outgoingPackets)
{
  sl_zigbee_core_debug_println("%s Packets",
                               (outgoingPackets
                                ? "Transmitted"
                                : "Received"));
  sl_zigbee_core_debug_println("index  %s %s %scrypt %s  fc",
                               "mic",
                               "size",
                               (outgoingPackets ? "en" : "de"),
                               "size");
  #if defined(CRYPTO_TIMING_INFO)
  const sli_zigbee_crypto_timing_info_t* data;
  uint8_t index = 0;
  while (NULL != (data = sli_zigbee_crypto_timing_get_data(index,
                                                           outgoingPackets))) {
    sl_zigbee_core_debug_println("%d:     %lu   %d   %lu   %d   0x%04lX",
                                 index,
                                 data->micTimingMicroSeconds,
                                 data->micPacketSize,
                                 data->encryptDecryptTimingMicroSeconds,
                                 data->encryptDecryptPacketSize,
                                 data->frameCounter);
    index++;
  }

  #endif
}

void printCryptoTimingCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  uint8_t *commandName = (uint8_t *) sl_cli_get_command_string(arguments, position);
  if (commandName[6] == 't') {
    printCryptoTiming(true); // tx packets?
  } else {
    printCryptoTiming(false);
  }
}

void sendNetworkTimeoutRequest(sl_cli_command_arg_t *arguments)
{
  uint8_t requestedTimeoutValue = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_test_send_network_timeout_request(requestedTimeoutValue);
}

void modifyNetworkTimeoutInternally(sl_cli_command_arg_t *arguments)
{
  if (!sli_zigbee_node_type_is_end_device()) {
    uint16_t shortId = sl_cli_get_argument_uint16(arguments, 0);
    uint8_t modifiedTimeoutValue = sl_cli_get_argument_uint8(arguments, 1);
    uint8_t endDeviceConfiguration = sl_cli_get_argument_uint8(arguments, 2);

    sli_zigbee_set_end_device_timeout(shortId, modifiedTimeoutValue, endDeviceConfiguration);
  } else {
    sl_zigbee_core_debug_println("Invalid call");
  }
}

static uint8_t mapTimeoutToIndex(uint32_t actualValue)
{
  if (actualValue <= 10) {
    return SL_ZIGBEE_POLL_TIMEOUT_10_SECONDS;
  } else {
    for (uint8_t i = 1; i < SL_ZIGBEE_POLL_TIMEOUT_16384_MINUTES; i++) {
      if (actualValue <= (uint32_t) (1 << i) * 60) {
        return i;
      }
    }
  }
  return SL_ZIGBEE_POLL_TIMEOUT_16384_MINUTES;
}

void CommonEndDeviceTimeout(uint8_t endDeviceTimeout, uint8_t endDeviceTimeoutShift, uint8_t endDeviceConfiguration)
{
  sli_zigbee_set_end_device_configuration(endDeviceConfiguration);
  sl_zigbee_set_end_device_poll_timeout(mapTimeoutToIndex(endDeviceTimeout << endDeviceTimeoutShift));
  sl_zigbee_core_debug_println("Set End Device Timeout");

  // We pretend we had a successful poll in order to allow a dynamic change
  // to its local timeout to create a mismatch with the parent.  This is useful
  // in negative testing.
  sli_zigbee_note_successful_poll();
}

void setSilentEndDeviceTiemout(sl_cli_command_arg_t *arguments)
{
  uint8_t endDeviceTimeout = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t endDeviceTimeoutShift = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t endDeviceConfiguration = sl_cli_get_argument_uint8(arguments, 2);
  CommonEndDeviceTimeout(endDeviceTimeout, endDeviceTimeoutShift, endDeviceConfiguration);
}

void setEndDeviceTimeout(sl_cli_command_arg_t *arguments)
{
  uint8_t endDeviceTimeout = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t endDeviceTimeoutShift = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t endDeviceConfiguration = sl_cli_get_argument_uint8(arguments, 2);
  CommonEndDeviceTimeout(endDeviceTimeout, endDeviceTimeoutShift, endDeviceConfiguration);
  //This will send EndDeviceTimeout request if sli_zigbee_enable_r21_stack_behavior enable.
  sl_zigbee_test_send_timeout_request();
}

void setKeepAliveSupport(sl_cli_command_arg_t *arguments)
{
  uint8_t keepAliveMode = sl_cli_get_argument_uint8(arguments, 0);
  if (keepAliveMode) { // to pass with subgig-interface
    sli_zigbee_power_negotiation_not_supported(sli_zigbee_what_i_support_as_a_parent);
  }

  sl_status_t status = sl_zigbee_set_keep_alive_mode(keepAliveMode);
  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_println("Keep alive support enabled.");
  } else {
    sl_zigbee_core_debug_println("failed to set keep alive mode error 0x%0x", (uint32_t)status);
  }
}

void useLegacyEndDeviceTimeoutMethod(sl_cli_command_arg_t *arguments)
{
  useLegacyTimeoutMethod = (bool)sl_cli_get_argument_uint32(arguments, 0);
}

void makeEndDeviceSleepy(sl_cli_command_arg_t *arguments)
{
  uint8_t makeSleepy = sl_cli_get_argument_uint8(arguments, 0);
  if (makeSleepy) {
    #if defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)
    sli_zigbee_af_current_zigbee_pro_network->nodeType = SL_ZIGBEE_SLEEPY_END_DEVICE;
    #endif // SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
    sli_zigbee_set_node_type(SL_ZIGBEE_SLEEPY_END_DEVICE);
  } else {
    #if defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)
    sli_zigbee_af_current_zigbee_pro_network->nodeType = SL_ZIGBEE_END_DEVICE;
    #endif // SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
    sli_zigbee_set_node_type(SL_ZIGBEE_END_DEVICE);
  }
}

void sendPermitJoinRequest(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  uint16_t sl_zigbee_node_id = sl_cli_get_argument_uint16(arguments, 0);
  status = sl_zigbee_permit_joining_request(sl_zigbee_node_id,
                                            0xFF,
                                            false, //Authenication.
                                            0);
  sl_zigbee_core_debug_println("sendPermitJoinRequest 0x%02x \r\n", status);
}

void printMtorrReceived(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_zigbee_core_debug_println("Total MTORR's received: %u", MtorrReceived);
}

void toggleMtorrFlowControl(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_mtorr_flow_control = sl_cli_get_argument_uint8(arguments, 0);
}

void setNewBroadcastEntryThreshold(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_new_broadcast_entry_threshold = sl_cli_get_argument_uint8(arguments, 0);
}

void setPassiveAckConfig(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  uint8_t config = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t min_acks_needed = sl_cli_get_argument_uint8(arguments, 1);
  status = sl_zigbee_set_passive_ack_config((sl_passive_ack_config_enum_t)config,
                                            min_acks_needed);
  if (SL_STATUS_OK == status) {
    sl_zigbee_core_debug_println("Passive ACKs configured: config=%d, min_acks=%d\n",
                                 (sl_passive_ack_config_enum_t)config,
                                 min_acks_needed);
  } else {
    sl_zigbee_core_debug_println("Error: Invalid value passive\n");
  }
}

void rejoinDiffDeviceType(sl_cli_command_arg_t *arguments)
{
  bool haveCurrentNetworkKey = sl_cli_get_argument_uint8(arguments, 0);
  uint32_t channelMask = sl_cli_get_argument_uint32(arguments, 1);
  uint8_t sl_zigbee_node_type = sl_cli_get_argument_uint8(arguments, 2);
  sl_status_t status = sl_zigbee_find_and_rejoin_network(haveCurrentNetworkKey,
                                                         channelMask,
                                                         SL_ZIGBEE_REJOIN_DUE_TO_APP_EVENT_1,
                                                         sl_zigbee_node_type);
  sl_zigbee_core_debug_println("Rejoin different 0x%02x \r\n", status);
}

void updateLinkKey(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_status_t status = sl_zigbee_request_link_key(NULL);

  sl_zigbee_core_debug_println("UpdateLinkKey 0x%0x \r\n", status);
}

//------------------------------------------------------------------------------

void requestKeyOptionEncryptCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t partnerAddress;
  sl_status_t status;
  uint8_t* address;
  sl_zigbee_current_security_state_t securityState;
  uint8_t optionEncrypt = sl_cli_get_argument_uint8(arguments, 1);
  if (sl_zigbee_copy_eui64_arg(arguments, 0, partnerAddress, false)) {
    address = partnerAddress;
  } else {
    if ( SL_STATUS_OK != sl_zigbee_get_current_security_state(&securityState) ) {
      printErrorMessage("TC Address unknown.");
      return;
    }
    address = securityState.trustCenterLongAddress;
  }

  status = sl_zigbee_request_link_key_with_option_encrypt(address, optionEncrypt);
  if ( SL_STATUS_OK != status ) {
    printCommandStatus(status, NULL, "Request Link Key failed.");
  }
}

// This API allows us to proxy a message from another node, effectively spoofing
// its Device Announce
void sendDeviceAnnounce(sl_cli_command_arg_t *arguments)
{
  uint16_t shortId = sl_cli_get_argument_uint16(arguments, 0);

  sl_802154_long_addr_t sourceEui;
  sl_802154_long_addr_t deviceAnnounceEui;
  sl_zigbee_copy_eui64_arg(arguments, 1, sourceEui, false);
  sl_zigbee_copy_eui64_arg(arguments, 2, deviceAnnounceEui, false);
  uint8_t capabilities = sl_cli_get_argument_uint8(arguments, 3);

  // There is at least one test case (12.14) that wants a device to spoof a
  // Device Announce from another node with a Device Announce (ZDO) payload that
  // shows an EUI of FFFFFFFFFFFFFFFF. Since we need the spoof to work
  // correctly, we need both the source EUI to put into the network security
  // layer and the dummy EUI (all Fs) into the ZDO layer

  sl_zigbee_test_spoof_device_announcement(shortId,
                                           sourceEui,
                                           deviceAnnounceEui,
                                           capabilities);
}

#ifndef SL_ZIGBEE_LEAF_STACK
void sendParentAnnounce(sl_cli_command_arg_t *arguments)
{
  uint8_t startIndex = sl_cli_get_argument_uint8(arguments, 0);

  sli_zigbee_parent_announce_index = startIndex;
  sl_zigbee_send_parent_announcement();
}
#endif

void addTransientKeys(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t partnerEui64;
  sl_zigbee_sec_man_key_t plaintext_key;
  sl_zigbee_copy_eui64_arg(arguments, 0, partnerEui64, false);
  sl_zigbee_copy_hex_arg(arguments,
                         1,
                         sl_zigbee_key_contents((sl_zigbee_key_data_t*) &plaintext_key),
                         SL_ZIGBEE_ENCRYPTION_KEY_SIZE,
                         true);

  sl_status_t status = sl_zigbee_sec_man_import_transient_key(partnerEui64, &plaintext_key);

  sl_zigbee_core_debug_println("add transient 0x%02lX \r\n",
                               (unsigned long)status);
}

void setAllowRejoins(sl_cli_command_arg_t *arguments)
{
  bool allowRejoins = (bool)sl_cli_get_argument_uint32(arguments, 0);

  // Set a timeout of 0 (don't set the value to false after default timeout)..
  sl_zigbee_set_tc_rejoins_using_well_known_key_timeout_sec(0);
  // ..and update the value
  sl_zigbee_set_tc_rejoins_using_well_known_key_allowed(allowRejoins);
}

// sendMulticastHello
void sendMulticastHello(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  uint8_t payload[EUI64_SIZE + HELLO_MSG_SIZE];
  sl_status_t status;
  sl_zigbee_aps_frame_t apsFrame;
  uint8_t data[HELLO_MSG_SIZE] = { 'h', 'e', 'l', 'l', 'o' };

  // the data - my long address and the string "hello"
  memmove(&(payload[0]), sl_zigbee_get_eui64(), EUI64_SIZE);
  memmove(&(payload[EUI64_SIZE]), data, HELLO_MSG_SIZE);

  // all of the defined values below are from app/sensor/common.h
  // with the exception of SL_ZIGBEE_APS_OPTION_NONE from stack/include/sl_zigbee.h
  apsFrame.profileId = PROFILE_ID;          // profile unique to this app
  apsFrame.clusterId = MSG_MULTICAST_HELLO; // message type
  apsFrame.sourceEndpoint = ENDPOINT;       // sensor endpoint
  apsFrame.destinationEndpoint = ENDPOINT;  // sensor endpoint
  apsFrame.options = SL_ZIGBEE_APS_OPTION_NONE; // none for multicast
  apsFrame.groupId = MULTICAST_ID;          // multicast ID unique to this app
  apsFrame.sequence = 0;                    // use seq of 0

  // send the message
  status = sl_zigbee_send_multicast(&apsFrame, // multicast ID & cluster
                                    10,        // radius
                                    6,        // non-member radius
                                    SL_ZIGBEE_NULL_NODE_ID,
                                    0,
                                    0x00,        // tag
                                    EUI64_SIZE + HELLO_MSG_SIZE,
                                    payload,
                                    NULL);

  sl_zigbee_core_debug_print("TX (multicast hello), status is 0x%02X\r\n",
                             status);
}

#if     (defined(PHY_DUAL) || defined(PHY_SIMULATION_DUAL) || defined(PHY_SIMULATION_GB) || defined(PHY_RAILGB))
void sendLPD(sl_cli_command_arg_t *arguments)
{
  uint8_t options = sl_cli_get_argument_uint8(arguments, 0);
  uint16_t target = sl_cli_get_argument_uint16(arguments, 1);
  uint8_t delta = sl_cli_get_argument_uint8(arguments, 2);
  sli_zigbee_send_link_power_delta(options, target, delta);
}

void sendMalformedLPD(sl_cli_command_arg_t *arguments)
{
  const uint16_t node_id = sl_cli_get_argument_uint16(arguments, 0);

  uint8_t commandFrame[] = {
    0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x53, 0x6f, 0x2c, 0x20, 0x75, 0x6e, 0x63, 0x6c, 0x65, 0x2c,
    0x20, 0x74, 0x68, 0x65, 0x72, 0x65, 0x20, 0x79, 0x6f, 0x75, 0x20, 0x61, 0x72, 0x65, 0x2e, 0x20,
    0x4e, 0x6f, 0x77, 0x20, 0x74, 0x6f, 0x20, 0x6d, 0x79, 0x20, 0x77, 0x6f, 0x72, 0x64, 0x3b, 0x0d,
    0x0a, 0x20, 0x20, 0x20, 0x20, 0x49, 0x74, 0x20, 0x69, 0x73, 0x20, 0x27, 0x41, 0x64, 0x69, 0x65,
    0x75, 0x2c, 0x20, 0x61, 0x64, 0x69, 0x65, 0x75, 0x21, 0x20, 0x72, 0x65, 0x6d, 0x65, 0x6d, 0x62,
    0x65, 0x72
  };

  uint8_t len = sizeof(commandFrame);
  sl_802154_long_addr_t testSubjectEUID;
  sl_zigbee_lookup_eui64_by_node_id(node_id, testSubjectEUID);

  slx_zigbee_network_send_command(node_id,
                                  commandFrame,
                                  len,
                                  false,
                                  testSubjectEUID);
}

void makeUseofNegotiatedPower(sl_cli_command_arg_t *arguments)
{
  useNegotiatedPowerbyLinkPowerDelta = sl_cli_get_argument_uint8(arguments, 0);
}

uint8_t dcTestState;

void printDcStats(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  EmPhyDcLimits dcLimits;
  EmPhyDcHectoPct curDc = emPhyDcGetDcAndLimits(&dcLimits);
  sl_zigbee_core_debug_print("mode %0x talk %d limit %d crit %d susp %d used %d\r\n",
                             emPhyDcGetMode(),
                             dcLimits.talkLimit,
                             dcLimits.limiThresh,
                             dcLimits.critThresh,
                             dcLimits.suspLimit,
                             curDc);
#if 0
  bool verbose = !!sl_zigbee_unsigned_command_argument(state, 0);
  if (verbose) {
    EmPhyDutyCycle* dcEntry = emPhyDcEntry();
    uint8_t bucket;
    if (dcEntry == NULL) {
      return;
    }
    // Show current channel's duty cycle measurement
    responsePrintf("{&n &th &tu &tl &tl\r\n  &tl &tl &tl &tl\r\n&tu: ",
                   "dcStatsV",
                   "channel", dcEntry->macPgChanPlusOne - 1,
                   "state", emPhyDcGetState(),
                   "maxBy", dcEntry->dcPeriodMaximumBytes,
                   "usedBy", dcEntry->dcPeriodConsumedBytes,
                   "talkBy", dcEntry->dcPeriodNonRdcBytes,
                   "nodcBy", dcEntry->dcPeriodThresholdBytes,
                   "critBy", dcEntry->dcPeriodCriticalBytes,
                   "lbtBy", dcEntry->dcPeriodLbtRdcBytes,
                   "bucket", emPhyDcBucketIndex(dcEntry->dcLastBucketStartMsTick));
    for (bucket = 0; bucket < DC_NUM_BUCKETS; bucket++) {
      responsePrintf("{%ld}", dcEntry->dcBucketBytes[bucket]);
    }
    responsePrintf("\r\nParams: &tu &tu &tu &tu &tu\r\n        &tu &tu &tu &tl}\r\n",
                   "talkRdc", (uint16_t)dcEntry->dcParams.nonRdcDenom10 * 10,
                   "criRdc", dcEntry->dcParams.criRdcDenom,
                   "lbtRdc", dcEntry->dcParams.lbtRdcDenom,
                   "byteUs", dcEntry->dcParams.byteTimeUs,
                   "pktOvh", dcEntry->dcParams.pktOvhBytes,
                   "maxPkt", dcEntry->dcParams.maxPktBytes,
                   "maxSeq", dcEntry->dcParams.maxSeqBytes,
                   "lbtCcaUs", dcEntry->dcParams.lbtCcaTimeUs,
                   "minOffUs", dcEntry->dcParams.lbtMinOffTimeUs);
  }
#endif
}
#endif

void versionCommandZCP(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_zigbee_core_debug_print("Version %d.%d.%d.%d (Build %d Type %d)\n",
                             SL_ZIGBEE_MAJOR_VERSION,
                             SL_ZIGBEE_MINOR_VERSION,
                             SL_ZIGBEE_PATCH_VERSION,
                             SL_ZIGBEE_SPECIAL_VERSION,
                             SL_ZIGBEE_BUILD_NUMBER,
                             SL_ZIGBEE_VERSION_TYPE);
}

void concentratorActionCommand(sl_cli_command_arg_t *arguments)
{
  switch (sl_cli_get_argument_uint8(arguments, 0)) {
    case 0:
      sl_zigbee_set_source_route_discovery_mode(SL_ZIGBEE_SOURCE_ROUTE_DISCOVERY_ON);
      break;
    case 1:
      sl_zigbee_set_source_route_discovery_mode(SL_ZIGBEE_SOURCE_ROUTE_DISCOVERY_OFF);
      break;
    case 2:
      sl_zigbee_set_source_route_discovery_mode(SL_ZIGBEE_SOURCE_ROUTE_DISCOVERY_RESCHEDULE);
      break;
    case 3:
      sl_zigbee_concentrator_note_route_error(SL_STATUS_ZIGBEE_SOURCE_ROUTE_FAILURE, SL_ZIGBEE_NULL_NODE_ID);
      break;
    case 4:
      sl_zigbee_concentrator_note_delivery_failure(SL_ZIGBEE_OUTGOING_DIRECT, SL_STATUS_ZIGBEE_DELIVERY_FAILED);
      break;
    default: {
    }
  } //close switch.
}

void sendRandomPacketCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t len = sl_cli_get_argument_uint8(arguments, 0);
  if (len <= 2 || len > 127) {
    return;
  }
  len -= 2;

  sl_status_t status;
  uint8_t i;
  uint8_t rawPacketBuffer[127] = { 0 };
  uint8_t frameControl[] = { 0, 0 };

  memmove(rawPacketBuffer, frameControl, sizeof(frameControl));
  for (i = 3; i < len; i++) {
    rawPacketBuffer[i] = i;
  }

  status = sl_zigbee_send_raw_message(rawPacketBuffer, len, SL_802154_TRANSMIT_PRIORITY_NORMAL, true);

  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_println("raw packet sent");
  } else {
    sl_zigbee_core_debug_println("send failed");
  }

  (void) sli_legacy_serial_wait_send(serialPort);
}

#if     (defined(PHY_DUAL) || defined(PHY_SIMULATION_DUAL) || defined(PHY_SIMULATION_GB) || defined(PHY_RAILGB))
uint8_t *packets[] = {
  (uint8_t *)"off",
  (uint8_t *)"NORMAL",
  (uint8_t *)"NORMAL",
  (uint8_t *)"LIMITED",
  (uint8_t *)"LIMITED",
  (uint8_t *)"ill5",
  (uint8_t *)"CRITICAL",
  (uint8_t *)"ill7",
  (uint8_t *)"CRITICAL",
  (uint8_t *)"ill",
  (uint8_t *)"ill",
  (uint8_t *)"ill",
  (uint8_t *)"ill",
  (uint8_t *)"ill",
  (uint8_t *)"ill",
  (uint8_t *)"ill",
  (uint8_t *)"SUSPENDED",
  (uint8_t *)"items"
};

uint8_t hashes[] = "###";

uint8_t packetstart[] = { 0, 0, 0, '@' };
uint16_t dcPacketCount = 0;
void sendPacket(void)
{
  uint8_t i;
  uint8_t state = dcTestState + 0x30;
  /*sli_buffer_manager_buffer_t buf = sl_legacy_buffer_manager_fill_linked_buffers(packetstart,3);
     sl_legacy_buffer_manager_append_to_linked_buffers(buf,&state,1);
     sl_legacy_buffer_manager_append_to_linked_buffers(buf,packets[emPhyDcGetState()],strlen(packets[emPhyDcGetState()]));
   */
  uint8_t buf[120] = { 0 };
  uint8_t timestamp[] = { 0, 0, 0, 0 };
  uint8_t rampup[] = { 0, 0 };
  uint8_t rampdown[] = { 0, 0 };

  memmove(buf, packetstart, 4);
  buf[4] = LOW_BYTE(dcPacketCount);
  buf[5] = HIGH_BYTE(dcPacketCount);
  memmove(&buf[6], timestamp, 4); /* timestamp */
  memmove(&buf[10], rampup, 2);   /* rampup */
  memmove(&buf[12], rampdown, 2); /* rampdown */
  memmove(&buf[13], &state, 1);
  memmove(&buf[14], &hashes, 3);
  memmove(&buf[17], &packets[emPhyDcGetState()], strlen((char*)packets[emPhyDcGetState()]));
  for (i = 17 + strlen((char*)packets[emPhyDcGetState()]); i < 120; i++) {
    buf[i] = '#';
  }

  sl_zigbee_send_raw_message(buf, 120, SL_802154_TRANSMIT_PRIORITY_NORMAL, true);
}

uint8_t eotpacket[] = "__END_OF_TEST__";
void sendEot(void)
{
  sl_zigbee_send_raw_message(eotpacket, strlen((char*)eotpacket), SL_802154_TRANSMIT_PRIORITY_NORMAL, true);
}
void sli_802154phy_dc_set_time_accel(uint16_t timeAccel);
void ccaThresh(sl_cli_command_arg_t *arguments)
{
  sl_mac_set_cca_threshold(sl_cli_get_argument_int8(arguments, 0));
}
void runDutyCycleTest(sl_cli_command_arg_t *arguments)
{
  dcTestState = 1;
  sli_802154phy_dc_set_time_accel(sl_cli_get_argument_uint16(arguments, 0));
  emPhyDcSetMode(EM_PHY_DC_MODE_DIALOG);
  dcPacketCount = 0;
  sendPacket();
}
uint8_t cbparm;
void dcPause()
{
  cbparm = 2;
  sl_zigbee_af_event_set_delay_ms(dcTestEvent, 2);
}

uint8_t count = 0;
uint8_t lastparm;
uint8_t lastteststate;
EmPhyDcState lastdcstate;

static void dcCallback(uint8_t parm)
{
  EmPhyDcState dcstate = emPhyDcGetState();
  if (parm != lastparm || lastteststate != dcTestState || lastdcstate != dcstate) {
    sl_zigbee_core_debug_print("DCC parm: %0x teststate: %0x dcstate %s\n", parm, dcTestState, packets[dcstate]);
  }
  lastparm = parm;
  lastteststate = dcTestState;
  lastdcstate = dcstate;

  switch (dcTestState) {
    case 1:
      if (dcstate != EM_PHY_DC_STATE_NORMAL && dcstate != EM_PHY_DC_STATE_REQUIRE_LBT) {
        dcTestState++;
        dcPause();
      } else {
        sendPacket();
      }
      break;
    case 2:
      if (dcstate == EM_PHY_DC_STATE_NORMAL || dcstate == EM_PHY_DC_STATE_REQUIRE_LBT) {
        dcTestState++;
        sendPacket();
      } else {
        dcPause();
      }
      break;

    case 3:
      if (dcstate == EM_PHY_DC_STATE_CRITICAL_LBT || dcstate == EM_PHY_DC_STATE_CRITICAL_NODC) {
        dcTestState++;
        dcPause();
      } else {
        sendPacket();
      }
      break;
    case 4:
      if (dcstate == EM_PHY_DC_STATE_LIMITED_LBT || dcstate == EM_PHY_DC_STATE_LIMITED_TALK || dcstate == 2 || dcstate == 1) {
        dcTestState++;
        sendPacket();
      } else {
        dcPause();
      }
      break;

    case 5:
      if (dcstate == EM_PHY_DC_STATE_SUSPENDED || parm) {
        dcTestState++;
        count = 0;
        dcPause();
      } else {
        sendPacket();
      }
      break;
    case 6:
      if (!parm) {
        count++;
      }
      if (count == 2) {
        dcTestState++;
        count = 0;
        dcPause();
      } else {
        sendPacket();
      }

      break;
    case 7:
      if (dcstate == EM_PHY_DC_STATE_LIMITED_LBT || dcstate == EM_PHY_DC_STATE_LIMITED_TALK || dcstate == 2 || dcstate == 1) {
        sendPacket();
        count++;
        if (count == 2) {
          dcTestState++;
          count = 0;
        }
      } else {
        dcPause();
      }
      break;
    case 8:
      if (dcstate == EM_PHY_DC_STATE_NORMAL || dcstate == EM_PHY_DC_STATE_REQUIRE_LBT) {
        sendPacket();
        count++;
        if (count == 2) {
          dcTestState++;
          count = 0;
        }
      } else {
        dcPause();
      }
      break;
    case 9:
      sendEot();
      dcTestState++;
      emPhyDcSetMode(EM_PHY_DC_MODE_LBT);
      break;
  }
}

static void dcTestEventHandler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_zigbee_af_event_set_inactive(dcTestEvent);
  dcCallback(cbparm);
}
#endif

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_raw_transmit_complete_handler_pro_compliance(uint8_t messageLength, uint8_t* messageContents, sl_status_t status, uint8_t messageTag)
#else
void sl_zigbee_raw_transmit_complete_handler(uint8_t messageLength, uint8_t* messageContents, sl_status_t status, uint8_t messageTag)
#endif
{
  (void)messageLength;
  (void)messageContents;

  if (messageTag > 0) {
    sl_zigbee_core_debug_println("raw tx complete for msg tag 0x%x", messageTag);
  }

  if (status == SL_STATUS_OK) {
#if     (defined(PHY_DUAL) || defined(PHY_SIMULATION_DUAL) || defined(PHY_SIMULATION_GB) || defined(PHY_RAILGB))
    cbparm = 0;
    sl_zigbee_af_event_set_delay_ms(dcTestEvent, 1);
    dcCallback(0);
#endif
  } else {
#if     (defined(PHY_DUAL) || defined(PHY_SIMULATION_DUAL) || defined(PHY_SIMULATION_GB) || defined(PHY_RAILGB))
    dcCallback(1);
    cbparm = 1;
    sl_zigbee_af_event_set_delay_ms(dcTestEvent, 1000);
#endif
#ifdef MAC_TEST_COMMANDS_SUPPORT
    if ( sli_zigbee_enable_mac_certification_test_mode ) {
      status = sli_zigbee_map_ember_error_codes_to_mac_certification_tests(status);
    }
#endif
    sl_zigbee_core_debug_println("transmit error 0x%02X",
                                 status);
  }
}

void joinListRequestCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t startIndex = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_test_join_list_request(startIndex);
}

void joinListAddCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t command = sl_cli_get_argument_uint8(arguments, 0);

  // first argument is commandId
  uint8_t longId_entries = sl_cli_get_argument_count(arguments) - 1;
  //aligns with default IPC maximum, although 9 is the highest used in tests
  #define MAX_JOIN_LIST_LONG_IDS 16
  sl_802154_long_addr_t longIdList[MAX_JOIN_LIST_LONG_IDS];

  if (command == 1) {
    for (uint8_t i = 1; i <= longId_entries; i++) {
      sl_zigbee_copy_eui64_arg(arguments, i, longIdList[i - 1], false);
    }
  }

  sl_zigbee_test_join_list_add(command,
                               (uint8_t*)longIdList,
                               longId_entries);
}

// This is to unicast or broadcast an ieee address request message.
// Need a cli command to broadcast ieee address request for 12.30 test case added as
// part for R22+. There is a cli command ieee_address defined in app/util/common/common.c
// to unicast ieee addr request, however it can't be modified to broadcast
// as there is no public api for sli_zigbee_ieee_address_request_to_target.
void ieeeAddressRequestCommand(sl_cli_command_arg_t *arguments)
{
  bool reportKids = (bool)sl_cli_get_argument_uint32(arguments, 1);
  sl_802154_short_addr_t discoveryNode = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 2);
  sl_zigbee_test_ieee_address_request_to_target(discoveryNode,
                                                reportKids,
                                                0,
                                                SL_ZIGBEE_ZDO_ENDPOINT + 1,
                                                SL_ZIGBEE_APS_OPTION_NONE,
                                                target);
}

void ebrAddEuiandPower(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t longId;
  sl_zigbee_copy_eui64_arg(arguments, 0, longId, false);
  int8_t power = sl_cli_get_argument_int8(arguments, 1);
  if (sli_zigbee_add_ebr_eui_and_power_entry(longId, power)) {
    sl_zigbee_core_debug_print("Entry added in ebr-list:  ");
    printBigEndianEui64(serialPort, longId);
    sl_zigbee_core_debug_println(" Power %d", power);
  } else {
    sl_zigbee_core_debug_println("Ebr-list is full");
  }
}

void ebrFindOrRemovePowerByEui64(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t longId;
  uint8_t remove = false;
  int8_t power;

  remove = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_copy_eui64_arg(arguments, 1, longId, false);
  if (remove) {
    sli_zigbee_remove_ebr_power_and_eui64_entry(longId);
  } else {
    power  = sli_zigbee_find_ebr_power_by_eui64(longId);
    printBigEndianEui64(serialPort, longId);
    sl_zigbee_core_debug_println(" Power: %d", power);
  }
}

void printEbrPowerAndEuiList(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  uint8_t  *buffer, *ptemp;
  uint8_t i = 0, count = sli_zigbee_get_ebr_power_and_eui_list_pointer(&buffer);

  if (!count && buffer == NULL) {
    sl_zigbee_core_debug_println("No entries present in ebr-list");
    return;
  }
  sl_zigbee_core_debug_println("Count %d", count);
  do {
    ptemp = buffer + (i * 9);   //DEVICE_EBR_EUI_POWER_PER_PAYLOAD_SIZE = 9
    sl_zigbee_core_debug_print("%d: ", i);
    printBigEndianEui64(serialPort, ptemp);
    sl_zigbee_core_debug_println("power %d", (int8_t)*(ptemp + EUI64_SIZE));
  } while (++i < count);

  sl_zigbee_core_debug_println("");
  sl_zigbee_core_debug_println("Printing buffer, if required");
  i = 0;
  while (i < 32) {
    sl_zigbee_core_debug_print("i=%d p=0x%0x ", i, *(buffer++));
    i++;
  }
  sl_zigbee_core_debug_println("");
}

void lpdEventCommand(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_link_power_delta_interval = sl_cli_get_argument_uint16(arguments, 0);

  if (sl_zigbee_link_power_delta_interval) {
    sli_zigbee_lpd_event_handler(NULL);
  } else {
    sli_zigbee_send_link_power_delta(1 /* request */, sli_zigbee_parent_id, 3 /* delta */);
  }
}

void printLinkPowerByNodeId(sl_cli_command_arg_t *arguments)
{
  uint16_t nodeId = sl_cli_get_argument_uint16(arguments, 0);
  int8_t linkPower;

  if (sli_zigbee_node_type_is_end_device()) {
    (void)nodeId; // as end device has single power value to communicate to parent
    linkPower = sli_zigbee_get_new_power_calculated_using_lpd();
  } else {
    linkPower = sl_zigbee_child_power(sl_zigbee_child_index(nodeId));
  }

  sl_zigbee_core_debug_println("Link power for 0x%02x is %d", nodeId, linkPower);
}

void sendNetworkCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t networkCommandId = sl_cli_get_argument_uint8(arguments, 0);
  sl_802154_short_addr_t destNodeId = sl_cli_get_argument_uint16(arguments, 1);
  uint8_t payload[UINT8_BUFFER_PAYLOAD_SIZE];
  payload[0] = networkCommandId;
  uint8_t* buffer = payload + NWK_CMD_OVERHEAD;
  uint8_t payloadLen = sl_zigbee_copy_hex_arg(arguments, 2, buffer, NWK_CMD_MAX_PAYLOAD, false);

  sl_802154_long_addr_t destinationLong;
  bool haveDestinationLong =
    (sl_zigbee_lookup_eui64_by_node_id(destNodeId, destinationLong) == SL_STATUS_OK);

  sl_zigbee_test_network_send_command(destNodeId,
                                      payload,
                                      NWK_CMD_OVERHEAD + payloadLen,
                                      haveDestinationLong,
                                      destinationLong);
}

void setDutyCycleLimitsInStack(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_duty_cycle_limits_t setLimits;
  setLimits.limitThresh = sl_cli_get_argument_uint16(arguments, 0);
  setLimits.critThresh = sl_cli_get_argument_uint16(arguments, 1);
  setLimits.suspLimit = sl_cli_get_argument_uint16(arguments, 2);

  sl_status_t status = sl_zigbee_set_duty_cycle_limits_in_stack(&setLimits);
  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_println(" Limits: limitThresh = %d critThresh %d suspLimit %d",
                                 setLimits.limitThresh, setLimits.critThresh, setLimits.suspLimit);
  } else {
    sl_zigbee_core_debug_println("Failed status 0x%0x", status);
  }
}

void getDutyCycleLimits(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_zigbee_duty_cycle_limits_t getLimits;

  sl_status_t status = sl_zigbee_get_duty_cycle_limits(&getLimits);
  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_println("limitThresh = %d critThresh %d suspLimit %d",
                                 getLimits.limitThresh, getLimits.critThresh, getLimits.suspLimit);
  } else {
    sl_zigbee_core_debug_println("Failed status 0x%0x", status);
  }
}

void getCurrentDutyCycle(sl_cli_command_arg_t *arguments)
{
  //maxDevice up to 32 + 1 (local device) = 33
  uint8_t i;
  sl_zigbee_per_device_duty_cycle_t arrayOfDeviceDutyCycles[SL_ZIGBEE_MAX_CHILDREN_FOR_PER_DEVICE_DUTY_CYCLE_MONITOR + 1];

  uint8_t maxDevices = sl_cli_get_argument_uint8(arguments, 0);
  maxDevices = maxDevices > (SL_ZIGBEE_MAX_CHILDREN_FOR_PER_DEVICE_DUTY_CYCLE_MONITOR + 1)
               ? (SL_ZIGBEE_MAX_CHILDREN_FOR_PER_DEVICE_DUTY_CYCLE_MONITOR + 1)
               : maxDevices;

  if (SL_STATUS_OK == sl_zigbee_get_current_duty_cycle(maxDevices, arrayOfDeviceDutyCycles)) {
    for (i = 0; i < maxDevices; i++ ) {
      sl_zigbee_core_debug_println("nodeId 0x%04X ---> dutyCycleConsumed %d",
                                   arrayOfDeviceDutyCycles[i].nodeId,
                                   arrayOfDeviceDutyCycles[i].dutyCycleConsumed);
    }
  } else {
    sl_zigbee_core_debug_println("Failed to get current duty cycle");
  }
}

void getDutyCycleState(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_zigbee_duty_cycle_state_t returnedState;

  sl_status_t status = sl_zigbee_get_duty_cycle_state(&returnedState);

  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_println(" duty cycle state = %d", returnedState);
  } else {
    sl_zigbee_core_debug_println("Failed status 0x%0x", status);
  }
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_duty_cycle_handler_pro_compliance(uint8_t channelPage,
                                                 uint8_t channel,
                                                 sl_zigbee_duty_cycle_state_t state,
                                                 uint8_t totalDevices,
                                                 sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles)
#else
void sl_zigbee_duty_cycle_handler(uint8_t channelPage,
                                  uint8_t channel,
                                  sl_zigbee_duty_cycle_state_t state,
                                  uint8_t totalDevices,
                                  sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles)
#endif
{
  (void)totalDevices;
  (void)arrayOfDeviceDutyCycles;
  // we should not call sli_legacy_serial_printf() from within an interrupt-context callback.
  // Not sure this api is designed to be reentrant.
#ifdef SL_ZIGBEE_TEST
  if (state != SL_ZIGBEE_DUTY_CYCLE_LBT_NORMAL) {
    sl_zigbee_core_debug_print("sl_zigbee_duty_cycle_handler: page %d channel %d dcState 0x%02x\r\n",
                               channelPage, channel, state);
  }
#else
  (void)channelPage;
  (void)channel;
  (void)state;
#endif
  return;
}

void setCorePrintingEnable(sl_cli_command_arg_t *arguments)
{
  proComplianceCorePrintingEnabled = (bool)sl_cli_get_argument_uint32(arguments, 0);
  sl_zigbee_af_core_println("Core printing is %s",
                            proComplianceCorePrintingEnabled ? "ON" : "OFF");
}

void addRouteCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t destNodeId = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_short_addr_t nextHopNodeId = sl_cli_get_argument_uint16(arguments, 1);
  sl_zigbee_core_debug_print("Add route: to %022X via %02X\r\n",
                             destNodeId, nextHopNodeId);

  sli_zigbee_add_route_entry(destNodeId, nextHopNodeId, ROUTE_ACTIVE, 0);
  sli_zigbee_current_lqi = 200;
  sli_zigbee_neighbor_process_quality(nextHopNodeId);
}

void deleteRouteCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t destNodeId = sl_cli_get_argument_uint16(arguments, 0);
  // For a many to one route, this will deactivate the route
  // For a non aggregator route, it will delete both route and discovery entry
  sli_zigbee_deactivate_or_delete_active_route(destNodeId);
  sl_zigbee_core_debug_print("Deleted route to %02X\n", destNodeId);
}

void clearRouteTableCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sli_zigbee_initialize_table_routing(sl_zigbee_get_current_network());
  sl_zigbee_core_debug_println("Routing Table cleared");
}

// Note: this command is compiled into the app conditionally if
// zigbee_multi_network is enabled
void showSourceRouteTableCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  // Now this command is only used by the multi-PAN tests,
  // we strip it out for SL_ZIGBEE_MULTI_NETWORK_STRIPPED cases
  // to save more code space.
  uint8_t i;
  uint8_t closerIndex;
  const sl_zigbee_source_route_table_entry_t* sli_zigbee_source_route_table = &(sli_zigbee_source_route_table_data[sl_zigbee_get_source_route_table_size() * sli_zigbee_get_network_index_for_forked_global()]);

  for (i = 0; i < sli_zigbee_source_route_get_count(); i++) {
    if (sli_zigbee_source_route_table[i].destination != SL_ZIGBEE_NULL_NODE_ID) {
      sl_zigbee_core_debug_print("%04X", sli_zigbee_source_route_table[i].destination);
      closerIndex = sli_zigbee_source_route_table[i].closerIndex;
      while (closerIndex != SOURCE_ROUTE_NULL_INDEX) {
        sl_zigbee_core_debug_print(" <- %04X", sli_zigbee_source_route_table[closerIndex].destination);
        closerIndex = sli_zigbee_source_route_table[closerIndex].closerIndex;
      }
      sl_zigbee_core_debug_println(" <- me");
      (void) sli_legacy_serial_wait_send(serialPort);
    }
  }

  sl_zigbee_core_debug_println("Entries in use: %d out of %d",
                               sli_zigbee_source_route_get_count(),
                               sl_zigbee_get_source_route_table_size());
}

void clearSourceTableCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sli_zigbee_source_route_clear_table();
  sl_zigbee_core_debug_println("Source routing Table cleared");
}

void assumeTcConcentratorTypeCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t type = sl_cli_get_argument_uint8(arguments, 0);
  if (type <= SL_ZIGBEE_ASSUME_TRUST_CENTER_IS_HIGH_RAM_CONCENTRATOR) {
    sl_zigbee_set_assumed_trust_center_concentrator_type(type);
    sl_zigbee_core_debug_println("set concentrator type 0x%0x", type);
  } else {
    sl_zigbee_core_debug_println("unknown concentrator type");
  }
}

void routeRecordPolicyCommand(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_route_record_policy_type_t policy = sl_cli_get_argument_uint8(arguments, 0);
  sl_status_t status = slx_zigbee_routing_set_route_record_policy(policy);
  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_println("route record policy set");
  } else {
    sl_zigbee_core_debug_println("route record policy not set");
  }
}

WEAK(void slx_zigbee_insecure_debug_generate_trace(slx_zigbee_insecure_debug_message_type msg_type,
                                                   void *debug_data))
{
  UNUSED_VAR(msg_type);
  UNUSED_VAR(debug_data);
}

void slx_zigbee_application_handle_new_aps_link_key_with_partner(sl_802154_long_addr_t partner)
{
  // export the key
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_init_context(&context);
  if (!sli_zigbee_am_trust_center) {
    context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;
  } else if (sl_zigbee_get_trust_center_link_key_request_policy() == SL_ZIGBEE_ALLOW_TC_LINK_KEY_REQUEST_AND_SEND_CURRENT_KEY) {
    context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;
  } else {
    context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK;
  }
  memmove(context.eui64, partner, EUI64_SIZE);
  context.flags |= ZB_SEC_MAN_FLAG_EUI_IS_VALID;
  sl_zigbee_sec_man_key_t plain_text;
  sl_status_t status = sl_zigbee_sec_man_export_key(&context,
                                                    &plain_text);
  if (status == SL_STATUS_OK) {
    // dump to wireshark
    slx_zigbee_insecure_debug_generate_trace(SLX_ZIGBEE_INSECURE_DEBUG_NWK_REPORT_KEY_PACKET,
                                             (void *) plain_text.key);
    // key transport
    slx_zigbee_insecure_debug_generate_trace(SLX_ZIGBEE_INSECURE_DEBUG_TRANSPORT_KEY_PACKET,
                                             (void *) plain_text.key);
  } else {
    sl_zigbee_core_debug_print("WARN: error (0x%02X) getting aps link key for ", status);
    sl_zigbee_core_debug_print_buffer(partner, EUI64_SIZE, false);
  }
}

#ifdef SL_CATALOG_KERNEL_PRESENT
//task size is specified in bytes and is word aligned
static osThreadId_t pro_compliance_app_task_id;
__ALIGNED(8) static uint8_t pro_compliance_app_task_stack[SL_ZIGBEE_APP_FRAMEWORK_RTOS_TASK_STACK_SIZE];
__ALIGNED(4) static uint8_t pro_compliance_app_task_cb[osThreadCbSize];
static osThreadAttr_t pro_compliance_app_task_attr;

static void pro_compliance_app_task(void *p_arg);
void pro_compliance_app_rtos_task_init(void);

sl_event_queue_t callback_event_queue;

void pro_compliance_app_subscribe_to_callback_event_queue(void)
{
  // Subscribe to publisher and set up queue for receiving events in the application framework task
  sl_event_queue_create(100, &callback_event_queue);
  sl_event_subscribe(SL_EVENT_CLASS_ZIGBEE, //event_class
                     0xFFFFFFFF,            //event_mask
                     callback_event_queue);
}

void pro_compliance_app_rtos_task_init(void)
{
  // Create Custom app task.
  pro_compliance_app_task_attr.name = "Custom App task";
  pro_compliance_app_task_attr.stack_mem = &pro_compliance_app_task_stack[0];
  pro_compliance_app_task_attr.stack_size = sizeof(pro_compliance_app_task_stack);
  pro_compliance_app_task_attr.cb_mem = pro_compliance_app_task_cb;
  pro_compliance_app_task_attr.cb_size = osThreadCbSize;
  pro_compliance_app_task_attr.priority = (osPriority_t)SL_ZIGBEE_APP_FRAMEWORK_RTOS_TASK_PRIORITY;
  pro_compliance_app_task_attr.attr_bits = 0;
  pro_compliance_app_task_attr.tz_module = 0;

  pro_compliance_app_task_id = osThreadNew(pro_compliance_app_task,
                                           NULL,
                                           &pro_compliance_app_task_attr);
  assert(pro_compliance_app_task_id != NULL);

  pro_compliance_app_subscribe_to_callback_event_queue();
}
#endif //#ifdef SL_CATALOG_KERNEL_PRESENT

static void proComplianceInit(void)
{
  initializeGroupsTable();

  securityAddressCacheInit(BASE_ADDRESS_TABLE_SIZE,
                           SECURITY_ADDRESS_CACHE_SIZE);

  // Defining MULTI_NETWORK_STRIPPED stubs out the sl_zigbee_set_current_network calls
  (void) sl_zigbee_set_current_network(1);
  sli_zigbee_network_security_level = 0;
  sl_zigbee_set_trust_center_link_key_request_policy(SL_ZIGBEE_ALLOW_TC_LINK_KEY_REQUEST_AND_SEND_CURRENT_KEY);
  sl_zigbee_set_app_link_key_request_policy(SL_ZIGBEE_ALLOW_APP_LINK_KEY_REQUEST);
  (void) sl_zigbee_set_current_network(0);
  sli_zigbee_network_security_level = 0;
  sl_zigbee_set_trust_center_link_key_request_policy(SL_ZIGBEE_ALLOW_TC_LINK_KEY_REQUEST_AND_SEND_CURRENT_KEY);
  sl_zigbee_set_app_link_key_request_policy(SL_ZIGBEE_ALLOW_APP_LINK_KEY_REQUEST);

  sli_zigbee_request_key_timeout = 0;  // no buffering by the TC before sending App keys.

  sl_zigbee_concentrator_stop_discovery();

  // TODO: this call should not be hard-coded here
  sl_zigbee_af_mac_address_filtering_init_cb(0x01);

  sli_zigbee_app_zdo_configuration_flags = SL_ZIGBEE_APP_RECEIVES_SUPPORTED_ZDO_REQUESTS;
}

extern sl_zigbee_af_event_t wildcard_key_refresh_event[];
extern void wildcard_key_refresh_event_handler(sl_zigbee_af_event_t * event);

void sli_zigbee_af_pro_compliance_init_callback(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
      sl_zigbee_af_event_init(sendEvent[0], sendEventHandler0);
      sl_zigbee_af_event_init(sendEvent[1], sendEventHandler1);
      sl_zigbee_af_event_init(sendEvent[2], sendEventHandler2);
      sl_zigbee_af_event_init(sendEvent[3], sendEventHandler3);

      sl_zigbee_af_event_init(pollEvent[0], pollEventHandler0);
      sl_zigbee_af_event_init(pollEvent[1], pollEventHandler1);
      sl_zigbee_af_event_init(pollEvent[2], pollEventHandler2);
      sl_zigbee_af_event_init(pollEvent[3], pollEventHandler3);

      sl_zigbee_af_event_init(&wildcard_key_refresh_event[0], wildcard_key_refresh_event_handler);
      sl_zigbee_af_event_init(&wildcard_key_refresh_event[1], wildcard_key_refresh_event_handler);
      sl_zigbee_af_event_init(&wildcard_key_refresh_event[2], wildcard_key_refresh_event_handler);
      sl_zigbee_af_event_init(&wildcard_key_refresh_event[3], wildcard_key_refresh_event_handler);
#if (defined(PHY_DUAL) || defined(PHY_SIMULATION_DUAL) || defined(PHY_SIMULATION_GB) || defined(PHY_RAILGB))
      sl_zigbee_af_event_init(dcTestEvent, dcTestEventHandler);
#endif // (defined(PHY_DUAL) || defined(PHY_SIMULATION_DUAL) || defined(PHY_SIMULATION_GB) || defined(PHY_RAILGB))

      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_DONE:
    {
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
      if (isZCPContext() == true)
#endif // SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
      {
        initialize();
        proComplianceInit();
      }
      // ZCP tests check the status when the app boots up.
      statusCommandZCP(NULL);
      break;
    }
  }
}

#ifdef SL_CATALOG_KERNEL_PRESENT
extern void sli_zigbee_process_stack_callbacks_event(sl_event_t *cb_event);

static void pro_compliance_app_task(void *p_arg)
{
  (void)p_arg;
  uint32_t yield_time_ms = 1;

  sli_zigbee_app_framework_init_callback(); // sl_system_common subscription, not dependent on app_framework

  while (true) {
    sli_zigbee_app_framework_tick_callback(); // sl_system_common subscription, not dependent on app_framework

    sl_event_t *event_ptr;

    //Process all events in the event queue
    while ( SL_STATUS_OK == sl_event_queue_get(callback_event_queue, 0, yield_time_ms, &event_ptr)) {
      sli_zigbee_process_stack_callbacks_event(event_ptr);
      sl_event_process(&event_ptr);
    }
  }
}
#endif
void factoryResetCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  // clear entries of the link key table
  uint8_t i;
  for (i = 0; i < sl_zigbee_key_table_size; i++) {
    eraseKeyTableEntryZCP(i);
  }
  // clear the child table
  sli_zigbee_erase_child_table();
  // clear binding related entries
  initializeGroupsTable();
  sl_zigbee_clear_binding_table();
  // clear the network key(s)
  sl_zigbee_sec_man_context_t key_context;
  sl_zigbee_sec_man_init_context(&key_context);
  key_context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_NETWORK;
  key_context.flags |= ZB_SEC_MAN_FLAG_KEY_INDEX_IS_VALID;
  key_context.key_index = 0;
  sl_zigbee_sec_man_delete_key(&key_context);
  key_context.key_index = 1;
  sl_zigbee_sec_man_delete_key(&key_context);

  halReboot();
}

extern uint8_t sli_zigbee_max_network_retries;
void setNwkRetriesCommand(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_max_network_retries = sl_cli_get_argument_uint8(arguments, 0);
}

extern uint8_t sli_super_retries_for_mac_data_poll_threshold;
void setDataPollAdditionalRetries(sl_cli_command_arg_t *arguments)
{
  sli_super_retries_for_mac_data_poll_threshold = sl_cli_get_argument_uint8(arguments, 0);
}
#ifndef SL_ZIGBEE_LEAF_STACK
extern uint8_t sl_zigbee_child_table_size;
extern uint8_t sli_zigbee_max_end_device_children;

void setChildTableSize(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_child_table_size = sl_cli_get_argument_uint8(arguments, 0);
  sli_zigbee_max_end_device_children = sl_cli_get_argument_uint8(arguments, 0);
}
#endif //SL_ZIGBEE_LEAF_STACK
// For UC apps, we have the main.c file that already
// has the main() function.

// all the complianec stubs that somehow need the af version
#include "zigbee_stack_callback_dispatcher.h"
#include "zigbee_app_framework_callback.h"
// -----------------------------------------------------------------------------
// Weak implementation of sli_zigbee_dispatch_packet_handoff_incoming_callback
WEAK(sl_zigbee_packet_action_t sli_zigbee_dispatch_packet_handoff_incoming_callback(
       sl_zigbee_zigbee_packet_type_t packetType,
       sli_buffer_manager_buffer_t packetBuffer,
       uint8_t index,
       // Return:
       void *data,
       uint8_t data_len))
{
  (void)packetType;
  (void)packetBuffer;
  (void)index;
  (void)data;
  (void)data_len;
  return SL_ZIGBEE_ACCEPT_PACKET;
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
// A callback that allows the app to instrument the stack on what to do with an
// incoming packet
sl_zigbee_packet_action_t sl_zigbee_internal_packet_handoff_incoming_handler_pro_compliance(
  sl_zigbee_zigbee_packet_type_t packetType,
  sli_buffer_manager_buffer_t packetBuffer,
  uint8_t index,
  // Return:
  void *data,
  uint8_t data_len)
#else
sl_zigbee_packet_action_t sl_zigbee_internal_packet_handoff_incoming_handler(
  sl_zigbee_zigbee_packet_type_t packetType,
  sli_buffer_manager_buffer_t packetBuffer,
  uint8_t index,
  // Return:
  void *data,
  uint8_t data_len)
#endif
{
  sl_zigbee_packet_action_t ret = sli_zigbee_dispatch_packet_handoff_incoming_callback(packetType, packetBuffer, index, data, data_len);
  sli_zigbee_dispatch_packet_handoff_incoming(packetType, packetBuffer, index, data, data_len);
  return ret;
}

void sendApsAckCommand(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_aps_frame_t apsStruct;
  memset(&apsStruct, 0, sizeof(sl_zigbee_aps_frame_t));
  apsStruct.profileId = sl_cli_get_argument_uint16(arguments, 0);
  apsStruct.clusterId = sl_cli_get_argument_uint16(arguments, 1);
  apsStruct.sourceEndpoint = sl_cli_get_argument_uint8(arguments, 2);
  apsStruct.destinationEndpoint = sl_cli_get_argument_uint8(arguments, 3);
  apsStruct.options = sl_cli_get_argument_uint8(arguments, 4);
  sl_802154_short_addr_t dest = sl_cli_get_argument_uint16(arguments, 5);
  if (sl_cli_get_argument_count(arguments) > 6) {
    apsStruct.sequence = sl_cli_get_argument_uint8(arguments, 6);
  }

  (void) sl_zigbee_send_aps_ack(apsStruct, dest);
}

void ignoreApsAckReqCommand(sl_cli_command_arg_t *arguments)
{
  bool ignore = (bool)sl_cli_get_argument_uint8(arguments, 0);
  slx_zigbee_ignore_incoming_aps_acks(ignore);
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
#define CONTEXT_ZCP 1
#define CONTEXT_BDB 0
#define CONTEXT_INVALID 2
bool useForcedAssociation = false;
void switchContextHandler(sl_cli_command_arg_t *arguments)
{
  uint8_t context = sl_cli_get_argument_uint8(arguments, 0);
  static tokStackContext tok = CONTEXT_INVALID;
  sli_zigbee_get_stack_token(&tok, COMMON_TOKEN_STACK_CONTEXT, sizeof(tok));
  if (tok != context) {
    tok = context;
    // the value in NVM is different from the context argument
    sli_zigbee_set_stack_token(COMMON_TOKEN_STACK_CONTEXT, &tok, sizeof(tok));
    sl_zigbee_core_debug_println("context set: %s", context ? "ZCP" : "BDB");
  }
}

bool isZCPContext()
{
  static tokStackContext tok = CONTEXT_INVALID;
  if (tok == CONTEXT_ZCP || tok == CONTEXT_BDB) {
    return (tok == CONTEXT_ZCP);
  }

  sli_zigbee_get_stack_token(&tok, COMMON_TOKEN_STACK_CONTEXT, sizeof(tok));
  return (tok == CONTEXT_ZCP);
}

void setForceAssociationHandler(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  useForcedAssociation = true;
}

bool isForceAssociationUsed()
{
  return useForcedAssociation;
}
#endif
