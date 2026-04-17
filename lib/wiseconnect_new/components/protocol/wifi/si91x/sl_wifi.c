/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_wifi_types.h"
#include "sl_wifi_constants.h"
#include "sl_wifi_device.h"
#include "sl_si91x_host_interface.h"
#include "sl_si91x_status.h"
#include "sl_si91x_types.h"
#include "sl_si91x_protocol_types.h"
#include "sl_si91x_driver.h"
#include "sl_rsi_utility.h"
#include "sli_wifi_types.h"
#include "sli_wifi.h"
#include "sli_wifi_utility.h"
#include "sl_log_helper_si91x.h"
#if defined(SLI_SI91X_SOCKETS)
#include "sl_si91x_socket_utility.h"
#endif
#include <stdint.h>
#include <string.h>
#include "sli_wifi_constants.h"
#include "sl_string.h"
#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
#include "sl_net_si91x_integration_handler.h"
#else
// This macro defines a handler for cleaning up resources.
#define SLI_NETWORK_CLEANUP_HANDLER() \
  {                                   \
  }
#endif

#ifndef MIN
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
#include "sl_si91x_driver.h"
#include "rsi_m4.h"
extern rsi_m4ta_desc_t crypto_desc[2];
#endif

#ifdef SLI_SI91X_MCU_INTERFACE
#include "rsi_wisemcu_hardware_setup.h"
#endif

#ifdef SL_CATALOG_LOGGER_COMPONENT_PRESENT
extern uint32_t sl_si91x_log_host_timesync_address;
#endif

extern bool device_initialized;
extern sl_wifi_advanced_scan_configuration_t advanced_scan_configuration;
sl_status_t sl_wifi_get_associated_client_list(const void *client_list_buffer,
                                               uint16_t buffer_length,
                                               uint32_t timeout);

sl_status_t sl_wifi_init(const sl_wifi_device_configuration_t *configuration,
                         const sl_wifi_device_context_t *device_context,
                         sl_wifi_event_handler_t event_handler)
{
  UNUSED_PARAMETER(device_context);
#ifdef SLI_SI91X_MCU_INTERFACE
#if defined(SLI_SI917)
  sli_wifi_efuse_data_t efuse_data;
#endif
#endif
  sl_status_t status = SL_STATUS_OK;
  status             = sl_si91x_driver_init(configuration, event_handler);
#ifdef SL_CATALOG_LOGGER_COMPONENT_PRESENT
#if defined(SLI_SI91X_MCU_INTERFACE)
  if (status == SL_STATUS_OK) {
    status = sl_si91x_configure_timestamp_memory_location(sizeof(uint32_t), &sl_si91x_log_host_timesync_address);
    if (status != SL_STATUS_OK) {
      SL_PRINT_STRING_ERROR("\r\nTimestamp Memory Location Configuration Failed with error: 0x%lX\r\n", status);
    }
    sl_log_level_t level        = sl_log_get_loglevel();
    sli_nwp_log_config_t config = { .log_config_level = (uint8_t)level };
    status                      = sli_nwp_log_configure(&config);
    if (status != SL_STATUS_IN_PROGRESS) {
      SL_PRINT_STRING_ERROR("\r\nNWP Log Configuration Failed with error: 0x%lX\r\n", status);
    }
    status = SL_STATUS_OK;
  }
#endif
#endif
#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  if (status == SL_STATUS_OK) {
    uint32_t crypto_desc_ptr = (uint32_t)crypto_desc;
    uint32_t *desc_ptr       = &crypto_desc_ptr;
    status = sl_si91x_m4_ta_secure_handshake(SL_SI91X_ENABLE_SIDE_BAND, sizeof(uint32_t), (uint8_t *)desc_ptr, 0, NULL);
  }
#endif
#ifdef SLI_SI91X_MCU_INTERFACE
#if defined(SLI_SI917)
  if (status == SL_STATUS_OK) {
    /*Getting PTE CRC value to distinguish firmware 17 and 18 boards.*/
    sli_si91x_get_flash_efuse_data(&efuse_data, SL_SI91X_EFUSE_PTE_CRC);

    /*PTE FW version check.*/
    if (efuse_data.pte_crc == FIRMWARE_17_PTE_CRC_VALUE) {
      /* Enable Higher PWM RO Frequency Mode for PMU for FW17 boards*/
      RSI_IPMU_Set_Higher_Pwm_Ro_Frequency_Mode_to_PMU();
      /* Set the RETN_LDO voltage to 0.8V for FW17 boards*/
      RSI_IPMU_Retn_Voltage_To_Default();
    }
  }
#endif
#endif
  return status;
}

sl_status_t sl_wifi_set_antenna(sl_wifi_interface_t interface, sl_wifi_antenna_t antenna)
{
  return sli_wifi_set_antenna(interface, antenna);
}

sl_status_t sl_wifi_wait_for_scan_results(sl_wifi_scan_result_t **scan_results, uint32_t max_scan_result_count)
{
  return sli_wifi_wait_for_scan_results(scan_results, max_scan_result_count);
}

sl_status_t sl_wifi_start_scan(sl_wifi_interface_t interface,
                               const sl_wifi_ssid_t *optional_ssid,
                               const sl_wifi_scan_configuration_t *configuration)
{
  return sli_wifi_start_scan(interface, optional_ssid, configuration);
}

sl_status_t sl_wifi_get_stored_scan_results(sl_wifi_interface_t interface,
                                            sl_wifi_extended_scan_result_parameters_t *extended_scan_parameters)
{
  return sli_wifi_get_stored_scan_results(interface, extended_scan_parameters);
}

sl_status_t sl_wifi_connect(sl_wifi_interface_t interface,
                            const sl_wifi_client_configuration_t *ap,
                            uint32_t timeout_ms)
{
  return sli_wifi_connect(interface, ap, timeout_ms);
}

sl_status_t sl_wifi_set_advanced_client_configuration(sl_wifi_interface_t interface,
                                                      const sl_wifi_advanced_client_configuration_t *configuration)
{
  return sli_wifi_set_advanced_client_configuration(interface, configuration);
}

sl_status_t sl_wifi_get_signal_strength(sl_wifi_interface_t interface, int32_t *rssi)
{
  return sli_wifi_get_signal_strength(interface, rssi);
}

sl_status_t sl_wifi_get_sta_tsf(sl_wifi_interface_t interface, sl_wifi_tsf64_t *tsf)
{
  return sli_wifi_get_sta_tsf(interface, tsf);
}

sl_status_t sl_wifi_set_mac_address(sl_wifi_interface_t interface, const sl_mac_address_t *mac_address)
{
  return sli_wifi_set_mac_address(interface, mac_address);
}

sl_status_t sl_wifi_get_mac_address(sl_wifi_interface_t interface, sl_mac_address_t *mac)
{
  return sli_wifi_get_mac_address(interface, mac);
}

sl_status_t sl_wifi_set_channel(sl_wifi_interface_t interface, sl_wifi_channel_t channel)
{
  return sli_wifi_set_channel(interface, channel);
}

sl_status_t sl_wifi_config_pll_mode(sl_wifi_pll_mode_t pll_mode)
{
  return sli_wifi_config_pll_mode(pll_mode);
}

sl_status_t sl_wifi_config_power_chain(sl_wifi_power_chain_t power_chain)
{
  return sli_wifi_config_power_chain(power_chain);
}

sl_status_t sl_wifi_get_channel(sl_wifi_interface_t interface, sl_wifi_channel_t *channel_info)
{
  sl_status_t status             = SL_STATUS_FAIL;
  sl_wifi_buffer_t *buffer       = NULL;
  sli_wlan_cmd_request_t command = 0;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  if ((interface == SL_WIFI_CLIENT_2_4GHZ_INTERFACE) || (interface == SL_WIFI_CLIENT_5GHZ_INTERFACE)
      || (interface == SL_WIFI_CLIENT_DUAL_INTERFACE))
    command = (sli_wlan_cmd_request_t)SLI_WIFI_REQ_QUERY_NETWORK_PARAMS;
  else if ((interface == SL_WIFI_AP_2_4GHZ_INTERFACE) || (interface == SL_WIFI_AP_5GHZ_INTERFACE)
           || (interface == SL_WIFI_AP_DUAL_INTERFACE))
    command = (sli_wlan_cmd_request_t)SLI_WIFI_REQ_QUERY_GO_PARAMS;

  status = sli_wifi_send_command(command,
                                 SLI_WIFI_WLAN_CMD,
                                 NULL,
                                 0,
                                 SLI_WIFI_WAIT_FOR_RESPONSE(SL_SI91X_GET_CHANNEL_TIMEOUT),
                                 NULL,
                                 (void **)&buffer);
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);
  sl_wifi_system_packet_t *packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);

  switch (interface) {
    case SL_WIFI_CLIENT_2_4GHZ_INTERFACE: {
      channel_info->channel = ((sli_si91x_network_params_response_t *)packet->data)->channel_number;
      channel_info->band    = SL_WIFI_BAND_2_4GHZ;
      break;
    }
    case SL_WIFI_CLIENT_5GHZ_INTERFACE: {
      channel_info->channel = ((sli_si91x_network_params_response_t *)packet->data)->channel_number;
      channel_info->band    = SL_WIFI_BAND_5GHZ;
      break;
    }
    case SL_WIFI_CLIENT_DUAL_INTERFACE: {
      channel_info->channel = ((sli_si91x_network_params_response_t *)packet->data)->channel_number;
      channel_info->band    = SL_WIFI_BAND_DUAL;
      break;
    }
    case SL_WIFI_AP_2_4GHZ_INTERFACE: {
      memcpy(&channel_info->channel, ((sli_wifi_client_info_response *)packet->data)->channel_number, 2);
      channel_info->band = SL_WIFI_BAND_2_4GHZ;
      break;
    }
    case SL_WIFI_AP_5GHZ_INTERFACE: {
      memcpy(&channel_info->channel, ((sli_wifi_client_info_response *)packet->data)->channel_number, 2);
      channel_info->band = SL_WIFI_BAND_5GHZ;
      break;
    }
    case SL_WIFI_AP_DUAL_INTERFACE: {
      memcpy(&channel_info->channel, ((sli_wifi_client_info_response *)packet->data)->channel_number, 2);
      channel_info->band = SL_WIFI_BAND_DUAL;
      break;
    }
    default:
      break;
  }

  sli_si91x_host_free_buffer(buffer);
  return status;
}

/*
 * This API doesn't have any affect if it is called after connect/start ap.
 */
sl_status_t sl_wifi_set_max_tx_power(sl_wifi_interface_t interface, sl_wifi_max_tx_power_t max_tx_power)
{
  return sli_wifi_set_max_tx_power(interface, max_tx_power);
}

sl_status_t sl_wifi_get_max_tx_power(sl_wifi_interface_t interface, sl_wifi_max_tx_power_t *max_tx_power)
{
  return sli_wifi_get_max_tx_power(interface, max_tx_power);
}

sl_status_t sl_wifi_set_rts_threshold(sl_wifi_interface_t interface, uint16_t rts_threshold)
{
  return sli_wifi_set_rts_threshold(interface, rts_threshold);
}

sl_status_t sl_wifi_get_rts_threshold(sl_wifi_interface_t interface, uint16_t *rts_threshold)
{
  return sli_wifi_get_rts_threshold(interface, rts_threshold);
}

sl_status_t sl_wifi_set_mfp(sl_wifi_interface_t interface, const sl_wifi_mfp_mode_t config)
{
  return sli_wifi_set_mfp(interface, config);
}

sl_status_t sl_wifi_get_mfp(sl_wifi_interface_t interface, sl_wifi_mfp_mode_t *config)
{
  return sli_wifi_get_mfp(interface, config);
}

sl_status_t sl_wifi_start_ap(sl_wifi_interface_t interface, const sl_wifi_ap_configuration_t *configuration)
{
  return sli_wifi_start_ap(interface, configuration);
}

sl_status_t sl_wifi_get_pairwise_master_key(sl_wifi_interface_t interface,
                                            const uint8_t type,
                                            const sl_wifi_ssid_t *ssid,
                                            const char *pre_shared_key,
                                            uint8_t *pairwise_master_key)
{
  return sli_wifi_get_pairwise_master_key(interface, type, ssid, pre_shared_key, pairwise_master_key);
}

sl_status_t sl_wifi_get_associated_client_list(const void *client_list_buffer, uint16_t buffer_length, uint32_t timeout)
{
  UNUSED_PARAMETER(client_list_buffer);
  UNUSED_PARAMETER(buffer_length);
  UNUSED_PARAMETER(timeout);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_wifi_disconnect_ap_client(sl_wifi_interface_t interface,
                                         const sl_mac_address_t *mac,
                                         sl_wifi_deauth_reason_t reason)
{
  return sli_wifi_disconnect_ap_client(interface, mac, reason);
}

sl_status_t sl_wifi_get_ap_client_info(sl_wifi_interface_t interface, sl_wifi_client_info_response_t *client_info)
{
  return sli_wifi_get_ap_client_info(interface, client_info);
}

sl_status_t sl_wifi_get_firmware_version(sl_wifi_firmware_version_t *version)
{
  return sl_si91x_get_firmware_version((sl_si91x_firmware_version_t *)version);
}

sl_status_t sl_wifi_get_interface_info(sl_wifi_interface_t interface, sl_wifi_interface_info_t *info)
{
  sl_status_t status       = 0;
  sl_wifi_buffer_t *buffer = NULL;
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  SL_WIFI_ARGS_CHECK_NULL_POINTER(info);

  // Select command based on interface
  sli_wifi_request_commands_t command;
  if (interface & SL_WIFI_CLIENT_INTERFACE) {
    command = (sli_wifi_request_commands_t)SLI_WIFI_REQ_QUERY_NETWORK_PARAMS;
  } else if (interface & SL_WIFI_AP_INTERFACE) {
    command = (sli_wifi_request_commands_t)SLI_WIFI_REQ_QUERY_GO_PARAMS;
  } else {
    return SL_STATUS_NOT_SUPPORTED;
  }

  status = sli_wifi_send_command(command,
                                 SLI_WIFI_WLAN_CMD,
                                 NULL,
                                 0,
                                 SLI_WIFI_WAIT_FOR_RESPONSE(SL_SI91X_GET_INTERFACE_INFO_TIMEOUT),
                                 NULL,
                                 (void **)&buffer);
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);
  sl_wifi_system_packet_t *packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  memset(info, 0, sizeof(sl_wifi_interface_info_t));

  if (packet->length > 0) {
    if (command == SLI_WIFI_REQ_QUERY_GO_PARAMS) {
      // AP mode
      sli_wifi_client_info_response *response = (sli_wifi_client_info_response *)packet->data;
      // wlan state: no of stations connected in AP mode
      memcpy(&info->wlan_state, (uint16_t *)&response->sta_count, sizeof(uint16_t));
      memcpy(&info->channel_number, (uint16_t *)&response->channel_number, sizeof(uint16_t));
      memcpy(info->ssid, response->ssid, MIN(sizeof(info->ssid), sizeof(response->ssid)));
      memcpy(info->mac_address, response->mac_address, SL_WIFI_MAC_ADDRESS_LENGTH);
      // PSK for AP mode, PMK for Client mode
      memcpy(info->psk_pmk, response->psk, 64);
    } else {
      // Station mode
      sli_si91x_network_params_response_t *response = (sli_si91x_network_params_response_t *)packet->data;
      memcpy(&info->wlan_state, (uint16_t *)&response->wlan_state, sizeof(uint8_t));
      memcpy((uint8_t *)&info->channel_number, &response->channel_number, sizeof(uint8_t));
      memcpy(info->ssid, response->ssid, MIN(sizeof(info->ssid), sizeof(response->ssid)));
      memcpy(info->mac_address, response->mac_address, SL_WIFI_MAC_ADDRESS_LENGTH);
      memcpy(&info->sec_type, &response->sec_type, sizeof(uint8_t));
      // PSK for AP mode, PMK for Client mode
      memcpy(info->psk_pmk, response->psk, 64);
      memcpy(info->bssid, response->bssid, 6);
      memcpy(&info->wireless_mode, &response->wireless_mode, sizeof(uint8_t));
    }
  }
  sli_si91x_host_free_buffer(buffer);
  return status;
}

sl_status_t sl_wifi_get_wireless_info(sl_si91x_rsp_wireless_info_t *info)
{
  sl_status_t status       = 0;
  sl_wifi_buffer_t *buffer = NULL;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  SL_WIFI_ARGS_CHECK_NULL_POINTER(info);

  if (sli_wifi_get_opermode() == SL_WIFI_ACCESS_POINT_MODE) {
    // Send cmd for wlan info in AP mode
    status = sli_wifi_send_command(SLI_WIFI_REQ_QUERY_GO_PARAMS,
                                   SLI_WIFI_WLAN_CMD,
                                   NULL,
                                   0,
                                   SLI_WIFI_WAIT_FOR_RESPONSE(1000),
                                   NULL,
                                   (void **)&buffer);
  } else if ((sli_wifi_get_opermode() == SL_WIFI_CLIENT_MODE)
             || (sli_wifi_get_opermode() == SL_WIFI_ENTERPRISE_CLIENT_MODE)) {
    //! Send cmd for wlan info in client mode
    status = sli_wifi_send_command(SLI_WIFI_REQ_QUERY_NETWORK_PARAMS,
                                   SLI_WIFI_WLAN_CMD,
                                   NULL,
                                   0,
                                   SLI_WIFI_WAIT_FOR_RESPONSE(1000),
                                   NULL,
                                   (void **)&buffer);
  } else {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);
  sl_wifi_system_packet_t *packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);

  memset(info, 0, sizeof(sl_si91x_rsp_wireless_info_t));

  //In AP mode, receives a buffer equivalent to sli_wifi_client_info_response.
  if (packet->length > 0 && sli_wifi_get_opermode() == SL_WIFI_ACCESS_POINT_MODE) {
    sli_wifi_client_info_response *response = (sli_wifi_client_info_response *)packet->data;
    // wlan state: no of stations connected in AP mode
    memcpy(&info->wlan_state, (uint16_t *)&response->sta_count, sizeof(uint16_t));
    memcpy(&info->channel_number, (uint16_t *)&response->channel_number, sizeof(uint16_t));
    memcpy(info->ssid, response->ssid, MIN(sizeof(info->ssid), sizeof(response->ssid)));
    memcpy(info->mac_address, response->mac_address, 6);
    // PSK for AP mode, PMK for Client mode
    memcpy(info->psk_pmk, response->psk, 64);
    memcpy(info->ipv4_address, response->ipv4_address, 4);
    memcpy(info->ipv6_address, response->ipv6_address, 16);
  }
  //In Client mode, receives a buffer equivalent to sli_si91x_network_params_response_t.
  else if (packet->length > 0
           && ((sli_wifi_get_opermode() == SL_WIFI_CLIENT_MODE)
               || (sli_wifi_get_opermode() == SL_WIFI_ENTERPRISE_CLIENT_MODE))) {
    sli_si91x_network_params_response_t *response = (sli_si91x_network_params_response_t *)packet->data;
    memcpy(&info->wlan_state, (uint16_t *)&response->wlan_state, sizeof(uint8_t));
    memcpy((uint8_t *)&info->channel_number, &response->channel_number, sizeof(uint8_t));
    memcpy(info->ssid, response->ssid, MIN(sizeof(info->ssid), sizeof(response->ssid)));
    memcpy(info->mac_address, response->mac_address, 6);
    memcpy(&info->sec_type, &response->sec_type, sizeof(uint8_t));
    // PSK for AP mode, PMK for Client mode
    memcpy(info->psk_pmk, response->psk, 64);
    memcpy(info->ipv4_address, response->ipv4_address, 4);
    memcpy(info->ipv6_address, response->ipv6_address, 16);
    memcpy(info->bssid, response->bssid, 6);
    memcpy(&info->wireless_mode, &response->wireless_mode, sizeof(uint8_t));
  }

  sli_si91x_host_free_buffer(buffer);
  return status;
}

sl_status_t sl_wifi_get_firmware_size(const void *buffer, uint32_t *fw_image_size)
{
  return sl_si91x_get_firmware_size(buffer, fw_image_size);
}

sl_status_t sl_wifi_disconnect(sl_wifi_interface_t interface)
{
  return sli_wifi_disconnect(interface);
}

sl_status_t sl_wifi_stop_ap(sl_wifi_interface_t interface)
{
  return sli_wifi_stop_ap(interface);
}

sl_status_t sl_wifi_get_statistics(sl_wifi_interface_t interface, sl_wifi_statistics_t *statistics)
{
  return sli_wifi_get_statistics(interface, statistics);
}

sl_status_t sl_wifi_get_operational_statistics(sl_wifi_interface_t interface,
                                               sl_wifi_operational_statistics_t *operational_statistics)
{
  return sli_wifi_get_operational_statistics(interface, operational_statistics);
}

sl_status_t sl_wifi_transmit_test_start(sl_wifi_interface_t interface,
                                        const sl_wifi_transmitter_test_info_t *test_tx_info)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  return sli_wifi_transmit_test_start(interface, test_tx_info);
}

sl_status_t sl_wifi_transmit_test_start_with_payload(sl_wifi_interface_t interface,
                                                     const sl_wifi_transmitter_test_info_t *tx_test_info,
                                                     const uint8_t *payload,
                                                     const uint16_t payload_length)
{
  UNUSED_PARAMETER(interface);
  UNUSED_PARAMETER(tx_test_info);
  UNUSED_PARAMETER(payload);
  UNUSED_PARAMETER(payload_length);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_wifi_transmit_test_stop(sl_wifi_interface_t interface)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  return sli_wifi_transmit_test_stop(interface);
}

sl_status_t sl_wifi_frequency_offset(sl_wifi_interface_t interface, const sl_wifi_freq_offset_t *frequency_calibration)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  UNUSED_PARAMETER(interface);
  return sli_wifi_frequency_offset(interface, frequency_calibration);
}

sl_status_t sl_wifi_dpd_calibration(sl_wifi_interface_t interface, const sl_wifi_dpd_calib_data_t *dpd_calib_data)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  return sli_wifi_dpd_calibration(interface, dpd_calib_data);
}

sl_status_t sl_wifi_start_statistic_report(sl_wifi_interface_t interface, sl_wifi_channel_t channel)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  return sli_wifi_start_statistic_report(interface, channel);
}

sl_status_t sl_wifi_stop_statistic_report(sl_wifi_interface_t interface)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  return sli_wifi_stop_statistic_report(interface);
}

sl_status_t sl_wifi_set_performance_profile(const sl_wifi_performance_profile_t *profile)
{
  return sli_wifi_set_performance_profile(profile);
}

sl_status_t sl_wifi_set_performance_profile_v2(const sl_wifi_performance_profile_v2_t *profile)
{
  return sli_wifi_set_performance_profile_v2(profile);
}

sl_status_t sl_wifi_get_performance_profile(sl_wifi_performance_profile_t *profile)
{
  return sli_wifi_get_performance_profile(profile);
}

sl_status_t sl_wifi_get_performance_profile_v2(sl_wifi_performance_profile_v2_t *profile)
{
  return sli_wifi_get_performance_profile_v2(profile);
}

sl_wifi_interface_t sl_wifi_get_default_interface(void)
{
  return sli_wifi_get_default_interface();
}

void sl_wifi_set_default_interface(sl_wifi_interface_t interface)
{
  return sli_wifi_set_default_interface(interface);
}

sl_status_t sl_wifi_deinit(void)
{
  sl_status_t status = SL_STATUS_OK;
  status             = sl_si91x_driver_deinit();
  if (status != SL_STATUS_OK) {
    return status;
  }
  sli_wifi_deinit();
  return status;
}

// 5GHz interface is currently not supported for Si91x
bool sl_wifi_is_interface_up(sl_wifi_interface_t interface)
{
  return sli_wifi_is_interface_up(interface);
}

sl_status_t sl_wifi_set_certificate_with_index(uint8_t certificate_type,
                                               uint8_t certificate_index,
                                               const uint8_t *buffer,
                                               uint32_t certificate_length)
{
  return sl_si91x_wifi_set_certificate_index(certificate_type, certificate_index, buffer, certificate_length);
}

sl_status_t sl_wifi_set_certificate(uint8_t certificate_type, const uint8_t *buffer, uint32_t certificate_length)
{
  return sl_si91x_wifi_set_certificate_index(certificate_type, 0, buffer, certificate_length);
}

sl_status_t sl_wifi_set_transmit_rate(sl_wifi_interface_t interface,
                                      sl_wifi_rate_protocol_t rate_protocol,
                                      sl_wifi_rate_t mask)
{
  return sli_wifi_set_transmit_rate(interface, rate_protocol, mask);
}

sl_status_t sl_wifi_get_transmit_rate(sl_wifi_interface_t interface,
                                      sl_wifi_rate_protocol_t *rate_protocol,
                                      sl_wifi_rate_t *mask)
{
  return sli_wifi_get_transmit_rate(interface, rate_protocol, mask);
}

sl_status_t sl_wifi_get_ap_client_count(sl_wifi_interface_t interface, uint32_t *client_list_count)
{
  return sli_wifi_get_ap_client_count(interface, client_list_count);
}

sl_status_t sl_wifi_get_ap_client_list(sl_wifi_interface_t interface,
                                       uint16_t client_list_count,
                                       sl_mac_address_t *client_list)
{
  return sli_wifi_get_ap_client_list(interface, client_list_count, client_list);
}
sl_status_t sl_wifi_generate_wps_pin(sl_wifi_wps_pin_t *wps_pin)
{
  return sli_wifi_generate_wps_pin(wps_pin);
}

sl_status_t sl_wifi_start_wps(sl_wifi_interface_t interface,
                              sl_wifi_wps_mode_t mode,
                              const sl_wifi_wps_pin_t *optional_wps_pin)
{
  return sli_wifi_start_wps(interface, mode, optional_wps_pin);
}

sl_status_t sl_wifi_start_wps_v2(sl_wifi_interface_t interface,
                                 sl_wifi_wps_config_t config,
                                 sl_wifi_wps_response_t *response)
{
  return sli_wifi_start_wps_v2(interface, config, response);
}

sl_status_t sl_wifi_set_roam_configuration(sl_wifi_interface_t interface,
                                           const sl_wifi_roam_configuration_t *roam_configuration)
{
  return sli_wifi_set_roam_configuration(interface, roam_configuration);
}

sl_status_t sl_wifi_set_advanced_scan_configuration(const sl_wifi_advanced_scan_configuration_t *configuration)
{
  return sli_wifi_set_advanced_scan_configuration(configuration);
}

sl_status_t sl_wifi_get_advanced_scan_configuration(sl_wifi_advanced_scan_configuration_t *configuration)
{
  return sli_wifi_get_advanced_scan_configuration(configuration);
}

sl_status_t sl_wifi_stop_scan(sl_wifi_interface_t interface)
{
  return sli_wifi_stop_scan(interface);
}

sl_status_t sl_wifi_set_ap_configuration(sl_wifi_interface_t interface, const sl_wifi_ap_configuration_t *configuration)
{
  UNUSED_PARAMETER(interface);
  UNUSED_PARAMETER(configuration);
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  //Firmware unable to configure the ap configuration of a running AP interface
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_wifi_get_ap_configuration(sl_wifi_interface_t interface, sl_wifi_ap_configuration_t *configuration)
{
  return sli_wifi_get_ap_configuration(interface, configuration);
}

sl_status_t sl_wifi_reconfigure_ap(sl_wifi_interface_t interface, sl_wifi_ap_reconfiguration_t config)
{
  return sli_wifi_reconfigure_ap(interface, config);
}

sl_status_t sl_wifi_test_client_configuration(sl_wifi_interface_t interface,
                                              const sl_wifi_client_configuration_t *ap,
                                              uint32_t timeout_ms)
{
  return sli_wifi_test_client_configuration(interface, ap, timeout_ms);
}

sl_status_t sl_wifi_send_raw_data_frame(sl_wifi_interface_t interface, const void *data, uint16_t data_length)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  SL_VERIFY_POINTER_OR_RETURN(data, SL_STATUS_NULL_POINTER);

  if (data_length == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  return sl_si91x_driver_raw_send_command(SLI_SEND_RAW_DATA, data, data_length, SLI_SEND_RAW_DATA_RESPONSE_WAIT_TIME);
}

sl_status_t sl_wifi_enable_target_wake_time(const sl_wifi_twt_request_t *twt_req)
{
  return sli_wifi_enable_target_wake_time(twt_req);
}

sl_status_t sl_wifi_target_wake_time_auto_selection(sl_wifi_twt_selection_t *twt_auto_request)
{
  return sli_wifi_target_wake_time_auto_selection(twt_auto_request);
}

sl_status_t sl_wifi_disable_target_wake_time(const sl_wifi_twt_request_t *twt_req)
{
  return sli_wifi_disable_target_wake_time(twt_req);
}

sl_status_t sl_wifi_reschedule_twt(uint8_t flow_id,
                                   sl_wifi_reschedule_twt_action_t twt_action,
                                   uint64_t suspend_duration)
{
  return sli_wifi_reschedule_twt(flow_id, twt_action, suspend_duration);
}

sl_status_t sl_wifi_filter_broadcast(uint16_t beacon_drop_threshold,
                                     uint8_t filter_bcast_in_tim,
                                     uint8_t filter_bcast_tim_till_next_cmd)
{
  return sli_wifi_filter_broadcast(beacon_drop_threshold, filter_bcast_in_tim, filter_bcast_tim_till_next_cmd);
}

sl_status_t sl_wifi_update_gain_table(uint8_t band, uint8_t bandwidth, const uint8_t *payload, uint16_t payload_length)
{
  return sli_wifi_update_gain_table(band, bandwidth, payload, payload_length);
}

sl_status_t sl_wifi_update_su_gain_table(uint8_t band,
                                         uint8_t bandwidth,
                                         const uint8_t *payload,
                                         uint16_t payload_length,
                                         uint8_t x_offset,
                                         uint8_t y_offset)
{
  return sli_wifi_update_su_gain_table(band, bandwidth, payload, payload_length, x_offset, y_offset);
}

sl_status_t sl_wifi_set_11ax_config(uint8_t guard_interval)
{
  return sli_wifi_set_11ax_config(guard_interval);
}

sl_status_t sl_wifi_set_listen_interval(sl_wifi_interface_t interface, sl_wifi_listen_interval_t listen_interval)
{
  return sli_wifi_set_listen_interval(interface, listen_interval);
}

sl_status_t sl_wifi_set_listen_interval_v2(sl_wifi_interface_t interface, sl_wifi_listen_interval_v2_t listen_interval)
{
  return sli_wifi_set_listen_interval_v2(interface, listen_interval);
}

sl_status_t sl_wifi_get_listen_interval(sl_wifi_interface_t interface, sl_wifi_listen_interval_t *listen_interval)
{
  return sli_wifi_get_listen_interval(interface, listen_interval);
}

sl_status_t sl_wifi_get_listen_interval_v2(sl_wifi_interface_t interface, sl_wifi_listen_interval_v2_t *listen_interval)
{
  return sli_wifi_get_listen_interval_v2(interface, listen_interval);
}

sl_status_t sl_wifi_enable_monitor_mode(sl_wifi_interface_t interface)
{
  UNUSED_PARAMETER(interface);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_wifi_disable_monitor_mode(sl_wifi_interface_t interface)
{
  UNUSED_PARAMETER(interface);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_wifi_start_p2p_discovery(sl_wifi_interface_t interface,
                                        const sl_wifi_p2p_configuration_t *configuration,
                                        sl_wifi_credential_id_t credential_id)
{
  UNUSED_PARAMETER(interface);
  UNUSED_PARAMETER(configuration);
  UNUSED_PARAMETER(credential_id);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_wifi_p2p_connect(sl_wifi_interface_t interface, const sl_wifi_p2p_configuration_t *configuration)
{
  UNUSED_PARAMETER(interface);
  UNUSED_PARAMETER(configuration);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_wifi_transceiver_set_channel(sl_wifi_interface_t interface, sl_wifi_transceiver_set_channel_t channel)
{
  return sli_wifi_transceiver_set_channel(interface, channel);
}

sl_status_t sl_wifi_set_transceiver_parameters(sl_wifi_interface_t interface, sl_wifi_transceiver_parameters_t *params)
{
  return sli_wifi_set_transceiver_parameters(interface, params);
}

sl_status_t sl_wifi_transceiver_up(sl_wifi_interface_t interface, sl_wifi_transceiver_configuration_t *config)
{
  return sli_wifi_transceiver_up(interface, config);
}

static int32_t sli_validate_wifi_datarate(sl_wifi_data_rate_t data_rate)
{
  switch (data_rate) {
    case SL_WIFI_DATA_RATE_1:
    case SL_WIFI_DATA_RATE_2:
    case SL_WIFI_DATA_RATE_5_5:
    case SL_WIFI_DATA_RATE_11:
    case SL_WIFI_DATA_RATE_6:
    case SL_WIFI_DATA_RATE_9:
    case SL_WIFI_DATA_RATE_12:
    case SL_WIFI_DATA_RATE_18:
    case SL_WIFI_DATA_RATE_24:
    case SL_WIFI_DATA_RATE_36:
    case SL_WIFI_DATA_RATE_48:
    case SL_WIFI_DATA_RATE_54:
      return SL_STATUS_OK;
    default:
      return SL_STATUS_TRANSCEIVER_INVALID_DATA_RATE;
  }
}

sl_status_t sl_wifi_send_transceiver_data(sl_wifi_interface_t interface,
                                          sl_wifi_transceiver_tx_data_control_t *control,
                                          const uint8_t *payload,
                                          uint16_t payload_len)
{
  sl_status_t status = SL_STATUS_OK;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!sli_wifi_is_interface_up(interface)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  if ((!payload_len) || (payload_len > MAX_PAYLOAD_LEN)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  SL_VERIFY_POINTER_OR_RETURN(control, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(payload, SL_STATUS_NULL_POINTER);

  if ((IS_FIXED_DATA_RATE(control->ctrl_flags)) && (sli_validate_wifi_datarate(control->rate))) {
    return SL_STATUS_TRANSCEIVER_INVALID_DATA_RATE;
  }

  status = sl_si91x_driver_send_transceiver_data(control, payload, payload_len, SLI_WIFI_WAIT_FOR(1000));

  return status;
}

sl_status_t sl_wifi_update_transceiver_peer_list(sl_wifi_interface_t interface, sl_wifi_transceiver_peer_update_t peer)
{
  return sli_wifi_update_transceiver_peer_list(interface, peer);
}

sl_status_t sl_wifi_set_transceiver_multicast_filter(sl_wifi_interface_t interface,
                                                     sl_wifi_transceiver_mcast_filter_t mcast)
{
  return sli_wifi_set_transceiver_multicast_filter(interface, mcast);
}

sl_status_t sl_wifi_flush_transceiver_data(sl_wifi_interface_t interface)
{
  return sli_wifi_flush_transceiver_data(interface);
}

sl_status_t sl_wifi_configure_multicast_filter(sl_wifi_multicast_filter_info_t *multicast_filter_info)
{
  return sli_wifi_configure_multicast_filter(multicast_filter_info);
}

sl_status_t sl_wifi_transmit_cw_tone_start(sl_wifi_interface_t interface, sl_wifi_cw_tone_config_t cw_tone_config)
{
  UNUSED_PARAMETER(interface);
  UNUSED_PARAMETER(cw_tone_config);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_wifi_transmit_cw_tone_stop(sl_wifi_interface_t interface)
{
  UNUSED_PARAMETER(interface);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_wifi_set_tx_powerdBm(int16_t txPower)
{
  UNUSED_PARAMETER(txPower);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_wifi_config_xo_ctune(sl_wifi_interface_t interface,
                                    sl_wifi_response_get_ctune_data_t *xo_ctune,
                                    uint32_t ctune_data)
{
  UNUSED_PARAMETER(interface);
  UNUSED_PARAMETER(xo_ctune);
  UNUSED_PARAMETER(ctune_data);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_wifi_read_ctune(sl_wifi_interface_t interface,
                               sl_wifi_response_get_ctune_data_t *get_xo_ctune,
                               const uint32_t *ctune_data)
{
  UNUSED_PARAMETER(interface);
  UNUSED_PARAMETER(get_xo_ctune);
  UNUSED_PARAMETER(ctune_data);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_wifi_stop_rx(sl_wifi_interface_t interface)
{
  UNUSED_PARAMETER(interface);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_wifi_add_vendor_ie(sl_wifi_vendor_ie_t *vendor_ie, uint8_t *fw_unique_id)
{
  return sli_wifi_add_vendor_ie(vendor_ie, fw_unique_id);
}

sl_status_t sl_wifi_remove_vendor_ie(uint8_t unique_id)
{
  return sli_wifi_remove_vendor_ie(unique_id);
}

sl_status_t sl_wifi_remove_all_vendor_ie(void)
{
  return sli_wifi_remove_all_vendor_ie();
}

sl_status_t sl_wifi_set_join_configuration(sl_wifi_interface_t interface, uint8_t join_feature_bitmap)
{
  return sli_wifi_set_join_configuration(interface, join_feature_bitmap);
}

sl_status_t sl_wifi_get_join_configuration(sl_wifi_interface_t interface, uint8_t *join_feature_bitmap)
{
  return sli_wifi_get_join_configuration(interface, join_feature_bitmap);
}