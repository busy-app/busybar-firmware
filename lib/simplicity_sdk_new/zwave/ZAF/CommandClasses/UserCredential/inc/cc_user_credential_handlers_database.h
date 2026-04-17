/**
 * @file
 * @brief User Credential database related handlers.
 * @details Command handlers for reading, adding, modifying and removing
 * users and credentials stored in the database.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2023 Silicon Laboratories Inc.
 */

#include "cc_user_credential_tx.h"

/**
 * Called upon receiving a User Set frame.
 *
 * @param[in] input Details of the incoming frame
 */
received_frame_status_t CC_UserCredential_UserSet_handler(
  cc_handler_input_t * input);

/**
 * Called upon receiving a User Get frame.
 *
 * @param[in] input Details of the incoming frame
 */
received_frame_status_t CC_UserCredential_UserGet_handler(
  cc_handler_input_t * input);

/**
 * Called upon receiving a Credential Set frame.
 *
 * @param[in] input Details of the incoming frame
 * @param[in] p_credential Pointer to the incoming credential data
 * @param[in] p_rx_options Pointer to the details of the received frame
 */
received_frame_status_t CC_UserCredential_CredentialSet_handler(
  u3c_operation_type_t operation_type,
  u3c_credential_t * p_credential,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options);

/**
 * Called upon receiving a Credential Get frame.
 *
 * @param[in] input Details of the incoming frame
 * @param[in] p_metadata Pointer to the requested credential's details
 * @param[in] p_rx_options Pointer to the details of the received frame
 */
bool CC_UserCredential_CredentialGet_handler(
  cc_handler_input_t * input,
  u3c_credential_metadata_t * p_metadata,
  u3c_credential_report_tx_callback_t callback
  );
