/**
 * @file CC_WakeUp.c
 * @brief Handler for Command Class Wake Up.
 * @copyright 2018 Silicon Laboratories Inc.
 *
 * @section wakeup_power_management Wake Up Command Class Power Management
 *
 * The Wake Up Command Class manages device power state during post-inclusion
 * operations to ensure the device remains awake for initial configuration:
 *
 * @verbatim
 * Wake Up Post-Inclusion Radio Power Management Timeline:
 *
 * Time:     0sec     2sec     4sec     6sec     8sec    10sec    12sec    14sec    16sec    18sec    20sec
 * State:    |--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|
 *           ^                                                                                   ^
 *           |                                                                                   |
 *   Post-Inclusion Start                                                              Stay-awake timeout
 *   Update stay-awake timeout                                                      (automatic sleep)
 *   zpal_radio_update_stay_awake(POST_INCLUSION_STAY_AWAKE_TIME)
 *           |                                                                                   |
 *           |<------------- Post-Inclusion Configuration Window ----------->|                   |
 *           |                                                               |                   |
 *        Start Post                                                     Config                Allow sleep
 *        Inclusion                                                      Complete              after timeout
 *
 * @endverbatim
 *
 * - **Post-Inclusion Stay-Awake**: After successful inclusion, we update the stay-awake
 *   timeout to POST_INCLUSION_STAY_AWAKE_TIME using zpal_radio_update_stay_awake().
 *   This ensures the device remains awake for initial configuration commands from
 *   the controller.
 *
 * - **Configuration Window**: The stay-awake period provides a window for the controller
 *   to send initial configuration commands such as associations, parameters, and
 *   other setup commands.
 *
 * - **Automatic Timeout**: The stay-awake request automatically expires after the
 *   configured timeout, allowing the device to return to its normal sleep/wake cycle
 *   without requiring explicit revocation.
 */

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/

#include <CC_WakeUp.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <CC_Common.h>
#include <misc.h>
#include <ZW_TransportSecProtocol.h>
#include <ZW_transport_api.h>
#include <AppTimer.h>
#include <ZAF_Common_interface.h>
#include <ZAF_file_ids.h>
#include <ZAF_nvm_app.h>
#include <ZAF_nvm.h>
#include <ZW_system_startup_api.h>
#include "zpal_log.h"
#include "zpal_radio.h"

/****************************************************************************/
/*                      PRIVATE TYPES and DEFINITIONS                       */
/****************************************************************************/

#define POST_INCLUSION_STAY_AWAKE_TIME    10000  // [ms]

/****************************************************************************/
/*                              PRIVATE DATA                                */
/****************************************************************************/

static bool wakeUpIsActive              = false;
static bool autoStayAwakeAfterInclusion = false;
static bool firstNotificationSent       = false;
static void (*ZCB_WakeUpTxCallback)(uint8_t txStatus, TX_STATUS_TYPE* pExtendedTxStatus);

static SWakeupCcData gWakeupCcData;

static SSwTimer WakeUpTimer;

static zpal_radio_stay_awake_id_t stay_awake_id = 0;

/****************************************************************************/
/*                              EXPORTED DATA                               */
/****************************************************************************/

// Nothing here.

/****************************************************************************/
/*                            PRIVATE FUNCTIONS                             */
/****************************************************************************/

/**
 * Transmits a Wake Up Notification command.
 * @param pCallback Pointer to callback function to be called upon transmission.
 * @return Status of the job.
 */
static bool CmdClassWakeupNotification(void (*pCallback)(uint8_t txStatus, TX_STATUS_TYPE* pExtendedTxStatus));

static void CC_WakeUp_stayAwakeIfActive(void);

static void TimerCallback(__attribute__((unused)) SSwTimer *pTimer)
{
  /* It is assumed that an Deep Sleep persistent timer is used with this call back
   * (see AppTimerDeepSleepPersistentRegister()). Otherwise this function will not
   * be called if the timer expires while the device is sleeping in deep sleep
   * hibernate.
   */

  ZPAL_LOG_DEBUG(ZPAL_LOG_CC_WAKE_UP, "CC_WakeUp TimerCallback() pTimer->Id=%d\n", pTimer->Id);

  CC_WakeUp_notification_tx(NULL);
}

static void LoadData(void)
{
  ZAF_nvm_read(ZAF_FILE_ID_WAKEUPCCDATA, &gWakeupCcData, sizeof(SWakeupCcData));
}

static void
send_first_notification(void)
{
  zpal_reset_reason_t resetReason = GetResetReason();

  if (!firstNotificationSent
      && ((ZPAL_RESET_REASON_POWER_ON == resetReason)
          || (ZPAL_RESET_REASON_BROWNOUT == resetReason)
          || (ZPAL_RESET_REASON_PIN == resetReason)
          || (ZPAL_RESET_REASON_SOFTWARE == resetReason)
          || (ZPAL_RESET_REASON_WATCHDOG == resetReason))) {
    CC_WakeUp_notification_tx(NULL);
  }

  firstNotificationSent = true;
}

/**
 * Resets the saved node ID in NVM.
 */
static void
CC_WakeUp_notificationMemorySetDefault(void)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_CC_WAKE_UP, "\r\nCCWdef");

  wakeUpIsActive = false;

  if (WakeUpTimer.pLiaison) { // Has the timer been initialized?
    // Ensure the wakeup timer is not running (OK to stop even if not running)
    AppTimerDeepSleepPersistentStop(&WakeUpTimer);
  }

  memset(&gWakeupCcData, 0, sizeof(SWakeupCcData));
  gWakeupCcData.SleepPeriod = cc_wake_up_config_get_default_sleep_time_sec();
  ZAF_nvm_write(ZAF_FILE_ID_WAKEUPCCDATA, &gWakeupCcData, sizeof(SWakeupCcData));
}

/**
 * Initialize the WakeUp Command class with a file ID for the Wake Up Command Class data.
 */
static void
init(void)
{
  wakeUpIsActive = false;

  AppTimerDeepSleepPersistentRegister(&WakeUpTimer, false, TimerCallback);

  //Verify that a WAKEUPCCDATA file exists
  size_t   dataLen;
  const zpal_status_t status = ZAF_nvm_get_object_size(ZAF_FILE_ID_WAKEUPCCDATA, &dataLen);
  //If there is no file or there is size mismatch write a default file
  if ((ZPAL_STATUS_OK != status) || (ZAF_FILE_SIZE_WAKEUPCCDATA != dataLen)) {
    CC_WakeUp_notificationMemorySetDefault();
  }

  LoadData();

  zaf_set_stay_awake_callback(CC_WakeUp_stayAwakeIfActive);

  send_first_notification();
}

static void
reset(void)
{
  CC_WakeUp_notificationMemorySetDefault();

  init();
}

/*
 * Callback function for sending wakeup notification
 *
 * Called from transport layer (when processing ack) or from
 * CC_WakeUp_notification_tx if tx queue is full.
 */
void
ZCB_WakeUpNotificationCallback(uint8_t txStatus, TX_STATUS_TYPE* pExtendedTxStatus)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_CC_WAKE_UP, "\nWakeUpNotificationCallback() txStatus: %u", txStatus);

  if (NULL != ZCB_WakeUpTxCallback) {
    ZCB_WakeUpTxCallback(txStatus, pExtendedTxStatus);
  }
}

/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/****************************************************************************/

void CC_WakeUp_notification_tx(void (*pCallback)(uint8_t txStatus, TX_STATUS_TYPE* pExtendedTxStatus))
{
  SApplicationHandles *pAppHandle;
  pAppHandle = ZAF_getAppHandle();

  /* Only send wakeup notifiers when sensor is node in a network */
  /* and a recovery operation is not in progress */
  if (0 == pAppHandle->pNetworkInfo->NodeId) {
    if (NULL != pCallback) {
      TX_STATUS_TYPE extendedTxStatus = { 0 };
      pCallback(TRANSMIT_COMPLETE_FAIL, &extendedTxStatus);
    }
    return;
  }

  ZCB_WakeUpTxCallback = pCallback;
  wakeUpIsActive = true;
  CC_WakeUp_stayAwakeIfActive();

  if (true != CmdClassWakeupNotification(ZCB_WakeUpNotificationCallback)) {
    TX_STATUS_TYPE extendedTxStatus = { 0 };
    ZCB_WakeUpNotificationCallback(TRANSMIT_COMPLETE_FAIL, &extendedTxStatus);
  }
}

static bool CmdClassWakeupNotification(void (*pCallback)(uint8_t txStatus, TX_STATUS_TYPE* pExtendedTxStatus))
{
  SApplicationHandles *pAppHandle;
  pAppHandle = ZAF_getAppHandle();

  ZPAL_LOG_DEBUG(ZPAL_LOG_CC_WAKE_UP, "\r\nN%d", gWakeupCcData.MasterNodeId);

  uint8_t WakeUpNotificationFrame[] = {
    COMMAND_CLASS_WAKE_UP,
    WAKE_UP_NOTIFICATION_V2
  };

  SZwaveTransmitPackage FramePackage = {
    .uTransmitParams.SendDataEx.FrameConfig.Handle = pCallback,
    .uTransmitParams.SendDataEx.FrameConfig.TransmitOptions = ZWAVE_PLUS_TX_OPTIONS,
    .uTransmitParams.SendDataEx.FrameConfig.iFrameLength = sizeof(WakeUpNotificationFrame),
    .uTransmitParams.SendDataEx.SourceNodeId = 0x00,
    .uTransmitParams.SendDataEx.TransmitOptions2 = 0x00,
    .uTransmitParams.SendDataEx.TransmitSecurityOptions = S2_TXOPTION_VERIFY_DELIVERY,
    .uTransmitParams.SendDataEx.eKeyType = GetHighestSecureLevel(pAppHandle->pNetworkInfo->SecurityKeys),
    .eTransmitType = EZWAVETRANSMITTYPE_EX
  };

  if ((0x00 < gWakeupCcData.MasterNodeId) && (ZW_MAX_NODES >= gWakeupCcData.MasterNodeId)) {
    FramePackage.uTransmitParams.SendDataEx.DestNodeId = gWakeupCcData.MasterNodeId;
  } else { // CC:0084.01.00.32.003
    FramePackage.uTransmitParams.SendDataEx.DestNodeId = 1;
  }
  memcpy(FramePackage.uTransmitParams.SendDataEx.FrameConfig.aFrame,
         WakeUpNotificationFrame,
         sizeof(WakeUpNotificationFrame));

  // Put the package on queue (and dont wait for it)
  ZPAL_LOG_DEBUG(ZPAL_LOG_CC_WAKE_UP, "\r\nQNSTB");
  if (EQUEUENOTIFYING_STATUS_SUCCESS != QueueNotifyingSendToBack(pAppHandle->pZwTxQueue,
                                                                 (uint8_t*)&FramePackage,
                                                                 0)) {
    return false;
  }
  return true;
}

void TimerCallbackDummy(void)
{
  // Do nothing for now
}

/**
 * Start the timer for sending wakeup notifications.
 *
 * This function doesn't differentiate whether the timer is already
 * running or not. In either case the timer is reset and started with
 * the current timeout configuration.
 */
static void
CC_WakeUp_startWakeUpNotificationTimer(void)
{
  if (gWakeupCcData.SleepPeriod > 0) {
    AppTimerDeepSleepPersistentStart(&WakeUpTimer, gWakeupCcData.SleepPeriod * 1000);
  } else {
    AppTimerDeepSleepPersistentStop(&WakeUpTimer);
  }
}

static received_frame_status_t
CC_WakeUp_handler(
  RECEIVE_OPTIONS_TYPE_EX *rxOpt,
  ZW_APPLICATION_TX_BUFFER *pCmd,
  __attribute__((unused)) uint8_t cmdLength,
  ZW_APPLICATION_TX_BUFFER *pFrameOut,
  uint8_t * pFrameOutLength)
{
  switch (pCmd->ZW_Common.cmd) {
    case  WAKE_UP_INTERVAL_SET_V2:
    {
      uint32_t requestedSleepPeriod = 0;
      uint32_t setSleepPeriod = 0;

      // Always AND with FF the right place... Then we don't care whether 1's are shifted in.
      requestedSleepPeriod |= (((uint32_t)pCmd->ZW_WakeUpIntervalSetV2Frame.seconds1) << 16) & 0x00FF0000;
      requestedSleepPeriod |= (((uint32_t)pCmd->ZW_WakeUpIntervalSetV2Frame.seconds2) << 8)  & 0x0000FF00;
      requestedSleepPeriod |= (((uint32_t)pCmd->ZW_WakeUpIntervalSetV2Frame.seconds3) << 0)  & 0x000000FF;

      /* Calculate correct sleep-period dependent of step resolution */
      if (requestedSleepPeriod > 0) {
        if (requestedSleepPeriod < cc_wake_up_config_get_minimum_sleep_time_sec()) {
          setSleepPeriod = cc_wake_up_config_get_minimum_sleep_time_sec();
        } else if (requestedSleepPeriod > cc_wake_up_config_get_maximum_sleep_time_sec()) {
          setSleepPeriod = cc_wake_up_config_get_maximum_sleep_time_sec();
        } else if (0 == cc_wake_up_config_get_sleep_step_time_sec()) {
          /* If STEP is 0 then only MIN or MAX is allowed.
           * Choose the one closest to the requested sleepPeriod value.
           * (the validations above ensures that sleepPeriod is not outside [MIN; MAX]
           */
          if ( (requestedSleepPeriod - cc_wake_up_config_get_minimum_sleep_time_sec())
               < (cc_wake_up_config_get_maximum_sleep_time_sec() - requestedSleepPeriod) ) {
            setSleepPeriod = cc_wake_up_config_get_minimum_sleep_time_sec();
          } else {
            setSleepPeriod = cc_wake_up_config_get_maximum_sleep_time_sec();
          }
        } else {
          /**
           * The following formula ensures that the sleep period will always match a valid step
           * value.
           *
           *                       input - min
           * sleep_period = min + ------------- * step
           *                           step
           */
          setSleepPeriod = cc_wake_up_config_get_minimum_sleep_time_sec()
                           + ((requestedSleepPeriod - cc_wake_up_config_get_minimum_sleep_time_sec()) / cc_wake_up_config_get_sleep_step_time_sec()) * cc_wake_up_config_get_sleep_step_time_sec();
        }
      }

      /*
       * Save master node ID and sleep period
       *
       * Since we're changing both variables in the struct, we do not need to read the file
       * before changing it. Inserting static assert to make sure changes in the struct size are
       * caught.
       */
      _Static_assert(sizeof(SWakeupCcData) == 8, "STATIC_ASSERT_FAILED_wake_up_cc_struct_wrong_size");
      gWakeupCcData.MasterNodeId = pCmd->ZW_WakeUpIntervalSetV2Frame.nodeid;
      gWakeupCcData.SleepPeriod = setSleepPeriod;

      ZAF_nvm_write(ZAF_FILE_ID_WAKEUPCCDATA, &gWakeupCcData, sizeof(SWakeupCcData));

      CC_WakeUp_startWakeUpNotificationTimer();

      //Return failure in case requested sleep period could not be set.
      if (requestedSleepPeriod != setSleepPeriod) {
        return RECEIVED_FRAME_STATUS_FAIL;
      }
    }
      return RECEIVED_FRAME_STATUS_SUCCESS;
      break;
    case WAKE_UP_INTERVAL_GET_V2:
      if (true == Check_not_legal_response_job(rxOpt)) {
        // The Wake Up CC does not support endpoint bit addressing.
        return RECEIVED_FRAME_STATUS_FAIL;
      }

      pFrameOut->ZW_WakeUpIntervalReportV2Frame.cmdClass = pCmd->ZW_Common.cmdClass;
      pFrameOut->ZW_WakeUpIntervalReportV2Frame.cmd = WAKE_UP_INTERVAL_REPORT_V2;

      pFrameOut->ZW_WakeUpIntervalReportV2Frame.seconds1 = (uint8_t)(gWakeupCcData.SleepPeriod >> 16); // MSB
      pFrameOut->ZW_WakeUpIntervalReportV2Frame.seconds2 = (uint8_t)(gWakeupCcData.SleepPeriod >> 8);
      pFrameOut->ZW_WakeUpIntervalReportV2Frame.seconds3 = (uint8_t)(gWakeupCcData.SleepPeriod >> 0); // LSB

      // Get wakeup destination node from file (saved by WAKE_UP_INTERVAL_SET_V2 - see above)
      pFrameOut->ZW_WakeUpIntervalReportV2Frame.nodeid = gWakeupCcData.MasterNodeId;

      *pFrameOutLength = sizeof(ZW_WAKE_UP_INTERVAL_REPORT_FRAME);

      return RECEIVED_FRAME_STATUS_SUCCESS;
      break;

    case WAKE_UP_NO_MORE_INFORMATION_V2:
      /*
       * The wakeup destination/master node has nothing more to send.
       * From the WakeUp CC perspective we're ready to stop radio immediately.
       * We signal that to the radio module.
       */
      zpal_radio_revoke_stay_awake(&stay_awake_id);
      wakeUpIsActive = false;

      autoStayAwakeAfterInclusion = false;

      return RECEIVED_FRAME_STATUS_SUCCESS;
      break;

    case WAKE_UP_INTERVAL_CAPABILITIES_GET_V2:
      if (true == Check_not_legal_response_job(rxOpt)) {
        // The Wake Up CC does not support endpoint bit addressing.
        return RECEIVED_FRAME_STATUS_FAIL;
      }

      pFrameOut->ZW_WakeUpIntervalCapabilitiesReportV2Frame.cmdClass = pCmd->ZW_Common.cmdClass;
      pFrameOut->ZW_WakeUpIntervalCapabilitiesReportV2Frame.cmd = WAKE_UP_INTERVAL_CAPABILITIES_REPORT_V2;

      pFrameOut->ZW_WakeUpIntervalCapabilitiesReportV2Frame.minimumWakeUpIntervalSeconds1 = (uint8_t)(cc_wake_up_config_get_minimum_sleep_time_sec() >> 16);
      pFrameOut->ZW_WakeUpIntervalCapabilitiesReportV2Frame.minimumWakeUpIntervalSeconds2 = (uint8_t)(cc_wake_up_config_get_minimum_sleep_time_sec() >> 8);
      pFrameOut->ZW_WakeUpIntervalCapabilitiesReportV2Frame.minimumWakeUpIntervalSeconds3 = (uint8_t)(cc_wake_up_config_get_minimum_sleep_time_sec() >> 0);

      pFrameOut->ZW_WakeUpIntervalCapabilitiesReportV2Frame.maximumWakeUpIntervalSeconds1 = (uint8_t)(cc_wake_up_config_get_maximum_sleep_time_sec() >> 16);
      pFrameOut->ZW_WakeUpIntervalCapabilitiesReportV2Frame.maximumWakeUpIntervalSeconds2 = (uint8_t)(cc_wake_up_config_get_maximum_sleep_time_sec() >> 8);
      pFrameOut->ZW_WakeUpIntervalCapabilitiesReportV2Frame.maximumWakeUpIntervalSeconds3 = (uint8_t)(cc_wake_up_config_get_maximum_sleep_time_sec() >> 0);

      pFrameOut->ZW_WakeUpIntervalCapabilitiesReportV2Frame.defaultWakeUpIntervalSeconds1 = (uint8_t)(cc_wake_up_config_get_default_sleep_time_sec() >> 16);
      pFrameOut->ZW_WakeUpIntervalCapabilitiesReportV2Frame.defaultWakeUpIntervalSeconds2 = (uint8_t)(cc_wake_up_config_get_default_sleep_time_sec() >> 8);
      pFrameOut->ZW_WakeUpIntervalCapabilitiesReportV2Frame.defaultWakeUpIntervalSeconds3 = (uint8_t)(cc_wake_up_config_get_default_sleep_time_sec() >> 0);

      pFrameOut->ZW_WakeUpIntervalCapabilitiesReportV2Frame.wakeUpIntervalStepSeconds1 = (uint8_t)(cc_wake_up_config_get_sleep_step_time_sec() >> 16);
      pFrameOut->ZW_WakeUpIntervalCapabilitiesReportV2Frame.wakeUpIntervalStepSeconds2 = (uint8_t)(cc_wake_up_config_get_sleep_step_time_sec() >> 8);
      pFrameOut->ZW_WakeUpIntervalCapabilitiesReportV2Frame.wakeUpIntervalStepSeconds3 = (uint8_t)(cc_wake_up_config_get_sleep_step_time_sec() >> 0);

      *pFrameOutLength = sizeof(ZW_WAKE_UP_INTERVAL_CAPABILITIES_REPORT_V2_FRAME);

      return RECEIVED_FRAME_STATUS_SUCCESS;
      break;
    default:
      // Do nothing.
      break;
  }
  return RECEIVED_FRAME_STATUS_NO_SUPPORT;
}

/**
 * Ensures that the node stays awake for 10 seconds
 */
static void
CC_WakeUp_stayAwake10s(void)
{
  /* Keep radio alive the next 10 seconds */
  zpal_radio_update_stay_awake(&stay_awake_id, POST_INCLUSION_STAY_AWAKE_TIME);
}

/**
 * Ensure that the node stays awake to wait for more messages from the
 * wake up destination node.
 *
 * Should be called to ensure that the node stays awake the next 10 seconds if
 * it is active because it has woken up and notified the wake up destination
 * node. (the wake up destination node can cancel the 10 second sleep delay
 * by sending a WAKE_UP_NO_MORE_INFORMATION)
 */
static void CC_WakeUp_stayAwakeIfActive(void)
{
  if ((true == wakeUpIsActive) || (true == autoStayAwakeAfterInclusion)) {
    CC_WakeUp_stayAwake10s();
  }
  if (true == wakeUpIsActive) {
    CC_WakeUp_startWakeUpNotificationTimer();
  }
}

/**
 * Tell the device to auto extend the 10 second timer following an inclusion
 *
 * Every accepted application frame sent or received will normally cause a
 * device to stays awake for 10 seconds more when it has been woken up.
 *
 * Since entering learn mode for a not-included node is not triggering the
 * usual wakeup logic this function must be called to tell the wakeup CC to
 * extend the awake period in this case also.
 *
 * NB: A function to turn this special mode off is not needed. When we
 *     finally fall asleep after learn mode has completed we will go to
 *     deep sleep. Next wakeup will therefore be a reset where this mode by
 *     default is off.
 */
static void
CC_WakeUp_AutoStayAwakeAfterInclusion(void)
{
  autoStayAwakeAfterInclusion = true;
}

void
zaf_learn_mode_finished(void)
{
  // Make sure that the application stays awake for 10 s after learn mode finished
  // to wait for more messages from the controller
  CC_WakeUp_stayAwake10s();

  /* Also tell application to automatically extend the stay awake period by 10
   * seconds on message activities - even though we did not get here by a proper
   * wakeup from Deep Sleep
   */
  CC_WakeUp_AutoStayAwakeAfterInclusion();

  // Start the wakeup timer if the learn mode operation finished in Included state
  if (EINCLUSIONSTATE_EXCLUDED != ZAF_GetInclusionState()) {
    CC_WakeUp_startWakeUpNotificationTimer();
  }
}

REGISTER_CC_V4(COMMAND_CLASS_WAKE_UP, WAKE_UP_VERSION_V2, CC_WakeUp_handler, NULL, NULL, NULL, 0, init, reset);
