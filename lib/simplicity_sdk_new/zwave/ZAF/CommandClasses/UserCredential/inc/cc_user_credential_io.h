/**
 * @file
 * User Credential Command Class I/O abstraction.
 *
 * @copyright 2023 Silicon Laboratories Inc.
 */

#ifndef CC_USER_CREDENTIAL_IO_H
#define CC_USER_CREDENTIAL_IO_H

#include "CC_UserCredential.h"

/**
 * @addtogroup CC
 * @{
 * @addtogroup UserCredential
 * @{
 */

/****************************************************************************/
/*                      EXPORTED TYPES and DEFINITIONS                      */
/****************************************************************************/

typedef enum u3c_db_operation_result_ {
  U3C_DB_OPERATION_RESULT_SUCCESS,         ///< The operation completed succesfully
  U3C_DB_OPERATION_RESULT_ERROR,           ///< An error occurred
  U3C_DB_OPERATION_RESULT_ERROR_IO,        ///< An I/O error occurred
  U3C_DB_OPERATION_RESULT_ERROR_DUPLICATE, ///< Duplicate Entry in table
  U3C_DB_OPERATION_RESULT_FAIL_DNE,        ///< The object does not exist
  U3C_DB_OPERATION_RESULT_FAIL_FULL,       ///< There is no space left for the object
  U3C_DB_OPERATION_RESULT_FAIL_OCCUPIED,   ///< The object already exists
  U3C_DB_OPERATION_RESULT_FAIL_REASSIGN,   ///< The credential is assigned to a different user
  U3C_DB_OPERATION_RESULT_FAIL_IDENTICAL   ///< The new data is identical to the data already stored locally
} u3c_db_operation_result;

/****************************************************************************/
/*                           GENERAL API FUNCTIONS                          */
/****************************************************************************/

/**
 * Resets the User Credential Command Class database to a blank state,
 * removing all existing users, credentials and the admin code.
 */
void CC_UserCredential_factory_reset(void);

/**
 * Initializes the User Credential Command Class database to a consistent state.
 * This function is called automatically by the Z-Wave Application Framework
 * when the application starts.
 */
void CC_UserCredential_init_database(void);

/****************************************************************************/
/*                         USER RELATED API FUNCTIONS                       */
/****************************************************************************/

/**
 * Copies the data for a given User from the database to an arbitrary location
 * in memory.
 *
 * @param[in] unique_identifier Unique Identifier of the requested User
 * @param[out] user The memory location where the User data will be copied,
 *                  or NULL if not requested
 * @param[out] name The memory location where the User name will be copied,
 *                  or NULL if not requested. If not NULL, user cannot be NULL
 *                  either.
 * @return the result of the operation
 */
u3c_db_operation_result CC_UserCredential_get_user(
  uint16_t unique_identifier,
  u3c_user_t * user,
  uint8_t * name
  );

/**
 * Finds the first or next available User Unique Identifier.
 *
 * @param[in] unique_identifier Unique Identifier of the current User, or
 * 0 if the first available User Unique Identifier is requested
 * @return The Unique Identifier of the next User if available, otherwise 0
 */
uint16_t CC_UserCredential_get_next_user(uint16_t unique_identifier);

u3c_db_operation_result CC_UserCredential_add_user(
  u3c_user_t * user,
  uint8_t * name
  );

/**
 * Modifies a user in the database.
 *
 * @param[in] user Pointer to the user's new metadata
 * @param[in] name Pointer to the user's new name (set to NULL to keep old name)
 */
u3c_db_operation_result CC_UserCredential_modify_user(
  u3c_user_t * user,
  uint8_t * name
  );

u3c_db_operation_result CC_UserCredential_delete_user(
  uint16_t user_unique_identifier
  );

/****************************************************************************/
/*                      CREDENTIAL RELATED API FUNCTIONS                    */
/****************************************************************************/

/**
 * Copies the data for a given Credential from the database to an arbitrary
 * location in memory.
 *
 * @param[in] user_unique_identifier Unique Identifier of the Credential's
 *                                   associated User (0 matches any UUID)
 * @param[in] credential_type Type of the requested Credential
 * @param[in] credential_slot Slot of the requested Credential
 * @param[out] credential_metadata The memory location where the Credential
 *                                 metadata will be copied, or NULL if not
 *                                 requested
 * @param[out] credential_data The memory location where the Credential data
 *                             will be copied, or NULL if not requested
 * @return the result of the operation
 */
u3c_db_operation_result CC_UserCredential_get_credential(
  uint16_t user_unique_identifier,
  u3c_credential_type credential_type,
  uint16_t credential_slot,
  u3c_credential_metadata_t * credential_metadata,
  uint8_t * credential_data
  );

/**
 * Finds the first or next available Credential entry for a given User or
 * Credential Type (set either one of these parameters to 0 to disable
 * filtering by that property).
 * Set credential_slot to 0 to get the first available Credential matching the
 * filter criteria.
 *
 * @param[in] user_unique_identifier Unique Identifier of the Credential's
 *                                   associated User (0 matches any UUID)
 * @param[in] credential_type Type of the current Credential (0 matches any
 *                            Credential Type)
 * @param[in] credential_slot Slot of the current Credential (0 to get the first
 *                            available Credential)
 * @param[out] next_credential_type Type of the next available Credential, if
 *                                  available
 * @param[out] next_credential_slot Slot of the next available Credential, if
 *                                  available
 * @return true if a subsequent Credential matching the criteria is found
 */
bool CC_UserCredential_get_next_credential(
  uint16_t user_unique_identifier,
  u3c_credential_type credential_type,
  uint16_t credential_slot,
  u3c_credential_type * next_credential_type,
  uint16_t * next_credential_slot
  );

/**
 * Adds a new credential to the database.
 *
 * @param[in] credential Pointer to the credential to be added
 * @return The result of the operation
 */
u3c_db_operation_result CC_UserCredential_add_credential(
  u3c_credential_t * credential
  );

u3c_db_operation_result CC_UserCredential_modify_credential(
  u3c_credential_t * credential
  );

/**
 * Deletes a Credential.
 *
 * @param[in] user_unique_identifier Unique Identifier of the Credential's
 *                                   associated User (0 matches any UUID)
 * @param[in] credential_type Type of the current Credential
 * @param[in] credential_slot Slot of the current Credential
 * @return the result of the operation
 */
u3c_db_operation_result CC_UserCredential_delete_credential(
  u3c_credential_type credential_type,
  uint16_t credential_slot
  );

/**
 * Moves a credential from one slot to another and/or assigns a credential from
 * one User Unique Identifier to another one.
 *
 * @param[in] credential_type Type of the credential
 * @param[in] source_credential_slot Slot of the credential to be moved
 * @param[in] destination_user_uid The new desired UUID to assign the credential to
 * @param[in] destination_credential_slot The new desired slot number for the credential
 */
u3c_db_operation_result CC_UserCredential_move_credential(
  u3c_credential_type credential_type,
  uint16_t source_credential_slot,
  uint16_t destination_user_uid,
  uint16_t destination_credential_slot
  );

/****************************************************************************/
/*                      ADMIN CODE RELATED API FUNCTIONS                    */
/****************************************************************************/

/**
 * Retrieves the admin code information from the node. Can be used to ensure node
 * supports the functionality.
 *
 * @param[out] code Admin Code information. If Admin Code is not supported,
 * the struct will be fully zeroed out upon return.
 * @return U3C_DB_OPERATION_SUCCESS if successful
 * @return U3C_DB_OPERATION_ERROR otherwise
 */
u3c_db_operation_result CC_UserCredential_get_admin_code_info(u3c_admin_code_metadata_t *code);

/**
 * Sets the admin code on the node, if supported.
 *
 * @param[in] code Admin Code information to apply to node
 * @return U3C_DB_OPERATION_ERROR_DUPLICATE if Admin Code is a duplicate of an
 * existing PIN Code or Admin Code
 * @return U3C_OPERATION_SUCCESS if successful
 * @return U3C_OPERATION_ERROR otherwise
 */
u3c_db_operation_result CC_UserCredential_set_admin_code(u3c_admin_code_metadata_t *code);

/**
 * @}
 * @}
 */

#endif /* CC_USER_CREDENTIAL_IO_H */
