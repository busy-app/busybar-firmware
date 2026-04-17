/**************************************************************************//**
 * @file
 * @brief Device Manager Clock Definition
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

#include "em_device.h"
#include "sl_device_clock.h"

/***************************************************************************//**
 * @addtogroup device_clock Device Manager Clock
 * @{
 ******************************************************************************/

#if defined(_CMU_CLKEN1_ACMP0_SHIFT)
// Define ACMP0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_ACMP0_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_ACMP0_SHIFT;
#endif

#if defined(_CMU_CLKEN1_AGC_SHIFT)
// Define AGC peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_AGC_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_AGC_SHIFT;
#endif

#if defined(_CMU_CLKEN0_AMUXCP0_SHIFT)
// Define AMUXCP0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_AMUXCP0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_AMUXCP0_SHIFT;
#endif

#if defined(_CMU_CLKEN1_BUFC_SHIFT)
// Define BUFC peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_BUFC_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_BUFC_SHIFT;
#endif

#if defined(_CMU_CLKEN0_BURAM_SHIFT)
// Define BURAM peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_BURAM_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_BURAM_SHIFT;
#endif

#if defined(_CMU_CLKEN0_BURTC_SHIFT)
// Define BURTC peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_BURTC_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_BURTC_SHIFT;
#endif

#if defined(_CMU_CLKEN1_CRYPTOACC_SHIFT)
// Define CRYPTOACC peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_CRYPTOACC_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_CRYPTOACC_SHIFT;
#endif

#if defined(_CMU_CLKEN0_DCDC_SHIFT)
// Define DCDC peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_DCDC_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_DCDC_SHIFT;
#endif

#if defined(_CMU_CLKEN0_DPLL0_SHIFT)
// Define DPLL0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_DPLL0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_DPLL0_SHIFT;
#endif

#if defined(_CMU_CLKEN1_ETAMPDET_SHIFT)
// Define ETAMPDET peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_ETAMPDET_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_ETAMPDET_SHIFT;
#endif

#if defined(_CMU_CLKEN1_EUSART0_SHIFT)
// Define EUSART0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_EUSART0_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_EUSART0_SHIFT;
#endif

#if defined(_CMU_CLKEN1_FRC_SHIFT)
// Define FRC peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_FRC_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_FRC_SHIFT;
#endif

#if defined(_CMU_CLKEN0_FSRCO_SHIFT)
// Define FSRCO peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_FSRCO_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_FSRCO_SHIFT;
#endif

#if defined(_CMU_CLKEN0_GPCRC_SHIFT)
// Define GPCRC0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_GPCRC0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_GPCRC_SHIFT;
#endif

#if defined(_CMU_CLKEN0_GPIO_SHIFT)
// Define GPIO peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_GPIO_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_GPIO_SHIFT;
#endif

#if defined(_CMU_CLKEN0_HFRCO0_SHIFT)
// Define HFRCO0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_HFRCO0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_HFRCO0_SHIFT;
#endif

#if defined(_CMU_CLKEN0_HFXO0_SHIFT)
// Define HFXO0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_HFXO0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_HFXO0_SHIFT;
#endif

#if defined(_CMU_CLKEN0_I2C0_SHIFT)
// Define I2C0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_I2C0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_I2C0_SHIFT;
#endif

#if defined(_CMU_CLKEN0_I2C1_SHIFT)
// Define I2C1 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_I2C1_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_I2C1_SHIFT;
#endif

#if defined(_CMU_CLKEN0_IADC0_SHIFT)
// Define IADC0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_IADC0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_IADC0_SHIFT;
#endif

#if defined(_CMU_CLKEN1_ICACHE0_SHIFT)
// Define ICACHE0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_ICACHE0_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_ICACHE0_SHIFT;
#endif

#if defined(_CMU_CLKEN1_IFADCDEBUG_SHIFT)
// Define IFADCDEBUG peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_IFADCDEBUG_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_IFADCDEBUG_SHIFT;
#endif

#if defined(_CMU_CLKEN0_LDMA_SHIFT)
// Define LDMA0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_LDMA0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_LDMA_SHIFT;
#endif

#if defined(_CMU_CLKEN0_LDMAXBAR_SHIFT)
// Define LDMAXBAR0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_LDMAXBAR0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_LDMAXBAR_SHIFT;
#endif

#if defined(_CMU_CLKEN0_LETIMER0_SHIFT)
// Define LETIMER0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_LETIMER0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_LETIMER0_SHIFT;
#endif

#if defined(_CMU_CLKEN0_LFRCO_SHIFT)
// Define LFRCO peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_LFRCO_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_LFRCO_SHIFT;
#endif

#if defined(_CMU_CLKEN0_LFXO_SHIFT)
// Define LFXO peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_LFXO_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_LFXO_SHIFT;
#endif

#if defined(_CMU_CLKEN1_MODEM_SHIFT)
// Define MODEM peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_MODEM_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_MODEM_SHIFT;
#endif

#if defined(_CMU_CLKEN1_MSC_SHIFT)
// Define MSC peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_MSC_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_MSC_SHIFT;
#endif

#if defined(_CMU_CLKEN0_PDM_SHIFT)
// Define PDM peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_PDM_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_PDM_SHIFT;
#endif

#if defined(_CMU_CLKEN1_PRORTC_SHIFT)
// Define PRORTC peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_PRORTC_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_PRORTC_SHIFT;
#endif

#if defined(_CMU_CLKEN1_PROTIMER_SHIFT)
// Define PROTIMER peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_PROTIMER_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_PROTIMER_SHIFT;
#endif

#if defined(_CMU_CLKEN0_PRS_SHIFT)
// Define PRS peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_PRS_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_PRS_SHIFT;
#endif

#if defined(_CMU_CLKEN1_RAC_SHIFT)
// Define RAC peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_RAC_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_RAC_SHIFT;
#endif

#if defined(_CMU_CLKEN0_RADIOAES_SHIFT)
// Define RADIOAES peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_RADIOAES_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_RADIOAES_SHIFT;
#endif

#if defined(_CMU_CLKEN1_RDMAILBOX0_SHIFT)
// Define RDMAILBOX0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_RDMAILBOX0_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_RDMAILBOX0_SHIFT;
#endif

#if defined(_CMU_CLKEN1_RDMAILBOX1_SHIFT)
// Define RDMAILBOX1 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_RDMAILBOX1_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_RDMAILBOX1_SHIFT;
#endif

#if defined(_CMU_CLKEN1_RDSCRATCHPAD_SHIFT)
// Define RDSCRATCHPAD peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_RDSCRATCHPAD_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_RDSCRATCHPAD_SHIFT;
#endif

#if defined(_CMU_CLKEN1_RFCRC_SHIFT)
// Define RFCRC peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_RFCRC_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_RFCRC_SHIFT;
#endif

#if defined(_CMU_CLKEN1_RFSENSE_SHIFT)
// Define RFSENSE peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_RFSENSE_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_RFSENSE_SHIFT;
#endif

#if defined(_CMU_CLKEN0_RTCC_SHIFT)
// Define RTCC peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_RTCC_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_RTCC_SHIFT;
#endif

#if defined(_CMU_CLKEN1_SMU_SHIFT)
// Define SMU peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_SMU_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_SMU_SHIFT;
#endif

#if defined(_CMU_CLKEN1_SYNTH_SHIFT)
// Define SYNTH peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_SYNTH_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_SYNTH_SHIFT;
#endif

#if defined(_CMU_CLKEN0_SYSCFG_SHIFT)
// Define SYSCFG peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_SYSCFG_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_SYSCFG_SHIFT;
#endif

#if defined(_CMU_CLKEN0_TIMER0_SHIFT)
// Define TIMER0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_TIMER0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_TIMER0_SHIFT;
#endif

#if defined(_CMU_CLKEN0_TIMER1_SHIFT)
// Define TIMER1 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_TIMER1_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_TIMER1_SHIFT;
#endif

#if defined(_CMU_CLKEN0_TIMER2_SHIFT)
// Define TIMER2 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_TIMER2_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_TIMER2_SHIFT;
#endif

#if defined(_CMU_CLKEN0_TIMER3_SHIFT)
// Define TIMER3 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_TIMER3_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_TIMER3_SHIFT;
#endif

#if defined(_CMU_CLKEN1_TIMER4_SHIFT)
// Define TIMER4 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_TIMER4_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_TIMER4_SHIFT;
#endif

#if defined(_CMU_CLKEN0_ULFRCO_SHIFT)
// Define ULFRCO peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_ULFRCO_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_ULFRCO_SHIFT;
#endif

#if defined(_CMU_CLKEN0_USART0_SHIFT)
// Define USART0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_USART0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_USART0_SHIFT;
#endif

#if defined(_CMU_CLKEN0_USART1_SHIFT)
// Define USART1 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_USART1_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_USART1_SHIFT;
#endif

#if defined(_CMU_CLKEN0_WDOG0_SHIFT)
// Define WDOG0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_WDOG0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_WDOG0_SHIFT;
#endif

/** @} (end addtogroup device_clock) */
