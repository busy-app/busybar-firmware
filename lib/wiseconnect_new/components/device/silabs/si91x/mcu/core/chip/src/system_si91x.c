/******************************************************************************
* @file  system_si91x.c
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

#include <stdint.h>
#include "system_si91x.h"
#include "rsi_error.h"
#include "rsi_ccp_common.h"
#include "rsi_ps_ram_func.h"
#include "rsi_ipmu.h"
#include "rsi_pll.h"
#include "rsi_power_save.h"
#include "rsi_ulpss_clk.h"
#include "rsi_rom_ulpss_clk.h"
#include "rsi_rom_clks.h"

#if defined(SLI_SI91X_MCU_PSRAM_PRESENT)
#include "rsi_d_cache.h"
#endif

#if defined(SI91X_32kHz_EXTERNAL_OSCILLATOR)
#include "sl_si91x_external_oscillator.h"
#define MCU_RETENTION_BASE_ADDRESS 0x24048600
#define NPSS_GPIO_CTRL             (MCU_RETENTION_BASE_ADDRESS + 0x1C)
#endif

// Default clock selection for LF_FSM
// This will be used if no specific configuration is provided.
#define LF_FSM_CLK_SEL KHZ_XTAL_CLK_SEL

// Conditional configuration if LF_FSM_CLOCK_UC is defined
#ifdef LF_FSM_CLOCK_UC
#include "sl_si91x_lf_fsm_clock_sel.h"
// Check for invalid simultaneous configuration
#if (_32KHZ_XTAL_CLOCK == 1) && (_32KHZ_RC_CLOCK == 1)
#error "Both _32KHZ_XTAL_CLOCK and _32KHZ_RC_CLOCK are enabled! Only one should be active."

// Check if the 32kHz XTAL clock is enabled
#elif (_32KHZ_XTAL_CLOCK == 1)
// Override default clock selection with XTAL clock
#undef LF_FSM_CLK_SEL
#define LF_FSM_CLK_SEL KHZ_XTAL_CLK_SEL

// Check if the 32kHz RC clock is enabled
#elif (_32KHZ_RC_CLOCK == 1)
// Override default clock selection with RC clock
#undef LF_FSM_CLK_SEL
#define LF_FSM_CLK_SEL KHZ_RC_CLK_SEL

// Error handling for invalid or missing configuration
#else
#error "No valid LF_FSM clock source defined under LF_FSM_CLOCK_UC!"
#endif
#endif // LF_FSM_CLOCK_UC

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
/*Cortex-m4 FPU registers*/
#define FPU_CPACR       0xE000ED88
#define SCB_MVFR0       0xE000EF40
#define SCB_MVFR0_RESET 0x10110021
#define SCB_MVFR1       0xE000EF44
#define SCB_MVFR1_RESET 0x11000011

/*Simulation macros*/
#define SIMULATION_SILICON_REV  0x14
#define SIMULATION_PACKAGE_TYPE 0x1

/* Constants required to manipulate the NVIC. */
/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock; /*!< System Clock Frequency (Core Clock)*/

SYSTEM_CLOCK_SOURCE_FREQUENCIES_T system_clocks; /*!< System Clock sources Frequencies */

uint32_t npssIntrState = 0;
uint32_t __sp;
uint32_t SiliconRev;
uint32_t package_type;

/**
 * @fn     void SystemCoreClockUpdate (void)
 * @brief  Updates the SystemCoreClock with current core Clock
 *         retrieved from cpu registers.
 * @return none
 */
void SystemCoreClockUpdate(void) /* Get Core Clock Frequency      */
{
  // Enables interrupts by clearing the PRIMASK register, allowing the processor to handle IRQs
  __asm volatile("cpsie i" ::: "memory");

  const retention_boot_status_word_t *retention_reg = (const retention_boot_status_word_t *)MCURET_BOOTSTATUS;

  /*Updated the default SOC clock frequency*/
  SystemCoreClock = DEFAULT_40MHZ_CLOCK;
#if (defined(RAM_COMPILATION) && defined(SLI_SI91X_MCU_COMMON_FLASH_MODE))
  /*Initialize QSPI for RAM based execution for common flash boards  */
  RSI_FLASH_Initialize();
#endif
#ifndef SIMULATION
#ifdef RAM_COMPILATION
  if (retention_reg->product_mode == MCU) {
    SiliconRev   = SIMULATION_SILICON_REV;
    package_type = SIMULATION_PACKAGE_TYPE;
  } else {
    SiliconRev   = SILICON_REV_WMCU;
    package_type = PACKAGE_TYPE_WMCU;
  }
#else
  if (retention_reg->product_mode == MCU) {
    SiliconRev   = SILICON_REV_MCU;
    package_type = PACKAGE_TYPE_MCU;
  } else {
    SiliconRev   = SILICON_REV_WMCU;
    package_type = PACKAGE_TYPE_WMCU;
  }
#endif
#if defined(SLI_SI91X_MCU_PSRAM_PRESENT)
  rsi_d_cache_invalidate_all();
#endif
  /*Initialize IPMU and MCU FSM blocks */
  RSI_Ipmu_Init();

  /*Configuring the ULP reference clock to 40MHz, as this frequency is required by the temperature sensor for chip supply mode configuration.*/
  system_clocks.rf_ref_clock = DEFAULT_40MHZ_CLOCK;
  RSI_ULPSS_RefClkConfig(ULPSS_40MHZ_CLK);

  /*configures chip supply mode */
  RSI_Configure_Ipmu_Mode();

#endif
  /*Default clock mux configurations */
  RSI_CLK_PeripheralClkEnable3(M4CLK, M4_SOC_CLK_FOR_OTHER_ENABLE);

  /* NWP clock is selected as 40MHZ clock from MCU */
  MCU_FSM->MCU_FSM_REF_CLK_REG_b.TASS_REF_CLK_SEL = ULP_MHZ_RC_CLK;
  /* Changing NPSS GPIO 0 mode to 0, to disable buck-boost enable mode*/
  MCU_RET->NPSS_GPIO_CNTRL[0].NPSS_GPIO_CTRLS_b.NPSS_GPIO_MODE = 0;
  /* Configuring MCU FSM clock for BG_PMU */
  RSI_IPMU_ClockMuxSel(2);

#if defined(SI91X_32kHz_EXTERNAL_OSCILLATOR)

  // Configuring the UULP_GPIOs for external oscillator
  *(volatile uint32 *)(NPSS_GPIO_CTRL + (4 * OSC_UULP_GPIO)) = (BIT(3) | UULP_GPIO_OSC_MODE);
  MCUAON_GEN_CTRLS_REG |= BIT(0);
  MCUAON_GEN_CTRLS_REG;

  // Configuring RC 32KHz Clock for LF-FSM
  RSI_PS_FsmLfClkSel(KHZ_RC_CLK_SEL);
#else
  /* Configuring 32kHz Clock for LF-FSM */
  RSI_PS_FsmLfClkSel(LF_FSM_CLK_SEL);
#endif // SI91X_32kHz_EXTERNAL_OSCILLATOR

  /* Configuring RC-MHz Clock for HF-FSM */
  RSI_PS_FsmHfClkSel(FSM_MHZ_RC);

  /* XTAL control pointed to Software and  XTAL is Turned-Off from M4 */
  RSI_ConfigXtal(XTAL_DISABLE_FROM_M4, XTAL_IS_IN_SW_CTRL_FROM_M4);

#if ((defined SLI_SI91X_MCU_COMMON_FLASH_MODE) && (!(defined(RAM_COMPILATION))))
  /* Before NWP is going to power save mode ,set m4ss_ref_clk_mux_ctrl ,
          tass_ref_clk_mux_ctrl, AON domain power supply controls from NWP to M4 */
  RSI_Set_Cntrls_To_M4();

#endif
  /*Update the system clock sources with source generating frequency*/
  system_clocks.m4ss_ref_clk     = DEFAULT_40MHZ_CLOCK;
  system_clocks.ulpss_ref_clk    = DEFAULT_40MHZ_CLOCK;
  system_clocks.soc_pll_clock    = DEFAULT_SOC_PLL_CLOCK;
  system_clocks.modem_pll_clock  = DEFAULT_MODEM_PLL_CLOCK;
  system_clocks.modem_pll_clock2 = DEFAULT_MODEM_PLL_CLOCK;
  system_clocks.intf_pll_clock   = DEFAULT_INTF_PLL_CLOCK;
  system_clocks.soc_clock        = DEFAULT_40MHZ_CLOCK;
  system_clocks.rc_32khz_clock   = DEFAULT_32KHZ_RC_CLOCK;
  system_clocks.rc_mhz_clock     = DEFAULT_MHZ_RC_CLOCK;
  system_clocks.ro_20mhz_clock   = DEFAULT_20MHZ_RO_CLOCK;
  system_clocks.ro_32khz_clock   = DEFAULT_32KHZ_RO_CLOCK;
  system_clocks.xtal_32khz_clock = DEFAULT_32KHZ_XTAL_CLOCK;
  system_clocks.doubler_clock    = DEFAULT_DOUBLER_CLOCK;
  system_clocks.rf_ref_clock     = DEFAULT_40MHZ_CLOCK;
  system_clocks.mems_ref_clock   = DEFAULT_MEMS_REF_CLOCK;
  system_clocks.byp_rc_ref_clock = DEFAULT_MHZ_RC_CLOCK;
  system_clocks.i2s_pll_clock    = DEFAULT_I2S_PLL_CLOCK;

  return;
}

/**
 * @fn       void fpuInit(void)
 * @brief    This API is used to Early initialization of the FPU 
 * @return   none 
 *
 */
void fpuInit(void)
{
#if __FPU_PRESENT != 0
  // from arm trm manual:
  //                ; CPACR is located at address 0xE000ED88
  //                LDR.W R0, =0xE000ED88
  //                ; Read CPACR
  //                LDR R1, [R0]
  //                ; Set bits 20-23 to enable CP10 and CP11 coprocessors
  //                ORR R1, R1, #(0xF << 20)
  //                ; Write back the modified value to the CPACR
  //                STR R1, [R0]

  volatile uint32_t *regCpacr       = (uint32_t *)FPU_CPACR;
  volatile const uint32_t *regMvfr0 = (volatile const uint32_t *)SCB_MVFR0;
  volatile const uint32_t *regMvfr1 = (volatile const uint32_t *)SCB_MVFR1;
  volatile uint32_t Cpacr;
  volatile uint32_t Mvfr0;
  volatile uint32_t Mvfr1;
  char vfpPresent = 0;

  Mvfr0 = *regMvfr0;
  Mvfr1 = *regMvfr1;

  vfpPresent = ((SCB_MVFR0_RESET == Mvfr0) && (SCB_MVFR1_RESET == Mvfr1));

  if (vfpPresent) {
    Cpacr = *regCpacr;
    Cpacr |= (0xF << 20);
    *regCpacr = Cpacr; // enable CP10 and CP11 for full access
  }
#endif /* __FPU_PRESENT != 0 */
}

/**
 * @fn     void SystemInit (void)       
 * @brief  This API is used Setup the RS1xxxx chip(Initialize the system)
 * @return none
 */
void SystemInit(void)
{
  volatile uint32_t ipmuDummyRead = 0;
  volatile uint32_t spareReg2     = 0;

  /*IPMU dummy read to make IPMU block out of RESET*/
  ipmuDummyRead = ULP_SPI_MEM_MAP(0x144);
  ipmuDummyRead = ipmuDummyRead;

  /*Update the REG Access SPI division factor to increase the SPI read/write speed*/
  RSI_SetRegSpiDivision(0U);

  ULP_SPI_MEM_MAP(BG_SCDC_PROG_REG_1) &= REF_SEL_LP_DCDC;

  /*Spare register write sequence*/
  spareReg2              = ULP_SPI_MEM_MAP(0x1C1);
  ULP_SPI_MEM_MAP(0x141) = spareReg2;
  //while(GSPI_CTRL_REG1 & SPI_ACTIVE);
  /*Spare register write sequence*/
  spareReg2              = ULP_SPI_MEM_MAP(0x1C0);
  ULP_SPI_MEM_MAP(0x140) = spareReg2;

  /*Set IPMU BITS*/
  ULP_SPI_MEM_MAP(SELECT_BG_CLK) |= (LATCH_TOP_SPI | LATCH_TRANSPARENT_HF | LATCH_TRANSPARENT_LF);

  while (GSPI_CTRL_REG1 & SPI_ACTIVE)
    ;

  MCU_AON->MCUAON_GEN_CTRLS_b.ENABLE_PDO      = 1;
  MCU_AON->MCUAON_GEN_CTRLS_b.NPSS_SUPPLY_0P9 = 0;

  /*Enable FPU*/
  fpuInit();

  /* Enable REF Clock Control*/
  // This will be configured by boot-loader based on product mode
  *(volatile uint32_t *)0x41300004 = BIT(24);

  /*Moving to BG sampling mode */
  *(volatile uint32_t *)0x24048140 = (BIT(19) | BIT(1) | BIT(0));

  /*Disable WIC based wake up */
  MCU_FSM->MCU_FSM_PERI_CONFIG_REG_b.WICENREQ = 0;

  /*Set ulp_wakeup_por*/
  MCU_AON->MCUAON_KHZ_CLK_SEL_POR_RESET_STATUS_b.MCU_FIRST_POWERUP_POR     = 1U;
  MCU_AON->MCUAON_KHZ_CLK_SEL_POR_RESET_STATUS_b.MCU_FIRST_POWERUP_RESET_N = 1U;
  /*Programmable delay 4mes for WDT reset */
  PMU_DIRECT_ACCESS(BG_SLEEP_TIMER_REG_OFFSET) |= BIT(19); //bgs_active_timer_sel
  /*Programmable delay 4mes for WDT reset */
  MCU_AON->MCUAON_SHELF_MODE_b.SHELF_MODE_WAKEUP_DELAY = 0x7;
  /* Enables software based control of isolation and reset for ULP AON */
  BATT_FF->M4SS_BYPASS_PWRCTRL_REG1_b.BYPASS_M4SS_PWRCTL_ULP_AON_b = 1;
  /* Enables software based control of isolation and reset for M4ss CORE */
  BATT_FF->M4SS_BYPASS_PWRCTRL_REG1_b.BYPASS_M4SS_PWRCTL_ULP_M4_CORE_b = 1;
  return;
}
/**
 *@}
 */
