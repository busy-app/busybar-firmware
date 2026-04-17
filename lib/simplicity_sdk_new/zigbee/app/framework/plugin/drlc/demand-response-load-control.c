/***************************************************************************//**
 * @file
 * @brief Routines for the DRLC plugin.
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
#include "demand-response-load-control.h"

#include "drlc-config.h"
#include "zap-cluster-command-parser.h"

#include "app/framework/plugin/esi-management/esi-management.h"

//------------------------------------------------------------------------------

/**
 * Checks to see if the passed UEG OR deviceClass match those stored in
 * the cluster attributes.
 *
 * Returns true if either the device class or the UEG match. This
 * is the assumed interpretation of the spec..
 **/
static bool afMatchUegOrDeviceClass(uint8_t endpoint,
                                    uint8_t utilityEnrollmentGroup,
                                    uint16_t deviceClass)
{
  uint8_t       ueg      = 0;
  uint16_t      dc;
  uint8_t       dcArray[2];
  uint8_t       dataType = 0;
  uint8_t       status;

  status =
    sl_zigbee_af_read_attribute(endpoint,
                                ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID,
                                ZCL_UTILITY_ENROLLMENT_GROUP_ATTRIBUTE_ID,
                                CLUSTER_MASK_CLIENT,
                                (uint8_t*)&ueg,
                                1,
                                &dataType);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return false;
  }

  dataType = 0;
  status =
    sl_zigbee_af_read_attribute(endpoint,
                                ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID,
                                ZCL_DEVICE_CLASS_VALUE_ATTRIBUTE_ID,
                                CLUSTER_MASK_CLIENT,
                                (uint8_t*)dcArray,
                                2,
                                &dataType);
  dc = (uint16_t)dcArray[0] | ((uint16_t)dcArray[1] << 8);
  sl_zigbee_af_demand_response_load_control_cluster_print("Device Class: 0x%04X, status: 0x%02X\r\n",
                                                          dc,
                                                          status);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return false;
  }
  // Our thinking is that
  // device class should always match otherwise there would be no way to
  // narrow the group and there would be no way to address specific devices
  // that are not in a specific group. - WEH
  else if (!(dc & deviceClass)) {
    return false;
  }
  if (ueg == 0 || utilityEnrollmentGroup == 0 || (utilityEnrollmentGroup == ueg)) {
    return true;
  }
  return false;
}

static void addSignature(void)
{
  // The requirement of signing DRLC messages was removed from SE 1.1, however
  // we still must include the signature type and a dummy signature in the
  // message for backwards compatibility.
  appResponseData[appResponseLength] = SL_ZIGBEE_ZCL_SIGNATURE_TYPE_RESERVED;
  appResponseLength++;
  memset(&(appResponseData[appResponseLength]), 0xFF, SL_ZIGBEE_SIGNATURE_SIZE);
  appResponseLength += SL_ZIGBEE_SIGNATURE_SIZE;
}

// -------------------------------------------------------
// DEMAND RESPONSE LOAD CONTROL CLIENT
// -------------------------------------------------------
static void fillReportEventData(uint8_t commandStatus,
                                uint8_t sequenceNumber,
                                sl_zigbee_af_load_control_event_t* e,
                                uint32_t statusTime)
{
  //sl_zigbee_af_demand_response_load_control_cluster_flush();
  sl_zigbee_af_demand_response_load_control_cluster_println("= TX Report Event Status =");
  //sl_zigbee_af_demand_response_load_control_cluster_flush();
  sl_zigbee_af_demand_response_load_control_cluster_println("  eid: %08X",
                                                            e->eventId);
  //sl_zigbee_af_demand_response_load_control_cluster_flush();
  sl_zigbee_af_demand_response_load_control_cluster_println("   es: %02X",
                                                            commandStatus);
  sl_zigbee_af_demand_response_load_control_cluster_println("   st: T%08X",
                                                            statusTime);
  //sl_zigbee_af_demand_response_load_control_cluster_flush();
  sl_zigbee_af_demand_response_load_control_cluster_println("  cla: %02X",
                                                            e->criticalityLevel);
  sl_zigbee_af_demand_response_load_control_cluster_println("   ec: %02X", e->eventControl);
  sl_zigbee_af_demand_response_load_control_cluster_println("ctspa: %04X (%d)",
                                                            e->coolingTempSetPoint,
                                                            e->coolingTempSetPoint);
  //sl_zigbee_af_demand_response_load_control_cluster_flush();
  sl_zigbee_af_demand_response_load_control_cluster_println("htspa: %04X (%d)",
                                                            e->heatingTempSetPoint,
                                                            e->heatingTempSetPoint);
  sl_zigbee_af_demand_response_load_control_cluster_println("  avg: %02X", e->avgLoadPercentage);
  //sl_zigbee_af_demand_response_load_control_cluster_flush();
  sl_zigbee_af_demand_response_load_control_cluster_println("   dc: %02X", e->dutyCycle);
  sl_zigbee_af_demand_response_load_control_cluster_flush();

  appResponseLength = 0;
  (void) sl_zigbee_af_put_int8u_in_resp(ZCL_CLUSTER_SPECIFIC_COMMAND
                                        | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER);
  (void) sl_zigbee_af_put_int8u_in_resp(sequenceNumber);
  (void) sl_zigbee_af_put_int8u_in_resp(ZCL_REPORT_EVENT_STATUS_COMMAND_ID);
  (void) sl_zigbee_af_put_int32u_in_resp(e->eventId);
  (void) sl_zigbee_af_put_int8u_in_resp(commandStatus); //cmd rx
  (void) sl_zigbee_af_put_int32u_in_resp(statusTime);
  (void) sl_zigbee_af_put_int8u_in_resp(e->criticalityLevel);
  (void) sl_zigbee_af_put_int16u_in_resp(e->coolingTempSetPoint);
  (void) sl_zigbee_af_put_int16u_in_resp(e->heatingTempSetPoint);
  (void) sl_zigbee_af_put_int8u_in_resp(e->avgLoadPercentage);
  (void) sl_zigbee_af_put_int8u_in_resp(e->dutyCycle);
  (void) sl_zigbee_af_put_int8u_in_resp(e->eventControl);
  addSignature();
}

/**
 * A mechanism for retrieving the right amount of randomization
 * if startRand is true, the table is requesting the amount of
 * start randomization, otherwise it is requesting the amount of
 * duration randomization. The value returned represents seconds of
 * randomization.
 */
static uint16_t afGetRandomizationTime(uint8_t endpoint,
                                       bool startRand)
{
  uint8_t maxRandMin, dataType;
  sl_status_t status =
    sl_zigbee_af_read_attribute(endpoint,
                                ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID,
                                (startRand
                                 ? ZCL_START_RANDOMIZATION_MINUTES_ATTRIBUTE_ID
                                 : ZCL_DURATION_RANDOMIZATION_MINUTES_ATTRIBUTE_ID),
                                CLUSTER_MASK_CLIENT,
                                (uint8_t*)&maxRandMin,
                                1,
                                &dataType);
  if ( (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) || (maxRandMin == 0) ) {
    return 0; //not performed
  } else {
    uint16_t randNum, maxRandSec, randSec;
    randNum = sl_zigbee_get_pseudo_random_number();
    maxRandSec = maxRandMin * 60;
    randSec = randNum % maxRandSec;
    return randSec; //return value rep in seconds.
  }
}

void sl_zigbee_af_demand_response_load_control_cluster_client_init_cb(uint8_t endpoint)
{
  uint16_t deviceClass = SL_ZIGBEE_AF_PLUGIN_DRLC_DEVICE_CLASS;
  // Initialize device class
  (void) sl_zigbee_af_write_attribute(endpoint,
                                      ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID,
                                      ZCL_DEVICE_CLASS_VALUE_ATTRIBUTE_ID,
                                      CLUSTER_MASK_CLIENT,
                                      (uint8_t*)(&deviceClass),
                                      ZCL_INT16U_ATTRIBUTE_TYPE);

  sli_zigbee_af_load_control_event_table_init(endpoint);

  // Start tick
  sl_zigbee_zcl_schedule_client_tick(endpoint,
                                     ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID,
                                     MILLISECOND_TICKS_PER_SECOND);
}

void sl_zigbee_af_demand_response_load_control_cluster_client_tick_cb(uint8_t endpoint)
{
  sli_zigbee_af_load_control_event_table_tick(endpoint);
  sl_zigbee_zcl_schedule_client_tick(endpoint,
                                     ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID,
                                     MILLISECOND_TICKS_PER_SECOND);
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_demand_response_load_control_cluster_load_control_event_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_demand_response_and_load_control_cluster_load_control_event_command_t cmd_data;
  sl_zigbee_af_load_control_event_t e;

  if (zcl_decode_demand_response_and_load_control_cluster_load_control_event_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_demand_response_load_control_cluster_println("= RX Load Control Event =");
  sl_zigbee_af_demand_response_load_control_cluster_println("  eid: %08X", cmd_data.issuerEventId);
  sl_zigbee_af_demand_response_load_control_cluster_println(" dc/g: %04X / %02X",
                                                            cmd_data.deviceClass,
                                                            cmd_data.utilityEnrollmentGroup);
  sl_zigbee_af_demand_response_load_control_cluster_println("   st: T%08X", cmd_data.startTime);
  sl_zigbee_af_demand_response_load_control_cluster_flush();
  sl_zigbee_af_demand_response_load_control_cluster_println("  dur: %04X", cmd_data.durationInMinutes);
  sl_zigbee_af_demand_response_load_control_cluster_println("  cla: %02X", cmd_data.criticalityLevel);
  sl_zigbee_af_demand_response_load_control_cluster_println("co/ho: %02X / %02X",
                                                            cmd_data.coolingTemperatureOffset,
                                                            cmd_data.heatingTemperatureOffset);
  sl_zigbee_af_demand_response_load_control_cluster_flush();
  sl_zigbee_af_demand_response_load_control_cluster_println("ctspa: %04X (%d)",
                                                            cmd_data.coolingTemperatureSetPoint,
                                                            cmd_data.coolingTemperatureSetPoint);
  sl_zigbee_af_demand_response_load_control_cluster_println("htspa: %04X (%d)",
                                                            cmd_data.heatingTemperatureSetPoint,
                                                            cmd_data.heatingTemperatureSetPoint);
  sl_zigbee_af_demand_response_load_control_cluster_flush();
  sl_zigbee_af_demand_response_load_control_cluster_println("  avg: %02X",
                                                            cmd_data.averageLoadAdjustmentPercentage);
  sl_zigbee_af_demand_response_load_control_cluster_println("   dc: %02X", cmd_data.dutyCycle);
  sl_zigbee_af_demand_response_load_control_cluster_println("   ec: %02X", cmd_data.eventControl);
  sl_zigbee_af_demand_response_load_control_cluster_flush();

  if (!afMatchUegOrDeviceClass(sl_zigbee_af_current_endpoint(),
                               cmd_data.utilityEnrollmentGroup,
                               cmd_data.deviceClass)) {
    sl_zigbee_af_demand_response_load_control_cluster_println(
      "Neither UEG nor Device class matched.  Ignoring.");

    // CCB 1380: Device must send back a default response of success, or do
    // nothing. We choose to do nothing.
    //sl_zigbee_af_send_default_response(sl_zigbee_af_current_command(),
    //                           SL_ZIGBEE_ZCL_STATUS_NOT_FOUND);
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  // Reserved criticality level
  if (cmd_data.criticalityLevel == SL_ZIGBEE_ZCL_AMI_CRITICALITY_LEVEL_RESERVED
      || cmd_data.criticalityLevel > SL_ZIGBEE_ZCL_AMI_CRITICALITY_LEVEL_UTILITY_DEFINED6) {
    sl_zigbee_af_demand_response_load_control_cluster_println(
      "Reserved criticality level.  Ignoring.");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  e.destinationEndpoint = cmd->apsFrame->destinationEndpoint;

  e.eventId = cmd_data.issuerEventId;
  e.deviceClass = cmd_data.deviceClass;
  e.utilityEnrollmentGroup = cmd_data.utilityEnrollmentGroup;
  if (cmd_data.startTime == 0) {
    e.startTime = sl_zigbee_af_get_current_time();
  } else {
    e.startTime = cmd_data.startTime;
  }
  e.duration = cmd_data.durationInMinutes;
  e.criticalityLevel = cmd_data.criticalityLevel;
  e.coolingTempOffset = cmd_data.coolingTemperatureOffset;
  e.heatingTempOffset = cmd_data.heatingTemperatureOffset;
  e.coolingTempSetPoint = cmd_data.coolingTemperatureSetPoint;
  e.heatingTempSetPoint = cmd_data.heatingTemperatureSetPoint;
  e.avgLoadPercentage = cmd_data.averageLoadAdjustmentPercentage;
  e.dutyCycle = cmd_data.dutyCycle;
  e.eventControl = cmd_data.eventControl;
  e.optionControl = EVENT_OPT_IN_DEFAULT;
  e.startRand = 0;
  e.durationRand = 0;
  e.esiBitmask = 0;

  if (cmd_data.eventControl & RANDOMIZE_START_TIME_FLAG) {
    e.startRand = afGetRandomizationTime(sl_zigbee_af_current_endpoint(), true);
  }
  if (cmd_data.eventControl & RANDOMIZE_DURATION_TIME_FLAG) {
    e.durationRand = afGetRandomizationTime(sl_zigbee_af_current_endpoint(), false);
  }

  sl_zigbee_af_demand_response_load_control_cluster_println(
    "schedule -- start: %08X, start-random: %04X duration-random: %04X",
    e.startTime,
    e.startRand,
    e.durationRand);

  sli_zigbee_af_schedule_load_control_event(sl_zigbee_af_current_endpoint(), &e);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_demand_response_load_control_cluster_cancel_load_control_event_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_demand_response_and_load_control_cluster_cancel_load_control_event_command_t cmd_data;

  if (zcl_decode_demand_response_and_load_control_cluster_cancel_load_control_event_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  // FILTER ON DEVICE CLASS AND UTILITY ENROLLMENT GROUP
  if (!afMatchUegOrDeviceClass(sl_zigbee_af_current_endpoint(),
                               cmd_data.utilityEnrollmentGroup,
                               cmd_data.deviceClass)) {
    // CCB 1380: Device must send back a default response of success, or do
    // nothing. We choose to do nothing.
    //sl_zigbee_af_send_default_response(sl_zigbee_af_current_command(), SL_ZIGBEE_ZCL_STATUS_NOT_FOUND);
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }
  sli_zigbee_af_cancel_load_control_event(sl_zigbee_af_current_endpoint(),
                                          cmd_data.issuerEventId,
                                          cmd_data.cancelControl,
                                          cmd_data.effectiveTime);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_demand_response_load_control_cluster_cancel_all_load_control_events_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_demand_response_and_load_control_cluster_cancel_all_load_control_events_command_t cmd_data;

  if (zcl_decode_demand_response_and_load_control_cluster_cancel_all_load_control_events_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (!sli_zigbee_af_cancel_all_load_control_events(sl_zigbee_af_current_endpoint(), cmd_data.cancelControl)) {
    // We didn't find any events to cancel but we were still successful in trying
    // So we return a default response of success.
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

void sl_zigbee_af_event_action(sl_zigbee_af_load_control_event_t *event,
                               uint8_t eventStatus,
                               uint8_t sequenceNumber,
                               uint8_t esiIndex)
{
  bool sendReport;
  uint8_t i;

  // Here we defer to custom callback which can do whatever it wants with
  // the even.
  sendReport = sl_zigbee_af_drlc_event_action_cb(event,
                                                 (sl_zigbee_af_ami_event_status_t)eventStatus,
                                                 sequenceNumber);

  if (!sendReport) {
    return;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE; i++) {
    sl_zigbee_af_plugin_esi_management_esi_entry_t* esiEntry;
    uint8_t curIndex = (esiIndex < 0xFF) ? esiIndex : i;

    // Bit associated to this ESI not set, skip to the next one. If we are
    // sending to a specific ESI (i.e., esiIndex < 0xFF) the ESI table should
    // always contain an active entry at the passed index.
    if (esiIndex == 0xFF && (event->esiBitmask & BIT(curIndex)) == 0) {
      continue;
    }

    assert(curIndex < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE);

    esiEntry = sl_zigbee_af_esi_management_esi_look_up_by_index(curIndex);

    if (esiEntry != NULL) {
      fillReportEventData(eventStatus,
                          sequenceNumber,
                          event,
                          sl_zigbee_af_get_current_time());
      sl_802154_short_addr_t node_id;
      (void) sl_zigbee_lookup_node_id_by_eui64(esiEntry->eui64, &node_id);
      assert(node_id == esiEntry->nodeId);
      sl_zigbee_af_response_destination = esiEntry->nodeId;
      sl_zigbee_af_response_aps_frame.sourceEndpoint = event->destinationEndpoint;
      sl_zigbee_af_response_aps_frame.destinationEndpoint = esiEntry->endpoint;

      sl_zigbee_af_response_aps_frame.clusterId =
        ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID;
      sl_zigbee_af_send_response();

      // If we are sending the response to a specific ESI, we brake the for cycle
      // after we are done.
      if (esiIndex < 0xFF) {
        break;
      }
    } else {
      assert(false);
    }
  }
}

uint32_t sl_zigbee_af_demand_response_load_control_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                                                sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_LOAD_CONTROL_EVENT_COMMAND_ID:
      {
        status = sl_zigbee_af_demand_response_load_control_cluster_load_control_event_cb(cmd);
        break;
      }
      case ZCL_CANCEL_LOAD_CONTROL_EVENT_COMMAND_ID:
      {
        status = sl_zigbee_af_demand_response_load_control_cluster_cancel_load_control_event_cb(cmd);
        break;
      }
      case ZCL_CANCEL_ALL_LOAD_CONTROL_EVENTS_COMMAND_ID:
      {
        status = sl_zigbee_af_demand_response_load_control_cluster_cancel_all_load_control_events_cb(cmd);
        break;
      }
    }
  }

  return status;
}
