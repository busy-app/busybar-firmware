/**
 * @file
 * Contains a list of file IDs used by the ZAF.
 *
 * Range: 0x00000 - 0x01FFF.
 *
 * @attention The file IDs MUST NOT be changed.
 *
 * @copyright 2019 Silicon Laboratories Inc.
 */

#ifndef ZAF_APPLICATIONUTILITIES_ZAF_FILE_IDS_H_
#define ZAF_APPLICATIONUTILITIES_ZAF_FILE_IDS_H_

/**
 * @addtogroup ZAF
 * @{
 * @addtogroup FILE_ID
 *
 * ZAF File IDs.
 *
 * @attention MUST NOT be changed.
 * @{
 */

#define ZAF_FILE_ID_APP_VERSION                 (0)
#define ZAF_FILE_ID_ASSOCIATIONINFO             (1)
#define ZAF_FILE_ID_USERCODE                    (2)  // Deprecated. Used for migration only
#define ZAF_FILE_ID_BATTERYDATA                 (3)
#define ZAF_FILE_ID_NOTIFICATIONDATA            (4)
#define ZAF_FILE_ID_WAKEUPCCDATA                (5)
#define ZAF_FILE_ID_CC_FIRMWARE_UPDATE          (6)
#define ZAF_FILE_ID_CC_CONFIGURATION            (7)  //Obsolete ID nr. (Do not remove since legacy files may exist.)
#define ZAF_FILE_ID_KEY_FOB_NODES               (8)
#define ZAF_FILE_ID_CC_DOOR_LOCK                (9)
#define ZAF_FILE_ID_BASIC_SET                   (10)
#define ZAF_FILE_ID_CENTRAL_SCENE_CONFIG        (11)
#define ZAF_FILE_ID_APP_NAME                    (12)
#define ZAF_FILE_ID_CC_USER_CODE_HEADER         (13)
//Add file IDs for single files here.

//Base file ID number for CC_Configuration. 100 file ID numbers are allocated for future use.
#define ZAF_FILE_ID_CC_CONFIGURATION_BASE (100)
#define ZAF_FILE_ID_CC_CONFIGURATION_LAST (199)

// Base file ID number for CC_UserCode. 255 file ID numbers are allocated for future use.
#define ZAF_FILE_ID_CC_USER_CODE_BASE (200)
#define ZAF_FILE_ID_CC_USER_CODE_LAST (455)

// Base file ID number for CC_ColorSwitch. 255 file ID numbers are allocated for future use.
#define ZAF_FILE_ID_CC_COLOR_SWITCH_BASE (456)
#define ZAF_FILE_ID_CC_COLOR_SWITCH_LAST (711)

// Base file ID number for CC_MutilevelSwitch. 255 file ID numbers are allocated for future use.
#define ZAF_FILE_ID_CC_MULTILEVEL_SWITCH_BASE (712)
#define ZAF_FILE_ID_CC_MULTILEVEL_SWITCH_LAST (967)

/**
 * Data object and database descriptor table file ID numbers for CC_UserCredential.
 * 255 file ID numbers are allocated for users and 255 for credentials.
 */
#define ZAF_FILE_ID_CC_USER_CREDENTIAL_NUMBER_OF_USERS (968)
#define ZAF_FILE_ID_CC_USER_CREDENTIAL_NUMBER_OF_CREDENTIALS (969)
#define ZAF_FILE_ID_CC_USER_CREDENTIAL_USER_DESCRIPTOR_TABLE (970)
#define ZAF_FILE_ID_CC_USER_CREDENTIAL_USER_BASE (971)
#define ZAF_FILE_ID_CC_USER_CREDENTIAL_USER_LAST (1226)
#define ZAF_FILE_ID_CC_USER_CREDENTIAL_USER_NAME_BASE (1227)
#define ZAF_FILE_ID_CC_USER_CREDENTIAL_USER_NAME_LAST (1482)
#define ZAF_FILE_ID_CC_USER_CREDENTIAL_CREDENTIAL_DESCRIPTOR_TABLE (1483)
#define ZAF_FILE_ID_CC_USER_CREDENTIAL_CREDENTIAL_BASE (1484)
#define ZAF_FILE_ID_CC_USER_CREDENTIAL_CREDENTIAL_LAST (1739)
#define ZAF_FILE_ID_CC_USER_CREDENTIAL_CREDENTIAL_DATA_BASE (1740)
#define ZAF_FILE_ID_CC_USER_CREDENTIAL_CREDENTIAL_DATA_LAST (1995)

/**
 * Admin PIN Code data
 */
#define ZAF_FILE_ID_ADMIN_PIN_CODE (1996)

#define ZAF_FILE_SIZE_APP_VERSION  (sizeof(uint32_t))

/**
 * @} // FILE_ID
 * @} // ZAF
 */

#endif /* ZAF_APPLICATIONUTILITIES_ZAF_FILE_IDS_H_ */
