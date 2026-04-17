/***************************************************************************//**
 * @file
 * @brief Clock Manager Init HAL API implementations.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "em_device.h"
#if !defined(FPGA)

#include <stdbool.h>
#include <limits.h>
#include "sl_clock_manager_oscillator_config.h"
#include "sl_clock_manager_tree_config.h"
#include "sli_clock_manager_init_hal.h"
#include "sli_clock_manager_hal.h"
#include "sl_clock_manager_init.h"
#include "sl_se_manager_extmem.h"
#include "sl_se_manager_util.h"
#include "sl_se_manager.h"
#include "sl_clock_manager.h"
#include "sl_hal_syscfg.h"
#include "sl_hal_system.h"
#include "sl_hal_bus.h"
#include "sl_gpio.h"
#include "sl_status.h"
#include "sl_common.h"
#include "sl_assert.h"
#include "sl_core.h"
#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_STATIC_TOKENS)
#include "sl_token_manager_api.h"
#include "sl_token_manager_defines.h"
#include "sl_token_manager_manufacturing.h"
#include "sli_token_manager_internal.h"
#endif

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
#include "sli_clock_manager_runtime_configuration.h"
#endif

#if defined(SL_CATALOG_CLOCK_MANAGER_EXT_FLASH_PRESENT)
#include "sl_clock_manager_ext_flash_config.h"
#endif

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define HFXO_IRQ_NUMBER  HFXO0_IRQn

// Allocated PRS channel for crystal sharing
#if defined(_HFXO_CTRL_PRSSTATUSSEL0_MASK)
#define HFXO_CRYSTSAL_SHARING_PRS_CHANNEL 0
#endif

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
#define FUNCTION_SCOPE
#else
#define FUNCTION_SCOPE static
#endif

#if defined(SLI_CLOCK_MANAGER_TEST_QSPI_FREQUENCY_CONFIGURATION)
#define QSPI_FUNCTION_SCOPE
#else
#define QSPI_FUNCTION_SCOPE static
#endif

#if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)
// Bus Clock macros for SOCPLL instances.
#define SOCPLL_BUS_CLOCK(n)                         (((n) == 0) ? SL_BUS_CLOCK_SOCPLL0 \
                                                     : 0x0UL)

// Clock Manager configuration macros for SOCPLL instances.
#define CLOCK_MANAGER_SOCPLL_EN(n)                  (((n) == 0) ? SLI_CLOCK_MANAGER_SOCPLL_EN \
                                                     : 0x0UL)

#define CLOCK_MANAGER_SOCPLL_FREQ(n)                (((n) == 0) ? SLI_CLOCK_MANAGER_SOCPLL_FREQ \
                                                     : 0x0UL)

#define CLOCK_MANAGER_SOCPLL_ADVANCED_SETTINGS(n)   (((n) == 0) ? SL_CLOCK_MANAGER_SOCPLL_ADVANCED_SETTINGS \
                                                     : 0x0UL)

#define CLOCK_MANAGER_SOCPLL_REFCLK(n)              (((n) == 0) ? SLI_CLOCK_MANAGER_SOCPLL_REFCLK \
                                                     : 0x0UL)

#define CLOCK_MANAGER_SOCPLL_FRACTIONAL_EN(n)       (((n) == 0) ? SLI_CLOCK_MANAGER_SOCPLL_FRACTIONAL_EN \
                                                     : 0x0UL)

#define CLOCK_MANAGER_SOCPLL_DIVF(n)                (((n) == 0) ? SLI_CLOCK_MANAGER_SOCPLL_DIVF \
                                                     : 0x0UL)

#define CLOCK_MANAGER_SOCPLL_DIVN(n)                (((n) == 0) ? SLI_CLOCK_MANAGER_SOCPLL_DIVN \
                                                     : 0x0UL)

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
#define CLOCK_MANAGER_SOCPLL_EN_OPEN_LOOP(n)        (((n) == 0) ? SLI_CLOCK_MANAGER_SOCPLL_EN_OPEN_LOOP \
                                                     : 0x0UL)
#endif
#endif

/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/

extern uint16_t clock_manager_lfxo_precision;
extern uint16_t clock_manager_hfxo_precision;
#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301) \
  && !defined(SL_CATALOG_CLOCK_MANAGER_PTE_PRESENT)
extern sli_se_qspi_flpll_config_t clock_manager_flpll_cfg;
extern sl_se_command_context_t clock_manager_cmd_ctx;
extern uint32_t clock_manager_se_version;
extern sl_oscillator_t initial_qspi_reference_clock;
#endif

#if defined(SL_CLOCK_MANAGER_HFXO_STARTUP_TIME_MEASUREMENT_EN) \
  && SL_CLOCK_MANAGER_HFXO_STARTUP_TIME_MEASUREMENT_EN
extern uint16_t clock_manager_hfxo_startup_time;
#endif

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
uint32_t SLI_CLOCK_MANAGER_HFXO_MODE = SL_CLOCK_MANAGER_HFXO_MODE;
uint32_t SLI_CLOCK_MANAGER_HFXO_FREQ = SL_CLOCK_MANAGER_HFXO_FREQ;
uint32_t SLI_CLOCK_MANAGER_HFXO_CTUNE_FIXED_STEADY = SLI_CLOCK_MANAGER_HFXO_CTUNE_FIXED_STEADY_DEFAULT;

uint32_t SLI_CLOCK_MANAGER_HFRCO_BAND     = SL_CLOCK_MANAGER_HFRCO_BAND;
bool     SLI_CLOCK_MANAGER_HFRCO_DPLL_EN  = SL_CLOCK_MANAGER_HFRCO_DPLL_EN;
uint32_t SLI_CLOCK_MANAGER_DPLL_FREQ      = SL_CLOCK_MANAGER_DPLL_FREQ;
uint32_t SLI_CLOCK_MANAGER_DPLL_N         = SL_CLOCK_MANAGER_DPLL_N;
uint32_t SLI_CLOCK_MANAGER_DPLL_M         = SL_CLOCK_MANAGER_DPLL_M;

#if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)
bool     SLI_CLOCK_MANAGER_SOCPLL_EN            = SL_CLOCK_MANAGER_SOCPLL_EN;
uint32_t SLI_CLOCK_MANAGER_SOCPLL_FREQ          = SL_CLOCK_MANAGER_SOCPLL_FREQ;
uint32_t SLI_CLOCK_MANAGER_SOCPLL_REFCLK        = SL_CLOCK_MANAGER_SOCPLL_REFCLK;
#if (SL_CLOCK_MANAGER_SOCPLL_REFCLK == SOCPLL_CTRL_REFCLKSEL_REF_HFXO)
uint32_t SLI_CLOCK_MANAGER_SOCPLL_REFCLK_FREQ   = SL_CLOCK_MANAGER_HFXO_FREQ;
#elif (SL_CLOCK_MANAGER_SOCPLL_REFCLK == SOCPLL_CTRL_REFCLKSEL_REF_HFRCO)
uint32_t SLI_CLOCK_MANAGER_SOCPLL_REFCLK_FREQ   = SL_CLOCK_MANAGER_HFRCO_BAND;
#else
#error "Unsupported Reference Clock for SOCPLL with the Runtime configuration."
#endif
bool     SLI_CLOCK_MANAGER_SOCPLL_FRACTIONAL_EN = SL_CLOCK_MANAGER_SOCPLL_FRACTIONAL_EN;
uint32_t SLI_CLOCK_MANAGER_SOCPLL_DIVN          = SL_CLOCK_MANAGER_SOCPLL_DIVN;
uint32_t SLI_CLOCK_MANAGER_SOCPLL_DIVF          = SL_CLOCK_MANAGER_SOCPLL_DIVF;
bool SLI_CLOCK_MANAGER_SOCPLL_EN_OPEN_LOOP      = false;
#endif
#endif

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/
#if defined(SL_CLOCK_MANAGER_HFXO_EN) \
  && (SL_CLOCK_MANAGER_HFXO_EN == 1)
static void get_hfxo_ctune(uint8_t *ctune_xi_steady, uint8_t *ctune_xo_steady)
{
#if (SL_CLOCK_MANAGER_HFXO_MODE == HFXO_CFG_MODE_XTAL) || defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
  if (SLI_CLOCK_MANAGER_HFXO_MODE == HFXO_CFG_MODE_XTAL) {
#else
  if (true) {
#endif
    // Determine CTUNE value.
    int32_t ctune = -1;

#if defined(SL_CLOCK_MANAGER_HFXO_SEPARATE_CTUNEXIANA_CTUNEXOANA_EN) && (SL_CLOCK_MANAGER_HFXO_SEPARATE_CTUNEXIANA_CTUNEXOANA_EN == 1)
    // Use separate XI and XO CTUNE values
    *ctune_xi_steady = SL_CLOCK_MANAGER_HFXO_CTUNEXIANA;
    *ctune_xo_steady = SL_CLOCK_MANAGER_HFXO_CTUNEXOANA;
    return;
#endif

    // Retrieve HFXO calibration value from DEVINFO data.
    ctune = sl_hal_system_get_hfxocal();
    if (ctune == 0) {
      ctune = -1;
    }

#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_STATIC_TOKENS)
    // Use HFXO tuning value from MFG token in UD page if not already set
    if (ctune == -1) {
      uint16_t mfg_ctune = 0;
      sl_status_t status;
      CORE_ATOMIC_SECTION(
        // Since SE Command is necessary to get the calibration value, and sl_se_init() is not yet called,
        // we need to enter atomic mode to prevent the SE Command from being interrupted.
        status = sli_token_manager_get_token_data_preinit(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(TOKEN_MFG_CTUNE),
                                                                      &mfg_ctune, TOKEN_MFG_CTUNE_SIZE);
      )
#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
      if (status != SL_STATUS_OK) {
        status = sl_token_manager_get_data(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(TOKEN_MFG_CTUNE),
                                           &mfg_ctune, TOKEN_MFG_CTUNE_SIZE);
      }
#endif
      if ((status == SL_STATUS_OK)
          && (mfg_ctune <= (_HFXO_XTALCTRL_CTUNEXIANA_MASK >> _HFXO_XTALCTRL_CTUNEXIANA_SHIFT))) {
        ctune = mfg_ctune;
      }
    }
#endif

    // Use HFXO tuning value from configurations as fallback
    if (ctune == -1) {
      ctune = SL_CLOCK_MANAGER_HFXO_CTUNE;
    }

    // Configure CTUNE XI and XO.
    if (ctune != -1) {
      *ctune_xi_steady = (uint8_t)ctune;

      // The delta accounts for internal chip load imbalance on chips with the BUFOUT feature
      // which is not present on Rainer.
      uint8_t ctune_delta = 0;

      // Ensure CTUNE XO plus a delta is within the correct range.
      ctune += ctune_delta;
      if (ctune > (int32_t)(_HFXO_XTALCTRL_CTUNEXOANA_MASK >> _HFXO_XTALCTRL_CTUNEXOANA_SHIFT)) {
        ctune = _HFXO_XTALCTRL_CTUNEXOANA_MASK >> _HFXO_XTALCTRL_CTUNEXOANA_SHIFT;
      }
      *ctune_xo_steady =  (uint8_t)ctune;
    } else {
      EFM_ASSERT(false);
    }
  }
#endif
}

/***************************************************************************//**
 * Initializes HFXO.
 ******************************************************************************/
FUNCTION_SCOPE void init_hfxo(void)
{
  sl_status_t status;
  uint32_t timeout_corebias_lsb;
  uint32_t timeout_steady_first_lock;
  uint32_t corebias_steady;
  uint8_t ctune_xi_steady;
  uint8_t ctune_xo_steady;
  uint8_t enXiDcBiasAna;

  // Enable HFXO bus clock.
  status = sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_HFXO0);
  EFM_ASSERT(status == SL_STATUS_OK);

#if defined(SL_CLOCK_MANAGER_HFXO_STARTUP_TIME_MEASUREMENT_EN) && SL_CLOCK_MANAGER_HFXO_STARTUP_TIME_MEASUREMENT_EN
  // Setup LF clock to measure startup time
  CLOCK_MANAGER_CLOCK_SELECT_SET(HFXO0LFCLK, SL_CLOCK_MANAGER_HFXO0LFCLK_SOURCE);
#endif

#if defined(SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_SUPPORT) && (SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_SUPPORT == 1)
  // Configure Sleepy Crystal settings
  // Configure CTUNE values for ANA stage : Only the 3 most significant bits of the CTUNE value are used
  HFXO0->SLEEPYXTALCFG0 = (HFXO0->SLEEPYXTALCFG0 & ~(_HFXO_SLEEPYXTALCFG0_CTUNEXIANASLEEPY_MASK | _HFXO_SLEEPYXTALCFG0_CTUNEXOANASLEEPY_MASK))
                          | ((SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_CTUNE_VALUE >> 5) << _HFXO_SLEEPYXTALCFG0_CTUNEXIANASLEEPY_SHIFT)
                          | ((SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_CTUNE_VALUE >> 5) << _HFXO_SLEEPYXTALCFG0_CTUNEXOANASLEEPY_SHIFT);
  // Configure Core Bias values for Startup, Startup Intermediate and ANA stages
  HFXO0->SLEEPYXTALCFG1 = (HFXO0->SLEEPYXTALCFG1 & ~(_HFXO_SLEEPYXTALCFG1_COREBIASSUPSLEEPY_MASK | _HFXO_SLEEPYXTALCFG1_COREBIASSUPISLEEPY_MASK | _HFXO_SLEEPYXTALCFG1_COREBIASANASLEEPY_MASK))
                          | (SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_CORE_BIAS_VALUE << _HFXO_SLEEPYXTALCFG1_COREBIASSUPSLEEPY_SHIFT)
                          | (SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_CORE_BIAS_VALUE << _HFXO_SLEEPYXTALCFG1_COREBIASSUPISLEEPY_SHIFT)
                          | (SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_CORE_BIAS_VALUE << _HFXO_SLEEPYXTALCFG1_COREBIASANASLEEPY_SHIFT);
  HFXO0->CFG_SET = HFXO_CFG_SLEEPYXTALSUPEN;

  // Enable HFXO Interrupt to handle HFXO error
  HFXO0->IEN_CLR = HFXO_IEN_SLEEPYXTAL;
  HFXO0->IF_CLR = HFXO_IF_SLEEPYXTAL;

  NVIC_ClearPendingIRQ(HFXO_IRQ_NUMBER);
  NVIC_EnableIRQ(HFXO_IRQ_NUMBER);

  HFXO0->IEN_SET = HFXO_IEN_SLEEPYXTAL;
#endif

#if (SL_CLOCK_MANAGER_HFXO_MODE == HFXO_CFG_MODE_XTAL) || defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
  if (SLI_CLOCK_MANAGER_HFXO_MODE == HFXO_CFG_MODE_XTAL) {
#else
  if (true) {
#endif
    timeout_corebias_lsb = _HFXO_XTALCFG_TIMEOUTCBLSB_T416US;
    timeout_steady_first_lock = _HFXO_XTALCFG_TIMEOUTSTEADY_T833US;
    corebias_steady = 60U;
  }
#else
  timeout_corebias_lsb = 0;
  timeout_steady_first_lock = 0;
  corebias_steady = 0U;
#endif
  ctune_xi_steady = 0;
  ctune_xo_steady = 0;

#if defined(SL_CLOCK_MANAGER_HFXO_ENXIDCBIASANA)
  enXiDcBiasAna = SL_CLOCK_MANAGER_HFXO_ENXIDCBIASANA;
#else
  enXiDcBiasAna = _HFXO_CFG_ENXIDCBIASANA_DEFAULT;
#endif

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
  // If the Core Bias Optimization is being skipped, it means it was already done and
  // we shouldn't change the Core Bias again.
  if (HFXO0->XTALCTRL & HFXO_XTALCTRL_SKIPCOREBIASOPT) {
    corebias_steady = (HFXO0->XTALCTRL & _HFXO_XTALCTRL_COREBIASANA_MASK) >> _HFXO_XTALCTRL_COREBIASANA_SHIFT;
  }
#endif

  EFM_ASSERT(SLI_CLOCK_MANAGER_HFXO_MODE <= (_HFXO_CFG_MODE_MASK >> _HFXO_CFG_MODE_SHIFT));

  // If HFXO is already selected as SYSCLK Clock Manager has already been initialized.
  // Startup time cannot occur if it's the case
  if ((CMU->SYSCLKCTRL & _CMU_SYSCLKCTRL_CLKSEL_MASK) == CMU_SYSCLKCTRL_CLKSEL_HFXO) {
    EFM_ASSERT(false);
  }

  // Unlock register interface.
  HFXO0->LOCK = HFXO_LOCK_LOCKKEY_UNLOCK;

  // Disable HFXO.
  HFXO0->CTRL_SET = HFXO_CTRL_DISONDEMAND;
  HFXO0->CTRL_CLR = HFXO_CTRL_FORCEEN;
  while (((HFXO0->STATUS & _HFXO_STATUS_ENS_MASK) != 0U) || ((HFXO0->STATUS & _HFXO_STATUS_SYNCBUSY_MASK) != 0U)) {
    // Wait for ENS or SyncBusy flags.
  }

  // Determine CTUNE value.
  get_hfxo_ctune(&ctune_xi_steady, &ctune_xo_steady);

  // Configure crystal related settings.
  sl_hal_bus_reg_write_mask(&HFXO0->XTALCFG,
                            _HFXO_XTALCFG_TIMEOUTCBLSB_MASK | _HFXO_XTALCFG_TIMEOUTSTEADY_MASK
                            | _HFXO_XTALCFG_CTUNEXOSTARTUP_MASK | _HFXO_XTALCFG_CTUNEXISTARTUP_MASK,
                            (timeout_corebias_lsb << _HFXO_XTALCFG_TIMEOUTCBLSB_SHIFT)
                            | (timeout_steady_first_lock << _HFXO_XTALCFG_TIMEOUTSTEADY_SHIFT)
                            | (0U << _HFXO_XTALCFG_CTUNEXOSTARTUP_SHIFT)
                            | (0U << _HFXO_XTALCFG_CTUNEXISTARTUP_SHIFT));

  sl_hal_bus_reg_write_mask(&HFXO0->XTALCTRL, _HFXO_XTALCTRL_CTUNEXOANA_MASK
                            | _HFXO_XTALCTRL_CTUNEXIANA_MASK | _HFXO_XTALCTRL_COREBIASANA_MASK,
                            (ctune_xo_steady << _HFXO_XTALCTRL_CTUNEXOANA_SHIFT)
                            | (ctune_xi_steady << _HFXO_XTALCTRL_CTUNEXIANA_SHIFT)
                            | (corebias_steady << _HFXO_XTALCTRL_COREBIASANA_SHIFT));

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
  if (SLI_CLOCK_MANAGER_HFXO_CTUNE_FIXED_STEADY == SLI_CLOCK_MANAGER_HFXO_CTUNE_FIXED_STEADY_DEFAULT) {
    SLI_CLOCK_MANAGER_HFXO_CTUNE_FIXED_STEADY = (HFXO0->XTALCTRL & _HFXO_XTALCTRL_CTUNEFIXANA_MASK) >> _HFXO_XTALCTRL_CTUNEFIXANA_SHIFT;
  } else {
    sl_hal_bus_reg_write_mask(&HFXO0->XTALCTRL, _HFXO_XTALCTRL_CTUNEFIXANA_MASK, SLI_CLOCK_MANAGER_HFXO_CTUNE_FIXED_STEADY << _HFXO_XTALCTRL_CTUNEFIXANA_SHIFT);
  }
#endif

  // Configure HFXO module settings
  sl_hal_bus_reg_write_mask(&HFXO0->CFG,
                            _HFXO_CFG_ENXIDCBIASANA_MASK | _HFXO_CFG_MODE_MASK,
                            (enXiDcBiasAna << _HFXO_CFG_ENXIDCBIASANA_SHIFT)
                            | (SLI_CLOCK_MANAGER_HFXO_MODE << _HFXO_CFG_MODE_SHIFT));

#if (SL_CLOCK_MANAGER_HFXO_MODE == HFXO_CFG_MODE_XTAL) || defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
  if (SLI_CLOCK_MANAGER_HFXO_MODE == HFXO_CFG_MODE_XTAL) {
#else
  if (true) {
#endif
    // Set HFXO with FORCEEN bit set for first startup.
    HFXO0->CTRL_SET = HFXO_CTRL_FORCEEN;

    while ((HFXO0->STATUS & (HFXO_STATUS_RDY | HFXO_STATUS_COREBIASOPTRDY | HFXO_STATUS_ENS))
           != (HFXO_STATUS_RDY | HFXO_STATUS_COREBIASOPTRDY | HFXO_STATUS_ENS)) {
      // Wait for HFXO lock and core bias algorithm to complete.
    }

    // Set new TIMEOUTSTEADY value for use on subsequent locks.
    HFXO0->XTALCFG = (HFXO0->XTALCFG & ~_HFXO_XTALCFG_TIMEOUTSTEADY_MASK)
                     | (_HFXO_XTALCFG_TIMEOUTSTEADY_T83US
                        << _HFXO_XTALCFG_TIMEOUTSTEADY_SHIFT);

    // Skip core bias algorithm on subsequent locks.
    HFXO0->XTALCTRL_SET = HFXO_XTALCTRL_SKIPCOREBIASOPT;
  }
#endif

  // Turn off HFXO
  HFXO0->CTRL_CLR = HFXO_CTRL_FORCEEN;

  // Prepare for the next measurements to get a better average.
#if defined(SL_CLOCK_MANAGER_HFXO_STARTUP_TIME_MEASUREMENT_EN) && SL_CLOCK_MANAGER_HFXO_STARTUP_TIME_MEASUREMENT_EN \
  && (SL_CLOCK_MANAGER_HFXO_MODE == HFXO_CFG_MODE_XTAL)

  while (((HFXO0->STATUS & _HFXO_STATUS_ENS_MASK) != 0U) || ((HFXO0->STATUS & _HFXO_STATUS_SYNCBUSY_MASK) != 0U)) {
    // Wait for HFXO to be disabled
  }

  // Configure HFXO for startup time measurement
  sl_hal_bus_reg_write_mask(&HFXO0->CFG,
                            _HFXO_CFG_NUMSTUPMEAS_MASK | _HFXO_CFG_STUPMEASEN_MASK,
                            SL_CLOCK_MANAGER_HFXO_STARTUP_TIME_MEASUREMENTS | HFXO_CFG_STUPMEASEN);

  // Clear any interrupt
  HFXO0->IF_CLR = HFXO_IF_STUPMEASDONE;

  // Enable HFXO Interrupt to handle HFXO startup time measurement
  NVIC_EnableIRQ(HFXO_IRQ_NUMBER);
  HFXO0->IEN_SET = HFXO_IEN_STUPMEASDONE;

  // Start the measure
  HFXO0->CMD_SET = HFXO_CMD_STARTMEAS;

#endif

  // Keep oscillator on-demand.
  HFXO0->CTRL_CLR = HFXO_CTRL_DISONDEMAND;

  // Update HFXO precision
  clock_manager_hfxo_precision = SL_CLOCK_MANAGER_HFXO_PRECISION;

  // Update HFXO Frequency
  SystemHFXOClockSet(SLI_CLOCK_MANAGER_HFXO_FREQ);
}
#endif

#if defined(SL_CLOCK_MANAGER_LFXO_EN) \
  && (SL_CLOCK_MANAGER_LFXO_EN == 1)
/***************************************************************************//**
 * Initializes LFXO.
 ******************************************************************************/
FUNCTION_SCOPE void init_lfxo(void)
{
  uint8_t gain;
  int8_t ctune = -1;

  // Adjust LFXO parameters based on specified mode
#if SL_CLOCK_MANAGER_LFXO_MODE == LFXO_CFG_MODE_XTAL
#ifdef SL_CLOCK_MANAGER_LFXO_GAIN
  gain = SL_CLOCK_MANAGER_LFXO_GAIN;
#else
  gain = _LFXO_CAL_GAIN_DEFAULT;
#endif
#else
  gain = 0;
#endif

  EFM_ASSERT(SL_CLOCK_MANAGER_LFXO_TIMEOUT
             <= _LFXO_CFG_TIMEOUT_MASK);
  EFM_ASSERT(SL_CLOCK_MANAGER_LFXO_MODE
             <= _LFXO_CFG_MODE_MASK);
  EFM_ASSERT(gain
             <= (_LFXO_CAL_GAIN_MASK >> _LFXO_CAL_GAIN_SHIFT));
  EFM_ASSERT(SL_CLOCK_MANAGER_LFXO_CTUNE
             <= (_LFXO_CAL_CAPTUNE_MASK >> _LFXO_CAL_CAPTUNE_SHIFT));

#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_STATIC_TOKENS)
  if (ctune == -1) {
    uint8_t mfg_lxfo_tune = 0;
    sl_status_t status;
    CORE_ATOMIC_SECTION(
      status = sli_token_manager_get_token_data_preinit(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(TOKEN_MFG_LFXO_TUNE),
                                                                    &mfg_lxfo_tune, TOKEN_MFG_LFXO_TUNE_SIZE);
    )
    if ((status == SL_STATUS_OK)
        && (mfg_lxfo_tune <= (_LFXO_CAL_CAPTUNE_MASK >> _LFXO_CAL_CAPTUNE_SHIFT))) {
      ctune = mfg_lxfo_tune;
    }
  }
#endif

  // Use LFXO tuning value from configuration header as fallback
  if (ctune == -1) {
    ctune = SL_CLOCK_MANAGER_LFXO_CTUNE;
  }

  // Max internal capacitance tuning value is 0x59 (29.1 pF)
  ctune = (uint8_t) SL_MIN(0x59U, (uint8_t)ctune);

  // Enable Bus Clock for LFXO.
  CMU->CLKEN0_SET = CMU_CLKEN0_LFXO;

  // Unlock register interface.
  LFXO->LOCK = LFXO_LOCK_LOCKKEY_UNLOCK;

  // Disable LFXO.
  LFXO->CTRL_SET = LFXO_CTRL_DISONDEMAND;
  LFXO->CTRL_CLR = LFXO_CTRL_FORCEEN;
  while ((LFXO->STATUS & _LFXO_STATUS_ENS_MASK) != 0U) {
  }

  // Configure LFXO as specified.
  LFXO->CAL = (LFXO->CAL & ~(_LFXO_CAL_GAIN_MASK | _LFXO_CAL_CAPTUNE_MASK))
              | ((uint32_t)gain  << _LFXO_CAL_GAIN_SHIFT)
              | ((uint32_t)ctune << _LFXO_CAL_CAPTUNE_SHIFT);

  LFXO->CFG = ((LFXO->CFG & ~(_LFXO_CFG_MODE_MASK | _LFXO_CFG_TIMEOUT_MASK
                              | _LFXO_CFG_AGCRATE_MASK | _LFXO_CFG_DRVCURR_MASK
                              | _LFXO_CFG_REFLEVEL_MASK))
               | SL_CLOCK_MANAGER_LFXO_MODE
               | SL_CLOCK_MANAGER_LFXO_TIMEOUT
               | LFXO_CFG_AGCRATE_DEFAULT
               | LFXO_CFG_DRVCURR_DEFAULT
               | LFXO_CFG_REFLEVEL_DEFAULT);

  // Re-enable on-demand.
  LFXO->CTRL_CLR = LFXO_CTRL_DISONDEMAND;

  clock_manager_lfxo_precision = SL_CLOCK_MANAGER_LFXO_PRECISION;
}
#endif

/***************************************************************************//**
 * Initializes Clock Input CLKIN0.
 ******************************************************************************/
static void init_clkin0(void)
{
#if (defined(SL_CLOCK_MANAGER_SYSCLK_SOURCE) && (SL_CLOCK_MANAGER_SYSCLK_SOURCE == CMU_SYSCLKCTRL_CLKSEL_CLKIN0)) \
  || (defined(SL_CLOCK_MANAGER_DPLL_REFCLK) && (SL_CLOCK_MANAGER_DPLL_REFCLK == CMU_DPLLREFCLKCTRL_CLKSEL_CLKIN0))

#if !defined(SL_CLOCK_MANAGER_CLKIN0_PORT) || !defined(SL_CLOCK_MANAGER_CLKIN0_PIN)
#error "Invalid configuration: CLKIN0 reference can't be use without configuring SL_CLOCK_MANAGER_CLKIN0 with a valid port and pin."
#endif

  sl_status_t status;
  sl_gpio_t clkin0_gpio = { SL_CLOCK_MANAGER_CLKIN0_PORT, SL_CLOCK_MANAGER_CLKIN0_PIN };

  status = sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
  EFM_ASSERT(status == SL_STATUS_OK);

  status = sl_gpio_set_pin_mode(&clkin0_gpio, SL_GPIO_MODE_INPUT, false);
  EFM_ASSERT(status == SL_STATUS_OK);

  GPIO->CMUROUTE.CLKIN0ROUTE = (clkin0_gpio.port << _GPIO_CMU_CLKIN0ROUTE_PORT_SHIFT)
                               | (clkin0_gpio.pin << _GPIO_CMU_CLKIN0ROUTE_PIN_SHIFT);
#endif
}

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION) \
  || (defined(SL_CLOCK_MANAGER_HFRCO_DPLL_EN) && (SL_CLOCK_MANAGER_HFRCO_DPLL_EN == 1))
/**************************************************************************//**
 * Initializes DPLL.
 *****************************************************************************/
FUNCTION_SCOPE void init_dpll(void)
{
#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
  if (SLI_CLOCK_MANAGER_HFRCO_DPLL_EN)
#endif
  {
    uint32_t hfrco_cal_val;
    uint32_t lock_status = 0;

    // Enable DPLL module's clock.
    CMU->CLKEN0_SET = CMU_CLKEN0_DPLL0;

    // Disable DPLL before configuring.
    DPLL0->EN_CLR = DPLL_EN_EN;
    while (DPLL0->EN & DPLL_EN_DISABLING) {
      // Wait for DPLL to be disabled before configuring.
    }

    EFM_ASSERT(SLI_CLOCK_MANAGER_DPLL_N <= (_DPLL_CFG1_N_MASK >> _DPLL_CFG1_N_SHIFT));
    EFM_ASSERT(SLI_CLOCK_MANAGER_DPLL_M <= (_DPLL_CFG1_M_MASK >> _DPLL_CFG1_M_SHIFT));

#if !defined(SL_CATALOG_CLOCK_MANAGER_PTE_PRESENT)
    CORE_ATOMIC_SECTION(
      // Since SE Command is necessary to get the calibration value, and sl_se_init() is not yet called,
      // we need to enter atomic mode to prevent the SE Command from being interrupted.
      hfrco_cal_val = sl_hal_system_get_hfrcodpll_band_calibration(SLI_CLOCK_MANAGER_DPLL_FREQ);
    )
    EFM_ASSERT((hfrco_cal_val != 0UL) && (hfrco_cal_val != UINT32_MAX));

    while (HFRCO0->STATUS & (HFRCO_STATUS_SYNCBUSY | HFRCO_STATUS_FREQBSY)) {
      // Updates to the CAL register are deferred if FREQBSY is high, so wait
      // until HFRCO is not busy to keep going.
    }

    HFRCO0->CAL = hfrco_cal_val;
#endif

    DPLL0->CFG1 = (DPLL0->CFG1 & ~(_DPLL_CFG1_N_MASK | _DPLL_CFG1_M_MASK))
                  | (SLI_CLOCK_MANAGER_DPLL_N << _DPLL_CFG1_N_SHIFT)
                  | (SLI_CLOCK_MANAGER_DPLL_M << _DPLL_CFG1_M_SHIFT);

    // Set the DPLL clock reference.
    CMU->DPLLREFCLKCTRL = (CMU->DPLLREFCLKCTRL & ~_CMU_DPLLREFCLKCTRL_CLKSEL_MASK)
                          | SL_CLOCK_MANAGER_DPLL_REFCLK;

    DPLL0->CFG = (DPLL0->CFG & ~(_DPLL_CFG_AUTORECOVER_MASK | _DPLL_CFG_DITHEN_MASK
                                 | _DPLL_CFG_EDGESEL_MASK | _DPLL_CFG_MODE_MASK))
                 | (SL_CLOCK_MANAGER_DPLL_AUTORECOVER << _DPLL_CFG_AUTORECOVER_SHIFT)
                 | (SL_CLOCK_MANAGER_DPLL_DITHER  << _DPLL_CFG_DITHEN_SHIFT)
                 | (SL_CLOCK_MANAGER_DPLL_EDGE  << _DPLL_CFG_EDGESEL_SHIFT)
                 | (SL_CLOCK_MANAGER_DPLL_LOCKMODE << _DPLL_CFG_MODE_SHIFT);

    // Update CMSIS HFRCODPLL frequency.
    SystemHFRCODPLLClockSet(SLI_CLOCK_MANAGER_DPLL_FREQ);

    // Enable DPLL.
    DPLL0->IF_CLR = DPLL_IF_LOCK | DPLL_IF_LOCKFAILLOW | DPLL_IF_LOCKFAILHIGH;

    // Lock DPLL.
    DPLL0->EN_SET = DPLL_EN_EN;
    while (lock_status == 0UL) {
      lock_status = DPLL0->IF;
    }

    EFM_ASSERT(lock_status == DPLL_IF_LOCK);
  }
}
#endif

/***************************************************************************//**
 * Initializes HFRCODPLL.
 ******************************************************************************/
FUNCTION_SCOPE void init_hfrcodpll(void)
{
#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION) \
  || (defined(SL_CLOCK_MANAGER_HFRCO_DPLL_EN) && (SL_CLOCK_MANAGER_HFRCO_DPLL_EN == 1))

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
  if (SLI_CLOCK_MANAGER_HFRCO_DPLL_EN)
#else
  if (true)
#endif
  {
    sl_status_t status;

    // The System Clock should not be running from the HFRCO.
    EFM_ASSERT((CMU->SYSCLKCTRL & _CMU_SYSCLKCTRL_CLKSEL_MASK) != CMU_SYSCLKCTRL_CLKSEL_HFRCODPLL);

    // Enable HFRCO module's clock.
    status = sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_HFRCO0);
    EFM_ASSERT(status == SL_STATUS_OK);

    // Initialize DPLL.
    init_dpll();
  }
#endif
// Initialize HFRCO without DPLL.
#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION) \
  || (defined(SL_CLOCK_MANAGER_HFRCO_DPLL_EN) && (SL_CLOCK_MANAGER_HFRCO_DPLL_EN == 0))

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
  else
#else
  if (true)
#endif
  {
    sl_status_t status;
    uint32_t freq_cal = 0;

    // Retrieve HFRCO calibration from DEVINFO data.
    switch (SLI_CLOCK_MANAGER_HFRCO_BAND) {
#if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)
      case 38000000:
        freq_cal = sl_hal_system_get_hfrco_default_calibration();
        break;
#endif
#if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_353)
      case 40000000:
        freq_cal = sl_hal_system_get_hfrco_default_calibration();
        break;
#endif
      case 100000000:
        freq_cal = sl_hal_system_get_hfrco_speed_calibration();
        break;
      default:
        EFM_ASSERT(false);
        break;
    }
    EFM_ASSERT((freq_cal != 0UL) && (freq_cal != UINT32_MAX));

    // Enable HFRCO module's clock.
    status = sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_HFRCO0);
    EFM_ASSERT(status == SL_STATUS_OK);

    // Make sure DPLL is disabled before configuring.
    status = sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_DPLL0);
    EFM_ASSERT(status == SL_STATUS_OK);
    EFM_ASSERT(DPLL0->EN == 0);
    status = sl_clock_manager_disable_bus_clock(SL_BUS_CLOCK_DPLL0);
    EFM_ASSERT(status == SL_STATUS_OK);

    while (HFRCO0->STATUS & (HFRCO_STATUS_SYNCBUSY | HFRCO_STATUS_FREQBSY)) {
      // Updates to the CAL register are deferred if FREQBSY is high, so wait
      // until HFRCO is not busy to keep going.
    }

    HFRCO0->CAL = freq_cal;

    // Update CMSIS HFRCODPLL frequency.
    SystemHFRCODPLLClockSet(SLI_CLOCK_MANAGER_HFRCO_BAND);
  }
#endif
}

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)                              \
  || (defined(SL_CLOCK_MANAGER_SOCPLL_EN) && (SL_CLOCK_MANAGER_SOCPLL_EN == 1))   \
  || (defined(SL_CLOCK_MANAGER_SOCPLL0_EN) && (SL_CLOCK_MANAGER_SOCPLL0_EN == 1)) \
  || (defined(SL_CLOCK_MANAGER_SOCPLL1_EN) && (SL_CLOCK_MANAGER_SOCPLL1_EN == 1)) \
  || (defined(SL_CLOCK_MANAGER_SOCPLL2_EN) && (SL_CLOCK_MANAGER_SOCPLL2_EN == 1))
/***************************************************************************//**
 * Initializes SOCPLL.
 ******************************************************************************/
FUNCTION_SCOPE void init_socpll(uint8_t socpll_num)
{
  sl_status_t status;
  SOCPLL_TypeDef *socpll = SOCPLL(socpll_num);
  EFM_ASSERT(socpll != NULL);
  sl_bus_clock_t socpll_bus_clock = SOCPLL_BUS_CLOCK(socpll_num);

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
  if (!CLOCK_MANAGER_SOCPLL_EN(socpll_num)) {
    return;
  }
#endif
  // Enable Bus Clock for SOCPLL.
  status = sl_clock_manager_enable_bus_clock(socpll_bus_clock);
  EFM_ASSERT(status == SL_STATUS_OK);

  // Unlock register interface.
  socpll->LOCK = SOCPLL_LOCK_LOCKKEY_UNLOCK;

  // Disable clock on-demand while configuring SOCPLL.
  socpll->CTRL_SET = SOCPLL_CTRL_DISONDEMAND;
  socpll->CTRL_CLR = SOCPLL_CTRL_FORCEEN;
  while ((socpll->STATUS & _SOCPLL_STATUS_ENS_MASK) != 0U) {
    // Wait until SOCPLL is disabled.
  }

  // Verify invalid configurations.
  // If Integer-N mode is selected, DIVN cannot be 0.
  EFM_ASSERT(CLOCK_MANAGER_SOCPLL_FRACTIONAL_EN(socpll_num) != 0
             || CLOCK_MANAGER_SOCPLL_DIVN(socpll_num) != 0);
  // If Integer-N mode is selected, DIVF must be 0.
  uint32_t divf = CLOCK_MANAGER_SOCPLL_FRACTIONAL_EN(socpll_num) ? CLOCK_MANAGER_SOCPLL_DIVF(socpll_num) : 0;

  // Set the SOCPLL clock reference and dividers.
#if defined(_SOCPLL_CTRL1_MASK)
  socpll->CTRL = (socpll->CTRL & ~(_SOCPLL_CTRL_ENFRACN_MASK | _SOCPLL_CTRL_REFCLKSEL_MASK))
                 | (CLOCK_MANAGER_SOCPLL_FRACTIONAL_EN(socpll_num) << _SOCPLL_CTRL_ENFRACN_SHIFT)
                 | (CLOCK_MANAGER_SOCPLL_REFCLK(socpll_num));
  socpll->CTRL1 = (socpll->CTRL1 & ~(_SOCPLL_CTRL1_DIVN_MASK | _SOCPLL_CTRL1_DIVF_MASK))
                  | (CLOCK_MANAGER_SOCPLL_DIVN(socpll_num) << _SOCPLL_CTRL1_DIVN_SHIFT)
                  | (divf << _SOCPLL_CTRL1_DIVF_SHIFT);
#else
  socpll->CTRL = (socpll->CTRL & ~(_SOCPLL_CTRL_ENFRACN_MASK | _SOCPLL_CTRL_REFCLKSEL_MASK | _SOCPLL_CTRL_DIVN_MASK | _SOCPLL_CTRL_DIVF_MASK))
                 | (CLOCK_MANAGER_SOCPLL_FRACTIONAL_EN(socpll_num) << _SOCPLL_CTRL_ENFRACN_SHIFT)
                 | (CLOCK_MANAGER_SOCPLL_REFCLK(socpll_num))
                 | (CLOCK_MANAGER_SOCPLL_DIVN(socpll_num) << _SOCPLL_CTRL_DIVN_SHIFT)
                 | (divf << _SOCPLL_CTRL_DIVF_SHIFT);
#endif

#if defined(_SOCPLL_SOCCLK0_MASK)
  // Set the output dividers.
  socpll->SOCCLK0 = (socpll->SOCCLK0 & ~(_SOCPLL_SOCCLK0_SOCCLK0OUTDIV_MASK | _SOCPLL_SOCCLK0_ENSOCCLK0_MASK))
                    | (CLOCK_MANAGER_SOCPLL_OUT0_DIV(socpll_num) << _SOCPLL_SOCCLK0_SOCCLK0OUTDIV_SHIFT)
                    | (CLOCK_MANAGER_SOCPLL_OUT0_EN(socpll_num) << _SOCPLL_SOCCLK0_ENSOCCLK0_SHIFT);

  socpll->SOCCLK1 = (socpll->SOCCLK1 & ~(_SOCPLL_SOCCLK1_SOCCLK1OUTDIV_MASK | _SOCPLL_SOCCLK1_ENSOCCLK1_MASK))
                    | (CLOCK_MANAGER_SOCPLL_OUT1_DIV(socpll_num) << _SOCPLL_SOCCLK1_SOCCLK1OUTDIV_SHIFT)
                    | (CLOCK_MANAGER_SOCPLL_OUT1_EN(socpll_num) << _SOCPLL_SOCCLK1_ENSOCCLK1_SHIFT);

  socpll->SOCCLK2 = (socpll->SOCCLK2 & ~(_SOCPLL_SOCCLK2_SOCCLK2OUTDIV_MASK | _SOCPLL_SOCCLK2_ENSOCCLK2_MASK))
                    | (CLOCK_MANAGER_SOCPLL_OUT2_DIV(socpll_num) << _SOCPLL_SOCCLK2_SOCCLK2OUTDIV_SHIFT)
                    | (CLOCK_MANAGER_SOCPLL_OUT2_EN(socpll_num) << _SOCPLL_SOCCLK2_ENSOCCLK2_SHIFT);

  socpll->DCOCFG = (socpll->DCOCFG & ~_SOCPLL_DCOCFG_OUTDIVINSEL_MASK)
                   | (CLOCK_MANAGER_SOCPLL_OUTDIVIN(socpll_num) << _SOCPLL_DCOCFG_OUTDIVINSEL_SHIFT);
#endif

#if defined(_SOCPLL_AUTO_MASK)
  // Load FREQPLAN if available and start AUTO mode.
  const sl_clock_manager_freqplan_data_t *freqplan_data = CLOCK_MANAGER_SOCPLL_FREQPLAN_DATA(socpll_num);
  if (freqplan_data != NULL) {
    for (uint8_t i = 0; i < FREQPLAN_FREQSEL_WIDTH; i++) {
      socpll->FREQPLANCTRL[i].AUTO = (uint32_t)(freqplan_data->socpll_config[i]);
    }
    socpll->CTRL1_SET = SOCPLL_CTRL1_MODESEL;
  }
#endif

  // Force a clock request by software to update analog part of SOCPLL.
  socpll->CTRL_SET = SOCPLL_CTRL_FORCEEN;

  while ((socpll->STATUS & (SOCPLL_STATUS_RDY | SOCPLL_STATUS_PLLLOCK | SOCPLL_STATUS_ENS))
         != (SOCPLL_STATUS_RDY | SOCPLL_STATUS_PLLLOCK | SOCPLL_STATUS_ENS)) {
    // Wait for SOCPLL lock and ready.
  }

#if defined(_SOCPLL_CTRL1_ENOPENLOOP_MASK)
  socpll->CTRL1 = (socpll->CTRL1 & ~(_SOCPLL_CTRL1_ENOPENLOOP_MASK)) | (CLOCK_MANAGER_SOCPLL_EN_OPEN_LOOP(socpll_num) << _SOCPLL_CTRL1_ENOPENLOOP_SHIFT);
#elif defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
  socpll->CTRL = (socpll->CTRL & ~(_SOCPLL_CTRL_ENOPENLOOP_MASK)) | (CLOCK_MANAGER_SOCPLL_EN_OPEN_LOOP(socpll_num) << _SOCPLL_CTRL_ENOPENLOOP_SHIFT);
#endif

  // Update CMSIS SOCPLL frequency.
#if defined(_SOCPLL_SOCCLK0_MASK)
  SystemSOCPLLClockSet(socpll_num, 0, CLOCK_MANAGER_SOCPLL_FREQ0(socpll_num));
  SystemSOCPLLClockSet(socpll_num, 1, CLOCK_MANAGER_SOCPLL_FREQ1(socpll_num));
  SystemSOCPLLClockSet(socpll_num, 2, CLOCK_MANAGER_SOCPLL_FREQ2(socpll_num));
#else
  SystemSOCPLLClockSet(socpll_num, 0, CLOCK_MANAGER_SOCPLL_FREQ(socpll_num));
#endif

  // Re-enable clock on-demand by hardware.
  socpll->CTRL_CLR = SOCPLL_CTRL_DISONDEMAND;
  socpll->CTRL_CLR = SOCPLL_CTRL_FORCEEN;
}
#endif

/***************************************************************************//**
 * Initializes HFRCOEM23.
 ******************************************************************************/
FUNCTION_SCOPE void init_hfrcoem23(void)
{
  sl_status_t status;
  uint32_t frequency_calibration;

#if !defined(SL_CATALOG_CLOCK_MANAGER_PTE_PRESENT)
  // Retrieve HFRCOEM23 Calibration value from DEVINFO data
  CORE_ATOMIC_SECTION(
    // Since SE Command is necessary to get the calibration value, and sl_se_init() is not yet called,
    // we need to enter atomic mode to prevent the SE Command from being interrupted.
    frequency_calibration = sl_hal_system_get_hfrcoem23_calibration(SL_CLOCK_MANAGER_HFRCOEM23_BAND);
  )
  EFM_ASSERT((frequency_calibration != 0UL) && (frequency_calibration != UINT_MAX));
#else
  frequency_calibration = _HFRCO_CAL_RESETVALUE;
#endif

  status = sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_HFRCOEM23);
  EFM_ASSERT(status == SL_STATUS_OK);

  // Set divider for 1MHz and 2Mhz bands.
  frequency_calibration &= ~_HFRCO_CAL_CLKDIV_MASK;

  switch (SL_CLOCK_MANAGER_HFRCOEM23_BAND) {
    case 1000000UL:
      frequency_calibration |= HFRCO_CAL_CLKDIV_DIV4;
      break;

    case 2000000UL:
      frequency_calibration |= HFRCO_CAL_CLKDIV_DIV2;
      break;

    default:
      break;
  }

  // Activate new band selection
  HFRCOEM23->CAL = frequency_calibration;
}

#if defined(LFRCO_PRESENT)
/***************************************************************************//**
 * Initializes LFRCO.
 ******************************************************************************/
FUNCTION_SCOPE void init_lfrco(void)
{
  sl_status_t status;

  status = sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LFRCO);
  EFM_ASSERT(status == SL_STATUS_OK);

  // Disable LFRCO before accessing the CFG register.
  LFRCO->CTRL_SET = LFRCO_CTRL_DISONDEMAND;
  LFRCO->CTRL_CLR = LFRCO_CTRL_FORCEEN;
  while ((LFRCO->STATUS & LFRCO_STATUS_ENS) != 0U) {
    // Wait for LFRCO to be disabled.
  }

#if defined(SL_CLOCK_MANAGER_LFRCO_PRECISION) && (SL_CLOCK_MANAGER_LFRCO_PRECISION == 1)
  EFM_ASSERT(SL_CLOCK_MANAGER_HFXO_EN == 1);

  uint32_t ref = 0;
  uint32_t nom_cal = 0;
  uint32_t nom_cal_inv = 0;

  ref = SystemHFXOClockGet();
  if (ref == 38400000UL) {
    // High Precision mode calibration Counts for 38.4MHz
    nom_cal = _LFRCO_NOMCAL_RESETVALUE;
    nom_cal_inv = _LFRCO_NOMCALINV_RESETVALUE;
  } else if (ref == 39000000UL) {
    // High Precision mode calibration Counts for 39.0MHz.
    nom_cal = 0x0005CFBBUL;
    nom_cal_inv = 0x0000581AUL;
  } else {
    // Compute calibration count, based on HFXO frequency
    nom_cal = (5 * ref) >> 9;
    nom_cal_inv = ((1UL << 31) / 5) << 2;
    nom_cal_inv /= (ref >> 9);
  }

  LFRCO->NOMCAL = nom_cal;
  LFRCO->NOMCALINV = nom_cal_inv;
  LFRCO->CFG_SET = LFRCO_CFG_HIGHPRECEN;
#else
  LFRCO->CFG_CLR = LFRCO_CFG_HIGHPRECEN;
#endif

  // Clear disable on-demand.
  LFRCO->CTRL_CLR = LFRCO_CTRL_DISONDEMAND;
}
#endif

/***************************************************************************//**
 * Initializes Clock branches.
 ******************************************************************************/
FUNCTION_SCOPE void init_clock_branches(void)
{
  // Initialize SYSCLK clock branch.
#if defined(SL_CLOCK_MANAGER_SYSCLK_SOURCE)
  // TODO validate clock frequency range and prescaler
#if (SL_CLOCK_MANAGER_SYSCLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(SYSCLK, CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(SYSCLK, SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_CONCATENATION));
#elif defined(SLI_CLOCK_MANAGER_SYSCLK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(SYSCLK, SLI_CLOCK_MANAGER_SYSCLK_SOURCE);
#else
  CLOCK_MANAGER_CLOCK_SELECT_SET(SYSCLK, SL_CLOCK_MANAGER_SYSCLK_SOURCE);
#endif
  CMU->SYSCLKCTRL = (CMU->SYSCLKCTRL & ~(_CMU_SYSCLKCTRL_HCLKPRESC_MASK | _CMU_SYSCLKCTRL_PCLKPRESC_MASK))
                    | SL_CLOCK_MANAGER_HCLK_DIVIDER
                    | SL_CLOCK_MANAGER_PCLK_DIVIDER;

  SystemHCLKGet();     // Update SystemCoreClock
#else
  EFM_ASSERT(false);
#endif

  // Initialize TRACECLK clock branch.
#if defined(CoreDebug_DEMCR_TRCENA_Msk)
  // Disable the Core Debug module if already enabled.
  bool trace_on = CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk;
  if (trace_on) {
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
  }
#endif
#if defined(SL_CLOCK_MANAGER_TRACECLK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(TRACECLK, SL_CLOCK_MANAGER_TRACECLK_SOURCE);
#endif
#if defined(SL_CLOCK_MANAGER_TRACECLK_DIVIDER)
  CMU->TRACECLKCTRL |= SL_CLOCK_MANAGER_TRACECLK_DIVIDER;
#endif
#if defined(CoreDebug_DEMCR_TRCENA_Msk)
  // Enable back the Core Debug module if it was already enabled.
  if (trace_on) {
    __ISB();
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  }
#endif
  // Ensure TraceClk configs are defined if TraceClk is present.
#if defined(_CMU_TRACECLKCTRL_MASK) && !(defined(SL_CLOCK_MANAGER_TRACECLK_SOURCE) || defined(SL_CLOCK_MANAGER_TRACECLK_DIVIDER))
  EFM_ASSERT(false);
#endif

  // Initialize EM01GRPACLK clock branch.
#if defined(SL_CLOCK_MANAGER_EM01GRPACLK_SOURCE)
#if (SL_CLOCK_MANAGER_EM01GRPACLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(EM01GRPACLK, CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(EM01GRPACLK, SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_CONCATENATION));
#else
  CLOCK_MANAGER_CLOCK_SELECT_SET(EM01GRPACLK, SL_CLOCK_MANAGER_EM01GRPACLK_SOURCE);
#endif
#else
  EFM_ASSERT(false);
#endif

  // Initialize EM01GRPCCLK clock branch.
#if defined(_CMU_EM01GRPCCLKCTRL_MASK)
#if defined(SL_CLOCK_MANAGER_EM01GRPCCLK_SOURCE)
#if (SL_CLOCK_MANAGER_EM01GRPCCLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(EM01GRPCCLK, CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(EM01GRPCCLK, SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_CONCATENATION));
#else
  CLOCK_MANAGER_CLOCK_SELECT_SET(EM01GRPCCLK, SL_CLOCK_MANAGER_EM01GRPCCLK_SOURCE);
#endif
#else
  EFM_ASSERT(false);
#endif
#endif

  // Initialize EM01GRPDCLK clock branch.
#if defined(_CMU_EM01GRPDCLKCTRL_MASK)
#if defined(SL_CLOCK_MANAGER_EM01GRPDCLK_SOURCE)
#if (SL_CLOCK_MANAGER_EM01GRPDCLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(EM01GRPDCLK, CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(EM01GRPDCLK, SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_CONCATENATION));
#else
  CLOCK_MANAGER_CLOCK_SELECT_SET(EM01GRPDCLK, SL_CLOCK_MANAGER_EM01GRPDCLK_SOURCE);
#endif
#else
  EFM_ASSERT(false);
#endif
#endif

  // Initialize EM23GRPACLK clock branch.
#if defined(SL_CLOCK_MANAGER_EM23GRPACLK_SOURCE)
#if (SL_CLOCK_MANAGER_EM23GRPACLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(EM23GRPACLK, CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(EM23GRPACLK, SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_CONCATENATION));
#else
  CLOCK_MANAGER_CLOCK_SELECT_SET(EM23GRPACLK, SL_CLOCK_MANAGER_EM23GRPACLK_SOURCE);
#endif
#else
  EFM_ASSERT(false);
#endif

  // Initialize EM4GRPACLK clock branch.
#if defined(SL_CLOCK_MANAGER_EM4GRPACLK_SOURCE)
#if (SL_CLOCK_MANAGER_EM4GRPACLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(EM4GRPACLK, CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(EM4GRPACLK, SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_CONCATENATION));
#else
  CLOCK_MANAGER_CLOCK_SELECT_SET(EM4GRPACLK, SL_CLOCK_MANAGER_EM4GRPACLK_SOURCE);
#endif
#else
  EFM_ASSERT(false);
#endif

  // Initialize ADCCLK clock branch.
#if defined(ADC_PRESENT)
#if defined(SL_CLOCK_MANAGER_ADCCLK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(ADCCLK, SL_CLOCK_MANAGER_ADCCLK_SOURCE);
#else
  EFM_ASSERT(false);
#endif
#endif

  // Initialize PIXELRZCLK clock branch.
#if defined(PIXELRZ_PRESENT)
#if defined(SL_CLOCK_MANAGER_PIXELRZCLK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(PIXELRZCLK, SL_CLOCK_MANAGER_PIXELRZCLK_SOURCE);
  CMU->PIXELRZCLKCTRL = (CMU->PIXELRZCLKCTRL & ~_CMU_PIXELRZCLKCTRL_PRESC_MASK)
                        | SL_CLOCK_MANAGER_PIXELRZCLK_DIVIDER;
#else
  EFM_ASSERT(false);
#endif
#endif

  // Initialize SYSRTC clock branch.
#if defined(SYSRTC_PRESENT)
#if defined(SL_CLOCK_MANAGER_SYSRTCCLK_SOURCE)
#if (SL_CLOCK_MANAGER_SYSRTCCLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(SYSRTC0CLK, CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(SYSRTC0CLK, SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_CONCATENATION));
#else
  CLOCK_MANAGER_CLOCK_SELECT_SET(SYSRTC0CLK, SL_CLOCK_MANAGER_SYSRTCCLK_SOURCE);
#endif
#else
  EFM_ASSERT(false);
#endif
#endif

  // Initialize WDOG0 clock branch.
#if defined(SL_CLOCK_MANAGER_WDOG0CLK_SOURCE)
#if (SL_CLOCK_MANAGER_WDOG0CLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(WDOG0CLK, CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(WDOG0CLK, SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_CONCATENATION));
#else
  CLOCK_MANAGER_CLOCK_SELECT_SET(WDOG0CLK, SL_CLOCK_MANAGER_WDOG0CLK_SOURCE);
#endif
#else
  EFM_ASSERT(false);
#endif

  // Initialize PCNT0 clock branch.
#if defined(PCNT_PRESENT)
#if defined(SL_CLOCK_MANAGER_PCNT0CLK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(PCNT0CLK, SL_CLOCK_MANAGER_PCNT0CLK_SOURCE);
#else
  EFM_ASSERT(false);
#endif
#endif

  // Initialize EUSART0
#if defined(EUSART_PRESENT)
#if defined(SL_CLOCK_MANAGER_EUSART0CLK_SOURCE)
#if (SL_CLOCK_MANAGER_EUSART0CLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_EUSART0_LF_CLOCK_SOURCE)
#if (SL_CLOCK_MANAGER_DEFAULT_EUSART0_LF_CLOCK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE)
  // Default EUSART0 "SL_CLOCK_MANAGER_EUSART0CLK_SOURCE=SL_CLOCK_MANAGER_DEFAULT_EUSART0_LF_CLOCK_SOURCE"
  CLOCK_MANAGER_CLOCK_SELECT_SET(EUSART0CLK, CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(EUSART0CLK, SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_CONCATENATION));

#endif
#else
  // EUSART clock source explicitly defined i.e. SL_CLOCK_MANAGER_EUSART0CLK_SOURCE = CMU_EUSART0CLKCTRL_CLKSEL_LFRCO
  CLOCK_MANAGER_CLOCK_SELECT_SET(EUSART0CLK, SL_CLOCK_MANAGER_EUSART0CLK_SOURCE);
#endif
#else
  EFM_ASSERT(false);
#endif
#endif

  // Initialize I2C0
#if defined(_CMU_I2C0CLKCTRL_MASK)
#if defined(SL_CLOCK_MANAGER_I2C0CLK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(I2C0CLK, SL_CLOCK_MANAGER_I2C0CLK_SOURCE);
#else
  EFM_ASSERT(false);
#endif
#endif

#if defined(SL_CLOCK_MANAGER_SYSTICKCLK_SOURCE)
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_SYSCFG);
#if (SL_CLOCK_MANAGER_SYSTICKCLK_SOURCE == 1)
  sl_hal_syscfg_set_systicextclken_cfgsystic();
  SysTick->CTRL = (SysTick->CTRL & ~SysTick_CTRL_CLKSOURCE_Msk);
#elif (SL_CLOCK_MANAGER_SYSTICKCLK_SOURCE == 0)
  sl_hal_syscfg_clear_systicextclken_cfgsystic();
  SysTick->CTRL = (SysTick->CTRL | SysTick_CTRL_CLKSOURCE_Msk);
#else
  EFM_ASSERT(false);
#endif
#else
  EFM_ASSERT(false);
#endif

#if defined(CLOCK_MANAGER_INIT_HAL_INTERNAL_PRESENT)
  sli_clock_manager_hal_init_clock_branches_internal();
#endif
}

#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301) \
  && !defined(SL_CATALOG_CLOCK_MANAGER_PTE_PRESENT)
/***************************************************************************//**
 * Initializes SE command context, and determines device SE firmware
 * version.
 ******************************************************************************/
static void init_se(sl_se_command_context_t *context, uint32_t  *se_fw_version)
{
  EFM_ASSERT((context != NULL) && (se_fw_version != NULL));

  sl_status_t status;

  // Initialize command context.
  status = sl_se_init_command_context(context);
  EFM_ASSERT(status == SL_STATUS_OK);

  // Fetch Device SE FW version.
  status = sl_se_get_se_version(context, se_fw_version);
  EFM_ASSERT(status == SL_STATUS_OK);
}
#endif

#if defined(SL_CLOCK_MANAGER_HFXO_EN) \
  && (SL_CLOCK_MANAGER_HFXO_EN == 1)

#if defined(SL_CLOCK_MANAGER_QSPICLK_ADVANCED_CONFIG_EN) \
  && (SL_CLOCK_MANAGER_QSPICLK_ADVANCED_CONFIG_EN == 0)
#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301) \
  && !defined(SL_CATALOG_CLOCK_MANAGER_PTE_PRESENT)
/***************************************************************************//**
 * Scales QSPI frequency to prevent interferences with the 2.4GHz radio
 * band.
 ******************************************************************************/
QSPI_FUNCTION_SCOPE void qspi_frequency_scale(uint32_t *qspi_frequency)
{
  EFM_ASSERT(qspi_frequency != NULL);

  uint32_t scaled_qspi_frequency;
  // Lower and upper limits of 2.4GHz radio band.
  uint32_t min_band = 2400000000;
  uint32_t max_band = 2483500000;

  // Calculate minimum and maximum harmonics.
  uint32_t min_harmonic = min_band / *qspi_frequency;
  uint32_t max_harmonic = min_harmonic + 1;

  uint32_t factored_max = *qspi_frequency * max_harmonic;

  // Adjust harmonics if necessary.
  if ((factored_max > min_band) && (factored_max < max_band)) {
    min_harmonic += 1;
    max_harmonic += 1;
  }

  // Calculate minimum and maximum boundaries.
  uint32_t max_boundary = min_band / min_harmonic;
  uint32_t min_boundary = max_band / max_harmonic;

  // Calculate acceptable QSPI clock frequency.
  // Ensure boundaries are in the correct range.
  EFM_ASSERT(min_boundary < max_boundary);
  scaled_qspi_frequency = (max_boundary + min_boundary) / 2;

  // Ensure scaled frequency is less than original frequency.
  EFM_ASSERT(scaled_qspi_frequency <= *qspi_frequency);

  // Update QSPI frequency.
  *qspi_frequency = scaled_qspi_frequency;
}
#endif
#endif

#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301) \
  && !defined(SL_CATALOG_CLOCK_MANAGER_PTE_PRESENT)
/***************************************************************************//**
 * Configures the FLPLL to achieve a radio safe QSPI frequency with a HFXO
 * reference clock.
 ******************************************************************************/
static void configure_flpll(sli_se_qspi_flpll_config_t *flpll_config)
{
  if (clock_manager_se_version < 0x00202L) {
    return;
  }

  uint32_t qspi_frequency;
  uint64_t frac_div;

#if defined(SL_CLOCK_MANAGER_EXT_FLASH_MAX_FREQ)
  // Ensure that the max external flash specified is not 0.
  EFM_ASSERT(SL_CLOCK_MANAGER_EXT_FLASH_MAX_FREQ > 0);
#if defined(SL_CLOCK_MANAGER_QSPICLK_ADVANCED_CONFIG_EN) && (SL_CLOCK_MANAGER_QSPICLK_ADVANCED_CONFIG_EN == 1)
  // Ensure the custom QSPI frequency is less than the external flash max frequency.
  EFM_ASSERT((SL_CLOCK_MANAGER_QSPICLK_CUSTOM_FREQ < SL_CLOCK_MANAGER_EXT_FLASH_MAX_FREQ) && (SL_CLOCK_MANAGER_QSPICLK_CUSTOM_FREQ != 0));
  qspi_frequency = SL_CLOCK_MANAGER_QSPICLK_CUSTOM_FREQ;
#else
  qspi_frequency = SL_CLOCK_MANAGER_EXT_FLASH_MAX_FREQ;
#endif
#else
  // Retrieve max frequency for co-packaged flash.
#if defined(PART_NUMBER)
  static const char *part_number = PART_NUMBER;
  if (part_number[12] == 'I') {
    // Max temp grade is 125C corresponding to max flash of 104 MHz.
    qspi_frequency = 104000000;
  } else if (part_number[12] == 'G') {
    // Max temp grade is 85C corresponding to max flash of 133 MHz.
    qspi_frequency = 133000000;
  } else {
    EFM_ASSERT(false);
  }
#if defined(SL_CLOCK_MANAGER_QSPICLK_ADVANCED_CONFIG_EN) && (SL_CLOCK_MANAGER_QSPICLK_ADVANCED_CONFIG_EN == 1)
  // Ensure the custom QSPI frequency is less than the co-packaged flash max frequency.
  EFM_ASSERT((SL_CLOCK_MANAGER_QSPICLK_CUSTOM_FREQ < qspi_frequency) && (SL_CLOCK_MANAGER_QSPICLK_CUSTOM_FREQ != 0));
  qspi_frequency = SL_CLOCK_MANAGER_QSPICLK_CUSTOM_FREQ;
#endif
#else
  #error "No Part Number is defined in your project."
#endif
#endif

// Do not scale custom flash frequencies.
#if defined(SL_CLOCK_MANAGER_QSPICLK_ADVANCED_CONFIG_EN) && (SL_CLOCK_MANAGER_QSPICLK_ADVANCED_CONFIG_EN == 0)
  // Scale max frequency to prevent radio interference.
  qspi_frequency_scale(&qspi_frequency);
#endif

  // Determine the prescaler for the final FLPLL divider. Assuming the PLLDIV2EN configuration is set.
  sli_se_qspi_clkperpresc_t clk_per_presc = SLI_SE_QSPI_CLKPERPRESC_DIV1;
  uint32_t int_div = (qspi_frequency * (2 * ((uint32_t)clk_per_presc + 1))) / SLI_CLOCK_MANAGER_HFXO_FREQ;

  if (int_div < 6) {
    // The integer divider can't be under 6, so if the resulting divider is below that, the second prescaler needs to be increased.
    clk_per_presc = SLI_SE_QSPI_CLKPERPRESC_DIV2;
    int_div = (qspi_frequency * (2 * ((uint32_t)clk_per_presc + 1))) / SLI_CLOCK_MANAGER_HFXO_FREQ;
  }

  // Calculate new dividers to achieve the scaled QSPI frequency, and update global config.
  flpll_config->ref_clock = SLI_SE_QSPI_REF_CLOCK_HFXO;
  flpll_config->int_div = (uint8_t)int_div;
  frac_div = (qspi_frequency * (2 * ((uint32_t)clk_per_presc + 1)))  % SLI_CLOCK_MANAGER_HFXO_FREQ;
  frac_div = (2048 * frac_div) / SLI_CLOCK_MANAGER_HFXO_FREQ;
  flpll_config->frac_div = (uint16_t)frac_div;
  flpll_config->clk_per_presc = clk_per_presc;

  // Determine the target range of SCLK frequency to ensure proper TRIM bits.
  uint32_t sclk_freq = qspi_frequency * ((uint32_t)clk_per_presc + 1);
  if (sclk_freq < 140000000) {
    flpll_config->pll_clk_freq_sel = SLI_SE_QSPI_PLLCLKFREQSEL_SCLK120MHZ;
  } else if (sclk_freq < 160000000) {
    flpll_config->pll_clk_freq_sel = SLI_SE_QSPI_PLLCLKFREQSEL_SCLK140MHZ;
  } else if (sclk_freq < 180000000) {
    flpll_config->pll_clk_freq_sel = SLI_SE_QSPI_PLLCLKFREQSEL_SCLK160MHZ;
  } else if (sclk_freq < 200000000) {
    flpll_config->pll_clk_freq_sel = SLI_SE_QSPI_PLLCLKFREQSEL_SCLK180MHZ;
  } else if (sclk_freq < 220000000) {
    flpll_config->pll_clk_freq_sel = SLI_SE_QSPI_PLLCLKFREQSEL_SCLK200MHZ;
  } else {
    flpll_config->pll_clk_freq_sel = SLI_SE_QSPI_PLLCLKFREQSEL_SCLK220MHZ;
  }
}
#endif
#endif

/*******************************************************************************
 **********************   GLOBAL INTERNAL FUNCTIONS   **************************
 ******************************************************************************/

/***************************************************************************//**
 * Initializes Oscillators and Clock branches.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_init(void)
{
  sl_status_t status;

  // Make sure SYSCLK is on FSRCO
  CLOCK_MANAGER_CLOCK_SELECT_SET(SYSCLK, CMU_SYSCLKCTRL_CLKSEL_FSRCO);

#if defined(SYSRTC_PRESENT)
  status =  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_SYSRTC0);
  EFM_ASSERT(status == SL_STATUS_OK);
#endif

  // Initialize Oscillators
#if defined(SL_CLOCK_MANAGER_LFXO_EN) && (SL_CLOCK_MANAGER_LFXO_EN == 1)
  init_lfxo();
#endif

#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301) \
  && !defined(SL_CATALOG_CLOCK_MANAGER_PTE_PRESENT)
  init_se(&clock_manager_cmd_ctx, &clock_manager_se_version);

  sli_clock_manager_hal_set_ext_flash_clk(SL_OSCILLATOR_FSRCO);
#endif

#if defined(SL_CLOCK_MANAGER_HFXO_EN) && (SL_CLOCK_MANAGER_HFXO_EN == 1)

  init_hfxo();

#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301) \
  && !defined(SL_CATALOG_CLOCK_MANAGER_PTE_PRESENT)
  configure_flpll(&clock_manager_flpll_cfg);

  sli_clock_manager_hal_set_ext_flash_clk(SL_OSCILLATOR_FLPLL);
#endif
#endif

  init_clkin0();

  init_hfrcodpll();

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)                            \
  || (defined(SL_CLOCK_MANAGER_SOCPLL_EN) && (SL_CLOCK_MANAGER_SOCPLL_EN == 1)) \
  || (defined(SL_CLOCK_MANAGER_SOCPLL0_EN) && (SL_CLOCK_MANAGER_SOCPLL0_EN == 1))
  init_socpll(0);
#endif

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION) && defined(SOCPLL1) \
  || (defined(SL_CLOCK_MANAGER_SOCPLL1_EN) && (SL_CLOCK_MANAGER_SOCPLL1_EN == 1))
  init_socpll(1);
#endif

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION) && defined(SOCPLL2) \
  || (defined(SL_CLOCK_MANAGER_SOCPLL2_EN) && (SL_CLOCK_MANAGER_SOCPLL2_EN == 1))
  init_socpll(2);
#endif

  init_hfrcoem23();

  init_lfrco();

#if defined(CLOCK_MANAGER_INIT_HAL_INTERNAL_PRESENT)
  sli_clock_manager_hal_init_oscillators_internal();
#endif

  // Initialize Clock branches
  init_clock_branches();

  return SL_STATUS_OK;
}
#endif // FPGA
