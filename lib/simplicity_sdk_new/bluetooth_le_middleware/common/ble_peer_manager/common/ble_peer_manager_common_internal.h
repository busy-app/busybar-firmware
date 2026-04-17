/***************************************************************************//**
 * @file
 * @brief Bluetooth Peer Manager - common internal header file
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
#ifndef BLE_PEER_MANAGER_COMMON_INTERNAL_H
#define BLE_PEER_MANAGER_COMMON_INTERNAL_H

#include "sl_bt_api.h"
#include "ble_peer_manager_common_config.h"
#if defined(SL_CATALOG_APP_LOG_PRESENT) && BLE_PEER_MANAGER_COMMON_LOG
  #include "app_log.h"
#endif // defined(SL_CATALOG_APP_LOG_PRESENT) && defined(BLE_PEER_MANAGER_COMMON_LOG)

// -----------------------------------------------------------------------------
// Defines
#if defined(SL_CATALOG_APP_LOG_PRESENT) && BLE_PEER_MANAGER_COMMON_LOG
#define PM_PREFIX                                 BLE_PEER_MANAGER_COMMON_LOG_PREFIX " "
#define ble_peer_manager_log_debug(...)           app_log_debug(PM_PREFIX __VA_ARGS__)
#define ble_peer_manager_log_info(...)            app_log_info(PM_PREFIX __VA_ARGS__)
#define ble_peer_manager_log_error(...)           app_log_error(PM_PREFIX __VA_ARGS__)
#define ble_peer_manager_log_hexdump(p_data, len) \
  do {                                            \
    app_log_append(PM_PREFIX);                    \
    app_log_hexdump_debug(p_data, len);           \
  } while (0);
#else
#define ble_peer_manager_log_debug(...)
#define ble_peer_manager_log_info(...)
#define ble_peer_manager_log_error(...)
#define ble_peer_manager_log_hexdump(p_data, len)
#endif

#endif // BLE_PEER_MANAGER_COMMON_INTERNAL_H
