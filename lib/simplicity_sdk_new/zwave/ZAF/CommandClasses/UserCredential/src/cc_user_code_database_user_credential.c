/**
 * @file
 * @brief Wrapper around the User Credential database for CC User Code
 * @details Replaces the database implementation of the User Code Command Class
 * with a wrapper accessing the PIN Codes stored by the User Credential Command
 * Class.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2024 Silicon Laboratories Inc.
 */

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/

#include "CC_UserCode.h"
#include "CC_UserCredential.h"
#include "cc_user_credential_io.h"
#include "cc_user_credential_config.h"
#include "cc_user_credential_config_api.h"
#include "cc_user_credential_io_config.h"
#include "cc_user_credential_validation.h"
#include "cc_user_credential_operations.h"
#include <string.h>
#include "assert.h"
#include "zpal_log.h"

/****************************************************************************/
/*                           STATIC PARAMETER CHECK                         */
/****************************************************************************/

// Ensure PIN slots are less or equal to 255 if User Code v1 is supported, CC:0083.01.00.21.015
_Static_assert(CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_PIN_CODE <= 255,
               "PIN slots must be less or equal to 255 if User Code v1 is supported");

// Ensure Duress and Disposable User Types are not enabled if User Code v1 is supported, CC:0083.01.00.21.017
_Static_assert(CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_DISPOSABLE == 0,
               "Disposable User Type must not be enabled if User Code v1 is supported");
_Static_assert(CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_DURESS == 0,
               "Duress User Type must not be enabled if User Code v1 is supported");

// Ensure Minimum and Maximum PIN Code Data Length is compatible with User Code CC, CC:0083.01.00.21.014
_Static_assert(CC_USER_CREDENTIAL_MIN_DATA_LENGTH_PIN_CODE == 4,
               "STATIC_ASSERT_FAILED_Minimum_length_PIN_Code_data_is_smaller_than_4");
_Static_assert(CC_USER_CREDENTIAL_MAX_DATA_LENGTH_PIN_CODE == 10,
               "STATIC_ASSERT_FAILED_Maximum_length_PIN_Code_data_is_larger_than_10");

/****************************************************************************/
/*                                CONSTANTS                                 */
/****************************************************************************/

#define USER_CODE_REPORT_DATA_MAX_LENGTH \
  TX_DATA_MAX_DATA_SIZE - offsetof(ZW_USER_CODE_REPORT_1BYTE_FRAME, userCode1)

/****************************************************************************/
/*                            PRIVATE FUNCTIONS                             */
/****************************************************************************/

static bool is_valid_user_identifier(uint8_t user_identifier)
{
  bool is_valid = user_identifier
                  <= cc_user_credential_get_max_credential_slots(
    CREDENTIAL_TYPE_PIN_CODE);
  return is_valid;
}

/**
 * Add or modify a User Code
 *
 * @param[in] user_identifier  The User Identifier.
 * @param[in] pUserCode        Pointer to the new user code.
 * @param[in] len              The length of the new user code.
 * @param[in] status           The new status of the user code.
 *                             Set to USER_ID_NO_STATUS to keep it unchanged.
 * @param[in] modifier_node_id The ID of the node that modified the user code.
 *                             Set to 0 if modifying locally.
 * @param[in] assigned_uuid    The UUID which this credential is assigned to.
 *                             0 indicates that the credential does not exist in
 *                             the database.
 *
 * @return true if the operation was successful
 */
static bool set_user_code(
  const uint8_t user_identifier,
  uint8_t * const pUserCode,
  const uint8_t len,
  const user_id_status_t status,
  const uint16_t modifier_node_id,
  const uint16_t assigned_uuid)
{
  if (status == USER_ID_AVAILABLE || status == USER_ID_NO_STATUS) {
    ZPAL_LOG_ERROR(ZPAL_LOG_CC_USER_CREDENTIAL, "The status 0x%X is invalid!", status);
    assert(false);
    return false;
  }

  // Ensure that the length of the data is correct
  if (len < cc_user_credential_get_min_length_of_data(
        CREDENTIAL_TYPE_PIN_CODE)
      || len > cc_user_credential_get_max_length_of_data(
        CREDENTIAL_TYPE_PIN_CODE)) {
    return false;
  }

  u3c_modifier_type modifier_type = (modifier_node_id == 0)
                                    ? MODIFIER_TYPE_LOCALLY
                                    : MODIFIER_TYPE_Z_WAVE;
  bool user_active = (status == USER_ID_OCCUPIED);
  // The UUID to associate the new/modified credential with
  uint16_t uuid = (assigned_uuid != 0)
                  ? assigned_uuid
                  : user_identifier;
  u3c_user_t existing_user = { 0 };
  // True if the associated user was found in the database
  bool user_found = CC_UserCredential_get_user(uuid, &existing_user, NULL)
                    == U3C_DB_OPERATION_RESULT_SUCCESS;

  if (!user_found) {
    // Add a new User with the same UUID as the User Identifier
    uint8_t name[U3C_BUFFER_SIZE_USER_NAME];
    u3c_user_t user = {
      .unique_identifier = user_identifier,
      .type = USER_TYPE_GENERAL,
      .modifier_type = modifier_type,
      .modifier_node_id = modifier_node_id,
      .credential_rule = CREDENTIAL_RULE_SINGLE,
      .active = user_active,
      .name_encoding = USER_NAME_ENCODING_STANDARD_ASCII,
      .name_length = 0
    };
    CC_UserCredential_set_default_name(name, &user);
    if (CC_UserCredential_add_user(&user, name)
        != U3C_DB_OPERATION_RESULT_SUCCESS) {
      return false;
    }
  } else if (existing_user.active != user_active) {
    // Modify the user's active state
    existing_user.active = user_active;
    CC_UserCredential_modify_user(&existing_user, NULL);
  }

  u3c_credential_t credential = {
    .data = pUserCode,
    .metadata = {
      .length = len,
      .modifier_type = modifier_type,
      .modifier_node_id = modifier_node_id,
      .uuid = uuid,
      .type = CREDENTIAL_TYPE_PIN_CODE,
      .slot = user_identifier
    }
  };

  if (!validate_new_credential_metadata(&credential.metadata)
      || !validate_new_credential_data(&credential, NULL)) {
    return false;
  }

  u3c_db_operation_result credential_set_result =
    (assigned_uuid != 0)
    ? CC_UserCredential_modify_credential(&credential)
    : CC_UserCredential_add_credential(&credential);

  return ((credential_set_result == U3C_DB_OPERATION_RESULT_SUCCESS)
          || (credential_set_result == U3C_DB_OPERATION_RESULT_FAIL_IDENTICAL));
}

/****************************************************************************/
/*                             PUBLIC FUNCTIONS                             */
/****************************************************************************/

uint8_t CC_UserCode_UsersNumberReport_handler(
  __attribute__((unused)) uint8_t endpoint)
{
  uint16_t max_slots =
    cc_user_credential_get_max_credential_slots(CREDENTIAL_TYPE_PIN_CODE);
  return (uint8_t)(max_slots > UINT8_MAX ? UINT8_MAX : max_slots);
}

bool CC_UserCode_getId_handler(
  uint8_t user_identifier,
  USER_ID_STATUS* pUserIdStatus,
  __attribute__((unused)) uint8_t endpoint)
{
  if (!pUserIdStatus) {
    assert(false);
    return false;
  }

  if (!is_valid_user_identifier(user_identifier)) {
    return false;
  }

  // Determine the User ID Status according to CC:0083.01.00.21.021
  *pUserIdStatus = USER_ID_NO_STATUS;
  u3c_credential_metadata_t credential_metadata = { 0 };
  switch (CC_UserCredential_get_credential(
            0, CREDENTIAL_TYPE_PIN_CODE, user_identifier, &credential_metadata,
            NULL)
          ) {
    case U3C_DB_OPERATION_RESULT_SUCCESS: {
      // The credential exists
      u3c_user_t user = { 0 };
      if (CC_UserCredential_get_user(credential_metadata.uuid, &user, NULL)
          == U3C_DB_OPERATION_RESULT_SUCCESS) {
        // The associated user exists
        *pUserIdStatus = (!user.active || (user.type == USER_TYPE_NON_ACCESS))
                         ? USER_ID_RESERVED
                         : USER_ID_OCCUPIED;
      } else {
        // The associated user does not exist!
        assert(false);
        return false;
      }
      break;
    }
    case U3C_DB_OPERATION_RESULT_FAIL_DNE: {
      // The credential does not exist
      *pUserIdStatus = USER_ID_AVAILABLE;
      break;
    }
    default: {
      // Error reading the credential from the database!
      assert(false);
      return false;
    }
  }

  return true;
}

bool CC_UserCode_Report_handler(
  uint8_t user_identifier,
  uint8_t* pUserCode,
  size_t * pLen,
  __attribute__((unused)) uint8_t endpoint)
{
  if (!pUserCode || !pLen) {
    assert(false);
    return false;
  }

  if (!is_valid_user_identifier(user_identifier)) {
    return false;
  }

  u3c_credential_metadata_t metadata = { 0 };
  uint8_t credential_data[U3C_BUFFER_SIZE_CREDENTIAL_DATA] = { 0 };
  u3c_db_operation_result result_credential_get =
    CC_UserCredential_get_credential(
      0, CREDENTIAL_TYPE_PIN_CODE, user_identifier, &metadata, credential_data);
  if (!(result_credential_get == U3C_DB_OPERATION_RESULT_SUCCESS
        || result_credential_get == U3C_DB_OPERATION_RESULT_FAIL_DNE)) {
    return false;
  }

  /**
   * If the constant length fields of the report + the PIN Code data do not fit
   * in the outgoing frame, abort.
   */
  if (metadata.length > USER_CODE_REPORT_DATA_MAX_LENGTH) {
    assert(false);
    return false;
  }

  if (result_credential_get == U3C_DB_OPERATION_RESULT_FAIL_DNE) {
    /**
     * CC:0063.01.01.11.009: If the User Code does not exist,
     * the User Code field must be set to 0x00000000 (4 bytes).
     */
    *pLen = 4;
    memset(pUserCode, 0x00, *pLen);
  } else {
    *pLen = metadata.length;
    memcpy(pUserCode, credential_data, metadata.length);
  }
  return true;
}

e_cmd_handler_return_code_t CC_UserCode_Set_handler(
  uint8_t user_identifier,
  USER_ID_STATUS user_id_status,
  uint8_t* pUserCode,
  uint8_t len,
  uint16_t modifier_node_id)
{
  if (!pUserCode) {
    assert(false);
    return E_CMD_HANDLER_RETURN_CODE_FAIL;
  }

  if (!is_valid_user_identifier(user_identifier)
      || user_id_status == USER_ID_NO_STATUS) {
    return E_CMD_HANDLER_RETURN_CODE_FAIL;
  }

  e_cmd_handler_return_code_t status = E_CMD_HANDLER_RETURN_CODE_FAIL;
  bool credential_found = false;
  u3c_credential_metadata_t credential_metadata = { 0 };
  u3c_db_operation_result get_result =
    CC_UserCredential_get_credential(0, CREDENTIAL_TYPE_PIN_CODE,
                                     user_identifier, &credential_metadata,
                                     NULL);
  switch (get_result) {
    case U3C_DB_OPERATION_RESULT_SUCCESS:
      credential_found = true;
      break;
    case U3C_DB_OPERATION_RESULT_FAIL_DNE:
      credential_found = false;
      break;
    default:
      // I/O error
      ZPAL_LOG_ERROR(ZPAL_LOG_CC_USER_CREDENTIAL, "Credential retrieval failed!");
      assert(false);
      return E_CMD_HANDLER_RETURN_CODE_FAIL;
  }

  if (user_id_status == USER_ID_AVAILABLE && user_identifier == 0) {
    uint16_t user_num = cc_user_credential_get_max_credential_slots(CREDENTIAL_TYPE_PIN_CODE);
    // Return value will be this, except on error. Those will be handled below.
    status = E_CMD_HANDLER_RETURN_CODE_HANDLED;
    for (uint16_t user_i = 1; user_i <= user_num; user_i++) {
      // Delete user code.
      // No action needed when successful or when the credential does not exist.
      u3c_db_operation_result delete_result =
        CC_UserCredential_delete_credential(CREDENTIAL_TYPE_PIN_CODE, user_i);

      if (delete_result != U3C_DB_OPERATION_RESULT_FAIL_DNE
          && delete_result != U3C_DB_OPERATION_RESULT_SUCCESS) {
        // I/O error
        ZPAL_LOG_ERROR(ZPAL_LOG_CC_USER_CREDENTIAL, "Credential deletion failed!");
        assert(false);
        status = E_CMD_HANDLER_RETURN_CODE_FAIL;
        break;
      }
    }
  } else if (user_id_status == USER_ID_AVAILABLE) {
    if (credential_found) {
      // Delete User Code
      u3c_db_operation_result delete_result =
        CC_UserCredential_delete_credential(CREDENTIAL_TYPE_PIN_CODE,
                                            user_identifier);
      if (delete_result == U3C_DB_OPERATION_RESULT_SUCCESS) {
        status = E_CMD_HANDLER_RETURN_CODE_HANDLED;
      } else {
        ZPAL_LOG_ERROR(ZPAL_LOG_CC_USER_CREDENTIAL, "Credential deletion failed!");
        assert(false);
      }
    } else {
      // No PIN code to delete, database is already in requested state
      status = E_CMD_HANDLER_RETURN_CODE_HANDLED;
    }
  } else {
    if (set_user_code(user_identifier, pUserCode, len, user_id_status,
                      modifier_node_id, credential_metadata.uuid)
        ) {
      status = E_CMD_HANDLER_RETURN_CODE_HANDLED;
    }
  }
  return status;
}

void CC_UserCode_reset_data(void)
{
  // The database is handled by the User Credential Command Class.
}

/**
 * @brief Set the default user code to a new value.
 *
 * @param[in] new_user_code The new user code.
 */
void CC_UserCode_set_usercode(char * new_user_code)
{
  const uint16_t user_identifier = 1;
  u3c_credential_metadata_t credential_metadata = { 0 };
  CC_UserCredential_get_credential(0, CREDENTIAL_TYPE_PIN_CODE, user_identifier,
                                   &credential_metadata, NULL);
  uint8_t length = (uint8_t)strnlen(new_user_code,
                                    cc_user_credential_get_max_length_of_data(
                                      CREDENTIAL_TYPE_PIN_CODE));

  if (!set_user_code(user_identifier, (uint8_t *)new_user_code, length,
                     USER_ID_NO_STATUS, 0, credential_metadata.uuid)
      ) {
    // Could not set the new user code
    assert(false);
  }
}

bool CC_UserCode_Validate(uint8_t identifier, const uint8_t *pCode, uint8_t len)
{
  uint8_t data[U3C_BUFFER_SIZE_CREDENTIAL_DATA] = { 0 };
  memcpy(&data, pCode, len);
  const u3c_credential_t credential = {
    .data = data,
    .metadata = {
      .length = len,
      .modifier_type = MODIFIER_TYPE_UNKNOWN,
      .modifier_node_id = 0,
      .uuid = 0,
      .type = CREDENTIAL_TYPE_PIN_CODE,
      .slot = identifier
    }
  };
  u3c_credential_metadata_t existing_metadata = { 0 };
  return find_existing_credential(&credential, &existing_metadata)
         && (existing_metadata.slot == identifier);
}
