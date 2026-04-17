/***************************************************************************//**
 * @file
 * @brief Logging routines for the Door Lock Server plugin.
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
#include "app/framework/plugin/door-lock-server/door-lock-server.h"

#include "zap-cluster-command-parser.h"

static sl_zigbee_af_plugin_door_lock_server_log_entry_t entries[SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_LOG_ENTRIES];
static uint8_t nextEntryId = 1;

#define ENTRY_ID_TO_INDEX(entryId) ((entryId) - 1)
#define ENTRY_ID_IS_VALID(entryId) ((entryId) > 0 && (entryId) < nextEntryId)
#define MOST_RECENT_ENTRY_ID() (nextEntryId - 1)
#define LOG_IS_EMPTY() (nextEntryId == 1)

static bool loggingIsEnabled(void)
{
  // This is hardcoded to endpoint 1 because...we need to add endpoint support...
  uint8_t endpoint = 1;
  bool logging = false;
  sl_zigbee_af_status_t status
    = sl_zigbee_af_read_server_attribute(endpoint,
                                         ZCL_DOOR_LOCK_CLUSTER_ID,
                                         ZCL_ENABLE_LOGGING_ATTRIBUTE_ID,
                                         (uint8_t *)&logging,
                                         sizeof(logging));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_door_lock_cluster_println("Could not read EnableLogging attribute: 0x%02X",
                                           status);
  }
  return logging;
}

bool sl_zigbee_af_door_lock_server_add_log_entry(sl_zigbee_af_door_lock_event_type_t eventType,
                                                 sl_zigbee_af_door_lock_event_source_t source,
                                                 uint8_t eventId,
                                                 uint16_t userId,
                                                 uint8_t pinLength,
                                                 uint8_t *pin)
{
  if (!loggingIsEnabled()
      || ENTRY_ID_TO_INDEX(nextEntryId) >= (uint8_t) COUNTOF(entries)) {
    return false;
  }

  sl_zigbee_af_plugin_door_lock_server_log_entry_t *nextEntry
    = &entries[ENTRY_ID_TO_INDEX(nextEntryId)];
  nextEntry->logEntryId = nextEntryId;
  nextEntry->timestamp = sl_zigbee_af_get_current_time_cb();
  nextEntry->eventType = eventType;
  nextEntry->source = source;
  nextEntry->eventId = eventId;
  nextEntry->userId = userId;
  // Truncate logged PIN if larger than log entry capacity.
  uint8_t moveLength = (pinLength > SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_PIN_LENGTH
                        ? SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_PIN_LENGTH
                        : pinLength);
  nextEntry->pin[0] = moveLength;
  memmove(nextEntry->pin + 1, pin, moveLength);

  nextEntryId++;

  return true;
}

bool sl_zigbee_af_door_lock_server_get_log_entry(uint16_t *entryId,
                                                 sl_zigbee_af_plugin_door_lock_server_log_entry_t *entry)
{
  if (LOG_IS_EMPTY()) {
    return false;
  }

  if (!ENTRY_ID_IS_VALID(*entryId)) {
    *entryId = MOST_RECENT_ENTRY_ID();
  }
  assert(ENTRY_ID_IS_VALID(*entryId));

  *entry = entries[ENTRY_ID_TO_INDEX(*entryId)];

  return true;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_log_record_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_get_log_record_command_t cmd_data;
  sl_status_t status;
  sl_zigbee_af_plugin_door_lock_server_log_entry_t entry;

  if (zcl_decode_door_lock_cluster_get_log_record_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (LOG_IS_EMPTY() || !sl_zigbee_af_door_lock_server_get_log_entry(&cmd_data.logIndex, &entry)) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }
  sl_zigbee_af_fill_command_door_lock_cluster_get_log_record_response(entry.logEntryId,
                                                                      entry.timestamp,
                                                                      entry.eventType,
                                                                      entry.source,
                                                                      entry.eventId,
                                                                      entry.userId,
                                                                      entry.pin);
  status = sl_zigbee_af_send_response();
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_door_lock_cluster_println("Failed to send GetLogRecordResponse: 0x%02X",
                                           status);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}
