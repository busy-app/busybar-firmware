/**
 * @file
 * @brief User Credential data validation functions
 * @details Contains functions for validating User Credential data of each
 * credential type against the rules set out by the specification.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2024 Silicon Laboratories Inc.
 */

#include "CC_UserCredential.h"
#include "cc_user_credential_validation.h"
#include "cc_user_credential_io.h"
#include "cc_user_credential_config_api.h"
#include "cc_user_credential_io_config.h"
#include "cc_user_credential_tx.h"
#include "assert.h"
#include <string.h>

#define ASCII_USER_NAME_CHAR_VALUE_MAX (0x7F)

/****************************************************************************/
/*                            PRIVATE FUNCTIONS                             */
/****************************************************************************/

static bool is_identical_to_admin_pin_code(
  u3c_credential_t * p_credential, RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  )
{
  if (!cc_user_credential_get_admin_code_supported()) {
    return false;
  }

  // Read the admin code from the database
  u3c_admin_code_metadata_t admin_code_metadata = { 0 };
  u3c_db_operation_result get_admin_code_result =
    CC_UserCredential_get_admin_code_info(&admin_code_metadata);
  if (get_admin_code_result != U3C_DB_OPERATION_RESULT_SUCCESS) {
    // Could not read the admin code
    assert(false);
    return false;
  }

  if (admin_code_metadata.code_length != p_credential->metadata.length) {
    /**
     * The provided PIN code cannot be identical to the admin PIN code if its
     * length is different
     */
    return false;
  }

  int comparison_result = strncmp(
    (char *)p_credential->data, (char *)admin_code_metadata.code_data,
    admin_code_metadata.code_length);

  if (comparison_result == 0) {
    // The PIN codes are identical

    if (p_rx_options) {
      // Report duplicate of admin PIN code

      // Get next credential's details
      u3c_credential_type next_credential_type = CREDENTIAL_TYPE_NONE;
      uint16_t next_credential_slot = 0;
      CC_UserCredential_get_next_credential(
        0, p_credential->metadata.type, p_credential->metadata.slot,
        &next_credential_type, &next_credential_slot
        );

      // Send report
      CC_UserCredential_CredentialReport_tx(
        CREDENTIAL_REP_TYPE_DUPLICATE_ADMIN_PIN_CODE, p_credential,
        next_credential_type, next_credential_slot, p_rx_options);
    }

    return true;
  }

  return false;
}

/**
 * Validates a PIN Code Credential
 *
 * @return true if the Credential is valid
 */
static bool validate_pin_code(u3c_credential_t * p_credential, RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  // If the Admin Code is supported, the PIN Code must not match the Admin Code
  // CC:0083.01.1A.13.005
  if (is_identical_to_admin_pin_code(p_credential, p_rx_options)) {
    return false;
  }

  // PIN Code must be at least 4 digits long
  // CC:0083.01.00.41.016
  if (p_credential->metadata.length < U3C_CREDENTIAL_TYPE_PIN_CODE_MIN_LENGTH_REQUIREMENT) {
    return false;
  }

  // Only allow numeric PIN Codes
  // CC:0083.01.0A.11.000
  for (uint8_t i = 0; i < p_credential->metadata.length; ++i) {
    uint8_t character = p_credential->data[i];
    if (character < '0' || character > '9') {
      return false;
    }
  }

  return true;
}

/****************************************************************************/
/*                             STATIC VARIABLES                             */
/****************************************************************************/

static u3c_credential_type_validator_t u3c_credential_validator_functions[CREDENTIAL_TYPE_NUMBER_OF_TYPES] = {
  [CREDENTIAL_TYPE_PIN_CODE] = validate_pin_code,
};

/****************************************************************************/
/*                             PUBLIC FUNCTIONS                             */
/****************************************************************************/

ZW_WEAK bool find_existing_credential(
  const u3c_credential_t * const p_credential,
  u3c_credential_metadata_t * p_existing_metadata)
{
  // Iterate through each User
  uint16_t uuid = CC_UserCredential_get_next_user(0);
  while (uuid) {
    // Iterate through each Credential
    u3c_credential_type type = CREDENTIAL_TYPE_NONE;
    uint16_t slot = 0;

    while (
      CC_UserCredential_get_next_credential(
        uuid, type, slot, &type, &slot)
      ) {
      if (p_credential->metadata.type == type) {
        // Read existing credential
        uint8_t e_data[U3C_BUFFER_SIZE_CREDENTIAL_DATA] = { 0 };

        CC_UserCredential_get_credential(
          uuid, type, slot, p_existing_metadata, e_data);

        // Check whether the incoming and existing data is identical
        if (p_existing_metadata->length == p_credential->metadata.length
            && (memcmp(e_data, p_credential->data, p_existing_metadata->length)
                == 0)
            ) {
          return true;
        }
      }
    }
    uuid = CC_UserCredential_get_next_user(uuid);
  }
  return false;
}

ZW_WEAK bool validate_credential_data(u3c_credential_t * p_credential, RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  if (u3c_credential_validator_functions[p_credential->metadata.type]) {
    return (
      u3c_credential_validator_functions[p_credential->metadata.type](p_credential, p_rx_options)
      );
  }

  return true;
}

ZW_WEAK bool validate_associated_uuid(
  uint16_t uuid)
{
  // CC:0083.01.05.11.016: Associated UUID must reference existing User
  return (U3C_DB_OPERATION_RESULT_SUCCESS == CC_UserCredential_get_user(uuid, NULL, NULL));
}

ZW_WEAK bool validate_new_credential_metadata(
  const u3c_credential_metadata_t * const p_metadata)
{
  if (!p_metadata) {
    assert(false);
    return false;
  }

  bool is_valid = (
    // CC:0083.01.0A.11.002
    cc_user_credential_is_credential_type_supported(p_metadata->type)

    // CC:0083.01.05.11.015
    && (p_metadata->uuid != 0)
    // CC:0083.01.05.11.014
    && (p_metadata->uuid <= cc_user_credential_get_max_user_unique_idenfitiers())

    // CC:0083.01.0A.11.004
    && (p_metadata->slot
        <= cc_user_credential_get_max_credential_slots(p_metadata->type))
    // CC:0083.01.0A.11.005
    && (p_metadata->slot != 0)
    );

  return is_valid;
}

ZW_WEAK bool validate_new_credential_data(
  u3c_credential_t * p_credential, RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  // Valiate the credential data against the rules in the specification
  if (!validate_credential_data(p_credential, p_rx_options)) {
    return false;
  }

  // CC:0083.01.0A.11.018: no duplicate credentials within a Credential Type
  u3c_credential_t existing_credential = {
    .metadata = { 0 },
    .data = p_credential->data
  };
  if (find_existing_credential(p_credential, &existing_credential.metadata)) {
    bool is_identifier_identical = (
      p_credential->metadata.type == existing_credential.metadata.type
      && p_credential->metadata.slot == existing_credential.metadata.slot
      );
    if (!is_identifier_identical) {
      if (p_rx_options) {
        // Get next credential's details
        u3c_credential_type next_credential_type = CREDENTIAL_TYPE_NONE;
        uint16_t next_credential_slot = 0;
        CC_UserCredential_get_next_credential(
          0, existing_credential.metadata.type,
          existing_credential.metadata.slot, &next_credential_type,
          &next_credential_slot);

        // Report duplicate
        CC_UserCredential_CredentialReport_tx(
          CREDENTIAL_REP_TYPE_DUPLICATE, &existing_credential,
          next_credential_type, next_credential_slot, p_rx_options);
      }
      return false;
    }
  }

  // CC:0083.01.0D.11.001: Credential must follow manufacturer security rules
  if (!CC_UserCredential_manufacturer_validate_credential(p_credential)) {
    if (p_rx_options) {
      CC_UserCredential_CredentialReport_tx(
        CREDENTIAL_REP_TYPE_MANUFACTURER_SECURITY_RULES, p_credential,
        CREDENTIAL_TYPE_NONE, 0, p_rx_options);
    }
    return false;
  }

  return true;
}

ZW_WEAK bool validate_admin_pin_code(u3c_admin_code_metadata_t * const data)
{
  /**
   * CC:0083.01.1A.13.004 - Ensure that the pin code provided is different than the existing one
   */
  u3c_admin_code_metadata_t current_code = { 0 };

  if (U3C_DB_OPERATION_RESULT_SUCCESS
      == CC_UserCredential_get_admin_code_info(&current_code)) {
    if (current_code.code_length == data->code_length
        && memcmp(data->code_data, current_code.code_data, current_code.code_length) == 0) {
      data->result = ADMIN_CODE_OPERATION_RESULT_INTERNAL_DUPLICATE_AC;
      return false;
    }
  } else {
    data->result = ADMIN_CODE_OPERATION_RESULT_ERROR_NODE;
    return false;
  }

  // Temporary credential wrapper
  u3c_credential_t credential = {
    .data = data->code_data,
    .metadata = {
      .length = data->code_length,
      .type = CREDENTIAL_TYPE_PIN_CODE,
    }
  };

  /**
   * CC:0083.01.1A.13.005 - Ensure that there is not a duplicate PIN code in the database
   */
  u3c_credential_metadata_t existing_credential = { 0 };
  if (find_existing_credential(&credential, &existing_credential)) {
    data->result = ADMIN_CODE_OPERATION_RESULT_FAIL_DUPLICATE_CRED;
    return false;
  }

  /**
   * CC:0083.01.1A.11.011 - Check manufacturer and application specific requirements
   */
  return CC_UserCredential_manufacturer_validate_admin_pin_code(data);
}

ZW_WEAK bool validate_user_name_encoding(const uint8_t * p_name, uint8_t p_name_length, u3c_user_name_encoding p_name_encoding)
{
  // CC:0083.01.05.11.044
  if (p_name_encoding == USER_NAME_ENCODING_STANDARD_ASCII) {
    for (uint8_t i = 0; i < p_name_length; i++) {
      if (p_name[i] > ASCII_USER_NAME_CHAR_VALUE_MAX) {
        return false;
      }
    }
  }
  return true;
}
