/***************************************************************************/ /**
 * @file button_baremetal.c
 * @brief button baremetal examples functions
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

#include "button_baremetal.h"
#include "sl_si91x_button.h"
#include "sl_si91x_button_pin_config.h"
#include "sl_si91x_button_instances.h"

#ifndef SI917_DEVKIT
#include "sl_si91x_led.h"
#include "sl_si91x_led_config.h"
#include "sl_si91x_led_instances.h"
#else
#include "sl_si91x_rgb_led.h"
#include "sl_si91x_rgb_led_config.h"
#include "sl_si91x_rgb_led_instances.h"
#endif

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
#ifndef BUTTON_INSTANCE
#define BUTTON_INSTANCE button_btn0
#endif

#ifndef LED_INSTANCE
#define LED_INSTANCE led_led0
#endif
/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/
/*******************************************************************************
**********************  Local Function prototypes   ***************************
******************************************************************************/
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/

/***************************************************************************/ /**
 * Initialize example.
 ******************************************************************************/
void button_init(void)
{
  return;
}

/***************************************************************************/ /**
 * Call back on button change.
 ******************************************************************************/
void button_process_action(void)
{
  return;
}

void sl_si91x_button_isr(uint8_t pin, int8_t state)
{
  if (pin == BUTTON_INSTANCE.pin) {
    if (state == BUTTON_PRESSED) {
#ifndef SI917_DEVKIT
      sl_si91x_led_toggle(LED_INSTANCE.pin);
#else
      sl_si91x_simple_rgb_led_toggle(&LED_INSTANCE);
#endif
    }
  }
}
