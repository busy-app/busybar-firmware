/***************************************************************************/ /**
* @file analog_comparator_example.c
* @brief Analog Comparator examples functions
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
#include "analog_comparator_example.h"
#include "rsi_debug.h"
#include "rsi_rom_egpio.h"
#include "sl_si91x_driver_gpio.h"
#include "rsi_opamp.h"
#include "rsi_dac.h"
#include "sl_si91x_analog_comparator.h"
#include "sl_si91x_analog_comparator_init.h"
#include <stdint.h>

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define INSTANCE_ONE 1 // For comparator1
#define INSTANCE_TWO 2 // For comparator2
// Update below macro with comparator instance used for application
#define ANALOG_COMPARATOR_USED INSTANCE_TWO

// Enable below macro to compare external pin inputs
#define COMPARE_POS_INPUT_EXTERNAL_NEG_INPUT_EXTERNAL ENABLE

// Enable below macro to compare external pin input with internal reference scaler voltage
#define COMPARE_POS_INPUT_EXTERNAL_NEG_INPUT_REF_SCALER DISABLE

// Enable below macro to compare external pin input with internal resistor bank voltage
#define COMPARE_POS_INPUT_EXTERNAL_NEG_INPUT_RESISTOR_BANK DISABLE

// Enable to compare internal voltages
#define COMPARE_POS_INPUT_RESISTOR_BANK_NEG_INPUT_REF_SCALER DISABLE

// Enable to compare OPAMP output with reference scaler voltage
#define COMPARE_POS_INPUT_OPAMP_NEG_INPUT_REF_SCALER DISABLE

// Enable to compare OPAMP output internal resistor bank voltage
#define COMPARE_POS_INPUT_OPAMP_NEG_INPUT_RESISTOR_BANK DISABLE

// Enable to compare OPAMP output with external voltage
#define COMPARE_POS_INPUT_OPAMP_NEG_INPUT_EXTERNAL DISABLE

// Enable to compare DAC output with external voltage
#define COMPARE_POS_INPUT_DAC_NEG_INPUT_EXTERNAL DISABLE

// Enable to compare DAC output with external voltage
#define COMPARE_POS_INPUT_DAC_NEG_INPUT_REF_SCALER DISABLE

#define THRESHOLD_VALUE SL_COMPARATOR_THRESHOLD_VALUE_FOR_2_15_VOLT // To change resistor bank output to 2.15 volts
#define SCALE_FACTOR_VALUE \
  SL_COMPARATOR_SCALE_FACTOR_VALUE_FOR_0_7_VOLT // To change Reference scaler output to 0.7 volts

#if (COMPARE_POS_INPUT_OPAMP_NEG_INPUT_REF_SCALER || COMPARE_POS_INPUT_OPAMP_NEG_INPUT_RESISTOR_BANK \
     || COMPARE_POS_INPUT_OPAMP_NEG_INPUT_EXTERNAL)
// macros for OPAMP
#define OPAMP_INSTANCE_USED 1 // opamp instance used
#define OPAMP_OUT_MUX_SEL   0
#define OPAMP_DYN_MODE_EN   0 /*OPAMP Dynamic mode*/
#define CHANNEL_NO          0 /*adc channel no in dynamic mode*/
#define OPAMP_LP_MODE       0 /*OPAMP low power enable*/
#define OUTPUT_MUX_ENABLE   1
#define OPAMP1_GPIO         1
#define OPAMP2_GPIO         0
#define OPAMP3_GPIO         0
#endif
// macros for gpio pins
#define PORT_4           4 // For GPIO port 4
#define PIN_1            1 // For ULP GPIO 1
#define PIN_4            4 // For ULP GPIO 4
#define PIN_5            5 // For ULP GPIO 5
#define PIN_MODE         0 // Pin mode
#define PIN_OUTPUT_VALUE 1 // Pin output value
#define PIN_DIRECTION    0 // Pin direction
#if (COMPARE_POS_INPUT_DAC_NEG_INPUT_REF_SCALER || COMPARE_POS_INPUT_DAC_NEG_INPUT_EXTERNAL)
// Macros for DAC
#define DAC_SAMPLING_RATE          5000000
#define NUMBER_OF_INPUT_SAMPLE     1
#define MAX_DAC_INPUT_SAMPLE_VALUE 1023
#define DAC_INPUT_SAMPLE_VALUE     1023
#endif
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
#if (ANALOG_COMPARATOR_USED == INSTANCE_ONE)
static void on_comparator1_callback(void);
#endif
#if (ANALOG_COMPARATOR_USED == INSTANCE_TWO)
static void on_comparator2_callback(void);
#endif
#if (COMPARE_POS_INPUT_OPAMP_NEG_INPUT_REF_SCALER || COMPARE_POS_INPUT_OPAMP_NEG_INPUT_RESISTOR_BANK \
     || COMPARE_POS_INPUT_OPAMP_NEG_INPUT_EXTERNAL)
static void opamp_init(uint8_t opamp_instance);
#endif
#if (COMPARE_POS_INPUT_DAC_NEG_INPUT_REF_SCALER || COMPARE_POS_INPUT_DAC_NEG_INPUT_EXTERNAL)
static void dac_callback(uint8_t event);
static void dac_init();
#endif
/*******************************************************************************
 **********************  Local variables   *************************************
 ******************************************************************************/
sl_status_t status;
sl_analog_comparator_scale_factor_values_t scale_factor_value;
sl_analog_comparator_threshold_values_t threshold_value;

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
void analog_comparator_example_init(void)
{
  // Updating scale factor variable as per SCALE_FACTOR_VALUE macro value
  scale_factor_value = SCALE_FACTOR_VALUE;
  // Updating threshold value variable as per THRESHOLD_VALUE macro value
  threshold_value = THRESHOLD_VALUE;

  do {
    // Initializing analog comparator module
    sl_si91x_analog_comparator_init();
    DEBUGOUT("Analog Comparator is initialized successfully \n");
    // Configuring comparator parameters as per selected usecase
#if (ANALOG_COMPARATOR_USED == INSTANCE_ONE)
    status = sl_si91x_analog_comparator_set_configurations(&sl_analog_comparator1_config_handle);
#endif
#if (ANALOG_COMPARATOR_USED == INSTANCE_TWO)
    status = sl_si91x_analog_comparator_set_configurations(&sl_analog_comparator2_config_handle);
#endif
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_analog_comparator_set_configurations, Error code: %lu", status);
      break;
    }
    DEBUGOUT("Analog Comparator configurations are set successfully \n");
#if (COMPARE_POS_INPUT_EXTERNAL_NEG_INPUT_REF_SCALER)
    status = sl_si91x_analog_comparator_set_reference_scaler_output(scale_factor_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_analog_comparator_set_reference_scaler_output, Error "
               "code: %lu",
               status);
      break;
    }
    DEBUGOUT("Analog Comparator scale factor is set successfully \n");
#endif
#if (COMPARE_POS_INPUT_EXTERNAL_NEG_INPUT_RESISTOR_BANK)
    // Setting threshold value for resistor bank output voltage
    status = sl_si91x_analog_comparator_set_resistor_bank_threshold(threshold_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_analog_comparator_set_resistor_bank_threshold, Error "
               "code: %lu",
               status);
      break;
    }
    DEBUGOUT("Analog Comparator resistor bank threshold is set successfully \n");
#endif
#if (COMPARE_POS_INPUT_RESISTOR_BANK_NEG_INPUT_REF_SCALER)
    // Setting scale factor for reference scaler output voltage
    status = sl_si91x_analog_comparator_set_reference_scaler_output(scale_factor_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_analog_comparator_set_reference_scaler_output, Error "
               "code: %lu",
               status);
      break;
    }
    DEBUGOUT("Analog Comparator scale factor is set successfully \n");
    // Setting threshold value for resistor bank output voltage
    status = sl_si91x_analog_comparator_set_resistor_bank_threshold(threshold_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_analog_comparator_set_resistor_bank_threshold, Error "
               "code: %lu",
               status);
      break;
    }
    DEBUGOUT("Analog Comparator resistor bank threshold is set successfully \n");
#endif
#if COMPARE_POS_INPUT_OPAMP_NEG_INPUT_EXTERNAL
    uint8_t opamp_instance = OPAMP_INSTANCE_USED;
    opamp_init(opamp_instance);
#endif
#if COMPARE_POS_INPUT_OPAMP_NEG_INPUT_REF_SCALER
    uint8_t opamp_instance = OPAMP_INSTANCE_USED;
    opamp_init(opamp_instance);
    // Setting scale factor for reference scaler output voltage
    status = sl_si91x_analog_comparator_set_reference_scaler_output(scale_factor_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_analog_comparator_set_reference_scaler_output, Error "
               "code: %lu",
               status);
      break;
    }
    DEBUGOUT("Analog Comparator scale factor is set successfully \n");
#endif
#if COMPARE_POS_INPUT_OPAMP_NEG_INPUT_RESISTOR_BANK
    uint8_t opamp_instance = OPAMP_INSTANCE_USED;
    opamp_init(opamp_instance);
    status = sl_si91x_analog_comparator_set_resistor_bank_threshold(threshold_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_analog_comparator_set_resistor_bank_threshold, Error "
               "code: %lu",
               status);
      break;
    }
    DEBUGOUT("Analog Comparator resistor bank threshold is set successfully \n");
#endif
#if COMPARE_POS_INPUT_DAC_NEG_INPUT_EXTERNAL
    dac_init();
#endif
#if COMPARE_POS_INPUT_DAC_NEG_INPUT_REF_SCALER
    dac_init();
    // Setting scale factor for reference scaler output voltage
    status = sl_si91x_analog_comparator_set_reference_scaler_output(scale_factor_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_analog_comparator_set_reference_scaler_output, Error "
               "code: %lu",
               status);
      break;
    }
    DEBUGOUT("Analog Comparator scale factor is set successfully \n");
#endif
#if (ANALOG_COMPARATOR_USED == INSTANCE_ONE)
    // Registering comparator callback and enabling interrupts
    status = sl_si91x_analog_comparator_register_callback(ANALOG_COMPARATOR_USED, on_comparator1_callback);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_analog_comparator_register_callback, Error code: %lu", status);
      break;
    }
    DEBUGOUT("Analog Comparator callback is registered successfully \n");
#endif
#if (ANALOG_COMPARATOR_USED == INSTANCE_TWO)
    // Registering comparator callback and enabling interrupts
    status = sl_si91x_analog_comparator_register_callback(ANALOG_COMPARATOR_USED, on_comparator2_callback);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_analog_comparator_register_callback, Error code: %lu", status);
      break;
    }
    DEBUGOUT("Analog Comparator callback is registered successfully \n");
#endif
  } while (false);
}

/***************************************************************************/ /**
* Function will run continuously and will wait for trigger
******************************************************************************/
void analog_comparator_example_process_action(void)
{
}
#if (COMPARE_POS_INPUT_DAC_NEG_INPUT_REF_SCALER || COMPARE_POS_INPUT_DAC_NEG_INPUT_EXTERNAL)
/***************************************************************************/ /**
* Function for DAC initialization
******************************************************************************/
static void dac_init()
{
  int16_t dac_input_sample_data[1] = { DAC_INPUT_SAMPLE_VALUE };
  // Initialize DAC
  DAC_Init(STATIC_MODE_EN, DAC_SAMPLING_RATE, dac_callback);
  // Starting DAC
  DAC_Start(STATIC_MODE_EN);
  // writing input to dac
  DAC_WriteData(STATIC_MODE_EN, dac_input_sample_data, NUMBER_OF_INPUT_SAMPLE);
}
/*******************************************************************************
 Comparator-2 callback function
 ******************************************************************************/
static void dac_callback(uint8_t event)
{
  (void)event;
}
#endif
#if (COMPARE_POS_INPUT_OPAMP_NEG_INPUT_REF_SCALER || COMPARE_POS_INPUT_OPAMP_NEG_INPUT_RESISTOR_BANK \
     || COMPARE_POS_INPUT_OPAMP_NEG_INPUT_EXTERNAL)
/***************************************************************************/ /**
* Function for opamp initialization
******************************************************************************/
static void opamp_init(uint8_t opamp_instance)
{
  ULP_SPI_MEM_MAP(BG_SCDC_PROG_REG_1) |= BIT(3);
  if (opamp_instance == 1) {
    // Running OPAMP1 with unity gain, connect input supply to opamp input (exp 15)
    RSI_OPAMP1_UGB(OPAMP1_GPIO,
                   ENABLE,
                   OPAMP_LP_MODE,
                   OUTPUT_MUX_ENABLE,
                   OPAMP_OUT_MUX_SEL,
                   OPAMP_DYN_MODE_EN,
                   CHANNEL_NO);
  }
  if (opamp_instance == 2) {
    // Running OPAMP3 with unity gain, connect input supply to opamp input (F6)
    RSI_OPAMP2_UGB(OPAMP2_GPIO, ENABLE, OPAMP_LP_MODE, OUTPUT_MUX_ENABLE, OPAMP_DYN_MODE_EN, CHANNEL_NO);
  }
  if (opamp_instance == 3) {
    // Running OPAMP3 with unity gain, connect input supply to opamp input (p17)
    RSI_OPAMP3_UGB(OPAMP3_GPIO, ENABLE, OPAMP_LP_MODE, OUTPUT_MUX_ENABLE, OPAMP_DYN_MODE_EN, CHANNEL_NO);
  }
}
#endif
#if (ANALOG_COMPARATOR_USED == INSTANCE_ONE)
/*******************************************************************************
 Comparator-1 callback function
 ******************************************************************************/
static void on_comparator1_callback(void)
{
  // Toggling ULP_GPIO_5 on interrupt
  sl_gpio_t port_pin = { PORT_4, PIN_5 };
  sl_gpio_driver_set_pin_mode(&port_pin, PIN_MODE, PIN_OUTPUT_VALUE);
  sl_si91x_gpio_driver_set_pin_direction(PORT_4, PIN_5, PIN_DIRECTION);
  sl_gpio_driver_set_pin(&port_pin);
  sl_gpio_driver_clear_pin(&port_pin);
  DEBUGOUT("Comparator-1 non-inverting input voltage is greater\n");
}
#endif
#if (ANALOG_COMPARATOR_USED == INSTANCE_TWO)
/*******************************************************************************
 Comparator-2 callback function
 ******************************************************************************/
static void on_comparator2_callback(void)
{
  // Toggling ULP_GPIO_1 on interrupt
  sl_gpio_t port_pin = { PORT_4, PIN_1 };
  sl_gpio_driver_set_pin_mode(&port_pin, PIN_MODE, PIN_OUTPUT_VALUE);
  sl_si91x_gpio_driver_set_pin_direction(PORT_4, PIN_1, PIN_DIRECTION);
  sl_gpio_driver_set_pin(&port_pin);
  sl_gpio_driver_clear_pin(&port_pin);
  DEBUGOUT("Comparator-2 non-inverting input voltage is greater\n");
}
#endif
