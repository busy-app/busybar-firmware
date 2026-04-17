/***************************************************************************//**
 * @file
 * @brief Implemented routines for storing prepayment snapshots.
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
#include "prepayment-snapshot-storage.h"

#include "prepayment-snapshot-storage-config.h"

// Dependency check
#if ( (!defined ZCL_USING_PREPAYMENT_CLUSTER_ACCUMULATED_DEBT_ATTRIBUTE)          \
  || (!defined ZCL_USING_PREPAYMENT_CLUSTER_DEBT_AMOUNT_1_ATTRIBUTE)              \
  || (!defined ZCL_USING_PREPAYMENT_CLUSTER_DEBT_AMOUNT_2_ATTRIBUTE)              \
  || (!defined ZCL_USING_PREPAYMENT_CLUSTER_DEBT_AMOUNT_3_ATTRIBUTE)              \
  || (!defined ZCL_USING_PREPAYMENT_CLUSTER_EMERGENCY_CREDIT_REMAINING_ATTRIBUTE) \
  || (!defined ZCL_USING_PREPAYMENT_CLUSTER_CREDIT_REMAINING_ATTRIBUTE) )
// These are required for the prepayment snapshot.
  #error "Prepayment snapshots require the Accumulated Debt, Debt Amount (1-3), Emergency Credit Remaining, and Credit Remaining Attributes"
#endif

static void initSnapshots(void);
static void initSchedules(void);
static sl_zigbee_af_prepayment_snapshot_payload_t *allocateSnapshot(void);
//static sl_zigbee_af_prepayment_snapshot_schedule_payload_t *allocateSchedule( void );
static uint16_t fillSnapshotPayloadBuffer(uint8_t *buffer, sl_zigbee_af_prepayment_snapshot_payload_t *snapshot);
static sl_zigbee_af_prepayment_snapshot_payload_t *findSnapshot(uint32_t startTime,
                                                                uint32_t endTime,
                                                                uint8_t snapshotOffset,
                                                                uint32_t snapshotCause,
                                                                uint8_t *totalMatchesFound);
static void snapshotPrintInfo(sl_zigbee_af_prepayment_snapshot_payload_t *snapshot);

static sl_zigbee_af_prepayment_snapshot_payload_t snapshots[SL_ZIGBEE_AF_PLUGIN_PREPAYMENT_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY];
static sl_zigbee_af_prepayment_snapshot_schedule_payload_t schedules[SL_ZIGBEE_AF_PLUGIN_PREPAYMENT_SNAPSHOT_STORAGE_SCHEDULE_CAPACITY];

static uint32_t SnapshotIdCounter;

static void initSnapshots()
{
  uint8_t i;
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PREPAYMENT_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY; i++ ) {
    snapshots[i].snapshotId = INVALID_SNAPSHOT_ID;
  }
}

static void initSchedules()
{
  uint8_t i;
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PREPAYMENT_SNAPSHOT_STORAGE_SCHEDULE_CAPACITY; i++ ) {
    schedules[i].snapshotScheduleId = INVALID_SNAPSHOT_SCHEDULE_ID;
  }
}

void sl_zigbee_af_prepayment_snapshot_storage_init_cb(uint8_t init_level)
{
  (void)init_level;

  SnapshotIdCounter = INVALID_SNAPSHOT_ID;
  initSchedules();
  initSnapshots();
  sl_zigbee_af_app_println("Init Snapshot Table");
}

static sl_zigbee_af_prepayment_snapshot_payload_t *allocateSnapshot()
{
  uint8_t i;
  uint32_t minSnapshotId = 0xFFFFFFFF;
  uint8_t minSnapshotIdIndex = 0xFF;

  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PREPAYMENT_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY; i++ ) {
    if ( snapshots[i].snapshotId == INVALID_SNAPSHOT_ID ) {
      return &snapshots[i];
    } else if ( snapshots[i].snapshotId < minSnapshotId ) {
      // Find snapshot with smallest ID if an unused snapshot doesn't exist.
      minSnapshotId = snapshots[i].snapshotId;
      minSnapshotIdIndex = i;
    }
  }
  if ( minSnapshotIdIndex < SL_ZIGBEE_AF_PLUGIN_PREPAYMENT_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY ) {
    return &snapshots[minSnapshotIdIndex];
  }
  return NULL;
}

/*
   static sl_zigbee_af_prepayment_snapshot_schedule_payload_t *allocateSchedule(){
   uint8_t i;
   for( i=0; i<SL_ZIGBEE_AF_PLUGIN_PREPAYMENT_SNAPSHOT_STORAGE_SCHEDULE_CAPACITY; i++ ){
    if( schedules[i].snapshotScheduleId == INVALID_SNAPSHOT_SCHEDULE_ID ){
      return &schedules[i];
    }
   }
   return NULL;
   }

   void sl_zigbee_af_prepayment_snapshot_server_schedule_snapshot_cb( uint8_t srcEndpoint, uint8_t dstEndpoint,
                                                                    sl_802154_short_addr_t dest,
                                                                    uint8_t *snapshotPayload,
                                                                    uint8_t *responsePaylaod ){
   sl_zigbee_af_prepayment_snapshot_schedule_payload_t *schedule;
   uint8_t scheduleId;
   uint8_t index = 0;

   //  TODO:  Confirmation support?

   // Attempt to allocate a schedule.
   schedule = allocateSchedule();
   if( schedule != NULL ){
    // Set the schedule


   }*/

#define SNAPSHOT_PAYLOAD_TYPE_DEBT_CREDIT_STATUS 0x00

uint32_t sl_zigbee_af_prepayment_snapshot_storage_take_snapshot(uint8_t endpoint, uint32_t snapshotCause)
{
  sl_zigbee_af_prepayment_snapshot_payload_t *snapshot;
  uint32_t snapshotId = INVALID_SNAPSHOT_ID;

  snapshot = allocateSnapshot();
  if ( snapshot != NULL ) {
    if ( SnapshotIdCounter == INVALID_SNAPSHOT_ID ) {
      SnapshotIdCounter++;
    }
    snapshotId = SnapshotIdCounter++;
    snapshot->snapshotId = snapshotId;
    snapshot->snapshotCauseBitmap = snapshotCause;
    snapshot->snapshotTime = sl_zigbee_af_get_current_time();

    // TODO:  Should snapshotType be passed in as a parameter to this function?
    snapshot->snapshotType = SNAPSHOT_PAYLOAD_TYPE_DEBT_CREDIT_STATUS;

    sl_zigbee_af_app_println("Storing Snapshot, addr=%d, id=%d, bitmap=0x%04X, type=%d, time=0x%08X", (size_t)snapshot,
                             snapshot->snapshotId, snapshot->snapshotCauseBitmap, snapshot->snapshotType, snapshot->snapshotTime);

    sl_zigbee_af_read_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, ZCL_ACCUMULATED_DEBT_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER, (uint8_t *)&snapshot->accumulatedDebt,
                                sizeof(snapshot->accumulatedDebt), NULL);
    sl_zigbee_af_read_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, ZCL_DEBT_AMOUNT_1_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER, (uint8_t *)&snapshot->type1DebtRemaining,
                                sizeof(snapshot->type1DebtRemaining), NULL);
    sl_zigbee_af_read_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, ZCL_DEBT_AMOUNT_2_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER, (uint8_t *)&snapshot->type2DebtRemaining,
                                sizeof(snapshot->type2DebtRemaining), NULL);
    sl_zigbee_af_read_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, ZCL_DEBT_AMOUNT_3_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER, (uint8_t *)&snapshot->type3DebtRemaining,
                                sizeof(snapshot->type3DebtRemaining), NULL);
    sl_zigbee_af_read_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, ZCL_EMERGENCY_CREDIT_REMAINING_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER, (uint8_t *)&snapshot->emergencyCreditRemaining,
                                sizeof(snapshot->emergencyCreditRemaining), NULL);
    sl_zigbee_af_read_attribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, ZCL_CREDIT_REMAINING_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER, (uint8_t *)&snapshot->creditRemaining,
                                sizeof(snapshot->creditRemaining), NULL);
  } else {
    sl_zigbee_af_app_println("Add Snapshot Event Failed");
  }
  return snapshotId;
}

#define MAX_SNAPSHOT_PAYLOAD_LEN  24

sl_status_t sl_zigbee_af_prepayment_snapshot_storage_publish_snapshot(sl_802154_short_addr_t nodeId,
                                                                      uint8_t srcEndpoint,
                                                                      uint8_t dstEndpoint,
                                                                      uint32_t snapshotId)
{
  uint8_t snapshotPayload[MAX_SNAPSHOT_PAYLOAD_LEN];
  sl_zigbee_af_prepayment_snapshot_payload_t *snapshot = NULL;
  uint16_t index = 0;

  for (index = 0; index < SL_ZIGBEE_AF_PLUGIN_PREPAYMENT_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY; index++) {
    sl_zigbee_af_prepayment_snapshot_payload_t * cur = &snapshots[index];
    if (cur->snapshotId == snapshotId) {
      snapshot = cur;
    }
  }

  if (snapshot == NULL) {
    sl_zigbee_af_app_println("Unable to find entries with snapshotId=%d", snapshotId);
    return SL_STATUS_FAIL;
  }

  snapshotPrintInfo(snapshot);
  uint16_t payloadSize = fillSnapshotPayloadBuffer(snapshotPayload, snapshot);

  sl_zigbee_af_fill_command_prepayment_cluster_publish_prepay_snapshot(snapshot->snapshotId,
                                                                       snapshot->snapshotTime,
                                                                       1,
                                                                       0,
                                                                       1,
                                                                       snapshot->snapshotCauseBitmap,
                                                                       snapshot->snapshotType,
                                                                       snapshotPayload,
                                                                       payloadSize);

  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
  return SL_STATUS_OK;
}

uint8_t sl_zigbee_af_prepayment_server_get_snapshot_cb(sl_802154_short_addr_t nodeId,
                                                       uint8_t srcEndpoint,
                                                       uint8_t dstEndpoint,
                                                       uint32_t startTime,
                                                       uint32_t endTime,
                                                       uint8_t  snapshotOffset,
                                                       uint32_t snapshotCause)
{
  sl_zigbee_af_prepayment_snapshot_payload_t *snapshot;
  uint8_t snapshotPayload[MAX_SNAPSHOT_PAYLOAD_LEN];
  uint16_t payloadSize = 0;
  uint8_t  totalMatchesFound;

  snapshot = findSnapshot(startTime, endTime, snapshotOffset, snapshotCause, &totalMatchesFound);
  if ( snapshot != NULL ) {
    payloadSize = fillSnapshotPayloadBuffer(snapshotPayload, snapshot);
    sl_zigbee_af_app_println("Found Snapshot, addr=%d, id=%d, matches=%d, bitmap=0x%04X, type=%d, time=0x%08X",
                             (size_t)snapshot,
                             snapshot->snapshotId,
                             totalMatchesFound,
                             snapshot->snapshotCauseBitmap,
                             snapshot->snapshotType,
                             snapshot->snapshotTime);

    snapshotPrintInfo(snapshot);

    sl_zigbee_af_fill_command_prepayment_cluster_publish_prepay_snapshot(snapshot->snapshotId,
                                                                         snapshot->snapshotTime,
                                                                         totalMatchesFound,
                                                                         0,
                                                                         0,
                                                                         snapshot->snapshotCauseBitmap,
                                                                         snapshot->snapshotType,
                                                                         snapshotPayload,
                                                                         payloadSize);

    sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
    sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
  } else {
    sl_zigbee_af_app_println("Snapshot not found, st=0x%08X, end=0x%08X, cause=0x%08X", startTime, endTime, snapshotCause);
    sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_NOT_FOUND);
  }
  return payloadSize;
}

static void snapshotPrintInfo(sl_zigbee_af_prepayment_snapshot_payload_t *snapshot)
{
  sl_zigbee_af_app_println("= Prepayment Snapshot =");
  sl_zigbee_af_app_println(" id=%d", snapshot->snapshotId);
  sl_zigbee_af_app_println(" bitmap=0x%04X", snapshot->snapshotCauseBitmap);
  sl_zigbee_af_app_println(" time=0x%08X", snapshot->snapshotTime);
  sl_zigbee_af_app_println(" type=%d", snapshot->snapshotType);
  sl_zigbee_af_app_println(" accumDebt=%d", snapshot->accumulatedDebt);
  sl_zigbee_af_app_println(" type1Debt=%d", snapshot->type1DebtRemaining);
  sl_zigbee_af_app_println(" type2Debt=%d", snapshot->type2DebtRemaining);
  sl_zigbee_af_app_println(" type3Debt=%d", snapshot->type3DebtRemaining);
  sl_zigbee_af_app_println(" emergCredit=%d", snapshot->emergencyCreditRemaining);
  sl_zigbee_af_app_println(" credit=%d", snapshot->creditRemaining);
}

static uint16_t fillSnapshotPayloadBuffer(uint8_t *buffer, sl_zigbee_af_prepayment_snapshot_payload_t *snapshot)
{
  uint16_t index = 0;
  sl_zigbee_af_copy_int32u(buffer, index, snapshot->accumulatedDebt);
  index += 4;
  sl_zigbee_af_copy_int32u(buffer, index, snapshot->type1DebtRemaining);
  index += 4;
  sl_zigbee_af_copy_int32u(buffer, index, snapshot->type2DebtRemaining);
  index += 4;
  sl_zigbee_af_copy_int32u(buffer, index, snapshot->type3DebtRemaining);
  index += 4;
  sl_zigbee_af_copy_int32u(buffer, index, snapshot->emergencyCreditRemaining);
  index += 4;
  sl_zigbee_af_copy_int32u(buffer, index, snapshot->creditRemaining);
  index += 4;
  return index;
}

static sl_zigbee_af_prepayment_snapshot_payload_t *findSnapshot(uint32_t startTime,
                                                                uint32_t endTime,
                                                                uint8_t snapshotOffset,
                                                                uint32_t snapshotCause,
                                                                uint8_t *totalMatchesFound)
{
  sl_zigbee_af_prepayment_snapshot_payload_t *matchSnapshot = NULL;
  uint8_t i;
  uint8_t matchCount = 0;

  // Keep this function simple to improve testability.  Caller must do something with the snapshot.
  //snapshot = findSnapshot( startTime, endTime, snapshotOffset, snapshotCause );
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PREPAYMENT_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY; i++ ) {
    if ( (snapshots[i].snapshotTime >= startTime) && (snapshots[i].snapshotTime <= endTime) ) {
      if ( (snapshotCause == SNAPSHOT_CAUSE_ALL_SNAPSHOTS)
           || ((snapshotCause & snapshots[i].snapshotCauseBitmap) == snapshotCause) ) {
        matchCount++;
        if ( snapshotOffset == 0 ) {
          //return &snapshots[i];
          matchSnapshot = &snapshots[i];
          break;
        } else {
          snapshotOffset--;
        }
      }
    }
  }
  *totalMatchesFound = matchCount;
  return matchSnapshot;
}
