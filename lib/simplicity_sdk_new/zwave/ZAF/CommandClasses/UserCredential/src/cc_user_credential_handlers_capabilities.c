/**
 * @file
 * @brief User Credential Capabilities command handlers.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2023 Silicon Laboratories Inc.
 */

#include "cc_user_credential_handlers_capabilities.h"
#include "cc_user_credential_config_api.h"

/// List of known valid User Type values
static uint8_t u3c_user_type_numeric_value[] = {
  0x00, // General User
  0x03, // Programming User
  0x04, // Non-Access User
  0x05, // Duress User
  0x06, // Disposable User
  0x07, // Expiring User
  0x09, // Remote Only User
};

ZW_WEAK void CC_UserCredential_UserCapabilitiesGet_handler(cc_handler_output_t * output)
{
  ZW_USER_CAPABILITIES_REPORT_2BYTE_FRAME * pReport = &output->frame->ZW_UserCapabilitiesReport2byteFrame;
  uint16_t max_user_unique_identifers =
    cc_user_credential_get_max_user_unique_idenfitiers();
  uint16_t supported_user_types_bit_mask = 0;
  for (uint8_t i = 0; i < sizeof(u3c_user_type_numeric_value); ++i) {
    uint8_t type = u3c_user_type_numeric_value[i];
    supported_user_types_bit_mask |= (cc_user_credential_is_user_type_supported(type) << type);
  }

  pReport->cmdClass = COMMAND_CLASS_USER_CREDENTIAL;
  pReport->cmd = USER_CAPABILITIES_REPORT;

  pReport->numberOfSupportedUserUniqueIdentifiers1 = max_user_unique_identifers >> 8;
  pReport->numberOfSupportedUserUniqueIdentifiers2 = max_user_unique_identifers & 0xFF;

  pReport->supportedCredentialRulesBitMask =
    cc_user_credential_is_credential_rule_supported(CREDENTIAL_RULE_SINGLE) << CREDENTIAL_RULE_SINGLE
      | cc_user_credential_is_credential_rule_supported(CREDENTIAL_RULE_DUAL) << CREDENTIAL_RULE_DUAL
      | cc_user_credential_is_credential_rule_supported(CREDENTIAL_RULE_TRIPLE) << CREDENTIAL_RULE_TRIPLE;

  pReport->maxLengthOfUserName = cc_user_credential_get_max_length_of_user_name();

  // #TODO: Implement support for User Schedule

  pReport->properties1 =
    ((0 << 7) & USER_CAPABILITIES_REPORT_PROPERTIES1_USER_SCHEDULE_SUPPORT_BIT_MASK)
    | ((cc_user_credential_is_all_users_checksum_supported() << 6) & USER_CAPABILITIES_REPORT_PROPERTIES1_ALL_USERS_CHECKSUM_SUPPORT_BIT_MASK)
    | ((cc_user_credential_is_user_checksum_supported() << 5) & USER_CAPABILITIES_REPORT_PROPERTIES1_USER_CHECKSUM_SUPPORT_BIT_MASK)
    | USER_CAPABILITIES_REPORT_PROPERTIES1_UTF_16_ENCODING_SUPPORT_BIT_MASK
    | USER_CAPABILITIES_REPORT_PROPERTIES1_EXTENDED_ASCII_ENCODING_SUPPORT_BIT_MASK
    | USER_CAPABILITIES_REPORT_PROPERTIES1_ASCII_ENCODING_SUPPORT_BIT_MASK;

  pReport->supportedUserTypesBitMaskLength = 2;
  pReport->supportedUserTypesBitMask1 = supported_user_types_bit_mask & 0xFF;
  pReport->supportedUserTypesBitMask2 = supported_user_types_bit_mask >> 8 & 0xFF;
}

/**
 * Called upon receiving a Credential Capabilities Get frame.
 *
 * @param[out] pReport Pointer to the data that will be included in the
 *                     Capabilities Report frame.
 * @param[out] pLength Pointer to the length of the frame.
 */
ZW_WEAK void CC_UserCredential_CredentialCapabilitiesGet_handler(
  cc_handler_output_t * output)
{
  ZW_CREDENTIAL_CAPABILITIES_REPORT_4BYTE_FRAME * pReport = &output->frame->ZW_CredentialCapabilitiesReport4byteFrame;
  uint8_t n_credential_types = cc_user_credential_get_number_of_supported_credential_types();

  pReport->cmdClass = COMMAND_CLASS_USER_CREDENTIAL;
  pReport->cmd = CREDENTIAL_CAPABILITIES_REPORT;
  pReport->properties1 = 0; // Unset all bits.
  pReport->properties1 = (cc_user_credential_is_credential_checksum_supported() << 7); // Bit 7: Credential Checksum Support
  /**
   * Populate admin code support info in report.
   * Bit 6 - Admin Code Supported
   * Bit 5 - Admin Code Deactivation Supported
   */
  pReport->properties1 |= (cc_user_credential_get_admin_code_supported() << 6);
  pReport->properties1 |= (cc_user_credential_get_admin_code_deactivate_supported() << 5);

  pReport->numberOfSupportedCredentialTypes = n_credential_types;

  uint8_t * pArray = (uint8_t *)(&pReport->variantgroup1.credentialType);
  u3c_credential_type i_type = CREDENTIAL_TYPE_NONE;

  /**
   * The frame's variant group will contain arrays with as many elements as
   * there are supported credential types
   */
  for (uint8_t i_array = 0; i_array < n_credential_types; ++i_array) {
    // Find next supported Credential Type
    do {
      ++i_type;
    } while (!cc_user_credential_is_credential_type_supported(i_type));

    // Assemble element of variant group
    bool cl_support = cc_user_credential_is_credential_learn_supported(i_type);
    uint16_t max_credential_slots = cc_user_credential_get_max_credential_slots(i_type);
    uint8_t min_length_of_credential_data = cc_user_credential_get_min_length_of_data(i_type);
    uint8_t max_length_of_credential_data = cc_user_credential_get_max_length_of_data(i_type);
    uint8_t cl_recommended_timeout = cc_user_credential_get_cl_recommended_timeout(i_type);
    uint8_t cl_number_of_steps = cc_user_credential_get_cl_number_of_steps(i_type);
    uint8_t max_hash_length = cc_user_credential_get_max_hash_length(i_type);

    // Credential Type
    *(pArray + i_array) = i_type;

    // Properties1 (bit 7 is CL Support)
    *(pArray + 1 * n_credential_types + i_array) = (cl_support << 7) & 0x80;

    // Number of Supported Credential Slots (MSB, LSB)
    *(pArray + 2 * n_credential_types + 2 * i_array + 0) = max_credential_slots >> 8;
    *(pArray + 2 * n_credential_types + 2 * i_array + 1) = max_credential_slots & 0xFF;

    // Min Length of Credential Data
    *(pArray + 4 * n_credential_types + i_array) = min_length_of_credential_data;

    // Max Length of Credential Data
    *(pArray + 5 * n_credential_types + i_array) = max_length_of_credential_data;

    // Credential Learn Recommended Timeout
    *(pArray + 6 * n_credential_types + i_array) = cl_recommended_timeout;

    // Credential Learn Number of Steps
    *(pArray + 7 * n_credential_types + i_array) = cl_number_of_steps;

    // Maximum Credential Hash Length
    *(pArray + 8 * n_credential_types + i_array) = max_hash_length;
  }

  output->length = 4 + CREDENTIAL_CAPABILITIES_REPORT_VG_SIZE * n_credential_types;
}
