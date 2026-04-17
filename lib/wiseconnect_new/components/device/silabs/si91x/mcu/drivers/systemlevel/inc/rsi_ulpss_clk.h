/*******************************************************************************
* @file  rsi_ulpss_clk.h
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

#ifndef __RSI_ULPSS_CLK_H__
#define __RSI_ULPSS_CLK_H__

#include "rsi_ccp_common.h"
#include "rsi_error.h"
#include "rsi_pll.h"
#include "rsi_ulpss_clk.h"
#include "rsi_power_save.h"

/*requied delays for turn on the clocks in micro seconds*/
#define MCU_ULP_40MHZ_CLK_EN_TURN_ON_DELAY          10   /*  delay to enable the ULP 40MHZ CLK*/
#define MCU_ULP_DOUBLER_CLK_EN_TURN_ON_DELAY        10   /*  delay to enable the ULP DOUBLER CLK*/
#define MCU_ULP_20MHZ_RING_OSC_CLK_EN_TURN_ON_DELAY 10   /*  delay to enable the ULP 20MHZ_RING_OSC CLK*/
#define MCU_ULP_MHZ_RC_CLK_EN_TURN_ON_DELAY         2    /*  delay to enable the ULP MHZ_RC CLK*/
#define MCU_ULP_32KHZ_XTAL_CLK_EN_TURN_ON_DELAY_1   500  /*  delay to enable the ULP 32KHZ_XTAL CLK*/
#define MCU_ULP_32KHZ_XTAL_CLK_EN_TURN_ON_DELAY_2   1500 /*  delay to enable the ULP 32KHZ_XTAL CLK*/
#define MCU_ULP_32KHZ_RO_CLK_EN_TURN_ON_DELAY       250  /*  delay to enable the ULP 32KHZ_RO CLK*/
#define MCU_ULP_32KHZ_RC_CLK_EN_TURN_ON_DELAY       150  /*  delay to enable the ULP 32KHZ_RC CLK*/

/*Clock enable Bits */
#define TOUCH_SENSOR_PCLK_ENABLE        BIT(31) /*  Enables TOUCH_SENSOR_PCLK_ENABLE */
#define FIM_AHB_CLK_ENABLE              BIT(30) /*  Enables FIM_AHB_CLK_ENABLE */
#define ULPSS_TASS_QUASI_SYNC           BIT(27) /*  Enables ULPSS_TASS_QUASI_SYNC */
#define ULPSS_M4SS_SLV_SEL              BIT(26) /*  Enables ULPSS_M4SS_SLV_SEL */
#define AUX_SOC_EXT_TRIG_2_SEL          BIT(25) /*  Enables TOUCH_SENSOR_PCLK_ENABLE */
#define AUX_SOC_EXT_TRIG_1_SEL          BIT(24) /*  Enables AUX_SOC_EXT_TRIG_1_SEL */
#define AUX_ULP_EXT_TRIG_2_SEL          BIT(23) /*  Enables AUX_ULP_EXT_TRIG_2_SEL */
#define AUX_ULP_EXT_TRIG_1_SEL          BIT(22) /*  Enables AUX_ULP_EXT_TRIG_1_SEL */
#define TIMER_PCLK_EN                   BIT(21) /*  Enables TIMER_PCLK_EN */
#define EGPIO_PCLK_EN                   BIT(20) /*  Enables EGPIO_PCLK_EN */
#define EGPIO_PCLK_DYN_CTRL_DISABLE_ULP BIT(19) /*  Enables EGPIO_PCLK_DYN_CTRL_DISABLE_ULP */
#define CLK_ENABLE_ULP_MEMORIES         BIT(18) /*  Enables CLK_ENABLE_ULP_MEMORIES */
#define VAD_CLK_EN                      BIT(17) /*  Enables VAD_CLK_EN */
#define FIM_CLK_EN                      BIT(16) /*  Enables FIM_CLK_EN */
#define REG_ACCESS_SPI_CLK_EN           BIT(15) /*  Enables REG_ACCESS_SPI_CLK_EN */
#define EGPIO_CLK_EN                    BIT(14) /*  Enables EGPIO_CLK_EN */
#define CLK_ENABLE_TIMER                BIT(13) /*  Enables CLK_ENABLE_TIMER */
#define VAD_PCLK_ENABLE                 BIT(12) /*  Enables VAD_PCLK_ENABLE */
#define FIM_PCLK_ENABLE                 BIT(11) /*  Enables FIM_PCLK_ENABLE */
#define SCLK_ENABLE_UART                BIT(10) /*  Enables SCLK_ENABLE_UART */
#define PCLK_ENABLE_UART                BIT(9)  /*  Enables PCLK_ENABLE_UART */
#define SCLK_ENABLE_SSI_MASTER          BIT(8)  /*  Enables SCLK_ENABLE_SSI_MASTER */
#define PCLK_ENABLE_SSI_MASTER          BIT(7)  /*  Enables PCLK_ENABLE_SSI_MASTER */
#define CLK_ENABLE_I2S                  BIT(6)  /*  Enables CLK_ENABLE_I2S */
#define PCLK_ENABLE_I2C                 BIT(5)  /*  Enables PCLK_ENABLE_I2C */
#define IR_PCLK_EN                      BIT(4)  /*  Enables IR_PCLK_EN */
#define PCM_FSYNC_START                 BIT(1)  /*  Enables PCM_FSYNC_START */
#define PCM_ENABLE                      BIT(0)  /*  Enables PCM_ENABLE */

#define I2C_PCLK_DYN_CTRL_DISABLE           BIT(0) /*  Enables PCM_ENABLE */
#define I2S_CLK_DYN_CTRL_DISABLE            BIT(1)
#define ULP_SSI_MST_PCLK_DYN_CTRL_DISABLE   BIT(2)
#define ULP_SSI_MST_SCLK_DYN_CTRL_DISABLE   BIT(3)
#define UART_CLK_DYN_CTRL_DISABLE           BIT(4)
#define UART_SCLK_DYN_CTRL_DISABLE          BIT(5)
#define ULP_TIMER_PCLK_DYN_CTRL_DISABLE     BIT(6)
#define ULP_TIMER_SCLK_DYN_CTRL_DISABLE     BIT(7)
#define REG_ACCESS_SPI_CLK_DYN_CTRL_DISABLE BIT(8)
#define FIM_CLK_DYN_CTRL_DISABLE            BIT(9)
#define VAD_CLK_DYN_CTRL_DISABLE            BIT(10)
#define AUX_PCLK_EN                         BIT(11)
#define AUX_CLK_EN                          BIT(12)
#define AUX_MEM_EN                          BIT(13)
#define AUX_PCLK_DYN_CTRL_DISABLE           BIT(14)
#define AUX_CLK_DYN_CTRL_DISABLE            BIT(15)
#define AUX_CLK_MEM_DYN_CTRL_DISABLE        BIT(16)
#define UDMA_CLK_ENABLE                     BIT(17)
#define IR_CLK_ENABLE                       BIT(18)
#define IR_CLK_DYN_CTRL_DISABLE             BIT(19)

/*ULP PROCESSOR CLOCK */
#define ULP_PROC_MAX_SEL             7   /* Maximum Seletion value for ulp processor clock source*/
#define ULP_PROC_MIN_SEL             0   /* Minimum Seletion value for ulp processor clock source*/
#define ULP_PROC_MAX_DIVISOIN_FACTOR 255 /* Maximum division factor value for ulp processor clock*/
#define ULP_PROC_MIN_DIVISOIN_FACTOR 0   /* Minimum division factor value for ulp processor clock*/

/*ULP SSI CLOCK*/
#define ULP_SSI_MAX_SEL             6   /* Maximum Seletion value for ulp SSI clock source*/
#define ULP_SSI_MIN_SEL             0   /* Minimum Seletion value for ulp SSI clock source*/
#define ULP_SSI_MAX_DIVISION_FACTOR 127 /* Maximum division factor value for ulp SSI clock*/
#define ULP_SSI_MIN_DIVISION_FACTOR 0   /* Minimum division factor value for ulp SSI clock*/

/*ULP I2S CLOCK*/
#define ULP_I2S_MAX_SEL             8   /* Maximum Seletion value for ulp I2S clock source*/
#define ULP_I2S_MIN_SEL             0   /* Minimum Seletion value for ulp I2S clock source*/
#define ULP_I2S_MAX_DIVISION_FACTOR 255 /* Maximum division factor value for ulp SSI clock*/
#define ULP_I2S_MIN_DIVISION_FACTOR 0   /* Minimum division factor value for ulp SSI clock*/

/*ULP UART CLOCK*/
#define ULP_UART_MAX_SEL             7 /* Maximum Seletion value for ulp Uart clock source*/
#define ULP_UART_MIN_SEL             0 /* Minimum Seletion value for ulp Uart clock source*/
#define ULP_UART_MAX_DIVISION_FACTOR 7 /* Maximum division factor value for ulp Uart clock*/
#define ULP_UART_MIN_DIVISION_FACTOR 0 /* Minimum division factor value for ulp Uart clock*/

/*ULP AUX CLOCK*/
#define ULP_AUX_MAX_SEL             8   /* Maximum Seletion value for ulp Aux clock source*/
#define ULP_AUX_MIN_SEL             0   /* Minimum Seletion value for ulp Aux clock source*/
#define ULP_AUX_MAX_DIVISION_FACTOR 255 /* Maximum division factor value for ulp Aux clock*/
#define ULP_AUX_MIN_DIVISION_FACTOR 0   /* Minimum division factor value for ulp Aux clock*/

/*ULP TIMER CLOCK*/
#define ULP_TIMER_MAX_SEL 6 /* Maximum Seletion value for ulp Timer clock source*/
#define ULP_TIMER_MIN_SEL 0 /* Minimum Seletion value for ulp Tiemer clock source*/

/*ULP VAD CLOCK*/
#define ULP_VAD_MAX_SEL             8   /* Maximum Seletion value for ulp Vad clock source*/
#define ULP_VAD_MIN_SEL             0   /* Minimum Seletion value for ulp Vad clock source*/
#define ULP_VAD_FCLK_MAX_SEL        8   /* Maximum Seletion value for ulp Vad fclock source*/
#define ULP_VAD_FCLK_MIN_SEL        0   /* Minimum Seletion value for ulp Vad fclock source*/
#define ULP_VAD_MAX_DIVISION_FACTOR 255 /* Maximum division factor value for ulp Vad clock*/
#define ULP_VAD_MIN_DIVISION_FACTOR 0   /* Minimum division factor value for ulp Vad clock*/

/*ULP TOUCH CLOCK*/
#define ULP_TOUCH_MAX_SEL             6   /* Maximum Seletion value for ulp Touch clock source*/
#define ULP_TOUCH_MIN_SEL             0   /* Minimum Seletion value for ulp Touch clock source*/
#define ULP_TOUCH_MAX_DIVISION_FACTOR 255 /* Maximum division factor value for ulp Touch clock*/
#define ULP_TOUCH_MIN_DIVISION_FACTOR 0   /* Minimum division factor value for ulp Touch clock*/

/*ULP SLEEP SENSOR */
#define ULP_SLP_SENSOR_MAX_DIVISION_FACTOR 255 /* Maximum division factor value for ulp Sleep sensor clock*/
#define ULP_SLP_SENSOR_MIN_DIVISION_FACTOR 0   /* Minimum division factor value for ulp Sleep sensor clock*/
/**
 *@brief ulpss Reference Input clock source selection
 **/
typedef enum ULPSS_REF_CLK_SEL {
  ULPSS_REF_BYP_CLK           = 1, /*!< REF_BYP_CLK selection*/
  ULPSS_ULP_MHZ_RC_CLK        = 2, /*!< ULP_MHZ_RC_CLK selection*/
  ULPSS_40MHZ_CLK             = 3, /*!< EXT_40MHZ_CLK selection*/
  ULPSS_MEMS_REF_CLK          = 4, /*!< MEMS_REF_CLK selection*/
  ULPSS_ULP_20MHZ_RINGOSC_CLK = 5, /*!< ULP_20MHZ_RINGOSC_CLK selection*/
  ULPSS_ULP_DOUBLER_CLK       = 6, /*!< ULP_DOUBLER_CLK selection*/
} ULPSS_REF_CLK_SEL_T;
/**
*@brief Different possible ref_clk sources for Ulp_proc_clk
**/
typedef enum ULP_PROC_CLK_SELECT {
  ULP_PROC_REF_CLK,            /*!< ULP_REF_CLK selection*/
  ULP_PROC_ULP_32KHZ_RO_CLK,   /*!< 32 KHz RO clock is not supported */
  ULP_PROC_ULP_32KHZ_RC_CLK,   /*!< ULP_32KHZ_RC_CLK selection*/
  ULP_PROC_ULP_32KHZ_XTAL_CLK, /*!< ULP_32KHZ_XTAL_CLK selection*/
  ULP_PROC_ULP_MHZ_RC_CLK,     /*!< ULP_MHZ_RC_CLK selection*/
  ULP_PROC_ULP_20MHZ_RO_CLK,   /*!< 20 MHz RO clock is not supported */
  ULP_PROC_SOC_CLK,            /*!< SOC_CLK selection*/
  ULP_PROC_ULP_DOUBLER_CLK     /*!< ULP_DOUBLER_CLK selection*/
} ULP_PROC_CLK_SELECT_T;
/**
*@brief Different possible input clk sources for Ulp_SSI_clk
**/
typedef enum ULP_SSI_CLK_SELECT {

  ULP_SSI_REF_CLK,            /*!< ULP_REF_CLK selection*/
  ULP_SSI_ULP_32KHZ_RO_CLK,   /*!< 32 KHz RO clock is not supported */
  ULP_SSI_ULP_32KHZ_RC_CLK,   /*!< ULP_32KHZ_RC_CLK selection*/
  ULP_SSI_ULP_32KHZ_XTAL_CLK, /*!< ULP_32KHZ_XTAL_CLK selection*/
  ULP_SSI_ULP_MHZ_RC_CLK,     /*!< ULP_MHZ_RC_CLK selection*/
  ULP_SSI_ULP_20MHZ_RO_CLK,   /*!< 20 MHz RO clock is not supported */
  ULP_SSI_SOC_CLK,            /*!< SOC_CLK selection*/
} ULP_SSI_CLK_SELECT_T;
/**
*@brief Different possible input clk sources for Ulp_I2S_clk
**/
typedef enum ULP_I2S_CLK_SELECT {

  ULP_I2S_REF_CLK,            /*!< ULP_REF_CLK selection*/
  ULP_I2S_ULP_32KHZ_RO_CLK,   /*!< 32 KHz RO clock is not supported */
  ULP_I2S_ULP_32KHZ_RC_CLK,   /*!< ULP_32KHZ_RC_CLK selection*/
  ULP_I2S_ULP_32KHZ_XTAL_CLK, /*!< ULP_32KHZ_XTAL_CLK selection*/
  ULP_I2S_ULP_MHZ_RC_CLK,     /*!< ULP_MHZ_RC_CLK selection*/
  ULP_I2S_ULP_20MHZ_RO_CLK,   /*!< 20 MHz RO clock is not supported */
  ULP_I2S_SOC_CLK,            /*!< SOC_CLK selection*/
  ULP_I2S_ULP_DOUBLER_CLK,    /*!< ULP_DOUBLER_CLK selection*/
  ULP_I2S_PLL_CLK             /*!< I2s_PLL_CLK selection*/

} ULP_I2S_CLK_SELECT_T;
/**
*@brief Different possible input clk sources for Ulp_Uart_clk
**/
typedef enum ULP_UART_CLK_SELECT {

  ULP_UART_REF_CLK,            /*!< ULP_REF_CLK selection*/
  ULP_UART_ULP_32KHZ_RO_CLK,   /*!< 32 KHz RO clock is not supported */
  ULP_UART_ULP_32KHZ_RC_CLK,   /*!< ULP_32KHZ_RC_CLK selection*/
  ULP_UART_ULP_32KHZ_XTAL_CLK, /*!< ULP_32KHZ_XTAL_CLK selection*/
  ULP_UART_ULP_MHZ_RC_CLK,     /*!< ULP_MHZ_RC_CLK selection*/
  ULP_UART_ULP_20MHZ_RO_CLK,   /*!< 20 MHz RO clock is not supported */
  ULP_UART_SOC_CLK,            /*!< SOC_CLK selection*/
  ULP_UART_ULP_DOUBLER_CLK,    /*!< ULP_DOUBLER_CLK selection*/
} ULP_UART_CLK_SELECT_T;
/**
*@brief Different possible input clk sources for Ulp_Timer_clk
**/
typedef enum ULP_TIMER_CLK_SELECT {

  ULP_TIMER_REF_CLK,        /*!< ULP_REF_CLK selection*/
  ULP_TIMER_32KHZ_RO_CLK,   /*!< 32 KHz RO clock is not supported */
  ULP_TIMER_32KHZ_RC_CLK,   /*!< ULP_32KHZ_RC_CLK selection*/
  ULP_TIMER_32KHZ_XTAL_CLK, /*!< ULP_32KHZ_XTAL_CLK selection*/
  ULP_TIMER_MHZ_RC_CLK,     /*!< ULP_MHZ_RC_CLK selection*/
  ULP_TIMER_20MHZ_RO_CLK,   /*!< 20 MHz RO clock is not supported */
  ULP_TIMER_ULP_SOC_CLK,    /*!< SOC_CLK selection*/
} ULP_TIMER_CLK_SELECT_T;
/**
*@brief Different possible input clk sources for Ulp_AUX_clk
**/
typedef enum ULP_AUX_CLK_SELECT {

  ULP_AUX_REF_CLK,         /*!< ULP_REF_CLK selection*/
  ULP_AUX_32KHZ_RO_CLK,    /*!< 32 KHz RO clock is not supported */
  ULP_AUX_32KHZ_RC_CLK,    /*!< ULP_32KHZ_RC_CLK selection*/
  ULP_AUX_32KHZ_XTAL_CLK,  /*!< ULP_32KHZ_XTAL_CLK selection*/
  ULP_AUX_MHZ_RC_CLK,      /*!< ULP_MHZ_RC_CLK selection*/
  ULP_AUX_20MHZ_RO_CLK,    /*!< 20 MHz RO clock is not supported */
  ULP_AUX_ULP_SOC_CLK,     /*!< SOC_CLK selection*/
  ULP_AUX_ULP_DOUBLER_CLK, /*!< ULP_DOUBLER_CLK selection*/
  ULP_AUX_I2S_PLL_CLK      /*!< I2s_PLL_CLK selection*/
} ULP_AUX_CLK_SELECT_T;
/**
*@brief Different possible input clk sources for Ulp_Vad_clk
**/
typedef enum ULP_VAD_CLK_SELECT {

  ULP_VAD_32KHZ_RO_CLK,   /*!< 32 KHz RO clock is not supported */
  ULP_VAD_32KHZ_RC_CLK,   /*!< ULP_32KHZ_RC_CLK selection*/
  ULP_VAD_32KHZ_XTAL_CLK, /*!< ULP_32KHZ_XTAL_CLK selection*/
} ULP_VAD_CLK_SELECT_T;
/**
*@brief Different possible input clk sources for Ulp_Vad_fclk
**/
typedef enum ULP_VAD_FCLK_SELECT {

  ULP_VAD_ULP_PROCESSOR_CLK, /*!< ULP_PROCESSOR_CLK selection*/
  ULP_VAD_REF_CLK,           /*!< ULP_REF_CLK selection*/
  ULP_VAD_MHZ_RC_CLK,        /*!< ULP_MHZ_RC_CLK selection*/
  ULP_VAD_20MHZ_RO_CLK,      /*!< 20 MHz RO clock is not supported */
  ULP_VAD_ULP_SOC_CLK,       /*!< SOC_CLK selection*/
} ULP_VAD_FCLK_SELECT_T;
/**
*@brief Different possible input clk sources for Ulp_Touch_clk
**/
typedef enum ULP_TOUCH_CLK_SELECT {

  ULP_TOUCH_REF_CLK,        /*!< ULP_REF_CLK selection*/
  ULP_TOUCH_32KHZ_RO_CLK,   /*!< 32 KHz RO clock is not supported */
  ULP_TOUCH_32KHZ_RC_CLK,   /*!< ULP_32KHZ_RC_CLK selection*/
  ULP_TOUCH_32KHZ_XTAL_CLK, /*!< ULP_32KHZ_XTAL_CLK selection*/
  ULP_TOUCH_MHZ_RC_CLK,     /*!< ULP_MHZ_RC_CLK selection*/
  ULP_TOUCH_20MHZ_RO_CLK,   /*!< 20 MHz RO clock is not supported */
  ULP_TOUCH_ULP_SOC_CLK     /*!< SOC_CLK selection*/

} ULP_TOUCH_CLK_SELECT_T;

/**
*@brief list of peripherals, particular ulp clock that to be disabled 
**/
typedef enum ULPPERIPHERALS_CLK {
  ULP_I2C_CLK,   /*!< Enables or Disables ULP_I2C Peripheral clock when it is passed */
  ULP_EGPIO_CLK, /*!< Enables or Disables Ulp_Egpio Peripheral clock when it is passed */
  ULP_AUX_CLK,   /*!< Enables or Disables Ulp_Aux Peripheral clock when it is passed */
  ULP_FIM_CLK,   /*!< Enables or Disables Ulp_Fim Peripheral clock when it is passed */
  ULP_VAD_CLK,   /*!< Enables or Disables Ulp_Vad Peripheral clock when it is passed */
  ULP_TIMER_CLK, /*!< Enables or Disables Ulp_Timer Peripheral clock when it is passed */
  ULP_UDMA_CLK,  /*!< Enables or Disables Ulp_Udma Peripheral clock when it is passed */
  ULP_TOUCH_CLK, /*!< Enables or Disables Ulp_Touch Peripheral clock when it is passed */
  ULP_UART_CLK,  /*!< Enables or Disables Ulp_Uart Peripheral clock when it is passed */
  ULP_SSI_CLK,   /*!< Enables or Disables Ulp_SSI Peripheral clock when it is passed */
  ULP_I2S_CLK,   /*!< Enables or Disables Ulp_I2S Peripheral clock when it is passed */
} ULPPERIPHERALS_CLK_T;

rsi_error_t ulpss_ulp_proc_clk_config(ULPCLK_Type *pULPCLK,
                                      ULP_PROC_CLK_SELECT_T clkSource,
                                      uint16_t divFactor,
                                      cdDelay delayFn);
rsi_error_t ulpss_ref_clk_config(ULPSS_REF_CLK_SEL_T clkSource);

rsi_error_t ulpss_clock_config(M4CLK_Type *pCLK, boolean_t clkEnable, uint16_t divFactor, boolean_t oddDivFactor);

rsi_error_t ulpss_ulp_proc_clk_config(ULPCLK_Type *pULPCLK,
                                      ULP_PROC_CLK_SELECT_T clkSource,
                                      uint16_t divFactor,
                                      cdDelay delayFn);

rsi_error_t ulpss_ulp_peri_clk_enable(ULPCLK_Type *pULPCLK, uint32_t u32Flags);

rsi_error_t ulpss_ulp_peri_clk_disable(ULPCLK_Type *pULPCLK, uint32_t u32Flags);

rsi_error_t ulpss_ulp_dyn_clk_enable(ULPCLK_Type *pULPCLK, uint32_t u32Flags);

rsi_error_t ulpss_ulp_dyn_clk_disable(ULPCLK_Type *pULPCLK, uint32_t u32Flags);

rsi_error_t ulpss_ulp_ssi_clk_config(ULPCLK_Type *pULPCLK,
                                     CLK_ENABLE_T clkType,
                                     ULP_SSI_CLK_SELECT_T clkSource,
                                     uint16_t divFactor);

rsi_error_t ulpss_ulp_i2s_clk_config(ULPCLK_Type *pULPCLK, ULP_I2S_CLK_SELECT_T clkSource, uint16_t divFactor);

rsi_error_t ulpss_ulp_uar_clk_config(ULPCLK_Type *pULPCLK,
                                     CLK_ENABLE_T clkType,
                                     boolean_t bFrClkSel,
                                     ULP_UART_CLK_SELECT_T clkSource,
                                     uint16_t divFactor);

rsi_error_t ulpss_time_clk_config(ULPCLK_Type *pULPCLK,
                                  CLK_ENABLE_T clkType,
                                  boolean_t bTmrSync,
                                  ULP_TIMER_CLK_SELECT_T clkSource,
                                  uint8_t skipSwitchTime);

rsi_error_t ulpss_aux_clk_config(ULPCLK_Type *pULPCLK, CLK_ENABLE_T clkType, ULP_AUX_CLK_SELECT_T clkSource);

rsi_error_t ulpss_vad_clk_config(ULPCLK_Type *pULPCLK,
                                 ULP_VAD_CLK_SELECT_T clkSource,
                                 ULP_VAD_FCLK_SELECT_T FclkSource,
                                 uint16_t divFactor);

rsi_error_t ulpss_touch_clk_config(ULPCLK_Type *pULPCLK, ULP_TOUCH_CLK_SELECT_T clkSource, uint16_t divFactor);

rsi_error_t ulpss_slp_sensor_clk_config(ULPCLK_Type *pULPCLK, boolean_t clkEnable, uint32_t divFactor);

rsi_error_t ulpss_peripheral_enable(ULPCLK_Type *pULPCLK, ULPPERIPHERALS_CLK_T module, CLK_ENABLE_T clkType);

rsi_error_t ulpss_peripheral_disable(ULPCLK_Type *pULPCLK, ULPPERIPHERALS_CLK_T module);

rsi_error_t ulpss_time_clk_disable(ULPCLK_Type *pULPCLK);

#endif /*__RSI_ULPSS_CLK_H__*/
