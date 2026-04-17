/**************************************************************************//**
 * @file
 * @brief CMSIS Cortex-M Peripheral Access Layer Header File
 *        for SIBG301M104XIL
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
#ifndef SIBG301M104XIL_H
#define SIBG301M104XIL_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * @addtogroup Parts
 * @{
 *****************************************************************************/

/**************************************************************************//**
 * @defgroup SIBG301M104XIL SIBG301M104XIL
 * @{
 *****************************************************************************/

/** Interrupt Number Definition */
typedef enum IRQn{
  /******  Cortex-M Processor Exceptions Numbers ******************************************/
  NonMaskableInt_IRQn    = -14,             /*!< -14 Cortex-M Non Maskable Interrupt      */
  HardFault_IRQn         = -13,             /*!< -13 Cortex-M Hard Fault Interrupt        */
  MemoryManagement_IRQn  = -12,             /*!< -12 Cortex-M Memory Management Interrupt */
  BusFault_IRQn          = -11,             /*!< -11 Cortex-M Bus Fault Interrupt         */
  UsageFault_IRQn        = -10,             /*!< -10 Cortex-M Usage Fault Interrupt       */
  SecureFault_IRQn       = -9,              /*!< -9  Cortex-M Secure Fault Interrupt      */
  SVCall_IRQn            = -5,              /*!< -5  Cortex-M SV Call Interrupt           */
  DebugMonitor_IRQn      = -4,              /*!< -4  Cortex-M Debug Monitor Interrupt     */
  PendSV_IRQn            = -2,              /*!< -2  Cortex-M Pend SV Interrupt           */
  SysTick_IRQn           = -1,              /*!< -1  Cortex-M System Tick Interrupt       */

  /******  SIBG301 Peripheral Interrupt Numbers ******************************************/

  SETAMPERHOST_IRQn      = 0,  /*!<  0 Si SETAMPERHOST Interrupt */
  SEMBRX_IRQn            = 1,  /*!<  1 Si SEMBRX Interrupt */
  SEMBTX_IRQn            = 2,  /*!<  2 Si SEMBTX Interrupt */
  SMU_SECURE_IRQn        = 3,  /*!<  3 Si SMU_SECURE Interrupt */
  SMU_S_PRIVILEGED_IRQn  = 4,  /*!<  4 Si SMU_S_PRIVILEGED Interrupt */
  SMU_NS_PRIVILEGED_IRQn = 5,  /*!<  5 Si SMU_NS_PRIVILEGED Interrupt */
  EMU_IRQn               = 6,  /*!<  6 Si EMU Interrupt */
  EMUDG_IRQn             = 7,  /*!<  7 Si EMUDG Interrupt */
  SYSMBLPW0CPU_IRQn      = 8,  /*!<  8 Si SYSMBLPW0CPU Interrupt */
  ETAMPDET_IRQn          = 9,  /*!<  9 Si ETAMPDET Interrupt */
  TIMER0_IRQn            = 10, /*!< 10 Si TIMER0 Interrupt */
  TIMER1_IRQn            = 11, /*!< 11 Si TIMER1 Interrupt */
  TIMER2_IRQn            = 12, /*!< 12 Si TIMER2 Interrupt */
  TIMER3_IRQn            = 13, /*!< 13 Si TIMER3 Interrupt */
  SYSRTC_SEQ_IRQn        = 14, /*!< 14 Si SYSRTC_SEQ Interrupt */
  SYSRTC_APP_IRQn        = 15, /*!< 15 Si SYSRTC_APP Interrupt */
  SYSRTC_MS_IRQn         = 16, /*!< 16 Si SYSRTC_MS Interrupt */
  EUSART0_RX_IRQn        = 17, /*!< 17 Si EUSART0_RX Interrupt */
  EUSART0_TX_IRQn        = 18, /*!< 18 Si EUSART0_TX Interrupt */
  EUSART1_RX_IRQn        = 19, /*!< 19 Si EUSART1_RX Interrupt */
  EUSART1_TX_IRQn        = 20, /*!< 20 Si EUSART1_TX Interrupt */
  EUSART2_RX_IRQn        = 21, /*!< 21 Si EUSART2_RX Interrupt */
  EUSART2_TX_IRQn        = 22, /*!< 22 Si EUSART2_TX Interrupt */
  L1ICACHE0_IRQn         = 23, /*!< 23 Si L1ICACHE0 Interrupt */
  L2ICACHE0_IRQn         = 24, /*!< 24 Si L2ICACHE0 Interrupt */
  BURTC_IRQn             = 25, /*!< 25 Si BURTC Interrupt */
  LETIMER0_IRQn          = 26, /*!< 26 Si LETIMER0 Interrupt */
  PIXELRZ0_IRQn          = 27, /*!< 27 Si PIXELRZ0 Interrupt */
  PIXELRZ1_IRQn          = 28, /*!< 28 Si PIXELRZ1 Interrupt */
  SYSCFG_IRQn            = 29, /*!< 29 Si SYSCFG Interrupt */
  DMEM_IRQn              = 30, /*!< 30 Si DMEM Interrupt */
  LDMA0_CHNL0_IRQn       = 31, /*!< 31 Si LDMA0_CHNL0 Interrupt */
  LDMA0_CHNL1_IRQn       = 32, /*!< 32 Si LDMA0_CHNL1 Interrupt */
  LDMA0_CHNL2_IRQn       = 33, /*!< 33 Si LDMA0_CHNL2 Interrupt */
  LDMA0_CHNL3_IRQn       = 34, /*!< 34 Si LDMA0_CHNL3 Interrupt */
  LDMA0_CHNL4_IRQn       = 35, /*!< 35 Si LDMA0_CHNL4 Interrupt */
  LDMA0_CHNL5_IRQn       = 36, /*!< 36 Si LDMA0_CHNL5 Interrupt */
  LDMA0_CHNL6_IRQn       = 37, /*!< 37 Si LDMA0_CHNL6 Interrupt */
  LDMA0_CHNL7_IRQn       = 38, /*!< 38 Si LDMA0_CHNL7 Interrupt */
  LFXO_IRQn              = 39, /*!< 39 Si LFXO Interrupt */
  LFRCO_IRQn             = 40, /*!< 40 Si LFRCO Interrupt */
  ULFRCO_IRQn            = 41, /*!< 41 Si ULFRCO Interrupt */
  GPIO_ODD_IRQn          = 42, /*!< 42 Si GPIO_ODD Interrupt */
  GPIO_EVEN_IRQn         = 43, /*!< 43 Si GPIO_EVEN Interrupt */
  I2C0_IRQn              = 44, /*!< 44 Si I2C0 Interrupt */
  I2C1_IRQn              = 45, /*!< 45 Si I2C1 Interrupt */
  I2C2_IRQn              = 46, /*!< 46 Si I2C2 Interrupt */
  BUFC_IRQn              = 47, /*!< 47 Si BUFC Interrupt */
  FRC_PRI_IRQn           = 48, /*!< 48 Si FRC_PRI Interrupt */
  FRC_IRQn               = 49, /*!< 49 Si FRC Interrupt */
  PROTIMER_IRQn          = 50, /*!< 50 Si PROTIMER Interrupt */
  RAC_RSM_IRQn           = 51, /*!< 51 Si RAC_RSM Interrupt */
  RAC_SEQ_IRQn           = 52, /*!< 52 Si RAC_SEQ Interrupt */
  SYNTH_IRQn             = 53, /*!< 53 Si SYNTH Interrupt */
  RFECA0_IRQn            = 54, /*!< 54 Si RFECA0 Interrupt */
  RFECA1_IRQn            = 55, /*!< 55 Si RFECA1 Interrupt */
  MODEM_IRQn             = 56, /*!< 56 Si MODEM Interrupt */
  AGC_IRQn               = 57, /*!< 57 Si AGC Interrupt */
  RFTIMER_IRQn           = 58, /*!< 58 Si RFTIMER Interrupt */
  SEQACC_IRQn            = 59, /*!< 59 Si SEQACC Interrupt */
  HFRCOLPW_IRQn          = 60, /*!< 60 Si HFRCOLPW Interrupt */
  HFRCODPLLLPW_IRQn      = 61, /*!< 61 Si HFRCODPLLLPW Interrupt */
  ACMP0_IRQn             = 62, /*!< 62 Si ACMP0 Interrupt */
  ACMP1_IRQn             = 63, /*!< 63 Si ACMP1 Interrupt */
  WDOG0_IRQn             = 64, /*!< 64 Si WDOG0 Interrupt */
  WDOG1_IRQn             = 65, /*!< 65 Si WDOG1 Interrupt */
  HFXO0_IRQn             = 66, /*!< 66 Si HFXO0 Interrupt */
  HFRCO0_IRQn            = 67, /*!< 67 Si HFRCO0 Interrupt */
  HFRCOEM23_IRQn         = 68, /*!< 68 Si HFRCOEM23 Interrupt */
  CMU_IRQn               = 69, /*!< 69 Si CMU Interrupt */
  RPA_IRQn               = 70, /*!< 70 Si RPA Interrupt */
  KSURPA_IRQn            = 71, /*!< 71 Si KSURPA Interrupt */
  KSULPWAES_IRQn         = 72, /*!< 72 Si KSULPWAES Interrupt */
  KSUHOSTSYMCRYPTO_IRQn  = 73, /*!< 73 Si KSUHOSTSYMCRYPTO Interrupt */
  SYMCRYPTO_IRQn         = 74, /*!< 74 Si SYMCRYPTO Interrupt */
  LPWAES_IRQn            = 75, /*!< 75 Si LPWAES Interrupt */
  ADC0_IRQn              = 76, /*!< 76 Si ADC0 Interrupt */
  DPLL0_IRQn             = 78, /*!< 78 Si DPLL0 Interrupt */
  SOCPLL0_IRQn           = 79, /*!< 79 Si SOCPLL0 Interrupt */
  PCNT0_IRQn             = 80, /*!< 80 Si PCNT0 Interrupt */
  SW0_IRQn               = 81, /*!< 81 Si SW0 Interrupt */
  SW1_IRQn               = 82, /*!< 82 Si SW1 Interrupt */
  SW2_IRQn               = 83, /*!< 83 Si SW2 Interrupt */
  SW3_IRQn               = 84, /*!< 84 Si SW3 Interrupt */
  KERNEL0_IRQn           = 85, /*!< 85 Si KERNEL0 Interrupt */
  KERNEL1_IRQn           = 86, /*!< 86 Si KERNEL1 Interrupt */
  M33CTI0_IRQn           = 87, /*!< 87 Si M33CTI0 Interrupt */
  M33CTI1_IRQn           = 88, /*!< 88 Si M33CTI1 Interrupt */
  FPUEXH_IRQn            = 89, /*!< 89 Si FPUEXH Interrupt */
} IRQn_Type;

/**************************************************************************//**
 * @defgroup SIBG301M104XIL_Core SIBG301M104XIL Core
 * @{
 * @brief Processor and Core Peripheral Section
 *****************************************************************************/

#define __CORTEXM                 1U      /**< Core architecture */
#define __CM33_REV                0x0004U /**< Cortex-M33 Core revision */
#define __DSP_PRESENT             1U      /**< Presence of DSP  */
#define __FPU_PRESENT             1U      /**< Presence of FPU  */
#define __MPU_PRESENT             1U      /**< Presence of MPU  */
#define __SAUREGION_PRESENT       1U      /**< Presence of FPU  */
#define __TZ_PRESENT              1U      /**< Presence of TrustZone */
#define __VTOR_PRESENT            1U      /**< Presence of VTOR register in SCB  */
#define __NVIC_PRIO_BITS          4U      /**< NVIC interrupt priority bits */
#define __Vendor_SysTickConfig    0U      /**< Is 1 if different SysTick counter is used */

/** @} End of group SIBG301M104XIL_Core */

/**************************************************************************//**
* @defgroup SIBG301M104XIL_Part SIBG301M104XIL Part
* @{
******************************************************************************/

/** Part number */

/* If part number is not defined as compiler option, define it */
#if !defined(SIBG301M104XIL)
#define SIBG301M104XIL    1 /**< FULL Part */
#endif

/** Configure part number */
#define PART_NUMBER                                       "SIBG301M104XIL" /**< Part Number */

/** Family / Line / Series / Config */
#define _SI_BLUE_FAMILY                                   1                              /** Device Family Name Identifier */
#define _SI_BG_FAMILY                                     1                              /** Device Family Identifier */
#define _SI_DEVICE                                        1                              /** Product Line Identifier */
#define _SILICON_LABS_32B_SERIES_3                                                       /** Product Series Identifier */
#define _SILICON_LABS_32B_SERIES                          3                              /** Product Series Identifier */
#define _SILICON_LABS_32B_SERIES_3_CONFIG_301                                            /** Product Config Identifier */
#define _SILICON_LABS_32B_SERIES_3_CONFIG                 301                            /** Product Config Identifier */
#define _SILICON_LABS_IP_LPW_TYPE_LPW_NONE                0                              /** LPWx Config Identifier */
#define _SILICON_LABS_IP_LPW_TYPE_LPWL                    1                              /** LPWx Config Identifier */
#define _SILICON_LABS_IP_LPW_TYPE_LPWH                    2                              /** LPWx Config Identifier */
#define _SILICON_LABS_IP_LPWL_PRESENT
#define _SILICON_LABS_IP_LPW_TYPE                         _SILICON_LABS_IP_LPW_TYPE_LPWL /** Active LPW Config Identifier */
#define _SILICON_LABS_IP_LPW_TYPE_IS_LPWL                                                /** Active LPW Config Identifier */
#define _SILICON_LABS_IP_LPWL_MAJOR_VERSION               1
#define _SILICON_LABS_IP_LPWL_MINOR_VERSION               0
#define _SILICON_LABS_SECURITY_FEATURE_VAULT              0                                    /** None */
#define _SILICON_LABS_SECURITY_FEATURE                    _SILICON_LABS_SECURITY_FEATURE_VAULT /** None */
#define _SILICON_LABS_DCDC_FEATURE_NOTUSED                0                                    /** Not Used */
#define _SILICON_LABS_DCDC_FEATURE_DCDC_BUCK              1                                    /** Includes Buck DCDC */
#define _SILICON_LABS_DCDC_FEATURE_DCDC_BOOST             2                                    /** Includes Boost DCDC */
#define _SILICON_LABS_DCDC_FEATURE_DCDC_BOB               3                                    /** Includes Buck or Boost DCDC */
#define _SILICON_LABS_DCDC_FEATURE                        _SILICON_LABS_DCDC_FEATURE_NOTUSED   /** DCDC feature set */
#define _SILICON_LABS_EFR32_RADIO_NONE                    0                                    /** No radio present */
#define _SILICON_LABS_EFR32_RADIO_SUBGHZ                  1                                    /** Radio supports Sub-GHz */
#define _SILICON_LABS_EFR32_RADIO_2G4HZ                   2                                    /** Radio supports 2.4 GHz */
#define _SILICON_LABS_EFR32_RADIO_DUALBAND                3                                    /** Radio supports dual band */
#define _SILICON_LABS_EFR32_RADIO_TYPE                    _SILICON_LABS_EFR32_RADIO_2G4HZ      /** Radio type */
#define _SILICON_LABS_EFR32_2G4HZ_MP_PA_MAX_OUTPUT_DBM    10                                   /** Radio 2G4HZ MP PA output power */
#define _SILICON_LABS_EFR32_2G4HZ_LP_PA_MAX_OUTPUT_DBM    0                                    /** Radio 2G4HZ LP PA output power */
#define _SILICON_LABS_EFR32_2G4HZ_MP_PA_PRESENT                                                /** Radio 2G4HZ MP PA is present */
#define _SILICON_LABS_EFR32_2G4HZ_LP_PA_PRESENT                                                /** Radio 2G4HZ LP PA is present */

/** Memory Base addresses and limits */
#define DMEM_INSTR_NS_MEM_BASE                            (0x00800000UL) /** DMEM_INSTR_NS_MEM base address */
#define DMEM_INSTR_NS_MEM_MAX_SIZE                        (0x00080000UL) /** DMEM_INSTR_NS_MEM available address space */
#define DMEM_INSTR_NS_MEM_END                             (0x0087FFFFUL) /** DMEM_INSTR_NS_MEM end address */
#define DMEM_INSTR_NS_MEM_BITS                            (0x14UL)       /** DMEM_INSTR_NS_MEM used bits */
#define L2ICACHE0_INSTR_NS_MEM_BASE                       (0x01000000UL) /** L2ICACHE0_INSTR_NS_MEM base address */
#define L2ICACHE0_INSTR_NS_MEM_MAX_SIZE                   (0x08000000UL) /** L2ICACHE0_INSTR_NS_MEM available address space */
#define L2ICACHE0_INSTR_NS_MEM_END                        (0x08FFFFFFUL) /** L2ICACHE0_INSTR_NS_MEM end address */
#define L2ICACHE0_INSTR_NS_MEM_BITS                       (0x1CUL)       /** L2ICACHE0_INSTR_NS_MEM used bits */
#define DMEM_INSTR_S_MEM_BASE                             (0x10800000UL) /** DMEM_INSTR_S_MEM base address */
#define DMEM_INSTR_S_MEM_MAX_SIZE                         (0x00080000UL) /** DMEM_INSTR_S_MEM available address space */
#define DMEM_INSTR_S_MEM_END                              (0x1087FFFFUL) /** DMEM_INSTR_S_MEM end address */
#define DMEM_INSTR_S_MEM_BITS                             (0x14UL)       /** DMEM_INSTR_S_MEM used bits */
#define L2ICACHE0_INSTR_S_MEM_BASE                        (0x11000000UL) /** L2ICACHE0_INSTR_S_MEM base address */
#define L2ICACHE0_INSTR_S_MEM_MAX_SIZE                    (0x08000000UL) /** L2ICACHE0_INSTR_S_MEM available address space */
#define L2ICACHE0_INSTR_S_MEM_END                         (0x18FFFFFFUL) /** L2ICACHE0_INSTR_S_MEM end address */
#define L2ICACHE0_INSTR_S_MEM_BITS                        (0x1CUL)       /** L2ICACHE0_INSTR_S_MEM used bits */
#define DMEM_NS_MEM_BASE                                  (0x20000000UL) /** DMEM_NS_MEM base address */
#define DMEM_NS_MEM_MAX_SIZE                              (0x00080000UL) /** DMEM_NS_MEM available address space */
#define DMEM_NS_MEM_END                                   (0x2007FFFFUL) /** DMEM_NS_MEM end address */
#define DMEM_NS_MEM_BITS                                  (0x14UL)       /** DMEM_NS_MEM used bits */
#define DMEM_S_MEM_BASE                                   (0x30000000UL) /** DMEM_S_MEM base address */
#define DMEM_S_MEM_MAX_SIZE                               (0x00080000UL) /** DMEM_S_MEM available address space */
#define DMEM_S_MEM_END                                    (0x3007FFFFUL) /** DMEM_S_MEM end address */
#define DMEM_S_MEM_BITS                                   (0x14UL)       /** DMEM_S_MEM used bits */
#define KSU_NS_MEM_BASE                                   (0x41C00000UL) /** KSU_NS_MEM base address */
#define KSU_NS_MEM_MAX_SIZE                               (0x00000280UL) /** KSU_NS_MEM available address space */
#define KSU_NS_MEM_END                                    (0x41C0027FUL) /** KSU_NS_MEM end address */
#define KSU_NS_MEM_BITS                                   (0xAUL)        /** KSU_NS_MEM used bits */
#define KSU_S_MEM_BASE                                    (0x51C00000UL) /** KSU_S_MEM base address */
#define KSU_S_MEM_MAX_SIZE                                (0x00000280UL) /** KSU_S_MEM available address space */
#define KSU_S_MEM_END                                     (0x51C0027FUL) /** KSU_S_MEM end address */
#define KSU_S_MEM_BITS                                    (0xAUL)        /** KSU_S_MEM used bits */
#define L2ICACHE0_NS_MEM_BASE                             (0x61000000UL) /** L2ICACHE0_NS_MEM base address */
#define L2ICACHE0_NS_MEM_MAX_SIZE                         (0x08000000UL) /** L2ICACHE0_NS_MEM available address space */
#define L2ICACHE0_NS_MEM_END                              (0x68FFFFFFUL) /** L2ICACHE0_NS_MEM end address */
#define L2ICACHE0_NS_MEM_BITS                             (0x1CUL)       /** L2ICACHE0_NS_MEM used bits */
#define L2ICACHE0_S_MEM_BASE                              (0x71000000UL) /** L2ICACHE0_S_MEM base address */
#define L2ICACHE0_S_MEM_MAX_SIZE                          (0x08000000UL) /** L2ICACHE0_S_MEM available address space */
#define L2ICACHE0_S_MEM_END                               (0x78FFFFFFUL) /** L2ICACHE0_S_MEM end address */
#define L2ICACHE0_S_MEM_BITS                              (0x1CUL)       /** L2ICACHE0_S_MEM used bits */

/** Flash and SRAM limits for SIBG301M104XIL */
#define FLASH_S_BASE                                      (0x11000000UL) /**< Flash Base Address */
#define SRAM_S_BASE                                       (0x30000000UL) /**< SRAM Base Address */
#define SRAM_ALIAS_S_BASE                                 (0x10800000UL) /**< SRAM Alias Base Address */
#define FLASH_NS_BASE                                     (0x01000000UL) /**< Flash Base Address */
#define SRAM_NS_BASE                                      (0x20000000UL) /**< SRAM Base Address */
#define SRAM_ALIAS_NS_BASE                                (0x00800000UL) /**< SRAM Alias Base Address */

#if defined(SL_TRUSTZONE_SECURE)
#define FLASH_BASE                                        (FLASH_S_BASE)       /**< Flash Base Address */
#define SRAM_BASE                                         (SRAM_S_BASE)        /**< SRAM Base Address */
#define SRAM_ALIAS_BASE                                   (SRAM_ALIAS_S_BASE)  /**< SRAM Alias Base Address */
#else
#define FLASH_BASE                                        (FLASH_NS_BASE)      /**< Flash Base Address */
#define SRAM_BASE                                         (SRAM_NS_BASE)       /**< SRAM Base Address */
#define SRAM_ALIAS_BASE                                   (SRAM_ALIAS_NS_BASE) /**< SRAM Alias Base Address */
#endif

#define FLASH_PAGE_SIZE                                   (0x00001000UL) /**< Flash Memory page size */
#define SRAM_SIZE                                         (0x00080000UL) /**< Available SRAM Memory */
#define DMA_CHAN_COUNT                                    LDMA_CH_NUM(0) /**< Number of DMA channels */
#define EXT_IRQ_COUNT                                     90             /**< Number of External (NVIC) interrupts */

/* GPIO Avalibility Info */
#define GPIO_PA_INDEX                                     0U         /**< Index of port PA */
#define GPIO_PA_COUNT                                     3U         /**< Number of pins on port PA */
#define GPIO_PA_MASK                                      (0x0007UL) /**< Port PA pin mask */
#define GPIO_PA_PIN0                                      1U         /**< GPIO pin PA0 is present. */
#define GPIO_PA_PIN1                                      1U         /**< GPIO pin PA1 is present. */
#define GPIO_PA_PIN2                                      1U         /**< GPIO pin PA2 is present. */
#define GPIO_PB_INDEX                                     1U         /**< Index of port PB */
#define GPIO_PB_COUNT                                     4U         /**< Number of pins on port PB */
#define GPIO_PB_MASK                                      (0x000FUL) /**< Port PB pin mask */
#define GPIO_PB_PIN0                                      1U         /**< GPIO pin PB0 is present. */
#define GPIO_PB_PIN1                                      1U         /**< GPIO pin PB1 is present. */
#define GPIO_PB_PIN2                                      1U         /**< GPIO pin PB2 is present. */
#define GPIO_PB_PIN3                                      1U         /**< GPIO pin PB3 is present. */
#define GPIO_PC_INDEX                                     2U         /**< Index of port PC */
#define GPIO_PC_COUNT                                     8U         /**< Number of pins on port PC */
#define GPIO_PC_MASK                                      (0x00FFUL) /**< Port PC pin mask */
#define GPIO_PC_PIN0                                      1U         /**< GPIO pin PC0 is present. */
#define GPIO_PC_PIN1                                      1U         /**< GPIO pin PC1 is present. */
#define GPIO_PC_PIN2                                      1U         /**< GPIO pin PC2 is present. */
#define GPIO_PC_PIN3                                      1U         /**< GPIO pin PC3 is present. */
#define GPIO_PC_PIN4                                      1U         /**< GPIO pin PC4 is present. */
#define GPIO_PC_PIN5                                      1U         /**< GPIO pin PC5 is present. */
#define GPIO_PC_PIN6                                      1U         /**< GPIO pin PC6 is present. */
#define GPIO_PC_PIN7                                      1U         /**< GPIO pin PC7 is present. */
#define GPIO_PD_INDEX                                     3U         /**< Index of port PD */
#define GPIO_PD_COUNT                                     7U         /**< Number of pins on port PD */
#define GPIO_PD_MASK                                      (0x007FUL) /**< Port PD pin mask */
#define GPIO_PD_PIN0                                      1U         /**< GPIO pin PD0 is present. */
#define GPIO_PD_PIN1                                      1U         /**< GPIO pin PD1 is present. */
#define GPIO_PD_PIN2                                      1U         /**< GPIO pin PD2 is present. */
#define GPIO_PD_PIN3                                      1U         /**< GPIO pin PD3 is present. */
#define GPIO_PD_PIN4                                      1U         /**< GPIO pin PD4 is present. */
#define GPIO_PD_PIN5                                      1U         /**< GPIO pin PD5 is present. */
#define GPIO_PD_PIN6                                      1U         /**< GPIO pin PD6 is present. */

/* Fixed Resource Locations */
#define ADC0_VREFP_PORT                                   GPIO_PA_INDEX /**< Port of VREFP.*/
#define ADC0_VREFP_PIN                                    0U            /**< Pin of VREFP.*/
#define ETAMPDET_ETAMPIN0_PORT                            GPIO_PB_INDEX /**< Port of ETAMPIN0.*/
#define ETAMPDET_ETAMPIN0_PIN                             1U            /**< Pin of ETAMPIN0.*/
#define ETAMPDET_ETAMPIN1_PORT                            GPIO_PC_INDEX /**< Port of ETAMPIN1.*/
#define ETAMPDET_ETAMPIN1_PIN                             0U            /**< Pin of ETAMPIN1.*/
#define ETAMPDET_ETAMPOUT0_PORT                           GPIO_PC_INDEX /**< Port of ETAMPOUT0.*/
#define ETAMPDET_ETAMPOUT0_PIN                            1U            /**< Pin of ETAMPOUT0.*/
#define ETAMPDET_ETAMPOUT1_PORT                           GPIO_PC_INDEX /**< Port of ETAMPOUT1.*/
#define ETAMPDET_ETAMPOUT1_PIN                            2U            /**< Pin of ETAMPOUT1.*/
#define GPIO_SWCLK_PORT                                   GPIO_PA_INDEX /**< Port of SWCLK.*/
#define GPIO_SWCLK_PIN                                    1U            /**< Pin of SWCLK.*/
#define GPIO_SWDIO_PORT                                   GPIO_PA_INDEX /**< Port of SWDIO.*/
#define GPIO_SWDIO_PIN                                    2U            /**< Pin of SWDIO.*/
#define GPIO_EM4WU10_PORT                                 GPIO_PD_INDEX /**< Port of EM4WU10.*/
#define GPIO_EM4WU10_PIN                                  5U            /**< Pin of EM4WU10.*/
#define GPIO_EM4WU3_PORT                                  GPIO_PB_INDEX /**< Port of EM4WU3.*/
#define GPIO_EM4WU3_PIN                                   1U            /**< Pin of EM4WU3.*/
#define GPIO_EM4WU4_PORT                                  GPIO_PB_INDEX /**< Port of EM4WU4.*/
#define GPIO_EM4WU4_PIN                                   3U            /**< Pin of EM4WU4.*/
#define GPIO_EM4WU6_PORT                                  GPIO_PC_INDEX /**< Port of EM4WU6.*/
#define GPIO_EM4WU6_PIN                                   0U            /**< Pin of EM4WU6.*/
#define GPIO_EM4WU7_PORT                                  GPIO_PC_INDEX /**< Port of EM4WU7.*/
#define GPIO_EM4WU7_PIN                                   5U            /**< Pin of EM4WU7.*/
#define GPIO_EM4WU8_PORT                                  GPIO_PC_INDEX /**< Port of EM4WU8.*/
#define GPIO_EM4WU8_PIN                                   7U            /**< Pin of EM4WU8.*/
#define GPIO_EM4WU9_PORT                                  GPIO_PD_INDEX /**< Port of EM4WU9.*/
#define GPIO_EM4WU9_PIN                                   2U            /**< Pin of EM4WU9.*/
#define LFXO_LFXTAL_I_PORT                                GPIO_PD_INDEX /**< Port of LFXTAL_I.*/
#define LFXO_LFXTAL_I_PIN                                 1U            /**< Pin of LFXTAL_I.*/
#define LFXO_LFXTAL_O_PORT                                GPIO_PD_INDEX /**< Port of LFXTAL_O.*/
#define LFXO_LFXTAL_O_PIN                                 0U            /**< Pin of LFXTAL_O.*/
#define LFXO_LF_EXTCLK_PORT                               GPIO_PD_INDEX /**< Port of LF_EXTCLK.*/
#define LFXO_LF_EXTCLK_PIN                                1U            /**< Pin of LF_EXTCLK.*/

/* Part number capabilities */
#define ACMP_PRESENT                                        /** ACMP is available in this part */
#define ACMP_COUNT                                        2 /** 2 ACMPs available  */
#define ADC_PRESENT                                         /** ADC is available in this part */
#define ADC_COUNT                                         1 /** 1 ADCs available  */
#define BURAM_PRESENT                                       /** BURAM is available in this part */
#define BURAM_COUNT                                       1 /** 1 BURAMs available  */
#define BURTC_PRESENT                                       /** BURTC is available in this part */
#define BURTC_COUNT                                       1 /** 1 BURTCs available  */
#define CMU_PRESENT                                         /** CMU is available in this part */
#define CMU_COUNT                                         1 /** 1 CMUs available  */
#define DEVINFO_PRESENT                                     /** DEVINFO is available in this part */
#define DEVINFO_COUNT                                     1 /** 1 DEVINFOs available  */
#define DMEM_PRESENT                                        /** DMEM is available in this part */
#define DMEM_COUNT                                        1 /** 1 DMEMs available  */
#define DPLL_PRESENT                                        /** DPLL is available in this part */
#define DPLL_COUNT                                        1 /** 1 DPLLs available  */
#define EMU_PRESENT                                         /** EMU is available in this part */
#define EMU_COUNT                                         1 /** 1 EMUs available  */
#define ETAMPDET_PRESENT                                    /** ETAMPDET is available in this part */
#define ETAMPDET_COUNT                                    1 /** 1 ETAMPDETs available  */
#define EUSART_PRESENT                                      /** EUSART is available in this part */
#define EUSART_COUNT                                      3 /** 3 EUSARTs available  */
#define FSRCO_PRESENT                                       /** FSRCO is available in this part */
#define FSRCO_COUNT                                       1 /** 1 FSRCOs available  */
#define GPCRC_PRESENT                                       /** GPCRC is available in this part */
#define GPCRC_COUNT                                       1 /** 1 GPCRCs available  */
#define GPIO_PRESENT                                        /** GPIO is available in this part */
#define GPIO_COUNT                                        1 /** 1 GPIOs available  */
#define HFRCO_PRESENT                                       /** HFRCO is available in this part */
#define HFRCO_COUNT                                       1 /** 1 HFRCOs available  */
#define HFRCOEM23_PRESENT                                   /** HFRCOEM23 is available in this part */
#define HFRCOEM23_COUNT                                   1 /** 1 HFRCOEM23s available  */
#define HFXO_PRESENT                                        /** HFXO is available in this part */
#define HFXO_COUNT                                        1 /** 1 HFXOs available  */
#define HOSTPORTAL_PRESENT                                  /** HOSTPORTAL is available in this part */
#define HOSTPORTAL_COUNT                                  1 /** 1 HOSTPORTALs available  */
#define I2C_PRESENT                                         /** I2C is available in this part */
#define I2C_COUNT                                         3 /** 3 I2Cs available  */
#define KSU_PRESENT                                         /** KSU is available in this part */
#define KSU_COUNT                                         1 /** 1 KSUs available  */
#define L1ICACHE_PRESENT                                    /** L1ICACHE is available in this part */
#define L1ICACHE_COUNT                                    1 /** 1 L1ICACHEs available  */
#define L2ICACHE_PRESENT                                    /** L2ICACHE is available in this part */
#define L2ICACHE_COUNT                                    1 /** 1 L2ICACHEs available  */
#define LDMA_PRESENT                                        /** LDMA is available in this part */
#define LDMA_COUNT                                        1 /** 1 LDMAs available  */
#define LDMAXBAR_PRESENT                                    /** LDMAXBAR is available in this part */
#define LDMAXBAR_COUNT                                    1 /** 1 LDMAXBARs available  */
#define LETIMER_PRESENT                                     /** LETIMER is available in this part */
#define LETIMER_COUNT                                     1 /** 1 LETIMERs available  */
#define LFRCO_PRESENT                                       /** LFRCO is available in this part */
#define LFRCO_COUNT                                       1 /** 1 LFRCOs available  */
#define LFXO_PRESENT                                        /** LFXO is available in this part */
#define LFXO_COUNT                                        1 /** 1 LFXOs available  */
#define LPW0PORTAL_PRESENT                                  /** LPW0PORTAL is available in this part */
#define LPW0PORTAL_COUNT                                  1 /** 1 LPW0PORTALs available  */
#define LPWAES_PRESENT                                      /** LPWAES is available in this part */
#define LPWAES_COUNT                                      1 /** 1 LPWAESs available  */
#define PCNT_PRESENT                                        /** PCNT is available in this part */
#define PCNT_COUNT                                        1 /** 1 PCNTs available  */
#define PIXELRZ_PRESENT                                     /** PIXELRZ is available in this part */
#define PIXELRZ_COUNT                                     2 /** 2 PIXELRZs available  */
#define PRS_PRESENT                                         /** PRS is available in this part */
#define PRS_COUNT                                         1 /** 1 PRSs available  */
#define RPA_PRESENT                                         /** RPA is available in this part */
#define RPA_COUNT                                         1 /** 1 RPAs available  */
#define SCRATCHPAD_PRESENT                                  /** SCRATCHPAD is available in this part */
#define SCRATCHPAD_COUNT                                  1 /** 1 SCRATCHPADs available  */
#define SEMAILBOX_PRESENT                                   /** SEMAILBOX is available in this part */
#define SEMAILBOX_COUNT                                   1 /** 1 SEMAILBOXs available  */
#define SEMAPHORE_PRESENT                                   /** SEMAPHORE is available in this part */
#define SEMAPHORE_COUNT                                   2 /** 2 SEMAPHOREs available  */
#define SEPORTAL_PRESENT                                    /** SEPORTAL is available in this part */
#define SEPORTAL_COUNT                                    1 /** 1 SEPORTALs available  */
#define SMU_PRESENT                                         /** SMU is available in this part */
#define SMU_COUNT                                         1 /** 1 SMUs available  */
#define SOCPLL_PRESENT                                      /** SOCPLL is available in this part */
#define SOCPLL_COUNT                                      1 /** 1 SOCPLLs available  */
#define SYMCRYPTO_PRESENT                                   /** SYMCRYPTO is available in this part */
#define SYMCRYPTO_COUNT                                   1 /** 1 SYMCRYPTOs available  */
#define SYSCFG_PRESENT                                      /** SYSCFG is available in this part */
#define SYSCFG_COUNT                                      1 /** 1 SYSCFGs available  */
#define SYSRTC_PRESENT                                      /** SYSRTC is available in this part */
#define SYSRTC_COUNT                                      1 /** 1 SYSRTCs available  */
#define TIMER_PRESENT                                       /** TIMER is available in this part */
#define TIMER_COUNT                                       4 /** 4 TIMERs available  */
#define ULFRCO_PRESENT                                      /** ULFRCO is available in this part */
#define ULFRCO_COUNT                                      1 /** 1 ULFRCOs available  */
#define WDOG_PRESENT                                        /** WDOG is available in this part */
#define WDOG_COUNT                                        2 /** 2 WDOGs available  */

/* Include standard ARM headers for the core */
#include "core_cm33.h"      /* Core Header File */
#include "system_sibg301.h" /* System Header File */

/** @} End of group SIBG301M104XIL_Part */

/**************************************************************************//**
 * @defgroup SIBG301M104XIL_Peripheral_TypeDefs SIBG301M104XIL Peripheral TypeDefs
 * @{
 * @brief Device Specific Peripheral Register Structures
 *****************************************************************************/
#include "sibg301_devinfo.h"
#include "sibg301_letimer.h"
#include "sibg301_adc.h"
#include "sibg301_acmp.h"
#include "sibg301_pcnt.h"
#include "sibg301_hfrco.h"
#include "sibg301_hfxo.h"
#include "sibg301_i2c.h"
#include "sibg301_wdog.h"
#include "sibg301_eusart.h"
#include "sibg301_socpll.h"
#include "sibg301_cmu.h"
#include "sibg301_hydraram.h"
#include "sibg301_ldma.h"
#include "sibg301_fsrco.h"
#include "sibg301_dpll.h"
#include "sibg301_lfxo.h"
#include "sibg301_lfrco.h"
#include "sibg301_ulfrco.h"
#include "sibg301_semaphore.h"
#include "sibg301_icache.h"
#include "sibg301_prs.h"
#include "sibg301_gpio.h"
#include "sibg301_ldmaxbar.h"
#include "sibg301_timer.h"
#include "sibg301_etampdet.h"
#include "sibg301_l2cache.h"
#include "sibg301_pixelrz.h"
#include "sibg301_burtc.h"
#include "sibg301_syscfg.h"
#include "sibg301_buram.h"
#include "sibg301_gpcrc.h"
#include "sibg301_rpa.h"
#include "sibg301_ksu.h"
#include "sibg301_symcrypto.h"
#include "sibg301_smu.h"
#include "sibg301_semailbox.h"
#include "sibg301_sysrtc.h"
#include "sibg301_scratchpad.h"
#include "sibg301_emu.h"
#include "sibg301_seportal.h"
#include "sibg301_lpw0portal.h"
#include "sibg301_hostportal.h"
#include "sibg301_devinfo_defines.h"

/* Custom headers for LDMAXBAR and PRS mappings */
#include "sibg301_prs_signals.h"
#include "sibg301_dma_descriptor.h"
#include "sibg301_ldmaxbar_defines.h"

/** @} End of group SIBG301M104XIL_Peripheral_TypeDefs  */

/**************************************************************************//**
 * @defgroup SIBG301M104XIL_Peripheral_Base SIBG301M104XIL Peripheral Memory Map
 * @{
 *****************************************************************************/

#define DEVINFO_S_BASE            (0x1FE08000UL) /* DEVINFO_S base address */
#define LETIMER0_S_BASE           (0x50000000UL) /* LETIMER0_S base address */
#define ADC0_S_BASE               (0x50004000UL) /* ADC0_S base address */
#define ACMP0_S_BASE              (0x50008000UL) /* ACMP0_S base address */
#define ACMP1_S_BASE              (0x5000C000UL) /* ACMP1_S base address */
#define PCNT0_S_BASE              (0x50040000UL) /* PCNT0_S base address */
#define HFRCOEM23_S_BASE          (0x50100000UL) /* HFRCOEM23_S base address */
#define HFXO0_S_BASE              (0x50104000UL) /* HFXO0_S base address */
#define I2C0_S_BASE               (0x50200000UL) /* I2C0_S base address */
#define WDOG0_S_BASE              (0x50204000UL) /* WDOG0_S base address */
#define EUSART0_S_BASE            (0x50208000UL) /* EUSART0_S base address */
#define SOCPLL0_S_BASE            (0x50800000UL) /* SOCPLL0_S base address */
#define CMU_S_BASE                (0x50804000UL) /* CMU_S base address */
#define DMEM_S_BASE               (0x50808000UL) /* DMEM_S base address */
#define HFRCO0_S_BASE             (0x50810000UL) /* HFRCO0_S base address */
#define LDMA0_S_BASE              (0x50814000UL) /* LDMA0_S base address */
#define FSRCO_S_BASE              (0x50818000UL) /* FSRCO_S base address */
#define DPLL0_S_BASE              (0x5081C000UL) /* DPLL0_S base address */
#define LFXO_S_BASE               (0x50820000UL) /* LFXO_S base address */
#define LFRCO_S_BASE              (0x50824000UL) /* LFRCO_S base address */
#define ULFRCO_S_BASE             (0x50828000UL) /* ULFRCO_S base address */
#define SEMAPHORE0_S_BASE         (0x5082C000UL) /* SEMAPHORE0_S base address */
#define SEMAPHORE1_S_BASE         (0x50830000UL) /* SEMAPHORE1_S base address */
#define L1ICACHE0_S_BASE          (0x50834000UL) /* L1ICACHE0_S base address */
#define PRS_S_BASE                (0x50838000UL) /* PRS_S base address */
#define GPIO_S_BASE               (0x5083C000UL) /* GPIO_S base address */
#define LDMAXBAR0_S_BASE          (0x50844000UL) /* LDMAXBAR0_S base address */
#define TIMER0_S_BASE             (0x50848000UL) /* TIMER0_S base address */
#define TIMER1_S_BASE             (0x5084C000UL) /* TIMER1_S base address */
#define TIMER2_S_BASE             (0x50850000UL) /* TIMER2_S base address */
#define TIMER3_S_BASE             (0x50854000UL) /* TIMER3_S base address */
#define ETAMPDET_S_BASE           (0x50858000UL) /* ETAMPDET_S base address */
#define L2ICACHE0_S_BASE          (0x5085C000UL) /* L2ICACHE0_S base address */
#define PIXELRZ0_S_BASE           (0x50860000UL) /* PIXELRZ0_S base address */
#define BURTC_S_BASE              (0x50864000UL) /* BURTC_S base address */
#define I2C1_S_BASE               (0x50868000UL) /* I2C1_S base address */
#define SYSCFG_S_CFGNS_BASE       (0x50870000UL) /* SYSCFG_S_CFGNS base address */
#define SYSCFG_S_BASE             (0x50874000UL) /* SYSCFG_S base address */
#define BURAM_S_BASE              (0x50878000UL) /* BURAM_S base address */
#define GPCRC0_S_BASE             (0x5087C000UL) /* GPCRC0_S base address */
#define EUSART1_S_BASE            (0x50884000UL) /* EUSART1_S base address */
#define EUSART2_S_BASE            (0x50888000UL) /* EUSART2_S base address */
#define PIXELRZ1_S_BASE           (0x5088C000UL) /* PIXELRZ1_S base address */
#define I2C2_S_BASE               (0x50890000UL) /* I2C2_S base address */
#define WDOG1_S_BASE              (0x50894000UL) /* WDOG1_S base address */
#define RPA_S_BASE                (0x50898000UL) /* RPA_S base address */
#define KSU_S_BASE                (0x5089C000UL) /* KSU_S base address */
#define SYMCRYPTO_S_BASE          (0x51000000UL) /* SYMCRYPTO_S base address */
#define LPWAES_S_BASE             (0x51004000UL) /* LPWAES_S base address */
#define SMU_S_BASE                (0x51008000UL) /* SMU_S base address */
#define SMU_S_CFGNS_BASE          (0x5100C000UL) /* SMU_S_CFGNS base address */
#define SEMAILBOX_S_HOST_BASE     (0x52800000UL) /* SEMAILBOX_S_HOST base address */
#define SYSRTC0_S_BASE            (0xB3000000UL) /* SYSRTC0_S base address */
#define SCRATCHPAD_S_BASE         (0xB3004000UL) /* SCRATCHPAD_S base address */
#define EMU_S_BASE                (0xB3008000UL) /* EMU_S base address */
#define EMU_S_CFGNS_BASE          (0xB300C000UL) /* EMU_S_CFGNS base address */
#define SEPORTAL_S_BASE           (0xB3010000UL) /* SEPORTAL_S base address */
#define LPW0PORTAL_S_BASE         (0xB3014000UL) /* LPW0PORTAL_S base address */
#define HOSTPORTAL_S_BASE         (0xB3018000UL) /* HOSTPORTAL_S base address */
#define DEVINFO_NS_BASE           (0x0FE08000UL) /* DEVINFO_NS base address */
#define LETIMER0_NS_BASE          (0x40000000UL) /* LETIMER0_NS base address */
#define ADC0_NS_BASE              (0x40004000UL) /* ADC0_NS base address */
#define ACMP0_NS_BASE             (0x40008000UL) /* ACMP0_NS base address */
#define ACMP1_NS_BASE             (0x4000C000UL) /* ACMP1_NS base address */
#define PCNT0_NS_BASE             (0x40040000UL) /* PCNT0_NS base address */
#define HFRCOEM23_NS_BASE         (0x40100000UL) /* HFRCOEM23_NS base address */
#define HFXO0_NS_BASE             (0x40104000UL) /* HFXO0_NS base address */
#define I2C0_NS_BASE              (0x40200000UL) /* I2C0_NS base address */
#define WDOG0_NS_BASE             (0x40204000UL) /* WDOG0_NS base address */
#define EUSART0_NS_BASE           (0x40208000UL) /* EUSART0_NS base address */
#define SOCPLL0_NS_BASE           (0x40800000UL) /* SOCPLL0_NS base address */
#define CMU_NS_BASE               (0x40804000UL) /* CMU_NS base address */
#define DMEM_NS_BASE              (0x40808000UL) /* DMEM_NS base address */
#define HFRCO0_NS_BASE            (0x40810000UL) /* HFRCO0_NS base address */
#define LDMA0_NS_BASE             (0x40814000UL) /* LDMA0_NS base address */
#define FSRCO_NS_BASE             (0x40818000UL) /* FSRCO_NS base address */
#define DPLL0_NS_BASE             (0x4081C000UL) /* DPLL0_NS base address */
#define LFXO_NS_BASE              (0x40820000UL) /* LFXO_NS base address */
#define LFRCO_NS_BASE             (0x40824000UL) /* LFRCO_NS base address */
#define ULFRCO_NS_BASE            (0x40828000UL) /* ULFRCO_NS base address */
#define SEMAPHORE0_NS_BASE        (0x4082C000UL) /* SEMAPHORE0_NS base address */
#define SEMAPHORE1_NS_BASE        (0x40830000UL) /* SEMAPHORE1_NS base address */
#define L1ICACHE0_NS_BASE         (0x40834000UL) /* L1ICACHE0_NS base address */
#define PRS_NS_BASE               (0x40838000UL) /* PRS_NS base address */
#define GPIO_NS_BASE              (0x4083C000UL) /* GPIO_NS base address */
#define LDMAXBAR0_NS_BASE         (0x40844000UL) /* LDMAXBAR0_NS base address */
#define TIMER0_NS_BASE            (0x40848000UL) /* TIMER0_NS base address */
#define TIMER1_NS_BASE            (0x4084C000UL) /* TIMER1_NS base address */
#define TIMER2_NS_BASE            (0x40850000UL) /* TIMER2_NS base address */
#define TIMER3_NS_BASE            (0x40854000UL) /* TIMER3_NS base address */
#define ETAMPDET_NS_BASE          (0x40858000UL) /* ETAMPDET_NS base address */
#define L2ICACHE0_NS_BASE         (0x4085C000UL) /* L2ICACHE0_NS base address */
#define PIXELRZ0_NS_BASE          (0x40860000UL) /* PIXELRZ0_NS base address */
#define BURTC_NS_BASE             (0x40864000UL) /* BURTC_NS base address */
#define I2C1_NS_BASE              (0x40868000UL) /* I2C1_NS base address */
#define SYSCFG_NS_CFGNS_BASE      (0x40870000UL) /* SYSCFG_NS_CFGNS base address */
#define SYSCFG_NS_BASE            (0x40874000UL) /* SYSCFG_NS base address */
#define BURAM_NS_BASE             (0x40878000UL) /* BURAM_NS base address */
#define GPCRC0_NS_BASE            (0x4087C000UL) /* GPCRC0_NS base address */
#define EUSART1_NS_BASE           (0x40884000UL) /* EUSART1_NS base address */
#define EUSART2_NS_BASE           (0x40888000UL) /* EUSART2_NS base address */
#define PIXELRZ1_NS_BASE          (0x4088C000UL) /* PIXELRZ1_NS base address */
#define I2C2_NS_BASE              (0x40890000UL) /* I2C2_NS base address */
#define WDOG1_NS_BASE             (0x40894000UL) /* WDOG1_NS base address */
#define RPA_NS_BASE               (0x40898000UL) /* RPA_NS base address */
#define KSU_NS_BASE               (0x4089C000UL) /* KSU_NS base address */
#define SYMCRYPTO_NS_BASE         (0x41000000UL) /* SYMCRYPTO_NS base address */
#define LPWAES_NS_BASE            (0x41004000UL) /* LPWAES_NS base address */
#define SMU_NS_BASE               (0x41008000UL) /* SMU_NS base address */
#define SMU_NS_CFGNS_BASE         (0x4100C000UL) /* SMU_NS_CFGNS base address */
#define SEMAILBOX_NS_HOST_BASE    (0x42800000UL) /* SEMAILBOX_NS_HOST base address */
#define SYSRTC0_NS_BASE           (0xA3000000UL) /* SYSRTC0_NS base address */
#define SCRATCHPAD_NS_BASE        (0xA3004000UL) /* SCRATCHPAD_NS base address */
#define EMU_NS_BASE               (0xA3008000UL) /* EMU_NS base address */
#define EMU_NS_CFGNS_BASE         (0xA300C000UL) /* EMU_NS_CFGNS base address */
#define SEPORTAL_NS_BASE          (0xA3010000UL) /* SEPORTAL_NS base address */
#define LPW0PORTAL_NS_BASE        (0xA3014000UL) /* LPW0PORTAL_NS base address */
#define HOSTPORTAL_NS_BASE        (0xA3018000UL) /* HOSTPORTAL_NS base address */

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"

#endif
#if defined(SL_CATALOG_TRUSTZONE_SECURE_CONFIG_PRESENT)
#include "sl_trustzone_secure_config.h"

#endif

#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_DEVINFO_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_DEVINFO_S) && (SL_TRUSTZONE_PERIPHERAL_DEVINFO_S != 0)))
#define DEVINFO_BASE           (DEVINFO_S_BASE)              /* DEVINFO base address */
#else
#define DEVINFO_BASE           (DEVINFO_NS_BASE)             /* DEVINFO base address */
#endif // SL_TRUSTZONE_PERIPHERAL_DEVINFO_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_LETIMER0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_LETIMER0_S) && (SL_TRUSTZONE_PERIPHERAL_LETIMER0_S != 0)))
#define LETIMER0_BASE          (LETIMER0_S_BASE)             /* LETIMER0 base address */
#else
#define LETIMER0_BASE          (LETIMER0_NS_BASE)            /* LETIMER0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_LETIMER0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_ADC0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_ADC0_S) && (SL_TRUSTZONE_PERIPHERAL_ADC0_S != 0)))
#define ADC0_BASE              (ADC0_S_BASE)                 /* ADC0 base address */
#else
#define ADC0_BASE              (ADC0_NS_BASE)                /* ADC0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_ADC0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_ACMP0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_ACMP0_S) && (SL_TRUSTZONE_PERIPHERAL_ACMP0_S != 0)))
#define ACMP0_BASE             (ACMP0_S_BASE)                /* ACMP0 base address */
#else
#define ACMP0_BASE             (ACMP0_NS_BASE)               /* ACMP0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_ACMP0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_ACMP1_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_ACMP1_S) && (SL_TRUSTZONE_PERIPHERAL_ACMP1_S != 0)))
#define ACMP1_BASE             (ACMP1_S_BASE)                /* ACMP1 base address */
#else
#define ACMP1_BASE             (ACMP1_NS_BASE)               /* ACMP1 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_ACMP1_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_PCNT0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_PCNT0_S) && (SL_TRUSTZONE_PERIPHERAL_PCNT0_S != 0)))
#define PCNT0_BASE             (PCNT0_S_BASE)                /* PCNT0 base address */
#else
#define PCNT0_BASE             (PCNT0_NS_BASE)               /* PCNT0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_PCNT0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_HFRCOEM23_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_HFRCOEM23_S) && (SL_TRUSTZONE_PERIPHERAL_HFRCOEM23_S != 0)))
#define HFRCOEM23_BASE         (HFRCOEM23_S_BASE)            /* HFRCOEM23 base address */
#else
#define HFRCOEM23_BASE         (HFRCOEM23_NS_BASE)           /* HFRCOEM23 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_HFRCOEM23_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_HFXO0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_HFXO0_S) && (SL_TRUSTZONE_PERIPHERAL_HFXO0_S != 0)))
#define HFXO0_BASE             (HFXO0_S_BASE)                /* HFXO0 base address */
#else
#define HFXO0_BASE             (HFXO0_NS_BASE)               /* HFXO0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_HFXO0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_I2C0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_I2C0_S) && (SL_TRUSTZONE_PERIPHERAL_I2C0_S != 0)))
#define I2C0_BASE              (I2C0_S_BASE)                 /* I2C0 base address */
#else
#define I2C0_BASE              (I2C0_NS_BASE)                /* I2C0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_I2C0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_WDOG0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_WDOG0_S) && (SL_TRUSTZONE_PERIPHERAL_WDOG0_S != 0)))
#define WDOG0_BASE             (WDOG0_S_BASE)                /* WDOG0 base address */
#else
#define WDOG0_BASE             (WDOG0_NS_BASE)               /* WDOG0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_WDOG0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_EUSART0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_EUSART0_S) && (SL_TRUSTZONE_PERIPHERAL_EUSART0_S != 0)))
#define EUSART0_BASE           (EUSART0_S_BASE)              /* EUSART0 base address */
#else
#define EUSART0_BASE           (EUSART0_NS_BASE)             /* EUSART0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_EUSART0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_SOCPLL0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_SOCPLL0_S) && (SL_TRUSTZONE_PERIPHERAL_SOCPLL0_S != 0)))
#define SOCPLL0_BASE           (SOCPLL0_S_BASE)              /* SOCPLL0 base address */
#else
#define SOCPLL0_BASE           (SOCPLL0_NS_BASE)             /* SOCPLL0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_SOCPLL0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_CMU_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_CMU_S) && (SL_TRUSTZONE_PERIPHERAL_CMU_S != 0)))
#define CMU_BASE               (CMU_S_BASE)                  /* CMU base address */
#else
#define CMU_BASE               (CMU_NS_BASE)                 /* CMU base address */
#endif // SL_TRUSTZONE_PERIPHERAL_CMU_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_DMEM_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_DMEM_S) && (SL_TRUSTZONE_PERIPHERAL_DMEM_S != 0)))
#define DMEM_BASE              (DMEM_S_BASE)                 /* DMEM base address */
#else
#define DMEM_BASE              (DMEM_NS_BASE)                /* DMEM base address */
#endif // SL_TRUSTZONE_PERIPHERAL_DMEM_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_HFRCO0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_HFRCO0_S) && (SL_TRUSTZONE_PERIPHERAL_HFRCO0_S != 0)))
#define HFRCO0_BASE            (HFRCO0_S_BASE)               /* HFRCO0 base address */
#else
#define HFRCO0_BASE            (HFRCO0_NS_BASE)              /* HFRCO0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_HFRCO0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_LDMA0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_LDMA0_S) && (SL_TRUSTZONE_PERIPHERAL_LDMA0_S != 0)))
#define LDMA0_BASE             (LDMA0_S_BASE)                /* LDMA0 base address */
#else
#define LDMA0_BASE             (LDMA0_NS_BASE)               /* LDMA0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_LDMA0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_FSRCO_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_FSRCO_S) && (SL_TRUSTZONE_PERIPHERAL_FSRCO_S != 0)))
#define FSRCO_BASE             (FSRCO_S_BASE)                /* FSRCO base address */
#else
#define FSRCO_BASE             (FSRCO_NS_BASE)               /* FSRCO base address */
#endif // SL_TRUSTZONE_PERIPHERAL_FSRCO_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_DPLL0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_DPLL0_S) && (SL_TRUSTZONE_PERIPHERAL_DPLL0_S != 0)))
#define DPLL0_BASE             (DPLL0_S_BASE)                /* DPLL0 base address */
#else
#define DPLL0_BASE             (DPLL0_NS_BASE)               /* DPLL0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_DPLL0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_LFXO_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_LFXO_S) && (SL_TRUSTZONE_PERIPHERAL_LFXO_S != 0)))
#define LFXO_BASE              (LFXO_S_BASE)                 /* LFXO base address */
#else
#define LFXO_BASE              (LFXO_NS_BASE)                /* LFXO base address */
#endif // SL_TRUSTZONE_PERIPHERAL_LFXO_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_LFRCO_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_LFRCO_S) && (SL_TRUSTZONE_PERIPHERAL_LFRCO_S != 0)))
#define LFRCO_BASE             (LFRCO_S_BASE)                /* LFRCO base address */
#else
#define LFRCO_BASE             (LFRCO_NS_BASE)               /* LFRCO base address */
#endif // SL_TRUSTZONE_PERIPHERAL_LFRCO_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_ULFRCO_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_ULFRCO_S) && (SL_TRUSTZONE_PERIPHERAL_ULFRCO_S != 0)))
#define ULFRCO_BASE            (ULFRCO_S_BASE)               /* ULFRCO base address */
#else
#define ULFRCO_BASE            (ULFRCO_NS_BASE)              /* ULFRCO base address */
#endif // SL_TRUSTZONE_PERIPHERAL_ULFRCO_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_SEMAPHORE0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_SEMAPHORE0_S) && (SL_TRUSTZONE_PERIPHERAL_SEMAPHORE0_S != 0)))
#define SEMAPHORE0_BASE        (SEMAPHORE0_S_BASE)           /* SEMAPHORE0 base address */
#else
#define SEMAPHORE0_BASE        (SEMAPHORE0_NS_BASE)          /* SEMAPHORE0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_SEMAPHORE0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_SEMAPHORE1_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_SEMAPHORE1_S) && (SL_TRUSTZONE_PERIPHERAL_SEMAPHORE1_S != 0)))
#define SEMAPHORE1_BASE        (SEMAPHORE1_S_BASE)           /* SEMAPHORE1 base address */
#else
#define SEMAPHORE1_BASE        (SEMAPHORE1_NS_BASE)          /* SEMAPHORE1 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_SEMAPHORE1_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_L1ICACHE0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_L1ICACHE0_S) && (SL_TRUSTZONE_PERIPHERAL_L1ICACHE0_S != 0)))
#define L1ICACHE0_BASE         (L1ICACHE0_S_BASE)            /* L1ICACHE0 base address */
#else
#define L1ICACHE0_BASE         (L1ICACHE0_NS_BASE)           /* L1ICACHE0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_L1ICACHE0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_PRS_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_PRS_S) && (SL_TRUSTZONE_PERIPHERAL_PRS_S != 0)))
#define PRS_BASE               (PRS_S_BASE)                  /* PRS base address */
#else
#define PRS_BASE               (PRS_NS_BASE)                 /* PRS base address */
#endif // SL_TRUSTZONE_PERIPHERAL_PRS_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_GPIO_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_GPIO_S) && (SL_TRUSTZONE_PERIPHERAL_GPIO_S != 0)))
#define GPIO_BASE              (GPIO_S_BASE)                 /* GPIO base address */
#else
#define GPIO_BASE              (GPIO_NS_BASE)                /* GPIO base address */
#endif // SL_TRUSTZONE_PERIPHERAL_GPIO_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_LDMAXBAR0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_LDMAXBAR0_S) && (SL_TRUSTZONE_PERIPHERAL_LDMAXBAR0_S != 0)))
#define LDMAXBAR0_BASE         (LDMAXBAR0_S_BASE)            /* LDMAXBAR0 base address */
#else
#define LDMAXBAR0_BASE         (LDMAXBAR0_NS_BASE)           /* LDMAXBAR0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_LDMAXBAR0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_TIMER0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_TIMER0_S) && (SL_TRUSTZONE_PERIPHERAL_TIMER0_S != 0)))
#define TIMER0_BASE            (TIMER0_S_BASE)               /* TIMER0 base address */
#else
#define TIMER0_BASE            (TIMER0_NS_BASE)              /* TIMER0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_TIMER0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_TIMER1_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_TIMER1_S) && (SL_TRUSTZONE_PERIPHERAL_TIMER1_S != 0)))
#define TIMER1_BASE            (TIMER1_S_BASE)               /* TIMER1 base address */
#else
#define TIMER1_BASE            (TIMER1_NS_BASE)              /* TIMER1 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_TIMER1_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_TIMER2_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_TIMER2_S) && (SL_TRUSTZONE_PERIPHERAL_TIMER2_S != 0)))
#define TIMER2_BASE            (TIMER2_S_BASE)               /* TIMER2 base address */
#else
#define TIMER2_BASE            (TIMER2_NS_BASE)              /* TIMER2 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_TIMER2_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_TIMER3_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_TIMER3_S) && (SL_TRUSTZONE_PERIPHERAL_TIMER3_S != 0)))
#define TIMER3_BASE            (TIMER3_S_BASE)               /* TIMER3 base address */
#else
#define TIMER3_BASE            (TIMER3_NS_BASE)              /* TIMER3 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_TIMER3_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_ETAMPDET_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_ETAMPDET_S) && (SL_TRUSTZONE_PERIPHERAL_ETAMPDET_S != 0)))
#define ETAMPDET_BASE          (ETAMPDET_S_BASE)             /* ETAMPDET base address */
#else
#define ETAMPDET_BASE          (ETAMPDET_NS_BASE)            /* ETAMPDET base address */
#endif // SL_TRUSTZONE_PERIPHERAL_ETAMPDET_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_L2ICACHE0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_L2ICACHE0_S) && (SL_TRUSTZONE_PERIPHERAL_L2ICACHE0_S != 0)))
#define L2ICACHE0_BASE         (L2ICACHE0_S_BASE)            /* L2ICACHE0 base address */
#else
#define L2ICACHE0_BASE         (L2ICACHE0_NS_BASE)           /* L2ICACHE0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_L2ICACHE0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_PIXELRZ0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_PIXELRZ0_S) && (SL_TRUSTZONE_PERIPHERAL_PIXELRZ0_S != 0)))
#define PIXELRZ0_BASE          (PIXELRZ0_S_BASE)             /* PIXELRZ0 base address */
#else
#define PIXELRZ0_BASE          (PIXELRZ0_NS_BASE)            /* PIXELRZ0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_PIXELRZ0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_BURTC_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_BURTC_S) && (SL_TRUSTZONE_PERIPHERAL_BURTC_S != 0)))
#define BURTC_BASE             (BURTC_S_BASE)                /* BURTC base address */
#else
#define BURTC_BASE             (BURTC_NS_BASE)               /* BURTC base address */
#endif // SL_TRUSTZONE_PERIPHERAL_BURTC_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_I2C1_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_I2C1_S) && (SL_TRUSTZONE_PERIPHERAL_I2C1_S != 0)))
#define I2C1_BASE              (I2C1_S_BASE)                 /* I2C1 base address */
#else
#define I2C1_BASE              (I2C1_NS_BASE)                /* I2C1 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_I2C1_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_SYSCFG_CFGNS_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_SYSCFG_CFGNS_S) && (SL_TRUSTZONE_PERIPHERAL_SYSCFG_CFGNS_S != 0)))
#define SYSCFG_CFGNS_BASE      (SYSCFG_S_CFGNS_BASE)         /* SYSCFG_CFGNS base address */
#else
#define SYSCFG_CFGNS_BASE      (SYSCFG_NS_CFGNS_BASE)        /* SYSCFG_CFGNS base address */
#endif // SL_TRUSTZONE_PERIPHERAL_SYSCFG_CFGNS_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_SYSCFG_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_SYSCFG_S) && (SL_TRUSTZONE_PERIPHERAL_SYSCFG_S != 0)))
#define SYSCFG_BASE            (SYSCFG_S_BASE)               /* SYSCFG base address */
#else
#define SYSCFG_BASE            (SYSCFG_NS_BASE)              /* SYSCFG base address */
#endif // SL_TRUSTZONE_PERIPHERAL_SYSCFG_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_BURAM_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_BURAM_S) && (SL_TRUSTZONE_PERIPHERAL_BURAM_S != 0)))
#define BURAM_BASE             (BURAM_S_BASE)                /* BURAM base address */
#else
#define BURAM_BASE             (BURAM_NS_BASE)               /* BURAM base address */
#endif // SL_TRUSTZONE_PERIPHERAL_BURAM_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_GPCRC0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_GPCRC0_S) && (SL_TRUSTZONE_PERIPHERAL_GPCRC0_S != 0)))
#define GPCRC0_BASE            (GPCRC0_S_BASE)               /* GPCRC0 base address */
#else
#define GPCRC0_BASE            (GPCRC0_NS_BASE)              /* GPCRC0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_GPCRC0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_EUSART1_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_EUSART1_S) && (SL_TRUSTZONE_PERIPHERAL_EUSART1_S != 0)))
#define EUSART1_BASE           (EUSART1_S_BASE)              /* EUSART1 base address */
#else
#define EUSART1_BASE           (EUSART1_NS_BASE)             /* EUSART1 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_EUSART1_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_EUSART2_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_EUSART2_S) && (SL_TRUSTZONE_PERIPHERAL_EUSART2_S != 0)))
#define EUSART2_BASE           (EUSART2_S_BASE)              /* EUSART2 base address */
#else
#define EUSART2_BASE           (EUSART2_NS_BASE)             /* EUSART2 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_EUSART2_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_PIXELRZ1_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_PIXELRZ1_S) && (SL_TRUSTZONE_PERIPHERAL_PIXELRZ1_S != 0)))
#define PIXELRZ1_BASE          (PIXELRZ1_S_BASE)             /* PIXELRZ1 base address */
#else
#define PIXELRZ1_BASE          (PIXELRZ1_NS_BASE)            /* PIXELRZ1 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_PIXELRZ1_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_I2C2_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_I2C2_S) && (SL_TRUSTZONE_PERIPHERAL_I2C2_S != 0)))
#define I2C2_BASE              (I2C2_S_BASE)                 /* I2C2 base address */
#else
#define I2C2_BASE              (I2C2_NS_BASE)                /* I2C2 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_I2C2_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_WDOG1_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_WDOG1_S) && (SL_TRUSTZONE_PERIPHERAL_WDOG1_S != 0)))
#define WDOG1_BASE             (WDOG1_S_BASE)                /* WDOG1 base address */
#else
#define WDOG1_BASE             (WDOG1_NS_BASE)               /* WDOG1 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_WDOG1_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_RPA_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_RPA_S) && (SL_TRUSTZONE_PERIPHERAL_RPA_S != 0)))
#define RPA_BASE               (RPA_S_BASE)                  /* RPA base address */
#else
#define RPA_BASE               (RPA_NS_BASE)                 /* RPA base address */
#endif // SL_TRUSTZONE_PERIPHERAL_RPA_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_KSU_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_KSU_S) && (SL_TRUSTZONE_PERIPHERAL_KSU_S != 0)))
#define KSU_BASE               (KSU_S_BASE)                  /* KSU base address */
#else
#define KSU_BASE               (KSU_NS_BASE)                 /* KSU base address */
#endif // SL_TRUSTZONE_PERIPHERAL_KSU_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_SYMCRYPTO_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_SYMCRYPTO_S) && (SL_TRUSTZONE_PERIPHERAL_SYMCRYPTO_S != 0)))
#define SYMCRYPTO_BASE         (SYMCRYPTO_S_BASE)            /* SYMCRYPTO base address */
#else
#define SYMCRYPTO_BASE         (SYMCRYPTO_NS_BASE)           /* SYMCRYPTO base address */
#endif // SL_TRUSTZONE_PERIPHERAL_SYMCRYPTO_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_LPWAES_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_LPWAES_S) && (SL_TRUSTZONE_PERIPHERAL_LPWAES_S != 0)))
#define LPWAES_BASE            (LPWAES_S_BASE)               /* LPWAES base address */
#else
#define LPWAES_BASE            (LPWAES_NS_BASE)              /* LPWAES base address */
#endif // SL_TRUSTZONE_PERIPHERAL_LPWAES_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_SMU_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_SMU_S) && (SL_TRUSTZONE_PERIPHERAL_SMU_S != 0)))
#define SMU_BASE               (SMU_S_BASE)                  /* SMU base address */
#else
#define SMU_BASE               (SMU_S_BASE)                  /* SMU base address */
#endif // SL_TRUSTZONE_PERIPHERAL_SMU_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_SMU_CFGNS_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_SMU_CFGNS_S) && (SL_TRUSTZONE_PERIPHERAL_SMU_CFGNS_S != 0)))
#define SMU_CFGNS_BASE         (SMU_S_CFGNS_BASE)            /* SMU_CFGNS base address */
#else
#define SMU_CFGNS_BASE         (SMU_NS_CFGNS_BASE)           /* SMU_CFGNS base address */
#endif // SL_TRUSTZONE_PERIPHERAL_SMU_CFGNS_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_SEMAILBOX_HOST_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_SEMAILBOX_HOST_S) && (SL_TRUSTZONE_PERIPHERAL_SEMAILBOX_HOST_S != 0)))
#define SEMAILBOX_HOST_BASE    (SEMAILBOX_S_HOST_BASE)       /* SEMAILBOX_HOST base address */
#else
#define SEMAILBOX_HOST_BASE    (SEMAILBOX_NS_HOST_BASE)      /* SEMAILBOX_HOST base address */
#endif // SL_TRUSTZONE_PERIPHERAL_SEMAILBOX_HOST_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_SYSRTC0_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_SYSRTC0_S) && (SL_TRUSTZONE_PERIPHERAL_SYSRTC0_S != 0)))
#define SYSRTC0_BASE           (SYSRTC0_S_BASE)              /* SYSRTC0 base address */
#else
#define SYSRTC0_BASE           (SYSRTC0_NS_BASE)             /* SYSRTC0 base address */
#endif // SL_TRUSTZONE_PERIPHERAL_SYSRTC0_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_SCRATCHPAD_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_SCRATCHPAD_S) && (SL_TRUSTZONE_PERIPHERAL_SCRATCHPAD_S != 0)))
#define SCRATCHPAD_BASE        (SCRATCHPAD_S_BASE)           /* SCRATCHPAD base address */
#else
#define SCRATCHPAD_BASE        (SCRATCHPAD_NS_BASE)          /* SCRATCHPAD base address */
#endif // SL_TRUSTZONE_PERIPHERAL_SCRATCHPAD_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_EMU_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_EMU_S) && (SL_TRUSTZONE_PERIPHERAL_EMU_S != 0)))
#define EMU_BASE               (EMU_S_BASE)                  /* EMU base address */
#else
#define EMU_BASE               (EMU_NS_BASE)                 /* EMU base address */
#endif // SL_TRUSTZONE_PERIPHERAL_EMU_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_EMU_CFGNS_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_EMU_CFGNS_S) && (SL_TRUSTZONE_PERIPHERAL_EMU_CFGNS_S != 0)))
#define EMU_CFGNS_BASE         (EMU_S_CFGNS_BASE)            /* EMU_CFGNS base address */
#else
#define EMU_CFGNS_BASE         (EMU_NS_CFGNS_BASE)           /* EMU_CFGNS base address */
#endif // SL_TRUSTZONE_PERIPHERAL_EMU_CFGNS_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_SEPORTAL_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_SEPORTAL_S) && (SL_TRUSTZONE_PERIPHERAL_SEPORTAL_S != 0)))
#define SEPORTAL_BASE          (SEPORTAL_S_BASE)             /* SEPORTAL base address */
#else
#define SEPORTAL_BASE          (SEPORTAL_NS_BASE)            /* SEPORTAL base address */
#endif // SL_TRUSTZONE_PERIPHERAL_SEPORTAL_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_LPW0PORTAL_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_LPW0PORTAL_S) && (SL_TRUSTZONE_PERIPHERAL_LPW0PORTAL_S != 0)))
#define LPW0PORTAL_BASE        (LPW0PORTAL_S_BASE)           /* LPW0PORTAL base address */
#else
#define LPW0PORTAL_BASE        (LPW0PORTAL_NS_BASE)          /* LPW0PORTAL base address */
#endif // SL_TRUSTZONE_PERIPHERAL_LPW0PORTAL_S
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_HOSTPORTAL_S)) || (defined(SL_TRUSTZONE_PERIPHERAL_HOSTPORTAL_S) && (SL_TRUSTZONE_PERIPHERAL_HOSTPORTAL_S != 0)))
#define HOSTPORTAL_BASE        (HOSTPORTAL_S_BASE)           /* HOSTPORTAL base address */
#else
#define HOSTPORTAL_BASE        (HOSTPORTAL_NS_BASE)          /* HOSTPORTAL base address */
#endif // SL_TRUSTZONE_PERIPHERAL_HOSTPORTAL_S

/** @} End of group SIBG301M104XIL_Peripheral_Base */

/**************************************************************************//**
 * @defgroup SIBG301M104XIL_Peripheral_Declaration SIBG301M104XIL Peripheral Declarations Map
 * @{
 *****************************************************************************/

#define DEVINFO_S            ((DEVINFO_TypeDef *) DEVINFO_S_BASE)                /**< DEVINFO_S base pointer */
#define LETIMER0_S           ((LETIMER_TypeDef *) LETIMER0_S_BASE)               /**< LETIMER0_S base pointer */
#define ADC0_S               ((ADC_TypeDef *) ADC0_S_BASE)                       /**< ADC0_S base pointer */
#define ACMP0_S              ((ACMP_TypeDef *) ACMP0_S_BASE)                     /**< ACMP0_S base pointer */
#define ACMP1_S              ((ACMP_TypeDef *) ACMP1_S_BASE)                     /**< ACMP1_S base pointer */
#define PCNT0_S              ((PCNT_TypeDef *) PCNT0_S_BASE)                     /**< PCNT0_S base pointer */
#define HFRCOEM23_S          ((HFRCO_TypeDef *) HFRCOEM23_S_BASE)                /**< HFRCOEM23_S base pointer */
#define HFXO0_S              ((HFXO_TypeDef *) HFXO0_S_BASE)                     /**< HFXO0_S base pointer */
#define I2C0_S               ((I2C_TypeDef *) I2C0_S_BASE)                       /**< I2C0_S base pointer */
#define WDOG0_S              ((WDOG_TypeDef *) WDOG0_S_BASE)                     /**< WDOG0_S base pointer */
#define EUSART0_S            ((EUSART_TypeDef *) EUSART0_S_BASE)                 /**< EUSART0_S base pointer */
#define SOCPLL0_S            ((SOCPLL_TypeDef *) SOCPLL0_S_BASE)                 /**< SOCPLL0_S base pointer */
#define CMU_S                ((CMU_TypeDef *) CMU_S_BASE)                        /**< CMU_S base pointer */
#define DMEM_S               ((HYDRARAM_TypeDef *) DMEM_S_BASE)                  /**< DMEM_S base pointer */
#define HFRCO0_S             ((HFRCO_TypeDef *) HFRCO0_S_BASE)                   /**< HFRCO0_S base pointer */
#define LDMA0_S              ((LDMA_TypeDef *) LDMA0_S_BASE)                     /**< LDMA0_S base pointer */
#define FSRCO_S              ((FSRCO_TypeDef *) FSRCO_S_BASE)                    /**< FSRCO_S base pointer */
#define DPLL0_S              ((DPLL_TypeDef *) DPLL0_S_BASE)                     /**< DPLL0_S base pointer */
#define LFXO_S               ((LFXO_TypeDef *) LFXO_S_BASE)                      /**< LFXO_S base pointer */
#define LFRCO_S              ((LFRCO_TypeDef *) LFRCO_S_BASE)                    /**< LFRCO_S base pointer */
#define ULFRCO_S             ((ULFRCO_TypeDef *) ULFRCO_S_BASE)                  /**< ULFRCO_S base pointer */
#define SEMAPHORE0_S         ((SEMAPHORE_TypeDef *) SEMAPHORE0_S_BASE)           /**< SEMAPHORE0_S base pointer */
#define SEMAPHORE1_S         ((SEMAPHORE_TypeDef *) SEMAPHORE1_S_BASE)           /**< SEMAPHORE1_S base pointer */
#define L1ICACHE0_S          ((ICACHE_TypeDef *) L1ICACHE0_S_BASE)               /**< L1ICACHE0_S base pointer */
#define PRS_S                ((PRS_TypeDef *) PRS_S_BASE)                        /**< PRS_S base pointer */
#define GPIO_S               ((GPIO_TypeDef *) GPIO_S_BASE)                      /**< GPIO_S base pointer */
#define LDMAXBAR0_S          ((LDMAXBAR_TypeDef *) LDMAXBAR0_S_BASE)             /**< LDMAXBAR0_S base pointer */
#define TIMER0_S             ((TIMER_TypeDef *) TIMER0_S_BASE)                   /**< TIMER0_S base pointer */
#define TIMER1_S             ((TIMER_TypeDef *) TIMER1_S_BASE)                   /**< TIMER1_S base pointer */
#define TIMER2_S             ((TIMER_TypeDef *) TIMER2_S_BASE)                   /**< TIMER2_S base pointer */
#define TIMER3_S             ((TIMER_TypeDef *) TIMER3_S_BASE)                   /**< TIMER3_S base pointer */
#define ETAMPDET_S           ((ETAMPDET_TypeDef *) ETAMPDET_S_BASE)              /**< ETAMPDET_S base pointer */
#define L2ICACHE0_S          ((L2CACHE_TypeDef *) L2ICACHE0_S_BASE)              /**< L2ICACHE0_S base pointer */
#define PIXELRZ0_S           ((PIXELRZ_TypeDef *) PIXELRZ0_S_BASE)               /**< PIXELRZ0_S base pointer */
#define BURTC_S              ((BURTC_TypeDef *) BURTC_S_BASE)                    /**< BURTC_S base pointer */
#define I2C1_S               ((I2C_TypeDef *) I2C1_S_BASE)                       /**< I2C1_S base pointer */
#define SYSCFG_S_CFGNS       ((SYSCFG_CFGNS_TypeDef *) SYSCFG_S_CFGNS_BASE)      /**< SYSCFG_S_CFGNS base pointer */
#define SYSCFG_S             ((SYSCFG_TypeDef *) SYSCFG_S_BASE)                  /**< SYSCFG_S base pointer */
#define BURAM_S              ((BURAM_TypeDef *) BURAM_S_BASE)                    /**< BURAM_S base pointer */
#define GPCRC0_S             ((GPCRC_TypeDef *) GPCRC0_S_BASE)                   /**< GPCRC0_S base pointer */
#define EUSART1_S            ((EUSART_TypeDef *) EUSART1_S_BASE)                 /**< EUSART1_S base pointer */
#define EUSART2_S            ((EUSART_TypeDef *) EUSART2_S_BASE)                 /**< EUSART2_S base pointer */
#define PIXELRZ1_S           ((PIXELRZ_TypeDef *) PIXELRZ1_S_BASE)               /**< PIXELRZ1_S base pointer */
#define I2C2_S               ((I2C_TypeDef *) I2C2_S_BASE)                       /**< I2C2_S base pointer */
#define WDOG1_S              ((WDOG_TypeDef *) WDOG1_S_BASE)                     /**< WDOG1_S base pointer */
#define RPA_S                ((RPA_TypeDef *) RPA_S_BASE)                        /**< RPA_S base pointer */
#define KSU_S                ((KSU_TypeDef *) KSU_S_BASE)                        /**< KSU_S base pointer */
#define SYMCRYPTO_S          ((SYMCRYPTO_TypeDef *) SYMCRYPTO_S_BASE)            /**< SYMCRYPTO_S base pointer */
#define LPWAES_S             ((SYMCRYPTO_TypeDef *) LPWAES_S_BASE)               /**< LPWAES_S base pointer */
#define SMU_S                ((SMU_TypeDef *) SMU_S_BASE)                        /**< SMU_S base pointer */
#define SMU_S_CFGNS          ((SMU_CFGNS_TypeDef *) SMU_S_CFGNS_BASE)            /**< SMU_S_CFGNS base pointer */
#define SEMAILBOX_S_HOST     ((SEMAILBOX_HOST_TypeDef *) SEMAILBOX_S_HOST_BASE)  /**< SEMAILBOX_S_HOST base pointer */
#define SYSRTC0_S            ((SYSRTC_TypeDef *) SYSRTC0_S_BASE)                 /**< SYSRTC0_S base pointer */
#define SCRATCHPAD_S         ((SCRATCHPAD_TypeDef *) SCRATCHPAD_S_BASE)          /**< SCRATCHPAD_S base pointer */
#define EMU_S                ((EMU_TypeDef *) EMU_S_BASE)                        /**< EMU_S base pointer */
#define EMU_S_CFGNS          ((EMU_CFGNS_TypeDef *) EMU_S_CFGNS_BASE)            /**< EMU_S_CFGNS base pointer */
#define SEPORTAL_S           ((SEPORTAL_TypeDef *) SEPORTAL_S_BASE)              /**< SEPORTAL_S base pointer */
#define LPW0PORTAL_S         ((LPW0PORTAL_TypeDef *) LPW0PORTAL_S_BASE)          /**< LPW0PORTAL_S base pointer */
#define HOSTPORTAL_S         ((HOSTPORTAL_TypeDef *) HOSTPORTAL_S_BASE)          /**< HOSTPORTAL_S base pointer */
#define DEVINFO_NS           ((DEVINFO_TypeDef *) DEVINFO_NS_BASE)               /**< DEVINFO_NS base pointer */
#define LETIMER0_NS          ((LETIMER_TypeDef *) LETIMER0_NS_BASE)              /**< LETIMER0_NS base pointer */
#define ADC0_NS              ((ADC_TypeDef *) ADC0_NS_BASE)                      /**< ADC0_NS base pointer */
#define ACMP0_NS             ((ACMP_TypeDef *) ACMP0_NS_BASE)                    /**< ACMP0_NS base pointer */
#define ACMP1_NS             ((ACMP_TypeDef *) ACMP1_NS_BASE)                    /**< ACMP1_NS base pointer */
#define PCNT0_NS             ((PCNT_TypeDef *) PCNT0_NS_BASE)                    /**< PCNT0_NS base pointer */
#define HFRCOEM23_NS         ((HFRCO_TypeDef *) HFRCOEM23_NS_BASE)               /**< HFRCOEM23_NS base pointer */
#define HFXO0_NS             ((HFXO_TypeDef *) HFXO0_NS_BASE)                    /**< HFXO0_NS base pointer */
#define I2C0_NS              ((I2C_TypeDef *) I2C0_NS_BASE)                      /**< I2C0_NS base pointer */
#define WDOG0_NS             ((WDOG_TypeDef *) WDOG0_NS_BASE)                    /**< WDOG0_NS base pointer */
#define EUSART0_NS           ((EUSART_TypeDef *) EUSART0_NS_BASE)                /**< EUSART0_NS base pointer */
#define SOCPLL0_NS           ((SOCPLL_TypeDef *) SOCPLL0_NS_BASE)                /**< SOCPLL0_NS base pointer */
#define CMU_NS               ((CMU_TypeDef *) CMU_NS_BASE)                       /**< CMU_NS base pointer */
#define DMEM_NS              ((HYDRARAM_TypeDef *) DMEM_NS_BASE)                 /**< DMEM_NS base pointer */
#define HFRCO0_NS            ((HFRCO_TypeDef *) HFRCO0_NS_BASE)                  /**< HFRCO0_NS base pointer */
#define LDMA0_NS             ((LDMA_TypeDef *) LDMA0_NS_BASE)                    /**< LDMA0_NS base pointer */
#define FSRCO_NS             ((FSRCO_TypeDef *) FSRCO_NS_BASE)                   /**< FSRCO_NS base pointer */
#define DPLL0_NS             ((DPLL_TypeDef *) DPLL0_NS_BASE)                    /**< DPLL0_NS base pointer */
#define LFXO_NS              ((LFXO_TypeDef *) LFXO_NS_BASE)                     /**< LFXO_NS base pointer */
#define LFRCO_NS             ((LFRCO_TypeDef *) LFRCO_NS_BASE)                   /**< LFRCO_NS base pointer */
#define ULFRCO_NS            ((ULFRCO_TypeDef *) ULFRCO_NS_BASE)                 /**< ULFRCO_NS base pointer */
#define SEMAPHORE0_NS        ((SEMAPHORE_TypeDef *) SEMAPHORE0_NS_BASE)          /**< SEMAPHORE0_NS base pointer */
#define SEMAPHORE1_NS        ((SEMAPHORE_TypeDef *) SEMAPHORE1_NS_BASE)          /**< SEMAPHORE1_NS base pointer */
#define L1ICACHE0_NS         ((ICACHE_TypeDef *) L1ICACHE0_NS_BASE)              /**< L1ICACHE0_NS base pointer */
#define PRS_NS               ((PRS_TypeDef *) PRS_NS_BASE)                       /**< PRS_NS base pointer */
#define GPIO_NS              ((GPIO_TypeDef *) GPIO_NS_BASE)                     /**< GPIO_NS base pointer */
#define LDMAXBAR0_NS         ((LDMAXBAR_TypeDef *) LDMAXBAR0_NS_BASE)            /**< LDMAXBAR0_NS base pointer */
#define TIMER0_NS            ((TIMER_TypeDef *) TIMER0_NS_BASE)                  /**< TIMER0_NS base pointer */
#define TIMER1_NS            ((TIMER_TypeDef *) TIMER1_NS_BASE)                  /**< TIMER1_NS base pointer */
#define TIMER2_NS            ((TIMER_TypeDef *) TIMER2_NS_BASE)                  /**< TIMER2_NS base pointer */
#define TIMER3_NS            ((TIMER_TypeDef *) TIMER3_NS_BASE)                  /**< TIMER3_NS base pointer */
#define ETAMPDET_NS          ((ETAMPDET_TypeDef *) ETAMPDET_NS_BASE)             /**< ETAMPDET_NS base pointer */
#define L2ICACHE0_NS         ((L2CACHE_TypeDef *) L2ICACHE0_NS_BASE)             /**< L2ICACHE0_NS base pointer */
#define PIXELRZ0_NS          ((PIXELRZ_TypeDef *) PIXELRZ0_NS_BASE)              /**< PIXELRZ0_NS base pointer */
#define BURTC_NS             ((BURTC_TypeDef *) BURTC_NS_BASE)                   /**< BURTC_NS base pointer */
#define I2C1_NS              ((I2C_TypeDef *) I2C1_NS_BASE)                      /**< I2C1_NS base pointer */
#define SYSCFG_NS_CFGNS      ((SYSCFG_CFGNS_TypeDef *) SYSCFG_NS_CFGNS_BASE)     /**< SYSCFG_NS_CFGNS base pointer */
#define SYSCFG_NS            ((SYSCFG_TypeDef *) SYSCFG_NS_BASE)                 /**< SYSCFG_NS base pointer */
#define BURAM_NS             ((BURAM_TypeDef *) BURAM_NS_BASE)                   /**< BURAM_NS base pointer */
#define GPCRC0_NS            ((GPCRC_TypeDef *) GPCRC0_NS_BASE)                  /**< GPCRC0_NS base pointer */
#define EUSART1_NS           ((EUSART_TypeDef *) EUSART1_NS_BASE)                /**< EUSART1_NS base pointer */
#define EUSART2_NS           ((EUSART_TypeDef *) EUSART2_NS_BASE)                /**< EUSART2_NS base pointer */
#define PIXELRZ1_NS          ((PIXELRZ_TypeDef *) PIXELRZ1_NS_BASE)              /**< PIXELRZ1_NS base pointer */
#define I2C2_NS              ((I2C_TypeDef *) I2C2_NS_BASE)                      /**< I2C2_NS base pointer */
#define WDOG1_NS             ((WDOG_TypeDef *) WDOG1_NS_BASE)                    /**< WDOG1_NS base pointer */
#define RPA_NS               ((RPA_TypeDef *) RPA_NS_BASE)                       /**< RPA_NS base pointer */
#define KSU_NS               ((KSU_TypeDef *) KSU_NS_BASE)                       /**< KSU_NS base pointer */
#define SYMCRYPTO_NS         ((SYMCRYPTO_TypeDef *) SYMCRYPTO_NS_BASE)           /**< SYMCRYPTO_NS base pointer */
#define LPWAES_NS            ((SYMCRYPTO_TypeDef *) LPWAES_NS_BASE)              /**< LPWAES_NS base pointer */
#define SMU_NS               ((SMU_TypeDef *) SMU_NS_BASE)                       /**< SMU_NS base pointer */
#define SMU_NS_CFGNS         ((SMU_CFGNS_TypeDef *) SMU_NS_CFGNS_BASE)           /**< SMU_NS_CFGNS base pointer */
#define SEMAILBOX_NS_HOST    ((SEMAILBOX_HOST_TypeDef *) SEMAILBOX_NS_HOST_BASE) /**< SEMAILBOX_NS_HOST base pointer */
#define SYSRTC0_NS           ((SYSRTC_TypeDef *) SYSRTC0_NS_BASE)                /**< SYSRTC0_NS base pointer */
#define SCRATCHPAD_NS        ((SCRATCHPAD_TypeDef *) SCRATCHPAD_NS_BASE)         /**< SCRATCHPAD_NS base pointer */
#define EMU_NS               ((EMU_TypeDef *) EMU_NS_BASE)                       /**< EMU_NS base pointer */
#define EMU_NS_CFGNS         ((EMU_CFGNS_TypeDef *) EMU_NS_CFGNS_BASE)           /**< EMU_NS_CFGNS base pointer */
#define SEPORTAL_NS          ((SEPORTAL_TypeDef *) SEPORTAL_NS_BASE)             /**< SEPORTAL_NS base pointer */
#define LPW0PORTAL_NS        ((LPW0PORTAL_TypeDef *) LPW0PORTAL_NS_BASE)         /**< LPW0PORTAL_NS base pointer */
#define HOSTPORTAL_NS        ((HOSTPORTAL_TypeDef *) HOSTPORTAL_NS_BASE)         /**< HOSTPORTAL_NS base pointer */
#define DEVINFO              ((DEVINFO_TypeDef *) DEVINFO_BASE)                  /**< DEVINFO base pointer */
#define LETIMER0             ((LETIMER_TypeDef *) LETIMER0_BASE)                 /**< LETIMER0 base pointer */
#define ADC0                 ((ADC_TypeDef *) ADC0_BASE)                         /**< ADC0 base pointer */
#define ACMP0                ((ACMP_TypeDef *) ACMP0_BASE)                       /**< ACMP0 base pointer */
#define ACMP1                ((ACMP_TypeDef *) ACMP1_BASE)                       /**< ACMP1 base pointer */
#define PCNT0                ((PCNT_TypeDef *) PCNT0_BASE)                       /**< PCNT0 base pointer */
#define HFRCOEM23            ((HFRCO_TypeDef *) HFRCOEM23_BASE)                  /**< HFRCOEM23 base pointer */
#define HFXO0                ((HFXO_TypeDef *) HFXO0_BASE)                       /**< HFXO0 base pointer */
#define I2C0                 ((I2C_TypeDef *) I2C0_BASE)                         /**< I2C0 base pointer */
#define WDOG0                ((WDOG_TypeDef *) WDOG0_BASE)                       /**< WDOG0 base pointer */
#define EUSART0              ((EUSART_TypeDef *) EUSART0_BASE)                   /**< EUSART0 base pointer */
#define SOCPLL0              ((SOCPLL_TypeDef *) SOCPLL0_BASE)                   /**< SOCPLL0 base pointer */
#define CMU                  ((CMU_TypeDef *) CMU_BASE)                          /**< CMU base pointer */
#define DMEM                 ((HYDRARAM_TypeDef *) DMEM_BASE)                    /**< DMEM base pointer */
#define HFRCO0               ((HFRCO_TypeDef *) HFRCO0_BASE)                     /**< HFRCO0 base pointer */
#define LDMA0                ((LDMA_TypeDef *) LDMA0_BASE)                       /**< LDMA0 base pointer */
#define FSRCO                ((FSRCO_TypeDef *) FSRCO_BASE)                      /**< FSRCO base pointer */
#define DPLL0                ((DPLL_TypeDef *) DPLL0_BASE)                       /**< DPLL0 base pointer */
#define LFXO                 ((LFXO_TypeDef *) LFXO_BASE)                        /**< LFXO base pointer */
#define LFRCO                ((LFRCO_TypeDef *) LFRCO_BASE)                      /**< LFRCO base pointer */
#define ULFRCO               ((ULFRCO_TypeDef *) ULFRCO_BASE)                    /**< ULFRCO base pointer */
#define SEMAPHORE0           ((SEMAPHORE_TypeDef *) SEMAPHORE0_BASE)             /**< SEMAPHORE0 base pointer */
#define SEMAPHORE1           ((SEMAPHORE_TypeDef *) SEMAPHORE1_BASE)             /**< SEMAPHORE1 base pointer */
#define L1ICACHE0            ((ICACHE_TypeDef *) L1ICACHE0_BASE)                 /**< L1ICACHE0 base pointer */
#define PRS                  ((PRS_TypeDef *) PRS_BASE)                          /**< PRS base pointer */
#define GPIO                 ((GPIO_TypeDef *) GPIO_BASE)                        /**< GPIO base pointer */
#define LDMAXBAR0            ((LDMAXBAR_TypeDef *) LDMAXBAR0_BASE)               /**< LDMAXBAR0 base pointer */
#define TIMER0               ((TIMER_TypeDef *) TIMER0_BASE)                     /**< TIMER0 base pointer */
#define TIMER1               ((TIMER_TypeDef *) TIMER1_BASE)                     /**< TIMER1 base pointer */
#define TIMER2               ((TIMER_TypeDef *) TIMER2_BASE)                     /**< TIMER2 base pointer */
#define TIMER3               ((TIMER_TypeDef *) TIMER3_BASE)                     /**< TIMER3 base pointer */
#define ETAMPDET             ((ETAMPDET_TypeDef *) ETAMPDET_BASE)                /**< ETAMPDET base pointer */
#define L2ICACHE0            ((L2CACHE_TypeDef *) L2ICACHE0_BASE)                /**< L2ICACHE0 base pointer */
#define PIXELRZ0             ((PIXELRZ_TypeDef *) PIXELRZ0_BASE)                 /**< PIXELRZ0 base pointer */
#define BURTC                ((BURTC_TypeDef *) BURTC_BASE)                      /**< BURTC base pointer */
#define I2C1                 ((I2C_TypeDef *) I2C1_BASE)                         /**< I2C1 base pointer */
#define SYSCFG_CFGNS         ((SYSCFG_CFGNS_TypeDef *) SYSCFG_CFGNS_BASE)        /**< SYSCFG_CFGNS base pointer */
#define SYSCFG               ((SYSCFG_TypeDef *) SYSCFG_BASE)                    /**< SYSCFG base pointer */
#define BURAM                ((BURAM_TypeDef *) BURAM_BASE)                      /**< BURAM base pointer */
#define GPCRC0               ((GPCRC_TypeDef *) GPCRC0_BASE)                     /**< GPCRC0 base pointer */
#define EUSART1              ((EUSART_TypeDef *) EUSART1_BASE)                   /**< EUSART1 base pointer */
#define EUSART2              ((EUSART_TypeDef *) EUSART2_BASE)                   /**< EUSART2 base pointer */
#define PIXELRZ1             ((PIXELRZ_TypeDef *) PIXELRZ1_BASE)                 /**< PIXELRZ1 base pointer */
#define I2C2                 ((I2C_TypeDef *) I2C2_BASE)                         /**< I2C2 base pointer */
#define WDOG1                ((WDOG_TypeDef *) WDOG1_BASE)                       /**< WDOG1 base pointer */
#define RPA                  ((RPA_TypeDef *) RPA_BASE)                          /**< RPA base pointer */
#define KSU                  ((KSU_TypeDef *) KSU_BASE)                          /**< KSU base pointer */
#define SYMCRYPTO            ((SYMCRYPTO_TypeDef *) SYMCRYPTO_BASE)              /**< SYMCRYPTO base pointer */
#define LPWAES               ((SYMCRYPTO_TypeDef *) LPWAES_BASE)                 /**< LPWAES base pointer */
#define SMU                  ((SMU_TypeDef *) SMU_BASE)                          /**< SMU base pointer */
#define SMU_CFGNS            ((SMU_CFGNS_TypeDef *) SMU_CFGNS_BASE)              /**< SMU_CFGNS base pointer */
#define SEMAILBOX_HOST       ((SEMAILBOX_HOST_TypeDef *) SEMAILBOX_HOST_BASE)    /**< SEMAILBOX_HOST base pointer */
#define SYSRTC0              ((SYSRTC_TypeDef *) SYSRTC0_BASE)                   /**< SYSRTC0 base pointer */
#define SCRATCHPAD           ((SCRATCHPAD_TypeDef *) SCRATCHPAD_BASE)            /**< SCRATCHPAD base pointer */
#define EMU                  ((EMU_TypeDef *) EMU_BASE)                          /**< EMU base pointer */
#define EMU_CFGNS            ((EMU_CFGNS_TypeDef *) EMU_CFGNS_BASE)              /**< EMU_CFGNS base pointer */
#define SEPORTAL             ((SEPORTAL_TypeDef *) SEPORTAL_BASE)                /**< SEPORTAL base pointer */
#define LPW0PORTAL           ((LPW0PORTAL_TypeDef *) LPW0PORTAL_BASE)            /**< LPW0PORTAL base pointer */
#define HOSTPORTAL           ((HOSTPORTAL_TypeDef *) HOSTPORTAL_BASE)            /**< HOSTPORTAL base pointer */

/** @} End of group SIBG301M104XIL_Peripheral_Declaration */

/**************************************************************************//**
 * @defgroup SIBG301M104XIL_Peripheral_Parameters SIBG301M104XIL Peripheral Parameters
 * @{
 * @brief Device peripheral parameter values
 *****************************************************************************/

/* Common peripheral register block offsets. */
#define PER_REG_BLOCK_SET_OFFSET                 0x1000UL     /**< Offset to SET register block */
#define PER_REG_BLOCK_CLR_OFFSET                 0x2000UL     /**< Offset to CLEAR register block */
#define PER_REG_BLOCK_TGL_OFFSET                 0x3000UL     /**< Offset to TOGGLE register block */
#define DMEM_BANK0_SIZE                          0x4000UL     /**> Bank0 size */
#define DMEM_BANK10_SIZE                         0x4000UL     /**> Bank10 size */
#define DMEM_BANK11_SIZE                         0x4000UL     /**> Bank11 size */
#define DMEM_BANK12_SIZE                         0x4000UL     /**> Bank12 size */
#define DMEM_BANK13_SIZE                         0x4000UL     /**> Bank13 size */
#define DMEM_BANK14_SIZE                         0x4000UL     /**> Bank14 size */
#define DMEM_BANK15_SIZE                         0x4000UL     /**> Bank15 size */
#define DMEM_BANK16_SIZE                         0x4000UL     /**> Bank16 size */
#define DMEM_BANK17_SIZE                         0x4000UL     /**> Bank17 size */
#define DMEM_BANK18_SIZE                         0x4000UL     /**> Bank18 size */
#define DMEM_BANK19_SIZE                         0x4000UL     /**> Bank19 size */
#define DMEM_BANK1_SIZE                          0x4000UL     /**> Bank1 size */
#define DMEM_BANK20_SIZE                         0x4000UL     /**> Bank20 size */
#define DMEM_BANK21_SIZE                         0x4000UL     /**> Bank21 size */
#define DMEM_BANK22_SIZE                         0x4000UL     /**> Bank22 size */
#define DMEM_BANK23_SIZE                         0x4000UL     /**> Bank23 size */
#define DMEM_BANK24_SIZE                         0x4000UL     /**> Bank24 size */
#define DMEM_BANK25_SIZE                         0x4000UL     /**> Bank25 size */
#define DMEM_BANK26_SIZE                         0x4000UL     /**> Bank26 size */
#define DMEM_BANK27_SIZE                         0x4000UL     /**> Bank27 size */
#define DMEM_BANK28_SIZE                         0x4000UL     /**> Bank28 size */
#define DMEM_BANK29_SIZE                         0x4000UL     /**> Bank29 size */
#define DMEM_BANK2_SIZE                          0x4000UL     /**> Bank2 size */
#define DMEM_BANK30_SIZE                         0x4000UL     /**> Bank30 size */
#define DMEM_BANK31_SIZE                         0x4000UL     /**> Bank31 size */
#define DMEM_BANK3_SIZE                          0x4000UL     /**> Bank3 size */
#define DMEM_BANK4_SIZE                          0x4000UL     /**> Bank4 size */
#define DMEM_BANK5_SIZE                          0x4000UL     /**> Bank5 size */
#define DMEM_BANK6_SIZE                          0x4000UL     /**> Bank6 size */
#define DMEM_BANK7_SIZE                          0x4000UL     /**> Bank7 size */
#define DMEM_BANK8_SIZE                          0x4000UL     /**> Bank8 size */
#define DMEM_BANK9_SIZE                          0x4000UL     /**> Bank9 size */
#define DMEM_ECC_EXCLUDE                         0x0UL        /**> ECC exclude */
#define DMEM_GLOBAL_MONITOR0_ID                  0x11UL       /**> Manager ID for global monitor0 */
#define DMEM_GLOBAL_MONITOR10_ID                 0xAUL        /**> Manager ID for global monitor10 */
#define DMEM_GLOBAL_MONITOR11_ID                 0xBUL        /**> Manager ID for global monitor11 */
#define DMEM_GLOBAL_MONITOR12_ID                 0xCUL        /**> Manager ID for global monitor12 */
#define DMEM_GLOBAL_MONITOR13_ID                 0xDUL        /**> Manager ID for global monitor13 */
#define DMEM_GLOBAL_MONITOR14_ID                 0xEUL        /**> Manager ID for global monitor14 */
#define DMEM_GLOBAL_MONITOR15_ID                 0xFUL        /**> Manager ID for global monitor15 */
#define DMEM_GLOBAL_MONITOR16_ID                 0x10UL       /**> Manager ID for global monitor16 */
#define DMEM_GLOBAL_MONITOR17_ID                 0x11UL       /**> Manager ID for global monitor17 */
#define DMEM_GLOBAL_MONITOR18_ID                 0x12UL       /**> Manager ID for global monitor18 */
#define DMEM_GLOBAL_MONITOR19_ID                 0x13UL       /**> Manager ID for global monitor19 */
#define DMEM_GLOBAL_MONITOR1_ID                  0x21UL       /**> Manager ID for global monitor1 */
#define DMEM_GLOBAL_MONITOR20_ID                 0x14UL       /**> Manager ID for global monitor20 */
#define DMEM_GLOBAL_MONITOR21_ID                 0x15UL       /**> Manager ID for global monitor21 */
#define DMEM_GLOBAL_MONITOR22_ID                 0x16UL       /**> Manager ID for global monitor22 */
#define DMEM_GLOBAL_MONITOR23_ID                 0x17UL       /**> Manager ID for global monitor23 */
#define DMEM_GLOBAL_MONITOR24_ID                 0x18UL       /**> Manager ID for global monitor24 */
#define DMEM_GLOBAL_MONITOR25_ID                 0x19UL       /**> Manager ID for global monitor25 */
#define DMEM_GLOBAL_MONITOR26_ID                 0x1AUL       /**> Manager ID for global monitor26 */
#define DMEM_GLOBAL_MONITOR27_ID                 0x1BUL       /**> Manager ID for global monitor27 */
#define DMEM_GLOBAL_MONITOR28_ID                 0x1CUL       /**> Manager ID for global monitor28 */
#define DMEM_GLOBAL_MONITOR29_ID                 0x1DUL       /**> Manager ID for global monitor29 */
#define DMEM_GLOBAL_MONITOR2_ID                  0x49UL       /**> Manager ID for global monitor2 */
#define DMEM_GLOBAL_MONITOR30_ID                 0x1EUL       /**> Manager ID for global monitor30 */
#define DMEM_GLOBAL_MONITOR31_ID                 0x1FUL       /**> Manager ID for global monitor31 */
#define DMEM_GLOBAL_MONITOR32_ID                 0x20UL       /**> Manager ID for global monitor32 */
#define DMEM_GLOBAL_MONITOR33_ID                 0x21UL       /**> Manager ID for global monitor33 */
#define DMEM_GLOBAL_MONITOR34_ID                 0x22UL       /**> Manager ID for global monitor34 */
#define DMEM_GLOBAL_MONITOR35_ID                 0x23UL       /**> Manager ID for global monitor35 */
#define DMEM_GLOBAL_MONITOR36_ID                 0x24UL       /**> Manager ID for global monitor36 */
#define DMEM_GLOBAL_MONITOR37_ID                 0x25UL       /**> Manager ID for global monitor37 */
#define DMEM_GLOBAL_MONITOR38_ID                 0x26UL       /**> Manager ID for global monitor38 */
#define DMEM_GLOBAL_MONITOR39_ID                 0x27UL       /**> Manager ID for global monitor39 */
#define DMEM_GLOBAL_MONITOR3_ID                  0x3UL        /**> Manager ID for global monitor3 */
#define DMEM_GLOBAL_MONITOR40_ID                 0x28UL       /**> Manager ID for global monitor40 */
#define DMEM_GLOBAL_MONITOR41_ID                 0x29UL       /**> Manager ID for global monitor41 */
#define DMEM_GLOBAL_MONITOR42_ID                 0x2AUL       /**> Manager ID for global monitor42 */
#define DMEM_GLOBAL_MONITOR43_ID                 0x2BUL       /**> Manager ID for global monitor43 */
#define DMEM_GLOBAL_MONITOR44_ID                 0x2CUL       /**> Manager ID for global monitor44 */
#define DMEM_GLOBAL_MONITOR45_ID                 0x2DUL       /**> Manager ID for global monitor45 */
#define DMEM_GLOBAL_MONITOR46_ID                 0x2EUL       /**> Manager ID for global monitor46 */
#define DMEM_GLOBAL_MONITOR47_ID                 0x2FUL       /**> Manager ID for global monitor47 */
#define DMEM_GLOBAL_MONITOR48_ID                 0x30UL       /**> Manager ID for global monitor48 */
#define DMEM_GLOBAL_MONITOR49_ID                 0x31UL       /**> Manager ID for global monitor49 */
#define DMEM_GLOBAL_MONITOR4_ID                  0x4UL        /**> Manager ID for global monitor4 */
#define DMEM_GLOBAL_MONITOR50_ID                 0x32UL       /**> Manager ID for global monitor50 */
#define DMEM_GLOBAL_MONITOR51_ID                 0x33UL       /**> Manager ID for global monitor51 */
#define DMEM_GLOBAL_MONITOR52_ID                 0x34UL       /**> Manager ID for global monitor52 */
#define DMEM_GLOBAL_MONITOR53_ID                 0x35UL       /**> Manager ID for global monitor53 */
#define DMEM_GLOBAL_MONITOR54_ID                 0x36UL       /**> Manager ID for global monitor54 */
#define DMEM_GLOBAL_MONITOR55_ID                 0x37UL       /**> Manager ID for global monitor55 */
#define DMEM_GLOBAL_MONITOR56_ID                 0x38UL       /**> Manager ID for global monitor56 */
#define DMEM_GLOBAL_MONITOR57_ID                 0x39UL       /**> Manager ID for global monitor57 */
#define DMEM_GLOBAL_MONITOR58_ID                 0x3AUL       /**> Manager ID for global monitor58 */
#define DMEM_GLOBAL_MONITOR59_ID                 0x3BUL       /**> Manager ID for global monitor59 */
#define DMEM_GLOBAL_MONITOR5_ID                  0x5UL        /**> Manager ID for global monitor5 */
#define DMEM_GLOBAL_MONITOR60_ID                 0x3CUL       /**> Manager ID for global monitor60 */
#define DMEM_GLOBAL_MONITOR61_ID                 0x3DUL       /**> Manager ID for global monitor61 */
#define DMEM_GLOBAL_MONITOR62_ID                 0x3EUL       /**> Manager ID for global monitor62 */
#define DMEM_GLOBAL_MONITOR63_ID                 0x3FUL       /**> Manager ID for global monitor63 */
#define DMEM_GLOBAL_MONITOR6_ID                  0x6UL        /**> Manager ID for global monitor6 */
#define DMEM_GLOBAL_MONITOR7_ID                  0x7UL        /**> Manager ID for global monitor7 */
#define DMEM_GLOBAL_MONITOR8_ID                  0x8UL        /**> Manager ID for global monitor8 */
#define DMEM_GLOBAL_MONITOR9_ID                  0x9UL        /**> Manager ID for global monitor9 */
#define DMEM_GLOBAL_MONITOR_EN                   0x1UL        /**> Enable global monitor checking */
#define DMEM_INTEGRITY_CHECKING_EN               0x0UL        /**> Enable Integrity checks */
#define DMEM_INTEGRITY_GENERATION_EN             0x0UL        /**> Enable Integrity generation */
#define DMEM_INTERLEAVE_EN                       0x1UL        /**> Enable Interleaving */
#define DMEM_INTERLEAVE_EXCLUDE                  0x0UL        /**> Interleaving Exclude */
#define DMEM_MEM_SIZE                            0x80000UL    /**> Total Bank Size */
#define DMEM_NUM_AHB_PORTS                       0x0UL        /**> Number of AHB Ports */
#define DMEM_NUM_AXI_PORTS                       0x0UL        /**> Number of AXI Ports */
#define DMEM_NUM_BANKS                           0x20UL       /**> Number of physical SRAM banks */
#define DMEM_INTERLEAVE2_BITS                    0x10UL       /**> Number of 2 bank interleaving bits */
#define DMEM_INTERLEAVE4_BITS                    0x8UL        /**> Number of 4 bank interleaving bits */
#define DMEM_INTERLEAVE8_BITS                    0x4UL        /**> Number of 8 bank interleaving bits */
#define DMEM_NUM_GLOBAL_MONITOR                  0x3UL        /**> Number of global monitor state machines */
#define DMEM_NUM_OBI_PORTS                       0x0UL        /**> Number of OBI Ports */
#define DMEM_NUM_SLAXI_PORTS                     0x4UL        /**> Number of SLAXI Ports */
#define DMEM_HYDRARAM_NUM_PORTS                  0x4UL        /**> Total number of HYDRARAM Ports */
#define DMEM_PRIORITY_WIDTH                      0x1UL        /**> Priority Width */
#define DMEM_PRIORITY_EXCLUDE                    0x0UL        /**> Priority Exclude */
#define DMEM_WAITSTATE_EXCLUDE                   0x1UL        /**> Waitstate exclude */
#define L2ICACHE0_ADDR_BITS                      0x0UL        /**> Cacheable size bits */
#define L2ICACHE0_ADDR_PIPE_STAGE                0x1UL        /**> Addr Pipeline Stage */
#define L2ICACHE0_ASYNCPORT0_MEMSIZE             0x8000000UL  /**> Memory size for async port 0 */
#define L2ICACHE0_DATA_PIPE_STAGE                0x1UL        /**> Data Pipeline Stage */
#define L2ICACHE0_MASTERID_BITS                  0x8UL        /**> Slaxi MasterID bits */
#define L2ICACHE0_MASTER_IDS                     0x0UL        /**> Arrary of Master IDs */
#define L2ICACHE0_NUM_ASYNC_PORTS                0x0UL        /**> Number of ASYNC SLAXI ports */
#define L2ICACHE0_NUM_MASTERS                    0x5UL        /**> Number of SLAXI Masters */
#define L2ICACHE0_NUM_MSHR_ENTRY                 0x8UL        /**> Number of MSHR */
#define L2ICACHE0_NUM_MSHR_SUBENTRY              0x1UL        /**> Number of MSHR Sub entry */
#define L2ICACHE0_NUM_SYNC_PORTS                 0x2UL        /**> Number of SYNC SLAXI Ports */
#define L2ICACHE0_PARITY_EXCLUDE                 0x0UL        /**> Use Parity */
#define L2ICACHE0_PC_BITS                        0x20UL       /**> Performance Counter bits */
#define L2ICACHE0_QSPI_SIZE                      0x8000000UL  /**> Flash size */
#define L2ICACHE0_QSPI_START                     0x0UL        /**> Flash start */
#define L2ICACHE0_RAM_ADDR_BITS                  0x0UL        /**> RAM Address bits */
#define L2ICACHE0_RAM_DATA_BITS                  0x0UL        /**> RAM Data bits */
#define L2ICACHE0_SET_BITS                       0x9UL        /**> Set bits */
#define L2ICACHE0_SYNCPORT0_MEMSIZE              0x8000000UL  /**> Memory size for sync port 0 */
#define L2ICACHE0_SYNCPORT1_MEMSIZE              0x40000UL    /**> Memory size for sync port 1 */
#define L2ICACHE0_TAG_ADDR_BITS                  0x0UL        /**> TAG RAM Addr bits */
#define L2ICACHE0_TAG_BITS                       0x0UL        /**> TAG data bits */
#define L2ICACHE0_TAG_DATA_BITS                  0x0UL        /**> TAG RAM Data bits */
#define L2ICACHE0_TAG_RAM_BANKS                  0x4UL        /**> Number of TAG RAM Banks */
#define L2ICACHE0_WAY_BITS                       0x2UL        /**> Way bits */
#define L2ICACHE0_WORDS_PER_LINE                 0x0UL        /**> Words Per Line */
#define L2ICACHE0_WPL_BITS                       0x3UL        /**> Words Per Line bits */
#define DEVINFO_LEGACY_RV                        0x810000UL   /**> Reset Value for Legacy Register */
#define DEVINFO_NUM_SPARE                        0x7UL        /**> Spare Register Parameter */
#define LETIMER0_CNT_WIDTH                       0x18UL       /**> Count Width */
#define ADC0_CHANNELS                            0x10UL       /**> scan channels */
#define ADC0_CONFIGNUM                           0x2UL        /**> CONFIG */
#define ACMP0_DAC_INPUT                          0x1UL        /**> None */
#define ACMP0_EXT_OVR_IF                         0x1UL        /**> None */
#define ACMP1_DAC_INPUT                          0x1UL        /**> None */
#define ACMP1_EXT_OVR_IF                         0x1UL        /**> None */
#define PCNT0_PCNT_WIDTH                         0x10UL       /**> None */
#define HFRCOEM23_EM23ONDEMAND                   0x1UL        /**> EM23 On Demand */
#define HFRCOEM23_EXCLUDEEM23ONDEMAND            0x0UL        /**> Exclude EM23 On Demand */
#define HFXO0_BUFOUT                             0x0UL        /**> BUFOUT */
#define HFXO0_EXCLUDEBUFOUT                      0x1UL        /**> Exclude BUFOUT */
#define I2C0_DELAY                               0x64UL       /**> Delay cell selection */
#define I2C0_DELAY_CHAIN_NUM                     0x28UL       /**> Number of delay chain */
#define WDOG0_PCNUM                              0x2UL        /**> None */
#define EUSART0_EM2_CAPABLE                      0x1UL        /**> EM2 Capable instance */
#define EUSART0_EXCLUDE_DALI                     0x0UL        /**> Exclude DALI */
#define EUSART0_FIFO_DEPTH                       0x20UL       /**> FIFO_DEPTH */
#define EUSART0_NOT_EM2_CAPABLE                  0x0UL        /**> Not EM2 Capable instance */
#define CMU_EXCLUDELCD                           0x1UL        /**> Exclude LCD */
#define CMU_EXCLUDELESENSE                       0x1UL        /**> Exclude LESENSE */
#define HFRCO0_EM23ONDEMAND                      0x0UL        /**> EM23 On Demand */
#define HFRCO0_EXCLUDEEM23ONDEMAND               0x1UL        /**> Exclude EM23 On Demand */
#define LDMA0_AHB1_ASYNC                         0x0UL        /**> AHB master 1 asynchronous support */
#define LDMA0_CH_BITS                            0x4UL        /**> Channel Index Bit-width */
#define LDMA0_CH_NUM                             0x8UL        /**> Number Of Chnnels */
#define LDMA0_DUALDSTCHNL                        0x0UL        /**> Dual Destination Channels */
#define LDMA0_DUALDSTCHNL_DIS                    0x1UL        /**> Dual Destination Disable */
#define LDMA0_FIFO_BITS                          0x5UL        /**> FIFO Bit Width */
#define LDMA0_FIFO_DEPTH                         0x10UL       /**> FIFO Depth */
#define LDMA0_ILCHNL                             0x3UL        /**> Interleaving Channels */
#define LDMA0_ILCHNL_DIS                         0x0UL        /**> Interleaving Disabled */
#define LDMA0_ULDMA_MODE                         0x0UL        /**> ULDMA Mode */
#define LDMA0_ADDRDEC_DIS                        0x0UL        /**> Address Decrement Disable */
#define LDMA0_ARBSLOT_DIS                        0x0UL        /**> Arbitration Slot Disable */
#define LDMA0_CHAN_CFG_DIS                       0x0UL        /**> Channel Config Disable */
#define LDMA0_EXTENDED_DIS                       0x0UL        /**> Extended Descriptor Disable */
#define LDMA0_IGNORESREQ_DIS                     0x0UL        /**> Ignore SREQ disable */
#define LDMA0_LOOP_DIS                           0x0UL        /**> Looping Disable */
#define LDMA0_MULTIMASTER_DIS                    0x0UL        /**> Multi-Master Disable */
#define LDMA0_RELADDR_DIS                        0x0UL        /**> Relative addressing Disable */
#define LDMA0_REQMODE_DIS                        0x0UL        /**> Request mode disable */
#define LDMA0_SYNC_NUM                           0x8UL        /**> Sync Channels */
#define LDMA0_SYNC_DIS                           0x0UL        /**> Sync struct disabled */
#define LDMA0_WRI_DIS                            0x0UL        /**> Write Immediate Disable */
#define LDMA0_XFERTYPE_DIS                       0x0UL        /**> Transfer type disable */
#define LFXO_NO_CTUNE                            0x0UL        /**> CTUNE Not Present */
#define LFXO_CTUNE                               0x1UL        /**> CTUNE Present */
#define L1ICACHE0_CACHEABLE_SIZE                 0x4000000UL  /**> Cache Size */
#define L1ICACHE0_CACHEABLE_START                0x1000000UL  /**> Cache Start */
#define L1ICACHE0_CACHE_REGION_MASK              0x10000000UL /**> Cacheable Region Mask */
#define L1ICACHE0_DEFAULT_OFF                    0x0UL        /**> Default off */
#define L1ICACHE0_INTEGRITY_EN                   0x0UL        /**> Generate Read integrity */
#define L1ICACHE0_LOOPCACHE_MEM_ADDR_BITS        0x3UL        /**> Loopcache Memory Address bits */
#define L1ICACHE0_LOOPCACHE_STICKINESS_BITS      0x4UL        /**> Loopcache Stickiness bits */
#define L1ICACHE0_PC_BITS                        0x20UL       /**> Performance Counter bits */
#define L1ICACHE0_PIPE_STAGE                     0x1UL        /**> Pipeline Stage */
#define L1ICACHE0_RAM_ADDR_BITS                  0x0UL        /**> RAM Address bits */
#define L1ICACHE0_RAM_DATA_BITS                  0x21UL       /**> RAM Data bits */
#define L1ICACHE0_SET_BITS                       0x8UL        /**> Set bits */
#define L1ICACHE0_USE_LOOPCACHE                  0x0UL        /**> Use Loopcache */
#define L1ICACHE0_WAY_BITS                       0x1UL        /**> Way bits */
#define L1ICACHE0_WORDS_PER_BLOCK                0x0UL        /**> Words Per Block */
#define L1ICACHE0_WPB_BITS                       0x2UL        /**> Words Per Block bits */
#define L1ICACHE0_WPL_BITS                       0x3UL        /**> Words Per Line bits */
#define PRS_ASYNC_CH_NUM                         0xCUL        /**> None */
#define PRS_PRSSEL_WIDTH                         0x4UL        /**> New Param */
#define PRS_SPRSSEL_WIDTH                        0x2UL        /**> New Param */
#define PRS_SYNC_CH_NUM                          0x4UL        /**> None */
#define GPIO_MODE_WIDTH                          0x4UL        /**> Mode Width */
#define GPIO_NUM_EM4_WU                          0xCUL        /**> New Param */
#define GPIO_NUM_EVEN_PA                         0x5UL        /**> Num of even pins port A */
#define GPIO_NUM_EVEN_PB                         0x3UL        /**> Num of even pins port B */
#define GPIO_NUM_EVEN_PC                         0x4UL        /**> Num of even pins port C */
#define GPIO_NUM_EVEN_PD                         0x3UL        /**> Num of even pins port D */
#define GPIO_NUM_EXT_INT                         0xCUL        /**> New Param */
#define GPIO_NUM_EXT_INT_L                       0x8UL        /**> New Param */
#define GPIO_NUM_EXT_INT_U                       0x4UL        /**> New Param */
#define GPIO_NUM_EXT_INT_U_ZERO                  0x0UL        /**> New Param */
#define GPIO_NUM_ODD_PA                          0x4UL        /**> Num of odd pins port A */
#define GPIO_NUM_ODD_PB                          0x2UL        /**> Num of odd pins port B */
#define GPIO_NUM_ODD_PC                          0x4UL        /**> Num of odd pins port C */
#define GPIO_NUM_ODD_PD                          0x3UL        /**> Num of odd pins port D */
#define GPIO_PINSEL_WIDTH                        0x4UL        /**> Route config pin select width */
#define GPIO_PORTSEL_WIDTH                       0x2UL        /**> Route config port select width */
#define GPIO_PORT_A_WIDTH                        0x9UL        /**> Port A Width */
#define GPIO_PORT_A_WIDTH_ZERO                   0x0UL        /**> Port A Width is Zero */
#define GPIO_PORT_A_WL                           0x8UL        /**> New Param */
#define GPIO_PORT_A_WU                           0x1UL        /**> New Param */
#define GPIO_PORT_A_WU_ZERO                      0x0UL        /**> New Param */
#define GPIO_PORT_B_WIDTH                        0x4UL        /**> Port B Width */
#define GPIO_PORT_B_WIDTH_ZERO                   0x0UL        /**> Port B Width is Zero */
#define GPIO_PORT_B_WL                           0x4UL        /**> New Param */
#define GPIO_PORT_B_WU                           0x0UL        /**> New Param */
#define GPIO_PORT_B_WU_ZERO                      0x1UL        /**> New Param */
#define GPIO_PORT_C_WIDTH                        0x8UL        /**> Port C Width */
#define GPIO_PORT_C_WIDTH_ZERO                   0x0UL        /**> Port C Width is Zero */
#define GPIO_PORT_C_WL                           0x8UL        /**> New Param */
#define GPIO_PORT_C_WU                           0x0UL        /**> New Param */
#define GPIO_PORT_C_WU_ZERO                      0x1UL        /**> New Param */
#define GPIO_PORT_D_WIDTH                        0x7UL        /**> Port D Width */
#define GPIO_PORT_D_WIDTH_ZERO                   0x0UL        /**> Port D Width is Zero */
#define GPIO_PORT_D_WL                           0x7UL        /**> New Param */
#define GPIO_PORT_D_WU                           0x0UL        /**> New Param */
#define GPIO_PORT_D_WU_ZERO                      0x1UL        /**> New Param */
#define GPIO_SLEWRATE_WIDTH                      0x3UL        /**> Slew Rate Width Param */
#define LDMAXBAR0_CH_BITS                        0x5UL        /**> None */
#define LDMAXBAR0_CH_NUM                         0x8UL        /**> None */
#define LDMAXBAR0_SIGSEL_W                       0x4UL        /**> New Param */
#define LDMAXBAR0_SOURCESEL_W                    0x6UL        /**> New Param */
#define TIMER0_CC_NUM                            0x3UL        /**> Number of Compare/Capture Channels */
#define TIMER0_CNTWIDTH                          0x20UL       /**> Counter Width */
#define TIMER0_DITHERWIDTH                       0x4UL        /**> Dither Width */
#define TIMER0_DTI                               0x1UL        /**> Dead-time insertion enabled */
#define TIMER0_DTI_CC_NUM                        0x3UL        /**> Number of DTI Channels */
#define TIMER0_NO_DTI                            0x0UL        /**>  */
#define TIMER1_CC_NUM                            0x3UL        /**> Number of Compare/Capture Channels */
#define TIMER1_CNTWIDTH                          0x20UL       /**> Counter Width */
#define TIMER1_DITHERWIDTH                       0x4UL        /**> Dither Width */
#define TIMER1_DTI                               0x1UL        /**> Dead-time insertion enabled */
#define TIMER1_DTI_CC_NUM                        0x3UL        /**> Number of DTI Channels */
#define TIMER1_NO_DTI                            0x0UL        /**>  */
#define TIMER2_CC_NUM                            0x7UL        /**> Number of Compare/Capture Channels */
#define TIMER2_CNTWIDTH                          0x20UL       /**> Counter Width */
#define TIMER2_DITHERWIDTH                       0x4UL        /**> Dither Width */
#define TIMER2_DTI                               0x1UL        /**> Dead-time insertion enabled */
#define TIMER2_DTI_CC_NUM                        0x3UL        /**> Number of DTI Channels */
#define TIMER2_NO_DTI                            0x0UL        /**>  */
#define TIMER3_CC_NUM                            0x7UL        /**> Number of Compare/Capture Channels */
#define TIMER3_CNTWIDTH                          0x20UL       /**> Counter Width */
#define TIMER3_DITHERWIDTH                       0x4UL        /**> Dither Width */
#define TIMER3_DTI                               0x1UL        /**> Dead-time insertion enabled */
#define TIMER3_DTI_CC_NUM                        0x3UL        /**> Number of DTI Channels */
#define TIMER3_NO_DTI                            0x0UL        /**>  */
#define ETAMPDET_NUM_CHNLS                       0x2UL        /**>  */
#define BURTC_CNTWIDTH                           0x20UL       /**> None */
#define BURTC_PRECNT_WIDTH                       0xFUL        /**>  */
#define I2C1_DELAY                               0x64UL       /**> Delay cell selection */
#define I2C1_DELAY_CHAIN_NUM                     0x28UL       /**> Number of delay chain */
#define SYSCFG_CHIP_PARTNUMBER                   0x10UL       /**> Chip Part Number */
#define SYSCFG_CHIP_SECOMPATID                   0x30UL       /**> Chip SE firmware die compatibility ID */
#define SYSCFG_CHIP_SEROMREV                     0x5UL        /**> Chip SE ROM revision */
#define SYSCFG_DEMODRAM_INST_COUNT               0x2UL        /**>  */
#define SYSCFG_FRCRAM_INST_COUNT                 0x1UL        /**>  */
#define SYSCFG_SEQRAM_INST_COUNT                 0xAUL        /**> None */
#define SYSCFG_SWINT_NUM                         0x4UL        /**> Software interupts */
#define EUSART1_EM2_CAPABLE                      0x0UL        /**> EM2 Capable instance */
#define EUSART1_EXCLUDE_DALI                     0x0UL        /**> Exclude DALI */
#define EUSART1_FIFO_DEPTH                       0x20UL       /**> FIFO_DEPTH */
#define EUSART1_NOT_EM2_CAPABLE                  0x1UL        /**> Not EM2 Capable instance */
#define EUSART2_EM2_CAPABLE                      0x0UL        /**> EM2 Capable instance */
#define EUSART2_EXCLUDE_DALI                     0x0UL        /**> Exclude DALI */
#define EUSART2_FIFO_DEPTH                       0x20UL       /**> FIFO_DEPTH */
#define EUSART2_NOT_EM2_CAPABLE                  0x1UL        /**> Not EM2 Capable instance */
#define I2C2_DELAY                               0x64UL       /**> Delay cell selection */
#define I2C2_DELAY_CHAIN_NUM                     0x28UL       /**> Number of delay chain */
#define WDOG1_PCNUM                              0x2UL        /**> None */
#define RPA_BUS_MASTER_ID                        0x19UL       /**> SLAXI bus master ID */
#define RPA_MAX_NUM_IRK                          0x20UL       /**> Maximum number of IRK to be supported */
#define RPA_NUM_MASK_REGS                        0x1UL        /**> Number of IRKMASK registers */
#define KSU_IRK_CRYPTOENG_ID                     0x0UL        /**> New Param */
#define KSU_KSU_RAM_DEPTH                        0x280UL      /**> ksu ram size */
#define KSU_MAX_KEY_SLOTS                        0x20UL       /**> New Param */
#define KSU_VALID_KEY_SLOT_CODE                  0x72UL       /**> New Param */
#define SYMCRYPTO_SIDECHANNEL_COUNTERMEASURES    0x1UL        /**> Enable sidechannel counter measures */
#define LPWAES_SIDECHANNEL_COUNTERMEASURES       0x1UL        /**> Enable sidechannel counter measures */
#define SMU_NUM_BMPUS                            0x7UL        /**> Number of BMPUs */
#define SMU_NUM_PPU_PERIPHS                      0x40UL       /**> Number of PPU Peripherals */
#define SMU_NUM_PPU_PERIPHS_MOD_32               0x0UL        /**> Number of PPU Peripherals (mod 32) */
#define SMU_NUM_PPU_PERIPHS_SUB_32               0x20UL       /**> Number of PPU peripherals minus 32 */
#define SMU_PERIPHID_BITS                        0x8UL        /**> Bits used for Peripheral ID */
#define SYSRTC0_GROUP0_ALTIRQDIS                 0x1UL        /**> Group 0 Alternate IRQ disable */
#define SYSRTC0_GROUP0_BUFCMPDIS                 0x1UL        /**> New Param */
#define SYSRTC0_GROUP0_CAPDIS                    0x0UL        /**> Group 0 Capture disable */
#define SYSRTC0_GROUP0_CMP0DIS                   0x0UL        /**> New Param */
#define SYSRTC0_GROUP0_CMP1DIS                   0x0UL        /**> Group 0 Compare1 disable */
#define SYSRTC0_GROUP0_DIS                       0x0UL        /**> Group 0 Disable */
#define SYSRTC0_GROUP0_PRETRIGDIS                0x0UL        /**> Group 0 Pretrigger */
#define SYSRTC0_GROUP0_ROOTDIS                   0x1UL        /**> Group 0 ROOT disable */
#define SYSRTC0_GROUP1_ALTIRQDIS                 0x0UL        /**> Group 1 Alternate IRQ disable */
#define SYSRTC0_GROUP1_BUFCMPDIS                 0x1UL        /**> New Param */
#define SYSRTC0_GROUP1_CAPDIS                    0x0UL        /**> Group 1 Capture disable */
#define SYSRTC0_GROUP1_CMP0DIS                   0x0UL        /**> New Param */
#define SYSRTC0_GROUP1_CMP1DIS                   0x0UL        /**> Group 1 Compare1 disable */
#define SYSRTC0_GROUP1_DIS                       0x0UL        /**> Group 1 Disable */
#define SYSRTC0_GROUP1_PRETRIGDIS                0x0UL        /**> Group 1 Pretrigger */
#define SYSRTC0_GROUP1_ROOTDIS                   0x1UL        /**> Group 1 ROOT disable */
#define SYSRTC0_GROUP2_ALTIRQDIS                 0x1UL        /**> Group 2 Alternate IRQ disable */
#define SYSRTC0_GROUP2_BUFCMPDIS                 0x1UL        /**> New Param */
#define SYSRTC0_GROUP2_CAPDIS                    0x1UL        /**> Group 2 Capture disable */
#define SYSRTC0_GROUP2_CMP0DIS                   0x0UL        /**> New Param */
#define SYSRTC0_GROUP2_CMP1DIS                   0x1UL        /**> Group 2 Compare1 disable */
#define SYSRTC0_GROUP2_DIS                       0x0UL        /**> Group 2 Disable */
#define SYSRTC0_GROUP2_PRETRIGDIS                0x1UL        /**> Group 2 Pretrigger */
#define SYSRTC0_GROUP2_ROOTDIS                   0x0UL        /**> Group 2 ROOT disable */
#define SYSRTC0_GROUP3_ALTIRQDIS                 0x1UL        /**> Group 3 Alternate IRQ disable */
#define SYSRTC0_GROUP3_BUFCMPDIS                 0x1UL        /**> New Param */
#define SYSRTC0_GROUP3_CAPDIS                    0x1UL        /**> Group 3 Capture disable */
#define SYSRTC0_GROUP3_CMP0DIS                   0x0UL        /**> New Param */
#define SYSRTC0_GROUP3_CMP1DIS                   0x1UL        /**> Group 3 Compare1 disable */
#define SYSRTC0_GROUP3_DIS                       0x1UL        /**> Group 3 Disable */
#define SYSRTC0_GROUP3_PRETRIGDIS                0x1UL        /**> Group 3 Pretrigger */
#define SYSRTC0_GROUP3_ROOTDIS                   0x1UL        /**> Group 3 ROOT disable */
#define SYSRTC0_GROUP4_ALTIRQDIS                 0x1UL        /**> Group 4 Alternate IRQ disable */
#define SYSRTC0_GROUP4_BUFCMPDIS                 0x1UL        /**> New Param */
#define SYSRTC0_GROUP4_CAPDIS                    0x1UL        /**> Group 4 Capture disable */
#define SYSRTC0_GROUP4_CMP0DIS                   0x0UL        /**> New Param */
#define SYSRTC0_GROUP4_CMP1DIS                   0x1UL        /**> Group 4 Compare1 disable */
#define SYSRTC0_GROUP4_DIS                       0x1UL        /**> Group 4 Disable */
#define SYSRTC0_GROUP4_PRETRIGDIS                0x1UL        /**> Group 4 Pretrigger */
#define SYSRTC0_GROUP4_ROOTDIS                   0x1UL        /**> Group 4 ROOT disable */
#define SYSRTC0_GROUP5_ALTIRQDIS                 0x1UL        /**> Group 5 Alternate IRQ disable */
#define SYSRTC0_GROUP5_BUFCMPDIS                 0x1UL        /**> New Param */
#define SYSRTC0_GROUP5_CAPDIS                    0x1UL        /**> Group 5 Capture disable */
#define SYSRTC0_GROUP5_CMP0DIS                   0x0UL        /**> New Param */
#define SYSRTC0_GROUP5_CMP1DIS                   0x1UL        /**> Group 5 Compare1 disable */
#define SYSRTC0_GROUP5_DIS                       0x1UL        /**> Group 5 Disable */
#define SYSRTC0_GROUP5_PRETRIGDIS                0x1UL        /**> Group 5 Pretrigger */
#define SYSRTC0_GROUP5_ROOTDIS                   0x1UL        /**> Group 5 ROOT disable */
#define SYSRTC0_GROUP6_ALTIRQDIS                 0x1UL        /**> Group 6 Alternate IRQ disable */
#define SYSRTC0_GROUP6_BUFCMPDIS                 0x1UL        /**> New Param */
#define SYSRTC0_GROUP6_CAPDIS                    0x1UL        /**> Group 6 Capture disable */
#define SYSRTC0_GROUP6_CMP0DIS                   0x0UL        /**> New Param */
#define SYSRTC0_GROUP6_CMP1DIS                   0x1UL        /**> Group 6 Compare1 disable */
#define SYSRTC0_GROUP6_DIS                       0x1UL        /**> Group 6 Disable */
#define SYSRTC0_GROUP6_PRETRIGDIS                0x1UL        /**> Group 6 Pretrigger */
#define SYSRTC0_GROUP6_ROOTDIS                   0x1UL        /**> Group 6 ROOT disable */
#define SYSRTC0_GROUP7_ALTIRQDIS                 0x1UL        /**> Group 7 Alternate IRQ disable */
#define SYSRTC0_GROUP7_BUFCMPDIS                 0x0UL        /**> New Param */
#define SYSRTC0_GROUP7_CAPDIS                    0x1UL        /**> Group 7 Capture disable */
#define SYSRTC0_GROUP7_CMP0DIS                   0x1UL        /**> New Param */
#define SYSRTC0_GROUP7_CMP1DIS                   0x1UL        /**> Group 7 Compare1 disable */
#define SYSRTC0_GROUP7_DIS                       0x1UL        /**> Group 7 Disable */
#define SYSRTC0_GROUP7_PRETRIGDIS                0x1UL        /**> Group 7 Pretrigger */
#define SYSRTC0_GROUP7_ROOTDIS                   0x1UL        /**> Group 7 ROOT disable */
#define SYSRTC0_ROOTDIS                          0x0UL        /**> ROOT disable */
#define EMU_DCDCCFG                              0x0UL        /**> DCDC config */
#define EMU_VSCALECFG                            0x0UL        /**> Voltage scaling config */

/* Instance macros for ACMP */
#define ACMP(n)                      (((n) == 0) ? ACMP0   \
                                      : ((n) == 1) ? ACMP1 \
                                      : 0x0UL)
#define ACMP_NUM(ref)                (((ref) == ACMP0) ? 0   \
                                      : ((ref) == ACMP1) ? 1 \
                                      : -1)
#define ACMP_DAC_INPUT(n)            (((n) == 0) ? ACMP0_DAC_INPUT   \
                                      : ((n) == 1) ? ACMP1_DAC_INPUT \
                                      : 0x0UL)
#define ACMP_EXT_OVR_IF(n)           (((n) == 0) ? ACMP0_EXT_OVR_IF   \
                                      : ((n) == 1) ? ACMP1_EXT_OVR_IF \
                                      : 0x0UL)

/* Instance macros for ADC */
#define ADC(n)                       (((n) == 0) ? ADC0 \
                                      : 0x0UL)
#define ADC_NUM(ref)                 (((ref) == ADC0) ? 0 \
                                      : -1)
#define ADC_CHANNELS(n)              (((n) == 0) ? ADC0_CHANNELS \
                                      : 0x0UL)
#define ADC_CONFIGNUM(n)             (((n) == 0) ? ADC0_CONFIGNUM \
                                      : 0x0UL)

/* Instance macros for EUSART */
#define EUSART(n)                    (((n) == 0) ? EUSART0   \
                                      : ((n) == 1) ? EUSART1 \
                                      : ((n) == 2) ? EUSART2 \
                                      : 0x0UL)
#define EUSART_NUM(ref)              (((ref) == EUSART0) ? 0   \
                                      : ((ref) == EUSART1) ? 1 \
                                      : ((ref) == EUSART2) ? 2 \
                                      : -1)
#define EUSART_EM2_CAPABLE(n)        (((n) == 0) ? EUSART0_EM2_CAPABLE   \
                                      : ((n) == 1) ? EUSART1_EM2_CAPABLE \
                                      : ((n) == 2) ? EUSART2_EM2_CAPABLE \
                                      : 0x0UL)
#define EUSART_EXCLUDE_DALI(n)       (((n) == 0) ? EUSART0_EXCLUDE_DALI   \
                                      : ((n) == 1) ? EUSART1_EXCLUDE_DALI \
                                      : ((n) == 2) ? EUSART2_EXCLUDE_DALI \
                                      : 0x0UL)
#define EUSART_FIFO_DEPTH(n)         (((n) == 0) ? EUSART0_FIFO_DEPTH   \
                                      : ((n) == 1) ? EUSART1_FIFO_DEPTH \
                                      : ((n) == 2) ? EUSART2_FIFO_DEPTH \
                                      : 0x0UL)
#define EUSART_NOT_EM2_CAPABLE(n)    (((n) == 0) ? EUSART0_NOT_EM2_CAPABLE   \
                                      : ((n) == 1) ? EUSART1_NOT_EM2_CAPABLE \
                                      : ((n) == 2) ? EUSART2_NOT_EM2_CAPABLE \
                                      : 0x0UL)

/* Instance macros for GPCRC */
#define GPCRC(n)                     (((n) == 0) ? GPCRC0 \
                                      : 0x0UL)
#define GPCRC_NUM(ref)               (((ref) == GPCRC0) ? 0 \
                                      : -1)

/* Instance macros for I2C */
#define I2C(n)                       (((n) == 0) ? I2C0   \
                                      : ((n) == 1) ? I2C1 \
                                      : ((n) == 2) ? I2C2 \
                                      : 0x0UL)
#define I2C_NUM(ref)                 (((ref) == I2C0) ? 0   \
                                      : ((ref) == I2C1) ? 1 \
                                      : ((ref) == I2C2) ? 2 \
                                      : -1)
#define I2C_DELAY(n)                 (((n) == 0) ? I2C0_DELAY   \
                                      : ((n) == 1) ? I2C1_DELAY \
                                      : ((n) == 2) ? I2C2_DELAY \
                                      : 0x0UL)
#define I2C_DELAY_CHAIN_NUM(n)       (((n) == 0) ? I2C0_DELAY_CHAIN_NUM   \
                                      : ((n) == 1) ? I2C1_DELAY_CHAIN_NUM \
                                      : ((n) == 2) ? I2C2_DELAY_CHAIN_NUM \
                                      : 0x0UL)

/* Instance macros for LDMA */
#define LDMA(n)                      (((n) == 0) ? LDMA0 \
                                      : 0x0UL)
#define LDMA_NUM(ref)                (((ref) == LDMA0) ? 0 \
                                      : -1)
#define LDMA_AHB1_ASYNC(n)           (((n) == 0) ? LDMA0_AHB1_ASYNC \
                                      : 0x0UL)
#define LDMA_CH_BITS(n)              (((n) == 0) ? LDMA0_CH_BITS \
                                      : 0x0UL)
#define LDMA_CH_NUM(n)               (((n) == 0) ? LDMA0_CH_NUM \
                                      : 0x0UL)
#define LDMA_DUALDSTCHNL(n)          (((n) == 0) ? LDMA0_DUALDSTCHNL \
                                      : 0x0UL)
#define LDMA_DUALDSTCHNL_DIS(n)      (((n) == 0) ? LDMA0_DUALDSTCHNL_DIS \
                                      : 0x0UL)
#define LDMA_FIFO_BITS(n)            (((n) == 0) ? LDMA0_FIFO_BITS \
                                      : 0x0UL)
#define LDMA_FIFO_DEPTH(n)           (((n) == 0) ? LDMA0_FIFO_DEPTH \
                                      : 0x0UL)
#define LDMA_ILCHNL(n)               (((n) == 0) ? LDMA0_ILCHNL \
                                      : 0x0UL)
#define LDMA_ILCHNL_DIS(n)           (((n) == 0) ? LDMA0_ILCHNL_DIS \
                                      : 0x0UL)
#define LDMA_ULDMA_MODE(n)           (((n) == 0) ? LDMA0_ULDMA_MODE \
                                      : 0x0UL)
#define LDMA_ADDRDEC_DIS(n)          (((n) == 0) ? LDMA0_ADDRDEC_DIS \
                                      : 0x0UL)
#define LDMA_ARBSLOT_DIS(n)          (((n) == 0) ? LDMA0_ARBSLOT_DIS \
                                      : 0x0UL)
#define LDMA_CHAN_CFG_DIS(n)         (((n) == 0) ? LDMA0_CHAN_CFG_DIS \
                                      : 0x0UL)
#define LDMA_EXTENDED_DIS(n)         (((n) == 0) ? LDMA0_EXTENDED_DIS \
                                      : 0x0UL)
#define LDMA_IGNORESREQ_DIS(n)       (((n) == 0) ? LDMA0_IGNORESREQ_DIS \
                                      : 0x0UL)
#define LDMA_LOOP_DIS(n)             (((n) == 0) ? LDMA0_LOOP_DIS \
                                      : 0x0UL)
#define LDMA_MULTIMASTER_DIS(n)      (((n) == 0) ? LDMA0_MULTIMASTER_DIS \
                                      : 0x0UL)
#define LDMA_RELADDR_DIS(n)          (((n) == 0) ? LDMA0_RELADDR_DIS \
                                      : 0x0UL)
#define LDMA_REQMODE_DIS(n)          (((n) == 0) ? LDMA0_REQMODE_DIS \
                                      : 0x0UL)
#define LDMA_SYNC_NUM(n)             (((n) == 0) ? LDMA0_SYNC_NUM \
                                      : 0x0UL)
#define LDMA_SYNC_DIS(n)             (((n) == 0) ? LDMA0_SYNC_DIS \
                                      : 0x0UL)
#define LDMA_WRI_DIS(n)              (((n) == 0) ? LDMA0_WRI_DIS \
                                      : 0x0UL)
#define LDMA_XFERTYPE_DIS(n)         (((n) == 0) ? LDMA0_XFERTYPE_DIS \
                                      : 0x0UL)

/* Instance macros for LDMAXBAR */
#define LDMAXBAR(n)                  (((n) == 0) ? LDMAXBAR0 \
                                      : 0x0UL)
#define LDMAXBAR_NUM(ref)            (((ref) == LDMAXBAR0) ? 0 \
                                      : -1)
#define LDMAXBAR_CH_BITS(n)          (((n) == 0) ? LDMAXBAR0_CH_BITS \
                                      : 0x0UL)
#define LDMAXBAR_CH_NUM(n)           (((n) == 0) ? LDMAXBAR0_CH_NUM \
                                      : 0x0UL)
#define LDMAXBAR_SIGSEL_W(n)         (((n) == 0) ? LDMAXBAR0_SIGSEL_W \
                                      : 0x0UL)
#define LDMAXBAR_SOURCESEL_W(n)      (((n) == 0) ? LDMAXBAR0_SOURCESEL_W \
                                      : 0x0UL)

/* Instance macros for LETIMER */
#define LETIMER(n)                   (((n) == 0) ? LETIMER0 \
                                      : 0x0UL)
#define LETIMER_NUM(ref)             (((ref) == LETIMER0) ? 0 \
                                      : -1)
#define LETIMER_CNT_WIDTH(n)         (((n) == 0) ? LETIMER0_CNT_WIDTH \
                                      : 0x0UL)

/* Instance macros for PCNT */
#define PCNT(n)                      (((n) == 0) ? PCNT0 \
                                      : 0x0UL)
#define PCNT_NUM(ref)                (((ref) == PCNT0) ? 0 \
                                      : -1)
#define PCNT_PCNT_WIDTH(n)           (((n) == 0) ? PCNT0_PCNT_WIDTH \
                                      : 0x0UL)

/* Instance macros for PIXELRZ */
#define PIXELRZ(n)                   (((n) == 0) ? PIXELRZ0   \
                                      : ((n) == 1) ? PIXELRZ1 \
                                      : 0x0UL)
#define PIXELRZ_NUM(ref)             (((ref) == PIXELRZ0) ? 0   \
                                      : ((ref) == PIXELRZ1) ? 1 \
                                      : -1)

/* Instance macros for SEMAPHORE */
#define SEMAPHORE(n)                 (((n) == 0) ? SEMAPHORE0   \
                                      : ((n) == 1) ? SEMAPHORE1 \
                                      : 0x0UL)
#define SEMAPHORE_NUM(ref)           (((ref) == SEMAPHORE0) ? 0   \
                                      : ((ref) == SEMAPHORE1) ? 1 \
                                      : -1)

/* Instance macros for SOCPLL */
#define SOCPLL(n)                    (((n) == 0) ? SOCPLL0 \
                                      : 0x0UL)
#define SOCPLL_NUM(ref)              (((ref) == SOCPLL0) ? 0 \
                                      : -1)

/* Instance macros for TIMER */
#define TIMER(n)                     (((n) == 0) ? TIMER0   \
                                      : ((n) == 1) ? TIMER1 \
                                      : ((n) == 2) ? TIMER2 \
                                      : ((n) == 3) ? TIMER3 \
                                      : 0x0UL)
#define TIMER_NUM(ref)               (((ref) == TIMER0) ? 0   \
                                      : ((ref) == TIMER1) ? 1 \
                                      : ((ref) == TIMER2) ? 2 \
                                      : ((ref) == TIMER3) ? 3 \
                                      : -1)
#define TIMER_CC_NUM(n)              (((n) == 0) ? TIMER0_CC_NUM   \
                                      : ((n) == 1) ? TIMER1_CC_NUM \
                                      : ((n) == 2) ? TIMER2_CC_NUM \
                                      : ((n) == 3) ? TIMER3_CC_NUM \
                                      : 0x0UL)
#define TIMER_CNTWIDTH(n)            (((n) == 0) ? TIMER0_CNTWIDTH   \
                                      : ((n) == 1) ? TIMER1_CNTWIDTH \
                                      : ((n) == 2) ? TIMER2_CNTWIDTH \
                                      : ((n) == 3) ? TIMER3_CNTWIDTH \
                                      : 0x0UL)
#define TIMER_DITHERWIDTH(n)         (((n) == 0) ? TIMER0_DITHERWIDTH   \
                                      : ((n) == 1) ? TIMER1_DITHERWIDTH \
                                      : ((n) == 2) ? TIMER2_DITHERWIDTH \
                                      : ((n) == 3) ? TIMER3_DITHERWIDTH \
                                      : 0x0UL)
#define TIMER_DTI(n)                 (((n) == 0) ? TIMER0_DTI   \
                                      : ((n) == 1) ? TIMER1_DTI \
                                      : ((n) == 2) ? TIMER2_DTI \
                                      : ((n) == 3) ? TIMER3_DTI \
                                      : 0x0UL)
#define TIMER_DTI_CC_NUM(n)          (((n) == 0) ? TIMER0_DTI_CC_NUM   \
                                      : ((n) == 1) ? TIMER1_DTI_CC_NUM \
                                      : ((n) == 2) ? TIMER2_DTI_CC_NUM \
                                      : ((n) == 3) ? TIMER3_DTI_CC_NUM \
                                      : 0x0UL)
#define TIMER_NO_DTI(n)              (((n) == 0) ? TIMER0_NO_DTI   \
                                      : ((n) == 1) ? TIMER1_NO_DTI \
                                      : ((n) == 2) ? TIMER2_NO_DTI \
                                      : ((n) == 3) ? TIMER3_NO_DTI \
                                      : 0x0UL)

/* Instance macros for WDOG */
#define WDOG(n)                      (((n) == 0) ? WDOG0   \
                                      : ((n) == 1) ? WDOG1 \
                                      : 0x0UL)
#define WDOG_NUM(ref)                (((ref) == WDOG0) ? 0   \
                                      : ((ref) == WDOG1) ? 1 \
                                      : -1)
#define WDOG_PCNUM(n)                (((n) == 0) ? WDOG0_PCNUM   \
                                      : ((n) == 1) ? WDOG1_PCNUM \
                                      : 0x0UL)

/** @} End of group SIBG301M104XIL_Peripheral_Parameters  */

/** @} End of group SIBG301M104XIL */
/** @}} End of group Parts */

#ifdef __cplusplus
}
#endif
#endif
