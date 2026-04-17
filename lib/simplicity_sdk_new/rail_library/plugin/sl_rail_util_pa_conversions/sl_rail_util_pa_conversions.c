/***************************************************************************//**
 * @file
 * @brief PA power conversion functions provided to the customer as source for
 *   highest level of customization.
 * @details This file contains the logic that converts dBm powers to
 *   chip-and-PA-specific power levels used by the RAIL library.
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
// For details on how to use this plugin, see
//   https://www.silabs.com/documents/public/application-notes/an1127-power-amplifier-power-conversion-functions.pdf

#include "em_device.h"
#if defined(_SILICON_LABS_32B_SERIES_2) || defined(SIMULATION_DEVICE)
#include "em_cmu.h"
#else
#include "sl_clock_manager.h"
#endif
#include "sl_rail.h"
#include "rail.h"
#include "sl_rail_util_pa_conversions.h"
#include "sl_rail_util_pa_config.h"

#if     SL_RAIL_UTIL_PA_NVM_ENABLED
#error  NVM PA configuration is not yet supported by the sl_rail_util_pa component
#endif//SL_RAIL_UTIL_PA_NVM_ENABLED

static sl_rail_tx_power_table_config_t sli_rail_util_pa_power_table;

#ifdef RAIL_MODULE_LIB
const bool sli_rail_3_pa = false;
#else
const bool sli_rail_3_pa = true;
#endif

#define PA_SUBMODE_MASK 0xC000UL
#define PA_SUBMODE_SHIFT 14U

sl_rail_status_t sl_rail_util_pa_init_tx_power_table(sl_rail_handle_t radio_handle,
                                                     const sl_rail_tx_power_table_config_t *p_tx_power_table_config)
{
  (void) radio_handle;
  sl_rail_status_t status = sl_rail_verify_tx_power_conversion(p_tx_power_table_config);
  if (status == RAIL_STATUS_NO_ERROR) {
    sli_rail_util_pa_power_table = *p_tx_power_table_config;
  }
  return status;
}

const sl_rail_pa_power_setting_t *sl_rail_util_pa_get_power_setting_table(sl_rail_handle_t rail_handle,
                                                                          sl_rail_tx_pa_mode_t pa_mode,
                                                                          sl_rail_tx_power_t *p_min_ddbm,
                                                                          sl_rail_tx_power_t *p_max_ddbm,
                                                                          sl_rail_tx_power_t *p_step_ddbm)
{
  (void)rail_handle;
  sl_rail_pa_descriptor_t *p_pa_descriptor = &(sli_rail_util_pa_power_table.p_pa_table_descriptor[pa_mode]);
  *p_min_ddbm = p_pa_descriptor->min_power_ddbm;
  *p_max_ddbm = p_pa_descriptor->max_power_ddbm;
  *p_step_ddbm = p_pa_descriptor->step_power_ddbm;
  return (sl_rail_pa_power_setting_t*)(p_pa_descriptor->p_power_setting_table); //This includes sl_rail_pa_power_setting_t and curr_pa_power_ddbm
}

sl_rail_status_t sl_rail_util_pa_get_tx_power_limits(sl_rail_handle_t rail_handle,
                                                     sl_rail_tx_pa_mode_t pa_mode,
                                                     sl_rail_tx_power_t *p_min_ddbm,
                                                     sl_rail_tx_power_t *p_max_ddbm,
                                                     sl_rail_tx_power_t *p_step_ddbm)
{
  (void) rail_handle;
  if (pa_mode > SL_RAIL_TX_PA_MODE_INVALID) {
    return SL_RAIL_STATUS_INVALID_PARAMETER;
  }
  if (pa_mode == SL_RAIL_TX_PA_MODE_INVALID) {
    pa_mode = sl_rail_get_pa_mode_from_channel_entry(rail_handle);
  }
  // Turn pa_mode into PA table index
#if (!SL_RAIL_SUPPORTS_2P4_GHZ_BAND)
  pa_mode -= 1U; // Underflow to 255 will be caught in next if condition
#endif
  if (pa_mode >= sli_rail_util_pa_power_table.num_of_tables) {
    return SL_RAIL_STATUS_INVALID_PARAMETER;
  }
  const sl_rail_pa_descriptor_t *p_pa_descriptor = &(sli_rail_util_pa_power_table.p_pa_table_descriptor[pa_mode]);
  if (p_min_ddbm != NULL) {
    *p_min_ddbm = p_pa_descriptor->min_power_ddbm;
  }
  if (p_max_ddbm != NULL) {
    *p_max_ddbm = p_pa_descriptor->max_power_ddbm;
  }
  if (p_step_ddbm != NULL) {
    *p_step_ddbm = p_pa_descriptor->step_power_ddbm;
  }
  return SL_RAIL_STATUS_NO_ERROR;
}

sl_rail_status_t sl_rail_util_pa_convert_power_to_actual(sl_rail_handle_t rail_handle,
                                                         sl_rail_tx_pa_mode_t pa_mode,
                                                         sl_rail_tx_power_t *p_ddbm)
{
  if ((p_ddbm == NULL)
      || (pa_mode > SL_RAIL_TX_PA_MODE_INVALID)) {
    return SL_RAIL_STATUS_INVALID_PARAMETER;
  }
  if (pa_mode == SL_RAIL_TX_PA_MODE_INVALID) {
    pa_mode = sl_rail_get_pa_mode_from_channel_entry(rail_handle);
  }
  sl_rail_tx_power_setting_entry_t power_entry;
  sl_rail_status_t status
    = sl_railcb_convert_ddbm_to_power_setting_entry(rail_handle,
                                                    *p_ddbm,
                                                    pa_mode,
                                                    SL_RAIL_TX_POWER_MAX,
                                                    &power_entry);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    return status;
  }
  *p_ddbm = power_entry.curr_pa_power_ddbm;
  return SL_RAIL_STATUS_NO_ERROR;
}

sl_rail_status_t sl_railcb_convert_ddbm_to_power_setting_entry(sl_rail_handle_t rail_handle,
                                                               sl_rail_tx_power_t power_ddbm,
                                                               sl_rail_tx_pa_mode_t pa_mode,
                                                               sl_rail_tx_power_t channel_restr_max_power_ddbm,
                                                               sl_rail_tx_power_setting_entry_t *p_power_setting_info)
{
  (void) rail_handle;
  // Initialize the powersetting config from the table in a init fn()
  // Choose the table based on pa_mode
  sl_rail_tx_power_table_config_t const *p_power_table_config = &sli_rail_util_pa_power_table;
  // Turn pa_mode into PA table index
#if (!SL_RAIL_SUPPORTS_2P4_GHZ_BAND)
  pa_mode -= 1U; // Underflow to 255 will be caught in next if condition
#endif
  if (pa_mode >= p_power_table_config->num_of_tables) {
    return SL_RAIL_STATUS_INVALID_CALL;
  }
  sl_rail_pa_descriptor_t *p_pa_descriptor = &(p_power_table_config->p_pa_table_descriptor[pa_mode]);
  if ((p_pa_descriptor == NULL)
      || (p_pa_descriptor->num_of_values == 0U)) {
    // Seems no power table was configured?!
    return SL_RAIL_STATUS_INVALID_STATE;
  }

  sl_rail_tx_power_t min_power_ddbm = p_pa_descriptor->min_power_ddbm;
  sl_rail_tx_power_t max_power_ddbm = p_pa_descriptor->max_power_ddbm;
  sl_rail_tx_power_t table_step_ddbm = p_pa_descriptor->step_power_ddbm;

  // Cap the power to within the range of the mapping table
  if (power_ddbm < min_power_ddbm) {
    power_ddbm = min_power_ddbm;
  } else if (power_ddbm > max_power_ddbm) {
    power_ddbm = max_power_ddbm;
  } else {
    // Power level is within bounds (MISRA required else)
  }

  if (table_step_ddbm == 0U) {
    return SL_RAIL_STATUS_INVALID_STATE;
  }
  // Calculate indices
  uint16_t max_index =  (p_pa_descriptor->num_of_values - 1U);
  uint16_t power_index = (uint16_t)((power_ddbm - min_power_ddbm) / table_step_ddbm);

  // Adjust the index if our estimated max power is less than max power provided
  // in the table because an extra entry is added in table in this case.
  if (max_power_ddbm > (min_power_ddbm + max_index * table_step_ddbm)) {
    max_index++;
  }

  // Ensure power_index is within bounds
  if (power_index > max_index || power_ddbm == max_power_ddbm) {
    power_index = max_index;
  }
  bool find_power_under_chan_restr = true;
  sl_rail_pa_power_setting_t power_setting = { 0 };
  while (find_power_under_chan_restr) {
    //TODO: Powersetting has to be extracted once offsets are also part of this 32 bits
    power_setting = p_pa_descriptor->p_power_setting_table[power_index];

    while ((power_index > 0U)
           && (power_setting
               == (sl_rail_pa_power_setting_t)p_pa_descriptor->p_power_setting_table[power_index - 1U])) {
      // TODO: This might not be true comparison if we add offset in the table,
      // we will have to extract the powersetting entry
      power_index--;
    }

    // get estimated power
    // (this is true until we have offsets in place)
    if (power_ddbm != max_power_ddbm) {
      // Calculate actual power
      power_ddbm = min_power_ddbm + ((sl_rail_tx_power_t)power_index * table_step_ddbm);
    }
    //TODO: Should there be a else? to get the max power
    #if 0
    //Once offset are in place remove the above if block and activate this block
    // And define the mask and shifts for offset as required by the table.
    power_ddbm  = ((p_pa_descriptor->p_power_setting_table[power_index])
                   & (PA_ACTUAL_DBM_OFFSET_MASK))
                  >> PA_ACTUAL_DBM_OFFSET_SHIFT;
    #endif
    // Restrictions should be based on rounding mode
    // TODO: this should also be depend on rounding mode
    // uint8_t rounding_mode = p_pa_descriptor->pa_rounding_mode;
    if (power_ddbm <= channel_restr_max_power_ddbm) {
      find_power_under_chan_restr = false;
    } else {
      if (power_index == 0) {
        break;
      }
      power_index--;
    }
  }

  // Find the powersetting entry from the table for power_ddbm and pa_mode
  // Check if the actual power returned from the table is within the
  p_power_setting_info->pa_power_setting = power_setting;
  //TODO:
#if 0
  p_power_setting_info->pa_power_setting = power_setting
                                           & (~PA_ACTUAL_DBM_OFFSET_MASK);
#endif
  p_power_setting_info->curr_pa_power_ddbm = power_ddbm;

  return SL_RAIL_STATUS_NO_ERROR;
}

void sl_rail_util_pa_init(void)
{
  (void) sl_rail_util_pa_init_tx_power_table(SL_RAIL_EFR32_HANDLE, &sl_rail_util_pa_tx_power_table);
  // Configure default config power
#if SL_RAIL_UTIL_PA_CALIBRATION_ENABLE
  sl_rail_enable_pa_cal(SL_RAIL_EFR32_HANDLE, true);
#else
  sl_rail_enable_pa_cal(SL_RAIL_EFR32_HANDLE, false);
#endif
}

sl_rail_status_t sl_rail_util_pa_post_init(sl_rail_handle_t rail_handle,
                                           sl_rail_tx_pa_mode_t pa_mode)
{
  (void) pa_mode; // may be used in the future
  sl_rail_set_tx_pa_ramp_time(rail_handle, SL_RAIL_UTIL_PA_RAMP_TIME_US);
  sl_rail_set_tx_pa_voltage(rail_handle, SL_RAIL_UTIL_PA_VOLTAGE_MV);
  return SL_RAIL_STATUS_NO_ERROR;
}

void sl_rail_util_pa_on_channel_config_change(sl_rail_handle_t rail_handle,
                                              const sl_rail_channel_config_entry_t *p_entry)
{
  (void) p_entry;
  sl_rail_tx_pa_mode_t current_pa_mode = sl_rail_get_pa_mode(rail_handle); //from state
  sl_rail_tx_pa_mode_t new_pa_mode = sl_rail_get_pa_mode_from_channel_entry(rail_handle); //from configs
  if (current_pa_mode == SL_RAIL_TX_PA_MODE_INVALID) {
    (void) sl_rail_util_pa_post_init(rail_handle, new_pa_mode);
  }
  if (current_pa_mode != new_pa_mode) {
    sl_rail_tx_power_t tx_power_ddbm = SL_RAIL_UTIL_PA_POWER_DECI_DBM;
    if (current_pa_mode != SL_RAIL_TX_PA_MODE_INVALID) {
      tx_power_ddbm = sl_rail_get_tx_power_dbm(rail_handle);
    }
    sl_rail_status_t status = sl_rail_set_tx_power_dbm(rail_handle, tx_power_ddbm);
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      while (true) {
      }     // Error: Can't set TX Power
    }
  }
}

// Begin RAIL 2.x compatibility APIs

#if SL_RAIL_SUPPORTS_2P4_GHZ_BAND
static RAIL_TxPowerConfig_t txPowerConfig2p4Ghz = {
  .mode = SL_RAIL_TX_POWER_MODE_2P4_GHZ_HIGHEST,
  .voltage = SL_RAIL_UTIL_PA_VOLTAGE_MV,
  .rampTime = SL_RAIL_UTIL_PA_RAMP_TIME_US,
};
#endif
#if SL_RAIL_SUPPORTS_SUB_GHZ_BAND
static RAIL_TxPowerConfig_t txPowerConfigSubGhz = {
  .mode = SL_RAIL_TX_POWER_MODE_SUB_GHZ_HIGHEST,
  .voltage = SL_RAIL_UTIL_PA_VOLTAGE_MV,
  .rampTime = SL_RAIL_UTIL_PA_RAMP_TIME_US,
};
#endif
#if SL_RAIL_SUPPORTS_OFDM_PA
static RAIL_TxPowerConfig_t txPowerConfigOFDM = {
  .mode = SL_RAIL_TX_POWER_MODE_OFDM_PA_POWERSETTING_TABLE,
  .voltage = SL_RAIL_UTIL_PA_VOLTAGE_MV,
  // rampTime is irrelevant for OFDM PA
};
#endif

RAIL_TxPowerConfig_t *sl_rail_util_pa_get_tx_power_config_2p4ghz(void)
{
#if SL_RAIL_SUPPORTS_2P4_GHZ_BAND
  return &txPowerConfig2p4Ghz;
#else
  return NULL;
#endif
}

RAIL_TxPowerConfig_t *sl_rail_util_pa_get_tx_power_config_subghz(void)
{
#if SL_RAIL_SUPPORTS_SUB_GHZ_BAND
  return &txPowerConfigSubGhz;
#else
  return NULL;
#endif
}

RAIL_TxPowerConfig_t *sl_rail_util_pa_get_tx_power_config_ofdm(void)
{
#if SL_RAIL_SUPPORTS_OFDM_PA
  return &txPowerConfigOFDM;
#else
  return NULL;
#endif
}

RAIL_TxPowerLevel_t RAIL_ConvertDbmToRaw(RAIL_Handle_t railHandle,
                                         RAIL_TxPowerMode_t mode,
                                         RAIL_TxPower_t power)
{
  // TODO stub, this will go as we change calls in the lib to use new cb
  (void)railHandle;
  (void)mode;
  (void)power;
  return RAIL_TX_POWER_LEVEL_INVALID;
}

RAIL_Status_t RAIL_ConvertDbmToPowerSettingEntry(RAIL_Handle_t railHandle,
                                                 RAIL_TxPowerMode_t mode,
                                                 RAIL_TxPower_t power,
                                                 RAIL_TxPowerSettingEntry_t *powerSettingInfo)
{
  (void) railHandle;
  (void)mode;
  (void)power;
  (void)powerSettingInfo;
  return RAIL_STATUS_INVALID_CALL;
}

RAIL_TxPower_t RAIL_ConvertRawToDbm(RAIL_Handle_t railHandle,
                                    RAIL_TxPowerMode_t mode,
                                    RAIL_TxPowerLevel_t powerLevel)
{
  // TODO stub, this will go as we change calls in the lib to use new cb
  (void)railHandle;
  (void)mode;
  (void) powerLevel;
  return SL_RAIL_TX_POWER_MIN;
}

// End RAIL 2.x compatibility APIs
