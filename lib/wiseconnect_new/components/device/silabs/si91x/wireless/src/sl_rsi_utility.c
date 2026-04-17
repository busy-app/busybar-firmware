/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_si91x_protocol_types.h"
#include "sl_si91x_constants.h"
#include "sl_si91x_driver.h"
#include "sl_wifi_constants.h"
#include "sl_wifi_credentials.h"
#include "sl_status.h"
#include "sl_constants.h"
#include "sl_wifi_types.h"
#include "sl_rsi_utility.h"
#include "sli_wifi_constants.h"
#include "cmsis_os2.h" // CMSIS RTOS2
#include "sl_cmsis_utility.h"
#include "sl_si91x_types.h"
#include "sli_wifi_command_engine.h"
#include "sl_si91x_core_utilities.h"
#ifndef __ZEPHYR__
#include "sli_cmsis_os2_ext_task_register.h"
#endif
#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
#include "sl_si91x_socket_constants.h"
#include "sl_si91x_socket_utility.h"
#endif
#include "sl_core.h"
#include <string.h>
#include "assert.h"
#include "sli_wifi_utility.h"
#include "sli_wifi_power_profile.h"
#include "sli_wifi_memory_manager.h"
#include "sli_wifi.h"
#include "sl_string.h"

static bool sli_si91x_tx_command_status              = false;
static volatile bool power_save_sequence_in_progress = false;
extern bool global_queue_block;

/******************************************************
 *               Macro Declarations
 ******************************************************/
// Macro to check the status and return it if it's not SL_STATUS_OK
#define SLI_VERIFY_STATUS(s) \
  do {                       \
    if (s != SL_STATUS_OK)   \
      return s;              \
  } while (0)

// WLAN Management Frame Sub-Type
#define SLI_WIFI_FRAME_SUBTYPE_MASK       0xf0 // WLAN Management Frame Sub-Type Mask
#define SLI_WIFI_FRAME_SUBTYPE_PROBE_RESP 0x50 // WLAN Management Frame Sub-Type Probe Response Frame
#define SLI_WIFI_FRAME_SUBTYPE_BEACON     0x80 // WLAN Management Frame Sub-Type Beacon Frame
#define SLI_WIFI_MINIMUM_FRAME_LENGTH     36   // Minimum Frame Length of WLAN Management Frame
#define SLI_WIFI_HARDWARE_ADDRESS_LENGTH  6    // Hardware Address Length

// WLAN Information Element Type
#define SLI_WLAN_TAG_SSID            0   // WLAN Information Element Type SSID
#define SLI_WLAN_TAG_RSN             48  // WLAN Robust Security Network Information Element
#define SLI_WLAN_TAG_VENDOR_SPECIFIC 221 // WLAN Vendor Specific Information Element

// Authentication key Management Type
#define SLI_AUTH_KEY_MGMT_UNSPEC_802_1X   0x000FAC01 // Unspecified Authentication key Management Type
#define SLI_AUTH_KEY_MGMT_PSK_OVER_802_1X 0x000FAC02 // PSK Authentication key Management Type
#define SLI_AUTH_KEY_MGMT_802_1X_SHA256   0x000FAC05 // SHA256 Authentication key Management Type
#define SLI_AUTH_KEY_MGMT_PSK_SHA256      0x000FAC06 // PSK SHA256 Authentication key Management Type
#define SLI_AUTH_KEY_MGMT_SAE             0x000FAC08 // SAE Authentication key Management Type
#define SLI_AUTH_KEY_MGMT_FT_SAE          0x000FAC09 // FT_SAE Authentication key Management Type

// Authentication key Management Type Flags
#define SLI_WLAN_AUTH_KEY_MGMT_TYPE_WPA           0x00000001 // WPA AKM Type
#define SLI_WLAN_AUTH_KEY_MGMT_TYPE_WPA2          0x00000002 // WPA2 AKM Type
#define SLI_WLAN_AUTH_KEY_MGMT_TYPE_WPA_PSK       0x00000004 // WPA_PSK AKM Type
#define SLI_WLAN_AUTH_KEY_MGMT_TYPE_WPA2_PSK      0x00000008 // WPA2_PSK AKM Type
#define SLI_WLAN_AUTH_KEY_MGMT_TYPE_SAE           0x00010000 // SAE AKM Type
#define SLI_WLAN_AUTH_KEY_MGMT_TYPE_FT_SAE        0x00100000 // FT_SAE AKM Type
#define SLI_WLAN_AUTH_KEY_MGMT_TYPE_802_1X_SHA256 0x00020000 // SHA256 AKM Type
#define SLI_WLAN_AUTH_KEY_MGMT_TYPE_PSK_SHA256    0x00040000 // PSK_SHA256 AKM Type

/// Task register ID to save firmware status
#define SLI_FW_STATUS_STORAGE_INVALID_INDEX 0xFF // Invalid index for firmware status storage
#define DEFAULT_BEACON_MISS_IGNORE_LIMIT    1

/******************************************************
 *               Local Type Declarations
 ******************************************************/
// WLAN Frame
typedef struct {
  uint8_t fc[2];                                   // Frame Control
  uint8_t duration[2];                             // Duration
  uint8_t da[SLI_WIFI_HARDWARE_ADDRESS_LENGTH];    // Destination Address
  uint8_t sa[SLI_WIFI_HARDWARE_ADDRESS_LENGTH];    // Source Address
  uint8_t bssid[SLI_WIFI_HARDWARE_ADDRESS_LENGTH]; // BSS Id
  uint8_t sc[2];                                   // Sequence Control Id
  uint8_t timestamp[8];                            // Time Stamp
  uint8_t bi[2];                                   // Beacon Interval
  uint8_t ci[2];                                   // Capability Information
  uint8_t tagged_info[];                           // Variable Information Elememt
} sli_wifi_data_frame_t;

// WLAN Information Element
typedef struct {
  uint8_t tag;         // Information Element Tag Id
  uint8_t data_length; // Information Element Data Length
  uint8_t data[];      // Information Element Data
} sli_wifi_data_tagged_info_t;

// Cipher suite
typedef struct {
  uint8_t cs_oui[3]; // Cipher Suite OUI
  uint8_t cs_type;   // Cipher Suite Type
} sli_wlan_cipher_suite_t;

// WLAN Robust Security Network Information Element
typedef struct {
  uint8_t version[2];          // RSN Version
  sli_wlan_cipher_suite_t gcs; // Group cipher suite
  uint8_t pcsc[2];             // Pairwise cipher suite count
  uint8_t pcsl[];              // Pairwise cipher suite list
} sli_wlan_rsn_element_t;

// WLAN Vendor Specific Information Element
typedef struct {
  uint8_t oui[3];              // Vendor OUI
  uint8_t vs_oui;              // Vendor specific OUI
  uint8_t type;                // WPA Information Element
  uint8_t wpa_version[2];      // WPA Version
  sli_wlan_cipher_suite_t mcs; // Multicast Cipher Suite
  uint8_t ucsc;                // Unicast Cipher Suite List Count
  uint8_t ucsl[];              // Unicast Cipher Suite List
} sli_wlan_vendor_specific_element_t;

/******************************************************
 *               Variable Declarations
 ******************************************************/
osThreadId_t si91x_thread       = 0;
osThreadId_t si91x_event_thread = 0;
extern osEventFlagsId_t sli_wifi_events;
osEventFlagsId_t si91x_async_events = 0;
osMutexId_t malloc_free_mutex       = 0;

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
osMutexId_t side_band_crypto_mutex = 0;
#endif

sli_wifi_command_queue_t cmd_queues[SI91X_CMD_MAX];
sli_wifi_buffer_queue_t sli_tx_data_queue;
#ifndef __ZEPHYR__
// For all the threads this is the index of the thread local array at which the firmware status will be stored.
sli_task_register_id_t sli_fw_status_storage_index = SLI_FW_STATUS_STORAGE_INVALID_INDEX;
#endif

static bool sli_si91x_packet_status             = 0;
static uint8_t firmware_queue_id[SI91X_CMD_MAX] = { [SLI_WIFI_COMMON_CMD]   = SLI_WLAN_MGMT_Q,
                                                    [SLI_WIFI_WLAN_CMD]     = SLI_WLAN_MGMT_Q,
                                                    [SLI_SI91X_NETWORK_CMD] = SLI_WLAN_MGMT_Q,
                                                    [SLI_SI91X_SOCKET_CMD]  = SLI_WLAN_MGMT_Q,
                                                    [SLI_SI91X_BT_CMD]      = SLI_BT_Q };

extern bool device_initialized;

// Declaration of external functions
extern void sli_si91x_async_rx_event_handler_thread(void *args);
sl_status_t sl_si91x_host_power_cycle(void);
extern sl_status_t sli_create_generic_rx_packet_from_params(sli_si91x_queue_packet_t **queue_packet,
                                                            sl_wifi_buffer_t **packet_buffer,
                                                            uint16_t packet_id,
                                                            uint8_t flags,
                                                            void *sdk_context,
                                                            uint16_t frame_status);
void sl_debug_log(const char *format, ...);

extern sli_wifi_performance_profile_t performance_profile;

// NOTE: Boolean value determines whether firmware automatically closes the TCP socket in case of receiving termination from remote node or not.
static bool tcp_auto_close_enabled;

sl_wifi_system_performance_profile_t current_performance_profile = HIGH_PERFORMANCE;

static sl_wifi_system_boot_configuration_t saved_boot_configuration = { 0 };

/******************************************************
 *             Internal Function Declarations
 ******************************************************/

void *sli_wifi_host_get_buffer_data(void *buffer, uint16_t offset, uint16_t *data_length)
{
  if (NULL == buffer) {
    return NULL;
  }
  sl_wifi_buffer_t *temp_buffer = (sl_wifi_buffer_t *)buffer;
  if (offset >= temp_buffer->length) {
    return NULL;
  }
  if (data_length) {
    *data_length = (uint16_t)(temp_buffer->length) - offset;
  }
  return (void *)&temp_buffer->data[offset];
}

sl_status_t sli_wifi_send_command(uint32_t command,
                                  sli_wifi_command_type_t command_type,
                                  const void *data,
                                  uint32_t data_length,
                                  sli_wifi_wait_period_t wait_period,
                                  void *sdk_context,
                                  void **data_buffer)
{
  sl_wifi_buffer_t *buffer            = NULL;
  sl_wifi_buffer_t *response          = NULL;
  sl_wifi_system_packet_t *sys_packet = NULL;
  sl_status_t status;
  sli_si91x_queue_packet_t *node   = NULL;
  uint8_t flags                    = 0;
  static uint8_t command_packet_id = 0;
  sl_wifi_buffer_t *queue_packet;

  // Check if the queue type is within valid range
  if (command_type >= SI91X_CMD_MAX) {
    return SL_STATUS_INVALID_INDEX;
  }

  // Allocate a buffer for the command with appropriate size
  status = sli_wifi_allocate_command_buffer(&buffer,
                                            (void **)&sys_packet,
                                            sizeof(sl_wifi_system_packet_t) + data_length,
                                            SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
  VERIFY_STATUS_AND_RETURN(status);

  // Clear the packet descriptor and copy the command data if available
  memset(sys_packet->desc, 0, sizeof(sys_packet->desc));
  if (data != NULL) {
    memcpy(sys_packet->data, data, data_length);
  }

  // Set SLI_SI91X_FEAT_FW_UPDATE_NEW_CODE in the feature bit map to retrieve the latest firmware result codes
  if (command == SLI_WIFI_REQ_OPERMODE) {
    sl_wifi_system_boot_configuration_t *boot_configuration = (sl_wifi_system_boot_configuration_t *)sys_packet->data;
    boot_configuration->feature_bit_map |= SLI_SI91X_FEAT_FW_UPDATE_NEW_CODE;
  }

  // Fill frame type
  sys_packet->length  = data_length & 0xFFF;
  sys_packet->command = (uint16_t)command;

  // Allocate a command packet and set flags based on the command type
  status = sli_wifi_allocate_command_buffer(&queue_packet,
                                            (void **)&node,
                                            sizeof(sli_si91x_queue_packet_t),
                                            SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
  if (status != SL_STATUS_OK) {
    sli_wifi_memory_manager_free_buffer(buffer);
    return status;
  }

  // Set flags using helper function
  flags = sli_set_command_packet_flags(command, wait_period, data_buffer);

  // Configure command packet node using helper function
  sli_configure_command_packet_node(node, buffer, command_type, flags, sdk_context, wait_period);

  // Enqueue command packet using helper function
  const uint8_t this_packet_id = command_packet_id;
  command_packet_id++;
  status = sli_enqueue_command_packet(command_type, queue_packet, buffer, this_packet_id);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Handle command response using helper function
  return sli_handle_command_response(command_type, wait_period, this_packet_id, data_buffer, &response);
}

// Helper function implementations

uint8_t sli_set_command_packet_flags(uint32_t command, sli_wifi_wait_period_t wait_period, const void *data_buffer)
{
  uint8_t flags = 0;

#ifdef RSI_CHIP_MFG_EN
  // WLAN soc frequency
  if (command == RSI_BOOTUP_PARAMS) {
    host_desc[14] = RSI_SOC_FREQ;
  }
#endif

  // Check the wait_period to determine the flags for packet handling
  if (wait_period == SLI_WIFI_RETURN_IMMEDIATELY) {
    // If wait_period indicates an immediate return, set flags to 0
    flags = 0;
  } else {
    // If not an immediate return, set the SLI_WIFI_PACKET_RESPONSE_STATUS flag
    flags |= SLI_WIFI_PACKET_RESPONSE_STATUS;
    // Additionally, set the SLI_WIFI_PACKET_RESPONSE_PACKET flag if the SLI_WIFI_WAIT_FOR_RESPONSE_BIT is set in wait_period
    if (data_buffer != NULL) {
      flags |= ((wait_period & SLI_WIFI_WAIT_FOR_RESPONSE_BIT) ? SLI_WIFI_PACKET_RESPONSE_PACKET : 0);
    }
  }

  // Check the command type and set the flags accordingly
  switch (command) {
    case SLI_WIFI_RSP_PWRMODE:
    case SLI_WIFI_RSP_OPERMODE:
    case SLI_COMMON_REQ_SOFT_RESET:
      flags |= SLI_WIFI_PACKET_GLOBAL_QUEUE_BLOCK;
      break;
    default:
      break;
  }

  return flags;
}

void sli_configure_command_packet_node(sli_si91x_queue_packet_t *node,
                                       sl_wifi_buffer_t *buffer,
                                       sli_wifi_command_type_t command_type,
                                       uint8_t flags,
                                       void *sdk_context,
                                       sli_wifi_wait_period_t wait_period)
{
  // Set various properties of the node representing the command packet
  node->host_packet       = buffer;
  node->firmware_queue_id = firmware_queue_id[command_type];
  node->command_type      = command_type;
  node->flags             = flags;
  node->sdk_context       = sdk_context;

  if (flags != SLI_WIFI_PACKET_WITH_ASYNC_RESPONSE) {
    node->command_tickcount = osKernelGetTickCount();
    // Calculate the wait time based on wait_period
    if ((wait_period & SLI_WIFI_WAIT_FOR_EVER) == SLI_WIFI_WAIT_FOR_EVER) {
      node->command_timeout = osWaitForever;
    } else {
      node->command_timeout = (wait_period & ~SLI_WIFI_WAIT_FOR_RESPONSE_BIT);
    }
  }
}

sl_status_t sli_enqueue_command_packet(sli_wifi_command_type_t command_type,
                                       sl_wifi_buffer_t *queue_packet,
                                       sl_wifi_buffer_t *buffer,
                                       uint8_t packet_id)
{
  //! Enter Critical Section
  CORE_irqState_t state = CORE_EnterAtomic();

  if (cmd_queues[command_type].is_queue_initialized == false) {
    sli_wifi_memory_manager_free_buffer(buffer);
    sli_wifi_memory_manager_free_buffer(queue_packet);
    CORE_ExitAtomic(state);
    return SL_STATUS_NOT_INITIALIZED;
  }

  buffer->id              = packet_id;
  queue_packet->id        = packet_id;
  queue_packet->node.node = NULL;
  sli_wifi_append_to_buffer_queue(&cmd_queues[command_type].tx_queue, queue_packet);
  tx_command_queues_status |= SLI_WIFI_TX_PENDING_FLAG(command_type);
  sli_wifi_set_event(SLI_WIFI_TX_PENDING_FLAG(command_type));
  CORE_ExitAtomic(state);

  return SL_STATUS_OK;
}

sl_status_t sli_handle_command_response(sli_wifi_command_type_t command_type,
                                        sli_wifi_wait_period_t wait_period,
                                        uint8_t packet_id,
                                        void **data_buffer,
                                        sl_wifi_buffer_t **response)
{
  sl_status_t status;
  sli_wifi_wait_period_t wait_time = 0;
  uint16_t firmware_status;
  sli_si91x_queue_packet_t *node;
  uint16_t packet_data_length = 0;

  // Check if the command should return immediately or wait for a response
  if (wait_period == SLI_WIFI_RETURN_IMMEDIATELY) {
    return SL_STATUS_IN_PROGRESS;
  }

  // Calculate the wait time based on wait_period
  if ((wait_period & SLI_WIFI_WAIT_FOR_EVER) == SLI_WIFI_WAIT_FOR_EVER) {
    wait_time = osWaitForever;
  } else {
    wait_time = (wait_period & ~SLI_WIFI_WAIT_FOR_RESPONSE_BIT);
  }

  // Wait for a response packet and handle it
  status = sli_wifi_wait_for_response_packet(&cmd_queues[command_type].rx_queue,
                                             sli_wifi_events,
                                             SLI_WIFI_RESPONSE_FLAG(command_type),
                                             packet_id,
                                             wait_time,
                                             response);
  // Check if the status is SL_STATUS_TIMEOUT, indicating a timeout has occurred
  if (status == SL_STATUS_TIMEOUT) {
    // Declare a temporary packet pointer to hold the packet to be removed
    sl_wifi_buffer_t *temp_packet;
    sl_status_t temp_status = sli_wifi_remove_buffer_from_queue_by_comparator(&cmd_queues[command_type].tx_queue,
                                                                              &packet_id,
                                                                              sli_wifi_packet_identification_function,
                                                                              &temp_packet);

    // Check if the packet removal was successful
    if (temp_status == SL_STATUS_OK) {

      // Retrieve the actual packet node data from the removed buffer
      sli_si91x_queue_packet_t *temp_node = sli_wifi_host_get_buffer_data(temp_packet, 0, NULL);
      // Free the host packet memory associated with the node (TX packet memory)
      sli_wifi_memory_manager_free_buffer(temp_node->host_packet);

      // Free the temporary buffer memory that held the packet
      sli_wifi_memory_manager_free_buffer(temp_packet);
    }
  }
  VERIFY_STATUS_AND_RETURN(status);

  // Process the response packet and return the firmware status
  node            = (sli_si91x_queue_packet_t *)sli_wifi_host_get_buffer_data(*response, 0, &packet_data_length);
  firmware_status = node->frame_status;

  // If a data_buffer is provided, set it to the host_packet
  if (NULL != data_buffer) {
    *data_buffer = (void *)node->host_packet;
  }
  // If the response packet flag is set, free the host_packet buffer
  else if (SLI_WIFI_PACKET_RESPONSE_PACKET == (node->flags & SLI_WIFI_PACKET_RESPONSE_PACKET)) {
    sli_wifi_memory_manager_free_buffer(node->host_packet);
  }

  // Free the response buffer and return the firmware status
  sli_wifi_memory_manager_free_buffer(*response);
  return sli_wifi_convert_and_save_firmware_status(firmware_status);
}

// SI91X-specific helper function implementations

void sli_configure_si91x_command_packet_node(sli_si91x_queue_packet_t *node,
                                             sl_wifi_buffer_t *buffer,
                                             sli_wifi_command_type_t command_type,
                                             uint8_t flags,
                                             void *sdk_context,
                                             sli_wifi_wait_period_t wait_period)
{
  // Set various properties of the node representing the command packet
  node->host_packet       = buffer;
  node->firmware_queue_id = firmware_queue_id[command_type];
  node->command_type      = command_type;
  node->flags             = flags;
  node->sdk_context       = sdk_context;
  node->event_mask        = SL_SI91X_RESPONSE_FLAG(command_type);

  if (flags != SLI_WIFI_PACKET_WITH_ASYNC_RESPONSE) {
    node->command_tickcount = osKernelGetTickCount();
    // Calculate the wait time based on wait_period
    if ((wait_period & SLI_WIFI_WAIT_FOR_EVER) == SLI_WIFI_WAIT_FOR_EVER) {
      node->command_timeout = osWaitForever;
    } else {
      node->command_timeout = (wait_period & ~SLI_WIFI_WAIT_FOR_RESPONSE_BIT);
    }
  }
}

sl_status_t sli_enqueue_si91x_command_packet(sli_wifi_command_type_t command_type,
                                             sl_wifi_buffer_t *queue_packet,
                                             sl_wifi_buffer_t *buffer,
                                             uint8_t packet_id)
{
  //! Enter Critical Section
  CORE_irqState_t state = CORE_EnterAtomic();

  if (cmd_queues[command_type].is_queue_initialized == false) {
    sli_si91x_host_free_buffer(buffer);
    sli_si91x_host_free_buffer(queue_packet);
    CORE_ExitAtomic(state);
    return SL_STATUS_NOT_INITIALIZED;
  }

  buffer->id              = packet_id;
  queue_packet->id        = packet_id;
  queue_packet->node.node = NULL;
  sli_wifi_append_to_buffer_queue(&cmd_queues[command_type].tx_queue, queue_packet);
  tx_command_queues_status |= SL_SI91X_TX_PENDING_FLAG(command_type);
  sli_wifi_set_event(SL_SI91X_TX_PENDING_FLAG(command_type));
  CORE_ExitAtomic(state);

  return SL_STATUS_OK;
}

sl_status_t sli_fw_status_storage_index_init(void)
{
  sl_status_t status = SL_STATUS_OK;

  // Declare a variable to store the current interrupt state
  CORE_DECLARE_IRQ_STATE;

  // Enter a critical section by disabling interrupts
  // This ensures that the following operations are executed atomically
  CORE_ENTER_CRITICAL();
#ifdef SL_CATALOG_KERNEL_PRESENT
  // Check if the code is running in a thread context & task register index is invalid
  if (osThreadGetId() != NULL && sli_fw_status_storage_index == SLI_FW_STATUS_STORAGE_INVALID_INDEX) {
    // Create a new task register id
    status = sli_osTaskRegisterNew(&sli_fw_status_storage_index);
    VERIFY_STATUS_AND_RETURN(status);
  }
#endif
  CORE_EXIT_CRITICAL();
  return status;
}

// Function to update a existing entry or create new entry for scan results database
static sli_scan_info_t *sli_update_or_create_scan_info_element(const sli_scan_info_t *info)
{
  sli_scan_info_t **scan_db_head = sli_get_scan_info_database();
  sli_scan_info_t *element       = NULL;

  element = *scan_db_head;
  while (NULL != element) {
    if (0 == memcmp(info->bssid, element->bssid, SLI_WIFI_HARDWARE_ADDRESS_LENGTH)) {
      element->channel       = info->channel;
      element->security_mode = info->security_mode;
      element->rssi          = info->rssi;
      element->network_type  = info->network_type;
      memcpy(element->ssid, info->ssid, 34);
      break;
    }
    element = element->next;
  }

  if (NULL == element) {
    element = (sli_scan_info_t *)malloc(sizeof(sli_scan_info_t));
    if (element == NULL) {
      return NULL;
    }
    memcpy(element, info, sizeof(sli_scan_info_t));
    element->next = NULL;
    return element;
  }

  return NULL;
}

// Function to store a given scan info element in scan results database
static void sli_store_scan_info_element(const sli_scan_info_t *info)
{
  sli_scan_info_t *element       = NULL;
  sli_scan_info_t *head          = NULL;
  sli_scan_info_t *tail          = NULL;
  sli_scan_info_t **scan_db_head = sli_get_scan_info_database();

  if (NULL == info) {
    return;
  }

  element = sli_update_or_create_scan_info_element(info);
  if (NULL == element) {
    return;
  }

  if (NULL == *scan_db_head) {
    *scan_db_head = element;
    return;
  }

  tail = *scan_db_head;
  while (NULL != tail) {
    if (element->rssi < tail->rssi) {
      element->next = tail;
      if (NULL == head) {
        *scan_db_head = element;
      } else {
        head->next = element;
      }
      break;
    }

    head = tail;
    tail = tail->next;

    if (NULL == tail) {
      head->next = element;
    }
  }

  return;
}

// Function to identify Authentication Key Management Type
static uint32_t sli_get_key_management_info(const sli_wlan_cipher_suite_t *akms, uint16_t akmsc)
{
  uint32_t key_mgmt = 0;
  uint32_t oui_type;

  if (NULL == akms) {
    return 0;
  }

  for (int i = 0; i < akmsc; i++) {
    oui_type = ((akms[i].cs_oui[0] << 24) | (akms[i].cs_oui[1] << 16) | (akms[i].cs_oui[2] << 8) | akms[i].cs_type);

    switch (oui_type) {
      case SLI_AUTH_KEY_MGMT_UNSPEC_802_1X:
        key_mgmt |= SLI_WLAN_AUTH_KEY_MGMT_TYPE_WPA | SLI_WLAN_AUTH_KEY_MGMT_TYPE_WPA2;
        break;
      case SLI_AUTH_KEY_MGMT_PSK_OVER_802_1X:
        key_mgmt |= SLI_WLAN_AUTH_KEY_MGMT_TYPE_WPA_PSK | SLI_WLAN_AUTH_KEY_MGMT_TYPE_WPA2_PSK;
        break;
      case SLI_AUTH_KEY_MGMT_802_1X_SHA256:
        key_mgmt |= SLI_WLAN_AUTH_KEY_MGMT_TYPE_802_1X_SHA256;
        break;
      case SLI_AUTH_KEY_MGMT_PSK_SHA256:
        key_mgmt |= SLI_WLAN_AUTH_KEY_MGMT_TYPE_PSK_SHA256;
        break;
      case SLI_AUTH_KEY_MGMT_SAE:
        key_mgmt |= SLI_WLAN_AUTH_KEY_MGMT_TYPE_SAE;
        break;
      case SLI_AUTH_KEY_MGMT_FT_SAE:
        key_mgmt |= SLI_WLAN_AUTH_KEY_MGMT_TYPE_FT_SAE;
        break;
      default:
        break;
    }
  }
  return key_mgmt;
}

// Helper function to process RSN element
static void sli_process_rsn_element(const sli_wifi_data_tagged_info_t *info, sli_scan_info_t *scan_info)
{
  scan_info->security_mode            = SL_WIFI_WPA2_ENTERPRISE;
  sli_wlan_rsn_element_t *rsn         = (sli_wlan_rsn_element_t *)info->data;
  uint16_t pcsc                       = (uint16_t)(rsn->pcsc[0] | (rsn->pcsc[1] << 8));
  uint8_t *akmslc                     = (rsn->pcsl + (pcsc * sizeof(sli_wlan_cipher_suite_t)));
  uint16_t akmsc                      = (uint16_t)(akmslc[0] | (akmslc[1] << 8));
  const sli_wlan_cipher_suite_t *akms = (sli_wlan_cipher_suite_t *)(akmslc + 2);
  uint8_t wlan_gcs_oui[3]             = { 0x00, 0x0F, 0xAC };

  SL_DEBUG_LOG("RSN OUI %02x:%02x:%02x.\n", rsn->gcs.cs_oui[0], rsn->gcs.cs_oui[1], rsn->gcs.cs_oui[2]);
  SL_DEBUG_LOG("Pairwise cipher suite count: %u.\n", pcsc);

  if (!memcmp(rsn->gcs.cs_oui, wlan_gcs_oui, 3)) {
    scan_info->security_mode = SL_WIFI_WPA2;
    uint32_t key             = sli_get_key_management_info(akms, akmsc);

    if (akms[0].cs_type == 1) {
      scan_info->security_mode = SL_WIFI_WPA2_ENTERPRISE;
    }

    if (key & SLI_WLAN_AUTH_KEY_MGMT_TYPE_802_1X_SHA256) {
      scan_info->security_mode = SL_WIFI_WPA3_ENTERPRISE;
      if ((key & SLI_WLAN_AUTH_KEY_MGMT_TYPE_WPA) || (key & SLI_WLAN_AUTH_KEY_MGMT_TYPE_WPA2)) {
        scan_info->security_mode = SL_WIFI_WPA3_TRANSITION_ENTERPRISE;
      }
    }

    if (key & SLI_WLAN_AUTH_KEY_MGMT_TYPE_SAE) {
      scan_info->security_mode = SL_WIFI_WPA3;
      if ((key & SLI_WLAN_AUTH_KEY_MGMT_TYPE_PSK_SHA256) || (key & SLI_WLAN_AUTH_KEY_MGMT_TYPE_WPA2_PSK)) {
        scan_info->security_mode = SL_WIFI_WPA3_TRANSITION;
      }
    }
  }
}

// Helper function to process Vendor Specific element
static void sli_process_vendor_specific_element(const sli_wifi_data_tagged_info_t *info, sli_scan_info_t *scan_info)
{
  sli_wlan_vendor_specific_element_t *vendor = (sli_wlan_vendor_specific_element_t *)info->data;
  uint8_t wlan_oui[3]                        = { 0x00, 0x50, 0xF2 };

  if ((!memcmp(vendor->oui, wlan_oui, 3)) && (vendor->vs_oui == 0x01)
      && ((scan_info->security_mode == SL_WIFI_OPEN) || (scan_info->security_mode == SL_WIFI_WEP))) {
    scan_info->security_mode            = SL_WIFI_WPA;
    uint8_t *list_count                 = (vendor->ucsl + (sizeof(sli_wlan_cipher_suite_t) * vendor->ucsc));
    uint16_t akmsc                      = (uint16_t)(list_count[0] | (list_count[1] << 8));
    const sli_wlan_cipher_suite_t *akms = (sli_wlan_cipher_suite_t *)(list_count + 2);

    if ((0 != akmsc) && (akms[akmsc - 1].cs_type == 1)) {
      scan_info->security_mode = SL_WIFI_WPA_ENTERPRISE;
    }
  }
}

// Function to parse Information elements in WiFi Beacon or Probe response frames
static void sli_process_tag_info(const sli_wifi_data_tagged_info_t *info, sli_scan_info_t *scan_info)
{
  switch (info->tag) {
    case SLI_WLAN_TAG_SSID:
      memcpy(scan_info->ssid, info->data, info->data_length);
      scan_info->ssid[info->data_length] = 0;
      break;

    case SLI_WLAN_TAG_RSN:
      sli_process_rsn_element(info, scan_info);
      break;

    case SLI_WLAN_TAG_VENDOR_SPECIFIC:
      sli_process_vendor_specific_element(info, scan_info);
      break;

    default:
      break;
  }

  return;
}

/******************************************************
 *            Internal Function Declarations
 ******************************************************/
// Function to Parse the Beacon and Probe response Frames
void sli_handle_wifi_beacon(sl_wifi_system_packet_t *packet)
{
  uint8_t subtype                   = 0;
  sli_wifi_data_frame_t *wifi_frame = (sli_wifi_data_frame_t *)packet->data;
  sli_scan_info_t scan_info         = { 0 };
  uint16_t ies_length               = 0;

  scan_info.rssi    = (~packet->desc[10]);
  scan_info.channel = packet->desc[11];

  // Check for ESS bit and TBSS status bit in capability info
  // 1 in ESS bit indicates that the transmitter is an AP
  if (1 == (wifi_frame->ci[0] & 0x03)) {
    scan_info.network_type = 1;
  } else {
    scan_info.network_type = 0;
  }

  if (wifi_frame->ci[0] & 0x08) {
    scan_info.security_mode = SL_WIFI_WEP;
  } else {
    scan_info.security_mode = SL_WIFI_OPEN;
  }

  subtype = wifi_frame->fc[0] & SLI_WIFI_FRAME_SUBTYPE_MASK;
  switch (subtype) {
    case SLI_WIFI_FRAME_SUBTYPE_PROBE_RESP:
    case SLI_WIFI_FRAME_SUBTYPE_BEACON: {
      if (packet->length <= SLI_WIFI_MINIMUM_FRAME_LENGTH) {
        return;
      }
      ies_length = packet->length - SLI_WIFI_MINIMUM_FRAME_LENGTH;

      memcpy(scan_info.bssid, wifi_frame->bssid, SLI_WIFI_HARDWARE_ADDRESS_LENGTH);

      sli_wifi_data_tagged_info_t *info = (sli_wifi_data_tagged_info_t *)wifi_frame->tagged_info;
      while (0 != ies_length) {
        sli_process_tag_info(info, &scan_info);
        ies_length -= (sizeof(sli_wifi_data_tagged_info_t) + info->data_length);
        info = (sli_wifi_data_tagged_info_t *)&(info->data[info->data_length]);

        if (ies_length <= sizeof(sli_wifi_data_tagged_info_t)) {
          ies_length = 0;
        }
      }

      sli_store_scan_info_element(&scan_info);
    } break;
    default:
      return;
  }

  return;
}

/******************************************************
 *               Function Declarations
 ******************************************************/

void sli_reset_coex_current_performance_profile(void)
{
  if (!power_save_sequence_in_progress) {
    memset(&performance_profile, 0, sizeof(sli_wifi_performance_profile_t));
  }
}

void sli_save_boot_configuration(const sl_wifi_system_boot_configuration_t *boot_configuration)
{
  memcpy(&saved_boot_configuration, boot_configuration, sizeof(sl_wifi_system_boot_configuration_t));
}

void sli_get_saved_boot_configuration(sl_wifi_system_boot_configuration_t *boot_configuration)
{
  memcpy(boot_configuration, &saved_boot_configuration, sizeof(sl_wifi_system_boot_configuration_t));
}

void sli_get_bt_current_performance_profile(sl_bt_performance_profile_t *profile)
{
  SL_ASSERT(profile != NULL);
  memcpy(profile, &performance_profile.bt_performance_profile, sizeof(sl_bt_performance_profile_t));
}

void sli_save_tcp_auto_close_choice(bool is_tcp_auto_close_enabled)
{
  tcp_auto_close_enabled = is_tcp_auto_close_enabled;
}

bool sli_is_tcp_auto_close_enabled()
{
  return tcp_auto_close_enabled;
}

sl_status_t sli_convert_si91x_wifi_client_info(sl_wifi_client_info_response_t *client_info_response,
                                               const sli_wifi_client_info_response *sli_wifi_client_info_response)
{

  SL_WIFI_ARGS_CHECK_NULL_POINTER(sli_wifi_client_info_response);
  SL_WIFI_ARGS_CHECK_NULL_POINTER(client_info_response);

  client_info_response->client_count =
    (uint8_t)(sli_wifi_client_info_response->sta_count[0] | sli_wifi_client_info_response->sta_count[1] << 8);

  for (uint8_t station_index = 0; station_index < client_info_response->client_count; station_index++) {
    const uint8_t *si91x_ip_address;
    uint8_t *sl_ip_address;

    sl_wifi_client_info_t *sl_client_info            = &client_info_response->client_info[station_index];
    const sli_wifi_station_info_t *si91x_client_info = &sli_wifi_client_info_response->sta_info[station_index];

    uint8_t ip_address_size = (uint8_t)(si91x_client_info->ip_version[0] | si91x_client_info->ip_version[1] << 8);

    si91x_ip_address = ip_address_size == SL_IPV4_ADDRESS_LENGTH ? si91x_client_info->ip_address.ipv4_address
                                                                 : si91x_client_info->ip_address.ipv6_address;
    sl_ip_address    = ip_address_size == SL_IPV4_ADDRESS_LENGTH ? sl_client_info->ip_address.ip.v4.bytes
                                                                 : sl_client_info->ip_address.ip.v6.bytes;

    sl_client_info->ip_address.type = ip_address_size == SL_IPV4_ADDRESS_LENGTH ? SL_IPV4 : SL_IPV6;

    memcpy(&sl_client_info->mac_adddress, si91x_client_info->mac, sizeof(sl_mac_address_t));
    memcpy(sl_ip_address, si91x_ip_address, ip_address_size);
  }

  return SL_STATUS_OK;
}

sl_wifi_event_t sli_convert_si91x_event_to_sl_wifi_event(uint32_t command, uint16_t frame_status)
{
  // Define a constant indicating a fail indication event
  const sl_wifi_event_t fail_indication = (frame_status != RSI_SUCCESS) ? SL_WIFI_EVENT_FAIL_INDICATION : 0;

  // Switch-case to map SI91x events to SL Wi-Fi events
  switch (command) {
    case SLI_WIFI_RSP_BG_SCAN:
    case SLI_WIFI_RSP_SCAN:
      return SL_WIFI_SCAN_RESULT_EVENT | fail_indication;
    case SLI_WIFI_RSP_JOIN:
      return SL_WIFI_JOIN_EVENT | fail_indication;
    case SLI_WIFI_RSP_GET_STATS:
      if (frame_status != RSI_SUCCESS) {
        return SL_WIFI_STATS_RESPONSE_EVENTS | fail_indication;
      }
      return SL_WIFI_STATS_EVENT;
    case SLI_WIFI_RSP_RX_STATS:
      if (frame_status != RSI_SUCCESS) {
        return SL_WIFI_STATS_RESPONSE_EVENTS | fail_indication;
      }
      return SL_WIFI_STATS_ASYNC_EVENT;
    case SLI_WLAN_RATE_RSP_STATS:
      if (frame_status != RSI_SUCCESS) {
        return SL_WIFI_STATS_RESPONSE_EVENTS | fail_indication;
      }
      return SL_WIFI_STATS_TEST_MODE_EVENT;
    case SLI_WIFI_RSP_EXT_STATS:
      if (frame_status != RSI_SUCCESS) {
        return SL_WIFI_STATS_RESPONSE_EVENTS | fail_indication;
      }
      return SL_WIFI_STATS_ADVANCE_EVENT;
    case SLI_WLAN_RSP_MODULE_STATE:
      if (frame_status != RSI_SUCCESS) {
        return SL_WIFI_STATS_RESPONSE_EVENTS | fail_indication;
      }
      return SL_WIFI_STATS_MODULE_STATE_EVENT;
    case SLI_WLAN_RSP_HTTP_OTAF:
      return SL_WIFI_HTTP_OTA_FW_UPDATE_EVENT | fail_indication;
    case SLI_WLAN_RSP_CLIENT_CONNECTED:
      return SL_WIFI_CLIENT_CONNECTED_EVENT | fail_indication;
    case SLI_WLAN_RSP_CLIENT_DISCONNECTED:
      return SL_WIFI_CLIENT_DISCONNECTED_EVENT | fail_indication;
    case SLI_WLAN_RSP_TWT_ASYNC:
      if (frame_status == RSI_SUCCESS) {
        return SL_WIFI_TWT_RESPONSE_EVENT;
      } else {
        return SL_WIFI_TWT_RESPONSE_EVENT | (frame_status << 16);
      }
    case SLI_WLAN_RSP_TRANSCEIVER_TX_DATA_STATUS:
      return SL_WIFI_TRANSCEIVER_TX_DATA_STATUS_CB | fail_indication;
    case SLI_SI91X_WIFI_RX_DOT11_DATA:
      return SL_WIFI_TRANSCEIVER_RX_DATA_RECEIVE_CB | fail_indication;
    default:
      return SL_WIFI_INVALID_EVENT;
  }
}

sl_status_t sl_si91x_platform_init(void)
{
  sl_status_t status = SL_STATUS_OK;

  // Initialize the command queues
  memset(cmd_queues, 0, sizeof(cmd_queues));

  // Create event flags
  if (NULL == sli_wifi_events) {
    sli_wifi_events = osEventFlagsNew(NULL);
  }

  if (NULL == si91x_async_events) {
    si91x_async_events = osEventFlagsNew(NULL);
  }

  status = sli_command_engine_status_queue_init(); // Initialize resources for the command engine status queue
  VERIFY_STATUS_AND_RETURN(status);

  // Create and start Command Engine thread
  sli_wifi_command_engine_init();

  // Create and start SI91X event handler thread
  if (NULL == si91x_event_thread) {
    const osThreadAttr_t attr = {
      .name       = "si91x_async_rx_event",
      .priority   = SL_WLAN_EVENT_THREAD_PRIORITY,
      .stack_mem  = 0,
      .stack_size = SL_SI91X_EVENT_HANDLER_STACK_SIZE,
      .cb_mem     = 0,
      .cb_size    = 0,
      .attr_bits  = 0u,
      .tz_module  = 0u,
    };
    si91x_event_thread = osThreadNew(&sli_si91x_async_rx_event_handler_thread, NULL, &attr);
  }

  // Initialize command queues and associated mutexes
  for (int i = 0; i < SI91X_CMD_MAX; i++) {
    cmd_queues[i].tx_queue.head        = NULL;
    cmd_queues[i].tx_queue.tail        = NULL;
    cmd_queues[i].rx_queue.head        = NULL;
    cmd_queues[i].rx_queue.tail        = NULL;
    cmd_queues[i].event_queue.head     = NULL;
    cmd_queues[i].event_queue.tail     = NULL;
    cmd_queues[i].mutex                = osMutexNew(NULL);
    cmd_queues[i].flag                 = (1 << i);
    cmd_queues[i].is_queue_initialized = true;
  }

  // Create malloc/free mutex
  if (malloc_free_mutex == NULL) {
    malloc_free_mutex = osMutexNew(NULL);
  }

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  // Create side_band_crypto_mutex mutex
  side_band_crypto_mutex = osMutexNew(NULL);
#endif

  return status;
}

sl_status_t sli_si91x_platform_deinit(void)
{
  sl_status_t status = SL_STATUS_OK;
  // Deallocate all threads, mutexes and event handlers

  status =
    sli_command_engine_status_queue_deinit(); // Deinitialize resources associated with the command engine status queue.
  VERIFY_STATUS_AND_RETURN(status);

  // Terminate Command Engine thread
  sli_wifi_command_engine_deinit();

  // Terminate SI91X event handler thread
  if (NULL != si91x_event_thread) {
    osThreadTerminate(si91x_event_thread);
    si91x_event_thread = NULL;
  }

  // Delete event flags
  if (NULL != sli_wifi_events) {
    osEventFlagsDelete(sli_wifi_events);
    sli_wifi_events = NULL;
  }

  if (NULL != si91x_async_events) {
    osEventFlagsDelete(si91x_async_events);
    si91x_async_events = NULL;
  }

  // Delete command queue mutexes
  for (int i = 0; i < SI91X_CMD_MAX; i++) {
    osMutexDelete(cmd_queues[i].mutex);
    cmd_queues[i].mutex                = NULL;
    cmd_queues[i].is_queue_initialized = false;
  }

  // Delete malloc/free mutex
  osMutexDelete(malloc_free_mutex);
  malloc_free_mutex = NULL;
  return SL_STATUS_OK;
}

sl_si91x_host_timestamp_t sl_si91x_host_get_timestamp(void)
{
  return osKernelGetTickCount();
}

// Calculate elapsed time from the given starting timestamp
sl_si91x_host_timestamp_t sl_si91x_host_elapsed_time(uint32_t starting_timestamp)
{
  return (osKernelGetTickCount() - starting_timestamp);
}

// Delay execution for a specified number of milliseconds using an OS-level delay
void sl_si91x_host_delay_ms(uint32_t delay_milliseconds)
{
  if (delay_milliseconds == osWaitForever) {
    osDelay(osWaitForever);
  } else {
    osDelay(SLI_SYSTEM_MS_TO_TICKS(delay_milliseconds));
  }
}

sl_status_t sli_si91x_add_to_queue(sli_wifi_buffer_queue_t *queue, sl_wifi_buffer_t *buffer)
{
  sli_wifi_append_to_buffer_queue(queue, buffer);
  return SL_STATUS_OK;
}

sl_status_t sli_si91x_remove_from_queue(sli_wifi_buffer_queue_t *queue, sl_wifi_buffer_t **buffer)
{
  return sli_wifi_pop_from_buffer_queue(queue, buffer);
}

sl_status_t sli_si91x_flush_all_tx_wifi_queues(uint16_t frame_status)
{
  // free all TX queues except BT
  for (int queue_id = 0; queue_id < SI91X_CMD_MAX; queue_id++) {
    if (queue_id == SLI_SI91X_BT_CMD) {
      continue;
    }
    sli_si91x_flush_queue_based_on_type(&cmd_queues[queue_id],
                                        SL_SI91X_RESPONSE_FLAG(queue_id),
                                        frame_status,
                                        NULL,
                                        NULL);
  }
  return SL_STATUS_OK;
}

sl_status_t sli_si91x_flush_generic_data_queues(sli_wifi_buffer_queue_t *tx_data_queue)
{
  sl_wifi_buffer_t *current_packet = NULL;
  sl_wifi_buffer_t *next_packet    = NULL;

  // Validate input
  if (tx_data_queue == NULL) {
    return SL_STATUS_FAIL;
  }

  // Prevent race conditions
  CORE_irqState_t state = CORE_EnterAtomic();

  // Free all packets in the queue
  current_packet = tx_data_queue->head;
  while (current_packet != NULL) {
    next_packet = (sl_wifi_buffer_t *)current_packet->node.node;
    sli_si91x_host_free_buffer(current_packet);
    current_packet = next_packet;
  }

  // Reset the queue
  tx_data_queue->head = NULL;
  tx_data_queue->tail = NULL;

  // Clear pending TX status
  tx_generic_socket_data_queues_status &= ~(SL_SI91X_GENERIC_DATA_TX_PENDING_EVENT);

  CORE_ExitAtomic(state);

  return SL_STATUS_OK;
}

void sli_reset_command_queue_trace(sli_wifi_command_queue_t *queue)
{
  // Reset command trace for the queue
  queue->command_in_flight = false;
  queue->frame_type        = 0;
  queue->flag              = 0;
  queue->command_tickcount = 0;
  queue->command_timeout   = 0;
  queue->event_mask        = 0;
}

void sli_flush_tx_packet(sli_wifi_command_queue_t *queue,
                         sl_wifi_buffer_t *current_packet,
                         sli_si91x_queue_packet_t *queue_node,
                         uint16_t frame_status,
                         uint32_t event_mask)
{
  // Check if the packet in the TX queue is synchronous or asynchronous
  if (queue_node->flags & SLI_WIFI_PACKET_RESPONSE_STATUS) {
    // Update the frame_status and other details
    queue_node->frame_status  = frame_status;
    current_packet->node.node = NULL;

    // Check if the TX packet is not expecting a response, then free the host packet
    if ((!(queue_node->flags & SLI_WIFI_PACKET_RESPONSE_PACKET)) && (queue_node->host_packet != NULL)) {
      sli_si91x_host_free_buffer(queue_node->host_packet);
      queue_node->host_packet = NULL;
    }

    // Check if the packet in the queue is synchronous or asynchronous response
    if (sl_si91x_host_elapsed_time(queue_node->command_tickcount) <= (queue_node->command_timeout)) {
      sli_si91x_add_to_queue(&queue->rx_queue, current_packet);
      sli_wifi_set_event(event_mask);
    } else {
      // no user thread is waiting for the response so flush the packet
      if (queue_node->flags & SLI_WIFI_PACKET_RESPONSE_PACKET) {
        sli_si91x_host_free_buffer(queue_node->host_packet);
      }
      sli_si91x_host_free_buffer(current_packet);
    }
  } else {
    // Handle asynchronous packets by freeing them
    sli_si91x_host_free_buffer(queue_node->host_packet);
    sli_si91x_host_free_buffer(current_packet);
  }
}

sl_status_t sli_flush_tx_queue(sli_wifi_command_queue_t *queue,
                               uint32_t event_mask,
                               uint16_t frame_status,
                               sli_si91x_compare_function_t compare_function,
                               void *user_data)
{
  // Start with the head of the TX queue
  sl_wifi_buffer_t *current_packet  = queue->tx_queue.head;
  sl_wifi_buffer_t *previous_packet = NULL;
  sl_wifi_buffer_t *next_packet     = NULL;

  // Iterate through all packets in the TX queue
  while (current_packet != NULL) {
    // Get the associated queue node
    sli_si91x_queue_packet_t *queue_node = sli_wifi_host_get_buffer_data(current_packet, 0, NULL);
    if (queue_node == NULL) {
      return SL_STATUS_NOT_AVAILABLE;
    }

    if (!compare_function || compare_function(queue_node->host_packet, user_data) == false) {
      previous_packet = current_packet;
      current_packet  = (sl_wifi_buffer_t *)current_packet->node.node;
      continue;
    }

    // Save the next packet in the queue
    next_packet = (sl_wifi_buffer_t *)current_packet->node.node;

    // Flush the current packet
    sli_flush_tx_packet(queue, current_packet, queue_node, frame_status, event_mask);

    // Update the head of the queue if the current packet is the head
    if (current_packet == queue->tx_queue.head) {
      queue->tx_queue.head = next_packet;
    }

    // Update the tail of the queue if the current packet is the tail
    if (current_packet == queue->tx_queue.tail) {
      queue->tx_queue.tail = previous_packet;
    }

    // Move to the next packet in the queue
    previous_packet = current_packet;
    current_packet  = next_packet;
  }

  // Check if the queue is empty and update tx_command_queues_status
  if (queue->tx_queue.head == NULL) {
    // Clear the tail pointer of the TX queue
    queue->tx_queue.tail = NULL;
    tx_command_queues_status &= ~event_mask;
  }

  return SL_STATUS_OK;
}

void sli_flush_command_in_flight_packet(sli_wifi_command_queue_t *queue,
                                        sl_wifi_buffer_t *current_packet,
                                        sl_wifi_buffer_t *dummy_packet_buffer,
                                        uint32_t event_mask)
{
  if (!(queue->flags & SLI_WIFI_PACKET_RESPONSE_PACKET)) {
    // Flag is response status, free the dummy packet buffer if it is not expecting a response buffer
    sli_si91x_host_free_buffer(dummy_packet_buffer);
  }

  // Check if the packet in the queue is synchronous or asynchronous response
  if (sl_si91x_host_elapsed_time(queue->command_tickcount) <= (queue->command_timeout)) {
    // Add the packet to the response queue and set the event
    sli_si91x_add_to_queue(&queue->rx_queue, current_packet);
    sli_wifi_set_event(event_mask);
  } else {
    // No user thread is waiting for the response so flush the packet
    sli_si91x_host_free_buffer(current_packet);
    // Free the host packet if timeout occurs for response packet
    if (queue->flags & SLI_WIFI_PACKET_RESPONSE_PACKET) {
      sli_si91x_host_free_buffer(dummy_packet_buffer);
    }
  }

  tx_command_queues_command_in_flight_status &= ~event_mask;
  sli_reset_command_queue_trace(queue);
}

sl_status_t sli_handle_command_in_flight_packet(sli_wifi_command_queue_t *queue,
                                                uint32_t event_mask,
                                                uint16_t frame_status,
                                                sli_si91x_compare_function_t compare_function,
                                                void *user_data)
{
  sl_wifi_buffer_t *current_packet     = NULL;
  sli_si91x_queue_packet_t *queue_node = NULL;
  uint16_t frame_type                  = 0;
  // Create a generic RX packet
  sl_status_t status = sli_create_generic_rx_packet_from_params(&queue_node,
                                                                &current_packet,
                                                                queue->packet_id,
                                                                queue->flags,
                                                                queue->sdk_context,
                                                                frame_status);
  if (status != SL_STATUS_OK)
    return status;

  sl_wifi_buffer_t *dummy_packet_buffer = NULL;
  // Allocate buffer for the dummy packet
  status = sli_si91x_host_allocate_buffer(&dummy_packet_buffer,
                                          SL_WIFI_RX_FRAME_BUFFER,
                                          sizeof(sl_wifi_system_packet_t),
                                          SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
  if (status != SL_STATUS_OK) {
    sli_si91x_host_free_buffer(current_packet);
    return status;
  }

  // Get the dummy packet data from the allocated buffer
  sl_wifi_system_packet_t *dummy_packet = sli_wifi_host_get_buffer_data(dummy_packet_buffer, 0, NULL);
  queue_node->host_packet               = dummy_packet_buffer;
  dummy_packet->desc[2]                 = (uint8_t)queue->frame_type;
  dummy_packet->desc[3]                 = (uint8_t)((0xFF00 & queue->frame_type) >> 8);

  frame_type = queue->frame_type;

  if (!compare_function || compare_function(queue_node->host_packet, user_data)) {
    sli_flush_command_in_flight_packet(queue, current_packet, dummy_packet_buffer, event_mask);
  } else {
    // If compare function returns false, free the allocated buffers
    sli_si91x_host_free_buffer(current_packet);
    sli_si91x_host_free_buffer(dummy_packet_buffer);
  }

  switch (frame_type) {
    case SLI_COMMON_RSP_OPERMODE:
    case SLI_COMMON_RSP_SOFT_RESET:
    case SLI_COMMON_RSP_PWRMODE: {
      global_queue_block = false;
      break;
    }
    default:
      break;
  }

  return SL_STATUS_OK;
}

sl_status_t sli_si91x_flush_queue_based_on_type(sli_wifi_command_queue_t *queue,
                                                uint32_t event_mask,
                                                uint16_t frame_status,
                                                sli_si91x_compare_function_t compare_function,
                                                void *user_data)
{
  sl_status_t status = SL_STATUS_FAIL;
  // Enter atomic section to prevent race conditions
  CORE_irqState_t state = CORE_EnterAtomic();

  // Check if the queue is not the BT command queue and has a command in flight
  if (queue != &cmd_queues[SLI_SI91X_BT_CMD] && queue->command_in_flight) {
    status = sli_handle_command_in_flight_packet(queue, event_mask, frame_status, compare_function, user_data);
    if (status != SL_STATUS_OK) {
      CORE_ExitAtomic(state);
      return status;
    }
  }

  status = sli_flush_tx_queue(queue, event_mask, frame_status, compare_function, user_data);

  CORE_ExitAtomic(state);
  return status;
}

#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
sl_status_t sli_si91x_flush_all_socket_command_queues(uint16_t frame_status, uint8_t vap_id)
{
  sl_status_t status;

  // Loop through all sockets
  for (int index = 0; index < SLI_NUMBER_OF_SOCKETS; index++) {
    // Check if the socket exists and matches the required VAP ID
    if ((sli_si91x_sockets[index] != NULL) && (sli_si91x_sockets[index]->vap_id == vap_id)) {
      // Flush the command queues for the current socket based on queue type
      status = sli_si91x_flush_socket_command_queues_based_on_queue_type((uint8_t)index, frame_status);
      // If flushing fails, return the error status immediately
      if (status != SL_STATUS_OK) {
        return status;
      }
    }
  }
  // Return SL_STATUS_OK if all sockets were processed successfully
  return SL_STATUS_OK;
}

void sli_process_socket_close(sli_si91x_queue_packet_t *queue_node, const sli_si91x_socket_t *socket)
{
  sli_si91x_host_free_buffer(queue_node->host_packet);
  sl_wifi_buffer_t *host_packet = NULL;
  // Allocate a buffer for the host packet
  sl_status_t status =
    sli_si91x_host_allocate_buffer(&host_packet,
                                   SL_WIFI_RX_FRAME_BUFFER,
                                   sizeof(sl_wifi_system_packet_t) + sizeof(sl_si91x_socket_close_response_t),
                                   SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
  // If buffer allocation fails, log the error and return failure status
  if (status != SL_STATUS_OK) {
    SL_DEBUG_LOG("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
    return;
  }
  queue_node->host_packet  = host_packet;
  queue_node->frame_status = SL_STATUS_OK;
  sl_wifi_system_packet_t *si91x_packet =
    (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(queue_node->host_packet, 0, NULL);
  si91x_packet->desc[12]                                  = 0;
  si91x_packet->desc[13]                                  = 0;
  sl_si91x_socket_close_response_t *socket_close_response = (sl_si91x_socket_close_response_t *)si91x_packet->data;
  socket_close_response->socket_id                        = (uint16_t)socket->id;
  socket_close_response->port_number                      = socket->local_address.sin6_port;

  return;
}

void sli_process_socket_read_data(sli_si91x_queue_packet_t *queue_node,
                                  uint16_t frame_status,
                                  sli_si91x_socket_t *socket)
{
  sl_wifi_system_packet_t *si91x_packet =
    (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(queue_node->host_packet, 0, NULL);
  if (si91x_packet != NULL) {
    si91x_packet->desc[12] = (uint8_t)(frame_status & 0x00FF);
    si91x_packet->desc[13] = (uint8_t)((frame_status & 0xFF00) >> 8);
    sli_si91x_add_to_queue(&socket->rx_data_queue, queue_node->host_packet);
  }

  return;
}

void sli_handle_frame_type(sli_si91x_queue_packet_t *queue_node,
                           int32_t frame_type,
                           uint16_t frame_status,
                           sli_si91x_socket_t *socket,
                           sl_wifi_buffer_t *current_packet)
{
  switch (frame_type) {
    case SLI_WLAN_RSP_SOCKET_READ_DATA:
      sli_process_socket_read_data(queue_node, frame_status, socket);
      break;
    case SLI_WLAN_RSP_SOCKET_CLOSE:
      sli_process_socket_close(queue_node, socket);
      __attribute__((fallthrough));
    default:
      sli_si91x_add_to_queue(&socket->command_queue.rx_queue, current_packet);
      break;
  }

  return;
}

void sli_flush_packet(sli_si91x_queue_packet_t *queue_node, sl_wifi_buffer_t *current_packet)
{
  // no user thread is waiting for the response so flush the packet
  if ((queue_node->flags & SLI_WIFI_PACKET_RESPONSE_PACKET) == SLI_WIFI_PACKET_RESPONSE_PACKET) {
    sli_si91x_host_free_buffer(queue_node->host_packet);
  }
  sli_si91x_host_free_buffer(current_packet);

  return;
}

void sli_process_frame_type(sli_si91x_queue_packet_t *queue_node,
                            int32_t frame_type,
                            uint16_t frame_status,
                            sli_si91x_socket_t *socket,
                            sl_wifi_buffer_t *current_packet)
{
  // Add to response queue and raise event based on frame type
  if (sl_si91x_host_elapsed_time(queue_node->command_tickcount) <= queue_node->command_timeout) {
    sli_handle_frame_type(queue_node, frame_type, frame_status, socket, current_packet);
    sli_si91x_set_socket_event((1 << socket->index));
  } else {
    sli_flush_packet(queue_node, current_packet);
  }

  return;
}

sl_status_t sli_si91x_flush_socket_command_queues_based_on_queue_type(uint8_t index, uint16_t frame_status)
{
  sl_wifi_buffer_t *current_packet     = NULL;
  sl_wifi_buffer_t *next_packet        = NULL;
  sli_si91x_queue_packet_t *queue_node = NULL;

  // Retrieve the socket using the provided index
  sli_si91x_socket_t *socket = sli_get_si91x_socket(index);

  if (socket == NULL) {
    return SL_STATUS_FAIL;
  }

  // Enter atomic state to prevent race conditions
  CORE_irqState_t state = CORE_EnterAtomic();

  // Check if there is a command currently in flight
  if ((socket->command_queue.command_in_flight) && ((socket->command_queue.flags) & SLI_WIFI_PACKET_RESPONSE_STATUS)) {
    // Create a generic RX packet from the current command parameters
    sl_status_t status = sli_create_generic_rx_packet_from_params(&queue_node,
                                                                  &current_packet,
                                                                  socket->command_queue.packet_id,
                                                                  socket->command_queue.flags,
                                                                  socket->command_queue.sdk_context,
                                                                  frame_status);
    // If packet creation fails, exit atomic state and return the error status
    if (status != SL_STATUS_OK) {
      CORE_ExitAtomic(state);
      return status;
    }

    // Handle packets that expect a response
    if (queue_node->flags & SLI_WIFI_PACKET_RESPONSE_PACKET) {
      sl_wifi_buffer_t *host_packet = NULL;
      uint16_t length               = 0;
      if (socket->command_queue.frame_type == SLI_WLAN_RSP_SOCKET_CLOSE) {
        length = sizeof(sl_wifi_system_packet_t) + sizeof(sl_si91x_socket_close_response_t);
      } else {
        length = sizeof(sl_wifi_system_packet_t);
      }
      // Allocate a buffer for the host packet
      status = sli_si91x_host_allocate_buffer(&host_packet,
                                              SL_WIFI_RX_FRAME_BUFFER,
                                              length,
                                              SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
      // If buffer allocation fails, log the error, release mutex lock, notify the command engine and return failure status
      if (status != SL_STATUS_OK) {
        SL_DEBUG_LOG("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
        CORE_ExitAtomic(state);
        return SL_STATUS_ALLOCATION_FAILED;
      }

      // Populate the packet descriptor with frame status
      sl_wifi_system_packet_t *si91x_packet =
        (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(host_packet, 0, NULL);
      si91x_packet->desc[12]  = (uint8_t)(frame_status & 0x00FF);
      si91x_packet->desc[13]  = (uint8_t)((frame_status & 0xFF00) >> 8);
      si91x_packet->desc[2]   = (uint8_t)(socket->command_queue.frame_type & 0x00FF);
      si91x_packet->desc[3]   = (uint8_t)((socket->command_queue.frame_type & 0xFF00) >> 8);
      queue_node->host_packet = host_packet;
      host_packet->id         = current_packet->id;
      if (socket->command_queue.frame_type == SLI_WLAN_RSP_SOCKET_CLOSE) {
        si91x_packet->desc[12]   = 0;
        si91x_packet->desc[13]   = 0;
        queue_node->frame_status = SL_STATUS_OK;
        sl_si91x_socket_close_response_t *socket_close_response =
          (sl_si91x_socket_close_response_t *)si91x_packet->data;
        socket_close_response->socket_id   = (uint16_t)socket->id;
        socket_close_response->port_number = socket->local_address.sin6_port;
      }
    }

    // Add the packet to the appropriate queue and set the event
    if (sl_si91x_host_elapsed_time(socket->command_queue.command_tickcount) <= socket->command_queue.command_timeout) {
      if (socket->command_queue.frame_type == SLI_WLAN_RSP_SOCKET_READ_DATA) {
        sli_si91x_add_to_queue(&socket->rx_data_queue, queue_node->host_packet);
        sli_si91x_host_free_buffer(current_packet);
      } else {
        sli_si91x_add_to_queue(&socket->command_queue.rx_queue, current_packet);
      }
      sli_si91x_set_socket_event((1 << socket->index));
    } else {
      // no user thread is waiting for the response so flush the packet
      if (queue_node->flags & SLI_WIFI_PACKET_RESPONSE_PACKET) {
        sli_si91x_host_free_buffer(queue_node->host_packet);
      }
      sli_si91x_host_free_buffer(current_packet);
    }
    tx_socket_command_command_in_flight_queues_status |= (1 << socket->index);
    // Reset command trace fields in the queue
    socket->command_queue.command_in_flight = false;
    socket->command_queue.command_tickcount = 0;
    socket->command_queue.command_timeout   = 0;
    socket->command_queue.frame_type        = 0;
    socket->command_queue.event_mask        = 0;
  }

  // Start processing the TX queue
  current_packet = socket->command_queue.tx_queue.head;

  // Iterate through all packets in the command queue
  while (current_packet != NULL) {
    // Retrieve the queue node from the current packet
    queue_node = sli_wifi_host_get_buffer_data(current_packet, 0, NULL);
    if (queue_node == NULL) {
      // If queue node is NULL, exit atomic state and return error status
      CORE_ExitAtomic(state);
      return SL_STATUS_NOT_AVAILABLE;
    }

    // Assign next_packet to the next node in the list
    next_packet = (sl_wifi_buffer_t *)current_packet->node.node;

    // Check if the packet in the TX queue is synchronous or asynchronous
    if (queue_node->flags & SLI_WIFI_PACKET_RESPONSE_STATUS) {
      // Update the frame_status and other details for synchronous packets
      queue_node->frame_status  = frame_status;
      current_packet->node.node = NULL;
      int32_t frame_type        = sli_get_socket_command_from_host_packet(queue_node->host_packet);
      // Check if the TX packet is not expecting a response, then free the host packet
      if ((!(queue_node->flags & SLI_WIFI_PACKET_RESPONSE_PACKET)) && (queue_node->host_packet != NULL)) {
        sli_si91x_host_free_buffer(queue_node->host_packet);
        queue_node->host_packet = NULL;
      }
      sli_process_frame_type(queue_node, frame_type, frame_status, socket, current_packet);

    } else {
      // Handle asynchronous packets by freeing the buffer
      sli_si91x_host_free_buffer(queue_node->host_packet);
      sli_si91x_host_free_buffer(current_packet);
    }

    // Move to the next packet in the queue
    current_packet = next_packet;
  }

  // Clear the head and tail pointers of the TX queue
  socket->command_queue.tx_queue.head = NULL;
  socket->command_queue.tx_queue.tail = NULL;
  // Clear the socket command queue status
  tx_socket_command_queues_status &= ~(1 << socket->index);

  // Exit atomic state
  CORE_ExitAtomic(state);
  // Return successful status
  return SL_STATUS_OK;
}
sl_status_t sli_si91x_flush_all_socket_data_queues(uint8_t vap_id)
{
  sl_status_t status;

  // Loop through all sockets
  for (int index = 0; index < SLI_NUMBER_OF_SOCKETS; index++) {
    // Retrieve the socket using the provided index
    const sli_si91x_socket_t *socket = sli_si91x_sockets[index];

    // Check if the socket exists and matches the required VAP ID
    if ((socket != NULL) && (socket->vap_id == vap_id)) {
      // Flush the data queues for the current socket based on queue type
      status = sli_si91x_flush_socket_data_queues_based_on_queue_type((uint8_t)index);
      // If flushing fails, return the error status immediately
      if (status != SL_STATUS_OK) {
        return status;
      }
    }
  }
  // Return SL_STATUS_OK if all sockets were processed successfully
  return SL_STATUS_OK;
}

sl_status_t sli_si91x_flush_socket_data_queues_based_on_queue_type(uint8_t index)
{
  sl_wifi_buffer_t *current_packet = NULL;
  sl_wifi_buffer_t *next_packet    = NULL;

  // Retrieve the socket using the provided index
  sli_si91x_socket_t *socket = sli_get_si91x_socket(index);

  if (socket == NULL) {
    return SL_STATUS_FAIL;
  }

  // Enter atomic section to prevent race conditions
  CORE_irqState_t state = CORE_EnterAtomic();

  // Start with the head of the TX data queue
  current_packet = socket->tx_data_queue.head;

  // Iterate through all packets in the queue
  while (current_packet != NULL) {
    // Save the next packet in the queue
    next_packet = (sl_wifi_buffer_t *)current_packet->node.node;

    // Free the current packet using the provided free function
    sli_si91x_host_free_buffer(current_packet);

    // Move to the next packet
    current_packet = next_packet;
  }

  // Clear the head and tail pointers of the queue to indicate it is empty
  socket->tx_data_queue.head = NULL;
  socket->tx_data_queue.tail = NULL;

  // Clear the socket data queue status
  tx_socket_data_queues_status &= ~(1 << socket->index);

  // Exit atomic section
  CORE_ExitAtomic(state);

  // Return SL_STATUS_OK indicating the operation was successful
  return SL_STATUS_OK;
}

#endif

uint32_t sli_si91x_host_queue_status(const sli_wifi_buffer_queue_t *queue)
{
  return queue->head != NULL;
}

uint32_t sli_si91x_wait_for_event(uint32_t event_mask, uint32_t timeout)
{
  if (timeout != osWaitForever) {
    timeout = SLI_SYSTEM_MS_TO_TICKS(timeout);
  }

  uint32_t result = osEventFlagsWait(sli_wifi_events, event_mask, osFlagsWaitAny, timeout);

  if (result == (uint32_t)osErrorTimeout || result == (uint32_t)osErrorResource) {
    return 0;
  }
  return result;
}

uint32_t sli_si91x_clear_event(uint32_t event_mask)
{
  uint32_t result = osEventFlagsClear(sli_wifi_events, event_mask);
  if (result == (uint32_t)osErrorResource) {
    return 0;
  }
  return result;
}

sl_status_t sli_si91x_send_power_save_request(const sl_wifi_performance_profile_v2_t *wifi_profile,
                                              const sl_bt_performance_profile_t *bt_profile)
{
  sl_status_t status;
  sli_wifi_power_save_request_t power_save_request                = { 0 };
  sl_wifi_system_performance_profile_t selected_coex_profile_mode = { 0 };

  power_save_sequence_in_progress = true;

  // Disable power save mode by setting it to HIGH_PERFORMANCE profile
  status = sli_si91x_driver_send_command(SLI_WIFI_REQ_PWRMODE,
                                         SLI_WIFI_COMMON_CMD,
                                         &power_save_request,
                                         sizeof(sli_wifi_power_save_request_t),
                                         SLI_WIFI_RSP_PWRMODE_WAIT_TIME,
                                         NULL,
                                         NULL);
  if (status != SL_STATUS_OK) {
    // Reset flag on failure
    power_save_sequence_in_progress = false;
    return status;
  }

  if (NULL != wifi_profile) {
    // Save the new Wi-Fi profile
    sli_wifi_save_current_performance_profile(wifi_profile);
  }

  if (NULL != bt_profile) {
    // Save the new BT/BLE profile
    sli_save_bt_current_performance_profile(bt_profile);
  }

  // get the updated coex profile
  sli_get_coex_performance_profile(&selected_coex_profile_mode);

  // If the requested performance profile is HIGH_PERFORMANCE, no need to send the request to firmware
  if (selected_coex_profile_mode == HIGH_PERFORMANCE) {
    // Reset flag on failure
    power_save_sequence_in_progress = false;
    return SL_STATUS_OK;
  }

  // Convert the performance profile to a power save request.
  sli_convert_performance_profile_to_power_save_command(selected_coex_profile_mode, &power_save_request);

  status = sli_si91x_driver_send_command(SLI_WIFI_REQ_PWRMODE,
                                         SLI_WIFI_COMMON_CMD,
                                         &power_save_request,
                                         sizeof(sli_wifi_power_save_request_t),
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WIFI_RSP_PWRMODE_WAIT_TIME),
                                         NULL,
                                         NULL);
  // Reset flag on failure
  power_save_sequence_in_progress = false;
  return status;
}

sl_status_t sl_si91x_host_power_cycle(void)
{
  sl_si91x_host_hold_in_reset();
  sl_si91x_host_delay_ms(100);

  sl_si91x_host_release_from_reset();
  sl_si91x_host_delay_ms(100);

  return SL_STATUS_OK;
}

void print_80211_packet(const uint8_t *packet, uint32_t packet_length, uint16_t max_payload_length)
{
  uint32_t dump_bytes    = 0;
  uint32_t header_length = MAC80211_HDR_MIN_LEN;

  header_length += (packet[0] & BIT(7)) ? 2 : 0;                           /* 2 bytes QoS control */
  header_length += ((packet[1] & BIT(0)) && (packet[1] & BIT(1))) ? 6 : 0; /* 6 byte Addr4 */

  sl_debug_log("%02x %02x | ", packet[0], packet[1]); /* FC */
  sl_debug_log("%02x %02x | ", packet[2], packet[3]); /* Dur */
  sl_debug_log("%02x:%02x:%02x:%02x:%02x:%02x | ",
               packet[4],
               packet[5],
               packet[6],
               packet[7],
               packet[8],
               packet[9]); /* Addr1/RA */
  sl_debug_log("%02x:%02x:%02x:%02x:%02x:%02x | ",
               packet[10],
               packet[11],
               packet[12],
               packet[13],
               packet[14],
               packet[15]); /* Addr2/NWP */
  sl_debug_log("%02x:%02x:%02x:%02x:%02x:%02x | ",
               packet[16],
               packet[17],
               packet[18],
               packet[19],
               packet[20],
               packet[21]);                             /* Addr3/DA */
  sl_debug_log("%02x %02x | ", packet[22], packet[23]); /* Seq control */
  if ((packet[1] & BIT(0)) && (packet[1] & BIT(1))) {   /* Addr4 */
    sl_debug_log("%02x:%02x:%02x:%02x:%02x:%02x | ",
                 packet[24],
                 packet[25],
                 packet[26],
                 packet[27],
                 packet[28],
                 packet[29]);
  }
  if (packet[0] & BIT(7)) {
    sl_debug_log("%02x %02x | ", packet[30], packet[31]); /* QoS control */
  }

  // Determine number of payload bytes to print
  dump_bytes = packet_length - header_length;
  dump_bytes = max_payload_length > dump_bytes ? dump_bytes : max_payload_length;

  for (uint32_t i = header_length; i < header_length + dump_bytes; i++) {
    sl_debug_log("%02x ", packet[i]);
  }

  sl_debug_log("|\r\n");
}

sli_wifi_command_queue_t *sli_si91x_get_command_queue(sli_wifi_command_type_t type)
{
  switch (type) {
    case SLI_WIFI_WLAN_CMD:
      return &cmd_queues[SLI_WIFI_WLAN_CMD];
    case SLI_SI91X_NETWORK_CMD:
      return &cmd_queues[SLI_SI91X_NETWORK_CMD];
    case SLI_SI91X_BT_CMD:
      return &cmd_queues[SLI_SI91X_BT_CMD];
    case SLI_SI91X_SOCKET_CMD:
      return &cmd_queues[SLI_SI91X_SOCKET_CMD];

    case SLI_WIFI_COMMON_CMD:
      __attribute__((fallthrough));
    default:
      return &cmd_queues[SLI_WIFI_COMMON_CMD];
  }
}

/* Function to get the current status of the NVM command progress
Returns true if an NVM command is in progress, false otherwise*/
bool sli_si91x_get_flash_command_status()
{
  return sli_si91x_packet_status;
}

void sli_si91x_update_flash_command_status(bool flag)
{
  sli_si91x_packet_status = flag;
}

bool sli_si91x_get_tx_command_status()
{
  return sli_si91x_tx_command_status;
}

void sli_si91x_update_tx_command_status(bool flag)
{
  sli_si91x_tx_command_status = flag;
}

/*  This function is used to update the power manager to see whether the device is ready for sleep or not.
 True indicates ready for sleep, and false indicates not ready for sleep.*/
bool sli_si91x_is_sdk_ok_to_sleep()
{
  return ((!sli_si91x_get_flash_command_status()) && (sl_si91x_is_device_initialized())
          && (!sli_si91x_get_tx_command_status()));
}

bool sl_si91x_is_device_initialized(void)
{
  return device_initialized;
}

#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
// Implementation of SNI extension setting for embedded sockets
sl_status_t sli_si91x_set_sni_for_embedded_socket(const sli_si91x_tls_extension_info_t *sni_extension,
                                                  sli_si91x_sni_target_protocol_t sni_target_protocol)
{
  sl_status_t status     = SL_STATUS_OK;
  uint32_t packet_length = 0;

  if (sni_extension == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // Validate SNI extension type
  if (sni_extension->type != SL_SI91X_TLS_EXTENSION_SNI_TYPE) {
    return SL_STATUS_INVALID_TYPE;
  }

  // Validate length before any memory operations
  if (sni_extension->length == 0
      || sizeof(sli_si91x_tls_extension_info_t) + sni_extension->length > SLI_SI91X_MAX_SIZE_OF_EXTENSION_DATA) {
    return SL_STATUS_WOULD_OVERFLOW;
  }

  sli_si91x_sni_for_embedded_socket_request_t *request = (sli_si91x_sni_for_embedded_socket_request_t *)malloc(
    sizeof(sli_si91x_sni_for_embedded_socket_request_t) + SLI_SI91X_MAX_SIZE_OF_EXTENSION_DATA);
  SLI_VERIFY_MALLOC_AND_RETURN(request);

  memset(request, 0, sizeof(sli_si91x_sni_for_embedded_socket_request_t) + SLI_SI91X_MAX_SIZE_OF_EXTENSION_DATA);

  request->protocol = (uint16_t)sni_target_protocol;

  request->offset = sizeof(sli_si91x_tls_extension_info_t);
  memcpy(&request->tls_extension_data, sni_extension, SLI_SI91X_MAX_SIZE_OF_EXTENSION_DATA);
  request->offset += sni_extension->length;
  packet_length = sizeof(sli_si91x_sni_for_embedded_socket_request_t) + SLI_SI91X_MAX_SIZE_OF_EXTENSION_DATA;

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_SET_SNI_EMBEDDED,
                                         SLI_SI91X_NETWORK_CMD,
                                         request,
                                         packet_length,
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WLAN_RSP_SET_SNI_EMBEDDED_WAIT_TIME),
                                         NULL,
                                         NULL);
  free(request);

  return status;
}

// Helper function to configure SNI using either extension or hostname
sl_status_t sli_configure_sni(const sli_si91x_tls_extension_info_t *sni_extension,
                              const uint8_t *host_name,
                              sli_si91x_sni_target_protocol_t sni_target_protocol)
{
  if (sni_extension != NULL) {
    return sli_si91x_set_sni_for_embedded_socket(sni_extension, sni_target_protocol);
  }

  if (host_name != NULL && host_name[0] != '\0') {
    size_t host_name_length = sl_strlen((const char *)host_name);
    sl_status_t status      = SL_STATUS_OK;

    // Validate length before allocation
    if (host_name_length > SLI_SI91X_MAX_SIZE_OF_EXTENSION_DATA) {
      return SL_STATUS_SI91X_MEMORY_ERROR;
    }

    sli_si91x_tls_extension_info_t *tls_sni =
      (sli_si91x_tls_extension_info_t *)malloc(sizeof(sli_si91x_tls_extension_info_t) + host_name_length);
    if (tls_sni == NULL) {
      return SL_STATUS_ALLOCATION_FAILED;
    }

    tls_sni->type   = SL_SI91X_TLS_EXTENSION_SNI_TYPE;
    tls_sni->length = (uint16_t)host_name_length;
    memcpy(tls_sni->value, host_name, tls_sni->length);

    status = sli_si91x_set_sni_for_embedded_socket(tls_sni, sni_target_protocol);
    free(tls_sni);
    return status;
  }

  return SL_STATUS_OK;
}
#endif