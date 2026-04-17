/***************************************************************************/ /**
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
#include "app.h"
#include "stddef.h"
#include "cmsis_os2.h"

#include "i2c_follower_app.h"
#include "usart_app.h"

/******************************************************
 *               Variable Definitions
 ******************************************************/
osThreadId_t tid_thread_i2c;   // thread id for i2c thread
osThreadId_t tid_thread_usart; // thread id for usart thread

const osThreadAttr_t i2c_follower_thread_attributes = {
  .name       = "i2c_follower",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 500, // using 500 for all the threads, may use as low as possible based on the need of respective thread
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};
const osThreadAttr_t usart_thread_attributes = {
  .name       = "usart",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 500, // using 500 for all the threads, may use as low as possible based on the need of respective thread
  .priority   = osPriorityLow1,
  .tz_module  = 0,
  .reserved   = 0,
};
/*******************************************************************************
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // initialize i2c driver
  i2c_follower_example_init();

  // initialize usart driver
  usart_example_init();

  // create a thread for i2c follower functionality
  tid_thread_i2c =
    osThreadNew((osThreadFunc_t)i2c_follower_example_process_action, NULL, &i2c_follower_thread_attributes);
  if (tid_thread_i2c == NULL) {
    // Handle error - Thread creation
    return;
  }

  // create a thread for usart functionality
  tid_thread_usart = osThreadNew((osThreadFunc_t)usart_example_process_action, NULL, &usart_thread_attributes);
  if (tid_thread_usart == NULL) {
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
