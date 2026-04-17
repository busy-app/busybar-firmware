/*******************************************************************************
* @file  rsi_power_save.h
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

#ifndef __RSI_POWER_SAVE_H__
#define __RSI_POWER_SAVE_H__

#include "rsi_error.h"
#include "base_types.h"
#include "rsi_ccp_common.h"
#include "rsi_reg_spi.h"
#include "rsi_ipmu.h"
#include "rsi_system_config.h"
#include "rsi_ccp_user_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*M4SS peripheral power gates */
/* M4SS_PWRCTRL_SET_REG Bits ((NPSS MCU(MCU DM POWER CTRL FF's Domain))*/
#define M4SS_PWRGATE_ULP_EXT_ROM  BIT(22)
#define M4SS_PWRGATE_ULP_M4_CORE  BIT(18)
#define M4SS_PWRGATE_ULP_IID      BIT(14)
#define M4SS_PWRGATE_ULP_SDIO_SPI BIT(11)
#define M4SS_PWRGATE_ULP_RPDMA    BIT(9)
#if defined(SLI_SI917)
#define M4SS_PWRGATE_ULP_EFUSE_PERI   BIT(4)
#define M4SS_PWRGATE_ULP_QSPI_ICACHE  BIT(13)
#define M4SS_PWRGATE_ULP_M4_DEBUG_FPU BIT(17)
#else
#define M4SS_PWRGATE_ULP_EFUSE    BIT(4)
#define M4SS_PWRGATE_ULP_QSPI     BIT(13)
#define M4SS_PWRGATE_ULP_M4_DEBUG BIT(17)
#endif
#if defined(CHIP_9118)
#define M4SS_PWRGATE_ULP_M4_FPU   BIT(16)
#define M4SS_PWRGATE_ULP_ICACHE   BIT(15)
#define M4SS_PWRGATE_ULP_ETHERNET BIT(12)
#define M4SS_PWRGATE_ULP_USB      BIT(10)
#define M4SS_PWRGATE_ULP_PERI1    BIT(8)
#define M4SS_PWRGATE_ULP_PERI2    BIT(7)
#define M4SS_PWRGATE_ULP_PERI3    BIT(6)
#define M4SS_PWRGATE_ULP_CCI      BIT(5)
#define M4SS_PWRGATE_ULP_SD_MEM   BIT(2)
#endif

/*ULPSS Peripheral power gate */
/* ULPSS_PWRCTRL_SET_REG bits (NPSS MCU(MCU DM POWER CTRL FF's Domain))*/
#define ULPSS_PWRGATE_ULP_MISC BIT(18)
#define ULPSS_PWRGATE_ULP_CAP  BIT(19) // DEPRECATED
#if defined(CHIP_9118)
#define ULPSS_PWRGATE_ULP_VAD BIT(20)
#endif
#define ULPSS_PWRGATE_ULP_UART BIT(21)
#define ULPSS_PWRGATE_ULP_SSI  BIT(22)
#define ULPSS_PWRGATE_ULP_I2S  BIT(23)
#define ULPSS_PWRGATE_ULP_I2C  BIT(24)
#define ULPSS_PWRGATE_ULP_AUX  BIT(25)
#define ULPSS_PWRGATE_ULP_IR   BIT(26)
#define ULPSS_PWRGATE_ULP_UDMA BIT(27)
#define ULPSS_PWRGATE_ULP_FIM  BIT(28)

/*NPSS Peripheral power gates */
/*MCUAON_NPSS_PWRCTRL_SET_REG Bits (MCU AON Domain)*/
#define SLPSS_PWRGATE_ULP_NWPAPB_MCU_CTRL BIT(16)
#define SLPSS_PWRGATE_ULP_TIMEPERIOD      BIT(10)
#define SLPSS_PWRGATE_ULP_MCUSTORE3       BIT(9)
#define SLPSS_PWRGATE_ULP_MCUSTORE2       BIT(8)
#define SLPSS_PWRGATE_ULP_MCUSTORE1       BIT(7)
#define SLPSS_PWRGATE_ULP_MCUTS           BIT(6)
#define SLPSS_PWRGATE_ULP_MCUPS           BIT(5)
#define SLPSS_PWRGATE_ULP_MCUWDT          BIT(4)
#define SLPSS_PWRGATE_ULP_MCURTC          BIT(3)
#define SLPSS_PWRGATE_ULP_MCUFSM          BIT(2)
#define SLPSS_PWRGATE_ULP_MCUBFFS         BIT(1)

/*Power supply controls*/
/*MCU_FSM_CRTL_PDM_AND_ENABLES_b(0x24) Reg Bits (MCU FSM )*/
#define ENABLE_WDT_IN_SLEEP          BIT(0)
#define ENABLE_WURX_DETECTION        BIT(1)
#define RESET_MCU_BBF_DM_EN          BIT(2)
#define DISABLE_TURNOFF_SRAM_PERI    BIT(3)
#define ENABLE_SRAM_DS_CTRL          BIT(4)
#define POWER_ENABLE_FSM_PERI        BIT(16)
#define POWER_ENABLE_TIMESTAMPING    BIT(17)
#define POWER_ENABLE_DEEPSLEEP_TIMER BIT(18)
#define POWER_ENABLE_RETENTION_DM    BIT(19)

/*Wake up sources configuration*/
/*MCU_FSM_CRTL_PDM_AND_ENABLES_b(0x24) Reg Bits (MCU FSM )*/
#define WDT_INTR_BASED_WAKEUP BIT(29)
#define MSEC_BASED_WAKEUP     BIT(28)
#define SEC_BASED_WAKEUP      BIT(27)
#define ALARM_BASED_WAKEUP    BIT(26)
#define SDCSS_BASED_WAKEUP    BIT(25)
#define ULPSS_BASED_WAKEUP    BIT(24)
#if !defined(SLI_SI917B0)
#define WAKEIF_BASED_WAKEUP BIT(22)
#else
#define SYSRTC_BASED_WAKEUP BIT(22)
#endif
#define COMPR_BASED_WAKEUP    BIT(21)
#define GPIO_BASED_WAKEUP     BIT(20)
#define M4_PROCS_BASED_WAKEUP BIT(19)
#define WIRELESS_BASED_WAKEUP BIT(18)
#define HOST_BASED_WAKEUP     BIT(17)
#define DST_BASED_WAKEUP      BIT(16)
#define WIC_BASED_WAKEUP      BIT(23)

/*RAM retention controls */
/*MCU FSM SLEEP CTRLS AND WAKEUP MODE Reg(0x0) (MCU FSM)*/
#define M4ULP_RAM16K_RETENTION_MODE_EN BIT(7)
#define ULPSS_RAM_RETENTION_MODE_EN    BIT(6)
#define TA_RAM_RETENTION_MODE_EN       BIT(5)
#define M4ULP_RAM_RETENTION_MODE_EN    BIT(4)
#define M4SS_RAM_RETENTION_MODE_EN     BIT(3) /* reserved in 917 */
#define HPSRAM_RET_ULP_MODE_EN         BIT(2)

/*RAM banks power gate controls*/
/* M4_SRAM_PWRCTRL_SET_REG1 reg bits((NPSS MCU(MCU DM POWER CTRL FF's Domain)) */
#if defined(CHIP_9118)
/*These banks typically used in M4SS high power mode */
#define RAM_BANK_0 BIT(0) /*!< RAM BANK 0 SIZE : 16K*/
#define RAM_BANK_1 BIT(1) /*!< RAM BANK 1 SIZE : 32K*/
#define RAM_BANK_2 BIT(2) /*!< RAM BANK 2 SIZE : 16K*/
/*Typically used in M4 ULP mode*/
/*m4ulp_ram16k_retention_mode_en*/
#define RAM_BANK_3 BIT(3) /*!< RAM BANK 3 SIZE : 4K*/
#define RAM_BANK_4 BIT(4) /*!< RAM BANK 4 SIZE : 4K*/
#define RAM_BANK_5 BIT(5) /*!< RAM BANK 4 SIZE : 4K*/
#define RAM_BANK_6 BIT(6) /*!< RAM BANK 4 SIZE : 4K*/
#define RAM_BANK_7 BIT(7) /*!< RAM BANK 4 SIZE : 32K*/
#define RAM_BANK_8 BIT(8) /*!< RAM BANK 4 SIZE : 64K*/
#define RAM_BANK_9 BIT(9) /*!< RAM BANK 4 SIZE : 16K*/
/*typically these banks are used in TASS */
#define RAM_BANK_10 BIT(16) /*!< RAM BANK 4 SIZE : 16K*/
#define RAM_BANK_11 BIT(17) /*!< RAM BANK 4 SIZE : 32K*/
#define RAM_BANK_12 BIT(18) /*!< RAM BANK 4 SIZE : 80K*/
#define RAM_BANK_13 BIT(19) /*!< RAM BANK 4 SIZE : 64K*/
#else
/*These banks typically used in M4SS high power mode */
#define RAM_BANK_0 BIT(0) /*!< 4KB (Bank1 of first 192k chunk)*/
#define RAM_BANK_1 BIT(1) /*!< 4KB (Bank2 of first 192k chunk)*/
#define RAM_BANK_2 BIT(2) /*!< 4KB (Bank3 of first 192k chunk)*/
#define RAM_BANK_3 BIT(3) /*!< 4KB (Bank4 of first 192k chunk)*/
#define RAM_BANK_4 BIT(4) /*!< 16KB (Bank 5 of first 192k chunk)*/
#define RAM_BANK_5 BIT(5) /*!< 32KB (Bank 6-7 of first 192k chunk*/
#define RAM_BANK_6 BIT(6) /*!< 64KB (Bank 8-11 of first 192k chunk)*/
#define RAM_BANK_7 BIT(7) /*!< 64KB (Bank 12-15 of first 192k chunk)*/
#define RAM_BANK_8 BIT(8) /*!< 64KB (Bank 1-4 of second 64k chunk)*/
#define RAM_BANK_9 BIT(9) /*!< 64KB (Bank 1-4 of third 64k chunk)*/
#endif

/*ULPSS RAM banks power gates */
/*ULPSS _RAM_PWRCTRL_SET_REG1 bits (NPSS MCU(MCU DM POWER CTRL FF's Domain)*/
#define ULPSS_2K_BANK_0 BIT(0)
#define ULPSS_2K_BANK_1 BIT(1)
#define ULPSS_2K_BANK_2 BIT(2)
#define ULPSS_2K_BANK_3 BIT(3)
#if defined(CHIP_9118)
#define ULPSS_2K_BANK_4 BIT(4)
#define ULPSS_2K_BANK_5 BIT(5)
#define ULPSS_2K_BANK_6 BIT(6)
#define ULPSS_2K_BANK_7 BIT(7)
#endif

/*Controls the PMU off delay time*/
#define PMU_LDO_OFF_DELAY 31U

/*LP control reg */
#define M4LP_CTRL_REG          *((volatile uint32_t *)(0x24041424))
#define ULP_M4_CORE_CLK_ENABLE BIT(2)

/*NPSS INTERRUPT */
#ifndef NPSS_INTR_BASE
#define NPSS_INTR_BASE 0x12080000
#endif // NPSS_INTR_BASE
#ifndef NPSS_INTR_MASK_SET_REG
#define NPSS_INTR_MASK_SET_REG (*(volatile uint32_t *)(NPSS_INTR_BASE + 0x00))
#endif // NPSS_INTR_MASK_SET_REG
#ifndef NPSS_INTR_MASK_CLR_REG
#define NPSS_INTR_MASK_CLR_REG (*(volatile uint32_t *)(NPSS_INTR_BASE + 0x04))
#endif // NPSS_INTR_MASK_CLR_REG
#ifndef NPSS_INTR_CLEAR_REG
#define NPSS_INTR_CLEAR_REG (*(volatile uint32_t *)(NPSS_INTR_BASE + 0x08))
#endif // NPSS_INTR_CLEAR_REG
#ifndef NPSS_INTR_STATUS_REG
#define NPSS_INTR_STATUS_REG (*(volatile uint32_t *)(NPSS_INTR_BASE + 0x0C))
#endif // NPSS_INTR_STATUS_REG
#define M4_ULP_SLP_STATUS_REG (*(volatile uint32_t *)(NPSS_INTR_BASE + 0x1C))
#define MCU_ULP_WAKEUP        BIT(0) /*	To check whether it is first boot up or Wake up */
#define NWPAON_POR_CTRL_BITS  *(volatile uint32_t *)(0x41300000 + 0x3C)
#define POC_CNTRL_REG_0       BIT(0)

/*NPSS interrupt numbers*/
#define NPSS_TO_MCU_WDT_INTR    BIT(0)
#define NPSS_TO_MCU_GPIO_INTR_0 BIT(1)
#define NPSS_TO_MCU_GPIO_INTR_1 BIT(2)
#define NPSS_TO_MCU_GPIO_INTR_2 BIT(3)
#define NPSS_TO_MCU_GPIO_INTR_3 BIT(4)
#define NPSS_TO_MCU_GPIO_INTR_4 BIT(5)

#define NPSS_TO_MCU_CMP_INTR_1 BIT(6)
#define NPSS_TO_MCU_CMP_INTR_2 BIT(7)
#define NPSS_TO_MCU_CMP_INTR_3 BIT(8)
#define NPSS_TO_MCU_CMP_INTR_4 BIT(9)

#define NPSS_TO_MCU_RFWAKEUP_INTR BIT(10)

#define NPSS_TO_MCU_BOD_INTR    BIT(11)
#define NPSS_TO_MCU_BUTTON_INTR BIT(12)

#define NPSS_TO_MCU_SDC_INTR       BIT(13)
#define NPSS_TO_MCU_WIRELESS_INTR  BIT(14)
#define NPSS_TO_MCU_WAKEUP_INTR    BIT(15)
#define NPSS_TO_MCU_ALARM_INTR     BIT(16)
#define NPSS_TO_MCU_SEC_INTR       BIT(17)
#define NPSS_TO_MCU_MSEC_INTR      BIT(18)
#define NPSS_TO_MCU_PROCESSOR_INTR BIT(19)
#define NPSS_TO_MCU_HOST_INTR      BIT(20)
#define NPSS_TO_MCU_DST_INTR       BIT(21)

#define ENABLE_NEGEDGE_ULP     BIT(1)
#define ULP_MODE_SWITCHED_NPSS BIT(3)
#define ULP_MODE_AFTR_CLK_SW   BIT(2)

/*wake up status register */
/*MCU_FSM_WAKEUP_STATUS_REG Reg(0x38) (MCU FSM)*/
#define CDBG_POWER_UP_REQUEST_WAKEUP     BIT(6)
#define WATCH_DOG_WINDOW_RESET_INTERRUPT BIT(5)
#define WWD_RESET_LOOKUP_WAKEUP          BIT(4)
#define WATCHDOG_RESET_INTERRUPT         BIT(3)
#define HOST_BASED_WAKEUP_S              BIT(2)
#define TIMEOUT_WAKEUP                   BIT(1)
#define WAKEUP_INDICATION                BIT(0)

/*wake up status clear register */
/*MCU_FSM_WAKEUP_STATUS_CLEAR Reg(0x3C) (MCU FSM)*/
#define MCU_WAKEUP_INTERRUPT                BIT(11)
#define RF_WAKEUP_CLEAR                     BIT(10)
#define COMP6_BASED_WAKEUP_STATUS_CLEAR     BIT(9)
#define COMP5_BASED_WAKEUP_STATUS_CLEAR     BIT(8)
#define COMP4_BASED_WAKEUP_STATUS_CLEAR     BIT(7)
#define COMP3_BASED_WAKEUP_STATUS_CLEAR     BIT(6)
#define COMP2_BASED_WAKEUP_STATUS_CLEAR     BIT(5)
#define COMP1_BASED_WAKEUP_STATUS_CLEAR     BIT(4)
#define RTC_ALARM_BASED_WAKEUP_STATUS_CLEAR BIT(3)
#define RTC_SEC_BASED_STATUS_CLEAR          BIT(2)
#define MILLI_SEC_BASED_STATUS_CLEAR        BIT(1)
#define WWD_INTERRUPT_STATUS_CLEAR          BIT(0)

/*PMU status configuration bits */
/* MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE Reg bits (0x0)(MCU FSM)*/
#define PMU_STS_DCDC_ON      BIT(10)
#define PMU_STS_FLASH_LDO_ON BIT(9)
#define PMU_STS_SOC_LDO_ON   BIT(8)

/*PMU  */
#define STANDBY_LDOSOC_R BIT(18)
#define STANDBY_LDORF_R  BIT(17)
#define BGPMU_SLEEP_EN_R BIT(16)

/*Retention sleep configurations*/
#if defined(SLI_SI917B0)

//!PSRAM only initialized upon wakeup and it branches to PSRAM
#define SL_SI91X_MCU_WAKEUP_PSRAM_MODE 1

//!PSRAM and FLASH both will be initialized upon wake up,BIT4 refers to program flash upon wakeup
#define RSI_WAKEUP_FROM_FLASH_MODE (0x1 | (BIT(4)))
#else
#define RSI_WAKEUP_FROM_FLASH_MODE 1
#endif
#define RSI_WAKEUP_WITH_OUT_RETENTION          2
#define RSI_WAKEUP_WITH_RETENTION              3
#define RSI_WAKEUP_WITH_RETENTION_WO_ULPSS_RAM 4
#define RSI_WAKEUP_WO_RETENTION_WO_ULPSS_RAM   5

//!Retention ram content ulp memory start ,end addresses for power save retention sleep cases
#define RETEN_RAM_CONTENT_START_LOCATION            (*(volatile uint32_t *)(0x24061F00))
#define RETEN_RAM_CONTENT_END_LOCATION              (*(volatile uint32_t *)(0x24061FCC))
#define RETEN_RAM_CONTENT_WAKEUP_FLASH_BIT_LOCATION (*(volatile uint32_t *)(0x24061FC8))

#define ICACHE2_ADDR_TRANSLATE_1_REG *(volatile uint32_t *)(0x20280000 + 0x24) // ICACHE address register
#ifndef MISC_CFG_SRAM_REDUNDANCY_CTRL
#define MISC_CFG_SRAM_REDUNDANCY_CTRL *(volatile uint32_t *)(0x46008000 + 0x18) // Misc config register
#endif
#ifndef MISC_CONFIG_MISC_CTRL1
#define MISC_CONFIG_MISC_CTRL1 *(volatile uint32_t *)(0x46008000 + 0x44) // Misc control register
#endif
#define P2P_STATUS_REGISTER        *(volatile uint32_t *)(0x46008000 + 0x174) // P2P status register
#define M4SS_P2P_INTR_SET_REGISTER *(volatile uint32_t *)(0x46008000 + 0x16C) // P2P interrupt set register

typedef enum FSM_CLK { FSM_NO_CLOCK = 0, FSM_20MHZ_RO = 1, FSM_MHZ_RC = 2, FSM_40MHZ_XTAL = 4 } FSM_CLK_T;

typedef enum AON_CLK {
  KHZ_RO_CLK_SEL   = 1, /* RO clock is not supported */
  KHZ_RC_CLK_SEL   = 2,
  KHZ_XTAL_CLK_SEL = 4
} AON_CLK_T;

/*Ship modes*/
typedef enum SHUT_DOWN_WKP_MODE {
  NPSS_GPIO_2_BASED       = 0,
  NPSS_GPIO_1_BASED       = 1,
  NPSS_GPIO_2_AND_3_BASED = 2,
  NPSS_GPIO_2_OR_3_BASED  = 3,
} SHUT_DOWN_WKP_MODE_T;

/// @brief Peri efuse power state
typedef enum { POWER_DOWN, POWER_UP } peri_efuse_power_state_t;

/*m4ss context switch top ULP mode selection */
typedef enum ULP_MODE { ULP_MCU_MODE = 1, UULP_MCU_MODE = 3 } ULP_MODE_T;

/*NPSS Comparator selection*/
typedef enum NPSS_COMPARATOR {
  NPSS_CMP_1,
  NPSS_CMP_2,
  NPSS_CMP_3,
  NPSS_CMP_4,
  NPSS_CMP_5,
} NPSS_COMPARATOR_T;

#define _SOC_LDO        3
#define MCU_SOC_LDO_LVL BIT(17)
#define MCU_DCDC_LVL    BIT(18)

/*Select value for ULPSS RAM Power MUX*/
#define PWR_MUX_SEL_ULPSSRAM_SOC_LDO  3
#define PWR_MUX_SEL_ULPSSRAM_SCDC_0_9 1
#define PWR_MUX_SEL_ULPSSRAM_SCDC_0_6 0

/*Select value for M4 ULP RAM Power MUX*/
#define PWR_MUX_SEL_M4_ULP_RAM_SOC_LDO  3
#define PWR_MUX_SEL_M4_ULP_RAM_SCDC_0_9 1
#define PWR_MUX_SEL_M4_ULP_RAM_SCDC_0_6 0

/*Select value for M4 ULP RAM 16K Power MUX*/
#define PWR_MUX_SEL_M4_ULP_RAM16K_SOC_LDO  3
#define PWR_MUX_SEL_M4_ULP_RAM16K_SCDC_0_9 1
#define PWR_MUX_SEL_M4_ULP_RAM16K_SCDC_0_6 0

/*Select value for M4 ULP (Cortex Core)Power MUX*/
#define PWR_MUX_SEL_M4ULP_SOC_LDO  3
#define PWR_MUX_SEL_M4ULP_SCDC_0_9 1
#define PWR_MUX_SEL_M4ULP_SCDC_0_6 0

/*Select value for ULPSS (Peripherals)Power MUX*/
#define PWR_MUX_SEL_ULPSS_SOC_LDO  1
#define PWR_MUX_SEL_ULPSS_SCDC_0_9 0

/*BG SAMPLE ENABLE AND DISABLE /ENABLE MACROS*/
#define ENABLE_BG_SAMPLE_ENABLE  1
#define DISABLE_BG_SAMPLE_ENABLE 0

/*DC DC ENABLE AND DISABLE /ENABLE MACROS*/
#define ENABLE_DC_DC_ENABLE  1
#define DISABLE_DC_DC_ENABLE 0

/*SOC LDO ENABLE AND DISABLE /ENABLE MACROS*/
#define ENABLE_SOCLDO_ENABLE  1
#define DISABLE_SOCLDO_ENABLE 0

/*STAND BY ENABLE AND DISABLE /ENABLE MACROS*/
#define ENABLE_STANDBYDC  1
#define DISABLE_STANDBYDC 0

/* NWP 192K RAM RETENTION MODE ENABLE in PS2 */
#define ENABLE_TA192K_RAM_RET  1
#define DISABLE_TA192K_RAM_RET 0

/* M4 64K RAM RETENTION MODE ENABLE in PS2  */
#define ENABLE_M464K_RAM_RET  1
#define DISABLE_M464K_RAM_RET 0

#define NWP_FSM_CLOCKS_SELECT      *(volatile uint32_t *)0x41300114
#define ENABLE_RO_32KHz_N_RO_20MHz 0x9

#define NWP_FSM_CLK_EN_AND_FIRST_BOOTUP *(volatile uint32_t *)0x41300120
#define DISABLE_ULP_CLKS                0x790000

#define NWPAON_NPSS_PWRCTRL_CLEAR_REG *(volatile uint32_t *)0x4130000C
#define PWRGATE_EN_N_ULP_NWPDOMAINS   0x78

#define STANDBY_DC1P3     BIT(19)
#define BGPMU_SLEEP_EN    BIT(1)
#define SCDCDC_LP_MODE_EN BIT(0)
#define NWP_PMU_CTRLS     *(volatile uint32_t *)0x41300140

#define TASS_REF_CLOCK_SELECT    *(volatile uint32_t *)0x41300110
#define TASS_REF_CLK_CLEANER_OFF BIT(22)
#define TASS_REF_CLK_CLEANER_ON  BIT(23)

#define TASS_PMU_LDO_CTRL_CLEAR *(volatile uint32_t *)0x41300424
#define TASS_SOC_LDO_LVL        BIT(17)
#define TASS_DCDC_LVL           BIT(18)

#define RF_AFE_PWR_CTRL_REG *(volatile uint32_t *)0x41300480
#define RF2G_SHUTDOWN       BIT(5)

#define RF_AFE_PWR_CTRL_REG *(volatile uint32_t *)0x41300480
#define RF2G_PG_EN          BIT(7)
#define AFE_PG_EN           BIT(1)

#define TASS_M4SS_CRTL_SET_REG     *(volatile uint32_t *)0x41300470
#define TASS_CTRL_SOCPLL_SPI_PG_EN BIT(20)

#define MODEM_BAND1_PWRCTRL_CLEAR_REG               *(volatile uint32_t *)0x41300408
#define MODEM_EXT_PWRGATE_EN_N_ULP_BBP_WLAN_GAINLUT BIT(28)
#define MODEM_EXT_PWRGATE_EN_N_ULP_BBP_TXPPLUT      BIT(24)
#define MODEM_EXT_PWRGATE_EN_N_ULP_PPE_SHARED       BIT(20)
#define MODEM_EXT_PWRGATE_EN_N_ULP_PPE_IMDM         BIT(16)

#define MODEM_ZBBT_PWRCTRL_CLEAR_REG                *(volatile uint32_t *)0x41300418
#define MODEM_EXT_PWRGATE_EN_N_ULP_BBP_ZBBT_GAINLUT BIT(4)

#define NWP_RTC_TIMER_CLOCK_PERIOD_SOC *(volatile uint32_t *)(0x41300204)
#define RTC_TIMER_CLK_PERIOD_VALUE     0x003E7FFF

#define SPI_RTC_TIMER_CLK_PERIOD_APPLIED BIT(31)

#define NWP_FSM_SLEEP_WAKEUP_MODES *(volatile uint32_t *)0x41300100
#define COUNT_TICK_ENABLE          0x1
#if defined(SLI_SI917B0)
#define MCURET_BOOTSTATUS_REG *(volatile uint32_t *)(MCU_NPSS_BASE_ADDR + 0x604)
#define KEY_SIZE_IN_DWORDS    8

#define RSI_WAKE_FROM_FLASH_CB_START_ADDR 0x24061F00

#define STACK_AND_CB_ADDR_PRESENT_IN_BBFF BIT(20)

#define RSI_WAKE_FROM_FLASH_JUMP_ADDR 0x24061F28

#define STACK_AND_CB_ADDR_BIT_NO     12
#define QSPI_KEY_SIZE_256            BIT(16)
#define PSRAM_SEC_EN                 BIT(23)
#define KEY_LENGTH                   BIT(11)
#define PSRAM_SEC_EN                 BIT(23)
#define M4SS_PSRAM_QSPI_BASE_ADDRESS 0x12040000
#endif

#define MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_REG *(volatile uint32 *)(0x24048100 + 0x0)
#define LDO_FLASH_ON                            BIT(9)
#define PMU_DCDC_ON                             BIT(10)

/** @addtogroup SOC2
* @{
*/
/**
 * @fn            STATIC INLINE rsi_error_t ps_power_state_change_ps4tops2( ULP_MODE_T enCtxSel          ,
 *                                                          uint8_t PwrMuxSelUlpssRam    ,
 *                                                          uint8_t pwrMuxSelM4UlpRam    ,
 *                                                          uint8_t pwrMuxSelM4UlpRam16K ,
 *                                                          uint8_t pwrMuxSelM4Ulp       ,
 *                                                          uint8_t pwrMuxSelUlpss       ,
 *                                                          uint8_t bgSampleEnable       ,
 *                                                          uint8_t dcDcEnable           ,
 *                                                          uint8_t socLdoEnable         ,
 *                                                          uint8_t standByDc
 *                                                        )
 * @brief         This API is used to used to change the power transition state from Power save state 4/3 to power save state 2
 * @param[in]     enCtxSel : is Select enum for the context top ULP mode
 *                \n  00, 10 - \ref HP-MCU/LP-MCU Mode
 *                \n  01 - \ref ULP-MCU Mode
 *                \n  11 - \ref UULP-MCU Mode
 *                \n  (1st 16K of M4 RAM is dedicated to IM,
 *                \n  2nd 16K of M4 RAM is dedicated to DM)
 * @param[in]     pwrMuxSelM4UlpRam : Select value for M4 ULP RAM Power MUX
 *                \n  3 \ref  SOC LDO
 *                \n  1 \ref  SCDCDC 0.9
 *                \n  0 \ref  SCDCDC 0.6
 * @param[in]    PwrMuxSelUlpssRam :Select value for ULPSS RAM Power MUX
 *                \n  3 \ref  SOC LDO
 *                \n  1 \ref  SCDCDC 0.9
 *                \n  0 \ref  SCDCDC 0.6
 * @param[in]     pwrMuxSelM4UlpRam16K : is Select value for M4 ULP RAM 16K Power MUX
 *                \n  3 \ref  SOC LDO
 *                \n  1 \ref  SCDCDC 0.9
 *                \n  0  \ref SCDCDC 0.6
 * @param[in]     pwrMuxSelM4Ulp : is Select value for M4 ULP (Peripherals + CORTEX Core )Power MUX
 *                \n  3 \ref SOC LDO
 *                \n  1 \ref SCDCDC 0.9
 *                \n  0 \ref SCDCDC 0.6
 * @param[in]     pwrMuxSelUlpss : is Select value for ULPSS(Peripherals) Power MUX
 *                \n  1 \ref SOC LDO
 *                \n  0 \ref SCDCDC 0.9
 * @param[in]     bgSampleEnable  : Value to enable or disable the bg Sample
 *                                  \n 0 :Disable
 *                                  \n 1 :Enale 
 * @param[in]     dcDcEnable      : Value to enable or disable the dcDcEnable 
 *                                  \n 0 :Disable
 *                                  \n 1 :Enale 
 * @param[in]     socLdoEnable    : Value to enable or disable the socLdoEnable 
 *                                  \n 0 :Disable
 *                                  \n 1 :Enale 
 * @param[in]     standByDc       : Value to enable or disable the standByDc
 *                                  \n 0 :Disable
 *                                  \n 1 :Enale 
 * @return        returns 0 \ref RSI_OK on success,return error code on error
 */
STATIC INLINE rsi_error_t ps_power_state_change_ps4tops2(ULP_MODE_T enCtxSel,
                                                         uint8_t PwrMuxSelUlpssRam,
                                                         uint8_t pwrMuxSelM4UlpRam,
                                                         uint8_t pwrMuxSelM4UlpRam16K,
                                                         uint8_t pwrMuxSelM4Ulp,
                                                         uint8_t pwrMuxSelUlpss,
                                                         uint8_t bgSampleEnable,
                                                         uint8_t dcDcEnable,
                                                         uint8_t socLdoEnable,
                                                         uint8_t standByDc)
{
  volatile int x = 0;

  /*return error if this function is called from PS2*/
  if (M4_ULP_SLP_STATUS_REG & ULP_MODE_SWITCHED_NPSS) {
    return ERROR_PS_INVALID_STATE;
  }

  /*Static clock enable m4 core in ULP mode*/
  M4LP_CTRL_REG |= ULP_M4_CORE_CLK_ENABLE;

  /*flushes the transactions in the Bridge before transition to PS2 state*/
  x = (int)ULPCLK->ULP_TA_CLK_GEN_REG;
  /*Select the MCU mode*/
  MCU_FSM->MCU_FSM_PERI_CONFIG_REG_b.M4SS_CONTEXT_SWITCH_TOP_ULP_MODE = enCtxSel;

  /*Avoid the junk system bus access*/
  for (x = 0; x < 10; x++) {
    __ASM("NOP");
  }
  /*
  Indicates the status of functional switching to ULP Mode operation
  0 - PS4 state
  1 - PS2 state
   */
  do {
    /*wait for status of functional switching to ULP Mode operation*/
    x++;
  } while ((M4_ULP_SLP_STATUS_REG & ULP_MODE_AFTR_CLK_SW) != ULP_MODE_AFTR_CLK_SW);

  /*Select value for ULPSS RAM Power Mux*/
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.POWER_MUX_SEL_ULPSS_RAM = (unsigned int)(PwrMuxSelUlpssRam & 0x3);
  /*Select value for M4 ULP RAM Power Mux*/
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.POWER_MUX_SEL_M4_ULP_RAM = (unsigned int)(pwrMuxSelM4UlpRam & 0x3);
  /*Select value for M4 ULP RAM 16K Power Mux*/
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.POWER_MUX_SEL_M4_ULP_RAM_16K = (unsigned int)(pwrMuxSelM4UlpRam16K & 0x3);
  /*Select value for M4 ULP (Peripherals + Cortex Core )Power Mux*/
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.POWER_MUX_SEL_M4_ULP = (unsigned int)(pwrMuxSelM4Ulp & 0x3);
  /*Select value for ULPSS(Peripherals) Power Mux*/
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.POWER_MUX_SEL_ULPSS = (unsigned int)(pwrMuxSelUlpss & 0x1);

  /*update the SOC LDO*/
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.FSM_PERI_SOC_LDO_EN = (unsigned int)(socLdoEnable & 0x1);

  if (socLdoEnable == 1) {
    MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.FSM_PERI_DCDC_EN = 1;
    MCU_FSM->MCU_FSM_PERI_CONFIG_REG_b.BGPMU_SAMPLING_EN_R   = 0;
  } else {
    MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.FSM_PERI_DCDC_EN = (unsigned int)(socLdoEnable & 0x1);
  }

  if (dcDcEnable == 1) {
    MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.FSM_PERI_DCDC_EN = (unsigned int)(dcDcEnable & 0x1);
    MCU_FSM->MCU_FSM_PERI_CONFIG_REG_b.BGPMU_SAMPLING_EN_R   = 0;
  } else {
    MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.FSM_PERI_DCDC_EN = (unsigned int)(dcDcEnable & 0x1);
    MCU_FSM->MCU_FSM_PERI_CONFIG_REG_b.BGPMU_SAMPLING_EN_R   = (unsigned int)(bgSampleEnable & 0x1);
  }

  if ((socLdoEnable == 0) && (dcDcEnable == 1)) {
    MCU_FSM->MCU_FSM_PMU_STATUS_REG_b.STANDBY_DC1P3_R = (unsigned int)(standByDc & 0x1);
  }
  /*Set the LDO turn off delay*/

  /*PMU off delay */
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.PS2_PMU_LDO_OFF_DELAY = PMU_LDO_OFF_DELAY;

#if !defined(SLI_SI917)
  /*Bridge clock disable*/
  ULPCLK->ULP_TA_CLK_GEN_REG_b.ULP2M4_A2A_BRDG_CLK_EN_b = 0;
#endif

  /*Set ulp mode isolation */
  BATT_FF->M4_ULP_MODE_CONFIG = (BATT_FF->M4_ULP_MODE_CONFIG | 0x3F);

  /* Setting RETN_LDO voltage to 0.75V */
  RSI_IPMU_RetnLdo0p75();

  /* enabling the RETN_LDO HP MODE */
  RSI_IPMU_RetnLdoHpmode();

  /*Enable the ULP mode */
  MCU_FSM->MCU_FSM_PERI_CONFIG_REG_b.ULP_MCU_MODE_EN = 1;

  /*  Wait for Status bit indicating Physical transition from PS2 to PS4*/
  do {
    /*wait for  status of Physical switching to ULP Mode operation*/
    x++;
  } while ((M4_ULP_SLP_STATUS_REG & ULP_MODE_SWITCHED_NPSS) != ULP_MODE_SWITCHED_NPSS);

  return RSI_OK;
}

/**
 * @fn          STATIC INLINE rsi_error_t ps_power_state_change_ps2_to_Ps4(uint32_t PmuBuckTurnOnWaitTime , uint32_t SocLdoTurnOnWaitTime)
 * @brief       This API is used to change the power state from PS2 to PS4
 * @param[in]   PmuBuckTurnOnWaitTime :  PMU buck time
 * @param[in]   SocLdoTurnOnWaitTime : soc ldo turn on time
 * @return       returns 0 \ref RSI_OK on success,return error code on error
 */
STATIC INLINE rsi_error_t ps_power_state_change_ps2_to_Ps4(uint32_t PmuBuckTurnOnWaitTime,
                                                           uint32_t SocLdoTurnOnWaitTime)
{
  uint8_t x;
  /*Return if this is issues in PS4 state */
  if (!(M4_ULP_SLP_STATUS_REG & ULP_MODE_SWITCHED_NPSS)) {
    return ERROR_PS_INVALID_STATE;
  }
#ifdef CHIP_9118
  /*Disables clock to ULP-M4SS AHB-AHB Bridge since the MCU is in PS2 state logically*/
  ULPCLK->ULP_TA_CLK_GEN_REG_b.ULP2M4_A2A_BRDG_CLK_EN_b = 0;
#endif
  /*Select value for ULPSS RAM Power Mux*/
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.POWER_MUX_SEL_ULPSS_RAM = _SOC_LDO;
  /*Select value for M4 ULP RAM Power Mux*/
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.POWER_MUX_SEL_M4_ULP_RAM = _SOC_LDO;
  /*Select value for M4 ULP RAM 16K Power Mux*/
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.POWER_MUX_SEL_M4_ULP_RAM_16K = _SOC_LDO;
  /*Select value for M4 ULP (Peripherals + Cortex Core )Power Mux*/
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.POWER_MUX_SEL_M4_ULP = _SOC_LDO;
  /*Select value for ULPSS(Peripherals) Power Mux*/
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.POWER_MUX_SEL_ULPSS = 0x01;

  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.PG4_BUCK_ON_DELAY   = (unsigned int)(PmuBuckTurnOnWaitTime & 0xF);
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.PS4_SOCLDO_ON_DELAY = (unsigned int)(SocLdoTurnOnWaitTime & 0xF);

  MCU_FSM->MCU_FSM_PMU_STATUS_REG_b.STANDBY_DC1P3_R = 0;

  /*Disable the ULP mode */
  MCU_FSM->MCU_FSM_PERI_CONFIG_REG_b.ULP_MCU_MODE_EN = 0;
  // NOTE :  nopes issue issue is fixed by adding this bit
  /*  Wait for Status bit indicating Physical transition from PS2 to PS4*/
  /*Avoid the junk system bus access*/
  for (x = 0; x < 10; x++) {
    __ASM("NOP");
  }

  while (M4_ULP_SLP_STATUS_REG & ULP_MODE_SWITCHED_NPSS)
    ;

  /*Disable ulp mode isolation */
  BATT_FF->M4_ULP_MODE_CONFIG &= ~0x3F;

#if !defined(SLI_SI917)
  /*Disables clock to ULP-M4SS AHB-AHB Bridge since the MCU is in PS2 state logically*/
  ULPCLK->ULP_TA_CLK_GEN_REG_b.ULP2M4_A2A_BRDG_CLK_EN_b = 1;
#endif

  /*Select the MCU mode*/
  MCU_FSM->MCU_FSM_PERI_CONFIG_REG_b.M4SS_CONTEXT_SWITCH_TOP_ULP_MODE = 0;
  // NOTE :  nopes issue issue is fixed by adding this bit
  /*  Wait for Status bit indicating Functional transition from PS4 to PS2*/
  while (M4_ULP_SLP_STATUS_REG & ULP_MODE_AFTR_CLK_SW)
    ;

  /* Setting RETN_LDO voltage to 0.7V */
  RSI_IPMU_RetnLdoVoltsel();

  /* enabling the RETN_LDO LP MODE  */
  RSI_IPMU_RetnLdoLpmode();
#if !defined(SLI_SI917)
  M4CLK->CLK_ENABLE_SET_REG1_b.M4SS_UM_CLK_STATIC_EN_b = 0x1;
#endif
  for (x = 0; x < 200; x++) {
    __ASM("NOP");
  }
#if !defined(SLI_SI917)
  M4CLK->CLK_ENABLE_CLR_REG1_b.M4SS_UM_CLK_STATIC_EN_b = 0x1;
#endif
  return RSI_OK;
}
/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup RSI_POWER_SAVE 
 *  @{
 *
 */
/**
 * @fn            STATIC INLINE void RSI_PS_PowerStateChangePs4toPs3(void)
 * @brief         This API is used to Change the power state from PS4 to PS3
 *                \n
 *                \ref MCU_PMU_LDO_CTRL_CLEAR
 */
STATIC INLINE void RSI_PS_PowerStateChangePs4toPs3(void)
{
  /*Clear the MCU_SOC_LDO_LVL */
  BATT_FF->MCU_PMU_LDO_CTRL_CLEAR = MCU_SOC_LDO_LVL;
}

/**
 * @fn            STATIC INLINE void RSI_PS_SetDcDcToHigerVoltage(void)
 * @brief         This API is used configure DCDC to give higher output voltage.
 *                \n
 *                \ref MCU_PMU_LDO_CTRL_SET
 */
STATIC INLINE void RSI_PS_SetDcDcToHigerVoltage(void)
{
  BATT_FF->MCU_PMU_LDO_CTRL_SET = MCU_DCDC_LVL;
}

/**
 * @fn            STATIC INLINE void RSI_PS_SetDcDcToLowerVoltage(void)
 * @brief         This API is used configure DCDC to give lower output voltage.
 *                \n
 *                \ref MCU_PMU_LDO_CTRL_CLEAR
 */
STATIC INLINE void RSI_PS_SetDcDcToLowerVoltage(void)
{
  BATT_FF->MCU_PMU_LDO_CTRL_CLEAR = MCU_DCDC_LVL;
}

/**
 * @fn            STATIC INLINE void RSI_PS_PowerStateChangePs3toPs4(void)
 * @brief         This API is used to Change the power state from PS3 to PS4
 *                \n
 *                \ref MCU_PMU_LDO_CTRL_SET
 */
STATIC INLINE void RSI_PS_PowerStateChangePs3toPs4(void)
{
  BATT_FF->MCU_PMU_LDO_CTRL_SET = MCU_SOC_LDO_LVL;
  return;
}

/**
 * @fn            STATIC INLINE void RSI_PS_M4ssPeriPowerDown(uint32_t mask)
 * @brief         This API is used to power gate the M4SS peripherals
 * @param[in]     mask  OR'ed value of the power gates
 *                \n
 *                \ref M4SS_PWRCTRL_CLEAR_REG
 * @return    none
 */
STATIC INLINE void RSI_PS_M4ssPeriPowerDown(uint32_t mask)
{
  BATT_FF->M4SS_PWRCTRL_CLEAR_REG = mask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_M4ssPeriPowerUp(uint32_t mask)
 * @brief         This API is used to un power gate the M4SS peripherals
 * @param[in]     mask  OR'ed value of the power gates
 *                \n
 *                \ref M4SS_PWRCTRL_SET_REG
 * @return    none
 */
STATIC INLINE void RSI_PS_M4ssPeriPowerUp(uint32_t mask)
{
  BATT_FF->M4SS_PWRCTRL_SET_REG = mask;
}

/**
 * @fn            STATIC INLINE void sl_si91x_peri_efuse_power_control(bool power_up)
 * @brief         This API is used to power gate the PERI_EFUSE power domain, This power domain contains the different M4SS peripherals those are
 *                SPI/SSI Master, I2C, USART, Micro-DMA Controller,  UART, SPI/SSI Slave, Generic-SPI Master, Config Timer, Random-Number Generator,
 *                CRC Accelerator, SIO, I2C, I2S Master/Slave, QEI, MCPWM ,EFUSE and MVP
 * @param[in]     power_up   1 - Power Up the EFUSE Peri Power domain
 *                           0 - Power Down the EFUSE Peri power domain
 * @return    none
 */
STATIC INLINE void sl_si91x_peri_efuse_power_state_control(peri_efuse_power_state_t power_up)
{
  if (power_up) {
    RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_EFUSE_PERI);
  } else {
    RSI_PS_M4ssPeriPowerDown(M4SS_PWRGATE_ULP_EFUSE_PERI);
  }
}

/**
 * @fn            STATIC INLINE void RSI_PS_M4ss_Tass_Ctrl_Clear(uint32_t mask)
 * @brief        This API is used to clear the M4SS TASS controls
 * @param[in]     mask  OR'ed value of the power gates
 *                \n
 *                \ref M4SS_TASS_CTRL_CLEAR_REG
 * @return    none
 */
STATIC INLINE void RSI_PS_M4ss_Tass_Ctrl_Clear(uint32_t mask)
{
  BATT_FF->M4SS_TASS_CTRL_CLEAR_REG = mask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_UlpssPeriPowerDown(uint32_t mask)
 * @brief         This API is used to  power gate the ULPSS peripherals
 * @param[in]     mask  OR'ed value of the power gates
 *                \n
 *                \ref  ULPSS_PWRCTRL_CLEAR_REG
 * @return    none
 */
STATIC INLINE void RSI_PS_UlpssPeriPowerDown(uint32_t mask)
{
  BATT_FF->ULPSS_PWRCTRL_CLEAR_REG = mask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_UlpssPeriPowerUp(uint32_t mask)
 * @brief         This API is used to un power gate the ULPSS peripherals
 * @param[in]     mask  OR'ed value of the power domains
 *                \n
 *                \ref ULPSS_PWRCTRL_SET_REG
 * @return    none
 */
STATIC INLINE void RSI_PS_UlpssPeriPowerUp(uint32_t mask)
{
  BATT_FF->ULPSS_PWRCTRL_SET_REG = mask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_NpssPeriPowerUp(uint32_t mask) 
 * @brief         This API is used to un power gate the NPSS peripherals
 * @param[in]     mask  OR'ed value of the power domains
 *                \n
 *                \ref MCUAON_NPSS_PWRCTRL_SET_REG
 * @return    none
 */
STATIC INLINE void RSI_PS_NpssPeriPowerUp(uint32_t mask)
{
  MCU_AON->MCUAON_NPSS_PWRCTRL_SET_REG = mask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_NpssPeriPowerDown(uint32_t mask) 
 * @brief          This API is used to power gate the NPSS peripherals
 * @param[in]     mask  OR'ed value of the power domains
 *                \n
 *                \ref MCUAON_NPSS_PWRCTRL_CLEAR_REG
 * @return    none
 */
STATIC INLINE void RSI_PS_NpssPeriPowerDown(uint32_t mask)
{
  MCU_AON->MCUAON_NPSS_PWRCTRL_CLEAR_REG = mask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_M4ssRamBanksPowerDown(uint32_t mask)
 * @brief        This API is used to power gate the M4SS RAM Banks
 * @param[in]     mask  OR'ed value of the RAM power gates
 *                \n
 *                \ref M4_SRAM_PWRCTRL_CLEAR_REG1 
 * @return    none
 */
STATIC INLINE void RSI_PS_M4ssRamBanksPowerDown(uint32_t mask)
{
  BATT_FF->M4_SRAM_PWRCTRL_CLEAR_REG1 = mask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_M4ssRamBanksPowerUp(uint32_t mask)
 * @brief        This API is used to un power gate the M4SS RAM Banks
 * @param[in]     mask  OR'ed value of the RAM power gates
 *                \n
 *                \ref M4_SRAM_PWRCTRL_SET_REG1
 * @return        none
 */
STATIC INLINE void RSI_PS_M4ssRamBanksPowerUp(uint32_t mask)
{
  BATT_FF->M4_SRAM_PWRCTRL_SET_REG1 = mask;
}

/**
 * @fn            STATIC INLINE uint32_t RSI_PS_M4ssRamBanksGetPowerSts(void)
 * @brief         This API is used to get the power gate status of M4SS RAM Banks
 *                \ref M4_SRAM_PWRCTRL_SET_REG1
 * @return        Ored bits of M4_SRAM_PWRCTRL_SET_REG1 reg
 */
STATIC INLINE uint32_t RSI_PS_M4ssRamBanksGetPowerSts(void)
{
  return BATT_FF->M4_SRAM_PWRCTRL_SET_REG1;
}

/**
 * @fn            STATIC INLINE void RSI_PS_M4ssRamBanksPeriPowerDown(uint32_t mask)
 * @brief         This API is used to power gate the M4SS RAM Banks periphery domain
 * @param[in]     mask  OR'ed value of the RAM power gates
 *                \n
 *                \ref M4_SRAM_PWRCTRL_CLEAR_REG1
 * @return    none
 */
STATIC INLINE void RSI_PS_M4ssRamBanksPeriPowerDown(uint32_t mask)
{
  BATT_FF->M4_SRAM_PWRCTRL_CLEAR_REG2 = mask;
}

/**
 * @fn           STATIC INLINE void RSI_PS_M4ssRamBanksPeriPowerUp(uint32_t mask)
 * @brief         This API is used to un-power gate the M4SS RAM Banks periphery domain
 * @param[in]     mask  OR'ed value of the RAM power gates
 *                \n
 *                \ref M4_SRAM_PWRCTRL_SET_REG1
 * @return        none
 */
STATIC INLINE void RSI_PS_M4ssRamBanksPeriPowerUp(uint32_t mask)
{
  BATT_FF->M4_SRAM_PWRCTRL_SET_REG2 = mask;
}

/**
 * @fn           STATIC INLINE uint32_t RSI_PS_M4ssRamBanksGetPeriPowerSts(void)
 * @brief        This API is used to get the power gate status of M4SS RAM Banks periphery domain
 *                \ref M4_SRAM_PWRCTRL_SET_REG1
 * @return        Ored bits of M4_SRAM_PWRCTRL_SET_REG2 register.
 */
STATIC INLINE uint32_t RSI_PS_M4ssRamBanksGetPeriPowerSts(void)
{
  return BATT_FF->M4_SRAM_PWRCTRL_SET_REG2;
}
/**
 * @fn            STATIC INLINE void RSI_PS_UlpssRamBanksPeriPowerDown(uint32_t mask)
 * @brief         This API is used to power gate the ULPSS RAM Banks periphery domain
 * @param[in]     mask  OR'ed value of the RAM power gates
 *                \n
 *                \ref M4_SRAM_PWRCTRL_CLEAR_REG1
 * @return    none
 */
STATIC INLINE void RSI_PS_UlpssRamBanksPeriPowerDown(uint32_t mask)
{
  BATT_FF->ULPSS_RAM_PWRCTRL_CLEAR_REG3 = mask;
}

/**
 * @fn           STATIC INLINE void RSI_PS_UlpssRamBanksPeriPowerUp(uint32_t mask)
 * @brief        This API is used to un-power gate the ULPSS RAM Banks periphery domain
 * @param[in]     mask  OR'ed value of the RAM power gates
 *                \n
 *                \ref M4_SRAM_PWRCTRL_SET_REG1
 * @return        none
 */
STATIC INLINE void RSI_PS_UlpssRamBanksPeriPowerUp(uint32_t mask)
{
  BATT_FF->ULPSS_RAM_PWRCTRL_SET_REG3 = mask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_SetRamRetention(uint32_t ramRetention)
 * @brief         This API is used to set the RAM retention enable for the RAM during sleep
 * @param[in]     ramRetention OR'ed value of the RAM retention bits
 *                \n
 *                \ref MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE
 * @return    none
 */
STATIC INLINE void RSI_PS_SetRamRetention(uint32_t ramRetention)
{
  MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE |= ramRetention;
}

/**
 * @fn            STATIC INLINE void RSI_PS_ClrRamRetention(uint32_t ramRetention)
 * @brief         This API is used to clear the RAM retention enable for the RAM during sleep
 * @param[in]     ramRetention  OR'ed value of the RAM retention bits
 *                \n
 *                \ref MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE
 * @return    none
 */
STATIC INLINE void RSI_PS_ClrRamRetention(uint32_t ramRetention)
{
  MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE &= ~ramRetention;
}

/**
 * @fn            STATIC INLINE void RSI_PS_UlpssRamBanksPowerDown(uint32_t mask)
 * @brief         This API is used to  power gate the ULPSS RAM Banks
 * @param[in]     mask  OR'ed value of the RAM power gates
 *                \n
 *                \ref ULPSS_RAM_PWRCTRL_CLEAR_REG1
 * @return    none
 */
STATIC INLINE void RSI_PS_UlpssRamBanksPowerDown(uint32_t mask)
{
  BATT_FF->ULPSS_RAM_PWRCTRL_CLEAR_REG1 = mask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_UlpssRamBanksPowerUp(uint32_t mask)
 * @brief         This API is used to  un power gate the ULPSS RAM Banks
 * @param[in]     mask  OR'ed value of the RAM power gates
 *                \n
 *                \ref ULPSS_RAM_PWRCTRL_SET_REG1
 * @return    none
 */
STATIC INLINE void RSI_PS_UlpssRamBanksPowerUp(uint32_t mask)
{
  BATT_FF->ULPSS_RAM_PWRCTRL_SET_REG1 = mask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_SetWkpSources(uint32_t wakeUpsrcMask)
 * @brief        This API is used to set the wake up source to wake up from deep sleep
 * @param[in]     wakeUpsrcMask OR'ed value of the wake up sources
 *                \n
 *                \ref MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE
 * @return    none
 */
STATIC INLINE void RSI_PS_SetWkpSources(uint32_t wakeUpsrcMask)
{
  MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE |= wakeUpsrcMask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_ClrWkpSources(uint32_t wakeUpsrcMask)
 * @brief         This API is used to clear the wake up source
 * @param[in]     wakeUpsrcMask OR'ed value of the wake up sources
 *                \ref MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE
 * @return    none
 */
STATIC INLINE void RSI_PS_ClrWkpSources(uint32_t wakeUpsrcMask)
{
  MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE &= ~wakeUpsrcMask;
}

/**
 * @fn            STATIC INLINE uint32_t RSI_PS_GetWkpSources(void)
 * @brief   This API is used to get the wake up source
 * @return    register bits of wake up sources
 */
STATIC INLINE uint32_t RSI_PS_GetWkpSources(void)
{
  return MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE;
}

/**
 * @fn            STATIC INLINE void RSI_PS_EnableFirstBootUp(boolean_t enable)
 * @brief   This API is used to SET and CLEAR the First boot up bit
 * @param[in]     enable : 
 *                \ref MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b  0: disable the first boot , 
 *                \ref MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b  1: enable the first boot up
 * @return    none
 */
STATIC INLINE void RSI_PS_EnableFirstBootUp(boolean_t enable)
{
  MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.FIRST_BOOTUP_MCU_N_b = (unsigned int)(enable & 0x01);
}

/**
 * @fn            STATIC INLINE void RSI_PS_PowerSupplyEnable(uint32_t mask)
 * @brief   This API is used to enable the supply to some NPSS peripherals
 * @param[in]     mask 0: disable the first boot ,  1: enable the first boot up
 * @return    none
 */
STATIC INLINE void RSI_PS_PowerSupplyEnable(uint32_t mask)
{
  MCU_FSM->MCU_FSM_CRTL_PDM_AND_ENABLES |= mask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_PowerSupplyDisable(uint32_t mask)
 * @brief   This API is used to disable the supply to some NPSS peripherals
 * @param[in]     mask  0: disable the first boot ,  1: enable the first boot up
 *                \ref MCU_FSM_CRTL_PDM_AND_ENABLES
 * @return    none
 */
STATIC INLINE void RSI_PS_PowerSupplyDisable(uint32_t mask)
{
  MCU_FSM->MCU_FSM_CRTL_PDM_AND_ENABLES &= ~mask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_FsmHfClkSel(FSM_CLK_T fsmHfClk)
 * @brief   This API is used to  configure the FSM high frequency clock
 * @param[in]     fsmHfClk : enum value of the high frequency clock sources
 *                \ref MCU_FSM_CLKS_REG_b
 * @return    none
 */
STATIC INLINE void RSI_PS_FsmHfClkSel(FSM_CLK_T fsmHfClk)
{
  MCU_FSM->MCU_FSM_CLKS_REG_b.HF_FSM_CLK_SELECT = fsmHfClk;
  while (MCU_FSM->MCU_FSM_CLKS_REG_b.HF_FSM_CLK_SWITCHED_SYNC != 1)
    ;
}

/**
 * @fn            STATIC INLINE void RSI_PS_FsmHfFreqConfig(uint32_t freq)
 * @brief   This API is used to  configure the FSM high frequency clock range
 * @param[in]     freq : frequency in MHz pass 2 if it is 2MHz
 * @return    none
 */
STATIC INLINE void RSI_PS_FsmHfFreqConfig(uint32_t freq)
{
  MCU_FSM->MCU_FSM_CLKS_REG_b.HF_FSM_CLK_FREQ = (unsigned int)(freq & 0x3F);
}

/**
 * @fn            STATIC INLINE void RSI_PS_FsmLfClkSel(AON_CLK_T fsmLfClk)
 * @brief   This API is used to configure the FSM low frequency clock
 * @param[in]     fsmLfClk enum value of the low frequency clock sources
 *                \ref MCUAON_KHZ_CLK_SEL_POR_RESET_STATUS_b
 * @return    none
 */
STATIC INLINE void RSI_PS_FsmLfClkSel(AON_CLK_T fsmLfClk)
{
  MCU_AON->MCUAON_KHZ_CLK_SEL_POR_RESET_STATUS_b.AON_KHZ_CLK_SEL = fsmLfClk;
  while (MCU_AON->MCUAON_KHZ_CLK_SEL_POR_RESET_STATUS_b.AON_KHZ_CLK_SEL_CLOCK_SWITCHED != 1)
    ;
}

/**
 * @fn            STATIC INLINE void RSI_PS_PmuGoodTimeDurationConfig(uint8_t pmuDuration) 
 * @brief   This API is used to  configure the PMU good time.
 * @param[in]     pmuDuration  (0 to 31) are possible value is applied in power of 2.
 * @return    none
 */
STATIC INLINE void RSI_PS_PmuGoodTimeDurationConfig(uint8_t pmuDuration)
{
#ifdef CHIP_9118
  MCU_FSM->MCU_FSM_XTAL_AND_PMU_GOOD_COUNT_REG_b.MCUFSM_PMU_POWERGOOD_DURATION_COUNT =
    (unsigned int)(pmuDuration & 0x1F);
#endif
#if defined(SLI_SI917)
  MCU_FSM->MCU_FSM_XTAL_AND_PMU_GOOD_COUNT_REG_b.MCUFSM_PMU_POWERGOOD_DURATION_COUNT =
    (unsigned int)(pmuDuration & 0x7F);
#endif
}

/**
 * @fn            STATIC INLINE void  RSI_PS_XtalGoodTimeDurationConfig(uint8_t xtalDuration)
 * @brief   This API is used to  configure the XTAL good time.
 * @param[in]     xtalDuration  (0 to 31) are possible value is applied in power of 2.
 *                \ref MCU_FSM_XTAL_AND_PMU_GOOD_COUNT_REG_b
 * @return    none
 */
STATIC INLINE void RSI_PS_XtalGoodTimeDurationConfig(uint8_t xtalDuration)
{
#ifdef CHIP_9118
  MCU_FSM->MCU_FSM_XTAL_AND_PMU_GOOD_COUNT_REG_b.MCUFSM_XTAL_GOODTIME_DURATION_COUNT =
    (unsigned int)(xtalDuration & 0x1F);
#endif
#if defined(SLI_SI917)
  MCU_FSM->MCU_FSM_XTAL_AND_PMU_GOOD_COUNT_REG_b.MCUFSM_XTAL_GOODTIME_DURATION_COUNT =
    (unsigned int)(xtalDuration & 0x7F);
#endif
}

/**
 * @fn            STATIC INLINE void RSI_PS_Ps2PmuLdoOffDelayConfig(uint8_t ldoOffDelay)
 * @brief   This API is used to  configure LDO off delay
 * @param[in]     ldoOffDelay  (0 to 31) are possible value is applied in power of 2.
 * @return    none
 */
STATIC INLINE void RSI_PS_Ps2PmuLdoOffDelayConfig(uint8_t ldoOffDelay)
{
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.PS2_PMU_LDO_OFF_DELAY = (unsigned int)(ldoOffDelay & 0x1F);
}

/**
 * @fn            STATIC INLINE void RSI_PS_Ps4PmuLdoOnDelayConfig(uint8_t ldoOnDelay)
 * @brief   This API is used to  configure LDO on delay
 * @param[in]     ldoOnDelay  (0 to 31) are possible value is applied in power of 2.
 *                \ref MCU_FSM_POWER_CTRL_AND_DELAY_b
 * @return    none
 */
STATIC INLINE void RSI_PS_Ps4PmuLdoOnDelayConfig(uint8_t ldoOnDelay)
{
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.PS4_SOCLDO_ON_DELAY = (unsigned int)(ldoOnDelay & 0xF);
}

/**
 * @fn            STATIC INLINE void RSI_PS_Ps4PmuBuckOnDelayConfig(uint8_t pmuBuckOnDelay)
 * @brief   This API is used to  configure buck on delay
 * @param[in]     pmuBuckOnDelay  (0 to 31) are possible value is applied in power of 2.
 *                \ref MCU_FSM_POWER_CTRL_AND_DELAY_b
 * @return    none
 */
STATIC INLINE void RSI_PS_Ps4PmuBuckOnDelayConfig(uint8_t pmuBuckOnDelay)
{
  MCU_FSM->MCU_FSM_POWER_CTRL_AND_DELAY_b.PG4_BUCK_ON_DELAY = (unsigned int)(pmuBuckOnDelay & 0xF);
}

/**
 * @fn            STATIC INLINE uint32_t RSI_PS_GetWkpUpStatus(void)
 * @brief   This API is used to  get the wake up/ NPSS interrupt status
 *                \ref NPSS_INTR_STATUS_REG
 * @return        register bits of NPSS interrupt status register
 * @return      wakeup/NPSS intr status
 */
STATIC INLINE uint32_t RSI_PS_GetWkpUpStatus(void)
{
  return NPSS_INTR_STATUS_REG;
}

/**
 * @fn            STATIC INLINE uint32_t RSI_PS_GetComnIntrSts(void)
 * @brief   This API is used to  get the wake up/ NPSS common interrupt status
 * @return    register bits of NPSS interrupt status register
 *                \ref MCU_FSM_WAKEUP_STATUS_REG
 * @return    wake up/NPSS common inrerrupt status
 */
STATIC INLINE uint32_t RSI_PS_GetComnIntrSts(void)
{
  return MCU_FSM->MCU_FSM_WAKEUP_STATUS_REG;
}

/**
 * @fn            STATIC INLINE void RSI_PS_NpssIntrUnMask(uint32_t mask)
 * @brief   This API is used to  un mask the NPSS interrupts
 * @param[in]     mask  is OR'ed value of the NPSS interrupt bits
 *                \ref NPSS_INTR_MASK_CLR_REG
 * @return    none
 */
STATIC INLINE void RSI_PS_NpssIntrUnMask(uint32_t mask)
{
  NPSS_INTR_MASK_CLR_REG = mask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_NpssIntrMask(uint32_t mask)
 * @brief   This API is used to  mask the NPSS interrupts
 * @param[in]     mask  is OR'ed value of the NPSS interrupt bits
 *                \ref NPSS_INTR_MASK_SET_REG
 * @return    none
 */
STATIC INLINE void RSI_PS_NpssIntrMask(uint32_t mask)
{
  NPSS_INTR_MASK_SET_REG = mask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_EnableLpSleep(boolean_t lpSleep)
 * @brief   This API is used to  enable/disable the lp sleep mode
 * @param[in]     lpSleep  1:enable lp sleep , 0 : disable lp sleep
 *                \ref MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_b
 * @return    none
 */
STATIC INLINE void RSI_PS_EnableLpSleep(boolean_t lpSleep)
{
  MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_b.LP_SLEEP_MODE_b = (unsigned int)(lpSleep & 0x1);
}

/**
 *@fn              STATIC INLINE void RSI_PS_SkipXtalWaitTime(boolean_t val)
 *@brief           This API is used to  skip the XTAL wait time
 *@param[in]       val 1: skip XTAL wait time
 *                 0 Do not skip XTAL wait time
 *                 \ref MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_b
 * @return    none
 */
STATIC INLINE void RSI_PS_SkipXtalWaitTime(boolean_t val)
{
  /*if package_type value is 5(M7DB) then don't skip xtal wait time   */
  if (package_type != 0x5) {
    MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_b.SKIP_XTAL_WAIT_TIME = (unsigned int)(val & 0x1);
  }
}
/**
 *@fn              STATIC INLINE void RSI_PS_UlpToDcDcMode(void)
 *@brief           This API is configures SC-DCDC from LDO to DCDC Mode
 * @return    none
 */
STATIC INLINE void RSI_PS_UlpToDcDcMode(void)
{
  ULP_SPI_MEM_MAP(0x126) = 0x3E002F;
  ULP_SPI_MEM_MAP(0x128) = 0x200020;
}

/**
 *@fn              STATIC INLINE void RSI_PS_LatchCntrlSet(uint32_t val)
 *@brief           This API is used to set the latch configurations
 * @return    none
 */
STATIC INLINE void RSI_PS_LatchCntrlSet(uint32_t val)
{
  ULP_SPI_MEM_MAP(SELECT_BG_CLK) |= val;
}

/**
 *@fn              STATIC INLINE void RSI_PS_LatchCntrlClr(uint32_t val)
 *@brief           This API is used to clear the latch configurations
 * @return    none
 */
STATIC INLINE void RSI_PS_LatchCntrlClr(uint32_t val)
{
  ULP_SPI_MEM_MAP(SELECT_BG_CLK) &= ~val;
}

/**
 *@fn              STATIC INLINE void RSI_PS_BodPwrGateButtonCalibEnable(void)
 *@brief           This API is used to enable the power-gate enable signal for button calib and vbatt status checking block
 * @return    none
 */
STATIC INLINE void RSI_PS_BodPwrGateButtonCalibEnable(void)
{
  ULP_SPI_MEM_MAP(0x1E3) |= (BIT(15));
}

/**
 *@fn              STATIC INLINE void RSI_PS_BodPwrGateButtonCalibDisable(void)
 *@brief           This API is used to disable the power-gate enable signal for button calib and vbatt status checking block
 * @return    none
 */
STATIC INLINE void RSI_PS_BodPwrGateButtonCalibDisable(void)
{
  ULP_SPI_MEM_MAP(0x1E3) &= ~(BIT(15));
}

/**
 *@fn              STATIC INLINE void RSI_PS_XtalEnable(void)
 *@brief           This API is used to enable the XTAL
 *@return     none
 */
STATIC INLINE void RSI_PS_XtalEnable(void)
{
  *(volatile uint32_t *)0x41300480 |= (BIT(10));
}

/**
 *@fn              STATIC INLINE void RSI_PS_XtalDisable(void)
 *@brief           This API is used to disable the XTAL
 *@return     none
 */
STATIC INLINE void RSI_PS_XtalDisable(void)
{
  *(volatile uint32_t *)0x41300480 &= ~(BIT(10));
}

/**
 *@fn              STATIC INLINE void RSI_PS_QspiDllDomainEnable(void)
 *@brief           This API is used to enable the power to the QSPI-DLL module
 *@return     none
 */
STATIC INLINE void RSI_PS_QspiDllDomainEnable(void)
{
  *(volatile uint32_t *)0x24048484 |= (BIT(2) | BIT(6));
}

/**
 *@fn              STATIC INLINE void RSI_PS_QspiDllDomainDisable(void)
 *@brief           This API is used to disable the power to the QSPI-DLL module
 *@return     none
 */
STATIC INLINE void RSI_PS_QspiDllDomainDisable(void)
{
  *(volatile uint32_t *)0x24048484 &= ~(BIT(2) | BIT(6));
}

/**
 *@fn              STATIC INLINE void RSI_PS_LdoSocDefaultModeEnable(void)
 *@brief           This API is used to enable the SOC LDO default mode (Set high for 1.1 V (default mode))
 *@return     none
 */
STATIC INLINE void RSI_PS_LdoSocDefaultModeEnable(void)
{
  PMU_SPI_MEM_MAP(0x1D6) |= (BIT(5));
}

/**
 *@fn              STATIC INLINE void RSI_PS_LdoSocDefaultModeDisable(void)
 *@brief           This API is used to disable the SOC LDO default mode
 *@return     none
 */
STATIC INLINE void RSI_PS_LdoSocDefaultModeDisable(void)
{
  PMU_SPI_MEM_MAP(0x1D6) &= ~(BIT(5));
}

/**
 *@fn              void RSI_PS_BypassLdoRfEnable(void)
 *@brief           This API is used to enable bypass of LDO-RF enable and control bits to control from outside
 *@return     none
 */
STATIC INLINE void RSI_PS_BypassLdoRfEnable(void)
{
  PMU_SPI_MEM_MAP(0x1D8) |= BIT(2);
}

/**
 *@fn              STATIC INLINE void RSI_PS_FlashLdoEnable(void)
 *@brief           This API is used to flash LDO enable.
 *@return     none
 */
STATIC INLINE void RSI_PS_FlashLdoEnable(void)
{
  BATT_FF->MCU_PMU_LDO_CTRL_SET = BIT(0);
}

/**
 *@fn              STATIC INLINE void RSI_PS_FlashLdoDisable(void)
 *@brief           This API is used to disable flash LDO.
 *@return     none
 */
STATIC INLINE void RSI_PS_FlashLdoDisable(void)
{
  BATT_FF->MCU_PMU_LDO_CTRL_CLEAR = BIT(0);
}

/**
 *@fn              void RSI_PS_BypassLdoRfDisable(void)
 *@brief           This API is used to disable bypass of LDO-RF enable and control bits to control from outside(i.e internal logic)
 *@return     none
 */
STATIC INLINE void RSI_PS_BypassLdoRfDisable(void)
{
  PMU_SPI_MEM_MAP(0x1D8) &= ~BIT(2);
}

/**
 *@fn              STATIC INLINE void RSI_PS_SocPllSpiDisable(void)
 *@brief           This API is used to disable the Soc-PLL SPI PG
 *@return     none
 */
STATIC INLINE void RSI_PS_SocPllSpiDisable(void)
{
  BATT_FF->PLLCCI_PWRCTRL_REG_b.SOCPLL_SPI_PG_EN = 0U;
}

/**
 *@fn              STATIC INLINE void RSI_PS_SocPllVddIsoEnable(void)
 *@brief           This API is used to enable the Soc-PLL ISO VDD
 *@return     none
 */
STATIC INLINE void RSI_PS_SocPllVddIsoEnable(void)
{
  BATT_FF->PLLCCI_PWRCTRL_REG_b.SOCPLL_ISO_ENABLE = 1U;
}

/**
 *@fn              STATIC INLINE void RSI_PS_SocPllVddIsoDiable(void)
 *@brief           This API is used to disable the Soc-PLL ISO VDD
 *@return     none
 */
STATIC INLINE void RSI_PS_SocPllVddIsoDiable(void)
{
  BATT_FF->PLLCCI_PWRCTRL_REG_b.SOCPLL_ISO_ENABLE = 0U;
}

/**
 *@fn              STATIC INLINE void RSI_PS_SocPllSpiEnable(void)
 *@brief           This API is used to enable the Soc-PLL SPI PG
 *@return     none
 */
STATIC INLINE void RSI_PS_SocPllSpiEnable(void)
{
  BATT_FF->PLLCCI_PWRCTRL_REG_b.SOCPLL_SPI_PG_EN = 1U;
}

/**
 *@fn              STATIC INLINE void RSI_ConfigBuckBoost(uint8_t cntrl , uint8_t enable)
 *@brief           This API is used to control the buck boost
 *@param[in]       cntrl 0: Software controlled 1: Hardware controlled.
 *@param[in]       enable 0: Disabled if controlled by software(cntrl = 0) 1: Enabled if controlled by software(cntrl = 1)
 *@return     none
 */
STATIC INLINE void RSI_ConfigBuckBoost(uint8_t cntrl, uint8_t enable)
{
  BATT_FF->MCU_FSM_CTRL_BYPASS_b.MCU_BUCK_BOOST_ENABLE_BYPASS      = (unsigned int)((enable & 0x1) & 0x01);
  BATT_FF->MCU_FSM_CTRL_BYPASS_b.MCU_BUCK_BOOST_ENABLE_BYPASS_CTRL = (unsigned int)((cntrl & 0x1) & 0x01);
}

/**
 *@fn              STATIC INLINE void RSI_ConfigPmuShutDown(uint8_t cntrl , uint8_t enable)
 *@brief           This API is used to control the pmu shut down mode
 *@param[in]       cntrl 0: Software controlled 1: Hardware controlled.
 *@param[in]       enable 0: Disabled if controlled by software(cntrl = 0) 1: Enabled if controlled by software(cntrl = 1)
 *@return     none
 */
STATIC INLINE void RSI_ConfigPmuShutDown(uint8_t cntrl, uint8_t enable)
{
  BATT_FF->MCU_FSM_CTRL_BYPASS_b.MCU_PMU_SHUT_DOWN_BYPASS      = (unsigned int)((enable & 0x1) & 0x01);
  BATT_FF->MCU_FSM_CTRL_BYPASS_b.MCU_PMU_SHUT_DOWN_BYPASS_CTRL = (unsigned int)((cntrl & 0x1) & 0x01);
}

/**
 *@fn              STATIC INLINE void RSI_ChangeTassRefClock(void)
 *@brief           This API is used to change the TASS reference clock to MHz RC , This API is used only in MCU mode , should not be used in WiSeMCU mode.
 *@return     none
 */
STATIC INLINE void RSI_ChangeTassRefClock(void)
{
  *(volatile uint32_t *)0x41300110 = (1 << 23) | (1 << 16) | (1 << 4) | 1;
}

/**
 *@fn              STATIC INLINE void RSI_SetRegSpiDivision(uint8_t div)
 *@brief           This API is used to change the reg access SPI clock division factor.
 *@return     none
 */
STATIC INLINE void RSI_SetRegSpiDivision(uint8_t div)
{
  *(volatile uint32_t *)(REG_SPI_BASE_ADDR_ULP + 0x00) &= ~0xF;
  *(volatile uint32_t *)(REG_SPI_BASE_ADDR_ULP + 0x00) |= div;
}

/**
 *@fn              STATIC INLINE void RSI_ConfigXtal(uint8_t cntrl , uint8_t enable)
 *@brief           This API is used to control the Xtal
 *@param[in]       cntrl 0: Software controlled 1: Hardware controlled.
 *@param[in]       enable 0: Disabled if controlled by software(cntrl = 0) 1: Enabled if controlled by software(cntrl = 1)
 *@return     none
 */
STATIC INLINE void RSI_ConfigXtal(uint8_t cntrl, uint8_t enable)
{
  BATT_FF->MCU_FSM_CTRL_BYPASS_b.MCU_XTAL_EN_40MHZ_BYPASS      = (unsigned int)((enable & 0x1) & 0x01);
  BATT_FF->MCU_FSM_CTRL_BYPASS_b.MCU_XTAL_EN_40MHZ_BYPASS_CTRL = (unsigned int)((cntrl & 0x1) & 0x01);
}

/**
 *@fn              STATIC INLINE void RSI_PS_PmuUltraSleepConfig(boolean_t en)
 *@brief           This API is used to enable the 'NPSS_PMU_STANDBY' 
 *@param[in]       en 1: enable the 'NPSS_PMU_STANDBY'
 *                 en 0: disable the 'NPSS_PMU_STANDBY'
 *@return     none
 */
STATIC INLINE void RSI_PS_PmuUltraSleepConfig(boolean_t en)
{
  MCU_FSM->MCU_FSM_PMU_STATUS_REG_b.STANDBY_DC1P3_R = (unsigned int)(en & 0x01);
}

/**
 *@fn              STATIC INLINE void RSI_PS_PmuSetConfig(uint32_t  mask)
 *@brief           This API is used to enable/set the PMU status
 *@param[in]       mask : Ored values of PMU status bits 
 *                 \n Following are the possible parameters for this parameter    
 *                 \n PMU_STS_DCDC_ON
 *                 \n PMU_STS_FLASH_LDO_ON
 *                 \n PMU_STS_SOC_LDO_ON
 *@return     none
 */
STATIC INLINE void RSI_PS_PmuSetConfig(uint32_t mask)
{
  MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE |= mask;
}

/**
 *@fn              STATIC INLINE void RSI_PS_PmuClrConfig(uint32_t  mask)
 *@brief           This API is used to disable/clear the PMU status 
 *@param[in]       mask : Ored values of PMU status bits 
 *                 \n Following are the possible parameters for this parameter    
 *                 \n PMU_STS_DCDC_ON
 *                 \n PMU_STS_FLASH_LDO_ON
 *                 \n PMU_STS_SOC_LDO_ON
 *@return     none
 */
STATIC INLINE void RSI_PS_PmuClrConfig(uint32_t mask)
{
  MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE &= ~mask;
}

/**
 *@fn         void RSI_PS_AnalogPeriPtatEnable(void)
 *@brief      This API is used to enable the ptat currents to analog peripherals
 * @return    execution status
 */
STATIC INLINE uint32_t RSI_PS_AnalogPeriPtatEnable(void)
{
  return RSI_IPMU_ProgramConfigData(ana_perif_ptat_common_config1);
}

/**
 *@fn         void RSI_PS_AnalogPeriPtatDisable(void)
 *@brief      This API is used to disable the ptat currents to analog peripherals
 *@return     execution status
 */
STATIC INLINE uint32_t RSI_PS_AnalogPeriPtatDisable(void)
{
  return RSI_IPMU_ProgramConfigData(ana_perif_ptat_common_config2);
}

/**
 *@fn         void RSI_PS_BodClksPtatEnable(void)
 *@brief      This API is used to enable the ptat currents to clocks and bod(cmp_npss)
 *@return     execution status
 */
STATIC INLINE uint32_t RSI_PS_BodClksPtatEnable(void)
{
  return RSI_IPMU_ProgramConfigData(ipmu_bod_clks_common_config1);
}

/**
 *@fn         void RSI_PS_BodClksPtatDisable(void)
 *@brief      This API is used to disable the ptat currents to clocks and bod(cmp_npss)
 *@return     execution status
 */
STATIC INLINE uint32_t RSI_PS_BodClksPtatDisable(void)
{
  return RSI_IPMU_ProgramConfigData(ipmu_bod_clks_common_config2);
}

/**
 *@fn         void RSI_PS_PS4SetRegisters(void)
 *@brief      This API is used configure the registers for clock more than 120 MHz in PS4
 *@return     none
 */
STATIC INLINE void RSI_PS_PS4SetRegisters(void)
{
  // Configure the prefetch and registering when SOC clock is more than 120 MHz
  ICACHE2_ADDR_TRANSLATE_1_REG = BIT(21); // Icache registering when clock frequency is more than 120 MHz
  // When set, enables registering in M4-NWP AHB2AHB. This will have performance penalty. This has to be set above 100 MHz
  MISC_CFG_SRAM_REDUNDANCY_CTRL = BIT(4);
  MISC_CONFIG_MISC_CTRL1 |= BIT(4); // Enable Register ROM as clock frequency is 200 MHz
}

/**
 *@fn         void RSI_PS_PS4ClearRegisters(void)
 *@brief      This API is used to clear the MISC registers for clock less than 120 MHz for core
 *@return     none
 */
STATIC INLINE void RSI_PS_PS4ClearRegisters(void)
{
  // Clears the prefetch and registering when SOC clock is less than 120 MHz
  ICACHE2_ADDR_TRANSLATE_1_REG &= ~BIT(21); // Clearing Icache registering when clock frequency is less than 120 MHz
  // When set, enables registering in M4-NWP AHB2AHB. This will have performance penalty. This has to be set above 100 MHz
  MISC_CFG_SRAM_REDUNDANCY_CTRL &= ~BIT(4);
  MISC_CONFIG_MISC_CTRL1 &= ~BIT(4); // Disable Register ROM as clock frequency is less than 120 MHz
}

/**
 *@fn         void RSI_PS_PS2UpdateClockVariable(void)
 *@brief      This API is used update the global clock variable after clock setting in PS2
 *@return     none
 */
STATIC INLINE void RSI_PS_PS2UpdateClockVariable(void)
{
  // Updates the system clock.
  system_clocks.rc_mhz_clock = 20000000;
  // Updating the systemcoreclock variable.
  SystemCoreClock = 20000000;
}

/**
 *@fn         void RSI_PS_WakeupTAandProgramFlash(void)
 *@brief      This API is used wakeup the NWP and program the flash
 *@return     none
 */
STATIC INLINE void RSI_PS_WakeupTAandProgramFlash(void)
{
  if (!(P2P_STATUS_REGISTER & BIT(3))) {
    //!wakeup NWP
    P2P_STATUS_REGISTER |= BIT(0);
    //!wait for NWP active
    while (!(P2P_STATUS_REGISTER & BIT(3)))
      ;
  }
  //! Request NWP to program flash
  //! raise an interrupt to NWP register
  M4SS_P2P_INTR_SET_REGISTER = BIT(4);
  P2P_STATUS_REGISTER        = BIT(0);

  while (!(P2P_STATUS_REGISTER & BIT(3)))
    ;
}

/**
 *@fn         void RSI_PS_SetMCUActiveStatus(void)
 *@brief      This API is used set the active status of mcu after wakeup
 *@return     none
 */
STATIC INLINE void RSI_PS_SetMCUActiveStatus(void)
{
  P2P_STATUS_REGISTER = BIT(1);
}

/**
 *@fn         void RSI_PS_IsPS2State(void)
 *@brief      This API is used to check is current state is PS2 or not
 *@return     true if PS2 state false if not
 */
STATIC INLINE boolean_t RSI_PS_IsPS2State(void)
{
  return (M4_ULP_SLP_STATUS_REG & ULP_MODE_SWITCHED_NPSS);
}

/** @} */

/* @} end of RSI_POWER_SAVE */

void ps_clr_wkp_up_status(uint32_t wakeUpIntrClear);

void ps_bg_ldo_config(uint8_t ldo_0p6_ctrl, uint8_t ldo_0p6_lp_mode);

void ps_configure_trim_values(uint16_t lf_ro_trim,
                              uint16_t lf_rc_trim,
                              uint16_t hf_ro_trim,
                              uint16_t hf_rc_trim,
                              uint16_t bg_ptat_trim,
                              uint16_t bg_trim);

void ps_wireless_shutdown(void);

/*End of file not truncated*/
#ifdef __cplusplus
}
#endif

#endif /*__RSI_POWER_SAVE_H__*/
