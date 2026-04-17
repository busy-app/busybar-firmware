/**
 * @file
 * @brief Extended operations for User Credential Command Class.
 *
 * @copyright 2023 Silicon Laboratories Inc.
 *
 * @details All of the functions in this module following the naming pattern
 * CC_UserCredential_<operation>_<object>_and_report are meant to process a
 * request to update the User Credential database and notify the Z-Wave network
 * of the result of the operation.
 * Generally, if the incoming request does not conform to the specification, it
 * will be discarded without any notification. Otherwise, incoming data will be
 * checked against the current state of the database.
 * If the requested operation can be performed and results in a new state,
 * the initiator of the request and the nodes in the Lifeline group will be
 * notified; otherwise, only the initiator will be notified.
 */

#ifndef CC_USER_CREDENTIAL_OPERATIONS_H
#define CC_USER_CREDENTIAL_OPERATIONS_H

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/

#include "cc_user_credential_io.h"

/**
 * @addtogroup CC
 * @{
 * @addtogroup UserCredential
 * @{
 */

/****************************************************************************/
/*                              API FUNCTIONS                               */
/****************************************************************************/

/**
 * @brief Tries to add a new user to the database and reports the result.
 *
 * @param[in] p_user Pointer to the new user's metadata
 * @param[in] p_name Pointer to the new user's name
 * @param[in] p_rx_options Pointer to the details of the received frame
 * @return The result of the operation
 */
u3c_db_operation_result CC_UserCredential_add_user_and_report(
  u3c_user_t * p_user,
  uint8_t * p_name,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  );

/**
 * @brief Tries to modify a user in the database and reports the result.
 *
 * @param[in] p_user Pointer to the user's new metadata
 * @param[in] p_name Pointer to the new user's new name
 * @param[in] p_rx_options Pointer to the details of the received frame
 * @return The result of the operation
 */
u3c_db_operation_result CC_UserCredential_modify_user_and_report(
  u3c_user_t * p_user, uint8_t * p_name, RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  );

/**
 * @brief Tries to delete a user from the database and reports the result.
 *
 * @param[in] uuid The target user's unique identifier or 0 to delete all users.
 * @param[in] p_rx_options Pointer to the details of the received frame
 * @return The result of the operation
 */
u3c_db_operation_result CC_UserCredential_delete_user_and_report(
  uint16_t uuid, RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  );

/**
 * @brief Tries to add a new credential to the database and reports the result.
 *
 * @param[in] p_credential Pointer to the new credential
 * @param[in] p_rx_options Pointer to the details of the received frame
 * @return The result of the operation
 */
u3c_db_operation_result CC_UserCredential_add_credential_and_report(
  u3c_credential_t * p_credential, RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  );

/**
 * @brief Tries to modify a credential in the database and reports the result.
 *
 * @param[in] p_credential Pointer to the credential with the new desired data
 * @param[in] p_rx_options Pointer to the details of the received frame
 * @return The result of the operation
 */
u3c_db_operation_result CC_UserCredential_modify_credential_and_report(
  u3c_credential_t * p_credential, RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  );

/**
 * @brief Tries to delete a credential from the database and reports the result.
 *
 * @param[in] uuid The UUID that the credentials are assigned to (only used for
 *                 bulk operations), 0 to disable filtering by user
 * @param[in] credential_type Type of the target credential
 * @param[in] credential_slot Slot of the target credential, or 0 for all credentials
 * @param[in] p_rx_options Pointer to the details of the received frame
 * @return The result of the operation
 */
u3c_db_operation_result CC_UserCredential_delete_credential_and_report(
  uint16_t uuid, u3c_credential_type credential_type,
  uint16_t credential_slot, RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  );

/**
 * @brief Assigns a credential to a different user
 *
 * @param[in] credential_type Type of the credential
 * @param[in] credential_slot Slot of the credential
 * @param[in] destination_uuid New desired UUID to assign the credential to
 * @param[in] p_rx_options Pointer to the details of the received frame
 * @return The result of the operation
 */
u3c_db_operation_result CC_UserCredential_move_credential_and_report(
  u3c_credential_type credential_type,
  uint16_t credential_slot, uint16_t destination_uuid,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  );

/**
 * @brief Sends a USER_CREDENTIAL_ASSOCIATION_REPORT frame.
 *
 * @deprecated Use CC_UserCredential_AssociationReport_tx instead.
 *
 * @param[in] p_source_metadata Source credential metadata.
 * @param[in] p_destination_metadata Destination credential metadata.
 * @param[in] status Status of association set action.
 * @param[in] p_rx_options Pointer to the details of the received frame.
 * @return true if the frame was enqueued succesfully.
 */
bool CC_UserCredential_send_association_report(
  u3c_credential_metadata_t const * const p_source_metadata,
  u3c_credential_metadata_t const * const p_destination_metadata,
  u3c_user_credential_association_report_status_t const status,
  RECEIVE_OPTIONS_TYPE_EX * const p_rx_options
  );

/**
 * Deletes all Credentials of a given type for a User.
 *
 * @param[in] user_uid    User Unique Identifier
 * @param[in] filter_type The type of Credentials to delete, set this to
 *                        CREDENTIAL_TYPE_NONE to delete every Credential
 * @return true if one or more Credentials were deleted, false otherwise
 */
bool CC_UserCredential_delete_all_credentials_of_type(
  uint16_t uuid,
  u3c_credential_type filter_type
  );

/**
 * @}
 * @}
 */

#endif /* CC_USER_CREDENTIAL_OPERATIONS_H */
