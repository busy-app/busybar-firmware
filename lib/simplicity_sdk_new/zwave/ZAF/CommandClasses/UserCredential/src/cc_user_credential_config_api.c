#include "cc_user_credential_config_api.h"
#include "cc_user_credential_config.h"
#include "cc_user_credential_io_config.h"
#include "CC_UserCredential.h"
#include "assert.h"

/****************************************************************************/
/*                           STATIC PARAMETER CHECK                         */
/****************************************************************************/

// Ensure that User Name fits into the designated buffer
_Static_assert(CC_USER_CREDENTIAL_MAX_LENGTH_USER_NAME <= U3C_BUFFER_SIZE_USER_NAME,
               "STATIC_ASSERT_FAILED_Maximum_length_User_Name_does_not_fit_in_buffer");

// Ensure that the Credential Data fits into the designated buffer
_Static_assert(CC_USER_CREDENTIAL_MAX_DATA_LENGTH_PIN_CODE <= U3C_BUFFER_SIZE_CREDENTIAL_DATA,
               "STATIC_ASSERT_FAILED_Maximum_length_PIN_Code_data_does_not_fit_in_buffer");
_Static_assert(CC_USER_CREDENTIAL_MAX_DATA_LENGTH_PASSWORD <= U3C_BUFFER_SIZE_CREDENTIAL_DATA,
               "STATIC_ASSERT_FAILED_Maximum_length_Password_data_does_not_fit_in_buffer");
_Static_assert(CC_USER_CREDENTIAL_MAX_DATA_LENGTH_RFID_CODE <= U3C_BUFFER_SIZE_CREDENTIAL_DATA,
               "STATIC_ASSERT_FAILED_Maximum_length_RFID_Code_data_does_not_fit_in_buffer");
_Static_assert(CC_USER_CREDENTIAL_MAX_DATA_LENGTH_BLE <= U3C_BUFFER_SIZE_CREDENTIAL_DATA,
               "STATIC_ASSERT_FAILED_Maximum_length_BLE_data_does_not_fit_in_buffer");
_Static_assert(CC_USER_CREDENTIAL_MAX_DATA_LENGTH_NFC <= U3C_BUFFER_SIZE_CREDENTIAL_DATA,
               "STATIC_ASSERT_FAILED_Maximum_length_NFC_data_does_not_fit_in_buffer");
_Static_assert(CC_USER_CREDENTIAL_MAX_DATA_LENGTH_UWB <= U3C_BUFFER_SIZE_CREDENTIAL_DATA,
               "STATIC_ASSERT_FAILED_Maximum_length_UWB_data_does_not_fit_in_buffer");
_Static_assert(CC_USER_CREDENTIAL_MAX_DATA_LENGTH_EYE_BIOMETRIC <= U3C_BUFFER_SIZE_CREDENTIAL_DATA,
               "STATIC_ASSERT_FAILED_Maximum_length_Eye_Biometric_data_does_not_fit_in_buffer");
_Static_assert(CC_USER_CREDENTIAL_MAX_DATA_LENGTH_FACE_BIOMETRIC <= U3C_BUFFER_SIZE_CREDENTIAL_DATA,
               "STATIC_ASSERT_FAILED_Maximum_length_Face_Biometric_data_does_not_fit_in_buffer");
_Static_assert(CC_USER_CREDENTIAL_MAX_DATA_LENGTH_FINGER_BIOMETRIC <= U3C_BUFFER_SIZE_CREDENTIAL_DATA,
               "STATIC_ASSERT_FAILED_Maximum_length_Finger_Biometric_data_does_not_fit_in_buffer");
_Static_assert(CC_USER_CREDENTIAL_MAX_DATA_LENGTH_HAND_BIOMETRIC <= U3C_BUFFER_SIZE_CREDENTIAL_DATA,
               "STATIC_ASSERT_FAILED_Maximum_length_Hand_Biometric_data_does_not_fit_in_buffer");
_Static_assert(CC_USER_CREDENTIAL_MAX_DATA_LENGTH_UNSPECIFIED_BIOMETRIC <= U3C_BUFFER_SIZE_CREDENTIAL_DATA,
               "STATIC_ASSERT_FAILED_Maximum_length_Unspecified_Biometric_data_does_not_fit_in_buffer");

// Ensure Min and Max User Name Length are even numbers for potential UTF-16 encoding
_Static_assert((CC_USER_CREDENTIAL_MAX_LENGTH_USER_NAME & 0x01)  == 0,
               "STATIC_ASSERT_FAILED_Maximum_length_User_Name_must_be_even_number");

// Ensure data Min Length is not greater than data Max Length
_Static_assert(CC_USER_CREDENTIAL_MIN_DATA_LENGTH_PIN_CODE <= CC_USER_CREDENTIAL_MAX_DATA_LENGTH_PIN_CODE,
               "STATIC_ASSERT_FAILED_Minimum_length_PIN_Code_data_must_be_less_than_maximum_length");
_Static_assert(CC_USER_CREDENTIAL_MIN_DATA_LENGTH_PASSWORD <= CC_USER_CREDENTIAL_MAX_DATA_LENGTH_PASSWORD,
               "STATIC_ASSERT_FAILED_Minimum_length_Password_data_must_be_less_than_maximum_length");
_Static_assert(CC_USER_CREDENTIAL_MIN_DATA_LENGTH_RFID_CODE <= CC_USER_CREDENTIAL_MAX_DATA_LENGTH_RFID_CODE,
               "STATIC_ASSERT_FAILED_Minimum_length_RFID_Code_data_must_be_less_than_maximum_length");
_Static_assert(CC_USER_CREDENTIAL_MIN_DATA_LENGTH_BLE <= CC_USER_CREDENTIAL_MAX_DATA_LENGTH_BLE,
               "STATIC_ASSERT_FAILED_Minimum_length_BLE_data_must_be_less_than_maximum_length");
_Static_assert(CC_USER_CREDENTIAL_MIN_DATA_LENGTH_NFC <= CC_USER_CREDENTIAL_MAX_DATA_LENGTH_NFC,
               "STATIC_ASSERT_FAILED_Minimum_length_NFC_data_must_be_less_than_maximum_length");
_Static_assert(CC_USER_CREDENTIAL_MIN_DATA_LENGTH_UWB <= CC_USER_CREDENTIAL_MAX_DATA_LENGTH_UWB,
               "STATIC_ASSERT_FAILED_Minimum_length_UWB_data_must_be_less_than_maximum_length");
_Static_assert(CC_USER_CREDENTIAL_MIN_DATA_LENGTH_EYE_BIOMETRIC <= CC_USER_CREDENTIAL_MAX_DATA_LENGTH_EYE_BIOMETRIC,
               "STATIC_ASSERT_FAILED_Minimum_length_Eye_Biometric_data_must_be_less_than_maximum_length");
_Static_assert(CC_USER_CREDENTIAL_MIN_DATA_LENGTH_FACE_BIOMETRIC <= CC_USER_CREDENTIAL_MAX_DATA_LENGTH_FACE_BIOMETRIC,
               "STATIC_ASSERT_FAILED_Minimum_length_Face_Biometric_data_must_be_less_than_maximum_length");
_Static_assert(CC_USER_CREDENTIAL_MIN_DATA_LENGTH_FINGER_BIOMETRIC <= CC_USER_CREDENTIAL_MAX_DATA_LENGTH_FINGER_BIOMETRIC,
               "STATIC_ASSERT_FAILED_Minimum_length_Finger_Biometric_data_must_be_less_than_maximum_length");
_Static_assert(CC_USER_CREDENTIAL_MIN_DATA_LENGTH_HAND_BIOMETRIC <= CC_USER_CREDENTIAL_MAX_DATA_LENGTH_HAND_BIOMETRIC,
               "STATIC_ASSERT_FAILED_Minimum_length_Hand_Biometric_data_must_be_less_than_maximum_length");
_Static_assert(CC_USER_CREDENTIAL_MIN_DATA_LENGTH_UNSPECIFIED_BIOMETRIC <= CC_USER_CREDENTIAL_MAX_DATA_LENGTH_UNSPECIFIED_BIOMETRIC,
               "STATIC_ASSERT_FAILED_Minimum_length_Unspecified_Biometric_data_must_be_less_than_maximum_length");

/****************************************************************************/
/*                               PRIVATE DATA                               */
/****************************************************************************/

/**
 * The nth bit of the byte corresponds to support for the nth User Type,
 * defined in the enum u3c_user_type.
 */
static uint16_t user_type_support =
  CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_GENERAL << USER_TYPE_GENERAL
    | CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_PROGRAMMING << USER_TYPE_PROGRAMMING
    | CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_NON_ACCESS << USER_TYPE_NON_ACCESS
    | CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_DURESS << USER_TYPE_DURESS
    | CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_DISPOSABLE << USER_TYPE_DISPOSABLE
    | CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_EXPIRING << USER_TYPE_EXPIRING
    | CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_REMOTE_ONLY << USER_TYPE_REMOTE_ONLY;

/**
 * The n-1st element of the array corresponds to the maximum number of slots for
 * the nth Credential Type, defined in the enum u3c_credential_type.
 */
static uint16_t credential_max_slots[] = {
  CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_PIN_CODE,
  CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_PASSWORD,
  CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_RFID_CODE,
  CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_BLE,
  CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_NFC,
  CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_UWB,
  CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_EYE_BIOMETRIC,
  CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_FACE_BIOMETRIC,
  CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_FINGER_BIOMETRIC,
  CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_HAND_BIOMETRIC,
  CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_UNSPECIFIED_BIOMETRIC
};

// Number of all known Credential Types
static uint16_t n_credential_types = sizeof(credential_max_slots) / sizeof(uint16_t);

/**
 * The nth bit of the word corresponds to Credential Learn Support for the nth
 * Credential Type, defined in the enum u3c_credential_type.
 */
static uint16_t credential_learn_support =
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_PIN_CODE << CREDENTIAL_TYPE_PIN_CODE
    | CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_PASSWORD << CREDENTIAL_TYPE_PASSWORD
    | CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_RFID_CODE << CREDENTIAL_TYPE_RFID_CODE
    | CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_BLE << CREDENTIAL_TYPE_BLE
    | CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_NFC << CREDENTIAL_TYPE_NFC
    | CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_UWB << CREDENTIAL_TYPE_UWB
    | CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_EYE_BIOMETRIC << CREDENTIAL_TYPE_EYE_BIOMETRIC
    | CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_FACE_BIOMETRIC << CREDENTIAL_TYPE_FACE_BIOMETRIC
    | CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_FINGER_BIOMETRIC << CREDENTIAL_TYPE_FINGER_BIOMETRIC
    | CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_HAND_BIOMETRIC << CREDENTIAL_TYPE_HAND_BIOMETRIC
    | CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_UNSPECIFIED_BIOMETRIC << CREDENTIAL_TYPE_UNSPECIFIED_BIOMETRIC;

/**
 * The nth bit of the byte corresponds to Credential Rule Support for the nth
 * Credential Type, defined in the enum u3c_credential_type.
 */
static uint8_t credential_rule_support = {
  CC_USER_CREDENTIAL_CREDENTIAL_RULE_SUPPORTED_SINGLE << CREDENTIAL_RULE_SINGLE
    | CC_USER_CREDENTIAL_CREDENTIAL_RULE_SUPPORTED_DUAL << CREDENTIAL_RULE_DUAL
    | CC_USER_CREDENTIAL_CREDENTIAL_RULE_SUPPORTED_TRIPLE << CREDENTIAL_RULE_TRIPLE
};

/**
 * The n-1st element of the array corresponds to the minimum length of data for
 * the nth Credential Type, defined in the enum u3c_credential_type.
 */
static uint8_t credential_min_length_of_data[] = {
  CC_USER_CREDENTIAL_MIN_DATA_LENGTH_PIN_CODE,
  CC_USER_CREDENTIAL_MIN_DATA_LENGTH_PASSWORD,
  CC_USER_CREDENTIAL_MIN_DATA_LENGTH_RFID_CODE,
  CC_USER_CREDENTIAL_MIN_DATA_LENGTH_BLE,
  CC_USER_CREDENTIAL_MIN_DATA_LENGTH_NFC,
  CC_USER_CREDENTIAL_MIN_DATA_LENGTH_UWB,
  CC_USER_CREDENTIAL_MIN_DATA_LENGTH_EYE_BIOMETRIC,
  CC_USER_CREDENTIAL_MIN_DATA_LENGTH_FACE_BIOMETRIC,
  CC_USER_CREDENTIAL_MIN_DATA_LENGTH_FINGER_BIOMETRIC,
  CC_USER_CREDENTIAL_MIN_DATA_LENGTH_HAND_BIOMETRIC,
  CC_USER_CREDENTIAL_MIN_DATA_LENGTH_UNSPECIFIED_BIOMETRIC
};

/**
 * The n-1st element of the array corresponds to the maximum length of data for
 * the nth Credential Type, defined in the enum u3c_credential_rule.
 */
static uint8_t credential_max_length_of_data[] = {
  CC_USER_CREDENTIAL_MAX_DATA_LENGTH_PIN_CODE,
  CC_USER_CREDENTIAL_MAX_DATA_LENGTH_PASSWORD,
  CC_USER_CREDENTIAL_MAX_DATA_LENGTH_RFID_CODE,
  CC_USER_CREDENTIAL_MAX_DATA_LENGTH_BLE,
  CC_USER_CREDENTIAL_MAX_DATA_LENGTH_NFC,
  CC_USER_CREDENTIAL_MAX_DATA_LENGTH_UWB,
  CC_USER_CREDENTIAL_MAX_DATA_LENGTH_EYE_BIOMETRIC,
  CC_USER_CREDENTIAL_MAX_DATA_LENGTH_FACE_BIOMETRIC,
  CC_USER_CREDENTIAL_MAX_DATA_LENGTH_FINGER_BIOMETRIC,
  CC_USER_CREDENTIAL_MAX_DATA_LENGTH_HAND_BIOMETRIC,
  CC_USER_CREDENTIAL_MAX_DATA_LENGTH_UNSPECIFIED_BIOMETRIC
};

/**
 * The n-1st element of the array corresponds to the Credential Learn
 * Recommended timeout for the nth Credential Type, defined in the enum
 * u3c_credential_rule.
 */
static uint8_t credential_learn_recommended_timeout[] = {
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_PIN_CODE,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_PASSWORD,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_RFID_CODE,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_BLE,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_NFC,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_UWB,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_EYE_BIOMETRIC,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_FACE_BIOMETRIC,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_FINGER_BIOMETRIC,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_HAND_BIOMETRIC,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_UNSPECIFIED_BIOMETRIC
};

/**
 * The n-1st element of the array corresponds to the Credential Learn Number of
 * Steps for the nth Credential Type, defined in the enum u3c_credential_rule.
 */
static uint8_t credential_learn_number_of_steps[] = {
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_PIN_CODE,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_PASSWORD,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_RFID_CODE,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_BLE,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_NFC,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_UWB,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_EYE_BIOMETRIC,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_FACE_BIOMETRIC,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_FINGER_BIOMETRIC,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_HAND_BIOMETRIC,
  CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_UNSPECIFIED_BIOMETRIC
};

/**
 * The n-1st element of the array corresponds to the maximum length of hash for
 * the nth Credential Type, defined in the enum u3c_credential_rule.
 */
static uint8_t credential_max_length_of_hash[] = {
  CC_USER_CREDENTIAL_MAX_HASH_LENGTH_PIN_CODE,
  CC_USER_CREDENTIAL_MAX_HASH_LENGTH_PASSWORD,
  CC_USER_CREDENTIAL_MAX_HASH_LENGTH_RFID_CODE,
  CC_USER_CREDENTIAL_MAX_HASH_LENGTH_BLE,
  CC_USER_CREDENTIAL_MAX_HASH_LENGTH_NFC,
  CC_USER_CREDENTIAL_MAX_HASH_LENGTH_UWB,
  CC_USER_CREDENTIAL_MAX_HASH_LENGTH_EYE_BIOMETRIC,
  CC_USER_CREDENTIAL_MAX_HASH_LENGTH_FACE_BIOMETRIC,
  CC_USER_CREDENTIAL_MAX_HASH_LENGTH_FINGER_BIOMETRIC,
  CC_USER_CREDENTIAL_MAX_HASH_LENGTH_HAND_BIOMETRIC,
  CC_USER_CREDENTIAL_MAX_HASH_LENGTH_UNSPECIFIED_BIOMETRIC
};

/****************************************************************************/
/*                             PRIVATE FUNCTIONS                            */
/****************************************************************************/

bool is_credential_type_in_range(u3c_credential_type credential_type)
{
  return (credential_type > CREDENTIAL_TYPE_NONE)
         && (credential_type <= CREDENTIAL_TYPE_UNSPECIFIED_BIOMETRIC);
}

/****************************************************************************/
/*                             PUBLIC FUNCTIONS                             */
/****************************************************************************/

ZW_WEAK bool cc_user_credential_is_user_type_supported(u3c_user_type user_type)
{
  if (user_type <= USER_TYPE_REMOTE_ONLY) {
    return (user_type_support >> user_type) & 0x01;
  }
  return false;
}

ZW_WEAK uint16_t cc_user_credential_get_max_user_unique_idenfitiers(void)
{
  return CC_USER_CREDENTIAL_MAX_USER_UNIQUE_IDENTIFIERS;
}

ZW_WEAK uint8_t cc_user_credential_get_max_length_of_user_name(void)
{
  return CC_USER_CREDENTIAL_MAX_LENGTH_USER_NAME;
}

ZW_WEAK uint16_t cc_user_credential_get_max_credential_slots(u3c_credential_type credential_type)
{
  uint16_t max_slots = 0;
  if (is_credential_type_in_range(credential_type)) {
    max_slots = credential_max_slots[credential_type - 1];
  }
  return max_slots;
}

ZW_WEAK bool cc_user_credential_is_credential_type_supported(u3c_credential_type credential_type)
{
  return cc_user_credential_get_max_credential_slots(credential_type) > 0;
}

ZW_WEAK bool cc_user_credential_is_credential_rule_supported(u3c_credential_rule credential_rule)
{
  return (credential_rule_support >> credential_rule) & 0x01;
}

ZW_WEAK uint8_t cc_user_credential_get_number_of_supported_credential_types(void)
{
  uint8_t number_of_supported_credential_types = 0;
  for (u3c_credential_type i = CREDENTIAL_TYPE_PIN_CODE; i <= n_credential_types; ++i) {
    if (cc_user_credential_is_credential_type_supported(i) > 0) {
      ++number_of_supported_credential_types;
    }
  }
  return number_of_supported_credential_types;
}

ZW_WEAK bool cc_user_credential_is_credential_learn_supported(u3c_credential_type credential_type)
{
  return (credential_learn_support >> credential_type) & 0x01;
}

ZW_WEAK uint8_t cc_user_credential_get_min_length_of_data(u3c_credential_type credential_type)
{
  uint8_t min_length = 0;
  if (is_credential_type_in_range(credential_type)) {
    min_length = credential_min_length_of_data[credential_type - 1];
  }
  return min_length;
}

ZW_WEAK uint8_t cc_user_credential_get_max_length_of_data(u3c_credential_type credential_type)
{
  uint8_t max_length = 0;
  if (is_credential_type_in_range(credential_type)) {
    max_length = credential_max_length_of_data[credential_type - 1];
  }
  return max_length;
}

ZW_WEAK uint8_t cc_user_credential_get_max_hash_length(u3c_credential_type type)
{
  uint8_t max_hash_length = 0;
  if (is_credential_type_in_range(type)) {
    max_hash_length = credential_max_length_of_hash[type - 1];
  }
  return max_hash_length;
}

ZW_WEAK uint8_t cc_user_credential_get_cl_recommended_timeout(u3c_credential_type credential_type)
{
  uint8_t cl_recommended_timeout = 0;
  if (is_credential_type_in_range(credential_type)
      && cc_user_credential_is_credential_learn_supported(credential_type)) {
    cl_recommended_timeout = credential_learn_recommended_timeout[credential_type - 1];
  }
  return cl_recommended_timeout;
}

ZW_WEAK uint8_t cc_user_credential_get_cl_number_of_steps(u3c_credential_type credential_type)
{
  uint8_t cl_number_of_steps = 0;
  if (is_credential_type_in_range(credential_type)
      && cc_user_credential_is_credential_learn_supported(credential_type)) {
    cl_number_of_steps =  credential_learn_number_of_steps[credential_type - 1];
  }
  return cl_number_of_steps;
}

ZW_WEAK bool cc_user_credential_is_all_users_checksum_supported(void)
{
  return CC_USER_CREDENTIAL_ALL_USERS_CHECKSUM_SUPPORTED;
}

ZW_WEAK bool cc_user_credential_is_user_checksum_supported(void)
{
  return CC_USER_CREDENTIAL_USER_CHECKSUM_SUPPORTED;
}

ZW_WEAK bool cc_user_credential_is_credential_checksum_supported(void)
{
  return CC_USER_CREDENTIAL_CREDENTIAL_CHECKSUM_SUPPORTED;
}

ZW_WEAK bool cc_user_credential_get_admin_code_supported(void)
{
  return (CC_USER_CREDENTIAL_ADMIN_CODE_SUPPORTED == 1)
         && (CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_PIN_CODE > 0); // Shorthand for pin code support.
}

ZW_WEAK bool cc_user_credential_get_admin_code_deactivate_supported(void)
{
  return (CC_USER_CREDENTIAL_ADMIN_CODE_DEACTIVATE_SUPPORTED == 1)
         && (CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_PIN_CODE > 0);
}
