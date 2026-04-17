/***************************************************************************//**
 * @file
 * @brief CS RAS Client - Internal header
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

#ifndef CS_RAS_CLIENT_INTERNAL_H
#define CS_RAS_CLIENT_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"
#include "sl_bt_api.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif // SL_CATALOG_POWER_MANAGER_PRESENT
#include "cs_ras_common.h"
#include "cs_ras_client_messaging.h"
#include "app_timer.h"

#define CS_RAS_CLIENT_INTER_EVENT_TIMEOUT_MS                     1000
#define CS_RAS_CLIENT_GET_ON_DEMAND_RANGING_DATA_TIMEOUT_MS      5000

// RAS client state
typedef enum {
  CLIENT_STATE_NOT_INITIALIZED = 0,
  CLIENT_STATE_READ_FEATURES,
  CLIENT_STATE_INITIALIZED,
  CLIENT_STATE_SELECT_ON_DEMAND_MODE_UNSUBSCRIBE,
  CLIENT_STATE_SELECT_ON_DEMAND_MODE_SUBSCRIBE,
  CLIENT_STATE_ON_DEMAND,
  CLIENT_STATE_SELECT_REAL_TIME_MODE_UNSUBSCRIBE,
  CLIENT_STATE_SELECT_REAL_TIME_MODE_SUBSCRIBE,
  CLIENT_STATE_REAL_TIME,
  CLIENT_STATE_DESELECT_MODE
} cs_ras_client_state_t;

// CCCD configuration
typedef struct {
  sl_bt_gatt_client_config_flag_t array[CS_RAS_CHARACTERISTIC_INDEX_COUNT];
} cccd_config_t;

// Client subscription storage type
typedef struct {
  cs_ras_characteristic_index_t current_index;
  cs_ras_characteristic_index_t mode_index;
  uint16_t request;
  struct {
    cccd_config_t config;
    cccd_config_t state;
    sl_bt_gatt_client_config_flag_t request;
  } cccd;
} subscription_t;

// RAS client structure
typedef struct {
  cs_ras_client_state_t state;
  subscription_t subscription;
  cs_ras_gattdb_handles_t *handles;
  uint16_t att_mtu;
  uint8_t connection;
  cs_ras_features_t features;
  bool features_read;
  cs_ras_cp_opcode_t op_code;
  bool operation;
  bool abort;
  cs_ras_ranging_counter_t ranging_counter;
  cs_ras_client_messaging_reception_t messaging;
  struct {
    app_timer_t data_arrived;
    app_timer_t data_ready;
    app_timer_t control_point;
  } timer;
} cs_ras_client_t;

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * Initialize RAS Client.
 *****************************************************************************/
void cs_ras_client_init(void);

/**************************************************************************//**
 * Find client instance based on connection handle
 * @param[in] connection Connection handle
 * @return Instance reference or NULL if not found.
 *****************************************************************************/
cs_ras_client_t *cs_ras_client_find(uint8_t connection);

/**************************************************************************//**
 * Find client instance based on its timer
 * @param[in] timer Timer reference
 * @return Instance reference or NULL if not found.
 *****************************************************************************/
cs_ras_client_t *cs_ras_client_find_by_timer(app_timer_t *timer);

/**************************************************************************//**
 * Bluetooth stack event handler.
 * @param[in] evt Event coming from the Bluetooth stack.
 * @return true to send the event to the host in NCP case.
 *****************************************************************************/
bool cs_ras_client_on_bt_event(sl_bt_msg_t *evt);

#ifdef __cplusplus
};
#endif

#endif // CS_RAS_CLIENT_INTERNAL_H
