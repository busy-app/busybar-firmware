/***************************************************************************//**
 * @file sl_iperf_cli.h
 * @brief iPerf CLI handler
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

#ifndef SL_IPERF_CLI_H
#define SL_IPERF_CLI_H

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
 * @brief iPerf CLI get parameter
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void sl_iperf_cli_get(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief iPerf CLI set parameter
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void sl_iperf_cli_set(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief iPerf CLI execute server test
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void sl_iperf_cli_exec_server(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief iPerf CLI execute server test
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void sl_iperf_cli_exec_client(const sl_cli_command_arg_t *arguments);

#ifdef __cplusplus
}
#endif

#endif // SL_IPERF_CLI_H
