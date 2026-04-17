/******************************************************************************
* @file  rsi_adc.c
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

// Include Files

#include "UDMA.h"
#include "rsi_adc.h"
#include "rsi_ulpss_clk.h"
#include "rsi_rom_ulpss_clk.h"
#include "rsi_dac.h"
#include "rsi_rom_egpio.h"
#include "aux_reference_volt_config.h"
#include <string.h>
#include "sl_si91x_dma.h"
#include "clock_update.h"
//---------------------- Macros -----------------//
#define MAXIMUM_ADC_SAMPLE_LEN    1023
#define MINIMUM_ADC_SAMPLE_LEN    1
#define SAMPLE_RATE_32KSPS        32000
#define SAMPLE_RATE_9KSPS         9000
#define SAMPLE_RATE_800KSPS       800000
#define MAXIMUM_NUMBER_OF_CHANNEL 16
#define MINIMUM_NUMBER_OF_CHANNEL 1
#define FIRST_MEM_SWITCH_INTR_BIT 7
#if defined(SLI_SI91X_MCU_ENABLE_RAM_BASED_EXECUTION)
#define DMA_INSTANCE 1
#else
#define DMA_INSTANCE 0
#endif
#define DMA_CHANNEL 1
// Define macros for repeated operations
#define CONFIGURE_ADC_ULP_GPIO(gpio_pin)     \
  RSI_EGPIO_UlpPadReceiverDisable(gpio_pin); \
  RSI_EGPIO_SetPinMux(EGPIO1, 0, gpio_pin, ANALOG_MODE)

#define CONFIGURE_ADC_TOP_GPIO(gpio_pin)       \
  RSI_EGPIO_PadReceiverDisable(gpio_pin);      \
  RSI_EGPIO_HostPadsGpioModeDisable(gpio_pin); \
  RSI_EGPIO_SetPinMux(EGPIO, 0, gpio_pin, TGPIO_ANALOG_MODE)
// ADC internal structure
adc_inter_config_t adcInterConfig;
adc_commn_config_t adc_commn_config;
// ADC channel number.
uint8_t adc_channel = 0;
// ADC calibration related variables
uint32_t auxadcCalibValueLoad = 0;
uint32_t auxadcCalibValue     = 0;
uint32_t calib_done           = 0;
// ADC ping or pong interrupt selection variable
uint8_t pong_enable_sel[MAXIMUM_NUMBER_OF_CHANNEL];

#if defined(SLI_SI917)
extern dac_config_t dac_callback_fun;
#endif
#ifdef ADC_MULTICHANNEL_WITH_EXT_DMA
extern int16_t adc_output_buf[UDMA_PING_PONG_LENGTH * 2];
// Channel sorting related veriable
uint16_t ch_data[16];
extern adc_commn_config_t adc_commn_config;
extern RSI_UDMA_HANDLE_T udmaHandle1;
// DMA descriptors must be aligned to 16 bytes
#if defined(__CC_ARM)
#ifdef PS2_STATE_ADC_DATA_AQU_ENABLE
int16_t adc_output_buf[UDMA_PING_PONG_LENGTH * 2] __attribute__((at(ADC_DATA_AQU_BUF_PS2)));
#else
int16_t adc_output_buf[UDMA_PING_PONG_LENGTH * 2] __attribute__((at(ADC_DATA_AQU_BUF_PS4)));
#endif
extern RSI_UDMA_DESC_T UDMA1_Table[32];
#endif // defined (__CC_ARM)

#if defined(__GNUC__)
#ifdef PS2_STATE_ADC_DATA_AQU_ENABLE
int16_t __attribute__((section(".adc_multi_ch_buf_ps2_addr"))) adc_output_buf[UDMA_PING_PONG_LENGTH * 2];
#else
int16_t __attribute__((section(".adc_multi_ch_buf_ps4_addr"))) adc_output_buf[UDMA_PING_PONG_LENGTH * 2];
#endif
extern RSI_UDMA_DESC_T __attribute__((section(".udma_addr1"))) UDMA1_Table[32];
#endif // defined (__GNUC__)
#endif

static void ADC_ConfigurePositiveInputPin(uint8_t pos_input_pinsel);
static void ADC_ConfigureNegativeInputPin(uint8_t neg_input_pinsel);

/** @addtogroup SOC13
* @{
*/
/*==============================================*/
/**
 * @fn         rsi_error_t ADC_Init(adc_ch_config_t adcChConfig,adc_config_t adcConfig,adccallbacFunc event)
 * @brief      This API use for initialize ADC.
 * @param[in]  adcChConfig  : ADC channels configuration structure variable.
 * @param[in]  adcConfig    : ADC operation configuration structure variable. 
 * @param[in]  event        : Callback function pointer.
 * @return     rsi_error_t      : Execution status,This function return 'RSI_OK' on
 *                            successful execution.  
 */
rsi_error_t ADC_Init(adc_ch_config_t adcChConfig, adc_config_t adcConfig, adccallbacFunc event)
{
#ifndef SIMULATION
  uint32_t integer_val = 0;
  float frac           = 0;
#endif

  // Register callback event
  adc_commn_config.call_back_event = event;

  // Configure receive buffer
  adc_commn_config.num_of_channel_enable = adcConfig.num_of_channel_enable;

  // Verify the user given sampling rate is proper or not
  if ((adcChConfig.sampling_rate[0] > MAX_SINGCH_SAMPLING_RATE) || (adcChConfig.sampling_rate[0] == 0)) {
    return INVALID_SAMPLING_RATE;
  }

  // Verify the user given sampling rate is proper or not
  for (uint32_t ch_num = 1; ch_num < adcConfig.num_of_channel_enable; ch_num++) {
    if (adcChConfig.sampling_rate[ch_num] > adcChConfig.sampling_rate[ch_num - 1]
        || (adcChConfig.sampling_rate[ch_num] == 0)) {
      return INVALID_SAMPLING_RATE;
    }
  }

  // Verify the user given sample length is proper or not
  if ((adcConfig.num_of_channel_enable == 1)
      && ((adcConfig.operation_mode && (adcChConfig.num_of_samples[0] > MINIMUM_ADC_SAMPLE_LEN))
          || (adcChConfig.num_of_samples[0] > MAXIMUM_ADC_SAMPLE_LEN))) {
    return INVALID_SAMPLE_LENGTH;
  }

  if ((adcConfig.num_of_channel_enable < MINIMUM_NUMBER_OF_CHANNEL)
      || (adcConfig.num_of_channel_enable > MAXIMUM_NUMBER_OF_CHANNEL)) {
    return INVALID_ADC_CHANNEL_ENABLE;
  }
  // Power up of ADC block
  RSI_ADC_PowerControl(ADC_POWER_ON);

  // Clock division factor for calibration,Calibrate ADC on 4MHz clock
  RSI_ADC_ClkDivfactor(AUX_ADC_DAC_COMP, 0, 4);

  // Set analog reference voltage
  RSI_AUX_RefVoltageConfig((float)2.8, (float)3.2);

  adc_commn_config.adc_clk_src = system_clocks.ulpss_ref_clk;

  // ADC Calibration
  RSI_ADC_Calibration();

#ifdef SIMULATION
  adc_commn_config.adc_sing_offset = 0x0;
  adc_commn_config.adc_diff_offset = 0x0;
  adc_commn_config.adc_diff_gain   = 0x0;
  adc_commn_config.adc_sing_gain   = 0x0;
#else

  // Offset value and gain value read from efuse
  adc_commn_config.adc_sing_offset = (uint16_t)RSI_IPMU_Auxadcoff_SeEfuse();
  adc_commn_config.adc_diff_offset = (uint16_t)RSI_IPMU_Auxadcoff_DiffEfuse();

  // Single ended gain
  integer_val = RSI_IPMU_Auxadcgain_SeEfuse();
  frac        = (float)(integer_val & 0x3FFF);
  frac /= 1000;
  adc_commn_config.adc_sing_gain = ((float)(integer_val >> 14) + frac);

  // Differential ended gain
  integer_val = RSI_IPMU_Auxadcgain_DiffEfuse();
  frac        = (float)(integer_val & 0x3FFF);
  frac /= 1000;
  adc_commn_config.adc_diff_gain = (((float)(integer_val >> 14)) + frac);
#endif
  sl_dma_init_t dma_init;
  dma_init.dma_number = DMA_INSTANCE;
  if (sl_si91x_dma_init(&dma_init)) {
    return ARM_DRIVER_ERROR;
  }
  return RSI_OK;
}

// Revisit for optimization
rsi_error_t ADC_Per_Channel_Init(adc_ch_config_t adcChConfig, adc_config_t adcConfig, adccallbacFunc event)
{
#ifndef SIMULATION
  uint32_t integer_val = 0;
  float frac           = 0;
#endif

  if (adcChConfig.channel > MAX_CHNL_NO) {
    return INVALID_PARAMETERS;
  }

  adc_channel = adcChConfig.channel;

  // Register callback event
  adc_commn_config.call_back_event = event;

  // Configure receive buffer
  adc_commn_config.num_of_channel_enable = adcConfig.num_of_channel_enable;

  // Verify the user given sampling rate is proper or not
  if ((adcChConfig.sampling_rate[adc_channel] > MAX_SINGCH_SAMPLING_RATE)
      || (adcChConfig.sampling_rate[adc_channel] == 0)) {
    return INVALID_SAMPLING_RATE;
  }

  // Verify the user given sample length is proper or not
  if ((adcConfig.num_of_channel_enable == 1)
      && ((adcConfig.operation_mode && (adcChConfig.num_of_samples[adc_channel] > MINIMUM_ADC_SAMPLE_LEN))
          || (adcChConfig.num_of_samples[adc_channel] > MAXIMUM_ADC_SAMPLE_LEN))) {
    return INVALID_SAMPLE_LENGTH;
  }

  if ((adcConfig.num_of_channel_enable < MINIMUM_NUMBER_OF_CHANNEL)
      || (adcConfig.num_of_channel_enable > MAXIMUM_NUMBER_OF_CHANNEL)) {
    return INVALID_ADC_CHANNEL_ENABLE;
  }
  // Power up of ADC block
  RSI_ADC_PowerControl(ADC_POWER_ON);

  // Clock division factor for calibration,Calibrate ADC on 4MHz clock
  RSI_ADC_ClkDivfactor(AUX_ADC_DAC_COMP, 0, 4);

  // Set analog reference voltage
  RSI_AUX_RefVoltageConfig((float)2.8, (float)3.2);

  // ADC Calibration
  RSI_ADC_Calibration();

  adc_commn_config.adc_clk_src = system_clocks.ulpss_ref_clk;

#ifdef SIMULATION
  adc_commn_config.adc_sing_offset = 0x0;
  adc_commn_config.adc_diff_offset = 0x0;
  adc_commn_config.adc_diff_gain   = 0x0;
  adc_commn_config.adc_sing_gain   = 0x0;
#else
  // Offset value and gain value read from efuse
  adc_commn_config.adc_sing_offset = (uint16_t)RSI_IPMU_Auxadcoff_SeEfuse();
  adc_commn_config.adc_diff_offset = (uint16_t)RSI_IPMU_Auxadcoff_DiffEfuse();

  // Single ended gain
  integer_val = RSI_IPMU_Auxadcgain_SeEfuse();
  frac        = (float)(integer_val & 0x3FFF);
  frac /= 1000;
  adc_commn_config.adc_sing_gain = ((float)(integer_val >> 14) + frac);

  // Differential ended gain
  integer_val = RSI_IPMU_Auxadcgain_DiffEfuse();
  frac        = (float)(integer_val & 0x3FFF);
  frac /= 1000;
  adc_commn_config.adc_diff_gain = ((float)(integer_val >> 14) + frac);
#endif
  sl_dma_init_t dma_init;
  dma_init.dma_number = DMA_INSTANCE;
  if (sl_si91x_dma_init(&dma_init)) {
    return ARM_DRIVER_ERROR;
  }
  return RSI_OK;
}

/*===================================================*/
/**
 * @fn          void config_ch_freq(adc_ch_config_t adcChConfig, adc_config_t adcConfig)
 * @brief       configure channel frequency value for channel1 to number of channel enable
 * @param[in]   adcChConfig  :  ADC channels configuration structure variable
 * @param[in]   adcConfig    :  ADC operation configuration structure variable
 * @return      none   
 */
STATIC INLINE void config_ch_freq(adc_ch_config_t adcChConfig, adc_config_t adcConfig)
{
  bool per_ch_config                        = false;
  float inverse_sampl_val                   = 0;
  uint32_t fs_adc                           = 0;
  uint16_t loop_index                       = 0;
  const volatile uint16_t *sampl_rate_index = (const volatile uint16_t *)&loop_index;

  // find out initial sampling rate
  fs_adc = (adc_commn_config.adc_clk_src / adc_commn_config.total_clk);

  // to find out if it is triggered by single channel or multiple channel
  if ((adcConfig.operation_mode != ADC_FIFOMODE_ENABLE)
      || (adcConfig.num_of_channel_enable == MINIMUM_NUMBER_OF_CHANNEL)) {
    per_ch_config    = true;
    sampl_rate_index = (const volatile uint16_t *)&adc_channel;
  }

  // Calculate the sampling factor for the current channel
  // Sampling factor = ADC clock frequency / desired sampling rate
  // The factor determines how many ADC clock cycles occur between samples
  float calculated_factor  = (float)fs_adc / (float)adcChConfig.sampling_rate[*sampl_rate_index];
  uint32_t sampling_factor = (uint32_t)adc_dac_ceil(calculated_factor);

  // Clamp the sampling factor to UINT16_MAX to prevent overflow
  if (sampling_factor > UINT16_MAX) {
    sampling_factor = UINT16_MAX;
  }

  adcInterConfig.ch_sampling_factor[*sampl_rate_index] = (uint16_t)sampling_factor;

  if (adcInterConfig.ch_sampling_factor[*sampl_rate_index] < 2) {
    adcInterConfig.ch_sampling_factor[*sampl_rate_index] = 2;
  }

  // Configure channel frequency value for channel1 to number of channel enable
  for (uint8_t ch_num = 0; ch_num < adcConfig.num_of_channel_enable; ch_num++) {
    inverse_sampl_val = 0;

    if (ch_num > 0) {
      // Get channel frequency value
      // Configure channel frequency value
      adcInterConfig.ch_sampling_factor[ch_num] = adcInterConfig.ch_sampling_factor[ch_num - 1]
                                                  * (uint16_t)adc_dac_ceil((float)adcChConfig.sampling_rate[ch_num - 1]
                                                                           / (float)adcChConfig.sampling_rate[ch_num]);
    }

    /* Check configured channel frequency value is power of 2 or not,
      If configured channel frequency value is not power of 2 then round of value with nearest power of 2 value */
    if ((!(adcConfig.num_of_channel_enable == 1)) && (!(check_power_two(adcInterConfig.ch_sampling_factor[ch_num])))) {
      adcInterConfig.ch_sampling_factor[ch_num] = (uint16_t)(roundupto_pwr2(adcInterConfig.ch_sampling_factor[ch_num]));
    }

    // if number of sample length is '1' it should divide by 4 for swallow factor
    if ((per_ch_config == true) && (adcChConfig.num_of_samples[*sampl_rate_index] == 1)) {
      adcInterConfig.ch_sampling_factor[*sampl_rate_index] /= 4;
    }
    // Sum of enabled channels channel frequency inverse
    for (loop_index = 0; loop_index <= ch_num; loop_index++) {
      inverse_sampl_val += (1 / (float)adcInterConfig.ch_sampling_factor[*sampl_rate_index]);
    }

    // Check sum of enabled channels channel frequency inverse is greater than 1
    if (inverse_sampl_val > 1) {
      for (loop_index = 0; loop_index <= ch_num; loop_index++) {
        adcInterConfig.ch_sampling_factor[*sampl_rate_index] *= 2;
      }
    }
  }
}

/*==============================================*/
/**
 * @fn         rsi_error_t ADC_ChannelConfig(adc_ch_config_t adcChConfig,adc_config_t adcConfig)
 * @brief      This API used for configure parameter to ADC channels.
 * @param[in]  adcChConfig  :  ADC channels configuration structure variable.
 * @param[in]  adcConfig    :  ADC operation configuration structure variable. 
 * @return     rsi_error_t      :  Execution status,This function return 'RSI_OK' on
 *                             successful execution.  
 */
rsi_error_t ADC_ChannelConfig(adc_ch_config_t adcChConfig, adc_config_t adcConfig)
{
  uint32_t f_sample_rate_achive = 0;
  uint16_t total_clk            = 0;
  uint16_t on_clk               = 0;
  sl_status_t status;
  uint32_t dma_channel      = DMA_CHANNEL;
  uint32_t channel_priority = 0;
  float min_sampl_time      = 0;
  adc_ping_pong_memory_adr_config_t PingPongMemoryAdrConfig;

  sl_dma_callback_t adc_dma_callback;

  // Get minimum sampling time form given configuration
  min_sampl_time = get_min_sampling_time(adcConfig.num_of_channel_enable, adcChConfig);

  // Find out the maximum sampling rate of ADC can achieved from given configuration
  f_sample_rate_achive = max_sample_rate_achive(min_sampl_time); // Need to implement   Step2

  // Find out total number of ADC cycle
  total_clk = (uint16_t)adc_dac_ceil((1 / (float)f_sample_rate_achive) / (1 / (float)adc_commn_config.adc_clk_src));

  // Find out number of ON cycles in total ADC cycle
  on_clk = (uint16_t)adc_dac_ceil(min_sampl_time / (1 / (float)adc_commn_config.adc_clk_src));

  if (total_clk == on_clk) {
    total_clk += 1;
  }

  // modify the total cycle number for ADC static mode operation
  if (adcConfig.operation_mode
      && (total_clk < adc_dac_ceil((float)adc_commn_config.adc_clk_src / (float)adcChConfig.sampling_rate[0]))) {
    total_clk = (uint16_t)adc_dac_ceil((float)adc_commn_config.adc_clk_src / (float)adcChConfig.sampling_rate[0]);
  }

  adc_commn_config.on_clk    = on_clk;
  adc_commn_config.total_clk = total_clk;

  if (!(adcConfig.operation_mode)) {
    // Configure channel frequency value for channel1 to number of channel enable
    config_ch_freq(adcChConfig, adcConfig);

    cal_adc_channel_offset();

#ifdef ADC_MULTICHANNEL_WITH_EXT_DMA
    if (adcConfig.num_of_channel_enable > 1) {
      UDMA_ADC_Init();
    }
#endif
  }

  // Configure ADC clock division factor
  RSI_ADC_ClkDivfactor(AUX_ADC_DAC_COMP, adc_commn_config.on_clk, adc_commn_config.total_clk); //Need to review

  // Enable noise average mode
  RSI_ADC_NoiseAvgMode(AUX_ADC_DAC_COMP, ENABLE);

  if (adcConfig.operation_mode) /* If ADC is working on static mode */
  {
#ifdef CHIP_9118
    // Configure the ADC in static mode
    RSI_ADC_Config(AUX_ADC_DAC_COMP, DYNAMIC_MODE_DI, adcConfig.operation_mode, 0, 0);
#endif
#if defined(SLI_SI917)
    RSI_ADC_Config(AUX_ADC_DAC_COMP, DYNAMIC_MODE_DI, adcConfig.operation_mode, 0, 0, 0);
#endif

    // Configure the input pin selection in static mode
    RSI_ADC_StaticMode(AUX_ADC_DAC_COMP,
                       adcChConfig.pos_inp_sel[0],
                       adcChConfig.neg_inp_sel[0],
                       adcChConfig.input_type[0]);

    // configures user given pins in analog mode
    ADC_PinMux(adcChConfig.pos_inp_sel[0], adcChConfig.neg_inp_sel[0], adcChConfig.input_type[0]);

#if defined(SLI_SI917)
    RSI_ADC_ChnlIntrUnMask(AUX_ADC_DAC_COMP, 0, adcConfig.operation_mode);
#endif
    adcInterConfig.achived_sampling_rate[0] = adc_commn_config.adc_clk_src / adc_commn_config.total_clk;
  } else {
#ifdef CHIP_9118
    // Configure ADC in FIFO mode operation
    if (adcConfig.num_of_channel_enable != 1) {
      RSI_ADC_Config(AUX_ADC_DAC_COMP, DYNAMIC_MODE_EN, ADC_FIFOMODE_ENABLE, ADC_FIFO_THR, EXTERNAL_DMA_EN);
    } else {
      RSI_ADC_Config(AUX_ADC_DAC_COMP, DYNAMIC_MODE_EN, ADC_FIFOMODE_ENABLE, ADC_FIFO_THR, INTERNAL_DMA_EN);
    }
#endif
#if defined(SLI_SI917)
#ifdef ADC_MULTICHANNEL_WITH_EXT_DMA
    RSI_ADC_Config(AUX_ADC_DAC_COMP,
                   DYNAMIC_MODE_EN,
                   ADC_FIFOMODE_ENABLE,
                   AEMPTY_THRSHOLD,
                   AFULL_THRSHOLD,
                   EXTERNAL_DMA_EN);
#else
    RSI_ADC_Config(AUX_ADC_DAC_COMP,
                   DYNAMIC_MODE_EN,
                   ADC_FIFOMODE_ENABLE,
                   AEMPTY_THRSHOLD,
                   AFULL_THRSHOLD,
                   INTERNAL_DMA_EN);
#endif
#endif

    for (uint8_t ch_num = 0; ch_num < adcConfig.num_of_channel_enable; ch_num++) {
      RSI_ADC_ChannelConfig(AUX_ADC_DAC_COMP,
                            ch_num,
                            adcChConfig.pos_inp_sel[ch_num],
                            adcChConfig.neg_inp_sel[ch_num],
                            adcChConfig.input_type[ch_num]);

      // Configure the sampling rate of ADC
      RSI_ADC_ChannelSamplingRate(AUX_ADC_DAC_COMP,
                                  ch_num,
                                  adcInterConfig.ch_offset_val[ch_num],
                                  adcInterConfig.ch_sampling_factor[ch_num]);

      // ADC channel enable
      RSI_ADC_ChnlEnable(AUX_ADC_DAC_COMP, ch_num);

      ADC_PinMux(adcChConfig.pos_inp_sel[ch_num], adcChConfig.neg_inp_sel[ch_num], adcChConfig.input_type[ch_num]);

#ifndef ADC_MULTICHANNEL_WITH_EXT_DMA
      RSI_ADC_InternalPerChnlDmaEnable(AUX_ADC_DAC_COMP, ch_num);
      PingPongMemoryAdrConfig.ping_addr   = adcChConfig.chnl_ping_address[ch_num];
      PingPongMemoryAdrConfig.pong_addr   = adcChConfig.chnl_pong_address[ch_num];
      PingPongMemoryAdrConfig.ping_length = adcChConfig.num_of_samples[ch_num];
      PingPongMemoryAdrConfig.pong_length = adcChConfig.num_of_samples[ch_num];
      PingPongMemoryAdrConfig.ping_enable = ADC_PING_EN;
      PingPongMemoryAdrConfig.pong_enable = ADC_PONG_EN;
      RSI_ADC_PingPongMemoryAdrConfig(AUX_ADC_DAC_COMP, ch_num, PingPongMemoryAdrConfig);
      RSI_ADC_PingpongEnable(AUX_ADC_DAC_COMP, ch_num);
#ifdef CHIP_9118
      RSI_ADC_ChnlIntrUnMask(AUX_ADC_DAC_COMP, ch_num);
#endif
#if defined(SLI_SI917)
      RSI_ADC_ChnlIntrUnMask(AUX_ADC_DAC_COMP, ch_num, adcConfig.operation_mode);
#endif
#endif
      adcInterConfig.achived_sampling_rate[ch_num] = ((adc_commn_config.adc_clk_src / adc_commn_config.total_clk)
                                                      / adcInterConfig.ch_sampling_factor[ch_num]); // Need to review

      adcInterConfig.num_of_samples[ch_num] = adcChConfig.num_of_samples[ch_num];

      adcInterConfig.rx_buf[ch_num] = adcChConfig.rx_buf[ch_num];

      adcInterConfig.input_type[ch_num] = adcChConfig.input_type[ch_num];
    }

    adc_dma_callback.transfer_complete_cb = ADC_DMA_Transfer_Complete;
    adc_dma_callback.error_cb             = ADC_DMA_Error_Callback;
    //Allocate DMA channel for Tx
    status = sl_si91x_dma_allocate_channel(DMA_INSTANCE, &dma_channel, channel_priority);
    if (status && (status != SL_STATUS_DMA_CHANNEL_ALLOCATED)) {
      return ARM_DRIVER_ERROR;
    }
    //Register transfer complete and error callback
    if (sl_si91x_dma_register_callbacks(DMA_INSTANCE, dma_channel, &adc_dma_callback)) {
      return ARM_DRIVER_ERROR;
    }
  }
  return RSI_OK;
}

// Revisit for optimization
rsi_error_t ADC_Per_ChannelConfig(adc_ch_config_t adcChConfig, adc_config_t adcConfig)
{
  uint32_t f_sample_rate_achive = 0;
  uint16_t total_clk            = 0;
  uint16_t on_clk               = 0;
  sl_status_t status;
  uint32_t dma_channel      = DMA_CHANNEL;
  uint32_t channel_priority = 0;
  float min_sampl_time      = 0;
  adc_ping_pong_memory_adr_config_t PingPongMemoryAdrConfig;

  if (adcChConfig.channel > MAX_CHNL_NO) {
    return INVALID_PARAMETERS;
  }

  adc_channel = adcChConfig.channel;
  sl_dma_callback_t adc_dma_callback;

  // Get minimum sampling time from given configuration
  min_sampl_time = get_min_sampling_time(adcConfig.num_of_channel_enable, adcChConfig);

  // Find out the maximum sampling rate of ADC can achieved from given configuration
  f_sample_rate_achive = max_sample_rate_achive(min_sampl_time); // Need to implement   Step2

  // Find out total number of ADC cycle
  total_clk = (uint16_t)adc_dac_ceil((1 / (float)f_sample_rate_achive) / (1 / (float)adc_commn_config.adc_clk_src));

  // Find out number of ON cycles in total ADC cycle
  on_clk = (uint16_t)adc_dac_ceil(min_sampl_time * (float)adc_commn_config.adc_clk_src);

  if (total_clk == on_clk) {
    total_clk = total_clk + 1;
  }

  // modify the total cycle number for ADC static mode operation
  if ((adcConfig.operation_mode)
      && (total_clk
          < adc_dac_ceil((float)adc_commn_config.adc_clk_src / (float)adcChConfig.sampling_rate[adc_channel]))) {
    total_clk =
      (uint16_t)adc_dac_ceil((float)adc_commn_config.adc_clk_src / (float)adcChConfig.sampling_rate[adc_channel]);
  }

  adc_commn_config.on_clk    = on_clk;
  adc_commn_config.total_clk = total_clk;

  if (!(adcConfig.operation_mode)) {
    // Configure channel frequency value for channel1 to number of channel enable
    config_ch_freq(adcChConfig, adcConfig);

    cal_adc_channel_offset();

#ifdef ADC_MULTICHANNEL_WITH_EXT_DMA
    if (adcConfig.num_of_channel_enable > 1) {
      UDMA_ADC_Init();
    }
#endif
  }

  // Configure ADC clock division factor
  RSI_ADC_ClkDivfactor(AUX_ADC_DAC_COMP, adc_commn_config.on_clk, adc_commn_config.total_clk); //Need to review

  // Enable noise average mode
  RSI_ADC_NoiseAvgMode(AUX_ADC_DAC_COMP, ENABLE);

  if (adcConfig.operation_mode) /* If ADC is working on static mode */
  {
#ifdef CHIP_9118
    // Configure the ADC in static mode
    RSI_ADC_Config(AUX_ADC_DAC_COMP, DYNAMIC_MODE_DI, adcConfig.operation_mode, 0, 0);
#endif
#if defined(SLI_SI917)
    RSI_ADC_Config(AUX_ADC_DAC_COMP, DYNAMIC_MODE_DI, adcConfig.operation_mode, 0, 0, 0);
#endif

    // Configure the input pin selection in static mode
    RSI_ADC_StaticMode(AUX_ADC_DAC_COMP,
                       adcChConfig.pos_inp_sel[adc_channel],
                       adcChConfig.neg_inp_sel[adc_channel],
                       adcChConfig.input_type[adc_channel]);

    // configures user given pins in analog mode
    ADC_PinMux(adcChConfig.pos_inp_sel[adc_channel],
               adcChConfig.neg_inp_sel[adc_channel],
               adcChConfig.input_type[adc_channel]);

#if defined(SLI_SI917)
    RSI_ADC_ChnlIntrUnMask(AUX_ADC_DAC_COMP, adc_channel, adcConfig.operation_mode);
#endif
    adcInterConfig.achived_sampling_rate[adc_channel] = (adc_commn_config.adc_clk_src / adc_commn_config.total_clk);
  } else {
#ifdef CHIP_9118
    // Configure ADC in FIFO mode operation
    if ((adcConfig.num_of_channel_enable) != 1) {
      RSI_ADC_Config(AUX_ADC_DAC_COMP, DYNAMIC_MODE_EN, ADC_FIFOMODE_ENABLE, ADC_FIFO_THR, EXTERNAL_DMA_EN);
    } else {
      RSI_ADC_Config(AUX_ADC_DAC_COMP, DYNAMIC_MODE_EN, ADC_FIFOMODE_ENABLE, ADC_FIFO_THR, INTERNAL_DMA_EN);
    }
#endif
#if defined(SLI_SI917)
#ifdef ADC_MULTICHANNEL_WITH_EXT_DMA
    RSI_ADC_Config(AUX_ADC_DAC_COMP,
                   DYNAMIC_MODE_EN,
                   ADC_FIFOMODE_ENABLE,
                   AEMPTY_THRSHOLD,
                   AFULL_THRSHOLD,
                   EXTERNAL_DMA_EN);
#else
    RSI_ADC_Config(AUX_ADC_DAC_COMP,
                   DYNAMIC_MODE_EN,
                   ADC_FIFOMODE_ENABLE,
                   AEMPTY_THRSHOLD,
                   AFULL_THRSHOLD,
                   INTERNAL_DMA_EN);
#endif
#endif

    for (uint8_t ch_num = 0; ch_num < adcConfig.num_of_channel_enable; ch_num++) {
      RSI_ADC_ChannelConfig(AUX_ADC_DAC_COMP,
                            adc_channel,
                            adcChConfig.pos_inp_sel[adc_channel],
                            adcChConfig.neg_inp_sel[adc_channel],
                            adcChConfig.input_type[adc_channel]);

      // Configure the sampling rate of ADC
      RSI_ADC_ChannelSamplingRate(AUX_ADC_DAC_COMP,
                                  adc_channel,
                                  adcInterConfig.ch_offset_val[adc_channel],
                                  adcInterConfig.ch_sampling_factor[adc_channel]);

      // ADC channel enable
      RSI_ADC_ChnlEnable(AUX_ADC_DAC_COMP, adc_channel);

      ADC_PinMux(adcChConfig.pos_inp_sel[adc_channel],
                 adcChConfig.neg_inp_sel[adc_channel],
                 adcChConfig.input_type[adc_channel]);

#ifndef ADC_MULTICHANNEL_WITH_EXT_DMA
      RSI_ADC_InternalPerChnlDmaEnable(AUX_ADC_DAC_COMP, adc_channel);
      PingPongMemoryAdrConfig.ping_addr   = adcChConfig.chnl_ping_address[adc_channel];
      PingPongMemoryAdrConfig.pong_addr   = adcChConfig.chnl_pong_address[adc_channel];
      PingPongMemoryAdrConfig.ping_length = adcChConfig.num_of_samples[adc_channel];
      PingPongMemoryAdrConfig.pong_length = adcChConfig.num_of_samples[adc_channel];
      PingPongMemoryAdrConfig.ping_enable = ADC_PING_EN;
      PingPongMemoryAdrConfig.pong_enable = ADC_PONG_EN;
      RSI_ADC_PingPongMemoryAdrConfig(AUX_ADC_DAC_COMP, adc_channel, PingPongMemoryAdrConfig);
      RSI_ADC_PingpongEnable(AUX_ADC_DAC_COMP, adc_channel);
#ifdef CHIP_9118
      RSI_ADC_ChnlIntrUnMask(AUX_ADC_DAC_COMP, ch_num);
#endif
#if defined(SLI_SI917)
      RSI_ADC_ChnlIntrUnMask(AUX_ADC_DAC_COMP, adc_channel, adcConfig.operation_mode);
#endif
#endif
      adcInterConfig.achived_sampling_rate[adc_channel] =
        ((adc_commn_config.adc_clk_src / adc_commn_config.total_clk)
         / adcInterConfig.ch_sampling_factor[adc_channel]); // Need to review

      adcInterConfig.num_of_samples[adc_channel] = adcChConfig.num_of_samples[adc_channel];

      adcInterConfig.rx_buf[adc_channel] = adcChConfig.rx_buf[adc_channel];

      adcInterConfig.input_type[adc_channel] = adcChConfig.input_type[adc_channel];
    }

    adc_dma_callback.transfer_complete_cb = ADC_DMA_Transfer_Complete;
    adc_dma_callback.error_cb             = ADC_DMA_Error_Callback;
    //Allocate DMA channel for Tx
    status = sl_si91x_dma_allocate_channel(DMA_INSTANCE, &dma_channel, channel_priority);
    if (status && (status != SL_STATUS_DMA_CHANNEL_ALLOCATED)) {
      return ARM_DRIVER_ERROR;
    }
    //Register transfer complete and error callback
    if (sl_si91x_dma_register_callbacks(DMA_INSTANCE, dma_channel, &adc_dma_callback)) {
      return ARM_DRIVER_ERROR;
    }
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        uint32_t ADC_GetSamplingRate(uint8_t ch_num)
 * @brief     This API get the channel sampling rate value which is configured to ADC.
 * @param[in] ch_num       : ADC Channel number (0 to 15).
 * @return    rsi_error_t       :Returns the channel sampling rate
 */
uint32_t ADC_GetSamplingRate(uint8_t ch_num)
{
  return (adcInterConfig.achived_sampling_rate[ch_num] =
            ((adc_commn_config.adc_clk_src / adc_commn_config.total_clk) / adcInterConfig.ch_sampling_factor[ch_num]));
}

/// @cond
#ifdef CHIP_9118

/*==============================================*/
/**
 * @fn        rsi_error_t ADC_Start(void)
 * @brief     This API used for start ADC operation.
 * @return    rsi_error_t      : Execution status,This function return 'RSI_OK' on
 *                        successful execution.  
 */
rsi_error_t ADC_Start(void)
#endif
/// @endcond
#if defined(SLI_SI917)
  /*==============================================*/
  /**
 * @fn        rsi_error_t ADC_Start(adc_config_t adcConfig)
 * @brief     This API used for start ADC operation.
 * @param[in]  adcConfig    : ADC operation configuration structure variable.
 * @return    rsi_error_t      : Execution status,This function return 'RSI_OK' on
 *                        successful execution.  
 */
  rsi_error_t ADC_Start(adc_config_t adcConfig)
#endif
{
  if (adc_commn_config.num_of_channel_enable > 1) {
#ifdef ADC_MULTICHANNEL_WITH_EXT_DMA
    int16_t dummy_read;
#ifdef CHIP_9118
    RSI_ADC_Start(AUX_ADC_DAC_COMP);
#endif
#if defined(SLI_SI917)
    RSI_ADC_Start(AUX_ADC_DAC_COMP, adcConfig.operation_mode);
#endif
    // Read the dummy ADC output
    while (AUX_ADC_DAC_COMP->FIFO_STATUS_REG_b.ADC_FIFO_EMPTY) {
    };
    dummy_read = RSI_ADC_ReadDataStatic(AUX_ADC_DAC_COMP, 0, 0);
    (void)dummy_read;
    UDMA_ADC_Start();
    return RSI_OK;
#endif
  }
  // if not multi channel or single channel with internal DMA
  NVIC_EnableIRQ(ADC_IRQn);
#ifdef CHIP_9118
  RSI_ADC_Start(AUX_ADC_DAC_COMP);
#endif
#if defined(SLI_SI917)
  RSI_ADC_Start(AUX_ADC_DAC_COMP, adcConfig.operation_mode);
#endif

  return RSI_OK;
}

/// @cond
#ifdef CHIP_9118

/*==============================================*/
/**
 * @fn     rsi_error_t ADC_Deinit(void)
 * @brief  This API used for De-initialize the ADC.
 * @return rsi_error_t      : Execution status,This function return 'RSI_OK' on
 *                        successful execution.  
 */
rsi_error_t ADC_Deinit(void)
#endif
/// @endcond
#if defined(SLI_SI917)

  /*==============================================*/
  /**
 * @fn         rsi_error_t ADC_Deinit(adc_config_t adcConfig)
 * @brief      This API used for De-initialize the ADC.
 * @param[in]  adcConfig    : ADC operation configuration structure variable.
 * @return     rsi_error_t      : Execution status,This function return 'RSI_OK' on
 *                            successful execution.  
 */
  rsi_error_t ADC_Deinit(adc_config_t adcConfig)
#endif
{
  (void)adcConfig;
#ifdef CHIP_9118
  // Stop ADC operation
  RSI_ADC_Stop(AUX_ADC_DAC_COMP);
#endif
  // Disable IRQ
  NVIC_DisableIRQ(ADC_IRQn);

  if (sl_si91x_dma_unregister_callbacks(DMA_INSTANCE, DMA_CHANNEL, SL_DMA_TRANSFER_DONE_CB | SL_DMA_ERROR_CB)) {
    return ARM_DRIVER_ERROR;
  }
  // Power down the ADC block
  RSI_ADC_PowerControl(ADC_POWER_OFF);

  return RSI_OK;
}

/// @cond
#ifdef CHIP_9118

/*==============================================*/
/**
 * @fn           rsi_error_t ADC_Stop(void)
 * @brief        This API is used to stop the ADC
 * @return       execution status 
 */
rsi_error_t ADC_Stop(void)
{
  RSI_ADC_Stop(AUX_ADC_DAC_COMP);
  return RSI_OK;
}
#endif
/// @endcond
#if defined(SLI_SI917)

/*==============================================*/
/**
 * @fn           rsi_error_t ADC_Stop(adc_config_t adcConfig)
 * @brief        This API is used to stop the ADC
 * @param[in]    adcConfig    : ADC operation configuration structure variable.
 * @return       execution status 
 */
rsi_error_t ADC_Stop(adc_config_t adcConfig)
{
  rsi_error_t error_status;
  error_status = RSI_ADC_Stop(AUX_ADC_DAC_COMP, adcConfig.operation_mode);
  return error_status;
}
#endif

/*==============================================*/
/**
 * @fn     rsi_error_t ADC_PingPongReconfig(uint8_t event, uint8_t channel_num)
 * @brief  This API used for reconfigure the ADC ping or pong descriptor address.
 *         Use this APIs to reconfigure the ping or pong descriptor in ADC multi-channel case.   
 * @param[in]  event        : EXTERNAL_DMA_RECONFIG or INTERNAL_DMA          
 * @param[in]  channel_num  : ADC Channel Number        
 * @return rsi_error_t          : Execution status,This function return 'RSI_OK' on
 *                            successful execution.  
 */
rsi_error_t ADC_PingPongReconfig(uint8_t event, uint8_t channel_num)
{
  if (pong_enable_sel[channel_num]) {
    if (event == EXTERNAL_DMA_RECONFIG) {

#ifdef ADC_MULTICHANNEL_WITH_EXT_DMA
      /* Configure the UDMA pong descriptor */
      UDMA_ADC_Ping_Write(0);

      ADC_ChannelsDataSort(1);
#endif
    } else {

#ifdef CHIP_9118
#ifndef DONOT_READ_DATA_FROM_MEM
#ifdef GAIN_OFFSET_CAL_EN
      RSI_ADC_ReadData(adcInterConfig.rx_buf[0],
                       pong_enable_sel[channel_num],
                       channel_num,
                       1,
                       adcInterConfig.input_type[0]);
#else
      RSI_ADC_ReadData(adcInterConfig.rx_buf[channel_num],
                       pong_enable_sel[channel_num],
                       channel_num,
                       0,
                       adcInterConfig.input_type[0]);
#endif
#endif
#endif
      // Reconfigure the pong address
      RSI_ADC_PingPongReInit(AUX_ADC_DAC_COMP, channel_num, 0, 1);
    }
  } else {
    if (event == EXTERNAL_DMA_RECONFIG) {

#ifdef ADC_MULTICHANNEL_WITH_EXT_DMA
      /* Configure the UDMA pong descriptor */
      UDMA_ADC_Pong_Write(0);

      ADC_ChannelsDataSort(0);
#endif
    } else {

#ifdef CHIP_9118
#ifndef DONOT_READ_DATA_FROM_MEM
#ifdef GAIN_OFFSET_CAL_EN
      RSI_ADC_ReadData(adcInterConfig.rx_buf[channel_num],
                       pong_enable_sel[channel_num],
                       channel_num,
                       1,
                       adcInterConfig.input_type[0]);
#else
      RSI_ADC_ReadData(adcInterConfig.rx_buf[channel_num],
                       pong_enable_sel[channel_num],
                       channel_num,
                       0,
                       adcInterConfig.input_type[0]);
#endif
#endif
#endif
      // Reconfigure the pong address
      RSI_ADC_PingPongReInit(AUX_ADC_DAC_COMP, channel_num, 1, 0);
    }
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn     void ADC_IRQ_Handler(void)
 * @brief  ADC IRQ handler to clear the ADC interrupt re configure the ping and pong address of ADC. 
 * @return None
 */
void ADC_IRQ_Handler(void)
{
#ifdef CHIP_9118
  // Clear channel0 ADC interrupt
  RSI_ADC_ChnlIntrClr(AUX_ADC_DAC_COMP, ADC_CHNL0_INTR);
  // Reconfigure the ADC ping or pong memory address
  ADC_PingPongReconfig(INTERNAL_DMA, 0);
  // Call ADC data acquisition complete event
  adc_commn_config.call_back_event(ADC_CHNL0_INTR, INTERNAL_DMA);
#endif
#if defined(SLI_SI917)
  RSI_ADC_InterruptHandler(AUX_ADC_DAC_COMP);
#endif
}

/*==============================================*/
/**
 * @fn         rsi_error_t ADC_ChannelsDataSort(uint8_t data_select)
 * @brief      This API used for sort channels data.  
 * @param[in]  data_select : This parameter define ADC data is available in ping location or pong location.
 *                           - data_select : 0 , Data available in pong location.
 *                           - data_select : 1 , Data available in ping location.
 * @return     rsi_error_t     : Execution status,This function return 'RSI_OK' on
 *                         successful execution.  
 */
rsi_error_t ADC_ChannelsDataSort(uint8_t data_select)
{
  (void)data_select;
#ifdef ADC_MULTICHANNEL_WITH_EXT_DMA
  uint16_t sample_incr = 0, ch_num = 0;
  int16_t channel_data;
  uint32_t addr_add;
  addr_add = (data_select * UDMA_PING_PONG_LENGTH);
  for (sample_incr = 0; sample_incr < UDMA_PING_PONG_LENGTH; sample_incr++) {
    // Read the Sample by sample from receive buffer
    channel_data = ((adc_output_buf + (addr_add))[sample_incr]);

#ifdef MAX_SAMPLING_RATE

#else
    // Find out channel number
    ch_num                                                                     = (((channel_data)) >> 12);
#endif

#ifdef GAIN_OFFSET_CAL_EN
    channel_data = (channel_data & 0x0fff);
    if (channel_data & BIT(11)) {
      channel_data = channel_data & (ADC_MASK_VALUE);
    } else {
      channel_data = channel_data | BIT(11);
    }

    if (adcInterConfig.input_type[ch_num]) {
      channel_data = ((channel_data - adc_commn_config.adc_diff_offset) * adc_commn_config.adc_diff_gain);
    } else {
      channel_data = ((channel_data - adc_commn_config.adc_sing_offset) * adc_commn_config.adc_sing_gain);
    }
    if (channel_data > 4095) {
      channel_data = 4095;
    }
    if (channel_data <= 0) {
      channel_data = 0;
    } else if (channel_data >= 2048) {
      channel_data = channel_data - 2048;
    } else {
      channel_data = channel_data + 2048;
    }
#ifdef CHIP_9118
    *(volatile int16_t *)(((adcInterConfig.rx_buf[ch_num]) + ch_data[ch_num])) = channel_data;
#endif
#if defined(SLI_SI917)
    if (ch_num != 0) {
      *(volatile int16_t *)(((adcInterConfig.rx_buf[ch_num - 1]) + ch_data[ch_num - 1])) = (channel_data & 0x0fff);
    }
#endif
#else
#ifdef CHIP_9118
    *(volatile int16_t *)(((adcInterConfig.rx_buf[ch_num]) + ch_data[ch_num])) = (channel_data & 0x0fff);
#endif
#if defined(SLI_SI917)
    if (ch_num != 0) {
      *(volatile int16_t *)(((adcInterConfig.rx_buf[ch_num - 1]) + ch_data[ch_num - 1])) = (channel_data & 0x0fff);
    }
#endif
#endif
#ifdef CHIP_9118
    ch_data[ch_num]++;

    if (ch_data[ch_num]
        >= adcInterConfig.num_of_samples[ch_num]) // Need to implement, comparision may be in configured value
    {
      adc_commn_config.call_back_event(ch_num, MULTI_CHANNEL_DATA_OCCOURE);
      ch_data[ch_num] = 0;
    }
#endif
#if defined(SLI_SI917)
    ch_data[ch_num - 1]++;

    if (ch_data[ch_num - 1]
        >= adcInterConfig.num_of_samples[ch_num - 1]) // Need to implement, comparision may be in configured value
    {
      adc_commn_config.call_back_event(ch_num - 1, MULTI_CHANNEL_DATA_OCCOURE);
      ch_data[ch_num - 1] = 0;
    }
#endif
  }
#endif
  return RSI_OK;
}
/**
 * @fn     uint8_t is_offset_used_by_channel(uint16_t offset, uint16_t max_factor, uint8_t ch_num)
 * @brief  Checks if a specific offset value is already used by any channel
 * 
 * @param  offset     The offset value to check
 * @param  max_factor Maximum factor to consider for checking
 * @param  ch_num     Number of channels to check against
 * @return 1 if offset is used by any channel, 0 otherwise
 */
STATIC INLINE uint8_t is_offset_used_by_channel(uint16_t offset, uint16_t max_factor, uint8_t ch_num)
{
  uint8_t used = 0;
  for (uint16_t n = 0; n < ch_num; n++) {
    for (uint16_t m = adcInterConfig.ch_offset_val[n]; m < max_factor; m += adcInterConfig.ch_sampling_factor[n]) {
      if (offset == m) {
        used = 1;
        break;
      }
    }
    if (used) {
      break;
    }
  }
  return used;
}
/*==============================================*/
/**
 * @fn     rsi_error_t cal_adc_channel_offset(void)
 * @brief  This API used for calculating adc channels offset value.
 *         this value define on which ADC clock channels will be sampled. 
 * @return rsi_error_t     : Execution status,This function return 'RSI_OK' on
 *                        successful execution.  
 */
rsi_error_t cal_adc_channel_offset(void)
{
  uint8_t ch_num;
  uint16_t max_chan_swallo_fac = 0;

  // Find out max number of channel frequency value
  for (ch_num = 0; ch_num < adc_commn_config.num_of_channel_enable; ch_num++) {
    if (max_chan_swallo_fac < adcInterConfig.ch_sampling_factor[adc_channel]) {
      max_chan_swallo_fac = adcInterConfig.ch_sampling_factor[adc_channel];
    }
  }
  // Find out channels offset based on channels frequency value
  adcInterConfig.ch_offset_val[adc_channel] = 0;
  for (ch_num = 1; ch_num < adc_commn_config.num_of_channel_enable; ch_num++) {
    for (uint16_t i = 0; i < max_chan_swallo_fac; i++) {
      // Check if offset 'i' is already used by any previously configured channel
      if (!is_offset_used_by_channel(i, max_chan_swallo_fac, ch_num)) {
        adcInterConfig.ch_offset_val[ch_num] = i;
        break;
      }
    }
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn     uint8_t check_power_two(uint16_t num)
 * @brief  This API used for find out given number is power of two. 
 * @param[in]  num      : Given number to find out number is power of two or not power of two. 
 * @return Return the status of given number is power of two or not power of two.
 *        - Return 1 : Number is power of two.
 *        - Return 0 : Number is not power of two. 
 */
uint8_t check_power_two(uint16_t num)
{
  while (num != 1) {
    if (num % 2) {
      return 0;
    }
    num /= 2;
  }
  return 1;
}

/*==============================================*/
/**
 * @fn     uint32_t roundupto_pwr2(uint16_t num)
 * @brief  This API used for round-off the number with nearest power of two number. 
 * @param[in]  num      : Given number to round-off with nearest power of two number. 
 * @return     Return nearest power of two number. 
 */
uint32_t roundupto_pwr2(uint16_t num)
{
  int power = 1;
  int diff  = 0;
  while (power < num) {
    power *= 2;
  }
  diff = num - (power / 2);
  if (diff < power / 4) {
    return (uint32_t)power / 2;
  } else {
    return (uint32_t)power;
  }
}

/*==============================================*/
/**
 * @fn     uint32_t max_sample_rate_achive(float min_sampling_time)
 * @brief  This API find-out maximum ADC sampling rate can achieve from required channels configuration. 
 * @param[in]  min_sampling_time : Minimum sampling time from given configuration
 * @return ADC effective sampling rate. 
 */
uint32_t max_sample_rate_achive(float min_sampling_time)
{
  uint32_t f_max1 = 0;
  uint32_t f_max2 = 5000000;
  uint32_t f_return;

  float t = 0;

  t = (min_sampling_time + (1 / (float)10000000));

  f_max1 = (uint32_t)(1 / t);

  f_return = f_max1 < f_max2 ? f_max1 : f_max2;

  return f_return;
}

/*===================================================*/
/**
 * @fn          float get_delay_based_on_gain(float gain_value)
 * @brief       select delay based on gain value
 * @param[in]   gain_value:  opamp gain value
 * @return      delay_return: delay value    
 */
STATIC INLINE float get_delay_based_on_gain(float gain_value)
{
  float delay_return = 0;

  if ((gain_value >= (float)2) && (gain_value < (float)2.5)) {
    delay_return = GAIN2_DELAY_HP;
  } else if ((gain_value >= (float)2.5) && (gain_value < (float)3)) {
    delay_return = GAIN2_5_DELAY_HP;
  } else if ((gain_value >= (float)3) && (gain_value < (float)4)) {
    delay_return = GAIN3_DELAY_HP;
  } else if ((gain_value >= (float)4) && (gain_value < (float)5)) {
    delay_return = GAIN4_DELAY_HP;
  } else if ((gain_value >= (float)5) && (gain_value < (float)6)) {
    delay_return = GAIN5_DELAY_HP;
  } else if ((gain_value >= (float)7) && (gain_value < (float)8)) {
    delay_return = GAIN7_DELAY_HP;
  } else if ((gain_value >= (float)8) && (gain_value < (float)9)) {
    delay_return = GAIN8_DELAY_HP;
  } else if ((gain_value >= (float)9) && (gain_value < (float)10)) {
    delay_return = GAIN9_DELAY_HP;
  } else if ((gain_value >= (float)12) && (gain_value < (float)14)) {
    delay_return = GAIN12_5_DELAY_HP;
  } else if ((gain_value >= (float)17) && (gain_value < (float)18)) {
    delay_return = GAIN17_DELAY_HP;
  } else if ((gain_value >= (float)25) && (gain_value <= (float)26)) {
    delay_return = GAIN25_DELAY_HP;
  } else if ((gain_value >= (float)27) && (gain_value <= (float)51)) {
    delay_return = GAIN51_DELAY_HP;
  }

  return delay_return;
}

/*==============================================*/
/**
 * @fn     float get_min_sampling_time(uint8_t number_of_channel, adc_ch_config_t adcChConfig)
 * @brief  This API gives required minimum sampling time(ON time of ADC cycles) form given channels configuration.
 * @param[in]  number_of_channel : Number of channel enable for ADC operation.
 * @param[in]  adcChConfig : ADC channels configuration structure variable.
 * @return Required minimum sampling time(ON time of ADC cycles)  
 */
float get_min_sampling_time(uint8_t number_of_channel, adc_ch_config_t adcChConfig)
{
  uint8_t opamp_cnt  = 0;
  uint8_t dac_cnt    = 0;
  float gain_value   = 0;
  float delay_return = 0;
  for (uint8_t chan_num_incr = 0; chan_num_incr < number_of_channel; chan_num_incr++) {
    if (((adcChConfig.pos_inp_sel[chan_num_incr] == POS20) || (adcChConfig.pos_inp_sel[chan_num_incr] == POS21)
         || (adcChConfig.pos_inp_sel[chan_num_incr] == POS22) || (adcChConfig.neg_inp_sel[chan_num_incr] == POS10)
         || (adcChConfig.neg_inp_sel[chan_num_incr] == POS11) || (adcChConfig.neg_inp_sel[chan_num_incr] == POS12))
        && (gain_value < adcChConfig.opamp_gain[chan_num_incr])) {
      gain_value = adcChConfig.opamp_gain[chan_num_incr];
      opamp_cnt++;
    }
    if (((adcChConfig.pos_inp_sel[chan_num_incr]) == POS24) || ((adcChConfig.neg_inp_sel[chan_num_incr]) == POS13)) {
      dac_cnt++;
    }
  }
  if (opamp_cnt) {
    delay_return = get_delay_based_on_gain(gain_value);
  }
  if (dac_cnt && (delay_return < (float)(3.5 / 1000000))) {
    delay_return = (float)(3.5 / 1000000);
  }

  if (delay_return < ((float)100 / (float)1000000000)) {
    delay_return = ((float)100 / (float)1000000000);
  }

  return delay_return;
}

/**
 * @brief Configure ADC pin multiplexing for single-ended or differential mode.
 *
 * Logic:
 * - Validate parameters for differential mode (negative pin must be POS0–POS9).
 * - Configure positive pin if valid (POS0–POS19).
 * - Configure negative pin only when input_type indicates differential mode.
 *
 * @param pos_input_pinsel  Positive input pin selection
 * @param neg_input_pinsel  Negative input pin selection (used in differential mode)
 * @param input_type        Mode indicator (0 = single-ended, 1 = differential)
 *
 * @return RSI_OK on success, ERROR_ADC_INVALID_ARG on invalid parameters
 */
rsi_error_t ADC_PinMux(uint8_t pos_input_pinsel, uint8_t neg_input_pinsel, uint8_t input_type)
{
  // Validate parameters
  // For differential mode, negative input pin must be valid (POS0–POS9)
  if (input_type && (neg_input_pinsel > POS9)) {
    return ERROR_ADC_INVALID_ARG;
  }

  // Configure positive input pins (valid range: POS0–POS19)
  // Note: POS20+ may be valid for internal signals (opamp/DAC) that don't need GPIO pin mux
  if (pos_input_pinsel <= POS19) {
    ADC_ConfigurePositiveInputPin(pos_input_pinsel);
  }

  // Configure negative input pins for differential mode (valid range: POS0–POS9)
  if (input_type) {
    ADC_ConfigureNegativeInputPin(neg_input_pinsel);
  }

  return RSI_OK;
}

static void ADC_ConfigurePositiveInputPin(uint8_t pos_input_pinsel)
{
  switch (pos_input_pinsel) {
    case POS0:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO0);
      break;
    case POS1:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO2);
      break;
    case POS2:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO4);
      break;
    case POS3:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO6);
      break;
    case POS4:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO8);
      break;
    case POS5:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO10);
      break;
    case POS6:
      *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
      CONFIGURE_ADC_TOP_GPIO(ADC_TOPGPIO25);
      break;
    case POS7:
      *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
      CONFIGURE_ADC_TOP_GPIO(ADC_TOPGPIO27);
      break;
    case POS8:
      *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
      CONFIGURE_ADC_TOP_GPIO(ADC_TOPGPIO29);
      break;
#ifdef CHIP_9118
    case POS9:
      CONFIGURE_ADC_TOP_GPIO(ADC_TOPGPIO23);
      break;
#endif
    case POS10:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO1);
      break;
    case POS11:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO3);
      break;
    case POS12:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO5);
      break;
    case POS13:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO11);
      break;
    case POS14:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO9);
      break;
    case POS15:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO7);
      break;
    case POS16:
      *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
      CONFIGURE_ADC_TOP_GPIO(ADC_TOPGPIO26);
      break;
    case POS17:
      *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
      CONFIGURE_ADC_TOP_GPIO(ADC_TOPGPIO28);
      break;
    case POS18:
      *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
      CONFIGURE_ADC_TOP_GPIO(ADC_TOPGPIO30);
      break;
#ifdef CHIP_9118
    case POS19:
      CONFIGURE_ADC_TOP_GPIO(ADC_TOPGPIO24);
      break;
#endif
    default:
      break;
  }
}

static void ADC_ConfigureNegativeInputPin(uint8_t neg_input_pinsel)
{
  // Configure negative input pins for differential mode
  switch (neg_input_pinsel) {
    case POS0:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO1);
      break;
    case POS1:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO3);
      break;
    case POS2:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO5);
      break;
    case POS3:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO11);
      break;
    case POS4:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO9);
      break;
    case POS5:
      CONFIGURE_ADC_ULP_GPIO(ADCGPIO7);
      break;
    case POS6:
      *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
      CONFIGURE_ADC_TOP_GPIO(ADC_TOPGPIO26);
      break;
    case POS7:
      *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
      CONFIGURE_ADC_TOP_GPIO(ADC_TOPGPIO28);
      break;
    case POS8:
      *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
      CONFIGURE_ADC_TOP_GPIO(ADC_TOPGPIO30);
      break;
    case POS9:
      CONFIGURE_ADC_TOP_GPIO(ADC_TOPGPIO24);
      break;
    default:
      break;
  }
}
/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_PingPongMemoryAdrConfig(AUX_ADC_DAC_COMP_Type *pstcADC, 
 *                                                       uint32_t channel ,uint32_t ping_addr,
 *                                   uint32_t pong_addr,uint16_t ping_length,
 *                             uint16_t pong_length,uint8_t ping_enable,
 *                             uint8_t pong_enable )
 * @brief        This API is used to configure Ping and pong memory location along with 
 *               length of ping memory and pong memory.
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel     : ADC channel to be configured as 0,1,2 ...15
 * @param[in]    ping_addr   : ping address configuration.
 * @param[in]    pong_addr   : pong address configuration.(e.g pong address = ping address +
 *                                                                             ping length ) 
 * @param[in]    ping_length : ping length 
 * @param[in]    pong_length : pong length 
 * @param[in]    ping_enable : ping enable 
 * @param[in]    pong_enable : pong enable  
 * @return       execution status 
 */
rsi_error_t RSI_ADC_PingPongMemoryAdrConfig(AUX_ADC_DAC_COMP_Type *pstcADC,
                                            uint32_t channel,
                                            adc_ping_pong_memory_adr_config_t PingPongMemoryAdrConfig)
{
  uint32_t ping_addr   = PingPongMemoryAdrConfig.ping_addr;
  uint32_t pong_addr   = PingPongMemoryAdrConfig.pong_addr;
  uint16_t ping_length = PingPongMemoryAdrConfig.ping_length;
  uint16_t pong_length = PingPongMemoryAdrConfig.pong_length;
  uint8_t ping_enable  = PingPongMemoryAdrConfig.ping_enable;
  uint8_t pong_enable  = PingPongMemoryAdrConfig.pong_enable;

  if ((channel > MAX_CHNL_NO) || (ping_length > PING_LEN_MAX) || (pong_length > PONG_LEN_MAX)
      || (ping_addr > ((uint32_t)PING_ADDR_MAX)) || (pong_addr > ((uint32_t)PONG_ADDR_MAX))) {
    return INVALID_PARAMETERS;
  }
  adcInterConfig.ping_addr[channel]   = ping_addr;
  adcInterConfig.pong_addr[channel]   = pong_addr;
  adcInterConfig.ping_length[channel] = ping_length;
  adcInterConfig.pong_length[channel] = pong_length;

  if (ping_addr == pong_addr) {
    return ERROR_PING_PONG_ADDR_MATCH;
  } else {
    if (ping_enable) {
      ping_addr                             = ping_addr + (ping_length * 2);
      pstcADC->ADC_INT_MEM_1_b.PROG_WR_DATA = ping_addr;
      adcInterConfig.ping_mem1[channel]     = ping_addr;

      pstcADC->ADC_INT_MEM_2            = (1 << POS15) | ((2 * channel) << POS10) | (ping_length << POS0);
      adcInterConfig.ping_mem2[channel] = pstcADC->ADC_INT_MEM_2;

      pstcADC->ADC_INT_MEM_2            = (0 << POS15) | ((2 * channel) << POS10) | (ping_length << POS0);
      adcInterConfig.ping_mem3[channel] = pstcADC->ADC_INT_MEM_2;
    }
    if (pong_enable) {
      pong_addr                             = pong_addr + (uint32_t)(ping_length + (pong_length * 2));
      pstcADC->ADC_INT_MEM_1_b.PROG_WR_DATA = pong_addr;
      adcInterConfig.pong_mem1[channel]     = pong_addr;

      pstcADC->ADC_INT_MEM_2            = (1 << POS15) | ((2 * channel + 1) << POS10) | (pong_length << POS0);
      adcInterConfig.pong_mem2[channel] = pstcADC->ADC_INT_MEM_2;

      pstcADC->ADC_INT_MEM_2            = (0 << POS15) | ((2 * channel + 1) << POS10) | (pong_length << POS0);
      adcInterConfig.pong_mem3[channel] = pstcADC->ADC_INT_MEM_2;
    }
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           uint32_t RSI_ADC_PingPongReInit(AUX_ADC_DAC_COMP_Type *pstcADC, uint8_t channel, uint8_t ping_enable,uint8_t pong_enable )
 * @brief        This API is used to reconfigure Ping-pong memory location along with the length of the samples
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel     : ADC channel to be configured as 0,1,2 ...15
 * @param[in]    ping_enable : ping enable 
 * @param[in]    pong_enable : pong enable
 * @return       execution status 
 *
 */
rsi_error_t RSI_ADC_PingPongReInit(AUX_ADC_DAC_COMP_Type *pstcADC,
                                   uint8_t channel,
                                   uint8_t ping_enable,
                                   uint8_t pong_enable)
{
  if (channel > MAX_CHNL_NO) {
    return INVALID_PARAMETERS;
  }
  if (ping_enable) {
    pstcADC->ADC_INT_MEM_1_b.PROG_WR_DATA = adcInterConfig.ping_mem1[channel];
    pstcADC->ADC_INT_MEM_2                = adcInterConfig.ping_mem2[channel];
    pstcADC->ADC_INT_MEM_2                = adcInterConfig.ping_mem3[channel];
  }
  if (pong_enable) {
    pstcADC->ADC_INT_MEM_1_b.PROG_WR_DATA = adcInterConfig.pong_mem1[channel];
    pstcADC->ADC_INT_MEM_2                = adcInterConfig.pong_mem2[channel];
    pstcADC->ADC_INT_MEM_2                = adcInterConfig.pong_mem3[channel];
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           void RSI_ADC_Calibration()
 * @brief        This API used to do ADC calibration.
 * @param[in]    None
 * @return       None 
 */
void RSI_ADC_Calibration(void)
{
  uint32_t impuDummyRead = 0;

  AUX_ADC_DAC_COMP->AUXADC_CTRL_1_b.ADC_ENABLE |= 1U;
  AUX_ADC_DAC_COMP->AUXADC_CONFIG_2 |= BIT(10);

  impuDummyRead = ULP_SPI_MEM_MAP(0);
  (void)impuDummyRead;

  if (calib_done == 0) {
    ULP_SPI_MEM_MAP(AUXADCREG0) |= BIT(13) | BIT(12) | BIT(11) | BIT(8) | BIT(7) | BIT(6);
    while (GSPI_CTRL_REG1 & SPI_ACTIVE)
      ;
    /*Wait for 1*/
    while (!(ULP_SPI_MEM_MAP(SPAREREG2) & BIT(0)))
      ;
    /*wait for 0*/
    while (ULP_SPI_MEM_MAP(SPAREREG2) & BIT(0))
      ;
    /*150 clocks of 1MHz wait*/
    auxadcCalibValue = ULP_SPI_MEM_MAP(AUXADCREG2);
    auxadcCalibValueLoad |= BIT(0) | BIT(7);
    auxadcCalibValueLoad |= (auxadcCalibValue & 0x1F) << 2;
    auxadcCalibValueLoad |= (((auxadcCalibValue >> POS5) & 0x1F) << POS13);
    auxadcCalibValueLoad |= (((auxadcCalibValue >> POS10) & 0x1F) << POS8);
    calib_done = 1;
  }

  ULP_SPI_MEM_MAP(AUXADCREG1) = auxadcCalibValueLoad;
  while (GSPI_CTRL_REG1 & SPI_ACTIVE)
    ;
  AUX_ADC_DAC_COMP->AUXADC_CTRL_1_b.ADC_ENABLE = 0;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_PingpongEnable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
 * @brief        This API is used to enable ping pong for corresponding ADC channels 
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel  : ADC channel to be configured as 0,1,2 ...15
 * @return       execution status 
 *
 */
rsi_error_t RSI_ADC_PingpongEnable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
{
  if (channel > MAX_CHNL_NO) {
    return INVALID_PARAMETERS;
  }
  pstcADC->ADC_SEQ_CTRL_b.ADC_SEQ_CTRL_PING_PONG |= (uint16_t)(1 << channel);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_PingpongDisable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
 * @brief        This API is used to disable the ping pong for corresponding ADC channels 
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel  :ADC channel to be configured as 0,1,2 ...15
 * @return       execution status 
 *
 */
rsi_error_t RSI_ADC_PingpongDisable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
{
  if (channel > MAX_CHNL_NO) {
    return INVALID_PARAMETERS;
  }
  pstcADC->ADC_SEQ_CTRL_b.ADC_SEQ_CTRL_PING_PONG &= (uint16_t)(~(1 << channel));
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_InternalPerChnlDmaEnable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
 * @brief        This API is used to enable internal DMA for corresponding ADC channels 
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel  :ADC channel to be configured as 0,1,2 ...15
 * @return       execution status 
 */
rsi_error_t RSI_ADC_InternalPerChnlDmaEnable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
{
  if (channel > MAX_CHNL_NO) {
    return INVALID_PARAMETERS;
  }
  pstcADC->ADC_SEQ_CTRL_b.ADC_SEQ_CTRL_DMA_MODE |= (uint16_t)(1 << channel);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_InternalPerChnlDmaDisable(AUX_ADC_DAC_COMP_Type *pstcADC,
 *                                                         uint32_t channel)
 * @brief        This API is used to disable the internal dma channel for corresponding ADC channels 
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel  :ADC channel to be configured as 0,1,2 ...15
 * @return       execution status 
 */
rsi_error_t RSI_ADC_InternalPerChnlDmaDisable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
{
  if (channel > MAX_CHNL_NO) {
    return INVALID_PARAMETERS;
  }
  pstcADC->ADC_SEQ_CTRL_b.ADC_SEQ_CTRL_DMA_MODE &= (uint16_t)(~(1 << channel));
  return RSI_OK;
}

/// @cond
#ifdef CHIP_9118

/*==============================================*/
/**
 * @fn           void RSI_ADC_Config(AUX_ADC_DAC_COMP_Type *pstcADC,uint8_t multi_channel_en,
 *                                   uint8_t static_fifo_mode, uint8_t fifo_threshold )
 * @brief        This API is used to configure the ADC channels 
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    multi_channel_en : Multi channel enable parameter.
 * @param[in]    static_fifo_mode : Static mode  enable parameter,
 *                                  - static_fifo_mode =1 static mode
 *                                  - static_fifo_mode =0 fifo mode
 * @param[in]    fifo_threshold   : fifo threshold value for ADC operation.
 * @param[in]    internal_dma_en  : Internal DMA enable parameter.
 * @return       execution status.
 */
rsi_error_t RSI_ADC_Config(AUX_ADC_DAC_COMP_Type *pstcADC,
                           uint8_t multi_channel_en,
                           uint8_t static_fifo_mode,
                           uint8_t fifo_threshold,
                           uint8_t internal_dma_en)
#endif
/// @endcond
#if defined(SLI_SI917)
  /*==============================================*/
  /**
 * @fn           rsi_error_t RSI_ADC_Config(AUX_ADC_DAC_COMP_Type *pstcADC, uint8_t multi_channel_en, uint8_t static_fifo_mode, uint8_t a_empty_threshold, uint8_t a_full_threshold, uint8_t internal_dma_en)
 * @brief        This API is used to configure the ADC channels 
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    multi_channel_en : Multi channel enable parameter.
 * @param[in]    static_fifo_mode : Static mode  enable parameter,
 *                                  - static_fifo_mode =1 static mode
 *                                  - static_fifo_mode =0 fifo mode
 * @param[in]    a_empty_threshold : aempty fifo threshold value.
 * @param[in]    a_full_threshold  : afull fifo threshold value. 
 * @param[in]    internal_dma_en   : Internal DMA enable parameter.
 * @return       execution status.
 */
  rsi_error_t RSI_ADC_Config(AUX_ADC_DAC_COMP_Type *pstcADC,
                             uint8_t multi_channel_en,
                             uint8_t static_fifo_mode,
                             uint8_t a_empty_threshold,
                             uint8_t a_full_threshold,
                             uint8_t internal_dma_en)
#endif
{
  (void)static_fifo_mode;
  (void)a_empty_threshold;
  (void)a_full_threshold;
#ifdef CHIP_9118
  if (fifo_threshold > MAX_THRS_VAL) {
    return INVALID_PARAMETERS;
  }
#endif
  pstcADC->AUXADC_CTRL_1_b.ADC_NUM_PHASE            = 1;
  pstcADC->AUXADC_CTRL_1_b.ADC_MULTIPLE_CHAN_ACTIVE = (unsigned int)(multi_channel_en & 0x01);
#ifdef CHIP_9118
  pstcADC->AUXADC_CTRL_1_b.ADC_STATIC_MODE = static_fifo_mode;
#endif
#if defined(SLI_SI917)
  if ((static_fifo_mode == ADC_STATICMODE_ENABLE) || (static_fifo_mode == ADC_STATICMODE_THRESHOLD_EN)) {
    pstcADC->AUXADC_CTRL_1_b.ADC_STATIC_MODE = 1;
  } else {
    pstcADC->AUXADC_CTRL_1_b.ADC_STATIC_MODE = (unsigned int)(static_fifo_mode & 0x01);
  }
#endif

#ifdef CHIP_9118
  pstcADC->AUXADC_CTRL_1_b.ADC_FIFO_THRESHOLD = fifo_threshold;
#endif
#if defined(SLI_SI917)
  pstcADC->ADC_FIFO_THRESHOLD = (unsigned int)(a_empty_threshold | a_full_threshold << 4);
#endif
  pstcADC->AUXADC_CTRL_1_b.ADC_FIFO_FLUSH = 1;
  if (internal_dma_en == INTERNAL_DMA_EN) {
    pstcADC->INTERNAL_DMA_CH_ENABLE_b.INTERNAL_DMA_ENABLE = (unsigned int)(internal_dma_en & 0x01);
  } else {
    pstcADC->INTERNAL_DMA_CH_ENABLE_b.INTERNAL_DMA_ENABLE = 0;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_ChannelConfig(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel , uint8_t an_perif_adc_ip_sel,uint8_t an_perif_adc_in_sel, uint8_t an_perif_adc_diffmode )
 * @brief        This API is used to configure the ADC channels 
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel  :ADC channel to be configured as 0,1,2 ...15
 * @param[in]    an_perif_adc_ip_sel     : ADC positive input select in static mode.
 * @param[in]    an_perif_adc_in_sel     : ADC negative input select in static mode.
 * @param[in]    an_perif_adc_diffmode   : ADC differential mode selection in static mode.
 * @return       execution status 
 */
rsi_error_t RSI_ADC_ChannelConfig(AUX_ADC_DAC_COMP_Type *pstcADC,
                                  uint32_t channel,
                                  uint8_t an_perif_adc_ip_sel,
                                  uint8_t an_perif_adc_in_sel,
                                  uint8_t an_perif_adc_diffmode)
{
  if ((channel > MAX_CHNL_NO) || (an_perif_adc_ip_sel > MAX_POS_IN_SEL) || (an_perif_adc_in_sel > MAX_NEG_IN_SEL)
      || (an_perif_adc_diffmode > MAX_DIFF_MODE)) {
    return INVALID_PARAMETERS;
  }
  if (pstcADC->AUXADC_CTRL_1_b.ADC_MULTIPLE_CHAN_ACTIVE) {
    if (an_perif_adc_diffmode) {
      pstcADC->ADC_CH_BIT_MAP_CONFIG[channel].ADC_CH_BIT_MAP_CONFIG_0_b.CHANNEL_BITMAP |=
        (uint32_t)((an_perif_adc_diffmode << POS26) | (an_perif_adc_in_sel << POS22));
    }
    pstcADC->ADC_CH_BIT_MAP_CONFIG[channel].ADC_CH_BIT_MAP_CONFIG_0_b.CHANNEL_BITMAP |= an_perif_adc_ip_sel << POS17;
  } else {
    if (an_perif_adc_diffmode) {
#ifdef CHIP_9118
      pstcADC->AUXADC_CONFIG_1_b.AUXADC_DIFF_MODE = an_perif_adc_diffmode;
      pstcADC->AUXADC_CONFIG_1_b.AUXADC_INN_SEL   = an_perif_adc_in_sel;
#endif
#if defined(SLI_SI917)
      pstcADC->AUXADC_CONFIG_2_b.AUXADC_DIFF_MODE = (unsigned int)(an_perif_adc_diffmode & 0x01);
      pstcADC->AUXADC_CONFIG_2_b.AUXADC_INN_SEL   = (unsigned int)(an_perif_adc_in_sel & 0x0F);
#endif
    }
#ifdef CHIP_9118
    pstcADC->AUXADC_CONFIG_1_b.AUXADC_INP_SEL = an_perif_adc_ip_sel;
#endif
#if defined(SLI_SI917)
    pstcADC->AUXADC_CONFIG_2_b.AUXADC_INP_SEL = (unsigned int)(an_perif_adc_ip_sel & 0x1F);
#endif
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_ChannelSamplingRate(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel, uint16_t adc_ch_offset, uint16_t adc_ch_freq_val)
 * @brief        This API is used to configure the ADC sampling rate for ADC channels. 
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel  :ADC channel to be configured as 0,1,2 ...15
 * @param[in]    adc_ch_offset     : channel offset for each channel 
 * @param[in]    adc_ch_freq_val   : channel frequency for each channel to set sampling rate
 * @return       execution status 
 */
rsi_error_t RSI_ADC_ChannelSamplingRate(AUX_ADC_DAC_COMP_Type *pstcADC,
                                        uint32_t channel,
                                        uint16_t adc_ch_offset,
                                        uint16_t adc_ch_freq_val)
{
  if (adc_ch_freq_val == 1) {
    return ERROR_FREQ_VAL;
  } else {
    pstcADC->ADC_CH_OFFSET_b[channel].CH_OFFSET = adc_ch_offset;
    pstcADC->ADC_CH_FREQ[channel]               = adc_ch_freq_val;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_StaticMode(AUX_ADC_DAC_COMP_Type *pstcADC, uint16_t an_perif_adc_ip_sel, uint16_t an_perif_adc_in_sel, uint8_t an_perif_adc_diffmode)
 * @brief        This API is used to configure the ADC in Static Mode 
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    an_perif_adc_ip_sel   : ADC positive input select in static mode.
 * @param[in]    an_perif_adc_in_sel   : ADC negative input select in static mode.
 * @param[in]    an_perif_adc_diffmode : ADC differential mode selection in static mode.
 * @return       execution status 
 */
rsi_error_t RSI_ADC_StaticMode(AUX_ADC_DAC_COMP_Type *pstcADC,
                               uint16_t an_perif_adc_ip_sel,
                               uint16_t an_perif_adc_in_sel,
                               uint8_t an_perif_adc_diffmode)
{

  if (an_perif_adc_diffmode) {
#ifdef CHIP_9118
    pstcADC->AUXADC_CONFIG_1_b.AUXADC_DIFF_MODE = an_perif_adc_diffmode;
    pstcADC->AUXADC_CONFIG_1_b.AUXADC_INN_SEL   = an_perif_adc_in_sel;
#endif
#if defined(SLI_SI917)
    pstcADC->AUXADC_CONFIG_2_b.AUXADC_DIFF_MODE = (unsigned int)(an_perif_adc_diffmode & 0x01);
    pstcADC->AUXADC_CONFIG_2_b.AUXADC_INN_SEL   = (unsigned int)(an_perif_adc_in_sel & 0x0F);
#endif
  }
#ifdef CHIP_9118
  pstcADC->AUXADC_CONFIG_1_b.AUXADC_INP_SEL = an_perif_adc_ip_sel;
#endif
#if defined(SLI_SI917)
  pstcADC->AUXADC_CONFIG_2_b.AUXADC_INP_SEL = (unsigned int)(an_perif_adc_ip_sel & 0x1F);
#endif
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_ADC_FifoMode(AUX_ADC_DAC_COMP_Type *pstcADC, uint16_t channel_no, uint16_t an_perif_adc_ip_sel, uint16_t an_perif_adc_in_sel, uint8_t an_perif_adc_diffmode)
 * @brief        This API is used to configure the ADC in FIFO Mode 
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel_no  : Pointer to ADC Channel number
 * @param[in]    an_perif_adc_ip_sel   : ADC positive input select in Fifo mode.
 * @param[in]    an_perif_adc_in_sel   : ADC negative input select in Fifo mode.
 * @param[in]    an_perif_adc_diffmode : ADC differential mode selection in Fifo mode.
 * @return       execution status 
 */
rsi_error_t RSI_ADC_FifoMode(AUX_ADC_DAC_COMP_Type *pstcADC,
                             uint16_t channel_no,
                             uint16_t an_perif_adc_ip_sel,
                             uint16_t an_perif_adc_in_sel,
                             uint8_t an_perif_adc_diffmode)
{
  (void)an_perif_adc_in_sel;
  (void)an_perif_adc_ip_sel;
  pstcADC->AUXADC_CTRL_1_b.ADC_NUM_PHASE = 1;

  pstcADC->INTERNAL_DMA_CH_ENABLE_b.PER_CHANNEL_ENABLE |= (uint16_t)(1 << channel_no);

  if (an_perif_adc_diffmode) {
#ifdef CHIP_9118
    pstcADC->AUXADC_CONFIG_1_b.AUXADC_DIFF_MODE = an_perif_adc_diffmode;
    pstcADC->AUXADC_CONFIG_1_b.AUXADC_INN_SEL   = an_perif_adc_in_sel;
#endif
#if defined(SLI_SI917)
    pstcADC->AUXADC_CONFIG_2_b.AUXADC_DIFF_MODE = (unsigned int)(an_perif_adc_diffmode & 0x01);
    pstcADC->AUXADC_CONFIG_2_b.AUXADC_INN_SEL   = (unsigned int)(an_perif_adc_in_sel & 0x0F);
#endif
  }
#ifdef CHIP_9118
  pstcADC->AUXADC_CONFIG_1_b.AUXADC_INP_SEL = an_perif_adc_ip_sel;
#endif
#if defined(SLI_SI917)
  pstcADC->AUXADC_CONFIG_2_b.AUXADC_INP_SEL = (unsigned int)(an_perif_adc_ip_sel & 0x1F);
#endif
  return RSI_OK;
}

/// @cond
#ifdef CHIP_9118
/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_Start( AUX_ADC_DAC_COMP_Type *pstcADC )
 * @brief        This API is used to start the ADC
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel  : ADC channel to be configured as 0,1,2 ...15
 * @return       execution status 
 */
rsi_error_t RSI_ADC_Start(AUX_ADC_DAC_COMP_Type *pstcADC)
#endif
/// @endcond
#if defined(SLI_SI917)
  /*==============================================*/
  /**
 * @fn           rsi_error_t RSI_ADC_Start(AUX_ADC_DAC_COMP_Type *pstcADC,uint8_t oper_mode)
 * @brief        This API is used to start the ADC
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    oper_mode   : oper_mode; Static or dynamic mode.
 * @return       execution status 
 */
  rsi_error_t RSI_ADC_Start(AUX_ADC_DAC_COMP_Type *pstcADC, uint8_t oper_mode)
#endif
{
#ifdef CHIP_9118
  pstcADC->AUXADC_CONFIG_2_b.AUXADC_CONFIG_ENABLE = 1U;
#endif
#if defined(SLI_SI917)
  if (oper_mode == ADC_FIFOMODE_ENABLE) {
    pstcADC->AUXADC_CONFIG_2_b.AUXADC_DYN_ENABLE = 1U;
  }
#endif
  pstcADC->AUXADC_CTRL_1_b.ADC_ENABLE = 1U;
  return RSI_OK;
}

/// @cond
#ifdef CHIP_9118
/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_Stop( AUX_ADC_DAC_COMP_Type *pstcADC )
 * @brief        This API is used to stop the ADC
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @return       execution status 
 */
rsi_error_t RSI_ADC_Stop(AUX_ADC_DAC_COMP_Type *pstcADC)
#endif
/// @endcond
#if defined(SLI_SI917)
  /*==============================================*/
  /**
 * @fn           rsi_error_t RSI_ADC_Stop(AUX_ADC_DAC_COMP_Type *pstcADC, uint8_t oper_mode)
 * @brief        This API is used to stop the ADC
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    oper_mode   : ADC operation mode select,
 *                             - oper_mode : 1 Static Mode
 *                             - oper_mode : 0 Dynamic Mode 
 * @return       execution status 
 */
  rsi_error_t RSI_ADC_Stop(AUX_ADC_DAC_COMP_Type *pstcADC, uint8_t oper_mode)
#endif
{
#ifdef CHIP_9118
  pstcADC->AUXADC_CONFIG_2_b.AUXADC_CONFIG_ENABLE = 0;
#endif
#if defined(SLI_SI917)
  if (oper_mode == ADC_FIFOMODE_ENABLE) {
    pstcADC->AUXADC_CONFIG_2_b.AUXADC_DYN_ENABLE = 0;
  }
#endif
  if (AUX_ADC_DAC_COMP->AUXADC_CTRL_1_b.ADC_ENABLE != 1U) {
    return RSI_FAIL;
  }
  pstcADC->AUXADC_CTRL_1_b.ADC_ENABLE = 0;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_ChnlEnable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
 * @brief        This API is used to Enable the ADC channel
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel  :ADC channel to be configured as 0,1,2 ...15
 * @return       execution status 
 */
rsi_error_t RSI_ADC_ChnlEnable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
{
  if (channel > MAX_CHNL_NO) {
    return INVALID_PARAMETERS;
  }
  if ((AUX_ADC_DAC_COMP->AUXADC_CTRL_1_b.ADC_MULTIPLE_CHAN_ACTIVE)
      || (AUX_ADC_DAC_COMP->INTERNAL_DMA_CH_ENABLE_b.INTERNAL_DMA_ENABLE)) {
    pstcADC->INTERNAL_DMA_CH_ENABLE_b.PER_CHANNEL_ENABLE |= (uint16_t)(1 << channel);
  } else {
    return ERROR_NO_MULTI_CHNL_ENABLE;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_ChnlDisable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
 * @brief        This API is used to Disable the ADC
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel  :ADC channel to be configured as 0,1,2 ...15
 * @return       execution status 
 */
rsi_error_t RSI_ADC_ChnlDisable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
{
  if (channel > MAX_CHNL_NO) {
    return INVALID_PARAMETERS;
  }
  pstcADC->INTERNAL_DMA_CH_ENABLE_b.PER_CHANNEL_ENABLE &= (uint16_t)(~(1 << channel));
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           int16_t RSI_ADC_ReadDataStatic(const AUX_ADC_DAC_COMP_Type *pstcADC, uint8_t data_process_en, uint8_t diff_en)
 * @brief        This API is used to Read the ADC samples when static mode is enabled.
 * @param[in]    pstcADC             : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[out]   data_process_en     : This parameter define for the output ADC samples 
 *                                     gain and calculation enable or disable.
 *                                     - data_process_en -  1: enable the gain 
 *                                                        and calculation on output samples
 *                                     - data_process_en -  0: disable the gain 
 *                                                        and calculation on output samples
 * @param[in]    diff_en             : ADC mode of operation single ended or differential ended
 *                                     - diff_en   - 1   differential ended mode enable.
 *                                     - diff_en   - 0   single ended mode enable. 
 * @return       execution status 
 */
int16_t RSI_ADC_ReadDataStatic(const AUX_ADC_DAC_COMP_Type *pstcADC, uint8_t data_process_en, uint8_t diff_en)
{
  int16_t adc_temp = 0;
  if (data_process_en) {
    adc_temp = pstcADC->AUXADC_DATA_b.AUXADC_DATA;
    adc_temp = RSI_ADC_GainOffsetCal(adc_temp, diff_en);
    return adc_temp;
  } else {
    return pstcADC->AUXADC_DATA_b.AUXADC_DATA;
  }
}
/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_ReadData(int16_t *data,uint8_t ping_pong,uint16_t channel,
 *                                        uint8_t data_process_en,uint8_t diff_en)
 * @brief        This API is used to read the ADC samples when ulp memories are used.
 * @param[out]   data            :Pointer to read buffer 
 * @param[in]    Channel         :ADC channel number 
 * @param[in]    ping_pong       : 
 *                                 - 1 - ping memory read
 *                                 - 0 - pong memory read
 * @param[in]    data_process_en :This parameter define gain and offset calculation 
 *                                enabled on ADC output samples. 
 *                                - data_process_en :  1 , Enable the gain and offset calculation on ADC data.
 *                                - data_process_en :  0 , Disable the gain and offset calculation on ADC data. 
 * @param[in]    diff_en         : ADC mode of operation single ended or differential ended
 *                                 - diff_en   - 1   differential ended mode
 *                                 - diff_en   - 0   single ended mode
 * @return       execution status 
 */
rsi_error_t RSI_ADC_ReadData(int16_t *data,
                             uint8_t ping_pong,
                             uint16_t channel,
                             uint8_t data_process_en,
                             uint8_t diff_en)
{
  uint32_t addr_read            = 0;
  uint32_t sample_len           = 0;
  sl_dma_xfer_t dma_transfer_tx = { 0 };

  if (ping_pong) {
    addr_read  = adcInterConfig.ping_addr[channel];
    sample_len = adcInterConfig.ping_length[channel];
  } else {
    addr_read  = adcInterConfig.pong_addr[channel] + adcInterConfig.ping_length[channel];
    sample_len = adcInterConfig.pong_length[channel];
  }

  // read all samples data from ping/pong address

  uint32_t dma_channel           = DMA_CHANNEL;
  dma_transfer_tx.src_addr       = (uint32_t *)addr_read;
  dma_transfer_tx.dest_addr      = (uint32_t *)data;
  dma_transfer_tx.src_inc        = SRC_INC_16;
  dma_transfer_tx.dst_inc        = DST_INC_16;
  dma_transfer_tx.xfer_size      = DST_SIZE_16;
  dma_transfer_tx.transfer_count = sample_len;
  dma_transfer_tx.transfer_type  = SL_DMA_MEMORY_TO_MEMORY;
  dma_transfer_tx.dma_mode       = UDMA_MODE_PINGPONG;
  dma_transfer_tx.signal         = 0;
  sl_si91x_dma_transfer(DMA_INSTANCE, dma_channel, &dma_transfer_tx);

  if (data_process_en) {
    for (uint32_t i = 0; i < sample_len; i++) {
      data[i] ^= (int16_t)BIT(11);

      if (diff_en) {
        data[i] = (int16_t)((data[i] - adc_commn_config.adc_diff_offset) * adc_commn_config.adc_diff_gain);
      } else {
        data[i] = (int16_t)((data[i] - adc_commn_config.adc_sing_offset) * adc_commn_config.adc_sing_gain);
      }

      if (data[i] > 4095) {
        data[i] = 4095;
      } else if (data[i] <= 0) {
        data[i] = 0;
      }
      if (data[i] >= 2048) {
        data[i] -= 2048;
      } else {
        data[i] += 2048;
      }
    }
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_ClkDivfactor(AUX_ADC_DAC_COMP_Type *pstcADC,
 *                                            uint16_t adc_on_time , uint16_t adc_total_duration)
 * @brief        This API is used to set clock with configurable on time 
 * @param[in]    pstcADC          : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    adcOnTime        : ON duration of the clock 
 * @param[in]    adcTotalDuration : Total ON and OFF duration of the clock
 * @return       execution status 
 */
rsi_error_t RSI_ADC_ClkDivfactor(AUX_ADC_DAC_COMP_Type *pstcADC, uint16_t adc_on_time, uint16_t adc_total_duration)
{
  if ((adc_on_time > MAX_ON_DUR_TIME) || (adc_total_duration > MAX_TOL_DUR_TIME)) {
    return INVALID_PARAMETERS;
  }
  pstcADC->AUXADC_CTRL_1_b.EN_ADC_CLK           = 0;
  pstcADC->AUXADC_CLK_DIV_FAC_b.ADC_CLK_ON_DUR  = (uint16_t)(adc_on_time & 0x01FF);
  pstcADC->AUXADC_CLK_DIV_FAC_b.ADC_CLK_DIV_FAC = (uint16_t)(adc_total_duration & 0x3FF);
  pstcADC->AUXADC_CTRL_1_b.EN_ADC_CLK           = 1;

  return RSI_OK;
}

/// @cond
#ifdef CHIP_9118

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_ChnlIntrMask(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
 * @brief        This API is used to Unmask the ADC channel
 * @param[in]    pstcADC  : Pointer to the AUX_ADC_DAC_COMP_Type structure. 
 * @param[in]    channel  : ADC channel to be configured as 0,1,2 ...15
 * @return       execution status 
 */
rsi_error_t RSI_ADC_ChnlIntrUnMask(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
#endif
/// @endcond
#if defined(SLI_SI917)

  /*==============================================*/
  /**
 * @fn           rsi_error_t RSI_ADC_ChnlIntrUnMask(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel, uint8_t oper_mode)
 * @brief        This API is used to Unmask the ADC channel
 * @param[in]    pstcADC  : Pointer to the AUX_ADC_DAC_COMP_Type structure. 
 * @param[in]    channel  : ADC channel to be configured as 0,1,2 ...15
 * @param[in]    oper_mode: ADC operation mode select,
 *                          - oper_mode : 1 Static Mode
 *                          - oper_mode : 0 Dynamic Mode
 * @return       execution status 
 */
  rsi_error_t RSI_ADC_ChnlIntrUnMask(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel, uint8_t oper_mode)
#endif
{
  if (channel > MAX_CHNL_NO) {
    return INVALID_PARAMETERS;
  }
#ifdef CHIP_9118
  if (AUX_ADC_DAC_COMP->INTERNAL_DMA_CH_ENABLE_b.INTERNAL_DMA_ENABLE) {
    pstcADC->INTR_MASK_REG_b.FIRST_MEM_SWITCH_INTR_MASK &= ~(1 << channel);
  } else {
    pstcADC->INTR_MASK_REG_b.THRESHOLD_DETECTION_INTR_EN = 0;
    pstcADC->INTR_MASK_REG_b.ADC_FIFO_AFULL_INTR_MASK    = 0;
    pstcADC->INTR_MASK_REG_b.ADC_FIFO_FULL_INTR_MASK     = 0;
  }
#endif
#if defined(SLI_SI917)
  if (oper_mode == ADC_STATICMODE_ENABLE) {
    pstcADC->INTR_MASK_REG_b.ADC_STATIC_MODE_DATA_INTR_MASK = 0;
  } else if (oper_mode == ADC_STATICMODE_THRESHOLD_EN) {
    pstcADC->INTR_MASK_REG_b.THRESHOLD_DETECTION_INTR_EN = 0;
    pstcADC->INTR_MASK_REG_b.ADC_FIFO_AFULL_INTR_MASK    = 0;
    pstcADC->INTR_MASK_REG_b.ADC_FIFO_FULL_INTR_MASK     = 0;
  } else {
    if (AUX_ADC_DAC_COMP->INTERNAL_DMA_CH_ENABLE_b.INTERNAL_DMA_ENABLE) {
      pstcADC->INTR_MASK_REG_b.FIRST_MEM_SWITCH_INTR_MASK &= (uint16_t)(~(1 << channel));
    }
  }
#endif
  return RSI_OK;
}

/// @cond
#ifdef CHIP_9118

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_ChnlIntrMask(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
 * @brief        This API is used to Mask the ADC channel
 * @param[in]    pstcADC  : Pointer to the AUX_ADC_DAC_COMP_Type structure. 
 * @param[in]    channel  : ADC channel to be configured as 0,1,2 ...15
 * @return       execution status 
 */
rsi_error_t RSI_ADC_ChnlIntrMask(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
#endif
/// @endcond
#if defined(SLI_SI917)

  /*==============================================*/
  /**
 * @fn           rsi_error_t RSI_ADC_ChnlIntrMask(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel, uint8_t oper_mode)
 * @brief        This API is used to Mask the ADC channel
 * @param[in]    pstcADC  : Pointer to the AUX_ADC_DAC_COMP_Type structure. 
 * @param[in]    channel  : ADC channel to be configured as 0,1,2 ...15
 * @param[in]    oper_mode: ADC operation mode select,
 *                         - oper_mode : 1 Static Mode
 *                         - oper_mode : 0 Dynamic Mode
 * @return       execution status 
 */
  rsi_error_t RSI_ADC_ChnlIntrMask(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel, uint8_t oper_mode)
#endif
{
  if (channel > MAX_CHNL_NO) {
    return INVALID_PARAMETERS;
  }
#ifdef CHIP_9118
  if ((AUX_ADC_DAC_COMP->INTERNAL_DMA_CH_ENABLE_b.INTERNAL_DMA_ENABLE)) {
    pstcADC->INTR_MASK_REG_b.FIRST_MEM_SWITCH_INTR_MASK |= (1 << channel);
  } else {
    pstcADC->INTR_MASK_REG_b.THRESHOLD_DETECTION_INTR_EN = 1;
    pstcADC->INTR_MASK_REG_b.ADC_FIFO_AFULL_INTR_MASK    = 1;
    pstcADC->INTR_MASK_REG_b.ADC_FIFO_FULL_INTR_MASK     = 1;
  }
#endif
#if defined(SLI_SI917)
  if (oper_mode == ADC_STATICMODE_ENABLE) {
    pstcADC->INTR_MASK_REG_b.ADC_STATIC_MODE_DATA_INTR_MASK = 1;
  } else if (oper_mode == ADC_STATICMODE_THRESHOLD_EN) {
    pstcADC->INTR_MASK_REG_b.THRESHOLD_DETECTION_INTR_EN = 1;
    pstcADC->INTR_MASK_REG_b.ADC_FIFO_AFULL_INTR_MASK    = 1;
    pstcADC->INTR_MASK_REG_b.ADC_FIFO_FULL_INTR_MASK     = 1;
  } else if (AUX_ADC_DAC_COMP->INTERNAL_DMA_CH_ENABLE_b.INTERNAL_DMA_ENABLE) {
    pstcADC->INTR_MASK_REG_b.FIRST_MEM_SWITCH_INTR_MASK |= (uint16_t)(1 << channel);
  }
#endif
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_ChnlIntrClr(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
 * @brief        This API is used to clear the ADC channel
 * @param[in]    pstcADC  : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel  : ADC channel to be configured as 2^n where n = channel number = 0,1,2,3...15
 * @return       execution status 
 */
rsi_error_t RSI_ADC_ChnlIntrClr(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel)
{
  if (channel > MAX_CHNL_NO) {
    return INVALID_PARAMETERS;
  }
  pstcADC->INTR_CLEAR_REG_b.INTR_CLEAR_REG |= (uint16_t)(1 << channel);
  pstcADC->INTR_CLEAR_REG_b.INTR_CLEAR_REG &= (uint16_t)(~(1 << channel));
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           uint32_t RSI_ADC_ChnlIntrStatus(const AUX_ADC_DAC_COMP_Type *pstcADC)
 * @brief        This API is used to status the ADC channel
 * @param[in]    pstcADC  : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @return       execution status 
 */
uint32_t RSI_ADC_ChnlIntrStatus(const AUX_ADC_DAC_COMP_Type *pstcADC)
{
#ifdef CHIP_9118
  return pstcADC->INTR_STATUS_REG_b.FIRST_MEM_SWITCH_INTR;
#endif
#if defined(SLI_SI917)
  return pstcADC->INTR_STATUS_REG;
#endif
}

/*==============================================*/
/**
 * @fn           void RSI_ADC_PowerControl(POWER_STATE state)
 * @brief        This API is used to Power On and off for ADC
 * @param[in]    state : 
 *                     \b ADC_POWER_ON - To powerup adc powergates
 *                     \b ADC_POWER_OFF - To powerdown adc powergates
 * @return       execution status 
 */
void RSI_ADC_PowerControl(POWER_STATE state)
{
  switch (state) {
    case ADC_POWER_ON:
      RSI_IPMU_PowerGateSet(AUXADC_PG_ENB);
      RSI_PS_UlpssPeriPowerUp(ULPSS_PWRGATE_ULP_AUX);
      if (analog_get_power_state() == 0) {
        // Select ULP Ref clock for ADC
        RSI_ULPSS_AuxClkConfig(ULPCLK, ENABLE_STATIC_CLK, ULP_AUX_REF_CLK);
      }
      analog_set_power_state(ADC_BIT_POS, ANALOG_POWERED_ON);
      break;
    case ADC_POWER_OFF:
      RSI_IPMU_PowerGateClr(AUXADC_PG_ENB);
      analog_set_power_state(ADC_BIT_POS, ANALOG_POWERED_OFF);
      if (!analog_get_power_state()) {
        RSI_PS_UlpssPeriPowerDown(ULPSS_PWRGATE_ULP_AUX);
        RSI_ULPSS_PeripheralDisable(ULPCLK, ULP_AUX_CLK);
      }
      break;
  }
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_NoiseAvgMode(AUX_ADC_DAC_COMP_Type *pstcADC, bool en_disable)
 * @brief        This API is used to Enable or Disable Noise averaging mode
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure. 
 * @param[in]    en_disable  :
 *                            - 1 - To enable noise averaging mode 
 *                            - 0 - To disable noise averaging mode        
 * @return       execution status 
 */
rsi_error_t RSI_ADC_NoiseAvgMode(AUX_ADC_DAC_COMP_Type *pstcADC, bool en_disable)
{
  pstcADC->AUXADC_CTRL_1_b.BYPASS_NOISE_AVG = en_disable;
  ULP_SPI_MEM_MAP(0x110) |= (uint32_t)en_disable << POS17;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_TempSensorEnable(AUX_ADC_DAC_COMP_Type *pstcADC)
 * @brief        This API is used to Enable temp-Sensor 
 * @param[in]    pstcADC             : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @return       execution status 
 */
rsi_error_t RSI_ADC_TempSensorEnable(AUX_ADC_DAC_COMP_Type *pstcADC)
{
  pstcADC->TS_PTAT_ENABLE_b.TS_PTAT_EN = 1U;
  return RSI_OK;
}

#if defined(SLI_SI917)

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_ThresholdConfig(AUX_ADC_DAC_COMP_Type *pstcADC, int16_t threshold1, uint8_t *threshold1_cond,
 *                                int16_t threshold2, uint8_t *threshold2_cond, uint8_t range)
 * @brief        This API is used to compare threshold value with adc data 
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    threshold1  : threshold value to be programmed
 * @param[in]    threshold1_cond : threshold1 condition configuration 
 *                                 - THRESHOLD1_EQ   : 1
 *                                 - THRESHOLD1_GRT  : 2
 *                                 - THRESHOLD1_LESS : 3
 * @param[in]    threshold2  : threshold value to be programmed when range is 1 
 * @param[in]    threshold2_cond : threshold2 condition configuration 
 *                                 - THRESHOLD2_EQ   : 1
 *                                 - THRESHOLD2_GRT  : 2
 *                                 - THRESHOLD2_LESS : 3
 * @param[in]    range       : When adc data compared lies in certain range of threshold values set this bit
 * @return       execution status 
 */
rsi_error_t RSI_ADC_ThresholdConfig(AUX_ADC_DAC_COMP_Type *pstcADC,
                                    int16_t threshold1,
                                    uint8_t *threshold1_cond,
                                    int16_t threshold2,
                                    uint8_t *threshold2_cond,
                                    uint8_t range)
{
  int16_t temp_1                                                = 0;
  temp_1                                                        = pstcADC->AUXADC_DATA_b.AUXADC_DATA;
  pstcADC->ADC_DET_THR_CTRL_0_b.ADC_INPUT_DETECTION_THRESHOLD_0 = (uint8_t)threshold1;
  pstcADC->ADC_DET_THR_CTRL_0_b.ADC_INPUT_DETECTION_THRESHOLD_1 = (unsigned int)((threshold1 << 8) & 0x0F);

  if ((pstcADC->ADC_DET_THR_CTRL_0_b.ADC_INPUT_DETECTION_THRESHOLD_0
       | (pstcADC->ADC_DET_THR_CTRL_0_b.ADC_INPUT_DETECTION_THRESHOLD_1 >> 8))
      <= temp_1) {
    if ((pstcADC->ADC_DET_THR_CTRL_0_b.ADC_INPUT_DETECTION_THRESHOLD_0
         | (pstcADC->ADC_DET_THR_CTRL_0_b.ADC_INPUT_DETECTION_THRESHOLD_1 >> 8))
        == temp_1) {
      pstcADC->ADC_DET_THR_CTRL_0_b.COMP_EQ_EN = 1;
      *threshold1_cond                         = THRESHOLD1_EQ;
    } else {
      pstcADC->ADC_DET_THR_CTRL_0_b.COMP_GRTR_THAN_EN = 1;
      *threshold1_cond                                = THRESHOLD1_GRT;
    }
  } else {
    pstcADC->ADC_DET_THR_CTRL_0_b.COMP_LESS_THAN_EN = 1;
    *threshold1_cond                                = THRESHOLD1_LESS;
  }

  if (range) {
    pstcADC->ADC_DET_THR_CTRL_0_b.RANGE_COMPARISON_ENABLE = 1U;

    pstcADC->ADC_DET_THR_CTRL_1_b.ADC_INPUT_DETECTION_THRESHOLD_2      = (uint8_t)threshold2;
    pstcADC->ADC_DET_THR_CTRL_1_b.ADC_DETECTION_THRESHOLD_4_UPPER_BITS = (unsigned int)((threshold2 << 8) & 0x0F);

    if ((pstcADC->ADC_DET_THR_CTRL_1_b.ADC_INPUT_DETECTION_THRESHOLD_2
         | (pstcADC->ADC_DET_THR_CTRL_1_b.ADC_DETECTION_THRESHOLD_4_UPPER_BITS >> 8))
        <= temp_1) {
      if ((pstcADC->ADC_DET_THR_CTRL_1_b.ADC_INPUT_DETECTION_THRESHOLD_2
           | (pstcADC->ADC_DET_THR_CTRL_1_b.ADC_DETECTION_THRESHOLD_4_UPPER_BITS >> 8))
          == temp_1) {
        pstcADC->ADC_DET_THR_CTRL_1_b.COMP_EQ_EN = 1;
        *threshold2_cond                         = THRESHOLD2_EQ;
      } else {
        pstcADC->ADC_DET_THR_CTRL_1_b.COMP_GRTR_THAN_EN = 1;
        *threshold2_cond                                = THRESHOLD2_GRT;
      }
    } else {
      pstcADC->ADC_DET_THR_CTRL_1_b.COMP_LESS_THAN_EN = 1;
      *threshold2_cond                                = THRESHOLD2_LESS;
    }
  }
  return RSI_OK;
}
#endif

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_Bbp(AUX_ADC_DAC_COMP_Type *pstcADC ,uint8_t adc_bbp_en, uint8_t bbp_en, uint8_t bbp_id)
 * @brief        This API is used for baseband
 * @param[in]    pstcADC    : Pointer to the AUX_ADC_DAC_COMP_Type structure. 
 * @param[in]    adc_bbp_en : To enable adc to bbp path
 * @param[in]    bbp_en     : Adc samples are given to bbp
 * @param[in]    bbp_id     : Channel id for bbp samples
 * @return       execution status 
 */
rsi_error_t RSI_ADC_Bbp(AUX_ADC_DAC_COMP_Type *pstcADC, uint8_t adc_bbp_en, uint8_t bbp_en, uint8_t bbp_id)
{
  pstcADC->VAD_BBP_ID_b.AUX_ADC_BPP_EN = (unsigned int)(adc_bbp_en & 0x01);
  pstcADC->VAD_BBP_ID_b.BPP_EN         = (unsigned int)(bbp_en & 0x01);
  pstcADC->VAD_BBP_ID_b.BPP_ID         = (unsigned int)(bbp_id & 0x01);

  return RSI_OK;
}

#if defined(SLI_SI917)

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_ADC_InterruptHandler(const AUX_ADC_DAC_COMP_Type *pstcADC)
 * @brief       Handles all interrupt of ADC
 * @param[in]   pstcADC   :  Pointer to the MCPWM instance register area
 */
rsi_error_t RSI_ADC_InterruptHandler(const AUX_ADC_DAC_COMP_Type *pstcADC)
{
  volatile uint32_t intr_status;
  intr_status = RSI_ADC_ChnlIntrStatus(AUX_ADC_DAC_COMP);

  if ((intr_status & ADC_STATIC_MODE_INTR) && (pstcADC->INTR_MASK_REG_b.ADC_STATIC_MODE_DATA_INTR_MASK == 0)) {
#if defined(SLI_SI917)
    RSI_ADC_ChnlIntrMask(AUX_ADC_DAC_COMP, 0, ADC_STATICMODE_ENABLE);
#endif
    adc_commn_config.call_back_event(ADC_CHNL0_INTR, ADC_STATIC_MODE_CALLBACK);
  } else if (intr_status & ADC_THRESHOLD_DET_INTR) {
    RSI_ADC_ThreshInterruptClr(AUX_ADC_DAC_COMP);
    adc_commn_config.call_back_event(ADC_CHNL0_INTR, ADC_THRSHOLD_CALLBACK);
  }
#if defined(SLI_SI917)
  else if (intr_status & DAC_STATIC_MODE_INTR) {
    RSI_DAC_InterruptMask(AUX_ADC_DAC_COMP, 1);
    dac_callback_fun.callback_event(DAC_STATIC_MODE_CALLBACK);
  }
#endif
  else if ((AUX_ADC_DAC_COMP->AUXADC_CTRL_1_b.ADC_MULTIPLE_CHAN_ACTIVE)
           || (AUX_ADC_DAC_COMP->INTERNAL_DMA_CH_ENABLE_b.INTERNAL_DMA_ENABLE)) {
    for (uint8_t ch_num = 0; ch_num <= MAX_CHNL_NO; ch_num++) {
      if (intr_status & BIT(ch_num + FIRST_MEM_SWITCH_INTR_BIT)) {
        RSI_ADC_ChnlIntrClr(AUX_ADC_DAC_COMP, ch_num);
        RSI_ADC_PingPongReInit(AUX_ADC_DAC_COMP, ch_num, !pong_enable_sel[ch_num], pong_enable_sel[ch_num]);
        adc_commn_config.call_back_event(ch_num, INTERNAL_DMA);
        pong_enable_sel[ch_num] = !pong_enable_sel[ch_num];
      }
    }
  } else if ((!(AUX_ADC_DAC_COMP->AUXADC_CTRL_1_b.ADC_STATIC_MODE))
             && (!(AUX_ADC_DAC_COMP->INTERNAL_DMA_CH_ENABLE_b.INTERNAL_DMA_ENABLE))) {
    AUX_ADC_DAC_COMP->AUXADC_CTRL_1_b.ADC_FIFO_FLUSH = 1;
    adc_commn_config.call_back_event((uint8_t)intr_status, FIFO_MODE_EVENT);
  }
  return RSI_OK;
}
#endif

void ADC_DMA_Transfer_Complete(uint32_t channel, void *data)
{
  (void)&data;
  adc_commn_config.call_back_event((uint8_t)channel, UDMA_EVENT_XFER_DONE);
}

void ADC_DMA_Error_Callback(uint32_t channel, void *data)
{
  (void)&data;
  adc_commn_config.call_back_event((uint8_t)channel, UDMA_EVENT_XFER_DONE);
}

/*==============================================*/
/**
 * @fn           void RSI_ADC_ThreshInterruptClr(AUX_ADC_DAC_COMP_Type *pstcADC)
 * @brief        This API is used clear the threshold interrupt.
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @return       None
 */
void RSI_ADC_ThreshInterruptClr(AUX_ADC_DAC_COMP_Type *pstcADC)
{
  pstcADC->INTR_CLEAR_REG_b.CLR_INTR = 1;
}

/*==============================================*/
/**
 * @fn           void RSI_ADC_AUXLdoConfig(AUX_ADC_DAC_COMP_Type *pstcADC,uint16_t bypass_en,uint16_t value)
 * @brief        This API is used enable or disable the LDO for application.
 * @param[in]    pstcADC   : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    bypass_en : This parameter define LDO bypass enable or disable.
 *                           - bypass_en : 1 bypass the LDO    
 *                           - bypass_en : 0 On LDO mode 
 * @param[in]    value     : Configuration value for LDO when bypass_en =1.
 * @return       None
 */
void RSI_ADC_AUXLdoConfig(AUX_ADC_DAC_COMP_Type *pstcADC, uint16_t bypass_en, uint16_t value)
{
  pstcADC->AUX_LDO_b.ENABLE_LDO       = 1;
  pstcADC->AUX_LDO_b.LDO_DEFAULT_MODE = 1;

  if (bypass_en) {
    pstcADC->AUX_LDO_b.BYPASS_LDO = 1;
    pstcADC->AUX_LDO_b.LDO_CTRL   = (unsigned int)(value & 0x0F);
  } else {
    pstcADC->AUX_LDO_b.LDO_CTRL = (unsigned int)(value & 0x0F);
  }
}

/*==============================================*/
/**
 * @fn           int16_t RSI_ADC_GainOffsetCal(int16_t data,uint8_t diff_en)
 * @brief        This API is used to calculate gain and offset value.
 * @param[in]    data : data sample. 
 * @param[in]    diff_en : This parameter define differential mode is enable or disable.
 * @return       None
 */
int16_t RSI_ADC_GainOffsetCal(int16_t data, uint8_t diff_en)
{
  if (data & BIT(11)) {
    data = data & (int16_t)(ADC_MASK_VALUE);
  } else {
    data = data | BIT(11);
  }

  if (diff_en) {
    data = (int16_t)((data - adc_commn_config.adc_diff_offset) * adc_commn_config.adc_diff_gain);
  } else {
    data = (int16_t)((data - adc_commn_config.adc_sing_offset) * adc_commn_config.adc_sing_gain);
  }
  if (data > 4095) {
    data = 4095;
  }
  if (data <= 0) {
    data = 0;
  }
  if (data >= 2048) {
    data = data - 2048;
  } else {
    data = data + 2048;
  }
  return data;
}

#ifdef ADC_MULTICHANNEL_WITH_EXT_DMA
/**
 * @brief  This function is callback function to reconfigure uDMA ping and pong descriptor.
 * @param[in]  udmaHandle  :  uDMA handler
 * @param[in]  pTranDesc   :  Event type
 * @param[in] channel_no   : Pointer to ADC Channel number
 * @return None
 */
void adc_udmaTransferComplete(RSI_UDMA_HANDLE_T udmaHandle, RSI_UDMA_DESC_T *pTranDesc, uint32_t channel_no)
{
}

/*==============================================*/
/**
 * @fn     void UDMA_Start(void)
 * @brief  Enable the UDMA channel for operation.
 * @return None
 */
void UDMA_ADC_Start(void)
{
  RSI_UDMA_ChannelEnable(udmaHandle1, 11);

  RSI_UDMA_UDMAEnable(udmaHandle1);

  /* uDMA NVIC enable */
  NVIC_EnableIRQ(UDMA1_IRQn);
}

/*==============================================*/
/**
 * @fn     void UDMA_ADC_Pong_Write(uint8_t reconfig)
 * @brief  This function use for configure descriptor fo primary or alternate structure.
 * @param[in]  reconfig : This parameter use for skip uDMA initialization function in 
 *                     ping descriptor reconfiguration time.
 *                        - reconfig : 1 Dont skip configuration 
 *                        - reconfig : 0 skip configuration 
 * @return None
 */
void UDMA_ADC_Pong_Write(uint8_t reconfig)
{
  RSI_UDMA_CHA_CONFIG_DATA_T control = { 0 };
  RSI_UDMA_CHA_CFG_T config          = { 0 };

  config.altStruct       = 1;
  config.burstReq        = 1;
  config.channelPrioHigh = 0;
  config.periAck         = 0;
  config.periphReq       = 0;
  config.reqMask         = 0;
  config.dmaCh           = 11;

  // Set up source to destination copy for trigger for memory
  // Descriptor configurations
  control.transferType       = UDMA_MODE_PINGPONG;
  control.nextBurst          = 0;
  control.totalNumOfDMATrans = UDMA_PING_PONG_LENGTH - 1;
  control.rPower             = ARBSIZE_4;
  control.srcProtCtrl        = 0x000;
  control.dstProtCtrl        = 0x000;
  control.srcSize            = SRC_SIZE_16;
  control.srcInc             = SRC_INC_NONE;
  control.dstSize            = DST_SIZE_16;
  control.dstInc             = DST_INC_16;

  if (reconfig) {
    RSI_UDMA_SetupChannel(udmaHandle1, &config);
  }

  RSI_UDMA_SetupChannelTransfer(udmaHandle1,
                                &config,
                                control,
                                (uint32_t *)ADC_OUTPUT_REG,
                                (uint32_t *)(adc_output_buf));
}

/*==============================================*/
/**
 * @fn         void UDMA_ADC_Ping_Write(uint8_t ping_reconfig)
 * @brief      This function use for configure descriptor fo primary or alternate structure.
 * @param[in]  reconfig : This parameter use for skip uDMA initialization function in 
 *                        ping descriptor reconfiguration time.
 *                        - reconfig : 1 Dont skip configuration 
 *                        - reconfig : 0 skip configuration 
 * @return None
 */
void UDMA_ADC_Ping_Write(uint8_t ping_reconfig)
{
  RSI_UDMA_CHA_CONFIG_DATA_T control = { 0 };
  RSI_UDMA_CHA_CFG_T config          = { 0 };

  config.altStruct       = 0;
  config.burstReq        = 1;
  config.channelPrioHigh = 0;
  config.periAck         = 0;
  config.periphReq       = 0;
  config.reqMask         = 0;
  config.dmaCh           = 11;

  // Set up source to destination copy for trigger for memory
  // Descriptor configurations
  control.transferType       = UDMA_MODE_PINGPONG;
  control.nextBurst          = 0;
  control.totalNumOfDMATrans = UDMA_PING_PONG_LENGTH - 1;
  control.rPower             = ARBSIZE_4;
  control.srcProtCtrl        = 0x000;
  control.dstProtCtrl        = 0x000;
  control.srcSize            = SRC_SIZE_16;
  control.srcInc             = SRC_INC_NONE;
  control.dstSize            = DST_SIZE_16;
  control.dstInc             = DST_INC_16;

  if (ping_reconfig) {
    RSI_UDMA_SetupChannel(udmaHandle1, &config);
  }

  RSI_UDMA_SetupChannelTransfer(udmaHandle1,
                                &config,
                                control,
                                (uint32_t *)ADC_OUTPUT_REG,
                                (uint32_t *)(adc_output_buf + (UDMA_PING_PONG_LENGTH)));
}

/*==============================================*/
/**
 * @fn     void UDMA_ADC_Init(void)
 * @brief  Configure UDMA clock and parameters.
 * @retval None
 */
void UDMA_ADC_Init(void)
{
  RSI_UDMA_INIT_T udmaInit;
  uint32_t devMem[30];
  volatile bool done1 = NULL;

  RSI_ULPSS_PeripheralEnable(ULPCLK, ULP_UDMA_CLK, ENABLE_STATIC_CLK);

  RSI_PS_UlpssPeriPowerUp(ULPSS_PWRGATE_ULP_UDMA);

  udmaInit.pUserData = (void *)&done1;
  udmaInit.base      = (uint32_t)UDMA1;
  udmaInit.sramBase  = (uint32_t)&UDMA1_Table;
  udmaHandle1        = RSI_UDMA_Init(devMem, &udmaInit);

  // Configure the UDMA ping descriptor
  UDMA_ADC_Pong_Write(1);

  // Configure the UDMA pong descriptor
  UDMA_ADC_Ping_Write(1);

  // Soft-ware trigger for uDMA channel
  RSI_UDMA_ChannelSoftwareTrigger(udmaHandle1, 11);

  RSI_UDMA_RegisterCallback(udmaHandle1, (void *)adc_udmaTransferComplete);
}
#endif

#if defined(SLI_SI917)

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_DectEdgeSel(uint8_t detection_num, uint8_t edge_sel)
 * @brief        This API is used to select detection edge.
 * @param[in]    detection_num      : Detection Number
 * @param[in]    edge_sel           : This parameter use for select edge for trigger operation.
 * @return       Execution Status
 */
rsi_error_t RSI_ADC_DectEdgeSel(uint8_t detection_num, uint8_t edge_sel)
{
  if (edge_sel == POSITIVE_EDGE) {
    if (detection_num == DETECTION1) {
      AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIG_DETECT_1 = 1;
    } else if (detection_num == DETECTION2) {
      AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIG_DETECT_2 = 1;
    } else if (detection_num == DETECTION3) {
      AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIG_DETECT_3 = 1;
    } else if (detection_num == DETECTION4) {
      AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIG_DETECT_4 = 1;
    } else {
      return ERROR_ADC_INVALID_ARG;
    }
  } else if (edge_sel == NEGATIVE_EDGE) {
    if (detection_num == DETECTION1) {
      AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIG_DETECT_1 = 2;
    } else if (detection_num == DETECTION2) {
      AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIG_DETECT_2 = 2;
    } else if (detection_num == DETECTION3) {
      AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIG_DETECT_3 = 2;
    } else if (detection_num == DETECTION4) {
      AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIG_DETECT_4 = 2;
    } else {
      return ERROR_ADC_INVALID_ARG;
    }
  } else if (edge_sel == POS_NEG_EDGE) {
    if (detection_num == DETECTION1) {
      AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIG_DETECT_1 = 3;
    } else if (detection_num == DETECTION2) {
      AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIG_DETECT_2 = 3;
    } else if (detection_num == DETECTION3) {
      AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIG_DETECT_3 = 3;
    } else if (detection_num == DETECTION4) {
      AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIG_DETECT_4 = 3;
    } else {
      return ERROR_ADC_INVALID_ARG;
    }
  } else {
    return ERROR_ADC_INVALID_ARG;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_ChnlTriggSel(uint8_t ext_sel_num, uint8_t chnl_id)
 * @brief        This API is used to select channel trigger selection.
 * @param[in]    ext_sel_num       : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    chnl_id           : Channel Number
 * @return       Execution Status
 */
rsi_error_t RSI_ADC_ChnlTriggSel(uint8_t ext_sel_num, uint8_t chnl_id)
{
  if (ext_sel_num == EXT_TRIGGER_SEL1) {
    AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIGGER_SEL_1 = (unsigned int)(chnl_id & 0x0F);
  } else if (ext_sel_num == EXT_TRIGGER_SEL2) {
    AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIGGER_SEL_2 = (unsigned int)(chnl_id & 0x0F);
  } else if (ext_sel_num == EXT_TRIGGER_SEL3) {
    AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIGGER_SEL_3 = (unsigned int)(chnl_id & 0x0F);
  } else if (ext_sel_num == EXT_TRIGGER_SEL4) {
    AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.EXT_TRIGGER_SEL_4 = (unsigned int)(chnl_id & 0x0F);
  } else {
    return ERROR_ADC_INVALID_ARG;
  }

  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_ExtTrigConfig(const AUX_ADC_DAC_COMP_Type *pstcADC, adc_extr_config_t ExtrTrigConfig)
 * @brief        This API is used clear the threshold interrupt.
 * @param[in]    pstcADC           : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    ExtrTrigConfig    : ADC external trigger configuration structure variable. .
 * @return       Execution Status
 */
rsi_error_t RSI_ADC_ExtTrigConfig(const AUX_ADC_DAC_COMP_Type *pstcADC, adc_extr_config_t ExtrTrigConfig)
{
  (void)pstcADC;
  if (ExtrTrigConfig.trigger_type == ULP_TIMER_EXT_TRIGGER) {
    if (ExtrTrigConfig.trigger_num == DETECTION1) {
      // Timer 0 Select for trigger
      ULPCLK->ULP_MISC_SOFT_SET_REG_b.AUX_ULP_EXT_TRIG_1_SEL_b = 1;
    } else if (ExtrTrigConfig.trigger_num == DETECTION2) {
      // Timer 1 Select for trigger
      ULPCLK->ULP_MISC_SOFT_SET_REG_b.AUX_ULP_EXT_TRIG_2_SEL_b = 1;
    } else if (ExtrTrigConfig.trigger_num == DETECTION3) {
      // Timer 2 Select for trigger
      ULPCLK->ULP_MISC_SOFT_SET_REG_b.AUX_SOC_EXT_TRIG_1_SEL_b = 1;
    } else if (ExtrTrigConfig.trigger_num == DETECTION4) {
      ULPCLK->ULP_MISC_SOFT_SET_REG_b.AUX_SOC_EXT_TRIG_2_SEL_b = 1;
    } else {
      return ERROR_ADC_INVALID_ARG;
    }
  } else if (ExtrTrigConfig.trigger_type == ULP_GPIO_EXT_TRIGGER) {
    if (ExtrTrigConfig.trigger_num == DETECTION1) {
      ULPCLK->ULP_MISC_SOFT_SET_REG_b.AUX_ULP_EXT_TRIG_1_SEL_b = 0;
    } else if (ExtrTrigConfig.trigger_num == DETECTION2) {
      ULPCLK->ULP_MISC_SOFT_SET_REG_b.AUX_ULP_EXT_TRIG_2_SEL_b = 0;
    } else {
      return ERROR_ADC_INVALID_ARG;
    }
  } else if (ExtrTrigConfig.trigger_type == M4_CT_EXT_TRIGGER) {
    if (ExtrTrigConfig.trigger_num == DETECTION3) {
      ULPCLK->ULP_MISC_SOFT_SET_REG_b.AUX_SOC_EXT_TRIG_1_SEL_b = 0;
    } else if (ExtrTrigConfig.trigger_num == DETECTION4) {
      ULPCLK->ULP_MISC_SOFT_SET_REG_b.AUX_SOC_EXT_TRIG_2_SEL_b = 0;
    } else {
      return ERROR_ADC_INVALID_ARG;
    }
  } else {
    return ERROR_ADC_INVALID_ARG;
  }
  if (RSI_OK != RSI_ADC_DectEdgeSel((uint8_t)ExtrTrigConfig.trigger_num, (uint8_t)ExtrTrigConfig.trigger_num)) {
    return ERROR_ADC_INVALID_ARG;
  }
  if (RSI_OK != RSI_ADC_ChnlTriggSel((uint8_t)ExtrTrigConfig.trigger_sel, (uint8_t)ExtrTrigConfig.trigger_sel_val)) {
    return ERROR_ADC_INVALID_ARG;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           uint8_t RSI_ADC_ExtTrigStatusRead(const AUX_ADC_DAC_COMP_Type *pstcADC, adc_extr_config_t ExtrTrigConfig)
 * @brief        This API is used read external triggger status.
 * @param[in]    pstcADC           : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    ExtrTrigConfig    : ADC external trigger configuration structure variable. .
 * @return       If API return non value then interrupt match value occured
 */
uint8_t RSI_ADC_ExtTrigStatusRead(const AUX_ADC_DAC_COMP_Type *pstcADC, adc_extr_config_t ExtrTrigConfig)
{
  (void)pstcADC;
  if (ExtrTrigConfig.trigger_num == DETECTION1) {
    return (AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.TRIG_1_MATCH);
  } else if (ExtrTrigConfig.trigger_num == DETECTION2) {
    return (AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.TRIG_2_MATCH);
  } else if (ExtrTrigConfig.trigger_num == DETECTION3) {
    return (AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.TRIG_3_MATCH);
  } else if (ExtrTrigConfig.trigger_num == DETECTION4) {
    return (AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.TRIG_4_MATCH);
  }
  return 0;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_ADC_ExtTrigStatusClear(const AUX_ADC_DAC_COMP_Type *pstcADC, adc_extr_config_t ExtrTrigConfig)
 * @brief        This API is used clear external trigger status.
 * @param[in]    pstcADC           : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    ExtrTrigConfig    : ADC external trigger configuration structure variable. .
 * @return       If API return non value then interrupt match value occured
 *
 */
rsi_error_t RSI_ADC_ExtTrigStatusClear(const AUX_ADC_DAC_COMP_Type *pstcADC, adc_extr_config_t ExtrTrigConfig)
{
  (void)pstcADC;
  if (ExtrTrigConfig.trigger_num == DETECTION1) {
    AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.TRIG_1_MATCH = 1;
    AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.TRIG_1_MATCH = 0;
  } else if (ExtrTrigConfig.trigger_num == DETECTION2) {
    AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.TRIG_2_MATCH = 1;
    AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.TRIG_2_MATCH = 0;
  } else if (ExtrTrigConfig.trigger_num == DETECTION3) {
    AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.TRIG_3_MATCH = 1;
    AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.TRIG_3_MATCH = 0;
  } else if (ExtrTrigConfig.trigger_num == DETECTION4) {
    AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.TRIG_4_MATCH = 1;
    AUX_ADC_DAC_COMP->ADC_CTRL_REG_2_b.TRIG_4_MATCH = 0;
  }
  return 0;
}
/** @} */
#endif
