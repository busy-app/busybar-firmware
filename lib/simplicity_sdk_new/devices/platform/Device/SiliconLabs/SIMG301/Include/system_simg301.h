/**************************************************************************//**
 * @file
 * @brief CMSIS system header file for SIMG301
 ******************************************************************************
 * # License
 * <b>Copyright 2026 Silicon Laboratories, Inc. www.silabs.com</b>
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

#ifndef SYSTEM_SIMG301_H
#define SYSTEM_SIMG301_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "sl_code_classification.h"

/***************************************************************************//**
 * @addtogroup Parts
 * @{
 ******************************************************************************/
/***************************************************************************//**
 * @addtogroup SIMG301 SIMG301
 * @{
 ******************************************************************************/

/*******************************************************************************
 ******************************   TYPEDEFS   ***********************************
 ******************************************************************************/

/* Interrupt vectortable entry */
typedef union {
  void (*VECTOR_TABLE_Type)(void);
  void *topOfStack;
} tVectorEntry;

/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/

#if !defined(SYSTEM_NO_STATIC_MEMORY)
extern uint32_t SystemCoreClock;     /**< System Clock Frequency (Core Clock) */
extern uint32_t SystemHfrcoFreq;     /**< System HFRCO frequency */
#endif

/*Re-direction of IRQn.*/
#if defined (SL_TRUSTZONE_SECURE)
#define SMU_PRIVILEGED_IRQn    SMU_S_PRIVILEGED_IRQn
#else
#define SMU_PRIVILEGED_IRQn    SMU_NS_PRIVILEGED_IRQn
#endif /* SL_TRUSTZONE_SECURE */

/*Re-direction of IRQHandler.*/
#if defined (SL_TRUSTZONE_SECURE)
#define SMU_PRIVILEGED_IRQHandler    SMU_S_PRIVILEGED_IRQHandler
#else
#define SMU_PRIVILEGED_IRQHandler    SMU_NS_PRIVILEGED_IRQHandler
#endif /* SL_TRUSTZONE_SECURE */

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

void Reset_Handler(void);           /**< Reset Handler */
void NMI_Handler(void);             /**< NMI Handler */
void HardFault_Handler(void);       /**< Hard Fault Handler */
void MemManage_Handler(void);       /**< MPU Fault Handler */
void BusFault_Handler(void);        /**< Bus Fault Handler */
void UsageFault_Handler(void);      /**< Usage Fault Handler */
void SecureFault_Handler(void);     /**< Secure Fault Handler */
void SVC_Handler(void);             /**< SVCall Handler */
void DebugMon_Handler(void);        /**< Debug Monitor Handler */
void PendSV_Handler(void);          /**< PendSV Handler */
void SysTick_Handler(void);         /**< SysTick Handler */

/* Part Specific Interrupts */
void SETAMPERHOST_IRQHandler(void);      /**< SETAMPERHOST IRQ Handler */
void SEMBRX_IRQHandler(void);            /**< SEMBRX IRQ Handler */
void SEMBTX_IRQHandler(void);            /**< SEMBTX IRQ Handler */
void SMU_SECURE_IRQHandler(void);        /**< SMU_SECURE IRQ Handler */
void SMU_S_PRIVILEGED_IRQHandler(void);  /**< SMU_S_PRIVILEGED IRQ Handler */
void SMU_NS_PRIVILEGED_IRQHandler(void); /**< SMU_NS_PRIVILEGED IRQ Handler */
void EMU_IRQHandler(void);               /**< EMU IRQ Handler */
void EMUDG_IRQHandler(void);             /**< EMUDG IRQ Handler */
void SYSMBLPW0CPU_IRQHandler(void);      /**< SYSMBLPW0CPU IRQ Handler */
void ETAMPDET_IRQHandler(void);          /**< ETAMPDET IRQ Handler */
void TIMER0_IRQHandler(void);            /**< TIMER0 IRQ Handler */
void TIMER1_IRQHandler(void);            /**< TIMER1 IRQ Handler */
void TIMER2_IRQHandler(void);            /**< TIMER2 IRQ Handler */
void TIMER3_IRQHandler(void);            /**< TIMER3 IRQ Handler */
void SYSRTC_SEQ_IRQHandler(void);        /**< SYSRTC_SEQ IRQ Handler */
void SYSRTC_APP_IRQHandler(void);        /**< SYSRTC_APP IRQ Handler */
void SYSRTC_MS_IRQHandler(void);         /**< SYSRTC_MS IRQ Handler */
void EUSART0_RX_IRQHandler(void);        /**< EUSART0_RX IRQ Handler */
void EUSART0_TX_IRQHandler(void);        /**< EUSART0_TX IRQ Handler */
void EUSART1_RX_IRQHandler(void);        /**< EUSART1_RX IRQ Handler */
void EUSART1_TX_IRQHandler(void);        /**< EUSART1_TX IRQ Handler */
void EUSART2_RX_IRQHandler(void);        /**< EUSART2_RX IRQ Handler */
void EUSART2_TX_IRQHandler(void);        /**< EUSART2_TX IRQ Handler */
void L1ICACHE0_IRQHandler(void);         /**< L1ICACHE0 IRQ Handler */
void L2ICACHE0_IRQHandler(void);         /**< L2ICACHE0 IRQ Handler */
void BURTC_IRQHandler(void);             /**< BURTC IRQ Handler */
void LETIMER0_IRQHandler(void);          /**< LETIMER0 IRQ Handler */
void PIXELRZ0_IRQHandler(void);          /**< PIXELRZ0 IRQ Handler */
void PIXELRZ1_IRQHandler(void);          /**< PIXELRZ1 IRQ Handler */
void SYSCFG_IRQHandler(void);            /**< SYSCFG IRQ Handler */
void DMEM_IRQHandler(void);              /**< DMEM IRQ Handler */
void LDMA0_CHNL0_IRQHandler(void);       /**< LDMA0_CHNL0 IRQ Handler */
void LDMA0_CHNL1_IRQHandler(void);       /**< LDMA0_CHNL1 IRQ Handler */
void LDMA0_CHNL2_IRQHandler(void);       /**< LDMA0_CHNL2 IRQ Handler */
void LDMA0_CHNL3_IRQHandler(void);       /**< LDMA0_CHNL3 IRQ Handler */
void LDMA0_CHNL4_IRQHandler(void);       /**< LDMA0_CHNL4 IRQ Handler */
void LDMA0_CHNL5_IRQHandler(void);       /**< LDMA0_CHNL5 IRQ Handler */
void LDMA0_CHNL6_IRQHandler(void);       /**< LDMA0_CHNL6 IRQ Handler */
void LDMA0_CHNL7_IRQHandler(void);       /**< LDMA0_CHNL7 IRQ Handler */
void LFXO_IRQHandler(void);              /**< LFXO IRQ Handler */
void LFRCO_IRQHandler(void);             /**< LFRCO IRQ Handler */
void ULFRCO_IRQHandler(void);            /**< ULFRCO IRQ Handler */
void GPIO_ODD_IRQHandler(void);          /**< GPIO_ODD IRQ Handler */
void GPIO_EVEN_IRQHandler(void);         /**< GPIO_EVEN IRQ Handler */
void I2C0_IRQHandler(void);              /**< I2C0 IRQ Handler */
void I2C1_IRQHandler(void);              /**< I2C1 IRQ Handler */
void I2C2_IRQHandler(void);              /**< I2C2 IRQ Handler */
void BUFC_IRQHandler(void);              /**< BUFC IRQ Handler */
void FRC_PRI_IRQHandler(void);           /**< FRC_PRI IRQ Handler */
void FRC_IRQHandler(void);               /**< FRC IRQ Handler */
void PROTIMER_IRQHandler(void);          /**< PROTIMER IRQ Handler */
void RAC_RSM_IRQHandler(void);           /**< RAC_RSM IRQ Handler */
void RAC_SEQ_IRQHandler(void);           /**< RAC_SEQ IRQ Handler */
void SYNTH_IRQHandler(void);             /**< SYNTH IRQ Handler */
void RFECA0_IRQHandler(void);            /**< RFECA0 IRQ Handler */
void RFECA1_IRQHandler(void);            /**< RFECA1 IRQ Handler */
void MODEM_IRQHandler(void);             /**< MODEM IRQ Handler */
void AGC_IRQHandler(void);               /**< AGC IRQ Handler */
void RFTIMER_IRQHandler(void);           /**< RFTIMER IRQ Handler */
void SEQACC_IRQHandler(void);            /**< SEQACC IRQ Handler */
void HFRCOLPW_IRQHandler(void);          /**< HFRCOLPW IRQ Handler */
void HFRCODPLLLPW_IRQHandler(void);      /**< HFRCODPLLLPW IRQ Handler */
void ACMP0_IRQHandler(void);             /**< ACMP0 IRQ Handler */
void ACMP1_IRQHandler(void);             /**< ACMP1 IRQ Handler */
void WDOG0_IRQHandler(void);             /**< WDOG0 IRQ Handler */
void WDOG1_IRQHandler(void);             /**< WDOG1 IRQ Handler */
void HFXO0_IRQHandler(void);             /**< HFXO0 IRQ Handler */
void HFRCO0_IRQHandler(void);            /**< HFRCO0 IRQ Handler */
void HFRCOEM23_IRQHandler(void);         /**< HFRCOEM23 IRQ Handler */
void CMU_IRQHandler(void);               /**< CMU IRQ Handler */
void RPA_IRQHandler(void);               /**< RPA IRQ Handler */
void KSURPA_IRQHandler(void);            /**< KSURPA IRQ Handler */
void KSULPWAES_IRQHandler(void);         /**< KSULPWAES IRQ Handler */
void KSUHOSTSYMCRYPTO_IRQHandler(void);  /**< KSUHOSTSYMCRYPTO IRQ Handler */
void SYMCRYPTO_IRQHandler(void);         /**< SYMCRYPTO IRQ Handler */
void LPWAES_IRQHandler(void);            /**< LPWAES IRQ Handler */
void ADC0_IRQHandler(void);              /**< ADC0 IRQ Handler */
void LEDDRV0_IRQHandler(void);           /**< LEDDRV0 IRQ Handler */
void DPLL0_IRQHandler(void);             /**< DPLL0 IRQ Handler */
void SOCPLL0_IRQHandler(void);           /**< SOCPLL0 IRQ Handler */
void PCNT0_IRQHandler(void);             /**< PCNT0 IRQ Handler */
void SW0_IRQHandler(void);               /**< SW0 IRQ Handler */
void SW1_IRQHandler(void);               /**< SW1 IRQ Handler */
void SW2_IRQHandler(void);               /**< SW2 IRQ Handler */
void SW3_IRQHandler(void);               /**< SW3 IRQ Handler */
void KERNEL0_IRQHandler(void);           /**< KERNEL0 IRQ Handler */
void KERNEL1_IRQHandler(void);           /**< KERNEL1 IRQ Handler */
void M33CTI0_IRQHandler(void);           /**< M33CTI0 IRQ Handler */
void M33CTI1_IRQHandler(void);           /**< M33CTI1 IRQ Handler */
void FPUEXH_IRQHandler(void);            /**< FPUEXH IRQ Handler */

#if (__FPU_PRESENT == 1)
void FPUEH_IRQHandler(void);        /**< FPU IRQ Handler */
#endif

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SYSTEM, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t SystemHCLKGet(void);

/**************************************************************************//**
 * @brief
 *   Update CMSIS SystemCoreClock variable.
 *
 * @details
 *   CMSIS defines a global variable SystemCoreClock that shall hold the
 *   core frequency in Hz. If the core frequency is dynamically changed, the
 *   variable must be kept updated in order to be CMSIS compliant.
 *
 *   Notice that only if changing the core clock frequency through the EMLIB
 *   CMU API, this variable will be kept updated. This function is only
 *   provided for CMSIS compliance and if a user modifies the the core clock
 *   outside the EMLIB CMU API.
 *****************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SYSTEM, SL_CODE_CLASS_TIME_CRITICAL)
static __INLINE uint32_t SystemCoreClockGet(void)
{
  return SystemHCLKGet();
}

/**************************************************************************//**
 * @brief
 *   Update CMSIS SystemCoreClock variable.
 *
 * @details
 *   CMSIS defines a global variable SystemCoreClock that shall hold the
 *   core frequency in Hz. If the core frequency is dynamically changed, the
 *   variable must be kept updated in order to be CMSIS compliant.
 *
 *   Notice that only if changing the core clock frequency through the EMLIB
 *   CMU API, this variable will be kept updated. This function is only
 *   provided for CMSIS compliance and if a user modifies the the core clock
 *   outside the EMLIB CMU API.
 *****************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SYSTEM, SL_CODE_CLASS_TIME_CRITICAL)
static __INLINE void SystemCoreClockUpdate(void)
{
  SystemCoreClockGet(); // Calling SystemCoreClockGet will update the variable
}

void     SystemInit(void);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SYSTEM, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t SystemHFRCODPLLClockGet(void);
void     SystemHFRCODPLLClockSet(uint32_t freq);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SYSTEM, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t SystemSYSCLKGet(void);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SYSTEM, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t SystemMaxCoreClockGet(void);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SYSTEM, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t SystemFSRCOClockGet(void);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SYSTEM, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t SystemHFXOClockGet(void);
void     SystemHFXOClockSet(uint32_t freq);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SYSTEM, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t SystemCLKIN0Get(void);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SYSTEM, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t SystemHFRCOEM23ClockGet(void);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SYSTEM, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t SystemLFXOClockGet(void);
void     SystemLFXOClockSet(uint32_t freq);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SYSTEM, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t SystemLFRCOClockGet(void);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SYSTEM, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t SystemULFRCOClockGet(void);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SYSTEM, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t SystemSOCPLLClockGet(uint8_t instance, uint8_t output);
void     SystemSOCPLLClockSet(uint8_t instance, uint8_t output, uint32_t freq);

/** @} End of group */
/** @} End of group Parts */

#ifdef __cplusplus
}
#endif
#endif /* SYSTEM_SIMG301_H */
