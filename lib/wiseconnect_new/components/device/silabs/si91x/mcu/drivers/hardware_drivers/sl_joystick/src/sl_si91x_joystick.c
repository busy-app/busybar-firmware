/***************************************************************************/ /**
 * @file sl_si91x_joystick.c
 * @brief Joystick API implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_si91x_joystick.h"
#include "clock_update.h"

#include "sl_adc_instances.h"

/*******************************************************************************
 ***************************  DEFINES / MACROS   *******************************
 ******************************************************************************/
#define PS4_SOC_FREQ          180000000 // PLL out clock 180MHz
#define SOC_PLL_REF_FREQUENCY 40000000  // PLL input REFERENCE clock
#define DVISION_FACTOR        0         // Division factor
#define ADC_MAX_OP_VALUE      4096      // ADC maximum output value
#define DIVISION_MULTIPLIER \
  1000 // For changing reference voltage as float value and again change as whole value to compare the int value.
#define AUXADC_DATA_TWELFTH BIT(11) // Reading twelfth bit of AUXADC_DATA register.
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static float vref_value = REFERENCE_VOLTAGE;
static sl_adc_config_t sl_adc_config;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static void callback_event(uint8_t channel_no, uint8_t event);

/*******************************************************************************
 * Initialization of Joystick.
 * This API initialize ADC and configure ADC sampling rate, set the
 * ADC peripheral clock. And registering callback function for ADC event.
 * Returning status of sl_status_t type.
 ******************************************************************************/
sl_status_t sl_si91x_joystick_init(void)
{
  sl_status_t status                  = 0;
  sl_adc_channel_config.channel       = SL_JOYSTICK_ADC_CHANNEL;
  uint8_t adc_channel                 = sl_adc_channel_config.channel;
  sl_adc_config.num_of_channel_enable = 1;
  sl_adc_config.operation_mode        = SL_ADC_STATIC_MODE;

  sl_adc_channel_config.num_of_samples[adc_channel] = 1;
  vref_value                                        = (vref_value / DIVISION_MULTIPLIER);

  do {
    //Initialize ADC
    status = sl_si91x_adc_init(sl_adc_channel_config, sl_adc_config, vref_value);
    if (status != SL_STATUS_OK) {
      break;
    }
    //Configure ADC channel
    status = sl_si91x_adc_set_channel_configuration(sl_adc_channel_config, sl_adc_config);
    if (status != SL_STATUS_OK) {
      break;
    }
    //Register ADC conversion completion event callback function.
    status = sl_si91x_adc_register_event_callback(callback_event);
    if (status != SL_STATUS_OK) {
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * Get the position of Joystick.
 * This API read the ADC samples and compare with joystick position value to
 * update the direction on pos parameter.
 * Returning status of sl_status_t type.
 ******************************************************************************/
sl_status_t sl_si91x_joystick_get_position(sl_joystick_state_t state, sl_joystick_position_t *pos)
{
  sl_status_t status = 0;
  sl_joystick_position_t joystick_direction;
  uint16_t adc_value;
  volatile float sample_data = 0;
  do {
    if (state != SL_JOYSTICK_ENABLED) {
      status = SL_STATUS_NOT_READY;
      break;
    }
    // Check ADC start.
    if (AUX_ADC_DAC_COMP->AUXADC_CTRL_1_b.ADC_ENABLE == ENABLE) {
      //Read the ADC sampling data.
      status = sl_si91x_adc_read_data_static(sl_adc_channel_config, sl_adc_config, &adc_value);
      if (status != SL_STATUS_OK) {
        break;
      }
      if (adc_value & AUXADC_DATA_TWELFTH) {
        adc_value = (int16_t)(adc_value & (ADC_MASK_VALUE));
      } else {
        adc_value = adc_value | AUXADC_DATA_TWELFTH;
      }
      sample_data = (((float)adc_value / (float)ADC_MAX_OP_VALUE) * vref_value);
      sample_data = (sample_data * DIVISION_MULTIPLIER);
      // determine which direction pad was pressed
      if ((sample_data >= JOYSTICK_MV_C - JOYSTICK_MV_ERROR) && (sample_data <= JOYSTICK_MV_C + JOYSTICK_MV_ERROR)) {
        joystick_direction = SL_JOYSTICK_C;
      } else if ((sample_data >= JOYSTICK_MV_N - JOYSTICK_MV_ERROR)
                 && (sample_data <= JOYSTICK_MV_N + JOYSTICK_MV_ERROR)) {
        joystick_direction = SL_JOYSTICK_N;
      } else if ((sample_data >= JOYSTICK_MV_E - JOYSTICK_MV_ERROR)
                 && (sample_data <= JOYSTICK_MV_E + JOYSTICK_MV_ERROR)) {
        joystick_direction = SL_JOYSTICK_E;
      } else if ((sample_data >= JOYSTICK_MV_S - JOYSTICK_MV_ERROR)
                 && (sample_data <= JOYSTICK_MV_S + JOYSTICK_MV_ERROR)) {
        joystick_direction = SL_JOYSTICK_S;
      } else if ((sample_data >= JOYSTICK_MV_W - JOYSTICK_MV_ERROR)
                 && (sample_data <= JOYSTICK_MV_W + JOYSTICK_MV_ERROR)) {
        joystick_direction = SL_JOYSTICK_W;
      } else {
        joystick_direction = SL_JOYSTICK_NONE;
      }
      //Update direction on pointer variable.
      *pos = joystick_direction;
    }
  } while (false);
  return status;
}

/***************************************************************************/ /**
 * Starts Analog Joystick data acquisition.
 * This API enable the ADC peripheral.
 ******************************************************************************/
sl_status_t sl_si91x_joystick_start(sl_joystick_state_t state)
{
  sl_status_t status = 0;
  do {
    if (state == SL_JOYSTICK_DISABLED) {
      status = SL_STATUS_ABORT;
      break;
    }
    //Start/Enable the ADC peripheral
    status = sl_si91x_adc_start(sl_adc_config);
    if (status != SL_STATUS_OK) {
      break;
    }
  } while (false);
  return status;
}

/***************************************************************************/ /**
 * Stops Analog Joystick data acquisition.
 * This API disable the ADC peripheral.
 ******************************************************************************/
sl_status_t sl_si91x_joystick_stop(sl_joystick_state_t state)
{
  sl_status_t status = 0;
  do {
    if (state == SL_JOYSTICK_ENABLED) {
      status = SL_STATUS_BUSY;
      break;
    }
    //Stop/Disable the ADC peripheral
    status = sl_si91x_adc_stop(sl_adc_config);
    if (status != SL_STATUS_OK) {
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * Static callback function for handling the callback events.
 * When ADC operation mode in STATIC the ADC_STATIC_MODE_CALLBACK event
 * will trigger and unmask the adc_channel.
 ******************************************************************************/
static void callback_event(uint8_t channel_no, uint8_t event)
{
  (void)channel_no;
  switch (event) {
    case SL_INTERNAL_DMA:
      break;
    case SL_ADC_STATIC_MODE_EVENT:
      break;
  }
}
