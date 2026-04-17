/**
 * @file
 * @brief User Credential Admin PIN Code handlers.
 * @details Command handlers for the Admin PIN Code feature.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2023 Silicon Laboratories Inc.
 */

#ifndef CC_USER_CREDENTIAL_HANDLERS_ADMIN_CODE_H
#define CC_USER_CREDENTIAL_HANDLERS_ADMIN_CODE_H

#include "CC_UserCredential.h"

received_frame_status_t CC_UserCredential_AdminCodeSet_handler(
  u3c_admin_code_metadata_t *data,
  RECEIVE_OPTIONS_TYPE_EX *rx_options
  );

received_frame_status_t CC_UserCredential_AdminCodeGet_handler(
  cc_handler_input_t *input
  );

#endif /* CC_USER_CREDENTIAL_HANDLERS_ADMIN_CODE_H */
