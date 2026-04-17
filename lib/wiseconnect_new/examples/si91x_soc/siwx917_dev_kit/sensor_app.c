/***************************************************************************/ /**
 * @file
 * @brief Embedded Sensor Demo
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
#include "light_sensor.h"
#include "motion_sensor.h"
#include "humidity_sensor.h"
#include "rgb_led.h"
#include "stdbool.h"
#include "sl_sleeptimer.h"
/******************************************************
 *                      Macros
 ******************************************************/

#define METHOD_BAD_REQUEST           "400 Bad Request"
#define METHOD_NOT_ALLOWED           "405 Not Allowed"
#define METHOD_INTERNAL_SERVER_ERROR "500 Internal Server Error"

#define DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED                                                                  \
  {                                                                                                               \
    .response_code = SL_HTTP_RESPONSE_METHOD_NOT_ALLOWED, .content_type = SL_HTTP_CONTENT_TYPE_TEXT_HTML,         \
    .headers = NULL, .header_count = 0, .data = (uint8_t *)METHOD_NOT_ALLOWED,                                    \
    .current_data_length = sizeof(METHOD_NOT_ALLOWED) - 1, .expected_data_length = sizeof(METHOD_NOT_ALLOWED) - 1 \
  }

#define HTTP_SERVER_PORT 80

// Server port number
#define SERVER_PORT 5000

#define SCAN_RESULT_BUFFER_SIZE (2000)

#define ON            "on"
#define OFF           "off"
#define OPEN          "Open"
#define WPA           "WPA"
#define WPA2          "WPA2"
#define WPA3          "WPA3"
#define MIXED_MODE    "Mixed Mode"
#define UNKNOWN       "Unknown"
#define SSID          "ssid"
#define SECURITY_TYPE "security_type"
#define PASSPHRASE    "passphrase"

#define LED_JSON_RESPONSE           "{\"red\": \"%s\", \"green\": \"%s\", \"blue\": \"%s\"}"
#define TEMPERATURE_JSON_RESPONSE   "{\"temperature_celcius\": \"%0.2f\"}"
#define LIGHT_JSON_RESPONSE         "{\"ambient_light_lux\": \"%0.2f\", \"white_light_lux\": \"%0.2f\"}"
#define MOTION_SENSOR_JSON_RESPONSE "{\"x\": \"%0.2f\", \"y\": \"%0.2f\", \"z\": \"%0.2f\"}"
#define HUMIDITY_JSON_RESPONSE      "{\"humidity_percentage\": \"%lu\"}"
#define MICROPHONE_JSON_RESPONSE    "{\"microphone_decibel\": \"%lu\"}"
#define STATUS_LED_JSON_RESPONSE    "{\"status_led\": \"%s\"}"

#define PROVISIONING_LED_INTERVAL_MS (300)
#define CONNECTING_LED_INTERVAL_MS   (2000)
#define CONNECTED_LED_INTERVAL_MS    (2000)
#define ERROR_LED_INTERVAL_MS        (150)
#define LED_ERROR_BLINK_COUNT        (3)

//! Enumeration for states in application
typedef enum {
  PROVISIONING_INIT_STATE,
  PROVISIONING_STATE,
  CONNECTING_STATE,
  CONNECTED_STATE,
  DISCONNECTING_STATE,
} app_state_t;

typedef enum {
  STATUS_LED_OFF,
  STATUS_LED_PROVISIONING,
  STATUS_LED_CONNECTING,
  STATUS_LED_CONNECTED,
} status_led_state_t;

/******************************************************
 *               Function Declarations
 ******************************************************/

static sl_status_t default_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
static sl_status_t provisioning_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
static sl_status_t light_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
static sl_status_t led_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
static sl_status_t temperature_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
static sl_status_t accelerometer_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
static sl_status_t gyroscope_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
static sl_status_t humidity_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
static sl_status_t microphone_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
static sl_status_t all_sensors_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
static sl_status_t status_led_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);

/******************************************************
 *               Variable Definitions
 ******************************************************/

static const char *rgb_colours[] = { [0] = "red", [1] = "green", [2] = "blue" };

sl_http_server_t server_handle = { 0 };
static app_state_t app_state   = PROVISIONING_INIT_STATE;

const sl_http_server_handler_t sensor_server_request_handlers[] = {
  { .uri = "/provisioning", .handler = provisioning_request_handler },
  { .uri = "/light", .handler = light_request_handler },
  { .uri = "/led", .handler = led_request_handler },
  { .uri = "/temperature", .handler = temperature_request_handler },
  { .uri = "/accelerometer", .handler = accelerometer_request_handler },
  { .uri = "/gyroscope", .handler = gyroscope_request_handler },
  { .uri = "/humidity", .handler = humidity_request_handler },
  { .uri = "/microphone", .handler = microphone_request_handler },
  { .uri = "/all_sensors", .handler = all_sensors_request_handler },
  { .uri = "/status_led", .handler = status_led_request_handler }
};

static bool status_led_on = false;

/******************************************************
 *               Function Definitions
 ******************************************************/

void sensors_init(void)
{
  // Init sensor when application starts
  light_sensor_init();
  humidity_sensor_init();
  motion_sensor_init();
}

void sensor_app_init(void)
{
  sl_http_server_config_t server_config = { 0 };
  sl_status_t status;

  server_config.port            = HTTP_SERVER_PORT;
  server_config.default_handler = default_handler;
  server_config.handlers_list   = (sl_http_server_handler_t *)sensor_server_request_handlers;
  server_config.handlers_count  = sizeof(sensor_server_request_handlers) / sizeof(sl_http_server_handler_t);

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
  printf("Sensor HTTP server started\r\n");

  printf("\r\nThe sensor demo is now connected to your Wi-Fi network\r\n");
}

static sl_status_t light_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;

  printf("Got request %s with data length : %lu\r\n", req->uri.path, req->request_data_length);

  if (req->type == SL_HTTP_REQUEST_GET) {
    char response_data[100] = { 0 };
    float ambient_light_lux = 0.0f;
    float white_light_lux   = 0.0f;
    ambient_light_read(&ambient_light_lux);
    white_light_read(&white_light_lux);

    snprintf(response_data, sizeof(response_data) - 1, LIGHT_JSON_RESPONSE, ambient_light_lux, white_light_lux);
    http_response.response_code        = SL_HTTP_RESPONSE_OK;
    http_response.content_type         = SL_HTTP_CONTENT_TYPE_APPLICATION_JSON;
    http_response.data                 = (uint8_t *)response_data;
    http_response.current_data_length  = strlen(response_data);
    http_response.expected_data_length = http_response.current_data_length;
  }
  sl_http_server_send_response(handle, &http_response);
  return SL_STATUS_OK;
}

static sl_status_t led_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;
  char response_data[100]                 = { 0 };
  bool rgb_states[RGB_LED_MAX]            = { false };

  printf("Got request %s with data length : %lu\r\n", req->uri.path, req->request_data_length);

  switch (req->type) {
    case SL_HTTP_REQUEST_POST: {
      char received_data_buffer[200]        = { 0 };
      sl_http_recv_req_data_t received_data = { 0 };
      jsmn_parser parser;
      jsmntok_t tokens[10];
      bool rgb_found[RGB_LED_MAX] = { false };

      if (status_led_on == true) {
        sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;
        sl_http_server_send_response(handle, &http_response);
        return SL_STATUS_OK;
      }

      received_data.request       = req;
      received_data.buffer        = (uint8_t *)received_data_buffer;
      received_data.buffer_length = sizeof(received_data_buffer) - 1;
      sl_http_server_read_request_data(handle, &received_data);

      jsmn_init(&parser);

      memset(&tokens, 0, sizeof(tokens));
      jsmn_parse(&parser, received_data_buffer, req->request_data_length, tokens, sizeof(tokens) / sizeof(jsmntok_t));
      for (uint8_t a = 1; a < RGB_LED_MAX * 2 + 1; a += 2) {
        if (tokens[a].type == JSMN_STRING) {
          for (uint8_t b = 0; b < 3; b++) {
            if (memcmp(&received_data_buffer[tokens[a].start], rgb_colours[b], tokens[a].end - tokens[a].start) == 0) {
              if (tokens[a + 1].type == JSMN_STRING) {
                if (memcmp(&received_data_buffer[tokens[a + 1].start], ON, tokens[a + 1].end - tokens[a + 1].start)
                    == 0) {
                  rgb_found[b]  = true;
                  rgb_states[b] = true;
                } else if (memcmp(&received_data_buffer[tokens[a + 1].start],
                                  OFF,
                                  tokens[a + 1].end - tokens[a + 1].start)
                           == 0) {
                  rgb_found[b]  = true;
                  rgb_states[b] = false;
                }
              }
            }
          }
        }
      }
      if (rgb_found[RGB_LED_RED] == false || rgb_found[RGB_LED_GREEN] == false || rgb_found[RGB_LED_BLUE] == false) {
        http_response.response_code        = SL_HTTP_RESPONSE_BAD_REQUEST;
        http_response.data                 = (uint8_t *)METHOD_BAD_REQUEST;
        http_response.current_data_length  = sizeof(METHOD_BAD_REQUEST) - 1;
        http_response.expected_data_length = http_response.current_data_length;
        break;
      } else {
        rgb_led_set_state(rgb_states[RGB_LED_RED], rgb_states[RGB_LED_GREEN], rgb_states[RGB_LED_BLUE]);
      }
    } // fall-through
    // @suppress("No break at end of case")
    case SL_HTTP_REQUEST_GET: {
      rgb_led_get_state(&rgb_states[RGB_LED_RED], &rgb_states[RGB_LED_GREEN], &rgb_states[RGB_LED_BLUE]);
      snprintf(response_data,
               sizeof(response_data) - 1,
               LED_JSON_RESPONSE,
               (rgb_states[RGB_LED_RED] == true) ? ON : OFF,
               (rgb_states[RGB_LED_GREEN] == true) ? ON : OFF,
               (rgb_states[RGB_LED_BLUE] == true) ? ON : OFF);
      http_response.response_code        = SL_HTTP_RESPONSE_OK;
      http_response.content_type         = SL_HTTP_CONTENT_TYPE_APPLICATION_JSON;
      http_response.data                 = (uint8_t *)response_data;
      http_response.current_data_length  = strlen(response_data);
      http_response.expected_data_length = http_response.current_data_length;
      break;
    }
    default:
      break;
  }
  sl_http_server_send_response(handle, &http_response);
  return SL_STATUS_OK;
}

static sl_status_t temperature_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;

  printf("Got request %s with data length : %lu\r\n", req->uri.path, req->request_data_length);

  if (req->type == SL_HTTP_REQUEST_GET) {
    char response_data[100] = { 0 };
    float temperature       = 0.0f;

    temperature_read(&temperature);

    snprintf(response_data, sizeof(response_data) - 1, TEMPERATURE_JSON_RESPONSE, temperature);
    http_response.response_code        = SL_HTTP_RESPONSE_OK;
    http_response.content_type         = SL_HTTP_CONTENT_TYPE_APPLICATION_JSON;
    http_response.data                 = (uint8_t *)response_data;
    http_response.current_data_length  = strlen(response_data);
    http_response.expected_data_length = http_response.current_data_length;
  }
  sl_http_server_send_response(handle, &http_response);
  return SL_STATUS_OK;
}

static sl_status_t accelerometer_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;

  if (req->type == SL_HTTP_REQUEST_GET) {
    char response_data[100] = { 0 };
    float x                 = 0.0f;
    float y                 = 0.0f;
    float z                 = 0.0f;

    printf("Got request %s with data length : %lu\r\n", req->uri.path, req->request_data_length);

    accelerometer_read(&x, &y, &z);

    snprintf(response_data, sizeof(response_data) - 1, MOTION_SENSOR_JSON_RESPONSE, x, y, z);
    http_response.response_code        = SL_HTTP_RESPONSE_OK;
    http_response.content_type         = SL_HTTP_CONTENT_TYPE_APPLICATION_JSON;
    http_response.data                 = (uint8_t *)response_data;
    http_response.current_data_length  = strlen(response_data);
    http_response.expected_data_length = http_response.current_data_length;
  }
  sl_http_server_send_response(handle, &http_response);
  return SL_STATUS_OK;
}

static sl_status_t gyroscope_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;

  printf("Got request %s with data length : %lu\r\n", req->uri.path, req->request_data_length);

  if (req->type == SL_HTTP_REQUEST_GET) {
    char response_data[100] = { 0 };
    float x                 = 0.0f;
    float y                 = 0.0f;
    float z                 = 0.0f;

    gyro_read(&x, &y, &z);

    snprintf(response_data, sizeof(response_data) - 1, MOTION_SENSOR_JSON_RESPONSE, x, y, z);
    http_response.response_code        = SL_HTTP_RESPONSE_OK;
    http_response.content_type         = SL_HTTP_CONTENT_TYPE_APPLICATION_JSON;
    http_response.data                 = (uint8_t *)response_data;
    http_response.current_data_length  = strlen(response_data);
    http_response.expected_data_length = http_response.current_data_length;
  }
  sl_http_server_send_response(handle, &http_response);
  return SL_STATUS_OK;
}

static sl_status_t humidity_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;

  printf("Got request %s with data length : %lu\r\n", req->uri.path, req->request_data_length);

  if (req->type == SL_HTTP_REQUEST_GET) {
    char response_data[100] = { 0 };
    uint32_t humidity       = 0;

    humidity_read(&humidity);

    snprintf(response_data, sizeof(response_data) - 1, HUMIDITY_JSON_RESPONSE, humidity);
    http_response.response_code        = SL_HTTP_RESPONSE_OK;
    http_response.content_type         = SL_HTTP_CONTENT_TYPE_APPLICATION_JSON;
    http_response.data                 = (uint8_t *)response_data;
    http_response.current_data_length  = strlen(response_data);
    http_response.expected_data_length = http_response.current_data_length;
  }
  sl_http_server_send_response(handle, &http_response);
  return SL_STATUS_OK;
}

static sl_status_t microphone_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;

  printf("Got request %s with data length : %lu\r\n", req->uri.path, req->request_data_length);

  if (req->type == SL_HTTP_REQUEST_GET) {
    char response_data[100]     = { 0 };
    uint32_t microphone_decibel = 0;

    // TODO: add call to microphone read

    snprintf(response_data, sizeof(response_data) - 1, MICROPHONE_JSON_RESPONSE, microphone_decibel);
    http_response.response_code        = SL_HTTP_RESPONSE_OK;
    http_response.content_type         = SL_HTTP_CONTENT_TYPE_APPLICATION_JSON;
    http_response.data                 = (uint8_t *)response_data;
    http_response.current_data_length  = strlen(response_data);
    http_response.expected_data_length = http_response.current_data_length;
  }
  sl_http_server_send_response(handle, &http_response);
  return SL_STATUS_OK;
}

static sl_status_t provisioning_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;

  printf("Got request %s with data length : %lu\r\n", req->uri.path, req->request_data_length);

  if (req->type == SL_HTTP_REQUEST_POST) {
    char response_data[100] = { 0 };

    snprintf(response_data, sizeof(response_data) - 1, "{\"status\": \"ok\"}");
    http_response.response_code        = SL_HTTP_RESPONSE_OK;
    http_response.content_type         = SL_HTTP_CONTENT_TYPE_APPLICATION_JSON;
    http_response.data                 = (uint8_t *)response_data;
    http_response.current_data_length  = strlen(response_data);
    http_response.expected_data_length = http_response.current_data_length;
    sl_http_server_send_response(handle, &http_response);
    if (app_state == CONNECTED_STATE) {
      app_state = DISCONNECTING_STATE;
    }
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

static sl_status_t all_sensors_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;

  if (req->type == SL_HTTP_REQUEST_GET) {
    char response_data[1000]         = { 0 };
    uint32_t remaining_buffer_length = sizeof(response_data) - 1;
    char *current_response_data      = response_data;
    uint32_t index                   = 0;
    bool red                         = false;
    bool green                       = false;
    bool blue                        = false;
    float temperature                = 0.0f;
    float ambient_light_lux          = 0.0f;
    float white_light_lux            = 0.0f;
    float x                          = 0.0f;
    float y                          = 0.0f;
    float z                          = 0.0f;
    uint32_t humidity                = 0;
    uint32_t microphone_decibel      = 0;

    printf("Got request %s with data length : %lu\r\n", req->uri.path, req->request_data_length);

    rgb_led_get_state(&red, &green, &blue);
    index = snprintf(current_response_data,
                     remaining_buffer_length,
                     "{\"led\": " LED_JSON_RESPONSE ",",
                     (red == true) ? ON : OFF,
                     (green == true) ? ON : OFF,
                     (blue == true) ? ON : OFF);
    current_response_data += index;
    remaining_buffer_length -= index;

    ambient_light_read(&ambient_light_lux);
    white_light_read(&white_light_lux);
    index = snprintf(current_response_data,
                     remaining_buffer_length,
                     "\"light\": " LIGHT_JSON_RESPONSE ",",
                     ambient_light_lux,
                     white_light_lux);
    current_response_data += index;
    remaining_buffer_length -= index;

    temperature_read(&temperature);
    index = snprintf(current_response_data,
                     remaining_buffer_length,
                     "\"temperature\": " TEMPERATURE_JSON_RESPONSE ",",
                     temperature);
    current_response_data += index;
    remaining_buffer_length -= index;

    accelerometer_read(&x, &y, &z);
    index = snprintf(current_response_data,
                     remaining_buffer_length,
                     "\"accelerometer\": " MOTION_SENSOR_JSON_RESPONSE ",",
                     x,
                     y,
                     z);
    current_response_data += index;
    remaining_buffer_length -= index;

    gyro_read(&x, &y, &z);
    index = snprintf(current_response_data,
                     remaining_buffer_length,
                     "\"gyroscope\": " MOTION_SENSOR_JSON_RESPONSE ",",
                     x,
                     y,
                     z);
    current_response_data += index;
    remaining_buffer_length -= index;

    humidity_read(&humidity);
    index =
      snprintf(current_response_data, remaining_buffer_length, "\"humidity\": " HUMIDITY_JSON_RESPONSE ",", humidity);
    current_response_data += index;
    remaining_buffer_length -= index;

    // TODO: add call to microphone read
    index = snprintf(current_response_data,
                     remaining_buffer_length,
                     "\"microphone\": " MICROPHONE_JSON_RESPONSE "}",
                     microphone_decibel);
    current_response_data += index;
    remaining_buffer_length -= index;

    http_response.response_code        = SL_HTTP_RESPONSE_OK;
    http_response.content_type         = SL_HTTP_CONTENT_TYPE_APPLICATION_JSON;
    http_response.data                 = (uint8_t *)response_data;
    http_response.current_data_length  = strlen(response_data);
    http_response.expected_data_length = http_response.current_data_length;
  }
  sl_http_server_send_response(handle, &http_response);
  return SL_STATUS_OK;
}

static sl_status_t status_led_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = DEFAULT_HTTP_RESPONSE_METHOD_NOT_ALLOWED;
  char response_data[100]                 = { 0 };

  printf("Got request %s with data length : %lu\r\n", req->uri.path, req->request_data_length);

  switch (req->type) {
    case SL_HTTP_REQUEST_POST: {
      char received_data_buffer[200]        = { 0 };
      sl_http_recv_req_data_t received_data = { 0 };
      jsmn_parser parser;
      jsmntok_t tokens[10];
      bool state_found = false;
      //      bool state       = false;

      received_data.request       = req;
      received_data.buffer        = (uint8_t *)received_data_buffer;
      received_data.buffer_length = sizeof(received_data_buffer) - 1;
      sl_http_server_read_request_data(handle, &received_data);

      jsmn_init(&parser);

      memset(&tokens, 0, sizeof(tokens));
      jsmn_parse(&parser, received_data_buffer, req->request_data_length, tokens, sizeof(tokens) / sizeof(jsmntok_t));
      if (tokens[1].type == JSMN_STRING) {
        if (memcmp(&received_data_buffer[tokens[1].start], "status_led", tokens[1].end - tokens[1].start) == 0) {
          if (tokens[2].type == JSMN_STRING) {
            if (memcmp(&received_data_buffer[tokens[2].start], ON, tokens[2].end - tokens[2].start) == 0) {
              state_found = true;
              //              state       = true;
            } else if (memcmp(&received_data_buffer[tokens[2].start], OFF, tokens[2].end - tokens[2].start) == 0) {
              state_found = true;
              //              state       = false;
            }
          }
        }
      }
      if (state_found == false) {
        http_response.response_code        = SL_HTTP_RESPONSE_BAD_REQUEST;
        http_response.data                 = (uint8_t *)METHOD_BAD_REQUEST;
        http_response.current_data_length  = sizeof(METHOD_BAD_REQUEST) - 1;
        http_response.expected_data_length = http_response.current_data_length;
        break;
      } else {
        //        status_led_set_state(state);
        //        status_led_update_state();
      }
    } // fall-through
    // @suppress("No break at end of case")
    case SL_HTTP_REQUEST_GET: {
      snprintf(response_data, sizeof(response_data) - 1, STATUS_LED_JSON_RESPONSE, (status_led_on == true) ? ON : OFF);
      http_response.response_code        = SL_HTTP_RESPONSE_OK;
      http_response.content_type         = SL_HTTP_CONTENT_TYPE_APPLICATION_JSON;
      http_response.data                 = (uint8_t *)response_data;
      http_response.current_data_length  = strlen(response_data);
      http_response.expected_data_length = http_response.current_data_length;
      break;
    }
    default:
      break;
  }
  sl_http_server_send_response(handle, &http_response);
  return SL_STATUS_OK;
}
