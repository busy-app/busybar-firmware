/***************************************************************************/ /**
 * @file ps3_state.c
 * @brief PS3 state example functions
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "sl_si91x_power_manager.h"
#include "ps3_state.h"
#include "rsi_debug.h"
#include "rsi_rom_clks.h"
#include "rsi_rom_ulpss_clk.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
void low_power_configuration(void);

#define ACTIVE_STATE 0 //Set the bit to enable the active state.

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

void ps3_state_init(void)
{
  // change the TASS reference clock to MHz RC.
  RSI_ChangeTassRefClock();
  // Power downs the domains.
  low_power_configuration();
  // Shutdown Wireless NWP.
  ps_wireless_shutdown();
  DEBUGOUT("Current State: PS%d \n", sl_si91x_power_manager_get_current_state());
  // Change the clock mode to performace mode(In ps3 clk frequency is 80MHz).
  sl_si91x_power_manager_set_clock_scaling(SL_SI91X_POWER_MANAGER_PERFORMANCE);

#if ACTIVE_STATE
  DEBUGOUT("PS%d Active State  \n", sl_si91x_power_manager_get_current_state());
  while (1) {
    // Idle loop to measure active current consumption
  }
#endif
  DEBUGOUT("PS%d Sleep State\n", sl_si91x_power_manager_get_current_state());

  // Call the sleep function, it goes to PS3 sleep as current state is PS3.
  sl_si91x_power_manager_sleep();
}

/******************************************************************************
 * Disables the Power domains.
 *****************************************************************************/
void low_power_configuration(void)
{
  // Disable OTHER_CLK which is enabled at Start-up
  RSI_CLK_PeripheralClkDisable3(M4CLK, M4_SOC_CLK_FOR_OTHER_ENABLE);
  // Disable Timer clock which is enabled in Bootloader
  RSI_ULPSS_TimerClkDisable(ULPCLK);
  // Disabling LF_RC Clocks
  RSI_ULPSS_DisableRefClks(MCU_ULP_32KHZ_RC_CLK_EN);
  // Power-Down Button Calibration
  RSI_PS_BodPwrGateButtonCalibDisable();
  // Disable PTAT for Analog Peripherals
  RSI_PS_AnalogPeriPtatDisable();
  // Disable PTAT for Brown-Out Detection Clocks
  RSI_PS_BodClksPtatDisable();
  // Power-Down Analog Peripherals
  RSI_IPMU_PowerGateClr(AUXDAC_PG_ENB | AUXADC_PG_ENB | WURX_CORR_PG_ENB | WURX_PG_ENB | ULP_ANG_CLKS_PG_ENB
                        | CMP_NPSS_PG_ENB);
  // Power-Down Domains in NPSS
  RSI_PS_NpssPeriPowerDown(SLPSS_PWRGATE_ULP_MCUWDT | SLPSS_PWRGATE_ULP_MCUPS | SLPSS_PWRGATE_ULP_MCUTS
                           | SLPSS_PWRGATE_ULP_MCUSTORE2 | SLPSS_PWRGATE_ULP_MCUSTORE3);
  // Power gate the M4SS peripherals
  RSI_PS_M4ssPeriPowerDown(M4SS_PWRGATE_ULP_IID | M4SS_PWRGATE_ULP_SDIO_SPI | M4SS_PWRGATE_ULP_RPDMA
                           | M4SS_PWRGATE_ULP_EFUSE_PERI | M4SS_PWRGATE_ULP_QSPI_ICACHE);

  // Power gate the ULPSS peripherals
  RSI_PS_UlpssPeriPowerDown(
#ifndef DEBUG_UART
    ULPSS_PWRGATE_ULP_UART |
#endif
    ULPSS_PWRGATE_ULP_SSI | ULPSS_PWRGATE_ULP_I2S | ULPSS_PWRGATE_ULP_I2C | ULPSS_PWRGATE_ULP_AUX | ULPSS_PWRGATE_ULP_IR
    | ULPSS_PWRGATE_ULP_UDMA | ULPSS_PWRGATE_ULP_FIM);

  // Disable the supply to some NPSS peripherals
  RSI_PS_PowerSupplyDisable(POWER_ENABLE_TIMESTAMPING);
  // Power-Down High-Frequency PLL Domain
  RSI_PS_SocPllSpiDisable();
  // Power-Down QSPI-DLL Domain
  RSI_PS_QspiDllDomainDisable();
  // Enable first boot up
  RSI_PS_EnableFirstBootUp(1);
  // XTAL wait time is skipped since RC_32MHZ Clock is used for Processor on wakeup
  RSI_PS_SkipXtalWaitTime(1);
}