/***************************************************************************//**
 * @brief Connect - NCP application
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "sl-connect-assert.h"
#include "sl_component_catalog.h"
#include "app_framework_common.h"
#include "sl_cpc.h"
#include "sl_platform_version.h"
#include "sl_memory_manager.h"
#include "sli-connect-api.h"
#include "stack-info.h"
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sli_power_manager.h"
#endif

#define RAIL_RX_FIFO_SIZE 2048

void app_init(void)
{
  emberSetRadioRxFifo(RAIL_RX_FIFO_SIZE);
}

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
/**
 * @brief Determines if the application is in a state where it is safe to enter sleep mode.
 *
 * This function is required for backward compatibility following the update from sl_system to sl_main.
 *
 * @return false Always returns false, indicating that it is not safe to sleep.
 */
bool app_is_ok_to_sleep(void)
{
  return false;
}
#endif

/**
 * The version returned by getting the app version through CPC is a char string
 * (e.g. "4.3.1").
 * This function can also be refined to return a user-defined application specific version
 */
const char* sl_cpc_secondary_app_version(void)
{
  return SL_PLATFORM_VERSION_STR;
}
