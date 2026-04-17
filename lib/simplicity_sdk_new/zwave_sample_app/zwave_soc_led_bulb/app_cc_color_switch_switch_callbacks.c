/***************************************************************************//**
 * @file
 * @brief app_cc_color_switch_switch_callbacks.c
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
#include <events.h>
#include <zaf_event_distributor_soc.h>
#include "CC_ColorSwitch.h"
#include "cc_color_switch_config_api.h"
#include "app_rgb_led.h"

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

// -----------------------------------------------------------------------------
//                          Public Functions Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/**
 * @brief Callback function for handling color switch changes.
 *
 * This function is called when a color component changes. It updates the RGB LED color
 * based on the current value of the color component.
 *
 * @param colorComponent Pointer to the color component that has changed.
 */
void cc_color_switch_cb(s_colorComponent * colorComponent)
{
  rgb_led_color color;
  app_rgbw_led_get_color(&color);

  switch (colorComponent->colorId) {
    case ECOLORCOMPONENT_RED:
      color.red = ZAF_Actuator_GetCurrentValue(&colorComponent->obj);
      break;
    case ECOLORCOMPONENT_GREEN:
      color.green = ZAF_Actuator_GetCurrentValue(&colorComponent->obj);
      break;
    case ECOLORCOMPONENT_BLUE:
      color.blue = ZAF_Actuator_GetCurrentValue(&colorComponent->obj);
      break;
    default:
      break;
  }

  app_rgbw_led_set_color(&color);
  app_rgbw_led_update();
}
