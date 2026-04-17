/***************************************************************************//**
 * @file
 * @brief Core application logic for FreeRTOS.
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
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"
#include "sl_main_init.h"
#include "app.h"
#include "app_assert.h"

#define APP_TASK_NAME          "app_task"
#define APP_TASK_STACK_SIZE    512u
#define APP_TASK_PRIO          24u

// Application task function
static void app_task(void *p_arg);

// Task handle
static TaskHandle_t      app_task_handle  = NULL;
// Semaphore for button state
static SemaphoreHandle_t button_semaphore = NULL;

/******************************************************************************
 * Application Runtime Init.
 *****************************************************************************/
void app_init_bt(void)
{
  BaseType_t ret;
  // Create the task for sl_app_process_action
  ret = xTaskCreate(app_task,
                    APP_TASK_NAME,
                    APP_TASK_STACK_SIZE,
                    NULL,
                    APP_TASK_PRIO,
                    &app_task_handle);
  app_assert(ret == pdPASS, "Application task creation failed.");
  // Create a semaphore for button state
  button_semaphore = xSemaphoreCreateCounting(UINT16_MAX, 0);
  app_assert(button_semaphore != NULL, "Button semaphore creation failed.");
}

/******************************************************************************
 * Set button state.
 *****************************************************************************/
void app_set_button_state(void)
{
  BaseType_t woken = pdFALSE;
  (void)xSemaphoreGiveFromISR(button_semaphore, &woken);
  portYIELD_FROM_ISR(woken);
}

/******************************************************************************
 * Get button state.
 * @return button state (true if pressed)
 *****************************************************************************/
bool app_get_button_state(void)
{
  BaseType_t ret = xSemaphoreTake(button_semaphore, portMAX_DELAY);
  return (ret == pdTRUE);
}

/******************************************************************************
 * Application task
 *****************************************************************************/
static void app_task(void *p_arg)
{
  (void)p_arg;
  for (;; ) {
    app_process_action();
  }
}
