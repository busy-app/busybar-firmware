/***************************************************************************//**
 * @file
 * @brief Extended result event serializer.
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

#ifndef EXTENDED_RESULT_H
#define EXTENDED_RESULT_H

#include "sl_rtl_clib_api.h"
#include "cs_initiator.h"

/**************************************************************************//**
 * Add extended result data to the ACP event buffer.
 *****************************************************************************/
void cs_on_extended_result(const uint8_t conn_handle,
                           const uint16_t ranging_counter,
                           const uint8_t *result,
                           const cs_result_session_data_t *result_data,
                           const cs_ranging_data_t *ranging_data,
                           const void *user_data);

/**************************************************************************//**
 * Extended result event serializer task.
 *****************************************************************************/
void extended_result_step(void);

#endif // EXTENDED_RESULT_H
