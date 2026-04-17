/**
 * @file
 * @brief User Credential Capabilities command handlers.
 * @details Command handlers that provide capability reports about
 * User Credential functionalities.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2023 Silicon Laboratories Inc.
 */

#ifndef CC_USER_CREDENTIAL_HANDLERS_CAPABILITIES_H
#define CC_USER_CREDENTIAL_HANDLERS_CAPABILITIES_H

#include "CC_UserCredential.h"

/// Size of a Capabilities Report Variant Group for a single Credential Type
#define CREDENTIAL_CAPABILITIES_REPORT_VG_SIZE (                   \
    sizeof(uint8_t) /**< Credential Type */                        \
    + sizeof(uint8_t) /**< Properties1 */                          \
    + sizeof(uint16_t) /**< Credential Slots */                    \
    + sizeof(uint8_t) /**< Minimum Length of Data */               \
    + sizeof(uint8_t) /**< Maximum Length of Data */               \
    + sizeof(uint8_t) /**< Credential Learn Recommended Timeout */ \
    + sizeof(uint8_t) /**< Credential Learn Number of Steps */     \
    + sizeof(uint8_t) /**< Maximum Credential Hash Length */       \
    )

/**
 * Called upon receiving a User Capabilities Get frame.
 *
 * @param[out] pReport Pointer to the data that will be included in the
 *                     Capabilities Report frame.
 */
void CC_UserCredential_UserCapabilitiesGet_handler(
  cc_handler_output_t * output
  );

/**
 * Called upon receiving a Credential Capabilities Get frame.
 *
 * @param[out] pReport Pointer to the data that will be included in the
 *                     Capabilities Report frame.
 * @param[out] pLength Pointer to the length of the frame.
 */
void CC_UserCredential_CredentialCapabilitiesGet_handler(
  cc_handler_output_t * output
  );

#endif /* CC_USER_CREDENTIAL_HANDLERS_CAPABILITIES_H */
