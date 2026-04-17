/***************************************************************************//**
 * @file
 * @brief Bluetooth Mesh Runtime Adaptor header
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
#ifndef APP_BTMESH_RTA_H
#define APP_BTMESH_RTA_H

// -----------------------------------------------------------------------------
// Includes
#include "app_rta.h"
#include "sl_status.h"
#include "app_btmesh_rta_config.h"
#include "app_btmesh_rta_internal.h"
#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************//**
 * @addtogroup app_btmesh_rta
 * @{
 * @details
 * Bluetooth Mesh Application Runtime Adaptor (RTA) provides a central resource
 * guarding service for Bluetooth Mesh SDK App components. Component guard
 * functions offer an abstraction layer for resource protection in baremetal
 * or RTOS systems, so the components can be system-agnostic (baremetal, RTOS).
 * BT Mesh RTA utilizes the Application Runtime Adaptor (app_rta) component to hide the
 * differences between RTOS and baremetal systems. Consequently, the guard
 * is implemented by a single mutex under RTOS, and it is NOP for baremetal
 * projects.
 *
 * Advantages opposed to component-wise guard by app_rta:
 *  - RAM consumption (app_rta structures & RTOS mutex)
 *      * The app_rta has RAM consumption overhead in baremetal projects as well, which
 *        can be critical on xG22 devices
 *  - Deadlock-free among BT Mesh SDK App components
 *
 * Disadvantages opposed to component-wise guard by app_rta:
 *  - Single guard could block parallel execution
 *      * The Bluetooth Mesh stack API has its own singleton central guard as well,
 *        and therefore parallel execution is blocked anyway
 *      * It is possible to use separate app_rta guard for one or more components,
 *        when it is justified.
 ******************************************************************************/

/***************************************************************************//**
 * Error callback function for Bluetooth Mesh RTA
 *
 * This is a callback which can be implemented in the application
 * @note If no implementation is provided in the application,
 *       then the default weak implementation only handles the failed
 *       runtime initialization by an assertion
 * @param[in] error   error code
 * @param[in] result  result code
 ******************************************************************************/
void app_btmesh_rta_on_runtime_error(app_rta_error_t error,
                                     sl_status_t     result);

#if APP_BTMESH_RTA_ACQUIRE_TIMEOUT_TRACE_ASSERT \
  || APP_BTMESH_RTA_ACQUIRE_TIMEOUT_TRACE_LOG   \
  || APP_BTMESH_RTA_GUARD_TRACE_LOG

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
#define app_btmesh_rta_acquire() _app_btmesh_rta_acquire_trace(__func__)

#else

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
#define app_btmesh_rta_acquire() _app_btmesh_rta_acquire()

#endif

#if APP_BTMESH_RTA_GUARD_TRACE_LOG

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
#define app_btmesh_rta_release() _app_btmesh_rta_release_trace(__func__)

#else

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
#define app_btmesh_rta_release() _app_btmesh_rta_release()

#endif

/** @} (end addtogroup app_btmesh_rta) */

#ifdef __cplusplus
}
#endif

#endif // APP_BTMESH_RTA_H
