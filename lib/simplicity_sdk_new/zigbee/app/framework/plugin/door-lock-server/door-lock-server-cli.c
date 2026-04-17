/***************************************************************************//**
 * @file
 * @brief CLI for the Door Lock Server plugin.
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

#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "app/framework/plugin/door-lock-server/door-lock-server.h"

#define ISO_639_1_STRING_LENGTH 3

static const char *getLockStateName(sl_zigbee_af_door_lock_state_t state)
{
  const char *stateNames[] = {
    "Not Fully Locked",
    "Locked",
    "Unlocked",
  };
  return (state < COUNTOF(stateNames) ? stateNames[state] : "?????");
}

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SOUND_VOLUME_ATTRIBUTE
static const char *getSoundVolumeName(uint8_t volume)
{
  const char *soundVolumeNames[] = {
    "Silent",
    "Low",
    "High",
  };
  return (volume < COUNTOF(soundVolumeNames)
          ? soundVolumeNames[volume]
          : "?????");
}
#endif

// plugin door-lock-server status
void sli_zigbee_af_door_lock_server_status_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  uint8_t state;
  sl_zigbee_af_status_t status = sl_zigbee_af_read_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                                                    ZCL_DOOR_LOCK_CLUSTER_ID,
                                                                    ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                                                    (uint8_t *)&state,
                                                                    sizeof(state));
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_app_println("State: %s", getLockStateName(state));
  } else {
    sl_zigbee_af_app_println("Error: cannot read attribute: 0x%02X", status);
  }

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_LANGUAGE_ATTRIBUTE
  uint8_t string[ISO_639_1_STRING_LENGTH + 1]; // + 1 for nul-terminator
  status = sl_zigbee_af_read_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                              ZCL_DOOR_LOCK_CLUSTER_ID,
                                              ZCL_LANGUAGE_ATTRIBUTE_ID,
                                              string,
                                              sizeof(string));
  string[sizeof(string) - 1] = '\0';
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_app_println("Language: %s", &string[1]); // skip the length byte
  } else {
    sl_zigbee_af_app_println("Error: cannot read attribute: 0x%02X", status);
  }
#endif

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SOUND_VOLUME_ATTRIBUTE
  uint8_t volume;
  status = sl_zigbee_af_read_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                              ZCL_DOOR_LOCK_CLUSTER_ID,
                                              ZCL_SOUND_VOLUME_ATTRIBUTE_ID,
                                              &volume,
                                              sizeof(volume));
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_app_println("Volume: %s", getSoundVolumeName(volume));
  } else {
    sl_zigbee_af_app_println("Error: cannot read attribute: 0x%02X", status);
  }
#endif
}

// plugin door-lock-server lock <userId:2>
// plugin door-lock-server unlock <userId:2>
void sli_zigbee_af_door_lock_server_lock_or_unlock_command(sl_cli_command_arg_t *arguments)
{
  bool doLock = memcmp(arguments->argv[arguments->arg_ofs - 1], "lock", strlen("lock")) == 0;
  uint16_t userId = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t state = (doLock
                   ? SL_ZIGBEE_ZCL_DOOR_LOCK_STATE_LOCKED
                   : SL_ZIGBEE_ZCL_DOOR_LOCK_STATE_UNLOCKED);
  sl_zigbee_af_status_t status = sl_zigbee_af_write_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                                                     ZCL_DOOR_LOCK_CLUSTER_ID,
                                                                     ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                                                     (uint8_t *)&state,
                                                                     ZCL_ENUM8_ATTRIBUTE_TYPE);

  const char *action = (doLock ? "lock" : "unlock");
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_app_println("Failed to %s door: 0x%02X", action, status);
  } else {
    sl_zigbee_af_app_println("Door is now %sed", action);

    if (!sl_zigbee_af_door_lock_server_add_log_entry(SL_ZIGBEE_ZCL_DOOR_LOCK_EVENT_TYPE_OPERATION,
                                                     SL_ZIGBEE_ZCL_DOOR_LOCK_EVENT_SOURCE_MANUAL,
                                                     (doLock
                                                      ? SL_ZIGBEE_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_LOCK
                                                      : SL_ZIGBEE_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_UNLOCK),
                                                     userId,
                                                     0,
                                                     NULL)) { // pin...nah
      sl_zigbee_af_app_println("Could not add log entry");
    }
  }
}

// plugin door-lock-server open
// plugin door-lock-server close
void sli_zigbee_af_door_lock_server_open_or_close_command(sl_cli_command_arg_t *arguments)
{
  static uint8_t eventCount = 0;
  uint8_t PIN[] = "";
  bool doOpen = memcmp(arguments->argv[arguments->arg_ofs - 1], "open", strlen("open")) == 0;
  sl_zigbee_af_door_state_t state = (doOpen
                                     ? SL_ZIGBEE_ZCL_DOOR_STATE_OPEN
                                     : SL_ZIGBEE_ZCL_DOOR_STATE_CLOSED);
  sl_zigbee_af_status_t status = sli_zigbee_af_door_lock_server_note_door_state_changed(state);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_app_println("Failed to %s door: 0x%02X",
                             (doOpen ? "open" : "close"),
                             status);
  } else {
    sl_zigbee_af_app_println("Door is now %s", (doOpen ? "open" : "closed"));
    sl_zigbee_af_fill_command_door_lock_cluster_operation_event_notification(SL_ZIGBEE_ZCL_DOOR_LOCK_EVENT_SOURCE_MANUAL,
                                                                             (doOpen
                                                                              ? SL_ZIGBEE_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_MANUAL_UNLOCK
                                                                              : SL_ZIGBEE_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_MANUAL_LOCK),
                                                                             0 /* userId */, PIN, eventCount++ /*timestamp*/, PIN);

    sl_zigbee_aps_frame_t *apsFrame = sl_zigbee_af_get_command_aps_frame();
    apsFrame->sourceEndpoint = 1; //sl_zigbee_af_current_endpoint();

    SEND_COMMAND_UNICAST_TO_BINDINGS();
  }
}

// plugin door-lock-server apply-pin <pin:?>
// plugin door-lock-server apply-rfid <rfid:?>
void sli_zigbee_af_door_lock_server_apply_code_command(sl_cli_command_arg_t *arguments)
{
  uint8_t length;
  bool isPin = memcmp(arguments->argv[arguments->arg_ofs - 1], "apply-pin", strlen("apply-pin")) == 0;

  uint8_t *code = sl_zigbee_cli_get_argument_string_and_length(arguments, 0, &length);

  sl_zigbee_af_status_t status
    = (isPin
       ? sl_zigbee_af_door_lock_server_apply_pin(code, length)
       : sl_zigbee_af_door_lock_server_apply_rfid(code, length));
  sl_zigbee_af_app_println("Apply %s: 0x%02X", (isPin ? "PIN" : "RFID"), status);
}
