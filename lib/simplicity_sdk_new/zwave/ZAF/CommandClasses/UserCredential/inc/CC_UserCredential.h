/**
 * @file
 * @brief Handler and API for Command Class User Credential.
 *
 * @details The User Credential Command Class is used to manage user Credentials
 * for gaining access to properties, typically through unlocking or opening
 * doors. This command class employs a user-centric model, allowing multiple
 * Credentials to be associated with one User Unique Identifier.
 *
 * @copyright 2023 Silicon Laboratories Inc.
 */

#ifndef _COMMANDCLASSUSERCREDENTIAL_H_
#define _COMMANDCLASSUSERCREDENTIAL_H_

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/

#include <stdbool.h>
#include "CC_Common.h"
#include "ZW_classcmd.h"
#include "cc_user_credential_config.h"

/**
 * @addtogroup CC
 * @{
 * @addtogroup UserCredential
 * @{
 */

/****************************************************************************/
/*                      EXPORTED TYPES and DEFINITIONS                      */
/****************************************************************************/

#define U3C_CREDENTIAL_TYPE_PIN_CODE_MIN_LENGTH_REQUIREMENT 4
#define U3C_CREDENTIAL_TYPE_PIN_CODE_MAX_LENGTH_REQUIREMENT 10
#define ASCII_AC_MAX 0x7F //< Maximum Admin PIN code character (highest ASCII code)
#define AC_MIN_LENGTH 4   //< Minimum Admin PIN code length
#define AC_MAX_LENGTH 10  //< Maximum Admin PIN code length
#define U3C_BUFFER_SIZE_USER_NAME CC_USER_CREDENTIAL_MAX_LENGTH_USER_NAME

/// Longest possible default user name: "User-65535", times 2 for UTF-16
#define DEFAULT_NAME_MAX_SIZE (sizeof("User-65535") - 1) * 2

// Number of bytes of a credential set frame if credential length and credential data fields are empty.
#define CREDENTIAL_SET_FRAME_LENGTH_WITHOUT_CREDENTIAL_LENGTH (offsetof(ZW_CREDENTIAL_SET_1BYTE_FRAME, credentialLength))
// Number of bytes of a credential set frame if credential data field is empty.
#define CREDENTIAL_SET_FRAME_LENGTH_WITHOUT_CREDENTIAL_DATA (offsetof(ZW_CREDENTIAL_SET_1BYTE_FRAME, credentialData1))

typedef enum u3c_modifier_type_ {
  MODIFIER_TYPE_DNE = CREDENTIAL_REPORT_DNE,
  MODIFIER_TYPE_UNKNOWN,
  MODIFIER_TYPE_Z_WAVE,
  MODIFIER_TYPE_LOCALLY,
  MODIFIER_TYPE_MOBILE_APP_OR_OTHER_IOT_TECHNOLOGY
} u3c_modifier_type;

typedef enum u3c_credential_type_ {
  CREDENTIAL_TYPE_NONE,
  CREDENTIAL_TYPE_PIN_CODE = CREDENTIAL_REPORT_PIN_CODE,
  CREDENTIAL_TYPE_PASSWORD,
  CREDENTIAL_TYPE_RFID_CODE,
  CREDENTIAL_TYPE_BLE,
  CREDENTIAL_TYPE_NFC,
  CREDENTIAL_TYPE_UWB,
  CREDENTIAL_TYPE_EYE_BIOMETRIC,
  CREDENTIAL_TYPE_FACE_BIOMETRIC,
  CREDENTIAL_TYPE_FINGER_BIOMETRIC,
  CREDENTIAL_TYPE_HAND_BIOMETRIC,
  CREDENTIAL_TYPE_UNSPECIFIED_BIOMETRIC,
  CREDENTIAL_TYPE_NUMBER_OF_TYPES
} u3c_credential_type;

typedef enum _u3c_credential_report_type_t_ {
  CREDENTIAL_REP_TYPE_ADDED =                       CREDENTIAL_REPORT_ADDED,
  CREDENTIAL_REP_TYPE_MODIFIED =                    CREDENTIAL_REPORT_MODIFIED,
  CREDENTIAL_REP_TYPE_DELETED =                     CREDENTIAL_REPORT_DELETED,
  CREDENTIAL_REP_TYPE_UNCHANGED =                   CREDENTIAL_REPORT_UNCHANGED,
  CREDENTIAL_REP_TYPE_RESPONSE_TO_GET =             CREDENTIAL_REPORT_RESPONSE_TO_GET,
  CREDENTIAL_REP_TYPE_ADD_AGAINST_OCCUPIED =        CREDENTIAL_REPORT_ADD_AGAINST_OCCUPIED,
  CREDENTIAL_REP_TYPE_MODIF_AGAINST_EMPTY =         CREDENTIAL_REPORT_MODIFY_AGAINST_EMPTY,
  CREDENTIAL_REP_TYPE_DUPLICATE =                   CREDENTIAL_REPORT_DUPLICATE,
  CREDENTIAL_REP_TYPE_MANUFACTURER_SECURITY_RULES = CREDENTIAL_REPORT_MANUFACTURER_SECURITY_RULES,
  CREDENTIAL_REP_TYPE_WRONG_UUID =                  CREDENTIAL_REPORT_WRONG_USER_UNIQUE_IDENTIFIER,
  CREDENTIAL_REP_TYPE_DUPLICATE_ADMIN_PIN_CODE =    CREDENTIAL_REPORT_DUPLICATE_ADMIN_PIN_CODE
} u3c_credential_report_type_t;

typedef enum u3c_credential_learn_status_t_ {
  CL_STATUS_STARTED =                       CREDENTIAL_LEARN_REPORT_STARTED,
  CL_STATUS_SUCCESS =                       CREDENTIAL_LEARN_REPORT_SUCCESS,
  CL_STATUS_ALREADY_IN_PROGRESS =           CREDENTIAL_LEARN_REPORT_ALREADY_IN_PROGRESS,
  CL_STATUS_ENDED_NOT_DUE_TO_TIMEOUT =      CREDENTIAL_LEARN_REPORT_ENDED_NOT_DUE_TO_TIMEOUT,
  CL_STATUS_TIMEOUT =                       CREDENTIAL_LEARN_REPORT_TIMEOUT,
  CL_STATUS_STEP_RETRY =                    CREDENTIAL_LEARN_REPORT_CREDENTIAL_LEARN_STEP_RETRY,
  CL_STATUS_INVALID_ADD_OPERATION_TYPE =    CREDENTIAL_LEARN_REPORT_INVALID_CREDENTIAL_LEARN_ADD_OPERATION_TYPE,
  CL_STATUS_INVALID_MODIFY_OPERATION_TYPE = CREDENTIAL_LEARN_REPORT_INVALID_CREDENTIAL_LEARN_MODIFY_OPERATION_TYPE
} u3c_credential_learn_status_t;

typedef enum {
  U3C_UCAR_STATUS_SUCCESS                                        = USER_CREDENTIAL_ASSOCIATION_REPORT_SUCCESS,
  U3C_UCAR_STATUS_CREDENTIAL_TYPE_INVALID                        = USER_CREDENTIAL_ASSOCIATION_REPORT_CREDENTIAL_TYPE_INVALID,
  U3C_UCAR_STATUS_CREDENTIAL_SLOT_INVALID                        = USER_CREDENTIAL_ASSOCIATION_REPORT_CREDENTIAL_SLOT_INVALID,
  U3C_UCAR_STATUS_CREDENTIAL_SLOT_EMPTY                          = USER_CREDENTIAL_ASSOCIATION_REPORT_CREDENTIAL_SLOT_EMPTY,
  U3C_UCAR_STATUS_DESTINATION_USER_UNIQUE_IDENTIFIER_INVALID     = USER_CREDENTIAL_ASSOCIATION_REPORT_DESTINATION_USER_UNIQUE_IDENTIFIER_INVALID,
  U3C_UCAR_STATUS_DESTINATION_USER_UNIQUE_IDENTIFIER_NONEXISTENT = USER_CREDENTIAL_ASSOCIATION_REPORT_DESTINATION_USER_UNIQUE_IDENTIFIER_NONEXISTENT,
} u3c_user_credential_association_report_status_t;

typedef enum u3c_user_report_type_t_ {
  USER_REP_TYPE_ADDED                = USER_REPORT_ADDED,
  USER_REP_TYPE_MODIFIED             = USER_REPORT_MODIFIED,
  USER_REP_TYPE_DELETED              = USER_REPORT_DELETED,
  USER_REP_TYPE_UNCHANGED            = USER_REPORT_UNCHANGED,
  USER_REP_TYPE_RESPONSE_TO_GET      = USER_REPORT_RESPONSE_TO_GET,
  USER_REP_TYPE_ADD_AGAINST_OCCUPIED = USER_REPORT_ADD_AGAINST_OCCUPIED,
  USER_REP_TYPE_MODIF_AGAINST_EMPTY  = USER_REPORT_MODIFY_AGAINST_EMPTY,
  USER_REP_TYPE_Z_EXP_MIN_INVALID    = USER_REPORT_ZERO_EXPIRING_MINUTES_INVALID
} u3c_user_report_type_t;

typedef enum u3c_user_type_ {
  USER_TYPE_GENERAL = 0x00,
  USER_TYPE_PROGRAMMING = 0x03,
  USER_TYPE_NON_ACCESS = 0x04,
  USER_TYPE_DURESS = 0x05,
  USER_TYPE_DISPOSABLE = 0x06,
  USER_TYPE_EXPIRING = 0x07,
  USER_TYPE_REMOTE_ONLY = 0x09,
  USER_TYPE_END
} u3c_user_type;

typedef enum u3c_credential_rule_ {
  CREDENTIAL_RULE_SINGLE = 0x01,
  CREDENTIAL_RULE_DUAL,
  CREDENTIAL_RULE_TRIPLE
} u3c_credential_rule;

typedef enum u3c_user_name_encoding_ {
  USER_NAME_ENCODING_STANDARD_ASCII,
  USER_NAME_ENCODING_STANDARD_AND_OEM_EXTENDED_ASCII,
  USER_NAME_ENCODING_UNICODE_UTF_16
} u3c_user_name_encoding;

/**
 * @brief This operation result can optionally be injected
 * into an Admin Code Report to pass information back to
 * the controlling node, in lieu of another report type.
 *
 * Reports MUST be sent for Get operations, but MAY be sent for Set operations.
 *
 * See specification for more requirement details.
 */
typedef enum _u3c_admin_code_operation_result_ {
  ADMIN_CODE_OPERATION_RESULT_MODIFIED = 0x01,                ///< Admin Code was modified (Set, External).
  ADMIN_CODE_OPERATION_RESULT_UNMODIFIED = 0x03,              ///< Admin code was not modified as current code was identical.
  ADMIN_CODE_OPERATION_RESULT_GET_RESP = 0x04,                ///< Default response, or response to successful Get.
  ADMIN_CODE_OPERATION_RESULT_FAIL_DUPLICATE_CRED = 0x07,     ///< Admin Code was not modified (Set) due to duplicate existing PIN code credential.
  ADMIN_CODE_OPERATION_RESULT_FAIL_MANUF_RULE = 0x08,         ///< Admin Code was not modified (Set) due to a manufacturer security rule.
  ADMIN_CODE_OPERATION_RESULT_ERROR_AC_NOT_SUPPORTED = 0x0D,  ///< Failed due to Admin Code not being supported (Get/Set).
  ADMIN_CODE_OPERATION_RESULT_ERROR_ACD_NOT_SUPPORTED = 0x0E, ///< Failed due to Admin Code Deactivation not being supported (Set).
  ADMIN_CODE_OPERATION_RESULT_ERROR_NODE = 0x0F,              ///< Failed due to unspecified node error
  // Internal result values selected to mask to appropriate result codes in the report
  ADMIN_CODE_OPERATION_RESULT_INTERNAL_DUPLICATE_AC = 0x13,   ///< Internal validation result; Code is the same as the current admin code
  ADMIN_CODE_OPERATION_RESULT_INTERNAL_NONE = 0x14,           ///< Internal validation result; No error
  ADMIN_CODE_OPERATION_RESULT_INTERNAL_INVALID_LENGTH = 0x1F, ///< Internal validation result; Code has an invalid length
  ADMIN_CODE_OPERATION_RESULT_INTERNAL_INVALID_CHAR = 0x2F,   ///< Internal validation result; Code has one or more invalid characters
} u3c_admin_code_operation_result;

typedef struct u3c_user_t_ {
  uint16_t unique_identifier;
  uint16_t modifier_node_id;
  uint16_t expiring_timeout_minutes;
  uint8_t name_length;
  u3c_user_type type;
  u3c_modifier_type modifier_type;
  u3c_credential_rule credential_rule;
  u3c_user_name_encoding name_encoding;
  bool active;
} u3c_user_t;

typedef struct u3c_credential_metadata_t_ {
  uint16_t uuid;
  uint16_t slot;
  uint16_t modifier_node_id;
  uint8_t length;
  u3c_modifier_type modifier_type;
  u3c_credential_type type;
} u3c_credential_metadata_t;

typedef struct u3c_credential_t_ {
  u3c_credential_metadata_t metadata;
  uint8_t * data;
} u3c_credential_t;

typedef enum {
  U3C_OPERATION_TYPE_ADD    = 0,
  U3C_OPERATION_TYPE_MODIFY = 1,
  U3C_OPERATION_TYPE_DELETE = 2
} u3c_operation_type_t;

typedef struct u3c_credential_identifier_t_ {
  uint16_t uuid;
  u3c_credential_type type;
  uint16_t slot;
} u3c_credential_identifier_t;

/// Payload for CC_USER_CREDENTIAL_EVENT_VALIDATE, received from the application
typedef struct u3c_event_data_validate_t_ {
  u3c_credential_t * credential;
  bool is_unlocked;
} u3c_event_data_validate_t;

/// Payload for CC_USER_CREDENTIAL_EVENT_LEARN_READ_DONE, passed to the application
typedef struct u3c_event_data_learn_read_done_t_ {
  uint8_t * data;
  uint8_t data_length;
} u3c_event_data_learn_read_done_t;

/// Credential Learn information passed to application layer
typedef struct u3c_event_data_learn_start_t_ {
  u3c_credential_identifier_t target;
  u3c_operation_type_t operation_type;
  uint16_t source_id;
  uint8_t timeout_seconds;
} u3c_event_data_learn_start_t;

typedef struct u3c_admin_code_metadata_t_ {
  u3c_admin_code_operation_result result;
  uint8_t code_length;
  // Admin code can't be any longer than the max length pin code
  uint8_t code_data[U3C_CREDENTIAL_TYPE_PIN_CODE_MAX_LENGTH_REQUIREMENT];
} u3c_admin_code_metadata_t;

typedef enum u3c_event_ {
  CC_USER_CREDENTIAL_EVENT_VALIDATE            = 1,  ///< Application is requesting a Credential to be validated
  CC_USER_CREDENTIAL_EVENT_VALIDATE_VALID      = 2,  ///< Credential found and User has sufficient rights
  CC_USER_CREDENTIAL_EVENT_VALIDATE_INVALID    = 3,  ///< Credential not found or User has insufficient rights
  CC_USER_CREDENTIAL_EVENT_LEARN_START_REQUEST = 4,  ///< Application is requesting to start Credential Learn
  CC_USER_CREDENTIAL_EVENT_LEARN_START         = 5,  ///< Credential Learn process started
  CC_USER_CREDENTIAL_EVENT_LEARN_STEP_START    = 6,  ///< Application progressed to new Credential Learn step
  CC_USER_CREDENTIAL_EVENT_LEARN_READ_DONE     = 7,  ///< Credential data is ready to be entered into the database
  CC_USER_CREDENTIAL_EVENT_LEARN_SUCCESS       = 8,  ///< Credential has been accepted into the database
  CC_USER_CREDENTIAL_EVENT_LEARN_FAILED        = 9,
  CC_USER_CREDENTIAL_EVENT_LEARN_STEP_RETRY    = 10, ///< Local Credential read failed, trying to read again
  CC_USER_CREDENTIAL_EVENT_LEARN_TIMEOUT       = 11, ///< Credential Learn step timed out
  CC_USER_CREDENTIAL_EVENT_LEARN_CANCEL_REMOTE = 12, ///< Remote node cancelled Credential Learn process
  CC_USER_CREDENTIAL_EVENT_LEARN_CANCEL_LOCAL  = 13  ///< Application cancelled Credential Learn process
} u3c_event_type;

/****************************************************************************/
/*                              API FUNCTIONS                               */
/****************************************************************************/

/**
 * Validates a Credential according to the manufacturer's security rules
 *
 * @return True if the Credential conforms to the security rules
 */
bool CC_UserCredential_manufacturer_validate_credential(
  const u3c_credential_t * const credential
  );

/**
 * @brief Validates the following conditions and requirements:
 * CC:0083.01.1A.11.011 - Code follows all manufacturer security rules.
 *
 * Should not be called if AC is not supported.
 *
 * @param code Pointer to structure containing Admin Code information.
 *
 * @returns True if all checks pass. If checks do not pass, returns false, and
 * result code populated in code struct.
 *
 * @note result code of a successful check should be ignored, but in
 * practice ensure it is set to ADMIN_CODE_OPERATION_RESULT_NONE in this case.
 */
bool CC_UserCredential_manufacturer_validate_admin_pin_code(
  u3c_admin_code_metadata_t * const code
  );

/**
 * Generates a default User Name value with either ASCII or UTF-16 encoding,
 * according to CC:0083.01.05.12.052
 *
 * @param[out] pName Pointer to the User Name
 * @param[in] pUser Pointer to the User object
 */
void CC_UserCredential_set_default_name(
  uint8_t * pName,
  u3c_user_t * pUser
  );

/**
 * @}
 * @}
 */

#endif /* _COMMANDCLASSUSERCREDENTIAL_H_ */
