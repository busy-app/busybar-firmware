/******************************************************************************
* @file sl_si91x_analog_comparator.c
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
#include "sl_si91x_analog_comparator.h"
#include "rsi_comparator.h"
#include "rsi_dac.h"
#include "rsi_rom_ulpss_clk.h"
#include "aux_reference_volt_config.h"

/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
#define CHIP_REF_VOLTAGE              3.3f // Chip Reference voltage for LDO i.e., it should be between 2.8V and 3.3V
#define LDO_REF_VOLTAGE               3.3f // Reference voltage to LDO, Min is 1.8V and Max is 3.3V
#define ANALOG_COMPARATOR1_IRQHandler IRQ008_Handler // Analog comparator-1 IRQ Handler
#define ANALOG_COMPARATOR2_IRQHandler IRQ007_Handler // Analog comparator-2 IRQ Handler

/*******************************************************************************
 ***************************  Local TYPES  ********************************
 ******************************************************************************/

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static sl_analog_comparator_callback_t comparator_callback_function_pointers[] = { NULL, NULL };
/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

/*******************************************************************************
 ***********************  Global function Prototypes *************************
 ******************************************************************************/
void ANALOG_COMPARATOR1_IRQHandler(void);
void ANALOG_COMPARATOR2_IRQHandler(void);

/*******************************************************************************
***********************  Global function Definitions *************************
 ******************************************************************************/
/*******************************************************************************
* @brief:Initializes Analog comparator peripheral

* @details: 
* Enables system core clock and Auxiliary clock with MHZ RC clock
* Configures reference LDO voltage as 2.8v
*******************************************************************************/
void sl_si91x_analog_comparator_init(void)
{
  float vref_value = LDO_REF_VOLTAGE; // Reference voltage to LDO, Min is 1.8V and Max is 3.3V

  RSI_PS_UlpssPeriPowerUp(ULPSS_PWRGATE_ULP_AUX);

  if (analog_get_power_state() == 0) {
    // Configure the AUX clock source
    RSI_ULPSS_AuxClkConfig(ULPCLK, ENABLE_STATIC_CLK, ULP_AUX_REF_CLK);
  }
  analog_set_power_state(COMP_BIT_POS, ANALOG_POWERED_ON);

  // Configure the reference LDO voltage as 3.3v
  RSI_AUX_RefVoltageConfig(vref_value, CHIP_REF_VOLTAGE);
}

/*******************************************************************************
* @brief: Configures analog comparator parameters

* @details: To configure analog comparator number, select non-inverting and inverting inputs, 
* enable or disable hystersis and enable or disable filter for comparator outputs.
* Also configure input pins.
*******************************************************************************/
sl_status_t sl_si91x_analog_comparator_set_configurations(sl_analog_comparator_config_t *comparator_config_ptr)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    // To validate the structure pointer, if the parameters is NULL, it
    // will return an error code
    if (comparator_config_ptr == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validating comparator parameters
    if ((comparator_config_ptr->comparator_number >= SL_COMPARATOR_LAST)
        || (comparator_config_ptr->non_inverting_input >= SL_COMPARATOR_INPUT_LAST)
        || (comparator_config_ptr->inverting_input >= SL_COMPARATOR_INPUT_LAST)
        || (comparator_config_ptr->hystersis_value >= SL_COMPARATOR_HYSTERSIS_VALUE_LAST)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Configuring comparator parameters
    RSI_COMP_Config(AUX_ADC_DAC_COMP,
                    comparator_config_ptr->comparator_number,
                    comparator_config_ptr->non_inverting_input,
                    comparator_config_ptr->inverting_input,
                    comparator_config_ptr->hystersis_value,
                    comparator_config_ptr->is_filter_enabled);
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Registers Callback for comparator interrupts and enables its interrupt

* @details:
* It will register the callback, i.e., stores the callback function address
  and pass to the variable that will be called in Interrupt Handler.

* It will also enables the timer interrupt.

* If any callback is already registered, the user needs to unregister the
* callback first before registering another callback.
*******************************************************************************/
sl_status_t sl_si91x_analog_comparator_register_callback(sl_analog_comparator_number_t comparator_number,
                                                         sl_analog_comparator_callback_t on_comparator_callback)
{
  sl_status_t status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating the 'on_timeout_callback function pointer' parameter and void pointer to callback data,
    // if they are NULL will return an error code
    if (on_comparator_callback == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validating comparator number
    if (comparator_number >= SL_COMPARATOR_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // To validate the function pointer, if the parameters is not NULL then it
    //will return an busy error code
    if (comparator_callback_function_pointers[(comparator_number - 1)] != NULL) {
      status = SL_STATUS_BUSY;
      break;
    }
    // The function pointer is fed to the static variable, which will be called in the IRQ handler
    comparator_callback_function_pointers[(comparator_number - 1)] = on_comparator_callback;
    //DEBUGOUT("Analog Comparator callback is registered successfully \n");
    // Enabling comparator interrupt
    RSI_COMP_Enable(AUX_ADC_DAC_COMP, comparator_number, ENABLE);
  } while (false);
  return status;
}

/*******************************************************************************
 * To configure analog comparator resistor bank threshold value to change resistor bank output voltage
 *
 * 
 *******************************************************************************/
sl_status_t sl_si91x_analog_comparator_set_resistor_bank_threshold(
  sl_analog_comparator_threshold_values_t threshold_value)
{
  sl_status_t status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating comparator threshold value param
    if (threshold_value >= SL_COMPARATOR_THRESHOLD_VALUE_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Setting resistor bank threshold value to change its output voltage
    RSI_COMP_ResBank(AUX_ADC_DAC_COMP, (uint16_t)threshold_value);
  } while (false);
  return status;
}

/*******************************************************************************
 * To configure scale factor value of analog comparator to change reference scaler output voltage
 *
 * 
 *******************************************************************************/
sl_status_t sl_si91x_analog_comparator_set_reference_scaler_output(
  sl_analog_comparator_scale_factor_values_t scale_factor_value)
{
  sl_status_t status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating comparator scalar factor value
    if (scale_factor_value >= SL_COMPARATOR_SCALE_FACTOR_VALUE_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Setting reference scale value to change its output voltage
    RSI_COMP_ReferenceScaler(AUX_ADC_DAC_COMP, (uint16_t)scale_factor_value);
  } while (false);
  return status;
}

/*******************************************************************************
 * unregisters analog comparator callback and disable its interrupts as per comparator number
 *
 * @details:
 * It will unregister the callback, i.e., clear the callback function address
 * and pass NULL value to the variable
 * It will also mask and disable the timer interrupt.
*******************************************************************************/
sl_status_t sl_si91x_analog_comparator_unregister_callback(sl_analog_comparator_number_t comparator_number)
{
  sl_status_t status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating comparator number
    if (comparator_number >= SL_COMPARATOR_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Disabling passed timer instance interrupt
    RSI_COMP_IntrEnableDisable(comparator_number, DISABLE);

    //The callback should be null in the unregister callback api because
    //one cannot register the callback if it is not null
    comparator_callback_function_pointers[(comparator_number - 1)] = NULL;
  } while (false);
  return status;
}

/***************************************************************************/ /**
* De-Initializes Analog comparator peripheral
* Disables Peripheral clock
*******************************************************************************/
void sl_si91x_analog_comparator_deinit(void)
{
  analog_set_power_state(COMP_BIT_POS, ANALOG_POWERED_OFF);

  if (!analog_get_power_state()) {
    RSI_PS_UlpssPeriPowerDown(ULPSS_PWRGATE_ULP_AUX);
    RSI_ULPSS_PeripheralDisable(ULPCLK, ULP_AUX_CLK);
  }
  // Disablng Interrupt
  NVIC_DisableIRQ(COMP1_IRQn);
  NVIC_DisableIRQ(COMP2_IRQn);
}
/*******************************************************************************
* Interrupt handler for comparator 1
*
* @param none
* @return none
*******************************************************************************/
void ANALOG_COMPARATOR1_IRQHandler(void)
{
  // calling callback function pointer for comparator-1
  comparator_callback_function_pointers[(SL_COMPARATOR_1 - 1)]();
}

///*******************************************************************************
//* Interrupt handler for comparator 2
//*
//* @param none
//* @return none
//*******************************************************************************/
void ANALOG_COMPARATOR2_IRQHandler(void)
{
  // calling callback function pointer for comparator-2
  comparator_callback_function_pointers[(SL_COMPARATOR_2 - 1)]();
}
