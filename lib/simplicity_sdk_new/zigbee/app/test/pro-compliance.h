/***************************************************************************//**
 * @file
 * @brief Application implementing Zigbee Pro
 * for compliance testing.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_ZIGBEE_PRO_COMPLIANCE_H
#define SL_ZIGBEE_PRO_COMPLIANCE_H

#include PLATFORM_HEADER

// Our configuration parameters.
#include "pro-compliance-configuration.h"

// Ember stack and related utilities.
#include "stack/include/sl_zigbee.h"               // Main stack definitions.
#include "stack/include/zigbee-device-stack.h" // ZigBee Device Object.

// HAL.
#include "hal/hal.h"

// Application utilities.
#include "serial/serial.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "app/util/common/common.h"
#include "app/util/zigbee-framework/zigbee-device-library.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

void pro_compliance_app_rtos_task_init(void);
#endif // SL_ZIGBEE_PRO_COMPLIANCE_H

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
bool isForceAssociationUsed();
bool isZCPContext();
void sl_zigbee_incoming_message_handler_pro_compliance(sl_zigbee_incoming_message_type_t type,
                                                       sl_zigbee_aps_frame_t *apsFrame,
                                                       sl_zigbee_rx_packet_info_t *packetInfo,
                                                       uint8_t messageLength,
                                                       uint8_t *message);
void sl_zigbee_network_found_handler_pro_compliance(sl_zigbee_zigbee_network_t *networkFound,
                                                    uint8_t lqi,
                                                    int8_t rssi);
void sl_zigbee_message_sent_handler_pro_compliance(sl_status_t status,
                                                   sl_zigbee_outgoing_message_type_t type,
                                                   uint16_t indexOrDestination,
                                                   sl_zigbee_aps_frame_t *apsFrame,
                                                   uint16_t messageTag,
                                                   uint8_t messageLength,
                                                   uint8_t *message);
sl_zigbee_join_decision_t sl_zigbee_internal_trust_center_join_handler_pro_compliance(sl_802154_short_addr_t newNodeId,
                                                                                      sl_802154_long_addr_t newNodeEui64,
                                                                                      sl_zigbee_device_update_t status,
                                                                                      sl_802154_short_addr_t parentOfNewNode);

sl_zigbee_packet_action_t sl_zigbee_internal_packet_handoff_incoming_handler_pro_compliance(
  sl_zigbee_zigbee_packet_type_t packetType,
  sli_buffer_manager_buffer_t packetBuffer,
  uint8_t index,
  // Return:
  void *data,
  uint8_t data_len);

void sl_zigbee_override_incoming_route_record_handler_pro_compliance(sl_zigbee_rx_packet_info_t *packetInfo,
                                                                     uint8_t relayCount,
                                                                     uint8_t *relayList,
                                                                     bool* consumed);

void sl_zigbee_stack_status_handler_pro_compliance(sl_status_t status);

void sl_zigbee_child_join_handler_pro_compliance(
  // The index of the child of interest.
  uint8_t index,
  // True if the child is joining. False the child is leaving.
  bool joining,
  // The node ID of the child.
  sl_802154_short_addr_t childId,
  // The EUI64 of the child.
  sl_802154_long_addr_t childEui64,
  // The node type of the child.
  sl_zigbee_node_type_t childType);

void sl_zigbee_duty_cycle_handler_pro_compliance(uint8_t channelPage,
                                                 uint8_t channel,
                                                 sl_zigbee_duty_cycle_state_t state,
                                                 uint8_t totalDevices,
                                                 sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles);
sl_zigbee_zdo_status_t sl_zigbee_internal_remote_delete_binding_handler_pro_compliance(uint8_t index);
void sl_zigbee_incoming_many_to_one_route_request_handler_pro_compliance(sl_802154_short_addr_t source,
                                                                         sl_802154_long_addr_t longId,
                                                                         uint8_t cost);
void sl_zigbee_incoming_route_error_handler_pro_compliance(sl_status_t status,
                                                           sl_802154_short_addr_t target);
void sl_zigbee_raw_transmit_complete_handler_pro_compliance(uint8_t messageLength, uint8_t* messageContents, sl_status_t status, uint8_t messageTag);
void sl_zigbee_switch_network_key_handler_pro_compliance(uint8_t sequenceNumber);
void sl_zigbee_zigbee_key_establishment_handler_pro_compliance(sl_802154_long_addr_t partner, sl_zigbee_key_status_t status);
sl_status_t sl_zigbee_internal_pan_id_conflict_handler_pro_compliance(int8_t conflictCount);
void sl_zigbee_scan_complete_handler_pro_compliance(uint8_t channel, sl_status_t status);
sl_zigbee_zdo_status_t sl_zigbee_internal_remote_set_binding_handler_pro_compliance(sl_zigbee_binding_table_entry_t *entry);
void sli_zigbee_af_fragmentation_message_sent_handler_pro_compliance(sl_status_t status,
                                                                     sl_zigbee_outgoing_message_type_t type,
                                                                     uint16_t indexOrDestination,
                                                                     sl_zigbee_aps_frame_t *apsFrame,
                                                                     uint16_t messageTag,
                                                                     uint8_t *buffer,
                                                                     uint16_t bufLen);
void versionCommandZCP(SL_CLI_COMMAND_ARG);
#endif
