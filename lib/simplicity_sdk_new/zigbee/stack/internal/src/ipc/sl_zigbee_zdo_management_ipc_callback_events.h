/***************************************************************************//**
 * @file sl_zigbee_zdo_management_ipc_callback_events.h
 * @brief callback struct and event handlers for sl_zigbee_zdo_management
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from sl_zigbee_zdo_management.h.  Do not manually edit
#ifndef SL_ZIGBEE_ZDO_MANAGEMENT_IPC_CALLBACK_EVENTS_H
#define SL_ZIGBEE_ZDO_MANAGEMENT_IPC_CALLBACK_EVENTS_H

#include "stack/internal/inc/sl_zigbee_zdo_management_internal_def.h"

typedef struct {
  sl_zigbee_zdo_status_t status;
  sl_zigbee_beacon_survey_results_t survey_results;
  uint8_t potential_parent_count;
  sl_zigbee_potential_parent_t potential_parents[16];
  uint16_t pan_id_conflicts;
} sli_zigbee_stack_beacon_survey_complete_callback_ipc_event_t;

#endif // SL_ZIGBEE_ZDO_MANAGEMENT_IPC_CALLBACK_EVENTS_H
