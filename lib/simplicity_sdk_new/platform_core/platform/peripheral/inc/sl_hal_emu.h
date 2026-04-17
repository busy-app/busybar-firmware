/***************************************************************************//**
 * @file
 * @brief Energy Management Unit (EMU) peripheral API
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_HAL_EMU_H
#define SL_HAL_EMU_H

#include "em_device.h"

#if defined(EMU_PRESENT)

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include "sl_status.h"
#include "sl_enum.h"
#include "sl_hal_syscfg.h"

// *****************************************************************************
/// @addtogroup emu EMU - Energy management unit
/// @brief Energy management unit control peripheral
/// @{
// *****************************************************************************

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

#define EMU_TEMP_ZERO_C_IN_KELVIN (273.15f)

// Check if DCDC is presented
#if defined(_SILICON_LABS_DCDC_FEATURE) && defined(_SILICON_LABS_32B_SERIES) && (_SILICON_LABS_32B_SERIES == 2)
#if ((_SILICON_LABS_DCDC_FEATURE == _SILICON_LABS_DCDC_FEATURE_DCDC_BUCK) \
  || (_SILICON_LABS_DCDC_FEATURE == _SILICON_LABS_DCDC_FEATURE_DCDC_BOB))
// DC-DC buck converter present.
#define SL_HAL_EMU_DCDC_BUCK_PRESENT
#endif
#if ((_SILICON_LABS_DCDC_FEATURE == _SILICON_LABS_DCDC_FEATURE_DCDC_BOOST) \
  || (_SILICON_LABS_DCDC_FEATURE == _SILICON_LABS_DCDC_FEATURE_DCDC_BOB))
// DC-DC boost converter present.
#define SL_HAL_EMU_DCDC_BOOST_PRESENT
#endif
#endif

/** @endcond */

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

#if defined(_EMU_TEMP_TEMPAVG_MASK)
/// Number of samples to use for temperature averaging.
SL_ENUM_GENERIC(sl_hal_emu_temperature_average_num_t, uint32_t) {
  SL_HAL_EMU_TEMP_AVERAGE_16     = EMU_CTRL_TEMPAVGNUM_N16,  ///< 16 samples used for temperature averaging.
  SL_HAL_EMU_TEMP_AVERAGE_64     = EMU_CTRL_TEMPAVGNUM_N64,  ///< 64 samples used for temperature averaging.
};
#endif

/// Peripheral reset control.
SL_ENUM(sl_hal_emu_reset_mode_t) {
  SL_HAL_EMU_RESET_MODE_DISABLED = 0,  ///< Reset mode disabled.
  SL_HAL_EMU_RESET_MODE_ENABLED  = 1,  ///< Reset mode enabled.
};

/// Peripheral reset control and reset source control.
SL_ENUM(sl_hal_emu_reset_source_t) {
#if defined(_EMU_RSTCTRL_WDOG0RMODE_SHIFT)
  SL_HAL_EMU_RESET_WDOG0        = _EMU_RSTCTRL_WDOG0RMODE_SHIFT,       ///< WDOG0 reset select.
#endif
#if defined(_EMU_RSTCTRL_WDOG1RMODE_SHIFT)
  SL_HAL_EMU_RESET_WDOG1        = _EMU_RSTCTRL_WDOG1RMODE_SHIFT,       ///< WDOG1 reset select.
#endif
#if defined(_EMU_RSTCTRL_SYSRMODE_MASK)
  SL_HAL_EMU_RESET_SYS          = _EMU_RSTCTRL_SYSRMODE_SHIFT,         ///< SYSRESET select.
#endif
#if defined(_EMU_RSTCTRL_LOCKUPRMODE_SHIFT)
  SL_HAL_EMU_RESET_CORE_LOCKUP  = _EMU_RSTCTRL_LOCKUPRMODE_SHIFT,      ///< Cortex lockup reset select.
#endif
#if defined(_EMU_RSTCTRL_AVDDBODRMODE_SHIFT)
  SL_HAL_EMU_RESET_AVDD         = _EMU_RSTCTRL_AVDDBODRMODE_SHIFT,     ///< AVDD monitoring select.
#endif
#if defined(_EMU_RSTCTRL_IOVDD0BODRMODE_SHIFT)
  SL_HAL_EMU_RESET_IOVDD0       = _EMU_RSTCTRL_IOVDD0BODRMODE_SHIFT,   ///< IOVDD0 monitoring select.
#endif
#if defined(_EMU_RSTCTRL_IOVDD1BODRMODE_SHIFT)
  SL_HAL_EMU_RESET_IOVDD1       = _EMU_RSTCTRL_IOVDD1BODRMODE_SHIFT,   ///< IOVDD1 monitoring select.
#endif
#if defined(_EMU_RSTCTRL_DECBODRMODE_SHIFT)
  SL_HAL_EMU_RESET_DECOUPLE     = _EMU_RSTCTRL_DECBODRMODE_SHIFT,      ///< Decouple monitoring select.
#endif
#if defined(_EMU_RSTCTRL_FLBODRMODE_SHIFT)
  SL_HAL_EMU_RESET_FLASH        = _EMU_RSTCTRL_FLBODRMODE_SHIFT,       ///< Flash Supply monitoring select.
#endif
#if defined(_EMU_RSTCTRL_SEM0SYSRMODE_SHIFT)
  SL_HAL_EMU_RESET_SE_SYS       = _EMU_RSTCTRL_SEM0SYSRMODE_SHIFT,     ///< M0+ (SE) system reset select.
#endif
#if defined(_EMU_RSTCTRL_SEM0LOCKUPRMODE_SHIFT)
  SL_HAL_EMU_RESET_SE_LOCKUP    = _EMU_RSTCTRL_SEM0LOCKUPRMODE_SHIFT,  ///< M0+ (SE) lockup select.
#endif
};

#if defined(_EMU_CTRL_HDREGSTOPGEAR_MASK)
/// HDREG Stop Gear Max Current Type.
SL_ENUM(sl_hal_emu_hdreg_stop_gear_current_limit_t) {
  SL_HAL_EMU_HDREG_STOP_GEAR_CURRENT_LIMIT_4MA    = _EMU_CTRL_HDREGSTOPGEAR_ILMT_4MA,    ///< HDREG current limit is 4mA.
  SL_HAL_EMU_HDREG_STOP_GEAR_CURRENT_LIMIT_8MA    = _EMU_CTRL_HDREGSTOPGEAR_ILMT_8MA,    ///< HDREG current limit is 8mA.
  SL_HAL_EMU_HDREG_STOP_GEAR_CURRENT_LIMIT_12MA   = _EMU_CTRL_HDREGSTOPGEAR_ILMT_12MA,   ///< HDREG current limit is 12mA.
  SL_HAL_EMU_HDREG_STOP_GEAR_CURRENT_LIMIT_16MA   = _EMU_CTRL_HDREGSTOPGEAR_ILMT_16MA,   ///< HDREG current limit is 16mA.
  SL_HAL_EMU_HDREG_STOP_GEAR_CURRENT_LIMIT_24MA   = _EMU_CTRL_HDREGSTOPGEAR_ILMT_24MA,   ///< HDREG current limit is 24mA.
  SL_HAL_EMU_HDREG_STOP_GEAR_CURRENT_LIMIT_48MA   = _EMU_CTRL_HDREGSTOPGEAR_ILMT_48MA,   ///< HDREG current limit is 48mA.
  SL_HAL_EMU_HDREG_STOP_GEAR_CURRENT_LIMIT_64MA   = _EMU_CTRL_HDREGSTOPGEAR_ILMT_64MA,   ///< HDREG current limit is 64mA.
  SL_HAL_EMU_HDREG_STOP_GEAR_CURRENT_LIMIT_Max    = _EMU_CTRL_HDREGSTOPGEAR_ILMT_MAX     ///< HDREG current limit is 64mA.
};
#endif

#if defined(SL_HAL_EMU_DCDC_BUCK_PRESENT) \
  || defined(SL_HAL_EMU_DCDC_BOOST_PRESENT)
/// DCDC mode.
SL_ENUM(sl_hal_emu_dcdc_mode_t) {
  SL_HAL_EMU_DCDC_MODE_BYPASS     = _DCDC_CTRL_MODE_BYPASS,          ///< DCDC regulator bypass.
  SL_HAL_EMU_DCDC_MODE_REGULATION = _DCDC_CTRL_MODE_DCDCREGULATION   ///< DCDC regulator on.
};
#endif

#if defined(SL_HAL_EMU_DCDC_BOOST_PRESENT)
/// DCDC Boost drive speed.
SL_ENUM(sl_hal_emu_dcdc_boost_drive_speed_t) {
  SL_HAL_EMU_DCDC_BOOST_DRIVE_SPEED_BEST_EMI        = _DCDC_BSTEM01CTRL_DRVSPEED_DEFAULT_SETTING,         ///< Recommend no options other than DEFAULT be used here, as there is no benefit.
  SL_HAL_EMU_DCDC_BOOST_DRIVE_SPEED_DEFAULT         = _DCDC_BSTEM01CTRL_DRVSPEED_DEFAULT_SETTING,         ///< Recommend no options other than DEFAULT be used here, as there is no benefit.
  SL_HAL_EMU_DCDC_BOOST_DRIVE_SPEED_INTERMEDIATE    = _DCDC_BSTEM01CTRL_DRVSPEED_DEFAULT_SETTING,         ///< Recommend no options other than DEFAULT be used here, as there is no benefit.
  SL_HAL_EMU_DCDC_BOOST_DRIVE_SPEED_BEST_EFFICIENCY = _DCDC_BSTEM01CTRL_DRVSPEED_DEFAULT_SETTING          ///< Recommend no options other than DEFAULT be used here, as there is no benefit.
};

/// DCDC Boost EM01 peak current setting.
SL_ENUM(sl_hal_emu_dcdc_boost_em01_peak_current_t) {
  SL_HAL_EMU_DCDC_BOOST_EM01_PEAK_CURRENT_LOAD_10MA = _DCDC_BSTEM01CTRL_IPKVAL_Load10mA,   ///< Load 10mA.
  SL_HAL_EMU_DCDC_BOOST_EM01_PEAK_CURRENT_LOAD_11MA = _DCDC_BSTEM01CTRL_IPKVAL_Load11mA,   ///< Load 11mA.
  SL_HAL_EMU_DCDC_BOOST_EM01_PEAK_CURRENT_LOAD_13MA = _DCDC_BSTEM01CTRL_IPKVAL_Load13mA,   ///< Load 13mA.
  SL_HAL_EMU_DCDC_BOOST_EM01_PEAK_CURRENT_LOAD_15MA = _DCDC_BSTEM01CTRL_IPKVAL_Load15mA,   ///< Load 15mA.
  SL_HAL_EMU_DCDC_BOOST_EM01_PEAK_CURRENT_LOAD_16MA = _DCDC_BSTEM01CTRL_IPKVAL_Load16mA,   ///< Load 16mA.
  SL_HAL_EMU_DCDC_BOOST_EM01_PEAK_CURRENT_LOAD_18MA = _DCDC_BSTEM01CTRL_IPKVAL_Load18mA,   ///< Load 18mA.
  SL_HAL_EMU_DCDC_BOOST_EM01_PEAK_CURRENT_LOAD_20MA = _DCDC_BSTEM01CTRL_IPKVAL_Load20mA,   ///< Load 20mA.
  SL_HAL_EMU_DCDC_BOOST_EM01_PEAK_CURRENT_LOAD_21MA = _DCDC_BSTEM01CTRL_IPKVAL_Load21mA,   ///< Load 21mA.
  SL_HAL_EMU_DCDC_BOOST_EM01_PEAK_CURRENT_LOAD_23MA = _DCDC_BSTEM01CTRL_IPKVAL_Load23mA,   ///< Load 23mA.
  SL_HAL_EMU_DCDC_BOOST_EM01_PEAK_CURRENT_LOAD_25MA = _DCDC_BSTEM01CTRL_IPKVAL_Load25mA    ///< Load 25mA.
};

/// DCDC Boost peak current detection maximum timeout.
SL_ENUM(sl_hal_emu_dcdc_boost_ton_max_timeout_t) {
  SL_HAL_EMU_DCDC_BOOST_TON_MAX_TIMEOUT_OFF    = _DCDC_BSTCTRL_IPKTMAXCTRL_OFF,            ///< Ton max off.
  SL_HAL_EMU_DCDC_BOOST_TON_MAX_TIMEOUT_0P35US = _DCDC_BSTCTRL_IPKTMAXCTRL_TMAX_0P35us,    ///< Ton max is 0.35us.
  SL_HAL_EMU_DCDC_BOOST_TON_MAX_TIMEOUT_0P63US = _DCDC_BSTCTRL_IPKTMAXCTRL_TMAX_0P63us,    ///< Ton max is 0.63us.
  SL_HAL_EMU_DCDC_BOOST_TON_MAX_TIMEOUT_0P91US = _DCDC_BSTCTRL_IPKTMAXCTRL_TMAX_0P91us,    ///< Ton max is 0.91us.
  SL_HAL_EMU_DCDC_BOOST_TON_MAX_TIMEOUT_1P19US = _DCDC_BSTCTRL_IPKTMAXCTRL_TMAX_1P19us,    ///< Ton max is 1.19us.
  SL_HAL_EMU_DCDC_BOOST_TON_MAX_TIMEOUT_1P47US = _DCDC_BSTCTRL_IPKTMAXCTRL_TMAX_1P47us,    ///< Ton max is 1.47us.
  SL_HAL_EMU_DCDC_BOOST_TON_MAX_TIMEOUT_1P75US = _DCDC_BSTCTRL_IPKTMAXCTRL_TMAX_1P75us,    ///< Ton max is 1.75us.
  SL_HAL_EMU_DCDC_BOOST_TON_MAX_TIMEOUT_2P03US = _DCDC_BSTCTRL_IPKTMAXCTRL_TMAX_2P03us     ///< Ton max is 2.03us.
};

/// DCDC Boost EM23 peak current setting.
SL_ENUM(sl_hal_emu_dcdc_boost_em23_peak_current_t) {
  SL_HAL_EMU_DCDC_BOOST_EM23_PEAK_CURRENT_LOAD_10MA = _DCDC_BSTEM23CTRL_IPKVAL_Load10mA,  ///< Load 10mA.
};

#if defined(_DCDC_CTRL_DVDDBSTPRG_MASK)
/// DCDC Boost output voltage.
SL_ENUM(sl_hal_emu_dcdc_boost_output_voltage_t) {
  SL_HAL_EMU_DCDC_BOOST_OUTPUT_VOLTAGE_1V8 = _DCDC_CTRL_DVDDBSTPRG_BOOST_1V8,     ///< Output voltage is 1.8V.
  SL_HAL_EMU_DCDC_BOOST_OUTPUT_VOLTAGE_1V9 = _DCDC_CTRL_DVDDBSTPRG_BOOST_1V9,     ///< Output voltage is 1.9V.
  SL_HAL_EMU_DCDC_BOOST_OUTPUT_VOLTAGE_2V0 = _DCDC_CTRL_DVDDBSTPRG_BOOST_2V,      ///< Output voltage is 2.0V.
  SL_HAL_EMU_DCDC_BOOST_OUTPUT_VOLTAGE_2V1 = _DCDC_CTRL_DVDDBSTPRG_BOOST_2V1,     ///< Output voltage is 2.1V.
  SL_HAL_EMU_DCDC_BOOST_OUTPUT_VOLTAGE_2V2 = _DCDC_CTRL_DVDDBSTPRG_BOOST_2V2,     ///< Output voltage is 2.2V.
  SL_HAL_EMU_DCDC_BOOST_OUTPUT_VOLTAGE_2V3 = _DCDC_CTRL_DVDDBSTPRG_BOOST_2V3,     ///< Output voltage is 2.3V.
  SL_HAL_EMU_DCDC_BOOST_OUTPUT_VOLTAGE_2V4 = _DCDC_CTRL_DVDDBSTPRG_BOOST_2V4      ///< Output voltage is 2.4V.
};
#endif
#endif /* defined(SL_HAL_EMU_DCDC_BOOST_PRESENT) */

#if defined(SL_HAL_EMU_DCDC_BUCK_PRESENT)
/// VREGIN comparator threshold.
SL_ENUM(sl_hal_emu_vregin_cmp_threshold_t) {
  SL_HAL_EMU_VREGIN_CMP_THRESHOLD_2V0 = 0,   ///< Comparator threshold is 2.0V.
  SL_HAL_EMU_VREGIN_CMP_THRESHOLD_2V1 = 1,   ///< Comparator threshold is 2.1V.
  SL_HAL_EMU_VREGIN_CMP_THRESHOLD_2V2 = 2,   ///< Comparator threshold is 2.2V.
  SL_HAL_EMU_VREGIN_CMP_THRESHOLD_2V3 = 3    ///< Comparator threshold is 2.3V.
};

/// DCDC Buck Ton max timeout.
SL_ENUM(sl_hal_emu_dcdc_ton_max_timeout_t) {
#if defined(_DCDC_CTRL_IPKTMAXCTRL_OFF)
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_OFF     = _DCDC_CTRL_IPKTMAXCTRL_OFF,             ///< Ton max off.
#endif
#if defined(_DCDC_CTRL_IPKTMAXCTRL_TMAX_0P35us)
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P35US  = _DCDC_CTRL_IPKTMAXCTRL_TMAX_0P35us,     ///< Ton max is 0.35us.
#endif
#if defined(_DCDC_CTRL_IPKTMAXCTRL_TMAX_0P63us)
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P63US  = _DCDC_CTRL_IPKTMAXCTRL_TMAX_0P63us,     ///< Ton max is 0.63us.
#endif
#if defined(_DCDC_CTRL_IPKTMAXCTRL_TMAX_0P91us)
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P91US  = _DCDC_CTRL_IPKTMAXCTRL_TMAX_0P91us,     ///< Ton max is 0.91us.
#endif
#if defined(_DCDC_CTRL_IPKTMAXCTRL_TMAX_1P19us)
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P19US  = _DCDC_CTRL_IPKTMAXCTRL_TMAX_1P19us,     ///< Ton max is 1.19us.
#endif
#if defined(_DCDC_CTRL_IPKTMAXCTRL_TMAX_1P47us)
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P47US  = _DCDC_CTRL_IPKTMAXCTRL_TMAX_1P47us,     ///< Ton max is 1.47us.
#endif
#if defined(_DCDC_CTRL_IPKTMAXCTRL_TMAX_1P75us)
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P75US  = _DCDC_CTRL_IPKTMAXCTRL_TMAX_1P75us,     ///< Ton max is 1.75us.
#endif
#if defined(_DCDC_CTRL_IPKTMAXCTRL_TMAX_2P03us)
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_2P03US  = _DCDC_CTRL_IPKTMAXCTRL_TMAX_2P03us      ///< Ton max is 2.03us.
#endif
#if !defined(_DCDC_CTRL_IPKTMAXCTRL_OFF) && !defined(_DCDC_CTRL_IPKTMAXCTRL_TMAX_0P35us)          \
  && !defined(_DCDC_CTRL_IPKTMAXCTRL_TMAX_0P63us) && !defined(_DCDC_CTRL_IPKTMAXCTRL_TMAX_0P91us) \
  && !defined(_DCDC_CTRL_IPKTMAXCTRL_TMAX_1P19us) && !defined(_DCDC_CTRL_IPKTMAXCTRL_TMAX_1P47us) \
  && !defined(_DCDC_CTRL_IPKTMAXCTRL_TMAX_1P75us) && !defined(_DCDC_CTRL_IPKTMAXCTRL_TMAX_2P03us)
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_OFF     = 0,                                      ///< Ton max off.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P14US  = 1,                                      ///< Ton max is 0.14us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P21US  = 2,                                      ///< Ton max is 0.21us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P28US  = 3,                                      ///< Ton max is 0.28us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P35US  = 4,                                      ///< Ton max is 0.35us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P42US  = 5,                                      ///< Ton max is 0.42us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P49US  = 6,                                      ///< Ton max is 0.49us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P56US  = 7,                                      ///< Ton max is 0.56us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P63US  = 8,                                      ///< Ton max is 0.63us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P70US  = 9,                                      ///< Ton max is 0.70us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P77US  = 10,                                     ///< Ton max is 0.77us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P84US  = 11,                                     ///< Ton max is 0.84us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P91US  = 12,                                     ///< Ton max is 0.91us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_0P98US  = 13,                                     ///< Ton max is 0.98us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P05US  = 14,                                     ///< Ton max is 1.05us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P12US  = 15,                                     ///< Ton max is 1.12us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P19US  = 16,                                     ///< Ton max is 1.19us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P26US  = 17,                                     ///< Ton max is 1.26us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P33US  = 18,                                     ///< Ton max is 1.33us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P40US  = 19,                                     ///< Ton max is 1.40us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P47US  = 20,                                     ///< Ton max is 1.47us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P54US  = 21,                                     ///< Ton max is 1.54us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P61US  = 22,                                     ///< Ton max is 1.61us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P68US  = 23,                                     ///< Ton max is 1.68us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P75US  = 24,                                     ///< Ton max is 1.75us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P82US  = 25,                                     ///< Ton max is 1.82us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P89US  = 26,                                     ///< Ton max is 1.89us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P96US  = 27,                                     ///< Ton max is 1.96us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_2P03US  = 28,                                     ///< Ton max is 2.03us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_2P10US  = 29,                                     ///< Ton max is 2.10us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_2P17US  = 30,                                     ///< Ton max is 2.17us.
  SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_2P24US  = 31                                      ///< Ton max is 2.24us.
  #endif
};

/// DCDC Buck drive speed.
SL_ENUM(sl_hal_emu_dcdc_drive_speed_t) {
  SL_HAL_EMU_DCDC_DRIVE_SPEED_BEST_EMI        = _DCDC_EM01CTRL0_DRVSPEED_DEFAULT_SETTING,  ///< Recommend no options other than DEFAULT be used here, as there is no benefit.
  SL_HAL_EMU_DCDC_DRIVE_SPEED_DEFAULT         = _DCDC_EM01CTRL0_DRVSPEED_DEFAULT_SETTING,  ///< Recommend no options other than DEFAULT be used here, as there is no benefit.
  SL_HAL_EMU_DCDC_DRIVE_SPEED_INTERMEDIATE    = _DCDC_EM01CTRL0_DRVSPEED_DEFAULT_SETTING,  ///< Recommend no options other than DEFAULT be used here, as there is no benefit.
  SL_HAL_EMU_DCDC_DRIVE_SPEED_BEST_EFFICIENCY = _DCDC_EM01CTRL0_DRVSPEED_DEFAULT_SETTING   ///< Recommend no options other than DEFAULT be used here, as there is no benefit.
};

/// DCDC Buck peak current setting.
SL_ENUM(sl_hal_emu_dcdc_peak_current_t) {
#if defined(_DCDC_EM23CTRL0_IPKVAL_Load5mA)
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_5MA     = _DCDC_EM23CTRL0_IPKVAL_Load5mA,    ///< Load 5mA, peak current 90mA.
#endif
#if defined(_DCDC_EM23CTRL0_IPKVAL_LOAD5MA)
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_5MA     = _DCDC_EM23CTRL0_IPKVAL_LOAD5MA,    ///< Load 5mA, peak current 90mA.
#endif
#if defined(_DCDC_EM23CTRL0_IPKVAL_Load10mA)
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_10MA    = _DCDC_EM23CTRL0_IPKVAL_Load10mA,   ///< Load 10mA, peak current 150mA.
#endif
#if defined(_DCDC_EM23CTRL0_IPKVAL_LOAD10MA)
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_10MA    = _DCDC_EM23CTRL0_IPKVAL_LOAD10MA,   ///< Load 10mA, peak current 150mA.
#endif
#if defined(_DCDC_EM01CTRL0_IPKVAL_Load28mA)
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_28MA    = _DCDC_EM01CTRL0_IPKVAL_Load28mA,   ///< Load 28mA, peak current 70mA.
#endif
#if defined(_DCDC_EM01CTRL0_IPKVAL_Load32mA)
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_32MA    = _DCDC_EM01CTRL0_IPKVAL_Load32mA,   ///< Load 32mA, peak current 80mA.
#endif
#if defined(_DCDC_EM01CTRL0_IPKVAL_Load36mA)
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_36MA    = _DCDC_EM01CTRL0_IPKVAL_Load36mA,   ///< Load 36mA, peak current 90mA.
#endif
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_40MA    = _DCDC_EM01CTRL0_IPKVAL_Load40mA,   ///< Load 40mA, peak current 100mA.
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_44MA    = _DCDC_EM01CTRL0_IPKVAL_Load44mA,   ///< Load 44mA, peak current 110mA.
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_48MA    = _DCDC_EM01CTRL0_IPKVAL_Load48mA,   ///< Load 48mA, peak current 120mA.
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_52MA    = _DCDC_EM01CTRL0_IPKVAL_Load52mA,   ///< Load 52mA, peak current 130mA.
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_56MA    = _DCDC_EM01CTRL0_IPKVAL_Load56mA,   ///< Load 56mA, peak current 140mA.
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_60MA    = _DCDC_EM01CTRL0_IPKVAL_Load60mA,   ///< Load 60mA, peak current 150mA.
#if defined(_DCDC_EM01CTRL0_IPKVAL_Load64mA)
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_64MA    = _DCDC_EM01CTRL0_IPKVAL_Load64mA,   ///< Load 64mA, peak current 160mA.
#endif
#if defined(_DCDC_EM01CTRL0_IPKVAL_Load68mA)
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_68MA    = _DCDC_EM01CTRL0_IPKVAL_Load68mA,   ///< Load 68mA, peak current 170mA.
#endif
#if defined(_DCDC_EM01CTRL0_IPKVAL_Load72mA)
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_72MA    = _DCDC_EM01CTRL0_IPKVAL_Load72mA,   ///< Load 72mA, peak current 180mA.
#endif
#if defined(_DCDC_EM01CTRL0_IPKVAL_Load76mA)
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_76MA    = _DCDC_EM01CTRL0_IPKVAL_Load76mA,   ///< Load 76mA, peak current 190mA.
#endif
#if defined(_DCDC_EM01CTRL0_IPKVAL_Load80mA)
  SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_80MA    = _DCDC_EM01CTRL0_IPKVAL_Load80mA    ///< Load 80mA, peak current 200mA.
#endif
};
#endif /* defined(SL_HAL_EMU_DCDC_BUCK_PRESENT) */

#if defined(_DCDC_DOCTRL_MASK)
/// DCDC Dual Output regulation type.
SL_ENUM(sl_hal_emu_dcdc_regulation_type_t) {
  SL_HAL_EMU_DCDC_REGULATION_TYPE_REGDVDD     = _DCDC_DOCTRL_REGULATIONTYPE_REGDVDD,    ///< Regulate DVDD only.
  SL_HAL_EMU_DCDC_REGULATION_TYPE_REGDEC      = _DCDC_DOCTRL_REGULATIONTYPE_REGDEC,     ///< Regulate DEC only.
  SL_HAL_EMU_DCDC_REGULATION_TYPE_REGDVDDDEC  = _DCDC_DOCTRL_REGULATIONTYPE_REGDVDDDEC, ///< Regulate both DVDD and DEC.
};
#endif

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

#if defined(SL_HAL_EMU_DCDC_BOOST_PRESENT)
/// DCDC Boost regulator initialization structure.
typedef struct {
  sl_hal_emu_dcdc_boost_ton_max_timeout_t       ton_max;                     ///< Ton max timeout control.
  bool                                          external_shutdown_enable;    ///< true = disable DCDC boost mode with BOOST_EN=0.
  sl_hal_emu_dcdc_boost_drive_speed_t           drive_speed_em01;            ///< DCDC drive speed in EM0/1.
  sl_hal_emu_dcdc_boost_drive_speed_t           drive_speed_em23;            ///< DCDC drive speed in EM2/3.
  sl_hal_emu_dcdc_boost_em01_peak_current_t     peak_current_em01;           ///< EM0/1 peak current setting.
  sl_hal_emu_dcdc_boost_em23_peak_current_t     peak_current_em23;           ///< EM2/3 peak current setting.
#if defined(_DCDC_CTRL_DVDDBSTPRG_MASK)
  sl_hal_emu_dcdc_boost_output_voltage_t        output_voltage;              ///< DCDC Boost output voltage.
#endif
} sl_hal_emu_dcdc_boost_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_emu_dcdc_boost_init_t sl_hal_emu_dcdc_boost_config_t;
/** @endcond */
#endif /* defined(SL_HAL_EMU_DCDC_BOOST_PRESENT) */

#if defined(SL_HAL_EMU_DCDC_BUCK_PRESENT)
/// DCDC regulator initialization structure.
typedef struct {
  sl_hal_emu_dcdc_mode_t                mode;                    ///< DCDC mode.
#if defined(_DCDC_DOCTRL_MASK)
  sl_hal_emu_dcdc_regulation_type_t     regulation_type;         ///< DCDC regulation type.
#endif
  sl_hal_emu_vregin_cmp_threshold_t     comparator_threshold;    ///< VREGIN comparator threshold.
  sl_hal_emu_dcdc_ton_max_timeout_t     ton_max;                 ///< Ton max timeout control.
#if defined(_DCDC_CTRL_DCMONLYEN_MASK)
  bool                                  dcm_only_enable;         ///< DCM only mode enable.
#endif
  sl_hal_emu_dcdc_drive_speed_t         drive_speed_em01;        ///< DCDC drive speed in EM0/1.
  sl_hal_emu_dcdc_drive_speed_t         drive_speed_em23;        ///< DCDC drive speed in EM2/3.
  sl_hal_emu_dcdc_peak_current_t        peak_current_em01;       ///< EM0/1 peak current setting.
  sl_hal_emu_dcdc_peak_current_t        peak_current_em23;       ///< EM2/3 peak current setting.
} sl_hal_emu_dcdc_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_emu_dcdc_init_t sl_hal_emu_dcdc_config_t;
/** @endcond */
#endif /* defined(SL_HAL_EMU_DCDC_BUCK_PRESENT) */

#if defined(SL_HAL_EMU_DCDC_BOOST_PRESENT)
/// Default DCDC Boost initialization.
#if defined(_DCDC_CTRL_DVDDBSTPRG_MASK)
#define SL_HAL_EMU_DCDC_BOOST_INIT_DEFAULT                                                                  \
  {                                                                                                         \
    SL_HAL_EMU_DCDC_BOOST_TON_MAX_TIMEOUT_1P19US,            /*< Ton max is 1.19us. */                      \
    true,                                                    /*< disable DCDC boost mode with BOOST_EN=0 */ \
    SL_HAL_EMU_DCDC_BOOST_DRIVE_SPEED_DEFAULT,               /*< Default efficiency in EM0/1. */            \
    SL_HAL_EMU_DCDC_BOOST_DRIVE_SPEED_DEFAULT,               /*< Default efficiency in EM2/3. */            \
    SL_HAL_EMU_DCDC_BOOST_EM01_PEAK_CURRENT_LOAD_23MA,       /*< Default peak current in EM0/1. */          \
    SL_HAL_EMU_DCDC_BOOST_EM23_PEAK_CURRENT_LOAD_10MA,       /*< Default peak current in EM2/3. */          \
    SL_HAL_EMU_DCDC_BOOST_OUTPUT_VOLTAGE_1V8                 /*< DCDC Boost output voltage. */              \
  }
#else
#define SL_HAL_EMU_DCDC_BOOST_INIT_DEFAULT                                                                  \
  {                                                                                                         \
    SL_HAL_EMU_DCDC_BOOST_TON_MAX_TIMEOUT_1P19US,            /*< Ton max is 1.19us. */                      \
    true,                                                    /*< disable DCDC boost mode with BOOST_EN=0 */ \
    SL_HAL_EMU_DCDC_BOOST_DRIVE_SPEED_DEFAULT,               /*< Default efficiency in EM0/1. */            \
    SL_HAL_EMU_DCDC_BOOST_DRIVE_SPEED_DEFAULT,               /*< Default efficiency in EM2/3. */            \
    SL_HAL_EMU_DCDC_BOOST_EM01_PEAK_CURRENT_LOAD_23MA,       /*< Default peak current in EM0/1. */          \
    SL_HAL_EMU_DCDC_BOOST_EM23_PEAK_CURRENT_LOAD_10MA        /*< Default peak current in EM2/3. */          \
  }
#endif
#endif /* defined(SL_HAL_EMU_DCDC_BOOST_PRESENT) */

#if defined(SL_HAL_EMU_DCDC_BUCK_PRESENT)
/// Default DCDC Buck initialization.
#if defined(_DCDC_CTRL_DCMONLYEN_MASK)
#if defined(_DCDC_DOCTRL_MASK)
#define SL_HAL_EMU_DCDC_INIT_DEFAULT                                                       \
  {                                                                                        \
    SL_HAL_EMU_DCDC_MODE_REGULATION,              /*< DCDC regulator on. */                \
    SL_HAL_EMU_DCDC_REGULATION_TYPE_REGDVDD,      /*< Regulation type is DVDD. */          \
    SL_HAL_EMU_VREGIN_CMP_THRESHOLD_2V3,          /*< 2.3V VREGIN comparator threshold. */ \
    SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P19US,       /*< Ton max is 1.19us. */                \
    true,                                         /*< Enable DCM only mode. */             \
    SL_HAL_EMU_DCDC_DRIVE_SPEED_DEFAULT,          /*< Default efficiency in EM0/1. */      \
    SL_HAL_EMU_DCDC_DRIVE_SPEED_DEFAULT,          /*< Default efficiency in EM2/3. */      \
    SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_60MA,       /*< Default peak current in EM0/1. */    \
    SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_5MA         /*< Default peak current in EM2/3. */    \
  }
#else
#define SL_HAL_EMU_DCDC_INIT_DEFAULT                                                       \
  {                                                                                        \
    SL_HAL_EMU_DCDC_MODE_REGULATION,              /*< DCDC regulator on. */                \
    SL_HAL_EMU_VREGIN_CMP_THRESHOLD_2V3,          /*< 2.3V VREGIN comparator threshold. */ \
    SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P19US,       /*< Ton max is 1.19us. */                \
    true,                                         /*< Enable DCM only mode. */             \
    SL_HAL_EMU_DCDC_DRIVE_SPEED_DEFAULT,          /*< Default efficiency in EM0/1. */      \
    SL_HAL_EMU_DCDC_DRIVE_SPEED_DEFAULT,          /*< Default efficiency in EM2/3. */      \
    SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_60MA,       /*< Default peak current in EM0/1. */    \
    SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_5MA         /*< Default peak current in EM2/3. */    \
  }
#endif
#else
#if defined(_DCDC_DOCTRL_MASK)
#define SL_HAL_EMU_DCDC_INIT_DEFAULT                                                       \
  {                                                                                         \
    SL_HAL_EMU_DCDC_MODE_REGULATION,               /*< DCDC regulator on. */                \
    SL_HAL_EMU_DCDC_REGULATION_TYPE_REGDVDD,       /*< Regulation type is DVDD. */           \
    SL_HAL_EMU_VREGIN_CMP_THRESHOLD_2V3,           /*< 2.3V VREGIN comparator threshold. */ \
    SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P19US,        /*< Ton max is 1.19us. */                \
    SL_HAL_EMU_DCDC_DRIVE_SPEED_DEFAULT,           /*< Default efficiency in EM0/1. */      \
    SL_HAL_EMU_DCDC_DRIVE_SPEED_DEFAULT,           /*< Default efficiency in EM2/3. */      \
    SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_60MA,        /*< Default peak current in EM0/1. */    \
    SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_5MA          /*< Default peak current in EM2/3. */    \
  }
#else
#define SL_HAL_EMU_DCDC_INIT_DEFAULT                                                       \
  {                                                                                         \
    SL_HAL_EMU_DCDC_MODE_REGULATION,               /*< DCDC regulator on. */                \
    SL_HAL_EMU_VREGIN_CMP_THRESHOLD_2V3,           /*< 2.3V VREGIN comparator threshold. */ \
    SL_HAL_EMU_DCDC_TON_MAX_TIMEOUT_1P19US,        /*< Ton max is 1.19us. */                \
    SL_HAL_EMU_DCDC_DRIVE_SPEED_DEFAULT,           /*< Default efficiency in EM0/1. */      \
    SL_HAL_EMU_DCDC_DRIVE_SPEED_DEFAULT,           /*< Default efficiency in EM2/3. */      \
    SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_60MA,        /*< Default peak current in EM0/1. */    \
    SL_HAL_EMU_DCDC_PEAK_CURRENT_LOAD_5MA          /*< Default peak current in EM2/3. */    \
  }
#endif
#endif
#endif /* defined(SL_HAL_EMU_DCDC_BUCK_PRESENT) */

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Alias for deprecated macro names used for backward compatibility purposes.
#define EMU_DCDCBOOSTINIT_DEFAULT     SL_HAL_EMU_DCDC_BOOST_INIT_DEFAULT
#define EMU_DCDCINIT_DEFAULT          SL_HAL_EMU_DCDC_INIT_DEFAULT
/** @endcond */

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Power down RAM memory blocks in EM2/EM3.
 *
 * @details
 *   This function will power down in EM2/EM3 all the RAM blocks that are within
 *   a given range. The RAM block layout is different between device families,
 *   so this function can be used in a generic way to power down a RAM memory
 *   region which is known to be unused.
 *
 *   This function will power down blocks from start to the end of RAM. For xg27,
 *   it will shut off blocks which are completely enclosed by the memory range
 *   given by [start, end].
 *
 *   This is an example to power down all RAM blocks except the first
 *   one. The first RAM block is special in that it cannot be powered down
 *   by the hardware. The size of the first RAM block is device-specific.
 *   See the reference manual to find the RAM block sizes.
 *
 * @code sl_hal_emu_ram_power_down(SRAM_BASE, SRAM_BASE + SRAM_SIZE);
 * @endcode
 *
 * @note
 *   On Series 3 devices, this function does nothing as the memory manager
 *   automatically manages the retention of memory blocks in EM2/EM3.
 *
 * @param[in] start
 *   The start address of the RAM region to power down
 *   This address is inclusive.
 *
 * @param[in] end
 *   The end address of the RAM region to power down. Except for xg27
 *   It can only have two values: 0 or more than RAM0_END. Any other
 *   valid RAM address will just do nothing without any error or
 *   indication that nothing happened. This address is exclusive.
 *   If this parameter is 0, all RAM blocks contained in the region
 *   from start to the end of RAM will be powered down.
 ******************************************************************************/
void sl_hal_emu_ram_power_down(uint32_t start,
                               uint32_t end);

/***************************************************************************//**
 * @brief
 *   Power up all available RAM memory blocks in EM2/EM3.
 *
 * @details
 *   This function will power up all the RAM blocks on a device
 *   this means that the RAM blocks are retained in EM2/EM3.
 *
 * @note
 *   On Series 3 devices, this function does nothing as the memory manager
 *   automatically manages the retention of memory blocks in EM2/EM3.
 ******************************************************************************/
void sl_hal_emu_ram_power_up(void);

#if (defined(SL_HAL_EMU_DCDC_BUCK_PRESENT) \
  || defined(SL_HAL_EMU_DCDC_BOOST_PRESENT))
/***************************************************************************//**
 * @brief
 *   Set DCDC regulator operating mode.
 *
 * @param[in] dcdc_mode
 *   DCDC mode.
 *
 * @return
 *   Returns the status of the DCDC mode set operation.
 *
 * @note
 *   Make sure the DCDC is unlocked before calling this function.
 *
 * @verbatim
 *   SL_STATUS_OK - Operation completed successfully.
 *   SL_STATUS_TIMEOUT - Operation EMU DCDC set mode timeout.
 * @endverbatim
 ******************************************************************************/
sl_status_t sl_hal_emu_set_dcdc_mode(sl_hal_emu_dcdc_mode_t dcdc_mode);

/***************************************************************************//**
 * Indicate that the DCDC peripheral bus clock enable has changed allowing
 * RAIL to react accordingly.
 *
 * @details
 *   This function is called after DCDC has been enabled or disabled.
 *   The function implementation does not perform anything, but it is
 *   SL_WEAK so that it can use the RAIL version if needed.
 ******************************************************************************/
void sl_hal_emu_dcdc_updated_hook(void);
#endif /* (defined(SL_HAL_EMU_DCDC_BUCK_PRESENT) || defined(SL_HAL_EMU_DCDC_BOOST_PRESENT)) */

#if defined(SL_HAL_EMU_DCDC_BOOST_PRESENT)
/***************************************************************************//**
 * @brief
 *   Configure the DCDC Boost regulator.
 *
 * @param[in] init
 *   The DCDC initialization structure.
 *
 * @return
 *   True if initialization parameters are valid.
 *
 * @note
 *   Make sure the DCDC is enabled and unlocked before calling this function.
 ******************************************************************************/
void sl_hal_emu_init_dcdc_boost(const sl_hal_emu_dcdc_boost_init_t *init);

/***************************************************************************//**
 * @brief
 *   Set EM01 mode Boost Peak Current setting.
 *
 * @param[in] boost_peak_current_em01
 *   Boost Peak load current coefficient in EM01 mode.
 *
 * @note
 *   Make sure the DCDC is unlocked before calling this function.
 ******************************************************************************/
void sl_hal_emu_set_em01_boost_peak_current(const sl_hal_emu_dcdc_boost_em01_peak_current_t boost_peak_current_em01);

#if defined(_DCDC_CTRL_DVDDBSTPRG_MASK)
/***************************************************************************//**
 * @brief
 *   Set DCDC Boost output voltage.
 *
 * @param[in] boost_output_voltage
 *   Boost output voltage.
 *
 * @note
 *   Make sure the DCDC is unlocked before calling this function.
 ******************************************************************************/
void sl_hal_emu_set_dcdc_boost_output_voltage(const sl_hal_emu_dcdc_boost_output_voltage_t boost_output_voltage);
#endif
#endif /* defined(SL_HAL_EMU_DCDC_BOOST_PRESENT) */

#if defined(SL_HAL_EMU_DCDC_BUCK_PRESENT)
/***************************************************************************//**
 * @brief
 *   Configure the DCDC regulator.
 *
 * @param[in] init
 *   The DCDC initialization structure.
 *
 * @return
 *   True if initialization parameters are valid.
 *
 * @note
 *   Make sure the DCDC is enabled and unlocked before calling this function.
 ******************************************************************************/
void sl_hal_emu_init_dcdc(const sl_hal_emu_dcdc_init_t *init);

/***************************************************************************//**
 * @brief
 *   Power off the DCDC regulator.
 *
 * @return
 *   SL_STATUS_OK if the DCDC regulator is powered off successfully.
 *   SL_STATUS_TIMEOUT if the DCDC regulator power off operation timed out.
 ******************************************************************************/
sl_status_t sl_hal_emu_dcdc_power_off(void);

/***************************************************************************//**
 * @brief
 *   Set EMO1 mode Peak Current setting.
 *
 * @param[in] peak_current_em01
 *   Peak load current coefficient in EM01 mode.
 *
 * @note
 *   Make sure the DCDC is enabled and unlocked before calling this function.
 ******************************************************************************/
void sl_hal_emu_set_em01_peak_current(const sl_hal_emu_dcdc_peak_current_t peak_current_em01);

#if defined(_DCDC_PFMXCTRL_IPKVAL_MASK)
/***************************************************************************//**
 * @brief
 *   Set PFMX mode Peak Current setting.
 *
 * @param[in] value
 *   Peak load current coefficient in PFMX mode.
 *
 * @note
 *   Make sure the DCDC is enabled and unlocked before calling this function.
 ******************************************************************************/
void sl_hal_emu_set_dcdc_pfmx_mode_peak_current(uint32_t value);
#endif

#if defined(_DCDC_PFMXCTRL_IPKTMAXCTRL_MASK)
/***************************************************************************//**
 * @brief
 *   Set Ton max timeout control.
 *
 * @param[in] value
 *   Maximum time for peak current detection.
 *
 * @note
 *   Make sure the DCDC is unlocked before calling this function.
 ******************************************************************************/
void sl_hal_emu_set_dcdc_pfmx_timeout_max_control(sl_hal_emu_dcdc_ton_max_timeout_t value);
#endif
#endif /* defined(SL_HAL_EMU_DCDC_BUCK_PRESENT) */

#if defined(_DCDC_DOCTRL_REGULATIONTYPE_MASK)
/***************************************************************************//**
 * @brief
 *   Set DCDC dual-output regulation type.
 *
 * @note
 *   The DCDC bus clock must be enabled before calling this function.
 *   Call sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_DCDC) first.
 *
 * @param[in] regulationType
 *   Regulation type selection:
 *   - SL_HAL_EMU_DCDC_REGULATION_TYPE_REGDVDD: Regulate DVDD only
 *   - SL_HAL_EMU_DCDC_REGULATION_TYPE_REGDEC: Regulate DEC only
 *   - SL_HAL_EMU_DCDC_REGULATION_TYPE_REGDVDDDEC: Regulate both DVDD and DEC
 ******************************************************************************/
void sl_hal_emu_dcdc_set_regulation_type(sl_hal_emu_dcdc_regulation_type_t regulation_type);

/***************************************************************************//**
 * @brief
 *   Get DCDC dual-output regulation type.
 *
 * @note
 *   The DCDC bus clock must be enabled before calling this function.
 *   Call sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_DCDC) first.
 *
 * @return
 *   Current regulation type setting.
 ******************************************************************************/
sl_hal_emu_dcdc_regulation_type_t sl_hal_emu_dcdc_get_regulation_type(void);
#endif /* defined(_DCDC_DOCTRL_REGULATIONTYPE_MASK)*/

#if defined(_DCDC_DOCTRL_DUALIPKEN_MASK)
/***************************************************************************//**
 * @brief
 *   Enable DCDC dual IPK DAC mode.
 *
 * @details
 *   When enabled, allows independent current limiting for DVDD and DEC outputs
 *   using separate IPK DAC values. This function controls the
 *   DOCTRL.DUALIPKEN bit.
 *
 * @note
 *   The DCDC bus clock must be enabled before calling this function.
 *   Call sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_DCDC) first.
 ******************************************************************************/
void sl_hal_emu_dcdc_dual_ipk_enable(void);

/***************************************************************************//**
 * @brief
 *   Disable DCDC dual IPK DAC mode.
 *
 * @details
 *   When disabled, the DCDC uses a single IPK DAC value for current limiting,
 *   shared between DVDD and DEC outputs. This function clears the
 *   DOCTRL.DUALIPKEN bit.
 *
 * @note
 *   The DCDC bus clock must be enabled before calling this function.
 *   Call sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_DCDC) first.
 ******************************************************************************/
void sl_hal_emu_dcdc_dual_ipk_disable(void);

/***************************************************************************//**
 * @brief
 *   Get DCDC dual IPK DAC enable status.
 *
 * @details
 *   Returns the current dual IPK DAC enable status from the
 *   DOCTRL.DUALIPKEN bit.
 *
 * @note
 *   The DCDC bus clock must be enabled before calling this function.
 *   Call sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_DCDC) first.
 *
 * @return
 *   True if dual IPK DAC mode is enabled, false otherwise.
 ******************************************************************************/
bool sl_hal_emu_dcdc_get_dual_ipk_enable(void);
#endif /* defined(_DCDC_DOCTRL_DUALIPKEN_MASK)*/

/***************************************************************************//**
 * @brief
 *   Lock EMU registers in order to protect them against unintended modification.
 *
 * @note
 *   If locking EMU registers, they must be unlocked prior to using any
 *   EMU API functions modifying EMU registers, excluding interrupt control
 *   and regulator control if the architecture has a EMU_PWRCTRL register.
 ******************************************************************************/
__INLINE void sl_hal_emu_lock(void)
{
  EMU->LOCK = 0x0;
}

/***************************************************************************//**
 * @brief
 *   Unlock the EMU so that writing to locked registers again is possible.
 ******************************************************************************/
__INLINE void sl_hal_emu_unlock(void)
{
  EMU->LOCK = EMU_LOCK_LOCKKEY_UNLOCK;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more EMU interrupts.
 *
 * @param[in] flags
 *   EMU interrupt sources to enable. Use one or more valid
 *   interrupt flags for the EMU module (EMU_IEN_nnn).
 *
 * @note
 *   Depending on the use, a pending interrupt may already be set prior to
 *   enabling the interrupt. To ignore a pending interrupt, consider using
 *   sl_hal_emu_clear_interrupts() prior to enabling the interrupt.
 ******************************************************************************/
__INLINE void sl_hal_emu_enable_interrupts(uint32_t flags)
{
  EMU->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more EMU interrupts.
 *
 * @param[in] flags
 *   EMU interrupt sources to disable. Use one or more valid
 *   interrupt flags for the EMU module (EMU_IEN_nnn).
 ******************************************************************************/
__INLINE void sl_hal_emu_disable_interrupts(uint32_t flags)
{
  EMU->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Clear one or more pending EMU interrupts.
 *
 * @param[in] flags
 *   Pending EMU interrupt sources to clear.
 *   Use one or more valid interrupt flags for the
 *   EMU module EMU_IF_nnn.
 ******************************************************************************/
__INLINE void sl_hal_emu_clear_interrupts(uint32_t flags)
{
  EMU->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending EMU interrupt flags.
 *
 * @return
 *    EMU interrupt sources pending. Returns one or more valid
 *    interrupt flags for the EMU module (EMU_IF_nnn).
 *
 * @note
 *   Event bits are not cleared by the use of this function.
 ******************************************************************************/
__INLINE uint32_t sl_hal_emu_get_pending_interrupts(void)
{
  return EMU->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending EMU interrupt flags.
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @return
 *   Pending and enabled EMU interrupt sources
 *   Return value is the bitwise AND of
 *   - the enabled interrupt sources in EMU_IEN and
 *   - the pending interrupt flags EMU_IF.
 *
 * @note
 *   Interrupt flags are not cleared by the use of this function.
 ******************************************************************************/
__INLINE uint32_t sl_hal_emu_get_enabled_pending_interrupts(void)
{
  uint32_t interrupt_flag = EMU->IF;
  uint32_t enabled_interrupt = EMU->IEN;

  return interrupt_flag & enabled_interrupt;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending EMU interrupts.
 *
 * @param[in] flags
 *   EMU interrupt sources to set to pending. Use one or more
 *   valid interrupt flags for the EMU module (EMU_IFS_nnn).
 ******************************************************************************/
__INLINE void sl_hal_emu_set_interrupts(uint32_t flags)
{
  EMU->IF_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get EMU STATUS register.
 *
 * @return
 *   STATUS register value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_emu_get_status(void)
{
  return EMU->STATUS;
}

/***************************************************************************//**
 * @brief
 *   Temperature measurement ready status.
 *
 * @return
 *   True if temperature measurement is ready.
 ******************************************************************************/
__INLINE bool sl_hal_emu_is_temperature_ready(void)
{
  return EMU->STATUS & EMU_STATUS_FIRSTTEMPDONE;
}

#if defined(_EMU_TEMP_TEMPAVG_MASK)
/***************************************************************************//**
 * @brief
 *   Get averaged temperature in degrees Celsius.
 *
 * @return
 *   Averaged temperature.
 *
 * @note
 *   An averaged temperature measurement must first be requested by calling
 *   @ref sl_hal_emu_get_averaged_temperature() and waiting for the TEMPAVG
 *   interrupt flag to go high.
 ******************************************************************************/
__INLINE float sl_hal_emu_get_averaged_temperature(void)
{
  return ((float) ((EMU->TEMP & _EMU_TEMP_TEMPAVG_MASK)
                   >> _EMU_TEMP_TEMPAVG_SHIFT)
          ) / 4.0f - EMU_TEMP_ZERO_C_IN_KELVIN;
}

/***************************************************************************//**
 * @brief
 *   Request averaged temperature.
 *
 * @param[in] num_samples
 *   Number of temperature samples to average.
 ******************************************************************************/
__INLINE void sl_hal_emu_request_averaged_temperature(sl_hal_emu_temperature_average_num_t num_samples)
{
  EMU->CTRL_CLR = _EMU_CTRL_TEMPAVGNUM_MASK;
  EMU->CTRL_SET = num_samples & _EMU_CTRL_TEMPAVGNUM_MASK;

  EMU->CMD = EMU_CMD_TEMPAVGREQ;
}
#endif

/***************************************************************************//**
 * @brief
 *   Get temperature in degrees Celsius.
 *
 * @return
 *   Temperature in degrees Celsius.
 ******************************************************************************/
__INLINE float sl_hal_emu_get_temperature(void)
{
  return ((float) ((EMU->TEMP & (_EMU_TEMP_TEMP_MASK | _EMU_TEMP_TEMPLSB_MASK) )
                   >> _EMU_TEMP_TEMPLSB_SHIFT)
          ) / 4.0f - EMU_TEMP_ZERO_C_IN_KELVIN;
}

/***************************************************************************//**
 * @brief
 *   Clear the reset cause register.
 *
 * @details
 *   This function clears all the reset cause bits of the RSTCAUSE register.
 *   The reset cause bits must be cleared by software before a new reset occurs.
 *   Otherwise, reset causes may accumulate. See sl_hal_emu_get_reset_cause().
 ******************************************************************************/
__INLINE void sl_hal_emu_clear_reset_cause(void)
{
  EMU->CMD_SET = EMU_CMD_RSTCAUSECLR;
}

/***************************************************************************//**
 * @brief
 *   Get the cause of the last reset.
 *
 * @details
 *   To be useful, the reset cause must be cleared by software before a new
 *   reset occurs. Otherwise, reset causes may accumulate, See
 *   sl_hal_emu_clear_reset_cause(). This function call will return the main cause for
 *   reset, which can be a bit mask (several causes) and clear away "noise".
 *
 * @return
 *   A reset cause mask. See the reference manual for a description
 *   of the reset cause mask.
 ******************************************************************************/
__INLINE uint32_t sl_hal_emu_get_reset_cause(void)
{
  return EMU->RSTCAUSE;
}

/***************************************************************************//**
 * @brief
 *   Disable/enable reset for various peripherals and signal sources.
 *
 * @param[in] reset
 *   Reset types to enable/disable.
 *
 * @param[in] mode
 *   Reset mode.
 ******************************************************************************/
__INLINE void sl_hal_emu_set_reset_control(sl_hal_emu_reset_source_t reset,
                                           sl_hal_emu_reset_mode_t mode)
{
  if (mode == 1) {
    EMU->RSTCTRL_SET = 1U << (uint32_t)reset;
  } else {
    EMU->RSTCTRL_CLR = 1U << (uint32_t)reset;
  }
}

#if defined(_EMU_CTRL_HDREGEM2EXITCLIM_MASK)
/***************************************************************************//**
 * @brief
 *   Enable HDREG EM2 Exit current limit.
 *
 * @details
 *   Limit HDREG max current drawn on EM2 exit by temporarily adjusting its
 *   output trim so current is pulled from DECOUPLE cap.
 ******************************************************************************/
__INLINE void sl_hal_emu_enable_hdreg_em2_exit_current_limit(void)
{
  EMU->CTRL_SET = EMU_CTRL_HDREGEM2EXITCLIM;
}

/***************************************************************************//**
 * @brief
 *   Disable HDREG EM2 Exit current limit.
 *
 * @details
 *   Limit HDREG max current drawn on EM2 exit by temporarily adjusting
 *   its output trim so current is pulled from DECOUPLE cap.
 ******************************************************************************/
__INLINE void sl_hal_emu_disable_hdreg_em2_exit_current_limit(void)
{
  EMU->CTRL_CLR = EMU_CTRL_HDREGEM2EXITCLIM;
}
#endif

#if defined(_EMU_CTRL_HDREGSTOPGEAR_MASK)
/***************************************************************************//**
 * @brief
 *   Set the HDREG max current capability limit.
 *
 * @param[in] current
 *   is HDREG max current capability limit.
 ******************************************************************************/
__INLINE void sl_hal_emu_set_hdreg_stop_gear(sl_hal_emu_hdreg_stop_gear_current_limit_t current)
{
  EMU->CTRL = ((current << _EMU_CTRL_HDREGSTOPGEAR_SHIFT) \
               & _EMU_CTRL_HDREGSTOPGEAR_MASK) | (EMU->CTRL & ~_EMU_CTRL_HDREGSTOPGEAR_MASK);
}
#endif

#if defined(_EMU_FLREG_MASK)
/***************************************************************************//**
 * @brief
 *   Enables the Flash Regulator (FLREG).
 *
 * @details
 *   Makes sure override bits are cleared when the Flash Regulator
 *   (FLREG) is enabled.
 ******************************************************************************/
__INLINE void sl_hal_emu_enable_flash_regulator(void)
{
  EMU->FLREG_CLR = EMU_FLREG_OVROVERRIDEEN;
  EMU->FLREG_CLR = EMU_FLREG_OVRENLP;
  EMU->FLREG_SET = EMU_FLREG_EN;
}

/***************************************************************************//**
 * @brief
 *   Disables the Flash Regulator (FLREG).
 *
 * @details
 *   Sets override bits to improve FLBOD accuracy when the Flash
 *   Regulator (FLREG) is disabled.
 ******************************************************************************/
__INLINE void sl_hal_emu_disable_flash_regulator(void)
{
  EMU->FLREG_CLR = EMU_FLREG_EN;
  EMU->FLREG_SET = EMU_FLREG_OVRENLP;
  EMU->FLREG_SET = EMU_FLREG_OVROVERRIDEEN;
}
#endif /* defined(_EMU_FLREG_MASK) */

#if defined(_EMU_EFPIF_MASK)
/***************************************************************************//**
 * @brief
 *   Disable one or more EFP interrupts.
 *
 * @param[in] flags
 *   EFP interrupt sources to disable. Use one or more valid
 *   interrupt flags for the EFP module (EFPIENnnn).
 ******************************************************************************/
__INLINE void sl_hal_emu_efp_disable_interrupts(uint32_t flags)
{
  EMU->EFPIEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more EFP interrupts.
 *
 * @param[in] flags
 *   EFP interrupt sources to enable. Use one or more valid
 *   interrupt flags for the EFP module (EFPIENnnn).
 ******************************************************************************/
__INLINE void sl_hal_emu_efp_enable_interrupts(uint32_t flags)
{
  EMU->EFPIEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending EMU EFP interrupt flags.
 *
 * @note
 *   Event bits are not cleared by the use of this function.
 *
 * @return
 *   EMU EFP interrupt sources pending.
 ******************************************************************************/
__INLINE uint32_t sl_hal_emu_efp_get_pending_interrupts(void)
{
  return EMU->EFPIF;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending EMU EFP interrupts.
 *
 * @param[in] flags
 *   EMU EFP interrupt sources to set to pending. Use one or more
 *   valid interrupt flags for the EMU EFP module.
 ******************************************************************************/
__INLINE void sl_hal_emu_efp_set_interrupts(uint32_t flags)
{
  EMU->EFPIF_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Clear one or more pending EMU EFP interrupts.
 *
 * @param[in] flags
 *   Pending EMU EFP interrupt sources to clear. Use one or more valid
 *   interrupt flags for the EMU EFP module.
 ******************************************************************************/
__INLINE void sl_hal_emu_efp_clear_interrupts(uint32_t flags)
{
  EMU->EFPIF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending EMU EFP interrupt flags.
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @note
 *   Interrupt flags are not cleared by the use of this function.
 *
 * @return
 *   Pending and enabled EMU EFP interrupt sources
 *   Return value is the bitwise AND of
 *   - the enabled interrupt sources in EMU_EFPIEN and
 *   - the pending interrupt flags EMU_EFPIF.
 ******************************************************************************/
__INLINE uint32_t sl_hal_emu_efp_get_enabled_pending_interrupts(void)
{
  uint32_t ien;

  ien = EMU->EFPIEN;

  return EMU->EFPIF & ien;
}

/***************************************************************************//**
 * @brief
 *   Enable EFP to drive DVDD voltage.
 *
 * @details
 *   Set this if EFP's DCDC output is powering DVDD supply. This mode
 *   assumes that internal DCDC is not being used.
 ******************************************************************************/
__INLINE void sl_hal_emu_enable_efp_drive_dvdd(void)
{
  EMU->CTRL_SET = EMU_CTRL_EFPDRVDVDD;
}

/***************************************************************************//**
 * @brief
 *   Disable EFP to drive DVDD voltage.
 ******************************************************************************/
__INLINE void sl_hal_emu_disable_efp_drive_dvdd(void)
{
  EMU->CTRL_CLR = EMU_CTRL_EFPDRVDVDD;
}

/***************************************************************************//**
 * @brief
 *   Enable EFP Direct Mode.
 ******************************************************************************/
__INLINE void sl_hal_emu_enable_efp_direct_mode(void)
{
  EMU->CTRL_SET = EMU_CTRL_EFPDIRECTMODEEN;
}

/***************************************************************************//**
 * @brief
 *   Disable EFP Direct Mode.
 ******************************************************************************/
__INLINE void sl_hal_emu_disable_efp_direct_mode(void)
{
  EMU->CTRL_CLR = EMU_CTRL_EFPDIRECTMODEEN;
}

/***************************************************************************//**
 * @brief
 *   Set to enable EFP to drive Decouple voltage.
 *
 * @details
 *   Once set, internal LDO will be disabled, and the EMU will control EFP
 *   for voltage-scaling. Note that because this bit disables the internal
 *   LDO powering the core, it should not be set until after EFP's DECOUPLE
 *   output has been configured and enabled.
 ******************************************************************************/
__INLINE void sl_hal_emu_enable_efp_drive_decouple(void)
{
  EMU->CTRL_SET = EMU_CTRL_EFPDRVDECOUPLE;
}

/***************************************************************************//**
 * @brief
 *   Set to disable EFP to drive Decouple voltage.
 ******************************************************************************/
__INLINE void sl_hal_emu_disable_efp_drive_decouple(void)
{
  EMU->CTRL_CLR = EMU_CTRL_EFPDRVDECOUPLE;
}
#endif /* defined(_EMU_EFPIF_MASK) */

#if defined(_DCDC_LOCK_MASK)
/***************************************************************************//**
 * @brief
 *   Lock DCDC registers in order to protect them against unintended modification.
 ******************************************************************************/
__INLINE void sl_hal_emu_dcdc_lock(void)
{
  DCDC->LOCK = ~(_DCDC_LOCK_LOCKKEY_UNLOCKKEY << _DCDC_LOCK_LOCKKEY_SHIFT);
}

/***************************************************************************//**
 * @brief
 *   Unlock the DCDC so that writing to locked registers again is possible.
 ******************************************************************************/
__INLINE void sl_hal_emu_dcdc_unlock(void)
{
  DCDC->LOCK = _DCDC_LOCK_LOCKKEY_UNLOCKKEY << _DCDC_LOCK_LOCKKEY_SHIFT;
}
#endif

#if defined(_DCDC_SYNCBUSY_MASK)
/***************************************************************************//**
 * @brief
 *   Wait for the DCDC to complete all synchronization of register changes.
 *
 * @param[in] mask
 *   A bitmask corresponding to SYNCBUSY register defined bits indicating
 *   registers that must complete any ongoing synchronization.
 ******************************************************************************/
__INLINE void sl_hal_emu_dcdc_sync(uint32_t mask)
{
  while (0UL != (DCDC->SYNCBUSY & mask)) {
    // Wait for previous synchronization to finish.
  }
}
#endif

#if defined(SL_HAL_EMU_DCDC_BOOST_PRESENT)
/***************************************************************************//**
 * @brief
 *   Enable Boost External Shutdown Mode.
 *
 * @note
 *   The boost DC-DC converter can be activated or deactivated from a
 *   dedicated BOOST_EN pin. For that, you need to enable the
 *   Boost External Shutdown Mode.
 ******************************************************************************/
__INLINE void sl_hal_emu_enable_boost_external_shutdown(void)
{
  EMU->BOOSTCTRL_CLR = EMU_BOOSTCTRL_BOOSTENCTRL;
}

/***************************************************************************//**
 * @brief
 *   Disable Boost External Shutdown Mode.
 ******************************************************************************/
__INLINE void sl_hal_emu_disable_boost_external_shutdown(void)
{
  EMU->BOOSTCTRL_SET = EMU_BOOSTCTRL_BOOSTENCTRL;
}
#endif

#if defined(_DCDC_EN_EN_MASK)
/***************************************************************************//**
 * @brief
 *   Enable EMU DCDC.
 ******************************************************************************/
__INLINE void sl_hal_emu_enable_dcdc(void)
{
  DCDC->EN_SET = DCDC_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Disable EMU DCDC.
 ******************************************************************************/
__INLINE void sl_hal_emu_disable_dcdc(void)
{
  DCDC->EN_CLR = DCDC_EN_EN;
}
#endif

#if defined(_DCDC_IEN_MASK)
/***************************************************************************//**
 * @brief
 *   Enable EMU DCDC interrupts.
 *
 * @param[in] flags
 *   DCDC interrupt sources to enable.
 ******************************************************************************/
__INLINE void sl_hal_emu_dcdc_enable_interrupts(uint32_t flags)
{
  DCDC->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable EMU DCDC interrupts.
 *
 * @param[in] flags
 *   DCDC interrupt sources to disable.
 ******************************************************************************/
__INLINE void sl_hal_emu_dcdc_disable_interrupts(uint32_t flags)
{
  DCDC->IEN_CLR = flags;
}
#endif

/** @} (end addtogroup emu) */

#ifdef __cplusplus
}
#endif

/* *INDENT-OFF* */
/***************************************************************************//**
 * @addtogroup emu EMU - Energy management unit
 * @{
 *
 *@n @section emu_example Example
 *
 *  EMU Basic temperature measurement example:
 *  @code{.c}
 *  {
 *    // Wait for temperature sensor to be ready
 *    while (!sl_hal_emu_is_temperature_ready()) {
 *      // Wait for temperature readiness
 *    }
 *
 *    // Read temperature in degrees Celsius
 *    float temperature = sl_hal_emu_get_temperature();
 *
 *    // Power management example with DCDC
 *    #if defined(SL_HAL_EMU_DCDC_BUCK_PRESENT)
 *    // Initialize clocks for DCDC
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_DCDC);
 *    // Initialize DCDC regulator
 *    sl_hal_emu_dcdc_init_t init = SL_HAL_EMU_DCDC_INIT_DEFAULT;
 *    sl_hal_emu_init_dcdc(&init);
 *
 *    // Set DCDC mode to regulation
 *    sl_hal_emu_set_dcdc_mode(SL_HAL_EMU_DCDC_MODE_REGULATION);
 *    #endif
 *
 *    // RAM power management
 *    // Power down all RAM blocks except the first one
 *    sl_hal_emu_ram_power_down(SRAM_BASE, SRAM_BASE + SRAM_SIZE);
 *
 *    // Power up all RAM blocks when needed again
 *    sl_hal_emu_ram_power_up();
 *
 *    // Clear reset cause when done handling it
 *    uint32_t reset_cause = sl_hal_emu_get_reset_cause();
 *    sl_hal_emu_clear_reset_cause();
 *  }
 *  @endcode
 *
 * @} (end addtogroup emu)
 ******************************************************************************/
/* *INDENT-ON* */

#endif /* defined(EMU_PRESENT) */
#endif /* #ifndef SL_HAL_EMU_H */
