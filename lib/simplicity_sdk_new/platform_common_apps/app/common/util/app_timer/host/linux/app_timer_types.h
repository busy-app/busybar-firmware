/***************************************************************************//**
 * @file
 * @brief Timer type definitions for Linux platform
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

#ifndef APP_TIMER_LINUX_H
#define APP_TIMER_LINUX_H

#include <signal.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct timer_data_s {
  timer_t                timer_id;       // Timer ID generated at timer start
  struct sigevent        sig_evt;        // Signal used for timers
  struct itimerspec      handle_timeout; // Timeout in required format
  bool                   triggered;      // Indication of expired timer
} app_timer_handle_t;

typedef struct app_timer app_timer_t;

/***************************************************************************//**
 * Expected prototype of the user's callback function which is called when a
 * timer expires.
 *
 * @param timer Pointer to the timer handle.
 * @param data An extra parameter for the user application.
 ******************************************************************************/
typedef void (*app_timer_callback_t)(app_timer_t *timer, void *data);

/// Structure to populate timers
struct app_timer {
  app_timer_handle_t app_timer_handle;  // Platform specific parameters
  app_timer_callback_t callback;        // Pointer to user defined callback
  void *callback_data;                  // Pointer to callback data
  uint32_t timeout;                     // Time elapsed before timer trigger
  bool periodic;                        // Periodicity indication
  app_timer_t *next;                    // Pointer to next timer if exist
};

#endif // APP_TIMER_LINUX_H
