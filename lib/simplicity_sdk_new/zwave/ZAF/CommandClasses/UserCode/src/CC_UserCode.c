/**
 * @file
 * @brief Handler for Command Class User Code.
 * @copyright 2018 Silicon Laboratories Inc.
 */

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
#include <assert.h>
#include <ZW_TransportLayer.h>
#include <CC_UserCode.h>
#include <cc_user_code_config.h>
#include <cc_user_code_io.h>
#include <string.h>
#include "zaf_event_distributor_soc.h"
#include "zpal_log.h"
#include "ZW_typedefs.h"
#include "ZAF_Common_interface.h"
#include "ZAF_TSE.h"
#include "zaf_transport_tx.h"

/****************************************************************************/
/*                      PRIVATE TYPES and DEFINITIONS                       */
/****************************************************************************/

#define CC_USER_CODE_MAX_IDS_MAX (255)

typedef struct {
  uint8_t userIdentifier;
  uint8_t userIdStatus;
  uint8_t userCode[USERCODE_MAX_LEN];
}
user_code_report_t;

/****************************************************************************/
/*                              PRIVATE DATA                                */
/****************************************************************************/

static s_CC_userCode_data_t userCodeData = { 0 };

/****************************************************************************/
/*                            PRIVATE FUNCTIONS                             */
/****************************************************************************/

_Static_assert(CC_USER_CODE_MAX_IDS <= CC_USER_CODE_MAX_IDS_MAX,
               "STATIC_ASSERT_FAILED_User_Code_Max_IDs_too_high");

static void CC_UserCode_report_stx(zaf_tx_options_t *tx_options, void* pData);

static received_frame_status_t
CC_UserCode_handler(
  RECEIVE_OPTIONS_TYPE_EX *rxOpt,
  ZW_APPLICATION_TX_BUFFER *pCmd,
  uint8_t cmdLength,
  ZW_APPLICATION_TX_BUFFER *pFrameOut,
  uint8_t * pFrameOutLength)
{
  switch (pCmd->ZW_Common.cmd) {
    case USER_CODE_GET:
      if (true == Check_not_legal_response_job(rxOpt)) {
        /*Get/Report do not support endpoint bit-addressing */
        return RECEIVED_FRAME_STATUS_FAIL;
      }

      size_t len = 0;
      uint8_t maxNumberOfUsers = CC_UserCode_UsersNumberReport_handler(rxOpt->destNode.endpoint);

      pFrameOut->ZW_UserCodeReport1byteFrame.cmdClass = COMMAND_CLASS_USER_CODE;
      pFrameOut->ZW_UserCodeReport1byteFrame.cmd = USER_CODE_REPORT;
      pFrameOut->ZW_UserCodeReport1byteFrame.userIdentifier = pCmd->ZW_UserCodeGetFrame.userIdentifier;

      if ((0 == pCmd->ZW_UserCodeGetFrame.userIdentifier)
          || (pCmd->ZW_UserCodeGetFrame.userIdentifier > maxNumberOfUsers)) {
        pFrameOut->ZW_UserCodeReport1byteFrame.userIdStatus = USER_ID_NO_STATUS;
      } else {
        if (false == CC_UserCode_getId_handler(
              pCmd->ZW_UserCodeGetFrame.userIdentifier,
              (user_id_status_t*)&(pFrameOut->ZW_UserCodeReport1byteFrame.userIdStatus),
              rxOpt->destNode.endpoint)) {
          return RECEIVED_FRAME_STATUS_FAIL;
        }

        if (false == CC_UserCode_Report_handler(
              pCmd->ZW_UserCodeGetFrame.userIdentifier,
              &(pFrameOut->ZW_UserCodeReport1byteFrame.userCode1),
              &len,
              rxOpt->destNode.endpoint)) {
          /*Job failed */
          return RECEIVED_FRAME_STATUS_FAIL; /*failing*/
        }
      }

      *pFrameOutLength = sizeof(ZW_USER_CODE_REPORT_1BYTE_FRAME) + len - 1;

      return RECEIVED_FRAME_STATUS_SUCCESS;

    case USER_CODE_SET:
    {
      bool status = true;
      uint16_t i;
      uint8_t user_code_length = cmdLength - 4;

      if (!((pCmd->ZW_UserCodeSet1byteFrame.userIdentifier <= CC_UserCode_UsersNumberReport_handler(rxOpt->destNode.endpoint))
            && ((USERCODE_MIN_LEN <= user_code_length) && (USERCODE_MAX_LEN >= user_code_length)))
          ) {
        return RECEIVED_FRAME_STATUS_FAIL;
      }

      switch (pCmd->ZW_UserCodeSet1byteFrame.userIdStatus) {
        case USER_ID_AVAILABLE:
          for (i = 0; i < 4; i++) {
            *(&pCmd->ZW_UserCodeSet1byteFrame.userCode1 + i) = 0x00;
          }
          user_code_length = 4;
          break;

        case USER_ID_OCCUPIED:
        case USER_ID_RESERVED:
          /**
           * CC:0063.01.01.12.001: The only allowed bulk operation is setting
           * all User ID statuses to 0x00.
           * CC:0083.01.0A.11.018: If the device also supports the
           * User Credential Command Class, setting multiple user codes
           * (mapped as PIN code credentials) to the same value is not allowed.
           */
          if (pCmd->ZW_UserCodeSet1byteFrame.userIdentifier == 0) {
            status = false;
            break;
          }

          // Validate user code are digits
          for (i = 0; i < user_code_length; i++) {
            if ( ((0x30 > (uint8_t)*(&pCmd->ZW_UserCodeSet1byteFrame.userCode1 + i)) || (0x39 < (uint8_t)*(&pCmd->ZW_UserCodeSet1byteFrame.userCode1 + i)))) {
              status = false;
              break;
            }
          }
          break;

        default:
          return RECEIVED_FRAME_STATUS_FAIL;
      }

      if (true == status) {
        e_cmd_handler_return_code_t return_code;
        return_code = CC_UserCode_Set_handler(pCmd->ZW_UserCodeSet1byteFrame.userIdentifier,
                                              pCmd->ZW_UserCodeSet1byteFrame.userIdStatus,
                                              &(pCmd->ZW_UserCodeSet1byteFrame.userCode1),
                                              user_code_length,
                                              rxOpt->sourceNode.nodeId);

        if (E_CMD_HANDLER_RETURN_CODE_FAIL == return_code || E_CMD_HANDLER_RETURN_CODE_HANDLED == return_code) {
          // Build up new CC data structure
          memset(&userCodeData, 0, sizeof(s_CC_userCode_data_t));
          userCodeData.rxOptions = *rxOpt;
          userCodeData.userIdentifier = pCmd->ZW_UserCodeGetFrame.userIdentifier;
          /* We cannot know if the same User Identifier was modified several times
              or not, so do not overwrite_previous_trigger. Also we want to know if a
              User Code was updated, even for a very short time */
          if (false == ZAF_TSE_Trigger(CC_UserCode_report_stx,
                                       (void*)&userCodeData,
                                       false)) {
            ZPAL_LOG_ERROR(ZPAL_LOG_CC_USER_CODE, "%s(): ZAF_TSE_Trigger failed\n", __func__);
          }
        }

        if (E_CMD_HANDLER_RETURN_CODE_HANDLED == return_code) {
          return RECEIVED_FRAME_STATUS_SUCCESS;
        }
      }
      return RECEIVED_FRAME_STATUS_FAIL;
    }
    break;

    case USERS_NUMBER_GET:
      if (false == Check_not_legal_response_job(rxOpt)) {
        pFrameOut->ZW_UsersNumberReportFrame.cmdClass = COMMAND_CLASS_USER_CODE;
        pFrameOut->ZW_UsersNumberReportFrame.cmd = USERS_NUMBER_REPORT;
        pFrameOut->ZW_UsersNumberReportFrame.supportedUsers =
          CC_UserCode_UsersNumberReport_handler(rxOpt->destNode.endpoint);

        *pFrameOutLength = sizeof(ZW_USERS_NUMBER_REPORT_FRAME);

        return RECEIVED_FRAME_STATUS_SUCCESS;
      }
      return RECEIVED_FRAME_STATUS_FAIL;
      break;
  }
  return RECEIVED_FRAME_STATUS_NO_SUPPORT;
}

JOB_STATUS CC_UserCode_SupportReport(
  AGI_PROFILE* pProfile,
  uint8_t sourceEndpoint,
  uint8_t userIdentifier,
  uint8_t userIdStatus,
  uint8_t* pUserCode,
  uint8_t userCodeLen,
  VOID_CALLBACKFUNC(pCallback)(TRANSMISSION_RESULT * pTransmissionResult))
{
  CMD_CLASS_GRP cmdGrp = {
    .cmdClass = COMMAND_CLASS_USER_CODE,
    .cmd = USER_CODE_REPORT
  };

  if ((0 == userIdentifier) || IS_NULL(pUserCode)
      || (userCodeLen > USERCODE_MAX_LEN) || (userCodeLen < USERCODE_MIN_LEN)
      || (2 < userIdStatus)) {
    return JOB_STATUS_BUSY;
  }

  user_code_report_t user_code_report = {
    .userIdentifier = userIdentifier,
    .userIdStatus = userIdStatus
  };

  memcpy(user_code_report.userCode, pUserCode, userCodeLen);

  return cc_engine_multicast_request(
    pProfile,
    sourceEndpoint,
    &cmdGrp,
    (uint8_t*)&user_code_report,
    sizeof(user_code_report_t) + userCodeLen - USERCODE_MAX_LEN,
    false,
    pCallback);
}

static void
CC_UserCode_report_stx(zaf_tx_options_t *tx_options, void* pData)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_CC_USER_CODE, "* %s() *\n"
                                        "\ttxOpt.src = %d\n"
                                        "\ttxOpt.options %#02x\n",
                 __func__, tx_options->source_endpoint, tx_options->tx_options);

  /* Prepare payload for report */
  size_t len;
  s_CC_userCode_data_t *pUserCodeData = (s_CC_userCode_data_t*)pData;

  ZW_APPLICATION_TX_BUFFER txBuf = {
    .ZW_UserCodeReport1byteFrame.cmdClass = COMMAND_CLASS_USER_CODE,
    .ZW_UserCodeReport1byteFrame.cmd = USER_CODE_REPORT,
    .ZW_UserCodeReport1byteFrame.userIdentifier = pUserCodeData->userIdentifier
  };

  if (false == CC_UserCode_getId_handler(
        pUserCodeData->userIdentifier,
        (user_id_status_t*)&(txBuf.ZW_UserCodeReport1byteFrame.userIdStatus),
        pUserCodeData->rxOptions.destNode.endpoint)) {
    ZPAL_LOG_ERROR(ZPAL_LOG_CC_USER_CODE, "%s(): CC_UserCode_getId_handler() failed. \n", __func__);
    return;
  }

  if (false == CC_UserCode_Report_handler(
        pUserCodeData->userIdentifier,
        &(txBuf.ZW_UserCodeReport1byteFrame.userCode1),
        &len,
        pUserCodeData->rxOptions.destNode.endpoint)) {
    /*Job failed */
    ZPAL_LOG_ERROR(ZPAL_LOG_CC_USER_CODE, "%s(): CC_UserCode_Report_handler() failed. \n", __func__);
    return;
  }
  tx_options->use_supervision = true;

  (void) zaf_transport_tx((uint8_t *)&txBuf,
                          sizeof(ZW_USER_CODE_REPORT_1BYTE_FRAME) + len - 1,
                          ZAF_TSE_TXCallback, tx_options);
}

ZW_WEAK e_cmd_handler_return_code_t
CC_UserCode_Set_handler(
  uint8_t identifier,
  USER_ID_STATUS id,
  uint8_t* pUserCode,
  uint8_t len,
  __attribute__((unused)) uint16_t endpoint)
{
  uint8_t i;
  __attribute__((unused)) bool status;
  SUserCode userCode = { 0 };

  // Make sure identifier is valid.
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

/**
 * @brief Set the user code to a new value.
 * @param[in] new_user_code The new user code.
 * @return void
 */
ZW_WEAK void CC_UserCode_set_usercode(char* new_user_code)
{
  if (strnlen(new_user_code, USERCODE_MAX_LEN + 1) > USERCODE_MAX_LEN) {
    ZPAL_LOG_DEBUG(ZPAL_LOG_CC_USER_CODE, "User code too long. Max length is %d\n", USERCODE_MAX_LEN);
    assert(false);
  }
  SUserCode newUserCode;
  newUserCode.user_id_status = USER_ID_OCCUPIED;
  newUserCode.userCodeLen = strnlen(new_user_code, USERCODE_MAX_LEN);
  memcpy(newUserCode.userCode, new_user_code, newUserCode.userCodeLen);
  CC_UserCode_Write(1, &newUserCode);
}

ZW_WEAK uint8_t
CC_UserCode_UsersNumberReport_handler(__attribute__((unused)) uint8_t endpoint)
{
  return CC_USER_CODE_MAX_IDS;
}

ZW_WEAK void
CC_UserCode_Migrate(void)
{
}

ZW_WEAK bool
CC_UserCode_Write(__attribute__((unused)) uint8_t identifier, __attribute__((unused)) SUserCode *userCodeData)
{
  return true;
}

ZW_WEAK bool
CC_UserCode_Read(__attribute__((unused)) uint8_t identifier, __attribute__((unused)) SUserCode *userCodeData)
{
  return true;
}

static uint8_t lifeline_reporting(ccc_pair_t * p_ccc_pair)
{
  p_ccc_pair->cmdClass = COMMAND_CLASS_USER_CODE;
  p_ccc_pair->cmd      = USER_CODE_REPORT;
  return 1;
}

static void
init(void)
{
  CC_UserCode_Migrate();
}

static void
reset(void)
{
  CC_UserCode_reset_data();
}

static void
migrate(void)
{
  CC_UserCode_Migrate();
}

REGISTER_CC_V6(CC_API_HANDLER_V2, COMMAND_CLASS_USER_CODE, USER_CODE_VERSION, CC_UserCode_handler, NULL, NULL, lifeline_reporting, 0, init, reset, migrate);

static void
user_code_event_handler(const uint8_t event, const void *data)
{
  cc_user_code_event_validate_data_t* validate_data;

  switch (event) {
    case CC_USER_CODE_EVENT_VALIDATE:
      if (ZAF_GetInclusionState() == EINCLUSIONSTATE_EXCLUDED) {
        ZPAL_LOG_WARNING(ZPAL_LOG_APP, "User Code validation request ignored: device not included\n");
        break;
      }
      validate_data = (cc_user_code_event_validate_data_t*) data;
      if (CC_UserCode_Validate(validate_data->id, validate_data->data, validate_data->length)) {
        zaf_event_distributor_enqueue_cc_event(COMMAND_CLASS_USER_CODE, CC_USER_CODE_EVENT_VALIDATE_VALID, NULL);
      } else {
        zaf_event_distributor_enqueue_cc_event(COMMAND_CLASS_USER_CODE, CC_USER_CODE_EVENT_VALIDATE_INVALID, NULL);
      }
      break;
  }
}

ZAF_EVENT_DISTRIBUTOR_REGISTER_CC_EVENT_HANDLER(COMMAND_CLASS_USER_CODE, user_code_event_handler);
