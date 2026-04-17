/**
 * @file
 * @brief User Credential Unsolicited Transmission Functions.
 * @details Contains the implementation for transmitting different unsolicited
 * User Credential Command Class reports.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2023 Silicon Laboratories Inc.
 */

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/

#include "cc_user_credential_tx.h"
#include "assert.h"
#include "cc_user_credential_config_api.h"
#include "CC_Notification.h"
#include "ZAF_Common_interface.h"
#include "zaf_transport_tx.h"
#include "ZAF_TSE.h"
#include "ZW_TransportSecProtocol.h"
#include "association_plus_base.h"
#include <string.h>

/****************************************************************************/
/*                             STATIC VARIABLES                             */
/****************************************************************************/

/// Persistent data for reports
static uint8_t report_out_frame[TX_DATA_MAX_DATA_SIZE] = { 0 };
static uint8_t report_out_size = 0;

/****************************************************************************/
/*                            PRIVATE FUNCTIONS                             */
/****************************************************************************/

/**
 * Callback function for ZAF TSE to send User or Credential Reports to multiple
 * destinations
 */
static void send_report_tse(
  zaf_tx_options_t * p_tx_options,
  __attribute__((unused)) const void * const p_data
  )
{
  zaf_transport_tx(
    report_out_frame,
    report_out_size,
    ZAF_TSE_TXCallback,
    p_tx_options
    );
}

/**
 * Sends a User or Credential report.
 *
 * @param[in] p_rx_options       Pointer to the properties of the incoming frame
 * @param[in] notify_lifeline    true if the nodes in the  Lifeline association
 *                               group should receive the report
 * @param[in] supervision_status What status the encapsulating Supervision frame
 *                               should have (if using Supervision)
 */
static void send_report(
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options,
  const bool notify_lifeline
  )
{
  zaf_tx_options_t tx_options;
  zaf_transport_rx_to_tx_options(p_rx_options, &tx_options);
  zaf_transport_tx(report_out_frame, report_out_size, NULL, &tx_options);

  if (notify_lifeline) {
    /**
     * Note: TSE will send the report to all nodes in the Lifeline association
     * group, except the initiator.
     */
    ZAF_TSE_Trigger((void *)&send_report_tse, p_rx_options, false);
  }
}

/****************************************************************************/
/*                            EXPORTED FUNCTIONS                            */
/****************************************************************************/

bool is_rx_frame_initiated_locally(const RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  return p_rx_options->destNode.nodeId == 0 || p_rx_options->sourceNode.nodeId == 0;
}

void fill_rx_frame_with_local(RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  /**
   * If an operation was initiated locally, there is no incoming
   * frame to parse the receive options from.
   * Use the first node in the Lifeline association group as the source
   * and the node's own ID as the destination.
   */

  destination_info_t * pNodeList = NULL;
  uint8_t listLength = 0;
  NODE_LIST_STATUS status = handleAssociationGetnodeList(LIFELINE_GROUP_ID, 0, &pNodeList, &listLength);
  if (status == NODE_LIST_STATUS_SUCCESS && pNodeList != NULL) {
    p_rx_options->sourceNode.nodeId = pNodeList->node.nodeId;
  } // Otherwise, it stays at 0
  p_rx_options->destNode.nodeId = ZAF_GetNodeID();
  p_rx_options->securityKey = GetHighestSecureLevel(ZAF_GetSecurityKeys());
}

void CC_UserCredential_UserReport_tx(
  const u3c_user_report_type_t report_type,
  const u3c_user_t * const p_user,
  const uint8_t * const p_name,
  const uint16_t next_uuid,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  )
{
  ZW_USER_REPORT_1BYTE_FRAME * pReport = (ZW_USER_REPORT_1BYTE_FRAME *)&report_out_frame;

  pReport->cmdClass = COMMAND_CLASS_USER_CREDENTIAL;
  pReport->cmd = USER_REPORT;
  pReport->userReportType = (uint8_t)report_type;
  pReport->nextUserUniqueIdentifier1 = next_uuid >> 8;
  pReport->nextUserUniqueIdentifier2 = next_uuid & 0xFF;
  pReport->userUniqueIdentifier1 = p_user->unique_identifier >> 8;
  pReport->userUniqueIdentifier2 = p_user->unique_identifier & 0xFF;
  pReport->userModifierType = (uint8_t)p_user->modifier_type;
  pReport->userModifierNodeId1 = p_user->modifier_node_id >> 8;
  pReport->userModifierNodeId2 = p_user->modifier_node_id & 0xFF;
  pReport->userType = (uint8_t)p_user->type;
  pReport->properties1 = (uint8_t)p_user->active & USER_REPORT_PROPERTIES1_USER_ACTIVE_STATE_BIT_MASK;
  pReport->credentialRule = (uint8_t)p_user->credential_rule;
  pReport->expiringTimeoutMinutes1 = p_user->expiring_timeout_minutes >> 8;
  pReport->expiringTimeoutMinutes2 = p_user->expiring_timeout_minutes & 0xFF;
  pReport->properties2 = p_user->name_encoding & USER_REPORT_PROPERTIES2_USER_NAME_ENCODING_MASK;
  pReport->userNameLength = p_user->name_length;
  // Copy User Name and increase length of outgoing frame by it
  memcpy(&pReport->userName1, p_name, p_user->name_length);
  report_out_size = sizeof(ZW_USER_REPORT_1BYTE_FRAME) - 1 + p_user->name_length;

  /**
   * Determine whether nodes in the the Lifeline association group must be
   * notified.
   * This only applies to successful database modifications.
   */
  bool notify_lifeline =
    (report_type == USER_REP_TYPE_ADDED)
    || (report_type == USER_REP_TYPE_MODIFIED)
    || (report_type == USER_REP_TYPE_DELETED);

  send_report(p_rx_options, notify_lifeline);
}

void CC_UserCredential_CredentialReport_tx(
  const u3c_credential_report_type_t report_type,
  const u3c_credential_t * const p_credential,
  const u3c_credential_type next_credential_type,
  const uint16_t next_credential_slot,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  )
{
  uint8_t * p_frame_start = report_out_frame;
  uint8_t * p_frame = p_frame_start;

  /**
   * CC:0083.01.0C.11.021: If the credential can be read back (Maximum
   * Credential Hash Length 0 in the Credential Capabilities Report Command),
   * the CRB flag MUST be set to 1.
   */
  bool crb =
    (p_credential->metadata.type != CREDENTIAL_TYPE_NONE)
    && (cc_user_credential_get_max_hash_length(p_credential->metadata.type) == 0);

  *p_frame++ = COMMAND_CLASS_USER_CREDENTIAL;
  *p_frame++ = CREDENTIAL_REPORT;
  *p_frame++ = (uint8_t)report_type;
  *p_frame++ = (uint8_t)(p_credential->metadata.uuid >> 8); // MSB
  *p_frame++ = (uint8_t)p_credential->metadata.uuid; // LSB
  *p_frame++ = (uint8_t)p_credential->metadata.type;
  *p_frame++ = (uint8_t)(p_credential->metadata.slot >> 8); // MSB
  *p_frame++ = (uint8_t)p_credential->metadata.slot; // LSB
  *p_frame++ = crb ? CREDENTIAL_REPORT_PROPERTIES1_CRB_BIT_MASK : 0; // CRB
  *p_frame++ = p_credential->metadata.length;
  memcpy(p_frame, p_credential->data, p_credential->metadata.length);
  p_frame += p_credential->metadata.length;
  *p_frame++ = (uint8_t)p_credential->metadata.modifier_type;
  *p_frame++ = (uint8_t)(p_credential->metadata.modifier_node_id >> 8); // MSB
  *p_frame++ = (uint8_t)p_credential->metadata.modifier_node_id; // LSB
  *p_frame++ = (uint8_t)next_credential_type;
  *p_frame++ = (uint8_t)(next_credential_slot >> 8); // MSB
  *p_frame++ = (uint8_t)next_credential_slot; // LSB
  report_out_size = (uint8_t)(p_frame - p_frame_start);

  /**
   * Determine whether nodes in the the Lifeline association group must be
   * notified.
   * This only applies to successful database modifications.
   */
  bool notify_lifeline =
    (report_type == CREDENTIAL_REP_TYPE_ADDED)
    || (report_type == CREDENTIAL_REP_TYPE_MODIFIED)
    || (report_type == CREDENTIAL_REP_TYPE_DELETED);

  send_report(p_rx_options, notify_lifeline);
}

void CC_UserCredential_CredentialLearnStatusReport_tx(
  const u3c_credential_learn_status_t status,
  const u3c_credential_identifier_t * const p_target,
  const uint8_t steps_remaining,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  const bool is_status_valid = status <= CL_STATUS_STEP_RETRY
                               || (status >= CL_STATUS_INVALID_ADD_OPERATION_TYPE);
  if (!is_status_valid || !p_target || !p_rx_options) {
    assert(false);
    return;
  }

  if (is_rx_frame_initiated_locally(p_rx_options)) {
    fill_rx_frame_with_local(p_rx_options);
  }

  ZW_APPLICATION_TX_BUFFER tx_buffer;
  ZW_CREDENTIAL_LEARN_REPORT_FRAME * p_frame = &tx_buffer.ZW_CredentialLearnReportFrame;

  p_frame->cmdClass = COMMAND_CLASS_USER_CREDENTIAL;
  p_frame->cmd = CREDENTIAL_LEARN_REPORT;
  p_frame->credentialLearnStatus = (uint8_t)status;
  p_frame->userUniqueIdentifier1 = p_target->uuid >> 8;
  p_frame->userUniqueIdentifier2 = p_target->uuid & 0xFF;
  p_frame->credentialType = (uint8_t)p_target->type;
  p_frame->credentialSlot1 = p_target->slot >> 8;
  p_frame->credentialSlot2 = p_target->slot & 0xFF;
  p_frame->credentialLearnStepsRemaining = steps_remaining;

  zaf_tx_options_t tx_options;
  zaf_transport_rx_to_tx_options(p_rx_options, &tx_options);
  zaf_transport_tx((uint8_t *)&tx_buffer, sizeof(ZW_CREDENTIAL_LEARN_REPORT_FRAME),
                   NULL, &tx_options);
}

void CC_UserCredential_AdminCodeReport_tx(
  const u3c_admin_code_metadata_t * const data,
  RECEIVE_OPTIONS_TYPE_EX * rx_options)
{
  ZW_ADMIN_PIN_CODE_REPORT_1BYTE_FRAME * tx_frame =
    (ZW_ADMIN_PIN_CODE_REPORT_1BYTE_FRAME *)&report_out_frame;

  // Populate frame with data from metadata structure
  tx_frame->cmdClass = COMMAND_CLASS_USER_CREDENTIAL;
  tx_frame->cmd = ADMIN_PIN_CODE_REPORT;
  tx_frame->properties1 = (uint8_t)(((data->result) << 4) & 0xF0) | (data->code_length & 0x0F); // Code length has to be less than 10 anyway
  if (data->code_length) {
    memcpy((void*)&tx_frame->adminCode1, data->code_data, data->code_length);
  } else {
    memset((void*)&tx_frame->adminCode1, 0x00, AC_MAX_LENGTH);
  }
  report_out_size =
    sizeof(ZW_ADMIN_PIN_CODE_REPORT_1BYTE_FRAME) - 1 + data->code_length;

  // CC:0083.01.1A.11.001: Notify the Lifeline on a succesful modification
  bool notify_lifeline = (data->result == ADMIN_CODE_OPERATION_RESULT_MODIFIED);
  send_report(rx_options, notify_lifeline);
}

void CC_UserCredential_AllUsersChecksumReport_tx(
  uint16_t checksum,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  )
{
  ZW_APPLICATION_TX_BUFFER buffer;
  uint8_t * p_frame_start = (uint8_t *)&buffer;
  uint8_t * p_frame = p_frame_start;
  *p_frame++ = COMMAND_CLASS_USER_CREDENTIAL;
  *p_frame++ = ALL_USERS_CHECKSUM_REPORT;
  *p_frame++ = (uint8_t)(checksum >> 8);
  *p_frame++ = (uint8_t)checksum;

  zaf_tx_options_t tx_options;
  zaf_transport_rx_to_tx_options(p_rx_options, &tx_options);
  zaf_transport_tx(p_frame_start, (uint8_t)(p_frame - p_frame_start), NULL, &tx_options);
  return;
}

void CC_UserCredential_UserChecksumReport_tx(
  uint16_t uuid,
  uint16_t checksum,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  )
{
  ZW_APPLICATION_TX_BUFFER buffer;
  uint8_t * p_frame_start = (uint8_t *)&buffer;
  uint8_t * p_frame = p_frame_start;
  *p_frame++ = COMMAND_CLASS_USER_CREDENTIAL;
  *p_frame++ = USER_CHECKSUM_REPORT;
  *p_frame++ = (uint8_t)(uuid >> 8);
  *p_frame++ = (uint8_t)uuid;
  *p_frame++ = (uint8_t)(checksum >> 8);
  *p_frame++ = (uint8_t)checksum;

  zaf_tx_options_t tx_options;
  zaf_transport_rx_to_tx_options(p_rx_options, &tx_options);
  zaf_transport_tx(p_frame_start, (uint8_t)(p_frame - p_frame_start), NULL, &tx_options);
  return;
}

void CC_UserCredential_CredentialChecksumReport_tx(
  u3c_credential_type type,
  uint16_t checksum,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  ZW_APPLICATION_TX_BUFFER buffer;
  uint8_t * p_frame_start = (uint8_t *)&buffer;
  uint8_t * p_frame = p_frame_start;
  *p_frame++ = COMMAND_CLASS_USER_CREDENTIAL;
  *p_frame++ = CREDENTIAL_CHECKSUM_REPORT;
  *p_frame++ = (uint8_t)type;
  *p_frame++ = (uint8_t)(checksum >> 8);
  *p_frame++ = (uint8_t)checksum;

  zaf_tx_options_t tx_options;
  zaf_transport_rx_to_tx_options(p_rx_options, &tx_options);
  zaf_transport_tx(p_frame_start, (uint8_t)(p_frame - p_frame_start), NULL, &tx_options);
  return;
}

JOB_STATUS CC_UserCredential_UsageNotification_tx(
  const uint8_t notification_event,
  const uint16_t uuid,
  const uint8_t credential_count,
  const u3c_credential_metadata_t * const p_credential_metadata
  )
{
  uint8_t notification_event_parameters[CREDENTIAL_USAGE_DATA_MAX_SIZE] = {
    uuid >> 8,
    uuid & 0xFF,
    credential_count
  };
  uint8_t count = credential_count;

  if (credential_count > CREDENTIAL_USAGE_MAX_ITEMS) {
    assert(false);
    count = CREDENTIAL_USAGE_MAX_ITEMS;
  }

  uint8_t * p_event_parameters = notification_event_parameters + 3;
  for (uint8_t i = 0; i < count; ++i) {
    *p_event_parameters++ = (uint8_t)p_credential_metadata[i].type;
    *p_event_parameters++ = p_credential_metadata[i].slot >> 8;
    *p_event_parameters++ = p_credential_metadata[i].slot & 0xFF;
  }

  return CC_Notification_TriggerAndTransmit(
    0, notification_event, notification_event_parameters,
    (uint8_t)(p_event_parameters - notification_event_parameters), NULL, false);
}

void CC_UserCredential_AssociationReport_tx(
  u3c_credential_metadata_t const * const p_source_metadata,
  u3c_credential_metadata_t const * const p_destination_metadata,
  u3c_user_credential_association_report_status_t const status,
  RECEIVE_OPTIONS_TYPE_EX * const p_rx_options
  )
{
  if ((p_source_metadata == NULL) || (p_destination_metadata == NULL)) {
    assert(false);
  }

  ZW_USER_CREDENTIAL_ASSOCIATION_REPORT_FRAME * p_frame =
    (ZW_USER_CREDENTIAL_ASSOCIATION_REPORT_FRAME *)report_out_frame;

  p_frame->cmdClass                         = COMMAND_CLASS_USER_CREDENTIAL;
  p_frame->cmd                              = USER_CREDENTIAL_ASSOCIATION_REPORT;
  p_frame->credentialType                   = (uint8_t)p_source_metadata->type;
  p_frame->credentialSlot1                  = (uint8_t)(p_source_metadata->slot >> 8); // MSB
  p_frame->credentialSlot2                  = (uint8_t)p_source_metadata->slot; // LSB
  p_frame->destinationUserUniqueIdentifier1 = (uint8_t)(p_destination_metadata->uuid >> 8); // MSB
  p_frame->destinationUserUniqueIdentifier2 = (uint8_t)p_destination_metadata->uuid; // LSB
  p_frame->userCredentialAssociationStatus  = (uint8_t)status;

  report_out_size = sizeof(ZW_USER_CREDENTIAL_ASSOCIATION_REPORT_FRAME);

  bool notify_lifeline = (status == U3C_UCAR_STATUS_SUCCESS);
  send_report(p_rx_options, notify_lifeline);
}
