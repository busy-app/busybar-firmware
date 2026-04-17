/***************************************************************************//**
 * @file sl_zigbee_mac_app_events_ipc_callback_events.h
 * @brief callback struct and event handlers for sl_zigbee_mac_app_events
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from sl_zigbee_mac_app_events.h.  Do not manually edit
#ifndef SL_ZIGBEE_MAC_APP_EVENTS_IPC_CALLBACK_EVENTS_H
#define SL_ZIGBEE_MAC_APP_EVENTS_IPC_CALLBACK_EVENTS_H

#include "stack/internal/inc/sl_zigbee_mac_app_events_internal_def.h"

typedef struct {
  uint8_t mac_index;
  sl_status_t status;
  uint8_t packet_length;
  uint8_t packet_data[MAX_IPC_VEC_ARG_CAPACITY];
} sli_802154_stackmac_communication_status_indication_handler_ipc_event_t;

#endif // SL_ZIGBEE_MAC_APP_EVENTS_IPC_CALLBACK_EVENTS_H
