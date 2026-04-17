/***************************************************************************//**
 * @file
 * @brief APIs and defines for the Door Lock Server plugin.
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

/**
 * @defgroup door-lock-server Door Lock Server
 * @ingroup component cluster
 * @brief API and Callbacks for the Door Lock Cluster Server Component.
 *
 * Silicon Labs implementation of the Door Lock server cluster.
 * This is an incomplete component implementation. It does all
 * of the mandatory and optional behavior in the Door Lock cluster
 * needed to pass the Door Lock cluster tests. It does not
 * store user and schedule tables in persistent memory, check user
 * pin activation against user status or user schedule, or offer
 * callbacks and component options to the application for further
 * customization of functionality.
 *
 */

/**
 * @addtogroup door-lock-server
 * @{
 */

// ------------------------------------------------------------------------------
// Core

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct {
  sl_zigbee_af_attribute_id_t id;
  uint16_t value;
} sli_zigbee_af_door_lock_server_attribute_data;

/** @brief Activate Door Lock.
 *
 *
 * @param[in] activate True if the lock should move to the locked position, false
 * if it should move to the unlocked position.
 * 
 * @return bool true if the command was handled by the application; false otherwise.

 */
bool sl_zigbee_af_door_lock_server_activate_door_lock_cb(bool activate);

// For each of the provided attribute ID/value pairs, write the attribute to the
// value on the DOOR_LOCK_SERVER_ENDPOINT. If this function encounters a failure,
// it will print something out. The description parameter is a string that
// describes the type of attributes that are being written.
void sli_zigbee_af_door_lock_server_write_attributes(const sli_zigbee_af_door_lock_server_attribute_data *data,
                                                     uint8_t dataLength,
                                                     const char *description);

// This function should be called when the door state has changed. A status
// describing the success or failure of the update will be returned.
sl_zigbee_af_status_t sli_zigbee_af_door_lock_server_note_door_state_changed(sl_zigbee_af_door_state_t state);
#endif

// At boot, the NumberOfPINUsersSupported attribute will be written to this
// value.
// Note: the DOOR_LOCK_USER_TABLE_SIZE symbol is respected because it
// was used originally as a configuration value for this plugin.
#ifdef DOOR_LOCK_USER_TABLE_SIZE
  #define SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE DOOR_LOCK_USER_TABLE_SIZE
#else
  #define SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE 8
#endif

/** @brief At boot, the NumberOfRFIDUsersSupported attribute will be written to this value. */
#define SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE 8

// This value should reflect the value of the MaxPINCodeLength attribute.
// Note: the DOOR_LOCK_MAX_PIN_LENGTH symbol is respected because it was used
// originally as a configuration value for this plugin.
#ifdef DOOR_LOCK_MAX_PIN_LENGTH
  #define SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_PIN_LENGTH DOOR_LOCK_MAX_PIN_LENGTH
#else
  #define SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_PIN_LENGTH 8
#endif

/** @brief This value should reflect the value of the MaxRFIDCodeLength attribute. */
#define SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_RFID_LENGTH 8

// At boot, the NumberOfWeekDaySchedulesSupportedPerUser attribute will be written
// to this value.
// Note: the DOOR_LOCK_SCHEDULE_TABLE_SIZE symbol is respected because it was
// used originally as a configuration value for this plugin.
// Also note: technically, this is the _total_ number of weekday schedules that
// can be stored across all users.
#ifdef DOOR_LOCK_SCHEDULE_TABLE_SIZE
  #define SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE DOOR_LOCK_SCHEDULE_TABLE_SIZE
#else
  #define SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE 4
#endif

/** @brief At boot, the NumberOfYearDaySchedulesSupportedPerUser attribute will be written to this value. */
/** @note Technically, this is the _total_ number of year day schedules that can be stored across all users. */
 #define SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE 8

/** @brief At boot, the NumberOfHolidaySchedulesSupported attribute will be written to this value. */
#define SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_HOLIDAY_SCHEDULE_TABLE_SIZE 8

#ifndef DOOR_LOCK_SERVER_ENDPOINT
  #define DOOR_LOCK_SERVER_ENDPOINT 1
#endif

// ------------------------------------------------------------------------------
// Logging

/** @brief This value should reflect the value of the NumberOfLogRecordsSupported attribute. */
#define SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_LOG_ENTRIES 16

typedef struct {
  uint16_t logEntryId;
  uint32_t timestamp;
  sl_zigbee_af_door_lock_event_type_t eventType;
  sl_zigbee_af_door_lock_event_source_t source;

  // This field is either a sl_zigbee_af_door_lock_operation_event_code_t or a
  // sl_zigbee_af_door_lock_programming_event_code_t.
  uint8_t eventId;

  uint16_t userId;

  // This field is a ZCL string representing the PIN code (i.e., the first byte
  // of the buffer is the length of the total buffer).
  uint8_t pin[SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_PIN_LENGTH + 1];
} sl_zigbee_af_plugin_door_lock_server_log_entry_t;

/**
 * @name API
 * @{
 */

/** @brief Add a log entry. Returns true if the entry was added. Note that the eventId
 * parameter should be of type sl_zigbee_af_door_lock_operation_event_code_t or
 * sl_zigbee_af_door_lock_programming_event_code_t.
 *
 * @param[in] eventType event type
 * @param[in] source event source
 * @param[in] eventId event ID
 * @param[in] userId user ID
 * @param[in] pinLength pin length
 * @param[in] pin pin
 *
 * @return bool True if the log entry was added.
 */
bool sl_zigbee_af_door_lock_server_add_log_entry(sl_zigbee_af_door_lock_event_type_t eventType,
                                                 sl_zigbee_af_door_lock_event_source_t source,
                                                 uint8_t eventId,
                                                 uint16_t userId,
                                                 uint8_t pinLength,
                                                 uint8_t *pin);

/** @brief Get a log entry associated with the entry ID. If the entry ID does not exist,
 * the most recent entry is returned and the entryId parameter is updated. The
 * entryId is a 1-based index into an array of log entries in order to match
 * GetLogRecord ZCL command. This will return true if the entry was successfully
 * returned.
 *
 * @param[in,out] entryId pointer to the entry ID
 * @param[out] entry pointer to log entry struct
 *
 * @return bool True if the command was successfully handled.
 */

bool sl_zigbee_af_door_lock_server_get_log_entry(uint16_t *entryId,
                                                 sl_zigbee_af_plugin_door_lock_server_log_entry_t *entry);

// ------------------------------------------------------------------------------
// Users

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Initialize the Door Lock users.
void sli_zigbee_af_door_lock_server_init_user(void);

// Initialize events.
void sli_zigbee_af_door_lock_server_init_events(uint8_t init_level);

// Set the user type associated with the provided user ID (userId) and return
// true if successful.
bool sli_zigbee_af_door_lock_server_set_pin_user_type(uint16_t userId,
                                                      sl_zigbee_af_door_lock_user_type_t type);
#endif

typedef struct {
  sl_zigbee_af_door_lock_user_status_t status;
  sl_zigbee_af_door_lock_user_type_t type;

  // This field is a Zigbee string, so the first byte is the length of the
  // remaining bytes.
  union {
    uint8_t pin[SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_PIN_LENGTH + 1]; /**< String (length-prefixed) containing PIN code. */
    uint8_t rfid[SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_RFID_LENGTH + 1]; /**< String (length-prefixed) containing RFID code. */
  } code;
} sl_zigbee_af_plugin_door_lock_server_user_t;

/** @brief Unlock the door with a pin.
 *
 * @param[in] pin pin to attempt
 * @param[in] pinLength pin length
 *
 * @return An ::sl_status_t value that indicates the success or failure of the API call. See ::sl_zigbee_af_write_server_attribute.
 */
sl_zigbee_af_status_t sl_zigbee_af_door_lock_server_apply_pin(uint8_t *pin,
                                                              uint8_t pinLength);

/** @brief Unlock the door with RFID.
 *
 * @param[in] rfid RFID to attempt
 * @param[in] rfidLength RFID length
 *
 * @return An ::sl_status_t value that indicates the success or failure of the API call. See ::sl_zigbee_af_write_server_attribute.
 *
 */
sl_zigbee_af_status_t sl_zigbee_af_door_lock_server_apply_rfid(uint8_t *rfid,
                                                               uint8_t rfidLength);

/** @} */ // end of name API

// ------------------------------------------------------------------------------
// Schedule

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Initialize the Door Lock schedules.
void sli_zigbee_af_door_lock_server_init_schedule(void);
#endif

typedef struct {
  uint16_t userId;
  uint8_t daysMask;
  uint8_t startHour;
  uint8_t startMinute;
  uint8_t stopHour;
  uint8_t stopMinute;
  bool inUse;
} sl_zigbee_af_plugin_door_lock_server_weekday_schedule_entry_t;

typedef struct {
  uint16_t userId;
  uint32_t localStartTime;
  uint32_t localEndTime;
  bool inUse;
} sl_zigbee_af_plugin_door_lock_server_yearday_schedule_entry_t;

typedef struct {
  uint32_t localStartTime;
  uint32_t localEndTime;
  sl_zigbee_af_door_lock_operating_mode_t operatingModeDuringHoliday;
  bool inUse;
} sl_zigbee_af_plugin_door_lock_server_holiday_schedule_entry_t;

typedef struct {
  uint32_t localStartTime;
  uint32_t localEndTime;
  bool     inUse;
} sl_zigbee_af_plugin_door_lock_server_disposable_schedule_entry_t;

// ------------------------------------------------------------------------------
// Utilities

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// This function will see if the space required (spaceReq) is greater than the
// space available (spaceAvail) and if so it will send a DefaultResponse
// command with the status of SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE and return
// false. Otherwise, it will return true.
bool sli_zigbee_af_door_lock_server_check_for_sufficient_space(uint8_t spaceReq, uint8_t spaceAvail);
#endif

// Critical Message Queue
// If the Critical Message Queue Plugin is available, use it for event notifications
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_CRITICAL_MESSAGE_QUEUE_PRESENT
#include "../critical-message-queue/critical-message-queue.h"
#define SEND_COMMAND_UNICAST_TO_BINDINGS() sl_zigbee_af_send_command_unicast_to_bindings_with_cb(sl_zigbee_af_critical_message_queue_enqueue_cb)
#else
#define SEND_COMMAND_UNICAST_TO_BINDINGS() sl_zigbee_af_send_command_unicast_to_bindings()
#endif
// ------------------------------------------------------------------------------
// Legacy

// The following preprocessor logic serves as a conversion layer from the old
// names for the symbols used by this plugin.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define sl_zigbee_af_door_lock_schedule_entry_t sl_zigbee_af_plugin_door_lock_server_weekday_schedule_entry_t
#define sl_zigbee_af_door_lock_user_t sl_zigbee_af_plugin_door_lock_server_user_t
#endif

/** @} */ // end of door-lock-server
