/***************************************************************************/ /**
 * @file rgb_led.h
 * @brief RGB LED functions
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

#ifndef RGB_LED_H
#define RGB_LED_H

typedef enum {
  RGB_LED_RED,
  RGB_LED_GREEN,
  RGB_LED_BLUE,
  RGB_LED_MAX,
} rgb_led_t;

void rgb_led_set_state(bool red, bool green, bool blue);

void rgb_led_get_state(bool *red, bool *green, bool *blue);

void red_led_toggle(void);

void green_led_toggle(void);

void blue_led_toggle(void);

void white_led_toggle(void);

#endif // RGB_LED_H
