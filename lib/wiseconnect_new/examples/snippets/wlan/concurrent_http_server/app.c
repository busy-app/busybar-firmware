/***************************************************************************/ /**
 * @file
 * @brief Concurrent HTTP Server Example Application
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

#include "sl_net.h"
#include "cmsis_os2.h"
#include "sl_wifi_device.h"
#include "sl_utility.h"
#include "sl_wifi.h"
#include "sl_net_wifi_types.h"
#include "sl_wifi_callback_framework.h"
#include "sl_http_server.h"
#include "sl_net_si91x.h"
#include "sl_si91x_driver.h"
#include "login.h"
#include "jsmn.h"
#include <string.h>
/******************************************************
 *                      Macros
 ******************************************************/
#define WIFI_AP_PROFILE_SSID "YOUR_AP_SSID"
#define WIFI_AP_CREDENTIAL   "YOUR_AP_PASSPHRASE"

//! IP address of the module
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#define DEFAULT_WIFI_MODULE_IP_ADDRESS 0x0A0AA8C0

//! IP address of netmask
//! E.g: 0x00FFFFFF == 255.255.255.0
#define DEFAULT_WIFI_SN_MASK_ADDRESS 0x00FFFFFF

//! IP address of Gateway
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#define DEFAULT_WIFI_GATEWAY_ADDRESS 0x0A0AA8C0

#define HTTP_SERVER_PORT 80

#define CHANNEL_NUMBER 1

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

volatile bool is_server_running = false;
char WIFI_CLIENT_PROFILE_SSID[32]; // Assuming SSID can be up to 32 characters long
char WIFI_CLIENT_CREDENTIAL[64];   // Assuming Password can be up to 64 characters long
char WIFI_CLIENT_SECURITY_TYPE[32];

static sl_net_wifi_ap_profile_t wifi_ap_profile = {
    .config = {
        .ssid.value = WIFI_AP_PROFILE_SSID,
        .ssid.length = sizeof(WIFI_AP_PROFILE_SSID)-1,
        .channel.channel = SL_WIFI_AUTO_CHANNEL,
        .channel.band = SL_WIFI_AUTO_BAND,
        .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH,
        .security = SL_WIFI_WPA2,
        .encryption = SL_WIFI_CCMP_ENCRYPTION,
        .rate_protocol = SL_WIFI_RATE_PROTOCOL_AUTO,
        .options = 0,
        .credential_id = SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID,
        .keepalive_type = SL_SI91X_AP_NULL_BASED_KEEP_ALIVE,
        .beacon_interval = 100,
        .client_idle_timeout = 0xFF,
        .dtim_beacon_count = 3,           
        .maximum_clients = 3,              
        .beacon_stop = 0,
        .is_11n_enabled = 0,
    },
    .ip = {
        .mode = SL_IP_MANAGEMENT_STATIC_IP,
        .type = SL_IPV4,
        .host_name = NULL,
        .ip = {
            .v4.ip_address.value = DEFAULT_WIFI_MODULE_IP_ADDRESS,
        .v4.gateway.value = DEFAULT_WIFI_GATEWAY_ADDRESS,
        .v4.netmask.value = DEFAULT_WIFI_SN_MASK_ADDRESS
        },
    }
};

sl_net_wifi_psk_credential_entry_t wifi_ap_credential = { .type        = SL_NET_WIFI_PSK,
                                                          .data_length = sizeof(WIFI_AP_CREDENTIAL) - 1,
                                                          .data        = WIFI_AP_CREDENTIAL };

static jsmntok_t t[128]; /* We expect no more than 128 tokens */
static sl_http_server_t server_handle = { 0 };
/******************************************************
 *               Function Declarations
 ******************************************************/

static sl_status_t ap_connected_event_handler(sl_wifi_event_t event,
                                              sl_status_t status_code,
                                              void *data,
                                              uint32_t data_length,
                                              void *arg);
static sl_status_t ap_disconnected_event_handler(sl_wifi_event_t event,
                                                 sl_status_t status_code,
                                                 void *data,
                                                 uint32_t data_length,
                                                 void *arg);
static void application_start(void *argument);

sl_status_t login_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
sl_status_t connect_data_handler(sl_http_server_t *handle, sl_http_server_request_t *req);

static sl_http_server_handler_t request_handlers[2] = { { .uri = "/login", .handler = login_request_handler },
                                                        { .uri = "/connect", .handler = connect_data_handler } };

/******************************************************
 *               Function Definitions
 ******************************************************/
void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static char response[300] = { 0 };
sl_status_t login_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = { 0 };

  printf("Got request %s with data length : %lu\n", req->uri.path, req->request_data_length);

  // Set the response code to 200 (OK)
  http_response.response_code = SL_HTTP_RESPONSE_OK;

  // Set the content type to plain text
  http_response.content_type = SL_HTTP_CONTENT_TYPE_TEXT_HTML;

  // Set the response data which is the connect page
  http_response.data                 = (uint8_t *)login_content;
  http_response.current_data_length  = strlen((const char *)login_content);
  http_response.expected_data_length = http_response.current_data_length;
  sl_http_server_send_response(handle, &http_response);
  return SL_STATUS_OK;
}

sl_status_t connect_data_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_recv_req_data_t recvData        = { 0 };
  sl_http_server_response_t http_response = { 0 };

  printf("Got request %s with data length : %lu\n", req->uri.path, req->request_data_length);
  if (req->request_data_length > 0) {
    recvData.request       = req;
    recvData.buffer        = (uint8_t *)response;
    recvData.buffer_length = 1024;

    sl_http_server_read_request_data(handle, &recvData);
    response[recvData.received_data_length] = 0;
    printf("Got request data as : %s\n", response);
  }

  // Set the response code to 200 (OK)
  http_response.response_code = SL_HTTP_RESPONSE_OK;

  // Set the content type to plain text
  http_response.content_type = SL_HTTP_CONTENT_TYPE_TEXT_HTML;

  // Set the response data as "Done"
  char *response_data                = "Done";
  http_response.data                 = (uint8_t *)response_data;
  http_response.current_data_length  = strlen(response_data);
  http_response.expected_data_length = http_response.current_data_length;
  sl_http_server_send_response(handle, &http_response);
  is_server_running = false;
  return SL_STATUS_OK;
}

sl_status_t default_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_recv_req_data_t recvData        = { 0 };
  sl_http_server_response_t http_response = { 0 };
  sl_http_header_t header                 = { .key = "Server", .value = "SI917-HTTPServer" };

  printf("Got request %s with data length : %lu\n", req->uri.path, req->request_data_length);
  if (req->request_data_length > 0) {
    recvData.request       = req;
    recvData.buffer        = (uint8_t *)response;
    recvData.buffer_length = 1024;

    sl_http_server_read_request_data(handle, &recvData);
    response[recvData.received_data_length] = 0;
    printf("Got request data as : %s\n", response);
  }

  http_response.response_code = SL_HTTP_RESPONSE_NOT_FOUND;
  http_response.content_type  = SL_HTTP_CONTENT_TYPE_TEXT_PLAIN;
  http_response.headers       = &header;
  http_response.header_count  = 1;

  // Set the response data to "Hello, World!"
  char *response_data                = "Invalid request!";
  http_response.data                 = (uint8_t *)response_data;
  http_response.current_data_length  = strlen(response_data);
  http_response.expected_data_length = http_response.current_data_length;
  sl_http_server_send_response(handle, &http_response);

  return SL_STATUS_OK;
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start
      && strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

void parse_json_response()
{
  int i;
  int r;
  jsmn_parser p;

  jsmn_init(&p);
  r = jsmn_parse(&p, response, strlen(response), t, sizeof(t) / sizeof(t[0]));
  if (r < 0) {
    printf("Failed to parse JSON: %d\n", r);
    return;
  }

  /* Assume the top-level element is an object */
  if (r < 1 || t[0].type != JSMN_OBJECT) {
    printf("Object expected\n");
    return;
  }

  /* Loop over all keys of the root object */
  for (i = 1; i < r; i++) {
    if (jsoneq(response, &t[i], "id") == 0) {
      /* We may use strndup() to fetch string value */
      SL_DEBUG_LOG("- SSID: %.*s\n", t[i + 1].end - t[i + 1].start, response + t[i + 1].start);
      snprintf(WIFI_CLIENT_PROFILE_SSID,
               sizeof(WIFI_CLIENT_PROFILE_SSID),
               "%.*s",
               t[i + 1].end - t[i + 1].start,
               response + t[i + 1].start);
      i++;
    } else if (jsoneq(response, &t[i], "password") == 0) {
      /* Only store password if it's not empty (not Open security) */
      if (t[i + 1].end - t[i + 1].start > 0) {
        SL_DEBUG_LOG("- Password: %.*s\n", t[i + 1].end - t[i + 1].start, response + t[i + 1].start);
        snprintf(WIFI_CLIENT_CREDENTIAL,
                 sizeof(WIFI_CLIENT_CREDENTIAL),
                 "%.*s",
                 t[i + 1].end - t[i + 1].start,
                 response + t[i + 1].start);
      } else {
        /* For Open networks, set empty password */
        WIFI_CLIENT_CREDENTIAL[0] = '\0';
        SL_DEBUG_LOG("- Password: (empty for Open network)\n");
      }
      i++;
    } else if (jsoneq(response, &t[i], "securityType") == 0) {
      /* We may want to do strtol() here to get numeric value */
      SL_DEBUG_LOG("- Security Type: %.*s\n", t[i + 1].end - t[i + 1].start, response + t[i + 1].start);
      snprintf(WIFI_CLIENT_SECURITY_TYPE,
               sizeof(WIFI_CLIENT_SECURITY_TYPE),
               "%.*s",
               t[i + 1].end - t[i + 1].start,
               response + t[i + 1].start);
      i++;
    } else {
      printf("Unexpected key: %.*s\n", t[i].end - t[i].start, response + t[i].start);
    }
  }
}

sl_wifi_security_t string_to_security_type(const char *securityType)
{
  if (strcmp(securityType, "Open") == 0) {
    return SL_WIFI_OPEN;
  } else if (strcmp(securityType, "WPA") == 0) {
    return SL_WIFI_WPA;
  } else if (strcmp(securityType, "WPA2") == 0) {
    return SL_WIFI_WPA2;
  } else if (strcmp(securityType, "Mixed Mode") == 0) {
    return SL_WIFI_WPA_WPA2_MIXED;
  } else if (strcmp(securityType, "WPA3") == 0) {
    return SL_WIFI_WPA3;
  } else if (strcmp(securityType, "WPA3_Transition") == 0) {
    return SL_WIFI_WPA3_TRANSITION;
  } else {
    return SL_WIFI_SECURITY_UNKNOWN;
  }
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;
  sl_net_wifi_client_profile_t profile = { 0 };
  sl_ip_address_t ip_address           = { 0 };

  status = sl_net_init(SL_NET_WIFI_AP_INTERFACE, &sl_wifi_default_concurrent_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to start Wi-Fi AP interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi AP interface init\r\n");

  sl_wifi_set_callback_v2(SL_WIFI_CLIENT_CONNECTED_EVENTS, ap_connected_event_handler, NULL);
  sl_wifi_set_callback_v2(SL_WIFI_CLIENT_DISCONNECTED_EVENTS, ap_disconnected_event_handler, NULL);

  wifi_ap_profile.config.channel.channel = CHANNEL_NUMBER;
  status = sl_net_set_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_ap_profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to set AP profile: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nSuccess to set AP profile \r\n");

  status = sl_net_set_credential(SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID,
                                 wifi_ap_credential.type,
                                 &wifi_ap_credential.data,
                                 wifi_ap_credential.data_length);
  if (status != SL_STATUS_OK) {
    printf("Failed to set credentials: 0x%lx\r\n", status);
    return;
  }
  printf("\nWi-Fi set credential success\n");

  status = sl_net_up(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring Wi-Fi AP interface up: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nAP started\r\n");

  sl_http_server_config_t server_config = { 0 };

  server_config.port             = HTTP_SERVER_PORT;
  server_config.default_handler  = default_handler;
  server_config.handlers_list    = request_handlers;
  server_config.handlers_count   = 2;
  server_config.client_idle_time = 1;

  status = sl_http_server_init(&server_handle, &server_config);
  if (status != SL_STATUS_OK) {
    printf("\r\nHTTP server init failed:%lx\r\n", status);
    return;
  }
  printf("\r\n Http Init done\r\n");

  status = sl_http_server_start(&server_handle);
  if (status != SL_STATUS_OK) {
    printf("\r\n Server start fail:%lx\r\n", status);
    return;
  }
  printf("\r\n Server start done\r\n");

  is_server_running = true;
  while (is_server_running) { //Server is exited when we receive the credentials
    osDelay(100);
  }

  parse_json_response();

  SL_DEBUG_LOG("SSID stored:%s\r\n", WIFI_CLIENT_PROFILE_SSID);
  SL_DEBUG_LOG("Password stored:%s\r\n", WIFI_CLIENT_CREDENTIAL);
  SL_DEBUG_LOG("Security type stored:%s\r\n", WIFI_CLIENT_SECURITY_TYPE);

  status = sl_http_server_stop(&server_handle);
  if (status != SL_STATUS_OK) {
    printf("\r\n Server stop fail:%lx\r\n", status);
    return;
  }
  printf("\r\n Server stop done\r\n");

  status = sl_http_server_deinit(&server_handle);
  if (status != SL_STATUS_OK) {
    printf("\r\n Server deinit fail:%lx\r\n", status);
    return;
  }
  printf("\r\n Server deinit done\r\n");

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_default_concurrent_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi Client interface init\r\n");

  sl_wifi_security_t sec_type = string_to_security_type(WIFI_CLIENT_SECURITY_TYPE);
  sl_wifi_credential_id_t id;
  sl_status_t cred_status = SL_STATUS_OK;

  if (sec_type == SL_WIFI_OPEN) {
    id = SL_NET_NO_CREDENTIAL_ID;
    SL_DEBUG_LOG("Connecting to Open network - no credentials required\n");
  } else {
    // Handle PSK-based security (WPA, WPA2, WPA3, etc.)
    id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;

    // Only set credentials if password is not empty
    if (strlen(WIFI_CLIENT_CREDENTIAL) > 0) {
      cred_status =
        sl_net_set_credential(id, SL_NET_WIFI_PSK, WIFI_CLIENT_CREDENTIAL, strlen((char *)WIFI_CLIENT_CREDENTIAL));
    } else {
      printf("Error: Password required for %s security\n", WIFI_CLIENT_SECURITY_TYPE);
      return;
    }
  }

  if (sec_type == SL_WIFI_OPEN || cred_status == SL_STATUS_OK) {
    SL_DEBUG_LOG("Credentials set, id : %lu\n", id);
    sl_wifi_client_configuration_t access_point = { 0 };
    access_point.ssid.length                    = strlen((char *)WIFI_CLIENT_PROFILE_SSID);
    memcpy(access_point.ssid.value, WIFI_CLIENT_PROFILE_SSID, access_point.ssid.length);
    access_point.security      = sec_type;
    access_point.encryption    = SL_WIFI_CCMP_ENCRYPTION;
    access_point.credential_id = id;

    status = sl_wifi_set_join_configuration(SL_WIFI_CLIENT_INTERFACE, SL_WIFI_JOIN_FEAT_LISTEN_INTERVAL_VALID);
    if (status != SL_STATUS_OK) {
      printf("Failed to start set join configuration: 0x%lx\r\n", status);
      return;
    }

    SL_DEBUG_LOG("SSID %s\n", access_point.ssid.value);
    do {
      status = sl_wifi_connect(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &access_point, 25000);
      if (status != SL_STATUS_OK) {
        printf("\r\nWLAN Connect Failed, Error Code : 0x%lX\r\n", status);
      } else {
        printf("\n WLAN connection is successful\n");

        status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_0, &profile);
        if (status != SL_STATUS_OK) {
          printf("\r\nFailed to get client profile: 0x%lx\r\n", status);
        }

        status = sl_si91x_configure_ip_address(&profile.ip, SL_SI91X_WIFI_CLIENT_VAP_ID);
        if (status != SL_STATUS_OK) {
          printf("\r\nIPv4 address configuration is failed : 0x%lx\r\n", status);
        }

        ip_address.type = SL_IPV4;
        memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
        print_sl_ip_address(&ip_address);
      }
    } while (status != SL_STATUS_OK);
  } else {
    printf("\r\nFailed to set credentials; status: %lu\n", cred_status);
  }
  printf("\r\nExample demonstration completed\r\n");
}

static sl_status_t ap_connected_event_handler(sl_wifi_event_t event,
                                              sl_status_t status_code,
                                              void *data,
                                              uint32_t data_length,
                                              void *arg)
{
  UNUSED_PARAMETER(data_length);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }

  printf("Remote Client connected: ");
  print_mac_address((sl_mac_address_t *)data);
  printf("\n");

  return SL_STATUS_OK;
}

static sl_status_t ap_disconnected_event_handler(sl_wifi_event_t event,
                                                 sl_status_t status_code,
                                                 void *data,
                                                 uint32_t data_length,
                                                 void *arg)
{
  UNUSED_PARAMETER(data_length);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }

  printf("Remote Client disconnected: ");
  print_mac_address((sl_mac_address_t *)data);
  printf("\n");

  return SL_STATUS_OK;
}
