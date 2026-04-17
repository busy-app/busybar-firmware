/**
 * @file
 * @brief Checksum command handlers for Command Class User Credential.
 *
 * @details This module provides functions to handle checksum-related operations
 * for credentials.
 *
 * @copyright 2023 Silicon Laboratories Inc.
 */

#ifndef CC_USER_CREDENTIAL_HANDLERS_CHECKSUM_H
#define CC_USER_CREDENTIAL_HANDLERS_CHECKSUM_H

#include "CC_UserCredential.h"
#include "zaf_transport_tx.h"

/**
 * @brief Handle All Users Checksum Get command.
 *
 * @param[in] input Pointer to the structure containing the input data for the handler.
 * @return The result of the attempt to handle the command.
 */
received_frame_status_t CC_UserCredential_AllUsersChecksumGet_handler(
  const cc_handler_input_t * input
  );

/**
 * @brief Handle User Checksum Get command.
 *
 * @param[in] input Pointer to the structure containing the input data for the handler.
 * @return The result of the attempt to handle the command.
 */
received_frame_status_t CC_UserCredential_UserChecksumGet_handler(
  const cc_handler_input_t * input
  );

/**
 * @brief Handle Credential Checksum Get command.
 *
 * @param[in] input Pointer to the structure containing the input data for the handler.
 * @return The result of the attempt to handle the command.
 */
received_frame_status_t CC_UserCredential_CredentialChecksumGet_handler(
  const cc_handler_input_t * input
  );

#endif // CC_USER_CREDENTIAL_HANDLERS_CHECKSUM_H
