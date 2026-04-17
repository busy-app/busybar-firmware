/***************************************************************************/ /**
 * @file
 * @brief Sleeptimer sample application
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

#include "sl_sleeptimer.h"
#include "sl_sleeptimer_config.h"
#include "app.h"
#include "sl_si91x_led.h"
#include "rsi_debug.h"
#include "sleeptimer.h"

#define TOOGLE_DELAY_MS1_ONESHOT  160000 //sleeptimer1 oneshot timeout in ticks
#define TOOGLE_DELAY_MS1_PERIODIC 400    //sleeptimer1 periodic timeout in ms
/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/
sl_sleeptimer_timer_handle_t timer1; //sleeptimer1 handle
bool toggle_timeout  = false;        //Indicates sleeptimer1 timeout
bool oneshot_timeout = true;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
//Sleeptimer timeout callbacks
static void on_timeout_timer1(sl_sleeptimer_timer_handle_t *handle,
                              void *data); //sleeptimer1 callback
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************/ /**
 * Initialize blink example.
 ******************************************************************************/
void sleeptimer_init(void)
{
  bool is_running = false;
#if defined(SL_SI91X_ACX_MODULE)
  //  Toggles the current state of a board '0' number LED.
  sl_si91x_led_toggle(SL_LED_LED0_PIN);
#else
  //  Toggles the current state of a board '1' number LED.
  sl_si91x_led_toggle(SL_LED_LED1_PIN);
#endif
  //Start a 5000ms oneshot timer
  sl_sleeptimer_start_timer(&timer1,
                            TOOGLE_DELAY_MS1_ONESHOT,
                            on_timeout_timer1,
                            NULL,
                            0,
                            SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  //Wait for timer1 oneshot timeout
  do {
    sl_sleeptimer_is_timer_running(&timer1, &is_running);
  } while (is_running);
  oneshot_timeout = false;
  //Start 400 ms periodic timer
  sl_sleeptimer_start_periodic_timer_ms(&timer1,
                                        TOOGLE_DELAY_MS1_PERIODIC,
                                        on_timeout_timer1,
                                        NULL,
                                        0,
                                        SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
}

/***************************************************************************/ /**
 * Blink ticking function.
 ******************************************************************************/
void sleeptimer_process_action(void)
{
  if (toggle_timeout == true) {
    // Toggles the current state of the LED.
#if defined(SL_SI91X_ACX_MODULE)
    sl_si91x_led_toggle(SL_LED_LED0_PIN);
#else
    sl_si91x_led_toggle(SL_LED_LED1_PIN);
#endif
    toggle_timeout = false;
  }
}

/***************************************************************************/ /**
 * Sleeptimer timeout callback.
 ******************************************************************************/
static void on_timeout_timer1(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)&handle;
  (void)&data;
  if (oneshot_timeout) {
    DEBUGOUT("\r\n *******Timer1 oneshot timeout******* \r\n");
  } else {
    DEBUGOUT("\r\n Timer1 periodic timeout \r\n");
  }
  //Toggle LED
  toggle_timeout = true;
}
