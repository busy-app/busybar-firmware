/***************************************************************************//**
 * @file
 * @brief CS RAS Client - RAS Control Point header
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

#ifndef CS_RAS_CLIENT_CONTROL_POINT_H
#define CS_RAS_CLIENT_CONTROL_POINT_H

#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"
#include "cs_ras_common.h"
#include "cs_ras_client_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * Request abort using RAS Control Point.
 * @param[in] client Client reference.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_control_point_abort(cs_ras_client_t *client);

/**************************************************************************//**
 * Request acknowledging of ranging data using RAS Control Point.
 * @param[in] client          Client reference.
 * @param[in] ranging_counter Ranging counter of the data to acknowledge.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_control_point_ack(cs_ras_client_t          *client,
                                            cs_ras_ranging_counter_t ranging_counter);

/**************************************************************************//**
 * Request receive (get) ranging data using RAS Control Point.
 * @param[in] client          Client reference.
 * @param[in] ranging_counter Ranging counter of the data to receive.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_control_point_get_ranging_data(cs_ras_client_t          *client,
                                                         cs_ras_ranging_counter_t ranging_counter);

/**************************************************************************//**
 * Request retrieval of lost ranging data segments using RAS Control Point.
 * @param[in] client          Client reference.
 * @param[in] ranging_counter Ranging counter of the data to receive.
 * @param[in] start_segment   Start segment.
 * @param[in] end_segment     End segment.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_control_point_retreive_lost_segments(cs_ras_client_t          *client,
                                                               cs_ras_ranging_counter_t ranging_counter,
                                                               uint8_t                  start_segment,
                                                               uint8_t                  end_segment);

/**************************************************************************//**
 * Bluetooth stack event handler.
 * @param[in] evt Event coming from the Bluetooth stack.
 * @return true to send the event to the host in NCP case.
 *****************************************************************************/
bool cs_ras_client_control_point_on_bt_event(sl_bt_msg_t *evt);

#ifdef __cplusplus
};
#endif

#endif // CS_RAS_CLIENT_CONTROL_POINT_H
