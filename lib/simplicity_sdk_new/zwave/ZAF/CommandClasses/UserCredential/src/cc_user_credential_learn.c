/**
 * @file
 * @brief Credential Learn module for Command Class User Credential.
 *
 * @details The Credential Learn module provides functions to start and cancel
 * the learning process for user credentials.
 *
 * @section credential_learn_power_management User Credential Learn Power Management
 *
 * The User Credential Learn process manages device power state to ensure the device
 * remains awake during credential learning operations:
 *
 * @verbatim
 * User Credential Learn Radio Power Management Timeline:
 *
 * Time:     0sec    30sec    60sec    90sec   120sec   150sec   180sec   210sec   240sec   270sec   300sec
 * State:    |--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|
 *           ^                                                                                   ^
 *           |                                                                                   |
 *   Credential Learn Start                                                             Learn Complete
 *   Update stay-awake timeout                                                          Revoke stay-awake
 *   zpal_radio_update_stay_awake(timeout_seconds)                                     zpal_radio_revoke_stay_awake()
 *           |                                                                                   |
 *           |<------------- Credential Learning Process ------------->|                         |
 *           |                                                         |                         |
 *        Start Credential                                         Complete                     Allow sleep
 *        Learn Mode                                              Learn Mode                    after learn
 *
 * @endverbatim
 *
 * - **Learn Mode Start**: When entering credential learn mode, we update the stay-awake
 *   timeout using zpal_radio_update_stay_awake() with the configured timeout duration.
 *   This ensures the device remains awake during the credential learning window.
 *
 * - **Learning Window**: The stay-awake period covers the entire credential learning
 *   process, allowing the device to receive and process credential data from users
 *   or external systems.
 *
 * - **Learn Mode End**: When credential learning completes (success, timeout, or cancel),
 *   we revoke the stay-awake request using zpal_radio_revoke_stay_awake() to allow
 *   the device to return to its normal sleep/wake cycle.
 *
 * - **Configurable Timeout**: The stay-awake duration is configurable based on the
 *   credential learning timeout requirements for the specific application.
 *
 * @copyright 2023 Silicon Laboratories Inc.
 */

#include "cc_user_credential_learn.h"
#include <string.h>
#include "assert.h"
#include "AppTimer.h"
#include "CC_UserCredential.h"
#include "cc_user_credential_config_api.h"
#include "cc_user_credential_io.h"
#include "cc_user_credential_tx.h"
#include "cc_user_credential_validation.h"
#include "ZAF_Common_interface.h"
#include "zpal_log.h"
#include "zpal_radio.h"

/****************************************************************************/
/*                       PRIVATE TYPES and DEFINITIONS                      */
/****************************************************************************/

#define S_TO_MS 1000 // Seconds to milliseconds, used for timers

// State and timer for the current Credential Learn operation
typedef struct credential_learn_status_t_ {
  bool in_progress;
  u3c_event_data_learn_start_t data;
  RECEIVE_OPTIONS_TYPE_EX rx_options;
  uint8_t total_steps;
  uint8_t remaining_steps;
} credential_learn_status_t;

/****************************************************************************/
/*                             STATIC VARIABLES                             */
/****************************************************************************/

/// Static Credential Learn data
static credential_learn_status_t cl_state;
static SSwTimer cl_timeout_timer;
static zpal_radio_stay_awake_id_t credential_learn_stay_awake_id = 0;

/****************************************************************************/
/*                             PRIVATE FUNCTIONS                            */
/****************************************************************************/

static void credential_learn_timer_timeout_callback(__attribute__((unused)) SSwTimer * pTimer)
{
  zaf_event_distributor_enqueue_cc_event(
    COMMAND_CLASS_USER_CREDENTIAL, CC_USER_CREDENTIAL_EVENT_LEARN_TIMEOUT, NULL
    );
  // CC:0083.01.0F.11.013
  CC_UserCredential_CredentialLearnStatusReport_tx(
    CL_STATUS_TIMEOUT, &cl_state.data.target, 0, &cl_state.rx_options
    );
  // CC:0083.01.0F.11.013
  credential_learn_reset();
}

/****************************************************************************/
/*                             PUBLIC FUNCTIONS                             */
/****************************************************************************/

bool is_credential_learn_in_progress()
{
  return cl_state.in_progress;
}

void credential_learn_reset()
{
  if (!cl_timeout_timer.TimerHandle) {
    AppTimerRegister(&cl_timeout_timer, false, &credential_learn_timer_timeout_callback);
  } else {
    TimerStop(&cl_timeout_timer);
  }
  zpal_radio_revoke_stay_awake(&credential_learn_stay_awake_id);
  memset(&cl_state, 0, sizeof(cl_state));
}

ZW_WEAK received_frame_status_t CC_UserCredential_CredentialLearnStart_handler(
  const u3c_operation_type_t operation_type,
  const u3c_credential_identifier_t * const p_learn_target,
  const uint8_t timeout_seconds,
  RECEIVE_OPTIONS_TYPE_EX * p_rx_options)
{
  if (!p_learn_target || !p_rx_options || operation_type == U3C_OPERATION_TYPE_DELETE) {
    assert(false);
    return RECEIVED_FRAME_STATUS_FAIL;
  }

  if (is_credential_learn_in_progress()) {
    CC_UserCredential_CredentialLearnStatusReport_tx(
      CL_STATUS_ALREADY_IN_PROGRESS, p_learn_target, cl_state.remaining_steps, p_rx_options
      );
    return RECEIVED_FRAME_STATUS_FAIL;
  }

  // Validate that the metadata is correct
  u3c_credential_metadata_t metadata = {
    .uuid = p_learn_target->uuid,
    .type = p_learn_target->type,
    .slot = p_learn_target->slot
  };

  if (
    !validate_associated_uuid(p_learn_target->uuid)
    || !validate_new_credential_metadata(&metadata)
    // CC:0083.01.0F.11.004
    || !cc_user_credential_is_credential_learn_supported(metadata.type)
    ) {
    zaf_event_distributor_enqueue_cc_event(
      COMMAND_CLASS_USER_CREDENTIAL, CC_USER_CREDENTIAL_EVENT_LEARN_FAILED, NULL);
    return RECEIVED_FRAME_STATUS_NO_SUPPORT;
  }

  /**
   * Check whether the requested Add or Modify operation is consistent with the
   * current state of the database
   */
  u3c_db_operation_result result = CC_UserCredential_get_credential(
    metadata.uuid, metadata.type, metadata.slot,
    &metadata, NULL);
  bool is_consistent_with_database = true;
  u3c_credential_learn_status_t status = CL_STATUS_STARTED;
  switch (operation_type) {
    case U3C_OPERATION_TYPE_ADD:
      if (result != U3C_DB_OPERATION_RESULT_FAIL_DNE) {
        status = CL_STATUS_INVALID_ADD_OPERATION_TYPE;
        is_consistent_with_database = false;
      }
      break;
    case U3C_OPERATION_TYPE_MODIFY:
      if (
        result != U3C_DB_OPERATION_RESULT_SUCCESS
        /**
         * The UUID must not be changed via this command.
         * User Credential Association Set should be used instead.
         */
        || metadata.uuid != p_learn_target->uuid
        ) {
        status = CL_STATUS_INVALID_MODIFY_OPERATION_TYPE;
        is_consistent_with_database = false;
      }
      break;
    default:
      assert(false); // Should not be possible due to earlier checks
      return RECEIVED_FRAME_STATUS_FAIL;
  }
  if (!is_consistent_with_database) {
    zaf_event_distributor_enqueue_cc_event(
      COMMAND_CLASS_USER_CREDENTIAL, CC_USER_CREDENTIAL_EVENT_LEARN_FAILED, NULL);
    CC_UserCredential_CredentialLearnStatusReport_tx(
      status, p_learn_target, 0, p_rx_options
      );
    return RECEIVED_FRAME_STATUS_FAIL;
  }

  // Store details of current Credential Learn operation
  memcpy(&cl_state.data.target, p_learn_target, sizeof(cl_state.data.target));
  memcpy(&cl_state.rx_options, p_rx_options, sizeof(RECEIVE_OPTIONS_TYPE_EX));
  cl_state.data.operation_type = operation_type;
  // If no Credential Learn Timeout value is provided, use the recommended value
  cl_state.data.timeout_seconds = timeout_seconds
                                  ? timeout_seconds
                                  : cc_user_credential_get_cl_recommended_timeout(p_learn_target->type);
  cl_state.data.source_id = p_rx_options->sourceNode.nodeId;
  cl_state.in_progress = true;

  // Signal start of process to application
  zaf_event_distributor_enqueue_cc_event(
    COMMAND_CLASS_USER_CREDENTIAL, CC_USER_CREDENTIAL_EVENT_LEARN_START, &cl_state.data
    );

  return RECEIVED_FRAME_STATUS_SUCCESS;
};

ZW_WEAK received_frame_status_t CC_UserCredential_CredentialLearnCancel_handler(
  const cc_handler_input_t * const input)
{
  received_frame_status_t frame_status = RECEIVED_FRAME_STATUS_FAIL;
  u3c_credential_learn_status_t cl_status = CL_STATUS_ALREADY_IN_PROGRESS;

  /**
   * Only allow the initiating node to cancel the operation, and only if it is
   * already in progress
   */
  if (is_credential_learn_in_progress()
      && (input->rx_options->sourceNode.nodeId == cl_state.rx_options.sourceNode.nodeId)
      ) {
    frame_status = RECEIVED_FRAME_STATUS_SUCCESS;
    cl_status = CL_STATUS_ENDED_NOT_DUE_TO_TIMEOUT;
    zaf_event_distributor_enqueue_cc_event(
      COMMAND_CLASS_USER_CREDENTIAL, CC_USER_CREDENTIAL_EVENT_LEARN_CANCEL_REMOTE, NULL
      );
    CC_UserCredential_CredentialLearnStatusReport_tx(
      cl_status, &cl_state.data.target, 0, &cl_state.rx_options
      );
    credential_learn_reset();
  }

  return frame_status;
}

ZW_WEAK void CC_UserCredential_learn_event_handler(
  const uint8_t event, const void * p_data)
{
  switch (event) {
    case CC_USER_CREDENTIAL_EVENT_LEARN_START_REQUEST: {
      /**
       * The application requests to start the Credential Learn process.
       * Proceed only if the node is already included.
       */
      if (ZAF_GetInclusionState() != EINCLUSIONSTATE_EXCLUDED) {
        const u3c_event_data_learn_start_t * const p_data_learn_start = (const u3c_event_data_learn_start_t *) p_data;
        RECEIVE_OPTIONS_TYPE_EX rx_options = { 0 };
        CC_UserCredential_CredentialLearnStart_handler(
          p_data_learn_start->operation_type, &p_data_learn_start->target,
          p_data_learn_start->timeout_seconds, &rx_options);
      }
      break;
    }
    case CC_USER_CREDENTIAL_EVENT_LEARN_STEP_START: {
      const uint8_t * const remaining_steps = (const uint8_t * const)p_data;
      cl_state.remaining_steps = *remaining_steps;
      // Start or restart timeout timer for this step
      TimerStart(&cl_timeout_timer, cl_state.data.timeout_seconds * S_TO_MS);
      /**
       * Keep device awake for the duration of this step.
       * This is necessary to ensure that the final reports will be sent out correctly.
       */
      zpal_radio_update_stay_awake(&credential_learn_stay_awake_id, cl_state.data.timeout_seconds * S_TO_MS);
      CC_UserCredential_CredentialLearnStatusReport_tx(
        CL_STATUS_STARTED, &cl_state.data.target, *remaining_steps, &cl_state.rx_options
        );
      break;
    }
    case CC_USER_CREDENTIAL_EVENT_LEARN_READ_DONE: {
      const u3c_event_data_learn_read_done_t * const p_event_data =
        (const u3c_event_data_learn_read_done_t * const) p_data;
      uint8_t data_length = p_event_data->data_length;
      uint8_t * data = p_event_data->data;

      TimerStop(&cl_timeout_timer);
      zpal_radio_revoke_stay_awake(&credential_learn_stay_awake_id);
      /**
       * Silently fail if the payload is missing or the process was not already
       * initiated
       */
      if (!data || !data_length || !is_credential_learn_in_progress()) {
        credential_learn_reset();
        break;
      }

      u3c_credential_t credential = {
        .metadata = {
          .uuid = cl_state.data.target.uuid,
          .type = cl_state.data.target.type,
          .slot = cl_state.data.target.slot,
          .length = data_length,
          .modifier_node_id = 0,
          .modifier_type = MODIFIER_TYPE_LOCALLY,
        },
        .data = data
      };
      u3c_db_operation_result db_op_result = U3C_DB_OPERATION_RESULT_ERROR;

      /**
       * CC:0083.01.0F.11.002: validate_new_credential_data() will send the
       * required Credential Set Error Report to the initiator upon failure
       */
      if (validate_new_credential_data(&credential, &cl_state.rx_options)) {
        // Write Credential into database
        switch (cl_state.data.operation_type) {
          case U3C_OPERATION_TYPE_ADD:
            db_op_result = CC_UserCredential_add_credential(&credential);
            break;
          case U3C_OPERATION_TYPE_MODIFY:
            db_op_result = CC_UserCredential_modify_credential(&credential);
            break;
          default:
            break;
        }
      }

      uint8_t event_out = CC_USER_CREDENTIAL_EVENT_LEARN_FAILED;
      u3c_credential_learn_status_t status_out = CL_STATUS_ENDED_NOT_DUE_TO_TIMEOUT;
      if (db_op_result == U3C_DB_OPERATION_RESULT_SUCCESS) {
        event_out = CC_USER_CREDENTIAL_EVENT_LEARN_SUCCESS;
        status_out = CL_STATUS_SUCCESS;
      } else {
        ZPAL_LOG_ERROR(ZPAL_LOG_CC_USER_CREDENTIAL, "Error: Credential Learn failed due to database error (%d)\n", db_op_result);
      }

      // Notify application
      zaf_event_distributor_enqueue_cc_event(
        COMMAND_CLASS_USER_CREDENTIAL, event_out, NULL);

      // CC:0083:01.0F.11.000: Notify initiating node
      CC_UserCredential_CredentialLearnStatusReport_tx(
        status_out, &cl_state.data.target, 0, &cl_state.rx_options
        );

      // CC:0083.01.0F.11.001: Send Credential Report of new Credential
      if (db_op_result == U3C_DB_OPERATION_RESULT_SUCCESS) {
        u3c_credential_report_type_t report_type = CREDENTIAL_REP_TYPE_UNCHANGED;
        switch (cl_state.data.operation_type) {
          case U3C_OPERATION_TYPE_ADD:
            report_type = CREDENTIAL_REP_TYPE_ADDED;
            break;
          case U3C_OPERATION_TYPE_MODIFY:
            report_type = CREDENTIAL_REP_TYPE_MODIFIED;
            break;
          default:
            assert(false); // Invalid database operation type!
            break;
        }

        // Get next credential's details
        u3c_credential_type next_credential_type = CREDENTIAL_TYPE_NONE;
        uint16_t next_credential_slot = 0;
        CC_UserCredential_get_next_credential(
          0, credential.metadata.type, credential.metadata.slot,
          &next_credential_type, &next_credential_slot);

        CC_UserCredential_CredentialReport_tx(
          report_type, &credential, next_credential_type, next_credential_slot,
          &cl_state.rx_options
          );
      }

      credential_learn_reset();

      break;
    }
    case CC_USER_CREDENTIAL_EVENT_LEARN_STEP_RETRY: {
      if (is_credential_learn_in_progress()) {
        const uint8_t * const remaining_steps = (const uint8_t * const) p_data;
        TimerRestart(&cl_timeout_timer);
        CC_UserCredential_CredentialLearnStatusReport_tx(
          CL_STATUS_STEP_RETRY, &cl_state.data.target, *remaining_steps, &cl_state.rx_options);
      }
      break;
    }
    case CC_USER_CREDENTIAL_EVENT_LEARN_CANCEL_LOCAL: {
      if (is_credential_learn_in_progress()) {
        CC_UserCredential_CredentialLearnStatusReport_tx(
          CL_STATUS_ENDED_NOT_DUE_TO_TIMEOUT, &cl_state.data.target, 0, &cl_state.rx_options);
        credential_learn_reset();
      }
      break;
    }
    default: {
      // Events that are not related to Credential Learn functionality should not be passed to this function.
      assert(false);
      break;
    }
  }
}
