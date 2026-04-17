/******************************************************************************
* @file sl_si91x_opamp.c
* @brief OPAMP API implementation
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
#include "sl_si91x_opamp.h"
#include "clock_update.h"
#include "rsi_bod.h"
#include "rsi_rom_ulpss_clk.h"
#include "aux_reference_volt_config.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_si91x_opamp_common_config.h"
#include "sl_si91x_opamp_init.h"
#include "sl_si91x_dac.h"
#if (defined(SL_OPAMP_OPAMP1) && defined(SL_OPAMP_OPAMP2)) && defined(SL_OPAMP_OPAMP3)
#include "sl_si91x_dac_config.h"
#endif
/*******************************************************************************
   ***************************  LOCAL MACROS   ***********************************
   ******************************************************************************/
// Reference voltage of OPAMP, Min is 1.6V and Max is 3.3V
#define SLI_OPAMP_MIN_BATTERY 1.6f // Minimum battery voltage
#define SLI_OPAMP_MAX_BATTERY 3.3f // Maximum battery voltage

// Defines macros for OPAMP pin selections, input/output configurations, and operational modes.
#define SL_OPAMP_OUTPUT_MUX_ENABLE 1 // OPAMP output mux enable
#define SL_OPAMP_DYN_MODE_EN       0 // OPAMP Dynamic mode
#define SL_OPAMP_CHANNEL_NO        0 // adc channel no in dynamic mode
// OPAMP1 specific configuration for instrumentation amplifier
#define SL_OPAMP1_INST_AMP_OUT_MUX_EN     0 // OPAMP1 output mux
#define SL_OPAMP1_INST_AMP_RES_TO_OUT_VDD 0 // OPAMP1 resistor to VDD
#define SL_OPAMP1_INST_AMP_EN_RES_BANK    0 // OPAMP1 resistor bank

// OPAMP2 specific configuration for instrumentation amplifier
#define SL_OPAMP2_INST_AMP_OUT_MUX_EN     0 // OPAMP2 output mux
#define SL_OPAMP2_INST_AMP_RES_TO_OUT_VDD 2 // OPAMP2 resistor to VDD
#define SL_OPAMP2_INST_AMP_EN_RES_BANK    1 // OPAMP2 resistor bank

// OPAMP3 specific configuration for instrumentation amplifier
#define SL_OPAMP3_INST_AMP_OUT_MUX_EN     1 // OPAMP3 output mux
#define SL_OPAMP3_INST_AMP_RES_TO_OUT_VDD 0 // OPAMP3 resistor to VDD
#define SL_OPAMP3_INST_AMP_EN_RES_BANK    1 // OPAMP3 resistor bank
typedef sl_gpio_t sl_si91x_gpio_t;

/*******************************************************************************
  ***********************  LOCAL FUNCTION Definitions *************************
  ******************************************************************************/

static void sli_si91x_opamp_configure_unity_gain_features(uint8_t opamp_config_inst, sl_opamp_config_t *user_config);
static void sli_si91x_opamp_configure_inverting_pga(uint8_t opamp_config_inst, sl_opamp_config_t *user_config);
static void sli_si91x_opamp_configure_non_inverting_pga(uint8_t opamp_config_inst, sl_opamp_config_t *user_config);
static void sli_si91x_opamp_configure_inverting_hyst_comp(uint8_t opamp_config_inst, sl_opamp_config_t *user_config);
static void sli_si91x_opamp_configure_non_inverting_hyst_comp(uint8_t opamp_config_inst,
                                                              sl_opamp_config_t *user_config);
static sl_status_t sli_si91x_opamp_configure_cascaded_inverting_pga(uint8_t opamp_config_inst,
                                                                    sl_opamp_config_t *user_config);
static sl_status_t sli_si91x_opamp_configure_cascaded_non_inverting_pga(uint8_t opamp_config_inst,
                                                                        sl_opamp_config_t *user_config);
static sl_status_t sli_si91x_opamp_configure_two_opamps_differential_amp(uint8_t opamp_config_inst,
                                                                         sl_opamp_config_t *user_config);
static void sli_si91x_opamp_configure_trans_impedance_amp(uint8_t opamp_config_inst, sl_opamp_config_t *user_config);
static sl_status_t sli_si91x_opamp_configure_instrumentation_amp(uint8_t opamp_config_inst,
                                                                 sl_opamp_config_t *user_config);
static sl_status_t get_battery_status(float *battery_status);
STATIC INLINE void fill_opamp_gpio_pin_config(sl_si91x_gpio_pin_config_t *pin_cfg, uint8_t pin, uint8_t direction);
static sl_status_t sli_si91x_opamp_configure_opamp_gpio(const sl_si91x_gpio_pin_config_t *pin_cfg);
static void sli_si91x_opamp_non_uc_config(Configure_OPAMP_t *all_config, const sl_opamp_config_t *user_config);
STATIC INLINE void fill_opamp_uc_config(Configure_OPAMP_t *opamp1_config);
STATIC INLINE void fill_opamp1_opamp2_uc_config(Configure_OPAMP1_OPAMP2_t *opamp12_config,
                                                sl_opamp_config_t *user_config);
STATIC INLINE void fill_opamp2_opamp3_uc_config(Configure_OPAMP2_OPAMP3_t *opamp23_config,
                                                sl_opamp_config_t *user_config);

#if (defined(SL_OPAMP_OPAMP1) && defined(SL_OPAMP_OPAMP2)) && defined(SL_OPAMP_OPAMP3)
// Define DAC Macros
#define DAC_SAMPLING_RATE          5000000
#define NUMBER_OF_INPUT_SAMPLE     1
#define MAX_DAC_INPUT_SAMPLE_VALUE 1023
#define DAC_INPUT_SAMPLE_VALUE     1023
static const int16_t dac_instrument_input_sample_data[1] = { 0x1FF };
static boolean_t dac_instrument_static_intr_flag         = false;
static boolean_t dac_instrument_fifo_intr_flag           = false;
static void dac_instrument_callback_event(uint8_t event);
#endif

/*******************************************************************************
 * @brief: Configure OPAMP1 when UC is enabled
 * @details: Configures OPAMP1 based on user configuration when UC is enabled
 *******************************************************************************/
STATIC INLINE void fill_opamp_uc_config(Configure_OPAMP_t *opamp1_config)
{
  (void)opamp1_config;
#if (OPAMP1_UC == ENABLE)
  opamp1_config->vin_p_sel   = SL_OPAMP1_VINP_SEL;
  opamp1_config->vin_n_sel   = SL_OPAMP1_VINN_SEL;
  opamp1_config->vref_sel    = SL_OPAMP1_VREF_SEL;
  opamp1_config->enable      = ENABLE;
  opamp1_config->lp_mode     = SL_OPAMP1_LP_MODE;
  opamp1_config->r1_sel      = SL_OPAMP1_R1_SELECT;
  opamp1_config->r2_sel      = SL_OPAMP1_R2_SELECT;
  opamp1_config->out_mux_en  = SL_OPAMP_OUTPUT_MUX_ENABLE;
  opamp1_config->out_mux_sel = SL_OPAMP_OUT_MUX_SEL;
#endif
}

/*******************************************************************************
 * @brief: Configure cascaded OPAMP1,2 when UC is disabled
 * @details: Configures OPAMP based on user configuration when UC is disabled
 *******************************************************************************/
STATIC INLINE void fill_opamp1_opamp2_uc_config(Configure_OPAMP1_OPAMP2_t *opamp12_config,
                                                sl_opamp_config_t *user_config)
{
  // Cascaded OPAMP1,2 config
  opamp12_config->vin1_p_sel    = user_config->opamp1_opamp2_config.vin1_p_sel;
  opamp12_config->vin1_n_sel    = user_config->opamp1_opamp2_config.vin1_n_sel;
  opamp12_config->vref1_sel     = user_config->opamp1_opamp2_config.vref1_sel;
  opamp12_config->lp_mode       = user_config->opamp1_opamp2_config.lp_mode;
  opamp12_config->opamp1_r1_sel = user_config->opamp1_opamp2_config.opamp1_r1_sel;
  opamp12_config->opamp1_r2_sel = user_config->opamp1_opamp2_config.opamp1_r2_sel;
  opamp12_config->out_mux_sel   = user_config->opamp1_opamp2_config.out_mux_sel;
  opamp12_config->vin2_p_sel    = user_config->opamp1_opamp2_config.vin2_p_sel;
  opamp12_config->vin2_n_sel    = user_config->opamp1_opamp2_config.vin2_n_sel;
  opamp12_config->vref2_sel     = user_config->opamp1_opamp2_config.vref2_sel;
  opamp12_config->opamp2_r1_sel = user_config->opamp1_opamp2_config.opamp2_r1_sel;
  opamp12_config->opamp2_r2_sel = user_config->opamp1_opamp2_config.opamp2_r2_sel;
  opamp12_config->lp_mode       = user_config->opamp1_opamp2_config.lp_mode;
}

/*******************************************************************************
 * @brief: Configure cascaded OPAMP2,3 when UC is disabled
 * @details: Configures OPAMP based on user configuration when UC is disabled
 *******************************************************************************/
STATIC INLINE void fill_opamp2_opamp3_uc_config(Configure_OPAMP2_OPAMP3_t *opamp23_config,
                                                sl_opamp_config_t *user_config)
{
  // Cascaded OPAMP2,3 config
  opamp23_config->vin2_p_sel    = user_config->opamp2_opamp3_config.vin2_p_sel;
  opamp23_config->vin2_n_sel    = user_config->opamp2_opamp3_config.vin2_n_sel;
  opamp23_config->vref2_sel     = user_config->opamp2_opamp3_config.vref2_sel;
  opamp23_config->lp_mode       = user_config->opamp2_opamp3_config.lp_mode;
  opamp23_config->opamp2_r1_sel = user_config->opamp2_opamp3_config.opamp2_r1_sel;
  opamp23_config->opamp2_r2_sel = user_config->opamp2_opamp3_config.opamp2_r2_sel;
  opamp23_config->out_mux_sel   = user_config->opamp2_opamp3_config.out_mux_sel;
  opamp23_config->vin3_p_sel    = user_config->opamp2_opamp3_config.vin3_p_sel;
  opamp23_config->vin3_n_sel    = user_config->opamp2_opamp3_config.vin3_n_sel;
  opamp23_config->vref3_sel     = user_config->opamp2_opamp3_config.vref3_sel;
  opamp23_config->opamp3_r1_sel = user_config->opamp2_opamp3_config.opamp3_r1_sel;
  opamp23_config->opamp3_r2_sel = user_config->opamp2_opamp3_config.opamp3_r2_sel;
  opamp23_config->lp_mode       = user_config->opamp2_opamp3_config.lp_mode;
}

/*******************************************************************************
 * @brief: Configure OPAMP when UC is disabled
 * @details: Configures OPAMP based on user configuration when UC is disabled
 *******************************************************************************/
static void sli_si91x_opamp_non_uc_config(Configure_OPAMP_t *all_config, const sl_opamp_config_t *user_config)
{
  all_config->vin_p_sel   = user_config->opamp_config.vin_p_sel;
  all_config->vin_n_sel   = user_config->opamp_config.vin_n_sel;
  all_config->vref_sel    = user_config->opamp_config.vref_sel;
  all_config->lp_mode     = user_config->opamp_config.lp_mode;
  all_config->r1_sel      = user_config->opamp_config.r1_sel;
  all_config->r2_sel      = user_config->opamp_config.r2_sel;
  all_config->out_mux_sel = user_config->opamp_config.out_mux_sel;
  all_config->enable      = ENABLE;
  all_config->out_mux_en  = SL_OPAMP_OUTPUT_MUX_ENABLE;
}

/*******************************************************************************
  * @brief: Initializes OPAMP peripheral
  * @details: This API enables the auxiliary clock using the MHz RC clock
  *           and configures the OPAMP reference voltage to either 2.5V or 3.3V
  *******************************************************************************/
sl_status_t sl_si91x_opamp_init(void)
{
  sl_status_t status;
  status               = SL_STATUS_OK;
  float battery_status = 0.0f;
  float vref_value     = ((float)OPAMP_REF_VOLT / 1000); // OPAMP Reference voltage from UC change into float
  // Get battery status
  status = get_battery_status(&battery_status);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Enable Powerup of AUX
  RSI_PS_UlpssPeriPowerUp(ULPSS_PWRGATE_ULP_AUX);

  // Check if the analog power state is off (0), and if so, configure the AUX clock source
  // to use the ULP AUX MHz RC clock in static mode. This ensures the AUX clock is properly
  // set up before enabling the OPAMP peripheral.

  if (analog_get_power_state() == 0) {
    RSI_ULPSS_AuxClkConfig(ULPCLK, ENABLE_STATIC_CLK, ULP_AUX_MHZ_RC_CLK);
  }
  analog_set_power_state(OPAMP_BIT_POS, ANALOG_POWERED_ON);

  // enable ptat currents to analog peripherals
  ULP_SPI_MEM_MAP(BG_SCDC_PROG_REG_1) |= BIT(3);
  // Configure the opamp reference voltage as 2.5 V or 3.3V
  RSI_AUX_RefVoltageConfig(vref_value, battery_status);
  return status;
}
/*******************************************************************************
  * @brief: Get battery voltage level status
  * @details: This API reads the current battery voltage and checks if it is within the valid range (1.6V to 3.3V)
  *******************************************************************************/
static sl_status_t get_battery_status(float *battery_status)
{

  if (battery_status == NULL) {
    return SL_STATUS_NULL_POINTER; // Return error if pointer is NULL
  }
  // Get the battery voltage value (float) from the RSI function
  float vbatt = RSI_BOD_SoftTriggerGetBatteryStatus();

  // Check if the battery voltage is within the valid range
  if ((vbatt >= SLI_OPAMP_MIN_BATTERY) && (vbatt <= SLI_OPAMP_MAX_BATTERY)) {
    *battery_status = vbatt; // Store the calculated battery voltage in pointer
    return SL_STATUS_OK;     // Return success status
  } else {
    return SL_STATUS_INVALID_PARAMETER; // Return invalid parameter status
  }
}

/*******************************************************************************
 * @brief  Set OPAMP GPIO pin configuration
 * @details Helper function to assign port, pin, and direction for OPAMP input/output pins
 ******************************************************************************/
STATIC INLINE void fill_opamp_gpio_pin_config(sl_si91x_gpio_pin_config_t *pin_cfg, uint8_t pin, uint8_t direction)
{
  uint8_t port           = (pin == TGPIO_PIN2 || pin == TGPIO_PIN4 || pin == TGPIO_PIN5) ? 0 : 4;
  pin_cfg->port_pin.port = port;
  pin_cfg->port_pin.pin  = pin;
  pin_cfg->direction     = direction;
}

/*******************************************************************************
 * @brief  Configure OPAMP GPIO pin with common conditions
 * @details Sets pad driver, disables pad receiver, and sets pin mode for OPAMP input/output pins
 ******************************************************************************/
static sl_status_t sli_si91x_opamp_configure_opamp_gpio(const sl_si91x_gpio_pin_config_t *pin_cfg)
{
  sl_status_t status = SL_STATUS_FAIL;
  sl_gpio_t gpio;
  gpio.port = pin_cfg->port_pin.port;
  gpio.pin  = pin_cfg->port_pin.pin;

  status = sl_gpio_set_configuration(*pin_cfg);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  if ((gpio.pin == TGPIO_PIN2) || (gpio.pin == TGPIO_PIN4) || (gpio.pin == TGPIO_PIN5)) {
    status = sl_si91x_gpio_driver_select_pad_driver_disable_state(pin_cfg->port_pin.pin,
                                                                  (sl_si91x_gpio_driver_disable_state_t)GPIO_HZ);
    if (status != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }
    status = sl_si91x_gpio_driver_disable_pad_receiver(pin_cfg->port_pin.pin);
    if (status != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }
    status = sl_gpio_driver_set_pin_mode(&gpio, TGPIO_MODE, 0);
    if (status != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }
  } else {
    status = sl_si91x_gpio_driver_select_ulp_pad_driver_disable_state(pin_cfg->port_pin.pin,
                                                                      (sl_si91x_gpio_driver_disable_state_t)GPIO_HZ);
    if (status != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }
    status = sl_si91x_gpio_driver_disable_ulp_pad_receiver(pin_cfg->port_pin.pin);
    if (status != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }
    status = sl_gpio_driver_set_pin_mode(&gpio, AGPIO_MODE, 0);
    if (status != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
  * @brief: Set OPAMP GPIO pins for input and output
  * @details: This API is used to validates input/output parameters and initialize OPAMP pins
  *******************************************************************************/
sl_status_t sl_si91x_opamp_pin_init(sl_opamp_pin_config_t *opamp_config_ptr)
{
  sl_status_t status = SL_STATUS_FAIL;
  sl_si91x_gpio_pin_config_t sl_gpio_pin_vinp, sl_gpio_pin_vinn, sl_gpio_pin_vinres, sl_gpio_pin_out;
  // Validate the structure pointer
  if (opamp_config_ptr == NULL) {
    status = SL_STATUS_NULL_POINTER; // Return error if pointer is NULL
    return status;
  }
  fill_opamp_gpio_pin_config(&sl_gpio_pin_vinp, opamp_config_ptr->vin_p_input, GPIO_INPUT);
  fill_opamp_gpio_pin_config(&sl_gpio_pin_vinn, opamp_config_ptr->vin_n_input, GPIO_INPUT);
  fill_opamp_gpio_pin_config(&sl_gpio_pin_vinres, opamp_config_ptr->vin_res_input, GPIO_INPUT);
  fill_opamp_gpio_pin_config(&sl_gpio_pin_out, opamp_config_ptr->vout_output, GPIO_OUTPUT);

  // Configure GPIO pins for OPAMP Pin config.
  status = sl_gpio_driver_init(); // Initialize the GPIO driver
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  sli_si91x_opamp_configure_opamp_gpio(&sl_gpio_pin_vinp);
  sli_si91x_opamp_configure_opamp_gpio(&sl_gpio_pin_vinn);
  sli_si91x_opamp_configure_opamp_gpio(&sl_gpio_pin_vinres);
  sli_si91x_opamp_configure_opamp_gpio(&sl_gpio_pin_out);

  return status; // Return success status
}

/*******************************************************************************
  * @brief: Configures OPAMP features based on the selected instance number
  * @details: This API is used to configure OPAMP by selecting feature based on the opamp instance number
  *******************************************************************************/
sl_status_t sl_si91x_opamp_set_configuration(sl_opamp_config_t *opamp_config)
{
  // Making these void as they are used conditionally when UC is disabled
  (void)sli_si91x_opamp_non_uc_config;
  (void)fill_opamp_uc_config;
  (void)fill_opamp1_opamp2_uc_config;
  (void)fill_opamp2_opamp3_uc_config;
  sl_status_t status;
  status = SL_STATUS_OK;
  // Override with UC configuration when enabled
  if (opamp_config->opamp_number == SL_OPAMP_1) {
#if (OPAMP1_UC == ENABLE)
    opamp_config = &sl_opamp1_feature;
#endif
  } else if (opamp_config->opamp_number == SL_OPAMP_2) {
#if (OPAMP2_UC == ENABLE)
    opamp_config = &sl_opamp2_feature;
#endif
  } else if (opamp_config->opamp_number == SL_OPAMP_3) {
#if (OPAMP3_UC == ENABLE)
    opamp_config = &sl_opamp3_feature;
#endif
  }
  // Validate opamp number
  if ((opamp_config->opamp_number >= SL_OPAMP_LAST) || (opamp_config->features >= SL_OPAMP_CONFIGURATION_LAST)) {
    status = SL_STATUS_INVALID_PARAMETER; // Return error if invalid parameter
    return status;
  }

  switch (opamp_config->features) {
    // Set the OPAMP Unity Gain feature
    case SL_OPAMP_UNITY_GAIN:
      sli_si91x_opamp_configure_unity_gain_features(opamp_config->opamp_number, opamp_config);
      break;

    // Set the OPAMP Inverting Programmable Gain Amplifier feature
    case SL_OPAMP_INVERTING_PROGRAMMABLE_GAIN_AMPLIFIER:
      sli_si91x_opamp_configure_inverting_pga(opamp_config->opamp_number, opamp_config);
      break;

    // Set the OPAMP Non-Inverting Programmable Gain Amplifier feature
    case SL_OPAMP_NON_INVERTING_PROGRAMMABLE_GAIN_AMPLIFIER:
      sli_si91x_opamp_configure_non_inverting_pga(opamp_config->opamp_number, opamp_config);
      break;

    // Set the OPAMP Inverting Programmable Hysteresis Comparator feature
    case SL_OPAMP_INVERTING_PROGRAMMABLE_HYST_COMP:
      sli_si91x_opamp_configure_inverting_hyst_comp(opamp_config->opamp_number, opamp_config);
      break;

    // Set the OPAMP Non-Inverting Programmable Hysteresis Comparator feature
    case SL_OPAMP_NON_INVERTING_PROGRAMMABLE_HYST_COMP:
      sli_si91x_opamp_configure_non_inverting_hyst_comp(opamp_config->opamp_number, opamp_config);
      break;

    // Set the OPAMP Cascaded Inverting Programmable Gain Amplifier feature
    case SL_OPAMP_CASCADED_INVERTING_PROGRAMMABLE_GAIN_AMPLIFIER:
      status = sli_si91x_opamp_configure_cascaded_inverting_pga(opamp_config->opamp_number, opamp_config);
      break;

    // Set the OPAMP Cascaded Non-Inverting Programmable Gain Amplifier feature
    case SL_OPAMP_CASCADED_NON_INVERTING_PROGRAMMABLE_GAIN_AMPLIFIER:
      status = sli_si91x_opamp_configure_cascaded_non_inverting_pga(opamp_config->opamp_number, opamp_config);
      break;

    // Set the OPAMP Two Opamps Differential Amplifier feature
    case SL_OPAMP_TWO_OPAMPS_DIFFERENTIAL_AMPLIFIER:
      status = sli_si91x_opamp_configure_two_opamps_differential_amp(opamp_config->opamp_number, opamp_config);
      break;

    // Set the OPAMP Trans-impedance Amplifier feature
    case SL_OPAMP_TRANS_IMPEDANCE_AMPLIFIER:
      sli_si91x_opamp_configure_trans_impedance_amp(opamp_config->opamp_number, opamp_config);
      break;

    // Set the OPAMP Instrumentation Amplifier feature
    case SL_OPAMP_INSTRUMENTATION_AMPLIFIER:
      status = sli_si91x_opamp_configure_instrumentation_amp(opamp_config->opamp_number, opamp_config);
      break;

    default:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
  }
  return status;
}

/*******************************************************************************
  * @brief: Configure OPAMP for Unity Gain Features
  * @details: Configures OPAMP for unity gain features based on the instance number
  *******************************************************************************/
static void sli_si91x_opamp_configure_unity_gain_features(uint8_t opamp_config_inst, sl_opamp_config_t *user_config)
{
  (void)user_config;
  if (opamp_config_inst == SL_OPAMP_1) {
    // Configure OPAMP1 for unity gain if macros are defined
#ifdef SL_OPAMP1_VINP_SEL
    Configure_OPAMP_t opamp1_config = { 0 };
#if (OPAMP1_UC == ENABLE)
    opamp1_config.vin_p_sel   = SL_OPAMP1_VINP_SEL;
    opamp1_config.vin_n_sel   = SL_OPAMP1_VINN_SEL;
    opamp1_config.vref_sel    = SL_OPAMP1_VREF_SEL;
    opamp1_config.enable      = ENABLE;
    opamp1_config.lp_mode     = SL_OPAMP1_LP_MODE;
    opamp1_config.out_mux_en  = SL_OPAMP_OUTPUT_MUX_ENABLE;
    opamp1_config.out_mux_sel = SL_OPAMP_OUT_MUX_SEL;
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp1_config, user_config);
    }
#endif
    RSI_OPAMP_UGB_V2(opamp_config_inst, &opamp1_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  } else if (opamp_config_inst == SL_OPAMP_2) {
    // Configure OPAMP2 for unity gain if macros are defined
#ifdef SL_OPAMP2_VINP_SEL
    Configure_OPAMP_t opamp2_config = { 0 };
#if (OPAMP2_UC == ENABLE)
    opamp2_config.vin_p_sel  = SL_OPAMP2_VINP_SEL;
    opamp2_config.vin_n_sel  = SL_OPAMP2_VINN_SEL;
    opamp2_config.vref_sel   = SL_OPAMP2_VREF_SEL;
    opamp2_config.enable     = ENABLE;
    opamp2_config.lp_mode    = SL_OPAMP2_LP_MODE;
    opamp2_config.out_mux_en = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp2_config, user_config);
    }
#endif
    RSI_OPAMP_UGB_V2(opamp_config_inst, &opamp2_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  } else if (opamp_config_inst == SL_OPAMP_3) {
    // Configure OPAMP3 for unity gain if macros are defined
#ifdef SL_OPAMP3_VINP_SEL
    Configure_OPAMP_t opamp3_config = { 0 };
#if (OPAMP3_UC == ENABLE)
    opamp3_config.vin_p_sel  = SL_OPAMP3_VINP_SEL;
    opamp3_config.vin_n_sel  = SL_OPAMP3_VINN_SEL;
    opamp3_config.vref_sel   = SL_OPAMP3_VREF_SEL;
    opamp3_config.enable     = ENABLE;
    opamp3_config.lp_mode    = SL_OPAMP3_LP_MODE;
    opamp3_config.out_mux_en = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp3_config, user_config);
    }
#endif
    RSI_OPAMP_UGB_V2(opamp_config_inst, &opamp3_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  }
}

/*******************************************************************************
  * @brief: Configure OPAMP for Inverting Programmable Gain Amplifier
  * @details: Configures OPAMP for inverting PGA based on the instance number
  *******************************************************************************/
static void sli_si91x_opamp_configure_inverting_pga(uint8_t opamp_config_inst, sl_opamp_config_t *user_config)
{
  (void)user_config;
  if (opamp_config_inst == SL_OPAMP_1) {
    // Configure OPAMP1 for inverting pga if macros are defined
#ifdef SL_OPAMP1_VINP_SEL
    Configure_OPAMP_t opamp1_config = { 0 };
#if (OPAMP1_UC == ENABLE)
    fill_opamp_uc_config(&opamp1_config);
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp1_config, user_config);
    }
#endif
    RSI_OPAMP_InvPGA_V2(opamp_config_inst, &opamp1_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  } else if (opamp_config_inst == SL_OPAMP_2) {
    // Configure OPAMP2 for inverting pga if macros are defined
#ifdef SL_OPAMP2_VINP_SEL
    Configure_OPAMP_t opamp2_config = { 0 };
#if (OPAMP2_UC == ENABLE)
    opamp2_config.vin_p_sel  = SL_OPAMP2_VINP_SEL;
    opamp2_config.vin_n_sel  = SL_OPAMP2_VINN_SEL;
    opamp2_config.vref_sel   = SL_OPAMP2_VREF_SEL;
    opamp2_config.enable     = ENABLE;
    opamp2_config.lp_mode    = SL_OPAMP2_LP_MODE;
    opamp2_config.r1_sel     = SL_OPAMP2_R1_SELECT;
    opamp2_config.r2_sel     = SL_OPAMP2_R2_SELECT;
    opamp2_config.out_mux_en = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp2_config, user_config);
    }
#endif
    RSI_OPAMP_InvPGA_V2(opamp_config_inst, &opamp2_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  } else if (opamp_config_inst == SL_OPAMP_3) {
    // Configure OPAMP3 for inverting pga if macros are defined
#ifdef SL_OPAMP3_VINP_SEL
    Configure_OPAMP_t opamp3_config = { 0 };
#if (OPAMP3_UC == ENABLE)
    opamp3_config.vin_p_sel  = SL_OPAMP3_VINP_SEL;
    opamp3_config.vin_n_sel  = SL_OPAMP3_VINN_SEL;
    opamp3_config.vref_sel   = SL_OPAMP3_VREF_SEL;
    opamp3_config.enable     = ENABLE;
    opamp3_config.lp_mode    = SL_OPAMP3_LP_MODE;
    opamp3_config.r1_sel     = SL_OPAMP3_R1_SELECT;
    opamp3_config.r2_sel     = SL_OPAMP3_R2_SELECT;
    opamp3_config.out_mux_en = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp3_config, user_config);
    }
#endif
    RSI_OPAMP_InvPGA_V2(opamp_config_inst, &opamp3_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  }
}

/*******************************************************************************
  * @brief: Configure OPAMP for Non-Inverting Programmable Gain Amplifier
  * @details: Configures OPAMP for non-inverting PGA based on the instance number
  *******************************************************************************/
static void sli_si91x_opamp_configure_non_inverting_pga(uint8_t opamp_config_inst, sl_opamp_config_t *user_config)
{
  (void)user_config;
  if (opamp_config_inst == SL_OPAMP_1) {
    // Configure OPAMP1 for non-inverting pga if macros are defined
#ifdef SL_OPAMP1_VINP_SEL
    Configure_OPAMP_t opamp1_config = { 0 };
#if (OPAMP1_UC == ENABLE)
    fill_opamp_uc_config(&opamp1_config);
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp1_config, user_config);
    }
#endif
    RSI_OPAMP_NonInvPGA_V2(opamp_config_inst, &opamp1_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  } else if (opamp_config_inst == SL_OPAMP_2) {
    // Configure OPAMP2 for non-inverting pga if macros are defined
#ifdef SL_OPAMP2_VINP_SEL
    Configure_OPAMP_t opamp2_config = { 0 };
#if (OPAMP2_UC == ENABLE)
    opamp2_config.vin_p_sel  = SL_OPAMP2_VINP_SEL;
    opamp2_config.vin_n_sel  = SL_OPAMP2_VINN_SEL;
    opamp2_config.vref_sel   = SL_OPAMP2_VREF_SEL;
    opamp2_config.enable     = ENABLE;
    opamp2_config.lp_mode    = SL_OPAMP2_LP_MODE;
    opamp2_config.r1_sel     = SL_OPAMP2_R1_SELECT;
    opamp2_config.r2_sel     = SL_OPAMP2_R2_SELECT;
    opamp2_config.out_mux_en = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp2_config, user_config);
    }
#endif
    RSI_OPAMP_NonInvPGA_V2(opamp_config_inst, &opamp2_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  } else if (opamp_config_inst == SL_OPAMP_3) {
    // Configure OPAMP3 for non-inverting pga if macros are defined
#ifdef SL_OPAMP3_VINP_SEL
    Configure_OPAMP_t opamp3_config = { 0 };
#if (OPAMP3_UC == ENABLE)
    opamp3_config.vin_p_sel  = SL_OPAMP3_VINP_SEL;
    opamp3_config.vin_n_sel  = SL_OPAMP3_VINN_SEL;
    opamp3_config.vref_sel   = SL_OPAMP3_VREF_SEL;
    opamp3_config.enable     = ENABLE;
    opamp3_config.lp_mode    = SL_OPAMP3_LP_MODE;
    opamp3_config.r1_sel     = SL_OPAMP3_R1_SELECT;
    opamp3_config.r2_sel     = SL_OPAMP3_R2_SELECT;
    opamp3_config.out_mux_en = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp3_config, user_config);
    }
#endif
    RSI_OPAMP_NonInvPGA_V2(opamp_config_inst, &opamp3_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  }
}

/*******************************************************************************
  * @brief: Configure OPAMP for Inverting Programmable Hysteresis Comparator
  * @details: Configures OPAMP for inverting hysteresis comparator based on the instance number
  *******************************************************************************/
static void sli_si91x_opamp_configure_inverting_hyst_comp(uint8_t opamp_config_inst, sl_opamp_config_t *user_config)
{
  (void)user_config;
  if (opamp_config_inst == SL_OPAMP_1) {
    // Configure OPAMP1 for inverting hysteresis comparator if macros are defined
#ifdef SL_OPAMP1_VINP_SEL
    Configure_OPAMP_t opamp1_config = { 0 };
#if (OPAMP1_UC == ENABLE)
    fill_opamp_uc_config(&opamp1_config);
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp1_config, user_config);
    }
#endif
    RSI_OPAMP_InvCMP_V2(opamp_config_inst, &opamp1_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  } else if (opamp_config_inst == SL_OPAMP_2) {
    // Configure OPAMP2 for inverting hysteresis comparator if macros are defined
#ifdef SL_OPAMP2_VINP_SEL
    Configure_OPAMP_t opamp2_config = { 0 };
#if (OPAMP2_UC == ENABLE)
    opamp2_config.vin_p_sel  = SL_OPAMP2_VINP_SEL;
    opamp2_config.vin_n_sel  = SL_OPAMP2_VINN_SEL;
    opamp2_config.vref_sel   = SL_OPAMP2_VREF_SEL;
    opamp2_config.enable     = ENABLE;
    opamp2_config.lp_mode    = SL_OPAMP2_LP_MODE;
    opamp2_config.r1_sel     = SL_OPAMP2_R1_SELECT;
    opamp2_config.r2_sel     = SL_OPAMP2_R2_SELECT;
    opamp2_config.out_mux_en = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp2_config, user_config);
    }
#endif
    RSI_OPAMP_InvCMP_V2(opamp_config_inst, &opamp2_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  } else if (opamp_config_inst == SL_OPAMP_3) {
    // Configure OPAMP3 for inverting hysteresis comparator if macros are defined
#ifdef SL_OPAMP3_VINP_SEL
    Configure_OPAMP_t opamp3_config = { 0 };
#if (OPAMP3_UC == ENABLE)
    opamp3_config.vin_p_sel  = SL_OPAMP3_VINP_SEL;
    opamp3_config.vin_n_sel  = SL_OPAMP3_VINN_SEL;
    opamp3_config.vref_sel   = SL_OPAMP3_VREF_SEL;
    opamp3_config.enable     = ENABLE;
    opamp3_config.lp_mode    = SL_OPAMP3_LP_MODE;
    opamp3_config.r1_sel     = SL_OPAMP3_R1_SELECT;
    opamp3_config.r2_sel     = SL_OPAMP3_R2_SELECT;
    opamp3_config.out_mux_en = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp3_config, user_config);
    }
#endif
    RSI_OPAMP_InvCMP_V2(opamp_config_inst, &opamp3_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  }
}

/*******************************************************************************
  * @brief: Configure OPAMP for Non-Inverting Programmable Hysteresis Comparator
  * @details: Configures OPAMP for non-inverting hysteresis comparator based on the instance number
  *******************************************************************************/
static void sli_si91x_opamp_configure_non_inverting_hyst_comp(uint8_t opamp_config_inst, sl_opamp_config_t *user_config)
{
  (void)user_config;
  if (opamp_config_inst == SL_OPAMP_1) {
    // Configure OPAMP1 for non-inverting hysteresis comparator if macros are defined
#ifdef SL_OPAMP1_VINP_SEL
    Configure_OPAMP_t opamp1_config = { 0 };
#if (OPAMP1_UC == ENABLE)
    fill_opamp_uc_config(&opamp1_config);
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp1_config, user_config);
    }
#endif
    RSI_OPAMP_NonInvCMP_V2(opamp_config_inst, &opamp1_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  } else if (opamp_config_inst == SL_OPAMP_2) {
    // Configure OPAMP2 for non-inverting hysteresis comparator if macros are defined
#ifdef SL_OPAMP2_VINP_SEL
    Configure_OPAMP_t opamp2_config = { 0 };
#if (OPAMP2_UC == ENABLE)
    opamp2_config.vin_p_sel  = SL_OPAMP2_VINP_SEL;
    opamp2_config.vin_n_sel  = SL_OPAMP2_VINN_SEL;
    opamp2_config.vref_sel   = SL_OPAMP2_VREF_SEL;
    opamp2_config.enable     = ENABLE;
    opamp2_config.lp_mode    = SL_OPAMP2_LP_MODE;
    opamp2_config.r1_sel     = SL_OPAMP2_R1_SELECT;
    opamp2_config.r2_sel     = SL_OPAMP2_R2_SELECT;
    opamp2_config.out_mux_en = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp2_config, user_config);
    }
#endif
    RSI_OPAMP_NonInvCMP_V2(opamp_config_inst, &opamp2_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  } else if (opamp_config_inst == SL_OPAMP_3) {
    // Configure OPAMP3 for non-inverting hysteresis comparator if macros are defined
#ifdef SL_OPAMP3_VINP_SEL
    Configure_OPAMP_t opamp3_config = { 0 };
#if (OPAMP3_UC == ENABLE)
    opamp3_config.vin_p_sel  = SL_OPAMP3_VINP_SEL;
    opamp3_config.vin_n_sel  = SL_OPAMP3_VINN_SEL;
    opamp3_config.vref_sel   = SL_OPAMP3_VREF_SEL;
    opamp3_config.enable     = ENABLE;
    opamp3_config.lp_mode    = SL_OPAMP3_LP_MODE;
    opamp3_config.r1_sel     = SL_OPAMP3_R1_SELECT;
    opamp3_config.r2_sel     = SL_OPAMP3_R2_SELECT;
    opamp3_config.out_mux_en = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp3_config, user_config);
    }
#endif
    RSI_OPAMP_NonInvCMP_V2(opamp_config_inst, &opamp3_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  }
}
/*******************************************************************************
  * @brief: Configure OPAMP for Cascaded Inverting Programmable Gain Amplifier
  * @details: Configures OPAMP for cascaded inverting pga
  *******************************************************************************/
sl_status_t sli_si91x_opamp_configure_cascaded_inverting_pga(uint8_t opamp_config_inst, sl_opamp_config_t *user_config)
{
  (void)user_config;
  (void)opamp_config_inst;
#if defined(SL_OPAMP_OPAMP1) && defined(SL_OPAMP_OPAMP2) && defined(SL_OPAMP_OPAMP3) // All OPAMPs enabled,
  return SL_STATUS_INVALID_PARAMETER;
  // Configure OPAMP1 and OPAMP2 for cascaded inverting pga if macros are defined
#elif defined(SL_OPAMP_OPAMP1) && defined(SL_OPAMP_OPAMP2)
  Configure_OPAMP1_OPAMP2_t opamp12_config_cas_invt = { 0 };
#if (OPAMP1_UC == ENABLE) && (OPAMP2_UC == ENABLE)
  opamp12_config_cas_invt.vin1_p_sel                = SL_OPAMP1_VINP_SEL;
  opamp12_config_cas_invt.vin1_n_sel                = SL_OPAMP1_VINN_SEL;
  opamp12_config_cas_invt.vref1_sel                 = SL_OPAMP1_VREF_SEL;
  opamp12_config_cas_invt.enable                    = ENABLE;
  opamp12_config_cas_invt.lp_mode                   = SL_OPAMP1_LP_MODE;
  opamp12_config_cas_invt.opamp1_r1_sel             = SL_OPAMP1_R1_SELECT;
  opamp12_config_cas_invt.opamp1_r2_sel             = SL_OPAMP1_R2_SELECT;
  opamp12_config_cas_invt.out_mux_en                = SL_OPAMP_OUTPUT_MUX_ENABLE;
  opamp12_config_cas_invt.out_mux_sel               = SL_OPAMP_OUT_MUX_SEL;

  opamp12_config_cas_invt.vin2_p_sel    = SL_OPAMP2_VINP_SEL;
  opamp12_config_cas_invt.vin2_n_sel    = SL_OPAMP2_VINN_SEL;
  opamp12_config_cas_invt.vref2_sel     = SL_OPAMP2_VREF_SEL;
  opamp12_config_cas_invt.enable        = ENABLE;
  opamp12_config_cas_invt.lp_mode       = SL_OPAMP2_LP_MODE;
  opamp12_config_cas_invt.opamp2_r1_sel = SL_OPAMP2_R1_SELECT;
  opamp12_config_cas_invt.opamp2_r2_sel = SL_OPAMP2_R2_SELECT;
  opamp12_config_cas_invt.out_mux_en    = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
  // Use user-provided configuration when UC is disabled
  if (user_config != NULL) {
    fill_opamp1_opamp2_uc_config(&opamp12_config_cas_invt, user_config);
  }
#endif
  RSI_OPAMP1_OPAMP2_CascInvtPGAmp(&opamp12_config_cas_invt, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
  return SL_STATUS_OK;
  // Configure OPAMP2 and OPAMP3 for cascaded inverting pga if macros are defined
#elif defined(SL_OPAMP_OPAMP2) && defined(SL_OPAMP_OPAMP3)
  Configure_OPAMP2_OPAMP3_t opamp23_config_cas_invt = { 0 };
#if (OPAMP2_UC == ENABLE) && (OPAMP3_UC == ENABLE)
  opamp23_config_cas_invt.vin2_p_sel                = SL_OPAMP2_VINP_SEL;
  opamp23_config_cas_invt.vin2_n_sel                = SL_OPAMP2_VINN_SEL;
  opamp23_config_cas_invt.vref2_sel                 = SL_OPAMP2_VREF_SEL;
  opamp23_config_cas_invt.enable                    = ENABLE;
  opamp23_config_cas_invt.lp_mode                   = SL_OPAMP2_LP_MODE;
  opamp23_config_cas_invt.opamp2_r1_sel             = SL_OPAMP2_R1_SELECT;
  opamp23_config_cas_invt.opamp2_r2_sel             = SL_OPAMP2_R2_SELECT;
  opamp23_config_cas_invt.out_mux_en                = SL_OPAMP_OUTPUT_MUX_ENABLE;

  opamp23_config_cas_invt.vin3_p_sel    = SL_OPAMP3_VINP_SEL;
  opamp23_config_cas_invt.vin3_n_sel    = SL_OPAMP3_VINN_SEL;
  opamp23_config_cas_invt.vref3_sel     = SL_OPAMP3_VREF_SEL;
  opamp23_config_cas_invt.enable        = ENABLE;
  opamp23_config_cas_invt.lp_mode       = SL_OPAMP3_LP_MODE;
  opamp23_config_cas_invt.opamp3_r1_sel = SL_OPAMP3_R1_SELECT;
  opamp23_config_cas_invt.opamp3_r2_sel = SL_OPAMP3_R2_SELECT;
  opamp23_config_cas_invt.out_mux_en    = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
  // Use user-provided configuration when UC is disabled
  if (user_config != NULL) {
    fill_opamp2_opamp3_uc_config(&opamp23_config_cas_invt, user_config);
  }
#endif
  RSI_OPAMP2_OPAMP3_CascInvtPGAmp(&opamp23_config_cas_invt, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
  return SL_STATUS_OK;
#else
  return SL_STATUS_INVALID_PARAMETER;
#endif
}

/*******************************************************************************
  * @brief: Configure OPAMP for Cascaded Non-Inverting Programmable Gain Amplifier
  * @details: Configures OPAMP for cascaded non-inverting pga
  *******************************************************************************/
sl_status_t sli_si91x_opamp_configure_cascaded_non_inverting_pga(uint8_t opamp_config_inst,
                                                                 sl_opamp_config_t *user_config)
{
  (void)user_config;
  (void)opamp_config_inst;
#if defined(SL_OPAMP_OPAMP1) && defined(SL_OPAMP_OPAMP2) && defined(SL_OPAMP_OPAMP3) // All OPAMPs enabled,
  return SL_STATUS_INVALID_PARAMETER;
  // Configure OPAMP1 and OPAMP2 for cascaded non-inverting pga if macros are defined
#elif defined(SL_OPAMP_OPAMP1) && defined(SL_OPAMP_OPAMP2)
  Configure_OPAMP1_OPAMP2_t opamp12_config_cas_noninvt = { 0 };
#if (OPAMP1_UC == ENABLE) && (OPAMP2_UC == ENABLE)
  opamp12_config_cas_noninvt.vin1_p_sel                = SL_OPAMP1_VINP_SEL;
  opamp12_config_cas_noninvt.vin1_n_sel                = SL_OPAMP1_VINN_SEL;
  opamp12_config_cas_noninvt.vref1_sel                 = SL_OPAMP1_VREF_SEL;
  opamp12_config_cas_noninvt.enable                    = ENABLE;
  opamp12_config_cas_noninvt.lp_mode                   = SL_OPAMP1_LP_MODE;
  opamp12_config_cas_noninvt.opamp1_r1_sel             = SL_OPAMP1_R1_SELECT;
  opamp12_config_cas_noninvt.opamp1_r2_sel             = SL_OPAMP1_R2_SELECT;
  opamp12_config_cas_noninvt.out_mux_en                = SL_OPAMP_OUTPUT_MUX_ENABLE;
  opamp12_config_cas_noninvt.out_mux_sel               = SL_OPAMP_OUT_MUX_SEL;

  opamp12_config_cas_noninvt.vin2_p_sel    = SL_OPAMP2_VINP_SEL;
  opamp12_config_cas_noninvt.vin2_n_sel    = SL_OPAMP2_VINN_SEL;
  opamp12_config_cas_noninvt.vref2_sel     = SL_OPAMP2_VREF_SEL;
  opamp12_config_cas_noninvt.enable        = ENABLE;
  opamp12_config_cas_noninvt.lp_mode       = SL_OPAMP2_LP_MODE;
  opamp12_config_cas_noninvt.opamp2_r1_sel = SL_OPAMP2_R1_SELECT;
  opamp12_config_cas_noninvt.opamp2_r2_sel = SL_OPAMP2_R2_SELECT;
  opamp12_config_cas_noninvt.out_mux_en    = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
  // Use user-provided configuration when UC is disabled
  if (user_config != NULL) {
    fill_opamp1_opamp2_uc_config(&opamp12_config_cas_noninvt, user_config);
  }
#endif
  RSI_OPAMP1_OPAMP2_CascNonInvtPGAmp(&opamp12_config_cas_noninvt, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
  return SL_STATUS_OK;
  // Configure OPAMP2 and OPAMP3 for cascaded non-inverting pga if macros are defined
#elif defined(SL_OPAMP_OPAMP2) && defined(SL_OPAMP_OPAMP3)
  Configure_OPAMP2_OPAMP3_t opamp23_config_cas_noninvt = { 0 };
#if (OPAMP2_UC == ENABLE) && (OPAMP3_UC == ENABLE)
  opamp23_config_cas_noninvt.vin2_p_sel                = SL_OPAMP2_VINP_SEL;
  opamp23_config_cas_noninvt.vin2_n_sel                = SL_OPAMP2_VINN_SEL;
  opamp23_config_cas_noninvt.vref2_sel                 = SL_OPAMP2_VREF_SEL;
  opamp23_config_cas_noninvt.enable                    = ENABLE;
  opamp23_config_cas_noninvt.lp_mode                   = SL_OPAMP2_LP_MODE;
  opamp23_config_cas_noninvt.opamp2_r1_sel             = SL_OPAMP2_R1_SELECT;
  opamp23_config_cas_noninvt.opamp2_r2_sel             = SL_OPAMP2_R2_SELECT;
  opamp23_config_cas_noninvt.out_mux_en                = SL_OPAMP_OUTPUT_MUX_ENABLE;

  opamp23_config_cas_noninvt.vin3_p_sel    = SL_OPAMP3_VINP_SEL;
  opamp23_config_cas_noninvt.vin3_n_sel    = SL_OPAMP3_VINN_SEL;
  opamp23_config_cas_noninvt.vref3_sel     = SL_OPAMP3_VREF_SEL;
  opamp23_config_cas_noninvt.enable        = ENABLE;
  opamp23_config_cas_noninvt.lp_mode       = SL_OPAMP3_LP_MODE;
  opamp23_config_cas_noninvt.opamp3_r1_sel = SL_OPAMP3_R1_SELECT;
  opamp23_config_cas_noninvt.opamp3_r2_sel = SL_OPAMP3_R2_SELECT;
  opamp23_config_cas_noninvt.out_mux_en    = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
  // Use user-provided configuration when UC is disabled
  if (user_config != NULL) {
    fill_opamp2_opamp3_uc_config(&opamp23_config_cas_noninvt, user_config);
  }
#endif
  RSI_OPAMP2_OPAMP3_CascNonInvtPGAmp(&opamp23_config_cas_noninvt, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
  return SL_STATUS_OK;
#else
  return SL_STATUS_INVALID_PARAMETER;
#endif
}

/*******************************************************************************
  * @brief: Configure OPAMP for Two Opamps Differential Amplifier
  * @details: Configures OPAMP for cascaded non-inverting pga
  *******************************************************************************/
sl_status_t sli_si91x_opamp_configure_two_opamps_differential_amp(uint8_t opamp_config_inst,
                                                                  sl_opamp_config_t *user_config)
{
  (void)user_config;
  (void)opamp_config_inst;
#if defined(SL_OPAMP_OPAMP1) && defined(SL_OPAMP_OPAMP2) && defined(SL_OPAMP_OPAMP3) // All OPAMPs enabled,
  return SL_STATUS_INVALID_PARAMETER;
  // Configure OPAMP1 and OPAMP2 for two opamps differential amplifier if macros are defined
#elif defined(SL_OPAMP_OPAMP1) && defined(SL_OPAMP_OPAMP2)
  Configure_OPAMP1_OPAMP2_t opamp12_config_two_opamps = { 0 };
#if (OPAMP1_UC == ENABLE) && (OPAMP2_UC == ENABLE)
  opamp12_config_two_opamps.vin1_p_sel                = SL_OPAMP1_VINP_SEL;
  opamp12_config_two_opamps.vin1_n_sel                = SL_OPAMP1_VINN_SEL;
  opamp12_config_two_opamps.vref1_sel                 = SL_OPAMP1_VREF_SEL;
  opamp12_config_two_opamps.enable                    = ENABLE;
  opamp12_config_two_opamps.lp_mode                   = SL_OPAMP1_LP_MODE;
  opamp12_config_two_opamps.out_mux_en                = SL_OPAMP_OUTPUT_MUX_ENABLE;
  opamp12_config_two_opamps.out_mux_sel               = SL_OPAMP_OUT_MUX_SEL;

  opamp12_config_two_opamps.vin2_p_sel    = SL_OPAMP2_VINP_SEL;
  opamp12_config_two_opamps.vin2_n_sel    = SL_OPAMP2_VINN_SEL;
  opamp12_config_two_opamps.vref2_sel     = SL_OPAMP2_VREF_SEL;
  opamp12_config_two_opamps.enable        = ENABLE;
  opamp12_config_two_opamps.lp_mode       = SL_OPAMP2_LP_MODE;
  opamp12_config_two_opamps.opamp2_r1_sel = SL_OPAMP2_R1_SELECT;
  opamp12_config_two_opamps.opamp2_r2_sel = SL_OPAMP2_R2_SELECT;
  opamp12_config_two_opamps.out_mux_en    = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
  // Use user-provided configuration when UC is disabled
  if (user_config != NULL) {
    fill_opamp1_opamp2_uc_config(&opamp12_config_two_opamps, user_config);
  }
#endif
  RSI_OPAMP1_OPAMP2_TwoOpampsDiffAmp(&opamp12_config_two_opamps, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
  return SL_STATUS_OK;
  // Configure OPAMP2 and OPAMP3 for two opamps differential amplifier if macros are defined
#elif defined(SL_OPAMP_OPAMP2) && defined(SL_OPAMP_OPAMP3)
  Configure_OPAMP2_OPAMP3_t opamp23_config_two_opamps = { 0 };
#if (OPAMP2_UC == ENABLE) && (OPAMP3_UC == ENABLE)
  opamp23_config_two_opamps.vin2_p_sel                = SL_OPAMP2_VINP_SEL;
  opamp23_config_two_opamps.vin2_n_sel                = SL_OPAMP2_VINN_SEL;
  opamp23_config_two_opamps.vref2_sel                 = SL_OPAMP2_VREF_SEL;
  opamp23_config_two_opamps.enable                    = ENABLE;
  opamp23_config_two_opamps.lp_mode                   = SL_OPAMP2_LP_MODE;
  opamp23_config_two_opamps.out_mux_en                = SL_OPAMP_OUTPUT_MUX_ENABLE;

  opamp23_config_two_opamps.vin3_p_sel    = SL_OPAMP3_VINP_SEL;
  opamp23_config_two_opamps.vin3_n_sel    = SL_OPAMP3_VINN_SEL;
  opamp23_config_two_opamps.vref3_sel     = SL_OPAMP3_VREF_SEL;
  opamp23_config_two_opamps.enable        = ENABLE;
  opamp23_config_two_opamps.lp_mode       = SL_OPAMP3_LP_MODE;
  opamp23_config_two_opamps.opamp3_r1_sel = SL_OPAMP3_R1_SELECT;
  opamp23_config_two_opamps.opamp3_r2_sel = SL_OPAMP3_R2_SELECT;
  opamp23_config_two_opamps.out_mux_en    = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
  // Use user-provided configuration when UC is disabled
  if (user_config != NULL) {
    fill_opamp2_opamp3_uc_config(&opamp23_config_two_opamps, user_config);
  }
#endif
  RSI_OPAMP2_OPAMP3_TwoOpampsDiffAmp(&opamp23_config_two_opamps, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
  return SL_STATUS_OK;
#else
  return SL_STATUS_INVALID_PARAMETER;
#endif
}

/*******************************************************************************
  * @brief: Configure OPAMP for Trans Impedance Amplifier
  * @details: Configures OPAMP for trans-impedance amplifier
  *******************************************************************************/
void sli_si91x_opamp_configure_trans_impedance_amp(uint8_t opamp_config_inst, sl_opamp_config_t *user_config)
{
  (void)user_config;
  if (opamp_config_inst == SL_OPAMP_1) {
    // Configure OPAMP1 for trans-impedance amplifier if macros are defined
#ifdef SL_OPAMP1_VINP_SEL
    Configure_OPAMP_t opamp1_config = { 0 };
#if (OPAMP1_UC == ENABLE)
    fill_opamp_uc_config(&opamp1_config);
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp1_config, user_config);
    }
#endif
    RSI_OPAMP_TIA_V2(opamp_config_inst, &opamp1_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  } else if (opamp_config_inst == SL_OPAMP_2) {
    // Configure OPAMP2 for trans-impedance amplifier if macros are defined
#ifdef SL_OPAMP2_VINP_SEL
    Configure_OPAMP_t opamp2_config = { 0 };
#if (OPAMP2_UC == ENABLE)
    opamp2_config.vin_p_sel  = SL_OPAMP2_VINP_SEL;
    opamp2_config.vin_n_sel  = SL_OPAMP2_VINN_SEL;
    opamp2_config.vref_sel   = SL_OPAMP2_VREF_SEL;
    opamp2_config.enable     = ENABLE;
    opamp2_config.lp_mode    = SL_OPAMP2_LP_MODE;
    opamp2_config.r1_sel     = SL_OPAMP2_R1_SELECT;
    opamp2_config.r2_sel     = SL_OPAMP2_R2_SELECT;
    opamp2_config.out_mux_en = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp2_config, user_config);
    }
#endif
    RSI_OPAMP_TIA_V2(opamp_config_inst, &opamp2_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  } else if (opamp_config_inst == SL_OPAMP_3) {
    // Configure OPAMP3 for trans-impedance amplifier if macros are defined
#ifdef SL_OPAMP3_VINP_SEL
    Configure_OPAMP_t opamp3_config = { 0 };
#if (OPAMP3_UC == ENABLE)
    opamp3_config.vin_p_sel  = SL_OPAMP3_VINP_SEL;
    opamp3_config.vin_n_sel  = SL_OPAMP3_VINN_SEL;
    opamp3_config.vref_sel   = SL_OPAMP3_VREF_SEL;
    opamp3_config.enable     = ENABLE;
    opamp3_config.lp_mode    = SL_OPAMP3_LP_MODE;
    opamp3_config.r1_sel     = SL_OPAMP3_R1_SELECT;
    opamp3_config.r2_sel     = SL_OPAMP3_R2_SELECT;
    opamp3_config.out_mux_en = SL_OPAMP_OUTPUT_MUX_ENABLE;
#else
    // Use user-provided configuration when UC is disabled
    if (user_config != NULL) {
      sli_si91x_opamp_non_uc_config(&opamp3_config, user_config);
    }
#endif
    RSI_OPAMP_TIA_V2(opamp_config_inst, &opamp3_config, SL_OPAMP_DYN_MODE_EN, SL_OPAMP_CHANNEL_NO);
#endif
  }
}

/*******************************************************************************
  * @brief: Configure OPAMP for Instrumentation Amplifier
  * @details: Configures OPAMP for instrumentation amplifier
  *******************************************************************************/
sl_status_t sli_si91x_opamp_configure_instrumentation_amp(uint8_t opamp_config_inst, sl_opamp_config_t *user_config)
{
  (void)opamp_config_inst;
  (void)user_config;
#if (defined(SL_OPAMP_OPAMP1) && defined(SL_OPAMP_OPAMP2)) && defined(SL_OPAMP_OPAMP3)
  sl_status_t status;
  sl_dac_clock_config_t dac_clock_config;
  static boolean_t dac_initialized = false;          // Static flag to track DAC initialization
  float vref_value = ((float)OPAMP_REF_VOLT / 1000); // OPAMP Reference voltage from UC change into float

  // Initialize DAC only once for instrumentation amplifier configuration
  if (!dac_initialized) {
    //Initializing DAC peripheral
    status = sl_si91x_dac_init(&dac_clock_config);
    if (status != SL_STATUS_OK) {
      return status;
    }
    // DAC configuration
    status = sl_si91x_dac_set_configuration(sl_dac_config, vref_value);
    if (status != SL_STATUS_OK) {
      return status;
    }
    // Register user callback function
    status = sl_si91x_dac_register_event_callback(dac_instrument_callback_event);
    if (status != SL_STATUS_OK) {
      return status;
    }
    // DAC input sample data writing
    status = sl_si91x_dac_write_data((int16_t *)dac_instrument_input_sample_data, NUMBER_OF_INPUT_SAMPLE);
    if (status != SL_STATUS_OK) {
      return status;
    }
    // Start DAC peripheral
    status = sl_si91x_dac_start();
    if (status != SL_STATUS_OK) {
      return status;
    }
    dac_initialized = true; // DAC is initialized
  }
  // Configure OPAMP1, OPAMP2 and OPAMP3 for instrumentation amplifier if macros are defined
  OPAMP_CONFIG_T Config;
  // Use UC configuration if all OPAMPs have UC enabled
#if (OPAMP1_UC == ENABLE) && (OPAMP2_UC == ENABLE) && (OPAMP3_UC == ENABLE)
  (void)user_config;
  Config.opamp1.opamp1_dyn_en         = SL_OPAMP_DYN_MODE_EN;
  Config.opamp1.opamp1_sel_p_mux      = SL_OPAMP1_VINP_SEL;
  Config.opamp1.opamp1_sel_n_mux      = SL_OPAMP1_VINN_SEL;
  Config.opamp1.opamp1_out_mux_en     = SL_OPAMP1_INST_AMP_OUT_MUX_EN;
  Config.opamp1.opamp1_out_mux_sel    = SL_OPAMP_OUT_MUX_SEL;
  Config.opamp1.opamp1_res_to_out_vdd = SL_OPAMP1_INST_AMP_RES_TO_OUT_VDD;
  Config.opamp1.opamp1_res_mux_sel    = SL_OPAMP1_VREF_SEL;
  Config.opamp1.opamp1_en_res_bank    = SL_OPAMP1_INST_AMP_EN_RES_BANK;
  Config.opamp1.opamp1_r2_sel         = SL_OPAMP1_R2_SELECT;
  Config.opamp1.opamp1_r1_sel         = SL_OPAMP1_R1_SELECT;
  Config.opamp1.opamp1_lp_mode        = SL_OPAMP1_LP_MODE;
  Config.opamp1.opamp1_enable         = ENABLE;

  Config.opamp2.opamp2_dyn_en         = SL_OPAMP_DYN_MODE_EN;
  Config.opamp2.opamp2_sel_p_mux      = SL_OPAMP2_VINP_SEL;
  Config.opamp2.opamp2_sel_n_mux      = SL_OPAMP2_VINN_SEL;
  Config.opamp2.opamp2_out_mux_en     = SL_OPAMP2_INST_AMP_OUT_MUX_EN;
  Config.opamp2.opamp2_res_to_out_vdd = SL_OPAMP2_INST_AMP_RES_TO_OUT_VDD;
  Config.opamp2.opamp2_res_mux_sel    = SL_OPAMP2_VREF_SEL;
  Config.opamp2.opamp2_en_res_bank    = SL_OPAMP2_INST_AMP_EN_RES_BANK;
  Config.opamp2.opamp2_r2_sel         = SL_OPAMP2_R2_SELECT;
  Config.opamp2.opamp2_r1_sel         = SL_OPAMP2_R1_SELECT;
  Config.opamp2.opamp2_lp_mode        = SL_OPAMP2_LP_MODE;
  Config.opamp2.opamp2_enable         = ENABLE;

  Config.opamp3.opamp3_dyn_en         = SL_OPAMP_DYN_MODE_EN;
  Config.opamp3.opamp3_sel_p_mux      = SL_OPAMP3_VINP_SEL;
  Config.opamp3.opamp3_sel_n_mux      = SL_OPAMP3_VINN_SEL;
  Config.opamp3.opamp3_out_mux_en     = SL_OPAMP3_INST_AMP_OUT_MUX_EN;
  Config.opamp3.opamp3_res_to_out_vdd = SL_OPAMP3_INST_AMP_RES_TO_OUT_VDD;
  Config.opamp3.opamp3_res_mux_sel    = SL_OPAMP3_VREF_SEL;
  Config.opamp3.opamp3_en_res_bank    = SL_OPAMP3_INST_AMP_EN_RES_BANK;
  Config.opamp3.opamp3_r2_sel         = SL_OPAMP3_R2_SELECT;
  Config.opamp3.opamp3_r1_sel         = SL_OPAMP3_R1_SELECT;
  Config.opamp3.opamp3_lp_mode        = SL_OPAMP3_LP_MODE;
  Config.opamp3.opamp3_enable         = ENABLE;
#else
  // Use user-provided configuration when UC is disabled
  Config.opamp1.opamp1_dyn_en = SL_OPAMP_DYN_MODE_EN;
  Config.opamp1.opamp1_sel_p_mux = user_config->opamp123_config.opamp1.opamp1_sel_p_mux;
  Config.opamp1.opamp1_sel_n_mux = user_config->opamp123_config.opamp1.opamp1_sel_n_mux;
  Config.opamp1.opamp1_out_mux_en = SL_OPAMP1_INST_AMP_OUT_MUX_EN;
  Config.opamp1.opamp1_out_mux_sel = user_config->opamp123_config.opamp1.opamp1_out_mux_sel;
  Config.opamp1.opamp1_res_to_out_vdd = SL_OPAMP1_INST_AMP_RES_TO_OUT_VDD;
  Config.opamp1.opamp1_res_mux_sel = user_config->opamp123_config.opamp1.opamp1_res_mux_sel;
  Config.opamp1.opamp1_en_res_bank = SL_OPAMP1_INST_AMP_EN_RES_BANK;
  Config.opamp1.opamp1_r2_sel = user_config->opamp123_config.opamp1.opamp1_r2_sel;
  Config.opamp1.opamp1_r1_sel = user_config->opamp123_config.opamp1.opamp1_r1_sel;
  Config.opamp1.opamp1_lp_mode = user_config->opamp123_config.opamp1.opamp1_lp_mode;
  Config.opamp1.opamp1_enable = ENABLE;

  Config.opamp2.opamp2_dyn_en = SL_OPAMP_DYN_MODE_EN;
  Config.opamp2.opamp2_sel_p_mux = user_config->opamp123_config.opamp2.opamp2_sel_p_mux;
  Config.opamp2.opamp2_sel_n_mux = user_config->opamp123_config.opamp2.opamp2_sel_n_mux;
  Config.opamp2.opamp2_out_mux_en = SL_OPAMP2_INST_AMP_OUT_MUX_EN;
  Config.opamp2.opamp2_res_to_out_vdd = SL_OPAMP2_INST_AMP_RES_TO_OUT_VDD;
  Config.opamp2.opamp2_res_mux_sel = user_config->opamp123_config.opamp2.opamp2_res_mux_sel;
  Config.opamp2.opamp2_en_res_bank = SL_OPAMP2_INST_AMP_EN_RES_BANK;
  Config.opamp2.opamp2_r2_sel = user_config->opamp123_config.opamp2.opamp2_r2_sel;
  Config.opamp2.opamp2_r1_sel = user_config->opamp123_config.opamp2.opamp2_r1_sel;
  Config.opamp2.opamp2_lp_mode = user_config->opamp123_config.opamp2.opamp2_lp_mode;
  Config.opamp2.opamp2_enable = ENABLE;

  Config.opamp3.opamp3_dyn_en = SL_OPAMP_DYN_MODE_EN;
  Config.opamp3.opamp3_sel_p_mux = user_config->opamp123_config.opamp3.opamp3_sel_p_mux;
  Config.opamp3.opamp3_sel_n_mux = user_config->opamp123_config.opamp3.opamp3_sel_n_mux;
  Config.opamp3.opamp3_out_mux_en = SL_OPAMP3_INST_AMP_OUT_MUX_EN;
  Config.opamp3.opamp3_res_to_out_vdd = SL_OPAMP3_INST_AMP_RES_TO_OUT_VDD;
  Config.opamp3.opamp3_res_mux_sel = user_config->opamp123_config.opamp3.opamp3_res_mux_sel;
  Config.opamp3.opamp3_en_res_bank = SL_OPAMP3_INST_AMP_EN_RES_BANK;
  Config.opamp3.opamp3_r2_sel = user_config->opamp123_config.opamp3.opamp3_r2_sel;
  Config.opamp3.opamp3_r1_sel = user_config->opamp123_config.opamp3.opamp3_r1_sel;
  Config.opamp3.opamp3_lp_mode = user_config->opamp123_config.opamp3.opamp3_lp_mode;
  Config.opamp3.opamp3_enable = ENABLE;
#endif
  RSI_OPAMP_Instrumentation_Amplifier(SL_OPAMP_CHANNEL_NO, &Config);
  return SL_STATUS_OK;
#else
  return SL_STATUS_INVALID_PARAMETER;
#endif
}

/*******************************************************************************
  * @brief: De-Initializes OPAMP peripheral
  * @details: Disables Peripheral clock and powers down AUX
  *******************************************************************************/
void sl_si91x_opamp_deinit(void)
{
  analog_set_power_state(OPAMP_BIT_POS, ANALOG_POWERED_OFF);

  // Disabling ULPSS AUX clock
  if (!analog_get_power_state()) {
    RSI_PS_UlpssPeriPowerDown(ULPSS_PWRGATE_ULP_AUX);
    RSI_ULPSS_PeripheralDisable(ULPCLK, ULP_AUX_CLK);
  }
}

/*******************************************************************************
  * DAC instrumentation callback function
  *
  * This function handling DAC events. It is invoked when
  * a DAC event occurs and can be customized for specific event handling needs.
  ******************************************************************************/
#if (defined(SL_OPAMP_OPAMP1) && defined(SL_OPAMP_OPAMP2)) && defined(SL_OPAMP_OPAMP3)
static void dac_instrument_callback_event(uint8_t event)
{
  if (event == SL_DAC_STATIC_MODE_EVENT) {
    dac_instrument_static_intr_flag = true;
  }
  if (event == SL_DAC_FIFO_MODE_EVENT) {
    dac_instrument_fifo_intr_flag = true;
  }
}
#endif
