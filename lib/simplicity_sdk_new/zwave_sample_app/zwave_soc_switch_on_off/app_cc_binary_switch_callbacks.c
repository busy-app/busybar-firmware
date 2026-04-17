/***************************************************************************//**
 * @file
 * @brief app_cc_binary_switch_callbacks.c
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
#include "app_button_handler.h"
#include "CC_BinarySwitch.h"
#include "sl_simple_led.h"
#include "sl_simple_led_instances.h"

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
 * @brief Callback function for handling binary switch changes.
 *
 * This function is called when the state of a binary switch changes. It updates the
 * corresponding state or hardware based on the new value of the switch.
 *
 * @param p_switch Pointer to the binary switch structure that has changed.
 */
void cc_binary_switch_handler(cc_binary_switch_t * p_switch)
{
  uint8_t value = ZAF_Actuator_GetCurrentValue(&p_switch->actuator);

  if ( value > 0) {
    sl_simple_led_turn_on(sl_led_led1.context);
  } else {
    sl_simple_led_turn_off(sl_led_led1.context);
  }
}
