/**************************************************************************//**
 * @file
 * @brief Device Manager Clock API Definition
 ******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories, Inc. www.silabs.com</b>
 ******************************************************************************
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
 *****************************************************************************/

#include "sl_device_clock.h"
#include "cmsis_compiler.h"

/***************************************************************************//**
 * @addtogroup device_clock Device Manager Clock
 * @{
 ******************************************************************************/

// External declaration for ACMP0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_ACMP0_VALUE = 0xFFFFFFFF;

// External declaration for ACMP1 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_ACMP1_VALUE = 0xFFFFFFFF;

// External declaration for ADC0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_ADC0_VALUE = 0xFFFFFFFF;

// External declaration for ADC0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_ADC1_VALUE = 0xFFFFFFFF;

// External declaration for AGC peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_AGC_VALUE = 0xFFFFFFFF;

// External declaration for AMUXCP0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_AMUXCP0_VALUE = 0xFFFFFFFF;

// External declaration for BUFC peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_BUFC_VALUE = 0xFFFFFFFF;

// External declaration for BURAM peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_BURAM_VALUE = 0xFFFFFFFF;

// External declaration for BURTC peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_BURTC_VALUE = 0xFFFFFFFF;

// External declaration for CRYPTOACC peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_CRYPTOACC_VALUE = 0xFFFFFFFF;

// External declaration for DCDC peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_DCDC_VALUE = 0xFFFFFFFF;

// External declaration for DEVINFO peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_DEVINFO_VALUE = 0xFFFFFFFF;

// External declaration for DMEM peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_DMEM_VALUE = 0xFFFFFFFF;

// External declaration for DPLL0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_DPLL0_VALUE = 0xFFFFFFFF;

// External declaration for ECAIFADC peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_ECAIFADC_VALUE = 0xFFFFFFFF;

// External declaration for ETAMPDET peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_ETAMPDET_VALUE = 0xFFFFFFFF;

// External declaration for EUART0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_EUART0_VALUE = 0xFFFFFFFF;

// External declaration for EUSART0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_EUSART0_VALUE = 0xFFFFFFFF;

// External declaration for EUSART1 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_EUSART1_VALUE = 0xFFFFFFFF;

// External declaration for EUSART2 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_EUSART2_VALUE = 0xFFFFFFFF;

// External declaration for EUSART3 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_EUSART3_VALUE = 0xFFFFFFFF;

// External declaration for EUSART4 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_EUSART4_VALUE = 0xFFFFFFFF;

// External declaration for FRC peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_FRC_VALUE = 0xFFFFFFFF;

// External declaration for FSRCO peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_FSRCO_VALUE = 0xFFFFFFFF;

// External declaration for GPCRC0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_GPCRC0_VALUE = 0xFFFFFFFF;

// External declaration for GPIO peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_GPIO_VALUE = 0xFFFFFFFF;

// External declaration for HFRCO0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_HFRCO0_VALUE = 0xFFFFFFFF;

// External declaration for HFRCOEM23 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_HFRCOEM23_VALUE = 0xFFFFFFFF;

// External declaration for HFXO0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_HFXO0_VALUE = 0xFFFFFFFF;

// External declaration for HOSTMAILBOX peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_HOSTMAILBOX_VALUE = 0xFFFFFFFF;

// External declaration for HOSTPORTAL peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_HOSTPORTAL_VALUE = 0xFFFFFFFF;

// External declaration for I2C0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_I2C0_VALUE = 0xFFFFFFFF;

// External declaration for I2C1 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_I2C1_VALUE = 0xFFFFFFFF;

// External declaration for I2C2 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_I2C2_VALUE = 0xFFFFFFFF;

// External declaration for I2C3 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_I2C3_VALUE = 0xFFFFFFFF;

// External declaration for IADC0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_IADC0_VALUE = 0xFFFFFFFF;

// External declaration for ICACHE0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_ICACHE0_VALUE = 0xFFFFFFFF;

// External declaration for IFADCDEBUG peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_IFADCDEBUG_VALUE = 0xFFFFFFFF;

// External declaration for KEYSCAN peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_KEYSCAN_VALUE = 0xFFFFFFFF;

// External declaration for KSU peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_KSU_VALUE = 0xFFFFFFFF;

// External declaration for L2ICACHE0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_L2ICACHE0_VALUE = 0xFFFFFFFF;

// External declaration for LCD peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_LCD_VALUE = 0xFFFFFFFF;

// External declaration for LDMA0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_LDMA0_VALUE = 0xFFFFFFFF;

// External declaration for LDMAXBAR0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_LDMAXBAR0_VALUE = 0xFFFFFFFF;

// External declaration for LEDDRV0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_LEDDRV0_VALUE = 0xFFFFFFFF;

// External declaration for LEDDRV0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_LEDSINK0_VALUE = 0xFFFFFFFF;

// External declaration for LESENSE peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_LESENSE_VALUE = 0xFFFFFFFF;

// External declaration for LETIMER0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_LETIMER0_VALUE = 0xFFFFFFFF;

// External declaration for LFRCO peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_LFRCO_VALUE = 0xFFFFFFFF;

// External declaration for LFXO peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_LFXO_VALUE = 0xFFFFFFFF;

// External declaration for LPWAES peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_LPWAES_VALUE = 0xFFFFFFFF;

// External declaration for LPW0PORTAL peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_LPW0PORTAL_VALUE = 0xFFFFFFFF;

// External declaration for MODEM peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_MODEM_VALUE = 0xFFFFFFFF;

// External declaration for MSC peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_MSC_VALUE = 0xFFFFFFFF;

// External declaration for MVP peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_MVP_VALUE = 0xFFFFFFFF;

// External declaration for PCNT0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_PCNT0_VALUE = 0xFFFFFFFF;

// External declaration for PDM peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_PDM_VALUE = 0xFFFFFFFF;

// External declaration for PIXELRZ0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_PIXELRZ0_VALUE = 0xFFFFFFFF;

// External declaration for PIXELRZ1 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_PIXELRZ1_VALUE = 0xFFFFFFFF;

// External declaration for PRORTC peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_PRORTC_VALUE = 0xFFFFFFFF;

// External declaration for PROTIMER peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_PROTIMER_VALUE = 0xFFFFFFFF;

// External declaration for PRS peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_PRS_VALUE = 0xFFFFFFFF;

// External declaration for RAC peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_RAC_VALUE = 0xFFFFFFFF;

// External declaration for RADIOAES peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_RADIOAES_VALUE = 0xFFFFFFFF;

// External declaration for RDMAILBOX0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_RDMAILBOX0_VALUE = 0xFFFFFFFF;

// External declaration for RDMAILBOX1 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_RDMAILBOX1_VALUE = 0xFFFFFFFF;

// External declaration for RDSCRATCHPAD peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_RDSCRATCHPAD_VALUE = 0xFFFFFFFF;

// External declaration for RFCRC peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_RFCRC_VALUE = 0xFFFFFFFF;

// External declaration for RFECA0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_RFECA0_VALUE = 0xFFFFFFFF;

// External declaration for RFECA1 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_RFECA1_VALUE = 0xFFFFFFFF;

// External declaration for RFFPLL0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_RFFPLL0_VALUE = 0xFFFFFFFF;

// External declaration for RFMAILBOX peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_RFMAILBOX_VALUE = 0xFFFFFFFF;

// External declaration for RFSCRATCHPAD peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_RFSCRATCHPAD_VALUE = 0xFFFFFFFF;

// External declaration for RFSENSE peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_RFSENSE_VALUE = 0xFFFFFFFF;

// External declaration for RPA peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_RPA_VALUE = 0xFFFFFFFF;

// External declaration for RTCC peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_RTCC_VALUE = 0xFFFFFFFF;

// External declaration for SCRATCHPAD peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_SCRATCHPAD_VALUE = 0xFFFFFFFF;

// External declaration for SEMAILBOX peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_SEMAILBOX_VALUE = 0xFFFFFFFF;

// External declaration for SEMAPHORE0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_SEMAPHORE0_VALUE = 0xFFFFFFFF;

// External declaration for SEMAPHORE1 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_SEMAPHORE1_VALUE = 0xFFFFFFFF;

// External declaration for SEPORTAL peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_SEPORTAL_VALUE = 0xFFFFFFFF;

// External declaration for SMU peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_SMU_VALUE = 0xFFFFFFFF;

// External declaration for SOCPLL0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_SOCPLL0_VALUE = 0xFFFFFFFF;

// External declaration for SYMCRYPTO peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_SYMCRYPTO_VALUE = 0xFFFFFFFF;

// External declaration for SYNTH peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_SYNTH_VALUE = 0xFFFFFFFF;

// External declaration for SYSCFG peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_SYSCFG_VALUE = 0xFFFFFFFF;

// External declaration for SYSRTC0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_SYSRTC0_VALUE = 0xFFFFFFFF;

// External declaration for TIMER0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_TIMER0_VALUE = 0xFFFFFFFF;

// External declaration for TIMER1 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_TIMER1_VALUE = 0xFFFFFFFF;

// External declaration for TIMER2 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_TIMER2_VALUE = 0xFFFFFFFF;

// External declaration for TIMER3 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_TIMER3_VALUE = 0xFFFFFFFF;

// External declaration for TIMER4 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_TIMER4_VALUE = 0xFFFFFFFF;

// External declaration for TIMER5 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_TIMER5_VALUE = 0xFFFFFFFF;

// External declaration for TIMER6 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_TIMER6_VALUE = 0xFFFFFFFF;

// External declaration for TIMER7 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_TIMER7_VALUE = 0xFFFFFFFF;

// External declaration for TIMER8 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_TIMER8_VALUE = 0xFFFFFFFF;

// External declaration for TIMER9 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_TIMER9_VALUE = 0xFFFFFFFF;

// External declaration for ULFRCO peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_ULFRCO_VALUE = 0xFFFFFFFF;

// External declaration for USART0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_USART0_VALUE = 0xFFFFFFFF;

// External declaration for USART1 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_USART1_VALUE = 0xFFFFFFFF;

// External declaration for USART2 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_USART2_VALUE = 0xFFFFFFFF;

// External declaration for USB peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_USB_VALUE = 0xFFFFFFFF;

// External declaration for VDAC0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_VDAC0_VALUE = 0xFFFFFFFF;

// External declaration for VDAC1 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_VDAC1_VALUE = 0xFFFFFFFF;

// External declaration for WDOG0 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_WDOG0_VALUE = 0xFFFFFFFF;

// External declaration for WDOG1 peripheral bus clock value.
__WEAK const uint32_t SL_BUS_CLOCK_WDOG1_VALUE = 0xFFFFFFFF;

/** @} (end addtogroup device_clock) */
