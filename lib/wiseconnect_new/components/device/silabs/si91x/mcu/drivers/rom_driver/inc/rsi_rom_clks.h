/******************************************************************************
* @file  rsi_rom_clks.h
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

//Includes

/**
 * @defgroup RSI_SPECIFIC_DRIVERS SoC Device-Specific Drivers
 * @ingroup SOC
 * @section Description 
 * RSI specific drivers are unique to a specific device. RSI specific drivers may use an IP driver as it's base driver or a custom implementation if that 
 * peripheral or IP on the chip is unique (ie, clocking)
 */

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 */

/**
 * \defgroup RSI_CHIP_CLOCK_DRIVERS 
 *  @{
 *
 */

/**
 * \defgroup RSI_M4SS_CLOCK_DRIVERS M4SS CLOCK 
 *  @{
 *
 */

#ifndef __RSI_ROM_CLK_H_
#define __RSI_ROM_CLK_H_

#include "rsi_ccp_user_config.h"
#include "rsi_packing.h"
#if defined(A11_ROM)
#include "rsi_rom_table_si91x.h"
#else
#include "rsi_rom_table_RS1xxxx.h"
#endif

#if SL_WIFI_COMPONENT_INCLUDED
#include "sl_rsi_utility.h"
#include "rsi_m4.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @fn           STATIC INLINE boolean_t RSI_CLK_CheckPllLock(PLL_TYPE_T  pllType)
 * @brief		     This API is used to check the lock status of pll
 * @param[in]	   pllType     :  To select the soc_pll, intf_pll and i2s_pll. See # PLL_TYPE_T for more info
 *	                            \n structure member is below.
 *                                   -    SOC_PLL  :  soc_pll clk
 *                                   -    INTF_PLL :  intf_pll clk
 *                                   -    I2S_PLL  :  i2s_pll clk
 * @return 		   return 1 then for lock status high(enable) and return 0 then for lock status low(disable)
 *
 */
STATIC INLINE boolean_t RSI_CLK_CheckPllLock(PLL_TYPE_T pllType)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_check_pll_lock(pllType);
#else
  return clk_check_pll_lock(pllType);
#endif
}

/**
 * @fn           STATIC INLINE rsi_error_t RSI_CLK_SocPllClkEnable(boolean_t clkEnable)
 * @brief		     This API is used to enable the SoC-PLL output clock
 * @param[in]	   clkEnable  : Enum value to enable or disable the clock
 *                            - Enable  : Enables clock
 *	                          - Disable : Disables clock
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_SocPllClkEnable(boolean_t clkEnable)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_soc_pll_clk_enable(clkEnable);
#else
  return clk_soc_pll_clk_enable(clkEnable);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_SocPllTurnOn()
 * @brief       This API is used to TurnOn the SOC_PLL
 * @return      returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_SocPllTurnOn()
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_soc_pll_turn_on();
#else
  return clk_soc_pll_turn_on();
#endif
}
/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_SetSocPllFreq(const M4CLK_Type *pCLK,uint32_t socPllFreq,uint32_t pllRefClk)
 * @brief		    This API is used to set the Soc PLL clock to particular frequency
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  socPllFreq : Frequency value in Mhz for Soc_Pll_Clk .
 * @param[in]	  pllRefClk  :  Frequency value in Mhz for Reference clk. Please refer # NOTE
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * @note        Only 1Mhz steps applicable to the this API, 0.96Mhz steps are not supported
 */
STATIC INLINE rsi_error_t RSI_CLK_SetSocPllFreq(const M4CLK_Type *pCLK, uint32_t socPllFreq, uint32_t pllRefClk)
{
  rsi_error_t ret             = (rsi_error_t)0;
  system_clocks.soc_pll_clock = socPllFreq;

  /* TurnON the SOC_PLL */
  RSI_CLK_SocPllTurnOn();
  if (pllRefClk == XTAL_CLK_FREQ) //avoid if XTAL req is already done
  {
#if SL_WIFI_COMPONENT_INCLUDED
    /*  Notify NWP that M4 requires XTAL clock source  */
    sli_si91x_xtal_turn_on_request_from_m4_to_TA();
#endif
    PLL_REF_CLK_CONFIG_REG &= SELECT_XTAL_MHZ_CLOCK; // Selecting the XTAL as PLL reference clock
  }

  SPI_MEM_MAP_PLL(SOC_PLL_500_CTRL_REG9) = 0xD900;
#if defined(CLOCK_ROMDRIVER_PRESENT)
  ret = ROMAPI_M4SS_CLK_API->clk_set_soc_pll_freq(pCLK, socPllFreq, pllRefClk);
#else

  ret = clk_set_soc_pll_freq(pCLK, socPllFreq, pllRefClk);
#endif
  if (ret != RSI_OK) {
    return ret;
  } else {
    socPllFreq /= 1000000;
    /* If SOC PLL frequency is less than 90MHz, configure LDO voltage based on preprocessor setting */
    if (socPllFreq < 90) {
#if defined(SL_SI91X_ENABLE_LDO_LOW_VOLTAGE) && (SL_SI91X_ENABLE_LDO_LOW_VOLTAGE == 1)
      /* Option 1: Enable low voltage mode (PS4 to PS3 transition)
        * This mode uses lower LDO voltage for reduced power consumption.
        * To enable this mode, define SL_SI91X_ENABLE_LDO_LOW_VOLTAGE=1 in preprocessor settings.
        */
      /* Change the power state from PS4 to PS3 (sets SOC LDO to LOW voltage) */
      RSI_PS_PowerStateChangePs4toPs3();
      /* Configure DCDC to give lower output voltage */
      RSI_PS_SetDcDcToLowerVoltage();
#else
      /* Option 2: Default mode - High voltage mode (PS3 to PS4 transition)
        * This is the default behavior when SL_SI91X_ENABLE_LDO_LOW_VOLTAGE is not defined.
        * Uses higher LDO voltage (1.15V) for better performance.
        */
      /* Configure DCDC to give higher output voltage (1.15V) */
      RSI_PS_SetDcDcToHigerVoltage();
      /* Change the power state from PS3 to PS4 (sets SOC LDO to HIGH voltage) */
      RSI_PS_PowerStateChangePs3toPs4();
#endif
    }
    if ((socPllFreq >= 90) && (!(BATT_FF->MCU_PMU_LDO_CTRL_CLEAR & MCU_SOC_LDO_LVL))) {
      /* Change the power state from PS3 to PS4 */
      RSI_PS_SetDcDcToHigerVoltage();
      /* Configure DCDC to give higher output voltage.*/
      RSI_PS_PowerStateChangePs3toPs4();
    }
  }
  return RSI_OK;
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_SocPllSetFreqDiv(const M4CLK_Type *pCLK , boolean_t clk_en,uint16_t
 *                                 divFactor,uint16_t nFactor,uint16_t mFactor,uint16_t fCwf,
 *                                uint16_t dcofixsel,uint16_t ldoprog)
 * @brief		    This API is used to configure the SOC PLL clock frequency
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  clk_en : To enable the soc_pll_clk output enable
 * @param[in]	  divFactor : PLL post division factor
 * @param[in]   nFactor : N factor of PLL
 * @param[in]   mFactor : M factor of PLL
 * @param[in]   fCwf : Fractional Frequency Control Word. For below 200MHz fcwF is 0 and above 200Mhz if the frequency is odd program FCW_F as 8192
 * @param[in]   dcofixsel : Dco Fixed Ring select. Please refer # Note
 * @param[in]   ldoprog : SOCPLL LDO output voltage select. Please refer # Note
 * @return 		  returns zero \ref RSI_OK (0) on success ,on failure return error code.
 * @note        For <= 200Mhz ---> ldo_prog =4 and dco_fix_sel=1
 *              - For 201-250Mhz ---> ldo_prog =5 and dco_fix_sel=0
 *              - For >=251Mhz ---> ldo_prog =5 and dco_fix_sel=2
 */
STATIC INLINE rsi_error_t RSI_CLK_SocPllSetFreqDiv(const M4CLK_Type *pCLK,
                                                   boolean_t clk_en,
                                                   uint16_t divFactor,
                                                   uint16_t nFactor,
                                                   uint16_t mFactor,
                                                   uint16_t fCwf,
                                                   uint16_t dcofixsel,
                                                   uint16_t ldoprog)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API
    ->clk_soc_pll_set_freq_div(pCLK, clk_en, divFactor, nFactor, mFactor, fCwf, dcofixsel, ldoprog);
#else
  return clk_soc_pll_set_freq_div(pCLK, clk_en, divFactor, nFactor, mFactor, fCwf, dcofixsel, ldoprog);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_SocPllClkSet(const M4CLK_Type *pCLK)
 * @brief		    This API is used to Enables the SoC-PLL
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_SocPllClkSet(const M4CLK_Type *pCLK)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_soc_pll_clk_set(pCLK);
#else
  return clk_soc_pll_clk_set(pCLK);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_SocPllClkBypassEnable(boolean_t clkEnable)
 * @brief		    This API is used to enable bypass clock
 * @param[in]	  clkEnable : Enum value to enable or disable the clock
 *                          - Enable (1) : Enables bypass clock
 *	                        - Disable (0) : Disables bypass clock
 * @return 		  returns zero \ref RSI_OK on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_SocPllClkBypassEnable(boolean_t clkEnable)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_soc_pll_clk_bypass_enable(clkEnable);
#else
  return clk_soc_pll_clk_bypass_enable(clkEnable);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_SocPllClkReset()
 * @brief		    This API is used to Reset the Soc_pll_clk
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_SocPllClkReset()
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_soc_pll_clk_reset();
#else
  return clk_soc_pll_clk_reset();
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_SocPllPdEnable(boolean_t en)
 * @brief		    This API is used to enable the PdEnable(power down)
 * @param[in]   en : Enable or disable the PdEnable
 *                  -  Enable  : Enables bypass clock
 *	                - Disable : Disables bypass clock
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_SocPllPdEnable(boolean_t en)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_soc_pll_pd_enable(en);
#else
  return clk_soc_pll_pd_enable(en);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_SocPllTurnOff()
 * @brief		    This API is used to TurnOff the SOC_PLL
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_SocPllTurnOff()
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_soc_pll_turn_off();
#else
  return clk_soc_pll_turn_off();
#endif
}

/**
 * @fn  	       STATIC INLINE rsi_error_t RSI_CLK_I2sPllClkEnable(boolean_t clkEnable)
 * @brief		     This API is used to enable the I2s_PLL output clock
 * @param[in]	   clkEnable    : Enum value to enable or disable the clock
 *                              - Enable(1)  : Enables clock for i2s
 *	                            - Disable(0) : Disables clock for i2s
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_I2sPllClkEnable(boolean_t clkEnable)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_i2s_pll_clk_enable(clkEnable);
#else
  return clk_i2s_pll_clk_enable(clkEnable);
#endif
}

/**
 * @fn	     	  STATIC INLINE rsi_error_t RSI_CLK_I2sPllClkBypassEnable(boolean_t clkEnable)
 * @brief		    This API is used to enable bypass clock
 * @param[in]	  clkEnable  : Enum value to enable or disable the clock
 *                          - Enable  : Enables bypass clock for i2s
 *	                        - Disable : Disables bypass clock for i2s
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_I2sPllClkBypassEnable(boolean_t clkEnable)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_i2s_pll_clk_bypass_enable(clkEnable);
#else
  return clk_i2s_pll_clk_bypass_enable(clkEnable);
#endif
}

/**
 * @fn	        STATIC INLINE rsi_error_t RSI_CLK_I2sPllPdEnable(boolean_t en)
 * @brief		    This API is used to enable the PdEnable(power down)
 * @param[in]   en : Enable or disable the PdEnable
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_I2sPllPdEnable(boolean_t en)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_i2s_pll_pd_enable(en);
#else
  return clk_i2s_pll_pd_enable(en);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_I2sPllTurnOff()
 * @brief		    This API is used to TurnOff the I2s_PLL
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_I2sPllTurnOff()
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_i2s_pll_turn_off();
#else
  return clk_i2s_pll_turn_off();
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_I2sPllTurnOn()
 * @brief		    This API is used to TurnOn the I2s_PLL
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_I2sPllTurnOn()
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_i2s_pll_turn_on();
#else
  return clk_i2s_pll_turn_on();
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_SetI2sPllFreq(const M4CLK_Type *pCLK,uint32_t i2sPllFreq, uint32_t fXtal)
 * @brief		    This API is used to set the I2s_pll clock to particular frequency
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  i2sPllFreq : Frequency value in Mhz for I2S_PLL Clk .
 * @param[in]	  fXtal : Frequency value in Mhz for crystal oscillator frequency.
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_SetI2sPllFreq(const M4CLK_Type *pCLK, uint32_t i2sPllFreq, uint32_t fXtal)
{
  system_clocks.i2s_pll_clock = i2sPllFreq;
  /* TurnON the I2S_PLL */
  RSI_CLK_I2sPllTurnOn();
#if SL_WIFI_COMPONENT_INCLUDED
  /*  Notify NWP that M4 requires XTAL clock source */
  sli_si91x_xtal_turn_on_request_from_m4_to_TA();
#endif
  SPI_MEM_MAP_PLL(I2S_PLL_CTRL_REG9) = 0xD900;
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_set_i2s_pll_freq(pCLK, i2sPllFreq, fXtal);
#else
  return clk_set_i2s_pll_freq(pCLK, i2sPllFreq, fXtal);
#endif
}

/**
 * @fn          	STATIC INLINE rsi_error_t RSI_CLK_I2sPllSetFreqDiv(const M4CLK_Type *pCLK,uint16_t u16DivFactor1,
 *	                                               uint16_t u16DivFactor2,uint16_t nFactor,uint16_t mFactor,
 *                                                 uint16_t fcwF)
 * @brief		    This API is used to divide I2s_PLL Clock
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  u16DivFactor1 : Post Division factor1. See user manual for more info.
 * @param[in]	  u16DivFactor2 : Post Division factor2. See user manual for more info.
 * @param[in]	  nFactor : N factor for PLL. See user manual for more info.
 * @param[in]	  mFactor : M factor for PLL. See user manual for more info.
 * @param[in]	  fcwF : Fractional Frequency Control Word.
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_I2sPllSetFreqDiv(const M4CLK_Type *pCLK,
                                                   uint16_t u16DivFactor1,
                                                   uint16_t u16DivFactor2,
                                                   uint16_t nFactor,
                                                   uint16_t mFactor,
                                                   uint16_t fcwF)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_i2s_pll_set_freq_div(pCLK, u16DivFactor1, u16DivFactor2, nFactor, mFactor, fcwF);
#else
  return clk_i2s_pll_set_freq_div(pCLK, u16DivFactor1, u16DivFactor2, nFactor, mFactor, fcwF);
#endif
}

/**
 * @fn   	      STATIC INLINE rsi_error_t RSI_CLK_I2sPllClkSet(const M4CLK_Type *pCLK)
 * @brief		    This API is used to set the I2s_pll_clk
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_I2sPllClkSet(const M4CLK_Type *pCLK)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_i2s_pll_clk_set(pCLK);
#else
  return clk_i2s_pll_clk_set(pCLK);
#endif
}

/**
 * @fn   	      STATIC INLINE rsi_error_t  RSI_CLK_I2sPllClkReset()
 * @brief		    This API is used to reset the I2s_pll_clk
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_I2sPllClkReset()
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_i2s_pll_clk_reset();
#else
  return clk_i2s_pll_clk_reset();
#endif
}

/**
 * @fn   	       STATIC INLINE rsi_error_t RSI_CLK_IntfPllClkEnable(boolean_t clkEnable)
 * @brief		     This API is used to enable the Intf_PLL output clock
 * @param[in]	   clkEnable  :  Enum value to enable or disable the clock
 *                            - Enable(1)  : Enables clock
 *	                          - Disable(0) : Disables clock
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_IntfPllClkEnable(boolean_t clkEnable)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_intf_pll_clk_enable(clkEnable);
#else
  return clk_intf_pll_clk_enable(clkEnable);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_IntfPllPdEnable(boolean_t en)
 * @brief		    This API is used to enable the PdEnable(power down)
 * @param[in]   en : Enable or disable the PdEnable
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_IntfPllPdEnable(boolean_t en)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_intf_pll_pd_enable(en);
#else
  return clk_intf_pll_pd_enable(en);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_CLK_IntfPLLTurnOff()
 * @brief		    This API is used to TurnOff the Intf_PLL
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_IntfPLLTurnOff()
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_intf_pll_turn_off();
#else
  return clk_intf_pll_turn_off();
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_CLK_IntfPLLTurnOn()
 * @brief       This API is used to TurnOn the Intf_PLL
 * @return      returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_IntfPLLTurnOn()
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_intf_pll_turn_on();
#else
  return clk_intf_pll_turn_on();
#endif
}
/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_SetIntfPllFreq(const M4CLK_Type *pCLK,uint32_t intfPllFreq,uint32_t pllRefClk)
 * @brief		    This API is used to set the INTFPLL clock to particular frequency
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  intfPllFreq : Frequency value in Mhz for INTFPLL Clk .
 * @param[in]	  pllRefClk : Frequency value in Mhz for Reference clk. Please refer # NOTE
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * @note        Only 1Mhz steps applicable to the this API, 0.96Mhz steps are not supported
 */
STATIC INLINE rsi_error_t RSI_CLK_SetIntfPllFreq(const M4CLK_Type *pCLK, uint32_t intfPllFreq, uint32_t pllRefClk)
{
  rsi_error_t error            = (rsi_error_t)0;
  system_clocks.intf_pll_clock = intfPllFreq;
  /* TurnON the INTF_PLL */
  RSI_CLK_IntfPLLTurnOn();

#if SL_WIFI_COMPONENT_INCLUDED
  if (pllRefClk == XTAL_CLK_FREQ) //avoid if XTAL req is already done
  {
    /*  Notify NWP that M4 requires XTAL clock source */
    sli_si91x_xtal_turn_on_request_from_m4_to_TA();
    PLL_REF_CLK_CONFIG_REG &= SELECT_XTAL_MHZ_CLOCK; // Selecting the XTAL as PLL reference clock
  }
#endif
  SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG9) = 0xD900;
#if defined(CLOCK_ROMDRIVER_PRESENT)
  error = ROMAPI_M4SS_CLK_API->clk_set_intf_pll_freq(pCLK, intfPllFreq, pllRefClk);
#else
  error = clk_set_intf_pll_freq(pCLK, intfPllFreq, pllRefClk);
#endif
  if (error != RSI_OK) {
    return error;
  } else {
    intfPllFreq /= 1000000;
    /* If INTF PLL frequency is less than 90MHz, configure LDO voltage based on preprocessor setting */
    if (intfPllFreq < 90) {
#if defined(SL_SI91X_ENABLE_LDO_LOW_VOLTAGE) && (SL_SI91X_ENABLE_LDO_LOW_VOLTAGE == 1)
      /* Option 1: Enable low voltage mode (PS4 to PS3 transition)
        * This mode uses lower LDO voltage for reduced power consumption.
        * To enable this mode, define SL_SI91X_ENABLE_LDO_LOW_VOLTAGE=1 in preprocessor settings.
        */
      /* Change the power state from PS4 to PS3 (sets INTF LDO to LOW voltage) */
      RSI_PS_PowerStateChangePs4toPs3();
      /* Configure DCDC to give lower output voltage */
      RSI_PS_SetDcDcToLowerVoltage();
#else
      /* Option 2: Default mode - High voltage mode (PS3 to PS4 transition)
        * This is the default behavior when SL_SI91X_ENABLE_LDO_LOW_VOLTAGE is not defined.
        * Uses higher LDO voltage (1.15V) for better performance.
        */
      /* Configure DCDC to give higher output voltage (1.15V) */
      RSI_PS_SetDcDcToHigerVoltage();
      /* Change the power state from PS3 to PS4 (sets INTF LDO to HIGH voltage) */
      RSI_PS_PowerStateChangePs3toPs4();
#endif
    }
    if ((intfPllFreq >= 90) && (!(BATT_FF->MCU_PMU_LDO_CTRL_CLEAR & MCU_SOC_LDO_LVL))) {
      /* Change the power state from PS3 to PS4 */
      RSI_PS_SetDcDcToHigerVoltage();
      /* Configure DCDC to give higher output voltage.*/
      RSI_PS_PowerStateChangePs3toPs4();
    }
  }
  return RSI_OK;
}

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_CLK_IntfPllSetFreqDiv(const M4CLK_Type *pCLK , boolean_t clk_en,
 *	                                               uint16_t divFactor,uint16_t nFactor,uint16_t mFactor,
 *	                                               uint16_t fcwF,uint16_t dcoFixSel,uint16_t ldoProg)
 * @brief		    This API is used to divide the Intf PLL clock frequency
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  clk_en : Enble the intf_pll_clk output enale
 * @param[in]	  divFactor : PLL post division factor ,see user for more info
 * @param[in]   nFactor : N factor of PLL , see user manual for more info
 * @param[in]   mFactor : M factor of PLL , see user manual for more info
 * @param[in]   fcwF : Fractional Frequency Control Word. For below 200MHz fcwF is 0 and above 200Mhz if the frequency is odd program FCW_F as 8192
 * @param[in]   dcoFixSel : Dco Fixed Ring select. Please refer # Note
 * @param[in]   ldoProg : INTFPLL LDO output voltage select. Please refer # Note
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * @note        For <= 200Mhz ---> ldo_prog =4 and dco_fix_sel=1
 *              - For 201-250Mhz ---> ldo_prog =5 and dco_fix_sel=0
 *              - For >=251Mhz ---> ldo_prog =5 and dco_fix_sel=2
 */
STATIC INLINE rsi_error_t RSI_CLK_IntfPllSetFreqDiv(const M4CLK_Type *pCLK,
                                                    boolean_t clk_en,
                                                    uint16_t divFactor,
                                                    uint16_t nFactor,
                                                    uint16_t mFactor,
                                                    uint16_t fcwF,
                                                    uint16_t dcoFixSel,
                                                    uint16_t ldoProg)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API
    ->clk_intf_pll_set_freq_div(pCLK, clk_en, divFactor, nFactor, mFactor, fcwF, dcoFixSel, ldoProg);
#else
  return clk_intf_pll_set_freq_div(pCLK, clk_en, divFactor, nFactor, mFactor, fcwF, dcoFixSel, ldoProg);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_CLK_IntfPLLClkBypassEnable(boolean_t clkEnable)
 * @brief		    This API is used to enable bypass clock
 * @param[in]	  clkEnable : is enum value to enable or disable the clock
 *                          - Enable  : Enables bypass clock
 *	                        - Disable : Disables bypass clock
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_IntfPLLClkBypassEnable(boolean_t clkEnable)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_intf_pll_clk_bypass_enable(clkEnable);
#else
  return clk_intf_pll_clk_bypass_enable(clkEnable);
#endif
}

/**
 * @fn	        STATIC INLINE rsi_error_t  RSI_CLK_IntfPllClkReset()
 * @brief		    This API is used to Reset the Intf_pll_clk
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_IntfPllClkReset()
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_intf_pll_clk_reset();
#else
  return clk_intf_pll_clk_reset();
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_CLK_IntfPllClkSet(const M4CLK_Type *pCLK)
 * @brief		    This API is used to Enables the Intf-PLL
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_IntfPllClkSet(const M4CLK_Type *pCLK)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_intf_pll_clk_set(pCLK);
#else
  return clk_intf_pll_clk_set(pCLK);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_CLK_PeripheralClkEnable1(M4CLK_Type *pCLK ,uint32_t flags)
 * @brief		    This API is used to enable the peripheral clocks for SET1 register
 * @param[in]	  pCLK     : Pointer to the pll register instance
 * @param[in]   flags : ORed values of peripheral bits to be enbled.
 *            	          - \ref USART1_PCLK_ENABLE
 *            	          - \ref USART1_SCLK_ENABLE
 *            	          - \ref USART2_PCLK_ENABLE
 *            	          - \ref USART2_SCLK_ENABLE
 *            	          - \ref CT_CLK_ENABLE
 *            	          - \ref CT_PCLK_ENABLE
 *            	          - \ref ICACHE_CLK_ENABLE
 *            	          - \ref ICACHE_CLK_2X_ENABLE
 *            	          - \ref GPDMA_HCLK_ENABLE
 *            	          - \ref SOC_PLL_SPI_CLK_ENABLE
 *            	          - \ref IID_CLK_ENABLE
 *            	          - \ref SDIO_SYS_HCLK_ENABLE
 *            	          - \ref CRC_CLK_ENABLE_M4
 *            	          - \ref M4SS_UM_CLK_STATIC_EN
 *            	          - \ref ETH_HCLK_ENABLE
 *            	          - \ref HWRNG_PCLK_ENABLE
 *            	          - \ref GNSS_MEM_CLK_ENABLE
 *            	          - \ref CCI_PCLK_ENABLE
 *            	          - \ref CCI_HCLK_ENABLE
 *            	          - \ref CCI_CLK_ENABLE
 *            	          - \ref MASK_HOST_CLK_WAIT_FIX
 *            	          - \ref MASK31_HOST_CLK_CNT
 *            	          - \ref SD_MEM_INTF_CLK_ENABLE
 *            	          - \ref MASK_HOST_CLK_AVAILABLE_FIX
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * 
 *  @b Example
 *         - RSI_Clk_PeripheralClkEnable1(&M4CLK ,(USART1_PCLK_ENABLE | USART1_SCLK_ENABLE ));
 */
STATIC INLINE rsi_error_t RSI_CLK_PeripheralClkEnable1(M4CLK_Type *pCLK, uint32_t flags)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_peripheral_clk_enable1(pCLK, flags);
#else
  return clk_peripheral_clk_enable1(pCLK, flags);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_CLK_PeripheralClkDisable1(M4CLK_Type *pCLK ,uint32_t flags)
 * @brief		    This API is used to disable the peripheral clocks for CLR1 register
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   flags : ORed values of peripheral bits to be enbled.
 *            	          - \ref USART1_PCLK_ENABLE
 *            	          - \ref USART1_SCLK_ENABLE
 *            	          - \ref USART2_PCLK_ENABLE
 *            	          - \ref USART2_SCLK_ENABLE
 *            	          - \ref CT_CLK_ENABLE
 *            	          - \ref CT_PCLK_ENABLE
 *            	          - \ref ICACHE_CLK_ENABLE
 *            	          - \ref ICACHE_CLK_2X_ENABLE
 *            	          - \ref GPDMA_HCLK_ENABLE
 *            	          - \ref SOC_PLL_SPI_CLK_ENABLE
 *            	          - \ref IID_CLK_ENABLE
 *            	          - \ref SDIO_SYS_HCLK_ENABLE
 *            	          - \ref CRC_CLK_ENABLE_M4
 *            	          - \ref M4SS_UM_CLK_STATIC_EN
 *            	          - \ref ETH_HCLK_ENABLE
 *            	          - \ref HWRNG_PCLK_ENABLE
 *            	          - \ref GNSS_MEM_CLK_ENABLE
 *            	          - \ref CCI_PCLK_ENABLE
 *            	          - \ref CCI_HCLK_ENABLE
 *            	          - \ref CCI_CLK_ENABLE
 *            	          - \ref MASK_HOST_CLK_WAIT_FIX
 *            	          - \ref MASK31_HOST_CLK_CNT
 *            	          - \ref SD_MEM_INTF_CLK_ENABLE
 *            	          - \ref MASK_HOST_CLK_AVAILABLE_FIX
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * 
 *  @b Example
 *          - RSI_Clk_PeripheralClkDisable1(&M4CLK ,(USART1_PCLK_ENABLE | USART1_SCLK_ENABLE ));
 */
STATIC INLINE rsi_error_t RSI_CLK_PeripheralClkDisable1(M4CLK_Type *pCLK, uint32_t flags)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_peripheral_clk_disable1(pCLK, flags);
#else
  return clk_peripheral_clk_disable1(pCLK, flags);
#endif
}

/**
 * @fn      	  STATIC INLINE rsi_error_t  RSI_CLK_PeripheralClkEnable2(M4CLK_Type *pCLK ,uint32_t flags)
 * @brief		    This API is used to enable the peripheral clocks for SET2 register
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   flags : ORed values of peripheral bits to be enabled.
 *            	          - \ref GEN_SPI_MST1_HCLK_ENABLE
 *            	          - \ref CAN1_PCLK_ENABLE
 *            	          - \ref CAN1_CLK_ENABLE
 *            	          - \ref UDMA_HCLK_ENABLE
 *            	          - \ref I2C_BUS_CLK_ENABLE
 *            	          - \ref I2C_2_BUS_CLK_ENABLE
 *            	          - \ref SSI_SLV_PCLK_ENABLE
 *            	          - \ref SSI_SLV_SCLK_ENABLE
 *            	          - \ref QSPI_CLK_ENABLE
 *            	          - \ref QSPI_HCLK_ENABLE
 *            	          - \ref I2SM_INTF_SCLK_ENABLE
 *            	          - \ref I2SM_PCLK_ENABLE
 *            	          - \ref QE_PCLK_ENABLE
 *            	          - \ref MCPWM_PCLK_ENABLE
 *            	          - \ref SGPIO_PCLK_ENABLE
 *            	          - \ref EGPIO_PCLK_ENABLE
 *            	          - \ref ARM_CLK_ENABLE
 *            	          - \ref SSI_MST_PCLK_ENABLE
 *            	          - \ref SSI_MST_SCLK_ENABLE
 *            	          - \ref MEM2_CLK_ENABLE
 *            	          - \ref MEM_CLK_ULP_ENABLE
 *            	          - \ref ROM_CLK_ENABLE
 *            	          - \ref PLL_INTF_CLK_ENABLE
 *            	          - \ref SEMAPHORE_CLK_ENABLE
 *            	          - \ref TOT_CLK_ENABLE
 *            	          - \ref RMII_SOFT_RESET
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * 
 *  @b Example
 *       - RSI_Clk_PeripheralClkEnable2(M4CLK ,(GEN_SPI_MST1_HCLK_ENABLE | SSI_MST_PCLK_ENABLE));
 */
STATIC INLINE rsi_error_t RSI_CLK_PeripheralClkEnable2(M4CLK_Type *pCLK, uint32_t flags)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_peripheral_clk_enable2(pCLK, flags);
#else
  return clk_peripheral_clk_enable2(pCLK, flags);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_CLK_PeripheralClkDisable2(M4CLK_Type *pCLK ,uint32_t flags)
 * @brief		    This API is used to disable the peripheral clocks for CLR2 register
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   flags : ORed values of peripheral bits to be enabled.
 *            	          - \ref GEN_SPI_MST1_HCLK_ENABLE
 *            	          - \ref CAN1_PCLK_ENABLE
 *            	          - \ref CAN1_CLK_ENABLE
 *            	          - \ref UDMA_HCLK_ENABLE
 *            	          - \ref I2C_BUS_CLK_ENABLE
 *            	          - \ref I2C_2_BUS_CLK_ENABLE
 *            	          - \ref SSI_SLV_PCLK_ENABLE
 *            	          - \ref SSI_SLV_SCLK_ENABLE
 *            	          - \ref QSPI_CLK_ENABLE
 *            	          - \ref QSPI_HCLK_ENABLE
 *            	          - \ref I2SM_INTF_SCLK_ENABLE
 *            	          - \ref I2SM_PCLK_ENABLE
 *            	          - \ref QE_PCLK_ENABLE
 *            	          - \ref MCPWM_PCLK_ENABLE
 *            	          - \ref SGPIO_PCLK_ENABLE
 *            	          - \ref EGPIO_PCLK_ENABLE
 *            	          - \ref ARM_CLK_ENABLE
 *            	          - \ref SSI_MST_PCLK_ENABLE
 *            	          - \ref SSI_MST_SCLK_ENABLE
 *            	          - \ref MEM2_CLK_ENABLE
 *            	          - \ref MEM_CLK_ULP_ENABLE
 *            	          - \ref ROM_CLK_ENABLE
 *            	          - \ref PLL_INTF_CLK_ENABLE
 *            	          - \ref SEMAPHORE_CLK_ENABLE
 *            	          - \ref TOT_CLK_ENABLE
 *            	          - \ref RMII_SOFT_RESET
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * 
 *  @b Example
 *        - RSI_Clk_PeripheralClkDisable2(M4CLK ,(GEN_SPI_MST1_HCLK_ENABLE | SSI_MST_PCLK_ENABLE));
 */
STATIC INLINE rsi_error_t RSI_CLK_PeripheralClkDisable2(M4CLK_Type *pCLK, uint32_t flags)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_peripheral_clk_disable2(pCLK, flags);
#else
  return clk_peripheral_clk_disable2(pCLK, flags);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_CLK_PeripheralClkEnable3(M4CLK_Type *pCLK ,uint32_t flags)
 * @brief		    This API is used to enable the peripheral clocks for SET3 register
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   flags : ORed values of peripheral bits to be enabled.
 *              	        - \ref BUS_CLK_ENABLE
 *              	        - \ref M4_CORE_CLK_ENABLE
 *              	        - \ref CM_BUS_CLK_ENABLE
 *              	        - \ref MISC_CONFIG_PCLK_ENABLE
 *              	        - \ref EFUSE_CLK_ENABLE
 *              	        - \ref ICM_CLK_ENABLE
 *              	        - \ref MEM1_CLK_ENABLE
 *              	        - \ref MEM3_CLK_ENABLE
 *              	        - \ref USB_PHY_CLK_IN_ENABLE
 *              	        - \ref QSPI_CLK_ONEHOT_ENABLE
 *              	        - \ref QSPI_M4_SOC_SYNC
 *              	        - \ref EGPIO_CLK_ENABLE
 *              	        - \ref I2C_CLK_ENABLE
 *              	        - \ref I2C_2_CLK_ENABLE
 *              	        - \ref EFUSE_PCLK_ENABLE
 *              	        - \ref SGPIO_PCLK_ENABLE
 *              	        - \ref TASS_M4SS_64K_SWITCH_CLK_ENABLE
 *              	        - \ref TASS_M4SS_128K_SWITCH_CLK_ENABLE
 *              	        - \ref TASS_M4SS_SDIO_SWITCH_CLK_ENABLE
 *              	        - \ref TASS_M4SS_USB_SWITCH_CLK_ENABLE
 *              	        - \ref ROM_MISC_STATIC_ENABLE
 *              	        - \ref M4_SOC_CLK_FOR_OTHER_ENABLE
 *              	        - \ref ICACHE_ENABLE
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * 
 *  @b Example
 *        - RSI_Clk_PeripheralClkEnable3(M4CLK ,(M4_SOC_CLK_FOR_OTHER_ENABLE | ROM_MISC_STATIC_ENABLE));
 */
STATIC INLINE rsi_error_t RSI_CLK_PeripheralClkEnable3(M4CLK_Type *pCLK, uint32_t flags)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_peripheral_clk_enable3(pCLK, flags);
#else
  return clk_peripheral_clk_enable3(pCLK, flags);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_CLK_PeripheralClkDisable3(M4CLK_Type *pCLK ,uint32_t flags)
 * @brief		    This API is used to disable the peripheral clocks for CLR3 register
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   flags : ORed values of peripheral bits to be enbled.
 *              	        - \ref BUS_CLK_ENABLE
 *              	        - \ref M4_CORE_CLK_ENABLE
 *              	        - \ref CM_BUS_CLK_ENABLE
 *              	        - \ref MISC_CONFIG_PCLK_ENABLE
 *              	        - \ref EFUSE_CLK_ENABLE
 *              	        - \ref ICM_CLK_ENABLE
 *              	        - \ref MEM1_CLK_ENABLE
 *              	        - \ref MEM3_CLK_ENABLE
 *              	        - \ref USB_PHY_CLK_IN_ENABLE
 *              	        - \ref QSPI_CLK_ONEHOT_ENABLE
 *              	        - \ref QSPI_M4_SOC_SYNC
 *              	        - \ref EGPIO_CLK_ENABLE
 *              	        - \ref I2C_CLK_ENABLE
 *              	        - \ref I2C_2_CLK_ENABLE
 *              	        - \ref EFUSE_PCLK_ENABLE
 *              	        - \ref SGPIO_PCLK_ENABLE
 *              	        - \ref TASS_M4SS_64K_SWITCH_CLK_ENABLE
 *              	        - \ref TASS_M4SS_128K_SWITCH_CLK_ENABLE
 *              	        - \ref TASS_M4SS_SDIO_SWITCH_CLK_ENABLE
 *              	        - \ref TASS_M4SS_USB_SWITCH_CLK_ENABLE
 *              	        - \ref ROM_MISC_STATIC_ENABLE
 *              	        - \ref M4_SOC_CLK_FOR_OTHER_ENABLE
 *              	        - \ref ICACHE_ENABLE
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * 
 *  @b Example
 *       - RSI_Clk_PeripheralClkDisable3(M4CLK ,(M4_SOC_CLK_FOR_OTHER_ENABLE | ROM_MISC_STATIC_ENABLE));
 */
STATIC INLINE rsi_error_t RSI_CLK_PeripheralClkDisable3(M4CLK_Type *pCLK, uint32_t flags)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_peripheral_clk_disable3(pCLK, flags);
#else
  return clk_peripheral_clk_disable3(pCLK, flags);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_CLK_DynamicClkGateDisable(M4CLK_Type *pCLK ,uint32_t  flags)
 * @brief		    This API is used to disable the dynamic clock gate for peripherals
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  flags : ORed value of the register bits
 *                        - \ref SDIO_SYS_HCLK_DYN_CTRL_DISABLE
 *                        - \ref BUS_CLK_DYN_CTRL_DISABLE
 *                        - \ref GPDMA_HCLK_DYN_CTRL_DISABLE
 *                        - \ref EGPIO_PCLK_DYN_CTRL_DISABLE
 *                        - \ref SGPIO_PCLK_DYN_CTRL_DISABLE
 *                        - \ref TOT_CLK_DYN_CTRL_DISABLE
 *                        - \ref HWRNG_PCLK_DYN_CTRL_DISABLE
 *                        - \ref USART1_SCLK_DYN_CTRL_DISABLE
 *                        - \ref USART1_PCLK_DYN_CTRL_DISABLE
 *                        - \ref USART2_SCLK_DYN_CTRL_DISABLE
 *                        - \ref USART2_PCLK_DYN_CTRL_DISABLE
 *                        - \ref SSI_SLV_SCLK_DYN_CTRL_DISABLE
 *                        - \ref SSI_SLV_PCLK_DYN_CTRL_DISABLE
 *                        - \ref I2SM_INTF_SCLK_DYN_CTRL_DISABLE
 *                        - \ref SEMAPHORE_CLK_DYN_CTRL_DISABLE
 *                        - \ref ARM_CLK_DYN_CTRL_DISABLE
 *                        - \ref SSI_MST_SCLK_DYN_CTRL_DISABLE
 *                        - \ref MEM1_CLK_DYN_CTRL_DISABLE
 *                        - \ref MEM2_CLK_DYN_CTRL_DISABLE
 *                        - \ref MEM_CLK_ULP_DYN_CTRL_DISABLE
 *                        - \ref MEM_CLK_ULP_DYN_CTRL_DISABLE
 *                        - \ref SSI_MST_PCLK_DYN_CTRL_DISABLE
 *                        - \ref ICACHE_DYN_GATING_DISABLE
 *                        - \ref CCI_PCLK_DYN_CTRL_DISABLE
 *                        - \ref MISC_CONFIG_PCLK_DYN_CTRL_DISABLE
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * 
 *  @b Example
 *      - RSI_Clk_DynamicClkGateDisable(M4CLK , (SDIO_SYS_HCLK_DYN_CTRL_DISABLE | BUS_CLK_DYN_CTRL_DISABLE));
 */
STATIC INLINE rsi_error_t RSI_CLK_DynamicClkGateDisable(M4CLK_Type *pCLK, uint32_t flags)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_dynamic_clk_gate_disable(pCLK, flags);
#else
  return clk_dynamic_clk_gate_disable(pCLK, flags);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_CLK_DynamicClkGateDisable2(M4CLK_Type *pCLK ,uint32_t  flags)
 * @brief		    This API is used to disable the dynamic clock gate for peripherals
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  flags : ORed value of the register bits
 *                     - \ref SOC_PLL_SPI_CLK_DYN_CTRL_DISABLE
 *                     - \ref I2C_BUS_DYN_CTRL_DISABLE
 *                     - \ref I2C_2_BUS_CLK_DYN_CTRL_DISABLE
 *                     - \ref CT_PCLK_DYN_CTRL_DISABLE
 *                     - \ref CAN1_PCLK_DYN_CTRL_DISABLE
 *                     - \ref EFUSE_CLK_DYN_CTRL_DISABLE
 *                     - \ref EFUSE_PCLK_DYN_CTRL_DISABLE
 *                     - \ref PWR_CTRL_CLK_DYN_CTRL_DISABLE
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * 
 *  @b Example
 *      - RSI_CLK_DynamicClkGateDisable2(M4CLK , (EFUSE_CLK_DYN_CTRL_DISABLE | EFUSE_PCLK_DYN_CTRL_DISABLE));
 */
STATIC INLINE rsi_error_t RSI_CLK_DynamicClkGateDisable2(M4CLK_Type *pCLK, uint32_t flags)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_dynamic_clk_gate_disable2(pCLK, flags);
#else
  return clk_dynamic_clk_gate_disable2(pCLK, flags);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_CLK_DynamicClkGateEnable(M4CLK_Type *pCLK ,uint32_t  flags)
 * @brief		    This API is used to enable the dynamic clock gate for peripherals
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  flags : ORed value of the register bits
 *                        - \ref SDIO_SYS_HCLK_DYN_CTRL_DISABLE
 *                        - \ref BUS_CLK_DYN_CTRL_DISABLE
 *                        - \ref GPDMA_HCLK_DYN_CTRL_DISABLE
 *                        - \ref EGPIO_PCLK_DYN_CTRL_DISABLE
 *                        - \ref SGPIO_PCLK_DYN_CTRL_DISABLE
 *                        - \ref TOT_CLK_DYN_CTRL_DISABLE
 *                        - \ref HWRNG_PCLK_DYN_CTRL_DISABLE
 *                        - \ref USART1_SCLK_DYN_CTRL_DISABLE
 *                        - \ref USART1_PCLK_DYN_CTRL_DISABLE
 *                        - \ref USART2_SCLK_DYN_CTRL_DISABLE
 *                        - \ref USART2_PCLK_DYN_CTRL_DISABLE
 *                        - \ref SSI_SLV_SCLK_DYN_CTRL_DISABLE
 *                        - \ref SSI_SLV_PCLK_DYN_CTRL_DISABLE
 *                        - \ref I2SM_INTF_SCLK_DYN_CTRL_DISABLE
 *                        - \ref SEMAPHORE_CLK_DYN_CTRL_DISABLE
 *                        - \ref ARM_CLK_DYN_CTRL_DISABLE
 *                        - \ref SSI_MST_SCLK_DYN_CTRL_DISABLE
 *                        - \ref MEM1_CLK_DYN_CTRL_DISABLE
 *                        - \ref MEM2_CLK_DYN_CTRL_DISABLE
 *                        - \ref MEM_CLK_ULP_DYN_CTRL_DISABLE
 *                        - \ref MEM_CLK_ULP_DYN_CTRL_DISABLE
 *                        - \ref SSI_MST_PCLK_DYN_CTRL_DISABLE
 *                        - \ref ICACHE_DYN_GATING_DISABLE
 *                        - \ref CCI_PCLK_DYN_CTRL_DISABLE
 *                        - \ref MISC_CONFIG_PCLK_DYN_CTRL_DISABLE
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * 
 *  @b Example
 *        - RSI_Clk_DynamicClkGateEnable(M4CLK , (SDIO_SYS_HCLK_DYN_CTRL_DISABLE | BUS_CLK_DYN_CTRL_DISABLE));
 */
STATIC INLINE rsi_error_t RSI_CLK_DynamicClkGateEnable(M4CLK_Type *pCLK, uint32_t flags)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_dynamic_clk_gate_enable(pCLK, flags);
#else
  return clk_dynamic_clk_gate_enable(pCLK, flags);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_CLK_DynamicClkGateEnable2(M4CLK_Type *pCLK ,uint32_t  flags)
 * @brief		    This API is used to enable the dynamic clock gate for peripherals
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  flags : ORed value of the register bits
 *                     - \ref SOC_PLL_SPI_CLK_DYN_CTRL_DISABLE
 *                     - \ref I2C_BUS_DYN_CTRL_DISABLE
 *                     - \ref I2C_2_BUS_CLK_DYN_CTRL_DISABLE
 *                     - \ref CT_PCLK_DYN_CTRL_DISABLE
 *                     - \ref CAN1_PCLK_DYN_CTRL_DISABLE
 *                     - \ref EFUSE_CLK_DYN_CTRL_DISABLE
 *                     - \ref EFUSE_PCLK_DYN_CTRL_DISABLE
 *                     - \ref PWR_CTRL_CLK_DYN_CTRL_DISABLE
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * 
 *  @b Example
 *        - RSI_Clk_DynamicClkGateEnable2(M4CLK , (EFUSE_CLK_DYN_CTRL_DISABLE | EFUSE_PCLK_DYN_CTRL_DISABLE));
 */
STATIC INLINE rsi_error_t RSI_CLK_DynamicClkGateEnable2(M4CLK_Type *pCLK, uint32_t flags)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_dynamic_clk_gate_enable2(pCLK, flags);
#else
  return clk_dynamic_clk_gate_enable2(pCLK, flags);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_ULPSS_EnableRefClks(REF_CLK_ENABLE_T enable,  SRC_TYPE_T srcType,cdDelay   delayFn)
 * @brief		    This API is used to enable the ULP reference clocks and provide delay for clock starting
 * @param[in]	  enable : To enable the particular reference clock. See \ref REF_CLK_ENABLE_T for more info
 * @param[in]	  srcType : To select the pheripheral clock or processor clk. See \ref SRC_TYPE_T for more info
 * @param[in]	  delayFn : Call back fuction used to create delay by using loops or timers in application code
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_ULPSS_EnableRefClks(REF_CLK_ENABLE_T enable, SRC_TYPE_T srcType, cdDelay delayFn)
{
#if defined(A11_ROM) && defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->ulpss_enable_ref_clks(enable, srcType, delayFn);
#else
  return ulpss_enable_ref_clks(enable, srcType, delayFn);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_ULPSS_DisableRefClks(REF_CLK_ENABLE_T clk_type)
 * @brief		    This API is used to disable the ULP reference clocks
 * @param[in]	  clk_type : To enable the particular reference clock. See \ref REF_CLK_ENABLE_T for more info
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_ULPSS_DisableRefClks(REF_CLK_ENABLE_T clk_type)
{
#if defined(A11_ROM) && defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->ulpss_disable_ref_clks(clk_type);
#else
  return ulpss_disable_ref_clks(clk_type);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_CLK_M4ssRefClkConfig(const M4CLK_Type *pCLK ,M4SS_REF_CLK_SEL_T clkSource)
 * @brief		    This API is used to configure the m4ss_ref clocks
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   clkSource : Enum values of different M4 ref source clocks \ref M4SS_REF_CLK_SEL_T
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_M4ssRefClkConfig(const M4CLK_Type *pCLK, M4SS_REF_CLK_SEL_T clkSource)
{
#if SL_WIFI_COMPONENT_INCLUDED
  if (clkSource == EXT_40MHZ_CLK) {
    /*  Notify NWP that M4 requires XTAL clock source */
    sli_si91x_xtal_turn_on_request_from_m4_to_TA();
  }
#endif
  return clk_m4ss_ref_clk_config(pCLK, clkSource);
}

/**
 * @fn	        STATIC INLINE rsi_error_t RSI_CLK_M4SocClkConfig(M4CLK_Type *pCLK ,M4_SOC_CLK_SRC_SEL_T clkSource ,
			                                         uint32_t divFactor)
 * @brief		    This API is used to configure the m4_soc clocks
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   clkSource : Enum values of different SOC source clocks.See #M4_SOC_CLK_SRC_SEL_T and NOTE for more info
 * @param[in]   divFactor : division value for M4SOC clock
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * @note        For using UlpRefClk clksource need to configure M4ssRefClk frequency. For that need to call \ref RSI_CLK_M4ssRefClkConfig Api first
 *              -  For using SocPllCLK clksource need to configure SocPll frequency. For that need to call \ref RSI_CLK_SetSocPllFreq Api first
 *							-  For using IntfPllCLK clksource need to configure IntfPll frequency. For that need to call \ref RSI_CLK_SetIntfPllFreq Api first
 *							-  For using Sleep clksource need to configure Sleep Clock. For that need to call \ref RSI_CLK_SlpClkConfig Api first
 */
STATIC INLINE rsi_error_t RSI_CLK_M4SocClkConfig(M4CLK_Type *pCLK, M4_SOC_CLK_SRC_SEL_T clkSource, uint32_t divFactor)
{
  return clk_m4_soc_clk_config(pCLK, clkSource, divFactor);
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_QspiClkConfig(M4CLK_Type *pCLK ,QSPI_CLK_SRC_SEL_T clkSource,boolean_t swalloEn,
			                           boolean_t OddDivEn,uint32_t divFactor)
 * @brief		    This API is used to configure the Qspi clocks
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   clkSource : Enum valuse for Qspi clock sources to be selected.
                           	see possible Qspi clock sources at \ref QSPI_CLK_SRC_SEL_T
 * @param[in]   swalloEn : To enable or disable the swallo functionality. See user manual for more info
 *                           - 1 : swalloEn enabled
 *                           - 0 : swalloEn disabled
 * @param[in]   OddDivEn : To enable or disable the odd div functionality. See user manual for more info
 *                           - 1 : OddDivEn enabled
 *                           - 0 : OddDivEn disabled
 * @param[in]   divFactor : Division value for Qspi Clock
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * @note        For using UlpRefClk clksource need to configure M4ssRefClk frequency.
                \n For that need to call \ref RSI_CLK_M4ssRefClkConfig Api first
 */
STATIC INLINE rsi_error_t RSI_CLK_QspiClkConfig(M4CLK_Type *pCLK,
                                                QSPI_CLK_SRC_SEL_T clkSource,
                                                boolean_t swalloEn,
                                                boolean_t OddDivEn,
                                                uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_qspi_clk_config(pCLK, clkSource, swalloEn, OddDivEn, divFactor);
#else
  return clk_qspi_clk_config(pCLK, clkSource, swalloEn, OddDivEn, divFactor);
#endif
}
#if defined(SLI_SI917B0)

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_Qspi2ClkConfig(M4CLK_Type *pCLK ,QSPI_CLK_SRC_SEL_T clkSource,boolean_t swalloEn,
			                           boolean_t OddDivEn,uint32_t divFactor)
 * @brief		    This API is used to configure the Qspi clocks
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   clkSource : Enum valuse for Qspi clock sources to be selected.
                           	see possible Qspi clock sources at \ref QSPI_CLK_SRC_SEL_T
 * @param[in]   swalloEn : To enable or disable the swallo functionality. See user manual for more info
 *                           - 1 : swalloEn enabled
 *                           - 0 : swalloEn disabled
 * @param[in]   OddDivEn : To enable or disable the odd div functionality. See user manual for more info
 *                           - 1 : OddDivEn enabled
 *                           - 0 : OddDivEn disabled
 * @param[in]   divFactor : Division value for Qspi Clock
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * @note        For using UlpRefClk clksource need to configure M4ssRefClk frequency.
                \n For that need to call \ref RSI_CLK_M4ssRefClkConfig Api first
 */
STATIC INLINE rsi_error_t RSI_CLK_Qspi2ClkConfig(M4CLK_Type *pCLK,
                                                 QSPI_CLK_SRC_SEL_T clkSource,
                                                 boolean_t swalloEn,
                                                 boolean_t OddDivEn,
                                                 uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_qspi_2_clk_config(pCLK, clkSource, swalloEn, OddDivEn, divFactor);
#else
  return clk_qspi_2_clk_config(pCLK, clkSource, swalloEn, OddDivEn, divFactor);
#endif
}
#endif
/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_UsartClkConfig(M4CLK_Type *pCLK ,CLK_ENABLE_T clkType,boolean_t FracDivEn,
			                                          EN_USART_T enUsart,USART_CLK_SRC_SEL_T clkSource,uint32_t divFactor)
 * @brief		    This API is used to configure the Usart clocks
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  clkType : Boolean value to enable or disable  clock mode
 *                        - 1  Enable : Enables the Usart clock
 *	                      - 0  Disable: Disables the Usart clock
 * @param[in]   FracDivEn : To enable or disable Fractional Division functionality
 *                        - 1 : FracDivEn enabled
 *                        - 0 : FracDivEn disabled
 * @param[in]   enUsart   : Enum values for different Usart instances see possible bypass clock sources at \ref EN_USART_T
 * @param[in]   clkSource : Enum values for Usart clock sources to be selected. see possible Usart clock sources at #USART_CLK_SRC_SEL_T
 * @param[in]   divFactor is the division value for Usart Clock
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 * @note        For using UlpRefClk clksource need to configure M4ssRefClk frequency. For that need to call #ROM_CLK_M4ssRefClkConfig Api first
 */
STATIC INLINE rsi_error_t RSI_CLK_UsartClkConfig(M4CLK_Type *pCLK,
                                                 CLK_ENABLE_T clkType,
                                                 boolean_t FracDivEn,
                                                 EN_USART_T enUsart,
                                                 USART_CLK_SRC_SEL_T clkSource,
                                                 uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_usart_clk_config(pCLK, clkType, FracDivEn, enUsart, clkSource, divFactor);
#else
  return clk_usart_clk_config(pCLK, clkType, FracDivEn, enUsart, clkSource, divFactor);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_SsiMstClkConfig(M4CLK_Type *pCLK ,CLK_ENABLE_T clkType  ,
	                                                SSI_MST_CLK_SRC_SEL_T clkSource ,uint32_t   divFactor)
 * @brief		    This API is used to configure the SSI clocks
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  clkType : Enum value to select static clock or dynamic clock. See #CLK_ENABLE_T for more info.
 * @param[in]   clkSource : Enum values for SSI clock sources to be selected. see possible SSI clock sources at #SSI_MST_CLK_SRC_SEL_T
 * @param[in]   divFactor :  is the division value for SSI Clock
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_SsiMstClkConfig(M4CLK_Type *pCLK,
                                                  CLK_ENABLE_T clkType,
                                                  SSI_MST_CLK_SRC_SEL_T clkSource,
                                                  uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_ssi_mst_clk_config(pCLK, clkType, clkSource, divFactor);
#else
  return clk_ssi_mst_clk_config(pCLK, clkType, clkSource, divFactor);
#endif
}

#ifdef CHIP_9118
/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_SdMemClkConfig(M4CLK_Type *pCLK ,boolean_t swalloEn ,SDMEM_CLK_SRC_SEL_T clkSource ,
	                                             uint32_t divFactor)
 * @brief		    This API is used to configure the SdMem clocks
 * @param[in]	  pCLK : Pointer to pll register instance
 * @param[in]   swalloEn : Enable or disable the swallo functionality
 *                         - 1 : swalloEn enabled
 *                         - 0 : swalloEn disabled
 * @param[in]   clkSource : Enum values for SdMem clock sources to be selected. see possible SdMem clock sources at #SDMEM_CLK_SRC_SEL_T
 * @param[in]   divFactor : Division value for SdMem Clock
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_SdMemClkConfig(M4CLK_Type *pCLK,
                                                 boolean_t swalloEn,
                                                 SDMEM_CLK_SRC_SEL_T clkSource,
                                                 uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_sd_mem_clk_config(pCLK, swalloEn, clkSource, divFactor);
#else
  return clk_sd_mem_clk_config(pCLK, swalloEn, clkSource, divFactor);
#endif
}
#endif

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_CtClkConfig(M4CLK_Type *pCLK ,CT_CLK_SRC_SEL_T clkSource ,uint32_t divFactor,
	                                           CLK_ENABLE_T clkType)
 * @brief		    This API is used to configure the CT clocks
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   clkSource : Enum values for CT clock sources to be selected. see possible CT clock
                           	sources at \ref CT_CLK_SRC_SEL_T
 * @param[in]   divFactor : Division value for CT Clock
 * @param[in]	  clkType : Enum value to select static clock or dynamic clock. See \ref CLK_ENABLE_T for more info.
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_CtClkConfig(M4CLK_Type *pCLK,
                                              CT_CLK_SRC_SEL_T clkSource,
                                              uint32_t divFactor,
                                              CLK_ENABLE_T clkType)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_ct_clk_config(pCLK, clkSource, divFactor, clkType);
#else
  return clk_ct_clk_config(pCLK, clkSource, divFactor, clkType);
#endif
}

#ifdef CHIP_9118
/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_CciClkConfig(M4CLK_Type *pCLK ,CCI_CLK_SRC_SEL_T clkSource ,uint32_t divFactor,
	                                    CLK_ENABLE_T clkType)
 * @brief		    This API is used to configure the CCI clocks
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   clkSource : Enum values for CCI clock sources to be selected. see possible CCI clock sources at #CCI_CLK_SRC_SEL_T
 * @param[in]   divFactor : Division value for CCI Clock
 * @param[in]	  clkType : Enum value to select static clock or dynamic clock. See #CLK_ENABLE_T for more info.
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_CciClkConfig(M4CLK_Type *pCLK,
                                               CCI_CLK_SRC_SEL_T clkSource,
                                               uint32_t divFactor,
                                               CLK_ENABLE_T clkType)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_cci_clk_config(pCLK, clkSource, divFactor, clkType);
#else
  return clk_cci_clk_config(pCLK, clkSource, divFactor, clkType);
#endif
}
#endif

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_I2sClkConfig(M4CLK_Type *pCLK ,I2S_CLK_SRC_SEL_T clkSource ,
			                                       uint32_t divFactor)
 * @brief		    This API is used to configure the I2S clocks
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   clkSource : Enum valuse for I2S clock sources to be selected. see possible I2S clock sources at #I2S_CLK_SRC_SEL_T
 * @param[in]   divFactor : Division value for I2S Clock
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_I2sClkConfig(M4CLK_Type *pCLK, I2S_CLK_SRC_SEL_T clkSource, uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_i2s_clk_config(pCLK, clkSource, divFactor);
#else
  return clk_i2s_clk_config(pCLK, clkSource, divFactor);
#endif
}

/**
 * @fn    	  	STATIC INLINE rsi_error_t RSI_CLK_McuClkOutConfig(M4CLK_Type *pCLK ,MCU_CLKOUT_SRC_SEL_T  clkSource  ,
			                                           uint32_t  divFactor)
 * @brief		    This API is used to configure the McuClkOut clocks
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   clkSource : Enum values of MCU_CLKOUT. See possible mcu_clk sources at #MCU_CLKOUT_SRC_SEL_T
 * @param[in]   divFactor : Division value for McuClkOut Clock
 * @return 		  returns zero \ref RSI_OK on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_McuClkOutConfig(M4CLK_Type *pCLK, MCU_CLKOUT_SRC_SEL_T clkSource, uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_mcu_clk_cut_config(pCLK, clkSource, divFactor);
#else
  return clk_mcu_clk_cut_config(pCLK, clkSource, divFactor);
#endif
}

#ifdef CHIP_9118
/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_CanClkConfig(M4CLK_Type *pCLK ,	uint32_t divFactor,CLK_ENABLE_T clkType)
 * @brief		    This API is used to configure the Can clocks
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   divFactor : Division value for Can Clock
 * @param[in]	  clkType : Enum value to select static clock or dynamic clock. See #CLK_ENABLE_T for more info.
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_CanClkConfig(M4CLK_Type *pCLK, uint32_t divFactor, CLK_ENABLE_T clkType)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_can_clk_config(pCLK, divFactor, clkType);
#else
  return clk_can_clk_config(pCLK, divFactor, clkType);
#endif
}

/**
 * @fn 	        STATIC INLINE rsi_error_t RSI_CLK_EthernetClkConfig(M4CLK_Type *pCLK  ,boolean_t swalloEn ,ETHERNET_CLK_SRC_SEL_T clkSource,
	                                  uint32_t divFactor)
 * @brief		    This API is used to configure the PLL_INTF clocks
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   swalloEn : Enable or disable the swallo functionality
 *                         - 1 : Swallo enabled
 *                         - 0 : Swallo disabled
 * @param[in]	  clkSource : Enum  values for PLL_Intf clock sources to be selected. see possible PLl clock sources at #ETHERNET_CLK_SRC_SEL_T
 * @param[in]	  divFactor : PLL_INTF clock division value
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_EthernetClkConfig(M4CLK_Type *pCLK,
                                                    boolean_t swalloEn,
                                                    ETHERNET_CLK_SRC_SEL_T clkSource,
                                                    uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_ethernet_clk_config(pCLK, swalloEn, clkSource, divFactor);
#else
  return clk_ethernet_clk_config(pCLK, swalloEn, clkSource, divFactor);
#endif
}
#endif

/**
 * @fn      	  STATIC INLINE rsi_error_t RSI_CLK_M4SocClkDiv(M4CLK_Type *pCLK ,uint32_t divFactor)
 * @brief		    This API is used to divide the M4soc  clock
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  divFactor : M4Soc clock division value
 * @return 		  returns 0 on success
 *			        \n Error code on failure
 */
STATIC INLINE rsi_error_t RSI_CLK_M4SocClkDiv(M4CLK_Type *pCLK, uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_m4_soc_clk_div(pCLK, divFactor);
#else
  return clk_m4_soc_clk_div(pCLK, divFactor);
#endif
}

/**
 * @fn          RSI_CLK_QspiClkDiv(M4CLK_Type *pCLK , boolean_t u8SwallowEn ,
                                   boolean_t u8OddDivEn , uint32_t divFactor )
 * @brief		    This API is used to divide the QSPI clock
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]   u8SwallowEn : To enable or disable the swallo functionality
 *                            - 1 : Swallo enabled
 *                            - 0 : Swallo disabled
 * @param[in]   u8OddDivEn : To enable or disable the odd division functionality
 *                            - 1 : Odd division enabled
 *                            - 0 : Odd division disabled
 * @param[in]	  divFactor : QSPI clock division value
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_QspiClkDiv(M4CLK_Type *pCLK,
                                             boolean_t u8SwallowEn,
                                             boolean_t u8OddDivEn,
                                             uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_qspi_clk_div(pCLK, u8SwallowEn, u8OddDivEn, divFactor);
#else
  return clk_qspi_clk_div(pCLK, u8SwallowEn, u8OddDivEn, divFactor);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_CtClkDiv(M4CLK_Type *pCLK , uint32_t divFactor)
 * @brief		    This API is used to divide the CT clock
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  divFactor : CT clock division value
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_CtClkDiv(M4CLK_Type *pCLK, uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_ct_clk_div(pCLK, divFactor);
#else
  return clk_ct_clk_div(pCLK, divFactor);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_SsiMstClkDiv(M4CLK_Type *pCLK , uint32_t divFactor )
 * @brief		    This API is used to divide the SSI clock
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  divFactor : SSI clock division value
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_SsiMstClkDiv(M4CLK_Type *pCLK, uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_ssi_mst_clk_div(pCLK, divFactor);
#else
  return clk_ssi_mst_clk_div(pCLK, divFactor);
#endif
}

#ifdef CHIP_9118
/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_CciClkDiv(M4CLK_Type *pCLK ,  uint32_t divFactor )
 * @brief		    This API is used to divide the CCI clock
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  divFactor : CCI clock division value
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_CciClkDiv(M4CLK_Type *pCLK, uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_cci_clk_div(pCLK, divFactor);
#else
  return clk_cci_clk_div(pCLK, divFactor);
#endif
}
#endif

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_I2sClkDiv(M4CLK_Type *pCLK ,  uint32_t divFactor )
 * @brief		    This API is used to divide the I2S clock
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  divFactor : I2S clock division value
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_I2sClkDiv(M4CLK_Type *pCLK, uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_i2s_clk_div(pCLK, divFactor);
#else
  return clk_i2s_clk_div(pCLK, divFactor);
#endif
}

#ifdef CHIP_9118
/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_SdmemClkDiv(M4CLK_Type *pCLK , boolean_t u8SwallowEn , uint32_t divFactor)
 * @brief		    This API is used to divide the SDMEM clock
 * @param[in]	  pCLK      : Pointer to the pll register instance
 * @param[in]   u8SwallowEn : To enable or disable the swallo functionality
 *                            - 1 : Swallo enabled
 *                            - 0 : Swallo disabled
 * @param[in]	  divFactor : SDMEM clock division value
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_SdmemClkDiv(M4CLK_Type *pCLK, boolean_t u8SwallowEn, uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_sd_mem_clk_div(pCLK, u8SwallowEn, divFactor);
#else
  return clk_sd_mem_clk_div(pCLK, u8SwallowEn, divFactor);
#endif
}
#endif

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_UsartClkDiv(M4CLK_Type *pCLK , EN_USART_T EN_USART_T ,
	                                          uint8_t u8FracDivEn, uint32_t divFactor)
 * @brief		    This API is used to divide the USART/UART clock
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  EN_USART_T : Enum  of uart numbers. See #EN_USART_T for more info
 * @param[in]	  u8FracDivEn : To enable or disable fractional division feature
 *                            - 1 : Clock Swallo type divider is selected
 *                            - 0 : Fractional Divider typed is selected
 * @param[in]	  divFactor : USART/UART clock division value
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_UsartClkDiv(M4CLK_Type *pCLK,
                                              EN_USART_T EN_USART,
                                              uint8_t u8FracDivEn,
                                              uint32_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_usart_clk_div(pCLK, EN_USART, u8FracDivEn, divFactor);
#else
  return clk_usart_clk_div(pCLK, EN_USART, u8FracDivEn, divFactor);
#endif
}

/**
 * @fn          uint32_t RSI_CLK_SlpClkCalibConfig(M4CLK_Type *pCLK ,  uint8_t clkCycles)
 * @brief		    This API is used to calibrate the sleep clock
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  clkCycles : These bits are used to program the number of clock cycles over which
	                            clock calibration is to be done.
 *                            - 00 => 1 Cycle
 *                            - 01 => 2 Cycles
 *                            - 10 => 3 Cycles
 *                            - 11 => 4 Cycles
 * @return 		  Returns the calibration duration.
 */
STATIC INLINE uint32_t RSI_CLK_SlpClkCalibConfig(M4CLK_Type *pCLK, uint8_t clkCycles)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_slp_clk_calib_config(pCLK, clkCycles);
#else
  return clk_slp_clk_calib_config(pCLK, clkCycles);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_GspiClkConfig(M4CLK_Type *pCLK ,GSPI_CLK_SRC_SEL_T clkSel )
 * @brief		    This API is used to configure the GSPI Clocks
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  clkSel : Enum values to select the clock sources. See possible values at \ref GSPI_CLK_SRC_SEL_T
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_GspiClkConfig(M4CLK_Type *pCLK, GSPI_CLK_SRC_SEL_T clkSel)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_gspi_clk_config(pCLK, clkSel);
#else
  return clk_gspi_clk_config(pCLK, clkSel);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_SlpClkConfig(M4CLK_Type *pCLK ,  SLEEP_CLK_SRC_SEL_T clkSrc)
 * @brief		    This API is used to configure the SLEEP Clocks
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  clkSrc : Enum values to select the clock sources for sleep clock. See \ref SLEEP_CLK_SRC_SEL_T for more info
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_SlpClkConfig(M4CLK_Type *pCLK, SLEEP_CLK_SRC_SEL_T clkSrc)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_slp_clk_config(pCLK, clkSrc);
#else
  return clk_slp_clk_config(pCLK, clkSrc);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_I2CClkConfig(M4CLK_Type *pCLK , boolean_t clkEnable,EN_I2C_T enI2C)
 * @brief		    This API is used to configure the I2C clock
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  clkEnable : Boolean value to enable or disable clock mode
 *                          - 1 clkEnable : Enables the I2C clock
 *	                        - 0 Disable   : Disables the I2C clock
 * @param[in]	  enI2C : Enum values. See \ref EN_I2C_T for more infomation
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_I2CClkConfig(M4CLK_Type *pCLK, boolean_t clkEnable, EN_I2C_T enI2C)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_i2c_clk_config(pCLK, clkEnable, enI2C);
#else
  return clk_i2c_clk_config(pCLK, clkEnable, enI2C);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_XtalClkConfig(uint8_t xtalPin)
 * @brief		    This API is used to configure the Xtal clock
 * @param[in]	  xtalPin : Pin number of NPSS_GPIO. Possible values are 0,1,2,3,4
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_XtalClkConfig(uint8_t xtalPin)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_xtal_clk_config(xtalPin);
#else
  return clk_xtal_clk_config(xtalPin);
#endif
}

#ifdef CHIP_9118
/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_USBClkConfig(M4CLK_Type *pCLK ,USB_CLK_SRC_SEL_T clkSource ,uint16_t divFactor)
 * @brief		    This API is used to configure the USB clock
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  clkSource : Different clock sources for USB_PHY_CLK. See #USB_CLK_SRC_SEL_T for more info
 * @param[in]	  divFactor : USB clock division value
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_USBClkConfig(M4CLK_Type *pCLK, USB_CLK_SRC_SEL_T clkSource, uint16_t divFactor)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_usb_clk_config(pCLK, clkSource, divFactor);
#else
  return clk_usb_clk_config(pCLK, clkSource, divFactor);
#endif
}
#endif

/**
 * @fn	        STATIC INLINE rsi_error_t RSI_CLK_PeripheralClkEnable(M4CLK_Type *pCLK ,PERIPHERALS_CLK_T module,CLK_ENABLE_T clkType)
 * @brief		    This API is used to enable the particular clock
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  module : To select particular pheripheral.
 * @param[in]	  clkType : To select the clock as dynamic or static clock.
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_PeripheralClkEnable(M4CLK_Type *pCLK, PERIPHERALS_CLK_T module, CLK_ENABLE_T clkType)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_peripheral_clk_enable(pCLK, module, clkType);
#else
  return clk_peripheral_clk_enable(pCLK, module, clkType);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_CLK_PeripheralClkDisable(M4CLK_Type *pCLK ,PERIPHERALS_CLK_T module)
 * @brief		    This API is used to disable the particular clock
 * @param[in]	  pCLK : Pointer to the pll register instance
 * @param[in]	  module : To select particular peripheral.
 * @return 		  returns zero \ref RSI_OK  on success ,on failure return error code.
 */
STATIC INLINE rsi_error_t RSI_CLK_PeripheralClkDisable(M4CLK_Type *pCLK, PERIPHERALS_CLK_T module)
{
#if defined(CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_M4SS_CLK_API->clk_peripheral_clk_disable(pCLK, module);
#else
  return clk_peripheral_clk_disable(pCLK, module);
#endif
}

/**
 *@fn              void RSI_CLK_SocPllLockConfig(boolean_t manual_lock , boolean_t bypass_manual_lock ,  uint8_t mm_count_limit)
 *@brief           This API is used to configure the SOC-PLL lock settings
 *@param[in]       manual_lock        : Enable for manual mode phase lock generation
 *@param[in]       bypass_manual_lock : Bypass signal for phase detector logic
 *@param[in]       mm_count_limit     : Lock count limit in manual mode phase lock generation
 */
STATIC INLINE void RSI_CLK_SocPllLockConfig(boolean_t manual_lock, boolean_t bypass_manual_lock, uint8_t mm_count_limit)
{
#if defined(A11_ROM) && defined(CLOCK_ROMDRIVER_PRESENT)
  ROMAPI_M4SS_CLK_API->clk_config_pll_lock(manual_lock, bypass_manual_lock, mm_count_limit);
#else
  clk_config_pll_lock(manual_lock, bypass_manual_lock, mm_count_limit);
#endif
}

/**
 *@fn              STATIC INLINE void RSI_CLK_SocPllRefClkConfig(uint8_t ref_clk_src)
 *@brief           This API is used to configure SOC-PLL config
 *@param[in]       ref_clk_src        : Reference clock to be programmed.
 */
STATIC INLINE void RSI_CLK_SocPllRefClkConfig(uint8_t ref_clk_src)
{
#if defined(A11_ROM) && defined(CLOCK_ROMDRIVER_PRESENT)
  ROMAPI_M4SS_CLK_API->clk_config_pll_ref_clk(ref_clk_src);
#else
  clk_config_pll_ref_clk(ref_clk_src);
#endif
}

#ifdef __cplusplus
}
#endif

#endif //__RSI_ROM_CLK_H_

/*@} end of RSI_M4SS_CLOCK_DRIVERS  */

/* @} end of RSI_CHIP_CLOCK_DRIVERS  */
