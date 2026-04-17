/******************************************************************************
* @file  rsi_deepsleep_soc.c
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
#include "rsi_ccp_common.h"
#include "rsi_power_save.h"
#include "rsi_temp_sensor.h"
#include "rsi_retention.h"
#ifdef DEBUG_UART
#include "rsi_debug.h"
#endif

#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1)
#include "rsi_m4.h"
#endif

void fpuInit(void);
#define NWPAON_MEM_HOST_ACCESS_CTRL_CLEAR_1 (*(volatile uint32_t *)(0x41300000 + 0x4))
#define NWPAON_MEM_HOST_ACCESS_CTRL_SET_1   (*(volatile uint32_t *)(0x41300000 + 0x0))
#define M4SS_TASS_CTRL_SET_REG              (*(volatile uint32_t *)(0x24048400 + 0x34))
#define M4SS_TASS_CTRL_CLEAR_REG            (*(volatile uint32_t *)(0x24048400 + 0x38))
#define M4SS_TASS_CTRL_CLR_REG              (*(volatile uint32_t *)(0x24048400 + 0x38))
#define MAX_NVIC_REGS                       3    // Max Interrupts register
#define MAX_IPS                             240  // Max Interrupt Priority registers
#define MAX_SHP                             12   // Max System Handlers Priority registers
#define NPSS_GPIO_CLR_VALUE                 0x3E // NPSS GPIO rising edge interrupt clear value

#ifdef SLI_SI91X_MCU_4MB_LITE_IMAGE
#define MBR_MAGIC_WORD (*(volatile uint32_t *)(0x8160000))
#else
#define MBR_MAGIC_WORD (*(volatile uint32_t *)(0x81F0000))
#endif

#define SL_SCDC_SLEEP  1
#define SL_SCDC_ACTIVE 0

/* MACRO definition for External LDO Handle, the user must provide the definition for External LDO ON/OFF Handle*/
#ifndef EXTERNAL_LDO_HANDLE
/*If User Handle definition is not available keep a Null Definition*/
#define EXTERNAL_LDO_HANDLE
#endif

#define COPY_CONFIG_REGS(dest, src, max_regs) \
  for (var = 0; var < (max_regs); ++var) {    \
    (dest)[var] = (src)[var];                 \
  }

extern void set_scdc(uint32_t Deepsleep);

#ifdef SLI_SI91X_MCU_ENABLE_PSRAM_FEATURE

#if defined(SLI_SI91X_MCU_PSRAM_APS1604M_SQR)
#include "sl_si91x_psram_aps1604m_sqr_config.h"
#elif defined(SLI_SI91X_MCU_PSRAM_APS6404L_SQH)
#include "sl_si91x_psram_aps6404l_sqh_config.h"
#elif defined(SLI_SI91X_MCU_PSRAM_APS6404L_SQRH)
#include "sl_si91x_psram_aps6404l_sqrh_config.h"
#else
#error "No valid PSRAM configuration defined"
#endif

#endif

uint32_t nvic_enable[MAX_NVIC_REGS] = { 0 };
uint8_t nvic_ip_reg[MAX_IPS]        = { 0 };
uint8_t scs_shp_reg[MAX_SHP]        = { 0 };
volatile uint32_t msp_value;
volatile uint32_t psp_value;
volatile uint32_t control_reg_val;
uint32_t npss_gpio_config = 0;

#if defined(SLI_SI91X_MCU_ENABLE_RAM_BASED_EXECUTION)
extern char ram_vector[SI91X_VECTOR_TABLE_ENTRIES];
#endif

/**
 * @fn           void RSI_Save_Context(void)
 * @brief        This function is to save Stack pointer value and Control registers.
 *
 */
void RSI_Save_Context(void)
{
  msp_value       = __get_MSP();
  psp_value       = __get_PSP();
  control_reg_val = __get_CONTROL();
}
/**
 * @fn           void RSI_Restore_Context(void)
 * @brief        This function is to Restore Stack pointer value and Control registers.
 *
 */
#ifdef SLI_SI91X_ENABLE_OS
STATIC INLINE void RSI_Restore_Context(void)
{
  __set_CONTROL(control_reg_val);
  __set_PSP(psp_value);
  __set_MSP(msp_value);
}
#endif
#if defined(__CC_ARM) /*------------------ARM CC Compiler -----------------*/
/**
 * @fn          __asm  void RSI_PS_SaveCpuContext(void)
 * @brief        This API is used to save the CPU status register into RAM, this API should be used when sleep with RET is required
 * @return       none
 */
__asm void RSI_PS_SaveCpuContext(void)
{
  IMPORT __sp;
  PUSH{ r0 };
  PUSH{ r1 };
  PUSH{ r2 };
  PUSH{ r3 };
  PUSH{ r4 };
  PUSH{ r5 };
  PUSH{ r6 };
  PUSH{ r7 };
  PUSH{ r8 };
  PUSH{ r9 };
  PUSH{ r10 };
  PUSH{ r11 };
  PUSH{ r12 };
  PUSH{ r14 };
  LDR r0, = __sp;
  MRS r1, msp;
  STR r1, [r0];
  WFI;
}

/**
 * @fn           void RSI_PS_RestoreCpuContext(void)
 * @brief        This API is used to restore the current CPU processing content from (POP) stack
 * @return       none
 */
__asm void RSI_PS_RestoreCpuContext(void)
{
  IMPORT __sp;
  LDR r0, = __sp;
  LDR sp, [r0, #0];
  POP{ r14 };
  POP{ r12 };
  POP{ r11 };
  POP{ r10 };
  POP{ r9 };
  POP{ r8 };
  POP{ r7 };
  POP{ r6 };
  POP{ r5 };
  POP{ r4 };
  POP{ r3 };
  POP{ r2 };
  POP{ r1 };
  POP{ r0 };
  BX LR;
}
#endif /*------------------ARM CC Compiler -----------------*/

#if defined(__GNUC__) /*------------------ GNU Compiler ---------------------*/

/**
 * @fn           void RSI_PS_SaveCpuContext(void)
 * @brief        This API is used to save the CPU status register into RAM, this API should be used when sleep with RET is required
 * @return       none
 */
void RSI_PS_SaveCpuContext(void)
{
  __asm("push {r0}");
  __asm("push {r1}");
  __asm("push {r2}");
  __asm("push {r3}");
  __asm("push {r4}");
  __asm("push {r5}");
  __asm("push {r6}");
  __asm("push {r7}");
  __asm("push {r8}");
  __asm("push {r9}");
  __asm("push {r10}");
  __asm("push {r11}");
  __asm("push {r12}");
  __asm("push {r14}");

  /*R13 Stack pointer */
  __asm("mov %0, sp\n\t" : "=r"(__sp));
  __asm("WFI");
}

/**
 * @fn           void RSI_PS_RestoreCpuContext(void)
 * @brief        This API is used to restore the current CPU processing content from (POP) stack
 * @return       none
 */
void RSI_PS_RestoreCpuContext(void)
{
#ifdef SL_SI91X_ENABLE_GCC_LTO
  __asm__ volatile("ldr r0, %[sym]\n" : : [sym] "m"(__sp) : "r0");
#else
  __asm("ldr r0 , =__sp");
#endif
  __asm("ldr sp , [r0 , #0]");
  __asm("pop {r14}");
  __asm("pop {r12}");
  __asm("pop {r11}");
  __asm("pop {r10}");
  __asm("pop {r9}");
  __asm("pop {r8}");
  __asm("pop {r7}");
  __asm("pop {r6}");
  __asm("pop {r5}");
  __asm("pop {r4}");
  __asm("pop {r3}");
  __asm("pop {r2}");
  __asm("pop {r1}");
  __asm("pop {r0}");
}
#endif /*------------------ GNU Compiler ---------------------*/

#if defined(__ICCARM__) /*------------------ IAR Compiler ---------------------*/
/**
 * @fn           void RSI_PS_SaveCpuContext(void)
 * @brief        This API is used to save the CPU status register into RAM, this API should be used when sleep with RET is required
 * @return       none
 */
void RSI_PS_SaveCpuContext(void)
{
  __asm("push {r0}");
  __asm("push {r1}");
  __asm("push {r2}");
  __asm("push {r3}");
  __asm("push {r4}");
  __asm("push {r5}");
  __asm("push {r6}");
  __asm("push {r7}");
  __asm("push {r8}");
  __asm("push {r9}");
  __asm("push {r10}");
  __asm("push {r11}");
  __asm("push {r12}");
  __asm("push {r14}");

  /*R13 Stack pointer */
  __asm("mov %0, sp\n\t" : "=r"(__sp));
  __asm("WFI");
}

/**
 * @fn           void RSI_PS_RestoreCpuContext(void)
 * @brief        This API is used to restore the current CPU processing content from (POP) stack
 * @return       none
 */
void RSI_PS_RestoreCpuContext(void)
{
  __asm("ldr r0 , =__sp");
  __asm("ldr sp , [r0 , #0]");
  __asm("pop {r14}");
  __asm("pop {r12}");
  __asm("pop {r11}");
  __asm("pop {r10}");
  __asm("pop {r9}");
  __asm("pop {r8}");
  __asm("pop {r7}");
  __asm("pop {r6}");
  __asm("pop {r5}");
  __asm("pop {r4}");
  __asm("pop {r3}");
  __asm("pop {r2}");
  __asm("pop {r1}");
  __asm("pop {r0}");
}
#endif /*------------------ IAR Compiler ---------------------*/

#ifdef SLI_SI91X_MCU_COMMON_FLASH_MODE
/**
 * @fn     void RSI_Set_Cntrls_To_M4(void)
 * @brief  This API is used to set m4ss_ref_clk_mux_ctrl ,tass_ref_clk_mux_ctrl, AON domain power supply controls
 *         from NWP to M4
 *
 *
 * @return none
 */
void RSI_Set_Cntrls_To_M4(void)
{
#ifdef SLI_SI917B0
  //!take TASS ref clock control to M4
  MCUAON_CONTROL_REG4 &= ~(MCU_TASS_REF_CLK_SEL_MUX_CTRL);
#else
  /* m4ss_ref_clk_mux_ctrl and tass_ref_clk_mux_ctr in M4 Control */
  NWPAON_MEM_HOST_ACCESS_CTRL_CLEAR_1 = (M4SS_REF_CLK_MUX_CTRL | TASS_REF_CLK_MUX_CTRL);
#endif
  /* M4SS controlling Power supply for TASS AON domain */
  BATT_FF->M4SS_TASS_CTRL_SET_REG_b.M4SS_CTRL_TASS_AON_PWRGATE_EN = ENABLE;
  /* M4SS controlling Power supply for TASS AON domains isolation enable in bypass mode*/
  BATT_FF->M4SS_TASS_CTRL_SET_REG_b.M4SS_CTRL_TASS_AON_DISABLE_ISOLATION_BYPASS = ENABLE;
  /* M4SS controlling Power supply for TASS AON domains reset pin in bypass mode. */
  M4SS_TASS_CTRL_CLR_REG = M4SS_CTRL_TASS_AON_PWR_DMN_RST_BYPASS_BIT;
  for (volatile uint8_t delay = 0; delay < 10; delay++) {
    __ASM("NOP");
  }
}
/**
 * @fn     void RSI_Set_Cntrls_To_TA(void)
 * @brief  This API is used to set m4ss_ref_clk_mux_ctrl ,tass_ref_clk_mux_ctrl ,AON domain power supply controls
 *         from M4 to NWP
 *
 *
 * @return none
 */
void RSI_Set_Cntrls_To_TA(void)
{
  /* tass_ref_clk_mux_ctr in NWP Control */
  NWPAON_MEM_HOST_ACCESS_CTRL_SET_1 = TASS_REF_CLK_MUX_CTRL;
}
#endif

#ifdef SLI_SI91X_MCU_COMMON_FLASH_MODE
/*===================================================*/
/**
 * @fn          void request_nwp_to_program_flash(uint8_t in_ps2_state)
 * @brief       to request NWP to program flash
 * @return      none
 */
STATIC INLINE void request_nwp_to_program_flash(uint8_t in_ps2_state)
{
  if (!in_ps2_state && !(M4SS_P2P_INTR_SET_REG & M4_USING_FLASH)) {
    //! check NWP wokeup or not
    if (!(P2P_STATUS_REG & TA_IS_ACTIVE)) {
      //! wakeup NWP
      P2P_STATUS_REG |= M4_WAKEUP_TA;

      //! wait for NWP active
      while (!(P2P_STATUS_REG & TA_IS_ACTIVE))
        ;
    }
    //! Check for TA_USING flash bit
    if (!(TASS_P2P_INTR_CLEAR_REG & TA_USING_FLASH)) {
      //! Request NWP to program flash
      //! raise an interrupt to NWP register
      M4SS_P2P_INTR_SET_REG = PROGRAM_COMMON_FLASH;

      //! Wait for NWP using flash bit
      while (!(TASS_P2P_INTR_CLEAR_REG & TA_USING_FLASH))
        ;
    }
    M4SS_P2P_INTR_SET_REG = M4_USING_FLASH;
  }
}
#endif

/**
 * @fn          rsi_error_t RSI_PS_EnterDeepSleep(SLEEP_TYPE_T sleepType , uint8_t lf_clk_mode)
 * @brief	    This is the common API to keep the system in sleep state. from all possible active states.
 * @param[in]   sleepType   : selects the retention or non retention mode of processor. refer 'SLEEP_TYPE_T'.
 *                              SLEEP_WITH_RETENTION : When this is used, user must configure the which RAMs to be retained during sleep by using the 'RSI_PS_SetRamRetention()' function.
 * @param[in]   lf_clk_mode : This parameter is used to switch the processor clock from high frequency clock to low-frequency clock. This is used in some critical power save cases.
 *                            0: 'DISABLE_LF_MODE' Normal mode of operation , recommended in most of the applications.
 *                            1: 'LF_32_KHZ_RC' Processor clock is configured to low-frequency RC clock
 *                            2: 'LF_32_KHZ_XTAL' Processor clock is configured to low-frequency XTAL clock
 *                            3: 'EXTERNAL_CAP_MODE' Switches the supply to internal cap mode 0.95V.
 * @par           note
 \n User must ensure the selected clocks are active before selecting the 'LF_32_KHZ_RC' and 'LF_32_KHZ_XTAL' clocks to the processor using this API.
 * @return      Returns the execution status.
 */
rsi_error_t RSI_PS_EnterDeepSleep(SLEEP_TYPE_T sleepType, uint8_t lf_clk_mode)
{
  volatile int var                       = 0;
  volatile int enable_sdcss_based_wakeup = 0;
  volatile int enable_m4ulp_retention    = 0;
  volatile int Temp;

  volatile uint8_t in_ps2_state = 0;

  uint32_t ipmuDummyRead         = 0;
  uint32_t m4ulp_ram_core_status = 0;
  uint32_t m4ulp_ram_peri_status = 0;
  uint32_t disable_pads_ctrl     = 0;
  uint32_t ulp_proc_clk          = 0;
  sl_p2p_intr_status_bkp_t p2p_intr_status_bkp;

  /* Save the NVIC registers */
  COPY_CONFIG_REGS(nvic_enable, NVIC->ISER, MAX_NVIC_REGS);

  /* Save the Interrupt Priority Register */
  COPY_CONFIG_REGS(nvic_ip_reg, NVIC->IP, MAX_IPS);

  /* Save the System Handlers Priority Registers */
  COPY_CONFIG_REGS(scs_shp_reg, SCB->SHP, MAX_SHP);

  /*store the NPSS interrupt mask clear status*/
  npssIntrState = NPSS_INTR_MASK_CLR_REG;
  // Stores the NPSS GPIO interrupt configurations
  npss_gpio_config = NPSS_GPIO_CONFIG_REG;

  /*Clear AUX and DAC pg enables */
  if (!((MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_b.SDCSS_BASED_WAKEUP_b)
        || (MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_b.ULPSS_BASED_WAKEUP_b))) {
    RSI_IPMU_PowerGateClr(AUXDAC_PG_ENB | AUXADC_PG_ENB);
  }
  /*Check the PS2 state or not*/
  if (M4_ULP_SLP_STATUS_REG & ULP_MODE_SWITCHED_NPSS) {

    in_ps2_state = 1U;

    if (!((MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_b.SDCSS_BASED_WAKEUP_b)
          || (MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_b.ULPSS_BASED_WAKEUP_b))) {
      disable_pads_ctrl = (ULP_SPI_MEM_MAP(0x141) & BIT(11)); // ULP PADS PDO Status
      ULP_SPI_MEM_MAP(0x141) &= ~(BIT(11));                   // ULP PADS PDO OFF
      enable_sdcss_based_wakeup = 1;
      RSI_PS_SetWkpSources(SDCSS_BASED_WAKEUP);
    }

    if (!(MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_b.M4ULP_RAM_RETENTION_MODE_EN_b)) {
      enable_m4ulp_retention = 1;
      m4ulp_ram_core_status  = RSI_PS_M4ssRamBanksGetPowerSts();
      m4ulp_ram_peri_status  = RSI_PS_M4ssRamBanksGetPeriPowerSts();
      RSI_PS_M4ssRamBanksPowerDown(RAM_BANK_7 | RAM_BANK_8 | RAM_BANK_9);
      RSI_PS_M4ssRamBanksPeriPowerDown(RAM_BANK_7 | RAM_BANK_8 | RAM_BANK_9);
#ifdef CHIP_9118
      RSI_PS_SetRamRetention(M4ULP_RAM_RETENTION_MODE_EN);
#endif
    }
  }
  /*Move to LP mode */
  RSI_IPMU_RetnLdoLpmode();
  if (sleepType == SLEEP_WITHOUT_RETENTION) {
    /*POC1 */
    RSI_IPMU_PocbiasCurrent();
    /*RO32K_00_EFUSE. */
    RSI_IPMU_RO32khzTrim00Efuse();
    /*RETN1 */
    RSI_IPMU_RetnLdoLpmode();
    /*RETN0 */
    RSI_IPMU_RetnLdoVoltsel();
  }

  if (!in_ps2_state && MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_b.ULPSS_BASED_WAKEUP_b) {
#if (XTAL_CAP_MODE == POWER_TARN_ALWAYS_USE)
    // disable the XTAL CAP mode
    RSI_IPMU_ProgramConfigData(lp_scdc_extcapmode);
#elif (XTAL_CAP_MODE == POWER_TARN_CONDITIONAL_USE)
    if (lf_clk_mode & BIT(4)) {
      RSI_PS_NpssPeriPowerUp(SLPSS_PWRGATE_ULP_MCUTS);
      /*configure the slope,nominal temperature and f2_nominal*/
      RSI_TS_Config(MCU_TEMP, 25);
      /*disable the bjt based temp sensor*/
      RSI_TS_RoBjtEnable(MCU_TEMP, 0);
      /*Enable the RO based temp sensor*/
      RSI_TS_Enable(MCU_TEMP, 1);
      /*update the temperature periodically*/
      RSI_Periodic_TempUpdate(TIME_PERIOD, 1, 0);
      /*read the temperature*/
      Temp = (int)RSI_TS_ReadTemp(MCU_TEMP);
      if (Temp > 45) {
        // disable the XTAL CAP mode
        RSI_IPMU_ProgramConfigData(lp_scdc_extcapmode);
      }
    }
#endif
  }

  /*Clear IPMU BITS*/
  RSI_PS_LatchCntrlClr(LATCH_TOP_SPI | LATCH_TRANSPARENT_HF | LATCH_TRANSPARENT_LF);

  ipmuDummyRead = MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP;
  (void)ipmuDummyRead;

  /*Update the SCB with Deep sleep BIT */
  SCB->SCR = 0x4;

  if (in_ps2_state) {
    /*Read processor clock */
    ulp_proc_clk = ULPCLK->ULP_TA_CLK_GEN_REG_b.ULP_PROC_CLK_SEL;

    /*LF processor clock configuration */
    switch (lf_clk_mode & 0x7) {
      case DISABLE_LF_MODE:
        /*Do nothing*/
        break;
      case LF_32_KHZ_RC:
        ULPCLK->ULP_TA_CLK_GEN_REG_b.ULP_PROC_CLK_SEL = 2U;
        break;
      case LF_32_KHZ_XTAL:
        ULPCLK->ULP_TA_CLK_GEN_REG_b.ULP_PROC_CLK_SEL = 3U;
        break;
      case HF_MHZ_RO:
        ULPCLK->ULP_TA_CLK_GEN_REG_b.ULP_PROC_CLK_SEL = 5U;
        break;
      default:
        return INVALID_PARAMETERS;
    }
    /* HF processor clock */
  }

  /* Before sleep,Reduce the SCDC voltage by one value */
  set_scdc(SL_SCDC_SLEEP);

#ifdef SLI_SI91X_MCU_ENABLE_PSRAM_FEATURE
#if (defined SLI_SI91X_MCU_INTERNAL_LDO_FOR_PSRAM)
  RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_EFUSE_PERI);
  if (sleepType == SLEEP_WITH_RETENTION) {
    //!enable flash LDO and PMU DCDC ON in M4 for PSRAM with retention
    MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_REG |= (LDO_FLASH_ON | PMU_DCDC_ON);
#if PSRAM_HALF_SLEEP_SUPPORTED != FALSE
    /* Put PSRAM device to sleep */
    sl_si91x_psram_sleep();
#endif
  }
#elif (defined SLI_SI91X_MCU_EXTERNAL_LDO_FOR_PSRAM)
#if PSRAM_HALF_SLEEP_SUPPORTED != FALSE
  if (sleepType == SLEEP_WITH_RETENTION) {
    /* Put PSRAM device to sleep */
    sl_si91x_psram_sleep();
  } else {
    /* External LDO handle for PSRAM */
    EXTERNAL_LDO_HANDLE;
  }
#endif
#endif
#endif

#ifdef SLI_SI91X_ENABLE_OS
  /* Save Stack pointer value and Control registers */
  RSI_Save_Context();
#endif
  /* Take backup before going to PowerSave */
  p2p_intr_status_bkp.tass_p2p_intr_mask_clr_bkp = TASS_P2P_INTR_MASK_CLR;
  p2p_intr_status_bkp.m4ss_p2p_intr_set_reg_bkp  = M4SS_P2P_INTR_SET_REG;

#if ((defined SLI_SI91X_MCU_COMMON_FLASH_MODE) && (!(defined(RAM_COMPILATION))))
  /* Reset M4_USING_FLASH bit before going to sleep */
  M4SS_P2P_INTR_CLR_REG = M4_USING_FLASH;
/*Before M4 is going to deep sleep , set m4ss_ref_clk_mux_ctrl ,tass_ref_clk_mux_ctr, AON domain power supply controls from M4 to NWP */
#ifdef SLI_SI917B0
  MCUAON_CONTROL_REG4 |= (MCU_TASS_REF_CLK_SEL_MUX_CTRL);
  MCUAON_CONTROL_REG4;
#else
  NWPAON_MEM_HOST_ACCESS_CTRL_SET_1 = TASS_REF_CLK_MUX_CTRL;
#endif
#endif
  /*Enter sleep with retention*/
  if (sleepType == SLEEP_WITH_RETENTION) {
    /*If retention mode is enabled save the CPU context*/
    RSI_PS_SaveCpuContext();
  } else {
    /*Clear RAM retentions*/
    RSI_PS_ClrRamRetention(M4ULP_RAM16K_RETENTION_MODE_EN | TA_RAM_RETENTION_MODE_EN | M4ULP_RAM_RETENTION_MODE_EN
                           | M4SS_RAM_RETENTION_MODE_EN);
    /*do not save CPU context and go to deep sleep */
    __asm("WFI");
  }
  //Disable the NVIC interrupts.
  __asm volatile("cpsid i" ::: "memory");
  __asm volatile("dsb");
  __asm volatile("isb");
#ifdef SLI_SI91X_MCU_COMMON_FLASH_MODE
  /* if flash is not initialised, then raise a request to NWP */
  request_nwp_to_program_flash(in_ps2_state);
#endif
#ifdef SLI_SI91X_MCU_ENABLE_PSRAM_FEATURE
#if PSRAM_HALF_SLEEP_SUPPORTED != FALSE
  /* Exit PSRAM device from sleep */
  sl_si91x_psram_wakeup();
#endif
#endif

#ifdef SLI_SI91X_ENABLE_OS
  /* Restore Stack pointer value and Control registers */
  RSI_Restore_Context();
#endif
  /*Restore the default value to the processor clock */
  if (in_ps2_state) {
    ULPCLK->ULP_TA_CLK_GEN_REG_b.ULP_PROC_CLK_SEL = (unsigned int)(ulp_proc_clk & 0xF);
  }

#if ((defined SLI_SI91X_MCU_COMMON_FLASH_MODE) && (!(defined(RAM_COMPILATION))))
  /* Before NWP is going to power save mode, set m4ss_ref_clk_mux_ctrl ,tass_ref_clk_mux_ctrl ,AON domain power supply controls from NWP to M4 */
  RSI_Set_Cntrls_To_M4();
#endif

// READ_MBR_MAGIC_WORD_ON_WAKEUP
#ifdef SLI_SI91X_MCU_COMMON_FLASH_MODE
  if (!in_ps2_state) {
    //!Poll for flash magic word
    while (MBR_MAGIC_WORD != 0x5A5A)
      ;
#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1)
    // Update TX and RX descriptors after M4 wakeup
    rsi_update_tx_dma_desc(1);
    rsi_update_rx_dma_desc();
#endif
  }
#endif

#ifndef SL_SI91X_NPSS_GPIO_BTN_HANDLER
  // Clearing NPSS GPIO rise edge interrupts to avoid false triggering after wakeup
  NPSS_GPIO_CONFIG_CLR_REG = NPSS_GPIO_CLR_VALUE;

  // Restoring the NPSS GPIO interrupt configurations after wakeup
  NPSS_GPIO_CONFIG_REG = npss_gpio_config;
#endif
  /* After wake-up, Set the SCDC voltage to the actual value*/
  /* As this function is located in flash accessing this fucntion only after getting controls*/
  set_scdc(SL_SCDC_ACTIVE);

  /*Update the REG Access SPI division factor to increase the SPI read/write speed*/

  RSI_SetRegSpiDivision(1U);

  /*IPMU dummy read to make IPMU block out of RESET*/
  ipmuDummyRead = ULP_SPI_MEM_MAP(0x144);
  (void)ipmuDummyRead;

  // After Wakeup
  if (!(in_ps2_state && (MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_b.ULPSS_BASED_WAKEUP_b))) {
#if (XTAL_CAP_MODE == POWER_TARN_ALWAYS_USE)
    // disable the XTAL CAP mode
    //SCDC0
    RSI_IPMU_ProgramConfigData(scdc_volt_sel1);
    RSI_IPMU_ProgramConfigData(scdc_volt_trim_efuse);
    //SCDC0_1
    RSI_IPMU_ProgramConfigData(scdc_volt_sel2);
#elif (XTAL_CAP_MODE == POWER_TARN_CONDITIONAL_USE)
    if (lf_clk_mode & BIT(4)) {
      // disable the XTAL CAP mode
      RSI_PS_NpssPeriPowerUp(SLPSS_PWRGATE_ULP_MCUTS);
      /*configure the slope,nominal temperature and f2_nominal*/
      RSI_TS_Config(MCU_TEMP, 25);
      /*disable the bjt based temp sensor*/
      RSI_TS_RoBjtEnable(MCU_TEMP, 0);
      /*Enable the RO based temp sensor*/
      RSI_TS_Enable(MCU_TEMP, 1);
      /*update the temperature periodically*/
      RSI_Periodic_TempUpdate(TIME_PERIOD, 1, 0);
      /*read the temperature*/
      Temp = (int)RSI_TS_ReadTemp(MCU_TEMP);
      if (Temp > 45) {
        //SCDC0
        RSI_IPMU_ProgramConfigData(scdc_volt_sel1);
        RSI_IPMU_ProgramConfigData(scdc_volt_trim_efuse);
        //SCDC0_1
        RSI_IPMU_ProgramConfigData(scdc_volt_sel2);
      }
    }
#endif
  }
  /*Spare register write sequence*/
  ipmuDummyRead          = ULP_SPI_MEM_MAP(0x1C1);
  ULP_SPI_MEM_MAP(0x141) = ipmuDummyRead;

  ipmuDummyRead          = ULP_SPI_MEM_MAP(0x1C0);
  ULP_SPI_MEM_MAP(0x140) = ipmuDummyRead;
  RSI_PS_LatchCntrlSet(LATCH_TOP_SPI);

  if (in_ps2_state) {
    /*Come out  of LP  mode */
    /* enabling the RETN_LDO HP MODE */
    RSI_IPMU_RetnLdoHpmode();
  }
  /*I2S-PLL Bypass*/
  *(volatile uint32_t *)(0x24041400 + 0x3C) |= BIT(0);

  if (enable_sdcss_based_wakeup) {
    RSI_PS_ClrWkpSources(SDCSS_BASED_WAKEUP);
  }
  if (enable_m4ulp_retention) {
    RSI_PS_M4ssRamBanksPowerUp(m4ulp_ram_core_status);
    RSI_PS_M4ssRamBanksPeriPowerUp(m4ulp_ram_peri_status);
  }
  if (disable_pads_ctrl) {
    ULP_SPI_MEM_MAP(0x141) |= (BIT(11)); // ULP PADS PDO ON
  }

  /* powerup FPU domain*/
  RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_M4_DEBUG_FPU);

  /*Initialize floating point unit  */
  fpuInit();

#if defined(SLI_SI91X_MCU_ENABLE_RAM_BASED_EXECUTION)
  //passing the ram vector address to VTOR register
  SCB->VTOR = (uint32_t)ram_vector;
#endif

  /* Restore NPSS INTERRUPTS*/
  NPSS_INTR_MASK_CLR_REG = ~npssIntrState;

  /* Restore P2P register values from backup */
  TASS_P2P_INTR_MASK_CLR = ~p2p_intr_status_bkp.tass_p2p_intr_mask_clr_bkp;
  M4SS_P2P_INTR_SET_REG  = p2p_intr_status_bkp.m4ss_p2p_intr_set_reg_bkp;

  /* Restore the Interrupt Priority Register  */
  COPY_CONFIG_REGS(NVIC->IP, nvic_ip_reg, MAX_IPS);

  /* Restore the System Handlers Priority Registers */
  COPY_CONFIG_REGS(SCB->SHP, scs_shp_reg, MAX_SHP);

  /* Restore the NVIC registers */
  COPY_CONFIG_REGS(NVIC->ISER, nvic_enable, MAX_NVIC_REGS);

  return RSI_OK;
}
