/***************************************************************************//**
 * @file sl_wisun_cli.h
 * @brief Wi-SUN CLI handler
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

#ifndef SL_WISUN_CLI_H
#define SL_WISUN_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_component_catalog.h"
#include "sl_cli.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

#if defined(SL_CATALOG_APP_PROJECT_INFO_PRESENT)
/**************************************************************************//**
 * @brief CLI app about handler
 * @details CLI function
 * @param[in] void
 *****************************************************************************/
void app_about(void);
#endif

#if defined(SL_CATALOG_WISUN_BR_STACK_PRESENT)
/**************************************************************************//**
 * @brief CLI app reboot handler
 * @details CLI function
 * @param[in] void
 *****************************************************************************/
void app_reboot(void);
#endif

#if defined(SL_CATALOG_WISUN_APP_CORE_PRESENT)
#if !defined(SL_CATALOG_WISUN_BR_STACK_PRESENT)
/**************************************************************************//**
 * @brief CLI app connect to FAN 1.0 handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_join_fan10(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief CLI app connect to FAN 1.1 handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_join_fan11(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief CLI app connect explicit handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_join_explicit(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief CLI app disconnect handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_disconnect(const sl_cli_command_arg_t *arguments);
#endif

/**************************************************************************//**
 * @brief CLI app MAC allow handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_mac_allow(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief CLI app MAC deny handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_mac_deny(const sl_cli_command_arg_t *arguments);
#endif

#if defined(SL_CATALOG_WISUN_STACK_DEBUG_PRESENT)
/**************************************************************************//**
 * @brief CLI app set trace level handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_set_trace_level(const sl_cli_command_arg_t *arguments);
#endif

/**************************************************************************//**
 * @brief App CLI handler
 * @details CLI function
 * @param[in] args Arguments
 *****************************************************************************/
void app_cli_handler(const sl_cli_command_arg_t *args);

/**************************************************************************//**
 * @brief OTA DFU CLI handler
 * @details CLI function
 * @param[in] args Arguments
 *****************************************************************************/
void sl_wisun_ota_dfu_cli_handler(const sl_cli_command_arg_t *args);

/**************************************************************************//**
 * @brief iPerf CLI handler
 * @details CLI function
 * @param[in] args Arguments
 *****************************************************************************/
void sl_iperf_cli_handler(const sl_cli_command_arg_t *args);

#if defined(SL_CATALOG_WISUN_BR_STACK_PRESENT)
#if defined(SL_CATALOG_WISUN_APP_CORE_PRESENT)
/**************************************************************************//**
 * @brief CLI app connect to FAN 1.0 handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_start_fan11(const sl_cli_command_arg_t *arguments);
#endif

/**************************************************************************//**
 * @brief CLI app connect to FAN 1.0 handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_stop(const sl_cli_command_arg_t *arguments);
#endif

#if defined(SL_CATALOG_WISUN_BR_WIFI_PRESENT)
/**************************************************************************//**
 * @brief CLI app connect to Wi-Fi handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_wifi_connect(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief CLI app disconnect from Wi-Fi handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_wifi_disconnect(const sl_cli_command_arg_t *arguments);
#endif

#ifdef __cplusplus
}
#endif

#endif // SL_WISUN_CLI_H
