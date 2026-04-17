/******************************************************************************
* @file  rsi_system_config.h
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

#ifndef __RSI_SYSTEM_CONFIG_H__
#define __RSI_SYSTEM_CONFIG_H__
#include "rsi_ccp_common.h"

#include "base_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t scdc_volt_sel1[];
extern uint32_t lp_scdc_extcapmode[];
extern uint32_t m32rc_osc_trim_efuse[];
extern uint32_t m20rc_osc_trim_efuse[];
extern uint32_t dblr_32m_trim_efuse[];
extern uint32_t m20ro_osc_trim_efuse[];
extern uint32_t ro_32khz_trim_efuse[];
extern uint32_t rc_16khz_trim_efuse[];
extern uint32_t rc_64khz_trim_efuse[];
extern uint32_t rc_32khz_trim_efuse[];
extern uint32_t vbatt_status_trim_efuse[];
extern uint32_t ro_ts_efuse[];
extern uint32_t ro_tempsense_config[];
extern uint32_t vbg_tsbjt_efuse[];
extern uint32_t auxadc_off_diff_efuse[];
extern uint32_t auxadc_gain_diff_efuse[];
extern uint32_t auxadc_off_se_efuse[];
extern uint32_t auxadc_gain_se_efuse[];
extern uint32_t bg_trim_efuse[];
extern uint32_t blackout_trim_efuse[];
extern uint32_t poc_bias_efuse[];
extern uint32_t buck_trim_efuse[];
extern uint32_t ldosoc_trim_efuse[];
extern uint32_t dpwm_freq_trim_efuse[];
extern uint32_t delvbe_tsbjt_efuse[];
extern uint32_t xtal1_bias_efuse[];
extern uint32_t xtal2_bias_efuse[];
extern uint32_t bod_cmp_hyst[];
extern uint32_t ipmu_bod_clks_common_config2[];
extern uint32_t ipmu_bod_clks_common_config1[];
extern uint32_t pmu_common_config[];
extern uint32_t ipmu_common_config[];
extern uint32_t xtal1_khz_fast_start_en[];
extern uint32_t xtal1_khz_fast_start_disable[];
extern uint32_t hp_ldo_voltsel[];
extern uint32_t poc_bias_current_11[];
extern uint32_t ro_32khz_trim00_efuse[];
extern uint32_t retnLP_volt_trim_efuse[];
extern uint32_t retnHP_volt_trim_efuse[];
extern uint32_t hpldo_volt_trim_efuse[];
extern uint32_t scdc_volt_trim_efuse[];
extern uint32_t poc_bias_current[];
extern uint32_t scdc_volt_sel2[];
extern uint32_t scdc_volt_trim_efuse[];
extern uint32_t ana_perif_ptat_common_config1[];
extern uint32_t ana_perif_ptat_common_config2[];
extern uint32_t retn_ldo_0p75[];
extern uint32_t retn_ldo_lpmode[];
extern uint32_t retn_ldo_hpmode[];
extern uint32_t retn_ldo_voltsel[];
extern uint32_t ipmu_scdc_enable[];
extern uint32_t buck_fast_transient_duty_1p8[];
extern uint32_t hpldo_tran[];
extern uint32_t LDOFLASH_BYPASS[];
#define POWER_TARN_DISABLE         0
#define POWER_TARN_ALWAYS_USE      1
#define POWER_TARN_CONDITIONAL_USE 2

#define XTAL_CAP_MODE POWER_TARN_CONDITIONAL_USE

#define IPMU_DOTC_PROG
#define IPMU_CALIB_DATA
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint8_t uint8;
#define cmemcpy memcpy

#ifdef __cplusplus
}
#endif

#endif //__RSI_SYSTEM_CONFIG_H__
