/**
 * @file
 * @brief Handler for Command Class User Credential.
 * @copyright 2023 Silicon Laboratories Inc.
 */

#include <string.h>
#include "cc_user_credential_operations.h"
#include "cc_user_credential_io_config.h"
#include "cc_user_credential_validation.h"
#include "cc_user_credential_tx.h"
#include "zaf_transport_tx.h"
#include "assert.h"

/****************************************************************************/
/*                            PRIVATE FUNCTIONS                             */
/****************************************************************************/

/**
 * Checks whether the user associated with an incoming credential exists and
 * sends a report to the initiating node if it does not.
 *
 * @param[in] p_credential Pointer to the incoming credential
 * @param[in] p_rx_options Pointer to the details of the received frame
 * @return The result of getting the associated user from the database
 */
static u3c_db_operation_result check_and_report_nonexistent_user(
  u3c_credential_t * p_credential, RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  if (!p_credential || !p_rx_options) {
    assert(false);
    return U3C_DB_OPERATION_RESULT_ERROR;
  }

  u3c_db_operation_result get_user_result =
    CC_UserCredential_get_user(p_credential->metadata.uuid, NULL, NULL);
  bool user_exists = (get_user_result == U3C_DB_OPERATION_RESULT_SUCCESS);

  /**
   * Send a report if the specified user does not exist and the request was
   * initiated by a remote node.
   */
  if (!user_exists && !is_rx_frame_initiated_locally(p_rx_options)) {
    u3c_credential_type next_credential_type = CREDENTIAL_TYPE_NONE;
    uint16_t next_credential_slot = 0;

    CC_UserCredential_get_next_credential(
      p_credential->metadata.uuid, p_credential->metadata.type,
      p_credential->metadata.slot,
      &next_credential_type, &next_credential_slot);

    p_credential->metadata.modifier_type = MODIFIER_TYPE_DNE;

    CC_UserCredential_CredentialReport_tx(
      CREDENTIAL_REP_TYPE_WRONG_UUID, p_credential, next_credential_type,
      next_credential_slot, p_rx_options);
  }

  return get_user_result;
}

/****************************************************************************/
/*                             PUBLIC FUNCTIONS                             */
/****************************************************************************/

ZW_WEAK bool CC_UserCredential_delete_all_credentials_of_type(
  uint16_t uuid, u3c_credential_type filter_type)
{
  // Remember whether any credentials were deleted
  bool deleted_any_credentials = false;

  // The Credential to be deleted in the current iteration
  u3c_credential_type target_type = filter_type;
  uint16_t target_slot = 0;

  // The Credential after the current target
  u3c_credential_type next_type = filter_type;
  uint16_t next_slot = 0;

  // Find first Credential
  bool next_exists = CC_UserCredential_get_next_credential(
    uuid, filter_type, 0, &target_type, &target_slot);

  // Loop through matching Credentials
  while (
    next_exists
    && ((filter_type == CREDENTIAL_TYPE_NONE) || (next_type == filter_type))
    ) {
    /**
     * Fetch the next Credential's details before deleting the current one
     * (otherwise CC_UserCredential_get_next_credential will not find the next
     * one)
     */
    next_exists = CC_UserCredential_get_next_credential(
      uuid, target_type, target_slot, &next_type, &next_slot);

    // Delete target Credential
    if (CC_UserCredential_delete_credential(target_type, target_slot)
        == U3C_DB_OPERATION_RESULT_SUCCESS) {
      deleted_any_credentials = true;
    }

    // Target the next Credential
    target_type = next_type;
    target_slot = next_slot;
  }

  return deleted_any_credentials;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_add_user_and_report(
  u3c_user_t * p_user, uint8_t * p_name, RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  if (!p_user || !p_name || !p_rx_options) {
    assert(false);
    return U3C_DB_OPERATION_RESULT_ERROR;
  }

  if (!validate_user_name_encoding(p_name, p_user->name_length, p_user->name_encoding)) {
    return U3C_DB_OPERATION_RESULT_ERROR;
  }

  u3c_db_operation_result operation_result = CC_UserCredential_add_user(p_user, p_name);

  if (is_rx_frame_initiated_locally(p_rx_options)) {
    // Locally initiated request, no report is required on error.
    if (operation_result != U3C_DB_OPERATION_RESULT_SUCCESS) {
      return operation_result;
    } else {
      fill_rx_frame_with_local(p_rx_options);
    }
  }

  switch (operation_result) {
    case U3C_DB_OPERATION_RESULT_SUCCESS: {
      CC_UserCredential_UserReport_tx(
        USER_REP_TYPE_ADDED, p_user, p_name,
        CC_UserCredential_get_next_user(p_user->unique_identifier), p_rx_options);
      break;
    }

    case U3C_DB_OPERATION_RESULT_FAIL_OCCUPIED: {
      CC_UserCredential_get_user(p_user->unique_identifier, p_user, p_name);
      CC_UserCredential_UserReport_tx(
        USER_REP_TYPE_ADD_AGAINST_OCCUPIED, p_user, p_name,
        CC_UserCredential_get_next_user(p_user->unique_identifier), p_rx_options);
      break;
    }

    case U3C_DB_OPERATION_RESULT_FAIL_FULL: {
      // Send report of a blank user with the User Unchanged report type
      memset(p_user, 0, sizeof(u3c_user_t));
      p_user->credential_rule = CREDENTIAL_RULE_SINGLE;
      CC_UserCredential_UserReport_tx(
        USER_REP_TYPE_UNCHANGED, p_user, NULL, 0, p_rx_options);
      break;
    }

    case U3C_DB_OPERATION_RESULT_FAIL_IDENTICAL: {
      /**
       * The received data is identical to that stored in the database.
       * Report data already occupying the UUID.
       */
      CC_UserCredential_get_user(p_user->unique_identifier, p_user, NULL);
      CC_UserCredential_UserReport_tx(
        USER_REP_TYPE_UNCHANGED, p_user, p_name,
        CC_UserCredential_get_next_user(p_user->unique_identifier), p_rx_options);
      break;
    }

    default:
      break;
  }
  return operation_result;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_modify_user_and_report(
  u3c_user_t * p_user, uint8_t * p_name, RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  if (!p_user || !p_name || !p_rx_options) {
    assert(false);
    return U3C_DB_OPERATION_RESULT_ERROR;
  }

  u3c_db_operation_result operation_result = U3C_DB_OPERATION_RESULT_SUCCESS;

  if (!validate_user_name_encoding(p_name, p_user->name_length, p_user->name_encoding)) {
    // Invalid user name. Check if user exists. Report is different.
    operation_result = CC_UserCredential_get_user(p_user->unique_identifier, NULL, NULL);
    if (operation_result == U3C_DB_OPERATION_RESULT_SUCCESS) {
      // User exists, but the new user name is invalid.
      operation_result = U3C_DB_OPERATION_RESULT_ERROR;
    }
  }

  if (operation_result == U3C_DB_OPERATION_RESULT_SUCCESS) {
    operation_result = CC_UserCredential_modify_user(p_user, p_name);
  }

  if (is_rx_frame_initiated_locally(p_rx_options)) {
    // Locally initiated request, no report is required on error.
    if (operation_result != U3C_DB_OPERATION_RESULT_SUCCESS) {
      return operation_result;
    } else {
      fill_rx_frame_with_local(p_rx_options);
    }
  }

  switch (operation_result) {
    case U3C_DB_OPERATION_RESULT_SUCCESS: {
      CC_UserCredential_UserReport_tx(
        USER_REP_TYPE_MODIFIED, p_user, p_name,
        CC_UserCredential_get_next_user(p_user->unique_identifier), p_rx_options);
      break;
    }

    case U3C_DB_OPERATION_RESULT_FAIL_IDENTICAL: {
      /**
       * The received data is identical to that stored in the database.
       * Report data already occupying the UUID.
       */
      CC_UserCredential_get_user(p_user->unique_identifier, p_user, p_name);
      CC_UserCredential_UserReport_tx(
        USER_REP_TYPE_UNCHANGED, p_user, p_name,
        CC_UserCredential_get_next_user(p_user->unique_identifier), p_rx_options);
      break;
    }

    case U3C_DB_OPERATION_RESULT_FAIL_DNE: {
      p_user->modifier_type = MODIFIER_TYPE_DNE;
      p_user->modifier_node_id = 0;
      CC_UserCredential_UserReport_tx(
        USER_REP_TYPE_MODIF_AGAINST_EMPTY, p_user, p_name, 0,
        p_rx_options);
      break;
    }

    default:
      break;
  }
  return operation_result;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_delete_user_and_report(
  uint16_t uuid, RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  u3c_user_t user = {
    .unique_identifier = uuid,
    .credential_rule = CREDENTIAL_RULE_SINGLE
  };
  uint8_t name[U3C_BUFFER_SIZE_USER_NAME];
  if (!p_rx_options) {
    assert(false);
    return U3C_DB_OPERATION_RESULT_ERROR;
  }

  u3c_db_operation_result operation_result = CC_UserCredential_get_user(uuid, &user, name);

  if (is_rx_frame_initiated_locally(p_rx_options)) {
    // Locally initiated request, no report is required on error.
    if (operation_result != U3C_DB_OPERATION_RESULT_SUCCESS) {
      return operation_result;
    } else {
      fill_rx_frame_with_local(p_rx_options);
    }
  }

  switch (operation_result) {
    case U3C_DB_OPERATION_RESULT_SUCCESS: {
      CC_UserCredential_delete_all_credentials_of_type(uuid, CREDENTIAL_TYPE_NONE);
      operation_result = CC_UserCredential_delete_user(user.unique_identifier);

      CC_UserCredential_UserReport_tx(
        USER_REPORT_DELETED, &user, name,
        CC_UserCredential_get_next_user(uuid), p_rx_options);
      break;
    }

    case U3C_DB_OPERATION_RESULT_FAIL_DNE: {
      CC_UserCredential_UserReport_tx(
        USER_REPORT_UNCHANGED, &user, NULL,
        CC_UserCredential_get_next_user(uuid), p_rx_options);
      break;
    }

    default: {
      break;
    }
  }

  return operation_result;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_add_credential_and_report(
  u3c_credential_t * p_credential, RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  if (!p_credential || !p_rx_options) {
    assert(false);
    return U3C_DB_OPERATION_RESULT_ERROR;
  }

  if (U3C_DB_OPERATION_RESULT_SUCCESS != check_and_report_nonexistent_user(p_credential, p_rx_options)) {
    return U3C_DB_OPERATION_RESULT_ERROR;
  }

  u3c_db_operation_result operation_result = CC_UserCredential_add_credential(p_credential);

  if (is_rx_frame_initiated_locally(p_rx_options)) {
    // Locally initiated request, no report is required on error.
    if (operation_result != U3C_DB_OPERATION_RESULT_SUCCESS) {
      return operation_result;
    } else {
      fill_rx_frame_with_local(p_rx_options);
    }
  }

  switch (operation_result) {
    case U3C_DB_OPERATION_RESULT_SUCCESS: {
      // Get next credential's details
      u3c_credential_type next_credential_type = CREDENTIAL_TYPE_NONE;
      uint16_t next_credential_slot = 0;
      CC_UserCredential_get_next_credential(
        p_credential->metadata.uuid, p_credential->metadata.type,
        p_credential->metadata.slot,
        &next_credential_type, &next_credential_slot);

      CC_UserCredential_CredentialReport_tx(
        CREDENTIAL_REP_TYPE_ADDED, p_credential, next_credential_type,
        next_credential_slot, p_rx_options);
      break;
    }

    case U3C_DB_OPERATION_RESULT_FAIL_OCCUPIED: {
      // Read existing Credential from database and send its details in the report
      CC_UserCredential_get_credential(
        p_credential->metadata.uuid, p_credential->metadata.type, p_credential->metadata.slot,
        &p_credential->metadata, p_credential->data);

      // Get next credential's details
      u3c_credential_type next_credential_type = CREDENTIAL_TYPE_NONE;
      uint16_t next_credential_slot = 0;
      CC_UserCredential_get_next_credential(
        p_credential->metadata.uuid, p_credential->metadata.type,
        p_credential->metadata.slot,
        &next_credential_type, &next_credential_slot);

      CC_UserCredential_CredentialReport_tx(
        CREDENTIAL_REP_TYPE_ADD_AGAINST_OCCUPIED, p_credential,
        next_credential_type, next_credential_slot, p_rx_options);
      break;
    }

    case U3C_DB_OPERATION_RESULT_FAIL_FULL: {
      // Report an empty credential with unchanged status
      p_credential->metadata.length = 0;
      p_credential->metadata.modifier_type = MODIFIER_TYPE_DNE;
      p_credential->metadata.modifier_node_id = 0;
      CC_UserCredential_CredentialReport_tx(
        CREDENTIAL_REP_TYPE_UNCHANGED, p_credential, CREDENTIAL_TYPE_NONE, 0,
        p_rx_options);
      break;
    }

    case U3C_DB_OPERATION_RESULT_FAIL_IDENTICAL: {
      // Read existing Credential from database and send its details in the report
      CC_UserCredential_get_credential(
        p_credential->metadata.uuid, p_credential->metadata.type, p_credential->metadata.slot,
        &p_credential->metadata, p_credential->data);

      // Get next credential's details
      u3c_credential_type next_credential_type = CREDENTIAL_TYPE_NONE;
      uint16_t next_credential_slot = 0;
      CC_UserCredential_get_next_credential(
        0, p_credential->metadata.type, p_credential->metadata.slot,
        &next_credential_type, &next_credential_slot);

      CC_UserCredential_CredentialReport_tx(
        CREDENTIAL_REP_TYPE_UNCHANGED, p_credential,
        next_credential_type, next_credential_slot, p_rx_options);
      break;
    }

    default:
      break;
  }
  return operation_result;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_modify_credential_and_report(
  u3c_credential_t * p_credential, RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  if (!p_credential || !p_rx_options) {
    assert(false);
    return U3C_DB_OPERATION_RESULT_ERROR;
  }

  if (U3C_DB_OPERATION_RESULT_SUCCESS != check_and_report_nonexistent_user(p_credential, p_rx_options)) {
    return U3C_DB_OPERATION_RESULT_ERROR;
  }

  u3c_db_operation_result operation_result = CC_UserCredential_modify_credential(p_credential);

  if (is_rx_frame_initiated_locally(p_rx_options)) {
    // Locally initiated request, no report is required on error.
    if (operation_result != U3C_DB_OPERATION_RESULT_SUCCESS) {
      return operation_result;
    } else {
      fill_rx_frame_with_local(p_rx_options);
    }
  }

  switch (operation_result) {
    case U3C_DB_OPERATION_RESULT_SUCCESS: {
      // Get next credential's details
      u3c_credential_type next_credential_type = CREDENTIAL_TYPE_NONE;
      uint16_t next_credential_slot = 0;
      CC_UserCredential_get_next_credential(
        0, p_credential->metadata.type, p_credential->metadata.slot,
        &next_credential_type, &next_credential_slot);

      CC_UserCredential_CredentialReport_tx(
        CREDENTIAL_REP_TYPE_MODIFIED, p_credential, next_credential_type,
        next_credential_slot, p_rx_options);
      break;
    }

    case U3C_DB_OPERATION_RESULT_FAIL_DNE: {
      p_credential->metadata.modifier_type = MODIFIER_TYPE_DNE;
      p_credential->metadata.modifier_node_id = 0;
      CC_UserCredential_CredentialReport_tx(
        CREDENTIAL_REP_TYPE_MODIF_AGAINST_EMPTY, p_credential,
        CREDENTIAL_TYPE_NONE, 0, p_rx_options);
      break;
    }

    case U3C_DB_OPERATION_RESULT_FAIL_REASSIGN: {
      /**
       * The UUID must not be changed via this command.
       * User Credential Association Set should be used instead.
       * Read existing Credential from database and send its details in the report
       */
      CC_UserCredential_get_credential(
        p_credential->metadata.uuid, p_credential->metadata.type, p_credential->metadata.slot,
        &p_credential->metadata, p_credential->data);

      // Get next credential's details
      u3c_credential_type next_credential_type = CREDENTIAL_TYPE_NONE;
      uint16_t next_credential_slot = 0;
      CC_UserCredential_get_next_credential(
        0, p_credential->metadata.type, p_credential->metadata.slot,
        &next_credential_type, &next_credential_slot);

      CC_UserCredential_CredentialReport_tx(
        CREDENTIAL_REP_TYPE_WRONG_UUID, p_credential,
        next_credential_type, next_credential_slot, p_rx_options);
      break;
    }

    case U3C_DB_OPERATION_RESULT_FAIL_IDENTICAL: {
      // Read existing Credential from database and send its details in the report
      CC_UserCredential_get_credential(
        p_credential->metadata.uuid, p_credential->metadata.type, p_credential->metadata.slot,
        &p_credential->metadata, p_credential->data);

      // Get next credential's details
      u3c_credential_type next_credential_type = CREDENTIAL_TYPE_NONE;
      uint16_t next_credential_slot = 0;
      CC_UserCredential_get_next_credential(
        0, p_credential->metadata.type, p_credential->metadata.slot,
        &next_credential_type, &next_credential_slot);

      CC_UserCredential_CredentialReport_tx(
        CREDENTIAL_REP_TYPE_UNCHANGED, p_credential,
        next_credential_type, next_credential_slot, p_rx_options);
      break;
    }

    default:
      break;
  }

  return operation_result;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_delete_credential_and_report(
  uint16_t uuid, u3c_credential_type credential_type,
  uint16_t credential_slot, RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  if (!p_rx_options) {
    assert(false);
    return U3C_DB_OPERATION_RESULT_ERROR;
  }

  u3c_credential_t credential = { 0 };
  uint8_t credential_data[U3C_BUFFER_SIZE_CREDENTIAL_DATA];
  // Read existing data pending deletion for use in Notification Report
  bool credential_exists = CC_UserCredential_get_credential(
    uuid,
    credential_type,
    credential_slot,
    &credential.metadata, credential_data
    ) == U3C_DB_OPERATION_RESULT_SUCCESS;

  credential.data = credential_data;

  u3c_db_operation_result operation_result = CC_UserCredential_delete_credential(
    credential_type,
    credential_slot
    );

  if (!credential_exists && !is_rx_frame_initiated_locally(p_rx_options)) {
    credential.metadata.uuid = uuid;
    credential.metadata.type = credential_type;
    credential.metadata.slot = credential_slot;
    credential.metadata.length = 0;
    credential.metadata.modifier_type = MODIFIER_TYPE_DNE;
    credential.metadata.modifier_node_id = 0;
    CC_UserCredential_CredentialReport_tx(
      CREDENTIAL_REP_TYPE_UNCHANGED, &credential, CREDENTIAL_TYPE_NONE, 0,
      p_rx_options);
  } else if (credential_exists && operation_result == U3C_DB_OPERATION_RESULT_SUCCESS) {
    // Send report of deleted Credential
    if (is_rx_frame_initiated_locally(p_rx_options)) {
      fill_rx_frame_with_local(p_rx_options);
    }
    CC_UserCredential_CredentialReport_tx(
      CREDENTIAL_REP_TYPE_DELETED, &credential, CREDENTIAL_TYPE_NONE, 0,
      p_rx_options);
  }

  return operation_result;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_move_credential_and_report(
  u3c_credential_type credential_type,
  uint16_t credential_slot, uint16_t destination_uuid,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  if (!p_rx_options) {
    assert(false);
    return U3C_DB_OPERATION_RESULT_ERROR;
  }

  u3c_db_operation_result operation_result;
  u3c_credential_metadata_t source_metadata = {
    .type = credential_type,
    .slot = credential_slot
  };
  u3c_credential_metadata_t destination_metadata = {
    .uuid = destination_uuid,
    .slot = credential_slot
  };

  bool local_initiative = is_rx_frame_initiated_locally(p_rx_options);
  if (local_initiative) {
    fill_rx_frame_with_local(p_rx_options);
  }

  // CC:0083.01.12.11.007: Destination User Unique Identifier must reference an existing User
  if (U3C_DB_OPERATION_RESULT_SUCCESS != CC_UserCredential_get_user(destination_uuid, NULL, NULL)
      && !local_initiative) {
    CC_UserCredential_AssociationReport_tx(
      &source_metadata, &destination_metadata, U3C_UCAR_STATUS_DESTINATION_USER_UNIQUE_IDENTIFIER_NONEXISTENT, p_rx_options);
    return U3C_DB_OPERATION_RESULT_ERROR;
  }

  /**
   * Attempt to execute the move operation. If the source Credential does not
   * exist, send an error report.
   */
  operation_result = CC_UserCredential_move_credential(
    source_metadata.type, source_metadata.slot,
    destination_metadata.uuid, destination_metadata.slot);

  u3c_user_credential_association_report_status_t status;
  switch (operation_result) {
    case U3C_DB_OPERATION_RESULT_SUCCESS:
      status = U3C_UCAR_STATUS_SUCCESS;
      break;
    case U3C_DB_OPERATION_RESULT_FAIL_DNE:
      status = U3C_UCAR_STATUS_CREDENTIAL_SLOT_EMPTY;
      break;
    default:
      // Database error
      return operation_result;
  }

  if (U3C_DB_OPERATION_RESULT_SUCCESS == operation_result || !local_initiative) {
    CC_UserCredential_AssociationReport_tx(&source_metadata, &destination_metadata, status, p_rx_options);
  }

  return operation_result;
}

ZW_WEAK bool CC_UserCredential_send_association_report(
  u3c_credential_metadata_t const * const p_source_metadata,
  u3c_credential_metadata_t const * const p_destination_metadata,
  u3c_user_credential_association_report_status_t const status,
  RECEIVE_OPTIONS_TYPE_EX * const p_rx_options
  )
{
  CC_UserCredential_AssociationReport_tx(p_source_metadata, p_destination_metadata, status, p_rx_options);
  return true;
}
