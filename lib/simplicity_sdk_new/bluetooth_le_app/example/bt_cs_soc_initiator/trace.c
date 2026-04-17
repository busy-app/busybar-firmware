/***************************************************************************//**
 * @file
 * @brief Debug trace.
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

#include "sl_component_catalog.h"

#ifdef SL_CATALOG_BGAPI_TRACE_PRESENT
#include "app_config.h"
#include <stdbool.h>
#include "sli_bgapi_trace.h"
#include "rtl_log.h"
#include "app_log.h"
#include "iostream_bgapi_trace.h"

// Trace feedback LED
#ifdef SL_CATALOG_SIMPLE_LED_PRESENT
#include "sl_simple_led.h"
#include "sl_simple_led_instances.h"
#define led_on()  sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(0))
#else // SL_CATALOG_SIMPLE_LED_PRESENT
#define led_on()
#endif // SL_CATALOG_SIMPLE_LED_PRESENT

// Trace enable mechanism
#ifdef SL_CATALOG_SIMPLE_BUTTON_PRESENT
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"
// Check button state.
#define is_trace_requested() \
  (sl_button_get_state(SL_SIMPLE_BUTTON_INSTANCE(0)) == SL_SIMPLE_BUTTON_PRESSED)
#else
// Implement your own solution here
#define is_trace_requested() false
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT

void trace_init(void)
{
  app_log_iostream_set(iostream_bgapi_trace_handle);
#if (ALWAYS_INIT_TRACE == 0)
  if (!is_trace_requested()) {
    return;
  }
#endif // (ALWAYS_INIT_TRACE == 0)
  sli_bgapi_trace_start();
  rtl_log_init();
  led_on();
  sli_bgapi_trace_sync();
}

#else
void trace_init(void)
{
  // Trace is inactive
}
#endif
