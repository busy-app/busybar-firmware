/***************************************************************************//**
 * @file high_datarate_phy_stack_interface_ipc_command_messages.h
 * @brief defines structured format for 'high_datarate_phy_stack_interface' ipc messages
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
// automatically generated from high_datarate_phy_stack_interface.h.  Do not manually edit
#ifndef HIGH_DATARATE_PHY_STACK_INTERFACE_IPC_COMMAND_MESSAGES_H
#define HIGH_DATARATE_PHY_STACK_INTERFACE_IPC_COMMAND_MESSAGES_H

#include "stack/include/high_datarate_phy_stack_interface.h"
#include "stack/internal/inc/high_datarate_phy_stack_interface_internal_def.h"

typedef struct {
  uint8_t hdr_csma_attempts;
  uint32_t min_bo_period_us;
  uint32_t max_bo_period_us;
} sli_mac_stack_force_tx_after_failed_hdr_phy_cca_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_mac_stack_force_tx_after_failed_hdr_phy_cca_ipc_rsp_t;

typedef struct {
  sli_mac_stack_force_tx_after_failed_hdr_phy_cca_ipc_req_t request;
  sli_mac_stack_force_tx_after_failed_hdr_phy_cca_ipc_rsp_t response;
} sli_mac_stack_force_tx_after_failed_hdr_phy_cca_ipc_msg_t;

typedef struct {
  sl_rail_csma_config_t csma_params;
} sli_mac_stack_lower_mac_set_high_datarate_csma_params_ipc_req_t;

typedef struct {
  sli_mac_stack_lower_mac_set_high_datarate_csma_params_ipc_req_t request;
} sli_mac_stack_lower_mac_set_high_datarate_csma_params_ipc_msg_t;

typedef struct {
  sl_802154_radio_priorities_t priorities;
} sli_mac_stack_lower_mac_set_high_datarate_phy_radio_priorities_ipc_req_t;

typedef struct {
  sli_mac_stack_lower_mac_set_high_datarate_phy_radio_priorities_ipc_req_t request;
} sli_mac_stack_lower_mac_set_high_datarate_phy_radio_priorities_ipc_msg_t;

typedef struct {
  uint8_t nwk_index;
  uint8_t payload[MAX_HIGH_DATARATE_PHY_PACKET_LENGTH];
} sli_mac_stack_send_raw_high_datarate_phy_message_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_mac_stack_send_raw_high_datarate_phy_message_ipc_rsp_t;

typedef struct {
  sli_mac_stack_send_raw_high_datarate_phy_message_ipc_req_t request;
  sli_mac_stack_send_raw_high_datarate_phy_message_ipc_rsp_t response;
} sli_mac_stack_send_raw_high_datarate_phy_message_ipc_msg_t;

typedef struct {
  uint8_t nwk_index;
  uint8_t payload[MAX_HIGH_DATARATE_PHY_PACKET_LENGTH];
  sl_rail_time_t timestamp;
} sli_mac_stack_send_raw_high_datarate_phy_scheduled_message_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_mac_stack_send_raw_high_datarate_phy_scheduled_message_ipc_rsp_t;

typedef struct {
  sli_mac_stack_send_raw_high_datarate_phy_scheduled_message_ipc_req_t request;
  sli_mac_stack_send_raw_high_datarate_phy_scheduled_message_ipc_rsp_t response;
} sli_mac_stack_send_raw_high_datarate_phy_scheduled_message_ipc_msg_t;

typedef struct {
  bool enable_f;
} sli_mac_stack_set_mode_switch_sync_detect_ipc_req_t;

typedef struct {
  sl_rail_status_t result;
} sli_mac_stack_set_mode_switch_sync_detect_ipc_rsp_t;

typedef struct {
  sli_mac_stack_set_mode_switch_sync_detect_ipc_req_t request;
  sli_mac_stack_set_mode_switch_sync_detect_ipc_rsp_t response;
} sli_mac_stack_set_mode_switch_sync_detect_ipc_msg_t;

#endif // HIGH_DATARATE_PHY_STACK_INTERFACE_IPC_COMMAND_MESSAGES_H
