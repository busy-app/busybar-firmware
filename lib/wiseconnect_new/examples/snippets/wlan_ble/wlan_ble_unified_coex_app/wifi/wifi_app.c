/*******************************************************************************
* @file  wifi_app.c
* @brief
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

/*=======================================================================*/
//  ! INCLUDES
/*=======================================================================*/
#include "rsi_common_app.h"
#if WLAN_TASK_ENABLE
#if (WIFI_APP == MQTT_APP)
#include "stdlib.h"
#include "wifi_app_config.h"

//! SL Wi-Fi SDK includes
#include "sl_constants.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "sl_net.h"
#include "sl_net_si91x.h"
#include "sl_utility.h"
#include "sl_si91x_driver.h"

#include "errno.h"
#include "socket.h"

#include "cmsis_os2.h"
#include <rsi_common_apis.h>
#include <string.h>

#include "sl_si91x_socket.h"

//! Certificates to be loaded
#include "aws_client_certificate.pem.crt.h"
#include "aws_client_private_key.pem.key.h"
#include "aws_starfield_ca.pem.h"

//! AWS files
#include "aws_iot_error.h"
#include "aws_iot_config.h"
#include "aws_iot_shadow_interface.h"
/*=======================================================================*/
//   ! MACROS
/*=======================================================================*/
#define DHCP_HOST_NAME    NULL
#define TIMEOUT_MS        10000
#define WIFI_SCAN_TIMEOUT 10000

/*=======================================================================*/
//   ! GLOBAL VARIABLES
/*=======================================================================*/
wifi_app_state_t wlan_app_cb; //! application control block

static volatile bool scan_complete          = false;
static volatile sl_status_t callback_status = SL_STATUS_OK;
sl_wifi_client_configuration_t access_point = { 0 };
sl_net_ip_configuration_t ip_address        = { 0 };

IoT_Publish_Message_Params publish_iot_msg = { 0 };

fd_set read_fds;

osSemaphoreId_t data_received_semaphore;

AWS_IoT_Client mqtt_client = { 0 };
#define RSI_FD_ISSET(x, y) rsi_fd_isset(x, y)
volatile uint8_t check_for_recv_data;
extern osSemaphoreId_t select_sem;
extern volatile uint8_t pub_state, qos1_publish_handle, select_given;
/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/

#if ENABLE_NWP_POWER_SAVE
volatile uint8_t powersave_given = 0;
extern osMutexId_t power_cmd_mutex;
#endif

extern osThreadId_t wifi_app_thread_id;
/*=======================================================================*/
//   ! EXTERN FUNCTIONS
/*=======================================================================*/

/*=======================================================================*/
//   ! CONSTANTS
/*=======================================================================*/
/******************************************************
 *               Function Declarations
 ******************************************************/
void wifi_app_task(void *unused);
sl_status_t load_certificates_in_flash(void);
sl_status_t start_aws_mqtt(void);
void subscribe_handler(struct _Client *pClient,
                       char *pTopicName,
                       short unsigned int topicNameLen,
                       IoT_Publish_Message_Params *pParams,
                       void *pClientData);
void disconnect_notify_handler(AWS_IoT_Client *pClient, void *data);
void async_socket_select(fd_set *fd_read, fd_set *fd_write, fd_set *fd_except, int32_t status);
/*************************************************************************/
//!  CALLBACK FUNCTIONS
/*************************************************************************/

sl_status_t join_callback_handler(sl_wifi_event_t event,
                                  sl_status_t status_code,
                                  char *result,
                                  uint32_t result_length,
                                  void *arg)
{
  UNUSED_PARAMETER(result);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    printf("F: Join Event received with %lu bytes payload\n", result_length);
    wlan_app_cb = WIFI_APP_UNCONNECTED_STATE;
    if (osThreadGetState(wifi_app_thread_id) == osThreadBlocked) {
      osThreadResume(wifi_app_thread_id);
    }
    return status_code;
  }

  return SL_STATUS_OK;
}

/*====================================================*/
/**
 * @fn          void wlan_app_callbacks_init(void)
 * @brief       To initialize WLAN application callback
 * @param[in]   void
 * @return      void
 * @section description
 * This callback is used to initialize WLAN
 * ==================================================*/
void wlan_app_callbacks_init(void)
{
  sl_wifi_set_join_callback_v2(join_callback_handler, NULL);
}

static sl_status_t show_scan_results(sl_wifi_scan_result_t *scan_result)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(scan_result);
  uint8_t *bssid = NULL;
  printf("%lu Scan results:\n", scan_result->scan_count);

  if (scan_result->scan_count) {
    printf("\n   %s %24s %s", "SSID", "SECURITY", "NETWORK");
    printf("%12s %12s %s\n", "BSSID", "CHANNEL", "RSSI");

    for (int a = 0; a < (int)scan_result->scan_count; ++a) {
      bssid = (uint8_t *)&scan_result->scan_info[a].bssid;
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
  UNUSED_PARAMETER(arg);
  UNUSED_PARAMETER(result_length);

  scan_complete = true;

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    callback_status = status_code;
    return status_code;
  }

  if (result_length != 0) {
    callback_status = show_scan_results(result);
  }

  return SL_STATUS_OK;
}

/*====================================================*/
/**
 * @fn         int32_t  wifi_app_task(void *unuseds)
 * @brief      Function to work with application task
 * @param[in]  void
 * @return     void
 *=====================================================*/

void wifi_app_task(void *unused)
{
  int32_t status = SL_STATUS_FAIL;
  UNUSED_PARAMETER(unused);

  select_sem              = NULL;
  data_received_semaphore = NULL;

  while (1) {
    switch (wlan_app_cb) {
      case WIFI_APP_INITIAL_STATE: {
        wlan_app_callbacks_init();                //! register callback to initialize WLAN
        wlan_app_cb = WIFI_APP_UNCONNECTED_STATE; //! update Wi-Fi application state to unconnected state
      } break;
      case WIFI_APP_UNCONNECTED_STATE: {
        sl_wifi_scan_configuration_t wifi_scan_configuration = { 0 };
        wifi_scan_configuration                              = default_wifi_scan_configuration;

        printf("WLAN scan started \r\n");
        scan_complete   = false;
        callback_status = SL_STATUS_FAIL;
        sl_wifi_set_scan_callback_v2(wlan_app_scan_callback_handler, NULL);
        status = sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, NULL, &wifi_scan_configuration);
        if (SL_STATUS_IN_PROGRESS == status) {
          printf("Scanning...\r\n");
          const uint32_t start = osKernelGetTickCount();

          while (!scan_complete && (osKernelGetTickCount() - start) <= WIFI_SCAN_TIMEOUT) {
            osThreadYield();
          }
          status = scan_complete ? callback_status : SL_STATUS_TIMEOUT;
        }
        if (status != SL_STATUS_OK) {
          printf("WLAN Scan failed %lx\r\n", status);
          break;
        } else {
#if !WIFI_CONTINUOUS_SCAN_MODE_ONLY
          wlan_app_cb = WIFI_APP_SCAN_STATE; //! update WLAN application state to connected state
#endif
          printf("Scan done state \r\n");
        }

      } break;
      case WIFI_APP_SCAN_STATE: {
        sl_wifi_credential_t cred  = { 0 };
        sl_wifi_credential_id_t id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;

        cred.type = SL_WIFI_PSK_CREDENTIAL;
        memcpy(cred.psk.value, PSK, strlen((char *)PSK));

        status = sl_net_set_credential(id, SL_NET_WIFI_PSK, PSK, strlen((char *)PSK));
        if (SL_STATUS_OK == status) {
          printf("Credentials set, id : %lu\n", id);

          access_point.ssid.length = strlen((char *)SSID);
          memcpy(access_point.ssid.value, SSID, access_point.ssid.length);
          access_point.security      = SECURITY_TYPE;
          access_point.encryption    = SL_WIFI_DEFAULT_ENCRYPTION;
          access_point.credential_id = id;

          printf("SSID %s\n", access_point.ssid.value);
          status = sl_wifi_connect(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &access_point, TIMEOUT_MS);
        }
        if (status != SL_STATUS_OK) {
          printf("WLAN connection failed %lx\r\n", status);
          break;
        } else {
          wlan_app_cb = WIFI_APP_JOIN_STATE; //! update WLAN application state to connected state
          printf("WLAN connected state \r\n");
        }
      } break;
      case WIFI_APP_JOIN_STATE: {
        //! Configure IP
        ip_address.type      = SL_IPV4;
        ip_address.mode      = SL_IP_MANAGEMENT_DHCP;
        ip_address.host_name = DHCP_HOST_NAME;

        // Configure IP
        status = sl_si91x_configure_ip_address(&ip_address, SL_SI91X_WIFI_CLIENT_VAP_ID);
        if (status != SL_STATUS_OK) {
          printf("IP Config failed %lx\r\n", status);
          break;
        } else {
          wlan_app_cb = WIFI_APP_IPCONFIG_DONE_STATE;
          printf("WLAN ipconfig done state \r\n");
          sl_ip_address_t ip = { 0 };
          ip.type            = ip_address.type;
          ip.ip.v4.value     = ip_address.ip.v4.ip_address.value;
          print_sl_ip_address(&ip);
          printf("\r\n");
        }
#if WIFI_CONNECTION_ONLY
#if ENABLE_NWP_POWER_SAVE
        IoT_Error_t rc                                    = FAILURE;
        sl_wifi_performance_profile_t performance_profile = { .profile         = ASSOCIATED_POWER_SAVE_LOW_LATENCY,
                                                              .listen_interval = 1000 };
        if (!powersave_given) {
          osMutexAcquire(power_cmd_mutex, 0xFFFFFFFFUL);
          rc = sl_wifi_set_performance_profile(&performance_profile);
          if (rc != SL_STATUS_OK) {
            printf("\r\nPower save configuration Failed, Error Code : %d\r\n", rc);
          }
          printf("\r\nAssociated Power Save is enabled\r\n");
          osMutexRelease(power_cmd_mutex);
          powersave_given = 1;
        }
#endif
        osThreadSuspend(osThreadGetId());
#endif
      } break;
      case WIFI_APP_IPCONFIG_DONE_STATE: {
        start_aws_mqtt();
      } break;
      case WIFI_APP_IDLE_STATE: {
        //! Thread has no work to do in this state, so adding a delay of 5sec
        osDelay(1000);
        break;
      }
      default:
        break;
    }
  }
  // Clean up semaphores after MQTT session completes
  if (select_sem != NULL) {
    osSemaphoreDelete(select_sem);
    select_sem = NULL;
  }
  if (data_received_semaphore != NULL) {
    osSemaphoreDelete(data_received_semaphore);
    data_received_semaphore = NULL;
  }
}

sl_status_t start_aws_mqtt(void)
{

  IoT_Error_t rc     = FAILURE;
  sl_status_t status = SL_STATUS_OK;

  status = load_certificates_in_flash();
  if (status != SL_STATUS_OK) {
    printf("\r\nError while loading certificates: 0x%lx\r\n", status);
    return status;
  }
  printf("\r\nLoaded certificates\r\n");

  select_sem = osSemaphoreNew(1, 0, NULL);
  if (select_sem == NULL) {
    printf("\r\nFailed to create select_sem semaphore\r\n");
    return SL_STATUS_FAIL;
  }

  data_received_semaphore = osSemaphoreNew(1, 0, NULL);
  if (data_received_semaphore == NULL) {
    printf("\r\nFailed to create data_received_semaphore semaphore\r\n");
    return SL_STATUS_FAIL;
  }

#if !(defined(SLI_SI91X_MCU_INTERFACE) && ENABLE_NWP_POWER_SAVE)
  uint32_t start_time         = 0;
  uint8_t publish_timer_start = 0;
  uint8_t publish_msg         = 0;
#endif

  IoT_Client_Init_Params mqtt_init_params       = iotClientInitParamsDefault;
  IoT_Client_Connect_Params mqtt_connect_params = iotClientConnectParamsDefault;

  sl_mac_address_t mac_addr = { 0 };
  char mac_id[18];
  char client_id[25];
  sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_addr);
  sprintf(mac_id,
          "%x:%x:%x:%x:%x:%x",
          mac_addr.octet[0],
          mac_addr.octet[1],
          mac_addr.octet[2],
          mac_addr.octet[3],
          mac_addr.octet[4],
          mac_addr.octet[5]);
  printf("\r\nMAC ID: %s \r\n", mac_id);
  sprintf(client_id, "silabs_%s", mac_id);
  printf("\r\nClient ID: %s\r\n", client_id);

  sl_wifi_firmware_version_t fw_version = { 0 };
  status                                = sl_wifi_get_firmware_version(&fw_version);
  if (status != SL_STATUS_OK) {
    printf("\r\nFirmware version Failed, Error Code : 0x%lX\r\n", status);
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
  mqtt_connect_params.pClientID              = client_id;
  mqtt_connect_params.clientIDLen            = (uint16_t)strlen(AWS_IOT_MQTT_CLIENT_ID);
  mqtt_connect_params.isWillMsgPresent       = false;
  mqtt_connect_params.pUsername              = MQTT_USERNAME;
  mqtt_connect_params.usernameLen            = strlen(MQTT_USERNAME);
  mqtt_connect_params.pPassword              = MQTT_PASSWORD;
  mqtt_connect_params.passwordLen            = strlen(MQTT_PASSWORD);

  wlan_app_cb = WIFI_APP_MQTT_INIT_STATE;

  while (1) {

    switch (wlan_app_cb) {

      case WIFI_APP_MQTT_INIT_STATE: {
        // Clean up state variables from previous session
        check_for_recv_data = 0;
        qos1_publish_handle = 0;
        pub_state           = 0;
        select_given        = 0;
        rc                  = aws_iot_mqtt_init(&mqtt_client, &mqtt_init_params);
        if (SUCCESS != rc) {
          wlan_app_cb = WIFI_APP_MQTT_INIT_STATE;
          printf("\r\nMQTT Initialization failed with error: %d\r\n", rc);
        } else {
          wlan_app_cb = WIFI_APP_MQTT_CONNECT_STATE;
        }

      } break;
      case WIFI_APP_MQTT_CONNECT_STATE: {
        rc = aws_iot_mqtt_connect(&mqtt_client, &mqtt_connect_params);
        if (SUCCESS != rc) {
          if (rc == NETWORK_ALREADY_CONNECTED_ERROR) {
            printf("\r\nNetwork is already connected\r\n");
            wlan_app_cb = WIFI_APP_MQTT_SUBSCRIBE_STATE;
          } else {
            printf("\r\nMQTT connect failed with error: %d\r\n", rc);
            wlan_app_cb = WIFI_APP_MQTT_INIT_STATE;
          }
        } else {
          printf("\r\nConnected to AWS IoT Cloud\n");
          wlan_app_cb = WIFI_APP_MQTT_SUBSCRIBE_STATE;
        }

      } break;
      case WIFI_APP_MQTT_SUBSCRIBE_STATE: {
        rc = aws_iot_mqtt_subscribe(&mqtt_client,
                                    SUBSCRIBE_TO_TOPIC,
                                    strlen(SUBSCRIBE_TO_TOPIC),
                                    SUBSCRIBE_QOS,
                                    subscribe_handler,
                                    NULL);

        if (SUCCESS != rc) {
          if (NETWORK_DISCONNECTED_ERROR == rc) {
            printf("\r\nSubscription failed with error: %d\r\n", rc);
            wlan_app_cb = WIFI_APP_MQTT_CONNECT_STATE;
          } else if (NETWORK_ATTEMPTING_RECONNECT == rc) {
            // If the client is attempting to reconnect skip the rest of the loop
            continue;
          } else {
            wlan_app_cb = WIFI_APP_MQTT_SUBSCRIBE_STATE;
          }
        } else {
          printf("\rSubscribed to the specified topic with QoS%d\n", SUBSCRIBE_QOS);
          wlan_app_cb = WIFI_APP_AWS_SELECT_CONNECT_STATE;
        }

      } break;

      case WIFI_APP_AWS_SELECT_CONNECT_STATE: {
        {
          if (!select_given) {
            select_given = 1;
            memset(&read_fds, 0, sizeof(fd_set));

            FD_SET(mqtt_client.networkStack.socket_id, &read_fds);
            printf("\rSocket ID: %d\n", mqtt_client.networkStack.socket_id);

            status =
              sl_si91x_select(mqtt_client.networkStack.socket_id + 1, &read_fds, NULL, NULL, NULL, async_socket_select);

            printf("\rSelect status: 0x%lX\r\n", status);

            // Break here to allow async_socket_select callback to execute
            // Prevents race condition where state machine continues before callback completes
            break;
          }

          if (check_for_recv_data) {
            check_for_recv_data = 0;
            select_given        = 0;
            wlan_app_cb         = WIFI_APP_MQTT_RECEIVE_STATE;
          } else if (SUBSCRIBE_QOS == QOS1 || PUBLISH_QOS == QOS1) {
            if (qos1_publish_handle == 0) {
              wlan_app_cb = WIFI_APP_MQTT_PUBLISH_STATE;
            } else {
#if ENABLE_NWP_POWER_SAVE
              qos1_publish_handle = 0;
              wlan_app_cb         = WIFI_APP_SLEEP_STATE;
#else
              wlan_app_cb = WIFI_APP_MQTT_PUBLISH_STATE;
#endif
            }
          } else if (SUBSCRIBE_QOS == QOS0 || PUBLISH_QOS == QOS0) {
            wlan_app_cb = WIFI_APP_MQTT_PUBLISH_STATE;
          }
        }

      } break;

      case WIFI_APP_MQTT_RECEIVE_STATE: {
        rc = aws_iot_shadow_yield(&mqtt_client, 1);
        if (SUCCESS == rc) {
          printf("\rYield is successful\n");
#if !(defined(SLI_SI91X_MCU_INTERFACE) && ENABLE_NWP_POWER_SAVE)
          publish_msg = 1;
#endif
        }
        wlan_app_cb = WIFI_APP_AWS_SELECT_CONNECT_STATE;

      } break;

      case WIFI_APP_MQTT_PUBLISH_STATE: {
#if !(defined(SLI_SI91X_MCU_INTERFACE) && ENABLE_NWP_POWER_SAVE)
        if ((!publish_timer_start) || publish_msg) {
#endif
          publish_iot_msg.qos        = PUBLISH_QOS;
          publish_iot_msg.payload    = MQTT_PUBLISH_PAYLOAD;
          publish_iot_msg.isRetained = 0;
          publish_iot_msg.payloadLen = strlen(MQTT_PUBLISH_PAYLOAD);

          if (SUBSCRIBE_QOS == QOS1 || PUBLISH_QOS == QOS1) {
            pub_state = 1;
          }
          printf("\rData to be published: %s\n", MQTT_PUBLISH_PAYLOAD);
          rc = aws_iot_mqtt_publish(&mqtt_client, PUBLISH_ON_TOPIC, strlen(PUBLISH_ON_TOPIC), &publish_iot_msg);

          if (rc != SUCCESS) {
            if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
              printf("\r\nACK not received for QoS%d publish\r\n", PUBLISH_QOS);
            } else {
              printf("\r\nMQTT Publish with QoS%d failed with error: %d\n", PUBLISH_QOS, rc);
            }
            wlan_app_cb = WIFI_APP_MQTT_DISCONNECT;
            break;
          } else {
            printf("\rQoS%d publish is successful\r\n", PUBLISH_QOS);
          }

#if !(defined(SLI_SI91X_MCU_INTERFACE) && ENABLE_NWP_POWER_SAVE)
          publish_msg = 0;
          if (!publish_timer_start) {
            publish_timer_start = 1;
            start_time          = sl_si91x_host_get_timestamp();
          }

        } else {

          if (sl_si91x_host_elapsed_time(start_time) >= PUBLISH_PERIODICITY) {
            start_time          = 0;
            publish_timer_start = 0;
            publish_msg         = 1;
            continue;
          }
        }
#endif

#if ENABLE_NWP_POWER_SAVE
        sl_wifi_performance_profile_v2_t performance_profile = { .profile         = ASSOCIATED_POWER_SAVE_LOW_LATENCY,
                                                                 .listen_interval = 1000 };
        if (!powersave_given) {
          osMutexAcquire(power_cmd_mutex, 0xFFFFFFFFUL);
          rc = sl_wifi_set_performance_profile_v2(&performance_profile);
          if (rc != SL_STATUS_OK) {
            printf("\r\nPower save configuration Failed, Error Code : %d\r\n", rc);
          }
          printf("\r\nAssociated Power Save is enabled\r\n");
          osMutexRelease(power_cmd_mutex);
          powersave_given = 1;
        }
        if (SUBSCRIBE_QOS == QOS1 || PUBLISH_QOS == QOS1) {
          wlan_app_cb = WIFI_APP_SLEEP_STATE;
        } else if (SUBSCRIBE_QOS == QOS0 || PUBLISH_QOS == QOS0) {
          wlan_app_cb = WIFI_APP_SLEEP_STATE;
        }
#else
        wlan_app_cb = WIFI_APP_IDLE_STATE;
#endif
      } break;

#if ENABLE_NWP_POWER_SAVE
      case WIFI_APP_SLEEP_STATE: {
        if (select_given == 1 && (check_for_recv_data != 1)) {
          osSemaphoreAcquire(data_received_semaphore, PUBLISH_PERIODICITY);
        }
        wlan_app_cb = WIFI_APP_AWS_SELECT_CONNECT_STATE;

      } break;
#endif
      case WIFI_APP_MQTT_DISCONNECT: {
        rc = aws_iot_mqtt_disconnect(&mqtt_client);
        if (SUCCESS != rc) {
          printf("\r\nMQTT disconnection error\r\n");
        }
        wlan_app_cb = WIFI_APP_MQTT_INIT_STATE;

      } break;
      case WIFI_APP_IDLE_STATE: {

        wlan_app_cb = WIFI_APP_AWS_SELECT_CONNECT_STATE;

      } break;

      default:
        break;
    }
  }

  return rc;
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

void async_socket_select(fd_set *fd_read, fd_set *fd_write, fd_set *fd_except, int32_t status)
{
  UNUSED_PARAMETER(fd_except);
  UNUSED_PARAMETER(fd_write);

  if (status == SL_STATUS_OK) {
    //!Check the data pending on this particular socket descriptor
    if (FD_ISSET(mqtt_client.networkStack.socket_id, fd_read)) {
      if (pub_state != 1) { //This check is for handling PUBACK in QOS1
        check_for_recv_data = 1;
        osSemaphoreRelease(data_received_semaphore);
      } else {
        osSemaphoreRelease(select_sem);
      }
    }
    wlan_app_cb = WIFI_APP_AWS_SELECT_CONNECT_STATE;
  }
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
#endif
#endif