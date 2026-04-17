/***************************************************************************//**
 * @file
 * @brief Routines for the Simple Metering Server plugin, which implements the
 *        server side of the Simple Metering cluster.
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
#include "simple-metering-server.h"
#ifdef SL_CATALOG_ZIGBEE_SIMPLE_METERING_SERVER_TEST_METER_PRESENT
#include "simple-metering-test.h"
#endif

#include "zap-cluster-command-parser.h"

#define MAX_FAST_POLLING_PERIOD 15
#define PROVIDER_ID 0x44556677

#define isValidSamplingDataIndex(index) ((index) < SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_MAX_SAMPLING_SESSIONS)

// Bug: SE1P2-18
// Get Sampled Data Response does not send the maximum number of samples.
// If we want 5 samples(which we interpret as diff's between 2 consecutive sampling data points)
// we need 6 data points.
#define REAL_MAX_SAMPLES_PER_SESSION (SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_MAX_SAMPLES_PER_SESSION + 1)

// Bug:SE1P2-19
uint32_t minIssuerEventId = 0x00000000;

typedef struct {
  uint8_t endpoint;
  uint32_t issuerEventId;
  uint32_t startTime;
  uint16_t sampleId;
  uint8_t  sampleType;
  uint16_t sampleRequestInterval;
  uint16_t maxNumberOfSamples;
  uint8_t  validSamples;
  uint8_t  samples[6][REAL_MAX_SAMPLES_PER_SESSION];
} EmberAfSimpleMeteringClusterSamplingData;

typedef struct SupplyEvent{
  uint8_t srcEndpoint;
  uint8_t destEndpoint;
  uint16_t nodeId;
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t implementationDateTime;
  uint8_t proposedSupplyStatus;
} EmberAfSimpleMeteringClusterSupplyEvent;

static uint32_t fastPollEndTimeUtcTable[SL_ZIGBEE_ZCL_SIMPLE_METERING_CLUSTER_SERVER_ENDPOINT_COUNT];
static EmberAfSimpleMeteringClusterSamplingData samplingData[SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_MAX_SAMPLING_SESSIONS];
static EmberAfSimpleMeteringClusterSupplyEvent changeSupply;

void sl_zigbee_af_simple_metering_server_sampling_event_handler(sl_zigbee_af_event_t * event);
void sl_zigbee_af_simple_metering_server_supply_event_handler(sl_zigbee_af_event_t * event);
sl_zigbee_af_event_t sl_zigbee_af_simple_metering_server_sampling_event;
sl_zigbee_af_event_t sl_zigbee_af_simple_metering_server_supply_event;

#define samplingEvent (&sl_zigbee_af_simple_metering_server_sampling_event)
#define supplyEvent (&sl_zigbee_af_simple_metering_server_supply_event)

static uint8_t fastPolling = 0;

void sli_zigbee_af_toggle_fast_polling(uint8_t enableFastPolling)
{
  fastPolling = enableFastPolling;
}

static void fastPollEndTimeUtcTableInit(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_ZCL_SIMPLE_METERING_CLUSTER_SERVER_ENDPOINT_COUNT; i++) {
    fastPollEndTimeUtcTable[i] = 0x00000000;
  }
}

static void samplingDataInit(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_MAX_SAMPLING_SESSIONS; i++) {
    samplingData[i].sampleId = 0xFFFF;
    samplingData[i].validSamples = 0x00;
  }
}

static uint8_t findSamplingSession(uint16_t sampleId)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_MAX_SAMPLING_SESSIONS; i++) {
    if (samplingData[i].sampleId == sampleId) {
      return i;
    }
  }
  return 0xFF;
}

static uint8_t findSamplingSessionByEventId(uint32_t issuerEventId)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_MAX_SAMPLING_SESSIONS; i++) {
    if (samplingData[i].issuerEventId == issuerEventId) {
      return i;
    }
  }
  return 0xFF;
}

WEAK(void sli_zigbee_af_test_meter_init(uint8_t endpoint))
{
  (void) endpoint;
}

WEAK(void sli_zigbee_af_test_meter_tick(uint8_t endpoint))
{
  (void) endpoint;
}

WEAK(sl_zigbee_af_zcl_request_status_t sli_zigbee_af_test_meter_get_profiles(uint8_t intervalChannel,
                                                                             uint32_t endTime,
                                                                             uint8_t numberOfPeriods))
{
  (void) intervalChannel;
  (void) endTime;
  (void) numberOfPeriods;

  return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
}

void sl_zigbee_af_simple_metering_cluster_server_init_cb(uint8_t endpoint)
{
  sl_zigbee_af_event_init(samplingEvent,
                          sl_zigbee_af_simple_metering_server_sampling_event_handler);
  sl_zigbee_af_event_init(supplyEvent,
                          sl_zigbee_af_simple_metering_server_supply_event_handler);

  sli_zigbee_af_test_meter_init(endpoint);

  fastPollEndTimeUtcTableInit();
  samplingDataInit();
  sl_zigbee_zcl_schedule_server_tick(endpoint,
                                     ZCL_SIMPLE_METERING_CLUSTER_ID,
                                     MILLISECOND_TICKS_PER_SECOND);
}

void sl_zigbee_af_simple_metering_cluster_server_tick_cb(uint8_t endpoint)
{
  sli_zigbee_af_test_meter_tick(endpoint); //run test module
  sl_zigbee_zcl_schedule_server_tick(endpoint,
                                     ZCL_SIMPLE_METERING_CLUSTER_ID,
                                     MILLISECOND_TICKS_PER_SECOND);
}

void sl_zigbee_af_simple_metering_cluster_server_default_response_cb(uint8_t endpoint,
                                                                     uint8_t commandId,
                                                                     sl_zigbee_af_status_t status)
{
  UNUSED_VAR(endpoint);
  if (commandId == ZCL_REMOVE_MIRROR_COMMAND_ID
      && status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_simple_metering_cluster_println("Mirror remove FAILED status 0x%02X", status);
  }
}

uint16_t sl_zigbee_af_simple_metering_server_start_sampling(uint16_t requestedSampleId,
                                                            uint32_t issuerEventId,
                                                            uint32_t startSamplingTime,
                                                            uint8_t sampleType,
                                                            uint16_t sampleRequestInterval,
                                                            uint16_t maxNumberOfSamples,
                                                            uint8_t endpoint)
{
  uint32_t delay;
  uint32_t currentTime;
  static uint16_t nextSampleId = 0x0001;
  uint16_t sampleId;
  uint8_t index;

  sl_zigbee_af_simple_metering_cluster_println("StartSampling: requestedSampleId %u issuerEventId %u startSamplingTime %u sampleType %u sampleRequestInterval %u maxNumberOfSamples %u endpoint %u", requestedSampleId, issuerEventId, startSamplingTime, sampleType, sampleRequestInterval, maxNumberOfSamples, endpoint);

  // Find an unused sampling session table entry
  index = findSamplingSession(0xFFFF);
  // The following conditional tells CSTAT that index is within array bounds;
  // previously was (index == 0xFF). Simpler to make this change rather than
  // suppress the CSTAT check on multiple code lines.
  if (!isValidSamplingDataIndex(index)) {
    sl_zigbee_af_simple_metering_cluster_println("ERR: No available entries in sampling session table");
    return 0xFFFF;
  }

  // if the caller requested a specific sampleId then check to make sure it's
  // not already in use.
  if (requestedSampleId != 0xFFFF) {
    if (findSamplingSession(requestedSampleId) != 0xFF) {
      sl_zigbee_af_simple_metering_cluster_println("ERR: requested sampleId already in use");
      return 0xFFFF;
    } else {
      sampleId = requestedSampleId;
    }
  } else {
    sampleId = nextSampleId;
    while (findSamplingSession(sampleId) != 0xFF) {
      sampleId = (sampleId == 0xFFFE) ? 1 : (sampleId + 1);
    }
    nextSampleId = (sampleId == 0xFFFE) ? 1 : (sampleId + 1);
  }

  samplingData[index].sampleId = sampleId;
  samplingData[index].issuerEventId = issuerEventId;
  samplingData[index].startTime = startSamplingTime;
  samplingData[index].sampleType = sampleType;
  samplingData[index].sampleRequestInterval = sampleRequestInterval;

  //This is a hardcoded limit in EmberAfSimpleMeteringClusterSamplingData.
  if (maxNumberOfSamples > REAL_MAX_SAMPLES_PER_SESSION) {
    maxNumberOfSamples = REAL_MAX_SAMPLES_PER_SESSION;
  }
  samplingData[index].maxNumberOfSamples = maxNumberOfSamples;
  samplingData[index].endpoint = endpoint;

  currentTime = sl_zigbee_af_get_current_time();
  if (startSamplingTime < currentTime) {
    startSamplingTime = currentTime;
  }
  delay = startSamplingTime - currentTime;
  sl_zigbee_af_event_set_delay_ms(samplingEvent,
                                  (delay * 1000));
  return sampleId;
}

void sl_zigbee_af_simple_metering_server_sampling_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  uint8_t i;
  sl_zigbee_af_attribute_type_t dataType;
  sl_status_t status;
  sl_zigbee_af_event_set_inactive(samplingEvent);

  // Now let's adjust the summation
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_MAX_SAMPLING_SESSIONS; i++) {
    if (samplingData[i].startTime <= sl_zigbee_af_get_current_time()
        && samplingData[i].validSamples <= samplingData[i].maxNumberOfSamples
        && samplingData[i].sampleId < 0xFFFF) {
      status = sl_zigbee_af_read_attribute(samplingData[i].endpoint,
                                           ZCL_SIMPLE_METERING_CLUSTER_ID,
                                           ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,
                                           CLUSTER_MASK_SERVER,
                                           samplingData[i].samples[samplingData[i].validSamples],
                                           6,
                                           &dataType);
      if (status == SL_STATUS_OK) {
        sl_zigbee_af_simple_metering_cluster_println("Sample %u: 0x%02X%02X%02X%02X%02X%02X",
                                                     samplingData[i].validSamples,
                                                     samplingData[i].samples[samplingData[i].validSamples][0],
                                                     samplingData[i].samples[samplingData[i].validSamples][1],
                                                     samplingData[i].samples[samplingData[i].validSamples][2],
                                                     samplingData[i].samples[samplingData[i].validSamples][3],
                                                     samplingData[i].samples[samplingData[i].validSamples][4],
                                                     samplingData[i].samples[samplingData[i].validSamples][5]);
        samplingData[i].validSamples++;
      }
      sl_zigbee_af_simple_metering_cluster_println("Interval %u", samplingData[i].sampleRequestInterval);
      sl_zigbee_af_event_set_delay_ms(samplingEvent,
                                      samplingData[i].sampleRequestInterval * 1000);
    }
  }
}

void sl_zigbee_af_simple_metering_server_supply_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  uint32_t changeTime;
  uint32_t currentTime;
  sl_zigbee_af_event_set_inactive(supplyEvent);

  sl_zigbee_af_read_server_attribute(changeSupply.srcEndpoint,
                                     ZCL_SIMPLE_METERING_CLUSTER_ID,
                                     ZCL_PROPOSED_CHANGE_SUPPLY_IMPLEMENTATION_TIME_ATTRIBUTE_ID,
                                     (uint8_t *)&changeTime,
                                     sizeof(changeTime));
  if (changeTime == 0xFFFFFFFF) {
    // Nothing pending
    return;
  }

  currentTime = sl_zigbee_af_get_current_time();
  if (changeTime > currentTime) {
    uint32_t delay = changeTime - currentTime;
    sl_zigbee_af_event_set_delay_ms(supplyEvent,
                                    delay * 1000);
  } else {
    // Per SE specification: "The ProposedChangeImplementationTime attribute
    // indicates the time at which a proposed change to the supply is to be
    // implemented. If there is no change of supply pending, this attribute
    // will be set to 0xFFFFFFFF.
    uint32_t implementationDateTime = 0xFFFFFFFF;
    (void) sl_zigbee_af_write_attribute(changeSupply.srcEndpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_PROPOSED_CHANGE_SUPPLY_IMPLEMENTATION_TIME_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER,
                                        (uint8_t *)&implementationDateTime,
                                        ZCL_UTC_TIME_ATTRIBUTE_TYPE);

    sl_zigbee_af_fill_command_simple_metering_cluster_supply_status_response(changeSupply.providerId,
                                                                             changeSupply.issuerEventId,
                                                                             changeSupply.implementationDateTime,
                                                                             changeSupply.proposedSupplyStatus);
    sl_zigbee_af_set_command_endpoints(changeSupply.srcEndpoint, changeSupply.destEndpoint);
    sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, changeSupply.nodeId);
  }
}

void sl_zigbee_af_simple_metering_cluster_read_attributes_response_cb(sl_zigbee_af_cluster_id_t clusterId,
                                                                      uint8_t *buffer,
                                                                      uint16_t bufLen)
{
  uint16_t bufIndex = 0;

  if (clusterId != ZCL_SIMPLE_METERING_CLUSTER_ID
      || sl_zigbee_af_current_command()->direction != ZCL_FRAME_CONTROL_CLIENT_TO_SERVER) {
    return;
  }

  // Each record in the response has a two-byte attribute id and a one-byte
  // status.  If the status is SUCCESS, there will also be a one-byte type and
  // variable-length data.
  while (bufIndex + 3 <= bufLen) {
    sl_zigbee_af_status_t status;
    sl_zigbee_af_attribute_id_t attributeId = (sl_zigbee_af_attribute_id_t)sl_zigbee_af_get_int16u(buffer,
                                                                                                   bufIndex,
                                                                                                   bufLen);
    bufIndex += 2;
    status = (sl_zigbee_af_status_t)sl_zigbee_af_get_int8u(buffer, bufIndex, bufLen);
    bufIndex++;
    if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      uint16_t dataSize;
      uint8_t dataType = sl_zigbee_af_get_int8u(buffer, bufIndex, bufLen);
      bufIndex++;

      dataSize = sl_zigbee_af_attribute_value_size(dataType,
                                                   buffer + bufIndex,
                                                   bufLen - bufIndex);

      if (dataSize != 0) {
        // The Notification Attribute Set is in the range of 0x0000 - 0x00FF
        // and are all 32 bit BitMap types. Each application may decide to
        // handle the notification flags differently so we'll callback to the
        // application for each notification flags attribute.
        if (attributeId < 0x0100) {
          uint32_t bitMap = sl_zigbee_af_get_int32u(buffer, bufIndex, bufLen);
          sl_zigbee_af_simple_metering_cluster_println("Attribute value 0x%08X", bitMap);
          sl_zigbee_af_simple_metering_server_process_notification_flags_cb(attributeId, bitMap);
        }
        bufIndex += dataSize;
      } else {
        // dataSize exceeds buffer length, terminate loop
        sl_zigbee_af_simple_metering_cluster_println("ERR: attr:%04X size %d exceeds buffer size", attributeId, dataSize);
        break;
      }
    }
  }
}

//-----------------------
// ZCL commands callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_get_profile_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_get_profile_command_t cmd_data;

  if (zcl_decode_simple_metering_cluster_get_profile_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  return sli_zigbee_af_test_meter_get_profiles(cmd_data.intervalChannel, cmd_data.endTime, cmd_data.numberOfPeriods);
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_request_fast_poll_mode_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_request_fast_poll_mode_command_t cmd_data;
  uint8_t endpoint;
  uint8_t ep;
  uint8_t appliedUpdateRate;
  sl_status_t status;
  uint8_t fastPollingUpdateAttribute;

  if (zcl_decode_simple_metering_cluster_request_fast_poll_mode_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (!fastPolling) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  endpoint = sl_zigbee_af_current_endpoint();
  ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint,
                                                       ZCL_SIMPLE_METERING_CLUSTER_ID);

  appliedUpdateRate = cmd_data.fastPollUpdatePeriod;

  if (ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX) {
    sl_zigbee_af_simple_metering_cluster_println("Invalid endpoint %02X", sl_zigbee_af_current_endpoint());
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  status = sl_zigbee_af_read_server_attribute(endpoint,
                                              ZCL_SIMPLE_METERING_CLUSTER_ID,
                                              ZCL_FAST_POLL_UPDATE_PERIOD_ATTRIBUTE_ID,
                                              (uint8_t *)&fastPollingUpdateAttribute,
                                              sizeof(fastPollingUpdateAttribute));

  if (status == SL_STATUS_OK) {
    if (cmd_data.fastPollUpdatePeriod < fastPollingUpdateAttribute) {
      appliedUpdateRate = fastPollingUpdateAttribute;
      sl_zigbee_af_simple_metering_cluster_println("Applying fast Poll rate %02X ep %u", appliedUpdateRate, ep);
    }
  } else {
    sl_zigbee_af_simple_metering_cluster_println("Reading fast Poll Attribute failed. ep %u  status %02X", ep, status);
    sl_zigbee_af_fill_command_simple_metering_cluster_request_fast_poll_mode_response(0,
                                                                                      0);
    sl_zigbee_af_send_response();
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  if (sl_zigbee_af_get_current_time() > fastPollEndTimeUtcTable[ep]) {
    cmd_data.duration = cmd_data.duration > MAX_FAST_POLLING_PERIOD ? MAX_FAST_POLLING_PERIOD : cmd_data.duration;
    fastPollEndTimeUtcTable[ep] = sl_zigbee_af_get_current_time() + (cmd_data.duration * 60);
    sl_zigbee_af_simple_metering_cluster_println("Starting fast polling for %u minutes  End Time 0x%08X,current Time 0x%08X", cmd_data.duration, fastPollEndTimeUtcTable[ep], sl_zigbee_af_get_current_time());
  } else {
    sl_zigbee_af_simple_metering_cluster_println("Fast polling mode currently active. ep %u fastPollEndTimeUtcTable[%u] 0x%08X current Time 0x%08X ", ep, ep, fastPollEndTimeUtcTable[ep], sl_zigbee_af_get_current_time());
  }
  sl_zigbee_af_fill_command_simple_metering_cluster_request_fast_poll_mode_response(appliedUpdateRate,
                                                                                    fastPollEndTimeUtcTable[ep]);
  sl_zigbee_af_send_response();
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_start_sampling_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_start_sampling_command_t cmd_data;
  static uint8_t firstIssuerId = 0;

  if (zcl_decode_simple_metering_cluster_start_sampling_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  // Bug: SE1P2-19
  // Issuer Eventid should not be ignored. We accept *anything* for the first time.
  // After that, we only accept values greater than the max id received so far(which
  // should be the last valid issuerId received).
  if (firstIssuerId == 0) {
    minIssuerEventId = cmd_data.issuerEventId;
    firstIssuerId = 1;
  } else if (cmd_data.issuerEventId <= minIssuerEventId && cmd_data.startSamplingTime != 0xFFFFFFFF) {
    sl_zigbee_af_simple_metering_cluster_println("Rejecting StartSamplingCallback issuerEventId %u minIssuerEventId %u", cmd_data.issuerEventId, minIssuerEventId);
    return SL_ZIGBEE_ZCL_STATUS_FAILURE;
  } else {
    minIssuerEventId = cmd_data.issuerEventId;
  }

  // Bug: SE1P2-17
  // StartSampling Event ID field is ignored by the metering server
  //Special time reserved to cancel a startSampling Request.
  if (cmd_data.startSamplingTime == 0xFFFFFFFF) {
    uint8_t eventIndex = findSamplingSessionByEventId(cmd_data.issuerEventId);
    //Event not found.
    // The following conditional tells CSTAT that index is within array bounds;
    // previously was (index == 0xFF). Simpler to make this change rather than
    // suppress the CSTAT check on multiple code lines.
    if (!isValidSamplingDataIndex(eventIndex)) {
      return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
    } else {
      samplingData[eventIndex].sampleId = 0xFFFF;
      samplingData[eventIndex].validSamples = 0x00;
      return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    }
  }

  uint16_t sampleId = sl_zigbee_af_simple_metering_server_start_sampling(0xFFFF,
                                                                         cmd_data.issuerEventId,
                                                                         cmd_data.startSamplingTime,
                                                                         cmd_data.sampleType,
                                                                         cmd_data.sampleRequestInterval,
                                                                         cmd_data.maxNumberOfSamples,
                                                                         sl_zigbee_af_current_endpoint());
  sl_zigbee_af_fill_command_simple_metering_cluster_start_sampling_response(sampleId);
  sl_zigbee_af_send_response();
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_get_sampled_data_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_get_sampled_data_command_t cmd_data;
  uint8_t i;

  if (zcl_decode_simple_metering_cluster_get_sampled_data_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_simple_metering_cluster_println("sampleId %u earliestSampleTime %u sampleType %u numberOfSamples %u",
                                               cmd_data.sampleId, cmd_data.earliestSampleTime, cmd_data.sampleType, cmd_data.numberOfSamples);
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_MAX_SAMPLING_SESSIONS; i++) {
    uint8_t j;
    uint32_t diff;
    if (samplingData[i].sampleId == cmd_data.sampleId
        && samplingData[i].sampleId < 0xFFFF
        && samplingData[i].startTime <= cmd_data.earliestSampleTime
        && samplingData[i].sampleType == cmd_data.sampleType) {
      //If we have only one sample data point or lesser, we don't have enough information
      //for even one sample.
      if (samplingData[i].validSamples <= 1) {
        goto kickout;
      }

      //Bug SE1P2-19: Use the max field from the client.

      if (cmd_data.numberOfSamples > samplingData[i].validSamples - 1) {
        cmd_data.numberOfSamples = samplingData[i].validSamples - 1;
      }

      sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                         | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_GET_SAMPLED_DATA_RESPONSE_COMMAND_ID,
                                        "vwuvv",
                                        samplingData[i].sampleId,
                                        samplingData[i].startTime,
                                        samplingData[i].sampleType,
                                        samplingData[i].sampleRequestInterval,
                                        cmd_data.numberOfSamples);

      sl_zigbee_af_simple_metering_cluster_println("numberOfSamples 0x%04X", cmd_data.numberOfSamples);

      for (j = 0; j < cmd_data.numberOfSamples; j++) {
        uint32_t b = (samplingData[i].samples[j + 1][3] << 24) | (samplingData[i].samples[j + 1][2] << 16) | (samplingData[i].samples[j + 1][1] << 8) | (samplingData[i].samples[j + 1][0] << 0);
        uint32_t a = (samplingData[i].samples[j][3] << 24) | (samplingData[i].samples[j][2] << 16) | (samplingData[i].samples[j][1] << 8) | (samplingData[i].samples[j][0] << 0);

        diff = b - a;
        (void) sl_zigbee_af_put_int24u_in_resp(diff);
        sl_zigbee_af_simple_metering_cluster_println("index %u numberOfSamples %u diff %u", j, cmd_data.numberOfSamples, diff);
      }
      sl_zigbee_af_send_response();
      return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
    }
  }

  kickout:
  return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_mirror_report_attribute_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_mirror_report_attribute_response_command_t cmd_data;

  if (zcl_decode_simple_metering_cluster_mirror_report_attribute_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (cmd_data.notificationScheme == 0x01) {
    uint32_t functionalFlags = sl_zigbee_af_get_int32u(cmd_data.notificationFlags, 0, 4);
    sl_zigbee_af_simple_metering_server_process_notification_flags_cb(ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID, functionalFlags);
  } else if (cmd_data.notificationScheme == 0x02) {
    uint32_t functionalFlags = sl_zigbee_af_get_int32u(cmd_data.notificationFlags, 0, 20);
    uint32_t notificationFlags2 = sl_zigbee_af_get_int32u(cmd_data.notificationFlags, 4, 20);
    uint32_t notificationFlags3 = sl_zigbee_af_get_int32u(cmd_data.notificationFlags, 8, 20);
    uint32_t notificationFlags4 = sl_zigbee_af_get_int32u(cmd_data.notificationFlags, 12, 20);
    uint32_t notificationFlags5 = sl_zigbee_af_get_int32u(cmd_data.notificationFlags, 16, 20);
    sl_zigbee_af_simple_metering_cluster_println("functionalFlags 0x%08X notificationFlags2-5 0x%08X 0x%08X 0x%08X 0x%08X",
                                                 functionalFlags, notificationFlags2, notificationFlags3,
                                                 notificationFlags4, notificationFlags5);
    sl_zigbee_af_simple_metering_server_process_notification_flags_cb(ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID, functionalFlags);
    sl_zigbee_af_simple_metering_server_process_notification_flags_cb(ZCL_NOTIFICATION_FLAGS_2_ATTRIBUTE_ID, notificationFlags2);
    sl_zigbee_af_simple_metering_server_process_notification_flags_cb(ZCL_NOTIFICATION_FLAGS_3_ATTRIBUTE_ID, notificationFlags3);
    sl_zigbee_af_simple_metering_server_process_notification_flags_cb(ZCL_NOTIFICATION_FLAGS_4_ATTRIBUTE_ID, notificationFlags4);
    sl_zigbee_af_simple_metering_server_process_notification_flags_cb(ZCL_NOTIFICATION_FLAGS_5_ATTRIBUTE_ID, notificationFlags5);
  } else {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_reset_load_limit_counter_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  uint8_t counter = 0;

#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
  sl_zcl_simple_metering_cluster_reset_load_limit_counter_command_t cmd_data;
  if (zcl_decode_simple_metering_cluster_reset_load_limit_counter_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_simple_metering_cluster_println("Reset Load Counter providerId %u issuerEventId %u", cmd_data.providerId, cmd_data.issuerEventId);
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

  (void) sl_zigbee_af_write_attribute(sl_zigbee_af_current_endpoint(),
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_LOAD_LIMIT_COUNTER_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      &counter,
                                      ZCL_INT8U_ATTRIBUTE_TYPE);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_change_supply_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_change_supply_command_t cmd_data;
  uint32_t delay;

  if (zcl_decode_simple_metering_cluster_change_supply_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_simple_metering_cluster_println("Change Supply Callback providerId %u issuerEventId %u implementationDateTime %u supplyStatus %u",
                                               cmd_data.providerId, cmd_data.issuerEventId, cmd_data.implementationDateTime, cmd_data.supplyControlBits);
  // TODO: fix this hard-coded check
  if (cmd_data.providerId != PROVIDER_ID) {
    return SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
  }
  if (cmd_data.implementationDateTime < sl_zigbee_af_get_current_time() && cmd_data.implementationDateTime != 0) {
    sl_zigbee_af_simple_metering_cluster_println("implementationDateTime %u", cmd_data.implementationDateTime);
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }
  if (cmd_data.implementationDateTime == 0xFFFFFFFF) {
    sl_zigbee_af_simple_metering_cluster_println("Canceling change supply");
    // Per SE specification: "The ProposedChangeImplementationTime attribute
    // indicates the time at which a proposed change to the supply is to be
    // implemented. If there is no change of supply pending, this attribute
    // will be set to 0xFFFFFFFF.
    (void) sl_zigbee_af_write_attribute(sl_zigbee_af_current_endpoint(),
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_PROPOSED_CHANGE_SUPPLY_IMPLEMENTATION_TIME_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER,
                                        (uint8_t *)&cmd_data.implementationDateTime,
                                        ZCL_UTC_TIME_ATTRIBUTE_TYPE);
    sl_zigbee_af_event_set_inactive(supplyEvent);
  }
  if (cmd_data.implementationDateTime == 0x00000000 ) {
    if ((cmd_data.supplyControlBits & 0x01) == 0x01 || cmd_data.proposedSupplyStatus == 2) {
      sl_zigbee_af_fill_command_simple_metering_cluster_supply_status_response(cmd_data.providerId,
                                                                               cmd_data.issuerEventId,
                                                                               cmd_data.implementationDateTime,
                                                                               cmd_data.proposedSupplyStatus);
      sl_zigbee_af_send_response();
      return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
    }
  } else {
    (void) sl_zigbee_af_write_attribute(sl_zigbee_af_current_endpoint(),
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_PROPOSED_CHANGE_SUPPLY_IMPLEMENTATION_TIME_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER,
                                        (uint8_t *)&cmd_data.implementationDateTime,
                                        ZCL_UTC_TIME_ATTRIBUTE_TYPE);

    (void) sl_zigbee_af_write_attribute(sl_zigbee_af_current_endpoint(),
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_PROPOSED_CHANGE_SUPPLY_STATUS_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER,
                                        &cmd_data.proposedSupplyStatus,
                                        ZCL_ENUM8_ATTRIBUTE_TYPE);
    delay = cmd_data.implementationDateTime - sl_zigbee_af_get_current_time();
    changeSupply.srcEndpoint = cmd->apsFrame->destinationEndpoint;
    changeSupply.destEndpoint = cmd->apsFrame->sourceEndpoint;
    changeSupply.nodeId = cmd->source;
    changeSupply.providerId = cmd_data.providerId;
    changeSupply.issuerEventId = cmd_data.issuerEventId;
    changeSupply.implementationDateTime = cmd_data.implementationDateTime;
    changeSupply.proposedSupplyStatus = cmd_data.proposedSupplyStatus;
    if ((cmd_data.supplyControlBits & 0x01) == 0x01) {
      sl_zigbee_af_event_set_delay_ms(supplyEvent,
                                      delay * 1000);
    }
  }

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_local_change_supply_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_local_change_supply_command_t cmd_data;

  if (zcl_decode_simple_metering_cluster_local_change_supply_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (cmd_data.proposedSupplyStatus < 1 || cmd_data.proposedSupplyStatus > 2) {
    return SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
  }
  sl_zigbee_af_simple_metering_cluster_println("Setting localSupply Status %u", cmd_data.proposedSupplyStatus);
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_set_supply_status_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_set_supply_status_command_t cmd_data;

  if (zcl_decode_simple_metering_cluster_set_supply_status_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  (void) sl_zigbee_af_write_attribute(sl_zigbee_af_current_endpoint(),
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_SUPPLY_TAMPER_STATE_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      &cmd_data.supplyTamperState,
                                      ZCL_ENUM8_ATTRIBUTE_TYPE);

  (void) sl_zigbee_af_write_attribute(sl_zigbee_af_current_endpoint(),
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_SUPPLY_DEPLETION_STATE_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      &cmd_data.supplyDepletionState,
                                      ZCL_ENUM8_ATTRIBUTE_TYPE);

  (void) sl_zigbee_af_write_attribute(sl_zigbee_af_current_endpoint(),
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_SUPPLY_UNCONTROLLED_FLOW_STATE_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      &cmd_data.supplyUncontrolledFlowState,
                                      ZCL_ENUM8_ATTRIBUTE_TYPE);

  (void) sl_zigbee_af_write_attribute(sl_zigbee_af_current_endpoint(),
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_LOAD_LIMIT_SUPPLY_STATE_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      &cmd_data.loadLimitSupplyState,
                                      ZCL_ENUM8_ATTRIBUTE_TYPE);
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_set_uncontrolled_flow_threshold_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

uint32_t sl_zigbee_af_simple_metering_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                                   sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_GET_PROFILE_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_get_profile_cb(cmd);
        break;
      }
      case ZCL_REQUEST_FAST_POLL_MODE_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_request_fast_poll_mode_cb(cmd);
        break;
      }
      case ZCL_START_SAMPLING_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_start_sampling_cb(cmd);
        break;
      }
      case ZCL_GET_SAMPLED_DATA_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_get_sampled_data_cb(cmd);
        break;
      }
      case ZCL_MIRROR_REPORT_ATTRIBUTE_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_mirror_report_attribute_response_cb(cmd);
        break;
      }
      case ZCL_RESET_LOAD_LIMIT_COUNTER_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_reset_load_limit_counter_cb(cmd);
        break;
      }
      case ZCL_CHANGE_SUPPLY_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_change_supply_cb(cmd);
        break;
      }
      case ZCL_LOCAL_CHANGE_SUPPLY_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_local_change_supply_cb(cmd);
        break;
      }
      case ZCL_SET_SUPPLY_STATUS_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_set_supply_status_cb(cmd);
        break;
      }
      case ZCL_SET_UNCONTROLLED_FLOW_THRESHOLD_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_set_uncontrolled_flow_threshold_cb(cmd);
        break;
      }
    }
  }

  return status;
}
