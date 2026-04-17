/***************************************************************************//**
 * @file sl_connect_rail_dmp_button.c
 * @brief Source file for the button handling in the Silicon Labs Connect
 *        RAIL DMP application.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_component_catalog.h"
#include "sl_connect_rail_dmp.h"
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
extern volatile bool tx_requested;
extern volatile bool start_rx;
extern volatile bool stop_rx;
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/**
 * @brief Callback function that handles button state changes.
 *
 * This function is called whenever a button state changes. It checks the state
 * of the button and performs actions based on which button was pressed or released.
 *
 * @param handle Pointer to the button handle that triggered the callback.
 *
 * When the button is pressed:
 * - If button 0 is pressed, it sets `tx_requested`.
 * - If button 1 is pressed, it sets `start_rx`.
 *
 * When the button is released:
 * - If button 1 is released, it sets `stop_rx`.
 */
void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (&sl_button_btn0 == handle) {
      tx_requested = true;
      post_os_flag();
    }
#ifdef SL_CATALOG_SIMPLE_BUTTON_BTN1_PRESENT
    if (&sl_button_btn1 == handle) {
      start_rx = true;
      post_os_flag();
    }
#endif
  }
#ifdef SL_CATALOG_SIMPLE_BUTTON_BTN1_PRESENT
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED && &sl_button_btn1 == handle) {
    stop_rx = true;
    post_os_flag();
  }
#endif
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
