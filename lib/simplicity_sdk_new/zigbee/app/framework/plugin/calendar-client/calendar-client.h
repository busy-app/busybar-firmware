/***************************************************************************/ /**
 * @file
 * @brief APIs for the Calendar Client plugin.
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

#include "calendar-client-config.h"

/**
 * @defgroup calendar-client Calendar Client
 * @ingroup component cluster
 * @brief API and Callbacks for the Calendar Cluster Client Component
 *
 * Silicon Labs implementation of the Calendar Cluster Client.
 * This component is capable of receiving calendars published by a server
 * but does not make requests on its own.
 *
 */

/**
 * @addtogroup calendar-client
 * @{
 */

#define SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_INVALID_CALENDAR_ID 0xFFFFFFFF
#define SL_ZIGBEE_AF_CALENDAR_MAXIMUM_CALENDAR_NAME_LENGTH 12

typedef struct {
  sl_zigbee_af_date_t seasonStartDate;
  uint8_t weekIdRef;
} sl_zigbee_af_calendar_season_t;

typedef struct {
  bool inUse;
  uint8_t dayIdRefMonday;
  uint8_t dayIdRefTuesday;
  uint8_t dayIdRefWednesday;
  uint8_t dayIdRefThursday;
  uint8_t dayIdRefFriday;
  uint8_t dayIdRefSaturday;
  uint8_t dayIdRefSunday;
} sl_zigbee_af_calendar_week_profile_t;

// All valid calendar types have the same general format for schedule entries:
// a two-byte start time followed by a one-byte, type-specific value. The code
// in this plugin takes advantage of this similarity to simplify the logic. If
// new types are added, the code will need to change. See
// sl_zigbee_af_calendar_cluster_publish_day_profile_cb.
typedef union {
  struct {
    uint16_t startTimeM;
    uint8_t priceTier;
  } rateSwitchTime;
  struct {
    uint16_t startTimeM;
    bool friendlyCreditEnable;
  } friendlyCreditSwitchTime;
  struct {
    uint16_t startTimeM;
    uint8_t auxiliaryLoadSwitchState;
  } auxilliaryLoadSwitchTime;
} sl_zigbee_af_calendar_schedule_entry_t;

typedef struct {
  bool inUse;
  uint8_t numberOfScheduleEntries;
  uint8_t receivedScheduleEntries;
  sl_zigbee_af_calendar_schedule_entry_t scheduleEntries[SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_SCHEDULE_ENTRIES];
} sl_zigbee_af_calendar_day_profile_t;

typedef struct {
  sl_zigbee_af_date_t specialDayDate;
  uint8_t dayIdRef;
} sl_zigbee_af_calendar_special_day_entry_t;

typedef struct {
  bool inUse;
  uint32_t startTimeUtc;
  uint8_t numberOfSpecialDayEntries;
  uint8_t receivedSpecialDayEntries;
  sl_zigbee_af_calendar_special_day_entry_t specialDayEntries[SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_SPECIAL_DAY_ENTRIES];
} sl_zigbee_af_calendar_special_day_profile_t;

typedef struct {
  bool inUse;
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t issuerCalendarId;
  uint32_t startTimeUtc;
  sl_zigbee_af_calendar_type_t calendarType;
  uint8_t calendarName[SL_ZIGBEE_AF_CALENDAR_MAXIMUM_CALENDAR_NAME_LENGTH + 1];
  uint8_t numberOfSeasons;
  uint8_t receivedSeasons;
  uint8_t numberOfWeekProfiles;
  uint8_t numberOfDayProfiles;
  sl_zigbee_af_calendar_season_t seasons[SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_SEASONS];
  sl_zigbee_af_calendar_week_profile_t weekProfiles[SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_WEEK_PROFILES];
  sl_zigbee_af_calendar_day_profile_t dayProfiles[SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_DAY_PROFILES];
  sl_zigbee_af_calendar_special_day_profile_t specialDayProfile;
} sl_zigbee_af_calendar_t;

/**
 * @name API
 * @{
 */

/**
 * @brief Get the first calendar index based on the calendar type.
 *
 * @param endpoint The relevant endpoint.
 * @param calendarType The type of calendar that should be searched for in the table.
 * @return The index of the first matching calendar, or SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS
 * if a match cannot be found.
 *
 **/
uint8_t sl_zigbee_af_calendar_client_get_calendar_index_by_type(uint8_t endpoint, uint8_t calendarType);

/**
 * @brief Get the calendar ID at the specified index.
 *
 * @param endpoint The relevant endpoint.
 * @param index The index in the calendar table whose calendar ID should be returned.
 * @return The calendar ID of the calendar at the specified index. If index is
 * out of bounds, or if a match cannot be found,
 * SL_ZIGBEE_AF_PLUGIN_CALENDAR_CLIENT_INVALID_CALENDAR_ID will be returned.
 *
 **/
uint32_t sl_zigbee_af_calendar_client_get_calendar_id(uint8_t endpoint, uint8_t index);

/** @} */ // end of name API
/** @} */ // end of calendar-client
