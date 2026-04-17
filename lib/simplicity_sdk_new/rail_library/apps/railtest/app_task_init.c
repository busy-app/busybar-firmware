/***************************************************************************//**
 * @file
 * @brief app_task_init.c
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
#include <stddef.h>
#include <assert.h>
#include "app_task_init.h"
#include "app_common.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define RAILTEST_STACK_SIZE 1024U
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static void railtest_task(void *p_arg);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
osMutexId_t railapp_mtx = NULL;

//  App framework mutex attribute
osMutexAttr_t railapp_mtx_attr = {
  .name      = "Railtest Mutex",
  .attr_bits = osMutexRecursive,
  .cb_mem    = NULL,
  .cb_size   = 0
};
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
__ALIGNED(8) static uint8_t railtest_stack[RAILTEST_STACK_SIZE & 0xFFFFFFF8u];
__ALIGNED(4) static uint8_t railtest_threadcb[osThreadCbSize];

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
extern void app_init(void);

void app_task_init(void)
{
  railapp_mtx = osMutexNew(&railapp_mtx_attr);
  assert(railapp_mtx != NULL);

  osThreadAttr_t attr = {
    .name       = "Railtest task",
    .priority   = osPriorityNormal,
    .attr_bits  = osThreadDetached,
    .cb_mem     = railtest_threadcb,
    .cb_size    = osThreadCbSize,
    .stack_mem  = railtest_stack,
    .stack_size = sizeof(railtest_stack)
  };

  osThreadNew(railtest_task, NULL, &attr);    // Create application main thread
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/*******************************************************************************
 * The function is the task that will be run by the kernel.
 *
 * @param None
 * @returns None
 ******************************************************************************/
static void railtest_task(void *p_arg)
{
  (void) (p_arg);

  app_init();
  while (1) {
    sl_rail_test_internal_app_process_action();
  }
}
