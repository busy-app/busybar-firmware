/***************************************************************************//**
 * @file
 * @brief Code to handle meter snapshot storage behavior.
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
#include "meter-snapshot-storage.h"

static sl_zigbee_af_snapshot_payload_t snapshots[SL_ZIGBEE_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY];

static sl_zigbee_af_snapshot_schedule_payload_t schedules[SL_ZIGBEE_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SCHEDULE_CAPACITY];

static uint32_t snapshotIdCounter;

static void initSnapshots(void);
static void initSchedules(void);
static sl_zigbee_af_snapshot_payload_t *allocateSnapshot(void);
static sl_zigbee_af_snapshot_schedule_payload_t *allocateSchedule(void);
static void processTiers(uint8_t endpoint,
                         sl_zigbee_af_snapshot_payload_t *snapshot,
                         bool delivered);
static void processTiersAndBlocks(uint8_t endpoint,
                                  sl_zigbee_af_snapshot_payload_t *snapshot,
                                  bool delivered);
static sl_zigbee_af_snapshot_payload_t *findSnapshot(uint32_t startTime,
                                                     uint32_t endTime,
                                                     uint8_t snapshotOffset,
                                                     uint32_t snapshotCause);
static uint16_t fillPayloadBuffer(uint8_t *buffer, sl_zigbee_af_snapshot_payload_t *payload);

static void initSnapshots(void)
{
  uint8_t i;

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY; i++) {
    snapshots[i].snapshotId = INVALID_SNAPSHOT_ID;
  }
}

static void initSchedules(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SCHEDULE_CAPACITY; i++) {
    schedules[i].snapshotScheduleId = INVALID_SNAPSHOT_SCHEDULE_ID;
  }
}

static sl_zigbee_af_snapshot_payload_t *allocateSnapshot(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY; i++) {
    if (snapshots[i].snapshotId == INVALID_SNAPSHOT_ID) {
      return &snapshots[i];
    }
  }

  return NULL;
}

static sl_zigbee_af_snapshot_schedule_payload_t *allocateSchedule(void)
{
  uint8_t i;

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SCHEDULE_CAPACITY; i++) {
    if (schedules[i].snapshotScheduleId == INVALID_SNAPSHOT_SCHEDULE_ID) {
      return &schedules[i];
    }
  }

  return NULL;
}

void sl_zigbee_af_meter_snapshot_storage_init_cb(uint8_t init_level)
{
  (void)init_level;

  snapshotIdCounter = INVALID_SNAPSHOT_ID;
  initSchedules();
  initSnapshots();
}

void sl_zigbee_af_meter_snapshot_server_schedule_snapshot_cb(uint8_t srcEndpoint,
                                                             uint8_t dstEndpoint,
                                                             sl_802154_short_addr_t dest,
                                                             uint8_t *snapshotPayload,
                                                             uint8_t *responsePayload)
{
  sl_zigbee_af_snapshot_schedule_payload_t *schedule;
  uint8_t index = 0;
  sl_zigbee_af_snapshot_schedule_confirmation_t confirmation = SL_ZIGBEE_ZCL_SNAPSHOT_SCHEDULE_CONFIRMATION_ACCEPTED;

  // Attempt to allocate a schedule
  schedule = allocateSchedule();

  if (schedule == NULL) {
    confirmation = SL_ZIGBEE_ZCL_SNAPSHOT_SCHEDULE_CONFIRMATION_INSUFFICIENT_SPACE_FOR_SNAPSHOT_SCHEDULE;
    goto kickout;
  }

  // Set the schedule
  schedule->snapshotScheduleId = snapshotPayload[index];
  if (schedule->snapshotScheduleId == INVALID_SNAPSHOT_SCHEDULE_ID) {
    confirmation = SL_ZIGBEE_ZCL_SNAPSHOT_SCHEDULE_CONFIRMATION_SNAPSHOT_SCHEDULE_NOT_CURRENTLY_AVAILABLE;
    goto kickout;
  }
  index++;

  schedule->snapshotStartDate = sl_zigbee_af_get_int32u(snapshotPayload,
                                                        index,
                                                        SNAPSHOT_SCHEDULE_PAYLOAD_SIZE);
  index += 4;
  schedule->snapshotSchedule = sl_zigbee_af_get_int24u(snapshotPayload,
                                                       index,
                                                       SNAPSHOT_SCHEDULE_PAYLOAD_SIZE);
  index += 3;
  schedule->snapshotPayloadType = sl_zigbee_af_get_int8u(snapshotPayload,
                                                         index,
                                                         SNAPSHOT_SCHEDULE_PAYLOAD_SIZE);
  index++;
  schedule->snapshotCause = sl_zigbee_af_get_int32u(snapshotPayload,
                                                    index,
                                                    SNAPSHOT_SCHEDULE_PAYLOAD_SIZE);

  // Log where to send the snapshot(s) when the time is right
  schedule->requestingId = dest;
  schedule->srcEndpoint = srcEndpoint;
  schedule->dstEndpoint = dstEndpoint;

  kickout:
  // Fill the response payload
  if (schedule == NULL) {
    responsePayload[0] = INVALID_SNAPSHOT_SCHEDULE_ID;
  } else {
    responsePayload[0] = schedule->snapshotScheduleId;
  }
  responsePayload[1] = confirmation;
}

uint32_t sl_zigbee_af_meter_snapshot_server_take_snapshot_cb(uint8_t endpoint,
                                                             uint32_t snapshotCause,
                                                             uint8_t *snapshotConfirmation)
{
  sl_zigbee_af_snapshot_payload_t *snapshot;
  uint32_t snapshotId = INVALID_SNAPSHOT_ID;
  uint8_t manualType = SL_ZIGBEE_AF_PLUGIN_METER_SNAPSHOT_STORAGE_MANUAL_SNAPSHOT_TYPE;
  bool delivered = (manualType == 0 || manualType == 2);

  // Attempt to allocate a snapshot
  snapshot = allocateSnapshot();

  if (snapshot == NULL) {
    // FAIL
    goto kickout;
  }

  // Set up snapshot identification information
  snapshot->snapshotId = ++snapshotIdCounter;

  if (snapshot->snapshotId == INVALID_SNAPSHOT_ID) {
    *snapshotConfirmation = 0x01;
    goto kickout;
  }

  snapshotId = snapshot->snapshotId;

  snapshot->payloadType = manualType;
  snapshot->tiersInUse = SUMMATION_TIERS;
  snapshot->tiersAndBlocksInUse = BLOCK_TIERS;
  snapshot->snapshotTime = sl_zigbee_af_get_current_time();
  snapshot->snapshotCause = snapshotCause | SNAPSHOT_CAUSE_MANUAL;
  sl_zigbee_af_read_attribute(endpoint,
                              ZCL_SIMPLE_METERING_CLUSTER_ID,
                              (delivered
                               ? ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID
                               : ZCL_CURRENT_SUMMATION_RECEIVED_ATTRIBUTE_ID),
                              CLUSTER_MASK_SERVER,
                              (uint8_t *)&(snapshot->currentSummation),
                              sizeof(snapshot->currentSummation),
                              NULL);
  sl_zigbee_af_read_attribute(endpoint,
                              ZCL_SIMPLE_METERING_CLUSTER_ID,
                              (delivered
                               ? ZCL_BILL_TO_DATE_DELIVERED_ATTRIBUTE_ID
                               : ZCL_BILL_TO_DATE_RECEIVED_ATTRIBUTE_ID),
                              CLUSTER_MASK_SERVER,
                              (uint8_t *)&(snapshot->billToDate),
                              sizeof(snapshot->billToDate),
                              NULL);
  sl_zigbee_af_read_attribute(endpoint,
                              ZCL_SIMPLE_METERING_CLUSTER_ID,
                              (delivered
                               ? ZCL_BILL_TO_DATE_TIME_STAMP_DELIVERED_ATTRIBUTE_ID
                               : ZCL_BILL_TO_DATE_TIME_STAMP_RECEIVED_ATTRIBUTE_ID),
                              CLUSTER_MASK_SERVER,
                              (uint8_t *)&(snapshot->billToDateTimeStamp),
                              sizeof(snapshot->billToDateTimeStamp),
                              NULL);
  sl_zigbee_af_read_attribute(endpoint,
                              ZCL_SIMPLE_METERING_CLUSTER_ID,
                              (delivered
                               ? ZCL_PROJECTED_BILL_DELIVERED_ATTRIBUTE_ID
                               : ZCL_PROJECTED_BILL_RECEIVED_ATTRIBUTE_ID),
                              CLUSTER_MASK_SERVER,
                              (uint8_t *)&(snapshot->projectedBill),
                              sizeof(snapshot->projectedBill),
                              NULL);
  sl_zigbee_af_read_attribute(endpoint,
                              ZCL_SIMPLE_METERING_CLUSTER_ID,
                              (delivered
                               ? ZCL_PROJECTED_BILL_TIME_STAMP_DELIVERED_ATTRIBUTE_ID
                               : ZCL_PROJECTED_BILL_TIME_STAMP_RECEIVED_ATTRIBUTE_ID),
                              CLUSTER_MASK_SERVER,
                              (uint8_t *)&(snapshot->projectedBillTimeStamp),
                              sizeof(snapshot->projectedBillTimeStamp),
                              NULL);
  sl_zigbee_af_read_attribute(endpoint,
                              ZCL_SIMPLE_METERING_CLUSTER_ID,
                              (delivered
                               ? ZCL_BILL_DELIVERED_TRAILING_DIGIT_ATTRIBUTE_ID
                               : ZCL_BILL_RECEIVED_TRAILING_DIGIT_ATTRIBUTE_ID),
                              CLUSTER_MASK_SERVER,
                              (uint8_t *)&(snapshot->billTrailingDigit),
                              sizeof(snapshot->billTrailingDigit),
                              NULL);

  processTiers(endpoint, snapshot, delivered);
  processTiersAndBlocks(endpoint, snapshot, delivered);

  *snapshotConfirmation = 0x00;
  kickout:
  return snapshotId;
}

void sl_zigbee_af_meter_snapshot_server_get_snapshot_cb(uint8_t srcEndpoint,
                                                        uint8_t dstEndpoint,
                                                        sl_802154_short_addr_t dest,
                                                        uint8_t *snapshotCriteria)
{
  uint8_t snapshotPayload[SNAPSHOT_PAYLOAD_SIZE];
  uint16_t payloadSize;
  sl_zigbee_af_snapshot_payload_t* payload;
  uint32_t startTime = sl_zigbee_af_get_int32u(snapshotCriteria, 0, 13);
  uint32_t endTime = sl_zigbee_af_get_int32u(snapshotCriteria, 4, 13);
  uint8_t snapshotOffset = sl_zigbee_af_get_int8u(snapshotCriteria, 8, 13);
  uint32_t snapshotCause = sl_zigbee_af_get_int32u(snapshotCriteria, 9, 13);

  sl_zigbee_af_core_println("Start Time %u snapshot Offset %u SnapShotCause %u", startTime, snapshotOffset, snapshotCause);

  // Find the snapshot
  payload = findSnapshot(startTime,
                         endTime,
                         snapshotOffset,
                         snapshotCause);
  if (payload == NULL) {
    sl_zigbee_af_cluster_command_t const * const cmd = sl_zigbee_af_current_command();
    sl_zigbee_af_core_println("No snapshot matches specified criteria");
    if (cmd != NULL) {
      // Send default response only if this is a response to a snapshot request.
      // Not if this is an unsolicited command (e.g. from a CLI).
      // Using sl_zigbee_af_send_default_response_with_cb() instead of
      // sl_zigbee_af_send_immediate_default_response() because the latter expands to
      // former anyway and calls sl_zigbee_af_current_command(). This way we have to
      // call sl_zigbee_af_current_command() only once.
      sl_zigbee_af_send_default_response_with_cb(sl_zigbee_af_current_command(),
                                                 SL_ZIGBEE_ZCL_STATUS_NOT_FOUND,
                                                 NULL);
    }
    return;
  }

  // Fill our payload buffer
  payloadSize = fillPayloadBuffer(snapshotPayload, payload);

  sl_zigbee_af_fill_command_simple_metering_cluster_publish_snapshot(payload->snapshotId,
                                                                     payload->snapshotTime,
                                                                     1,
                                                                     0,
                                                                     1,
                                                                     payload->snapshotCause,
                                                                     payload->payloadType,
                                                                     (uint8_t *)payload,
                                                                     payloadSize);
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dest);
}

static void processTiers(uint8_t endpoint,
                         sl_zigbee_af_snapshot_payload_t *snapshot,
                         bool delivered)
{
  uint16_t i, attr = (delivered
                      ? ZCL_CURRENT_TIER1_SUMMATION_DELIVERED_ATTRIBUTE_ID
                      : ZCL_CURRENT_TIER1_SUMMATION_RECEIVED_ATTRIBUTE_ID);

  for (i = 0; i < SUMMATION_TIERS; i++) {
    sl_zigbee_af_read_attribute(endpoint,
                                ZCL_SIMPLE_METERING_CLUSTER_ID,
                                attr + i,
                                CLUSTER_MASK_SERVER,
                                (uint8_t *)(&(snapshot->tierSummation[i * 6])),
                                6,
                                NULL);
  }
}

static void processTiersAndBlocks(uint8_t endpoint,
                                  sl_zigbee_af_snapshot_payload_t *snapshot,
                                  bool delivered)
{
  uint16_t i, attr = (delivered
                      ? ZCL_CURRENT_TIER1_BLOCK1_SUMMATION_DELIVERED_ATTRIBUTE_ID
                      : ZCL_CURRENT_TIER1_BLOCK1_SUMMATION_RECEIVED_ATTRIBUTE_ID);

  for (i = 0; i < BLOCK_TIERS; i++) {
    sl_zigbee_af_read_attribute(endpoint,
                                ZCL_SIMPLE_METERING_CLUSTER_ID,
                                attr + i,
                                CLUSTER_MASK_SERVER,
                                (uint8_t *)(&(snapshot->tierBlockSummation[i * 6])),
                                6,
                                NULL);
  }
}

static sl_zigbee_af_snapshot_payload_t *findSnapshot(uint32_t startTime,
                                                     uint32_t endTime,
                                                     uint8_t snapshotOffset,
                                                     uint32_t snapshotCause)
{
  uint8_t i, offsetCount = snapshotOffset;
  sl_zigbee_af_snapshot_payload_t *snapshot;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY; i++) {
    snapshot = &(snapshots[i]);
    if (snapshot->snapshotTime >= startTime && snapshot->snapshotTime <= endTime) {
      if ((snapshot->snapshotCause & snapshotCause)) {
        if (offsetCount == 0) {
          return &(snapshots[i]);
        }
        offsetCount--;
      }
    }
  }

  return NULL;
}

static uint16_t fillPayloadBuffer(uint8_t *buffer, sl_zigbee_af_snapshot_payload_t *payload)
{
  uint16_t index = 0;
  uint16_t i;
  bool block = (payload->payloadType == 2 || payload->payloadType == 3);

  memmove(buffer, payload->currentSummation, 6);
  index += 6;

  sl_zigbee_af_copy_int32u(buffer, index, payload->billToDate);
  index += 4;

  sl_zigbee_af_copy_int32u(buffer, index, payload->billToDateTimeStamp);
  index += 4;

  sl_zigbee_af_copy_int32u(buffer, index, payload->projectedBill);
  index += 4;

  sl_zigbee_af_copy_int32u(buffer, index, payload->projectedBillTimeStamp);
  index += 4;

  sl_zigbee_af_copy_int8u(buffer, index, payload->billTrailingDigit);
  index++;

  sl_zigbee_af_copy_int8u(buffer, index, payload->tiersInUse);
  index++;

  for (i = 0; i < SUMMATION_TIERS; i++) {
    memmove((buffer + index), &(payload->tierSummation[i * 6]), 6);
    index += 6;
  }

  // Our payload is bigger if we're operating on the block information set
  if (block) {
    sl_zigbee_af_copy_int8u(buffer, index, payload->tiersAndBlocksInUse);
    index++;

    for (i = 0; i < BLOCK_TIERS; i++) {
      memmove((buffer + index), &(payload->tierBlockSummation[i * 6]), 6);
      index += 6;
    }
  }

  return index;
}
