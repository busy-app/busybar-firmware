/***************************************************************************//**
 * @file
 * @brief PA power conversion functions provided to the customer as source for
 *   highest level of customization.
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

#ifndef SL_RAIL_UTIL_COMPATIBLE_PA_H
#define SL_RAIL_UTIL_COMPATIBLE_PA_H

#include "sl_rail_types.h"
#include "rail_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup PA_Power_Conversions PA Power Conversions
 * @ingroup PA
 * @{
 */

/**
 * Initialize PA dBm conversion for transmit.
 */
void sl_rail_util_pa_init(void);

/**
 * Establish PA mode and related parameters from the PA configuration
 * post \ref sl_rail_init().
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] pa_mode The PA mode to establish.
 * @return Status code indicating success of the function call.
 *
 * This function should be called soon after \ref sl_rail_init() when
 * the application is not using the \ref rail_util_callbacks component.
 */
sl_rail_status_t sl_rail_util_pa_post_init(sl_rail_handle_t rail_handle,
                                           sl_rail_tx_pa_mode_t pa_mode);

/**
 * Get the TX PA power conversion limits for the indicated PA mode.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] pa_mode The PA mode to use for the conversion.
 *   If this is \ref SL_RAIL_TX_PA_MODE_INVALID then the PA mode
 *   associated with the current channel configuration is used.
 * @param[out] p_min_ddbm A pointer to a \ref sl_rail_tx_power_t; may be NULL.
 * @param[out] p_max_ddbm A pointer to a \ref sl_rail_tx_power_t; may be NULL.
 * @param[out] p_step_ddbm A pointer to a \ref sl_rail_tx_power_t; may be NULL.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_util_pa_get_tx_power_limits(sl_rail_handle_t rail_handle,
                                                     sl_rail_tx_pa_mode_t pa_mode,
                                                     sl_rail_tx_power_t *p_min_ddbm,
                                                     sl_rail_tx_power_t *p_max_ddbm,
                                                     sl_rail_tx_power_t *p_step_ddbm);

/**
 * Convert requested power to actual power achievable for the indicated PA mode.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] pa_mode The PA mode to use for the conversion.
 *   If this is \ref SL_RAIL_TX_PA_MODE_INVALID then the PA mode
 *   associated with the current channel configuration is used.
 * @param[in,out] p_ddbm A non-NULL pointer to the deci-dBm value to convert,
 *   which the function updates with the resulting actual power based on
 *   the power conversion algorithm for the currently selected PA
 *   when returning success.
 * @return Status code indicating success of the function call.
 *
 * @note This function does not take into account any channel configuration
 *   power restrictions.
 */
sl_rail_status_t sl_rail_util_pa_convert_power_to_actual(sl_rail_handle_t rail_handle,
                                                         sl_rail_tx_pa_mode_t pa_mode,
                                                         sl_rail_tx_power_t *p_ddbm);

/**
 * Returns a RAIL 2.x compatible power configuration for the default
 * 2.4 GHz PA selection on platforms that support the 2.4 GHz band,
 * or NULL otherise.
 *
 * @return A pointer to the TX Power Config stucture.
 *
 * @deprecated This RAIL 2.x function should be replaced in RAIL 3 by use
 *   of \ref sl_rail_util_pa_post_init().
 */
RAIL_TxPowerConfig_t *sl_rail_util_pa_get_tx_power_config_2p4ghz(void);

/**
 * Returns a RAIL 2.x compatible power configuration for the default
 * Sub-GHz PA selection on platforms that support the Sub-GHz band,
 * or NULL otherise.
 *
 * @return A pointer to the TX Power Config stucture.
 *
 * @deprecated This RAIL 2.x function should be replaced in RAIL 3 by use
 *   of \ref sl_rail_util_pa_post_init().
 */
RAIL_TxPowerConfig_t *sl_rail_util_pa_get_tx_power_config_subghz(void);

/**
 * Returns a RAIL 2.x compatible power configuration for the default
 * OFDM PA selection on platforms that support OFDM, or NULL otherise.
 *
 * @return A pointer to the TX Power Config stucture.
 *
 * @deprecated This RAIL 2.x function should be replaced in RAIL 3 by use
 *   of \ref sl_rail_util_pa_post_init().
 */
RAIL_TxPowerConfig_t *sl_rail_util_pa_get_tx_power_config_ofdm(void);

/** @} */ // PA_Power_Conversions

#ifdef __cplusplus
}
#endif

#endif // SL_RAIL_UTL_COMPATIBLE_PA_H
