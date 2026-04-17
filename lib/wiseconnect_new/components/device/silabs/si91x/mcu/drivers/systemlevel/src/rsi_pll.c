/*******************************************************************************
* @file  rsi_pll.c
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
#include <math.h>
#include "rsi_rom_clks.h"
#ifndef PLL_ROMDRIVER_PRESENT
/** @addtogroup SOC3
* @{
*/
/*==============================================*/
/**
 * @fn         boolean_t clk_check_pll_lock(PLL_TYPE_T pllType)
 * @brief      This API is used to check the lock status of pll
 * @param[in]  pllType : PLL type
 * @return     Enable on success
 *			   Disable on failure
 */

boolean_t clk_check_pll_lock(PLL_TYPE_T pllType)
{
  uint16_t lock = 0;

  if (pllType == SOC_PLL) {
    lock = SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG13) >> 14;
  } else if (pllType == INTF_PLL) {
    lock = SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG13) >> 14;
  } else {
    lock = SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG13) >> 14;
  }
  if (lock & 1) {
    return Enable;
  } else {
    return Disable;
  }
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_soc_pll_clk_enable(boolean_t clkEnable)
 * @brief      This API is used to enable SoC-PLL output clock
 * @param[in]  clkEnable : enable the clock for SoC-PLL output clock
 * @return     RSI_OK on success
 */

rsi_error_t clk_soc_pll_clk_enable(boolean_t clkEnable)
{
  if (clkEnable == Enable) {
    /* Enable SoC-PLL */
    SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) |= PLL_500_CLK_ENABLE;
  } else {
    /* Disable SoC-PLL */
    SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) &= (uint16_t)(~(PLL_500_CLK_ENABLE));
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          rsi_error_t clk_set_soc_pll_freq(const M4CLK_Type *pCLK, uint32_t socPllFreq, uint32_t pllRefClk)
 * @brief       This API is used to set the Soc PLL clock to particular frequency
 * @param[in]   pCLK        : pointer to the processor clock source
 * @param[in]   socPllFreq  : SoC PLL frequency for Soc PLL clock to particular frequency
 * @param[in]   pllRefClk   : PLL reference clock for Soc PLL clock to particular frequency
 * @return      RSI_OK on success
 */

rsi_error_t clk_set_soc_pll_freq(const M4CLK_Type *pCLK, uint32_t socPllFreq, uint32_t pllRefClk)
{
  uint16_t shiftFac     = 0;
  uint16_t socPllMulFac = 0;
  uint16_t dcoFreq      = 0;
  uint16_t reg          = 0;
  uint16_t socPllDivFac = 0;
  uint16_t socreg1      = 0x31c9;
  uint16_t socreg3      = 0;
  uint16_t socPllTvRead = 0;

  /* parameter validation */

  if ((pCLK == NULL) || (socPllFreq < SOC_PLL_MIN_FREQUECY) || (socPllFreq > SOC_PLL_MAX_FREQUECY)) {
    return INVALID_PARAMETERS;
  }
  socPllFreq = (socPllFreq / 1000000);
  pllRefClk  = (pllRefClk / 1000000);

  if ((MCU_RET->CHIP_CONFIG_MCU_READ_b.LIMIT_M4_FREQ_110MHZ_b == 1) || (M4_BBFF_STORAGE1 & BIT(10))) {
    if (socPllFreq >= 201) {
      /* New table */
      /* Program PLL to 200Mhz */
      /* Call low level API with m,n,p values for 200Mhz configuration. */
      clk_soc_pll_set_freq_div(pCLK, 1, 0, 39, 199, 0, 1, 1);
      return ERROR_INVALID_INPUT_FREQUENCY;
    } else {
      /* Derive the m,n,p values as per the newer table */
      /* program PLL with derived values */
      /* Wait for PLL lock */
      if (socPllFreq < 2) {
        shiftFac = 7;
      } else if (socPllFreq < 3) {
        shiftFac = 6;
      } else if (socPllFreq < 6) {
        shiftFac = 5;
      } else if (socPllFreq < 12) {
        shiftFac = 4;
      } else if (socPllFreq < 23) {
        shiftFac = 3;
      } else if (socPllFreq < 46) {
        shiftFac = 2;
      } else if (socPllFreq < 91) {
        shiftFac = 1;
      } else {
        shiftFac = 0;
      }

      socPllDivFac = (uint16_t)((1 << shiftFac) - 1);

      socPllMulFac = (uint16_t)(((socPllDivFac + 1) * socPllFreq) - 1);

      /* RESET PLL */
      clk_soc_pll_clk_reset();

      reg = SPI_MEM_MAP_PLL(SOCPLLMACROREG3);
      reg = (uint16_t)(reg << 1); /* according to RTL bug SOCPLLMACROREG3 (read issue) will shift to one left shift */
      reg &= (uint16_t)~LDO_PROG_SOCPLL;
      reg |= 1 << 13;

      SPI_MEM_MAP_PLL(SOCPLLMACROREG3) = reg;

      socreg1 &= (uint16_t)~DCO_FIX_SEL_MASK;
      /* writing the value into the dco_fix_sel=1 */
      socreg1 |= 1;

      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG2) = (uint16_t)(socPllDivFac << 9 | (pllRefClk - 1) << 3);
      socreg1 &= (uint16_t)~PLL_500_M_MASK;
      socreg1 |= (uint16_t)((socPllMulFac << 6) | PLL_500_CLK_ENABLE); /* m factor */
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) = socreg1;

      /* Enable */
      clk_soc_pll_clk_set(pCLK); /* wait for lock */
      /* Set and clear(read modify write) PLL500CTRLREG7 BIT[4] :  TV value will be latched */

      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG7) |= SPI_INP_RD_EN;
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG7) &= (uint16_t)(~(SPI_INP_RD_EN));

      /* Read the TV value from PLL500CTRLREG12  BIT[15 : 11] */
      socPllTvRead = (SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG12) & 0xF800) >> 11;

      if (socPllTvRead <= (M4_BBFF_STORAGE1 & 0x001F)) {
        /* New table */
        /* Program PLL to 200Mhz */
        /* Call low level API with m,n,p values for 200Mhz configuration. */
        clk_soc_pll_set_freq_div(pCLK, 1, 0, 39, 199, 0, 1, 1);
        return ERROR_INVALID_INPUT_FREQUENCY;

      } else {
        /* Program the required frequency by user */
        return RSI_OK;
      }
    }
  } else {
    /* Older table */
    if (socPllFreq < 2) {
      shiftFac = 7;
    } else if (socPllFreq < 4) {
      shiftFac = 6;
    } else if (socPllFreq < 8) {
      shiftFac = 5;
    } else if (socPllFreq < 16) {
      shiftFac = 4;
    } else if (socPllFreq < 32) {
      shiftFac = 3;
    } else if (socPllFreq < 64) {
      shiftFac = 2;
    } else if (socPllFreq < 127) {
      shiftFac = 1;
    } else {
      shiftFac = 0;
    }
    dcoFreq      = (uint16_t)(socPllFreq << shiftFac);
    socPllMulFac = (uint16_t)((socPllFreq << shiftFac) - 1);
    socPllDivFac = (uint16_t)((1 << shiftFac) - 1);

    /* RESET PLL */
    clk_soc_pll_clk_reset();

    if (socPllFreq >= 201) {
      if ((socPllFreq % 2) == 0) {
        socreg3 &= (uint16_t)~FCW_F_MASK;
        SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG3) = socreg3;
      } else {
        socreg3 |= (8192 << 2);
        SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG3) = socreg3;
      }
    }
    if (dcoFreq >= 251) {
      /* clearing the two bits i.e dco_fix_sel */
      socreg1 &= (uint16_t)~DCO_FIX_SEL_MASK;
      /* writing the value into the dco_fix_sel=2 */
      socreg1 |= 2;
      reg = SPI_MEM_MAP_PLL(SOCPLLMACROREG3);
      reg = (uint16_t)(reg << 1); /* according to RTL bug SOCPLLMACROREG3 (read issue) will shift to one left shift */
      reg &= (uint16_t)~LDO_PROG_SOCPLL;
      reg |= 5 << 13;
      SPI_MEM_MAP_PLL(SOCPLLMACROREG3)       = reg;
      socPllMulFac                           = ((dcoFreq / 2) - 1);
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG2) = (uint16_t)(socPllDivFac << 9 | (pllRefClk - 1) << 3);
      socreg1 &= (uint16_t)~PLL_500_M_MASK;
      socreg1 |= (uint16_t)((socPllMulFac << 6) | PLL_500_CLK_ENABLE); /* m factor */
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) = socreg1;
    } else if ((dcoFreq >= 201) && (dcoFreq <= 250)) {
      socreg1 &= (uint16_t)~DCO_FIX_SEL_MASK;
      reg = SPI_MEM_MAP_PLL(SOCPLLMACROREG3);
      reg = (uint16_t)(reg << 1); /* according to RTL bug SOCPLLMACROREG3 (read issue) will shift to one left shift */
      reg &= (uint16_t)~LDO_PROG_SOCPLL;
      reg |= 5 << 13;
      SPI_MEM_MAP_PLL(SOCPLLMACROREG3)       = reg;
      socPllMulFac                           = ((dcoFreq / 2) - 1);
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG2) = (uint16_t)(socPllDivFac << 9 | (pllRefClk - 1) << 3);
      socreg1 &= (uint16_t)~PLL_500_M_MASK;
      socreg1 |= (uint16_t)((socPllMulFac << 6) | PLL_500_CLK_ENABLE); /* m factor */
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) = socreg1;
    } else {
      socreg3 &= (uint16_t)~FCW_F_MASK;
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG3) = socreg3;
      socreg1 &= (uint16_t)~DCO_FIX_SEL_MASK;
      socreg1 |= 1;
      reg = SPI_MEM_MAP_PLL(SOCPLLMACROREG3);
      reg = (uint16_t)(reg << 1); /* according to RTL bug SOCPLLMACROREG3 (read issue) will shift to one left shift */
      reg &= (uint16_t)~LDO_PROG_SOCPLL;
      reg |= 4 << 13;
      SPI_MEM_MAP_PLL(SOCPLLMACROREG3)       = reg;
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG2) = (uint16_t)(socPllDivFac << 9 | (pllRefClk - 1) << 3);
      socreg1 &= (uint16_t)~PLL_500_M_MASK;
      socreg1 |= (uint16_t)((socPllMulFac << 6) | PLL_500_CLK_ENABLE); // m factor
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) = socreg1;
    }
    /* Enable */
    clk_soc_pll_clk_set(pCLK);
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          rsi_error_t clk_soc_pll_set_freq_div(const M4CLK_Type *pCLK,
 *                                boolean_t clk_en,
 *                                uint16_t divFactor,
 *                                uint16_t nFactor,
 *                                uint16_t mFactor,
 *                                uint16_t fcwF,
 *                                uint16_t dcoFixSel,
 *                                uint16_t ldoProg)
 * @brief       This API is used to configure the SOC PLL clock frequency
 * @param[in]   pCLK       : pointer to the processor clock source
 * @param[in]   clk_en     : enable the clock for SOC PLL clock frequency
 * @param[in]   divFactor  : division factor for SOC PLL clock frequency
 * @param[in]   nFactor    : n number of factor for SOC PLL clock frequency
 * @param[in]   mFactor    : m number of factor for SOC PLL clock frequency
 * @param[in]   fcwF       : frequency for SOC PLL clock frequency
 * @param[in]   dcoFixSel  : fixed select for SOC PLL clock frequency
 * @param[in]   ldoProg    : ldo program for SOC PLL clock frequency 
 * @return      RSI_OK on success
 */

rsi_error_t clk_soc_pll_set_freq_div(const M4CLK_Type *pCLK,
                                     boolean_t clk_en,
                                     uint16_t divFactor,
                                     uint16_t nFactor,
                                     uint16_t mFactor,
                                     uint16_t fcwF,
                                     uint16_t dcoFixSel,
                                     uint16_t ldoProg)
{
  uint16_t socreg1      = 0x31c9;
  uint16_t socreg3      = 0;
  uint16_t reg          = 0;
  uint16_t socPllTvRead = 0;
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  /* RESET PLL */
  clk_soc_pll_clk_reset();
  if ((MCU_RET->CHIP_CONFIG_MCU_READ_b.LIMIT_M4_FREQ_110MHZ_b == 1) || (M4_BBFF_STORAGE1 & BIT(10))) {
    if (clk_en) {
      socreg3 &= (uint16_t)~FCW_F_MASK;
      socreg3                                = (uint16_t)(fcwF << 2);
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG3) = socreg3;
      socreg1 &= (uint16_t)~DCO_FIX_SEL_MASK;
      socreg1 |= 1;
      reg = SPI_MEM_MAP_PLL(SOCPLLMACROREG3);
      reg = (uint16_t)(reg << 1); /* according to RTL bug SOCPLLMACROREG3 (read issue) will shift to one left shift */
      reg &= (uint16_t)~LDO_PROG_SOCPLL;
      reg |= 1 << 13;
      SPI_MEM_MAP_PLL(SOCPLLMACROREG3) = reg;
      socreg1 |= PLL_500_CLK_ENABLE;
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG2) = (uint16_t)(divFactor << 9 | nFactor << 3);
      socreg1 &= (uint16_t)~PLL_500_M_MASK;
      socreg1 |= (uint16_t)(mFactor << 6 | PLL_500_CLK_ENABLE);
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) = socreg1;
    } else {
      socreg1 &= (uint16_t)(~PLL_500_CLK_ENABLE); /* soc_pll_clk o/p disable */
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) = socreg1;
    }

    /* wait for lock */
    clk_soc_pll_clk_set(pCLK);
    /* Set and clear(read modify write) PLL500CTRLREG7 BIT[4] :  TV value will be latched */

    SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG7) |= SPI_INP_RD_EN;
    SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG7) &= (uint16_t)(~(SPI_INP_RD_EN));

    /* Read the TV value from PLL500CTRLREG12  BIT[15 : 11] */
    socPllTvRead = (SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG12) & 0xF800) >> 11;

    if ((socPllTvRead <= (M4_BBFF_STORAGE1 & 0x001F)) && (divFactor == 0)) {
      /* RESET PLL */
      clk_soc_pll_clk_reset();
      return ERROR_INVALID_INPUT_FREQUENCY;
    } else {
      /* Program the required frequency by user */
      return RSI_OK;
    }
  } else {
    if (clk_en) {
      socreg3 &= (uint16_t)~FCW_F_MASK;
      socreg3                                = (uint16_t)(fcwF << 2);
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG3) = socreg3;
      socreg1 &= (uint16_t)~DCO_FIX_SEL_MASK;
      socreg1 |= dcoFixSel;
      reg = SPI_MEM_MAP_PLL(SOCPLLMACROREG3);
      reg = (uint16_t)(reg << 1); // according to RTL bug SOCPLLMACROREG3 (read issue) will shift to one left shift
      reg &= (uint16_t)~LDO_PROG_SOCPLL;
      reg |= (uint16_t)(ldoProg << 13);
      SPI_MEM_MAP_PLL(SOCPLLMACROREG3) = reg;
      socreg1 |= PLL_500_CLK_ENABLE;
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG2) = (uint16_t)(divFactor << 9 | nFactor << 3);
      socreg1 &= (uint16_t)~PLL_500_M_MASK;
      socreg1 |= (uint16_t)((mFactor << 6) | PLL_500_CLK_ENABLE);
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) = socreg1;
    } else {
      socreg1 &= (uint16_t)(~PLL_500_CLK_ENABLE); // soc_pll_clk o/p disable
      SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) = socreg1;
    }

    /* Enable */
    clk_soc_pll_clk_set(pCLK);
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_soc_pll_clk_set(const M4CLK_Type *pCLK)
 * @brief     This API is used to Enables the SoC-PLL
 * @param[in] pCLK : pointer to the processor clock source
 * @return    RSI_OK on success
 */

rsi_error_t clk_soc_pll_clk_set(const M4CLK_Type *pCLK)
{
  SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG11) = 0xFFFF;
  /* Wait for lock */
  while ((pCLK->PLL_STAT_REG_b.SOCPLL_LOCK) != 1)
    ;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_soc_pll_clk_bypass_enable(boolean_t clkEnable)
 * @brief      This API is used to Enable bypass clock
 * @param[in]  clkEnable : enable the clock
 * @return     RSI_OK on success
 */

rsi_error_t clk_soc_pll_clk_bypass_enable(boolean_t clkEnable)
{
  if (clkEnable == Enable) {
    /* Enable PLL clock */
    SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) |= PLL_500_BYPASS;
  } else {
    /* Disable PLL clock */
    SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) &= (uint16_t)(~(PLL_500_BYPASS));
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_soc_pll_clk_reset(void)
 * @brief     This API is used to Reset the Soc_pll_clk
 * @return    RSI_OK on success
 */

rsi_error_t clk_soc_pll_clk_reset(void)
{
  SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG11) = 0x01FF;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_soc_pll_pd_enable(boolean_t en)
 * @brief      This API is used to Enable the PdEnable(power down)
 * @param[in]  en : enable
 * @return     RSI_OK on success
 */

rsi_error_t clk_soc_pll_pd_enable(boolean_t en)
{
  if (en == Enable) {
    /* Enable power down */
    SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) |= PLL_500_PD;
  } else {
    /* Disable power down */
    SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) &= (uint16_t)(~(PLL_500_PD));
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_soc_pll_turn_off(void)
 * @brief     This API is used to TurnOff the SOC_PLL
 * @return    RSI_OK on success
 */

rsi_error_t clk_soc_pll_turn_off(void)
{
  uint16_t socreg1 = 0x31c9;
  /* Set PLL PD Bit */
  socreg1 |= PLL_500_PD;
  socreg1 |= PLL_500_RST;
  SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) = socreg1;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_soc_pll_turn_on(void)
 * @brief     This API is used to TurnOn the SOC_PLL
 * @return    RSI_OK on success
 */

rsi_error_t clk_soc_pll_turn_on(void)
{
  uint16_t socreg1 = 0x31c9;
  /* Disable power down */
  socreg1 &= (uint16_t)(~(PLL_500_RST));
  socreg1 &= (uint16_t)(~(PLL_500_PD));
  SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG1) = socreg1;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_i2s_pll_clk_enable(boolean_t clkEnable)
 * @brief      This API is used to Enable the I2s_PLL output clock
 * @param[in]  clkEnable : enable the clock  for I2s_PLL output clock
 * @return     RSI_OK on success
 */

rsi_error_t clk_i2s_pll_clk_enable(boolean_t clkEnable)
{
  if (clkEnable == Enable) {
    /* Enable SoC-PLL */
    SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG1) |= PLL_500_CLK_ENABLE;
  } else {
    /* Disable SoC-PLL */
    SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG1) &= (uint16_t)(~(PLL_500_CLK_ENABLE));
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_i2s_pll_clk_bypass_enable(boolean_t clkEnable)
 * @brief      This API is used to Enable bypass clock
 * @param[in]  clkEnable : enable the clock  for bypass clock
 * @return     RSI_OK on success
 */

rsi_error_t clk_i2s_pll_clk_bypass_enable(boolean_t clkEnable)
{
  if (clkEnable == Enable) {
    /* Enable PLL clock */
    SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG1) |= PLL_500_BYPASS;
  } else {
    /* Disable PLL clock */
    SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG1) &= (uint16_t)(~(PLL_500_BYPASS));
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_i2s_pll_pd_enable(boolean_t en)
 * @brief      This API is used to Enable the PdEnable(power down)
 * @param[in]  en : enable 
 * @return     RSI_OK on success
 */

rsi_error_t clk_i2s_pll_pd_enable(boolean_t en)
{
  if (en == Enable) {
    /* Enable power down */
    SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG1) |= PLL_500_PD;
  } else {
    /* Disable power down */
    SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG1) &= (uint16_t)(~(PLL_500_PD));
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_i2s_pll_turn_off(void)             
 * @brief     This API is used to TurnOff the I2s_PLL
 * @return    RSI_OK on success
 */

rsi_error_t clk_i2s_pll_turn_off(void)
{
  uint16_t i2sreg1 = 0x1244;
  /* Set PLL PD Bit */
  i2sreg1 |= PLL_500_PD;
  i2sreg1 |= PLL_500_RST;
  SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG1) |= i2sreg1;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn       rsi_error_t clk_i2s_pll_turn_on(void)
 * @brief    This API is used to TurnOn the I2s_PLL
 * @return   RSI_OK on success
 */

rsi_error_t clk_i2s_pll_turn_on(void)
{
  uint16_t i2sreg1 = 0x1244;
  i2sreg1 &= (uint16_t)(~(PLL_500_PD));
  i2sreg1 &= (uint16_t)(~(PLL_500_RST));
  SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG1) = i2sreg1;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_set_i2s_pll_freq(const M4CLK_Type *pCLK, uint32_t i2sPllFreq, uint32_t fXtal)
 * @brief     This API is used to set the I2s_pll clock to particular frequency 
 * @param[in] pCLK       : pointer to the processor clock source
 * @param[in] i2sPllFreq : PLL clock of I2S for particular frequency
 * @param[in] fXtal      : frequency oscillator for particular frequency
 * @return    RSI_OK on success
 */

rsi_error_t clk_set_i2s_pll_freq(const M4CLK_Type *pCLK, uint32_t i2sPllFreq, uint32_t fXtal)
{
  uint16_t p_div         = 0;
  uint16_t u16DivFactor1 = 0;
  uint16_t u16DivFactor2 = 0;
  uint16_t N             = 0;
  uint16_t M             = 0;
  uint16_t FCW_F         = 0;
  uint32_t fref          = 0;
  uint32_t Fdco;
  float g;
  double FCW;
  double frac;
  uint16_t i2sreg1 = 0x1244;
  uint16_t i2sreg2 = 0x5850;
  uint16_t i2sreg3 = 0xba60;
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }

  if (!((fXtal == 9600000) || (fXtal == 19200000) || (fXtal == 38400000))) {
    fref = 1000000; /* One Mega herz steps */
  } else {
    fref = 960000; /* 0.96 Mega herz steps */
  }
  /* Calculating p_div value */
  g = ((float)I2S_DCO_FREQ1 / (float)i2sPllFreq);
  if (g - (float)(int)g == 0) /* checking if the value is an integer */
  {
    p_div = (uint16_t)(g);
    Fdco  = I2S_DCO_FREQ1;
  } else {
    p_div = (uint16_t)(I2S_DCO_FREQ2 / i2sPllFreq);
    Fdco  = I2S_DCO_FREQ2;
  }
  N = (uint16_t)(fXtal / fref); /* calculating N value */
  /* deriving M and FCW_F value */
  FCW   = (float)Fdco / (float)fref;
  M     = (uint16_t)FCW;
  frac  = (FCW - M);
  FCW_F = (uint16_t)(frac * (1 << 14));
  if (Fdco == I2S_DCO_FREQ1) {
    FCW_F = (FCW_F + 1);
  }
  /* deriving the p_div1 and p_div2 values from p_div value */
  /* From the refernce of PLL Programming table */
  if (p_div >= 288) {
    u16DivFactor2 = 4;
    u16DivFactor1 = ((p_div >> 4) - 1);
  } else if ((p_div >= 96) && (p_div <= 144)) {
    u16DivFactor2 = 3;
    u16DivFactor1 = ((p_div >> 3) - 1);
  } else if (p_div == 72) {
    u16DivFactor2 = 2;
    u16DivFactor1 = ((p_div >> 2) - 1);
  } else if ((p_div >= 36) && (p_div <= 48)) {
    u16DivFactor2 = 1;
    u16DivFactor1 = ((p_div >> 1) - 1);
  } else {
    u16DivFactor2 = 0;
    u16DivFactor1 = (p_div - 1);
  }
  /* RESET PLL */
  clk_i2s_pll_clk_reset();
  i2sreg2 &= (uint16_t)~N_DIV_MASK;
  i2sreg2 |= (uint16_t)(N << 1);
  i2sreg3 &= (uint16_t)~FCW_F_MASK;
  i2sreg3 |= (uint16_t)(FCW_F << 2);
  i2sreg1 &= (uint16_t)~PLL_500_M_MASK;
  i2sreg1 |= (uint16_t)(M << 6);
  i2sreg2 &= (uint16_t)~0xff00;
  i2sreg2 |= (uint16_t)((u16DivFactor1 << 11) | (u16DivFactor2 << 8));
  i2sreg1 &= (uint16_t)(~PLL_500_PD);
  i2sreg1 &= (uint16_t)(~PLL_500_RST);
  i2sreg1 |= PLL_500_BYPASS;
  SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG1) = i2sreg1;
  SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG2) = i2sreg2;
  SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG3) = i2sreg3;
  clk_i2s_pll_clk_set(pCLK);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_i2s_pll_set_freq_div(const M4CLK_Type *pCLK,
 *                               uint16_t u16DivFactor1,
 *                               uint16_t u16DivFactor2,
 *                               uint16_t nFactor,
 *                               uint16_t mFactor,
 *                               uint16_t fcwF)
 * @brief     This API is used to divide I2s_PLL Clock
 * @param[in] pCLK           : pointer to the processor clock source
 * @param[in] u16DivFactor1  : division factor of 1 
 * @param[in] u16DivFactor2  : division factor of 2
 * @param[in] nFactor        : n number of factor for I2s_PLL Clock
 * @param[in] mFactor        : m number of factor for I2s_PLL Clock
 * @param[in] fcwF           : frequency for for I2s_PLL Clock
 * @return    RSI_OK on success
 */

rsi_error_t clk_i2s_pll_set_freq_div(const M4CLK_Type *pCLK,
                                     uint16_t u16DivFactor1,
                                     uint16_t u16DivFactor2,
                                     uint16_t nFactor,
                                     uint16_t mFactor,
                                     uint16_t fcwF)
{
  uint16_t i2sreg1 = 0x1244;
  uint16_t i2sreg2 = 0x5850;
  uint16_t i2sreg3 = 0xba60;
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  clk_i2s_pll_clk_reset();
  if (u16DivFactor1) {
    i2sreg2 &= (uint16_t)~N_DIV_MASK;
    i2sreg2 |= (uint16_t)(nFactor << 1);
    i2sreg3 &= (uint16_t)~FCW_F_MASK;
    i2sreg3 |= (uint16_t)(fcwF << 2);
    i2sreg1 &= (uint16_t)~PLL_500_M_MASK;
    i2sreg1 |= (uint16_t)(mFactor << 6);
    i2sreg2 &= (uint16_t)~0xff00;
    i2sreg2 |= (uint16_t)((u16DivFactor1 << 11) | (u16DivFactor2 << 8));
    i2sreg1 &= (uint16_t)~PLL_500_PD;
    i2sreg1 &= (uint16_t)(~PLL_500_RST);
    i2sreg1 |= PLL_500_BYPASS;
    SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG1) = i2sreg1;
    SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG2) = i2sreg2;
    SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG3) = i2sreg3;
  }
  clk_i2s_pll_clk_set(pCLK);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_i2s_pll_clk_set(const M4CLK_Type *pCLK)
 * @brief      This API is used to set the I2s_pll_clk
 * @param[in]  pCLK  : pointer to the processor clock source
 * @return     RSI_OK on success
 */

rsi_error_t clk_i2s_pll_clk_set(const M4CLK_Type *pCLK)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG11) = 0xFFFF;
  while ((pCLK->PLL_STAT_REG_b.I2SPLL_LOCK) != 1)
    ; /* checking for pll lck */
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_i2s_pll_clk_reset(void)
 * @brief     This API is used to set the I2s_pll_clk
 * @return    RSI_OK on success
 */

rsi_error_t clk_i2s_pll_clk_reset(void)
{
  SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG11) = 0x01FF;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_intf_pll_clk_enable(boolean_t clkEnable)
 * @brief      This API is used to Enable the Intf_PLL output clock
 * @param[in]  clkEnable : enable the clock
 * @return     RSI_OK on success
 */

rsi_error_t clk_intf_pll_clk_enable(boolean_t clkEnable)
{
  if (clkEnable == Enable) {
    /* Enable SoC-PLL */
    SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) |= PLL_500_CLK_ENABLE;
  } else {
    /* Disable SoC-PLL */
    SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) &= (uint16_t)(~(PLL_500_CLK_ENABLE));
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_intf_pll_pd_enable(boolean_t en)
 * @brief      This API is used to Enable the PdEnable(power down)
 * @param[in]  en : enable
 * @return     RSI_OK on success
 */

rsi_error_t clk_intf_pll_pd_enable(boolean_t en)
{
  if (en == Enable) {
    /* Enable power down */
    SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) |= PLL_500_PD;
  } else {
    /* Disable power down */
    SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) &= (uint16_t)(~(PLL_500_PD));
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_intf_pll_turn_off(void)
 * @brief      This API is used to TurnOff the Intf_PLL
 * @return     RSI_OK on success
 */

rsi_error_t clk_intf_pll_turn_off(void)
{
  uint16_t intfreg1 = 0x31c9;
  /* Set PLL PD Bit */
  intfreg1 |= PLL_500_PD;
  /* setting the bit reset */
  intfreg1 |= PLL_500_RST;
  SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) = intfreg1;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_set_intf_pll_freq(const M4CLK_Type *pCLK, uint32_t intfPllFreq, uint32_t pllRefClk)
 * @brief      This API is used to set the INTFPLL clock to particular frequency
 * @param[in]  pCLK        : pointer to the processor clock source
 * @param[in]  intfPllFreq : input frequency of PLL frequency
 * @param[in]  pllRefClk   : PLL reference clock  
 * @return     RSI_OK on success
 */

rsi_error_t clk_set_intf_pll_freq(const M4CLK_Type *pCLK, uint32_t intfPllFreq, uint32_t pllRefClk)
{
  uint16_t shiftFac      = 0;
  uint16_t intfPllMulFac = 0;
  uint16_t intfPllDivFac = 0;
  uint16_t reg           = 0;
  uint16_t dcoFreq       = 0;
  uint16_t intfreg1      = 0x31c9;
  uint16_t intfreg3      = 0;
  uint16_t intfPllTvRead = 0;

  /* Parameter validation */
  if ((pCLK == NULL) || (intfPllFreq < INTF_PLL_MIN_FREQUECY) || (intfPllFreq > INTF_PLL_MAX_FREQUECY)) {
    return INVALID_PARAMETERS;
  }
  intfPllFreq = (intfPllFreq / 1000000);
  pllRefClk   = (pllRefClk / 1000000);

  if ((MCU_RET->CHIP_CONFIG_MCU_READ_b.LIMIT_M4_FREQ_110MHZ_b == 1) || (M4_BBFF_STORAGE1 & BIT(10))) {
    if (intfPllFreq >= 201) {
      /* New table */
      /* Program PLL to 200Mhz */
      /* Call low level API with m,n,p values for 200Mhz configuration. */
      clk_intf_pll_set_freq_div(pCLK, 1, 0, 39, 199, 0, 1, 1);
      return ERROR_INVALID_INPUT_FREQUENCY;
    } else {
      /* Derive the m,n,p values as per the newer table */
      /* program PLL with derived values */
      /* Wait for PLL lock */
      if (intfPllFreq < 2) {
        shiftFac = 7;
      } else if (intfPllFreq < 3) {
        shiftFac = 6;
      } else if (intfPllFreq < 6) {
        shiftFac = 5;
      } else if (intfPllFreq < 12) {
        shiftFac = 4;
      } else if (intfPllFreq < 23) {
        shiftFac = 3;
      } else if (intfPllFreq < 46) {
        shiftFac = 2;
      } else if (intfPllFreq < 91) {
        shiftFac = 1;
      } else {
        shiftFac = 0;
      }

      intfPllDivFac = (uint16_t)((1 << shiftFac) - 1);

      intfPllMulFac = (uint16_t)(((intfPllDivFac + 1) * intfPllFreq) - 1);

      /* RESET PLL */
      clk_intf_pll_clk_reset();

      reg = SPI_MEM_MAP_PLL(SOCPLLMACROREG3);
      reg = (uint16_t)(reg << 1); /* according to RTL bug SOCPLLMACROREG3 (read issue) will shift to one left shift */
      reg &= (uint16_t)~LDO_PROG_INTFPLL;
      reg |= 1 << 10;

      SPI_MEM_MAP_PLL(SOCPLLMACROREG3) = reg;

      intfreg1 &= (uint16_t)~DCO_FIX_SEL_MASK;
      /* writing the value into the dco_fix_sel=1 */
      intfreg1 |= 1;

      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG2) = (uint16_t)(intfPllDivFac << 9 | (pllRefClk - 1) << 3);
      intfreg1 &= (uint16_t)~PLL_500_M_MASK;
      intfreg1 |= (uint16_t)((intfPllMulFac << 6) | PLL_500_CLK_ENABLE); /* m factor */
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) = intfreg1;

      /* Enable */
      clk_intf_pll_clk_set(pCLK);
      /* Set and clear(read modify write) INTFPLL500CTRLREG7 BIT[4] :  TV value will be latched */

      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG7) |= SPI_INP_RD_EN;
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG7) &= (uint16_t)(~(SPI_INP_RD_EN));
      /* Read the TV value from INTFPLL500CTRLREG12  BIT[15 : 11] */
      intfPllTvRead = (SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG12) & 0xF800) >> 11;

      if (intfPllTvRead <= ((M4_BBFF_STORAGE1 & 0x03E0) >> 5)) {
        /* New table */
        /* Program PLL to 200Mhz */
        /* Call low level API with m,n,p values for 200Mhz configuration. */
        clk_intf_pll_set_freq_div(pCLK, 1, 0, 39, 199, 0, 1, 1);
        return ERROR_INVALID_INPUT_FREQUENCY;

      } else {
        /* Program the required frequency by user */
        return RSI_OK;
      }
    }

  } else { /* older table */
    if (intfPllFreq < 2) {
      shiftFac = 7;
    } else if (intfPllFreq < 4) {
      shiftFac = 6;
    } else if (intfPllFreq < 8) {
      shiftFac = 5;
    } else if (intfPllFreq < 16) {
      shiftFac = 4;
    } else if (intfPllFreq < 32) {
      shiftFac = 3;
    } else if (intfPllFreq < 64) {
      shiftFac = 2;
    } else if (intfPllFreq < 127) {
      shiftFac = 1;
    } else {
      shiftFac = 0;
    }
    dcoFreq       = (uint16_t)(intfPllFreq << shiftFac);
    intfPllMulFac = (uint16_t)((intfPllFreq << shiftFac) - 1);
    intfPllDivFac = (uint16_t)((1 << shiftFac) - 1);
    /* RESET PLL */
    clk_intf_pll_clk_reset();
    if (intfPllFreq >= 201) {
      if ((intfPllFreq % 2) == 0) {
        intfreg3 &= (uint16_t)~FCW_F_MASK;
        SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG3) = intfreg3;
      } else {
        intfreg3 |= (8192 << 2);
        SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG3) = intfreg3;
      }
    }

    if (dcoFreq >= 251) {

      intfreg1 &= (uint16_t)~DCO_FIX_SEL_MASK;
      /* writing the value into the dco_fix_sel=2 */
      intfreg1 |= 2;
      reg = SPI_MEM_MAP_PLL(SOCPLLMACROREG3);
      reg = (uint16_t)(reg << 1);
      reg &= (uint16_t)~LDO_PROG_INTFPLL;
      reg |= 5 << 10;
      SPI_MEM_MAP_PLL(SOCPLLMACROREG3) = reg;
      intfPllMulFac                    = ((dcoFreq / 2) - 1);
      intfreg1 |= PLL_500_CLK_ENABLE; /* soc_pll_clk o/p en */
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG2) = (uint16_t)((intfPllDivFac << 9) | ((pllRefClk - 1) << 3));
      intfreg1 &= (uint16_t)~PLL_500_M_MASK;
      intfreg1 |= (uint16_t)((intfPllMulFac << 6) | PLL_500_CLK_ENABLE); /* m factor */
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) = intfreg1;
    } else if ((dcoFreq >= 201) && (dcoFreq <= 250)) {
      intfreg1 &= (uint16_t)~DCO_FIX_SEL_MASK;
      reg = SPI_MEM_MAP_PLL(SOCPLLMACROREG3);
      reg = (uint16_t)(reg << 1); /* according to RTL bug SOCPLLMACROREG3 (read issue) will shift to one left shift */
      reg &= (uint16_t)~LDO_PROG_INTFPLL;
      reg |= 5 << 10;
      SPI_MEM_MAP_PLL(SOCPLLMACROREG3) = reg;
      intfPllMulFac                    = ((dcoFreq / 2) - 1);
      intfreg1 |= PLL_500_CLK_ENABLE; /* soc_pll_clk o/p en */
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG2) = (uint16_t)((intfPllDivFac << 9) | ((pllRefClk - 1) << 3));
      intfreg1 &= (uint16_t)~PLL_500_M_MASK;
      intfreg1 |= (uint16_t)((intfPllMulFac << 6) | PLL_500_CLK_ENABLE); /* m factor */
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) = intfreg1;
    } else {
      intfreg3 &= (uint16_t)~FCW_F_MASK;
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG3) = intfreg3;
      intfreg1 &= (uint16_t)~DCO_FIX_SEL_MASK;
      intfreg1 |= 1;
      reg = SPI_MEM_MAP_PLL(SOCPLLMACROREG3);
      reg = (uint16_t)(reg << 1);
      reg &= (uint16_t)~LDO_PROG_INTFPLL;
      reg |= 4 << 10;
      SPI_MEM_MAP_PLL(SOCPLLMACROREG3) = reg;
      intfreg1 |= PLL_500_CLK_ENABLE; /* soc_pll_clk o/p en */
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG2) = (uint16_t)((intfPllDivFac << 9) | ((pllRefClk - 1) << 3));
      intfreg1 &= (uint16_t)~PLL_500_M_MASK;
      intfreg1 |= (uint16_t)((intfPllMulFac << 6) | PLL_500_CLK_ENABLE); /* m factor */
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) = intfreg1;
    }
    /* Enable */

    clk_intf_pll_clk_set(pCLK);
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          rsi_error_t clk_intf_pll_set_freq_div(const M4CLK_Type *pCLK,
 *                                boolean_t clk_en,
 *                                uint16_t divFactor,
 *                                uint16_t nFactor,
 *                                uint16_t mFactor,
 *                                uint16_t fcwF,
 *                                uint16_t dcoFixSel,
 *                                uint16_t ldoProg)
 * @brief       This API is used to divide the Intf PLL clock frequency
 * @param[in]   pCLK      : pointer to the processor clock source
 * @param[in]   clk_en    : enable the clock for Intf PLL clock frequency
 * @param[in]   divFactor : division factor for Intf PLL clock frequency
 * @param[in]   nFactor   : n number of factor for Intf PLL clock frequency
 * @param[in]   mFactor   : m number of factor for Intf PLL clock frequency
 * @param[in]   fcwF      : frequency for Intf PLL clock frequency
 * @param[in]   dcoFixSel : fixed select for Intf PLL clock frequency
 * @param[in]   ldoProg   : ldo program for Intf PLL clock frequency 
 * @return      RSI_OK on success  
 */

rsi_error_t clk_intf_pll_set_freq_div(const M4CLK_Type *pCLK,
                                      boolean_t clk_en,
                                      uint16_t divFactor,
                                      uint16_t nFactor,
                                      uint16_t mFactor,
                                      uint16_t fcwF,
                                      uint16_t dcoFixSel,
                                      uint16_t ldoProg)
{
  uint16_t intfreg1      = 0x31c9;
  uint16_t intfreg3      = 0;
  uint16_t reg           = 0;
  uint16_t intfPllTvRead = 0;

  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  /* RESET PLL */
  clk_intf_pll_clk_reset();
  if ((MCU_RET->CHIP_CONFIG_MCU_READ_b.LIMIT_M4_FREQ_110MHZ_b == 1) || (M4_BBFF_STORAGE1 & BIT(10))) {
    if (clk_en) {
      intfreg3 &= (uint16_t)~FCW_F_MASK;
      intfreg3                                = (uint16_t)(fcwF << 2);
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG3) = intfreg3;
      intfreg1 &= (uint16_t)~DCO_FIX_SEL_MASK;
      intfreg1 |= 1;
      reg = SPI_MEM_MAP_PLL(SOCPLLMACROREG3);
      reg = (uint16_t)(reg << 1); /* according to RTL bug SOCPLLMACROREG3 (read issue) will shift to one left shift */
      reg &= (uint16_t)~LDO_PROG_INTFPLL;
      reg |= 1 << 10;
      SPI_MEM_MAP_PLL(SOCPLLMACROREG3) = reg;
      intfreg1 |= PLL_500_CLK_ENABLE; /* soc_pll_clk o/p en */
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG2) = (uint16_t)((divFactor << 9) | (nFactor << 3));
      intfreg1 &= (uint16_t)~PLL_500_M_MASK;
      intfreg1 |= (uint16_t)((mFactor << 6) | PLL_500_CLK_ENABLE); /* m factor */
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) = intfreg1;
    } else {
      intfreg1 &= (uint16_t)(~PLL_500_CLK_ENABLE);
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) = intfreg1; /* soc_pll_clk o/p disable */
    }
    /* Enable */
    clk_intf_pll_clk_set(pCLK);
    /* Set and clear(read modify write) PLL500CTRLREG7 BIT[4] :  TV value will be latched */

    SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG7) |= SPI_INP_RD_EN;
    SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG7) &= (uint16_t)(~(SPI_INP_RD_EN));

    /* Read the TV value from PLL500CTRLREG12  BIT[15 : 11] */
    intfPllTvRead = (SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG12) & 0xF800) >> 11;

    if ((intfPllTvRead <= ((M4_BBFF_STORAGE1 & 0x03E0) >> 5)) && (divFactor == 0)) {
      /* RESET PLL */
      clk_intf_pll_clk_reset();
      return ERROR_INVALID_INPUT_FREQUENCY;
    } else {
      /* Program the required frequency by user */
      return RSI_OK;
    }
  } else {
    if (clk_en) {
      intfreg3 &= (uint16_t)~FCW_F_MASK;
      intfreg3                                = (uint16_t)(fcwF << 2);
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG3) = intfreg3;
      intfreg1 &= (uint16_t)~DCO_FIX_SEL_MASK;
      intfreg1 |= dcoFixSel;
      reg = SPI_MEM_MAP_PLL(SOCPLLMACROREG3);
      reg = (uint16_t)(reg << 1); /* according to RTL bug SOCPLLMACROREG3 (read issue) will shift to one left shift */
      reg &= (uint16_t)~LDO_PROG_INTFPLL;
      reg |= (uint16_t)(ldoProg << 10);
      SPI_MEM_MAP_PLL(SOCPLLMACROREG3) = reg;
      intfreg1 |= PLL_500_CLK_ENABLE; /* soc_pll_clk o/p en */
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG2) = (uint16_t)((divFactor << 9) | (nFactor << 3));
      intfreg1 &= (uint16_t)~PLL_500_M_MASK;
      intfreg1 |= (uint16_t)((mFactor << 6) | PLL_500_CLK_ENABLE); /* m factor */
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) = intfreg1;
    } else {
      intfreg1 &= (uint16_t)(~PLL_500_CLK_ENABLE);
      SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) = intfreg1; /* soc_pll_clk o/p disable */
    }
    /* Enable */
    clk_intf_pll_clk_set(pCLK);
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_intf_pll_clk_bypass_enable(boolean_t clkEnable)  
 * @brief     This API is used to Enable bypass clock
 * @param[in] clkEnable : enable the clock 
 * @return    RSI_OK on success
 */

rsi_error_t clk_intf_pll_clk_bypass_enable(boolean_t clkEnable)
{
  if (clkEnable == Enable) {
    /* Enable PLL clock */
    SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) |= PLL_500_BYPASS;
  } else {
    /* Disable PLL clock */
    SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) &= (uint16_t)(~(PLL_500_BYPASS));
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn       rsi_error_t clk_intf_pll_turn_on()
 * @brief    This API is used to TurnOn the Intf_PLL
 * @return   RSI_OK on success
 */

rsi_error_t clk_intf_pll_turn_on()
{
  uint16_t intfreg1 = 0x31c9;

  intfreg1 &= (uint16_t)(~PLL_500_PD);  /* clearing pd */
  intfreg1 &= (uint16_t)(~PLL_500_RST); /* clearing reset */
  SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG1) = intfreg1;

  return RSI_OK;
}

/*==============================================*/
/**
 * @fn       rsi_error_t clk_intf_pll_clk_reset(void)
 * @brief    This API is used to Reset the Intf_PLL
 * @return   RSI_OK on success
 */

rsi_error_t clk_intf_pll_clk_reset(void)
{
  SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG11) = 0x01FF;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_intf_pll_clk_set(const M4CLK_Type *pCLK)
 * @brief     This API is used to Enable the Intf_PLL
 * @param[in] pCLK  : pointer to the processor clock source
 * @return    RSI_OK on success
 */

rsi_error_t clk_intf_pll_clk_set(const M4CLK_Type *pCLK)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG11) = 0xFFFF;
  while ((pCLK->PLL_STAT_REG_b.INTFPLL_LOCK) != 1)
    ; // checking for pll lck
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_peripheral_clk_enable1(M4CLK_Type *pCLK, uint32_t flags)
 * @brief     This API is used to Enable the peripheral cloks for SET1 register
 * @param[in] pCLK  : pointer to the processor clock source
 * @param[in] flags : flags for SET1 register
 * @return    RSI_OK on success
 */

rsi_error_t clk_peripheral_clk_enable1(M4CLK_Type *pCLK, uint32_t flags)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pCLK->CLK_ENABLE_SET_REG1 = flags;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_peripheral_clk_disable1(M4CLK_Type *pCLK, uint32_t flags)
 * @brief      This API is used to disable the peripheral cloks for CLR1 register
 * @param[in]  pCLK   : pointer to the processor clock source
 * @param[in]  flags  : flags for CLR1 register
 * @return     RSI_OK on success
 */

rsi_error_t clk_peripheral_clk_disable1(M4CLK_Type *pCLK, uint32_t flags)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pCLK->CLK_ENABLE_CLEAR_REG1 = flags;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_peripheral_clk_enable2(M4CLK_Type *pCLK, uint32_t flags)
 * @brief      This API is used to Enable the peripheral cloks for SET2 register
 * @param[in]  pCLK  : pointer to the processor clock source
 * @param[in]  flags : flags for SET2 register
 * @return     RSI_OK on success
 */

rsi_error_t clk_peripheral_clk_enable2(M4CLK_Type *pCLK, uint32_t flags)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pCLK->CLK_ENABLE_SET_REG2 = flags;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_peripheral_clk_disable2(M4CLK_Type *pCLK, uint32_t flags)
 * @brief     This API is used to disable the peripheral cloks for CLR2 register
 * @param[in] pCLK  : pointer to the processor clock source
 * @param[in] flags : flags for CLR2 register
 * @return    RSI_OK on success
 */

rsi_error_t clk_peripheral_clk_disable2(M4CLK_Type *pCLK, uint32_t flags)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pCLK->CLK_ENABLE_CLEAR_REG2 = flags;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_peripheral_clk_enable3(M4CLK_Type *pCLK, uint32_t flags)
 * @brief     This API is used to Enable the peripheral cloks for SET3 register
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] flags : flags for SET3 register
 * @return    RSI_OK on success
 */

rsi_error_t clk_peripheral_clk_enable3(M4CLK_Type *pCLK, uint32_t flags)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pCLK->CLK_ENABLE_SET_REG3 = flags;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn       rsi_error_t clk_peripheral_clk_disable3(M4CLK_Type *pCLK, uint32_t flags)
 * @brief    This API is used to disable the peripheral cloks for CLR3 register
 * @param[in] pCLK  : pointer to the processor clock source
 * @param[in] flags : flags for CLR3 register
 * @return    RSI_OK on success
 */

rsi_error_t clk_peripheral_clk_disable3(M4CLK_Type *pCLK, uint32_t flags)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pCLK->CLK_ENABLE_CLEAR_REG3 = flags;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_dynamic_clk_gate_disable(M4CLK_Type *pCLK, uint32_t flags)
 * @brief     This API is used to disable the dynamic clock gate for peripherals 
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] flags : flags for dynamic clock gate of peripherals 
 * @return    RSI_OK on success
 */

rsi_error_t clk_dynamic_clk_gate_disable(M4CLK_Type *pCLK, uint32_t flags)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pCLK->DYN_CLK_GATE_DISABLE_REG = (pCLK->DYN_CLK_GATE_DISABLE_REG) & (0xFFFFFFFF & ~flags);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn       rsi_error_t clk_dynamic_clk_gate_disable2(M4CLK_Type *pCLK, uint32_t flags)
 * @brief    This API is used to disable the dynamic clock gate for peripherals 
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] flags : flags for dynamic clock gate of peripherals 
 * @return    RSI_OK on success
 */

rsi_error_t clk_dynamic_clk_gate_disable2(M4CLK_Type *pCLK, uint32_t flags)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pCLK->DYN_CLK_GATE_DISABLE_REG2 = (pCLK->DYN_CLK_GATE_DISABLE_REG2) & (0xFFFFFFFF & ~flags);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_dynamic_clk_gate_enable(M4CLK_Type *pCLK, uint32_t flags)
 * @brief     This API is used to enable the dynamic clock gate for peripherals 
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] flags : flags for dynamic clock gate of peripherals 
 * @return    RSI_OK on success
 */

rsi_error_t clk_dynamic_clk_gate_enable(M4CLK_Type *pCLK, uint32_t flags)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pCLK->DYN_CLK_GATE_DISABLE_REG = (pCLK->DYN_CLK_GATE_DISABLE_REG | flags) & 0xFFFFFFFF;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_dynamic_clk_gate_enable2(M4CLK_Type *pCLK, uint32_t flags)
 * @brief     This API is used to enable the dynamic clock gate for peripherals 
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] flags : flags for dynamic clock gate of peripherals 
 * @return    RSI_OK on success 
 */

rsi_error_t clk_dynamic_clk_gate_enable2(M4CLK_Type *pCLK, uint32_t flags)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pCLK->DYN_CLK_GATE_DISABLE_REG2 = (pCLK->DYN_CLK_GATE_DISABLE_REG2 | flags) & 0xFFFFFFFF;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn       rsi_error_t clk_qspi_clk_config(M4CLK_Type *pCLK,
 *                          QSPI_CLK_SRC_SEL_T clkSource,
 *                          boolean_t swalloEn,
 *                          boolean_t OddDivEn,
 *                          uint32_t divFactor)
 * @brief    This API is used to configure the Qspi clocks 
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] clkSource : clock source for configure the Qspi clocks
 * @param[in] swalloEn : enable for Qspi clocks
 * @param[in] oddDivEn : enable for Qspi clocks
 * @param[in] divFactor : division factor for Qspi clocks
 * @return    clock spi on success           
 */

rsi_error_t clk_qspi_clk_config(M4CLK_Type *pCLK,
                                QSPI_CLK_SRC_SEL_T clkSource,
                                boolean_t swalloEn,
                                boolean_t OddDivEn,
                                uint32_t divFactor)
{
  rsi_error_t errorCode = RSI_OK;
  /* Parameter validation */
  if ((pCLK == NULL) || (divFactor > QSPI_MAX_CLK_DIVISION_FACTOR)) {
    return INVALID_PARAMETERS;
  }

  /* disabling the clocks */
  clk_peripheral_clk_disable(pCLK, QSPI_CLK);
  /* Select clock MUX */
  switch (clkSource) {
    case QSPI_ULPREFCLK:
      pCLK->CLK_CONFIG_REG1_b.QSPI_CLK_SEL = clkSource;
      break;

    case QSPI_INTFPLLCLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, INTF_PLL_CLK_PRESENT)) {
        errorCode = ERROR_CLOCK_NOT_ENABLED;
        break;
      } /* Update the clock MUX */
      pCLK->CLK_CONFIG_REG1_b.QSPI_CLK_SEL = 0x01;
      break;

    case QSPI_MODELPLLCLK2:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, MODEM_PLL_CLK_PRESENT)) {
        errorCode = ERROR_CLOCK_NOT_ENABLED;
        break;
      }
      /* Update the clock MUX */
      pCLK->CLK_CONFIG_REG1_b.QSPI_CLK_SEL = 0x02;
      break;

    case QSPI_SOCPLLCLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, SOC_PLL_CLK_PRESENT)) {
        errorCode = ERROR_CLOCK_NOT_ENABLED;
        break;
      }
      pCLK->CLK_CONFIG_REG1_b.QSPI_CLK_SEL = clkSource;
      break;

    case M4_SOCCLKNOSWLSYNCCLKTREEGATED:
      /* incase of qspi in sync with soc */
      pCLK->CLK_ENABLE_SET_REG3 = QSPI_M4_SOC_SYNC;
      break;

    default:
      errorCode = INVALID_PARAMETERS;
      break;
  }
  if (errorCode == RSI_OK) {
    /* wait for QSPI clock switched */
    while ((pCLK->PLL_STAT_REG_b.QSPI_CLK_SWITCHED) != true)
      ;

    /* update the division factor */
    pCLK->CLK_CONFIG_REG1_b.QSPI_CLK_DIV_FAC = (unsigned int)(divFactor & 0x3F);
    /* Specifies whether QSPI clock is in sync with Soc clock.
	  Before enabling this make sure that qspi_clk_onehot_enable is 1\92b0 to enable glitch free switching */
    /* Enable the QSPI clock */
    pCLK->CLK_CONFIG_REG1_b.QSPI_CLK_SWALLOW_SEL = swalloEn ? ENABLE : DISABLE;
    pCLK->CLK_CONFIG_REG2_b.QSPI_ODD_DIV_SEL     = OddDivEn ? ENABLE : DISABLE;
  }
  clk_peripheral_clk_enable(pCLK, QSPI_CLK, ENABLE_STATIC_CLK);
  return errorCode;
}
#if defined(SLI_SI917B0)
/*==============================================*/
/**
 * @fn       rsi_error_t clk_qspi_2_clk_config(M4CLK_Type *pCLK,
 *                          QSPI_CLK_SRC_SEL_T clkSource,
 *                          boolean_t swalloEn,
 *                          boolean_t OddDivEn,
 *                          uint32_t divFactor)
 * @brief    This API is used to configure the Qspi clocks 
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] clkSource : clock source for configure the Qspi clocks
 * @param[in] swalloEn : enable for Qspi clocks
 * @param[in] oddDivEn : enable for Qspi clocks
 * @param[in] divFactor : division factor for Qspi clocks
 * @return    clock spi on success           
 */

rsi_error_t clk_qspi_2_clk_config(M4CLK_Type *pCLK,
                                  QSPI_CLK_SRC_SEL_T clkSource,
                                  boolean_t swalloEn,
                                  boolean_t OddDivEn,
                                  uint32_t divFactor)
{

  rsi_error_t errorCode = RSI_OK;
  /* Parameter validation */
  if ((pCLK == NULL) || (divFactor > QSPI_MAX_CLK_DIVISION_FACTOR)) {
    return INVALID_PARAMETERS;
  }

  /* disabling the clocks */
  clk_peripheral_clk_disable(pCLK, QSPI_2_CLK);
  /* Select clock MUX */
  switch (clkSource) {
    case QSPI_ULPREFCLK:
      pCLK->CLK_CONFIG_REG6_b.QSPI_2_CLK_SEL = 0x00;
      break;

    case QSPI_MODELPLLCLK2:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, MODEM_PLL_CLK_PRESENT)) {
        errorCode = ERROR_CLOCK_NOT_ENABLED;
        break;
      }
      /* Update the clock MUX */
      pCLK->CLK_CONFIG_REG6_b.QSPI_2_CLK_SEL = 0x02;
      break;

    case QSPI_INTFPLLCLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, INTF_PLL_CLK_PRESENT)) {
        errorCode = ERROR_CLOCK_NOT_ENABLED;
        break;
      } /* Update the clock MUX */
      pCLK->CLK_CONFIG_REG6_b.QSPI_2_CLK_SEL = 0x01;
      break;

    case QSPI_SOCPLLCLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, SOC_PLL_CLK_PRESENT)) {
        errorCode = ERROR_CLOCK_NOT_ENABLED;
        break;
      }
      pCLK->CLK_CONFIG_REG6_b.QSPI_2_CLK_SEL = 0x03;
      break;

    case M4_SOCCLKNOSWLSYNCCLKTREEGATED:
      /* incase of qspi in sync with soc */
      pCLK->CLK_ENABLE_SET_REG1 = QSPI_2_M4_SOC_SYNC;
      break;

    default:
      errorCode = INVALID_PARAMETERS;
      break;
  }
  if (errorCode == RSI_OK) {
    /* wait for QSPI clock switched */
    while ((pCLK->PLL_STAT_REG_b.QSPI_2_CLK_SWITCHED) != 1)
      ;

    /* update the division factor */
    pCLK->CLK_CONFIG_REG6_b.QSPI_2_CLK_DIV_FAC = (unsigned int)(divFactor & 0x3F);
    /* Specifies whether QSPI clock is in sync with Soc clock.
	  Before enabling this make sure that qspi_clk_onehot_enable is 1\92b0 to enable glitch free switching */
    /* Enable the QSPI clock */
    if (swalloEn) {
      pCLK->CLK_CONFIG_REG6_b.QSPI_2_CLK_SWALLOW_SEL = 1;
    } else {
      pCLK->CLK_CONFIG_REG6_b.QSPI_2_CLK_SWALLOW_SEL = 0;
    }
    if (OddDivEn) {
      pCLK->CLK_CONFIG_REG6_b.QSPI_2_ODD_DIV_SEL = 1;
    } else {
      pCLK->CLK_CONFIG_REG6_b.QSPI_2_ODD_DIV_SEL = 0;
    }
  }
  clk_peripheral_clk_enable(pCLK, QSPI_2_CLK, ENABLE_STATIC_CLK);
  return errorCode;
}
#endif
/*==============================================*/
/**
* @fn        rsi_error_t clk_ssi_mst_clk_config(M4CLK_Type *pCLK,
*                              CLK_ENABLE_T clkType,
*                              SSI_MST_CLK_SRC_SEL_T clkSource,
*                              uint32_t divFactor)
* @brief     This API is used to configure the SSI clocks
* @param[in] pCLK : pointer to the processor clock source
* @param[in] clkType : clock type for SSI clocks
* @param[in] clkSource : cource clock  for SSI clocks
* @param[in] divFactor : division factor for SSI clocks
* @return    RSI_OK on success 
 */

rsi_error_t clk_ssi_mst_clk_config(M4CLK_Type *pCLK,
                                   CLK_ENABLE_T clkType,
                                   SSI_MST_CLK_SRC_SEL_T clkSource,
                                   uint32_t divFactor)
{
  /* Parameter validation */
  if ((pCLK == NULL) || (divFactor > SSI_MAX_CLK_DIVISION_FACTOR)) {
    return INVALID_PARAMETERS;
  }

  /* Disable the clock */
  clk_peripheral_clk_disable(pCLK, SSIMST_CLK);
  /* Master mode */
  switch (clkSource) {
    case SSI_ULPREFCLK:
      pCLK->CLK_CONFIG_REG1_b.SSI_MST_SCLK_SEL = 0x00;
      break;

    case SSI_SOCPLLCLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, SOC_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG1_b.SSI_MST_SCLK_SEL = 0x01;
      break;

    case SSI_MODEMPLLCLK1:

      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, MODEM_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG1_b.SSI_MST_SCLK_SEL = 0x02;
      break;

    case SSI_INTFPLLCLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, INTF_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG1_b.SSI_MST_SCLK_SEL = 0x03;
      break;

    case SSI_MODELPLLCLK2:
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, MODEM_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG1_b.SSI_MST_SCLK_SEL = 0x04;
      break;

    case M4_SOCCLKFOROTHERCLKS:
      pCLK->CLK_ENABLE_SET_REG3                = M4_SOC_CLK_FOR_OTHER_ENABLE;
      pCLK->CLK_CONFIG_REG1_b.SSI_MST_SCLK_SEL = 0x05;
      break;

    default:
      return INVALID_PARAMETERS;
  }
  /* wait for clock switch */
  while ((pCLK->PLL_STAT_REG_b.SSI_MST_SCLK_SWITCHED) != 1)
    ;
  /* division factor */
  pCLK->CLK_CONFIG_REG1_b.SSI_MST_SCLK_DIV_FAC = (unsigned int)(divFactor & 0x0F);
  /* Enable the SSI clock */
  clk_peripheral_clk_enable(pCLK, SSIMST_CLK, clkType);
  return RSI_OK;
}
#if !defined(SLI_SI917)

/*==============================================*/
/**
 * @fn        rsi_error_t clk_sd_mem_clk_config(M4CLK_Type *pCLK, boolean_t swalloEn, SDMEM_CLK_SRC_SEL_T clkSource, uint32_t divFactor)
 * @brief     This API is used to configure the SdMem clocks
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] swalloEn : enable for SdMem clocks
 * @param[in] clkSource : clock source for SdMem clocks
 * @param[in] divFactor : division factor for SdMem clocks 
 * @return    RSI_OK on success
 */

rsi_error_t clk_sd_mem_clk_config(M4CLK_Type *pCLK,
                                  boolean_t swalloEn,
                                  SDMEM_CLK_SRC_SEL_T clkSource,
                                  uint32_t divFactor)
{
  /* Parameter validation */
  if ((pCLK == NULL) || (divFactor > SDMEM_MAX_CLK_DIVISION_FACTOR)) {
    return INVALID_PARAMETERS;
  }

  /* Disable the SDMEM clock */
  clk_peripheral_clk_disable(pCLK, SD_MEM_CLK);
  switch (clkSource) {
    case SDMEM_SOCPLLCLK:
      /* Check clock is present or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, SOC_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->SD_MEM_CLOCK_REG_b.SD_MEM_INTF_CLK_SEL = 0x00;
      break;

    case SDMEM_MODEMPLLCLK1:
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, MODEM_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->SD_MEM_CLOCK_REG_b.SD_MEM_INTF_CLK_SEL = 0x01;
      break;

    case SDMEM_INTFPLLCLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, INTF_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->SD_MEM_CLOCK_REG_b.SD_MEM_INTF_CLK_SEL = 0x02;
      break;

    case M4_SOCCLKFOROTHERCLKSSDMEM:
      pCLK->CLK_ENABLE_SET_REG3                    = M4_SOC_CLK_FOR_OTHER_ENABLE;
      pCLK->SD_MEM_CLOCK_REG_b.SD_MEM_INTF_CLK_SEL = 0x03;
      break;

    default:
      return INVALID_PARAMETERS;
  }
  /* wait for SD mem clock switch */
  while ((pCLK->PLL_STAT_REG_b.SD_MEM_INTF_CLK_SWITCHED) != 1)
    ;
  /* Update the division factor */
  pCLK->SD_MEM_CLOCK_REG_b.SD_MEM_INTF_CLK_DIV_FAC = divFactor;
  if (swalloEn) {
    pCLK->SD_MEM_CLOCK_REG_b.SD_MEM_INTF_CLK_SWALLOW_SEL = 1;
  } else {
    pCLK->SD_MEM_CLOCK_REG_b.SD_MEM_INTF_CLK_SWALLOW_SEL = 0;
  }
  /* Enable SD memory clock */
  clk_peripheral_clk_enable(pCLK, SD_MEM_CLK, ENABLE_STATIC_CLK);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_cci_clk_config(M4CLK_Type *pCLK, CCI_CLK_SRC_SEL_T clkSource, uint32_t divFactor, CLK_ENABLE_T clkType)
 * @brief     This API is used to configure the CCI clocks
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] clkSource : source clock for configure the CCI clocks
 * @param[in] clkType : clock type for configure the CCI clocks
 * @param[in] divFactor : division factor for configure the CCI clocks
 * @return    RSI_OK on success
 */

rsi_error_t clk_cci_clk_config(M4CLK_Type *pCLK, CCI_CLK_SRC_SEL_T clkSource, uint32_t divFactor, CLK_ENABLE_T clkType)
{
  /* Parameter validation */
  if ((pCLK == NULL) || (divFactor > CCI_MAX_CLK_DIVISION_FACTOR)) {
    return INVALID_PARAMETERS;
  }
  /* Enable CCI clock */
  clk_peripheral_clk_disable(pCLK, CCI_CLK);
  /* cci_sync_mode_enable_for_ams = 0 */
  MISC_CFG_MISC_CTRL &= ~CCI_SYNC_MODE;
  switch (clkSource) {
    case CCI_M4_SOC_CLK_FOR_OTHER_CLKS:
      pCLK->CLK_ENABLE_SET_REG3           = M4_SOC_CLK_FOR_OTHER_ENABLE;
      pCLK->CLK_CONFIG_REG4_b.CCI_CLK_SEL = 0x00;
      break;

    case CCI_INTF_PLL_CLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, INTF_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG4_b.CCI_CLK_SEL = 0x01;
      break;

    case CCI_M4_SOC_CLK_NO_SWL_SYNC_CLK_TREE:
      /* cci_sync_mode_enable_for_ams = 1 */
      MISC_CFG_MISC_CTRL |= CCI_SYNC_MODE;
      break;
    default:
      return INVALID_PARAMETERS;
  }
  /* update the division factor */
  pCLK->CLK_CONFIG_REG2_b.CCI_CLK_DIV_FAC = divFactor;
  /* Enable the CCI clock */
  clk_peripheral_clk_enable(pCLK, CCI_CLK, clkType);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_can_clk_config(M4CLK_Type *pCLK, uint32_t divFactor, CLK_ENABLE_T clkType)
 * @brief     This API is used to configure the Can clocks
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] divFactor : division factor for configure the Can clocks
 * @param[in] clkType : clock type for configure the Can clocks
 * @return    RSI_OK on success 
 */

rsi_error_t clk_can_clk_config(M4CLK_Type *pCLK, uint32_t divFactor, CLK_ENABLE_T clkType)
{
  if ((pCLK == NULL) || (divFactor > CAN_MAX_CLK_DIVISION_FACTOR)) {
    return INVALID_PARAMETERS;
  }
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  clk_peripheral_clk_disable(pCLK, CAN_CLK);

  pCLK->CLK_CONFIG_REG3_b.CAN1_CLK_DIV_FAC = divFactor;

  /* Disable the clock */
  clk_peripheral_clk_enable(pCLK, CAN_CLK, clkType);

  return RSI_OK;
}

/*==============================================*/
/**
 * @fn       rsi_error_t clk_ethernet_clk_config(M4CLK_Type *pCLK,
 *                              boolean_t swalloEn,
 *                              ETHERNET_CLK_SRC_SEL_T clkSource,
 *                              uint32_t divFactor)
 * @brief    This API is used to configure the PLL_INTF clock
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] swalloEn : enable for PLL_INTF clock
 * @param[in] clkSource : clock source for PLL_INTF clock
 * @param[in] divFactor : division factor for PLL_INTF clock
 * @return    RSI_OK on success 
 */

rsi_error_t clk_ethernet_clk_config(M4CLK_Type *pCLK,
                                    boolean_t swalloEn,
                                    ETHERNET_CLK_SRC_SEL_T clkSource,
                                    uint32_t divFactor)
{
  /* Parameter validation */
  if ((pCLK == NULL) || (divFactor > PLL_INTF_MAX_CLK_DIVISION_FACTOR)) {
    return INVALID_PARAMETERS;
  }
  /* Disable Ethernet clock */
  pCLK->CLK_ENABLE_CLEAR_REG2 = PLL_INTF_CLK_ENABLE;
  if (clkSource == ETH_INTF_PLL_CLK) {
    /* Check clock is present is or not before switching */
    if (RSI_OK != RSI_CLK_CheckPresent(pCLK, INTF_PLL_CLK_PRESENT)) {
      return ERROR_CLOCK_NOT_ENABLED;
    }
    pCLK->CLK_CONFIG_REG1_b.PLL_INTF_CLK_SEL = 0;
  } else {
    /* Check clock is present or not before switching */
    if (RSI_OK != RSI_CLK_CheckPresent(pCLK, SOC_PLL_CLK_PRESENT)) {
      return ERROR_CLOCK_NOT_ENABLED;
    }
    pCLK->CLK_CONFIG_REG1_b.PLL_INTF_CLK_SEL = 1;
  }
  while ((pCLK->PLL_STAT_REG_b.PLL_INTF_CLK_SWITCHED) != 1)
    ;
  /* Update the division factor */
  pCLK->CLK_CONFIG_REG1_b.PLL_INTF_CLK_DIV_FAC = divFactor;
  if (swalloEn) {
    pCLK->CLK_CONFIG_REG1_b.PLL_INTF_CLK_SWALLOW_SEL = 1;
  } else {
    pCLK->CLK_CONFIG_REG1_b.PLL_INTF_CLK_SWALLOW_SEL = 0;
  }
  pCLK->CLK_ENABLE_SET_REG2 = PLL_INTF_CLK_ENABLE;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn       rsi_error_t clk_cci_clk_div(M4CLK_Type *pCLK, uint32_t divFactor)
 * @brief    This API is used to divide the CCI clock
 * @param[in] pCLK : pointer to the processor clock source 
 * @param[in] divFactor : division factor for CCI clock 
 * @return    RSI_OK on success 
 */

rsi_error_t clk_cci_clk_div(M4CLK_Type *pCLK, uint32_t divFactor)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pCLK->CLK_CONFIG_REG2_b.CCI_CLK_DIV_FAC = divFactor;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_sd_mem_clk_div(M4CLK_Type *pCLK, boolean_t u8SwallowEn, uint32_t divFactor)
 * @brief     This API is used to divide the SDMEM clock
 * @param[in] pCLK : pointer to the processor clock source 
 * @param[in] u8SwallowEn : enable for SDMEM clock
 * @param[in] divFactor : division factor for SDMEM clock
 * @return    RSI_OK on success 
 */

rsi_error_t clk_sd_mem_clk_div(M4CLK_Type *pCLK, boolean_t u8SwallowEn, uint32_t divFactor)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }

  if (u8SwallowEn) {
    pCLK->SD_MEM_CLOCK_REG_b.SD_MEM_INTF_CLK_SWALLOW_SEL = 1;
  } else {
    pCLK->SD_MEM_CLOCK_REG_b.SD_MEM_INTF_CLK_SWALLOW_SEL = 0;
  }

  /* SDMEM division selection */
  pCLK->SD_MEM_CLOCK_REG_b.SD_MEM_INTF_CLK_DIV_FAC = divFactor;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn       rsi_error_t clk_usb_clk_config(M4CLK_Type *pCLK, USB_CLK_SRC_SEL_T clkSource, uint16_t divFactor)
 * @brief    This API is used to configure the USB clock
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] clkSource : source clock
 * @param[in] divFactor : division factor
 * @return    RSI_OK on success 
 */

rsi_error_t clk_usb_clk_config(M4CLK_Type *pCLK, USB_CLK_SRC_SEL_T clkSource, uint16_t divFactor)
{
  /* Parameter validation */
  if ((pCLK == NULL) || (divFactor > USB_MAX_CLK_DIVISION_FACTOR)) {
    return INVALID_PARAMETERS;
  }
  /* clock Disable */
  pCLK->CLK_ENABLE_CLEAR_REG3 = USB_PHY_CLK_IN_ENABLE;
  switch (clkSource) {
    case USB_MEMS_REF_CLK:
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, MODEM_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      /* mems_ref_clk from Modem PLL */
      TASS_PLL_CTRL_SET_REG(AFEPLLCTRLREG1) |= MEMS_REF_CLK_ENABLE;
      pCLK->CLK_CONFIG_REG5_b.USB_CLK_SEL = 0x00;
      break;

    case USB_REFERENCE_CLK:
      /* Reference clock */
      pCLK->CLK_CONFIG_REG5_b.USB_CLK_SEL = 0x01;
      break;
    case USB_PLL_CLK:
      /* usb_pll_clk from Modem PLL */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, MODEM_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG5_b.USB_CLK_SEL = 0x02;
      break;
    default:
      return INVALID_PARAMETERS;
  }
  /* Program the division factor */
  pCLK->CLK_CONFIG_REG6_b.USB_PHY_CLK_DIV_FAC = divFactor;
  /* clock Enable */
  pCLK->CLK_ENABLE_SET_REG3 = USB_PHY_CLK_IN_ENABLE;
  return RSI_OK;
}
#endif

/*==============================================*/
/**
 * @fn        rsi_error_t clk_ct_clk_config(M4CLK_Type *pCLK, CT_CLK_SRC_SEL_T clkSource, uint32_t divFactor, CLK_ENABLE_T clkType)
 * @brief     This API is used to configure the CT clocks
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] clkSource : source clock for CT clocks
 * @param[in] clkType : source clock for CT clocks
 * @param[in] divFactor : division factor for CT clocks
 * @return    RSI_OK on success
 */

rsi_error_t clk_ct_clk_config(M4CLK_Type *pCLK, CT_CLK_SRC_SEL_T clkSource, uint32_t divFactor, CLK_ENABLE_T clkType)
{
  /* Parameter validation */
  if ((pCLK == NULL) || (divFactor > CT_MAX_CLK_DIVISION_FACTOR)) {
    return INVALID_PARAMETERS;
  }
  /* Disable SCT clock */
  clk_peripheral_clk_disable(pCLK, CT_CLK);
  switch (clkSource) {
    case CT_ULPREFCLK:
      pCLK->CLK_CONFIG_REG5_b.CT_CLK_SEL = 0x00;
      break;

    case CT_INTFPLLCLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, INTF_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG5_b.CT_CLK_SEL = 0x01;
      break;

    case CT_SOCPLLCLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, SOC_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG5_b.CT_CLK_SEL = 0x02;
      break;

    case M4_SOCCLKFOROTHERCLKSCT:
      pCLK->CLK_ENABLE_SET_REG3          = M4_SOC_CLK_FOR_OTHER_ENABLE;
      pCLK->CLK_CONFIG_REG5_b.CT_CLK_SEL = 0x03;
      break;
    default:
      return INVALID_PARAMETERS;
  }
  /* wait for SCT switched */
  while ((pCLK->PLL_STAT_REG_b.CT_CLK_SWITCHED) != 1)
    ;
  /* Program the division factor */
  pCLK->CLK_CONFIG_REG5_b.CT_CLK_DIV_FAC = (unsigned int)(divFactor & 0x3F);
  clk_peripheral_clk_enable(pCLK, CT_CLK, clkType);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_i2s_clk_config(M4CLK_Type *pCLK, I2S_CLK_SRC_SEL_T clkSource, uint32_t divFactor)
 * @brief     This API is used to configure the I2S clocks
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] clkSource : source clock
 * @param[in] divFactor : division factor
 * @return    RSI_OK on success 
 */

rsi_error_t clk_i2s_clk_config(M4CLK_Type *pCLK, I2S_CLK_SRC_SEL_T clkSource, uint32_t divFactor)
{
  /* Parameter validation */
  if ((pCLK == NULL) || (divFactor > I2S_MAX_CLK_DIVISION_FACTOR)) {
    return INVALID_PARAMETERS;
  }
  /* Disable the I2S clock */
  clk_peripheral_clk_disable(pCLK, I2SM_CLK);
  MISC_CFG_MISC_CTRL1 |= I2S_MASTER_SLAVE_MODE;
  if (clkSource == I2S_PLLCLK) {
    /* Check clock is present is or not before switching */
    if (RSI_OK != RSI_CLK_CheckPresent(pCLK, I2S_PLL_CLK_PRESENT)) {
      return ERROR_CLOCK_NOT_ENABLED;
    }
    pCLK->CLK_CONFIG_REG5_b.I2S_CLK_SEL = 0;
  } else {
    pCLK->CLK_ENABLE_SET_REG3           = M4_SOC_CLK_FOR_OTHER_ENABLE;
    pCLK->CLK_CONFIG_REG5_b.I2S_CLK_SEL = 1;
  }

  /* Wait for I2S clock switch */
  while ((pCLK->PLL_STAT_REG_b.I2S_CLK_SWITCHED) != 1)
    ;
  /* update the division factor */
  pCLK->CLK_CONFIG_REG5_b.I2S_CLK_DIV_FAC = (unsigned int)(divFactor & 0x3F);
  /* enable the clock */
  clk_peripheral_clk_enable(pCLK, I2SM_CLK, ENABLE_STATIC_CLK);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_mcu_clk_cut_config(M4CLK_Type *pCLK, MCU_CLKOUT_SRC_SEL_T clkSource, uint32_t divFactor)
 * @brief     This API is used to configure the McuClkOut clocks
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] clkSource : source clock
 * @param[in] divFactor : division factor
 * @return    RSI_OK on success 
 */

rsi_error_t clk_mcu_clk_cut_config(M4CLK_Type *pCLK, MCU_CLKOUT_SRC_SEL_T clkSource, uint32_t divFactor)
{
  /* Parameter validation */
  if ((pCLK == NULL) || (divFactor >= MCU_CLKOUT_MAX_CLK_DIVISION_FACTOR)) {
    return INVALID_PARAMETERS;
  }
  clk_peripheral_clk_disable(pCLK, MCUCLKOUT_CLK);
  /* clock out mux select */
  /* apply division factor */
  pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_DIV_FAC = (unsigned int)(divFactor & 0x3F);
  switch (clkSource) {
    case MCUCLKOUT_ULP_MHZ_RC_CLK:
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = 0x01;
      break;

    case MCUCLKOUT_RF_REF_CLK:
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = 0x02;
      break;

    case MCUCLKOUT_MEMS_REF_CLK:
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, MODEM_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      TASS_PLL_CTRL_SET_REG(AFEPLLCTRLREG1) |= MEMS_REF_CLK_ENABLE;
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = 0x03;
      break;

    case MCUCLKOUT_ULP_20MHZ_RINGOSC_CLK:
      ulpss_enable_ref_clks(MCU_ULP_20MHZ_RING_OSC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = 0x04;
      break;

    case MCUCLKOUT_ULP_DOUBLER_CLK:
      ulpss_enable_ref_clks(MCU_ULP_DOUBLER_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = 0x05;
      break;

    case MCUCLKOUT_ULP_32KHZ_RC_CLK:
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = 0x07;
      break;

    case MCUCLKOUT_ULP_32KHZ_XTAL_CLK:
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_XTAL_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = 0x08;
      break;

    case MCUCLKOUT_ULP_32KHZ_RO_CLK: /* 32 KHz RO clock is not supported */
      ulpss_enable_ref_clks(MCU_ULP_32KHZ_RO_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = 0x09;
      break;

    case MCUCLKOUT_INTF_PLL_CLK:
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, INTF_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = 0x0A;
      break;

    case MCUCLKOUT_MODEM_PLL_CLK1:
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, MODEM_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = 0x0B;
      break;

    case MCUCLKOUT_MODEM_PLL_CLK2:
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, MODEM_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = 0x0C;
      break;

    case MCUCLKOUT_SOC_PLL_CLK:
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, SOC_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = 0x0D;
      break;

    case MCUCLKOUT_I2S_PLL_CLK:
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, I2S_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = 0x0E;
      break;

    case MCUCLKOUT_USB_PLL_CLK:
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, MODEM_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = 0x0F;
      break;

    default:
      return INVALID_PARAMETERS;
  }
  clk_peripheral_clk_enable(pCLK, MCUCLKOUT_CLK, ENABLE_STATIC_CLK);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_m4_soc_clk_div(M4CLK_Type *pCLK, uint32_t divFactor)
 * @brief     This API is used to divide the M4soc  clock
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] divFactor : division factor
 * @return    RSI_OK on success 
 */

rsi_error_t clk_m4_soc_clk_div(M4CLK_Type *pCLK, uint32_t divFactor)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pCLK->CLK_CONFIG_REG5_b.M4_SOC_CLK_DIV_FAC = (unsigned int)(divFactor & 0x3F);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_qspi_clk_div(M4CLK_Type *pCLK, boolean_t u8SwallowEn, boolean_t u8OddDivEn, uint32_t divFactor)
 * @brief     This API is used to divide the QSPI clock
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] u8SwallowEn : enable
 * @param[in] u8OddDivEn : odd division enable
 * @param[in] divFactor : division factor
 * @return    RSI_OK on success 
 */

rsi_error_t clk_qspi_clk_div(M4CLK_Type *pCLK, boolean_t u8SwallowEn, boolean_t u8OddDivEn, uint32_t divFactor)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  if (u8SwallowEn) {
    pCLK->CLK_CONFIG_REG1_b.QSPI_CLK_SWALLOW_SEL = 1;
  } else {
    pCLK->CLK_CONFIG_REG1_b.QSPI_CLK_SWALLOW_SEL = 0;
  }
  if (u8OddDivEn) {
    pCLK->CLK_CONFIG_REG2_b.QSPI_ODD_DIV_SEL = 1;
  } else {
    pCLK->CLK_CONFIG_REG2_b.QSPI_ODD_DIV_SEL = 0;
  }
  pCLK->CLK_CONFIG_REG1_b.QSPI_CLK_DIV_FAC = (unsigned int)(divFactor & 0x3F);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_ct_clk_div(M4CLK_Type *pCLK, uint32_t divFactor)
 * @brief      This API is used to divide the CT clock
 * @param[in]  pCLK : pointer to the processor clock source
 * @param[in]  divFactor : division factor
 * @return     RSI_OK on success
 */

rsi_error_t clk_ct_clk_div(M4CLK_Type *pCLK, uint32_t divFactor)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pCLK->CLK_CONFIG_REG5_b.CT_CLK_DIV_FAC = (unsigned int)(divFactor & 0x3F);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_ssi_mst_clk_div(M4CLK_Type *pCLK, uint32_t divFactor)
 * @brief      This API is used to divide the SSI clock
 * @param[in]  pCLK : pointer to the processor clock source
 * @param[in]  divFactor : division factor
 * @return     RSI_OK on success
 */

rsi_error_t clk_ssi_mst_clk_div(M4CLK_Type *pCLK, uint32_t divFactor)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pCLK->CLK_CONFIG_REG1_b.SSI_MST_SCLK_DIV_FAC = (unsigned int)(divFactor & 0x0F);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_i2s_clk_div(M4CLK_Type *pCLK, uint32_t divFactor)
 * @brief     This API is used to divide the I2S clock
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] divFactor : division factor
 * @return    RSI_OK on success
 */

rsi_error_t clk_i2s_clk_div(M4CLK_Type *pCLK, uint32_t divFactor)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  pCLK->CLK_CONFIG_REG5_b.I2S_CLK_DIV_FAC = (unsigned int)(divFactor & 0x3F);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_usart_clk_div(M4CLK_Type *pCLK, EN_USART_T enUsart, boolean_t u8FracDivEn, uint32_t divFactor)
 * @brief      This API is used to divide the USART/UART  clock
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] enUsart : enable UART
 * @param[in] u8FracDivEn : fractional divison enable
 * @param[in] divFactor : division factor
 * @return    RSI_OK on success
 */

rsi_error_t clk_usart_clk_div(M4CLK_Type *pCLK, EN_USART_T enUsart, boolean_t u8FracDivEn, uint32_t divFactor)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  if (enUsart == USART1) {
    if (u8FracDivEn) {
      pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_FRAC_SEL = 1;
    } else {
      pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_FRAC_SEL = 0;
    }
    pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_DIV_FAC = (unsigned int)(divFactor & 0x0F);
  } else {
    if (u8FracDivEn) {
      pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_FRAC_SEL = 1;
    } else {
      pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_FRAC_SEL = 0;
    }
    pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_DIV_FAC = (unsigned int)(divFactor & 0x0F);
  }

  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_xtal_clk_config(uint8_t xtalPin)
 * @brief      This API is used to configure the Xtal clock 
 * @param[in]  xtalPin : oscillator pin
 * @return     RSI_OK on success 
 */

rsi_error_t clk_xtal_clk_config(uint8_t xtalPin)
{
  if (xtalPin > 4) {
    return INVALID_PARAMETERS;
  }
  switch (xtalPin) {
    case 1:
      MCU_RET->NPSS_GPIO_CNTRL[xtalPin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_MODE = 3;
      MCU_RET->NPSS_GPIO_CNTRL[xtalPin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_REN  = 1;
      break;

    case 2:
      MCU_RET->NPSS_GPIO_CNTRL[xtalPin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_MODE = 4;
      MCU_RET->NPSS_GPIO_CNTRL[xtalPin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_REN  = 1;
      break;

    case 3:
      MCU_RET->NPSS_GPIO_CNTRL[xtalPin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_MODE = 5;
      MCU_RET->NPSS_GPIO_CNTRL[xtalPin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_REN  = 1;
      break;

    case 4:
      MCU_RET->NPSS_GPIO_CNTRL[xtalPin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_MODE = 6;
      MCU_RET->NPSS_GPIO_CNTRL[xtalPin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_REN  = 1;
      break;

    default:
      // Handle unexpected xtalPin values
      // You can log an error, set a default configuration, or take other appropriate actions
      break;
  }
  // enables the XTAL clock
  MCU_AON->MCUAON_GEN_CTRLS_b.XTAL_CLK_FROM_GPIO = 1;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_slp_clk_config(M4CLK_Type *pCLK, SLEEP_CLK_SRC_SEL_T clkSrc)
 * @brief      This API is used to calibrate the sleep clock
 * @param[in]  pCLK : pointer to the processor clock source
 * @param[in]  clkEnable : enable the clock
 * @param[in]  clkSrc : source clock
 * @return     RSI_OK on success
 */

rsi_error_t clk_slp_clk_config(M4CLK_Type *pCLK, SLEEP_CLK_SRC_SEL_T clkSrc)
{
  /* Parameter validation */
  if ((pCLK == NULL) || (clkSrc > SLP_MAX_SEL)) {
    return INVALID_PARAMETERS;
  }
  switch (clkSrc) {
    case SLP_ULP_32KHZ_RC_CLK:
      /* Enable clock */
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_32KHZ_RC_CLK_EN_b = 1;
      pCLK->CLK_CONFIG_REG4_b.SLEEP_CLK_SEL                                 = clkSrc;
      break;
    case SLP_ULP_32KHZ_XTAL_CLK:
      /* Enable clock */
      /* NOTE: In order to enable the Xtal clk source need to configure the NPSS_GPIO pins
    	which can be done through clk_xtal_clk_config(uint8_t xtalPin) API i.e we need to call that API first */
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_32KHZ_XTAL_CLK_EN_b = 1;
      pCLK->CLK_CONFIG_REG4_b.SLEEP_CLK_SEL                                   = clkSrc;
      break;

    case SLP_CLK_GATED:
      /* default value i.e, clock is gated */
      pCLK->CLK_CONFIG_REG4_b.SLEEP_CLK_SEL = clkSrc;
      break;

    case SLP_ULP_32KHZ_RO_CLK: /* 32 kHz RO clock is not supported */
      /*Enable clock*/
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_32KHZ_RO_CLK_EN_b = 1;
      pCLK->CLK_CONFIG_REG4_b.SLEEP_CLK_SEL                                 = clkSrc;
      break;

    default:
      return INVALID_PARAMETERS;
  }
  while ((pCLK->PLL_STAT_REG_b.SLEEP_CLK_SWITCHED) != 1)
    ;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         uint32_t clk_slp_clk_calib_config(M4CLK_Type *pCLK, uint8_t clkCycles)
 * @brief      This API is used to calibrate the sleep clock
 * @param[in]  pCLK : pointer to the processor clock source
 * @param[in]  clkCycles : clock cycle
 * @return     pCLK on success
 */

uint32_t clk_slp_clk_calib_config(M4CLK_Type *pCLK, uint8_t clkCycles)
{
  if (pCLK == NULL || clkCycles > MAX_SLP_CYCLES) {
    return INVALID_PARAMETERS;
  }
  pCLK->SLEEP_CALIB_REG_b.SLP_CALIB_CYCLES = (unsigned int)(clkCycles & 0x03);
  /* Start the sleep clock */
  pCLK->SLEEP_CALIB_REG_b.SLP_CALIB_START_b = 1;
  /* Wait for calib done */
  while (pCLK->SLEEP_CALIB_REG_b.SLP_CALIB_DONE_b != 1)
    ;
  /* Return the value */
  return pCLK->SLEEP_CALIB_REG_b.SLP_CALIB_DURATION_b;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_gspi_clk_config(M4CLK_Type *pCLK, GSPI_CLK_SRC_SEL_T clkSel)
 * @brief     This API is used to configure the GSPI Clocks
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] clkSel : clock select
 * @return    RSI_OK on success
 */

rsi_error_t clk_gspi_clk_config(M4CLK_Type *pCLK, GSPI_CLK_SRC_SEL_T clkSel)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  /* Disable the clocks */
  pCLK->CLK_CONFIG_REG1_b.GEN_SPI_MST1_SCLK_SEL = 0x07;
  /* Without this clk enabled, div_fac and mux select for sclk cannot be programmed) */
  pCLK->CLK_ENABLE_SET_REG2 = GEN_SPI_MST1_HCLK_ENABLE;

  switch (clkSel) {
    case GSPI_M4_SOC_CLK_FOR_OTHER_CLKS:
      /* M4 SOC Clock for others enable */
      pCLK->CLK_ENABLE_SET_REG3                     = M4_SOC_CLK_FOR_OTHER_ENABLE;
      pCLK->CLK_CONFIG_REG1_b.GEN_SPI_MST1_SCLK_SEL = 0x00;
      break;

    case GSPI_ULP_REF_CLK:
      pCLK->CLK_CONFIG_REG1_b.GEN_SPI_MST1_SCLK_SEL = 0x01;
      break;

    case GSPI_SOC_PLL_CLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, SOC_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG1_b.GEN_SPI_MST1_SCLK_SEL = 0x02;
      break;

    case GSPI_MODEM_PLL_CLK2:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, MODEM_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG1_b.GEN_SPI_MST1_SCLK_SEL = 0x03;
      break;

    case GSPI_INTF_PLL_CLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, INTF_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      pCLK->CLK_CONFIG_REG1_b.GEN_SPI_MST1_SCLK_SEL = 0x04;
      break;
  }
  /* Wait for GSPI switched */
  while ((pCLK->PLL_STAT_REG_b.GEN_SPI_MST1_SCLK_SWITCHED) != 1)
    ;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_i2c_clk_config(M4CLK_Type *pCLK, boolean_t clkEnable, EN_I2C_T enI2C)
 * @brief      This API is used to configure the I2C clock
 * @param[in]  pCLK : pointer to the processor clock source
 * @param[in]  clkEnable : module for I2C clock
 * @param[in]  enI2C  : enable I2C bus for I2C clock
 * @return     RSI_OK on success
 */

rsi_error_t clk_i2c_clk_config(M4CLK_Type *pCLK, boolean_t clkEnable, EN_I2C_T enI2C)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  if (enI2C == I2C1_INSTAN) {
    if (clkEnable) {
      /* I2C 1 bus clock enable */
      pCLK->CLK_ENABLE_SET_REG2 = I2C_BUS_CLK_ENABLE;
      /* I2C clk enable */
      pCLK->CLK_ENABLE_SET_REG3 = I2C_CLK_ENABLE;
    } else {
      /* I2C bus clock disable */
      pCLK->CLK_ENABLE_CLEAR_REG2 = I2C_BUS_CLK_ENABLE;
      /* I2C clk disable */
      pCLK->CLK_ENABLE_CLEAR_REG3 = I2C_CLK_ENABLE;
    }
  } else {
    if (clkEnable) {
      /* I2C2 bus clock enable */
      pCLK->CLK_ENABLE_SET_REG2 = I2C_2_BUS_CLK_ENABLE;
      /* I2C2 clk enable */
      pCLK->CLK_ENABLE_SET_REG3 = I2C_2_CLK_ENABLE;
    } else {
      /* I2C2 bus clock disable */
      pCLK->CLK_ENABLE_CLEAR_REG2 = I2C_2_BUS_CLK_ENABLE;
      /* I2C2 clk disable */
      pCLK->CLK_ENABLE_CLEAR_REG3 = I2C_2_CLK_ENABLE;
    }
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t clk_peripheral_clk_enable(M4CLK_Type *pCLK, PERIPHERALS_CLK_T module, CLK_ENABLE_T clkType)
 * @brief     This API is used to enable the particular clock
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] module : module for particular clock
 * @param[in] clkType  : clock type for particular clock
 * @return    RSI_OK on success 
 */

rsi_error_t clk_peripheral_clk_enable(M4CLK_Type *pCLK, PERIPHERALS_CLK_T module, CLK_ENABLE_T clkType)
{
  /* valid parameter check */
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  switch (module) {
    case USART1_CLK:
      if (clkType == ENABLE_STATIC_CLK) {
        pCLK->CLK_ENABLE_SET_REG1 = (USART1_SCLK_ENABLE | USART1_PCLK_ENABLE);
      } else {
        pCLK->CLK_ENABLE_CLEAR_REG1                                     = (USART1_SCLK_ENABLE | USART1_PCLK_ENABLE);
        pCLK->DYN_CLK_GATE_DISABLE_REG_b.USART1_SCLK_DYN_CTRL_DISABLE_b = 0;
        pCLK->DYN_CLK_GATE_DISABLE_REG_b.USART1_PCLK_DYN_CTRL_DISABLE_b = 0;
      }
      break;
    case USART2_CLK:
      if (clkType == ENABLE_STATIC_CLK) {
        pCLK->CLK_ENABLE_SET_REG1 = (USART2_SCLK_ENABLE | USART2_PCLK_ENABLE);
      } else {
        pCLK->CLK_ENABLE_CLEAR_REG1                                     = (USART2_SCLK_ENABLE | USART2_PCLK_ENABLE);
        pCLK->DYN_CLK_GATE_DISABLE_REG_b.USART2_SCLK_DYN_CTRL_DISABLE_b = 0;
        pCLK->DYN_CLK_GATE_DISABLE_REG_b.USART2_PCLK_DYN_CTRL_DISABLE_b = 0;
      }
      break;
    case SSIMST_CLK:
      pCLK->CLK_ENABLE_SET_REG3 = M4_SOC_CLK_FOR_OTHER_ENABLE;
      if (clkType == ENABLE_STATIC_CLK) {
        pCLK->CLK_ENABLE_SET_REG2 = (SSI_MST_SCLK_ENABLE | SSI_MST_PCLK_ENABLE);
      } else {
        pCLK->CLK_ENABLE_CLEAR_REG2                                      = (SSI_MST_SCLK_ENABLE | SSI_MST_PCLK_ENABLE);
        pCLK->DYN_CLK_GATE_DISABLE_REG_b.SSI_MST_SCLK_DYN_CTRL_DISABLE_b = 0;
        pCLK->DYN_CLK_GATE_DISABLE_REG_b.SSI_MST_PCLK_DYN_CTRL_DISABLE_b = 0;
      }
      break;
    case SSISLAVE_CLK:
      pCLK->CLK_ENABLE_SET_REG3 = M4_SOC_CLK_FOR_OTHER_ENABLE;
      if (clkType == ENABLE_STATIC_CLK) {
        pCLK->CLK_ENABLE_SET_REG2 = (SSI_SLV_SCLK_ENABLE | SSI_SLV_PCLK_ENABLE);
      } else {
        pCLK->CLK_ENABLE_CLEAR_REG2                                      = (SSI_SLV_SCLK_ENABLE | SSI_SLV_PCLK_ENABLE);
        pCLK->DYN_CLK_GATE_DISABLE_REG_b.SSI_SLV_SCLK_DYN_CTRL_DISABLE_b = 0;
        pCLK->DYN_CLK_GATE_DISABLE_REG_b.SSI_SLV_PCLK_DYN_CTRL_DISABLE_b = 0;
      }
      break;

    case CT_CLK:
      pCLK->CLK_ENABLE_SET_REG1 = CT_CLK_ENABLE;
      if (clkType == ENABLE_STATIC_CLK) {
        pCLK->CLK_ENABLE_SET_REG1 = CT_PCLK_ENABLE;
      } else {
        pCLK->CLK_ENABLE_CLEAR_REG1                                  = CT_PCLK_ENABLE;
        pCLK->DYN_CLK_GATE_DISABLE_REG2_b.CT_PCLK_DYN_CTRL_DISABLE_b = 0;
      }
      break;
#if !defined(SLI_SI917)
    case SD_MEM_CLK:
      pCLK->CLK_ENABLE_SET_REG1 = SD_MEM_INTF_CLK_ENABLE;
      break;

    case CCI_CLK:
      pCLK->CLK_ENABLE_SET_REG1 = (CCI_CLK_ENABLE | CCI_HCLK_ENABLE);
      pCLK->CLK_ENABLE_SET_REG3 = M4_SOC_CLK_FOR_OTHER_ENABLE;
      if (clkType == ENABLE_STATIC_CLK) {
        pCLK->CLK_ENABLE_SET_REG1 = CCI_PCLK_ENABLE;
      } else {
        pCLK->CLK_ENABLE_CLEAR_REG1                                  = CCI_PCLK_ENABLE;
        pCLK->DYN_CLK_GATE_DISABLE_REG_b.CCI_PCLK_DYN_CTRL_DISABLE_b = 0;
      }
      break;
    case CAN_CLK:
      /* Enable the clock */
      pCLK->CLK_ENABLE_SET_REG2 = CAN1_CLK_ENABLE;
      pCLK->CLK_ENABLE_SET_REG3 = M4_SOC_CLK_FOR_OTHER_ENABLE;
      if (clkType == ENABLE_STATIC_CLK) {
        pCLK->CLK_ENABLE_SET_REG2 = CAN1_PCLK_ENABLE;
      } else {
        pCLK->CLK_ENABLE_CLEAR_REG2                                    = CAN1_PCLK_ENABLE;
        pCLK->DYN_CLK_GATE_DISABLE_REG2_b.CAN1_PCLK_DYN_CTRL_DISABLE_b = 0;
      }
      break;
#endif
    case QSPI_CLK:
      pCLK->CLK_ENABLE_SET_REG2 = (QSPI_CLK_ENABLE | QSPI_HCLK_ENABLE);
      pCLK->CLK_ENABLE_SET_REG3 = QSPI_CLK_ONEHOT_ENABLE;
      break;
#if defined(SLI_SI917B0)
    case QSPI_2_CLK:
      pCLK->CLK_ENABLE_SET_REG1 = (QSPI_2_CLK_ENABLE | QSPI_2_HCLK_ENABLE);
      pCLK->CLK_ENABLE_SET_REG1 = QSPI_2_CLK_ONEHOT_ENABLE;
      break;
#endif
    case RPDMA_CLK:
      pCLK->CLK_ENABLE_SET_REG1 = RPDMA_HCLK_ENABLE;
      break;
    case UDMA_CLK:
      pCLK->CLK_ENABLE_SET_REG2 = UDMA_HCLK_ENABLE;
      break;
    case PWM_CLK:
      pCLK->CLK_ENABLE_SET_REG2 = MCPWM_PCLK_ENABLE;
      break;
    case GSPI_CLK:
      pCLK->CLK_ENABLE_SET_REG3 = M4_SOC_CLK_FOR_OTHER_ENABLE;
      pCLK->CLK_ENABLE_SET_REG2 = GEN_SPI_MST1_HCLK_ENABLE;
      break;
    case EGPIO_CLK:
      pCLK->CLK_ENABLE_SET_REG3 = EGPIO_CLK_ENABLE;
      pCLK->CLK_ENABLE_SET_REG2 = EGPIO_PCLK_ENABLE;
      break;
    case ETHERNET_CLK:
      pCLK->CLK_ENABLE_SET_REG3 = M4_SOC_CLK_FOR_OTHER_ENABLE;
      pCLK->CLK_ENABLE_SET_REG1 = ETH_HCLK_ENABLE;
      break;
    case MCUCLKOUT_CLK:
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_ENABLE = 1;
      break;
    case HWRNG_CLK:
      pCLK->CLK_ENABLE_SET_REG1 = HWRNG_PCLK_ENABLE;
      break;
    case I2SM_CLK:
      pCLK->CLK_ENABLE_SET_REG2 = (I2SM_INTF_SCLK_ENABLE | I2SM_SCLK_ENABLE | I2SM_PCLK_ENABLE | I2SM_SCLK_ENABLE);
      break;
    default:
      return INVALID_PARAMETERS;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_peripheral_clk_disable(M4CLK_Type *pCLK, PERIPHERALS_CLK_T module)
 * @brief      This API is used to disable the particular clock
 * @param[in]  pCLK : pointer to the processor clock source
 * @param[in]  module : module
 * @return     RSI_OK on success
 */

rsi_error_t clk_peripheral_clk_disable(M4CLK_Type *pCLK, PERIPHERALS_CLK_T module)
{
  /* valid parameter check */
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  switch (module) {
    case USART1_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG1                                     = (USART1_SCLK_ENABLE | USART1_PCLK_ENABLE);
      pCLK->DYN_CLK_GATE_DISABLE_REG_b.USART1_SCLK_DYN_CTRL_DISABLE_b = 1;
      pCLK->DYN_CLK_GATE_DISABLE_REG_b.USART1_PCLK_DYN_CTRL_DISABLE_b = 1;
      break;
    case USART2_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG1                                     = (USART2_SCLK_ENABLE | USART2_PCLK_ENABLE);
      pCLK->DYN_CLK_GATE_DISABLE_REG_b.USART2_SCLK_DYN_CTRL_DISABLE_b = 1;
      pCLK->DYN_CLK_GATE_DISABLE_REG_b.USART2_PCLK_DYN_CTRL_DISABLE_b = 1;
      break;
    case SSIMST_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG2                                      = SSI_MST_SCLK_ENABLE;
      pCLK->DYN_CLK_GATE_DISABLE_REG_b.SSI_MST_SCLK_DYN_CTRL_DISABLE_b = 1;
      break;
    case SSISLAVE_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG2                                      = (SSI_SLV_SCLK_ENABLE | SSI_SLV_PCLK_ENABLE);
      pCLK->DYN_CLK_GATE_DISABLE_REG_b.SSI_SLV_SCLK_DYN_CTRL_DISABLE_b = 1;
      pCLK->DYN_CLK_GATE_DISABLE_REG_b.SSI_SLV_PCLK_DYN_CTRL_DISABLE_b = 1;
      break;
    case CT_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG1 = CT_CLK_ENABLE;
      break;
    case SD_MEM_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG1 = SD_MEM_INTF_CLK_ENABLE;
      break;
    case CCI_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG1 = CCI_CLK_ENABLE;
      pCLK->CLK_ENABLE_CLEAR_REG3 = M4_SOC_CLK_FOR_OTHER_ENABLE;
      break;
    case QSPI_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG2 = QSPI_CLK_ENABLE;
      pCLK->CLK_ENABLE_CLEAR_REG3 = (QSPI_CLK_ONEHOT_ENABLE | QSPI_M4_SOC_SYNC);
      break;
#if defined(SLI_SI917B0)
    case QSPI_2_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG1 = QSPI_2_CLK_ENABLE;
      pCLK->CLK_ENABLE_CLEAR_REG1 = (QSPI_2_CLK_ONEHOT_ENABLE | QSPI_2_M4_SOC_SYNC);
      break;
#endif
    case RPDMA_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG1 = RPDMA_HCLK_ENABLE;
      break;
    case UDMA_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG2 = UDMA_HCLK_ENABLE;
      break;
    case PWM_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG2 = MCPWM_PCLK_ENABLE;
      break;
    case CAN_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG2 = CAN1_CLK_ENABLE;
      break;
    case GSPI_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG2 = GEN_SPI_MST1_HCLK_ENABLE;
      break;
    case EGPIO_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG2 = EGPIO_PCLK_ENABLE;
      pCLK->CLK_ENABLE_CLEAR_REG3 = EGPIO_CLK_ENABLE;
      break;
    case ETHERNET_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG1 = ETH_HCLK_ENABLE;
      break;
    case MCUCLKOUT_CLK:
      pCLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_ENABLE = 0;
      break;
    case HWRNG_CLK:
      pCLK->CLK_ENABLE_CLEAR_REG1 = HWRNG_PCLK_ENABLE;
      break;
    case I2SM_CLK:
#if !defined(SLI_SI917)
      pCLK->DYN_CLK_GATE_DISABLE_REG_b.I2SM_INTF_SCLK_DYN_CTRL_DISABLE_b = 1;
#endif
      pCLK->CLK_ENABLE_CLEAR_REG2 = (I2SM_INTF_SCLK_ENABLE | I2SM_SCLK_ENABLE);
      break;
    default:
      return INVALID_PARAMETERS;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           void clk_config_pll_ref_clk(uint8_t ref_clk_src)
 * @brief        This API is used to Configures the PLL lock configurations
 * @param[in]    ref_clk_src  : reference clock source
 * @return       none
 */

void clk_config_pll_ref_clk(uint8_t ref_clk_src)
{
  uint32_t reg_read = 0;
  reg_read          = SPI_MEM_MAP_PLL(SOCPLLMACROREG2);
  reg_read &= ~((uint16_t)(0x3 << 14));
  reg_read |= (ref_clk_src << 14U);
  SPI_MEM_MAP_PLL(SOCPLLMACROREG2) = (uint16_t)reg_read;
}
#endif //PLL_ROMDRIVER_PRESENT

#if !defined(CHIP_9118) || !defined(A11_ROM) || !defined(PLL_ROMDRIVER_PRESENT)

/*==============================================*/
/**
 * @fn          void clk_config_pll_lock(boolean_t manual_lock, boolean_t bypass_manual_lock, uint8_t mm_count_limit)
 * @brief       This API is used to Configures the PLL lock configurations
 * @param[in]   manual_clock    : manual clock
 * @param[in]   bypass_manual_clock    : bypass manual clock
 * @param[in]   mm_count_limit    : count limit
 * @return      none
 */

void clk_config_pll_lock(boolean_t manual_lock, boolean_t bypass_manual_lock, uint8_t mm_count_limit)
{
  uint32_t reg_read = 0;
  reg_read          = SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG9);
  reg_read &= ~((0xFF << 6) | BIT(15) | BIT(14));
  reg_read |= (uint32_t)((manual_lock << 15U) | (bypass_manual_lock << 14U) | (mm_count_limit << 6U));
  SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG9) = (uint16_t)reg_read;
}
#endif

/*==============================================*/
/**
 * @fn             uint32_t RSI_CLK_CheckPresent(const M4CLK_Type *pCLK  ,CLK_PRESENT_T clkPresent)
 * @brief		   This API is used to enable the dynamic clock gate for peripherals
 * @param[in]	   pCLK       : Pointer to the pll register instance \ref M4CLK_Type
 * @param[in]	   clkPresent : structure variable of CLK_PRESENT_T , \ref CLK_PRESENT_T
 * @return         zero on success
 *                 RSI_OK on error error code
 *                 ERROR_CLOCK_NOT_ENABLED
 */

uint32_t RSI_CLK_CheckPresent(const M4CLK_Type *pCLK, CLK_PRESENT_T clkPresent)
{
  uint32_t errorReturn = 0;
  switch (clkPresent) {
    case SOC_PLL_CLK_PRESENT:
      if (pCLK->PLL_STAT_REG_b.SOCPLL_LOCK == 1) {
        errorReturn = RSI_OK;
      } else {
        errorReturn = ERROR_CLOCK_NOT_ENABLED;
      }
      break;
    case INTF_PLL_CLK_PRESENT:
      if (pCLK->PLL_STAT_REG_b.INTFPLL_LOCK == 1) {
        errorReturn = RSI_OK;
      } else {
        errorReturn = ERROR_CLOCK_NOT_ENABLED;
      }
      break;
    case I2S_PLL_CLK_PRESENT:
      if (pCLK->PLL_STAT_REG_b.I2SPLL_LOCK == 1) {
        errorReturn = RSI_OK;
      } else {
        errorReturn = ERROR_CLOCK_NOT_ENABLED;
      }
      break;
    case MODEM_PLL_CLK_PRESENT:
      if (pCLK->PLL_STAT_REG_b.MODEMPLL_LOCK == 1) {
        errorReturn = RSI_OK;
      } else {
        errorReturn = ERROR_CLOCK_NOT_ENABLED;
      }
      break;
  }
  return errorReturn;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_m4ss_ref_clk_config(const M4CLK_Type *pCLK, M4SS_REF_CLK_SEL_T clkSource)
 * @brief      This API is used to configure the m4ss_ref clocks
 * @param[in]  pCLK is pointer to the processor clock source
 * @param[in]  clkSource is source clock
 * @return     RSI_OK on success
 */

rsi_error_t clk_m4ss_ref_clk_config(const M4CLK_Type *pCLK, M4SS_REF_CLK_SEL_T clkSource)
{
  if (pCLK == NULL) {
    return INVALID_PARAMETERS;
  }
  switch (clkSource) {
    case ULP_MHZ_RC_BYP_CLK:
      MCU_FSM->MCU_FSM_REF_CLK_REG_b.M4SS_REF_CLK_SEL = clkSource;
      system_clocks.m4_ref_clock_source               = clkSource;
      system_clocks.m4ss_ref_clk                      = system_clocks.byp_rc_ref_clock;
      break;
    case ULP_MHZ_RC_CLK:
      MCU_FSM->MCU_FSM_REF_CLK_REG_b.M4SS_REF_CLK_SEL = clkSource;
      system_clocks.m4_ref_clock_source               = clkSource;
      system_clocks.m4ss_ref_clk                      = system_clocks.rc_mhz_clock;
      break;
    case EXT_40MHZ_CLK:
      MCU_FSM->MCU_FSM_REF_CLK_REG_b.M4SS_REF_CLK_SEL = clkSource;
      system_clocks.m4_ref_clock_source               = clkSource;
      system_clocks.m4ss_ref_clk                      = system_clocks.rf_ref_clock;
      break;
    case MEMS_REF_CLK:
      TASS_PLL_CTRL_SET_REG(AFEPLLCTRLREG1)           = MEMS_REF_CLK_ENABLE;
      MCU_FSM->MCU_FSM_REF_CLK_REG_b.M4SS_REF_CLK_SEL = clkSource;
      system_clocks.m4_ref_clock_source               = clkSource;
      system_clocks.m4ss_ref_clk                      = system_clocks.mems_ref_clock;
      break;
    case ULP_20MHZ_RINGOSC_CLK:
      /* Enable clock */
      ulpss_enable_ref_clks(MCU_ULP_20MHZ_RING_OSC_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      MCU_FSM->MCU_FSM_REF_CLK_REG_b.M4SS_REF_CLK_SEL = clkSource;
      system_clocks.m4_ref_clock_source               = clkSource;
      system_clocks.m4ss_ref_clk                      = system_clocks.ro_20mhz_clock;
      break;
    case ULP_DOUBLER_CLK:
      /* 6: ulp_doubler_clk */
      /* Enable clock */
      ulpss_enable_ref_clks(MCU_ULP_DOUBLER_CLK_EN, ULP_PERIPHERAL_CLK, 0);
      MCU_FSM->MCU_FSM_REF_CLK_REG_b.M4SS_REF_CLK_SEL = clkSource;
      system_clocks.m4_ref_clock_source               = clkSource;
      system_clocks.m4ss_ref_clk                      = system_clocks.doubler_clock;
      break;
    default:
      return INVALID_PARAMETERS;
  }
  /* wait for clock switched */
  while ((pCLK->PLL_STAT_REG_b.ULP_REF_CLK_SWITCHED) != true)
    ;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        rsi_error_t ulpss_disable_ref_clks(REF_CLK_ENABLE_T clk_type)
 * @brief     This API is used to enable the ULP reference clocks and provide delay for clock starting
 * @param[in] clk_Type : clock type 
 * @return    RSI_OK on success
 */

rsi_error_t ulpss_disable_ref_clks(REF_CLK_ENABLE_T clk_type)
{
  /* Select clock source */
  switch (clk_type) {
    case MCU_ULP_40MHZ_CLK_EN:
      /* Cleaners are disabled */
      *(volatile uint32_t *)0x2404811C |= (BIT(7) | BIT(23));
      *(volatile uint32_t *)0x2404811C &= ~(BIT(8) | BIT(24)); /////// API with XTAL Clock disabled ///////
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_40MHZ_CLK_EN_b = 0;
      break;

    case MCU_ULP_DOUBLER_CLK_EN:
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_DOUBLER_CLK_EN_b = 0;
      break;

    case MCU_ULP_20MHZ_RING_OSC_CLK_EN:
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_20MHZ_RING_OSC_CLK_EN_b = 0;
      break;

    case MCU_ULP_MHZ_RC_CLK_EN:
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_MHZ_RC_CLK_EN_b = 0;
      break;

    case MCU_ULP_32KHZ_XTAL_CLK_EN:
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_32KHZ_XTAL_CLK_EN_b = 0;
      break;
    case MCU_ULP_32KHZ_RO_CLK_EN: /* 32 KHz RO clock is not supported */
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_32KHZ_RO_CLK_EN_b = 0;
      break;

    case MCU_ULP_32KHZ_RC_CLK_EN:
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_32KHZ_RC_CLK_EN_b = 0;
      break;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn         rsi_error_t clk_m4_soc_clk_config(M4CLK_Type *pCLK, M4_SOC_CLK_SRC_SEL_T clkSource, uint32_t divFactor)
 * @brief      This API is used to configure the m4_soc clocks
 * @param[in]  pCLK : pointer to the processor clock source
 * @param[in]  clkSource : source clock
 * @param[in]  divFactor : division factor
 * @return     RSI_OK on success
 */

rsi_error_t clk_m4_soc_clk_config(M4CLK_Type *pCLK, M4_SOC_CLK_SRC_SEL_T clkSource, uint32_t divFactor)
{
  /* check valid parameters */
  if ((pCLK == NULL) || (divFactor >= SOC_MAX_CLK_DIVISION_FACTOR)) {
    return INVALID_PARAMETERS;
  }

  /* Added for MCU 100 MHz variant mode setting
	 * Clock will be max/2 in this mode */
  if (MCU_RET->CHIP_CONFIG_MCU_READ_b.LIMIT_M4_FREQ_110MHZ_b == 1) {
    divFactor = divFactor / 2;
  }
  /* Selects one of the clock sources for M4 SoC clock */
  switch (clkSource) {
    case M4_ULPREFCLK:
      /* Update the clock MUX */
      pCLK->CLK_CONFIG_REG5_b.M4_SOC_CLK_SEL = clkSource;
      SystemCoreClock                        = system_clocks.m4ss_ref_clk;
      break;

    case M4_SOCPLLCLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, SOC_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      /* Update the clock MUX */
      pCLK->CLK_CONFIG_REG5_b.M4_SOC_CLK_SEL = clkSource;
      SystemCoreClock                        = system_clocks.soc_pll_clock;
      break;

    case M4_MODEMPLLCLK1:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, MODEM_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      /* Update the clock MUX */
      pCLK->CLK_CONFIG_REG5_b.M4_SOC_CLK_SEL = clkSource;
      SystemCoreClock                        = system_clocks.modem_pll_clock;
      break;

    case M4_INTFPLLCLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, INTF_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      } /* Update the clock MUX */
      pCLK->CLK_CONFIG_REG5_b.M4_SOC_CLK_SEL = clkSource;
      SystemCoreClock                        = system_clocks.intf_pll_clock;
      break;

    case M4_SLEEPCLK:
      /* Check clock is present is or not before switching */
      if (ULPCLK->M4LP_CTRL_REG_b.ULP_M4_CORE_CLK_ENABLE_b == 1) {
        /* Update the clock MUX */
        pCLK->CLK_CONFIG_REG5_b.M4_SOC_CLK_SEL = clkSource;
      } else {
        /* If clock is not presented return the error */
        return ERROR_CLOCK_NOT_ENABLED;
      }
      SystemCoreClock = system_clocks.sleep_clock;
      break;
    default:
      return INVALID_PARAMETERS;
  }
  /* wait for clock switched */
  while ((pCLK->PLL_STAT_REG_b.M4_SOC_CLK_SWITCHED) != 1)
    ;
  /* update the division factor */
  pCLK->CLK_CONFIG_REG5_b.M4_SOC_CLK_DIV_FAC = (unsigned int)(divFactor & 0x3F);

  if (divFactor) {
    SystemCoreClock /= divFactor;
  }
  system_clocks.soc_clock = SystemCoreClock;
  return RSI_OK;
}

#if defined(CHIP_9118) || !defined(A11_ROM) || !defined(PLL_ROMDRIVER_PRESENT)

/*==============================================*/
/**
 * @fn            rsi_error_t clk_usart_clk_config(M4CLK_Type *pCLK,
 *                           CLK_ENABLE_T clkType,
 *                           boolean_t FracDivEn,
 *                           EN_USART_T enUsart,
 *                           USART_CLK_SRC_SEL_T clkSource,
 *                           uint32_t divFactor)
 * @brief         This API is used to configure the Usart clocks
 * @param[in]     pCLK : pointer to the processor clock source
 * @param[in]     clkType : enable the clock
 * @param[in]     FracDivEn : fractional clock divider
 * @param[in]     enUsart : used for enable Usart
 * @param[in]     clkSource: clock source
 * @param[in]     divFactor : division factor
 * @return        RSI_OK on success
 */

rsi_error_t clk_usart_clk_config(M4CLK_Type *pCLK,
                                 CLK_ENABLE_T clkType,
                                 boolean_t FracDivEn,
                                 EN_USART_T enUsart,
                                 USART_CLK_SRC_SEL_T clkSource,
                                 uint32_t divFactor)
{

  /* Parameter validation */
  if ((pCLK == NULL) || (divFactor > USART_MAX_CLK_DIVISION_FACTOR)) {
    return INVALID_PARAMETERS;
  }

  /* Disable the USART clock */
  if (enUsart == USART1) {
    RSI_CLK_PeripheralClkDisable(pCLK, USART1_CLK);
  } else {
    RSI_CLK_PeripheralClkDisable(pCLK, USART2_CLK);
  }
  /* Select clock MUX */
  switch (clkSource) {
    case USART_ULPREFCLK:
      if (enUsart == USART1) {
        /* select the MUX for the USART 1 */
        pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_SEL = 0x00;
        /* Wait for the clock MUX switch status */
        while ((pCLK->PLL_STAT_REG_b.USART1_SCLK_SWITCHED) != 1)
          ;
        /* Update the division factor */
        pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_DIV_FAC = (unsigned int)(divFactor & 0x0F);
        /* fractional clock divider select */
        if (FracDivEn) {
          pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_FRAC_SEL = 0;
        } else {
          pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_FRAC_SEL = 1;
        }
      } else if (enUsart == USART2) {
        /* select the MUX for the USART 1 */
        pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_SEL = 0x00;
        /* Wait for the clock MUX switch status */
        while ((pCLK->PLL_STAT_REG_b.USART2_SCLK_SWITCHED) != 1)
          ;
        /* fractional clock divider select */
        /* Update the division factor */
        pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_DIV_FAC = (unsigned int)(divFactor & 0x0F);

        if (FracDivEn) {
          pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_FRAC_SEL = 0;
        } else {
          pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_FRAC_SEL = 1;
        }
      }
      break;

    case USART_MODELPLLCLK2:
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, MODEM_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      if (enUsart == USART1) {
        /* select the MUX for the USART 1 */
        pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_SEL = 0x02;
        /* Wait for the clock MUX switch status */
        while ((pCLK->PLL_STAT_REG_b.USART1_SCLK_SWITCHED) != 1)
          ;
        /* Update the division factor */
        pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_DIV_FAC = (unsigned int)(divFactor & 0x0F);
        /* fractional clock divider select */
        if (FracDivEn) {
          pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_FRAC_SEL = 0;
        } else {
          pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_FRAC_SEL = 1;
        }
      } else if (enUsart == USART2) {
        /* select the MUX for the USART 1 */
        pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_SEL = 0x02;
        /* Wait for the clock MUX switch status */
        while ((pCLK->PLL_STAT_REG_b.USART2_SCLK_SWITCHED) != 1)
          ;
        /* fractional clock divider select */
        /* Update the division factor */
        pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_DIV_FAC = (unsigned int)(divFactor & 0x0F);
        if (FracDivEn) {
          pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_FRAC_SEL = 0;
        } else {
          pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_FRAC_SEL = 1;
        }
      }
      break;

    case USART_INTFPLLCLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, INTF_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      if (enUsart == USART1) {
        /* select the MUX for the USART 1 */
        pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_SEL = 0x03;
        /* Wait for the clock MUX switch status */
        while ((pCLK->PLL_STAT_REG_b.USART1_SCLK_SWITCHED) != 1)
          ;
        /* fractional clock divider select */
        /* Update the division factor */
        pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_DIV_FAC = (unsigned int)(divFactor & 0x0F);
        if (FracDivEn) {
          pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_FRAC_SEL = 0;
        } else {
          pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_FRAC_SEL = 1;
        }
      } else if (enUsart == USART2) {
        /* select the MUX for the USART 1 */
        pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_SEL = 0x03;
        /* Wait for the clock MUX switch status */
        while ((pCLK->PLL_STAT_REG_b.USART2_SCLK_SWITCHED) != 1)
          ;
        /* Update the division factor */
        pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_DIV_FAC = (unsigned int)(divFactor & 0x0F);

        /* fractional clock divider select */
        if (FracDivEn) {
          pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_FRAC_SEL = 0;
        } else {
          pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_FRAC_SEL = 1;
        }
      }
      break;

    case USART_SOCPLLCLK:
      /* Check clock is present is or not before switching */
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, SOC_PLL_CLK_PRESENT)) {
        return ERROR_CLOCK_NOT_ENABLED;
      }
      if (enUsart == USART1) {
        /* select the MUX for the USART 1 */
        pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_SEL = 0x01;
        /* Wait for the clock MUX switch status */
        while ((pCLK->PLL_STAT_REG_b.USART1_SCLK_SWITCHED) != 1)
          ;
        /* Update the division factor */
        pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_DIV_FAC = (unsigned int)(divFactor & 0x0F);
        /* fractional clock divider select */
        if (FracDivEn) {
          pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_FRAC_SEL = 0;
        } else {
          pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_FRAC_SEL = 1;
        }
      } else if (enUsart == USART2) {
        /* select the MUX for the USART 1 */
        pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_SEL = 0x01;
        /* Wait for the clock MUX switch status */
        while ((pCLK->PLL_STAT_REG_b.USART2_SCLK_SWITCHED) != 1)
          ;
        /* Update the division factor */
        pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_DIV_FAC = (unsigned int)(divFactor & 0x0F);
        /* fractional clock divider select */
        if (FracDivEn) {
          pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_FRAC_SEL = 0;
        } else {
          pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_FRAC_SEL = 1;
        }
      }
      break;

    case M4_SOCCLKFOROTHERCLOCKS:

      if (enUsart == USART1) {
        /* select the MUX for the USART 1 */
        pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_SEL = 0x04;
        pCLK->CLK_ENABLE_SET_REG3               = M4_SOC_CLK_FOR_OTHER_ENABLE;
        /* Wait for the clock MUX switch status */
        while ((pCLK->PLL_STAT_REG_b.USART1_SCLK_SWITCHED) != 1)
          ;
        /* Update the division factor */
        pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_DIV_FAC = (unsigned int)(divFactor & 0x0F);
        /* fractional clock divider select */
        if (FracDivEn) {
          pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_FRAC_SEL = 0;
        } else {
          pCLK->CLK_CONFIG_REG2_b.USART1_SCLK_FRAC_SEL = 1;
        }
      } else if (enUsart == USART2) {
        /* select the MUX for the USART 1 */
        pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_SEL = 0x04;
        pCLK->CLK_ENABLE_SET_REG3               = M4_SOC_CLK_FOR_OTHER_ENABLE;
        /* Wait for the clock MUX switch status */
        while ((pCLK->PLL_STAT_REG_b.USART2_SCLK_SWITCHED) != 1)
          ;
        /* Update the division factor */
        pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_DIV_FAC = (unsigned int)(divFactor & 0x0F);
        /* fractional clock divider select */
        if (FracDivEn) {
          pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_FRAC_SEL = 0;
        } else {
          pCLK->CLK_CONFIG_REG2_b.USART2_SCLK_FRAC_SEL = 1;
        }
      }
      break;
    default:
      return INVALID_PARAMETERS;
  }
  /* Enable USART clock */
  if (enUsart == USART1) {
    RSI_CLK_PeripheralClkEnable(pCLK, USART1_CLK, clkType);
  } else {
    RSI_CLK_PeripheralClkEnable(pCLK, USART2_CLK, clkType);
  }
  return RSI_OK;
}
#endif

/*==============================================*/
/**
* @fn         rsi_error_t ulpss_enable_ref_clks(REF_CLK_ENABLE_T enable, SRC_TYPE_T srcType, cdDelay delayFn)
* @brief      This API is used to enable the ULP reference clocks and provide delay for clock starting
* @param[in]  enable  : enable
* @param[in]  srcType : source type
* @param[in]  delayFn : delay function
* @return     RSI_OK on success
 */

rsi_error_t ulpss_enable_ref_clks(REF_CLK_ENABLE_T enable, SRC_TYPE_T srcType, cdDelay delayFn)
{
  /* Select clock source */
  switch (enable) {
    case MCU_ULP_40MHZ_CLK_EN:
      /* Enable the clock source */
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_40MHZ_CLK_EN_b = 1;

      /* Wait for the time out only in case of ULP processor clock configuration */
      if (srcType == ULP_PROCESSOR_CLK) {
        /*wait for clock source is enabled */
        _usdelay(MCU_ULP_40MHZ_CLK_EN_TURN_ON_DELAY, delayFn);
      }
      /* Cleaners are Enabled */
      *(volatile uint32_t *)0x2404811C &= ~(BIT(7) | BIT(23)); /////// API with XTAL Clock disabled ///////
      *(volatile uint32_t *)0x2404811C |= (BIT(8) | BIT(24));  /////// API with XTAL Clock disabled ///////
      break;

    case MCU_ULP_DOUBLER_CLK_EN:
      /* NOTE: 20MHz RC is interdependent on Doubler clock */
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_20MHZ_RING_OSC_CLK_EN_b = 1;
      /* Enable the clock source */
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_DOUBLER_CLK_EN_b = 1;

      if (srcType == ULP_PROCESSOR_CLK) {
        /* wait for clock source is enabled */
        _usdelay(MCU_ULP_DOUBLER_CLK_EN_TURN_ON_DELAY, delayFn);
      }
      break;

    case MCU_ULP_20MHZ_RING_OSC_CLK_EN:
      /* Enable the clock source */
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_20MHZ_RING_OSC_CLK_EN_b = 1;

      if (srcType == ULP_PROCESSOR_CLK) {
        /* wait for clock source is enabled */
        _usdelay(MCU_ULP_20MHZ_RING_OSC_CLK_EN_TURN_ON_DELAY, delayFn);
      }
      break;

    case MCU_ULP_MHZ_RC_CLK_EN:
      /* Enable the clock */
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_MHZ_RC_CLK_EN_b = 1;

      if (srcType == ULP_PROCESSOR_CLK) {
        /* wait for clock source is enabled */
        _usdelay(MCU_ULP_MHZ_RC_CLK_EN_TURN_ON_DELAY, delayFn);
      }
      break;

    case MCU_ULP_32KHZ_XTAL_CLK_EN:
      /* Program the IPMU structure */

      /* Enable the clock source from NPSS */
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_32KHZ_XTAL_CLK_EN_b = 1;

      /* Wait for 0.5 sec delay */
      _usdelay(MCU_ULP_32KHZ_XTAL_CLK_EN_TURN_ON_DELAY_1, delayFn);

      /* Program the IPMU structure */

      /* Wait for clock source to be enabled for 1.5 seconds */
      _usdelay(MCU_ULP_32KHZ_XTAL_CLK_EN_TURN_ON_DELAY_2, delayFn);
      break;

    case MCU_ULP_32KHZ_RO_CLK_EN:
      /* Enable the clock source */
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_32KHZ_RO_CLK_EN_b = 1;

      if (srcType == ULP_PROCESSOR_CLK) {
        /* wait for clock source is enabled */
        _usdelay(MCU_ULP_32KHZ_RO_CLK_EN_TURN_ON_DELAY, delayFn);
      }
      break;

    case MCU_ULP_32KHZ_RC_CLK_EN:
      /* Enable the clock source */
      MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_32KHZ_RC_CLK_EN_b = 1;

      if (srcType == ULP_PROCESSOR_CLK) {
        /* wait for clock source is enabled */
        _usdelay(MCU_ULP_32KHZ_RC_CLK_EN_TURN_ON_DELAY, delayFn);
      }
      break;
  }
  return RSI_OK;
}
/** @} */

/* End of file not truncated */
