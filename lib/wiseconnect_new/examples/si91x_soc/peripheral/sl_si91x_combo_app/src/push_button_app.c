/*******************************************************************************
* @file push_button_app.c
* @brief button baremetal examples functions
********************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
********************************************************************************
*
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
*******************************************************************************/

#include "push_button_app.h"
#include "cmsis_os2.h"

#include "rsi_debug.h"
#include "sl_component_catalog.h"
#include "sl_si91x_button.h"
#include "sl_si91x_button_instances.h"
#include "sl_si91x_button_pin_config.h"
#include "sl_si91x_led.h"
#include "sl_si91x_led_config.h"
#include "sl_si91x_led_instances.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
#ifndef BUTTON_INSTANCE_0
#define BUTTON_INSTANCE_0 button_btn0
#endif

#ifndef LED_INSTANCE_0
#define LED_INSTANCE_0 led_led0
#endif

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/
#if defined(SL_CATALOG_KERNEL_PRESENT)
extern osSemaphoreId_t pwm_thread_sem;
#endif
/*******************************************************************************
 * Initialize example.
 ******************************************************************************/
void button_init(void)
{
  return;
}

/*******************************************************************************
 * Call back on button change.
 ******************************************************************************/
void button_app(void)
{
  while (1) {
    // Do Nothing
    osDelay(1000);
  }
  return;
}

void sl_si91x_button_isr(uint8_t pin, int8_t state)
{
  osStatus_t status;

  if (pin == BUTTON_INSTANCE_0.pin) {
    if (state == BUTTON_RELEASED) {
      sl_si91x_led_toggle(LED_INSTANCE_0.pin);

#if defined(SL_CATALOG_KERNEL_PRESENT)
      status = osSemaphoreRelease(pwm_thread_sem);
      if (status != osOK) {
        DEBUGOUT("button isr failed to release semaphore: %d\n", status);
      } else {
        DEBUGOUT("Button task: released semaphore\n");
      }
#endif
    }
  }
}
