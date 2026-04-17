/***************************************************************************//**
 * @file
 * @brief Routines for the GBCS Device Log plugin.
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
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include "gbcs-device-log.h"

#define GBCS_DEVICE_LOG_ENTRY_INUSE  BIT(1)
#define deviceLogEntryInUse(entry)   ((entry)->flags & GBCS_DEVICE_LOG_ENTRY_INUSE)

typedef struct {
  sl_802154_long_addr_t deviceId;
  sl_zigbee_af_g_b_c_s_device_log_info_t deviceInfo;
  uint8_t flags;
} sli_zigbee_device_log_entry_t;

static sli_zigbee_device_log_entry_t deviceLog[SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE];
#define SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_INVALID_INDEX  0xFF
#define isValidDeviceLogIndex(index) ((index) < SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE)

#define SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_NULL_EUI { 0, 0, 0, 0, 0, 0, 0, 0 }
static const sl_802154_long_addr_t NULL_EUI   = SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_NULL_EUI;
#define isNullEui(eui)               (memcmp(eui, NULL_EUI, EUI64_SIZE) == 0)

#define CONCAT(x, y, z) x ## y ## z
#define GET_DEVICE_TYPE(value) CONCAT(SL_ZIGBEE_AF_GBCS_, value, _DEVICE_TYPE)

static sl_802154_long_addr_t DEVICE1_EUI   = SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE1_EUI;
#define DEVICE1_TYPE GET_DEVICE_TYPE(SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE1_TYPE)
static sl_802154_long_addr_t DEVICE2_EUI   = SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE2_EUI;
#define DEVICE2_TYPE GET_DEVICE_TYPE(SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE2_TYPE)
static sl_802154_long_addr_t DEVICE3_EUI = SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE3_EUI;
#define DEVICE3_TYPE GET_DEVICE_TYPE(SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE3_TYPE)
static sl_802154_long_addr_t DEVICE4_EUI  = SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE4_EUI;
#define DEVICE4_TYPE GET_DEVICE_TYPE(SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE4_TYPE)

//------------------------------------------------------------------------------
// Forward declarations

static uint8_t findDeviceLogEntry(sl_802154_long_addr_t deviceId);
static uint8_t findFreeDeviceLogEntry(void);

//------------------------------------------------------------------------------
// Initialization

void sl_zigbee_af_gbcs_device_log_init_cb(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_gbcs_device_log_reset();
}

//------------------------------------------------------------------------------
// Public API

void sl_zigbee_af_gbcs_device_log_clear(void)
{
  memset(deviceLog, 0, sizeof(sli_zigbee_device_log_entry_t) * SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE);
  sl_zigbee_af_debug_println("GBCS Device Log: Cleared");
}

void sl_zigbee_af_gbcs_device_log_reset(void)
{
  sl_802154_long_addr_t deviceId;
  sl_zigbee_af_g_b_c_s_device_log_info_t deviceInfo;

  sl_zigbee_af_gbcs_device_log_clear();

  if (!isNullEui(DEVICE1_EUI)) {
    deviceInfo.deviceType = DEVICE1_TYPE;
    sl_util_reverse_mem_copy(deviceId, DEVICE1_EUI, EUI64_SIZE);
    sl_zigbee_af_gbcs_device_log_store(deviceId, &deviceInfo);
  }

  if (!isNullEui(DEVICE2_EUI)) {
    deviceInfo.deviceType = DEVICE2_TYPE;
    sl_util_reverse_mem_copy(deviceId, DEVICE2_EUI, EUI64_SIZE);
    sl_zigbee_af_gbcs_device_log_store(deviceId, &deviceInfo);
  }

  if (!isNullEui(DEVICE3_EUI)) {
    deviceInfo.deviceType = DEVICE3_TYPE;
    sl_util_reverse_mem_copy(deviceId, DEVICE3_EUI, EUI64_SIZE);
    sl_zigbee_af_gbcs_device_log_store(deviceId, &deviceInfo);
  }

  if (!isNullEui(DEVICE4_EUI)) {
    deviceInfo.deviceType = DEVICE4_TYPE;
    sl_util_reverse_mem_copy(deviceId, DEVICE4_EUI, EUI64_SIZE);
    sl_zigbee_af_gbcs_device_log_store(deviceId, &deviceInfo);
  }

  sl_zigbee_af_debug_println("GBCS Device Log: Reset complete");
}

uint8_t sl_zigbee_af_gbcs_device_log_max_size(void)
{
  return SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE;
}

uint8_t sl_zigbee_af_gbcs_device_log_count(void)
{
  uint8_t i;
  uint8_t count = 0;

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE; i++) {
    if (deviceLogEntryInUse(&deviceLog[i])) {
      count++;
    }
  }

  sl_zigbee_af_debug_println("GBCS Device Log: Current number of entries is %d", count);
  return count;
}

bool sl_zigbee_af_gbcs_device_log_store(sl_802154_long_addr_t deviceId,
                                        sl_zigbee_af_g_b_c_s_device_log_info_t *deviceInfo)
{
  uint8_t i;

  i = findDeviceLogEntry(deviceId);
  if (isValidDeviceLogIndex(i)) {
    sl_zigbee_af_debug_print("GBCS Device Log: Updating device at index %d. EUI64=", i);
    sl_zigbee_af_debug_debug_exec(sl_zigbee_af_print_big_endian_eui64(deviceId));
    sl_zigbee_af_debug_println(", type=%d", deviceInfo->deviceType);
    deviceLog[i].deviceInfo.deviceType     = deviceInfo->deviceType;
    return true;
  }

  i = findFreeDeviceLogEntry();
  if (isValidDeviceLogIndex(i)) {
    sl_zigbee_af_debug_print("GBCS Device Log: Adding device at index %d. EUI64=", i);
    sl_zigbee_af_debug_debug_exec(sl_zigbee_af_print_big_endian_eui64(deviceId));
    sl_zigbee_af_debug_println(", type=%d", deviceInfo->deviceType);
    memcpy(deviceLog[i].deviceId, deviceId, EUI64_SIZE);
    deviceLog[i].deviceInfo.deviceType     = deviceInfo->deviceType;
    deviceLog[i].flags |= GBCS_DEVICE_LOG_ENTRY_INUSE;
    return true;
  }

  sl_zigbee_af_gbcs_device_log_println("%s%s%s",
                                       "Error: ",
                                       "Cannot add device to GBCS Device Log: ",
                                       "Too many entries");
  return false;
}

bool sl_zigbee_af_gbcs_device_log_remove(sl_802154_long_addr_t deviceId)
{
  uint8_t i;

  i = findDeviceLogEntry(deviceId);
  if (isValidDeviceLogIndex(i)) {
    sl_zigbee_af_debug_print("GBCS Device Log: Removing device at index %d. EUI64=", i);
    sl_zigbee_af_debug_debug_exec(sl_zigbee_af_print_big_endian_eui64(deviceId));
    sl_zigbee_af_debug_println("");
    memset(&deviceLog[i], 0, sizeof(sli_zigbee_device_log_entry_t));
    sl_zigbee_af_gbcs_device_log_device_removed_cb(deviceId);
    return true;
  }

  sl_zigbee_af_gbcs_device_log_println("%s%s%s",
                                       "Error: ",
                                       "Cannot remove device from GBCS Device Log: ",
                                       "Device does not exist");
  return false;
}

bool sl_zigbee_af_gbcs_device_log_retrieve_by_index(uint8_t index,
                                                    sl_802154_long_addr_t returnDeviceId,
                                                    sl_zigbee_af_g_b_c_s_device_log_info_t* returnDeviceInfo)
{
  if (!isValidDeviceLogIndex(index)) {
    return false;
  }

  if (!deviceLogEntryInUse(&deviceLog[index])) {
    return false;
  }

  memcpy(returnDeviceId, deviceLog[index].deviceId, EUI64_SIZE);
  *returnDeviceInfo = deviceLog[index].deviceInfo;
  return true;
}

bool sl_zigbee_af_gbcs_device_log_get(sl_802154_long_addr_t deviceId,
                                      sl_zigbee_af_g_b_c_s_device_log_info_t *deviceInfo)
{
  uint8_t i;

  i = findDeviceLogEntry(deviceId);
  if (isValidDeviceLogIndex(i)) {
    // sl_zigbee_af_debug_print("GBCS Device Log: Retrieving device info at index %d. EUI64=", i);
    // sl_zigbee_af_debug_debug_exec(sl_zigbee_af_print_big_endian_eui64(deviceId));
    // sl_zigbee_af_debug_println(", type=%d, deviceLog[i].deviceInfo.deviceType);
    deviceInfo->deviceType = deviceLog[i].deviceInfo.deviceType;
    return true;
  }

  sl_zigbee_af_gbcs_device_log_println("%s%s%s",
                                       "Error: ",
                                       "Cannot retrieve device info from GBCS Device Log: ",
                                       "Device does not exist");
  return false;
}

bool sl_zigbee_af_gbcs_device_log_exists(sl_802154_long_addr_t deviceId,
                                         sl_zigbee_af_g_b_c_s_device_type_t deviceType)
{
  uint8_t i;

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE; i++) {
    if (deviceLogEntryInUse(&deviceLog[i])
        && (deviceLog[i].deviceInfo.deviceType == deviceType)
        && (memcmp(deviceId, deviceLog[i].deviceId, EUI64_SIZE) == 0)) {
      return true;
    }
  }

  return false;
}

void sl_zigbee_af_gbcs_device_log_print_entries(void)
{
  uint8_t i;
  bool entryPrinted = false;

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE; i++) {
    if (deviceLogEntryInUse(&deviceLog[i])) {
      sl_zigbee_af_gbcs_device_log_print("Device info at index %d. EUI64=", i);
      sl_zigbee_af_gbcs_device_log_debug_exec(sl_zigbee_af_print_big_endian_eui64(deviceLog[i].deviceId));
      sl_zigbee_af_gbcs_device_log_println(", type=%d", deviceLog[i].deviceInfo.deviceType);
      entryPrinted = true;
    }
  }

  if (!entryPrinted) {
    sl_zigbee_af_gbcs_device_log_println("There are currently no entries in the GBCS Device Log");
  }
}

//------------------------------------------------------------------------------
// Internal functions

static uint8_t findDeviceLogEntry(sl_802154_long_addr_t deviceId)
{
  uint8_t i;

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE; i++) {
    if (deviceLogEntryInUse(&deviceLog[i])
        && (memcmp(deviceId, deviceLog[i].deviceId, EUI64_SIZE) == 0)) {
      return i;
    }
  }

  return SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_INVALID_INDEX;
}

static uint8_t findFreeDeviceLogEntry(void)
{
  uint8_t i;

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE; i++) {
    if (!deviceLogEntryInUse(&deviceLog[i])) {
      return i;
    }
  }

  return SL_ZIGBEE_AF_PLUGIN_GBCS_DEVICE_LOG_INVALID_INDEX;
}
