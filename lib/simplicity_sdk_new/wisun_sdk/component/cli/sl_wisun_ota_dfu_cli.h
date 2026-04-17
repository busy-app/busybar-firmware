/***************************************************************************//**
 * @file sl_wisun_ota_dfu_cli.h
 * @brief Wi-SUN OTA DFU CLI handler
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

#ifndef SL_WISUN_OTA_DFU_CLI_H
#define SL_WISUN_OTA_DFU_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
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

/**************************************************************************//**
 * @brief Firmware update start CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_fw_update_start(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief Firmware update stop CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_fw_update_stop(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief Firmware reboot and install CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_fw_reboot_and_install(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief Firmware update status CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_fw_update_status(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief Set host address CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_set_host_addr(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief Get host address CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_get_host_addr(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief Set host URI CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_set_host_uri(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief Get host URI CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_get_host_uri(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief Set GBL path CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_set_gbl_path(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief Get GBL path CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_get_gbl_path(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief Set notify host CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_set_notify_host_addr(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief Get notify host CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_get_notify_host_addr(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief Set notify host URI CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_set_notify_host_uri(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief Get notify host URI CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_get_notify_host_uri(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief Set notify download chunk CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_set_notify_download_chunk(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief Get notify download chunk CLI handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_get_notify_download_chunk(const sl_cli_command_arg_t *arguments);

#ifdef __cplusplus
}
#endif

#endif // SL_WISUN_OTA_DFU_CLI_H
