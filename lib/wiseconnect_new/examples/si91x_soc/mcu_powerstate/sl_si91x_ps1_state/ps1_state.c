/***************************************************************************/ /**
 * @file ps1_state.c
 * @brief PS1 State example functions
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
#include "ps1_state.h"
#include "rsi_debug.h"
#include "sl_si91x_calendar.h"
#include "sl_ulp_timer_instances.h"
#include "sl_si91x_ulp_timer_common_config.h"
#include "rsi_rom_clks.h"
#include "rsi_rom_ulpss_clk.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
static void clear_ulp_timer_wakeup_source(void);
static void ulp_timer_callback(void);
static void set_ulp_timer_wakeup_source(void);
void low_power_configuration(void);

#define ULP_TIMER_MATCH_VALUE 40000000 // Timer match value for down-counter type with 20mhz clock for 2 seconds
#define ULP_TIMER_INSTANCE \
  SL_ULP_TIMER_TIMER0 // ulp-timer instance to be used, user can pass selected timer-number in place of '0'

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

void ps1_state_init(void)
{
  sl_status_t status;
  // change the TASS reference clock to MHz RC.
  RSI_ChangeTassRefClock();
  // Power downs the domains.
  low_power_configuration();
  // Shutdown Wireless NWP.
  ps_wireless_shutdown();
  // PS2 State requirement is added, it transits to PS2 state.
  status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with the error code.
    DEBUGOUT("Error Code: 0x%lX, Power State Transition Failed \n", status);
  }
  DEBUGOUT("PS%d Active State \n", sl_si91x_power_manager_get_current_state());
  // Configure and initialize the ulp timer wakeup source
  set_ulp_timer_wakeup_source();
  // PS2 state requirement is removed.
  status = sl_si91x_power_manager_remove_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with the error code.
    DEBUGOUT("Error Code: 0x%lX, Power State Transition Failed \n", status);
  }
  DEBUGOUT("Current State: PS1 \n");
  // PS1 State requirement is added, it transits to PS1 state.
  status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS1);
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with the error code.
    DEBUGOUT("Error Code: 0x%lX, Power State Transition Failed \n", status);
  }
  DEBUGOUT("PS%d Active State \n", sl_si91x_power_manager_get_current_state());
  // Clears ulp timer wakeup source
  clear_ulp_timer_wakeup_source();
  DEBUGOUT("PS%d Sleep State\n", sl_si91x_power_manager_get_current_state());
  // Call the sleep function.
  sl_si91x_power_manager_sleep();
}
/*******************************************************************************
 * Configure and initialize the ulp timer wakeup source.
 * ULP timer interrupt is used to wakeup from PS1.
 * ULP timer is initialzed and configured for 2 seconds.
 * Callback is registered for ULP Timer interrupt.
 ******************************************************************************/
static void set_ulp_timer_wakeup_source(void)
{
  sl_status_t status;
  // Set ulp timer timer input clock source as RC MHz
  sl_timer_clk_handle.ulp_timer_clk_input_src = ULP_TIMER_MHZ_RC_CLK_SRC;
  // ULP Timer initialization, the values are fetched from the UC.
  status = sl_si91x_ulp_timer_init(&sl_timer_clk_handle);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_si91x_ulp_timer_init failed, Error Code: 0x%lX \n", status);
    return;
  }
  // Match value is set to 2 seconds.
  sl_timer_handle_timer0.timer_match_value = ULP_TIMER_MATCH_VALUE;
  // ULP timer configurations are set, the values are fetched from the UC.
  status = sl_si91x_ulp_timer_set_configuration(&sl_timer_handle_timer0);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_si91x_ulp_timer_set_configuration failed, Error Code: 0x%lX \n", status);
    return;
  }
  // Callback is registered to enable the timer interrupt.
  status = sl_si91x_ulp_timer_register_timeout_callback(ULP_TIMER_INSTANCE, ulp_timer_callback);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_si91x_ulp_timer_register_timeout_callback failed, Error Code: 0x%lX \n", status);
    return;
  }
  // ULP based wakeup source is selected.
  status = sl_si91x_power_manager_set_wakeup_sources(SL_SI91X_POWER_MANAGER_ULPSS_WAKEUP, true);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_si91x_power_manager_set_wakeup_sources failed, Error Code: 0x%lX \n", status);
    return;
  }

  // ULP Timer is started for timer 0.
  status = sl_si91x_ulp_timer_start(ULP_TIMER_INSTANCE);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_si91x_ulp_timer_start failed, Error Code: 0x%lX \n", status);
    return;
  }
}

/*******************************************************************************
 * Clears ulp timer wakeup source.
 * ULP timer is stopped here.
 * Callback is unregistered for ULP Timer interrupt.
 ******************************************************************************/
static void clear_ulp_timer_wakeup_source(void)
{
  sl_status_t status;
  // After waking up, ulp timer is stopped.
  status = sl_si91x_ulp_timer_stop(ULP_TIMER_INSTANCE);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_si91x_ulp_timer_stop failed, Error Code: 0x%lX \n", status);
    return;
  }
  // After waking up, ulp timer callback is unregistered.
  status = sl_si91x_ulp_timer_unregister_timeout_callback(ULP_TIMER_INSTANCE);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_si91x_ulp_timer_unregister_timeout_callback failed, Error Code: 0x%lX \n", status);
    return;
  }
  // Once the sleep-wakeup is completed, ulp based wakeup source is removed.
  status = sl_si91x_power_manager_set_wakeup_sources(SL_SI91X_POWER_MANAGER_ULPSS_WAKEUP, false);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_si91x_power_manager_set_wakeup_sources failed, Error Code: 0x%lX \n", status);
    return;
  }
}

/*******************************************************************************
 * ULP Timer callback.
 * Interrupt Clearing is handled in driver layer, so this function is defined
 * to avoid undefined function while registering callback.
 ******************************************************************************/
static void ulp_timer_callback(void)
{
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
}
