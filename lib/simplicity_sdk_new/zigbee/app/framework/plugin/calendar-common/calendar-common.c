/***************************************************************************//**
 * @file
 * @brief CLI for the Calendar Common plugin.
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
#include "calendar-common.h"

//-----------------------------------------------------------------------------
// Globals

sl_zigbee_af_calendar_struct_t calendars[SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS];

/*
 * From GBCS V0.8.1 section 10.4.2.11.
 *
 * When processing a command where Issuer Calendar ID has the value 0xFFFFFFFF
 * or 0xFFFFFFFE, a GPF or GSME shall interpret 0xFFFFFFFF as meaning the
 * currently in force Tariff Switching Table calendar and 0xFFFFFFFE as meaning
 * the currently in force Non-Disablement Calendar
 */
#if defined(SL_CATALOG_ZIGBEE_GAS_PROXY_FUNCTION_PRESENT)
uint32_t tariffSwitchingCalendarId = SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_INVALID_CALENDAR_ID;
uint32_t nonDisablementCalendarId  = SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_INVALID_CALENDAR_ID;
#endif

//-----------------------------------------------------------------------------

// Templated to the "local_data_init" context.
void sl_zigbee_af_calendar_common_init_cb(uint8_t init_level)
{
  (void)init_level;

  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    memset(&(calendars[i]), 0, sizeof(sl_zigbee_af_calendar_struct_t));
    calendars[i].calendarId = SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_INVALID_CALENDAR_ID;
  }
}

uint8_t sl_zigbee_af_calendar_common_get_calendar_by_id(uint32_t calendarId,
                                                        uint32_t providerId)
{
  uint8_t i;

#if defined(SL_CATALOG_ZIGBEE_GAS_PROXY_FUNCTION_PRESENT)
  if (calendarId == GBCS_TARIFF_SWITCHING_CALENDAR_ID) {
    calendarId = tariffSwitchingCalendarId;
  } else if (calendarId == GBCS_NON_DISABLEMENT_CALENDAR_ID) {
    calendarId = nonDisablementCalendarId;
  }
#endif

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    if (calendarId == calendars[i].calendarId
        && (providerId == SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_PROVIDER_ID
            || providerId == calendars[i].providerId)) {
      return i;
    }
  }
  return SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX;
}

static sl_zigbee_af_calendar_struct_t * findCalendarByCalId(uint32_t issuerCalendarId)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    sl_zigbee_af_calendar_struct_t * cal = &calendars[i];
    if (cal->calendarId == issuerCalendarId) {
      return cal;
    }
  }
  sl_zigbee_af_calendar_cluster_println("ERR: Unable to find calendar with id(0x%08X) ", issuerCalendarId);
  return NULL;
}

uint32_t sl_zigbee_af_calendar_common_end_time_utc(const sl_zigbee_af_calendar_struct_t *calendar)
{
  uint32_t endTimeUtc = MAX_INT32U_VALUE;
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    if (calendar->providerId == calendars[i].providerId
        && calendar->issuerEventId < calendars[i].issuerEventId
        && calendar->startTimeUtc < calendars[i].startTimeUtc
        && calendar->calendarType == calendars[i].calendarType
        && calendars[i].startTimeUtc < endTimeUtc) {
      endTimeUtc = calendars[i].startTimeUtc;
    }
  }

  return endTimeUtc;
}

bool sl_zigbee_af_calendar_common_set_cal_info(uint8_t index,
                                               uint32_t providerId,
                                               uint32_t issuerEventId,
                                               uint32_t issuerCalendarId,
                                               uint32_t startTimeUtc,
                                               uint8_t calendarType,
                                               uint8_t *calendarName,
                                               uint8_t numberOfSeasons,
                                               uint8_t numberOfWeekProfiles,
                                               uint8_t numberOfDayProfiles)
{
  sl_zigbee_af_calendar_struct_t * cal;
  if (index >= SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS) {
    sl_zigbee_af_calendar_cluster_println("Index must be in the range of 0 to %d", SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS - 1);
    return false;
  }

  if (calendarName == NULL || calendarName[0] > SL_ZIGBEE_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH) {
    sl_zigbee_af_calendar_cluster_println("Invalid calendar name!");
    return false;
  }

  cal = &calendars[index];
  cal->providerId = providerId;
  cal->issuerEventId = issuerEventId;
  cal->calendarId = issuerCalendarId;
  cal->startTimeUtc = startTimeUtc;
  cal->calendarType = calendarType;
  sl_zigbee_af_copy_string(cal->name, calendarName, SL_ZIGBEE_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH);
  cal->numberOfSeasons = numberOfSeasons;
  cal->numberOfWeekProfiles = numberOfWeekProfiles;
  cal->numberOfDayProfiles = numberOfDayProfiles;
  cal->flags = 0;
  return true;
}

//
/*
 * Note: Nested and overlapping calendars are not allowed. SE specifications
 * elaborate more on the details in PublishCalendar Command description.
 */
//
bool sl_zigbee_af_calendar_common_add_cal_info(uint32_t providerId,
                                               uint32_t issuerEventId,
                                               uint32_t issuerCalendarId,
                                               uint32_t startTimeUtc,
                                               uint8_t calendarType,
                                               uint8_t *calendarName,
                                               uint8_t numberOfSeasons,
                                               uint8_t numberOfWeekProfiles,
                                               uint8_t numberOfDayProfiles)
{
  bool status = false;
  uint8_t i = 0;
  uint8_t matchingEntryIndex = 0xFF;
  uint8_t smallestEventIdEntryIndex = 0x0;
  sl_zigbee_af_calendar_struct_t * cal;

  // Try to find an existing entry to overwrite.
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    sl_zigbee_af_calendar_struct_t * cal = &calendars[i];
    if (cal->providerId == providerId
        && cal->calendarId == issuerCalendarId) {
      matchingEntryIndex = i;
    }

    if (cal->issuerEventId < calendars[smallestEventIdEntryIndex].issuerEventId) {
      smallestEventIdEntryIndex = i;
    }
  }

  if (matchingEntryIndex == 0xFF) {
    i = smallestEventIdEntryIndex;
  } else {
    i = matchingEntryIndex;
  }

  cal = &calendars[i];

  if (numberOfSeasons > SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SEASON_PROFILE_MAX) {
    sl_zigbee_af_calendar_cluster_println("ERR: Insufficient space for requested number of seasons(%d)!", numberOfSeasons);
    status = false;
    goto kickout;
  }
  if (numberOfWeekProfiles > SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX) {
    sl_zigbee_af_calendar_cluster_println("ERR: Insufficient space for requested number of week profiles(%d)!", numberOfWeekProfiles);
    status = false;
    goto kickout;
  }
  if (numberOfDayProfiles > SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX) {
    sl_zigbee_af_calendar_cluster_println("ERR: Insufficient space for requested number of day profiles(%d)!", numberOfDayProfiles);
    status = false;
    goto kickout;
  }

  // Calendars must be replaced as a whole.
  memset(cal, 0x00, sizeof(sl_zigbee_af_calendar_struct_t));
  cal->providerId = providerId;
  cal->issuerEventId = issuerEventId;
  cal->calendarId = issuerCalendarId;
  cal->startTimeUtc = startTimeUtc;
  cal->calendarType = calendarType;
  sl_zigbee_af_copy_string(cal->name, calendarName, SL_ZIGBEE_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH);
  cal->numberOfSeasons = numberOfSeasons;
  cal->numberOfWeekProfiles = numberOfWeekProfiles;
  cal->numberOfDayProfiles = numberOfDayProfiles;
  cal->flags = 0;
  status = true;

#if defined(SL_CATALOG_ZIGBEE_GAS_PROXY_FUNCTION_PRESENT)
  if (cal->calendarType == SL_ZIGBEE_ZCL_CALENDAR_TYPE_DELIVERED_CALENDAR) {
    tariffSwitchingCalendarId = cal->calendarId;
  } else if (cal->calendarType == SL_ZIGBEE_ZCL_CALENDAR_TYPE_FRIENDLY_CREDIT_CALENDAR) {
    nonDisablementCalendarId = cal->calendarId;
  }
#endif

  kickout:
  return status;
}

bool sl_zigbee_af_calendar_common_add_special_days_info(uint32_t issuerCalendarId,
                                                        uint8_t totalNumberOfSpecialDays,
                                                        uint8_t * specialDaysEntries,
                                                        uint16_t specialDaysEntriesLength,
                                                        uint8_t * unknownSpecialDaysMask)
{
  sl_zigbee_af_calendar_struct_t * cal = findCalendarByCalId(issuerCalendarId);
  bool status = false;
  uint8_t numberOfSpecialDaysEntries = specialDaysEntriesLength / SPECIAL_DAY_ENTRY_SIZE;
  uint8_t i;

  *unknownSpecialDaysMask = 0;

  if (cal == NULL) {
    status = false;
    goto kickout;
  }

  if (totalNumberOfSpecialDays > SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SPECIAL_DAY_PROFILE_MAX) {
    sl_zigbee_af_calendar_cluster_println("ERR: %d number of special days being added is above the current maximum(%d).",
                                          totalNumberOfSpecialDays,
                                          SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SPECIAL_DAY_PROFILE_MAX);
    status = false;
    goto kickout;
  }

  if (totalNumberOfSpecialDays != numberOfSpecialDaysEntries) {
    sl_zigbee_af_calendar_cluster_println("ERR: adding special days with inconsistent information.");
    status = false;
    goto kickout;
  }

  if ((specialDaysEntries == NULL) || (numberOfSpecialDaysEntries == 0)) {
    status = false;
    sl_zigbee_af_calendar_cluster_println("ERR: Unable to add special days.");
    goto kickout;
  }

  for (i = 0; i < numberOfSpecialDaysEntries; i++) {
    sl_zigbee_af_date_t startDate = { 0 };
    uint8_t normalDayId;

    sl_zigbee_af_get_date(specialDaysEntries,
                          SPECIAL_DAY_ENTRY_SIZE * i,
                          specialDaysEntriesLength,
                          &startDate);
    normalDayId = sl_zigbee_af_get_int8u(specialDaysEntries,
                                         SPECIAL_DAY_ENTRY_SIZE * i + sizeof(sl_zigbee_af_date_t),
                                         specialDaysEntriesLength);
    if (cal->numberOfSpecialDayProfiles < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SPECIAL_DAY_PROFILE_MAX) {
      uint8_t normalDayIndex;
      bool update = true;

      // find corresponding normal day index
      for (normalDayIndex = 0; normalDayIndex < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX; normalDayIndex++) {
        if (cal->normalDays[normalDayIndex].id == normalDayId) {
          uint8_t index;
          // check for redundant specialDays
          for (index = 0; index < cal->numberOfSpecialDayProfiles; index++) {
            sl_zigbee_af_calendar_special_day_struct_t * specialDay = &cal->specialDays[index];
            if ((specialDay->normalDayIndex == normalDayIndex)
                && (sl_zigbee_af_compare_dates(&specialDay->startDate, &startDate) == 0)) {
              update = false;
            }
          }

          break;
        }
      }

      if (normalDayIndex >= SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX) {
        // The normal day ID is unknown, so do not publish this special
        // day. Set a bit in the unknown day mask to alert the caller
        // of this function. By not incrementing numberOfSpecialDayProfiles,
        // the calendar will not be updated with this special day.
        *unknownSpecialDaysMask |= BIT(i);
      } else if (update) {
        cal->specialDays[cal->numberOfSpecialDayProfiles].normalDayIndex = normalDayIndex;
        cal->specialDays[cal->numberOfSpecialDayProfiles].flags = 0;
        cal->specialDays[cal->numberOfSpecialDayProfiles].startDate = startDate;

        sl_zigbee_af_calendar_cluster_println("Updated: Calendar(calId=0x%08X)",
                                              cal->calendarId);
        sl_zigbee_af_calendar_cluster_println("         SpecialDays[%d]", cal->numberOfSpecialDayProfiles);
        sl_zigbee_af_calendar_cluster_print("           startDate: ");
        sl_zigbee_af_print_dateln(&cal->specialDays[cal->numberOfSpecialDayProfiles].startDate);
        sl_zigbee_af_calendar_cluster_println("           normalDayIndex: %d", cal->specialDays[cal->numberOfSpecialDayProfiles].normalDayIndex);
        cal->numberOfSpecialDayProfiles++;
      } else {
        sl_zigbee_af_calendar_cluster_println("ERR: Invalid dayId! Unable to store SpecialDays info.");
      }
    } else {
      sl_zigbee_af_calendar_cluster_println("ERR: Insufficient space to store more SpecialDays info!");
    }
  }
  status = (*unknownSpecialDaysMask == 0 ? true : false);

  kickout:
  return status;
}

bool sl_zigbee_af_calendar_common_add_day_prof_info(uint32_t issuerCalendarId,
                                                    uint8_t dayId,
                                                    uint8_t * dayScheduleEntries,
                                                    uint16_t dayScheduleEntriesLength)
{
  sl_zigbee_af_calendar_struct_t * cal = findCalendarByCalId(issuerCalendarId);
  bool status = false;
  uint8_t numberOfScheduledEntries = dayScheduleEntriesLength / SCHEDULE_ENTRY_SIZE;
  uint8_t scheduleEntryIndex = 0;

  if (cal == NULL) {
    sl_zigbee_af_calendar_cluster_println("ERR: Unable to find calendar with id(0x%08X)",
                                          issuerCalendarId);
    status = false;
    goto kickout;
  }

  if ((dayScheduleEntries == NULL)
      || (dayScheduleEntriesLength == 0)
      || ((dayScheduleEntriesLength % SCHEDULE_ENTRY_SIZE) != 0) // each struct occupy 3 bytes.
      || (cal->numberOfReceivedDayProfiles >= SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX)
      || (numberOfScheduledEntries > SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SCHEDULE_ENTRIES_MAX)) {
    status = false;
    sl_zigbee_af_calendar_cluster_println("ERR: Unable to add DayProfile");
    goto kickout;
  }

  sl_zigbee_af_calendar_cluster_println("Updated: DayProfile[%d]",
                                        cal->numberOfReceivedDayProfiles);
  sl_zigbee_af_calendar_cluster_println("           DayId=%d",
                                        dayId);

  cal->normalDays[cal->numberOfReceivedDayProfiles].id = dayId;
  cal->normalDays[cal->numberOfReceivedDayProfiles].numberOfScheduleEntries = numberOfScheduledEntries;

  for (scheduleEntryIndex = 0; scheduleEntryIndex < numberOfScheduledEntries; scheduleEntryIndex++) {
    uint8_t priceTier;
    uint16_t minutesFromMidnight;
    sl_zigbee_af_calendar_day_schedule_entry_struct_t * normalDay;
    normalDay = &cal->normalDays[cal->numberOfReceivedDayProfiles].scheduleEntries[scheduleEntryIndex];
    minutesFromMidnight = sl_zigbee_af_get_int16u(dayScheduleEntries,
                                                  scheduleEntryIndex * SCHEDULE_ENTRY_SIZE,
                                                  dayScheduleEntriesLength);
    priceTier = sl_zigbee_af_get_int8u(dayScheduleEntries,
                                       scheduleEntryIndex * SCHEDULE_ENTRY_SIZE + 2,
                                       dayScheduleEntriesLength);
    normalDay->minutesFromMidnight =  minutesFromMidnight;
    normalDay->data =  priceTier;
    sl_zigbee_af_calendar_cluster_println("           ScheduledEntries[%d]",
                                          scheduleEntryIndex);
    sl_zigbee_af_calendar_cluster_println("             startTime: 0x%04X from midnight",
                                          minutesFromMidnight);
    sl_zigbee_af_calendar_cluster_println("             price tier: 0x%02X",
                                          priceTier);
  }

  cal->numberOfReceivedDayProfiles++;
  status = true;

  kickout:
  return status;
}

bool sl_zigbee_af_calendar_common_set_day_prof_info(uint8_t index,
                                                    uint8_t dayId,
                                                    uint8_t entryId,
                                                    uint16_t minutesFromMidnight,
                                                    uint8_t data)
{
  if (index >= SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS) {
    sl_zigbee_af_calendar_cluster_println("Index must be in the range of 0 to %d", SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS - 1);
    return false;
  } else if (dayId >= SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX) {
    sl_zigbee_af_calendar_cluster_println("DayId must be in the range of 0 to %d", SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX - 1);
    return false;
  } else if (entryId >= SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SCHEDULE_ENTRIES_MAX) {
    sl_zigbee_af_calendar_cluster_println("EntryId must be in the range of 0 to %d", SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SCHEDULE_ENTRIES_MAX - 1);
    return false;
  }

  calendars[index].normalDays[dayId].scheduleEntries[entryId].minutesFromMidnight =  minutesFromMidnight;
  calendars[index].normalDays[dayId].scheduleEntries[entryId].data = data;
  return true;
}

bool sl_zigbee_af_calendar_server_add_week_prof_info(uint32_t issuerCalendarId,
                                                     uint8_t weekId,
                                                     uint8_t dayIdRefMon,
                                                     uint8_t dayIdRefTue,
                                                     uint8_t dayIdRefWed,
                                                     uint8_t dayIdRefThu,
                                                     uint8_t dayIdRefFri,
                                                     uint8_t dayIdRefSat,
                                                     uint8_t dayIdRefSun)
{
  sl_zigbee_af_calendar_struct_t * cal = findCalendarByCalId(issuerCalendarId);
  uint8_t dayIdRefs[7];
  uint8_t dayCount = 7;
  uint8_t * normalDayIndexes;
  sl_zigbee_af_calendar_day_struct_t * normalDays;
  uint8_t dayIdRefsIndex;

  if ((cal == NULL)
      || (cal->numberOfReceivedWeekProfiles >= SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX)) {
    return false;
  }

  dayIdRefs[0] = dayIdRefMon;
  dayIdRefs[1] = dayIdRefTue;
  dayIdRefs[2] = dayIdRefWed;
  dayIdRefs[3] = dayIdRefThu;
  dayIdRefs[4] = dayIdRefFri;
  dayIdRefs[5] = dayIdRefSat;
  dayIdRefs[6] = dayIdRefSun;
  normalDays = (sl_zigbee_af_calendar_day_struct_t *)&cal->normalDays;
  normalDayIndexes = (uint8_t *)&cal->weeks[cal->numberOfReceivedWeekProfiles].normalDayIndexes;

  cal->weeks[cal->numberOfReceivedWeekProfiles].id = weekId;
  sl_zigbee_af_calendar_cluster_println("Updated: WeekProfile[%d]",
                                        cal->numberOfReceivedWeekProfiles);
  sl_zigbee_af_calendar_cluster_println("           weekId=%d", weekId);

  for (dayIdRefsIndex = 0; dayIdRefsIndex < dayCount; dayIdRefsIndex++) {
    uint8_t normalDayIndex;
    uint8_t dayId = dayIdRefs[dayIdRefsIndex];
    for (normalDayIndex = 0; normalDayIndex < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX; normalDayIndex++) {
      if (normalDays[normalDayIndex].id == dayId) {
        normalDayIndexes[dayIdRefsIndex] = normalDayIndex;
        sl_zigbee_af_calendar_cluster_println("           normalDayIndexes[%d]=%d",
                                              dayIdRefsIndex,
                                              normalDayIndex);
      }
    }
  }
  cal->numberOfReceivedWeekProfiles++;
  return true;
}

bool sl_zigbee_af_calendar_server_set_week_prof_info(uint8_t index,
                                                     uint8_t weekId,
                                                     uint8_t dayIdRefMon,
                                                     uint8_t dayIdRefTue,
                                                     uint8_t dayIdRefWed,
                                                     uint8_t dayIdRefThu,
                                                     uint8_t dayIdRefFri,
                                                     uint8_t dayIdRefSat,
                                                     uint8_t dayIdRefSun)
{
  if (index >= SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS) {
    sl_zigbee_af_calendar_cluster_println("Index must be in the range of 0 to %d", SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS - 1);
    return false;
  } else if (weekId >= SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX) {
    sl_zigbee_af_calendar_cluster_println("WeekId must be in the range of 0 to %d", SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX - 1);
    return false;
  }

  calendars[index].weeks[weekId].normalDayIndexes[0] = dayIdRefMon;
  calendars[index].weeks[weekId].normalDayIndexes[1] = dayIdRefTue;
  calendars[index].weeks[weekId].normalDayIndexes[2] = dayIdRefWed;
  calendars[index].weeks[weekId].normalDayIndexes[3] = dayIdRefThu;
  calendars[index].weeks[weekId].normalDayIndexes[4] = dayIdRefFri;
  calendars[index].weeks[weekId].normalDayIndexes[5] = dayIdRefSat;
  calendars[index].weeks[weekId].normalDayIndexes[6] = dayIdRefSun;
  return true;
}

bool sl_zigbee_af_calendar_server_add_seasons_info(uint32_t issuerCalendarId,
                                                   uint8_t * seasonsEntries,
                                                   uint8_t seasonsEntriesLength,
                                                   uint8_t * unknownWeekIdSeasonsMask)
{
  bool status = false;
  sl_zigbee_af_calendar_struct_t * cal = findCalendarByCalId(issuerCalendarId);
  uint8_t seasonEntryCount = seasonsEntriesLength / SEASON_ENTRY_SIZE;
  uint8_t seasonEntryIndex;

  *unknownWeekIdSeasonsMask = 0;

  if (cal == NULL) {
    status = false;
    goto kickout;
  }

  if (cal->numberOfReceivedSeasons + seasonEntryCount > cal->numberOfSeasons) {
    status = false;
    goto kickout;
  }

  for (seasonEntryIndex = 0; seasonEntryIndex < seasonEntryCount; seasonEntryIndex++) {
    sl_zigbee_af_calendar_season_struct_t  * season = &cal->seasons[cal->numberOfReceivedSeasons];
    sl_zigbee_af_date_t startDate;
    uint8_t weekId;
    sl_zigbee_af_get_date(seasonsEntries,
                          seasonEntryIndex * SEASON_ENTRY_SIZE,
                          seasonsEntriesLength,
                          &startDate);
    weekId = sl_zigbee_af_get_int8u(seasonsEntries,
                                    seasonEntryIndex * SEASON_ENTRY_SIZE + sizeof(sl_zigbee_af_date_t),
                                    seasonsEntriesLength);
    season->startDate = startDate;
    sl_zigbee_af_calendar_cluster_println("Updated: Seasons[%d]", cal->numberOfReceivedSeasons);
    sl_zigbee_af_calendar_cluster_print("            startDate: ");
    sl_zigbee_af_print_dateln(&startDate);
    {
      // search for week index.
      uint8_t i;
      season->weekIndex = 0xFF;
      for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX; i++) {
        if (cal->weeks[i].id == weekId) {
          season->weekIndex = i;
        }
      }
    }

    sl_zigbee_af_calendar_cluster_println("            weekIndex: %d", season->weekIndex);

    if (season->weekIndex != 0xFF) {
      cal->numberOfReceivedSeasons++;
    } else {
      // Do not increment numberOfReceivedSeasons, and overwrite
      // this seasonEntryIndex with the next seasonEntry.
      *unknownWeekIdSeasonsMask |= BIT(seasonEntryIndex);
    }
  }
  status = (*unknownWeekIdSeasonsMask == 0 ? true : false);

  kickout:
  return status;
}

bool sl_zigbee_af_calendar_server_set_seasons_info(uint8_t index,
                                                   uint8_t seasonId,
                                                   sl_zigbee_af_date_t startDate,
                                                   uint8_t weekIdRef)
{
  if (index >= SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS) {
    sl_zigbee_af_calendar_cluster_println("Index must be in the range of 0 to %d", SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS - 1);
    return false;
  } else if (seasonId >= SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SEASON_PROFILE_MAX) {
    sl_zigbee_af_calendar_cluster_println("SeasonId must be in the range of 0 to %d", SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SEASON_PROFILE_MAX - 1);
    return false;
  }

  calendars[index].seasons[seasonId].startDate = startDate;
  calendars[index].seasons[seasonId].weekIndex = weekIdRef;
  return true;
}
