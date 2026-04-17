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

#include "sl_clock_manager_oscillator_config.h"
#include "sl_clock_manager_tree_config.h"
#include "sli_clock_manager_init_hal.h"
#include "sl_clock_manager.h"
#include "sl_status.h"
#include "sl_assert.h"
#include "em_device.h"
#include "em_cmu.h"
#include "sl_gpio.h"

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
#include "sli_clock_manager_runtime_configuration.h"
#endif

#if defined(RFFPLL_PRESENT)
#if defined(SL_CLOCK_MANAGER_RFFPLL_BAND) && (SL_CLOCK_MANAGER_RFFPLL_BAND == 7)
#ifdef SL_CLOCK_MANAGER_AUTO_BAND_VALID
#include "rail_config.h"
#endif
#endif
#endif

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Fetch HFXO CTUNE value from USERDATA page as a manufacturing token
#define MFG_CTUNE_HFXO_ADDR 0x0FE00100UL
#define MFG_CTUNE_HFXO_VAL  (*((uint16_t *) (MFG_CTUNE_HFXO_ADDR)))

// Fetch LFXO CTUNE value from USERDATA page as a manufacturing token
#define MFG_CTUNE_LFXO_ADDR 0x0FE0009CUL
#define MFG_CTUNE_LFXO_VAL  (*((uint8_t *) (MFG_CTUNE_LFXO_ADDR)))

#if defined(RFFPLL_PRESENT)
// If RADIO_CONFIG_RFFPLL_CONFIG_PRESENT is not defined, either there is no
// radio configuration in the project or the radio configuration that is
// present does not include an RFFPLL configuration.
// In either case, the Auto-Band feature cannot work, so fall back to the
// default band (Band 9xx).
#if defined(SL_CLOCK_MANAGER_RFFPLL_BAND) && (SL_CLOCK_MANAGER_RFFPLL_BAND == 7)
#ifndef RADIO_CONFIG_RFFPLL_CONFIG_PRESENT
#undef SL_CLOCK_MANAGER_RFFPLL_BAND
#define SL_CLOCK_MANAGER_RFFPLL_BAND  6
#endif
#endif
#endif

// Allocated PRS channel for crystal sharing
#if defined(_HFXO_CTRL_PRSSTATUSSEL0_MASK)
#define HFXO_CRYSTSAL_SHARING_PRS_CHANNEL 0
#endif

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
#define FUNCTION_SCOPE
#else
#define FUNCTION_SCOPE static
#endif

/*******************************************************************************
 ******************************   TYPEDEFS   ***********************************
 ******************************************************************************/

#if defined(RFFPLL_PRESENT)
typedef struct {
  uint32_t frequency;  // Host target frequency.
  uint8_t divider_y;   // Divider Y for digital.
  uint8_t divider_x;   // Divider X for Radio.
  uint8_t divider_n;   // Feedback divider N.
} clock_manager_rffpll_config_t;
#endif

/*******************************************************************************
 **************************   LOCAL VARIABLES   ********************************
 ******************************************************************************/

#if defined(RFFPLL_PRESENT)                                               \
  && defined(SL_CLOCK_MANAGER_HFXO_EN) && (SL_CLOCK_MANAGER_HFXO_EN == 1) \
  && (SL_CLOCK_MANAGER_RFFPLL_CUSTOM_BAND == 0)                           \
  && !((SL_CLOCK_MANAGER_RFFPLL_BAND == 7) && defined(RADIO_CONFIG_RFFPLL_CONFIG_PRESENT))
// Table of possible radio frequency bands and their associated settings.
static clock_manager_rffpll_config_t rffpll_band_config_39MHz[] = {
  { 97500000, 23, 7, 115 }, // Band 450 MHz
  { 98380000, 22, 7, 111 }, // Band 470 MHz
  { 97500000, 23, 7, 115 }, // Band 780 MHz
  { 97500000, 20, 6, 100 }, // Band 863 MHz
  { 97500000, 23, 7, 115 }, // Band 896 MHz
  { 96520000, 20, 6, 99 },  // Band 928 MHz
  { 97500000, 20, 6, 100 }  // Band 9xx MHz (covers from 901 to 928 MHz)
};
#endif

/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
uint32_t SLI_CLOCK_MANAGER_HFXO_MODE = SL_CLOCK_MANAGER_HFXO_MODE;
uint32_t SLI_CLOCK_MANAGER_HFXO_FREQ = SL_CLOCK_MANAGER_HFXO_FREQ;
uint32_t SLI_CLOCK_MANAGER_HFXO_CTUNE_FIXED_STEADY = SLI_CLOCK_MANAGER_HFXO_CTUNE_FIXED_STEADY_DEFAULT;

uint32_t SLI_CLOCK_MANAGER_HFRCO_BAND     = SL_CLOCK_MANAGER_HFRCO_BAND;
bool     SLI_CLOCK_MANAGER_HFRCO_DPLL_EN  = SL_CLOCK_MANAGER_HFRCO_DPLL_EN;
uint32_t SLI_CLOCK_MANAGER_DPLL_FREQ      = SL_CLOCK_MANAGER_DPLL_FREQ;
uint32_t SLI_CLOCK_MANAGER_DPLL_N         = SL_CLOCK_MANAGER_DPLL_N;
uint32_t SLI_CLOCK_MANAGER_DPLL_M         = SL_CLOCK_MANAGER_DPLL_M;
#endif

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

#if defined(HFXO_PRESENT) \
  && defined(SL_CLOCK_MANAGER_HFXO_EN) && SL_CLOCK_MANAGER_HFXO_EN == 1
/***************************************************************************//**
 * Initializes HFXO Oscillator.
 ******************************************************************************/
FUNCTION_SCOPE void init_hfxo(void)
{
  CMU_HFXOInit_TypeDef clock_manager_hfxo_init = CMU_HFXOINIT_DEFAULT;
  clock_manager_hfxo_init.mode = SLI_CLOCK_MANAGER_HFXO_MODE >> _HFXO_CFG_MODE_SHIFT;

  if (SLI_CLOCK_MANAGER_HFXO_MODE == cmuHfxoOscMode_ExternalSine) {
    clock_manager_hfxo_init = (CMU_HFXOInit_TypeDef)CMU_HFXOINIT_EXTERNAL_SINE;
#if defined(_HFXO_CFG_MODE_EXTCLKPKDET)
  } else if (SLI_CLOCK_MANAGER_HFXO_MODE == cmuHfxoOscMode_ExternalSinePkDet) {
    clock_manager_hfxo_init = (CMU_HFXOInit_TypeDef)CMU_HFXOINIT_EXTERNAL_SINEPKDET;
#endif
  }
#if defined (SL_CLOCK_MANAGER_HFXO_ENXIDCBIASANA)
  clock_manager_hfxo_init.enXiDcBiasAna = SL_CLOCK_MANAGER_HFXO_ENXIDCBIASANA;
#endif

  int ctune = -1;

#if defined(SL_CLOCK_MANAGER_HFXO_SEPARATE_CTUNEXIANA_CTUNEXOANA_EN) && (SL_CLOCK_MANAGER_HFXO_SEPARATE_CTUNEXIANA_CTUNEXOANA_EN == 1)
  ctune = SL_CLOCK_MANAGER_HFXO_CTUNEXIANA;
  CMU_HFXOCTuneDeltaSet(SL_CLOCK_MANAGER_HFXO_CTUNEXOANA - SL_CLOCK_MANAGER_HFXO_CTUNEXIANA);
#endif

#if defined(_DEVINFO_MODXOCAL_HFXOCTUNEXIANA_MASK)
  // Use HFXO tuning value from DEVINFO if available (PCB modules)
  if ((ctune == -1) && ((DEVINFO->MODULEINFO & _DEVINFO_MODULEINFO_HFXOCALVAL_MASK) == 0)) {
    ctune = DEVINFO->MODXOCAL & _DEVINFO_MODXOCAL_HFXOCTUNEXIANA_MASK;
  }
#endif

  // Use HFXO tuning value from MFG token in UD page if not already set
  if ((ctune == -1)
#if defined(SL_CLOCK_MANAGER_CTUNE_MFG_HFXO_EN)
      && (SL_CLOCK_MANAGER_CTUNE_MFG_HFXO_EN == 1)
#endif
      && (MFG_CTUNE_HFXO_VAL <= (_HFXO_XTALCTRL_CTUNEXIANA_MASK >> _HFXO_XTALCTRL_CTUNEXIANA_SHIFT))) {
    ctune = MFG_CTUNE_HFXO_VAL;
  }

  // Use HFXO tuning value from configurations as fallback
  if (ctune == -1) {
    ctune = SL_CLOCK_MANAGER_HFXO_CTUNE;
  }

  // Configure CTUNE XI and XO.
  if (ctune != -1) {
    clock_manager_hfxo_init.ctuneXiAna = (uint8_t)ctune;

    // Ensure CTUNE XO plus a delta is within the correct range. The delta accounts for internal chip
    // load imbalance on some series 2 chips.
    ctune += CMU_HFXOCTuneDeltaGet();
    if (ctune < 0) {
      ctune = 0;
    } else if (ctune > ((int)(_HFXO_XTALCTRL_CTUNEXOANA_MASK >> _HFXO_XTALCTRL_CTUNEXOANA_SHIFT))) {
      ctune = (int)(_HFXO_XTALCTRL_CTUNEXOANA_MASK >> _HFXO_XTALCTRL_CTUNEXOANA_SHIFT);
    }
    clock_manager_hfxo_init.ctuneXoAna = ctune;
  }

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
  if (SLI_CLOCK_MANAGER_HFXO_CTUNE_FIXED_STEADY != SLI_CLOCK_MANAGER_HFXO_CTUNE_FIXED_STEADY_DEFAULT) {
    clock_manager_hfxo_init.ctuneFixAna = SLI_CLOCK_MANAGER_HFXO_CTUNE_FIXED_STEADY;
  }
#endif

#if defined(SL_CLOCK_MANAGER_HFXO_CRYSTAL_SHARING_EN) && (SL_CLOCK_MANAGER_HFXO_CRYSTAL_SHARING_EN == 1)
  // Set port and pin.
  sl_status_t status;
  GPIO_Port_TypeDef port = SL_CLOCK_MANAGER_HFXO_CRYSTAL_SHARING_GPIO_PORT;
  unsigned int pin = SL_CLOCK_MANAGER_HFXO_CRYSTAL_SHARING_GPIO_PIN;

  // Enable HFXO and GPIO bus clocks.
  status = sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_HFXO0);
  EFM_ASSERT(status == SL_STATUS_OK);
  status = sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
  EFM_ASSERT(status == SL_STATUS_OK);
#if defined(SL_CLOCK_MANAGER_HFXO_CRYSTAL_SHARING_FOLLOWER_EN) && (SL_CLOCK_MANAGER_HFXO_CRYSTAL_SHARING_FOLLOWER_EN == 1)
  // Initialize Crystal sharing follower.
  status = sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);
  EFM_ASSERT(status == SL_STATUS_OK);

  EFM_ASSERT(GPIO_PORT_PIN_VALID(port, pin));

  uint32_t mask = 0U, value = 0U;

  bool was_locked = HFXO0->STATUS & HFXO_STATUS_LOCK_LOCKED ? true : false;

  // Unlock register interface.
  HFXO0->LOCK = HFXO_LOCK_LOCKKEY_UNLOCK;
  HFXO0->IEN_SET = HFXO_IEN_BUFOUTRDY;

  BUS_RegMaskedWrite(&HFXO0->CTRL, _HFXO_CTRL_PRSSTATUSSEL0_MASK, (_HFXO_CTRL_PRSSTATUSSEL0_ENS << _HFXO_CTRL_PRSSTATUSSEL0_SHIFT));

  if (was_locked) {
    HFXO0->LOCK = ~HFXO_LOCK_LOCKKEY_UNLOCK;
  }

  value = _PRS_ASYNC_CH_CTRL_AUXSEL_DEFAULT     << _PRS_ASYNC_CH_CTRL_AUXSEL_SHIFT
          | _PRS_ASYNC_CH_CTRL_FNSEL_A          << _PRS_ASYNC_CH_CTRL_FNSEL_SHIFT
          | _PRS_ASYNC_CH_CTRL_SOURCESEL_HFXO0L << _PRS_ASYNC_CH_CTRL_SOURCESEL_SHIFT
          | _PRS_ASYNC_CH_CTRL_SIGSEL_HFXO0LSTATUS << _PRS_ASYNC_CH_CTRL_SIGSEL_SHIFT;

  mask = _PRS_ASYNC_CH_CTRL_AUXSEL_MASK
         | _PRS_ASYNC_CH_CTRL_FNSEL_MASK
         | _PRS_ASYNC_CH_CTRL_SOURCESEL_MASK
         | _PRS_ASYNC_CH_CTRL_SIGSEL_MASK;

  BUS_RegMaskedWrite(&(PRS->ASYNC_CH[HFXO_CRYSTSAL_SHARING_PRS_CHANNEL].CTRL), mask, value);

  GPIO_PinModeSet(port, pin, gpioModeWiredOrPullDown, 0U);

  (&(GPIO->PRSROUTE[0].ASYNCH0ROUTE))[HFXO_CRYSTSAL_SHARING_PRS_CHANNEL] = pin << _GPIO_PRS_ASYNCH0ROUTE_PIN_SHIFT
                                                                           | port << _GPIO_PRS_ASYNCH0ROUTE_PORT_SHIFT;
  GPIO->PRSROUTE[0].ROUTEEN = 1U << (_GPIO_PRS_ROUTEEN_ASYNCH0PEN_SHIFT + HFXO_CRYSTSAL_SHARING_PRS_CHANNEL);

  //sl_clock_manager_disable_bus_clock(SL_BUS_CLOCK_PRS);
#elif defined(SL_CLOCK_MANAGER_HFXO_CRYSTAL_SHARING_LEADER_EN) && (SL_CLOCK_MANAGER_HFXO_CRYSTAL_SHARING_LEADER_EN == 1)
  EFM_ASSERT(GPIO_PORT_PIN_VALID(port, pin));

  // Configure Bufout request GPIO.
  GPIO_PinModeSet(port, pin, gpioModeInput, 0U);

  GPIO->SYXOROUTE[0].BUFOUTREQINASYNCROUTE = pin << _GPIO_SYXO_BUFOUTREQINASYNCROUTE_PIN_SHIFT
                                             | port << _GPIO_SYXO_BUFOUTREQINASYNCROUTE_PORT_SHIFT;

  bool was_locked = HFXO0->STATUS & HFXO_STATUS_LOCK_LOCKED ? true : false;

  // Unlock register interface.
  HFXO0->LOCK = HFXO_LOCK_LOCKKEY_UNLOCK;

  HFXO0->CTRL_CLR = _HFXO_CTRL_BUFOUTFREEZE_MASK | _HFXO_CTRL_DISONDEMANDBUFOUT_MASK;

  BUS_RegMaskedWrite(&HFXO0->BUFOUTCTRL,
                     _HFXO_BUFOUTCTRL_MINIMUMSTARTUPDELAY_MASK
                     | _HFXO_BUFOUTCTRL_TIMEOUTSTARTUP_MASK,
                     ((uint32_t)SL_CLOCK_MANAGER_HFXO_CRYSTAL_SHARING_LEADER_MIN_STARTUP_DELAY_EN)  << _HFXO_BUFOUTCTRL_MINIMUMSTARTUPDELAY_SHIFT
                     | SL_CLOCK_MANAGER_HFXO_CRYSTAL_SHARING_LEADER_TIMEOUT_STARTUP);

  if (was_locked) {
    HFXO0->LOCK = ~HFXO_LOCK_LOCKKEY_UNLOCK;
  }
#endif
#endif

  SystemHFXOClockSet(SLI_CLOCK_MANAGER_HFXO_FREQ);
  CMU_HFXOInit(&clock_manager_hfxo_init);
  CMU_HFXOPrecisionSet(SL_CLOCK_MANAGER_HFXO_PRECISION);

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
  if (SLI_CLOCK_MANAGER_HFXO_CTUNE_FIXED_STEADY == SLI_CLOCK_MANAGER_HFXO_CTUNE_FIXED_STEADY_DEFAULT) {
    SLI_CLOCK_MANAGER_HFXO_CTUNE_FIXED_STEADY = (HFXO0->XTALCTRL & _HFXO_XTALCTRL_CTUNEFIXANA_MASK) >> _HFXO_XTALCTRL_CTUNEFIXANA_SHIFT;
  }
#endif
}
#endif

#if defined(LFXO_PRESENT) \
  && defined(SL_CLOCK_MANAGER_LFXO_EN) && SL_CLOCK_MANAGER_LFXO_EN == 1
/***************************************************************************//**
 * Initializes LFXO Oscillator.
 ******************************************************************************/
FUNCTION_SCOPE void init_lfxo(void)
{
  CMU_LFXOInit_TypeDef clock_manager_lfxo_init = CMU_LFXOINIT_DEFAULT;
#ifdef SL_CLOCK_MANAGER_LFXO_GAIN
  clock_manager_lfxo_init.gain = SL_CLOCK_MANAGER_LFXO_GAIN;
#endif

  clock_manager_lfxo_init.mode = SL_CLOCK_MANAGER_LFXO_MODE >> _LFXO_CFG_MODE_SHIFT;
  clock_manager_lfxo_init.timeout = SL_CLOCK_MANAGER_LFXO_TIMEOUT >> _LFXO_CFG_TIMEOUT_SHIFT;
  clock_manager_lfxo_init.capTune = 0xFF;

#ifndef _SILICON_LABS_32B_SERIES_2_CONFIG_9
#if defined(_DEVINFO_MODXOCAL_LFXOCAPTUNE_MASK)
  // Use LFXO tuning value from DEVINFO if available (PCB modules)
  if ((DEVINFO->MODULEINFO & _DEVINFO_MODULEINFO_LFXOCALVAL_MASK) == _DEVINFO_MODULEINFO_LFXOCALVAL_VALID) {
    clock_manager_lfxo_init.capTune = DEVINFO->MODXOCAL & _DEVINFO_MODXOCAL_LFXOCAPTUNE_MASK;
  }
#endif
#endif

  if ((clock_manager_lfxo_init.capTune == 0xFF)
#if defined(SL_CLOCK_MANAGER_CTUNE_MFG_LFXO_EN)
      && (SL_CLOCK_MANAGER_CTUNE_MFG_LFXO_EN == 1)
#endif
      && (MFG_CTUNE_LFXO_VAL <= (_LFXO_CAL_CAPTUNE_MASK >> _LFXO_CAL_CAPTUNE_SHIFT))) {
    clock_manager_lfxo_init.capTune = MFG_CTUNE_LFXO_VAL;
  }

  if (clock_manager_lfxo_init.capTune == 0xFF) {
    clock_manager_lfxo_init.capTune = SL_CLOCK_MANAGER_LFXO_CTUNE;
  }

  CMU_LFXOInit(&clock_manager_lfxo_init);
  CMU_LFXOPrecisionSet(SL_CLOCK_MANAGER_LFXO_PRECISION);
}
#endif

/***************************************************************************//**
 * Initializes Clock Input CLKIN0.
 ******************************************************************************/
static void init_clkin0(void)
{
#if (defined(SL_CLOCK_MANAGER_SYSCLK_SOURCE) && (SL_CLOCK_MANAGER_SYSCLK_SOURCE == CMU_SYSCLKCTRL_CLKSEL_CLKIN0))  \
  || (defined(SL_CLOCK_MANAGER_DPLL_REFCLK) && (SL_CLOCK_MANAGER_DPLL_REFCLK == CMU_DPLLREFCLKCTRL_CLKSEL_CLKIN0)) \
  || (defined(SL_CLOCK_MANAGER_EM01GRPBCLK_SOURCE) && (SL_CLOCK_MANAGER_EM01GRPBCLK_SOURCE == CMU_EM01GRPBCLKCTRL_CLKSEL_CLKIN0))

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

#if defined(HFRCO_PRESENT)
/***************************************************************************//**
 * Initializes HFRCODPLL Oscillator.
 ******************************************************************************/
FUNCTION_SCOPE void init_hfrcodpll(void)
{
#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION) \
  || (defined(SL_CLOCK_MANAGER_HFRCO_DPLL_EN) && (SL_CLOCK_MANAGER_HFRCO_DPLL_EN == 1))

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
  if (SLI_CLOCK_MANAGER_HFRCO_DPLL_EN)
#endif
  {
    sl_status_t status;

    CMU_DPLLInit_TypeDef clock_manager_dpll_init = {
      .frequency = SLI_CLOCK_MANAGER_DPLL_FREQ,
      .n = SLI_CLOCK_MANAGER_DPLL_N,
      .m = SLI_CLOCK_MANAGER_DPLL_M,
      .edgeSel = SL_CLOCK_MANAGER_DPLL_EDGE,
      .lockMode = SL_CLOCK_MANAGER_DPLL_LOCKMODE,
      .autoRecover = SL_CLOCK_MANAGER_DPLL_AUTORECOVER,
      .ditherEn = SL_CLOCK_MANAGER_DPLL_DITHER
    };

    // Convert the DPLL Reference clock configuration to emlib CMU_Select_TypeDef type.
    switch (SL_CLOCK_MANAGER_DPLL_REFCLK) {
      case CMU_DPLLREFCLKCTRL_CLKSEL_HFXO:
        clock_manager_dpll_init.refClk = cmuSelect_HFXO;
        break;
      case CMU_DPLLREFCLKCTRL_CLKSEL_LFXO:
        clock_manager_dpll_init.refClk = cmuSelect_LFXO;
        break;
      case CMU_DPLLREFCLKCTRL_CLKSEL_CLKIN0:
        clock_manager_dpll_init.refClk = cmuSelect_CLKIN0;
        break;
      default:
        EFM_ASSERT(false);
        break;
    }

    CMU_Select_TypeDef selected_sysclk = CMU_ClockSelectGet(cmuClock_SYSCLK);

    if (selected_sysclk == cmuSelect_HFRCODPLL) {
      // The CMU should not be running from the HFRCO. If necessary, the CMU
      // should switch to the FSRCO until after the DPLL has locked to avoid
      // over-clocking due to overshoot.
      CMU->SYSCLKCTRL = (CMU->SYSCLKCTRL & ~_CMU_SYSCLKCTRL_CLKSEL_MASK) | CMU_SYSCLKCTRL_CLKSEL_FSRCO;
    }

    status = sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_DPLL0);
    EFM_ASSERT(status == SL_STATUS_OK);

    bool success = CMU_DPLLLock(&clock_manager_dpll_init);

    // If CMU was initially running from HFRCO switch back from FSRCO.
    if (selected_sysclk == cmuSelect_HFRCODPLL) {
      CMU->SYSCLKCTRL = (CMU->SYSCLKCTRL & ~_CMU_SYSCLKCTRL_CLKSEL_MASK) | CMU_SYSCLKCTRL_CLKSEL_HFRCODPLL;
    }

    // Assert if DPLL lock was unsuccessful.
    EFM_ASSERT(success == true);
  }
#endif
#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION) \
  || (defined(SL_CLOCK_MANAGER_HFRCO_DPLL_EN) && (SL_CLOCK_MANAGER_HFRCO_DPLL_EN == 0))

#if defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
  else
#else
  if (true)
#endif
  {
    CMU_HFRCODPLLBandSet(SLI_CLOCK_MANAGER_HFRCO_BAND);
  }
#endif
}
#endif

#if defined(HFRCOEM23_PRESENT)
/***************************************************************************//**
 * Initializes HFRCOEM23 Oscillator.
 ******************************************************************************/
FUNCTION_SCOPE void init_hfrcoem23(void)
{
  CMU_HFRCOEM23BandSet(SL_CLOCK_MANAGER_HFRCOEM23_BAND);
}
#endif

#if defined(LFRCO_PRESENT)
/***************************************************************************//**
 * Initializes LFRCO Oscillator.
 ******************************************************************************/
FUNCTION_SCOPE void init_lfrco(void)
{
  sl_status_t status;

  status = sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LFRCO);
  EFM_ASSERT(status == SL_STATUS_OK);

#if defined(PLFRCO_PRESENT)
#if !(defined(SL_CLOCK_MANAGER_HFXO_EN) && (SL_CLOCK_MANAGER_HFXO_EN == 1))
  EFM_ASSERT(SL_CLOCK_MANAGER_LFRCO_PRECISION != cmuPrecisionHigh);
#endif
  CMU_LFRCOSetPrecision(SL_CLOCK_MANAGER_LFRCO_PRECISION);
#endif
}
#endif

#if defined(RFFPLL_PRESENT) \
  && defined(SL_CLOCK_MANAGER_HFXO_EN) && (SL_CLOCK_MANAGER_HFXO_EN == 1)
/***************************************************************************//**
 * Initializes RFFPLL Oscillator.
 ******************************************************************************/
FUNCTION_SCOPE void init_rffpll(void)
{
  CMU_RFFPLL_Init_TypeDef rffpll_init = CMU_RFFPLL_DEFAULT;

  // Overwrite default configurations.
#if (SL_CLOCK_MANAGER_RFFPLL_CUSTOM_BAND == 1)
  // Custom settings provided by the user.
  rffpll_init.frequency = SL_CLOCK_MANAGER_RFFPLL_FREQ;
  rffpll_init.dividerY = SL_CLOCK_MANAGER_RFFPLL_DIV_Y;
  rffpll_init.dividerX = SL_CLOCK_MANAGER_RFFPLL_DIV_X;
  rffpll_init.dividerN = SL_CLOCK_MANAGER_RFFPLL_DIV_N;
#elif (SL_CLOCK_MANAGER_RFFPLL_BAND == 7) && defined(RADIO_CONFIG_RFFPLL_CONFIG_PRESENT)
  // Settings from the RAIL lib.
  rffpll_init.frequency = radioConfigRffpllConfig->sysclkFreqHz;
  rffpll_init.dividerY = ((radioConfigRffpllConfig->dividers & RAIL_RFFPLL_DIVIDERY_MASK) >> RAIL_RFFPLL_DIVIDERY_SHIFT);
  rffpll_init.dividerX = ((radioConfigRffpllConfig->dividers & RAIL_RFFPLL_DIVIDERX_MASK) >> RAIL_RFFPLL_DIVIDERX_SHIFT);
  rffpll_init.dividerN = ((radioConfigRffpllConfig->dividers & RAIL_RFFPLL_DIVIDERN_MASK) >> RAIL_RFFPLL_DIVIDERN_SHIFT);
#else
  // Pre-determined settings.
  rffpll_init.frequency = rffpll_band_config_39MHz[SL_CLOCK_MANAGER_RFFPLL_BAND].frequency;
  rffpll_init.dividerY = rffpll_band_config_39MHz[SL_CLOCK_MANAGER_RFFPLL_BAND].divider_y;
  rffpll_init.dividerX = rffpll_band_config_39MHz[SL_CLOCK_MANAGER_RFFPLL_BAND].divider_x;
  rffpll_init.dividerN = rffpll_band_config_39MHz[SL_CLOCK_MANAGER_RFFPLL_BAND].divider_n;
#endif

  // Initialize RFFPLL.
  CMU_RFFPLLInit(&rffpll_init);

  // Update RFFPLL frequency in System file
  SystemRFFPLLClockSet(rffpll_init.frequency);

  // At this point, RFFPLL has been initialized. The clock source for SYSCLK can be
  // RFFPLLSYS input clock. If you want RFFPLLSYS, configure SL_CLOCK_MANAGER_SYSCLK_SOURCE
  // to CMU_SYSCLKCTRL_CLKSEL_RFFPLL0SYS.
}
#endif

#if defined(USBPLL_PRESENT) \
  && defined(SL_CLOCK_MANAGER_HFXO_EN) && (SL_CLOCK_MANAGER_HFXO_EN == 1)
/***************************************************************************//**
 * Initializes USBPLL Oscillator.
 ******************************************************************************/
FUNCTION_SCOPE void init_usbpll(void)
{
  CMU_USBPLL_Init_TypeDef usbpll_config;
  uint32_t hfxo_freq = SystemHFXOClockGet();

  // Validate that HFXO frequency is adequate for USB PLL and set the right frequency.
  switch (hfxo_freq) {
    case 38000000:
      usbpll_config.hfxoRefFreq = cmuHFXORefFreq_38M0Hz;
      break;

    case 38400000:
      usbpll_config.hfxoRefFreq = cmuHFXORefFreq_38M4Hz;
      break;

    case 39000000:
      usbpll_config.hfxoRefFreq = cmuHFXORefFreq_39M0Hz;
      break;

    case 40000000:
      usbpll_config.hfxoRefFreq = cmuHFXORefFreq_40M0Hz;
      break;

    default:
      EFM_ASSERT(false);
      break;
  }

  // Set additional configurations.
  usbpll_config.shuntRegEn = false;
  usbpll_config.disOnDemand = false;
  usbpll_config.regLock = true;

  // Set Force Enable feature at first to force the PLL to start and validate it works.
  usbpll_config.forceEn = true;

  // Initialize USB PLL and wait for it to be ready.
  CMU_USBPLLInit(&usbpll_config);

  // Remove the Force Enable feature to let PLL module on-demand.
  usbpll_config.forceEn = false;

  // Re-initialized without the Force Enable feature.
  CMU_USBPLLInit(&usbpll_config);
}
#endif

/***************************************************************************//**
 * Initializes Clock branches.
 ******************************************************************************/
FUNCTION_SCOPE void init_clock_branches(void)
{
  // Initialize SYSCLK clock branch.
#if defined(SL_CLOCK_MANAGER_SYSCLK_SOURCE)
  sli_em_cmu_SYSCLKInitPreClockSelect();
#if (SL_CLOCK_MANAGER_SYSCLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(SYSCLK, CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(SYSCLK, SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_CONCATENATION));
#else
  CLOCK_MANAGER_CLOCK_SELECT_SET(SYSCLK, SL_CLOCK_MANAGER_SYSCLK_SOURCE);
#endif
  sli_em_cmu_SYSCLKInitPostClockSelect(false);

  // Set HCLK and PCLK dividers
  CMU->SYSCLKCTRL = (CMU->SYSCLKCTRL & ~(_CMU_SYSCLKCTRL_HCLKPRESC_MASK | _CMU_SYSCLKCTRL_PCLKPRESC_MASK))
                    | SL_CLOCK_MANAGER_HCLK_DIVIDER
                    | SL_CLOCK_MANAGER_PCLK_DIVIDER;
#else
  EFM_ASSERT(false);
#endif

  // Initialize TRACECLK clock branch.
#if defined(CoreDebug_DEMCR_TRCENA_Msk)
  // Disable the Core Debug module if already enabled
  bool trace_on = CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk;
  if (trace_on) {
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
  }
#endif
#if defined(SL_CLOCK_MANAGER_TRACECLK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(TRACECLK, SL_CLOCK_MANAGER_TRACECLK_SOURCE);
#endif
#if defined(SL_CLOCK_MANAGER_TRACECLK_DIVIDER)
  CMU->TRACECLKCTRL_SET = SL_CLOCK_MANAGER_TRACECLK_DIVIDER;
#endif
#if defined(CoreDebug_DEMCR_TRCENA_Msk)
  // Enable back the Core Debug module if it was already enabled
  if (trace_on) {
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

  // Initialize EM01GRPBCLK clock branch.
#if defined(_CMU_EM01GRPBCLKCTRL_MASK)
#if defined(SL_CLOCK_MANAGER_EM01GRPBCLK_SOURCE)
#if (SL_CLOCK_MANAGER_EM01GRPBCLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(EM01GRPBCLK, CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(EM01GRPBCLK, SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_CONCATENATION));
#else
  CLOCK_MANAGER_CLOCK_SELECT_SET(EM01GRPBCLK, SL_CLOCK_MANAGER_EM01GRPBCLK_SOURCE);
#endif
#else
  EFM_ASSERT(false);
#endif
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

  // Initialize IADCCLK clock branch.
#if defined(_CMU_IADCCLKCTRL_MASK)
#if defined(SL_CLOCK_MANAGER_IADCCLK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(IADCCLK, SL_CLOCK_MANAGER_IADCCLK_SOURCE);
#else
  EFM_ASSERT(false);
#endif
#endif

  // Initialize LESENSEHFCLK clock branch.
#if defined(LESENSE_PRESENT)
#if defined(SL_CLOCK_MANAGER_LESENSEHFCLK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(LESENSEHFCLK, SL_CLOCK_MANAGER_LESENSEHFCLK_SOURCE);
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

  // Initialize RTCC clock branch.
#if defined(RTCC_PRESENT)
#if defined(SL_CLOCK_MANAGER_RTCCCLK_SOURCE)
#if (SL_CLOCK_MANAGER_RTCCCLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(RTCCCLK, CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(RTCCCLK, SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_CONCATENATION));
#else
  CLOCK_MANAGER_CLOCK_SELECT_SET(RTCCCLK, SL_CLOCK_MANAGER_RTCCCLK_SOURCE);
#endif
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

  // Initialize WDOG1 clock branch.
#if WDOG_COUNT > 1
#if defined(SL_CLOCK_MANAGER_WDOG1CLK_SOURCE)
#if (SL_CLOCK_MANAGER_WDOG1CLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(WDOG1CLK, CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(WDOG1CLK, SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_CONCATENATION));
#else
  CLOCK_MANAGER_CLOCK_SELECT_SET(WDOG1CLK, SL_CLOCK_MANAGER_WDOG1CLK_SOURCE);
#endif
#else
  EFM_ASSERT(false);
#endif
#endif

  // Initialize LCD clock branch.
#if defined(LCD_PRESENT)
#if defined(SL_CLOCK_MANAGER_LCDCLK_SOURCE)
#if (SL_CLOCK_MANAGER_LCDCLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(LCDCLK, CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(LCDCLK, SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_CONCATENATION));
#else
  CLOCK_MANAGER_CLOCK_SELECT_SET(LCDCLK, SL_CLOCK_MANAGER_LCDCLK_SOURCE);
#endif
#else
  EFM_ASSERT(false);
#endif
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
// EUSART clock source defined as SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE
#if (SL_CLOCK_MANAGER_EUSART0CLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_EUSART0_LF_CLOCK_SOURCE)
#if defined(CMU_EUSART0CLKCTRL_CLKSEL_EM23GRPACLK)
  // EUSART0 uses EM23GRPACLK as default LF clock source
  CLOCK_MANAGER_CLOCK_SELECT_SET(EUSART0CLK, CMU_EUSART0CLKCTRL_CLKSEL_EM23GRPACLK);
#else
#if (SL_CLOCK_MANAGER_DEFAULT_EUSART0_LF_CLOCK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE)
  // Default EUSART0 "SL_CLOCK_MANAGER_EUSART0CLK_SOURCE=SL_CLOCK_MANAGER_DEFAULT_EUSART0_LF_CLOCK_SOURCE"
  CLOCK_MANAGER_CLOCK_SELECT_SET(EUSART0CLK, CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(EUSART0CLK, SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_CONCATENATION));
#endif
#endif
#else
  // EUSART clock source explicitly defined i.e. SL_CLOCK_MANAGER_EUSART0CLK_SOURCE = CMU_EUSART0CLKCTRL_CLKSEL_LFRCO
  CLOCK_MANAGER_CLOCK_SELECT_SET(EUSART0CLK, SL_CLOCK_MANAGER_EUSART0CLK_SOURCE);
#endif
#else
  EFM_ASSERT(false);
#endif
#endif

  // Initialize EUART
#if defined(EUART_PRESENT)
#if defined(SL_CLOCK_MANAGER_EUART0CLK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(EUART0CLK, SL_CLOCK_MANAGER_EUART0CLK_SOURCE);
#else
  EFM_ASSERT(false);
#endif
#endif

  // Initialize SYSTICK clock branch.
#if defined(SL_CLOCK_MANAGER_SYSTICKCLK_SOURCE)
#if (SL_CLOCK_MANAGER_SYSTICKCLK_SOURCE == 1)
  CMU_CLOCK_SELECT_SET(SYSTICK, EM23GRPACLK);
#elif (SL_CLOCK_MANAGER_SYSTICKCLK_SOURCE == 0)
  CMU_CLOCK_SELECT_SET(SYSTICK, HCLK);
#else
  EFM_ASSERT(false);
#endif
#else
  EFM_ASSERT(false);
#endif

  // Initialize VDAC0 clock branch.
#if defined(VDAC_PRESENT)
#if defined(SL_CLOCK_MANAGER_VDAC0CLK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(VDAC0CLK, SL_CLOCK_MANAGER_VDAC0CLK_SOURCE);
#else
  EFM_ASSERT(false);
#endif

  // Initialize VDAC1 clock branch.
#if VDAC_COUNT > 1
#if defined(SL_CLOCK_MANAGER_VDAC1CLK_SOURCE)
  CLOCK_MANAGER_CLOCK_SELECT_SET(VDAC1CLK, SL_CLOCK_MANAGER_VDAC1CLK_SOURCE);
#else
  EFM_ASSERT(false);
#endif
#endif
#endif
}

/*******************************************************************************
 **********************   GLOBAL INTERNAL FUNCTIONS   **************************
 ******************************************************************************/

/***************************************************************************//**
 * Initializes Oscillators and Clock branches.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_init(void)
{
#if defined(SYSRTC_PRESENT)
  sl_status_t status;

  status =  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_SYSRTC0);
  EFM_ASSERT(status == SL_STATUS_OK);
#endif

  // Initialize Oscillators
#if defined(LFXO_PRESENT) \
  && defined(SL_CLOCK_MANAGER_LFXO_EN) && (SL_CLOCK_MANAGER_LFXO_EN == 1)
  init_lfxo();
#endif

#if defined(HFXO_PRESENT) \
  && defined(SL_CLOCK_MANAGER_HFXO_EN) && (SL_CLOCK_MANAGER_HFXO_EN == 1)
  init_hfxo();
#endif

  init_clkin0();

#if defined(HFRCO_PRESENT)
  init_hfrcodpll();
#endif

#if defined(HFRCOEM23_PRESENT)
  init_hfrcoem23();
#endif

#if defined(LFRCO_PRESENT)
  init_lfrco();
#endif

#if defined(RFFPLL_PRESENT) \
  && defined(SL_CLOCK_MANAGER_HFXO_EN) && (SL_CLOCK_MANAGER_HFXO_EN == 1)
  init_rffpll();
#endif

#if defined(USBPLL_PRESENT) \
  && defined(SL_CLOCK_MANAGER_HFXO_EN) && (SL_CLOCK_MANAGER_HFXO_EN == 1)
  init_usbpll();
#endif

  // Initialize Clock branches
  init_clock_branches();

  return SL_STATUS_OK;
}
