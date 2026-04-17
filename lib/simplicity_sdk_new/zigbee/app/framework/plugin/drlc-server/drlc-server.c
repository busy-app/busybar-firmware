/***************************************************************************//**
 * @file
 * @brief For more information on the usage of this plugin see drlc-server.h
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

#include "../../include/af.h"
#include "../../util/common.h"
#include "drlc-server.h"

#include "drlc-server-config.h"
#include "zap-cluster-command-parser.h"

static sl_zigbee_af_load_control_event_t scheduledLoadControlEventTable[SL_ZIGBEE_ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE];
// The following matrix is a map of event order that is sorted by startTime and issuerEventId
// e.g. [ep][0] -> 2 (index of scheduledLoadControlEventTable)
//      [ep][1] -> 0
static uint8_t eventOrderToLoadControlIndex[SL_ZIGBEE_ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE];
#define ORDER_NOT_SET     0xFF

void sl_zigbee_af_demand_response_load_control_cluster_server_init_cb(uint8_t endpoint)
{
  sli_zigbee_af_clear_scheduled_load_control_events(endpoint); //clear all events at init
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_demand_response_load_control_cluster_get_scheduled_events_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_demand_response_and_load_control_cluster_get_scheduled_events_command_t cmd_data;
  uint8_t i, sent = 0;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(sl_zigbee_af_current_endpoint(),
                                                               ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  uint8_t orderedIndex;
  sl_zigbee_af_load_control_event_t event;

  if (ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX
      || zcl_decode_demand_response_and_load_control_cluster_get_scheduled_events_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  // CCB 1297
  // startTime = 0 does NOT mean get all events from "now"

  // Go through our sorted table (by following the eventOrderToLoadControlIndex
  // map) and send out the scheduled events
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE; i++) {
    orderedIndex = eventOrderToLoadControlIndex[ep][i];
    // If no more events have been set (and thus, ordered), break
    if (ORDER_NOT_SET == orderedIndex) {
      break;
    }

    // CCB 1297: events with startTime = "now" are sent with startTime = 0, not
    // the current time that we internally set it to at the time of creation
    // We do, however, update the duration based on the elapsed time
    // sli_zigbee_af_get_scheduled_load_control_event() normalizes the duration time
    if (SL_STATUS_OK != sli_zigbee_af_get_scheduled_load_control_event(
          sl_zigbee_af_current_endpoint(),
          orderedIndex,
          &event)) {
      continue;
    }

    // check how many we have sent, if they have a positive number of events
    // they want returned and we have hit it we should exit.
    if (cmd_data.numberOfEvents != 0 && sent >= cmd_data.numberOfEvents) {
      break;
    }

    // If the event is inactive or its start time is before the requested start
    // time we ignore it.
    if (event.source[0] == 0xFF || event.startTime < cmd_data.startTime) {
      continue;
    }

    // CCB 1297: filter on issuerEventId if it was sent in command
    if ((cmd_data.issuerEventId != 0xFFFFFFFF)
        && (event.startTime == cmd_data.startTime)
        && (event.eventId < cmd_data.issuerEventId)) {
      continue;
    }

    // send the event
    sl_zigbee_af_fill_command_demand_response_load_control_cluster_load_control_event(event.eventId,
                                                                                      event.deviceClass,
                                                                                      event.utilityEnrollmentGroup,
                                                                                      event.startTime,
                                                                                      event.duration,
                                                                                      event.criticalityLevel,
                                                                                      event.coolingTempOffset,
                                                                                      event.heatingTempOffset,
                                                                                      event.coolingTempSetPoint,
                                                                                      event.heatingTempSetPoint,
                                                                                      event.avgLoadPercentage,
                                                                                      event.dutyCycle,
                                                                                      event.eventControl);
    sl_zigbee_af_set_command_endpoints(cmd->apsFrame->destinationEndpoint,
                                       cmd->apsFrame->sourceEndpoint);
    sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
    sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, cmd->source);
    sent++;  //record that we sent it. and continue
  }

  // Bug 13547:
  //   Only send a Default response if there were no events returned.
  //   The LCE messages are the "next" messages in the sequence so no
  //   default response is needed for that successful case.
  if (sent == 0) {
    return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_demand_response_load_control_cluster_report_event_status_cb(sl_zigbee_af_cluster_command_t *cmd)
{
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
  sl_zcl_demand_response_and_load_control_cluster_report_event_status_command_t cmd_data;

  if (zcl_decode_demand_response_and_load_control_cluster_report_event_status_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_demand_response_load_control_cluster_println("= RX Event Status =");
  sl_zigbee_af_demand_response_load_control_cluster_println("  eid: %08X", cmd_data.issuerEventId);
  sl_zigbee_af_demand_response_load_control_cluster_println("   es: %02X", cmd_data.eventStatus);
  sl_zigbee_af_demand_response_load_control_cluster_println("  est: T%08X", cmd_data.eventStatusTime);
  sl_zigbee_af_demand_response_load_control_cluster_flush();
  sl_zigbee_af_demand_response_load_control_cluster_println("  cla: %02X", cmd_data.criticalityLevelApplied);
  sl_zigbee_af_demand_response_load_control_cluster_flush();
  sl_zigbee_af_demand_response_load_control_cluster_println("ctspa: %04X (%d)",
                                                            cmd_data.coolingTemperatureSetPointApplied,
                                                            cmd_data.coolingTemperatureSetPointApplied);
  sl_zigbee_af_demand_response_load_control_cluster_println("htspa: %04X (%d)",
                                                            cmd_data.heatingTemperatureSetPointApplied,
                                                            cmd_data.heatingTemperatureSetPointApplied);
  sl_zigbee_af_demand_response_load_control_cluster_flush();
  sl_zigbee_af_demand_response_load_control_cluster_println("  avg: %02X",
                                                            cmd_data.averageLoadAdjustmentPercentageApplied);
  sl_zigbee_af_demand_response_load_control_cluster_println("   dc: %02X", cmd_data.dutyCycleApplied);
  sl_zigbee_af_demand_response_load_control_cluster_println("   ec: %02X", cmd_data.eventControl);
  sl_zigbee_af_demand_response_load_control_cluster_flush();
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

void sli_zigbee_af_clear_scheduled_load_control_events(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);

  if (ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX) {
    return;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE; i++) {
    sl_zigbee_af_load_control_event_t *event = &scheduledLoadControlEventTable[ep][i];
    event->source[0] = 0xFF; // event inactive if first byte of source is 0xFF
  }

  clearEventOrderToLoadControlIndex(ep);
}

sl_status_t sli_zigbee_af_get_scheduled_load_control_event(uint8_t endpoint, uint8_t index, sl_zigbee_af_load_control_event_t *event)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);

  if (ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (index < SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE) {
    memmove(event,
            &scheduledLoadControlEventTable[ep][index],
            sizeof(sl_zigbee_af_load_control_event_t));
    if (event->source[1] == 0x01 // start now?
        && (sl_zigbee_af_get_current_time()
            < event->startTime + (uint32_t)event->duration * 60)) {
      event->duration = (event->duration
                         - (sl_zigbee_af_get_current_time() - event->startTime) / 60);
      event->startTime = 0x0000;
    }
    return SL_STATUS_OK;
  }
  sl_zigbee_af_demand_response_load_control_cluster_println("slce index out of range");
  return SL_STATUS_INVALID_INDEX;
}

sl_status_t sli_zigbee_af_set_scheduled_load_control_event(uint8_t endpoint, uint8_t index, const sl_zigbee_af_load_control_event_t *event)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);

  if (ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (index < SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE) {
    bool newEvent = (scheduledLoadControlEventTable[ep][index].source[0] == 0xFF);
    memmove(&scheduledLoadControlEventTable[ep][index],
            event,
            sizeof(sl_zigbee_af_load_control_event_t));
    if (scheduledLoadControlEventTable[ep][index].startTime == 0x0000) {
      scheduledLoadControlEventTable[ep][index].startTime = sl_zigbee_af_get_current_time();
      scheduledLoadControlEventTable[ep][index].source[1] = 0x01;
    } else {
      scheduledLoadControlEventTable[ep][index].source[1] = 0x00;
    }
    updateEventOrderToLoadControlIndex(ep, index, true, newEvent);
    return SL_STATUS_OK;
  }
  sl_zigbee_af_demand_response_load_control_cluster_println("slce index out of range");
  return SL_STATUS_INVALID_INDEX;
}

void sli_zigbee_af_drlc_server_print_info(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);

  if (ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX) {
    return;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE; i++) {
    sl_zigbee_af_load_control_event_t *lce = &scheduledLoadControlEventTable[ep][i];
    sl_zigbee_af_demand_response_load_control_cluster_flush();
    sl_zigbee_af_demand_response_load_control_cluster_println("= LCE %d =", i);
    sl_zigbee_af_demand_response_load_control_cluster_println("eid: 0x%08X", lce->eventId);
    sl_zigbee_af_demand_response_load_control_cluster_println("src: 0x%02X%02X", lce->source[1], lce->source[0]);
    sl_zigbee_af_demand_response_load_control_cluster_println("sep: 0x%02X", lce->sourceEndpoint);
    sl_zigbee_af_demand_response_load_control_cluster_flush();
    sl_zigbee_af_demand_response_load_control_cluster_println("dep: 0x%02X", lce->destinationEndpoint);
    sl_zigbee_af_demand_response_load_control_cluster_println("dev: 0x%04X", lce->deviceClass);
    sl_zigbee_af_demand_response_load_control_cluster_println("ueg: 0x%02X", lce->utilityEnrollmentGroup);
    sl_zigbee_af_demand_response_load_control_cluster_println(" st: 0x%08X", lce->startTime);
    sl_zigbee_af_demand_response_load_control_cluster_flush();
    sl_zigbee_af_demand_response_load_control_cluster_println("dur: 0x%04X", lce->duration);
    sl_zigbee_af_demand_response_load_control_cluster_println(" cl: 0x%02X", lce->criticalityLevel);
    sl_zigbee_af_demand_response_load_control_cluster_println("cto: 0x%02X", lce->coolingTempOffset);
    sl_zigbee_af_demand_response_load_control_cluster_println("hto: 0x%02X", lce->heatingTempOffset);
    sl_zigbee_af_demand_response_load_control_cluster_flush();
    sl_zigbee_af_demand_response_load_control_cluster_println("cts: 0x%04X", lce->coolingTempSetPoint);
    sl_zigbee_af_demand_response_load_control_cluster_println("hts: 0x%04X", lce->heatingTempSetPoint);
    sl_zigbee_af_demand_response_load_control_cluster_println("alp: 0x%02X", lce->avgLoadPercentage);
    sl_zigbee_af_demand_response_load_control_cluster_println(" dc: 0x%02X", lce->dutyCycle);
    sl_zigbee_af_demand_response_load_control_cluster_flush();
    sl_zigbee_af_demand_response_load_control_cluster_println(" ev: 0x%02X", lce->eventControl);
    sl_zigbee_af_demand_response_load_control_cluster_println(" sr: 0x%04X", lce->startRand);
    sl_zigbee_af_demand_response_load_control_cluster_println(" dr: 0x%04X", lce->durationRand);
    sl_zigbee_af_demand_response_load_control_cluster_println(" oc: 0x%02X", lce->optionControl);
    sl_zigbee_af_demand_response_load_control_cluster_flush();
  }
  sl_zigbee_af_demand_response_load_control_cluster_println("Table size: %d",
                                                            SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE);
}

void sli_zigbee_af_drlc_server_slce_message(sl_802154_short_addr_t nodeId,
                                            uint8_t srcEndpoint,
                                            uint8_t dstEndpoint,
                                            uint8_t index)
{
  sl_zigbee_af_load_control_event_t event;
  sl_status_t status;
  status = sli_zigbee_af_get_scheduled_load_control_event(srcEndpoint, index, &event);

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_demand_response_load_control_cluster_println("send slce fail: 0x%02X", status);
    return;
  }

  sl_zigbee_af_fill_command_demand_response_load_control_cluster_load_control_event(event.eventId,
                                                                                    event.deviceClass,
                                                                                    event.utilityEnrollmentGroup,
                                                                                    event.startTime,
                                                                                    event.duration,
                                                                                    event.criticalityLevel,
                                                                                    event.coolingTempOffset,
                                                                                    event.heatingTempOffset,
                                                                                    event.coolingTempSetPoint,
                                                                                    event.heatingTempSetPoint,
                                                                                    event.avgLoadPercentage,
                                                                                    event.dutyCycle,
                                                                                    event.eventControl);

  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
}

// This internal function checks where the load event control should be placed
// in a startTime- and issuerEventId-sorted array
// This function does not check if the load control event is already in the
// ordered array, so calling it with an existing entry will return a new index
// position, for which the caller must handle appropriately
// E.g. a LCE has start time 5 and issuer event ID 2 and lies at index 4 in the
// eventOrderToLoadControlIndex array. The user changes the start time to 9 and
// issuer event ID to 1, calls this function, and receives index 10. It is up to
// the caller to move the values at indices 5-10 to indices 4-9 and then set
// eventOrderToLoadControlIndex[10] to the LCE's index in the
// scheduledLoadControlEventTable array
uint8_t findPlacementInEventOrderToLoadControlIndex(uint8_t ep, uint8_t index)
{
  uint8_t i;
  uint8_t currentIndex;

  for (i = 0; (i < SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE) && (ORDER_NOT_SET != eventOrderToLoadControlIndex[ep][i]); i++) {
    currentIndex = eventOrderToLoadControlIndex[ep][i];
    if ((scheduledLoadControlEventTable[ep][index].startTime < scheduledLoadControlEventTable[ep][currentIndex].startTime)
        || ((scheduledLoadControlEventTable[ep][index].startTime == scheduledLoadControlEventTable[ep][currentIndex].startTime)
            && (scheduledLoadControlEventTable[ep][index].eventId < scheduledLoadControlEventTable[ep][currentIndex].eventId))) {
      break;
    }
  }

  return i;
}

void updateEventOrderToLoadControlIndex(uint8_t ep,
                                        uint8_t index,
                                        bool eventAdded,
                                        bool isNewEvent)
{
  uint8_t i;
  uint8_t currentOrderIndex;
  uint8_t newOrderIndex;

  if (eventAdded) {
    // A brand new load control event was added to the table
    if (isNewEvent) {
      // First find the ordered placement of this new event
      newOrderIndex = findPlacementInEventOrderToLoadControlIndex(ep, index);
      if (SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE == newOrderIndex) {
        return;
      }

      // Now push everyone forward one spot from the found index
      for (i = SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE - 1; i > newOrderIndex; i--) {
        eventOrderToLoadControlIndex[ep][i] = eventOrderToLoadControlIndex[ep][i - 1];
      }

      // Finally, set the mapping for the newly added event
      eventOrderToLoadControlIndex[ep][newOrderIndex] = index;
    }
    // An existing load control event was modified
    else {
      // First record the current ordered index and calculate the new index
      for (currentOrderIndex = 0; currentOrderIndex < SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE; currentOrderIndex++) {
        if (eventOrderToLoadControlIndex[ep][currentOrderIndex] == index) {
          break;
        }
      }

      if (SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE == currentOrderIndex) {
        return;
      }

      newOrderIndex = findPlacementInEventOrderToLoadControlIndex(ep, index);
      // SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE is a perfectly
      // valid return. It means the new event must go last in the ordered array

      // If the event must go last and it already is last, done
      if ((SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE == newOrderIndex)
          && (SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE == currentOrderIndex + 1)) {
        return;
      }
      // else, if the new index is not the final index and it's calculated to be
      // be the next index, then this effectively is the same position
      // findPlacementInEventOrderToLoadControlIndex will never return the same
      // index, since the comparative operators won't reach that result
      else if (newOrderIndex == currentOrderIndex + 1) {
        return;
      }

      if (SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE == newOrderIndex) {
        newOrderIndex--;
      }

      // else, move the indices' values and set the new index
      if (currentOrderIndex < newOrderIndex) {
        // e.g. from 5 to 8, move 6-8 down one
        for (i = currentOrderIndex + 1; i <= newOrderIndex; i++) {
          eventOrderToLoadControlIndex[ep][i - 1] = eventOrderToLoadControlIndex[ep][i];
        }
      } else {
        // e.g. from 8 to 5, move 5-7 up one
        for (i = currentOrderIndex; i > newOrderIndex; i--) {
          eventOrderToLoadControlIndex[ep][i] = eventOrderToLoadControlIndex[ep][i - 1];
        }
      }

      eventOrderToLoadControlIndex[ep][newOrderIndex] = index;
    }
  } else {
    // An existing load control event cannot currently be removed
  }
}

void clearEventOrderToLoadControlIndex(uint8_t ep)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE; i++) {
    eventOrderToLoadControlIndex[ep][i] = 0xFF;
  }
}

uint32_t sl_zigbee_af_demand_response_load_control_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                                                sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_REPORT_EVENT_STATUS_COMMAND_ID:
      {
        status = sl_zigbee_af_demand_response_load_control_cluster_report_event_status_cb(cmd);
        break;
      }
      case ZCL_GET_SCHEDULED_EVENTS_COMMAND_ID:
      {
        status = sl_zigbee_af_demand_response_load_control_cluster_get_scheduled_events_cb(cmd);
        break;
      }
      default:
        break;
    }
  }

  return status;
}
