/*******************************************************************************
 * @file  rgb_led.c
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
/**============================================================================
 * @section Description
 * This files contains example code to demonstrate the GPIO/LED toggle functionality.
 ============================================================================**/

// Include Files
#include "rsi_ccp_user_config.h"
#include "sl_si91x_rgb_led_instances.h"
#include "rsi_debug.h"
#include "sl_si91x_rgb_led.h"
#include "sl_si91x_clock_manager.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/* RGB LED instances */
#ifndef RGB_LED
#define RGB_LED led_led0
#endif

/* Default RGB colors */
#define COLOR_COUNT 10
const uint32_t RGB_COLORS[COLOR_COUNT] = {
  0xFF0000, // Red
  0x00FF00, // Green
  0x0000FF, // Blue
  0xFFFF00, // Yellow
  0xFF00FF, // Magenta
  0x00FFFF, // Cyan
  0xFF8000, // Orange
  0x8000FF, // Purple
  0x00FF80, // Teal
  0xFF0080  // Pink
};

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************/ /**
 * Initialize RGB LED example.
 ******************************************************************************/
void rgb_led_init(void)
{
  // Switch on the LED
  sl_si91x_simple_rgb_led_on(&RGB_LED);
}

/***************************************************************************/ /**
 * RGB LED ticking function.
 ******************************************************************************/
void rgb_led_process_action(void)
{
  for (int i = 0; i < COLOR_COUNT; i++) {
    // Set the current color from the array
    sl_si91x_simple_rgb_led_set_colour(&RGB_LED, RGB_COLORS[i]);

    // delay
    sl_si91x_delay_ms(100);
  }
  // Switch off the LED
  sl_si91x_simple_rgb_led_off(&RGB_LED);
}
