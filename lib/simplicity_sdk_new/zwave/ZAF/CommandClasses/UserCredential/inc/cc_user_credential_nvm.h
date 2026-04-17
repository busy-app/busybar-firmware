/**
 * @file
 * Non-volatile memory implementation for Command Class User Credential I/O
 *
 * @copyright 2024 Silicon Laboratories Inc.
 */

#ifndef CC_USER_CREDENTIAL_NVM
#define CC_USER_CREDENTIAL_NVM

#include "CC_UserCredential.h"
#include "cc_user_credential_config.h"

/****************************************************************************/
/*                          CONSTANTS and TYPEDEFS                          */
/****************************************************************************/

// Maximum number of User and User Name objects that can be stored in the NVM
#define MAX_USER_OBJECTS                   \
  ZAF_FILE_ID_CC_USER_CREDENTIAL_USER_LAST \
  - ZAF_FILE_ID_CC_USER_CREDENTIAL_USER_BASE
/**
 * Maximum number of Credential and Credential Data objects that can be stored
 * in the NVM
 */
#define MAX_CREDENTIAL_OBJECTS                   \
  ZAF_FILE_ID_CC_USER_CREDENTIAL_CREDENTIAL_LAST \
  - ZAF_FILE_ID_CC_USER_CREDENTIAL_CREDENTIAL_BASE

/**
 * Credential metadata object for storage in NVM.
 */
typedef struct credential_metadata_nvm_t_ {
  uint16_t uuid;
  uint16_t modifier_node_id;
  uint8_t length;
  u3c_modifier_type modifier_type;
} credential_metadata_nvm_t;

/**
 * The User descriptor table is an array of associations between User Unique IDs
 * and file IDs of User objects.
 * Only the entries from ZAF_FILE_ID_CC_USER_CREDENTIAL_CREDENTIAL_BASE to
 * ZAF_FILE_ID_CC_USER_CREDENTIAL_CREDENTIAL_BASE + n_users - 1 are to be
 * considered valid.
 */
typedef struct user_descriptor_t_ {
  uint16_t unique_identifier;
  uint16_t object_offset;
} user_descriptor_t;

/**
 * The Credential descriptor table is an array of associations between unique
 * Credentials and file IDs of Credential metadata objects. A Credential is
 * identified by its owner's User Unique ID and the Credential's type and slot.
 * Only the entries from ZAF_FILE_ID_CC_USER_CREDENTIAL_USER_BASE to
 * ZAF_FILE_ID_CC_USER_CREDENTIAL_USER_BASE + n_credentials - 1 are to be
 * considered valid.
 */
typedef struct credential_descriptor_t_ {
  uint16_t user_unique_identifier;
  uint16_t credential_slot;
  uint16_t object_offset;
  u3c_credential_type credential_type;
} credential_descriptor_t;

/**
 * @brief Metadata to track current admin code information
 */
typedef struct admin_pin_code_metadata_nvm_t_ {
  uint8_t code_length; // Admin Code functionality disabled if 0
  uint8_t code[CC_USER_CREDENTIAL_MAX_DATA_LENGTH_PIN_CODE];
} admin_pin_code_metadata_nvm_t;

typedef enum u3c_nvm_operation_ {
  U3C_READ,
  U3C_WRITE
} u3c_nvm_operation;

typedef enum u3c_nvm_area_ {
  AREA_NUMBER_OF_USERS,
  AREA_NUMBER_OF_CREDENTIALS,
  AREA_USER_DESCRIPTORS,
  AREA_USERS,
  AREA_USER_NAMES,
  AREA_CREDENTIAL_DESCRIPTORS,
  AREA_CREDENTIAL_METADATA,
  AREA_CREDENTIAL_DATA,
  AREA_ADMIN_PIN_CODE_DATA,
} u3c_nvm_area;

#endif /* CC_USER_CREDENTIAL_NVM */
