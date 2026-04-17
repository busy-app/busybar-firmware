/***************************************************************************//**
 * @file
 * @brief Routines for the Door Lock Server plugin.
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

static sl_zigbee_af_plugin_door_lock_server_weekday_schedule_entry_t weekdayScheduleTable[SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE];
static sl_zigbee_af_plugin_door_lock_server_yearday_schedule_entry_t yeardayScheduleTable[SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE];
static sl_zigbee_af_plugin_door_lock_server_holiday_schedule_entry_t holidayScheduleTable[SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_HOLIDAY_SCHEDULE_TABLE_SIZE];
static sl_zigbee_af_plugin_door_lock_server_disposable_schedule_entry_t disposableScheduleTable[SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE];

static void printWeekdayScheduleTable(void)
{
  uint8_t i;
  sl_zigbee_af_door_lock_cluster_println("id uid dm strth strtm stph stpm");
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE; i++ ) {
    sl_zigbee_af_door_lock_schedule_entry_t *entry = &weekdayScheduleTable[i];
    if (entry->inUse) {
      sl_zigbee_af_door_lock_cluster_println("%02X %04X  %02X %08X   %08X   %08X  %08X",
                                             i,
                                             entry->userId,
                                             entry->daysMask,
                                             entry->startHour,
                                             entry->stopHour,
                                             entry->stopMinute);
    }
  }
}

static void clearWeekdayScheduleTable(void)
{
  for (uint8_t i = 0;
       i < SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE;
       i++) {
    weekdayScheduleTable[i].inUse = false;
  }
}

static void clearYeardayScheduleTable(void)
{
  for (uint8_t i = 0;
       i < SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE;
       i++) {
    yeardayScheduleTable[i].inUse = false;
  }
}

static void clearHolidayScheduleTable(void)
{
  for (uint8_t i = 0;
       i < SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_HOLIDAY_SCHEDULE_TABLE_SIZE;
       i++) {
    holidayScheduleTable[i].inUse = false;
  }
}

static void clearDisposableScheduleTable(void)
{
  for (uint8_t i = 0;
       i < SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE;
       i++) {
    disposableScheduleTable[i].inUse = false;
  }
}

void sli_zigbee_af_door_lock_server_init_schedule(void)
{
  clearWeekdayScheduleTable();
  clearYeardayScheduleTable();
  clearHolidayScheduleTable();
  clearDisposableScheduleTable();

#if defined(ZCL_USING_DOOR_LOCK_CLUSTER_NUM_WEEKDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE)  \
  || defined(ZCL_USING_DOOR_LOCK_CLUSTER_NUM_YEARDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE) \
  || defined(ZCL_USING_DOOR_LOCK_CLUSTER_NUM_HOLIDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE)
  const sli_zigbee_af_door_lock_server_attribute_data data[] = {
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_NUM_WEEKDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE
    { ZCL_NUM_WEEKDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE_ID,
      SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE },
#endif

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_NUM_YEARDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE
    { ZCL_NUM_YEARDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE_ID,
      SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE },
#endif

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_NUM_HOLIDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE
    { ZCL_NUM_HOLIDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE_ID,
      SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_HOLIDAY_SCHEDULE_TABLE_SIZE },
#endif
  };
  sli_zigbee_af_door_lock_server_write_attributes(data, COUNTOF(data), "schedule table");
#endif
}

static void sendResponse(const char *responseName)
{
  sl_status_t status = sl_zigbee_af_send_response();
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_door_lock_cluster_println("Failed to send %s: 0x%02X",
                                           responseName,
                                           status);
  }
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_set_weekday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_set_weekday_schedule_command_t cmd_data;
  uint8_t status = 0x00;
  uint8_t userPin = 0x00;
  uint16_t rfProgrammingEventMask = 0xffff; //event sent by default

  if (zcl_decode_door_lock_cluster_set_weekday_schedule_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (!sli_zigbee_af_door_lock_server_check_for_sufficient_space(cmd_data.scheduleId,
                                                                 SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE)
      || !sli_zigbee_af_door_lock_server_check_for_sufficient_space(cmd_data.userId,
                                                                    SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE)) {
    status = 0x01;
  }
  if (!status) {
    sl_zigbee_af_door_lock_schedule_entry_t *entry = &weekdayScheduleTable[cmd_data.scheduleId];
    entry->inUse = true;
    entry->userId = cmd_data.userId;
    entry->daysMask = cmd_data.daysMask;
    entry->startHour = cmd_data.startHour;
    entry->startMinute = cmd_data.startMinute;
    entry->stopHour = cmd_data.endHour;
    entry->stopMinute = cmd_data.endMinute;
    sl_zigbee_af_door_lock_cluster_println("***RX SET WEEKDAY SCHEDULE***");
    printWeekdayScheduleTable();
  }
  sl_zigbee_af_fill_command_door_lock_cluster_set_weekday_schedule_response(status);
  sl_zigbee_af_send_response();

  //get bitmask so we can check if we should send event notification
  sl_zigbee_af_read_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                     ZCL_DOOR_LOCK_CLUSTER_ID,
                                     ZCL_RF_PROGRAMMING_EVENT_MASK_ATTRIBUTE_ID,
                                     (uint8_t*)&rfProgrammingEventMask,
                                     sizeof(rfProgrammingEventMask));

  if (rfProgrammingEventMask & BIT(0)) {
    sl_zigbee_af_fill_command_door_lock_cluster_programming_event_notification(0x01, 0x00, cmd_data.userId, &userPin, 0x00, 0x00, 0x00, &userPin);
    SEND_COMMAND_UNICAST_TO_BINDINGS();
  }

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_weekday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_get_weekday_schedule_command_t cmd_data;
  sl_zigbee_af_status_t zclStatus;
  sl_zigbee_af_door_lock_schedule_entry_t *entry;

  if (zcl_decode_door_lock_cluster_get_weekday_schedule_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  zclStatus = ((cmd_data.scheduleId > SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE)
               ? SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD
               : SL_ZIGBEE_ZCL_STATUS_SUCCESS);
  entry = &weekdayScheduleTable[0];
  if (zclStatus == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    entry = &weekdayScheduleTable[cmd_data.scheduleId];
    zclStatus = (!entry->inUse
                 ? SL_ZIGBEE_ZCL_STATUS_NOT_FOUND
                 : (entry->userId != cmd_data.userId
                    ? SL_ZIGBEE_ZCL_STATUS_NOT_FOUND
                    : SL_ZIGBEE_ZCL_STATUS_SUCCESS));
  }

  if (zclStatus == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_fill_command_door_lock_cluster_get_weekday_schedule_response(cmd_data.scheduleId,
                                                                              cmd_data.userId,
                                                                              zclStatus,
                                                                              entry->daysMask,
                                                                              entry->startHour,
                                                                              entry->startMinute,
                                                                              entry->stopHour,
                                                                              entry->stopMinute);
  } else {
    // For error status, the schedule detail fields are omitted from the response.
    sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                       | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                                       | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                      ZCL_DOOR_LOCK_CLUSTER_ID,
                                      ZCL_GET_WEEKDAY_SCHEDULE_RESPONSE_COMMAND_ID,
                                      "uvu",
                                      cmd_data.scheduleId,
                                      cmd_data.userId,
                                      zclStatus);
  }

  sendResponse("GetWeekdayScheduleResponse");

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_weekday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_clear_weekday_schedule_command_t cmd_data;
  sl_zigbee_af_status_t zclStatus;

  if (zcl_decode_door_lock_cluster_clear_weekday_schedule_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  zclStatus = ((cmd_data.scheduleId > SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE)
               ? SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD
               : SL_ZIGBEE_ZCL_STATUS_SUCCESS);
  if (zclStatus == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    weekdayScheduleTable[cmd_data.scheduleId].inUse = false;
    sli_zigbee_af_door_lock_server_set_pin_user_type(cmd_data.userId,
                                                     SL_ZIGBEE_ZCL_DOOR_LOCK_USER_TYPE_UNRESTRICTED);
  }

  sl_zigbee_af_fill_command_door_lock_cluster_clear_weekday_schedule_response(zclStatus);

  sendResponse("ClearWeekdayScheduleResponse");

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_set_yearday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_set_yearday_schedule_command_t cmd_data;
  uint8_t status;

  if (zcl_decode_door_lock_cluster_set_yearday_schedule_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (cmd_data.scheduleId >= SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE) {
    status = 0x01; // failure (per 7.3.2.17.15)
  } else {
    yeardayScheduleTable[cmd_data.scheduleId].userId = cmd_data.userId;
    yeardayScheduleTable[cmd_data.scheduleId].localStartTime = cmd_data.localStartTime;
    yeardayScheduleTable[cmd_data.scheduleId].localEndTime = cmd_data.localEndTime;
    yeardayScheduleTable[cmd_data.scheduleId].inUse = true;
    status = 0x00; // success (per 7.3.2.17.15)
  }
  sl_zigbee_af_fill_command_door_lock_cluster_set_yearday_schedule_response(status);

  sendResponse("SetYeardayScheduleResponse");

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_yearday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_get_yearday_schedule_command_t cmd_data;
  sl_zigbee_af_plugin_door_lock_server_yearday_schedule_entry_t *entry = &yeardayScheduleTable[0];
  sl_zigbee_af_status_t zclStatus;

  if (zcl_decode_door_lock_cluster_get_yearday_schedule_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (cmd_data.scheduleId >= SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE
      || cmd_data.userId >= SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE) {
    zclStatus = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else {
    entry = &yeardayScheduleTable[cmd_data.scheduleId];
    if (!entry->inUse || entry->userId != cmd_data.userId) {
      zclStatus = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
    } else {
      zclStatus = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    }
  }

  if (zclStatus == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_fill_command_door_lock_cluster_get_yearday_schedule_response(cmd_data.scheduleId,
                                                                              cmd_data.userId,
                                                                              zclStatus,
                                                                              entry->localStartTime,
                                                                              entry->localEndTime);
  } else {
    // For error status, the schedule detail fields are omitted from the response.
    sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                       | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                                       | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                      ZCL_DOOR_LOCK_CLUSTER_ID,
                                      ZCL_GET_YEARDAY_SCHEDULE_RESPONSE_COMMAND_ID,
                                      "uvu",
                                      cmd_data.scheduleId,
                                      cmd_data.userId,
                                      zclStatus);
  }

  sendResponse("GetYeardayScheduleResponse");

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_yearday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_clear_yearday_schedule_command_t cmd_data;
  uint8_t status;

  if (zcl_decode_door_lock_cluster_clear_yearday_schedule_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (cmd_data.scheduleId >= SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE) {
    status = 0x01; // failure (per 7.3.2.17.17)
  } else {
    yeardayScheduleTable[cmd_data.scheduleId].inUse = false;
    sli_zigbee_af_door_lock_server_set_pin_user_type(cmd_data.userId,
                                                     SL_ZIGBEE_ZCL_DOOR_LOCK_USER_TYPE_UNRESTRICTED);
    status = 0x00; // success (per 7.3.2.17.17)
  }
  sl_zigbee_af_fill_command_door_lock_cluster_clear_yearday_schedule_response(status);

  sendResponse("ClearYeardayScheduleResponse");

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_set_holiday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_set_holiday_schedule_command_t cmd_data;
  uint8_t status;

  if (zcl_decode_door_lock_cluster_set_holiday_schedule_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (cmd_data.scheduleId
      >= SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_HOLIDAY_SCHEDULE_TABLE_SIZE) {
    status = 0x01; // failure (per 7.3.2.17.18)
  } else {
    holidayScheduleTable[cmd_data.scheduleId].localStartTime
      = cmd_data.localStartTime;
    holidayScheduleTable[cmd_data.scheduleId].localEndTime
      = cmd_data.localEndTime;
    holidayScheduleTable[cmd_data.scheduleId].operatingModeDuringHoliday
      = cmd_data.operatingModeDuringHoliday;
    holidayScheduleTable[cmd_data.scheduleId].inUse
      = true;
    status = 0x00; // success (per 7.3.2.17.18)
  }
  sl_zigbee_af_fill_command_door_lock_cluster_set_holiday_schedule_response(status);

  sendResponse("SetHolidayScheduleResponse");

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_holiday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_get_holiday_schedule_command_t cmd_data;
  sl_zigbee_af_plugin_door_lock_server_holiday_schedule_entry_t *entry = &holidayScheduleTable[0];
  sl_zigbee_af_status_t zclStatus;

  if (zcl_decode_door_lock_cluster_get_holiday_schedule_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (cmd_data.scheduleId >= SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_HOLIDAY_SCHEDULE_TABLE_SIZE) {
    zclStatus = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else {
    entry = &holidayScheduleTable[cmd_data.scheduleId];
    if (!entry->inUse) {
      zclStatus = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
    } else {
      zclStatus = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    }
  }

  if (zclStatus == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_fill_command_door_lock_cluster_get_holiday_schedule_response(cmd_data.scheduleId,
                                                                              zclStatus,
                                                                              entry->localStartTime,
                                                                              entry->localEndTime,
                                                                              entry->operatingModeDuringHoliday);
  } else {
    // For error status, the schedule detail fields are omitted from the response.
    sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                       | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                                       | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                      ZCL_DOOR_LOCK_CLUSTER_ID,
                                      ZCL_GET_HOLIDAY_SCHEDULE_RESPONSE_COMMAND_ID,
                                      "uu",
                                      cmd_data.scheduleId,
                                      zclStatus);
  }

  sendResponse("GetHolidayScheduleResponse");

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_holiday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_clear_holiday_schedule_command_t cmd_data;
  uint8_t status;

  if (zcl_decode_door_lock_cluster_clear_holiday_schedule_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (cmd_data.scheduleId >= SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE) {
    status = 0x01; // failure (per 7.3.2.17.20)
  } else {
    holidayScheduleTable[cmd_data.scheduleId].inUse = false;
    status = 0x00; // success (per 7.3.2.17.20)
  }
  sl_zigbee_af_fill_command_door_lock_cluster_clear_holiday_schedule_response(status);

  sendResponse("ClearYeardayScheduleResponse");

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_set_disposable_schedule_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_set_disposable_schedule_command_t cmd_data;
  uint8_t status = 0;

  if (zcl_decode_door_lock_cluster_set_disposable_schedule_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_door_lock_cluster_println("Set Disposable Schedule ");

  if (cmd_data.userId >= SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE) {
    status = 0x01;
  } else {
    disposableScheduleTable[cmd_data.userId].localStartTime = cmd_data.localStartTime;
    disposableScheduleTable[cmd_data.userId].localEndTime = cmd_data.localEndTime;
    disposableScheduleTable[cmd_data.userId].inUse = true;
    status = 0x00;
  }

  sl_zigbee_af_fill_command_door_lock_cluster_set_disposable_schedule_response(status);

  sendResponse("SetDisposableScheduleResponse");

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_disposable_schedule_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_get_disposable_schedule_command_t cmd_data;
  sl_zigbee_af_plugin_door_lock_server_disposable_schedule_entry_t *entry = &disposableScheduleTable[0];
  sl_zigbee_af_status_t zclStatus;

  if (zcl_decode_door_lock_cluster_get_disposable_schedule_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_door_lock_cluster_println("Get Disposable Schedule ");

  if (cmd_data.userId >= SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE) {
    zclStatus = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else {
    entry = &disposableScheduleTable[cmd_data.userId];
    if (!entry->inUse) {
      zclStatus = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
    } else {
      zclStatus = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    }
  }

  if (zclStatus == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_fill_command_door_lock_cluster_get_disposable_schedule_response(cmd_data.userId,
                                                                                 zclStatus,
                                                                                 entry->localStartTime,
                                                                                 entry->localEndTime);
  } else {
    // For error status, the schedule detail fields are omitted from the response.
    sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                       | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                                       | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                      ZCL_DOOR_LOCK_CLUSTER_ID,
                                      ZCL_GET_DISPOSABLE_SCHEDULE_RESPONSE_COMMAND_ID,
                                      "vu",
                                      cmd_data.userId,
                                      zclStatus);
  }

  sendResponse("GetDisposableScheduleResponse");

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_disposable_schedule_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_clear_disposable_schedule_command_t cmd_data;
  uint8_t status = 0;

  if (zcl_decode_door_lock_cluster_clear_disposable_schedule_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_door_lock_cluster_println("Clear Disposable Schedule ");

  if (cmd_data.userId >= SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE) {
    status = 0x01;
  } else {
    disposableScheduleTable[cmd_data.userId].inUse = false;
    sli_zigbee_af_door_lock_server_set_pin_user_type(cmd_data.userId,
                                                     SL_ZIGBEE_ZCL_DOOR_LOCK_USER_TYPE_UNRESTRICTED);
    status = 0x00;
  }

  sl_zigbee_af_fill_command_door_lock_cluster_clear_disposable_schedule_response(status);

  sendResponse("ClearDisposableScheduleResponse");

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}
