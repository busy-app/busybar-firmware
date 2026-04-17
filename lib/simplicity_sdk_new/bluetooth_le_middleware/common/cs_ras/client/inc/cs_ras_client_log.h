/***************************************************************************//**
 * @file
 * @brief CS RAS Client - Log header
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

#ifndef CS_RAS_CLIENT_LOG_H
#define CS_RAS_CLIENT_LOG_H

#include "cs_ras_client_config.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_APP_LOG_PRESENT
#include "app_log.h"
#endif // SL_CATALOG_APP_LOG_PRESENT

#ifdef __cplusplus
extern "C" {
#endif

// Log API
#if defined(SL_CATALOG_APP_LOG_PRESENT) && defined(CS_RAS_CLIENT_CONFIG_LOG_ENABLE) && CS_RAS_CLIENT_CONFIG_LOG_ENABLE
#define LOG_PREFIX                  "[RAS_C] "
#define LOG_NL                      APP_LOG_NL
#define CONN_PREFIX                 "[%u] "
#define cs_ras_client_log_debug(...)    app_log_debug(LOG_PREFIX  __VA_ARGS__)
#define cs_ras_client_log_info(...)     app_log_info(LOG_PREFIX  __VA_ARGS__)
#define cs_ras_client_log_warning(...)  app_log_warning(LOG_PREFIX  __VA_ARGS__)
#define cs_ras_client_log_error(...)    app_log_error(LOG_PREFIX  __VA_ARGS__)
#define cs_ras_client_log_critical(...) app_log_critical(LOG_PREFIX  __VA_ARGS__)
#else
#define LOG_NL
#define INSTANCE_PREFIX
#define cs_ras_client_log_debug(...)
#define cs_ras_client_log_info(...)
#define cs_ras_client_log_warning(...)
#define cs_ras_client_log_error(...)
#define cs_ras_client_log_critical(...)
#endif

#ifdef __cplusplus
};
#endif

#endif // CS_RAS_CLIENT_LOG_H
