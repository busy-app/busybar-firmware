/***************************************************************************/ /**
 * @file ps0_state.c
 * @brief PS0 State example functions
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
#include "ps0_state.h"
#include "rsi_debug.h"
#include "rsi_rom_clks.h"
#include "rsi_rom_ulpss_clk.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
void low_power_configuration(void);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
void ps0_state_init(void)
{
  sl_status_t status;
  // change the TASS reference clock to MHz RC.
  RSI_ChangeTassRefClock();
  // Power downs the domains.
  low_power_configuration();
  // Shutdown Wireless NWP.
  ps_wireless_shutdown();
  DEBUGOUT("PS0 Sleep State \n");
  // PS0 state requirement is added, it transits to PS0 sleep state.
  status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS0);
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with the error code.
    DEBUGOUT("Error Code: 0x%lX, Power State Transition Failed \n", status);
  }
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
  RSI_PS_NpssPeriPowerDown(SLPSS_PWRGATE_ULP_MCURTC | SLPSS_PWRGATE_ULP_MCUWDT | SLPSS_PWRGATE_ULP_MCUPS
                           | SLPSS_PWRGATE_ULP_MCUTS | SLPSS_PWRGATE_ULP_MCUSTORE1 | SLPSS_PWRGATE_ULP_MCUSTORE2
                           | SLPSS_PWRGATE_ULP_MCUSTORE3);
  // Disable the supply to some NPSS peripherals
  RSI_PS_PowerSupplyDisable(POWER_ENABLE_TIMESTAMPING | POWER_ENABLE_DEEPSLEEP_TIMER | POWER_ENABLE_RETENTION_DM);
  // Clear ULPSS_RAM_RETENTION_MODE_EN bit
  MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE &= (~BIT(6));
}
