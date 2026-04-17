/***************************************************************************//**
 * @file
 * @brief cli_led_bulb.c
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZW_CLI_COMMON_PRESENT

#include "zaf_event_distributor_soc.h"
#include "CC_MultilevelSwitch_Support.h"
#include "sl_cli.h"
#include "app_log.h"
#include "ev_man.h"
#include "events.h"
#ifdef SL_CATALOG_RGB_LED_PRESENT
#include <sl_simple_rgb_pwm_led.h>
#include <sl_simple_rgb_pwm_led_instances.h>
#endif
#ifdef SL_CATALOG_PWM_PRESENT
#include <sl_pwm_instances.h>
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
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/******************************************************************************
 * CLI - get_rgb_values: Get the RGB LED values
 *****************************************************************************/
void cli_get_rgb_values(__attribute__((unused)) sl_cli_command_arg_t *arguments)
{
  app_log_info("Get RGB LED values\r\n");
#ifdef SL_CATALOG_RGB_LED_PRESENT
  uint16_t color_switch_red_value, color_switch_green_value, color_switch_blue_value;
  sl_led_get_rgb_color(&sl_simple_rgb_pwm_led_rgb_led0, &color_switch_red_value, &color_switch_green_value, &color_switch_blue_value);
  app_log_info("Red: %d, Green: %d, Blue: %d\r\n", color_switch_red_value, color_switch_green_value, color_switch_blue_value);
#endif
#ifdef SL_CATALOG_PWM_PRESENT
  uint8_t color_switch_monochrome_value = sl_pwm_get_duty_cycle(&sl_pwm_led1);
  app_log_info("Monochrome: %d%%\r\n", color_switch_monochrome_value);
#endif
}

/******************************************************************************
 * CLI - toggle_led_bulb: Toggle the LED bulb on/off
 *****************************************************************************/
void cli_toggle_led_bulb(__attribute__((unused)) sl_cli_command_arg_t *arguments)
{
  cc_multilevel_switch_t *switches;
  switches = cc_multilevel_switch_support_config_get_switches();
  cc_multilevel_switch_stop_level_change(&switches[0]);

  if (cc_multilevel_switch_get_current_value(&switches[0]) > 0) {
    cc_multilevel_switch_set(&switches[0], 0, 0); // Turn it off
  } else {
    cc_multilevel_switch_set(&switches[0], 0xFF, 0); // 0xFF for last on value.
  }
}

#endif // SL_CATALOG_ZW_CLI_COMMON_PRESENT
