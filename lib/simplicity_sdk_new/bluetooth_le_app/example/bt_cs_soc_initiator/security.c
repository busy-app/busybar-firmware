/***************************************************************************//**
 * @file
 * @brief Bluetooth security.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "app_config.h"
#include "trace.h"
#include "security.h"

// -----------------------------------------------------------------------------
// Defines
#define APP_SEC_PREFIX                 "[APP] [Security] "
#define NL_SEC                         "\n"
// Fallbacks
#ifndef CS_APP_CAPABILITY
#define CS_APP_CAPABILITY              sl_bt_sm_io_capability_noinputnooutput
#endif // CS_APP_CAPABILITY
#ifndef CS_APP_PASSKEY
#define CS_APP_PASSKEY                 1234
#endif // CS_APP_PASSKEY

// -----------------------------------------------------------------------------
// Static variables
static uint8_t sm_config_flags = 0u;
static uint8_t conn_handle_in_progress = SL_BT_INVALID_CONNECTION_HANDLE;
static enum security_send_confirmation send_confirmation = SECURITY_PROCESS_NOT_STARTED;

// -----------------------------------------------------------------------------
// Public function definitions

sl_status_t on_event_security(const sl_bt_msg_t *evt)
{
  sl_status_t sc = SL_STATUS_OK;

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id:
      #if defined(DELETE_BONDINGS_ON_STARTUP) && (DELETE_BONDINGS_ON_STARTUP == 1)
      // Delete all bondings on startup
      sc = sl_bt_sm_delete_bondings();
      if (sc != SL_STATUS_OK) {
        app_log_error(APP_SEC_PREFIX "Failed to delete bondings, sc = %lu" NL_SEC, sc);
      }
      #endif
      #if defined(SET_DEBUG_MODE) && (SET_DEBUG_MODE == 1)
      // Set the device in debug mode
      // In this mode, the secure connections bonding uses known debug keys
      sc = sl_bt_sm_set_debug_mode();
      if (sc != SL_STATUS_OK) {
        app_log_error(APP_SEC_PREFIX "Failed to set debug mode, sc = %lu" NL_SEC, sc);
      }
      #endif
      // Set security configuration flags
      sc = sl_bt_sm_configure(sm_config_flags,
                              CS_APP_CAPABILITY);
      if (sc != SL_STATUS_OK) {
        log_error(APP_SEC_PREFIX "Failed to configure security management, sc = %lu" NL_SEC, sc);
        return sc;
      }
      // Set bondable mode
      sl_bt_sm_set_bondable_mode(ALLOW_BONDING);
      #if defined(NEW_BOND_REQUIRES_PASSKEY) && (NEW_BOND_REQUIRES_PASSKEY == 1)
      sc = sl_bt_sm_set_passkey(CS_APP_PASSKEY);
      if (sc != SL_STATUS_OK) {
        log_error(APP_SEC_PREFIX "Failed to set passkey, sc = %lu" NL_SEC, sc);
        return sc;
      }
      #endif
      break;
    case sl_bt_evt_sm_bonded_id:
      log_info(APP_SEC_PREFIX "Device bonded" NL_SEC);
      break;
    case sl_bt_evt_sm_bonding_failed_id:
      log_info(APP_SEC_PREFIX "Bonding failed, reason: %u" NL_SEC, evt->data.evt_sm_bonding_failed.reason);
      break;

    case sl_bt_evt_sm_passkey_display_id:
    {
      // Display the passkey on the console
      log_info(APP_SEC_PREFIX "Displaying passkey: %06lu" NL_SEC,
               evt->data.evt_sm_passkey_display.passkey);
    }
    break;
    case sl_bt_evt_sm_passkey_request_id:
    {
      log_info(APP_SEC_PREFIX "Passkey request has arrived" NL_SEC);
      sc = sl_bt_sm_enter_passkey(evt->data.evt_sm_passkey_request.connection, CS_APP_PASSKEY);
      log_info(APP_SEC_PREFIX "Passkey entered %d, sc = %lu" NL_SEC, CS_APP_PASSKEY, sc);
      if (sc != SL_STATUS_OK) {
        log_error(APP_SEC_PREFIX "Failed to enter passkey, sc = %lu" NL_SEC, sc);
        return sc;
      }
    }
    break;
    case sl_bt_evt_sm_confirm_passkey_id:
    {
      // Confirmation request arrived
      log_info(APP_SEC_PREFIX "Confirmation needed for passkey %lu:" NL_SEC, evt->data.evt_sm_confirm_passkey.passkey);
      conn_handle_in_progress = evt->data.evt_sm_confirm_passkey.connection;
      send_confirmation = SECURITY_PROCESS_STARTED;
    }
    break;
    default:
      break;
  }
  return sc;
} // end of on_event_security

void security_set_config_flags()
{
  #if defined(PREFER_AUTENTICATED_PAIRING) && (PREFER_AUTENTICATED_PAIRING == 1)
  sm_config_flags = sm_config_flags | SL_BT_SM_CONFIGURATION_MITM_REQUIRED;
  #endif
  #if defined(NEW_BOND_REQUIRES_PASSKEY) && (NEW_BOND_REQUIRES_PASSKEY == 1)
  sm_config_flags = sm_config_flags | SL_BT_SM_CONFIGURATION_CONNECTIONS_FROM_BONDED_DEVICES_ONLY;
  #endif
  #if !defined(ALLOW_DEBUG_KEYS) || (ALLOW_DEBUG_KEYS == 0)
  sm_config_flags = sm_config_flags | SL_BT_SM_CONFIGURATION_REJECT_DEBUG_KEYS;
  #endif
}

sl_status_t security_send_confirmation(void)
{
  sl_status_t sc = SL_STATUS_OK;

  if (send_confirmation == SECURITY_ALLOW_CONFIRMATION) {
    sc = sl_bt_sm_passkey_confirm(conn_handle_in_progress, 1);
    log_info(APP_SEC_PREFIX "Passkey confirmed." NL_SEC);
    conn_handle_in_progress = SL_BT_INVALID_CONNECTION_HANDLE;
    send_confirmation = SECURITY_PROCESS_NOT_STARTED;
  } else if (send_confirmation == SECURITY_DENY_CONFIRMATION) {
    sc = sl_bt_sm_passkey_confirm(conn_handle_in_progress, 0);
    log_info(APP_SEC_PREFIX "Passkey denied." NL_SEC);
    conn_handle_in_progress = SL_BT_INVALID_CONNECTION_HANDLE;
    send_confirmation = SECURITY_PROCESS_NOT_STARTED;
  }

  return sc;
}

bool security_is_confirmation_in_progress(void)
{
  return (send_confirmation == SECURITY_PROCESS_STARTED);
}

void security_set_confirmation(enum security_send_confirmation confirmation)
{
  send_confirmation = confirmation;
}
