/***************************************************************************/ /**
 * @file sl_si91x_rgb_led.c
 * @brief RGB LED manipulation routines; stack and example APIs
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
#include "sl_si91x_rgb_led.h"
#include "sl_si91x_rgb_led_instances.h"
#include "si91x_device.h"
#include "sl_driver_gpio.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_sleeptimer.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
/*Delay for PWM simulation*/
#ifndef TICK_DELAY
#define TICK_DELAY 2
#endif

/*Total delay for each PWM cycle*/
#ifndef PULSE_PERIOD
#define PULSE_PERIOD (TICK_DELAY * 0xFF)
#endif

typedef enum { COLOUR_RED, COLOUR_GREEN, COLOUR_BLUE } rgb_colour_t;

#define LED_ON  0
#define LED_OFF 1

/*Default colour white*/
#define RGB_DEFAULT_COLOUR 0xFFFFFF

#define INVALID_HANDLE -1

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/
typedef struct {
  int32_t red;
  int32_t blue;
  int32_t green;
} rgb_values_t; // Represents both duty cycle and timer counter for software PWM

typedef struct {
  rgb_values_t timer_counter;             // Timer counter for each color
  rgb_values_t duty_cycle;                // Duty cycle for each color
  bool on_state;                          // ON state of each LED
  sl_sleeptimer_timer_handle_t pwm_timer; // Sleeptimer handle for each LED
} rgb_led_params_t;

rgb_led_params_t rgb_led_params[SL_SI91X_RGB_LED_COUNT];

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static void rgb_led_callback(sl_sleeptimer_timer_handle_t *timer, void *data);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

void sl_si91x_simple_rgb_led_init(const sl_rgb_led_t *handle)
{
  /* Check for null pointer */
  if (handle == NULL || handle->red == NULL || handle->green == NULL || handle->blue == NULL) {
    return; // Exit if any handle is null
  }

  /*Enable clock*/
  sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)M4CLK_GPIO);

  sl_si91x_gpio_pin_config_t sl_gpio_red_pin_config = { { handle->red->port, handle->red->pin }, GPIO_OUTPUT };
  sl_gpio_set_configuration(sl_gpio_red_pin_config);

  sl_si91x_gpio_pin_config_t sl_gpio_green_pin_config = { { handle->green->port, handle->green->pin }, GPIO_OUTPUT };
  sl_gpio_set_configuration(sl_gpio_green_pin_config);

  sl_si91x_gpio_pin_config_t sl_gpio_blue_pin_config = { { handle->blue->port, handle->blue->pin }, GPIO_OUTPUT };
  sl_gpio_set_configuration(sl_gpio_blue_pin_config);

  // Set default color white color
  sl_si91x_simple_rgb_led_set_colour(handle, RGB_DEFAULT_COLOUR);
}

void sl_si91x_simple_rgb_led_on(const sl_rgb_led_t *handle)
{
  sl_status_t status;

  /* Check for null pointer */
  if (handle == NULL || handle->red == NULL || handle->green == NULL || handle->blue == NULL) {
    return; // Exit if any handle is null
  }

  // Timer associated with the RGB LED instance
  uint8_t led_instance = handle->red->led_number;

  /*Start Sleeptimer*/
  status = sl_sleeptimer_start_periodic_timer(&rgb_led_params[led_instance].pwm_timer,
                                              TICK_DELAY,
                                              rgb_led_callback,
                                              (void *)handle,
                                              0,
                                              SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);

  /*change the on_state of LED*/
  if (status == SL_STATUS_OK) {
    rgb_led_params[led_instance].on_state = true;
  }
}

static void led_colour_on_off(const sl_rgb_led_t *handle, rgb_colour_t colour, int operation)
{
  sl_gpio_t gpio_port_pin;

  /* Check for null pointer */
  if (handle == NULL || handle->red == NULL || handle->green == NULL || handle->blue == NULL) {
    return; // Exit if any handle is null
  }

  switch (colour) {
    case COLOUR_RED:
      gpio_port_pin.port = handle->red->port;
      gpio_port_pin.pin  = handle->red->pin;
      break;

    case COLOUR_GREEN:
      gpio_port_pin.port = handle->green->port;
      gpio_port_pin.pin  = handle->green->pin;
      break;

    case COLOUR_BLUE:
      gpio_port_pin.port = handle->blue->port;
      gpio_port_pin.pin  = handle->blue->pin;
      break;

    default:
      break;
  }

  if (operation == LED_ON) {
    sl_gpio_driver_clear_pin(&gpio_port_pin);
  } else {
    sl_gpio_driver_set_pin(&gpio_port_pin);
  }
}

void sl_si91x_simple_rgb_led_off(const sl_rgb_led_t *handle)
{
  /* Check for null pointer */
  if (handle == NULL || handle->red == NULL || handle->green == NULL || handle->blue == NULL) {
    return; // Exit if any handle is null
  }

  // Timer associated with the RGB LED instance
  uint8_t led_instance = handle->red->led_number;

  sl_gpio_t red_gpio_port_pin   = { handle->red->port, handle->red->pin };
  sl_gpio_t green_gpio_port_pin = { handle->green->port, handle->green->pin };
  sl_gpio_t blue_gpio_port_pin  = { handle->blue->port, handle->blue->pin };

  sl_gpio_driver_set_pin(&red_gpio_port_pin);
  sl_gpio_driver_set_pin(&green_gpio_port_pin);
  sl_gpio_driver_set_pin(&blue_gpio_port_pin);

  /*Stop Sleeptimer*/
  sl_sleeptimer_stop_timer(&rgb_led_params[led_instance].pwm_timer);

  /*Change the on_state of LED*/
  rgb_led_params[led_instance].on_state = false;
}

void sl_si91x_simple_rgb_led_toggle(const sl_rgb_led_t *handle)
{
  /* Check for null pointer */
  if (handle == NULL || handle->red == NULL || handle->green == NULL || handle->blue == NULL) {
    return; // Exit if any handle is null
  }

  // Timer associated with the RGB LED instance
  uint8_t led_instance = handle->red->led_number;

  if (rgb_led_params[led_instance].on_state == true) {
    sl_si91x_simple_rgb_led_off(handle);
  } else {
    sl_si91x_simple_rgb_led_on(handle);
  }
}

void sl_si91x_simple_rgb_led_set_colour(const sl_rgb_led_t *handle, int rgb_colour)
{
  /* Check for null pointer */
  if (handle == NULL || handle->red == NULL || handle->green == NULL || handle->blue == NULL) {
    return; // Exit if any handle is null
  }

  /*Extract duty cycle values from the colour hex code*/
  rgb_led_params[handle->red->led_number].duty_cycle.red     = ((rgb_colour >> 16) & 0xFF) * TICK_DELAY;
  rgb_led_params[handle->green->led_number].duty_cycle.green = ((rgb_colour >> 8) & 0xFF) * TICK_DELAY;
  rgb_led_params[handle->blue->led_number].duty_cycle.blue   = (rgb_colour & 0xFF) * TICK_DELAY;
}

uint8_t sl_si91x_simple_rgb_led_get_current_state(const sl_rgb_led_t *handle)
{
  /* Check for null pointer */
  if (handle == NULL || handle->red == NULL || handle->green == NULL || handle->blue == NULL) {
    return INVALID_HANDLE; // Exit if any handle is null
  }

  // Timer associated with the RGB LED instance
  uint8_t led_instance = handle->red->led_number;

  return (uint8_t)rgb_led_params[led_instance].on_state;
}

void sl_si91x_simple_rgb_led_get_colour(const sl_rgb_led_t *handle, uint16_t *red, uint16_t *green, uint16_t *blue)
{
  /* Check for null pointer */
  if (handle == NULL || handle->red == NULL || handle->green == NULL || handle->blue == NULL) {
    return; // Exit if any handle is null
  }

  // Retrieve the current duty cycle (intensity) values for each color
  *red   = (uint16_t)(rgb_led_params[handle->red->led_number].duty_cycle.red);
  *green = (uint16_t)(rgb_led_params[handle->green->led_number].duty_cycle.green);
  *blue  = (uint16_t)(rgb_led_params[handle->blue->led_number].duty_cycle.blue);
}

/**
 * This function simulates a software-based PWM to pulse RGB LEDs based on their
 * respective duty cycle levels.
 */
static void rgb_led_callback(sl_sleeptimer_timer_handle_t *timer_handle, void *data)
{
  sl_rgb_led_t *handle = (sl_rgb_led_t *)data;

  uint8_t led_instance = handle->red->led_number;

  if (timer_handle == &rgb_led_params[led_instance].pwm_timer) {
    /*Pulse drive for red*/
    if (rgb_led_params[led_instance].timer_counter.red == rgb_led_params[led_instance].duty_cycle.red) {
      led_colour_on_off(handle, COLOUR_RED, LED_OFF);
    } else {
      if (rgb_led_params[led_instance].timer_counter.red > PULSE_PERIOD) {
        led_colour_on_off(handle, COLOUR_RED, LED_ON);
        rgb_led_params[led_instance].timer_counter.red = -1;
      }
    }

    /*Pulse drive for green*/
    if (rgb_led_params[led_instance].timer_counter.green == rgb_led_params[led_instance].duty_cycle.green) {
      led_colour_on_off(handle, COLOUR_GREEN, LED_OFF);
    } else {
      if (rgb_led_params[led_instance].timer_counter.green > PULSE_PERIOD) {
        led_colour_on_off(handle, COLOUR_GREEN, LED_ON);
        rgb_led_params[led_instance].timer_counter.green = -1;
      }
    }

    /*Pulse drive for blue*/
    if (rgb_led_params[led_instance].timer_counter.blue == rgb_led_params[led_instance].duty_cycle.blue) {
      led_colour_on_off(handle, COLOUR_BLUE, LED_OFF);
    } else {
      if (rgb_led_params[led_instance].timer_counter.blue > PULSE_PERIOD) {
        led_colour_on_off(handle, COLOUR_BLUE, LED_ON);
        rgb_led_params[led_instance].timer_counter.blue = -1;
      }
    }

    rgb_led_params[led_instance].timer_counter.red++;
    rgb_led_params[led_instance].timer_counter.green++;
    rgb_led_params[led_instance].timer_counter.blue++;
  }
}

/*Older version APIs (Not recommended to use)*/

void sl_si91x_rgb_led_init(const sl_led_t *handle)
{
  /*Enable clock*/
  sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)M4CLK_GPIO);

  /*Set the GPIO pin MUX */
  sl_gpio_t rgb_gpio_port_pin = { handle->port, handle->pin };
  sl_gpio_driver_set_pin_mode(&rgb_gpio_port_pin, 0, 1);

  /*PAD selection*/
  sl_si91x_gpio_driver_enable_pad_selection(handle->pad);

  /*Set pin Direction*/
  sl_si91x_gpio_driver_set_pin_direction(handle->port, handle->pin, (sl_si91x_gpio_direction_t)GPIO_OUTPUT);

  /*REN enable */
  sl_si91x_gpio_driver_enable_pad_receiver(handle->pin);
}

void sl_si91x_rgb_led_on(const sl_led_t *handle)
{
  sl_gpio_t rgb_gpio_port_pin = { handle->port, handle->pin };
  sl_gpio_driver_clear_pin(&rgb_gpio_port_pin);
}

void sl_si91x_rgb_led_off(const sl_led_t *handle)
{
  sl_gpio_t rgb_gpio_port_pin = { handle->port, handle->pin };
  sl_gpio_driver_set_pin(&rgb_gpio_port_pin);
}

uint8_t sl_si91x_rgb_led_get_current_state(const sl_led_t *handle)
{
  sl_gpio_t rgb_gpio_port_pin = { handle->port, handle->pin };
  return sl_gpio_driver_get_pin_output(&rgb_gpio_port_pin);
}

void sl_si91x_rgb_led_toggle(const sl_led_t *handle)
{
  sl_gpio_t rgb_gpio_port_pin = { handle->port, handle->pin };
  sl_gpio_driver_toggle_pin(&rgb_gpio_port_pin);
}