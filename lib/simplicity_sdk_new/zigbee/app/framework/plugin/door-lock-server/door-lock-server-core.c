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

static void setActuatorEnable(void)
{
  // The Door Lock cluster test spec expects this attribute set to be true by
  // default...
  bool troo = true;
  sl_zigbee_af_status_t status
    = sl_zigbee_af_write_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                          ZCL_DOOR_LOCK_CLUSTER_ID,
                                          ZCL_ACTUATOR_ENABLED_ATTRIBUTE_ID,
                                          (uint8_t *)&troo,
                                          ZCL_BOOLEAN_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_door_lock_cluster_println("Failed to write ActuatorEnabled attribute: 0x%02X",
                                           status);
  }
}

static void setDoorState(void)
{
  uint8_t state = SL_ZIGBEE_ZCL_DOOR_STATE_ERROR_UNSPECIFIED;
  sl_zigbee_af_status_t status = sl_zigbee_af_write_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                                                     ZCL_DOOR_LOCK_CLUSTER_ID,
                                                                     ZCL_DOOR_STATE_ATTRIBUTE_ID,
                                                                     (uint8_t *)&state,
                                                                     ZCL_ENUM8_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_door_lock_cluster_println("Failed to write DoorState attribute: 0x%02X",
                                           status);
  }
}

static void setLanguage(void)
{
  uint8_t englishString[] = { 0x02, 'e', 'n' };
  sl_zigbee_af_status_t status
    = sl_zigbee_af_write_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                          ZCL_DOOR_LOCK_CLUSTER_ID,
                                          ZCL_LANGUAGE_ATTRIBUTE_ID,
                                          englishString,
                                          ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_door_lock_cluster_println("Failed to write Language attribute: 0x%02X",
                                           status);
  }
}

void sl_zigbee_af_door_lock_server_init_cb(uint8_t init_level)
{
  (void)init_level;

  sli_zigbee_af_door_lock_server_init_user();
  sli_zigbee_af_door_lock_server_init_schedule();

  setActuatorEnable();
  setDoorState();
  setLanguage();
}

void sli_zigbee_af_door_lock_server_write_attributes(const sli_zigbee_af_door_lock_server_attribute_data *data,
                                                     uint8_t dataLength,
                                                     const char *description)
{
  for (uint8_t i = 0; i < dataLength; i++) {
    sl_zigbee_af_status_t status
      = sl_zigbee_af_write_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                            ZCL_DOOR_LOCK_CLUSTER_ID,
                                            data[i].id,
                                            (uint8_t *)&data[i].value,
                                            ZCL_INT16U_ATTRIBUTE_TYPE);
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_door_lock_cluster_println("Failed to write %s attribute 0x%04X: 0x%02X",
                                             data[i].id,
                                             status,
                                             description);
    }
  }
}

sl_zigbee_af_status_t sli_zigbee_af_door_lock_server_note_door_state_changed(sl_zigbee_af_door_state_t state)
{
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;

#if !defined(ZCL_USING_DOOR_LOCK_CLUSTER_DOOR_STATE_ATTRIBUTE)        \
  && !defined(ZCL_USING_DOOR_LOCK_CLUSTER_DOOR_OPEN_EVENTS_ATTRIBUTE) \
  && !defined(ZCL_USING_DOOR_LOCK_CLUSTER_DOOR_CLOSED_EVENTS_ATTRIBUTE)
  UNUSED_VAR(state);
#endif

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_DOOR_STATE_ATTRIBUTE
  status = sl_zigbee_af_write_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                               ZCL_DOOR_LOCK_CLUSTER_ID,
                                               ZCL_DOOR_STATE_ATTRIBUTE_ID,
                                               (uint8_t *)&state,
                                               ZCL_ENUM8_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }
#endif

#if defined(ZCL_USING_DOOR_LOCK_CLUSTER_DOOR_OPEN_EVENTS_ATTRIBUTE) \
  || defined(ZCL_USING_DOOR_LOCK_CLUSTER_DOOR_CLOSED_EVENTS_ATTRIBUTE)
  if (state == SL_ZIGBEE_ZCL_DOOR_STATE_OPEN
      || state == SL_ZIGBEE_ZCL_DOOR_STATE_CLOSED) {
    sl_zigbee_af_attribute_id_t attributeId = (state == SL_ZIGBEE_ZCL_DOOR_STATE_OPEN
                                               ? ZCL_DOOR_OPEN_EVENTS_ATTRIBUTE_ID
                                               : ZCL_DOOR_CLOSED_EVENTS_ATTRIBUTE_ID);
    uint32_t events;
    status = sl_zigbee_af_read_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                                ZCL_DOOR_LOCK_CLUSTER_ID,
                                                attributeId,
                                                (uint8_t *)&events,
                                                sizeof(events));
    if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      events++;
      status = sl_zigbee_af_write_server_attribute(DOOR_LOCK_SERVER_ENDPOINT,
                                                   ZCL_DOOR_LOCK_CLUSTER_ID,
                                                   attributeId,
                                                   (uint8_t *)&events,
                                                   ZCL_INT32U_ATTRIBUTE_TYPE);
    }
  }
#endif

  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_lock_door_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_unlock_door_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_unlock_with_timeout_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_log_record_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_set_pin_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_pin_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_pin_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_all_pins_cb(void);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_set_weekday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_weekday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_weekday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_set_yearday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_yearday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_yearday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_set_holiday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_holiday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_holiday_schedule_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_set_user_type_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_user_type_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_set_rfid_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_rfid_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_rfid_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_all_rfids_cb(void);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_set_disposable_schedule_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_get_disposable_schedule_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_disposable_schedule_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_biometric_credential_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_door_lock_cluster_clear_all_biometric_credentials_cb(void);

uint32_t sl_zigbee_af_door_lock_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                             sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_LOCK_DOOR_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_lock_door_cb(cmd);
        break;
      }
      case ZCL_UNLOCK_DOOR_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_unlock_door_cb(cmd);
        break;
      }
      case ZCL_UNLOCK_WITH_TIMEOUT_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_unlock_with_timeout_cb(cmd);
        break;
      }
      case ZCL_GET_LOG_RECORD_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_get_log_record_cb(cmd);
        break;
      }
      case ZCL_SET_PIN_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_set_pin_cb(cmd);
        break;
      }
      case ZCL_GET_PIN_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_get_pin_cb(cmd);
        break;
      }
      case ZCL_CLEAR_PIN_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_clear_pin_cb(cmd);
        break;
      }
      case ZCL_CLEAR_ALL_PINS_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_clear_all_pins_cb();
        break;
      }
      case ZCL_SET_WEEKDAY_SCHEDULE_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_set_weekday_schedule_cb(cmd);
        break;
      }
      case ZCL_GET_WEEKDAY_SCHEDULE_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_get_weekday_schedule_cb(cmd);
        break;
      }
      case ZCL_CLEAR_WEEKDAY_SCHEDULE_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_clear_weekday_schedule_cb(cmd);
        break;
      }
      case ZCL_SET_YEARDAY_SCHEDULE_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_set_yearday_schedule_cb(cmd);
        break;
      }
      case ZCL_GET_YEARDAY_SCHEDULE_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_get_yearday_schedule_cb(cmd);
        break;
      }
      case ZCL_CLEAR_YEARDAY_SCHEDULE_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_clear_yearday_schedule_cb(cmd);
        break;
      }
      case ZCL_SET_HOLIDAY_SCHEDULE_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_set_holiday_schedule_cb(cmd);
        break;
      }
      case ZCL_GET_HOLIDAY_SCHEDULE_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_get_holiday_schedule_cb(cmd);
        break;
      }
      case ZCL_CLEAR_HOLIDAY_SCHEDULE_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_clear_holiday_schedule_cb(cmd);
        break;
      }
      case ZCL_SET_USER_TYPE_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_set_user_type_cb(cmd);
        break;
      }
      case ZCL_GET_USER_TYPE_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_get_user_type_cb(cmd);
        break;
      }
      case ZCL_SET_RFID_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_set_rfid_cb(cmd);
        break;
      }
      case ZCL_GET_RFID_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_get_rfid_cb(cmd);
        break;
      }
      case ZCL_CLEAR_RFID_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_clear_rfid_cb(cmd);
        break;
      }
      case ZCL_CLEAR_ALL_RFIDS_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_clear_all_rfids_cb();
        break;
      }
      case ZCL_SET_DISPOSABLE_SCHEDULE_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_set_disposable_schedule_cb(cmd);
        break;
      }
      case ZCL_GET_DISPOSABLE_SCHEDULE_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_get_disposable_schedule_cb(cmd);
        break;
      }
      case ZCL_CLEAR_DISPOSABLE_SCHEDULE_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_clear_disposable_schedule_cb(cmd);
        break;
      }
      case ZCL_CLEAR_BIOMETRIC_CREDENTIAL_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_clear_biometric_credential_cb(cmd);
        break;
      }
      case ZCL_CLEAR_ALL_BIOMETRIC_CREDENTIALS_COMMAND_ID:
      {
        status = sl_zigbee_af_door_lock_cluster_clear_all_biometric_credentials_cb();
        break;
      }
    }
  }

  return status;
}
