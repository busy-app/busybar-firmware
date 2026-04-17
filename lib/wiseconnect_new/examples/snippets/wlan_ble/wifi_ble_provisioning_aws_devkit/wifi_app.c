/*******************************************************************************
 * @file  wifi_app.c
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @brief : This file contains example application for wifi_ble_provisioning_aws_devkit
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
#include "sl_constants.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "sl_net.h"
#include "sl_net_si91x.h"
#include "sl_utility.h"
#include "sl_status.h"
#include "sl_wifi_device.h"
#include "sl_net_wifi_types.h"
#include "sl_si91x_driver.h"
#include "sl_board_configuration.h"
#include "errno.h"
#include "socket.h"
#include "sl_si91x_socket.h"
#include "sl_wifi_device.h"
#include "sl_si91x_driver.h"
#include "app.h"
#include "rgb_led.h"
#include "sl_i2c_instances.h"
#include "sl_si91x_rgb_led_instances.h"

#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_power_manager.h"
#include "rsi_rom_power_save.h"
#include "sl_si91x_driver_gpio.h"
//! I2C related files
#include "rsi_ps_config.h"
#endif

#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "wifi_config.h"
#include "rsi_common_apis.h"
#include "rsi_bt_common_apis.h"

#include "aws_iot_config.h"
#include "aws_iot_shadow_interface.h"
#include "ble_config.h"
#include "wifi_config.h"
#include "aws_client_certificate.pem.crt.h"
#include "aws_client_private_key.pem.key.h"
#include "aws_starfield_ca.pem.h"
#include "jsmn.h"

//! AWS files
#include "aws_iot_log.h"
#include "aws_iot_error.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include <rsi_ble_apis.h>

#include "FreeRTOSConfig.h"

#if (defined(SLI_SI91X_MCU_INTERFACE) && (SL_SI91X_TICKLESS_MODE == 0))
#include "sl_si91x_m4_ps.h"
#endif

extern rsi_ble_event_conn_status_t conn_event_to_app;

/******************************************************
 *                    Constants
 ******************************************************/
#define DHCP_HOST_NAME    NULL
#define TIMEOUT_MS        5000
#define WIFI_SCAN_TIMEOUT 10000

#define MQTT_TOPIC1         "aws_status"   //! Subscribe Topic to receive the message from cloud
#define MQTT_TOPIC2         "si91x_status" //! Publish Topic to send the status from application to cloud
#define PUBLISH_PERIODICITY (30000)        //! Publish periodicity in milliseconds
#define MQTT_USERNAME       "username"
#define MQTT_PASSWORD       "password"

#define ENABLE_NWP_POWER_SAVE  0
#define I2C_SENSOR_PERI_ENABLE 0
#define LOW                    0

#define MAX_TOKENS 10 //! Adjust the size based on your JSON structure

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
osSemaphoreId_t data_received_semaphore;
extern osSemaphoreId_t select_sem;

uint8_t connected = 0, timeout = 0;
uint8_t disconnected = 0, disassosiated = 0;
uint8_t a = 0;

sl_wifi_client_configuration_t access_point = { 0 };
sl_net_ip_configuration_t ip_address        = { 0 };

static uint32_t wlan_app_event_map;
#if !(defined(SLI_SI91X_MCU_INTERFACE) && ENABLE_NWP_POWER_SAVE)
volatile uint8_t publish_msg = 0;
#endif

uint8_t retry = 1;

uint8_t yield;
AWS_IoT_Client client   = { 0 };
uint8_t disconnect_flag = 0;

uint32_t first_time                     = 0;
IoT_Publish_Message_Params publish_QOS0 = { 0 };
volatile uint8_t nvic_enable1[240];

fd_set read_fds;
volatile uint8_t check_for_recv_data = 0;
extern volatile uint8_t select_given;

int32_t status1            = RSI_SUCCESS;
AWS_IoT_Client mqtt_client = { 0 };
#define RSI_FD_ISSET(x, y) rsi_fd_isset(x, y)

typedef struct wlan_app_cb_s {
  //! wifi application state
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

char response_data_buffer[SENSOR_RESPONSE_BUFFER_SIZE] = { 0 };

/*
 *********************************************************************************************************
 *                                               DATA TYPES
 *********************************************************************************************************
 */
extern void wifi_app_send_to_ble(uint16_t msg_type, uint8_t *data, uint16_t data_len);
extern uint8_t coex_ssid[50], pwd[34], sec_type;
void wifi_app_mqtt_task(void);
void m4_sleep_wakeup(void);
void wakeup_source_config(void);
static sl_status_t show_scan_results();
void wifi_app_set_event(uint32_t event_num);
extern void get_sensor_data(char *response_data);
extern void sensors_init(void);
extern void sensors_deinit(void);

uint8_t conn_status;
extern uint8_t magic_word;

// WLAN include file for configuration
osSemaphoreId_t rsi_mqtt_sem;
extern osSemaphoreId_t wlan_thread_sem;

#ifdef SLI_SI91X_MCU_INTERFACE
void gpio_uulp_pin_interrupt_callback(uint32_t pin_intr)
{
  while (sl_si91x_gpio_get_uulp_npss_pin(pin_intr) == LOW)
    ; // waiting for the button release
#if (SL_SI91X_TICKLESS_MODE == ENABLE)
  osSemaphoreRelease(data_received_semaphore);
#endif
}
#endif
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

  // update wlan application state
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

void async_socket_select(fd_set *fd_read, fd_set *fd_write, fd_set *fd_except, int32_t status)
{
  UNUSED_PARAMETER(fd_except);
  UNUSED_PARAMETER(fd_write);
  UNUSED_PARAMETER(status);
  //!Check the data pending on this particular socket descriptor
  if (FD_ISSET(mqtt_client.networkStack.socket_id, fd_read)) {
    osSemaphoreRelease(data_received_semaphore);
    check_for_recv_data = 1;
  }
  wifi_app_set_event(WIFI_APP_AWS_SELECT_CONNECT_STATE);
}

// Helper function to compare JSON token with a string
static int jsoneq(const char *json, const jsmntok_t *tok, const char *s)
{
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start
      && strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

/**
 * @brief This parameter will avoid infinite loop of publish and exit the program after certain number of publishes
 */
static void iot_subscribe_callback_handler(AWS_IoT_Client *pClient,
                                           char *topicName,
                                           uint16_t topicNameLen,
                                           IoT_Publish_Message_Params *pParams,
                                           void *pData)
{
  UNUSED_PARAMETER(pClient);
  UNUSED_PARAMETER(topicName);
  UNUSED_PARAMETER(topicNameLen);
  UNUSED_PARAMETER(pData);

  printf("\rData received on the Subscribed Topic: %.*s \r\n", pParams->payloadLen, (char *)pParams->payload);

  // Initialize the JSON parser
  jsmn_parser parser;
  jsmn_init(&parser);

  // Allocate tokens for parsing JSON
  jsmntok_t tokens[MAX_TOKENS];
  int token_count =
    jsmn_parse(&parser, (char *)pParams->payload, pParams->payloadLen, tokens, sizeof(tokens) / sizeof(tokens[0]));

  if (token_count < 0) {
    printf("Failed to parse JSON: %d\n", token_count);
    return;
  }

  // Extract the values for the red, green, and blue LEDs
  bool red   = false;
  bool green = false;
  bool blue  = false;

  for (int i = 1; i < token_count; i++) {
    if (jsoneq((char *)pParams->payload, &tokens[i], "red") == 0) {
      if (jsoneq((char *)pParams->payload, &tokens[i + 1], "on") == 0) {
        red = true;
      }
      i++;
    } else if (jsoneq((char *)pParams->payload, &tokens[i], "green") == 0) {
      if (jsoneq((char *)pParams->payload, &tokens[i + 1], "on") == 0) {
        green = true;
      }
      i++;
    } else if (jsoneq((char *)pParams->payload, &tokens[i], "blue") == 0) {
      if (jsoneq((char *)pParams->payload, &tokens[i + 1], "on") == 0) {
        blue = true;
      }
      i++;
    }
  }

  // Check if more than one light is requested to be on
  if ((red && green) || (red && blue) || (green && blue)) {
    printf("Error: Only one light can be on at a time.\n");
  } else {
    // Invoke the rgb_led_set_state() function with the extracted values
    rgb_led_init_instances();
    rgb_led_set_state(red, green, blue);
  }

#if !(defined(SLI_SI91X_MCU_INTERFACE) && ENABLE_NWP_POWER_SAVE)
  publish_msg = 1;
#endif

  wifi_app_set_event(WIFI_APP_AWS_SELECT_CONNECT_STATE);
}

static void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data)
{
  IoT_Error_t rc = FAILURE;
  LOG_PRINT("MQTT Disconnect\r\n");
  if (NULL == pClient) {
    return;
  }
  IOT_UNUSED(data);
  if (aws_iot_is_autoreconnect_enabled(pClient)) {
    LOG_PRINT("Auto Reconnect is enabled, Reconnecting attempt will start now\r\n");
  } else {
    LOG_PRINT("Auto Reconnect not enabled. Starting manual reconnect...\r\n");
    rc = aws_iot_mqtt_attempt_reconnect(pClient);
    if (NETWORK_RECONNECTED == rc) {
      LOG_PRINT("Manual Reconnect Successful\r\n");
    } else {
      LOG_PRINT("Manual Reconnect \r\n");
    }
  }
}

sl_status_t load_certificates_in_flash(void)
{
  sl_status_t status;

  // Load SSL CA certificate
  status = sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0),
                                 SL_NET_SIGNING_CERTIFICATE,
                                 aws_starfield_ca,
                                 sizeof(aws_starfield_ca) - 1);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nLoading TLS CA certificate in to FLASH Failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  LOG_PRINT("\r\nLoading TLS CA certificate at index %d Successfull\r\n", 0);

  // Load SSL Client certificate
  status = sl_net_set_credential(SL_NET_TLS_CLIENT_CREDENTIAL_ID(0),
                                 SL_NET_CERTIFICATE,
                                 aws_client_certificate,
                                 sizeof(aws_client_certificate) - 1);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nLoading TLS Client certificate in to FLASH Failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  LOG_PRINT("\r\nLoading TLS Client certificate at index %d Successfull\r\n", 0);

  // Load SSL Client private key
  status = sl_net_set_credential(SL_NET_TLS_CLIENT_CREDENTIAL_ID(0),
                                 SL_NET_PRIVATE_KEY,
                                 aws_client_private_key,
                                 sizeof(aws_client_private_key) - 1);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nLoading TLS Client private key in to FLASH Failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  LOG_PRINT("\r\nLoading TLS Client private key at index %d Successfull\r\n", 0);

  return SL_STATUS_OK;
}

int32_t rsi_wlan_mqtt_certs_init(void)
{
  sl_status_t status = RSI_SUCCESS;

  status = load_certificates_in_flash();
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nUnexpected error while loading certificates: 0x%lx\r\n", status);
    return status;
  }

  rsi_wlan_app_callbacks_init();

  return status;
}

static sl_status_t show_scan_results()
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(scan_result);
  uint8_t *bssid = NULL;
  LOG_PRINT("%lu Scan results:\r\n", scan_result->scan_count);

  if (scan_result->scan_count) {
    LOG_PRINT("\r\n   %s %24s %s", "SSID", "SECURITY", "NETWORK");
    LOG_PRINT("%12s %12s %s\r\n", "BSSID", "CHANNEL", "RSSI");

    for (int a = 0; a < (int)scan_result->scan_count; ++a) {
      bssid = (uint8_t *)&scan_result->scan_info[a].bssid;
      LOG_PRINT("%-24s %4u,  %4u, ",
                scan_result->scan_info[a].ssid,
                scan_result->scan_info[a].security_mode,
                scan_result->scan_info[a].network_type);
      LOG_PRINT("  %02x:%02x:%02x:%02x:%02x:%02x, %4u,  -%u\r\n",
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
  SL_VERIFY_POINTER_OR_RETURN(scan_result, SL_STATUS_NULL_POINTER);
  memset(scan_result, 0, scanbuf_size);
  memcpy(scan_result, result, scanbuf_size);

  if (result_length != 0) {
    callback_status = show_scan_results();
  }

  // Send wlan scan result to BLE module
  wifi_app_send_to_ble(WIFI_APP_SCAN_RESP, (uint8_t *)scan_result, scanbuf_size);

  return SL_STATUS_OK;
}

void wifi_app_task(void)
{
  int32_t status          = RSI_SUCCESS;
  int32_t event_id        = 0;
  data_received_semaphore = osSemaphoreNew(1, 0, NULL);

  // Allocate memory for scan buffer
  scan_result = (sl_wifi_scan_result_t *)malloc(scanbuf_size);
  if (scan_result == NULL) {
    LOG_PRINT("Failed to allocate memory for scan result\r\n");
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

        // update wlan application state
        if (magic_word) {
          // clear the served event
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
          wifi_app_set_event(WIFI_APP_SCAN_STATE);
          osDelay(1000);
        }
      } break;

      case WIFI_APP_JOIN_STATE: {
        sl_wifi_credential_t cred  = { 0 };
        sl_wifi_credential_id_t id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;
        memset(&access_point, 0, sizeof(sl_wifi_client_configuration_t));

        wifi_app_clear_event(WIFI_APP_JOIN_STATE);

        cred.type = SL_WIFI_PSK_CREDENTIAL;
        memcpy(cred.psk.value, pwd, strlen((char *)pwd));

        if (sec_type != SL_WIFI_OPEN) {
          status = sl_net_set_credential(id, SL_NET_WIFI_PSK, pwd, strlen((char *)pwd));
          if (SL_STATUS_OK == status) {
            LOG_PRINT("Credentials set, id : %lu\r\n", id);
          }
          if (status != SL_STATUS_OK) {
            printf("Credentials set failed, id : %lu\r\n", id);
            continue;
          }
        }
        access_point.ssid.length = strlen((char *)coex_ssid);
        memcpy(access_point.ssid.value, coex_ssid, access_point.ssid.length);
        access_point.security      = sec_type;
        access_point.encryption    = SL_WIFI_DEFAULT_ENCRYPTION;
        access_point.credential_id = id;

        LOG_PRINT("SSID %s\r\n", access_point.ssid.value);
        status = sl_wifi_connect(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &access_point, TIMEOUT_MS);

        if (status != RSI_SUCCESS) {
          timeout = 1;
          wifi_app_send_to_ble(WIFI_APP_TIMEOUT_NOTIFY, (uint8_t *)&timeout, 1);
          LOG_PRINT("\r\nWLAN Connect Failed, Error Code : 0x%lX\r\n", status);

          // update wlan application state
          disconnected = 1;
          connected    = 0;
        } else {
          LOG_PRINT("\n WLAN Connection Success\r\n");
          // update wlan application state
          wifi_app_set_event(WIFI_APP_CONNECTED_STATE);
        }
      } break;

      case WIFI_APP_FLASH_STATE: {
        wifi_app_clear_event(WIFI_APP_FLASH_STATE);

        if (retry) {
          status = sl_wifi_connect(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &access_point, TIMEOUT_MS);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\nWLAN Connect Failed, Error Code : 0x%lX\r\n", status);
            break;
          } else {
            wifi_app_set_event(WIFI_APP_CONNECTED_STATE);
          }
        }
      } break;

      case WIFI_APP_CONNECTED_STATE: {
        wifi_app_clear_event(WIFI_APP_CONNECTED_STATE);

        ip_address.type      = SL_IPV4;
        ip_address.mode      = SL_IP_MANAGEMENT_DHCP;
        ip_address.host_name = DHCP_HOST_NAME;

        // Configure IP
        status = sl_si91x_configure_ip_address(&ip_address, SL_SI91X_WIFI_CLIENT_VAP_ID);
        if (status != RSI_SUCCESS) {
          a++;
          if (a == 3) {
            a       = 0;
            timeout = 1;
            status  = sl_wifi_disconnect(SL_WIFI_CLIENT_INTERFACE);
            if (status == RSI_SUCCESS) {
              connected     = 0;
              disassosiated = 1;
              wifi_app_send_to_ble(WIFI_APP_TIMEOUT_NOTIFY, (uint8_t *)&timeout, 1);
              wifi_app_set_event(WIFI_APP_ERROR_STATE);
            }
          }
          LOG_PRINT("\r\nIP Config Failed, Error Code : 0x%lX\r\n", status);
          break;
        } else {
          a             = 0;
          connected     = 1;
          conn_status   = 1;
          disconnected  = 0;
          disassosiated = 0;

#if defined(SL_SI91X_PRINT_DBG_LOG)
          sl_ip_address_t ip = { 0 };
          ip.type            = ip_address.type;
          ip.ip.v4.value     = ip_address.ip.v4.ip_address.value;
          print_sl_ip_address(&ip);
#endif

          // update wlan application state
          wifi_app_set_event(WIFI_APP_IPCONFIG_DONE_STATE);
          wifi_app_send_to_ble(WIFI_APP_CONNECTION_STATUS, (uint8_t *)&connected, 1);
        }
      } break;

      case WIFI_APP_IPCONFIG_DONE_STATE: {
        wifi_app_clear_event(WIFI_APP_IPCONFIG_DONE_STATE);
        wlan_app_cb.state = WIFI_APP_MQTT_INIT_STATE;
        wifi_app_mqtt_task();

        LOG_PRINT("WIFI App IPCONFIG Done State\r\n");
      } break;

      case WIFI_APP_ERROR_STATE: {

      } break;

      case WIFI_APP_DISCONNECTED_STATE: {
        wifi_app_clear_event(WIFI_APP_DISCONNECTED_STATE);
        retry = 1;
        wifi_app_send_to_ble(WIFI_APP_DISCONNECTION_STATUS, (uint8_t *)&disconnected, 1);
        wifi_app_set_event(WIFI_APP_FLASH_STATE);

        LOG_PRINT("WIFI App Disconnected State\r\n");
      } break;

      case WIFI_APP_DISCONN_NOTIFY_STATE: {
        wifi_app_clear_event(WIFI_APP_DISCONN_NOTIFY_STATE);

        status = sl_wifi_disconnect(SL_WIFI_CLIENT_INTERFACE);
        if (status == RSI_SUCCESS) {
#if RSI_WISE_MCU_ENABLE
          rsi_flash_erase((uint32_t)FLASH_ADDR_TO_STORE_AP_DETAILS);
#endif
          LOG_PRINT("\r\nWLAN Disconnected\r\n");
          disassosiated   = 1;
          connected       = 0;
          yield           = 0;
          disconnect_flag = 0; // reset flag to allow disconnecting again
          select_given    = 0;
          osSemaphoreRelease(select_sem);
          wifi_app_send_to_ble(WIFI_APP_DISCONNECTION_NOTIFY, (uint8_t *)&disassosiated, 1);
          wifi_app_set_event(WIFI_APP_UNCONNECTED_STATE);
        } else {
          LOG_PRINT("\r\nWIFI Disconnect Failed, Error Code : 0x%lX\r\n", status);
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

void wifi_app_mqtt_task(void)
{
  IoT_Error_t rc = FAILURE;

  IoT_Client_Init_Params mqttInitParams   = iotClientInitParamsDefault;
  IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;
#if !(defined(SLI_SI91X_MCU_INTERFACE) && ENABLE_NWP_POWER_SAVE)
  uint32_t start_time         = 0;
  uint8_t publish_timer_start = 0;
#endif

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

  mqttInitParams.enableAutoReconnect       = false;
  mqttInitParams.pHostURL                  = AWS_IOT_MQTT_HOST;
  mqttInitParams.port                      = AWS_IOT_MQTT_PORT;
  mqttInitParams.pRootCALocation           = (char *)aws_starfield_ca;
  mqttInitParams.pDeviceCertLocation       = (char *)aws_client_certificate;
  mqttInitParams.pDevicePrivateKeyLocation = (char *)aws_client_private_key;
  mqttInitParams.mqttCommandTimeout_ms     = 20000;
  mqttInitParams.tlsHandshakeTimeout_ms    = 5000;
  mqttInitParams.isSSLHostnameVerify       = true;
  mqttInitParams.disconnectHandler         = disconnectCallbackHandler;
  mqttInitParams.disconnectHandlerData     = NULL;

  connectParams.keepAliveIntervalInSec = 600;
  connectParams.isCleanSession         = true;
  connectParams.MQTTVersion            = MQTT_3_1_1;
  connectParams.pClientID              = client_id;
  connectParams.clientIDLen            = (uint16_t)strlen(client_id);
  connectParams.isWillMsgPresent       = false;

  connectParams.pUsername   = MQTT_USERNAME;
  connectParams.usernameLen = strlen(MQTT_USERNAME);
  connectParams.pPassword   = MQTT_PASSWORD;
  connectParams.passwordLen = strlen(MQTT_PASSWORD);

  wlan_app_cb.state = WIFI_APP_MQTT_INIT_STATE;

  osSemaphoreRelease(rsi_mqtt_sem);

  while (1) {
    osSemaphoreAcquire(rsi_mqtt_sem, osWaitForever);

    if (wifi_app_get_event() == WIFI_APP_DISCONN_NOTIFY_STATE) {
      LOG_PRINT("WLAN disconnect initiated\r\n");
      rc = aws_iot_mqtt_disconnect(&mqtt_client);
      if (SUCCESS != rc) {
        LOG_PRINT("MQTT Disconnection error : %d\r\n", rc);
      } else {
        LOG_PRINT("MQTT Disconnection Successful\r\n");
      }
      return;
    }

    switch (wlan_app_cb.state) {

      case WIFI_APP_MQTT_INIT_STATE: {
        rc = aws_iot_mqtt_init(&mqtt_client, &mqttInitParams);
        if (SUCCESS != rc) {
          wlan_app_cb.state = WIFI_APP_MQTT_INIT_STATE;
          LOG_PRINT("\r\nMqtt Init failed with error: %d\r\n", rc);
        } else {
          wlan_app_cb.state = WIFI_APP_MQTT_CONNECT_STATE;
        }

        osSemaphoreRelease(rsi_mqtt_sem);
      } break;

      case WIFI_APP_MQTT_CONNECT_STATE: {
        LOG_PRINT("AWS IOT MQTT Connecting...\r\n");
        rc = aws_iot_mqtt_connect(&mqtt_client, &connectParams);
        if (SUCCESS != rc) {
          if (rc == NETWORK_ALREADY_CONNECTED_ERROR) {
            LOG_PRINT("Network is already connected\r\n");
            //wlan_app_cb.state = WIFI_APP_MQTT_PUBLISH_STATE;
          } else {
            LOG_PRINT("\r\nMqtt Connect failed with error: %d\r\n", rc);
            wlan_app_cb.state = WIFI_APP_MQTT_INIT_STATE;
          }
        } else {
          wlan_app_cb.state = WIFI_APP_MQTT_AUTO_RECONNECT_SET_STATE;
        }
        osSemaphoreRelease(rsi_mqtt_sem);
      } break;

      case WIFI_APP_MQTT_AUTO_RECONNECT_SET_STATE: {
        rc = aws_iot_mqtt_autoreconnect_set_status(&mqtt_client, false);
        if (SUCCESS != rc) {
          if (NETWORK_DISCONNECTED_ERROR == rc) {
            LOG_PRINT("MQTT auto reconnect error : %d\r\n", rc);
            wlan_app_cb.state = WIFI_APP_MQTT_CONNECT_STATE;
          } else if (NETWORK_ATTEMPTING_RECONNECT == rc) {
            // If the client is attempting to reconnect we will skip the rest of the loop.
            continue;
          }
          LOG_PRINT("Unable to set Auto Reconnect to true\r\n ");
          wlan_app_cb.state = WIFI_APP_MQTT_AUTO_RECONNECT_SET_STATE;
        } else {
          wlan_app_cb.state = WIFI_APP_MQTT_SUBSCRIBE_STATE;
        }
        osSemaphoreRelease(rsi_mqtt_sem);
      } break;

      case WIFI_APP_MQTT_SUBSCRIBE_STATE: {
        LOG_PRINT("\r\nAWS IOT MQTT Subscribe...\r\n");
        rc = aws_iot_mqtt_subscribe(&mqtt_client,
                                    MQTT_TOPIC1,
                                    strlen(MQTT_TOPIC1),
                                    QOS0,
                                    iot_subscribe_callback_handler,
                                    NULL);

        if (SUCCESS != rc) {
          if (NETWORK_DISCONNECTED_ERROR == rc) {
            LOG_PRINT("\r\nSubscribe error : %d\r\n", rc);
            wlan_app_cb.state = WIFI_APP_MQTT_CONNECT_STATE;
          } else if (NETWORK_ATTEMPTING_RECONNECT == rc) {
            // If the client is attempting to reconnect we will skip the rest of the loop.
            continue;
          }
          wlan_app_cb.state = WIFI_APP_MQTT_SUBSCRIBE_STATE;
        }
        wlan_app_cb.state = WIFI_APP_AWS_SELECT_CONNECT_STATE;
#if ENABLE_NWP_POWER_SAVE
        //! initiating power save in BLE mode
        if (rsi_bt_power_save_profile(PSP_MODE, PSP_TYPE) != RSI_SUCCESS) {
          LOG_PRINT("\r\n Failed to initiate power save in BLE mode \r\n");
        }

        sl_wifi_performance_profile_v2_t performance_profile = { .profile         = ASSOCIATED_POWER_SAVE_LOW_LATENCY,
                                                                 .listen_interval = 1000 };

        sl_status_t status = sl_wifi_set_performance_profile_v2(&performance_profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nPower save configuration Failed, Error Code : 0x%lX\r\n", status);
        }
        LOG_PRINT("\r\nAssociated Power Save Enabled\r\n");
#endif

        osSemaphoreRelease(rsi_mqtt_sem);
      } break;

      case WIFI_APP_AWS_SELECT_CONNECT_STATE: {
        {

          if (!select_given) {
            select_given = 1;
            memset(&read_fds, 0, sizeof(fd_set));

            FD_SET(mqtt_client.networkStack.socket_id, &read_fds);
            LOG_PRINT("\r\n Socket ID: %d\r\n", mqtt_client.networkStack.socket_id);

            status1 =
              sl_si91x_select(mqtt_client.networkStack.socket_id + 1, &read_fds, NULL, NULL, NULL, async_socket_select);
          }

          if (check_for_recv_data) {
            check_for_recv_data = 0;
            select_given        = 0;
            status1             = aws_iot_shadow_yield(&mqtt_client, 1);

            wlan_app_cb.state = WIFI_APP_AWS_SELECT_CONNECT_STATE;
          } else {
            wlan_app_cb.state = WIFI_APP_MQTT_PUBLISH_STATE;
          }
        }
        osSemaphoreRelease(rsi_mqtt_sem);
      } break;

      case WIFI_APP_MQTT_PUBLISH_STATE: {
        if (NETWORK_ATTEMPTING_RECONNECT == rc) {
          // If the client is attempting to reconnect we will skip the rest of the loop.
          continue;
        }
#if !(defined(SLI_SI91X_MCU_INTERFACE) && ENABLE_NWP_POWER_SAVE)
        if ((!publish_timer_start) || publish_msg) {
#endif

          publish_QOS0.qos = QOS0;
          sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS3);
          sl_i2c_init_instances();
          // Initialize the sensors
          sensors_init();
          get_sensor_data(response_data_buffer);
          // Deinitialize the sensors
          sensors_deinit();
          sl_si91x_power_manager_remove_ps_requirement(SL_SI91X_POWER_MANAGER_PS3);
          publish_QOS0.payload    = response_data_buffer;
          publish_QOS0.isRetained = 0;
          publish_QOS0.payloadLen = strlen(response_data_buffer);

#if (defined(SLI_SI91X_MCU_INTERFACE) && I2C_SENSOR_PERI_ENABLE)
          char temp_string[48] = { 0 };

          // Initialize I2C
          i2c_init();
          i2c_leader_example_process_action();

          snprintf(temp_string, sizeof(temp_string) - 1, "Current Temperature in Celsius: %.2f", sensor_data);

          publish_QOS0.payload    = temp_string;
          publish_QOS0.payloadLen = strlen(temp_string);
#endif
          printf("\rData to be published: %s\n", response_data_buffer);
          // mqtt publish with QOS0
          rc = aws_iot_mqtt_publish(&mqtt_client, MQTT_TOPIC2, strlen(MQTT_TOPIC2), &publish_QOS0);

          if (rc != SUCCESS) {
            LOG_PRINT("\r\nMqtt Publish for QOS0 failed with error: %d\r\n", rc);
            wlan_app_cb.state = WIFI_APP_MQTT_DISCONNECT;
            break;
          }

          if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
            LOG_PRINT("QOS0 publish ack not received.\r\n");
          }
          LOG_PRINT("\r\nMQTT Publish Successful\r\n");
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

        wlan_app_cb.state = WIFI_APP_AWS_SELECT_CONNECT_STATE;

#if ENABLE_NWP_POWER_SAVE
        wlan_app_cb.state = WIFI_APP_SLEEP_STATE;
#endif

        osSemaphoreRelease(rsi_mqtt_sem);
      } break;

#if ENABLE_NWP_POWER_SAVE
      case WIFI_APP_SLEEP_STATE: {
        osDelay(200);

#ifdef SLI_SI91X_MCU_INTERFACE

        if (select_given == 1 && (check_for_recv_data != 1)) {
          printf("M4 in sleep\r\n");
#if (SL_SI91X_TICKLESS_MODE == 0)
          sl_si91x_power_manager_sleep();
#else
          if (osSemaphoreAcquire(data_received_semaphore, PUBLISH_PERIODICITY) == osOK) {
          }
#endif
          printf("M4 Wake up\r\n");
        }

#endif

        wlan_app_cb.state = WIFI_APP_AWS_SELECT_CONNECT_STATE;

        osSemaphoreRelease(rsi_mqtt_sem);
      } break;
#endif

      case WIFI_APP_MQTT_DISCONNECT: {
        rc = aws_iot_mqtt_disconnect(&mqtt_client);
        if (SUCCESS != rc) {
          LOG_PRINT("MQTT Disconnection error : %d\r\n", rc);
          wlan_app_cb.state = WIFI_APP_MQTT_INIT_STATE;
        } else {
          LOG_PRINT("MQTT Disconnection Successful\r\n");
          wlan_app_cb.state = WIFI_APP_MQTT_INIT_STATE;
        }
        osSemaphoreRelease(rsi_mqtt_sem);
      } break;
      case WIFI_APP_DISCONN_NOTIFY_STATE: {
        return;
      }
      default:
        break;
    }
  }
}
