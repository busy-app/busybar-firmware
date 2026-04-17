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

#ifndef SL_RAIL_PA_CONVERSIONS_H
#define SL_RAIL_PA_CONVERSIONS_H

#include "sl_rail_types.h"
#include "sl_rail_util_pa_types.h"
#include "sl_rail_util_compatible_pa.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup PA_Power_Conversions PA Power Conversions
 * @ingroup PA
 * @{
 */

/// The PA table to be used for transmission
extern const sl_rail_tx_power_table_config_t sl_rail_util_pa_tx_power_table;

/**
 * Initialize Transmit power tables.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @param[in] p_tx_power_table_config A pointer to the TX power table to use.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_util_pa_init_tx_power_table(sl_rail_handle_t radio_handle,
                                                     const sl_rail_tx_power_table_config_t *p_tx_power_table_config);

/**
 * Provide a channel config change callback capable of configuring the PA
 * correctly.
 *
 * @param[in] rail_handle The real RAIL handle being passed into this callback.
 * @param[in] p_entry A pointer to the channel config entry being switched
 *   to by hardware.
 */
void sl_rail_util_pa_on_channel_config_change(sl_rail_handle_t rail_handle,
                                              const sl_rail_channel_config_entry_t *p_entry);

/**
 * Get the TX PA power setting table and related values.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] mode PA mode for which to get the powersetting table.
 * @param[out] p_min_ddbm A pointer to a \ref sl_rail_tx_power_t.
 * @param[out] p_max_ddbm A pointer to a \ref sl_rail_tx_power_t.
 * @param[out] p_step_ddbm A pointer to a \ref sl_rail_tx_power_t.
 * @return Power setting table start address. When NULL is returned all out params
 *   above won't be set.
 *
 * The number of entries in the table can be calculated based on output
 * *p_min_ddbm, *p_max_ddbm, and *p_step_ddbm parameters.
 * For example, for *p_min_ddbm = 115 (11.5 dBm),
 * *p_max_ddbm = 300 (30 dBm), and *p_step_ddbm = 1, the number of
 * entries in table would be 186.
 */
const sl_rail_pa_power_setting_t *sl_rail_util_pa_get_power_setting_table(sl_rail_handle_t rail_handle,
                                                                          sl_rail_tx_power_mode_t mode,
                                                                          sl_rail_tx_power_t *p_min_ddbm,
                                                                          sl_rail_tx_power_t *p_max_ddbm,
                                                                          sl_rail_tx_power_t *p_step_ddbm);

/** @} */ // PA_Power_Conversions

#ifdef __cplusplus
}
#endif

#endif // SL_RAIL_PA_CONVERSIONS_H
