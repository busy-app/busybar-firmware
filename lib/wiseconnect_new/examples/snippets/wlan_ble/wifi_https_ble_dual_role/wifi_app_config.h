/*******************************************************************************
* @file  wifi_app_config.h
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
 *
 *  @brief : This file contains user configurable details to configure the device  
 *
 *  @section Description  This file contains user configurable details to configure the device 
 */

#ifndef WIFI_APP_CONFIG_H
#define WIFI_APP_CONFIG_H

/*=======================================================================*/
//  ! INCLUDES
/*=======================================================================*/
#include <stdint.h>

/***********************************************************************************************************************************************/
//! APP CONFIG defines
/**********************************************************************************************************************************************/
#define SSID          "AndroidAP5655" //! Access point SSID to connect
#define SECURITY_TYPE SL_WIFI_WPA2    //! Security type -  RSI_WPA2 or RSI_OPEN
#define PSK           "12345678"      //! Password
#define CHANNEL_NO    0
#define DHCP_MODE     1 //! DHCP mode 1- Enable 0- Disable

#define WLAN_SCAN_ONLY       0 //! Enable this for WLAN scanning only case
#define WLAN_CONNECTION_ONLY 0 //! Enable this for WLAN connection only case

#define SERVER_IP_ADDRESS "192.168.43.99" //! Server IP address
#define HTTPS_DOWNLOAD    1               //! Enable this to test HTTPS download and also set RX_DATA to '1'
#if HTTPS_DOWNLOAD
#define SSL              1   //! Enable SSL or not
#define LOAD_CERTIFICATE 1   //! Load certificate to device flash :
#define SERVER_PORT      443 //! Server port number
#else
#define SERVER_PORT 80 //! Server port number
#endif
#define DOWNLOAD_FILENAME "index.html"

#define CONTINUOUS_HTTP_DOWNLOAD 0
#define SOCKET_ASYNC_FEATURE     0
#define RSI_APP_BUF_SIZE         1600
#define RECV_BUFFER_SIZE         1500

#define USE_CONNECTION_CLOSE 0 //! 1- Module sends connection close headers to server

//! Enumeration for states in application
typedef enum rsi_wlan_app_state_e {
  RSI_WLAN_INITIAL_STATE = 0,
  RSI_WLAN_UNCONNECTED_STATE,
  RSI_WLAN_SCAN_STATE,
  RSI_WLAN_JOIN_STATE,
  RSI_WLAN_CONNECTED_STATE,
  RSI_WLAN_IPCONFIG_DONE_STATE,
  RSI_WLAN_DISCONNECTED_STATE,
  RSI_WLAN_SOCKET_CONNECTED_STATE,
  RSI_WLAN_DATA_RECEIVE_STATE,
  RSI_POWER_SAVE_STATE
} rsi_wlan_app_state_t;

//! WLAN application control block
typedef struct rsi_wlan_app_cb_s {
  volatile rsi_wlan_app_state_t state; //! WLAN application state
  uint32_t length;                     //! length of buffer to copy
  uint8_t buffer[RSI_APP_BUF_SIZE];    //! application buffer
  uint8_t buf_in_use;                  //! to check application buffer availability
  uint32_t event_map;                  //! application events bit map
} rsi_wlan_app_cb_t;

#endif /* WIFI_APP_CONFIG_H */
