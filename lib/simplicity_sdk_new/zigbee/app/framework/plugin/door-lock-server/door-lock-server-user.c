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

#include "../../include/af.h"
#include "../../util/common.h"
#include "app/framework/plugin/door-lock-server/door-lock-server.h"

#include "zap-cluster-command-parser.h"
sl_zigbee_af_event_t sl_zigbee_af_door_lock_server_lockout_event;
sl_zigbee_af_event_t sl_zigbee_af_door_lock_server_relock_event;
#define lockoutEvent (&sl_zigbee_af_door_lock_server_lockout_event)
#define relockEvent (&sl_zigbee_af_door_lock_server_relock_event)
void sl_zigbee_af_door_lock_server_lockout_event_handler(sl_zigbee_af_event_t * event);
void sl_zigbee_af_door_lock_server_relock_event_handler(sl_zigbee_af_event_t * event);

// The index into these tables is a userId.
static sl_zigbee_af_plugin_door_lock_server_user_t pinUserTable[SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE];
static sl_zigbee_af_plugin_door_lock_server_user_t rfidUserTable[SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE];

// This is the current number of invalid PIN/RFID's in a row.
static uint8_t wrongCodeEntryCount = 0;

void sli_zigbee_af_door_lock_server_init_events(uint8_t init_level)
{
  (void) init_level;
  sl_zigbee_af_event_init(&sl_zigbee_af_door_lock_server_lockout_event,
                          sl_zigbee_af_door_lock_server_lockout_event_handler);
  sl_zigbee_af_event_init(&sl_zigbee_af_door_lock_server_relock_event,
                          sl_zigbee_af_door_lock_server_relock_event_handler);
}

bool sli_zigbee_af_door_lock_server_check_for_sufficient_space(uint8_t spaceReq, uint8_t spaceAvail)
{
  if (spaceReq > spaceAvail) {
    sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE);
    return false;
  }
  return true;
}

// ------------------------------------------------------------------------------
// Initialization

static void enableSendPinOverTheAir(void)
{
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SEND_PIN_OVER_THE_AIR_ATTRIBUTE
  bool troo = true;
  sl_zigbee_af_status_t status
    = sl_zigbee_af_write_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                          ZCL_DOOR_LOCK_CLUSTER_ID,
                                          ZCL_SEND_PIN_OVER_THE_AIR_ATTRIBUTE_ID,
                                          (uint8_t *)&troo,
                                          ZCL_BOOLEAN_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_door_lock_cluster_println("Failed to write SendPinOverTheAir attribute: 0x%02X",
                                           status);
  }
#endif
}

void sli_zigbee_af_door_lock_server_init_user(void)
{
#if defined(ZCL_USING_DOOR_LOCK_CLUSTER_NUM_TOTAL_USERS_SUPPORTED_ATTRIBUTE) \
  || defined(ZCL_USING_DOOR_LOCK_CLUSTER_NUM_PIN_USERS_SUPPORTED_ATTRIBUTE)  \
  || defined(ZCL_USING_DOOR_LOCK_CLUSTER_NUM_RFID_USERS_SUPPORTED_ATTRIBUTE)
  const sli_zigbee_af_door_lock_server_attribute_data data[] = {
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_NUM_TOTAL_USERS_SUPPORTED_ATTRIBUTE
    // This attribute is...confusing. Here is the description of it from
    // 7.3.2.12.2.
    //   "Number of total users supported by the lock. This value is equal to the
    //    higher one of [# of PIN Users Supported] and [# of RFID Users
    //    Supported]"
    { ZCL_NUM_TOTAL_USERS_SUPPORTED_ATTRIBUTE_ID,
      ((SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE
        > SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE)
       ? SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE
       : SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE) },
#endif

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_NUM_PIN_USERS_SUPPORTED_ATTRIBUTE
    { ZCL_NUM_PIN_USERS_SUPPORTED_ATTRIBUTE_ID,
      SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE },
#endif

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_NUM_RFID_USERS_SUPPORTED_ATTRIBUTE
    { ZCL_NUM_RFID_USERS_SUPPORTED_ATTRIBUTE_ID,
      SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE },
#endif
  };
  sli_zigbee_af_door_lock_server_write_attributes(data, COUNTOF(data), "user table");
#endif

  enableSendPinOverTheAir();
}

// "pin" parameter is Zigbee string, so first byte is length.
static void printPin(uint8_t *pin)
{
  uint8_t pinLength = sl_zigbee_af_string_length(pin);
  sl_zigbee_af_door_lock_cluster_print("(%02X)", pinLength);
  for (uint8_t i = 0; i < pinLength; i++) {
    sl_zigbee_af_door_lock_cluster_print(" %c", pin[i + 1]);
  }
}

static void printUserTables(void)
{
  uint8_t i;
  sl_zigbee_af_door_lock_cluster_println("id   st ty PIN");
  sl_zigbee_af_door_lock_cluster_println("PIN:");
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE; i++) {
    sl_zigbee_af_plugin_door_lock_server_user_t *user = &pinUserTable[i];
    sl_zigbee_af_door_lock_cluster_print("%04X %02X %02X ", i, user->status, user->type);
    printPin(user->code.pin);
    sl_zigbee_af_door_lock_cluster_println("");
  }
  sl_zigbee_af_door_lock_cluster_println("RFID:");
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE; i++) {
    sl_zigbee_af_plugin_door_lock_server_user_t *user = &rfidUserTable[i];
    sl_zigbee_af_door_lock_cluster_print("%04X %02X %02X ", i, user->status, user->type);
    printPin(user->code.rfid);
    sl_zigbee_af_door_lock_cluster_println("");
  }
}

// Returns status byte for use in SetPinResponse and SetRfidResponse commands.
static uint8_t setUser(uint16_t userId,
                       uint8_t userStatus,
                       uint8_t userType,
                       uint8_t *code,
                       sl_zigbee_af_plugin_door_lock_server_user_t *userTable,
                       uint8_t userTableSize)
{
  bool success = false;
  // "code" (i.e. PIN/RFID) is stored in table entry in ZCL format (1-byte
  // length prefix). Don't allow a code with length that exceeds capacity
  // of the table entry field. Note there are potentially different max
  // lengths for PIN v. RFID.
  bool validCodeLength = false;
  if (code != NULL
      && ((userTable == pinUserTable
           && sl_zigbee_af_string_length(code) <= SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_PIN_LENGTH)
          || (sl_zigbee_af_string_length(code) <= SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_RFID_LENGTH))) {
    validCodeLength = true;
  }

  if (validCodeLength && userId < userTableSize) {
    sl_zigbee_af_plugin_door_lock_server_user_t *user = &userTable[userId];
    user->status = userStatus;
    user->type = userType;
    memmove(user->code.rfid,
            code,
            sl_zigbee_af_string_length(code) + 1); // + 1 for Zigbee string length byte

    sl_zigbee_af_door_lock_cluster_println("***RX SET %s ***",
                                           (userTable == pinUserTable ? "PIN" : "RFID"));
    printUserTables();

    success = true;
  }
  return (success ? 0x00 : 0x01); // See 7.3.2.17.6 and 7.3.2.17.23).
}

// Returns true for success, false for failure.
static bool getUser(uint16_t userId,
                    sl_zigbee_af_plugin_door_lock_server_user_t *userTable,
                    uint8_t userTableSize,
                    sl_zigbee_af_plugin_door_lock_server_user_t *returnedUser)
{
  bool success = false;
  if (userId < userTableSize) {
    *returnedUser = userTable[userId];
    success = true;
  }
  return success;
}

// Returns status byte for use in ClearPin and ClearRfid response commands.
static uint8_t clearUserPinOrRfid(uint16_t userId,
                                  sl_zigbee_af_plugin_door_lock_server_user_t *userTable,
                                  uint8_t userTableSize)
{
  bool success = false;
  if (userId < userTableSize) {
    // Since the rfid member of the struct is a Zigbee string, setting the first
    // byte to 0 will indicate that we have a 0-length pin.
    memset((userTable == pinUserTable
            ? userTable[userId].code.pin
            : userTable[userId].code.rfid),
           0x00,
           sizeof(userTable[userId].code));
    success = true;
  }
  return (success ? 0x00 : 0x01); // See 7.3.2.17.8 and 7.3.2.17.25).
}

bool sli_zigbee_af_door_lock_server_set_pin_user_type(uint16_t userId,
                                                      sl_zigbee_af_door_lock_user_type_t type)
{
  if (userId >= SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE) {
    return false;
  } else {
    pinUserTable[userId].type = type;
    return true;
  }
}

static bool getSendPinOverTheAir(void)
{
  bool sendPinOverTheAir = true;
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SEND_PIN_OVER_THE_AIR_ATTRIBUTE
  sl_zigbee_af_status_t status
    = sl_zigbee_af_read_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                         ZCL_DOOR_LOCK_CLUSTER_ID,
                                         ZCL_SEND_PIN_OVER_THE_AIR_ATTRIBUTE_ID,
                                         (uint8_t *)&sendPinOverTheAir,
                                         sizeof(sendPinOverTheAir));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_door_lock_cluster_println("Failed to read SendPinOverTheAir attribute: 0x%02X",
                                           status);
  }
#endif
  return sendPinOverTheAir;
}

static void printSuccessOrFailure(bool success)
{
  if (success) {
    sl_zigbee_af_door_lock_cluster_println("SUCCESS!");
  } else {
    sl_zigbee_af_door_lock_cluster_println("FAILURE!");
  }
}

/**
 * This function checks to see if a pin is required and, if it is
 * and a pin is provided, it validates the pin against those known in
 * the user table.
 *
 * Note that the "pin" parameter is a Zigbee string, so the first byte is the
 * length of the remaining bytes
 */
static bool verifyPin(uint8_t *pin, uint8_t *userId)
{
  bool pinRequired = false;
  sl_status_t status;
  uint8_t i;

  status = sl_zigbee_af_read_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                              ZCL_DOOR_LOCK_CLUSTER_ID,
                                              ZCL_REQUIRE_PIN_FOR_RF_OPERATION_ATTRIBUTE_ID,
                                              (uint8_t *) &pinRequired,
                                              sizeof(pinRequired));
  if (SL_STATUS_OK != status || !pinRequired) {
    return true;
  } else if (pin == NULL) {
    return false;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE; i++) {
    sl_zigbee_af_plugin_door_lock_server_user_t *user = &pinUserTable[i];
    uint8_t userPinLength = sl_zigbee_af_string_length(user->code.pin);
    if (userPinLength == sl_zigbee_af_string_length(pin)
        && 0 == memcmp(&user->code.pin[1], &pin[1], userPinLength)) {
      *userId = i;
      return true;
    }
  }

  return false;
}

static uint8_t getWrongCodeEntryLimit(void)
{
  uint8_t limit = UINT8_MAX;
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_WRONG_CODE_ENTRY_LIMIT_ATTRIBUTE
  sl_zigbee_af_status_t status
    = sl_zigbee_af_read_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                         ZCL_DOOR_LOCK_CLUSTER_ID,
                                         ZCL_WRONG_CODE_ENTRY_LIMIT_ATTRIBUTE_ID,
                                         &limit,
                                         sizeof(limit));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_door_lock_cluster_println("Failed to read WrongCodeEntryLimitAttribute: 0x%02X",
                                           status);
  }
#endif
  return limit;
}

static uint8_t getUserCodeTemporaryDisableTime(void)
{
  uint8_t timeS = 0; // 0 effectively turns off the temporary disable time
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_USER_CODE_TEMPORARY_DISABLE_TIME_ATTRIBUTE
  sl_zigbee_af_status_t status
    = sl_zigbee_af_read_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                         ZCL_DOOR_LOCK_CLUSTER_ID,
                                         ZCL_USER_CODE_TEMPORARY_DISABLE_TIME_ATTRIBUTE_ID,
                                         &timeS,
                                         sizeof(timeS));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_door_lock_cluster_println("Failed to read UserCodeTemporaryDisableTime: 0x%02X",
                                           status);
  }
#endif
  return timeS;
}

static void startLockout(void)
{
  sl_zigbee_af_door_lock_cluster_println("Door lock entering lockout mode");

  uint8_t lockoutTimeS = getUserCodeTemporaryDisableTime();
  if (lockoutTimeS != 0) {
    sl_zigbee_af_event_set_delay_ms(lockoutEvent,
                                    lockoutTimeS * MILLISECOND_TICKS_PER_SECOND);
  }
}

// If code is NULL, then the door will automatically be unlocked.
static sl_zigbee_af_status_t applyCode(uint8_t *code,
                                       uint8_t codeLength,
                                       sl_zigbee_af_plugin_door_lock_server_user_t *userTable,
                                       uint8_t userTableLength)
{
  for (uint8_t i = 0; i < userTableLength; i++) {
    uint8_t *userCode = (userTable == pinUserTable
                         ? userTable[i].code.pin
                         : userTable[i].code.rfid);
    if (code == NULL
        || (sl_zigbee_af_string_length(userCode) == codeLength
            && memcmp(code, userCode + 1, codeLength) == 0)) {
      sl_zigbee_af_door_lock_state_t state = SL_ZIGBEE_ZCL_DOOR_LOCK_STATE_UNLOCKED;
      return sl_zigbee_af_write_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                                 ZCL_DOOR_LOCK_CLUSTER_ID,
                                                 ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                                 (uint8_t *)&state,
                                                 ZCL_ENUM8_ATTRIBUTE_TYPE);
    }
  }

  wrongCodeEntryCount++;
  if (wrongCodeEntryCount >= getWrongCodeEntryLimit()) {
    startLockout();
  }
  return SL_ZIGBEE_ZCL_STATUS_FAILURE;
}

void sl_zigbee_af_door_lock_server_lockout_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_zigbee_af_event_set_inactive(lockoutEvent);
  sl_zigbee_af_door_lock_cluster_println("Door lock entering normal mode");
}

sl_zigbee_af_status_t sl_zigbee_af_door_lock_server_apply_rfid(uint8_t *rfid,
                                                               uint8_t rfidLength)
{
  return applyCode(rfid,
                   rfidLength,
                   rfidUserTable,
                   SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE);
}

sl_zigbee_af_status_t sl_zigbee_af_door_lock_server_apply_pin(uint8_t *pin,
                                                              uint8_t pinLength)
{
  return applyCode(pin,
                   pinLength,
                   pinUserTable,
                   SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE);
}

// --------------------------------------
// Auto relock functionality

// Passing 0 to this function will disable the Relock event.
// Passing UINT32_MAX to this function will force it to use the AutoRelock
// attribute.
static void scheduleAutoRelock(uint32_t autoRelockTimeS)
{
  if (autoRelockTimeS == UINT32_MAX) {
    sl_zigbee_af_status_t status
      = sl_zigbee_af_read_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                           ZCL_DOOR_LOCK_CLUSTER_ID,
                                           ZCL_AUTO_RELOCK_TIME_ATTRIBUTE_ID,
                                           (uint8_t *)&autoRelockTimeS,
                                           sizeof(autoRelockTimeS));
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_door_lock_cluster_println("Failed to read AutoRelockTime attribute: 0x%02X",
                                             status);
      return;
    }
  }

  if (autoRelockTimeS == 0) {
    sl_zigbee_af_event_set_inactive(relockEvent);
  } else {
    sl_zigbee_af_event_set_delay_ms(relockEvent,
                                    (autoRelockTimeS
                                     * MILLISECOND_TICKS_PER_SECOND));
  }
}

void sl_zigbee_af_door_lock_server_relock_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_zigbee_af_event_set_inactive(relockEvent);

  sl_zigbee_af_status_t status = applyCode(NULL,
                                           0,
                                           pinUserTable,
                                           SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE);
  sl_zigbee_af_door_lock_cluster_println("Door automatically relocked: 0x%02X", status);
}

void sl_zigbee_af_door_lock_cluster_server_attribute_changed_cb(uint8_t endpoint,
                                                                sl_zigbee_af_attribute_id_t attributeId)
{
  if (endpoint == DOOR_LOCK_SERVER_ENDPOINT
      && attributeId == ZCL_LOCK_STATE_ATTRIBUTE_ID) {
    uint8_t lockState;
    sl_zigbee_af_status_t status = sl_zigbee_af_read_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                                                      ZCL_DOOR_LOCK_CLUSTER_ID,
                                                                      ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                                                      &lockState,
                                                                      sizeof(lockState));
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_door_lock_cluster_println("Failed to read LockState attribute: 0x%02X",
                                             status);
    } else if (lockState == SL_ZIGBEE_ZCL_DOOR_LOCK_STATE_UNLOCKED) {
      scheduleAutoRelock(UINT32_MAX);
    }
  }
}

// --------------------------------------
// Command callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_lock_door_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_lock_door_command_t cmd_data;
  uint8_t userId = 0;
  bool pinVerified;
  bool doorLocked = false;
  uint8_t lockStateLocked = 0x01;
  uint16_t rfOperationEventMask = 0xffff; //will send events by default

  if (zcl_decode_door_lock_cluster_lock_door_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  pinVerified = verifyPin(cmd_data.PIN, &userId);

  sl_zigbee_af_door_lock_cluster_print("LOCK DOOR ");
  printSuccessOrFailure(pinVerified);

  if (pinVerified) {
    doorLocked =
      sl_zigbee_af_door_lock_server_activate_door_lock_cb(true); //lock door
  }

  if (doorLocked) {
    sl_zigbee_af_write_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                        ZCL_DOOR_LOCK_CLUSTER_ID,
                                        ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                        &lockStateLocked,
                                        ZCL_INT8U_ATTRIBUTE_TYPE);
  }

  //send response
  sl_zigbee_af_fill_command_door_lock_cluster_lock_door_response(doorLocked
                                                                 ? SL_ZIGBEE_ZCL_STATUS_SUCCESS
                                                                 : SL_ZIGBEE_ZCL_STATUS_FAILURE);
  sl_zigbee_af_send_response();

  //check if we should send event notification
  sl_zigbee_af_read_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                     ZCL_DOOR_LOCK_CLUSTER_ID,
                                     ZCL_RF_OPERATION_EVENT_MASK_ATTRIBUTE_ID,
                                     (uint8_t*)&rfOperationEventMask,
                                     sizeof(rfOperationEventMask));

  // Possibly send operation event
  if (doorLocked) {
    if (rfOperationEventMask & BIT(1) && (cmd_data.PIN != NULL)) {
      sl_zigbee_af_fill_command_door_lock_cluster_operation_event_notification(0x01, 0x01, userId, cmd_data.PIN, 0x00, cmd_data.PIN);
    }
  } else {
    if (rfOperationEventMask & BIT(3) && (cmd_data.PIN != NULL)) {
      sl_zigbee_af_fill_command_door_lock_cluster_operation_event_notification(0x01, 0x03, userId, cmd_data.PIN, 0x00, cmd_data.PIN);
    }
  }
  SEND_COMMAND_UNICAST_TO_BINDINGS();

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_unlock_door_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_unlock_door_command_t cmd_data;
  uint8_t userId = 0;
  bool pinVerified;
  bool doorUnlocked = false;
  uint8_t lockStateUnlocked = 0x02;
  uint16_t rfOperationEventMask = 0xffff; //sends event by default

  if (zcl_decode_door_lock_cluster_unlock_door_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  pinVerified = verifyPin(cmd_data.PIN, &userId);

  sl_zigbee_af_door_lock_cluster_print("UNLOCK DOOR ");
  printSuccessOrFailure(pinVerified);

  if (pinVerified) {
    doorUnlocked =
      sl_zigbee_af_door_lock_server_activate_door_lock_cb(false); //unlock door
  }
  if (doorUnlocked) {
    sl_zigbee_af_write_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                        ZCL_DOOR_LOCK_CLUSTER_ID,
                                        ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                        &lockStateUnlocked,
                                        ZCL_INT8U_ATTRIBUTE_TYPE);
  }

  sl_zigbee_af_fill_command_door_lock_cluster_unlock_door_response(doorUnlocked
                                                                   ? SL_ZIGBEE_ZCL_STATUS_SUCCESS
                                                                   : SL_ZIGBEE_ZCL_STATUS_FAILURE);
  sl_zigbee_af_send_response();

  //get bitmask so we can check if we should send event notification
  sl_zigbee_af_read_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                     ZCL_DOOR_LOCK_CLUSTER_ID,
                                     ZCL_RF_OPERATION_EVENT_MASK_ATTRIBUTE_ID,
                                     (uint8_t*)&rfOperationEventMask,
                                     sizeof(rfOperationEventMask));

  //send operation event
  if (doorUnlocked && (rfOperationEventMask & BIT(2)) && (cmd_data.PIN != NULL)) {
    sl_zigbee_af_fill_command_door_lock_cluster_operation_event_notification(SL_ZIGBEE_ZCL_DOOR_LOCK_EVENT_SOURCE_RF,
                                                                             SL_ZIGBEE_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_UNLOCK,
                                                                             userId,
                                                                             cmd_data.PIN,
                                                                             sl_zigbee_af_get_current_time(),
                                                                             cmd_data.PIN);
    SEND_COMMAND_UNICAST_TO_BINDINGS();
  }

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_unlock_with_timeout_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_unlock_with_timeout_command_t cmd_data;
  uint8_t userId;
  uint8_t status;

  if (zcl_decode_door_lock_cluster_unlock_with_timeout_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (verifyPin(cmd_data.pin, &userId)) {
    uint8_t lockState = SL_ZIGBEE_ZCL_DOOR_LOCK_STATE_LOCKED;
    sl_zigbee_af_status_t readStatus
      = sl_zigbee_af_write_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                            ZCL_DOOR_LOCK_CLUSTER_ID,
                                            ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                            &lockState,
                                            ZCL_ENUM8_ATTRIBUTE_TYPE);
    if (readStatus != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_door_lock_cluster_println("Failed to write LockState attribute: 0x%02X",
                                             readStatus);
    }

    scheduleAutoRelock(cmd_data.timeoutInSeconds);
    status = 0x00; // success (per 7.3.2.17.4)
  } else {
    status = 0x01; // failure (per 7.3.2.17.4)
  }

  sl_zigbee_af_fill_command_door_lock_cluster_unlock_with_timeout_response(status);
  sl_status_t sl_zigbee_status = sl_zigbee_af_send_response();
  if (sl_zigbee_status != SL_STATUS_OK) {
    sl_zigbee_af_door_lock_cluster_println("Failed to send UnlockWithTimeoutResponse: 0x%02X",
                                           status);
  }

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_set_pin_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_set_pin_command_t cmd_data;
  uint8_t status;

  if (zcl_decode_door_lock_cluster_set_pin_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  //send response
  status = setUser(cmd_data.userId,
                   cmd_data.userStatus,
                   cmd_data.userType,
                   cmd_data.pin,
                   pinUserTable,
                   SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE);
  sl_zigbee_af_fill_command_door_lock_cluster_set_pin_response(status);
  sl_zigbee_af_send_response();

  //get bitmask so we can check if we should send event notification
  uint16_t rfProgrammingEventMask = 0xffff; //send event by default
  sl_zigbee_af_read_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                     ZCL_DOOR_LOCK_CLUSTER_ID,
                                     ZCL_RF_PROGRAMMING_EVENT_MASK_ATTRIBUTE_ID,
                                     (uint8_t*)&rfProgrammingEventMask,
                                     sizeof(rfProgrammingEventMask));
  if ((rfProgrammingEventMask & BIT(2)) && !status && (cmd_data.pin != NULL)) {
    sl_zigbee_af_fill_command_door_lock_cluster_programming_event_notification(SL_ZIGBEE_ZCL_DOOR_LOCK_EVENT_SOURCE_RF,
                                                                               SL_ZIGBEE_ZCL_DOOR_LOCK_PROGRAMMING_EVENT_CODE_PIN_ADDED,
                                                                               cmd_data.userId,
                                                                               cmd_data.pin,
                                                                               cmd_data.userType,
                                                                               cmd_data.userStatus,
                                                                               sl_zigbee_af_get_current_time(),
                                                                               cmd_data.pin);
    SEND_COMMAND_UNICAST_TO_BINDINGS();
  }

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_pin_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_get_pin_command_t cmd_data;
  sl_zigbee_af_plugin_door_lock_server_user_t user;
  sl_status_t status;

  if (zcl_decode_door_lock_cluster_get_pin_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (getUser(cmd_data.userId,
              pinUserTable,
              SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE,
              &user)) {
    uint8_t fakePin = 0x00;
    sl_zigbee_af_fill_command_door_lock_cluster_get_pin_response(cmd_data.userId,
                                                                 user.status,
                                                                 user.type,
                                                                 (getSendPinOverTheAir()
                                                                  ? user.code.pin
                                                                  : &fakePin));
    status = sl_zigbee_af_send_response();
  } else {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_door_lock_cluster_println("Failed to send response to GetPin: 0x%02X",
                                           status);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_pin_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_clear_pin_command_t cmd_data;
  uint8_t status;

  if (zcl_decode_door_lock_cluster_clear_pin_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  status = clearUserPinOrRfid(cmd_data.userId,
                              pinUserTable,
                              SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE);

  sl_zigbee_af_fill_command_door_lock_cluster_clear_pin_response(status);

  sl_status_t sl_zigbee_status = sl_zigbee_af_send_response();
  if (sl_zigbee_status != SL_STATUS_OK) {
    sl_zigbee_af_door_lock_cluster_println("Failed to send ClearPinResponse: 0x%02X",
                                           sl_zigbee_status);
  }

  //get bitmask so we can check if we should send event notification
  uint16_t rfProgrammingEventMask = 0xffff; //event sent by default
  uint8_t userPin = 0x00; // Zero length Zigbee string
  sl_zigbee_af_read_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                     ZCL_DOOR_LOCK_CLUSTER_ID,
                                     ZCL_RF_PROGRAMMING_EVENT_MASK_ATTRIBUTE_ID,
                                     (uint8_t*)&rfProgrammingEventMask,
                                     sizeof(rfProgrammingEventMask));
  if ((rfProgrammingEventMask & BIT(2)) && !status) {
    sl_zigbee_af_fill_command_door_lock_cluster_programming_event_notification(0x01, 0x03, cmd_data.userId, &userPin, 0x00, 0x00, 0x00, &userPin);
    SEND_COMMAND_UNICAST_TO_BINDINGS();
  } else if ((rfProgrammingEventMask & BIT(0)) && status) {
    sl_zigbee_af_fill_command_door_lock_cluster_programming_event_notification(0x01, 0x00, cmd_data.userId, &userPin, 0x00, 0x00, 0x00, &userPin);
    SEND_COMMAND_UNICAST_TO_BINDINGS();
  }

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_set_user_type_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_set_user_type_command_t cmd_data;
  uint8_t status;
  sl_status_t sl_zigbee_status;

  if (zcl_decode_door_lock_cluster_set_user_type_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  status = (sli_zigbee_af_door_lock_server_set_pin_user_type(cmd_data.userId, cmd_data.userType)
            ? 0x00   // success (per 7.3.2.17.21)
            : 0x01); // failure (per 7.3.2.17.21)
  sl_zigbee_af_fill_command_door_lock_cluster_set_user_type_response(status);

  sl_zigbee_status = sl_zigbee_af_send_response();
  if (sl_zigbee_status != SL_STATUS_OK) {
    sl_zigbee_af_door_lock_cluster_println("Failed to send SetUserTypeResponse: 0x%02X",
                                           sl_zigbee_status);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_user_type_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_get_user_type_command_t cmd_data;

  if (zcl_decode_door_lock_cluster_get_user_type_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (sli_zigbee_af_door_lock_server_check_for_sufficient_space(cmd_data.userId,
                                                                SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE)) {
    sl_zigbee_af_plugin_door_lock_server_user_t *user = &pinUserTable[cmd_data.userId];
    sl_zigbee_af_fill_command_door_lock_cluster_get_user_type_response(cmd_data.userId, user->type);
    sl_status_t status = sl_zigbee_af_send_response();
    if (status != SL_STATUS_OK) {
      sl_zigbee_af_door_lock_cluster_println("Failed to send GetUserTypeResponse: 0x%02X",
                                             status);
    }
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }
  return SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_set_rfid_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_set_rfid_command_t cmd_data;
  uint8_t status;

  if (zcl_decode_door_lock_cluster_set_rfid_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  status = setUser(cmd_data.userId,
                   cmd_data.userStatus,
                   cmd_data.userType,
                   cmd_data.id,
                   rfidUserTable,
                   SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE);
  sl_zigbee_af_fill_command_door_lock_cluster_set_rfid_response(status);

  sl_status_t sl_zigbee_status = sl_zigbee_af_send_response();
  if (sl_zigbee_status != SL_STATUS_OK) {
    sl_zigbee_af_door_lock_cluster_println("Failed to send SetRfidResponse: 0x%02X",
                                           sl_zigbee_status);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_rfid_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_get_rfid_command_t cmd_data;
  sl_zigbee_af_plugin_door_lock_server_user_t user;
  sl_status_t status;

  if (zcl_decode_door_lock_cluster_get_rfid_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (getUser(cmd_data.userId,
              rfidUserTable,
              SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE,
              &user)) {
    sl_zigbee_af_fill_command_door_lock_cluster_get_rfid_response(cmd_data.userId,
                                                                  user.status,
                                                                  user.type,
                                                                  user.code.pin);
    status = sl_zigbee_af_send_response();
    if (status != SL_STATUS_OK) {
      sl_zigbee_af_door_lock_cluster_println("Failed to send response to GetRfid: 0x%02X",
                                             status);
    }
  } else {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_rfid_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_door_lock_cluster_clear_rfid_command_t cmd_data;
  uint8_t status;

  if (zcl_decode_door_lock_cluster_clear_rfid_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  status = clearUserPinOrRfid(cmd_data.userId,
                              rfidUserTable,
                              SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE);
  sl_zigbee_af_fill_command_door_lock_cluster_clear_rfid_response(status);

  sl_status_t sl_zigbee_status = sl_zigbee_af_send_response();
  if (sl_zigbee_status != SL_STATUS_OK) {
    sl_zigbee_af_door_lock_cluster_println("Failed to send ClearRfidResponse: 0x%02X",
                                           sl_zigbee_status);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_biometric_credential_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;

  sl_zigbee_af_door_lock_cluster_println("Clear Biometric Credential ");

  // Nothing in specification for this so far
  sl_zigbee_af_fill_command_door_lock_cluster_clear_biometric_credential_response(0x00);
  sl_status_t status = sl_zigbee_af_send_response();
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_door_lock_cluster_println("Failed to send ClearBiometricCredentialResponse: 0x%02X",
                                           status);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_all_pins_cb(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE; i++) {
    clearUserPinOrRfid(i,
                       pinUserTable,
                       SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE);
  }

  // 7.3.2.17.9 says that "0x00" indicates success.
  sl_zigbee_af_fill_command_door_lock_cluster_clear_all_pins_response(0x00);
  sl_status_t status = sl_zigbee_af_send_response();
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_door_lock_cluster_println("Failed to send ClearAllPins: 0x%02X", status);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_all_rfids_cb(void)
{
  for (uint8_t i = 0;
       i < SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE;
       i++) {
    clearUserPinOrRfid(i,
                       rfidUserTable,
                       SL_ZIGBEE_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE);
  }

  // 7.3.2.17.26 says that "0x00" indicates success.
  sl_zigbee_af_fill_command_door_lock_cluster_clear_all_rfids_response(0x00);
  sl_status_t status = sl_zigbee_af_send_response();
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_door_lock_cluster_println("Failed to send ClearAllRfidsResponse: 0x%02X",
                                           status);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_all_biometric_credentials_cb(void)
{
  sl_zigbee_af_door_lock_cluster_println("Clear All Biometric Credentials ");

  // Nothing in specification for this so far
  sl_zigbee_af_fill_command_door_lock_cluster_clear_all_biometric_credentials_response(0x00);
  sl_status_t status = sl_zigbee_af_send_response();
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_door_lock_cluster_println("Failed to send ClearAllBiometricCredentialsResponse: 0x%02X",
                                           status);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}
