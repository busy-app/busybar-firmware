/***************************************************************************//**
 * @file
 * @brief Main Kernel Initialization.
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
#include "sl_event_handler.h"
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "sl_assert.h"
#include "sl_main_init.h"
#include "sl_main_start_task_config.h"

#include <stdbool.h>

osThreadId_t start_task_id = { 0U };
osThreadAttr_t * start_task_attribute = NULL;

#if defined(__GNUC__)
extern int main();
extern int __real_main();
#define ORIGINAL_MAIN         __real_main
#elif defined(__IAR_SYSTEMS_ICC__)
extern int main();
extern int $Super$$main();
#define ORIGINAL_MAIN         $Super$$main
#endif

/******************************************************************************
 * @brief Start task process function.
 *
 * @details This function handles the start task process, calling the original
 *          main function and terminating the task afterwards.
 *****************************************************************************/
static void start_task_handler(void *argument)
{
  (void)argument;

  ORIGINAL_MAIN();  // calls the original main() function within the start task.

  // Terminate the start task. Function should not return.
  osThreadTerminate(start_task_id);
}

/******************************************************************************
 * @brief Initialize the start task.
 *
 * @details This function initializes the start task with the highest priority.
 *****************************************************************************/
void sli_main_kernel_start_task_initialize(void)
{
  const osThreadAttr_t thread_attribute = {
    .name = "",
    .attr_bits = 0U,
    .cb_mem = NULL,
    .cb_size = 0U,
    .stack_mem = NULL,
    .stack_size = SL_MAIN_START_TASK_STACK_SIZE_BYTES,
    .priority = osPriorityRealtime7,
    .tz_module = 0U,
    .reserved = 0U,
  };

  start_task_id = osThreadNew(&start_task_handler, NULL, &thread_attribute);
  EFM_ASSERT(start_task_id != NULL);
}

/******************************************************************************
 * @brief Start the selected kernel.
 *****************************************************************************/
void sl_main_kernel_start(void)
{
  sl_kernel_start();
}

/******************************************************************************
 * @brief User-defined function to determine if the start task should continue.
 *
 * @note By default the start task should not continue, but this function can be
 *       re-implemented to force the start task to continue running.
 *****************************************************************************/
__WEAK bool sl_main_start_task_should_continue(void)
{
  return false;
}
