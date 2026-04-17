/******************************************************************************
* @file sl_si91x_sysrtc.c
* @brief Sysrtc API implementation
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
#include "sl_si91x_sysrtc.h"

/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
#define SYSRTC_IRQHandler      IRQ022_Handler // SYSRTC IRQ Handler
#define MAX_DIVISION_FACTOR    33             // maximum value for division factor
#define SYSRTC_RELEASE_VERSION 0              ///< Config-timer Release version
#define SYSRTC_MAJOR_VERSION   0              ///< Config-timer SQA version
#define SYSRTC_MINOR_VERSION   1              ///< Config-timer Developer version
/*******************************************************************************
 ***************************  Local TYPES  ********************************
 ******************************************************************************/

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static sl_sysrtc_callback_t callback_function_ptr = NULL;
static void *callback_flags                       = NULL;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static sl_status_t sli_si91x_sysrtc_compare_gpio_config(sl_sysrtc_group_number_t group_number,
                                                        sl_sysrtc_channel_number_t channel,
                                                        sl_si91x_gpio_pin_config_t sysrtc_prs_gpio);
static sl_status_t sli_si91x_sysrtc_gpio_capture_config(sl_sysrtc_group_number_t group_number,
                                                        sl_si91x_gpio_pin_config_t sysrtc_prs_gpio);

/*******************************************************************************
 **********************  Local Function Definition****************************
 ******************************************************************************/

/*******************************************************************************
***********************  Global function Definitions *************************
 ******************************************************************************/

/***************************************************************************/ /**
* @brief:Configures sysrtc parameters

* @details:Initializing and enabling sysrtc modules
*******************************************************************************/
sl_status_t sl_si91x_sysrtc_init(const sl_sysrtc_config_t *config_ptr)
{
  sl_status_t status;
  // To validate the structure pointer, if the parameters is NULL, it
  // will return an error code
  if (config_ptr == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    // initializing sysrtc module
    rsi_sysrtc_init(config_ptr);
    status = SL_STATUS_OK;
  }
  // Enabling sysrtc module
  rsi_sysrtc_enable();
  return status;
}

/*******************************************************************************
* @brief: Configures sysrtc clock sources

*******************************************************************************/
sl_status_t sl_si91x_sysrtc_configure_clock(sl_sysrtc_clock_config_t *clk_ptr)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    // To validate the structure pointer, if the parameters is NULL, it
    // will return an error code
    if (clk_ptr == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if ((clk_ptr->clock_source != RSI_SYSRTC_CLK_1kHz_clk) && (clk_ptr->clock_source != RSI_SYSRTC_CLK_32KHz_RO)
        && (clk_ptr->clock_source != RSI_SYSRTC_CLK_32kHz_RC) && (clk_ptr->clock_source != RSI_SYSRTC_CLK_32kHz_Xtal)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if ((clk_ptr->division_factor) >= MAX_DIVISION_FACTOR) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // configuring sysrtc clock source
    rsi_sysrtc_clk_set(clk_ptr->clock_source, clk_ptr->division_factor);
  } while (false);
  return status;
}

/*******************************************************************************
 * To configure SYSRTC compare and capture channel's groups, configures group number
 * and enables its compare & capture channels. Also Configures match out actions
 * for respective compare channel
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_configure_group(sl_sysrtc_group_number_t group_number,
                                            sl_sysrtc_group_config_t const *config_ptr)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    // To validate the structure pointer, if the parameters is NULL, it
    // will return an error code
    if (config_ptr == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (group_number >= SL_SYSRTC_GROUP_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // initializing group as per group number
    rsi_sysrtc_init_group(group_number, config_ptr);
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Registers callback of sysrtc interrupt and enabling respective interrupts as
*        per selected interrupt and group

*******************************************************************************/
sl_status_t sl_si91x_sysrtc_register_callback(sl_sysrtc_callback_t sl_sysrtc_callback,
                                              void *callback_flag_value,
                                              sl_sysrtc_group_number_t group_number,
                                              sl_sysrtc_interrupt_enables_t *interrupt_enable_ptr)
{
  sl_status_t status;
  status                               = SL_STATUS_OK;
  uint32_t ored_interrupt_enable_value = 0;
  do {
    // Validating the 'callback function pointer' parameter and pointer to interrupt_flag,
    // if they are NULL will return an error code
    if ((interrupt_enable_ptr == NULL) || (sl_sysrtc_callback == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // To validate the function pointer, if the parameters is not NULL then it
    // will return an busy error code
    if (callback_function_ptr != NULL) {
      status = SL_STATUS_BUSY;
      break;
    }
    if (group_number >= SL_SYSRTC_GROUP_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (group_number == SL_SYSRTC_GROUP_0) {
      // Disable & clear group 0 interrupts
      rsi_sysrtc_disable_group_interrupts(group_number, _SYSRTC_GRP0_IEN_MASK);
      rsi_sysrtc_clear_group_interrupts(group_number, _SYSRTC_GRP0_IF_MASK);
    }
    if (group_number == SL_SYSRTC_GROUP_1) {
      // Disable & clear group 1 interrupts
      rsi_sysrtc_disable_group_interrupts(group_number, _SYSRTC_GRP1_IEN_MASK);
      rsi_sysrtc_clear_group_interrupts(group_number, _SYSRTC_GRP1_IF_MASK);
    }
    // Calculating ored interrupt flag value
    if (interrupt_enable_ptr->group0_overflow_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP0_IEN_OVF;
    }
    if (interrupt_enable_ptr->group0_compare0_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP0_IEN_CMP0;
    }
    if (interrupt_enable_ptr->group0_compare1_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP0_IEN_CMP1;
    }
    if (interrupt_enable_ptr->group0_capture0_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP0_IEN_CAP0;
    }
    if (interrupt_enable_ptr->group1_overflow_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP1_IEN_OVF;
    }
    if (interrupt_enable_ptr->group1_compare0_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP1_IEN_CMP0;
    }
    if (interrupt_enable_ptr->group1_compare1_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP1_IEN_CMP1;
    }
    if (interrupt_enable_ptr->group1_capture0_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP1_IEN_CAP0;
    }
    // Enabling Interrupt as per flags
    rsi_sysrtc_enable_group_interrupts((uint8_t)group_number, ored_interrupt_enable_value);
    // Un-masking interrupts & Enabling NVIC
    rsi_sysrtc_enable_interrupts();
    // The function pointer is fed to the static variable, which will be called
    // in the IRQ handler
    callback_function_ptr = sl_sysrtc_callback;
    callback_flags        = callback_flag_value;
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Unregisters Callback on sysrtc expiration and disables interrupts

* @details:
* It will unregister the callback, i.e., clear the callback function address
  and pass NULL value to the variable
* It will also mask and disable the timer interrupt.
*******************************************************************************/
sl_status_t sl_si91x_sysrtc_unregister_callback(sl_sysrtc_group_number_t group_number,
                                                sl_sysrtc_interrupt_enables_t *interrupt_enable_ptr)
{
  sl_status_t status;
  uint32_t ored_interrupt_enable_value = 0;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating the 'callback function pointer' parameter and pointer to interrupt_flag,
    // if they are NULL will return an error code
    if (interrupt_enable_ptr == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (group_number >= SL_SYSRTC_GROUP_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Calculating ored interrupt flag value
    if (interrupt_enable_ptr->group0_overflow_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP0_IEN_OVF;
    }
    if (interrupt_enable_ptr->group0_compare0_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP0_IEN_CMP0;
    }
    if (interrupt_enable_ptr->group0_compare1_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP0_IEN_CMP1;
    }
    if (interrupt_enable_ptr->group0_capture0_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP0_IEN_CAP0;
    }
    if (interrupt_enable_ptr->group1_overflow_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP1_IEN_OVF;
    }
    if (interrupt_enable_ptr->group1_compare0_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP1_IEN_CMP0;
    }
    if (interrupt_enable_ptr->group1_compare1_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP1_IEN_CMP1;
    }
    if (interrupt_enable_ptr->group1_capture0_interrupt_is_enabled) {
      ored_interrupt_enable_value |= SYSRTC_GRP1_IEN_CAP0;
    }
    // Disabling passed timer instance interrupt
    rsi_sysrtc_disable_group_interrupts((uint8_t)group_number, ored_interrupt_enable_value);
    rsi_sysrtc_disable_interrupts();
    //The callback should be null in the unregister callback api because
    //one cannot register the callback if it is not null
    callback_function_ptr = NULL;
    callback_flags        = NULL;
  } while (false);
  return status;
}

/*******************************************************************************
 * Sets SYSRTC compare register value for selected channel of given group.
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_set_compare_value(sl_sysrtc_group_number_t group_number,
                                              sl_sysrtc_channel_number_t channel,
                                              uint32_t compare_value)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    if (group_number >= SL_SYSRTC_GROUP_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (channel >= SL_SYSRTC_CHANNEL_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    rsi_sysrtc_set_compare_value((uint8_t)group_number, (uint8_t)channel, compare_value);
  } while (false);
  return status;
}

/*******************************************************************************
 * Gets SYSRTC compare register value for selected channel of given group.
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_get_compare_value(sl_sysrtc_group_number_t group_number,
                                              sl_sysrtc_channel_number_t channel,
                                              uint32_t *compare_value)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    // To validate the pointer, if the parameters is NULL, it
    // will return an error code
    if (compare_value == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (group_number >= SL_SYSRTC_GROUP_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (channel >= SL_SYSRTC_CHANNEL_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // reading group compare value
    *compare_value = rsi_sysrtc_get_group_compare_channel_value((uint8_t)group_number, (uint8_t)channel);
  } while (false);
  return status;
}

/*******************************************************************************
 * Sets register input high for capture channel of respective group of SYSRTC
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_sets_register_capture_input(sl_sysrtc_group_number_t group_number)
{
  sl_status_t status;
  if (group_number >= SL_SYSRTC_GROUP_LAST) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    rsi_sysrtc_set_capture_reg((const uint32_t)group_number);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * Configures SYSRTC Input(Capture) gpio pins
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_set_gpio_as_capture_input(sl_sysrtc_group_number_t group_number)
{
  sl_status_t status;
  if (group_number >= SL_SYSRTC_GROUP_LAST) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    rsi_sysrtc_set_capture_gpio((const uint32_t)group_number);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * Sets SYSRTC compare output gpio
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_set_compare_output_gpio(sl_sysrtc_group_number_t group_number,
                                                    sl_sysrtc_channel_number_t channel)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    if (group_number >= SL_SYSRTC_GROUP_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (channel >= SL_SYSRTC_CHANNEL_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // reading group compare value
    rsi_sysrtc_set_compare_gpio((const uint32_t)group_number, (const uint32_t)channel);
  } while (false);
  return status;
}

/*******************************************************************************
 * Configures SYSRTC capture input PRS_IN through gpio
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_set_capture_input_prs_gpio(sl_sysrtc_group_number_t group_number,
                                                       sl_si91x_gpio_pin_config_t sysrtc_prs_gpio)
{
  sl_status_t status = SL_STATUS_OK;
  if (group_number >= SL_SYSRTC_GROUP_LAST) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else
    status = sli_si91x_sysrtc_gpio_capture_config(group_number, sysrtc_prs_gpio);

  return status;
}

/*******************************************************************************
 * configures SYSRTC compare output PRS_OUT through gpio
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_set_compare_output_prs_gpio(sl_sysrtc_group_number_t group_number,
                                                        sl_sysrtc_channel_number_t channel,
                                                        sl_si91x_gpio_pin_config_t sysrtc_prs_gpio)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    if (group_number >= SL_SYSRTC_GROUP_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (channel >= SL_SYSRTC_CHANNEL_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // reading group compare value
    status = sli_si91x_sysrtc_compare_gpio_config(group_number, channel, sysrtc_prs_gpio);

  } while (false);
  return status;
}
/*******************************************************************************
 * Sets GPIO configuration for PRS_OUT according to group number and channel number
 ******************************************************************************/
static sl_status_t sli_si91x_sysrtc_compare_gpio_config(sl_sysrtc_group_number_t group_number,
                                                        sl_sysrtc_channel_number_t channel,
                                                        sl_si91x_gpio_pin_config_t sysrtc_prs_gpio)
{
  sl_si91x_uulp_npss_mode_t mux_mode;
  sl_status_t status = sl_gpio_driver_init(); // Initialize the GPIO driver
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  // Configure GPIO pins for SYSRTC Pin config.
  status = sl_gpio_set_configuration(sysrtc_prs_gpio);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  if (group_number == SL_SYSRTC_GROUP_0) {

    if (channel == SL_SYSRTC_CHANNEL_0) {
      mux_mode = NPSS_GPIO_PIN_MUX_MODE3;
    } else if (channel == SL_SYSRTC_CHANNEL_1) {
      mux_mode = NPSS_GPIO_PIN_MUX_MODE4;
    }
  }

  if (group_number == SL_SYSRTC_GROUP_1) {
    mux_mode = NPSS_GPIO_PIN_MUX_MODE5;
  }
  status = sl_si91x_gpio_driver_set_uulp_npss_pin_mux(sysrtc_prs_gpio.port_pin.pin, mux_mode);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  status = sl_si91x_gpio_driver_select_uulp_npss_receiver(sysrtc_prs_gpio.port_pin.pin, GPIO_RECEIVER_DS);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  status = sl_si91x_gpio_driver_select_uulp_npss_polarity(sysrtc_prs_gpio.port_pin.pin, GPIO_POLARITY_0);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/*******************************************************************************
 * Sets GPIO configuration for PRS_IN according to group number
 ******************************************************************************/
static sl_status_t sli_si91x_sysrtc_gpio_capture_config(sl_sysrtc_group_number_t group_number,
                                                        sl_si91x_gpio_pin_config_t sysrtc_prs_gpio)
{
  sl_si91x_uulp_npss_mode_t mux_mode;
  sl_status_t status = sl_gpio_driver_init(); // Initialize the GPIO driver
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  // Configure GPIO pins for SYSRTC Pin config.

  status = sl_gpio_set_configuration(sysrtc_prs_gpio);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  if (group_number == SL_SYSRTC_GROUP_0) {

    mux_mode = NPSS_GPIO_PIN_MUX_MODE3;
  }

  if (group_number == SL_SYSRTC_GROUP_1) {
    mux_mode = NPSS_GPIO_PIN_MUX_MODE4;
  }

  status = sl_si91x_gpio_driver_set_uulp_npss_pin_mux(sysrtc_prs_gpio.port_pin.pin, mux_mode);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  status = sl_si91x_gpio_driver_select_uulp_npss_receiver(sysrtc_prs_gpio.port_pin.pin, GPIO_RECEIVER_EN);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  status = sl_si91x_gpio_driver_select_uulp_npss_polarity(sysrtc_prs_gpio.port_pin.pin, GPIO_POLARITY_0);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}
/*******************************************************************************
 * Gets SYSRTC counter register value.
 *
 * @return   SYSRTC Current count value.
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_get_count(uint32_t *count_value)
{
  sl_status_t status;
  // Validating the 'pointer to count_value' parameter,
  // if it is NULL will return an error code
  if (count_value == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    // reading counter current count value
    *count_value = rsi_sysrtc_get_counter();
    status       = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * Gets SYSRTC capture register value of given group.

 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_get_capture_value(sl_sysrtc_group_number_t group_number, uint32_t *capture_value)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    if (group_number >= SL_SYSRTC_GROUP_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validating the 'pointer to capture_value' parameter,
    // if it is NULL will return an error code
    if (capture_value == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // reading capture value
    *capture_value = rsi_sysrtc_get_capture_val((uint8_t)group_number);
  } while (false);
  return status;
}

/*******************************************************************************
 * Gets SYSRTC output of compare-channel of given group 
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_get_compare_output(sl_sysrtc_group_number_t group_number,
                                               sl_sysrtc_channel_number_t channel,
                                               uint32_t *compare_output)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    if (group_number >= SL_SYSRTC_GROUP_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validating the 'pointer to capture_value' parameter,
    // if it is NULL will return an error code
    if (compare_output == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // reading capture value
    *compare_output = rsi_sysrtc_get_compare_bit((const uint32_t)group_number, (const uint32_t)channel);
  } while (false);
  return status;
}

/*******************************************************************************
 * Gets SYSRTC running status
 * Updates true if sysrtc is running and false if not running
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_is_running(boolean_t *running_status)
{
  sl_status_t status;
  uint32_t sysrtc_status = 0;
  status                 = SL_STATUS_OK;
  do {
    // To validate the pointer, if the parameters is NULL, it
    // will return an error code
    if (running_status == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    sysrtc_status = rsi_sysrtc_get_status();
    if (sysrtc_status & SYSRTC_STATUS_RUNNING) {
      *running_status = true;
      break;
    }
    *running_status = false;
  } while (false);
  return status;
}

/*******************************************************************************
 * Gets SYSRTC lock status
 * Updates true if sysrtc is locked and false if unlocked
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_is_locked(boolean_t *lock_status)
{
  sl_status_t status;
  uint32_t sysrtc_status = 0;
  status                 = SL_STATUS_OK;
  do {
    // To validate the pointer, if the parameters is NULL, it
    // will return an error code
    if (lock_status == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    sysrtc_status = rsi_sysrtc_get_status();
    if (sysrtc_status & SYSRTC_STATUS_LOCKSTATUS) {
      *lock_status = true;
      break;
    }
    *lock_status = false;
  } while (false);
  return status;
}

/*******************************************************************************
 * Sets SYSRTC input output mode
 * The input and output mode for capture and compare respectively can be set via
 * MCUSYSRTC_REG1 or via GPIO.
 * If passed true then input to capture channel and output of compare channel will
 * be through GPIO
 ******************************************************************************/
void sl_si91x_sysrtc_enable_input_output_gpio(bool is_gpio_enabled)
{
  rsi_sysrtc_set_gpio(is_gpio_enabled);
}

/*******************************************************************************
 * To get the release, sqa and dev version of SYSRTC
 * It returns the structure for SYSRTC version.
 * Structure includes three members:
 * - Release version
 * - Major version (SQA version)
 * - Minor version (Dev version)
 ******************************************************************************/
sl_sysrtc_version_t sl_si91x_sysrtc_get_version(void)
{
  sl_sysrtc_version_t version;
  version.release = SYSRTC_RELEASE_VERSION;
  version.major   = SYSRTC_MAJOR_VERSION;
  version.minor   = SYSRTC_MINOR_VERSION;
  return version;
}
/*******************************************************************************
* De-Initialization of sysrtc
* Unregisters SYSRTC callback and disables its interrupts
*
* @details:
* This API is used to de-initializes sysrtc by disabling peripheral
*******************************************************************************/
void sl_si91x_sysrtc_deinit(void)
{
  sl_sysrtc_interrupt_enables_t group_0_interrupt_flags;
  sl_sysrtc_interrupt_enables_t group_1_interrupt_flags;
  group_0_interrupt_flags.group0_capture0_interrupt_is_enabled = true;
  group_0_interrupt_flags.group0_compare0_interrupt_is_enabled = true;
  group_0_interrupt_flags.group0_compare1_interrupt_is_enabled = true;
  group_0_interrupt_flags.group0_overflow_interrupt_is_enabled = true;
  group_1_interrupt_flags.group1_capture0_interrupt_is_enabled = true;
  group_1_interrupt_flags.group1_compare0_interrupt_is_enabled = true;
  group_1_interrupt_flags.group1_compare1_interrupt_is_enabled = true;
  group_1_interrupt_flags.group1_overflow_interrupt_is_enabled = true;
  // Unregistering callback for all interrupts and disabling interrupts of group-0
  sl_si91x_sysrtc_unregister_callback(SL_SYSRTC_GROUP_0, &group_0_interrupt_flags);
  // Unregistering callback for all interrupts and disabling interrupts of group-1
  sl_si91x_sysrtc_unregister_callback(SL_SYSRTC_GROUP_1, &group_1_interrupt_flags);
  // disabling SYSRTC
  rsi_sysrtc_disable();
}

/*******************************************************************************
* Interrupt handler for SYSRTCgit 
*
* @param none
* @return none
*******************************************************************************/
void SYSRTC_IRQHandler(void)
{
  //Disable Interrupts
  rsi_sysrtc_disable_interrupts();
  uint32_t irq_flag;
  if (SYSRTC0->GRP0_IF) {
    irq_flag = rsi_sysrtc_get_group_interrupts((uint8_t)SL_SYSRTC_GROUP_0);
    if (irq_flag & SYSRTC_GRP0_IEN_OVF) {
      rsi_sysrtc_clear_group_interrupts((uint8_t)SL_SYSRTC_GROUP_0, (irq_flag & SYSRTC_GRP0_IF_OVF));
      *(uint32_t *)callback_flags |= SYSRTC_GRP0_IF_OVF;
    }
    if (irq_flag & SYSRTC_GRP0_IEN_CMP0) {
      rsi_sysrtc_clear_group_interrupts((uint8_t)SL_SYSRTC_GROUP_0, (irq_flag & SYSRTC_GRP0_IF_CMP0));
      *(uint32_t *)callback_flags |= SYSRTC_GRP0_IF_CMP0;
    }
    if (irq_flag & SYSRTC_GRP0_IEN_CMP1) {
      rsi_sysrtc_clear_group_interrupts((uint8_t)SL_SYSRTC_GROUP_0, (irq_flag & SYSRTC_GRP0_IF_CMP1));
      *(uint32_t *)callback_flags |= SYSRTC_GRP0_IF_CMP1;
    }
    if (irq_flag & SYSRTC_GRP0_IEN_CAP0) {
      rsi_sysrtc_clear_group_interrupts((uint8_t)SL_SYSRTC_GROUP_0, (irq_flag & SYSRTC_GRP0_IF_CAP0));
      *(uint32_t *)callback_flags |= SYSRTC_GRP0_IF_CAP0;
    }
  }
  if (SYSRTC0->GRP1_IF) {
    irq_flag = rsi_sysrtc_get_group_interrupts((uint8_t)SL_SYSRTC_GROUP_1);
    if (irq_flag & SYSRTC_GRP1_IEN_OVF) {
      rsi_sysrtc_clear_group_interrupts((uint8_t)SL_SYSRTC_GROUP_1, (irq_flag & SYSRTC_GRP1_IF_OVFIF));
      *(uint32_t *)callback_flags |= SYSRTC_GRP1_IF_OVFIF;
    }
    if (irq_flag & SYSRTC_GRP1_IEN_CMP0) {
      rsi_sysrtc_clear_group_interrupts((uint8_t)SL_SYSRTC_GROUP_1, (irq_flag & SYSRTC_GRP1_IF_CMP0IF));
      *(uint32_t *)callback_flags |= SYSRTC_GRP1_IF_CMP0IF;
    }
    if (irq_flag & SYSRTC_GRP1_IEN_CMP1) {
      rsi_sysrtc_clear_group_interrupts((uint8_t)SL_SYSRTC_GROUP_1, (irq_flag & SYSRTC_GRP1_IF_CMP1IF));
      *(uint32_t *)callback_flags |= SYSRTC_GRP1_IF_CMP1IF;
    }
    if (irq_flag & SYSRTC_GRP1_IEN_CAP0) {
      rsi_sysrtc_clear_group_interrupts((uint8_t)SL_SYSRTC_GROUP_1, (irq_flag & SYSRTC_GRP1_IF_CAP0IF));
      *(uint32_t *)callback_flags = SYSRTC_GRP1_IF_CAP0IF;
    }
  }
  // calling the callback function
  callback_function_ptr(callback_flags);
  // Enabling Interrupts
  rsi_sysrtc_enable_interrupts();
}
