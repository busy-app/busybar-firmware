/*******************************************************************************
* @file  rsi_rom_ulpss_clk.h
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

#ifndef __RSI_ROM_ULPSS_CLK_H__
#define __RSI_ROM_ULPSS_CLK_H__

#include "rsi_rom_egpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 */

/**
 * \defgroup RSI_CHIP_CLOCK_DRIVERS CLOCK 
 *  @{
 *
 */

/**
 * \defgroup RSI_ULPSS_CLOCK_DRIVERS ULPSS CLOCK 
 *  @{
 *
 */
#include "rsi_ccp_user_config.h"
#include "rsi_packing.h"
#include "rsi_ulpss_clk.h"
#if defined(A11_ROM)
#include "rsi_rom_table_si91x.h"
#else
#include "rsi_rom_table_RS1xxxx.h"
#endif
#if SL_WIFI_COMPONENT_INCLUDED
#include "sl_rsi_utility.h"
#include "rsi_m4.h"
#endif

/**
 * @fn          STATIC INLINE rsi_error_t RSI_ULPSS_RefClkConfig(ULPSS_REF_CLK_SEL_T clkSource)
 * @brief		    This API is used to select the ULPSS processor ref clk configuration
 * @param[in]	  clkSource : Enum values of clock source to select as ulp processor ref clock.Please refer #ULPSS_REF_CLK_SEL_T
 * @return 		  returns 0 \ref RSI_OK on success ,Error code on failure
 */
STATIC INLINE rsi_error_t RSI_ULPSS_RefClkConfig(ULPSS_REF_CLK_SEL_T clkSource)
{
#if SL_WIFI_COMPONENT_INCLUDED
  if (clkSource == ULPSS_40MHZ_CLK) {
    /*  Notify NWP that M4 requires XTAL clock source  */
    sli_si91x_xtal_turn_on_request_from_m4_to_TA();
  }
#endif
  return ulpss_ref_clk_config(clkSource);
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_ULPSS_ClockConfig(M4CLK_Type *pCLK, boolean_t clkEnable,uint16_t  divFactor,boolean_t  oddDivFactor)
 * @brief	    	This API is used to select the ULPSS processor clock source when input is soc clk source which is greater than 100 MHz
 * @param[in]   pCLK      : Pointer to the pll register instance
 * @param[in]	  clkEnable : is to enable or disable the ulpss_soc clock
 *              -  Enable  1: Enables the clock
 *              -  Disable 0: Disables the clock
 * @param[in]	  divFactor : To divide the clock, ensure that oddDivFactor is 0 then divFactor must be even value or else odd value
 * @param[in]   oddDivFactor : Selects the type of divider for m4_soc_clk_2ulpss
 *              -  0 => Clock Divider(even) is selected
 *              -  1 => Odd Divider is selected
 * @return 		  returns 0 \ref RSI_OK on success ,Error code on failure
 */
STATIC INLINE rsi_error_t RSI_ULPSS_ClockConfig(M4CLK_Type *pCLK,
                                                boolean_t clkEnable,
                                                uint16_t divFactor,
                                                boolean_t oddDivFactor)
{
#if defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_clock_config(pCLK, clkEnable, divFactor, oddDivFactor);
#else
  return ulpss_clock_config(pCLK, clkEnable, divFactor, oddDivFactor);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_ULPSS_UlpProcClkConfig(ULPCLK_Type *pULPCLK ,
					                                      ULP_PROC_CLK_SELECT_T  clkSource,
				                                        uint16_t   divFactor,cdDelay delayFn)
 * @brief		    This API is used to configure the ULPSS processor clock source
 * @param[in]   pULPCLK    : Pointer to the ulp clock register instance
 * @param[in]	  clkSource : Enum values of clock source to select as processor clock.Please refer #ULP_PROC_CLK_SELECT_T and refer  NOTE
 * @param[in]   divFactor : To divide the clock
 * @param[in]   delayFn   : delay enable parameter is 1 for enable 0 for not enable.
 * @return 		  returns 0 \ref RSI_OK on success ,Error code on failure
 * @note        
 *              In order to enable the XTAL CLK source need to configure the NPSS_GPIO pins
 *   	          - which can be done through RSI_CLK_XtalClkConfig(uint8_t xtalPin) API i.e we need to call that API first
 *              - In order to enable the soc CLK source need to configure the Ulpss soc Clk from M4 soc clk
 *              - please refer RSI_ULPSS_ClockConfig(M4CLK_Type *pCLK,boolean_t clkEnable,uint16_t  divFactor,boolean_t  oddDivFactor);
 */
STATIC INLINE rsi_error_t RSI_ULPSS_UlpProcClkConfig(ULPCLK_Type *pULPCLK,
                                                     ULP_PROC_CLK_SELECT_T clkSource,
                                                     uint16_t divFactor,
                                                     cdDelay delayFn)
{
  return ulpss_ulp_proc_clk_config(pULPCLK, clkSource, divFactor, delayFn);
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_ULPSS_UlpPeriClkEnable(ULPCLK_Type *pULPCLK , uint32_t  u32Flags)
 * @brief		    This API is used to enable different pheriheral clocks in ULPSS
 * @param[in]   pULPCLK   : Pointer to the ulp clock register instance
 * @param[in]	  u32Flags : Ored value of the clock enable bits of particular peripheral
 *               - \ref Possible values for this parameter are
 *               - \ref ULPSS_TASS_QUASI_SYNC
 *               - \ref ULPSS_M4SS_SLV_SEL
 *               - \ref AUX_SOC_EXT_TRIG_2_SEL
 *               - \ref AUX_SOC_EXT_TRIG_1_SEL
 *               - \ref AUX_ULP_EXT_TRIG_2_SEL
 *               - \ref AUX_ULP_EXT_TRIG_1_SEL
 *               - \ref TIMER_PCLK_EN
 *               - \ref EGPIO_PCLK_ENABLE
 *               - \ref EGPIO_PCLK_DYN_CTRL_DISABLE
 *               - \ref CLK_ENABLE_ULP_MEMORIES
 *               - \ref VAD_CLK_EN
 *               - \ref FIM_CLK_EN
 *               - \ref REG_ACCESS_SPI_CLK_EN
 *               - \ref EGPIO_CLK_EN
 *               - \ref CLK_ENABLE_TIMER
 *               - \ref VAD_PCLK_ENABLE
 *               - \ref FIM_PCLK_ENABLE
 *               - \ref SCLK_ENABLE_UART
 *               - \ref PCLK_ENABLE_UART
 *               - \ref SCLK_ENABLE_SSI_MASTER
 *               - \ref PCLK_ENABLE_SSI_MASTER
 *               - \ref CLK_ENABLE_I2S
 *               - \ref PCLK_ENABLE_I2C
 *               - \ref RELEASE_SOFT_RESET
 *               - \ref PCM_FSYNC_START
 *               - \ref PCM_ENABLE
 * @return 		returns 0 \ref RSI_OK on success ,Error code on failure
 */
STATIC INLINE rsi_error_t RSI_ULPSS_UlpPeriClkEnable(ULPCLK_Type *pULPCLK, uint32_t u32Flags)
{
#if defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_ulp_peri_clk_enable(pULPCLK, u32Flags);
#else
  return ulpss_ulp_peri_clk_enable(pULPCLK, u32Flags);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_ULPSS_UlpPeriClkDisable(ULPCLK_Type *pULPCLK , uint32_t  u32Flags)
 * @brief		    This API is used to disable different peripheral clocks in ULPSS
 * @param[in]   pULPCLK  : Pointer to the ulp clock register instance
 * @param[in]	  u32Flags is to Ored value of the clock enable bits of particular peripheral
 *               \n Possible values for this parameter are
 *               - \ref ULPSS_TASS_QUASI_SYNC
 *               - \ref ULPSS_M4SS_SLV_SEL
 *               - \ref AUX_SOC_EXT_TRIG_2_SEL
 *               - \ref AUX_SOC_EXT_TRIG_1_SEL
 *               - \ref AUX_ULP_EXT_TRIG_2_SEL
 *               - \ref AUX_ULP_EXT_TRIG_1_SEL
 *               - \ref TIMER_PCLK_EN
 *               - \ref EGPIO_PCLK_ENABLE
 *               - \ref EGPIO_PCLK_DYN_CTRL_DISABLE
 *               - \ref CLK_ENABLE_ULP_MEMORIES
 *               - \ref VAD_CLK_EN
 *               - \ref FIM_CLK_EN
 *               - \ref REG_ACCESS_SPI_CLK_EN
 *               - \ref EGPIO_CLK_EN
 *               - \ref CLK_ENABLE_TIMER
 *               - \ref VAD_PCLK_ENABLE
 *               - \ref FIM_PCLK_ENABLE
 *               - \ref SCLK_ENABLE_UART
 *               - \ref PCLK_ENABLE_UART
 *               - \ref SCLK_ENABLE_SSI_MASTER
 *               - \ref PCLK_ENABLE_SSI_MASTER
 *               - \ref CLK_ENABLE_I2S
 *               - \ref PCLK_ENABLE_I2C
 *               - \ref RELEASE_SOFT_RESET
 *               - \ref PCM_FSYNC_START
 *               - \ref PCM_ENABLE
 * @return 		returns 0 \ref RSI_OK on success ,Error code on failure
 */
STATIC INLINE rsi_error_t RSI_ULPSS_UlpPeriClkDisable(ULPCLK_Type *pULPCLK, uint32_t u32Flags)
{
#if defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_ulp_peri_clk_disable(pULPCLK, u32Flags);
#else
  return ulpss_ulp_peri_clk_disable(pULPCLK, u32Flags);
#endif
}
/**
 * @fn          STATIC INLINE rsi_error_t RSI_ULPSS_UlpDynClkEnable(ULPCLK_Type *pULPCLK , uint32_t  u32Flags)
 * @brief		    This API is used to enable different peripheral clocks in ULPSS
 * @param[in]   pULPCLK   : Pointer to the ulp clock register instance
 * @param[in]	  u32Flags : Ored value of the clock enable bits of particular peripheral
 *               \n Possible values for this parameter are
 *               - \ref ULPSS_TASS_QUASI_SYNC
 *               - \ref ULPSS_M4SS_SLV_SEL
 *               - \ref AUX_SOC_EXT_TRIG_2_SEL
 *               - \ref AUX_SOC_EXT_TRIG_1_SEL
 *               - \ref AUX_ULP_EXT_TRIG_2_SEL
 *               - \ref AUX_ULP_EXT_TRIG_1_SEL
 *               - \ref TIMER_PCLK_EN
 *               - \ref EGPIO_PCLK_ENABLE
 *               - \ref EGPIO_PCLK_DYN_CTRL_DISABLE
 *               - \ref CLK_ENABLE_ULP_MEMORIES
 *               - \ref VAD_CLK_EN
 *               - \ref FIM_CLK_EN
 *               - \ref REG_ACCESS_SPI_CLK_EN
 *               - \ref EGPIO_CLK_EN
 *               - \ref CLK_ENABLE_TIMER
 *               - \ref VAD_PCLK_ENABLE
 *               - \ref FIM_PCLK_ENABLE
 *               - \ref SCLK_ENABLE_UART
 *               - \ref PCLK_ENABLE_UART
 *               - \ref SCLK_ENABLE_SSI_MASTER
 *               - \ref PCLK_ENABLE_SSI_MASTER
 *               - \ref CLK_ENABLE_I2S
 *               - \ref PCLK_ENABLE_I2C
 *               - \ref RELEASE_SOFT_RESET
 *               - \ref PCM_FSYNC_START
 *               - \ref PCM_ENABLE
 * @return 		  returns 0 \ref RSI_OK on success ,Error code on failure
 */
STATIC INLINE rsi_error_t RSI_ULPSS_UlpDynClkEnable(ULPCLK_Type *pULPCLK, uint32_t u32Flags)
{
#if defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_ulp_dyn_clk_enable(pULPCLK, u32Flags);
#else
  return ulpss_ulp_dyn_clk_enable(pULPCLK, u32Flags);
#endif
}
/**
 * @fn          STATIC INLINE rsi_error_t RSI_ULPSS_UlpDynClkDisable(ULPCLK_Type *pULPCLK , uint32_t  u32Flags)
 * @brief		    This API is used to disable different peripheral clocks in ULPSS
 * @param[in]   pULPCLK   : Pointer to the ulp clock register instance
 * @param[in]	  u32Flags : is to Ored value of the clock enable bits of particular peripheral
 *                    \n    Possible values for this parameter are
 *               - \ref ULPSS_TASS_QUASI_SYNC
 *               - \ref ULPSS_M4SS_SLV_SEL
 *               - \ref AUX_SOC_EXT_TRIG_2_SEL
 *               - \ref AUX_SOC_EXT_TRIG_1_SEL
 *               - \ref AUX_ULP_EXT_TRIG_2_SEL
 *               - \ref AUX_ULP_EXT_TRIG_1_SEL
 *               - \ref TIMER_PCLK_EN
 *               - \ref EGPIO_PCLK_ENABLE
 *               - \ref EGPIO_PCLK_DYN_CTRL_DISABLE
 *               - \ref CLK_ENABLE_ULP_MEMORIES
 *               - \ref VAD_CLK_EN
 *               - \ref FIM_CLK_EN
 *               - \ref REG_ACCESS_SPI_CLK_EN
 *               - \ref EGPIO_CLK_EN
 *               - \ref CLK_ENABLE_TIMER
 *               - \ref VAD_PCLK_ENABLE
 *               - \ref FIM_PCLK_ENABLE
 *               - \ref SCLK_ENABLE_UART
 *               - \ref PCLK_ENABLE_UART
 *               - \ref SCLK_ENABLE_SSI_MASTER
 *               - \ref PCLK_ENABLE_SSI_MASTER
 *               - \ref CLK_ENABLE_I2S
 *               - \ref PCLK_ENABLE_I2C
 *               - \ref RELEASE_SOFT_RESET
 *               - \ref PCM_FSYNC_START
 *               - \ref PCM_ENABLE
 * @return 		  returns 0 \ref RSI_OK on success ,Error code on failure
 */
STATIC INLINE rsi_error_t RSI_ULPSS_UlpDynClkDisable(ULPCLK_Type *pULPCLK, uint32_t u32Flags)
{
#if defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_ulp_dyn_clk_disable(pULPCLK, u32Flags);
#else
  return ulpss_ulp_dyn_clk_disable(pULPCLK, u32Flags);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_ULPSS_UlpSsiClkConfig(ULPCLK_Type *pULPCLK ,CLK_ENABLE_T clkType    ,
						          ULP_SSI_CLK_SELECT_T clkSource,
								   uint16_t divFactor)
 * @brief	    	This API is used to configure the SSI clock source
 * @param[in]   pULPCLK    : Pointer to the ulp clock register instance
 * @param[in]	  clkType    : To select the clock as dynamic or static clock. See the #CLK_ENABLE_T for more info
 * @param[in]	  clkSource : Value of clock source to select as Ulp SSI clock. Please refer #ULP_SSI_CLK_SELECT_T for more info
 *              -  0: \ref_clk (output of dynamic clock mux for different possible ref_clk sources)
 *              -  2: \ref ulp_32khz_rc_clk
 *              -  3: \ref ulp_32khz_xtal_clk  #refer NOTE
 *              -  4: \ref ulp_mhz_rc_clk
 *              -  6: \ref soc_clk             #refer NOTE
 * @param[in]   divFactor : To divide the clock
 * @return 		  returns 0 \ref RSI_OK on success ,Error code on failure
 * @note        There are two \ref XTAL Clk sources one is Internal and external \ref XTAL clk source. In order to enable the external XTAL clk source need to configure the \ref NPSS_GPIO pins
 *   	          - which can be done through RSI_CLK_XtalClkConfig(uint8_t xtalPin) API that is we need to call that API first
 *              - In order to enable the soc CLK source need to configure the Ulpss soc Clk from M4 soc clk
 *              - please refer \ref RSI_ULPSS_ClockConfig(M4CLK_Type *pCLK,boolean_t clkEnable,uint16_t  divFactor,boolean_t  oddDivFactor);
 */

STATIC INLINE rsi_error_t RSI_ULPSS_UlpSsiClkConfig(ULPCLK_Type *pULPCLK,
                                                    CLK_ENABLE_T clkType,
                                                    ULP_SSI_CLK_SELECT_T clkSource,
                                                    uint16_t divFactor)
{
#if defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_ulp_ssi_clk_config(pULPCLK, clkType, clkSource, divFactor);
#else
  return ulpss_ulp_ssi_clk_config(pULPCLK, clkType, clkSource, divFactor);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_ULPSS_UlpI2sClkConfig(ULPCLK_Type *pULPCLK,
                                   ULP_I2S_CLK_SELECT_T clkSource ,
	                               uint16_t divFactor)
 * @brief		    This API is used to configure the I2S clock source
 * @param[in]   pULPCLK    : Pointer to the ulp clock register instance
 * @param[in]	  clkSource : Value of clock source to select as I2S clock. Please refer #ULP_SSI_CLK_SELECT_T for more info
 *               - 0: \ref ref_clk (output of dynamic clock mux for different possible ref_clk sources)
 *               - 2: \ref ulp_32khz_rc_clk
 *               - 3: \ref ulp_32khz_xtal_clk     #refer NOTE
 *               - 4: \ref ulp_mhz_rc_clk
 *               - 6: \ref soc_clk                #refer NOTE
 *               - 7: \ref ulp_doubler_clk
 *               - 8: \ref I2S PLL
 * @param[in]  divFactor : To divide the clock
 * @return 		 returns 0 \ref RSI_OK on success ,Error code on failure
 * @note       There are two XTAL Clk sources one is Internal and external XTAL clk source. In order to enable the external XTAL clk source need to configure the NPSS_GPIO pins
 *   	         - which can be done through RSI_CLK_XtalClkConfig(uint8_t xtalPin) API i.e we need to call that API first
 *             - In order to enable the soc CLK source need to configure the Ulpss soc Clk from M4 soc clk
 *             - please refer RSI_ULPSS_ClockConfig(M4CLK_Type *pCLK,boolean_t clkEnable,uint16_t  divFactor,boolean_t  oddDivFactor);
 */
STATIC INLINE rsi_error_t RSI_ULPSS_UlpI2sClkConfig(ULPCLK_Type *pULPCLK,
                                                    ULP_I2S_CLK_SELECT_T clkSource,
                                                    uint16_t divFactor)
{
#if defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_ulp_i2s_clk_config(pULPCLK, clkSource, divFactor);
#else
  return ulpss_ulp_i2s_clk_config(pULPCLK, clkSource, divFactor);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_ULPSS_UlpUartClkConfig(ULPCLK_Type *pULPCLK,CLK_ENABLE_T clkType,
	                                boolean_t bFrClkSel,ULP_UART_CLK_SELECT_T clkSource ,
	                                uint16_t divFactor)
 * @brief		    This API is used to configure the UART clock source
 * @param[in]   pULPCLK     : Pointer to the ulp clock register instance
 * @param[in]	  clkType    : To select the clock as dynamic or static clock. See the #CLK_ENABLE_T for more info
 * @param[in]	  bFrClkSel  : To enable or disable ulp uart clk selection
 *               -  1: Fractional Divider output is selected
 *               -  0: Swallow Divider output is selected
 * @param[in]	  clkSource : Value of clock source to select as processor clock
 *               -  0: \ref ref_clk (output of dynamic clock mux for different possible ref_clk sources)
 *               -  2: \ref ulp_32khz_rc_clk
 *               -  3: \ref ulp_32khz_xtal_clk    #refer NOTE
 *               -  4: \ref ulp_mhz_rc_clk
 *               -  6: \ref soc_clk               #refer NOTE
 * @param[in]    divFactor : To divide the clock
 * @return 		   returns 0 \ref RSI_OK on success ,Error code on failure
 * @note         There are two XTAL Clk sources one is Internal and external XTAL clk source. In order to enable the external XTAL clk source need to configure the NPSS_GPIO pins
 *   	           - which can be done through RSI_CLK_XtalClkConfig(uint8_t xtalPin) API that is we need to call that API first
 *               - In order to enable the soc CLK source need to configure the Ulpss soc Clk from M4 soc clk
 *               - please refer RSI_ULPSS_ClockConfig(M4CLK_Type *pCLK,boolean_t clkEnable,uint16_t  divFactor,boolean_t  oddDivFactor);
 */
STATIC INLINE rsi_error_t RSI_ULPSS_UlpUartClkConfig(ULPCLK_Type *pULPCLK,
                                                     CLK_ENABLE_T clkType,
                                                     boolean_t bFrClkSel,
                                                     ULP_UART_CLK_SELECT_T clkSource,
                                                     uint16_t divFactor)
{
#if defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_ulp_uar_clk_config(pULPCLK, clkType, bFrClkSel, clkSource, divFactor);
#else
  return ulpss_ulp_uar_clk_config(pULPCLK, clkType, bFrClkSel, clkSource, divFactor);
#endif
}
/**
 * @fn           STATIC INLINE rsi_error_t RSI_ULPSS_TimerClkConfig(ULPCLK_Type *pULPCLK ,CLK_ENABLE_T  clkType,
		                                          boolean_t bTmrSync ,ULP_TIMER_CLK_SELECT_T clkSource,
	 	                                          uint8_t   skipSwitchTime)
 * @brief		     This API is used to configure the timer clock source
 * @param[in]    pULPCLK       : Pointer to the ulp clock register instance
 * @param[in]	   clkType      : To select the clock as dynamic or static clock. See the #CLK_ENABLE_T for more info
 * @param[in]	   bTmrSync     : To enable Ulp timer in synchronous mode to ULPSS pclk
 *                 -  1: Enables
 *                 -  0: Disables
 * @param[in]	   clkSource     : Value of clock source to select as Timer clock. Please refer #ULP_TIMER_CLK_SELECT_T for possible values
 *                -  0: \ref ref_clk (output of dynamic clock mux for different possible ref_clk sources)
 *                -  2: \ref ulp_32khz_rc_clk
 *                -  3: \ref ulp_32khz_xtal_clk     #refer NOTE
 *                -  4: \ref ulp_mhz_rc_clk    
 *                -  6: \ref soc_clk                #refer NOTE
 *                -  7: \ref ulp_doubler_clk
 * @param[in]    skipSwitchTime : To skip the switching of timer clk.
 *                -  1 : Wait for switching timer clk
 *                -  0 : Skip waiting for switching timer clk
 * @return 		   returns 0 \ref RSI_OK on success ,Error code on failure
 * @note         There are two XTAL Clk sources one is Internal and external XTAL clk source. In order to enable the external XTAL clk source need to configure the NPSS_GPIO pins
 *   	           - which can be done through RSI_CLK_XtalClkConfig(uint8_t xtalPin) API that is we need to call that API first
 *               - In order to enable the soc CLK source need to configure the Ulpss soc Clk from M4 soc clk
 *               - please refer RSI_ULPSS_ClockConfig(M4CLK_Type *pCLK,boolean_t clkEnable,uint16_t  divFactor,boolean_t  oddDivFactor);
 */
STATIC INLINE rsi_error_t RSI_ULPSS_TimerClkConfig(ULPCLK_Type *pULPCLK,
                                                   CLK_ENABLE_T clkType,
                                                   boolean_t bTmrSync,
                                                   ULP_TIMER_CLK_SELECT_T clkSource,
                                                   uint8_t skipSwitchTime)
{
#if defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_time_clk_config(pULPCLK, clkType, bTmrSync, clkSource, skipSwitchTime);
#else
  return ulpss_time_clk_config(pULPCLK, clkType, bTmrSync, clkSource, skipSwitchTime);
#endif
}

/**
 * @fn           STATIC INLINE rsi_error_t RSI_ULPSS_TimerClkDisable(ULPCLK_Type *pULPCLK )
 * @brief		     This API is used to disable  the timer clock source
 * @param[in]    pULPCLK       : Pointer to the ulp clock register instance
 * @return 		   returns 0 \ref RSI_OK on success ,Error code on failure
 */
STATIC INLINE rsi_error_t RSI_ULPSS_TimerClkDisable(ULPCLK_Type *pULPCLK)
{
#if defined(CHIP_9118) && defined(A11_ROM) && defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_time_clk_disable(pULPCLK);
#else
  return ulpss_time_clk_disable(pULPCLK);
#endif
}

/**
 * @fn           STATIC INLINE rsi_error_t RSI_ULPSS_AuxClkConfig(ULPCLK_Type *pULPCLK ,
														CLK_ENABLE_T clkType,
												 ULP_AUX_CLK_SELECT_T clkSource
														)
 * @brief		     This API is used to configure the AUX clock source
 * @param[in]    pULPCLK    : Pointer to the ulp clock register instance
 * @param[in]	   clkType   : To select the clock as dynamic or static clock. See the #CLK_ENABLE_T for more info
 * @param[in]	   clkSource : Value of clock source to select as AUX clock. Please refer #ULP_AUX_CLK_SELECT_T for more info
 *                -  0: \ref ref_clk (output of dynamic clock mux for different possible ref_clk sources)
 *                -  2: \ref ulp_32khz_rc_clk
 *                -  3: \ref ulp_32khz_xtal_clk     #refer NOTE
 *                -  4: \ref ulp_mhz_rc_clk
 *                -  6: \ref soc_clk                #refer NOTE
 *                -  7: \ref ulp_doubler_clk
 *                -  8: \ref I2S PLL
 * @return 		   returns 0 \ref RSI_OK on success ,Error code on failure
 * @note         - There are two XTAL Clk sources one is Internal and external XTAL clk source. In order to enable the external XTAL clk source need to configure the NPSS_GPIO pins
 *   	           - which can be done through RSI_CLK_XtalClkConfig(uint8_t xtalPin) API that is we need to call that API first
 *               - In order to enable the soc CLK source need to configure the Ulpss soc Clk from M4 soc clk
 *               - please refer RSI_ULPSS_ClockConfig(M4CLK_Type *pCLK,boolean_t clkEnable,uint16_t  divFactor,boolean_t  oddDivFactor);
 */
STATIC INLINE rsi_error_t RSI_ULPSS_AuxClkConfig(ULPCLK_Type *pULPCLK,
                                                 CLK_ENABLE_T clkType,
                                                 ULP_AUX_CLK_SELECT_T clkSource)
{
#if defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_aux_clk_config(pULPCLK, clkType, clkSource);
#else
  return ulpss_aux_clk_config(pULPCLK, clkType, clkSource);
#endif
}

/**
 * @fn           STATIC INLINE rsi_error_t RSI_ULPSS_VadClkConfig(ULPCLK_Type *pULPCLK  ,
													 ULP_VAD_CLK_SELECT_T clkSource  ,
													 ULP_VAD_FCLK_SELECT_T  FclkSource ,
													 uint16_t divFactor)
 * @brief		     This API is used to configure the VAD clock source
 * @param[in]    pULPCLK    : Pointer to the ulp clock register instance
 * @param[in]	   clkSource : Value of clock source to select as vad clock. Please refer #ULP_VAD_CLK_SELECT_T for more info
 *               -  1: \ref ulp_32khz_rc_clk
 *               -  2: \ref ulp_32khz_xtal_clk        #refer NOTE
 *	              \n NOTE: In order to enable the XTAL CLK source need to configure the NPSS_GPIO pins
 *   	            \n which can be done through RSI_CLK_XtalClkConfig(uint8_t xtalPin) API that is we need to call that API first
 * @param[in]	   FclkSource : Ulp vad Fast clock select. Please refer #ULP_VAD_FCLK_SELECT_T for more info
 *               -  0: ulpss processor clock     #refer NOTE
 *               -  1: \ref ref_clk (output of dynamic clock mux for different possible ref_clk sources)
 *               -  2: \ref ulp_mhz_rc_clk
 *               -  4: \ref soc_clk                 #refer NOTE
 * @param[in]    divFactor : To divide the clock
 * @return 		   returns 0 \ref RSI_OK on success ,Error code on failure
 * @note          - There are two XTAL Clk sources one is Internal and external XTAL clk source. In order to enable the external XTAL clk source need to configure the NPSS_GPIO pins
 *   	            - which can be done through RSI_CLK_XtalClkConfig(uint8_t xtalPin) API that is we need to call that API first
 *                - In order to enable the soc CLK source need to configure the Ulpss soc Clk from M4 soc clk
 *                - please refer RSI_ULPSS_ClockConfig(M4CLK_Type *pCLK,boolean_t clkEnable,uint16_t  divFactor,boolean_t  oddDivFactor);
 *                - In order to enable the  ulpss processor clock source need to configure the
 *		            - RSI_ULPSS_UlpProcClkConfig(ULPCLK_Type *pULPCLK ,boolean_t clkEnable,uint8_t clkSource,uint16_t divFactor,delayMs cbDelay )
 */
STATIC INLINE rsi_error_t RSI_ULPSS_VadClkConfig(ULPCLK_Type *pULPCLK,
                                                 ULP_VAD_CLK_SELECT_T clkSource,
                                                 ULP_VAD_FCLK_SELECT_T FclkSource,
                                                 uint16_t divFactor)
{
#if defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_vad_clk_config(pULPCLK, clkSource, FclkSource, divFactor);
#else
  return ulpss_vad_clk_config(pULPCLK, clkSource, FclkSource, divFactor);
#endif
}
/**
 * @fn           STATIC INLINE rsi_error_t RSI_ULPSS_TouchClkConfig(ULPCLK_Type *pULPCLK ,ULP_TOUCH_CLK_SELECT_T clkSource ,
															 uint16_t divFactor )
 * @brief		     This API is used to configure the Touch clock source
 * @param[in]    pULPCLK    : Pointer to the ulp clock register instance
 * @param[in]	   clkSource : Value of clock source to select as Touch clock. Please refer #ULP_TOUCH_CLK_SELECT_T for more info.
 *                         -  0: \ref ref_clk (output of dynamic clock mux for different possible ref_clk sources)  
 *                         -  2: \ref ulp_32khz_rc_clk
 *                         -  3: \ref ulp_32khz_xtal_clk    #refer NOTE
 *                         -  4: \ref ulp_mhz_rc_clk
 *                         -  6: \ref soc_clk               #refer NOTE
 * @param[in]    divFactor : To divide the clock
 * @return 		   returns 0 \ref RSI_OK on success ,Error code on failure
 * @note         There are two XTAL Clk sources one is Internal and external XTAL clk source. In order to enable the external XTAL clk source need to configure the NPSS_GPIO pins
 *   	           - which can be done through RSI_CLK_XtalClkConfig(uint8_t xtalPin) API that is we need to call that API first
 *               - In order to enable the soc CLK source need to configure the Ulpss soc Clk from M4 soc clk
 *               - please refer RSI_ULPSS_ClockConfig(M4CLK_Type *pCLK,boolean_t clkEnable,uint16_t  divFactor,boolean_t  oddDivFactor);
 */
STATIC INLINE rsi_error_t RSI_ULPSS_TouchClkConfig(ULPCLK_Type *pULPCLK,
                                                   ULP_TOUCH_CLK_SELECT_T clkSource,
                                                   uint16_t divFactor)
{
#if defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_touch_clk_config(pULPCLK, clkSource, divFactor);
#else
  return ulpss_touch_clk_config(pULPCLK, clkSource, divFactor);
#endif
}
/**
 * @fn          STATIC INLINE rsi_error_t RSI_ULPSS_SlpSensorClkConfig(ULPCLK_Type *pULPCLK , boolean_t clkEnable ,uint32_t divFactor)
 * @brief		    This API is used to configure the sleep sensor clock source
 * @param[in]   pULPCLK    : Pointer to the ulp clock register instance
 * @param[in]	  clkEnable : To enable or disable the sleep sensor clock
 *                          - Enable  1: Enables the clock
 *                          - Disable 0: Disables the clock
 * @param[in]   divFactor : To divide the clock
 * @return 		  returns 0 \ref RSI_OK on success ,Error code on failure
 * @note        In order to enable the XTAL CLK source need to configure the NPSS_GPIO pins
 *   	          - which can be done through RSI_CLK_XtalClkConfig(uint8_t xtalPin) API that is we need to call that API first
 */
STATIC INLINE rsi_error_t RSI_ULPSS_SlpSensorClkConfig(ULPCLK_Type *pULPCLK, boolean_t clkEnable, uint32_t divFactor)
{
#if defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_slp_sensor_clk_config(pULPCLK, clkEnable, divFactor);
#else
  return ulpss_slp_sensor_clk_config(pULPCLK, clkEnable, divFactor);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_ULPSS_PeripheralEnable(ULPCLK_Type *pULPCLK,
                                                    ULPPERIPHERALS_CLK_T module,CLK_ENABLE_T clkType)
 * @brief		    This API is used to enable the particular ULP peripherial Clock
 * @param[in]   pULPCLK  : Pointer to the ulp clock register instance
 * @param[in]	  module  : To select particular ulp pheripheral. See \ref ULPPERIPHERALS_ENABLE_T for more info.
 * @param[in]   clkType :To select the clock as dynamic or static clock. See the #CLK_ENABLE_T for more info
 * @return 		  returns 0 \ref RSI_OK on success ,Error code on failure
 */
STATIC INLINE rsi_error_t RSI_ULPSS_PeripheralEnable(ULPCLK_Type *pULPCLK,
                                                     ULPPERIPHERALS_CLK_T module,
                                                     CLK_ENABLE_T clkType)
{
#if defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_peripheral_enable(pULPCLK, module, clkType);
#else
  return ulpss_peripheral_enable(pULPCLK, module, clkType);
#endif
}
/**
 * @fn          STATIC INLINE rsi_error_t RSI_ULPSS_PeripheralDisable(ULPCLK_Type *pULPCLK, ULPPERIPHERALS_CLK_T module)
 * @brief		    This API is used to Disable the particular ULP peripherial Clock
 * @param[in]   pULPCLK : Pointer to the ulp clock register instance
 * @param[in]  	module : To select particular ulp pheripheral. \ref ULPPERIPHERALS_DISABLE_T for more info.
 * @return 		  returns 0 \ref RSI_OK on success ,Error code on failure
 */
STATIC INLINE rsi_error_t RSI_ULPSS_PeripheralDisable(ULPCLK_Type *pULPCLK, ULPPERIPHERALS_CLK_T module)
{
#if defined(ULPSS_CLOCK_ROMDRIVER_PRESENT)
  return ROMAPI_ULPSS_CLK_API->ulpss_peripheral_disable(pULPCLK, module);
#else
  return ulpss_peripheral_disable(pULPCLK, module);
#endif
}

STATIC INLINE rsi_error_t RSI_ULPSS_RefClkConfig(ULPSS_REF_CLK_SEL_T clkSource);

STATIC INLINE rsi_error_t RSI_ULPSS_ClockConfig(M4CLK_Type *pCLK,
                                                boolean_t clkEnable,
                                                uint16_t divFactor,
                                                boolean_t oddDivFactor);

STATIC INLINE rsi_error_t RSI_ULPSS_UlpProcClkConfig(ULPCLK_Type *pULPCLK,
                                                     ULP_PROC_CLK_SELECT_T clkSource,
                                                     uint16_t divFactor,
                                                     cdDelay delayFn);

STATIC INLINE rsi_error_t RSI_ULPSS_UlpPeriClkEnable(ULPCLK_Type *pULPCLK, uint32_t u32Flags);

STATIC INLINE rsi_error_t RSI_ULPSS_UlpPeriClkDisable(ULPCLK_Type *pULPCLK, uint32_t u32Flags);

STATIC INLINE rsi_error_t RSI_ULPSS_UlpDynClkEnable(ULPCLK_Type *pULPCLK, uint32_t u32Flags);

STATIC INLINE rsi_error_t RSI_ULPSS_UlpDynClkDisable(ULPCLK_Type *pULPCLK, uint32_t u32Flags);

STATIC INLINE rsi_error_t RSI_ULPSS_UlpSsiClkConfig(ULPCLK_Type *pULPCLK,
                                                    CLK_ENABLE_T clkType,
                                                    ULP_SSI_CLK_SELECT_T clkSource,
                                                    uint16_t divFactor);

STATIC INLINE rsi_error_t RSI_ULPSS_UlpI2sClkConfig(ULPCLK_Type *pULPCLK,
                                                    ULP_I2S_CLK_SELECT_T clkSource,
                                                    uint16_t divFactor);

STATIC INLINE rsi_error_t RSI_ULPSS_UlpUartClkConfig(ULPCLK_Type *pULPCLK,
                                                     CLK_ENABLE_T clkType,
                                                     boolean_t bFrClkSel,
                                                     ULP_UART_CLK_SELECT_T clkSource,
                                                     uint16_t divFactor);

STATIC INLINE rsi_error_t RSI_ULPSS_TimerClkConfig(ULPCLK_Type *pULPCLK,
                                                   CLK_ENABLE_T clkType,
                                                   boolean_t bTmrSync,
                                                   ULP_TIMER_CLK_SELECT_T clkSource,
                                                   uint8_t skipSwitchTime);

STATIC INLINE rsi_error_t RSI_ULPSS_AuxClkConfig(ULPCLK_Type *pULPCLK,
                                                 CLK_ENABLE_T clkType,
                                                 ULP_AUX_CLK_SELECT_T clkSource);

STATIC INLINE rsi_error_t RSI_ULPSS_VadClkConfig(ULPCLK_Type *pULPCLK,
                                                 ULP_VAD_CLK_SELECT_T clkSource,
                                                 ULP_VAD_FCLK_SELECT_T FclkSource,
                                                 uint16_t divFactor);

STATIC INLINE rsi_error_t RSI_ULPSS_TouchClkConfig(ULPCLK_Type *pULPCLK,
                                                   ULP_TOUCH_CLK_SELECT_T clkSource,
                                                   uint16_t divFactor);

STATIC INLINE rsi_error_t RSI_ULPSS_SlpSensorClkConfig(ULPCLK_Type *pULPCLK, boolean_t clkEnable, uint32_t divFactor);

STATIC INLINE rsi_error_t RSI_ULPSS_PeripheralEnable(ULPCLK_Type *pULPCLK,
                                                     ULPPERIPHERALS_CLK_T module,
                                                     CLK_ENABLE_T clkType);

STATIC INLINE rsi_error_t RSI_ULPSS_PeripheralDisable(ULPCLK_Type *pULPCLK, ULPPERIPHERALS_CLK_T module);

#ifdef __cplusplus
}
#endif

#endif /*__RSI_ROM_ULPSS_CLK_H__*/

/*@} end of RSI_ULPSS_CLOCK_DRIVERS  */

/* @} end ofRSI_CHIP_CLOCK_DRIVERS  */
