/***************************************************************************//**
 * @file
 * @brief app_init.c
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_component_catalog.h"
#include "sl_rail_sdk_service_phy_config.h"
#include "sl_common.h"
#include "sl_sleeptimer.h"
#include "app_init.h"
#include "app_process.h"
#include "app_menu.h"
#include "app_measurement.h"
#include "sl_code_classification.h"

#if defined(SL_CATALOG_GLIB_PRESENT)
#include "app_graphics.h"
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "app_task_init.h"
#endif

#if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
#include "rail_config.h"
#endif

#if defined(SL_CATALOG_APP_LOG_PRESENT)
#include "app_log.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
/// Timer expiration callback for the delay function.
static void init_screen_timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
/// Timer for the delay of the showing init screen
static sl_sleeptimer_timer_handle_t init_screen_timer;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/******************************************************************************
 * Print sample app name
 *****************************************************************************/
SL_WEAK void print_sample_app_name(const char* app_name)
{
#ifdef SL_CATALOG_APP_LOG_PRESENT
  app_log_info("%s\n", app_name);
#endif
  // Avoid unused parameter warning
  (void)app_name;
}

/*******************************************************************************
 * The function is used for application initialization.
 * @return sl_rail_handle_t Null pointer to keep style with other sample apps
 ******************************************************************************/
void rail_app_init(void)
{
  sl_status_t sleep_timer_status = 0;
  uint16_t start;
  uint16_t end;
  uint16_t channel;

#if defined(SL_CATALOG_GLIB_PRESENT)
  graphics_init();
#endif

  range_test_settings.service_phy = SL_SERVICE_PHY_NUM;
  get_rail_channel_range(&start, &end, range_test_settings.service_phy);
  channel = start + SL_SERVICE_CHANNEL_NUM_OFFSET;
  if (start <= channel && channel <= end) {  // if the requested service channel is in the valid range
    range_test_settings.service_channel = channel;
  } else {
    range_test_settings.service_channel = start;
  }

  init_range_test_phys();
  menu_init();

  set_next_state(INFO_SCREEN);
  request_refresh_screen();

  sleep_timer_status = sl_sleeptimer_start_timer(&init_screen_timer,
                                                 sl_sleeptimer_ms_to_tick(3000), init_screen_timer_callback,
                                                 NULL, 0, 0);

  if (sleep_timer_status != 0) {
    app_log_error("Sleeptimer start failed with code %lu",
                  sleep_timer_status);
  }

  // CLI info message
#ifdef SL_CATALOG_RANGE_TEST_STD_COMPONENT_PRESENT
#ifdef SL_CATALOG_RANGE_TEST_DMP_COMPONENT_PRESENT
  print_sample_app_name("Range test STD DMP");
#else
  print_sample_app_name("Range test STD");
#endif
#else
#ifdef SL_CATALOG_RANGE_TEST_DMP_COMPONENT_PRESENT
  print_sample_app_name("Range test DMP");
#else
  print_sample_app_name("Range test");
#endif
#endif
}

void app_init(void)
{
#if !defined(SL_CATALOG_KERNEL_PRESENT)
  rail_app_init();
#else
  app_task_init();
#endif
}

/*******************************************************************************
 * The function is used for stopping the timer in the init block.
 ******************************************************************************/
void end_init_timer(void)
{
  bool is_running = false;
  sl_status_t sleep_timer_status = 0;
  sleep_timer_status = sl_sleeptimer_is_timer_running(&init_screen_timer,
                                                      &is_running);
  if (sleep_timer_status != 0) {
    app_log_error("Sleeptimer state read failed with code %lu", sleep_timer_status);
  }
  if (is_running) {
    sleep_timer_status = sl_sleeptimer_stop_timer(&init_screen_timer);
    if (sleep_timer_status != 0) {
      app_log_error("Sleeptimer stop failed with code %lu", sleep_timer_status);
    }
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/*******************************************************************************
 * Timer expiration callback for the delay function.
 *
 * @param[in] handle Pointer to handle to timer.
 * @param[in] data Pointer to delay flag.
 ******************************************************************************/
SL_CODE_RAM static void init_screen_timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)data;  // Unused parameter.
  (void)handle;  // Unused parameter.

  set_next_state(MENU_SCREEN);
  request_refresh_screen();
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}
