/***************************************************************************//**
 * @file
 * @brief Clock Manager - Oscillators configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2026 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_CLOCK_MANAGER_OSCILLATOR_CONFIG_H
#define SL_CLOCK_MANAGER_OSCILLATOR_CONFIG_H

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"

#endif

// Internal Defines: DO NOT MODIFY
#define SL_CLOCK_MANAGER_HFXO_EN_ENABLE     1
#define SL_CLOCK_MANAGER_HFXO_EN_DISABLE    0

#if defined(SL_CATALOG_RAIL_LIB_PRESENT)
#define SL_CLOCK_MANAGER_HFXO_EN_AUTO       SL_CLOCK_MANAGER_HFXO_EN_ENABLE
#else
#define SL_CLOCK_MANAGER_HFXO_EN_AUTO       SL_CLOCK_MANAGER_HFXO_EN_DISABLE
#endif

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Oscillators Settings

// <h> HFXO Settings (if High Frequency crystal is used)

// <o SL_CLOCK_MANAGER_HFXO_EN> Enable
// <i> Enable to configure HFXO
// <i> AUTO enables HFXO if a radio is used
// <SL_CLOCK_MANAGER_HFXO_EN_AUTO=> AUTO
// <SL_CLOCK_MANAGER_HFXO_EN_ENABLE=> ENABLE
// <SL_CLOCK_MANAGER_HFXO_EN_DISABLE=> DISABLE
// <d> SL_CLOCK_MANAGER_HFXO_EN_AUTO
#ifndef SL_CLOCK_MANAGER_HFXO_EN
#define SL_CLOCK_MANAGER_HFXO_EN    SL_CLOCK_MANAGER_HFXO_EN_AUTO
#endif

// <o SL_CLOCK_MANAGER_HFXO_MODE> Mode
// <i>
// <HFXO_CFG_MODE_XTAL=> XTAL
// <HFXO_CFG_MODE_EXTCLK=> EXTCLK
// <HFXO_CFG_MODE_EXTCLKPKDET=> EXTCLKPKDET
// <d> HFXO_CFG_MODE_XTAL
#ifndef SL_CLOCK_MANAGER_HFXO_MODE
#define SL_CLOCK_MANAGER_HFXO_MODE    HFXO_CFG_MODE_XTAL
#endif

// <o SL_CLOCK_MANAGER_HFXO_FREQ> Frequency in Hz <38000000-40000000>
// <d> 38400000
#ifndef SL_CLOCK_MANAGER_HFXO_FREQ
#define SL_CLOCK_MANAGER_HFXO_FREQ    38400000
#endif

// <o SL_CLOCK_MANAGER_HFXO_CTUNE> CTUNE <0-255>
// <d> 170
#ifndef SL_CLOCK_MANAGER_HFXO_CTUNE
#define SL_CLOCK_MANAGER_HFXO_CTUNE    170
#endif

// <q SL_CLOCK_MANAGER_HFXO_ENXIDCBIASANA> Enable DC Bias (AC-Coupled EXTCLK)
// <i> Enable to add an internal DC bias
// <i> when AC-coupled connected external oscillator
// <d> 0
#ifndef SL_CLOCK_MANAGER_HFXO_ENXIDCBIASANA
#define SL_CLOCK_MANAGER_HFXO_ENXIDCBIASANA    0
#endif

// <o SL_CLOCK_MANAGER_HFXO_PRECISION> Precision in PPM <0-65535>
// <d> 50
#ifndef SL_CLOCK_MANAGER_HFXO_PRECISION
#define SL_CLOCK_MANAGER_HFXO_PRECISION    50
#endif

// <e SL_CLOCK_MANAGER_HFXO_SEPARATE_CTUNEXIANA_CTUNEXOANA_EN> Enable Separate CTUNE XI/XO
// <i> Enable to be able to configure the XI and XO CTUNE values separately
#ifndef SL_CLOCK_MANAGER_HFXO_SEPARATE_CTUNEXIANA_CTUNEXOANA_EN
#define SL_CLOCK_MANAGER_HFXO_SEPARATE_CTUNEXIANA_CTUNEXOANA_EN    0
#endif

// <o SL_CLOCK_MANAGER_HFXO_CTUNEXIANA> CTUNE XI <0-255>
// <d> 170
#ifndef SL_CLOCK_MANAGER_HFXO_CTUNEXIANA
#define SL_CLOCK_MANAGER_HFXO_CTUNEXIANA    170
#endif

// <o SL_CLOCK_MANAGER_HFXO_CTUNEXOANA> CTUNE XO <0-255>
// <d> 170
#ifndef SL_CLOCK_MANAGER_HFXO_CTUNEXOANA
#define SL_CLOCK_MANAGER_HFXO_CTUNEXOANA    170
#endif

// </e>

// <e SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_SUPPORT> HFXO Sleepy Crystal Support
// <i> Enable to configure HFXO Sleepy Crystal Support
#ifndef SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_SUPPORT
#define SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_SUPPORT    0
#endif

// <o SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_CTUNE_VALUE> HFXO_SLEEPY_CRYSTAL_CTUNE <0-255>
// <i> This setting is temporarily used to force oscillation on sleepy crystal. Default values should be enough to wake-up sleepy crystals.
// <i> Only the 3 most significant bits will be used for this configuration.
// <d> 64
#ifndef SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_CTUNE_VALUE
#define SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_CTUNE_VALUE    64
#endif

// <o SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_CORE_BIAS_VALUE> HFXO_SLEEPY_CRYSTAL_CORE_BIAS <0-255>
// <i> This setting is temporarily used to force oscillation on sleepy crystal. Default values should be enough to wake-up sleepy crystals.
// <d> 511
#ifndef SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_CORE_BIAS_VALUE
#define SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_CORE_BIAS_VALUE    511
#endif
// </e>
// </h>

// <e> LFXO Settings (if Low Frequency crystal is used)
// <i> Enable to configure LFXO
#ifndef SL_CLOCK_MANAGER_LFXO_EN
#define SL_CLOCK_MANAGER_LFXO_EN    0
#endif

// <o SL_CLOCK_MANAGER_LFXO_MODE> Mode
// <i>
// <LFXO_CFG_MODE_XTAL=> XTAL
// <LFXO_CFG_MODE_XTAL_HP=> XTAL_HP
// <LFXO_CFG_MODE_BUFEXTCLK=> BUFEXTCLK
// <LFXO_CFG_MODE_DIGEXTCLK=> DIGEXTCLK
// <d> LFXO_CFG_MODE_XTAL
#ifndef SL_CLOCK_MANAGER_LFXO_MODE
#define SL_CLOCK_MANAGER_LFXO_MODE    LFXO_CFG_MODE_XTAL
#endif

// <o SL_CLOCK_MANAGER_LFXO_CTUNE> CTUNE <0-127>
// <d> 63
#ifndef SL_CLOCK_MANAGER_LFXO_CTUNE
#define SL_CLOCK_MANAGER_LFXO_CTUNE    63
#endif

// <o SL_CLOCK_MANAGER_LFXO_GAIN> Gain <0-3>
// <i> LFXO gain setting for load capacitance matching
// <d> 1
#ifndef SL_CLOCK_MANAGER_LFXO_GAIN
#define SL_CLOCK_MANAGER_LFXO_GAIN    1
#endif

// <o SL_CLOCK_MANAGER_LFXO_PRECISION> LFXO precision in PPM <0-65535>
// <d> 50
#ifndef SL_CLOCK_MANAGER_LFXO_PRECISION
#define SL_CLOCK_MANAGER_LFXO_PRECISION    50
#endif

// <o SL_CLOCK_MANAGER_LFXO_TIMEOUT> Startup Timeout Delay
// <i>
// <LFXO_CFG_TIMEOUT_CYCLES2=> CYCLES2
// <LFXO_CFG_TIMEOUT_CYCLES256=> CYCLES256
// <LFXO_CFG_TIMEOUT_CYCLES1K=> CYCLES1K
// <LFXO_CFG_TIMEOUT_CYCLES2K=> CYCLES2K
// <LFXO_CFG_TIMEOUT_CYCLES4K=> CYCLES4K
// <LFXO_CFG_TIMEOUT_CYCLES8K=> CYCLES8K
// <LFXO_CFG_TIMEOUT_CYCLES16K=> CYCLES16K
// <LFXO_CFG_TIMEOUT_CYCLES32K=> CYCLES32K
// <d> LFXO_CFG_TIMEOUT_CYCLES4K
#ifndef SL_CLOCK_MANAGER_LFXO_TIMEOUT
#define SL_CLOCK_MANAGER_LFXO_TIMEOUT    LFXO_CFG_TIMEOUT_CYCLES4K
#endif
// </e>

// <h> HFRCO and DPLL Settings
// <o SL_CLOCK_MANAGER_HFRCO_BAND> Frequency Band
// <i> RC Oscillator Frequency Band
// <38000000=> 38 MHz
// <100000000=> 100 MHz
// <d> 38000000
#ifndef SL_CLOCK_MANAGER_HFRCO_BAND
#define SL_CLOCK_MANAGER_HFRCO_BAND    38000000
#endif

// <e> Use DPLL
// <i> Enable to use the DPLL with HFRCO
#ifndef SL_CLOCK_MANAGER_HFRCO_DPLL_EN
#define SL_CLOCK_MANAGER_HFRCO_DPLL_EN    0
#endif
// <o SL_CLOCK_MANAGER_DPLL_FREQ> Target Frequency in Hz <20000000-100000000>
// <i> DPLL target frequency
// <d> 76800000
#ifndef SL_CLOCK_MANAGER_DPLL_FREQ
#define SL_CLOCK_MANAGER_DPLL_FREQ    76800000
#endif

// <o SL_CLOCK_MANAGER_DPLL_N> Numerator (N) <300-4095>
// <i> Value of N for output frequency calculation fout = fref * (N+1) / (M+1)
// <d> 3839
#ifndef SL_CLOCK_MANAGER_DPLL_N
#define SL_CLOCK_MANAGER_DPLL_N    3839
#endif

// <o SL_CLOCK_MANAGER_DPLL_M> Denominator (M) <0-4095>
// <i> Value of M for output frequency calculation fout = fref * (N+1) / (M+1)
// <d> 1919
#ifndef SL_CLOCK_MANAGER_DPLL_M
#define SL_CLOCK_MANAGER_DPLL_M    1919
#endif

// <o SL_CLOCK_MANAGER_DPLL_REFCLK> Reference Clock
// <i> Reference clock source for DPLL
// <CMU_DPLLREFCLKCTRL_CLKSEL_DISABLED=> DISABLED
// <CMU_DPLLREFCLKCTRL_CLKSEL_HFXO=> HFXO
// <CMU_DPLLREFCLKCTRL_CLKSEL_LFXO=> LFXO
// <CMU_DPLLREFCLKCTRL_CLKSEL_CLKIN0=> CLKIN0
// <d> CMU_DPLLREFCLKCTRL_CLKSEL_HFXO
#ifndef SL_CLOCK_MANAGER_DPLL_REFCLK
#define SL_CLOCK_MANAGER_DPLL_REFCLK    CMU_DPLLREFCLKCTRL_CLKSEL_HFXO
#endif

// <o SL_CLOCK_MANAGER_DPLL_EDGE> Reference Clock Edge Detect
// <i> Edge detection for reference clock
// <0=> Falling Edge
// <1=> Rising Edge
// <d> 0
#ifndef SL_CLOCK_MANAGER_DPLL_EDGE
#define SL_CLOCK_MANAGER_DPLL_EDGE    0
#endif

// <o SL_CLOCK_MANAGER_DPLL_LOCKMODE> DPLL Lock Mode
// <i> Lock mode
// <_DPLL_CFG_MODE_FLL=> Frequency-Lock Loop
// <_DPLL_CFG_MODE_PLL=> Phase-Lock Loop
// <d> cmuDPLLLockMode_Freq
#ifndef SL_CLOCK_MANAGER_DPLL_LOCKMODE
#define SL_CLOCK_MANAGER_DPLL_LOCKMODE    _DPLL_CFG_MODE_PLL
#endif

// <q SL_CLOCK_MANAGER_DPLL_AUTORECOVER> Automatic Lock Recovery
// <d> 1
#ifndef SL_CLOCK_MANAGER_DPLL_AUTORECOVER
#define SL_CLOCK_MANAGER_DPLL_AUTORECOVER    1
#endif

// <q SL_CLOCK_MANAGER_DPLL_DITHER> Enable Dither
// <d> 0
#ifndef SL_CLOCK_MANAGER_DPLL_DITHER
#define SL_CLOCK_MANAGER_DPLL_DITHER    0
#endif
// </e>
// </h>

// <h> HFRCOEM23 Settings
// <o SL_CLOCK_MANAGER_HFRCOEM23_BAND> Frequency Band
// <i> RC Oscillator Frequency Band
// <20000000=> 20 MHz
// <40000000=> 40 MHz
// <d> 20000000
#ifndef SL_CLOCK_MANAGER_HFRCOEM23_BAND
#define SL_CLOCK_MANAGER_HFRCOEM23_BAND    20000000
#endif
// </h>

// <h> LFRCO Settings
// <o SL_CLOCK_MANAGER_LFRCO_PRECISION> Precision Mode
// <i> Precision mode uses hardware to automatically re-calibrate the LFRCO
// <i> against a crystal driven by the HFXO. Hardware detects temperature
// <i> changes and initiates a re-calibration of the LFRCO as needed when
// <i> operating in EM0, EM1, or EM2. If a re-calibration is necessary and the
// <i> HFXO is not active, the precision mode hardware will automatically
// <i> enable HFXO for a short time to perform the calibration. EM4 operation is
// <i> not allowed while precision mode is enabled.
// <i> If high precision is selected on devices that do not support it, default
// <i> precision will be used.
// <0=> Default precision
// <1=> High precision
// <d> 0
#ifndef SL_CLOCK_MANAGER_LFRCO_PRECISION
#define SL_CLOCK_MANAGER_LFRCO_PRECISION    0
#endif
// </h>

// <h> SOCPLL Settings
// <e> Use SOCPLL
// <i> Enable to initialize and use the SOCPLL.
// <d> 1
#ifndef SL_CLOCK_MANAGER_SOCPLL_EN
#define SL_CLOCK_MANAGER_SOCPLL_EN    1
#endif

// <o SL_CLOCK_MANAGER_SOCPLL_FREQ> Target Frequency in Hz
// <i> SOCPLL target output frequency
// <145000000=> 145 MHz
// <150000000=> 150 MHz
// <d> 150000000
#ifndef SL_CLOCK_MANAGER_SOCPLL_FREQ
#define SL_CLOCK_MANAGER_SOCPLL_FREQ    150000000
#endif

// <e> SOCPLL Advanced Settings
// <i> Enable to configure SOCPLL manually
#ifndef SL_CLOCK_MANAGER_SOCPLL_ADVANCED_SETTINGS
#define SL_CLOCK_MANAGER_SOCPLL_ADVANCED_SETTINGS    0
#endif

#if SL_CLOCK_MANAGER_SOCPLL_ADVANCED_SETTINGS == 1
// <o SL_CLOCK_MANAGER_SOCPLL_REFCLK> Reference Clock
// <i> Reference clock source for SOCPLL
// <SOCPLL_CTRL_REFCLKSEL_REF_HFXO=> REF_HFXO
// <SOCPLL_CTRL_REFCLKSEL_REF_HFRCO=> REF_HFRCO
// <SOCPLL_CTRL_REFCLKSEL_REF_EXTCLK=> REF_EXTCLK
// <d> SOCPLL_CTRL_REFCLKSEL_REF_HFRCO
#ifndef SL_CLOCK_MANAGER_SOCPLL_REFCLK
#define SL_CLOCK_MANAGER_SOCPLL_REFCLK    SOCPLL_CTRL_REFCLKSEL_REF_HFRCO
#endif
#endif
// </e>
// </e>
// </h>

// <h> CLKIN0 Settings
// <o SL_CLOCK_MANAGER_CLKIN0_FREQ> Frequency in Hz <1000000-38000000>
// <d> 38000000
#ifndef SL_CLOCK_MANAGER_CLKIN0_FREQ
#define SL_CLOCK_MANAGER_CLKIN0_FREQ    38000000
#endif
// </h>

// </h>

// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>

// <cmu signal=CLKIN0 optional=true> SL_CLOCK_MANAGER_CLKIN0
// $[CMU_SL_CLOCK_MANAGER_CLKIN0]

// [CMU_SL_CLOCK_MANAGER_CLKIN0]$

// <<< sl:end pin_tool >>>

#endif /* SL_CLOCK_MANAGER_OSCILLATOR_CONFIG_H */
