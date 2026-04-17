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
#include "sl_wifi.h"
#include "sli_wifi.h"
#include "sl_si91x_host_interface.h"
#include "sl_wifi_types.h"
#include "sl_si91x_status.h"
#include "sl_si91x_types.h"
#include "sl_si91x_protocol_types.h"
#include "sl_wifi_device.h"
#include "sl_rsi_utility.h"
#include "sl_si91x_driver.h"
#include "cmsis_os2.h"
#include "sl_cmsis_utility.h"
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "sl_si91x_core_utilities.h"
#include "sl_core.h"
#include "sli_wifi_constants.h"
#include "sli_wifi_utility.h"
#include "sli_wifi_power_profile.h"
#include "sl_log_helper_si91x.h"
#ifdef SLI_SI91X_MCU_INTERFACE
#include "sli_siwx917_soc.h"
#include "rsi_rom_clks.h"
#include "rsi_m4.h"
#include "rsi_wisemcu_hardware_setup.h"
#endif
#ifdef SLI_SI91X_ENABLE_BLE
#include "rsi_common_apis.h"
#endif
#ifdef SL_WDT_MANAGER_PRESENT
#if defined SL_WDT_MANAGER_PRESENT_M4_UC_RC
#include "sl_si91x_wdt_rc_config.h"
#elif defined SL_WDT_MANAGER_PRESENT_M4_UC_XTAL
#include "sl_si91x_wdt_xtal_config.h"
#endif
#endif

#ifdef SLI_SI91X_SOCKETS
#include "sl_si91x_socket_utility.h"
#include "sl_si91x_socket_callback_framework.h"
#endif

// Define the event flag for command engine status notification if not already defined
#ifndef SLI_SI91X_NCP_HOST_COMMAND_ENGINE_STATUS_NOTIFICATION_EVENT
#define SLI_SI91X_NCP_HOST_COMMAND_ENGINE_STATUS_NOTIFICATION_EVENT (1 << (SI91X_CMD_MAX + 1))
#endif

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
#include "rsi_m4.h"
#define SLI_SIDE_BAND_DONE (1 << 2) //! had to be redefined as this macro is not in .h
rsi_m4ta_desc_t crypto_desc[2];
extern osEventFlagsId_t ta_events;
extern osMutexId_t side_band_crypto_mutex;
#endif

#ifndef NULL
#define NULL (void *)0
#endif

#define htole16(x) (x)
#define htole32(x) (x)

#ifndef SL_WIFI_SET_MAC_COMMAND_TIME_OUT
#define SL_WIFI_SET_MAC_COMMAND_TIME_OUT 30100 // Retrieved from SAPI 1.0

#endif

#define SLI_SI91X_GET_TCP_IP_TOTAL_SELECTS_BITS(x) ((x & 0x0000F000) >> 12)

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
#define SL_HOST_DESC_LEN  16
#define SL_CRYPTO_PKT_LEN 128
#endif

#ifdef SLI_SI91X_MCU_INTERFACE
// Combined Image defines
#define SLI_COMBINED_IMAGE             BIT(7)
#define SLI_COMBINED_IMAGE_SIZE_OFFSET 48
#endif

// Macro defining the timeout for command engine status queue operations.
#define SLI_SI91X_ERROR_QUEUE_TIMEOUT_MS 1000

// Enterprise configuration command parameters
/*=======================================================================*/

// Enterprise method ,should be one of among TLS, TTLS, FAST or PEAP

#define SL_EAP_TLS_METHOD  "TLS"
#define SL_EAP_TTLS_METHOD "TTLS"

// This parameter is used to configure the module in Enterprise security mode
#define SL_EAP_INNER_METHOD "\"auth=MSCHAPV2\""

// Private Key Password is required for encrypted private key, format is like "\"12345678\""
#define SL_DEFAULT_PRIVATE_KEY_PASSWORD ""

// Internal Wi-Fi transceiver mode configurations are currently not supported.
#define SLI_EIA_BIT_IN_CTRL_FLAG         BIT(6) //< Extended Information Available bit in Control Flags
#define SLI_EIA_BIT_IN_HOST_DESC         BIT(4) //< Extended Information Available bit in Host Descriptor
#define SLI_IMMEDIATE_TRF_CTRL_FLAGS     BIT(7) //< Immediate Transfer bit in Control Flags
#define SLI_IMMEDIATE_TRF_HOST_DESC      BIT(6) //< Immediate Transfer bit in Host Descriptor
#define SLI_LAST_PKT                     BIT(0) //< Last Packet bit in Control Flags1
#define SLI_IS_LAST_PKT(ctrl_flags1)     (ctrl_flags1 & SLI_LAST_PKT)
#define SLI_IS_EIA_PKT(ctrl_flags)       (ctrl_flags & SLI_EIA_BIT_IN_CTRL_FLAG)
#define SLI_IS_IMMEDIATE_TRF(ctrl_flags) (ctrl_flags & SLI_IMMEDIATE_TRF_CTRL_FLAGS)
#define SLI_EXT_DESC_SIZE_IF_EIA_PKT     5

/*========================================================================*/
// 11ax params
/*========================================================================*/
#define SLI_GUARD_INTERVAL 3

#ifdef SLI_SI91X_MCU_INTERFACE
#define SLI_TX_PKT_TRANSFER_DONE_INTERRUPT BIT(2)
#define SLI_RX_PKT_TRANSFER_DONE_INTERRUPT BIT(1)

// Function declarations related to M4 interface
sl_status_t sli_si91x_submit_rx_pkt(void);
static sl_status_t sl_si91x_soft_reset(void);
void sli_siwx917_update_system_core_clock(void);
void sli_m4_ta_interrupt_init(void);
#endif

#define SLI_SI91X_XTAL_GOOD_TIME_MIN 600  // XTAL good time lower bound (µs)
#define SLI_SI91X_XTAL_GOOD_TIME_MAX 5000 // XTAL good time upper bound (µs)

#define SLI_SI91X_PMU_GOOD_TIME_MAX     2000 // PMU good time common upper bound (µs)
#define SLI_SI91X_PMU_GOOD_TIME_MIN_SOC 900  // PMU good time lower bound for SoC
#define SLI_SI91X_PMU_GOOD_TIME_MIN_NCP 600  // PMU good time lower bound for NCP

#ifdef SLI_SI91X_MCU_INTERFACE
#define SLI_SI91X_PMU_GOOD_TIME_MIN SLI_SI91X_PMU_GOOD_TIME_MIN_SOC
#else
#define SLI_SI91X_PMU_GOOD_TIME_MIN SLI_SI91X_PMU_GOOD_TIME_MIN_NCP
#endif

// Structure to hold packet information and payload
typedef struct {
  uint16_t packet_id;
  sli_si91x_queue_packet_t *packet;
  void *payload;
} sl_si91x_driver_context_t;

static sl_wifi_timeout_t timeout_glbl = { .auth_assoc_timeout_value       = SL_WIFI_DEFAULT_AUTH_ASSOCIATION_TIMEOUT,
                                          .active_chan_scan_timeout_value = SL_WIFI_DEFAULT_ACTIVE_CHANNEL_SCAN_TIME,
                                          .keep_alive_timeout_value       = SL_WIFI_DEFAULT_KEEP_ALIVE_TIMEOUT,
                                          .passive_scan_timeout_value     = SL_WIFI_DEFAULT_PASSIVE_CHANNEL_SCAN_TIME };

sl_status_t sli_si91x_driver_send_command_packet(uint32_t command,
                                                 sli_wifi_command_type_t command_type,
                                                 sl_wifi_buffer_t *buffer,
                                                 sli_wifi_wait_period_t wait_period,
                                                 void *sdk_context,
                                                 sl_wifi_buffer_t **data_buffer);
static sl_status_t sl_si91x_driver_send_data_packet(sl_wifi_buffer_t *buffer, uint32_t wait_time);
sl_status_t sl_si91x_driver_raw_send_command(uint8_t command,
                                             const void *data,
                                             uint32_t data_length,
                                             uint32_t wait_time);
sl_status_t sl_si91x_allocate_data_buffer(sl_wifi_buffer_t **host_buffer,
                                          void **buffer,
                                          uint32_t data_size,
                                          uint32_t wait_duration_ms);
sl_status_t sl_si91x_driver_init_wifi_radio(const sl_wifi_device_configuration_t *config);
sl_status_t sli_verify_device_boot(uint32_t *rom_version);
sl_status_t sl_si91x_enable_radio(void);
sl_status_t sli_wifi_select_option(const uint8_t configuration);
sl_status_t sli_si91x_bootup_firmware(const uint8_t select_option, uint8_t image_number);
sl_status_t sl_si91x_host_power_cycle(void);
/**
 * @brief Handle command response and cleanup for SI91X driver
 * @param command_type Command type
 * @param wait_period Wait period configuration
 * @param packet_id Packet ID
 * @param data_buffer Pointer to data buffer (can be NULL)
 * @param response Pointer to response buffer
 * @return Status of the operation
 */
static sl_status_t sli_handle_si91x_command_response(sli_wifi_command_type_t command_type,
                                                     sli_wifi_wait_period_t wait_period,
                                                     uint8_t packet_id,
                                                     sl_wifi_buffer_t **data_buffer,
                                                     sl_wifi_buffer_t **response);

// This variable stores the frame status of response packet in case of API executed being failed.
// Note: This will not store the error values of asynchronous events.
sl_wifi_event_handler_t si91x_event_handler = NULL;

// Global variables for device and driver management
sl_wifi_interface_t default_interface;
bool device_initialized = false;

// XTAL and PMU good time values provided via sl_si91x_set_nwp_config_request().
// Each may be configured before initialization; both are applied (XTAL first, then PMU) during driver init.
static uint16_t sli_xtal_good_time_us = 0; // Valid: 600-5000 µs.
static uint16_t sli_pmu_good_time_us  = 0; // Valid: 900-2000 µs for SoC, 600-2000 µs for NCP.

bool interface_is_up[SL_WIFI_MAX_INTERFACE_INDEX] = { false, false, false, false, false };
bool bg_enabled                                   = false;
uint32_t frontend_switch_control                  = 0;
static uint32_t feature_bit_map                   = 0;
static sli_wifi_efuse_data_t si91x_efuse_data     = { 0 };
//! Currently, initialized_opermode is used only to handle concurrent mode using sl_net_init()
extern uint16_t initialized_opermode;
extern sli_wifi_command_queue_t cmd_queues[SI91X_CMD_MAX];
extern sli_wifi_buffer_queue_t sli_tx_data_queue;
extern osEventFlagsId_t sli_wifi_events;
extern volatile uint32_t tx_command_queues_status;
extern volatile uint32_t tx_generic_socket_data_queues_status;
osMessageQueueId_t sli_command_engine_status_msg_queue = NULL;
extern osEventFlagsId_t si91x_async_events;

#ifdef SLI_SI91X_ENABLE_BLE
//! Memory length for driver
#define GLOBAL_BUFF_LEN 1500

//! Memory to initialize driver
uint8_t global_buf[GLOBAL_BUFF_LEN] = { 0 };
#endif
// clang-format off
const sl_wifi_scan_configuration_t default_wifi_scan_configuration = { .type  = SL_WIFI_SCAN_TYPE_ACTIVE,
                                                                       .flags = 0,
                                                                       .periodic_scan_interval = 0,
                                                                       .channel_bitmap_2g4     = 0xFFFF,
                                                                       .channel_bitmap_5g      = { 0xFFFFFFFF,
                                                                                              0xFFFFFFFF,
                                                                                              0xFFFFFFFF,
                                                                                              0xFFFFFFFF,
                                                                                              0xFFFFFFFF,
                                                                                              0xFFFFFFFF,
                                                                                              0xFFFFFFFF,
                                                                                              0xFFFFFFFF }, 
                                                                       .lp_mode = 0};

// clang-format on
const sl_wifi_buffer_configuration_t default_buffer_configuration = {
  .control_buffer_quota = 10,
  .tx_buffer_quota      = 10,
  .rx_buffer_quota      = 10,
#ifdef SPI_EXTENDED_TX_LEN_2K
  .block_size = 2300,
#else
  .block_size = 1616,
#endif
  .buffer_memory = NULL,
};

const sl_wifi_ap_configuration_t default_wifi_ap_configuration = {
  .ssid                = { .value = "SILICON_LABS_AP", .length = sizeof("SILICON_LABS_AP") - 1 },
  .security            = SL_WIFI_WPA2,
  .encryption          = SL_WIFI_CCMP_ENCRYPTION,
  .channel             = { .channel = 11 },
  .rate_protocol       = SL_WIFI_RATE_PROTOCOL_AUTO,
  .options             = 0,
  .credential_id       = 2, //SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID,
  .keepalive_type      = SL_WIFI_AP_NULL_BASED_KEEP_ALIVE,
  .beacon_interval     = 100,
  .client_idle_timeout = 255,
  .dtim_beacon_count   = 4,
  .maximum_clients     = 4
};

// clang-format off
static uint8_t firmware_queue_id[SI91X_CMD_MAX]   = { [SLI_WIFI_COMMON_CMD]  = SLI_WLAN_MGMT_Q,
                                                    [SLI_WIFI_WLAN_CMD]      = SLI_WLAN_MGMT_Q,
                                                    [SLI_SI91X_NETWORK_CMD]   = SLI_WLAN_MGMT_Q,
													[SLI_SI91X_SOCKET_CMD]    = SLI_WLAN_MGMT_Q,
                                                    [SLI_SI91X_BT_CMD]        = SLI_BT_Q };
//static uint32_t response_event_map[SI91X_CMD_MAX] = { [SLI_WIFI_COMMON_CMD]  = NCP_HOST_COMMON_RESPONSE_EVENT,
//                                                      [SLI_WIFI_WLAN_CMD]    = NCP_HOST_WLAN_RESPONSE_EVENT,
//                                                      [SLI_SI91X_NETWORK_CMD] = NCP_HOST_NETWORK_RESPONSE_EVENT,
//                                                      [SLI_SI91X_SOCKET_CMD]  = NCP_HOST_SOCKET_RESPONSE_EVENT,
//                                                      [SLI_SI91X_BT_CMD]      = NCP_HOST_BT_RESPONSE_EVENT };
// clang-format on
#ifdef SLI_SI91X_MCU_INTERFACE
extern sl_wifi_buffer_t *rx_pkt_buffer;
#endif

static bool sli_si91x_packet_identification_function(const sl_wifi_buffer_t *buffer, const void *user_data)
{
  const uint8_t *packet_id = (const uint8_t *)user_data;

  // Check if the packet's packet ID matches the expected one
  return (*packet_id == buffer->id);
}

static sl_status_t sli_handle_si91x_command_response(sli_wifi_command_type_t command_type,
                                                     sli_wifi_wait_period_t wait_period,
                                                     uint8_t packet_id,
                                                     sl_wifi_buffer_t **data_buffer,
                                                     sl_wifi_buffer_t **response)
{
  sl_status_t status;
  sli_wifi_wait_period_t wait_time = 0;
  uint16_t firmware_status;
  sli_si91x_queue_packet_t *node;
  uint16_t data_length = 0;

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
                                             SL_SI91X_RESPONSE_FLAG(command_type),
                                             packet_id,
                                             wait_time,
                                             response);
  // Check if the status is SL_STATUS_TIMEOUT, indicating a timeout has occurred
  if (status == SL_STATUS_TIMEOUT) {
    // Declare a temporary packet pointer to hold the packet to be removed
    sl_wifi_buffer_t *temp_packet;
    sl_status_t temp_status = sli_wifi_remove_buffer_from_queue_by_comparator(&cmd_queues[command_type].tx_queue,
                                                                              &packet_id,
                                                                              sli_si91x_packet_identification_function,
                                                                              &temp_packet);

    // Check if the packet removal was successful
    if (temp_status == SL_STATUS_OK) {

      // Retrieve the actual packet node data from the removed buffer
      sli_si91x_queue_packet_t *temp_node = sli_wifi_host_get_buffer_data(temp_packet, 0, NULL);

      // Free the host packet memory associated with the node (TX packet memory)
      sli_si91x_host_free_buffer(temp_node->host_packet);

      // Free the temporary buffer memory that held the packet
      sli_si91x_host_free_buffer(temp_packet);
    }
  }
  VERIFY_STATUS_AND_RETURN(status);

  // Process the response packet and return the firmware status
  node            = (sli_si91x_queue_packet_t *)sli_wifi_host_get_buffer_data(*response, 0, &data_length);
  firmware_status = node->frame_status;

  // If a data_buffer is provided, set it to the host_packet
  if (NULL != data_buffer) {
    *data_buffer = node->host_packet;
  }
  // If the response packet flag is set, free the host_packet buffer
  else if (SLI_WIFI_PACKET_RESPONSE_PACKET == (node->flags & SLI_WIFI_PACKET_RESPONSE_PACKET)) {
    sli_si91x_host_free_buffer(node->host_packet);
  }

  // Free the response buffer and return the firmware status
  sli_si91x_host_free_buffer(*response);
  return sli_convert_and_save_firmware_status(firmware_status);
}

void sli_si91x_get_efuse_data(sli_wifi_efuse_data_t *efuse_data)
{
  memcpy(efuse_data, &si91x_efuse_data, sizeof(sli_wifi_efuse_data_t));
}

void sli_si91x_set_efuse_data(const sli_wifi_efuse_data_t *efuse_data)
{
  memcpy(&si91x_efuse_data, efuse_data, sizeof(sli_wifi_efuse_data_t));
}

// Function to initialize the message queue
sl_status_t sli_command_engine_status_queue_init()
{
  if (sli_command_engine_status_msg_queue == NULL) {
    // Attempt to create the command engine status message queue
    sli_command_engine_status_msg_queue = osMessageQueueNew(10, sizeof(sl_status_t), NULL);
    if (sli_command_engine_status_msg_queue != NULL) {
      return SL_STATUS_OK;
    } else {
      SL_PRINT_STRING_ERROR("\r\nERROR: FAILED TO CREATE COMMAND ENGINE MESSAGE QUEUE\r\n");
      return SL_STATUS_FAIL;
    }
  }
  return SL_STATUS_OK;
}

// Function to Deinitialize the message queue
sl_status_t sli_command_engine_status_queue_deinit()
{
  if (sli_command_engine_status_msg_queue != NULL) {
    // Attempt to delete the message queue
    if (osMessageQueueDelete(sli_command_engine_status_msg_queue) == osOK) {
      sli_command_engine_status_msg_queue = NULL; // Clear the queue handle
      return SL_STATUS_OK;
    } else {
      SL_PRINT_STRING_ERROR("\r\n Failed to delete the queue.\r\n");
      return SL_STATUS_FAIL;
    }
  }
  return SL_STATUS_OK;
}

/***************************************************************************/ /**
* @brief
*   Enqueue a status message to the command engine status queue and set an event.
*
* @details
*   Implementation of the function to add a status code to the command engine status queue
*   and trigger an event flag to notify listeners of a critical error condition.
*
* @param[in] status
*   The status code indicating the error condition to be enqueued.
******************************************************************************/
void sli_command_engine_status_queue_enqueue_and_set_event(sl_status_t status)
{
  // Check if the queue is initialized
  if (sli_command_engine_status_msg_queue == NULL) {
    SL_PRINT_STRING_ERROR("Command engine status queue not initialized\r\n");
    return;
  }

  // Attempt to enqueue the error_status into the message queue
  if (osMessageQueuePut(sli_command_engine_status_msg_queue, &status, 0, SLI_SI91X_ERROR_QUEUE_TIMEOUT_MS) != osOK) {
    SL_PRINT_STRING_ERROR("Failed to enqueue status message into command engine status queue\r\n");
  } else {
    // Trigger the event once the error_status is successfully enqueued
    osEventFlagsSet(si91x_async_events, SLI_SI91X_NCP_HOST_COMMAND_ENGINE_STATUS_NOTIFICATION_EVENT);
  }
}

sl_status_t sl_si91x_driver_init_wifi_radio(const sl_wifi_device_configuration_t *config)
{
  sl_status_t status;

// Set 11ax configuration with guard interval if SLI_SI91X_CONFIG_WIFI6_PARAMS is supported
#ifdef SLI_SI91X_CONFIG_WIFI6_PARAMS
  status = sl_wifi_set_11ax_config(SLI_GUARD_INTERVAL);
  VERIFY_STATUS_AND_RETURN(status);
#endif

  // Send WLAN request to set the operating band (2.4GHz or 5GHz)
  status = sli_si91x_driver_send_command(SLI_WIFI_REQ_BAND,
                                         SLI_WIFI_WLAN_CMD,
                                         &config->band,
                                         1,
                                         SLI_WIFI_RSP_BAND_WAIT_TIME,
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);

  if (config->boot_config.oper_mode != SL_SI91X_ACCESS_POINT_MODE) {
    if (timeout_glbl.active_chan_scan_timeout_value != SL_WIFI_DEFAULT_ACTIVE_CHANNEL_SCAN_TIME) {
      status =
        sl_si91x_configure_timeout(SL_SI91X_CHANNEL_ACTIVE_SCAN_TIMEOUT, timeout_glbl.active_chan_scan_timeout_value);
      VERIFY_STATUS_AND_RETURN(status);
    }

    if (timeout_glbl.auth_assoc_timeout_value != SL_WIFI_DEFAULT_AUTH_ASSOCIATION_TIMEOUT) {
      status =
        sl_si91x_configure_timeout(SL_SI91X_AUTHENTICATION_ASSOCIATION_TIMEOUT, timeout_glbl.auth_assoc_timeout_value);
      VERIFY_STATUS_AND_RETURN(status);
    }

    if (timeout_glbl.keep_alive_timeout_value != SL_WIFI_DEFAULT_KEEP_ALIVE_TIMEOUT) {
      status = sl_si91x_configure_timeout(SL_SI91X_KEEP_ALIVE_TIMEOUT, timeout_glbl.keep_alive_timeout_value);
      VERIFY_STATUS_AND_RETURN(status);
    }
    if (timeout_glbl.passive_scan_timeout_value != SL_WIFI_DEFAULT_PASSIVE_CHANNEL_SCAN_TIME) {
      status =
        sl_si91x_configure_timeout(SL_SI91X_CHANNEL_PASSIVE_SCAN_TIMEOUT, timeout_glbl.passive_scan_timeout_value);
      VERIFY_STATUS_AND_RETURN(status);
    }
  }

  // Initialize the WLAN subsystem
  status = sli_si91x_driver_send_command(SLI_WIFI_REQ_INIT,
                                         SLI_WIFI_WLAN_CMD,
                                         NULL,
                                         0,
                                         SLI_WIFI_RSP_INIT_WAIT_TIME,
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);

#ifndef SL_SI91X_ACX_MODULE
  if (SL_WIFI_IGNORE_REGION != config->region_code) {
    // Set the device's region based on configuration
    status = sl_si91x_set_device_region(config->boot_config.oper_mode, config->band, config->region_code);
    SL_DEBUG_LOG("Region code set to %d\r\n", config->region_code);
    VERIFY_STATUS_AND_RETURN(status);
  }
#else
  SL_DEBUG_LOG("Region code configuration skipped for Modules\r\n");
#endif

  // Configure the RTS threshold for WLAN
  sli_wifi_config_request_t config_request = { .config_type = SLI_WIFI_CONFIG_RTS_THRESHOLD,
                                               .value       = SLI_WIFI_RTS_THRESHOLD };
  status                                   = sli_si91x_driver_send_command(SLI_WIFI_REQ_CONFIG,
                                         SLI_WIFI_WLAN_CMD,
                                         &config_request,
                                         sizeof(config_request),
                                         SLI_WLAN_RSP_CONFIG_WAIT_TIME,
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

static sl_status_t sli_apply_xtal_pmu_good_time(uint16_t value, uint32_t code)
{
  if ((value == 0) || (code == 0)) {
    return SL_STATUS_OK; // Nothing to apply
  }
  sl_si91x_nwp_configuration_t cfg = { 0 };
  cfg.code                         = code;
  cfg.values.config_val            = value;
  return sli_si91x_driver_send_command(SLI_COMMON_REQ_SET_CONFIG,
                                       SLI_WIFI_COMMON_CMD,
                                       &cfg,
                                       sizeof(sl_si91x_nwp_configuration_t),
                                       SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_SET_CONFIG_WAIT_TIME),
                                       NULL,
                                       NULL);
}

static void sli_si91x_set_device_initialized_status(const sl_wifi_device_configuration_t *config)
{
  device_initialized   = true;
  initialized_opermode = config->boot_config.oper_mode;

  // Set interface status flags based on operating mode and band
  if ((config->boot_config.oper_mode == SL_SI91X_CLIENT_MODE)
      || (config->boot_config.oper_mode == SL_SI91X_ENTERPRISE_CLIENT_MODE)
      || (config->boot_config.oper_mode == SL_SI91X_CONCURRENT_MODE)
      || (config->boot_config.oper_mode == SL_SI91X_TRANSMIT_TEST_MODE)) {
    if (config->band == SL_WIFI_DUAL_BAND_MODE) {
      interface_is_up[SL_WIFI_CLIENT_DUAL_INTERFACE_INDEX] = true;
    } else if (config->band == SL_WIFI_BAND_MODE_5GHZ) {
      interface_is_up[SL_WIFI_CLIENT_5GHZ_INTERFACE_INDEX] = true;
    } else if (config->band == SL_WIFI_BAND_MODE_2_4GHZ) {
      interface_is_up[SL_WIFI_CLIENT_2_4GHZ_INTERFACE_INDEX] = true;
    }
  }
}

sl_status_t sl_si91x_driver_init(const sl_wifi_device_configuration_t *config, sl_wifi_event_handler_t event_handler)
{
  sl_status_t status;
  sl_si91x_host_init_configuration_t init_config = { 0 };
  const uint8_t select_option                    = config->boot_option;
  if (config->boot_config.coex_mode == SL_SI91X_WLAN_MODE) { // Check for not support mode
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Determine the default interface based on operating mode (AP or Client)
  if (config->boot_config.oper_mode == SL_SI91X_ACCESS_POINT_MODE) {
    default_interface = SL_WIFI_AP_INTERFACE;
  } else {
    default_interface = SL_WIFI_CLIENT_INTERFACE;
  }

  // Configure the interface for 5GHz band if selected (currently not supported for Si91x)
  switch (config->band) {
    case SL_WIFI_DUAL_BAND_MODE:
      default_interface |= SL_WIFI_DUAL_INTERFACE;
      break;
    case SL_WIFI_BAND_MODE_5GHZ:
      default_interface |= SL_WIFI_5GHZ_INTERFACE;
      break;
    case SL_WIFI_BAND_MODE_2_4GHZ:
      default_interface |= SL_WIFI_2_4GHZ_INTERFACE;
      break;
  }

  // Set the event handler for the SI91x wireless driver
  si91x_event_handler = event_handler;

  // Check if the device is already initialized
  if (device_initialized) {
    // If it's already initialized, ensure the requested operating mode is compatible
    if (initialized_opermode == SL_SI91X_CONCURRENT_MODE) {
      return (initialized_opermode == config->boot_config.oper_mode) ? SL_STATUS_OK : SL_STATUS_WIFI_INVALID_OPERMODE;
    }
    return SL_STATUS_ALREADY_INITIALIZED;
  }
  // Initialize BLE if BLE is enabled
#ifdef SLI_SI91X_ENABLE_BLE
  int32_t rsi_status = 0;
  rsi_status         = rsi_ble_driver_init(global_buf, GLOBAL_BUFF_LEN);
  if ((rsi_status < 0) || (rsi_status > GLOBAL_BUFF_LEN)) {
    return SL_STATUS_FAIL;
  }
#endif

#ifndef SLI_SI91X_ENABLE_IPV6
  const uint32_t *tcp_ip_feature_bit_map = &(config->boot_config.tcp_ip_feature_bit_map);

  // check if the module network stack is in IPV6 mode.
  if (*tcp_ip_feature_bit_map & (SLI_SI91X_IPV6_MODE)) {
    return SL_STATUS_NOT_AVAILABLE;
  }
#endif

  // Initialize the buffer manager
  status = sli_si91x_host_init_buffer_manager(&default_buffer_configuration);
  if (status != SL_STATUS_OK) {
    return status;
  }

  init_config.rx_irq      = sli_si91x_bus_rx_irq_handler;
  init_config.rx_done     = sli_si91x_bus_rx_done_handler;
  init_config.boot_option = config->boot_option;

  // Initialize the SI91x host
  status = sl_si91x_host_init(&init_config);
  VERIFY_STATUS_AND_RETURN(status);

  // Initialize the SI91x platform
  status = sl_si91x_platform_init();
  VERIFY_STATUS_AND_RETURN(status);

  // Power cycle the SI91x device
  status = sl_si91x_host_power_cycle();
  VERIFY_STATUS_AND_RETURN(status);

  // sl_si91x_bus_init() will be implemented for all available buses
  status = sl_si91x_bus_init();
  VERIFY_STATUS_AND_RETURN(status);

#ifdef SLI_SI91X_MCU_INTERFACE
  // firmware bootup is require only for the first time, no need to do it again if we call init after deinit
  static bool is_bootup_firmware_required = true;
  if (is_bootup_firmware_required) {
    status = sli_si91x_bootup_firmware(select_option, config->nwp_fw_image_number);
    VERIFY_STATUS_AND_RETURN(status);
    is_bootup_firmware_required = false;
  } else {
    // Initialize NWP interrupt and submit RX packets
    sli_m4_ta_interrupt_init();
    status = sli_si91x_submit_rx_pkt();
    if (status != SL_STATUS_OK) {
      SL_PRINT_STRING_ERROR("\r\n RX packet submission failed with status: %d \r\n", status);
      return status;
    }
  }
#else
  status = sli_si91x_bootup_firmware(select_option, config->nwp_fw_image_number);
  VERIFY_STATUS_AND_RETURN(status);
#endif

  if (select_option == BURN_NWP_FW) {
    sli_si91x_set_device_initialized_status(config);
    return SL_STATUS_OK;
  }

  // Initialize task register index to save firmware status
  status = sli_fw_status_storage_index_init();
  VERIFY_STATUS_AND_RETURN(status);

#ifdef SL_SI91X_SPI_HIGH_SPEED_ENABLE
  // Enable high speed bus on the device and the host
  status = sli_si91x_bus_enable_high_speed();
  VERIFY_STATUS_AND_RETURN(status);
  sl_si91x_host_enable_high_speed_bus();
#endif
  sl_si91x_host_enable_bus_interrupt();

// Wait for card ready command response
#ifdef SLI_SI91X_MCU_INTERFACE
  // NWP would not send card ready command response, if we call init after deinit

  if (sli_wifi_get_card_ready_required()) {
    uint32_t events = sli_si91x_wait_for_event(NCP_HOST_COMMON_RESPONSE_EVENT, 5000);
    if (!(events & NCP_HOST_COMMON_RESPONSE_EVENT)) {
      return SL_STATUS_CARD_READY_TIMEOUT;
    }
    sli_wifi_set_card_ready_required(false);
  }
#else
  uint32_t events = sli_si91x_wait_for_event(NCP_HOST_COMMON_RESPONSE_EVENT, 3000);
  if (!(events & NCP_HOST_COMMON_RESPONSE_EVENT)) {
    return SL_STATUS_CARD_READY_TIMEOUT;
  }
#endif

  // Apply XTAL and PMU good times (provided via sl_si91x_set_nwp_config_request()). XTAL is applied before PMU.
  if (sli_xtal_good_time_us != 0) {
    status = sli_apply_xtal_pmu_good_time(sli_xtal_good_time_us, SL_SI91X_SET_XTAL_GOOD_TIME_FROM_HOST);
    VERIFY_STATUS_AND_RETURN(status);
    sli_xtal_good_time_us = 0;
  }
  if (sli_pmu_good_time_us != 0) {
    status = sli_apply_xtal_pmu_good_time(sli_pmu_good_time_us, SL_SI91X_SET_PMU_GOOD_TIME_FROM_HOST);
    VERIFY_STATUS_AND_RETURN(status);
    sli_pmu_good_time_us = 0;
  }

  sl_wifi_system_boot_configuration_t boot_config = config->boot_config;

  // In coex mode, 160 MHz SoC clock is not supported.
  // If 160 MHz is configured in coex mode, it will automatically fall back to 120 MHz.
  if ((boot_config.coex_mode) && (boot_config.custom_feature_bit_map & SL_SI91X_CUSTOM_FEAT_SOC_CLK_CONFIG_160MHZ)) {
    SL_DEBUG_LOG("\r\n 160 MHz clock is not supported in coex mode. Falling back to 120 MHz.\r\n");
    // Clamp the configuration to 120 MHz
    boot_config.custom_feature_bit_map &= ~SL_SI91X_CUSTOM_FEAT_SOC_CLK_CONFIG_160MHZ;
    boot_config.custom_feature_bit_map |= SL_SI91X_CUSTOM_FEAT_SOC_CLK_CONFIG_120MHZ;
  }

  // Send WLAN request to set the operating mode and configuration
  status = sli_si91x_driver_send_command(SLI_WIFI_REQ_OPERMODE,
                                         SLI_WIFI_COMMON_CMD,
                                         &boot_config,
                                         sizeof(sl_wifi_system_boot_configuration_t),
                                         SLI_WIFI_RSP_OPERMODE_WAIT_TIME,
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);
#if (SL_WDT_MANAGER_PRESENT_NWP == 1)
  sl_si91x_nwp_configuration_t nwp_config;

  // NWP WDT configuration
  memset(&nwp_config, 0, sizeof(sl_si91x_nwp_configuration_t));
  nwp_config.code                    = SL_SI91X_ENABLE_NWP_WDT_FROM_HOST;
  nwp_config.values.wdt_timer_val    = SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_PERIOD_NWP;
  nwp_config.values.wdt_enable_in_ps = 0;
  status                             = sl_si91x_set_nwp_config_request(nwp_config);
  VERIFY_STATUS_AND_RETURN(status);
#endif

  feature_bit_map = config->boot_config.feature_bit_map;

#ifdef SLI_SI91X_ENABLE_BLE
  if (config->boot_config.coex_mode == SL_SI91X_BLE_MODE || config->boot_config.coex_mode == SL_SI91X_WLAN_BLE_MODE) {
    // Wait for BT card ready
    rsi_bt_common_init();
  }
#endif

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_DYNAMIC_POOL,
                                         SLI_WIFI_WLAN_CMD,
                                         &config->ta_pool,
                                         sizeof(sl_wifi_system_dynamic_pool_t),
                                         SLI_WLAN_RSP_DYNAMIC_POOL_WAIT_TIME,
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);

  // Configure various wireless features
  sli_wifi_feature_frame_config_t current_config = sli_wifi_get_feature_frame_config();

  sli_si91x_feature_frame_request feature_frame_request = { .pll_mode        = (uint8_t)current_config.pll_mode,
                                                            .rf_type         = RF_TYPE,
                                                            .wireless_mode   = (uint8_t)current_config.power_chain,
                                                            .enable_ppp      = ENABLE_PPP,
                                                            .afe_type        = AFE_TYPE,
                                                            .feature_enables = SLI_FEATURE_ENABLES };

  // Set PLL mode to 1 when 120 MHz or 160 MHz SoC clock is configured
  if (boot_config.custom_feature_bit_map
      & (SL_SI91X_CUSTOM_FEAT_SOC_CLK_CONFIG_160MHZ | SL_SI91X_CUSTOM_FEAT_SOC_CLK_CONFIG_120MHZ)) {
    feature_frame_request.pll_mode = 1;
  } else {
    feature_frame_request.pll_mode = 0;
  }

  // For the transmit test mode we need to disable BIT 0, 4, 5. These bitmaps are only required in powersave.
  // To receive broadcast data packets in transceiver opermode, we need to enable BIT 1.
  if (config->boot_config.oper_mode == SL_SI91X_TRANSMIT_TEST_MODE) {
    feature_frame_request.feature_enables &= ~(SLI_FEATURE_ENABLES);
  } else if (config->boot_config.oper_mode == SL_SI91X_TRANSCEIVER_MODE) {
    feature_frame_request.feature_enables |= SLI_FEAT_FRAME_PERMIT_UNDESTINED_PACKETS;
  } else {
    feature_frame_request.feature_enables = feature_frame_request.feature_enables;
  }

  // Dispatch a feature request frame to the SI91x driver
  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_FEATURE_FRAME,
                                         SLI_WIFI_COMMON_CMD,
                                         &feature_frame_request,
                                         sizeof(feature_frame_request),
                                         SLI_COMMON_RSP_FEATURE_FRAME_WAIT_TIME,
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);

  // if 16th bit of ext_tcp_ip_feature_bit_map is not set, then firmware auto closes the TCP socket on remote termination.
  sli_save_tcp_auto_close_choice(
    (config->boot_config.ext_tcp_ip_feature_bit_map & SL_SI91X_EXT_TCP_IP_WAIT_FOR_SOCKET_CLOSE) == 0);

#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
  sli_si91x_socket_init(SLI_SI91X_GET_TCP_IP_TOTAL_SELECTS_BITS(config->boot_config.ext_tcp_ip_feature_bit_map));
  VERIFY_STATUS_AND_RETURN(status);
#endif

  // Set the MAC address if provided in the configuration
  if (config->mac_address != NULL) {
    status = sl_wifi_set_mac_address(default_interface, config->mac_address);
    VERIFY_STATUS_AND_RETURN(status);
  }

  // Initialize the WiFi radio if the coexistence mode is not BLE
  if (config->boot_config.coex_mode != SL_SI91X_BLE_MODE) {
    status = sl_si91x_driver_init_wifi_radio(config);
    VERIFY_STATUS_AND_RETURN(status);
  }

  // Check and update the frontend switch control based on custom feature bit map
#ifndef __ZEPHYR__
  if (config->boot_config.custom_feature_bit_map & SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID) {
#else
  if (config->boot_config.custom_feature_bit_map & SL_SI91X_CUSTOM_FEAT_EXTENSION_VALID) {
#endif
    frontend_switch_control = (config->boot_config.ext_custom_feature_bit_map & (BIT(29) | (BIT(30))));
  }

#ifdef SLI_SI91X_MCU_INTERFACE
  // Program wireless GPIO front-end switch controls
  if (frontend_switch_control != 0) {
    sli_si91x_configure_wireless_frontend_controls(frontend_switch_control);
  }
#endif

  sli_si91x_set_device_initialized_status(config);
  // Save the coexistence mode in the driver
  sli_save_coex_mode(config->boot_config.coex_mode);
#ifdef SL_SI91X_GET_EFUSE_DATA
  status = sli_si91x_get_flash_efuse_data(&si91x_efuse_data, config->efuse_data_type);
#endif
#ifdef SLI_SI91X_MCU_INTERFACE
  if (status == SL_STATUS_OK) {
    /* send a notification to the NWP indicating whether the M4 core is currently utilizing the XTAL as its clock source*/
    sli_si91x_send_m4_xtal_usage_notification_to_ta();
  }
#endif

  return status;
}

sl_status_t sl_si91x_driver_deinit(void)
{
  sl_status_t status = SL_STATUS_OK;

  // Check if the device has been initialized if not, return an error
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

#ifdef SLI_SI91X_MCU_INTERFACE
  // If the SLI_SI91X_MCU_INTERFACE is defined, perform a soft reset
  status = sl_si91x_soft_reset();
  VERIFY_STATUS_AND_RETURN(status);

  // Mask specific interrupts related to packet transfer
  mask_ta_interrupt(SLI_TX_PKT_TRANSFER_DONE_INTERRUPT | SLI_RX_PKT_TRANSFER_DONE_INTERRUPT);
#endif

#ifdef SLI_SI91X_ENABLE_BLE
  int32_t rsi_status = 0;
  // If SLI_SI91X_ENABLE_BLE is defined, deinitialize the BLE driver and check for errors
  rsi_status = rsi_ble_driver_deinit();
  if (rsi_status != RSI_SUCCESS) {
    return SL_STATUS_FAIL;
  }
#endif

  // Deinitialize the SI91x platform
  status = sli_si91x_platform_deinit();
  VERIFY_STATUS_AND_RETURN(status);

  // Deinitialize the SI91x host
  status = sl_si91x_host_deinit();
  VERIFY_STATUS_AND_RETURN(status);

  // Flush all TX Wi-Fi queues with the status indicating Wi-Fi connection is lost
  sli_si91x_flush_all_tx_wifi_queues((uint16_t)SL_STATUS_WIFI_CONNECTION_LOST);

  // Flush the generic TX data queue
  sli_si91x_flush_generic_data_queues(&sli_tx_data_queue);

#if defined(SLI_SI91X_OFFLOAD_NETWORK_STACK) && defined(SLI_SI91X_SOCKETS)

  // Flush all pending socket commands in the client VAP queue due to Wi-Fi connection loss
  sli_si91x_flush_all_socket_command_queues((uint16_t)SL_STATUS_WIFI_CONNECTION_LOST, SL_WIFI_CLIENT_VAP_ID);

  // Flush all pending socket data in the client VAP queue due to Wi-Fi connection loss
  sli_si91x_flush_all_socket_data_queues(SL_WIFI_CLIENT_VAP_ID);

  // Flush all pending socket commands in the AP VAP queue due to Wi-Fi connection loss
  sli_si91x_flush_all_socket_command_queues((uint16_t)SL_STATUS_WIFI_CONNECTION_LOST, SL_WIFI_AP_VAP_ID);

  // Flush all pending socket data in the AP VAP queue due to Wi-Fi connection loss
  sli_si91x_flush_all_socket_data_queues(SL_WIFI_AP_VAP_ID);

  // Shutdown and change the state of the client VAP sockets
  status = sli_si91x_vap_shutdown(SL_WIFI_CLIENT_VAP_ID, SLI_SI91X_BSD_DISCONNECT_REASON_INTERFACE_DOWN);
  VERIFY_STATUS_AND_RETURN(status);

  // Shutdown and change the state of the AP VAP sockets
  status = sli_si91x_vap_shutdown(SL_WIFI_AP_VAP_ID, SLI_SI91X_BSD_DISCONNECT_REASON_INTERFACE_DOWN);
  VERIFY_STATUS_AND_RETURN(status);

  // Deinitialize and free all socket-related resources
  status = sli_si91x_socket_deinit();
  VERIFY_STATUS_AND_RETURN(status);

#endif // End of check for offloaded network stack and sockets

#ifdef SLI_SI91X_MCU_INTERFACE
  // Check the RX buffer valid bit is set or not.
  if (M4SS_P2P_INTR_SET_REG & RX_BUFFER_VALID) {

    // Clear the RX buffer valid bit.
    M4SS_P2P_INTR_CLR_REG = (RX_BUFFER_VALID);

    // Clear the RX buffer.
    sli_si91x_host_free_buffer(rx_pkt_buffer);
  }
#endif

  // Deinitialize the buffer manager
  status = sli_si91x_host_deinit_buffer_manager();
  VERIFY_STATUS_AND_RETURN(status);

  sl_si91x_host_disable_bus_interrupt();

  status = sl_si91x_host_power_cycle();
  VERIFY_STATUS_AND_RETURN(status);

  // Clear the event handler and reset initialization status
  si91x_event_handler  = NULL;
  device_initialized   = false;
  initialized_opermode = SLI_WIFI_INVALID_MODE;

  // Reset all the interfaces
  memset(interface_is_up, 0, sizeof(interface_is_up));

  return status;
}

sl_status_t sli_si91x_get_flash_efuse_data(sli_wifi_efuse_data_t *efuse_data, uint8_t efuse_data_type)
{
  sl_status_t status;
  sl_wifi_buffer_t *buffer = NULL;
  SL_WIFI_ARGS_CHECK_NULL_POINTER(efuse_data);

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_GET_EFUSE_DATA,
                                         SLI_WIFI_COMMON_CMD,
                                         &efuse_data_type,
                                         sizeof(efuse_data_type),
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_GET_EFUSE_DATA_WAIT_TIME),
                                         NULL,
                                         &buffer);
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);

  const sl_wifi_system_packet_t *packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  if (packet->length > 0) {
    switch (efuse_data_type) {
      case SL_SI91X_EFUSE_MFG_SW_VERSION:
        memcpy(&efuse_data->mfg_sw_version, packet->data, packet->length);
        break;
      case SL_SI91X_EFUSE_PTE_CRC:
        memcpy(&efuse_data->pte_crc, packet->data, packet->length);
        break;
      default:
        break;
    }
  }
  sli_si91x_host_free_buffer(buffer);
  return SL_STATUS_OK;
}

sl_status_t sl_si91x_driver_raw_send_command(uint8_t command,
                                             const void *data,
                                             uint32_t data_length,
                                             uint32_t wait_time)
{
  UNUSED_PARAMETER(wait_time);
  sl_wifi_buffer_t *buffer        = NULL;
  sl_wifi_system_packet_t *packet = NULL;
  sl_status_t status              = SL_STATUS_OK;

  // Allocate a data buffer with space for the data and metadata
  status = sl_si91x_allocate_data_buffer(&buffer,
                                         (void **)&packet,
                                         sizeof(sl_wifi_system_packet_t) + data_length,
                                         SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
  VERIFY_STATUS_AND_RETURN(status);

  // If the packet is not allocated successfully, return an allocation failed error
  if (packet == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  // Clear the packet descriptor and copy the command data if available
  memset(packet->desc, 0, sizeof(packet->desc));
  if (data != NULL) {
    memcpy(packet->data, data, data_length);
  }
  packet->length  = data_length & 0xFFF;
  packet->command = command;

  // Adding the packet to the queue with atomic action
  return sl_si91x_driver_send_data_packet(buffer, wait_time);
}

sl_status_t sli_si91x_driver_send_socket_data(const sli_si91x_socket_send_request_t *request,
                                              const void *data,
                                              uint32_t wait_time)
{
  UNUSED_PARAMETER(wait_time);
  sl_wifi_buffer_t *buffer        = NULL;
  sl_wifi_system_packet_t *packet = NULL;
  sli_si91x_socket_send_request_t *send;

  sl_status_t status     = SL_STATUS_OK;
  uint16_t header_length = (request->data_offset - sizeof(sli_si91x_socket_send_request_t));
  uint32_t data_length   = request->length;

  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // Allocate a buffer for the socket data with appropriate size
  status = sli_si91x_host_allocate_buffer(
    &buffer,
    SL_WIFI_TX_FRAME_BUFFER,
    sizeof(sl_wifi_system_packet_t) + sizeof(sli_si91x_socket_send_request_t) + header_length + data_length,
    SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);

  VERIFY_STATUS_AND_RETURN(status);
  packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);

  // If the packet is not allocated successfully, return an allocation failed error
  if (packet == NULL) {
    return SL_STATUS_WIFI_BUFFER_ALLOC_FAIL;
  }

  memset(packet->desc, 0, sizeof(packet->desc));

  send = (sli_si91x_socket_send_request_t *)packet->data;
  memcpy(send, request, sizeof(sli_si91x_socket_send_request_t));
  memcpy((send->send_buffer + header_length), data, data_length);

  // Fill frame type
  packet->length = (sizeof(sli_si91x_socket_send_request_t) + header_length + data_length) & 0xFFF;

  return sl_si91x_driver_send_data_packet(buffer, wait_time);
}

sl_status_t sl_si91x_custom_driver_send_command(uint32_t command,
                                                sli_wifi_command_type_t command_type,
                                                const void *data,
                                                uint32_t data_length,
                                                sli_wifi_wait_period_t wait_period,
                                                void *sdk_context,
                                                sl_wifi_buffer_t **data_buffer,
                                                uint8_t custom_host_desc)
{
  sl_wifi_buffer_t *buffer        = NULL;
  sl_wifi_system_packet_t *packet = NULL;
  sl_status_t status;

  // Check if the queue type is within valid range
  if (command_type >= SI91X_CMD_MAX) {
    return SL_STATUS_INVALID_INDEX;
  }

  // Allocate a buffer for the command with appropriate size
  status = sli_si91x_allocate_command_buffer(&buffer,
                                             (void **)&packet,
                                             sizeof(sl_wifi_system_packet_t) + data_length,
                                             SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
  VERIFY_STATUS_AND_RETURN(status);

  // Clear the packet descriptor and copy the command data if available
  memset(packet->desc, 0, sizeof(packet->desc));
  if (data != NULL) {
    memcpy(packet->data, data, data_length);
  }

  // Fill frame type
  packet->length  = data_length & 0xFFF;
  packet->command = (uint16_t)command;
  // Fill the packet identifier
  packet->unused[1] = custom_host_desc;
  return sli_si91x_driver_send_command_packet(command, command_type, buffer, wait_period, sdk_context, data_buffer);
}

sl_status_t sli_si91x_driver_send_command(uint32_t command,
                                          sli_wifi_command_type_t command_type,
                                          const void *data,
                                          uint32_t data_length,
                                          sli_wifi_wait_period_t wait_period,
                                          void *sdk_context,
                                          sl_wifi_buffer_t **data_buffer)
{
  sl_wifi_buffer_t *buffer        = NULL;
  sl_wifi_system_packet_t *packet = NULL;
  sl_status_t status;

  // Check if the queue type is within valid range
  if (command_type >= SI91X_CMD_MAX) {
    return SL_STATUS_INVALID_INDEX;
  }

  // Allocate a buffer for the command with appropriate size
  status = sli_si91x_allocate_command_buffer(&buffer,
                                             (void **)&packet,
                                             sizeof(sl_wifi_system_packet_t) + data_length,
                                             SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
  VERIFY_STATUS_AND_RETURN(status);

  // Clear the packet descriptor and copy the command data if available
  memset(packet->desc, 0, sizeof(packet->desc));
  if (data != NULL) {
    memcpy(packet->data, data, data_length);
  }

  // Set SLI_SI91X_FEAT_FW_UPDATE_NEW_CODE in the feature bit map to retrieve the latest firmware result codes
  if (command == SLI_WIFI_REQ_OPERMODE) {
    sl_wifi_system_boot_configuration_t *boot_configuration = (sl_wifi_system_boot_configuration_t *)packet->data;
    boot_configuration->feature_bit_map |= SLI_SI91X_FEAT_FW_UPDATE_NEW_CODE;
  }

  // Fill frame type
  packet->length  = data_length & 0xFFF;
  packet->command = (uint16_t)command;
  return sli_si91x_driver_send_command_packet(command, command_type, buffer, wait_period, sdk_context, data_buffer);
}
#ifdef SL_SI91X_SIDE_BAND_CRYPTO
sl_status_t sl_si91x_driver_send_side_band_crypto(uint32_t command,
                                                  const void *data,
                                                  uint32_t data_length,
                                                  sli_wifi_wait_period_t wait_period)
{
  sl_wifi_buffer_t *buffer        = NULL;
  sl_wifi_system_packet_t *packet = NULL;
  uint32_t result;
  sl_status_t status = SL_STATUS_OK;

  // Allocate a buffer for the command with appropriate size
  status =
    sli_si91x_allocate_command_buffer(&buffer, (void **)&packet, sizeof(sl_wifi_system_packet_t) + data_length, 1000);
  VERIFY_STATUS_AND_RETURN(status);

  // Clear the packet descriptor and copy the command data if available
  memset(packet->desc, 0, sizeof(packet->desc));
  if (data != NULL) {
    memcpy(packet->data, data, data_length);
  }

  // Fill frame type
  packet->length  = data_length & 0xFFF;
  packet->command = command;

  // Acquire Mutex
  osMutexAcquire(side_band_crypto_mutex, 0xFFFFFFFFUL);

  // fill crypto desc
  crypto_desc[0].addr   = (uint32_t)packet->desc;
  crypto_desc[0].length = SL_HOST_DESC_LEN;
  crypto_desc[1].addr   = (uint32_t)packet->data;
  crypto_desc[1].length = SL_CRYPTO_PKT_LEN;

  //! Enter Critical Section
  __disable_irq();

  sli_si91x_raise_side_band_interrupt_to_ta();

  //! Exit Critical Section
  __enable_irq();

  if (wait_period != osWaitForever) {
    wait_period = SLI_SYSTEM_MS_TO_TICKS(wait_period);
  }

  result =
    osEventFlagsWait(ta_events, SLI_SIDE_BAND_DONE, (osFlagsWaitAny), (wait_period & ~SLI_WIFI_WAIT_FOR_RESPONSE_BIT));
  if (result == (uint32_t)osErrorTimeout || result == (uint32_t)osErrorResource) {
    osMutexRelease(side_band_crypto_mutex);
    sli_si91x_host_free_buffer(buffer);
    return SL_STATUS_TIMEOUT;
  }

  status =
    (((uint8_t *)crypto_desc[0].addr)[12] + (((uint8_t *)crypto_desc[0].addr)[13] << 8)); // Extract the frame status

  // Release Mutex
  osMutexRelease(side_band_crypto_mutex);

  sli_si91x_host_free_buffer(buffer);
  return status;
}
#endif

sl_status_t sli_si91x_driver_send_bt_command(sli_wlan_cmd_request_t command,
                                             sli_wifi_command_type_t command_type,
                                             sl_wifi_buffer_t *data,
                                             uint8_t sync_command)
{
  sli_wifi_wait_period_t wait_period = SLI_WIFI_RETURN_IMMEDIATELY;

  // Check if the queue type is within valid range
  if (command_type >= SI91X_CMD_MAX) {

    return SL_STATUS_INVALID_INDEX;
  }

  if (sync_command) {
    return sli_si91x_driver_send_command_packet(command, command_type, data, wait_period, NULL, NULL);
  } else {
    return sli_si91x_driver_send_async_command(command, command_type, data, 0);
  }
}

sl_status_t sl_si91x_driver_wait_for_response(sli_wlan_cmd_request_t command, sli_wifi_wait_period_t wait_period)
{
  UNUSED_PARAMETER(command);
  UNUSED_PARAMETER(wait_period);
#ifdef SI91x_ENABLE_WAIT_ON_RESULTS
  // Wait for WLAN response events with a specified timeout
  uint32_t events =
    sli_si91x_wait_for_event(NCP_HOST_WLAN_RESPONSE_EVENT, (wait_period & ~SLI_WIFI_WAIT_FOR_RESPONSE_BIT));

  sli_si91x_clear_event(NCP_HOST_WLAN_RESPONSE_EVENT);

  //TODO: Change error handling from event based to response
  if (events & NCP_HOST_WLAN_RESPONSE_EVENT) {
    return sli_convert_and_save_firmware_status(si91x_frame_error_status);
  } else if (events == 0) {
    return SL_STATUS_TIMEOUT;
  }
#endif
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sli_si91x_driver_wait_for_response_packet(sli_wifi_buffer_queue_t *queue,
                                                      osEventFlagsId_t event_flag,
                                                      uint32_t event_mask,
                                                      uint16_t packet_id,
                                                      sli_wifi_wait_period_t wait_period,
                                                      sl_wifi_buffer_t **packet_buffer)
{
  SL_VERIFY_POINTER_OR_RETURN(packet_buffer, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(queue, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(event_flag, SL_STATUS_NULL_POINTER);

  uint32_t start_time      = osKernelGetTickCount();
  uint32_t elapsed_time    = 0;
  sl_wifi_buffer_t *buffer = NULL;

  if (wait_period != osWaitForever) {
    wait_period = SLI_SYSTEM_MS_TO_TICKS(wait_period);
  }

  while (1) {
    // Calculate the remaining timeout for the event wait
    uint32_t remaining_timeout = (wait_period > elapsed_time) ? (wait_period - elapsed_time) : 0;

    // Wait for the event flag to be set, with the specified timeout
    uint32_t events = osEventFlagsWait(event_flag, event_mask, (osFlagsWaitAny | osFlagsNoClear), remaining_timeout);

    // If the event wait times out or resources are unavailable, return timeout status
    if (events == (uint32_t)osErrorTimeout || events == (uint32_t)osErrorResource) {
      return SL_STATUS_TIMEOUT;
    }

    // Enter atomic section to safely access the queue
    CORE_irqState_t state = CORE_EnterAtomic();
    // Try to remove the buffer with the matching packet_id from the queue
    sl_status_t packet_status =
      sli_wifi_remove_buffer_from_queue_by_comparator(queue,
                                                      &packet_id,
                                                      sli_si91x_packet_identification_function,
                                                      &buffer);

    if (packet_status == SL_STATUS_OK) {
      // If the queue is now empty, clear the event flag
      if (0 == sli_si91x_host_queue_status(queue)) {
        osEventFlagsClear(event_flag, event_mask);
      }
      CORE_ExitAtomic(state);
      *packet_buffer = buffer;
      return SL_STATUS_OK;
    } else if (packet_status == SL_STATUS_EMPTY) {
      // If the queue is empty, clear the event flag to avoid spurious wakeups
      osEventFlagsClear(event_flag, event_mask);
    }
    CORE_ExitAtomic(state);
    if (packet_status == SL_STATUS_NOT_FOUND) {
      osDelay(SLI_SYSTEM_MS_TO_TICKS(2)); // Add a small delay to avoid busy waiting
    }

    // Update elapsed time for the next iteration
    elapsed_time = sl_si91x_host_elapsed_time(start_time);
  }
  // This code path should not be reached; loop exits on success or timeout.
  return SL_STATUS_FAIL;
}

sl_status_t sli_si91x_driver_send_command_packet(uint32_t command,
                                                 sli_wifi_command_type_t command_type,
                                                 sl_wifi_buffer_t *buffer,
                                                 sli_wifi_wait_period_t wait_period,
                                                 void *sdk_context,
                                                 sl_wifi_buffer_t **data_buffer)
{
  sli_si91x_queue_packet_t *node = NULL;
  sl_status_t status;
  sl_wifi_buffer_t *packet         = NULL;
  sl_wifi_buffer_t *response       = NULL;
  uint8_t flags                    = 0;
  static uint8_t command_packet_id = 0;

  // Allocate a command packet and set flags based on the command type
  status = sli_si91x_allocate_command_buffer(&packet,
                                             (void **)&node,
                                             sizeof(sli_si91x_queue_packet_t),
                                             SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
  if (status != SL_STATUS_OK) {
    sli_si91x_host_free_buffer(buffer);
    return status;
  }

  // Set flags using helper function
  flags = sli_set_command_packet_flags(command, wait_period, data_buffer);

  // Configure command packet node using SI91X-specific helper function
  sli_configure_si91x_command_packet_node(node, buffer, command_type, flags, sdk_context, wait_period);

  // Enqueue command packet using SI91X-specific helper function
  const uint8_t this_packet_id = command_packet_id;
  command_packet_id++;
  status = sli_enqueue_si91x_command_packet(command_type, packet, buffer, this_packet_id);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Handle command response using SI91X-specific helper function
  return sli_handle_si91x_command_response(command_type, wait_period, this_packet_id, data_buffer, &response);
}

static sl_status_t sl_si91x_driver_send_data_packet(sl_wifi_buffer_t *buffer, uint32_t wait_time)
{
  UNUSED_PARAMETER(wait_time);
  sli_wifi_append_to_buffer_queue(&sli_tx_data_queue, buffer);
  CORE_irqState_t state = CORE_EnterAtomic();
  tx_generic_socket_data_queues_status |= SL_SI91X_GENERIC_DATA_TX_PENDING_EVENT;
  sli_wifi_set_event(SL_SI91X_GENERIC_DATA_TX_PENDING_EVENT);
  CORE_ExitAtomic(state);

  return SL_STATUS_OK;
}

sl_status_t sli_si91x_driver_send_async_command(uint32_t command,
                                                sli_wifi_command_type_t command_type,
                                                void *data,
                                                uint32_t data_length)
{

  sli_si91x_queue_packet_t *node = NULL;
  sl_status_t return_status;
  sl_wifi_buffer_t *raw_rx_buffer        = NULL;
  sl_wifi_buffer_t *buffer               = NULL;
  sl_wifi_system_packet_t *raw_rx_packet = NULL;
  sl_status_t status;

  if (command_type == SLI_SI91X_BT_CMD) {
    // BLE packet is created in upper layer, no allocations required here.
    raw_rx_buffer = (sl_wifi_buffer_t *)data;
  } else {
    status = sli_si91x_allocate_command_buffer(&raw_rx_buffer,
                                               (void **)&raw_rx_packet,
                                               sizeof(sl_wifi_system_packet_t) + data_length,
                                               SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
    VERIFY_STATUS_AND_RETURN(status);

    memset(raw_rx_packet->desc, 0, sizeof(raw_rx_packet->desc));
    if (data != NULL) {
      memcpy(raw_rx_packet->data, data, data_length);
    }

    // Fill frame type
    raw_rx_packet->length  = data_length & 0xFFF;
    raw_rx_packet->command = (uint16_t)command;
  }

  return_status = sli_si91x_allocate_command_buffer(&buffer,
                                                    (void **)&node,
                                                    sizeof(sli_si91x_queue_packet_t),
                                                    SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);

  if (return_status != SL_STATUS_OK) {
    sli_si91x_host_free_buffer(raw_rx_buffer);
    return return_status;
  }

#ifdef RSI_CHIP_MFG_EN
  // WLAN soc frequency
  if (command == RSI_BOOTUP_PARAMS) {
    host_desc[14] = RSI_SOC_FREQ;
  }
#endif

  // Configure the node representing the command packet
  node->host_packet       = raw_rx_buffer;
  node->firmware_queue_id = firmware_queue_id[command_type];
  node->command_type      = command_type;
  node->sdk_context       = NULL;
  node->flags             = SLI_WIFI_PACKET_WITH_ASYNC_RESPONSE;

  CORE_irqState_t irqState = CORE_EnterAtomic();
  if (cmd_queues[command_type].is_queue_initialized == false) {
    sli_si91x_host_free_buffer(buffer);
    sli_si91x_host_free_buffer(raw_rx_buffer);
    CORE_ExitAtomic(irqState);
    return SL_STATUS_NOT_INITIALIZED;
  }
  buffer->id = 0; // Does not use packet ID as async packets do not have a matching response
  sli_wifi_append_to_buffer_queue(&cmd_queues[command_type].tx_queue, buffer);
  tx_command_queues_status |= SL_SI91X_TX_PENDING_FLAG(command_type);
  sli_wifi_set_event(SL_SI91X_TX_PENDING_FLAG(command_type));
  CORE_ExitAtomic(irqState);

  return SL_STATUS_OK;
}

/*
 * Verifies that the device has booted successfully.
 * @return SL_STATUS_OK if verification successful
 *         otherwise RSI_ERROR_WAITING_FOR_BOARD_READY, RSI_ERROR_BOOTUP_OPTIONS_NOT_SAVED, RSI_ERROR_BOOTUP_OPTIONS_CHECKSUM_FAIL
 */
sl_status_t sli_verify_device_boot(uint32_t *rom_version)
{
  sl_status_t status;
  uint8_t value[2] = { 0, 0 };

  // Read a memory register to check if it's valid
  status = sl_si91x_bus_read_memory(SLI_HOST_INTF_REG_OUT, 2, (const uint8_t *)&value[0]);
  SLI_VERIFY_STATUS(status);

  // Verify register read was valid
  if (value[1] != SLI_WIFI_REGISTER_VALID) {
    return SL_STATUS_WAITING_FOR_BOARD_READY;
  }

  // Verify register value
  if (value[0] == SLI_BOOTUP_OPTIONS_LAST_CONFIG_NOT_SAVED) {
    return SL_STATUS_BOOTUP_OPTIONS_NOT_SAVED;
  } else if (value[0] == SLI_BOOTUP_OPTIONS_CHECKSUM_FAIL) {
    return SL_STATUS_BOOTUP_OPTIONS_CHECKSUM_FAILURE;
  }
#if RSI_BOOTLOADER_VERSION_CHECK
  else if (value[0] == RSI_BOOTLOADER_VERSION) {
  } else {
    return SL_STATUS_BOOTLOADER_VERSION_MISMATCH;
  }
#endif

  // Extract ROM version info
  if (value[0] == SLI_BOOTLOADER_VERSION_1P0) {
    *rom_version = SLI_ROM_VERSION_1P0;
  } else if (value[0] == SLI_BOOTLOADER_VERSION_1P1) {
    *rom_version = SLI_ROM_VERSION_1P1;
  }

  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 * @brief Allocate a buffer for the Wi-Fi driver
 *
 * @param buffer
 * @param buffer_size is the size of the buffer to allocate
 * @param wait_duration_ms is the duration before returning SL_TIMEOUT
 * @return SL_STATUS_OK if the values are retrieved correctly,
 * SL_TIMEOUT if the buffer is not allocated in time, SL_ERROR otherwise
 *****************************************************************************/
sl_status_t sli_si91x_allocate_command_buffer(sl_wifi_buffer_t **host_buffer,
                                              void **buffer,
                                              uint32_t requested_buffer_size,
                                              uint32_t wait_duration_ms)
{
  // Allocate a buffer from the SI91x host for WLAN control messages
  sl_status_t status =
    sli_si91x_host_allocate_buffer(host_buffer, SL_WIFI_CONTROL_BUFFER, requested_buffer_size, wait_duration_ms);
  VERIFY_STATUS_AND_RETURN(status);

  uint16_t temp;
  // Get a pointer to the allocated buffer's data area
  *buffer = sli_wifi_host_get_buffer_data(*host_buffer, 0, &temp);
  return SL_STATUS_OK;
}

sl_status_t sl_si91x_allocate_data_buffer(sl_wifi_buffer_t **host_buffer,
                                          void **buffer,
                                          uint32_t data_size,
                                          uint32_t wait_duration_ms)
{
  // Allocate a buffer from the SI91x host for WLAN data transmission
  sl_status_t status = sli_si91x_host_allocate_buffer(
    host_buffer,
    SL_WIFI_TX_FRAME_BUFFER,
    sizeof(sl_wifi_system_packet_t) + sizeof(sli_si91x_socket_send_request_t) + data_size,
    wait_duration_ms);
  VERIFY_STATUS_AND_RETURN(status);

  uint16_t temp;
  // Get a pointer to the allocated buffer's data area
  *buffer = sli_wifi_host_get_buffer_data(*host_buffer, 0, &temp);
  return SL_STATUS_OK;
}

sl_status_t sli_wifi_select_option(const uint8_t configuration)
{
  uint16_t boot_command = 0;
  sl_status_t status    = 0;
  uint16_t read_value   = 0;

  // Write a boot command to initiate the option selection
  status = sl_si91x_bus_write_memory(SLI_HOST_INTF_REG_OUT, 2, (uint8_t *)&boot_command);
  VERIFY_STATUS_AND_RETURN(status);

  if ((configuration == BURN_NWP_FW) || (configuration == BURN_M4_FW)) {
    boot_command = SLI_HOST_INTERACT_REG_VALID_FW | configuration;
  } else {
    boot_command = SLI_HOST_INTERACT_REG_VALID | configuration;
  }

  if (configuration == BURN_M4_FW) {
    boot_command |= SLI_M4_FW_IMAGE_NUMBER;
  }

  // Write the configuration to the SI91x host for option selection
  status = sl_si91x_bus_write_memory(SLI_HOST_INTF_REG_IN, 2, (uint8_t *)&boot_command);
  VERIFY_STATUS_AND_RETURN(status);

  // Check for a specific response to ensure successful option selection
  if ((configuration != LOAD_NWP_FW) && (configuration != LOAD_DEFAULT_NWP_FW_ACTIVE_LOW)) {
    uint32_t timestamp = sl_si91x_host_get_timestamp();
    while (sl_si91x_host_elapsed_time(timestamp) < 300) {
      status = sl_si91x_bus_read_memory(SLI_HOST_INTF_REG_OUT, 2, (const uint8_t *)&read_value);
      VERIFY_STATUS_AND_RETURN(status);

      if ((configuration == BURN_NWP_FW) || (configuration == BURN_M4_FW)) {
        if (read_value == (SLI_HOST_INTERACT_REG_VALID | SLI_SEND_RPS_FILE)) {
          return SL_STATUS_OK;
        }
      } else if (read_value == (SLI_HOST_INTERACT_REG_VALID | configuration)) {
        return SL_STATUS_OK;
      }
    }
  } else {
    // Check up to 3 seconds for firmware load or upgrade status
    uint32_t timestamp               = sl_si91x_host_get_timestamp();
    uint16_t default_nwp_fw_selected = 0;
    while (sl_si91x_host_elapsed_time(timestamp) < 3000) {
      status = sl_si91x_bus_read_memory(SLI_HOST_INTF_REG_OUT, 2, (const uint8_t *)&read_value);
      if (status != SL_STATUS_OK)
        continue;

      if ((read_value & 0xF000) == (SLI_HOST_INTERACT_REG_VALID_FW & 0xF000)) {
        if ((read_value & 0xFF) == SLI_VALID_FIRMWARE_NOT_PRESENT) {
          if (default_nwp_fw_selected == 0) {
            boot_command = SLI_HOST_INTERACT_REG_VALID_FW | SLI_SELECT_DEFAULT_NWP_FW_IMAGE_NUMBER;
            status       = sl_si91x_bus_write_memory(SLI_HOST_INTF_REG_IN, 2, (uint8_t *)&boot_command);
            if (status != SL_STATUS_OK) {
              return status;
            }

            while (sl_si91x_host_elapsed_time(timestamp) < 2000) {
              status = sl_si91x_bus_read_memory(SLI_HOST_INTF_REG_OUT, 2, (const uint8_t *)&read_value);
              if (status != SL_STATUS_OK)
                continue;
              if (read_value == (SLI_HOST_INTERACT_REG_VALID | SLI_SELECT_DEFAULT_NWP_FW_IMAGE_NUMBER)) {
                break;
              }
            }

            boot_command = SLI_HOST_INTERACT_REG_VALID_FW | configuration;
            status       = sl_si91x_bus_write_memory(SLI_HOST_INTF_REG_IN, 2, (uint8_t *)&boot_command);
            if (status != SL_STATUS_OK) {
              return status;
            }
            default_nwp_fw_selected = 1;
            continue;
          } else {
            return SL_STATUS_VALID_FIRMWARE_NOT_PRESENT;
          }
        }
        if ((read_value & 0xFF) == SLI_INVALID_OPTION) {
          return SL_STATUS_INVALID_OPTION;
        }
        if ((read_value & 0xFF) == SLI_CHECKSUM_SUCCESS) {
          return status;
        }
      }
    }
  }
  return SL_STATUS_FW_LOAD_OR_UPGRADE_TIMEOUT;
}

sl_status_t sl_si91x_enable_radio(void)
{
  uint8_t data       = 1;
  sl_status_t status = sli_si91x_driver_send_command(SLI_WLAN_REQ_RADIO,
                                                     SLI_WIFI_WLAN_CMD,
                                                     &data,
                                                     1,
                                                     SLI_WLAN_RSP_RADIO_WAIT_TIME,
                                                     NULL,
                                                     NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_si91x_disable_radio(void)
{
  uint8_t data       = 0;
  sl_status_t status = sli_si91x_driver_send_command(SLI_WLAN_REQ_RADIO,
                                                     SLI_WIFI_WLAN_CMD,
                                                     &data,
                                                     1,
                                                     SLI_WLAN_RSP_RADIO_WAIT_TIME,
                                                     NULL,
                                                     NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_si91x_write_calibration_data(const si91x_calibration_data_t *data)
{
  sl_status_t status = sli_si91x_driver_send_command(SLI_WLAN_REQ_CALIB_WRITE,
                                                     SLI_WIFI_WLAN_CMD,
                                                     data,
                                                     sizeof(si91x_calibration_data_t),
                                                     SLI_WLAN_RSP_CALIB_WRITE_WAIT_TIME,
                                                     NULL,
                                                     NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_si91x_wifi_set_certificate_index(uint8_t certificate_type,
                                                uint8_t certificate_index,
                                                const uint8_t *buffer,
                                                uint32_t certificate_length)
{
  uint32_t rem_len                          = 0;
  uint16_t chunk_size                       = 0;
  uint16_t data_size                        = 0;
  uint8_t chunks_remaining                  = 0;
  uint32_t offset                           = 0;
  sl_status_t status                        = SL_STATUS_OK;
  sli_si91x_req_set_certificate_t chunk_ptr = { 0 };

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  // Get the certificate chunk size
  chunk_size = (SLI_SI91X_MAX_CERT_SEND_SIZE - sizeof(sli_si91x_cert_info_t));

  // Get certificate length
  rem_len = certificate_length;

  do {
    // Memset the PKT
    memset(&chunk_ptr, 0, sizeof(sli_si91x_req_set_certificate_t));

    // If certificate length is 0, erase certificate
    if (rem_len == 0) {
      // Mark Data Size
      data_size = 0;

      // More chunks to send
      chunks_remaining = 0;
    } else if (rem_len >= chunk_size) {
      // Mark Data Size
      data_size = chunk_size;

      // More chunks to send
      chunks_remaining = 1;
    } else {
      // Mark Data Size
      data_size = (uint16_t)rem_len;

      // More chunks to send
      chunks_remaining = 0;
    }

    if ((data_size > 0) && (NULL != buffer)) {
      // Copy the certificate chunk
      memcpy(chunk_ptr.certificate, buffer + offset, data_size);
    }

    // Move the offset by chunk size
    offset += data_size;

    // Subtract the rem_len by the chunk size
    rem_len -= data_size;

    //Set the total length of certificate
    memcpy(&chunk_ptr.cert_info.total_len, &certificate_length, sizeof(chunk_ptr.cert_info.total_len));

    // Set the certificate type
    chunk_ptr.cert_info.certificate_type = certificate_type;

    // Set the certificate index
    chunk_ptr.cert_info.certificate_inx = certificate_index;

    // More chunks to send
    chunk_ptr.cert_info.more_chunks = chunks_remaining;

    // Set the length of the certificate chunk
    chunk_ptr.cert_info.certificate_length = data_size;

    // Send the driver command
    status = sli_si91x_driver_send_command(SLI_WLAN_REQ_SET_CERTIFICATE,
                                           SLI_WIFI_WLAN_CMD,
                                           &chunk_ptr,
                                           (sizeof(sli_si91x_cert_info_t) + data_size),
                                           SLI_WLAN_RSP_SET_CERTIFICATE_WAIT_TIME,
                                           NULL,
                                           NULL);
    VERIFY_STATUS_AND_RETURN(status);
  } while (rem_len > 0);

  // Return status
  return status;
}

sl_status_t sl_si91x_set_rtc_timer(const sl_si91x_module_rtc_time_t *timer)
{
  sl_status_t status = SL_STATUS_OK;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if ((timer->tm_sec > 59) || (timer->tm_min > 59) || (timer->tm_hour > 23)
      || ((timer->tm_mday < 1) || (timer->tm_mday > 31)) || (timer->tm_mon > 11)
      || ((timer->tm_wday < 1) || (timer->tm_wday > 7))) {

    // Checking Invalid Parameters
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Send set RTC timer request
  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_SET_RTC_TIMER,
                                         SLI_WIFI_COMMON_CMD,
                                         timer,
                                         sizeof(sl_si91x_module_rtc_time_t),
                                         SLI_COMMON_RSP_SET_RTC_TIMER_WAIT_TIME,
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_si91x_get_rtc_timer(sl_si91x_module_rtc_time_t *response)
{
  sl_status_t status       = SL_STATUS_OK;
  sl_wifi_buffer_t *buffer = NULL;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  SL_WIFI_ARGS_CHECK_NULL_POINTER(response);

  // Send get RTC timer request
  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_GET_RTC_TIMER,
                                         SLI_WIFI_COMMON_CMD,
                                         NULL,
                                         0,
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_GET_RTC_TIMER_WAIT_TIME),
                                         NULL,
                                         &buffer);

  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);

  // Extract the RTC timer data from the response
  const sl_wifi_system_packet_t *packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  memcpy(response, packet->data, sizeof(sl_si91x_module_rtc_time_t));
  sli_si91x_host_free_buffer(buffer);
  return SL_STATUS_OK;
}

sl_status_t sl_si91x_set_device_region(sl_wifi_operation_mode_t operation_mode,
                                       sl_wifi_band_mode_t band,
                                       sl_wifi_region_code_t region_code)
{
  return sli_wifi_set_device_region(operation_mode, band, region_code);
}

#ifdef SLI_SI91X_MCU_INTERFACE

sl_status_t sl_si91x_command_to_write_common_flash(uint32_t write_address,
                                                   const uint8_t *write_data,
                                                   uint16_t write_data_length,
                                                   uint8_t flash_sector_erase_enable)
{
  // Check if write_data_length is non-zero
  if (write_data_length == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status                         = SL_STATUS_OK;
  sli_si91x_request_ta2m4_t ta_to_m4_request = { 0 };
  uint32_t send_size                         = 0;
  uint16_t remaining_length                  = write_data_length;

  // If flash_sector_erase_enable is 1, Send request to NWP in chunks of 4k
  if (flash_sector_erase_enable == 1) {
    while (remaining_length > 0) {
      // Calculate the chunk size, capped at 4k
      size_t chunkSize = (remaining_length < FLASH_SECTOR_SIZE) ? remaining_length : FLASH_SECTOR_SIZE;

      // Fill the request structure
      memset(&ta_to_m4_request, 0, sizeof(sli_si91x_request_ta2m4_t));
      ta_to_m4_request.sub_cmd                   = SL_SI91X_WRITE_TO_COMMON_FLASH;
      ta_to_m4_request.addr                      = write_address;
      ta_to_m4_request.input_buffer_length       = (uint16_t)chunkSize;
      ta_to_m4_request.flash_sector_erase_enable = flash_sector_erase_enable;

      send_size = sizeof(sli_si91x_request_ta2m4_t);

      status = sli_si91x_driver_send_command(SLI_COMMON_REQ_TA_M4_COMMANDS,
                                             SLI_WIFI_COMMON_CMD,
                                             &ta_to_m4_request,
                                             send_size,
                                             SLI_COMMON_RSP_TA_M4_COMMANDS_WAIT_TIME,
                                             NULL,
                                             NULL);
      VERIFY_STATUS_AND_RETURN(status);

      // Adjust write_address for the next chunk
      write_address += chunkSize;

      // Adjust remaining_length for the next chunk
      remaining_length -= chunkSize;
    }
  }

  else {
    // Check if write_data pointer is valid
    SL_VERIFY_POINTER_OR_RETURN(write_data, SL_STATUS_INVALID_PARAMETER);

    // Write in chunks of MAX_CHUNK_SIZE for flash_sector_erase_enable != 1
    while (write_data_length > 0) {
      size_t chunkSize = (write_data_length < MAX_CHUNK_SIZE) ? write_data_length : MAX_CHUNK_SIZE;

      // Fill the request structure
      memset(&ta_to_m4_request, 0, sizeof(sli_si91x_request_ta2m4_t));
      ta_to_m4_request.sub_cmd                   = SL_SI91X_WRITE_TO_COMMON_FLASH;
      ta_to_m4_request.addr                      = write_address;
      ta_to_m4_request.input_buffer_length       = (uint16_t)chunkSize;
      ta_to_m4_request.flash_sector_erase_enable = flash_sector_erase_enable;

      // Copy write_data into the request structure
      memcpy(&ta_to_m4_request.input_data, write_data, chunkSize);

      // Calculate the send size and send the command to write to common flash
      send_size = sizeof(sli_si91x_request_ta2m4_t) - MAX_CHUNK_SIZE + chunkSize;
      status    = sli_si91x_driver_send_command(SLI_COMMON_REQ_TA_M4_COMMANDS,
                                             SLI_WIFI_COMMON_CMD,
                                             &ta_to_m4_request,
                                             send_size,
                                             SLI_COMMON_RSP_TA_M4_COMMANDS_WAIT_TIME,
                                             NULL,
                                             NULL);
      VERIFY_STATUS_AND_RETURN(status);

      // Adjust pointers and counters
      write_address += chunkSize;
      write_data += chunkSize;
      write_data_length -= chunkSize;
    }
  }
  return status;
}

sl_status_t sl_si91x_command_to_read_common_flash(uint32_t read_address, size_t length, uint8_t *output_buffer)
{
  // Check if output_buffer pointer is valid
  SL_VERIFY_POINTER_OR_RETURN(output_buffer, SL_STATUS_INVALID_PARAMETER);

  // Check if length is non-zero
  if (length == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status                    = SL_STATUS_OK;
  sl_wifi_buffer_t *buffer              = NULL;
  const sl_wifi_system_packet_t *packet = NULL;

  while (length > 0) {
    size_t chunkSize = (length < MAX_CHUNK_SIZE) ? length : MAX_CHUNK_SIZE;

    sli_si91x_read_flash_request_t m4_to_ta_read_request = { 0 };
    memset(&m4_to_ta_read_request, 0, sizeof(sli_si91x_read_flash_request_t));
    m4_to_ta_read_request.sub_cmd              = SL_SI91X_READ_FROM_COMMON_FLASH;
    m4_to_ta_read_request.nwp_address          = read_address;
    m4_to_ta_read_request.output_buffer_length = (uint16_t)chunkSize;

    uint32_t send_size = sizeof(sli_si91x_read_flash_request_t);

    status = sli_si91x_driver_send_command(SLI_COMMON_REQ_TA_M4_COMMANDS,
                                           SLI_WIFI_COMMON_CMD,
                                           &m4_to_ta_read_request,
                                           send_size,
                                           SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_TA_M4_COMMANDS_WAIT_TIME),
                                           NULL,
                                           &buffer);
    if (status != SL_STATUS_OK) {
      if (buffer != NULL)
        sli_si91x_host_free_buffer(buffer);
      return status;
    }
    VERIFY_STATUS_AND_RETURN(status);

    packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
    memcpy(output_buffer, packet->data, packet->length);
    sli_si91x_host_free_buffer(buffer);

    // Adjust pointers and counters
    read_address += chunkSize;
    output_buffer += chunkSize;
    length -= chunkSize;
  }

  return status;
}

sl_status_t sl_si91x_m4_ta_secure_handshake(uint8_t sub_cmd_type,
                                            uint8_t input_len,
                                            const uint8_t *input_data,
                                            uint8_t output_len,
                                            const uint8_t *output_data)
{
  UNUSED_PARAMETER(output_len);
  UNUSED_PARAMETER(output_data);
  sli_si91x_ta_m4_handshake_parameters_t *handshake_request = NULL;
  sl_status_t status                                        = SL_STATUS_OK;

  SL_VERIFY_POINTER_OR_RETURN(input_data, SL_STATUS_INVALID_PARAMETER);

  handshake_request = malloc(sizeof(sli_si91x_ta_m4_handshake_parameters_t) + input_len);
  SL_VERIFY_POINTER_OR_RETURN(handshake_request, SL_STATUS_ALLOCATION_FAILED);
  memset(handshake_request, 0, sizeof(sli_si91x_ta_m4_handshake_parameters_t) + input_len);
  handshake_request->sub_cmd         = sub_cmd_type;
  handshake_request->input_data_size = input_len;
  memcpy(handshake_request->input_data, input_data, input_len);

  // Send the secure handshake command to the M4 core
  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_TA_M4_COMMANDS,
                                         SLI_WIFI_COMMON_CMD,
                                         handshake_request,
                                         sizeof(sli_si91x_ta_m4_handshake_parameters_t) + input_len,
                                         SLI_COMMON_RSP_TA_M4_COMMANDS_WAIT_TIME,
                                         NULL,
                                         NULL);
  free(handshake_request);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_si91x_configure_timestamp_memory_location(uint8_t addr_len, const uint32_t *address)
{
  sli_si91x_ta_m4_handshake_parameters_t *handshake_request = NULL;
  sl_status_t status                                        = SL_STATUS_OK;

  SL_VERIFY_POINTER_OR_RETURN(address, SL_STATUS_INVALID_PARAMETER);

  handshake_request = malloc(sizeof(sli_si91x_ta_m4_handshake_parameters_t) + addr_len);
  SL_VERIFY_POINTER_OR_RETURN(handshake_request, SL_STATUS_ALLOCATION_FAILED);
  memset(handshake_request, 0, sizeof(sli_si91x_ta_m4_handshake_parameters_t) + addr_len);
  handshake_request->sub_cmd         = SL_SI91X_SET_TIMESTAMP_MEMORY_ADDRESS;
  handshake_request->input_data_size = addr_len;
  memcpy(handshake_request->input_data, address, addr_len);

  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_TA_M4_COMMANDS,
                                         SLI_WIFI_COMMON_CMD,
                                         handshake_request,
                                         sizeof(sli_si91x_ta_m4_handshake_parameters_t) + addr_len,
                                         SLI_COMMON_RSP_TA_M4_COMMANDS_WAIT_TIME,
                                         NULL,
                                         NULL);
  free(handshake_request);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_si91x_read_status(sl_si91x_read_status_t read_id, uint8_t *output)
{
  sl_wifi_buffer_t *buffer              = NULL;
  const sl_wifi_system_packet_t *packet = NULL;
  sl_status_t status                    = SL_STATUS_OK;
  sli_si91x_read_status_t read_request  = { 0 };

  SL_VERIFY_POINTER_OR_RETURN(output, SL_STATUS_NULL_POINTER);

  if ((read_id < SL_SI91X_READ_NWP_DEBUG_PORT_STATUS) || (read_id > SL_SI91X_READ_MCU_DEBUG_PORT_STATUS)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  read_request.sub_cmd = (uint8_t)read_id;
  status               = sli_si91x_driver_send_command(SLI_COMMON_REQ_TA_M4_COMMANDS,
                                         SLI_WIFI_COMMON_CMD,
                                         &read_request,
                                         sizeof(sli_si91x_read_status_t),
                                         SLI_COMMON_RSP_TA_M4_COMMANDS_WAIT_TIME,
                                         NULL,
                                         &buffer);
  if (status != SL_STATUS_OK) {
    if (buffer != NULL)
      sli_si91x_host_free_buffer(buffer);
    return status;
  }
  VERIFY_STATUS_AND_RETURN(status);

  packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  if (packet->length > 0)
    memcpy(output, packet->data, packet->length);
  sli_si91x_host_free_buffer(buffer);

  return status;
}

// Perform a soft reset
static sl_status_t sl_si91x_soft_reset(void)
{
  sl_status_t status;
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_SOFT_RESET,
                                         SLI_WIFI_COMMON_CMD,
                                         NULL,
                                         0,
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_TA_M4_COMMANDS_WAIT_TIME),
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}
#endif

sl_status_t sl_si91x_assert()
{
  sl_status_t status = SL_STATUS_OK;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_ASSERT,
                                         SLI_WIFI_WLAN_CMD,
                                         NULL,
                                         0,
                                         SLI_COMMON_RSP_ASSERT_WAIT_TIME,
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_si91x_get_ram_log(uint32_t address, uint32_t length)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  sl_status_t status       = SL_STATUS_OK;
  sli_si91x_ram_dump_t ram = { .address = address, .length = length };

  // Check for invalid length parameter
  if (length == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Send RAM log request
  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_GET_RAM_DUMP,
                                         SLI_WIFI_COMMON_CMD,
                                         &ram,
                                         sizeof(sli_si91x_ram_dump_t),
                                         SLI_COMMON_RSP_GET_RAM_DUMP_WAIT_TIME,
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_si91x_transmit_test_start(const sl_wifi_request_tx_test_info_t *tx_test_info)
{
  sl_status_t status = SL_STATUS_OK;

  status = sli_si91x_driver_send_command(SLI_WIFI_REQ_TX_TEST_MODE,
                                         SLI_WIFI_WLAN_CMD,
                                         tx_test_info,
                                         sizeof(sl_wifi_request_tx_test_info_t),
                                         SLI_WLAN_RSP_TX_TEST_MODE_WAIT_TIME,
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_si91x_transmit_test_stop(void)
{
  sl_status_t status                          = SL_STATUS_OK;
  sl_wifi_request_tx_test_info_t tx_test_info = { 0 };
  tx_test_info.enable                         = 0;
  // Send the transmit test stop command
  status = sli_si91x_driver_send_command(SLI_WIFI_REQ_TX_TEST_MODE,
                                         SLI_WIFI_WLAN_CMD,
                                         &tx_test_info,
                                         sizeof(sl_wifi_request_tx_test_info_t),
                                         SLI_WLAN_RSP_TX_TEST_MODE_WAIT_TIME,
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_si91x_calibration_write(sl_si91x_calibration_write_t calib_write)
{
  sl_status_t status = SL_STATUS_OK;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_CALIB_WRITE,
                                         SLI_WIFI_WLAN_CMD,
                                         &calib_write,
                                         sizeof(sl_si91x_calibration_write_t),
                                         SLI_WLAN_RSP_CALIB_WRITE_WAIT_TIME,
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_si91x_calibration_read(sl_si91x_calibration_read_t target, sl_si91x_calibration_read_t *calibration_read)
{
  sl_wifi_buffer_t *buffer = NULL;
  sl_status_t status       = SL_STATUS_OK;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  SL_VERIFY_POINTER_OR_RETURN(calibration_read, SL_STATUS_NULL_POINTER);

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_CALIB_READ,
                                         SLI_WIFI_WLAN_CMD,
                                         &target,
                                         sizeof(sl_si91x_calibration_read_t),
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WLAN_RSP_CALIB_READ_WAIT_TIME),
                                         NULL,
                                         &buffer);

  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
    return status;
  }

  const sl_wifi_system_packet_t *packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  memcpy(calibration_read, packet->data, sizeof(sl_si91x_calibration_read_t));
  sli_si91x_host_free_buffer(buffer);
  return status;
}

sl_status_t sl_si91x_frequency_offset(const sl_si91x_freq_offset_t *frequency_calibration)
{
  sl_status_t status = SL_STATUS_OK;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  SL_VERIFY_POINTER_OR_RETURN(frequency_calibration, SL_STATUS_NULL_POINTER);

  // Send the frequency offset calibration command to the SI91x WLAN module
  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_FREQ_OFFSET,
                                         SLI_WIFI_WLAN_CMD,
                                         frequency_calibration,
                                         sizeof(sl_si91x_freq_offset_t),
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WLAN_RSP_FREQ_OFFSET_WAIT_TIME),
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_si91x_evm_offset(const sl_si91x_evm_offset_t *evm_offset)
{
  sl_status_t status = SL_STATUS_OK;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  SL_VERIFY_POINTER_OR_RETURN(evm_offset, SL_STATUS_NULL_POINTER);

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_EVM_OFFSET,
                                         SLI_WIFI_WLAN_CMD,
                                         evm_offset,
                                         sizeof(sl_si91x_evm_offset_t),
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_RSP_EVM_OFFSET_WAIT_TIME),
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t sl_si91x_evm_write(const sl_si91x_evm_write_t *evm_write)
{
  sl_status_t status = SL_STATUS_OK;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  SL_VERIFY_POINTER_OR_RETURN(evm_write, SL_STATUS_NULL_POINTER);

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_EVM_WRITE,
                                         SLI_WIFI_WLAN_CMD,
                                         evm_write,
                                         sizeof(sl_si91x_evm_write_t),
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WLAN_RSP_EVM_WRITE_WAIT_TIME),
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t sl_si91x_dpd_calibration(const sl_si91x_get_dpd_calib_data_t *dpd_calib_data)
{
  sl_status_t status = SL_STATUS_OK;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  SL_VERIFY_POINTER_OR_RETURN(dpd_calib_data, SL_STATUS_NULL_POINTER);

  status = sli_si91x_driver_send_command(SLI_WIFI_REQ_GET_DPD_DATA,
                                         SLI_WIFI_WLAN_CMD,
                                         dpd_calib_data,
                                         sizeof(sl_si91x_get_dpd_calib_data_t),
                                         SLI_WLAN_RSP_GET_DPD_DATA_WAIT_TIME,
                                         NULL,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_si91x_efuse_read(const sl_si91x_efuse_read_t *efuse_read, uint8_t *efuse_read_buf)
{
  sl_wifi_buffer_t *buffer = NULL;
  sl_status_t status       = SL_STATUS_OK;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  SL_VERIFY_POINTER_OR_RETURN(efuse_read, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(efuse_read_buf, SL_STATUS_NULL_POINTER);

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_EFUSE_READ,
                                         SLI_WIFI_WLAN_CMD,
                                         efuse_read,
                                         sizeof(sl_si91x_efuse_read_t),
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WLAN_RSP_EFUSE_READ_WAIT_TIME),
                                         NULL,
                                         &buffer);

  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
    return status;
  }

  const sl_wifi_system_packet_t *packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  memcpy(efuse_read_buf, packet->data, efuse_read->efuse_read_data_len);
  sli_si91x_host_free_buffer(buffer);
  return status;
}

sl_status_t sl_si91x_set_join_configuration(sl_wifi_interface_t interface, uint8_t join_feature_bitmap)
{
  return sli_wifi_set_join_configuration(interface, join_feature_bitmap);
}

sl_status_t sl_si91x_get_join_configuration(sl_wifi_interface_t interface, uint8_t *join_feature_bitmap)
{
  return sli_wifi_get_join_configuration(interface, join_feature_bitmap);
}

void sl_si91x_set_listen_interval(uint32_t listen_interval)
{
  sl_wifi_listen_interval_v2_t v2_params = {
    .listen_interval            = listen_interval,
    .listen_interval_multiplier = 1, /* Default multiplier; matches sli_wifi DEFAULT_LISTEN_INTERVAL_MULTIPLIER */
  };
  sli_wifi_set_listen_interval_v2(SL_WIFI_CLIENT_INTERFACE, v2_params);
}

void sl_si91x_set_timeout(const sl_wifi_timeout_t *timeout_config)
{
  memcpy(&timeout_glbl, timeout_config, sizeof(sl_wifi_timeout_t));
  return;
}

sl_status_t sl_si91x_configure_timeout(sl_si91x_timeout_type_t timeout_type, uint16_t timeout_value)
{
  sl_status_t status                          = SL_STATUS_OK;
  sli_si91x_request_timeout_t timeout_request = { 0 };

  if (timeout_type > SL_SI91X_CHANNEL_PASSIVE_SCAN_TIMEOUT) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  timeout_request.timeout_bitmap = BIT(timeout_type);
  timeout_request.timeout_value  = timeout_value;
  status                         = sli_si91x_driver_send_command(SLI_WLAN_REQ_TIMEOUT,
                                         SLI_WIFI_WLAN_CMD,
                                         &timeout_request,
                                         sizeof(sli_si91x_request_timeout_t),
                                         SLI_WIFI_WAIT_FOR(30100),
                                         NULL,
                                         NULL);
  return status;
}

uint16_t sli_get_seq_ctrl(uint8_t is_qos)
{
  static uint16_t qos_pkt_count     = 0;
  static uint16_t non_qos_pkt_count = 0;

  if (qos_pkt_count > 4095)
    qos_pkt_count = 0;

  if (non_qos_pkt_count > 4095)
    non_qos_pkt_count = 0;

  return is_qos ? qos_pkt_count++ : non_qos_pkt_count++;
}

int32_t sli_encapsulate_tx_data_packet(sl_wifi_transceiver_tx_data_control_t *control,
                                       uint8_t *pkt_data,
                                       uint32_t mac_hdr_len)
{
  uint16_t seq_ctrl = 0;
  uint16_t *frame_ctrl;
  uint32_t qos_ctrl_off = MAC80211_HDR_MIN_LEN;

  SL_VERIFY_POINTER_OR_RETURN(control, SL_STATUS_NULL_POINTER);

  if (IS_MAC_ZERO(control->addr1)) {
    return SL_STATUS_TRANSCEIVER_INVALID_MAC_ADDRESS;
  }

  /* Auto-rate is unsupported if Peer DS feature in MAC layer is disabled */
  if ((!IS_PEER_DS_SUPPORT_ENABLED(feature_bit_map)) && !IS_FIXED_DATA_RATE(control->ctrl_flags)) {
    return SL_STATUS_TRANSCEIVER_INVALID_DATA_RATE;
  }

  /* Ignore QoS flag for bcast/mcast frames */
  if (IS_BCAST_MCAST_MAC(control->addr1[0])) {
    control->ctrl_flags &= ~TX_DATA_CTRL_FLAG_QOS_BIT;
  }

  if ((IS_QOS_PKT(control->ctrl_flags) && !IS_BCAST_MCAST_MAC(control->addr1[0])) && (control->priority > 3)) {
    return SL_STATUS_TRANSCEIVER_INVALID_QOS_PRIORITY;
  }

  if (IS_4ADDR(control->ctrl_flags)) {
    qos_ctrl_off += MAC80211_HDR_ADDR4_LEN;
  }

  memset(pkt_data, 0, mac_hdr_len);
  /* Add frame control (2 bytes) */
  frame_ctrl = (uint16_t *)&pkt_data[0];
  *frame_ctrl |= FC_TYPE_DATA;

  if (IS_4ADDR(control->ctrl_flags)) {
    *frame_ctrl |= FC_TO_DS;
    *frame_ctrl |= FC_FROM_DS;
  } else {
    *frame_ctrl |= IS_TODS(control->ctrl_flags) ? FC_TO_DS : 0;
    *frame_ctrl |= IS_FROMDS(control->ctrl_flags) ? FC_FROM_DS : 0;
  }

  /* Add Addr1, Addr2, Addr3 (18 bytes) */
  memcpy(&pkt_data[4], control->addr1, 6);
  memcpy(&pkt_data[10], control->addr2, 6);
  memcpy(&pkt_data[16], control->addr3, 6);

  if (!IS_PEER_DS_SUPPORT_ENABLED(feature_bit_map)) {
    seq_ctrl = (uint16_t)(sli_get_seq_ctrl(IS_QOS_PKT(control->ctrl_flags)) << 4);
    memcpy(&pkt_data[22], &seq_ctrl, 2);
  }

  /* Add Addr4 optionally based on ctrl_flag (6 bytes) */
  if (IS_4ADDR(control->ctrl_flags)) {
    memcpy(&pkt_data[24], control->addr4, 6); /* sa */
  }
  /* Add QoS control optionally based on ctrl_flag (2 bytes) */
  if (IS_QOS_PKT(control->ctrl_flags) && !IS_BCAST_MCAST_MAC(control->addr1[0])) {
    *frame_ctrl |= FC_SUBTYPE_QOS_DATA;
    pkt_data[qos_ctrl_off] = WME_AC_TO_TID(control->priority);
  }

  return SL_STATUS_OK;
}

sl_status_t sl_si91x_driver_send_transceiver_data(sl_wifi_transceiver_tx_data_control_t *control,
                                                  const uint8_t *payload,
                                                  uint16_t payload_len,
                                                  uint32_t wait_time)
{
  sl_wifi_buffer_t *buffer        = NULL;
  sl_wifi_system_packet_t *packet = NULL;
  sl_status_t status              = SL_STATUS_OK;
  uint8_t *pkt_offset;
  uint8_t ext_desc_size;
  uint8_t *host_desc;
  uint32_t mac_hdr_len = MAC80211_HDR_MIN_LEN;

  if (IS_QOS_PKT(control->ctrl_flags) && !IS_BCAST_MCAST_MAC(control->addr1[0])) {
    mac_hdr_len += MAC80211_HDR_QOS_CTRL_LEN;
  }

  if (IS_4ADDR(control->ctrl_flags)) {
    mac_hdr_len += MAC80211_HDR_ADDR4_LEN;
  }

  // Initialize ext_desc_size with the base size for transceiver TX data
  ext_desc_size = TRANSCEIVER_TX_DATA_EXT_DESC_SIZE;

  // Check if the control flags indicate an EIA packet
  // Note: Bits 6 and 7 of ctrl_flags, bit 0 of ctrl_flags1, and the channel and tx_power fields are currently not supported.
  if (SLI_IS_EIA_PKT(control->ctrl_flags)) {
    // If it is an EIA packet, add 5 bytes for the following fields: channel, tx_power, is_last_packet, reserved1, reserved2
    ext_desc_size += SLI_EXT_DESC_SIZE_IF_EIA_PKT;
  }

  // Allocate a data buffer with space for the data and metadata
  status = sl_si91x_allocate_data_buffer(&buffer,
                                         (void **)&packet,
                                         sizeof(sl_wifi_system_packet_t) + ext_desc_size + mac_hdr_len + payload_len,
                                         SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
  VERIFY_STATUS_AND_RETURN(status);

  // If the packet is not allocated successfully, return an allocation failed error
  if (packet == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  pkt_offset = packet->data + ext_desc_size;
  status     = sli_encapsulate_tx_data_packet(control, pkt_offset, mac_hdr_len);
  if (status != SL_STATUS_OK) {
    sli_si91x_host_free_buffer(buffer);
    return status;
  }

  memcpy(pkt_offset + mac_hdr_len, payload, payload_len);

#ifdef TX_RX_FRAME_DUMP_BYTE_COUNT
  print_80211_packet(pkt_offset, mac_hdr_len + payload_len, TX_RX_FRAME_DUMP_BYTE_COUNT);
#endif

  // Clear the packet descriptor and copy the command data if available
  memset(packet->desc, 0, sizeof(packet->desc));

  // Fill length in first 2 host_desc bytes
  packet->length = (ext_desc_size + mac_hdr_len + payload_len) & 0xFFF;

  // Fill packet type
  host_desc = packet->desc;

  // Clear the extended descriptor if EIA is set
  if (SLI_IS_EIA_PKT(control->ctrl_flags)) {
    memset(packet->data, 0, ext_desc_size); //! Clear ext_desc_size bytes starting from packet->data
  }

  host_desc[2] = 0x01; //! Frame Type
  if (IS_CFM_TO_HOST_SET(control->ctrl_flags)) {
    host_desc[3] |= CONFIRM_REQUIRED_TO_HOST; //! This bit is used to set CONFIRM_REQUIRED_TO_HOST in firmware.
  }
  host_desc[4] = ext_desc_size;                     //! xtend_desc size
  host_desc[5] = (uint8_t)((mac_hdr_len + 3) & ~3); //! Mac_header length

  if (IS_BCAST_MCAST_MAC(control->addr1[0])) {
    host_desc[7] |= BCAST_INDICATION; //! Bcast_indication
    //! If auto-rate is enabled for bcast/mcast pkts, use 1 Mbps
    if (!IS_FIXED_DATA_RATE(control->ctrl_flags)) {
      host_desc[6] |= MAC_INFO_ENABLE; //! Fixed Rate
      host_desc[8] = SL_WIFI_DATA_RATE_1;
    }
  }

  if (IS_FIXED_DATA_RATE(control->ctrl_flags)) {
    host_desc[6] |= MAC_INFO_ENABLE; //! Fixed Rate
    host_desc[8] = (uint8_t)control->rate;
  }

  if (IS_QOS_PKT(control->ctrl_flags) && !IS_BCAST_MCAST_MAC(control->addr1[0])) {
    host_desc[13] |= QOS_ENABLE; // QOS ENABLE
  }

  host_desc[14] =
    (uint8_t)(((WME_AC_TO_TID(control->priority) & 0xf) << 4) | (WME_AC_TO_QNUM(control->priority) & 0xf));

  //! Initialize extended desc
  memcpy(&host_desc[16], &control->token, TRANSCEIVER_TX_DATA_EXT_DESC_SIZE);

  // Note: Bits 6 and 7 of ctrl_flags, bit 0 of ctrl_flags1, and the channel and tx_power fields are currently not supported.
  //! If it is an EIA packet, update extended descriptor fields
  if (SLI_IS_EIA_PKT(control->ctrl_flags)) {
    host_desc[7] |= SLI_EIA_BIT_IN_HOST_DESC; //! EIA Enable
    if (SLI_IS_IMMEDIATE_TRF(control->ctrl_flags)) {
      host_desc[7] |= SLI_IMMEDIATE_TRF_HOST_DESC; //! Immediate Transfer
    }
    host_desc[20] = control->channel;  //! Channel
    host_desc[21] = control->tx_power; //! Transmission Power
    if (SLI_IS_LAST_PKT(control->ctrl_flags1)) {
      host_desc[22] |= SLI_LAST_PKT; //! Last Packet
    }
  }

  // Send command packet to the SI91x socket data queue and await a response
  return sl_si91x_driver_send_data_packet(buffer, wait_time);
}

sl_status_t sl_si91x_get_firmware_version(sl_si91x_firmware_version_t *version)
{
  sl_status_t status       = SL_STATUS_OK;
  sl_wifi_buffer_t *buffer = NULL;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  SL_WIFI_ARGS_CHECK_NULL_POINTER(version);

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_FULL_FW_VERSION,
                                         SLI_WIFI_WLAN_CMD,
                                         NULL,
                                         0,
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WLAN_RSP_FULL_FW_VERSION_WAIT_TIME),
                                         NULL,
                                         &buffer);
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);

  const sl_wifi_system_packet_t *packet =
    (const sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);

  if (packet == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (packet->length > 0) {
    const sl_wifi_firmware_version_t *response = (const sl_wifi_firmware_version_t *)packet->data;

    if (response == NULL) {
      return SL_STATUS_NULL_POINTER;
    }

    version->chip_id          = response->chip_id;
    version->rom_id           = response->rom_id;
    version->major            = response->major;
    version->minor            = response->minor;
    version->security_version = response->security_version;
    version->patch_num        = response->patch_num;
    version->customer_id      = response->customer_id;
    version->build_num        = (packet->data[7] | packet->data[8]);
  }

  sli_si91x_host_free_buffer(buffer);
  return status;
}

sl_status_t sl_si91x_get_firmware_size(const void *buffer, uint32_t *fw_image_size)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(buffer);
  const sl_wifi_firmware_header_t *firmware_header = (const sl_wifi_firmware_header_t *)buffer;

#ifdef SLI_SI91X_MCU_INTERFACE
  // Check for SLI_COMBINED_IMAGE flag in control_flags
  if (firmware_header->control_flags & SLI_COMBINED_IMAGE) {
    // If SLI_COMBINED_IMAGE is set, read the combined image size from firmware_header[48-51]
    *fw_image_size = *(const uint32_t *)((const uint8_t *)firmware_header + SLI_COMBINED_IMAGE_SIZE_OFFSET);
  } else {
    // If SLI_COMBINED_IMAGE is not set, calculate the single image size
    *fw_image_size = firmware_header->image_size + sizeof(sl_wifi_firmware_header_t);
  }
#else
  *fw_image_size = firmware_header->image_size + sizeof(sl_wifi_firmware_header_t);
#endif

  return SL_STATUS_OK;
}

sl_status_t sl_si91x_set_nwp_config_request(sl_si91x_nwp_configuration_t nwp_config)
{
  sl_status_t status = SL_STATUS_OK;
  if ((nwp_config.code & SL_SI91X_XO_CTUNE_FROM_HOST) || (nwp_config.code & SL_SI91X_ENABLE_NWP_WDT_FROM_HOST)
      || (nwp_config.code & SL_SI91X_DISABLE_NWP_WDT_FROM_HOST)) {
    status = sli_si91x_driver_send_command(SLI_COMMON_REQ_SET_CONFIG,
                                           SLI_WIFI_COMMON_CMD,
                                           &nwp_config,
                                           sizeof(sl_si91x_nwp_configuration_t),
                                           SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_SET_CONFIG_WAIT_TIME),
                                           NULL,
                                           NULL);
    return status;
  }

  if (nwp_config.code & SL_SI91X_SET_XTAL_GOOD_TIME_FROM_HOST) {
    // Valid range 600 - 5000 microseconds
    if ((nwp_config.values.config_val < SLI_SI91X_XTAL_GOOD_TIME_MIN)
        || (nwp_config.values.config_val > SLI_SI91X_XTAL_GOOD_TIME_MAX)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    sli_xtal_good_time_us = nwp_config.values.config_val;
    return SL_STATUS_OK;
  }

  if (nwp_config.code & SL_SI91X_SET_PMU_GOOD_TIME_FROM_HOST) {
    if ((nwp_config.values.config_val < SLI_SI91X_PMU_GOOD_TIME_MIN)
        || (nwp_config.values.config_val > SLI_SI91X_PMU_GOOD_TIME_MAX)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    sli_pmu_good_time_us = nwp_config.values.config_val;
    return SL_STATUS_OK;
  }

  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_si91x_get_nwp_config(const sl_si91x_nwp_get_configuration_t *nwp_config, uint8_t *response)
{
  sl_status_t status                    = SL_STATUS_OK;
  sl_wifi_buffer_t *buffer              = NULL;
  const sl_wifi_system_packet_t *packet = NULL;

  if (nwp_config->sub_command_type == GET_OPN_BOARD_CONFIG) {
    status = sli_si91x_driver_send_command(SLI_COMMON_REQ_GET_CONFIG,
                                           SLI_WIFI_COMMON_CMD,
                                           nwp_config,
                                           sizeof(sl_si91x_nwp_get_configuration_t),
                                           SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_GET_CONFIG_WAIT_TIME),
                                           NULL,
                                           &buffer);
    if ((status != SL_STATUS_OK) && (buffer != NULL)) {
      sli_si91x_host_free_buffer(buffer);
    }
    VERIFY_STATUS_AND_RETURN(status);

    packet = (const sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);
    if (packet == NULL) {
      sli_si91x_host_free_buffer(buffer);
      return SL_STATUS_NULL_POINTER;
    }

    memcpy(response, packet->data, packet->length);
    sli_si91x_host_free_buffer(buffer);
  } else {
    return SL_STATUS_NOT_SUPPORTED;
  }
  return status;
}

sl_status_t sl_si91x_debug_log(const sl_si91x_assertion_t *assertion)
{
  sl_status_t status                 = SL_STATUS_OK;
  sli_si91x_debug_log_t debug_config = { 0 };

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if ((debug_config.assertion_type > SL_SI91X_ASSERTION_TYPE_ALL)
      || (debug_config.assertion_level > SL_SI91X_ASSERTION_LEVEL_MAX)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  debug_config.assertion_type  = assertion->assert_type;
  debug_config.assertion_level = assertion->assert_level;

  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_DEBUG_LOG,
                                         SLI_WIFI_COMMON_CMD,
                                         &debug_config,
                                         sizeof(sli_si91x_debug_log_t),
                                         SLI_COMMON_RSP_DEBUG_LOG_WAIT_TIME,
                                         NULL,
                                         NULL);

  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

void print_firmware_version(const sl_wifi_firmware_version_t *firmware_version)
{
  printf("\r\nFirmware version is: %x%x.%d.%d.%d.%d.%d.%d\r\n",
         firmware_version->chip_id,
         firmware_version->rom_id,
         firmware_version->major,
         firmware_version->minor,
         firmware_version->security_version,
         firmware_version->patch_num,
         firmware_version->customer_id,
         firmware_version->build_num);
}

sl_status_t sli_get_nwp_timestamp(uint32_t *timestamp)
{

  if (timestamp == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sl_status_t status                          = SL_STATUS_OK;
  sl_wifi_buffer_t *buffer                    = NULL;
  const sl_wifi_system_packet_t *packet       = NULL;
  sl_si91x_nwp_get_configuration_t nwp_config = { .sub_command_type = GET_NWP_TIMESTAMP };

  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_GET_CONFIG,
                                         SLI_WIFI_COMMON_CMD,
                                         &nwp_config,
                                         sizeof(sl_si91x_nwp_get_configuration_t),
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_GET_CONFIG_WAIT_TIME),
                                         NULL,
                                         &buffer);

  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);

  packet = (const sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);

  if (packet == NULL) {
    sli_si91x_host_free_buffer(buffer);
    return SL_STATUS_NULL_POINTER;
  }

  memcpy(timestamp, packet->data, packet->length);
  sli_si91x_host_free_buffer(buffer);

  return status;
}

sl_status_t sli_wifi_send_data_packet(void *data, uint16_t length, void *context)
{
  UNUSED_PARAMETER(data);
  UNUSED_PARAMETER(length);
  UNUSED_PARAMETER(context);
  return SL_STATUS_NOT_SUPPORTED;
}