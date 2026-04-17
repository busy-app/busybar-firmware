/***************************************************************************//**
 * @file
 * @brief PWM LED driver implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stddef.h>

#include "sl_pwm_led.h"

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
/// stores the address of corresponding sl_peripheral_t variable
static sl_peripheral_t peripheral;
/*
 *  Private functions
 */
static sl_bus_clock_t pwm_led_get_timer_clock(TIMER_TypeDef *timer)
{
  sl_bus_clock_t timer_clock = 0;
  switch ((uint32_t)timer) {
#if defined(TIMER0_BASE)
    case TIMER0_BASE:
      timer_clock = SL_BUS_CLOCK_TIMER0;
      peripheral = SL_PERIPHERAL_TIMER0;
      break;
#endif
#if defined(TIMER1_BASE)
    case TIMER1_BASE:
      timer_clock = SL_BUS_CLOCK_TIMER1;
      peripheral = SL_PERIPHERAL_TIMER1;
      break;
#endif
#if defined(TIMER2_BASE)
    case TIMER2_BASE:
      timer_clock = SL_BUS_CLOCK_TIMER2;
      peripheral = SL_PERIPHERAL_TIMER2;
      break;
#endif
#if defined(TIMER3_BASE)
    case TIMER3_BASE:
      timer_clock = SL_BUS_CLOCK_TIMER3;
      peripheral = SL_PERIPHERAL_TIMER3;
      break;
#endif
#if defined(TIMER4_BASE)
    case TIMER4_BASE:
      timer_clock = SL_BUS_CLOCK_TIMER4;
      peripheral = SL_PERIPHERAL_TIMER4;
      break;
#endif
#if defined(TIMER5_BASE)
    case TIMER5_BASE:
      timer_clock = SL_BUS_CLOCK_TIMER5;
      peripheral = SL_PERIPHERAL_TIMER5;
      break;
#endif
#if defined(TIMER6_BASE)
    case TIMER6_BASE:
      timer_clock = SL_BUS_CLOCK_TIMER6;
      peripheral = SL_PERIPHERAL_TIMER6;
      break;
#endif
    default:
      EFM_ASSERT(0);
      break;
  }
  return timer_clock;
}

void sl_pwm_led_start(void *led_handler)
{
  sl_led_pwm_t *led = (sl_led_pwm_t *)led_handler;
  // Enable PWM output
#if defined(_TIMER_ROUTE_MASK)
#if defined(_SILICON_LABS_32B_SERIES_2)
  BUS_RegMaskedSet(&led->timer->ROUTE,
                   1 << (led->channel + _TIMER_ROUTE_CC0PEN_SHIFT));
#else
  sl_hal_bus_reg_set_mask(&led->timer->ROUTE,
                          1 << (led->channel + _TIMER_ROUTE_CC0PEN_SHIFT));
#endif
#elif defined(_TIMER_ROUTELOC0_MASK)
#if defined(_SILICON_LABS_32B_SERIES_2)
  BUS_RegMaskedSet(&led->timer->ROUTEPEN,
                   1 << (led->channel + _TIMER_ROUTEPEN_CC0PEN_SHIFT));
#else
  sl_hal_bus_reg_set_mask(&led->timer->ROUTEPEN,
                          1 << (led->channel + _TIMER_ROUTEPEN_CC0PEN_SHIFT));
#endif
#elif defined(_GPIO_TIMER_ROUTEEN_MASK)
  GPIO->TIMERROUTE_SET[TIMER_NUM(led->timer)].ROUTEEN = 1 << (led->channel + _GPIO_TIMER_ROUTEEN_CC0PEN_SHIFT);
#elif defined(_GPIO_TIMER0_ROUTEEN_MASK)
  switch (TIMER_NUM(led->timer)) {
    case 0:
      GPIO->TIMER0ROUTE_SET[0].ROUTEEN = 1 << (led->channel + _GPIO_TIMER0_ROUTEEN_CC0PEN_SHIFT);
      break;
    case 1:
      GPIO->TIMER1ROUTE_SET[0].ROUTEEN = 1 << (led->channel + _GPIO_TIMER1_ROUTEEN_CC0PEN_SHIFT);
      break;
    case 2:
      GPIO->TIMER2ROUTE_SET[0].ROUTEEN = 1 << (led->channel + _GPIO_TIMER2_ROUTEEN_CC0PEN_SHIFT);
      break;
    case 3:
      GPIO->TIMER3ROUTE_SET[0].ROUTEEN = 1 << (led->channel + _GPIO_TIMER3_ROUTEEN_CC0PEN_SHIFT);
      break;
    default:
      EFM_ASSERT(0);
      break;
  }
#else
#error "Unknown route setting"
#endif
}

void sl_pwm_led_stop(void *led_handler)
{
  sl_led_pwm_t *led = (sl_led_pwm_t *)led_handler;
  // Disable PWM output
#if defined(_TIMER_ROUTE_MASK)
#if defined(_SILICON_LABS_32B_SERIES_2)
  BUS_RegMaskedClear(&led->timer->ROUTE,
                     1 << (led->channel + _TIMER_ROUTE_CC0PEN_SHIFT));
#else
  sl_hal_bus_reg_clear_mask(&led->timer->ROUTE,
                            1 << (led->channel + _TIMER_ROUTE_CC0PEN_SHIFT));
#endif
#elif defined(_TIMER_ROUTELOC0_MASK)
#if defined(_SILICON_LABS_32B_SERIES_2)
  BUS_RegMaskedClear(&led->timer->ROUTEPEN,
                     1 << (led->channel + _TIMER_ROUTEPEN_CC0PEN_SHIFT));
#else
  sl_hal_bus_reg_clear_mask(&led->timer->ROUTEPEN,
                            1 << (led->channel + _TIMER_ROUTEPEN_CC0PEN_SHIFT));
#endif
#elif defined(_GPIO_TIMER_ROUTEEN_MASK)
  GPIO->TIMERROUTE_CLR[TIMER_NUM(led->timer)].ROUTEEN = 1 << (led->channel + _GPIO_TIMER_ROUTEEN_CC0PEN_SHIFT);
#elif defined(_GPIO_TIMER0_ROUTEEN_MASK)
  switch (TIMER_NUM(led->timer)) {
    case 0:
      GPIO->TIMER0ROUTE_CLR[0].ROUTEEN = 1 << (led->channel + _GPIO_TIMER0_ROUTEEN_CC0PEN_SHIFT);
      break;
    case 1:
      GPIO->TIMER1ROUTE_CLR[0].ROUTEEN = 1 << (led->channel + _GPIO_TIMER1_ROUTEEN_CC0PEN_SHIFT);
      break;
    case 2:
      GPIO->TIMER2ROUTE_CLR[0].ROUTEEN = 1 << (led->channel + _GPIO_TIMER2_ROUTEEN_CC0PEN_SHIFT);
      break;
    case 3:
      GPIO->TIMER3ROUTE_CLR[0].ROUTEEN = 1 << (led->channel + _GPIO_TIMER3_ROUTEEN_CC0PEN_SHIFT);
      break;
    default:
      EFM_ASSERT(0);
      break;
  }
#else
#error "Unknown route setting"
#endif
}

/*
 *  Public functions
 */
sl_status_t sl_pwm_led_init(void *led_handler)
{
  sl_led_pwm_t *led = (sl_led_pwm_t *)led_handler;

  // Validate that the selected channel exist in the selected timer.
#if defined(_SILICON_LABS_32B_SERIES_2)
  EFM_ASSERT(TIMER_REF_VALID(led->timer));
  EFM_ASSERT(TIMER_CH_VALID(led->channel));
#else
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(led->timer));
#endif
  led->set_color = sl_pwm_led_set_color;
  led->get_color = sl_pwm_led_get_color;

  sl_bus_clock_t timer_clock = pwm_led_get_timer_clock(led->timer);
  sl_clock_manager_enable_bus_clock(timer_clock);

#if defined(_SILICON_LABS_32B_SERIES_2)
  TIMER_InitCC_TypeDef channel_init = TIMER_INITCC_DEFAULT;
  channel_init.mode = timerCCModePWM;
  channel_init.cmoa = timerOutputActionToggle;
  channel_init.edge = timerEdgeBoth;
#else
  sl_hal_timer_channel_config_t channel_init = SL_HAL_TIMER_CHANNEL_CONFIG_DEFAULT;
  channel_init.channel_mode = SL_HAL_TIMER_CHANNEL_MODE_PWM;
  channel_init.compare_match_output_action = SL_HAL_TIMER_CHANNEL_OUTPUT_ACTION_TOGGLE;
  channel_init.input_capture_edge = SL_HAL_TIMER_CHANNEL_EDGE_BOTH;
#endif
#if defined(_SILICON_LABS_32B_SERIES_2)
  led->timer->EN_SET = TIMER_EN_EN;
#else
  sl_hal_timer_enable(led->timer);
#endif

  // The configured resolution configuration has to be at least 2. And not bigger than the timer max count + 1
#if defined(_SILICON_LABS_32B_SERIES_2)
  uint32_t max_count = TIMER_MaxCount(led->timer);
#else
  uint32_t max_count = SL_HAL_TIMER_MAX_COUNT(led->timer);
#endif
  if ((led->resolution < 2UL) || (led->resolution > (max_count))) {
    // The user configured max level value is invalid

    sl_clock_manager_disable_bus_clock(timer_clock);

    return SL_STATUS_FAIL;
  }

  // User configuration values are OK. Continue with the initialization...

  // Set CC channel parameters
#if defined(_SILICON_LABS_32B_SERIES_2)
  channel_init.outInvert = !!led->polarity;
  TIMER_InitCC(led->timer, led->channel, &channel_init);
#else
  channel_init.output_invert = !!led->polarity;
  sl_hal_timer_channel_init(led->timer, led->channel, &channel_init);
#endif

  // Set PWM pin as output
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

  sl_gpio_t gpio;
  gpio.port = (sl_gpio_port_t)led->port;
  gpio.pin = led->pin;
  sl_gpio_set_pin_mode(&gpio,
                       SL_GPIO_MODE_PUSH_PULL,
                       led->polarity);

  // Configure CC channel pinout
#if defined(_TIMER_ROUTE_MASK)
#if defined(_SILICON_LABS_32B_SERIES_2)
  BUS_RegMaskedWrite(&led->timer->ROUTE,
                     _TIMER_ROUTE_LOCATION_MASK,
                     led->location << _TIMER_ROUTE_LOCATION_SHIFT);
#else
  sl_hal_bus_reg_write_mask(&led->timer->ROUTE,
                            _TIMER_ROUTE_LOCATION_MASK,
                            led->location << _TIMER_ROUTE_LOCATION_SHIFT);
#endif
#elif defined(_TIMER_ROUTELOC0_MASK)
#if defined(_SILICON_LABS_32B_SERIES_2)
  BUS_RegMaskedWrite(&led->timer->ROUTELOC0,
                     _TIMER_ROUTELOC0_CC0LOC_MASK << (led->channel * 8U),
                     led->location << (led->channel * 8U));
#else
  sl_hal_bus_reg_write_mask(&led->timer->ROUTELOC0,
                            _TIMER_ROUTELOC0_CC0LOC_MASK << (led->channel * 8U),
                            led->location << (led->channel * 8U));
#endif
#elif defined(_GPIO_TIMER_ROUTEEN_MASK)
  volatile uint32_t * route_register = &GPIO->TIMERROUTE[TIMER_NUM(led->timer)].CC0ROUTE;
  *(route_register + led->channel) = (led->port << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT)
                                     | (led->pin << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);
#elif defined(_GPIO_TIMER0_ROUTEEN_MASK)
  switch (TIMER_NUM(led->timer)) {
    volatile uint32_t * route_register;
    case 0:
      route_register = &GPIO->TIMER0ROUTE[0].CC0ROUTE;
      *(route_register + led->channel) = (led->port << _GPIO_TIMER0_CC0ROUTE_PORT_SHIFT)
                                         | (led->pin << _GPIO_TIMER0_CC0ROUTE_PIN_SHIFT);
      break;
    case 1:
      route_register = &GPIO->TIMER1ROUTE[0].CC0ROUTE;
      *(route_register + led->channel) = (led->port << _GPIO_TIMER1_CC0ROUTE_PORT_SHIFT)
                                         | (led->pin << _GPIO_TIMER1_CC0ROUTE_PIN_SHIFT);
      break;
    case 2:
      route_register = &GPIO->TIMER2ROUTE[0].CC0ROUTE;
      *(route_register + led->channel) = (led->port << _GPIO_TIMER2_CC0ROUTE_PORT_SHIFT)
                                         | (led->pin << _GPIO_TIMER2_CC0ROUTE_PIN_SHIFT);
      break;
    case 3:
      route_register = &GPIO->TIMER3ROUTE[0].CC0ROUTE;
      *(route_register + led->channel) = (led->port << _GPIO_TIMER3_CC0ROUTE_PORT_SHIFT)
                                         | (led->pin << _GPIO_TIMER3_CC0ROUTE_PIN_SHIFT);
      break;
    default:
      EFM_ASSERT(0);
      return SL_STATUS_FAIL;
  }
#else
#error "Unknown route setting"
#endif

// Configure the Timer TOP value on basis of the application PWM frequency and resolution config values
  uint32_t top = 0;
  uint32_t clock_freq;
  sl_clock_branch_t clock_branch;
// First find the TOP value that satisfies the PWM frequency config (if not set to 0 = "don't care")
  if (led->frequency > 0U) {
    clock_branch = sl_device_peripheral_get_clock_branch(peripheral);
    sl_clock_manager_get_clock_branch_frequency(clock_branch, &clock_freq);
    top = (clock_freq / (led->frequency)) - 1U;
  }

// Then adjust it to make it an even multiplier of the PWM resolution config
  top = (top / (led->resolution - 1)) * (led->resolution - 1);

  if (top < (led->resolution - 1)) {
    // The PWM resolution config demands a higher TOP value than what the frequency config sets.
    // In this case the resolution config takes precedence over frequency.
    // So simply set the TOP value equal to the max input value (resolution value - 1).
    top = led->resolution - 1;
  }

#if defined(_SILICON_LABS_32B_SERIES_2)
  TIMER_TopSet(led->timer, top);

// Initialize TIMER
  TIMER_Init_TypeDef timer_init = TIMER_INIT_DEFAULT;
  TIMER_Init(led->timer, &timer_init);
#else
  // Initialize TIMER 
  sl_hal_timer_config_t timer_init = SL_HAL_TIMER_CONFIG_DEFAULT;
  sl_hal_timer_init(led->timer, &timer_init);
  sl_hal_timer_enable(led->timer);
  sl_hal_timer_set_top(led->timer, top);
  sl_hal_timer_wait_sync(led->timer);
  sl_hal_timer_start(led->timer);
#endif

// Initialize the level settings values
  sl_pwm_led_set_color(led, led->level);

// Set LED state to Off.
  sl_pwm_led_stop(led);

  return SL_STATUS_OK;
}

void sl_pwm_led_set_color(void *led_handler, uint16_t color)
{
  sl_led_pwm_t *led = (sl_led_pwm_t *)led_handler;

#if defined(_SILICON_LABS_32B_SERIES_2)
  // Get the counter increments per intensity level step
  uint32_t level_increments = TIMER_TopGet(led->timer) / (led->resolution - 1);
  led->level = color;

  // Ensure that the color level value does not exceed the timer's size.
  EFM_ASSERT((led->level * level_increments) <= TIMER_MaxCount(led->timer));

  sl_pwm_led_stop(led);
  TIMER_CompareBufSet(led->timer, led->channel, led->level * level_increments);
#else
  // Get the counter increments per intensity level step
  uint32_t level_increments = sl_hal_timer_get_top(led->timer) / (led->resolution - 1);
  led->level = color;

  // Ensure that the color level value does not exceed the timer's size.
  EFM_ASSERT((led->level * level_increments) <= SL_HAL_TIMER_MAX_COUNT(led->timer));

  sl_pwm_led_stop(led);
  sl_hal_timer_channel_set_compare_buffer(led->timer, led->channel, led->level * level_increments);
#endif

  sl_pwm_led_start(led);
}

void sl_pwm_led_get_color(void *led_handler, uint16_t *color)
{
  sl_led_pwm_t *led = (sl_led_pwm_t *)led_handler;
  *color   = led->level;
}
