/***************************************************************************//**
 * @file
 * @brief Baremetal compatibility layer.
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
#include <stdint.h>
#include <stdbool.h>
#include "sl_core.h"
#include "sl_main_init.h"
#include "app.h"

// "Semaphore" indicating that it is required to execute application process action.
static uint16_t proceed_request;

// Application Runtime Init.
void app_init_bt(void)
{
  proceed_request = 0;
}

// Proceed with execution.
void app_proceed(void)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  if (proceed_request < UINT16_MAX) {
    proceed_request++;
  }
  CORE_EXIT_CRITICAL();
}

// Check if it is required to process with execution.
bool app_is_process_required(void)
{
  bool ret = false;
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  if (proceed_request > 0) {
    proceed_request--;
    ret = true;
  }
  CORE_EXIT_CRITICAL();
  return ret;
}

// Acquire access to protected variables
bool app_mutex_acquire(void)
{
  // There are no tasks to protect shared resources from.
  return true;
}

// Finish access to protected variables
void app_mutex_release(void)
{
  // There are no tasks to protect shared resources from.
}
