/***************************************************************************//**
 * @file
 * @brief cli_power_strip.c
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
#include "CC_MultilevelSwitch_Support.h"
#include "CC_BinarySwitch.h"
#ifdef SL_CATALOG_RGB_LED_PRESENT
#include "sl_simple_rgb_pwm_led.h"
#include "sl_simple_rgb_pwm_led_instances.h"
#endif
#ifdef SL_CATALOG_PWM_PRESENT
#include "sl_pwm_instances.h"
#endif

#define DIMMING_TRANSITION_PERIOD_SEC 1 //Time [s] required for transition between 2 values, 0 means instant transition
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
 * CLI - toggle_endpoint: Toggle the endpoint 1 or 2
 *****************************************************************************/
void cli_toggle_endpoint(sl_cli_command_arg_t *arguments)
{
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  if (endpoint == 1) {
    app_log_info("Toggle endpoint 1\r\n");
    zaf_event_distributor_enqueue_app_event(EVENT_APP_OUTLET1_TOGGLE);
  } else if (endpoint == 2) {
    app_log_info("Toggle endpoint 2\r\n");
    zaf_event_distributor_enqueue_app_event(EVENT_APP_OUTLET2_DIMMER_SHORT_PRESS);
  } else {
    app_log_error("Invalid endpoint\r\n");
  }
}

/******************************************************************************
 * CLI - dim_endpoint: Dim the endpoint 2
 *****************************************************************************/
void cli_dim_endpoint(sl_cli_command_arg_t *arguments)
{
  uint8_t dimming_rate_level = sl_cli_get_argument_uint8(arguments, 0);
  if (dimming_rate_level > CC_MULTILEVEL_SWITCH_ACTUATOR_MAX_VALUE) {
    app_log_error("Invalid dimming rate\r\n");
    return;
  }
  app_log_info("Dimming endpoint 2 to %d%%\r\n", dimming_rate_level);
  cc_multilevel_switch_t *switches;
  switches = cc_multilevel_switch_support_config_get_switches();
  cc_multilevel_switch_set_level(&switches[0], dimming_rate_level, DIMMING_TRANSITION_PERIOD_SEC);
}

/******************************************************************************
 * CLI - cli_toggle_notification_sending: Toggle the notification sending
 *****************************************************************************/
void cli_toggle_notification_sending(sl_cli_command_arg_t *arguments)
{
  static bool notification_sending = false;
  (void) arguments;

  if (notification_sending) {
    app_log_info("Stop sending Overload detected notification\r\n");
    notification_sending = false;
  } else {
    app_log_info("Start sending Overload detected notification\r\n");
    notification_sending = true;
  }
  zaf_event_distributor_enqueue_app_event(EVENT_APP_NOTIFICATION_TOGGLE);
}

/******************************************************************************
 * CLI - get_led_state: Get the state of the LED1
 *****************************************************************************/
void cli_get_led_state(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  app_log_info("Get the state of the LED1\r\n");
  cc_binary_switch_t * p_switches = cc_binary_switch_get_config();
  char* state = cc_binary_switch_get_current_value(&p_switches[0]) > 0 ? "on" : "off";
  app_log_info("LED1 state: %s\r\n", state);
}

/******************************************************************************
 * CLI - get_rgb_values: Get the RGB values of the RGB LED
 *****************************************************************************/
void cli_get_rgb_values(__attribute__((unused)) sl_cli_command_arg_t *arguments)
{
  app_log_info("Get rgb LED values\r\n");
#ifdef SL_CATALOG_RGB_LED_PRESENT
  uint16_t color_switch_red_value, color_switch_green_value, color_switch_blue_value;
  sl_led_get_rgb_color(&sl_simple_rgb_pwm_led_rgb_led0, &color_switch_red_value, &color_switch_green_value, &color_switch_blue_value);
  app_log_info("Red: %d, Green: %d, Blue: %d\r\n", color_switch_red_value, color_switch_green_value, color_switch_blue_value);
#endif

#ifdef SL_CATALOG_PWM_PRESENT
  uint8_t monochrome_percent =
    100 * cc_multilevel_switch_get_current_value(&cc_multilevel_switch_support_config_get_switches()[0])
    / cc_multilevel_switch_get_max_value();
  app_log_info("Monochrome: %d%%\r\n", monochrome_percent);
#endif
}

void cli_log_cc_binary_switch_events(const uint8_t event, const void * const data)
{
  cc_binary_switch_t * p_switch = (cc_binary_switch_t *)data;
  if (p_switch->endpoint != cc_binary_switch_get_config()[0].endpoint) {
    return;
  }
  const char message_common[] = "Binary switch";
  switch (event) {
    case CC_BINARY_SWITCH_EVENT_START_LEVEL_CHANGE:
      if (p_switch->actuator.valueCurrent != p_switch->actuator.valueTarget) {
        app_log_info("%s is turning %s\r\n", message_common,
                     p_switch->actuator.valueTarget == 0 ? "off" : "on");
      }
      break;
    case CC_BINARY_SWITCH_EVENT_REACHED_FINAL_VALUE:
      app_log_info("%s turned %s\r\n", message_common,
                   p_switch->actuator.valueCurrent == 0 ? "off" : "on");
      break;
    default:
      break;
  }
}

ZAF_EVENT_DISTRIBUTOR_REGISTER_CC_EVENT_HANDLER(
  COMMAND_CLASS_SWITCH_BINARY, cli_log_cc_binary_switch_events);

void cli_log_cc_multilevel_switch_events(const uint8_t event, const void * const data)
{
  cc_multilevel_switch_t * p_switch = (cc_multilevel_switch_t *)data;
  if (p_switch->endpoint != cc_multilevel_switch_support_config_get_switches()[0].endpoint) {
    return;
  }
  const char message_common[] = "Multilevel switch";
  switch (event) {
    case CC_MULTILEVEL_SWITCH_EVENT_START_LEVEL_CHANGE:
      app_log_info("%s level change started: %d -> %d\r\n", message_common,
                   p_switch->actuator.valueCurrent / 10,
                   p_switch->actuator.valueTarget / 10);
      break;
    case CC_MULTILEVEL_SWITCH_EVENT_STOP_LEVEL_CHANGE:
      app_log_info("%s level change stopped at %d\r\n", message_common,
                   p_switch->actuator.valueCurrent / 10);
      break;
    case CC_MULTILEVEL_SWITCH_EVENT_REACHED_FINAL_VALUE:
      if (p_switch->actuator.valueCurrent % 10 == 0) { // Avoid logging intermediate states
        app_log_info("%s level is now at %d\r\n", message_common,
                     p_switch->actuator.valueCurrent / 10);
      }
      break;
    default:
      break;
  }
}

ZAF_EVENT_DISTRIBUTOR_REGISTER_CC_EVENT_HANDLER(
  COMMAND_CLASS_SWITCH_MULTILEVEL, cli_log_cc_multilevel_switch_events);

#endif // SL_CATALOG_ZW_CLI_COMMON_PRESENT
