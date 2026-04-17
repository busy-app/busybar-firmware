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

#include <events.h>
#include <zaf_event_distributor_soc.h>
#include "CC_MultilevelSwitch_Support.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

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
  __attribute__((unused)) const uint8_t level = ZAF_Actuator_GetCurrentValue(&p_switch->actuator);
#ifdef SL_CATALOG_RGB_LED_PRESENT
  sl_led_set_rgb_color(&sl_simple_rgb_pwm_led_rgb_led0, (uint16_t)level, (uint16_t)level, (uint16_t)level);
#endif

#ifdef SL_CATALOG_PWM_PRESENT
  static bool pwm_led_initialized = false;
  if (!pwm_led_initialized) {
    sl_pwm_start(&sl_pwm_led1);
    pwm_led_initialized = true;
  }
  sl_pwm_set_duty_cycle(&sl_pwm_led1, (uint8_t)level);
#endif
}
