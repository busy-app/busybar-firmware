/***************************************************************************//**
 * @file
 * @brief Clock Manager Init HAL APIs.
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

#ifndef SL_CLOCK_MANAGER_INIT_HAL_H
#define SL_CLOCK_MANAGER_INIT_HAL_H

#include "sl_status.h"
#include "sl_clock_manager_tree_config.h"
#include "sl_clock_manager_oscillator_config.h"
#include "em_device.h"

#if defined(CLOCK_MANAGER_INIT_HAL_INTERNAL_PRESENT)
#include "sli_clock_manager_init_hal_internal.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 ********************************  MACROS   ************************************
 ******************************************************************************/

#if (SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_HFRCODPLL)
#define SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_CONCATENATION      _HFRCODPLL
#elif (SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_HFXO)
#define SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_CONCATENATION      _HFXO
#elif (SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_FSRCO)
#define SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_CONCATENATION      _FSRCO
#else
#error "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE configuration value is invalid."
#endif

#if (SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_LFRCO)
#define SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_CONCATENATION       _LFRCO
#elif (SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_LFXO)
#define SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_CONCATENATION       _LFXO
#elif (SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_ULFRCO)
#define SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_CONCATENATION      _ULFRCO
#else
#error "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE configuration value is invalid."
#endif

#define CLOCK_MANAGER_CLOCK_SELECT_SET(clock_branch, clock_source)                               \
  do {                                                                                           \
    CMU->clock_branch##CTRL = (CMU->clock_branch##CTRL & ~_CMU_##clock_branch##CTRL_CLKSEL_MASK) \
                              | clock_source;                                                    \
  } while (0)

#define CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE_NX(clock_branch, clock_source) CMU_##clock_branch##CTRL_CLKSEL##clock_source
#define CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE(clock_branch, clock_source)    CLOCK_MANAGER_GET_DEFAULT_CLOCK_SOURCE_NX(clock_branch, clock_source)

#define SLI_CLOCK_MANAGER_FSRCO_DEFAULT_FREQ  20000000UL

#if !defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)
#define SLI_CLOCK_MANAGER_HFXO_MODE SL_CLOCK_MANAGER_HFXO_MODE
#define SLI_CLOCK_MANAGER_HFXO_FREQ SL_CLOCK_MANAGER_HFXO_FREQ

#define SLI_CLOCK_MANAGER_HFRCO_BAND      SL_CLOCK_MANAGER_HFRCO_BAND
#define SLI_CLOCK_MANAGER_HFRCO_DPLL_EN   SL_CLOCK_MANAGER_HFRCO_DPLL_EN
#define SLI_CLOCK_MANAGER_DPLL_FREQ       SL_CLOCK_MANAGER_DPLL_FREQ
#define SLI_CLOCK_MANAGER_DPLL_N          SL_CLOCK_MANAGER_DPLL_N
#define SLI_CLOCK_MANAGER_DPLL_M          SL_CLOCK_MANAGER_DPLL_M

#if defined(SL_CLOCK_MANAGER_SOCPLL_EN) && (SL_CLOCK_MANAGER_SOCPLL_EN == 1)
#if defined(SL_CLOCK_MANAGER_SOCPLL_ADVANCED_SETTINGS) && (SL_CLOCK_MANAGER_SOCPLL_ADVANCED_SETTINGS == 0)
#if defined(SL_CATALOG_RAIL_LIB_PRESENT) || (defined(SL_CLOCK_MANAGER_HFXO_EN) && (SL_CLOCK_MANAGER_HFXO_EN == 1))
#define SL_CLOCK_MANAGER_SOCPLL_REFCLK        SOCPLL_CTRL_REFCLKSEL_REF_HFXO
#else
#define SL_CLOCK_MANAGER_SOCPLL_REFCLK        SOCPLL_CTRL_REFCLKSEL_REF_HFRCO
#endif
#endif // SL_CLOCK_MANAGER_SOCPLL_ADVANCED_SETTINGS
#endif // SL_CLOCK_MANAGER_SOCPLL_EN
#endif // !defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION)

#if defined(SL_CLOCK_MANAGER_SOCPLL_EN) \
  && ((SL_CLOCK_MANAGER_SOCPLL_EN == 1) || defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION))
#if (SL_CLOCK_MANAGER_SOCPLL_REFCLK == SOCPLL_CTRL_REFCLKSEL_REF_HFXO)
#define SL_CLOCK_MANAGER_SOCPLL_REFCLK_FREQ   SL_CLOCK_MANAGER_HFXO_FREQ
#elif (SL_CLOCK_MANAGER_SOCPLL_REFCLK == SOCPLL_CTRL_REFCLKSEL_REF_HFRCO)
#define SL_CLOCK_MANAGER_SOCPLL_REFCLK_FREQ   SL_CLOCK_MANAGER_HFRCO_BAND
#elif (SL_CLOCK_MANAGER_SOCPLL_REFCLK == SOCPLL_CTRL_REFCLKSEL_REF_EXTCLK)
#error External Clock is not supported as SOCPLL reference clock
#endif

#if (SL_CLOCK_MANAGER_SOCPLL_REFCLK_FREQ < 34000000) || (SL_CLOCK_MANAGER_SOCPLL_REFCLK_FREQ > 44000000)
#error SOCPLL reference clock frequency must be between 34MHz and 44MHz
#endif

#ifndef SL_CLOCK_MANAGER_SOCPLL_FRACTIONAL_EN
#define SL_CLOCK_MANAGER_SOCPLL_FRACTIONAL_EN 1
#endif

// SOCPLL Formula: SOCPLL_FREQ = REFCLK_FREQ * (DIVN+2 + DIVF/1024) / 6
// SL_CLOCK_MANAGER_SOCPLL_DIVN is rounded down and SL_CLOCK_MANAGER_SOCPLL_DIVF is rounded to the closest integer.
#ifndef SL_CLOCK_MANAGER_SOCPLL_DIVN
#define SL_CLOCK_MANAGER_SOCPLL_DIVN          (6ULL * SL_CLOCK_MANAGER_SOCPLL_FREQ / SL_CLOCK_MANAGER_SOCPLL_REFCLK_FREQ - 2ULL)
#endif
#ifndef SL_CLOCK_MANAGER_SOCPLL_DIVF
#define SL_CLOCK_MANAGER_SOCPLL_DIVF          ((6ULL * 1024ULL * SL_CLOCK_MANAGER_SOCPLL_FREQ + SL_CLOCK_MANAGER_SOCPLL_REFCLK_FREQ / 2ULL) / SL_CLOCK_MANAGER_SOCPLL_REFCLK_FREQ - 1024ULL * (SL_CLOCK_MANAGER_SOCPLL_DIVN + 2ULL))
#endif
#endif // SL_CLOCK_MANAGER_SOCPLL_EN

#if !defined(SLI_CLOCK_MANAGER_RUNTIME_CONFIGURATION) \
  && (defined(SL_CLOCK_MANAGER_SOCPLL_EN) && (SL_CLOCK_MANAGER_SOCPLL_EN == 1))
#define SLI_CLOCK_MANAGER_SOCPLL_EN            SL_CLOCK_MANAGER_SOCPLL_EN
#define SLI_CLOCK_MANAGER_SOCPLL_FREQ          SL_CLOCK_MANAGER_SOCPLL_FREQ
#define SLI_CLOCK_MANAGER_SOCPLL_REFCLK        SL_CLOCK_MANAGER_SOCPLL_REFCLK
#define SLI_CLOCK_MANAGER_SOCPLL_REFCLK_FREQ   SL_CLOCK_MANAGER_SOCPLL_REFCLK_FREQ
#define SLI_CLOCK_MANAGER_SOCPLL_FRACTIONAL_EN SL_CLOCK_MANAGER_SOCPLL_FRACTIONAL_EN
#define SLI_CLOCK_MANAGER_SOCPLL_DIVN          SL_CLOCK_MANAGER_SOCPLL_DIVN
#define SLI_CLOCK_MANAGER_SOCPLL_DIVF          SL_CLOCK_MANAGER_SOCPLL_DIVF
#endif

/*******************************************************************************
 ****************************  PCLK AUTO DIVIDER  ****************************
 ******************************************************************************/

#if defined(_SILICON_LABS_32B_SERIES_2)
// Define frequency mappings for common HFRCO DPLL frequencies.
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_1M0Hz     1000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_1000000                    1000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_2M0Hz     2000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_2000000                    2000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_4M0Hz     4000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_4000000                    4000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_5M0Hz     5000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_5000000                    5000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_7M0Hz     7000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_7000000                    7000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_10M0Hz    10000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_10000000                   10000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_13M0Hz    13000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_13000000                   13000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_16M0Hz    16000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_16000000                   16000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_19M0Hz    19000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_19000000                   19000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_20M0Hz    20000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_20000000                   20000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_26M0Hz    26000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_26000000                   26000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_32M0Hz    32000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_32000000                   32000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_38M0Hz    38000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_38000000                   38000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_48M0Hz    48000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_48000000                   48000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_56M0Hz    56000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_56000000                   56000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_64M0Hz    64000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_64000000                   64000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_80M0Hz    80000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_80000000                   80000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_cmuHFRCODPLLFreq_100M0Hz   100000000UL
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ_100000000                  100000000UL

// Helper macros to extract the frequency from enum names.
#define SLI_ENUM_TO_FREQ_EXTRACT_FREQ(enum_name) SLI_ENUM_TO_FREQ_EXTRACT_FREQ_##enum_name
#define SLI_HFRCO_BAND_FREQ(hfrco_band_config) SLI_ENUM_TO_FREQ_EXTRACT_FREQ(hfrco_band_config)
#else
#define SLI_HFRCO_BAND_FREQ(hfrco_band_config) hfrco_band_config
#endif

// Calculate SYSCLK frequency based on configured source.
#if !defined(SLI_CLOCK_MANAGER_SYSCLK_FREQ_HZ)
#if (SL_CLOCK_MANAGER_SYSCLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_HFRCODPLL) \
  || (SL_CLOCK_MANAGER_SYSCLK_SOURCE == CMU_SYSCLKCTRL_CLKSEL_HFRCODPLL)
  #if (SL_CLOCK_MANAGER_HFRCO_DPLL_EN == 1)
    #define SLI_CLOCK_MANAGER_SYSCLK_FREQ_HZ    SL_CLOCK_MANAGER_DPLL_FREQ
  #else
    #define SLI_CLOCK_MANAGER_SYSCLK_FREQ_HZ    SLI_HFRCO_BAND_FREQ(SL_CLOCK_MANAGER_HFRCO_BAND)
  #endif
#elif (SL_CLOCK_MANAGER_SYSCLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_HFXO) \
  || (SL_CLOCK_MANAGER_SYSCLK_SOURCE == CMU_SYSCLKCTRL_CLKSEL_HFXO)
  #define SLI_CLOCK_MANAGER_SYSCLK_FREQ_HZ      SL_CLOCK_MANAGER_HFXO_FREQ
#elif (SL_CLOCK_MANAGER_SYSCLK_SOURCE == SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_FSRCO) \
  || (SL_CLOCK_MANAGER_SYSCLK_SOURCE == CMU_SYSCLKCTRL_CLKSEL_FSRCO)
  #define SLI_CLOCK_MANAGER_SYSCLK_FREQ_HZ      SLI_CLOCK_MANAGER_FSRCO_DEFAULT_FREQ
#elif (SL_CLOCK_MANAGER_SYSCLK_SOURCE == CMU_SYSCLKCTRL_CLKSEL_CLKIN0)
  #define SLI_CLOCK_MANAGER_SYSCLK_FREQ_HZ      SL_CLOCK_MANAGER_CLKIN0_FREQ
#elif defined(CMU_SYSCLKCTRL_CLKSEL_SOCPLL) && (SL_CLOCK_MANAGER_SYSCLK_SOURCE == CMU_SYSCLKCTRL_CLKSEL_SOCPLL)
  #define SLI_CLOCK_MANAGER_SYSCLK_FREQ_HZ    SL_CLOCK_MANAGER_SOCPLL_FREQ
#elif (SL_CLOCK_MANAGER_SYSCLK_SOURCE == CMU_SYSCLKCTRL_CLKSEL_RFFPLL0SYS)
  #define SLI_CLOCK_MANAGER_SYSCLK_FREQ_HZ    SL_CLOCK_MANAGER_RFFPLL_FREQ
#else
#error "SL_CLOCK_MANAGER_SYSCLK_SOURCE configuration value is invalid or unsupported for this device."
#endif
#endif

// Calculate HCLK frequency based on SYSCLK and HCLK divider.
#define SLI_CLOCK_MANAGER_HCLK_FREQ_HZ        SLI_CLOCK_MANAGER_SYSCLK_FREQ_HZ / ((SL_CLOCK_MANAGER_HCLK_DIVIDER >> _CMU_SYSCLKCTRL_HCLKPRESC_SHIFT) + 1)

// Calculate optimal PCLK divider to keep the PCLK frequency inside the operating conditions.
#if !defined(SLI_CLOCK_MANAGER_PCLK_MAX_FREQ_HZ)
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_3)  \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_13) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_4) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_6) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_7) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_8) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_9) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_11)  \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_13)
#define SLI_CLOCK_MANAGER_PCLK_MAX_FREQ_HZ      40000000UL
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2)  \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_5)
// The maximum PCLK frequency for EFX32XG25 is actually 48.75MHz,
// but 50MHz is used to account for the fact that the HFRCO Band
// said to be 100MHz would actually be around 97.5MHz. 50MHz is
// used to prevent validation warnings.
#define SLI_CLOCK_MANAGER_PCLK_MAX_FREQ_HZ      50000000UL
#elif defined(_SILICON_LABS_32B_SERIES_2)
#warning "No maximum PCLK frequency defined for this device."
#elif defined(_SILICON_LABS_32B_SERIES_3)
#define SLI_CLOCK_MANAGER_PCLK_MAX_FREQ_HZ      75000000UL
#endif
#endif

// Define optimal PCLK divider based on HCLK and PCLK maximum frequency.
#if (SLI_CLOCK_MANAGER_HCLK_FREQ_HZ <= SLI_CLOCK_MANAGER_PCLK_MAX_FREQ_HZ)
#define SL_CLOCK_MANAGER_PCLK_DIV_MIN   CMU_SYSCLKCTRL_PCLKPRESC_DIV1
#elif ((SLI_CLOCK_MANAGER_HCLK_FREQ_HZ / 2) <= (SLI_CLOCK_MANAGER_PCLK_MAX_FREQ_HZ))
#define SL_CLOCK_MANAGER_PCLK_DIV_MIN   CMU_SYSCLKCTRL_PCLKPRESC_DIV2
#else
#define SL_CLOCK_MANAGER_PCLK_DIV_MIN   CMU_SYSCLKCTRL_PCLKPRESC_DIV2
#warning "Maximum PCLK frequency exceeded with biggest available PCLK divider."
#endif

#if !defined(SL_CLOCK_MANAGER_DEFAULT_EUSART0_LF_CLOCK_SOURCE)
#if defined(CMU_EUSART0CLKCTRL_CLKSEL_EM23GRPACLK)
#define SL_CLOCK_MANAGER_DEFAULT_EUSART0_LF_CLOCK_SOURCE CMU_EUSART0CLKCTRL_CLKSEL_EM23GRPACLK
#else
#define SL_CLOCK_MANAGER_DEFAULT_EUSART0_LF_CLOCK_SOURCE SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE
#endif
#endif
/*******************************************************************************
 ******************************  PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * Initializes Oscillators and Clock branches.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_init(void);

#ifdef __cplusplus
}
#endif

#endif // SL_CLOCK_MANAGER_INIT_HAL_H
