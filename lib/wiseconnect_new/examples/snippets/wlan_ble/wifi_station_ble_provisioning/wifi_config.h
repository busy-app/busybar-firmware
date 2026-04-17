/*******************************************************************************
* @file  wifi_config.h
* @brief
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file         wifi_config.h
 * @version      0.1
 *
 *  @brief : This file contains user configurable details to configure the device
 *
 *  @section Description  This file contains user configurable details to configure the device
 *
 *
 */
#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// Enumeration for states in application
typedef enum wifi_app_state_e {
  WIFI_APP_INITIAL_STATE             = 0,
  WIFI_APP_UNCONNECTED_STATE         = 1,
  WIFI_APP_CONNECTED_STATE           = 2,
  WIFI_APP_IPCONFIG_DONE_STATE       = 3,
  WIFI_APP_SCAN_STATE                = 4,
  WIFI_APP_JOIN_STATE                = 5,
  WIFI_APP_SOCKET_RECEIVE_STATE      = 6,
  WIFI_APP_MQTT_INIT_DONE_STATE      = 7,
  WIFI_APP_MQTT_SUBSCRIBE_DONE_STATE = 8,
  BLE_APP_GATT_WRITE_EVENT           = 9,
  WIFI_APP_DISCONNECTED_STATE        = 10,
  WIFI_APP_DISCONN_NOTIFY_STATE      = 11,
  WIFI_APP_ERROR_STATE               = 12,
  WIFI_APP_FLASH_STATE               = 13,
  WIFI_APP_DATA_RECEIVE_STATE        = 14,
  WIFI_APP_SD_WRITE_STATE            = 15,
  WIFI_APP_DEMO_COMPLETE_STATE       = 16
} wifi_app_state_t;

typedef enum wifi_app_cmd_e {
  WIFI_APP_DATA                 = 0,
  WIFI_APP_SCAN_RESP            = 1,
  WIFI_APP_CONNECTION_STATUS    = 2,
  WIFI_APP_DISCONNECTION_STATUS = 3,
  WIFI_APP_DISCONNECTION_NOTIFY = 4,
  WIFI_APP_TIMEOUT_NOTIFY       = 5
} wifi_app_cmd_t;

#endif
