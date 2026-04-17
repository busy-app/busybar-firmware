/***************************************************************************/ /**
 * @file
 * @brief Bluetooth Network Co-Processor (NCP) Interface
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

#ifndef APP_NCP_H
#define APP_NCP_H

/***********************************************************************************************/ /**
 * @addtogroup ncp
 * @{
 **************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"
#include "app_ncp_config.h"
#include "app_wifi_api.h"
//#include "sl_power_manager.h"

/**************************************************************************/ /**
 * NCP initialization function.
 *****************************************************************************/
void app_ncp_init(void);

/**************************************************************************/ /**
 * NCP process action function.
 *****************************************************************************/
void app_ncp_step(void);

/**************************************************************************/ /**
 * Local event processor
 *****************************************************************************/
bool app_ncp_local_evt_process(app_wifi_msg_t *evt);

/**************************************************************************/ /**
 * Check if NCP allows go to sleep
 *****************************************************************************/
bool app_internal_ncp_is_ok_to_sleep(void);

/**************************************************************************/ /**
 * Routine for power manager handler
 *****************************************************************************/
//sl_power_manager_on_isr_exit_t app_internal_ncp_sleep_on_isr_exit(void);

/**************************************************************************/ /**
 * OS initialization function - if the OS is present
 *****************************************************************************/
void app_ncp_os_task_init(void);

/**************************************************************************/ /**
 * Function to trigger the OS task to proceed - if the OS is present
 *****************************************************************************/
void app_ncp_os_task_proceed(void);

/** @} (end addtogroup ncp) */
#endif // APP_NCP_H
