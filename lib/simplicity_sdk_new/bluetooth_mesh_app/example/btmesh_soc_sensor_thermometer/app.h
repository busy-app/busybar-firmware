/***************************************************************************//**
 * @file
 * @brief Application interface provided to main().
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

#ifndef APP_H
#define APP_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_component_catalog.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SL_CATALOG_APP_LOG_PRESENT
#define app_log(...)
#define app_log_status_error_f(...)
#endif // SL_CATALOG_APP_LOG_PRESENT

#ifdef SL_CATALOG_BTMESH_WSTK_LCD_PRESENT
#define lcd_print(...) sl_btmesh_LCD_write(__VA_ARGS__)
#else
#define lcd_print(...)
#endif // SL_CATALOG_BTMESH_WSTK_LCD_PRESENT

/*******************************************************************************
 * Change buttons to LEDs in case of shared pin
 ******************************************************************************/
void app_change_buttons_to_leds(void);

/*******************************************************************************
 * Change LEDs to buttons in case of shared pin
 ******************************************************************************/
void app_change_leds_to_buttons(void);

/***************************************************************************//**
 * Handles button press and does a factory reset
 *
 * @return true if there is no button press
 ******************************************************************************/
bool app_handle_reset_conditions(void);

/**************************************************************************//**
 * Check if it is required to process with execution.
 * @return true if required, false otherwise.
 *****************************************************************************/
bool app_is_process_required(void);

/**************************************************************************//**
 * Proceed with execution. (Indicate that it is required to run the application
 * process action.)
 *****************************************************************************/
void app_proceed(void);

/**************************************************************************//**
 * Initialize the application for BT Mesh.
 *****************************************************************************/
void app_permanent_memory_alloc(void);

/**************************************************************************//**
 * Initialization steps for BT Mesh after the kernel is started in case of RTOS
 * In case of bare metal, this function only exists to maintain the same API.
 *****************************************************************************/
void app_init_runtime(void);

/***************************************************************************//**
 * Shows the provisioning start information
 * @param[in] result  Result code. 0: success, non-zero: error
 ******************************************************************************/
void app_show_btmesh_node_provisioning_started(uint16_t result);

/***************************************************************************//**
 * Shows the provisioning fail information
 * @param[in] result  Result code. 0: success, non-zero: error
 ******************************************************************************/
void app_show_btmesh_node_provisioning_failed(uint16_t result);

/***************************************************************************//**
 * Shows the provisioning completed information
 * @param[in] address      Unicast address of the primary element of the node.
                           Ignored if unprovisioned.
 * @param[in] iv_index     IV index for the first network of the node
                           Ignored if unprovisioned.
 ******************************************************************************/
void app_show_btmesh_node_provisioned(uint16_t address, uint32_t iv_index);

#endif // APP_H
