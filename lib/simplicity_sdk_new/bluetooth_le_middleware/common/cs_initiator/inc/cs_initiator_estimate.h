/***************************************************************************//**
 * @file
 * @brief CS initiator - estimate header
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
#ifndef CS_INITIATOR_ESTIMATE_H
#define CS_INITIATOR_ESTIMATE_H

// -----------------------------------------------------------------------------
// Includes

#include <stdint.h>
#include "sl_rtl_clib_api.h"
#include "cs_initiator_client.h"
#include "cs_initiator_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

// -----------------------------------------------------------------------------
// Function declarations

/******************************************************************************
 * Initialize the RTL library
 * @param[in] conn_handle connection handle
 * @param[in] handle RTL library item handle reference
 * @param[in] config RTL library configuration reference
 * @param[out] instance_id RTL library instance identifier reference
 *
 * @return status of the operation
 *****************************************************************************/
enum sl_rtl_error_code rtl_library_init(const uint8_t conn_handle,
                                        sl_rtl_cs_libitem *handle,
                                        rtl_config_t      *config,
                                        uint8_t           *instance_id);

/******************************************************************************
 * Create estimator for the RTL library with the given CS configuration
 * @param[in] conn_handle connection handle
 * @param[in] handle RTL library item handle reference
 * @param[in] config RTL library configuration reference
 * @param[in] cs_parameters Channel Sounding parameters referebce
 * @param[in] cs_mode CS measurement mode
 *
 * @return status of the operation
 *****************************************************************************/
enum sl_rtl_error_code rtl_library_create_estimator(const uint8_t conn_handle,
                                                    sl_rtl_cs_libitem *handle,
                                                    rtl_config_t      *config,
                                                    sl_rtl_cs_params  *cs_parameters,
                                                    const uint8_t     cs_mode,
                                                    const uint8_t     cs_sub_mode);

/******************************************************************************
 * Get the number of tones from the channel map
 * @param[in] ch_map channel map data reference
 * @param[in] ch_map_len channel map length
 *****************************************************************************/
uint32_t get_num_tones_from_channel_map(const uint8_t *ch_map,
                                        const uint32_t ch_map_len);

/******************************************************************************
 * Calculate the distance between the initiator and reflector
 * @param[in] initiator instance reference
 *
 *****************************************************************************/
void calculate_distance(cs_initiator_t *initiator);

#ifdef __cplusplus
}
#endif

#endif // CS_INITIATOR_ESTIMATE_H
