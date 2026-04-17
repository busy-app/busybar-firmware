/*******************************************************************************
 * @file
 * @brief Top level application functions (FreeRTOS adaptation)
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

// FreeRTOS task handle
static TaskHandle_t wallclockTaskHandle = NULL;

// FreeRTOS task function for wallclock logic
static void wallclock_task(void *pvParameters)
{
  (void)pvParameters;
  sleeptimer_app_init();
  while (1) {
    sleeptimer_app_process_action();
    vTaskDelay(pdMS_TO_TICKS(100)); // Run every 100 ms (adjust as needed)
  }
}

/*******************************************************************************
 * Initialize application and create FreeRTOS task.
 ******************************************************************************/
void app_init(void)
{
  xTaskCreate(wallclock_task,
              "WallclockTask",
              configMINIMAL_STACK_SIZE + 256, // Adjust stack size as needed
              NULL,
              tskIDLE_PRIORITY + 1,
              &wallclockTaskHandle);
}

/*******************************************************************************
 * App ticking function (unused in FreeRTOS version).
 ******************************************************************************/
void app_process_action(void)
{
  // No action needed; processing happens in the FreeRTOS task.
}
