/*******************************************************************************
* @file  rsi_ulpss_clk.c
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

// Includes

#include "rsi_ccp_user_config.h"
#include "rsi_rom_ulpss_clk.h"
#ifndef ULPSS_CLOCK_ROMDRIVER_PRESENT
/** @addtogroup SOC3
* @{
*/
/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_clock_config(M4CLK_Type *pCLK, boolean_t clkEnable, uint16_t divFactor, boolean_t oddDivFactor)
 * @brief     This API is used to select the ULPSS processor clock source when input is soc clk source which is greater than 100MHz
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] clkEnable  : clock enable for clock source
 * @param[in] divFactor : division factor for clock to configure Reg 4
 * @param[in] oddDivFactor : odd divison factor  for clock to configure Reg 5
 * @return    RSI_OK on success 
 */

rsi_error_t ulpss_clock_config(M4CLK_Type *pCLK, boolean_t clkEnable, uint16_t divFactor, boolean_t oddDivFactor)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  if (clkEnable == Enable) {
    pCLK->CLK_CONFIG_REG4_b.ULPSS_CLK_DIV_FAC = (unsigned int)(divFactor & 0x3F);
    pCLK->CLK_CONFIG_REG5_b.ULPSS_ODD_DIV_SEL = (unsigned int)(oddDivFactor & 0x01);
    pCLK->CLK_ENABLE_SET_REG1                 = ULPSS_CLK_ENABLE;
  } else {
    pCLK->CLK_ENABLE_CLEAR_REG1 = ULPSS_CLK_ENABLE;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_ulp_peri_clk_enable(ULPCLK_Type *pULPCLK, uint32_t u32Flags)
 * @brief     This API is used to enable different peripherial clocks in ULPSS
 * @param[in] pULPCLK : pointer to the processor ULP clock 
 * @param[in] u32Flags  : flags for perpheral clocks
 * @return    RSI_OK on success
 */

rsi_error_t ulpss_ulp_peri_clk_enable(ULPCLK_Type *pULPCLK, uint32_t u32Flags)
{
  if (pULPCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pULPCLK->ULP_MISC_SOFT_SET_REG = (pULPCLK->ULP_MISC_SOFT_SET_REG | u32Flags) & 0xFFFFFFFF;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_ulp_peri_clk_disable(ULPCLK_Type *pULPCLK, uint32_t u32Flags)
 * @brief     This API is used to disable different peripherial clocks in ULPSS
 * @param[in] pULPCLK : pointer to the processor ULP clock source
 * @param[in] u32Flags  : flags for peripheral clocks
 * @return    RSI_OK on success
 */

rsi_error_t ulpss_ulp_peri_clk_disable(ULPCLK_Type *pULPCLK, uint32_t u32Flags)
{
  if (pULPCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pULPCLK->ULP_MISC_SOFT_SET_REG = (pULPCLK->ULP_MISC_SOFT_SET_REG & ~u32Flags) & 0xFFFFFFFF;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_ulp_dyn_clk_enable(ULPCLK_Type *pULPCLK, uint32_t u32Flags)
 * @brief     This API is used to enable different dynamic clocks in ULPSS
 * @param[in] pULPCLK : pointer to the processor ULP clock source
 * @param[in] u32Flags  : flags for dynamic clocks
 * @return    RSI_OK on success
 */

rsi_error_t ulpss_ulp_dyn_clk_enable(ULPCLK_Type *pULPCLK, uint32_t u32Flags)
{
  if (pULPCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pULPCLK->ULP_DYN_CLK_CTRL_DISABLE = (pULPCLK->ULP_DYN_CLK_CTRL_DISABLE | u32Flags) & 0xFFFFFFFF;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_ulp_dyn_clk_disable(ULPCLK_Type *pULPCLK, uint32_t u32Flags)
 * @brief     This API is used to disable different dynamic clocks in ULPSS
 * @param[in] pULPCLK : pointer to the processor ULP clock source
 * @param[in] u32Flags  : flags for dynamic clocks
 * @return    RSI_OK on success
 */

rsi_error_t ulpss_ulp_dyn_clk_disable(ULPCLK_Type *pULPCLK, uint32_t u32Flags)
{
  if (pULPCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pULPCLK->ULP_DYN_CLK_CTRL_DISABLE = (pULPCLK->ULP_DYN_CLK_CTRL_DISABLE & ~u32Flags) & 0xFFFFFFFF;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t ulpss_ulp_ssi_clk_config(ULPCLK_Type *pULPCLK,
 *                                CLK_ENABLE_T clkType,
 *                                ULP_SSI_CLK_SELECT_T clkSource,
 *                                uint16_t divFactor)
 * @brief      This API is used to configure the SSI clock source
 * @param[in]  pULPCLK : pointer to the processor ULP clock source
 * @param[in]  clkType  : clock type for SSI clock source
 * @param[in]  clkSource  : clock source for ULP SSI
 * @param[in]  divFactor  : divison factor for ULP SSI 
 * @return     RSI_OK on success 
 */

rsi_error_t ulpss_ulp_ssi_clk_config(ULPCLK_Type *pULPCLK,
                                     CLK_ENABLE_T clkType,
                                     ULP_SSI_CLK_SELECT_T clkSource,
                                     uint16_t divFactor)
{
  /*Parameter validation */
  if ((pULPCLK == NULL) || (divFactor > ULP_SSI_MAX_DIVISION_FACTOR) || (clkSource > ULP_SSI_MAX_SEL)) {
    return INVALID_PARAMETERS;
  }

  ulpss_peripheral_disable(pULPCLK, ULP_SSI_CLK);
  /*Select */
  switch (clkSource) {
    /*0: ref_clk (output of dynamic clock MUX for different possible ref_clk sources)*/
    case ULP_SSI_REF_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_40MHZ_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_I2C_SSI_CLK_GEN_REG_b.ULP_SSI_CLK_SEL = clkSource;
      break;
      /*1: ulp_32khz_ro_clk */
    case ULP_SSI_ULP_32KHZ_RO_CLK: /* 32KHZ RO clock not supported */
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RO_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_I2C_SSI_CLK_GEN_REG_b.ULP_SSI_CLK_SEL = clkSource;
      break;
      /*2: ulp_32khz_rc_clk*/
    case ULP_SSI_ULP_32KHZ_RC_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_I2C_SSI_CLK_GEN_REG_b.ULP_SSI_CLK_SEL = clkSource;
      break;
      /*3: ulp_32khz_xtal_clk*/
    case ULP_SSI_ULP_32KHZ_XTAL_CLK:
      /*Enable clock*/
      /*NOTE: In order to enable the Xtal clk source need to configure the NPSS_GPIO pins
    	which can be done through clk_xtal_clk_config(uint8_t xtalPin) API i.e we need to call that API first*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_XTAL_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      pULPCLK->ULP_I2C_SSI_CLK_GEN_REG_b.ULP_SSI_CLK_SEL = clkSource;
      break;
      /*4: ulp_mhz_rc_clk*/
    case ULP_SSI_ULP_MHZ_RC_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_MHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_I2C_SSI_CLK_GEN_REG_b.ULP_SSI_CLK_SEL = clkSource;
      break;
      /*5: ulp_20mhz_ro_clk*/
    case ULP_SSI_ULP_20MHZ_RO_CLK: /* 20MHZ RO clock not supported */
      ulpss_enable_ref_clks(MCU_ULP_20MHZ_RING_OSC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_I2C_SSI_CLK_GEN_REG_b.ULP_SSI_CLK_SEL = clkSource;
      break;
    case ULP_SSI_SOC_CLK:
      /*6: soc_clk*/
      pULPCLK->ULP_I2C_SSI_CLK_GEN_REG_b.ULP_SSI_CLK_SEL = clkSource;
      break;

    default:
      return INVALID_PARAMETERS;
  }

  /* Wait for clock switched */
  while (pULPCLK->CLOCK_STAUS_REG_b.CLOCK_SWITCHED_SSI_b != 1)
    ;

  /*Update the division factor */
  pULPCLK->ULP_I2C_SSI_CLK_GEN_REG_b.ULP_SSI_CLK_DIV_FACTOR = (unsigned int)(divFactor & 0x7F);

  ulpss_peripheral_enable(pULPCLK, ULP_SSI_CLK, clkType);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_ulp_i2s_clk_config(ULPCLK_Type *pULPCLK, ULP_I2S_CLK_SELECT_T clkSource, uint16_t divFactor)
 * @brief     This API is used to configure the I2S clock source
 * @param[in] pULPCLK : pointer to the processor ULP clock source
 * @param[in] clkType  : clock type for ULP I2S
 * @param[in] clkSource  : clock source for ULP I2S
 * @param[in] divFactor  : divison factor for ULP I2S
 * @return    RSI_OK on success 
 */

rsi_error_t ulpss_ulp_i2s_clk_config(ULPCLK_Type *pULPCLK, ULP_I2S_CLK_SELECT_T clkSource, uint16_t divFactor)
{
  /*Parameter validation */
  if ((pULPCLK == NULL) || (divFactor > ULP_I2S_MAX_DIVISION_FACTOR) || (clkSource > ULP_I2S_MAX_SEL)) {
    return INVALID_PARAMETERS;
  }
  ulpss_peripheral_disable(pULPCLK, ULP_I2S_CLK);
  /*Select */
  switch (clkSource) {
    /*0: ref_clk (output of dynamic clock MUX for different possible ref_clk sources)*/
    case ULP_I2S_REF_CLK:
      /*Select clock MUX */
      pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_CLK_SEL_b = clkSource;
      break;
      /*1: ulp_32khz_ro_clk */
    case ULP_I2S_ULP_32KHZ_RO_CLK: /* 32KHZ RO clock not supported */
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RO_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_CLK_SEL_b = clkSource;
      break;
      /*2: ulp_32khz_rc_clk*/
    case ULP_I2S_ULP_32KHZ_RC_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_CLK_SEL_b = clkSource;
      break;
      /*3: ulp_32khz_xtal_clk*/
    case ULP_I2S_ULP_32KHZ_XTAL_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_XTAL_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_CLK_SEL_b = clkSource;
      break;
      /*4: ulp_mhz_rc_clk*/
    case ULP_I2S_ULP_MHZ_RC_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_MHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_CLK_SEL_b = clkSource;
      break;
      /*5: ulp_20mhz_ro_clk*/
    case ULP_I2S_ULP_20MHZ_RO_CLK: /* 20MHZ RO clock not supported */
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_20MHZ_RING_OSC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_CLK_SEL_b = clkSource;
      break;
    case ULP_I2S_SOC_CLK:
      /*6: soc_clk*/
      pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_CLK_SEL_b = clkSource;
      break;
    case ULP_I2S_ULP_DOUBLER_CLK:
      /*7: ulp_doubler_clk*/
      ulpss_enable_ref_clks(MCU_ULP_DOUBLER_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_CLK_SEL_b = clkSource;
      break;

    case ULP_I2S_PLL_CLK:
      /*I2s clock*/
      if (M4CLK->PLL_STAT_REG_b.I2SPLL_LOCK == 0) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      /*NOTE: this clock source is not valid in PS2 state. PLL is turned off in PS2*/
      M4CLK->CLK_CONFIG_REG5_b.I2S_CLK_SEL = 0;
      /*Select clock MUX */
      pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_CLK_SEL_b = clkSource;
      break;
    default:
      return INVALID_PARAMETERS;
  }

  /* Wait for clock switched */
  while (pULPCLK->CLOCK_STAUS_REG_b.CLOCK_SWITCHED_I2S_CLK_b != 1U)
    ;

  /*Set the division factor */
  pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_CLKDIV_FACTOR = (uint8_t)divFactor;

  ulpss_peripheral_enable(pULPCLK, ULP_I2S_CLK, ENABLE_STATIC_CLK);

  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t ulpss_ulp_uar_clk_config(ULPCLK_Type *pULPCLK,
 *                                CLK_ENABLE_T clkType,
 *                                boolean_t bFrClkSel,
 *                                ULP_UART_CLK_SELECT_T clkSource,
 *                                uint16_t divFactor)
 * @brief      This API is used to configure the UART clock source
 * @param[in]  pULPCLK : pointer to the processor ULP clock source
 * @param[in]  clkType  : clock type for UART
 * @param[in]  bFrClkSel : fractional clock select for ULP UART
 * @param[in]  clkSource  : clock source for ULP UART
 * @param[in]  divFactor  : divison factor for ULP UART
 * @return     RSI_OK on success 
 */

rsi_error_t ulpss_ulp_uar_clk_config(ULPCLK_Type *pULPCLK,
                                     CLK_ENABLE_T clkType,
                                     boolean_t bFrClkSel,
                                     ULP_UART_CLK_SELECT_T clkSource,
                                     uint16_t divFactor)
{
  /*Parameter validation */
  if ((pULPCLK == NULL) || (divFactor > ULP_UART_MAX_DIVISION_FACTOR) || (clkSource > ULP_UART_MAX_SEL)) {
    return INVALID_PARAMETERS;
  }
  ulpss_peripheral_disable(pULPCLK, ULP_UART_CLK);
  /*UART Fractional clock select */
  pULPCLK->ULP_UART_CLK_GEN_REG_b.ULP_UART_FRAC_CLK_SEL_b = (unsigned int)(bFrClkSel & 0x01);

  /*Select */
  switch (clkSource) {
    /*0: ref_clk (output of dynamic clock MUX for different possible ref_clk sources)*/
    case ULP_UART_REF_CLK:
      /*Configure the ULPSS reference clock from NPSS clock MUX this is common for all these Sources */
      /*Select clock MUX */
      pULPCLK->ULP_UART_CLK_GEN_REG_b.ULP_UART_CLK_SEL = clkSource;
      break;
      /*1: ulp_32khz_ro_clk */
    case ULP_UART_ULP_32KHZ_RO_CLK: /* 32KHZ RO clock not supported */
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RO_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_UART_CLK_GEN_REG_b.ULP_UART_CLK_SEL = clkSource;
      break;
      /*2: ulp_32khz_rc_clk*/
    case ULP_UART_ULP_32KHZ_RC_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_UART_CLK_GEN_REG_b.ULP_UART_CLK_SEL = clkSource;
      break;
      /*3: ulp_32khz_xtal_clk*/
    case ULP_UART_ULP_32KHZ_XTAL_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_XTAL_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      pULPCLK->ULP_UART_CLK_GEN_REG_b.ULP_UART_CLK_SEL = clkSource;
      break;
      /*4: ulp_mhz_rc_clk*/
    case ULP_UART_ULP_MHZ_RC_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_MHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_UART_CLK_GEN_REG_b.ULP_UART_CLK_SEL = clkSource;
      break;
      /*5: ulp_20mhz_ro_clk*/
    case ULP_UART_ULP_20MHZ_RO_CLK: /* 20MHZ RO clock not supported */
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_20MHZ_RING_OSC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_UART_CLK_GEN_REG_b.ULP_UART_CLK_SEL = clkSource;
      break;
    case ULP_UART_SOC_CLK:
      /*6: soc_clk*/
      pULPCLK->ULP_UART_CLK_GEN_REG_b.ULP_UART_CLK_SEL = clkSource;
      break;
    case ULP_UART_ULP_DOUBLER_CLK:
      /*7: ulp_doubler_clk*/
      ulpss_enable_ref_clks(MCU_ULP_DOUBLER_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_UART_CLK_GEN_REG_b.ULP_UART_CLK_SEL = clkSource;
      break;
    default:
      return INVALID_PARAMETERS;
  }

  /* Wait for clock switched */
  while (pULPCLK->CLOCK_STAUS_REG_b.CLOCK_SWITCHED_UART_CLK_b != 1U)
    ;

  pULPCLK->ULP_UART_CLK_GEN_REG_b.ULP_UART_CLKDIV_FACTOR = (unsigned int)(divFactor & 0x07);

  ulpss_peripheral_enable(pULPCLK, ULP_UART_CLK, clkType);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t ulpss_time_clk_config(ULPCLK_Type *pULPCLK,
 *                             CLK_ENABLE_T clkType,
 *                             boolean_t bTmrSync,
 *                             ULP_TIMER_CLK_SELECT_T clkSource,
 *                             uint8_t skipSwitchTime)
 * @brief      This API is used to configure the timer clock source
 * @param[in]  pULPCLK : pointer to the processor ULP clock source
 * @param[in]  clkType  : clock type for timer
 * @param[in]  bTmrSync : Timer Synchronisation for ULP timer
 * @param[in]  clkSource  : clock source for ULP timer
 * @param[in]  skipSwitchTime  : wait for clock switched for ULP timer
 * @return     RSI_OK on success
 */

rsi_error_t ulpss_time_clk_config(ULPCLK_Type *pULPCLK,
                                  CLK_ENABLE_T clkType,
                                  boolean_t bTmrSync,
                                  ULP_TIMER_CLK_SELECT_T clkSource,
                                  uint8_t skipSwitchTime)
{
  /*Parameter validation */
  if ((pULPCLK == NULL) || (clkSource > ULP_TIMER_MAX_SEL)) {
    return INVALID_PARAMETERS;
  }
  /*Timer PCLK enable */
  ulpss_peripheral_disable(pULPCLK, ULP_TIMER_CLK);

  if (bTmrSync) {
    /*Enable m4 core clock*/
    pULPCLK->ULP_TIMER_CLK_GEN_REG_b.ULP_TIMER_IN_SYNC_b = 1;
  } else {
    pULPCLK->ULP_TIMER_CLK_GEN_REG_b.ULP_TIMER_IN_SYNC_b = 0;
  }
  /*clock select*/
  switch (clkSource) {
    case ULP_TIMER_REF_CLK:
      /*Select clock MUX */
      pULPCLK->ULP_TIMER_CLK_GEN_REG_b.ULP_TIMER_CLK_SEL = clkSource;
      break;
      /*1: ulp_32khz_ro_clk */
    case ULP_TIMER_32KHZ_RO_CLK: /* 32KHZ RO clock not supported */
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RO_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_TIMER_CLK_GEN_REG_b.ULP_TIMER_CLK_SEL = clkSource;
      break;
      /*2: ulp_32khz_rc_clk*/
    case ULP_TIMER_32KHZ_RC_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_TIMER_CLK_GEN_REG_b.ULP_TIMER_CLK_SEL = clkSource;
      break;
      /*3: ulp_32khz_xtal_clk*/
    case ULP_TIMER_32KHZ_XTAL_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_XTAL_CLK_EN, ULP_PERIPHERAL_CLK, 0);

      pULPCLK->ULP_TIMER_CLK_GEN_REG_b.ULP_TIMER_CLK_SEL = clkSource;
      break;
      /*4: ulp_mhz_rc_clk*/
    case ULP_TIMER_MHZ_RC_CLK:
      ulpss_enable_ref_clks(MCU_ULP_MHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0);

      pULPCLK->ULP_TIMER_CLK_GEN_REG_b.ULP_TIMER_CLK_SEL = clkSource;
      break;
      /*5: ulp_20mhz_ro_clk*/
    case ULP_TIMER_20MHZ_RO_CLK: /* 20MHZ RO clock not supported */
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_20MHZ_RING_OSC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_TIMER_CLK_GEN_REG_b.ULP_TIMER_CLK_SEL = clkSource;
      break;
    case ULP_TIMER_ULP_SOC_CLK:
      /*6: soc_clk*/
      pULPCLK->ULP_TIMER_CLK_GEN_REG_b.ULP_TIMER_CLK_SEL = clkSource;
      break;
    default:
      return INVALID_PARAMETERS;
  }
  if (skipSwitchTime == 1) {
    /* Wait for clock switched */
    while (pULPCLK->CLOCK_STAUS_REG_b.CLOCK_SWITCHED_TIMER_b != 1U)
      ;
  }
  ulpss_peripheral_enable(pULPCLK, ULP_TIMER_CLK, clkType);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_aux_clk_config(ULPCLK_Type *pULPCLK, CLK_ENABLE_T clkType, ULP_AUX_CLK_SELECT_T clkSource)
 * @brief     This API is used to configure the AUX clock source
 * @param[in] pULPCLK : pointer to the processor ULP clock source
 * @param[in] clkType  : clock type for AUX
 * @param[in] clkSource  : clock source for AUX
 * @return    RSI_OK on success 
 */

rsi_error_t ulpss_aux_clk_config(ULPCLK_Type *pULPCLK, CLK_ENABLE_T clkType, ULP_AUX_CLK_SELECT_T clkSource)
{
  /*Parameter validation */
  if ((pULPCLK == NULL) || (clkSource > ULP_AUX_MAX_SEL)) {
    return INVALID_PARAMETERS;
  }
  ulpss_peripheral_disable(pULPCLK, ULP_AUX_CLK);

  /*select clock MUX */

  /*Select */
  switch (clkSource) {
    /*0: ref_clk (output of dynamic clock MUX for different possible ref_clk sources)*/
    case ULP_AUX_REF_CLK:
      /*Select clock MUX */
      pULPCLK->ULP_AUXADC_CLK_GEN_REG_b.ULP_AUX_CLK_SEL = clkSource;
      break;
      /*1: ulp_32khz_ro_clk */
    case ULP_AUX_32KHZ_RO_CLK: /* 32KHZ RO clock not supported */
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RO_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_AUXADC_CLK_GEN_REG_b.ULP_AUX_CLK_SEL = clkSource;
      break;
      /*2: ulp_32khz_rc_clk*/
    case ULP_AUX_32KHZ_RC_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_AUXADC_CLK_GEN_REG_b.ULP_AUX_CLK_SEL = clkSource;
      break;
      /*3: ulp_32khz_xtal_clk*/
    case ULP_AUX_32KHZ_XTAL_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_XTAL_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      pULPCLK->ULP_AUXADC_CLK_GEN_REG_b.ULP_AUX_CLK_SEL = clkSource;
      break;
      /*4: ulp_mhz_rc_clk*/
    case ULP_AUX_MHZ_RC_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_MHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_AUXADC_CLK_GEN_REG_b.ULP_AUX_CLK_SEL = clkSource;
      break;
      /*5: ulp_20mhz_ro_clk*/
    case ULP_AUX_20MHZ_RO_CLK: /* 20MHZ RO clock not supported */
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_20MHZ_RING_OSC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_AUXADC_CLK_GEN_REG_b.ULP_AUX_CLK_SEL = clkSource;
      break;
    case ULP_AUX_ULP_SOC_CLK:
      /*6: soc_clk*/
      pULPCLK->ULP_AUXADC_CLK_GEN_REG_b.ULP_AUX_CLK_SEL = clkSource;
      break;
    case ULP_AUX_ULP_DOUBLER_CLK:
      /*7: ulp_doubler_clk*/
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_DOUBLER_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_AUXADC_CLK_GEN_REG_b.ULP_AUX_CLK_SEL = clkSource;
      break;
    case ULP_AUX_I2S_PLL_CLK:
      /*8: i2s_pll_clk*/
      if (M4CLK->PLL_STAT_REG_b.I2SPLL_LOCK == 0) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      /*Enable clock*/
      M4CLK->CLK_CONFIG_REG5_b.I2S_CLK_SEL = 0;
      /*Select clock MUX */
      pULPCLK->ULP_AUXADC_CLK_GEN_REG_b.ULP_AUX_CLK_SEL = clkSource;
      break;
    default:
      return INVALID_PARAMETERS;
  }

  /* wait for clock switched */
  while (pULPCLK->CLOCK_STAUS_REG_b.CLOCK_SWITCHED_AUXADC_b != 1U)
    ;

  ulpss_peripheral_enable(pULPCLK, ULP_AUX_CLK, clkType);

  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_vad_clk_config(ULPCLK_Type *pULPCLK,
 *                            ULP_VAD_CLK_SELECT_T clkSource,
 *                            ULP_VAD_FCLK_SELECT_T FclkSource,
 *                            uint16_t divFactor)
 * @brief     This API is used to configure the VAD clock source
 * @param[in] pULPCLK : pointer to the processor ULP clock source
 * @param[in] clkSource  : clock source for ULP VAD
 * @param[in] FclkSource : fast clock cource for ULP VAD
 * @param[in] divFactor : divison factor for ULP VAD
 * @return    RSI_OK on success 
 */

rsi_error_t ulpss_vad_clk_config(ULPCLK_Type *pULPCLK,
                                 ULP_VAD_CLK_SELECT_T clkSource,
                                 ULP_VAD_FCLK_SELECT_T FclkSource,
                                 uint16_t divFactor)
{
  /* Parameter validation */
  if (pULPCLK == NULL || divFactor >= ULP_VAD_MAX_DIVISION_FACTOR || clkSource > ULP_VAD_MAX_SEL
      || FclkSource > ULP_VAD_FCLK_MAX_SEL) {
    return INVALID_PARAMETERS;
  }

  ulpss_peripheral_disable(pULPCLK, ULP_VAD_CLK);
  /*Select the VAD clock MUX */
  switch (clkSource) {
    case ULP_VAD_32KHZ_RO_CLK: /* 32KHZ RO clock not supported */
      pULPCLK->ULP_VAD_CLK_GEN_REG_b.ULP_VAD_CLK_SEL = clkSource;
      break;
    case ULP_VAD_32KHZ_RC_CLK:
      /*1:ulp_32khz_rc_clk */
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      pULPCLK->ULP_VAD_CLK_GEN_REG_b.ULP_VAD_CLK_SEL = clkSource;
      break;
    case ULP_VAD_32KHZ_XTAL_CLK:
      /*2: ulp_32khz_xtal_clk*/
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_XTAL_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      pULPCLK->ULP_VAD_CLK_GEN_REG_b.ULP_VAD_CLK_SEL = clkSource;
      break;
    default:
      return INVALID_PARAMETERS;
  }
  /*Select the VAD Fast clock MUX */
  switch (FclkSource) {
    case ULP_VAD_ULP_PROCESSOR_CLK:
      /* ulpss processor clock */
      pULPCLK->ULP_VAD_CLK_GEN_REG_b.ULP_VAD_FCLK_SEL = FclkSource;
      break;
    case ULP_VAD_REF_CLK:
      /*1: ref_clk */
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_40MHZ_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      pULPCLK->ULP_VAD_CLK_GEN_REG_b.ULP_VAD_FCLK_SEL = FclkSource;
      break;
    case ULP_VAD_MHZ_RC_CLK:
      /*2: ulp_mhz_rc_clk*/
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_MHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      pULPCLK->ULP_VAD_CLK_GEN_REG_b.ULP_VAD_FCLK_SEL = FclkSource;
      break;
    case ULP_VAD_20MHZ_RO_CLK: /* 20MHZ RO clock not supported */
      /*3: ulp_20mhz_ro_clk*/
      ulpss_enable_ref_clks(MCU_ULP_20MHZ_RING_OSC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      pULPCLK->ULP_VAD_CLK_GEN_REG_b.ULP_VAD_FCLK_SEL = FclkSource;
      break;
    case ULP_VAD_ULP_SOC_CLK:
      /*4: soc_clk*/
      pULPCLK->ULP_VAD_CLK_GEN_REG_b.ULP_VAD_FCLK_SEL = FclkSource;
      break;
    default:
      return INVALID_PARAMETERS;
  }

  /*wait for clock switched */
  while (pULPCLK->CLOCK_STAUS_REG_b.CLOCK_SWITCHED_FCLK_VAD_b != 1U)
    ;
  while (pULPCLK->CLOCK_STAUS_REG_b.CLOCK_SWITCHED_SCLK_VAD_b != 1U)
    ;
  while (pULPCLK->CLOCK_STAUS_REG_b.CLOCK_SWITCHED_VAD_b != 1U)
    ;
  /*Set VAD clock division factor */
  pULPCLK->ULP_VAD_CLK_GEN_REG_b.ULP_VAD_CLKDIV_FACTOR = (uint8_t)divFactor;

  ulpss_peripheral_enable(pULPCLK, ULP_VAD_CLK, ENABLE_STATIC_CLK);

  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_touch_clk_config(ULPCLK_Type *pULPCLK, ULP_TOUCH_CLK_SELECT_T clkSource, uint16_t divFactor)
 * @brief     This API is used to configure the Touch clock source
 * @param[in] pULPCLK : pointer to the processor ULP clock source
 * @param[in] clkSource  : clock source for ULP Touch
 * @param[in] divFactor : divison factor for ULP Touch
 * @return    RSI_OK on success
 */

rsi_error_t ulpss_touch_clk_config(ULPCLK_Type *pULPCLK, ULP_TOUCH_CLK_SELECT_T clkSource, uint16_t divFactor)
{
  /*Parameter validation */
  if ((pULPCLK == NULL) || (divFactor > ULP_TOUCH_MAX_DIVISION_FACTOR) || (clkSource > ULP_TOUCH_MAX_SEL)) {
    return INVALID_PARAMETERS;
  }
  ulpss_peripheral_disable(pULPCLK, ULP_TOUCH_CLK);

  /*Select */
  switch (clkSource) {
    /*0: ref_clk (output of dynamic clock MUX for different possible ref_clk sources)*/
    case ULP_TOUCH_REF_CLK:
      /*Select clock MUX */
      pULPCLK->ULP_TOUCH_CLK_GEN_REG_b.ULP_TOUCH_CLK_SEL = clkSource;
      break;
      /*1: ulp_32khz_ro_clk */
    case ULP_TOUCH_32KHZ_RO_CLK: /* 32KHZ RO clock not supported */
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RO_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_TOUCH_CLK_GEN_REG_b.ULP_TOUCH_CLK_SEL = clkSource;
      break;
      /*2: ulp_32khz_rc_clk*/
    case ULP_TOUCH_32KHZ_RC_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_TOUCH_CLK_GEN_REG_b.ULP_TOUCH_CLK_SEL = clkSource;
      break;
      /*3: ulp_32khz_xtal_clk*/
    case ULP_TOUCH_32KHZ_XTAL_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_XTAL_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      pULPCLK->ULP_TOUCH_CLK_GEN_REG_b.ULP_TOUCH_CLK_SEL = clkSource;
      break;
      /*4: ulp_mhz_rc_clk*/
    case ULP_TOUCH_MHZ_RC_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_MHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_TOUCH_CLK_GEN_REG_b.ULP_TOUCH_CLK_SEL = clkSource;
      break;
      /*5: ulp_20mhz_ro_clk*/
    case ULP_TOUCH_20MHZ_RO_CLK: /* 20MHZ RO clock not supported */
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_20MHZ_RING_OSC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      /*Select clock MUX */
      pULPCLK->ULP_TOUCH_CLK_GEN_REG_b.ULP_TOUCH_CLK_SEL = clkSource;
      break;
    case ULP_TOUCH_ULP_SOC_CLK:
      /*6: soc_clk*/
      pULPCLK->ULP_TOUCH_CLK_GEN_REG_b.ULP_TOUCH_CLK_SEL = clkSource;
      break;

    default:
      return INVALID_PARAMETERS;
  }

  /*Wait for clock switched */
  while ((pULPCLK->CLOCK_STAUS_REG_b.CLOCK_SWITCHED_TOUCH_SENSOR_b) != 1)
    ;

  /*Program the division factor */
  pULPCLK->ULP_TOUCH_CLK_GEN_REG_b.ULP_TOUCH_CLKDIV_FACTOR = (uint8_t)divFactor;

  ulpss_peripheral_enable(pULPCLK, ULP_TOUCH_CLK, ENABLE_STATIC_CLK);

  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_slp_sensor_clk_config(ULPCLK_Type *pULPCLK, boolean_t clkEnable, uint32_t divFactor)
 * @brief     This API is used to configure the sleep sensor clock source
 * @param[in] pULPCLK : pointer to the processor ULP clock source
 * @param[in] clkEnable  : enable clock for ULP sleep sensor
 * @param[in] divFactor : divison factor for ULP sleep sensor
 * @return    RSI_OK on success
 */

rsi_error_t ulpss_slp_sensor_clk_config(ULPCLK_Type *pULPCLK, boolean_t clkEnable, uint32_t divFactor)
{
  /*Parameter validation */
  if ((pULPCLK == NULL) || (divFactor > ULP_SLP_SENSOR_MAX_DIVISION_FACTOR)) {
    return INVALID_PARAMETERS;
  }
  if (clkEnable) {
    /*Enable sleep sensor clock */
    pULPCLK->SLP_SENSOR_CLK_REG_b.ENABLE_b = 1;
  } else {
    /*Disable sleep sensor clock */
    pULPCLK->SLP_SENSOR_CLK_REG_b.ENABLE_b = 0;
  }
  /*Apply division factor*/
  pULPCLK->SLP_SENSOR_CLK_REG_b.DIVISON_FACTOR = (uint8_t)divFactor;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_peripheral_enable(ULPCLK_Type *pULPCLK, ULPPERIPHERALS_CLK_T module, CLK_ENABLE_T clkType)
 * @brief     This API is used to enable the particular ULP perpheral Clock
 * @param[in] pULPCLK : pointer to the processor ULP clock source
 * @param[in] module : module for ULP peripheral Clock
 * @param[in] clkType  : clock type for ULP peripheral Clock
 * @return    RSI_OK on success
 */

rsi_error_t ulpss_peripheral_enable(ULPCLK_Type *pULPCLK, ULPPERIPHERALS_CLK_T module, CLK_ENABLE_T clkType)
{
  if (pULPCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  switch (module) {
    case ULP_I2C_CLK:
      pULPCLK->ULP_I2C_SSI_CLK_GEN_REG_b.ULP_I2C_CLK_EN_b = 1;
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.PCLK_ENABLE_I2C_b  = 1;
      break;
    case ULP_EGPIO_CLK:
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.EGPIO_CLK_EN_b      = 1;
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.EGPIO_PCLK_ENABLE_b = 1;
      break;
    case ULP_AUX_CLK:
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.AUX_CLK_EN_b = 1;

      if (clkType == ENABLE_STATIC_CLK) {
        pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.AUX_PCLK_EN_b  = 1;
        pULPCLK->ULP_AUXADC_CLK_GEN_REG_b.ULP_AUX_CLK_EN_b = 1;
      } else {
        /**Static enable **/
        pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.AUX_PCLK_EN_b               = 0;
        pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.AUX_CLK_DYN_CTRL_DISABLE_b  = 0;
        pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.AUX_PCLK_DYN_CTRL_DISABLE_b = 0;
      }
      break;
    case ULP_FIM_CLK:
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.FIM_PCLK_ENABLE_b = 1;
      if (clkType == ENABLE_STATIC_CLK) {
        pULPCLK->ULP_MISC_SOFT_SET_REG_b.FIM_CLK_EN_b = 1;
      } else {
        pULPCLK->ULP_MISC_SOFT_SET_REG_b.FIM_CLK_EN_b                  = 0;
        pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.FIM_CLK_DYN_CTRL_DISABLE_b = 0;
      }
      break;
    case ULP_VAD_CLK:
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.VAD_CLK_EN_b      = 1;
      pULPCLK->ULP_VAD_CLK_GEN_REG_b.ULP_VAD_CLK_EN_b    = 1;
      pULPCLK->ULP_VAD_CLK_GEN_REG_b.ULP_VAD_FCLK_EN     = 1;
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.VAD_PCLK_ENABLE_b = 1;
      break;
    case ULP_TIMER_CLK:
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.CLK_ENABLE_TIMER_b = 1;
      if (clkType == ENABLE_STATIC_CLK) {
        pULPCLK->ULP_MISC_SOFT_SET_REG_b.TIMER_PCLK_EN_b = 1;
      } else {
        pULPCLK->ULP_MISC_SOFT_SET_REG_b.TIMER_PCLK_EN_b                  = 0;
        pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.TIMER_PCLK_DYN_CTRL_DISABLE_b = 0;
        pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.TIMER_SCLK_DYN_CTRL_DISABLE_b = 0;
      }
      break;
    case ULP_UDMA_CLK:
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.UDMA_CLK_ENABLE_b = 1;
      break;
    case ULP_TOUCH_CLK:
      pULPCLK->ULP_TOUCH_CLK_GEN_REG_b.ULP_TOUCH_CLK_EN_b = 1;
      break;
    case ULP_UART_CLK:
      if (clkType == ENABLE_STATIC_CLK) {
        pULPCLK->ULP_MISC_SOFT_SET_REG_b.PCLK_ENABLE_UART_b = 1;
        pULPCLK->ULP_MISC_SOFT_SET_REG_b.SCLK_ENABLE_UART_b = 1;
      } else {
        pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.UART_CLK_DYN_CTRL_DISABLE_b  = 0;
        pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.UART_SCLK_DYN_CTRL_DISABLE_b = 0;
        pULPCLK->ULP_MISC_SOFT_SET_REG_b.PCLK_ENABLE_UART_b              = 0;
        pULPCLK->ULP_MISC_SOFT_SET_REG_b.SCLK_ENABLE_UART_b              = 0;
      }
      break;
    case ULP_SSI_CLK:
      pULPCLK->ULP_I2C_SSI_CLK_GEN_REG_b.ULP_SSI_CLK_EN_b       = 1;
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.SCLK_ENABLE_SSI_MASTER_b = 1;
      if (clkType == ENABLE_STATIC_CLK) {
        pULPCLK->ULP_MISC_SOFT_SET_REG_b.PCLK_ENABLE_SSI_MASTER_b = 1;
      } else {
        pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.SSI_MST_PCLK_DYN_CTRL_DISABLE_b = 0;
        pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.SSI_MST_SCLK_DYN_CTRL_DISABLE_b = 0;
        pULPCLK->ULP_MISC_SOFT_SET_REG_b.PCLK_ENABLE_SSI_MASTER_b           = 0;
      }
      break;
    case ULP_I2S_CLK:
      /*ULPSS I2S master*/
      pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_CLK_EN_b   = 1;
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.CLK_ENABLE_I2S_b = 1;
      pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_PCLK_EN_b  = 1;
      break;

    default:
      return INVALID_PARAMETERS;
  }
  return RSI_OK;
}
#endif

/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_peripheral_disable(ULPCLK_Type *pULPCLK, ULPPERIPHERALS_CLK_T module)
 * @brief     This API is used to Disable the particulat ULP perpheral Clock
 * @param[in] pULPCLK : pointer to the processor ULP clock source
 * @param[in] module : module for ULP peripheral Clock
 * @return    RSI_OK on success	
 */

rsi_error_t ulpss_peripheral_disable(ULPCLK_Type *pULPCLK, ULPPERIPHERALS_CLK_T module)
{
  if (pULPCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  switch (module) {
    case ULP_I2C_CLK:
      pULPCLK->ULP_I2C_SSI_CLK_GEN_REG_b.ULP_I2C_CLK_EN_b             = 0;
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.PCLK_ENABLE_I2C_b              = 0;
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.I2C_PCLK_DYN_CTRL_DISABLE_b = 1;
      break;
    case ULP_EGPIO_CLK:
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.EGPIO_CLK_EN_b                = 0;
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.EGPIO_PCLK_ENABLE_b           = 0;
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.EGPIO_PCLK_DYN_CTRL_DISABLE_b = 1;
      break;
    case ULP_AUX_CLK:
      pULPCLK->ULP_AUXADC_CLK_GEN_REG_b.ULP_AUX_CLK_EN_b              = 0;
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.AUX_CLK_EN_b                = 0;
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.AUX_PCLK_EN_b               = 0;
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.AUX_CLK_DYN_CTRL_DISABLE_b  = 1;
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.AUX_PCLK_DYN_CTRL_DISABLE_b = 1;
      break;
    case ULP_FIM_CLK:
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.FIM_CLK_EN_b                  = 0;
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.FIM_PCLK_ENABLE_b             = 0;
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.FIM_CLK_DYN_CTRL_DISABLE_b = 1;
      break;
    case ULP_VAD_CLK:
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.VAD_CLK_EN_b                  = 0;
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.VAD_PCLK_ENABLE_b             = 0;
      pULPCLK->ULP_VAD_CLK_GEN_REG_b.ULP_VAD_CLK_EN_b                = 0;
      pULPCLK->ULP_VAD_CLK_GEN_REG_b.ULP_VAD_FCLK_EN                 = 0;
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.VAD_CLK_DYN_CTRL_DISABLE_b = 1;
      break;
    case ULP_TIMER_CLK:
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.TIMER_PCLK_EN_b                  = 0;
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.CLK_ENABLE_TIMER_b               = 0;
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.TIMER_PCLK_DYN_CTRL_DISABLE_b = 1;
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.TIMER_SCLK_DYN_CTRL_DISABLE_b = 1;
      break;
    case ULP_UDMA_CLK:
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.UDMA_CLK_ENABLE_b = 0;
      break;
    case ULP_TOUCH_CLK:
      pULPCLK->ULP_TOUCH_CLK_GEN_REG_b.ULP_TOUCH_CLK_EN_b = 0;
      break;
    case ULP_UART_CLK:
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.PCLK_ENABLE_UART_b              = 0;
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.SCLK_ENABLE_UART_b              = 0;
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.UART_SCLK_DYN_CTRL_DISABLE_b = 1;
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.UART_CLK_DYN_CTRL_DISABLE_b  = 1;
      break;
    case ULP_SSI_CLK:
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.SCLK_ENABLE_SSI_MASTER_b           = 0;
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.PCLK_ENABLE_SSI_MASTER_b           = 0;
      pULPCLK->ULP_I2C_SSI_CLK_GEN_REG_b.ULP_SSI_CLK_EN_b                 = 0;
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.SSI_MST_PCLK_DYN_CTRL_DISABLE_b = 1;
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.SSI_MST_SCLK_DYN_CTRL_DISABLE_b = 1;
      break;
    case ULP_I2S_CLK:
      pULPCLK->ULP_MISC_SOFT_SET_REG_b.CLK_ENABLE_I2S_b              = 0;
      pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_PCLK_EN_b               = 0;
      pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_CLK_EN_b                = 0;
      pULPCLK->ULP_DYN_CLK_CTRL_DISABLE_b.I2S_CLK_DYN_CTRL_DISABLE_b = 1;
      pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_PCLK_DYN_CTRL_DISABLE_b = 1;
      pULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_SCLK_DYN_CTRL_DISABLE_b = 1;
      break;
    default:
      return INVALID_PARAMETERS;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_time_clk_disable(ULPCLK_Type *pULPCLK)
 * @brief     This API is used to disable the timer clock
 * @param[in] pULPCLK : pointer to the processor ULP clock source
 * @return    ulpss_peripheral_disable on success	
 */

rsi_error_t ulpss_time_clk_disable(ULPCLK_Type *pULPCLK)
{
  pULPCLK->ULP_TIMER_CLK_GEN_REG_b.ULP_TIMER_CLK_SEL = 0xF;
  return ulpss_peripheral_disable(pULPCLK, ULP_TIMER_CLK);
}

/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_ref_clk_config(ULPSS_REF_CLK_SEL_T clkSource)
 * @brief     This API is used to select the ULPSS processor ref clk configuration
 * @param[in] clkSource  : clock source for ULPSS processor reference clock select
 * @return    RSI_OK on success	
 */

rsi_error_t ulpss_ref_clk_config(ULPSS_REF_CLK_SEL_T clkSource)
{
  switch (clkSource) {
    case ULPSS_REF_BYP_CLK:
      MCU_FSM->MCU_FSM_REF_CLK_REG_b.ULPSS_REF_CLK_SEL_b = clkSource;
      system_clocks.ulp_ref_clock_source                 = clkSource;
      system_clocks.ulpss_ref_clk                        = system_clocks.byp_rc_ref_clock;
      break;

    case ULPSS_ULP_MHZ_RC_CLK:
      MCU_FSM->MCU_FSM_REF_CLK_REG_b.ULPSS_REF_CLK_SEL_b = clkSource;
      system_clocks.ulp_ref_clock_source                 = clkSource;
      system_clocks.ulpss_ref_clk                        = system_clocks.rc_mhz_clock;
      break;

    case ULPSS_40MHZ_CLK:
      MCU_FSM->MCU_FSM_REF_CLK_REG_b.ULPSS_REF_CLK_SEL_b = clkSource;
      system_clocks.ulp_ref_clock_source                 = clkSource;
      system_clocks.ulpss_ref_clk                        = system_clocks.rf_ref_clock;
      break;

    case ULPSS_MEMS_REF_CLK:
      TASS_PLL_CTRL_SET_REG(AFEPLLCTRLREG1)              = MEMS_REF_CLK_ENABLE;
      MCU_FSM->MCU_FSM_REF_CLK_REG_b.ULPSS_REF_CLK_SEL_b = clkSource;
      system_clocks.ulp_ref_clock_source                 = clkSource;
      system_clocks.ulpss_ref_clk                        = system_clocks.mems_ref_clock;
      break;

    case ULPSS_ULP_20MHZ_RINGOSC_CLK:
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_20MHZ_RING_OSC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      MCU_FSM->MCU_FSM_REF_CLK_REG_b.ULPSS_REF_CLK_SEL_b = clkSource;
      system_clocks.ulp_ref_clock_source                 = clkSource;
      system_clocks.ulpss_ref_clk                        = system_clocks.ro_20mhz_clock;
      break;

    case ULPSS_ULP_DOUBLER_CLK:
      /*6: ulp_doubler_clk*/
      /*Enable clock*/
      ulpss_enable_ref_clks(MCU_ULP_DOUBLER_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      MCU_FSM->MCU_FSM_REF_CLK_REG_b.ULPSS_REF_CLK_SEL_b = clkSource;
      system_clocks.ulp_ref_clock_source                 = clkSource;
      system_clocks.ulpss_ref_clk                        = system_clocks.doubler_clock;
      break;

    default:
      return INVALID_PARAMETERS;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_ulp_proc_clk_config(ULPCLK_Type *pULPCLK,
 *                                 ULP_PROC_CLK_SELECT_T clkSource,
 *                                 uint16_t divFactor,
 *                                 cdDelay delayFn)
 * @brief     This API is used to configure the ULPSS processor clock source
 * @param[in] pULPCLK : pointer to the processor ULP clock source
 * @param[in] clkSource  : clock source for ULPSS processor clock select
 * @param[in] divFactor : divison factor for processor clock source
 * @param[in] delayFn : delay functionfor processor clock source
 * @return    RSI_OK on success	
 */

rsi_error_t ulpss_ulp_proc_clk_config(ULPCLK_Type *pULPCLK,
                                      ULP_PROC_CLK_SELECT_T clkSource,
                                      uint16_t divFactor,
                                      cdDelay delayFn)
{
  /*Parameter validation */
  if ((pULPCLK == NULL) || (divFactor > ULP_PROC_MAX_DIVISOIN_FACTOR) || (clkSource > ULP_PROC_MAX_SEL)) {
    return INVALID_PARAMETERS;
  }

  /*Select */
  switch (clkSource) {

    case ULP_PROC_REF_CLK:
      /*Select clock MUX */
      RSI_SetRegSpiDivision(0U);
      pULPCLK->ULP_TA_CLK_GEN_REG_b.ULP_PROC_CLK_SEL = clkSource;
      SystemCoreClock                                = system_clocks.ulpss_ref_clk;
      break;

    case ULP_PROC_ULP_32KHZ_RO_CLK: /* 32KHZ RO clock not supported */
      /*Enable clock*/
      RSI_SetRegSpiDivision(0U);
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RO_CLK_EN, ULP_PROCESSOR_CLK, delayFn);
      /*Select clock MUX */
      pULPCLK->ULP_TA_CLK_GEN_REG_b.ULP_PROC_CLK_SEL = clkSource;
      SystemCoreClock                                = system_clocks.ro_32khz_clock;
      break;

    case ULP_PROC_ULP_32KHZ_RC_CLK:
      /*Enable clock*/
      RSI_SetRegSpiDivision(0U);
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RC_CLK_EN, ULP_PROCESSOR_CLK, delayFn);
      SystemCoreClock = system_clocks.rc_32khz_clock;
      /*Select clock MUX */
      pULPCLK->ULP_TA_CLK_GEN_REG_b.ULP_PROC_CLK_SEL = clkSource;
      break;

    case ULP_PROC_ULP_32KHZ_XTAL_CLK:
      /*Enable clock*/
      RSI_SetRegSpiDivision(0U);
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_XTAL_CLK_EN, ULP_PROCESSOR_CLK, delayFn);
      pULPCLK->ULP_TA_CLK_GEN_REG_b.ULP_PROC_CLK_SEL = clkSource;
      SystemCoreClock                                = system_clocks.xtal_32khz_clock;
      break;

    case ULP_PROC_ULP_MHZ_RC_CLK:
      /*Enable clock*/
      RSI_SetRegSpiDivision(0U);
      ulpss_enable_ref_clks(MCU_ULP_MHZ_RC_CLK_EN, ULP_PROCESSOR_CLK, delayFn);
      /*Select clock MUX */
      pULPCLK->ULP_TA_CLK_GEN_REG_b.ULP_PROC_CLK_SEL = clkSource;
      SystemCoreClock                                = system_clocks.rc_mhz_clock;
      break;

    case ULP_PROC_ULP_20MHZ_RO_CLK: /* 20MHZ RO clock not supported */
      /*Enable clock*/
      RSI_SetRegSpiDivision(0U);
      ulpss_enable_ref_clks(MCU_ULP_20MHZ_RING_OSC_CLK_EN, ULP_PROCESSOR_CLK, delayFn);
      /*Select clock MUX */
      pULPCLK->ULP_TA_CLK_GEN_REG_b.ULP_PROC_CLK_SEL = clkSource;
      SystemCoreClock                                = system_clocks.ro_20mhz_clock;
      break;

    case ULP_PROC_SOC_CLK:
      /*6: soc_clk*/
      RSI_SetRegSpiDivision(2U);
      pULPCLK->ULP_TA_CLK_GEN_REG_b.ULP_PROC_CLK_SEL = clkSource;
      SystemCoreClock                                = system_clocks.soc_clock;
      break;

    case ULP_PROC_ULP_DOUBLER_CLK:
      /*Enable clock*/
      RSI_SetRegSpiDivision(0U);
      ulpss_enable_ref_clks(MCU_ULP_DOUBLER_CLK_EN, ULP_PROCESSOR_CLK, delayFn);
      /*Select clock MUX */
      pULPCLK->ULP_TA_CLK_GEN_REG_b.ULP_PROC_CLK_SEL = clkSource;
      SystemCoreClock                                = system_clocks.doubler_clock;
      break;
    default:
      return INVALID_PARAMETERS;
  }
  while (pULPCLK->CLOCK_STAUS_REG_b.CLOCK_SWITCHED_PROC_CLK_b != 1)
    ;
  /*update the division factor */
  pULPCLK->ULP_TA_CLK_GEN_REG_b.ULP_PROC_CLK_DIV_FACTOR = (char)divFactor;
  /*clock Enable */
  pULPCLK->ULP_TA_CLK_GEN_REG_b.ULP2M4_A2A_BRDG_CLK_EN_b = 1;

  if (divFactor) {
    SystemCoreClock = (SystemCoreClock / divFactor);
  }
  system_clocks.soc_clock = SystemCoreClock;
  return RSI_OK;
}
/** @} */
