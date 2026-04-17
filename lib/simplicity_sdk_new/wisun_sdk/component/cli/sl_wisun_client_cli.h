/***************************************************************************//**
 * @file sl_wisun_client_cli.h
 * @brief Wi-SUN Client CLI handler
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

#ifndef SL_WISUN_CLIENT_CLI_H
#define SL_WISUN_CLIENT_CLI_H

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

/**************************************************************************//**
 * @brief Init Client CLI
 * @details Initialization of CLI client mempool
 * @param[in] void
 *****************************************************************************/
void sl_wisun_client_cli_init(void);

#if defined(SL_CATALOG_WISUN_TCP_CLIENT_PRESENT)
/**************************************************************************//**
 * @brief CLI app tcp client create handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_tcp_client(const sl_cli_command_arg_t *arguments);
#endif

#if defined(SL_CATALOG_WISUN_UDP_CLIENT_PRESENT)
/**************************************************************************//**
 * @brief CLI app udp client create handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_udp_client(const sl_cli_command_arg_t *arguments);
#endif

/**************************************************************************//**
 * @brief CLI app tcp/udp client close handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_socket_close(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief CLI app tcp/udp client write handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_socket_write(const sl_cli_command_arg_t *arguments);

/**************************************************************************//**
 * @brief CLI app tcp/udp client read handler
 * @details CLI function
 * @param[in] arguments Arguments
 *****************************************************************************/
void app_socket_read(const sl_cli_command_arg_t *arguments);

#ifdef __cplusplus
}
#endif

#endif // SL_WISUN_CLIENT_CLI_H
