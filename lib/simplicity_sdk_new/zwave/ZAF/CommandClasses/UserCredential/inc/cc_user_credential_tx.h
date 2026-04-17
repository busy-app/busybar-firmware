/**
 * @file
 * @brief User Credential Unsolicited Transmission Functions.
 * @details Functions for transmitting different unsolicited
 * User Credential Command Class reports.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2023 Silicon Laboratories Inc.
 */

#ifndef CC_USER_CREDENTIAL_TX_H
#define CC_USER_CREDENTIAL_TX_H

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/

#include "CC_UserCredential.h"
#include "cc_user_credential_tx.h"

/****************************************************************************/
/*                          CONSTANTS and TYPEDEFS                          */
/****************************************************************************/

/// Maximum number of items in a Credential Usage Data Notification
#define CREDENTIAL_USAGE_MAX_ITEMS CREDENTIAL_RULE_TRIPLE

/**
 * Maximum size for Credential Usage Data Notification Event/State parameters
 *
 * UUID (16 bits) | Credential Count (8 bits)
 * Max items * (Credential Type (8 bits) + Credential Slot (16 bits))
 */
#define CREDENTIAL_USAGE_DATA_MAX_SIZE (                                       \
    sizeof(uuid) + sizeof(credential_count)                                    \
    + (CREDENTIAL_USAGE_MAX_ITEMS * (1 + sizeof(p_credential_metadata->slot))) \
    )

typedef void (*u3c_credential_report_tx_callback_t)(
  const u3c_credential_report_type_t report_type,
  const u3c_credential_t * const p_credential,
  const u3c_credential_type next_credential_type,
  const uint16_t next_credential_slot,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  );

/****************************************************************************/
/*                              API FUNCTIONS                               */
/****************************************************************************/

/**
 * Checks whether a frame was initiated locally.
 *
 * @param[in] p_rx_options Pointer to the properties of the incoming frame
 *
 * @return true if the frame was initiated locally.
 */
bool is_rx_frame_initiated_locally(const RECEIVE_OPTIONS_TYPE_EX * p_rx_options);

/**
 * Fills the frame info to properly handle locally initiated operations.
 *
 * @param[in] p_rx_options Pointer to the properties of the incoming frame
 */
void fill_rx_frame_with_local(RECEIVE_OPTIONS_TYPE_EX * p_rx_options);

/**
 * Sends one or more User Report frames to the appropriate destinations
 * depending on the type of the report
 *
 * @param[in] report_type  The type of report to send
 * @param[in] p_user       Pointer to the User metadata
 * @param[in] p_name       Pointer to the User Name
 * @param[in] next_uuid    The UUID of the next User
 * @param[in] p_rx_options Pointer to the properties of the incoming frame
 */
void CC_UserCredential_UserReport_tx(
  const u3c_user_report_type_t report_type,
  const u3c_user_t * const p_user,
  const uint8_t * const p_name,
  const uint16_t next_uuid,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  );

/**
 * Sends one or more Credential Report frames to the appropriate destinations
 * depending on the type of the report
 *
 * @param[in] report_type          The type of report to send
 * @param[in] p_credential         Pointer to the metadata and content of the credential
 * @param[in] next_credential_type The type of the next credential
 * @param[in] next_credential_slot The slot number of the next credential
 * @param[in] p_rx_options         Pointer to the properties of the incoming frame
 */
void CC_UserCredential_CredentialReport_tx(
  const u3c_credential_report_type_t report_type,
  const u3c_credential_t * p_credential,
  const u3c_credential_type next_credential_type,
  const uint16_t next_credential_slot,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  );

/**
 * @brief Translate admin code data to Report command and send
 * to the original source per the RX options.
 *
 * @param data pointer to Admin Code metadata retrieved from the database.
 * @param rx_options Information about the source of the original command.
 */
void CC_UserCredential_AdminCodeReport_tx(
  const u3c_admin_code_metadata_t * const data,
  RECEIVE_OPTIONS_TYPE_EX * rx_options
  );

/**
 * @brief Transmit All Users Checksum Report.
 *
 * @details Sends a report containing the checksum of all users and their
 * associated credentials.
 *
 * @param[in] checksum     The calculated checksum for all users.
 * @param[in] p_rx_options Pointer to the structure containing the received frame's details.
 */
void CC_UserCredential_AllUsersChecksumReport_tx(
  uint16_t checksum,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  );

/**
 * @brief Transmit User Checksum Report.
 *
 * @details Sends a report containing the checksum of a specific user and their
 * associated credentials.
 *
 * @param[in] uuid         The unique identifier of the user.
 * @param[in] checksum     The calculated checksum for the user.
 * @param[in] p_rx_options Pointer to the structure containing the received frame's details.
 */
void CC_UserCredential_UserChecksumReport_tx(
  uint16_t uuid,
  uint16_t checksum,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  );

/**
 * @brief Transmit Credential Checksum Report.
 *
 * @details Sends a report containing the checksum of all credentials of a specific type.
 *
 * @param[in] type         The type of the credential.
 * @param[in] checksum     The calculated checksum for the credential.
 * @param[in] p_rx_options Pointer to the structure containing the received frame's details.
 */
void CC_UserCredential_CredentialChecksumReport_tx(
  u3c_credential_type type,
  uint16_t checksum,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options
  );

/**
 * Sends a Credential Learn Status Report frame.
 *
 * @param[in] status          Status to report
 * @param[in] p_target        Pointer to the identifying details of the credential
 * @param[in] steps_remaining Number of remaining steps in the current learn process
 * @param[in] p_rx_options    Pointer to the details of the frame that prompted the process
 *                            (provide a blank struct if initiated locally)
 */
void CC_UserCredential_CredentialLearnStatusReport_tx(
  const u3c_credential_learn_status_t status,
  const u3c_credential_identifier_t * const p_target,
  const uint8_t steps_remaining,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options);

/**
 * Sends a Notification Report frame with Event/State parameters containing the
 * Credential Usage Data
 *
 * @param[in] notification_event    The type of notification to send
 * @param[in] uuid                  The unique identifier of the user
 * @param[in] credential_count      The number of credentials in the notification
 * @param[in] p_credential_metadata Pointer to the metadata of the credentials
 *
 * @returns Whether the transmission was successful
 */
JOB_STATUS CC_UserCredential_UsageNotification_tx(
  const uint8_t notification_event,
  const uint16_t uuid,
  const uint8_t credential_count,
  const u3c_credential_metadata_t * const p_credential_metadata
  );

/**
 * @brief Sends a Credential Association Report frame.
 *
 * @param[in] p_source_metadata Source credential metadata.
 * @param[in] p_destination_metadata Destination credential metadata.
 * @param[in] status Status of association set action.
 * @param[in] p_rx_options Pointer to the details of the received frame.
 * @return true if the frame was enqueued succesfully.
 */
void CC_UserCredential_AssociationReport_tx(
  u3c_credential_metadata_t const * const p_source_metadata,
  u3c_credential_metadata_t const * const p_destination_metadata,
  u3c_user_credential_association_report_status_t const status,
  RECEIVE_OPTIONS_TYPE_EX * const p_rx_options
  );

#endif /* CC_USER_CREDENTIAL_TX_H */
