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
 *
 *  @brief : This file contains user configurable details to configure the device
 *
 *  @section Description  This file contains user configurable details to configure the device
 *
 *
 */
#include "rsi_common_app.h"
#if (WIFI_APP == MQTT_APP)
#if WLAN_TASK_ENABLE
#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

//! Access point SSID to connect
#define SSID "YOUR_AP_SSID"

//! Password
#define PSK "YOUR_AP_PASSPHRASE"

//! Security type
#define SECURITY_TYPE SL_WIFI_WPA2

#define CERTIFICATE_INDEX 0

#define SUBSCRIBE_TO_TOPIC "aws_status"     //! Subscribe to topic to receive the message from the cloud.
#define PUBLISH_ON_TOPIC   "siwx91x_status" //! Publish topic to send the status from application to cloud.
#define MQTT_PUBLISH_PAYLOAD \
  "Hi from SiWx91x" //! Publish message. Increase the AWS_IOT_MQTT_TX_BUF_LEN value in aws_iot_config.h for larger payloads.
#define SUBSCRIBE_QOS       QOS1  //! Quality of Service for subscribed topic "SUBSCRIBE_TO_TOPIC".
#define PUBLISH_QOS         QOS1  //! Quality of Service for publish topic "PUBLISH_ON_TOPIC".
#define PUBLISH_PERIODICITY 30000 //! Publish periodicity in milliseconds.
#define MQTT_USERNAME       "username"
#define MQTT_PASSWORD       "password"
#define LOW                 0

// If this macro is enabled, the WiFi thread continuously scans for the APs and prints the scan results.
#define WIFI_CONTINUOUS_SCAN_MODE_ONLY 0 //! 0 - Disable, 1 - Enable

// If this macro is enabled, the WiFi thread connects to the AP, prints the IP address, and then gets suspended.
#define WIFI_CONNECTION_ONLY 0 //! 0 - Disable, 1 - Enable

#define RSI_APP_BUF_SIZE 1600

//! Enumeration for states in application
typedef enum wifi_app_state_e {
  WIFI_APP_INITIAL_STATE        = 0,
  WIFI_APP_UNCONNECTED_STATE    = 1,
  WIFI_APP_FLASH_STATE          = 2,
  WIFI_APP_CONNECTED_STATE      = 3,
  WIFI_APP_IPCONFIG_DONE_STATE  = 4,
  WIFI_APP_SCAN_STATE           = 5,
  WIFI_APP_JOIN_STATE           = 6,
  WIFI_APP_DISCONNECTED_STATE   = 7,
  WIFI_APP_DISCONN_NOTIFY_STATE = 8,

  WIFI_APP_AWS_SELECT_CONNECT_STATE      = 9,
  WIFI_APP_MQTT_INIT_STATE               = 10,
  WIFI_APP_MQTT_CONNECT_STATE            = 11,
  WIFI_APP_MQTT_AUTO_RECONNECT_SET_STATE = 12,
  WIFI_APP_MQTT_SUBSCRIBE_STATE          = 13,
  WIFI_APP_MQTT_PUBLISH_STATE            = 14,
  WIFI_APP_MQTT_DISCONNECT               = 15,
  WIFI_APP_MQTT_RECEIVE_STATE            = 16,
  WIFI_APP_SLEEP_STATE                   = 17,
  WIFI_APP_IDLE_STATE                    = 18
} wifi_app_state_t;

#endif /* WIFI_CONFIG_H */
#endif
#endif
