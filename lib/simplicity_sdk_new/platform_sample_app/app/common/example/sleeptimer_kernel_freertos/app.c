/*******************************************************************************
 * @file
 * @brief Top level application functions (FreeRTOS variant)
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "sleeptimer_app.h"
#include "FreeRTOS.h"
#include "task.h"

#define APP_TASK_STACK_SIZE      (256)
#define APP_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)

static void app_task(void *pvParameters)
{
  (void)pvParameters;
  sleeptimer_app_init();
  while (1) {
    sleeptimer_app_process_action();
    vTaskDelay(pdMS_TO_TICKS(100)); // Poll every 100ms
  }
}

/*****************************************************************************
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  xTaskCreate(app_task, "AppTask", APP_TASK_STACK_SIZE, NULL, APP_TASK_PRIORITY, NULL);
}

/*****************************************************************************
 * App ticking function (not used in FreeRTOS variant).
 ******************************************************************************/
void app_process_action(void)
{
  // In FreeRTOS, the app_process_action is handled in the app_task.
  // This function can be left empty or used for other periodic tasks if needed.
}
