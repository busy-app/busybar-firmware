/***************************************************************************//**
 * @file
 * @brief segment lcd tempsensor examples functions
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_segmentlcd.h"
#include "sl_clock_manager.h"
#include "sl_gpio.h"
#include "sl_i2cspm.h"
#include "sl_i2cspm_instances.h"
#include "sl_si70xx.h"
#include "sl_sleeptimer.h"

#define TIMEOUT_MS         5000     // Periodic timer duration in ms
uint32_t rh_data = 0;               // Relative humidity data
int32_t temp_data = 0;              // Temperature data

static sl_sleeptimer_timer_handle_t periodic_timer;

/***************************************************************************//**
 * Periodic timer callback function
 ******************************************************************************/
void on_periodic_timeout(sl_sleeptimer_timer_handle_t *handle,
                         void *data)
{
  // This prevents unused parameter warnings
  (void)&handle;
  (void)&data;

  // Measure the values for relative humidity and temperature
  sl_si70xx_measure_rh_and_temp(sl_i2cspm_sensor,
                                SI7021_ADDR,
                                &rh_data,
                                &temp_data);

  // Read the values for relative humidity and temperature
  sl_si70xx_read_rh_and_temp(sl_i2cspm_sensor,
                             SI7021_ADDR,
                             &rh_data,
                             &temp_data);
  sl_segment_lcd_temp_display(temp_data);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void segment_lcd_app_init(void)
{
  // Initialize the Si7021 sensor, sleeptimer, and Segment LCD display
  sl_si70xx_init(sl_i2cspm_sensor, SI7021_ADDR);
  sl_sleeptimer_init();

  // Configure LCD to use step down mode and disable unused segments
  // Default display value 0
  sl_segment_lcd_init(false);
  LCD->BIASCTRL_SET = LCD_BIASCTRL_VDDXSEL_AVDD;
#if defined(SL_SEGMENT_LCD_MODULE_CL010_1087) || defined(SL_SEGMENT_LCD_MODULE_CE322_1001)
  // Example only used upper numeric segments; disable unused segments
  SL_LCD_SEGMENTS_NUM_DIS();
  // Display 25 degC upon initialization
  sl_segment_lcd_lower_number(25000);
#endif
  // Display Degree C symbol
  sl_segment_lcd_symbol(SL_LCD_SYMBOL_DEGC, 1);
#if defined(SL_SEGMENT_LCD_MODULE_CL010_1087)
  // Display decimal symbol
  sl_segment_lcd_symbol(SL_LCD_SYMBOL_DP5, 1);
#endif
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void segment_lcd_app_process_action(void)
{
  // Periodic timer for updating the temperature value displayed
  sl_sleeptimer_start_periodic_timer_ms(&periodic_timer,
                                        TIMEOUT_MS,
                                        on_periodic_timeout,
                                        NULL,
                                        0,
                                        0);
}
