/***************************************************************************/ /**
 * @file  si91x_sntp_client_types.h
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
#pragma once
#include "stdint.h"

/******************************************************
 * *                      Macros
 * ******************************************************/
//SNTP client feature
#define SLI_SI91X_SNTP_CLIENT_CREATE            1
#define SLI_SI91X_SNTP_CLIENT_GETTIME           2
#define SLI_SI91X_SNTP_CLIENT_GETTIME_DATE      3
#define SLI_SI91X_SNTP_CLIENT_GETSERVER_ADDRESS 4
#define SLI_SI91X_SNTP_CLIENT_DELETE            5
#define SLI_SI91X_SNTP_CLIENT_GET_SERVER_INFO   6
#define SLI_SI91X_SNTP_CLIENT_SERVER_ASYNC_RSP  7

/******************************************************
 * *                 Type Definitions
 * ******************************************************/

// Define for SNTP client initialization
typedef struct {
  uint8_t command_type;
  uint8_t ip_version;
  union {
    uint8_t ipv4_address[4];
    uint8_t ipv6_address[16];
  } server_ip_address;
  uint8_t sntp_method;
  uint8_t sntp_timeout[2];
} sli_si91x_sntp_client_t;

typedef struct {
  uint8_t command_type;
  uint8_t ip_version;
  union {
    uint8_t ipv4_address[4];
    uint8_t ipv6_address[16];
  } server_ip_address;
  uint8_t sntp_method;
} sli_si91x_sntp_server_info_rsp_t;

typedef struct {
  uint8_t ip_version;
  union {
    uint8_t ipv4_address[4];
    uint8_t ipv6_address[16];
  } server_ip_address;
  uint8_t sntp_method;
} sli_si91x_sntp_server_rsp_t;