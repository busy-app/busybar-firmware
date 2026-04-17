/***************************************************************************//**
 * @file
 * @brief app_rgb_led.c
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <string.h>
#include "app_rgb_led.h"
#include "CC_MultilevelSwitch_Support.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#include "assert.h"
#include "zpal_log.h"

#ifdef SL_CATALOG_RGB_LED_PRESENT
#include "sl_simple_rgb_pwm_led.h"
#include "sl_simple_rgb_pwm_led_instances.h"
#endif
#ifdef SL_CATALOG_PWM_PRESENT
#include "sl_pwm.h"
#include "sl_pwm_instances.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
static rgb_led_setting rgb_led_attributes;

// -----------------------------------------------------------------------------
//                          Public Functions Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/**
 * @brief Gets the color of the RGB LED.
 *
 * This function returns the color of the RGB LED set by app_rgbw_led_set_color.
 *
 * @param color Pointer to an `rgb_led_color` structure where the latest set values
 * will be written to.
 */
void app_rgbw_led_get_color(rgb_led_color* colorOut)
{
  assert(colorOut);
  memcpy(colorOut, &rgb_led_attributes.color, sizeof(rgb_led_color));
}

/**
 * @brief Sets the color of the RGB LED.
 *
 * This function sets the color of the RGB LED by updating the internal color attributes.
 *
 * @param color Pointer to an `rgb_led_color` structure containing the new color values.
 */
void app_rgbw_led_set_color(rgb_led_color const* color)
{
  if (color) {
    memcpy(&rgb_led_attributes.color, color, sizeof(rgb_led_color));
  }
}

/**
 * @brief Sets the intensity of the RGB LED.
 *
 * This function sets the intensity of the RGB LED by updating the internal intensity attribute.
 *
 * @param intensity The new intensity value to set.
 */
void app_rgbw_led_set_intensity(uint8_t intensity)
{
  rgb_led_attributes.intensity = intensity;
}

/**
 * @brief Updates the RGB LED with the current color and intensity settings.
 *
 * This function applies the current color and intensity settings to the RGB LED.
 * It calculates the appropriate PWM values and sets the LED accordingly.
 */
void app_rgbw_led_update(void)
{
  uint8_t multilevel_switch_max = cc_multilevel_switch_get_max_value();

  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "%s Setting RGB=(%u,%u,%u)\n", __func__,
                 (rgb_led_attributes.color.red * rgb_led_attributes.intensity) / multilevel_switch_max,
                 (rgb_led_attributes.color.green * rgb_led_attributes.intensity) / multilevel_switch_max,
                 (rgb_led_attributes.color.blue * rgb_led_attributes.intensity) / multilevel_switch_max);

#ifdef SL_CATALOG_RGB_LED_PRESENT
  sl_led_set_rgb_color(&sl_simple_rgb_pwm_led_rgb_led0,
                       (uint16_t)((rgb_led_attributes.color.red * rgb_led_attributes.intensity) / multilevel_switch_max),
                       (uint16_t)((rgb_led_attributes.color.green * rgb_led_attributes.intensity) / multilevel_switch_max),
                       (uint16_t)((rgb_led_attributes.color.blue * rgb_led_attributes.intensity) / multilevel_switch_max));
#endif

#ifdef SL_CATALOG_PWM_PRESENT
  static bool pwm_led_initialized = false;
  if (!pwm_led_initialized) {
    sl_pwm_start(&sl_pwm_led1);
    pwm_led_initialized = true;
  }
  uint16_t sum_colors = ((rgb_led_attributes.color.red * rgb_led_attributes.intensity) / multilevel_switch_max)
                        + ((rgb_led_attributes.color.green * rgb_led_attributes.intensity) / multilevel_switch_max)
                        + ((rgb_led_attributes.color.blue * rgb_led_attributes.intensity) / multilevel_switch_max);
  // The PWM duty cycle is a percentage in the range of 0-100
  uint16_t monochrome_value = (100 * sum_colors) / (3 * 255);
  sl_pwm_set_duty_cycle(&sl_pwm_led1, (uint8_t)monochrome_value);
#endif
}
