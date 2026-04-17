/**
 * @file
 * @brief Handler for Command Class User Credential.
 * @copyright 2023 Silicon Laboratories Inc.
 */

#include <string.h>
#include "CC_UserCredential.h"
#include "cc_user_credential_config_api.h"
#include "cc_user_credential_io_config.h"
#include "cc_user_credential_io.h"
#include "cc_user_credential_operations.h"
#include "cc_user_credential_validation.h"
#include "cc_user_credential_learn.h"
#include "cc_user_credential_handlers_admin_code.h"
#include "cc_user_credential_handlers_capabilities.h"
#include "cc_user_credential_handlers_checksum.h"
#include "cc_user_credential_handlers_database.h"
#include "CC_Notification.h"
#include "zaf_event_distributor_soc.h"
#include "assert.h"

static void init(void)
{
  CC_UserCredential_init_database();
  credential_learn_reset();
}

static void reset(void)
{
  CC_UserCredential_factory_reset();
}

void CC_UserCredential_set_default_name(uint8_t * pName, u3c_user_t * pUser)
{
  const char prefix[] = { 'U', 's', 'e', 'r', '-' };
  uint8_t id_text[DEFAULT_NAME_MAX_SIZE - sizeof(prefix)] = { 0 };
  bool is_16_bit_encoding =
    pUser->name_encoding == USER_NAME_ENCODING_UNICODE_UTF_16;

  uint8_t digits = 0;
  uint16_t ten_exponent = 1;
  for (uint8_t i = 0; i < sizeof(id_text); ++i) {
    uint16_t digit = (pUser->unique_identifier / ten_exponent) % 10;
    id_text[sizeof(id_text) - 1 - i] = '0' + (uint8_t)digit;
    if (digit > 0) {
      digits = i + 1;
    }
    ten_exponent *= 10;
  }
  uint8_t character_count = sizeof(prefix) + digits;

  for (uint8_t i = 0; i < character_count; ++i) {
    uint8_t current_character;
    if (i < sizeof(prefix)) {
      current_character = (uint8_t)prefix[i];
    } else {
      current_character = id_text[i - sizeof(prefix) + sizeof(id_text) - digits];
    }
    if (is_16_bit_encoding) {
      pName[2 * i] = '\0';
      pName[(2 * i) + 1] = current_character;
    } else {
      pName[i] = current_character;
    }
  }

  pUser->name_length = character_count * (is_16_bit_encoding ? 2 : 1);
}

static received_frame_status_t CC_UserCredential_CredentialSet_parser(
  cc_handler_input_t * input)
{
  uint16_t uuid = (uint16_t)(input->frame->ZW_CredentialSet1byteFrame.userUniqueIdentifier1 << 8)
                  | input->frame->ZW_CredentialSet1byteFrame.userUniqueIdentifier2;
  u3c_credential_type credential_type;
  credential_type = (u3c_credential_type)input->frame->ZW_CredentialSet1byteFrame.credentialType;

  // Validate operation type
  u3c_operation_type_t operation_type;
  operation_type = (u3c_operation_type_t)input->frame->ZW_CredentialSet1byteFrame.properties1
                   & CREDENTIAL_SET_PROPERTIES1_OPERATION_TYPE_MASK;
  if (operation_type > U3C_OPERATION_TYPE_DELETE) {
    return RECEIVED_FRAME_STATUS_FAIL;
  }

  // Validate length
  uint8_t credential_length = 0;
  if (input->length < CREDENTIAL_SET_FRAME_LENGTH_WITHOUT_CREDENTIAL_LENGTH) {
    return RECEIVED_FRAME_STATUS_FAIL;
  } else if (input->length > CREDENTIAL_SET_FRAME_LENGTH_WITHOUT_CREDENTIAL_LENGTH) {
    credential_length = input->frame->ZW_CredentialSet1byteFrame.credentialLength;
    if (input->length != (CREDENTIAL_SET_FRAME_LENGTH_WITHOUT_CREDENTIAL_DATA + credential_length)) {
      return RECEIVED_FRAME_STATUS_FAIL;
    }
  }

  if (
    operation_type != U3C_OPERATION_TYPE_DELETE
    && (credential_length < cc_user_credential_get_min_length_of_data(credential_type)
        || credential_length > cc_user_credential_get_max_length_of_data(credential_type))
    ) {
    return RECEIVED_FRAME_STATUS_FAIL;
  }

  uint16_t credential_slot = (uint16_t)(input->frame->ZW_CredentialSet1byteFrame.credentialSlot1 << 8)
                             | input->frame->ZW_CredentialSet1byteFrame.credentialSlot2;
  u3c_credential_t credential = {
    .metadata = {
      .uuid = uuid,
      .length = credential_length,
      .modifier_node_id = input->rx_options->sourceNode.nodeId,
      .modifier_type = MODIFIER_TYPE_Z_WAVE,
      .slot = credential_slot,
      .type = credential_type
    },
    .data = &input->frame->ZW_CredentialSet1byteFrame.credentialData1
  };

  received_frame_status_t status;
  status = CC_UserCredential_CredentialSet_handler(
    operation_type, &credential, input->rx_options
    );
  return status;
}

static received_frame_status_t CC_UserCredential_CredentialGet_parser(
  cc_handler_input_t * input)
{
  u3c_credential_metadata_t metadata = {
    .uuid = (uint16_t)(input->frame->ZW_CredentialGetFrame.userUniqueIdentifier1 << 8)
            | input->frame->ZW_CredentialGetFrame.userUniqueIdentifier2,
    .length = 0,
    .modifier_type = MODIFIER_TYPE_UNKNOWN,
    .modifier_node_id = 0,
    .type = input->frame->ZW_CredentialGetFrame.credentialType,
    .slot = (uint16_t)(input->frame->ZW_CredentialGetFrame.credentialSlot1 << 8)
            | input->frame->ZW_CredentialGetFrame.credentialSlot2
  };

  bool result = CC_UserCredential_CredentialGet_handler(input, &metadata, CC_UserCredential_CredentialReport_tx);

  return (true == result) ? RECEIVED_FRAME_STATUS_SUCCESS : RECEIVED_FRAME_STATUS_FAIL;
}

static received_frame_status_t CC_UserCredential_CredentialLearnStart_parser(
  cc_handler_input_t * input)
{
  const ZW_CREDENTIAL_LEARN_START_FRAME * const pFrameIn = &input->frame->ZW_CredentialLearnStartFrame;

  // Extract Credential data from incoming frame
  u3c_operation_type_t operation_type = pFrameIn->properties1 & CREDENTIAL_LEARN_START_PROPERTIES1_OPERATION_TYPE_MASK;
  uint8_t timeout_seconds = pFrameIn->credentialLearnTimeout;
  u3c_credential_identifier_t learn_target = {
    .uuid = (uint16_t)(pFrameIn->userUniqueIdentifier1 << 8 | pFrameIn->userUniqueIdentifier2),
    .type = pFrameIn->credentialType,
    .slot = (uint16_t)(pFrameIn->credentialSlot1 << 8 | pFrameIn->credentialSlot2),
  };

  if (
    learn_target.uuid == 0    // CC:0083.01.0F.11.005
    || learn_target.slot == 0 // CC:0083.01.0F.11.006
    ) {
    // Ignore the frame
    return RECEIVED_FRAME_STATUS_FAIL;
  }

  return CC_UserCredential_CredentialLearnStart_handler(
    operation_type, &learn_target, timeout_seconds, input->rx_options
    );
}

static received_frame_status_t CC_UserCredential_UserCredentialAssociationSet_handler(
  RECEIVE_OPTIONS_TYPE_EX * const p_rx_options,
  const u3c_credential_metadata_t * const source_metadata,
  const u3c_credential_metadata_t * const destination_metadata
  )
{
  if (!p_rx_options || !source_metadata || !destination_metadata) {
    assert(false);
    return RECEIVED_FRAME_STATUS_FAIL;
  }

  u3c_db_operation_result operation_result =
    CC_UserCredential_move_credential_and_report(source_metadata->type,
                                                 source_metadata->slot, destination_metadata->uuid, p_rx_options);

  return operation_result == U3C_DB_OPERATION_RESULT_SUCCESS
         ?RECEIVED_FRAME_STATUS_SUCCESS : RECEIVED_FRAME_STATUS_FAIL;
}

static received_frame_status_t CC_UserCredential_UserCredentialAssociationSet_parser(
  cc_handler_input_t const * const input)
{
  uint16_t slot = (uint16_t)(input->frame->ZW_UserCredentialAssociationSetFrame.credentialSlot1 << 8)
                  | input->frame->ZW_UserCredentialAssociationSetFrame.credentialSlot2;

  uint16_t destination_uuid = (uint16_t)(input->frame->ZW_UserCredentialAssociationSetFrame.destinationUserUniqueIdentifier1 << 8)
                              | input->frame->ZW_UserCredentialAssociationSetFrame.destinationUserUniqueIdentifier2;

  u3c_credential_metadata_t source_metadata = { 0 };
  source_metadata.type = input->frame->ZW_UserCredentialAssociationSetFrame.credentialType;
  source_metadata.slot = slot;

  u3c_credential_metadata_t destination_metadata = { 0 };
  destination_metadata.uuid = destination_uuid;
  destination_metadata.type = source_metadata.type;
  destination_metadata.slot = slot;

  bool parsing_success = false;
  u3c_user_credential_association_report_status_t status = U3C_UCAR_STATUS_SUCCESS;

  // Configuration-related checks according to Table 2.546

  // Source Credential Type must be supported
  if (!cc_user_credential_is_credential_type_supported(source_metadata.type)) {
    status = U3C_UCAR_STATUS_CREDENTIAL_TYPE_INVALID;
  }
  // Source Credential Slot must be in supported range
  else if (slot == 0 || slot > cc_user_credential_get_max_credential_slots(source_metadata.type)) {
    status = U3C_UCAR_STATUS_CREDENTIAL_SLOT_INVALID;
  }
  // Destination User Unique Identifier must be in supported range
  else if (0 == destination_uuid || destination_uuid > cc_user_credential_get_max_user_unique_idenfitiers()) {
    status = U3C_UCAR_STATUS_DESTINATION_USER_UNIQUE_IDENTIFIER_INVALID;
  } else {
    parsing_success = true;
  }

  if (!parsing_success) {
    CC_UserCredential_send_association_report(&source_metadata, &destination_metadata, status, input->rx_options);
    return RECEIVED_FRAME_STATUS_FAIL;
  }

  return CC_UserCredential_UserCredentialAssociationSet_handler(
    input->rx_options, &source_metadata, &destination_metadata
    );
}

static received_frame_status_t CC_UserCredential_AdminCodeSet_parser(cc_handler_input_t *input)
{
  received_frame_status_t status = RECEIVED_FRAME_STATUS_FAIL;
  if (!cc_user_credential_get_admin_code_supported()) {
    /**
     * CC:0083.01.1A.13.002 - Ignore frame if AC not supported.
     */
    return RECEIVED_FRAME_STATUS_NO_SUPPORT;
  }
  /**
   * First, checks that the following requirements are met:
   * CC:0083.01.1A.11.004 - Code is of appropriate length.
   * CC:0083.01.1A.13.003 - If code is of length 0, ensures ACD is supported.
   * CC:0083.01.1A.11.008 - Code is ASCII encoded using characters '0' - '9'.
   */
  u3c_admin_code_metadata_t data = { 0 };
  ZW_ADMIN_PIN_CODE_SET_1BYTE_FRAME *frame_in = &input->frame->ZW_AdminPinCodeSet1byteFrame;
  data.code_length = (frame_in->properties1 & 0x0F);
  data.result = ADMIN_CODE_OPERATION_RESULT_INTERNAL_NONE;

  if (data.code_length == 0 && !cc_user_credential_get_admin_code_deactivate_supported()) {
    /**
     * CC:0083.01.1A.13.003 - We MAY return a frame if ACD is not supported.
     * Length of 0 could be a mistake, worth reporting back to controller.
     */
    data.result = ADMIN_CODE_OPERATION_RESULT_ERROR_ACD_NOT_SUPPORTED;
    status = RECEIVED_FRAME_STATUS_NO_SUPPORT;
  } else if (data.code_length) {
    // Check valid length
    if (data.code_length < AC_MIN_LENGTH || data.code_length > AC_MAX_LENGTH) {
      /**
       *  CC:0083.01.1C.11.004 - length must be between 4 and 10 characters if length is not 0.
       */
      return status;
    }

    // Copy code data into frame
    if (data.code_length) {
      memcpy(data.code_data, &frame_in->adminCode1, data.code_length);
    }

    /**
     * CC:0083.01.1A.11.010 - Check and ensure that if an Admin Code is provided, that the characters
     * are valid. If they are not, ignore the command.
     * CC:0063.02.0E.11.006 - Each byte in this field MUST be encoded with ASCII representation.
     */
    for (int i = 0; i < data.code_length; i++) {
      if (data.code_data[i] > ASCII_AC_MAX) {
        return status;
      }
    }
  }

  if (data.result != ADMIN_CODE_OPERATION_RESULT_INTERNAL_NONE) {
    CC_UserCredential_AdminCodeReport_tx(&data, input->rx_options);
    return status;
  }
  return CC_UserCredential_AdminCodeSet_handler(&data, input->rx_options);
}

static received_frame_status_t CC_UserCredential_handler(
  cc_handler_input_t * input,
  cc_handler_output_t * output)
{
  received_frame_status_t status = RECEIVED_FRAME_STATUS_FAIL;
  switch (input->frame->ZW_Common.cmd) {
    case USER_CAPABILITIES_GET:
      CC_UserCredential_UserCapabilitiesGet_handler(output);
      output->length = sizeof(ZW_USER_CAPABILITIES_REPORT_2BYTE_FRAME);
      status = RECEIVED_FRAME_STATUS_SUCCESS;
      break;

    case CREDENTIAL_CAPABILITIES_GET:
      CC_UserCredential_CredentialCapabilitiesGet_handler(output);
      status = RECEIVED_FRAME_STATUS_SUCCESS;
      break;

    case USER_SET:
      status = CC_UserCredential_UserSet_handler(input);
      break;

    case USER_GET:
      status = CC_UserCredential_UserGet_handler(input);
      break;

    case CREDENTIAL_SET:
      status = CC_UserCredential_CredentialSet_parser(input);
      break;

    case CREDENTIAL_GET:
      status = CC_UserCredential_CredentialGet_parser(input);
      break;

    case CREDENTIAL_LEARN_START:
      status = CC_UserCredential_CredentialLearnStart_parser(input);
      break;

    case CREDENTIAL_LEARN_CANCEL:
      status = CC_UserCredential_CredentialLearnCancel_handler(input);
      break;

    case USER_CREDENTIAL_ASSOCIATION_SET:
      status = CC_UserCredential_UserCredentialAssociationSet_parser(input);
      break;

    case ALL_USERS_CHECKSUM_GET:
      status = CC_UserCredential_AllUsersChecksumGet_handler(input);
      break;

    case USER_CHECKSUM_GET:
      status = CC_UserCredential_UserChecksumGet_handler(input);
      break;

    case CREDENTIAL_CHECKSUM_GET:
      status = CC_UserCredential_CredentialChecksumGet_handler(input);
      break;

    case ADMIN_PIN_CODE_GET:
      status = CC_UserCredential_AdminCodeGet_handler(input);
      break;

    case ADMIN_PIN_CODE_SET:
      status = CC_UserCredential_AdminCodeSet_parser(input);
      break;

    default:
      // Unknown command
      status = RECEIVED_FRAME_STATUS_NO_SUPPORT;
      break;
  }
  return status;
}

static uint8_t lifeline_reporting(ccc_pair_t * p_ccc_pair)
{
  p_ccc_pair->cmdClass = COMMAND_CLASS_USER_CREDENTIAL;
  p_ccc_pair->cmd      = USER_REPORT;
  ++p_ccc_pair;
  p_ccc_pair->cmdClass = COMMAND_CLASS_USER_CREDENTIAL;
  p_ccc_pair->cmd      = CREDENTIAL_REPORT;
  ++p_ccc_pair;
  p_ccc_pair->cmdClass = COMMAND_CLASS_USER_CREDENTIAL;
  p_ccc_pair->cmd      = ADMIN_PIN_CODE_REPORT;
  ++p_ccc_pair;
  p_ccc_pair->cmdClass = COMMAND_CLASS_USER_CREDENTIAL;
  p_ccc_pair->cmd      = USER_CREDENTIAL_ASSOCIATION_REPORT;
  return 4;
}

REGISTER_CC_V5(COMMAND_CLASS_USER_CREDENTIAL, USER_CREDENTIAL_VERSION,
               CC_UserCredential_handler, NULL, NULL, lifeline_reporting, 0,
               init, reset);

static void
user_credential_event_handler(const uint8_t event, const void * p_data)
{
  switch (event) {
    case CC_USER_CREDENTIAL_EVENT_VALIDATE: {
      const u3c_event_data_validate_t * const p_data_validate = (const u3c_event_data_validate_t * const) p_data;
      uint8_t notification_event = NOTIFICATION_EVENT_ACCESS_CONTROL_NO_EVENT;
      uint8_t event_out = CC_USER_CREDENTIAL_EVENT_VALIDATE_INVALID;
      u3c_credential_metadata_t stored_credential = { 0 }; // Metadata of the existing credential in the database

      if (!find_existing_credential(p_data_validate->credential, &stored_credential)) {
        // The Credential does not exist in the database
        notification_event = NOTIFICATION_EVENT_ACCESS_CONTROL_INVALID_CREDENTIAL_USED_TO_ACCESS_THE_NODE;
        CC_Notification_TriggerAndTransmit(
          0, notification_event, NULL, 0, NULL, false);
      } else {
        // The Credential exists in the database, perform additional checks
        u3c_user_t current_user = { 0 };
        CC_UserCredential_get_user(stored_credential.uuid, &current_user, NULL);

        if (current_user.type == USER_TYPE_NON_ACCESS) {
          // Non-access Users may not operate the lock, but a report should be generated
          notification_event = NOTIFICATION_EVENT_ACCESS_CONTROL_NON_ACCESS_USER_ENTERED;
        } else {
          if (!current_user.active) {
            // Users with the "Occupied Disabled" state may not be granted access
            notification_event = NOTIFICATION_EVENT_ACCESS_CONTROL_ACCESS_DENIED_OCCUPIED_DISABLED;
          } else {
            // Grant access
            event_out = CC_USER_CREDENTIAL_EVENT_VALIDATE_VALID;
            notification_event = p_data_validate->is_unlocked
                                 ? NOTIFICATION_EVENT_ACCESS_CONTROL_CREDENTIAL_LOCK_CLOSE_OPERATION
                                 : NOTIFICATION_EVENT_ACCESS_CONTROL_CREDENTIAL_UNLOCK_OPEN_OPERATION;
          }
        }

        CC_UserCredential_UsageNotification_tx(
          notification_event, stored_credential.uuid, 1, &stored_credential
          );
      }

      zaf_event_distributor_enqueue_cc_event(
        COMMAND_CLASS_USER_CREDENTIAL, event_out, NULL);

      break;
    }
    case CC_USER_CREDENTIAL_EVENT_LEARN_START_REQUEST:
      __attribute__ ((fallthrough));
    case CC_USER_CREDENTIAL_EVENT_LEARN_STEP_START:
      __attribute__ ((fallthrough));
    case CC_USER_CREDENTIAL_EVENT_LEARN_READ_DONE:
      __attribute__ ((fallthrough));
    case CC_USER_CREDENTIAL_EVENT_LEARN_STEP_RETRY:
      __attribute__ ((fallthrough));
    case CC_USER_CREDENTIAL_EVENT_LEARN_CANCEL_LOCAL: {
      CC_UserCredential_learn_event_handler(event, p_data);
      break;
    }
    default:
      break;
  }
}

ZAF_EVENT_DISTRIBUTOR_REGISTER_CC_EVENT_HANDLER(COMMAND_CLASS_USER_CREDENTIAL, user_credential_event_handler);
