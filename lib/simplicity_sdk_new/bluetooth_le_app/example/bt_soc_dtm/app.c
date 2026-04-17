/***************************************************************************//**
 * @file
 * @brief Core application logic.
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
#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "sl_sleeptimer.h"
#include "sl_board_control.h"
#include "sl_bluetooth.h"
#include "app_assert.h"
#include "dtm.h"
#include "app.h"
#include "sl_main_init.h"
#include "sl_component_catalog.h"

enum signal{
  signal_testmode_command_ready = 1,
};

static config_t config = {
  .tx = sl_iostream_putchar,
  .get_ticks = sl_sleeptimer_get_tick_count,
  .ms_to_tick = sl_sleeptimer_ms_to_tick,
  .command_ready_signal = signal_testmode_command_ready,
};

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////

  // Default IOStream instance is used for communication with the DTM tester
  // equipment.
  config.stream = sl_iostream_get_default();

  // Enable VCOM
  sl_board_enable_vcom();

  testmode_init(&config);
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void)
{
  sl_status_t sc;
  size_t bytes_read;
  // This is a blocking call. If you wish to add more application code,
  // - Make sure to use an RTOS
  // - And create a separate task
  app_assert_s(config.rx_len <= sizeof(config.rx_buf));

  sc = sl_iostream_read(config.stream, &config.rx_buf[config.rx_len], sizeof(config.rx_buf) - config.rx_len, &bytes_read);
  if (sc == SL_STATUS_OK) {
    config.rx_len += bytes_read;
  } else if (sc != SL_STATUS_EMPTY) {
    // Unhandled error.
    app_assert_status(sc);
  }

  for (size_t i = 0; i < config.rx_len; i++) {
    testmode_process_command_byte(config.rx_buf[i]);
  }

  config.rx_len = 0;
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the default weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  testmode_on_event(evt);
}

#if !defined(SL_CATALOG_KERNEL_PRESENT) && defined(SL_CATALOG_POWER_MANAGER_PRESENT)
/**************************************************************************//**
 * Checks whether the application is ready to go to sleep.
 *
 * @return whether there are any pending RX bytes to be processed.
 *****************************************************************************/
bool app_is_ok_to_sleep(void)
{
  sl_status_t sc;
  size_t bytes_read;

  if (config.rx_len != 0) {
    // Data pending to be processed, block sleep.
    return false;
  }

  sc = sl_iostream_read(config.stream, config.rx_buf, sizeof(config.rx_buf), &bytes_read);
  if (sc != SL_STATUS_OK) {
    // No more data, ready to sleep.
    return true;
  }

  config.rx_len += bytes_read;

  return false;
}
#endif
