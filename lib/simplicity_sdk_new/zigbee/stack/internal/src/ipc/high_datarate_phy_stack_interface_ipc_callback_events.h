/***************************************************************************//**
 * @file high_datarate_phy_stack_interface_ipc_callback_events.h
 * @brief callback struct and event handlers for high_datarate_phy_stack_interface
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
// automatically generated from high_datarate_phy_stack_interface.h.  Do not manually edit
#ifndef HIGH_DATARATE_PHY_STACK_INTERFACE_IPC_CALLBACK_EVENTS_H
#define HIGH_DATARATE_PHY_STACK_INTERFACE_IPC_CALLBACK_EVENTS_H

#include "stack/internal/inc/high_datarate_phy_stack_interface_internal_def.h"

typedef struct {
  uint8_t packet[MAX_HIGH_DATARATE_PHY_PACKET_LENGTH];
  uint8_t linkQuality;
  int8_t rssi;
  uint32_t pkt_rx_timestamp;
} sli_mac_stack_high_datarate_phy_rx_callback_ipc_event_t;

typedef struct {
  uint8_t mac_index;
  sl_status_t status;
  uint16_t packet_length;
  uint8_t packet_contents[MAX_HIGH_DATARATE_PHY_PACKET_LENGTH];
  uint8_t tag;
} sli_mac_stack_high_datarate_phy_tx_callback_ipc_event_t;

#endif // HIGH_DATARATE_PHY_STACK_INTERFACE_IPC_CALLBACK_EVENTS_H
