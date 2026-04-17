/***************************************************************************/ /**
 * @file
 * @brief Wi-Fi Provisioning via Access Point Example Application
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

#include "stdbool.h"
#include "sl_net.h"
#include "app.h"
#include "errno.h"
#include "sl_utility.h"
#include "sl_wifi.h"
#include "sl_net_wifi_types.h"
#include "sl_si91x_socket_support.h"
#include "sl_si91x_socket_constants.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_socket.h"
#include "sl_net_si91x.h"
#include "login.h"
#include "provisioning.h"
#include "jsmn.h"
#include "sl_http_server.h"
#include "wifi_provisioning.h"

/******************************************************
 *                      Macros
 ******************************************************/

// HTTP Status Codes
#define METHOD_BAD_REQUEST           "400 Bad Request"
#define METHOD_NOT_ALLOWED           "405 Not Allowed"
#define METHOD_INTERNAL_SERVER_ERROR "500 Internal Server Error"

// HTTP Chunk Size
#define HTTP_CHUNK_SIZE 512

// Default HTTP Response for Method Not Allowed
#define DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED                                                                  \
  {                                                                                                               \
    .response_code = SL_HTTP_RESPONSE_METHOD_NOT_ALLOWED, .content_type = SL_HTTP_CONTENT_TYPE_TEXT_HTML,         \
    .headers = NULL, .header_count = 0, .data = (uint8_t *)METHOD_NOT_ALLOWED,                                    \
    .current_data_length = sizeof(METHOD_NOT_ALLOWED) - 1, .expected_data_length = sizeof(METHOD_NOT_ALLOWED) - 1 \
  }

// HTTP Server Port
#define HTTP_SERVER_PORT 80

// Server port number
#define SERVER_PORT 5000

// Buffer size for storing scan results
#define SCAN_RESULT_BUFFER_SIZE (2000)

// Constants for various states and settings
#define ON              "on"
#define OFF             "off"
#define OPEN            "Open"
#define WPA             "WPA"
#define WPA2            "WPA2"
#define WPA3            "WPA3"
#define WPA3_TRANSITION "WPA3_Transition"
#define MIXED_MODE      "Mixed Mode"
#define UNKNOWN         "Unknown"
#define SSID            "ssid"
#define SECURITY_TYPE   "security_type"
#define PASSPHRASE      "passphrase"

// Enumeration for states in the application
typedef enum {
  PROVISIONING_INIT_STATE,
  PROVISIONING_STATE,
  CONNECTING_STATE,
  CONNECTED_STATE,
  DISCONNECTING_STATE,
} app_state_t;

/******************************************************
 *               Function Declarations
 ******************************************************/

// Access Point (AP) event handlers
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

//Application start API
static void application_start(void *argument);

// Security type conversion functions
static sl_wifi_security_t string_to_security_type(const char *security_type);
static char *security_type_to_string(sl_wifi_security_t security_type);

// HTTP server request handlers
static sl_status_t index_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
static sl_status_t default_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
static sl_status_t connect_page_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
static sl_status_t connect_data_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
static sl_status_t wifi_scan_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);

/******************************************************
 *               Variable Definitions
 ******************************************************/

static bool scan_complete                                      = false;
static bool disconnect_complete                                = false;
static uint8_t retry                                           = 0;
static sl_http_server_t server_handle                          = { 0 };
static sl_status_t callback_status                             = SL_STATUS_OK;
static app_state_t app_state                                   = PROVISIONING_INIT_STATE;
static sl_wifi_client_configuration_t provisioned_access_point = { 0 };

char wifi_client_profile_ssid[32]; // Assuming SSID can be up to 32 characters long
char wifi_client_credential[64];   // Assuming Password can be up to 64 characters long
char wifi_client_security_type[32];

static const osThreadAttr_t thread_attributes = {
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

static sl_net_wifi_client_profile_t wifi_client_profile_4 = {
    .config = {
        .channel.channel = SL_WIFI_AUTO_CHANNEL,
        .channel.band = SL_WIFI_AUTO_BAND,
        .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH,
        .bssid = {{0}},
        .bss_type = SL_WIFI_BSS_TYPE_INFRASTRUCTURE,
        .client_options = 0,
        .credential_id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
    },
    .ip = {
        .mode = SL_IP_MANAGEMENT_DHCP,
        .type = SL_IPV4,
        .host_name = NULL,
        .ip = {{{0}}},
    }
};

static const sl_http_server_handler_t provisioning_server_request_handlers[] = {
  { .uri = "/", .handler = index_request_handler },
  { .uri = "/index.html", .handler = index_request_handler },
  { .uri = "/connect.html", .handler = connect_page_request_handler },
  { .uri = "/connect", .handler = connect_data_handler },
  { .uri = "/scan", .handler = wifi_scan_request_handler }
};

/******************************************************
 *               Function Definitions
 ******************************************************/

void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static sl_status_t join_callback_handler(sl_wifi_event_t event,
                                         sl_status_t status_code,
                                         char *result,
                                         uint32_t result_length,
                                         void *arg)
{
  UNUSED_PARAMETER(result);
  UNUSED_PARAMETER(arg);
  printf("in join CB\r\n");
  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    printf("F: Join Event received with %lu bytes payload\n", result_length);
    return status_code;
  }
  return SL_STATUS_OK;
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status                    = SL_STATUS_OK;
  sl_net_wifi_client_profile_t profile  = { 0 };
  sl_ip_address_t ip_address            = { 0 };
  sl_http_server_config_t server_config = { 0 };

  printf("\r\nWi-Fi Provisioning demo started\r\n");

  while (1) {
    switch (app_state) {
      case PROVISIONING_INIT_STATE: {
        // Initialize and start Wi-Fi AP (Access Point) interface
        sl_net_wifi_ap_profile_t ap_profile;

        // Initialize the Wi-Fi AP interface with default configuration
        status = sl_net_init(SL_NET_WIFI_AP_INTERFACE, (const void *)&sl_wifi_default_ap_configuration, NULL, NULL);
        if (status != SL_STATUS_OK) {
          printf("Failed to start Wi-Fi AP interface: 0x%lx\r\n", status);
          return;
        }

        // Set callbacks for AP client connection and disconnection events
        sl_wifi_set_callback_v2(SL_WIFI_CLIENT_CONNECTED_EVENTS, ap_connected_event_handler, NULL);
        sl_wifi_set_callback_v2(SL_WIFI_CLIENT_DISCONNECTED_EVENTS, ap_disconnected_event_handler, NULL);
        printf("Wi-Fi AP initialized\r\n");

        // Bring the Wi-Fi AP interface up
        status = sl_net_up(SL_NET_WIFI_AP_INTERFACE, SL_NET_DEFAULT_WIFI_AP_PROFILE_ID);
        if (status != SL_STATUS_OK) {
          printf("Failed to bring Wi-Fi AP interface up: 0x%lx\r\n", status);
          return;
        }
        printf("Wi-Fi AP started\r\n");

        // Configure and start the HTTP server for provisioning
        server_config.port            = HTTP_SERVER_PORT;
        server_config.default_handler = default_handler;
        server_config.handlers_list   = (sl_http_server_handler_t *)provisioning_server_request_handlers;
        server_config.handlers_count  = sizeof(provisioning_server_request_handlers) / sizeof(sl_http_server_handler_t);
        server_config.client_idle_time = 1; // 1 second timeout to prevent hanging on slow/dead clients

        status = sl_http_server_init(&server_handle, &server_config);
        if (status != SL_STATUS_OK) {
          printf("HTTP server init failed:%lx\r\n", status);
          return;
        }
        status = sl_http_server_start(&server_handle);
        if (status != SL_STATUS_OK) {
          printf("Server start fail:%lx\r\n", status);
          return;
        }
        printf("Provisioning HTTP server started\r\n");

        sl_net_get_profile(SL_NET_WIFI_AP_INTERFACE,
                           SL_NET_DEFAULT_WIFI_AP_PROFILE_ID,
                           (sl_net_profile_t *)&ap_profile);
        printf("\r\nConnect to access point \"%s\" from your device\r\n", ap_profile.config.ssid.value);
        printf("Go to http://%u.%u.%u.%u/ on your browser to provisioning of Wi-Fi\r\n",
               ap_profile.ip.ip.v4.ip_address.bytes[0],
               ap_profile.ip.ip.v4.ip_address.bytes[1],
               ap_profile.ip.ip.v4.ip_address.bytes[2],
               ap_profile.ip.ip.v4.ip_address.bytes[3]);
        app_state = PROVISIONING_STATE;
        break;
      }
      case CONNECTING_STATE: {
        // Transition from AP provisioning to client connection
        // Stop and deinitialize the HTTP server
        status = sl_http_server_stop(&server_handle);
        if (status != SL_STATUS_OK) {
          printf("Server stop fail:%lx\r\n", status);
          return;
        }
        status = sl_http_server_deinit(&server_handle);
        if (status != SL_STATUS_OK) {
          printf("Server deinit fail:%lx\r\n", status);
          return;
        }
        printf("HTTP Server deinitialized\r\n");

        status = sl_net_deinit(SL_NET_WIFI_AP_INTERFACE);
        if (status != SL_STATUS_OK) {
          printf("Ap deinit : 0x%lx\r\n", status);
          return;
        }
        printf("Wi-Fi AP deinitialized\r\n");

        // Initialize the Wi-Fi client interface
        status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_default_client_configuration, NULL, NULL);
        if (status != SL_STATUS_OK) {
          printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
          return;
        }
        printf("Wi-Fi client interface initialized\r\n");

        sl_wifi_security_t sec_type = string_to_security_type(wifi_client_security_type);
        sl_wifi_credential_id_t id;

        if (sec_type == SL_WIFI_OPEN) {
          id = SL_NET_NO_CREDENTIAL_ID;
        } else {
          // Handle PSK-based security (WPA, WPA2, WPA3, etc.)
          id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;
          status =
            sl_net_set_credential(id, SL_NET_WIFI_PSK, wifi_client_credential, strlen((char *)wifi_client_credential));
          if (status != SL_STATUS_OK) {
            printf("Failed to set Wi-Fi client credential: 0x%lx\r\n", status);
            return;
          }
        }

        memset(&provisioned_access_point, 0, sizeof(provisioned_access_point));
        provisioned_access_point.ssid.length = strlen((char *)wifi_client_profile_ssid);
        memcpy(provisioned_access_point.ssid.value, wifi_client_profile_ssid, provisioned_access_point.ssid.length);
        provisioned_access_point.security      = sec_type;
        provisioned_access_point.encryption    = SL_WIFI_CCMP_ENCRYPTION;
        provisioned_access_point.credential_id = id;

        wifi_client_profile_4.config.ssid.length = strlen((char *)wifi_client_profile_ssid);
        memcpy(wifi_client_profile_4.config.ssid.value,
               wifi_client_profile_ssid,
               wifi_client_profile_4.config.ssid.length);
        wifi_client_profile_4.config.security      = string_to_security_type(wifi_client_security_type);
        wifi_client_profile_4.config.credential_id = (wifi_client_profile_4.config.security == SL_WIFI_OPEN)
                                                       ? SL_NET_NO_CREDENTIAL_ID
                                                       : SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;

        //  Keeping the station ipv4 record in profile_id_0
        status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_0, &wifi_client_profile_4);
        if (status != SL_STATUS_OK) {
          printf("Failed to set client profile: 0x%lx\r\n", status);
          return;
        }
        printf("Wi-Fi set client profile v4 success\r\n");
        printf("SSID %s\r\n", provisioned_access_point.ssid.value);

        // Connect to the provisioned access point
        sl_wifi_set_join_callback_v2(join_callback_handler, NULL);
        status = sl_wifi_connect(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &provisioned_access_point, 25000);
        if (status != SL_STATUS_OK) {
          printf("Failed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
          // If scan failed retry again after 5 seconds
          if (status == SL_STATUS_SI91X_NO_AP_FOUND) {
            if (retry) {
              retry--;
              osDelay(5000);
              break;
            } else {
              retry = 5;
            }
          }
          status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
          if (status != SL_STATUS_OK) {
            printf("STA deinit : 0x%lx\r\n", status);
          }
          app_state = PROVISIONING_INIT_STATE;
          break;
        }

        // Configure IP address and start the HTTP server for the client mode
        status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_0, &profile);
        if (status != SL_STATUS_OK) {
          printf("Failed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
          return;
        }

        status = sl_si91x_configure_ip_address(&profile.ip, SL_SI91X_WIFI_CLIENT_VAP_ID);
        if (status != SL_STATUS_OK) {
          printf("IPv4 address configuration is failed : 0x%lx\r\n", status);
          return;
        }

        printf("IPv4 address configuration complete\r\n");
        memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
        printf("Client IPv4: ");
        print_sl_ipv4_address(&ip_address.ip.v4);
        printf("\r\n");

        app_state = DISCONNECTING_STATE;
        break;
      }
      case DISCONNECTING_STATE: {
        printf("Wi-Fi Provisioning demo is successfully completed\r\n");
        disconnect_complete = true;
        break;
      }
      case PROVISIONING_STATE:
      case CONNECTED_STATE: {
        // Delay for 1 second
        osDelay(1000);
        break;
      }
    }
    // Exit the loop if Demo is completed
    if (disconnect_complete) {
      break;
    }
  }
  while (1) {
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // Let the CPU go to sleep if the system allows it.
    sl_power_manager_sleep();
#else
    osDelay(osWaitForever);
#endif
  }
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
  printf("\r\n");
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
  printf("\r\n");

  return SL_STATUS_OK;
}

static sl_status_t index_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;

  printf("Got request %s with data length : %lu\r\n", req->uri.path, req->request_data_length);

  // Handle GET requests for the index page
  if (req->type == SL_HTTP_REQUEST_GET) {

    http_response.response_code        = SL_HTTP_RESPONSE_OK;
    http_response.content_type         = SL_HTTP_CONTENT_TYPE_TEXT_HTML;
    uint32_t data_length               = strlen((const char *)wifi_provisioning_content);
    uint32_t tx_length                 = (data_length > HTTP_CHUNK_SIZE ? HTTP_CHUNK_SIZE : data_length);
    http_response.data                 = (uint8_t *)wifi_provisioning_content;
    http_response.current_data_length  = tx_length;
    http_response.expected_data_length = data_length;

    // Send the first chunk
    sl_http_server_send_response(handle, &http_response);

    // Update the remaining data length
    data_length -= tx_length;

    while (data_length > 0) {
      // Calculate the next chunk size (handle case where the remaining data is less than 1024 bytes)
      tx_length = (data_length > HTTP_CHUNK_SIZE ? HTTP_CHUNK_SIZE : data_length);

      // Update the data pointer after sending the current chunk
      http_response.data += http_response.current_data_length;

      // Send the current chunk
      sl_http_server_write_data(handle, http_response.data, tx_length);

      // Update the remaining data length
      data_length -= tx_length;

      // Update the current chunk length for next iteration
      http_response.current_data_length = tx_length;
    }
    // Send the last chunk if there is any remaining data
    if (data_length > 0) {
      http_response.data += tx_length;
      http_response.current_data_length = data_length;
      sl_http_server_write_data(handle, http_response.data, data_length);
    }
  } else {
    // Send the HTTP response
    sl_http_server_send_response(handle, &http_response);
  }
  return SL_STATUS_OK;
}

static sl_status_t connect_page_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;

  printf("Got request %s with data length : %lu\r\n", req->uri.path, req->request_data_length);

  // Handle GET requests for the connect page
  if (req->type == SL_HTTP_REQUEST_GET) {
    http_response.response_code        = SL_HTTP_RESPONSE_OK;
    http_response.content_type         = SL_HTTP_CONTENT_TYPE_TEXT_HTML;
    http_response.data                 = (uint8_t *)login_content;
    http_response.current_data_length  = strlen((const char *)login_content);
    http_response.expected_data_length = http_response.current_data_length;
  }
  // Send the HTTP response
  sl_http_server_send_response(handle, &http_response);
  return SL_STATUS_OK;
}

static sl_status_t connect_data_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;

  printf("Got request %s with data length : %lu\r\n", req->uri.path, req->request_data_length);

  // Handle POST requests for connection data
  if (req->type == SL_HTTP_REQUEST_POST) {
    char received_data_buffer[200]        = { 0 }; // Buffer to hold received data
    sl_http_recv_req_data_t received_data = { 0 }; // Structure to hold received data
    char response_data[100]               = { 0 }; // Buffer for response data
    jsmn_parser parser;                            // JSON parser
    jsmntok_t tokens[10];                          // JSON tokens
    bool ssid_found          = false;
    bool security_type_found = false;
    bool passphrase_found    = false;

    received_data.request       = req;
    received_data.buffer        = (uint8_t *)received_data_buffer;
    received_data.buffer_length = sizeof(received_data_buffer) - 1;
    sl_http_server_read_request_data(handle, &received_data); // Read request data

    jsmn_init(&parser); // Initialize JSON parser

    memset(&tokens, 0, sizeof(tokens));
    jsmn_parse(&parser, received_data_buffer, req->request_data_length, tokens, sizeof(tokens) / sizeof(jsmntok_t));

    // Process JSON tokens to extract SSID, passphrase, and security type
    for (uint8_t a = 1; a < 7; a += 2) {
      if (tokens[a].type == JSMN_STRING) {
        if (memcmp(&received_data_buffer[tokens[a].start], SSID, tokens[a].end - tokens[a].start) == 0) {
          if (tokens[a + 1].type == JSMN_STRING) {
            snprintf(wifi_client_profile_ssid,
                     sizeof(wifi_client_profile_ssid),
                     "%.*s",
                     tokens[a + 1].end - tokens[a + 1].start,
                     received_data_buffer + tokens[a + 1].start);
            ssid_found = true;
          }
        } else if (memcmp(&received_data_buffer[tokens[a].start], PASSPHRASE, tokens[a].end - tokens[a].start) == 0) {
          if (tokens[a].type == JSMN_STRING) {
            snprintf(wifi_client_credential,
                     sizeof(wifi_client_credential),
                     "%.*s",
                     tokens[a + 1].end - tokens[a + 1].start,
                     received_data_buffer + tokens[a + 1].start);
            passphrase_found = true;
          }
        } else if (memcmp(&received_data_buffer[tokens[a].start], SECURITY_TYPE, tokens[a].end - tokens[a].start)
                   == 0) {
          snprintf(wifi_client_security_type,
                   sizeof(wifi_client_security_type),
                   "%.*s",
                   tokens[a + 1].end - tokens[a + 1].start,
                   received_data_buffer + tokens[a + 1].start);
          security_type_found = true;
        }
      }
    }

    // Check if all required fields were found
    if (ssid_found == false || passphrase_found == false || security_type_found == false) {
      // Respond with Bad Request if any field is missing
      http_response.response_code        = SL_HTTP_RESPONSE_BAD_REQUEST;
      http_response.data                 = (uint8_t *)METHOD_BAD_REQUEST;
      http_response.current_data_length  = sizeof(METHOD_BAD_REQUEST) - 1;
      http_response.expected_data_length = http_response.current_data_length;
      sl_http_server_send_response(handle, &http_response);
    } else {
      // Respond with success if all fields are correctly received
      snprintf(response_data, sizeof(response_data) - 1, "{\"status\": \"ok\"}");
      http_response.response_code        = SL_HTTP_RESPONSE_OK;
      http_response.content_type         = SL_HTTP_CONTENT_TYPE_APPLICATION_JSON;
      http_response.data                 = (uint8_t *)response_data;
      http_response.current_data_length  = strlen(response_data);
      http_response.expected_data_length = http_response.current_data_length;
      sl_http_server_send_response(handle, &http_response);
      app_state = CONNECTING_STATE; // Transition to the connecting state
    }
  } else {
    // Handle methods other than POST with Method Not Allowed response
    sl_http_server_send_response(handle, &http_response);
  }
  return SL_STATUS_OK;
}

static sl_status_t wlan_app_scan_callback_handler(sl_wifi_event_t event,
                                                  sl_status_t status_code,
                                                  sl_wifi_scan_result_t *scan_result,
                                                  uint32_t result_length,
                                                  void *arg)
{
  char *scan_result_buffer = (char *)arg;
  uint8_t *bssid           = NULL;
  UNUSED_PARAMETER(arg);
  UNUSED_PARAMETER(result_length);

  callback_status = status_code;
  // Check if the scan event indicates failure
  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }

  if (scan_result->scan_count) {
    uint32_t buffer_length = SCAN_RESULT_BUFFER_SIZE - 1;
    int32_t index =
      snprintf(scan_result_buffer, buffer_length, "{\"count\": \"%lu\", \"scan_results\": [", scan_result->scan_count);
    scan_result_buffer += index;
    buffer_length -= index;

    // Iterate through scan results and format them as JSON
    for (uint32_t a = 0; a < scan_result->scan_count; a++) {
      bssid = (uint8_t *)&scan_result->scan_info[a].bssid;
      index = snprintf(scan_result_buffer,
                       buffer_length,
                       "{\"ssid\": \"%s\", \"security_type\": \"%s\", \"network_type\": \"%u\",",
                       scan_result->scan_info[a].ssid,
                       security_type_to_string(scan_result->scan_info[a].security_mode),
                       scan_result->scan_info[a].network_type);
      scan_result_buffer += index;
      buffer_length -= index;
      index = snprintf(scan_result_buffer,
                       buffer_length,
                       " \"bssid\": \"%02x:%02x:%02x:%02x:%02x:%02x\", \"channel\": \"%u\", \"rssi\": \"-%u\"}",
                       bssid[0],
                       bssid[1],
                       bssid[2],
                       bssid[3],
                       bssid[4],
                       bssid[5],
                       scan_result->scan_info[a].rf_channel,
                       scan_result->scan_info[a].rssi_val);
      scan_result_buffer += index;
      buffer_length -= index;
      if (a < scan_result->scan_count - 1) {
        index = snprintf(scan_result_buffer, buffer_length, ",");
        scan_result_buffer += index;
        buffer_length -= index;
      }
    }
    index = snprintf(scan_result_buffer, buffer_length, "]}");
    scan_result_buffer += index;
    buffer_length -= index;
  }

  scan_complete = true; // Indicate that the scan is complete
  return SL_STATUS_OK;
}

static sl_status_t wifi_scan_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;

  // Handle GET requests for Wi-Fi scan results
  if (req->type == SL_HTTP_REQUEST_GET) {
    sl_status_t status;
    char *scan_result_buffer                             = (char *)malloc(SCAN_RESULT_BUFFER_SIZE);
    sl_wifi_scan_configuration_t wifi_scan_configuration = default_wifi_scan_configuration;

    printf("Got request %s with data length : %lu\r\n", req->uri.path, req->request_data_length);
    memset(scan_result_buffer, 0, SCAN_RESULT_BUFFER_SIZE);

    printf("WLAN scan started \r\n");
    scan_complete   = false;
    callback_status = SL_STATUS_FAIL;

    sl_wifi_set_scan_callback_v2(wlan_app_scan_callback_handler, (void *)scan_result_buffer);
    status = sl_wifi_start_scan(SL_WIFI_AP_INTERFACE, NULL, &wifi_scan_configuration);
    if (SL_STATUS_IN_PROGRESS == status) {
      printf("Scanning...\r\n");
      const uint32_t start = osKernelGetTickCount();

      // Wait for scan completion or timeout
      while (!scan_complete && (osKernelGetTickCount() - start) <= 10000) {
        osThreadYield();
      }
      status = scan_complete ? callback_status : SL_STATUS_TIMEOUT;
    }
    if (status != RSI_SUCCESS) {
      printf("WLAN Scan failed %lx. Please make sure the latest connectivity firmware is used.\r\n", status);
      http_response.response_code        = SL_HTTP_RESPONSE_INTERNAL_SERVER_ERROR;
      http_response.data                 = (uint8_t *)METHOD_INTERNAL_SERVER_ERROR;
      http_response.current_data_length  = sizeof(METHOD_INTERNAL_SERVER_ERROR) - 1;
      http_response.expected_data_length = http_response.current_data_length;

      sl_http_server_send_response(handle, &http_response);
    } else {
      printf("Scan done state \r\n");
      uint32_t data_length               = strlen((const char *)scan_result_buffer);
      uint32_t tx_length                 = (data_length > HTTP_CHUNK_SIZE ? HTTP_CHUNK_SIZE : data_length);
      http_response.response_code        = SL_HTTP_RESPONSE_OK;
      http_response.content_type         = SL_HTTP_CONTENT_TYPE_APPLICATION_JSON;
      http_response.data                 = (uint8_t *)scan_result_buffer;
      http_response.current_data_length  = tx_length;
      http_response.expected_data_length = data_length;

      sl_http_server_send_response(handle, &http_response); // Send response
      data_length -= tx_length;
      while (data_length > 0) {
        tx_length = (data_length > HTTP_CHUNK_SIZE ? HTTP_CHUNK_SIZE : data_length);

        // Update the data pointer after sending the current chunk
        http_response.data += http_response.current_data_length;

        // Send the current chunk
        sl_http_server_write_data(handle, http_response.data, tx_length);

        // Update the remaining data length
        data_length -= tx_length;

        // Update the current chunk length for next iteration
        http_response.current_data_length = tx_length;
      }
      // Send the last chunk if there is any remaining data
      if (data_length > 0) {
        http_response.data += tx_length;
        http_response.current_data_length = data_length;
        sl_http_server_write_data(handle, http_response.data, data_length);
      }
    }

    free(scan_result_buffer); // Free allocated buffer
  } else {
    sl_http_server_send_response(handle, &http_response);
  }
  return SL_STATUS_OK;
}

static sl_status_t default_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;
  sl_http_header_t header                 = { .key = "Server", .value = "SI917-HTTPServer" };

  UNUSED_PARAMETER(req);

  // Handle requests for unknown or unsupported URIs
  http_response.response_code = SL_HTTP_RESPONSE_NOT_FOUND;
  http_response.content_type  = SL_HTTP_CONTENT_TYPE_TEXT_PLAIN;
  http_response.headers       = &header;
  http_response.header_count  = 1;

  char *response_data                = "404 Not Found";
  http_response.data                 = (uint8_t *)response_data;
  http_response.current_data_length  = strlen(response_data);
  http_response.expected_data_length = http_response.current_data_length;
  sl_http_server_send_response(handle, &http_response);

  return SL_STATUS_OK;
}

static sl_wifi_security_t string_to_security_type(const char *security_type)
{
  // Compare the input string to known security type strings
  if (strcmp(security_type, OPEN) == 0) {
    return SL_WIFI_OPEN;
  } else if (strcmp(security_type, WPA) == 0) {
    return SL_WIFI_WPA;
  } else if (strcmp(security_type, WPA2) == 0) {
    return SL_WIFI_WPA2;
  } else if (strcmp(security_type, WPA3) == 0) {
    return SL_WIFI_WPA3;
  } else if (strcmp(security_type, WPA3_TRANSITION) == 0) {
    return SL_WIFI_WPA3_TRANSITION;
  } else if (strcmp(security_type, MIXED_MODE) == 0) {
    return SL_WIFI_WPA_WPA2_MIXED;
  } else {
    return SL_WIFI_SECURITY_UNKNOWN;
  }
}

static char *security_type_to_string(sl_wifi_security_t security_type)
{
  // Return the string representation for the given enum value
  switch (security_type) {
    case SL_WIFI_OPEN:
      return OPEN;
    case SL_WIFI_WPA:
      return WPA;
    case SL_WIFI_WPA2:
      return WPA2;
    case SL_WIFI_WPA3:
      return WPA3;
    case SL_WIFI_WPA3_TRANSITION:
      return WPA3_TRANSITION;
    case SL_WIFI_WPA_WPA2_MIXED:
      return MIXED_MODE;
    default:
      return UNKNOWN;
  }
}
