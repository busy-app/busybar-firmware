/********************************************************************************
 * @file  app_xapi_common.c
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
#include "sl_net_wifi_types.h"

DEFINE_XAPI_CLASS(wifi, common, WIFI, COMMON, NULL, NULL);

/******************************************************
*               Function Definitions
******************************************************/
void app_wifi_cmd_common_get_firmware_version(void)
{
  sl_status_t status                  = SL_STATUS_FAIL;
  sl_si91x_firmware_version_t version = { 0 }; // Define structure for firmware version details
  status                              = sl_si91x_get_firmware_version(&version); // Get firmware version details
  app_wifi_rsp_common_get_firmware_version(status,
                                           version.chip_id,
                                           version.rom_id,
                                           version.major,
                                           version.minor,
                                           version.security_version,
                                           version.patch_num,
                                           version.customer_id,
                                           version.build_num); // Send firmware version details
}
