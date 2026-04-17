/******************************************************************************
* @file  rsi_dac.c
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

#include "clock_update.h"
#include "UDMA.h"
#include "rsi_dac.h"
#include "rsi_power_save.h"
#include "rsi_egpio.h"
#include "rsi_opamp.h"
#include "rsi_ulpss_clk.h"
#include "rsi_rom_ulpss_clk.h"
#include "rsi_rom_udma.h"

// UDMA config
// DMA descriptors must be aligned to 16 bytes
#if defined(__CC_ARM)
extern RSI_UDMA_DESC_T UDMA1_Table[32];
#endif // defined (__CC_ARM)

#ifdef DAC_FIFO_MODE_EN
#if defined(__GNUC__)
extern RSI_UDMA_DESC_T __attribute__((section(".udma_addr1"))) UDMA1_Table[32];
#endif // defined (__GNUC__)

extern RSI_UDMA_HANDLE_T udmaHandle1;
#endif

#define DAC_SAMPLE_RATE_32KSPS 32000
#define DAC_SAMPLE_RATE_80KSPS 80000
#define DAC_CLK_SRC_32KHZ      32000
#define DAC_CLK_SRC_MHZ        32000000

dac_config_t dac_callback_fun;
uint8_t dac_pong_enable_sel = 0;
uint32_t devMem[30];
analog_power_control_t analog_power_ctrl;

/** @addtogroup SOC22
* @{
*/
/*==============================================*/
/**
 * @fn           uint32_t DAC_Init(uint8_t operation_mode,uint32_t sampling_rate,daccallbacFunc event)
 * @brief        This API used for initialize DAC operation in static mode or in FIFO mode. 
 * @param[in]    operation_mode     : This parameter define DAC in FIFO mode or in static mode
 *                                    - Configure operation_mode = 0  , for Fifo mode
 *                                    - Configure operation_mode = 1  , for Static mode 
 * @param[in]    sampling_rate      : Sampling rate for DAC operation.
 * @param[in]    event              : Call back event pointer. 
 * @return       Return the sampling value which configured in DAC module.
 */
uint32_t DAC_Init(uint8_t operation_mode, uint32_t sampling_rate, daccallbacFunc event)
{
  uint32_t achived_sample_freq = 0;

  dac_callback_fun.callback_event = event;

  if (sampling_rate > MAX_DAC_SAMP_RATE) {
    return INVALID_PARAMETERS;
  }

  // DAC power gate enable
  RSI_DAC_PowerControl(DAC_POWER_ON);

  // Configure sampling rate
  achived_sample_freq = dac_set_clock(sampling_rate);

  if (operation_mode) {
    // Configure the DAC control parameter
    RSI_DAC_Config(AUX_ADC_DAC_COMP, operation_mode, ENABLE, DISABLE, DISABLE);
  } else {
    //Set the DAC threshold value
    RSI_DAC_SetFifoThreshold(AUX_ADC_DAC_COMP, DAC_FIFO_THR);
#if !defined(SLI_SI917B0)
    // Configure the DAC control parameter
    RSI_DAC_Config(AUX_ADC_DAC_COMP, operation_mode, ENABLE, DISABLE, ENABLE);
#else
    RSI_DAC_Config(AUX_ADC_DAC_COMP, operation_mode, ENABLE, ENABLE, ENABLE);
#endif
#ifdef DAC_FIFO_MODE_EN
    // Configure the UDMA for DAC peripheral
    dac_udma_init();
#endif
  }

#ifndef SL_SI91X_DAC
#if !defined(SLI_SI917B0)
  //Configure DAC output on AGPIO4
  DAC_PinMux(0);
#else
  DAC_PinMux(1);
#endif
#endif

  return achived_sample_freq;
}

/*==============================================*/
/**
 * @fn           rsi_error_t DAC_WriteData(uint8_t operation_mode,int16_t *wr_buf,uint16_t length)
 * @brief        This API use for writing digital sample value in DAC input register. 
 * @param[in]    operation_mode     : This parameter define DAC in FIFO mode or in static mode
 *                                    - Configure operation_mode = 0  , for Fifo mode
 *                                    - Configure operation_mode = 1  , for Static mode
 * @param[in]    wr_buf             : Input samples buffer pointer.
 * @param[in]    length             : Number of sample, This value will be one when DAC 
 *                                    using static mode for operation.
 * @return       Return zero on successful API execution.         
 */
rsi_error_t DAC_WriteData(uint8_t operation_mode, int16_t *wr_buf, uint16_t length)
{
  if (operation_mode) {
    // writing digital sample value in DAC input register in DAC in static mode
    RSI_DAC_WriteData(AUX_ADC_DAC_COMP, wr_buf, STATIC_MODE_EN, length);
  } else {
#ifdef DAC_FIFO_MODE_EN
    // Enable UDMA for DAC channel
    // Configure ping descriptor address
    dac_udma_write(1, length, wr_buf, ENABLE);
    // Configure pong descriptor address
    dac_udma_write(0, length, wr_buf, ENABLE);
#endif
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t DAC_Start(uint8_t operation_mode)
 * @brief        Start DAC operation.  
 * @param[in]    operation_mode     : This parameter define DAC in FIFO mode or in static mode
 *                                    - Configure operation_mode = 0  , for Fifo mode
 *                                    - Configure operation_mode = 1  , for Static mode
 * @return       Return zero on successful API execution.
 */
rsi_error_t DAC_Start(uint8_t operation_mode)
{
  // FIFO mode enable
  if (operation_mode) {
#if defined(SLI_SI917)
    RSI_DAC_InterruptUnMask(AUX_ADC_DAC_COMP, operation_mode);
    NVIC_EnableIRQ(ADC_IRQn);
#endif
    // Enable DAC
    RSI_DAC_Start(AUX_ADC_DAC_COMP, ENABLE);
  } else {
#ifdef DAC_FIFO_MODE_EN
    dac_udma_start();
#endif
    // Enable DAC
    RSI_DAC_Start(AUX_ADC_DAC_COMP, ENABLE);
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t DAC_PingPongReconfig(int16_t *wr_buf, uint16_t length)
 * @brief        This API used for reconfigure the udma ping or pong descriptor for 
 *               playing continuous digital word in DAC input buffer. 
 *               This API used in DAC in FIFO mode.
 * @param[in]    wr_buf  : Input samples buffer pointer.
 * @param[in]    length  : Number of samples to play in DAC.
 * @return       Return zero on successful API execution.
 */
rsi_error_t DAC_PingPongReconfig(int16_t *wr_buf, uint16_t length)
{
  (void)wr_buf;
  (void)length;
#ifdef DAC_FIFO_MODE_EN
  if (dac_pong_enable_sel) {
    dac_udma_write(dac_pong_enable_sel, length, wr_buf, DISABLE);
    dac_pong_enable_sel = 0;
  } else {
    dac_udma_write(dac_pong_enable_sel, length, wr_buf, DISABLE);
    dac_pong_enable_sel = 1;
  }
#endif
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t DAC_Stop(void)
 * @brief        Stop DAC operation.  
 * @return       Return zero on successful API execution.
 */
rsi_error_t DAC_Stop(void)
{
#ifdef DAC_FIFO_MODE_EN
  dac_udma_stop();
#endif
  RSI_DAC_Stop(AUX_ADC_DAC_COMP);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t DAC_Deinit(void)
 * @brief        This API used for De-initialize DAC.  
 * @return       Return zero on successful API execution.
 */
rsi_error_t DAC_Deinit(void)
{
  // DAC power gate enable
  RSI_DAC_PowerControl(DAC_POWER_OFF);

  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           uint32_t dac_set_clock(uint32_t sampl_rate)
 * @brief        Set DAC sampling rate.
 * @param[in]    sampling_rate      : Sampling rate for DAC operation.
 * @return       Return the sampling value which configured in DAC module.
 */
uint32_t dac_set_clock(uint32_t sampl_rate)
{
  uint32_t clk_src_val = 0;
  uint16_t clk_div_fac = 0;

  // Check the AUX-Clock is enable or not
  if (ULPCLK->ULP_AUXADC_CLK_GEN_REG_b.ULP_AUX_CLK_EN_b) {
    clk_src_val = RSI_CLK_GetBaseClock(ULPSS_AUX);

    if ((clk_src_val * 2) >= sampl_rate) {
      clk_div_fac = (uint16_t)(adc_dac_ceil((float)(2 * clk_src_val) / (float)sampl_rate));
      if (clk_div_fac > 0x03FF) {
        clk_div_fac = 0x03FF;
      }
      // Configure the DAC division factor for required sampling rate
      RSI_DAC_ClkDivFactor(AUX_ADC_DAC_COMP, clk_div_fac);
    }
    return clk_src_val;
  } else {
    clk_div_fac = (uint16_t)adc_dac_ceil((float)(2 * system_clocks.ulpss_ref_clk) / (float)sampl_rate);
    if (clk_div_fac > 0x03FF) {
      clk_div_fac = 0x03FF;
    }
    // Configure the DAC division factor for required sampling rate
    RSI_DAC_ClkDivFactor(AUX_ADC_DAC_COMP, clk_div_fac);
    return ((clk_div_fac * sampl_rate) / 2);
  }
}

/*==============================================*/
/**
 * @fn           rsi_error_t DAC_PinMux(uint8_t pin_sel)
 * @brief        This API use for configure the DAC output pin in analog mode.
 * @param[in]    pin_sel      : 
 *                              - pin_sel = 1 , Configure the ULP_GPIO15 in Analog mode.
 *                              - pin_sel = 0 , Configure the ULP_GPIO4 in Analog mode.
 */
rsi_error_t DAC_PinMux(uint8_t pin_sel)
{
  if (pin_sel) {
#if !defined(SLI_SI917B0)
    RSI_EGPIO_UlpPadReceiverDisable(DAC_OUT_AGPIO15); //REN disable
    // Configure ULP_GPIO15 in analog mode
    RSI_EGPIO_SetPinMux(EGPIO1, 0, DAC_OUT_AGPIO15, EGPIO_PIN_MUX_MODE7);
#else
    RSI_EGPIO_PadReceiverDisable(TGPIO_PIN5);
    RSI_EGPIO_HostPadsGpioModeDisable(TGPIO_PIN5);
    RSI_EGPIO_PadSdioConnected();
    RSI_EGPIO_SetPinMux(EGPIO, 0, TGPIO_PIN5, TGPIO_MODE);
#endif
  } else {
    RSI_EGPIO_UlpPadReceiverDisable(DAC_OUT_AGPIO4);
    // Configure ULP_GPIO15 in analog mode
    RSI_EGPIO_SetPinMux(EGPIO1, 0, DAC_OUT_AGPIO4, EGPIO_PIN_MUX_MODE7);
  }
  return RSI_OK;
}

#ifdef SL_SI91X_DAC
/*==============================================*/
/**
 * @fn           rsi_error_t DAC_PinMux_config(uint8_t pin, uint8_t port)
 * @brief        This API use for configure the DAC output pin in analog mode for B0 boards using sl_dac driver.
 * @param[in]    pin      : Pin number
 *               port     : Port number
 */
rsi_error_t DAC_PinMux_config(uint8_t pin, uint8_t port)
{
  RSI_EGPIO_PadReceiverDisable(pin);
  RSI_EGPIO_HostPadsGpioModeDisable(pin);
  RSI_EGPIO_PadSdioConnected();
  RSI_EGPIO_SetPinMux(EGPIO, port, pin, TGPIO_MODE);
  return RSI_OK;
}
#endif

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_DAC_Config(AUX_ADC_DAC_COMP_Type *pstcDAC,uint8_t static_fifo_mode,
 *                                      uint16_t aux_dac_out_mux_en,uint16_t aux_dac_out_mux_sel,  uint8_t prbs_sel)
 * @brief        This API is used to configure DAC in static or FIFO mode.
 * @param[in]    pstcADC              : Pointer to the AUX_ADC_DAC_COMP_Type structure.  
 * @param[in]    static_fifo_mode     : This parameter define DAC in FIFO mode or static mode
 *                                      - static_fifo_mode = 0  , for Fifo mode
 *                                      - static_fifo_mode = 1  , for Static mode 
 * @param[in]    aux_dac_out_mux_en   : This parameter define DAC output play on AGPIO or not. 
 *                                      - aux_dac_out_mux_en = 0  , DAC output not play on any AGPIO.
 *                                      - aux_dac_out_mux_en = 1  , DAC output on GPIO. 
 * @param[in]    aux_dac_out_mux_sel  : This parameter define DAC output play on which AGPIO 
 *                                      - aux_dac_out_mux_sel = 0 , DAC output play on ULP_GPIO4
 *                                      - aux_dac_out_mux_sel = 1 , DAC output play on ULP_GPIO15
 * @param[in]    prbs_sel             : This parameter used for switch on or off PRBS RRBS for DAC.
 *                                      - prbs_sel = 0 ,Switch off PRBS RRBS for DAC
 *                                      - prbs_sel = 1 ,Switch on PRBS RRBS for DAC
 * @return       execution status 
 */
rsi_error_t RSI_DAC_Config(AUX_ADC_DAC_COMP_Type *pstcDAC,
                           uint8_t static_fifo_mode,
                           uint16_t aux_dac_out_mux_en,
                           uint16_t aux_dac_out_mux_sel,
                           uint8_t prbs_sel)
{
  if (prbs_sel) {
    ULP_SPI_MEM_MAP(AUXDACREG0) |= (BIT(5));
  } else {
    ULP_SPI_MEM_MAP(AUXDACREG0) &= ~(BIT(5));
  }
  pstcDAC->AUXDAC_CONIG_1_b.AUXDAC_OUT_MUX_EN  = (unsigned int)(aux_dac_out_mux_en & 0x01);
  pstcDAC->AUXDAC_CONIG_1_b.AUXDAC_OUT_MUX_SEL = (unsigned int)(aux_dac_out_mux_sel & 0x01);
  pstcDAC->AUXDAC_CTRL_1_b.DAC_STATIC_MODE     = (unsigned int)(static_fifo_mode & 0x01);
  if (static_fifo_mode == 0) {
    pstcDAC->AUXDAC_CONIG_1_b.AUXDAC_DYN_EN = 1U;
  }
#if defined(SLI_SI917)
  if (static_fifo_mode == 1) {
    pstcDAC->AUXDAC_CTRL_1_b.DAC_FIFO_AEMPTY_THRESHOLD = 0;
    pstcDAC->AUXDAC_CONIG_1_b.AUXDAC_DYN_EN            = 1U;
  }
#endif
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_DAC_DynamicModeConfig(AUX_ADC_DAC_COMP_Type *pstcDAC,uint16_t channel_no,
 *                                                uint16_t aux_dac_out_mux_en,uint16_t aux_dac_out_mux_sel)
 * @brief        This API is used to configure DAC in dynamic mode.
 * @param[in]    pstcADC              : Pointer to the AUX_ADC_DAC_COMP_Type structure. 
 * @param[in]    channel_no           : DAC channel to be configured as 0,1,2 ...15 when ADC multichannel enable is present
 * @param[in]    aux_dac_out_mux_en   : This parameter define DAC output play on AGPIO or not. 
 *                                      - aux_dac_out_mux_en = 0  , DAC output not play on any AGPIO.
 *                                      - aux_dac_out_mux_en = 1  , DAC output on GPIO. 
 * @param[in]    aux_dac_out_mux_sel  : This parameter define DAC output play on which AGPIO 
 *                                      - aux_dac_out_mux_sel = 0 , DAC output play on ULP_GPIO4
 *                                      - aux_dac_out_mux_sel = 1 , DAC output play on ULP_GPIO15
 * @return       execution status 
 */
rsi_error_t RSI_DAC_DynamicModeConfig(AUX_ADC_DAC_COMP_Type *pstcDAC,
                                      uint16_t channel_no,
                                      uint16_t aux_dac_out_mux_en,
                                      uint16_t aux_dac_out_mux_sel)
{
  pstcDAC->INTERNAL_DMA_CH_ENABLE_b.PER_CHANNEL_ENABLE |= (uint16_t)(1 << channel_no);
  pstcDAC->AUXDAC_CONIG_1_b.AUXDAC_DYN_EN = 1U;
#ifdef CHIP_9118
  pstcDAC->AUXDAC_CTRL_1_b.DAC_TO_CTRL_ADC = 1U;
#endif
  pstcDAC->ADC_CH_BIT_MAP_CONFIG[channel_no].ADC_CH_BIT_MAP_CONFIG_0_b.CHANNEL_BITMAP |=
    (uint32_t)(aux_dac_out_mux_en << 29 | aux_dac_out_mux_sel << 30);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_DAC_DynamicModeWriteData(AUX_ADC_DAC_COMP_Type *pstcDAC, uint16_t channel_no,
 *                                                    const uint16_t *data, uint32_t len)
 * @brief        This API is used to write input data DAC in dynamic mode  
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure. 
 * @param[in]    channel_no  : DAC channel to be configured as 0,1,2 ...15 when ADC multichannel enable is present
 * @param[in]    data        : Input buffer pointer
 * @param[in]    len         : length of digital samples which play in DAC .
 * @return       execution status 
 */
rsi_error_t RSI_DAC_DynamicModeWriteData(AUX_ADC_DAC_COMP_Type *pstcDAC,
                                         uint16_t channel_no,
                                         const uint16_t *data,
                                         uint32_t len)
{
  for (uint32_t i = 0; i < len; i++) {
    // clear the DAC input register in dynamic mode
    pstcDAC->ADC_CH_BIT_MAP_CONFIG[channel_no].ADC_CH_BIT_MAP_CONFIG_0_b.CHANNEL_BITMAP &= 0x7FFFFFFF;
    // Feed the input sample in dynamic mode
    pstcDAC->ADC_CH_BIT_MAP_CONFIG[channel_no].ADC_CH_BIT_MAP_CONFIG_0_b.CHANNEL_BITMAP |= (data[i]) << 31;
    // clear the DAC input register in dynamic mode
    pstcDAC->ADC_CH_BIT_MAP_CONFIG[channel_no].ADC_CH_BIT_MAP_CONFIG_1_b.CHANNEL_BITMAP &= 0xFFFFFE00;
    // Feed the input sample in dynamic mode
    pstcDAC->ADC_CH_BIT_MAP_CONFIG[channel_no].ADC_CH_BIT_MAP_CONFIG_1_b.CHANNEL_BITMAP |= (data[i]) >> 1;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_DAC_WriteData(AUX_ADC_DAC_COMP_Type *pstcDAC, const int16_t *data,
 *                               uint8_t static_fifo_mode,uint16_t len)
 * @brief        This API is used to write input data to DAC in static or in FIFO mode. 
 * @param[in]    pstcADC             : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    data                : This parameter define input data to DAC.
 * @param[in]    static_fifo_mode    : This parameter define write data to DAC in FIFO mode 
 *                                      or in static mode
 *                                      - static_fifo_mode = 1  , Write data to DAC in static mode
 *                                      - static_fifo_mode = 0  , Write data to DAC in fifo mode  
 * @param[in]    len                 : This parameter define length of data to DAC
 * @return       execution status 
 */
rsi_error_t RSI_DAC_WriteData(AUX_ADC_DAC_COMP_Type *pstcDAC,
                              const int16_t *data,
                              uint8_t static_fifo_mode,
                              uint16_t len)
{
  (void)static_fifo_mode;
  for (uint32_t i = 0; i < len; i++) {
#ifdef CHIP_9118
    if (static_fifo_mode == 0) {
      pstcDAC->AUXDAC_DATA_REG_b.AUXDAC_DATA = (*data);
      data++;
    } else {
      pstcDAC->AUXDAC_CONIG_1_b.AUXDAC_DATA_S = (*data);
      data++;
    }
#endif
#if defined(SLI_SI917)
    pstcDAC->AUXDAC_DATA_REG_b.AUXDAC_DATA = (unsigned int)((*data) & 0x03FF);
    data++;
#endif
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           uint16_t RSI_DAC_ReadData(const AUX_ADC_DAC_COMP_Type *pstcDAC)
 * @brief        This API is used to Read output data of DAC in FIFO or static mode. 
 * @param[in]    pstcADC             : Pointer to the AUX_ADC_DAC_COMP_Type structure. 
 * @param[out]   data                : Output data 
 * @return       Output data of DAC. 
 */
uint16_t RSI_DAC_ReadData(const AUX_ADC_DAC_COMP_Type *pstcDAC)
{
  uint16_t data;
  data = pstcDAC->AUXDAC_CONIG_1_b.AUXDAC_DATA_S;
  return data;
}

/*==============================================*/
/**
 * @fn           uint16_t RSI_DAC_DynamicModeReadData(const AUX_ADC_DAC_COMP_Type *pstcDAC, uint32_t channel,uint16_t data)
 * @brief        This API is used to Read output data 
 * @param[in]    pstcADC  : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel  : DAC channel to be configured as 0,1,2 ...15 when ADC multichannel enable is present 
 * @param[out]   data     : Output data in dynamic mode  
 * @return       DAC output data , where DAC in dynamic mode. 
 */
uint16_t RSI_DAC_DynamicModeReadData(const AUX_ADC_DAC_COMP_Type *pstcDAC, uint32_t channel, uint16_t data)
{
  data = (uint16_t)(pstcDAC->ADC_CH_BIT_MAP_CONFIG[channel].ADC_CH_BIT_MAP_CONFIG_1_b.CHANNEL_BITMAP);
  data = (uint16_t)(data << 1);
  data = (uint16_t)(data | (pstcDAC->ADC_CH_BIT_MAP_CONFIG[channel].ADC_CH_BIT_MAP_CONFIG_0_b.CHANNEL_BITMAP >> 31));
  return data;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_DAC_Start(AUX_ADC_DAC_COMP_Type *pstcDAC, uint16_t aux_dac_en)
 * @brief        This API is used to start the DAC operation in Static or FIFO mode.
 * @param[in]    pstcADC     : Pointer to the AUX_ADC_DAC_COMP_Type structure. 
 * @param[in]    aux_dac_en  : This parameter define DAC module start or not start.
 *                             - aux_dac_en  : 1  -> Enable DAC in static or in dynamic mode operation.
 *                             - aux_dac_en  : 0  -> Disable DAC in static or in dynamic mode operation. 
 * @return       execution status 
 */
rsi_error_t RSI_DAC_Start(AUX_ADC_DAC_COMP_Type *pstcDAC, uint16_t aux_dac_en)
{
  pstcDAC->AUXDAC_CONIG_1_b.AUXDAC_EN_S      = 1U;
  pstcDAC->AUXDAC_CTRL_1_b.ENDAC_FIFO_CONFIG = 1U;
  pstcDAC->AUXDAC_CTRL_1_b.DAC_ENABLE_F      = (unsigned int)(aux_dac_en & 0x01);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           uint32_t RSI_DAC_Stop( AUX_ADC_DAC_COMP_Type *pstcDAC )
 * @brief        This API is used to stop the DAC operation in FIFO or Static mode.
 * @param[in]    pstcADC             : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @return       execution status 
 */
rsi_error_t RSI_DAC_Stop(AUX_ADC_DAC_COMP_Type *pstcDAC)
{
  pstcDAC->AUXDAC_CONIG_1_b.AUXDAC_DYN_EN    = 0U;
  pstcDAC->AUXDAC_CONIG_1_b.AUXDAC_EN_S      = 0U;
  pstcDAC->AUXDAC_CTRL_1_b.DAC_ENABLE_F      = 0U;
  pstcDAC->AUXDAC_CTRL_1_b.ENDAC_FIFO_CONFIG = 0U;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_DAC_DynamicModeStart(AUX_ADC_DAC_COMP_Type *pstcDAC, uint32_t channel, uint16_t aux_dac_en)
 * @brief        This API is used to start the DAC in dynamic mode.
 * @param[in]    pstcADC      : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel      : DAC channel to be configured as 0,1,2 ...15 when ADC multichannel enable is present
 * @param[in]    aux_dac_en   : This parameter define DAC enable or disable.
 *                              - aux_dac_en = 0 , Disable DAC for dynamic mode operation. 
 *                              - aux_dac_en = 1 , Enable  DAC for dynamic mode operation. 
 * @return       execution status 
 */
rsi_error_t RSI_DAC_DynamicModeStart(AUX_ADC_DAC_COMP_Type *pstcDAC, uint32_t channel, uint16_t aux_dac_en)
{
  pstcDAC->AUXDAC_CONIG_1_b.AUXDAC_EN_S = 1U;
  pstcDAC->ADC_CH_BIT_MAP_CONFIG[channel].ADC_CH_BIT_MAP_CONFIG_0_b.CHANNEL_BITMAP |= aux_dac_en << 28;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_DAC_DynamicModeStop( AUX_ADC_DAC_COMP_Type *pstcDAC,uint32_t channel)
 * @brief        This API is used to stop the DAC in dynamic mode.
 * @param[in]    pstcADC  : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    channel  : DAC channel to be configured as 0,1,2 ...15 when ADC multichannel enable is present
 * @return       execution status 
 */
rsi_error_t RSI_DAC_DynamicModeStop(AUX_ADC_DAC_COMP_Type *pstcDAC, uint32_t channel)
{
  pstcDAC->AUXDAC_CONIG_1_b.AUXDAC_DYN_EN                                          = 0;
  pstcDAC->AUXDAC_CONIG_1_b.AUXDAC_EN_S                                            = 0;
  pstcDAC->ADC_CH_BIT_MAP_CONFIG[channel].ADC_CH_BIT_MAP_CONFIG_0_b.CHANNEL_BITMAP = 0 << 28;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           uint32_t RSI_DAC_ClkDivFactor(AUX_ADC_DAC_COMP_Type *pstcDAC, uint16_t div_factor)
 * @brief        This API is used to set clock with configurable on time 
 * @param[in]    pstcADC      : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    div_factor   : clock division factor to be programmed
 * @return       execution status 
 *
 */
rsi_error_t RSI_DAC_ClkDivFactor(AUX_ADC_DAC_COMP_Type *pstcDAC, uint16_t div_factor)
{
  if (div_factor > MAX_DIV_FAC_DAC) {
    return INVALID_PARAMETERS;
  }
  pstcDAC->AUXDAC_CLK_DIV_FAC_b.DAC_CLK_DIV_FAC = (unsigned int)(div_factor & 0x03FF);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           void RSI_DAC_PowerControl(POWER_STATE_DAC state)
 * @brief        This API is used to Power On and off for DAC
 * @param[in]    state : 
 *                      - DAC_POWER_ON - To powerup dac powergates
 *                      - DAC_POWER_OFF - To powerdown dac powergates
 * @return       execution status 
 *
 */
void RSI_DAC_PowerControl(POWER_STATE_DAC state)
{
  switch (state) {
    case DAC_POWER_ON:
      RSI_IPMU_PowerGateSet(AUXDAC_PG_ENB);
      RSI_PS_UlpssPeriPowerUp(ULPSS_PWRGATE_ULP_AUX);
      if (analog_get_power_state() == 0) {
        // Select ULP Ref clock for ADC
        if (!(M4_ULP_SLP_STATUS_REG & ULP_MODE_SWITCHED_NPSS)) {
          RSI_ULPSS_RefClkConfig(ULPSS_40MHZ_CLK);
        }
        RSI_ULPSS_AuxClkConfig(ULPCLK, ENABLE_STATIC_CLK, ULP_AUX_REF_CLK);
      }
      analog_set_power_state(DAC_BIT_POS, ANALOG_POWERED_ON);
      break;
    case DAC_POWER_OFF:
      RSI_IPMU_PowerGateClr(AUXDAC_PG_ENB);
      analog_set_power_state(DAC_BIT_POS, ANALOG_POWERED_OFF);
      if (!analog_get_power_state()) {
        RSI_PS_UlpssPeriPowerDown(ULPSS_PWRGATE_ULP_AUX);
        RSI_ULPSS_PeripheralDisable(ULPCLK, ULP_AUX_CLK);
      }

      break;
  }
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_DAC_SetFifoThreshold(AUX_ADC_DAC_COMP_Type *pstcDAC,uint32_t fifo_threshold)
 * @brief        This API is used to set fifo threshold for DAC.
 * @param[in]    pstcADC        : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    fifo_threshold : FIFO threshold value.
 * @return       execution status 
 *
 */
rsi_error_t RSI_DAC_SetFifoThreshold(AUX_ADC_DAC_COMP_Type *pstcDAC, uint32_t fifo_threshold)
{
  if (fifo_threshold > MAX_DAC_FIFO_THR) {
    return INVALID_PARAMETERS;
  }
  pstcDAC->AUXDAC_CTRL_1_b.DAC_FIFO_THRESHOLD = (unsigned int)(fifo_threshold & 0x07);
  pstcDAC->AUXDAC_CTRL_1_b.DAC_FIFO_FLUSH     = 1U;
  return RSI_OK;
}
/// @cond
#ifdef CHIP_9118

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_DAC_InterruptUnMask(AUX_ADC_DAC_COMP_Type *pstcDAC)
 * @brief        This API is used to unmask the DAC interrupt in FIFO mode.
 * @param[in]    pstcADC  : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @return       zero on success. 
 *
 */
rsi_error_t RSI_DAC_InterruptUnMask(AUX_ADC_DAC_COMP_Type *pstcDAC)
#endif
/// @endcond
#if defined(SLI_SI917)

  /*==============================================*/
  /**
 * @fn           rsi_error_t RSI_DAC_InterruptUnMask(AUX_ADC_DAC_COMP_Type *pstcDAC, uint8_t oper_mode)
 * @brief        This API is used to unmask the DAC interrupt in FIFO mode.
 * @param[in]    pstcADC   : Pointer to the AUX_ADC_DAC_COMP_Type structure.
 * @param[in]    oper_mode : Operation mode select. 0 for Static mode and 1 for FIFO mode. 
 * @return       zero on success. 
 *
 */
  rsi_error_t RSI_DAC_InterruptUnMask(AUX_ADC_DAC_COMP_Type *pstcDAC, uint8_t oper_mode)
#endif
{
#ifdef CHIP_9118
  pstcDAC->INTR_MASK_REG_b.DAC_FIFO_EMPTY_INTR_MASK  = 0;
  pstcDAC->INTR_MASK_REG_b.DAC_FIFO_AEMPTY_INTR_MASK = 0;
#endif
#if defined(SLI_SI917)
  /// @cond PRIVATE
  if (oper_mode == STATIC_MODE_EN) {
    pstcDAC->INTR_MASK_REG_b.DAC_STATIC_MODE_DATA_INTR_MASK = 0;
  } else {
    pstcDAC->INTR_MASK_REG_b.DAC_FIFO_EMPTY_INTR_MASK  = 0;
    pstcDAC->INTR_MASK_REG_b.DAC_FIFO_AEMPTY_INTR_MASK = 0;
  }
  /// @endcond
#endif
  return RSI_OK;
}
/// @cond
#ifdef CHIP_9118

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_DAC_InterruptMask(AUX_ADC_DAC_COMP_Type *pstcDAC)
 * @brief        This API is used to mask the DAC interrupt in FIFO mode.
 * @param[in]    pstcADC      : Pointer to the AUX_ADC_DAC_COMP_Type structure. 
 * @return       zero on success. 
 *
 */
rsi_error_t RSI_DAC_InterruptMask(AUX_ADC_DAC_COMP_Type *pstcDAC)
#endif
/// @endcond
#if defined(SLI_SI917)

  /*==============================================*/
  /**
 * @fn           rsi_error_t RSI_DAC_InterruptMask(AUX_ADC_DAC_COMP_Type *pstcDAC, uint8_t oper_mode)
 * @brief        This API is used to mask the DAC interrupt in FIFO mode.
 * @param[in]    pstcADC      : Pointer to the AUX_ADC_DAC_COMP_Type structure. 
 * @param[in]    oper_mode    : Operation mode select. 0 for Static mode and 1 for FIFO mode. 
 * @return       zero on success. 
 *
 */
  rsi_error_t RSI_DAC_InterruptMask(AUX_ADC_DAC_COMP_Type *pstcDAC, uint8_t oper_mode)
#endif
{
#ifdef CHIP_9118
  pstcDAC->INTR_MASK_REG_b.DAC_FIFO_EMPTY_INTR_MASK  = 1;
  pstcDAC->INTR_MASK_REG_b.DAC_FIFO_AEMPTY_INTR_MASK = 1;
#endif
#if defined(SLI_SI917)
  /// @cond PRIVATE
  if (oper_mode == STATIC_MODE_EN) {
    pstcDAC->INTR_MASK_REG_b.DAC_STATIC_MODE_DATA_INTR_MASK = 1;
  } else {
    pstcDAC->INTR_MASK_REG_b.DAC_FIFO_EMPTY_INTR_MASK  = 1;
    pstcDAC->INTR_MASK_REG_b.DAC_FIFO_AEMPTY_INTR_MASK = 1;
  }
  /// @endcond
#endif
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_DAC_InterruptClr(const AUX_ADC_DAC_COMP_Type *pstcDAC)
 * @brief        This API is used to clear the DAC interrupt in FIFO mode.
 * @param[in]    pstcADC      : Pointer to the AUX_ADC_DAC_COMP_Type structure. 
 * @return       zero on success. 
 *
 */
rsi_error_t RSI_DAC_InterruptClr(const AUX_ADC_DAC_COMP_Type *pstcDAC)
{
#ifdef CHIP_9118
  pstcDAC->AUXDAC_CTRL_1_b.DAC_FIFO_FLUSH = 1U;
#endif
#if defined(SLI_SI917)
  int32_t dac_data_read = 0;
  dac_data_read         = pstcDAC->AUXDAC_DATA_REG_b.AUXDAC_DATA;
#endif
  (void)dac_data_read;
  return RSI_OK;
}

#ifdef DAC_FIFO_MODE_EN

/*==============================================*/
/**
 * @fn void dac_udmaTransferComplete(RSI_UDMA_HANDLE_T udmaHandle, RSI_UDMA_DESC_T *pTranDesc, uint32_t channel_no)
 * @brief  This function is callback function to reconfigure uDMA ping and pong descriptor.
 * @param[in]  udmaHandle  :  uDMA handler
 * @param[in]  pTranDesc   :  Event type 
 * @param[in]  channel_no  :  Channel number
 * @return     None
 */
void dac_udmaTransferComplete(RSI_UDMA_HANDLE_T udmaHandle, RSI_UDMA_DESC_T *pTranDesc, uint32_t channel_no)
{
  (void)pTranDesc;
  (void)udmaHandle;
  (void)channel_no;
}

/*==============================================*/
/**
 * @fn void UDMA_DAC_Ping_Write(uint16_t num_of_samples, int16_t *input_buff, uint8_t pingreconfig)
 * @brief  This function use for configure descriptor for  primary structure.
 * @param[in]  ping_reconfig  : This parameter use for skip uDMA initialization function in 
 *                          ping descriptor reconfiguration time.
 * @param[in]  num_of_samples : Number of samples to play in DAC.
 * @param[in]  input_buff     : Input samples buffer pointer.
 * @return     None
 */
void UDMA_DAC_Ping_Write(uint16_t num_of_samples, int16_t *input_buff, uint8_t pingreconfig)
{
  RSI_UDMA_CHA_CONFIG_DATA_T control = { 0 };
  RSI_UDMA_CHA_CFG_T config          = { 0 };

  config.altStruct       = 0;
  config.burstReq        = 1;
  config.channelPrioHigh = 0;
  config.periAck         = 0;
  config.periphReq       = 0;
  config.reqMask         = 0;
  config.dmaCh           = 10;

  // Setup source to destination copy for trigger for memory
  // Descriptor configurations
  control.transferType       = UDMA_MODE_PINGPONG;
  control.nextBurst          = 0;
  control.totalNumOfDMATrans = (unsigned int)((num_of_samples - 1) & 0x03FF);
  control.rPower             = ARBSIZE_4;
  control.srcProtCtrl        = 0x000;
  control.dstProtCtrl        = 0x000;
  control.srcSize            = SRC_SIZE_16;
  control.srcInc             = SRC_INC_16;
  control.dstSize            = DST_SIZE_16;
  control.dstInc             = DST_INC_NONE;

  if (pingreconfig) {
    RSI_UDMA_SetupChannel(udmaHandle1, &config);
  }

  RSI_UDMA_SetupChannelTransfer(udmaHandle1,
                                &config,
                                control,
                                (uint32_t *)input_buff,
                                (uint32_t *)(DAC_INPUT_REG_ADDR));

  if (RSI_UDMA_ChannelIsEnabled(udmaHandle1, config.dmaCh) != RSI_OK) {
    RSI_UDMA_ChannelEnable(udmaHandle1, config.dmaCh);
  }
}

/*==============================================*/
/**
 * @fn    void UDMA_DAC_Pong_Write(uint16_t num_of_samples, int16_t *input_buff, uint8_t pongreconfig)
 * @brief  This function use for configure descriptor for alternate uDMA structure.
 * @param[in]  pong_reconfig  : This parameter use for skip uDMA initialization function in 
 *                          ping descriptor reconfiguration time.
 * @param[in]  num_of_samples : Number of samples to play in DAC.
 * @param[in]  input_buff     : Input samples buffer pointer.
 * @return None
 */
void UDMA_DAC_Pong_Write(uint16_t num_of_samples, int16_t *input_buff, uint8_t pongreconfig)
{
  RSI_UDMA_CHA_CONFIG_DATA_T control = { 0 };
  RSI_UDMA_CHA_CFG_T config          = { 0 };

  config.altStruct       = 1;
  config.burstReq        = 1;
  config.channelPrioHigh = 0;
  config.periAck         = 0;
  config.periphReq       = 0;
  config.reqMask         = 0;
  config.dmaCh           = 10;

  // Setup source to destination copy for trigger for memory
  // Descriptor configurations
  control.transferType       = UDMA_MODE_PINGPONG;
  control.nextBurst          = 0;
  control.totalNumOfDMATrans = (unsigned int)((num_of_samples - 1) & 0x03FF);
  control.rPower             = ARBSIZE_4;
  control.srcProtCtrl        = 0x000;
  control.dstProtCtrl        = 0x000;
  control.srcSize            = SRC_SIZE_16;
  control.srcInc             = SRC_INC_16;
  control.dstSize            = DST_SIZE_16;
  control.dstInc             = DST_INC_NONE;

  if (pongreconfig) {
    RSI_UDMA_SetupChannel(udmaHandle1, &config);
  }
  RSI_UDMA_SetupChannelTransfer(udmaHandle1,
                                &config,
                                control,
                                (uint32_t *)input_buff,
                                (uint32_t *)(DAC_INPUT_REG_ADDR));

  if (RSI_UDMA_ChannelIsEnabled(udmaHandle1, config.dmaCh) != RSI_OK) {
    RSI_UDMA_ChannelEnable(udmaHandle1, config.dmaCh);
  }
}

/*==============================================*/
/**
 * @fn    void dac_udma_init(void)
 * @brief  This API use for Init ULP-uDMA.
 * @return None
 */
void dac_udma_init(void)
{
  RSI_UDMA_INIT_T udmaInit;
  volatile bool done1 = NULL;

  RSI_ULPSS_PeripheralEnable(ULPCLK, ULP_UDMA_CLK, ENABLE_STATIC_CLK);

  RSI_PS_UlpssPeriPowerUp(ULPSS_PWRGATE_ULP_UDMA);

  udmaInit.pUserData = (void *)&done1;
  udmaInit.base      = (uint32_t)UDMA1;
  udmaInit.sramBase  = (uint32_t)&UDMA1_Table;
  udmaHandle1        = RSI_UDMA_Init(devMem, &udmaInit);

  RSI_UDMA_RegisterCallback(udmaHandle1, (void *)dac_udmaTransferComplete);
}

/*==============================================*/
/**
 * @fn void dac_udma_stop(void)
 * @brief  This API use for stop/disable the uDMA channel-10.
 * @return None
 */
void dac_udma_stop(void)
{
  RSI_UDMA_ChannelDisable(udmaHandle1, 10);
}
/*==============================================*/
/**
 * @fn void dac_udma_start(void)
 * @brief  This API use for trigger uDMA channel10 for DAC operation.
 * @return None
 */
void dac_udma_start(void)
{
  RSI_UDMA_UDMAEnable(udmaHandle1);

  // uDMA NVIC enable
  NVIC_EnableIRQ(UDMA1_IRQn);
}

/*==============================================*/
/**
 * @fn  void dac_udma_write(uint8_t ping_pong_write, uint16_t num_of_samples, int16_t *input_buff, uint8_t skip_flag)
 * @brief  Configure ping or pong descriptor address.
 * @param[in]  ping_pong_write  - Initialize Ping descriptor
 * @param[in]  num_of_samples   - Number of samples to play in DAC
 * @param[in]  input_buff       - Input samples buffer pointer
 * @param[in]  skip_flag        - Skips the flag
 * @return     None
 */
void dac_udma_write(uint8_t ping_pong_write, uint16_t num_of_samples, int16_t *input_buff, uint8_t skip_flag)
{
  if (ping_pong_write) {
    // Initialize Ping descriptor
    UDMA_DAC_Ping_Write(num_of_samples, input_buff, skip_flag);
  } else {
    // Initialize Pong descriptor
    UDMA_DAC_Pong_Write(num_of_samples, input_buff, skip_flag);
  }
}
#endif
/** @} */
