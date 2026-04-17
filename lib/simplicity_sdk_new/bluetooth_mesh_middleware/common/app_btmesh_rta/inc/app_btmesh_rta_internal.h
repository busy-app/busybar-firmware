/***************************************************************************//**
 * @file
 * @brief Bluetooth Mesh Runtime Adaptor internal header
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
#ifndef APP_BTMESH_RTA_INTERNAL_H
#define APP_BTMESH_RTA_INTERNAL_H

// -----------------------------------------------------------------------------
// Includes
#include "app_rta.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************//**
 * Initialize the Runtime Adaptor (RTA) context for Bluetooth Mesh
 ******************************************************************************/
void _app_btmesh_rta_init(void);

/***************************************************************************//**
 * Acquire access to the Bluetooth Mesh RTA context
 *
 * @return    status of the operation
 * @retval    SL_STATUS_OK             if acquire was successful.
 * @retval    SL_STATUS_INVALID_HANDLE if context is not valid.
 * @retval    SL_STATUS_NOT_SUPPORTED  if guard is not supported.
 * @retval    SL_STATUS_INVALID_STATE  if context is not enabled.
 * @retval    SL_STATUS_FAIL           if acquire failed (timed out).
 ******************************************************************************/
sl_status_t _app_btmesh_rta_acquire(void);

/***************************************************************************//**
 * Acquire access to the Bluetooth Mesh RTA context with debug capabilities
 *
 * @return    status of the operation
 * @retval    SL_STATUS_OK             if acquire was successful.
 * @retval    SL_STATUS_INVALID_HANDLE if context is not valid.
 * @retval    SL_STATUS_NOT_SUPPORTED  if guard is not supported.
 * @retval    SL_STATUS_INVALID_STATE  if context is not enabled.
 * @retval    SL_STATUS_FAIL           if acquire failed (timed out).
 ******************************************************************************/
sl_status_t _app_btmesh_rta_acquire_trace(const char* func_name);

/***************************************************************************//**
 * Release access to the Bluetooth Mesh RTA context
 *
 * @return    status of the operation
 * @retval    SL_STATUS_OK             if acquire was successful.
 * @retval    SL_STATUS_INVALID_HANDLE if context is not valid.
 * @retval    SL_STATUS_NOT_SUPPORTED  if guard is not supported.
 * @retval    SL_STATUS_INVALID_STATE  if context is not enabled.
 * @retval    SL_STATUS_FAIL           if acquire failed (timed out).
 ******************************************************************************/
sl_status_t _app_btmesh_rta_release(void);

/***************************************************************************//**
 * Release access to the Bluetooth Mesh RTA context with debug capabilities
 *
 * @return    status of the operation
 * @retval    SL_STATUS_OK             if acquire was successful.
 * @retval    SL_STATUS_INVALID_HANDLE if context is not valid.
 * @retval    SL_STATUS_NOT_SUPPORTED  if guard is not supported.
 * @retval    SL_STATUS_INVALID_STATE  if context is not enabled.
 * @retval    SL_STATUS_FAIL           if acquire failed (timed out).
 ******************************************************************************/
sl_status_t _app_btmesh_rta_release_trace(const char* func_name);

#ifdef __cplusplus
}
#endif

#endif // APP_BTMESH_RTA_INTERNAL_H
