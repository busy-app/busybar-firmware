/*******************************************************************************
 * @file  sl_si91x_etm.h
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef ETM_H_
#define ETM_H_

/*******************************************************************************
 ******************************   ETM Macros   *********************************
 ******************************************************************************/

#define PAD_SEL_REG       0x41300610 ///< Pad selection Register
#define REN_SEL_REG       0x460040D0 ///<  REN Selection register
#define GPIO52_CONFIG_REG 0x46130340 ///< GPIO 52 Config Register
#define GPIO53_CONFIG_REG 0x46130350 ///< GPIO 53 Config Register
#define GPIO54_CONFIG_REG 0x46130360 ///< GPIO 54 Config Register
#define GPIO55_CONFIG_REG 0x46130370 ///< GPIO 55 Config Register
#define GPIO56_CONFIG_REG 0x46130380 ///< GPIO 56 Config Register
#define GPIO57_CONFIG_REG 0x46130390 ///< GPIO 57 Config Register
#define GPIO6_CONFIG_REG  0x46130060 ///< GPIO 06 Config Register
#define GPIO6_TOGGLE_REG  0x46130064

#define PAD_SEL_VALUE_ITM 0x3F0000 ///< Pad selection for Trace Pin
#define REN_SEL_VALUE_ITM 31       ///< REN Selection register for Trace Pin

#define GPIO52_CONFIG_VALUE 0x19 ///< GPIO 52 Config Value
#define GPIO53_CONFIG_VALUE 0x19 ///< GPIO 53 Config Value
#define GPIO54_CONFIG_VALUE 0x19 ///< GPIO 54 Config Value
#define GPIO55_CONFIG_VALUE 0x19 ///< GPIO 55 Config Value
#define GPIO56_CONFIG_VALUE 0x19 ///< GPIO 56 Config Value
#define GPIO57_CONFIG_VALUE 0x19 ///< GPIO 57 Config Value

#define SOC_PLL_REF_FREQUENCY 40000000 ///< PLL input Reference clock 40MHZ

/* Note: Change this macro to required PLL frequency in hertz*/
#define PS4_SOC_FREQ          120000000 ///< PLL out clock 120MHz
#define PAD_SEL_MCU_CLK       7         ///< Pad selection MCU Clock
#define GPIO_MCU_CLK          12        ///< GPIO for MCU Clock
#define PAD_SEL_TRACE_CLK_OUT 17        ///< Pad Selection For Trace Clock Out
#define GPIO_TRACE_CLK_OUT    53        ///< GPIO for Trace Clock Out
#define PAD_SEL_TRACE_CLK_IN  8         ///< Pad Selection For Trace Clock In
#define GPIO_TRACE_CLK_IN     15        ///< GPIO for Trace Clock In

/*******************************************************************************
 **************************   ETM Function Declaration  ************************
 ******************************************************************************/

/***************************************************************************/ /**
 *  @fn          void sl_si91x_etm_int(void)
 *  @brief       This function initializes ETM.
 *  @param[in]   None
 ******************************************************************************/
void sl_si91x_etm_int(void);

#endif /* ETM_H_*/