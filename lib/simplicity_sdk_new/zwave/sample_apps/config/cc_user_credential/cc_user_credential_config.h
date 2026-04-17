/***************************************************************************//**
 * # License
 * <b> Copyright 2024 Silicon Laboratories Inc. www.silabs.com </b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of the Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * https://www.silabs.com/about-us/legal/master-software-license-agreement
 * By installing, copying or otherwise using this software, you agree to the
 * terms of the MSLA.
 *
 ******************************************************************************/

#ifndef _CC_USER_CREDENTIAL_CONFIG_H_
#define _CC_USER_CREDENTIAL_CONFIG_H_

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Users

// <o CC_USER_CREDENTIAL_MAX_USER_UNIQUE_IDENTIFIERS> Number of supported User Unique Identifiers <1..65535:1> <f.d>
// <i> Default: 20
#define CC_USER_CREDENTIAL_MAX_USER_UNIQUE_IDENTIFIERS  20

// <o CC_USER_CREDENTIAL_MAX_LENGTH_USER_NAME> Maximum length of User Name <1..255:2> <f.d>
// <i> Default: 16
#define CC_USER_CREDENTIAL_MAX_LENGTH_USER_NAME  16

// <q CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_GENERAL> Support User Type 'General'
// <i> Defines whether User Type 'General' is supported or not
// <i> Default: 1
#define CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_GENERAL  1

// <q CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_PROGRAMMING> Support User Type 'Programming'
// <i> Defines whether User Type 'Programming' is supported or not
// <i> Default: 1
#define CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_PROGRAMMING  1

// <q CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_NON_ACCESS> Support User Type 'Non-Access'
// <i> Defines whether User Type 'Non-Access' is supported or not
// <i> Default: 1
#define CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_NON_ACCESS  1

// <q CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_DURESS> Support User Type 'Duress'
// <i> Defines whether User Type 'Duress' is supported or not
// <i> Default: 0
#define CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_DURESS  0

// <q CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_DISPOSABLE> Support User Type 'Disposable'
// <i> Defines whether User Type 'Disposable' is supported or not
// <i> Default: 0
#define CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_DISPOSABLE  0

// <q CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_EXPIRING> Support User Type 'Expiring'
// <i> Defines whether User Type 'Expiring' is supported or not
// <i> Default: 0
#define CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_EXPIRING  0

// <q CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_REMOTE_ONLY> Support User Type 'Remote Only'
// <i> Defines whether User Type 'Remote Only' is supported or not
// <i> Default: 0
#define CC_USER_CREDENTIAL_USER_TYPE_SUPPORTED_REMOTE_ONLY  0

// </h>
// <h> Credential Rules

// <q CC_USER_CREDENTIAL_CREDENTIAL_RULE_SUPPORTED_SINGLE> Support Credential Rule 'Single'
// <i> Default: 1
#define CC_USER_CREDENTIAL_CREDENTIAL_RULE_SUPPORTED_SINGLE  1

// <q CC_USER_CREDENTIAL_CREDENTIAL_RULE_SUPPORTED_DUAL> Support Credential Rule 'Dual'
// <i> Default: 0
#define CC_USER_CREDENTIAL_CREDENTIAL_RULE_SUPPORTED_DUAL  0

// <q CC_USER_CREDENTIAL_CREDENTIAL_RULE_SUPPORTED_TRIPLE> Support Credential Rule 'Triple'
// <i> Default: 0
#define CC_USER_CREDENTIAL_CREDENTIAL_RULE_SUPPORTED_TRIPLE  0

// </h>
// <h> Checksums

// <q CC_USER_CREDENTIAL_ALL_USERS_CHECKSUM_SUPPORTED> Support All Users Checksum
// <i> Default: 1
#define CC_USER_CREDENTIAL_ALL_USERS_CHECKSUM_SUPPORTED  1

// <q CC_USER_CREDENTIAL_USER_CHECKSUM_SUPPORTED> Support User Checksum
// <i> Default: 1
#define CC_USER_CREDENTIAL_USER_CHECKSUM_SUPPORTED  1

// <q CC_USER_CREDENTIAL_CREDENTIAL_CHECKSUM_SUPPORTED> Support Credential Checksum
// <i> Default: 1
#define CC_USER_CREDENTIAL_CREDENTIAL_CHECKSUM_SUPPORTED  1

// </h>
// <h> Admin Code

// <q CC_USER_CREDENTIAL_ADMIN_CODE_SUPPORTED> Support for Admin Code
// <i> Default: 1
#define CC_USER_CREDENTIAL_ADMIN_CODE_SUPPORTED  1

// <q CC_USER_CREDENTIAL_ADMIN_CODE_DEACTIVATE_SUPPORTED> Support for Admin Code Deactivation
// <i> Default: 1
#define CC_USER_CREDENTIAL_ADMIN_CODE_DEACTIVATE_SUPPORTED  1

// </h>
// <h> Credential Type: PIN Code

// <o CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_PIN_CODE> Number of Supported Credential Slots for PIN Code data <0..65535:1> <f.d>
// <i> Also defines whether this Credential Type is supported or not.
// <i> 0 Slots imply no support, any other number implies support.
// <i> Default: 5
#define CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_PIN_CODE  5

// <o CC_USER_CREDENTIAL_MAX_HASH_LENGTH_PIN_CODE> Maximum length of PIN Code hash <0..255:1> <f.d>
// <i> Default: 0
#define CC_USER_CREDENTIAL_MAX_HASH_LENGTH_PIN_CODE  0

// <o CC_USER_CREDENTIAL_MIN_DATA_LENGTH_PIN_CODE> Minimum length of PIN Code data <0..255:1> <f.d>
// <i> Default: 4
#define CC_USER_CREDENTIAL_MIN_DATA_LENGTH_PIN_CODE  4

// <o CC_USER_CREDENTIAL_MAX_DATA_LENGTH_PIN_CODE> Maximum length of PIN Code data <0..255:1> <f.d>
// <i> Default: 10
#define CC_USER_CREDENTIAL_MAX_DATA_LENGTH_PIN_CODE  10

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_PIN_CODE> Support Credential Learn functionality for Credential Type 'PIN Code'
// <i> Default: 1
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_PIN_CODE  1

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_PIN_CODE> Recommended Credential Learn timeout for Credential Type 'PIN Code'
// <i> Default: 20
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_PIN_CODE  20

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_PIN_CODE> Credential Learn Number of Steps for Credential Type 'PIN Code'
// <i> Default: 1
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_PIN_CODE  1

// <q USE_U3C_PIN_CODE_RECOMMENDATIONS> Use User Credential Command Class recommendations for PIN Code manufacturer validation
// <i> Controls whether to use the User Credential Command Class recommendations for PIN Code manufacturer validation,
// <i> as defined in the specification (reject same digit, ascending and descending sequences).
// <i> If the application supports the User Code Command Class, then enforcing these recommendations
// <i> can result in a failed certification as the User Code Command Class is less strict
// <i> than the User Credential Command Class.
// <i> Default: 0
#define USE_U3C_PIN_CODE_RECOMMENDATIONS  0

// </h>
// <h> Credential Type: Password

// <o CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_PASSWORD> Number of Supported Credential Slots for Password data <0..65535:1> <f.d>
// <i> Also defines whether this Credential Type is supported or not.
// <i> 0 Slots imply no support, any other number implies support.
// <i> Default: 5
#define CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_PASSWORD  5

// <o CC_USER_CREDENTIAL_MAX_HASH_LENGTH_PASSWORD> Maximum length of password hash <0..255:1> <f.d>
// <i> Default: 0
#define CC_USER_CREDENTIAL_MAX_HASH_LENGTH_PASSWORD  0

// <o CC_USER_CREDENTIAL_MIN_DATA_LENGTH_PASSWORD> Minimum length of Password data <0..255:2> <f.d>
// <i> Default: 2
#define CC_USER_CREDENTIAL_MIN_DATA_LENGTH_PASSWORD  2

// <o CC_USER_CREDENTIAL_MAX_DATA_LENGTH_PASSWORD> Maximum length of Password data <0..255:2> <f.d>
// <i> Default: 20
#define CC_USER_CREDENTIAL_MAX_DATA_LENGTH_PASSWORD  20

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_PASSWORD> Support Credential Learn functionality for Credential Type 'Password'
// <i> Default: 0
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_PASSWORD  0

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_PASSWORD> Recommended Credential Learn timeout for Credential Type 'Password'
// <i> Default: 20
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_PASSWORD  20

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_PASSWORD> Credential Learn Number of Steps for Credential Type 'Password'
// <i> Default: 1
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_PASSWORD  1

// </h>
// <h> Credential Type: RFID Code

// <o CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_RFID_CODE> Number of Supported Credential Slots for RFID Code data <0..65535:1> <f.d>
// <i> Also defines whether this Credential Type is supported or not.
// <i> 0 Slots imply no support, any other number implies support.
// <i> Default: 0
#define CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_RFID_CODE  0

// <o CC_USER_CREDENTIAL_MAX_HASH_LENGTH_RFID_CODE> Maximum length of RFID Code hash <0..255:1> <f.d>
// <i> Default: 0
#define CC_USER_CREDENTIAL_MAX_HASH_LENGTH_RFID_CODE  0

// <o CC_USER_CREDENTIAL_MIN_DATA_LENGTH_RFID_CODE> Minimum length of RFID Code data <0..255:1> <f.d>
// <i> Default: 1
#define CC_USER_CREDENTIAL_MIN_DATA_LENGTH_RFID_CODE  1

// <o CC_USER_CREDENTIAL_MAX_DATA_LENGTH_RFID_CODE> Maximum length of RFID Code data <0..255:1> <f.d>
// <i> Default: 32
#define CC_USER_CREDENTIAL_MAX_DATA_LENGTH_RFID_CODE  32

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_RFID_CODE> Support Credential Learn functionality for Credential Type 'RFID Code'
// <i> Default: 0
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_RFID_CODE  0

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_RFID_CODE> Recommended Credential Learn timeout for Credential Type 'RFID Code'
// <i> Default: 20
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_RFID_CODE  20

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_RFID_CODE> Credential Learn Number of Steps for Credential Type 'RFID Code'
// <i> Default: 1
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_RFID_CODE  1

// </h>
// <h> Credential Type: BLE

// <o CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_BLE> Number of Supported Credential Slots for BLE data <0..65535:1> <f.d>
// <i> Also defines whether this Credential Type is supported or not.
// <i> 0 Slots imply no support, any other number implies support.
// <i> Default: 0
#define CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_BLE  0

// <o CC_USER_CREDENTIAL_MAX_HASH_LENGTH_BLE> Maximum length of BLE data hash <0..255:1> <f.d>
// <i> Default: 0
#define CC_USER_CREDENTIAL_MAX_HASH_LENGTH_BLE  0

// <o CC_USER_CREDENTIAL_MIN_DATA_LENGTH_BLE> Minimum length of BLE data <0..255:1> <f.d>
// <i> Default: 1
#define CC_USER_CREDENTIAL_MIN_DATA_LENGTH_BLE  1

// <o CC_USER_CREDENTIAL_MAX_DATA_LENGTH_BLE> Maximum length of BLE data <0..255:1> <f.d>
// <i> Default: 32
#define CC_USER_CREDENTIAL_MAX_DATA_LENGTH_BLE  32

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_BLE> Support Credential Learn functionality for Credential Type 'BLE'
// <i> Default: 0
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_BLE  0

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_BLE> Recommended Credential Learn timeout for Credential Type 'BLE'
// <i> Default: 20
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_BLE  20

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_BLE> Credential Learn Number of Steps for Credential Type 'BLE'
// <i> Default: 1
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_BLE  1

// </h>
// <h> Credential Type: NFC

// <o CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_NFC> Number of Supported Credential Slots for NFC data <0..65535:1> <f.d>
// <i> Also defines whether this Credential Type is supported or not.
// <i> 0 Slots imply no support, any other number implies support.
// <i> Default: 0
#define CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_NFC  0

// <o CC_USER_CREDENTIAL_MAX_HASH_LENGTH_NFC> Maximum length of NFC data hash <0..255:1> <f.d>
// <i> Default: 0
#define CC_USER_CREDENTIAL_MAX_HASH_LENGTH_NFC  0

// <o CC_USER_CREDENTIAL_MIN_DATA_LENGTH_NFC> Minimum length of NFC data <0..255:1> <f.d>
// <i> Default: 1
#define CC_USER_CREDENTIAL_MIN_DATA_LENGTH_NFC  1

// <o CC_USER_CREDENTIAL_MAX_DATA_LENGTH_NFC> Maximum length of NFC data <0..255:1> <f.d>
// <i> Default: 32
#define CC_USER_CREDENTIAL_MAX_DATA_LENGTH_NFC  32

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_NFC> Support Credential Learn functionality for Credential Type 'NFC'
// <i> Default: 0
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_NFC  0

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_NFC> Recommended Credential Learn timeout for Credential Type 'NFC'
// <i> Default: 20
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_NFC  20

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_NFC> Credential Learn Number of Steps for Credential Type 'NFC'
// <i> Default: 1
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_NFC  1

// </h>
// <h> Credential Type: UWB

// <o CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_UWB> Number of Supported Credential Slots for UWB data <0..65535:1> <f.d>
// <i> Also defines whether this Credential Type is supported or not.
// <i> 0 Slots imply no support, any other number implies support.
// <i> Default: 0
#define CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_UWB  0

// <o CC_USER_CREDENTIAL_MAX_HASH_LENGTH_UWB> Maximum length of UWB data hash <0..255:1> <f.d>
// <i> Default: 4
#define CC_USER_CREDENTIAL_MAX_HASH_LENGTH_UWB  4

// <o CC_USER_CREDENTIAL_MIN_DATA_LENGTH_UWB> Minimum length of UWB data <0..255:1> <f.d>
// <i> Default: 1
#define CC_USER_CREDENTIAL_MIN_DATA_LENGTH_UWB  1

// <o CC_USER_CREDENTIAL_MAX_DATA_LENGTH_UWB> Maximum length of UWB data <0..255:1> <f.d>
// <i> Default: 32
#define CC_USER_CREDENTIAL_MAX_DATA_LENGTH_UWB  32

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_UWB> Support Credential Learn functionality for Credential Type 'UWB'
// <i> Default: 0
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_UWB  0

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_UWB> Recommended Credential Learn timeout for Credential Type 'UWB'
// <i> Default: 20
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_UWB  20

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_UWB> Credential Learn Number of Steps for Credential Type 'UWB'
// <i> Default: 1
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_UWB  1

// </h>
// <h> Credential Type: Eye Biometric

// <o CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_EYE_BIOMETRIC> Number of Supported Credential Slots for Eye Biometric data <0..65535:1> <f.d>
// <i> Also defines whether this Credential Type is supported or not.
// <i> 0 Slots imply no support, any other number implies support.
// <i> Default: 0
#define CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_EYE_BIOMETRIC  0

// <o CC_USER_CREDENTIAL_MAX_HASH_LENGTH_EYE_BIOMETRIC> Maximum length of Eye Biometric data hash <0..255:1> <f.d>
// <i> Default: 4
#define CC_USER_CREDENTIAL_MAX_HASH_LENGTH_EYE_BIOMETRIC  4

// <o CC_USER_CREDENTIAL_MIN_DATA_LENGTH_EYE_BIOMETRIC> Minimum length of Eye Biometric data <0..255:1> <f.d>
// <i> Default: 1
#define CC_USER_CREDENTIAL_MIN_DATA_LENGTH_EYE_BIOMETRIC  1

// <o CC_USER_CREDENTIAL_MAX_DATA_LENGTH_EYE_BIOMETRIC> Maximum length of Eye Biometric data <0..255:1> <f.d>
// <i> Default: 32
#define CC_USER_CREDENTIAL_MAX_DATA_LENGTH_EYE_BIOMETRIC  32

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_EYE_BIOMETRIC> Support Credential Learn functionality for Credential Type 'Eye Biometric'
// <i> Default: 0
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_EYE_BIOMETRIC  0

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_EYE_BIOMETRIC> Recommended Credential Learn timeout for Credential Type 'Eye Biometric'
// <i> Default: 20
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_EYE_BIOMETRIC  20

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_EYE_BIOMETRIC> Credential Learn Number of Steps for Credential Type 'Eye Biometric'
// <i> Default: 1
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_EYE_BIOMETRIC  1

// </h>
// <h> Credential Type: Face Biometric

// <o CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_FACE_BIOMETRIC> Number of Supported Credential Slots for Face Biometric data <0..65535:1> <f.d>
// <i> Also defines whether this Credential Type is supported or not.
// <i> 0 Slots imply no support, any other number implies support.
// <i> Default: 0
#define CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_FACE_BIOMETRIC  0

// <o CC_USER_CREDENTIAL_MAX_HASH_LENGTH_FACE_BIOMETRIC> Maximum length of Face Biometric data hash <0..255:1> <f.d>
// <i> Default: 4
#define CC_USER_CREDENTIAL_MAX_HASH_LENGTH_FACE_BIOMETRIC  4

// <o CC_USER_CREDENTIAL_MIN_DATA_LENGTH_FACE_BIOMETRIC> Minimum length of Face Biometric data <0..255:1> <f.d>
// <i> Default: 1
#define CC_USER_CREDENTIAL_MIN_DATA_LENGTH_FACE_BIOMETRIC  1

// <o CC_USER_CREDENTIAL_MAX_DATA_LENGTH_FACE_BIOMETRIC> Maximum length of Face Biometric data <0..255:1> <f.d>
// <i> Default: 32
#define CC_USER_CREDENTIAL_MAX_DATA_LENGTH_FACE_BIOMETRIC  32

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_FACE_BIOMETRIC> Support Credential Learn functionality for Credential Type 'Face Biometric'
// <i> Default: 0
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_FACE_BIOMETRIC  0

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_FACE_BIOMETRIC> Recommended Credential Learn timeout for Credential Type 'Face Biometric'
// <i> Default: 20
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_FACE_BIOMETRIC  20

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_FACE_BIOMETRIC> Credential Learn Number of Steps for Credential Type 'Face Biometric'
// <i> Default: 1
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_FACE_BIOMETRIC  1

// </h>
// <h> Credential Type: Finger Biometric

// <o CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_FINGER_BIOMETRIC> Number of Supported Credential Slots for Finger Biometric data <0..65535:1> <f.d>
// <i> Also defines whether this Credential Type is supported or not.
// <i> 0 Slots imply no support, any other number implies support.
// <i> Default: 0
#define CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_FINGER_BIOMETRIC  0

// <o CC_USER_CREDENTIAL_MAX_HASH_LENGTH_FINGER_BIOMETRIC> Maximum length of Finger Biometric data hash <0..255:1> <f.d>
// <i> Default: 4
#define CC_USER_CREDENTIAL_MAX_HASH_LENGTH_FINGER_BIOMETRIC  4

// <o CC_USER_CREDENTIAL_MIN_DATA_LENGTH_FINGER_BIOMETRIC> Minimum length of Finger Biometric data <0..255:1> <f.d>
// <i> Default: 1
#define CC_USER_CREDENTIAL_MIN_DATA_LENGTH_FINGER_BIOMETRIC  1

// <o CC_USER_CREDENTIAL_MAX_DATA_LENGTH_FINGER_BIOMETRIC> Maximum length of Finger Biometric data <0..255:1> <f.d>
// <i> Default: 32
#define CC_USER_CREDENTIAL_MAX_DATA_LENGTH_FINGER_BIOMETRIC  32

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_FINGER_BIOMETRIC> Support Credential Learn functionality for Credential Type 'Finger Biometric'
// <i> Default: 0
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_FINGER_BIOMETRIC  0

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_FINGER_BIOMETRIC> Recommended Credential Learn timeout for Credential Type 'Finger Biometric'
// <i> Default: 20
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_FINGER_BIOMETRIC  20

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_FINGER_BIOMETRIC> Credential Learn Number of Steps for Credential Type 'Finger Biometric'
// <i> Default: 1
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_FINGER_BIOMETRIC  1

// </h>
// <h> Credential Type: Hand Biometric

// <o CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_HAND_BIOMETRIC> Number of Supported Credential Slots for Hand Biometric data <0..65535:1> <f.d>
// <i> Also defines whether this Credential Type is supported or not.
// <i> 0 Slots imply no support, any other number implies support.
// <i> Default: 0
#define CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_HAND_BIOMETRIC  0

// <o CC_USER_CREDENTIAL_MAX_HASH_LENGTH_HAND_BIOMETRIC> Maximum length of Hand Biometric data hash <0..255:1> <f.d>
// <i> Default: 4
#define CC_USER_CREDENTIAL_MAX_HASH_LENGTH_HAND_BIOMETRIC  4

// <o CC_USER_CREDENTIAL_MIN_DATA_LENGTH_HAND_BIOMETRIC> Minimum length of Hand Biometric data <0..255:1> <f.d>
// <i> Default: 1
#define CC_USER_CREDENTIAL_MIN_DATA_LENGTH_HAND_BIOMETRIC  1

// <o CC_USER_CREDENTIAL_MAX_DATA_LENGTH_HAND_BIOMETRIC> Maximum length of Hand Biometric data <0..255:1> <f.d>
// <i> Default: 32
#define CC_USER_CREDENTIAL_MAX_DATA_LENGTH_HAND_BIOMETRIC  32

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_HAND_BIOMETRIC> Support Credential Learn functionality for Credential Type 'Hand Biometric'
// <i> Default: 0
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_HAND_BIOMETRIC  0

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_HAND_BIOMETRIC> Recommended Credential Learn timeout for Credential Type 'Hand Biometric'
// <i> Default: 20
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_HAND_BIOMETRIC  20

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_HAND_BIOMETRIC> Credential Learn Number of Steps for Credential Type 'Hand Biometric'
// <i> Default: 1
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_HAND_BIOMETRIC  1

// </h>
// <h> Credential Type: Unspecified Biometric

// <o CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_UNSPECIFIED_BIOMETRIC> Number of Supported Credential Slots for Unspecified Biometric data <0..65535:1> <f.d>
// <i> Also defines whether this Credential Type is supported or not.
// <i> 0 Slots imply no support, any other number implies support.
// <i> Default: 0
#define CC_USER_CREDENTIAL_MAX_CREDENTIAL_SLOTS_UNSPECIFIED_BIOMETRIC  0

// <o CC_USER_CREDENTIAL_MAX_HASH_LENGTH_UNSPECIFIED_BIOMETRIC> Maximum length of Unspecified Biometric data hash <0..255:1> <f.d>
// <i> Default: 4
#define CC_USER_CREDENTIAL_MAX_HASH_LENGTH_UNSPECIFIED_BIOMETRIC  4

// <o CC_USER_CREDENTIAL_MIN_DATA_LENGTH_UNSPECIFIED_BIOMETRIC> Minimum length of Unspecified Biometric data <0..255:1> <f.d>
// <i> Default: 1
#define CC_USER_CREDENTIAL_MIN_DATA_LENGTH_UNSPECIFIED_BIOMETRIC  1

// <o CC_USER_CREDENTIAL_MAX_DATA_LENGTH_UNSPECIFIED_BIOMETRIC> Maximum length of Unspecified Biometric data <0..255:1> <f.d>
// <i> Default: 32
#define CC_USER_CREDENTIAL_MAX_DATA_LENGTH_UNSPECIFIED_BIOMETRIC  32

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_UNSPECIFIED_BIOMETRIC> Support Credential Learn functionality for Credential Type 'Unspecified Biometric'
// <i> Default: 0
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORTED_UNSPECIFIED_BIOMETRIC  0

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_UNSPECIFIED_BIOMETRIC> Recommended Credential Learn timeout for Credential Type 'Unspecified Biometric'
// <i> Default: 20
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT_UNSPECIFIED_BIOMETRIC  20

// <q CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_UNSPECIFIED_BIOMETRIC> Credential Learn Number of Steps for Credential Type 'Unspecified Biometric'
// <i> Default: 1
#define CC_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS_UNSPECIFIED_BIOMETRIC  1

// </h>

// <<< end of configuration section >>>

#endif /* _CC_USER_CREDENTIAL_CONFIG_H_ */
