/***************************************************************************//**
 * @file
 * @brief Provides API to create and configure HCI event filter for filtering
 * Extended Advertisment Reports using RSSI and UUID.
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

#include <stdint.h>
#include <string.h>
#include "sl_btctrl_hci_event.h"
#include "sl_bt_hci_ext_adv_filter.h"
#include "sl_bt_hci_ext_adv_filter_config.h"
#include "sl_btctrl_hci_handler.h"

// -----------------------------------------------------------------------------

// Status codes defined in Bluetooth spec
#define BT_OK             0 // Command was succesful
#define BT_ERR_INVALID 0x12 // Used as a generic error

#define HCI_EVENT_LE_META_EVENT 0x3e
#define HCI_EVENT_LE_EXTENDED_ADVERTISING_REPORT 0x0d

#define SL_BT_HCI_FILTER_CONFIG_FULL 0x7f
#define SL_BT_HCI_FILTER_CONFIG_UUID_FULL (SL_BT_HCI_FILTER_CONFIG_FULL >> 1)

#define SL_BT_HCI_AD_TYPE_INCOMPLETE_SERVICE_CLASS_UUID_16_BIT 0x02
#define SL_BT_HCI_AD_TYPE_COMPLETE_SERVICE_CLASS_UUID_16_BIT 0x03

#define SL_BT_HCI_AD_TYPE_INCOMPLETE_SERVICE_CLASS_UUID_32_BIT 0x04
#define SL_BT_HCI_AD_TYPE_COMPLETE_SERVICE_CLASS_UUID_32_BIT 0x05

#define SL_BT_HCI_AD_TYPE_SERVICE_DATA_UUID_16_BIT 0x16
#define SL_BT_HCI_AD_TYPE_SERVICE_DATA_UUID_32_BIT 0x20

#define HCI_ADV_REPORT_MAX_DATA_LEN 229

// Structure replicating HCI Event packet with Extended Advertisment Report
SL_PACK_START(1)
typedef struct {
  uint8_t sub_event_code;
  uint8_t num_reports;
  uint16_t event_type;
  uint8_t address_type;
  uint8_t address[6];
  uint8_t primary_phy;
  uint8_t secondary_phy;
  uint8_t advertising_sid;
  int8_t tx_power;
  int8_t rssi;
  uint16_t periodic_advertising_interval;
  uint8_t directed_address_type;
  uint8_t directed_address[6];
  uint8_t data_length;
  uint8_t data[HCI_ADV_REPORT_MAX_DATA_LEN];
} SL_ATTRIBUTE_PACKED hci_le_extended_advertising_report_t;
SL_PACK_END()

// -----------------------------------------------------------------------------
// Forward declarations of private functions

/*******************************************************************************
 * HCI command (callback) for filter configuration
 * @param hcimsg HCI message containing filter configuration
 ******************************************************************************/
static bool hci_configure_filter_command(struct sl_btctrl_hci_message *hcimsg);

/*******************************************************************************
 * Callback function that performs RSSI filtering
 *
 * @param event is a pointer to sl_btctrl_hci_event structure containing the
 * HCI event
 * @returns Returns SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT if event's
 * RSSI value is equal or larger than confiured RSSI threshold value.
 ******************************************************************************/
static enum sl_btctrl_hci_event_filter_status filter_by_rssi(
  struct sl_btctrl_hci_event *event);

/*******************************************************************************
 * Filters Service Data UUIDs from AD structure
 *
 * @params uuid_size size of the UUIDs to filter
 * @params ad_data pointer to a AD structures's AD Data field
 * @params uuid_array_length lengtht of the UUID array
 * @params uuid_array pointer to UUID array to compare
 * @returns Returns true if matching Service Data UUID is found from the AD
 * structure
 *
 * Function compares the Service Data UUIDs provided in the UUID array by the
 * filter with the Service Data UUIDs in AD Structure.
 ******************************************************************************/
static bool filter_service_data_uuid(uint8_t uuid_size,
                                     uint8_t *ad_data,
                                     uint8_t uuid_array_length,
                                     uint8_t *uuid_array);

/*******************************************************************************
 * Filters List of Service (Class) UUIDs from AD structure
 *
 * @params uuid_size, size of the UUIDs to filter
 * @params ad_len length of the AD Structure
 * @params ad_data pointer to AD structure's data (AD Data field)
 * @params uuid_array_length lengtht of the UUID array
 * @params uuid_array pointer to UUID array to compare
 * @returns Returns true if matcing Service Class UUID is found in the Service
 * Class UUID List
 *
 * Function goes trough AD structure's Incomplete/Complete list of Service
 * Class UUIDs and compares them with UUIDs provided in the UUID array by the
 * filter.
 ******************************************************************************/
static bool filter_list_of_service_class_uuids(uint8_t uuid_size,
                                               uint8_t ad_len,
                                               uint8_t *ad_data,
                                               uint8_t uuid_array_length,
                                               uint8_t *uuid_array);

/*******************************************************************************
 * Filters 16-bit Service Data UUIDs from AD structure's data
 *
 * @params ad_data pointer to a AD structures's AD Data field
 * @returns Returns SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT if matching
 * 16-bit Service Data UUID is found from the AD structure's data field.
 ******************************************************************************/
static enum sl_btctrl_hci_event_filter_status filter_16bit_service(uint8_t *ad_data);

/*******************************************************************************
 * Filters 32-bit Service Data UUIDs from AD structure's data
 *
 * @params ad_data pointer to a AD structures's AD Data field
 * @returns Returns SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT if matching
 * 32-bit Service Data UUID is found from the AD structure's data field.
 ******************************************************************************/
static enum sl_btctrl_hci_event_filter_status filter_32bit_service(uint8_t *ad_data);

/*******************************************************************************
 * Filters List of 16-bit Service Class UUIDs from AD structure's data
 *
 * @params ad_data pointer to a AD structures's AD Data field
 * @returns Returns SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT if matching
 * 16-bit Service Class UUID is found from the list.
 ******************************************************************************/
static enum sl_btctrl_hci_event_filter_status filter_16bit_list(uint8_t ad_len,
                                                                uint8_t *ad_data);

/*******************************************************************************
 * Filters List of 32-bit Service Class UUIDs from AD structure's data
 *
 * @params ad_data pointer to a AD structures's AD Data field
 * @returns Returns SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT if matching
 * 32-bit Service Class UUID is found from the list.
 ******************************************************************************/
static enum sl_btctrl_hci_event_filter_status filter_32bit_list(uint8_t ad_len,
                                                                uint8_t *ad_data);

/*******************************************************************************
 * Goes trough a event's AD Structure and filters AD structure if no matching
 * UUIDs are found
 *
 * @params ad_len length of the AD Structure
 * @params ad_type the type of the AD structure
 * @params ad_data pointer to AD structure's data (AD Data field)
 * @returns Returns SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT if matching
 * UUID is found.
 ******************************************************************************/
static enum sl_btctrl_hci_event_filter_status filter_ad_structure(
  uint8_t ad_len,
  uint8_t ad_type,
  uint8_t *ad_data);

/*******************************************************************************
 * Callback function that performs UUID filtering
 *
 * @param event pointer to sl_btctrl_hci_event structure containing the HCI
 * event
 * @returns Returns SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT if matching
 * UUID is found from the event's data
 ******************************************************************************/
static enum sl_btctrl_hci_event_filter_status filter_by_uuid(
  struct sl_btctrl_hci_event *event);

// -----------------------------------------------------------------------------
// Private variables

// Handler used for registering hci_configure_filter_command function
static sl_btctrl_command_handler_t hci_command_handler;

// Event handler structure used for initialzing callback funtion for RSSI filtering
static sl_btctrl_hci_event_handler_t filter_by_rssi_handler;

// Event handler structure used for initialzing callback function for UUID filtering
static sl_btctrl_hci_event_handler_t filter_by_uuid_handler;

struct sli_btctrl_hci_event_uuid {
  uint8_t uuid_array_length;
  uint8_t *uuid_array;
};

struct sli_btctrl_hci_event_filter {
  uint32_t filter_config;
  int8_t rssi_threshold;
  struct sli_btctrl_hci_event_uuid uuids_16bit;
  struct sli_btctrl_hci_event_uuid uuids_32bit;
};

// Internal filter structure used for filtering
// This structure is modifed with sl_btctrl_hci_event_configure_filtering function
static struct sli_btctrl_hci_event_filter hci_event_filter;

#define WORK_MEMORY (SL_BT_HCI_FILTER_16BIT_UUID_ARRAY_LEN   \
                     + SL_BT_HCI_FILTER_32BIT_UUID_ARRAY_LEN \
                     + 2)

SL_PACK_START(1)
struct sli_work_filter {
  uint32_t config;
  int8_t rssi_threshold;
  uint8_t data[WORK_MEMORY];
} SL_ATTRIBUTE_PACKED work_filter;
SL_PACK_END()

// Variable to indicate if we have received an incomplete advertisment report
static bool advertisment_report_incomplete = false;

// -----------------------------------------------------------------------------
// Public functions

/*******************************************************************************
 * Initialize HCI event filtering HCI command
 ******************************************************************************/
void sli_bt_hci_filter_init(void)
{
  hci_event_filter.filter_config = SL_BT_HCI_FILTER_CONFIG;
  hci_event_filter.rssi_threshold = SL_BT_HCI_FILTER_RSSI_THRESHOLD;
  sl_btctrl_hci_register_handler(&hci_command_handler,
                                 &hci_configure_filter_command);
}

/*******************************************************************************
 * Configure HCI event filtering
 ******************************************************************************/
sl_status_t sl_btctrl_hci_event_configure_filtering(
  void *filter)
{
  struct sli_work_filter *user_filter = (struct sli_work_filter*)filter;
  if (filter == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (user_filter->config & ~SL_BT_HCI_FILTER_CONFIG_MASK) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (user_filter->config & SL_BT_HCI_FILTER_CONFIG_RSSI_ENABLE_MASK) {
    int rssi = user_filter->rssi_threshold;
    if (rssi < SL_BT_HCI_FILTER_RSSI_MIN || rssi > SL_BT_HCI_FILTER_RSSI_MAX) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }

  // copy the lenght of the 16-bit uuid array, which should be the first
  // byte of the uuid_data
  uint8_t uuid16_list_length = user_filter->data[0];
  if (uuid16_list_length > SL_BT_HCI_FILTER_16BIT_UUID_ARRAY_LEN) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // copy the lenght of the 32-bit uuid array, which should be the first
  // byte after the 16-bit uuid array of the uuid_data
  uint8_t uuid32_list_length = user_filter->data[uuid16_list_length + 1];
  if (uuid32_list_length > SL_BT_HCI_FILTER_32BIT_UUID_ARRAY_LEN) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  hci_event_filter.filter_config = user_filter->config;
  hci_event_filter.rssi_threshold = user_filter->rssi_threshold;
  hci_event_filter.uuids_16bit.uuid_array_length = uuid16_list_length;
  hci_event_filter.uuids_16bit.uuid_array = NULL;
  if (uuid16_list_length > 0) {
    hci_event_filter.uuids_16bit.uuid_array = &user_filter->data[1];
  }

  hci_event_filter.uuids_32bit.uuid_array_length = uuid32_list_length;
  hci_event_filter.uuids_32bit.uuid_array = NULL;
  if (uuid32_list_length > 0) {
    hci_event_filter.uuids_32bit.uuid_array = \
      &user_filter->data[uuid16_list_length + 2];
  }

  return SL_STATUS_OK;
}

/*******************************************************************************
 * Initialize HCI event filtering with RSSI and UUID filtering enabled
 ******************************************************************************/
sl_status_t sl_btctrl_rssi_uuid_filtering_init(void)
{
  sl_status_t status = SL_STATUS_OK;

  status = sl_btctrl_hci_register_event_handler(&filter_by_rssi_handler,
                                                filter_by_rssi);
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = sl_btctrl_hci_register_event_handler(&filter_by_uuid_handler,
                                                filter_by_uuid);
  if (status != SL_STATUS_OK) {
    return status;
  }

  return status;
}

// -----------------------------------------------------------------------------
// Private functions

static bool hci_configure_filter_command(struct sl_btctrl_hci_message *hcimsg)
{
  sl_status_t status = SL_STATUS_OK;
  uint16_t opcode;
  size_t length;

  status = sl_btctrl_hci_message_get_opcode(hcimsg, &opcode);
  if (status != SL_STATUS_OK) {
    return false;
  }

  status = sl_btctrl_hci_message_get_length(hcimsg, &length);
  if (opcode != SL_BT_HCI_CONFIGURE_FILTERING_OPCODE) {
    return false;
  }

  uint8_t min_length = sizeof(hci_event_filter.filter_config)
                       + sizeof(hci_event_filter.rssi_threshold);

  if (length < min_length) {
    status = SL_STATUS_INVALID_PARAMETER;
  }

  uint8_t max_length = sizeof(struct sli_work_filter);
  if (length > max_length) {
    status = SL_STATUS_INVALID_PARAMETER;
  }

  if (status == SL_STATUS_OK) {
    status = sl_btctrl_hci_message_get_parameters(hcimsg,
                                                  (uint8_t *)&work_filter,
                                                  length);
  }

  if (status == SL_STATUS_OK) {
    status = sl_btctrl_hci_event_configure_filtering(&work_filter);
  }

  sl_btctrl_hci_message_set_response(hcimsg,
                                     status == SL_STATUS_OK
                                     ? BT_OK : BT_ERR_INVALID,
                                     NULL,
                                     0);
  return true;
}

static enum sl_btctrl_hci_event_filter_status filter_by_rssi(
  struct sl_btctrl_hci_event *event)
{
  sl_status_t status;
  uint8_t opcode;
  uint8_t subevent_opcode;
  size_t len;
  size_t bytes_copied;

  if (!(hci_event_filter.filter_config & SL_BT_HCI_FILTER_CONFIG_RSSI_ENABLE_MASK)) {
    return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT;
  }

  status = sl_btctrl_hci_event_get_opcode(event, &opcode, &subevent_opcode);
  if (status != SL_STATUS_OK) {
    return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
  }

  if (opcode != HCI_EVENT_LE_META_EVENT
      || subevent_opcode != HCI_EVENT_LE_EXTENDED_ADVERTISING_REPORT) {
    return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT;
  }

  status = sl_btctrl_hci_event_get_length(event, &len);
  if (status != SL_STATUS_OK) {
    return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
  }

  int8_t rssi = 0;
  status = sl_btctrl_hci_event_get_parameters(event,
                                              (uint8_t*)&rssi,
                                              sizeof(int8_t),
                                              offsetof(hci_le_extended_advertising_report_t, rssi),
                                              &bytes_copied);

  if (status != SL_STATUS_OK) {
    return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
  }

  if (bytes_copied != sizeof(int8_t)) {
    return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
  }

  if (rssi < hci_event_filter.rssi_threshold) {
    return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
  }

  return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT;
}

static bool filter_service_data_uuid(uint8_t uuid_size,
                                     uint8_t *ad_data,
                                     uint8_t uuid_array_length,
                                     uint8_t *uuid_array)
{
  for (uint8_t i = 0; i < uuid_array_length; i += uuid_size) {
    if (memcmp(ad_data, &uuid_array[i], uuid_size) == 0) {
      return true;
    }
  }
  return false;
}

static bool filter_list_of_service_class_uuids(uint8_t uuid_size,
                                               uint8_t ad_len,
                                               uint8_t *ad_data,
                                               uint8_t uuid_array_length,
                                               uint8_t *uuid_array)
{
  for (uint8_t i = 0; i < (ad_len - 1); i += uuid_size) {
    for (uint8_t j = 0; j < uuid_array_length; j += uuid_size) {
      if (memcmp(&ad_data[i], &uuid_array[j], uuid_size) == 0) {
        return true;
      }
    }
  }

  return false;
}

static enum sl_btctrl_hci_event_filter_status filter_16bit_service(
  uint8_t *ad_data)
{
  uint8_t uuid_array_length = hci_event_filter.uuids_16bit.uuid_array_length;
  uint8_t *uuid_array = hci_event_filter.uuids_16bit.uuid_array;
  bool accepted = filter_service_data_uuid(sizeof(uint16_t),
                                           ad_data,
                                           uuid_array_length,
                                           uuid_array);
  return accepted ? SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT
         : SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
}

static enum sl_btctrl_hci_event_filter_status filter_32bit_service(
  uint8_t *ad_data)
{
  uint8_t uuid_array_length = hci_event_filter.uuids_32bit.uuid_array_length;
  uint8_t *uuid_array = hci_event_filter.uuids_32bit.uuid_array;
  bool accepted = filter_service_data_uuid(sizeof(uint32_t),
                                           ad_data,
                                           uuid_array_length,
                                           uuid_array);
  return accepted ? SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT
         : SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
}

static enum sl_btctrl_hci_event_filter_status filter_16bit_list(
  uint8_t ad_len,
  uint8_t *ad_data)
{
  uint8_t uuid_array_length = hci_event_filter.uuids_16bit.uuid_array_length;
  uint8_t *uuid_array = hci_event_filter.uuids_16bit.uuid_array;
  bool accepted = filter_list_of_service_class_uuids(sizeof(uint16_t),
                                                     ad_len,
                                                     ad_data,
                                                     uuid_array_length,
                                                     uuid_array);
  return accepted ? SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT
         : SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
}

/*******************************************************************************
 * Filters List of 32-bit Service Class UUIDs from AD structure's data
 *
 * @params ad_data pointer to a AD structures's AD Data field
 * @returns Returns SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT if matching
 * 32-bit Service Class UUID is found from the list.
 ******************************************************************************/
static enum sl_btctrl_hci_event_filter_status filter_32bit_list(
  uint8_t ad_len,
  uint8_t *ad_data)
{
  uint8_t uuid_array_length = hci_event_filter.uuids_32bit.uuid_array_length;
  uint8_t *uuid_array = hci_event_filter.uuids_32bit.uuid_array;
  bool accepted = filter_list_of_service_class_uuids(sizeof(uint32_t),
                                                     ad_len,
                                                     ad_data,
                                                     uuid_array_length,
                                                     uuid_array);
  return accepted ? SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT
         : SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
}

static enum sl_btctrl_hci_event_filter_status filter_ad_structure(uint8_t ad_len,
                                                                  uint8_t ad_type,
                                                                  uint8_t *ad_data)
{
  uint32_t config = hci_event_filter.filter_config;

  switch (ad_type) {
    case SL_BT_HCI_AD_TYPE_SERVICE_DATA_UUID_16_BIT:
      if (config & (SL_BT_HCI_FILTER_CONFIG_SERVICE_DATA_UUID_16_BIT_ENABLE_MASK)) {
        return filter_16bit_service(ad_data);
      }
      break;

    case SL_BT_HCI_AD_TYPE_SERVICE_DATA_UUID_32_BIT:
      if (config & (SL_BT_HCI_FILTER_CONFIG_SERVICE_DATA_UUID_32_BIT_ENABLE_MASK)) {
        return filter_32bit_service(ad_data);
      }
      break;

    case SL_BT_HCI_AD_TYPE_INCOMPLETE_SERVICE_CLASS_UUID_16_BIT:
      if (config & (SL_BT_HCI_FILTER_CONFIG_INCOMPLETE_UUID_16_BIT_ENABLE_MASK)) {
        return filter_16bit_list(ad_len, ad_data);
      }
      break;

    case SL_BT_HCI_AD_TYPE_COMPLETE_SERVICE_CLASS_UUID_16_BIT:
      if (config & (SL_BT_HCI_FILTER_CONFIG_COMPLETE_UUID_16_BIT_ENABLE_MASK)) {
        return filter_16bit_list(ad_len, ad_data);
      }
      break;

    case SL_BT_HCI_AD_TYPE_INCOMPLETE_SERVICE_CLASS_UUID_32_BIT:
      if (config & (SL_BT_HCI_FILTER_CONFIG_INCOMPLETE_UUID_32_BIT_ENABLE_MASK)) {
        return filter_32bit_list(ad_len, ad_data);
      }
      break;

    case SL_BT_HCI_AD_TYPE_COMPLETE_SERVICE_CLASS_UUID_32_BIT:
      if (config & (SL_BT_HCI_FILTER_CONFIG_COMPLETE_UUID_32_BIT_ENABLE_MASK)) {
        return filter_32bit_list(ad_len, ad_data);
      }
      break;

    default:
      break;
  }

  return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
}

static enum sl_btctrl_hci_event_filter_status filter_by_uuid(
  struct sl_btctrl_hci_event *event)
{
  sl_status_t status;
  uint8_t opcode;
  uint8_t subevent_opcode;
  size_t len;
  size_t bytes_copied;
  uint8_t buffer[sizeof(hci_le_extended_advertising_report_t)] = { 0 };
  hci_le_extended_advertising_report_t *ext_adv_report =
    (hci_le_extended_advertising_report_t *)buffer;

  uint32_t mask = (SL_BT_HCI_FILTER_CONFIG_MASK
                   & ~SL_BT_HCI_FILTER_CONFIG_RSSI_ENABLE_MASK);

  if (!(hci_event_filter.filter_config & mask)) {
    return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT;
  }

  status = sl_btctrl_hci_event_get_opcode(event, &opcode, &subevent_opcode);
  if (status != SL_STATUS_OK) {
    return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
  }

  if (opcode != HCI_EVENT_LE_META_EVENT
      || subevent_opcode != HCI_EVENT_LE_EXTENDED_ADVERTISING_REPORT) {
    return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT;
  }

  status = sl_btctrl_hci_event_get_length(event, &len);
  if (status != SL_STATUS_OK) {
    return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
  }

  status = sl_btctrl_hci_event_get_parameters(event,
                                              buffer,
                                              len,
                                              0,
                                              &bytes_copied);
  if (status != SL_STATUS_OK) {
    return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
  }

  if (bytes_copied != len) {
    return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
  }

  uint8_t data_len = 0;
  uint8_t *data;

  if (!advertisment_report_incomplete) {
    // if the report is incomplete and is the first report of the report chain,
    // send it to the host without filtering
    if (ext_adv_report->event_type & (1 << 5)) {
      advertisment_report_incomplete = true;
      return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT;
    }
  } else {
    // if the report is complete and is the last report of the report chain,
    // send it to the host without filtering
    if (!(ext_adv_report->event_type & (1 << 5))) {
      advertisment_report_incomplete = false;
      return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT;
    }
  }

  data_len = ext_adv_report->data_length;
  data = ext_adv_report->data;

  uint8_t current_idx = 0;
  while (data_len != 0) {
    uint8_t ad_len = data[current_idx];
    uint8_t ad_type = data[current_idx + 1];
    uint8_t *ad_data = &data[current_idx + 2];

    if (ad_len > data_len) {
      return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
    }

    enum sl_btctrl_hci_event_filter_status status;
    status = filter_ad_structure(ad_len, ad_type, ad_data);
    if (status == SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT) {
      return status;
    }

    data_len = data_len - (ad_len + 1);
    current_idx = current_idx + (ad_len + 1);
  }

  return SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD;
}
