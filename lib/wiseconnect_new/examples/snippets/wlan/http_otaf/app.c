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
#include "sl_net_si91x.h"
#include "sl_net_wifi_types.h"

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
#define COMBINED_FW_UPDATE 2

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

//! set 1 for selecting SL_SI91X_HTTPS_CERTIFICATE_INDEX_1, set 2 for selecting SL_SI91X_HTTPS_CERTIFICATE_INDEX_2
#define CERTIFICATE_INDEX 0

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
#define HTTP_URL "rps/firmware.rps"
#else
#define HTTP_URL "isp.bin"
#endif
//! Server Hostname
char *hostname = "otafaws.s3.ap-south-1.amazonaws.com";
//! set HTTP extended header
//! if NULL , driver fills default extended header
#define HTTP_EXTENDED_HEADER NULL
//! set Username
#define USERNAME ""
//! set Password
#define PASSWORD    ""
#define SERVER_NAME "AWS Server"

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
#define SERVER_NAME          "AZURE Server"
#else
#define FLAGS                  0
//! Server port number
#define HTTP_PORT              80
//! HTTP Server IP address.
#define HTTP_SERVER_IP_ADDRESS "192.168.0.100"
//! HTTP resource name
#if (FW_UPDATE_TYPE == TA_FW_UPDATE)
#define HTTP_URL "rps/firmware.rps"
#else
#define HTTP_URL "isp.bin"
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
#define SERVER_NAME          "Local Apache Server"
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
  .boot_config = { .oper_mode              = SL_SI91X_CLIENT_MODE,
                   .coex_mode              = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map        = (SL_WIFI_FEAT_SECURITY_PSK | SL_WIFI_FEAT_AGGREGATION),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_HTTP_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID | SL_SI91X_TCP_IP_FEAT_SSL
                                              | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT),
                   .custom_feature_bit_map = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
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
                   .config_feature_bit_map  = 0 }
};

//! Enumeration for states in application
typedef enum {
  WLAN_INITIAL_STATE = 0,
  WLAN_UNCONNECTED_STATE,
  WLAN_FIRMWARE_UPDATE,
  WLAN_NET_DOWN_STATE,
  WLAN_OTA_UPDATE_DONE,
} wlan_app_state_t;

volatile wlan_app_state_t app_state;
volatile bool response               = false;
volatile sl_status_t callback_status = SL_STATUS_OK;

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

  //! Load SSL CA certificate
  status = sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(CERTIFICATE_INDEX),
                                 SL_NET_SIGNING_CERTIFICATE,
                                 cert,
                                 cert_length);
  if (status != SL_STATUS_OK) {
    printf("\r\nLoading TLS CA certificate in to FLASH Failed, Error Code : 0x%lX\r\n", status);
  } else {
    printf("\r\nLoad TLS CA certificate at index %d Success\r\n", CERTIFICATE_INDEX);
  }

  return status;
}
#endif

sl_status_t join_callback_handler(sl_wifi_event_t event,
                                  sl_status_t status_code,
                                  char *result,
                                  uint32_t result_length,
                                  void *arg)
{
  UNUSED_PARAMETER(result);
  UNUSED_PARAMETER(arg);

  app_state = WLAN_UNCONNECTED_STATE;

  printf("\r\nIn Join CB\r\n");

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    printf("F: Initiating rejoin %lu bytes payload\n", result_length);
    return status_code;
  }
  return SL_STATUS_OK;
}

void application_start(const void *unused)
{
  UNUSED_PARAMETER(unused);
  sl_status_t status = SL_STATUS_OK;
  uint16_t flags     = FLAGS;
  char server_ip[16];

  if (CERTIFICATE_INDEX == 1) {
    flags |= SL_SI91X_HTTPS_CERTIFICATE_INDEX_1;
  } else if (CERTIFICATE_INDEX == 2) {
    flags |= SL_SI91X_HTTPS_CERTIFICATE_INDEX_2;
  }

#if (FW_UPDATE_TYPE == TA_FW_UPDATE)
  sl_wifi_firmware_version_t version = { 0 };
#endif
#if defined(AWS_ENABLE) || defined(AZURE_ENABLE)
  sl_ip_address_t dns_query_rsp = { 0 };
  uint32_t server_address;
  int32_t dns_retry_count = MAX_DNS_RETRY_COUNT;
#endif

  while (1) {
    switch (app_state) {
      case WLAN_INITIAL_STATE: {
        //! Client initialization
        status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &station_init_configuration, NULL, NULL);
        if (status != SL_STATUS_OK) {
          printf("\r\nWi-Fi Client initialization failed , Error Code: 0x%lX\r\n", status);
          return;
        }
        printf("\r\nWi-Fi Client initialization success\r\n");

        //! Load certificate
#if LOAD_CERTIFICATE
        if (FLAGS & HTTPS_SUPPORT) {
          status = clear_and_load_certificates_in_flash();
          if (status != SL_STATUS_OK) {
            printf("\r\nUnexpected error while loading certificate: 0x%lX\r\n", status);
            return;
          }
        }
#endif
        app_state = WLAN_UNCONNECTED_STATE;
      } break;
      case WLAN_UNCONNECTED_STATE: {
        sl_wifi_set_join_callback_v2(join_callback_handler, NULL);
        //! Bring up client interface
        status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
        if (status != SL_STATUS_OK && status != SL_STATUS_SI91X_SCAN_ISSUED_IN_ASSOCIATED_STATE) {
          printf("\r\nFailed to bring Wi-Fi client interface up: 0x%lX\r\n", status);
          app_state = WLAN_UNCONNECTED_STATE;
          break;
        }
        printf("\r\nWi-Fi Client interface up\r\n");
        app_state = WLAN_FIRMWARE_UPDATE;

      } break;
      case WLAN_FIRMWARE_UPDATE: {
#if (FW_UPDATE_TYPE == TA_FW_UPDATE)
        status = sl_wifi_get_firmware_version(&version);
        print_firmware_version(&version);
#endif

        sl_wifi_set_callback_v2(SL_WIFI_HTTP_OTA_FW_UPDATE_EVENTS,
                                (sl_wifi_callback_function_v2_t)&http_fw_update_response_handler,
                                NULL);

#if defined(AWS_ENABLE) || defined(AZURE_ENABLE)
        do {
          //! Getting IP address of the AWS server using DNS request
          status =
            sl_net_dns_resolve_hostname((const char *)hostname, DNS_TIMEOUT, SL_NET_DNS_TYPE_IPV4, &dns_query_rsp);
          dns_retry_count--;
        } while ((dns_retry_count != 0) && (status != SL_STATUS_OK));

        if (status != SL_STATUS_OK) {
          printf("\r\nUnexpected error while resolving dns, Error 0x%lX\r\n", status);
          return;
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
#elif AZURE_ENABLE
        printf("\nResolved AZURE Blob Storage IP address = %s\n", server_ip);
#endif

#else
        strcpy(server_ip, HTTP_SERVER_IP_ADDRESS);
        printf("\r\n%s IP Address : %s\r\n", SERVER_NAME, HTTP_HOSTNAME);
#endif
        printf("\r\nFirmware download from %s is in progress...\r\n", SERVER_NAME);

        sl_si91x_http_otaf_params_t http_params = { 0 };

        http_params.flags           = (uint16_t)flags;
        http_params.ip_address      = (uint8_t *)server_ip;
        http_params.port            = (uint16_t)HTTP_PORT;
        http_params.resource        = (uint8_t *)HTTP_URL;
        http_params.host_name       = (uint8_t *)hostname;
        http_params.extended_header = (uint8_t *)HTTP_EXTENDED_HEADER;
        http_params.user_name       = (uint8_t *)USERNAME;
        http_params.password        = (uint8_t *)PASSWORD;

        //! OTAF firmware upgrade
        status = sl_si91x_http_otaf_v2(&http_params);

        if (status != SL_STATUS_OK) {
          printf("\r\n Firmware update status = 0x%lX\r\n", status);
          app_state = WLAN_UNCONNECTED_STATE;
          break;
        } else {
          printf("\r\nCompleted firmware download using %s\r\n", SERVER_NAME);
          printf("\r\nUpdating the firmware...\r\n");
        }
        app_state = WLAN_NET_DOWN_STATE;
      } break;
      case WLAN_NET_DOWN_STATE: {
#if (FW_UPDATE_TYPE == TA_FW_UPDATE)
        //! Client deinitialization
        status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
        if (status != SL_STATUS_OK) {
          printf("\r\nError while wifi deinit: 0x%lX \r\n", status);
          return;
        }
        printf("\r\nWi-Fi Deinit is successful\r\n");

#if SL_NCP_UART_INTERFACE
        printf("Waiting for firmware upgrade to complete\n");
        osDelay(40000);
        printf("Waiting Done\n");
#endif
        //! Client initialization
        status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &station_init_configuration, NULL, NULL);
        if (status != SL_STATUS_OK) {
          printf("Failed to start Wi-Fi client interface: 0x%lX\r\n", status);
          return;
        }
        printf("\r\nWi-Fi Init success\r\n");

        //! Check firmware version
        status = sl_wifi_get_firmware_version(&version);
        print_firmware_version(&version);
#else
        printf("\r\nSoC Soft Reset initiated!\r\n");
        sl_si91x_soc_nvic_reset();
#endif
        app_state = WLAN_OTA_UPDATE_DONE;
      } break;
      default:
        break;
    }
  }
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
