/***************************************************************************/ /**
 * @file wifi_app.h
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

#ifndef WIFI_APP_H
#define WIFI_APP_H

/***************************************************************************/ /**
 * Initialize application.
 ******************************************************************************/
#include <stdint.h>

#define APP_BUF_SIZE 256

//! SEND event number used in the application
#define RSI_SEND_EVENT BIT(0)

//! Enumeration for states in applcation
typedef enum wifi_app_state_e {
  WIFI_APP_INITIAL_STATE          = 0,
  WIFI_APP_UNCONNECTED_STATE      = 1,
  WIFI_APP_CONNECTED_STATE        = 2,
  WIFI_APP_IPCONFIG_DONE_STATE    = 3,
  WIFI_APP_SOCKET_CONNECTED_STATE = 4
} wifi_app_state_t;

//! wifi application control block
typedef struct wifi_app_cb_s {
  //! WiFi application state
  wifi_app_state_t state;

  //! length of buffer to copy
  uint32_t length;

  //! application buffer
  uint8_t buffer[APP_BUF_SIZE];

  //! to check application buffer availability
  uint8_t buf_in_use;

  //! application events bit map
  uint32_t event_map;
} wifi_app_cb_t;

/***************************************************************************/ /**
 * App ticking function.
 ******************************************************************************/
#if ENABLE_NWP_POWER_SAVE
int32_t rsi_initiate_power_save(void);
#endif
void wifi_ble_send_data(void);
#endif // WIFI_APP_H
