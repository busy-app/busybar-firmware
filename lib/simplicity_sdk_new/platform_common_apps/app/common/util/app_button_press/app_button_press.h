/***************************************************************************//**
 * @file
 * @brief Button press header file
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

#ifndef APP_BUTTON_PRESS_H
#define APP_BUTTON_PRESS_H

#include "sl_power_manager.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup ButtonPress
 * @{
 ******************************************************************************/

/*******************************************************************************
 * Public definitions
 ******************************************************************************/
#define APP_BUTTON_PRESS_DURATION_SHORT        0
#define APP_BUTTON_PRESS_DURATION_MEDIUM       1
#define APP_BUTTON_PRESS_DURATION_LONG         2
#define APP_BUTTON_PRESS_DURATION_VERYLONG     3
#define APP_BUTTON_PRESS_NONE                  4
#define APP_BUTTON_PRESS_PRESSED_DOWN          5

/***************************************************************************//**
 * Enable button press callbacks.
 * @return status of the operation
 ******************************************************************************/
sl_status_t app_button_press_enable(void);

/***************************************************************************//**
 * Disable button press callbacks.
 * @return status of the operation
 ******************************************************************************/
sl_status_t app_button_press_disable(void);

/***************************************************************************//**
 * Step function for button press
 ******************************************************************************/
void app_button_press_step(void);

/***************************************************************************//**
 * Callback for button press
 * @param button button ID. From 0 up to SL_SIMPLE_BUTTON_COUNT-1
 * @param duration duration of button press. Either of
 *                   - APP_BUTTON_PRESS_NONE
 *                   - APP_BUTTON_PRESS_DURATION_SHORT
 *                   - APP_BUTTON_PRESS_DURATION_MEDIUM
 *                   - APP_BUTTON_PRESS_DURATION_LONG
 *                   - APP_BUTTON_PRESS_DURATION_VERYLONG
 *                   - APP_BUTTON_PRESS_PRESSED_DOWN
 ******************************************************************************/
void app_button_press_cb(uint8_t button, uint8_t duration);

/***************************************************************************//**
 * Callback for error
 * @param status status code
 ******************************************************************************/
void app_button_press_error(sl_status_t status);

/***************************************************************************//**
 * Check if the device was woken up from EM4 by a button press
 ******************************************************************************/
void app_button_press_check_wakeup_cause(void);

/** @} (end addtogroup ButtonPress) */

#ifdef __cplusplus
};
#endif

#endif // APP_BUTTON_PRESS_H
