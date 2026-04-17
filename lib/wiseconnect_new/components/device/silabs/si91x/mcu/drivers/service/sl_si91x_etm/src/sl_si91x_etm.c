/*******************************************************************************
* @file  sl_si91x_etm.c
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
******************************************************************************/

/*******************************************************************************************************
* @brief : This file contains the API for ETM(Embedded Trace Macrocell).
* @section Description: 
* An Embedded Trace Macrocell (ETM) is a real-time trace module providing instruction execution
* flow and data access patterns, for debugging and optimization. In this file, we are configuring
* the ETM pins and core registers that are needed for ETM initialization. The operating frequency of 
* the ETM in Si917 is greater than or equal to 40MHz. It is configured to operate at 120 MHz by default. 
********************************************************************************************************/

#include "sl_si91x_etm.h"
#include "rsi_power_save.h"
#include "si91x_device.h"
#include "rsi_rom_egpio.h"
#include "rsi_rom_clks.h"

/*****************************************************************************/ /**
 * @fn         void sl_si91x_etm_int(void)
 * @brief      This function initializes ETM.
 * @param[in]  void
 * @return     void     
 *********************************************************************************/
void sl_si91x_etm_int(void)
{
  /*Configure the PLL frequency*/
  RSI_CLK_SetSocPllFreq(M4CLK, PS4_SOC_FREQ, SOC_PLL_REF_FREQUENCY);
  /*Configure the prefetch and registering when SOC clock is more than 120Mhz*/
  /*Configure the SOC PLL to 180MHz*/
  if (PS4_SOC_FREQ >= 120000000) {
    /* Icache registering when clk freq more than 120*/
    ICACHE2_ADDR_TRANSLATE_1_REG = BIT(21);
    /*When set, enables registering in M4-NWP AHB2AHB. This will have performance penalty. This has to be set above 100MHz*/
    MISC_CFG_SRAM_REDUNDANCY_CTRL = BIT(4);
    /*Enable Register ROM as clock frequency is 200 Mhz*/
    MISC_CONFIG_MISC_CTRL1 |= BIT(4);
  }
  /*Configure the GPIO pin mux to MCU clock Out*/
  RSI_EGPIO_PadSelectionEnable(PAD_SEL_MCU_CLK);
  RSI_EGPIO_PadReceiverEnable(GPIO_MCU_CLK);
  RSI_EGPIO_SetPinMux(EGPIO, 0, GPIO_MCU_CLK, EGPIO_PIN_MUX_MODE8);

  RSI_EGPIO_PadSelectionEnable(6);
  /*EGPIO Receive enable*/
  RSI_EGPIO_PadReceiverEnable(11);
  /*Configure the GPIO pin mux to MCU clock Out*/
  RSI_EGPIO_SetPinMux(EGPIO, 0, 11, EGPIO_PIN_MUX_MODE12);
  RSI_CLK_McuClkOutConfig(M4CLK, MCUCLKOUT_SOC_PLL_CLK, 0);

  /*Configure the GPIO pin mux to trace clock Out */
  RSI_EGPIO_PadSelectionEnable(PAD_SEL_TRACE_CLK_OUT);
  RSI_EGPIO_SetPinMux(EGPIO, 0, GPIO_TRACE_CLK_OUT, EGPIO_PIN_MUX_MODE6);
  RSI_EGPIO_SetDir(EGPIO, 0, GPIO_TRACE_CLK_OUT, EGPIO_CONFIG_DIR_OUTPUT);
  /*Configure the GPIO pin mux to trace clock In */
  RSI_EGPIO_PadReceiverEnable(GPIO_TRACE_CLK_IN);
  RSI_EGPIO_PadSelectionEnable(PAD_SEL_TRACE_CLK_IN);
  RSI_EGPIO_SetPinMux(EGPIO, 0, GPIO_TRACE_CLK_IN, EGPIO_PIN_MUX_MODE6);
  RSI_EGPIO_SetDir(EGPIO, 0, GPIO_TRACE_CLK_IN, EGPIO_CONFIG_DIR_INPUT);

  /*Pad selection for trace pin*/
  (*(volatile unsigned int *)(PAD_SEL_REG)) |= PAD_SEL_VALUE_ITM;
  (*(volatile unsigned int *)(REN_SEL_REG)) = REN_SEL_VALUE_ITM;
  /*Mode selection for GPIO_52*/
  (*(volatile unsigned int *)(GPIO52_CONFIG_REG)) = GPIO52_CONFIG_VALUE;
  /*Mode selection for GPIO_53*/
  (*(volatile unsigned int *)(GPIO53_CONFIG_REG)) = GPIO53_CONFIG_VALUE;
  /*Mode selection for GPIO_54*/
  (*(volatile unsigned int *)(GPIO54_CONFIG_REG)) = GPIO54_CONFIG_VALUE;
  /*Mode selection for GPIO_55*/
  (*(volatile unsigned int *)(GPIO55_CONFIG_REG)) = GPIO55_CONFIG_VALUE;
  /*Mode selection for GPIO_56*/
  (*(volatile unsigned int *)(GPIO56_CONFIG_REG)) = GPIO56_CONFIG_VALUE;
  /*Mode selection for GPIO_57*/
  (*(volatile unsigned int *)(GPIO57_CONFIG_REG)) = GPIO57_CONFIG_VALUE;
  /*CoreDebug_BASE*/
  (*(volatile unsigned int *)(0xE000EDF0)) = 0xA05F0003;
  /*CoreDebug_Type*/
  (*(volatile unsigned int *)(0xE000EDFC)) = 0x01000000;
  /*ETMLAR*/
  (*(volatile unsigned int *)(0xE0041FB0)) = 0xC5ACCE55;
  /*ETMCR*/
  (*(volatile unsigned int *)(0xE0041000)) = 0x00000410;
  /*TPIU_CSPSR*/
  (*(volatile unsigned int *)(0xE0040004)) = 0x00000008;
  /*TPIU_ACPR*/
  (*(volatile unsigned int *)(0xE0040010)) = 0x00000000;
  /*TPIU_SPPR*/
  (*(volatile unsigned int *)(0xE00400F0)) = 0x00000000;
  /*TPIU_FFCR*/
  (*(volatile unsigned int *)(0xE0040304)) = 0x00000002;
  /*not available*/
  (*(volatile unsigned int *)(0xE0000FB0)) = 0xC5ACCE55;
  /*ITM_TCR*/
  (*(volatile unsigned int *)(0xE0000E80)) = 0x0001000D;
  /*ITM_TER*/
  (*(volatile unsigned int *)(0xE0000E00)) = 0xFFFFFFFF;
  /*ITM_TPR*/
  (*(volatile unsigned int *)(0xE0000E40)) = 0x00000008;
  /* ETMCR*/
  (*(volatile unsigned int *)(0xE0041000)) = 0x00002C10;
  /*ETMTECR1*/
  (*(volatile unsigned int *)(0xE0041024)) = 0x00000000;
  /* ETMTEEVR*/
  (*(volatile unsigned int *)(0xE0041020)) = 0x0000006F;
  /*ETMTRACEIDR*/
  (*(volatile unsigned int *)(0xE0041200)) = 0x00000002;
  /*ETMCR*/
  (*(volatile unsigned int *)(0xE0041000)) = 0x00002810;
  /* DWT_CTRL*/
  (*(volatile unsigned int *)(0xE0001000)) = 0x003F061F;
  /*DWT_CPICNT*/
  (*(volatile unsigned int *)(0xE0001008)) = 0x0;
  /* DWT_EXCCNT*/
  (*(volatile unsigned int *)(0xE000100C)) = 0x00000000;
  /*DWT_SLEEPCNT*/
  (*(volatile unsigned int *)(0xE0001010)) = 0x00000000;
  /* DWT_LSUCNT*/
  (*(volatile unsigned int *)(0xE0001014)) = 0x00000000;
  /*DWT_FOLDCNT*/
  (*(volatile unsigned int *)(0xE0001018)) = 0x00000000;
  /*DWT_CYCCNT*/
  (*(volatile unsigned int *)(0xE0001004)) = 0x000003FF;
  /* DWT_CTRL*/
  (*(volatile unsigned int *)(0xE0001000)) = 0x007F061F;
  /*ITM_TCR*/
  (*(volatile unsigned int *)(0xE0000E80)) = 0x0001000F;
  /*GPIO6 mode configuration as GPIO mode*/
  (*(volatile unsigned int *)(GPIO6_CONFIG_REG)) = 0x0;
  /*GPIO6 Pad selection*/
  (*(volatile unsigned int *)(PAD_SEL_REG)) |= 2;
}
