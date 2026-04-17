/***************************************************************************//**
 * @file
 * @brief Routines for the Reporting plugin, which sends asynchronous reports
 *        when a ZCL attribute's value has changed.
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
#include "app/framework/util/common.h"
#include "app/framework/util/attribute-storage.h"
#include "app/framework/plugin/reporting/reporting.h"
#include "stack/include/sl_zigbee_token.h"

#if !defined(EZSP_HOST) && !defined(ENABLE_EXPANDED_TABLE) && !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
#include "sl_token_manager_api.h"
#endif

#ifdef ZCL_ATTRIBUTE_LARGEST
// The ZCL Long String type data type takes first 2 bytes as the length
// And the sl_zigbee_af_read_attribute takes an uint8_t as the output length.
// So, it is important to ensure the largest value is less than the 253.
#define READ_DATA_SIZE           \
  ((ZCL_ATTRIBUTE_LARGEST) > 253 \
   ? 253                         \
   : (ZCL_ATTRIBUTE_LARGEST))
#else
#define READ_DATA_SIZE 8 // max size if attributes aren't present
#endif

static uint16_t reportTableActiveLength = 0;

static void conditionallySendReport(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId);
static void scheduleTick(void);
static void removeConfiguration(uint16_t index);
static void removeConfigurationAndScheduleTick(uint16_t index);
static sl_zigbee_af_status_t configureReceivedAttribute(const sl_zigbee_af_cluster_command_t *cmd,
                                                        sl_zigbee_af_attribute_id_t attributeId,
                                                        uint8_t mask,
                                                        uint16_t timeout);
static void putReportableChangeInResp(const sl_zigbee_af_plugin_reporting_entry_t *entry,
                                      sl_zigbee_af_attribute_type_t dataType);
static void retrySendReport(sl_zigbee_outgoing_message_type_t type,
                            uint16_t indexOrDestination,
                            sl_zigbee_aps_frame_t *apsFrame,
                            uint16_t msgLen,
                            uint8_t *message,
                            sl_status_t status);
static uint32_t computeStringHash(uint8_t *data, uint8_t length);
static sl_zigbee_af_status_t readAttributeAndGetLastValue(const sl_zigbee_af_plugin_reporting_entry_t* const entry,
                                                          uint8_t entryIndex,
                                                          sl_zigbee_af_attribute_type_t* pDataType,
                                                          uint16_t* pDataSize,
                                                          uint8_t* pReadData,
                                                          uint16_t readDataSize,
                                                          bool reportChange);
static void markReportTableChange(uint8_t *dataRef,
                                  uint8_t dataSize,
                                  sl_zigbee_af_attribute_type_t dataType,
                                  const sl_zigbee_af_plugin_reporting_entry_t* const entry,
                                  uint8_t entryIndex);
sl_zigbee_af_event_t sl_zigbee_af_reporting_tick_event;
#define tickEvent (&sl_zigbee_af_reporting_tick_event)
void sl_zigbee_af_reporting_tick_event_handler(sl_zigbee_af_event_t * event);
#define slxu_zigbee_event_init(x, y) sl_zigbee_af_event_init(x, y)

sli_zigbee_af_report_volatile_data_type sli_zigbee_af_report_volatile_data[REPORT_TABLE_SIZE];

static void retrySendReport(sl_zigbee_outgoing_message_type_t type,
                            uint16_t indexOrDestination,
                            sl_zigbee_aps_frame_t *apsFrame,
                            uint16_t msgLen,
                            uint8_t *message,
                            sl_status_t status)
{
  // Retry once, and do so by unicasting without a pointer to this callback
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_send_unicast(type, indexOrDestination, apsFrame, msgLen, message);
  }
}

// Implementation based on public domain Fowler/Noll/Vo FNV-1a hash function:
// http://isthe.com/chongo/tech/comp/fnv/
// https://tools.ietf.org/html/draft-eastlake-fnv-14
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
//
// Used to save and compare hashes of CHAR and OCTET string values in order to detect
// reportable changes. The strings themselves are longer than the storage size.
#define FNV1_OFFSET_BASIS (2166136261)
#define FNV1_PRIME (16777619)
static uint32_t computeStringHash(uint8_t *data, uint8_t length)
{
  // FNV-1a, 32-bit hash
  uint32_t hash = FNV1_OFFSET_BASIS;
  for (int i = 0; i < length; ++i) {
    hash ^= data[i];
    hash *= FNV1_PRIME; // Or, hash += (hash<<1) + (hash<<4) + (hash<<7) + (hash<<8) + (hash<<24);
  }
  return hash;
}

#define ifValidIndex(expr) do {      \
    if (index < REPORT_TABLE_SIZE) { \
      expr;                          \
    }                                \
} while (0)

#ifdef EZSP_HOST
static sl_zigbee_af_plugin_reporting_entry_t table[REPORT_TABLE_SIZE];
void sli_zigbee_af_reporting_get_entry(uint16_t index, sl_zigbee_af_plugin_reporting_entry_t *result)
{
  ifValidIndex(memmove(result, &table[index], sizeof(sl_zigbee_af_plugin_reporting_entry_t)));
}
void sli_zigbee_af_reporting_set_entry(uint16_t index, sl_zigbee_af_plugin_reporting_entry_t *value)
{
  ifValidIndex(memmove(&table[index], value, sizeof(sl_zigbee_af_plugin_reporting_entry_t)));
}
#elif defined(ENABLE_EXPANDED_TABLE) // SOC and expanded table is enabled
#define reportingTableKey(index) (NVM3KEY_REPORTING_TABLE_EXPANDED + (index))
void sli_zigbee_af_reporting_get_entry(uint16_t index, sl_zigbee_af_plugin_reporting_entry_t *result)
{
  ifValidIndex(nvm3_readData(nvm3_defaultHandle, reportingTableKey(index), result, sizeof(sl_zigbee_af_plugin_reporting_entry_t)));
}
void sli_zigbee_af_reporting_set_entry(uint16_t index, sl_zigbee_af_plugin_reporting_entry_t *value)
{
  ifValidIndex(nvm3_writeData(nvm3_defaultHandle, reportingTableKey(index), value, sizeof(sl_zigbee_af_plugin_reporting_entry_t)));
}
#else // SoC and expanded table is disabled

void sli_zigbee_af_reporting_get_entry(uint16_t index, sl_zigbee_af_plugin_reporting_entry_t *result)
{
  ifValidIndex((void)sl_token_manager_get_data(COMMON_TOKEN_REPORT_TABLE + index, (void *)result, sizeof(sl_zigbee_af_plugin_reporting_entry_t)));
}
void sli_zigbee_af_reporting_set_entry(uint16_t index, sl_zigbee_af_plugin_reporting_entry_t *value)
{
  ifValidIndex((void)sl_token_manager_set_data(COMMON_TOKEN_REPORT_TABLE + index, (void *)value, sizeof(sl_zigbee_af_plugin_reporting_entry_t)));
}
#endif

// TODO: renamed for naming consistency purposes
void sli_zigbee_af_reporting_stack_status_callback(sl_status_t status)
{
  if (status == SL_STATUS_NETWORK_UP) {
    // Load default reporting configurations
    sl_zigbee_af_reporting_load_reporting_config_defaults();

    scheduleTick();
  }
}

sl_status_t sl_zigbee_af_reporting_token_init(void)
{
  sl_zigbee_af_plugin_reporting_entry_t reporting_entry_default = TOKEN_REPORT_TABLE_DEFAULT;
  return sl_zigbee_initialize_index_token(COMMON_TOKEN_REPORT_TABLE, &reporting_entry_default, sizeof(sl_zigbee_af_plugin_reporting_entry_t), REPORT_TABLE_SIZE);
}

void sl_zigbee_af_reporting_init_cb(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
      slxu_zigbee_event_init(tickEvent,
                             sl_zigbee_af_reporting_tick_event_handler);
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_DONE:
    {
      // Deferring this to the DONE phase as it relies on endpoint
      // enable/disable state having first been initialized. That
      // state is referenced directly here, and in tick scheduler.
      //
      // On device initialization, any attributes that have been set up to report
      // should generate an attribute report.
      uint16_t i;
      for (i = 0; i < REPORT_TABLE_SIZE; i++) {
        sl_zigbee_af_plugin_reporting_entry_t entry;
        entry.endpoint = SL_ZIGBEE_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID;
        sli_zigbee_af_reporting_get_entry(i, &entry);
        if (entry.endpoint == SL_ZIGBEE_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID) {
          // we structure the table so that all "active" entries fall within
          // a single range.  When we encounter an unused entry, assume
          // we've reached the end of active entries and break the loop
          break;
        }
        if (sl_zigbee_af_endpoint_is_enabled(entry.endpoint)
            && entry.direction == SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED) {
          sli_zigbee_af_report_volatile_data[i].reportableChange = true;
        }
      }
      reportTableActiveLength = i;
      scheduleTick();
      break;
    }

    default:
      // MISRA requires default case.
      break;
  }
  assert(SL_STATUS_OK == sl_zigbee_af_reporting_token_init());
}

uint16_t sli_zigbee_af_reporting_num_entries(void)
{
  return reportTableActiveLength;
}

void sl_zigbee_af_reporting_tick_event_handler(sl_zigbee_af_event_t * event)
{
  (void)event;
  sl_zigbee_aps_frame_t *apsFrame = NULL;
  sl_zigbee_af_status_t status;
  sl_zigbee_af_attribute_type_t dataType;
  uint16_t manufacturerCode = 0;
  uint8_t readData[READ_DATA_SIZE];
  uint16_t dataSize;
  bool clientToServer = false;
  sl_zigbee_binding_table_entry_t bindingEntry;
  uint8_t reportSize = 0, currentPayloadMaxLength = 0, smallestPayloadMaxLength = 0;
  uint16_t i;

  for (i = 0; i < reportTableActiveLength; i++) {
    sl_zigbee_af_plugin_reporting_entry_t entry;
    uint32_t elapsedMs;
    sli_zigbee_af_reporting_get_entry(i, &entry);
    // We will only send reports for active reported attributes and only if a
    // reportable change has occurred and the minimum interval has elapsed or
    // if the maximum interval is set and has elapsed.
    elapsedMs = elapsedTimeInt32u(sli_zigbee_af_report_volatile_data[i].lastReportTimeMs,
                                  halCommonGetInt32uMillisecondTick());
    if (!sl_zigbee_af_endpoint_is_enabled(entry.endpoint)
        || entry.direction != SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED
        || (elapsedMs
            < entry.data.reported.minInterval * MILLISECOND_TICKS_PER_SECOND)
        || (!sli_zigbee_af_report_volatile_data[i].reportableChange
            && (entry.data.reported.maxInterval == 0
                || (elapsedMs
                    < (entry.data.reported.maxInterval
                       * MILLISECOND_TICKS_PER_SECOND))))) {
      continue;
    }
    status = readAttributeAndGetLastValue(&entry, i, &dataType, &dataSize, readData, READ_DATA_SIZE, false);
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      goto skipAttribute;
    }

    reportSize = sizeof(entry.attributeId) + sizeof(dataType) + dataSize;

    // If we have already started a report for a different attribute or
    // destination, or if the current entry is too big for current report, send it and create a new one.
    if (apsFrame != NULL
        && (!(entry.endpoint == apsFrame->sourceEndpoint
              && entry.clusterId == apsFrame->clusterId
              && sl_zigbee_af_cluster_is_client(&entry) == clientToServer
              && entry.manufacturerCode == manufacturerCode)
            || (appResponseLength + reportSize > smallestPayloadMaxLength))) {
      if (appResponseLength + reportSize > smallestPayloadMaxLength) {
        sl_zigbee_af_reporting_println("Reporting Entry Full - creating new report");
      }
      conditionallySendReport(apsFrame->sourceEndpoint, apsFrame->clusterId);
      apsFrame = NULL;
    }

    // If we haven't made the message header, make it.
    if (apsFrame == NULL) {
      apsFrame = sl_zigbee_af_get_command_aps_frame();
      clientToServer = sl_zigbee_af_cluster_is_client(&entry);
      // The manufacturer-specfic version of the fill API only creates a
      // manufacturer-specfic command if the manufacturer code is set.  For
      // non-manufacturer-specfic reports, the manufacturer code is unset, so
      // we can get away with using this API for both cases.
      sl_zigbee_af_fill_external_manufacturer_specific_buffer((clientToServer
                                                               ? (ZCL_GLOBAL_COMMAND
                                                                  | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                                                  | SL_ZIGBEE_AF_DEFAULT_RESPONSE_POLICY_REQUESTS)
                                                               : (ZCL_GLOBAL_COMMAND
                                                                  | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                                                                  | SL_ZIGBEE_AF_DEFAULT_RESPONSE_POLICY_REQUESTS)),
                                                              entry.clusterId,
                                                              entry.manufacturerCode,
                                                              ZCL_REPORT_ATTRIBUTES_COMMAND_ID,
                                                              "");
      apsFrame->sourceEndpoint = entry.endpoint;
      apsFrame->options = SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS;
      manufacturerCode = entry.manufacturerCode;

      // EMAPPFWKV2-1327: Reporting plugin does not account for reporting too many attributes
      //                  in the same ZCL:ReportAttributes message

      // find smallest maximum payload that the destination can receive for this cluster and source endpoint
      uint8_t index;
      smallestPayloadMaxLength = MAX_INT8U_VALUE;
      for (index = 0; index < SL_ZIGBEE_BINDING_TABLE_SIZE; index++) {
        status = (sl_zigbee_af_status_t)sl_zigbee_get_binding(index, &bindingEntry);
        if (status == (sl_zigbee_af_status_t)SL_STATUS_OK && bindingEntry.local == entry.endpoint && bindingEntry.clusterId == entry.clusterId) {
          currentPayloadMaxLength = sl_zigbee_af_maximum_aps_payload_length(bindingEntry.type, bindingEntry.networkIndex, apsFrame);
          if (currentPayloadMaxLength < smallestPayloadMaxLength) {
            smallestPayloadMaxLength = currentPayloadMaxLength;
          }
        }
      }
    }

    // Payload is [attribute id:2] [type:1] [data:N].
    (void) sl_zigbee_af_put_int16u_in_resp(entry.attributeId);
    (void) sl_zigbee_af_put_int8u_in_resp(dataType);

#if (BIGENDIAN_CPU)
    if (isThisDataTypeSentLittleEndianOTA(dataType)) {
      uint8_t i;
      for (i = 0; i < dataSize; i++) {
        (void) sl_zigbee_af_put_int8u_in_resp(readData[dataSize - i - 1]);
      }
    } else {
      sl_zigbee_af_put_block_in_resp(readData, dataSize);
    }
#else
    sl_zigbee_af_put_block_in_resp(readData, dataSize);
#endif

    // Normally will arrive here at the conclusion of attribute processing.
    // Update the state used to decide if an attribute value is ready to
    // be reported. The shortest and longest intervals between reports for
    // this attribute will be governed by the minInterval and maxInterval
    // settings in the attribute's report configuration.
    //
    // Alternatively...
    //
    skipAttribute:
    //
    // ...may arrive here via goto label if attribute processing detected
    // a condition that prevents the attribute value from being reported.
    // In that case, this state still must be updated; otherwise the tick
    // scheduler, executed at the end of this handler, will see the attribute
    // as still being ready to be reported, and will schedule the handler
    // to execute again IMMEDIATELY. If the problematic attribute condition
    // persists, the handler will effectively try to execute continuously.
    sli_zigbee_af_report_volatile_data[i].reportableChange = false;
    sli_zigbee_af_report_volatile_data[i].lastReportTimeMs = halCommonGetInt32uMillisecondTick();
  }

  if (apsFrame != NULL) {
    conditionallySendReport(apsFrame->sourceEndpoint, apsFrame->clusterId);
  }
  scheduleTick();
}

static void conditionallySendReport(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId)
{
  sl_status_t status;
  if ((sl_zigbee_af_is_device_enabled(endpoint)
       || clusterId == ZCL_IDENTIFY_CLUSTER_ID)
      // No transmissions from the s2s shades device is desired. The S2S
      // Shades sets up bindings at startup. The device then tries to send
      // messages over those bindings - something we don't want. The below code
      // stops all reports if the device is a s2s device
      && ((sl_zigbee_af_network_state() != SL_ZIGBEE_JOINED_NETWORK_S2S_INITIATOR)
          && (sl_zigbee_af_network_state() != SL_ZIGBEE_JOINED_NETWORK_S2S_TARGET))) {
    status = sl_zigbee_af_send_command_unicast_to_bindings_with_cb((sl_zigbee_af_message_sent_function_t)(&retrySendReport));

    // If the callback table is full, attempt to send the message with no
    // callback.  Note that this could lead to a message failing to transmit
    // with no notification to the user for any number of reasons (ex: hitting
    // the message queue limit), but is better than not sending the message at
    // all because the system hits its callback queue limit.
    if (status == SL_STATUS_FULL) {
      sl_zigbee_af_send_command_unicast_to_bindings();
    }

#ifdef ENABLE_GROUP_BOUND_REPORTS
    sl_zigbee_af_send_command_multicast_to_bindings();
#endif // ENABLE_GROUP_BOUND_REPORTS
  }
}

bool sl_zigbee_af_configure_reporting_command_cb(const sl_zigbee_af_cluster_command_t *cmd)
{
  sl_status_t sendStatus;
  uint16_t bufIndex = cmd->payloadStartIndex;
  uint8_t frameControl, mask;
  bool failures = false;

  sl_zigbee_af_reporting_print("%s: ", "CFG_RPT");
  sl_zigbee_af_reporting_debug_exec(sl_zigbee_af_decode_and_print_cluster_with_mfg_code(cmd->apsFrame->clusterId, cmd->mfgCode));
  sl_zigbee_af_reporting_println("");
  sl_zigbee_af_reporting_flush();

  if (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER) {
    frameControl = (ZCL_GLOBAL_COMMAND
                    | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                    | SL_ZIGBEE_AF_DEFAULT_RESPONSE_POLICY_REQUESTS);
    mask = CLUSTER_MASK_SERVER;
  } else {
    frameControl = (ZCL_GLOBAL_COMMAND
                    | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                    | SL_ZIGBEE_AF_DEFAULT_RESPONSE_POLICY_REQUESTS);
    mask = CLUSTER_MASK_CLIENT;
  }

  // The manufacturer-specfic version of the fill API only creates a
  // manufacturer-specfic command if the manufacturer code is set.  For non-
  // manufacturer-specfic reports, the manufacturer code is unset, so we can
  // get away with using this API for both cases.
  sl_zigbee_af_fill_external_manufacturer_specific_buffer(frameControl,
                                                          cmd->apsFrame->clusterId,
                                                          cmd->mfgCode,
                                                          ZCL_CONFIGURE_REPORTING_RESPONSE_COMMAND_ID,
                                                          "");

  // Each record in the command has at least a one-byte direction and a two-
  // byte attribute id.  Additional fields are present depending on the value
  // of the direction field.
  while (bufIndex + 3 < cmd->bufLen) {
    sl_zigbee_af_attribute_id_t attributeId;
    sl_zigbee_af_reporting_direction_t direction;
    sl_zigbee_af_status_t status;

    direction = (sl_zigbee_af_reporting_direction_t)sl_zigbee_af_get_int8u(cmd->buffer,
                                                                           bufIndex,
                                                                           cmd->bufLen);
    bufIndex++;
    attributeId = (sl_zigbee_af_attribute_id_t)sl_zigbee_af_get_int16u(cmd->buffer,
                                                                       bufIndex,
                                                                       cmd->bufLen);
    bufIndex += 2;

    sl_zigbee_af_reporting_println(" - direction:%02X, attr:%04X", direction, attributeId);

    switch (direction) {
      case SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED:
      {
        sl_zigbee_af_attribute_metadata_t* metadata;
        sl_zigbee_af_attribute_type_t dataType;
        uint16_t minInterval, maxInterval;
        uint32_t reportableChange = 0;

        dataType = (sl_zigbee_af_attribute_type_t)sl_zigbee_af_get_int8u(cmd->buffer,
                                                                         bufIndex,
                                                                         cmd->bufLen);
        bufIndex++;
        minInterval = sl_zigbee_af_get_int16u(cmd->buffer, bufIndex, cmd->bufLen);
        bufIndex += 2;
        maxInterval = sl_zigbee_af_get_int16u(cmd->buffer, bufIndex, cmd->bufLen);
        bufIndex += 2;

        sl_zigbee_af_reporting_println("   type:%02X, min:%04X, max:%04X",
                                       dataType,
                                       minInterval,
                                       maxInterval);
        sl_zigbee_af_reporting_flush();

        if (sl_zigbee_af_get_attribute_analog_or_discrete_type(dataType)
            == SL_ZIGBEE_AF_DATA_TYPE_ANALOG) {
          uint8_t dataSize = sl_zigbee_af_get_data_size(dataType);
          reportableChange = sl_zigbee_af_get_int(cmd->buffer,
                                                  bufIndex,
                                                  cmd->bufLen,
                                                  dataSize);

          sl_zigbee_af_reporting_print("   change:");
          sl_zigbee_af_reporting_print_buffer(cmd->buffer + bufIndex, dataSize, false);
          sl_zigbee_af_reporting_println("");

          bufIndex += dataSize;
        }

        // sl_zigbee_af_reporting_configure_reported_attribute handles non-
        // existent attributes, but does not verify the attribute data type, so
        // we need to check it here.
        metadata = sl_zigbee_af_locate_attribute_metadata(cmd->apsFrame->destinationEndpoint,
                                                          cmd->apsFrame->clusterId,
                                                          attributeId,
                                                          mask,
                                                          cmd->mfgCode);
        if (metadata != NULL && metadata->attributeType != dataType) {
          status = SL_ZIGBEE_ZCL_STATUS_INVALID_DATA_TYPE;
        } else if (ZCL_ARRAY_ATTRIBUTE_TYPE == dataType
                   || ZCL_STRUCT_ATTRIBUTE_TYPE == dataType
                   || ZCL_SET_ATTRIBUTE_TYPE == dataType
                   || ZCL_BAG_ATTRIBUTE_TYPE == dataType) {
          // The attribute type is array, structure, set or bag
          status = SL_ZIGBEE_ZCL_STATUS_UNREPORTABLE_ATTRIBUTE;
        } else {
          // Add a reporting entry for a reported attribute.  The reports will
          // be sent from us to the source of the Configure Reporting command.
          sl_zigbee_af_plugin_reporting_entry_t newEntry;
          newEntry.direction = SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED;
          newEntry.endpoint = cmd->apsFrame->destinationEndpoint;
          newEntry.clusterId = cmd->apsFrame->clusterId;
          newEntry.attributeId = attributeId;
          newEntry.mask = mask;
          newEntry.manufacturerCode = cmd->mfgCode;
          newEntry.data.reported.minInterval = minInterval;
          newEntry.data.reported.maxInterval = maxInterval;
          newEntry.data.reported.reportableChange = reportableChange;
          status = sl_zigbee_af_reporting_configure_reported_attribute(&newEntry);
        }
        break;
      }
      case SL_ZIGBEE_ZCL_REPORTING_DIRECTION_RECEIVED:
      {
        uint16_t timeout = sl_zigbee_af_get_int16u(cmd->buffer, bufIndex, cmd->bufLen);
        bufIndex += 2;

        sl_zigbee_af_reporting_println("   timeout:%04X", timeout);

        // Add a reporting entry from a received attribute.  The reports
        // will be sent to us from the source of the Configure Reporting
        // command.
        status = configureReceivedAttribute(cmd,
                                            attributeId,
                                            mask,
                                            timeout);
        break;
      }
      default:
        // This will abort the processing (see below).
        status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
        break;
    }

    // If a report cannot be configured, the status, direction, and
    // attribute are added to the response.  If the failure was due to an
    // invalid field, we have to abort after this record because we don't
    // know how to interpret the rest of the data in the request.
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      (void) sl_zigbee_af_put_int8u_in_resp(status);
      (void) sl_zigbee_af_put_int8u_in_resp(direction);
      (void) sl_zigbee_af_put_int16u_in_resp(attributeId);
      failures = true;
      if (status == SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD) {
        break;
      }
    }
  }

  // We just respond with SUCCESS if we made it through without failures.
  if (!failures) {
    (void) sl_zigbee_af_put_int8u_in_resp(SL_ZIGBEE_ZCL_STATUS_SUCCESS);
  }

  sendStatus = sl_zigbee_af_send_response();
  if (SL_STATUS_OK != sendStatus) {
    sl_zigbee_af_reporting_println("Reporting: failed to send %s response: 0x%02X",
                                   "configure_reporting",
                                   sendStatus);
  }
  return true;
}

bool sl_zigbee_af_read_reporting_configuration_command_cb(const sl_zigbee_af_cluster_command_t *cmd)
{
  sl_status_t sendStatus;
  uint16_t bufIndex = cmd->payloadStartIndex;
  uint8_t frameControl, mask;

  sl_zigbee_af_reporting_print("%s: ", "READ_RPT_CFG");
  sl_zigbee_af_reporting_debug_exec(sl_zigbee_af_decode_and_print_cluster_with_mfg_code(cmd->apsFrame->clusterId, cmd->mfgCode));
  sl_zigbee_af_reporting_println("");
  sl_zigbee_af_reporting_flush();

  if (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER) {
    frameControl = (ZCL_GLOBAL_COMMAND
                    | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                    | SL_ZIGBEE_AF_DEFAULT_RESPONSE_POLICY_REQUESTS);
    mask = CLUSTER_MASK_SERVER;
  } else {
    frameControl = (ZCL_GLOBAL_COMMAND
                    | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                    | SL_ZIGBEE_AF_DEFAULT_RESPONSE_POLICY_REQUESTS);
    mask = CLUSTER_MASK_CLIENT;
  }

  // The manufacturer-specfic version of the fill API only creates a
  // manufacturer-specfic command if the manufacturer code is set.  For non-
  // manufacturer-specfic reports, the manufacturer code is unset, so we can
  // get away with using this API for both cases.
  sl_zigbee_af_fill_external_manufacturer_specific_buffer(frameControl,
                                                          cmd->apsFrame->clusterId,
                                                          cmd->mfgCode,
                                                          ZCL_READ_REPORTING_CONFIGURATION_RESPONSE_COMMAND_ID,
                                                          "");

  // Each record in the command has a one-byte direction and a two-byte
  // attribute id.
  while (bufIndex + 3 <= cmd->bufLen) {
    sl_zigbee_af_attribute_id_t attributeId;
    sl_zigbee_af_attribute_metadata_t *metadata = NULL;
    sl_zigbee_af_plugin_reporting_entry_t entry;
    sl_zigbee_af_reporting_direction_t direction;
    uint8_t i;
    bool found = false;

    direction = (sl_zigbee_af_reporting_direction_t)sl_zigbee_af_get_int8u(cmd->buffer,
                                                                           bufIndex,
                                                                           cmd->bufLen);
    bufIndex++;
    attributeId = (sl_zigbee_af_attribute_id_t)sl_zigbee_af_get_int16u(cmd->buffer,
                                                                       bufIndex,
                                                                       cmd->bufLen);
    bufIndex += 2;

    switch (direction) {
      case SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED:
      case SL_ZIGBEE_ZCL_REPORTING_DIRECTION_RECEIVED:
        metadata = sl_zigbee_af_locate_attribute_metadata(cmd->apsFrame->destinationEndpoint,
                                                          cmd->apsFrame->clusterId,
                                                          attributeId,
                                                          mask,
                                                          cmd->mfgCode);
        if (metadata == NULL) {
          (void) sl_zigbee_af_put_int8u_in_resp(SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE);
          (void) sl_zigbee_af_put_int8u_in_resp(direction);
          (void) sl_zigbee_af_put_int16u_in_resp(attributeId);
          continue;
        }
        break;
      default:
        (void) sl_zigbee_af_put_int8u_in_resp(SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD);
        (void) sl_zigbee_af_put_int8u_in_resp(direction);
        (void) sl_zigbee_af_put_int16u_in_resp(attributeId);
        continue;
    }

    // 075123r03 seems to suggest that SUCCESS is returned even if reporting
    // isn't configured for the requested attribute.  The individual fields
    // of the response for this attribute get populated with defaults.
    // CCB 1854 removes the ambiguity and requires NOT_FOUND to be returned in
    // the status field and all fields except direction and attribute identifier
    // to be omitted if there is no report configuration found.
    for (i = 0; i < reportTableActiveLength; i++) {
      sli_zigbee_af_reporting_get_entry(i, &entry);
      if (entry.direction == direction
          && entry.endpoint == cmd->apsFrame->destinationEndpoint
          && entry.clusterId == cmd->apsFrame->clusterId
          && entry.attributeId == attributeId
          && entry.mask == mask
          && entry.manufacturerCode == cmd->mfgCode
          && (entry.direction == SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED
              || (entry.data.received.source == cmd->source
                  && entry.data.received.endpoint == cmd->apsFrame->sourceEndpoint))) {
        found = true;
        break;
      }
    }
    // Attribute supported, reportable, no report configuration was found.
    if (found == false) {
      (void) sl_zigbee_af_put_int8u_in_resp(SL_ZIGBEE_ZCL_STATUS_NOT_FOUND);
      (void) sl_zigbee_af_put_int8u_in_resp(direction);
      (void) sl_zigbee_af_put_int16u_in_resp(attributeId);
      continue;
    }
    // Attribute supported, reportable, report configuration was found.
    (void) sl_zigbee_af_put_int8u_in_resp(SL_ZIGBEE_ZCL_STATUS_SUCCESS);
    (void) sl_zigbee_af_put_int8u_in_resp(direction);
    (void) sl_zigbee_af_put_int16u_in_resp(attributeId);
    switch (direction) {
      case SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED:
        if (metadata != NULL) {
          (void) sl_zigbee_af_put_int8u_in_resp(metadata->attributeType);
          (void) sl_zigbee_af_put_int16u_in_resp(entry.data.reported.minInterval);
          (void) sl_zigbee_af_put_int16u_in_resp(entry.data.reported.maxInterval);
          if (sl_zigbee_af_get_attribute_analog_or_discrete_type(metadata->attributeType)
              == SL_ZIGBEE_AF_DATA_TYPE_ANALOG) {
            putReportableChangeInResp(&entry, metadata->attributeType);
          }
        }
        break;
      case SL_ZIGBEE_ZCL_REPORTING_DIRECTION_RECEIVED:
        (void) sl_zigbee_af_put_int16u_in_resp(entry.data.received.timeout);
        break;
    }
  }

  sendStatus = sl_zigbee_af_send_response();
  if (SL_STATUS_OK != sendStatus) {
    sl_zigbee_af_reporting_println("Reporting: failed to send %s response: 0x%02X",
                                   "read_reporting_configuration",
                                   sendStatus);
  }
  return true;
}

sl_status_t sl_zigbee_af_clear_report_table_cb(void)
{
  // this operation is more efficient if we loop over the table backwards
  while (reportTableActiveLength > 0) {
    removeConfiguration(reportTableActiveLength - 1);
  }
  sl_zigbee_af_event_set_inactive(tickEvent);
  return SL_STATUS_OK;
}

sl_status_t sli_zigbee_af_reporting_remove_entry(uint16_t index)
{
  sl_status_t status = SL_STATUS_INVALID_INDEX;
  if (index < reportTableActiveLength) {
    removeConfigurationAndScheduleTick(index);
    status = SL_STATUS_OK;
  }
  return status;
}

// This function will check all entries in report table and update
// lastReportValue field with current value of attributes
void sli_zigbee_af_reporting_get_last_value_all(void)
{
  uint16_t i;
  for (i = 0; i < reportTableActiveLength; i++) {
    sl_zigbee_af_plugin_reporting_entry_t entry;
    sli_zigbee_af_reporting_get_entry(i, &entry);
    if (sl_zigbee_af_endpoint_is_enabled(entry.endpoint)
        && entry.direction == SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED) {
      readAttributeAndGetLastValue(&entry, i, NULL, NULL, NULL, 0, true);
    }
  }
  scheduleTick();
}

static void markReportTableChange(uint8_t *dataRef,
                                  uint8_t dataSize,
                                  sl_zigbee_af_attribute_type_t dataType,
                                  const sl_zigbee_af_plugin_reporting_entry_t* const entry,
                                  uint8_t entryIndex)
{
  // If we are reporting this particular attribute, we only care whether
  // the new value meets the reportable change criteria.  If it does, we
  // mark the entry as ready to report and reschedule the tick.  Whether
  // the tick will be scheduled for immediate or delayed execution depends
  // on the minimum reporting interval.  This is handled in the scheduler.
  sl_zigbee_af_difference_type_t difference
    = sl_zigbee_af_get_difference(dataRef,
                                  sli_zigbee_af_report_volatile_data[entryIndex].lastReportValue,
                                  dataSize,
                                  dataType);
  uint8_t analogOrDiscrete = sl_zigbee_af_get_attribute_analog_or_discrete_type(dataType);
  bool changed = false;

  if (dataType == ZCL_FLOAT_SEMI_ATTRIBUTE_TYPE
      || dataType == ZCL_FLOAT_DOUBLE_ATTRIBUTE_TYPE) {
    changed = sl_zigbee_af_detect_report_changed_cb(entry->data.reported.reportableChange,
                                                    difference,
                                                    dataType);
  } else if (dataType == ZCL_FLOAT_SINGLE_ATTRIBUTE_TYPE) {
    float reportableChange = 0, diffFloat = 0;

    memcpy(&reportableChange, &entry->data.reported.reportableChange, sizeof(float));
    memcpy(&diffFloat, &difference, sizeof(float));

    changed = reportableChange <= diffFloat;
  } else {
    changed = entry->data.reported.reportableChange <= difference;
  }

  if ((analogOrDiscrete == SL_ZIGBEE_AF_DATA_TYPE_DISCRETE && difference != 0)
      || (analogOrDiscrete == SL_ZIGBEE_AF_DATA_TYPE_ANALOG && changed)) {
    sli_zigbee_af_report_volatile_data[entryIndex].reportableChange = true;
    scheduleTick();
  }
}

// This function will check specified entry in report table and update
// lastReportValue field with current value of attribute
static sl_zigbee_af_status_t readAttributeAndGetLastValue(const sl_zigbee_af_plugin_reporting_entry_t* const entry,
                                                          uint8_t entryIndex,
                                                          sl_zigbee_af_attribute_type_t* pDataType,
                                                          uint16_t* pDataSize,
                                                          uint8_t* pReadData,
                                                          uint16_t readDataSize,
                                                          bool reportChange)
{
  uint8_t readData[READ_DATA_SIZE];
  uint16_t dataSize;
  sl_zigbee_af_attribute_type_t dataType;

  sl_zigbee_af_status_t status = sli_zigbee_af_read_attribute(entry->endpoint,
                                                              entry->clusterId,
                                                              entry->attributeId,
                                                              entry->mask,
                                                              entry->manufacturerCode,
                                                              readData,
                                                              READ_DATA_SIZE,
                                                              &dataType);

  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_reporting_println("ERR: reading cluster 0x%04X attribute 0x%04X: 0x%02X",
                                   entry->clusterId,
                                   entry->attributeId,
                                   status);
    return status;
  }

  if (sl_zigbee_af_is_long_string_attribute_type(dataType)) {
    // LONG string types are rarely used and even more rarely (never?)
    // reported; ignore and leave ensuing handling of other types unchanged.
    sl_zigbee_af_reporting_println("ERR: reporting of LONG string attribute type not supported: cluster 0x%04X attribute 0x%04X",
                                   entry->clusterId,
                                   entry->attributeId);
    return SL_ZIGBEE_ZCL_STATUS_INVALID_DATA_TYPE;
  }

  // find size of current report
  dataSize = sl_zigbee_af_attribute_value_size(dataType, readData, READ_DATA_SIZE);
  if (dataSize == 0  || (pReadData != NULL && dataSize > readDataSize)) {
    return SL_ZIGBEE_ZCL_STATUS_FAILURE;   // defensive; read attribute above should have failed
  }

  // Store the current attribute value for comparison with future values
  // to detect reportable changes. Use the actual attribute value for
  // data types that are small enough to efficiently store; for string
  // types, substitute a 32-bit hash of the string value.
  uint32_t stringHash = 0;
  uint8_t *copyData = readData;
  uint8_t copySize = dataSize;
  if (dataType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE || dataType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE) {
    // dataSize was set above to count the string's length byte, in addition to string length.
    // Compute hash on string value only.
    stringHash = computeStringHash(readData + 1, dataSize - 1);
    copyData = (uint8_t *)&stringHash;
    copySize = sizeof(stringHash);
  }

  // If need to report attribute change, calculate the difference and schedule to send reportAttributes
  if (reportChange) {
    markReportTableChange(readData, copySize, dataType, entry, entryIndex);
    // The lastReportValue will be updated later in sl_zigbee_af_reporting_tick_event_handler
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  if (copySize <= sizeof(sli_zigbee_af_report_volatile_data[entryIndex].lastReportValue)) {
    sli_zigbee_af_report_volatile_data[entryIndex].lastReportValue = 0;
#if (BIGENDIAN_CPU)
    memmove(((uint8_t *)&sli_zigbee_af_report_volatile_data[entryIndex].lastReportValue
             + sizeof(sli_zigbee_af_report_volatile_data[entryIndex].lastReportValue)
             - copySize),
            copyData,
            copySize);
#else
    memmove(&sli_zigbee_af_report_volatile_data[entryIndex].lastReportValue, copyData, copySize);
#endif
  }

  if (pDataType != NULL) {
    *pDataType = dataType;
  }
  if (pDataSize != NULL) {
    *pDataSize = dataSize;
  }
  if (pReadData != NULL) {
    memcpy(pReadData, readData, dataSize);
  }

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

void sl_zigbee_af_reporting_attribute_change_cb(uint8_t endpoint,
                                                sl_zigbee_af_cluster_id_t clusterId,
                                                sl_zigbee_af_attribute_id_t attributeId,
                                                uint8_t mask,
                                                uint16_t manufacturerCode,
                                                sl_zigbee_af_attribute_type_t type,
                                                uint8_t *data)
{
  uint16_t i;
  for (i = 0; i < reportTableActiveLength; i++) {
    sl_zigbee_af_plugin_reporting_entry_t entry;
    sli_zigbee_af_reporting_get_entry(i, &entry);
    if (entry.direction == SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED
        && entry.endpoint == endpoint
        && entry.clusterId == clusterId
        && entry.attributeId == attributeId
        && entry.mask == mask
        && entry.manufacturerCode == manufacturerCode) {
      // For CHAR and OCTET strings, the string value may be too long to fit into the
      // lastReportValue field (sl_zigbee_af_difference_type_t), so instead we save the string's
      // hash, and detect changes in string value based on unequal hash.
      uint32_t stringHash = 0;
      uint8_t dataSize  = sl_zigbee_af_get_data_size(type);
      uint8_t *dataRef = data;
      if (type == ZCL_OCTET_STRING_ATTRIBUTE_TYPE || type == ZCL_CHAR_STRING_ATTRIBUTE_TYPE) {
        stringHash = computeStringHash(data + 1, sl_zigbee_af_string_length(data));
        dataRef = (uint8_t *)&stringHash;
        dataSize = sizeof(stringHash);
      }
      markReportTableChange(dataRef, dataSize, type, &entry, i);
      break;
    }
  }
}

bool sli_zigbee_af_reporting_do_entries_match(const sl_zigbee_af_plugin_reporting_entry_t* const entry1,
                                              const sl_zigbee_af_plugin_reporting_entry_t* const entry2)
{
  // Verify that the reporting parameters of both entries match.
  // If the entries are for SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED, the
  // reporting configurations do not need to match.  If the direction is
  // SL_ZIGBEE_ZCL_REPORTING_DIRECTION_RECEIVED, then the source and destination
  // endpoints need to match.
  if ((entry1->endpoint == entry2->endpoint)
      && (entry1->clusterId == entry2->clusterId)
      && (entry1->attributeId == entry2->attributeId)
      && (entry1->mask == entry2->mask)
      && (entry1->manufacturerCode == entry2->manufacturerCode)
      && (entry1->direction == entry2->direction)
      && ((entry1->direction == SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED)
          || ((entry1->data.received.source == entry2->data.received.source)
              && (entry1->data.received.endpoint
                  == entry2->data.received.endpoint)))) {
    return true;
  }
  return false;
}

uint16_t sli_zigbee_af_reporting_add_entry(sl_zigbee_af_plugin_reporting_entry_t* newEntry)
{
  uint16_t i;
  sl_zigbee_af_plugin_reporting_entry_t oldEntry;

  // If an entry already exists, or exists but with different parameters,
  // overwrite it with the new entry to prevent pollution of the report table
  for (i = 0; i < reportTableActiveLength; i++) {
    sli_zigbee_af_reporting_get_entry(i, &oldEntry);
    if (sli_zigbee_af_reporting_do_entries_match(&oldEntry, newEntry)) {
      sli_zigbee_af_reporting_set_entry(i, newEntry);
      return i;
    }
  }

  // if no entry is found, add it to an unused index
  return sli_zigbee_af_reporting_append_entry(newEntry);
}

uint16_t sli_zigbee_af_reporting_append_entry(sl_zigbee_af_plugin_reporting_entry_t* newEntry)
{
  uint16_t index = NULL_INDEX;
  if (reportTableActiveLength < REPORT_TABLE_SIZE) {
    index = reportTableActiveLength;
    sli_zigbee_af_reporting_set_entry(index, newEntry);
    reportTableActiveLength++;
  }
  return index;
}

static void scheduleTick(void)
{
  uint32_t delayMs = MAX_INT32U_VALUE;
  uint16_t i;
  for (i = 0; i < reportTableActiveLength; i++) {
    sl_zigbee_af_plugin_reporting_entry_t entry;
    sli_zigbee_af_reporting_get_entry(i, &entry);
    if (sl_zigbee_af_endpoint_is_enabled(entry.endpoint)
        && entry.direction == SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED) {
      uint32_t minIntervalMs = (entry.data.reported.minInterval
                                * MILLISECOND_TICKS_PER_SECOND);
      uint32_t maxIntervalMs = (entry.data.reported.maxInterval
                                * MILLISECOND_TICKS_PER_SECOND);
      uint32_t elapsedMs = elapsedTimeInt32u(sli_zigbee_af_report_volatile_data[i].lastReportTimeMs,
                                             halCommonGetInt32uMillisecondTick());
      uint32_t remainingMs = MAX_INT32U_VALUE;
      if (sli_zigbee_af_report_volatile_data[i].reportableChange) {
        remainingMs = (minIntervalMs < elapsedMs
                       ? 0
                       : minIntervalMs - elapsedMs);
      } else if (maxIntervalMs) {
        remainingMs = (maxIntervalMs < elapsedMs
                       ? 0
                       : maxIntervalMs - elapsedMs);
      }
      if (remainingMs < delayMs) {
        delayMs = remainingMs;
      }
    }
  }
  if (delayMs != MAX_INT32U_VALUE) {
    sl_zigbee_af_debug_println("sched report event for: 0x%08X", delayMs);
    sl_zigbee_af_event_set_delay_ms(tickEvent, delayMs);
  } else {
    sl_zigbee_af_debug_println("deactivate report event");
    sl_zigbee_af_event_set_inactive(tickEvent);
  }
}

static void removeConfiguration(uint16_t index)
{
  if (index < reportTableActiveLength) {
    sl_zigbee_af_plugin_reporting_entry_t entry;
    sli_zigbee_af_reporting_get_entry(index, &entry);
    entry.endpoint = SL_ZIGBEE_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID;
    reportTableActiveLength--;
    if (reportTableActiveLength == index) {
      // we are removing the last entry from the list
      sli_zigbee_af_reporting_set_entry(index, &entry);
    } else {
      // we are removing an entry from the middle of the list
      // must perform a swap in order to maintain list structure
      sl_zigbee_af_plugin_reporting_entry_t swap;
      sli_zigbee_af_report_volatile_data_type ramSwap;
      // swap with the last entry
      sli_zigbee_af_reporting_get_entry(reportTableActiveLength, &swap);
      ramSwap = sli_zigbee_af_report_volatile_data[reportTableActiveLength];
      sli_zigbee_af_reporting_set_entry(index, &swap);
      sli_zigbee_af_report_volatile_data[index] = ramSwap;
      sli_zigbee_af_reporting_set_entry(reportTableActiveLength, &entry);
      // TODO add a callback that fires when indices change to inform anyone who might be watching a specific index
    }

    sl_zigbee_af_reporting_configured_cb(&entry);
  }
}

static void removeConfigurationAndScheduleTick(uint16_t index)
{
  removeConfiguration(index);
  scheduleTick();
}

sl_zigbee_af_status_t sl_zigbee_af_reporting_configure_reported_attribute(const sl_zigbee_af_plugin_reporting_entry_t* newEntry)
{
  sl_zigbee_af_attribute_metadata_t *metadata;
  sl_zigbee_af_plugin_reporting_entry_t entry;
  sl_zigbee_af_status_t status;
  uint16_t i, index = reportTableActiveLength;
  bool initialize = true;

  // Verify that we support the attribute and that the data type matches.
  metadata = sl_zigbee_af_locate_attribute_metadata(newEntry->endpoint,
                                                    newEntry->clusterId,
                                                    newEntry->attributeId,
                                                    newEntry->mask,
                                                    newEntry->manufacturerCode);
  if (metadata == NULL) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  // Verify the minimum and maximum intervals make sense.
  if (newEntry->data.reported.maxInterval != 0
      && (newEntry->data.reported.maxInterval
          < newEntry->data.reported.minInterval)) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }

  // Check the table for an entry that matches this request and also watch for
  // empty slots along the way.  If a report exists, it will be overwritten
  // with the new configuration.  Otherwise, a new entry will be created and
  // initialized.
  for (i = 0; i < reportTableActiveLength; i++) {
    sli_zigbee_af_reporting_get_entry(i, &entry);
    if (entry.direction == SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED
        && entry.endpoint == newEntry->endpoint
        && entry.clusterId == newEntry->clusterId
        && entry.attributeId == newEntry->attributeId
        && entry.mask == newEntry->mask
        && entry.manufacturerCode == newEntry->manufacturerCode) {
      initialize = false;
      index = i;
      break;
    }
  }

  // If the maximum reporting interval is 0xFFFF, the device shall not issue
  // reports for the attribute and the configuration information for that
  // attribute need not be maintained.
  if (newEntry->data.reported.maxInterval == 0xFFFF) {
    if (!initialize) {
      removeConfigurationAndScheduleTick(index);
    }
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  // ZCL v6 Section 2.5.7.1.6 Maximum Reporting Interval Field
  // If this value is set to 0x0000 and the minimum reporting interval field
  // equals 0xffff, then the device SHALL revert back to its default reporting
  // configuration. The reportable change field, if present, SHALL be set to
  // zero.
  // Verify special condition to reset the reporting configuration to defaults
  // if the minimum == 0xFFFF and maximum == 0x0000
  bool reset = false;
  if ((newEntry->data.reported.maxInterval == 0x0000)
      && (newEntry->data.reported.minInterval == 0xFFFF)) {
    // Get the configuration from the default configuration table for this
    memmove(&entry, newEntry, sizeof(sl_zigbee_af_plugin_reporting_entry_t));
    if (sl_zigbee_af_reporting_get_reporting_config_defaults(&entry)) {
      // Then it must be initialise with the default config - explicity
      initialize = true;
      reset = true;
    }
  }

  if (initialize) {
    if (index >= REPORT_TABLE_SIZE) {
      return SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
    }
    entry.direction = SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED;
    entry.endpoint = newEntry->endpoint;
    entry.clusterId = newEntry->clusterId;
    entry.attributeId = newEntry->attributeId;
    entry.mask = newEntry->mask;
    entry.manufacturerCode = newEntry->manufacturerCode;
  }

  // For new or updated entries, set the intervals and reportable change.
  // Updated entries will retain all other settings configured previously.
  if (!reset) {
    entry.data.reported.minInterval = newEntry->data.reported.minInterval;
    entry.data.reported.maxInterval = newEntry->data.reported.maxInterval;
    entry.data.reported.reportableChange = newEntry->data.reported.reportableChange;
  }
  // Give the application a chance to review the configuration that we have
  // been building up.  If the application rejects it, we just do not save the
  // record.  If we were supposed to add a new configuration, it will not be
  // created.  If we were supposed to update an existing configuration, we will
  // keep the old one and just discard any changes.  So, in either case, life
  // continues unchanged if the application rejects the configuration.
  status = sl_zigbee_af_reporting_configured_cb(&entry);
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    if (index == reportTableActiveLength) {
      reportTableActiveLength++;
    }
    // Always update the lastReportTimeMs and lastReportValue when the entry is updated or newly added
    sli_zigbee_af_report_volatile_data[index].lastReportTimeMs = halCommonGetInt32uMillisecondTick();
    if (readAttributeAndGetLastValue(&entry, index, NULL, NULL, NULL, 0, false) != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sli_zigbee_af_report_volatile_data[index].lastReportValue = 0;
    }
    sli_zigbee_af_reporting_set_entry(index, &entry);
    scheduleTick();
  }
  return status;
}

static sl_zigbee_af_status_t configureReceivedAttribute(const sl_zigbee_af_cluster_command_t *cmd,
                                                        sl_zigbee_af_attribute_id_t attributeId,
                                                        uint8_t mask,
                                                        uint16_t timeout)
{
  sl_zigbee_af_plugin_reporting_entry_t entry;
  sl_zigbee_af_status_t status;
  uint16_t i, index = reportTableActiveLength;
  bool initialize = true;

  // Check the table for an entry that matches this request and also watch for
  // empty slots along the way.  If a report exists, it will be overwritten
  // with the new configuration.  Otherwise, a new entry will be created and
  // initialized.
  for (i = 0; i < reportTableActiveLength; i++) {
    sli_zigbee_af_reporting_get_entry(i, &entry);
    if (entry.direction == SL_ZIGBEE_ZCL_REPORTING_DIRECTION_RECEIVED
        && entry.endpoint == cmd->apsFrame->destinationEndpoint
        && entry.clusterId == cmd->apsFrame->clusterId
        && entry.attributeId == attributeId
        && entry.mask == mask
        && entry.manufacturerCode == cmd->mfgCode
        && entry.data.received.source == cmd->source
        && entry.data.received.endpoint == cmd->apsFrame->sourceEndpoint) {
      initialize = false;
      index = i;
      break;
    }
  }

  if (initialize) {
    if (index >= REPORT_TABLE_SIZE) {
      return SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
    }
    entry.direction = SL_ZIGBEE_ZCL_REPORTING_DIRECTION_RECEIVED;
    entry.endpoint = cmd->apsFrame->destinationEndpoint;
    entry.clusterId = cmd->apsFrame->clusterId;
    entry.attributeId = attributeId;
    entry.mask = mask;
    entry.manufacturerCode = cmd->mfgCode;
    entry.data.received.source = cmd->source;
    entry.data.received.endpoint = cmd->apsFrame->sourceEndpoint;
  }

  // For new or updated entries, set the timeout.  Updated entries will retain
  // all other settings configured previously.
  entry.data.received.timeout = timeout;

  // Give the application a chance to review the configuration that we have
  // been building up.  If the application rejects it, we just do not save the
  // record.  If we were supposed to add a new configuration, it will not be
  // created.  If we were supposed to update an existing configuration, we will
  // keep the old one and just discard any changes.  So, in either case, life
  // continues unchanged if the application rejects the configuration.  If the
  // application accepts the change, the tick does not have to be rescheduled
  // here because we don't do anything with received reports.
  status = sl_zigbee_af_reporting_configured_cb(&entry);
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    if (index == reportTableActiveLength) {
      reportTableActiveLength++;
    }

    sli_zigbee_af_reporting_set_entry(index, &entry);
  }
  return status;
}

static void putReportableChangeInResp(const sl_zigbee_af_plugin_reporting_entry_t *entry,
                                      sl_zigbee_af_attribute_type_t dataType)
{
  uint8_t bytes = sl_zigbee_af_get_data_size(dataType);
  if (entry == NULL) { // default, 0xFF...UL or 0x80...L
    for (; bytes > 0; bytes--) {
      uint8_t b = 0xFF;
      if (sl_zigbee_af_is_type_signed(dataType)) {
        b = (bytes == 1 ? 0x80 : 0x00);
      }
      (void) sl_zigbee_af_put_int8u_in_resp(b);
    }
  } else { // reportable change value
    uint32_t value = entry->data.reported.reportableChange;
    for (; bytes > 0; bytes--) {
      uint8_t b = BYTE_0(value);
      (void) sl_zigbee_af_put_int8u_in_resp(b);
      value >>= 8;
    }
  }
}

// Conditionally add reporting entry.
static bool reportEntryDoesNotExist(const sl_zigbee_af_plugin_reporting_entry_t* newEntry)
{
  uint16_t i;
  sl_zigbee_af_plugin_reporting_entry_t entry;

  for (i = 0; i < reportTableActiveLength; i++) {
    sli_zigbee_af_reporting_get_entry(i, &entry);
    if (sli_zigbee_af_reporting_do_entries_match(&entry, newEntry)) {
      return false;
    }
  }

  return true;
}

uint16_t sli_zigbee_af_reporting_conditionally_add_reporting_entry(sl_zigbee_af_plugin_reporting_entry_t* newEntry)
{
  if (reportEntryDoesNotExist(newEntry)) {
    return sli_zigbee_af_reporting_append_entry(newEntry);
  }
  return 0;
}

//------------------------------------------------------------------------------
// Weak callback definitions.

WEAK(sl_zigbee_af_status_t sl_zigbee_af_reporting_configured_cb(const sl_zigbee_af_plugin_reporting_entry_t *entry))
{
  (void)entry;
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

WEAK(bool sl_zigbee_af_reporting_get_default_reporting_config_cb(sl_zigbee_af_plugin_reporting_entry_t *entry))
{
  // Change the values as appropriate for the application.
  entry->data.reported.minInterval = 1;
  entry->data.reported.maxInterval = 0xFFFE;
  entry->data.reported.reportableChange = 1;
  entry->direction = SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED;
  return true;
}

WEAK(bool sl_zigbee_af_detect_report_changed_cb(sl_zigbee_af_difference_type_t threshold,
                                                sl_zigbee_af_difference_type_t diff,
                                                sl_zigbee_af_attribute_type_t dataType))
{
  (void)threshold;
  (void)diff;
  (void)dataType;
  return false;
}
