/******************************************************************************
* @file sl_si91x_qei.c
* @brief QEI driver API implementation
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
#include "sl_si91x_qei.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_gpio_board.h"
/*******************************************************************************
****************************  DEFINES / MACROS   ********************************
*******************************************************************************/
#define MAX_REG_VALUE       0xFFFFFFFF                                            // maximum value for 32-bit register
#define QEI_IRQHandler      IRQ049_Handler                                        // QEI IRQ handler
#define QEI_RELEASE_VERSION 0                                                     // QEI Release version
#define QEI_SQA_VERSION     0                                                     // QEI SQA version
#define QEI_DEV_VERSION     2                                                     // QEI Developer version
#define OUTPUT              0                                                     // Output value
#define MAX_DELTA_TIME      1048576                                               // Maximum delta time value
#define VALID_INTR          (BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4) | BIT(5)) // Validate interrupt bit
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static sl_qei_callback_t callback_func_ptr = NULL;
static void *callback_flag                 = NULL;
/*******************************************************************************
 ***************************  Local Types  ********************************
 ******************************************************************************/
/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
void QEI_IRQHandler(void);

/*******************************************************************************
 **********************  Local Function Definition****************************
 ******************************************************************************/
static bool validate_intr_clr_config(uint32_t intr_config);
static bool validate_intr_unmask_config(sl_qei_intr_unmask_t *intr_flags);
static sl_status_t configure_gpio_pin(sl_gpio_pin_config_t *pin_config);
/*******************************************************************************
 ***********************  Global function Definitions *************************
 ******************************************************************************/
/*******************************************************************************
 * This API is used to start the velocity counter by setting the delta time.
 ******************************************************************************/
sl_status_t sl_si91x_qei_start_velocity(uint32_t period)
{
  sl_status_t status;
  // Check if the delta time exceeds the maximum allowed value
  if (period > MAX_DELTA_TIME) {
    // Set status to invalid parameter
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // Set delta time
    RSI_QEI_SetDeltaTime(QEI, period);
    // Start velocity counter based on the state
    RSI_QEI_StartVelocityCounter(QEI);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to set the position counter value.
 ******************************************************************************/
sl_status_t sl_si91x_qei_set_position_counter(uint32_t pos_cnt_value)
{
  sl_status_t status;
  // Check if the position counter value exceeds the maximum allowed
  if (pos_cnt_value > MAX_REG_VALUE) {
    // Set status to invalid parameter
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // Set the position counter to the specified value
    RSI_QEI_SetPosition(QEI, pos_cnt_value);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to set the encoding mode for the QEI
 ******************************************************************************/
sl_status_t sl_si91x_qei_set_encoding_mode(sl_qei_encoding_mode_t mode)
{
  sl_status_t status;
  // Check if the mode is valid
  if (mode >= SL_QEI_ENCODE_MODE_LAST) {
    // Set status to invalid parameter
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // Set the encoding mode
    RSI_QEI_SetMode(QEI, mode);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to set the Position Counter Direction indication from user for the QEI
 ******************************************************************************/
sl_status_t sl_si91x_qei_set_pos_direction(sl_qei_direction_t direction)
{
  sl_status_t status;
  // Check if the direction is valid
  if (direction >= SL_QEI_DIRECTION_LAST) {
    // Set status to invalid parameter
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // Set the direction
    RSI_QEI_SetDirection(QEI, direction);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to set the index counter value
 ******************************************************************************/
sl_status_t sl_si91x_qei_set_index_counter(uint32_t index_count)
{
  sl_status_t status;
  // Check if the index count exceeds the maximum allowed
  if (index_count > MAX_REG_VALUE) {
    // Set status to invalid parameter
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // Set the index counter to the specified value
    RSI_QEI_SetIndex(QEI, index_count);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to configure the delta time frequency for velocity calculation
 ******************************************************************************/
sl_status_t sl_si91x_qei_configure_delta_time_frequency(uint32_t freq, uint32_t period)
{
  sl_status_t status;
  // Check if delta time exceeds the maximum allowed
  if (period > MAX_DELTA_TIME) {
    // Set status to invalid parameter
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // Configure delta time and frequency
    RSI_QEI_ConfigureDeltaTimeAndFreq(QEI, freq, period);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to set the delta time for velocity calculation
 ******************************************************************************/
sl_status_t sl_si91x_qei_set_delta_time(uint32_t delta_time)
{
  sl_status_t status;
  // Check if the delta time exceeds the maximum allowed
  if (delta_time > MAX_DELTA_TIME) {
    // Set status to invalid parameter
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // Set the delta time
    RSI_QEI_SetDeltaTime(QEI, delta_time);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to set the position match value
 ******************************************************************************/
sl_status_t sl_si91x_qei_set_position_match(uint32_t pos_match)
{
  sl_status_t status;
  // Check if the position match value exceeds the maximum allowed
  if (pos_match > MAX_REG_VALUE) {
    // Set status to invalid parameter
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // Set the position match value
    RSI_QEI_SetPosMatch(QEI, pos_match);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to set a value for velocity comparison
 ******************************************************************************/
sl_status_t sl_si91x_qei_set_velocity_value_to_compare(uint32_t velocity_value)
{
  sl_status_t status;
  // Check if the velocity value exceeds the maximum allowed
  if (velocity_value > MAX_REG_VALUE) {
    // Set status to invalid parameter
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // Set the velocity value for comparison
    RSI_QEI_VelocityValueToCompare(QEI, velocity_value);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to set the maximum value for the index counter
 ******************************************************************************/
sl_status_t sl_si91x_qei_set_index_maximum_counter(uint32_t max_index)
{
  sl_status_t status;
  // Check if the max_index exceeds the allowed maximum value
  if (max_index > MAX_REG_VALUE) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // Set the maximum index
    RSI_QEI_SetMaxIndex(QEI, max_index);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to swap phases A and B for the QEI
 ******************************************************************************/
sl_status_t sl_si91x_qei_swap_a_b(sl_qei_swap_ab_t swap_select)
{
  sl_status_t status;
  // Check if the provided selection is valid
  if (swap_select >= SL_QEI_SWAP_LAST) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // Perform the swap
    RSI_QEI_Swap_PhaseAB(QEI, swap_select);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to configure the QEI to operate in timer mode
 ******************************************************************************/
sl_status_t sl_si91x_qei_configure_timer_mode(sl_qei_timer_mode_t timer_mode)
{
  sl_status_t status;
  // Check if the provided timer mode is valid
  if (timer_mode >= SL_QEI_MODE_LAST) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // Configure the timer mode
    RSI_QEI_ConfigureTimerMode(QEI, timer_mode);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to configure GPIO pins for QEI module
 ******************************************************************************/
static sl_status_t configure_gpio_pin(sl_gpio_pin_config_t *pin_config)
{
  sl_status_t status;
  do {
    // Validates the null pointer, if true returns error code
    if (pin_config == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if ((pin_config->pin == SL_SI91X_GPIO_31_PIN) || (pin_config->pin == SL_SI91X_GPIO_32_PIN)
        || (pin_config->pin == SL_SI91X_GPIO_33_PIN) || (pin_config->pin == SL_SI91X_GPIO_34_PIN)) {
      // Enable pad selection for GPIO 31, 32, 33, and 34
      sl_si91x_gpio_driver_enable_pad_selection(PAD_SELECT_9);
    }
    if (pin_config->pin >= GPIO_MAX_PIN) {
      sl_si91x_gpio_pin_config_t gpio_config = {
        .port_pin = { .port = SL_GPIO_ULP_PORT, .pin = (pin_config->pin - GPIO_MAX_PIN) },
      };
      sl_gpio_set_configuration(gpio_config);
      sl_gpio_set_pin_mode(SL_GPIO_ULP_PORT, (uint8_t)(pin_config->pin - GPIO_MAX_PIN), ULP_GPIO_MODE_6, OUTPUT);
    } else {
      sl_si91x_gpio_pin_config_t gpio_config = {
        .port_pin = { .port = pin_config->port, .pin = pin_config->pin },
      };
      sl_gpio_set_configuration(gpio_config);
    }
    sl_gpio_set_pin_mode(pin_config->port, pin_config->pin, pin_config->mux, OUTPUT);
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to initialize QEI module
 ******************************************************************************/
sl_status_t sl_si91x_qei_init(sl_qei_init_t *qei_init)
{
  sl_status_t status;
  // Validates the null pointer, if true returns error code
  if (qei_init == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    // Configure QEI pins
    status = configure_gpio_pin(&qei_init->dir); // QEI dir pin mux
    if (status != SL_STATUS_OK) {
      return status;
    }
    status = configure_gpio_pin(&qei_init->idx); // QEI idx pin mux
    if (status != SL_STATUS_OK) {
      return status;
    }
    status = configure_gpio_pin(&qei_init->phasea); // QEI phasea pin mux
    if (status != SL_STATUS_OK) {
      return status;
    }
    status = configure_gpio_pin(&qei_init->phaseb); // QEI phaseb pin mux
    if (status != SL_STATUS_OK) {
      return status;
    }
    // QEI clock enable
    RSI_CLK_PeripheralClkEnable2(M4CLK, QE_PCLK_ENABLE);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to de-initialize QEI module
 ******************************************************************************/
void sl_si91x_qei_deinit(void)
{
  uint32_t configParms = 0;
  // Reset the position counter and index counter
  configParms = (SL_QEI_POS_CNT_RST) | (SL_QEI_INDEX_COUNT_RESET);
  RSI_QEI_SetConfiguration(QEI, configParms);
  RSI_QEI_Disable(QEI); // Disable the QEI
}

/*******************************************************************************
 * This API is used to set the frequency for the QEI module
 ******************************************************************************/
sl_status_t sl_si91x_qei_set_module_frequency(uint32_t frequency)
{
  sl_status_t status;
  // Set the module frequency
  RSI_QEI_SetModuleFreq(QEI, frequency);
  status = SL_STATUS_OK;
  return status;
}

/*******************************************************************************
 * This API is used to set the digital filter clock division
 ******************************************************************************/
sl_status_t sl_si91x_qei_set_digital_filter_clock_division(sl_qei_clk_div_t clock_division)
{
  sl_status_t status;
  // Check if the clock division exceeds the maximum allowed value
  if (clock_division >= SL_QEI_CLK_DIV_LAST) {
    // Set status to invalid parameter
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // Set the clock division
    RSI_QEI_SetDigitalFilterClkDiv(QEI, clock_division);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to register callback
 ******************************************************************************/
sl_status_t sl_si91x_qei_register_callback(sl_qei_callback_t callback,
                                           void *callback_flag_value,
                                           sl_qei_intr_unmask_t *intr_flags)
{
  sl_status_t status;
  status               = SL_STATUS_OK;
  uint32_t intr_unmask = 0;
  do {
    // Validate null pointers
    if ((callback == NULL) || (intr_flags == NULL)) {
      status = SL_STATUS_NULL_POINTER; // Set status to null pointer error
      break;
    }
    if (!(validate_intr_unmask_config(intr_flags))) {
      // Set status to invalid parameter
      status = SL_STATUS_INVALID_PARAMETER;
    }
    // Check if a callback is already registered
    if (callback_func_ptr != NULL) {
      status = SL_STATUS_BUSY; // Set status to busy
      break;
    }
    // Evaluate and set unmasked interrupt flags
    if (intr_flags->qei_index_cnt_match_intr_unmask) {
      intr_unmask |= SL_QEI_IDX_CNT_MATCH_INTR_UNMASK; // Unmask index count match interrupt
    }
    if (intr_flags->qei_position_cnt_match_intr_unmask) {
      intr_unmask |= SL_QEI_POS_CNT_MATCH_INTR_UNMASK; // Unmask position count match interrupt
    }
    if (intr_flags->qei_position_cnt_reset_intr_unmask) {
      intr_unmask |= SL_QEI_POS_CNT_RESET_INTR_UNMASK; // Unmask position count reset interrupt
    }
    if (intr_flags->qei_position_cntr_err_intr_unmask) {
      intr_unmask |= SL_QEI_POS_CNT_ERR_INTR_UNMASK; // Unmask position counter error interrupt
    }
    if (intr_flags->qei_velocity_less_than_intr_unmask) {
      intr_unmask |= SL_QEI_VELOCITY_LESS_INTR_UNMASK; // Unmask velocity less interrupt
    }
    if (intr_flags->qei_velocity_computation_over_intr_unmask) {
      intr_unmask |= SL_QEI_VELOCITY_COMP_OVER_INTR_UNMASK; // Unmask velocity computation overflow interrupt
    }
    RSI_QEI_IntrUnMask(QEI, intr_unmask);    // Unmask the interrupts
    callback_func_ptr = callback;            // Register the callback function
    callback_flag     = callback_flag_value; // Register the callback flag value
    NVIC_EnableIRQ(QEI_IRQn);                // Enable the QEI interrupt
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to unregister callback
 ******************************************************************************/
sl_status_t sl_si91x_qei_unregister_callback(sl_qei_intr_mask_t *intr_mask)
{
  sl_status_t status;
  uint32_t intr_mask_value = 0;
  status                   = SL_STATUS_OK;
  do {
    if (intr_mask == NULL) {
      // Set status to null pointer error
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Check if a callback is not registered
    if (callback_func_ptr == NULL) {
      status = SL_STATUS_FAIL;
      break;
    }
    // Evaluate and set masked interrupt flags
    if (intr_mask->qei_index_cnt_match_intr_mask) {
      intr_mask_value |= SL_QEI_IDX_CNT_MATCH_INTR_MASK; // Mask index count match interrupt
    }
    if (intr_mask->qei_position_cnt_match_intr_mask) {
      intr_mask_value |= SL_QEI_POS_CNT_MATCH_INTR_MASK; // Mask position count match interrupt
    }
    if (intr_mask->qei_position_cnt_reset_intr_mask) {
      intr_mask_value |= SL_QEI_POS_CNT_RESET_INTR_MASK; // Mask position count reset interrupt
    }
    if (intr_mask->qei_position_cntr_err_intr_mask) {
      intr_mask_value |= SL_QEI_POS_CNT_ERR_INTR_MASK; // Mask position counter error interrupt
    }
    if (intr_mask->qei_velocity_computation_over_intr_mask) {
      intr_mask_value |= SL_QEI_VELOCITY_COMP_OVER_INTR_MASK; // Mask velocity computation overflow interrupt
    }
    if (intr_mask->qei_velocity_less_than_intr_mask) {
      intr_mask_value |= SL_QEI_VELOCITY_LESS_INTR_MASK; // Mask velocity less interrupt
    }
    RSI_QEI_IntrMask(QEI, intr_mask_value); // Mask the interrupts
    if (RSI_QEI_GetIntrMask(QEI) == 0) {
      callback_func_ptr = NULL; // Clear the callback function
      callback_flag     = NULL; // Clear the callback flag
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to set the maximum position counter value
 ******************************************************************************/
sl_status_t sl_si91x_qei_set_maximum_position_counter(uint32_t max_pos_cnt)
{
  sl_status_t status;
  // Check if the maximum position counter exceeds the allowed maximum value
  if (max_pos_cnt > MAX_REG_VALUE) {
    // Set status to invalid parameter
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // Set the maximum position counter
    RSI_QEI_SetMaxPosCnt(QEI, max_pos_cnt);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to set configuration parameters for the QEI module
 ******************************************************************************/
sl_status_t sl_si91x_qei_set_configuration(sl_qei_control_t state, sl_qei_config_t *config)
{
  sl_status_t status;
  do {
    if ((config == NULL)) {
      // Set status to null pointer error
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (state >= SL_QEI_STATE_LAST) {
      // Set status to invalid parameter
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Set or clear the configuration based on the state
    if (state == SL_QEI_SET) {
      // Set the configuration
      RSI_QEI_SetConfiguration(QEI, config->config_param);
      RSI_QEI_SetControls(QEI, config->ctrl_param);
    } else {
      // Clear the configuration
      RSI_QEI_ClrConfiguration(QEI, config->config_param);
      RSI_QEI_ClrControls(QEI, config->ctrl_param);
    }
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to get configuration parameters for the QEI module.
 ******************************************************************************/
sl_status_t sl_si91x_qei_get_configuration(sl_qei_control_t state, sl_qei_config_t *config)
{
  sl_status_t status;
  uint32_t config_param = 0;
  uint32_t ctrl_param   = 0;
  do {
    // Check for NULL pointer
    if (config == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Check for valid state
    if (state >= SL_QEI_STATE_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Get the current configuration based on the state
    if (state == SL_QEI_SET) {
      // Retrieve the set configuration
      config_param = RSI_QEI_GetSetConfiguration(QEI);
      ctrl_param   = RSI_QEI_GetControls(QEI);
    } else {
      // Retrieve the clear configuration
      config_param = RSI_QEI_GetClrConfiguration(QEI);
    }
    config->config_param = config_param; // Store the bit field configuration
    config->ctrl_param   = ctrl_param;   // Store the control parameters
    status               = SL_STATUS_OK;
  } while (false);

  return status;
}

/*******************************************************************************
 * This API is used for QEI interrupt handler
 ******************************************************************************/
void QEI_IRQHandler(void)
{
  do {
    // Check and handle count match interrupt
    if (RSI_QEI_GetIntrStatus(QEI) & SL_QEI_IDX_CNT_MATCH_INTR_LVL) {
      // Clear the count match interrupt
      RSI_QEI_ClrIntrStatus(QEI, SL_QEI_IDX_CNT_MATCH_INTR_LVL);
      // Set callback flag for count match
      *(uint32_t *)callback_flag = SL_QEI_IDX_CNT_MATCH_INTR_LVL;
      break;
    }
    // Check and handle position count match interrupt
    if (RSI_QEI_GetIntrStatus(QEI) & SL_QEI_POS_CNT_MATCH_INTR_LVL) {
      // Clear the position count match interrupt
      RSI_QEI_ClrIntrStatus(QEI, SL_QEI_POS_CNT_MATCH_INTR_LVL);
      // Set callback flag for position count match
      *(uint32_t *)callback_flag = SL_QEI_POS_CNT_MATCH_INTR_LVL;
      break;
    }
    // Check and handle position count reset interrupt
    if (RSI_QEI_GetIntrStatus(QEI) & SL_QEI_POS_CNT_RESET_INTR_LVL) {
      // Clear the position count reset interrupt
      RSI_QEI_ClrIntrStatus(QEI, SL_QEI_POS_CNT_RESET_INTR_LVL);
      // Set callback flag for position count reset
      *(uint32_t *)callback_flag = SL_QEI_POS_CNT_RESET_INTR_LVL;
      break;
    }
    // Check and handle position counter error interrupt
    if (RSI_QEI_GetIntrStatus(QEI) & SL_QEI_POS_CNT_ERR_INTR_LVL) {
      // Clear the position counter error interrupt
      RSI_QEI_ClrIntrStatus(QEI, SL_QEI_POS_CNT_ERR_INTR_LVL);
      // Set callback flag for position counter error
      *(uint32_t *)callback_flag = SL_QEI_POS_CNT_ERR_INTR_LVL;
      break;
    }
    // Check and handle velocity computation overflow interrupt
    if (RSI_QEI_GetIntrStatus(QEI) & SL_QEI_VELOCITY_COMP_OVER_INTR_LVL) {
      // Clear the velocity computation overflow interrupt
      RSI_QEI_ClrIntrStatus(QEI, SL_QEI_VELOCITY_COMP_OVER_INTR_LVL);
      // Set callback flag for velocity computation overflow
      *(uint32_t *)callback_flag = SL_QEI_VELOCITY_COMP_OVER_INTR_LVL;
      break;
    }
    // Check and handle velocity less than interrupt
    if (RSI_QEI_GetIntrStatus(QEI) & SL_QEI_VELOCITY_LESS_INTR_LVL) {
      // Clear the velocity less than interrupt
      RSI_QEI_ClrIntrStatus(QEI, SL_QEI_VELOCITY_LESS_INTR_LVL);
      // Set callback flag for velocity less than
      *(uint32_t *)callback_flag = SL_QEI_VELOCITY_LESS_INTR_LVL;
      break;
    }
  } while (false);
  // Call the registered callback function
  callback_func_ptr(callback_flag);
}

/*******************************************************************************
 * This API is used to clear interrupts in the QEI module
 ******************************************************************************/
sl_status_t sl_si91x_qei_clear_interrupt(uint32_t int_mask)
{
  sl_status_t status;
  // Check if the interrupt mask exceeds the maximum allowed value
  if (!(validate_intr_clr_config(int_mask))) {
    // Set status to invalid parameter
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // Clear the specified interrupt(s)
    RSI_QEI_ClrIntrStatus(QEI, int_mask);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to get the release, SQA and DEV version of QEI
 * It returns the structure for QEI version.
 * Structure includes three members:
 * - Release version
 * - SQA version
 * - DEV version
 ******************************************************************************/
sl_qei_version_t sl_si91x_qei_get_version(void)
{
  sl_qei_version_t version;
  version.minor   = QEI_DEV_VERSION;
  version.release = QEI_RELEASE_VERSION;
  version.major   = QEI_SQA_VERSION;
  return version;
}

/*******************************************************************************
 * This API is used to evaluate Validate the interrupt clear configuration
 ******************************************************************************/
static bool validate_intr_clr_config(uint32_t intr_config)
{
  // Check if the bits set in intr_config are within the valid mask
  return (intr_config & ~VALID_INTR) == 0;
}

/*******************************************************************************
 * This API is used to evaluate Validate the interrupt unmask configuration
 ******************************************************************************/
static bool validate_intr_unmask_config(sl_qei_intr_unmask_t *intr_flags)
{
  // Check if the bits set in intr_config are within the valid mask
  return (*(uint32_t *)intr_flags & ~VALID_INTR) == 0;
}
