/**
 * Z-Wave Certified Application Door Lock Key Pad
 *
 * @copyright 2018 Silicon Laboratories Inc.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "zw_config_rf.h"
#include <assert.h>
#include "MfgTokens.h"
#include "zpal_log.h"
#include "AppTimer.h"
#include "SwTimer.h"
#include "ZW_system_startup_api.h"
#include "CC_DoorLock.h"
#include "CC_Battery.h"
#include "zaf_config_security.h"
#include "ZAF_Common_helper.h"
#include "ZAF_Common_interface.h"
#include "ZAF_network_learn.h"
#include "events.h"
#include "zpal_watchdog.h"
#include "board_indicator.h"
#include "app_credentials.h"
#include "ZAF_ApplicationEvents.h"
#include "zaf_event_distributor_soc.h"
#include "zpal_misc.h"
#include "zaf_protocol_config.h"
#include "ZAF_PrintAppInfo.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZW_CLI_COMMON_PRESENT
#include "zw_cli_common.h"
#endif

#include "app_hw.h"

/* Interval for checking and reporting battery status (in minutes) */
#define PERIODIC_BATTERY_CHECKING_INTERVAL_MINUTES 5

static SSwTimer BatteryCheckTimer;

void ApplicationTask(SApplicationHandles* pAppHandles);
void ZCB_BatteryCheckTimerCallback(SSwTimer *pTimer);
void CredentialLearnTimerCallback(SSwTimer *pTimer);

/**
 * @brief See description for function prototype in ZW_basis_api.h.
 */
ZW_APPLICATION_STATUS ApplicationInit(__attribute__((unused)) zpal_reset_reason_t eResetReason)
{
  SRadioConfig_t* RadioConfig;

  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "Enabling watchdog\n");
  zpal_watchdog_init();
  zpal_enable_watchdog(true);

  ZPAL_LOG_INFO(ZPAL_LOG_APP, "ApplicationInit eResetReason = %d\n", eResetReason);

  RadioConfig = zaf_get_radio_config();

  // Read Rf region from MFG_ZWAVE_COUNTRY_FREQ
  zpal_radio_region_t regionMfg;
  ZW_GetMfgTokenDataCountryFreq((void*) &regionMfg);
  if (isRfRegionValid(regionMfg)) {
    RadioConfig->eRegion = regionMfg;
  } else {
    ZW_SetMfgTokenDataCountryRegion((void*) &RadioConfig->eRegion);
  }

  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "Rf region: %d\n", RadioConfig->eRegion);

  /*************************************************************************************
  * CREATE USER TASKS  -  ZW_ApplicationRegisterTask() and ZW_UserTask_CreateTask()
  *************************************************************************************
  * Register the main APP task function.
  *
  * ATTENTION: This function is the only task that can call ZAF API functions!!!
  * Failure to follow guidelines will result in undefined behavior.
  *
  * Furthermore, this function is the only way to register Event Notification
  * Bit Numbers for associating to given event handlers.
  *
  * ZW_UserTask_CreateTask() can be used to create additional tasks.
  * @see zwave_soc_sensor_pir example for more info.
  *************************************************************************************/
  __attribute__((unused)) bool bWasTaskCreated = ZW_ApplicationRegisterTask(
    ApplicationTask,
    EAPPLICATIONEVENT_ZWRX,
    EAPPLICATIONEVENT_ZWCOMMANDSTATUS,
    zaf_get_protocol_config()
    );
  assert(bWasTaskCreated);

  return(APPLICATION_RUNNING);
}

/**
 * A pointer to this function is passed to ZW_ApplicationRegisterTask() making it the FreeRTOS
 * application task.
 */
void
ApplicationTask(SApplicationHandles* pAppHandles)
{
  uint32_t unhandledEvents = 0;
  ZAF_Init(xTaskGetCurrentTaskHandle(), pAppHandles);

  ZAF_PrintAppInfo();

  app_hw_init();

  /* Timer for periodic battery level checking */
  AppTimerRegister(&BatteryCheckTimer, true, ZCB_BatteryCheckTimerCallback);
  TimerStart(&BatteryCheckTimer, PERIODIC_BATTERY_CHECKING_INTERVAL_MINUTES * 60 * 1000);

  // User Credential Command Class related functions
  credentials_init();

  /* Enter SmartStart*/
  /* Protocol will commence SmartStart only if the node is NOT already included in the network */
  ZAF_setNetworkLearnMode(E_NETWORK_LEARN_MODE_INCLUSION_SMARTSTART);

  // Wait for and process events
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "DoorLockKeyPad Event processor Started\r\n");
  for (;;) {
    unhandledEvents = zaf_event_distributor_distribute();
    if (0 != unhandledEvents) {
      ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "Unhandled Events: 0x%08lx\n", unhandledEvents);
#ifdef UNIT_TEST
      return;
#endif
    }
  }
}

static void
send_battery_level_report(void)
{
  (void) CC_Battery_LevelReport_tx(NULL, ENDPOINT_ROOT, NULL);
}

/**
 * @brief The core state machine of this sample application.
 * @param event The event that triggered the call of zaf_event_distributor_app_event_manager.
 */
void
zaf_event_distributor_app_event_manager(const uint8_t event)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "zaf_event_distributor_app_event_manager Ev: %d\r\n", event);

  switch (event) {
    case EVENT_APP_BATTERY_REPORT:
      ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "\r\nBattery Level report transmit (keypress trig)\r\n");
      send_battery_level_report();
      break;
    case EVENT_APP_PERIODIC_BATTERY_CHECK_TRIGGER:
      /* Check the battery level and send a report to lifeline if required */
      if (cc_battery_check_level_changed()) {
        send_battery_level_report();
      }
      break;
    case EVENT_APP_DOORHANDLE_ACTIVATED:
    {
      /* Outside door handle #1 activated? */
      CC_DoorLock_SetOutsideDoorHandleState(DOOR_HANDLE_1);
      break;
    }
    case EVENT_APP_DOORHANDLE_DEACTIVATED:
    {
      /* Outside door handle #1 deactivated? */
      CC_DoorLock_ClearOutsideDoorHandleState(DOOR_HANDLE_1);
      break;
    }
    case EVENT_SYSTEM_LEARNMODE_FINISHED:
    {
      initialize_user_credential_database();
      break;
    }
    case EVENT_APP_CREDENTIAL_LEARN_START:
    {
      // The Credential Learn process has started
      request_credential_from_user();
      break;
    }
    default:
      break;
  }

#ifdef SL_CATALOG_ZW_CLI_COMMON_PRESENT
  cli_log_system_events(event);
#endif
}

/**
 * @brief Timer function for periodic battery level checking
 * @param pTimer Timer object assigned to this function
 */
void
ZCB_BatteryCheckTimerCallback(__attribute__((unused)) SSwTimer *pTimer)
{
  /* Send a battery level report to the lifeline  */
  if (false == zaf_event_distributor_enqueue_app_event(EVENT_APP_PERIODIC_BATTERY_CHECK_TRIGGER)) {
    ZPAL_LOG_ERROR(ZPAL_LOG_APP, "\r\n** Periodic battery checking trigger FAILED\r\n");
  }
}

// Weak functions for non-user credential application
ZW_WEAK void credentials_init(void)
{
}

ZW_WEAK void initialize_user_credential_database(void)
{
}

ZW_WEAK void request_credential_from_user(void)
{
}

ZW_WEAK void user_credential_app_event_handler(__attribute__((unused)) const uint8_t event, __attribute__((unused)) const void *data)
{
}
