/***************************************************************************/ /**
 * @file    sli_wifi_utility.c
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
#include "sli_wifi.h"
#include "sli_wifi_utility.h"
#include "sl_wifi_device.h"
#include "sli_wifi_constants.h"
#include "sli_wifi_types.h"
#include "sl_core.h"
#include "sl_wifi_credentials.h"
#include "assert.h"
#include "sli_wifi_memory_manager.h"
#include <string.h>
#ifndef __ZEPHYR__
#include "sli_cmsis_os2_ext_task_register.h"
/// External variable representing the index of the thread local array at which the firmware status will be stored.
extern sli_task_register_id_t sli_fw_status_storage_index;
#endif
#define DEFAULT_BEACON_MISS_IGNORE_LIMIT   1
#define DEFAULT_LISTEN_INTERVAL_MULTIPLIER 1
static uint32_t client_listen_interval            = 1000;
static uint32_t client_listen_interval_multiplier = 1;

// This value will be used in connect request/ AP configurations to set the TX power of the module
static sl_wifi_max_tx_power_t wifi_max_tx_power = {
  .scan_tx_power = 0x1f, //Default power value set to max value supported in dBm
  .join_tx_power = 0x1f, //Default power value set to max value supported in dBm
};

// This value will be used in set RTS threshold command to set the RTS threshold of the module
static sl_wifi_rts_threshold_t wifi_rts_threshold = {
  .rts_threshold = SLI_WIFI_RTS_THRESHOLD // Default RTS threshold value
};

// This value will be used in set MFP mode command to set the MFP mode of the module
static sl_wifi_mfp_config_t wifi_mfp_config = { .mfp_mode      = SL_WIFI_MFP_DISABLED,
                                                .is_configured = false,
                                                .reserved      = { 0 } };
// Global configuration storage with default values
static sli_wifi_feature_frame_config_t global_feature_config = {
  .pll_mode    = SL_WIFI_PLL_MODE_20MHZ, // Default: PLL mode for 20MHz operations
  .power_chain = SL_WIFI_HP_CHAIN,       // Default: High Power chain
};
extern bool device_initialized;
uint16_t initialized_opermode              = SLI_WIFI_INVALID_MODE;
static bool is_card_ready_required         = true;
static sl_wifi_rate_t saved_wifi_data_rate = SL_WIFI_AUTO_RATE;
static sl_wifi_ap_configuration_t ap_configuration;
sli_wifi_performance_profile_t performance_profile;
static sli_scan_info_t *scan_info_database = NULL;

extern osEventFlagsId_t sli_wifi_events;

void sli_wifi_set_opermode(sl_wifi_operation_mode_t mode)
{
  initialized_opermode = mode;
}

sl_wifi_operation_mode_t sli_wifi_get_opermode(void)
{
  return initialized_opermode;
}

sl_status_t sli_wifi_pop_from_buffer_queue(sli_wifi_buffer_queue_t *queue, sl_wifi_buffer_t **buffer)
{
  sl_status_t status    = SL_STATUS_EMPTY;
  CORE_irqState_t state = CORE_EnterAtomic();
  if (queue->head == NULL) {
    assert(queue->tail == NULL); // Both should be NULL at the same time
    *buffer = NULL;
    status  = SL_STATUS_EMPTY;
  } else {
    *buffer = queue->head;
    status  = SL_STATUS_OK;
    if (queue->head == queue->tail) {
      queue->head = NULL;
      queue->tail = NULL;
    } else {
      queue->head = (sl_wifi_buffer_t *)queue->head->node.node;
    }
  }
  CORE_ExitAtomic(state);
  return status;
}

sl_status_t sli_wifi_set_listen_interval(sl_wifi_interface_t interface, sl_wifi_listen_interval_t listen_interval)
{
  UNUSED_PARAMETER(interface);
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }
  client_listen_interval            = listen_interval.listen_interval;
  client_listen_interval_multiplier = DEFAULT_LISTEN_INTERVAL_MULTIPLIER;
  return SL_STATUS_OK;
}

sl_status_t sli_wifi_set_listen_interval_v2(sl_wifi_interface_t interface, sl_wifi_listen_interval_v2_t listen_interval)
{
  UNUSED_PARAMETER(interface);

  if (listen_interval.listen_interval_multiplier < DEFAULT_LISTEN_INTERVAL_MULTIPLIER) {
    SL_DEBUG_LOG("\r\n listen_interval_multiplier minimum value should be 1, Updating to the minimum value.\r\n");
    listen_interval.listen_interval_multiplier = DEFAULT_LISTEN_INTERVAL_MULTIPLIER;
  }
  client_listen_interval            = listen_interval.listen_interval;
  client_listen_interval_multiplier = listen_interval.listen_interval_multiplier;

  return SL_STATUS_OK;
}
sl_status_t sli_wifi_get_listen_interval(sl_wifi_interface_t interface, sl_wifi_listen_interval_t *listen_interval)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  SL_WIFI_ARGS_CHECK_NULL_POINTER(listen_interval);
  listen_interval->listen_interval = client_listen_interval;
  return SL_STATUS_OK;
}

sl_status_t sli_wifi_get_listen_interval_v2(sl_wifi_interface_t interface,
                                            sl_wifi_listen_interval_v2_t *listen_interval)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  SL_WIFI_ARGS_CHECK_NULL_POINTER(listen_interval);
  listen_interval->listen_interval            = client_listen_interval;
  listen_interval->listen_interval_multiplier = client_listen_interval_multiplier;
  return SL_STATUS_OK;
}

// Function to identify expected scan result based on filter
static bool sli_filter_scan_info(const sli_scan_info_t *scan_info,
                                 const sl_wifi_extended_scan_result_parameters_t *extended_scan_parameters)
{
  if (NULL == scan_info) {
    return false;
  }

  if ((NULL != extended_scan_parameters->channel_filter)
      && (*(extended_scan_parameters->channel_filter) != scan_info->channel)) {
    return false;
  }

  if ((NULL != extended_scan_parameters->security_mode_filter)
      && (*(extended_scan_parameters->security_mode_filter) != scan_info->security_mode)) {
    return false;
  }

  if ((NULL != extended_scan_parameters->rssi_filter)
      && (*(extended_scan_parameters->rssi_filter) <= scan_info->rssi)) {
    return false;
  }

  if ((NULL != extended_scan_parameters->network_type_filter)
      && (*(extended_scan_parameters->network_type_filter) != scan_info->network_type)) {
    return false;
  }

  return true;
}

// Function to get the total count of stored extended scan results (for callback data_length)
sl_status_t sli_wifi_get_stored_scan_result_count(sl_wifi_interface_t interface, uint16_t *scan_count)
{
  UNUSED_PARAMETER(interface);

  if (NULL == scan_count) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  *scan_count = 0;
  for (sli_scan_info_t *scan_info = scan_info_database; scan_info != NULL; scan_info = scan_info->next) {
    (*scan_count)++;
  }
  return SL_STATUS_OK;
}

// Function to get all or filtered scan results from scan result database
sl_status_t sli_wifi_get_stored_scan_results(sl_wifi_interface_t interface,
                                             sl_wifi_extended_scan_result_parameters_t *extended_scan_parameters) //Done
{
  UNUSED_PARAMETER(interface);
  if (NULL == extended_scan_parameters) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_wifi_extended_scan_result_t *scan_results = extended_scan_parameters->scan_results;
  uint16_t *result_count                       = extended_scan_parameters->result_count;
  uint16_t length                              = extended_scan_parameters->array_length;
  sli_scan_info_t *scan_info                   = scan_info_database;

  if ((NULL == scan_results) || (NULL == result_count) || (0 == length)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  *result_count = 0;

  while ((0 != length) && (NULL != scan_info)) {
    if (true == sli_filter_scan_info(scan_info, extended_scan_parameters)) {
      scan_results[*result_count].rf_channel    = scan_info->channel;
      scan_results[*result_count].security_mode = scan_info->security_mode;
      scan_results[*result_count].rssi          = scan_info->rssi;
      scan_results[*result_count].network_type  = scan_info->network_type;
      memcpy(scan_results[*result_count].bssid, scan_info->bssid, SLI_WIFI_HARDWARE_ADDRESS_LENGTH);
      memcpy(scan_results[*result_count].ssid, scan_info->ssid, 34);
      (*result_count)++;
      length--;
    }
    scan_info = scan_info->next;
  }

  return SL_STATUS_OK;
}

// Function to Clean up all the scan results in scan result database
void sli_wifi_flush_scan_results_database(void)
{
  sli_scan_info_t *scan_info = scan_info_database;
  sli_scan_info_t *node      = NULL;

  while (NULL != scan_info) {
    node      = scan_info;
    scan_info = scan_info->next;
    free(node);
  }
  scan_info_database = NULL;

  return;
}

sl_status_t sli_wifi_allocate_command_buffer(sl_wifi_buffer_t **host_buffer,
                                             void **buffer,
                                             uint32_t requested_buffer_size,
                                             uint32_t wait_duration_ms)
{
  // Allocate a buffer from the SI91x host for WLAN control messages
  sl_status_t status = sli_wifi_memory_manager_allocate_buffer(host_buffer,
                                                               SL_WIFI_CONTROL_BUFFER,
                                                               requested_buffer_size,
                                                               wait_duration_ms);
  VERIFY_STATUS_AND_RETURN(status);

  uint16_t temp;
  // Get a pointer to the allocated buffer's data area
  *buffer = sli_wifi_host_get_buffer_data(*host_buffer, 0, &temp);
  return SL_STATUS_OK;
}

void sli_wifi_append_to_buffer_queue(sli_wifi_buffer_queue_t *queue, sl_wifi_buffer_t *buffer)
{
  CORE_irqState_t state = CORE_EnterAtomic();
  if (queue->tail == NULL) {
    assert(queue->head == NULL); // Both should be NULL at the same time
    queue->head = buffer;
    queue->tail = buffer;
  } else {
    queue->tail->node.node = &buffer->node;
    queue->tail            = buffer;
  }
  CORE_ExitAtomic(state);
}

void sli_wifi_set_event(uint32_t event_mask)
{
  osEventFlagsSet(sli_wifi_events, event_mask);
}

sl_status_t sli_wifi_remove_buffer_from_queue_by_comparator(sli_wifi_buffer_queue_t *queue,
                                                            const void *user_data,
                                                            sli_wifi_buffer_comparator comparator,
                                                            sl_wifi_buffer_t **buffer)
{
  // Check if the queue is empty
  if (queue->head == NULL) {
    assert(queue->tail == NULL);
    return SL_STATUS_EMPTY;
  }

  CORE_irqState_t state            = CORE_EnterAtomic();
  sl_wifi_buffer_t *current_buffer = queue->head;
  sl_wifi_buffer_t *prev_buffer    = NULL;

  // Iterate through the queue to find a matching buffer based on the comparator function
  while (current_buffer != NULL) {
    // Skip to the next buffer if this one doesn't match
    if (!comparator(current_buffer, user_data)) {
      prev_buffer    = current_buffer;
      current_buffer = (sl_wifi_buffer_t *)current_buffer->node.node;
      continue;
    }

    // Buffer matches, proceed to remove it
    *buffer = current_buffer;

    if (current_buffer == queue->head) {
      // Removing the head of the queue
      queue->head = (sl_wifi_buffer_t *)current_buffer->node.node;
      if (queue->head == NULL) {
        queue->tail = NULL; // Queue is now empty
      }
    } else if (current_buffer == queue->tail) {
      // Removing the tail of the queue
      queue->tail            = prev_buffer;
      prev_buffer->node.node = NULL;
    } else {
      // Removing a buffer from the middle
      prev_buffer->node.node = current_buffer->node.node;
    }

    CORE_ExitAtomic(state);
    return SL_STATUS_OK;
  }

  // No matching buffer was found
  CORE_ExitAtomic(state);
  return SL_STATUS_NOT_FOUND;
}

sl_wifi_max_tx_power_t sli_get_max_tx_power()
{
  return wifi_max_tx_power;
}

sl_wifi_rts_threshold_t sli_get_rts_threshold()
{
  return wifi_rts_threshold;
}

sl_wifi_mfp_config_t sli_wifi_get_mfp_mode()
{
  return wifi_mfp_config;
}

sli_wifi_feature_frame_config_t sli_wifi_get_feature_frame_config(void)
{
  return global_feature_config;
}

sl_status_t sli_wifi_get_saved_rate(sl_wifi_rate_t *transfer_rate)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(transfer_rate);

  *transfer_rate = saved_wifi_data_rate;
  return SL_STATUS_OK;
}

sl_status_t sli_wifi_host_get_credentials(sl_wifi_credential_id_t id, uint8_t type, sl_wifi_credential_t *cred)
{
  uint32_t credential_length = sizeof(sl_wifi_credential_t) - offsetof(sl_wifi_credential_t, pmk);
  sl_status_t status         = sl_wifi_get_credential(id, &cred->type, &cred->pmk, &credential_length);
  VERIFY_STATUS_AND_RETURN(status);

  if (type == SL_WIFI_PSK_CREDENTIAL) {
    if ((cred->type == SL_WIFI_PSK_CREDENTIAL) || (cred->type == SL_WIFI_PMK_CREDENTIAL)) {
      return SL_STATUS_OK;
    } else {
      return SL_STATUS_FAIL;
    }
  } else if (type != cred->type) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

void sli_save_max_tx_power(uint8_t max_scan_tx_power, uint8_t max_join_tx_power)
{
  wifi_max_tx_power.scan_tx_power = max_scan_tx_power;
  wifi_max_tx_power.join_tx_power = max_join_tx_power;
}

void sli_wifi_save_rts_threshold(uint16_t rts_threshold)
{
  wifi_rts_threshold.rts_threshold = rts_threshold;
}

void sli_wifi_save_pll_mode(const sl_wifi_pll_mode_t pll_mode)
{
  global_feature_config.pll_mode = pll_mode;
}

void sli_wifi_save_power_chain(const sl_wifi_power_chain_t power_chain)
{
  global_feature_config.power_chain = power_chain;
}

sl_status_t sli_wifi_save_mfp_mode(const sl_wifi_mfp_config_t *config)
{
  if (config == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  wifi_mfp_config.mfp_mode      = config->mfp_mode;
  wifi_mfp_config.is_configured = config->is_configured;
  return SL_STATUS_OK;
}

sl_status_t sli_wifi_save_ap_configuration(const sl_wifi_ap_configuration_t *wifi_ap_configuration)
{
  // Check if the input pointer is valid
  SL_WIFI_ARGS_CHECK_NULL_POINTER(wifi_ap_configuration);
  ap_configuration = *wifi_ap_configuration;

  return SL_STATUS_OK;
}

void sli_wifi_reset_ap_configuration()
{
  // Initialize the ap_configuration structure with zeros
  ap_configuration = (sl_wifi_ap_configuration_t){ 0 };
}

sl_status_t sli_wifi_send_power_save_request(const sl_wifi_performance_profile_v2_t *wifi_profile,
                                             const sl_bt_performance_profile_t *bt_profile)
{
  sl_status_t status;
  sli_wifi_power_save_request_t power_save_request                = { 0 };
  sl_wifi_system_performance_profile_t selected_coex_profile_mode = { 0 };
  // Disable power save mode by setting it to HIGH_PERFORMANCE profile
  status = sli_wifi_send_command(SLI_WIFI_REQ_PWRMODE,
                                 SLI_WIFI_COMMON_CMD,
                                 &power_save_request,
                                 sizeof(sli_wifi_power_save_request_t),
                                 SLI_WIFI_RSP_PWRMODE_WAIT_TIME,
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);

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
  if (selected_coex_profile_mode == SL_WIFI_SYSTEM_HIGH_PERFORMANCE) {
    return SL_STATUS_OK;
  }

  // Convert the performance profile to a power save request.
  sli_convert_performance_profile_to_power_save_command(selected_coex_profile_mode, &power_save_request);

  status = sli_wifi_send_command(SLI_WIFI_REQ_PWRMODE,
                                 SLI_WIFI_COMMON_CMD,
                                 &power_save_request,
                                 sizeof(sli_wifi_power_save_request_t),
                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WIFI_RSP_PWRMODE_WAIT_TIME),
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_get_saved_ap_configuration(sl_wifi_ap_configuration_t *wifi_ap_configuration)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(wifi_ap_configuration);
  *wifi_ap_configuration = ap_configuration;

  return SL_STATUS_OK;
}

sl_status_t sli_wifi_get_rate_protocol_and_data_rate(const uint8_t data_rate,
                                                     sl_wifi_rate_protocol_t *rate_protocol,
                                                     sl_wifi_rate_t *mask)
{
  // Map the provided data_rate to a rate_protocol and set the mask accordingly
  switch (data_rate) {
    case SL_WIFI_AUTO_RATE: {
      *rate_protocol = SL_WIFI_RATE_PROTOCOL_AUTO;
      break;
    }
    case SL_WIFI_RATE_11B_1:
    case SL_WIFI_RATE_11B_2:
    case SL_WIFI_RATE_11B_5_5:
    case SL_WIFI_RATE_11B_11: {
      *rate_protocol = SL_WIFI_RATE_PROTOCOL_B_ONLY;
      break;
    }
    case SL_WIFI_RATE_11G_6:
    case SL_WIFI_RATE_11G_9:
    case SL_WIFI_RATE_11G_12:
    case SL_WIFI_RATE_11G_18:
    case SL_WIFI_RATE_11G_24:
    case SL_WIFI_RATE_11G_36:
    case SL_WIFI_RATE_11G_48:
    case SL_WIFI_RATE_11G_54: {
      *rate_protocol = SL_WIFI_RATE_PROTOCOL_G_ONLY;
      break;
    }
    case SL_WIFI_RATE_11N_MCS0:
    case SL_WIFI_RATE_11N_MCS1:
    case SL_WIFI_RATE_11N_MCS2:
    case SL_WIFI_RATE_11N_MCS3:
    case SL_WIFI_RATE_11N_MCS4:
    case SL_WIFI_RATE_11N_MCS5:
    case SL_WIFI_RATE_11N_MCS6:
    case SL_WIFI_RATE_11N_MCS7: {
      *rate_protocol = SL_WIFI_RATE_PROTOCOL_N_ONLY;
      break;
    }
    case SL_WIFI_RATE_11AX_MCS0:
    case SL_WIFI_RATE_11AX_MCS1:
    case SL_WIFI_RATE_11AX_MCS2:
    case SL_WIFI_RATE_11AX_MCS3:
    case SL_WIFI_RATE_11AX_MCS4:
    case SL_WIFI_RATE_11AX_MCS5:
    case SL_WIFI_RATE_11AX_MCS6:
    case SL_WIFI_RATE_11AX_MCS7: {
      *rate_protocol = SL_WIFI_RATE_PROTOCOL_AX_ONLY;
      break;
    }
    default: {
      return SL_STATUS_INVALID_CONFIGURATION;
    }
  }

  *mask = data_rate;

  return SL_STATUS_OK;
}
sl_status_t sli_wifi_save_rate(sl_wifi_rate_t transfer_rate)
{
  // Check if the provided transfer rate is valid
  if (transfer_rate == SL_WIFI_RATE_INVALID) {
    return SL_STATUS_INVALID_CONFIGURATION;
  }

  saved_wifi_data_rate = transfer_rate;

  return SL_STATUS_OK;
}

void sli_wifi_reset_max_tx_power()
{
  wifi_max_tx_power.scan_tx_power = 0x1f;
  wifi_max_tx_power.join_tx_power = 0x1f;
}

void sli_wifi_reset_sl_wifi_rate()
{
  saved_wifi_data_rate = 0;
}

bool sli_wifi_get_card_ready_required()
{
  return is_card_ready_required;
}

void sli_wifi_set_card_ready_required(bool card_ready_required)
{
  is_card_ready_required = card_ready_required;
}

//In Access point mode NWP only supports No Encryption, TKIP and CCMP encryptions.
sl_status_t sli_wifi_get_nwp_encryption(sl_wifi_encryption_t encryption_mode, uint8_t *encryption_request)
{
  switch (encryption_mode) {
    case SL_WIFI_NO_ENCRYPTION:
      *encryption_request = SLI_WIFI_NO_ENCRYPTION;
      break;
    case SL_WIFI_TKIP_ENCRYPTION:
      *encryption_request = SLI_WIFI_TKIP_ENCRYPTION;
      break;
    case SL_WIFI_CCMP_ENCRYPTION:
    case SL_WIFI_DEFAULT_ENCRYPTION:
      *encryption_request = SLI_WIFI_CCMP_ENCRYPTION;
      break;
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

uint8_t sli_lmac_crc8_c(uint8_t crc8_din, uint8_t crc8_state, uint8_t end)
{
  uint8_t din[8];
  uint8_t state[8];
  uint8_t state_c[8];
  uint8_t crc8_out;

  din[0] = ((crc8_din & BIT(7)) >> 7);
  din[1] = ((crc8_din & BIT(6)) >> 6);
  din[2] = ((crc8_din & BIT(5)) >> 5);
  din[3] = ((crc8_din & BIT(4)) >> 4);
  din[4] = ((crc8_din & BIT(3)) >> 3);
  din[5] = ((crc8_din & BIT(2)) >> 2);
  din[6] = ((crc8_din & BIT(1)) >> 1);
  din[7] = ((crc8_din & BIT(0)) >> 0);

  state[0] = ((crc8_state & BIT(0)) >> 0);
  state[1] = ((crc8_state & BIT(1)) >> 1);
  state[2] = ((crc8_state & BIT(2)) >> 2);
  state[3] = ((crc8_state & BIT(3)) >> 3);
  state[4] = ((crc8_state & BIT(4)) >> 4);
  state[5] = ((crc8_state & BIT(5)) >> 5);
  state[6] = ((crc8_state & BIT(6)) >> 6);
  state[7] = ((crc8_state & BIT(7)) >> 7);

  state_c[7] = (state[7] ^ din[7]) ^ (state[6] ^ din[6]) ^ (state[5] ^ din[5]);

  state_c[6] = (state[6] ^ din[6]) ^ (state[5] ^ din[5]) ^ (state[4] ^ din[4]);

  state_c[5] = (state[5] ^ din[5]) ^ (state[4] ^ din[4]) ^ (state[3] ^ din[3]);

  state_c[4] = (state[4] ^ din[4]) ^ (state[3] ^ din[3]) ^ (state[2] ^ din[2]);

  state_c[3] = (state[1] ^ din[1]) ^ (state[2] ^ din[2]) ^ (state[3] ^ din[3]) ^ (state[7] ^ din[7]);

  state_c[2] = (state[0] ^ din[0]) ^ (state[1] ^ din[1]) ^ (state[2] ^ din[2]) ^ (state[6] ^ din[6]);

  state_c[1] = (state[0] ^ din[0]) ^ (state[1] ^ din[1]) ^ (state[6] ^ din[6]);

  state_c[0] = (state[0] ^ din[0]) ^ (state[7] ^ din[7]) ^ (state[6] ^ din[6]);
  if (!end) {
    crc8_out = (uint8_t)(((state_c[0] & BIT(0)) << 0) | ((state_c[1] & BIT(0)) << 1) | ((state_c[2] & BIT(0)) << 2)
                         | ((state_c[3] & BIT(0)) << 3) | ((state_c[4] & BIT(0)) << 4) | ((state_c[5] & BIT(0)) << 5)
                         | ((state_c[6] & BIT(0)) << 6) | ((state_c[7] & BIT(0)) << 7));
  } else {
    crc8_out = (uint8_t)(((state_c[7] & BIT(0)) << 0) | ((state_c[6] & BIT(0)) << 1) | ((state_c[5] & BIT(0)) << 2)
                         | ((state_c[4] & BIT(0)) << 3) | ((state_c[3] & BIT(0)) << 4) | ((state_c[2] & BIT(0)) << 5));

    crc8_out = ~crc8_out;
    crc8_out &= 0x3f;
  }
  return crc8_out;
}

uint8_t sli_multicast_mac_hash(const uint8_t *mac)
{
  uint8_t crc = 0xff;
  for (uint8_t i = 0; i < 6; i++) {
    crc = sli_lmac_crc8_c(mac[i], crc, ((i == 5) ? 1 : 0));
  }
  return crc;
}

#ifndef __ZEPHYR__
/******************************************************************************
 * @brief
 * 	A utility function that store the firmware status code in thread specific storage.
 * @param[in] converted_firmware_status
 *	Firmware status code that needs to be saved.
 *****************************************************************************/
void sli_wifi_save_firmware_status(sl_status_t converted_firmware_status)
{
  sli_osTaskRegisterSetValue(NULL, sli_fw_status_storage_index, converted_firmware_status);
}
#endif

/***************************************************************************/ /**
 * @brief
 *   Returns a pointer to the scan information database.
 * @pre Pre-conditions:
 *   The Wi-Fi scan operation should be completed before accessing the scan information database.
 * @return
 *   Pointer to the scan information database.
 *   The returned pointer can be used to retrieve or manipulate scan information as needed.
 ******************************************************************************/
sli_scan_info_t **sli_get_scan_info_database(void)
{
  return &scan_info_database;
}

bool sli_wifi_packet_identification_function(const sl_wifi_buffer_t *buffer, const void *user_data)
{
  const uint8_t *packet_id = (const uint8_t *)user_data;

  // Check if the packet's packet ID matches the expected one
  return (*packet_id == buffer->id);
}

uint32_t sli_wifi_host_queue_status(const sli_wifi_buffer_queue_t *queue)
{
  return queue->head != NULL;
}

uint32_t sl_wifi_host_elapsed_time(uint32_t starting_timestamp)
{
  return (osKernelGetTickCount() - starting_timestamp);
}
