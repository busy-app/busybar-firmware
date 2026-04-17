/***************************************************************************//**
 * @file
 * @brief Application Over-the-Air Device Firmware Update Configuration header
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef SL_BT_APP_OTA_DFU_CONFIG_H
#define SL_BT_APP_OTA_DFU_CONFIG_H

/***********************************************************************************************//**
 * @addtogroup app_ota_dfu
 * @{
 **************************************************************************************************/

#include "app_rta.h"

// <<< Use Configuration Wizard in Context Menu >>>

// <o SL_BT_APP_OTA_DFU_CONFIG_PRIORITY> Runtime context priority
// <APP_RTA_PRIORITY_LOW=> Low
// <APP_RTA_PRIORITY_BELOW_NORMAL=> Below normal
// <APP_RTA_PRIORITY_NORMAL=> Normal
// <APP_RTA_PRIORITY_ABOVE_NORMAL=> Above normal
// <APP_RTA_PRIORITY_HIGH=> High
// <i> Default: Normal
#define SL_BT_APP_OTA_DFU_CONFIG_PRIORITY      APP_RTA_PRIORITY_NORMAL

// <o SL_BT_APP_OTA_DFU_CONFIG_STACK> Stack size (in bytes)
// <i> Default: 2560
#define SL_BT_APP_OTA_DFU_CONFIG_STACK         2560

// <o SL_BT_APP_OTA_DFU_CONFIG_WAIT> Timeout for guard (in ticks)
// <i> Default: 10
#define SL_BT_APP_OTA_DFU_CONFIG_WAIT          10

// <<< end of configuration section >>>

/** @} (end addtogroup app_ota_dfu) */
#endif // SL_BT_APP_OTA_DFU_CONFIG_H
