/**
 * @file
 * @brief Non-volatile memory implementation for Command Class User Credential
 *        I/O
 *
 * @copyright 2023 Silicon Laboratories Inc.
 */

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/

#include "cc_user_credential_nvm.h"
#include "cc_user_credential_io.h"
#include "cc_user_credential_io_config.h"
#include "ZAF_file_ids.h"
#include "ZAF_nvm.h"
#include "cc_user_credential_config_api.h"
#include "assert.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/****************************************************************************/
/*                             STATIC VARIABLES                             */
/****************************************************************************/

/**
 * The following variables must be loaded with the appropriate values by
 * invoking @ref CC_UserCredential_init_database after an application reset.
 */

// Maximum number of entries
static uint16_t max_users = 0;
static uint16_t max_credentials = 0;

// Current number of entries
static uint16_t n_users = UINT16_MAX;
static uint16_t n_credentials = UINT16_MAX;

/**
 * @brief Index of the circular buffer's head
 *
 * The range of file IDs between
 * ZAF_FILE_ID_CC_USER_CREDENTIAL_<OBJECT_TYPE>_BASE and
 * ZAF_FILE_ID_CC_USER_CREDENTIAL_<OBJECT_TYPE>_LAST is treated as a circular
 * buffer.
 * When adding a new entry, the pool of available file IDs is scanned until a
 * free ID is found, which is then populated with the corresponding object's
 * data.
 */
static uint16_t users_buffer_head = 0;
static uint16_t credentials_buffer_head = 0;

/**
 * @brief Mirror for admin pin code information.
 * Admin code information is passed around via pointer,
 * which need a persistent memory reference to work properly
 */
static admin_pin_code_metadata_nvm_t admin_code = { 0 };

/****************************************************************************/
/*                             PRIVATE FUNCTIONS                            */
/****************************************************************************/

/**
 * Execute an NVM read or write operation for object types handled by the User
 * Credential Command Class.
 *
 * @return true if the operation has been executed succesfully and more than 0
 *         bytes were transferred
 */
bool nvm(
  u3c_nvm_operation operation, u3c_nvm_area area, uint16_t offset, void* pData,
  uint16_t size)
{
  // Set parameters depending on the NVM area
  zpal_nvm_object_key_t file_base;
  switch (area) {
    /**********************/
    /* Known size objects */
    /**********************/
    case AREA_NUMBER_OF_USERS:
      file_base = ZAF_FILE_ID_CC_USER_CREDENTIAL_NUMBER_OF_USERS;
      size = sizeof(uint16_t);
      offset = 0;
      break;

    case AREA_NUMBER_OF_CREDENTIALS:
      file_base = ZAF_FILE_ID_CC_USER_CREDENTIAL_NUMBER_OF_CREDENTIALS;
      size = sizeof(uint16_t);
      offset = 0;
      break;

    case AREA_USER_DESCRIPTORS:
      file_base = ZAF_FILE_ID_CC_USER_CREDENTIAL_USER_DESCRIPTOR_TABLE;
      size = sizeof(user_descriptor_t) * n_users;
      offset = 0;
      break;

    case AREA_CREDENTIAL_DESCRIPTORS:
      file_base = ZAF_FILE_ID_CC_USER_CREDENTIAL_CREDENTIAL_DESCRIPTOR_TABLE;
      size = sizeof(credential_descriptor_t) * n_credentials;
      offset = 0;
      break;

    case AREA_USERS:
      file_base = ZAF_FILE_ID_CC_USER_CREDENTIAL_USER_BASE;
      size = sizeof(u3c_user_t);
      break;

    case AREA_CREDENTIAL_METADATA:
      file_base = ZAF_FILE_ID_CC_USER_CREDENTIAL_CREDENTIAL_BASE;
      size = sizeof(credential_metadata_nvm_t);
      break;

    case AREA_ADMIN_PIN_CODE_DATA:
      file_base = ZAF_FILE_ID_ADMIN_PIN_CODE;
      size = sizeof(admin_pin_code_metadata_nvm_t);
      break;

    /************************/
    /* Dynamic size objects */
    /************************/
    case AREA_CREDENTIAL_DATA:
      file_base = ZAF_FILE_ID_CC_USER_CREDENTIAL_CREDENTIAL_DATA_BASE;
      break;

    case AREA_USER_NAMES:
      file_base = ZAF_FILE_ID_CC_USER_CREDENTIAL_USER_NAME_BASE;
      break;

    default:
      return false;
  }

  if (size == 0) {
    return true;
  }

  zpal_status_t nvm_result = ZPAL_STATUS_FAIL;
  switch (operation) {
    case U3C_READ:
      nvm_result = ZAF_nvm_read(file_base + offset, pData, (size_t)size);
      break;
    case U3C_WRITE:
      nvm_result = ZAF_nvm_write(file_base + offset, pData, (size_t)size);
      break;
    default:
      break;
  }
  return nvm_result == ZPAL_STATUS_OK;
}

/**
 * Convert a u3c_credential_metadata_t struct to the format stored in the NVM.
 *
 * @param[out] p_metadata_nvm Pointer to the Credential metadata struct to be
 *                            stored in the NVM
 * @param[in] p_metadata_in   Pointer to the incoming standard Credential
 *                            metadata struct
 */
void convert_credential_metadata_to_nvm(
  credential_metadata_nvm_t * p_metadata_nvm,
  u3c_credential_metadata_t * p_metadata_in)
{
  p_metadata_nvm->uuid = p_metadata_in->uuid;
  p_metadata_nvm->length = p_metadata_in->length;
  p_metadata_nvm->modifier_node_id = p_metadata_in->modifier_node_id;
  p_metadata_nvm->modifier_type = p_metadata_in->modifier_type;
}

/**
 * Convert Credential metadata stored in the NVM to a u3c_credential_metadata_t
 * struct.
 *
 * @param[out] p_metadata_out Pointer to the outgoing standard Credential
 *                            struct
 * @param[in] p_metadata_nvm  Pointer to the Credential metadata struct stored
 *                            in the NVM
 * @param[in] type            The type of the Credential
 * @param[in] slot            The slot of the Credential
 * @return True if the conversion has succeeded
 */
bool convert_credential_metadata_from_nvm(
  u3c_credential_metadata_t * p_metadata_out,
  credential_metadata_nvm_t * p_metadata_nvm,
  u3c_credential_type type,
  uint16_t slot)
{
  p_metadata_out->length = p_metadata_nvm->length;
  p_metadata_out->modifier_node_id = p_metadata_nvm->modifier_node_id;
  p_metadata_out->modifier_type = p_metadata_nvm->modifier_type;
  p_metadata_out->uuid = p_metadata_nvm->uuid;
  p_metadata_out->slot = slot;
  p_metadata_out->type = type;
  return true;
}

static void ordered_insert_user_descriptor(user_descriptor_t * users, u3c_user_t * user, uint16_t offset)
{
  uint16_t insert_index = n_users;

  // Find the correct position to insert the new user
  for (uint16_t i = 0; i < n_users; ++i) {
    if (users[i].unique_identifier > user->unique_identifier) {
      insert_index = i;
      break;
    }
  }

  // Shift the elements to make room for the new user
  memmove(&users[insert_index + 1], &users[insert_index], (n_users - insert_index) * sizeof(user_descriptor_t));

  // Insert the new user at the correct position
  users[insert_index].object_offset = offset;
  users[insert_index].unique_identifier = user->unique_identifier;

  // Increment the number of users
  ++n_users;
}

/**
 * Returns the index of a Credential in the Credential descriptor table.
 */
static uint16_t get_credential_descriptor_index(
  const credential_descriptor_t * const credentials, u3c_credential_type type, uint16_t slot)
{
  uint16_t index = 0;

  // Seek until current Credential Type
  while (
    (index < n_credentials)
    && (credentials[index].credential_type < type)
    ) {
    ++index;
  }

  // Seek until after previous Credential Slot or next Credential Type
  while (
    (index < n_credentials)
    && (credentials[index].credential_type == type)
    && (credentials[index].credential_slot < slot)
    ) {
    ++index;
  }

  return index;
}

/**
 * Inserts a Credential descriptor into the Credential Descriptor table,
 * preserving:
 * 1. The ascending order of Credential Types, and
 * 2. The ascending order of Credential Slots inside of each Credential Type.
 */
static void ordered_insert_credential_descriptor(credential_descriptor_t * credentials, u3c_credential_t * p_credential, uint16_t object_offset)
{
  uint16_t insert_index = get_credential_descriptor_index(credentials, p_credential->metadata.type, p_credential->metadata.slot);

  // Shift the elements to make room for the new credential
  memmove(&credentials[insert_index + 1], &credentials[insert_index], (n_credentials - insert_index) * sizeof(credential_descriptor_t));

  // Insert the new credential at the correct position
  credentials[insert_index].object_offset = object_offset;
  credentials[insert_index].user_unique_identifier = p_credential->metadata.uuid;
  credentials[insert_index].credential_type = p_credential->metadata.type;
  credentials[insert_index].credential_slot = p_credential->metadata.slot;

  // Increment the number of credentials
  ++n_credentials;
}

void init_database_variables(void)
{
  users_buffer_head = 0;
  credentials_buffer_head = 0;
  max_users = cc_user_credential_get_max_user_unique_idenfitiers();
  max_credentials = MAX_CREDENTIAL_OBJECTS;

  /**
   * The upper limit of the descriptor table buffer sizes takes precedence over
   * the limits provided by the configuration API.
   */
  if (max_users > U3C_BUFFER_SIZE_USER_DESCRIPTORS) {
    max_users = U3C_BUFFER_SIZE_CREDENTIAL_DESCRIPTORS;
  }
  if (max_credentials > U3C_BUFFER_SIZE_CREDENTIAL_DESCRIPTORS) {
    max_credentials = U3C_BUFFER_SIZE_CREDENTIAL_DESCRIPTORS;
  }

  // Ensure that there are enough file IDs reserved for objects
  assert(max_users <= MAX_USER_OBJECTS);
  assert(max_credentials <= MAX_CREDENTIAL_OBJECTS);
}

bool is_user_identical(
  const u3c_user_t * const p_user, const uint8_t * const p_name,
  const uint16_t object_offset
  )
{
  // Check whether the incoming and stored metadata are identical
  u3c_user_t stored_user = { 0 };
  if (!nvm(U3C_READ, AREA_USERS, object_offset, &stored_user, 0)) {
    return false; // Database error
  }
  if (
    (p_user->unique_identifier != stored_user.unique_identifier)
    || (p_user->type != stored_user.type)
    || (p_user->active != stored_user.active)
    || (p_user->credential_rule != stored_user.credential_rule)
    || (p_user->expiring_timeout_minutes
        != stored_user.expiring_timeout_minutes)
    || (p_user->name_encoding != stored_user.name_encoding)
    || (p_user->name_length != stored_user.name_length)
    // Modifier Type and Modifier Node ID are ignored
    ) {
    return false; // User metadata are not identical
  }

  // Check whether the incoming and stored names are identical
  if (p_name) {
    uint8_t stored_name[U3C_BUFFER_SIZE_USER_NAME] = { 0 };
    if (!nvm(U3C_READ, AREA_USER_NAMES, object_offset, stored_name,
             stored_user.name_length)
        ) {
      return false; // Database error
    }
    return (bool)(memcmp(p_name, stored_name, p_user->name_length) == 0);
  }

  return true;
}

bool is_credential_identical(
  const u3c_credential_t * const p_credential, const uint16_t object_offset
  )
{
  // Check whether the incoming and stored metadata are identical
  credential_metadata_nvm_t stored_metadata = { 0 };
  if (!nvm(U3C_READ, AREA_CREDENTIAL_METADATA, object_offset,
           &stored_metadata, 0)
      ) {
    return false; // Database error
  }
  if (
    (p_credential->metadata.length != stored_metadata.length)
    || (p_credential->metadata.uuid != stored_metadata.uuid)
    // Modifier Type and Modifier Node ID are ignored.
    ) {
    return false; // Credential metadata are not identical
  }

  // Check whether the incoming and stored credential data are identical
  uint8_t stored_data[U3C_BUFFER_SIZE_USER_NAME] = { 0 };
  if (!nvm(U3C_READ, AREA_CREDENTIAL_DATA, object_offset, stored_data,
           stored_metadata.length)
      ) {
    return false; // Database error
  }
  return (bool)(
    memcmp(p_credential->data, stored_data, stored_metadata.length) == 0
    );
}

/****************************************************************************/
/*                           GENERAL API FUNCTIONS                          */
/****************************************************************************/

void CC_UserCredential_factory_reset(void)
{
  n_users = 0;
  n_credentials = 0;
  // Create empty descriptor tables to initialize their NVM files
  user_descriptor_t user_desciptors[1] = { 0 };
  credential_descriptor_t credential_descriptors[1] = { 0 };
  admin_pin_code_metadata_nvm_t ac = { 0 };
  nvm(U3C_WRITE, AREA_USER_DESCRIPTORS, 0, user_desciptors, 0);
  nvm(U3C_WRITE, AREA_CREDENTIAL_DESCRIPTORS, 0, credential_descriptors, 0);
  // Initialize static database variables
  nvm(U3C_WRITE, AREA_NUMBER_OF_USERS, 0, &n_users, 0);
  nvm(U3C_WRITE, AREA_NUMBER_OF_CREDENTIALS, 0, &n_credentials, 0);
  // Initialize admin code area
  nvm(U3C_WRITE, AREA_ADMIN_PIN_CODE_DATA, 0, &ac, 0);
  init_database_variables();
}

void CC_UserCredential_init_database(void)
{
  // Read stored variables, or initialize them to factory defaults if unset
  bool is_nvm_initialized = true;
  is_nvm_initialized &= nvm(U3C_READ, AREA_NUMBER_OF_USERS, 0, &n_users, 0);
  is_nvm_initialized &= nvm(U3C_READ, AREA_NUMBER_OF_CREDENTIALS, 0, &n_credentials, 0);
  if (!is_nvm_initialized
      || n_users == UINT16_MAX || n_credentials == UINT16_MAX
      ) {
    CC_UserCredential_factory_reset();
  } else {
    init_database_variables();
  }
}

/****************************************************************************/
/*                        USER RELATED API FUNCTIONS                        */
/****************************************************************************/

u3c_db_operation_result CC_UserCredential_get_user(
  uint16_t unique_identifier, u3c_user_t * user, uint8_t * name)
{
  // Check if the database is empty
  if (n_users < 1) {
    return U3C_DB_OPERATION_RESULT_FAIL_DNE;
  }

  // Name can only be requested if user is requested too.
  assert(user || !name);

  // Read the User descriptor table from NVM
  user_descriptor_t users[U3C_BUFFER_SIZE_USER_DESCRIPTORS];
  memset(users, 0, sizeof(users));

  if (!nvm(U3C_READ, AREA_USER_DESCRIPTORS, 0, &users, 0)) {
    return U3C_DB_OPERATION_RESULT_ERROR_IO;
  }

  // Find User
  for (uint16_t i = 0; i < n_users; ++i) {
    if (users[i].unique_identifier == unique_identifier) {
      // User found

      // Copy User object from NVM if requested
      if (user) {
        if (!nvm(U3C_READ, AREA_USERS, users[i].object_offset, user, 0)) {
          return U3C_DB_OPERATION_RESULT_ERROR_IO;
        }
      }

      // Copy User name from NVM if requested
      if (name) {
        if (!nvm(U3C_READ, AREA_USER_NAMES, users[i].object_offset, name,
                 user->name_length)) {
          return U3C_DB_OPERATION_RESULT_ERROR_IO;
        }
      }

      return U3C_DB_OPERATION_RESULT_SUCCESS;
    }
  }

  // User not found
  return U3C_DB_OPERATION_RESULT_FAIL_DNE;
}

uint16_t CC_UserCredential_get_next_user(uint16_t unique_identifier)
{
  // Check if the database is empty
  if (n_users < 1) {
    return 0;
  }

  // Read the User descriptor table from NVM
  user_descriptor_t users[U3C_BUFFER_SIZE_USER_DESCRIPTORS];
  memset(users, 0, sizeof(users));

  if (!nvm(U3C_READ, AREA_USER_DESCRIPTORS, 0, &users, 0)) {
    return 0;
  }

  uint16_t result = 0;
  if (unique_identifier == 0) {
    // Find the first User
    return users[0].unique_identifier;
  } else {
    // Find the next User
    for (uint16_t i = 0; i < n_users - 1; ++i) {
      if (users[i].unique_identifier == unique_identifier) {
        result = users[i + 1].unique_identifier;
        break;
      }
    }
  }

  return result;
}

u3c_db_operation_result CC_UserCredential_add_user(
  u3c_user_t * user, uint8_t * name)
{
  // Check if the database is full
  if (n_users >= max_users) {
    return U3C_DB_OPERATION_RESULT_FAIL_FULL;
  }

  // Read the User descriptor table from NVM if it is not empty
  user_descriptor_t users[U3C_BUFFER_SIZE_USER_DESCRIPTORS];
  memset(users, 0, sizeof(users));

  if (n_users > 0 && !nvm(U3C_READ, AREA_USER_DESCRIPTORS, 0, &users, 0)) {
    return U3C_DB_OPERATION_RESULT_ERROR_IO;
  }

  // Find next empty object
  bool available = false;
  uint16_t object_offset = 0;

  // Loop through all objects
  for (uint16_t attempts = 0; attempts < max_users; ++attempts) {
    object_offset = users_buffer_head;
    available = true;

    // Loop through the descriptor table
    for (uint16_t i = 0; i < n_users; ++i) {
      // Check if the user already exists
      if (users[i].unique_identifier == user->unique_identifier) {
        // Check whether the incoming user is identical to the stored one
        if (is_user_identical(user, name, object_offset)) {
          return U3C_DB_OPERATION_RESULT_FAIL_IDENTICAL;
        } else {
          return U3C_DB_OPERATION_RESULT_FAIL_OCCUPIED;
        }
      }

      // Check if the object is not assigned to any User
      if (users[i].object_offset == object_offset) {
        users_buffer_head = (uint16_t)((users_buffer_head + 1) % max_users);
        available = false;
        break; // Try the next object
      }
    }

    if (available) {
      // Write User object and name in NVM
      if (!nvm(U3C_WRITE, AREA_USERS, object_offset, user, 0)
          || !nvm(U3C_WRITE, AREA_USER_NAMES, object_offset, name,
                  user->name_length)) {
        return U3C_DB_OPERATION_RESULT_ERROR_IO;
      }

      // Update the descriptor table
      ordered_insert_user_descriptor(users, user, object_offset);

      // Update the descriptor table and number of Users in NVM
      if (
        nvm(U3C_WRITE, AREA_USER_DESCRIPTORS, 0, users, 0)
        && nvm(U3C_WRITE, AREA_NUMBER_OF_USERS, 0, &n_users, 0)
        ) {
        return U3C_DB_OPERATION_RESULT_SUCCESS;
      } else {
        --n_users;
        return U3C_DB_OPERATION_RESULT_ERROR_IO;
      }
    }
  }

  // Impossible path! The database is not full, but no free object was found
  return U3C_DB_OPERATION_RESULT_ERROR;
}

u3c_db_operation_result CC_UserCredential_modify_user(
  u3c_user_t * user, uint8_t * name)
{
  // Check if the database is empty
  if (n_users < 1) {
    return U3C_DB_OPERATION_RESULT_FAIL_DNE;
  }

  // Read the User descriptor table from NVM
  user_descriptor_t users[U3C_BUFFER_SIZE_USER_DESCRIPTORS];
  memset(users, 0, sizeof(users));

  if (!nvm(U3C_READ, AREA_USER_DESCRIPTORS, 0, &users, 0)) {
    return U3C_DB_OPERATION_RESULT_ERROR_IO;
  }

  // Find User
  for (uint16_t i = 0; i < n_users; ++i) {
    if (users[i].unique_identifier == user->unique_identifier) {
      uint16_t object_offset = users[i].object_offset;

      // Check whether the incoming user is identical to the stored one
      if (is_user_identical(user, name, object_offset)) {
        return U3C_DB_OPERATION_RESULT_FAIL_IDENTICAL;
      }

      bool write_successful = true;
      // Overwrite User object in NVM
      write_successful &= nvm(U3C_WRITE, AREA_USERS, object_offset, user, 0);
      if (write_successful && name) {
        // Overwrite User name in NVM
        write_successful &= nvm(U3C_WRITE, AREA_USER_NAMES, object_offset, name,
                                user->name_length);
      }

      return write_successful
             ? U3C_DB_OPERATION_RESULT_SUCCESS
             : U3C_DB_OPERATION_RESULT_ERROR_IO;
    }
  }

  // User not found
  return U3C_DB_OPERATION_RESULT_FAIL_DNE;
}

u3c_db_operation_result CC_UserCredential_delete_user(
  uint16_t user_unique_identifier)
{
  // Check if the database is empty
  if (n_users < 1) {
    return U3C_DB_OPERATION_RESULT_FAIL_DNE;
  }

  // Read the User descriptor table from NVM
  user_descriptor_t users[U3C_BUFFER_SIZE_USER_DESCRIPTORS];
  memset(users, 0, sizeof(users));

  if (!nvm(U3C_READ, AREA_USER_DESCRIPTORS, 0, &users, 0)) {
    return U3C_DB_OPERATION_RESULT_ERROR_IO;
  }

  // Find User
  for (uint16_t i = 0; i < n_users; ++i) {
    if (users[i].unique_identifier == user_unique_identifier) {
      --n_users;

      // If the deleted User was not the last in the list
      if (i < n_users) {
        // Shift the elements to fill the gap
        memmove(&users[i], &users[i + 1], (n_users - i) * sizeof(user_descriptor_t));
      }
      // Otherwise, simply consider its entry 'popped' from the array.

      // Update the descriptor table in NVM
      if (!nvm(U3C_WRITE, AREA_USER_DESCRIPTORS, 0, &users, 0)) {
        ++n_users;
        return U3C_DB_OPERATION_RESULT_ERROR_IO;
      }

      // Update the number of Users in NVM
      if (!nvm(U3C_WRITE, AREA_NUMBER_OF_USERS, 0, &n_users, 0)) {
        return U3C_DB_OPERATION_RESULT_ERROR_IO;
      }

      // Make sure the buffer's head is pointing at a valid object
      if (users_buffer_head >= n_users) {
        users_buffer_head = 0;
      }

      return U3C_DB_OPERATION_RESULT_SUCCESS;
    }
  }

  // User not found
  return U3C_DB_OPERATION_RESULT_FAIL_DNE;
}

/****************************************************************************/
/*                     CREDENTIAL RELATED API FUNCTIONS                     */
/****************************************************************************/

u3c_db_operation_result CC_UserCredential_get_credential(
  uint16_t user_unique_identifier, u3c_credential_type credential_type,
  uint16_t credential_slot, u3c_credential_metadata_t * p_credential_metadata,
  uint8_t * p_credential_data)
{
  // Check if the database is empty
  if (n_credentials < 1) {
    return U3C_DB_OPERATION_RESULT_FAIL_DNE;
  }

  // Read the Credential descriptor table from NVM
  credential_descriptor_t credentials[U3C_BUFFER_SIZE_CREDENTIAL_DESCRIPTORS];
  if (!nvm(U3C_READ, AREA_CREDENTIAL_DESCRIPTORS, 0, &credentials, 0)) {
    return U3C_DB_OPERATION_RESULT_ERROR_IO;
  }

  bool match_any_user = (user_unique_identifier == 0);

  // Find Credential
  for (uint16_t i = 0; i < n_credentials; ++i) {
    if ((match_any_user
         || credentials[i].user_unique_identifier == user_unique_identifier)
        && credentials[i].credential_type == credential_type
        && credentials[i].credential_slot == credential_slot
        ) {
      // Copy Credential metadata from NVM if requested
      if (p_credential_metadata) {
        credential_metadata_nvm_t metadata;
        memset(&metadata, 0, sizeof(credential_metadata_nvm_t));

        if (!nvm(U3C_READ, AREA_CREDENTIAL_METADATA, credentials[i].object_offset,
                 &metadata, 0)) {
          return U3C_DB_OPERATION_RESULT_ERROR_IO;
        }

        // Assemble outgoing Credential metadata struct
        p_credential_metadata->uuid = metadata.uuid;
        p_credential_metadata->slot = credential_slot;
        p_credential_metadata->type = credential_type;
        p_credential_metadata->length = metadata.length;
        p_credential_metadata->modifier_node_id = metadata.modifier_node_id;
        p_credential_metadata->modifier_type = metadata.modifier_type;
      }

      // Copy Credential data from NVM if requested

      if (p_credential_data) {
        if (!nvm(U3C_READ, AREA_CREDENTIAL_DATA, credentials[i].object_offset,
                 p_credential_data, p_credential_metadata->length)) {
          return U3C_DB_OPERATION_RESULT_ERROR_IO;
        }
      }
      return U3C_DB_OPERATION_RESULT_SUCCESS;
    }
  }

  // Credential not found
  return U3C_DB_OPERATION_RESULT_FAIL_DNE;
}

bool CC_UserCredential_get_next_credential(
  uint16_t user_unique_identifier, u3c_credential_type credential_type,
  uint16_t credential_slot, u3c_credential_type * next_credential_type,
  uint16_t * next_credential_slot)
{
  // Check if the database is empty
  if (n_credentials < 1) {
    return false;
  }

  // Read the Credential descriptor table from NVM
  credential_descriptor_t credentials[U3C_BUFFER_SIZE_CREDENTIAL_DESCRIPTORS];
  if (!nvm(U3C_READ, AREA_CREDENTIAL_DESCRIPTORS, 0, &credentials, 0)) {
    return false;
  }

  bool match_any_user = (user_unique_identifier == 0);
  bool match_any_type = (credential_type == CREDENTIAL_TYPE_NONE);

  bool was_next_found = false;
  uint16_t next_index = 0;

  if (credential_slot == 0) {
    // Find the first Credential
    for (uint16_t i = 0; i < n_credentials; ++i) {
      if (
        (match_any_user
         || credentials[i].user_unique_identifier == user_unique_identifier)
        && (match_any_type || credentials[i].credential_type == credential_type)
        ) {
        was_next_found = true;
        next_index = i;
        break;
      }
    }
  } else {
    if (match_any_type) {
      // A credential type must be provided for a non-zero slot number.
      return false;
    }
    for (uint16_t i = 0; i < n_credentials; ++i) {
      // Discard credentials associated to a different user if specified
      if (
        !match_any_user
        && (credentials[i].user_unique_identifier != user_unique_identifier)
        ) {
        continue;
      }

      // Check if this credential is past the current one
      if (
        (credentials[i].credential_type > credential_type)
        || (
          (credentials[i].credential_type == credential_type)
          && (credentials[i].credential_slot > credential_slot)
          )
        ) {
        was_next_found = true;
        next_index = i;
        break;
      }
    }
  }

  if (was_next_found) {
    *next_credential_type = credentials[next_index].credential_type;
    *next_credential_slot = credentials[next_index].credential_slot;
  }
  return was_next_found;
}

u3c_db_operation_result CC_UserCredential_add_credential(
  u3c_credential_t * p_credential)
{
  // Check if the database is full
  if (n_credentials >= max_credentials) {
    return U3C_DB_OPERATION_RESULT_FAIL_FULL;
  }

  // Read Credential descriptor table if it is not empty
  credential_descriptor_t credentials[U3C_BUFFER_SIZE_CREDENTIAL_DESCRIPTORS] = { 0 };
  if (n_credentials > 0
      && !nvm(U3C_READ, AREA_CREDENTIAL_DESCRIPTORS, 0, &credentials, 0)
      ) {
    return U3C_DB_OPERATION_RESULT_ERROR_IO;
  }

  // Find next empty object
  bool available = false;
  uint16_t object_offset = 0;

  // Loop through all objects
  for (uint16_t attempts = 0; attempts < max_credentials; ++attempts) {
    object_offset = credentials_buffer_head;
    available = true;

    // Loop through the descriptor table
    for (uint16_t i = 0; i < n_credentials; ++i) {
      // Check if the Credential already exists
      if (credentials[i].credential_type == p_credential->metadata.type
          && credentials[i].credential_slot == p_credential->metadata.slot) {
        // Check whether the incoming credential is identical to the stored one
        if (is_credential_identical(p_credential, object_offset)) {
          return U3C_DB_OPERATION_RESULT_FAIL_IDENTICAL;
        } else {
          return U3C_DB_OPERATION_RESULT_FAIL_OCCUPIED;
        }
      }

      // Check if object is not assigned to any Credential
      if (credentials[i].object_offset == object_offset) {
        credentials_buffer_head = (uint16_t)((credentials_buffer_head + 1)
                                             % max_credentials);
        available = false;
        break; // Try the next object
      }
    }

    if (available) {
      credential_metadata_nvm_t metadata;
      convert_credential_metadata_to_nvm(&metadata, &p_credential->metadata);

      // Write Credential metadata and data in NVM
      if (!nvm(U3C_WRITE, AREA_CREDENTIAL_METADATA, object_offset, &metadata, 0)
          || !nvm(U3C_WRITE, AREA_CREDENTIAL_DATA, object_offset,
                  p_credential->data, metadata.length)) {
        return U3C_DB_OPERATION_RESULT_ERROR_IO;
      }

      // Update descriptor table
      ordered_insert_credential_descriptor(credentials, p_credential, object_offset);

      // Update the descriptor table and number of Credentials in NVM
      if (
        nvm(U3C_WRITE, AREA_CREDENTIAL_DESCRIPTORS, 0, &credentials, 0)
        && nvm(U3C_WRITE, AREA_NUMBER_OF_CREDENTIALS, 0, &n_credentials, 0)) {
        return U3C_DB_OPERATION_RESULT_SUCCESS;
      } else {
        --n_credentials;
        return U3C_DB_OPERATION_RESULT_ERROR_IO;
      }
    }
  }

  // Impossible path! The database is not full, but no free object was found
  return U3C_DB_OPERATION_RESULT_ERROR;
}

u3c_db_operation_result CC_UserCredential_modify_credential(
  u3c_credential_t * p_credential)
{
  // Check if the database is empty
  if (n_credentials < 1) {
    return U3C_DB_OPERATION_RESULT_FAIL_DNE;
  }

  // Read the Credential descriptor table from NVM
  credential_descriptor_t credentials[U3C_BUFFER_SIZE_CREDENTIAL_DESCRIPTORS];
  if (!nvm(U3C_READ, AREA_CREDENTIAL_DESCRIPTORS, 0, &credentials, 0)) {
    return U3C_DB_OPERATION_RESULT_ERROR_IO;
  }

  bool match_any_user = (p_credential->metadata.uuid == 0);

  // Find Credential
  for (uint16_t i = 0; i < n_credentials; ++i) {
    if (credentials[i].credential_type == p_credential->metadata.type
        && credentials[i].credential_slot == p_credential->metadata.slot
        ) {
      uint16_t object_offset = credentials[i].object_offset;

      /**
       * Check if the UUID is being modified. This operation is not allowed.
       * @ref CC_UserCredential_move_credential should be used instead.
       */
      if (!match_any_user
          && (credentials[i].user_unique_identifier != p_credential->metadata.uuid)) {
        return U3C_DB_OPERATION_RESULT_FAIL_REASSIGN;
      }

      // Check whether the incoming credential is identical to the stored one
      if (is_credential_identical(p_credential, object_offset)) {
        return U3C_DB_OPERATION_RESULT_FAIL_IDENTICAL;
      }

      credential_metadata_nvm_t metadata;
      convert_credential_metadata_to_nvm(&metadata, &p_credential->metadata);

      bool nvm_success = true;
      // Overwrite Credential metadata in NVM
      nvm_success &= nvm(U3C_WRITE, AREA_CREDENTIAL_METADATA, object_offset, &metadata, 0);
      // Overwrite Credential data in NVM
      nvm_success &= nvm(U3C_WRITE, AREA_CREDENTIAL_DATA, object_offset,
                         p_credential->data, p_credential->metadata.length);
      return nvm_success ? U3C_DB_OPERATION_RESULT_SUCCESS : U3C_DB_OPERATION_RESULT_ERROR_IO;
    }
  }

  // Credential not found
  return U3C_DB_OPERATION_RESULT_FAIL_DNE;
}

u3c_db_operation_result CC_UserCredential_delete_credential(
  u3c_credential_type credential_type,
  uint16_t credential_slot)
{
  // Check if the database is empty
  if (n_credentials < 1) {
    return U3C_DB_OPERATION_RESULT_FAIL_DNE;
  }

  // Read the Credential descriptor table from NVM
  credential_descriptor_t credentials[U3C_BUFFER_SIZE_CREDENTIAL_DESCRIPTORS];
  if (!nvm(U3C_READ, AREA_CREDENTIAL_DESCRIPTORS, 0, &credentials, 0)) {
    return U3C_DB_OPERATION_RESULT_ERROR_IO;
  }

  // Find Credential
  for (uint16_t i = 0; i < n_credentials; ++i) {
    if (credentials[i].credential_type == credential_type
        && credentials[i].credential_slot == credential_slot) {
      --n_credentials;

      // If the deleted Credential was not the last in the list
      if (i < n_credentials) {
        // Shift the elements to fill the gap
        memmove(&credentials[i], &credentials[i + 1], (n_credentials - i) * sizeof(credential_descriptor_t));
      }
      // Otherwise, simply consider its entry 'popped' from the array.

      // Update the descriptor table
      if (!nvm(U3C_WRITE, AREA_CREDENTIAL_DESCRIPTORS, 0, credentials, 0)) {
        ++n_credentials;
        return U3C_DB_OPERATION_RESULT_ERROR_IO;
      }

      // Update the number of Credentials
      if (!nvm(U3C_WRITE, AREA_NUMBER_OF_CREDENTIALS, 0, &n_credentials, 0)) {
        return U3C_DB_OPERATION_RESULT_ERROR_IO;
      }

      // Make sure the buffer's head is pointing at a valid object
      if (credentials_buffer_head >= n_credentials) {
        credentials_buffer_head = 0;
      }

      return U3C_DB_OPERATION_RESULT_SUCCESS;
    }
  }

  // Credential not found
  return U3C_DB_OPERATION_RESULT_FAIL_DNE;
}

u3c_db_operation_result CC_UserCredential_move_credential(
  u3c_credential_type credential_type,
  uint16_t source_credential_slot, uint16_t destination_user_uid,
  uint16_t destination_credential_slot)
{
  // Check if the database is empty
  if (n_credentials < 1) {
    return U3C_DB_OPERATION_RESULT_FAIL_DNE;
  }

  // Read the Credential descriptor table from NVM
  credential_descriptor_t credentials[U3C_BUFFER_SIZE_CREDENTIAL_DESCRIPTORS];
  if (!nvm(U3C_READ, AREA_CREDENTIAL_DESCRIPTORS, 0, &credentials, 0)) {
    return U3C_DB_OPERATION_RESULT_ERROR_IO;
  }

  bool source_exists = false;
  bool same_slot = (source_credential_slot == destination_credential_slot);
  bool same_uuid = false;
  bool destination_occupied = false;
  uint16_t source_index;

  // Find source and destination Credentials
  for (uint16_t i = 0; i < n_credentials; ++i) {
    if (credentials[i].credential_type == credential_type) {
      // Source credential slot must exist
      if (credentials[i].credential_slot == source_credential_slot) {
        source_index = i;
        source_exists = true;
        if (credentials[i].user_unique_identifier == destination_user_uid) {
          same_uuid = true;
        }
        if (same_slot) {
          break;
        }
      }

      // Destination credential slot must not be occupied if different
      if (!same_slot
          && (credentials[i].credential_slot == destination_credential_slot)
          ) {
        destination_occupied = true;
      }
    }
  }
  if (!source_exists) {
    return U3C_DB_OPERATION_RESULT_FAIL_DNE;
  }
  if (destination_occupied) {
    return U3C_DB_OPERATION_RESULT_FAIL_OCCUPIED;
  }

  uint16_t object_offset = credentials[source_index].object_offset;

  if (!same_uuid) {
    // Change the associated UUID in the stored credential metadata
    credential_metadata_nvm_t metadata = { 0 };
    nvm(U3C_READ, AREA_CREDENTIAL_METADATA, object_offset, &metadata, 0);
    metadata.uuid = destination_user_uid;
    nvm(U3C_WRITE, AREA_CREDENTIAL_METADATA, object_offset, &metadata, 0);
  }

  // Remove the old element from the credential descriptor array
  memmove(&credentials[source_index], &credentials[source_index + 1],
          (n_credentials - source_index) * sizeof(credential_descriptor_t));
  n_credentials--;

  /**
   * Insert the new element into the credential descriptor array with
   * the old credential's object offset.
   * The partially initialized u3c_credential_t struct is only used for passing
   * data to the @ref ordered_insert_credential_descriptor function.
   */
  u3c_credential_t credential = {
    .metadata = {
      .uuid = destination_user_uid,
      .slot = destination_credential_slot,
      .type = credential_type,
    }
  };
  ordered_insert_credential_descriptor(credentials, &credential, object_offset);

  return (
    // Overwrite Credential descriptor table in NVM
    nvm(U3C_WRITE, AREA_CREDENTIAL_DESCRIPTORS, 0, &credentials, 0))
         ? U3C_DB_OPERATION_RESULT_SUCCESS
         : U3C_DB_OPERATION_RESULT_ERROR_IO;
}

u3c_db_operation_result CC_UserCredential_get_admin_code_info(
  u3c_admin_code_metadata_t *code)
{
  admin_pin_code_metadata_nvm_t nvm_code = { 0 };
  if (!nvm(U3C_READ, AREA_ADMIN_PIN_CODE_DATA, 0, &nvm_code, 0)) {
    code->result = ADMIN_CODE_OPERATION_RESULT_ERROR_NODE;
    return U3C_DB_OPERATION_RESULT_ERROR;
  }
  // Move data into known good mirror and return
  memcpy((void*)&admin_code, (void*)&nvm_code, sizeof(admin_pin_code_metadata_nvm_t));
  memcpy((void*)code->code_data, (void*)admin_code.code, admin_code.code_length);
  code->result = ADMIN_CODE_OPERATION_RESULT_GET_RESP;
  code->code_length = admin_code.code_length;
  return U3C_DB_OPERATION_RESULT_SUCCESS;
}

u3c_db_operation_result CC_UserCredential_set_admin_code(
  u3c_admin_code_metadata_t *code)
{
  // Create temporary code struct for write
  admin_pin_code_metadata_nvm_t nvm_code = {
    .code_length = code->code_length,
  };
  memcpy((void*)nvm_code.code, (void*)code->code_data, code->code_length);

  if (!nvm(U3C_WRITE, AREA_ADMIN_PIN_CODE_DATA, 0, &nvm_code, 0)) {
    code->result = ADMIN_CODE_OPERATION_RESULT_ERROR_NODE;
    return U3C_DB_OPERATION_RESULT_ERROR;
  }
  // If successful, update mirror with known good, new value and return
  memcpy((void*)&admin_code, (void*)&nvm_code, sizeof(admin_pin_code_metadata_nvm_t));
  code->result = ADMIN_CODE_OPERATION_RESULT_MODIFIED;
  return U3C_DB_OPERATION_RESULT_SUCCESS;
}
