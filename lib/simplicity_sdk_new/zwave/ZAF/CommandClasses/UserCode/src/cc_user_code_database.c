/**
 * @file
 * @brief Database functions for Command Class User Code.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2024 Silicon Laboratories Inc.
 */

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/

#include "CC_UserCode.h"
#include "cc_user_code_io.h"
#include "cc_user_code_config.h"
#include "zpal_log.h"
#include "assert.h"
#include <string.h>

/****************************************************************************/
/*                            PRIVATE FUNCTIONS                             */
/****************************************************************************/

ZW_WEAK uint8_t
CC_UserCode_UsersNumberReport_handler(__attribute__((unused)) uint8_t endpoint)
{
  return CC_USER_CODE_MAX_IDS;
}

ZW_WEAK bool
CC_UserCode_getId_handler(
  uint8_t identifier,
  USER_ID_STATUS* pId,
  __attribute__((unused)) uint8_t endpoint)
{
  __attribute__((unused)) bool status;
  SUserCode userCode = { 0 };

  if (identifier == 0) {
    *pId = USER_ID_AVAILABLE;
    return true;
  }

  status = CC_UserCode_Read(identifier, &userCode);
  assert(status);

  *pId = userCode.user_id_status;
  return true;
}

ZW_WEAK bool
CC_UserCode_Report_handler(
  uint8_t identifier,
  uint8_t* pUserCode,
  size_t * pLen,
  __attribute__((unused)) uint8_t endpoint)
{
  __attribute__((unused)) bool status;
  SUserCode userCode = { 0 };

  if (identifier == 0) {
    *pLen = 4;
    memset(pUserCode, 0x00, *pLen);
    return true;
  }

  status = CC_UserCode_Read(identifier, &userCode);
  assert(status);

  *pLen = userCode.userCodeLen;
  if (USERCODE_MAX_LEN >= *pLen) {
    memcpy(pUserCode, userCode.userCode, *pLen);

    ZPAL_LOG_DEBUG(ZPAL_LOG_CC_USER_CODE, "hCmdUC_Report = ");
    for (size_t i = 0; i < *pLen; i++) {
      ZPAL_LOG_DEBUG(ZPAL_LOG_CC_USER_CODE, "%d", *(pUserCode + i));
    }
    ZPAL_LOG_DEBUG(ZPAL_LOG_CC_USER_CODE, "\r\n");
    return true;
  }
  return false;
}

ZW_WEAK e_cmd_handler_return_code_t
CC_UserCode_Set_handler(
  uint8_t identifier,
  USER_ID_STATUS id,
  uint8_t* pUserCode,
  uint8_t len,
  __attribute__((unused)) uint16_t modifierNodeId)
{
  uint8_t i;
  __attribute__((unused)) bool status;
  SUserCode userCode = { 0 };

  // Make sure identifier is valid
  // Avoid type-limits warning.
  #if CC_USER_CODE_MAX_IDS < UINT8_MAX
  if (identifier > CC_USER_CODE_MAX_IDS) {
    return E_CMD_HANDLER_RETURN_CODE_HANDLED;
  }
  #endif

  // it is possible to remove all user codes at once when identifier == 0
  if (identifier == 0) {
    if (id == USER_ID_AVAILABLE) {
      status = CC_UserCode_EraseAllUserCodes();
      assert(status);
    }
  } else {
    userCode.user_id_status = id;
    memcpy(userCode.userCode, pUserCode, len);
    userCode.userCodeLen = len;

    status = CC_UserCode_Write(identifier, &userCode);
    assert(status);
  }

  for (i = 0; i < len; i++) {
    ZPAL_LOG_DEBUG(ZPAL_LOG_CC_USER_CODE, "%d", *(pUserCode + i));
  }
  ZPAL_LOG_DEBUG(ZPAL_LOG_CC_USER_CODE, "\r\n");
  return E_CMD_HANDLER_RETURN_CODE_HANDLED;
}

ZW_WEAK void
CC_UserCode_reset_data(void)
{
  uint8_t defaultUserCode[] = CC_USER_CODE_DEFAULT;
  __attribute__((unused)) bool status;

  SUserCode userCodeDefaultData = {
    .user_id_status = USER_ID_OCCUPIED,
    .userCodeLen = sizeof(defaultUserCode)
  };
  memcpy(userCodeDefaultData.userCode, defaultUserCode, userCodeDefaultData.userCodeLen);

  if (CC_USER_CODE_MAX_IDS > 1) {
    status = CC_UserCode_EraseAllUserCodes();
    assert(status);
  }

  status = CC_UserCode_Write(1, &userCodeDefaultData);
  assert(status);
}

ZW_WEAK bool
CC_UserCode_Validate(uint8_t identifier, const uint8_t *pCode, uint8_t len)
{
  __attribute__((unused)) bool status;
  SUserCode userCode = { 0 };

  status = CC_UserCode_Read(identifier, &userCode);
  assert(status);

  if ((len == userCode.userCodeLen)
      && ((USER_ID_OCCUPIED == userCode.user_id_status)
          || (USER_ID_RESERVED == userCode.user_id_status))) {
    if (0 == memcmp(pCode, userCode.userCode, len)) {
      return true;
    }
  }
  return false;
}
