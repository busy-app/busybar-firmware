/**
 * @file
 * User Credential Command Class credential validation.
 *
 * @copyright 2024 Silicon Laboratories Inc.
 */

#ifndef CC_USER_CREDENTIAL_VALIDATION_H
#define CC_USER_CREDENTIAL_VALIDATION_H

#include "CC_UserCredential.h"

/**
 * Function pointer type for Credential Type Validators
 */
typedef bool (*u3c_credential_type_validator_t)(u3c_credential_t*, RECEIVE_OPTIONS_TYPE_EX*);

/**
 * Checks whether a Credential is identical to an existing Credential in the
 * database.
 *
 * @param[in]  p_credential        Pointer to the incoming credential
 * @param[out] p_existing_metadata Pointer to the existing credential metadata
 *                                 (valid only if true was returned)
 *
 * @return true if the credential is a duplicate
 */
bool find_existing_credential(
  const u3c_credential_t * const p_credential,
  u3c_credential_metadata_t * p_existing_metadata
  );

/**
 * Function for validating a Credential against the rules mandated by the
 * specification
 *
 * @param[in] p_credential Pointer to the incoming credential
 * @param[in] p_rx_options Pointer to the properties of the incoming frame
 *
 * @return true if the credential passes all checks
 */
bool validate_credential_data(u3c_credential_t * p_credential, RECEIVE_OPTIONS_TYPE_EX * p_rx_options);

/**
 * Validates that a new Credential conforms to the active configuration of the
 * User Credential Command Class.
 *
 * @param[in] p_metadata Pointer to the incoming Credential data
 *
 * @return true if the Credential passes all checks
 */
bool validate_new_credential_metadata(
  const u3c_credential_metadata_t * const p_metadata
  );

/**
 * Validates that a UUID belongs to an existing user.
 *
 * @param[in] uuid UUID to check if exists
 *
 * @return true if user exists
 */
bool validate_associated_uuid(
  uint16_t uuid
  );

/**
 * Validates that a new Credential is not a duplicate and that it conforms to
 * the manufacturer's security rules and sends the required reports if these
 * checks fail.
 *
 * @param[in] p_credential Pointer to the incoming Credential data
 * @param[in] p_rx_options Pointer to the properties of the incoming frame
 *                         (set to NULL if a response should not be sent)
 *
 * @return true if the Credential passes all checks
 */
bool validate_new_credential_data(
  u3c_credential_t * p_credential,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  );

/**
 * @brief Validates that credential does not exist in the database, is not a
 * duplicate of the admin code and conforms to the manufacturer security rules.
 *
 * This function checks that the following requirements are satisfied:
 *
 * CC:0083.01.1A.13.004 - AC is not a duplicate of the current AC
 * CC:0083.01.1A.13.005 - AC is not a duplicate of the existing credential
 * CC:0083.01.1A.11.011 - Manufacturer and application specific requirements
 *
 * @param data Pointer to structure containing Admin Code information.
 *
 * @returns True if all checks pass. If checks do not pass, returns false, and
 * result code populated in code struct.
 *
 * @note result code of a successful check should be ignored, but in
 * practice ensure it is set to ADMIN_CODE_OPERATION_RESULT_NONE in this case.
 */
bool validate_admin_pin_code(u3c_admin_code_metadata_t * const data);

/**
 * Validates user name encoding.
 *
 * @param[in] p_name User name string.
 * @param[in] p_name_length Length of username in bytes.
 * @param[in] p_name_encoding User name encoding.
 *
 * @return true if user name is valid for the given encoding.
 */
bool validate_user_name_encoding(const uint8_t * p_name, uint8_t p_name_length, u3c_user_name_encoding p_name_encoding);

/**
 * @}
 * @}
 */
#endif /* CC_USER_CREDENTIAL_VALIDATION_H */
