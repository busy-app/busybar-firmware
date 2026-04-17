/***************************************************************************//**
 * @file sl_rail_sdk_simple_assistance.c
 * @brief Simple RAIL Assistance Component
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_rail_sdk_simple_assistance.h"

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
 * An API for toggling the dedicated receive LED on the board.
 *****************************************************************************/
void toggle_receive_led(void)
{
#if defined(SL_CATALOG_LED0_PRESENT)
  sl_led_toggle(&sl_led_led0);
#endif
}

/******************************************************************************
 * An API for setting the dedicated receive LED on the board.
 *****************************************************************************/
void set_receive_led(void)
{
#if defined(SL_CATALOG_LED0_PRESENT)
  sl_led_turn_on(&sl_led_led0);
#endif
}

/******************************************************************************
 * An API for clearing the dedicated receive LED on the board.
 *****************************************************************************/
void clear_receive_led(void)
{
#if defined(SL_CATALOG_LED0_PRESENT)
  sl_led_turn_off(&sl_led_led0);
#endif
}

/******************************************************************************
 * An API for toggling the dedicated send LED on the board.
 *****************************************************************************/
void toggle_send_led(void)
{
#if defined(SL_CATALOG_LED1_PRESENT)
  sl_led_toggle(&sl_led_led1);
#else
#if defined(SL_CATALOG_LED0_PRESENT)
  sl_led_toggle(&sl_led_led0);
#endif
#endif
}

/******************************************************************************
 * An API for setting the dedicated send LED on the board.
 *****************************************************************************/
void set_send_led(void)
{
#if defined(SL_CATALOG_LED1_PRESENT)
  sl_led_turn_on(&sl_led_led1);
#else
#if defined(SL_CATALOG_LED0_PRESENT)
  sl_led_turn_on(&sl_led_led0);
#endif
#endif
}

/******************************************************************************
 * An API for clearing the dedicated send LED on the board.
 *****************************************************************************/
void clear_send_led(void)
{
#if defined(SL_CATALOG_LED1_PRESENT)
  sl_led_turn_off(&sl_led_led1);
#else
#if defined(SL_CATALOG_LED0_PRESENT)
  sl_led_turn_off(&sl_led_led0);
#endif
#endif
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
