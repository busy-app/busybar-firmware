/***************************************************************************//**
 * @file
 * @brief Bluetooth Peer Manager - filter for scan responses
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include <stdio.h>
#include "sl_component_catalog.h"
#include "sl_common.h"
#include "sl_memory_manager.h"
#include "ble_peer_manager_filter.h"
#include "ble_peer_manager_filter_config.h"

#if defined(SL_CATALOG_APP_LOG_PRESENT) && defined(BLE_PEER_MANAGER_FILTER_LOG)
  #include "app_log.h"
#endif // defined(SL_CATALOG_APP_LOG_PRESENT) && defined(BLE_PEER_MANAGER_FILTER_LOG)

// -----------------------------------------------------------------------------
// Definitions
#define AD_TYPE_INCOMPLETE_LIST_16_BIT_SERVICE_IDS  0x02
#define AD_TYPE_COMPLETE_LIST_16_BIT_SERVICE_IDS    0x03
#define AD_TYPE_INCOMPLETE_LIST_128_BIT_SERVICE_IDS 0x06
#define AD_TYPE_COMPLETE_LIST_128_BIT_SERVICE_IDS   0x07
#define AD_TYPE_SERVICE_DATA                        0x21
#define AD_TYPE_MANUFACTURER_DATA                   0xFF
#define AD_TYPE_SHORTENED_LOCAL_NAME                0x08
#define AD_TYPE_COMPLETE_LOCAL_NAME                 0x09
#define FILTER_SET                                  0x01
#define FILTER_CLEARED                              0x00
#define INVALID_ADDRESS_TYPE                        0xFE
#define MAX_NUMBER_OF_ADDRESS_FILTERS               10
#if defined(SL_CATALOG_APP_LOG_PRESENT) && BLE_PEER_MANAGER_FILTER_LOG
#define ble_peer_manager_filter_log_debug(...)           app_log_debug(__VA_ARGS__)
#define ble_peer_manager_filter_log_info(...)            app_log_info(__VA_ARGS__)
#define ble_peer_manager_filter_log_error(...)           app_log_error(__VA_ARGS__)
#define ble_peer_manager_filter_hexdump(p_data, len)     app_log_hexdump_debug(p_data, len)
#else
#define ble_peer_manager_filter_log_debug(...)
#define ble_peer_manager_filter_log_info(...)
#define ble_peer_manager_filter_log_error(...)
#define ble_peer_manager_filter_log_hexdump(p_data, len)
#endif

// Bitfield to store which filters are set
typedef union {
  struct {
    uint8_t rssi : 1;
    uint8_t device_name : 1;
    uint8_t device_name_full_match : 1;
    uint8_t address : 1;
    uint8_t address_type : 1;
    uint8_t service_data : 1;
    uint8_t service_uuid16 : 1;
    uint8_t service_uuid128 : 1;
    uint8_t manufacturer_data : 1;
  } flags;
  uint16_t total;
} filter_set_t;

// Filter datatype for storing filter parameters
typedef struct ble_peer_manager_filter_s {
  filter_set_t filter_set;
  int8_t rssi;
  char *device_name;
  uint8_t device_name_len;
  bd_addr address[MAX_NUMBER_OF_ADDRESS_FILTERS];
  sl_bt_gap_address_type_t address_type;
  sl_bt_uuid_16_t service_uuid16;
  uuid_128 service_uuid128;
  uint8_t *service_data;
  uint8_t service_data_offset;
  uint8_t service_data_len;
  uint8_t *manufacturer_data;
  uint8_t manufacturer_data_offset;
  uint8_t manufacturer_data_len;
} ble_peer_manager_filter_t;

// -----------------------------------------------------------------------------
// Static variables
static ble_peer_manager_filter_t active_filter;

// -----------------------------------------------------------------------------
// Public functions
void ble_peer_manager_filter_init(void)
{
  for (uint8_t i = 0; i < MAX_NUMBER_OF_ADDRESS_FILTERS; i++) {
    memset(&(active_filter.address[i].addr), 0xFF, sizeof(bd_addr));
  }
  (void)ble_peer_manager_reset_filter();
}

void ble_peer_manager_set_filter_bt_address(bool enabled)
{
  if (enabled) {
    active_filter.filter_set.flags.address = FILTER_SET;
    ble_peer_manager_filter_log_info("BT address filtering enabled" APP_LOG_NL);
  } else {
    active_filter.filter_set.flags.address = FILTER_CLEARED;
    ble_peer_manager_filter_log_info("BT address filtering disabled" APP_LOG_NL);
  }
}

sl_status_t ble_peer_manager_add_allowed_bt_address(bd_addr *filter_addr)
{
  if (!ble_peer_manager_is_filter_set_allowed()) {
    return SL_STATUS_INVALID_STATE;
  }
  if (filter_addr == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Find empty slot for the address filter
  bd_addr empty_addr = { { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } };
  for (uint8_t i = 0u; i < MAX_NUMBER_OF_ADDRESS_FILTERS; i++) {
    if (memcmp(&(active_filter.address[i].addr), empty_addr.addr, sizeof(bd_addr)) == 0) {
      // If an empty slot is found, add the filter
      memcpy(&(active_filter.address[i]), filter_addr, sizeof(bd_addr));
      ble_peer_manager_filter_log_info("BT address accept filter added for '%02x:%02x:%02x:%02x:%02x:%02x'" APP_LOG_NL,
                                       active_filter.address[i].addr[5],
                                       active_filter.address[i].addr[4],
                                       active_filter.address[i].addr[3],
                                       active_filter.address[i].addr[2],
                                       active_filter.address[i].addr[1],
                                       active_filter.address[i].addr[0]);
      return SL_STATUS_OK;
    }
  }
  ble_peer_manager_filter_log_error("Could not add BT address accept filter, maximum address filter number (%u) reached"
                                    APP_LOG_NL,
                                    MAX_NUMBER_OF_ADDRESS_FILTERS);
  return SL_STATUS_NO_MORE_RESOURCE;
}

sl_status_t ble_peer_manager_remove_allowed_bt_address(bd_addr *filter_addr)
{
  if (filter_addr == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Find the address filter and clear it
  for (uint8_t i = 0u; i < MAX_NUMBER_OF_ADDRESS_FILTERS; i++) {
    if (memcmp(&(active_filter.address[i].addr), filter_addr->addr, sizeof(bd_addr)) == 0) {
      // If the specified BT address is found, clear it
      ble_peer_manager_filter_log_info("BT address accept filter cleared for '%02x:%02x:%02x:%02x:%02x:%02x'" APP_LOG_NL,
                                       active_filter.address[i].addr[5],
                                       active_filter.address[i].addr[4],
                                       active_filter.address[i].addr[3],
                                       active_filter.address[i].addr[2],
                                       active_filter.address[i].addr[1],
                                       active_filter.address[i].addr[0]);
      memset(&(active_filter.address[i].addr), 0xFF, sizeof(bd_addr));
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t ble_peer_manager_set_filter_address_type(sl_bt_gap_address_type_t addr_type)
{
  if (!ble_peer_manager_is_filter_set_allowed()) {
    return SL_STATUS_INVALID_STATE;
  }
  active_filter.address_type = addr_type;
  active_filter.filter_set.flags.address_type = FILTER_SET;
  ble_peer_manager_filter_log_info("Filtering by address type, only accepting '%02x'" APP_LOG_NL,
                                   active_filter.address_type);
  return SL_STATUS_OK;
}

sl_status_t ble_peer_manager_set_filter_device_name(const char *device_name,
                                                    uint8_t device_name_len,
                                                    bool full_match)
{
  if (!ble_peer_manager_is_filter_set_allowed()) {
    return SL_STATUS_INVALID_STATE;
  }
  if (device_name == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  active_filter.device_name = (char *)sl_malloc(device_name_len);
  if (active_filter.device_name == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  memcpy(active_filter.device_name, device_name, device_name_len);
  active_filter.device_name_len = device_name_len;
  active_filter.filter_set.flags.device_name = FILTER_SET;
  if (full_match) {
    active_filter.filter_set.flags.device_name_full_match = FILTER_SET;
  }
  ble_peer_manager_filter_log_info("Filtering by device name, only accepting '%s'" APP_LOG_NL,
                                   active_filter.device_name);
  return SL_STATUS_OK;
}

sl_status_t ble_peer_manager_set_filter_service_uuid16(sl_bt_uuid_16_t *service_uuid16)
{
  if (!ble_peer_manager_is_filter_set_allowed()) {
    return SL_STATUS_INVALID_STATE;
  }
  if (service_uuid16 == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  memcpy(&(active_filter.service_uuid16), service_uuid16, sizeof(sl_bt_uuid_16_t));
  active_filter.filter_set.flags.service_uuid16 = FILTER_SET;
  ble_peer_manager_filter_log_info("Filtering by 16 bit UUID" APP_LOG_NL);
  return SL_STATUS_OK;
}

sl_status_t ble_peer_manager_set_filter_service_uuid128(uuid_128 *service_uuid128)
{
  if (!ble_peer_manager_is_filter_set_allowed()) {
    return SL_STATUS_INVALID_STATE;
  }
  if (service_uuid128 == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  memcpy(&(active_filter.service_uuid128), service_uuid128, sizeof(uuid_128));
  active_filter.filter_set.flags.service_uuid128 = FILTER_SET;
  ble_peer_manager_filter_log_info("Filtering by 128 bit UUID" APP_LOG_NL);
  return SL_STATUS_OK;
}

sl_status_t ble_peer_manager_set_filter_service_data(uint8_t *service_data,
                                                     uint8_t service_data_offset,
                                                     uint8_t service_data_len)
{
  if (!ble_peer_manager_is_filter_set_allowed()) {
    return SL_STATUS_INVALID_STATE;
  }
  if (service_data == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  active_filter.service_data = (uint8_t *)sl_malloc(service_data_len);
  if (active_filter.service_data == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  memcpy(active_filter.service_data, service_data, service_data_len);
  active_filter.service_data_offset = service_data_offset;
  active_filter.service_data_len = service_data_len;
  active_filter.filter_set.flags.service_data = FILTER_SET;
  ble_peer_manager_filter_log_info("Filtering by service data" APP_LOG_NL);
  return SL_STATUS_OK;
}

sl_status_t ble_peer_manager_set_filter_manufacturer_data(uint8_t *manufacturer_data,
                                                          uint8_t manufacturer_data_offset,
                                                          uint8_t manufacturer_data_len)
{
  if (!ble_peer_manager_is_filter_set_allowed()) {
    return SL_STATUS_INVALID_STATE;
  }
  if (manufacturer_data == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  active_filter.manufacturer_data = (uint8_t *)sl_malloc(manufacturer_data_len);
  if (active_filter.manufacturer_data == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  memcpy(active_filter.manufacturer_data, manufacturer_data, manufacturer_data_len);
  active_filter.manufacturer_data_offset = manufacturer_data_offset;
  active_filter.manufacturer_data_len = manufacturer_data_len;
  active_filter.filter_set.flags.manufacturer_data = FILTER_SET;
  ble_peer_manager_filter_log_info("Filtering by manufacturer data" APP_LOG_NL);
  return SL_STATUS_OK;
}

sl_status_t ble_peer_manager_set_filter_rssi(int8_t rssi)
{
  if (!ble_peer_manager_is_filter_set_allowed()) {
    return SL_STATUS_INVALID_STATE;
  }
  if (rssi > 20) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  active_filter.rssi = rssi;
  active_filter.filter_set.flags.rssi = FILTER_SET;
  ble_peer_manager_filter_log_info("Filtering by RSSI, only accepting from %d dBm" APP_LOG_NL,
                                   active_filter.rssi);
  return SL_STATUS_OK;
}

bool ble_peer_manager_is_filter_set(void)
{
  if (active_filter.filter_set.total > FILTER_CLEARED) {
    return true;
  } else {
    return false;
  }
}

sl_status_t ble_peer_manager_reset_filter(void)
{
  if (!ble_peer_manager_is_filter_set_allowed()) {
    return SL_STATUS_INVALID_STATE;
  }
  // Free pointers and set to 0
  if (active_filter.device_name != NULL) {
    sl_free(active_filter.device_name);
    active_filter.device_name = NULL;
  }
  if (active_filter.service_data != NULL) {
    sl_free(active_filter.service_data);
    active_filter.service_data = NULL;
  }
  if (active_filter.manufacturer_data != NULL) {
    sl_free(active_filter.manufacturer_data);
    active_filter.manufacturer_data = NULL;
  }

  // Set values of active_filter to 0
  active_filter.rssi = 0;
  active_filter.device_name_len = 0;
  active_filter.service_data_len = 0;
  active_filter.manufacturer_data_len = 0;
  active_filter.address_type = INVALID_ADDRESS_TYPE;
  active_filter.service_data_offset = 0;
  active_filter.manufacturer_data_offset = 0;

  // Set flags to 0
  active_filter.filter_set.total = FILTER_CLEARED;
  return SL_STATUS_OK;
}

// Find match for the filters
bool ble_peer_manager_find_match(bd_addr *address,
                                 sl_bt_gap_address_type_t address_type,
                                 int8_t rssi,
                                 const uint8array *adv_data)
{
  uint8_t advertisement_length;
  uint8_t advertisement_type;
  uint8_t i = 0;
  bool found_service_uuid16 = false;
  bool found_service_uuid128 = false;
  bool found_name = false;
  filter_set_t found;
  found.total = 0;

  if (active_filter.filter_set.flags.address == FILTER_SET) {
    found.flags.address = FILTER_SET;

    bool found = false;
    for (uint8_t i = 0u; i < MAX_NUMBER_OF_ADDRESS_FILTERS; i++) {
      if (memcmp(&(active_filter.address[i].addr), address->addr, sizeof(bd_addr)) == 0) {
        ble_peer_manager_filter_log_debug("Found matching BT address" APP_LOG_NL);
        found = true;
        break;
      }
    }
    if (!found) {
      return false;
    }
  }
  if (active_filter.filter_set.flags.address_type == FILTER_SET) {
    found.flags.address_type = FILTER_SET;
    if (active_filter.address_type != address_type) {
      return false;
    }
    ble_peer_manager_filter_log_debug("Found matching address type" APP_LOG_NL);
  }
  if (active_filter.filter_set.flags.rssi == FILTER_SET) {
    found.flags.rssi = FILTER_SET;
    if (active_filter.rssi >= rssi) {
      return false;
    }
    ble_peer_manager_filter_log_debug("Found matching RSSI" APP_LOG_NL);
  }

  while (i < (adv_data->len - 1)) {
    advertisement_length = adv_data->data[i];
    advertisement_type = adv_data->data[i + 1];
    if (active_filter.filter_set.flags.device_name == FILTER_SET) {
      if ((advertisement_type == AD_TYPE_SHORTENED_LOCAL_NAME) || (advertisement_type == AD_TYPE_COMPLETE_LOCAL_NAME)) {
        found.flags.device_name = FILTER_SET;
        if (active_filter.filter_set.flags.device_name_full_match) {
          found.flags.device_name_full_match = FILTER_SET;
        }
        if (advertisement_length - 1 >= active_filter.device_name_len
            && (!active_filter.filter_set.flags.device_name_full_match
                || advertisement_length - 1 == active_filter.device_name_len) ) {
          if (memcmp(&adv_data->data[i + 2], active_filter.device_name, active_filter.device_name_len) == 0) {
            ble_peer_manager_filter_log_debug("Found matching device name" APP_LOG_NL);
            found_name = true;
          }
        }
      }
    }
    if (active_filter.filter_set.flags.service_uuid16 == FILTER_SET) {
      if ((advertisement_type == AD_TYPE_INCOMPLETE_LIST_16_BIT_SERVICE_IDS)
          || (advertisement_type == AD_TYPE_COMPLETE_LIST_16_BIT_SERVICE_IDS)) {
        found.flags.service_uuid16 = FILTER_SET;
        for (uint8_t j = 0; j < (advertisement_length - 1); j = j + sizeof(sl_bt_uuid_16_t)) {
          if (memcmp(&(active_filter.service_uuid16),
                     &adv_data->data[i + 2 + j],
                     sizeof(active_filter.service_uuid16)) == 0) {
            ble_peer_manager_filter_log_debug("Found matching 16 bit UUID" APP_LOG_NL);
            found_service_uuid16 = true;
          }
        }
      }
    }
    if (active_filter.filter_set.flags.service_uuid128 == FILTER_SET) {
      if ((advertisement_type == AD_TYPE_INCOMPLETE_LIST_128_BIT_SERVICE_IDS)
          || (advertisement_type == AD_TYPE_COMPLETE_LIST_128_BIT_SERVICE_IDS)) {
        found.flags.service_uuid128 = FILTER_SET;
        for (uint8_t j = 0; j < (advertisement_length - 1); j = j + sizeof(uuid_128)) {
          if (memcmp(&(active_filter.service_uuid128),
                     &adv_data->data[i + 2 + j],
                     sizeof(active_filter.service_uuid128)) == 0) {
            ble_peer_manager_filter_log_debug("Found matching 128 bit UUID" APP_LOG_NL);
            found_service_uuid128 = true;
          }
        }
      }
    }
    if (active_filter.filter_set.flags.service_data == FILTER_SET) {
      if (advertisement_type == AD_TYPE_SERVICE_DATA) {
        found.flags.service_data = FILTER_SET;
        if (advertisement_length - 1
            < (active_filter.service_data_len + active_filter.service_data_offset)) {
          return false;
        }
        if (memcmp(&(active_filter.service_data),
                   &adv_data->data[i + 2 + active_filter.service_data_offset],
                   active_filter.service_data_len) == 0) {
          ble_peer_manager_filter_log_debug("Found matching service data" APP_LOG_NL);
        } else {
          return false;
        }
      }
    }
    if (active_filter.filter_set.flags.manufacturer_data == FILTER_SET) {
      if (advertisement_type == AD_TYPE_MANUFACTURER_DATA) {
        found.flags.manufacturer_data = FILTER_SET;
        if (advertisement_length - 1
            < (active_filter.manufacturer_data_len + active_filter.manufacturer_data_offset)) {
          return false;
        }
        if (memcmp(&(active_filter.manufacturer_data),
                   &adv_data->data[i + 2 + active_filter.manufacturer_data_offset],
                   active_filter.manufacturer_data_len) == 0) {
          ble_peer_manager_filter_log_debug("Found matching manufacturer data" APP_LOG_NL);
        } else {
          return false;
        }
      }
    }
    // Jump to next AD record
    i = i + advertisement_length + 1;
  }
  if ((active_filter.filter_set.flags.device_name == FILTER_SET
       || active_filter.filter_set.flags.device_name_full_match == FILTER_SET)
      && !found_name) {
    return false;
  }
  if (active_filter.filter_set.flags.service_uuid16 == FILTER_SET
      && !found_service_uuid16) {
    return false;
  }
  if (active_filter.filter_set.flags.service_uuid128 == FILTER_SET
      && !found_service_uuid128) {
    return false;
  }
  if (active_filter.filter_set.total != found.total) {
    return false;
  }

  ble_peer_manager_filter_log_info("Found matching device based on the configured filters" APP_LOG_NL);
  return true;
}

SL_WEAK bool ble_peer_manager_is_filter_set_allowed()
{
  return true;
}

sl_status_t ble_peer_manager_str_to_address(const char *str, bd_addr *address)
{
  if ((str == NULL) || (address == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }
  const char * const format[] = {
    "%2X%2X%2X%2X%2X%2X%n",
    "%2X:%2X:%2X:%2X:%2X:%2X%n"
  };
  // Due to the limitations of newlib, reading directly to the uint8_t array of
  // the address variable doesn't work. This intermediate buffer also helps to
  // leave the address value unchanged if the parsing fails.
  unsigned int read_buffer[sizeof(bd_addr)];
  for (size_t i = 0; i < sizeof(format) / sizeof(format[0]); i++) {
    int processed = 0;
    int items_read = sscanf(str,
                            format[i],
                            &read_buffer[5],
                            &read_buffer[4],
                            &read_buffer[3],
                            &read_buffer[2],
                            &read_buffer[1],
                            &read_buffer[0],
                            &processed);
    if (items_read == sizeof(bd_addr) && str[processed] == '\0') {
      for (int j = 0; j < items_read; j++) {
        address->addr[j] = (uint8_t)read_buffer[j];
      }
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_INVALID_PARAMETER;
}
