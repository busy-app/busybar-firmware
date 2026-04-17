/**
 * @file
 * @brief User Credential Admin PIN Code handlers.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2023 Silicon Laboratories Inc.
 */

#include "cc_user_credential_config_api.h"
#include "cc_user_credential_handlers_admin_code.h"
#include "cc_user_credential_io.h"
#include "cc_user_credential_tx.h"
#include "cc_user_credential_validation.h"

ZW_WEAK received_frame_status_t CC_UserCredential_AdminCodeSet_handler(u3c_admin_code_metadata_t *data, RECEIVE_OPTIONS_TYPE_EX *rx_options)
{
  received_frame_status_t status = RECEIVED_FRAME_STATUS_FAIL;
  if (data->code_length >= AC_MIN_LENGTH && !validate_admin_pin_code(data)) {
    // Something went wrong - find out what. This will not run an IO operation and instead
    // will transmit a report.
    switch (data->result) {
      // Error cases
      case ADMIN_CODE_OPERATION_RESULT_INTERNAL_DUPLICATE_AC:
        data->result = ADMIN_CODE_OPERATION_RESULT_UNMODIFIED;
        break;
      case ADMIN_CODE_OPERATION_RESULT_FAIL_DUPLICATE_CRED:
      case ADMIN_CODE_OPERATION_RESULT_FAIL_MANUF_RULE:
        break;
      default:
        return status;
    }
    /**
     * If nothing goes wrong, run the IO operation.
     */
  } else {
    u3c_db_operation_result result = CC_UserCredential_set_admin_code(data);
    if (result != U3C_DB_OPERATION_RESULT_SUCCESS) {
      data->result = ADMIN_CODE_OPERATION_RESULT_ERROR_NODE;
    } else {
      status = RECEIVED_FRAME_STATUS_SUCCESS;
    }
  }
  /**
   * Result of the operation(s) trickles down to here. This contains
   * the result within the metadata struct.
   */
  CC_UserCredential_AdminCodeReport_tx(data, rx_options);
  return status;
}

ZW_WEAK received_frame_status_t CC_UserCredential_AdminCodeGet_handler(cc_handler_input_t *input)
{
  received_frame_status_t status = RECEIVED_FRAME_STATUS_FAIL;
  /**
   * CC:0083.01.1B.13.001 - if AC is unsupported, command may be ignored.
   */
  if (cc_user_credential_get_admin_code_supported()) {
    /**
     * Result code will come from application layer handler with the exception
     * of an unspecified node error.
     */
    u3c_admin_code_metadata_t data = { 0 };

    if (CC_UserCredential_get_admin_code_info(&data) != U3C_DB_OPERATION_RESULT_SUCCESS) {
      data.result = ADMIN_CODE_OPERATION_RESULT_ERROR_NODE;
    }
    CC_UserCredential_AdminCodeReport_tx(&data, input->rx_options);
    status = RECEIVED_FRAME_STATUS_SUCCESS;
  } else {
    status = RECEIVED_FRAME_STATUS_NO_SUPPORT;
  }

  return status;
}
