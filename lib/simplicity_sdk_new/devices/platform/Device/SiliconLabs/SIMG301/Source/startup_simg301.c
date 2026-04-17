/******************************************************************************
* @file     startup_simg301.c
* @brief    CMSIS-Core(M) Device Startup File for
*           Device SIMG301
* @version  V2.1.0
* @date     16. December 2020
******************************************************************************/
/*
 * Copyright (c) 2009-2021 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdbool.h>
#include "em_device.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"

#endif

#if defined(BOOTLOADER_ENABLE)
#include "api/btl_interface.h"

#endif
#if defined(SL_APP_PROPERTIES)
#include "api/application_properties.h"

#endif

#define TOTAL_INTERRUPTS    (16 + EXT_IRQ_COUNT)

#if defined(BOOTLOADER_ENABLE)
extern MainBootloaderTable_t mainStageTable;

/*----------------------------------------------------------------------------
 * Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void (*VECTOR_TABLE_Type)(void);
#endif // defined(BOOTLOADER_ENABLE)

#if defined(BOOTLOADER_ENABLE) || defined(USER_SYSTEM_INIT_ENABLE)
extern void SystemInit2(void);
#endif // defined(BOOTLOADER_ENABLE) || defined(USER_SYSTEM_INIT_ENABLE)

#if defined(SL_APP_PROPERTIES)
extern ApplicationProperties_t sl_app_properties;

/*----------------------------------------------------------------------------
 * Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void (*VECTOR_TABLE_Type)(void);
#endif // defined(SL_APP_PROPERTIES)

/*---------------------------------------------------------------------------
 * External References
 *---------------------------------------------------------------------------*/
extern uint32_t __INITIAL_SP;
extern uint32_t __STACK_LIMIT;
#if defined (SL_TRUSTZONE_SECURE) \
  && defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
extern uint64_t __STACK_SEAL;
#endif // defined(SL_TRUSTZONE_SECURE)

extern __NO_RETURN void __PROGRAM_START(void);

#if defined (__START) && defined (__GNUC__)
extern int  __START(void) __attribute__((noreturn));    /* main entry point */
void Copy_Table();
void Zero_Table();
#endif // defined(__START) && defined(__GNUC__)

/*---------------------------------------------------------------------------
 * Internal References
 *---------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler(void);
void Default_Handler(void);

#if defined (__GNUC__)
#ifndef __STACK_SIZE
#define __STACK_SIZE    0x00000400
#endif

#ifndef __HEAP_SIZE
#define __HEAP_SIZE    0x00000C00
#endif
#endif // defined(__GNUC__)

/*----------------------------------------------------------------------------
 * Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Cortex-M Processor Exceptions */
void NMI_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void SecureFault_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
#ifndef SL_APP_PROPERTIES
/* Provide a dummy value for the sl_app_properties symbol. */
void sl_app_properties(void);    /* Prototype to please MISRA checkers. */
void sl_app_properties(void) __attribute__ ((weak, alias("Default_Handler")));
#endif // defined(SL_APP_PROPERTIES)

/* Part Specific Interrupts */
void SETAMPERHOST_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SEMBRX_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SEMBTX_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SMU_SECURE_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SMU_S_PRIVILEGED_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SMU_NS_PRIVILEGED_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void EMU_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void EMUDG_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SYSMBLPW0CPU_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void ETAMPDET_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER2_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER3_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SYSRTC_SEQ_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SYSRTC_APP_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SYSRTC_MS_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void EUSART0_RX_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void EUSART0_TX_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void EUSART1_RX_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void EUSART1_TX_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void EUSART2_RX_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void EUSART2_TX_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void L1ICACHE0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void L2ICACHE0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void BURTC_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LETIMER0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void PIXELRZ0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void PIXELRZ1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SYSCFG_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMEM_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LDMA0_CHNL0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LDMA0_CHNL1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LDMA0_CHNL2_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LDMA0_CHNL3_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LDMA0_CHNL4_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LDMA0_CHNL5_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LDMA0_CHNL6_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LDMA0_CHNL7_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LFXO_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LFRCO_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void ULFRCO_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO_ODD_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO_EVEN_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void I2C0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void I2C2_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void BUFC_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void FRC_PRI_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void FRC_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void PROTIMER_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void RAC_RSM_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void RAC_SEQ_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SYNTH_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void RFECA0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void RFECA1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void MODEM_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void AGC_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void RFTIMER_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SEQACC_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void HFRCOLPW_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void HFRCODPLLLPW_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void ACMP0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void ACMP1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void WDOG0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void WDOG1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void HFXO0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void HFRCO0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void HFRCOEM23_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void CMU_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void RPA_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void KSURPA_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void KSULPWAES_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void KSUHOSTSYMCRYPTO_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SYMCRYPTO_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LPWAES_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void ADC0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LEDDRV0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DPLL0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SOCPLL0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void PCNT0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SW0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SW1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SW2_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SW3_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void KERNEL0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void KERNEL1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void M33CTI0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void M33CTI1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void FPUEXH_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));

/*----------------------------------------------------------------------------
 * Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined (__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif // defined(__GNUC__)

#if defined (__ICCARM__)
#pragma data_alignment=512
extern const tVectorEntry __VECTOR_TABLE[TOTAL_INTERRUPTS];
const tVectorEntry        __VECTOR_TABLE[TOTAL_INTERRUPTS] __VECTOR_TABLE_ATTRIBUTE = {
#elif defined(__GNUC__)
extern const tVectorEntry __VECTOR_TABLE[TOTAL_INTERRUPTS];
const tVectorEntry __VECTOR_TABLE[TOTAL_INTERRUPTS] __attribute__((aligned(512)))
__VECTOR_TABLE_ATTRIBUTE = {
#else
extern const tVectorEntry __VECTOR_TABLE[TOTAL_INTERRUPTS];
const tVectorEntry __VECTOR_TABLE[TOTAL_INTERRUPTS] __VECTOR_TABLE_ATTRIBUTE = {
#endif // defined(__ICCARM__)
  { .topOfStack = &__INITIAL_SP },            /*      Initial Stack Pointer     */
  { &Reset_Handler },                         /*      Reset Handler             */
  { &NMI_Handler },                           /*      -14 NMI Handler           */
  { &HardFault_Handler },                     /*      -13 Hard Fault Handler    */
  { &MemManage_Handler },                     /*      -12 MPU Fault Handler     */
  { &BusFault_Handler },                      /*      -11 Bus Fault Handler     */
  { &UsageFault_Handler },                    /*      -10 Usage Fault Handler   */
  { &SecureFault_Handler },                   /*      -9 Secure Fault Handler   */
  { &Default_Handler },                       /*      Reserved                  */
  { &Default_Handler },                       /*      Reserved                  */
#if defined(BOOTLOADER_ENABLE)
  { (VECTOR_TABLE_Type) & mainStageTable },
#else
  { &Default_Handler },                        /*      Reserved                  */
#endif // defined(BOOTLOADER_ENABLE)
  { &SVC_Handler },                            /*      -5 SVCall Handler         */
  { &DebugMon_Handler },                       /*      -4 Debug Monitor Handler  */
#if defined(SL_APP_PROPERTIES)
  { (VECTOR_TABLE_Type) & sl_app_properties }, /*      Application properties    */
#else
  { &sl_app_properties },                      /*      Application properties    */
#endif // defined(SL_APP_PROPERTIES)
  { &PendSV_Handler },                         /*      -2 PendSV Handler         */
  { &SysTick_Handler },                        /*      -1 SysTick Handler        */

  /* External interrupts */
  { &SETAMPERHOST_IRQHandler },                    /* 00 = SETAMPERHOST */
  { &SEMBRX_IRQHandler },                          /* 01 = SEMBRX */
  { &SEMBTX_IRQHandler },                          /* 02 = SEMBTX */
  { &SMU_SECURE_IRQHandler },                      /* 03 = SMU_SECURE */
  { &SMU_S_PRIVILEGED_IRQHandler },                /* 04 = SMU_S_PRIVILEGED */
  { &SMU_NS_PRIVILEGED_IRQHandler },               /* 05 = SMU_NS_PRIVILEGED */
  { &EMU_IRQHandler },                             /* 06 = EMU */
  { &EMUDG_IRQHandler },                           /* 07 = EMUDG */
  { &SYSMBLPW0CPU_IRQHandler },                    /* 08 = SYSMBLPW0CPU */
  { &ETAMPDET_IRQHandler },                        /* 09 = ETAMPDET */
  { &TIMER0_IRQHandler },                          /* 10 = TIMER0 */
  { &TIMER1_IRQHandler },                          /* 11 = TIMER1 */
  { &TIMER2_IRQHandler },                          /* 12 = TIMER2 */
  { &TIMER3_IRQHandler },                          /* 13 = TIMER3 */
  { &SYSRTC_SEQ_IRQHandler },                      /* 14 = SYSRTC_SEQ */
  { &SYSRTC_APP_IRQHandler },                      /* 15 = SYSRTC_APP */
  { &SYSRTC_MS_IRQHandler },                       /* 16 = SYSRTC_MS */
  { &EUSART0_RX_IRQHandler },                      /* 17 = EUSART0_RX */
  { &EUSART0_TX_IRQHandler },                      /* 18 = EUSART0_TX */
  { &EUSART1_RX_IRQHandler },                      /* 19 = EUSART1_RX */
  { &EUSART1_TX_IRQHandler },                      /* 20 = EUSART1_TX */
  { &EUSART2_RX_IRQHandler },                      /* 21 = EUSART2_RX */
  { &EUSART2_TX_IRQHandler },                      /* 22 = EUSART2_TX */
  { &L1ICACHE0_IRQHandler },                       /* 23 = L1ICACHE0 */
  { &L2ICACHE0_IRQHandler },                       /* 24 = L2ICACHE0 */
  { &BURTC_IRQHandler },                           /* 25 = BURTC */
  { &LETIMER0_IRQHandler },                        /* 26 = LETIMER0 */
  { &PIXELRZ0_IRQHandler },                        /* 27 = PIXELRZ0 */
  { &PIXELRZ1_IRQHandler },                        /* 28 = PIXELRZ1 */
  { &SYSCFG_IRQHandler },                          /* 29 = SYSCFG */
  { &DMEM_IRQHandler },                            /* 30 = DMEM */
  { &LDMA0_CHNL0_IRQHandler },                     /* 31 = LDMA0_CHNL0 */
  { &LDMA0_CHNL1_IRQHandler },                     /* 32 = LDMA0_CHNL1 */
  { &LDMA0_CHNL2_IRQHandler },                     /* 33 = LDMA0_CHNL2 */
  { &LDMA0_CHNL3_IRQHandler },                     /* 34 = LDMA0_CHNL3 */
  { &LDMA0_CHNL4_IRQHandler },                     /* 35 = LDMA0_CHNL4 */
  { &LDMA0_CHNL5_IRQHandler },                     /* 36 = LDMA0_CHNL5 */
  { &LDMA0_CHNL6_IRQHandler },                     /* 37 = LDMA0_CHNL6 */
  { &LDMA0_CHNL7_IRQHandler },                     /* 38 = LDMA0_CHNL7 */
  { &LFXO_IRQHandler },                            /* 39 = LFXO */
  { &LFRCO_IRQHandler },                           /* 40 = LFRCO */
  { &ULFRCO_IRQHandler },                          /* 41 = ULFRCO */
  { &GPIO_ODD_IRQHandler },                        /* 42 = GPIO_ODD */
  { &GPIO_EVEN_IRQHandler },                       /* 43 = GPIO_EVEN */
  { &I2C0_IRQHandler },                            /* 44 = I2C0 */
  { &I2C1_IRQHandler },                            /* 45 = I2C1 */
  { &I2C2_IRQHandler },                            /* 46 = I2C2 */
  { &BUFC_IRQHandler },                            /* 47 = BUFC */
  { &FRC_PRI_IRQHandler },                         /* 48 = FRC_PRI */
  { &FRC_IRQHandler },                             /* 49 = FRC */
  { &PROTIMER_IRQHandler },                        /* 50 = PROTIMER */
  { &RAC_RSM_IRQHandler },                         /* 51 = RAC_RSM */
  { &RAC_SEQ_IRQHandler },                         /* 52 = RAC_SEQ */
  { &SYNTH_IRQHandler },                           /* 53 = SYNTH */
  { &RFECA0_IRQHandler },                          /* 54 = RFECA0 */
  { &RFECA1_IRQHandler },                          /* 55 = RFECA1 */
  { &MODEM_IRQHandler },                           /* 56 = MODEM */
  { &AGC_IRQHandler },                             /* 57 = AGC */
  { &RFTIMER_IRQHandler },                         /* 58 = RFTIMER */
  { &SEQACC_IRQHandler },                          /* 59 = SEQACC */
  { &HFRCOLPW_IRQHandler },                        /* 60 = HFRCOLPW */
  { &HFRCODPLLLPW_IRQHandler },                    /* 61 = HFRCODPLLLPW */
  { &ACMP0_IRQHandler },                           /* 62 = ACMP0 */
  { &ACMP1_IRQHandler },                           /* 63 = ACMP1 */
  { &WDOG0_IRQHandler },                           /* 64 = WDOG0 */
  { &WDOG1_IRQHandler },                           /* 65 = WDOG1 */
  { &HFXO0_IRQHandler },                           /* 66 = HFXO0 */
  { &HFRCO0_IRQHandler },                          /* 67 = HFRCO0 */
  { &HFRCOEM23_IRQHandler },                       /* 68 = HFRCOEM23 */
  { &CMU_IRQHandler },                             /* 69 = CMU */
  { &RPA_IRQHandler },                             /* 70 = RPA */
  { &KSURPA_IRQHandler },                          /* 71 = KSURPA */
  { &KSULPWAES_IRQHandler },                       /* 72 = KSULPWAES */
  { &KSUHOSTSYMCRYPTO_IRQHandler },                /* 73 = KSUHOSTSYMCRYPTO */
  { &SYMCRYPTO_IRQHandler },                       /* 74 = SYMCRYPTO */
  { &LPWAES_IRQHandler },                          /* 75 = LPWAES */
  { &ADC0_IRQHandler },                            /* 76 = ADC0 */
  { &LEDDRV0_IRQHandler },                         /* 77 = LEDDRV0 */
  { &DPLL0_IRQHandler },                           /* 78 = DPLL0 */
  { &SOCPLL0_IRQHandler },                         /* 79 = SOCPLL0 */
  { &PCNT0_IRQHandler },                           /* 80 = PCNT0 */
  { &SW0_IRQHandler },                             /* 81 = SW0 */
  { &SW1_IRQHandler },                             /* 82 = SW1 */
  { &SW2_IRQHandler },                             /* 83 = SW2 */
  { &SW3_IRQHandler },                             /* 84 = SW3 */
  { &KERNEL0_IRQHandler },                         /* 85 = KERNEL0 */
  { &KERNEL1_IRQHandler },                         /* 86 = KERNEL1 */
  { &M33CTI0_IRQHandler },                         /* 87 = M33CTI0 */
  { &M33CTI1_IRQHandler },                         /* 88 = M33CTI1 */
  { &FPUEXH_IRQHandler },                          /* 89 = FPUEXH */
};

#if defined (__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined (__START) && defined (__GNUC__)
void Copy_Table()
{
  uint32_t        *pSrc, *pDest;
  extern uint32_t __etext;
  extern uint32_t __data_start__;
  extern uint32_t __data_end__;
  pSrc  = &__etext;
  pDest = &__data_start__;

  for (; pDest < &__data_end__; ) {
    *pDest++ = *pSrc++;
  }
}

void Zero_Table()
{
  uint32_t        *pDest;
  extern uint32_t __bss_start__;
  extern uint32_t __bss_end__;
  pDest = &__bss_start__;

  for (; pDest < &__bss_end__; ) {
    *pDest++ = 0UL;
  }
}
#endif // defined(__START) && defined(__GNUC__)

#if !defined(SL_LEGACY_LINKER) \
  && !defined(SL_RAM_LINKER)
#if defined (__GNUC__) && !defined(__clang__)
__attribute__((optimize("no-tree-loop-distribute-patterns")))
#endif

void CopyMemory(const uint32_t *from, uint32_t *to, uint32_t count)
{
  while (count--) {
    *to++ = *from++;
  }
}
#if defined (__GNUC__)
void CopyToRam(void)
{
  extern uint32_t __lma_ramfuncs_start__;
  extern uint32_t __lma_ramfuncs_end__;
  extern uint32_t __ramfuncs_start__;

  uint32_t        num_instructions = &__lma_ramfuncs_end__ - &__lma_ramfuncs_start__;

  // Copy RAM functions
  CopyMemory(&__lma_ramfuncs_start__, &__ramfuncs_start__, num_instructions);
}
#elif defined (__ICCARM__)
#pragma language=save
#pragma language=extended
#pragma section="text_ram"
#pragma section="text_ram_init"
void CopyToRam(void)
{
  uint32_t num_instructions = (__section_size("text_ram") + 3) / 4;
  uint32_t * from           = __section_begin("text_ram_init");
  uint32_t * to             = __section_begin("text_ram");

  CopyMemory(from, to, num_instructions);
}
#pragma language=restore
#endif // defined(__GNUC__)
#endif // !defined(SL_LEGACY_LINKER) && !defined(SL_RAM_LINKER)
#if !defined(SL_RAM_LINKER)             \
  && !defined(SL_SRAM_DMEM_ECC_DISABLE) \
  && !defined(SL_CATALOG_GECKO_BOOTLOADER_INTERFACE_PRESENT)
/*---------------------------------------------------------------------------
 * ECC fast initialization function using ARM Store Multiple (STM) instruction
 *
 * @param start Pointer to the start address of the memory region to initialize
 * @param end   Pointer to the end address of the memory region to initialize
 *---------------------------------------------------------------------------*/
__STATIC_FORCEINLINE void ecc_write_zeros(uint32_t *start, uint32_t *end)
{
  uint32_t                remaining    = 0;
  volatile const uint32_t *ram_ptr     = (volatile const uint32_t *) start;
  volatile const uint32_t *end_ram_ptr = (volatile const uint32_t *) end;

  // This fast ECC initialization performs N times 256-byte writes of value 0 by using the ARM
  // Store Multiple (STM)instruction. Each STM can write up to 32 bytes at once. Hence, 256-byte
  // writes takes 8 STMs of 32 byte each. The remaining RAM space size may not be multiple of
  // 256 bytes. So the last RAM chunk (whose size < 256 bytes)is written with zeros using 32-bit
  // word writes (STR instruction).
  //
  // NOTE: Reset_Handler() pushes a few data onto the main stack. However, the following code
  // will overwrite the stack data with zeros. This is not a problem as the stack data
  // of Reset_Handler() will never be used after the ECC initialization. An application
  // is not supposed to return from Reset_Handler().
  __ASM volatile (
    // Initialize registers and RAM end address for the 1st loop.
    "MOVS r5, #0                       \n"           // Store 32 bytes of zeros in 8 registers (R5-R12).
    "MOVS r6, #0                       \n"
    "MOVS r7, #0                       \n"
    "MOVS r8, #0                       \n"
    "MOVS r9, #0                       \n"
    "MOVS r10, #0                      \n"
    "MOVS r11, #0                      \n"
    "MOVS r12, #0                      \n"
    "SUBS %[remaining], %[end_ptr], %[ram_ptr] \n" // remaining = end_ptr - ram_ptr.
    "MOV r3, #256                      \n"         // R3 = 256 (block size).
    "UDIV r4, %[remaining], r3         \n"         // R4 = remaining / 256 (number of 256-byte blocks).
    "MUL r4, r4, r3                    \n"         // R4 = R4 * 256 (total size of 256-byte blocks).
    "ADD r4, %[ram_ptr], r4            \n"         // R4 = ram_ptr + total size of 256-byte blocks.
    // 1st loop for N x 256-byte writes using STM
    "1:                                \n"
    "CMP %[ram_ptr], r4                \n"    // Compare ram_ptr with R4.
    "BHS 2f                            \n"    // If ram_ptr >= R4, branch to label 2.
    "STM %[ram_ptr]!, {r5-r12}         \n"    // Store multiple registers (r5-r12) into memory and increment ram_ptr.
    "STM %[ram_ptr]!, {r5-r12}         \n"
    "STM %[ram_ptr]!, {r5-r12}         \n"
    "STM %[ram_ptr]!, {r5-r12}         \n"
    "STM %[ram_ptr]!, {r5-r12}         \n"
    "STM %[ram_ptr]!, {r5-r12}         \n"
    "STM %[ram_ptr]!, {r5-r12}         \n"
    "STM %[ram_ptr]!, {r5-r12}         \n"
    "B 1b                              \n"    // Branch to label 1.
    // 2nd loop in case there is a last chunk < 256 bytes using STR
    "2:                                \n"
    "CMP %[ram_ptr], %[end_ptr]        \n"                  // Compare ram_ptr with end_ptr.
    "BHS 3f                            \n"                  // If ram_ptr >= end_ptr, branch to label 3.
    "STR r5, [%[ram_ptr]], #4          \n"                  // Store R5 at ram_ptr and increment ram_ptr.
    "B 2b                              \n"                  // Branch to label 2.
    "3:                                \n"
    :
    :[end_ptr] "r" (end_ram_ptr), [ram_ptr] "r" (ram_ptr), [remaining] "r" (remaining)  // ram_ptr and remaining are read/write. end_ptr is read-only.
    : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "memory"
    );
}
/*---------------------------------------------------------------------------
 * ECC preserve initialization function using ARM Load/Store Multiple
 * (LDM/STM) instructions
 *
 * @param start Pointer to the start address of the memory region to initialize
 * @param end   Pointer to the end address of the memory region to initialize
 *---------------------------------------------------------------------------*/
__STATIC_FORCEINLINE void ecc_mem_init(uint32_t *start, uint32_t *end)
{
  uint32_t                remaining    = 0;
  volatile const uint32_t *ram_ptr     = (volatile const uint32_t *) start;
  volatile const uint32_t *end_ram_ptr = (volatile const uint32_t *) end;

  // The ECC preserve initialization leverages the LDM/STM instructions working each on 32 bytes
  // of data at a time.
  __ASM volatile (
    // Initialize RAM end address for the 1st loop.
    "SUBS %[remaining], %[end_ptr], %[ram_ptr] \n" // remaining = end_ptr - ram_ptr.
    "MOV r3, #32                       \n"         // R3 = 32 (block size).
    "UDIV r4, %[remaining], r3         \n"         // R4 = remaining / 32 (number of 32-byte blocks).
    "MUL r4, r4, r3                    \n"         // R4 = R4 * 32 (total size of 32-byte blocks).
    "ADD r4, %[ram_ptr], r4            \n"         // R4 = ram_ptr + total size of 32-byte blocks.
    // 1st loop for N x 32-byte read/write using LDM/STM
    "1:                                \n"
    "CMP %[ram_ptr], r4                \n"         // Compare ram_ptr with R4.
    "BHS 2f                            \n"         // If ram_ptr >= R4, branch to label 2.
    "LDM %[ram_ptr], {r5-r12}         \n"          // Load multiple values into r5-r12 from ram_ptr.
    "STM %[ram_ptr]!, {r5-r12}         \n"         // Store multiple registers (r5-r12) into memory and increment ram_ptr.
    "B 1b                              \n"         // Branch to label 1.
    // 2nd loop in case there is a last chunk < 32 bytes using LDR/STR
    "2:                                \n"
    "CMP %[ram_ptr], %[end_ptr]        \n"                  // Compare ram_ptr with end_ptr.
    "BHS 3f                            \n"                  // If ram_ptr >= end_ptr, branch to label 3.
    "LDR r5, [%[ram_ptr]]              \n"                  // Load the current RAM word into r5.
    "STR r5, [%[ram_ptr]], #4          \n"                  // Store the value read by LDR at ram_ptr and increment ram_ptr.
    "B 2b                              \n"                  // Branch to label 2.
    "3:                                \n"
    :
    :[end_ptr] "r" (end_ram_ptr), [ram_ptr] "r" (ram_ptr), [remaining] "r" (remaining)  // ram_ptr and remaining are read/write. end_ram_ptr is read-only.
    : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "memory"
    );
}
#endif // !defined(SL_RAM_LINKER) && !defined(SL_SRAM_DMEM_ECC_DISABLE) && !defined(SL_CATALOG_GECKO_BOOTLOADER_INTERFACE_PRESENT)
/*---------------------------------------------------------------------------
 * Reset Handler called on controller reset
 *---------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler(void)
{
#if defined(__ICCARM__)
  #error "IAR is not supported on SixG30x devices."
#endif

#if !defined(SL_RAM_LINKER) \
  && !defined(SL_SRAM_DMEM_ECC_DISABLE)
#if !defined(SL_CATALOG_GECKO_BOOTLOADER_INTERFACE_PRESENT)
  // The ECC initialiation has two variants:
  // - ECC fast initialization using the ARM Store Multiple (STM) instruction to write
  // the entire DMEM
  // - ECC preserve initialization using the ARM Load/Store Multiple (LDM/STM) instructions
  // to write DMEM while preserving its current content.
  //
  // ECC fast initialization will always be done on a target Power-On-Reset and provide the
  // fastest ECC syndromes initialization and thus contribute to a quick target boot up during a
  // cold start.
  // ECC preserve initialization will always be done for any other reset reasons (e.g. pin reset,
  // watchdog reset, EM4 reset, brownout reset, system/soft reset). In that case, a repaint
  // algorithm is used.
  // The differentiation between a Power-On-Reset and the other reset reasons is done by
  // reading the EMU_RSTCAUSE bits. the EMU_RSTCAUSE bits are sticky, it might cause to detect a
  // hard reset + soft reset situation at the same time. EMU_RSTCAUSE bits are not cleared by the
  // startup code. The main program should handle the clearing if it is required. EMU_RSTCAUSE bits
  // are automatically cleared when the target power is turned off.
  //
  // The ECC initialization code uses assembly to protect the ECC initialization sequence
  // from C compiler with optimization settings.

  extern uint32_t __ram_end__; // Linker script variable

  bool            ecc_init_fast = true;
  uint32_t        reset_cause   = EMU->RSTCAUSE & ~_EMU_RSTCAUSE_POR_MASK;

  if (reset_cause != 0) {
    // Not a Power-On-Reset. Perform the repaint algorithm.
    ecc_init_fast = false;
  }
#endif // !defined(SL_CATALOG_GECKO_BOOTLOADER_INTERFACE_PRESENT)

  // Enable DMEM controller registers.
  CMU->CLKEN1_SET = CMU_CLKEN1_DMEM;
  // Enable ECC syndrome writes.
  DMEM->CTRL_SET = HYDRARAM_CTRL_ECCWEN;
  // If the application is build with a bootloader (i.e. SL_CATALOG_GECKO_BOOTLOADER_INTERFACE_PRESENT defined),
  // the ECC initialization is completely skipped as the bootloader Reset_Handler() takes care
  // of the ECC initialization.
#if !defined(SL_CATALOG_GECKO_BOOTLOADER_INTERFACE_PRESENT)
  if (ecc_init_fast) {
    ecc_write_zeros((uint32_t *) SRAM_BASE, &__ram_end__);
  } else {
    ecc_mem_init((uint32_t *) SRAM_BASE, &__ram_end__);
  }
#endif // !defined(SL_CATALOG_GECKO_BOOTLOADER_INTERFACE_PRESENT)
  // Enable full ECC protection on RAM with reads verification (auto-correction of 1 bit and bus fault on 2 bits).
  DMEM->CTRL_SET = HYDRARAM_CTRL_ECCEN | HYDRARAM_CTRL_ECCERRFAULTEN;

  // Disable DMEM controller registers once done.
  CMU->CLKEN1_CLR = CMU_CLKEN1_DMEM;
#endif // !defined(SL_RAM_LINKER) && !defined(SL_SRAM_DMEM_ECC_DISABLE)

  __set_MSPLIM((uint32_t) (&__STACK_LIMIT));

#if defined (SL_TRUSTZONE_SECURE) \
  && defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
  __TZ_set_STACKSEAL_S((uint32_t *) (&__STACK_SEAL));
#endif // defined(SL_TRUSTZONE_SECURE) && defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)

  #ifndef __NO_SYSTEM_INIT
  SystemInit();                    /* CMSIS System Initialization */
  #endif

#if !defined(SL_LEGACY_LINKER) \
  && !defined(SL_RAM_LINKER)
  CopyToRam();
#endif // !defined(SL_LEGACY_LINKER) && !defined(SL_RAM_LINKER)

#if defined(BOOTLOADER_ENABLE) || defined(USER_SYSTEM_INIT_ENABLE)
  SystemInit2();
#endif // defined(BOOTLOADER_ENABLE) || defined (USER_SYSTEM_INIT_ENABLE)

#if defined (__GNUC__) && defined (__START)
  Copy_Table();
  Zero_Table();
  __START();
#else
  __PROGRAM_START();               /* Enter PreMain (C library entry point) */
#endif // defined(__GNUC__) && defined(__START)
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif // defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)

/*----------------------------------------------------------------------------
 * Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void)
{
  while (true) {
    // Default behavior is halting execution.
  }
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic pop
#endif // defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
