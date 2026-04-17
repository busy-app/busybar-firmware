/***************************************************************************/ /**
* @file
* @brief  MQTT OVER TCP Application
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
#include "MQTTClient.h"
#include "cmsis_os2.h"
#include "sl_wifi.h"
#include "errno.h"
#include "socket.h"
#include "sl_net_si91x.h"
#include "sl_net.h"
#include "sl_si91x_driver.h"
#include "sl_utility.h"
#include "sl_constants.h"
#include "sl_net_wifi_types.h"
#include <string.h>
#include "cacert.pem.h"

/******************************************************
 *                    Constants
 ******************************************************/
#ifdef SLI_SI91X_ENABLE_IPV6
#define MQTT_BROKER_IP "2409:40f2:102b:8ce2:d924:d6b2:ba7d:f9f8"
#else
#define MQTT_BROKER_IP "192.168.0.190"
#endif

//MQTT Broker port
#define MQTT_BROKER_PORT 1883 //if enable ssl use 8883
//! Client port number
#define CLIENT_PORT 5001

#define USERNAME "username"
#define PASSWORD "password"

#define QOS                    0
#define TOPIC_TO_BE_SUBSCRIBED "THERMOSTAT-DATA\0"

#define TCP_MQTT_CLIENT_INIT_BUFF_LEN 3500
// Rx buffer size
#define TCP_MQTT_CLIENT_RX_BUFFER_SIZE 1500
// Tx buffer size
#define TCP_MQTT_CLIENT_TX_BUFFER_SIZE 1500
#define TCP_MQTT_CONNECT_TIME_OUT      20000
#define KEEP_ALIVE_PERIOD              100
#define MQTT_VERSION                   4
volatile int halt = 0;

bool enable_ssl           = false; // Set to true to enable ssl
int8_t clientID[]         = "WISECONNECT_SDK_TOPIC";
uint8_t publish_message[] = "THIS IS MQTT CLIENT DEMO FROM APPLICATION";

int8_t tcp_mqtt_client_buffer[TCP_MQTT_CLIENT_INIT_BUFF_LEN];

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
  .priority   = osPriorityNormal,
  .tz_module  = 0,
  .reserved   = 0,
};

static const sl_wifi_device_configuration_t wifi_mqtt_client_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .boot_config = { .oper_mode              = SL_SI91X_CLIENT_MODE,
                   .coex_mode              = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map        = (SL_WIFI_FEAT_SECURITY_PSK | SL_WIFI_FEAT_AGGREGATION),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_SSL | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID
#ifdef SLI_SI91X_ENABLE_IPV6
                                              | SL_SI91X_TCP_IP_FEAT_DHCPV6_CLIENT | SL_SI91X_TCP_IP_FEAT_IPV6
#endif
                                              ),
                   .custom_feature_bit_map     = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map = (SL_SI91X_EXT_FEAT_SSL_VERSIONS_SUPPORT | SL_SI91X_EXT_FEAT_XTAL_CLK
                                                  | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
#ifdef SLI_SI917
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_WINDOW_SCALING | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(10)
                      | SL_SI91X_EXT_TCP_IP_FEAT_SSL_THREE_SOCKETS | SL_SI91X_EXT_TCP_IP_FEAT_SSL_MEMORY_CLOUD),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = 0 }
};

typedef struct mqtt_client_s {
  // MQTT Client structure
  Client client;
  // MQTT broker port
  uint32_t server_port;
  // MQTT client port
  uint32_t client_port;
  // Server IP address
  sl_ip_address_t server_ip;
  // Keep alive interval
  uint16_t keep_alive_interval;
  // MQTT client TX buffer
  int8_t *tcp_mqtt_tx_buffer;
  // MQTT client RX buffer
  int8_t *tcp_mqtt_rx_buffer;

} mqtt_client_t;

/******************************************************
 *               Function Declarations
******************************************************/
static void application_start(void *argument);
int paho_mqtt_demo();

/******************************************************
 *               Function Definitions
 ******************************************************/
void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}
//! Call back when a message is received on the published topic
void message_arrived(MessageData *md)
{
  if (md == NULL || md->message == NULL) {
    printf("\nReceived NULL message!\n");
    return;
  }
  printf("\r\nMessage: %.*s\r\n", md->message->payloadlen, (char *)md->message->payload);
  //! process the received data
  halt = 1;
  return;
}
static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status                   = 0;
  sl_ip_address_t ip_address           = { 0 };
  sl_net_wifi_client_profile_t profile = { 0 };

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &wifi_mqtt_client_configuration, NULL, NULL);
  if (status != SL_STATUS_OK && status != SL_STATUS_ALREADY_INITIALIZED) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client interface up Success\r\n");
  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring Wi-Fi client interface up: 0x%lX\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client connected\r\n");

  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile);
  if (status != SL_STATUS_OK) {
    printf("Failed to get client profile: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nSuccess to get client profile\r\n");

  // Load SSL CA certificate
  if (enable_ssl) {
    status =
      sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0), SL_NET_SIGNING_CERTIFICATE, cacert, sizeof(cacert) - 1);
    if (status != SL_STATUS_OK) {
      printf("\r\nLoading TLS CA certificate in to FLASH Failed, Error Code : 0x%lX\r\n", status);
      return;
    }
    printf("\r\nLoad SSL CA certificate at index %d Success\r\n", 0);
  }

  if (profile.ip.type == SL_IPV4) {
    ip_address.type = SL_IPV4;
    memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
    printf("\r\nIP address is ");
    print_sl_ip_address(&ip_address);
  } else if (profile.ip.type == SL_IPV6) {
    sl_ip_address_t link_local_address = { 0 };
    memcpy(&link_local_address.ip.v6, &profile.ip.ip.v6.link_local_address, SL_IPV6_ADDRESS_LENGTH);
    link_local_address.type = SL_IPV6;
    printf("Link Local Address: ");
    print_sl_ip_address(&link_local_address);

    sl_ip_address_t global_address = { 0 };
    memcpy(&global_address.ip.v6, &profile.ip.ip.v6.global_address, SL_IPV6_ADDRESS_LENGTH);
    global_address.type = SL_IPV6;
    printf("Global Address: ");
    print_sl_ip_address(&global_address);

    sl_ip_address_t gateway = { 0 };
    memcpy(&gateway.ip.v6, &profile.ip.ip.v6.gateway, SL_IPV6_ADDRESS_LENGTH);
    gateway.type = SL_IPV6;
    printf("Gateway Address: ");
    print_sl_ip_address(&gateway);
  }

  int result = paho_mqtt_demo();
  if (result == 0) {
    printf("Paho MQTT over TCP demo executed successfully.\n");
  } else {
    printf("Paho MQTT over TCP demo failed with error code: %d\n", result);
  }
}

int paho_mqtt_demo()
{
  int status;
  uint16_t flags             = 0;
  mqtt_client_t *mqtt_client = NULL;
  MQTTMessage publish_msg;
  sl_ip_address_t server_address = { 0 };
#ifdef SLI_SI91X_ENABLE_IPV6
  unsigned char hex_addr[SL_IPV6_ADDRESS_LENGTH] = { 0 };
  sl_inet_pton6((char *)MQTT_BROKER_IP,
                (char *)MQTT_BROKER_IP + strlen((char *)MQTT_BROKER_IP),
                hex_addr,
                (unsigned int *)&server_address);
#else
  sl_net_inet_addr((char *)MQTT_BROKER_IP, (uint32_t *)&server_address);
#endif
  int8_t *server_ip  = (int8_t *)&server_address;
  int8_t *buffer_ptr = tcp_mqtt_client_buffer;
  mqtt_client        = (mqtt_client_t *)buffer_ptr;
  buffer_ptr += sizeof(mqtt_client_t);
  mqtt_client->client.ipstack = (Network *)buffer_ptr;
  buffer_ptr += sizeof(Network);
  mqtt_client->server_port         = MQTT_BROKER_PORT;
  mqtt_client->client_port         = CLIENT_PORT;
  mqtt_client->keep_alive_interval = KEEP_ALIVE_PERIOD;
#ifdef SLI_SI91X_ENABLE_IPV6
  memcpy(&mqtt_client->server_ip.ip.v6, server_ip, 16);
#else
  memcpy(&mqtt_client->server_ip.ip.v4, server_ip, 4);
#endif
  mqtt_client->tcp_mqtt_tx_buffer = buffer_ptr;
  buffer_ptr += TCP_MQTT_CLIENT_TX_BUFFER_SIZE;
  mqtt_client->tcp_mqtt_rx_buffer = buffer_ptr;
  buffer_ptr += TCP_MQTT_CLIENT_RX_BUFFER_SIZE;
  mqtt_client->client.ipstack->transport_type = MQTT_TRANSPORT_TCP;

  NetworkInit(mqtt_client->client.ipstack);

  MQTTClient((Client *)mqtt_client,
             mqtt_client->client.ipstack,
             TCP_MQTT_CONNECT_TIME_OUT,
             (uint8_t *)mqtt_client->tcp_mqtt_tx_buffer,
             TCP_MQTT_CLIENT_TX_BUFFER_SIZE,
             (uint8_t *)mqtt_client->tcp_mqtt_rx_buffer,
             TCP_MQTT_CLIENT_RX_BUFFER_SIZE);

  status = NetworkConnect(mqtt_client->client.ipstack,
                          flags,
                          (char *)&(mqtt_client->server_ip),
                          mqtt_client->server_port,
                          mqtt_client->client_port,
                          enable_ssl);

  if (status == NETWORK_ERROR_NULL_STRUCTURE) {
    printf("\r\nError: Network structure is NULL.\r\n");
    return status;
  } else if (status == NETWORK_ERROR_NULL_ADDRESS) {
    printf("\r\nError: Address is NULL.\r\n");
    return status;
  } else if (status == NETWORK_ERROR_INVALID_TYPE) {
    printf("\r\nError: Invalid transport type.\r\n");
    return status;
  } else if (status != 0) {
    printf("\r\n TCP Connection Failed: %d\r\n", status);
    return status;
  }

  MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
  connectData.willFlag               = 0;
  // MQTT Version
  connectData.MQTTVersion = MQTT_VERSION;
  // Assign client ID
  connectData.clientID.cstring = (char *)clientID;
  // Fill username
  connectData.username.cstring = USERNAME;
  // Fill password
  connectData.password.cstring = PASSWORD;
  // Keep Alive interval
  connectData.keepAliveInterval = mqtt_client->keep_alive_interval;
  // New connection
  connectData.cleansession = 1;
  // Connect to MQTT broker
  status = MQTTConnect(&mqtt_client->client, &connectData);
  if (status != 0) {
    printf("\r\nMQTT Connection Failed: %d\r\n", status);
    return status;
  }
  printf("\r\nMQTT Connected Successfully!\r\n");
  // Subscribe to the topic given
  status = MQTTSubscribe(&mqtt_client->client, (char *)TOPIC_TO_BE_SUBSCRIBED, (enum QoS)QOS, message_arrived);
  if (status != 0) {
    printf("\r\nSubscription Failed: %d\r\n", status);
    return status;
  }
  printf("\r\nSubscribed to topic: %s\r\n", TOPIC_TO_BE_SUBSCRIBED);

  publish_msg.dup = 0;
  if (QOS == QOS0) {
    publish_msg.qos = QOS0;
  } else if (QOS == QOS1) {
    publish_msg.qos = QOS1;
  } else {
    publish_msg.qos = QOS2;
  }
  publish_msg.retained   = 0;
  publish_msg.payload    = publish_message;
  publish_msg.payloadlen = strlen((char *)publish_message);

  // Publish message on the topic
  status = MQTTPublish(&mqtt_client->client, (const char *)TOPIC_TO_BE_SUBSCRIBED, &publish_msg);
  if (status != 0) {
    printf("\r\nMQTT Publish Failed: %d\r\n", status);
    return status;
  } else {
    printf("\r\nPublishes to Topic successfully\r\n");
  }

  while (!halt) {
    // Recv data published on the subscribed topic
    status = MQTTYield(&mqtt_client->client, 60000);
    if (status != SL_STATUS_OK) {
      //! Error in receiving
      printf("\r\nReceive Data Failed, Error Code : 0x%X\r\n", status);
      return status;
    } else {
      printf("\r\nReceive Data Success\r\n");
    }
  }

  // UnSubscribe to the topic given
  status = MQTTUnsubscribe(&mqtt_client->client, (const char *)TOPIC_TO_BE_SUBSCRIBED);
  if (status != SL_STATUS_OK) {
    printf("\r\nUnsubscription to Topic Failed, Error Code : 0x%X\r\n", status);
    return status;
  } else {
    printf("\r\nUnsubscription to Topic Success\r\n");
  }

  // Disconnect to the MQTT broker
  status = MQTTDisconnect(&mqtt_client->client);
  if (status != SL_STATUS_OK) {
    printf("\r\nDisconnect to the MQTT broker Failed, Error Code : 0x%X\r\n", status);
    return status;
  } else {
    printf("\r\nDisconnect to the MQTT broker Success\r\n");
  }

  if (mqtt_client->client.ipstack) {
    NetworkDisconnect(mqtt_client->client.ipstack);
  }

  printf("\r\nExecution completed!\r\n");

  return 0;
}
