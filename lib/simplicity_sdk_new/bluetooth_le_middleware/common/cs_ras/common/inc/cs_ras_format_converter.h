/***************************************************************************//**
 * @file
 * @brief CS RAS Format converter
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

#ifndef CS_RAS_FORMAT_CONVERTER
#define CS_RAS_FORMAT_CONVERTER

/***********************************************************************************************//**
 * @addtogroup cs_ras
 * @{
 **************************************************************************************************/

#include "cs_ras_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INVALID_ANTENNA_CONF           0xff

/**************************************************************************//**
 * Convert from event data to RAS format as
 * specified by the SIG standard.
 *
 * @param[out] ras_subevent_header  The connection handle.
 * @param[out] ras_header           Filter pattern.
 * @param[in]  cs_res               CS result data.
 * @param[in]  antenna_config       CS antenna configuration.
 * @param[in]  continue_evt         True, if this is a continuation event.
 * @return status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_format_convert_header(cs_ras_subevent_header_t *ras_subevent_header,
                                         cs_ras_ranging_header_t *ras_header,
                                         const sl_bt_msg_t *cs_res,
                                         uint8_t antenna_config,
                                         bool continue_evt);

/**************************************************************************//**
 * Get first subevent header in RAS data
 *
 * @param[in]  data                Data pointer.
 * @param[in]  data_end            End of the data.
 * @param[out] subevent_header_out Subevent header output.
 * @return status of the operation.
 * @retval SL_STATUS_OK First subevent header was found.
 * @retval SL_STATUS_WOULD_OVERFLOW First subevent header does not fit in data.
 * @retval SL_STATUS_NULL_POINTER At least one input pointer is NULL.
 *****************************************************************************/
sl_status_t cs_ras_format_get_first_subevent_header(uint8_t *data,
                                                    uint8_t *data_end,
                                                    cs_ras_subevent_header_t **subevent_header_out);

/**************************************************************************//**
 * Get next subevent header in RAS data
 *
 * @param[in]  subevent_header     Subevent header pointer.
 * @param[in]  is_initiator        True for initiator.
 * @param[in]  antenna_path_num    Number of antenna paths.
 * @param[out] subevent_header_out Subevent header output.
 * @return status of the operation.
 * @retval SL_STATUS_OK Next subevent header was found.
 * @retval SL_STATUS_NOT_FOUND next Subevent header was not found in data.
 * @retval SL_STATUS_WOULD_OVERFLOW Subevent data parsing overflow.
 * @retval SL_STATUS_NULL_POINTER Subevent header, output or data_end is NULL.
 * @retval SL_STATUS_INVALID_PARAMETER Invalid antenna path number specified.
 * @retval SL_STATUS_INVALID_MODE Invalid step mode found in the data.
 *****************************************************************************/
sl_status_t cs_ras_format_get_next_subevent_header(cs_ras_subevent_header_t *subevent_header,
                                                   uint8_t *data_end,
                                                   bool is_initiator,
                                                   uint8_t antenna_path_num,
                                                   cs_ras_subevent_header_t **subevent_header_out);

#ifdef __cplusplus
};
#endif

/** @} (end addtogroup cs_ras) */
#endif
