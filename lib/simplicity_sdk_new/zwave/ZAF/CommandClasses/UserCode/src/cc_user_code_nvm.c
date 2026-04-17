/**
 * @file
 * Command Class User Code I/O handling with NVM
 * @copyright 2022 Silicon Laboratories Inc.
 */
#include <string.h>

#include <cc_user_code_io.h>
#include <cc_user_code_config.h>

#include <ZAF_nvm_app.h>
#include <ZAF_nvm.h>
#include <ZAF_file_ids.h>
#include <zpal_nvm.h>
#include <zpal_misc.h>
#include <SizeOf.h>

// This version number must be increased if we make changes in the UserCodeNvmHeader_t structure.
// Everything before the introduction of this version number is considered version 0.
#define CC_USER_CODE_LATEST_FILE_VERSION  (0x01)
#define FLAG_USER_ID_INVALID              (false)
#define FLAG_USER_ID_VALID                (true)
#define VALID_FLAGS_NUM                   (32)

#define CC_USER_CODE_FILE_OFFSET(a) (ZAF_FILE_ID_CC_USER_CODE_BASE + a)

typedef struct {
  // Always keep version the first byte to allow detecting any changes.
  uint8_t version;
  uint8_t validFlags[VALID_FLAGS_NUM];
} UserCodeNvmHeader_t;

static UserCodeNvmHeader_t userCodeNvmHeader;

static bool GetValidFlag(uint8_t identifier)
{
  uint8_t byteIndex = identifier / 8;
  uint8_t bitIndex = identifier % 8;
  return (userCodeNvmHeader.validFlags[byteIndex] & (1 << bitIndex));
}

static void SetValidFlag(uint8_t identifier, bool value)
{
  uint8_t byteIndex = identifier / 8;
  uint8_t bitIndex = identifier % 8;
  if (value) {
    userCodeNvmHeader.validFlags[byteIndex] |= (1 << bitIndex);
  } else {
    userCodeNvmHeader.validFlags[byteIndex] &= ~(1 << bitIndex);
  }
}

static void
MigrateNvmV0ToV1(void)
{
  SUserCode userCodeData;
  memset(&userCodeNvmHeader, 0, sizeof(userCodeNvmHeader));
  userCodeNvmHeader.version = 1;

  for (uint8_t i = 0; i < CC_USER_CODE_MAX_IDS; i++) {
    if (ZAF_nvm_read(CC_USER_CODE_FILE_OFFSET(i), &userCodeData, sizeof(SUserCode)) != ZPAL_STATUS_OK) {
      continue;
    }
    if (userCodeData.user_id_status != USER_ID_AVAILABLE) {
      // User code is not empty and contains valid data
      SetValidFlag(i, FLAG_USER_ID_VALID);
    }
  }
  ZAF_nvm_write(ZAF_FILE_ID_CC_USER_CODE_HEADER, &userCodeNvmHeader, sizeof(userCodeNvmHeader));
}

bool
CC_UserCode_Write(uint8_t identifier, SUserCode *userCodeData)
{
  zpal_status_t status = ZPAL_STATUS_OK;

  // Make sure identifier is valid
  // Avoid type-limits warning.
  #if CC_USER_CODE_MAX_IDS < UINT8_MAX
  if (identifier == 0 || identifier > CC_USER_CODE_MAX_IDS) {
    return false;
  }
  #endif

  if (userCodeData->user_id_status == USER_ID_AVAILABLE && GetValidFlag(identifier - 1) == FLAG_USER_ID_VALID) {
    SetValidFlag(identifier - 1, FLAG_USER_ID_INVALID);
    status = ZAF_nvm_write(ZAF_FILE_ID_CC_USER_CODE_HEADER, &userCodeNvmHeader, sizeof(userCodeNvmHeader));
  } else if (userCodeData->user_id_status != USER_ID_AVAILABLE) {
    status = ZAF_nvm_write(CC_USER_CODE_FILE_OFFSET(identifier - 1), userCodeData, sizeof(SUserCode));
    if (status != ZPAL_STATUS_OK) {
      return false;
    }
    if (GetValidFlag(identifier - 1) == FLAG_USER_ID_INVALID) {
      SetValidFlag(identifier - 1, FLAG_USER_ID_VALID);
      status = ZAF_nvm_write(ZAF_FILE_ID_CC_USER_CODE_HEADER, &userCodeNvmHeader, sizeof(userCodeNvmHeader));
    }
  }

  return status == ZPAL_STATUS_OK;
}

bool
CC_UserCode_Read(uint8_t identifier, SUserCode *userCodeData)
{
  zpal_status_t status;

  // Make sure identifier is valid
  // Avoid type-limits warning.
  #if CC_USER_CODE_MAX_IDS < UINT8_MAX
  if (identifier == 0 || identifier > CC_USER_CODE_MAX_IDS) {
    return false;
  }
  #endif

  if (GetValidFlag(identifier - 1) == FLAG_USER_ID_INVALID) {
    userCodeData->user_id_status = USER_ID_AVAILABLE;
    // CC:0063.01.01.11.009
    userCodeData->userCodeLen = 4;
    memset(userCodeData->userCode, 0x00, sizeof_array(userCodeData->userCode));
    status = ZPAL_STATUS_OK;
  } else {
    status = ZAF_nvm_read(CC_USER_CODE_FILE_OFFSET(identifier - 1), userCodeData, sizeof(SUserCode));
  }

  return status == ZPAL_STATUS_OK;
}

bool
CC_UserCode_EraseAllUserCodes(void)
{
  zpal_status_t status;

  memset(&userCodeNvmHeader, 0, sizeof(userCodeNvmHeader));
  userCodeNvmHeader.version = CC_USER_CODE_LATEST_FILE_VERSION;
  status = ZAF_nvm_write(ZAF_FILE_ID_CC_USER_CODE_HEADER, &userCodeNvmHeader, sizeof(userCodeNvmHeader));

  return status == ZPAL_STATUS_OK;
}

void
CC_UserCode_Migrate(void)
{
  zpal_status_t status;
  #ifdef ZW_MIGRATION_TO_7_19
  SUserCode userCodeData;

  // Migration needed for all apps before 7.19.0
  for (uint8_t i = 0; i < CC_USER_CODE_MAX_IDS; i++) {
    status = ZAF_nvm_read_object_part(ZAF_FILE_ID_USERCODE, &userCodeData, i * sizeof(userCodeData), sizeof(userCodeData));
    // If the file is not present, we don't have anything to migrate
    if (status != ZPAL_STATUS_OK) {
      break;
    }
    ZAF_nvm_write(CC_USER_CODE_FILE_OFFSET(i), &userCodeData, sizeof(userCodeData));
  }

  if (status == ZPAL_STATUS_OK) {
    ZAF_nvm_erase_object(ZAF_FILE_ID_USERCODE);
  }

  #endif // 7.19 migration code block

  status = ZAF_nvm_read(ZAF_FILE_ID_CC_USER_CODE_HEADER, &userCodeNvmHeader, sizeof(userCodeNvmHeader));
  if (status != ZPAL_STATUS_OK) {
    MigrateNvmV0ToV1();
  }
}
