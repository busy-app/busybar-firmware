/*******************************************************************************
 * @file app.c
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "blinky.h"
#include "app.h"
#include "cmsis_os2.h"
/******************************************************
 *               Variable Definitions
 ******************************************************/
osThreadId_t tid_thread_led_on;  // thread id for LED ON thread
osThreadId_t tid_thread_led_off; // thread id for LED OFF thread

const osThreadAttr_t led_on_thread_attributes = {
  .name       = "led_on",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 500, // using 500 for all the threads, may use as low as possible based on the need of respective thread
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};
const osThreadAttr_t led_off_thread_attributes = {
  .name       = "led_off",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 500, // using 500 for all the threads, may use as low as possible based on the need of respective thread
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};
/*******************************************************************************
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // initialize LED for blinky functionality
  blinky_init();

  // create a thread for LED ON functionality
  tid_thread_led_on = osThreadNew((osThreadFunc_t)led_on_process_action, NULL, &led_on_thread_attributes);
  if (tid_thread_led_on == NULL) {
    // Handle error - Thread creation
    return;
  }

  // create a thread for LED OFF functionality
  tid_thread_led_off = osThreadNew((osThreadFunc_t)led_off_process_action, NULL, &led_off_thread_attributes);
  if (tid_thread_led_off == NULL) {
    // Handle error - Thread creation
    return;
  }
}

/*******************************************************************************
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}
