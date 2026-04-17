/***************************************************************************//**
 * @file
 * @brief Application Over-the-Air Device Firmware Update
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
#ifndef SLI_BT_APP_OTA_DFU_H
#define SLI_BT_APP_OTA_DFU_H

#include "sl_bt_api.h"

/**************************************************************************//**
 * Application OTA DFU initialization.
 *****************************************************************************/
void sli_bt_app_ota_dfu_init(void);

/**************************************************************************//**
 * Finalize initialization. (App RTA calls that requires scheduler.)
 *****************************************************************************/
void sli_bt_app_ota_dfu_rta_ready(void);

/**************************************************************************//**
 * Bluetooth stack event handler.
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sli_bt_app_ota_dfu_on_event(sl_bt_msg_t *evt);

#endif // SLI_BT_APP_OTA_DFU_H
