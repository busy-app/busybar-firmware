/*******************************************************************************
* @file  wifi_config.h
* @brief
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
/**
 * @file         wifi_config.h
 * @version      0.1
 *
 *  @brief : This file contains user configurable details to configure the device
 *
 *  @section Description  This file contains user configurable details to configure the device
 * 
 */

#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include "socket.h"
#include "sl_si91x_socket_utility.h"
#include "sl_si91x_socket_constants.h"
#include "sl_si91x_core_utilities.h"
#include "sl_si91x_socket.h"
#include "sl_si91x_socket_support.h"
#include <errno.h>

#define APP_BUF_SIZE 1600

#define WIFI_AP_PROFILE_SSID "MY_DUAL_AP_SSID"
#define WIFI_AP_CREDENTIAL   "MY_AP_PASSPHRASE"

#define AP_ONLY_MODE 0 // Set to 1 to run in AP only mode, 0 for concurrent mode

#if AP_ONLY_MODE
#define WIFI_MAX_CLIENTS 8
#else
#define WIFI_MAX_CLIENTS 4
#endif

// Type of throughput
#define UDP_TX 1
#define TCP_TX 2

#define THROUGHPUT_TYPE 0 // Set this to enable throughput testing

// Memory length for send buffer
#define TCP_BUFFER_SIZE 1460
#define UDP_BUFFER_SIZE 1470

#if (THROUGHPUT_TYPE == UDP_TX)
#define BUFFER_SIZE UDP_BUFFER_SIZE
#elif (THROUGHPUT_TYPE == TCP_TX)
#define BUFFER_SIZE TCP_BUFFER_SIZE
#endif

//! Enumeration for states in application
typedef enum wifi_app_state_e {
  WIFI_APP_INITIAL_STATE          = 0,
  WIFI_APP_UNCONNECTED_STATE      = 1,
  WIFI_APP_CONNECTED_STATE        = 2,
  WIFI_APP_IPCONFIG_DONE_STATE    = 3,
  WIFI_APP_SOCKET_CONNECTED_STATE = 4,
  WIFI_APP_SCAN_STATE             = 5,
  WIFI_APP_JOIN_STATE             = 6,
  WIFI_APP_IDLE_STATE             = 23,
  WIFI_APP_DISCONNECTED_STATE     = 11,
  WIFI_APP_DISCONN_NOTIFY_STATE   = 12,
  WIFI_AP_INIT_STATE              = 13,
  WIFI_AP_BRING_UP_STATE          = 14,
  WIFI_AP_BRING_DOWN_STATE        = 15,
  WIFI_ENABLE_NAT                 = 16,
  WIFI_DISABLE_NAT                = 17,
} wifi_app_state_t;

typedef enum wifi_app_cmd_e {
  WIFI_APP_DATA                      = 0,
  WIFI_APP_SCAN_RESP                 = 1,
  WIFI_APP_CONNECTION_STATUS         = 2,
  WIFI_APP_DISCONNECTION_STATUS      = 3,
  WIFI_APP_DISCONNECTION_NOTIFY      = 4,
  WIFI_APP_TIMEOUT_NOTIFY            = 5,
  WIFI_APP_REQ_BLE_DISCONNECT        = 6,
  WIFI_APP_REQ_BLE_START_ADVERTISING = 7,
  WIFI_APP_REQ_BLE_STOP_ADVERTISING  = 8
} wifi_app_cmd_t;

/*****************************************************
 *                      Socket configuration
*****************************************************/

#define TOTAL_SOCKETS                   WIFI_MAX_CLIENTS //@ Total number of sockets. TCP TX + TCP RX + UDP TX + UDP RX
#define TOTAL_TCP_SOCKETS               WIFI_MAX_CLIENTS //@ Total TCP sockets. TCP TX + TCP RX
#define TOTAL_UDP_SOCKETS               0                //@ Total UDP sockets. UDP TX + UDP RX
#define TCP_TX_ONLY_SOCKETS             0                //@ Total TCP TX only sockets. TCP TX
#define TCP_RX_ONLY_SOCKETS             0                //@ Total TCP RX only sockets. TCP RX
#define UDP_TX_ONLY_SOCKETS             0                // UDP TX only sockets
#define UDP_RX_ONLY_SOCKETS             0                // UDP RX only sockets
#define TCP_RX_HIGH_PERFORMANCE_SOCKETS 0                //@ Total TCP RX High Performance sockets
#define TCP_RX_WINDOW_SIZE_CAP          44               //@ TCP RX Window size
#define TCP_RX_WINDOW_DIV_FACTOR        44               //@ TCP RX Window division factor

#define LISTENING_PORT 5000
#define BACK_LOG       WIFI_MAX_CLIENTS

#define BYTES_TO_SEND    (1 << 29) //512MB
#define BYTES_TO_RECEIVE (1 << 28) //256MB
#define TEST_TIMEOUT     (30000)   //30sec

#define SL_HIGH_PERFORMANCE_SOCKET BIT(7)

/*=======================================================================*/
// NWP buffer allocation parameters
/*=======================================================================*/

#ifndef TX_POOL_RATIO
#define TX_POOL_RATIO 1
#endif

#ifndef RX_POOL_RATIO
#define RX_POOL_RATIO 1
#endif

#ifndef GLOBAL_POOL_RATIO
#define GLOBAL_POOL_RATIO 1
#endif

#endif
