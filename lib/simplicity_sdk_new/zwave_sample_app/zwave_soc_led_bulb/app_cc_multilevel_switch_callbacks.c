/***************************************************************************//**
 * @file
 * @brief app_cc_multilevel_switch_callbacks.c
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

#include <stdint.h>
#include <events.h>
#include "app_rgb_led.h"
#include <zaf_event_distributor_soc.h>
#include "CC_MultilevelSwitch_Support.h"
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
 * @brief Callback function for handling multilevel switch changes.
 *
 * This function is called when the value of a multilevel switch changes. It updates the
 * corresponding state or hardware based on the new value of the switch.
 *
 * @param p_switch Pointer to the multilevel switch structure that has changed.
 */
void cc_multilevel_switch_support_cb(cc_multilevel_switch_t * p_switch)
{
  uint8_t multilevel_switch_value = cc_multilevel_switch_get_current_value(p_switch);
  app_rgbw_led_set_intensity(multilevel_switch_value);
  app_rgbw_led_update();
}
