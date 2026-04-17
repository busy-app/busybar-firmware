/******************************************************************************
* @file  sl_si91x_peripheral_sdc.c
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

//Include files

#include "sl_si91x_peripheral_sdc.h"
#include "rsi_power_save.h"

/**
 * @brief Configures the pin multiplexing for the SDC ADC input pins.
 *
 * This function sets up the pin multiplexing for the positive and negative input pins
 * of the SDC, as well as the input type configuration.
 *
 * @param pos_input_pinsel  Pin select value for the positive input.
 * @param neg_input_pinsel  Pin select value for the negative input.
 * @param input_type        Type of input configuration (e.g., single-ended, differential).
 */
static void sli_si91x_sdc_pin_mux(uint8_t pos_input_pinsel, uint8_t neg_input_pinsel, uint8_t input_type);

/*
* Configures the pin multiplexing for SDC ADC positive and negative input pins.
* Disables pad receiver for the selected analog pins and sets the pin mux for each possible positive and negative input pin.
* Handles both ULP and TOP GPIOs, and supports both single-ended and differential input types.
*/
static void sli_si91x_sdc_pin_mux(uint8_t pos_input_pinsel, uint8_t neg_input_pinsel, uint8_t input_type)
{
  switch (pos_input_pinsel) {
    case 0:
      RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO0);
      RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO0, ANALOG_MODE);
      break;
    case POS1:
      RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO2);
      RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO2, ANALOG_MODE);
      break;
    case POS2:
      RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO4);
      RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO4, ANALOG_MODE);
      break;
    case POS3:
      RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO6);
      RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO6, ANALOG_MODE);
      break;
    case POS4:
      RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO8);
      RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO8, ANALOG_MODE);
      break;
    case 5:
      RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO10);
      RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO10, ANALOG_MODE);
      break;
    case POS6:
      *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
      RSI_EGPIO_PadReceiverDisable(ADC_TOPGPIO25);
      RSI_EGPIO_HostPadsGpioModeDisable(ADC_TOPGPIO25);
      RSI_EGPIO_SetPinMux(EGPIO, 0, ADC_TOPGPIO25, TGPIO_ANALOG_MODE);
      break;
    case POS7:
      *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
      RSI_EGPIO_PadReceiverDisable(ADC_TOPGPIO27);
      RSI_EGPIO_HostPadsGpioModeDisable(ADC_TOPGPIO27);
      RSI_EGPIO_SetPinMux(EGPIO, 0, ADC_TOPGPIO27, TGPIO_ANALOG_MODE);
      break;
    case POS8:
      *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
      RSI_EGPIO_PadReceiverDisable(ADC_TOPGPIO29);
      RSI_EGPIO_HostPadsGpioModeDisable(ADC_TOPGPIO29);
      RSI_EGPIO_SetPinMux(EGPIO, 0, ADC_TOPGPIO29, TGPIO_ANALOG_MODE);
      break;
    default:
      // No action for other values
      break;
  }
  if ((pos_input_pinsel == POS10) || ((neg_input_pinsel == POS0) && (input_type == POS1))) {
    RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO1);
    RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO1, ANALOG_MODE);
  }
  if ((pos_input_pinsel == POS11) || ((neg_input_pinsel == POS1) && (input_type == POS1))) {
    RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO3);
    RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO3, ANALOG_MODE);
  }
  if ((pos_input_pinsel == POS12) || ((neg_input_pinsel == POS2) && (input_type == POS1))) {
    RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO5);
    RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO5, ANALOG_MODE);
  }
  if ((pos_input_pinsel == POS13) || ((neg_input_pinsel == POS3) && (input_type == POS1))) {
    RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO11);
    RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO11, ANALOG_MODE);
  }
  if ((pos_input_pinsel == POS14) || ((neg_input_pinsel == POS4) && (input_type == POS1))) {
    RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO9);
    RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO9, ANALOG_MODE);
  }
  if ((pos_input_pinsel == POS15) || ((neg_input_pinsel == POS5) && (input_type == POS1))) {
    RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO7);
    RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO7, ANALOG_MODE);
  }
  if ((pos_input_pinsel == POS16) || ((neg_input_pinsel == POS6) && (input_type == POS1))) {
    *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
    RSI_EGPIO_PadReceiverDisable(ADC_TOPGPIO26);
    RSI_EGPIO_HostPadsGpioModeDisable(ADC_TOPGPIO26);
    RSI_EGPIO_SetPinMux(EGPIO, 0, ADC_TOPGPIO26, TGPIO_ANALOG_MODE);
  }
  if ((pos_input_pinsel == POS17) || ((neg_input_pinsel == POS7) && (input_type == POS1))) {
    *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
    RSI_EGPIO_PadReceiverDisable(ADC_TOPGPIO28);
    RSI_EGPIO_HostPadsGpioModeDisable(ADC_TOPGPIO28);
    RSI_EGPIO_SetPinMux(EGPIO, 0, ADC_TOPGPIO28, TGPIO_ANALOG_MODE);
  }
  if ((pos_input_pinsel == POS18) || ((neg_input_pinsel == POS8) && (input_type == POS1))) {
    *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
    RSI_EGPIO_PadReceiverDisable(ADC_TOPGPIO30);
    RSI_EGPIO_HostPadsGpioModeDisable(ADC_TOPGPIO30);
    RSI_EGPIO_SetPinMux(EGPIO, 0, ADC_TOPGPIO30, TGPIO_ANALOG_MODE);
  }
}

/**
 * @brief  initializes the SDC hardware.
 *
 * @details 
 * 1. Configures the SDC reference clock selection to use the RC_32MHz clock.
 * 2. Enables the dynamic clock for SDC.
 * 3. Enables the static clock for SDC.
 * 4. Clears any pending SDC interrupts.
 * 5. Enables the SDC interrupt in the NVIC.
 * 6. Resets the SDC FIFO write pointer to ensure it starts from the beginning of the buffer. 
 * 
 */
sl_status_t sl_si91x_sdc_init(void)
{

  sl_status_t status = SL_STATUS_OK;

  MCU_FSM->MCU_FSM_REF_CLK_REG_b.SDCSS_CLK_SEL_b       = 0x01; //SDC clock selection enable (01-RC_32MHz,10-HF_RO clock)
  MCU_FSM->MCU_FSM_REF_CLK_REG_b.SDCSS_CLK_EN_b        = 0x01; //To enable dynamic clock for SDC
  MCU_FSM->MCU_FSM_REF_CLK_REG_b.SDCSS_STATIC_CLK_EN_b = 0x01; //To enable static clock for SDC

  sl_si91x_sdc_interrupt_clear();

  NVIC_EnableIRQ(NPSS_TO_MCU_SDC_INTR_IRQn);

  sl_si91x_sdc_fifo_reset();
  return status;
}

/**
 * @brief  Deinitializes the SDC hardware.
 * @details
 *    This function performs the following steps:
 *     1. Disables the SDC reference clock selection.
 *     2. Disables the dynamic clock for SDC.
 *     3. Disables the static clock for SDC.
 *     4. Clears any pending SDC interrupts.
 *     5. Disables the SDC interrupt in the NVIC.
 * 
 */
sl_status_t sl_si91x_sdc_deinit(void)
{

  sl_si91x_sdc_interrupt_clear();

  MCU_FSM->MCU_FSM_REF_CLK_REG_b.SDCSS_CLK_SEL_b       = 0x00; //SDC clock selection enable (01-RC_32MHz,10-HF_RO clock)
  MCU_FSM->MCU_FSM_REF_CLK_REG_b.SDCSS_CLK_EN_b        = 0x00; //To enable dynamic clock for SDC
  MCU_FSM->MCU_FSM_REF_CLK_REG_b.SDCSS_STATIC_CLK_EN_b = 0x00; //To enable static clock for SDC

  NVIC_DisableIRQ(NPSS_TO_MCU_SDC_INTR_IRQn);

  return SL_STATUS_OK;
}

/**
 * @brief   Configures the SDC general settings.
  * @details This function configures the SDC general parameters such as 
           1.sample threshold
           2.channel count
           3.sampling interval
           4.trigger event selection 
           5.clock division factor.
 * 
 */
sl_status_t sl_si91x_sdc_config(sl_si91x_sdc_config_t *sdc_general_config)
{

  sl_si91x_sdc_config_t *sdc_config = sdc_general_config;
  uint32_t number_of_channels       = sdc_config->number_of_channels;

  if (number_of_channels == 3 || number_of_channels > 4 || number_of_channels == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if ((sdc_config->sample_threshold * number_of_channels) > 14) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (sdc_config->sampling_interval > 1023) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (sdc_config->trigger_ms_sec_select > 1) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (sdc_config->clock_division_factor > 160 || sdc_config->clock_division_factor < 1) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  SDC->SDC_GEN_CONFIG_1_b.SAMP_THRESH   = sdc_config->sample_threshold;
  SDC->SDC_GEN_CONFIG_2_b.NUM_CH_SEL    = sdc_config->number_of_channels;
  SDC->SDC_GEN_CONFIG_3_b.CNT_TRIG_EVNT = sdc_config->sampling_interval;
  SDC->SDC_GEN_CONFIG_3_b.SAMP_TRIG_SEL = sdc_config->trigger_ms_sec_select;
  SDC->SDC_GEN_CONFIG_3_b.SDC_CLK_DIV   = sdc_config->clock_division_factor;

  return SL_STATUS_OK;
}

/**
 * @brief Clears the SDC interrupt status.
 * 
 */
void sl_si91x_sdc_interrupt_clear(void)
{
  SDC->SDC_GEN_CONFIG_0_b.INTR_STATUS_CLEAR = 1;
}

/**
 * @brief   Resets the SDC FIFO write pointer.
 * 
 */
void sl_si91x_sdc_fifo_reset(void)
{
  SDC->SDC_GEN_CONFIG_1_b.RST_WRT_PTR = 1;
  SDC->SDC_GEN_CONFIG_1_b.RST_WRT_PTR = 0;
}

/**
 * @brief   Disable static clock and Enables SDC sampling.
 * 
 */
void sl_si91x_sdc_sample_enable(void)
{
  MCU_FSM->MCU_FSM_REF_CLK_REG_b.SDCSS_STATIC_CLK_EN_b = 0x00; //Disable static clock for SDC
  SDC->SDC_GEN_CONFIG_2_b.SDC_SAMP_EN                  = ENABLE;
}

/**
 * @brief  Configures the SDC ADC input pins for all channels.
* @details
* This function performs the following steps:
* 1. Iterates through each channel in the provided SDC ADC configuration.
* 2. Calls the `sli_si91x_sdc_pin_mux` function to configure the pin multiplexing for the positive and negative input pins of each channel.
* 3. Sets the positive and negative input selections and differential mode for each channel in the SDC hardware registers.
* 4. Enables the SDC ADC and its configuration. 
 * 
 */
void sl_si91x_sdc_adc_pin_config(sl_si91x_sdc_adc_config_t *sdc_adc_config, uint32_t number_of_channels)
{
  for (uint32_t i = 0; i < number_of_channels; i++) {
    sli_si91x_sdc_pin_mux(sdc_adc_config[i].sdc_adc_p_input_pinsel,
                          sdc_adc_config[i].sdc_adc_n_input_pinsel,
                          sdc_adc_config[i].sdc_adc_mode);
  }

  SDC->SDC_AUXADC_CONFIG_1_b.SDC_AUXADC_INPUT_P_SEL_CH1 = sdc_adc_config[0].sdc_adc_p_input_pinsel; //c
  SDC->SDC_AUXADC_CONFIG_1_b.SDC_AUXADC_INPUT_N_SEL_CH1 = sdc_adc_config[0].sdc_adc_n_input_pinsel; //c
  SDC->SDC_AUXADC_CONFIG_1_b.SDC_AUXADC_DIFF_MODE_CH1   = sdc_adc_config[0].sdc_adc_mode;           //c

  if (number_of_channels > 1) {

    SDC->SDC_AUXADC_CONFIG_2_b.SDC_AUXADC_INPUT_P_SEL_CH2 = sdc_adc_config[1].sdc_adc_p_input_pinsel;
    SDC->SDC_AUXADC_CONFIG_2_b.SDC_AUXADC_INPUT_N_SEL_CH2 = sdc_adc_config[1].sdc_adc_n_input_pinsel;
    SDC->SDC_AUXADC_CONFIG_2_b.SDC_AUXADC_DIFF_MODE_CH2   = sdc_adc_config[1].sdc_adc_mode;
  }
  if (number_of_channels > 2) {
    SDC->SDC_AUXADC_CONFIG_3_b.SDC_AUXADC_INPUT_P_SEL_CH3 = sdc_adc_config[2].sdc_adc_p_input_pinsel;
    SDC->SDC_AUXADC_CONFIG_3_b.SDC_AUXADC_INPUT_N_SEL_CH3 = sdc_adc_config[2].sdc_adc_n_input_pinsel;
    SDC->SDC_AUXADC_CONFIG_3_b.SDC_AUXADC_DIFF_MODE_CH3   = sdc_adc_config[2].sdc_adc_mode;
  }

  if (number_of_channels > 3) {
    SDC->SDC_AUXADC_CONFIG_4_b.SDC_AUXADC_INPUT_P_SEL_CH4 = sdc_adc_config[3].sdc_adc_p_input_pinsel;
    SDC->SDC_AUXADC_CONFIG_4_b.SDC_AUXADC_INPUT_N_SEL_CH4 = sdc_adc_config[3].sdc_adc_n_input_pinsel;
    SDC->SDC_AUXADC_CONFIG_4_b.SDC_AUXADC_DIFF_MODE_CH4   = sdc_adc_config[3].sdc_adc_mode;
  }

  SDC->SDC_AUXADC_CONFIG_1_b.SDC_AUXADC_EN     = ENABLE;
  SDC->SDC_AUXADC_CONFIG_1_b.SDC_ADC_CONFIG_EN = ENABLE;
}

/**
 * @brief Enables the LDO for the SDC peripheral.
 * 
 */
void sl_si91x_sdc_ldo_enable(void)
{
  SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_EN = 1;
}

/**
 * @brief   Configures the SDC LDO voltage based on the reference voltage and chip voltage.
 * @details
* This function performs the following steps:
* 1. Enables the SDC LDO and its configuration.
* 2. Checks if the reference voltage is above 2.8V or chip voltage is below 3.0V, and enables LDO bypass mode if true.
* 3. If the reference voltage is below 1.6V, disables the LDO and its configuration.
* 4. Otherwise, calculates the LDO voltage control value based on the reference voltage and sets it in the SDC LDO configuration register.
* 5. Ensures the calculated voltage control value is within the valid range (0 to 15).
 * 
 */
void sl_si91x_sdc_ldo_config(float vref_voltage, uint32_t chip_voltage)
{
  uint8_t temp                                  = 0;
  float cntrl_val                               = 0;
  SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_EN        = 1;
  SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_CONFIG_EN = 1;
  if ((vref_voltage > (float)2.8) || (chip_voltage < (float)3.0)) {
    SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_BYP_EB = 1;
  } else {
    if (vref_voltage < (float)1.6) {
      SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_VOLT_CTRL = 0;
      SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_EN        = 0;
      SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_CONFIG_EN = 0;
    } else {
      cntrl_val = ((vref_voltage - (float)1.6) / (float)(0.08));
      temp      = (uint8_t)(cntrl_val + 1);
      cntrl_val = (cntrl_val + (float)0.50);
      if ((float)temp > cntrl_val) {
        temp = (uint8_t)cntrl_val;
      }
      SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_VOLT_CTRL = (unsigned int)(temp & 0x0F);
    }
  }
}

/**
 * @brief  Disables the SDC LDO.
 * 
 */
void sl_si91x_sdc_ldo_disable(void)
{
  SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_EN = 0;
}

/**
 * @brief   Bypasses the SDC LDO.
 */
void sl_si91x_sdc_ldo_bypass(void)
{
  SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_BYP_EB = 1;
}

/**
 * @brief   Reads the SDC FIFO data at the specified position.
* @details
* This function performs the following steps:
* 1. Checks if the provided FIFO position is valid (0 to 15).
* 2. Checks if the provided data and channel ID pointers are not NULL.
* 3. Reads the data from the SDC FIFO at the specified position and stores it in the provided data pointer.
* 4. Stores the channel ID from the FIFO data register in the provided channel ID pointer
* 5. Returns SL_STATUS_OK if successful, or an appropriate error code if any checks fail.
 * 
 */
sl_status_t sl_si91x_sdc_read_data(int16_t *data, uint32_t fifo_position, uint32_t *channel_id)
{
  if (fifo_position > 15) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (data == NULL || channel_id == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // store the data from the FIFO at the specified position
  *data = SDC->SDC_DATA_REG[fifo_position].DATA_REG_b.SDC_DATA_SAMPLE;

  // store the channel ID from the FIFO data register
  *channel_id = SDC->SDC_DATA_REG[fifo_position].DATA_REG_b.SMP_ID_CH;

  return SL_STATUS_OK;
}

/**
 * @brief   Stops the SDC data sampling.
 * 
 */
void sl_si91x_sdc_stop(void)
{
  SDC->SDC_GEN_CONFIG_2_b.SDC_SAMP_EN = DISABLE;
}
