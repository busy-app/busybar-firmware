/***************************************************************************//**
 * @file
 * @brief Application Runtime Adaptor (RTA) - Internal header
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
#include <stdint.h>
#include "app_rta.h"
#include "sl_slist.h"

#ifndef APP_RTA_INTERNAL_H
#define APP_RTA_INTERNAL_H

// General runtime data type
typedef void *app_rta_runtime_data_t;

// Context list item type
typedef struct {
  sl_slist_node_t            node;         // List node
  app_rta_config_t           config;       // Saved configuration
  bool                       enabled;      // Context is enabled
  app_rta_runtime_data_t     runtime_data; // Runtime data
} app_rta_internal_context_t;

// Runtime list item type
typedef struct {
  sl_slist_node_t        node;          // List node
  app_rta_priority_t     priority;      // Runtime priority
  size_t                 stack_size;    // Calculated stack size
  sl_slist_node_t        *context_list; // Head of context list
  sl_slist_node_t        *rr_state;     // Current context
  bool                   enabled;       // Runtime is enabled
  bool                   separate;      // Separate
  app_rta_runtime_data_t runtime_data;  // Runtime data
} app_rta_runtime_t;

#ifdef __cplusplus
extern "C" {
#endif

// Runtime list type
typedef sl_slist_node_t *app_rta_runtime_list_t;

/******************************************************************************
 * Start Application Runtime Adaptor (RTA) module.
 * @note Called by the system.
 *****************************************************************************/
void app_rta_internal_init(void);

/******************************************************************************
 * Initialize components that was template contributed to RTA init.
 *
 * It is recommended for contributors to place @ref app_rta_create_context
 * calls here, and also not to call any other App RTA API, since the scheduler
 * has not started yet.
 * @note implementation is automatically generated.
 *****************************************************************************/
void app_rta_init_contributors(void);

/******************************************************************************
 * Finalize initialization for components that were template contributed to
 * RTA ready.
 *
 * At this point the scheduler is started, therefore every API is available.
 * However @ref app_rta_create_context is an exception. It is expected to be
 * only called at @ref app_rta_init_contributors.
 * @note implementation is automatically generated.
 *****************************************************************************/
void app_rta_ready(void);

/******************************************************************************
 * Runtime execution function
 * @param[in] arg Runtime argument
 *****************************************************************************/
void runtime_function(void *arg);

#ifdef __cplusplus
};
#endif

#endif // APP_RTA_INTERNAL_H
