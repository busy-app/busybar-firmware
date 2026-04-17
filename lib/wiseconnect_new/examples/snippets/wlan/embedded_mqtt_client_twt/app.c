/***************************************************************************/ /**
 * @file
 * @brief Embedded MQTT Client TWT Example Application
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
#include "sl_utility.h"
#include "cmsis_os2.h"
#include "sl_constants.h"
#include "sl_mqtt_client.h"
#include "cacert.pem.h"
#include "sl_wifi.h"
#include "sl_wifi_device.h"
#include "string.h"
#include "sl_wifi_callback_framework.h"
#include "sl_net_wifi_types.h"

#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_power_manager.h"
#include "sl_si91x_m4_ps.h"
#endif

/******************************************************
 *                    Constants
 ******************************************************/
#define ENABLE_MQTT_SUBSCRIBE_PUBLISH 0

#define MQTT_BROKER_PORT 8886

#define CLIENT_PORT 1

#define CLIENT_ID "WIFI-SDK-MQTT-CLIENT"

#define TOPIC_TO_BE_SUBSCRIBED "THERMOSTAT-DATA\0"
#define QOS_OF_SUBSCRIPTION    SL_MQTT_QOS_LEVEL_1

#define PUBLISH_TOPIC          TOPIC_TO_BE_SUBSCRIBED
#define PUBLISH_MESSAGE        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do"
#define QOS_OF_PUBLISH_MESSAGE 0

#define IS_DUPLICATE_MESSAGE 0
#define IS_MESSAGE_RETAINED  0
#define IS_CLEAN_SESSION     1

#define LAST_WILL_TOPIC       "WiFiSDK-MQTT-CLIENT-LAST-WILL"
#define LAST_WILL_MESSAGE     "WiFiSDK-MQTT-CLIENT has been disconnect from network"
#define QOS_OF_LAST_WILL      1
#define IS_LAST_WILL_RETAINED 1

#define ENCRYPT_CONNECTION   0
#define KEEP_ALIVE_INTERVAL  2000 // in seconds
#define MQTT_CONNECT_TIMEOUT 5000 // in milli seconds

#define SEND_CREDENTIALS 0

#define USERNAME "WIFISDK"
#define PASSWORD "password"

#define TWT_AUTO_CONFIG  1
#define TWT_SCAN_TIMEOUT 10000

// Use case based TWT selection params
#define DEVICE_AVERAGE_THROUGHPUT            20000
#define ESTIMATE_EXTRA_WAKE_DURATION_PERCENT 0
#define TWT_TOLERABLE_DEVIATION              10
#define TWT_DEFAULT_WAKE_INTERVAL_MS         1024     // in milli seconds
#define TWT_DEFAULT_WAKE_DURATION_MS         8        // in milli seconds
#define MAX_TX_AND_RX_LATENCY_LIMIT          22118400 // 6hrs in milli seconds
#define MAX_BEACON_WAKE_UP_AFTER_SP \
  2 // The number of beacons after the service period completion for which the module wakes up and listens for any pending RX.

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

static sl_wifi_device_configuration_t wifi_mqtt_client_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
#ifdef SLI_SI91X_MCU_INTERFACE
                      | SL_WIFI_FEAT_WPS_DISABLE
#endif
                      ),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID
                                              | SL_SI91X_TCP_IP_FEAT_SSL),
                   .custom_feature_bit_map = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map = (SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK
                                                  | SL_SI91X_EXT_FEAT_DISABLE_DEBUG_PRINTS | MEMORY_CONFIG
#ifdef SLI_SI917
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID | SL_SI91X_EXT_EMB_MQTT_ENABLE),
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | SL_WIFI_ENABLE_ENHANCED_MAX_PSP) }
};

sl_wifi_twt_request_t default_twt_setup_configuration = {
  .twt_enable              = 1,
  .twt_flow_id             = 1,
  .wake_duration           = 0x80,
  .wake_duration_unit      = 0,
  .wake_duration_tol       = 0x80,
  .wake_int_exp            = 13,
  .wake_int_exp_tol        = 13,
  .wake_int_mantissa       = 0x1B00,
  .wake_int_mantissa_tol   = 0x1B00,
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

sl_status_t callback_status = SL_STATUS_OK;

sl_mqtt_client_t client              = { 0 };
sl_ip_address_t ip_address           = { 0 };
sl_net_wifi_client_profile_t profile = { 0 };

uint8_t is_execution_completed = 0;

sl_mqtt_client_credentials_t *client_credentails = NULL;

sl_mqtt_client_configuration_t mqtt_client_configuration = { .is_clean_session = IS_CLEAN_SESSION,
                                                             .client_id        = (uint8_t *)CLIENT_ID,
                                                             .client_id_length = strlen(CLIENT_ID),
                                                             .client_port      = CLIENT_PORT };

sl_mqtt_broker_t mqtt_broker_configuration = {
  .ip                      = SL_IPV4_ADDRESS(192, 168, 1, 13),
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
static void application_start(void *argument);
void mqtt_client_message_handler(void *client, sl_mqtt_client_message_t *message, void *context);
void mqtt_client_event_handler(void *client, sl_mqtt_client_event_t event, void *event_data, void *context);
void mqtt_client_error_event_handler(void *client, sl_mqtt_client_error_status_t *error);
void mqtt_client_cleanup();
void print_char_buffer(char *buffer, uint32_t buffer_length);
sl_status_t mqtt_example();
sl_status_t twt_callback_handler(sl_wifi_event_t event,
                                 sl_status_t status_code,
                                 sl_wifi_twt_response_t *result,
                                 uint32_t result_length,
                                 void *arg);
/******************************************************
 *               Function Definitions
 ******************************************************/

void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &wifi_mqtt_client_configuration, NULL, NULL);
  if (status != SL_STATUS_OK && status != SL_STATUS_ALREADY_INITIALIZED) {
    printf("Failed to start Wi-Fi client interface: 0x%lX\r\n", status);
    return;
  }
  printf("Start Wi-Fi client interface Success \r\n");

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("Failed to bring Wi-Fi client interface up: 0x%lX\r\n", status);
    return;
  }
  printf("Wi-Fi client connected\r\n");

  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile);
  if (status != SL_STATUS_OK) {
    printf("Failed to get client profile: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nSuccess to get client profile\r\n");

  ip_address.type = SL_IPV4;
  memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
  print_sl_ip_address(&ip_address);

  mqtt_example();

#ifdef SLI_SI91X_MCU_INTERFACE

#if (SL_SI91X_TICKLESS_MODE == 0)
  sl_si91x_power_manager_sleep();
#else
  osSemaphoreId_t wait_semaphore;
  wait_semaphore = osSemaphoreNew(1, 0, NULL);
  if (wait_semaphore == NULL) {
    printf("Failed to create semaphore\r\n");
    return;
  }
  // Waiting forever using semaphore to put M4 to sleep in tick less mode
  osSemaphoreAcquire(wait_semaphore, osWaitForever);
#endif

#endif
}

sl_status_t twt_callback_handler(sl_wifi_event_t event,
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

  switch (event) {
    case SL_WIFI_TWT_RESPONSE_EVENT:
      printf("\r\nTWT Setup success");
      break;
    case SL_WIFI_TWT_UNSOLICITED_SESSION_SUCCESS_EVENT:
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
      printf("\r\nTWT Setup Failed.");
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

void mqtt_client_cleanup()
{
  SL_CLEANUP_MALLOC(client_credentails);
  is_execution_completed = 1;
}

void mqtt_client_message_handler(void *client, sl_mqtt_client_message_t *message, void *context)
{
  UNUSED_PARAMETER(context);

  sl_status_t status;
  printf("Message Received on Topic: ");

  print_char_buffer((char *)message->topic, message->topic_length);
  print_char_buffer((char *)message->content, message->content_length);

  // Unsubscribing to already subscribed topic.
  status = sl_mqtt_client_unsubscribe(client,
                                      (uint8_t *)TOPIC_TO_BE_SUBSCRIBED,
                                      strlen(TOPIC_TO_BE_SUBSCRIBED),
                                      0,
                                      TOPIC_TO_BE_SUBSCRIBED);
  if (status != SL_STATUS_IN_PROGRESS) {
    printf("Failed to unsubscribe : 0x%lX\r\n", status);

    mqtt_client_cleanup();
    return;
  }
}

void print_char_buffer(char *buffer, uint32_t buffer_length)
{
  for (uint32_t index = 0; index < buffer_length; index++) {
    printf("%c", buffer[index]);
  }

  printf("\r\n");
}

void mqtt_client_error_event_handler(void *client, sl_mqtt_client_error_status_t *error)
{
  UNUSED_PARAMETER(client);

  printf("Terminating program, Error: %d\r\n", *error);
  mqtt_client_cleanup();
}

void mqtt_client_event_handler(void *client, sl_mqtt_client_event_t event, void *event_data, void *context)
{
#if !(ENABLE_MQTT_SUBSCRIBE_PUBLISH)
  UNUSED_PARAMETER(context);
#endif

  switch (event) {
    case SL_MQTT_CLIENT_CONNECTED_EVENT: {
      printf("MQTT client connection success\r\n");
#if ENABLE_MQTT_SUBSCRIBE_PUBLISH
      sl_status_t status;
      status = sl_mqtt_client_subscribe(client,
                                        (uint8_t *)TOPIC_TO_BE_SUBSCRIBED,
                                        strlen(TOPIC_TO_BE_SUBSCRIBED),
                                        QOS_OF_SUBSCRIPTION,
                                        0,
                                        mqtt_client_message_handler,
                                        TOPIC_TO_BE_SUBSCRIBED);
      if (status != SL_STATUS_IN_PROGRESS) {
        printf("Failed to subscribe : 0x%lX\r\n", status);

        mqtt_client_cleanup();
        return;
      }

      status = sl_mqtt_client_publish(client, &message_to_be_published, 0, &message_to_be_published);
      if (status != SL_STATUS_IN_PROGRESS) {
        printf("Failed to publish message: 0x%lX\r\n", status);

        mqtt_client_cleanup();
        return;
      }
#else
      mqtt_client_cleanup();
#endif
      break;
    }
#if ENABLE_MQTT_SUBSCRIBE_PUBLISH
    case SL_MQTT_CLIENT_MESSAGE_PUBLISHED_EVENT: {
      sl_mqtt_client_message_t *published_message = (sl_mqtt_client_message_t *)context;

      printf("Published message successfully on topic: ");
      print_char_buffer((char *)published_message->topic, published_message->topic_length);

      break;
    }

    case SL_MQTT_CLIENT_SUBSCRIBED_EVENT: {
      char *subscribed_topic = (char *)context;

      printf("Subscribed to Topic: %s\r\n", subscribed_topic);
      break;
    }

    case SL_MQTT_CLIENT_UNSUBSCRIBED_EVENT: {
      char *unsubscribed_topic = (char *)context;
      sl_status_t status;

      printf("Unsubscribed from topic: %s\r\n", unsubscribed_topic);

      status = sl_mqtt_client_disconnect(client, 0);
      if (status != SL_STATUS_IN_PROGRESS) {
        printf("Failed to disconnect : 0x%lX\r\n", status);

        mqtt_client_cleanup();
        return;
      }
      break;
    }
#endif

    case SL_MQTT_CLIENT_DISCONNECTED_EVENT: {
      printf("Disconnected from MQTT broker\r\n");

      mqtt_client_cleanup();
      break;
    }

    case SL_MQTT_CLIENT_ERROR_EVENT: {
      mqtt_client_error_event_handler(client, (sl_mqtt_client_error_status_t *)event_data);
      break;
    }

    default: {
      break;
    }
  }
}

sl_status_t mqtt_example()
{
  sl_status_t status;
  sl_wifi_performance_profile_v2_t performance_profile = { 0 };

  //! Enable Broadcast data filter
  status = sl_wifi_filter_broadcast(5000, 1, 1);
  VERIFY_STATUS_AND_RETURN(status);
  printf("\r\nEnabled Broadcast Data Filter\n");

  if (ENCRYPT_CONNECTION) {
    // Load SSL CA certificate
    status =
      sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0), SL_NET_SIGNING_CERTIFICATE, cacert, sizeof(cacert) - 1);
    if (status != SL_STATUS_OK) {
      printf("\r\nLoading TLS CA certificate in to FLASH Failed, Error Code : 0x%lX\r\n", status);
      return status;
    }
    printf("\r\nLoad TLS CA certificate at index %d Success\r\n", 0);
  }

  if (SEND_CREDENTIALS) {
    uint16_t username_length, password_length;

    username_length = strlen(USERNAME);
    password_length = strlen(PASSWORD);

    uint32_t malloc_size = sizeof(sl_mqtt_client_credentials_t) + username_length + password_length;

    client_credentails = malloc(malloc_size);
    if (client_credentails == NULL)
      return SL_STATUS_ALLOCATION_FAILED;
    memset(client_credentails, 0, malloc_size);
    client_credentails->username_length = username_length;
    client_credentails->password_length = password_length;

    memcpy(&client_credentails->data[0], USERNAME, username_length);
    memcpy(&client_credentails->data[username_length], PASSWORD, password_length);

    status = sl_net_set_credential(SL_NET_MQTT_CLIENT_CREDENTIAL_ID(0),
                                   SL_NET_MQTT_CLIENT_CREDENTIAL,
                                   client_credentails,
                                   malloc_size);

    if (status != SL_STATUS_OK) {
      mqtt_client_cleanup();
      printf("Failed to set credentials: 0x%lX\r\n ", status);

      return status;
    }

    mqtt_client_configuration.credential_id = SL_NET_MQTT_CLIENT_CREDENTIAL_ID(0);
  }

  status = sl_mqtt_client_init(&client, mqtt_client_event_handler);
  if (status != SL_STATUS_OK) {
    printf("Failed to init mqtt client: 0x%lX\r\n", status);

    mqtt_client_cleanup();
    return status;
  }
  printf("\r\nMQTT Client Init Done\r\n");

  status =
    sl_mqtt_client_connect(&client, &mqtt_broker_configuration, &last_will_message, &mqtt_client_configuration, 0);
  if (status != SL_STATUS_IN_PROGRESS) {
    printf("Failed to connect to mqtt broker: 0x%lX\r\n", status);

    mqtt_client_cleanup();
    return status;
  }

  while (!is_execution_completed) {
    osThreadYield();
  }

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

  //! Apply power save profile
  performance_profile.profile = ASSOCIATED_POWER_SAVE_LOW_LATENCY;
  status                      = sl_wifi_set_performance_profile_v2(&performance_profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nPowersave Configuration Failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nAssociated Power Save Enabled\n");

  printf("\r\nExample execution completed \r\n");

  return SL_STATUS_OK;
}
