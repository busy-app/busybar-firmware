/**
 * @file
 * @brief Weak implementation for Command Class User Credential manufacturer
 * Credential validation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2024 Silicon Laboratories Inc.
 *
 * @copyright 2024 Silicon Laboratories Inc.
 */

#include "CC_UserCredential.h"

/**
 * @brief Validate the PIN Code credential
 * This function implements the manufacturer specific recommendations for PIN Code validation.
 * The behavior of this function can be configured with the USE_U3C_PIN_CODE_RECOMMENDATIONS macro.
 * @see cc_user_credential_config.h
 *
 * @param[in] p_credential Pointer to the credential to validate
 *
 * @return true if the credential is valid, false otherwise
 */
static bool CC_UserCredential_manufacturer_validate_pin_code(const u3c_credential_t * const p_credential)
{
  /**
   * The following requirements are marked as SHOULD in the specification.
   * If the application supports the User Code Command Class, then enforcing these recommendations
   * can result in a failed certification as the User Code Command Class is less strict
   * than the User Credential Command Class.
   * It is up to the manufacturer to decide whether to use these recommendations or not.
   */

#if USE_U3C_PIN_CODE_RECOMMENDATIONS
  // Check if the PIN Code only contains the same digit
  // CC:0083.01.00.42.015
  bool is_same_digit = true;
  for (uint8_t i = 1; i < p_credential->metadata.length; ++i) {
    if (p_credential->data[i] != p_credential->data[0]) {
      is_same_digit = false;
      break;
    }
  }

  if (is_same_digit) {
    return false;
  }

  // Check if the PIN Code digits are ascending or descending
  // CC:0083.01.00.42.014
  bool is_ascending = true;
  bool is_descending = true;
  for (uint8_t i = 1; i < p_credential->metadata.length; ++i) {
    if (p_credential->data[i] != (p_credential->data[i - 1] + 1)) {
      is_ascending = false;
    }
    if (p_credential->data[i] != (p_credential->data[i - 1] - 1)) {
      is_descending = false;
    }
  }

  if (is_ascending || is_descending) {
    return false;
  }

#else
  /**
   * If the User Credential Command Class feature is shipped with the User Code Command Class
   * then the backwards compatibility must be maintained. So the:
   * CC:0083.01.00.42.015
   * CC:0083.01.00.42.014
   * requirements are not used. Only the all zeros (0000) Pin Code is not allowed.
   */
  bool is_zeros = true;
  for (uint8_t i = 0; i < p_credential->metadata.length; ++i) {
    if (p_credential->data[i] != '0') {
      is_zeros = false;
      break;
    }
  }

  if (is_zeros) {
    return false;
  }
#endif

  return true;
}

ZW_WEAK bool CC_UserCredential_manufacturer_validate_credential(
  const u3c_credential_t * const credential)
{
  bool result = true;
  switch (credential->metadata.type) {
    case CREDENTIAL_TYPE_PIN_CODE:
      result = CC_UserCredential_manufacturer_validate_pin_code(credential);
      break;
    default:
      break;
  }
  return result;
}

ZW_WEAK bool CC_UserCredential_manufacturer_validate_admin_pin_code(
  u3c_admin_code_metadata_t * const code
  )
{
  const u3c_credential_t credential = {
    .data = code->code_data,
    .metadata = {
      .length = code->code_length,
      .type = CREDENTIAL_TYPE_PIN_CODE
    }
  };
  bool is_valid =
    CC_UserCredential_manufacturer_validate_pin_code(&credential);
  code->result = is_valid
                 ? ADMIN_CODE_OPERATION_RESULT_INTERNAL_NONE
                 : ADMIN_CODE_OPERATION_RESULT_FAIL_MANUF_RULE;
  return is_valid;
}
