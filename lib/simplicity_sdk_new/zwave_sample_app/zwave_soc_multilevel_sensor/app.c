/**
 * Z-Wave Application Multilevel Sensor
 *
 * @copyright 2018 Silicon Laboratories Inc.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include "MfgTokens.h"
#include "zpal_log.h"
#include "ZAF_file_ids.h"
#include "ZAF_nvm_app.h"
#include "ZAF_nvm.h"
#include "AppTimer.h"
#include "ZW_system_startup_api.h"
#include "CC_Battery.h"
#include "CC_MultilevelSensor_Support.h"
#include "ZAF_Common_helper.h"
#include "ZAF_Common_interface.h"
#include "ZAF_network_learn.h"
#include "ZAF_network_management.h"
#include "events.h"
#include "zpal_watchdog.h"
#include "board_indicator.h"
#include "ZAF_ApplicationEvents.h"
#include "zaf_event_distributor_soc.h"
#include "zpal_misc.h"
#include "zaf_protocol_config.h"
#include "ZW_TransportEndpoint.h"
#include "ZAF_PrintAppInfo.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZW_CLI_SLEEPING_PRESENT
#include "zw_cli_sleeping.h"
#include "zw_cli_sleeping_config.h"
#endif

#ifdef SL_CATALOG_ZW_SHUTDOWN_MANAGER_PRESENT
#include "zw_shutdown_manager.h"
#endif // SL_CATALOG_ZW_SHUTDOWN_MANAGER_PRESENT

#include "app_hw.h"

void ApplicationTask(SApplicationHandles* pAppHandles);

/**
 * @brief See description for function prototype in ZW_basis_api.h.
 */
ZW_APPLICATION_STATUS
ApplicationInit(__attribute__((unused)) zpal_reset_reason_t eResetReason)
{
  SRadioConfig_t* RadioConfig;

#ifdef SL_CATALOG_ZW_SHUTDOWN_MANAGER_PRESENT
  zw_shutdown_manager_init();
#endif
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

  /* Register task function */
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
  * @see Sensor_MultiThread example for more info.
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
  zpal_reset_reason_t resetReason;

  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "Multilevel Sensor Main App/Task started!\n");

  ZAF_Init(xTaskGetCurrentTaskHandle(), pAppHandles);

  ZAF_PrintAppInfo();

  app_hw_init();

  resetReason = GetResetReason();

  /* Re-load and process Deep Sleep persistent application timers.
   * NB: Before calling AppTimerDeepSleepPersistentLoadAll here all
   *     application timers must have been been registered with
   *     AppTimerRegister() or AppTimerDeepSleepPersistentRegister().
   *     Essentially it means that all CC handlers must be
   *     initialized first.
   */
  AppTimerDeepSleepPersistentLoadAll(resetReason);

  if (ZPAL_RESET_REASON_DEEP_SLEEP_EXT_INT == resetReason) {
    app_hw_deep_sleep_wakeup_handler();
  }

  /**
   * Set the maximum inclusion request interval for SmartStart.
   * Valid range 0 and 5-99. 0 is default value and corresponds to 512 sec.
   * The range 5-99 corresponds to 640-12672sec in units of 128sec/tick in between.
   */
  ZAF_SetMaxInclusionRequestIntervals(0);

  if (ZPAL_RESET_REASON_DEEP_SLEEP_EXT_INT != resetReason) {
    /* Enter SmartStart*/
    /* Protocol will commence SmartStart only if the node is NOT already included in the network */
    ZAF_setNetworkLearnMode(E_NETWORK_LEARN_MODE_INCLUSION_SMARTSTART);
  }

  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "IsWakeupCausedByRtccTimeout=%s\n", (IsWakeupCausedByRtccTimeout()) ? "true" : "false");
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "CompletedSleepDurationMs   =%u\n", GetCompletedSleepDurationMs());

#ifdef SL_CATALOG_ZW_CLI_SLEEPING_PRESENT
  // Stay awake to allow user to send the prevent sleeping command through the CLI
  if (GetResetReason() == ZPAL_RESET_REASON_PIN) {
    zw_cli_sleeping_util_prevent_sleeping_timeout(ZW_CLI_SLEEPING_WAKEUP_TIME_AFTER_RESET);
  }
#endif

  // Wait for and process events
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "Multilevel Sensor Event Distributor Started\n");
  for (;; ) {
    unhandledEvents = zaf_event_distributor_distribute();
    if (0 != unhandledEvents) {
      ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "Unhandled Events: 0x%08lx\n", unhandledEvents);
#ifdef UNIT_TEST
      return;
#endif
    }
  }
}

/**
 * @brief The core state machine of this sample application.
 * @param event The event that triggered the call of zaf_event_distributor_app_event_manager.
 */
void
zaf_event_distributor_app_event_manager(const uint8_t event)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "zaf_event_distributor_app_event_manager Ev: 0x%02x\r\n", event);

  switch (event) {
    case EVENT_APP_SEND_BATTERY_LEVEL_AND_SENSOR_REPORT:
      (void) CC_Battery_LevelReport_tx(NULL, ENDPOINT_ROOT, NULL);
      cc_multilevel_sensor_send_sensor_data();
      break;
    default:
      break;
  }

#ifdef SL_CATALOG_ZW_CLI_COMMON_PRESENT
  cli_log_system_events(event);
#endif
}

void
zaf_nvm_app_reset(void)
{
  AppTimerDeepSleepPersistentResetStorage();
}
