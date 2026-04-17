/***************************************************************************//**
 * @file
 * @brief Parse header file
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

#ifndef PARSE_H
#define PARSE_H

#include "sl_bt_api.h"
#include "app_log_cli.h"
#include "ncp_host.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup parse
 * @{
 ******************************************************************************/

/*******************************************************************************
 * Public Macros and Definitions
 ******************************************************************************/
// Optstring argument for getopt.
#define OPTSTRING      NCP_HOST_OPTSTRING APP_LOG_OPTSTRING "o:a:b:s:xeth"
// Usage info.
#define USAGE          APP_LOG_NL "%s " NCP_HOST_USAGE  APP_LOG_USAGE \
  APP_LOG_NL "[-o <file_name>] [-a <bt_address>] [-s <sampling_rate>] \
[-x] [-e] [-t] [-h]" APP_LOG_NL

/*******************************************************************************
 * Public Function Declarations
 ******************************************************************************/

/***************************************************************************//**
 *  \brief  Print bluetooth address on stdout.
 *  \param[in]  bluetooth address
 ******************************************************************************/
void print_address(bd_addr *addr);

/***************************************************************************//**
 *  \brief  Parse application arguments.
 *  \param[in]  argc  number of arguments
 *  \param[in]  argv  arguments array
 ******************************************************************************/
void parse_arguments(int argc, char **argv);

/***************************************************************************//**
 *  \brief  Print help on stdout.
 ******************************************************************************/
void help(void);

/***************************************************************************//**
 *  \brief  Print configuration parameters on stdout.
 ******************************************************************************/
void print_configuration(void);

/** @} (end addtogroup parse) */

#ifdef __cplusplus
};
#endif // __cplusplus

#endif /* PARSE_H */
