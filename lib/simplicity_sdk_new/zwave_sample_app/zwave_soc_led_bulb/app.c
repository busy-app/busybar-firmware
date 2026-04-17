/**
 * Z-Wave Application LED Bulb
 *
 * @copyright 2020 Silicon Laboratories Inc.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "MfgTokens.h"
#include "zpal_log.h"
#include "ZW_system_startup_api.h"
#include "CC_MultilevelSwitch_Support.h"
#include "ZAF_Common_helper.h"
#include "ZAF_Common_interface.h"
#include "ZAF_network_learn.h"
#include "events.h"
#include "zpal_watchdog.h"
#include "board_indicator.h"
#include "zw_region_config.h"
#include "ZAF_ApplicationEvents.h"
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

static void ApplicationTask(SApplicationHandles* pAppHandles);

/**
 * @brief See description for function prototype in ZW_basis_api.h.
 */
ZW_APPLICATION_STATUS ApplicationInit(__attribute__((unused)) zpal_reset_reason_t eResetReason)
{
  SRadioConfig_t* RadioConfig;

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

  /*
   * Register the main application task.
   *
   * Attention: this is the only FreeRTOS task that can invoke the ZAF API.
   *
   * ZW_UserTask_CreateTask() can be used to create additional application tasks. See the
   * Sensor PIR application for an example use of ZW_UserTask_CreateTask().
   */
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
static void ApplicationTask(SApplicationHandles* pAppHandles)
{
  uint32_t unhandledEvents = 0;
  ZAF_Init(xTaskGetCurrentTaskHandle(), pAppHandles);

  ZAF_PrintAppInfo();

  app_hw_init();

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  // Add EM1 requirement to prevent application from entering EM2 sleep mode
  // The Timer peripheral used by the pwm or simple_rgb_pwm_led module is only available in EM1.
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif

  /* Enter SmartStart*/
  /* Protocol will commence SmartStart only if the node is NOT already included in the network */
  ZAF_setNetworkLearnMode(E_NETWORK_LEARN_MODE_INCLUSION_SMARTSTART);

  // Wait for and process events
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "LED Bulb Event processor Started\r\n");
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
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "zaf_event_distributor_app_event_manager Ev: %d\r\n", event);

  switch (event) {
    default:
      // Unknow event - do nothing.
      break;
  }

#ifdef SL_CATALOG_ZW_CLI_COMMON_PRESENT
  cli_log_system_events(event);
#endif
}
