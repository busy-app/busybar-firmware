/***************************************************************************//**
 * @file
 * @brief CS RAS Server - Internal header
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

#ifndef CS_RAS_SERVER_INTERNAL_H
#define CS_RAS_SERVER_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"
#include "sl_bt_api.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif // SL_CATALOG_POWER_MANAGER_PRESENT
#include "cs_ras_server_config.h"
#include "cs_ras_common.h"
#include "cs_ras_server_messaging.h"
#include "app_queue.h"
#include "app_timer.h"

#if defined(CS_RAS_SERVER_CONFIG_SUPPORT_FEATURE_REAL_TIME_RANGING_DATA) && (CS_RAS_SERVER_CONFIG_SUPPORT_FEATURE_REAL_TIME_RANGING_DATA == 1)
#define FEATURE_REAL_TIME_RANGING_DATA
#endif // defined(CS_RAS_SERVER_CONFIG_SUPPORT_FEATURE_REAL_TIME_RANGING_DATA) && (CS_RAS_SERVER_CONFIG_SUPPORT_FEATURE_REAL_TIME_RANGING_DATA == 1)
#if defined(CS_RAS_SERVER_CONFIG_SUPPORT_FEATURE_RETRIEVE_LOST_RANGING_DATA_SEGMENTS) && (CS_RAS_SERVER_CONFIG_SUPPORT_FEATURE_RETRIEVE_LOST_RANGING_DATA_SEGMENTS == 1)
#define FEATURE_RETRIEVE_LOST_SEGMENTS
#endif // defined(CS_RAS_SERVER_CONFIG_SUPPORT_FEATURE_RETRIEVE_LOST_RANGING_DATA_SEGMENTS) && (CS_RAS_SERVER_CONFIG_SUPPORT_FEATURE_RETRIEVE_LOST_RANGING_DATA_SEGMENTS == 1)
#if defined(CS_RAS_SERVER_CONFIG_SUPPORT_FEATURE_ABORT) && (CS_RAS_SERVER_CONFIG_SUPPORT_FEATURE_ABORT == 1)
#define FEATURE_ABORT
#endif // defined(CS_RAS_SERVER_CONFIG_SUPPORT_FEATURE_ABORT) && (CS_RAS_SERVER_CONFIG_SUPPORT_FEATURE_ABORT == 1))
#if defined(CS_RAS_SERVER_CONFIG_DATA_READY_NOTIFICATIONS) && (CS_RAS_SERVER_CONFIG_DATA_READY_NOTIFICATIONS == 1)
#define FEATURE_DATA_READY_NOTIFICATIONS
#endif // defined(CS_RAS_SERVER_CONFIG_DATA_READY_NOTIFICATIONS) && (CS_RAS_SERVER_CONFIG_DATA_READY_NOTIFICATIONS == 1)
#if defined(CS_RAS_SERVER_CONFIG_OVERWRITTEN_NOTIFICATIONS) && (CS_RAS_SERVER_CONFIG_OVERWRITTEN_NOTIFICATIONS == 1)
#define FEATURE_DATA_OVERWRITTEN_NOTIFICATIONS
#endif // defined(CS_RAS_SERVER_CONFIG_OVERWRITTEN_NOTIFICATIONS) && (CS_RAS_SERVER_CONFIG_OVERWRITTEN_NOTIFICATIONS == 1)
#if defined(CS_RAS_SERVER_CONFIG_DATA_READY_READ) && CS_RAS_SERVER_CONFIG_DATA_READY_READ == 1
#define FEATURE_DATA_READY_READ
#endif // defined(CS_RAS_SERVER_CONFIG_DATA_READY_READ) && CS_RAS_SERVER_CONFIG_DATA_READY_READ == 1
#if defined(CS_RAS_SERVER_CONFIG_OVERWRITTEN_READ) && CS_RAS_SERVER_CONFIG_OVERWRITTEN_READ == 1
#define FEATURE_DATA_OVERWRITTEN_READ
#endif // defined(CS_RAS_SERVER_CONFIG_OVERWRITTEN_READ) && CS_RAS_SERVER_CONFIG_OVERWRITTEN_READ == 1

// Response timeout
#define CS_RAS_SERVER_RESPONSE_TIMEOUT_MS      5000

// RAS Server internal state type
typedef enum {
  SERVER_STATE_IDLE,
  SERVER_STATE_IN_CP_PROCEDURE,
} cs_ras_server_state_t;

typedef struct {
  bool     indication;
  uint16_t characteristic;
  uint8_t  size;
  uint8_t  data[sizeof(cs_ras_cp_response_t)];
} cs_ras_server_queue_item_t;

typedef struct {
  cs_ras_server_state_t state;
  cs_ras_cp_opcode_t op_code;
  cs_ras_ranging_counter_t ranging_counter;
  cs_ras_ranging_counter_t data_ready_counter;
  cs_ras_ranging_counter_t data_overwritten_counter;
  struct {
    uint16_t control_point            : 1;
    uint16_t data_ready_indication    : 1;
    uint16_t data_ready_notification  : 1;
    uint16_t overwritten_indication   : 1;
    uint16_t overwritten_notification : 1;
    uint16_t real_time_indication     : 1;
    uint16_t real_time_notification   : 1;
    uint16_t on_demand_indication     : 1;
    uint16_t on_demand_notification   : 1;
  } cccd;
  cs_ras_server_messaging_transmit_t transmit;
  app_queue_t tx_queue;
  struct {
    app_timer_t retention;
    app_timer_t response;
  } timer;
  cs_ras_server_queue_item_t tx_queue_data[CS_RAS_SERVER_CONFIG_TX_QUEUE_SIZE];
  uint16_t att_mtu;
  uint8_t connection;
  int8_t tx_power_dbm;
  bool transmit_request;
  uint8_t antenna_config;
} cs_ras_server_t;

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * Initialize RAS Server.
 *****************************************************************************/
void cs_ras_server_init(void);

/**************************************************************************//**
 * Step function (process action).
 *****************************************************************************/
void cs_ras_server_step(void);

/**************************************************************************//**
 * Find server instance based on connection handle
 * @param[in] connection Connection handle
 * @return Instance reference or NULL if not found.
 *****************************************************************************/
cs_ras_server_t *cs_ras_server_find(uint8_t connection);

/**************************************************************************//**
 * Get ranging data for a given ranging counter value.
 * @param[in]  server  Server instance
 * @param[in]  counter Ranging counter
 * @param[out] sent    Data for the ranging counter has already been sent.
 *                     Pass NULL if not required.
 * @param[out] data    Pointer to the Ranging Data. Pass NULL if not required.
 * @param[out] size    Size of the Ranging Data. Pass NULL if not required.
 * @return true if the ranging data was found.
 *****************************************************************************/
bool cs_ras_server_get_ranging_data(cs_ras_server_t           *server,
                                    cs_ras_ranging_counter_t  counter,
                                    bool                      *sent,
                                    uint8_t                   **data,
                                    uint32_t                  *size);

/**************************************************************************//**
 * Delete ranging data for a ranging counter value.
 * @param[in]  server  Server instance
 * @param[in]  counter Ranging counter
 * @return true if the ranging data was deleted successfully.
 *****************************************************************************/
bool cs_ras_server_delete_ranging_data(cs_ras_server_t          *server,
                                       cs_ras_ranging_counter_t counter);

/**************************************************************************//**
 * Register that ranging data for a ranging counter value has been sent.
 * @param[in]  server  Server instance
 * @param[in]  counter Ranging counter
 * @return true if the ranging data was registered as sent successfully.
 *****************************************************************************/
bool cs_ras_server_ranging_data_sent(cs_ras_server_t          *server,
                                     cs_ras_ranging_counter_t counter);

/**************************************************************************//**
 * Request to send indication/notification
 *
 * @note Try to send indication or notification immediately but if the TX
 *       queue is not empty, or the instant transmit fails temporarily,
 *       the request will be added into the TX queue and sent later on.
 *
 * @param[in]  server         Server instance.
 * @param[in]  indication     True for indication, false for notification.
 * @param[in]  characteristic Characteristic handle.
 * @param[in]  data_size      Size of the data to send.
 * @param[in]  data           Data to send.
 * @return status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_server_send(cs_ras_server_t *server,
                               bool            indication,
                               uint16_t        characteristic,
                               uint8_t         data_size,
                               uint8_t         *data);

/**************************************************************************//**
 * Get current mode of a RAS server.
 * @param[in]  server     Server reference.
 * @param[out] mode       Ranging mode.
 * @param[out] indication true for indications, false for notifications
 *****************************************************************************/
void cs_ras_server_get_mode(cs_ras_server_t *server,
                            cs_ras_mode_t   *mode,
                            bool            *indication);

/**************************************************************************//**
 * Bluetooth event handler of the RAS Server.
 * @param[in] evt Event coming from the Bluetooth stack.
 * @return true to send the event to the host in NCP case.
 *****************************************************************************/
bool cs_ras_server_on_bt_event(sl_bt_msg_t *evt);

/**************************************************************************//**
 * Send RAS data when ready.
 * @param[in] connection Connection handle.
 * @param[in] ranging_counter Ranging counter.
 * @return status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_send_data_ready(uint8_t connection,
                                   cs_ras_ranging_counter_t ranging_counter);

/**************************************************************************//**
 * Send RAS data when it is due to retransmit.
 * @param[in] connection Connection handle.
 * @param[in] ranging_counter Ranging counter.
 * @return status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_send_overwritten(uint8_t connection,
                                    cs_ras_ranging_counter_t ranging_counter);

/**************************************************************************//**
 * Get handle for a given characteristic index
 * @param[in] index Characteristic index.
 * @return characteristic handle.
 *****************************************************************************/
uint16_t cs_ras_server_get_cccd_handle(cs_ras_characteristic_index_t index);

/**************************************************************************//**
 * Get CCCD handle for a given characteristic index
 * @param[in] index Characteristic index.
 * @return characteristic handle.
 *****************************************************************************/
uint16_t cs_ras_server_get_handle(cs_ras_characteristic_index_t index);

#ifdef SL_CATALOG_POWER_MANAGER_PRESENT

/***************************************************************************//**
 * Checks if it is ok to sleep now / power manager
 * @return true - if ready to sleep, false otherwise
 ******************************************************************************/
bool cs_ras_server_is_ok_to_sleep(void);

/***************************************************************************//**
 * Routine for power manager handler
 * @return sl_power_manager_on_isr_exit_t
 * @retval SL_POWER_MANAGER_IGNORE = (1UL << 0UL), < The module did not trigger an ISR and it doesn't want to contribute to the decision
 * @retval SL_POWER_MANAGER_SLEEP  = (1UL << 1UL), < The module was the one that caused the system wakeup and the system SHOULD go back to sleep
 * @retval SL_POWER_MANAGER_WAKEUP = (1UL << 2UL), < The module was the one that caused the system wakeup and the system MUST NOT go back to sleep
 ******************************************************************************/
sl_power_manager_on_isr_exit_t cs_ras_server_sleep_on_isr_exit(void);

#endif // SL_CATALOG_POWER_MANAGER_PRESENT

#ifdef __cplusplus
};
#endif

#endif // CS_RAS_SERVER_INTERNAL_H
