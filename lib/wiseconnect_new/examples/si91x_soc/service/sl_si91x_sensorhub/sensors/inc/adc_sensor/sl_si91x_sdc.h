/***************************************************************************/ /**
 * @file sl_si91x_sdc.h
 * @brief sl_si91x_sdc API implementation
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

#ifndef SL_SI91X_SDC_H_
#define SL_SI91X_SDC_H_

#include "rsi_bod.h"

#define SDC_IRQHandler IRQ025_Handler

#define SDCSS_Base_Addr 0x24042400

#define NPSS_GPIO_PIN_3        3
#define NPSS_GPIO_PIN_0        0
#define SDC_OPAMP1_OUT_MUX_SEL 0
#define SDC_OUTPUT1_MUX_ENABLE 1
#define SDC_OPAMP1_P_SEL_PIN   7
#define SDC_OPAMP1_OUTPUT_PIN  30

#define SDC_CLK_DIV_VALUE 160

#define SDC_SAMP_THRESH   1 ///< Number of data sampled to be collected from Aux-ADC
#define SDC_NUM_CH_SEL    1 ///< Number of Channels to be used for SDC
#define SDC_SAMP_TRIG_SEL 0 ///< Trigger event on which AUX-ADC Data is sampled (1-1ms;0-1sec)
#define SDC_CNT_TRIG_EVNT \
  0 ///< 0 - Sample Data on every Trigger Event 1 - Sample Data on every alternate Trigger Event etc...

typedef struct {
  uint8_t sdc_input_type[16];
  uint32_t sdc_sampling_rate[16];
  uint8_t sdc_pos_inp_sel[16];
  uint8_t sdc_neg_inp_sel[16];
  float sdc_opamp_gain[16];
  int16_t *sdc_rx_buf[16];         // Pointer to in data buffer
  uint16_t sdc_num_of_samples[16]; // Number of samples in 1 integration
} sdc_ch_config_t;

typedef struct {
  uint8_t sdc_p_channel_sel[4];
  uint8_t sdc_n_channel_sel[4];
  uint8_t sdc_sample_ther;
  uint8_t sdc_no_channel_sel;
  uint8_t sdc_data_trigger_sel;
  uint8_t sdc_sample_trigger_sel;
  uint8_t sdc_cnt_trig_evnt;
  uint8_t sdc_auxadc_diff_mode_ch[4];
  uint8_t sdc_auxadc_en;
  uint8_t sdc_adc_config_en;
  uint8_t sdc_clk_div;
  uint8_t sdc_samp_en;
} sl_drv_sdc_config_t;
/**************************************************************************/ /**
 *  @fn          void sl_si91x_sh_rtc_start(void)
 *  @brief       Enable the RTC Configurations
 *  @param[in]   None
 *  @return      None
 *******************************************************************************/
void sl_si91x_sh_rtc_start(void);

/**************************************************************************/ /**
 *  @fn          uint8_t IRQ029_Handler(void)
 *  @brief       RTC IRQ handler
 *  @param[in]   None
 *  @return      None
 *******************************************************************************/
void IRQ029_Handler(void);

/**************************************************************************/ /**
 *  @fn          uint8_t SDC_IRQHandler(void)
 *  @brief       Initialize the ADC Interface based on the configuration.
 *  @param[in]   None
 *  @return      status 0 if successful,
 *               else error code
 *               \ref SL_STATUS_FAIL - Fail ,
 *               \ref SL_STATUS_OK - Success,
*******************************************************************************/
void SDC_IRQHandler(void);

/**************************************************************************/ /**
 *  @fn          uint8_t sl_si91x_sdc_pin_configuration(void)
 *  @brief       This API used for configure GPIO pins in analog mode for SDC operation.
 *  @param[in]   None
 *******************************************************************************/
void sl_si91x_sdc_pin_configuration(void);

/**************************************************************************/ /**
 * @fn     sdc_pin_mux(uint8_t pos_input_pinsel, uint8_t neg_input_pinsel, uint8_t input_type)
 * @brief  This API used for configure GPIO pins in analog mode for SDC operation.
 * @param[in]  pos_inp_sel : Positive input selection for SDC channel.
 * @param[in] neg_inp_sel  : Negative input selection for SDC channel.
 * @param[in]  input_type  : Select single ended or differential ended input to SDC,
 *                           - For Single ended : 0
 *                           - Differential ended : 1
 * @return rsi_error_t         : Execution status,This function return 'RSI_OK' on
 *                           successful execution.
 *******************************************************************************/
rsi_error_t sdc_pin_mux(uint8_t pos_input_pinsel, uint8_t neg_input_pinsel, uint8_t input_type);

/**************************************************************************/ /**
 *  @fn          void sl_si91x_sdc_configure_clock(void)
 *  @brief       Configure the SDC Clocks.
 *  @param[in]   NONE
 *  @param[in]   NONE
 *******************************************************************************/
void sl_si91x_sdc_configure_clock(void);

/**************************************************************************/ /**
 *  @fn          uint16_t sl_si91x_read_sdc_data(uint16_t *sdc_value)
 *  @brief       Read the SDC data.
 *  @param[in]   sdc_value
 *  @param[out]  sdc_status
 *******************************************************************************/
uint16_t sl_si91x_read_sdc_data(int16_t sdc_drv_value[]);

/**************************************************************************/ /**
 *  @fn          uint16_t sl_si91x_read_sdc_data(uint16_t *sdc_value)
 *  @brief       Read the SDC data.
 *  @param[in]   sdc_value
 *  @param[out]  sdc_status
 *******************************************************************************/
uint16_t sl_si91x_read_sdc_multi_channel_data(int16_t sdc_drv_value[]);

/**************************************************************************/ /**
 *  @fn          void sl_si91x_SDC_AUXLdo_config(float sdc_verf_val, float sdc_chip_vltg)
 *  @brief       Configure the SDC LDO.
 *  @param[in]   sdc_verf_val
 *  @param[in]   sdc_chip_vltg
 *******************************************************************************/
void sl_si91x_SDC_AUXLdo_config(float sdc_verf_val, float sdc_chip_vltg);

/**************************************************************************/ /**
 *  @fn          void sl_si91x_SDC_calibration(void)
 *  @brief       Calibrate the SDC.
 *  @param[in]   None
 *  @param[out]   None
 *******************************************************************************/
void sl_si91x_SDC_calibration(void);

/**************************************************************************/ /**
 *  @fn          float sl_si91x_sdc_get_chip_voltage(void)
 *  @brief       If input supply to chip is less than 2.4V then switch input voltage supply
 *  @param[in]   None
 *  @return      sdc chip voltage
 *******************************************************************************/
float sl_si91x_sdc_get_chip_voltage(void);

/**************************************************************************/ /**
 *  @fn          void sl_si91x_configure_sdcss(void)
 *  @brief       Initialize the SDCC Interface based on the configuration.
 *  @param[in]   None
 *  @param[out]   None
 *******************************************************************************/
void sl_si91x_configure_sdcss(sl_drv_sdc_config_t sh_sdc_config);

/**************************************************************************/ /**
 *  @fn          void sl_si91x_sdc_intr_event_set(uint8_t channel_no, uint8_t event)
 *  @brief       Enable and set the priority of GPIO interrupt.
 *  @param[in]   channel_no     SDC Channel Number
 *  @param[in]   event          Event
 *  Return       None
*******************************************************************************/
void sl_si91x_sdc_intr_event_set(uint8_t channel_no, uint8_t event);

/**************************************************************************/ /**
 *  @fn          int16_t sl_si91x_SDC_GainOffsetCal(int16_t data, uint8_t diff_en)
 *  @brief       This API is used to calculate gain and offset value.
 *  @param[in]   data : data sample.
 *  @param[in]   diff_en : This parameter define differential mode is enable or disable.
 *  @return      data
 *******************************************************************************/
int16_t sl_si91x_SDC_GainOffsetCal(int16_t data, uint8_t diff_en);

#endif /* SL_SI91X_SDC_H_ */
