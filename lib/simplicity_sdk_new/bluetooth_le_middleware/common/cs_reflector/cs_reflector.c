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

#include "sl_bt_api.h"
#include "app_assert.h"
#include "app_log.h"
#include "cs_ras_server.h"
#include "cs_reflector.h"
#include "cs_reflector_config.h"
#include "cs_reflector_connmanager.h"
#include "sl_component_catalog.h"
#include "cs_sync_antenna.h"
#ifdef SL_CATALOG_APP_LOG_PRESENT
#include "app_log.h"
#endif // SL_CATALOG_APP_LOG_PRESENT

// -----------------------------------------------------------------------------
// Macros

// Component logging
#if defined(SL_CATALOG_APP_LOG_PRESENT) && CS_REFLECTOR_LOG
#define LOG_PREFIX                  CS_REFLECTOR_LOG_PREFIX " "
#define NL                          APP_LOG_NL
#define INSTANCE_PREFIX             "[%u] "
#define reflector_log_debug(...)    app_log_debug(LOG_PREFIX  __VA_ARGS__)
#define reflector_log_info(...)     app_log_info(LOG_PREFIX  __VA_ARGS__)
#define reflector_log_warning(...)  app_log_warning(LOG_PREFIX  __VA_ARGS__)
#define reflector_log_error(...)    app_log_error(LOG_PREFIX  __VA_ARGS__)
#define reflector_log_critical(...) app_log_critical(LOG_PREFIX  __VA_ARGS__)
#else
#define NL
#define INSTANCE_PREFIX
#define reflector_log_debug(...)
#define reflector_log_info(...)
#define reflector_log_warning(...)
#define reflector_log_error(...)
#define reflector_log_critical(...)
#endif

// -----------------------------------------------------------------------------
// Public function definitions

bool cs_reflector_identify(uint8_t conn_handle)
{
  connection_ctx_t *conn_ctx = cs_rcm_get_connection_ctx(conn_handle);
  if (conn_ctx == NULL) {
    return false;
  }
  return true;
}

sl_status_t cs_reflector_create(uint8_t               conn_handle,
                                cs_reflector_config_t *reflector_config)
{
  sl_status_t sc;
  uint8_t num_antennas;
  reflector_log_info(INSTANCE_PREFIX "Creating new reflector instance; conn_handle='%u'" NL,
                     conn_handle,
                     conn_handle);

  // Prepare for the CS main mode: RTT antenna usage
  sc = sl_bt_cs_read_local_supported_capabilities(NULL,
                                                  NULL,
                                                  &num_antennas,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL);

  if (sc != SL_STATUS_OK) {
    return sc;
  }

  switch (reflector_config->cs_sync_antenna) {
    case CS_SYNC_ANTENNA_1:
      reflector_log_info(INSTANCE_PREFIX "CS - RTT - Using the antenna ID 1" NL,
                         conn_handle);
      break;
    case CS_SYNC_ANTENNA_2:
      if (num_antennas >= 2) {
        reflector_log_info(INSTANCE_PREFIX "CS - RTT - 2-antenna device! Using the antenna ID 2" NL,
                           conn_handle);
      } else {
        reflector_log_info(INSTANCE_PREFIX "CS - RTT - only 1-antenna device! Using the antenna ID 1" NL,
                           conn_handle);
        reflector_config->cs_sync_antenna = CS_SYNC_ANTENNA_1;
      }
      break;
    case CS_SYNC_SWITCHING:
      reflector_log_info(INSTANCE_PREFIX "CS - RTT - switching between %u available antennas" NL,
                         conn_handle,
                         num_antennas);
      reflector_config->cs_sync_antenna = CS_SYNC_SWITCHING;
      break;
    default:
      reflector_log_info(INSTANCE_PREFIX "CS - RTT - unknown antenna usage! "
                                         "Using the default setting: antenna ID 1" NL,
                         conn_handle);
      reflector_config->cs_sync_antenna = CS_SYNC_ANTENNA_1;
      break;
  }

  sc = sl_bt_cs_set_default_settings(conn_handle,
                                     sl_bt_cs_role_status_disable,
                                     sl_bt_cs_role_status_enable,
                                     reflector_config->cs_sync_antenna,
                                     reflector_config->max_tx_power_dbm);
  if (sc != SL_STATUS_OK) {
    return sc;
  }

  // Add the new instance
  sc = cs_rcm_add_new_initiator_connection(conn_handle);
  if (sc != SL_STATUS_OK) {
    // New instance cannot be added - we're at capacity
    reflector_log_warning(INSTANCE_PREFIX "Cannot add new reflector connection" NL,
                          conn_handle);
    return SL_STATUS_ALLOCATION_FAILED;
  }

  reflector_log_info("Instances: %u/%u" NL,
                     cs_rcm_get_number_of_connections(),
                     SL_BT_CONFIG_MAX_CONNECTIONS);
  return sc;
}

uint8_t cs_reflector_get_active_instance_count(void)
{
  return cs_rcm_get_number_of_connections();
}

sl_status_t cs_reflector_delete(uint8_t conn_handle)
{
  if (!cs_reflector_identify(conn_handle)) {
    return SL_STATUS_NOT_FOUND;
  }
  reflector_log_info(INSTANCE_PREFIX "Deleting instance" NL, conn_handle);
  cs_rcm_remove_initiator_connection(conn_handle);
  reflector_log_info("Instances: %u/%u" NL,
                     cs_rcm_get_number_of_connections(),
                     SL_BT_CONFIG_MAX_CONNECTIONS);
  return SL_STATUS_OK;
}

void cs_ras_server_on_mode_change(uint8_t       connection,
                                  cs_ras_mode_t mode,
                                  bool          indication)
{
  reflector_log_debug(INSTANCE_PREFIX "RAS mode changed to %u" NL, connection, mode);
  (void)connection;
  (void)mode;
  (void)indication;
}

// -----------------------------------------------------------------------------
// Event / callback definitions

bool cs_reflector_on_bt_event(sl_bt_msg_t *evt)
{
  bool handled = false;
  sl_status_t sc;
  (void)sc;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates that a CS procedure was started by the initiator
    case sl_bt_evt_cs_procedure_enable_complete_id:
    {
      uint8_t conn_handle = evt->data.evt_cs_procedure_enable_complete.connection;
      if (!cs_reflector_identify(conn_handle)) {
        break;
      }
      // Set message handled
      handled = true;
      if (evt->data.evt_cs_procedure_enable_complete.status == SL_STATUS_OK) {
        if (evt->data.evt_cs_procedure_enable_complete.state
            == sl_bt_cs_procedure_state_disabled) {
          reflector_log_debug(INSTANCE_PREFIX "CS procedure disabled" NL,
                              conn_handle);
          reflector_log_debug("-------------------------------------------------------" NL);
          break;
        }
        reflector_log_debug("-------------------------------------------------------" NL);
        reflector_log_debug(INSTANCE_PREFIX "CS procedure enabled" NL,
                            conn_handle);
        reflector_log_debug(INSTANCE_PREFIX "Antenna config: %u" NL,
                            conn_handle,
                            evt->data.evt_cs_procedure_enable_complete.antenna_config);
      } else {
        reflector_log_warning(INSTANCE_PREFIX "CS procedure failed, status = 0x%04x, state = %u" NL,
                              conn_handle,
                              evt->data.evt_cs_procedure_enable_complete.status,
                              evt->data.evt_cs_procedure_enable_complete.state);
      }
    }
    break;

    // -------------------------------
    // This event indicates that the Channel Sounding Security Enable procedure has completed
    case sl_bt_evt_cs_security_enable_complete_id:
    {
      uint8_t conn_handle = evt->data.evt_cs_security_enable_complete.connection;
      if (!cs_reflector_identify(conn_handle)) {
        break;
      }
      // Set message handled
      handled = true;
      reflector_log_info(INSTANCE_PREFIX "CS Security Enable completed" NL, conn_handle);
    }
    break;

    // -------------------------------
    // This event indicates that the Channel Sounding configuration was completed
    case sl_bt_evt_cs_config_complete_id:
    {
      uint8_t conn_handle = evt->data.evt_cs_config_complete.connection;
      if (!cs_reflector_identify(conn_handle)) {
        break;
      }
      // Set message handled
      handled = true;
      reflector_log_info(INSTANCE_PREFIX "CS configuration completed" NL, conn_handle);
    }
    break;
  }
  // Return false if the message was handled above
  return !handled;
}
