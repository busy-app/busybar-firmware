/******************************************************************************
* @file  rsi_power_save.c
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

/**
 * Includes
 */
#include "rsi_ccp_user_config.h"
#include "rsi_rom_power_save.h"
#ifdef SLI_SI91X_MCU_COMMON_FLASH_MODE
#define NWPAON_MEM_HOST_ACCESS_CTRL_CLEAR_1 (*(volatile uint32_t *)(0x41300000 + 0x4))
#define M4SS_TASS_CTRL_SET_REG              (*(volatile uint32_t *)(0x24048400 + 0x34))
#define M4SS_TASS_CTRL_CLR_REG              (*(volatile uint32_t *)(0x24048400 + 0x38))
#endif

#ifndef PS_ROMDRIVER_PRESENT
#define FPGA_MODE 0

/** @addtogroup SOC2
* @{
*/
/*==============================================*/
/** 	    
 * @fn          void ps_clr_wkp_up_status(uint32_t wakeUpIntrClear)
 * @brief       This API is used clear the NPSS/wake up interrupts.
 * @param[in]   wakeUpIntrClear : 
 *                                - 1: wakeup  the interrupt status
 *                                - 0: clear  the interrupt status.  
 * @return      none   
 */

void ps_clr_wkp_up_status(uint32_t wakeUpIntrClear)
{
  if (wakeUpIntrClear & NPSS_TO_MCU_WDT_INTR) {
    NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_WDT_INTR;
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = WWD_INTERRUPT_STATUS_CLEAR;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_GPIO_INTR_0) {
    NPSS_INTR_CLEAR_REG = NPSS_TO_MCU_GPIO_INTR_0;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_GPIO_INTR_1) {
    NPSS_INTR_CLEAR_REG = NPSS_TO_MCU_GPIO_INTR_1;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_GPIO_INTR_2) {
    NPSS_INTR_CLEAR_REG = NPSS_TO_MCU_GPIO_INTR_2;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_GPIO_INTR_3) {
    NPSS_INTR_CLEAR_REG = NPSS_TO_MCU_GPIO_INTR_3;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_GPIO_INTR_4) {
    NPSS_INTR_CLEAR_REG = NPSS_TO_MCU_GPIO_INTR_4;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_CMP_INTR_1) {
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = COMP1_BASED_WAKEUP_STATUS_CLEAR;
    NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_CMP_INTR_1;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_CMP_INTR_2) {
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = COMP2_BASED_WAKEUP_STATUS_CLEAR;
    NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_CMP_INTR_2;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_CMP_INTR_3) {
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = COMP3_BASED_WAKEUP_STATUS_CLEAR;
    NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_CMP_INTR_3;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_CMP_INTR_4) {
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = COMP4_BASED_WAKEUP_STATUS_CLEAR;
    NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_CMP_INTR_4;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_RFWAKEUP_INTR) {
    NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_RFWAKEUP_INTR;
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = RF_WAKEUP_CLEAR;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_BOD_INTR) {
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = COMP5_BASED_WAKEUP_STATUS_CLEAR;
    NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_BOD_INTR;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_BUTTON_INTR) {
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = COMP6_BASED_WAKEUP_STATUS_CLEAR;
    NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_BUTTON_INTR;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_SDC_INTR) {
    *(volatile uint32_t *)(0x24042404) |= BIT(0);
    *(volatile uint32_t *)(0x24042404) &= ~BIT(0);
    /*Clear SCDC interrupt */
    *(volatile uint32_t *)(0x24042400) |= BIT(0);
    /*Clear SDC interrupt */
    NPSS_INTR_CLEAR_REG = NPSS_TO_MCU_SDC_INTR;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_WIRELESS_INTR) {
    NPSS_INTR_CLEAR_REG = NPSS_TO_MCU_WIRELESS_INTR;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_ALARM_INTR) {
    NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_ALARM_INTR;
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = RTC_ALARM_BASED_WAKEUP_STATUS_CLEAR;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_SEC_INTR) {
    NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_SEC_INTR;
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = RTC_SEC_BASED_STATUS_CLEAR;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_MSEC_INTR) {
    NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_MSEC_INTR;
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = MILLI_SEC_BASED_STATUS_CLEAR;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_PROCESSOR_INTR) {
    NPSS_INTR_CLEAR_REG                = NPSS_TO_MCU_WAKEUP_INTR;
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_REG = 1;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_HOST_INTR) {
    NPSS_INTR_CLEAR_REG                = NPSS_TO_MCU_WAKEUP_INTR;
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_REG = 1;
  }

  if (wakeUpIntrClear & NPSS_TO_MCU_DST_INTR) {
    NPSS_INTR_CLEAR_REG                = NPSS_TO_MCU_WAKEUP_INTR;
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_REG = 1;
  }
  return;
}

/*==============================================*/
/** 	  
 * @fn          void ps_bg_ldo_config(uint8_t ldo_0p6_ctrl, uint8_t ldo_0p6_lp_mode)
 * @brief       This API is used to read ldo data.
 * @param[in]   Mode : 
 *                     - 1: enable the ldo data shift
 *                     - 0: disable the ldo data shift
 * @param[in] ldo_0p6_lp_mode : 
 * @return      none   
 */

void ps_bg_ldo_config(uint8_t ldo_0p6_ctrl, uint8_t ldo_0p6_lp_mode)
{
  volatile uint32_t read_ldo_data = 0;
  read_ldo_data                   = ULP_SPI_MEM_MAP(0x129);

  if (ldo_0p6_lp_mode) {
    read_ldo_data |= (1 << 16);
  } else {
    read_ldo_data &= (uint32_t)(~(1 << 16));
  }
  read_ldo_data &= (uint32_t)(~(0x7 << 18));
  read_ldo_data |= (ldo_0p6_ctrl << 18);
  ULP_SPI_MEM_MAP(0x129) = read_ldo_data;
  return;
}

/*==============================================*/
/** 	  
 * @fn          void ps_configure_trim_values(uint16_t lf_ro_trim,
 *                             uint16_t lf_rc_trim,
 *                             uint16_t hf_ro_trim,
 *                             uint16_t hf_rc_trim,
 *                             uint16_t bg_ptat_trim,
 *                             uint16_t bg_trim)
 * @brief       This API is used to get the trim the values. 
 * @param       programmable trim bits for RO and RC 32kHz clock 
 * @param         lf_ro_trim : trim value for low frequency RO clock
 * @param         lf_rc_trim : trim value for low frequency RC clock
 * @param         hf_ro_trim : trim value for high frequency RO clock
 * @param         hf_rc_trim : trim value for high frequency RC clock
 * @param         bg_ptat_trim : trim value for bg ptat 
 * @param         bg_trim    : trim value for bg(Band Gap)
 * @return      none 
 */

void ps_configure_trim_values(uint16_t lf_ro_trim,
                              uint16_t lf_rc_trim,
                              uint16_t hf_ro_trim,
                              uint16_t hf_rc_trim,
                              uint16_t bg_ptat_trim,
                              uint16_t bg_trim)
{
  uint32_t reg_read = 0;

  /*Programmable trim bits for RO 32KHz clock*/
  reg_read = ULP_SPI_MEM_MAP(0x102);
  reg_read &= (uint32_t)(~(0x1F << 16));
  ULP_SPI_MEM_MAP(0x102) = (reg_read | (lf_ro_trim << 16));

  /*Programmable fine trim for RC 32KHz clock*/
  reg_read = ULP_SPI_MEM_MAP(0x103);
  reg_read &= (uint32_t)(~(0x7F << 14));
  ULP_SPI_MEM_MAP(0x103) = (reg_read | (lf_rc_trim << 14));

  /* Trim bits for RC MHz clock. Do not go beyond 100 */
  reg_read = ULP_SPI_MEM_MAP(0x104);
  reg_read &= (uint32_t)(~(0x7F << 14));
  ULP_SPI_MEM_MAP(0x104) = (reg_read | (hf_rc_trim << 14));

  /*Trim bits for High frequency ring oscillator*/
  reg_read = ULP_SPI_MEM_MAP(0x105);
  reg_read &= (uint32_t)(~(0x7F << 14));
  ULP_SPI_MEM_MAP(0x105) = (reg_read | BIT(12) | (hf_ro_trim << 14));

  /*update the 'bg_r_ptat' and 'bg_r' */
  reg_read = ULP_SPI_MEM_MAP(0x127);
  reg_read &= (uint32_t)(~(0x3F << 16));
  reg_read |= (uint32_t)((bg_ptat_trim << 19) | (bg_trim << 16));
  ULP_SPI_MEM_MAP(0x127) = reg_read;
}
#endif

#if !defined(CHIP_9118) || !defined(A11_ROM) || !defined(PS_ROMDRIVER_PRESENT)

/*==============================================*/
/** 	  
 * @fn          void ps_wireless_shutdown(void)
 * @brief       This API is used to shutdown the wireless
 * @param[in]   Clocks : 1: enable the clock
 *                       0: disable the clock      
 * @return      none   
 */

void ps_wireless_shutdown(void)
{
  /* Enable the NWP FSM and HF FSM clock  for RO*/
  NWP_FSM_CLOCKS_SELECT |= ENABLE_RO_32KHz_N_RO_20MHz;

  /* Disable ULP clocks */
  NWP_FSM_CLK_EN_AND_FIRST_BOOTUP &= ~DISABLE_ULP_CLKS;

  /* Power down NWP Keyholder,NWPSTORE,NWPRTC,NWPFSM,CONENECTBFFS domains   */
  NWPAON_NPSS_PWRCTRL_CLEAR_REG = PWRGATE_EN_N_ULP_NWPDOMAINS;

  /* Enable BGPMU Sleep ,SCDCDC LP mode ,Standby DC1P3   */
  NWP_PMU_CTRLS = (STANDBY_DC1P3 | BGPMU_SLEEP_EN | SCDCDC_LP_MODE_EN);

  /* Enable Clock cleaner off signal for selecting tass ref clk */
  TASS_REF_CLOCK_SELECT |= TASS_REF_CLK_CLEANER_OFF;

  /* Disable Clock cleaner on signal for tass ref clk select */
  TASS_REF_CLOCK_SELECT &= ~TASS_REF_CLK_CLEANER_ON;

  /*  DCDC is configured to give low output voltage. and  SOC LDO is configured to give low output voltage. */
  TASS_PMU_LDO_CTRL_CLEAR = (TASS_SOC_LDO_LVL | TASS_DCDC_LVL);

  /* Shutdown RF */
  RF_AFE_PWR_CTRL_REG |= RF2G_SHUTDOWN;

  /* Power supply is gated for RF and AFE   */
  RF_AFE_PWR_CTRL_REG &= ~(RF2G_PG_EN | AFE_PG_EN);

  /* TASS Controlling SOCPLL Power Gate Disable */
  TASS_M4SS_CRTL_SET_REG = TASS_CTRL_SOCPLL_SPI_PG_EN;

  /* Functional Control signal for is disabled for WLAN Gain LUT , WLAN TxPP LUT,WLAN TxPP LUT,PPE-SHARED LUT,PPE-IMDM LUT*/
  MODEM_BAND1_PWRCTRL_CLEAR_REG = (MODEM_EXT_PWRGATE_EN_N_ULP_BBP_WLAN_GAINLUT | MODEM_EXT_PWRGATE_EN_N_ULP_BBP_TXPPLUT
                                   | MODEM_EXT_PWRGATE_EN_N_ULP_PPE_SHARED | MODEM_EXT_PWRGATE_EN_N_ULP_PPE_IMDM);

  /* Functional Control signal for  ZBBT Gain LUT is disabled */
  MODEM_ZBBT_PWRCTRL_CLEAR_REG = MODEM_EXT_PWRGATE_EN_N_ULP_BBP_ZBBT_GAINLUT;

  /* RTC timer clock period programmed by SOC */
  NWP_RTC_TIMER_CLOCK_PERIOD_SOC = RTC_TIMER_CLK_PERIOD_VALUE;

  /* wait until  Programmed period applied */
  while ((!(NWP_RTC_TIMER_CLOCK_PERIOD_SOC)) & (SPI_RTC_TIMER_CLK_PERIOD_APPLIED))
    ;

  /* Shutdown NWP */
  NWP_FSM_SLEEP_WAKEUP_MODES = COUNT_TICK_ENABLE;
  return;
}
#endif

/** @} */

/*End of file not truncated*/
