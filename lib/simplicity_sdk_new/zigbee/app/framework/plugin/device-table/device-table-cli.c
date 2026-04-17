/***************************************************************************//**
 * @file
 * @brief CLI for the Device Table plugin.
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

#include PLATFORM_HEADER
#ifdef EZSP_HOST
// Includes needed for functions related to the EZSP host
  #include "stack/include/sl_zigbee_types.h"
  #include "app/util/ezsp/ezsp-protocol.h"
  #include "app/util/ezsp/ezsp.h"
  #include "app/util/ezsp/serial-interface.h"
  #include "app/util/zigbee-framework/zigbee-device-common.h"
#else
  #include "stack/include/sl_zigbee.h"
#endif

#include "hal/hal.h"
#include "app/framework/include/af.h"

#include "app/util/serial/sl_zigbee_command_interpreter.h"

#include <stdlib.h>
#include "app/framework/plugin/device-table/device-table.h"
#include "app/framework/plugin/device-table/device-table-internal.h"

static sl_802154_short_addr_t discoverNode;

// --------------------------------
// Print commands
void sli_zigbee_af_device_table_print_eui64(uint8_t *eui64)
{
  uint8_t i;
  for (i = 8; i > 0; i--) {
    sl_zigbee_af_core_print("%02X", eui64[i - 1]);
  }
}

static void printState(sl_zigbee_af_plugin_device_table_device_state_t state)
{
  switch (state) {
    case SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_JOINED:
      sl_zigbee_af_core_print(" JOINED");
      break;
    case SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_UNRESPONSIVE:
      sl_zigbee_af_core_print(" UNRESPONSIVE");
      break;
    case SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_LEAVE_SENT:
      sl_zigbee_af_core_print(" LEAVE_SENT");
      break;
    case SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_LEFT:
      sl_zigbee_af_core_print(" LEFT");
      break;
    default:
      break;
  }
}

static void printDeviceId(uint16_t deviceId)
{
  switch (deviceId) {
    case DEVICE_ID_ON_OFF_SWITCH:
      sl_zigbee_af_core_print("ON_OFF_SWITCH");
      break;

    case DEVICE_ID_LEVEL_CONTROL_SWITCH:
      sl_zigbee_af_core_print("LEVEL_CONTROL_SWITCH");
      break;

    case DEVICE_ID_ON_OFF_OUTPUT:
      sl_zigbee_af_core_print("ON_OFF_OUTPUT");
      break;

    case DEVICE_ID_LEVEL_CONTROL_OUTPUT:
      sl_zigbee_af_core_print("LEVEL_CONTROL_OUTPUT");
      break;

    case DEVICE_ID_SCENE_SELECTOR:
      sl_zigbee_af_core_print("SCENE_SELECTOR");
      break;

    case DEVICE_ID_CONFIG_TOOL:
      sl_zigbee_af_core_print("CONFIG_TOOL");
      break;

    case DEVICE_ID_REMOTE_CONTROL:
      sl_zigbee_af_core_print("REMOTE_CONTROL");
      break;

    case DEVICE_ID_COMBINED_INTERFACE:
      sl_zigbee_af_core_print("COMBINED_INTERFACE");
      break;

    case DEVICE_ID_RANGE_EXTENDER:
      sl_zigbee_af_core_print("RANGE_EXTENDER");
      break;

    case DEVICE_ID_MAINS_POWER_OUTLET:
      sl_zigbee_af_core_print("MAINS_POWER_OUTLET");
      break;

    case DEVICE_ID_DOOR_LOCK:
      sl_zigbee_af_core_print("DOOR_LOCK");
      break;

    case DEVICE_ID_DOOR_LOCK_CONTROLLER:
      sl_zigbee_af_core_print("DOOR_LOCK_CONTROLLER");
      break;

    case DEVICE_ID_SIMPLE_SENSOR:
      sl_zigbee_af_core_print("SIMPLE_SENSOR");
      break;

    case DEVICE_ID_CONSUMPTION_AWARENESS_DEVICE:
      sl_zigbee_af_core_print("CONSUMPTION_AWARENESS_DEVICE");
      break;

    case DEVICE_ID_HOME_GATEWAY:
      sl_zigbee_af_core_print("HOME_GATEWAY");
      break;

    case DEVICE_ID_SMART_PLUG:
      sl_zigbee_af_core_print("SMART_PLUG");
      break;

    case DEVICE_ID_WHITE_GOODS:
      sl_zigbee_af_core_print("WHITE_GOODS");
      break;

    case DEVICE_ID_METER_INTERFACE:
      sl_zigbee_af_core_print("METER_INTERFACE");
      break;

    case DEVICE_ID_ON_OFF_LIGHT:
      sl_zigbee_af_core_print("ON_OFF_LIGHT");
      break;

    case DEVICE_ID_DIMMABLE_LIGHT:
      sl_zigbee_af_core_print("DIMMABLE_LIGHT");
      break;

    case DEVICE_ID_COLOR_DIMMABLE_LIGHT:
      sl_zigbee_af_core_print("COLOR_DIMMABLE_LIGHT");
      break;

    case DEVICE_ID_ON_OFF_LIGHT_SWITCH:
      sl_zigbee_af_core_print("ON_OFF_LIGHT_SWITCH");
      break;

    case DEVICE_ID_DIMMER_SWITCH:
      sl_zigbee_af_core_print("DIMMER_SWITCH");
      break;

    case DEVICE_ID_COLOR_DIMMER_SWITCH:
      sl_zigbee_af_core_print("COLOR_DIMMER_SWITCH");
      break;

    case DEVICE_ID_LIGHT_SENSOR:
      sl_zigbee_af_core_print("LIGHT_SENSOR");
      break;

    case DEVICE_ID_OCCUPANCY_SENSOR:
      sl_zigbee_af_core_print("OCCUPANCY_SENSOR");
      break;

    case DEVICE_ID_SHADE:
      sl_zigbee_af_core_print("SHADE");
      break;

    case DEVICE_ID_SHADE_CONTROLLER:
      sl_zigbee_af_core_print("SHADE_CONTROLLER");
      break;

    case DEVICE_ID_WINDOW_COVERING_DEVICE:
      sl_zigbee_af_core_print("WINDOW_COVERING_DEVICE");
      break;

    case DEVICE_ID_WINDOW_COVERING_CONTROLLER:
      sl_zigbee_af_core_print("WINDOW_COVERING_CONTROLLER");
      break;

    case DEVICE_ID_HEATING_COOLING_UNIT:
      sl_zigbee_af_core_print("HEATING_COOLING_UNIT");
      break;

    case DEVICE_ID_THERMOSTAT:
      sl_zigbee_af_core_print("THERMOSTAT");
      break;

    case DEVICE_ID_TEMPERATURE_SENSOR:
      sl_zigbee_af_core_print("TEMPERATURE_SENSOR");
      break;

    case DEVICE_ID_PUMP:
      sl_zigbee_af_core_print("PUMP");
      break;

    case DEVICE_ID_PUMP_CONTROLLER:
      sl_zigbee_af_core_print("PUMP_CONTROLLER");
      break;

    case DEVICE_ID_PRESSURE_SENSOR:
      sl_zigbee_af_core_print("PRESSURE_SENSOR");
      break;

    case DEVICE_ID_FLOW_SENSOR:
      sl_zigbee_af_core_print("FLOW_SENSOR");
      break;

    case DEVICE_ID_MINI_SPLIT_AC:
      sl_zigbee_af_core_print("MINI_SPLIT_AC");
      break;

    case DEVICE_ID_IAS_CIE:
      sl_zigbee_af_core_print("IAS_CIE");
      break;

    case DEVICE_ID_IAS_ANCILLARY_CONTROL:
      sl_zigbee_af_core_print("IAS_ANCILLARY_CONTROL");
      break;

    case DEVICE_ID_IAS_ZONE:
      sl_zigbee_af_core_print("IAS_ZONE");
      break;

    case DEVICE_ID_IAS_WARNING:
      sl_zigbee_af_core_print("IAS_WARNING");
      break;

    default:
      break;
  }
}

void sl_zigbee_af_device_table_print_device_table(void)
{
  uint16_t totalDevices = 0;
  sl_zigbee_af_plugin_device_table_entry_t *deviceTable = sl_zigbee_af_device_table_pointer();
  uint16_t index;

  for (index = 0;
       index < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE;
       index++) {
    if (deviceTable[index].nodeId != SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
      sl_zigbee_af_core_print("%d %04X:  ", totalDevices, deviceTable[index].nodeId);
      sli_zigbee_af_device_table_print_eui64(deviceTable[index].eui64);
      sl_zigbee_af_core_print(" %d ", deviceTable[index].endpoint);
      printDeviceId(deviceTable[index].deviceId);
      printState(deviceTable[index].state);
      sl_zigbee_af_core_println(" %ld", sl_zigbee_af_device_table_time_since_last_message(index));
      totalDevices++;
    }
  }
  sl_zigbee_af_core_println("Total Devices %d", totalDevices);
}

void sli_zigbee_af_device_table_print_buffer(uint8_t *buffer, uint16_t bufLen)
{
  int i;
  for (i = 0; i < bufLen; i++) {
    sl_zigbee_af_core_print("%02X ", buffer[i]);
  }
  sl_zigbee_af_core_println("");
}

// --------------------------------
// CLI section
static void deviceTabeCliServiceDiscoveryCallback(const sl_zigbee_af_service_discovery_result_t* result)
{
  if (result->zdoRequestClusterId == IEEE_ADDRESS_REQUEST) {
    if (result->status == SL_ZIGBEE_AF_UNICAST_SERVICE_DISCOVERY_TIMEOUT) {
      sl_zigbee_af_core_println("Unicast Timeout");
      return;
    }

    uint8_t* eui64ptr = (uint8_t*)(result->responseData);
    sl_zigbee_af_device_table_new_device_join_handler(discoverNode, eui64ptr);
  }
}

void sli_zigbee_af_device_table_index_remove_command(sl_cli_command_arg_t *arguments)
{
  uint16_t index = sl_cli_get_argument_uint8(arguments, 0);
  sli_zigbee_af_device_table_delete_entry(index);
}

void sli_zigbee_af_device_table_remove_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t eui64;
  uint16_t index;

  sl_zigbee_copy_eui64_arg(arguments, 0, eui64, true);
  index = sl_zigbee_af_device_table_get_first_index_from_eui64(eui64);

  while (index != SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    sli_zigbee_af_device_table_delete_entry(index);
    index = sli_zigbee_af_device_table_find_next_endpoint(index);
  }
}

void sli_zigbee_af_device_table_index_route_repair_command(sl_cli_command_arg_t *arguments)
{
  uint16_t index = sl_cli_get_argument_uint8(arguments, 0);
  uint16_t nodeId = sl_zigbee_af_device_table_get_node_id_from_index(index);

  if (nodeId != SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
    sl_zigbee_af_core_println("ROUTE REPAIR:  CLI");
    sl_zigbee_af_device_table_initiate_route_repair(nodeId);
  }
}

void sli_zigbee_af_device_table_route_repair_command(sl_cli_command_arg_t *arguments)
{
  uint16_t nodeId;
  sl_802154_long_addr_t eui64;

  sl_zigbee_copy_eui64_arg(arguments, 0, eui64, true);
  nodeId = sl_zigbee_af_device_table_get_node_id_from_eui64(eui64);

  if (nodeId != SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
    sl_zigbee_af_core_println("ROUTE REPAIR:  CLI");
    sl_zigbee_af_device_table_initiate_route_repair(nodeId);
  }
}

void sli_zigbee_af_device_table_discover_present_node_command(sl_cli_command_arg_t *arguments)
{
  discoverNode = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(arguments, 0);
  sl_zigbee_af_find_ieee_address(discoverNode,
                                 deviceTabeCliServiceDiscoveryCallback);
}

void sli_zigbee_af_device_table_index_send_command(sl_cli_command_arg_t *arguments)
{
  uint16_t index = (uint16_t)sl_cli_get_argument_uint16(arguments, 0);
  sl_zigbee_af_device_table_cli_index_send(index);
}

void sli_zigbee_af_device_table_send_command(sl_cli_command_arg_t *arguments)
{
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 1);
  sl_802154_long_addr_t eui64;

  sl_zigbee_copy_eui64_arg(arguments, 0, eui64, true);
  sl_zigbee_af_device_table_send(eui64, endpoint);
}

void sli_zigbee_af_device_table_save_command(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_af_device_table_save();
}

void sli_zigbee_af_device_table_load_command(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_af_device_table_load();
}

void sli_zigbee_af_device_table_send_leave_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t eui64;
  sl_zigbee_copy_eui64_arg(arguments, 0, eui64, true);
}
