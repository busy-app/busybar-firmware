/***************************************************************************/ /**
 * @file    sli_wifi.c
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
#include "sl_status.h"
#include "sl_string.h"
#include "sl_wifi.h"
#include "sli_wifi.h"
#include "sli_wifi_constants.h"
#include "sli_wifi_types.h"
#include "cmsis_os2.h"
#include "sl_cmsis_utility.h"
#include "sl_core.h"
#include "sl_wifi_region_db_config.h"
#include "sli_wifi_utility.h"
#include "sli_wifi_memory_manager.h"
#include <string.h>
#include "sli_buffer_manager.h"

#ifndef SL_NCP_DEFAULT_COMMAND_WAIT_TIME
#define SL_NCP_DEFAULT_COMMAND_WAIT_TIME 3000
#endif
#ifndef MIN
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

#define SLI_MAX_SIZE_OF_UINT16_T 65535

#define MAX_FLOW_ID                              7
#define MAX_WAKE_INTERVAL_EXPONENT               31
#define MAX_WAKE_INTERVAL_EXPONENT_TOLERANCE     31
#define MAX_WAKE_DURATION_UNIT                   1
#define MAX_TWT_RETRY_LIMIT                      15
#define MIN_TWT_RETRY_INTERVAL                   5
#define MAX_TWT_REQ_TYPE                         2
#define REQUEST_TWT                              0
#define TWT_WAKE_DURATION_UNIT_1024TU            1024
#define TWT_WAKE_DURATION_UNIT_256TU             256
#define ABSOLUTE_POWER_VALUE_TOGGLE              0x80
#define MAX_2_4G_CHANNEL                         14
#define PASSIVE_SCAN_ENABLE                      BIT(7)
#define LP_CHAIN_ENABLE                          BIT(6)
#define QUICK_SCAN_ENABLE                        1
#define SCAN_RESULTS_TO_HOST                     2
#define DEFAULT_LISTEN_INTERVAL_MULTIPLIER       1
#define ALWAYS_ROAM                              1
#define MAX_TX_AND_RX_LATENCY_LIMIT              21600000
#define MAX_TWT_SUSPEND_DURATION                 0x5265c00
#define DEVICE_AVERAGE_THROUGHPUT                20000
#define SLI_WIFI_AP_OPT_KEEPALIVE_TYPE_MASK      0x03 // bits 0-1
#define SLI_WIFI_AP_OPT_BEACON_STOP              BIT(2)
#define SLI_WIFI_AP_OPT_DYNAMIC_HIDDEN_SSID_CONF BIT(3)
#define ETH_MAC_ADR_LEN                          6

/*=======================================================================*/
// Enterprise configuration command parameters
/*=======================================================================*/

// Enterprise method should be one of the following: TLS, TTLS, FAST or PEAP.
// PEAP encryption now supports three security levels through different enum values:
// 	SL_WIFI_PEAP_MSCHAPV2_ENCRYPTION - The EAP server may bypass Phase2 authentication (less secure).
// 	SL_WIFI_PEAP_SAFE1_ENCRYPTION - If a client certificate (private_key/client_cert) is not used and TLS session resumption is not used, then Phase2 authentication is mandatory.
// 	SL_WIFI_PEAP_SAFE2_ENCRYPTION - Requires Phase2 authentication in all cases (most secure).
// Users can select the desired PEAP security level by setting the appropriate encryption type in their Wi-Fi profile.

#define SL_EAP_TLS_METHOD       "TLS"
#define SL_EAP_TTLS_METHOD      "TTLS"
#define SL_EAP_FAST_METHOD      "FAST"
#define SL_EAP_PEAP_METHOD      "PEAP"
#define SL_EAP_PEAPSAFE1_METHOD "PEAPSAFE1"
#define SL_EAP_PEAPSAFE2_METHOD "PEAPSAFE2"
#define SL_EAP_LEAP_METHOD      "LEAP"
// This parameter is used to configure the module in Enterprise security mode
#define SL_EAP_INNER_METHOD "\"auth=MSCHAPV2\""
// Private Key Password is required for encrypted private key, format is like "\"12345678\""
#define SL_DEFAULT_PRIVATE_KEY_PASSWORD ""

/*========================================================================*/
// 11ax params
/*========================================================================*/
#define SLI_NOMINAL_PE         2
#define SLI_DCM_ENABLE         0
#define SLI_LDPC_ENABLE        0
#define SLI_NG_CB_ENABLE       0
#define SLI_NG_CB_VALUES       0
#define SLI_UORA_ENABLE        0
#define SLI_TRIGGER_RESP_IND   0xF
#define SLI_IPPS_VALID_VALUE   0
#define SLI_TX_ONLY_ON_AP_TRIG 0
#define SLI_CONFIG_ER_SU \
  0 // 0 - NO ER_SU support, 1 - Use ER_SU rates along with Non_ER_SU rates, 2 - Use ER_SU rates only
#define SLI_SI91X_ENABLE_TWT_FEATURE      1
#define SLI_ENABLE_BEAMFORMEE_SUPPORT     0
#define SLI_DISABLE_SU_BEAMFORMEE_SUPPORT 1
#define SLI_DISABLE_MU_BEAMFORMEE_SUPPORT 2
/*=========================================================================*/
extern sli_wifi_performance_profile_t performance_profile;
extern bool device_initialized;
extern bool bg_enabled;
extern bool interface_is_up[SL_WIFI_MAX_INTERFACE_INDEX];
extern volatile uint32_t tx_command_queues_status;
extern sli_wifi_command_queue_t cmd_queues[SI91X_CMD_MAX];
extern sl_wifi_interface_t default_interface;
osEventFlagsId_t sli_wifi_events                                             = 0;
sl_wifi_advanced_scan_configuration_t advanced_scan_configuration            = { 0 };
static sl_wifi_advanced_client_configuration_t advanced_client_configuration = { 0 };
static uint8_t client_join_feature_bitmap                                    = SL_WIFI_JOIN_FEAT_LISTEN_INTERVAL_VALID;
static uint8_t ap_join_feature_bitmap                                        = SL_WIFI_JOIN_FEAT_LISTEN_INTERVAL_VALID;

static sl_status_t sli_configure_scan_request(const sl_wifi_client_configuration_t *ap,
                                              sli_wifi_request_scan_t *scan_request,
                                              sl_wifi_interface_t interface)
{
  memset(scan_request, 0, sizeof(*scan_request));

  //! copying SSID & length
  if (ap->ssid.length > 0) {
    memcpy(scan_request->ssid, ap->ssid.value, ap->ssid.length);
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Configure channel bitmap for selective channel scan
  if ((interface & SL_WIFI_CLIENT_2_4GHZ_INTERFACE) && (ap->channel_bitmap.channel_bitmap_2_4 > 0)) {
    memcpy(&scan_request->channel_bit_map_2_4,
           &ap->channel_bitmap.channel_bitmap_2_4,
           sizeof(scan_request->channel_bit_map_2_4));
  }

  if ((interface & SL_WIFI_CLIENT_5GHZ_INTERFACE) && (ap->channel_bitmap.channel_bitmap_5 > 0)) {
    memcpy(&scan_request->channel_bit_map_5,
           &ap->channel_bitmap.channel_bitmap_5,
           sizeof(scan_request->channel_bit_map_5));
  }

  sl_wifi_max_tx_power_t wifi_max_tx_power = sli_get_max_tx_power();
  // Within the 1-byte scan_feature_bimap variable, last 5 bits(bit 3 through bit 7) are allocated for
  // encoding the transmit power during scan procedure.
  scan_request->scan_feature_bitmap = (uint8_t)(wifi_max_tx_power.scan_tx_power << 3);

  return SL_STATUS_OK;
}

static sl_status_t sli_handle_enterprise_security(const sl_wifi_client_configuration_t *ap,
                                                  sli_wifi_request_eap_config_t *eap_req)
{
  memset(eap_req, 0, sizeof(*eap_req));
  eap_req->user_identity[0] = '"';
  eap_req->password[0]      = '"';

  sl_wifi_credential_t cred;
  sl_status_t status = sli_wifi_host_get_credentials(ap->credential_id, SL_WIFI_EAP_CREDENTIAL, &cred);
  VERIFY_STATUS_AND_RETURN(status);

  memcpy(&(eap_req->user_identity[1]), cred.eap.username, 63);
  memcpy(&(eap_req->password[1]), cred.eap.password, 127);
  uint8_t uid_len                     = (uint8_t)sl_strnlen((char *)eap_req->user_identity, 64);
  uint8_t psk_len                     = (uint8_t)sl_strnlen((char *)eap_req->password, 128);
  eap_req->user_identity[uid_len]     = '"';
  eap_req->password[psk_len]          = '"';
  eap_req->user_identity[uid_len + 1] = 0;
  eap_req->password[psk_len + 1]      = 0;

  if ((uint64_t)((SL_WIFI_EAP_TLS_ENCRYPTION == ap->encryption)
                 && (sizeof(eap_req->eap_method)) > sl_strlen(SL_EAP_TLS_METHOD))) {
    memcpy(eap_req->eap_method, SL_EAP_TLS_METHOD, sl_strlen(SL_EAP_TLS_METHOD));
  } else if ((SL_WIFI_EAP_TTLS_ENCRYPTION == ap->encryption)
             && (sizeof(eap_req->eap_method) > (size_t)sl_strlen(SL_EAP_TTLS_METHOD))) {
    memcpy(eap_req->eap_method, SL_EAP_TTLS_METHOD, sl_strlen(SL_EAP_TTLS_METHOD));
  } else if ((SL_WIFI_EAP_FAST_ENCRYPTION == ap->encryption)
             && (sizeof(eap_req->eap_method) > (size_t)sl_strlen(SL_EAP_FAST_METHOD))) {
    memcpy(eap_req->eap_method, SL_EAP_FAST_METHOD, sl_strlen(SL_EAP_FAST_METHOD));
  } else if ((SL_WIFI_PEAP_MSCHAPV2_ENCRYPTION == ap->encryption)
             && (sizeof(eap_req->eap_method) > (size_t)sl_strlen(SL_EAP_PEAP_METHOD))) {
    memcpy(eap_req->eap_method, SL_EAP_PEAP_METHOD, sl_strlen(SL_EAP_PEAP_METHOD));
  } else if (SL_WIFI_PEAP_SAFE1_ENCRYPTION == ap->encryption) {
    memcpy(eap_req->eap_method, SL_EAP_PEAPSAFE1_METHOD, sl_strlen(SL_EAP_PEAPSAFE1_METHOD));
  } else if (SL_WIFI_PEAP_SAFE2_ENCRYPTION == ap->encryption) {
    memcpy(eap_req->eap_method, SL_EAP_PEAPSAFE2_METHOD, sl_strlen(SL_EAP_PEAPSAFE2_METHOD));
  } else if ((SL_WIFI_EAP_LEAP_ENCRYPTION == ap->encryption)
             && (sizeof(eap_req->eap_method) > (size_t)sl_strlen(SL_EAP_LEAP_METHOD))) {
    memcpy(eap_req->eap_method, SL_EAP_LEAP_METHOD, sl_strlen(SL_EAP_LEAP_METHOD));
  } else {
    return SL_STATUS_WIFI_INVALID_ENCRYPTION_METHOD;
  }

  if (sizeof(eap_req->inner_method) > (size_t)sl_strlen(SL_EAP_INNER_METHOD)) {
    memcpy(eap_req->inner_method, SL_EAP_INNER_METHOD, sl_strlen(SL_EAP_INNER_METHOD));
  }
  memcpy(eap_req->okc_enable, &cred.eap.eap_flags, sizeof(cred.eap.eap_flags));

  uint8_t key_len = (uint8_t)sl_strnlen((char *)cred.eap.certificate_key, SL_WIFI_EAP_CERTIFICATE_KEY_LENGTH);
  if (key_len > 0) {
    eap_req->private_key_password[0] = '"';
    if (sizeof(eap_req->private_key_password) > sizeof(cred.eap.certificate_key)) {
      memcpy(eap_req->private_key_password, cred.eap.certificate_key, sizeof(cred.eap.certificate_key));
    }
    eap_req->private_key_password[key_len + 1] = '"';
  } else if (sizeof(eap_req->private_key_password) > sl_strlen(SL_DEFAULT_PRIVATE_KEY_PASSWORD)) {
    memcpy(eap_req->private_key_password, SL_DEFAULT_PRIVATE_KEY_PASSWORD, sl_strlen(SL_DEFAULT_PRIVATE_KEY_PASSWORD));
  }

  return sli_wifi_send_command(SLI_WIFI_REQ_EAP_CONFIG,
                               SLI_WIFI_WLAN_CMD,
                               eap_req,
                               sizeof(*eap_req),
                               SLI_WIFI_WAIT_FOR_COMMAND_SUCCESS,
                               NULL,
                               NULL);
}

static sl_status_t sli_handle_psk_security(const sl_wifi_client_configuration_t *ap)
{
  sli_wifi_request_psk_t psk_request;
  memset(&psk_request, 0, sizeof(psk_request));

  sl_wifi_credential_t cred = { 0 };
  sl_status_t status        = sli_wifi_host_get_credentials(ap->credential_id, SL_WIFI_PSK_CREDENTIAL, &cred);
  VERIFY_STATUS_AND_RETURN(status);

  psk_request.type = cred.type == SL_WIFI_PSK_CREDENTIAL ? 1 : 2;
  if (cred.type == SL_WIFI_PSK_CREDENTIAL) {
    memcpy(psk_request.psk_or_pmk, cred.psk.value, SL_WIFI_MAX_PSK_LENGTH);
  } else {
    memcpy(psk_request.psk_or_pmk, cred.pmk.value, SL_WIFI_MAX_PMK_LENGTH);
  }

  return sli_wifi_send_command(SLI_WIFI_REQ_HOST_PSK,
                               SLI_WIFI_WLAN_CMD,
                               &psk_request,
                               sizeof(psk_request),
                               SLI_WIFI_WAIT_FOR_COMMAND_SUCCESS,
                               NULL,
                               NULL);
}

static sl_status_t sli_wifi_convert_client_info(sl_wifi_client_info_response_t *client_info_response,
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

sl_status_t sli_wifi_configure_timeout(sli_wifi_timeout_type_t timeout_type, uint16_t timeout_value)
{
  sl_status_t status                         = SL_STATUS_OK;
  sli_wifi_request_timeout_t timeout_request = { 0 };

  if (timeout_type > SLI_WIFI_CHANNEL_PASSIVE_SCAN_TIMEOUT) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  timeout_request.timeout_bitmap = BIT(timeout_type);
  timeout_request.timeout_value  = timeout_value;
  status                         = sli_wifi_send_command(SLI_WIFI_REQ_TIMEOUT,
                                 SLI_WIFI_WLAN_CMD,
                                 &timeout_request,
                                 sizeof(sli_wifi_request_timeout_t),
                                 SLI_WIFI_WAIT_FOR(30100),
                                 NULL,
                                 NULL);
  return status;
}

bool sli_wifi_is_interface_up(sl_wifi_interface_t interface)
{
  switch (interface) {
    case SL_WIFI_CLIENT_INTERFACE:
      return interface_is_up[SL_WIFI_CLIENT_2_4GHZ_INTERFACE_INDEX]
             || interface_is_up[SL_WIFI_CLIENT_5GHZ_INTERFACE_INDEX]
             || interface_is_up[SL_WIFI_CLIENT_DUAL_INTERFACE_INDEX];
    case SL_WIFI_AP_INTERFACE:
      return interface_is_up[SL_WIFI_AP_2_4GHZ_INTERFACE_INDEX] || interface_is_up[SL_WIFI_AP_5GHZ_INTERFACE_INDEX]
             || interface_is_up[SL_WIFI_AP_DUAL_INTERFACE_INDEX];
    case SL_WIFI_CLIENT_2_4GHZ_INTERFACE:
      return interface_is_up[SL_WIFI_CLIENT_2_4GHZ_INTERFACE_INDEX];
    case SL_WIFI_AP_2_4GHZ_INTERFACE:
      return interface_is_up[SL_WIFI_AP_2_4GHZ_INTERFACE_INDEX];
    case SL_WIFI_CLIENT_5GHZ_INTERFACE:
      return interface_is_up[SL_WIFI_CLIENT_5GHZ_INTERFACE_INDEX];
    case SL_WIFI_AP_5GHZ_INTERFACE:
      return interface_is_up[SL_WIFI_AP_5GHZ_INTERFACE_INDEX];
    case SL_WIFI_CLIENT_DUAL_INTERFACE:
      return interface_is_up[SL_WIFI_CLIENT_DUAL_INTERFACE_INDEX];
    case SL_WIFI_AP_DUAL_INTERFACE:
      return interface_is_up[SL_WIFI_AP_DUAL_INTERFACE_INDEX];
    case SL_WIFI_2_4GHZ_INTERFACE:
      return interface_is_up[SL_WIFI_CLIENT_2_4GHZ_INTERFACE_INDEX]
             || interface_is_up[SL_WIFI_AP_2_4GHZ_INTERFACE_INDEX];
    case SL_WIFI_5GHZ_INTERFACE:
      return interface_is_up[SL_WIFI_CLIENT_5GHZ_INTERFACE_INDEX] || interface_is_up[SL_WIFI_AP_5GHZ_INTERFACE_INDEX];
    case SL_WIFI_DUAL_INTERFACE:
      return interface_is_up[SL_WIFI_CLIENT_DUAL_INTERFACE_INDEX] || interface_is_up[SL_WIFI_AP_DUAL_INTERFACE_INDEX];
    case SL_WIFI_ALL_INTERFACES:
      return interface_is_up[SL_WIFI_CLIENT_5GHZ_INTERFACE_INDEX] || interface_is_up[SL_WIFI_AP_5GHZ_INTERFACE_INDEX]
             || interface_is_up[SL_WIFI_CLIENT_2_4GHZ_INTERFACE_INDEX]
             || interface_is_up[SL_WIFI_AP_2_4GHZ_INTERFACE_INDEX]
             || interface_is_up[SL_WIFI_CLIENT_DUAL_INTERFACE_INDEX]
             || interface_is_up[SL_WIFI_AP_DUAL_INTERFACE_INDEX];
    case SL_WIFI_TRANSCEIVER_INTERFACE:
      return interface_is_up[SL_WIFI_TRANSCEIVER_INTERFACE_INDEX];
    default:
      return false;
  }
}

sl_status_t sli_wifi_wait_for_response_packet(sli_wifi_buffer_queue_t *queue,
                                              osEventFlagsId_t event_flag,
                                              uint32_t event_mask,
                                              uint16_t packet_id,
                                              sli_wifi_wait_period_t wait_period,
                                              sl_wifi_buffer_t **packet_buffer)
{
  // Verify that packet_buffer is a valid pointer, return error if invalid
  SL_VERIFY_POINTER_OR_RETURN(packet_buffer, SL_STATUS_INVALID_PARAMETER);
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
    sl_status_t packet_status = sli_wifi_remove_buffer_from_queue_by_comparator(queue,
                                                                                &packet_id,
                                                                                sli_wifi_packet_identification_function,
                                                                                &buffer);

    if (packet_status == SL_STATUS_OK) {
      // If the queue is now empty, clear the event flag
      if (0 == sli_wifi_host_queue_status(queue)) {
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
    elapsed_time = sl_wifi_host_elapsed_time(start_time);
  }
  // This code path should not be reached; loop exits on success or timeout.
  return SL_STATUS_FAIL;
}

static sl_status_t sli_wifi_convert_old_gain_table_to_su_gain_table(const uint8_t old_table[],
                                                                    uint8_t new_table[],
                                                                    uint16_t payload_length,
                                                                    uint16_t *su_tb_payload_length)
{
  sl_status_t status    = SL_STATUS_OK;
  uint8_t new_offset    = 0;
  uint8_t old_offset    = 0;
  uint8_t loop_size     = 0;
  uint8_t column_num    = 0;
  uint8_t extra_col_len = 0;

  if ((old_table == NULL) || (new_table == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  new_table[new_offset++] = old_table[old_offset++];                 // Num of regions
  new_table[new_offset++] = old_table[old_offset++];                 // Region code
  loop_size               = ((old_table[old_offset] & 0xf) * 5) + 3; // Total_Regions+Region Code+Total Channels
  new_table[new_offset++] = old_table[old_offset++];                 // Num of channels
  while (old_offset <= payload_length) {
    while (old_offset <= loop_size) { // Repeat loop for number of current region rows*columns
      if (column_num == 5) {
        new_table[new_offset++] = old_table[old_offset - 1]; // Duplicating the 5th row(11ax SU) to 6th row(11ax TB)
        extra_col_len++;
        column_num = 0;
        if (old_offset == loop_size)
          break;
      } else {
        column_num++;
        new_table[new_offset++] = old_table[old_offset++];
      }
    }
    new_table[new_offset++] = old_table[old_offset++];                // Region code
    loop_size = old_offset + ((old_table[old_offset] & 0xf) * 5) + 1; // Updated the loop size based on num of rows
    new_table[new_offset++] = old_table[old_offset++];                // Num of channels
  }
  *su_tb_payload_length = payload_length + extra_col_len;
  return status;
}

static sl_status_t sli_wifi_fill_join_request_security_using_encryption(sl_wifi_encryption_t encryption_mode,
                                                                        uint8_t *security_type)
{
  if (encryption_mode == SL_WIFI_CCMP_ENCRYPTION) {
    *security_type |= BIT(7);
  }
  return SL_STATUS_OK;
}

static sl_status_t sli_wifi_configure_mfp_mode(sl_wifi_mfp_config_t *mfp_config,
                                               uint8_t security_type,
                                               uint8_t *join_feature_bitmap,
                                               sl_wifi_interface_t interface)
{
  sl_status_t status = SL_STATUS_OK;
  if (mfp_config->is_configured) {
    // User explicitly configured MFP - honor it for both STA and AP modes
    switch (mfp_config->mfp_mode) {
      case SL_WIFI_MFP_REQUIRED:
        *join_feature_bitmap |= SL_WIFI_JOIN_FEAT_MFP_CAPABLE_REQUIRED;
        break;
      case SL_WIFI_MFP_CAPABLE:
        *join_feature_bitmap |= SL_WIFI_JOIN_FEAT_MFP_CAPABLE_ONLY;
        break;
      default:
        break;
    }
  } else {
    // User did not configure MFP
    if ((security_type == SL_WIFI_WPA3) || (security_type == SL_WIFI_WPA3_ENTERPRISE)) {
      *join_feature_bitmap |= SL_WIFI_JOIN_FEAT_MFP_CAPABLE_REQUIRED;
      mfp_config->mfp_mode = SL_WIFI_MFP_REQUIRED;
    } else if ((security_type == SL_WIFI_WPA3_TRANSITION) || (security_type == SL_WIFI_WPA3_TRANSITION_ENTERPRISE)) {
      *join_feature_bitmap &= ~(SL_WIFI_JOIN_FEAT_MFP_CAPABLE_REQUIRED);
      *join_feature_bitmap |= SL_WIFI_JOIN_FEAT_MFP_CAPABLE_ONLY;
      mfp_config->mfp_mode = SL_WIFI_MFP_CAPABLE;
    } else if (security_type == SL_WIFI_WPA2 || security_type == SL_WIFI_WPA_WPA2_MIXED) {
      // Automatically enable MFP only for STA/Client mode with WPA2
      // For AP mode, MFP should only be enabled if user explicitly configures it
      if (interface & SL_WIFI_CLIENT_INTERFACE) {
        *join_feature_bitmap |= SL_WIFI_JOIN_FEAT_MFP_CAPABLE_ONLY;
        mfp_config->mfp_mode = SL_WIFI_MFP_CAPABLE;
      }
    }
    status = sli_wifi_save_mfp_mode(mfp_config);
  }
  return status;
}

sl_status_t sli_wifi_get_join_configuration(sl_wifi_interface_t interface, uint8_t *join_feature_bitmap)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(join_feature_bitmap);

  // Determine whether to retrieve the configuration for the client or AP interface
  if (interface & SL_WIFI_CLIENT_INTERFACE) {
    *join_feature_bitmap = client_join_feature_bitmap;
  } else if (interface & SL_WIFI_AP_INTERFACE) {
    *join_feature_bitmap = ap_join_feature_bitmap;
  } else {
    return SL_STATUS_WIFI_UNKNOWN_INTERFACE;
  }

  return SL_STATUS_OK;
}

sl_status_t sli_wifi_set_join_configuration(sl_wifi_interface_t interface, uint8_t join_feature_bitmap)
{
  // Determine whether the configuration is for the client or AP interface
  if (interface & SL_WIFI_CLIENT_INTERFACE) {
    client_join_feature_bitmap = join_feature_bitmap;
  } else if (interface & SL_WIFI_AP_INTERFACE) {
    ap_join_feature_bitmap = join_feature_bitmap;
  } else {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

sl_status_t sli_wifi_get_configured_join_request(sl_wifi_interface_t module_interface,
                                                 const void *configuration,
                                                 sli_wifi_join_request_t *join_request)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(configuration);
  SL_WIFI_ARGS_CHECK_NULL_POINTER(join_request);
  sl_status_t status = SL_STATUS_OK;

  if (module_interface & SL_WIFI_CLIENT_INTERFACE) {
    // get join feature bitmap
    status = sli_wifi_get_join_configuration(SL_WIFI_CLIENT_INTERFACE, &(join_request->join_feature_bitmap));
    VERIFY_STATUS_AND_RETURN(status);

    const sl_wifi_client_configuration_t *client_configuration = (const sl_wifi_client_configuration_t *)configuration;

    sl_wifi_listen_interval_v2_t listen_interval;
    sli_wifi_get_listen_interval_v2(module_interface, &listen_interval);
    // narrowing conversion from Enum of uint16 to uint8
    sli_wifi_get_saved_rate(&join_request->data_rate);
    memcpy(join_request->ssid, client_configuration->ssid.value, client_configuration->ssid.length);

    join_request->ssid_len      = client_configuration->ssid.length;
    join_request->security_type = (uint8_t)client_configuration->security;

    sl_wifi_mfp_config_t mfp_config = sli_wifi_get_mfp_mode();
    status                          = sli_wifi_configure_mfp_mode(&mfp_config,
                                         join_request->security_type,
                                         &join_request->join_feature_bitmap,
                                         SL_WIFI_CLIENT_INTERFACE);
    VERIFY_STATUS_AND_RETURN(status);

    sli_wifi_fill_join_request_security_using_encryption(client_configuration->encryption,
                                                         &(join_request->security_type));

    join_request->vap_id                     = SL_WIFI_CLIENT_VAP_ID; // For Station vap_id will be 0
    join_request->listen_interval            = listen_interval.listen_interval;
    join_request->listen_interval_multiplier = (uint8_t)listen_interval.listen_interval_multiplier;
    memcpy(join_request->join_bssid, client_configuration->bssid.octet, sizeof(join_request->join_bssid));
  } else if (module_interface & SL_WIFI_AP_INTERFACE) {
    // get join feature bitmap
    status = sli_wifi_get_join_configuration(SL_WIFI_AP_INTERFACE, &(join_request->join_feature_bitmap));
    VERIFY_STATUS_AND_RETURN(status);

    const sl_wifi_ap_configuration_t *ap_configuration = (const sl_wifi_ap_configuration_t *)configuration;

    sli_wifi_get_saved_rate(&join_request->data_rate);
    memcpy(join_request->ssid, ap_configuration->ssid.value, ap_configuration->ssid.length);

    join_request->ssid_len      = ap_configuration->ssid.length;
    join_request->security_type = (uint8_t)ap_configuration->security;
    join_request->vap_id        = 0;

    sl_wifi_mfp_config_t mfp_config = sli_wifi_get_mfp_mode();
    status                          = sli_wifi_configure_mfp_mode(&mfp_config,
                                         join_request->security_type,
                                         &join_request->join_feature_bitmap,
                                         SL_WIFI_AP_INTERFACE);
    VERIFY_STATUS_AND_RETURN(status);

    if (sli_wifi_get_opermode() == SL_WIFI_CONCURRENT_MODE) {
      join_request->vap_id = SL_WIFI_AP_VAP_ID; // For Concurrent mode AP vap_id should be 1 else 0.
    }
  } else {
    return SL_STATUS_FAIL;
  }

  sl_wifi_max_tx_power_t wifi_max_tx_power = sli_get_max_tx_power();

  /* The MSB is set by default, the 'power_level' variable encodes the absolute power value using bits 2 through 6. */
  join_request->power_level = (uint8_t)((wifi_max_tx_power.join_tx_power << 2) | ABSOLUTE_POWER_VALUE_TOGGLE);

  return SL_STATUS_OK;
}

static void sli_configure_channel_bitmap(sl_wifi_interface_t interface,
                                         const sl_wifi_scan_configuration_t *configuration,
                                         sli_wifi_request_scan_t *scan_request)
{
  if (!(interface & SL_WIFI_2_4GHZ_INTERFACE) || (configuration->channel_bitmap_2g4 == 0xFFFF)) {
    return;
  }

  memcpy(&scan_request->channel_bit_map_2_4,
         &configuration->channel_bitmap_2g4,
         sizeof(scan_request->channel_bit_map_2_4));

  uint16_t channel_bitmap = configuration->channel_bitmap_2g4;

  // Set channel, if only one channel is selected in channel_bitmap
  // if channel is 0, scan on all channels.
  if ((channel_bitmap > 0) && (0 == (channel_bitmap & (channel_bitmap - 1)))) {
    for (uint8_t channel = 1; channel <= MAX_2_4G_CHANNEL; channel++) {
      if (BIT((channel - 1)) == channel_bitmap) {
        scan_request->channel[0] = channel;
        break;
      }
    }
  }
}
sl_status_t sli_wifi_wps_connect(sli_wifi_wps_config_t wps_config, sl_wifi_wps_response_t *wps_response)
{
  sl_status_t status;
  sli_wifi_request_scan_t scan_request = { 0 };
  sli_wifi_join_request_t join_request = { 0 };
  sl_wifi_buffer_t *buffer             = NULL;
  sl_wifi_system_packet_t *packet      = NULL;

  status = sli_wifi_send_command(SLI_WIFI_REQ_SCAN,
                                 SLI_WIFI_WLAN_CMD,
                                 &scan_request,
                                 sizeof(scan_request),
                                 SLI_WIFI_WAIT_FOR(60000),
                                 NULL,
                                 NULL);
  SL_DEBUG_LOG("status of scan req : 0x%lx\n", status);
  VERIFY_STATUS_AND_RETURN(status);

  status = sli_wifi_send_command(SLI_WIFI_REQ_JOIN,
                                 SLI_WIFI_WLAN_CMD,
                                 &join_request,
                                 sizeof(join_request),
                                 SLI_WIFI_WAIT_FOR_OTAF_RESPONSE,
                                 NULL,
                                 (void **)&buffer);
  SL_DEBUG_LOG("status of join req : 0x%lx\n", status);

  if (buffer != NULL) {
    packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
    memcpy(wps_response, &packet->data[1], sizeof(sl_wifi_wps_response_t));
    sli_wifi_memory_manager_free_buffer(buffer);
  } else {
    return SL_STATUS_FAIL;
  }

  if (!(wps_config.auto_connect == 1 && status == SL_STATUS_OK)) {
    sl_status_t temp_status = sli_wifi_send_command(SLI_WIFI_REQ_INIT,
                                                    SLI_WIFI_WLAN_CMD,
                                                    NULL,
                                                    0,
                                                    SLI_WIFI_WAIT_FOR_COMMAND_SUCCESS,
                                                    NULL,
                                                    NULL);
    SL_DEBUG_LOG("status of init req : 0x%lx\n", temp_status);
    // When auto_connect is disabled and credentials were received, don't fail on INIT errors.
    // The subsequent logic will handle the credentials case and return appropriate status.
    if (wps_config.auto_connect == 1 || status != SL_STATUS_SI91X_WPS_CREDENTIALS_RECEIVED_WITHOUT_JOIN_COMMAND) {
      VERIFY_STATUS_AND_RETURN(temp_status);
    }
  }

  if (wps_config.auto_connect == 0 && status != SL_STATUS_OK) {
    if (status == SL_STATUS_SI91X_WPS_CREDENTIALS_RECEIVED_WITHOUT_JOIN_COMMAND) {
      // Return SL_STATUS_OK because credentials were received successfully.
      // Since auto_connect is disabled, joining is not required and the WPS operation is considered successful.
      return SL_STATUS_OK;
    }
  }

  if (wps_config.auto_connect == 1 && status != SL_STATUS_OK) {
    if (wps_response->ssid_len != 0) {
      SL_DEBUG_LOG("Receive credential success but join failure\n");
    }
  }
  return status;
}

sl_status_t sli_wifi_connect(sl_wifi_interface_t interface,
                             const sl_wifi_client_configuration_t *ap,
                             uint32_t timeout_ms)
{
  sl_status_t status;
  sli_wifi_request_scan_t scan_request;
  sli_wifi_request_eap_config_t eap_req;
  sli_wifi_join_request_t join_request;
  sl_wifi_buffer_t *buffer              = NULL;
  const sl_wifi_system_packet_t *packet = NULL;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (interface & SL_WIFI_AP_INTERFACE) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  SL_WIFI_ARGS_CHECK_NULL_POINTER(ap);

  status = sli_configure_scan_request(ap, &scan_request, interface);
  VERIFY_STATUS_AND_RETURN(status);

  if (advanced_scan_configuration.active_channel_time != SL_WIFI_DEFAULT_ACTIVE_CHANNEL_SCAN_TIME) {
    status =
      sli_wifi_configure_timeout(SLI_WIFI_CHANNEL_ACTIVE_SCAN_TIMEOUT, advanced_scan_configuration.active_channel_time);
    VERIFY_STATUS_AND_RETURN(status);
  }

  status = sli_wifi_send_command(SLI_WIFI_REQ_SCAN,
                                 SLI_WIFI_WLAN_CMD,
                                 &scan_request,
                                 sizeof(scan_request),
                                 SLI_WIFI_WAIT_FOR(60000),
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);

  if ((SL_WIFI_WPA_ENTERPRISE == ap->security) || (SL_WIFI_WPA2_ENTERPRISE == ap->security)
      || (SL_WIFI_WPA3_ENTERPRISE == ap->security) || (SL_WIFI_WPA3_TRANSITION_ENTERPRISE == ap->security)) {
    status = sli_handle_enterprise_security(ap, &eap_req);
    VERIFY_STATUS_AND_RETURN(status);
  } else if ((SL_WIFI_WPA == ap->security) || (SL_WIFI_WPA2 == ap->security) || (SL_WIFI_WPA_WPA2_MIXED == ap->security)
             || (SL_WIFI_WPA3 == ap->security) || (SL_WIFI_WPA3_TRANSITION == ap->security)) {
    status = sli_handle_psk_security(ap);
    VERIFY_STATUS_AND_RETURN(status);
  } else if (SL_WIFI_WEP == ap->security) {
    return SL_STATUS_NOT_SUPPORTED;
  } else if (SL_WIFI_OPEN != ap->security) {
    return SL_STATUS_WIFI_UNKNOWN_SECURITY_TYPE;
  }

  memset(&join_request, 0, sizeof(join_request));

  status = sli_wifi_get_configured_join_request(SL_WIFI_CLIENT_INTERFACE, ap, &join_request);
  VERIFY_STATUS_AND_RETURN(status);

  status = sli_wifi_send_command(SLI_WIFI_REQ_JOIN,
                                 SLI_WIFI_WLAN_CMD,
                                 &join_request,
                                 sizeof(join_request),
                                 timeout_ms ? SLI_WIFI_WAIT_FOR_RESPONSE(timeout_ms) : SLI_WIFI_RETURN_IMMEDIATELY,
                                 NULL,
                                 (void **)&buffer);

  if (timeout_ms != 0 && status != SL_STATUS_OK) {
    if (buffer != NULL) {
      sli_wifi_memory_manager_free_buffer(buffer);
    }
    sl_status_t temp_status = sli_wifi_send_command(SLI_WIFI_REQ_INIT,
                                                    SLI_WIFI_WLAN_CMD,
                                                    NULL,
                                                    0,
                                                    SLI_WIFI_WAIT_FOR_COMMAND_SUCCESS,
                                                    NULL,
                                                    NULL);
    VERIFY_STATUS_AND_RETURN(temp_status);
  }

  VERIFY_STATUS_AND_RETURN(status);

  packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);

  if (packet->data[0] != 'C') {
    sli_wifi_memory_manager_free_buffer(buffer);
    return SL_STATUS_NOT_AVAILABLE;
  }

  sli_wifi_memory_manager_free_buffer(buffer);
  return SL_STATUS_OK;
}
/***************************************************************************/ /**
 * @brief
 *   to enable/disable ht capabilities
 * @pre Pre-conditions:
 * - @ref sl_wifi_init should be called before this API.
 * @param[in] interface
 *   Wi-Fi Access Point interface as identified by @ref sl_wifi_interface_t
 *  @param[in] HtCaps
 * HtCaps is identified by @ref sli_wifi_request_ap_high_throughput_capability_t
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 * @note 
 *   Client interfaces are not supported.
 ******************************************************************************/
static sl_status_t sli_wifi_set_high_throughput_capability(sl_wifi_interface_t interface,
                                                           sli_wifi_request_ap_high_throughput_capability_t HtCaps)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (interface & SL_WIFI_CLIENT_INTERFACE) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  sl_status_t status = SL_STATUS_OK;
  status             = sli_wifi_send_command(SLI_WIFI_REQ_HT_CAPABILITIES,
                                 SLI_WIFI_WLAN_CMD,
                                 &HtCaps,
                                 sizeof(sli_wifi_request_ap_high_throughput_capability_t),
                                 SLI_WIFI_RSP_HT_CAPABILITIES_WAIT_TIME,
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

static sl_status_t sli_handle_standard_scan(sl_wifi_interface_t interface,
                                            const sl_wifi_ssid_t *optional_ssid,
                                            const sl_wifi_scan_configuration_t *configuration)
{
  sli_wifi_request_scan_t scan_request = { 0 };

  //! copying SSID & length
  if (optional_ssid != NULL) {
    memcpy(scan_request.ssid, optional_ssid->value, optional_ssid->length);
  }

  sli_configure_channel_bitmap(interface, configuration, &scan_request);

  if (configuration->type == SL_WIFI_SCAN_TYPE_PASSIVE) {
    scan_request.pscan_bitmap[3] |= PASSIVE_SCAN_ENABLE;
  }
  if (configuration->lp_mode) {
    scan_request.pscan_bitmap[3] |= LP_CHAIN_ENABLE;
  }

  sl_wifi_max_tx_power_t wifi_max_tx_power = sli_get_max_tx_power();
  // Within the 1-byte scan_feature_bimap variable, last 5 bits(bit 3 through bit 7) are allocated for
  // encoding the transmit power during scan procedure.
  scan_request.scan_feature_bitmap = (uint8_t)(wifi_max_tx_power.scan_tx_power << 3);

  // Enable Quick Scan, if SSID and channel are available
  // Quick Scan is disabled, if channel is 0
  if ((optional_ssid != NULL) && (scan_request.channel[0] != 0)) {
    scan_request.scan_feature_bitmap |= QUICK_SCAN_ENABLE;
  }
  // Configure active scan timeout only if active_channel_time is set to a non-default and non-zero value.
  if (advanced_scan_configuration.active_channel_time != SL_WIFI_DEFAULT_ACTIVE_CHANNEL_SCAN_TIME
      && advanced_scan_configuration.active_channel_time != 0) {
    sl_status_t status =
      sli_wifi_configure_timeout(SLI_WIFI_CHANNEL_ACTIVE_SCAN_TIMEOUT, advanced_scan_configuration.active_channel_time);
    VERIFY_STATUS_AND_RETURN(status);
  }

  if (SL_WIFI_SCAN_TYPE_EXTENDED == configuration->type) {
    scan_request.scan_feature_bitmap |= SCAN_RESULTS_TO_HOST;
  }

  sli_wifi_flush_scan_results_database();

  return sli_wifi_send_command(SLI_WIFI_REQ_SCAN,
                               SLI_WIFI_WLAN_CMD,
                               &scan_request,
                               sizeof(scan_request),
                               SLI_WIFI_RETURN_IMMEDIATELY,
                               NULL,
                               NULL);
}

static sl_status_t sli_handle_background_scan(const sl_wifi_scan_configuration_t *configuration)
{
  sli_wifi_request_bg_scan_t scan_request = { 0 };

  scan_request.bgscan_enable            = SLI_WIFI_BG_SCAN_ENABLE;
  scan_request.enable_instant_bgscan    = advanced_scan_configuration.enable_instant_scan;
  scan_request.passive_scan_duration    = advanced_scan_configuration.passive_channel_time;
  scan_request.active_scan_duration     = advanced_scan_configuration.active_channel_time;
  scan_request.bgscan_periodicity       = (uint16_t)configuration->periodic_scan_interval;
  scan_request.rssi_tolerance_threshold = (uint16_t)advanced_scan_configuration.trigger_level_change;
  scan_request.bgscan_threshold         = (uint16_t)(-1 * advanced_scan_configuration.trigger_level);
  scan_request.multi_probe              = advanced_scan_configuration.enable_multi_probe;

  return sli_wifi_send_command(SLI_WIFI_REQ_BG_SCAN,
                               SLI_WIFI_WLAN_CMD,
                               &scan_request,
                               sizeof(sli_wifi_request_bg_scan_t),
                               SLI_WIFI_RETURN_IMMEDIATELY,
                               NULL,
                               NULL);
}

sl_wifi_interface_t sli_wifi_get_default_interface(void)
{
  return default_interface;
}

void sli_wifi_set_default_interface(sl_wifi_interface_t interface)
{
  default_interface = interface;
}

sl_status_t sli_wifi_set_antenna(sl_wifi_interface_t interface, sl_wifi_antenna_t antenna)
{
  UNUSED_PARAMETER(interface);
  sl_status_t status;
  sli_wifi_antenna_select_t rsi_antenna = { 0 };

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  // Antenna type
  rsi_antenna.antenna_value = (antenna == SL_WIFI_ANTENNA_EXTERNAL) ? SL_WIFI_SELECT_EXTERNAL_ANTENNA
                                                                    : SL_WIFI_SELECT_INTERNAL_ANTENNA;

  status = sli_wifi_send_command(SLI_WIFI_REQ_ANTENNA_SELECT,
                                 SLI_WIFI_COMMON_CMD,
                                 &rsi_antenna,
                                 sizeof(rsi_antenna),
                                 SLI_WIFI_RSP_ANTENNA_SELECT_WAIT_TIME,
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_wait_for_scan_results(sl_wifi_scan_result_t **scan_results, uint32_t max_scan_result_count)
{
  UNUSED_PARAMETER(scan_results);
  UNUSED_PARAMETER(max_scan_result_count);

#ifdef SI91x_ENABLE_WAIT_ON_RESULTS
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  sl_status_t status;
  sl_wifi_system_packet_t *buffer;
  sl_wifi_system_packet_t *packet;

  status = sl_si91x_driver_wait_for_response(SLI_WIFI_RSP_SCAN, SLI_WIFI_WAIT_FOR(3000));
  VERIFY_STATUS_AND_RETURN(status);

  status = sl_si91x_host_remove_from_queue(SI91X_WLAN_RESPONSE_QUEUE, &buffer);
  VERIFY_STATUS_AND_RETURN(status);

  packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  status = sli_wifi_convert_and_save_firmware_status(sli_get_si91x_frame_status(packet));
  VERIFY_STATUS_AND_RETURN(status);

  if (packet->command == SLI_WIFI_RSP_SCAN) {
    *scan_results = (sl_wifi_scan_result_t *)malloc(packet->length);
    if (scan_results == NULL) {
      sli_wifi_memory_manager_free_buffer(buffer);
      return SL_STATUS_ALLOCATION_FAILED;
    }
    memcpy(*scan_results, packet->data, packet->length);
  }
  sli_wifi_memory_manager_free_buffer(buffer);
#endif

  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sli_wifi_start_scan(sl_wifi_interface_t interface,
                                const sl_wifi_ssid_t *optional_ssid,
                                const sl_wifi_scan_configuration_t *configuration)
{
  sl_status_t status = SL_STATUS_FAIL;

  // if ((interface & SL_WIFI_5GHZ_INTERFACE) || (interface & SL_WIFI_DUAL_INTERFACE)) {
  //   return SL_STATUS_NOT_SUPPORTED;
  // }

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  if (SL_WIFI_SCAN_TYPE_ADV_SCAN != configuration->type) {
    status = sli_handle_standard_scan(interface, optional_ssid, configuration);
  } else {
    status = sli_handle_background_scan(configuration);
  }

  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_set_advanced_client_configuration(sl_wifi_interface_t interface,
                                                       const sl_wifi_advanced_client_configuration_t *configuration)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (interface & SL_WIFI_AP_INTERFACE) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  //Check if provided client interface is up or not
  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  sli_wifi_rejoin_params_t rejoin_request = { .max_retry_attempts      = configuration->max_retry_attempts,
                                              .scan_interval           = configuration->scan_interval,
                                              .beacon_missed_count     = configuration->beacon_missed_count,
                                              .first_time_retry_enable = configuration->first_time_retry_enable };

  if (rejoin_request.beacon_missed_count < 40) {
    SL_DEBUG_LOG("\r\nBeacon Missed Count minimum value should be 40, Updating to the minimum value.\r\n");
    rejoin_request.beacon_missed_count = 40;
  }

  sl_status_t status = sli_wifi_send_command(SLI_WIFI_REQ_REJOIN_PARAMS,
                                             SLI_WIFI_WLAN_CMD,
                                             &rejoin_request,
                                             sizeof(rejoin_request),
                                             SLI_WIFI_RSP_REJOIN_PARAMS_WAIT_TIME,
                                             NULL,
                                             NULL);
  VERIFY_STATUS_AND_RETURN(status);

  memcpy(&advanced_client_configuration, configuration, sizeof(sl_wifi_advanced_client_configuration_t));

  return status;
}

sl_status_t sli_wifi_get_signal_strength(sl_wifi_interface_t interface, int32_t *rssi)
{
  sl_status_t status;
  sl_wifi_buffer_t *buffer = NULL;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  SL_VERIFY_POINTER_OR_RETURN(rssi, SL_STATUS_WIFI_NULL_PTR_ARG);

  if (interface & SL_WIFI_AP_INTERFACE) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  status = sli_wifi_send_command(SLI_WIFI_REQ_RSSI,
                                 SLI_WIFI_WLAN_CMD,
                                 NULL,
                                 0,
                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WIFI_RSP_RSSI_WAIT_TIME),
                                 NULL,
                                 (void **)&buffer);
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_wifi_memory_manager_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);
  const sl_wifi_system_packet_t *packet =
    (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data((void *)buffer, 0, NULL);
  *rssi = -(packet->data[0]);
  sli_wifi_memory_manager_free_buffer(buffer);
  return SL_STATUS_OK;
}

sl_status_t sli_wifi_get_sta_tsf(sl_wifi_interface_t interface, sl_wifi_tsf64_t *tsf)
{
  sl_status_t status;
  sl_wifi_buffer_t *buffer = NULL;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  SL_VERIFY_POINTER_OR_RETURN(tsf, SL_STATUS_WIFI_NULL_PTR_ARG);

  if (interface & SL_WIFI_AP_INTERFACE) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  status = sli_wifi_send_command(SLI_WIFI_REQ_TSF,
                                 SLI_WIFI_WLAN_CMD,
                                 NULL,
                                 0,
                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WIFI_RSP_TSF_WAIT_TIME),
                                 NULL,
                                 (void **)&buffer);
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_wifi_memory_manager_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);
  const sl_wifi_system_packet_t *packet =
    (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data((void *)buffer, 0, NULL);
  memcpy(tsf, packet->data, packet->length);
  sli_wifi_memory_manager_free_buffer(buffer);
  return SL_STATUS_OK;
}

sl_status_t sli_wifi_set_mac_address(sl_wifi_interface_t interface, const sl_mac_address_t *mac_address)
{
  UNUSED_PARAMETER(interface);

  SL_VERIFY_POINTER_OR_RETURN(mac_address, SL_STATUS_NULL_POINTER);

  sl_status_t status = sli_wifi_send_command(SLI_WIFI_REQ_SET_MAC_ADDRESS,
                                             SLI_WIFI_WLAN_CMD,
                                             mac_address,
                                             sizeof(sl_mac_address_t),
                                             SLI_WIFI_RSP_SET_MAC_ADDRESS_WAIT_TIME,
                                             NULL,
                                             NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}
sl_status_t sli_wifi_get_mac_address(sl_wifi_interface_t interface, sl_mac_address_t *mac)
{
  sl_status_t status;
  sl_wifi_buffer_t *buffer = NULL;

  SL_WIFI_ARGS_CHECK_NULL_POINTER(mac);

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  status = sli_wifi_send_command(SLI_WIFI_REQ_MAC_ADDRESS,
                                 SLI_WIFI_WLAN_CMD,
                                 NULL,
                                 0,
                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WIFI_RSP_MAC_ADDRESS_WAIT_TIME),
                                 NULL,
                                 (void **)&buffer);
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_wifi_memory_manager_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);
  const sl_wifi_system_packet_t *packet =
    (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data((void *)buffer, 0, NULL);
  if (packet->length > 0) {
    // In Concurrent mode, for Client Interface, mac address will be at offset 0
    uint8_t mac_address_offset = 0;
    if ((SL_WIFI_CONCURRENT_MODE == sli_wifi_get_opermode()) && (SL_WIFI_AP_INTERFACE == interface)) {
      // In Concurrent mode, for AP Interface, mac address will be at offset 6
      mac_address_offset = sizeof(sl_mac_address_t);
    }
    memcpy(mac->octet, packet->data + mac_address_offset, sizeof(*mac));
  }
  sli_wifi_memory_manager_free_buffer(buffer);
  return SL_STATUS_OK;
}

sl_status_t sli_wifi_set_channel(sl_wifi_interface_t interface, sl_wifi_channel_t channel)
{
  UNUSED_PARAMETER(channel);
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  // Firmware doesn't support to change the channel of a running Access Point interface.
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sli_wifi_set_max_tx_power(sl_wifi_interface_t interface, sl_wifi_max_tx_power_t max_tx_power)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (
    (interface & SL_WIFI_CLIENT_INTERFACE) // For the client interface, it’s necessary to check if the interface is up. However, for the AP interface, this check isn’t required since this API will be called before starting the AP.
    && (!sli_wifi_is_interface_up(interface))) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  uint8_t max_scan_tx_power = max_tx_power.scan_tx_power;
  uint8_t max_join_tx_power = max_tx_power.join_tx_power;

  if ((max_scan_tx_power < 1 || max_scan_tx_power > 31) || (max_join_tx_power < 1 || max_join_tx_power > 31)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_save_max_tx_power(max_scan_tx_power, max_join_tx_power);
  return SL_STATUS_OK;
}

sl_status_t sli_wifi_get_max_tx_power(sl_wifi_interface_t interface, sl_wifi_max_tx_power_t *max_tx_power)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  SL_WIFI_ARGS_CHECK_NULL_POINTER(max_tx_power);

  *max_tx_power = sli_get_max_tx_power();

  return SL_STATUS_OK;
}

sl_status_t sli_wifi_config_pll_mode(sl_wifi_pll_mode_t pll_mode)
{
  if (device_initialized) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  // Validate PLL mode
  if ((pll_mode != SL_WIFI_PLL_MODE_20MHZ) && (pll_mode != SL_WIFI_PLL_MODE_40MHZ)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sli_wifi_save_pll_mode(pll_mode);
  return SL_STATUS_OK;
}

sl_status_t sli_wifi_config_power_chain(sl_wifi_power_chain_t power_chain)
{
  if (device_initialized) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  // Validate power chain
  if (power_chain != SL_WIFI_HP_CHAIN && power_chain != SL_WIFI_LP_CHAIN) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_wifi_save_power_chain(power_chain);
  return SL_STATUS_OK;
}

sl_status_t sli_wifi_start_ap(sl_wifi_interface_t interface, const sl_wifi_ap_configuration_t *configuration)
{
  sl_status_t status                           = SL_STATUS_OK;
  sl_wifi_buffer_t *rx_buffer                  = NULL;
  const sl_wifi_system_packet_t *join_response = NULL;
  sli_wifi_ap_config_request request           = { 0 };
  sli_wifi_join_request_t join_request         = { 0 };
  sl_wifi_credential_t cred                    = { 0 };

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if ((configuration->client_idle_timeout) > UINT8_MAX) {
    return SL_STATUS_INVALID_PARAMETER;
  }

#if defined(SLI_SI917)
  if (!(interface & SL_WIFI_2_4GHZ_INTERFACE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
#endif

  memcpy(request.ssid, configuration->ssid.value, configuration->ssid.length);
  request.security_type = (uint8_t)configuration->security;

  // Skip credential retrieval if the security mode is open
  if (configuration->security != SL_WIFI_OPEN) {
    status = sli_wifi_host_get_credentials(configuration->credential_id, SL_WIFI_PSK_CREDENTIAL, &cred);
    VERIFY_STATUS_AND_RETURN(status);
    memcpy(request.psk, cred.psk.value, sizeof(cred.psk.value));
  }
  if (request.security_type != SL_WIFI_OPEN) {
    memcpy(request.psk, cred.psk.value, sizeof(request.psk));
  }
  //This encryption convertions is only required in access point mode
  status = sli_wifi_get_nwp_encryption(configuration->encryption, &request.encryption_mode);
  VERIFY_STATUS_AND_RETURN(status);

  //Configures TDI[7-4] value here
  if ((configuration->security == SL_WIFI_WPA3) || (configuration->security == SL_WIFI_WPA3_TRANSITION)) {
    request.encryption_mode |= configuration->tdi_flags & 0xF0;
  }

  request.channel         = configuration->channel.channel;
  request.beacon_interval = configuration->beacon_interval;
  request.dtim_period     = configuration->dtim_beacon_count;
  request.max_sta_support = configuration->maximum_clients;
  if (configuration->keepalive_type) {
    request.options             = (configuration->keepalive_type & SLI_WIFI_AP_OPT_KEEPALIVE_TYPE_MASK);
    request.ap_keepalive_period = (uint8_t)configuration->client_idle_timeout;
  }
  if (configuration->beacon_stop) {
    // if beacon_stop is set, use 2nd bit to indicate that Beacon Stop is enabled.
    request.options |= SLI_WIFI_AP_OPT_BEACON_STOP;
  }

  // options in struct sl_wifi_ap_configuration_t is used only for HIDDEN SSID right now, so check only bit 1.
  if (configuration->options & SL_WIFI_HIDDEN_SSID) {
    // if HIDDEN_SSID is set, use 3rd bit of options var in sli_wifi_ap_config_request to indicate that dynamic configuration of Hidden SSID is enabled.
    request.options |= SLI_WIFI_AP_OPT_DYNAMIC_HIDDEN_SSID_CONF;
  }

  status = sli_wifi_send_command(SLI_WIFI_REQ_AP_CONFIGURATION,
                                 SLI_WIFI_WLAN_CMD,
                                 &request,
                                 sizeof(request),
                                 SLI_WIFI_WAIT_FOR(15000),
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);

  if (configuration->is_11n_enabled) {
    sli_wifi_request_ap_high_throughput_capability_t htcaps = { 0 };
    htcaps.mode_11n_enable                                  = true;
    htcaps.ht_caps_bitmap = SL_WIFI_HT_CAPS_NUM_RX_STBC | SL_WIFI_HT_CAPS_SHORT_GI_20MHZ | SL_WIFI_HT_CAPS_GREENFIELD_EN
#if !defined(SLI_SI917)
                            | SL_WIFI_HT_CAPS_SUPPORT_CH_WIDTH
#endif
      ;
    status = sli_wifi_set_high_throughput_capability(SL_WIFI_AP_INTERFACE, htcaps);
    VERIFY_STATUS_AND_RETURN(status);
  }

  status = sli_wifi_get_configured_join_request(SL_WIFI_AP_INTERFACE, configuration, &join_request);
  VERIFY_STATUS_AND_RETURN(status);

  status = sli_wifi_send_command(SLI_WIFI_REQ_JOIN,
                                 SLI_WIFI_WLAN_CMD,
                                 &join_request,
                                 sizeof(join_request),
                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WIFI_RSP_JOIN_WAIT_TIME),
                                 NULL,
                                 (void **)&rx_buffer);
  if ((status != SL_STATUS_OK) && (rx_buffer != NULL)) {
    sli_wifi_memory_manager_free_buffer(rx_buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);

  join_response = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data((void *)rx_buffer, 0, NULL);

  if (join_response->data[0] != 'G') {
    sli_wifi_memory_manager_free_buffer(rx_buffer);
    return SL_STATUS_NOT_AVAILABLE;
  }

  sli_wifi_save_ap_configuration(configuration);
  if (interface == SL_WIFI_AP_DUAL_INTERFACE)
    interface_is_up[SL_WIFI_AP_DUAL_INTERFACE_INDEX] = true;
  else if (interface == SL_WIFI_AP_5GHZ_INTERFACE)
    interface_is_up[SL_WIFI_AP_5GHZ_INTERFACE_INDEX] = true;
  else if (interface == SL_WIFI_AP_2_4GHZ_INTERFACE)
    interface_is_up[SL_WIFI_AP_2_4GHZ_INTERFACE_INDEX] = true;

  sli_wifi_memory_manager_free_buffer(rx_buffer);
  return SL_STATUS_OK;
}

sl_status_t sli_wifi_get_pairwise_master_key(sl_wifi_interface_t interface,
                                             const uint8_t type,
                                             const sl_wifi_ssid_t *ssid,
                                             const char *pre_shared_key,
                                             uint8_t *pairwise_master_key)
{
  UNUSED_PARAMETER(interface);
  sl_status_t status;
  sl_wifi_buffer_t *buffer                           = NULL;
  sli_wifi_request_psk_t pairwise_master_key_request = { 0 };

  if (pairwise_master_key == NULL) {
    return SL_STATUS_WIFI_NULL_PTR_ARG;
  }

  pairwise_master_key_request.type = type;
  memcpy(pairwise_master_key_request.psk_or_pmk, pre_shared_key, sl_strlen((char *)pre_shared_key));
  memcpy(pairwise_master_key_request.ap_ssid, ssid->value, ssid->length);

  status = sli_wifi_send_command(SLI_WIFI_REQ_HOST_PSK,
                                 SLI_WIFI_WLAN_CMD,
                                 &pairwise_master_key_request,
                                 sizeof(sli_wifi_request_psk_t),
                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WIFI_RSP_HOST_PSK_WAIT_TIME),
                                 NULL,
                                 (void **)&buffer);
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_wifi_memory_manager_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);
  const sl_wifi_system_packet_t *packet =
    (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data((void *)buffer, 0, NULL);
  memcpy(pairwise_master_key, packet->data, packet->length);
  sli_wifi_memory_manager_free_buffer(buffer);
  return SL_STATUS_OK;
}

sl_status_t sli_wifi_disconnect_ap_client(sl_wifi_interface_t interface,
                                          const sl_mac_address_t *mac,
                                          sl_wifi_deauth_reason_t reason)
{
  UNUSED_PARAMETER(reason);
  SL_WIFI_ARGS_CHECK_NULL_POINTER(mac);

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  sli_wifi_disassociation_request_t disconnect_request = { 0 };

  disconnect_request.mode_flag = SL_WIFI_AP_VAP_ID;
  memcpy(&disconnect_request.client_mac_address, mac, sizeof(sl_mac_address_t));

  sl_status_t status = sli_wifi_send_command(SLI_WIFI_REQ_DISCONNECT,
                                             SLI_WIFI_WLAN_CMD,
                                             &disconnect_request,
                                             sizeof(disconnect_request),
                                             SLI_WLAN_RSP_DISCONNECT_WAIT_TIME,
                                             (void *)mac,
                                             NULL);

  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_get_ap_client_info(sl_wifi_interface_t interface, sl_wifi_client_info_response_t *client_info)
{
  sl_status_t status;
  sl_wifi_buffer_t *buffer = NULL;
  const sl_wifi_system_packet_t *packet;
  sli_wifi_client_info_response sli_wifi_client_info_response = { 0 };

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (interface & SL_WIFI_CLIENT_INTERFACE) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (!sli_wifi_is_interface_up(SL_WIFI_AP_INTERFACE)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  SL_WIFI_ARGS_CHECK_NULL_POINTER(client_info);

  status = sli_wifi_send_command(SLI_WIFI_REQ_QUERY_GO_PARAMS,
                                 SLI_WIFI_WLAN_CMD,
                                 NULL,
                                 0,
                                 SLI_WIFI_RSP_QUERY_GO_PARAMS_WAIT_TIME,
                                 NULL,
                                 (void **)&buffer);
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_wifi_memory_manager_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);

  packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data((void *)buffer, 0, NULL);

  memcpy(&sli_wifi_client_info_response, packet->data, sizeof(sli_wifi_client_info_response));
  sli_wifi_convert_client_info(client_info, &sli_wifi_client_info_response);

  sli_wifi_memory_manager_free_buffer(buffer);
  return status;
}
sl_status_t sli_wifi_disconnect(sl_wifi_interface_t interface)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (interface & SL_WIFI_AP_INTERFACE) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  sli_wifi_disassociation_request_t disconnect_request = { 0 };

  sl_status_t status = sli_wifi_send_command(SLI_WIFI_REQ_DISCONNECT,
                                             SLI_WIFI_WLAN_CMD,
                                             &disconnect_request,
                                             sizeof(disconnect_request),
                                             SLI_WLAN_RSP_DISCONNECT_WAIT_TIME,
                                             NULL,
                                             NULL);
  VERIFY_STATUS_AND_RETURN(status);

  sli_wifi_reset_coex_current_performance_profile();

  status =
    sli_wifi_send_command(SLI_WIFI_REQ_INIT, SLI_WIFI_WLAN_CMD, NULL, 0, SLI_WIFI_RSP_INIT_WAIT_TIME, NULL, NULL);
  VERIFY_STATUS_AND_RETURN(status);

  return status;
}

sl_status_t sli_wifi_stop_ap(sl_wifi_interface_t interface)
{
  sl_status_t status                                   = SL_STATUS_OK;
  sli_wifi_disassociation_request_t disconnect_request = { 0 };

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (interface & SL_WIFI_CLIENT_INTERFACE) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  disconnect_request.mode_flag = SL_WIFI_AP_VAP_ID;
  status                       = sli_wifi_send_command(SLI_WIFI_REQ_AP_STOP,
                                 SLI_WIFI_WLAN_CMD,
                                 &disconnect_request,
                                 sizeof(disconnect_request),
                                 SLI_WIFI_RSP_AP_STOP_WAIT_TIME,
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);

  sli_wifi_reset_ap_configuration();
  if (interface == SL_WIFI_AP_DUAL_INTERFACE)
    interface_is_up[SL_WIFI_AP_DUAL_INTERFACE_INDEX] = false;
  else if (interface == SL_WIFI_AP_5GHZ_INTERFACE)
    interface_is_up[SL_WIFI_AP_5GHZ_INTERFACE_INDEX] = false;
  else if (interface == SL_WIFI_AP_2_4GHZ_INTERFACE)
    interface_is_up[SL_WIFI_AP_2_4GHZ_INTERFACE_INDEX] = false;

  return status;
}

sl_status_t sli_wifi_get_statistics(sl_wifi_interface_t interface, sl_wifi_statistics_t *statistics)
{
  sl_status_t status       = SL_STATUS_OK;
  sl_wifi_buffer_t *buffer = NULL;
  const sl_wifi_system_packet_t *packet;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  SL_WIFI_ARGS_CHECK_INVALID_INTERFACE(interface);
  SL_WIFI_ARGS_CHECK_NULL_POINTER(statistics);

  status = sli_wifi_send_command(SLI_WIFI_REQ_EXT_STATS,
                                 SLI_WIFI_WLAN_CMD,
                                 NULL,
                                 0,
                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WIFI_RSP_EXT_STATS_WAIT_TIME),
                                 NULL,
                                 (void **)&buffer);
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_wifi_memory_manager_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);

  packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data((void *)buffer, 0, NULL);
  if (packet->length != sizeof(sl_wifi_statistics_t)) {
    sli_wifi_memory_manager_free_buffer(buffer);
    return SL_STATUS_FAIL;
  }

  if (packet->length > 0) {
    memcpy(statistics, packet->data, packet->length);
  }

  sli_wifi_memory_manager_free_buffer(buffer);
  return status;
}
sl_status_t sli_wifi_get_operational_statistics(sl_wifi_interface_t interface,
                                                sl_wifi_operational_statistics_t *operational_statistics)
{
  sl_status_t status       = SL_STATUS_OK;
  sl_wifi_buffer_t *buffer = NULL;
  const sl_wifi_system_packet_t *packet;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  SL_WIFI_ARGS_CHECK_INVALID_INTERFACE(interface);
  SL_WIFI_ARGS_CHECK_NULL_POINTER(operational_statistics);

  status = sli_wifi_send_command(SLI_WIFI_REQ_GET_STATS,
                                 SLI_WIFI_WLAN_CMD,
                                 NULL,
                                 0,
                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WIFI_RSP_GET_STATS_WAIT_TIME),
                                 NULL,
                                 (void **)&buffer);
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_wifi_memory_manager_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);

  packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data((void *)buffer, 0, NULL);
  if (packet->length != sizeof(sl_wifi_operational_statistics_t)) {
    sli_wifi_memory_manager_free_buffer(buffer);
    return SL_STATUS_FAIL;
  }

  if (packet->length > 0) {
    memcpy(operational_statistics, packet->data, packet->length);
  }

  sli_wifi_memory_manager_free_buffer(buffer);
  return status;
}

sl_status_t sli_wifi_transmit_test_start(sl_wifi_interface_t interface,
                                         const sl_wifi_transmitter_test_info_t *test_tx_info)
{
  UNUSED_PARAMETER(interface);
  // Check if the operation mode is PER imode
  if (sli_wifi_get_opermode() != SL_WIFI_TRANSMIT_TEST_MODE) {
    return SL_STATUS_INVALID_MODE;
  }
  sl_status_t status = SL_STATUS_OK;
  status             = sli_wifi_send_command(SLI_WIFI_REQ_TX_TEST_MODE,
                                 SLI_WIFI_WLAN_CMD,
                                 test_tx_info,
                                 sizeof(sl_wifi_request_tx_test_info_t),
                                 SLI_WIFI_WAIT_FOR(30100),
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_transmit_test_stop(sl_wifi_interface_t interface)
{
  UNUSED_PARAMETER(interface);
  sl_status_t status = SL_STATUS_OK;
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  // Check if the operation mode is PER imode
  if (sli_wifi_get_opermode() != SL_WIFI_TRANSMIT_TEST_MODE) {
    return SL_STATUS_INVALID_MODE;
  }

  sl_wifi_request_tx_test_info_t tx_test_info = { 0 };
  tx_test_info.enable                         = 0;
  // Send the transmit test stop command
  status = sli_wifi_send_command(SLI_WIFI_REQ_TX_TEST_MODE,
                                 SLI_WIFI_WLAN_CMD,
                                 &tx_test_info,
                                 sizeof(sl_wifi_request_tx_test_info_t),
                                 SLI_WIFI_WAIT_FOR(30100),
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_frequency_offset(sl_wifi_interface_t interface, const sl_wifi_freq_offset_t *frequency_calibration)
{
  UNUSED_PARAMETER(interface);
  sl_status_t status = SL_STATUS_OK;
  SL_VERIFY_POINTER_OR_RETURN(frequency_calibration, SL_STATUS_NULL_POINTER);
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  status = sli_wifi_send_command(SLI_WIFI_REQ_FREQ_OFFSET,
                                 SLI_WIFI_WLAN_CMD,
                                 frequency_calibration,
                                 sizeof(sl_wifi_freq_offset_t),
                                 SLI_WIFI_WAIT_FOR_RESPONSE(35000),
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_dpd_calibration(sl_wifi_interface_t interface, const sl_wifi_dpd_calib_data_t *dpd_calib_data)
{
  UNUSED_PARAMETER(interface);
  sl_status_t status = SL_STATUS_OK;
  SL_VERIFY_POINTER_OR_RETURN(dpd_calib_data, SL_STATUS_NULL_POINTER);
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  status = sli_wifi_send_command(SLI_WIFI_REQ_GET_DPD_DATA,
                                 SLI_WIFI_WLAN_CMD,
                                 dpd_calib_data,
                                 sizeof(sl_wifi_dpd_calib_data_t),
                                 SLI_WIFI_WAIT_FOR_COMMAND_SUCCESS,
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}
sl_status_t sli_wifi_start_statistic_report(sl_wifi_interface_t interface, sl_wifi_channel_t channel)
{
  UNUSED_PARAMETER(interface);
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  sl_status_t status                   = SL_STATUS_OK;
  sli_wifi_request_rx_stats_t rx_stats = { 0 };

  // Configure to start RX stats
  rx_stats.start[0] = SLI_WIFI_START_STATISTICS_REPORT;
  // Copy the channel number
  memcpy(rx_stats.channel, &channel.channel, sizeof(rx_stats.channel));
  status = sli_wifi_send_command(SLI_WIFI_REQ_RX_STATS,
                                 SLI_WIFI_WLAN_CMD,
                                 &rx_stats,
                                 sizeof(rx_stats),
                                 SLI_WIFI_RETURN_IMMEDIATELY,
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_stop_statistic_report(sl_wifi_interface_t interface)
{
  UNUSED_PARAMETER(interface);
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sli_wifi_request_rx_stats_t rx_stats = { 0 };

  // Configure to stop RX stats
  rx_stats.start[0] = SLI_WIFI_STOP_STATISTICS_REPORT;

  sl_status_t status = sli_wifi_send_command(SLI_WIFI_REQ_RX_STATS,
                                             SLI_WIFI_WLAN_CMD,
                                             &rx_stats,
                                             sizeof(rx_stats),
                                             SLI_WIFI_RETURN_IMMEDIATELY,
                                             NULL,
                                             NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

void sli_wifi_deinit(void)
{
  bg_enabled = false;
  sli_wifi_reset_coex_current_performance_profile();
  sli_wifi_reset_max_tx_power();
  sli_wifi_reset_ap_configuration();
  sli_wifi_reset_sl_wifi_rate();
  memset(&advanced_scan_configuration, 0, sizeof(sl_wifi_advanced_scan_configuration_t));
  sli_wifi_flush_scan_results_database();
  return;
}

sl_status_t sli_wifi_set_transmit_rate(sl_wifi_interface_t interface,
                                       sl_wifi_rate_protocol_t rate_protocol,
                                       sl_wifi_rate_t mask)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (sli_wifi_is_interface_up(interface)) {
    if (interface == SL_WIFI_AP_INTERFACE) {
      return SL_STATUS_WIFI_UNSUPPORTED;
    }
  } else {
    if (interface == SL_WIFI_CLIENT_INTERFACE) {
      return SL_STATUS_WIFI_INTERFACE_NOT_UP;
    }
  }

  switch (rate_protocol) {
    case SL_WIFI_RATE_PROTOCOL_B_ONLY: {
      if (mask < SL_WIFI_RATE_11B_MIN || mask > SL_WIFI_RATE_11B_MAX) {
        return SL_STATUS_INVALID_CONFIGURATION;
      }
      break;
    }
    case SL_WIFI_RATE_PROTOCOL_G_ONLY: {
      if (mask < SL_WIFI_RATE_11G_MIN || mask > SL_WIFI_RATE_11G_MAX) {
        return SL_STATUS_INVALID_CONFIGURATION;
      }
      break;
    }
    case SL_WIFI_RATE_PROTOCOL_N_ONLY: {
      if (mask < SL_WIFI_RATE_11N_MIN || mask > SL_WIFI_RATE_11N_MAX) {
        return SL_STATUS_INVALID_CONFIGURATION;
      }
      break;
    }
    case SL_WIFI_RATE_PROTOCOL_AC_ONLY: {
      return SL_STATUS_NOT_SUPPORTED;
    }
    case SL_WIFI_RATE_PROTOCOL_AX_ONLY: {
      if (mask < SL_WIFI_RATE_11AX_MIN || mask > SL_WIFI_RATE_11AX_MAX) {
        return SL_STATUS_INVALID_CONFIGURATION;
      }
      break;
    }
    case SL_WIFI_RATE_PROTOCOL_AUTO: {
      if (mask != SL_WIFI_AUTO_RATE) {
        return SL_STATUS_INVALID_CONFIGURATION;
      }
      break;
    }
    default: {
      return SL_STATUS_INVALID_CONFIGURATION;
    }
  }

  return sli_wifi_save_rate(mask);
}

sl_status_t sli_wifi_get_transmit_rate(sl_wifi_interface_t interface,
                                       sl_wifi_rate_protocol_t *rate_protocol,
                                       sl_wifi_rate_t *mask)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  SL_WIFI_ARGS_CHECK_NULL_POINTER(rate_protocol);
  SL_WIFI_ARGS_CHECK_NULL_POINTER(mask);

  sl_wifi_rate_t transfer_rate;
  sli_wifi_get_saved_rate(&transfer_rate);

  return sli_wifi_get_rate_protocol_and_data_rate((uint8_t)transfer_rate, rate_protocol, mask);
}

sl_status_t sli_wifi_get_ap_client_count(sl_wifi_interface_t interface, uint32_t *client_list_count)
{
  sl_wifi_client_info_response_t client_info = { 0 };

  SL_WIFI_ARGS_CHECK_NULL_POINTER(client_list_count);

  sl_status_t status = sli_wifi_get_ap_client_info(interface, &client_info);

  if (status == SL_STATUS_OK) {
    *client_list_count = 0;
    memcpy(client_list_count, (uint32_t *)&client_info.client_count, sizeof(uint8_t));
  }

  return status;
}

sl_status_t sli_wifi_get_ap_client_list(sl_wifi_interface_t interface,
                                        uint16_t client_list_count,
                                        sl_mac_address_t *client_list)
{
  sl_wifi_client_info_response_t client_info = { 0 };

  sl_status_t status = sli_wifi_get_ap_client_info(interface, &client_info);

  if (status == SL_STATUS_OK) {
    for (uint16_t station_info_index = 0;
         station_info_index < client_info.client_count && station_info_index < client_list_count;
         station_info_index++) {
      memcpy(client_list[station_info_index].octet,
             client_info.client_info[station_info_index].mac_adddress.octet,
             sizeof(sl_mac_address_t));
    }
  }

  return status;
}

sl_status_t sli_wifi_generate_wps_pin(sl_wifi_wps_pin_t *wps_pin)
{
  sl_status_t status                               = SL_STATUS_OK;
  sl_wifi_buffer_t *buffer                         = NULL;
  const sl_wifi_system_packet_t *packet            = NULL;
  sli_wifi_wps_method_request_t wps_method_request = { 0 };

  SL_WIFI_ARGS_CHECK_NULL_POINTER(wps_pin);

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  // Set configuration to generate new WPS pin
  wps_method_request.wps_method   = SLI_WIFI_SET_WPS_METHOD_PIN;
  wps_method_request.generate_pin = SLI_WIFI_SET_WPS_GENERATE_PIN;

  status = sli_wifi_send_command(SLI_WIFI_REQ_WPS_METHOD,
                                 SLI_WIFI_WLAN_CMD,
                                 &wps_method_request,
                                 sizeof(sli_wifi_wps_method_request_t),
                                 SLI_WIFI_RSP_WPS_METHOD_WAIT_TIME,
                                 NULL,
                                 (void **)&buffer);
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_wifi_memory_manager_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);

  packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data((void *)buffer, 0, NULL);
  if (packet->length > 0) {
    memcpy(wps_pin->digits, packet->data, sizeof(sl_wifi_wps_pin_t));
  }

  sli_wifi_memory_manager_free_buffer(buffer);
  return status;
}

sl_status_t sli_wifi_start_wps(sl_wifi_interface_t interface,
                               sl_wifi_wps_mode_t mode,
                               const sl_wifi_wps_pin_t *optional_wps_pin)
{
  UNUSED_PARAMETER(optional_wps_pin);

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (mode != SL_WIFI_WPS_PUSH_BUTTON_MODE || (interface & SL_WIFI_AP_INTERFACE) == 0) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  sl_status_t status;
  sli_wifi_join_request_t wps_button_press_request = { 0 };
  sl_wifi_ap_configuration_t ap_configuration      = { 0 };

  sli_wifi_get_saved_ap_configuration(&ap_configuration);
  status = sli_wifi_get_configured_join_request(SL_WIFI_AP_INTERFACE, &ap_configuration, &wps_button_press_request);

  VERIFY_STATUS_AND_RETURN(status);
  status = sli_wifi_send_command(SLI_WIFI_REQ_JOIN,
                                 SLI_WIFI_WLAN_CMD,
                                 &wps_button_press_request,
                                 sizeof(wps_button_press_request),
                                 SLI_WIFI_RSP_JOIN_WAIT_TIME,
                                 NULL,
                                 NULL);

  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_start_wps_v2(sl_wifi_interface_t interface,
                                  sl_wifi_wps_config_t config,
                                  sl_wifi_wps_response_t *response)
{
  sli_wifi_wps_config_t wps_config = { 0 };
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (config.mode != SL_WIFI_WPS_PUSH_BUTTON_MODE || (interface & SL_WIFI_CLIENT_INTERFACE) == 0
      || (config.role != SL_WIFI_WPS_ENROLLEE_ROLE)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  if ((config.auto_connect == false || config.auto_connect == true) && response == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status = SL_STATUS_FAIL;

  wps_config.wps_method   = (uint16_t)config.mode;
  wps_config.auto_connect = config.auto_connect ? 1 : 0;
  status                  = sli_wifi_send_command(SLI_WIFI_REQ_WPS_METHOD,
                                 SLI_WIFI_WLAN_CMD,
                                 &wps_config,
                                 sizeof(sli_wifi_wps_config_t),
                                 SLI_WIFI_RSP_WPS_METHOD_WAIT_TIME,
                                 NULL,
                                 NULL);
  SL_DEBUG_LOG("status of wps method req : 0x%lx\n", status);
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = sli_wifi_wps_connect(wps_config, response);
  SL_DEBUG_LOG("status of wps connect API : 0x%lx\n", status);
  return status;
}

sl_status_t sli_wifi_set_roam_configuration(sl_wifi_interface_t interface,
                                            const sl_wifi_roam_configuration_t *roam_configuration)
{
  sli_wifi_request_roam_params_t roam_param_request = { 0 };

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (interface & SL_WIFI_AP_INTERFACE) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  if (roam_configuration->trigger_level != SL_WIFI_NEVER_ROAM) {
    roam_param_request.roam_enable = ALWAYS_ROAM;
  }

  roam_param_request.roam_threshold  = -1 * roam_configuration->trigger_level;
  roam_param_request.roam_hysteresis = roam_configuration->trigger_level_change;

  sl_status_t status = sli_wifi_send_command(SLI_WIFI_REQ_ROAM_PARAMS,
                                             SLI_WIFI_WLAN_CMD,
                                             &roam_param_request,
                                             sizeof(sli_wifi_request_roam_params_t),
                                             SLI_WIFI_RSP_ROAM_PARAMS_WAIT_TIME,
                                             NULL,
                                             NULL);

  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_set_advanced_scan_configuration(const sl_wifi_advanced_scan_configuration_t *configuration)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(configuration);

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  memcpy(&advanced_scan_configuration, configuration, sizeof(sl_wifi_advanced_scan_configuration_t));

  return SL_STATUS_OK;
}

sl_status_t sli_wifi_get_advanced_scan_configuration(sl_wifi_advanced_scan_configuration_t *configuration)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(configuration);

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  memcpy(configuration, &advanced_scan_configuration, sizeof(sl_wifi_advanced_scan_configuration_t));

  return SL_STATUS_OK;
}

sl_status_t sli_wifi_stop_scan(sl_wifi_interface_t interface)
{
  sl_status_t status                      = SL_STATUS_OK;
  sli_wifi_request_bg_scan_t scan_request = { 0 };

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (interface & SL_WIFI_AP_INTERFACE) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  // Once stop_scan() support for foreground scan is available, "bg_enabled" should be removed.
  if (bg_enabled == true) {
    scan_request.bgscan_enable = SLI_WIFI_BG_SCAN_DISABLE;
    status                     = sli_wifi_send_command(SLI_WIFI_REQ_BG_SCAN,
                                   SLI_WIFI_WLAN_CMD,
                                   &scan_request,
                                   sizeof(sli_wifi_request_bg_scan_t),
                                   SLI_WIFI_RSP_BG_SCAN_WAIT_TIME,
                                   NULL,
                                   NULL);
    VERIFY_STATUS_AND_RETURN(status);
    bg_enabled = false;
  } else {
    return SL_STATUS_NOT_SUPPORTED;
  }

  return status;
}

sl_status_t sli_wifi_get_ap_configuration(sl_wifi_interface_t interface, sl_wifi_ap_configuration_t *configuration)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  SL_VERIFY_POINTER_OR_RETURN(configuration, SL_STATUS_NULL_POINTER);

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  sl_wifi_ap_configuration_t saved_ap_configuration = { 0 };
  sli_wifi_get_saved_ap_configuration(&saved_ap_configuration);

  memcpy(configuration, &saved_ap_configuration, sizeof(sl_wifi_ap_configuration_t));

  return SL_STATUS_OK;
}

sl_status_t sli_wifi_reconfigure_ap(sl_wifi_interface_t interface, sl_wifi_ap_reconfiguration_t config)
{
  sl_status_t status = SL_STATUS_OK;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (interface & SL_WIFI_CLIENT_INTERFACE) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  status = sli_wifi_send_command(SLI_WIFI_REQ_BEACON_STOP,
                                 SLI_WIFI_WLAN_CMD,
                                 &config,
                                 sizeof(sl_wifi_ap_reconfiguration_t),
                                 SLI_WIFI_RSP_BEACON_STOP_WAIT_TIME,
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);

  return status;
}
sl_status_t sli_wifi_test_client_configuration(sl_wifi_interface_t interface,
                                               const sl_wifi_client_configuration_t *ap,
                                               uint32_t timeout_ms)
{
  sl_status_t status = sl_wifi_connect(interface, ap, timeout_ms);
  VERIFY_STATUS_AND_RETURN(status);

  status = sli_wifi_disconnect(SL_WIFI_CLIENT_INTERFACE);
  VERIFY_STATUS_AND_RETURN(status);

  return SL_STATUS_OK;
}

sl_status_t sli_wifi_enable_target_wake_time(const sl_wifi_twt_request_t *twt_req)
{
  if (!twt_req->twt_enable) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (twt_req->twt_flow_id > MAX_FLOW_ID) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (twt_req->wake_int_exp > MAX_WAKE_INTERVAL_EXPONENT) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (twt_req->wake_int_exp_tol > MAX_WAKE_INTERVAL_EXPONENT_TOLERANCE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (twt_req->wake_duration_unit > MAX_WAKE_DURATION_UNIT) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (twt_req->un_announced_twt > 1 || twt_req->triggered_twt > 1 || twt_req->restrict_tx_outside_tsp > 1) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (twt_req->twt_retry_limit > MAX_TWT_RETRY_LIMIT) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (twt_req->twt_retry_interval < MIN_TWT_RETRY_INTERVAL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (twt_req->req_type > MAX_TWT_REQ_TYPE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if ((twt_req->req_type != REQUEST_TWT)
      && (twt_req->wake_duration == 0 || twt_req->wake_int_mantissa == 0
          || ((uint32_t)twt_req->wake_duration
              * (uint32_t)(twt_req->wake_duration_unit ? TWT_WAKE_DURATION_UNIT_1024TU : TWT_WAKE_DURATION_UNIT_256TU))
               > ((twt_req->wake_int_mantissa) * ((uint64_t)1 << twt_req->wake_int_exp)))) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if ((twt_req->twt_channel != 0) || (twt_req->twt_protection != 0) || (twt_req->implicit_twt != 1)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sl_status_t status = sli_wifi_send_command(SLI_WIFI_REQ_TWT_PARAMS,
                                             SLI_WIFI_WLAN_CMD,
                                             twt_req,
                                             sizeof(sl_wifi_twt_request_t),
                                             SLI_WIFI_RSP_TWT_WAIT_TIME,
                                             NULL,
                                             NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_target_wake_time_auto_selection(sl_wifi_twt_selection_t *twt_auto_request)
{
  if ((twt_auto_request->twt_enable != 0) && (twt_auto_request->twt_enable != 1)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if ((twt_auto_request->twt_enable == 1)
      && ((twt_auto_request->rx_latency > MAX_TX_AND_RX_LATENCY_LIMIT)
          || (twt_auto_request->tx_latency > MAX_TX_AND_RX_LATENCY_LIMIT))) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  twt_auto_request->rx_latency = (twt_auto_request->rx_latency == 0) ? 2000 : twt_auto_request->rx_latency;
  if ((twt_auto_request->rx_latency < 2000)
      || (twt_auto_request->average_tx_throughput > (DEVICE_AVERAGE_THROUGHPUT / 2))) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if ((twt_auto_request->tx_latency < 200) && (twt_auto_request->tx_latency != 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sl_status_t status = sli_wifi_send_command(SLI_WIFI_REQ_TWT_AUTO_CONFIG,
                                             SLI_WIFI_WLAN_CMD,
                                             twt_auto_request,
                                             sizeof(sl_wifi_twt_selection_t),
                                             SLI_WIFI_RSP_TWT_WAIT_TIME,
                                             NULL,
                                             NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_disable_target_wake_time(const sl_wifi_twt_request_t *twt_req)
{
  if ((!twt_req->twt_enable) && ((twt_req->twt_flow_id == 0xFF) || (twt_req->twt_flow_id <= 7))) {
    sl_status_t status = sli_wifi_send_command(SLI_WIFI_REQ_TWT_PARAMS,
                                               SLI_WIFI_WLAN_CMD,
                                               twt_req,
                                               sizeof(sl_wifi_twt_request_t),
                                               SLI_WIFI_RSP_TWT_WAIT_TIME,
                                               NULL,
                                               NULL);
    VERIFY_STATUS_AND_RETURN(status);
    return status;
  }

  return SL_STATUS_INVALID_PARAMETER;
}

sl_status_t sli_wifi_reschedule_twt(uint8_t flow_id,
                                    sl_wifi_reschedule_twt_action_t twt_action,
                                    uint64_t suspend_duration)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (flow_id > MAX_FLOW_ID) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if ((twt_action == SL_WIFI_SUSPEND_INDEFINITELY || twt_action == SL_WIFI_RESUME_IMMEDIATELY)
      && suspend_duration > 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (twt_action == SL_WIFI_SUSPEND_FOR_DURATION
      && (suspend_duration < 1 || suspend_duration > MAX_TWT_SUSPEND_DURATION)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (twt_action != SL_WIFI_SUSPEND_INDEFINITELY && twt_action != SL_WIFI_SUSPEND_FOR_DURATION
      && twt_action != SL_WIFI_RESUME_IMMEDIATELY) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_wifi_reschedule_twt_config_t reschedule_twt_config = { 0 };
  reschedule_twt_config.flow_id                         = flow_id;
  reschedule_twt_config.twt_action                      = twt_action;
  reschedule_twt_config.suspend_duration                = suspend_duration;
  sl_status_t status                                    = sli_wifi_send_command(SLI_WIFI_REQ_RESCHEDULE_TWT,
                                             SLI_WIFI_WLAN_CMD,
                                             &reschedule_twt_config,
                                             sizeof(sl_wifi_reschedule_twt_config_t),
                                             SLI_WIFI_RSP_TWT_WAIT_TIME,
                                             NULL,
                                             NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_filter_broadcast(uint16_t beacon_drop_threshold,
                                      uint8_t filter_bcast_in_tim,
                                      uint8_t filter_bcast_tim_till_next_cmd)
{
  sl_status_t status                                       = SL_STATUS_OK;
  sli_wifi_request_wlan_filter_broadcast_t sl_filter_bcast = { 0 };
  sl_filter_bcast.beacon_drop_threshold[0]                 = beacon_drop_threshold & 0x00FF;
  sl_filter_bcast.beacon_drop_threshold[1]                 = (beacon_drop_threshold >> 8) & 0x00FF;
  sl_filter_bcast.filter_bcast_in_tim                      = filter_bcast_in_tim;
  sl_filter_bcast.filter_bcast_tim_till_next_cmd           = filter_bcast_tim_till_next_cmd;

  status = sli_wifi_send_command(SLI_WIFI_REQ_FILTER_BCAST_PACKETS,
                                 SLI_WIFI_WLAN_CMD,
                                 &sl_filter_bcast,
                                 sizeof(sli_wifi_request_wlan_filter_broadcast_t),
                                 SLI_WIFI_RSP_FILTER_BCAST_PACKETS_WAIT_TIME,
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_update_gain_table(uint8_t band, uint8_t bandwidth, const uint8_t *payload, uint16_t payload_length)
{
#define SLI_MAX_GAIN_TABLE_SIZE_WITH_TB_ONLY 128

  sl_status_t status            = SL_STATUS_OK;
  uint16_t su_tb_payload_length = 0;
  if ((payload == NULL) || (payload_length == 0)) {
    return SL_STATUS_INVALID_CONFIGURATION;
  }

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (payload_length > SLI_MAX_GAIN_TABLE_SIZE_WITH_TB_ONLY) {
    SL_DEBUG_LOG("Wrong user gain table used\r\n");
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint8_t *su_tb_payload = malloc(
    SLI_MAX_GAIN_TABLE_SIZE_WITH_SU_TB); // Allocate memory for gain table with separate gain value entries for 11ax SU and TB

  if (su_tb_payload == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  memset(su_tb_payload, 0, SLI_MAX_GAIN_TABLE_SIZE_WITH_SU_TB);
  status =
    sli_wifi_convert_old_gain_table_to_su_gain_table(payload, su_tb_payload, payload_length, &su_tb_payload_length);
  if (status != SL_STATUS_OK) {
    free(su_tb_payload);
    return status;
  }
  sli_wifi_gain_table_info_t *gain_table_info = malloc(sizeof(sli_wifi_gain_table_info_t) + su_tb_payload_length);
  if (gain_table_info == NULL) {
    free(su_tb_payload);
    return SL_STATUS_ALLOCATION_FAILED;
  }
  memset(gain_table_info, 0, sizeof(sli_wifi_gain_table_info_t) + su_tb_payload_length);
  gain_table_info->band               = band;
  gain_table_info->bandwidth          = bandwidth;
  gain_table_info->size               = su_tb_payload_length;
  gain_table_info->x_offset           = 0;
  gain_table_info->y_offset           = 0;
  gain_table_info->gain_table_version = 1;
  gain_table_info->reserved           = 0;

  memcpy(gain_table_info->gain_table, su_tb_payload, su_tb_payload_length);

  status = sli_wifi_send_command(SLI_WIFI_REQ_GAIN_TABLE,
                                 SLI_WIFI_WLAN_CMD,
                                 gain_table_info,
                                 sizeof(sli_wifi_gain_table_info_t) + (gain_table_info->size),
                                 SLI_WIFI_RSP_GAIN_TABLE_WAIT_TIME,
                                 NULL,
                                 NULL);
  free(su_tb_payload);
  free(gain_table_info);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_update_su_gain_table(uint8_t band,
                                          uint8_t bandwidth,
                                          const uint8_t *payload,
                                          uint16_t payload_length,
                                          uint8_t x_offset,
                                          uint8_t y_offset)
{
  sl_status_t status = SL_STATUS_OK;

  if ((payload == NULL) || (payload_length == 0)) {
    return SL_STATUS_INVALID_CONFIGURATION;
  }

  if (payload_length > SLI_MAX_GAIN_TABLE_SIZE_WITH_SU_TB) {
    SL_DEBUG_LOG("Wrong user gain table used\r\n");
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sli_wifi_gain_table_info_t *gain_table_info = malloc(sizeof(sli_wifi_gain_table_info_t) + payload_length);
  if (gain_table_info == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  memset(gain_table_info, 0, sizeof(sli_wifi_gain_table_info_t) + payload_length);
  gain_table_info->band               = band;
  gain_table_info->bandwidth          = bandwidth;
  gain_table_info->size               = payload_length;
  gain_table_info->x_offset           = x_offset;
  gain_table_info->y_offset           = y_offset;
  gain_table_info->gain_table_version = 1;
  gain_table_info->reserved           = 0;

  memcpy(gain_table_info->gain_table, payload, payload_length);
  status = sli_wifi_send_command(SLI_WIFI_REQ_GAIN_TABLE,
                                 SLI_WIFI_WLAN_CMD,
                                 gain_table_info,
                                 sizeof(sli_wifi_gain_table_info_t) + (gain_table_info->size),
                                 SLI_WIFI_RSP_GAIN_TABLE_WAIT_TIME,
                                 NULL,
                                 NULL);
  free(gain_table_info);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_set_11ax_config(uint8_t guard_interval)
{
#if !(SLI_SI91X_CONFIG_WIFI6_PARAMS)
  UNUSED_PARAMETER(guard_interval);
  return SL_STATUS_NOT_SUPPORTED;
#else
  sl_status_t status                               = SL_STATUS_OK;
  sli_wifi_11ax_config_params_t config_11ax_params = { 0 };
  config_11ax_params.guard_interval                = guard_interval;
  config_11ax_params.nominal_pe                    = SLI_NOMINAL_PE;
  config_11ax_params.dcm_enable                    = SLI_DCM_ENABLE;
  config_11ax_params.ldpc_enable                   = SLI_LDPC_ENABLE;
  config_11ax_params.ng_cb_enable                  = SLI_NG_CB_ENABLE;
  config_11ax_params.ng_cb_values                  = SLI_NG_CB_VALUES;
  config_11ax_params.uora_enable                   = SLI_UORA_ENABLE;
  config_11ax_params.trigger_rsp_ind               = SLI_TRIGGER_RESP_IND;
  config_11ax_params.ipps_valid_value              = SLI_IPPS_VALID_VALUE;
  config_11ax_params.tx_only_on_ap_trig            = SLI_TX_ONLY_ON_AP_TRIG;
  config_11ax_params.twt_support                   = SLI_SI91X_ENABLE_TWT_FEATURE;
  config_11ax_params.config_er_su                  = SLI_CONFIG_ER_SU;
  config_11ax_params.beamformee_support            = SLI_ENABLE_BEAMFORMEE_SUPPORT;

  status = sli_wifi_send_command(SLI_WIFI_REQ_11AX_PARAMS,
                                 SLI_WIFI_WLAN_CMD,
                                 &config_11ax_params,
                                 sizeof(config_11ax_params),
                                 SLI_WIFI_RSP_11AX_PARAMS_WAIT_TIME,
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
#endif
}

sl_status_t sli_wifi_transceiver_set_channel(sl_wifi_interface_t interface, sl_wifi_transceiver_set_channel_t channel)
{
  sl_status_t status = SL_STATUS_OK;
  sl_wifi_operation_mode_t opermode;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  opermode = sli_wifi_get_opermode();
  if (opermode != SL_WIFI_TRANSCEIVER_MODE) {
    SL_DEBUG_LOG("Invalid mode: %d. Command only supported in Wi-Fi transceiver opermode(7)\r\n", opermode);
    return SL_STATUS_SI91X_COMMAND_GIVEN_IN_INVALID_STATE;
  }

  if ((channel.chan_info.channel < 1) || (channel.chan_info.channel > 14)) {
    return SL_STATUS_TRANSCEIVER_INVALID_CHANNEL;
  }

  if (interface & SL_WIFI_2_4GHZ_INTERFACE) {
    channel.chan_info.band      = SL_WIFI_BAND_2_4GHZ;
    channel.chan_info.bandwidth = SL_WIFI_BANDWIDTH_20MHz;
  }

  else if (interface & SL_WIFI_5GHZ_INTERFACE) {
    channel.chan_info.band      = SL_WIFI_BAND_5GHZ;
    channel.chan_info.bandwidth = SL_WIFI_BANDWIDTH_20MHz;
  }

  else if (interface & SL_WIFI_DUAL_INTERFACE) {
    channel.chan_info.band      = SL_WIFI_BAND_DUAL;
    channel.chan_info.bandwidth = SL_WIFI_BANDWIDTH_20MHz;
  }

  status = sli_wifi_send_command(SLI_WIFI_REQ_SET_TRANSCEIVER_CHANNEL,
                                 SLI_WIFI_WLAN_CMD,
                                 &channel,
                                 sizeof(channel),
                                 SLI_WIFI_RSP_SET_TRANSCEIVER_CHANNEL_WAIT_TIME,
                                 NULL,
                                 NULL);

  VERIFY_STATUS_AND_RETURN(status);

  if (interface == SL_WIFI_TRANSCEIVER_INTERFACE) {
    interface_is_up[SL_WIFI_TRANSCEIVER_INTERFACE_INDEX] = true;
  }

  return status;
}

sl_status_t sli_wifi_set_transceiver_parameters(sl_wifi_interface_t interface, sl_wifi_transceiver_parameters_t *params)
{
  sl_status_t status                    = SL_STATUS_OK;
  sl_wifi_operation_mode_t opermode     = 0;
  sl_wifi_buffer_t *buffer              = NULL;
  const sl_wifi_system_packet_t *packet = NULL;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  opermode = sli_wifi_get_opermode();
  if (opermode != SL_WIFI_TRANSCEIVER_MODE) {
    SL_DEBUG_LOG("Invalid mode: %d. Command only supported in Wi-Fi transceiver opermode(7)\r\n", opermode);
    return SL_STATUS_SI91X_COMMAND_GIVEN_IN_INVALID_STATE;
  }

  SL_VERIFY_POINTER_OR_RETURN(params, SL_STATUS_NULL_POINTER);

  if (params->set) {
    //! Transceiver configurations shall not be changed dynamically
    if (sli_wifi_is_interface_up(interface)) {
      return SL_STATUS_SI91X_COMMAND_GIVEN_IN_INVALID_STATE;
    }

    if ((!params->retransmit_count) || (params->retransmit_count > MAX_RETRANSMIT_COUNT)) {
      return SL_STATUS_TRANSCEIVER_INVALID_CONFIG;
    }
    for (uint8_t i = 0; i < 4; i++) {
      if ((params->cw_params[i].cwmin > MAX_CW_EXPN_COUNT) || (params->cw_params[i].cwmax > MAX_CW_EXPN_COUNT)
          || (params->cw_params[i].aifsn > MAX_AIFSN)) {
        return SL_STATUS_TRANSCEIVER_INVALID_CONFIG;
      }
    }
  }

  status = sli_wifi_send_command(SLI_WIFI_REQ_TRANSCEIVER_CONFIG_PARAMS,
                                 SLI_WIFI_WLAN_CMD,
                                 params,
                                 sizeof(sl_wifi_transceiver_parameters_t),
                                 params->set
                                   ? SLI_WIFI_RSP_TRANSCEIVER_CONFIG_PARAMS_WAIT_TIME
                                   : SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WIFI_RSP_TRANSCEIVER_CONFIG_PARAMS_WAIT_TIME),
                                 NULL,
                                 (void **)&buffer);

  //! Return if API called to set params. Otherwise, if API called to get params, continue further to copy params received from firmware.
  if (params->set) {
    return status;
  }

  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_wifi_memory_manager_free_buffer(buffer);
  }

  VERIFY_STATUS_AND_RETURN(status);

  packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data((void *)buffer, 0, NULL);
  if (packet->length > 0) {
    memcpy(params, packet->data, sizeof(sl_wifi_transceiver_parameters_t));
  }

  sli_wifi_memory_manager_free_buffer(buffer);
  return status;
}

sl_status_t sli_wifi_transceiver_up(sl_wifi_interface_t interface, sl_wifi_transceiver_configuration_t *config)
{
  sl_status_t status = SL_STATUS_OK;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  SL_VERIFY_POINTER_OR_RETURN(config, SL_STATUS_NULL_POINTER);

  status = sli_wifi_set_transceiver_parameters(interface, &config->parameters);
  if (status != SL_STATUS_OK) {
    SL_DEBUG_LOG("\r\nSet transceiver config params failed, error code : 0x%lX\r\n", status);
    return status;
  } else {
    SL_DEBUG_LOG("\r\nSet transceiver config params successful");
  }

  // Update params.set = 0 to fetch transceiver config params
  memset(&config->parameters, 0, sizeof(sl_wifi_transceiver_parameters_t));
  status = sli_wifi_set_transceiver_parameters(interface, &config->parameters);
  if (status != SL_STATUS_OK) {
    SL_DEBUG_LOG("\r\nGet transceiver config params failed, error code : 0x%lX\r\n", status);
  } else {
    SL_DEBUG_LOG("\r\nTransceiver config params:");
    SL_DEBUG_LOG("\r\nRetransmit count: %d", config->parameters.retransmit_count);
    for (uint8_t i = 0; i < 4; i++) {
      SL_DEBUG_LOG("\r\nAC index[%d] - cwmin: %d, cwmax: %d, aifsn: %d",
                   i,
                   config->parameters.cw_params[i].cwmin,
                   config->parameters.cw_params[i].cwmax,
                   config->parameters.cw_params[i].aifsn);
    }
  }

  // Set transceiver mode channel
  status = sli_wifi_transceiver_set_channel(interface, config->channel);
  if (status != SL_STATUS_OK) {
    SL_DEBUG_LOG("\r\nSet Channel Failed, Error Code : 0x%lX\r\n", status);
    return status;
  } else {
    SL_DEBUG_LOG("\r\nSet Channel(%d) Initialization success\r\n", config->channel.chan_info.channel);
  }

  // Get DUT MAC address to use as Addr2/Transmitter Addresss
  status = sli_wifi_get_mac_address(interface, &config->dut_mac);
  if (status == SL_STATUS_OK) {
    SL_DEBUG_LOG("\r\nDevice MAC address: %x:%x:%x:%x:%x:%x\r\n",
                 config->dut_mac.octet[0],
                 config->dut_mac.octet[1],
                 config->dut_mac.octet[2],
                 config->dut_mac.octet[3],
                 config->dut_mac.octet[4],
                 config->dut_mac.octet[5]);
  } else {
    SL_DEBUG_LOG("\r\nFailed to get mac address: 0x%lX\r\n", status);
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t sli_wifi_update_transceiver_peer_list(sl_wifi_interface_t interface, sl_wifi_transceiver_peer_update_t peer)
{
  sl_status_t status = SL_STATUS_OK;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  if ((peer.flags & TRANSCEIVER_PEER_ADD_FLAG) && (peer.flags & TRANSCEIVER_PEER_AUTO_RATE_FLAG)
      && (!peer.peer_supported_rate_bitmap)) {
    return SL_STATUS_TRANSCEIVER_INVALID_DATA_RATE;
  }

  if (IS_MAC_ZERO(peer.peer_mac_address)) {
    return SL_STATUS_TRANSCEIVER_INVALID_MAC_ADDRESS;
  }
  if (IS_BCAST_MCAST_MAC(peer.peer_mac_address[0])) {
    return SL_STATUS_TRANSCEIVER_INVALID_MAC_ADDRESS;
  }

  status = sli_wifi_send_command(SLI_WIFI_REQ_TRANSCEIVER_PEER_LIST_UPDATE,
                                 SLI_WIFI_WLAN_CMD,
                                 &peer,
                                 sizeof(sl_wifi_transceiver_peer_update_t),
                                 SLI_WIFI_RSP_TRANSCEIVER_PEER_LIST_UPDATE_WAIT_TIME,
                                 NULL,
                                 NULL);
  return status;
}

sl_status_t sli_wifi_set_transceiver_multicast_filter(sl_wifi_interface_t interface,
                                                      sl_wifi_transceiver_mcast_filter_t mcast)
{
  sl_status_t status = SL_STATUS_OK;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }
  if (mcast.flags & TRANSCEIVER_MCAST_FILTER_EN) {
    if ((!mcast.num_of_mcast_addr) || mcast.num_of_mcast_addr > TRANSCEIVER_MCAST_FILTER_ADDR_LIMIT) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    for (uint8_t i = 0; i < mcast.num_of_mcast_addr; i++) {
      if (IS_MAC_ZERO(mcast.mac[i])) {
        return SL_STATUS_TRANSCEIVER_INVALID_MAC_ADDRESS;
      }
    }
  }

  status = sli_wifi_send_command(SLI_WIFI_REQ_SET_TRANSCEIVER_MCAST_FILTER,
                                 SLI_WIFI_WLAN_CMD,
                                 &mcast,
                                 sizeof(sl_wifi_transceiver_mcast_filter_t),
                                 SLI_WIFI_RSP_SET_TRANSCEIVER_MCAST_FILTER_WAIT_TIME,
                                 NULL,
                                 NULL);
  return status;
}

sl_status_t sli_wifi_flush_transceiver_data(sl_wifi_interface_t interface)
{
  sl_status_t status = SL_STATUS_OK;
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }
  status = sli_wifi_send_command(SLI_WIFI_REQ_TRANSCEIVER_FLUSH_DATA_Q,
                                 SLI_WIFI_WLAN_CMD,
                                 NULL,
                                 0,
                                 SLI_WIFI_RSP_TRANSCEIVER_FLUSH_DATA_Q_WAIT_TIME,
                                 NULL,
                                 NULL);

  return status;
}

sl_status_t sli_wifi_configure_multicast_filter(sl_wifi_multicast_filter_info_t *multicast_filter_info)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (NULL == multicast_filter_info) {
    return SL_STATUS_NULL_POINTER;
  }
  uint16_t multicast_bitmap = 0;
  multicast_bitmap          = (uint16_t)multicast_filter_info->command_type;

  if ((multicast_filter_info->command_type == SL_WIFI_MULTICAST_MAC_ADD_BIT)
      || (multicast_filter_info->command_type == SL_WIFI_MULTICAST_MAC_CLEAR_BIT)) {
    multicast_bitmap |= (sli_multicast_mac_hash((uint8_t *)&(multicast_filter_info->mac_address)) << 8);
  }

  sl_status_t status = SL_STATUS_OK;

  status = sli_wifi_send_command(SLI_WIFI_REQ_SET_MULTICAST_FILTER,
                                 SLI_WIFI_WLAN_CMD,
                                 &multicast_bitmap,
                                 sizeof(multicast_bitmap),
                                 SLI_WIFI_RSP_SET_MULTICAST_FILTER_WAIT_TIME,
                                 NULL,
                                 NULL);

  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_get_mfp(sl_wifi_interface_t interface, sl_wifi_mfp_mode_t *config)
{
  // only supported in STA mode
  if ((interface & SL_WIFI_CLIENT_INTERFACE) == 0 || (interface & SL_WIFI_AP_INTERFACE) != 0) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Parameter validation
  SL_WIFI_ARGS_CHECK_NULL_POINTER(config);

  sl_wifi_mfp_config_t mfp_config = sli_wifi_get_mfp_mode();
  // Return current MFP mode
  *config = mfp_config.mfp_mode;
  return SL_STATUS_OK;
}

sl_status_t sli_wifi_set_mfp(sl_wifi_interface_t interface, const sl_wifi_mfp_mode_t config)
{
  // only supported in STA mode
  if ((interface & SL_WIFI_CLIENT_INTERFACE) == 0 || (interface & SL_WIFI_AP_INTERFACE) != 0) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Store the MFP configuration for future join operations
  sl_wifi_mfp_config_t mfp_config = sli_wifi_get_mfp_mode();
  sl_status_t status;
  mfp_config.mfp_mode      = config;
  mfp_config.is_configured = true;
  status                   = sli_wifi_save_mfp_mode(&mfp_config);
  return status;
}

sl_status_t sli_wifi_set_rts_threshold(sl_wifi_interface_t interface, uint16_t rts_threshold)
{
  UNUSED_PARAMETER(interface);

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sli_wifi_config_request_t config_request = { .config_type = SLI_WIFI_CONFIG_RTS_THRESHOLD, .value = rts_threshold };
  sl_status_t status                       = sli_wifi_send_command(SLI_WIFI_REQ_CONFIG,
                                             SLI_WIFI_WLAN_CMD,
                                             &config_request,
                                             sizeof(config_request),
                                             SLI_WIFI_WAIT_FOR_COMMAND_SUCCESS,
                                             NULL,
                                             NULL);
  if (status == SL_STATUS_OK) {
    sli_wifi_save_rts_threshold(rts_threshold);
  }
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_get_rts_threshold(sl_wifi_interface_t interface, uint16_t *rts_threshold)
{
  UNUSED_PARAMETER(interface);
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  SL_WIFI_ARGS_CHECK_NULL_POINTER(rts_threshold);

  *rts_threshold = sli_get_rts_threshold().rts_threshold;

  return SL_STATUS_OK;
}

sl_status_t sli_wifi_transmit_cw_tone_start(sl_wifi_interface_t interface, sl_wifi_cw_tone_config_t cw_tone_config)
{
  sl_status_t status = SL_STATUS_FAIL;
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (!sl_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }
  if (!((default_interface & interface) == interface)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Check if the operation mode is PER imode
  if (sli_wifi_get_opermode() != SL_WIFI_TRANSMIT_TEST_MODE) {
    return SL_STATUS_INVALID_MODE;
  }

  sli_wifi_request_cw_tone_config_t transmit_cw_tone_config = { 0 };

  // Assuming transmit_cw_tone_config is of type sl_wifi_request_cw_tone_config_t
  memcpy(&transmit_cw_tone_config.cw_tone_config, &cw_tone_config, sizeof(sl_wifi_cw_tone_config_t));

  transmit_cw_tone_config.frame_body_type.sub_type = SLI_WIFI_SUBTYPE_TRANSMIT_CW;

  transmit_cw_tone_config.enable = 1;

  status = sli_wifi_send_command(SLI_WIFI_REQ_WIFI_RAIL,
                                 SLI_WIFI_COMMON_CMD,
                                 &transmit_cw_tone_config,
                                 sizeof(sli_wifi_request_cw_tone_config_t),
                                 SLI_WIFI_WAIT_FOR_COMMAND_SUCCESS,
                                 NULL,
                                 NULL);

  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_transmit_cw_tone_stop(sl_wifi_interface_t interface)
{
  sl_status_t status = SL_STATUS_FAIL;
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (!sl_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }
  if (!((default_interface & interface) == interface)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Check if the operation mode is PER mode
  if (sli_wifi_get_opermode() != SL_WIFI_TRANSMIT_TEST_MODE) {
    return SL_STATUS_INVALID_MODE;
  }

  sli_wifi_request_cw_tone_config_t transmit_cw_tone_config = { 0 };

  transmit_cw_tone_config.enable = 0;

  transmit_cw_tone_config.frame_body_type.sub_type = SLI_WIFI_SUBTYPE_TRANSMIT_CW;

  status = sli_wifi_send_command(SLI_WIFI_REQ_WIFI_RAIL,
                                 SLI_WIFI_COMMON_CMD,
                                 &transmit_cw_tone_config,
                                 sizeof(sli_wifi_request_cw_tone_config_t),
                                 SLI_WIFI_WAIT_FOR_COMMAND_SUCCESS,
                                 NULL,
                                 NULL);

  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_set_tx_powerdBm(int16_t txPower)
{
  sl_status_t status = SL_STATUS_FAIL;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sli_wifi_request_tx_power_t tx_power_request = { 0 };
  tx_power_request.Txpower                     = txPower;
  tx_power_request.frame_body_type.sub_type    = SLI_WIFI_SUBTYPE_SET_TX_POWER_DBM;

  status = sli_wifi_send_command(SLI_WIFI_REQ_WIFI_RAIL,
                                 SLI_WIFI_COMMON_CMD,
                                 &tx_power_request,
                                 sizeof(sli_wifi_request_tx_power_t),
                                 SLI_WIFI_WAIT_FOR_COMMAND_SUCCESS,
                                 NULL,
                                 NULL);

  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_stop_rx(sl_wifi_interface_t interface)
{
  sl_status_t status = SL_STATUS_FAIL;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sl_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  if (!((default_interface & interface) == interface)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_wifi_request_stop_rx_t stop_rx_request = { 0 };
  stop_rx_request.frame_body_type.sub_type   = SLI_WIFI_SUBTYPE_RX_STOP;

  status = sli_wifi_send_command(SLI_WIFI_REQ_WIFI_RAIL,
                                 SLI_WIFI_COMMON_CMD,
                                 &stop_rx_request,
                                 sizeof(sli_wifi_request_stop_rx_t),
                                 SLI_WIFI_WAIT_FOR_COMMAND_SUCCESS,
                                 NULL,
                                 NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_config_xo_ctune(sl_wifi_interface_t interface,
                                     sl_wifi_response_get_ctune_data_t *xo_ctune,
                                     uint32_t ctune_data)
{
  sl_status_t status       = SL_STATUS_FAIL;
  sl_wifi_buffer_t *buffer = NULL;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (!sl_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }
  if (!((default_interface & interface) == interface)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sli_wifi_request_configure_xo_ctune_t xo_ctune_request = { 0 };
  xo_ctune_request.ctune_data                            = ctune_data;
  xo_ctune_request.frame_body_type.sub_type              = SLI_WIFI_SUBTYPE_CONFIG_XO_CTUNE;

  status = sli_wifi_send_command(SLI_WIFI_REQ_WIFI_RAIL,
                                 SLI_WIFI_COMMON_CMD,
                                 &xo_ctune_request,
                                 sizeof(sli_wifi_request_configure_xo_ctune_t),
                                 SLI_WIFI_WAIT_FOR_RESPONSE(1000),
                                 NULL,
                                 (void **)&buffer);

  if ((status != SL_STATUS_OK) && (NULL != buffer)) {
    sli_wifi_memory_manager_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);

  const sl_wifi_system_packet_t *packet =
    (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data((void *)buffer, 0, NULL);

  if ((xo_ctune != NULL) && (packet != NULL)) {
    memcpy(xo_ctune, packet->data, packet->length);
    sli_wifi_memory_manager_free_buffer(buffer);
  }

  return status;
}

sl_status_t sli_wifi_read_ctune(sl_wifi_interface_t interface,
                                sl_wifi_response_get_ctune_data_t *get_xo_ctune,
                                const uint32_t *ctune_data)
{
  sl_status_t status       = SL_STATUS_FAIL;
  sl_wifi_buffer_t *buffer = NULL;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (!sl_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }
  if (!((default_interface & interface) == interface)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_wifi_request_get_xo_ctune_t get_ctune_request = { 0 };
  memcpy(get_ctune_request.ctune_data, ctune_data, sizeof(get_ctune_request.ctune_data));
  get_ctune_request.frame_body_type.sub_type = SLI_WIFI_SUBTYPE_GET_XO_CTUNE;

  status = sli_wifi_send_command(SLI_WIFI_REQ_WIFI_RAIL,
                                 SLI_WIFI_COMMON_CMD,
                                 &get_ctune_request,
                                 sizeof(sli_wifi_request_get_xo_ctune_t),
                                 SLI_WIFI_WAIT_FOR_RESPONSE(1000),
                                 NULL,
                                 (void **)&buffer);

  if ((status != SL_STATUS_OK) && (NULL != buffer)) {
    sli_wifi_memory_manager_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);
  const sl_wifi_system_packet_t *packet =
    (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data((void *)buffer, 0, NULL);

  if ((get_xo_ctune != NULL) && (packet != NULL)) {
    memcpy(get_xo_ctune, packet->data, packet->length);
    sli_wifi_memory_manager_free_buffer(buffer);
  }
  return status;
}

sl_status_t sli_wifi_add_vendor_ie(sl_wifi_vendor_ie_t *vendor_ie, uint8_t *fw_unique_id)
{
  sl_status_t status;
  uint16_t ie_buffer_length                  = 0;
  uint16_t ie_buffer_size                    = 0;
  sli_wifi_manage_vendor_ie_packet_t *packet = NULL;
  sl_wifi_buffer_t *buffer                   = NULL;

  // Validate input parameters
  if ((vendor_ie == NULL) || (vendor_ie->ie_buffer == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }

  ie_buffer_length =
    vendor_ie->ie_buffer[1]
    + SLI_WIFI_VENDOR_IE_HEADER_LENGTH;         // Calculate the buffer length from length provided in IE buffer.
  ie_buffer_size = vendor_ie->ie_buffer_length; // Store the original buffer size
  if ((ie_buffer_size > SLI_WIFI_MAX_VENDOR_IE_BUFFER_LENGTH) || (ie_buffer_size < ie_buffer_length)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (vendor_ie->unique_id > SLI_WIFI_MAX_VENDOR_IE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  packet = (sli_wifi_manage_vendor_ie_packet_t *)malloc(sizeof(sli_wifi_manage_vendor_ie_packet_t) + ie_buffer_size);
  if (packet == NULL) {
    return SL_STATUS_FAIL;
  }

  // Prepare the packet to send to firmware
  packet->version           = SLI_WIFI_VENDOR_IE_FRAME_VERSION;
  packet->action            = SLI_WIFI_VENDOR_IE_ACTION_ADD;
  packet->unique_id         = vendor_ie->unique_id;
  packet->mgmt_frame_bitmap = vendor_ie->mgmt_frame_bitmap;
  memset(packet->reserved, 0, sizeof(packet->reserved));
  packet->ie_buffer_length = vendor_ie->ie_buffer_length;
  memcpy(packet->ie_buffer, vendor_ie->ie_buffer, ie_buffer_size);

  // Send the command to firmware
  status = sli_wifi_send_command(SLI_WIFI_REQ_VENDOR_IE,
                                 SLI_WIFI_WLAN_CMD,
                                 packet,
                                 sizeof(sli_wifi_manage_vendor_ie_packet_t) + ie_buffer_size,
                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WIFI_VENDOR_IE_CMD_TIMEOUT),
                                 NULL,
                                 (void **)&buffer);

  free(packet);
  if ((status != SL_STATUS_OK) && (NULL != buffer)) {
    sli_wifi_memory_manager_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);
  const sl_wifi_system_packet_t *resp_packet =
    (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data((void *)buffer, 0, NULL);

  if ((resp_packet != NULL)) {
    *fw_unique_id = resp_packet->data[0];
    SL_DEBUG_LOG("vendor IE added with unique ID: %d\r\n", *fw_unique_id);
    sli_wifi_memory_manager_free_buffer(buffer);
  }

  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_remove_vendor_ie(uint8_t unique_id)
{
  sl_status_t status;
  sli_wifi_manage_vendor_ie_packet_t packet = { 0 };

  // Validate unique_id
  if (unique_id > SLI_WIFI_MAX_VENDOR_IE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Prepare the packet to send to firmware
  packet.version           = SLI_WIFI_VENDOR_IE_FRAME_VERSION;
  packet.action            = SLI_WIFI_VENDOR_IE_ACTION_REMOVE;
  packet.unique_id         = unique_id;
  packet.mgmt_frame_bitmap = 0;
  memset(packet.reserved, 0, sizeof(packet.reserved));
  packet.ie_buffer_length = 0;
  // Not assigning anything to packet.ie_buffer as it has not been allocated any memory.

  // Send the command to firmware
  status = sli_wifi_send_command(SLI_WIFI_REQ_VENDOR_IE,
                                 SLI_WIFI_WLAN_CMD,
                                 &packet,
                                 sizeof(packet),
                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WIFI_VENDOR_IE_CMD_TIMEOUT),
                                 NULL,
                                 NULL);

  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_remove_all_vendor_ie(void)
{
  sl_status_t status;
  sli_wifi_manage_vendor_ie_packet_t packet = { 0 };

  // Prepare the packet to send to firmware
  packet.version           = SLI_WIFI_VENDOR_IE_FRAME_VERSION;
  packet.action            = SLI_WIFI_VENDOR_IE_ACTION_REMOVE_ALL;
  packet.unique_id         = 0;
  packet.mgmt_frame_bitmap = 0;
  memset(packet.reserved, 0, sizeof(packet.reserved));
  packet.ie_buffer_length = 0;
  // Not assigning anything to packet.ie_buffer as it has not been allocated any memory.

  // Send the command to firmware
  status = sli_wifi_send_command(SLI_WIFI_REQ_VENDOR_IE,
                                 SLI_WIFI_WLAN_CMD,
                                 &packet,
                                 sizeof(packet),
                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WIFI_VENDOR_IE_CMD_TIMEOUT),
                                 NULL,
                                 NULL);

  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_wifi_set_device_region(sl_wifi_operation_mode_t operation_mode,
                                       sl_wifi_band_mode_t band,
                                       sl_wifi_region_code_t region_code)
{
  sl_status_t status = SL_STATUS_OK;

  switch (operation_mode) {
    case SL_WIFI_CLIENT_MODE:
    case SL_WIFI_ENTERPRISE_CLIENT_MODE:
    case SL_WIFI_TRANSMIT_TEST_MODE:
    case SL_WIFI_TRANSCEIVER_MODE: {
      // For client and transmit test modes, send a command to set the region code
      sli_wifi_set_region_request_t request = { 0 };

      request.set_region_code_from_user_cmd = SET_REGION_CODE_FROM_USER;
      request.region_code                   = (uint8_t)region_code;

      status = sli_wifi_send_command(SLI_WIFI_REQ_SET_REGION,
                                     SLI_WIFI_WLAN_CMD,
                                     &request,
                                     sizeof(sli_wifi_set_region_request_t),
                                     SLI_WIFI_RSP_SET_REGION_WAIT_TIME,
                                     NULL,
                                     NULL);
      VERIFY_STATUS_AND_RETURN(status);
      break;
    }

    case SL_WIFI_CONCURRENT_MODE:
    case SL_WIFI_ACCESS_POINT_MODE: {
      if (operation_mode == SL_WIFI_CONCURRENT_MODE) {
        // For concurrent mode, send a command to set the region code
        sli_wifi_set_region_request_t request = { 0 };

        request.set_region_code_from_user_cmd = SET_REGION_CODE_FROM_USER;
        request.region_code                   = (uint8_t)region_code;

        status = sli_wifi_send_command(SLI_WIFI_REQ_SET_REGION,
                                       SLI_WIFI_WLAN_CMD,
                                       &request,
                                       sizeof(sli_wifi_set_region_request_t),
                                       SLI_WIFI_RSP_SET_REGION_WAIT_TIME,
                                       NULL,
                                       NULL);
        VERIFY_STATUS_AND_RETURN(status);
      }

      // For AP and concurrent modes, configure region-specific settings
      sli_wifi_set_region_ap_request_t request = { 0 };
      request.set_region_code_from_user_cmd    = SET_REGION_CODE_FROM_USER;

      // Configure region-specific settings based on the region code and band
      switch (region_code) {
        // Configure settings for different regions and bands
        case DEFAULT_REGION:
        case SL_WIFI_REGION_US: {
          if (band == SL_WIFI_BAND_MODE_2_4GHZ) {
            request = default_US_region_2_4GHZ_configurations;
          } else {
            request = default_US_region_5GHZ_configurations;
          }
          break;
        }
        case SL_WIFI_REGION_EU: {
          if (band == SL_WIFI_BAND_MODE_2_4GHZ) {
            request = default_EU_region_2_4GHZ_configurations;
          } else {
            request = default_EU_region_5GHZ_configurations;
          }
          break;
        }
        case SL_WIFI_REGION_JP: {
          if (band == SL_WIFI_BAND_MODE_2_4GHZ) {
            request = default_JP_region_2_4GHZ_configurations;
          } else {
            request = default_JP_region_5GHZ_configurations;
          }
          break;
        }
        case SL_WIFI_REGION_KR: {
          if (band == SL_WIFI_BAND_MODE_2_4GHZ) {
            request = default_KR_region_2_4GHZ_configurations;
          } else {
            request = default_KR_region_5GHZ_configurations;
          }
          break;
        }
        case SL_WIFI_REGION_SG: {
          if (band == SL_WIFI_BAND_MODE_2_4GHZ) {
            request = default_SG_region_2_4GHZ_configurations;
          } else {
            request = default_SG_region_5GHZ_configurations;
          }
          break;
        }
        case SL_WIFI_REGION_CN: {
          if (band == SL_WIFI_BAND_MODE_2_4GHZ) {
            request = default_CN_region_2_4GHZ_configurations;
          } else {
            request = default_CN_region_5GHZ_configurations;
          }
          break;
        }
        default:
          return SL_STATUS_NOT_SUPPORTED;
      }
      status = sli_wifi_send_command(SLI_WIFI_REQ_SET_REGION_AP,
                                     SLI_WIFI_WLAN_CMD,
                                     &request,
                                     sizeof(sli_wifi_set_region_ap_request_t),
                                     SLI_WIFI_RSP_SET_REGION_AP_WAIT_TIME,
                                     NULL,
                                     NULL);
      VERIFY_STATUS_AND_RETURN(status);
      break;
    }

    default:
      break;
  }

  return status;
}

void sli_wifi_prepare_mac_frame_header(const void *buf,
                                       const uint8_t *addr1,
                                       const uint8_t *addr2,
                                       const uint8_t *addr3)
{
  if (buf == NULL || addr1 == NULL || addr2 == NULL || addr3 == NULL) {
    return;
  }

  sli_ieee80211_hdr_t *hdr = (sli_ieee80211_hdr_t *)buf;

  hdr->frame_control = 0x8;        //Data packet type
  hdr->frame_control |= 0x01 << 8; //To the Distribution system
  hdr->duration_id = 0x00;
  memcpy(hdr->addr1, addr1, ETH_MAC_ADR_LEN);
  memcpy(hdr->addr2, addr2, ETH_MAC_ADR_LEN);
  memcpy(hdr->addr3, addr3, ETH_MAC_ADR_LEN);
}

sl_status_t sli_wifi_send_mac_data_frame(sl_wifi_interface_t interface,
                                         sl_wifi_transmitter_test_info_t *per_params,
                                         sl_wifi_system_packet_t *packet,
                                         uint16_t chunk_length)
{
  unsigned int temp_word;
  uint8_t _11ax_transmit         = 0;
  unsigned int bbp_info          = 0;
  unsigned int rate_field_params = 0;
  unsigned int rate_flags        = 0;
  unsigned int ch_bw             = 0;
  unsigned int greenfield        = 0;
  unsigned char extended_desc    = 4;
  sl_status_t status;

  // If the packet is not allocated successfully, return an allocation failed error
  if (packet == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  if (!device_initialized) {
    sli_buffer_manager_free_buffer((sli_buffer_t *)packet);
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    sli_buffer_manager_free_buffer((sli_buffer_t *)packet);
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  if (chunk_length <= FRAME_DESC_SZ) {
    sli_buffer_manager_free_buffer((sli_buffer_t *)packet);
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Clear the packet descriptor and copy the command data if available
  memset(packet->desc, 0, sizeof(packet->desc));

  packet->length  = (chunk_length - FRAME_DESC_SZ) & 0xFFF;
  packet->command = (uint16_t)SLI_SEND_MAC_FRAME;

  // Set the packet's transmitter test information
  _11ax_transmit = per_params->enable_11ax;
  if (_11ax_transmit) {
    bbp_info = (per_params->coding_type << 0) | (per_params->nominal_pe << 1) | (per_params->ul_dl << 3)
               | (per_params->he_ppdu_type << 4) | (per_params->beam_change << 6) | (per_params->bw << 7)
               | (per_params->stbc << 8) | (per_params->tx_bf << 9) | (per_params->dcm << 10)
               | (per_params->gi_ltf << 11) | (per_params->nsts_midamble << 13);
  }
  rate_field_params = (per_params->nominal_pe << RATE_OFFSET_NOMINAL_PE)
                      | (per_params->coding_type << RATE_OFFSET_CODING_TYPE)
                      | (per_params->gi_ltf << RATE_OFFSET_GI_LTF) | (per_params->dcm << RATE_OFFSET_DCM);

  *(uint32_t *)&packet->desc[4] = (MIN_802_11_HDR_LEN << 8) | (BIT(2) << 16); //MIN_HDR_LEN Insert Seq no

  if (per_params->aggr_enable) {
    temp_word = ((ENABLE_MAC_INFO) << 16); //In mac_info set bit0 and bit9 for bcast pkt
    *(uint32_t *)&packet->desc[12] |= (QOS_EN);
  } else {
    temp_word = ((BROADCAST_IND | ENABLE_MAC_INFO) << 16); //In mac_info set bit0 and bit9 for bcast pkt
  }
  rate_flags = (per_params->rate_flags << 2);
  ch_bw      = (rate_flags & 0x00F0);

  if (ch_bw & BIT(4))
    ch_bw = 0;
  greenfield = (rate_flags & 0x0008);
  greenfield = (greenfield << 10);

  if (rate_flags & 0x0004) //checking short_GI
  {
    per_params->rate |= BIT(9);
  }

  *(uint32_t *)&packet->desc[4] |= (temp_word | extended_desc);

  if (_11ax_transmit) {
    *(uint32_t *)&packet->desc[8] = ((per_params->rate & SLI_11AX_BE_RATE_MASK) | BIT(10) | rate_field_params);
    *(uint32_t *)&packet->desc[8] |= (bbp_info << 16);
  } else {

    *(uint32_t *)&packet->desc[8] = ((per_params->rate & 0x3ff) | (ch_bw << 12) | greenfield);
  }
  *(uint32_t *)&packet->data[0] = (per_params->power) & 0xff;
  if (per_params->mode == PER_CONT_MODE) {
    *(uint32_t *)&packet->data[0] |= (3 << 8);
  }

  // Modify the packet's descriptor to include the firmware queue ID in the length field
  packet->desc[1] |= (5 << 4);
  packet->desc[14] |= 1;

  // Calling the routing utility to route the packet
  status =
    sli_wifi_send_data_packet((void *)packet, ((packet->length & 0xFFF) + sizeof(sl_wifi_system_packet_t)), NULL);

  if (status != SL_STATUS_OK) {
    sli_buffer_manager_free_buffer((sli_buffer_t *)packet);
  }
  return status;
}
