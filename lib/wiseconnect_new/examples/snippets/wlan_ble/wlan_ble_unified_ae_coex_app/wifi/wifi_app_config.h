/*******************************************************************************
* @file  wifi_app_config.h
* @brief
*******************************************************************************
* # License
* <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
******************************************************************************/
/**
 * @file         wifi_app_config.h
 * @version      0.1
 * @date         01 JAN 2025
 *
 *  @brief : This file contains user configurable details to configure the device
 *
 *  @section Description  This file contains user configurable details to configure the device
 */

#ifndef WIFI_APP_CONFIG_H
#define WIFI_APP_CONFIG_H
#include "stdint.h"
#include "sl_status.h"
#include "sl_si91x_socket_utility.h"
/*=======================================================================================================*/
//!	 APP CONFIG defines
/*=======================================================================================================*/
// If this macro is enabled, the WiFi thread continuously scans for the APs and prints the scan results.
#define WIFI_CONTINUOUS_SCAN_MODE_ONLY 0 //! 0 - Disable, 1 - Enable

// If this macro is enabled, the WiFi thread connects to the AP, prints the IP address, and then gets suspended.
#define WIFI_CONNECTION_ONLY 0 //! 0 - Disable, 1 - Enable

// Type of throughput
#define UDP_TX 0
#define UDP_RX 1
#define TCP_TX 2
#define TCP_RX 3
#define TLS_TX 4
#define TLS_RX 5

// Access point configurations
#define SSID          "YOUR_AP_SSID"       //! Access point SSID to connect
#define SECURITY_TYPE SL_WIFI_WPA2         //! Security type -  RSI_WPA2 or RSI_OPEN
#define CHANNEL_NO    0                    //! Channel in which device should scan, 0 - scan all channels
#define PSK           "YOUR_AP_PASSPHRASE" //! Password

// Throughput measurement type
#define THROUGHPUT_TYPE TCP_TX
#define SERVER_IP       "192.168.43.99" //! Server IP address
#define DEVICE_PORT     5005            //! Module port number
// Server port number
#if ((THROUGHPUT_TYPE == TLS_RX) || (THROUGHPUT_TYPE == TLS_TX))
#define SERVER_PORT 443
#else
#define SERVER_PORT 5000
#endif
// Module port number
#define LISTENING_PORT 5005
#define BACK_LOG       1

#define DHCP_MODE 1 //! DHCP mode 1- Enable 0- Disable

#if ((THROUGHPUT_TYPE == TLS_RX) || (THROUGHPUT_TYPE == TLS_TX))
#define SSL 1
//! Load certificate to device flash :
//! Certificate could be loaded once and need not be loaded for every boot up
#define LOAD_CERTIFICATE 1
#endif

#define RSI_APP_BUF_SIZE 1600

#define BACK_LOG 1

#define BYTES_TO_SEND    (1 << 30) //1024 MB
#define BYTES_TO_RECEIVE (1 << 30) //1024 MB
#define TEST_TIMEOUT     (30000)   //30sec

#define SL_HIGH_PERFORMANCE_SOCKET BIT(7)

#define SOCKET_ASYNC_FEATURE 1 //disable this macro to use synchronous sockets

/*****************************************************
 *                      Socket configuration
*****************************************************/
#define TOTAL_SOCKETS                   3  //@ Total number of sockets. TCP TX + TCP RX + UDP TX + UDP RX
#define TOTAL_TCP_SOCKETS               3  //@ Total TCP sockets. TCP TX + TCP RX
#define TOTAL_UDP_SOCKETS               0  //@ Total UDP sockets. UDP TX + UDP RX
#define TCP_TX_ONLY_SOCKETS             1  //@ Total TCP TX only sockets. TCP TX
#define TCP_RX_ONLY_SOCKETS             1  //@ Total TCP RX only sockets. TCP RX
#define UDP_TX_ONLY_SOCKETS             0  //@ Total UDP TX only sockets. UDP TX
#define UDP_RX_ONLY_SOCKETS             0  //@ Total UDP RX only sockets. UDP RX
#define TCP_RX_HIGH_PERFORMANCE_SOCKETS 1  //@ Total TCP RX High Performance sockets
#define TCP_RX_WINDOW_SIZE_CAP          44 //@ TCP RX Window size
#define TCP_RX_WINDOW_DIV_FACTOR        44 //@ TCP RX Window division factor

/******************************************************
 *                    Constants
 ******************************************************/
/******************************************************

 *               Variable Definitions
 ******************************************************/

static sl_si91x_socket_config_t socket_config = {
  TOTAL_SOCKETS,                   // Total sockets
  TOTAL_TCP_SOCKETS,               // Total TCP sockets
  TOTAL_UDP_SOCKETS,               // Total UDP sockets
  TCP_TX_ONLY_SOCKETS,             // TCP TX only sockets
  TCP_RX_ONLY_SOCKETS,             // TCP RX only sockets
  UDP_TX_ONLY_SOCKETS,             // UDP TX only sockets
  UDP_RX_ONLY_SOCKETS,             // UDP RX only sockets
  TCP_RX_HIGH_PERFORMANCE_SOCKETS, // TCP RX high performance sockets
  TCP_RX_WINDOW_SIZE_CAP,          // TCP RX window size
  TCP_RX_WINDOW_DIV_FACTOR         // TCP RX window division factor
};

//! Enumeration for states in application
typedef enum wlan_app_state_e {
  WLAN_INITIAL_STATE          = 0,
  WLAN_UNCONNECTED_STATE      = 1,
  WLAN_SCAN_DONE_STATE        = 2,
  WLAN_CONNECTED_STATE        = 3,
  WLAN_IPCONFIG_DONE_STATE    = 4,
  WLAN_DISCONNECTED_STATE     = 5,
  WLAN_SOCKET_CONNECTED_STATE = 6,
  NWP_POWER_SAVE_STATE        = 7,
  WLAN_IDLE_STATE             = 8,
} wlan_app_state_t;

//! WLAN application control block
typedef struct wlan_app_cb_s {
  wlan_app_state_t state;           //! WLAN application state
  uint32_t length;                  //! length of buffer to copy
  uint8_t buffer[RSI_APP_BUF_SIZE]; //! application buffer
  uint8_t buf_in_use;               //! to check application buffer availability
  uint32_t event_map;               //! application events bit map
} wlan_app_cb_t;

#if (THROUGHPUT_TYPE == TLS_TX || THROUGHPUT_TYPE == TLS_RX)
sl_status_t clear_and_load_certificates_in_flash(void);
#endif

#endif
