/***************************************************************************//**
 * @file
 * @brief Routines for the Device Table plugin.
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

#include "app/framework/include/af.h"
#include "af-main.h"
#include "app/framework/util/attribute-storage.h"
#include "app/util/common/common.h"
#include "hal/hal.h"

#include "app/framework/plugin/device-table/device-table.h"
#include "app/framework/plugin/device-table/device-table-internal.h"
#include "app/framework/util/util.h"
#include <stdlib.h>

void sli_zigbee_af_device_table_save(void);
void sli_zigbee_af_device_table_load(void);

// Framework message send global data
extern uint8_t appZclBuffer[];
extern uint16_t appZclBufferLen;
extern bool zclCmdIsBuilt;
extern sl_zigbee_aps_frame_t globalApsFrame;

extern void sli_zigbee_af_aps_frame_endpoint_setup(uint8_t srcEndpoint,
                                                   uint8_t dstEndpoint);
extern void sl_zigbee_af_device_table_initialized(void);

extern void sl_zigbee_af_device_table_index_removed_cb(uint16_t index);
static sl_zigbee_af_plugin_device_table_entry_t deviceTable[SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE];

sl_status_t sl_zigbee_af_get_child_data(uint8_t index,
                                        sl_zigbee_child_data_t *childData);

// Device discovery global declarations
void sli_zigbee_af_device_table_initiate_route_repair(sl_802154_short_addr_t nodeId);
static void clearDeviceTableIndex(uint16_t index);

sl_zigbee_af_plugin_device_table_entry_t* sl_zigbee_af_device_table_pointer(void)
{
  return deviceTable;
}

uint16_t sl_zigbee_af_device_table_get_node_id_from_index(uint16_t index)
{
  sl_zigbee_af_plugin_device_table_entry_t *deviceTable = sl_zigbee_af_device_table_pointer();
  assert(index < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE);
  return deviceTable[index].nodeId;
}

uint8_t sli_zigbee_af_device_table_get_first_endpoint_from_index(uint16_t index)
{
  assert(index < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE);
  return deviceTable[index].endpoint;
}

static void matchReverseEui64(sl_802154_long_addr_t eui64a, sl_802154_long_addr_t eui64b)
{
  uint8_t i;

  for (i = 0; i < EUI64_SIZE; i++) {
    if (eui64a[i] != eui64b[(EUI64_SIZE - 1) - i]) {
      return;
    }
  }

  sl_zigbee_af_core_println("MATCH_EUI:  EUI matches backwards");

  sl_zigbee_af_core_print("A:");
  sli_zigbee_af_device_table_print_eui64(eui64a);
  sl_zigbee_af_core_print(" B:");
  sli_zigbee_af_device_table_print_eui64(eui64b);
  sl_zigbee_af_core_println("");
}

static void checkNullEui64(sl_802154_long_addr_t eui64a, sl_802154_long_addr_t eui64b)
{
  uint8_t i;
  for (i = 0; i < EUI64_SIZE; i++) {
    if (eui64a[i] != 0xff
        || eui64b[i] != 0xff) {
      return;
    }
  }

  sl_zigbee_af_core_println("MatchEUI:  two null EUI");
}

static bool matchEui64(sl_802154_long_addr_t a, sl_802154_long_addr_t b)
{
  checkNullEui64(a, b);

  if (memcmp(a, b, EUI64_SIZE) == 0) {
    return true;
  } else {
    // Test to see if the EUI64 is backwards
    matchReverseEui64(a, b);

    return false;
  }
}

bool sl_zigbee_af_device_table_match_eui64(sl_802154_long_addr_t eui64a, sl_802154_long_addr_t eui64b)
{
  return matchEui64(eui64a, eui64b);
}

static void unsetEui64(sl_802154_long_addr_t eui64)
{
  uint8_t i;
  for (i = 0; i < 8; i++) {
    eui64[i] = 0xff;
  }
}

static void clearDeviceTableIndex(uint16_t index)
{
  uint8_t i;

  assert(index < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE);

  deviceTable[index].nodeId = SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID;
  unsetEui64(deviceTable[index].eui64);
  deviceTable[index].state = SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_NULL;
  deviceTable[index].endpoint = 0;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE; i++) {
    deviceTable[index].clusterIds[i] = ZCL_NULL_CLUSTER_ID;
  }
  deviceTable[index].clusterOutStartPosition = 0;
}

void sli_zigbee_af_device_table_delete_entry(uint16_t index)
{
  uint16_t currentIndex;

  while (index != SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    currentIndex = index;

    // Need to compute the next index before deleting the current one.  Or else
    // the call to next endpoint will yield a bogus result.
    index = sli_zigbee_af_device_table_find_next_endpoint(index);

    clearDeviceTableIndex(currentIndex);
    sl_zigbee_af_device_table_index_removed_cb(currentIndex);
  }
}

void sli_zigbee_af_device_table_init(void)
{
  uint16_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    clearDeviceTableIndex(i);
  }
}

void sl_zigbee_af_device_table_clear(void)
{
  sli_zigbee_af_device_table_init();
  sli_zigbee_af_device_table_save();
  sl_zigbee_af_device_table_cleared_cb();
}

uint16_t sl_zigbee_af_device_table_get_index_from_eui64_and_endpoint(sl_802154_long_addr_t eui64,
                                                                     uint8_t endpoint)
{
  uint16_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (matchEui64(deviceTable[i].eui64, eui64)
        && deviceTable[i].endpoint == endpoint) {
      return i;
    }
  }
  return SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX;
}

uint16_t sl_zigbee_af_device_table_get_node_id_from_eui64(sl_802154_long_addr_t eui64)
{
  uint16_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (matchEui64(deviceTable[i].eui64, eui64) ) {
      return deviceTable[i].nodeId;
    }
  }
  return SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID;
}

bool sl_zigbee_af_device_table_get_eui64_from_node_id(sl_802154_short_addr_t sl_zigbee_node_id, sl_802154_long_addr_t eui64)
{
  uint16_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (deviceTable[i].nodeId == sl_zigbee_node_id) {
      memcpy(eui64, deviceTable[i].eui64, EUI64_SIZE);
      return true;
    }
  }
  return false;
}

uint16_t sl_zigbee_af_device_table_get_index_from_node_id(sl_802154_short_addr_t sl_zigbee_node_id)
{
  uint16_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (deviceTable[i].nodeId == sl_zigbee_node_id) {
      return i;
    }
  }
  return SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX;
}

uint16_t sli_zigbee_af_device_table_find_free_device_table_index(void)
{
  uint16_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (deviceTable[i].nodeId == SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
      return i;
    }
  }
  return SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX;
}

uint16_t sl_zigbee_af_device_table_get_endpoint_from_node_id_and_endpoint(sl_802154_short_addr_t sl_zigbee_node_id,
                                                                          uint8_t endpoint)
{
  uint16_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (deviceTable[i].nodeId == sl_zigbee_node_id
        && deviceTable[i].endpoint == endpoint) {
      return i;
    }
  }
  return SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX;
}

void sli_zigbee_af_device_table_copy_device_table_entry(uint16_t fromIndex, uint16_t toIndex)
{
  sl_zigbee_af_plugin_device_table_entry_t* from = &(deviceTable[fromIndex]);
  sl_zigbee_af_plugin_device_table_entry_t* to = &(deviceTable[toIndex]);

  // make sure the fromIndex is in the valud range.
  assert(fromIndex < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE);

  // make sure the fromIndex has a valid entry
  assert(deviceTable[fromIndex].nodeId
         != SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID);

  // make sure the toIndex is in the valud range.
  assert(toIndex < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE);

  memcpy(to, from, sizeof(sl_zigbee_af_plugin_device_table_entry_t));
}

uint8_t sli_zigbee_af_device_table_number_of_endpoints_from_index(uint16_t index)
{
  uint8_t count = 0;
  uint16_t currentNodeId = sl_zigbee_af_device_table_get_node_id_from_index(index);
  uint16_t i;

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (deviceTable[i].nodeId == currentNodeId) {
      count++;
    }
  }
  return count;
}

static uint16_t findIndexFromNodeIdAndIndex(uint16_t nodeId, uint16_t index)
{
  uint16_t i;
  for (i = index; i < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (nodeId == sl_zigbee_af_device_table_get_node_id_from_index(i)) {
      return i;
    }
  }
  return SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX;
}

static uint16_t findIndexFromEui64AndIndex(sl_802154_long_addr_t eui64, uint16_t index)
{
  uint16_t i;
  for (i = index; i < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (matchEui64(eui64, deviceTable[i].eui64)) {
      return i;
    }
  }
  return SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX;
}

uint16_t sli_zigbee_af_device_table_find_first_endpoint_node_id(uint16_t nodeId)
{
  return findIndexFromNodeIdAndIndex(nodeId, 0);
}

uint16_t sli_zigbee_af_device_table_find_next_endpoint(uint16_t index)
{
  return findIndexFromEui64AndIndex(deviceTable[index].eui64,
                                    index + 1);
}

uint16_t sli_zigbee_af_device_table_find_first_endpoint_ieee(sl_802154_long_addr_t eui64)
{
  return findIndexFromEui64AndIndex(eui64, 0);
}

uint16_t sl_zigbee_af_device_table_get_first_index_from_eui64(sl_802154_long_addr_t eui64)
{
  return sli_zigbee_af_device_table_find_first_endpoint_ieee(eui64);
}

sl_zigbee_af_status_t sli_zigbee_af_device_table_add_new_endpoint(uint16_t index, uint8_t newEndpoint)
{
  uint16_t newIndex = sli_zigbee_af_device_table_find_free_device_table_index();

  if (newIndex == SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_FAILURE;
  }

  sli_zigbee_af_device_table_copy_device_table_entry(index, newIndex);

  deviceTable[newIndex].endpoint = newEndpoint;

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

uint16_t sli_zigbee_af_device_table_find_index_node_id_endpoint(uint16_t nodeId,
                                                                uint8_t endpoint)
{
  uint16_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (deviceTable[i].nodeId == nodeId
        && deviceTable[i].endpoint == endpoint) {
      return i;
    }
  }

  return SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX;
}

sl_zigbee_af_plugin_device_table_entry_t *sl_zigbee_af_device_table_find_device_table_entry(uint16_t index)
{
  assert(index < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID);
  return &(deviceTable[index]);
}

void sli_zigbee_af_device_table_update_node_id(uint16_t currentNodeId, uint16_t newNodeId)
{
  uint16_t index = sli_zigbee_af_device_table_find_first_endpoint_node_id(currentNodeId);

  while (index != SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    deviceTable[index].nodeId = newNodeId;

    index = sli_zigbee_af_device_table_find_next_endpoint(index);
  }
}

void sli_zigbee_af_device_table_update_device_state(uint16_t index, uint8_t newState)
{
  while (index != SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    deviceTable[index].state = newState;

    index = sli_zigbee_af_device_table_find_next_endpoint(index);
  }
}

uint32_t sl_zigbee_af_device_table_time_since_last_message(uint16_t index)
{
  uint32_t timeSinceLastMessage = halCommonGetInt32uMillisecondTick();

  timeSinceLastMessage -= deviceTable[index].lastMsgTimestamp;
  timeSinceLastMessage /= MILLISECOND_TICKS_PER_SECOND;

  return timeSinceLastMessage;
}

extern sl_zigbee_af_event_t sl_zigbee_af_device_table_new_device_event;
extern void sl_zigbee_af_device_table_new_device_event_handler(sl_zigbee_af_event_t * event);

// AF Framework callbacks.  This is where the plugin implements the callbacks.

void sl_zigbee_af_device_table_init_cb(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
      sl_zigbee_af_event_init(&sl_zigbee_af_device_table_new_device_event,
                              sl_zigbee_af_device_table_new_device_event_handler);
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA:
    {
      sli_zigbee_af_device_table_init();
      sli_zigbee_af_device_table_load();
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_DONE:
    {
      sl_zigbee_af_device_table_initialized();
      break;
    }
  }
}

void sl_zigbee_af_device_table_stack_status_cb(sl_status_t status)
{
  // If we leave the network, this plugin needs to clear out all of its device
  // state.

  sl_zigbee_af_core_println("%d %d", status, sl_zigbee_af_network_state());

  if (status == SL_STATUS_NETWORK_DOWN
      && sl_zigbee_af_network_state() == SL_ZIGBEE_NO_NETWORK) {
    sl_zigbee_af_core_println("DeviceTable: Clear State");

    sl_zigbee_af_device_table_clear();
  }
}

// --------------------------------
// Save/Load the devices
void sli_zigbee_af_device_table_save(void)
{
#if defined(EZSP_HOST) && !defined(SL_ZIGBEE_TEST)
  FILE *fp;
  sl_zigbee_af_plugin_device_table_entry_t *deviceTable = sl_zigbee_af_device_table_pointer();
  uint8_t i;
  uint8_t j;

  // Save device table
  fp = fopen("devices.txt", "w");

  for (i = 0;
       i < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE;
       i++) {
    if (deviceTable[i].nodeId != SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
      fprintf(fp,
              "%02X %02X %02X ",
              deviceTable[i].nodeId,
              deviceTable[i].endpoint,
              deviceTable[i].deviceId);
      for (j = 0; j < 8; j++) {
        fprintf(fp, "%02X ", deviceTable[i].eui64[j]);
      }
      for (j = 0; j < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE; j++) {
        fprintf(fp, "%02X ", deviceTable[i].clusterIds[j]);
      }
      fprintf(fp, "%d ", deviceTable[i].clusterOutStartPosition);
    }
  }

  // Write ffffffff to mark the end
  fprintf(fp, "\r\nffffffff\r\n");
  fclose(fp);

#endif // defined(EZSP_HOST) && !defined(SL_ZIGBEE_TEST)
}

void sli_zigbee_af_device_table_load(void)
{
#if defined(EZSP_HOST) && !defined(SL_ZIGBEE_TEST)
  uint16_t i;
  uint16_t j;
  FILE *fp;
  unsigned int data, data2, data3;
  sl_zigbee_af_plugin_device_table_entry_t *deviceTable = sl_zigbee_af_device_table_pointer();

  fp = fopen("devices.txt", "r");

  if (!fp) {
    return;
  }

  for (i = 0;
       i < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE && feof(fp) == false;
       i++) {
    fscanf(fp, "%02X %02X %02X", &data2, &data, &data3);
    deviceTable[i].endpoint = (uint8_t) data;
    deviceTable[i].nodeId = (uint16_t) data2;
    deviceTable[i].deviceId = (uint16_t) data3;

    if (deviceTable[i].nodeId != SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
      for (j = 0; j < 8; j++) {
        fscanf(fp, "%02X", &data);
        deviceTable[i].eui64[j] = (uint8_t) data;
      }
      for (j = 0; j < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE; j++) {
        fscanf(fp, "%02X", &data);
        deviceTable[i].clusterIds[j] = (uint16_t) data;
      }
      fscanf(fp, "%d", &data);
      deviceTable[i].clusterOutStartPosition = (uint16_t) data;

      deviceTable[i].state = SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_STATE_JOINED;
    }

    deviceTable[i].lastMsgTimestamp = halCommonGetInt32uMillisecondTick();
  }

  fclose(fp);

  // Set the rest of the device table to null.
  for (; i < SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    deviceTable[i].nodeId = SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID;
  }

#endif // #if defined(EZSP_HOST) && !defined(SL_ZIGBEE_TEST)
}

// --------------------------------
// Message send section
// Command to send the CIE IEEE address to the IAS Zone cluster
void sli_zigbee_af_device_table_send_cie_address_write(sl_802154_short_addr_t nodeId, uint8_t endpoint)
{
  sl_802154_long_addr_t eui64;
  uint8_t outgoingBuffer[15];
  uint32_t i;

  sl_zigbee_af_get_eui64(eui64);

  globalApsFrame.options = SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS;
  globalApsFrame.clusterId = ZCL_IAS_ZONE_CLUSTER_ID;
  globalApsFrame.sourceEndpoint = 0x01;
  globalApsFrame.destinationEndpoint = endpoint;

  outgoingBuffer[0] = 0x00;
  outgoingBuffer[1] = sl_zigbee_af_next_sequence();
  outgoingBuffer[2] = ZCL_WRITE_ATTRIBUTES_COMMAND_ID;
  outgoingBuffer[3] = LOW_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID);
  outgoingBuffer[4] = HIGH_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID);
  outgoingBuffer[5] = ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE;

  for (i = 0; i < 8; i++) {
    outgoingBuffer[6 + i] = eui64[i];
  }

  sl_zigbee_af_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                            nodeId,
                            &globalApsFrame,
                            14,
                            outgoingBuffer);
}

void sl_zigbee_af_device_table_cli_index_send_with_endpoint(uint16_t index,
                                                            uint8_t endpoint)
{
  sl_802154_short_addr_t nodeId;

  nodeId = sl_zigbee_af_device_table_get_node_id_from_index(index);
  sli_zigbee_af_aps_frame_endpoint_setup(sl_zigbee_af_primary_endpoint(), endpoint);
  (void)sl_zigbee_af_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                  nodeId,
                                  &globalApsFrame,
                                  appZclBufferLen,
                                  appZclBuffer);

  zclCmdIsBuilt = false;
}

void sl_zigbee_af_device_table_cli_index_send(uint16_t index)
{
  uint8_t endpoint = sli_zigbee_af_device_table_get_first_endpoint_from_index(index);
  sl_zigbee_af_device_table_cli_index_send_with_endpoint(index, endpoint);
}

void sl_zigbee_af_device_table_send(sl_802154_long_addr_t eui64, uint8_t endpoint)
{
  uint16_t index = sl_zigbee_af_device_table_get_first_index_from_eui64(eui64);

  if (index != SL_ZIGBEE_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    sl_zigbee_af_device_table_cli_index_send_with_endpoint(index, endpoint);
  }
}

void sl_zigbee_af_device_table_command_index_send_with_endpoint(uint16_t index,
                                                                uint8_t endpoint)
{
  sl_802154_short_addr_t nodeId;

  nodeId = sl_zigbee_af_device_table_get_node_id_from_index(index);

  if (sl_zigbee_af_current_command() == NULL) {
    sli_zigbee_af_command_aps_frame->sourceEndpoint = sl_zigbee_af_primary_endpoint();
  } else {
    sli_zigbee_af_command_aps_frame->sourceEndpoint = sl_zigbee_af_current_endpoint();
  }

  sli_zigbee_af_command_aps_frame->destinationEndpoint = endpoint;
  sl_zigbee_af_core_println("device table send with ep: 0x%04X, %d",
                            nodeId,
                            endpoint);
  (void)sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);

  zclCmdIsBuilt = false;
}

void sl_zigbee_af_device_table_command_index_send(uint16_t index)
{
  uint8_t endpoint = sli_zigbee_af_device_table_get_first_endpoint_from_index(index);
  sl_zigbee_af_device_table_command_index_send_with_endpoint(index, endpoint);
}

void sl_zigbee_af_device_table_command_send_with_endpoint(sl_802154_long_addr_t eui64,
                                                          uint8_t endpoint)
{
  uint16_t index = sl_zigbee_af_device_table_get_first_index_from_eui64(eui64);
  sl_zigbee_af_device_table_command_index_send_with_endpoint(index, endpoint);
}
