/***************************************************************************//**
 * @file
 * @brief Application Runtime Adaptor (RTA) - List management header
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
#ifndef APP_RTA_LIST_H
#define APP_RTA_LIST_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "app_rta_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Initialize runtime list.
 *****************************************************************************/
void app_rta_list_init(void);

/******************************************************************************
 * Complete initialization.
 *****************************************************************************/
void app_rta_list_complete(void);

/******************************************************************************
 * Add a context with a priority: create runtime if required.
 * @param[in] ctx Runtime context reference.
 * @return status of the operation
 *****************************************************************************/
sl_status_t app_rta_list_add_context(app_rta_internal_context_t *ctx);

/******************************************************************************
 * Enable or disable context.
 * @note Initializes context and runtime on demand if enabling is required.
 * @param[in] ctx    Runtime context reference.
 * @param[in] enable True to enable or false to disable context.
 * @return status of the operation
 *****************************************************************************/
sl_status_t app_rta_list_enable_context(app_rta_internal_context_t *ctx,
                                        bool enable);

/******************************************************************************
 * Search for a runtime.
 * @param[in] ctx Runtime context reference.
 * @return runtime reference or NULL if not found.
 *****************************************************************************/
app_rta_runtime_t *app_rta_list_find_runtime(app_rta_internal_context_t *ctx);

/******************************************************************************
 * Get the the next context for execution.
 * @param[in] runtime Runtime reference.
 * @return runtime reference or NULL if no context is found to execute.
 *****************************************************************************/
app_rta_internal_context_t *app_rta_list_rr_next(app_rta_runtime_t *runtime);

#ifdef __cplusplus
};
#endif

#endif // APP_RTA_LIST_H
