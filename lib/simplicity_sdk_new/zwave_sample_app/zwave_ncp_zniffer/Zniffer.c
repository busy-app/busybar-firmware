/**
 * Z-Wave Zniffer Application
 *
 * @copyright 2022 Silicon Laboratories Inc.
 */

#include <stdbool.h>
#include <stdint.h>
#include "zw_config_rf.h"
#include <assert.h>
#include "MfgTokens.h"
#include "zpal_log.h"
#include "ZW_system_startup_api.h"
#include "zaf_config_security.h"
#include "ZAF_Common_helper.h"
#include "ZAF_Common_interface.h"
#include "ZAF_network_learn.h"
#include "ZAF_network_management.h"
#include "zpal_watchdog.h"
#include "app_hw.h"
#include "zw_region_config.h"
#include "ZAF_ApplicationEvents.h"
#include "ZAF_PrintAppInfo.h"

#include "zniffer_handler.h"

/**
 * Defines used to initialize the Association Group Information (AGI)
 * Command Class.
 *
 * @attention
 * The sum of ZAF_CONFIG_NUMBER_OF_END_POINTS, MAX_ASSOCIATION_GROUPS and MAX_ASSOCIATION_IN_GROUP
 * may not exceed 18 as defined by ASSOCIATION_ALLOCATION_MAX or an error will be thrown
 * during compilation.
 *
 * @attention
 * It is advised not to change the parameters once a product has been launched, as subsequent
 * upgrades will erase the old structure's content and start a fresh association table.
 */
#define MAX_ASSOCIATION_GROUPS      1
#define MAX_ASSOCIATION_IN_GROUP    5

/**
 * The value basic set command should use  when an event occur
 */
#define BASIC_SET_TRIGGER_VALUE     (0xFF)
/**
 * The timeout value in sec that used when sending basic set command due to an event
 */
#define BASIC_SET_TIMEOUT           10

/**
 * Security keys
 */
//@ [REQUESTED_SECURITY_KEYS_ID]
#define REQUESTED_SECURITY_KEYS   (SECURITY_KEY_S0_BIT | SECURITY_KEY_S2_UNAUTHENTICATED_BIT | SECURITY_KEY_S2_AUTHENTICATED_BIT)
//@ [REQUESTED_SECURITY_KEYS_ID]

/**
 * Set up security keys to request when joining a network.
 */
static const uint8_t SecureKeysRequested = ZAF_CONFIG_REQUESTED_SECURITY_KEYS;

static const SAppNodeInfo_t AppNodeInfo =
{
  .DeviceOptionsMask = ZAF_CONFIG_DEVICE_OPTION_MASK,
  .NodeType.generic = ZAF_CONFIG_GENERIC_TYPE,
  .NodeType.specific = ZAF_CONFIG_SPECIFIC_TYPE,
};

static SRadioConfig_t RadioConfig =
{
  .iListenBeforeTalkThreshold = ELISTENBEFORETALKTRESHOLD_DEFAULT,
  .iTxPowerLevelMax = APP_MAX_TX_POWER,
  .iTxPowerLevelAdjust = APP_MEASURED_0DBM_TX_POWER,
  .iTxPowerLevelMaxLR = APP_MAX_TX_POWER_LR,
  .eRegion = ZW_REGION,
  .radio_debug_enable = true, // The Zniffer must always enable the radio debug connection
  .radio_application = ZPAL_RADIO_APPLICATION_ZNIFFER,
};

static const SProtocolConfig_t ProtocolConfig = {
  .pVirtualSlaveNodeInfoTable = NULL,
  .pSecureKeysRequested = &SecureKeysRequested,
  .pNodeInfo = &AppNodeInfo,
  .pRadioConfig = &RadioConfig
};

void ApplicationTask(SApplicationHandles* pAppHandles);

/**
 * @brief See description for function prototype in ZW_basis_api.h.
 */
ZW_APPLICATION_STATUS
ApplicationInit(__attribute__((unused)) zpal_reset_reason_t eResetReason)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "Enabling watchdog\n");
  zpal_watchdog_init();
  zpal_enable_watchdog(true);

  ZPAL_LOG_INFO(ZPAL_LOG_APP, "ApplicationInit eResetReason = %d\n", eResetReason);
  ZAF_PrintAppInfo();

  // Read Rf region from MFG_ZWAVE_COUNTRY_FREQ
  zpal_radio_region_t regionMfg;
  ZW_GetMfgTokenDataCountryFreq((void*) &regionMfg);
  if (isRfRegionValid(regionMfg)) {
    RadioConfig.eRegion = regionMfg;
  } else {
    ZW_SetMfgTokenDataCountryRegion((void*) &RadioConfig.eRegion);
  }
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "Rf region: %d\n", RadioConfig.eRegion);

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
    &ProtocolConfig
    );
  assert(bWasTaskCreated);

  return(APPLICATION_RUNNING);
}

/**
 * A pointer to this function is passed to ZW_ApplicationRegisterTask() making it the FreeRTOS
 * application task.
 */
void
ApplicationTask(__attribute__((unused)) SApplicationHandles* pAppHandles)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "Zniffer Application Started\r\n");
  run_zniffer();
}

/*
 * The below functions should be implemented as hardware specific functions in a separate source
 * file, e.g. Zniffer_hw.c.
 */
ZW_WEAK void Zniffer_hw_init(void)
{
}

ZW_WEAK const void * Zniffer_get_uart_config_ext(void)
{
  return NULL;
}
