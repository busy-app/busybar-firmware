/*******************************************************************************
* @file  app_common_config.h
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

#ifndef APP_COMMON_CONFIG_H
#define APP_COMMON_CONFIG_H
#include <stdio.h>
#include <stdint.h>
/*=======================================================================*/
//   ! MACROS
/*=======================================================================*/

//! Enable/Disable individual protocol activities
#define RSI_ENABLE_BLE_TEST  1
#define RSI_ENABLE_WLAN_TEST 1

#define WLAN_SYNC_REQ \
  1 //! Enable this macro to start ble activities after wlan connection, else all activities starts concurrently

/*=======================================================================*/
//!	Powersave configurations
/*=======================================================================*/
#define ENABLE_NWP_POWER_SAVE 0 //! Set to 1 for power save mode
#define PSP_MODE              RSI_SLEEP_MODE_2
#define PSP_TYPE              RSI_MAX_PSP

/*=======================================================================*/
//   ! Data type declarations
/*=======================================================================*/

/*=======================================================================*/
//   ! Function prototype declarations
/*=======================================================================*/
int32_t rsi_initiate_power_save(void);
int32_t rsi_wlan_app_task(void);
void rsi_ble_main_app_task(void);
void rsi_ble_task_on_conn(void *parameters);
void rsi_ui_app_task(void);
void rsi_common_app_task(void);
#endif
