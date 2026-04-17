/**
 * Z-Wave Certified Application Switch On/Off
 *
 * @copyright 2018 Silicon Laboratories Inc.
 */

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include "MfgTokens.h"
#include "ZW_system_startup_api.h"
#include "zaf_config_security.h"
#include "ZAF_Common_helper.h"
#include "ZAF_Common_interface.h"
#include "ZAF_network_learn.h"
#include "ZAF_network_management.h"
#include "zpal_log.h"
#include "events.h"
#include "zpal_watchdog.h"
#include "board_indicator.h"
#include "ZAF_ApplicationEvents.h"
#include "CC_BinarySwitch.h"
#include "zaf_event_distributor_soc.h"
#include "zpal_misc.h"
#include "zaf_protocol_config.h"
#include "ZAF_PrintAppInfo.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif

#ifdef SL_CATALOG_ZW_CLI_COMMON_PRESENT
#include "zw_cli_common.h"
#endif

#include "app_hw.h"

void ApplicationTask(SApplicationHandles* pAppHandles);

/**
 * @brief See description for function prototype in ZW_basis_api.h.
 */
ZW_APPLICATION_STATUS
ApplicationInit(__attribute__((unused)) zpal_reset_reason_t eResetReason)
{
  SRadioConfig_t* RadioConfig;

  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "Enabling watchdog\n");
  zpal_watchdog_init();
  zpal_enable_watchdog(true);

  ZPAL_LOG_INFO(ZPAL_LOG_APP, "ApplicationInit eResetReason = %d\n", eResetReason);

  RadioConfig = zaf_get_radio_config();

  // Read Rf region from MFG_ZWAVE_COUNTRY_FREQ
  zpal_radio_region_t regionMfg = REGION_UNDEFINED;
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

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  // Add EM1 requirement to prevent application from entering EM2 sleep mode
  // The requirement is added to consistently keep always-on apps in EM1.
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif

  /* Enter SmartStart*/
  /* Protocol will commence SmartStart only if the node is NOT already included in the network */
  ZAF_setNetworkLearnMode(E_NETWORK_LEARN_MODE_INCLUSION_SMARTSTART);

  // Wait for and process events
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "SwitchOnOff Event processor Started\r\n");
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
zaf_event_distributor_app_event_manager(uint8_t event)
{
  /**************************** PUT YOUR CODE HERE ****************************\
  *
  * This is the main body of the application.
  * You can use the event manager to capture your own custom events
  * (which you can define in events.h) and execute related business logic.
  *
  \****************************************************************************/

  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "zaf_event_distributor_app_event_manager Ev: %d\r\n", event);
  cc_binary_switch_t * p_switches;

  switch (event) {
    case EVENT_APP_TOGGLE_LED:

      p_switches = cc_binary_switch_get_config();
      if (cc_binary_switch_get_current_value(&p_switches[0]) > 0) {
        cc_binary_switch_set(&p_switches[0], 0);
      } else {
        cc_binary_switch_set(&p_switches[0], 0xFF);   // 0xFF for last on value.
      }
      break;

    case EVENT_APP_SEND_NIF:
      /*
       * This shows how to force the transmission of an Included NIF. Normally, an INIF is
       * transmitted when the product is power cycled while already being included in a network.
       * If the enduser cannot power cycle the product, e.g. because the battery cannot be
       * removed, the transmission of an INIF must be manually triggered.
       */
      ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "TX INIF\n");
      ZAF_SendINIF(NULL);
      break;

    default:
      // Unknow event - do nothing.
      break;
  }

#ifdef SL_CATALOG_ZW_CLI_COMMON_PRESENT
  cli_log_system_events(event);
#endif
}
