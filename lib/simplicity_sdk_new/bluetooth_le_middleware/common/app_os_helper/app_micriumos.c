/***************************************************************************//**
 * @file
 * @brief Micirum OS helper functions for the application task.
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
#include "os.h"
#include "sl_memory_manager.h"
#include "sl_main_init.h"
#include "app_assert.h"
#include "app.h"

#define APP_TASK_NAME          "app_task"
#define APP_TASK_STACK_SIZE    1024u
#define APP_TASK_PRIO          31u
#define APP_MUTEX_NAME         "app_mutex"
#define APP_MUTEX_WAIT         100 // Timeout to wait for mutex in ticks

// Application task.
static void app_task(void *p_arg);
// Task stack
static CPU_STK *app_task_stack;
// Task handle
static OS_TCB  app_task_handle;
// Semaphore handle
static OS_SEM  app_semaphore_handle;
// Mutex handle
static OS_MUTEX *app_mutex_handle;

// Application Runtime Init.
void app_init_bt(void)
{
  RTOS_ERR err;
  // Allocate stack for the task
  size_t stack_size = APP_TASK_STACK_SIZE;
  stack_size -= (stack_size % CPU_CFG_STK_ALIGN_BYTES);
  app_task_stack = (CPU_STK *)sl_malloc(stack_size);
  app_assert(app_task_stack != NULL,
             "Application task stack allocation failed.");
  // Create the task for sl_app_process_action
  OSTaskCreate(&app_task_handle,
               APP_TASK_NAME,
               app_task,
               0u,
               APP_TASK_PRIO,
               &app_task_stack[0u],
               0u,
               stack_size / sizeof(CPU_STK),
               0u,
               0u,
               0u,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &err);
  app_assert(err.Code == RTOS_ERR_NONE,
             "Application task creation failed.");
  // Create the semaphore
  OSSemCreate(&app_semaphore_handle, "Application semaphore", 0, &err);
  app_assert(err.Code == RTOS_ERR_NONE,
             "Application semaphore creation failed.");
  // Create the mutex
  app_mutex_handle = (OS_MUTEX *)sl_malloc(sizeof(OS_MUTEX));
  app_assert(app_mutex_handle != NULL,
             "Application mutex creation failed.");
  OSMutexCreate(app_mutex_handle, APP_MUTEX_NAME, &err);
  app_assert(err.Code == RTOS_ERR_NONE,
             "Application mutex creation failed.");
}

/******************************************************************************
 * Application task.
 *****************************************************************************/
static void app_task(void *p_arg)
{
  (void)p_arg;
  while (1) {
    app_process_action();
  }
}

// Proceed with execution.
void app_proceed(void)
{
  RTOS_ERR err;
  OSSemPost(&app_semaphore_handle, OS_OPT_POST_1, &err);
  app_assert(err.Code == RTOS_ERR_NONE, "Semaphore post failed!");
}

// Check if it is required to process with execution.
bool app_is_process_required(void)
{
  RTOS_ERR err;
  OSSemPend(&app_semaphore_handle,
            (OS_TICK)0,
            OS_OPT_PEND_BLOCKING,
            DEF_NULL,
            &err);
  return (err.Code == RTOS_ERR_NONE);
}

// Acquire access to protected variables
bool app_mutex_acquire(void)
{
  RTOS_ERR err;
  OSMutexPend(app_mutex_handle,
              (OS_TICK)APP_MUTEX_WAIT,
              OS_OPT_PEND_BLOCKING,
              DEF_NULL,
              &err);
  return err.Code == RTOS_ERR_NONE || err.Code == RTOS_ERR_IS_OWNER;
}

// Finish access to protected variables
void app_mutex_release(void)
{
  RTOS_ERR err;
  OSMutexPost(app_mutex_handle, OS_OPT_POST_NONE, &err);
  (void)err;
}
