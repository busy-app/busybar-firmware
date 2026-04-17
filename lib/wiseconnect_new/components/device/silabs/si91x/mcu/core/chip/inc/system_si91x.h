/******************************************************************************
* @file  system_si91x.h
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

#include <stdint.h>

#ifndef __SYSTEM_ARMCM4_H__
#define __SYSTEM_ARMCM4_H__

/**
 * 
 */
#include "rsi_error.h"
#include "rsi_ccp_user_config.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t SystemCoreClock; /*!< System Clock Frequency (Core Clock)  */
extern uint32_t npssIntrState;
extern uint32_t __sp;
extern uint32_t SiliconRev;
extern uint32_t package_type;

void RSI_PS_ConfigureTaMemories(void);
/*WiSeAOC specific VTOR values*/
#if defined(WISE_AOC_MODE)
#define VECTOR_OFF_SET_TA_ROM        (0x100000 + 0x70100) /*<! M4 execution from NWP ROM                  */
#define VECTOR_OFF_SET_M4_ROM        (0x300000 + 0xB100)  /*<! M4 execution from M4 ROM                  */
#define VECTOR_OFF_SET_TA_ROM_DIRECT (0x80000 + 0x70100)  /*<! M4 execution from NWP ROM in directed mode */
#endif                                                    //WISE_AOC_MODE

typedef enum SLEEP_TYPE {
  SLEEP_WITH_RETENTION,    /*Saves the CPU context while entering into sleep */
  SLEEP_WITHOUT_RETENTION, /*do not Save the CPU context while entering into sleep */
} SLEEP_TYPE_T;

#define DISABLE_LF_MODE           0x00
#define LF_32_KHZ_RC              BIT(0)
#define LF_32_KHZ_XTAL            BIT(1)
#define EXTERNAL_CAP_MODE         BIT(2)
#define HF_MHZ_RO                 3
#define BG_SLEEP_TIMER_REG_OFFSET 0x125
/*System default clocks*/
#define DEFAULT_SOC_PLL_CLOCK    0
#define DEFAULT_INTF_PLL_CLOCK   0
#define DEFAULT_MODEM_PLL_CLOCK  80000000
#define DEFAULT_MHZ_RC_CLOCK     32000000 /* Not recommended to use for applicatios */
#define DEFAULT_20MHZ_RO_CLOCK   20000000
#define DEFAULT_DOUBLER_CLOCK    (2 * DEFAULT_20MHZ_RO_CLOCK)
#define DEFAULT_32KHZ_RC_CLOCK   32000
#define DEFAULT_32KHZ_RO_CLOCK   32000
#define DEFAULT_32KHZ_XTAL_CLOCK 32768
#define DEFAULT_40MHZ_CLOCK      40000000
#define DEFAULT_MEMS_REF_CLOCK   40000000
#define DEFAULT_BYP_RC_CLOCK     32000000
#define DEFAULT_I2S_PLL_CLOCK    0
#define DEFAULT_REF_CLOCK        2

/* Selecting the PLL reference clock */
/* 0 - XTAL_CLK, 1 - Reserved, 2 - Reserved, 3 - Reserved */
#define PLL_REF_CLK_CONFIG_REG (*(volatile uint32_t *)(0x46180000UL + 0x00008000 + 0x04))
#define SELECT_RC_MHZ_CLOCK    BIT(15)
#define SELECT_XTAL_MHZ_CLOCK  ~(BIT(14) | BIT(15))
#define XTAL_CLK_FREQ          40000000UL

#define M4SS_P2P_INT_BASE_ADDRESS 0x46008000
#ifdef SLI_SI91X_MCU_COMMON_FLASH_MODE
#define NWPAON_MEM_HOST_ACCESS_CTRL_SET_1   (*(volatile uint32_t *)(0x41300000 + 0x0))
#define NWPAON_MEM_HOST_ACCESS_CTRL_CLEAR_1 (*(volatile uint32_t *)(0x41300000 + 0x4))
#define MCUAON_CONTROL_REG4                 (*(volatile uint32_t *)(0x24048600 + 0x10))

#define M4SS_TASS_CTRL_SET_REG_VAL (*(volatile uint32_t *)(0x24048400 + 0x34))
#define M4SS_TASS_CTRL_CLR_REG_VAL (*(volatile uint32_t *)(0x24048400 + 0x38))
#define M4SS_P2P_INTR_CLR_REG      *(volatile uint32_t *)(M4SS_P2P_INT_BASE_ADDRESS + 0x170)
#define M4SS_REF_CLK_MUX_CTRL      BIT(24)
#define TASS_REF_CLK_MUX_CTRL      BIT(25)
#if defined(SLI_SI917)
#define MCU_TASS_REF_CLK_SEL_MUX_CTRL BIT(8)
#endif
#define M4SS_CTRL_TASS_AON_PWR_DMN_RST_BYPASS_BIT BIT(2)
#define M4_USING_FLASH                            BIT(3)
#endif
#define M4SS_P2P_INTR_SET_REG  *(volatile uint32_t *)(M4SS_P2P_INT_BASE_ADDRESS + 0x16C)
#define P2P_STATUS_REG         *(volatile uint32_t *)(M4SS_P2P_INT_BASE_ADDRESS + 0x174)
#define TASS_P2P_INTR_MASK_CLR *(volatile uint32_t *)(M4SS_P2P_INT_BASE_ADDRESS + 0x17C)

#define M4_is_active    BIT(1)
#define RX_BUFFER_VALID BIT(1)

#define TA_USING_FLASH          BIT(3)
#define PROGRAM_COMMON_FLASH    BIT(4)
#define M4_WAKEUP_TA            BIT(0)
#define TA_IS_ACTIVE            BIT(3)
#define TASS_P2P_INTR_CLEAR_REG *(volatile uint32_t *)(M4SS_P2P_INT_BASE_ADDRESS + 0x180)

/* Flash offset to read Silicon revision */
#define TA_FLASH_BASE 0x04000000
#ifdef CHIP_9118
#define SILICON_REV_WMCU  (*(volatile uint32_t *)(TA_FLASH_BASE + 1024 + 55))
#define PACKAGE_TYPE_WMCU (*(volatile uint32_t *)(TA_FLASH_BASE + 992 + 86))
#endif

#if defined(SLI_SI917B0)
#ifdef SLI_SI91X_MCU_COMMON_FLASH_MODE
#define SILICON_REV_WMCU  (*(volatile uint32_t *)(SILICON_REV_VALUES_OFFSET_COMMON_FLASH))
#define PACKAGE_TYPE_WMCU (*(volatile uint32_t *)(PACKAGE_TYPE_VALUES_OFFSET_COMMON_FLASH))
#else
#define SILICON_REV_WMCU  (*(volatile uint32_t *)(SILICON_REV_VALUES_OFFSET_DUAL_FLASH))
#define PACKAGE_TYPE_WMCU (*(volatile uint32_t *)(PACKAGE_TYPE_VALUES_OFFSET_DUAL_FLASH))
#endif
#endif

#define M4_FLASH_BASE    0x08000000
#define SILICON_REV_MCU  (*(volatile uint32_t *)(M4_FLASH_BASE + 1024 + 55))
#define PACKAGE_TYPE_MCU (*(volatile uint32_t *)(M4_FLASH_BASE + 992 + 86))

/* Board capabilities */
#define SLI_CRYPTOACC_PRESENT_SI91X

/*XTAL bypass from MCU macros */
#define XTAL_IS_IN_SW_CTRL_FROM_M4 0
#define XTAL_DISABLE_FROM_M4       0

/* system clock source look up table*/
typedef struct SYSTEM_CLOCK_SOURCE_FREQUENCIES {
  uint32_t m4ss_ref_clk;
  uint32_t ulpss_ref_clk;
  uint32_t soc_pll_clock;
  uint32_t soc_clock;
  uint32_t modem_pll_clock;
  uint32_t intf_pll_clock;
  uint32_t i2s_pll_clock;
  uint32_t modem_pll_clock2;
  uint32_t ro_32khz_clock;
  uint32_t rc_32khz_clock;
  uint32_t xtal_32khz_clock;
  uint32_t rc_mhz_clock;
  uint32_t ro_20mhz_clock;
  uint32_t doubler_clock;
  uint32_t sleep_clock;
  uint32_t rf_ref_clock;
  uint32_t mems_ref_clock;
  uint32_t byp_rc_ref_clock;
  uint32_t m4_ref_clock_source;
  uint32_t ulp_ref_clock_source;
} SYSTEM_CLOCK_SOURCE_FREQUENCIES_T;

typedef struct sl_p2p_intr_status_bkp_s {
  uint32_t tass_p2p_intr_mask_clr_bkp;
  uint32_t m4ss_p2p_intr_set_reg_bkp;
} sl_p2p_intr_status_bkp_t;

extern SYSTEM_CLOCK_SOURCE_FREQUENCIES_T system_clocks;

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 */

/**
 * \defgroup RSI_SYSTEM_RS1XXXX RSI:SYSTEM RS1XXXX  
 *  @{
 *
 */

#define SI91X_EXT_IRQ_COUNT        75 /**< Number of External (NVIC) interrupts of M4*/
#define SI91X_RESERVED_IRQ_COUNT   16
#define SI91X_VECTOR_TABLE_ENTRIES (SI91X_RESERVED_IRQ_COUNT + SI91X_EXT_IRQ_COUNT)

typedef union {
  void (*VECTOR_TABLE_Type)(void);
  void *topOfStack;
} tVectorEntry;

#ifndef __VECTOR_TABLE
#define __VECTOR_TABLE __Vectors
#endif
#ifndef __VECTOR_TABLE_ATTRIBUTE
#define __VECTOR_TABLE_ATTRIBUTE __attribute__((used, section(".isr_vector")))
#endif
extern void SystemInit(void);

extern void SystemCoreClockUpdate(void);

static inline uint32_t SystemCoreClockGet(void)
{
  return SystemCoreClock;
}

void RSI_Set_Cntrls_To_M4(void);
void RSI_Set_Cntrls_To_TA(void);

rsi_error_t RSI_PS_EnterDeepSleep(SLEEP_TYPE_T sleepType, uint8_t lf_clk_mode);

void RSI_PS_SaveCpuContext(void);
void RSI_PS_RestoreCpuContext(void);
void SystemCoreClockUpdate(void);
void fpuInit(void);
void RSI_Save_Context(void);
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

/*Boot up functions*/
void RSI_Default_Reset_Handler(void);
void RSI_Default_WakeUp_Handler(void);
void RSI_PS_RestoreCpuContext(void);

void IRQ000_Handler(void);                                           /*!<  ULP Processor Interrupt   0      */
void IRQ001_Handler(void);                                           /*!<  ULP Processor Interrupt   1      */
void IRQ002_Handler(void);                                           /*!<  ULP Processor Interrupt   2      */
void IRQ003_Handler(void);                                           /*!<  ULP Processor Interrupt   3      */
void IRQ004_Handler(void);                                           /*!<  ULP Processor Interrupt   4      */
void IRQ005_Handler(void);                                           /*!<  ULP Processor Interrupt   5      */
void IRQ006_Handler(void);                                           /*!<  ULP Processor Interrupt   6      */
void IRQ007_Handler(void);                                           /*!<  ULP Processor Interrupt   7      */
void IRQ008_Handler(void);                                           /*!<  ULP Processor Interrupt   8      */
void IRQ009_Handler(void);                                           /*!<  ULP Processor Interrupt   9      */
void IRQ010_Handler(void);                                           /*!<  ULP Processor Interrupt   10     */
void IRQ011_Handler(void);                                           /*!<  ULP Processor Interrupt   11     */
void IRQ012_Handler(void);                                           /*!<  ULP Processor Interrupt   12     */
void IRQ013_Handler(void);                                           /*!<  ULP Processor Interrupt   13     */
void IRQ014_Handler(void);                                           /*!<  ULP Processor Interrupt   14     */
void IRQ015_Handler(void);                                           /*!<  ULP Processor Interrupt   15     */
void IRQ016_Handler(void);                                           /*!<  ULP Processor Interrupt   15     */
void IRQ017_Handler(void);                                           /*!<  ULP Processor Interrupt   17     */
void IRQ018_Handler(void);                                           /*!<  ULP Processor Interrupt   18     */
void IRQ019_Handler(void);                                           /*!<  ULP Processor Interrupt   19     */
void IRQ020_Handler(void); /*!<  Sleep Sensor Interrupts   0      */ /*WDT*/
void IRQ021_Handler(void);                                           /*!<  Sleep Sensor Interrupts   1      */
void IRQ022_Handler(void);                                           /*!<  Sleep Sensor Interrupts   2      */
void IRQ023_Handler(void);                                           /*!<  Sleep Sensor Interrupts   3      */
void IRQ024_Handler(void);                                           /*!<  Sleep Sensor Interrupts   4      */
void IRQ025_Handler(void);                                           /*!<  Sleep Sensor Interrupts   5      */
void IRQ026_Handler(void);                                           /*!<  Sleep Sensor Interrupts   6      */
void IRQ027_Handler(void);                                           /*!<  Sleep Sensor Interrupts   7      */
void IRQ028_Handler(void); /*!<  Sleep Sensor Interrupts   8      */ /*Alarm*/
void IRQ029_Handler(void); /*!<  Sleep Sensor Interrupts   9      */ /*Msec and sec interrupt */
void IRQ030_Handler(void);                                           /*!<  Reserved                         */
void IRQ031_Handler(void);                                           /*!<  M4SS DMA interrupt               */
void IRQ032_Handler(void);                                           /*!<  Reserved                         */
void IRQ033_Handler(void);                                           /*!<  M4SS DMA interrupt               */
void IRQ034_Handler(void);                                           /*!<  M4SS SCT interrupt               */
void HIF1_IRQHandler(void);                                          /*!<  HIF Interrupt 1                  */
void HIF2_IRQHandler(void);                                          /*!<  HIF Interrupt 2                  */
void IRQ037_Handler(void);                                           /*!<  SIO Interrupt                    */
void IRQ038_Handler(void);                                           /*!<  USART 1 Interrupt                */
void IRQ039_Handler(void);                                           /*!<  Reserved                         */
void IRQ040_Handler(void);                                           /*!<  Reserved                         */
void IRQ041_Handler(void);                                           /*!<  Reserved                         */
void IRQ042_Handler(void);                                           /*!<  I2C Interrupt                    */
void IRQ043_Handler(void);                                           /*!<  Reserved                         */
void IRQ044_Handler(void);                                           /*!<  SSI Slave Interrupt              */
void IRQ045_Handler(void);                                           /*!<  Reserved                         */
void IRQ046_Handler(void);                                           /*!<  GSPI Master 1 Interrupt          */
void IRQ047_Handler(void);                                           /*!<  Reserved                         */
void IRQ048_Handler(void);                                           /*!<  MCPWM Interrupt                  */
void IRQ049_Handler(void);                                           /*!<  QEI Interrupt                    */
void IRQ050_Handler(void);                                           /*!<  GPIO Group Interrupt 0           */
void IRQ051_Handler(void);                                           /*!<  GPIO Group Interrupt 1           */
void IRQ052_Handler(void);                                           /*!<  GPIO Pin Interrupt   0           */
void IRQ053_Handler(void);                                           /*!<  GPIO Pin Interrupt   1           */
void IRQ054_Handler(void);                                           /*!<  GPIO Pin Interrupt   2           */
void IRQ055_Handler(void);                                           /*!<  GPIO Pin Interrupt   3           */
void IRQ056_Handler(void);                                           /*!<  GPIO Pin Interrupt   4           */
void IRQ057_Handler(void);                                           /*!<  GPIO Pin Interrupt   5           */
void IRQ058_Handler(void);                                           /*!<  GPIO Pin Interrupt   6           */
void IRQ059_Handler(void);                                           /*!<  GPIO Pin Interrupt   7           */
void IRQ060_Handler(void);                                           /*!<  QSPI Interrupt                   */
void IRQ061_Handler(void);                                           /*!<  I2C 2 Interrupt                  */
void IRQ062_Handler(void);                                           /*!<  Ethernet Interrupt               */
void IRQ063_Handler(void);                                           /*!<  Reserved                         */
void IRQ064_Handler(void);                                           /*!<  I2S master Interrupt             */
void IRQ065_Handler(void);                                           /*!<  Reserved                         */
void IRQ066_Handler(void);                                           /*!<  Can 1 Interrupt                  */
void IRQ067_Handler(void);                                           /*!<  Reserved                         */
void IRQ068_Handler(void);                                           /*!<  SDMEM Interrupt                  */
void IRQ069_Handler(void);                                           /*!<  PLL clock ind Interrupt          */
void IRQ070_Handler(void);                                           /*!<  Reserved                         */
void IRQ071_Handler(void);                                           /*!<  CCI system Interrupt Out         */
void IRQ072_Handler(void);                                           /*!<  FPU exception                    */
void IRQ073_Handler(void);                                           /*!<  USB INTR                         */
void IRQ074_Handler(void);                                           /*!<  TASS_P2P_INTR                    */
#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_ARMCM4_H__ */
