/***************************************************************************/ /**
 * @file app.h
 * @brief Top level application functions
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

#ifndef APP_H
#define APP_H

/***************************************************************************/ /**
 * Initialize application.
 ******************************************************************************/
void app_init(void);

#include <stdio.h>
#include <stdint.h>

#define APP_BUF_SIZE 1600

/*=======================================================================*/
// TA buffer allocation parameters
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

/*****************************************************
 *                      Socket configuration
*****************************************************/
#define TOTAL_SOCKETS                   6  //@ Total number of sockets. TCP TX + TCP RX + UDP TX + UDP RX
#define TOTAL_TCP_SOCKETS               1  //@ Total TCP sockets. TCP TX + TCP RX
#define TOTAL_UDP_SOCKETS               5  //@ Total UDP sockets. UDP TX + UDP RX
#define TCP_TX_ONLY_SOCKETS             0  //@ Total TCP TX only sockets. TCP TX
#define TCP_RX_ONLY_SOCKETS             0  //@ Total TCP RX only sockets. TCP RX
#define UDP_TX_ONLY_SOCKETS             5  //@ Total UDP TX only sockets. UDP TX
#define UDP_RX_ONLY_SOCKETS             0  //@ Total UDP RX only sockets. UDP RX
#define TCP_RX_HIGH_PERFORMANCE_SOCKETS 1  //@ Total TCP RX High Performance sockets
#define TCP_RX_WINDOW_SIZE_CAP          44 //@ TCP RX Window size
#define TCP_RX_WINDOW_DIV_FACTOR        44 //@ TCP RX Window division factor

#define MAX_SERVER_CLEINT_SOCKETS \
  (TOTAL_SOCKETS + TCP_RX_HIGH_PERFORMANCE_SOCKETS + TCP_RX_ONLY_SOCKETS) // client sockets + server sockets

#define LOG_PRINT(...)                          \
  {                                             \
    osMutexAcquire(printf_mutex, 0xFFFFFFFFUL); \
    printf(__VA_ARGS__);                        \
    osMutexRelease(printf_mutex);               \
  }

//! Enumeration for states in application
typedef enum app_state_e {
  INITIAL_STATE = 0,       // This state is to bring up module as AP.
  PROVISION_STATE,         // This state is to get Third-party AP credentials through HTTP SERVER & AP DOWN
  STA_BRINGUP_STATE,       // This state is to bring module as CONC_STA
  AP_BRINGUP_STATE,        // This state is to bring module as CONC_AP & start HTTP SERVER
  DATA_TRANSFER_STATE,     // This state is to perform data transfer on STA & AP Vaps
  WAIT_STATE,              // This state is to keep the module in idle state
  MODULE_DISCONNECT_STATE, // This state is to stop HTTP SERVER & CONC_AP upon rejoin failure
} app_state_t;

//! WLAN application control block
typedef struct app_cb_s {
  volatile app_state_t state;   //! WLAN application state
  uint32_t length;              //! length of buffer to copy
  uint8_t buffer[APP_BUF_SIZE]; //! application buffer
  uint8_t buf_in_use;           //! to check application buffer availability
  uint32_t event_map;           //! application events bit map
} app_cb_t;

/***************************************************************************/ /**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void);

#endif // APP_H
