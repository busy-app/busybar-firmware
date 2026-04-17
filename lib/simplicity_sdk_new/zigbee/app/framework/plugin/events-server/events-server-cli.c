/***************************************************************************//**
 * @file
 * @brief CLI for the Events Server plugin.
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
#include "events-server.h"

// plugin events-server clear <endpoint:1> <logId:1>
void sli_zigbee_af_events_server_cli_clear(sl_cli_command_arg_t *arguments)
{
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_event_log_id_t logId = (sl_zigbee_af_event_log_id_t)sl_cli_get_argument_uint8(arguments, 1);
  sl_zigbee_af_events_server_clear_event_log(endpoint, logId);
}

// plugin events-server print <endpoint:1> <logId:1>
void sli_zigbee_af_events_server_cli_print(sl_cli_command_arg_t *arguments)
{
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_event_log_id_t logId = (sl_zigbee_af_event_log_id_t)sl_cli_get_argument_uint8(arguments, 1);
  sl_zigbee_af_events_server_print_event_log(endpoint, logId);
}

// plugin events-server eprint <endpoint:1> <logId:1> <index:1>
void sli_zigbee_af_events_server_cli_print_event(sl_cli_command_arg_t *arguments)
{
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_event_log_id_t logId = (sl_zigbee_af_event_log_id_t)sl_cli_get_argument_uint8(arguments, 1);
  uint8_t index = sl_cli_get_argument_uint8(arguments, 2);
  sl_zigbee_af_zcl_event_t event;
  if (sl_zigbee_af_events_server_get_event(endpoint, logId, index, &event)) {
    sl_zigbee_af_events_cluster_println("Event at index 0x%02X in log 0x%02X", index, logId);
    sl_zigbee_af_events_server_print_event(&event);
  } else {
    sl_zigbee_af_events_cluster_println("Event at index 0x%02X in log 0x%02X is not present", index, logId);
  }
}

// plugin events-server set <endpoint:1> <logId:1> <index:1> <eventId:2> <eventTime:4> <data:?>
void sli_zigbee_af_events_server_cli_set(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_zcl_event_t event;
  uint8_t length;
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_event_log_id_t logId = (sl_zigbee_af_event_log_id_t)sl_cli_get_argument_uint8(arguments, 1);
  uint8_t index = sl_cli_get_argument_uint8(arguments, 2);
  event.eventId = sl_cli_get_argument_uint16(arguments, 3);
  event.eventTime = sl_cli_get_argument_uint32(arguments, 4);
  length = sl_zigbee_copy_string_arg(arguments, 5,
                                     event.eventData + 1,
                                     SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_EVENT_DATA_LENGTH, false);
  event.eventData[0] = length;
  if (!sl_zigbee_af_events_server_set_event(endpoint, logId, index, &event)) {
    sl_zigbee_af_events_cluster_println("Event at index 0x%02X in log 0x%02X is not present", index, logId);
  } else {
    sl_zigbee_af_events_cluster_println("Event added to log 0x%02X at index 0x%02X", logId, index);
  }
}

// plugin events-server add <endpoint:1> <logId:1> <eventId:2> <eventTime:4> <data:?>
void sli_zigbee_af_events_server_cli_add(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_zcl_event_t event;
  uint8_t length;
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_event_log_id_t logId = (sl_zigbee_af_event_log_id_t)sl_cli_get_argument_uint8(arguments, 1);
  event.eventId = sl_cli_get_argument_uint16(arguments, 2);
  event.eventTime = sl_cli_get_argument_uint32(arguments, 3);
  length = sl_zigbee_copy_string_arg(arguments, 4,
                                     event.eventData + 1,
                                     SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_EVENT_DATA_LENGTH, false);
  event.eventData[0] = length;
  sl_zigbee_af_events_server_add_event(endpoint, logId, &event);
}

// plugin events-server publish <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <logId:1> <index:1> <eventControl:1>
void sli_zigbee_af_events_server_cli_publish(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t nodeId = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(arguments, 0);
  uint8_t srcEndpoint = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t dstEndpoint = sl_cli_get_argument_uint8(arguments, 2);
  sl_zigbee_af_event_log_id_t logId = (sl_zigbee_af_event_log_id_t)sl_cli_get_argument_uint8(arguments, 3);
  uint8_t index = sl_cli_get_argument_uint8(arguments, 4);
  uint8_t eventControl = sl_cli_get_argument_uint8(arguments, 5);
  sl_zigbee_af_events_server_publish_event_message(nodeId,
                                                   srcEndpoint,
                                                   dstEndpoint,
                                                   logId,
                                                   index,
                                                   eventControl);
}
