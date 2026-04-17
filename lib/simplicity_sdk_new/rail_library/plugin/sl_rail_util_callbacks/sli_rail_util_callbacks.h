/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_RAIL_UTIL_CALLBACKS_H
#define SLI_RAIL_UTIL_CALLBACKS_H

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "sl_rail.h"
#include "rail.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An internal callback, called on RAIL init completion.
 *
 * @param[in] rail_handle The RAIL handle associated with the RAIL init
 * completion notification.
 *
 * @note: This function is not intended for application use. For application
 * use, see \ref sl_rail_util_on_rf_ready.
 */
#ifdef SL_CATALOG_SL_RAIL_UTIL_CALLBACKS_PRESENT
void sli_rail_util_on_rf_ready(sl_rail_handle_t rail_handle);
#else//!SL_CATALOG_SL_RAIL_UTIL_CALLBACKS_PRESENT
void sli_rail_util_on_rf_ready(RAIL_Handle_t rail_handle);
#endif//SL_CATALOG_SL_RAIL_UTIL_CALLBACKS_PRESENT

/**
 * An internal callback, called on a channel configuration change.
 *
 * @param[in] rail_handle The RAIL handle associated with the channel config
 * change notification.
 * @param[in] p_entry A pointer to the channel configuration being changed to.
 *
 * @note: This function is not intended for application use. For application
 * use, see \ref sl_rail_util_on_channel_config_change.
 */
#ifdef SL_CATALOG_SL_RAIL_UTIL_PA_PRESENT
void sli_rail_util_on_channel_config_change(sl_rail_handle_t rail_handle,
                                            const sl_rail_channel_config_entry_t *p_entry);
#else//!SL_CATALOG_SL_RAIL_UTIL_PA_PRESENT
void sli_rail_util_on_channel_config_change(RAIL_Handle_t rail_handle,
                                            const RAIL_ChannelConfigEntry_t *p_entry);
#endif//SL_CATALOG_SL_RAIL_UTIL_PA_PRESENT

/**
 * An internal callback, called on registered RAIL events.
 *
 * @param[in] rail_handle The RAIL handle associated with the RAIL event
 * notification.
 * @param[in] events The RAIL events having occurred.
 *
 * @note: This function is not intended for application use. For application
 * use, see \ref sl_rail_util_on_event.
 */
#ifdef  SL_CATALOG_SL_RAIL_UTIL_CALLBACKS_PRESENT
void sli_rail_util_on_event(sl_rail_handle_t rail_handle,
                            sl_rail_events_t events);
#else//!SL_CATALOG_SL_RAIL_UTIL_CALLBACKS_PRESENT
void sli_rail_util_on_event(RAIL_Handle_t rail_handle,
                            RAIL_Events_t events);
#endif//SL_CATALOG_SL_RAIL_UTIL_CALLBACKS_PRESENT

#ifdef __cplusplus
}
#endif

#endif // SLI_RAIL_UTIL_CALLBACKS_H
