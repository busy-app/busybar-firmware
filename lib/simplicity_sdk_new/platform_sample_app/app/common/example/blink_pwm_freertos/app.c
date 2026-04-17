/*******************************************************************************
 * @file
 * @brief Top level application functions
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
#include "blink_pwm_app.h"
#include "FreeRTOS.h"
#include "task.h"

#define BLINK_PWM_TASK_STACK_SIZE   (512)
#define BLINK_PWM_TASK_PRIORITY     (tskIDLE_PRIORITY + 2)

static TaskHandle_t blinkPwmTaskHandle = NULL;

static void blink_pwm_task(void *pvParameters)
{
  (void)pvParameters;
  blink_pwm_init();

  while (1) {
    blink_pwm_process_action();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

/*******************************************************************************
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  xTaskCreate(blink_pwm_task,
              "BlinkPwmTask",
              BLINK_PWM_TASK_STACK_SIZE,
              NULL,
              BLINK_PWM_TASK_PRIORITY,
              &blinkPwmTaskHandle);
}

/*******************************************************************************
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}
