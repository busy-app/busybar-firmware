/*******************************************************************************
 * @file  rgb_led.c
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_sleeptimer.h"
#include "sl_si91x_rgb_led_instances.h"
#include "rsi_debug.h"
#include "sl_si91x_rgb_led.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
/* RGB LED instances*/
#ifndef RED
#define RED led_red
#endif

#ifndef GREEN
#define GREEN led_green
#endif

#ifndef BLUE
#define BLUE led_blue
#endif

static bool red_state   = false;
static bool green_state = false;
static bool blue_state  = false;

void rgb_led_set_state(bool red, bool green, bool blue)
{
  if (red == true) {
    sl_si91x_rgb_led_on(&RED);
  } else {
    sl_si91x_rgb_led_off(&RED);
  }
  red_state = red;

  if (green == true) {
    sl_si91x_rgb_led_on(&GREEN);
  } else {
    sl_si91x_rgb_led_off(&GREEN);
  }
  blue_state = blue;

  if (blue == true) {
    sl_si91x_rgb_led_on(&BLUE);
  } else {
    sl_si91x_rgb_led_off(&BLUE);
  }
  green_state = green;
}

void rgb_led_get_state(bool *red, bool *green, bool *blue)
{
  *red   = red_state;
  *green = green_state;
  *blue  = blue_state;
}

void red_led_toggle(void)
{
  if (red_state == false) {
    sl_si91x_rgb_led_on(&RED);
    red_state = true;
  } else {
    sl_si91x_rgb_led_off(&RED);
    red_state = false;
  }
}

void green_led_toggle(void)
{
  if (green_state == false) {
    sl_si91x_rgb_led_on(&GREEN);
    green_state = true;
  } else {
    sl_si91x_rgb_led_off(&GREEN);
    green_state = false;
  }
}

void blue_led_toggle(void)
{
  if (blue_state == false) {
    sl_si91x_rgb_led_on(&BLUE);
    blue_state = true;
  } else {
    sl_si91x_rgb_led_off(&BLUE);
    blue_state = false;
  }
}

void white_led_toggle(void)
{
  if (red_state == false) {
    sl_si91x_rgb_led_on(&RED);
    red_state = true;
    sl_si91x_rgb_led_on(&GREEN);
    green_state = true;
    sl_si91x_rgb_led_on(&BLUE);
    blue_state = true;
  } else {
    sl_si91x_rgb_led_off(&RED);
    red_state = false;
    sl_si91x_rgb_led_off(&GREEN);
    green_state = false;
    sl_si91x_rgb_led_off(&BLUE);
    blue_state = false;
  }
}
