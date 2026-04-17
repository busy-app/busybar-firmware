/*******************************************************************************
* @file  wifi_app_config.h
* @brief
*******************************************************************************
* # License
* <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @date         01 Feb 2021
 *
 *  @brief : This file contains user configurable details to configure the device
 *
 *  @section Description  This file contains user configurable details to configure the device
 */

#ifndef WIFI_APP_CONFIG_H
#define WIFI_APP_CONFIG_H

#include "app_common_config.h"

/*=======================================================================================================*/
//!	 APP CONFIG defines
/*=======================================================================================================*/
#define SSID              "AndroidAP5655" //! Access point SSID to connect
#define SECURITY_TYPE     SL_WIFI_WPA2    //! Security type -  RSI_WPA2 or RSI_OPEN
#define CHANNEL_NO        0               //! Channel in which device should scan, 0 - scan all channels
#define PSK               "12345678"      //! Password
#define DEVICE_PORT       5005            //! Module port number
#define SERVER_PORT       5000            //! Server port number
#define SERVER_IP_ADDRESS "192.168.43.99" //! Server IP address
#define SSL_SERVER_PORT   443             //! Server port number of SSL client
#define DHCP_MODE         1               //! DHCP mode 1- Enable 0- Disable

//! Type of throughput
#define UDP_RX 1
#define UDP_TX 2
#define TCP_TX 4
#define TCP_RX 8
#define SSL_TX 16
#define SSL_RX 32

#define THROUGHPUT_TYPE UDP_TX

#define TCP_BUFFER_SIZE     1460
#define UDP_BUFFER_SIZE     1470
#define SSL_BUFFER_SIZE     1370
#define DEFAULT_BUFFER_SIZE 1460

#define CONTINUOUS_THROUGHPUT \
  0 /* Enable this only while measuring wlan alone throughput
                              1- Measure throughput for every interval of 'THROUGHPUT_AVG_TIME'
                              0- Measure throughput only once for interval of 'THROUGHPUT_AVG_TIME' */
#define TX_RX_RATIO_ENABLE 0

#if ((THROUGHPUT_TYPE == SSL_TX) || (THROUGHPUT_TYPE == SSL_RX))
#define SSL 1
//! Load certificate to device flash :
//! Certificate could be loaded once and need not be loaded for every boot up
#define LOAD_CERTIFICATE 1
#endif
#define RSI_APP_BUF_SIZE 1600

/*=======================================================================*/
//!	Powersave configurations
/*=======================================================================*/
#define ENABLE_NWP_POWER_SAVE 0 //! Set 1 to enable power save mode
#define PSP_MODE              RSI_SLEEP_MODE_8
#define PSP_TYPE              RSI_MAX_PSP

/*=======================================================================*/
//! Socket configuration
/*=======================================================================*/
#define TCP_RX_WINDOW_SIZE_CAP   44 //@ TCP RX Window size - 64KB (44 * 1460)
#define TCP_RX_WINDOW_DIV_FACTOR 44
#define USE_SELECT_FEATURE       0

//! Enumeration for states in application
typedef enum rsi_wlan_app_state_e {
  RSI_WLAN_INITIAL_STATE          = 0,
  RSI_WLAN_UNCONNECTED_STATE      = 1,
  RSI_WLAN_SCAN_DONE_STATE        = 2,
  RSI_WLAN_CONNECTED_STATE        = 3,
  RSI_WLAN_IPCONFIG_DONE_STATE    = 4,
  RSI_WLAN_DISCONNECTED_STATE     = 5,
  RSI_WLAN_SOCKET_CONNECTED_STATE = 6,
  RSI_POWER_SAVE_STATE            = 7,
  RSI_WLAN_IDLE_STATE             = 8,
} rsi_wlan_app_state_t;

//! WLAN application control block
typedef struct rsi_wlan_app_cb_s {
  rsi_wlan_app_state_t state;       //! WLAN application state
  uint32_t length;                  //! length of buffer to copy
  uint8_t buffer[RSI_APP_BUF_SIZE]; //! application buffer
  uint8_t buf_in_use;               //! to check application buffer availability
  uint32_t event_map;               //! application events bit map
} rsi_wlan_app_cb_t;

typedef struct wlan_throughput_config_s {
  osSemaphoreId_t wlan_app_sem;
  osSemaphoreId_t wlan_app_sem1;
  uint8_t thread_id;
  uint8_t throughput_type;
  //uint32_t server_port;
  //uint8_t *server_addr;
} wlan_throughput_config_t;

sl_status_t clear_and_load_certificates_in_flash(void);
#endif
