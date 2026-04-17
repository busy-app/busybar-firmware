/***************************************************************************//**
 * @file sl_wisun_ble_dmp_advertise_controller.c
 * @brief Wi-SUN BLE DMP Advertise Controller
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>

#include "sl_assert.h"
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "sl_button.h"
#include "sl_simple_button_instances.h"
#include "sl_status.h"
#include "ble_peer_manager_peripheral.h"
#include "ble_peer_manager_common.h"
#include "ble_peer_manager_common_config.h"
#include "ble_peer_manager_connections.h"
#include "sl_wisun_led_driver.h"
#include "sl_wisun_ble_dmp_advertise_controller.h"

_Static_assert(BLE_PEER_MANAGER_COMMON_MAX_ALLOWED_CONN_COUNT == 1UL,
               "BLE_PEER_MANAGER_COMMON_MAX_ALLOWED_CONN_COUNT must be 1, as only one connection is supported.");
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// DMP Thread stack size in words
#define SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_STACK_SIZE_WORD   (128UL)

// DMP Event flag all mask
#define SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ALL_MSK       (0x00FFFFFFUL)

// DMP Event flag advertising ON mask
#define SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ADV_ON_MSK    (0x00000001UL << 0UL)

// DMP Event flag advertising OFF mask
#define SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ADV_OFF_MSK   (0x00000001UL << 1UL)

// DMP Event flag error mask
#define SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ERROR_MSK     (0x00000001UL << 31UL)
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @brief Task function
 * @details This function is the main task for the DMP advertise controller
 *
 * @param[in] args Arguments
 *****************************************************************************/
static void _adv_controller_task_fnc(void *args);

/**************************************************************************//**
 * @brief Start BLE advertising
 * @details This function starts BLE advertising
 *****************************************************************************/
static void _start_ble_advertising(void);

/**************************************************************************//**
 * @brief Stop BLE advertising
 * @details This function stops BLE advertising
 *****************************************************************************/
static void _stop_ble_advertising(void);
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
// DMP advertise controller task ID
static osThreadId_t _adv_controller_task = NULL;

// DMP advertise controller task attributes
static const osThreadAttr_t _adv_controller_task_attr = {
  .name       = "AdvControllerTask",
  .attr_bits  = osThreadDetached,
  .cb_mem     = NULL,
  .cb_size    = 0UL,
  .stack_mem  = NULL,
  .stack_size = (SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_STACK_SIZE_WORD * sizeof(void *)) & 0xFFFFFFF8U,
  .priority   = osPriorityNormal1,
  .tz_module  = 0UL,
  .reserved   = 0UL
};

// BLE advertising event flags
static osEventFlagsId_t _ble_adv_evt = NULL;

// BLE advertising event flags attributes
static const osEventFlagsAttr_t _ble_adv_evt_attr = {
  .name      = "BleAdvEvtFlags",
  .attr_bits = 0UL,
  .cb_mem    = NULL,
  .cb_size   = 0UL
};

// LED signal for LED0
static sl_wisun_led_signal_t _led0 = {
  .id = SL_WISUN_LED0_ID,
  .high_ms = 500U,
  .low_ms = 500U,
  .period = SL_WISUN_LED_PERIOD_CONTINUOUS_SIGNAL
};
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/* Wi-SUN BLE DMP Advertise Controller module initialiser */
void sl_wisun_ble_dmp_advertise_controller_init(void)
{
  // Create event flags
  _ble_adv_evt = osEventFlagsNew(&_ble_adv_evt_attr);
  EFM_ASSERT(_ble_adv_evt != NULL);

  // Create task
  _adv_controller_task = osThreadNew(_adv_controller_task_fnc, NULL, &_adv_controller_task_attr);
  EFM_ASSERT(_adv_controller_task != NULL);
}

/* Wi-SUN BLE DMP enable advertisement */
void sl_wisun_ble_dmp_advertise_controller_adv_enable(void)
{
  (void) osEventFlagsSet(_ble_adv_evt, SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ADV_ON_MSK);
}

/* Wi-SUN BLE DMP disable advertisement */
void sl_wisun_ble_dmp_advertise_controller_adv_disable(void)
{
  (void) osEventFlagsSet(_ble_adv_evt, SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ADV_OFF_MSK);
}

/**************************************************************************//**
 * Simple Button
 * Button state changed callback
 * @param[in] handle Button event handle
 *****************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  uint32_t flags = 0UL;

  // Button pressed
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    flags = osEventFlagsGet(_ble_adv_evt);
    // Event flag error
    if (flags & SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ERROR_MSK) {
      printf("[Failed: BLE adv evt flags (0x%08lX)]\n", flags);
      return;
    }

    // Toggle advertising state
    if (flags & SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ADV_ON_MSK) {
      sl_wisun_ble_dmp_advertise_controller_adv_disable();
    } else if (flags & SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ADV_OFF_MSK) {
      sl_wisun_ble_dmp_advertise_controller_adv_enable();
    }
  }
}

/**************************************************************************//**
 * BLE Peer manager event handler
 *
 * @param[in] event Event coming from the peer manager
 *****************************************************************************/
void ble_peer_manager_on_event_wisun_advertise_controller(ble_peer_manager_evt_type_t *event)
{
  switch (event->evt_id) {
    // -------------------------------
    // This event indicates that a new connection was opened.
    case BLE_PEER_MANAGER_ON_CONN_OPENED_PERIPHERAL:
      // Disable buttons
      sl_button_disable(SL_SIMPLE_BUTTON_INSTANCE(0));
      sl_button_disable(SL_SIMPLE_BUTTON_INSTANCE(1));

      // Update advertising status
      sl_wisun_ble_dmp_advertise_controller_adv_disable();

      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case BLE_PEER_MANAGER_ON_CONN_CLOSED:
      // Enable buttons
      sl_button_enable(SL_SIMPLE_BUTTON_INSTANCE(0));
      sl_button_enable(SL_SIMPLE_BUTTON_INSTANCE(1));

      break;

    // -------------------------------
    // Default event handler
    default:
      break;
  }
}
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/* Start BLE advertising */
static void _start_ble_advertising(void)
{
  sl_status_t sc = SL_STATUS_FAIL;

  sc = ble_peer_manager_peripheral_start_advertising(SL_BT_INVALID_ADVERTISING_SET_HANDLE);
  if (sc == SL_STATUS_OK) {
    sl_wisun_led_set(&_led0);
    printf("[BLE advertising is started]\n");
  } else {
    printf("[Failed: unable to start BLE advertising: 0x%04lX]\n", sc);
  }
}

/* Stop BLE advertising */
static void _stop_ble_advertising(void)
{
  sl_status_t sc = SL_STATUS_FAIL;

  sc = ble_peer_manager_peripheral_stop_advertising(SL_BT_INVALID_ADVERTISING_SET_HANDLE);
  if (sc == SL_STATUS_OK) {
    sl_wisun_led_clear((sl_wisun_led_id_t)_led0.id);
    printf("[BLE advertising is stopped]\n");
  } else {
    printf("[Failed: unable to stop BLE advertising: 0x%04lX]\n", sc);
  }
}

/* DMP advertise controller task function */
static void _adv_controller_task_fnc(void *args)
{
  static uint32_t prev_flags = 0UL;
  uint32_t flags = 0UL;

  (void) args;

  // Clear all event flags
  (void) osEventFlagsClear(_ble_adv_evt, SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ALL_MSK);

  // Set default adv state (adv disabled)
  (void) osEventFlagsSet(_ble_adv_evt, SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ADV_OFF_MSK);

  SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_THREAD_LOOP {
    prev_flags = osEventFlagsGet(_ble_adv_evt);
    if (prev_flags & SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ERROR_MSK) {
      printf("[Failed: BLE adv evt flags (0x%08lX)]\n", flags);
      continue;
    }

    flags = osEventFlagsWait(_ble_adv_evt,
                             SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ADV_ON_MSK
                             | SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ADV_OFF_MSK,
                             osFlagsWaitAll | osFlagsNoClear,
                             osWaitForever);

    if (flags & SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ERROR_MSK) {
      printf("[Failed: BLE adv evt flags (0x%08lX)]\n", flags);
      continue;
    }

    if (flags & SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ADV_ON_MSK
        && prev_flags & SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ADV_OFF_MSK) {
      _start_ble_advertising();
      (void) osEventFlagsClear(_ble_adv_evt, SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ADV_OFF_MSK);
    } else if (flags & SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ADV_OFF_MSK
               && prev_flags & SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ADV_ON_MSK) {
      _stop_ble_advertising();
      (void) osEventFlagsClear(_ble_adv_evt, SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_EVT_ADV_ON_MSK);
    }
  }
}
