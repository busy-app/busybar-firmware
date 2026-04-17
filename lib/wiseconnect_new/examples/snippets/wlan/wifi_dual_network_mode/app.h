/***************************************************************************/ /**
 * @file app.h
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
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
#define SL_SI91X_APP_BUF_SIZE 1600
#define TX_RX_RATIO_ENABLE    1

/*=======================================================================*/
// NWP buffer allocation parameters
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
/*=======================================================================*/

//! Enumeration for states in application
typedef enum wlan_app_state_e {
  WLAN_INITIAL_STATE = 0,
  WLAN_UNCONNECTED_STATE,
  WLAN_NET_UP_STATE,
  WLAN_POWERSAVE_STATE,
  WLAN_IP_CONFIG_RETRY_STATE,
  WLAN_SOCKET_CREATE_STATE,
  WLAN_SEND_SOCKET_DATA_STATE,
  WLAN_SOCKET_CLOSE_STATE,
  WLAN_NET_DOWN_STATE,
} wlan_app_state_t;

//! WLAN application control block
typedef struct wlan_app_cb_s {
  volatile wlan_app_state_t state;       //! WLAN application state
  uint32_t length;                       //! length of buffer to copy
  uint8_t buffer[SL_SI91X_APP_BUF_SIZE]; //! application buffer
  uint8_t buf_in_use;                    //! to check application buffer availability
  uint32_t event_map;                    //! application events bit map
} wlan_app_cb_t;

/***************************************************************************/ /**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void);

#endif // APP_H
