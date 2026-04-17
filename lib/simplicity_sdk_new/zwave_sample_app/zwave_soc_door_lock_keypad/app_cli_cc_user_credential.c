/***************************************************************************//**
 * @file
 * @brief cli_door_lock_keypad.c
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZW_CLI_COMMON_PRESENT

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "zaf_event_distributor_soc.h"
#include "CC_UserCode.h"
#include "sl_cli.h"
#include "app_log.h"
#include "CC_UserCredential.h"
#include "cc_user_credential_io.h"
#include "cc_user_credential_operations.h"
#include "cc_user_credential_io_config.h"
#include "CC_DoorLock.h"
#include "events.h"
#include "cc_user_credential_config_api.h"
#include "cc_user_credential_validation.h"
#include "cc_user_credential_tx.h"
#include "app_credentials.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
typedef enum _u3c_credential_attribute_t {
  CREDENTIAL_ATTRIBUTE_NONE,
  CREDENTIAL_ATTRIBUTE_CREDENTIAL,
  CREDENTIAL_ATTRIBUTE_SLOT,
  CREDENTIAL_ATTRIBUTE_UUID,
  CREDENTIAL_ATTRIBUTE_END
} u3c_credential_attribute;

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static bool u3c_add_user(const char * const user_name);
static bool u3c_add_credential(u3c_credential_type type, uint16_t uuid, uint16_t slot, unsigned char* credential_data);
static bool u3c_modify_user_name(u3c_user_t *user, const char * const name);
static bool u3c_modify_user_type(u3c_user_t *user, const char * const type);
static bool u3c_modify_user_active(u3c_user_t *user, const char * const state);
static bool u3c_modify_credential(uint16_t slot, u3c_credential_type type, unsigned char* credential_data);
static bool u3c_move_credential_slot(uint16_t slot, u3c_credential_type type, uint16_t new_slot);
static bool u3c_move_credential_uuid(uint16_t slot, u3c_credential_type type, uint16_t new_uuid);
static u3c_credential_type u3c_convert_str_to_credential_type(unsigned char* str);
static u3c_user_type u3c_convert_str_to_user_type(const char * const str);
static u3c_credential_attribute u3c_convert_str_to_credential_attribute(const char * const str);
static uint16_t u3c_generate_uuid(void);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static uint8_t credential_learn_data[U3C_BUFFER_SIZE_CREDENTIAL_DATA + 1] = { 0 };
static u3c_event_data_learn_read_done_t event_learn_read_done_data = {
  .data = credential_learn_data,
  .data_length = 0
};

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/*
 * @brief CLI command to add a user.
 *
 * This function adds a user.
 *
 * @param arguments The arguments of the command.
 */
void cli_u3c_db_add_user(sl_cli_command_arg_t *arguments)
{
  const char * const user_name = sl_cli_get_argument_string(arguments, 0);

  if (u3c_add_user(user_name) == true) {
    app_log_info("%s stored successfully!\r\n", user_name);
  } else {
    app_log_error("Storing %s failed!\r\n", user_name);
  }
}

/**
 * @brief Adds a credential to the database.
 *
 * This function is used to add a new credential to the database. It takes the
 * credential information as input parameters and stores it in the database.
 *
 * @param arguments The arguments of the command.
 */
void cli_u3c_db_add_credential(sl_cli_command_arg_t *arguments)
{
  const uint16_t     uuid       = sl_cli_get_argument_uint16(arguments, 0);
  const char * const type       = sl_cli_get_argument_string(arguments, 1);
  const uint16_t     slot       = sl_cli_get_argument_uint16(arguments, 2);
  const char * const credential = sl_cli_get_argument_string(arguments, 3);
  u3c_credential_type credential_type;

  credential_type = u3c_convert_str_to_credential_type((unsigned char*)type);

  if ((credential_type != CREDENTIAL_TYPE_NONE) && (credential_type != CREDENTIAL_TYPE_NUMBER_OF_TYPES)) {
    app_log_info("Storing new %s credential.\r\n", type);
    if (u3c_add_credential(credential_type, uuid, slot, (unsigned char*)credential) == true) {
      app_log_info("%s stored successfully!\r\n", type);
    } else {
      app_log_error("Storing %s failed!\r\n", type);
    }
  }
}

/**
 * @brief Modifies a user in the database.
 *
 * This function is used to modify the details of a user in the database.
 * It takes the user ID and the new user details as parameters and updates
 * the user's information in the database.
 *
 * @param arguments The arguments of the command.
 */
void cli_u3c_db_modify_user(sl_cli_command_arg_t *arguments)
{
  const uint16_t     uuid            = sl_cli_get_argument_uint16(arguments, 0);
  const char * const attribute       = sl_cli_get_argument_string(arguments, 1);
  const char * const attribute_value = sl_cli_get_argument_string(arguments, 2);
  u3c_user_t user;

  if (CC_UserCredential_get_user(uuid, &user, NULL) == U3C_DB_OPERATION_RESULT_SUCCESS) {
    if (0 == strcmp(attribute, "type")) {
      if (u3c_modify_user_type(&user, attribute_value)) {
        app_log_info("User update with new type %s was successful\r\n", attribute_value);
      } else {
        app_log_error("User update failed with new type %s!\r\n", attribute_value);
      }
    } else if (0 == strcmp(attribute, "name")) {
      if (u3c_modify_user_name(&user, attribute_value)) {
        app_log_info("User update with new name %s was successful\r\n", attribute_value);
      } else {
        app_log_error("User update failed with name %s\r\n", attribute_value);
      }
    } else if (0 == strcmp(attribute, "active")) {
      u3c_modify_user_active(&user, attribute_value);
    } else {
      app_log_error("%s attribute is not supported!\r\n", attribute);
    }
  } else {
    app_log_error("User with uuid %d not found!\r\n", uuid);
  }
}

/**
 * @brief Modifies the credential for a given user in the database.
 *
 * This function allows the modification of the credential (e.g., password) for a specific user in the database.
 * The function takes the slot and type of the credential to identify the credential in the database to modify.
 * It also takes an attribute name and its value to update. Plus the UID of the assigned user.
 *
 * @param arguments The arguments of the command.
 */
void cli_u3c_db_modify_credential(sl_cli_command_arg_t *arguments)
{
  bool update_status = false;
  const char * const type            = sl_cli_get_argument_string(arguments, 0);
  const uint16_t     slot            = sl_cli_get_argument_uint16(arguments, 1);
  const char * const attribute       = sl_cli_get_argument_string(arguments, 2);
  const char * const attribute_value = sl_cli_get_argument_string(arguments, 3);

  u3c_credential_type credential_type = u3c_convert_str_to_credential_type((unsigned char*)type);
  u3c_credential_attribute attribute_to_change = u3c_convert_str_to_credential_attribute(attribute);

  if (credential_type != CREDENTIAL_TYPE_NUMBER_OF_TYPES) {
    switch (attribute_to_change) {
      case CREDENTIAL_ATTRIBUTE_CREDENTIAL:
        update_status = u3c_modify_credential(slot, credential_type, (unsigned char*)attribute_value);
        break;
      case CREDENTIAL_ATTRIBUTE_SLOT:
        update_status = u3c_move_credential_slot(slot, credential_type, atoi(attribute_value));
        app_log_warning("The lifeline group cannot be notified about this change!\r\n");
        break;
      case CREDENTIAL_ATTRIBUTE_UUID:
        update_status = u3c_move_credential_uuid(slot, credential_type, atoi(attribute_value));
        break;
      default:
        break;
    }
  }

  if (update_status) {
    app_log_info("Credential update was successful!\r\n");
  } else {
    app_log_error("Credential update failed!\r\n");
  }
}

/**
 * @brief Deletes a user from the database.
 *
 * This function deletes a user from the database based on the provided user ID.
 * The command also deletes all of the assigned credentials too.
 *
 * @param arguments The arguments of the command.
 */
void cli_u3c_db_delete_user(sl_cli_command_arg_t *arguments)
{
  const uint16_t uuid = sl_cli_get_argument_uint16(arguments, 0);
  RECEIVE_OPTIONS_TYPE_EX rx_options = { 0 };

  if (CC_UserCredential_delete_user_and_report(uuid, &rx_options) == U3C_DB_OPERATION_RESULT_SUCCESS) {
    app_log_info("User successfully deleted!\r\n");
    app_log_info("Deleting associated credentials\r\n");
    CC_UserCredential_delete_all_credentials_of_type(uuid, CREDENTIAL_TYPE_NONE);
    app_log_info("Done\r\n");
  } else {
    app_log_error("User deletion failed!\r\n");
  }
}

/**
 * @brief Deletes a credential from the database.
 *
 * This function deletes a credential from the database based on the provided credential ID.
 *
 * @param arguments The arguments of the command.
 */
void cli_u3c_db_delete_credential(sl_cli_command_arg_t *arguments)
{
  const char * const type = sl_cli_get_argument_string(arguments, 0);
  const uint16_t     slot = sl_cli_get_argument_uint16(arguments, 1);
  u3c_credential_type credential_type = u3c_convert_str_to_credential_type((unsigned char*)type);

  if ( credential_type != CREDENTIAL_TYPE_NUMBER_OF_TYPES) {
    RECEIVE_OPTIONS_TYPE_EX rx_options = { 0 };
    if (CC_UserCredential_delete_credential_and_report(0, credential_type, slot, &rx_options) == U3C_DB_OPERATION_RESULT_SUCCESS) {
      app_log_info("Credential successfully deleted!\r\n");
    } else {
      app_log_error("Credential deletion failed!\r\n");
    }
  }
}

/**
 * @brief Get a user's parameters from the database.
 *
 * This function is used to get the details of a user in the database.
 * It takes the user ID and prints the user's information from the database.
 *
 * @param arguments The arguments of the command.
 */
void cli_u3c_db_get_user(sl_cli_command_arg_t *arguments)
{
  const uint16_t uuid = sl_cli_get_argument_uint16(arguments, 0);
  u3c_user_t user;
  uint8_t user_name[U3C_BUFFER_SIZE_USER_NAME];

  if (CC_UserCredential_get_user(uuid, &user, user_name) == U3C_DB_OPERATION_RESULT_SUCCESS) {
    // user_name is stored as a non-null-terminated string.
    user_name[user.name_length] = '\0';

    app_log_info("User parameters of uuid %d:\r\n", uuid);
    app_log_append_info("\tname: %s\r\n", user_name);
    app_log_append_info("\tactive: %d\r\n", user.active);
    app_log_append_info("\tunique_identifier: %d\r\n", user.unique_identifier);
    app_log_append_info("\tmodifier_node_id: %d\r\n", user.modifier_node_id);
    app_log_append_info("\texpiring_timeout_minutes: %d\r\n", user.expiring_timeout_minutes);
    app_log_append_info("\tname_length: %d\r\n", user.name_length);
    app_log_append_info("\ttype: %d\r\n", user.type);
    app_log_append_info("\tmodifier_type: %d\r\n", user.modifier_type);
    app_log_append_info("\tcredential_rule: %d\r\n", user.credential_rule);
    app_log_append_info("\tname_encodings: %d\r\n", user.name_encoding);
  } else {
    app_log_error("User with uuid %d not found!\r\n", uuid);
  }
}

/**
 * @brief Get a credential's parameters from the database.
 *
 * This function is used to get the details of a credential in the database.
 * It takes the crenential's slot and type,
 * and prints the credential's information from the database.
 *
 * @param arguments The arguments of the command.
 */
void cli_u3c_db_get_credential(sl_cli_command_arg_t *arguments)
{
  const char* const type = sl_cli_get_argument_string(arguments, 0);
  const uint16_t slot    = sl_cli_get_argument_uint16(arguments, 1);
  u3c_credential_type credential_type;
  u3c_credential_t credential;
  uint8_t credential_data[U3C_BUFFER_SIZE_CREDENTIAL_DATA + 1] = { 0 };
  u3c_db_operation_result db_operation;

  credential_type = u3c_convert_str_to_credential_type((unsigned char*)type);

  if ((credential_type != CREDENTIAL_TYPE_NONE) && (credential_type != CREDENTIAL_TYPE_NUMBER_OF_TYPES)) {
    db_operation = CC_UserCredential_get_credential(0, credential_type, slot, &credential.metadata, credential_data);

    if (U3C_DB_OPERATION_RESULT_SUCCESS == db_operation) {
      app_log_info("Credential parameters:\r\n");
      app_log_append_info("\ttype: %d\r\n", credential.metadata.type);
      app_log_append_info("\tuuid: %d\r\n", credential.metadata.uuid);
      app_log_append_info("\tslot: %d\r\n", credential.metadata.slot);
      app_log_append_info("\tlength: %d\r\n", credential.metadata.length);
      app_log_append_info("\tmodifier_node_id: %d\r\n", credential.metadata.modifier_node_id);
      app_log_append_info("\tmodifier_type: %d\r\n", credential.metadata.modifier_type);
      app_log_append_info("\tdata: %s\r\n", credential_data);
    } else if (U3C_DB_OPERATION_RESULT_FAIL_DNE == db_operation) {
      app_log_error("Credential does not exist!\r\n");
    } else {
      app_log_error("Cannot get credential!\r\n");
    }
  }
}

/**
 * @brief Enters a credential via the CLI.
 *
 * This function allows the user to enter a credential via the Command Line Interface (CLI).
 * The entered credential is then used for authentication and authorization purposes.
 *
 * @param arguments The arguments of the command.
 */
void cli_u3c_enter_credential(sl_cli_command_arg_t *arguments)
{
  static char credential_buffer[U3C_BUFFER_SIZE_CREDENTIAL_DATA] = { 0 };
  const char* const type            = sl_cli_get_argument_string(arguments, 0);
  const char* const credential_data = sl_cli_get_argument_string(arguments, 1);
  uint16_t credential_size = 0;
  u3c_credential_type credential_type;

  credential_size = strlen(credential_data);

  memset(credential_buffer, 0, sizeof(credential_buffer));
  memcpy(credential_buffer, credential_data, credential_size);

  credential_type = u3c_convert_str_to_credential_type((unsigned char*)type);

  static u3c_credential_t credential = {
    .metadata = {
      .uuid = 1,
      .slot = 1,
      .modifier_node_id = 0,
      .modifier_type = MODIFIER_TYPE_LOCALLY
    }
  };
  static u3c_event_data_validate_t user_credential_event_validate_data = {
    .credential = &credential,
    .is_unlocked = true
  };

  if (credential_type != CREDENTIAL_TYPE_NUMBER_OF_TYPES) {
    u3c_credential_t credential_search = {
      .metadata = {
        .type = credential_type,
        .length = credential_size
      },
      .data = (uint8_t*)credential_buffer
    };
    u3c_credential_metadata_t credential_find_metadata;
    bool is_credential_found = find_existing_credential(&credential_search, &credential_find_metadata);
    if (is_credential_found) {
      u3c_user_t user;
      uint8_t user_name[U3C_BUFFER_SIZE_USER_NAME + 1] = { 0 };
      CC_UserCredential_get_user(credential_find_metadata.uuid, &user, user_name);
      app_log_info("Credential found for user:\r\n");
      app_log_append_info("\tname: %s\r\n", user_name);
      app_log_append_info("\tunique_identifier: %d\r\n", user.unique_identifier);
      app_log_append_info("\tslot: %d\r\n", credential_find_metadata.slot);
    } else {
      app_log_warning("Credential not found\r\n");
    }

    credential.metadata.type = credential_type;
    credential.metadata.length = credential_size;
    credential.data = (uint8_t *) credential_buffer;

    user_credential_event_validate_data.is_unlocked = door_lock_hw_bolt_is_unlocked();
    uint16_t command_class = COMMAND_CLASS_USER_CREDENTIAL;
    uint8_t  cc_event =  CC_USER_CREDENTIAL_EVENT_VALIDATE;
    void* cc_data = &user_credential_event_validate_data;
    zaf_event_distributor_enqueue_cc_event(command_class, cc_event, cc_data);
  }
}

/**
 * @brief Starts the credential learn mode via the CLI.
 *
 * This function is used to start the credential learn mode via the Command Line Interface (CLI).
 * The command triggers the EVENT_APP_CREDENTIAL_LEARN_START event,
 *
 * @param arguments The arguments of the command.
 */
void cli_u3c_credential_learn_start(sl_cli_command_arg_t *arguments)
{
  const uint16_t     uuid = sl_cli_get_argument_uint16(arguments, 0);
  const char * const type = sl_cli_get_argument_string(arguments, 1);
  const uint16_t     slot = sl_cli_get_argument_uint16(arguments, 2);

  uint16_t command_class = COMMAND_CLASS_NO_OPERATION;
  uint8_t cc_event = 0;
  void *cc_data = NULL;
  static u3c_event_data_learn_start_t credential_learn_start_data;

  u3c_credential_type credential_type = u3c_convert_str_to_credential_type((unsigned char*)type);

  if (credential_type != CREDENTIAL_TYPE_NUMBER_OF_TYPES) {
    credential_learn_start_data.target.uuid = uuid;
    credential_learn_start_data.target.slot = slot;
    credential_learn_start_data.target.type = credential_type;
    credential_learn_start_data.operation_type = U3C_OPERATION_TYPE_ADD;
    credential_learn_start_data.source_id = 0;
    credential_learn_start_data.timeout_seconds = 0;

    // Initiates the Credential Learn process locally
    command_class = COMMAND_CLASS_USER_CREDENTIAL;
    cc_event = CC_USER_CREDENTIAL_EVENT_LEARN_START_REQUEST;
    cc_data = &credential_learn_start_data;
    zaf_event_distributor_enqueue_cc_event(command_class, cc_event, cc_data);
  }
}

/**
 * @brief
 *
 * This function is used to enter a credential during the Credential Learn process.
 * It has no effect if the Credential Learn process is not active.
 * The command triggers the EVENT_APP_CREDENTIAL_LEARN_START event,
 *
 * @param arguments The arguments of the command.
 */
void cli_u3c_credential_learn(sl_cli_command_arg_t *arguments)
{
  const char * const data = sl_cli_get_argument_string(arguments, 0);

  event_learn_read_done_data.data_length = strnlen(data, U3C_BUFFER_SIZE_CREDENTIAL_DATA);
  strncpy((char *)credential_learn_data, data, event_learn_read_done_data.data_length);

  zaf_event_distributor_enqueue_cc_event(
    COMMAND_CLASS_USER_CREDENTIAL, CC_USER_CREDENTIAL_EVENT_LEARN_READ_DONE,
    &event_learn_read_done_data);
}

/**
 * @brief Adds a new user to the system.
 *
 * This function adds a new user to the system with the provided username and password.
 * It performs necessary validations and checks before adding the user.
 *
 * @param username The username of the user to be added.
 * @return Returns true if the user is successfully added, otherwise false.
 */
static bool u3c_add_user(const char * const user_name)
{
  static uint16_t uuid = 0;
  bool operation_result = false;

  // Check whether the User Name's length is valid
  uint8_t user_name_max_length = cc_user_credential_get_max_length_of_user_name();
  uint8_t user_name_length = strnlen(user_name, UINT8_MAX);
  if (user_name_length > user_name_max_length) {
    app_log_error("Username cannot be longer than %u characters\r\n", user_name_max_length);
    return false;
  }
  if (user_name_length == 0) {
    app_log_error("Username cannot be empty!\r\n");
    return false;
  }

  uuid = u3c_generate_uuid();

  u3c_user_t user = {
    .active = true,
    .unique_identifier = uuid,
    .modifier_node_id = 0,
    .expiring_timeout_minutes = 0,
    .name_length = user_name_length,
    .type = USER_TYPE_GENERAL,
    .modifier_type = MODIFIER_TYPE_LOCALLY,
    .credential_rule = CREDENTIAL_RULE_SINGLE,
    .name_encoding = USER_NAME_ENCODING_STANDARD_ASCII,
  };

  RECEIVE_OPTIONS_TYPE_EX rx_options = { 0 };
  if (CC_UserCredential_add_user_and_report(&user, (unsigned char*)user_name, &rx_options) == U3C_DB_OPERATION_RESULT_SUCCESS) {
    operation_result = true;
    uuid--;
    app_log_info("User added with the following parameters:\r\n");
    app_log_append_info("\tname: %s\r\n", user_name);
    app_log_append_info("\tactive: %d\r\n", user.active);
    app_log_append_info("\tunique_identifier: %d\r\n", user.unique_identifier);
    app_log_append_info("\tmodifier_node_id: %d\r\n", user.modifier_node_id);
    app_log_append_info("\texpiring_timeout_minutes: %d\r\n", user.expiring_timeout_minutes);
    app_log_append_info("\tname_length: %d\r\n", user.name_length);
    app_log_append_info("\ttype: %d\r\n", user.type);
    app_log_append_info("\tmodifier_type: %d\r\n", user.modifier_type);
    app_log_append_info("\tcredential_rule: %d\r\n", user.credential_rule);
    app_log_append_info("\tname_encodings: %d\r\n", user.name_encoding);
  } else {
    app_log_error("Add user failed!\r\n");
  }
  return operation_result;
}

/**
 * @brief Modifies a credential.
 *
 * This function allows the modification of the credential data.
 * It takes the slot, type pair to identify the credential and the new credential data.
 *
 * @param slot Slot of the existing credential
 * @param type Type of the existing credential
 * @param credential_data New credential data
 */
static bool u3c_modify_credential(uint16_t slot, u3c_credential_type type, unsigned char* credential_data)
{
  bool operation_result = false;

  // Retrieve the credential from the database
  u3c_credential_t credential = { 0 };
  u3c_db_operation_result get_result = CC_UserCredential_get_credential(0, type, slot, &credential.metadata, NULL);
  if (get_result != U3C_DB_OPERATION_RESULT_SUCCESS) {
    app_log_error("Credential does not exist!\r\n");
    return false;
  }

  // Commit the new credential data locally
  credential.data = credential_data;
  credential.metadata.length = strnlen((char *)credential_data, U3C_BUFFER_SIZE_CREDENTIAL_DATA);
  credential.metadata.modifier_type    = MODIFIER_TYPE_LOCALLY;
  credential.metadata.modifier_node_id = 0;

  /**
   * Check if the credential length is valid.
   * This covers the checks performed in validate_new_credential_metadata()
   * so it doesn't need to be invoked.
   */
  uint8_t credential_min_length = cc_user_credential_get_min_length_of_data(credential.metadata.type);
  uint8_t credential_max_length = cc_user_credential_get_max_length_of_data(credential.metadata.type);
  if (credential.metadata.length < credential_min_length
      || credential.metadata.length > credential_max_length) {
    app_log_error("Credential length must be between %u and %u\r\n", credential_min_length, credential_max_length);
    return false;
  }

  // Check the new credential data for conformance with the specification and manufacturer security rules
  if (!validate_new_credential_data(&credential, NULL)) {
    app_log_error("Invalid credential data!\r\n");
    return false;
  }

  RECEIVE_OPTIONS_TYPE_EX rx_options = { 0 };
  if (CC_UserCredential_modify_credential_and_report(&credential, &rx_options) == U3C_DB_OPERATION_RESULT_SUCCESS) {
    operation_result = true;
  }

  return operation_result;
}

/**
 * @brief Moves the credential to another slot.
 *
 * The function moves an existing credential to a new slot.
 *
 * @param slot Current slot of the credential
 * @param type Type of the credential
 * @param new_slot Number of the new slot to move the credential to
 *
 * @return True if the move was successful, false otherwise.
 */
static bool u3c_move_credential_slot(uint16_t slot, u3c_credential_type type, uint16_t new_slot)
{
  u3c_credential_t credential = { 0 };
  uint8_t credential_data[U3C_BUFFER_SIZE_CREDENTIAL_DATA + 1] = { 0 };
  credential.data = credential_data;
  if (CC_UserCredential_get_credential(0, type, slot, &credential.metadata, credential.data) != U3C_DB_OPERATION_RESULT_SUCCESS) {
    return false;
  }

  if (
    /**
     * This action is no longer supported by the specification.
     * The lifeline group cannot be notified about this change.
     */
    CC_UserCredential_move_credential(type, slot, credential.metadata.uuid, new_slot)
    == U3C_DB_OPERATION_RESULT_SUCCESS) {
    return true;
  }

  return false;
}

/**
 * @brief Assigns an existing credential to another user.
 *
 * The function assigns an existing credential to another existing user in the database
 *
 * @param uuid Current UID of the assigned user
 * @param slot Slot of the credential
 * @param type Type of the credential
 * @param new_uuid UID of the user to assign to credential to
 *
 * @return True if the move was successful, false otherwise.
 */
static bool u3c_move_credential_uuid(uint16_t slot, u3c_credential_type type, uint16_t new_uuid)
{
  bool operation_result = false;

  RECEIVE_OPTIONS_TYPE_EX rx_options = { 0 };
  if (CC_UserCredential_move_credential_and_report(type, slot, new_uuid, &rx_options) == U3C_DB_OPERATION_RESULT_SUCCESS) {
    operation_result = true;
  }

  return operation_result;
}

/**
 * @brief Adds a new credential to the database.
 *
 * This function allows the user to add a new credential to the database.
 *
 * @param type u3c_credential_type instance to set the type of the credential.
 * @param uuid Assigned user's uid.
 * @param slot Number of the slot where to store the new credential.
 * @param credential_data Actual data of the credential.
 *
 * @return Returns true if the credential was successfully added, or a false
 *         error code if an error occurred.
 */
static bool u3c_add_credential(u3c_credential_type type, uint16_t uuid, uint16_t slot, unsigned char* credential_data)
{
  bool operation_result = false;

  u3c_credential_t credential = {
    .metadata = {
      .uuid = uuid,
      .type = type,
      .slot = slot,
      .modifier_node_id = 0,
      .length = strnlen((char *)credential_data, U3C_BUFFER_SIZE_USER_NAME),
      .modifier_type = MODIFIER_TYPE_LOCALLY
    },
    .data = (uint8_t *)credential_data
  };

  if (!CC_UserCredential_manufacturer_validate_credential(&credential)) {
    app_log_error("Credential does not follow manufacturer security rules!\r\n");
    return false;
  }

  if (!validate_new_credential_metadata(&credential.metadata)
      || !validate_new_credential_data(&credential, NULL)) {
    app_log_error("Invalid credential!\r\n");
    return false;
  }

  uint8_t credential_min_length = cc_user_credential_get_min_length_of_data(credential.metadata.type);
  uint8_t credential_max_length = cc_user_credential_get_max_length_of_data(credential.metadata.type);
  if (credential.metadata.length < credential_min_length
      || credential.metadata.length > credential_max_length) {
    app_log_error("Credential length must be between %u and %u\r\n", credential_min_length, credential_max_length);
    return false;
  }

  RECEIVE_OPTIONS_TYPE_EX rx_options = { 0 };
  if (CC_UserCredential_add_credential_and_report(&credential, &rx_options) == U3C_DB_OPERATION_RESULT_SUCCESS) {
    operation_result = true;
    app_log_info("Credential added with the following parameters:\r\n");
    app_log_append_info("\ttype: %d\r\n", credential.metadata.type);
    app_log_append_info("\tuuid: %d\r\n", credential.metadata.uuid);
    app_log_append_info("\tslot: %d\r\n", credential.metadata.slot);
    app_log_append_info("\tlength: %d\r\n", credential.metadata.length);
    app_log_append_info("\tmodifier_node_id: %d\r\n", credential.metadata.modifier_node_id);
    app_log_append_info("\tmodifier_type: %d\r\n", credential.metadata.modifier_type);
    app_log_append_info("\tdata: %s\r\n", credential.data);
  }
  return operation_result;
}

/**
 * @brief Modifies the name of a user.
 *
 * This function allows for modifying the name of a user in the database.
 * It takes an instance of u3c_user_t structure and the new name as parameters and updates the
 * user's name accordingly.
 *
 * @param user Instance of u3c_user_t structure.
 * @param new_name The new name to assign to the user.
 * @return Returns true if the user's name was successfully modified, or
 *         false value if an error occurred.
 */
static bool u3c_modify_user_name(u3c_user_t *user, const char * const name)
{
  bool operation_result = false;

  // Check whether the User Name's length is valid
  uint8_t user_name_max_length = cc_user_credential_get_max_length_of_user_name();
  uint8_t user_name_length = strnlen(name, UINT8_MAX);
  if (user_name_length > user_name_max_length) {
    app_log_error("Username cannot be longer than %u characters\r\n", user_name_max_length);
    return false;
  }
  if (user_name_length == 0) {
    app_log_error("Username cannot be empty!\r\n");
    return false;
  }
  user->name_length = user_name_length;

  RECEIVE_OPTIONS_TYPE_EX rx_options = { 0 };
  if (CC_UserCredential_modify_user_and_report(user, (unsigned char*)name, &rx_options) == U3C_DB_OPERATION_RESULT_SUCCESS) {
    operation_result = true;
  }

  return operation_result;
}

/**
 * @brief Modifies the user type of a given user.
 *
 * This function is used to modify the user type of a specified user in the database.
 *
 * @param user Pointer to the user structure to be modified.
 * @param type The new user type to be assigned.
 * @return Returns true if the user type was successfully modified, false otherwise.
 */
static bool u3c_modify_user_type(u3c_user_t *user, const char * const type)
{
  bool operation_result = false;
  uint8_t name[U3C_BUFFER_SIZE_USER_NAME];
  u3c_user_type new_user_type;
  new_user_type = u3c_convert_str_to_user_type(type);

  if (new_user_type != USER_TYPE_END) {
    CC_UserCredential_get_user(user->unique_identifier, user, name);
    user->type = new_user_type;
    RECEIVE_OPTIONS_TYPE_EX rx_options = { 0 };
    if ( CC_UserCredential_modify_user_and_report(user, name, &rx_options) == U3C_DB_OPERATION_RESULT_SUCCESS) {
      operation_result = true;
    }
  }

  return operation_result;
}

/**
 * @brief Modifies the active status of a user.
 *
 * This function allows for modifying the active status of a user in the database.
 * The user's active status can be set to either active or inactive.
 *
 * @param user The u3c_user_t instance to modify.
 * @param state The new active status for the user.
 *
 * @return Returns true if the user's active status was successfully modified, or false if an error occurred.
 */
static bool u3c_modify_user_active(u3c_user_t *user, const char * const state)
{
  bool operation_result = true;
  bool active_state     = false;
  char operation_name[sizeof("deactivation")] = "activation";

  if (0 == strcmp(state, "true")) {
    active_state = true;
  } else if (0 == strcmp(state, "false")) {
    active_state = false;
    strcpy(operation_name, "deactivation");
  } else {
    app_log_error("State %s is not supported!\r\n", state);
    app_log_info("Valid values are:\r\n");
    app_log_append_info("\t- true\r\n");
    app_log_append_info("\t- false\r\n");
    operation_result = false;
  }

  if (operation_result) {
    RECEIVE_OPTIONS_TYPE_EX rx_options = { 0 };
    uint8_t name[U3C_BUFFER_SIZE_USER_NAME];
    CC_UserCredential_get_user(user->unique_identifier, user, name);
    user->active = active_state;
    switch (CC_UserCredential_modify_user_and_report(user, name, &rx_options)) {
      case U3C_DB_OPERATION_RESULT_SUCCESS:
        break;
      case U3C_DB_OPERATION_RESULT_FAIL_IDENTICAL:
        app_log_info("User is already %s", active_state ? "active" : "inactive");
        return true;
      default:
        operation_result = false;
        break;
    }
  }

  if (operation_result) {
    app_log_info("User %s was successful\r\n", operation_name);
  } else {
    app_log_error("User %s failed!\r\n", operation_name);
  }

  return operation_result;
}

/**
 * @brief Converts a string representation of a credential type to its corresponding u3c_credential_type value.
 *
 * This function takes a string as input and converts it to the corresponding u3c_credential_type value.
 * The string should represent a valid credential type.
 *
 * @param str The string representation of the credential type.
 * @return The u3c_credential_type value corresponding to the input string. CREDENTIAL_TYPE_NUMBER_OF_TYPES in case of unknown raw string.
 */
static u3c_credential_type u3c_convert_str_to_credential_type(unsigned char* str)
{
  u3c_credential_type credential_type = CREDENTIAL_TYPE_NUMBER_OF_TYPES;

  if (str) {
    if (0 == strcmp((char*)str, "pin")) {
      credential_type = CREDENTIAL_TYPE_PIN_CODE;
    } else if (0 == strcmp((char*)str, "password")) {
      credential_type = CREDENTIAL_TYPE_PASSWORD;
    } else {
      app_log_error("Credential type %s is not supported!\r\n", str);
      app_log_info("Supported typed are:\r\n");
      app_log_append_info("\t- pin");
      app_log_append_info("\t- password\r\n");
    }
  }

  return credential_type;
}

/**
 * @brief Converts a string to u3c_user_type type.
 *
 * This function takes a string as input and converts it to u3c_user_type type.
 * The conversion process involves parsing the string and extracting the relevant
 * information to create an instance of the user-defined type.
 *
 * @param str The string to be converted.
 * @return The converted u3c_user_type type. USER_TYPE_END in case of unknown raw string.
 */
static u3c_user_type u3c_convert_str_to_user_type(const char * const str)
{
  u3c_user_type user_type = USER_TYPE_END;

  if (0 == strcmp(str, "general")) {
    user_type = USER_TYPE_GENERAL;
  } else if (0 == strcmp(str, "programming")) {
    user_type = USER_TYPE_PROGRAMMING;
  } else if (0 == strcmp(str, "non_access")) {
    user_type = USER_TYPE_NON_ACCESS;
  } else if (0 == strcmp(str, "duress")) {
    user_type = USER_TYPE_DURESS;
  } else if (0 == strcmp(str, "disposable")) {
    user_type = USER_TYPE_DISPOSABLE;
  } else if (0 == strcmp(str, "expiring")) {
    user_type = USER_TYPE_EXPIRING;
  } else if (0 == strcmp(str, "remote_only")) {
    user_type = USER_TYPE_REMOTE_ONLY;
  } else {
    app_log_error("New type %s is not supported!\r\n", str);
    app_log_info("Valid values are\r\n");
    app_log_append_info("\t- general\r\n");
    app_log_append_info("\t- programming\r\n");
    app_log_append_info("\t- non_access\r\n");
    app_log_append_info("\t- duress\r\n");
    app_log_append_info("\t- disposable\r\n");
    app_log_append_info("\t- expiring\r\n");
    app_log_append_info("\t- remote_only\r\n");
  }

  return user_type;
}

/**
 * @brief Converts a string to a Credential attribute.
 *
 * This function takes a string as input and converts it to a Credential attribute.
 * The Credential attribute is a data structure that represents a specific attribute of a credential.
 *
 * @param str The string to be converted.
 * @return The converted Credential attribute.
 */
static u3c_credential_attribute u3c_convert_str_to_credential_attribute(const char * const str)
{
  u3c_credential_attribute attribute = CREDENTIAL_ATTRIBUTE_END;

  if (0 == strcmp(str, "data")) {
    attribute = CREDENTIAL_ATTRIBUTE_CREDENTIAL;
  } else if (0 == strcmp(str, "uuid")) {
    attribute = CREDENTIAL_ATTRIBUTE_UUID;
  } else if (0 == strcmp(str, "slot")) {
    attribute = CREDENTIAL_ATTRIBUTE_SLOT;
  } else {
    app_log_error("Credential attribute %s is not supported!\r\n", str);
    app_log_info("Valid values are\r\n");
    app_log_append_info("\t- data\r\n");
    app_log_append_info("\t- uuid\r\n");
    app_log_append_info("\t- slot\r\n");
  }

  return attribute;
}

/**
 * @brief Generates a UUID.
 *
 * Gets an availavle uuid from the database, which can be assigned to a new user.
 *
 * @return Returns an available uuid
 */
static uint16_t u3c_generate_uuid(void)
{
  static uint16_t uuid = 1;
  uint16_t max_uuid = 0;
  u3c_db_operation_result db_operation_result = U3C_DB_OPERATION_RESULT_ERROR;

  max_uuid = cc_user_credential_get_max_user_unique_idenfitiers();

  for (; uuid <= max_uuid; uuid++) {
    db_operation_result = CC_UserCredential_get_user(uuid, NULL, NULL);

    if (db_operation_result == U3C_DB_OPERATION_RESULT_FAIL_DNE) {
      break;
    } else if (db_operation_result == U3C_DB_OPERATION_RESULT_SUCCESS) {
      continue;
    } else {
      app_log_error("Database operation failed with %d error code!\r\n", db_operation_result);
      break;
    }
  }

  return uuid;
}

void cli_log_u3c_events(
  const uint8_t event, const void * const data)
{
  switch (event) {
    case CC_USER_CREDENTIAL_EVENT_VALIDATE_VALID:
      app_log_info("The entered credential is valid\r\n");
      break;
    case CC_USER_CREDENTIAL_EVENT_VALIDATE_INVALID:
      app_log_warning("The entered credential is invalid\r\n");
      break;
    case CC_USER_CREDENTIAL_EVENT_LEARN_START:
      app_log_append_info("\r\n");
      app_log_info("Credential Learn: Process started\r\n");
      break;
    case CC_USER_CREDENTIAL_EVENT_LEARN_STEP_START:
      uint8_t * remaining_steps = (uint8_t *)data;
      app_log_info("Credential Learn: Progressed to a new step (remaining: %d)\r\n", *remaining_steps);
      if (*remaining_steps == 1) {
        app_log_append_info("Enter the new credential data now using the u3c_credential_learn command.\r\n> ");
      }
      break;
    case CC_USER_CREDENTIAL_EVENT_LEARN_READ_DONE:
      app_log_append_info("\r\n");
      app_log_info("Credential Learn: New credential data has been acquired\r\n");
      break;
    case CC_USER_CREDENTIAL_EVENT_LEARN_SUCCESS:
      app_log_info("Credential Learn: Finished successfully\r\n> ");
      break;
    case CC_USER_CREDENTIAL_EVENT_LEARN_FAILED:
      app_log_append_info("\r\n");
      app_log_warning("Credential Learn: Process failed\r\n> ");
      break;
    case CC_USER_CREDENTIAL_EVENT_LEARN_STEP_RETRY:
      app_log_append_info("\r\n");
      app_log_info("Credential Learn: Failed to acquire data, retrying\r\n");
      break;
    case CC_USER_CREDENTIAL_EVENT_LEARN_TIMEOUT:
      app_log_append_info("\r\n");
      app_log_warning("Credential Learn: Process timed out\r\n");
      break;
    case CC_USER_CREDENTIAL_EVENT_LEARN_CANCEL_REMOTE:
      app_log_append_info("\r\n");
      app_log_info("Credential Learn: Process cancelled by remote node\r\n");
      break;
    case CC_USER_CREDENTIAL_EVENT_LEARN_CANCEL_LOCAL:
      app_log_append_info("\r\n");
      app_log_info("Credential Learn: Process cancelled by application\r\n");
      break;
    default:
      break;
  }
}

ZAF_EVENT_DISTRIBUTOR_REGISTER_CC_EVENT_HANDLER(
  COMMAND_CLASS_USER_CREDENTIAL, cli_log_u3c_events);

#endif // SL_CATALOG_ZW_CLI_COMMON_PRESENT
