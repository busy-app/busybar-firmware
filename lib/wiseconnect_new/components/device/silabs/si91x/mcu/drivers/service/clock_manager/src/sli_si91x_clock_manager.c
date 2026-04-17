/******************************************************************************
* @file sli_si91x_clock_manager.c
* @brief Clock Manager Internal API implementation
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

#include "sli_si91x_clock_manager.h"
#include "sl_si91x_power_manager.h"
#include "rsi_rom_clks.h"
#include "rsi_rom_ulpss_clk.h"
#include "rsi_debug.h"
#ifdef SLI_SI91X_MCU_ENABLE_PSRAM_FEATURE
#include "rsi_d_cache.h"

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
/************************************************************************************
 *************************  DEFINES / MACROS  ***************************************
 ************************************************************************************/
#define PS4_PERFORMANCE_MODE_SOC_FREQ  (180000000UL) // PS4 high power soc pll clock frequency
#define PS4_PERFORMANCE_MODE_INTF_FREQ (160000000UL) // PS4 high power intf pll clock frequency
#define PS4_POWERSAVE_MODE_FREQ        (100000000UL) // PS4 low power clock frequency
#define PS3_PERFORMANCE_MODE_FREQ      (80000000UL)  // PS3 high power clock frequency
#define PS3_POWERSAVE_MODE_FREQ        (40000000UL)  // PS3 low power clock frequency

#define QSPI_ODD_DIV_EN 0 // Odd division enable for QSPI clock
#define QSPI_SWALLO_EN  0 // Swallo enable for QSPI clock

/* The division factor for the QSPI clock is as follows:
If both swallow and odd_div_en are set to zero,
then for a division factor of 1, the output clock (Clock_out) will be equal to half of the input clock (Clock_in), i.e., Clock_out = Clock_in / 2.
For a division factor of 2, the output clock will be Clock_in divided by 4, i.e.,Clock_out = Clock_in / (2 * 2) */
#define QSPI_DIV_FACTOR 1 // Division factor for QSPI clock
/************************************************************************************
 *************************  LOCAL VARIABLES  ****************************************
 ************************************************************************************/

/************************************************************************************
 *************************  GLOBAL VARIABLES  ****************************************
 ************************************************************************************/

/************************************************************************************
 *************************  LOCAL TYPE DEFINITIONS  *********************************
 ************************************************************************************/

/************************************************************************************
 *************************  LOCAL FUNCTION PROTOTYPES  ******************************
 ************************************************************************************/
STATIC INLINE sl_status_t config_sleep_clks(void);
/************************************************************************************
 *************************  GLOBAL FUNCTION DEFINITIONS  ****************************
 ************************************************************************************/
/***************************************************************************/
/**
 * @brief Configures the clocks based on the requested power state and power mode.
 * 
 * @param[in] power_state Enum values of different power states. See \ref sl_power_state_t for more info.
 * @param[in] power_mode  Clock scaling mode in PS4 and PS3 power states.
 *                        Possible values:
 *                        - SL_SI91X_POWER_MANAGER_POWERSAVE   (Minimum supported frequency in a power state)
 *                        - SL_SI91X_POWER_MANAGER_PERFORMANCE (Maximum supported frequency in a power state)
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - Corresponding error code on failure.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 **************************************************************************************************/
sl_status_t sli_si91x_clock_manager_config_clks_on_ps_change(sl_power_state_t power_state, boolean_t power_mode)
{
  sl_status_t sli_status = SL_STATUS_OK;
  uint32_t soc_pll_freq;
  QSPI_CLK_SRC_SEL_T qspi_clk_source = QSPI_ULPREFCLK;
  uint8_t qspi_div_fac               = QSPI_DIV_FACTOR;
#ifdef SL_SI91X_REQUIRES_INTF_PLL
  uint32_t intf_pll_freq;
#endif
  switch (power_state) {
    case SL_SI91X_POWER_MANAGER_PS4:
      /* Configure Ref clocks to 40MHz crystal */
      RSI_CLK_M4ssRefClkConfig(M4CLK, EXT_40MHZ_CLK);
      RSI_ULPSS_RefClkConfig(ULPSS_40MHZ_CLK);
      qspi_div_fac = 1;
      // Set SOC PLL and configure M4 source to SOC PLL based on current state and mode
      soc_pll_freq = power_mode ? PS4_PERFORMANCE_MODE_SOC_FREQ : PS4_POWERSAVE_MODE_FREQ;
      sli_status   = sl_si91x_clock_manager_m4_set_core_clk(M4_SOCPLLCLK, soc_pll_freq);
      if (sli_status != SL_STATUS_OK) {
        break;
      }

#ifdef SL_SI91X_REQUIRES_INTF_PLL
      // Set INTF PLL based on current state and mode
      intf_pll_freq = power_mode ? PS4_PERFORMANCE_MODE_INTF_FREQ : PS4_POWERSAVE_MODE_FREQ;

      sli_status = sl_si91x_clock_manager_set_pll_freq(INTF_PLL, intf_pll_freq, PLL_REF_CLK_VAL_XTAL);
      if (sli_status != SL_STATUS_OK) {
        break;
      }
      if (intf_pll_freq == PS4_PERFORMANCE_MODE_INTF_FREQ) {
        qspi_clk_source = QSPI_INTFPLLCLK;
      } else {
        qspi_clk_source = QSPI_ULPREFCLK;
      }
#endif /* SL_SI91X_REQUIRES_INTF_PLL */

      // Configure QSPI clock with INTF PLL as input source
#if defined(CLOCK_ROMDRIVER_PRESENT)
      ROMAPI_M4SS_CLK_API->clk_qspi_clk_config(M4CLK, qspi_clk_source, QSPI_SWALLO_EN, QSPI_ODD_DIV_EN, qspi_div_fac);
#endif

      // Configure QSPI2 clock with INTF PLL as input source
#ifdef SLI_SI91X_MCU_ENABLE_PSRAM_FEATURE
      rsi_d_cache_invalidate_all();
/* Configuring clock for PSRAM operation based on selected configs */
#if defined(CLOCK_ROMDRIVER_PRESENT)
      ROMAPI_M4SS_CLK_API->clk_qspi_2_clk_config(M4CLK, qspi_clk_source, QSPI_SWALLO_EN, QSPI_ODD_DIV_EN, qspi_div_fac);
#endif
#endif
      break;

    case SL_SI91X_POWER_MANAGER_PS3:
      /* Configure Ref clocks to 40MHz crystal */
      RSI_CLK_M4ssRefClkConfig(M4CLK, EXT_40MHZ_CLK);
      RSI_ULPSS_RefClkConfig(ULPSS_40MHZ_CLK);
      qspi_div_fac = 2;

      // configure M4 source frequency based on current state and mode
      soc_pll_freq = power_mode ? PS3_PERFORMANCE_MODE_FREQ : PS3_POWERSAVE_MODE_FREQ;
      if (power_mode) {
        // configure M4 source to SOC PLL
        sli_status = sl_si91x_clock_manager_m4_set_core_clk(M4_SOCPLLCLK, soc_pll_freq);
      } else {
        // Configure M4 source to ULP REF clock
        sli_status = sl_si91x_clock_manager_m4_set_core_clk(M4_ULPREFCLK, 0);
        sl_si91x_clock_manager_set_pll_freq(SOC_PLL, soc_pll_freq, PLL_REF_CLK_VAL_XTAL);
      }
      if (sli_status != SL_STATUS_OK) {
        break;
      }

#ifdef SL_SI91X_REQUIRES_INTF_PLL
      // Set INTF PLL based on current state and mode
      intf_pll_freq = power_mode ? PS3_PERFORMANCE_MODE_FREQ : PS3_POWERSAVE_MODE_FREQ;
      sli_status    = sl_si91x_clock_manager_set_pll_freq(INTF_PLL, intf_pll_freq, PLL_REF_CLK_VAL_XTAL);
      if (sli_status != SL_STATUS_OK) {
        break;
      }
      if (intf_pll_freq == PS3_PERFORMANCE_MODE_FREQ) {
        qspi_clk_source = QSPI_INTFPLLCLK;
        qspi_div_fac    = 1;
      } else {
        qspi_clk_source = QSPI_ULPREFCLK;
        qspi_div_fac    = 2;
      }
#endif /* SL_SI91X_REQUIRES_INTF_PLL */

      // Configure QSPI clock with INTF PLL as input source
#if defined(CLOCK_ROMDRIVER_PRESENT)
      ROMAPI_M4SS_CLK_API->clk_qspi_clk_config(M4CLK, qspi_clk_source, QSPI_SWALLO_EN, QSPI_ODD_DIV_EN, qspi_div_fac);
#endif

      // Configure QSPI2 clock with INTF PLL as input source
#ifdef SLI_SI91X_MCU_ENABLE_PSRAM_FEATURE
      rsi_d_cache_invalidate_all();
/* Configuring clock for PSRAM operation based on selected configs */
#if defined(CLOCK_ROMDRIVER_PRESENT)
      ROMAPI_M4SS_CLK_API->clk_qspi_2_clk_config(M4CLK, qspi_clk_source, QSPI_SWALLO_EN, QSPI_ODD_DIV_EN, qspi_div_fac);
#endif
#endif
      break;

    case SL_SI91X_POWER_MANAGER_PS2:
      // Power modes are not applicable for PS2 state
      UNUSED_PARAMETER(power_mode);

      // Configures the clock with 20MHz
      RSI_IPMU_M20rcOsc_TrimEfuse();
      // Sets FSM HF frequency to 20MHz
      RSI_PS_FsmHfFreqConfig(20);
      // Updated the clock global variables
      RSI_PS_PS2UpdateClockVariable();

      // The remaining clock configurations are common for PS2 and Sleep states
      sli_status = config_sleep_clks();
      break;

    case SL_SI91X_POWER_MANAGER_SLEEP:
      // Power modes are not applicable for Sleep state
      UNUSED_PARAMETER(power_mode);

      // Configure clocks as per sleep state
      sli_status = config_sleep_clks();
      break;

    case SL_SI91X_POWER_MANAGER_PS1:
    case SL_SI91X_POWER_MANAGER_PS0:
    case SL_SI91X_POWER_MANAGER_STANDBY:
      // Not needed for these states
      sli_status = SL_STATUS_INVALID_STATE;
      break;

    default:
      // If reaches here, return error code
      sli_status = SL_STATUS_INVALID_PARAMETER;
      break;
  }

  // Reinit debug uart after clock configuration
  DEBUGINIT();

  return sli_status;
}
/*******************************************************************************
 * Configure clocks as per sleep state
 * Switch Subsystems' Ref clocks to MHz RC
 * Set M4 SOC and QSPI2 clock to Ref clock
 ******************************************************************************/
STATIC INLINE sl_status_t config_sleep_clks(void)
{
  sl_status_t sli_status = SL_STATUS_OK;

  // Change ref clocks to RC clock before moving to PS2/Sleep and not requested from PS2
  if (sl_si91x_power_manager_get_current_state() != SL_SI91X_POWER_MANAGER_PS2) {
    // Change Subsystems' ref clocks from 40MHz XTAL to MHz RC
    RSI_CLK_M4ssRefClkConfig(M4CLK, ULP_MHZ_RC_CLK);
    RSI_ULPSS_RefClkConfig(ULPSS_ULP_MHZ_RC_CLK);

    // Configure M4 source to ULP REF clock
    sli_status = sl_si91x_clock_manager_m4_set_core_clk(M4_ULPREFCLK, 0);
    if (sli_status != SL_STATUS_OK) {
      return sli_status;
    }

    // Configure QSPI clock with ULPREF as input source
#if defined(CLOCK_ROMDRIVER_PRESENT)
    ROMAPI_M4SS_CLK_API->clk_qspi_clk_config(M4CLK, QSPI_ULPREFCLK, QSPI_SWALLO_EN, QSPI_ODD_DIV_EN, QSPI_DIV_FACTOR);
#endif

    // Configure QSPI2 clock with ULPREF as input source
#ifdef SLI_SI91X_MCU_ENABLE_PSRAM_FEATURE
/* Configuring clock for PSRAM operation based on selected configs */
#if defined(CLOCK_ROMDRIVER_PRESENT)
    ROMAPI_M4SS_CLK_API->clk_qspi_2_clk_config(M4CLK, QSPI_ULPREFCLK, QSPI_SWALLO_EN, QSPI_ODD_DIV_EN, QSPI_DIV_FACTOR);
#endif
#endif
  }

  return sli_status;
}
/*******************************************************************************
 * @brief This API Switch Subsystems' Ref clocks to MHz RC,Set M4 SOC and QSPI/QSPI2 clock to Ref clock
 ******************************************************************************/
sl_status_t sli_si91x_config_clocks_to_mhz_rc(void)
{
  sl_status_t sli_status = SL_STATUS_OK;
  if (!(M4_ULP_SLP_STATUS_REG & ULP_MODE_SWITCHED_NPSS)) {
    // Change Subsystems' ref clocks from 40MHz XTAL to MHz RC
    MCU_FSM->MCU_FSM_REF_CLK_REG_b.M4SS_REF_CLK_SEL    = ULP_MHZ_RC_CLK;
    MCU_FSM->MCU_FSM_REF_CLK_REG_b.ULPSS_REF_CLK_SEL_b = ULPSS_ULP_MHZ_RC_CLK;
    /*wait for clock switched*/
    while ((M4CLK->PLL_STAT_REG_b.ULP_REF_CLK_SWITCHED) != true)
      ;
    // Configure M4 source to ULP REF clock
    M4CLK->CLK_CONFIG_REG5_b.M4_SOC_CLK_SEL = M4_ULPREFCLK;

    // Configure QSPI clock with ULPREF as input source
#if defined(CLOCK_ROMDRIVER_PRESENT)
    sli_status =
      ROMAPI_M4SS_CLK_API->clk_qspi_clk_config(M4CLK, QSPI_ULPREFCLK, QSPI_SWALLO_EN, QSPI_ODD_DIV_EN, QSPI_DIV_FACTOR);
#endif

    // Configure QSPI2 clock with ULPREF as input source
#ifdef SLI_SI91X_MCU_ENABLE_PSRAM_FEATURE
/* Configuring clock for PSRAM operation based on selected configs */
#if defined(CLOCK_ROMDRIVER_PRESENT)
    ROMAPI_M4SS_CLK_API->clk_qspi_2_clk_config(M4CLK, QSPI_ULPREFCLK, QSPI_SWALLO_EN, QSPI_ODD_DIV_EN, QSPI_DIV_FACTOR);
#endif
#endif
  }
  return sli_status;
}
