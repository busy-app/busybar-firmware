/***************************************************************************//**
 * @file
 * @brief Source routing stub code
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

#include PLATFORM_HEADER
#include "stack/include/sl_zigbee_types.h"
#include "stack/internal/inc/internal-defs-patch.h"
#include "stack/include/message.h"
#include "stack/include/sl_zigbee_types_internal.h"
#include "source-route-table-update.h"
#include "stack/core/sl_zigbee_multi_network.h"

// -----------------------------------------------------------------------------
// Functions

//Mtorr related functions

void sli_zigbee_source_route_update_event_handler(sli_zigbee_event_t *event)
{
  (void) event;
}

//source route table related functions
uint8_t sli_zigbee_source_route_find_index(sl_802154_short_addr_t id)
{
  (void)id;
  return SOURCE_ROUTE_NULL_INDEX;
}

uint8_t sli_zigbee_source_route_add_entry_with_closer_next_hop(sl_802154_short_addr_t newId,
                                                               sl_802154_short_addr_t closerNodeId)
{
  (void)newId;
  (void)closerNodeId;
  return 0xFF;
}

uint8_t sli_zigbee_stack_get_source_route_table_filled_size(void)
{
  return 0;
}

uint8_t sli_zigbee_stack_get_source_route_table_total_size(void)
{
  return 0;
}

void sli_zigbee_change_source_route_entry(sl_802154_short_addr_t newChildId,
                                          sl_802154_short_addr_t newParentId,
                                          bool ourChild,
                                          bool deviceLeft)
{
  (void)newChildId;
  (void)newParentId;
  (void)ourChild;
  (void)deviceLeft;
}

void sli_zigbee_incoming_route_record(sl_802154_short_addr_t source,
                                      sl_802154_long_addr_t sourceEui,
                                      uint8_t relayCount,
                                      sli_buffer_manager_buffer_t header,
                                      uint8_t relayListIndex)
{
  bool apiConsumed = true;
  sli_zigbee_enable_application_current_network(); // SET app current network
  sl_zigbee_rx_packet_info_t packetInfo;
  packetInfo.sender_short_id = source;
  if (sourceEui != NULL) {
    memmove(packetInfo.sender_long_id, sourceEui, EUI64_SIZE);
  }
  sli_zigbee_stack_override_incoming_route_record_handler(
    &packetInfo,
    relayCount,
    sli_legacy_buffer_manager_get_buffer_pointer(header) + relayListIndex,
    &apiConsumed);
  sli_zigbee_restore_current_network();   // RESTORE
}

uint8_t sli_zigbee_append_source_route(sl_802154_short_addr_t destination,
                                       sli_buffer_manager_buffer_t* header)
{
  bool appConsumed = true;
  return sl_zigbee_internal_override_append_source_route_handler(destination, header, &appConsumed);
}

sl_status_t sli_zigbee_stack_get_source_route_table_entry(uint8_t index,
                                                          sl_802154_short_addr_t *destination,
                                                          uint8_t *closerIndex)
{
  (void)index;
  (void)destination;
  (void)closerIndex;
  return SL_STATUS_NOT_FOUND;
}

uint8_t sli_zigbee_stack_get_source_route_overhead(sl_802154_short_addr_t destination)
{
  (void)destination;
  return 0;
}

void sli_zigbee_source_route_table_init(void)
{
}

void sli_zigbee_source_route_update_init(void)
{
}

sl_status_t sli_zigbee_stack_set_concentrator(bool on,
                                              uint16_t concentratorTypeParam,
                                              uint16_t minTime,
                                              uint16_t maxTime,
                                              uint8_t routeErrorThresholdParam,
                                              uint8_t deliveryFailureThresholdParam,
                                              uint8_t maxHopsParam)
{
  (void)on;
  (void)concentratorTypeParam;
  (void)minTime;
  (void)maxTime;
  (void)routeErrorThresholdParam;
  (void)deliveryFailureThresholdParam;
  (void)maxHopsParam;
  return SL_STATUS_INVALID_STATE;
}

uint32_t sli_zigbee_stack_set_source_route_discovery_mode(sl_zigbee_source_route_discovery_mode_t mode)
{
  (void)mode;
  return MAX_INT32U_VALUE;
}

// mtorr related
void sli_zigbee_stack_concentrator_note_route_error(sl_status_t status,
                                                    sl_802154_short_addr_t nodeId)
{
  (void)status;
  (void)nodeId;
}

void sli_zigbee_stack_concentrator_note_delivery_failure(sl_zigbee_outgoing_message_type_t type, sl_status_t status)
{
  (void)type;
  (void)status;
}

// these public API wrappers are needed because of exceptional case that these APIs are called from af-soc
sl_status_t sl_zigbee_get_source_route_table_entry(uint8_t index,
                                                   sl_802154_short_addr_t * destination,
                                                   uint8_t * closerIndex)

{
  (void)index;
  (void)destination;
  (void)closerIndex;
  return SL_STATUS_NOT_FOUND;
}

uint8_t sl_zigbee_get_source_route_table_filled_size(void)
{
  return 0;
}

uint8_t sl_zigbee_get_source_route_table_total_size(void)
{
  return 0;
}

void sl_zigbee_concentrator_note_delivery_failure(sl_zigbee_outgoing_message_type_t type,
                                                  sl_status_t status)
{
  (void)type;
  (void)status;
}
