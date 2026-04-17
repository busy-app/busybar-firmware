/***************************************************************************/ /**
 * @file sl_si91x_sdc.c
 * @brief sdc example
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
 *    claim that you wrote the original software.
 *    If you use this software In in a product,
 *    an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
/***************************************************************************
 * @brief: This file contains all the Sensor data collector operations.
 * This file will contains the configuration and initializations of SDC.
 ******************************************************************************/

#include "si91x_device.h"
#include "sl_si91x_sdc.h"
#include "rsi_debug.h"
#include "rsi_adc.h"
#include "aux_reference_volt_config.h"
#include "rsi_ulpss_clk.h"
#include "rsi_ipmu.h"
#include "rsi_reg_spi.h"
#include "rsi_rom_egpio.h"
#include "rsi_rom_ulpss_clk.h"
#include "rsi_rtc.h"

#define NVIC_SEC_MSEC_RTC MCU_CAL_RTC_IRQn
volatile uint32_t sdc_calib_done      = 0;
volatile uint32_t sdc_calib_valueload = 0, sdc_calib_value = 0;
#define SDC_VREF_VALUE 2.8 // reference voltage

#define VREF_VALUE 3.3 // reference voltage
#define SDC_OPAMP  0

volatile uint8_t sdc_intr_done = 0;

/**************************************************************************/ /**
 *  @fn          void sl_si91x_sh_rtc_start(void)
 *  @brief       Enable the RTC Configurations
 *  @param[in]   None
 *  @return      None
 *******************************************************************************/
void sl_si91x_sh_rtc_start(void)
{
  /*Init the rtc block */
  RSI_RTC_Init(RTC);

  /*Start RTC */
  RSI_RTC_Start(RTC);

  /*Enables RTC NVIC */
  NVIC_EnableIRQ(NVIC_SEC_MSEC_RTC);

  if (SDC_SAMP_TRIG_SEL == 0) {
    //Sec Intr
    RSI_RTC_IntrUnMask(RTC_SEC_INTR);
  }

  if (SDC_SAMP_TRIG_SEL == 1) {
    //Msec Intr
    RSI_RTC_IntrUnMask(RTC_MSEC_INTR);
  }
}
/**************************************************************************/ /**
 *  @fn          uint8_t IRQ029_Handler(void)
 *  @brief       RTC IRQ handler
 *  @param[in]   None
 *  @return      None
 *******************************************************************************/
//Sec and Msec interrupt
void IRQ029_Handler(void)
{
  if (SDC_SAMP_TRIG_SEL == 0) {
    //Sec Intr
    RSI_RTC_IntrClear(RTC_SEC_INTR);
  }

  if (SDC_SAMP_TRIG_SEL == 1) {
    //Msec Intr
    RSI_RTC_IntrClear(RTC_MSEC_INTR);
  }
}
/**************************************************************************/ /**
 *  @fn          uint8_t SDC_IRQHandler(void)
 *  @brief       SDC IRQ handler
 *  @param[in]   None
 *  @return      None
 *******************************************************************************/
void SDC_IRQHandler(void)
{
  sdc_intr_done = 1;

  sl_si91x_sdc_intr_event_set(0, 0); //using this for setting the

  SDC->SDC_GEN_CONFIG_1_b.RST_WRT_PTR       = 1;
  SDC->SDC_GEN_CONFIG_1_b.RST_WRT_PTR       = 0;
  SDC->SDC_GEN_CONFIG_0_b.INTR_STATUS_CLEAR = 1;

  /*Clear SDC interrupt */
  NPSS_INTR_CLEAR_REG = NPSS_TO_MCU_SDC_INTR;
}

/**************************************************************************/ /**
 *  @fn          uint8_t sl_si91x_sdc_pin_configuration(void)
 *  @brief       This API used for configure GPIO pins in analog mode for SDC operation.
 *  @param[in]   None
 *******************************************************************************/
void sl_si91x_sdc_pin_configuration(void)
{
  RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO10);
  RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO10, ANALOG_MODE);
  RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO0);
  RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO0, ANALOG_MODE);
}

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
rsi_error_t sdc_pin_mux(uint8_t pos_input_pinsel, uint8_t neg_input_pinsel, uint8_t input_type)
{
  if (pos_input_pinsel == 0) {
    RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO0);
    RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO0, ANALOG_MODE);
  }
  if (pos_input_pinsel == POS1) {
    RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO2);
    RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO2, ANALOG_MODE);
  }
  if (pos_input_pinsel == POS2) {
    RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO4);
    RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO4, ANALOG_MODE);
  }
  if (pos_input_pinsel == POS3) {
    RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO6);
    RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO6, ANALOG_MODE);
  }
  if (pos_input_pinsel == POS4) {
    RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO8);
    RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO8, ANALOG_MODE);
  }
  if (pos_input_pinsel == 5) {
    RSI_EGPIO_UlpPadReceiverDisable(ADCGPIO10);
    RSI_EGPIO_SetPinMux(EGPIO1, 0, ADCGPIO10, ANALOG_MODE);
  }
  if (pos_input_pinsel == POS6) {
    *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
    RSI_EGPIO_PadReceiverDisable(ADC_TOPGPIO25);
    RSI_EGPIO_HostPadsGpioModeDisable(ADC_TOPGPIO25);
    RSI_EGPIO_SetPinMux(EGPIO, 0, ADC_TOPGPIO25, TGPIO_ANALOG_MODE);
  }
  if (pos_input_pinsel == POS7) {
    *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
    RSI_EGPIO_PadReceiverDisable(ADC_TOPGPIO27);
    RSI_EGPIO_HostPadsGpioModeDisable(ADC_TOPGPIO27);
    RSI_EGPIO_SetPinMux(EGPIO, 0, ADC_TOPGPIO27, TGPIO_ANALOG_MODE);
  }
  if (pos_input_pinsel == POS8) {
    *(volatile uint32_t *)(SDIO_HOST_CONTROL_IN_M4) = BIT(5);
    RSI_EGPIO_PadReceiverDisable(ADC_TOPGPIO29);
    RSI_EGPIO_HostPadsGpioModeDisable(ADC_TOPGPIO29);
    RSI_EGPIO_SetPinMux(EGPIO, 0, ADC_TOPGPIO29, TGPIO_ANALOG_MODE);
  }
#ifdef CHIP_9118
  if (pos_input_pinsel == POS9) {
    RSI_EGPIO_PadReceiverDisable(ADC_TOPGPIO23);
    RSI_EGPIO_SetPinMux(EGPIO, 0, ADC_TOPGPIO23, TGPIO_ANALOG_MODE);
  }
#endif
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
#ifdef CHIP_9118
  if ((pos_input_pinsel == POS19) || ((neg_input_pinsel == POS9) && (input_type == POS1))) {
    RSI_EGPIO_PadReceiverDisable(ADC_TOPGPIO24);
    RSI_EGPIO_SetPinMux(EGPIO, 0, ADC_TOPGPIO24, TGPIO_ANALOG_MODE);
  }
#endif
  return RSI_OK;
}

/**************************************************************************/ /**
 *  @fn          void sl_si91x_sdc_configure_clock(void)
 *  @brief       Configure the SDC Clocks.
 *  @param[in]   NONE
 *  @param[in]   NONE
 *******************************************************************************/
void sl_si91x_sdc_configure_clock(void)
{
  MCU_FSM->MCU_FSM_REF_CLK_REG_b.SDCSS_CLK_SEL_b       = 0x01; //SDC clock selection enable (01-RC_32MHz,10-HF_RO clock)
  MCU_FSM->MCU_FSM_REF_CLK_REG_b.SDCSS_CLK_EN_b        = 0x01; //To enable dynamic clock for SDC
  MCU_FSM->MCU_FSM_REF_CLK_REG_b.SDCSS_STATIC_CLK_EN_b = 0x01; //To enable static clock for SDC

  /* Note:Once the SDC configuration register are programmed,
  program SDCSS_STATIC_CLK_EN bit as '0'  in register MCU_FSM_REF_CLK to disable free running clock.
  Clock will be enabled once the configured trigger event occurs and it will remain, till sampling operation is completed. */
}

/**************************************************************************/ /**
 *  @fn          uint16_t sl_si91x_read_sdc_data(uint16_t *sdc_value)
 *  @brief       Read the SDC data.
 *  @param[in]   sdc_value
 *  @param[out]  sdc_status
 *******************************************************************************/
uint16_t sl_si91x_read_sdc_data(int16_t sdc_drv_value[])
{
  int16_t fifo_read_data[16];
  uint16_t sdc_status = 0;

  for (int16_t i = 0; i <= SDC_SAMP_THRESH; i++) {
    fifo_read_data[i] = SDC->SDC_DATA_REG[i].DATA_REG_b.SDC_DATA_SAMPLE;
    fifo_read_data[i] = sl_si91x_SDC_GainOffsetCal(fifo_read_data[i], 0);

    if (fifo_read_data[i] & BIT(11)) {
      fifo_read_data[i] = (fifo_read_data[i] & (ADC_MASK_VALUE));
    } else {
      fifo_read_data[i] = fifo_read_data[i] | BIT(11);
    }
    sdc_drv_value[i] = fifo_read_data[i];
    //DEBUGOUT("\r\n SDC_Reg_Data[%d] : %d \r\n",i,SDC->SDC_DATA_REG[i].DATA_REG_b.SDC_DATA_SAMPLE);
  }
  MCU_FSM->MCU_FSM_REF_CLK_REG_b.SDCSS_STATIC_CLK_EN_b = 0x00; //To disable static clock for SDC
  return sdc_status;
}

/**************************************************************************/ /**
 *  @fn          uint16_t sl_si91x_read_sdc_data(uint16_t *sdc_value)
 *  @brief       Read the SDC data.
 *  @param[in]   sdc_value
 *  @param[out]  sdc_status
 *******************************************************************************/
uint16_t sl_si91x_read_sdc_multi_channel_data(int16_t sdc_drv_value[])
{
  int16_t fifo_read_data[16];
  uint16_t sdc_status = 0, data_value = 0;

  for (int16_t i = 0; i <= SDC_SAMP_THRESH; i++) {
    data_value        = SDC->SDC_DATA_REG[i].DATA_REG;
    fifo_read_data[i] = SDC->SDC_DATA_REG[i].DATA_REG_b.SDC_DATA_SAMPLE;
    fifo_read_data[i] = sl_si91x_SDC_GainOffsetCal(fifo_read_data[i], 0);

    if (fifo_read_data[i] & BIT(11)) {
      fifo_read_data[i] = (fifo_read_data[i] & (ADC_MASK_VALUE));
    } else {
      fifo_read_data[i] = fifo_read_data[i] | BIT(11);
    }
    data_value       = data_value & 0xf000;
    sdc_drv_value[i] = (data_value | fifo_read_data[i]);
    // DEBUGOUT("\r\n SDC_Reg_Data[%d] : %d %lu data:%d drv:%d\r\n",i,SDC->SDC_DATA_REG[i].DATA_REG_b.SDC_DATA_SAMPLE,SDC->SDC_DATA_REG[i].DATA_REG,
    //          data_value,sdc_drv_value[i]);
  }
  MCU_FSM->MCU_FSM_REF_CLK_REG_b.SDCSS_STATIC_CLK_EN_b = 0x00; //To disable static clock for SDC
  return sdc_status;
}

/**************************************************************************/ /**
 *  @fn          void sl_si91x_SDC_AUXLdo_config(float sdc_verf_val, float sdc_chip_vltg)
 *  @brief       Configure the SDC LDO.
 *  @param[in]   sdc_verf_val
 *  @param[in]   sdc_chip_vltg
 *******************************************************************************/
void sl_si91x_SDC_AUXLdo_config(float sdc_verf_val, float sdc_chip_vltg)
{

  uint8_t temp                                  = 0;
  float cntrl_val                               = 0;
  SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_EN        = 1;
  SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_CONFIG_EN = 1;
  if ((sdc_verf_val > (float)2.8) || (sdc_chip_vltg < (float)3.0)) {
    SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_BYP_EB = 1;
  } else {
    if (sdc_verf_val < (float)1.6) {
      SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_VOLT_CTRL = 0;
      SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_EN        = 0;
      SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_CONFIG_EN = 0;
    } else {
      cntrl_val = ((sdc_verf_val - (float)1.6) / (float)(0.08));
      temp      = (uint8_t)(cntrl_val + 1);
      cntrl_val = (cntrl_val + (float)0.50);
      if ((float)temp > cntrl_val) {
        temp = (uint8_t)cntrl_val;
      }
      SDC->SDC_AUXLDO_CONFIG_b.SDC_AUXLDO_VOLT_CTRL = (unsigned int)(temp & 0x0F);
    }
  }
}

/**************************************************************************/ /**
 *  @fn          void sl_si91x_SDC_calibration(void)
 *  @brief       Calibrate the SDC.
 *  @param[in]   None
 *  @param[out]   None
 *******************************************************************************/
void sl_si91x_SDC_calibration(void)
{
  uint32_t impuDummyRead = 0;
  (void)impuDummyRead;

  AUX_ADC_DAC_COMP->AUXADC_CTRL_1_b.ADC_ENABLE |= 1U;
  AUX_ADC_DAC_COMP->AUXADC_CONFIG_2 |= BIT(10);

  impuDummyRead = ULP_SPI_MEM_MAP(0);
  if (sdc_calib_done == 0) {
    ULP_SPI_MEM_MAP(AUXADCREG0) |= BIT(13) | BIT(12) | BIT(11) | BIT(8) | BIT(7) | BIT(6);
    while (GSPI_CTRL_REG1 & SPI_ACTIVE)
      ;
    /*Wait for 1*/
    while (!(ULP_SPI_MEM_MAP(SPAREREG2) & BIT(0)))
      ;
    /*wait for 0*/
    while ((ULP_SPI_MEM_MAP(SPAREREG2) & BIT(0)))
      ;
    /*150 clocks of 1 Mhz wait*/
    sdc_calib_value = ULP_SPI_MEM_MAP(AUXADCREG2);
    sdc_calib_valueload |= BIT(0) | BIT(7);
    sdc_calib_valueload |= (sdc_calib_value & 0x1F) << 2;
    sdc_calib_valueload |= (((sdc_calib_value >> POS5) & 0x1F) << POS13);
    sdc_calib_valueload |= (((sdc_calib_value >> POS10) & 0x1F) << POS8);
    sdc_calib_done = 1;
  } else {
  }

  ULP_SPI_MEM_MAP(AUXADCREG1) = (sdc_calib_valueload);
  while (GSPI_CTRL_REG1 & SPI_ACTIVE)
    ;
}
/**************************************************************************/ /**
 *  @fn          float sl_si91x_sdc_get_chip_voltage(void)
 *  @brief       If input supply to chip is less than 2.4V then switch input voltage supply
 *  @param[in]   None
 *  @return      sdc chip voltage
 *******************************************************************************/
float sl_si91x_sdc_get_chip_voltage(void)
{
  volatile float sdc_chip_volt = 0;
  float max_ip_volt_scdc       = (float)2.4;
  //Get the input voltage of chip.
  sdc_chip_volt = RSI_BOD_SoftTriggerGetBatteryStatus();
  //If input to chip less than 2.4V then switch input voltage supply from SCDC to HPLDO
  if (sdc_chip_volt < max_ip_volt_scdc) {
    RSI_IPMU_ProgramConfigData(hp_ldo_voltsel);
  }
  return sdc_chip_volt;
}
/**************************************************************************/ /**
 *  @fn          int16_t sl_si91x_SDC_GainOffsetCal(int16_t data, uint8_t diff_en)
 *  @brief       This API is used to calculate gain and offset value.
 *  @param[in]   data : data sample.
 *  @param[in]   diff_en : This parameter define differential mode is enable or disable.
 *  @return      data
 *******************************************************************************/
int16_t sl_si91x_SDC_GainOffsetCal(int16_t data, uint8_t diff_en)
{
  uint16_t sdc_sing_offset = 0;
  uint16_t sdc_diff_offset = 0;
  float sdc_sing_gain = 0, sdc_diff_gain = 0;
  uint32_t integer_val = 0;
  float frac           = 0;
  if (data & BIT(11)) {
    data = data & (int16_t)(ADC_MASK_VALUE);
  } else {
    data = data | BIT(11);
  }

  // Offset value and gain value read from efuse
  sdc_sing_offset = (uint16_t)RSI_IPMU_Auxadcoff_SeEfuse();
  sdc_diff_offset = (uint16_t)RSI_IPMU_Auxadcoff_DiffEfuse();

  // Single ended gain
  integer_val = RSI_IPMU_Auxadcgain_SeEfuse();
  frac        = (float)((integer_val) & (0x3FFF));
  frac /= 1000;
  sdc_sing_gain = ((float)(integer_val >> 14) + frac);

  // Differential ended gain
  integer_val = RSI_IPMU_Auxadcgain_DiffEfuse();
  frac        = (float)((integer_val) & (0x3FFF));
  frac /= 1000;
  sdc_diff_gain = (((float)(integer_val >> 14)) + frac);

  if (diff_en) {
    data = (int16_t)((data - sdc_diff_offset) * sdc_diff_gain);
  } else {
    data = (int16_t)((data - sdc_sing_offset) * sdc_sing_gain);
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

/**************************************************************************/ /**
 *  @fn          void sl_si91x_configure_sdcss(void)
 *  @brief       Initialize the SDCC Interface based on the configuration.
 *  @param[in]   None
 *  @param[out]   None
 *******************************************************************************/
void sl_si91x_configure_sdcss(sl_drv_sdc_config_t sh_sdc_config)
{
  volatile float battery_status = 0;

  battery_status = sl_si91x_sdc_get_chip_voltage();

  battery_status = 3.3;

  NVIC_EnableIRQ(NPSS_TO_MCU_SDC_INTR_IRQn);

  //masking SDCSS intr
  NPSS_INTR_MASK_CLR_REG = NPSS_TO_MCU_SDC_INTR;

  RSI_ADC_PowerControl(ADC_POWER_ON);

  //Clock division factor for calibration,Calibrate ADC on 4MHZ clock
  RSI_ADC_ClkDivfactor(AUX_ADC_DAC_COMP, 0, 4);

  //Set analog reference voltage
  RSI_AUX_RefVoltageConfig((float)2.8, (float)3.2);

  sl_si91x_SDC_calibration();

  SDC->SDC_GEN_CONFIG_1_b.SAMP_THRESH   = sh_sdc_config.sdc_sample_ther;        //c
  SDC->SDC_GEN_CONFIG_2_b.NUM_CH_SEL    = sh_sdc_config.sdc_no_channel_sel;     //c
  SDC->SDC_GEN_CONFIG_3_b.CNT_TRIG_EVNT = sh_sdc_config.sdc_cnt_trig_evnt;      //c
  SDC->SDC_GEN_CONFIG_3_b.SAMP_TRIG_SEL = sh_sdc_config.sdc_sample_trigger_sel; //c //RTC(1 � 1ms 0 � 1sec)
  SDC->SDC_GEN_CONFIG_3_b.SDC_CLK_DIV   = SDC_CLK_DIV_VALUE;                    //c

  sl_si91x_SDC_AUXLdo_config(SDC_VREF_VALUE, battery_status);

  SDC->SDC_AUXADC_CONFIG_1_b.SDC_AUXADC_INPUT_P_SEL_CH1 = sh_sdc_config.sdc_p_channel_sel[0];       //c
  SDC->SDC_AUXADC_CONFIG_1_b.SDC_AUXADC_INPUT_N_SEL_CH1 = sh_sdc_config.sdc_n_channel_sel[0];       //c
  SDC->SDC_AUXADC_CONFIG_1_b.SDC_AUXADC_DIFF_MODE_CH1   = sh_sdc_config.sdc_auxadc_diff_mode_ch[0]; //c

#ifdef SDC_MUTI_CHANNEL_ENABLE
#ifdef SDC_CHANNEL_2
  SDC->SDC_AUXADC_CONFIG_2_b.SDC_AUXADC_INPUT_P_SEL_CH2 = sh_sdc_config.sdc_p_channel_sel[1];
  SDC->SDC_AUXADC_CONFIG_2_b.SDC_AUXADC_INPUT_N_SEL_CH2 = sh_sdc_config.sdc_n_channel_sel[1];
  SDC->SDC_AUXADC_CONFIG_2_b.SDC_AUXADC_DIFF_MODE_CH2   = sh_sdc_config.sdc_auxadc_diff_mode_ch[1];
#endif

#ifdef SDC_CHANNEL_3

  SDC->SDC_AUXADC_CONFIG_3_b.SDC_AUXADC_INPUT_P_SEL_CH3 = sh_sdc_config.sdc_p_channel_sel[2];
  SDC->SDC_AUXADC_CONFIG_3_b.SDC_AUXADC_INPUT_N_SEL_CH3 = sh_sdc_config.sdc_n_channel_sel[2];
  SDC->SDC_AUXADC_CONFIG_3_b.SDC_AUXADC_DIFF_MODE_CH3   = sh_sdc_config.sdc_auxadc_diff_mode_ch[2];

#endif

#ifdef SDC_CHANNEL_4

  SDC->SDC_AUXADC_CONFIG_4_b.SDC_AUXADC_INPUT_P_SEL_CH4 = sh_sdc_config.sdc_p_channel_sel[3];
  SDC->SDC_AUXADC_CONFIG_4_b.SDC_AUXADC_INPUT_N_SEL_CH4 = sh_sdc_config.sdc_n_channel_sel[3];
  SDC->SDC_AUXADC_CONFIG_4_b.SDC_AUXADC_DIFF_MODE_CH4   = sh_sdc_config.sdc_auxadc_diff_mode_ch[3];

#endif
#endif

#if SDC_OPAMP
  ////////////////////////OPAMP-1///////////////////////////////////////////////
  RSI_EGPIO_SetPinMux(EGPIO1, 0, SDC_OPAMP1_P_SEL_PIN, AGPIO_MODE); // OPAMP1_P_SEL_PIN=7
  RSI_EGPIO_PadReceiverDisable(SDC_OPAMP1_OUTPUT_PIN);
  RSI_EGPIO_HostPadsGpioModeDisable(SDC_OPAMP1_OUTPUT_PIN);
  RSI_EGPIO_PadSdioConnected();
  RSI_EGPIO_SetPinMux(EGPIO, 0, SDC_OPAMP1_OUTPUT_PIN, TGPIO_MODE);

  SDC->SDC_AUXOPAMP_CONFIG_1_b.SDC_OPAMP_EN_CH1         = 1; //  1bit
  SDC->SDC_AUXOPAMP_CONFIG_1_b.SDC_OPAMP_LP_MODE        = 0; //  2 Bit
  SDC->SDC_AUXOPAMP_CONFIG_1_b.SDC_OPAMP_R1_SEL         = 1;
  SDC->SDC_AUXOPAMP_CONFIG_1_b.SDC_OPAMP_R2_SEL         = 0;
  SDC->SDC_AUXOPAMP_CONFIG_1_b.SDC_OPAMP_RES_BACK_EN    = 0; //  7 Bit
  SDC->SDC_AUXOPAMP_CONFIG_1_b.SDC_OPAMP_RES_MUX_SEL    = 0;
  SDC->SDC_AUXOPAMP_CONFIG_1_b.SDC_OPAMP_RES_TO_OUT_VDD = 0;
  SDC->SDC_AUXOPAMP_CONFIG_1_b.SDC_OPAMP_OUT_MUX_EN     = 1;
  SDC->SDC_AUXOPAMP_CONFIG_1_b.SDC_OPAMP_OUT_MUX_SEL    = 1;
  SDC->SDC_AUXOPAMP_CONFIG_1_b.SDC_OPAMP_IN_N_SEL       = 4; //  13-15 bits
  SDC->SDC_AUXOPAMP_CONFIG_1_b.SDC_OPAMP_IN_P_SEL_CH1   = 1; //  16-19 bits
  SDC->SDC_AUXOPAMP_CONFIG_1_b.SDC_OPAMP_CONFIG_EN      = 1; //  31 bit
  //////////////////////////////////////////////////////////////////////////////
#endif

  SDC->SDC_AUXADC_CONFIG_1_b.SDC_AUXADC_EN     = ENABLE;
  SDC->SDC_AUXADC_CONFIG_1_b.SDC_ADC_CONFIG_EN = ENABLE;
  SDC->SDC_GEN_CONFIG_2_b.SDC_SAMP_EN          = ENABLE;
}
/*End of file not truncated*/
