/*******************************************************************************
 * @file  wifi_app.c
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
/*************************************************************************
 *
 */

/*================================================================================
 * @brief : This file contains example application for Wlan Station BLE
 * Provisioning
 * @section Description :
 * This application explains how to get the WLAN connection functionality using
 * BLE provisioning.
 * Silicon Labs Module starts advertising and with BLE Provisioning the Access Point
 * details are fetched.
 * Silicon Labs device is configured as a WiFi station and connects to an Access Point.
 =================================================================================*/

/**
 * Include files
 * */

//! SL Wi-Fi SDK includes
#include "sl_wifi_callback_framework.h"
#include "sl_net_wifi_types.h"
#include "sl_si91x_driver.h"
#include <string.h>
#include <stdio.h>

#include "rsi_common_apis.h"
#include "rsi_bt_common_apis.h"

#include "ble_config.h"
#include "wifi_config.h"

//! LCD related include files
#include "sl_sleeptimer.h"
#include "sl_sleeptimer_config.h"
#include "app.h"
#include "glib.h"
#include "dmd.h"
#include "RTE_Device_917.h"
#include "rsi_retention.h"
#include "sl_status.h"
#include "rsi_ccp_user_config.h"

#include "em_assert.h"

#include <rsi_ble_apis.h>

#include "sl_memlcd.h"

//! MQTT related include files
#include "sl_mqtt_client.h"
#include "sl_wifi.h"
#include "sl_net_dns.h"
#include "sl_net_ping.h"

#include "sl_si91x_button.h"
#include "sl_si91x_button_pin_config.h"
#include "sl_si91x_button_instances.h"

#include "logo_bitmaps.h"
#include "cmsis_os2.h"
/******************************************************
 *                    Constants
 ******************************************************/

typedef enum {
  SENSOR_EVENT_BUTTON_PRESSED = (1 << 0),
} sensor_event_t;

#define WIFI_CLIENT_PROFILE_SSID "YOUR_INITIAL_AP_SSID"

// LCD related defines
#define SL_BOARD_ENABLE_DISPLAY_PIN  0
#define SL_BOARD_ENABLE_DISPLAY_PORT 0

#define TIMER_0  0  // Timer 0
#define ONE_MSEC 32 // Ticks required for every one millisecond

//MQTT related defines
#define WIFI_SCAN_TIMEOUT 10000
#define PING_PACKET_SIZE  64
#define MQTT_BROKER_PORT  1883
#define CLIENT_PORT       1
#define CLIENT_ID         "WIFI-SDK-MQTT-CLIENT"
#define TOPIC_TO_BE_SUBSCRIBED \
  "Si917_MQTT_RECEIVE" // 917 subscribes to this topic, any messages published on this will be received
#define QOS_OF_SUBSCRIPTION    SL_MQTT_QOS_LEVEL_0
#define PUBLISH_TOPIC          "Si917_APP_STATUS" // Publishing from the module to MQTT broker/server
#define PUBLISH_MESSAGE        "Button is pressed"
#define QOS_OF_PUBLISH_MESSAGE 0
#define IS_DUPLICATE_MESSAGE   0
#define IS_MESSAGE_RETAINED    0
#define IS_CLEAN_SESSION       1
#define LAST_WILL_TOPIC        "MQTT-CLIENT-LAST-WILL"
#define LAST_WILL_MESSAGE      "MQTT-CLIENT has been disconnect from network"
#define QOS_OF_LAST_WILL       1
#define IS_LAST_WILL_RETAINED  1
#define ENCRYPT_CONNECTION     0
#define KEEP_ALIVE_INTERVAL    2000
#define MQTT_CONNECT_TIMEOUT   5000
#define DNS_TIMEOUT            20000
#define MAX_DNS_RETRY_COUNT    5

#define DHCP_HOST_NAME NULL
#define TIMEOUT_MS     5000
#define NO_OF_PINGS    5

#ifndef BUTTON_INSTANCE_0
#define BUTTON_INSTANCE_0 button_btn0
#endif

static osEventFlagsId_t sensor_event_flags = 0;

char *mqtt_hostname = "test.mosquitto.org";

sl_mqtt_client_t client                                  = { 0 };
uint8_t is_execution_completed                           = 0;
sl_mqtt_client_credentials_t *client_credentails         = NULL;
sl_mqtt_client_configuration_t mqtt_client_configuration = { .is_clean_session = IS_CLEAN_SESSION,
                                                             .client_port      = CLIENT_PORT };

sl_mqtt_broker_t mqtt_broker_configuration = {
  .port                    = MQTT_BROKER_PORT,
  .is_connection_encrypted = ENCRYPT_CONNECTION,
  .connect_timeout         = MQTT_CONNECT_TIMEOUT,
  .keep_alive_interval     = KEEP_ALIVE_INTERVAL,
};

sl_mqtt_client_message_t message_to_be_published = {
  .qos_level            = QOS_OF_PUBLISH_MESSAGE,
  .is_retained          = IS_MESSAGE_RETAINED,
  .is_duplicate_message = IS_DUPLICATE_MESSAGE,
  .topic                = (uint8_t *)PUBLISH_TOPIC,
  .topic_length         = strlen(PUBLISH_TOPIC),
  .content              = (uint8_t *)PUBLISH_MESSAGE,
  .content_length       = strlen(PUBLISH_MESSAGE),
};

sl_mqtt_client_last_will_message_t last_will_message = {
  .is_retained         = IS_LAST_WILL_RETAINED,
  .will_qos_level      = QOS_OF_LAST_WILL,
  .will_topic          = (uint8_t *)LAST_WILL_TOPIC,
  .will_topic_length   = strlen(LAST_WILL_TOPIC),
  .will_message        = (uint8_t *)LAST_WILL_MESSAGE,
  .will_message_length = strlen(LAST_WILL_MESSAGE),
};

/******************************************************
 *               Function Declarations
 ******************************************************/

void mqtt_client_message_handler(void *client, sl_mqtt_client_message_t *message, void *context);
void mqtt_client_event_handler(void *client, sl_mqtt_client_event_t event, void *event_data, void *context);
void mqtt_client_error_event_handler(void *client, sl_mqtt_client_error_status_t *error);
void mqtt_client_cleanup();
void print_char_buffer(char *buffer, uint32_t buffer_length);
sl_status_t mqtt_example();
void memlcd_app_init(void);
void wifi_app_set_event(uint32_t event_num);
void wifi_app_clear_event(uint32_t event_num);
int32_t wifi_app_get_event(void);
sl_status_t join_callback_handler(sl_wifi_event_t event,
                                  sl_status_t status_code,
                                  char *result,
                                  uint32_t result_length,
                                  void *arg);
void rsi_wlan_app_callbacks_init(void);
sl_status_t wlan_app_scan_callback_handler(sl_wifi_event_t event,
                                           sl_status_t status_code,
                                           sl_wifi_scan_result_t *result,
                                           uint32_t result_length,
                                           void *arg);
void ping_silabs();
void test_mosquitto_org_pub();
/*
 *********************************************************************************************************
 *                                         LOCAL GLOBAL VARIABLES
 *********************************************************************************************************
 */

sl_wifi_scan_result_t *scan_result          = NULL;
static volatile bool scan_complete          = false;
static volatile sl_status_t callback_status = SL_STATUS_OK;
uint16_t scanbuf_size = (sizeof(sl_wifi_scan_result_t) + (SL_WIFI_MAX_SCANNED_AP * sizeof(scan_result->scan_info[0])));

sl_wifi_performance_profile_v2_t wifi_profile = { .profile = ASSOCIATED_POWER_SAVE_LOW_LATENCY };

//MQTT related variables
uint8_t connected = 0, timeout = 0;
uint8_t disconnected = 0, disassociated = 0;
uint8_t a = 0;

//MQTT disconnect flag
uint8_t mqtt_disconnect_flag = 0;

sl_net_wifi_client_profile_t client_profile = { 0 };

static uint32_t wlan_app_event_map;

uint8_t retry = 1;

uint8_t yield;
uint8_t disconnect_flag = 0;
char *hostname          = "www.silabs.com";

volatile uint8_t mqtt_connected = 0;

//LCD related variables
GLIB_Context_t glibContext;
volatile int currentLine = 0;

sl_mac_address_t mac_addr = { 0 };
sl_ipv4_address_t *fetch_ip;
char msg[100];

char mac_id[18];
char fw[32];
char ip_add[32];
sl_ip_address_t ip = { 0 };

// Application level WLAN reconnection attempt counter and retry limit
#define APP_RECONN_LOOP_CTR_LIM 3
uint8_t app_reconn_loop_ctr = 0;

typedef struct wlan_app_cb_s {
  //! WLAN application state
  volatile wifi_app_state_t state;

  //! length of buffer to copy
  uint32_t length;

  //! application buffer
  uint8_t buffer[RSI_APP_BUF_SIZE];

  //! to check application buffer availability
  uint8_t buf_in_use;

  //! application events bit map
  uint32_t event_map;

} wlan_app_cb_t;
wlan_app_cb_t wlan_app_cb; //! application control block

/*
 *********************************************************************************************************
 *                                               DATA TYPES
 *********************************************************************************************************
 */
extern void wifi_app_send_to_ble(uint16_t msg_type, uint8_t *data, uint16_t data_len);
extern uint8_t coex_ssid[50], pwd[34], sec_type;
void lcd_mac(void);

uint8_t conn_status;
extern uint8_t magic_word;

// WLAN include file for configuration
osSemaphoreId_t rsi_mqtt_sem;
extern osSemaphoreId_t wlan_thread_sem;

static sl_net_wifi_client_profile_t wifi_client_profile = {
    .config = {
        .ssid.value = WIFI_CLIENT_PROFILE_SSID,
        .ssid.length = sizeof(WIFI_CLIENT_PROFILE_SSID)-1,
        .channel.channel = SL_WIFI_AUTO_CHANNEL,
        .channel.band = SL_WIFI_AUTO_BAND,
        .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH,
        .bssid = {{0}},
        .bss_type = SL_WIFI_BSS_TYPE_INFRASTRUCTURE,
        .security = SL_WIFI_WPA2,
        .encryption = SL_WIFI_DEFAULT_ENCRYPTION,
        .client_options = 0,
        .credential_id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
    },
    .ip = {
        .mode = SL_IP_MANAGEMENT_DHCP,
        .type = SL_IPV4,
        .host_name = DHCP_HOST_NAME,
        .ip = {{{0}}},

    }
};

/*==============================================*/
/**
 * @fn         wifi_app_set_event
 * @brief      sets the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to set/raise the specific event.
 */
void wifi_app_set_event(uint32_t event_num)
{
  wlan_app_event_map |= BIT(event_num);

  osSemaphoreRelease(wlan_thread_sem);

  return;
}

/*==============================================*/
/**
 * @fn         wifi_app_clear_event
 * @brief      clears the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to clear the specific event.
 */
void wifi_app_clear_event(uint32_t event_num)
{
  wlan_app_event_map &= ~BIT(event_num);
  return;
}

/*==============================================*/
/**
 * @fn         wifi_app_get_event
 * @brief      returns the first set event based on priority
 * @param[in]  none.
 * @return     int32_t
 *             > 0  = event number
 *             -1   = not received any event
 * @section description
 * This function returns the highest priority event among all the set events
 */
int32_t wifi_app_get_event(void)
{
  uint32_t ix;

  for (ix = 0; ix < 32; ix++) {
    if (wlan_app_event_map & (1 << ix)) {
      return ix;
    }
  }

  return (-1);
}

// rejoin failure call back handler in station mode
sl_status_t join_callback_handler(sl_wifi_event_t event,
                                  sl_status_t status_code,
                                  char *result,
                                  uint32_t result_length,
                                  void *arg)
{
  UNUSED_PARAMETER(result);
  UNUSED_PARAMETER(result_length);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }

  // Update WLAN application state
  disconnected = 1;
  connected    = 0;

  wifi_app_set_event(WIFI_APP_DISCONNECTED_STATE);

  return SL_STATUS_OK;
}

void rsi_wlan_app_callbacks_init(void)
{
  //! Initialize join fail call back
  sl_wifi_set_join_callback_v2(join_callback_handler, NULL);
}

static sl_status_t show_scan_results()
{
  printf("%lu Scan results:\n", scan_result->scan_count);

  if (scan_result->scan_count) {
    printf("\n   %s %24s %s", "SSID", "SECURITY", "NETWORK");
    printf("%12s %12s %s\n", "BSSID", "CHANNEL", "RSSI");

    for (int a = 0; a < (int)scan_result->scan_count; ++a) {
      uint8_t *bssid = (uint8_t *)&scan_result->scan_info[a].bssid;
      printf("%-24s %4u,  %4u, ",
             scan_result->scan_info[a].ssid,
             scan_result->scan_info[a].security_mode,
             scan_result->scan_info[a].network_type);
      printf("  %02x:%02x:%02x:%02x:%02x:%02x, %4u,  -%u\n",
             bssid[0],
             bssid[1],
             bssid[2],
             bssid[3],
             bssid[4],
             bssid[5],
             scan_result->scan_info[a].rf_channel,
             scan_result->scan_info[a].rssi_val);
    }
  }

  return SL_STATUS_OK;
}

sl_status_t wlan_app_scan_callback_handler(sl_wifi_event_t event,
                                           sl_status_t status_code,
                                           sl_wifi_scan_result_t *result,
                                           uint32_t result_length,
                                           void *arg)
{
  UNUSED_PARAMETER(result_length);
  UNUSED_PARAMETER(arg);

  scan_complete = true;

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    callback_status = status_code;
    return status_code;
  }

  memset(scan_result, 0, scanbuf_size);
  memcpy(scan_result, result, scanbuf_size);

  if (result_length != 0) {
    callback_status = show_scan_results();
  }

  // Send wlan scan result to BLE module
  wifi_app_send_to_ble(WIFI_APP_SCAN_RESP, (uint8_t *)scan_result, scanbuf_size);

  return SL_STATUS_OK;
}

sl_status_t network_event_handler(sl_net_event_t event, sl_status_t status, void *data, uint32_t data_length)
{
  UNUSED_PARAMETER(data_length);
  switch (event) {
    case SL_NET_PING_RESPONSE_EVENT: {
      sl_net_ping_response_t *response = (sl_net_ping_response_t *)data;
      UNUSED_VARIABLE(response);
      if (status != SL_STATUS_OK) {
        printf("\r\nPing request unsuccessful\r\n");
        return status;
      }
      printf("\r\nPing response from www.silabs.com \r\n");
      break;
    }
    default:
      break;
  }

  return SL_STATUS_OK;
}

void ping_silabs()
{
  sl_status_t status = 0;
  uint32_t i         = 0;

  sl_ip_address_t remote_ip_address = { 0 };

  remote_ip_address.type = SL_IPV4;

  UNUSED_VARIABLE(remote_ip_address);

  sl_ip_address_t dns_query_rsp = { 0 };
  uint32_t server_address;
  int32_t dns_retry_count = MAX_DNS_RETRY_COUNT;
  char server_ip[16];

  do {
    //! Getting IP address of the remote server using DNS request
    status = sl_net_dns_resolve_hostname((const char *)hostname, DNS_TIMEOUT, SL_NET_DNS_TYPE_IPV4, &dns_query_rsp);
    dns_retry_count--;
  } while ((dns_retry_count != 0) && (status != SL_STATUS_OK));

  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error while resolving dns, Error 0x%lX\r\n", status);
    return;
  }

  server_address = dns_query_rsp.ip.v4.value;
  sprintf((char *)server_ip,
          "%ld.%ld.%ld.%ld",
          server_address & 0x000000ff,
          (server_address & 0x0000ff00) >> 8,
          (server_address & 0x00ff0000) >> 16,
          (server_address & 0xff000000) >> 24);

  printf("\r\nResolved DNS - www.silabs.com IP address = %s\n", server_ip);
  printf("\r\nPinging www.silabs.com");

  while (i < NO_OF_PINGS) {
    // Send ping
    status = sl_si91x_send_ping(dns_query_rsp, PING_PACKET_SIZE);
    if (status != SL_STATUS_IN_PROGRESS) {
      printf("\r\nPing request failed with status 0x%lX\r\n", status);
      return;
    }

    // Sleep for 2 seconds
    osDelay(2000);
    i++;
  }
}

void mqtt_client_cleanup()
{
  SL_CLEANUP_MALLOC(client_credentails);
  is_execution_completed = 1;
}

void mqtt_client_message_handler(void *client, sl_mqtt_client_message_t *message, void *context)
{
  UNUSED_PARAMETER(context);
  UNUSED_PARAMETER(client);
  printf("Message Received on Topic: ");
  print_char_buffer((char *)message->topic, message->topic_length);
  printf("\r\n");
  print_char_buffer((char *)message->content, message->content_length);
  printf("\r\n");
  strncpy(msg, (char *)message->content, message->content_length);
  msg[message->content_length] = '\0';
  GLIB_clear(&glibContext);
  GLIB_drawBitmap(&glibContext,
                  SILABS_LOGO_POSITION_X,
                  SILABS_LOGO_POSITION_Y + 5,
                  SILABS_LOGO_WIDTH,
                  SILABS_LOGO_HEIGHT,
                  silabsLogo);
  GLIB_drawBitmap(&glibContext,
                  BLE_ICON_POSITION_X,
                  BLE_ICON_POSITION_Y + 5,
                  BLUETOOTH_ICON_SIZE,
                  BLUETOOTH_ICON_SIZE,
                  bleLogo);
  GLIB_drawBitmap(&glibContext,
                  WIFI_ICON_POSITION_X,
                  WIFI_ICON_POSITION_Y + 5,
                  WIFI_BITMAP_HEIGHT,
                  WIFI_BITMAP_HEIGHT,
                  wifiLogo);
  currentLine = 3;
  GLIB_drawStringOnLine(&glibContext, "MQTT message", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
  GLIB_drawStringOnLine(&glibContext, "received", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
  GLIB_drawStringOnLine(&glibContext, "", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
  GLIB_drawStringOnLine(&glibContext, "Topic:", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
  GLIB_drawStringOnLine(&glibContext, TOPIC_TO_BE_SUBSCRIBED, currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
  GLIB_drawStringOnLine(&glibContext, "", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
  GLIB_drawStringOnLine(&glibContext, "Message:", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
  GLIB_drawStringOnLine(&glibContext, (char *)message->content, currentLine, GLIB_ALIGN_LEFT, 5, 5, true);
  DMD_updateDisplay();
}

void print_char_buffer(char *buffer, uint32_t buffer_length)
{
  printf("\r\n");

  for (uint32_t index = 0; index < buffer_length; index++) {
    printf("%c", buffer[index]);
  }
}

void mqtt_client_error_event_handler(void *client, sl_mqtt_client_error_status_t *error)
{
  UNUSED_PARAMETER(client);
  printf("Terminating program, Error: %d\r\n", *error);
  mqtt_client_cleanup();
}

void mqtt_client_event_handler(void *client, sl_mqtt_client_event_t event, void *event_data, void *context)
{
  switch (event) {
    case SL_MQTT_CLIENT_CONNECTED_EVENT: {
      printf("\r\nMQTT connection established\r\n");

      sl_status_t status;

      status = sl_mqtt_client_subscribe(client,
                                        (uint8_t *)TOPIC_TO_BE_SUBSCRIBED,
                                        strlen(TOPIC_TO_BE_SUBSCRIBED),
                                        QOS_OF_SUBSCRIPTION,
                                        0,
                                        mqtt_client_message_handler,
                                        TOPIC_TO_BE_SUBSCRIBED);
      if (status != SL_STATUS_IN_PROGRESS) {
        printf("\r\nFailed to subscribe : 0x%lx\r\n", status);

        mqtt_client_cleanup();
        return;
      }

      break;
    }

    case SL_MQTT_CLIENT_MESSAGE_PUBLISHED_EVENT: {
      sl_mqtt_client_message_t *published_message = (sl_mqtt_client_message_t *)context;
      printf("Published message successfully on topic: ");
      print_char_buffer((char *)published_message->topic, published_message->topic_length);
      print_char_buffer((char *)published_message->content, published_message->content_length);
      printf("\r\n");
      GLIB_clear(&glibContext);
      GLIB_drawBitmap(&glibContext,
                      SILABS_LOGO_POSITION_X,
                      SILABS_LOGO_POSITION_Y + 5,
                      SILABS_LOGO_WIDTH,
                      SILABS_LOGO_HEIGHT,
                      silabsLogo);
      GLIB_drawBitmap(&glibContext,
                      BLE_ICON_POSITION_X,
                      BLE_ICON_POSITION_Y + 5,
                      BLUETOOTH_ICON_SIZE,
                      BLUETOOTH_ICON_SIZE,
                      bleLogo);
      GLIB_drawBitmap(&glibContext,
                      WIFI_ICON_POSITION_X,
                      WIFI_ICON_POSITION_Y + 5,
                      WIFI_BITMAP_HEIGHT,
                      WIFI_BITMAP_HEIGHT,
                      wifiLogo);
      currentLine = 3;
      GLIB_drawStringOnLine(&glibContext, "Message published", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
      GLIB_drawStringOnLine(&glibContext, "to MQTT broker", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
      GLIB_drawStringOnLine(&glibContext, " ", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
      GLIB_drawStringOnLine(&glibContext, "Topic: ", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
      GLIB_drawStringOnLine(&glibContext, PUBLISH_TOPIC, currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
      GLIB_drawStringOnLine(&glibContext, "", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
      GLIB_drawStringOnLine(&glibContext, "Message: ", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
      GLIB_drawStringOnLine(&glibContext,
                            (char *)published_message->content,
                            currentLine++,
                            GLIB_ALIGN_LEFT,
                            5,
                            5,
                            true);
      DMD_updateDisplay();

      break;
    }

    case SL_MQTT_CLIENT_SUBSCRIBED_EVENT: {
      char *subscribed_topic = (char *)context;

      printf("Subscribed to Topic: %s\r\n", subscribed_topic);
      mqtt_connected = 1;
      break;
    }

    case SL_MQTT_CLIENT_UNSUBSCRIBED_EVENT: {
      char *unsubscribed_topic = (char *)context;

      printf("\r\nUnsubscribed from topic: %s\r\n", unsubscribed_topic);

      sl_mqtt_client_disconnect(client, 0);
      break;
    }

    case SL_MQTT_CLIENT_DISCONNECTED_EVENT: {
      printf("\r\nDisconnected from MQTT broker\r\n");
      mqtt_disconnect_flag = 1;
      mqtt_client_cleanup();
      break;
    }

    case SL_MQTT_CLIENT_ERROR_EVENT: {
      mqtt_client_error_event_handler(client, (sl_mqtt_client_error_status_t *)event_data);
      break;
    }
    default:
      break;
  }
}

void test_mosquitto_org_pub()
{
  sl_status_t status = 0;
  status             = sl_mqtt_client_publish(&client, &message_to_be_published, 0, &message_to_be_published);
  if (status != SL_STATUS_IN_PROGRESS) {
    printf("\r\nFailed to publish message: 0x%lx\r\n", status);
    mqtt_client_cleanup();
    return;
  }
}

sl_status_t mqtt_example()
{
  sl_status_t status;

  status = sl_mqtt_client_init(&client, mqtt_client_event_handler);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to initialize MQTT client: 0x%lx\r\n", status);

    mqtt_client_cleanup();
    return status;
  }
  printf("\r\nMQTT client initialization successful\r\n");

  sl_ip_address_t remote_ip_address = { 0 };

  remote_ip_address.type = SL_IPV4;

  UNUSED_VARIABLE(remote_ip_address);

  sl_ip_address_t dns_query_rsp = { 0 };
  uint32_t server_address;
  int32_t dns_retry_count = MAX_DNS_RETRY_COUNT;
  char server_ip[16];

  do {

    status =
      sl_net_dns_resolve_hostname((const char *)mqtt_hostname, DNS_TIMEOUT, SL_NET_DNS_TYPE_IPV4, &dns_query_rsp);
    dns_retry_count--;
  } while ((dns_retry_count != 0) && (status != SL_STATUS_OK));

  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error while resolving DNS, Error 0x%lX\r\n", status);
  }

  server_address = dns_query_rsp.ip.v4.value;
  sprintf((char *)server_ip,
          "%ld.%ld.%ld.%ld",
          server_address & 0x000000ff,
          (server_address & 0x0000ff00) >> 8,
          (server_address & 0x00ff0000) >> 16,
          (server_address & 0xff000000) >> 24);

  printf("\r\nResolved test.mosquitto.org's IP address = %s\n", server_ip);

  mqtt_broker_configuration.ip.type        = SL_IPV4;
  mqtt_broker_configuration.ip.ip.v4.value = dns_query_rsp.ip.v4.value;

  sl_mac_address_t mac_addr = { 0 };
  uint8_t mac_id[18];
  sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_addr);
  sprintf((char *)mac_id,
          "%x:%x:%x:%x:%x:%x",
          mac_addr.octet[0],
          mac_addr.octet[1],
          mac_addr.octet[2],
          mac_addr.octet[3],
          mac_addr.octet[4],
          mac_addr.octet[5]);
  /* The MQTT client ID is set to device MAC ID, to ensure that a unique client ID is selected, 
  as the remote MQTT server rejects multiple simultaneous connections with same MQTT ID*/
  mqtt_client_configuration.client_id        = mac_id;
  mqtt_client_configuration.client_id_length = strlen((char *)mqtt_client_configuration.client_id);

  status =
    sl_mqtt_client_connect(&client, &mqtt_broker_configuration, &last_will_message, &mqtt_client_configuration, 0);
  if (status != SL_STATUS_IN_PROGRESS) {
    printf("Failed to connect to MQTT broker: 0x%lx\r\n", status);

    mqtt_client_cleanup();
    return status;
  }

  osDelay(2000);

  return SL_STATUS_OK;
}

void wifi_app_task(void)
{
  int32_t status   = RSI_SUCCESS;
  int32_t event_id = 0;

  // Allocate memory for scan buffer
  scan_result = (sl_wifi_scan_result_t *)malloc(scanbuf_size);
  if (scan_result == NULL) {
    LOG_PRINT("Failed to allocate memory for scan result\n");
    return;
  }
  memset(scan_result, 0, scanbuf_size);
  while (1) {
    // checking for events list
    event_id = wifi_app_get_event();
    if (event_id == -1) {
      osSemaphoreAcquire(wlan_thread_sem, osWaitForever);

      // if events are not received loop will be continued.
      continue;
    }

    switch (event_id) {
      case WIFI_APP_INITIAL_STATE: {
        wifi_app_clear_event(WIFI_APP_INITIAL_STATE);

        // Update WLAN application state
        if (magic_word) {
          wifi_app_set_event(WIFI_APP_FLASH_STATE);
        } else {
          wifi_app_set_event(WIFI_APP_SCAN_STATE);
        }
      } break;

      case WIFI_APP_UNCONNECTED_STATE: {
        wifi_app_clear_event(WIFI_APP_UNCONNECTED_STATE);

        osSemaphoreRelease(wlan_thread_sem);
      } break;

      case WIFI_APP_SCAN_STATE: {
        wifi_app_clear_event(WIFI_APP_SCAN_STATE);

        sl_wifi_scan_configuration_t wifi_scan_configuration = { 0 };
        wifi_scan_configuration                              = default_wifi_scan_configuration;
        scan_complete                                        = false;
        callback_status                                      = SL_STATUS_FAIL;

        sl_wifi_set_scan_callback_v2(wlan_app_scan_callback_handler, NULL);

        status = sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, NULL, &wifi_scan_configuration);
        if (SL_STATUS_IN_PROGRESS == status) {
          const uint32_t start = osKernelGetTickCount();

          while (!scan_complete && (osKernelGetTickCount() - start) <= WIFI_SCAN_TIMEOUT) {
            osThreadYield();
          }
          status = scan_complete ? callback_status : SL_STATUS_TIMEOUT;
        }
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nWLAN Scan Wait Failed, Error Code : 0x%lX\r\n", status);
          GLIB_drawStringOnLine(&glibContext, "WLAN Scan Failed", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
          DMD_updateDisplay();
          osDelay(1000);
          wifi_app_set_event(WIFI_APP_SCAN_STATE);
          osDelay(1000);
        }
      } break;

      case WIFI_APP_JOIN_STATE: {
        sl_wifi_credential_t cred = { 0 };

        wifi_app_clear_event(WIFI_APP_JOIN_STATE);

        cred.type = SL_WIFI_PSK_CREDENTIAL;
        memcpy(cred.psk.value, pwd, strlen((char *)pwd));

        wifi_client_profile.config.ssid.length = strlen((char *)coex_ssid);
        memcpy(wifi_client_profile.config.ssid.value, coex_ssid, wifi_client_profile.config.ssid.length);
        wifi_client_profile.config.security = sec_type;

        // Set the custom Wi-Fi client profile
        if (sec_type != SL_WIFI_OPEN) {
          status =
            sl_net_set_credential(wifi_client_profile.config.credential_id, SL_NET_WIFI_PSK, pwd, strlen((char *)pwd));
          if (status != SL_STATUS_OK) {
            printf("\r\nFailed to set client credentials: 0x%lx\r\n", status);
            continue;
          }
        } else {
          // For OPEN security, clear the credential ID
          wifi_client_profile.config.credential_id = SL_WIFI_NO_CREDENTIAL_ID;
        }

        status =
          sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &wifi_client_profile);
        if (SL_STATUS_OK == status) {
          wifi_client_profile.config.ssid.length = strlen((char *)coex_ssid);
          memcpy(wifi_client_profile.config.ssid.value, coex_ssid, wifi_client_profile.config.ssid.length);
          wifi_client_profile.config.security = sec_type;

          // Enabling required MFP bits in join feature bitmap for WPA3 Personal mode security type
          if (wifi_client_profile.config.security == SL_WIFI_WPA3) {
            status = sl_wifi_set_join_configuration(
              SL_WIFI_CLIENT_2_4GHZ_INTERFACE,
              SL_WIFI_JOIN_FEAT_MFP_CAPABLE_REQUIRED | SL_WIFI_JOIN_FEAT_LISTEN_INTERVAL_VALID);
            if (status != SL_STATUS_OK) {
              printf("\r\n join configuration settings for WPA3 failed\r\n");
            }
          } else if (wifi_client_profile.config.security == SL_WIFI_WPA3_TRANSITION) {
            status = sl_wifi_set_join_configuration(
              SL_WIFI_CLIENT_2_4GHZ_INTERFACE,
              SL_WIFI_JOIN_FEAT_MFP_CAPABLE_ONLY | SL_WIFI_JOIN_FEAT_LISTEN_INTERVAL_VALID);
            if (status != SL_STATUS_OK) {
              printf("\r\n Join configuration settings for WPA3 failed\r\n");
            }
          } else {
            status =
              sl_wifi_set_join_configuration(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, SL_WIFI_JOIN_FEAT_LISTEN_INTERVAL_VALID);
            if (status != SL_STATUS_OK) {
              printf("\r\n Join configuration settings for WPA3 failed\r\n");
            }
          }

          printf("\r\nSelected SSID:");
          for (int i = 0; i < wifi_client_profile.config.ssid.length; i++) {
            printf("%c", wifi_client_profile.config.ssid.value[i]);
          }

          do {
            status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
            app_reconn_loop_ctr++;
          } while ((status != SL_STATUS_OK) && (app_reconn_loop_ctr < APP_RECONN_LOOP_CTR_LIM));
          app_reconn_loop_ctr = 0;
        }
        if (status != RSI_SUCCESS) {
          timeout = 1;
          wifi_app_send_to_ble(WIFI_APP_TIMEOUT_NOTIFY, (uint8_t *)&timeout, 1);
          LOG_PRINT("\r\nWLAN Connect Failed, Error Code : 0x%lX\r\n", status);

          GLIB_drawStringOnLine(&glibContext, "WLAN Connect Failed", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
          DMD_updateDisplay();
          osDelay(1000);

          // Update WLAN application state
          disconnected = 1;
          connected    = 0;
        } else {

          LOG_PRINT("\r\nWLAN connection successful\r\n");

          // Update WLAN application state
          wifi_app_set_event(WIFI_APP_CONNECTED_STATE);
        }

      } break;

      case WIFI_APP_FLASH_STATE: {
        wifi_app_clear_event(WIFI_APP_FLASH_STATE);

        if (retry) {
          status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\nWLAN connection failed, Error Code : 0x%lX\r\n", status);
            break;
          } else {
            wifi_app_set_event(WIFI_APP_CONNECTED_STATE);
          }
        }
      } break;

      case WIFI_APP_CONNECTED_STATE: {
        wifi_app_clear_event(WIFI_APP_CONNECTED_STATE);

        // Configure IP
        status =
          sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &client_profile);
        ;
        if (status != RSI_SUCCESS) {
          a++;
          if (a == 3) {
            a       = 0;
            timeout = 1;
            status  = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
            if (status == RSI_SUCCESS) {
              connected     = 0;
              disassociated = 1;
              wifi_app_send_to_ble(WIFI_APP_TIMEOUT_NOTIFY, (uint8_t *)&timeout, 1);
              wifi_app_set_event(WIFI_APP_IDLE_STATE);
            }
          }
          LOG_PRINT("\r\nIP configuration failed, Error Code : 0x%lX\r\n", status);
          break;
        } else {
          a             = 0;
          connected     = 1;
          conn_status   = 1;
          disconnected  = 0;
          disassociated = 0;

          fetch_ip = &client_profile.ip.ip.v4.ip_address;
          sprintf(ip_add,
                  "%d.%d.%d.%d",
                  fetch_ip->bytes[0],
                  fetch_ip->bytes[1],
                  fetch_ip->bytes[2],
                  fetch_ip->bytes[3]);
          printf("\r\nIP Address:%s \r\n", ip_add);
          GLIB_clear(&glibContext);
          GLIB_drawBitmap(&glibContext,
                          SILABS_LOGO_POSITION_X,
                          SILABS_LOGO_POSITION_Y + 5,
                          SILABS_LOGO_WIDTH,
                          SILABS_LOGO_HEIGHT,
                          silabsLogo);
          GLIB_drawBitmap(&glibContext,
                          BLE_ICON_POSITION_X,
                          BLE_ICON_POSITION_Y + 5,
                          BLUETOOTH_ICON_SIZE,
                          BLUETOOTH_ICON_SIZE,
                          bleLogo);
          GLIB_drawBitmap(&glibContext,
                          WIFI_ICON_POSITION_X,
                          WIFI_ICON_POSITION_Y + 5,
                          WIFI_BITMAP_HEIGHT,
                          WIFI_BITMAP_HEIGHT,
                          wifiLogo);
          currentLine = 3;
          GLIB_drawStringOnLine(&glibContext, "WLAN connected (IP):", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
          GLIB_drawStringOnLine(&glibContext, ip_add, currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
          DMD_updateDisplay();
          osDelay(1000);

          // Update WLAN application state
          wifi_app_send_to_ble(WIFI_APP_CONNECTION_STATUS, (uint8_t *)&connected, 1);
          wifi_app_set_event(WIFI_APP_IPCONFIG_DONE_STATE);
          osDelay(1000);
        }
      } break;

      case WIFI_APP_IPCONFIG_DONE_STATE: {
        wifi_app_clear_event(WIFI_APP_IPCONFIG_DONE_STATE);
        wlan_app_cb.state = WIFI_APP_MQTT_INIT_STATE;

        sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_addr);
        sprintf(mac_id,
                "%.02X:%.02X:%.02X:%.02X:%.02X:%.02X",
                mac_addr.octet[0],
                mac_addr.octet[1],
                mac_addr.octet[2],
                mac_addr.octet[3],
                mac_addr.octet[4],
                mac_addr.octet[5]);
        printf("\r\nMAC Address:%s \r\n", mac_id);

        GLIB_drawStringOnLine(&glibContext, "", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);

        lcd_mac();

        GLIB_drawStringOnLine(&glibContext, "Pinging", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
        GLIB_drawStringOnLine(&glibContext, "www.silabs.com", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);

        DMD_updateDisplay();
        osDelay(1000);

        ping_silabs();

        GLIB_clear(&glibContext);
        GLIB_drawBitmap(&glibContext,
                        SILABS_LOGO_POSITION_X,
                        SILABS_LOGO_POSITION_Y + 5,
                        SILABS_LOGO_WIDTH,
                        SILABS_LOGO_HEIGHT,
                        silabsLogo);
        GLIB_drawBitmap(&glibContext,
                        BLE_ICON_POSITION_X,
                        BLE_ICON_POSITION_Y + 5,
                        BLUETOOTH_ICON_SIZE,
                        BLUETOOTH_ICON_SIZE,
                        bleLogo);
        GLIB_drawBitmap(&glibContext,
                        WIFI_ICON_POSITION_X,
                        WIFI_ICON_POSITION_Y + 5,
                        WIFI_BITMAP_HEIGHT,
                        WIFI_BITMAP_HEIGHT,
                        wifiLogo);
        currentLine = 3;
        GLIB_drawStringOnLine(&glibContext, "WLAN connected (IP):", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
        GLIB_drawStringOnLine(&glibContext, ip_add, currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
        GLIB_drawStringOnLine(&glibContext, "", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
        GLIB_drawStringOnLine(&glibContext, "Ping success", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
        GLIB_drawStringOnLine(&glibContext, "www.silabs.com", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
        GLIB_drawStringOnLine(&glibContext, "", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
        DMD_updateDisplay();

        mqtt_example();

        while (mqtt_connected != 1)
          ;

        GLIB_drawStringOnLine(&glibContext, "MQTT connection done", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
        DMD_updateDisplay();

        //! initiating power save in BLE mode
        status = rsi_bt_power_save_profile(PSP_MODE, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\nFailed to initiate power save in BLE mode\r\n");
        }
        uint32_t rc                                          = SL_STATUS_FAIL;
        sl_wifi_performance_profile_v2_t performance_profile = { .profile         = ASSOCIATED_POWER_SAVE_LOW_LATENCY,
                                                                 .listen_interval = 1000 };

        rc = sl_wifi_set_performance_profile_v2(&performance_profile);
        if (rc != SL_STATUS_OK) {
          printf("\r\nPower save configuration Failed, Error Code : 0x%lX\r\n", rc);
        }
        printf("\r\nAssociated power save enabled\r\n");
        GLIB_drawStringOnLine(&glibContext, "", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
        GLIB_drawStringOnLine(&glibContext, "NWP Lowpower enabled", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
        DMD_updateDisplay();
        osDelay(1000);
        wifi_app_set_event(WIFI_APP_IDLE_STATE);
      } break;

      case WIFI_APP_IDLE_STATE: {
        osDelay(1000);
      } break;

      case WIFI_APP_DISCONNECTED_STATE: {
        wifi_app_clear_event(WIFI_APP_DISCONNECTED_STATE);
        retry = 1;
        wifi_app_send_to_ble(WIFI_APP_DISCONNECTION_STATUS, (uint8_t *)&disconnected, 1);
        wifi_app_set_event(WIFI_APP_FLASH_STATE);
      } break;

      case WIFI_APP_DISCONN_NOTIFY_STATE: {
        wifi_app_clear_event(WIFI_APP_DISCONN_NOTIFY_STATE);

        if (client.state == SL_MQTT_CLIENT_CONNECTED) {
          sl_mqtt_client_disconnect(&client, 0);
          do {
            osDelay(10);
          } while (mqtt_disconnect_flag == 0);
        }

        mqtt_connected = 0;

        if (client.state == SL_MQTT_CLIENT_DISCONNECTED) {
          sl_mqtt_client_deinit(&client);
        }

        status = sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);

        if (status == RSI_SUCCESS) {
#if RSI_WISE_MCU_ENABLE
          rsi_flash_erase((uint32_t)FLASH_ADDR_TO_STORE_AP_DETAILS);
#endif
          LOG_PRINT("\r\nWLAN disconnected\r\n");
          disassociated   = 1;
          connected       = 0;
          yield           = 0;
          disconnect_flag = 0; // reset flag to allow disconnecting again

          wifi_app_send_to_ble(WIFI_APP_DISCONNECTION_NOTIFY, (uint8_t *)&disassociated, 1);
          wifi_app_set_event(WIFI_APP_UNCONNECTED_STATE);
        } else {
          LOG_PRINT("\r\nWi-Fi disconnect failed, Error Code : 0x%lX\r\n", status);
        }
      } break;
      default:
        break;
    }
  }

  if (scan_result != NULL) {
    free(scan_result);
  }
}

void memlcd_app_init(void)
{
  sl_status_t status;

  // Enabling LCD display
  RSI_NPSSGPIO_InputBufferEn(SL_BOARD_ENABLE_DISPLAY_PIN, 1U);
  RSI_NPSSGPIO_SetPinMux(SL_BOARD_ENABLE_DISPLAY_PIN, 0);
  RSI_NPSSGPIO_SetDir(SL_BOARD_ENABLE_DISPLAY_PIN, 0);
  RSI_NPSSGPIO_SetPin(SL_BOARD_ENABLE_DISPLAY_PIN, 1U);

  /* Initialize the DMD support for memory lcd display */
  status = DMD_init(0);
  EFM_ASSERT(status == DMD_OK);

  /* Initialize the glib context */
  status = GLIB_contextInit(&glibContext);
  EFM_ASSERT(status == GLIB_OK);

  glibContext.backgroundColor = White;
  glibContext.foregroundColor = Black;

  /* Fill lcd with background color */
  GLIB_clear(&glibContext);

  /* Use Narrow font */
  GLIB_setFont(&glibContext, (GLIB_Font_t *)&GLIB_FontNarrow6x8);
}

void lcd_mac(void)
{
  GLIB_drawStringOnLine(&glibContext, "MAC ID:", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
  GLIB_drawStringOnLine(&glibContext, mac_id, currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
  GLIB_drawStringOnLine(&glibContext, "                   ", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
  DMD_updateDisplay();
}

static osStatus_t set_sensor_event(uint32_t event)
{
  return osEventFlagsSet(sensor_event_flags, event);
}

static uint32_t wait_for_sensor_event(uint32_t event_mask)
{
  uint32_t event = osEventFlagsWait(sensor_event_flags, event_mask, (osFlagsWaitAny | osFlagsNoClear), osWaitForever);

  // Clear the received event flags
  osEventFlagsClear(sensor_event_flags, event_mask);
  return event;
}

void sensor_event_thread(void *argument)
{
  UNUSED_PARAMETER(argument);
  sensor_event_flags = osEventFlagsNew(NULL);

  while (1) {
    uint32_t event = wait_for_sensor_event(SENSOR_EVENT_BUTTON_PRESSED);

    for (uint8_t a = 0; a < 32; a++) {
      uint32_t current_event = event & (1 << a);
      switch (current_event) {
        case SENSOR_EVENT_BUTTON_PRESSED:
          if (mqtt_connected == 1) {
            test_mosquitto_org_pub();
          }
          break;

        default:
          break;
      }
    }
  }
}

void sl_si91x_button_isr(uint8_t pin, int8_t state)
{
  if ((pin == BUTTON_INSTANCE_0.pin) && (state == BUTTON_PRESSED)) {
    set_sensor_event(SENSOR_EVENT_BUTTON_PRESSED);
  }
}
