/***************************************************************************/ /**
 * @file
 * @brief HTTP OTAF Example Application
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "cmsis_os2.h"
#include "sl_board_configuration.h"
#include "sl_net.h"
#include "sl_wifi_types.h"
#include <string.h>
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "firmware_upgradation.h"
#include "sl_net_dns.h"
#include "sl_utility.h"

// include certificates
#include "aws_starfield_ca.pem.h"
#include "silabs_dgcert_ca.pem.h"
#include "cacert.pem.h"

#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_hal_soc_soft_reset.h"
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
//! Type of FW update
#define M4_FW_UPDATE       0
#define TA_FW_UPDATE       1
#define TWT_SCAN_TIMEOUT   10000
#define TWT_AUTO_CONFIG    1
#define COMBINED_FW_UPDATE 2

// Use case based TWT selection params
#define DEVICE_AVERAGE_THROUGHPUT            20000
#define ESTIMATE_EXTRA_WAKE_DURATION_PERCENT 0
#define TWT_TOLERABLE_DEVIATION              10
#define TWT_DEFAULT_WAKE_INTERVAL_MS         1024     // in milli seconds
#define TWT_DEFAULT_WAKE_DURATION_MS         8        // in milli seconds
#define MAX_TX_AND_RX_LATENCY_LIMIT          22118400 // 6hrs in milli seconds
#define MAX_BEACON_WAKE_UP_AFTER_SP \
  2 // The number of beacons after the service period completion for which the module wakes up and listens for any pending RX.

//! Set FW update type
#define FW_UPDATE_TYPE TA_FW_UPDATE

//! Load certificate to device flash :
//! Certificate should be loaded once and need not be loaded for every boot up
#define LOAD_CERTIFICATE 1

// Macro to set specified bit position
#define BIT(a) ((uint32_t)1U << a)

//! Enable IPv6 set this bit in FLAGS, Default is IPv4
#define HTTPV6 BIT(3)

//! Set HTTPS_SUPPORT to use HTTPS feature
#define HTTPS_SUPPORT BIT(0)

//! Set HTTP_POST_DATA to use HTTP POST LARGE DATA feature
#define HTTP_POST_DATA BIT(5)

//! Set HTTP_V_1_1 to use HTTP version 1.1
#define HTTP_V_1_1 BIT(6)

//! Enable user defined http content type in FLAGS
#define HTTP_USER_DEFINED_CONTENT_TYPE BIT(7)

// HTTP OTAF
#define HTTP_OTAF 2

#define DNS_TIMEOUT         20000
#define MAX_DNS_RETRY_COUNT 5
#define OTAF_TIMEOUT        600000
#ifdef AWS_ENABLE
//! for example select required flag bits,  Eg:(HTTPS_SUPPORT | HTTPV6 | HTTP_USER_DEFINED_CONTENT_TYPE)
#define FLAGS HTTPS_SUPPORT
//! Server port number
#define HTTP_PORT 443
//! Server URL
#if (FW_UPDATE_TYPE == TA_FW_UPDATE)
#define HTTP_URL "SiWG917-A.2.9.0.0.16.rps"
#else
#define HTTP_URL "wifi_wlan_throughput_isp.bin"
#endif
//! Server Hostname
char *hostname = "otafaws.s3.ap-south-1.amazonaws.com";
//! set HTTP extended header
//! if NULL , driver fills default extended header
#define HTTP_EXTENDED_HEADER NULL
//! set Username
#define USERNAME ""
//! set Password
#define PASSWORD ""

#elif AZURE_ENABLE
//! for example select required flag bits,  Eg:(HTTPS_SUPPORT | HTTPV6 | HTTP_USER_DEFINED_CONTENT_TYPE)
#define FLAGS                HTTPS_SUPPORT
//! Server port number
#define HTTP_PORT            443
//! Server URL
#define HTTP_URL             "rps/firmware.rps"
//! Server Hostname
char *hostname = "si917updates.blob.core.windows.net";
//! set HTTP extended header
#define HTTP_EXTENDED_HEADER NULL
//! set Username
#define USERNAME             ""
//! set Password
#define PASSWORD             ""
#else
#define FLAGS                  0
//! Server port number
#define HTTP_PORT              80
//! HTTP Server IP address.
#define HTTP_SERVER_IP_ADDRESS "192.168.0.100"
//! HTTP resource name
#if (FW_UPDATE_TYPE == TA_FW_UPDATE)
#define HTTP_URL "SiWG917-A.2.9.0.0.16.rps"
#else
#define HTTP_URL "wifi_access_point_isp.bin"
#endif
//! set HTTP hostname
#define HTTP_HOSTNAME        "192.168.0.100"
char *hostname = HTTP_HOSTNAME;
//! set HTTP extended header
//! if NULL , driver fills default extended header
#define HTTP_EXTENDED_HEADER NULL
//! set HTTP hostname
#define USERNAME             "admin"
//! set HTTP hostname
#define PASSWORD             "admin"
#endif

/******************************************************
 *               Variable Definitions
 ******************************************************/
const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};

static const sl_wifi_device_configuration_t station_init_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
                     (SL_WIFI_FEAT_SECURITY_PSK | SL_WIFI_FEAT_AGGREGATION | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
#ifdef RSI_M4_INTERFACE
                      | SL_WIFI_FEAT_WPS_DISABLE
#endif
                      ),
                   .tcp_ip_feature_bit_map     = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_HTTP_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID | SL_SI91X_TCP_IP_FEAT_SSL
                                              | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT),
                   .custom_feature_bit_map     = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map = (SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK
                                                  | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
#ifdef SLI_SI917
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_HTTP_OTAF_SUPPORT | SL_SI91X_EXT_TCP_IP_SSL_16K_RECORD
                      | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | SL_WIFI_ENABLE_ENHANCED_MAX_PSP) }
};

sl_wifi_twt_request_t default_twt_setup_configuration = {
  .twt_enable              = 1,
  .twt_flow_id             = 1,
  .wake_duration           = 0x60,
  .wake_duration_unit      = 0,
  .wake_duration_tol       = 0x60,
  .wake_int_exp            = 13,
  .wake_int_exp_tol        = 13,
  .wake_int_mantissa       = 0x1D4C,
  .wake_int_mantissa_tol   = 0x1D4C,
  .implicit_twt            = 1,
  .un_announced_twt        = 1,
  .triggered_twt           = 0,
  .twt_channel             = 0,
  .twt_protection          = 0,
  .restrict_tx_outside_tsp = 1,
  .twt_retry_limit         = 6,
  .twt_retry_interval      = 10,
  .req_type                = 1,
  .negotiation_type        = 0,
};

sl_wifi_twt_selection_t default_twt_selection_configuration = {
  .twt_enable                            = 1,
  .average_tx_throughput                 = 1000,
  .tx_latency                            = 0,
  .rx_latency                            = 5000,
  .device_average_throughput             = DEVICE_AVERAGE_THROUGHPUT,
  .estimated_extra_wake_duration_percent = ESTIMATE_EXTRA_WAKE_DURATION_PERCENT,
  .twt_tolerable_deviation               = TWT_TOLERABLE_DEVIATION,
  .default_wake_interval_ms              = TWT_DEFAULT_WAKE_INTERVAL_MS,
  .default_minimum_wake_duration_ms      = TWT_DEFAULT_WAKE_DURATION_MS,
  .beacon_wake_up_count_after_sp         = MAX_BEACON_WAKE_UP_AFTER_SP
};

int twt_active_session               = 0;
int power_save_enabled               = 0;
volatile bool response               = false;
volatile sl_status_t callback_status = SL_STATUS_OK;
sl_wifi_twt_response_t twt_response;

/******************************************************
 *               Function Declarations
 ******************************************************/
void application_start(const void *unused);
sl_status_t http_otaf_app();
static sl_status_t http_fw_update_response_handler(sl_wifi_event_t event,
                                                   sl_status_t status_code,
                                                   uint16_t *data,
                                                   uint32_t data_length,
                                                   void *arg);
sl_status_t set_twt(void);
static sl_status_t twt_callback_handler(sl_wifi_event_t event,
                                        sl_status_t status_code,
                                        sl_wifi_twt_response_t *result,
                                        uint32_t result_length,
                                        void *arg);
#if LOAD_CERTIFICATE
static sl_status_t clear_and_load_certificates_in_flash(void);
#endif

/******************************************************
 *               Function Definitions
 ******************************************************/
void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

void application_start(const void *unused)
{
  UNUSED_PARAMETER(unused);
  sl_status_t status = SL_STATUS_OK;

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &station_init_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lX\r\n", status);
    return;
  }
  printf("\r\nWi-Fi Init is successful\r\n");

#if LOAD_CERTIFICATE
  if (FLAGS & HTTPS_SUPPORT) {
    status = clear_and_load_certificates_in_flash();
    if (status != SL_STATUS_OK) {
      printf("\r\nUnexpected error while loading certificate: 0x%lX\r\n", status);
      return;
    }
  }
#endif

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring Wi-Fi client interface up: 0x%lX\r\n", status);
    return;
  }
  printf("\r\nConnected to Wi-Fi\r\n");

  status = http_otaf_app();
  if (status != SL_STATUS_OK) {
    printf("\r\nFirmware update failed: 0x%lX\r\n", status);
    return;
  }
  printf("\r\nFirmware update is successful\r\n");
}

#if LOAD_CERTIFICATE
sl_status_t clear_and_load_certificates_in_flash(void)
{
  sl_status_t status;
  void *cert           = NULL;
  uint32_t cert_length = 0;

#ifdef AWS_ENABLE
  cert        = (void *)aws_starfield_ca;
  cert_length = (sizeof(aws_starfield_ca) - 1);
#elif AZURE_ENABLE
  cert        = (void *)silabs_dgcert_ca;
  cert_length = (sizeof(silabs_dgcert_ca) - 1);
#else
  cert        = (uint8_t *)cacert;
  cert_length = (sizeof(cacert) - 1);
#endif

  // Load SSL CA certificate
  status = sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0), SL_NET_SIGNING_CERTIFICATE, cert, cert_length);
  if (status != SL_STATUS_OK) {
    printf("\r\nLoading TLS CA certificate in to FLASH Failed, Error Code : 0x%lX\r\n", status);
  } else {
    printf("\r\nLoad TLS CA certificate at index %d Success\r\n", 0);
  }

  return status;
}
#endif

sl_status_t http_otaf_app()
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t flags      = FLAGS;
  char server_ip[16];
  sl_wifi_performance_profile_v2_t performance_profile = { 0 };

#if (FW_UPDATE_TYPE == TA_FW_UPDATE)
  sl_wifi_firmware_version_t version = { 0 };
  status                             = sl_wifi_get_firmware_version(&version);
  VERIFY_STATUS_AND_RETURN(status);
  print_firmware_version(&version);
#endif

  sl_wifi_set_callback_v2(SL_WIFI_HTTP_OTA_FW_UPDATE_EVENTS,
                          (sl_wifi_callback_function_v2_t)&http_fw_update_response_handler,
                          NULL);

#if defined(AWS_ENABLE) || defined(AZURE_ENABLE)
  sl_ip_address_t dns_query_rsp = { 0 };
  uint32_t server_address;
  int32_t dns_retry_count = MAX_DNS_RETRY_COUNT;
  do {
    //! Getting IP address of the AWS server using DNS request
    status = sl_net_dns_resolve_hostname((const char *)hostname, DNS_TIMEOUT, SL_NET_DNS_TYPE_IPV4, &dns_query_rsp);
    dns_retry_count--;
  } while ((dns_retry_count != 0) && (status != SL_STATUS_OK));

  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error while resolving dns, Error 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nResolving dns Success\r\n");

  server_address = dns_query_rsp.ip.v4.value;
  sprintf((char *)server_ip,
          "%ld.%ld.%ld.%ld",
          server_address & 0x000000ff,
          (server_address & 0x0000ff00) >> 8,
          (server_address & 0x00ff0000) >> 16,
          (server_address & 0xff000000) >> 24);

#ifdef AWS_ENABLE
  printf("\nResolved AWS S3 Bucket IP address = %s\n", server_ip);
  printf("\r\nFirmware download from AWS S3 Bucket is in progress...\r\n");
#elif AZURE_ENABLE
  printf("\nResolved AZURE Blob Storage IP address = %s\n", server_ip);
  printf("\r\nFirmware download from AZURE Blob Storage is in progress...\r\n");
#endif

#else
  strcpy(server_ip, HTTP_SERVER_IP_ADDRESS);
  printf("\r\nLocal Apache Server IP Address: %s\r\n", HTTP_HOSTNAME);
#endif
  status = set_twt();
  if (status != SL_STATUS_OK) {
    printf("\r\nError while configuring TWT parameters: 0x%lx \r\n", status);
    return status;
  }
  printf("\r\nTWT Config Done\r\n");

  if (twt_active_session == 1) {
    status = sl_wifi_reschedule_twt(twt_response.twt_flow_id, SL_WIFI_SUSPEND_INDEFINITELY, 0);
    if (status != SL_STATUS_OK) {
      printf("\r\nSuspending TWT Failed: 0x%lx \r\n", status);
      return status;
    } else {
      twt_active_session = 0;
      printf("\r\nSuspend TWT Done\r\n");
    }
  }

  if (power_save_enabled == 1) {
    performance_profile.profile = HIGH_PERFORMANCE;
    status                      = sl_wifi_set_performance_profile_v2(&performance_profile);
    if (status != SL_STATUS_OK) {
      printf("\r\nPowersave Disabling Failed, Error Code : 0x%lX\r\n", status);
      return status;
    } else {
      power_save_enabled = 0;
      printf("\r\nPower Save Disabled\n");
    }
  }

#ifdef AWS_ENABLE
  printf("\r\nFirmware download from AWS S3 Bucket is in progress...\r\n");
#elif AZURE_ENABLE
  printf("\r\nFirmware download from AZURE Blob Storage is in progress...\r\n");
#else
  printf("\r\nFirmware download from Local Apache Server is in progress...\r\n");
#endif
  sl_si91x_http_otaf_params_t http_params = { 0 };

  http_params.flags           = (uint8_t)flags;
  http_params.ip_address      = (uint8_t *)server_ip;
  http_params.port            = (uint16_t)HTTP_PORT;
  http_params.resource        = (uint8_t *)HTTP_URL;
  http_params.host_name       = (uint8_t *)hostname;
  http_params.extended_header = (uint8_t *)HTTP_EXTENDED_HEADER;
  http_params.user_name       = (uint8_t *)USERNAME;
  http_params.password        = (uint8_t *)PASSWORD;

  status = sl_si91x_http_otaf_v2(&http_params);

  printf("\r\nFirmware update status: 0x%lX\r\n", status);
  if (SL_STATUS_IN_PROGRESS == status) {
    const uint32_t start = osKernelGetTickCount();

    while (!response && (osKernelGetTickCount() - start) <= OTAF_TIMEOUT) {
      osThreadYield();
    }

    status = response ? callback_status : SL_STATUS_TIMEOUT;
  }

  if (status != SL_STATUS_OK) {
    printf("\r\nFirmware update FAILED with error: 0x%lX\r\n", status);
    return status;
  } else {
#ifdef AWS_ENABLE
    printf("\r\nCompleted firmware download using AWS\r\n");
#elif AZURE_ENABLE
    printf("\r\nCompleted firmware download using AZURE\r\n");
#else
    printf("\r\nCompleted firmware download using Local Apache Server\r\n");
#endif
    printf("\r\nUpdating the firmware...\r\n");
  }

#if (FW_UPDATE_TYPE == TA_FW_UPDATE)
  status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
  if (status != SL_STATUS_OK) {
    printf("\r\nError while wifi deinit: 0x%lX \r\n", status);
    return status;
  }
  printf("\r\nWi-Fi Deinit is successful\r\n");

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &station_init_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nWi-Fi Init success\r\n");

  status = sl_wifi_get_firmware_version(&version);
  VERIFY_STATUS_AND_RETURN(status);
  print_firmware_version(&version);
#else
  printf("\r\nSoC Soft Reset initiated!\r\n");
  sl_si91x_soc_nvic_reset();
#endif

  return status;
}

sl_status_t set_twt(void)
{
  sl_wifi_performance_profile_v2_t performance_profile = { 0 };
  sl_status_t status                                   = SL_STATUS_OK;

  //! Set TWT Config
  sl_wifi_set_twt_config_callback_v2(twt_callback_handler, NULL);
  if (TWT_AUTO_CONFIG == 1) {
    performance_profile.twt_selection = default_twt_selection_configuration;
    status                            = sl_wifi_target_wake_time_auto_selection(&performance_profile.twt_selection);
  } else {
    performance_profile.twt_request = default_twt_setup_configuration;
    status                          = sl_wifi_enable_target_wake_time(&performance_profile.twt_request);
  }
  VERIFY_STATUS_AND_RETURN(status);
  // A small delay is added so that the asynchronous response from TWT is printed in correct format.
  osDelay(100);

  //! Enable Broadcast data filter
  status = sl_wifi_filter_broadcast(5000, 1, 1);
  VERIFY_STATUS_AND_RETURN(status);
  printf("\r\nEnabled Broadcast Data Filter\n");

  //! Apply power save profile
  performance_profile.profile = ASSOCIATED_POWER_SAVE_LOW_LATENCY;
  status                      = sl_wifi_set_performance_profile_v2(&performance_profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nPowersave Configuration Failed, Error Code : 0x%lX\r\n", status);
    return status;
  } else {
    power_save_enabled = 1;
    printf("\r\nAssociated Power Save Enabled\n");
  }
  return SL_STATUS_OK;
}

static sl_status_t twt_callback_handler(sl_wifi_event_t event,
                                        sl_status_t status_code,
                                        sl_wifi_twt_response_t *result,
                                        uint32_t result_length,
                                        void *arg)
{
  UNUSED_PARAMETER(result_length);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }

  twt_active_session = 0;
  memcpy(&twt_response, result, sizeof(sl_wifi_twt_response_t));
  switch (event) {
    case SL_WIFI_TWT_RESPONSE_EVENT:
      twt_active_session = 1;
      printf("\r\nTWT Setup success");
      break;
    case SL_WIFI_TWT_UNSOLICITED_SESSION_SUCCESS_EVENT:
      twt_active_session = 1;
      printf("\r\nUnsolicited TWT Setup success");
      break;
    case SL_WIFI_TWT_AP_REJECTED_EVENT:
      printf("\r\nTWT Setup Failed. TWT Setup rejected by AP");
      break;
    case SL_WIFI_TWT_OUT_OF_TOLERANCE_EVENT:
      printf("\r\nTWT Setup Failed. TWT response out of tolerance limits");
      break;
    case SL_WIFI_TWT_RESPONSE_NOT_MATCHED_EVENT:
      printf("\r\nTWT Setup Failed. TWT Response not matched with the request parameters");
      break;
    case SL_WIFI_TWT_UNSUPPORTED_RESPONSE_EVENT:
      printf("\r\nTWT Setup Failed. TWT Response Unsupported");
      break;
    case SL_WIFI_TWT_FAIL_MAX_RETRIES_REACHED_EVENT:
      printf("\r\nTWT Setup Failed. Max retries reached");
      break;
    case SL_WIFI_TWT_INACTIVE_DUE_TO_ROAMING_EVENT:
      printf("\r\nTWT session inactive due to roaming");
      break;
    case SL_WIFI_TWT_INACTIVE_DUE_TO_DISCONNECT_EVENT:
      printf("\r\nTWT session inactive due to wlan disconnection");
      break;
    case SL_WIFI_TWT_TEARDOWN_SUCCESS_EVENT:
      printf("\r\nTWT session teardown success");
      break;
    case SL_WIFI_TWT_AP_TEARDOWN_SUCCESS_EVENT:
      printf("\r\nTWT session teardown from AP");
      break;
    case SL_WIFI_TWT_INACTIVE_NO_AP_SUPPORT_EVENT:
      printf("\r\nConnected AP Does not support TWT");
      break;
    case SL_WIFI_RESCHEDULE_TWT_SUCCESS_EVENT:
      printf("\r\nTWT rescheduled");
      break;
    case SL_WIFI_TWT_INFO_FRAME_EXCHANGE_FAILED_EVENT:
      printf("\r\nTWT rescheduling failed due to a failure in the exchange of TWT information frames.");
      break;
    default:
      printf("TWT Setup Failed.");
  }
  if (event < SL_WIFI_TWT_TEARDOWN_SUCCESS_EVENT) {
    printf("\r\n wake duration : 0x%X", result->wake_duration);
    printf("\r\n wake_duration_unit: 0x%X", result->wake_duration_unit);
    printf("\r\n wake_int_exp : 0x%X", result->wake_int_exp);
    printf("\r\n negotiation_type : 0x%X", result->negotiation_type);
    printf("\r\n wake_int_mantissa : 0x%X", result->wake_int_mantissa);
    printf("\r\n implicit_twt : 0x%X", result->implicit_twt);
    printf("\r\n un_announced_twt : 0x%X", result->un_announced_twt);
    printf("\r\n triggered_twt : 0x%X", result->triggered_twt);
    printf("\r\n twt_channel : 0x%X", result->twt_channel);
    printf("\r\n twt_protection : 0x%X", result->twt_protection);
    printf("\r\n twt_flow_id : 0x%X\r\n", result->twt_flow_id);
  } else if (event < SL_WIFI_TWT_EVENTS_END) {
    printf("\r\n twt_flow_id : 0x%X", result->twt_flow_id);
    printf("\r\n negotiation_type : 0x%X\r\n", result->negotiation_type);
  }
  return SL_STATUS_OK;
}

static sl_status_t http_fw_update_response_handler(sl_wifi_event_t event,
                                                   sl_status_t status_code,
                                                   uint16_t *data,
                                                   uint32_t data_length,
                                                   void *arg)
{
  UNUSED_PARAMETER(data);
  UNUSED_PARAMETER(data_length);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    response = false;
    return status_code;
  }
  response = true;
  return SL_STATUS_OK;
}
