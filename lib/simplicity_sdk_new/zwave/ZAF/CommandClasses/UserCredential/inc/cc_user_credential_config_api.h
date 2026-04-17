/**
 * @file
 * User Credential Command Class configuration API.
 *
 * @copyright 2024 Silicon Laboratories Inc.
 */

#ifndef CC_USER_CREDENTIAL_CONFIG_API_H
#define CC_USER_CREDENTIAL_CONFIG_API_H

#include "CC_UserCredential.h"

/**
 * Checks whether the application is configured to support a specific user type.
 *
 * @param[in] user_type The requested user type
 * @return true if this user type is supported
 */
bool cc_user_credential_is_user_type_supported(u3c_user_type user_type);

/**
 * Gets the configured maximum number of supported user unique identifiers.
 *
 * @return The maximum number of supported UUIDs
 */
uint16_t cc_user_credential_get_max_user_unique_idenfitiers(void);

/**
 * Gets the configured maximum user name length.
 *
 * @return The maximum length of a user name (in bytes)
 */
uint8_t cc_user_credential_get_max_length_of_user_name(void);

/**
 * Checks whether the application is configured to support a specific credential type.
 *
 * @param[in] credential_type The specific credential type
 * @return true if this credential type is supported
 */
bool cc_user_credential_is_credential_type_supported(u3c_credential_type credential_type);

/**
 * Checks whether the Credential Learn functionality is supported for a specific
 * credential type by the application.
 *
 * @param[in] credential_type The specific credential type
 * @return true if Credential Learn is supported for this credential type
 */
bool cc_user_credential_is_credential_learn_supported(u3c_credential_type credential_type);

/**
 * Checks whether a specific credential rule is supported by the application.
 *
 * @param[in] credential_rule The specific credential rule
 * @return true if this credential rule is supported
 */
bool cc_user_credential_is_credential_rule_supported(u3c_credential_rule credential_rule);

/**
 * Gets the number of credential types configured as supported by the application.
 *
 * @return The number of supported credential types
 */
uint8_t cc_user_credential_get_number_of_supported_credential_types(void);

/**
 * Gets the configured maximum number of slots for a specific credential type.
 *
 * @param[in] credential_type The specific credential type
 * @return The maximum number of credential slots for this credential type
 */
uint16_t cc_user_credential_get_max_credential_slots(u3c_credential_type credential_type);

/**
 * Gets the configured minimum length of the credential data for a specific credential type.
 *
 * @param[in] credential_type The specific credential type
 * @return The minimum allowed data length for this credential type (in bytes)
 */
uint8_t cc_user_credential_get_min_length_of_data(u3c_credential_type credential_type);

/**
 * Gets the configured maximum length of the credential data for a specific credential type.
 *
 * @param[in] credential_type The specific credential type
 * @return The maximum allowed data length for this credential type (in bytes)
 */
uint8_t cc_user_credential_get_max_length_of_data(u3c_credential_type credential_type);

/**
 * Gets the configured maximum hash length for a specific credential type.
 * If the credential type is supported by the application, a maximum hash length
 * of 0 implies that credentials of this type must always be read back when sent
 * in a report.
 *
 * @param[in] credential_type The specific credential type
 * @return The maximum allowed hash length for this credential type (in bytes),
 *         if supported
 */
uint8_t cc_user_credential_get_max_hash_length(u3c_credential_type type);

/**
 * Gets the configured recommended Credential Learn timeout for a specific credential type.
 *
 * @param[in] credential_type The specific credential type
 * @return The recommended Credential Learn timeout (in seconds)
 */
uint8_t cc_user_credential_get_cl_recommended_timeout(u3c_credential_type credential_type);

/**
 * Gets the number of steps required to complete the Credential Learn process
 * for a specific credential type.
 *
 * @param[in] credential_type The specific credential type
 * @return The number of steps required to complete the Credential Learn process
 */
uint8_t cc_user_credential_get_cl_number_of_steps(u3c_credential_type credential_type);

/**
 * Gets the configured recommended Credential Learn timeout for a specific credential type.
 *
 * @param[in] credential_type The specific credential type
 * @return The recommended Credential Learn timeout (in seconds)
 */
bool cc_user_credential_is_all_users_checksum_supported(void);

/**
 * Checks whether the User Checksum functionality is supported by the application.
 *
 * @return true if User Checksum is supported
 */
bool cc_user_credential_is_user_checksum_supported(void);

/**
 * Checks whether the Credential Checksum functionality is supported by the application.
 *
 * @return true if Credential Checksum is supported
 */
bool cc_user_credential_is_credential_checksum_supported(void);

/**
 * Checks whether the Admin Code functionality is supported by the application.
 *
 * @return true if Admin Code is supported
 */
bool cc_user_credential_get_admin_code_supported(void);

/**
 * Checks whether the Admin Code Deactivate functionality is supported by the application.
 *
 * @return true if Admin Code Deactivate is supported
 */
bool cc_user_credential_get_admin_code_deactivate_supported(void);

#endif /* CC_USER_CREDENTIAL_CONFIG_API_H */
