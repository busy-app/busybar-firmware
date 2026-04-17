/*******************************************************************************
 * @file  debug_logs_event_hdlr_auto_gen.h.j2
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef DEBUG_LOGS_EVENT_AUTO_GEN_H
#define DEBUG_LOGS_EVENT_AUTO_GEN_H
#include "event_loop.h"
#include "rsi_ble_apis.h"

#ifdef FRDM_K28F
#define APP_WEAK __attribute__((weak))
#else
#ifdef MXRT_595s
#define APP_WEAK __weak
#else
#define APP_WEAK __attribute__((weak))
#endif
#endif

#define FIXED_LEN_RESPONSE_BUFFER_VALID    1
#define VARIABLE_LEN_RESPONSE_BUFFER_VALID 2

//! Enumerations
enum debug_logs_event_ids { controller_logs_event_id, max_debug_logs_event_id };

typedef struct rsi_bt_event_controller_logs_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_bt_event_controller_logs_t rsi_bt_event_controller_logs;
} rsi_bt_event_controller_logs_msg_t;

//! Function declarations
void rsi_bt_on_controller_logs_driver_callback(uint16_t status,
                                               rsi_bt_event_controller_logs_t *rsi_bt_event_controller_logs);
void rsi_bt_on_controller_logs_wrapper(void *msg);
void rsi_bt_on_controller_logs(uint16_t __attribute__((unused)) status, void __attribute__((unused)) * event_data);

//! Extern functions
extern void rsi_bt_on_controller_logs(uint16_t __attribute__((unused)) status,
                                      void __attribute__((unused)) * event_data);

extern generic_event_handler_lut_entry_t debug_logs_events_hdlr_lut[];
#endif