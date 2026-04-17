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
 * @file         app_common_config.h
 * @version      0.1
 * @date         01 Feb 2021
 *
 *  @brief : This file contains user configurable details to configure the device
 *
 *  @section Description  This file contains user configurable details to configure the device
 */

#ifndef APP_COMMON_CONFIG_H
#define APP_COMMON_CONFIG_H

#include "stdint.h"
#include <string.h>
#include <stdbool.h>

#include "ble_config.h"

/*=======================================================================*/
//   ! MACROS
/*=======================================================================*/
//! Enable/Disable individual protocol activities
#define RSI_ENABLE_BLE_TEST  1 //! enable to test BLE functionality
#define RSI_ENABLE_WLAN_TEST 1 //! enable to test WLAN functionality

//! configure below macro for measuring WLAN throughput
#if RSI_ENABLE_WLAN_TEST
#define WLAN_THROUGHPUT_TEST 1 //! 1- TO Measure SSL/TCP/UDP tx/rx throughput
#endif

//! configure below macro for measuring BLE throughput
#if RSI_ENABLE_BLE_TEST
#define BLE_THROUGHPUT_TEST 0 //! 1- To Measure BLE throughput using sniffer, 0- default BLE operation
#endif

//! BLE controls
#if (RSI_ENABLE_BLE_TEST && WLAN_THROUGHPUT_TEST)
#define BLE_INIT_DONE           0 //! make it 1 for BLE init only
#define BLE_CONNECTED           0 //! make it 1 for BLE connection only
#define BLE_DATA_TRANSFER_START 1 //! make it 1 for BLE data transfer
#endif

//! WLAN controls
#if (RSI_ENABLE_WLAN_TEST && BLE_THROUGHPUT_TEST)
#define WLAN_SCAN_ONLY    1 //! make it 1 for wlan scan only
#define WLAN_CONNECT_ONLY 0 //! make it 1 for wlan connection only
#define WLAN_DATATRANSFER 1 //! make it 1 for wlan tcp download
#endif

#define WLAN_SYNC_REQ \
  1 //! Enable this macro to start BLE activities after WI-FI connection, else all activities starts concurrently

/*=======================================================================*/
//   ! Data type declarations
/*=======================================================================*/

/*=======================================================================*/
//   ! Function prototype declarations
/*=======================================================================*/
int32_t rsi_initiate_power_save(void);
void rsi_wlan_app_thread(void *unused);
void rsi_ble_task_on_conn(void *parameters);
void wlan_throughput_task(void);
void rsi_ble_main_app_task(void);
void rsi_ble_peripheral_app_task(void);
void rsi_common_app_task(void);
#endif
