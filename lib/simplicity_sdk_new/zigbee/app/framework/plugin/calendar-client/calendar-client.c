/***************************************************************************//**
 * @file
 * @brief Routines for the Calendar Client plugin.
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
#include "calendar-client.h"

#include "calendar-client-config.h"
#include "zap-cluster-command-parser.h"

#ifndef SL_ZIGBEE_AF_GENERATE_CLI
  #error The Calendar Client plugin is not compatible with the legacy CLI.
#endif

#define fieldLength(field) \
  (sl_zigbee_af_current_command()->bufLen - ((field) - sl_zigbee_af_current_command()->buffer))

static sl_zigbee_af_calendar_t tempCalendar;
static sl_zigbee_af_status_t addCalendar(sl_zigbee_af_calendar_t *calendar);
static sl_zigbee_af_calendar_t *findCalendar(uint32_t providerId,
                                             uint32_t issuerCalendarId);
static void removeCalendar(sl_zigbee_af_calendar_t *calendar);

static sl_zigbee_af_calendar_t calendars[SL_ZIGBEE_ZCL_CALENDAR_CLUSTER_CLIENT_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS];

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_calendar_cluster_publish_calendar_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_calendar_cluster_publish_calendar_command_t cmd_data;
  sl_zigbee_af_status_t status;

  if (zcl_decode_calendar_cluster_publish_calendar_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_calendar_cluster_print("RX: PublishCalendar 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%02X, \"",
                                      cmd_data.providerId,
                                      cmd_data.issuerEventId,
                                      cmd_data.issuerCalendarId,
                                      cmd_data.startTime,
                                      cmd_data.calendarType);
  sl_zigbee_af_calendar_cluster_print_string(cmd_data.calendarName);
  sl_zigbee_af_calendar_cluster_println("\", %d, %d, %d",
                                        cmd_data.numberOfSeasons,
                                        cmd_data.numberOfWeekProfiles,
                                        cmd_data.numberOfDayProfiles);

  if (findCalendar(cmd_data.providerId, cmd_data.issuerCalendarId) != NULL) {
    sl_zigbee_af_debug_println("ERR: Duplicate calendar: 0x%08X/0x%08X",
                               cmd_data.providerId,
                               cmd_data.issuerCalendarId);
    status = SL_ZIGBEE_ZCL_STATUS_DUPLICATE_EXISTS;
  } else if (SL_ZIGBEE_ZCL_CALENDAR_TYPE_AUXILLIARY_LOAD_SWITCH_CALENDAR
             < cmd_data.calendarType) {
    sl_zigbee_af_debug_println("ERR: Invalid calendar type: 0x%02X", cmd_data.calendarType);
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else if (SL_ZIGBEE_ZCL_CALENDAR_TIME_REFERENCE_UTC_TIME
             != cmd_data.calendarTimeReference) {
    sl_zigbee_af_debug_println("ERR: Invalid calendar time reference: 0x%02X", cmd_data.calendarTimeReference);
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else if (0 < cmd_data.numberOfSeasons && cmd_data.numberOfWeekProfiles == 0) {
    sl_zigbee_af_debug_println("ERR: Number of week profiles cannot be zero");
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else if (SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_SEASONS < cmd_data.numberOfSeasons) {
    sl_zigbee_af_debug_println("ERR: Insufficient space for seasons: %d < %d",
                               SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_SEASONS,
                               cmd_data.numberOfSeasons);
    status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
  } else if (SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_WEEK_PROFILES
             < cmd_data.numberOfWeekProfiles) {
    sl_zigbee_af_debug_println("ERR: Insufficient space for week profiles: %d < %d",
                               SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_WEEK_PROFILES,
                               cmd_data.numberOfWeekProfiles);
    status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
  } else if (cmd_data.numberOfDayProfiles == 0) {
    sl_zigbee_af_debug_println("ERR: Number of day profiles cannot be zero");
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else if (SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_DAY_PROFILES
             < cmd_data.numberOfDayProfiles) {
    sl_zigbee_af_debug_println("ERR: Insufficient space for day profiles: %d < %d",
                               SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_DAY_PROFILES,
                               cmd_data.numberOfDayProfiles);
    status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
  } else {
    uint8_t i;
    tempCalendar.inUse = true;
    tempCalendar.providerId = cmd_data.providerId;
    tempCalendar.issuerEventId = cmd_data.issuerEventId;
    tempCalendar.issuerCalendarId = cmd_data.issuerCalendarId;
    tempCalendar.startTimeUtc = cmd_data.startTime;
    tempCalendar.calendarType = cmd_data.calendarType;
    sl_zigbee_af_copy_string(tempCalendar.calendarName,
                             cmd_data.calendarName,
                             SL_ZIGBEE_AF_CALENDAR_MAXIMUM_CALENDAR_NAME_LENGTH);
    tempCalendar.numberOfSeasons = cmd_data.numberOfSeasons;
    tempCalendar.receivedSeasons = 0;
    tempCalendar.numberOfWeekProfiles = cmd_data.numberOfWeekProfiles;
    tempCalendar.numberOfDayProfiles = cmd_data.numberOfDayProfiles;
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_WEEK_PROFILES; i++) {
      tempCalendar.weekProfiles[i].inUse = false;
    }
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_DAY_PROFILES; i++) {
      tempCalendar.dayProfiles[i].inUse = false;
    }
    tempCalendar.specialDayProfile.inUse = false;
    status = addCalendar(&tempCalendar);
  }

  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_calendar_cluster_publish_day_profile_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_calendar_cluster_publish_day_profile_command_t cmd_data;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  sl_zigbee_af_calendar_t *calendar;

  if (zcl_decode_calendar_cluster_publish_day_profile_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }

  sl_zigbee_af_calendar_cluster_print("RX: PublishDayProfile 0x%08X, 0x%08X, 0x%08X, %d, %d, %d, %d, 0x%02X, [",
                                      cmd_data.providerId,
                                      cmd_data.issuerEventId,
                                      cmd_data.issuerCalendarId,
                                      cmd_data.dayId,
                                      cmd_data.totalNumberOfScheduleEntries,
                                      cmd_data.commandIndex,
                                      cmd_data.totalNumberOfCommands,
                                      cmd_data.calendarType);
  // TODO: print dayScheduleEntries
  sl_zigbee_af_calendar_cluster_println("]");

  // The PublishDayProfile command is published in response to a GetDayProfile
  // command.  If the IssuerCalendarID does not match with one of the stored
  // calendar instances, the client shall ignore the command and respond using
  // ZCL Default Response with a status response of NOT_FOUND.

  // The Calendar server shall send only the number of Schedule Entries
  // belonging to this calendar instance.  Server and clients shall be able to
  // store at least 1 DayProfile and at least one ScheduleEntries per day
  // profile.  If the client is not able to store all ScheduleEntries, the
  // device should respond using ZCL Default Response with a status response of
  // INSUFFICIENT_SPACE.

  calendar = findCalendar(cmd_data.providerId, cmd_data.issuerCalendarId);
  if (calendar == NULL) {
    sl_zigbee_af_debug_println("ERR: Calendar not found: 0x%08X/0x%08X",
                               cmd_data.providerId,
                               cmd_data.issuerCalendarId);
    status = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  } else if (cmd_data.dayId == 0) {
    sl_zigbee_af_debug_println("ERR: Day id cannot be zero");
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else if (calendar->numberOfDayProfiles < cmd_data.dayId) {
    sl_zigbee_af_debug_println("ERR: Invalid day: %d < %d",
                               calendar->numberOfDayProfiles,
                               cmd_data.dayId);
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else if (SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_DAY_PROFILES < cmd_data.dayId) {
    // This should never happen because of the previous check and the one in
    // PublishCalendar.  It is here for completeness.
    sl_zigbee_af_debug_println("ERR: Insufficient space for day: %d < %d",
                               SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_DAY_PROFILES,
                               cmd_data.dayId);
    status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
  } else if (SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_SCHEDULE_ENTRIES
             < cmd_data.totalNumberOfScheduleEntries) {
    sl_zigbee_af_debug_println("ERR: Insufficient space for schedule entries: %d < %d",
                               SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_SCHEDULE_ENTRIES,
                               cmd_data.totalNumberOfScheduleEntries);
    status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
  } else if (cmd_data.totalNumberOfCommands <= cmd_data.commandIndex) {
    sl_zigbee_af_debug_println("ERR: Inconsistent command index: %d <= %d",
                               cmd_data.totalNumberOfCommands,
                               cmd_data.commandIndex);
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else if (calendar->calendarType != cmd_data.calendarType) {
    sl_zigbee_af_debug_println("ERR: Inconsistent calendar type: 0x%02X != 0x%02X",
                               calendar->calendarType,
                               cmd_data.calendarType);
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else {
    sl_zigbee_af_calendar_day_profile_t *dayProfile = &calendar->dayProfiles[cmd_data.dayId - 1];
    uint16_t dayScheduleEntriesLength = fieldLength(cmd_data.dayScheduleEntries);
    uint16_t dayScheduleEntriesIndex = 0;

    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    if (!dayProfile->inUse) {
      dayProfile->inUse = true;
      dayProfile->numberOfScheduleEntries = cmd_data.totalNumberOfScheduleEntries;
    } else if (dayProfile->numberOfScheduleEntries
               != cmd_data.totalNumberOfScheduleEntries) {
      sl_zigbee_af_debug_println("ERR: Inconsistent number of schedule entries: 0x%02X != 0x%02X",
                                 dayProfile->numberOfScheduleEntries,
                                 cmd_data.totalNumberOfScheduleEntries);
      status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    }

    dayProfile->receivedScheduleEntries = 0;
    while (dayScheduleEntriesIndex < dayScheduleEntriesLength
           && status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      // The rate switch times, friendly credit switch times, and auxilliary
      // load switch times all use a two-byte start time followed by a one-
      // byte value.  These are the only valid types, so all entries must have
      // at least three bytes and, below, we just stuff the bytes into the rate
      // switch time entry in the union to simplify the code.
      if (dayScheduleEntriesLength < dayScheduleEntriesIndex + 3) {
        sl_zigbee_af_debug_println("ERR: Malformed schedule entry");
        status = SL_ZIGBEE_ZCL_STATUS_MALFORMED_COMMAND;
      } else if (dayProfile->numberOfScheduleEntries
                 <= dayProfile->receivedScheduleEntries) {
        sl_zigbee_af_debug_println("ERR: Inconsistent number of received schedule entries: %d <= %d",
                                   dayProfile->numberOfScheduleEntries,
                                   dayProfile->receivedScheduleEntries);
        status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
      } else if (SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_SCHEDULE_ENTRIES
                 <= dayProfile->receivedScheduleEntries) {
        // This should never happen because of the checks above.  It is here
        // for completeness.
        sl_zigbee_af_debug_println("ERR: Insufficient space for received schedule entries: %d <= %d",
                                   SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_SCHEDULE_ENTRIES,
                                   dayProfile->receivedScheduleEntries);
        status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
      } else {
        sl_zigbee_af_calendar_schedule_entry_t *scheduleEntry = &dayProfile->scheduleEntries[dayProfile->receivedScheduleEntries++];
        scheduleEntry->rateSwitchTime.startTimeM = sl_zigbee_af_get_int16u(cmd_data.dayScheduleEntries,
                                                                           dayScheduleEntriesIndex,
                                                                           dayScheduleEntriesLength);
        dayScheduleEntriesIndex += 2;
        scheduleEntry->rateSwitchTime.priceTier = sl_zigbee_af_get_int8u(cmd_data.dayScheduleEntries,
                                                                         dayScheduleEntriesIndex,
                                                                         dayScheduleEntriesLength);
        dayScheduleEntriesIndex++;
        if (scheduleEntry != dayProfile->scheduleEntries) {
          sl_zigbee_af_calendar_schedule_entry_t *previous = scheduleEntry - 1;
          if (scheduleEntry->rateSwitchTime.startTimeM
              <= previous->rateSwitchTime.startTimeM) {
            sl_zigbee_af_debug_println("ERR: Inconsistent start times: 0x%04X <= 0x%04X",
                                       scheduleEntry->rateSwitchTime.startTimeM,
                                       previous->rateSwitchTime.startTimeM);
            status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
          }
        }
      }
    }
  }

  if (calendar != NULL && status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_debug_println("ERR: Removing invalid calendar: 0x%08X/0x%08X",
                               calendar->providerId,
                               calendar->issuerCalendarId);
    removeCalendar(calendar);
  }

  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_calendar_cluster_publish_week_profile_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_calendar_cluster_publish_week_profile_command_t cmd_data;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  sl_zigbee_af_calendar_t *calendar;

  if (zcl_decode_calendar_cluster_publish_week_profile_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }

  sl_zigbee_af_calendar_cluster_println("RX: PublishWeekProfile 0x%08X, 0x%08X, 0x%08X, %d, %d, %d, %d, %d, %d, %d, %d",
                                        cmd_data.providerId,
                                        cmd_data.issuerEventId,
                                        cmd_data.issuerCalendarId,
                                        cmd_data.weekId,
                                        cmd_data.dayIdRefMonday,
                                        cmd_data.dayIdRefTuesday,
                                        cmd_data.dayIdRefWednesday,
                                        cmd_data.dayIdRefThursday,
                                        cmd_data.dayIdRefFriday,
                                        cmd_data.dayIdRefSaturday,
                                        cmd_data.dayIdRefSunday);

  // The PublishWeekProfile command is published in response to a
  // GetWeekProfile command.  If the IssuerCalendarID does not match with one
  // of the stored calendar instances, the client shall ignore the command and
  // respond using ZCL Default Response with a status response of NOT_FOUND.

  // The Price server shall send only the number of WeekProfiles belonging to
  // this calendar instance.  Server and clients shall be able to store at
  // least 4 WeekProfiles.  If the client is not able to store all entries, the
  // device should respond using ZCL Default Response with a status response of
  // INSUFFICIENT_SPACE.

  calendar = findCalendar(cmd_data.providerId, cmd_data.issuerCalendarId);
  if (calendar == NULL) {
    sl_zigbee_af_debug_println("ERR: Calendar not found: 0x%08X/0x%08X",
                               cmd_data.providerId,
                               cmd_data.issuerCalendarId);
    status = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  } else if (cmd_data.weekId == 0) {
    sl_zigbee_af_debug_println("ERR: Day id cannot be zero");
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else if (calendar->numberOfWeekProfiles < cmd_data.weekId) {
    sl_zigbee_af_debug_println("ERR: Invalid week: %d < %d",
                               calendar->numberOfWeekProfiles,
                               cmd_data.weekId);
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else if (SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_WEEK_PROFILES < cmd_data.weekId) {
    // This should never happen because of the previous check and the one in
    // PublishCalendar.  It is here for completeness.
    sl_zigbee_af_debug_println("ERR: Insufficient space for week: %d < %d",
                               SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_WEEK_PROFILES,
                               cmd_data.weekId);
    status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
  } else {
    sl_zigbee_af_calendar_week_profile_t *weekProfile = &calendar->weekProfiles[cmd_data.weekId - 1];
    weekProfile->inUse = true;
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    if (calendar->numberOfDayProfiles < cmd_data.dayIdRefMonday) {
      sl_zigbee_af_debug_println("ERR: Invalid %sday reference: %d < %d",
                                 "Mon",
                                 calendar->numberOfDayProfiles,
                                 cmd_data.dayIdRefMonday);
      status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    } else {
      weekProfile->dayIdRefMonday = cmd_data.dayIdRefMonday;
    }
    if (calendar->numberOfDayProfiles < cmd_data.dayIdRefTuesday) {
      sl_zigbee_af_debug_println("ERR: Invalid %sday reference: %d < %d",
                                 "Tues",
                                 calendar->numberOfDayProfiles,
                                 cmd_data.dayIdRefTuesday);
      status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    } else {
      weekProfile->dayIdRefTuesday = cmd_data.dayIdRefTuesday;
    }
    if (calendar->numberOfDayProfiles < cmd_data.dayIdRefWednesday) {
      sl_zigbee_af_debug_println("ERR: Invalid %sday reference: %d < %d",
                                 "Wednes",
                                 calendar->numberOfDayProfiles,
                                 cmd_data.dayIdRefWednesday);
      status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    } else {
      weekProfile->dayIdRefWednesday = cmd_data.dayIdRefWednesday;
    }
    if (calendar->numberOfDayProfiles < cmd_data.dayIdRefThursday) {
      sl_zigbee_af_debug_println("ERR: Invalid %sday reference: %d < %d",
                                 "Thurs",
                                 calendar->numberOfDayProfiles,
                                 cmd_data.dayIdRefThursday);
      status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    } else {
      weekProfile->dayIdRefThursday = cmd_data.dayIdRefThursday;
    }
    if (calendar->numberOfDayProfiles < cmd_data.dayIdRefFriday) {
      sl_zigbee_af_debug_println("ERR: Invalid %sday reference: %d < %d",
                                 "Fri",
                                 calendar->numberOfDayProfiles,
                                 cmd_data.dayIdRefFriday);
      status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    } else {
      weekProfile->dayIdRefFriday = cmd_data.dayIdRefFriday;
    }
    if (calendar->numberOfDayProfiles < cmd_data.dayIdRefSaturday) {
      sl_zigbee_af_debug_println("ERR: Invalid %sday reference: %d < %d",
                                 "Satur",
                                 calendar->numberOfDayProfiles,
                                 cmd_data.dayIdRefSaturday);
      status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    } else {
      weekProfile->dayIdRefSaturday = cmd_data.dayIdRefSaturday;
    }
    if (calendar->numberOfDayProfiles < cmd_data.dayIdRefSunday) {
      sl_zigbee_af_debug_println("ERR: Invalid %sday reference: %d < %d",
                                 "Sun",
                                 calendar->numberOfDayProfiles,
                                 cmd_data.dayIdRefSunday);
      status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    } else {
      weekProfile->dayIdRefSunday = cmd_data.dayIdRefSunday;
    }
  }

  if (calendar != NULL && status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_debug_println("ERR: Removing invalid calendar: 0x%08X/0x%08X",
                               calendar->providerId,
                               calendar->issuerCalendarId);
    removeCalendar(calendar);
  }

  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_calendar_cluster_publish_seasons_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_calendar_cluster_publish_seasons_command_t cmd_data;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  sl_zigbee_af_calendar_t *calendar;
  uint16_t seasonEntryCount;
  uint16_t seasonEntryIndex;

  if (zcl_decode_calendar_cluster_publish_seasons_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }

  seasonEntryCount = fieldLength(cmd_data.seasonEntries) / (sizeof(uint32_t) + sizeof(uint8_t));

  sl_zigbee_af_calendar_cluster_print("RX: PublishSeasons 0x%08X, 0x%08X, 0x%08X, %d, %d, [",
                                      cmd_data.providerId,
                                      cmd_data.issuerEventId,
                                      cmd_data.issuerCalendarId,
                                      cmd_data.commandIndex,
                                      cmd_data.totalNumberOfCommands);

  for (seasonEntryIndex = 0; seasonEntryIndex < seasonEntryCount; seasonEntryIndex++) {
    uint32_t seasonStartDate = cmd_data.seasonEntries[seasonEntryIndex * (sizeof(uint32_t) + sizeof(uint8_t))];
    uint8_t weekIdRef = cmd_data.seasonEntries[seasonEntryIndex * (sizeof(uint32_t) + sizeof(uint8_t)) + sizeof(uint32_t)];
    sl_zigbee_af_calendar_cluster_print("0x%08X, 0x%02X, ", seasonStartDate, weekIdRef);
  }
  sl_zigbee_af_calendar_cluster_println("]");

  // The PublishSeasons command is published in response to a GetSeason
  // command.  If the IssuerCalendarID does not match with one of the stored
  // calendar instances, the client shall ignore the command and respond using
  // ZCL Default Response with a status response of NOT_FOUND.

  // The Price server shall send only the number of SeasonEntries belonging to
  // this calendar instance.  Server and clients shall be able to store at
  // least 4 SeasonEntries.  If the client is not able to store all Season
  // Entries, the device should respond using ZCL Default Response with a
  // status response of INSUFFICIENT_SPACE.

  calendar = findCalendar(cmd_data.providerId, cmd_data.issuerCalendarId);
  if (calendar == NULL) {
    sl_zigbee_af_debug_println("ERR: Calendar not found: 0x%08X/0x%08X",
                               cmd_data.providerId,
                               cmd_data.issuerCalendarId);
    status = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  } else if (cmd_data.totalNumberOfCommands <= cmd_data.commandIndex) {
    sl_zigbee_af_debug_println("ERR: Inconsistent command index: %d <= %d",
                               cmd_data.totalNumberOfCommands,
                               cmd_data.commandIndex);
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else {
    uint16_t seasonEntriesLength = fieldLength(cmd_data.seasonEntries);
    uint16_t seasonEntriesIndex = 0;

    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    while (seasonEntriesIndex < seasonEntriesLength
           && status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      if (seasonEntriesLength < seasonEntriesIndex + 5) {
        sl_zigbee_af_debug_println("ERR: Malformed season");
        status = SL_ZIGBEE_ZCL_STATUS_MALFORMED_COMMAND;
      } else if (calendar->numberOfSeasons <= calendar->receivedSeasons) {
        sl_zigbee_af_debug_println("ERR: Inconsistent number of received seasons: %d <= %d",
                                   calendar->numberOfSeasons,
                                   calendar->receivedSeasons);
        status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
      } else if (SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_SEASONS
                 <= calendar->receivedSeasons) {
        // This should never happen because of the previous check and the one
        // in PublishCalendar.  It is here for completeness.
        sl_zigbee_af_debug_println("ERR: Insufficient space for received seasons: %d <= %d",
                                   SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_SEASONS,
                                   calendar->receivedSeasons);
        status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
      } else {
        sl_zigbee_af_calendar_season_t *season = &calendar->seasons[calendar->receivedSeasons++];
        seasonEntriesIndex += sl_zigbee_af_get_date(cmd_data.seasonEntries,
                                                    seasonEntriesIndex,
                                                    seasonEntriesLength,
                                                    &season->seasonStartDate);
        season->weekIdRef = sl_zigbee_af_get_int8u(cmd_data.seasonEntries,
                                                   seasonEntriesIndex,
                                                   seasonEntriesLength);
        seasonEntriesIndex++;
        if (season != calendar->seasons
            && sl_zigbee_af_compare_dates(&season->seasonStartDate, &((season - 1)->seasonStartDate)) <= 0) {
          sl_zigbee_af_debug_println("ERR: Inconsistent start dates: 0x%08X <= 0x%08X",
                                     season->seasonStartDate,
                                     (season - 1)->seasonStartDate);
          status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
        } else if (calendar->numberOfWeekProfiles < season->weekIdRef) {
          sl_zigbee_af_debug_println("ERR: Invalid week reference: %d < %d",
                                     calendar->numberOfWeekProfiles,
                                     season->weekIdRef);
          status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
        }
      }
    }
  }

  if (calendar != NULL && status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_debug_println("ERR: Removing invalid calendar: 0x%08X/0x%08X",
                               calendar->providerId,
                               calendar->issuerCalendarId);
    removeCalendar(calendar);
  }

  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_calendar_cluster_publish_special_days_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_calendar_cluster_publish_special_days_command_t cmd_data;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  sl_zigbee_af_calendar_t *calendar;

  if (zcl_decode_calendar_cluster_publish_special_days_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }

  sl_zigbee_af_calendar_cluster_print("RX: PublishSpecialDays 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%02X, %d, %d, %d, [",
                                      cmd_data.providerId,
                                      cmd_data.issuerEventId,
                                      cmd_data.issuerCalendarId,
                                      cmd_data.startTime,
                                      cmd_data.calendarType,
                                      cmd_data.totalNumberOfSpecialDays,
                                      cmd_data.commandIndex,
                                      cmd_data.totalNumberOfCommands);
  // TODO: print specialDayEntries
  sl_zigbee_af_calendar_cluster_println("]");

  // If the Calendar Type does not match with one of the stored calendar
  // instances, the client shall ignore the command and respond using ZCL
  // Default Response with a status response of NOT_FOUND.

  // Server and clients shall be able to store at least 50 SpecialDayEntries.
  // If the client is not able to store all SpecialDayEntries, the device
  // should respond using ZCL Default Response with a status response of
  // INSUFFICIENT_SPACE.

  // If the maximum application payload is not sufficient to transfer all
  // SpecialDayEntries in one command, the ESI may send as many
  // PublishSpecialDays commands as needed.
  // Note that, in this case, it is the client's responsibility to ensure that
  // it receives all associated PublishSpecialDays commands before any of the
  // payloads can be used.

  calendar = findCalendar(cmd_data.providerId, cmd_data.issuerCalendarId);
  if (calendar == NULL) {
    sl_zigbee_af_debug_println("ERR: Calendar not found: 0x%08X/0x%08X",
                               cmd_data.providerId,
                               cmd_data.issuerCalendarId);
    status = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  } else if (calendar->calendarType != cmd_data.calendarType) {
    sl_zigbee_af_debug_println("ERR: Inconsistent calendar type: 0x%02X != 0x%02X",
                               calendar->calendarType,
                               cmd_data.calendarType);
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else if (SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_SPECIAL_DAY_ENTRIES
             < cmd_data.totalNumberOfSpecialDays) {
    sl_zigbee_af_debug_println("ERR: Insufficient space for special day entries: %d < %d",
                               SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_SPECIAL_DAY_ENTRIES,
                               cmd_data.totalNumberOfSpecialDays);
    status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
  } else if (cmd_data.totalNumberOfCommands <= cmd_data.commandIndex) {
    sl_zigbee_af_debug_println("ERR: Inconsistent command index: %d <= %d",
                               cmd_data.totalNumberOfCommands,
                               cmd_data.commandIndex);
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else {
    uint16_t specialDayEntriesLength = fieldLength(cmd_data.specialDayEntries);
    uint16_t specialDayEntriesIndex = 0;

    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    if (!calendar->specialDayProfile.inUse) {
      calendar->specialDayProfile.inUse = true;
      calendar->specialDayProfile.startTimeUtc = cmd_data.startTime;
    }
    calendar->specialDayProfile.numberOfSpecialDayEntries = cmd_data.totalNumberOfSpecialDays;
    calendar->specialDayProfile.receivedSpecialDayEntries = 0;

    while (specialDayEntriesIndex < specialDayEntriesLength
           && status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      if (specialDayEntriesLength < specialDayEntriesIndex + 5) {
        sl_zigbee_af_debug_println("ERR: Malformed special day entry");
        status = SL_ZIGBEE_ZCL_STATUS_MALFORMED_COMMAND;
      } else if (calendar->specialDayProfile.numberOfSpecialDayEntries
                 <= calendar->specialDayProfile.receivedSpecialDayEntries) {
        sl_zigbee_af_debug_println("ERR: Inconsistent number of received special day entries: %d <= %d",
                                   calendar->specialDayProfile.numberOfSpecialDayEntries,
                                   calendar->specialDayProfile.receivedSpecialDayEntries);
        status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
      } else if (SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_SPECIAL_DAY_ENTRIES
                 <= calendar->specialDayProfile.receivedSpecialDayEntries) {
        // This should never happen because of the checks above.  It is here
        // for completeness.
        sl_zigbee_af_debug_println("ERR: Insufficient space for received special day entries: %d <= %d",
                                   SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_SPECIAL_DAY_ENTRIES,
                                   calendar->specialDayProfile.receivedSpecialDayEntries);
        status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
      } else {
        sl_zigbee_af_calendar_special_day_entry_t *specialDayEntry = &calendar->specialDayProfile.specialDayEntries[calendar->specialDayProfile.receivedSpecialDayEntries++];
        specialDayEntriesIndex += sl_zigbee_af_get_date(cmd_data.specialDayEntries,
                                                        specialDayEntriesIndex,
                                                        specialDayEntriesLength,
                                                        &specialDayEntry->specialDayDate);;
        specialDayEntry->dayIdRef = sl_zigbee_af_get_int8u(cmd_data.specialDayEntries,
                                                           specialDayEntriesIndex,
                                                           specialDayEntriesLength);
        specialDayEntriesIndex++;
        if (calendar->numberOfDayProfiles < specialDayEntry->dayIdRef) {
          sl_zigbee_af_debug_println("ERR: Invalid day reference: %d < %d",
                                     calendar->numberOfDayProfiles,
                                     specialDayEntry->dayIdRef);
          status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
        }
      }
    }
  }

  if (calendar != NULL && status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_debug_println("ERR: Removing invalid calendar: 0x%08X/0x%08X",
                               calendar->providerId,
                               calendar->issuerCalendarId);
    removeCalendar(calendar);
  }

  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_calendar_cluster_cancel_calendar_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_calendar_cluster_cancel_calendar_command_t cmd_data;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t index = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint,
                                                                  ZCL_CALENDAR_CLUSTER_ID);

  if (zcl_decode_calendar_cluster_cancel_calendar_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }

  sl_zigbee_af_calendar_cluster_println("RX: CancelCalendar 0x%08X, 0x%08X, 0x%02X",
                                        cmd_data.providerId,
                                        cmd_data.issuerCalendarId,
                                        cmd_data.calendarType);

  // The client device shall discard all instances of PublishCalendar,
  // PublishDayProfile, PublishWeekProfile, PublishSeasons, and
  // PublishSpecialDays commands associated with the stated Provider ID,
  // Calendar Type, and Issuer Calendar ID.

  if (SL_ZIGBEE_ZCL_CALENDAR_TYPE_AUXILLIARY_LOAD_SWITCH_CALENDAR < cmd_data.calendarType) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  } else {
    uint8_t i;
    status = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS; i++) {
      if (cmd_data.providerId == calendars[index][i].providerId
          && cmd_data.issuerCalendarId == calendars[index][i].issuerCalendarId
          && cmd_data.calendarType  == calendars[index][i].calendarType) {
        calendars[index][i].inUse = false;
        status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
      }
    }
  }

  return status;
}

uint8_t sl_zigbee_af_calendar_client_get_calendar_index_by_type(uint8_t endpoint, uint8_t calendarType)
{
  uint8_t i;
  uint8_t epIndex = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_CALENDAR_CLUSTER_ID);
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS; i++) {
    if ( calendars[epIndex][i].calendarType == calendarType ) {
      break;
    }
  }
  return i;
}

uint32_t sl_zigbee_af_calendar_client_get_calendar_id(uint8_t endpoint, uint8_t index)
{
  uint8_t epIndex = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_CALENDAR_CLUSTER_ID);
  if ( (epIndex == 0xFF) || (index >= SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS) ) {
    return SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_INVALID_CALENDAR_ID;
  } else {
    return calendars[epIndex][index].issuerCalendarId;
  }
}

static sl_zigbee_af_status_t addCalendar(sl_zigbee_af_calendar_t *calendar)
{
  uint32_t nowUtc = sl_zigbee_af_get_current_time();
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t index = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint,
                                                                  ZCL_CALENDAR_CLUSTER_ID);
  uint8_t i;

  if (calendar->startTimeUtc == 0) {
    calendar->startTimeUtc = nowUtc;
  }

  // Nested and overlapping calendars are not allowed.  In the case of
  // overlapping calendars of the same type (calendar type), the calendar with
  // the newer IssuerCalendarID takes priority over all nested and overlapping
  // calendars.  All existing calendar instances that overlap, even partially,
  // should be removed.  The only exception to this is if a calendar instance
  // with a newer Issuer Event ID overlaps with the end of the current active
  // calendar but is not yet active, then the active calendar is not deleted
  // but modified so that the active calendar ends when the new calendar
  // begins.

  // First, check that this calendar has a newer issuer calendar id than all
  // active calendars of the same type.  If it doesn't, then it overlaps, and
  // must be rejected.
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS; i++) {
    if (calendars[index][i].inUse
        && calendar->issuerEventId < calendars[index][i].issuerEventId
        && calendar->calendarType == calendars[index][i].calendarType) {
      sl_zigbee_af_debug_println("ERR: Overlaps with newer calendar: 0x%08X/0x%08X",
                                 calendars[index][i].providerId,
                                 calendars[index][i].issuerCalendarId);
      return SL_ZIGBEE_ZCL_STATUS_FAILURE;
    }
  }

  // We now know that this calendar is newer and that we're going to store it,
  // assuming we have room.  Before saving it, we need to check if it starts
  // before any existing calendars of the same type.  If so, it means those
  // calendars overlap with it and must be removed.
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS; i++) {
    if (calendars[index][i].inUse
        && calendar->startTimeUtc <= calendars[index][i].startTimeUtc
        && calendar->calendarType == calendars[index][i].calendarType) {
      sl_zigbee_af_debug_println("INFO: Overlaps with older calendar: 0x%08X/0x%08X",
                                 calendars[index][i].providerId,
                                 calendars[index][i].issuerCalendarId);
      removeCalendar(&calendars[index][i]);
    }
  }

  // Now, look for any calendars that started in the past and have been
  // superceded by a newer calendar.  These are stale and can be removed.
  // TODO: Maybe this should only be done if we don't have space to store the
  // new calendar?
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS; i++) {
    if (calendars[index][i].inUse
        && calendars[index][i].startTimeUtc < nowUtc) {
      uint8_t j;
      for (j = 0; j < SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS; j++) {
        if (i != j
            && calendars[index][j].inUse
            && (calendars[index][i].startTimeUtc
                < calendars[index][j].startTimeUtc)) {
          sl_zigbee_af_debug_println("INFO: Removing expired calendar: 0x%08X/0x%08X",
                                     calendars[index][i].providerId,
                                     calendars[index][i].issuerCalendarId);
          removeCalendar(&calendars[index][i]);
        }
      }
    }
  }

  // Look for an empty slot to save this calendar.  If we don't have room, we
  // have to drop it.
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS; i++) {
    if (!calendars[index][i].inUse) {
      memcpy(&calendars[index][i], calendar, sizeof(sl_zigbee_af_calendar_t));
      return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    }
  }

  sl_zigbee_af_debug_println("ERR: Insufficient space for calendar: 0x%08X/0x%08X",
                             calendar->providerId,
                             calendar->issuerCalendarId);
  return SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
}

// This function uses sl_zigbee_af_current_endpoint and therefore must only be called
// in the context of an incoming message.
static sl_zigbee_af_calendar_t *findCalendar(uint32_t providerId,
                                             uint32_t issuerCalendarId)
{
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t index = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint,
                                                                  ZCL_CALENDAR_CLUSTER_ID);
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS; i++) {
    if (calendars[index][i].inUse
        && calendars[index][i].providerId == providerId
        && calendars[index][i].issuerCalendarId == issuerCalendarId) {
      return &calendars[index][i];
    }
  }
  return NULL;
}

static void removeCalendar(sl_zigbee_af_calendar_t *calendar)
{
  sl_zigbee_af_debug_println("INFO: Removed calendar: 0x%08X/0x%08X",
                             calendar->providerId,
                             calendar->issuerCalendarId);
  calendar->inUse = false;
}

// plugin calendar-client clear <endpoint:1>
void sl_zigbee_af_calendar_client_clear_command(sl_cli_command_arg_t *arguments)
{
  uint8_t i;
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t index = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint,
                                                                  ZCL_CALENDAR_CLUSTER_ID);
  if (index == 0xFF) {
    return;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS; i++) {
    if (calendars[index][i].inUse) {
      removeCalendar(&calendars[index][i]);
    }
  }
}

// plugin calendar-client print <endpoint:1>
void sl_zigbee_af_calendar_client_print_command(sl_cli_command_arg_t *arguments)
{
  uint8_t i, j, k;
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t index = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint,
                                                                  ZCL_CALENDAR_CLUSTER_ID);
  if (index == 0xFF) {
    return;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS; i++) {
    const sl_zigbee_af_calendar_t *calendar = &calendars[index][i];
    if (!calendar->inUse) {
      continue;
    }

    sl_zigbee_af_calendar_cluster_println("calendar:                       %d", i);
    sl_zigbee_af_calendar_cluster_println("  providerId:                   0x%08X", calendar->providerId);
    sl_zigbee_af_calendar_cluster_println("  issuerEventId:                0x%08X", calendar->issuerEventId);
    sl_zigbee_af_calendar_cluster_println("  issuerCalendarId:             0x%08X", calendar->issuerCalendarId);
    sl_zigbee_af_calendar_cluster_println("  startTimeUtc:                 0x%08X", calendar->startTimeUtc);
    sl_zigbee_af_calendar_cluster_println("  calendarType:                 0x%02X", calendar->calendarType);
    sl_zigbee_af_calendar_cluster_print("  calendarName:                 \"");
    sl_zigbee_af_calendar_cluster_print_string(calendar->calendarName);
    sl_zigbee_af_calendar_cluster_println("\"");
    sl_zigbee_af_calendar_cluster_println("  numberOfSeasons:              %d", calendar->numberOfSeasons);
    sl_zigbee_af_calendar_cluster_println("  receivedSeasons:              %d", calendar->receivedSeasons);
    sl_zigbee_af_calendar_cluster_println("  numberOfWeekProfiles:         %d", calendar->numberOfWeekProfiles);
    sl_zigbee_af_calendar_cluster_println("  numberOfDayProfiles:          %d", calendar->numberOfDayProfiles);

    for (j = 0; j < calendar->receivedSeasons; j++) {
      const sl_zigbee_af_calendar_season_t *season = &calendar->seasons[j];
      sl_zigbee_af_calendar_cluster_println("  season:                       %d", i);
      sl_zigbee_af_calendar_cluster_println("    seasonStartDate:            0x%08X", season->seasonStartDate);
      sl_zigbee_af_calendar_cluster_println("    weekIdRef:                  %d", season->weekIdRef);
    }

    for (j = 0; j < calendar->numberOfWeekProfiles; j++) {
      const sl_zigbee_af_calendar_week_profile_t *weekProfile = &calendar->weekProfiles[j];
      if (weekProfile->inUse) {
        sl_zigbee_af_calendar_cluster_println("  weekProfile:                  %d", j);
        sl_zigbee_af_calendar_cluster_println("    weekId:                     %d", j + 1);
        sl_zigbee_af_calendar_cluster_println("    dayIdRefMonday:             %d", weekProfile->dayIdRefMonday);
        sl_zigbee_af_calendar_cluster_println("    dayIdRefTuesday:            %d", weekProfile->dayIdRefTuesday);
        sl_zigbee_af_calendar_cluster_println("    dayIdRefWednesday:          %d", weekProfile->dayIdRefWednesday);
        sl_zigbee_af_calendar_cluster_println("    dayIdRefThursday:           %d", weekProfile->dayIdRefThursday);
        sl_zigbee_af_calendar_cluster_println("    dayIdRefFriday:             %d", weekProfile->dayIdRefFriday);
        sl_zigbee_af_calendar_cluster_println("    dayIdRefSaturday:           %d", weekProfile->dayIdRefSaturday);
        sl_zigbee_af_calendar_cluster_println("    dayIdRefSunday:             %d", weekProfile->dayIdRefSunday);
      }
    }

    for (j = 0; j < calendar->numberOfDayProfiles; j++) {
      const sl_zigbee_af_calendar_day_profile_t *dayProfile = &calendar->dayProfiles[j];
      if (dayProfile->inUse) {
        sl_zigbee_af_calendar_cluster_println("  dayProfile:                   %d", j);
        sl_zigbee_af_calendar_cluster_println("    dayId:                      %d", j + 1);
        sl_zigbee_af_calendar_cluster_println("    numberOfScheduleEntries:    %d", dayProfile->numberOfScheduleEntries);
        sl_zigbee_af_calendar_cluster_println("    receivedScheduleEntries:    %d", dayProfile->receivedScheduleEntries);
        for (k = 0; k < dayProfile->receivedScheduleEntries; k++) {
          const sl_zigbee_af_calendar_schedule_entry_t *scheduleEntry = &dayProfile->scheduleEntries[k];
          sl_zigbee_af_calendar_cluster_println("    scheduleEntry:              %d", k);
          switch (calendar->calendarType) {
            case SL_ZIGBEE_ZCL_CALENDAR_TYPE_DELIVERED_CALENDAR:
            case SL_ZIGBEE_ZCL_CALENDAR_TYPE_RECEIVED_CALENDAR:
            case SL_ZIGBEE_ZCL_CALENDAR_TYPE_DELIVERED_AND_RECEIVED_CALENDAR:
              sl_zigbee_af_calendar_cluster_println("      startTimeM:               0x%04X", scheduleEntry->rateSwitchTime.startTimeM);
              sl_zigbee_af_calendar_cluster_println("      priceTier:                0x%02X", scheduleEntry->rateSwitchTime.priceTier);
              break;
            case SL_ZIGBEE_ZCL_CALENDAR_TYPE_FRIENDLY_CREDIT_CALENDAR:
              sl_zigbee_af_calendar_cluster_println("      startTimeM:               0x%04X", scheduleEntry->friendlyCreditSwitchTime.startTimeM);
              sl_zigbee_af_calendar_cluster_println("      friendlyCreditEnable:     %s", scheduleEntry->friendlyCreditSwitchTime.friendlyCreditEnable ? "true" : "false");
              break;
            case SL_ZIGBEE_ZCL_CALENDAR_TYPE_AUXILLIARY_LOAD_SWITCH_CALENDAR:
              sl_zigbee_af_calendar_cluster_println("      startTimeM:               0x%04X", scheduleEntry->auxilliaryLoadSwitchTime.startTimeM);
              sl_zigbee_af_calendar_cluster_println("      auxiliaryLoadSwitchState: 0x%02X", scheduleEntry->auxilliaryLoadSwitchTime.auxiliaryLoadSwitchState);
              break;
          }
        }
      }
    }

    {
      const sl_zigbee_af_calendar_special_day_profile_t *specialDayProfile = &calendar->specialDayProfile;
      if (specialDayProfile->inUse) {
        sl_zigbee_af_calendar_cluster_println("  specialDayProfile:");
        sl_zigbee_af_calendar_cluster_println("    startTimeUtc:               0x%08X", specialDayProfile->startTimeUtc);
        sl_zigbee_af_calendar_cluster_println("    numberOfSpecialDayEntries:  %d", specialDayProfile->numberOfSpecialDayEntries);
        sl_zigbee_af_calendar_cluster_println("    receivedSpecialDayEntries:  %d", specialDayProfile->receivedSpecialDayEntries);
        for (j = 0; j < specialDayProfile->receivedSpecialDayEntries; j++) {
          const sl_zigbee_af_calendar_special_day_entry_t *specialDayEntry = &specialDayProfile->specialDayEntries[j];
          sl_zigbee_af_calendar_cluster_println("    specialDayEntry:            %d", j);
          sl_zigbee_af_calendar_cluster_println("      specialDayDate:           0x%08X", specialDayEntry->specialDayDate);
          sl_zigbee_af_calendar_cluster_println("      dayIdRef:                 %d", specialDayEntry->dayIdRef);
        }
      }
    }
  }
}

uint32_t sl_zigbee_af_calendar_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                            sl_service_function_context_t *context)
{
  UNUSED_VAR(opcode);

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_PUBLISH_CALENDAR_COMMAND_ID:
      {
        status = sl_zigbee_af_calendar_cluster_publish_calendar_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_DAY_PROFILE_COMMAND_ID:
      {
        status = sl_zigbee_af_calendar_cluster_publish_day_profile_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_WEEK_PROFILE_COMMAND_ID:
      {
        status = sl_zigbee_af_calendar_cluster_publish_week_profile_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_SEASONS_COMMAND_ID:
      {
        status = sl_zigbee_af_calendar_cluster_publish_seasons_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_SPECIAL_DAYS_COMMAND_ID:
      {
        status = sl_zigbee_af_calendar_cluster_publish_special_days_cb(cmd);
        break;
      }
      case ZCL_CANCEL_CALENDAR_COMMAND_ID:
      {
        status = sl_zigbee_af_calendar_cluster_cancel_calendar_cb(cmd);
        break;
      }
    }
  }

  return status;
}
