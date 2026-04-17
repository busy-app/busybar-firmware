/***************************************************************************//**
 * @file
 * @brief This file contains all of the common ZCL command and attribute handling
 * code for Ember's ZCL implementation
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

#include "../include/af.h"
#include "af-main.h"
#include "app/framework/util/common.h"
#include "global-callback.h"
#include "app/framework/util/af-event.h"
#include "app/framework/util/time-util.h"

#include "zigbee_zcl_callback_dispatcher.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_NETWORK_TEST_PRESENT
#include "network_test_config.h"
#endif //SL_CATALOG_ZIGBEE_NETWORK_TEST_PRESENT

#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
#include "app/test/pro-compliance.h"
#include "app/test/test-profile.h"
#endif

#ifndef ZCL_SL_WWAH_CLUSTER_ID
#define ZCL_SL_WWAH_CLUSTER_ID 0xFC57u
#endif
//------------------------------------------------------------------------------
// Forward Declarations
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void initSuppressionTable(void);
bool checkSuppression(void);
#endif
//------------------------------------------------------------------------------
// Globals

// Storage and functions for turning on and off devices
bool afDeviceEnabled[MAX_ENDPOINT_COUNT];

#ifdef SL_ZIGBEE_AF_ENABLE_STATISTICS
// a variable containing the number of messages send from the utilities
// since sl_zigbee_af_init was called.
uint32_t afNumPktsSent;
#endif

const sl_zigbee_af_cluster_name_t sl_zigbee_zcl_cluster_names[] = {
  CLUSTER_IDS_TO_NAMES                 // defined in print-cluster.h
  { ZCL_NULL_CLUSTER_ID, SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE, NULL },  // terminator
};

static const sl_zigbee_af_cluster_command_t staticCmd;
sl_zigbee_af_cluster_command_t curCmd;
// A pointer to the current command being processed
// This struct is allocated on the stack inside
// sl_zigbee_af_process_message. The pointer below is set
// to NULL when the function exits.
sl_zigbee_af_cluster_command_t *sli_zigbee_af_current_command;

// variable used for toggling Aps Link security. Set by the CLI
uint8_t sli_zigbee_af_test_aps_security_override = APS_TEST_SECURITY_DEFAULT;

// DEPRECATED.
uint8_t sl_zigbee_af_incoming_zcl_sequence_number = 0xFFu;

static bool afNoSecurityForDefaultResponse = false;

// Sequence used for outgoing messages if they are
// not responses.
uint8_t sl_zigbee_af_sequence_number = 0xFFu;

// A bool value so we know when the device is performing
// key establishment.
bool sli_zigbee_af_device_is_performing_key_establishment = false;

static uint8_t /*enum sl_zigbee_af_retry_override_t*/ sl_zigbee_af_aps_retry_override = SL_ZIGBEE_AF_RETRY_OVERRIDE_NONE;
static uint8_t /*enum sl_zigbee_af_disable_default_response_t*/ sli_zigbee_af_disable_default_response = SL_ZIGBEE_AF_DISABLE_DEFAULT_RESPONSE_NONE;
static uint8_t /*enum sl_zigbee_af_disable_default_response_t*/ sli_zigbee_af_saved_disable_default_response_vale = SL_ZIGBEE_AF_DISABLE_DEFAULT_RESPONSE_NONE;

// Holds the response type
uint8_t sl_zigbee_af_response_type = ZCL_UTIL_RESP_NORMAL;

static sl_zigbee_af_interpan_header_t interpanResponseHeader;

static const uint8_t sl_zigbee_af_analog_discrete_thresholds[] = {
  0x07, SL_ZIGBEE_AF_DATA_TYPE_NONE,
  0x1F, SL_ZIGBEE_AF_DATA_TYPE_DISCRETE,
  0x2F, SL_ZIGBEE_AF_DATA_TYPE_ANALOG,
  0x37, SL_ZIGBEE_AF_DATA_TYPE_DISCRETE,
  0x3F, SL_ZIGBEE_AF_DATA_TYPE_ANALOG,
  0x57, SL_ZIGBEE_AF_DATA_TYPE_DISCRETE,
  0xDF, SL_ZIGBEE_AF_DATA_TYPE_NONE,
  0xE7, SL_ZIGBEE_AF_DATA_TYPE_ANALOG,
  0xFF, SL_ZIGBEE_AF_DATA_TYPE_NONE
};

uint8_t sli_zigbee_af_extended_pan_id[EXTENDED_PAN_ID_SIZE] = { 0, 0, 0, 0, 0, 0, 0, 0, };
bool sli_command_or_default_response_submitted = false;

#ifdef SL_CATALOG_KERNEL_PRESENT
extern osMutexId_t sli_zigbee_af_mutex_id;
#endif

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
#define MAX_SUPPRESSION_ENTRIES 5
typedef struct {
  uint16_t clusterId;
  bool isClientCluster;
  uint8_t commandId;
} SuppressionTableEntry;
static SuppressionTableEntry suppressionTable[MAX_SUPPRESSION_ENTRIES];
#endif // #ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
//------------------------------------------------------------------------------

// Device enabled/disabled functions
bool sl_zigbee_af_is_device_enabled(uint8_t endpoint)
{
#if !defined(ZCL_USING_BASIC_CLUSTER_DEVICE_ENABLED_ATTRIBUTE) && (MAX_ENDPOINT_COUNT == 0)
  UNUSED_VAR(endpoint);
#endif
#ifdef ZCL_USING_BASIC_CLUSTER_DEVICE_ENABLED_ATTRIBUTE
  bool deviceEnabled;
  if (sl_zigbee_af_read_server_attribute(endpoint,
                                         ZCL_BASIC_CLUSTER_ID,
                                         ZCL_DEVICE_ENABLED_ATTRIBUTE_ID,
                                         (uint8_t *)&deviceEnabled,
                                         sizeof(deviceEnabled))
      == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return deviceEnabled;
  }
#endif
#if (MAX_ENDPOINT_COUNT > 0)
  uint8_t index;
  index = sl_zigbee_af_index_from_endpoint(endpoint);
  if (index != 0xFFu && index < (uint8_t)sizeof(afDeviceEnabled)) {
    return afDeviceEnabled[index];
  }
#endif // (MAX_ENDPOINT_COUNT > 0)
  return false;
}

void sl_zigbee_af_set_device_enabled(uint8_t endpoint, bool enabled)
{
#if !defined(ZCL_USING_BASIC_CLUSTER_DEVICE_ENABLED_ATTRIBUTE) && (MAX_ENDPOINT_COUNT == 0)
  UNUSED_VAR(endpoint);
  UNUSED_VAR(enabled);
#endif
#if (MAX_ENDPOINT_COUNT > 0)
  uint8_t index = sl_zigbee_af_index_from_endpoint(endpoint);
  if (index != 0xFFu && index < (uint8_t)sizeof(afDeviceEnabled)) {
    afDeviceEnabled[index] = enabled;
  }
#endif // (MAX_ENDPOINT_COUNT > 0)
#ifdef ZCL_USING_BASIC_CLUSTER_DEVICE_ENABLED_ATTRIBUTE
  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_BASIC_CLUSTER_ID,
                                      ZCL_DEVICE_ENABLED_ATTRIBUTE_ID,
                                      (uint8_t *)&enabled,
                                      ZCL_BOOLEAN_ATTRIBUTE_TYPE);
#endif
}

// Is the device identifying?
bool sl_zigbee_af_is_device_identifying(uint8_t endpoint)
{
#ifdef ZCL_USING_IDENTIFY_CLUSTER_SERVER
  uint16_t identifyTime;
  sl_zigbee_af_status_t status = sl_zigbee_af_read_server_attribute(endpoint,
                                                                    ZCL_IDENTIFY_CLUSTER_ID,
                                                                    ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                                                                    (uint8_t *)&identifyTime,
                                                                    sizeof(identifyTime));
  return (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS && 0u < identifyTime);
#else
  (void)endpoint;
  return false;
#endif
}

static sl_zigbee_af_difference_type_t getDiffFloat(sl_zigbee_af_difference_type_t value1,
                                                   sl_zigbee_af_difference_type_t value2)
{
  float valueFloat = 0, value2Float = 0, diffFloat = 0;
  sl_zigbee_af_difference_type_t diff = 0;

  memcpy(&valueFloat, &value1, sizeof(float));
  memcpy(&value2Float, &value2, sizeof(float));

  if (valueFloat > value2Float) {
    diffFloat = valueFloat - value2Float;
  } else {
    diffFloat = value2Float - valueFloat;
  }
  memcpy(&diff, &diffFloat, sizeof(float));
  return diff;
}

// Calculates difference. See sl_zigbee_af_difference_type_t for the maximum data size
// that this function will support.
sl_zigbee_af_difference_type_t sl_zigbee_af_get_difference(uint8_t *pData,
                                                           sl_zigbee_af_difference_type_t value,
                                                           uint8_t dataSize,
                                                           sl_zigbee_af_attribute_type_t dataType)
{
  sl_zigbee_af_difference_type_t value2 = 0, diff;
  uint8_t i;

  // only support data types up to 8 bytes
  if (dataSize > sizeof(sl_zigbee_af_difference_type_t)) {
    return 0;
  }

  // get the value
  for (i = 0; i < dataSize; i++) {
    value2 = value2 << 8;
#if (BIGENDIAN_CPU)
    value2 += pData[i];
#else //BIGENDIAN
    value2 += pData[dataSize - i - 1];
#endif //BIGENDIAN
  }

  // For semi-/double-precision users must provide their own callback to convert between precisions
  // and calculate the difference
  if (dataType == ZCL_FLOAT_SEMI_ATTRIBUTE_TYPE
      || dataType == ZCL_FLOAT_DOUBLE_ATTRIBUTE_TYPE) {
    return sl_zigbee_af_get_diff_cb(value, value2, dataType);
  } else if (dataType == ZCL_FLOAT_SINGLE_ATTRIBUTE_TYPE) {
    // If the attribute data is single-float
    return getDiffFloat(value, value2);
  } else {
    if (value > value2) {
      diff = value - value2;
    } else {
      diff = value2 - value;
    }

    return diff;
  }
}

// --------------------------------------------------

static void prepareForResponse(const sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zigbee_af_response_aps_frame.profileId           = cmd->apsFrame->profileId;
  sl_zigbee_af_response_aps_frame.clusterId           = cmd->apsFrame->clusterId;
  sl_zigbee_af_response_aps_frame.sourceEndpoint      = cmd->apsFrame->destinationEndpoint;
  sl_zigbee_af_response_aps_frame.destinationEndpoint = cmd->apsFrame->sourceEndpoint;

  // Use the default APS options for the response, but also use encryption and
  // retries if the incoming message used them.  The rationale is that the
  // sender of the request cares about some aspects of the delivery, so we as
  // the receiver should make equal effort for the response.
  sl_zigbee_af_response_aps_frame.options = SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS;
  if ((cmd->apsFrame->options & SL_ZIGBEE_APS_OPTION_ENCRYPTION) != 0U) {
    sl_zigbee_af_response_aps_frame.options |= SL_ZIGBEE_APS_OPTION_ENCRYPTION;
  }
  if ((cmd->apsFrame->options & SL_ZIGBEE_APS_OPTION_RETRY) != 0U) {
    sl_zigbee_af_response_aps_frame.options |= SL_ZIGBEE_APS_OPTION_RETRY;
  }

  if (cmd->interPanHeader == NULL) {
    sl_zigbee_af_response_destination = cmd->source;
    sl_zigbee_af_response_type &= ~ZCL_UTIL_RESP_INTERPAN;
  } else {
    sl_zigbee_af_response_type |= ZCL_UTIL_RESP_INTERPAN;
    memmove(&interpanResponseHeader,
            cmd->interPanHeader,
            sizeof(sl_zigbee_af_interpan_header_t));
    // Always send responses as unicast
    interpanResponseHeader.messageType = SL_ZIGBEE_AF_INTER_PAN_UNICAST;
  }
}

// ****************************************
// Initialize Clusters
// ****************************************
void sl_zigbee_af_init(uint8_t init_level)
{
  (void)init_level;
#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
  if (isZCPContext() == true) {
    return;
  }
#endif // SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
  uint8_t i;
#ifdef SL_ZIGBEE_AF_ENABLE_STATISTICS
  afNumPktsSent = 0;
#endif

  for (i = 0; i < SL_ZIGBEE_SUPPORTED_NETWORKS; i++) {
    (void) sl_zigbee_af_push_network_index(i);
    sl_zigbee_af_initialize_attributes(SL_ZIGBEE_BROADCAST_ENDPOINT);
    (void) sl_zigbee_af_pop_network_index();
  }
  sli_zigbee_af_init_attribute_storage();

  memset(afDeviceEnabled, true, sl_zigbee_af_endpoint_count());

  // Set up client API buffer.
  sl_zigbee_af_set_external_buffer(appResponseData,
                                   SL_ZIGBEE_AF_RESPONSE_BUFFER_LEN,
                                   &appResponseLength,
                                   &sl_zigbee_af_response_aps_frame);

// TODO: this is now done by the app framework common component

// TODO: in UC this is done via templating/bookkeeping

  sli_zigbee_af_call_inits();
}

// TODO: this is now done by the app framework common component

// ****************************************
// This function is called by the application when the stack goes down,
// such as after a leave network. This allows zcl utils to clear state
// that should not be kept when changing networks
// ****************************************
void sl_zigbee_af_stack_down(void)
{
  // (Case 14696) Clearing the report table is only necessary if the stack is
  // going down for good; if we're rejoining, leave the table intact since we'll
  // be right back, hopefully.
  // (Issue 77101) Also don't clear the table if the stack has gone down as a
  // a result of losing its parent or some other transient state where a future
  // rejoin is expected to get us back online.
  if (sl_zigbee_is_performing_rejoin() == false
      && sl_zigbee_network_state() == SL_ZIGBEE_NO_NETWORK) {
    // the report table should be cleared when the stack comes down.
    // going to a new network means new report devices should be discovered.
    // if the table isnt cleared the device keeps trying to send messages.
    sl_zigbee_af_clear_report_table_cb();
  }

  sl_zigbee_af_registration_abort_cb();
  sl_zigbee_af_trust_center_keepalive_abort_cb();
}

// ****************************************
// Print out information about each cluster
// ****************************************

uint16_t sl_zigbee_af_find_cluster_name_index_with_mfg_code(uint16_t cluster, uint16_t mfgCode)
{
  uint16_t index = 0u;
  while (sl_zigbee_zcl_cluster_names[index].id != ZCL_NULL_CLUSTER_ID) {
    if (sl_zigbee_zcl_cluster_names[index].id == cluster
        // This check sees if its a standard cluster, in which mfgCode is ignored
        // due to the name being well defined.
        // If it is manufacturer specific, then we try to check to see if we
        // know the name of the cluster within the list.
        // If the mfgCode we are given is null, then we just ignore it for backward
        // compatibility reasons
        && (cluster < 0xFC00u
            || sl_zigbee_zcl_cluster_names[index].mfgCode == mfgCode
            || mfgCode == SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE)) {
      return index;
    }
    index++;
  }
  return 0xFFFFu;
}

uint16_t sl_zigbee_af_find_cluster_name_index(uint16_t cluster)
{
  return sl_zigbee_af_find_cluster_name_index_with_mfg_code(cluster, SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
}

// This function parses into the cluster name table, and tries to find
// the index in the table that has the two keys: cluster + mfgcode.
void sl_zigbee_af_decode_and_print_cluster_with_mfg_code(uint16_t cluster, uint16_t mfgCode)
{
  uint16_t index = sl_zigbee_af_find_cluster_name_index_with_mfg_code(cluster, mfgCode);
  if (index == 0xFFFFu) {
    sl_zigbee_af_print(sl_zigbee_af_print_active_area,
                       "(Unknown clus. [0x%04X])",
                       cluster);
  } else {
    sl_zigbee_af_print(sl_zigbee_af_print_active_area,
                       "(%s)",
                       sl_zigbee_zcl_cluster_names[index].name);
  }
}

void sl_zigbee_af_decode_and_print_cluster(uint16_t cluster)
{
  sl_zigbee_af_decode_and_print_cluster_with_mfg_code(cluster, SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
}

// This function makes the assumption that
// sl_zigbee_af_current_command will either be NULL
// when invalid, or will have a valid mfgCode
// when called.
// If it is invalid, we just return the
// SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE, which we tend to use
// for references to the standard library.
uint16_t sl_zigbee_af_get_mfg_code_from_current_command(void)
{
  if (sl_zigbee_af_current_command() != NULL) {
    return sl_zigbee_af_current_command()->mfgCode;
  } else {
    return SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE;
  }
}

static void printIncomingZclMessage(const sl_zigbee_af_cluster_command_t *cmd)
{
#if ((defined(EMBER_AF_PRINT_ENABLE) && defined(SL_ZIGBEE_AF_PRINT_APP)) || (!defined(LARGE_NETWORK_TESTING) ))
  if (sl_zigbee_af_print_received_messages) {
    sl_zigbee_af_app_print("\r\nT%08X:", sl_zigbee_af_get_current_time());
    sl_zigbee_af_app_print("RX len %d, ep %02X, clus 0x%04X ",
                           cmd->bufLen,
                           cmd->apsFrame->destinationEndpoint,
                           cmd->apsFrame->clusterId);
    sl_zigbee_af_app_debug_exec(sl_zigbee_af_decode_and_print_cluster_with_mfg_code(cmd->apsFrame->clusterId, cmd->mfgCode));
    if (cmd->mfgSpecific) {
      sl_zigbee_af_app_print(" mfgId %04X", cmd->mfgCode);
    }
    sl_zigbee_af_app_print(" FC %02X seq %02X cmd %02X payload[",
                           cmd->buffer[0], // frame control
                           cmd->seqNum,
                           cmd->commandId);
    sl_zigbee_af_app_flush();
    sl_zigbee_af_app_print_buffer(cmd->buffer + cmd->payloadStartIndex, // message
                                  cmd->bufLen - cmd->payloadStartIndex, // length
                                  true);                        // spaces?
    sl_zigbee_af_app_flush();
    sl_zigbee_af_app_println("]");
  }
#else
  (void)cmd;
#endif
}

static bool dispatchZclMessage(sl_zigbee_af_cluster_command_t *cmd)
{
  uint8_t index = sl_zigbee_af_index_from_endpoint(cmd->apsFrame->destinationEndpoint);
  if (index == 0xFFu
#if (MAX_ENDPOINT_COUNT > 0)
      || (index >= MAX_ENDPOINT_COUNT)
#endif
      ) {
    sl_zigbee_af_debug_print("Drop cluster 0x%04X command 0x%02X",
                             cmd->apsFrame->clusterId,
                             cmd->commandId);
    sl_zigbee_af_debug_print(" due to invalid endpoint: ");
    sl_zigbee_af_debug_println("0x%02X", cmd->apsFrame->destinationEndpoint);
    return false;
  } else if (sl_zigbee_af_network_index_from_endpoint_index(index) != cmd->networkIndex) {
    sl_zigbee_af_debug_print("Drop cluster 0x%04X command 0x%02X",
                             cmd->apsFrame->clusterId,
                             cmd->commandId);
    sl_zigbee_af_debug_print(" for endpoint 0x%02X due to wrong %s: ",
                             cmd->apsFrame->destinationEndpoint,
                             "network");
    sl_zigbee_af_debug_println("%d", cmd->networkIndex);
    return false;
  } else if (sl_zigbee_af_profile_id_from_index(index) != cmd->apsFrame->profileId
             && (cmd->apsFrame->profileId != SL_ZIGBEE_WILDCARD_PROFILE_ID
                 || (SL_ZIGBEE_MAXIMUM_STANDARD_PROFILE_ID
                     < sl_zigbee_af_profile_id_from_index(index)))) {
    sl_zigbee_af_debug_print("Drop cluster 0x%04X command 0x%02X",
                             cmd->apsFrame->clusterId,
                             cmd->commandId);
    sl_zigbee_af_debug_print(" for endpoint 0x%02X due to wrong %s: ",
                             cmd->apsFrame->destinationEndpoint,
                             "profile");
    sl_zigbee_af_debug_println("0x%04X", cmd->apsFrame->profileId);
    return false;
  } else if ((cmd->type == SL_ZIGBEE_INCOMING_MULTICAST
              || cmd->type == SL_ZIGBEE_INCOMING_MULTICAST_LOOPBACK)
             && !sl_zigbee_af_groups_cluster_endpoint_in_group_cb(cmd->apsFrame->destinationEndpoint,
                                                                  cmd->apsFrame->groupId)) {
    sl_zigbee_af_debug_print("Drop cluster 0x%04X command 0x%02X",
                             cmd->apsFrame->clusterId,
                             cmd->commandId);
    sl_zigbee_af_debug_print(" for endpoint 0x%02X due to wrong %s: ",
                             cmd->apsFrame->destinationEndpoint,
                             "group");
    sl_zigbee_af_debug_println("0x%04X", cmd->apsFrame->groupId);
    return false;
  } else {
    return (cmd->clusterSpecific
            ? sli_zigbee_af_process_cluster_specific_command(cmd)
            : sli_zigbee_af_process_global_command(cmd));
  }
}

// a single call to process global and cluster-specific messages and callbacks.
bool sl_zigbee_af_process_message(sl_zigbee_aps_frame_t *apsFrame,
                                  sl_zigbee_incoming_message_type_t type,
                                  uint8_t *message,
                                  uint16_t msgLen,
                                  sl_802154_short_addr_t source,
                                  sli_zigbee_interpan_header_t *interPanHeader)
{
  sl_status_t sendStatus;
  bool msgHandled = false;
  sli_command_or_default_response_submitted = false;
  //reset/reinitialize curCmd
  curCmd =  staticCmd;
  if (!sl_zigbee_af_process_message_into_zcl_cmd(apsFrame,
                                                 type,
                                                 message,
                                                 msgLen,
                                                 source,
                                                 interPanHeader,
                                                 &curCmd)) {
    goto kickout;
  }

  sli_zigbee_af_current_command = &curCmd;

  // All of these should be covered by the sl_zigbee_af_cluster_command_t but are
  // still here until all the code is moved over to use the cmd. -WEH
  sl_zigbee_af_incoming_zcl_sequence_number = curCmd.seqNum;

  printIncomingZclMessage(&curCmd);
  prepareForResponse(&curCmd);

  if (sli_zigbee_af_pre_command_received(&curCmd)) {
    msgHandled = true;
    goto kickout;
  }

  if (interPanHeader == NULL) {
    bool broadcast = (type == SL_ZIGBEE_INCOMING_BROADCAST
                      || type == SL_ZIGBEE_INCOMING_BROADCAST_LOOPBACK
                      || type == SL_ZIGBEE_INCOMING_MULTICAST
                      || type == SL_ZIGBEE_INCOMING_MULTICAST_LOOPBACK);

    // if the cluster for the incoming message requires security and
    // doesnt have it return default response STATUS_FAILURE
    if (sl_zigbee_af_determine_if_link_security_is_required(curCmd.commandId,
                                                            true, // incoming
                                                            broadcast,
                                                            curCmd.apsFrame->profileId,
                                                            curCmd.apsFrame->clusterId,
                                                            curCmd.source)
        && (!(curCmd.apsFrame->options & SL_ZIGBEE_APS_OPTION_ENCRYPTION))) {
      sl_zigbee_af_debug_println("Drop clus %04X due to no aps security",
                                 curCmd.apsFrame->clusterId);
      afNoSecurityForDefaultResponse = true;
#if defined(ZCL_USING_SL_WWAH_CLUSTER_SERVER) || defined(ZCL_USING_SL_WWAH_CLUSTER_CLIENT)
      // Spec collision between Smart Energy (SE) and WWAH. As per WWAH requirements,
      // a node should reject incoming packet with default response status NOT_AUTHORIZED
      // if no APS security found for APS required clusters unlike FAILURE status for SE.
      sendStatus = sl_zigbee_af_send_default_response(&curCmd, SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED);
#else
      sendStatus = sl_zigbee_af_send_default_response(&curCmd, SL_ZIGBEE_ZCL_STATUS_FAILURE);
#endif
      if (SL_STATUS_OK != sendStatus) {
        sl_zigbee_af_debug_println("Util: failed to send %s response: 0x%02X",
                                   "default",
                                   sendStatus);
      }
      afNoSecurityForDefaultResponse = false;

      // Mark the message as processed.  It failed security processing, so no
      // other parts of the code should act upon it.
      msgHandled = true;
      goto kickout;
    }
  } else if (!(interPanHeader->options
               & SL_ZIGBEE_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS)) {
    // For safety, dump all interpan messages that don't have a long
    // source in the MAC layer.  In theory they should not get past
    // the MAC filters but this is insures they will not get processed.
    goto kickout;
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }

  if (curCmd.apsFrame->destinationEndpoint == SL_ZIGBEE_BROADCAST_ENDPOINT) {
    uint8_t i;
    for (i = 0u; i < sl_zigbee_af_endpoint_count(); i++) {
      uint8_t endpoint = sl_zigbee_af_endpoint_from_index(i);
      if (!sl_zigbee_af_endpoint_index_is_enabled(i)
          || !sl_zigbee_af_contains_cluster_with_mfg_code(endpoint, curCmd.apsFrame->clusterId, curCmd.mfgCode)) {
        continue;
      }
      // Since the APS frame is cleared after each sending,
      // we must reinitialize it.  It is cleared to prevent
      // data from leaking out and being sent inadvertently.
      prepareForResponse(&curCmd);

      // Change the destination endpoint of the incoming command and the source
      // source endpoint of the response so they both reflect the endpoint the
      // message is actually being passed to in this iteration of the loop.
      curCmd.apsFrame->destinationEndpoint      = endpoint;
      sl_zigbee_af_response_aps_frame.sourceEndpoint = endpoint;
      if (dispatchZclMessage(&curCmd)) {
        msgHandled = true;
      }
    }
  } else {
    msgHandled = dispatchZclMessage(&curCmd);
  }

  kickout:
  sl_zigbee_af_clear_response_data();
  memset(&interpanResponseHeader,
         0u,
         sizeof(sl_zigbee_af_interpan_header_t));
  sli_zigbee_af_current_command = NULL;
  return msgHandled;
}

uint8_t sl_zigbee_af_next_sequence(void)
{
  return ((++sl_zigbee_af_sequence_number) & SL_ZIGBEE_AF_ZCL_SEQUENCE_MASK);
}

uint8_t sl_zigbee_af_get_last_sequence_number(void)
{
  return (sl_zigbee_af_sequence_number & SL_ZIGBEE_AF_ZCL_SEQUENCE_MASK);
}

// the caller to the library can set a flag to say do not respond to the
// next ZCL message passed in to the library. Passing true means disable
// the reply for the next ZCL message. Setting to false re-enables the
// reply (in the case where the app disables it and then doesnt send a
// message that gets parsed).
void sl_zigbee_af_set_no_reply_for_next_message(bool set)
{
  if (set) {
    sl_zigbee_af_response_type |= ZCL_UTIL_RESP_NONE;
  } else {
    sl_zigbee_af_response_type &= ~ZCL_UTIL_RESP_NONE;
  }
}

void sl_zigbee_af_set_retry_override(sl_zigbee_af_retry_override_t value)
{
  sl_zigbee_af_aps_retry_override = value;
}

sl_zigbee_af_retry_override_t sl_zigbee_af_get_retry_override(void)
{
  return (sl_zigbee_af_retry_override_t) sl_zigbee_af_aps_retry_override;
}

void sli_zigbee_af_apply_retry_override(sl_zigbee_aps_option_t *options)
{
  if (options == NULL) {
    return;
  } else if (sl_zigbee_af_aps_retry_override == SL_ZIGBEE_AF_RETRY_OVERRIDE_SET) {
    *options |= SL_ZIGBEE_APS_OPTION_RETRY;
  } else if (sl_zigbee_af_aps_retry_override == SL_ZIGBEE_AF_RETRY_OVERRIDE_UNSET) {
    *options &= ~SL_ZIGBEE_APS_OPTION_RETRY;
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }
}

void sl_zigbee_af_set_disable_default_response(sl_zigbee_af_disable_default_response_t value)
{
  sli_zigbee_af_disable_default_response = value;
  if (value != SL_ZIGBEE_AF_DISABLE_DEFAULT_RESPONSE_ONE_SHOT) {
    sli_zigbee_af_saved_disable_default_response_vale = value;
  }
}

sl_zigbee_af_disable_default_response_t sl_zigbee_af_get_disable_default_response(void)
{
  return (sl_zigbee_af_disable_default_response_t) sli_zigbee_af_disable_default_response;
}

void sli_zigbee_af_apply_disable_default_response(uint8_t *frame_control)
{
  if (frame_control == NULL) {
    return;
  } else if (sli_zigbee_af_disable_default_response == SL_ZIGBEE_AF_DISABLE_DEFAULT_RESPONSE_ONE_SHOT) {
    sli_zigbee_af_disable_default_response = sli_zigbee_af_saved_disable_default_response_vale;
    *frame_control |= ZCL_DISABLE_DEFAULT_RESPONSE_MASK;
  } else if (sli_zigbee_af_disable_default_response == SL_ZIGBEE_AF_DISABLE_DEFAULT_RESPONSE_PERMANENT) {
    *frame_control |= ZCL_DISABLE_DEFAULT_RESPONSE_MASK;
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }
}

sl_status_t sl_zigbee_af_send_response_with_cb(sl_zigbee_af_message_sent_function_t callback)
{
  sl_status_t status;
  uint8_t label;
  sl_802154_short_addr_t alias = SL_ZIGBEE_NULL_NODE_ID;
  uint8_t seq = 0;
  sli_command_or_default_response_submitted = true;
  // If the no-response flag is set, don't send anything.
  if ((sl_zigbee_af_response_type & ZCL_UTIL_RESP_NONE) != 0U) {
    sl_zigbee_af_debug_println("ZCL Util: no response at user request");
    return SL_STATUS_OK;
  }

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
  // Check for cluster or command response suppression.
  if (checkSuppression()) {
    sl_zigbee_af_debug_println("ZCL Util: response suppressed");
    return SL_STATUS_OK;
  } else {
    sl_zigbee_af_debug_println("ZCL Util: response allowed");
  }
#endif // #ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT

  // Make sure we are respecting the request APS options
  // there are seemingly some calls to sl_zigbee_af_send_response
  //  that occur outside of the sl_zigbee_af_process_message context,
  //  which leads to a bad memory reference - AHilton
  if (sl_zigbee_af_current_command() != NULL) {
    if ((sl_zigbee_af_current_command()->apsFrame->options & SL_ZIGBEE_APS_OPTION_ENCRYPTION) != 0U) {
      sl_zigbee_af_response_aps_frame.options |= SL_ZIGBEE_APS_OPTION_ENCRYPTION;
    }
    if ((sl_zigbee_af_current_command()->apsFrame->options & SL_ZIGBEE_APS_OPTION_RETRY) != 0U) {
      sl_zigbee_af_response_aps_frame.options |= SL_ZIGBEE_APS_OPTION_RETRY;
    }
  }

  // Fill commands may increase the sequence.  For responses, we want to make
  // sure the sequence is reset to that of the request.
  if ((appResponseData[0] & ZCL_MANUFACTURER_SPECIFIC_MASK) != 0U) {
    appResponseData[3] = sl_zigbee_af_incoming_zcl_sequence_number;
  } else {
    appResponseData[1] = sl_zigbee_af_incoming_zcl_sequence_number;
  }

  // The manner in which the message is sent depends on the response flags and
  // the destination of the message.
  if ((sl_zigbee_af_response_type & ZCL_UTIL_RESP_INTERPAN) != 0U) {
    label = 'I';
    status = sl_zigbee_af_interpan_send_message_cb(&interpanResponseHeader,
                                                   appResponseLength,
                                                   appResponseData);
    sl_zigbee_af_response_type &= ~ZCL_UTIL_RESP_INTERPAN;
  } else if (sl_zigbee_af_response_destination < SL_ZIGBEE_BROADCAST_ADDRESS) {
    label = 'U';
    status = sl_zigbee_af_send_unicast_with_cb(SL_ZIGBEE_OUTGOING_DIRECT,
                                               sl_zigbee_af_response_destination,
                                               &sl_zigbee_af_response_aps_frame,
                                               appResponseLength,
                                               appResponseData,
                                               callback);
  } else {
    label = 'B';
    status = sl_zigbee_af_send_broadcast_with_cb(sl_zigbee_af_response_destination,
                                                 &sl_zigbee_af_response_aps_frame,
                                                 appResponseLength,
                                                 appResponseData,
                                                 alias,
                                                 seq,
                                                 callback);
  }
  UNUSED_VAR(label);
  sl_zigbee_af_debug_println("T%08X:TX (%s) %ccast 0x%02X%s",
                             sl_zigbee_af_get_current_time(),
                             "resp",
                             label,
                             status,
                             ((sl_zigbee_af_response_aps_frame.options
                               & SL_ZIGBEE_APS_OPTION_ENCRYPTION)
                              ? " w/ link key" : ""));
  sl_zigbee_af_debug_print("TX buffer: [");
  sl_zigbee_af_debug_flush();
  sl_zigbee_af_debug_print_buffer(appResponseData, appResponseLength, true);
  sl_zigbee_af_debug_println("]");
  sl_zigbee_af_debug_flush();

#ifdef SL_ZIGBEE_AF_ENABLE_STATISTICS
  if (status == SL_STATUS_OK) {
    afNumPktsSent++;
  }
#endif

  return status;
}

sl_status_t sl_zigbee_af_send_response(void)
{
  return sl_zigbee_af_send_response_with_cb(NULL);
}

sl_status_t sl_zigbee_af_send_immediate_default_response_with_cb(sl_zigbee_af_status_t status,
                                                                 sl_zigbee_af_message_sent_function_t callback)
{
  return sl_zigbee_af_send_default_response_with_cb(sl_zigbee_af_current_command(), status, callback);
}

sl_status_t sl_zigbee_af_send_immediate_default_response(sl_zigbee_af_status_t status)
{
  return sl_zigbee_af_send_immediate_default_response_with_cb(status, NULL);
}

sl_status_t sl_zigbee_af_send_default_response_with_cb(const sl_zigbee_af_cluster_command_t *cmd,
                                                       sl_zigbee_af_status_t status,
                                                       sl_zigbee_af_message_sent_function_t callback)
{
  uint8_t frameControl;

  // Default Response commands are only sent in response to unicast commands.
  if (cmd->type != SL_ZIGBEE_INCOMING_UNICAST
      && cmd->type != SL_ZIGBEE_INCOMING_UNICAST_REPLY) {
    return SL_STATUS_OK;
  }

  // If the Disable Default Response sub-field is set, Default Response commands
  // are only sent if there was an error.
  if ((cmd->buffer[0] & ZCL_DISABLE_DEFAULT_RESPONSE_MASK)
      && status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_STATUS_OK;
  }

  // Default Response commands are never sent in response to other Default
  // Response commands.
  if (!cmd->clusterSpecific
      && cmd->commandId == ZCL_DEFAULT_RESPONSE_COMMAND_ID) {
    return SL_STATUS_OK;
  }

  appResponseLength = 0;
  frameControl = (ZCL_GLOBAL_COMMAND
                  | (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER
                     ? ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                     : ZCL_FRAME_CONTROL_CLIENT_TO_SERVER));

  if (!cmd->mfgSpecific) {
    (void) sl_zigbee_af_put_int8u_in_resp(frameControl & (uint8_t) ~ZCL_MANUFACTURER_SPECIFIC_MASK);
  } else {
    (void) sl_zigbee_af_put_int8u_in_resp(frameControl | ZCL_MANUFACTURER_SPECIFIC_MASK);
    (void) sl_zigbee_af_put_int16u_in_resp(cmd->mfgCode);
  }
  (void) sl_zigbee_af_put_int8u_in_resp(cmd->seqNum);
  (void) sl_zigbee_af_put_int8u_in_resp(ZCL_DEFAULT_RESPONSE_COMMAND_ID);
  (void) sl_zigbee_af_put_int8u_in_resp(cmd->commandId);
  (void) sl_zigbee_af_put_int8u_in_resp(status);

  prepareForResponse(cmd);
  return sl_zigbee_af_send_response_with_cb(callback);
}

sl_status_t sl_zigbee_af_send_default_response(const sl_zigbee_af_cluster_command_t *cmd,
                                               sl_zigbee_af_status_t status)
{
  return sl_zigbee_af_send_default_response_with_cb(cmd, status, NULL);
}

bool sl_zigbee_af_determine_if_link_security_is_required(uint8_t commandId,
                                                         bool incoming,
                                                         bool broadcast,
                                                         sl_zigbee_af_profile_id_t profileId,
                                                         sl_zigbee_af_cluster_id_t clusterId,
                                                         sl_802154_short_addr_t remoteNodeId)
{
  (void)afNoSecurityForDefaultResponse; // remove warning if not used

  // If we have turned off all APS security (needed for testing), then just
  // always return false.
  if ((sli_zigbee_af_test_aps_security_override == APS_TEST_SECURITY_DISABLED) || afNoSecurityForDefaultResponse) {
    afNoSecurityForDefaultResponse = false;
    return false;
  }

  // NOTE: In general if it is a unicast, and one of the SE clusters, it
  // requires APS encryption.  A few special cases exists that we allow for
  // but those must be explicitly spelled out here.

  // Assume that if the local device is broadcasting, even if it is using one
  // of the SE clusters, this is okay.
  if (!incoming && broadcast) {
    return false;
  }

  // At this point if the CLI command has been issued, it's safe to over any other settings
  // and return.
  // This change allows HA applications to use the CLI option to enable APS security.
  if (sli_zigbee_af_test_aps_security_override == APS_TEST_SECURITY_ENABLED) {
    return true;
  } else if (sli_zigbee_af_test_aps_security_override == APS_TEST_SECURITY_DISABLED) {
    //The default return value before this change.
    return false;
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }

#ifdef SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE
  if (sl_zigbee_af_is_current_security_profile_smart_energy()) {
    // Check against profile IDs that use APS security on these clusters.
    if (profileId != SE_PROFILE_ID && profileId != SL_ZIGBEE_WILDCARD_PROFILE_ID) {
      return false;
    }

    // Loopback packets do not require security
    if (sl_zigbee_get_node_id() == remoteNodeId) {
      return false;
    }

    // This list comes from Section 5.4.6 of the SE spec.
    switch (clusterId) {
      case ZCL_TIME_CLUSTER_ID:
      case ZCL_COMMISSIONING_CLUSTER_ID:
      case ZCL_KEEPALIVE_CLUSTER_ID:
      case ZCL_PRICE_CLUSTER_ID:
      case ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID:
      case ZCL_SIMPLE_METERING_CLUSTER_ID:
      case ZCL_MESSAGING_CLUSTER_ID:
      case ZCL_TUNNELING_CLUSTER_ID:
      case ZCL_GENERIC_TUNNEL_CLUSTER_ID:
      case ZCL_PREPAYMENT_CLUSTER_ID:
      case ZCL_CALENDAR_CLUSTER_ID:
      case ZCL_DEVICE_MANAGEMENT_CLUSTER_ID:
      case ZCL_EVENTS_CLUSTER_ID:
      case ZCL_MDU_PAIRING_CLUSTER_ID:
      case ZCL_ENERGY_MANAGEMENT_CLUSTER_ID:
      case ZCL_SUB_GHZ_CLUSTER_ID:
        return true;
      case ZCL_OTA_BOOTLOAD_CLUSTER_ID:
        if (commandId == ZCL_IMAGE_NOTIFY_COMMAND_ID && broadcast) {
          return false;
        } else {
          return true;
        }
      default:
        break;
    }
  }
#else //SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE
  (void)remoteNodeId;
#endif //SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE

  // All works with all hubs commands require aps link key authorization
  if (clusterId == ZCL_SL_WWAH_CLUSTER_ID) {
    return true;
  }

  if (sl_zigbee_af_cluster_security_custom_cb(profileId,
                                              clusterId,
                                              incoming,
                                              commandId)) {
    return true;
  }

//APS_TEST_SECURITY_DEFAULT at this point returns false.
  return false;
}

uint8_t sl_zigbee_af_maximum_aps_payload_length(sl_zigbee_outgoing_message_type_t type,
                                                uint16_t indexOrDestination,
                                                sl_zigbee_aps_frame_t *apsFrame)
{
#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
  if (isZCPContext() == true) {
    return sl_zigbee_af_maximum_aps_payload_length_pro_compliance(type, indexOrDestination, apsFrame);
  }
#endif
  sl_802154_short_addr_t destination = SL_ZIGBEE_UNKNOWN_NODE_ID;
  sl_802154_long_addr_t eui64;
  uint8_t sourceRouteOverhead = SL_ZIGBEE_SOURCE_ROUTE_OVERHEAD_UNKNOWN;
  uint8_t max = SL_ZIGBEE_AF_MAXIMUM_APS_PAYLOAD_LENGTH;

  if ((apsFrame->options & SL_ZIGBEE_APS_OPTION_ENCRYPTION) != 0U) {
    max -= SL_ZIGBEE_AF_APS_ENCRYPTION_OVERHEAD;
  }
  if ((apsFrame->options & SL_ZIGBEE_APS_OPTION_SOURCE_EUI64) != 0U) {
    max -= EUI64_SIZE;
  }
  if ((apsFrame->options & SL_ZIGBEE_APS_OPTION_DESTINATION_EUI64) != 0U) {
    max -= EUI64_SIZE;
  }
  if ((apsFrame->options & SL_ZIGBEE_APS_OPTION_FRAGMENT) != 0U) {
    max -= SL_ZIGBEE_AF_APS_FRAGMENTATION_OVERHEAD;
  }

  switch (type) {
    case SL_ZIGBEE_OUTGOING_DIRECT:
      destination = indexOrDestination;
      break;
    case SL_ZIGBEE_OUTGOING_VIA_ADDRESS_TABLE:
      sl_zigbee_get_address_table_info(indexOrDestination, &destination, eui64);
      break;
    case SL_ZIGBEE_OUTGOING_VIA_BINDING:
      destination = sl_zigbee_get_binding_remote_node_id(indexOrDestination);
      break;
    case SL_ZIGBEE_OUTGOING_MULTICAST:
      // APS multicast messages include the two-byte group id and exclude the
      // one-byte destination endpoint, for a net loss of an extra byte.
      max--;
      break;
    case SL_ZIGBEE_OUTGOING_BROADCAST:
      break;
    default:
      // MISRA requires default case.
      break;
  }

  sourceRouteOverhead = sl_zigbee_af_get_source_route_overhead_cb(destination);
  if (sourceRouteOverhead != SL_ZIGBEE_SOURCE_ROUTE_OVERHEAD_UNKNOWN) {
    max -= sourceRouteOverhead;
  }

  return max;
}

void sl_zigbee_af_copy_int16u(uint8_t *data, uint16_t index, uint16_t x)
{
  data[index]   = (uint8_t) ( ((x)    ) & 0xFFu);
  data[index + 1] = (uint8_t) ( ((x) >> 8) & 0xFFu);
}

void sl_zigbee_af_copy_int24u(uint8_t *data, uint16_t index, uint32_t x)
{
  data[index]   = (uint8_t) ( ((x)    ) & 0xFFu);
  data[index + 1] = (uint8_t) ( ((x) >> 8) & 0xFFu);
  data[index + 2] = (uint8_t) ( ((x) >> 16) & 0xFFu);
}

void sl_zigbee_af_copy_int32u(uint8_t *data, uint16_t index, uint32_t x)
{
  data[index]   = (uint8_t) ( ((x)    ) & 0xFFu);
  data[index + 1] = (uint8_t) ( ((x) >> 8) & 0xFFu);
  data[index + 2] = (uint8_t) ( ((x) >> 16) & 0xFFu);
  data[index + 3] = (uint8_t) ( ((x) >> 24) & 0xFFu);
}

void sl_zigbee_af_copy_string(uint8_t *dest, uint8_t *src, uint8_t size)
{
  if ( src == NULL ) {
    dest[0] = 0u; // Zero out the length of string
  } else if (src[0] == 0xFFu) {
    dest[0] = src[0];
  } else {
    uint8_t length = sl_zigbee_af_string_length(src);
    if (size < length) {
      length = size;
    }
    memmove(dest + 1, src + 1, length);
    dest[0] = length;
  }
}

void sl_zigbee_af_copy_long_string(uint8_t *dest, uint8_t *src, uint16_t size)
{
  if ( src == NULL ) {
    dest[0] = dest[1] = 0u; // Zero out the length of string
  } else if ((src[0] == 0xFFu)
             && (src[1] == 0xFFu)) {
    dest[0] = 0xFFu;
    dest[1] = 0xFFu;
  } else {
    uint16_t length = sl_zigbee_af_long_string_length(src);
    if (size < length) {
      length = size;
    }
    memmove(dest + 2, src + 2, length);
    dest[0] = LOW_BYTE(length);
    dest[1] = HIGH_BYTE(length);
  }
}

#if (BIGENDIAN_CPU)
  #define EM_BIG_ENDIAN true
#else
  #define EM_BIG_ENDIAN false
#endif

// You can pass in val1 as NULL, which will assume that it is
// pointing to an array of all zeroes. This is used so that
// default value of NULL is treated as all zeroes.
int8_t sl_zigbee_af_compare_values(uint8_t* val1,
                                   uint8_t* val2,
                                   uint8_t len,
                                   bool signedNumber)
{
  uint8_t i, j, k;

  SL_ZIGBEE_TEST_ASSERT(len > 0);

  if (signedNumber) { // signed number comparison
    if (len <= 4) { // only number with 32-bits or less is supported
      int32_t accum1 = 0x0;
      int32_t accum2 = 0x0;
      int32_t all1s = -1;

      for (i = 0; i < len; i++) {
        j = (val1 == NULL
             ? 0
             : (EM_BIG_ENDIAN ? val1[i] : val1[(len - 1) - i])
             );
        accum1 |= j << (8 * (len - 1 - i));

        k = (EM_BIG_ENDIAN
             ? val2[i]
             : val2[(len - 1) - i]);
        accum2 |= k << (8 * (len - 1 - i));
      }

      // sign extending, no need for 32-bits numbers
      if (len < 4) {
        if ((accum1 & (1 << (8 * len - 1))) != 0) { // check sign
          accum1 |= all1s - ((1 << (len * 8)) - 1);
        }
        if ((accum2 & (1 << (8 * len - 1))) != 0) { // check sign
          accum2 |= all1s - ((1 << (len * 8)) - 1);
        }
      }

      if (accum1 > accum2) {
        return 1;
      } else if (accum1 < accum2) {
        return -1;
      } else {
        return 0;
      }
    } else { // not supported
      return 0;
    }
  } else { // regular unsigned number comparison
    for (i = 0; i < len; i++) {
      j = (val1 == NULL
           ? 0
           : (EM_BIG_ENDIAN ? val1[i] : val1[(len - 1) - i])
           );
      k = (EM_BIG_ENDIAN
           ? val2[i]
           : val2[(len - 1) - i]);

      if (j > k) {
        return 1;
      } else if (k > j) {
        return -1;
      } else {
        // MISRA requires ..else if.. to have terminating else.
      }
    }
    return 0;
  }
}

#if 0
// Moving to time-util.c
int8_t sl_zigbee_af_compare_dates(sl_zigbee_af_date_t* date1, sl_zigbee_af_date_t* date2)
{
  uint32_t val1 = sl_zigbee_af_encode_date(date1);
  uint32_t val2 = sl_zigbee_af_encode_date(date2);
  return (val1 == val2) ? 0 : ((val1 < val2) ? -1 : 1);
}
#endif

// returns the type that the attribute is, either SL_ZIGBEE_AF_DATA_TYPE_ANALOG,
// SL_ZIGBEE_AF_DATA_TYPE_DISCRETE, or SL_ZIGBEE_AF_DATA_TYPE_NONE. This is based on table
// 2.15 from the ZCL spec 075123r02
uint8_t sl_zigbee_af_get_attribute_analog_or_discrete_type(uint8_t dataType)
{
  uint8_t index = 0u;

  while ( sl_zigbee_af_analog_discrete_thresholds[index] < dataType ) {
    index += 2u;
  }
  return sl_zigbee_af_analog_discrete_thresholds[index + 1u];
}

// Zigbee spec says types between signed 8 bit and signed 64 bit
bool sl_zigbee_af_is_type_signed(sl_zigbee_af_attribute_type_t dataType)
{
  return (dataType >= ZCL_INT8S_ATTRIBUTE_TYPE
          && dataType <= ZCL_INT64S_ATTRIBUTE_TYPE);
}

bool sl_zigbee_af_is_this_my_eui64(sl_802154_long_addr_t eui64)
{
  sl_802154_long_addr_t myEui64;
  sl_zigbee_af_get_eui64(myEui64);
  return (0 == memcmp(eui64, myEui64, EUI64_SIZE)
          ? true
          : false);
}

uint8_t sl_zigbee_af_append_characters(uint8_t * zclString,
                                       uint8_t zclStringMaxLen,
                                       const uint8_t * appendingChars,
                                       uint8_t appendingCharsLen)
{
  uint8_t freeChars;
  uint8_t curLen;
  uint8_t charsToWrite;

  if ((zclString == NULL)
      || (zclStringMaxLen == 0u)
      || (appendingChars == NULL)
      || (appendingCharsLen == 0u)) {
    return 0;
  }

  curLen = sl_zigbee_af_string_length(zclString);

  if ((zclString[0] == 0xFFu)
      || (curLen >= zclStringMaxLen)) {
    return 0u;
  }

  freeChars = zclStringMaxLen - curLen;
  charsToWrite = (freeChars > appendingCharsLen) ? appendingCharsLen : freeChars;

  memcpy(&zclString[1u + curLen], // 1 is to account for zcl's length byte
         appendingChars,
         charsToWrite);
  zclString[0] = curLen + charsToWrite;
  return charsToWrite;
}

uint32_t sl_zigbee_af_get_buffer_crc(uint8_t *pbuffer, uint16_t length, uint32_t initialValue)
{
  uint16_t i;
  uint32_t crc32 = initialValue;
  for (i = 0u; i < length; i++) {
    crc32 = halCommonCrc32(pbuffer[i], crc32);
  }
  return crc32;
}

/*
   On each page, first channel maps to channel number zero and so on.
   Example:
   page    Band      Rage of 90 channels    Per page channel mapping
   28     863 MHz        0-26                    0-26
   29     863 MHz        27-34,62                0-8 (Here 7th channel maps to 34 and 8th to 62)
   30     863 MHz        35 - 61                 0-26
   31     915            0-26                    0-26

 */
sl_status_t sli_zigbee_af_validate_channel_pages(uint8_t page, uint8_t channel)
{
  switch (page) {
    case 0u:
      if (!((channel <= SL_ZIGBEE_MAX_802_15_4_CHANNEL_NUMBER)
            && ((SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER == 0)
                || (channel >= SL_ZIGBEE_MIN_802_15_4_CHANNEL_NUMBER)))) {
        return SL_STATUS_TRANSMIT_INVALID_CHANNEL;
      }
      break;
    case 28u:
    case 30u:
    case 31u:
      if (channel > SL_ZIGBEE_MAX_SUBGHZ_CHANNEL_NUMBER_ON_PAGES_28_30_31) {
        return SL_STATUS_TRANSMIT_INVALID_CHANNEL;
      }
      break;
    case 29u:
      if (channel > SL_ZIGBEE_MAX_SUBGHZ_CHANNEL_NUMBER_ON_PAGE_29) {
        return SL_STATUS_TRANSMIT_INVALID_CHANNEL;
      }
      break;
    default:
      return SL_STATUS_TRANSMIT_INVALID_CHANNEL;
      break;
  }
  return SL_STATUS_OK;
}

void slabAssert(const char * file, int line)
{
  (void)file;  // Unused parameter
  (void)line;  // Unused parameter
  // Wait forever until the watchdog fires
  while (true) {
  }
}

#define ENCODED_8BIT_CHANPG_PAGE_MASK           0xE0u   // top 3 bits
#define ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_0    0x00u   // 0b000xxxxx
#define ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_28   0x80u   // 0b100xxxxx
#define ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_29   0xA0u   // 0b101xxxxx
#define ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_30   0xC0u   // 0b110xxxxx
#define ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_31   0xE0u   // 0b111xxxxx

#define ENCODED_8BIT_CHANPG_CHANNEL_MASK        0x1Fu   // bottom 5 bits

uint8_t sl_zigbee_af_get_page_from_8bit_encoded_chan_pg(uint8_t chanPg)
{
  switch (chanPg & ENCODED_8BIT_CHANPG_PAGE_MASK) {
    case ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_0:
      return 0u;
    case ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_28:
      return 28u;
    case ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_29:
      return 29u;
    case ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_30:
      return 30u;
    case ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_31:
      return 31u;
    default:
      return 0xFFu;
  }
}

uint8_t sl_zigbee_af_get_channel_from_8bit_encoded_chan_pg(uint8_t chanPg)
{
  return chanPg & ENCODED_8BIT_CHANPG_CHANNEL_MASK;
}

uint8_t sl_zigbee_af_make_8bit_encoded_chan_pg(uint8_t page, uint8_t channel)
{
  if (sli_zigbee_af_validate_channel_pages(page, channel) != SL_STATUS_OK) {
    return 0xFFu;
  }

  switch (page) {
    case 28u:
      return channel | ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_28;
    case 29u:
      return channel | ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_29;
    case 30u:
      return channel | ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_30;
    case 31u:
      return channel | ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_31;
    default:
      // Strictly speaking, we only need case 0 here, but MISRA in its infinite
      // wisdom requires a default case. Since we have validated the arguments
      // already, and 0 is the only remaining case, we simply treat the default
      // as case 0 to make MISRA happy.
      return channel | ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_0;
  }
}

WEAK(sl_zigbee_af_difference_type_t sl_zigbee_af_get_diff_cb(sl_zigbee_af_difference_type_t value1,
                                                             sl_zigbee_af_difference_type_t value2,
                                                             sl_zigbee_af_attribute_type_t dataType))
{
  (void)value1;
  (void)value2;
  (void)dataType;
  return 0;
}

#ifdef SL_CATALOG_KERNEL_PRESENT
void sl_zigbee_af_acquire_lock()
{
  osStatus_t ret;
  ret = osMutexAcquire(sli_zigbee_af_mutex_id, osWaitForever);
  assert(ret == osOK);
}

void sl_zigbee_af_release_lock()
{
  osStatus_t ret;
  ret = osMutexRelease(sli_zigbee_af_mutex_id);
  assert(ret == osOK);
}
#else
void sl_zigbee_af_acquire_lock()
{
}
void sl_zigbee_af_release_lock()
{
}
#endif

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
// Called from sl_zigbee_af_init()
void initSuppressionTable(void)
{
  for (uint8_t i = 0; i < MAX_SUPPRESSION_ENTRIES; i++) {
    // It should suffice just to initialize the cluster id.
    SuppressionTableEntry *entry = &suppressionTable[i];
    entry->clusterId = 0xffff;
  }
}

bool checkSuppression(void)
{
  bool isLocal = sli_zigbee_af_current_command->clusterSpecific;
  uint8_t isClientCluster = sli_zigbee_af_current_command->direction;
  uint16_t clusterId = sli_zigbee_af_current_command->apsFrame->clusterId;
  uint8_t commandId = sli_zigbee_af_current_command->commandId;

  sl_zigbee_af_debug_println("incoming cmd: lcl %d cls %d dir %d cmd %d", isLocal, clusterId, isClientCluster, commandId);

  return sl_zigbee_af_get_suppress_cluster(clusterId, isClientCluster)
         || (sl_zigbee_af_get_suppress_command(clusterId, isClientCluster, commandId) && isLocal);
}

sl_zigbee_af_status_t sl_zigbee_af_set_suppress_cluster(uint16_t clusterId, bool isClientCluster)
{
  // The command works as a toggle, so we first search for an existing entry to unset.
  uint8_t freeEntry = 0xff;
  for (uint8_t i = 0; i < MAX_SUPPRESSION_ENTRIES; i++) {
    SuppressionTableEntry *entry = &suppressionTable[i];
    if (clusterId == entry->clusterId
        && isClientCluster == entry->isClientCluster
        && 0xff == entry->commandId) {
      entry->clusterId = 0xffff;
      return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    } else if (entry->clusterId == 0xffff && freeEntry == 0xff) {
      freeEntry = i;
    }
  }

  // No match, so we need to add a new entry.
  if (freeEntry != 0xff) {
    SuppressionTableEntry *entry = &suppressionTable[freeEntry];
    entry->clusterId = clusterId;
    entry->isClientCluster = isClientCluster;
    entry->commandId = 0xff;
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }
  return SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
}

bool sl_zigbee_af_get_suppress_cluster(uint16_t clusterId, bool isClientCluster)
{
  for (uint8_t i = 0; i < MAX_SUPPRESSION_ENTRIES; i++) {
    SuppressionTableEntry *entry = &suppressionTable[i];
    if (clusterId == entry->clusterId
        && isClientCluster == entry->isClientCluster
        && 0xff == entry->commandId) {
      sl_zigbee_af_debug_println("sl_zigbee_af_get_suppress_cluster match: isClient = %d, cluster = %04X", isClientCluster, clusterId);
      return true;
    }
  }
  return false;
}

sl_zigbee_af_status_t sl_zigbee_af_set_suppress_command(uint16_t clusterId, bool isClientCluster, uint8_t commandId)
{
  // The command works as a toggle, so we first search for an existing entry to unset.
  uint8_t freeEntry = 0xff;
  for (uint8_t i = 0; i < MAX_SUPPRESSION_ENTRIES; i++) {
    SuppressionTableEntry *entry = &suppressionTable[i];
    if (clusterId == entry->clusterId
        && isClientCluster == entry->isClientCluster
        && commandId == entry->commandId) {
      entry->clusterId = 0xffff;
      entry->commandId = 0xff;
      return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    } else if (entry->clusterId == 0xffff && freeEntry == 0xff) {
      freeEntry = i;
    }
  }

  // No match, so we need to add a new entry.
  if (freeEntry != 0xff) {
    SuppressionTableEntry *entry = &suppressionTable[freeEntry];
    entry->clusterId = clusterId;
    entry->isClientCluster = isClientCluster;
    entry->commandId = commandId;
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }
  return SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
}

bool sl_zigbee_af_get_suppress_command(uint16_t clusterId, bool isClientCluster, uint8_t commandId)
{
  sl_zigbee_af_debug_println("sl_zigbee_af_get_suppress_command: isClient = %d, cluster = %04X", isClientCluster, clusterId);

  for (uint8_t i = 0; i < MAX_SUPPRESSION_ENTRIES; i++) {
    SuppressionTableEntry *entry = &suppressionTable[i];
    if (clusterId == entry->clusterId
        && isClientCluster == entry->isClientCluster
        && commandId == entry->commandId) {
      return true;
    }
  }
  return false;
}

void sl_zigbee_af_print_suppression_table(void)
{
  sl_zigbee_af_app_println("");
  sl_zigbee_af_app_println("clstr side cmd");
  for (uint8_t i = 0; i < MAX_SUPPRESSION_ENTRIES; i++) {
    SuppressionTableEntry *entry = &suppressionTable[i];
    sl_zigbee_af_app_println("%04X   %d   %02X", entry->clusterId,
                             entry->isClientCluster,
                             entry->commandId);
  }
}
#endif // #ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
