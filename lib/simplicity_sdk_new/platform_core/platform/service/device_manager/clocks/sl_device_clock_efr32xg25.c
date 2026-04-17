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

#if defined(_CMU_CLKEN1_ACMP1_SHIFT)
// Define ACMP1 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_ACMP1_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_ACMP1_SHIFT;
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

#if defined(_CMU_CLKEN0_DCDC_SHIFT)
// Define DCDC peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_DCDC_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_DCDC_SHIFT;
#endif

#if defined(_CMU_CLKEN1_DMEM_SHIFT)
// Define DMEM peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_DMEM_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_DMEM_SHIFT;
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

#if defined(_CMU_CLKEN1_EUSART1_SHIFT)
// Define EUSART1 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_EUSART1_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_EUSART1_SHIFT;
#endif

#if defined(_CMU_CLKEN1_EUSART2_SHIFT)
// Define EUSART2 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_EUSART2_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_EUSART2_SHIFT;
#endif

#if defined(_CMU_CLKEN1_EUSART3_SHIFT)
// Define EUSART3 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_EUSART3_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_EUSART3_SHIFT;
#endif

#if defined(_CMU_CLKEN1_EUSART4_SHIFT)
// Define EUSART4 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_EUSART4_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_EUSART4_SHIFT;
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

#if defined(_CMU_CLKEN0_HFRCOEM23_SHIFT)
// Define HFRCOEM23 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_HFRCOEM23_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_HFRCOEM23_SHIFT;
#endif

#if defined(_CMU_CLKEN0_HFXO0_SHIFT)
// Define HFXO0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_HFXO0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_HFXO0_SHIFT;
#endif

#if defined(_CMU_CLKEN1_HOSTMAILBOX_SHIFT)
// Define HOSTMAILBOX peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_HOSTMAILBOX_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_HOSTMAILBOX_SHIFT;
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

#if defined(_CMU_CLKEN0_LDMA_SHIFT)
// Define LDMA0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_LDMA0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_LDMA_SHIFT;
#endif

#if defined(_CMU_CLKEN0_LDMAXBAR_SHIFT)
// Define LDMAXBAR0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_LDMAXBAR0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_LDMAXBAR_SHIFT;
#endif

#if defined(_CMU_CLKEN0_LESENSE_SHIFT)
// Define LESENSE peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_LESENSE_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_LESENSE_SHIFT;
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

#if defined(_CMU_CLKEN1_MSC_SHIFT)
// Define MSC peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_MSC_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_MSC_SHIFT;
#endif

#if defined(_CMU_CLKEN1_PCNT0_SHIFT)
// Define PCNT0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_PCNT0_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_PCNT0_SHIFT;
#endif

#if defined(_CMU_CLKEN0_PRS_SHIFT)
// Define PRS peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_PRS_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_PRS_SHIFT;
#endif

#if defined(_CMU_CLKEN0_RADIOAES_SHIFT)
// Define RADIOAES peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_RADIOAES_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_RADIOAES_SHIFT;
#endif

#if defined(_CMU_CLKEN1_RFFPLL0_SHIFT)
// Define RFFPLL0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_RFFPLL0_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_RFFPLL0_SHIFT;
#endif

#if defined(_CMU_CLKEN1_SEMAILBOXHOST_SHIFT)
// Define SEMAILBOX peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_SEMAILBOX_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_SEMAILBOXHOST_SHIFT;
#endif

#if defined(_CMU_CLKEN1_SMU_SHIFT)
// Define SMU peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_SMU_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_SMU_SHIFT;
#endif

#if defined(_CMU_CLKEN0_SYSCFG_SHIFT)
// Define SYSCFG peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_SYSCFG_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_SYSCFG_SHIFT;
#endif

#if defined(_CMU_CLKEN0_SYSRTC0_SHIFT)
// Define SYSRTC0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_SYSRTC0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_SYSRTC0_SHIFT;
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

#if defined(_CMU_CLKEN0_TIMER4_SHIFT)
// Define TIMER4 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_TIMER4_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_TIMER4_SHIFT;
#endif

#if defined(_CMU_CLKEN0_TIMER5_SHIFT)
// Define TIMER5 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_TIMER5_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_TIMER5_SHIFT;
#endif

#if defined(_CMU_CLKEN1_TIMER6_SHIFT)
// Define TIMER6 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_TIMER6_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_TIMER6_SHIFT;
#endif

#if defined(_CMU_CLKEN1_TIMER7_SHIFT)
// Define TIMER7 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_TIMER7_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_TIMER7_SHIFT;
#endif

#if defined(_CMU_CLKEN0_ULFRCO_SHIFT)
// Define ULFRCO peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_ULFRCO_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_ULFRCO_SHIFT;
#endif

#if defined(_CMU_CLKEN1_USB_SHIFT)
// Define USB peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_USB_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_USB_SHIFT;
#endif

#if defined(_CMU_CLKEN1_VDAC0_SHIFT)
// Define VDAC0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_VDAC0_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_VDAC0_SHIFT;
#endif

#if defined(_CMU_CLKEN0_WDOG0_SHIFT)
// Define WDOG0 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_WDOG0_VALUE = (BUS_CLOCK_CLKEN0 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN0_WDOG0_SHIFT;
#endif

#if defined(_CMU_CLKEN1_WDOG1_SHIFT)
// Define WDOG1 peripheral bus clock value.
const uint32_t SL_BUS_CLOCK_WDOG1_VALUE = (BUS_CLOCK_CLKEN1 << _BUS_CLOCK_CLKENX_SHIFT) | _CMU_CLKEN1_WDOG1_SHIFT;
#endif

/** @} (end addtogroup device_clock) */
