/***************************************************************************//**
 * @file
 * @brief Application Runtime Adaptor (RTA) - Runtime header
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
#ifndef APP_RTA_RUNTIME_H
#define APP_RTA_RUNTIME_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "app_rta_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Initialize the context runtime data using the included configuration.
 * @param[in] ctx                           Reference to the context structure.
 * @return    status of the operation
 * @retval    SL_STATUS_OK                  if operation was successful.
 * @retval    SL_STATUS_ALREADY_INITIALIZED if runtime data is not NULL.
 * @retval    SL_STATUS_ALLOCATION_FAILED   if failed to allocate memory.
 *****************************************************************************/
sl_status_t app_rta_runtime_init_context(app_rta_internal_context_t *ctx);

/******************************************************************************
 * Deinitialize the context runtime data.
 * @param[in] ctx                       Reference to the context structure.
 * @return    status of the operation
 * @retval    SL_STATUS_OK              if operation was successful.
 * @retval    SL_STATUS_NOT_INITIALIZED if runtime data is NULL.
 *****************************************************************************/
sl_status_t app_rta_runtime_deinit_context(app_rta_internal_context_t *ctx);

/******************************************************************************
 * Initialize runtime and common objects.
 * @param[in] runtime                       Reference to the runtime structure.
 * @return    status of the operation
 * @retval    SL_STATUS_OK                  if operation was successful.
 * @retval    SL_STATUS_ALREADY_INITIALIZED if runtime data is not NULL.
 * @retval    SL_STATUS_ALLOCATION_FAILED   if failed to allocate memory.
 *****************************************************************************/
sl_status_t app_rta_runtime_init(app_rta_runtime_t *runtime);

/******************************************************************************
 * Deinitialize runtime and common objects.
 * @param[in] runtime                   Reference to the runtime structure.
 * @return    status of the operation
 * @retval    SL_STATUS_OK              if operation was successful.
 * @retval    SL_STATUS_NOT_INITIALIZED if runtime data is NULL.
 *****************************************************************************/
sl_status_t app_rta_runtime_deinit(app_rta_runtime_t *runtime);

/******************************************************************************
 * Proceed with execution (common runtime and context).
 * @param[in] runtime        Reference to the runtime structure.
 * @param[in] ctx            Reference to the context structure.
 * @return    status of the operation
 * @retval    SL_STATUS_OK   if proceed was successful.
 * @retval    SL_STATUS_FAIL if proceed failed.
 *****************************************************************************/
sl_status_t app_rta_runtime_proceed(app_rta_runtime_t          *runtime,
                                    app_rta_internal_context_t *ctx);

/******************************************************************************
 * Check execution requirement status (proceed) of the context.
 * @param[in] ctx  Reference to the context structure.
 * @return    true if execution is required.
 *****************************************************************************/
bool app_rta_runtime_check_context(app_rta_internal_context_t *ctx);

/******************************************************************************
 * Check common execution requirement status (proceed).
 * @param[in] ctx Reference to the context structure.
 * @return    true if execution is required.
 *****************************************************************************/
bool app_rta_runtime_common_check(app_rta_runtime_t *runtime);

/******************************************************************************
 * Acquire recursive mutex that implements the guard for RTOS.
 * @param[in] ctx  Reference to the context structure.
 * @return    true if operation was successful.
 *****************************************************************************/
bool app_rta_runtime_acquire(app_rta_internal_context_t *ctx);

/******************************************************************************
 * Release recursive mutex that implements the guard for RTOS.
 * @param[in] ctx Reference to the context structure.
 *****************************************************************************/
void app_rta_runtime_release(app_rta_internal_context_t *ctx);

/******************************************************************************
 * Signal runtime context from another context, even from ISR context.
 * @param[in] ctx Reference to the context structure.
 * @return    status of the operation.
 * @retval    SL_STATUS_OK    if a signal was sent.
 * @retval    SL_STATUS_FAIL  if signaling or the necessary proceed failed.
 *****************************************************************************/
sl_status_t app_rta_runtime_signal(app_rta_internal_context_t *ctx);

/******************************************************************************
 * Receive a signal in runtime context.
 * @param[in] ctx Reference to the context structure.
 * @return    status of the operation.
 * @retval    SL_STATUS_OK    if a signal was received.
 * @retval    SL_STATUS_EMPTY if no signal present.
 *****************************************************************************/
sl_status_t app_rta_runtime_signal_check(app_rta_internal_context_t *ctx);

/******************************************************************************
 * Push data into the Application RTA context queue from another context, even
 * from ISR context.
 * @param[in] ctx  Application RTA context handle.
 * @param[in] data Data to send.
 * @param[in] size Size of the data.
 * @return    status of the operation
 * @retval    SL_STATUS_OK   if data was pushed to the queue successfully.
 * @retval    SL_STATUS_FULL if the queue is full.
 *****************************************************************************/
sl_status_t app_rta_runtime_queue_push(app_rta_internal_context_t *ctx,
                                       uint8_t                    *data,
                                       size_t                     size);

/******************************************************************************
 * Read the queue coming from another context, even from ISR context.
 * @param[in]  ctx  Application RTA context handle.
 * @param[in]  data Data arrived.
 * @param[out] size Size of the data.
 * @return     status of the operation
 * @retval     SL_STATUS_OK    if data was received from the queue.
 * @retval     SL_STATUS_EMPTY if no data present in the queue.
 *****************************************************************************/
sl_status_t app_rta_runtime_queue_read(app_rta_internal_context_t *ctx,
                                       uint8_t                    *data,
                                       size_t                     *size);

/******************************************************************************
 * Protect runtime by suspending other tasks.
 * @return     status of the operation
 * @retval     SL_STATUS_OK   successfully entered to the protected section.
 * @retval     SL_STATUS_FAIL entering to protected section failed.
 *****************************************************************************/
sl_status_t app_rta_runtime_protected_begin(void);

/******************************************************************************
 * Enable scheduler and resume other tasks.
 * @return status of the operation
 * @retval SL_STATUS_OK   successfully exited from the protected section.
 * @retval SL_STATUS_FAIL exiting from protected section failed.
 *****************************************************************************/
sl_status_t app_rta_runtime_protected_end(void);

#ifdef __cplusplus
};
#endif

#endif // APP_RTA_RUNTIME_H
