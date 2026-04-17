/******************************************************************************
* @file sl_si91x_pwm.c
* @brief PWM API implementation
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_si91x_pwm.h"
#include "rsi_rom_clks.h"
#include "sl_si91x_peripheral_gpio.h"
/*******************************************************************************
 ***************************  LOCAL MACROS   ***********************************
 ******************************************************************************/
#define OUTPUT                1     // Output value set
#define MAX_POLARITY          1     // Maximum polarity value
#define DEAD_TIME_MAX         1     // Dead time counter maximum value
#define ULP_PORT              4     // GPIO ULP port
#define ULP_MODE              6     // ULP GPIO mode
#define MAX_FLAG              15    // PWM maximum flag
#define HOST_MIN              24    // GPIO host pad minimum pin number
#define HOST_MAX              31    // GPIO host pad maximum pin number
#define MAX_DEAD_TIME         63    // dead time A/B max value
#define PWM_RELEASE_VERSION   0     // PWM Release version
#define PWM_SQA_VERSION       0     // PWM SQA version
#define PWM_DEV_VERSION       2     // PWM Developer version
#define MAX_GPIO              64    // maximum GPIO pins
#define MAX_COUNT_VALUE_16BIT 65535 // maximum count value for 16-bit base time period, time counter, duty cycle
#define MCPWM_IRQHANDLER      IRQ048_Handler // PWM IRQ handler
/*******************************************************************************
 ***************************  Global  VARIABLES ********************************
 ******************************************************************************/

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/
sl_si91x_pwm_callback_t callback;
uint16_t time_period = 0;
uint32_t ticks       = 0;
/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

/*******************************************************************************
 **************************   Global function Definitions   ********************
 ******************************************************************************/
void MCPWM_IRQHANDLER(void);

/*******************************************************************************
 * This API is used for initialize PWM pins and clock. The formal argument passed takes
 * pointer to structure of type \ref sl_pwm_init_t. This holds the GPIO port,pin, pad, mux
 * configuration. The members are assigned to SL macros defined in sl_pwm_board.h.
 * The SL macros are integrated to RTE macros present in RTE device file.
 ******************************************************************************/
sl_status_t sl_si91x_pwm_init(sl_pwm_init_t *pwm_init)
{
  sl_status_t status;
  do {
    // Validates the null pointer, if true returns error code
    if (pwm_init == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (pwm_init->pin_l >= MAX_GPIO) {
      sl_si91x_gpio_enable_ulp_pad_receiver((uint8_t)(pwm_init->pin_l - MAX_GPIO));
      sl_gpio_set_pin_mode(ULP_PORT, (uint8_t)(pwm_init->pin_l - MAX_GPIO), ULP_MODE, OUTPUT);
    } else {
      sl_si91x_gpio_enable_pad_receiver(pwm_init->pin_l);
    }
    if (pwm_init->pin_l >= HOST_MIN && pwm_init->pin_l <= HOST_MAX) {
      sl_si91x_gpio_enable_pad_selection(pwm_init->pin_l);
    } else {
      sl_si91x_gpio_enable_pad_selection(pwm_init->pad_l);
    }
    sl_gpio_set_pin_mode(pwm_init->port_l, pwm_init->pin_l, pwm_init->mux_l, OUTPUT);

    if (pwm_init->pin_h >= MAX_GPIO) {
      sl_si91x_gpio_enable_ulp_pad_receiver((uint8_t)(pwm_init->pin_h - MAX_GPIO));
      sl_gpio_set_pin_mode(ULP_PORT, (uint8_t)(pwm_init->pin_h - MAX_GPIO), ULP_MODE, OUTPUT);
    } else {
      sl_si91x_gpio_enable_pad_receiver(pwm_init->pin_h);
    }
    if (pwm_init->pin_l >= HOST_MIN && pwm_init->pin_l <= HOST_MAX) {
      sl_si91x_gpio_enable_pad_selection(pwm_init->pin_h);
    } else {
      sl_si91x_gpio_enable_pad_selection(pwm_init->pad_h);
    }
    sl_gpio_set_pin_mode(pwm_init->port_h, pwm_init->pin_h, pwm_init->mux_h, OUTPUT);

    // Initialization of PWM clock
    RSI_CLK_PeripheralClkEnable(M4CLK, PWM_CLK, ENABLE_STATIC_CLK);
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to De-initialize PWM peripheral
 ******************************************************************************/
void sl_si91x_pwm_deinit(void)
{
  callback.cbFunc = NULL;
  RSI_CLK_PeripheralClkDisable(M4CLK, PWM_CLK);
}

/*******************************************************************************
 * This API is used to get the release, SQA and DEV version of PWM
 * It returns the structure for PWM version.
 * Structure includes three members:
 * - Release version
 * - SQA version
 * - DEV version
 ******************************************************************************/
sl_pwm_version_t sl_si91x_pwm_get_version(void)
{
  sl_pwm_version_t version;
  version.minor   = PWM_DEV_VERSION;
  version.release = PWM_RELEASE_VERSION;
  version.major   = PWM_SQA_VERSION;
  return version;
}

/*******************************************************************************
 * This API is used to validate configuration parameters
 ******************************************************************************/
static sl_status_t validate_config_parameters(sl_pwm_config_t *pwm_config)
{
  sl_status_t status = 0;
  do {
    // Validates the null pointer, if true returns error code
    if (pwm_config == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validating PWM Channel value
    if (pwm_config->channel >= SL_CHANNEL_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validating PWM base timer mode value
    if (pwm_config->is_mode >= SL_MODE_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validating PWM output polarity low
    if (pwm_config->is_polarity_low >= SL_POLARITYL_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validating PWM output polarity high
    if (pwm_config->is_polarity_high >= SL_POLARITYH_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to set the PWM configuration parameters
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_set_configuration(sl_pwm_config_t *pwm_config)
{
  sl_status_t status;
  uint32_t rate = 0;
  rate          = (SystemCoreClock / pwm_config->frequency);
  do {
    status = validate_config_parameters(pwm_config);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Set output polarity
    status = sl_si91x_pwm_set_output_polarity(pwm_config->is_polarity_low, pwm_config->is_polarity_high);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Set time period
    status = sl_si91x_pwm_set_time_period(pwm_config->channel, rate, pwm_config->base_time_counter_initial_value);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Set output mode
    status = sl_si91x_pwm_set_output_mode(pwm_config->is_mode, pwm_config->channel);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Get time period
    status = sl_si91x_pwm_get_time_period(pwm_config->channel, &time_period);
    if (status != SL_STATUS_OK) {
      break;
    }
    ticks = (uint32_t)((time_period * pwm_config->duty_cycle) / 100);
    // Set Duty cycle value for channel
    status = sl_si91x_pwm_set_duty_cycle(ticks, pwm_config->channel);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Set the base timer mode as TMR_FREE_RUN_MODE
    status = sl_si91x_pwm_set_base_timer_mode(pwm_config->base_timer_mode, pwm_config->channel);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Set base timer selection for channel
    status = sl_si91x_pwm_control_base_timer(pwm_config->channel_timer_selection);
    if (status != SL_STATUS_OK) {
      break;
    }
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to set output polarity for MCPWM
 * The actions to be performed are:
 *      - Enable PWM clock using @ref sl_si91x_pwm_enable_clock()\n
 *      - Select base timer for PWM channel @ref sl_si91x_pwm_control_base_timer()\n
 ******************************************************************************/
sl_status_t sl_si91x_pwm_set_output_polarity(boolean_t polarity_low, boolean_t polarity_high)
{
  sl_status_t status;
  if ((polarity_low > MAX_POLARITY) || (polarity_high > MAX_POLARITY)) {
    // Returns invalid parameter status code polarity low and high > 1
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    RSI_MCPWM_SetOutputPolarity(MCPWM, polarity_low, polarity_high);
    status = SL_STATUS_OK; // Returns status error code
  }
  return status;
}

/*******************************************************************************
 * This API is used to start the MCPWM operation for required channel.
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_start(sl_pwm_channel_t channel)
{
  sl_status_t status;
  rsi_error_t error_status;
  // Start PWM
  if (channel >= SL_CHANNEL_LAST) {
    // Returns invalid parameter status code if channel >=4
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    error_status = sl_si91x_pwm_reset_counter_disable(channel);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status error code
    } else {
      status = SL_STATUS_OK; // Returns status OK if no error occurs
    }
    error_status = RSI_MCPWM_Start(MCPWM, channel);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status error code
    } else {
      status = SL_STATUS_OK; // Returns status OK if no error occurs
    }
  }
  return status;
}

/*******************************************************************************
 * This API is used to stop the MCPWM operation for required channel
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()\n
 *      - Start PWM using @ref sl_si91x_pwm_start()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_stop(sl_pwm_channel_t channel)
{
  sl_status_t status;
  rsi_error_t error_status;
  // Stop PWM
  if (channel >= SL_CHANNEL_LAST) {
    // Returns invalid parameter status code if channel >= 4
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    error_status = sl_si91x_pwm_reset_counter(channel);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; //  Returns status error code
    } else {
      status = SL_STATUS_OK; // Returns status OK if no error occurs
    }
    error_status = RSI_MCPWM_Stop(MCPWM, channel);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status OK if no error occurs
    } else {
      status = SL_STATUS_OK; // Returns status error code
    }
  }
  return status;
}

/*******************************************************************************
 * This API is used to select number of base timers as four base timers for
 *    four channels or one base timer for all channels of MCPWM
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_control_base_timer(sl_pwm_timer_t base_timer)
{
  sl_status_t status;
  do {
    if (base_timer >= SL_BASE_TIMER_LAST) {
      // Returns invalid parameter status code if base_timer >= 2
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    RSI_MCPWM_BaseTimerSelect(MCPWM, base_timer);
    status = SL_STATUS_OK; // Returns status OK if no error occurs
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to set time period and counter initial, value for the required MCPWM channel.
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_set_time_period(sl_pwm_channel_t channel, uint32_t period, uint32_t init_val)
{
  sl_status_t status;
  rsi_error_t error_status;
  do {
    if (channel >= SL_CHANNEL_LAST) {
      // Returns invalid parameter status code if channel >= 4
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if ((period > MAX_COUNT_VALUE_16BIT) || (init_val > MAX_COUNT_VALUE_16BIT)) {
      // Returns invalid parameter status code if period, init_val > 65535
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    error_status = RSI_MCPWM_SetTimePeriod(MCPWM, channel, (uint16_t)period, (uint16_t)init_val);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status OK if no error occurs
    }
    status = SL_STATUS_OK; // Returns status error code
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to configure special event trigger generation for required MCPWM channel which
 *    allows the A/D converter to be synchronized to the PWM time base.
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_trigger_special_event(sl_pwm_svt_t direction, sl_si91x_pwm_svt_config_t *pwm_config)
{
  sl_status_t status;
  do {
    // Validates the null pointer, if true returns error code
    if (pwm_config == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (direction >= SL_SVT_COUNT_LAST) {
      // Returns invalid parameter status code if direction >= 2
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    RSI_MCPWM_SpecialEventTriggerConfig(MCPWM, direction, pwm_config);
    status = SL_STATUS_OK; // Returns status OK if no error occurs
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to Configure Dead time insertion parameters for MCPWM
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_configure_dead_time(sl_si91x_pwm_dt_config_t *dead_time, sl_pwm_channel_t channel)
{
  sl_status_t status;
  rsi_error_t error_status;
  do {
    // Validates the null pointer, if true returns error code
    if (dead_time == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (channel >= SL_CHANNEL_LAST) {
      // Returns invalid parameter status code if channel >= 4
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if ((dead_time->counterSelect > DEAD_TIME_MAX) || (dead_time->deadTimeA > MAX_DEAD_TIME)
        || (dead_time->deadTimeB > MAX_DEAD_TIME)) {
      // Returns invalid parameter status code
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    error_status = RSI_MCPWM_DeadTimeValueSet(MCPWM, dead_time, channel);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status OK if no error occurs
      break;
    }
    status = SL_STATUS_OK; // Returns status error code
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to reset the required channel of MCPWM.
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_reset_channel(sl_pwm_channel_t channel)
{
  sl_status_t status;
  rsi_error_t error_status;
  if (channel >= SL_CHANNEL_LAST) {
    // Returns invalid parameter status code if channel >= 4
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    error_status = RSI_MCPWM_ChannelReset(MCPWM, channel);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status OK if no error occurs
    } else {
      status = SL_STATUS_OK; // Returns status error code
    }
  }
  return status;
}

/*******************************************************************************
 * This API is used to reset the counter from required channel of MCPWM
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_reset_counter(sl_pwm_channel_t channel)
{
  sl_status_t status;
  rsi_error_t error_status;
  if (channel >= SL_CHANNEL_LAST) {
    // Returns invalid parameter status code if channel >= 4
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    error_status = RSI_MCPWM_CounterReset(MCPWM, channel);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status OK if no error occurs
    } else {
      status = SL_STATUS_OK; // Returns status error code
    }
  }
  return status;
}

/*******************************************************************************
 * This API is used to set base time period control for the required MCPWM channel.
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_control_period(sl_pwm_post_t post_scale, sl_pwm_pre_t pre_scale, sl_pwm_channel_t channel)
{
  sl_status_t status;
  rsi_error_t error_status;
  if ((channel >= SL_CHANNEL_LAST) || (post_scale >= SL_TIME_PERIOD_POSTSCALE_1_LAST)
      || (pre_scale >= SL_TIME_PERIOD_PRESCALE_LAST)) {
    // Returns invalid parameter status code if post_scale >= 16, pre_scale >= 7, channel >= 4
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    error_status = RSI_MCPWM_PeriodControlConfig(MCPWM, post_scale, pre_scale, channel);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status OK if no error occurs
    } else {
      status = SL_STATUS_OK; // Returns status error code
    }
  }
  return status;
}

/*******************************************************************************
 * This API is used to control fault A/B pin output value to be overridden when fault condition occurs.
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_control_fault(sl_pwm_fault_t fault, sl_pwm_output_t pwm_output, sl_pwm_override_value_t value)
{
  sl_status_t status;
  rsi_error_t error_status;
  do {
    if ((pwm_output >= SL_OUTPUT_LAST) || (value >= SL_OVERRIDE_VALUE_LAST) || (fault >= SL_FAULT_LAST)) {
      // Returns invalid parameter status code if fault value >= 2, pwm_output >= 8, value >= 2
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (fault == SL_FAULTA) {
      error_status = RSI_MCPWM_FaultAValueSet(MCPWM, pwm_output, value);
      if (error_status != RSI_OK) {
        status = SL_STATUS_FAIL; // Returns status error code
        break;
      }
      status = SL_STATUS_OK; // Returns status OK if no error occurs
      break;
    }
    error_status = RSI_MCPWM_FaultBValueSet(MCPWM, pwm_output, value);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status error code
      break;
    }
    status = SL_STATUS_OK; // Returns status OK if no error occurs
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to set the mode of base timer for required channel
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_set_base_timer_mode(sl_pwm_base_timer_mode_t mode, sl_pwm_channel_t channel)
{
  sl_status_t status;
  rsi_error_t error_status;
  if ((channel >= SL_CHANNEL_LAST) || (mode >= SL_BASE_TIMER_MODE_LAST)) {
    // Returns invalid parameter status code if channel >= 4, mode >= 6
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    error_status = RSI_MCPWM_SetBaseTimerMode(MCPWM, mode, channel);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status OK if no error occurs
    } else {
      status = SL_STATUS_OK; // Returns status error code
    }
  }
  return status;
}

/*******************************************************************************
 * This API is used to set output mode for the MCPWM
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_set_output_mode(sl_pwm_mode_t mode, sl_pwm_channel_t channel)
{
  sl_status_t status;
  rsi_error_t error_status;
  if ((channel >= SL_CHANNEL_LAST) || (mode >= SL_MODE_LAST)) {
    // Returns invalid parameter status code channel >= 4, mode >= 2
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    error_status = RSI_MCPWM_SetOutputMode(MCPWM, mode, channel);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status OK if no error occurs
    } else {
      status = SL_STATUS_OK; // Returns status error code
    }
  }
  return status;
}

/*******************************************************************************
 * This API is used to handle all interrupt flags of MCPWM.
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_register_callback(sl_si91x_pwm_callback_t *callback_event, uint16_t flag)
{
  sl_status_t status;
  do {
    if (callback_event == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    callback.cbFunc = NULL;
    NVIC_EnableIRQ(MCPWM_IRQn);
    callback.cbFunc = callback_event->cbFunc;
    RSI_MCPWM_InterruptHandler(MCPWM, callback_event);
    // Enabling timer interrupt
    RSI_MCPWM_InterruptEnable(MCPWM, flag);
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to disable the interrupts of MCPWM
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()\n
 *      - Register PWM callback using @ref sl_si91x_pwm_register_timeout_callback()\n
 *      - Start PWM using @ref sl_si91x_pwm_start()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_unregister_callback(uint16_t flag)
{
  sl_status_t status;
  // Disabling passed timer instance interrupt
  RSI_MCPWM_InterruptDisable(MCPWM, flag);
  status = SL_STATUS_OK;
  return status;
}

/*******************************************************************************
 * This API is used to read the counter current value
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()\n
 *      - Start PWM using @ref sl_si91x_pwm_start()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_read_counter(uint16_t *counter_value, sl_pwm_channel_t channel)
{
  sl_status_t status;
  rsi_error_t error_status;
  do {
    if (counter_value == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (channel >= SL_CHANNEL_LAST) {
      // Returns invalid parameter status code if channel >= 4
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    error_status = RSI_MCPWM_ReadCounter(MCPWM, counter_value, channel);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status error code
    }
    status = SL_STATUS_OK; // Returns status OK if no error occurs
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to get time period counter direction status of required MCPWM channel
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()\n
 *      - Start PWM using @ref sl_si91x_pwm_start()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_get_counter_direction(uint8_t *counter_direction, sl_pwm_channel_t channel)
{
  sl_status_t status;
  rsi_error_t error_status;
  do {
    if (counter_direction == NULL) {
      // Validating the null pointer
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (channel >= SL_CHANNEL_LAST) {
      // Returns invalid parameter status code if channel >= 4
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    error_status = RSI_MCPWM_GetCounterDir(MCPWM, counter_direction, channel);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status error code
    }
    status = SL_STATUS_OK; // Returns status OK if no error occurs
  } while (false);
  return status;
}

/*******************************************************************************
 * This API control dead time insertion at rise edge (or) fall edge of any four channels
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_control_dead_time(sl_pwm_dead_time_t dead_time, uint32_t flag)
{
  sl_status_t status;
  do {
    if (dead_time >= SL_DEAD_TIME_LAST) {
      // Returns invalid parameter status code if dead_time >= 2
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (dead_time == SL_DEAD_TIME_ENABLE) {
      RSI_MCPWM_Dead_Time_Enable(MCPWM, flag);
      status = SL_STATUS_OK; // Returns status OK if no error occurs
      break;
    }
    RSI_MCPWM_Dead_Time_Disable(MCPWM, flag);
    status = SL_STATUS_OK; // Returns status OK if no error occurs
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to clear the interrupts of MCPWM
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()\n
 *      - Register PWM callback using @ref sl_si91x_pwm_register_timeout_callback()\n
 *      - Start PWM using @ref sl_si91x_pwm_start()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_clear_interrupt(uint32_t flag)
{
  sl_status_t status;
  RSI_MCPWM_InterruptClear(MCPWM, flag);
  status = SL_STATUS_OK; // Returns status OK if no error occurs
  return status;
}

/*******************************************************************************
 * This API is used to get the interrupt status of interrupt flags of MCPWM.
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()\n
 *      - Register PWM callback using @ref sl_si91x_pwm_register_timeout_callback()\n
 *      - Start PWM using @ref sl_si91x_pwm_start()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_get_interrupt_status(uint32_t flag, uint16_t *intr_status)
{
  sl_status_t status;
  do {
    if (intr_status == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (flag > MAX_COUNT_VALUE_16BIT) {
      // Returns invalid parameter status code if period, flag > 65535
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Reading interrupt status, and store the return value to intr_status
    *intr_status = RSI_PWM_GetInterruptStatus(MCPWM, (uint16_t)flag);
    status       = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to set duty cycle control parameters for the required MCPWM channel
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_configure_duty_cycle(sl_pwm_duty_cycle_t duty_cycle, uint32_t value, sl_pwm_channel_t channel)
{
  sl_status_t status;
  rsi_error_t error_status;
  do {
    if ((channel >= SL_CHANNEL_LAST) || (duty_cycle >= SL_DUTY_CYCLE_LAST)) {
      // Returns invalid parameter status code if channel >= 4, duty_cycle >= 2
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Evaluating duty cycle control parameters
    if (duty_cycle == SL_DUTY_CYCLE_ENABLE) {
      error_status = RSI_MCPWM_DutyCycleControlSet(MCPWM, value, channel);
      if (error_status != RSI_OK) {
        status = SL_STATUS_FAIL; // Returns status error code
        break;
      }
      status = SL_STATUS_OK; // Returns status OK if no error occurs
      break;
    }
    error_status = RSI_MCPWM_DutyCycleControlReset(MCPWM, value, channel);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status error code
      break;
    }
    status = SL_STATUS_OK; // Returns status OK if no error occurs
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to enable the output override operation of MCPWM
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_output_override(sl_pwm_override_t override, sl_pwm_output_t pwm_output)
{
  sl_status_t status;
  rsi_error_t error_status;
  do {
    if ((pwm_output >= SL_OUTPUT_LAST) || (override >= SL_OVERRIDE_LAST)) {
      // Returns invalid parameter status code if pwm_output >= 8, override >= 2
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (override == SL_OVERRIDE_SET) {
      error_status = RSI_MCPWM_OutputOverrideEnable(MCPWM, pwm_output);
      if (error_status != RSI_OK) {
        status = SL_STATUS_FAIL; // Returns status error code
        break;
      }
      status = SL_STATUS_OK; // Returns status OK if no error occurs
      break;
    }
    error_status = RSI_MCPWM_OutputOverrideDisable(MCPWM, pwm_output);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status error code
      break;
    }
    status = SL_STATUS_OK; // Returns status OK if no error occurs
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to set the override control parameter,output is in sync with pwm time period
 *     depending on operating mode
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_control_override(sl_pwm_override_t override, sl_pwm_output_override_t value)
{
  sl_status_t status;
  do {
    if ((value >= SL_OP_OVERRIDE_LAST) || (override >= SL_OVERRIDE_LAST)) {
      // Returns invalid parameter status code if output override >= 2, value >= 2
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (override == SL_OVERRIDE_SET) {
      RSI_MCPWM_OverrideControlSet(MCPWM, value);
      status = SL_STATUS_OK;
      break;
    }
    RSI_MCPWM_OverrideControlReSet(MCPWM, value);
    status = SL_STATUS_OK; // Returns status OK if no error occurs
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to set override value for the required output of MCPWM.
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_control_override_value(sl_pwm_override_t override,
                                                sl_pwm_output_t pwm_output,
                                                sl_pwm_override_value_t value)
{
  sl_status_t status;
  rsi_error_t error_status;
  do {
    if ((pwm_output >= SL_OUTPUT_LAST) || (value >= SL_OVERRIDE_VALUE_LAST) || (override >= SL_OVERRIDE_LAST)) {
      // Returns invalid parameter status code if pwm_output >= 8, override >= 2, value >= 2
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (override == SL_OVERRIDE_SET) {
      error_status = RSI_MCPWM_OverrideValueSet(MCPWM, pwm_output, value);
      if (error_status != RSI_OK) {
        status = SL_STATUS_FAIL; // Returns status error code
        break;
      }
      status = SL_STATUS_OK; // Returns status OK if no error occurs
      break;
    }
    error_status = RSI_MCPWM_OverrideValueReSet(MCPWM, pwm_output, value);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status error code
      break;
    }
    status = SL_STATUS_OK; // Returns status OK if no error occurs
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to set output fault override control parameters for required PWM channel
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_control_output_fault(sl_pwm_output_fault_t output_fault, uint32_t value)
{
  sl_status_t status;
  do {
    if (output_fault >= SL_OUTPUT_FAULT_LAST) {
      // Returns invalid parameter status code if output_fault >= 2
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (output_fault == SL_OUTPUT_FAULT_SET) {
      RSI_MCPWM_FaultControlSet(MCPWM, value);
      status = SL_STATUS_OK;
      break;
    }
    RSI_MCPWM_FaultControlReSet(MCPWM, value);
    status = SL_STATUS_OK; // Returns status OK if no error occurs
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to enable generation of special event trigger for required channel of MCPWM
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_control_special_event_trigger(sl_pwm_event_t event)
{
  sl_status_t status;
  do {
    if (event >= SL_EVENT_LAST) {
      // Returns invalid parameter status code if event >= 2
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (event == SL_EVENT_ENABLE) {
      RSI_MCPWM_SpecialEventTriggerEnable(MCPWM);
      status = SL_STATUS_OK;
      break;
    }
    RSI_MCPWM_SpecialEventTriggerDisable(MCPWM);
    status = SL_STATUS_OK; // Returns status OK if no error occurs
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to set dead time control parameters for the required channel.
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_select_dead_time(sl_pwm_dead_time_t dead_time, uint32_t value)
{
  sl_status_t status;
  do {
    if (dead_time >= SL_DEAD_TIME_LAST) {
      // Returns invalid parameter status code if dead_time >= 2
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (dead_time == SL_DEAD_TIME_ENABLE) {
      RSI_MCPWM_DeadTimeControlSet(MCPWM, value);
      MCPWM->PWM_FLT_OVERRIDE_CTRL_SET_REG_b.COMPLEMENTARY_MODE = true; // Set complementary mode for dead time control
      status                                                    = SL_STATUS_OK;
      break;
    }
    RSI_MCPWM_DeadTimeControlReSet(MCPWM, value);
    MCPWM->PWM_FLT_OVERRIDE_CTRL_SET_REG_b.COMPLEMENTARY_MODE = false; // Reset complementary mode for dead time control
    status                                                    = SL_STATUS_OK; // Returns status OK if no error occurs
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to set duty cycle for the required MCPWM channel.
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_set_duty_cycle(uint32_t duty_cycle, sl_pwm_channel_t channel)
{
  sl_status_t status;
  rsi_error_t error_status;
  do {
    if (channel >= SL_CHANNEL_LAST) {
      // Returns invalid parameter status code if channel >= 4
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (duty_cycle > MAX_COUNT_VALUE_16BIT) {
      // Returns invalid parameter status code
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    error_status = RSI_MCPWM_SetDutyCycle(MCPWM, (uint16_t)duty_cycle, channel);
    if (error_status == RSI_OK) {
      status = SL_STATUS_OK; // Returns status OK if no error occurs
    } else {
      status = SL_STATUS_FAIL; // Returns status error code
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to get the duty cycle
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_get_duty_cycle(sl_pwm_channel_t channel, uint32_t *duty_cycle)
{
  sl_status_t status;
  do {
    if (duty_cycle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (channel >= SL_CHANNEL_LAST) {
      // Returns invalid parameter status code if channel >= 4
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    *duty_cycle = MCPWM->PWM_DUTYCYCLE_REG_WR_VALUE_b[channel].PWM_DUTYCYCLE_REG_WR_VALUE_CH;
    status      = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to enable to use external trigger for base time counter
 *    increment or decrement of MCPWM
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_enable_external_trigger(sl_pwm_trigger_t enable)
{
  sl_status_t status;
  if (enable >= SL_TRIGGER_LAST) {
    // Returns invalid parameter status code if enable >= 2
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    RSI_MCPWM_ExternalTriggerControl(MCPWM, enable);
    status = SL_STATUS_OK; // Returns status OK if no error occurs
  }
  return status;
}

/*******************************************************************************
 * This API is used to get time period for required channel.
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()
 ******************************************************************************/
sl_status_t sl_si91x_pwm_get_time_period(sl_pwm_channel_t channel, uint16_t *period)
{
  sl_status_t status;
  rsi_error_t error_status;
  do {
    // Validates the null pointer, if true returns error code
    if (period == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (channel >= SL_CHANNEL_LAST) {
      // Returns invalid parameter status code if channel > 3
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    error_status = RSI_MCPWM_GetTimePeriod(MCPWM, channel, period);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status error code
      break;
    }
    status = SL_STATUS_OK; // Returns status OK if no error occurs
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used for initialize PWM event pins
 ******************************************************************************/
sl_status_t sl_si91x_pwm_fault_init(sl_pwm_fault_init_t *pwm_fault)
{
  sl_status_t status;
  do {
    // Validates the null pointer, if true returns error code
    if (pwm_fault == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (pwm_fault->pin >= MAX_GPIO) {
      sl_si91x_gpio_enable_ulp_pad_receiver((uint8_t)(pwm_fault->pin - MAX_GPIO));
      sl_gpio_set_pin_mode(ULP_PORT, (uint8_t)(pwm_fault->pin - MAX_GPIO), ULP_MODE, OUTPUT);
    } else {
      sl_si91x_gpio_enable_pad_receiver(pwm_fault->pin);
    }
    if (pwm_fault->pin >= HOST_MIN && pwm_fault->pin <= HOST_MAX) {
      sl_si91x_gpio_enable_pad_selection(pwm_fault->pin);
    } else {
      sl_si91x_gpio_enable_pad_selection(pwm_fault->pad);
    }
    sl_gpio_set_pin_mode(pwm_fault->port, pwm_fault->pin, pwm_fault->mux, OUTPUT);
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to disable the reset for required channel of MCPWM.
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()\n
 *      - Start PWM using @ref sl_si91x_pwm_start()\n
 ******************************************************************************/
sl_status_t sl_si91x_pwm_reset_channel_disable(sl_pwm_channel_t channel)
{
  sl_status_t status;
  rsi_error_t error_status;
  if (channel >= SL_CHANNEL_LAST) {
    // Returns invalid parameter status code if channel >= 4
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    error_status = RSI_PWM_Channel_Reset_Disable(MCPWM, channel);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status error code
    } else {
      status = SL_STATUS_OK; // Returns status OK if no error occurs
    }
  }
  return status;
}

/*******************************************************************************
 * This API is used to disable the counter reset for required channel of MCPWM
 * The actions to be performed are:
 *      - Initialize PWM using @ref sl_si91x_pwm_init()\n
 *      - Set configuration for PWM @ref sl_si91x_pwm_set_configuration()\n
 *      - Set base timer mode using @ref sl_si91x_pwm_set_base_timer_mode()\n
 *      - Start PWM using @ref sl_si91x_pwm_start()\n
 ******************************************************************************/
sl_status_t sl_si91x_pwm_reset_counter_disable(sl_pwm_channel_t channel)
{
  sl_status_t status;
  rsi_error_t error_status;
  if (channel >= SL_CHANNEL_LAST) {
    // Returns invalid parameter status code if channel >= 4
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    error_status = RSI_PWM_Counter_Reset_Disable(MCPWM, channel);
    if (error_status != RSI_OK) {
      status = SL_STATUS_FAIL; // Returns status error code
    } else {
      status = SL_STATUS_OK; // Returns status OK if no error occurs
    }
  }
  return status;
}

/*******************************************************************************
 * This API is used for PWM interrupt handler
 ******************************************************************************/
void MCPWM_IRQHANDLER(void)
{
  RSI_MCPWM_InterruptHandler(MCPWM, &callback);
}
