/*******************************************************************************
* @file  nvm_ids.h
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

#pragma once

#define EXAMPLE_NVM_OFFSET 0
#define EXAMPLE_NVM_ID(x)  (EXAMPLE_NVM_OFFSET + x)

typedef enum {
  SL_NET_WIFI_CLIENT_PROFILE_0_NVM_ID    = EXAMPLE_NVM_ID(0),
  SL_NET_WIFI_CLIENT_PROFILE_1_NVM_ID    = EXAMPLE_NVM_ID(1),
  SL_NET_WIFI_CLIENT_PROFILE_2_NVM_ID    = EXAMPLE_NVM_ID(2),
  SL_NET_WIFI_CLIENT_PROFILE_3_NVM_ID    = EXAMPLE_NVM_ID(3),
  SL_NET_WIFI_CREDENTIALS_0_NVM_ID       = EXAMPLE_NVM_ID(4),
  SL_NET_WIFI_CREDENTIALS_1_NVM_ID       = EXAMPLE_NVM_ID(5),
  SL_NET_WIFI_CREDENTIALS_2_NVM_ID       = EXAMPLE_NVM_ID(6),
  SL_NET_WIFI_CREDENTIALS_3_NVM_ID       = EXAMPLE_NVM_ID(7),
  SL_NET_WIFI_CREDENTIALS_4_NVM_ID       = EXAMPLE_NVM_ID(8),
  SL_NET_WIFI_ACCESSPOINT_PROFILE_NVM_ID = EXAMPLE_NVM_ID(9),
  SL_NET_SETUP_NVM_ID                    = EXAMPLE_NVM_ID(10),
  SL_BLE_GATT_TABLE1_NVM_ID              = EXAMPLE_NVM_ID(11),
  SL_BLE_GATT_TABLE2_NVM_ID              = EXAMPLE_NVM_ID(12),
  SL_NET_CLOUD_CONFIG_NVM_ID             = EXAMPLE_NVM_ID(13),
  SL_NET_CLOUD_CONNECTOR_NVM_ID          = EXAMPLE_NVM_ID(14),
  SL_NET_WIFI_AP_NVM_ID                  = EXAMPLE_NVM_ID(15),
  XMODEM_CONFIGURATION_NVM_ID            = EXAMPLE_NVM_ID(16),
} example_nvm_id_t;
