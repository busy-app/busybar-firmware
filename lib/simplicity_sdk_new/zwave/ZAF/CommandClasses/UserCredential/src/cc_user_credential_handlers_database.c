/**
 * @file
 * @brief User Credential database related handlers.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2023 Silicon Laboratories Inc.
 */

#include "cc_user_credential_handlers_database.h"
#include <string.h>
#include "CC_UserCredential.h"
#include "cc_user_credential_config_api.h"
#include "cc_user_credential_io.h"
#include "cc_user_credential_io_config.h"
#include "cc_user_credential_operations.h"
#include "cc_user_credential_validation.h"

/****************************************************************************/
/*                             PUBLIC FUNCTIONS                             */
/****************************************************************************/

ZW_WEAK received_frame_status_t CC_UserCredential_UserSet_handler(
  cc_handler_input_t * input)
{
  ZW_USER_SET_1BYTE_FRAME * pFrameIn = &input->frame->ZW_UserSet1byteFrame;
  RECEIVE_OPTIONS_TYPE_EX * rxOptions = input->rx_options;
  received_frame_status_t status = RECEIVED_FRAME_STATUS_FAIL;
  u3c_db_operation_result operation_result = U3C_DB_OPERATION_RESULT_SUCCESS;

  // Extract User data from frame
  uint8_t name[U3C_BUFFER_SIZE_USER_NAME];
  u3c_user_t user = {
    .unique_identifier = (uint16_t)(pFrameIn->userUniqueIdentifier1 << 8 | pFrameIn->userUniqueIdentifier2),
    .type = pFrameIn->userType,
    .active = pFrameIn->properties2 & USER_SET_PROPERTIES2_USER_ACTIVE_STATE_BIT_MASK,
    .credential_rule = pFrameIn->credentialRule,
    .expiring_timeout_minutes = (uint16_t)(pFrameIn->expiringTimeoutMinutes1 << 8 | pFrameIn->expiringTimeoutMinutes2),
    .name_encoding = pFrameIn->properties3 & USER_SET_PROPERTIES3_USER_NAME_ENCODING_MASK,
    .name_length = pFrameIn->userNameLength,
    .modifier_type = MODIFIER_TYPE_Z_WAVE,
    .modifier_node_id = rxOptions->sourceNode.nodeId
  };
  uint8_t * pName = &pFrameIn->userName1;
  uint8_t operation = pFrameIn->properties1 & USER_SET_PROPERTIES1_OPERATION_TYPE_MASK;
  bool is_name_truncated = false;

  // CC:0083.01.05.11.014: Ignore frames where the User Unique Identifier is greater than allowed
  if (user.unique_identifier > cc_user_credential_get_max_user_unique_idenfitiers()) {
    return RECEIVED_FRAME_STATUS_FAIL;
  }

  if (operation != USER_SET_OPERATION_TYPE_DELETE) {
    // Ignore frames with unsupported values
    if (
      // CC:0083.01.05.11.015
      user.unique_identifier == 0
      // CC:0083.01.05.11.030
      || !cc_user_credential_is_user_type_supported(user.type)
      // CC:0083.01.05.11.039
      || !cc_user_credential_is_credential_rule_supported(user.credential_rule)
      // CC:0083.01.05.11.045
      || (user.name_encoding > USER_NAME_ENCODING_UNICODE_UTF_16)
      ) {
      return RECEIVED_FRAME_STATUS_FAIL;
    }

    // CC:0083.01.05.11.042: An expiring user must have a non-zero timeout
    if (user.type == USER_TYPE_EXPIRING && user.expiring_timeout_minutes == 0) {
      // Send back data of existing user
      bool user_found = CC_UserCredential_get_user(
        user.unique_identifier, &user, name)
                        == U3C_DB_OPERATION_RESULT_SUCCESS;
      if (!user_found) {
        // Or send a blank user if not found
        memset(&user, 0, sizeof(u3c_user_t));
        user.credential_rule = CREDENTIAL_RULE_SINGLE;
      }
      CC_UserCredential_UserReport_tx(
        USER_REP_TYPE_Z_EXP_MIN_INVALID, &user, name,
        CC_UserCredential_get_next_user(user.unique_identifier), rxOptions);
      return RECEIVED_FRAME_STATUS_FAIL;
    }

    // CC:0083.01.05.11.053: Set non-Expiring User's Expiring Timeout to 0
    if (user.type != USER_TYPE_EXPIRING && user.expiring_timeout_minutes != 0) {
      user.expiring_timeout_minutes = 0;
    }

    // Truncate User Name if it is longer than allowed
    uint8_t user_name_max_length = cc_user_credential_get_max_length_of_user_name();
    if (user.name_length > user_name_max_length) {
      user.name_length = user_name_max_length;
      is_name_truncated = true;
    }

    // If no name is provided, set it to the default value
    if (user.name_length == 0) {
      CC_UserCredential_set_default_name(name, &user);
      pName = name;
    }

    // Do not assign a schedule to a Programming User
  }

  switch (operation) {
    case USER_SET_OPERATION_TYPE_ADD: {
      operation_result = CC_UserCredential_add_user_and_report(&user, pName, rxOptions);
      status = (((operation_result == U3C_DB_OPERATION_RESULT_SUCCESS)
                 || (operation_result == U3C_DB_OPERATION_RESULT_FAIL_IDENTICAL))
                && (is_name_truncated == false))
               ? RECEIVED_FRAME_STATUS_SUCCESS : RECEIVED_FRAME_STATUS_FAIL;
      break;
    }

    case USER_SET_OPERATION_TYPE_MODIFY: {
      operation_result = CC_UserCredential_modify_user_and_report(&user, pName, rxOptions);
      status = (((operation_result == U3C_DB_OPERATION_RESULT_SUCCESS)
                 || (operation_result == U3C_DB_OPERATION_RESULT_FAIL_IDENTICAL))
                && (is_name_truncated == false))
               ? RECEIVED_FRAME_STATUS_SUCCESS : RECEIVED_FRAME_STATUS_FAIL;
      break;
    }

    case USER_SET_OPERATION_TYPE_DELETE: {
      if (user.unique_identifier == 0) {
        // Delete all Users
        uint16_t user_uid = CC_UserCredential_get_next_user(0);

        while (user_uid) {
          /**
           * Fetch the next User's details before deleting the current one
           * (otherwise CC_UserCredential_get_next_user will not find the next
           * one)
           */
          uint16_t next_user_uid = CC_UserCredential_get_next_user(user_uid);
          CC_UserCredential_delete_all_credentials_of_type(user_uid, CREDENTIAL_TYPE_NONE);
          CC_UserCredential_delete_user(user_uid);
          user_uid = next_user_uid;
        }

        // Mirror request in report, resetting non-identifying parameters
        user.modifier_type = MODIFIER_TYPE_DNE;
        user.modifier_node_id = 0;
        user.active = false;
        user.expiring_timeout_minutes = 0;
        CC_UserCredential_UserReport_tx(
          USER_REPORT_DELETED, &user, pName, 0, rxOptions);
        status = RECEIVED_FRAME_STATUS_SUCCESS;
      } else {
        // Delete a single User and report
        operation_result = CC_UserCredential_delete_user_and_report(user.unique_identifier, rxOptions);
        status = ((operation_result == U3C_DB_OPERATION_RESULT_SUCCESS)
                  || (operation_result == U3C_DB_OPERATION_RESULT_FAIL_DNE))
                 ? RECEIVED_FRAME_STATUS_SUCCESS : RECEIVED_FRAME_STATUS_FAIL;
      }
      break;
    }

    default: {
      status = RECEIVED_FRAME_STATUS_NO_SUPPORT;
    }
  }

  return status;
}

ZW_WEAK received_frame_status_t CC_UserCredential_UserGet_handler(
  cc_handler_input_t * input)
{
  const ZW_USER_GET_FRAME * const pFrameIn = &input->frame->ZW_UserGetFrame;
  uint8_t name[U3C_BUFFER_SIZE_USER_NAME] = { 0 };
  u3c_user_t user = { 0 };

  uint16_t uid = (uint16_t)(pFrameIn->userUniqueIdentifier1 << 8 | pFrameIn->userUniqueIdentifier2);
  if (uid == 0) {
    // Return the first available User's data
    uid = CC_UserCredential_get_next_user(0);
  }
  uint16_t next_user_uid = CC_UserCredential_get_next_user(uid);

  u3c_db_operation_result result = CC_UserCredential_get_user(uid, &user, name);

  if (result != U3C_DB_OPERATION_RESULT_SUCCESS) {
    // The User doesn't exist, return blank data, but make sure all fields are valid
    memset(&user, 0, sizeof(user));
    user.unique_identifier = uid;
    user.credential_rule = CREDENTIAL_RULE_SINGLE;
  }

  CC_UserCredential_UserReport_tx(
    USER_REP_TYPE_RESPONSE_TO_GET, &user, name, next_user_uid, input->rx_options);

  return (result == U3C_DB_OPERATION_RESULT_SUCCESS)
         ? RECEIVED_FRAME_STATUS_SUCCESS
         : RECEIVED_FRAME_STATUS_FAIL;
}

ZW_WEAK received_frame_status_t CC_UserCredential_CredentialSet_handler(
  u3c_operation_type_t operation_type,
  u3c_credential_t * p_credential,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  received_frame_status_t status = RECEIVED_FRAME_STATUS_FAIL;
  u3c_db_operation_result operation_result = U3C_DB_OPERATION_RESULT_ERROR;

  if (operation_type != U3C_OPERATION_TYPE_DELETE) {
    if (!validate_new_credential_metadata(&p_credential->metadata)
        // CC:0083.01.0A.11.015,
        || (p_credential->metadata.length
            < cc_user_credential_get_min_length_of_data(p_credential->metadata.type))
        // CC:0083.01.0A.11.014
        || (p_credential->metadata.length
            > cc_user_credential_get_max_length_of_data(p_credential->metadata.type))
        || !validate_new_credential_data(p_credential, p_rx_options)) {
      return RECEIVED_FRAME_STATUS_FAIL;
    }
  } else {
    // CC:0083.01.0A.11.002: an unsupported Credential Type must be ignored
    if ((p_credential->metadata.type != CREDENTIAL_TYPE_NONE)
        && !cc_user_credential_is_credential_type_supported(p_credential->metadata.type)) {
      return RECEIVED_FRAME_STATUS_FAIL;
    }
  }

  switch (operation_type) {
    case U3C_OPERATION_TYPE_ADD: {
      operation_result = CC_UserCredential_add_credential_and_report(p_credential, p_rx_options);
      status = ((operation_result == U3C_DB_OPERATION_RESULT_SUCCESS)
                || (operation_result == U3C_DB_OPERATION_RESULT_FAIL_IDENTICAL))
               ? RECEIVED_FRAME_STATUS_SUCCESS : RECEIVED_FRAME_STATUS_FAIL;
      break;
    }

    case U3C_OPERATION_TYPE_MODIFY: {
      operation_result = CC_UserCredential_modify_credential_and_report(p_credential, p_rx_options);
      status = ((operation_result == U3C_DB_OPERATION_RESULT_SUCCESS)
                || (operation_result == U3C_DB_OPERATION_RESULT_FAIL_IDENTICAL))
               ? RECEIVED_FRAME_STATUS_SUCCESS : RECEIVED_FRAME_STATUS_FAIL;
      break;
    }

    case U3C_OPERATION_TYPE_DELETE: {
      if (p_credential->metadata.slot == 0) {
        // Bulk delete credentials

        // Remember whether any credentials were deleted
        bool deleted_any_credentials = false;

        uint16_t user_uid = p_credential->metadata.uuid;
        if (user_uid == 0) {
          user_uid = CC_UserCredential_get_next_user(0);
        }

        // Iterate through each user
        while (user_uid) {
          // Delete a user's every credential (of a certain type, if specified)
          deleted_any_credentials |= CC_UserCredential_delete_all_credentials_of_type(user_uid, p_credential->metadata.type);
          user_uid = CC_UserCredential_get_next_user(user_uid);

          /**
           * If the UUID was specified in the request, don't process any
           * additional users.
           */
          if (p_credential->metadata.uuid != 0) {
            break;
          }
        }

        p_credential->metadata.modifier_type = MODIFIER_TYPE_DNE;
        p_credential->metadata.modifier_node_id = 0;

        u3c_credential_report_type_t report_type =
          deleted_any_credentials
          ? CREDENTIAL_REP_TYPE_DELETED
          : CREDENTIAL_REP_TYPE_UNCHANGED;

        // Echo data from request back to sender
        CC_UserCredential_CredentialReport_tx(
          report_type, p_credential, CREDENTIAL_TYPE_NONE, 0,
          p_rx_options);

        status = RECEIVED_FRAME_STATUS_SUCCESS;
      } else if (p_credential->metadata.type != 0) {
        // Delete a single Credential and report
        operation_result = CC_UserCredential_delete_credential_and_report(
          p_credential->metadata.uuid, p_credential->metadata.type,
          p_credential->metadata.slot, p_rx_options);
        status = ((operation_result == U3C_DB_OPERATION_RESULT_SUCCESS)
                  || (operation_result == U3C_DB_OPERATION_RESULT_FAIL_DNE))
                 ? RECEIVED_FRAME_STATUS_SUCCESS : RECEIVED_FRAME_STATUS_FAIL;
      }
      // Type == 0 && Slot != 0 is not a valid request
    }
    break;

    default: {
      status = RECEIVED_FRAME_STATUS_NO_SUPPORT;
    }
  }

  return status;
}

ZW_WEAK bool CC_UserCredential_CredentialGet_handler(
  cc_handler_input_t * input,
  u3c_credential_metadata_t * p_metadata,
  u3c_credential_report_tx_callback_t callback
  )
{
  /**
   * CC:0083.01.0B.11.006 Other combinations of zero and non-zero User Unique Identifier,
   * Credential Type, and Credential Slot are considered invalid and MUST be ignored.
   * CC:0083.01.0C.11.002 - Table
   */
  if (p_metadata->type == CREDENTIAL_TYPE_NONE && p_metadata->slot != 0) {
    return false;
  }

  uint8_t data[U3C_BUFFER_SIZE_CREDENTIAL_DATA] = { 0 };
  u3c_credential_t credential = {
    .metadata = {
      .length = 0,
      .modifier_node_id = 0,
      .modifier_type = MODIFIER_TYPE_DNE,
      .slot = p_metadata->slot,
      .type = p_metadata->type,
      .uuid = p_metadata->uuid
    },
    .data = data
  };
  u3c_credential_type next_credential_type = CREDENTIAL_TYPE_NONE;
  uint16_t next_credential_slot = 0;
  bool credential_found = false;

  if (p_metadata->slot == 0) {
    // Fetch the first available Credential's ID
    CC_UserCredential_get_next_credential(
      p_metadata->uuid, p_metadata->type, 0,
      &p_metadata->type, &p_metadata->slot);
  }

  u3c_db_operation_result get_result = CC_UserCredential_get_credential(
    p_metadata->uuid, p_metadata->type, p_metadata->slot,
    &credential.metadata, credential.data);

  if (get_result == U3C_DB_OPERATION_RESULT_SUCCESS) {
    credential_found = true;
  }
  /**
   * CC:0083.01.0C.11.016, CC:0083.01.0C.11.017
   * Query the next credential based on the incoming request if the
   * requested credential does not exist.
   * Otherwise, prefer the ID returned by the database. This will be
   * different from the request if that was for the first available
   * credential (slot number == 0).
   */
  CC_UserCredential_get_next_credential(
    p_metadata->uuid,
    credential_found ? credential.metadata.type : p_metadata->type,
    credential_found ? credential.metadata.slot : p_metadata->slot,
    &next_credential_type, &next_credential_slot);

  callback(
    CREDENTIAL_REP_TYPE_RESPONSE_TO_GET, &credential, next_credential_type,
    next_credential_slot, input->rx_options);

  return credential_found;
}
