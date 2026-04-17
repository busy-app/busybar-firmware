/******************************************************************************
* @file  clock_update.c
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
#include "rsi_pll.h"
#include "rsi_power_save.h"
#include "rsi_ulpss_clk.h"

#ifdef __SYSTICK
volatile uint32_t _dwTickCount = 0;

/** @addtogroup SOC2
* @{
*/
/*==============================================*/
/**
 * @fn         void SysTick_Handler(void)
 * @brief      Systic handler use for generation delay for XTAL clock enable      
 * @return     None
 */
void SysTick_Handler(void)
{
  _dwTickCount++;
}

/*==============================================*/
/**
 * @fn         uint32_t GetTickCount(void)
 * @brief      Get tick count.      
 * @return     return the count of tick.
 */
uint32_t GetTickCount(void)
{
  /*gets the tick count from systic ISR */
  return _dwTickCount;
}

/*==============================================*/
/**
 * @fn      void rsi_delay_ms(uint32_t val)
 * @brief   This callback function for generation delay for XTAL clock.    
 * @param[in]   Number of delay in us.   
 * @return  None.
 */
void rsi_delay_ms(uint32_t val)
{
  uint32_t start = 0;

  start = GetTickCount();
  do {
  } while (GetTickCount() - start < val);

  return;
}
#endif

/*===================================================*/
/**
 * @fn          uint32_t RSI_CLK_GetM4RefClock(void)
 * @brief       select M4 ref clock
 * @return      m4_ref_clk: M4 ref clock value    
 */
STATIC INLINE uint32_t RSI_CLK_GetM4RefClock(void)
{
  uint32_t m4_ref_clk = 0;

  switch (system_clocks.m4_ref_clock_source) {
    case ULPSS_REF_BYP_CLK:
      m4_ref_clk = system_clocks.byp_rc_ref_clock;
      break;
    case ULPSS_ULP_MHZ_RC_CLK:
      m4_ref_clk = system_clocks.rc_mhz_clock;
      break;
    case ULPSS_40MHZ_CLK:
      m4_ref_clk = system_clocks.rf_ref_clock;
      break;
    case ULPSS_MEMS_REF_CLK:
      m4_ref_clk = system_clocks.rc_mhz_clock;
      break;
    case ULPSS_ULP_20MHZ_RINGOSC_CLK:
      m4_ref_clk = system_clocks.ro_20mhz_clock;
      break;
    case ULPSS_ULP_DOUBLER_CLK:
      m4_ref_clk = system_clocks.doubler_clock;
      break;
    default:
      break;
  }

  return m4_ref_clk;
}

// It will gets the clock of each peripheral
STATIC INLINE uint32_t RSI_CLK_GetUlpssTouchClock(uint32_t src_clk_mux, uint32_t div_fac)
{
  uint32_t ulpss_touch_src_clk = 0;

  switch (src_clk_mux) {
    case ULP_TOUCH_REF_CLK:
      ulpss_touch_src_clk = system_clocks.ulpss_ref_clk;
      break;
    case ULP_TOUCH_32KHZ_RO_CLK: /*!< 32KHZ RO clock not supported*/
      ulpss_touch_src_clk = system_clocks.ro_32khz_clock;
      break;
    case ULP_TOUCH_32KHZ_RC_CLK:
      ulpss_touch_src_clk = system_clocks.rc_32khz_clock;
      break;
    case ULP_TOUCH_32KHZ_XTAL_CLK:
      ulpss_touch_src_clk = system_clocks.xtal_32khz_clock;
      break;
    case ULP_TOUCH_MHZ_RC_CLK:
      ulpss_touch_src_clk = system_clocks.rc_mhz_clock;
      break;
    case ULP_TOUCH_20MHZ_RO_CLK: /*!< 20MHZ RO clock not supported*/
      ulpss_touch_src_clk = system_clocks.ro_20mhz_clock;
      break;
    case ULP_TOUCH_ULP_SOC_CLK:
      ulpss_touch_src_clk = system_clocks.soc_clock;
      break;
    default:
      break;
  }

  // apply division factor
  if (div_fac != 0) {
    ulpss_touch_src_clk /= div_fac;
  }

  return ulpss_touch_src_clk;
}
STATIC INLINE uint32_t RSI_CLK_GetUlpssAuxClock(uint32_t src_clk_mux)
{
  uint32_t ulpss_aux_src_clk = 0;

  switch (src_clk_mux) {
    case ULP_AUX_REF_CLK:
      ulpss_aux_src_clk = system_clocks.ulpss_ref_clk;
      break;
    case ULP_AUX_32KHZ_RO_CLK: /*!< 32KHZ RO clock not supported*/
      ulpss_aux_src_clk = system_clocks.ro_32khz_clock;
      break;
    case ULP_AUX_32KHZ_RC_CLK:
      ulpss_aux_src_clk = system_clocks.rc_32khz_clock;
      break;
    case ULP_AUX_32KHZ_XTAL_CLK:
      ulpss_aux_src_clk = system_clocks.xtal_32khz_clock;
      break;
    case ULP_AUX_MHZ_RC_CLK:
      ulpss_aux_src_clk = system_clocks.rc_mhz_clock;
      break;
    case ULP_AUX_20MHZ_RO_CLK: /*!< 20MHZ RO clock not supported*/
      ulpss_aux_src_clk = system_clocks.ro_20mhz_clock;
      break;
    case ULP_AUX_ULP_SOC_CLK:
      ulpss_aux_src_clk = system_clocks.soc_clock;
      break;
    case ULP_AUX_ULP_DOUBLER_CLK:
      ulpss_aux_src_clk = system_clocks.doubler_clock;
      break;
    case ULP_AUX_I2S_PLL_CLK:
      ulpss_aux_src_clk = system_clocks.i2s_pll_clock;
      break;
    default:
      break;
  }
  //no division factor

  return ulpss_aux_src_clk;
}
STATIC INLINE uint32_t RSI_CLK_GetUlpssTimerClock(uint32_t src_clk_mux)
{
  uint32_t ulpss_timer_src_clk = 0;

  switch (src_clk_mux) {
    case ULP_TIMER_REF_CLK:
      ulpss_timer_src_clk = system_clocks.ulpss_ref_clk;
      break;
    case ULP_TIMER_32KHZ_RO_CLK: /*!< 32KHZ RO clock not supported*/
      ulpss_timer_src_clk = system_clocks.ro_32khz_clock;
      break;
    case ULP_TIMER_32KHZ_RC_CLK:
      ulpss_timer_src_clk = system_clocks.rc_32khz_clock;
      break;
    case ULP_TIMER_32KHZ_XTAL_CLK:
      ulpss_timer_src_clk = system_clocks.xtal_32khz_clock;
      break;
    case ULP_TIMER_MHZ_RC_CLK:
      ulpss_timer_src_clk = system_clocks.rc_mhz_clock;
      break;
    case ULP_TIMER_20MHZ_RO_CLK: /*!< 20MHZ RO clock not supported*/
      ulpss_timer_src_clk = system_clocks.ro_20mhz_clock;
      break;
    case ULP_TIMER_ULP_SOC_CLK:
      ulpss_timer_src_clk = system_clocks.soc_clock;
      break;
    default:
      break;
  }
  //no division factor

  return ulpss_timer_src_clk;
}
STATIC INLINE uint32_t RSI_CLK_GetUlpssUartClock(uint32_t src_clk_mux, uint32_t div_fac)
{
  uint32_t ulpss_uart_src_clk = 0;

  switch (src_clk_mux) {
    case ULP_UART_REF_CLK:
      ulpss_uart_src_clk = system_clocks.ulpss_ref_clk;
      break;
    case ULP_UART_ULP_32KHZ_RO_CLK: /*!< 32KHZ RO clock not supported*/
      ulpss_uart_src_clk = system_clocks.ro_32khz_clock;
      break;
    case ULP_UART_ULP_32KHZ_RC_CLK:
      ulpss_uart_src_clk = system_clocks.rc_32khz_clock;
      break;
    case ULP_UART_ULP_32KHZ_XTAL_CLK:
      ulpss_uart_src_clk = system_clocks.xtal_32khz_clock;
      break;
    case ULP_UART_ULP_MHZ_RC_CLK:
      ulpss_uart_src_clk = system_clocks.rc_mhz_clock;
      break;
    case ULP_UART_ULP_20MHZ_RO_CLK: /*!< 20MHZ RO clock not supported*/
      ulpss_uart_src_clk = system_clocks.ro_20mhz_clock;
      break;
    case ULP_UART_SOC_CLK:
      ulpss_uart_src_clk = system_clocks.soc_clock;
      break;
    case ULP_UART_ULP_DOUBLER_CLK:
      ulpss_uart_src_clk = system_clocks.doubler_clock;
      break;
    default:
      break;
  }

  // apply division factor
  if (div_fac != 0) {
    ulpss_uart_src_clk /= div_fac;
  }

  return ulpss_uart_src_clk;
}
STATIC INLINE uint32_t RSI_CLK_GetUlpssI2sClock(uint32_t src_clk_mux, uint32_t div_fac)
{
  uint32_t ulpss_i2s_src_clk = 0;

  switch (src_clk_mux) {
    case ULP_I2S_REF_CLK:
      ulpss_i2s_src_clk = system_clocks.ulpss_ref_clk;
      break;
    case ULP_I2S_ULP_32KHZ_RO_CLK: /*!< 32KHZ RO clock not supported*/
      ulpss_i2s_src_clk = system_clocks.ro_32khz_clock;
      break;
    case ULP_I2S_ULP_32KHZ_RC_CLK:
      ulpss_i2s_src_clk = system_clocks.rc_32khz_clock;
      break;
    case ULP_I2S_ULP_32KHZ_XTAL_CLK:
      ulpss_i2s_src_clk = system_clocks.xtal_32khz_clock;
      break;
    case ULP_I2S_ULP_MHZ_RC_CLK:
      ulpss_i2s_src_clk = system_clocks.rc_mhz_clock;
      break;
    case ULP_I2S_ULP_20MHZ_RO_CLK: /*!< 20MHZ RO clock not supported*/
      ulpss_i2s_src_clk = system_clocks.ro_20mhz_clock;
      break;
    default:
      break;
  }

  // apply division factor
  if (div_fac != 0) {
    ulpss_i2s_src_clk /= (2 * div_fac);
  }

  return ulpss_i2s_src_clk;
}
STATIC INLINE uint32_t RSI_CLK_GetUlpssSsiClock(uint32_t src_clk_mux, uint32_t div_fac)
{
  uint32_t ulpss_ssi_src_clk = 0;

  switch (src_clk_mux) {
    case ULP_SSI_REF_CLK:
      ulpss_ssi_src_clk = system_clocks.ulpss_ref_clk;
      break;
    case ULP_SSI_ULP_32KHZ_RO_CLK: /*!< 32KHZ RO clock not supported*/
      ulpss_ssi_src_clk = system_clocks.ro_32khz_clock;
      break;
    case ULP_SSI_ULP_32KHZ_RC_CLK:
      ulpss_ssi_src_clk = system_clocks.rc_32khz_clock;
      break;
    case ULP_SSI_ULP_32KHZ_XTAL_CLK:
      ulpss_ssi_src_clk = system_clocks.xtal_32khz_clock;
      break;
    case ULP_SSI_ULP_MHZ_RC_CLK:
      ulpss_ssi_src_clk = system_clocks.rc_mhz_clock;
      break;
    case ULP_SSI_ULP_20MHZ_RO_CLK: /*!< 20MHZ RO clock not supported*/
      ulpss_ssi_src_clk = system_clocks.ro_20mhz_clock;
      break;
    case ULP_SSI_SOC_CLK:
      ulpss_ssi_src_clk = system_clocks.soc_clock;
      break;
    default:
      break;
  }

  // apply division factor
  if (div_fac != 0) {
    ulpss_ssi_src_clk /= div_fac;
  }

  return ulpss_ssi_src_clk;
}
STATIC INLINE uint32_t RSI_CLK_GetI2smClock(uint32_t src_clk_mux, uint32_t div_fac)
{
  uint32_t i2sm_src_clk = 0;

  switch (src_clk_mux) {
    case I2S_PLLCLK:
      i2sm_src_clk = system_clocks.i2s_pll_clock;
      break;
    case I2S_M4SOCCLKFOROTHERS:
      i2sm_src_clk = system_clocks.soc_clock;
      break;
    default:
      break;
  }
  // apply division factor
  if (div_fac != 0) {
    i2sm_src_clk /= (2 * div_fac);
  }

  return i2sm_src_clk;
}
STATIC INLINE uint32_t RSI_CLK_GetEthernetClock(uint32_t src_clk_mux, uint32_t div_fac, uint32_t swallow_val)
{
  uint32_t ethernet_src_clk = 0;

  switch (src_clk_mux) {
    case ETH_INTF_PLL_CLK:
      ethernet_src_clk = system_clocks.intf_pll_clock;
      break;
    case ETH_SOC_PLL_CLK:
      ethernet_src_clk = system_clocks.soc_pll_clock;
      break;
    default:
      break;
  }

  // apply division factor
  if (swallow_val) {
    if (div_fac != 0) {
      ethernet_src_clk /= div_fac;
    }
  } else {
    if (div_fac == 0) {
      ethernet_src_clk /= 2; //divider bypassed
    } else {
      ethernet_src_clk /= (2 * div_fac);
    }
  }

  return ethernet_src_clk;
}
STATIC INLINE uint32_t RSI_CLK_GetGspiClock(uint32_t src_clk_mux)
{
  uint32_t gspi_src_clk = 0;

  switch (src_clk_mux) {
    case GSPI_M4_SOC_CLK_FOR_OTHER_CLKS:
      gspi_src_clk = system_clocks.soc_clock;
      break;
    case GSPI_ULP_REF_CLK:
      gspi_src_clk = RSI_CLK_GetM4RefClock();
      break;
    case GSPI_SOC_PLL_CLK:
      gspi_src_clk = system_clocks.soc_pll_clock;
      break;
    case GSPI_MODEM_PLL_CLK2:
      gspi_src_clk = system_clocks.modem_pll_clock2;
      break;
    case GSPI_INTF_PLL_CLK:
      gspi_src_clk = system_clocks.intf_pll_clock;
      break;
    default:
      break;
  }
  //no division factor

  return gspi_src_clk;
}
STATIC INLINE uint32_t RSI_CLK_GetQspi2Clock(uint32_t src_clk_mux,
                                             uint32_t div_fac,
                                             uint32_t swallow_val,
                                             uint32_t odd_div)
{
  uint32_t qspi2_src_clk = 0;

  switch (src_clk_mux) {
    case QSPI_ULPREFCLK:
      qspi2_src_clk = system_clocks.rc_mhz_clock;
      break;
    case QSPI_MODELPLLCLK2:
      qspi2_src_clk = system_clocks.modem_pll_clock2;
      break;
    case QSPI_INTFPLLCLK:
      qspi2_src_clk = system_clocks.intf_pll_clock;
      break;
    case QSPI_SOCPLLCLK:
      qspi2_src_clk = system_clocks.soc_pll_clock;
      break;
    case M4_SOCCLKNOSWLSYNCCLKTREEGATED:
      break;
    default:
      break;
  }

  // apply division factor
  if (swallow_val) {
    if (odd_div == 0) {
      if (div_fac != 0) {
        qspi2_src_clk /= div_fac;
      }
    } else {
      if ((div_fac % 2 != 0) && (div_fac >= 3)) {
        qspi2_src_clk /= div_fac;
      }
    }
  } else {
    if (odd_div == 0) {
      if (div_fac == 0) {
        qspi2_src_clk /= 2;
      } else {
        qspi2_src_clk /= (2 * div_fac);
      }
    } else {
      if ((div_fac % 2 != 0) && (div_fac >= 3)) {
        qspi2_src_clk /= div_fac;
      }
    }
  }

  return qspi2_src_clk;
}
STATIC INLINE uint32_t RSI_CLK_GetQspiClock(uint32_t src_clk_mux,
                                            uint32_t div_fac,
                                            uint32_t swallow_val,
                                            uint32_t odd_div)
{
  uint32_t qspi_src_clk = 0;

  switch (src_clk_mux) {
    case QSPI_ULPREFCLK:
      qspi_src_clk = system_clocks.rc_mhz_clock;
      break;
    case QSPI_MODELPLLCLK2:
      qspi_src_clk = system_clocks.modem_pll_clock2;
      break;
    case QSPI_INTFPLLCLK:
      qspi_src_clk = system_clocks.intf_pll_clock;
      break;
    case QSPI_SOCPLLCLK:
      qspi_src_clk = system_clocks.soc_pll_clock;
      break;
    case M4_SOCCLKNOSWLSYNCCLKTREEGATED:
      break;
    default:
      break;
  }

  // apply division factor
  if (swallow_val) {
    if (odd_div == 0) {
      if (div_fac != 0) {
        qspi_src_clk /= div_fac;
      }
    } else {
      if ((div_fac % 2 != 0) && (div_fac >= 3)) {
        qspi_src_clk /= div_fac;
      }
    }
  } else {
    if (odd_div == 0) {
      if (div_fac == 0) {
        qspi_src_clk /= 2;
      } else {
        qspi_src_clk /= (2 * div_fac);
      }
    } else {
      if ((div_fac % 2 != 0) && (div_fac >= 3)) {
        qspi_src_clk /= div_fac;
      }
    }
  }

  return qspi_src_clk;
}
#if !defined(SLI_SI917)
STATIC INLINE uint32_t RSI_CLK_GetCciClock(uint32_t src_clk_mux, uint32_t div_fac)
{
  uint32_t cci_src_clk = 0;

  switch (src_clk_mux) {
    case CCI_M4_SOC_CLK_FOR_OTHER_CLKS:
      cci_src_clk = system_clocks.soc_clock;
      break;
    case CCI_INTF_PLL_CLK:
      cci_src_clk = system_clocks.intf_pll_clock;
      break;
    default:
      break;
  }

  // apply division factor
  if (div_fac != 0) {
    cci_src_clk /= (2 * div_fac);
  }

  return cci_src_clk;
}
STATIC INLINE uint32_t RSI_CLK_GetSdMemClock(uint32_t src_clk_mux, uint32_t div_fac, uint32_t swallow_val)
{
  uint32_t sdmem_src_clk = 0;

  switch (src_clk_mux) {
    case SDMEM_SOCPLLCLK:
      sdmem_src_clk = system_clocks.soc_pll_clock;
      break;
    case SDMEM_MODEMPLLCLK1:
      sdmem_src_clk = system_clocks.modem_pll_clock2;
      break;
    case SDMEM_INTFPLLCLK:
      sdmem_src_clk = system_clocks.intf_pll_clock;
      break;
    case M4_SOCCLKFOROTHERCLKSSDMEM:
      sdmem_src_clk = system_clocks.soc_clock;
      break;
    default:
      break;
  }

  // apply division factor
  if (swallow_val) {
    if (div_fac != 0) {
      sdmem_src_clk /= div_fac;
    }
  } else {
    if (div_fac == 0) {
      sdmem_src_clk /= 2; //divider bypassed
    } else {
      sdmem_src_clk /= (2 * div_fac);
    }
  }

  return sdmem_src_clk;
}
#endif
STATIC INLINE uint32_t RSI_CLK_GetCtClock(uint32_t src_clk_mux, uint32_t div_fac)
{
  uint32_t ct_src_clk = 0;

  switch (src_clk_mux) {
    case CT_ULPREFCLK:
      ct_src_clk = RSI_CLK_GetM4RefClock();
      break;
    case CT_INTFPLLCLK:
      ct_src_clk = system_clocks.intf_pll_clock;
      break;
    case CT_SOCPLLCLK:
      ct_src_clk = system_clocks.soc_pll_clock;
      break;
    case M4_SOCCLKFOROTHERCLKSCT:
      ct_src_clk = system_clocks.soc_clock;
      break;
    default:
      break;
  }

  // apply division factor
  if (div_fac != 0) {
    ct_src_clk /= (2 * div_fac);
  }

  return ct_src_clk;
}
STATIC INLINE uint32_t RSI_CLK_GetSsiMstClock(uint32_t src_clk_mux, uint32_t div_fac)
{
  uint32_t ssi_mst_src_clk = 0;

  switch (src_clk_mux) {
    case SSI_ULPREFCLK:
      ssi_mst_src_clk = RSI_CLK_GetM4RefClock();
      break;
    case SSI_SOCPLLCLK:
      ssi_mst_src_clk = system_clocks.soc_pll_clock;
      break;
    case SSI_MODEMPLLCLK1:
      ssi_mst_src_clk = system_clocks.modem_pll_clock;
      break;
    case SSI_INTFPLLCLK:
      ssi_mst_src_clk = system_clocks.intf_pll_clock;
      break;
    case SSI_MODELPLLCLK2:
      ssi_mst_src_clk = system_clocks.modem_pll_clock2;
      break;
    case M4_SOCCLKFOROTHERCLKS:
      ssi_mst_src_clk = system_clocks.soc_clock;
      break;
    default:
      break;
  }

  // apply division factor
  if (div_fac != 0) {
    ssi_mst_src_clk /= div_fac;
  }

  return ssi_mst_src_clk;
}
STATIC INLINE uint32_t RSI_CLK_GetUart1Clock(uint32_t src_clk_mux, uint32_t div_fac, uint32_t swallow_val)
{
  uint32_t uart1_src_clk = 0;

  switch (src_clk_mux) {
    case USART_ULPREFCLK:
      uart1_src_clk = RSI_CLK_GetM4RefClock();
      break;
    case USART_MODELPLLCLK2:
      uart1_src_clk = system_clocks.modem_pll_clock2;
      break;
    case USART_INTFPLLCLK:
      uart1_src_clk = system_clocks.intf_pll_clock;
      break;
    case USART_SOCPLLCLK:
      uart1_src_clk = system_clocks.soc_pll_clock;
      break;
    case M4_SOCCLKFOROTHERCLOCKS:
      uart1_src_clk = system_clocks.soc_clock;
      break;
    default:
      break;
  }

  // apply division factor
  if (swallow_val) {
    if (div_fac != 0) {
      uart1_src_clk = (uint32_t)(uart1_src_clk / (div_fac + 0.5));
    }
  } else {
    if (div_fac != 0) {
      uart1_src_clk /= div_fac;
    }
  }

  return uart1_src_clk;
}
STATIC INLINE uint32_t RSI_CLK_GetUsart0Clock(uint32_t src_clk_mux, uint32_t div_fac, uint32_t swallow_val)
{
  uint32_t usart0_src_clk = 0;

  switch (src_clk_mux) {
    case USART_ULPREFCLK:
      usart0_src_clk = RSI_CLK_GetM4RefClock();
      break;
    case USART_MODELPLLCLK2:
      usart0_src_clk = system_clocks.modem_pll_clock2;
      break;
    case USART_INTFPLLCLK:
      usart0_src_clk = system_clocks.intf_pll_clock;
      break;
    case USART_SOCPLLCLK:
      usart0_src_clk = system_clocks.soc_pll_clock;
      break;
    case M4_SOCCLKFOROTHERCLOCKS:
      usart0_src_clk = system_clocks.soc_clock;
      break;
    default:
      break;
  }
  // apply division factor
  if (swallow_val) {
    if (div_fac != 0) {
      usart0_src_clk = (uint32_t)(usart0_src_clk / (div_fac + 0.5));
    }
  } else {
    if (div_fac != 0) {
      usart0_src_clk /= div_fac;
    }
  }

  return usart0_src_clk;
}

/*==============================================*/
/**
 * @fn          uint32_t RSI_CLK_GetBaseClock(PERI_CLKS_T peri_src)
 * @brief       gets the clock of each peripheral    
 * @param[in]   peri_src
 * @return      src_clk
 */
uint32_t RSI_CLK_GetBaseClock(PERI_CLKS_T peri_src)
{
  uint32_t src_clk_mux = 0;
  uint32_t src_clk     = 0;
  uint32_t div_fac     = 0;
  uint32_t swallow_val = 0;
  uint32_t odd_div     = 0;

  switch (peri_src) {
    case M4_USART0:
      // Read the ULP UART clock mux
      src_clk_mux = M4CLK->CLK_CONFIG_REG2_b.USART1_SCLK_SEL;
      div_fac     = M4CLK->CLK_CONFIG_REG2_b.USART1_SCLK_DIV_FAC;
      swallow_val = M4CLK->CLK_CONFIG_REG2_b.USART1_SCLK_FRAC_SEL;
      src_clk     = RSI_CLK_GetUsart0Clock(src_clk_mux, div_fac, swallow_val);
      break;

    case M4_UART1:
      src_clk_mux = M4CLK->CLK_CONFIG_REG2_b.USART2_SCLK_SEL;
      div_fac     = M4CLK->CLK_CONFIG_REG2_b.USART2_SCLK_DIV_FAC;
      swallow_val = M4CLK->CLK_CONFIG_REG2_b.USART2_SCLK_FRAC_SEL;
      src_clk     = RSI_CLK_GetUart1Clock(src_clk_mux, div_fac, swallow_val);
      break;

    case M4_SSI_MST:
      src_clk_mux = M4CLK->CLK_CONFIG_REG1_b.SSI_MST_SCLK_SEL;
      div_fac     = M4CLK->CLK_CONFIG_REG1_b.SSI_MST_SCLK_DIV_FAC;
      src_clk     = RSI_CLK_GetSsiMstClock(src_clk_mux, div_fac);
      break;

    case M4_CT:
      src_clk_mux = M4CLK->CLK_CONFIG_REG5_b.CT_CLK_SEL;
      div_fac     = M4CLK->CLK_CONFIG_REG5_b.CT_CLK_DIV_FAC;
      src_clk     = RSI_CLK_GetCtClock(src_clk_mux, div_fac);
      break;

#if !defined(SLI_SI917)
    case M4_SD_MEM:
      src_clk_mux = M4CLK->SD_MEM_CLOCK_REG_b.SD_MEM_INTF_CLK_SEL;
      div_fac     = M4CLK->SD_MEM_CLOCK_REG_b.SD_MEM_INTF_CLK_DIV_FAC;
      swallow_val = M4CLK->SD_MEM_CLOCK_REG_b.SD_MEM_INTF_CLK_SWALLOW_SEL;
      src_clk     = RSI_CLK_GetSdMemClock(src_clk_mux, div_fac, swallow_val);
      break;

    case M4_CCI:
      src_clk_mux = M4CLK->CLK_CONFIG_REG4_b.CCI_CLK_SEL;
      div_fac     = M4CLK->CLK_CONFIG_REG2_b.CCI_CLK_DIV_FAC;
      src_clk     = RSI_CLK_GetCciClock(src_clk_mux, div_fac);
      break;
#endif
    case M4_QSPI:
      src_clk_mux = M4CLK->CLK_CONFIG_REG1_b.QSPI_CLK_SEL;
      div_fac     = M4CLK->CLK_CONFIG_REG1_b.QSPI_CLK_DIV_FAC;
      swallow_val = M4CLK->CLK_CONFIG_REG1_b.QSPI_CLK_SWALLOW_SEL;
      odd_div     = M4CLK->CLK_CONFIG_REG2_b.QSPI_ODD_DIV_SEL;
      src_clk     = RSI_CLK_GetQspiClock(src_clk_mux, div_fac, swallow_val, odd_div);
      break;

    case M4_QSPI2:
      src_clk_mux = M4CLK->CLK_CONFIG_REG6_b.QSPI_2_CLK_SEL;
      div_fac     = M4CLK->CLK_CONFIG_REG1_b.QSPI_CLK_DIV_FAC;
      swallow_val = M4CLK->CLK_CONFIG_REG1_b.QSPI_CLK_SWALLOW_SEL;
      odd_div     = M4CLK->CLK_CONFIG_REG2_b.QSPI_ODD_DIV_SEL;
      src_clk     = RSI_CLK_GetQspi2Clock(src_clk_mux, div_fac, swallow_val, odd_div);
      break;

    case M4_GSPI:
      src_clk_mux = M4CLK->CLK_CONFIG_REG1_b.GEN_SPI_MST1_SCLK_SEL;
      src_clk     = RSI_CLK_GetGspiClock(src_clk_mux);
      break;

    case M4_ETHERNET:
      src_clk_mux = M4CLK->CLK_CONFIG_REG1_b.PLL_INTF_CLK_SEL;
      div_fac     = M4CLK->CLK_CONFIG_REG1_b.PLL_INTF_CLK_DIV_FAC;
      swallow_val = M4CLK->CLK_CONFIG_REG1_b.PLL_INTF_CLK_SWALLOW_SEL;
      src_clk     = RSI_CLK_GetEthernetClock(src_clk_mux, div_fac, swallow_val);
      break;

    case M4_I2SM:
      src_clk_mux = M4CLK->CLK_CONFIG_REG5_b.I2S_CLK_SEL;
      div_fac     = M4CLK->CLK_CONFIG_REG5_b.I2S_CLK_DIV_FAC;
      src_clk     = RSI_CLK_GetI2smClock(src_clk_mux, div_fac);
      break;

    case ULPSS_SSI:
      src_clk_mux = ULPCLK->ULP_I2C_SSI_CLK_GEN_REG_b.ULP_SSI_CLK_SEL;
      div_fac     = ULPCLK->ULP_I2C_SSI_CLK_GEN_REG_b.ULP_SSI_CLK_DIV_FACTOR;
      src_clk     = RSI_CLK_GetUlpssSsiClock(src_clk_mux, div_fac);
      break;

    case ULPSS_I2S:
      src_clk_mux = ULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_CLK_SEL_b;
      div_fac     = ULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_CLKDIV_FACTOR;
      src_clk     = RSI_CLK_GetUlpssI2sClock(src_clk_mux, div_fac);
      break;

    case ULPSS_UART:
      src_clk_mux = ULPCLK->ULP_UART_CLK_GEN_REG_b.ULP_UART_CLK_SEL;
      div_fac     = ULPCLK->ULP_UART_CLK_GEN_REG_b.ULP_UART_CLKDIV_FACTOR;
      src_clk     = RSI_CLK_GetUlpssUartClock(src_clk_mux, div_fac);
      break;

    case ULPSS_TIMER:
      src_clk_mux = ULPCLK->ULP_TIMER_CLK_GEN_REG_b.ULP_TIMER_CLK_SEL;
      src_clk     = RSI_CLK_GetUlpssTimerClock(src_clk_mux);
      break;

    case ULPSS_AUX:
      src_clk_mux = ULPCLK->ULP_AUXADC_CLK_GEN_REG_b.ULP_AUX_CLK_SEL;
      src_clk     = RSI_CLK_GetUlpssAuxClock(src_clk_mux);
      break;

    case ULPSS_TOUCH:
      src_clk_mux = ULPCLK->ULP_TOUCH_CLK_GEN_REG_b.ULP_TOUCH_CLK_SEL;
      div_fac     = ULPCLK->ULP_TOUCH_CLK_GEN_REG_b.ULP_TOUCH_CLKDIV_FACTOR;
      src_clk     = RSI_CLK_GetUlpssTouchClock(src_clk_mux, div_fac);
      break;
  }
  return src_clk;
}
/** @} */
