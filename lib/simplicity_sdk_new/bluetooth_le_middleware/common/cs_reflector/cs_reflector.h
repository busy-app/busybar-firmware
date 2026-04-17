/***************************************************************************//**
 * @file
 * @brief CS Reflector
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

#ifndef CS_REFLECTOR_H
#define CS_REFLECTOR_H

/***********************************************************************************************//**
 * @addtogroup cs_reflector
 * @{
 **************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "sl_bt_api.h"
#include "sl_status.h"

typedef PACKSTRUCT (struct {
  int8_t max_tx_power_dbm;
  uint8_t cs_sync_antenna;
}) cs_reflector_config_t;

/**************************************************************************//**
 * Determine whether a connection handle has a reflector associated to it
 *
 * @param[in] conn_handle the connection handle to identify
 *
 * @return `true` if the handle has an associated reflector instance,
 *         `false` otherwise
 *****************************************************************************/
bool cs_reflector_identify(uint8_t conn_handle);

/**************************************************************************//**
 * Create a reflector for the given connection handle
 *
 * @param[in] conn_handle the connection handle for which to create the reflector
 * @param[in] reflector_config reflector configuration parameters
 *
 * @return status of the operation
 *****************************************************************************/
sl_status_t cs_reflector_create(uint8_t               conn_handle,
                                cs_reflector_config_t *reflector_config);

/**************************************************************************//**
 * Get the number of active reflector instances
 *
 * @return the number of active reflector instances
 *****************************************************************************/
uint8_t cs_reflector_get_active_instance_count(void);

/**************************************************************************//**
 * Delete the reflector associated with the given connection handle
 *
 * @param[in] conn_handle the connection handle for which to delete the reflector
 *
 * @return status of the operation
 *****************************************************************************/
sl_status_t cs_reflector_delete(uint8_t conn_handle);

/**************************************************************************//**
 * Handle a Bluetooth stack event
 *
 * @param[in] evt the Bluetooth stack event to handle
 *
 * @return true to forward the message to the host in NCP case
 *****************************************************************************/
bool cs_reflector_on_bt_event(sl_bt_msg_t *evt);

/** @} (end addtogroup cs_reflector) */
#endif // CS_REFLECTOR_H
