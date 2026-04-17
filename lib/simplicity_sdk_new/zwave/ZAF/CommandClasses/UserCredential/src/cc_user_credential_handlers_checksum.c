/**
 * @file
 * @brief User Credential Checksum command handlers.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2023 Silicon Laboratories Inc.
 */

#include "cc_user_credential_handlers_checksum.h"
#include "CC_UserCredential.h"
#include "cc_user_credential_config_api.h"
#include "cc_user_credential_io_config.h"
#include "cc_user_credential_io.h"
#include "cc_user_credential_tx.h"
#include "zw_crc.h" // CC:0083.01.15.11.000 & CC:0083.01.17.11.000 & CC:0083.01.19.11.001

/****************************************************************************/
/*                             PRIVATE FUNCTIONS                            */
/****************************************************************************/

static void calculate_credentials_checksum_for_uuid(
  const uint16_t uuid, uint16_t * checksum)
{
  u3c_credential_type type = CREDENTIAL_TYPE_NONE;
  uint16_t slot = 0;
  u3c_credential_metadata_t existing_metadata = { 0 };

  /**
   * When more than one Credential of the given uuid exists,
   * we must concatenate the data of each Credential and calculate the CRC-16
   * CC:0083.01.17.11.007
   * If there is no Credential data set for a User Unique Identifier, the checksum MUST NOT be modified
   * other than for the User Unique Identifier data.
   * CC:0083.01.17.11.005
   */
  while (
    CC_UserCredential_get_next_credential(
      uuid, type, slot, &type, &slot)
    ) {
    uint8_t e_data[U3C_BUFFER_SIZE_CREDENTIAL_DATA] = { 0 };
    uint8_t slot_msb = 0;
    uint8_t slot_lsb = 0;

    CC_UserCredential_get_credential(
      uuid, type, slot, &existing_metadata, e_data);

    slot_msb = (slot >> 8);
    slot_lsb = slot & 0xFF;
    /**
     * Credential Type (8 bits) | Credential Slot (16 bits) |
     * Credential Length (8 bits) | Credential Data (Credential Length bytes)
     * CC:0083.01.17.11.002
     */
    *checksum = zw_crc_check_crc16(*checksum, (uint8_t *)&type, 1);
    *checksum = zw_crc_check_crc16(*checksum, &slot_msb, 1);
    *checksum = zw_crc_check_crc16(*checksum, &slot_lsb, 1);
    *checksum = zw_crc_check_crc16(*checksum, &existing_metadata.length, 1);
    *checksum = zw_crc_check_crc16(*checksum, e_data, existing_metadata.length);
  }
}

/****************************************************************************/
/*                             PUBLIC FUNCTIONS                             */
/****************************************************************************/

ZW_WEAK received_frame_status_t CC_UserCredential_AllUsersChecksumGet_handler(const cc_handler_input_t * input)
{
  /**
   * This command MUST be ignored by a node advertising no support for the All Users Checksum
   * functionality in the User Capabilities Report Command.
   * CC:0083.01.14.11.000
   */
  if (!cc_user_credential_is_all_users_checksum_supported()) {
    return RECEIVED_FRAME_STATUS_NO_SUPPORT;
  }

  u3c_user_t user = { 0 };
  uint8_t name[UINT8_MAX] = { 0 };
  uint8_t uuid_msb = 0;
  uint8_t uuid_lsb = 0;
  bool user_is_available = false;

  uint16_t checksum = ZW_CRC_INITIAL_VALUE; // CC:0083.01.15.11.000
  uint16_t user_uid = CC_UserCredential_get_next_user(0);

  while (user_uid) {
    user_is_available = true;
    if (CC_UserCredential_get_user(user_uid, &user, name) != U3C_DB_OPERATION_RESULT_SUCCESS) {
      // Driver error or database corruption
      return RECEIVED_FRAME_STATUS_FAIL;
    } else {
      /**
       * User Unique Identifier (16 bits) | User Type (8 bits) | User Active State (8 bits) |
       * Credential Rule (8 bits) | User Name Encoding (8 bits) | User Name Length (8 bits) |
       * User Name (User Name Length bytes)
       * CC:0083.01.15.11.001
       * CC:0083.01.15.11.002
       */
      uuid_msb = (user_uid >> 8);
      uuid_lsb = user_uid & 0xFF;
      checksum = zw_crc_check_crc16(checksum, &uuid_msb, 1);
      checksum = zw_crc_check_crc16(checksum, &uuid_lsb, 1);
      checksum = zw_crc_check_crc16(checksum, (uint8_t*)&user.type, 1);
      checksum = zw_crc_check_crc16(checksum, (uint8_t*)&user.active, 1);
      checksum = zw_crc_check_crc16(checksum, (uint8_t*)&user.credential_rule, 1);
      checksum = zw_crc_check_crc16(checksum, (uint8_t*)&user.name_encoding, 1);
      checksum = zw_crc_check_crc16(checksum, &user.name_length, 1);
      checksum = zw_crc_check_crc16(checksum, name, user.name_length);

      calculate_credentials_checksum_for_uuid(user_uid, &checksum);
    }
    user_uid = CC_UserCredential_get_next_user(user_uid);
  }

  /**
   * If there is no Users data (and thus no Credentials data) set at the node at all,
   * the checksum MUST be set to 0x0000.
   * CC:0083.01.15.11.006
   */
  checksum = user_is_available ? checksum : 0;

  /**
   * All Users Checksum Report command must be returned if this functionality is supported.
   * CC:0083.01.14.11.001
   */
  CC_UserCredential_AllUsersChecksumReport_tx(checksum, input->rx_options);

  return RECEIVED_FRAME_STATUS_SUCCESS;
}

ZW_WEAK received_frame_status_t CC_UserCredential_UserChecksumGet_handler(const cc_handler_input_t * input)
{
  /**
   * This command MUST be ignored by a node advertising no support for the User Checksum functionality
   * in the User Capabilities Report Command.
   * CC:0083.01.16.11.000
   */
  if (!cc_user_credential_is_user_checksum_supported()) {
    return RECEIVED_FRAME_STATUS_NO_SUPPORT;
  }

  u3c_user_t user = { 0 };
  uint8_t name[UINT8_MAX] = { 0 };

  uint16_t uuid = (uint16_t)(input->frame->ZW_UserChecksumGetFrame.userUniqueIdentifier1 << 8
                             | input->frame->ZW_UserChecksumGetFrame.userUniqueIdentifier2);

  uint16_t checksum = ZW_CRC_INITIAL_VALUE; // CC:0083.01.17.11.000

  u3c_db_operation_result result = CC_UserCredential_get_user(uuid, &user, name);

  if (result == U3C_DB_OPERATION_RESULT_SUCCESS) {
    /**
     * User Type (8 bits) | User Active State (8 bits) | Credential Rule (8 bits) | User Name
     * Encoding (8 bits) | User Name Length (8 bits) | User Name (User Name Length bytes)
     * CC:0083.01.17.11.001
     * CC:0083.01.17.11.002
     */
    checksum = zw_crc_check_crc16(checksum, (uint8_t*)&user.type, 1);
    checksum = zw_crc_check_crc16(checksum, (uint8_t*)&user.active, 1);
    checksum = zw_crc_check_crc16(checksum, (uint8_t*)&user.credential_rule, 1);
    checksum = zw_crc_check_crc16(checksum, (uint8_t*)&user.name_encoding, 1);
    checksum = zw_crc_check_crc16(checksum, &user.name_length, 1); // CC:0083.01.17.11.004
    checksum = zw_crc_check_crc16(checksum, name, user.name_length);

    calculate_credentials_checksum_for_uuid(uuid, &checksum);
  } else if (result == U3C_DB_OPERATION_RESULT_FAIL_DNE) {
    /**
     * If there is no User data (and thus no Credentials data) set at the node for a User Unique Identifier,
     * the checksum MUST be set to 0x0000.
     * CC:0083.01.17.11.006
     */
    checksum = 0;
  } else {
    // Driver error or database corruption
    return RECEIVED_FRAME_STATUS_FAIL;
  }

  /**
   * User Checksum Report command must be returned if this functionality is supported.
   * CC:0083.01.16.11.001
   */
  CC_UserCredential_UserChecksumReport_tx(uuid, checksum, input->rx_options);

  return RECEIVED_FRAME_STATUS_SUCCESS;
}

ZW_WEAK received_frame_status_t CC_UserCredential_CredentialChecksumGet_handler(const cc_handler_input_t * input)
{
  /**
   * This command MUST be ignored by a node advertising no support for the Credential Checksum
   * functionality in the Credential Capabilities Report Command.
   * CC:0083.01.18.11.000
   */
  if (!cc_user_credential_is_credential_checksum_supported()) {
    return RECEIVED_FRAME_STATUS_NO_SUPPORT;
  }

  u3c_credential_type searched_type = input->frame->ZW_CredentialChecksumGetFrame.credentialType;
  uint16_t next_slot = 0;
  uint8_t next_slot_msb = 0;
  uint8_t next_slot_lsb = 0;
  u3c_credential_type next_type = CREDENTIAL_TYPE_NONE;
  uint16_t checksum = ZW_CRC_INITIAL_VALUE; // CC:0083.01.19.11.001
  bool credential_is_available = false;

  u3c_credential_metadata_t metadata = { 0 };
  uint8_t e_data[U3C_BUFFER_SIZE_CREDENTIAL_DATA] = { 0 };

  while (CC_UserCredential_get_next_credential(0, searched_type, next_slot, &next_type, &next_slot)) {
    /**
     * Credential Slot (16 bits) | Credential Length (8 bits) |
     * Credential Data (Credential Length bytes)
     * CC:0083.01.19.11.002
     * CC:0083.01.19.11.003
     */
    credential_is_available = true;
    if (next_type != searched_type) {
      break;
    }

    if (CC_UserCredential_get_credential(0, searched_type, next_slot, &metadata, e_data) == U3C_DB_OPERATION_RESULT_SUCCESS) {
      next_slot_msb = next_slot >> 8;
      next_slot_lsb = next_slot & 0xFF;

      checksum = zw_crc_check_crc16(checksum, &next_slot_msb, 1);
      checksum = zw_crc_check_crc16(checksum, &next_slot_lsb, 1);
      checksum = zw_crc_check_crc16(checksum, &metadata.length, 1);
      checksum = zw_crc_check_crc16(checksum, (uint8_t*)e_data, metadata.length);
    } else {
      // Driver error or database corruption
      return RECEIVED_FRAME_STATUS_FAIL;
    }
  }

  /**
   * If there is no Credentials data set at the node for a Credential Type, the checksum MUST be
   * set to 0x0000.
   * CC:0083.01.19.11.006
   */
  checksum = credential_is_available ? checksum : 0;

  /**
   * Credential Checksum Report command must be returned if this functionality is supported.
   * CC:0083.01.18.11.001
   */
  CC_UserCredential_CredentialChecksumReport_tx(searched_type, checksum, input->rx_options);

  return RECEIVED_FRAME_STATUS_SUCCESS;
}
