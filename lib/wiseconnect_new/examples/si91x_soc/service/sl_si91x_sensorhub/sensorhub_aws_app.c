/***************************************************************************/ /**
 * @file sensorhub_aws_app.c
 * @brief Wi-Fi and AWS MQTT application code
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
/***************************************************************************
 * @brief : This file connects the SiWx917 to AWS MQTT cloud and  publishes the 
 * sensor data collected from the sensorhub task to the cloud. (NWP powersave is also supported)
 ******************************************************************************/

#include <string.h>
#include "cmsis_os2.h"
#include "sl_status.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_wifi_device.h"
#include "sl_net_wifi_types.h"
#include "sl_utility.h"
#include "sl_si91x_driver.h"

#include "sl_board_configuration.h"
#include "errno.h"
#include "socket.h"
#include "sl_net_si91x.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_socket.h"

#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_m4_ps.h"
#endif

//! Certificates to be loaded
#include "aws_client_certificate.pem.crt.h"
#include "aws_client_private_key.pem.key.h"
#include "aws_starfield_ca.pem.h"

//! AWS files
#include "aws_iot_error.h"
#include "aws_iot_config.h"
#include "aws_iot_shadow_interface.h"

/******************************************************
*                    Constants
******************************************************/

#define CERTIFICATE_INDEX 0

#define SUBSCRIBE_TO_TOPIC  "aws_status_555"     //! Subscribe Topic to receive the message from cloud.
#define PUBLISH_ON_TOPIC    "siwx91x_status_555" //! Publish Topic to send the status from application to cloud.
#define SUBSCRIBE_QOS       QOS1                 //! Quality of Service for subscribed topic "SUBSCRIBE_TO_TOPIC".
#define PUBLISH_QOS         QOS1                 //! Quality of Service for publish topic "PUBLISH_ON_TOPIC".
#define PUBLISH_PERIODICITY 1000                 //! Publish periodicity in milliseconds.
#define MQTT_USERNAME       "username"
#define MQTT_PASSWORD       "password"

#define ENABLE_NWP_POWER_SAVE 0

#if SH_AWS_ENABLE
extern char mqtt_publish_payload[500];
#else
#define MQTT_PUBLISH_PAYLOAD \
  "Hi from SiWx91x" //! Publish message. Increase the AWS_IOT_MQTT_TX_BUF_LEN value in aws_iot_config.h for larger payloads.
#endif

#if ENABLE_NWP_POWER_SAVE
volatile uint8_t powersave_given = 0;
#endif

/******************************************************
*               Function Declarations
******************************************************/
void sl_si91x_aws_task(void);
sl_status_t load_certificates_in_flash(void);
sl_status_t start_aws_mqtt(void);
void subscribe_handler(struct _Client *pClient,
                       char *pTopicName,
                       short unsigned int topicNameLen,
                       IoT_Publish_Message_Params *pParams,
                       void *pClientData);
void disconnect_notify_handler(AWS_IoT_Client *pClient, void *data);

//! Enumeration for states in application
typedef enum app_state {
  WLAN_INITIAL_STATE = 0,
  WLAN_SCAN_STATE,
  WLAN_UNCONNECTED_STATE,
  WLAN_CONNECTED_STATE,
  WLAN_DISCONNECT,
  AWS_MQTT_INIT_STATE,
  AWS_MQTT_CONNECT_STATE,
  AWS_MQTT_SUBSCRIBE_STATE,
  AWS_MQTT_PUBLISH_STATE,
  AWS_MQTT_RECEIVE_STATE,
  AWS_MQTT_DISCONNECT,
  AWS_SELECT_CONNECT_STATE,
  IDLE_STATE
} app_state_t;

volatile app_state_t application_state;

#if SH_AWS_ENABLE
extern osSemaphoreId_t sl_semaphore_app_task_id_2;
osSemaphoreId_t sl_semaphore_aws_task_id;
osSemaphoreAttr_t sl_aws_semaphore_attr_st;
#endif

/******************************************************
*               Variable Definitions
******************************************************/

IoT_Publish_Message_Params publish_iot_msg = { 0 };

fd_set read_fds;
AWS_IoT_Client mqtt_client = { 0 };
#define RSI_FD_ISSET(x, y) rsi_fd_isset(x, y)
volatile uint8_t check_for_recv_data;
extern osSemaphoreId_t select_sem;
extern volatile uint8_t pub_state, qos1_publish_handle, select_given;
int32_t sh_aws_status = SL_STATUS_OK;

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

//! No of ltcp socktes
#define RSI_NUMBER_OF_LTCP_SOCKETS 0

//! Default number of sockets supported,max 10 sockets are supported
#define RSI_NUMBER_OF_SOCKETS (6 + RSI_NUMBER_OF_LTCP_SOCKETS)

//! Default number of sockets supported,max 10 selects are supported
#define RSI_NUMBER_OF_SELECTS (RSI_NUMBER_OF_SOCKETS)

static const sl_wifi_device_configuration_t client_init_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
#ifdef SLI_SI91X_MCU_INTERFACE
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_WPS_DISABLE | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE),
#else
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION
#if ENABLE_NWP_POWER_SAVE
                      | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
#endif
                      ),
#endif

                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT | SL_SI91X_TCP_IP_FEAT_SSL
#ifdef SLI_SI91X_ENABLE_IPV6
                      | SL_SI91X_TCP_IP_FEAT_DHCPV6_CLIENT | SL_SI91X_TCP_IP_FEAT_IPV6
#endif
                      | SL_SI91X_TCP_IP_FEAT_ICMP | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0 | SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE
                      | SL_SI91X_RAM_LEVEL_NWP_MEDIUM_MCU_MEDIUM
#if defined(SLI_SI917)
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_WINDOW_SCALING | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(1)
                      | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
#ifdef SLI_SI91X_MCU_INTERFACE
                   .config_feature_bit_map = 0
#else
#if ENABLE_NWP_POWER_SAVE
                   .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP)
#else
                   .config_feature_bit_map = 0
#endif
#endif
  }
};
/******************************************************
*               Function Definitions
******************************************************/

void async_socket_select(fd_set *fd_read, fd_set *fd_write, fd_set *fd_except, int32_t status)
{
  UNUSED_PARAMETER(fd_except);
  UNUSED_PARAMETER(fd_write);
  UNUSED_PARAMETER(status);

  //!Check the data pending on this particular socket descriptor
  if (FD_ISSET(mqtt_client.networkStack.socket_id, fd_read)) {
    if (pub_state != 1) { //This check is for handling PUBACK in QOS1
      check_for_recv_data = 1;
      application_state   = AWS_SELECT_CONNECT_STATE;
    } else if (pub_state == 1) { //This check is for handling PUBACK in QOS1
      osSemaphoreRelease(select_sem);
    }
  }
  application_state = AWS_SELECT_CONNECT_STATE;
}

void disconnect_notify_handler(AWS_IoT_Client *pClient, void *data)
{
  UNUSED_PARAMETER(pClient);
  UNUSED_PARAMETER(data);
  printf("\r\nMQTT disconnected abruptly and pClient state is: %d\r\n", pClient->clientStatus.clientState);
}

void subscribe_handler(struct _Client *pClient,
                       char *pTopicName,
                       short unsigned int topicNameLen,
                       IoT_Publish_Message_Params *pParams,
                       void *data)
{
  UNUSED_PARAMETER(pClient);
  UNUSED_PARAMETER(pTopicName);
  UNUSED_PARAMETER(topicNameLen);
  UNUSED_PARAMETER(data);
  printf("\rData received on the Subscribed Topic: %.*s \r\n", pParams->payloadLen, (char *)pParams->payload);
}
void sl_si91x_aws_task(void)
{
  sl_net_wifi_client_profile_t profile = { 0 };
  sl_ip_address_t ip_address           = { 0 };
#if SH_AWS_ENABLE
  sl_aws_semaphore_attr_st.attr_bits = 0U;
  sl_aws_semaphore_attr_st.cb_mem    = NULL;
  sl_aws_semaphore_attr_st.cb_size   = 0U;
  sl_aws_semaphore_attr_st.name      = NULL;
#endif

  select_sem = osSemaphoreNew(1, 0, NULL);
#if SH_AWS_ENABLE
  sl_semaphore_aws_task_id = osSemaphoreNew(1U, 0U, &sl_aws_semaphore_attr_st);
#endif
  printf("\r\nWi-Fi Begin\r\n");
  sl_status_t status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &client_init_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error while initializing Wi-Fi: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi is Initialized\r\n");

#ifdef SLI_SI91X_MCU_INTERFACE
  uint8_t xtal_enable = 1;
  status              = sl_si91x_m4_ta_secure_handshake(SL_SI91X_ENABLE_XTAL, 1, &xtal_enable, 0, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring m4_ta_secure_handshake: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nM4-NWP secure handshake is successful\r\n");
#endif

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("\r\nError while connecting to Access point: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nConnected to Access point\r\n");

  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to get client profile: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nGetting client profile is successful\r\n");

  ip_address.type = SL_IPV4;
  memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
  printf("\r\nIP address is ");
  print_sl_ip_address(&ip_address);
  printf("\r\n");

  sh_aws_status = load_certificates_in_flash();
  if (sh_aws_status != SL_STATUS_OK) {
    printf("\r\Error while loading certificates: 0x%lx\r\n", sh_aws_status);
    return;
  }
  printf("\r\nLoaded certificates\r\n");

  sh_aws_status = start_aws_mqtt();
  if (sh_aws_status != SL_STATUS_OK) {
    printf("\r\nUnexpected error occurred in AWS connection: 0x%lx\r\n", sh_aws_status);
    return;
  }
}

sl_status_t load_certificates_in_flash(void)
{
  sl_status_t status;

  // Load SSL CA certificate
  status = sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(CERTIFICATE_INDEX),
                                 SL_NET_SIGNING_CERTIFICATE,
                                 aws_starfield_ca,
                                 sizeof(aws_starfield_ca) - 1);
  if (status != SL_STATUS_OK) {
    printf("\r\nLoading TLS CA certificate in to FLASH Failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nLoading TLS CA certificate at index %d Successful\r\n", CERTIFICATE_INDEX);

  // Load SSL Client certificate
  status = sl_net_set_credential(SL_NET_TLS_CLIENT_CREDENTIAL_ID(CERTIFICATE_INDEX),
                                 SL_NET_CERTIFICATE,
                                 aws_client_certificate,
                                 sizeof(aws_client_certificate) - 1);
  if (status != SL_STATUS_OK) {
    printf("\r\nLoading TLS certificate in to FLASH failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nLoading TLS Client certificate at index %d Successful\r\n", CERTIFICATE_INDEX);

  // Load SSL Client private key
  status = sl_net_set_credential(SL_NET_TLS_CLIENT_CREDENTIAL_ID(CERTIFICATE_INDEX),
                                 SL_NET_PRIVATE_KEY,
                                 aws_client_private_key,
                                 sizeof(aws_client_private_key) - 1);
  if (status != SL_STATUS_OK) {
    printf("\r\nLoading TLS Client private key in to FLASH Failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nLoading TLS Client private key at index %d Successful\r\n", CERTIFICATE_INDEX);

  return SL_STATUS_OK;
}

sl_status_t start_aws_mqtt(void)
{
  IoT_Error_t rc = FAILURE;

  IoT_Client_Init_Params mqtt_init_params       = iotClientInitParamsDefault;
  IoT_Client_Connect_Params mqtt_connect_params = iotClientConnectParamsDefault;
#if SH_AWS_ENABLE
  osStatus_t sl_semawstaskacq_status;
#endif

  sl_mac_address_t mac_addr = { 0 };
  char mac_id[18];
  sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_addr);
  sprintf(mac_id,
          "%x:%x:%x:%x:%x:%x",
          mac_addr.octet[0],
          mac_addr.octet[1],
          mac_addr.octet[2],
          mac_addr.octet[3],
          mac_addr.octet[4],
          mac_addr.octet[5]);
  printf("\r\n client_id:%s \r\n", mac_id);

  sl_wifi_firmware_version_t fw_version = { 0 };
  sh_aws_status                         = sl_wifi_get_firmware_version(&fw_version);
  if (sh_aws_status != SL_STATUS_OK) {
    printf("\r\nFirmware version Failed, Error Code : 0x%lX\r\n", sh_aws_status);
  } else {
    print_firmware_version(&fw_version);
  }

  mqtt_init_params.enableAutoReconnect       = true;
  mqtt_init_params.pHostURL                  = AWS_IOT_MQTT_HOST;
  mqtt_init_params.port                      = AWS_IOT_MQTT_PORT;
  mqtt_init_params.pRootCALocation           = (char *)aws_starfield_ca;
  mqtt_init_params.pDeviceCertLocation       = (char *)aws_client_certificate;
  mqtt_init_params.pDevicePrivateKeyLocation = (char *)aws_client_private_key;
  mqtt_init_params.mqttCommandTimeout_ms     = 20000;
  mqtt_init_params.tlsHandshakeTimeout_ms    = 5000;
  mqtt_init_params.isSSLHostnameVerify       = true;
  mqtt_init_params.disconnectHandler         = disconnect_notify_handler;
  mqtt_init_params.disconnectHandlerData     = NULL;

  mqtt_connect_params.keepAliveIntervalInSec = 1200;
  mqtt_connect_params.isCleanSession         = true;
  mqtt_connect_params.MQTTVersion            = MQTT_3_1_1;
  mqtt_connect_params.pClientID              = AWS_IOT_MQTT_CLIENT_ID;
  mqtt_connect_params.clientIDLen            = (uint16_t)strlen(AWS_IOT_MQTT_CLIENT_ID);
  mqtt_connect_params.isWillMsgPresent       = false;
  mqtt_connect_params.pUsername              = MQTT_USERNAME;
  mqtt_connect_params.usernameLen            = strlen(MQTT_USERNAME);
  mqtt_connect_params.pPassword              = MQTT_PASSWORD;
  mqtt_connect_params.passwordLen            = strlen(MQTT_PASSWORD);

  application_state = AWS_MQTT_INIT_STATE;

  while (1) {

    switch (application_state) {

      case AWS_MQTT_INIT_STATE: {
        rc = aws_iot_mqtt_init(&mqtt_client, &mqtt_init_params);
        if (SUCCESS != rc) {
          application_state = AWS_MQTT_INIT_STATE;
          printf("\r\nMQTT Initialization failed with error: 0x%x\r\n", rc);
        } else {
          application_state = AWS_MQTT_CONNECT_STATE;
          printf("\r\nMQTT Initialization success\r\n");
        }

      } break;
      case AWS_MQTT_CONNECT_STATE: {
        rc = aws_iot_mqtt_connect(&mqtt_client, &mqtt_connect_params);
        if (SUCCESS != rc) {
          if (rc == NETWORK_ALREADY_CONNECTED_ERROR) {
            printf("\r\nNetwork is already connected\r\n");

          } else {
            printf("\r\nMQTT connect failed with error: 0x%x\r\n", rc);
            application_state = AWS_MQTT_DISCONNECT;
          }
        } else {
          printf("\r\nConnected to AWS IoT Cloud\n");
          application_state = AWS_MQTT_SUBSCRIBE_STATE;
        }

      } break;
      case AWS_MQTT_SUBSCRIBE_STATE: {
        rc = aws_iot_mqtt_subscribe(&mqtt_client,
                                    SUBSCRIBE_TO_TOPIC,
                                    strlen(SUBSCRIBE_TO_TOPIC),
                                    SUBSCRIBE_QOS,
                                    subscribe_handler,
                                    NULL);

        if (SUCCESS != rc) {
          if (NETWORK_DISCONNECTED_ERROR == rc) {
            printf("\r\nSubscription failed with error: 0x%x\r\n", rc);
            application_state = AWS_MQTT_CONNECT_STATE;
          } else if (NETWORK_ATTEMPTING_RECONNECT == rc) {
            // If the client is attempting to reconnect skip the rest of the loop
            continue;
          }
          application_state = AWS_MQTT_SUBSCRIBE_STATE;
        } else {
          printf("\rSubscribed to the specified topic with QoS%d\n", SUBSCRIBE_QOS);
          application_state = AWS_SELECT_CONNECT_STATE;
        }

      } break;

      case AWS_SELECT_CONNECT_STATE: {
        {
#if SH_AWS_ENABLE
          sl_semawstaskacq_status = osSemaphoreRelease(sl_semaphore_app_task_id_2);
          if (sl_semawstaskacq_status != osOK) {
            DEBUGOUT("\r\n  osSemaphoreRelease failed :%d \r\n", sl_semawstaskacq_status);
          }
          sl_semawstaskacq_status = osSemaphoreAcquire(sl_semaphore_aws_task_id, osWaitForever);
          if (sl_semawstaskacq_status != osOK) {
            DEBUGOUT("\r\n osSemaphoreAcquire failed :%d \r\n", sl_semawstaskacq_status);
          }
#endif
          if (!select_given) {
            select_given = 1;
            memset(&read_fds, 0, sizeof(fd_set));

            FD_SET(mqtt_client.networkStack.socket_id, &read_fds);
            printf("\rSocket ID: %d\n", mqtt_client.networkStack.socket_id);

            sh_aws_status =
              sl_si91x_select(mqtt_client.networkStack.socket_id + 1, &read_fds, NULL, NULL, NULL, async_socket_select);

            printf("\rSelect status: 0x%lX\r\n", sh_aws_status);
          }

          // waiting for the semaphore release

          if (check_for_recv_data) {
            check_for_recv_data = 0;
            select_given        = 0;
            application_state   = AWS_MQTT_RECEIVE_STATE;
          } else {
            application_state = AWS_MQTT_PUBLISH_STATE;
          }
        }

      } break;

      case AWS_MQTT_RECEIVE_STATE: {
        rc = aws_iot_shadow_yield(&mqtt_client, 1);
        if (SUCCESS == rc) {
          printf("\rYield is successful\n");
        }
        application_state = AWS_SELECT_CONNECT_STATE;

      } break;

      case AWS_MQTT_PUBLISH_STATE: {
#if SH_AWS_ENABLE
        printf("\rData to be published: %s\n", mqtt_publish_payload);

        publish_iot_msg.qos        = PUBLISH_QOS;
        publish_iot_msg.payload    = mqtt_publish_payload;
        publish_iot_msg.isRetained = 0;
        publish_iot_msg.payloadLen = strlen(mqtt_publish_payload);
#else
        printf("\rData to be published: %s\n", MQTT_PUBLISH_PAYLOAD);

        publish_iot_msg.qos        = PUBLISH_QOS;
        publish_iot_msg.payload    = MQTT_PUBLISH_PAYLOAD;
        publish_iot_msg.isRetained = 0;
        publish_iot_msg.payloadLen = sizeof(MQTT_PUBLISH_PAYLOAD);
#endif
        if (SUBSCRIBE_QOS == QOS1 || PUBLISH_QOS == QOS1) {
          pub_state = 1;
        }
        rc = aws_iot_mqtt_publish(&mqtt_client, PUBLISH_ON_TOPIC, strlen(PUBLISH_ON_TOPIC), &publish_iot_msg);

        if (rc != SUCCESS) {
          printf("\r\nMQTT Publish with QoS%d failed with error: 0x%x\n", PUBLISH_QOS, rc);
          application_state = AWS_MQTT_DISCONNECT;
          break;
        }

        else if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
          printf("\r\nACK not received for QoS%d publish\r\n", PUBLISH_QOS);
        } else {
          printf("\rQoS%d publish is successful\r\n", PUBLISH_QOS);
        }

#if ENABLE_NWP_POWER_SAVE
        sl_wifi_performance_profile_v2_t performance_profile = { .profile         = ASSOCIATED_POWER_SAVE,
                                                                 .listen_interval = 1000 };
        if (!powersave_given) {
          rc = sl_wifi_set_performance_profile_v2(&performance_profile);
          if (rc != SL_STATUS_OK) {
            printf("\r\nPower save configuration Failed, Error Code : 0x%X\r\n", rc);
          }
          printf("\r\nAssociated Power Save is enabled\r\n");
          powersave_given = 1;
        }
        application_state = AWS_SELECT_CONNECT_STATE;
#else
        application_state          = IDLE_STATE;
#endif
      } break;

      case AWS_MQTT_DISCONNECT: {
        rc = aws_iot_mqtt_disconnect(&mqtt_client);
        if (SUCCESS != rc) {
          printf("\r\nMQTT disconnection error\r\n");
          application_state = AWS_MQTT_INIT_STATE;
        }
        application_state = AWS_MQTT_INIT_STATE;

      } break;
      case IDLE_STATE: {

        application_state = AWS_SELECT_CONNECT_STATE;

      } break;

#if 0
        case RSI_WLAN_DISCONNECT: {
          rc = sl_net_down();
          if (SUCCESS != rc) {
            printf("\r\nWLAN disconnection error\r\n");
            application_state = WLAN_DISCONNECT;
          }
          sl_si91x_host_delay_ms(100);
          application_state = WLAN_INITIAL_STATE;
        }
          break;
#endif
      default:
        break;
    }
  }

  return rc;
}
