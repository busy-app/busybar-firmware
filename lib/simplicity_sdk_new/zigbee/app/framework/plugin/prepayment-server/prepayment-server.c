/***************************************************************************//**
 * @file
 * @brief Implemented routines for prepayment server.
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

#include "app/framework/include/af.h"
#include "app/framework/util/util.h"
#include "prepayment-server.h"
#include "prepayment-debt-log.h"
#include "prepayment-debt-schedule.h"
#include "prepayment-modes-table.h"

#include "prepayment-server-config.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "zap-cluster-command-parser.h"
#include "../calendar-client/calendar-client.h"
extern void sl_zigbee_af_prepayment_snapshot_storage_init_cb(void);

typedef uint16_t sli_zigbee_payment_control_configuration_t;
typedef uint8_t  sli_zigbee_friendly_credit_t;

extern uint8_t sl_zigbee_af_prepayment_server_get_snapshot_cb(sl_802154_short_addr_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint,
                                                              uint32_t startTime,
                                                              uint32_t endTime,
                                                              uint8_t  snapshotOffset,
                                                              uint32_t snapshotCause);

void sl_zigbee_af_prepayment_cluster_server_init_cb(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PREPAYMENT_CLUSTER_ID);

  sl_zigbee_af_prepayment_snapshot_storage_init_cb();
  sli_zigbee_af_init_prepayment_modes_table();
  sl_zigbee_af_prepayment_server_init_debt_log();
  sl_zigbee_af_prepayment_server_init_debt_schedule();

  if ( ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX ) {
    return;
  }
}

//-----------------------
// ZCL commands callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_select_available_emergency_credit_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;

  sl_zigbee_af_prepayment_cluster_println("Rx: Select Available Emergency Credit");
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

#define CUTOFF_UNCHANGED 0xFFFFFFFF
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_change_payment_mode_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_prepayment_cluster_change_payment_mode_command_t cmd_data;
  // The requester can be obtained with sl_zigbee_af_response_destination;
  uint8_t endpoint;
  sli_zigbee_friendly_credit_t friendlyCredit;
  uint32_t friendlyCreditCalendarId;
  uint32_t emergencyCreditLimit;
  uint32_t emergencyCreditThreshold;
  uint8_t  dataType;
  uint8_t  i;

  if (zcl_decode_prepayment_cluster_change_payment_mode_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_prepayment_cluster_println("RX: ChangePaymentMode, pid=0x%08X, eid=0x%08X, cfg=0x%04X", cmd_data.providerId, cmd_data.issuerEventId, cmd_data.proposedPaymentControlConfiguration);
  endpoint = sl_zigbee_af_current_endpoint();

  if ( cmd_data.cutOffValue != CUTOFF_UNCHANGED ) {
#ifdef ZCL_USING_PREPAYMENT_CLUSTER_CUT_OFF_VALUE_ATTRIBUTE
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID,
                                        ZCL_CUT_OFF_VALUE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        (uint8_t *)&cmd_data.cutOffValue, ZCL_INT32S_ATTRIBUTE_TYPE);
#endif
  }

  sl_zigbee_af_prepayment_schedule_prepayment_mode(sl_zigbee_af_current_endpoint(), cmd_data.providerId, cmd_data.issuerEventId, cmd_data.implementationDateTime,
                                                   cmd_data.proposedPaymentControlConfiguration);

  // Setup the friendly credit & emergency credit limit attributes.
#ifdef SL_CATALOG_ZIGBEE_CALENDAR_CLIENT_PRESENT
  i = sl_zigbee_af_calendar_client_get_calendar_index_by_type(endpoint, SL_ZIGBEE_ZCL_CALENDAR_TYPE_FRIENDLY_CREDIT_CALENDAR);
  friendlyCredit = (i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS) ? 0x01 : 0x00;
  friendlyCreditCalendarId = sl_zigbee_af_calendar_client_get_calendar_id(endpoint, i);
#else // !SL_CATALOG_ZIGBEE_CALENDAR_CLIENT_PRESENT
  friendlyCredit = 0x00;
  friendlyCreditCalendarId = SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_INVALID_CALENDAR_ID;
#endif // SL_CATALOG_ZIGBEE_CALENDAR_CLIENT_PRESENT

#if (!defined ZCL_USING_PREPAYMENT_CLUSTER_EMERGENCY_CREDIT_LIMIT_ALLOWANCE_ATTRIBUTE) \
  || (!defined ZCL_USING_PREPAYMENT_CLUSTER_EMERGENCY_CREDIT_THRESHOLD_ATTRIBUTE)
#error "Prepayment Emergency Credit Limit/Allowance and Threshold attributes required for this plugin!"
#endif
  sl_zigbee_af_read_attribute(sl_zigbee_af_current_endpoint(), ZCL_PREPAYMENT_CLUSTER_ID,
                              ZCL_EMERGENCY_CREDIT_LIMIT_ALLOWANCE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                              (uint8_t *)&emergencyCreditLimit, 4, &dataType);
  sl_zigbee_af_read_attribute(sl_zigbee_af_current_endpoint(), ZCL_PREPAYMENT_CLUSTER_ID,
                              ZCL_EMERGENCY_CREDIT_THRESHOLD_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                              (uint8_t *)&emergencyCreditThreshold, 4, &dataType);

#ifdef SL_CATALOG_ZIGBEE_GBCS_COMPATIBILITY_PRESENT
  // GBCS explicitly lists some commands that need to be sent with "disable
  // default response" flag set. This is one of them.
  // We make it conditional on GBCS so it does not affect standard SE apps.
  sl_zigbee_af_set_disable_default_response(SL_ZIGBEE_AF_DISABLE_DEFAULT_RESPONSE_ONE_SHOT);
#endif

  sl_zigbee_af_fill_command_prepayment_cluster_change_payment_mode_response(friendlyCredit, friendlyCreditCalendarId,
                                                                            emergencyCreditLimit, emergencyCreditThreshold);
  sl_zigbee_af_send_response();
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_emergency_credit_setup_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_prepayment_cluster_emergency_credit_setup_command_t cmd_data;

  if (zcl_decode_prepayment_cluster_emergency_credit_setup_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_prepayment_cluster_println("Rx: Emergency Credit Setup");
#ifdef ZCL_USING_PREPAYMENT_CLUSTER_EMERGENCY_CREDIT_LIMIT_ALLOWANCE_ATTRIBUTE
  (void) sl_zigbee_af_write_attribute(sl_zigbee_af_current_endpoint(), ZCL_PREPAYMENT_CLUSTER_ID,
                                      ZCL_EMERGENCY_CREDIT_LIMIT_ALLOWANCE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                      (uint8_t *)&cmd_data.emergencyCreditLimit, ZCL_INT32U_ATTRIBUTE_TYPE);
#else
  #error "Prepayment Emergency Credit Limit Allowance attribute is required for this plugin."
#endif

#ifdef ZCL_USING_PREPAYMENT_CLUSTER_EMERGENCY_CREDIT_THRESHOLD_ATTRIBUTE
  (void) sl_zigbee_af_write_attribute(sl_zigbee_af_current_endpoint(), ZCL_PREPAYMENT_CLUSTER_ID,
                                      ZCL_EMERGENCY_CREDIT_THRESHOLD_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                      (uint8_t *)&cmd_data.emergencyCreditThreshold, ZCL_INT32U_ATTRIBUTE_TYPE);
#else
  #error "Prepayment Emergency Credit Threshold attribute is required for this plugin."
#endif
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

enum {
  CREDIT_ADJUSTMENT_TYPE_INCREMENTAL = 0x00,
  CREDIT_ADJUSTMENT_TYPE_ABSOLUTE    = 0x01,
};

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_credit_adjustment_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_prepayment_cluster_credit_adjustment_command_t cmd_data;

  if (zcl_decode_prepayment_cluster_credit_adjustment_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

#ifdef ZCL_USING_PREPAYMENT_CLUSTER_CREDIT_REMAINING_TIMESTAMP_ATTRIBUTE
  uint32_t currTimeUtc;
#endif
  int32_t currCreditAdjustmentValue;
  uint8_t  dataType;

  sl_zigbee_af_prepayment_cluster_println("Rx: Credit Adjustment");
#ifdef ZCL_USING_PREPAYMENT_CLUSTER_CREDIT_REMAINING_ATTRIBUTE
  if ( cmd_data.creditAdjustmentType == CREDIT_ADJUSTMENT_TYPE_INCREMENTAL ) {
    // Read current value, then add it to the adjustment.
    sl_zigbee_af_read_attribute(sl_zigbee_af_current_endpoint(), ZCL_PREPAYMENT_CLUSTER_ID,
                                ZCL_CREDIT_REMAINING_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                (uint8_t *)&currCreditAdjustmentValue, 4, &dataType);
    currCreditAdjustmentValue += cmd_data.creditAdjustmentValue;
  }

  (void) sl_zigbee_af_write_attribute(sl_zigbee_af_current_endpoint(), ZCL_PREPAYMENT_CLUSTER_ID,
                                      ZCL_CREDIT_REMAINING_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                      (uint8_t *)&cmd_data.creditAdjustmentValue, ZCL_INT32S_ATTRIBUTE_TYPE);
#else
  #error "Prepayment Credit Adjustment attribute is required for this plugin."
#endif

#ifdef ZCL_USING_PREPAYMENT_CLUSTER_CREDIT_REMAINING_TIMESTAMP_ATTRIBUTE
  // This one is optional - we'll track it if supported.
  currTimeUtc = sl_zigbee_af_get_current_time();
  (void) sl_zigbee_af_write_attribute(sl_zigbee_af_current_endpoint(), ZCL_PREPAYMENT_CLUSTER_ID,
                                      ZCL_CREDIT_REMAINING_TIMESTAMP_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                      (uint8_t *)&currTimeUtc, ZCL_UTC_TIME_ATTRIBUTE_TYPE);
#endif

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

#define MAX_SNAPSHOT_PAYLOAD_LEN  24
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_get_prepay_snapshot_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_prepayment_cluster_get_prepay_snapshot_command_t cmd_data;
  sl_802154_short_addr_t nodeId;
  uint8_t srcEndpoint, dstEndpoint;

  if (zcl_decode_prepayment_cluster_get_prepay_snapshot_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_prepayment_cluster_println("RX: GetPrepaySnapshot, st=0x%08X, offset=%d, cause=%d",
                                          cmd_data.earliestStartTime, cmd_data.snapshotOffset, cmd_data.snapshotCause);
  nodeId = sl_zigbee_af_current_command()->source;
  srcEndpoint = sl_zigbee_af_get_command_aps_frame()->destinationEndpoint;
  dstEndpoint = sl_zigbee_af_get_command_aps_frame()->sourceEndpoint;
  sl_zigbee_af_prepayment_cluster_println("... from 0x%04X, ep=%d", nodeId, dstEndpoint);

  sl_zigbee_af_prepayment_server_get_snapshot_cb(nodeId, srcEndpoint, dstEndpoint,
                                                 cmd_data.earliestStartTime, cmd_data.latestEndTime, cmd_data.snapshotOffset, cmd_data.snapshotCause);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_change_debt_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_get_debt_repayment_log_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_consumer_top_up_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_get_top_up_log_cb(sl_zigbee_af_cluster_command_t *cmd);

uint32_t sl_zigbee_af_prepayment_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                              sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  switch (cmd->commandId) {
    case ZCL_SELECT_AVAILABLE_EMERGENCY_CREDIT_COMMAND_ID:
    {
      status = sl_zigbee_af_prepayment_cluster_select_available_emergency_credit_cb(cmd);
      break;
    }
    case ZCL_CHANGE_DEBT_COMMAND_ID:
    {
      status = sl_zigbee_af_prepayment_cluster_change_debt_cb(cmd);
      break;
    }
    case ZCL_EMERGENCY_CREDIT_SETUP_COMMAND_ID:
    {
      status = sl_zigbee_af_prepayment_cluster_emergency_credit_setup_cb(cmd);
      break;
    }
    case ZCL_CONSUMER_TOP_UP_COMMAND_ID:
    {
      status = sl_zigbee_af_prepayment_cluster_consumer_top_up_cb(cmd);
      break;
    }
    case ZCL_CREDIT_ADJUSTMENT_COMMAND_ID:
    {
      status = sl_zigbee_af_prepayment_cluster_credit_adjustment_cb(cmd);
      break;
    }
    case ZCL_CHANGE_PAYMENT_MODE_COMMAND_ID:
    {
      status = sl_zigbee_af_prepayment_cluster_change_payment_mode_cb(cmd);
      break;
    }
    case ZCL_GET_PREPAY_SNAPSHOT_COMMAND_ID:
    {
      status = sl_zigbee_af_prepayment_cluster_get_prepay_snapshot_cb(cmd);
      break;
    }
    case ZCL_GET_TOP_UP_LOG_COMMAND_ID:
    {
      status = sl_zigbee_af_prepayment_cluster_get_top_up_log_cb(cmd);
      break;
    }
    case ZCL_GET_DEBT_REPAYMENT_LOG_COMMAND_ID:
    {
      status = sl_zigbee_af_prepayment_cluster_get_debt_repayment_log_cb(cmd);
      break;
    }
    default:
    {
      // Unrecognized command ID, error status will apply.
      break;
    }
  }

  return status;
}
