/***************************************************************************//**
 * @file
 * @brief Routines for interacting with the prepayment-server.
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
#include "app/framework/util/af-main.h"
#include "app/framework/util/util.h"
#include "prepayment-server.h"
#include "prepayment-modes-table.h"
#include "prepayment-debt-log.h"
#include "prepayment-debt-schedule.h"
#include "prepayment-tick.h"
#include "app/framework/plugin/prepayment-snapshot-storage/prepayment-snapshot-storage.h"

#include "prepayment-server-config.h"

typedef uint16_t sli_zigbee_payment_control_configuration_t;
typedef uint32_t sli_zigbee_prepay_snapshot_payload_cause_t;
typedef uint8_t  sli_zigbee_prepay_snapshot_payload_type_t;
typedef uint8_t  sli_zigbee_friendly_credit_t;

static void print_af_time(sl_zigbee_af_time_struct_t *pafTime)
{
  sl_zigbee_af_prepayment_cluster_println("== AF TIME ==");
  sl_zigbee_af_prepayment_cluster_println("  Year=%d", pafTime->year);
  sl_zigbee_af_prepayment_cluster_println("  Month=%d", pafTime->month);
  sl_zigbee_af_prepayment_cluster_println("  Day=%d", pafTime->day);
  sl_zigbee_af_prepayment_cluster_println("  Hour=%d", pafTime->hours);
  sl_zigbee_af_prepayment_cluster_println("  Min=%d", pafTime->minutes);
  sl_zigbee_af_prepayment_cluster_println("  Sec=%d", pafTime->seconds);
}

#define SECONDS_PER_DAY (3600 * 24)
extern sli_zigbee_af_debt_schedule_entry DebtSchedule[];

#include "sl_cli.h"

void sli_zigbee_af_prepayment_server_cli_init(sl_cli_command_arg_t *arguments)
{
  uint8_t endpoint;
  endpoint = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_prepayment_cluster_server_init_cb(endpoint);
}

// plugin prepayment-server writeAttribute <endpoint:1> <attributeId:2> <attributeType:1> <numBytes:1> <value:4>
void sli_zigbee_af_prepayment_server_cli_write_attribute(sl_cli_command_arg_t *arguments)
{
  uint16_t attributeId;
  uint8_t  status;
  uint8_t  endpoint;
  uint8_t  attributeType;
  uint32_t value;

  endpoint      = sl_cli_get_argument_uint8(arguments, 0);
  attributeId   = sl_cli_get_argument_uint16(arguments, 1);
  attributeType = sl_cli_get_argument_uint8(arguments, 2);
  // NOTE unused cli argument
  // uint8_t numBytes      = sl_cli_get_argument_uint8(arguments, 3);
  value         = sl_cli_get_argument_uint32(arguments, 4);

  status = sl_zigbee_af_write_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId,
                                        CLUSTER_MASK_SERVER, (uint8_t *)&value, attributeType);
  sl_zigbee_af_prepayment_cluster_println("Write Attribute status=0x%02X", status);
}

void sli_zigbee_af_prepayment_server_cli_verify_payment_mode(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_payment_control_configuration_t expectedPaymentControlConfiguration;
  sli_zigbee_payment_control_configuration_t readPaymentControlConfiguration;
  sl_zigbee_af_status_t status;
  uint8_t endpoint;
  uint8_t dataType;

  endpoint = sl_cli_get_argument_uint8(arguments, 0);
  expectedPaymentControlConfiguration = sl_cli_get_argument_uint16(arguments, 1);
  status = sl_zigbee_af_read_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID,
                                       ZCL_PAYMENT_CONTROL_CONFIGURATION_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                       (uint8_t *)&readPaymentControlConfiguration, 2, &dataType);

  if ( (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) && (expectedPaymentControlConfiguration == readPaymentControlConfiguration) ) {
    sl_zigbee_af_prepayment_cluster_println("Payment Mode Match Success - %d", readPaymentControlConfiguration);
  } else {
    sl_zigbee_af_prepayment_cluster_println("Payment Mode Failed Match, status=0x%02X, read=%d, exp=%d", status, readPaymentControlConfiguration, expectedPaymentControlConfiguration);
  }
}

void sli_zigbee_af_prepayment_server_cli_verify_attribute(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_status_t status;
  uint32_t readAttributeValue;
  uint32_t expectedAttributeValue;
  uint16_t attributeId;

  uint8_t endpoint;
  uint8_t dataType;
  uint8_t attributeSize;

  endpoint = sl_cli_get_argument_uint8(arguments, 0);
  attributeId = sl_cli_get_argument_uint16(arguments, 1);
  attributeSize = sl_cli_get_argument_uint8(arguments, 2);
  expectedAttributeValue = sl_cli_get_argument_uint32(arguments, 3);
  status = sl_zigbee_af_read_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID,
                                       attributeId, CLUSTER_MASK_SERVER,
                                       (uint8_t *)&readAttributeValue, attributeSize, &dataType);

  if ( (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) && (expectedAttributeValue == readAttributeValue) ) {
    sl_zigbee_af_prepayment_cluster_println("Attribute Read Match Success - %d", status);
  } else {
    sl_zigbee_af_prepayment_cluster_println("Attribute Read Failed Match status=0x%02X, read=%d, exp=%d", status, readAttributeValue, expectedAttributeValue);
  }
}

#define MAX_SNAPSHOT_PAYLOAD_LEN  24
void sli_zigbee_af_prepayment_server_cli_publish_prepay_snapshot(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t nodeId = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(arguments, 0);
  uint8_t srcEndpoint = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t dstEndpoint = sl_cli_get_argument_uint8(arguments, 2);
  uint32_t snapshotTableIndex = sl_cli_get_argument_uint32(arguments, 3);

  sl_zigbee_af_prepayment_snapshot_storage_publish_snapshot(nodeId,
                                                            srcEndpoint,
                                                            dstEndpoint,
                                                            snapshotTableIndex);
}

void sli_zigbee_af_prepayment_server_cli_add_snapshot_event(sl_cli_command_arg_t *arguments)
{
  uint32_t snapshotCause;
  uint8_t  endpoint;

  endpoint = sl_cli_get_argument_uint8(arguments, 0);
  snapshotCause = sl_cli_get_argument_uint32(arguments, 1);
  sl_zigbee_af_prepayment_cluster_println("CLI Add Snapshot Event, endpoint=%d cause=0x%08X", endpoint, snapshotCause);

  sl_zigbee_af_prepayment_snapshot_storage_take_snapshot(endpoint, snapshotCause);
}

void sli_zigbee_af_prepayment_read_debt_log(sl_cli_command_arg_t *arguments)
{
  uint8_t index;
  index = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_prepayment_print_debt_log_index(index);
}

void sli_zigbee_af_prepayment_read_debt_attributes(sl_cli_command_arg_t *arguments)
{
  uint8_t endpoint;
  uint8_t index;
  endpoint = sl_cli_get_argument_uint8(arguments, 0);
  index = sl_cli_get_argument_uint8(arguments, 1);
  sl_zigbee_af_prepayment_print_debt_attributes(endpoint, index);
}

void sli_zigbee_af_prepayment_get_top_up_percentage(sl_cli_command_arg_t *arguments)
{
  uint8_t  endpoint;
  uint32_t topUpValue;

  endpoint = sl_cli_get_argument_uint8(arguments, 0);
  topUpValue = sl_cli_get_argument_uint32(arguments, 1);
  sl_zigbee_af_prepayment_get_debt_recovery_top_up_percentage(endpoint, topUpValue);
}

void sli_zigbee_af_prepayment_check_calendar_cli(sl_cli_command_arg_t *arguments)
{
  uint32_t utcTime;
  uint32_t calcUtcTime;
  sl_zigbee_af_time_struct_t afTime;

  utcTime = sl_cli_get_argument_uint32(arguments, 0);
  sl_zigbee_af_fill_time_struct_from_utc(utcTime, &afTime);
  print_af_time(&afTime);

  calcUtcTime = sl_zigbee_af_get_utc_from_time_struct(&afTime);
  if ( calcUtcTime == utcTime ) {
    sl_zigbee_af_prepayment_cluster_println("= UTC Times Match, 0x%08X", calcUtcTime);
  } else {
    sl_zigbee_af_prepayment_cluster_println(" ERROR: UTC Times Don't Match, 0x%08X != 0x%08X", utcTime, calcUtcTime);
  }
}

void sli_zigbee_af_prepayment_get_weekday_cli(sl_cli_command_arg_t *arguments)
{
  uint8_t weekday;
  uint32_t utcTime;

  utcTime = sl_cli_get_argument_uint32(arguments, 0);
  weekday = sl_zigbee_af_get_weekday_from_utc(utcTime);
  sl_zigbee_af_prepayment_cluster_println("UTC Time=0x%08X, Weekday=%d", utcTime, weekday);
}

void sli_zigbee_af_prepayment_schedule_debt_repayment_cli(sl_cli_command_arg_t *arguments)
{
  uint8_t  endpoint;
  uint8_t  debtType;
  uint16_t collectionTime;
  uint32_t issuerEventId;
  uint32_t collectionTimeSec;
  uint32_t startTime;
  uint8_t  collectionFrequency;
  uint8_t  i;

  endpoint = sl_cli_get_argument_uint8(arguments, 0);
  issuerEventId = sl_cli_get_argument_uint32(arguments, 1);
  debtType = sl_cli_get_argument_uint8(arguments, 2);
  collectionTime = sl_cli_get_argument_uint16(arguments, 3);
  startTime = sl_cli_get_argument_uint32(arguments, 4);
  collectionFrequency = sl_cli_get_argument_uint8(arguments, 5);

  sl_zigbee_af_prepayment_server_schedule_debt_repayment(endpoint, issuerEventId, debtType, collectionTime, startTime, collectionFrequency);

  // Convert collectionTime (mins) to seconds for future comparisons.
  collectionTimeSec = ((uint32_t)collectionTime) * 60;

  // After calling the ScheduleDebtRepayment() function, verify a couple things.
  if ( debtType >= 3 ) {
    sl_zigbee_af_prepayment_cluster_println("Debt type out of bounds");
  } else {
    i = debtType;
    if ( (DebtSchedule[i].firstCollectionTimeSec >= startTime)
         && (DebtSchedule[i].issuerEventId == issuerEventId)
         && (DebtSchedule[i].collectionFrequency == collectionFrequency)
         && (DebtSchedule[i].nextCollectionTimeUtc >= startTime)
         && ((DebtSchedule[i].nextCollectionTimeUtc % SECONDS_PER_DAY) == collectionTimeSec)
         && ((DebtSchedule[i].firstCollectionTimeSec % SECONDS_PER_DAY) == collectionTimeSec) ) {
      sl_zigbee_af_prepayment_cluster_println("Valid Debt Schedule");
    } else {
      sl_zigbee_af_prepayment_cluster_println("INVALID Debt Schedule");
      sl_zigbee_af_prepayment_cluster_println("  first=%d, startTime=%d", DebtSchedule[i].firstCollectionTimeSec, startTime);
      sl_zigbee_af_prepayment_cluster_println("  issuerEvtId=%d, %d", DebtSchedule[i].issuerEventId, issuerEventId);
      sl_zigbee_af_prepayment_cluster_println("  collFreq=%d, %d", DebtSchedule[i].collectionFrequency, collectionFrequency);
      sl_zigbee_af_prepayment_cluster_println("  nextColl=%d, startTime=%d", DebtSchedule[i].nextCollectionTimeUtc, startTime);
      sl_zigbee_af_prepayment_cluster_println("  nextMOD=%d, collectTimeSec=%d", (DebtSchedule[i].nextCollectionTimeUtc % SECONDS_PER_DAY), collectionTimeSec);
      sl_zigbee_af_prepayment_cluster_println("  firstMOD=%d, collectTimeSec=%d", (DebtSchedule[i].firstCollectionTimeSec % SECONDS_PER_DAY), collectionTimeSec);
    }
  }
}
