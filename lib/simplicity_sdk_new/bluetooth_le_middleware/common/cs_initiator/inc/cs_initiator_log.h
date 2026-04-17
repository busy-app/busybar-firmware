/***************************************************************************//**
 * @file
 * @brief CS initiator - logging interface header
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

#ifndef CS_INITIATOR_LOG_H
#define CS_INITIATOR_LOG_H

#include "sl_component_catalog.h"
#include "cs_initiator_config.h"

#if defined(SL_CATALOG_APP_LOG_PRESENT) && CS_INITIATOR_LOG
#include "app_log.h"
#define LOG_PREFIX                  CS_INITIATOR_LOG_PREFIX " "
#define LOG_NL                      APP_LOG_NL
#define INSTANCE_PREFIX             "[%u] "
#define initiator_log_wrap(code)  code
#else
#define initiator_log_wrap(...)
#endif // defined(SL_CATALOG_APP_LOG_PRESENT) && CS_INITIATOR_LOG

#define initiator_log_debug(...)         initiator_log_wrap(app_log_debug(LOG_PREFIX  __VA_ARGS__))
#define initiator_log_info(...)          initiator_log_wrap(app_log_info(LOG_PREFIX  __VA_ARGS__))
#define initiator_log_warning(...)       initiator_log_wrap(app_log_warning(LOG_PREFIX  __VA_ARGS__))
#define initiator_log_error(...)         initiator_log_wrap(app_log_error(LOG_PREFIX  __VA_ARGS__))
#define initiator_log_critical(...)      initiator_log_wrap(app_log_critical(LOG_PREFIX  __VA_ARGS__))
#define initiator_log_hexdump_debug(...) initiator_log_wrap(app_log_hexdump_debug(__VA_ARGS__))
#define initiator_log_append_debug(...) initiator_log_wrap(app_log_append_debug(__VA_ARGS__))

#endif // CS_INITIATOR_LOG_H
