/******************************************************************************
 * @file sl_si91x_bod.c
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_si91x_bod.h"
#include "clock_update.h"
#include "rsi_rom_clks.h"
#include "rsi_rom_ulpss_clk.h"
#include "sl_status.h"
#include "sl_si91x_bod_config.h"
/*******************************************************************************
 ***************************  DEFINES / MACROS   *******************************
 ******************************************************************************/

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
// Static variable to hold the registered callback for BOD
static bod_callback_t user_callback = NULL;

// Static variable to hold the registered callback for button
static bod_button_callback_t user_button_callback = NULL;

// Enumeration for BOD modes
typedef enum sl_bod_mode {
  SL_BOD_MODE_AUTOMATIC, ///< Automatic mode
  SL_BOD_MODE_MANUAL     ///< Manual mode
} sl_bod_mode_t;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

/*******************************************************************************
 * Function to initialize the BOD
 ******************************************************************************/
sl_status_t sl_si91x_bod_init(void)
{
  // Enable the BOD comparator
  BOD->BOD_COMP_SEL_REG_b.CMP_5_EN = ENABLE;
  // Reset manual comparator mux selection
  BOD->BOD_COMP_MODE_REG_b.MANUAL_CMP_MUX_SEL = DISABLE;
  // Set manual comparator mux selection to 4
  BOD->BOD_COMP_MODE_REG_b.MANUAL_CMP_MUX_SEL = SL_BOD_CMP_MUX_SEL_EN;
  // Disable brown out interrupt threshold during initialization
  BOD->BOD_TEST_PG_VBATT_STATUS_REG_b.BROWN_OUT_INTERRUPT_THRESHOLD = DISABLE;
  // Check if the BOD comparator is enabled successfully
  if (BOD->BOD_COMP_SEL_REG_b.CMP_5_EN == ENABLE) {
    return SL_STATUS_OK; // Return success status
  } else {
    return SL_STATUS_BOD_NOT_ENABLED; // Return failure status
  }
}

/*******************************************************************************
 * Function to deinitialize the BOD
 ******************************************************************************/
sl_status_t sl_si91x_bod_deinit(void)
{
  // Disable the BOD comparator
  BOD->BOD_COMP_SEL_REG_b.CMP_5_EN = DISABLE;
  // Reset manual comparator mux selection
  BOD->BOD_COMP_MODE_REG_b.MANUAL_CMP_MUX_SEL = DISABLE;
  // Unregister the BOD callback function
  sl_si91x_bod_unregister_callback();
  // Check if the BOD comparator is disabled successfully
  if (BOD->BOD_COMP_SEL_REG_b.CMP_5_EN == DISABLE) {
    return SL_STATUS_OK; // Return success status
  } else {
    return SL_STATUS_FAIL; // Return failure status
  }
}

/*******************************************************************************
 * Function to configure the BOD voltage threshold
 ******************************************************************************/
sl_status_t sl_si91x_bod_set_threshold(float vbatt_threshold)
{
  static uint32_t threshold_i = 0; // Integer part of the threshold
  static float threshold_f    = 0; // Floating part of the threshold

  // Validate threshold range
  if (vbatt_threshold < SL_BOD_MIN_THRESHOLD || vbatt_threshold > SL_BOD_MAX_THRESHOLD) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Enable the BOD comparator
  BOD->BOD_COMP_SEL_REG |= SL_BOD_ENABLE_SIGNAL; // A Dummy write is required,it's due to write buffer latency
  BOD->BOD_COMP_SEL_REG_b.CMP_5_EN = ENABLE;
  // Check if BOD is enabled
  if (BOD->BOD_COMP_SEL_REG_b.CMP_5_EN == ENABLE) {
    // Calculate the threshold value
    // Calculate the battery threshold voltage (threshold_f) using the given formula.
    // The formula is derived from the battery voltage computation elements.
    // vbatt_threshold: The input battery threshold voltage.
    // SL_BOD_VBATT_COMPUT_FIRST_ELEMENT: A constant used for the first element in the computation.
    // SL_BOD_VBATT_COMPUT_SECOND_ELEMENT: A constant used for the second element in the computation.
    // SL_BOD_VBATT_COMPUT_THIRD_ELEMENT: A constant used for the third element in the computation.
    // The formula performs the following steps:
    // 1. Divide the input battery threshold voltage by the first computation element.
    // 2. Multiply the result by the sum of the second and third computation elements.
    // 3. Subtract the second computation element from the result to get the final threshold voltage.
    threshold_f = ((vbatt_threshold / SL_BOD_VBATT_COMPUT_FIRST_ELEMENT)
                   * (SL_BOD_VBATT_COMPUT_SECOND_ELEMENT + SL_BOD_VBATT_COMPUT_THIRD_ELEMENT))
                  - SL_BOD_VBATT_COMPUT_SECOND_ELEMENT;
    threshold_i = (uint32_t)((vbatt_threshold / SL_BOD_VBATT_COMPUT_FIRST_ELEMENT)
                             * (SL_BOD_VBATT_COMPUT_SECOND_ELEMENT + SL_BOD_VBATT_COMPUT_THIRD_ELEMENT))
                  - (uint32_t)SL_BOD_VBATT_COMPUT_SECOND_ELEMENT;
    // Round up the threshold value if necessary
    if (threshold_f - (float)threshold_i > (float)SL_BOD_THRESHOLD_OFFSET_VALUE) {
      threshold_i += SL_BOD_THRESHOLD_ROUNDUP_VALUE;
    }

    // Set the BOD threshold value
    BOD->BOD_COMP_SEL_REG &= (uint32_t)(~0x7E);
    BOD->BOD_COMP_SEL_REG |= (threshold_i << 1);
  } else {
    return SL_STATUS_BOD_THRESHOLD_CONFIG_FAIL;
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Function to configure the BOD slot value
 ******************************************************************************/
sl_status_t sl_si91x_bod_config_slot_value(uint16_t slot_value)
{
  // Check if BOD is not enabled
  if (BOD->BOD_COMP_SEL_REG_b.CMP_5_EN == DISABLE) {
    return SL_STATUS_NOT_INITIALIZED; // Return not initialized status
  }
  // Enable the BOD comparator
  BOD->BOD_COMP_SEL_REG_b.CMP_5_EN = ENABLE;
  // Validate slot value range
  if (slot_value < SL_BOD_SLOT_MIN_VALUE) {
    return SL_STATUS_INVALID_PARAMETER; // Return invalid parameter status
  }
  // Configure the BOD Slot value
  BOD->BOD_COMP_MODE_REG_b.CMP_SLOT_VALUE = SL_ZERO_VAL;
  // Configure the BOD Slot value
  BOD->BOD_COMP_MODE_REG_b.CMP_SLOT_VALUE = slot_value;
  // Check if the slot value is configured correctly
  if (BOD->BOD_COMP_MODE_REG_b.CMP_SLOT_VALUE == slot_value) {
    return SL_STATUS_OK; // Return success status
  } else {
    return SL_STATUS_FAIL; // Return failure status
  }
}

/*******************************************************************************
 * Function to configure button values
 ******************************************************************************/
void sl_si91x_bod_button_configuration(uint16_t button_max_value_,
                                       uint16_t button_1_min_value,
                                       uint16_t button_2_min_value,
                                       uint16_t button_3_min_value)
{
  // Clear the button range values
  BOD->BOD_BUTTON_REG &= SL_BUTTON_RANGE_VALUE_CLR;

  // Set button values
  BOD->BOD_BUTTON_REG_b.BUTTON_MAX_VALUE  = button_max_value_;
  BOD->BOD_BUTTON_REG_b.BUTTON1_MIN_VALUE = button_1_min_value;
  BOD->BOD_BUTTON_REG_b.BUTTON2_MIN_VALUE = button_2_min_value;
  BOD->BOD_BUTTON_REG_b.BUTTON3_MIN_VALUE = button_3_min_value;
}

/*******************************************************************************
 * Function to enable button wakeup
 ******************************************************************************/
void sl_si91x_bod_button_wakeup_enable(uint8_t enable)
{
  volatile uint32_t delay_button = 0, button_loop = 0;
  if (enable == ENABLE) {
    // Calculate delay based on clock selection
    if (SL_BOD_ULP_PROC_CLK_SEL == 0) {
      delay_button = (16 / SL_BOD_CLOCK_DIVISON_FACTOR) + 1;
    } else {
      delay_button = 16;
    }
    // Enable the button wake up for the calculated delay
    for (button_loop = 0; button_loop <= delay_button; button_loop++) {
      BOD->BOD_COMP_SEL_REG_b.BUTTON_WAKEUP_EN = Enable;
    }

    // Selecting and fixing the inputs of comparator
    BOD->BOD_COMP_MODE_REG &= ~SL_BOD_MANUAL_CMP_MUX_SEL_BUTTON_VALUE;

    // Write the comp mux value 5 for button mode
    BOD->BOD_COMP_MODE_REG |= SL_BOD_MANUAL_CMP_MUX_SEL_BUTTON_VALUE;

    // Set the UULPSS GPIO[2] in analog mode (TO GIVE VOLTAGE SUPPLY THROUGH
    // PIN)
    SL_NPSS_GPIO_2_ANALOG_MODE |= SL_NPSS_GPIO_2_ANALOG_MODE_VALUE;
  } else {
    // Check if the comparator mux value is set correctly
    if ((BOD->BOD_COMP_MODE_REG & SL_BOD_MANUAL_CMP_MUX_SEL_BUTTON_CLR) == SL_BOD_MANUAL_CMP_MUX_SEL_BUTTON_VALUE) {
      // Disable manual BOD mode
      BOD->BOD_COMP_MODE_REG_b.MANUAL_CMP_MODE_EN = DISABLE;

      // Disable button mode for the calculated delay
      for (button_loop = 0; button_loop <= delay_button; button_loop++) {
        BOD->BOD_COMP_SEL_REG_b.BUTTON_WAKEUP_EN = DISABLE;
      }
    } else {
      // Disable button mode for the calculated delay
      for (button_loop = 0; button_loop <= delay_button; button_loop++) {
        BOD->BOD_COMP_SEL_REG_b.BUTTON_WAKEUP_EN = DISABLE;
      }
    }
  }
}

/*******************************************************************************
 * Function to enable button wakeup
 ******************************************************************************/
sl_status_t sl_si91x_bod_button_wakeup_enable_v2(boolean_t bod_button_enable)
{

#if (SL_BOD_BUTTON_UC_CONFIG_EN == ENABLE)
  bod_button_enable = button_uc_config_param.button_wakeup_enable;
#endif

  if (bod_button_enable == ENABLE) {
    BOD->BOD_COMP_SEL_REG_b.BUTTON_WAKEUP_EN = Enable;
    // Selecting and fixing the inputs of comparator
    BOD->BOD_COMP_MODE_REG &= ~SL_BOD_MANUAL_CMP_MUX_SEL_BUTTON_VALUE;
    // Write the comp mux value 5 for button mode
    BOD->BOD_COMP_MODE_REG |= SL_BOD_MANUAL_CMP_MUX_SEL_BUTTON_VALUE;
    // Set the UULPSS GPIO[2] in analog mode (TO GIVE VOLTAGE SUPPLY THROUGH PIN)
    SL_NPSS_GPIO_2_ANALOG_MODE |= SL_NPSS_GPIO_2_ANALOG_MODE_VALUE;
    if (BOD->BOD_COMP_SEL_REG_b.BUTTON_WAKEUP_EN == Enable)
      return SL_STATUS_OK;
    else
      return SL_STATUS_FAIL;
  } else
    return SL_STATUS_FAIL;
}

/*******************************************************************************
 * Function to disable button wakeup
 ******************************************************************************/
sl_status_t sl_si91x_bod_button_wakeup_disable(void)
{
  // Check if the comparator mux value is set correctly
  if ((BOD->BOD_COMP_MODE_REG & SL_BOD_MANUAL_CMP_MUX_SEL_BUTTON_CLR) == SL_BOD_MANUAL_CMP_MUX_SEL_BUTTON_VALUE) {
    // Disable manual BOD mode
    BOD->BOD_COMP_MODE_REG_b.MANUAL_CMP_MODE_EN = DISABLE;
    BOD->BOD_COMP_SEL_REG_b.BUTTON_WAKEUP_EN    = DISABLE;

  } else {
    // Disable button mode for the calculated delay
    BOD->BOD_COMP_SEL_REG_b.BUTTON_WAKEUP_EN = DISABLE;
  }
  if (BOD->BOD_COMP_SEL_REG_b.BUTTON_WAKEUP_EN == DISABLE)
    return SL_STATUS_OK;
  else
    return SL_STATUS_FAIL;
}
/*******************************************************************************
 * Function to enable button interrupt
 ******************************************************************************/
sl_status_t sl_si91x_bod_button_interrupt_enable(void)
{
  boolean_t status_read = 0;
  // Enable the button wake up interrupt
  BOD->BOD_COMP_SEL_REG_b.BUTTON_WAKEUP_EN = Enable;
  status_read                              = BOD->BOD_COMP_SEL_REG_b.BUTTON_WAKEUP_EN;
  if (status_read) {
    return SL_STATUS_OK;
  }
  return SL_STATUS_FAIL;
}

/*******************************************************************************
 * Function to disable button interrupt
 ******************************************************************************/
sl_status_t sl_si91x_bod_button_interrupt_disable(void)
{
  boolean_t status_read = 0;
  // Disable the button wake up interrupt
  BOD->BOD_COMP_SEL_REG_b.BUTTON_WAKEUP_EN = DISABLE;
  status_read                              = BOD->BOD_COMP_SEL_REG_b.BUTTON_WAKEUP_EN;
  if (!status_read) {
    return SL_STATUS_OK;
  }
  return SL_STATUS_FAIL;
}

/*******************************************************************************
 * Function to get or read the black out enable/ disable status
 ******************************************************************************/
en_t sl_si91x_bod_get_blackout_status(void)
{
  // Get the blackout enable status
  return BOD->BOD_TEST_PG_VBATT_STATUS_REG_b.BLACKOUT_EN;
}

/*******************************************************************************
 * Function to set BOD configuration
 ******************************************************************************/
sl_status_t sl_si91x_bod_set_configuration(sl_bod_uc_param_t usr_config_params)
{
  sl_status_t status;

#if (SL_BOD_UC_CONFIG_EN == ENABLE)
  usr_config_params = uc_config_params;
#endif
  // Configure the BOD slot value
  status = sl_si91x_bod_config_slot_value(usr_config_params.slot_value);
  if (status != SL_STATUS_OK) {
    return status; // Return if configuring slot value fails
  }

  // Set the BOD mode to automatic
  status = sl_si91x_bod_set_mode(SL_BOD_MODE_AUTOMATIC);
  if (status != SL_STATUS_OK) {
    return status; // Return if setting mode fails
  }

  // Enable button interrupt if button_en is set
  if (usr_config_params.blackout_en == ENABLE) {
    sl_si91x_bod_black_out_reset(ENABLE);
  } else {
    sl_si91x_bod_black_out_reset(DISABLE);
  }

  return SL_STATUS_OK; // Return success status
}

/*******************************************************************************
 * Function to get the BOD threshold value
 ******************************************************************************/
sl_status_t sl_si91x_bod_get_threshold(float *vbatt_threshold)
{
  // Check if the vbatt_threshold pointer is NULL
  if (vbatt_threshold == NULL) {
    return SL_STATUS_INVALID_PARAMETER; // Return invalid parameter status
  }

  // Get the integer part of the threshold from the BOD register
  uint32_t threshold_i = BOD->BOD_COMP_SEL_REG_b.BATT_SCALE_FACTOR;
  // Convert the integer part to float
  float threshold_f = (float)threshold_i;

  // Calculate the battery voltage threshold using the formula
  *vbatt_threshold = ((threshold_f + SL_BOD_VBATT_COMPUT_SECOND_ELEMENT)
                      / (SL_BOD_VBATT_COMPUT_SECOND_ELEMENT + SL_BOD_VBATT_COMPUT_THIRD_ELEMENT))
                     * SL_BOD_VBATT_COMPUT_FIRST_ELEMENT;

  return SL_STATUS_OK; // Return success status
}

/*******************************************************************************
 * Function to enable BOD interrupt in NVIC
 ******************************************************************************/
void sl_si91x_bod_NVIC_enable_irq(void)
{
  NVIC_EnableIRQ(SL_NVIC_BOD);
}

/*******************************************************************************
 * Function to register a callback for BOD
 ******************************************************************************/
sl_status_t sl_si91x_bod_register_callback(bod_callback_t callback)
{
  // Validate instance, if the parameters is NULL, it returns an error code.
  if (callback == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // To validate the function pointer if the parameters is not NULL then, it
  // returns an error code
  if (user_callback != NULL) {
    return SL_STATUS_BUSY;
  }
  // User callback address is passed to the static variable which is called
  // at the time of interrupt
  user_callback = callback;
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Function to register a callback for BOD Button
 ******************************************************************************/
sl_status_t sl_si91x_bod_button_register_callback(bod_button_callback_t callback)
{
  // Validate instance, if the parameters is NULL, it returns an error code.
  if (callback == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // To validate the function pointer if the parameters is not NULL then, it
  // returns an error code
  if (user_button_callback != NULL) {
    return SL_STATUS_BUSY;
  }
  // User callback address is passed to the static variable which is called
  // at the time of interrupt
  user_button_callback = callback; // Register the callback function
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Function to unregister a callback for BOD
 ******************************************************************************/
void sl_si91x_bod_unregister_callback(void)
{
  user_callback = NULL; // Unregister the callback function
}

/*******************************************************************************
 * Function to get the current battery status
 ******************************************************************************/
sl_status_t sl_si91x_bod_get_battery_status(float *battery_status)
{
  // Check if the battery_status pointer is NULL
  if (battery_status == NULL) {
    return SL_STATUS_INVALID_PARAMETER; // Return invalid parameter status
  }

  // Declare and initialize variables to hold battery status and supply values
  static volatile uint32_t batt_status, batt_status_supply = 0;
  static volatile float vbatt = 0;

  // Disable the BOD interrupt
  sl_si91x_bod_disable_interrupt();

  // Enable power gating for calibration block and check vbatt status bit
  BOD->BOD_TEST_PG_VBATT_STATUS_REG_b.BOD_PWRGATE_EN_N_ULP_BUTTON_CALIB = ENABLE;
  BOD->BOD_TEST_PG_VBATT_STATUS_REG_b.CHECK_VBATT_STATUS                = ENABLE;

  // Wait for vbatt status valid bit to be 1
  while (!(BOD->BOD_TEST_PG_VBATT_STATUS_REG_b.VBATT_STATUS_VALID)) {
  }

  // Get battery status value
  batt_status = BOD->BOD_TEST_PG_VBATT_STATUS_REG_b.VBATT_STATUS;

  // Ensure battery status does not exceed maximum value
  if (batt_status > SL_BOD_BATTERY_STATUS_MAX) {
    batt_status = SL_BOD_BATTERY_STATUS_MAX;
  }

  // Calculate the battery voltage based on trim efuse value
  if (vbatt_status_trim_efuse[2]) {
    batt_status_supply = vbatt_status_trim_efuse[2] & SL_BOD_CLEAR_BATTERY_STATUS_BITS_EFUSE;
    vbatt              = SL_BOD_VBATT_COMPUT_FIRST_ELEMENT * (SL_BOD_VBATT_COMPUT_SECOND_ELEMENT + (float)batt_status)
            / (SL_BOD_VBATT_COMPUT_SECOND_ELEMENT + (float)batt_status_supply);
  } else {
    vbatt = SL_BOD_VBATT_COMPUT_FIRST_ELEMENT * (SL_BOD_VBATT_COMPUT_SECOND_ELEMENT + (float)batt_status)
            / (SL_BOD_VBATT_COMPUT_SECOND_ELEMENT + SL_BOD_VBATT_COMPUT_THIRD_ELEMENT);
  }

  // Store the calculated battery voltage in the provided pointer
  *battery_status = vbatt;

  // Clear the BOD interrupt
  sl_si91x_bod_clear_interrupt();

  // Enable the BOD interrupt
  sl_si91x_bod_enable_interrupt();

  return SL_STATUS_OK; // Return success status
}

/*******************************************************************************
 * Function to enable BOD interrupt
 ******************************************************************************/
void sl_si91x_bod_enable_interrupt(void)
{
  // Clear the interrupt mask to enable BOD interrupt
  NPSS_INTR_MASK_CLR_REG = NPSS_TO_MCU_BOD_INTR;
  // Enable the BOD interrupt in the NVIC
  NVIC_EnableIRQ(SL_NVIC_BOD);
}

/*******************************************************************************
 * Function to compute the floor value of a number
 ******************************************************************************/
double sli_my_floor(double x)
{
  // If x is non-negative, simply cast to int to truncate the decimal part
  if (x >= 0) {
    return (double)((int)x);
  } else {
    // For negative numbers, casting to int truncates toward zero,
    // so check if x is already an integer
    int int_x = (int)x;
    // If x is exactly an integer, return it; otherwise, subtract 1 to get the floor
    return (x == int_x) ? (double)int_x : (double)(int_x - 1);
  }
}

/*******************************************************************************
 * Function to Map voltage to register value
 ******************************************************************************/
static uint8_t sli_map_voltage_to_register(float voltage)
{
  float voltage_thresholds[SL_BOD_MAX_BUFF_SIZE] = { 0 };
  float vbatt                                    = SL_BOD_VBATT_STATUS_MIN;

  float resistor_bank_fraction_values[SL_BOD_MAX_BUFF_SIZE] = {
    0.689655172, 0.670398552, 0.652188091, 0.634940792, 0.61858221,  0.603045379, 0.588269898, 0.574201143,
    0.560789592, 0.547990246, 0.535762122, 0.524067814, 0.512873115, 0.502146677, 0.491859722, 0.481985781,
    0.472500473, 0.463381293, 0.454607446, 0.446159681, 0.438020149, 0.430172284, 0.422600685, 0.415291015,
    0.408229915, 0.401404917, 0.394804374, 0.388417393, 0.382233774, 0.376243957, 0.37043897,  0.36481039,
    0.359350295, 0.354051231, 0.348906179, 0.34390852,  0.339052011, 0.334330753, 0.329739176
  };
  if (sl_si91x_bod_get_battery_status(&vbatt) != SL_STATUS_OK) {
    return (uint8_t)SL_STATUS_FAIL; // VBATT read failed
  }
  int size = sizeof(resistor_bank_fraction_values) / sizeof(resistor_bank_fraction_values[0]);
  // Precompute floor values outside the loop
  for (int i = 0; i < size; i++) {
    voltage_thresholds[i] =
      (double)((int)(resistor_bank_fraction_values[i] * vbatt * 100)) / 100; // Optimized ROUNDDOWN logic
  }
  // Check if the voltage is within the valid range
  if (voltage < voltage_thresholds[SL_BOD_MAX_BUFF_SIZE - 1] || voltage > voltage_thresholds[SL_BOD_MIN_BUFF_SIZE]) {
    return (uint8_t)SL_STATUS_INVALID_PARAMETER; // Default for invalid voltages
  }
  for (uint8_t i = 0; i < SL_BOD_MAX_BUFF_SIZE; i++) {
    if (voltage >= voltage_thresholds[i]) {
      return (i == 0) ? 1 : i;
    }
  }
  return (uint8_t)SL_STATUS_INVALID_PARAMETER; // Default for invalid voltages
  return (uint8_t)SL_STATUS_FAIL;
}

/*******************************************************************************
 * Function to configure BOD voltage
 ******************************************************************************/
sl_status_t sl_si91x_bod_button_set_configuration(sl_bod_button_uc_config_param_t uc_config_param,
                                                  uint8_t *button_max_value,
                                                  uint8_t *button_1_min_value,
                                                  uint8_t *button_2_min_value,
                                                  uint8_t *button_3_min_value)
{
  sl_status_t status = SL_STATUS_FAIL;
  float vbatt        = 0;
#if (SL_BOD_BUTTON_UC_CONFIG_EN == ENABLE)
  uc_config_param = button_uc_config_param;
#endif

  if (uc_config_param.button_wakeup_enable) {
    // Get the current battery voltage
    if (sl_si91x_bod_get_battery_status(&vbatt) != SL_STATUS_OK) {
      return SL_STATUS_FAIL; // Exit if unable to retrieve battery voltage
    }

    // Calculate actual voltage thresholds based on percentages
    float button_1_min_voltage = (uc_config_param.button_1_min_voltage_percentage / 100.0f) * vbatt;
    float button_1_max_voltage = (uc_config_param.button_1_max_voltage_percentage / 100.0f) * vbatt;
    float button_2_min_voltage = (uc_config_param.button_2_min_voltage_percentage / 100.0f) * vbatt;
    float button_2_max_voltage = (uc_config_param.button_2_max_voltage_percentage / 100.0f) * vbatt;
    float button_3_min_voltage = (uc_config_param.button_3_min_voltage_percentage / 100.0f) * vbatt;
    float button_3_max_voltage = (uc_config_param.button_3_max_voltage_percentage / 100.0f) * vbatt;
    // Map voltage ranges to register values
    *button_1_min_value = sli_map_voltage_to_register(button_1_max_voltage);
    *button_2_min_value = sli_map_voltage_to_register(button_2_max_voltage);
    *button_3_min_value = sli_map_voltage_to_register(button_3_max_voltage);

    uint8_t button_1_max_value = sli_map_voltage_to_register(button_1_min_voltage);
    uint8_t button_3_max_value = sli_map_voltage_to_register(button_3_min_voltage);

    if (*button_2_min_value <= button_1_max_value)
      *button_2_min_value = button_1_max_value + 1;

    uint8_t button_2_max_value = sli_map_voltage_to_register(button_2_min_voltage);

    if (*button_3_min_value <= button_2_max_value)
      *button_3_min_value = button_2_max_value + 1;

    // Calculate the maximum button value
    *button_max_value = ((button_2_max_value - *button_2_min_value) + (button_3_max_value - *button_3_min_value)
                         + (button_1_max_value - *button_1_min_value))
                        / SL_BOD_BUTTON_TOTAL_NUM_OF_BUTTONS;

    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * Function to enable BOD button interrupt
 ******************************************************************************/
void sl_si91x_bod_button_enable_interrupt(void)
{
  // Clear the interrupt mask to enable BOD interrupt
  NPSS_INTR_MASK_CLR_REG = NPSS_TO_MCU_BUTTON_INTR;
  // BOD NVIC enable
  NVIC_EnableIRQ(SL_BOD_NVIC_BUTTON);
}
/*******************************************************************************
 * Function to disable BOD button interrupt
 ******************************************************************************/
void sl_si91x_bod_button_disable_interrupt(void)
{
  // Clear the interrupt mask to enable BOD interrupt
  NPSS_INTR_MASK_SET_REG = NPSS_TO_MCU_BUTTON_INTR;
  // BOD NVIC enable
  NVIC_DisableIRQ(SL_BOD_NVIC_BUTTON);
}

/*******************************************************************************
 * Function to disable BOD interrupt
 ******************************************************************************/
void sl_si91x_bod_disable_interrupt(void)
{
  // Set the interrupt mask to disable BOD interrupt
  NPSS_INTR_MASK_SET_REG = NPSS_TO_MCU_BOD_INTR;
  NVIC_DisableIRQ(SL_NVIC_BOD);
}

/*******************************************************************************
 * Function to clear BOD interrupt
 ******************************************************************************/
void sl_si91x_bod_clear_interrupt(void)
{
  // Clear the wakeup status for comparator 5
  MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = COMP5_BASED_WAKEUP_STATUS_CLEAR;
  // Clear the BOD interrupt in the NPSS interrupt clear register
  NPSS_INTR_CLEAR_REG = NPSS_TO_MCU_BOD_INTR;
}

/*******************************************************************************
 * Function to clear button interrupt
 ******************************************************************************/
void sl_si91x_bod_button_clear_interrupt(void)
{
  /* clear the button  interrupt */
  MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = COMP6_BASED_WAKEUP_STATUS_CLEAR;
  NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_BUTTON_INTR;
}

/*******************************************************************************
 * Function to read the button value
 ******************************************************************************/
uint8_t sl_si91x_bod_button_value_read(void)
{
  // Check if the button wakeup status is enabled
  if (BOD->BOD_TEST_PG_VBATT_STATUS_REG_b.BUTTON1_WAKEUP)
    return SL_BOD_BUTTON_ONE;
  else if (BOD->BOD_TEST_PG_VBATT_STATUS_REG_b.BUTTON2_WAKEUP)
    return SL_BOD_BUTTON_TWO;
  else if ((BOD->BOD_TEST_PG_VBATT_STATUS_REG_b.BUTTON3_WAKEUP))
    return SL_BOD_BUTTON_THREE;
  else
    return 0;
}

/*******************************************************************************
 * Function to set the BOD mode
 ******************************************************************************/
sl_status_t sl_si91x_bod_set_mode(sl_bod_mode_t mode)
{
  // Check if the mode is valid
  if (mode >= SL_BOD_MAX_MODE_VALUE) {
    return SL_STATUS_INVALID_PARAMETER; // Return invalid parameter status
  }
  // Enable the BOD comparator
  BOD->BOD_COMP_SEL_REG_b.CMP_5_EN = ENABLE;
  // Wait for the synchronization reset to complete
  while (!(BOD->BOD_BUTTON_REG_b.SYNC_RESET_READ)) {
  }

  // Set the BOD mode based on the provided mode
  if (mode == SL_BOD_MODE_AUTOMATIC) {
    BOD->BOD_COMP_MODE_REG_b.MANUAL_CMP_MODE_EN = DISABLE; // Disable manual mode
    BOD->BOD_COMP_MODE_REG_b.AUTO_CMP_MODE_EN   = ENABLE;  // Enable automatic mode
  } else {
    BOD->BOD_COMP_MODE_REG_b.AUTO_CMP_MODE_EN   = DISABLE; // Disable automatic mode
    BOD->BOD_COMP_MODE_REG_b.MANUAL_CMP_MODE_EN = ENABLE;  // Enable manual mode
  }

  return SL_STATUS_OK; // Return success status
}
/*******************************************************************************
 * Function to Enable Blackout reset
 ******************************************************************************/
void sl_si91x_bod_black_out_reset(en_t enable)
{
  if (enable == ENABLE) {
    /* Enables black out in active state and when brown out is detected.*/
    BOD->BOD_TEST_PG_VBATT_STATUS_REG_b.BLACKOUT_EN = ENABLE;
    sl_si91x_bod_enable_blackout_in_sleep_mode();
  } else {
    /* Disables black out in active state and when brown out is detected.*/
    BOD->BOD_TEST_PG_VBATT_STATUS_REG_b.BLACKOUT_EN = DISABLE;
    sl_si91x_bod_disable_blackout_in_sleep_mode();
  }
}

/*******************************************************************************
 * Function to calculate the battery percentage
 ******************************************************************************/
sl_status_t sl_si91x_bod_battery_percentage(float voltage, float *vbat_per)
{
  if (vbat_per == NULL) {
    return SL_STATUS_INVALID_PARAMETER; // Check for NULL pointer
  }

  if ((voltage >= SL_BOD_MIN_BATTERY_VOLTAGE) && (voltage <= SL_BOD_MAX_BATTERY_VOLTAGE)) {
    // Linear interpolation formula
    *vbat_per =
      ((voltage - SL_BOD_MIN_BATTERY_VOLTAGE) / (SL_BOD_MAX_BATTERY_VOLTAGE - SL_BOD_MIN_BATTERY_VOLTAGE)) * 100.0f;
  } else if (voltage >= SL_BOD_MAX_BATTERY_VOLTAGE) {
    *vbat_per = 100.0f; // Clamp to 100% if voltage is greater than max voltage
  } else {
    *vbat_per = 0.0f; // Clamp to 0% if voltage is less than min voltage
    return SL_STATUS_INVALID_PARAMETER;
  }

  return SL_STATUS_OK;
}

/*******************************************************************************
 * IRQ Handler for BOD
 ******************************************************************************/
void BOD_IRQ_Handler(void)
{
  // Check if a user callback is registered
  if (user_callback != NULL) {
    // Call the registered user callback function
    user_callback();
  }
  // Clear the BOD interrupt
  sl_si91x_bod_clear_interrupt();
}

/*******************************************************************************
 * Function to clear the button interrupt
 ******************************************************************************/
void sl_si91x_bod_button_interrupt_clear(void)
{
  /* clear the button  interrupt */
  MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = COMP6_BASED_WAKEUP_STATUS_CLEAR;
  NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_BUTTON_INTR;
}

/*******************************************************************************
 * IRQ Handler for BOD Button
 ******************************************************************************/
void SL_BOD_NPSS_TO_MCU_BUTTON_IRQHandler(void)
{
  if (user_button_callback != NULL) {
    // Call the registered user callback function
    user_button_callback();
  }
  /*Clear the Button interrupt */
  sl_si91x_bod_button_interrupt_clear();
}

/*******************************************************************************
 * This API is used to Enable the blackout reset in sleep mode
 ******************************************************************************/
void sl_si91x_bod_enable_blackout_in_sleep_mode(void)
{
  // Enable low power mode for SCDCDC in sleep mode
  MCU_FSM->MCU_FSM_PMU_STATUS_REG |= SL_SCDC_IN_LP_MODE_EN;
  // Enable background PMU sleep mode
  MCU_FSM->MCU_FSM_PMU_STATUS_REG_b.BGPMU_SLEEP_EN_R_b = DISABLE;
}

/*******************************************************************************
 * This API is used to Disable the blackout reset in sleep mode
 ******************************************************************************/
void sl_si91x_bod_disable_blackout_in_sleep_mode(void)
{
  // Disable low power mode for SCDCDC in sleep mode
  MCU_FSM->MCU_FSM_PMU_STATUS_REG &= ~(SL_SCDC_IN_LP_MODE_EN);
  // Disable background PMU sleep mode
  MCU_FSM->MCU_FSM_PMU_STATUS_REG_b.BGPMU_SLEEP_EN_R_b = DISABLE;
}
