/***************************************************************************//**
 * @file
 * @brief A list of all devices known in the network and their services.
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
#include "app/framework/plugin/device-database/device-database.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"

#include "device-database-config.h"

//============================================================================
// Globals

typedef struct {
  sl_zigbee_af_device_discovery_status_t status;
  const char* string;
} sli_zigbee_discovery_status_code_to_string_t;

static const sli_zigbee_discovery_status_code_to_string_t statusToString[] = {
  { SL_ZIGBEE_AF_DEVICE_DISCOVERY_STATUS_NONE, "None" },
  { SL_ZIGBEE_AF_DEVICE_DISCOVERY_STATUS_NEW, "New"  },
  { SL_ZIGBEE_AF_DEVICE_DISCOVERY_STATUS_FIND_ENDPOINTS, "Find Endpoints" },
  { SL_ZIGBEE_AF_DEVICE_DISCOVERY_STATUS_FIND_CLUSTERS, "Find Clusters"  },
  { SL_ZIGBEE_AF_DEVICE_DISCOVERY_STATUS_FIND_STACK_REVISION, "Find Stack Revision" },
  { SL_ZIGBEE_AF_DEVICE_DISCOVERY_STATUS_DONE, "Done" },
  { SL_ZIGBEE_AF_DEVICE_DISCOVERY_STATUS_FAILED, "Failed" },
};

const char unknownStatus[] = "???";

//============================================================================

const char* device_database_get_status_string(sl_zigbee_af_device_discovery_status_t status)
{
  uint8_t i;
  for (i = 0; i < sizeof(statusToString) / sizeof(sli_zigbee_discovery_status_code_to_string_t); i++) {
    if (status == statusToString[i].status) {
      return statusToString[i].string;
    }
  }
  return unknownStatus;
}

void sli_zigbee_af_device_database_print_all(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  uint8_t i;
  uint8_t deviceCount = 0;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DEVICE_DATABASE_MAX_DEVICES; i++) {
    const sl_zigbee_af_device_info_t* device = sl_zigbee_af_device_database_get_device_by_index(i);
    if (device == NULL) {
      continue;
    }
    sl_zigbee_af_core_print("%d - ", i);
    deviceCount++;

    sl_zigbee_af_print_big_endian_eui64(device->eui64);
    sl_zigbee_af_core_println(" - Capabilities: 0x%02X - EPs: %d - Status: %s - Failures:%d - Stack Revision: %d",
                              device->capabilities,
                              device->endpointCount,
                              device_database_get_status_string(device->status),
                              device->discoveryFailures,
                              device->stackRevision);
  }
  sl_zigbee_af_core_println("%d of %d devices in database", deviceCount, SL_ZIGBEE_AF_PLUGIN_DEVICE_DATABASE_MAX_DEVICES);
}

void sli_zigbee_af_device_database_print_device(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t eui64;
  sl_zigbee_copy_eui64_arg(arguments, 0, eui64, true);
  const sl_zigbee_af_device_info_t* device = sl_zigbee_af_device_database_find_device_by_eui64(eui64);
  if (device == NULL) {
    sl_zigbee_af_core_println("Error: Cannot find device in database.");
    return;
  }

  uint8_t i;
  for (i = 0; i < device->endpointCount; i++) {
    sl_zigbee_af_core_println("EP: %d - Profile ID: 0x%04X - Device ID: 0x%04X - Clusters: %d",
                              device->endpoints[i].endpoint,
                              device->endpoints[i].profileId,
                              device->endpoints[i].deviceId,
                              device->endpoints[i].clusterCount);
    uint8_t j;
    for (j = 0; j < device->endpoints[i].clusterCount; j++) {
      sl_zigbee_af_core_println("  Cluster: 0x%04X %s",
                                device->endpoints[i].clusters[j].clusterId,
                                (device->endpoints[i].clusters[j].server
                                 ? "Server"
                                 : "Client"));
    }
  }
  sl_zigbee_af_core_println("%d total endpoints", device->endpointCount);
}

#define DUMMY_BASE_ENDPOINT    100
#define DUMMY_BASE_PROFILE_ID  0xA000
#define DUMMY_BASE_DEVICE_ID   0x0200
#define DUMMY_BASE_CLUSTER_ID  0x0000
#define DUMMY_STACK_REVISION   21

void sli_zigbee_af_device_database_add_dummy_device(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_device_info_t dummy;
  memset(&dummy, 0, sizeof(sl_zigbee_af_device_info_t));
  sl_zigbee_copy_eui64_arg(arguments, 0, dummy.eui64, true);
  dummy.endpointCount = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t clusterCount = sl_cli_get_argument_uint8(arguments, 2);
  dummy.stackRevision = DUMMY_STACK_REVISION;

  if (dummy.endpointCount > SL_ZIGBEE_AF_MAX_ENDPOINTS_PER_DEVICE) {
    sl_zigbee_af_core_println("Error: Cannot add more than %d endpoints", SL_ZIGBEE_AF_MAX_ENDPOINTS_PER_DEVICE);
    return;
  }

  if ((uint16_t)dummy.endpointCount + (uint16_t)DUMMY_BASE_ENDPOINT >= 255) {
    sl_zigbee_af_core_println("Error:  Base endpoint of %d + number of endpoints %d would exceed 254",
                              DUMMY_BASE_ENDPOINT,
                              dummy.endpointCount);
    return;
  }

  if (clusterCount > SL_ZIGBEE_AF_MAX_CLUSTERS_PER_ENDPOINT) {
    sl_zigbee_af_core_println("Error: Cannot add more than %d clusters per endpoint.", SL_ZIGBEE_AF_MAX_CLUSTERS_PER_ENDPOINT);
    return;
  }
  if ((uint32_t)clusterCount + (uint32_t)DUMMY_BASE_CLUSTER_ID >= 65535) {
    sl_zigbee_af_core_println("Error: Base cluster ID 0x%04X + number of clusters %d would exceed 65534",
                              DUMMY_BASE_CLUSTER_ID,
                              clusterCount);
    return;
  }

  dummy.status = SL_ZIGBEE_AF_DEVICE_DISCOVERY_STATUS_DONE;

  uint8_t i;
  for (i = 0; i < dummy.endpointCount; i++) {
    dummy.endpoints[i].endpoint = DUMMY_BASE_ENDPOINT + i;
    dummy.endpoints[i].deviceId = DUMMY_BASE_DEVICE_ID + i;
    dummy.endpoints[i].profileId = DUMMY_BASE_PROFILE_ID + i;
    dummy.endpoints[i].clusterCount = clusterCount;

    uint8_t j;
    for (j = 0; j < clusterCount; j++) {
      dummy.endpoints[i].clusters[j].clusterId = DUMMY_BASE_CLUSTER_ID + j;
      dummy.endpoints[i].clusters[j].server = true;
    }
  }

  if (SL_STATUS_OK != sl_zigbee_af_device_database_add_device_with_all_info(&dummy)) {
    sl_zigbee_af_core_println("Error: Could not add device to database.");
  }
}

void sli_zigbee_af_device_database_erase(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t eui64;
  sl_zigbee_copy_eui64_arg(arguments, 0, eui64, true);
  if (!sl_zigbee_af_device_database_erase_device(eui64)) {
    sl_zigbee_af_core_println("Error:  Could not delete device.");
  }
}
