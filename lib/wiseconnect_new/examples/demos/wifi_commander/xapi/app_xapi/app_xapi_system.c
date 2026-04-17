/********************************************************************************
 * @file  app_xapi_system.c
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "sl_status.h"
#include "app_wifi_xapi_cmd_rx_handler.h"
#include "si91x_device.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_hal_soc_soft_reset.h"
#endif

#define WIFI_SCAN_TIMEOUT    10000
#define ENABLE_POWERSAVE_CLI 1

#define FIRST_PARAM(sc, ...) sc
#define SET_RSP_AND_RETURN(commander_rsp_func, ...) \
  do {                                              \
    commander_rsp_func(__VA_ARGS__);                \
    return;                                         \
  } while (0)

#define SET_RSP_AND_RETURN_ON_FAILURE(commander_rsp_func, ...) \
  do {                                                         \
    if ((FIRST_PARAM(__VA_ARGS__)) != SL_STATUS_OK) {          \
      SET_RSP_AND_RETURN(commander_rsp_func, __VA_ARGS__);     \
    }                                                          \
  } while (0)

#define SYSTEM_INITIALIZE_SET_RSP_AND_RETURN_ON_FAILURE(sc) \
  SET_RSP_AND_RETURN_ON_FAILURE(commander_rsp_system_initialize, sc)
#define SYSTEM_START_SCAN_SET_RSP_AND_RETURN_ON_FAILURE(sc) \
  SET_RSP_AND_RETURN_ON_FAILURE(commander_rsp_system_start_scan, sc)

DEFINE_XAPI_CLASS(wifi, system, WIFI, SYSTEM, NULL, NULL);

void app_wifi_cmd_system_hello(const void *nil)
{
  (void)nil;
  app_wifi_rsp_system_hello(SL_STATUS_OK);
}

void app_wifi_cmd_system_echo(const app_wifi_cmd_system_echo_t *msg)
{
  app_wifi_rsp_system_echo(SL_STATUS_OK, msg->number_in);
}

void app_wifi_cmd_system_reset(const void *nil)
{
  (void)nil;

#ifdef SLI_SI91X_MCU_INTERFACE
  sl_si91x_soc_nvic_reset();
#endif

  return;
}
