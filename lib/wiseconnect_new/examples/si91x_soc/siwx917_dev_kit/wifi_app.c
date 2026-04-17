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
#include "RTE_Device_917.h"
#include "rsi_retention.h"
#include "sl_status.h"
#include "rsi_ccp_user_config.h"

#include "em_assert.h"

#include <rsi_ble_apis.h>

#include "sl_wifi.h"
#include "sl_net_dns.h"
#include "sl_net_ping.h"
#include "cmsis_os2.h"
#include "sensor_app.h"

/******************************************************
 *                    Constants
 ******************************************************/

#define WIFI_CLIENT_PROFILE_SSID "YOUR_INITIAL_AP_SSID"

//MQTT related defines
#define WIFI_SCAN_TIMEOUT 10000

#define DHCP_HOST_NAME NULL

/******************************************************
 *               Function Declarations
 ******************************************************/

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
        status =
          sl_net_set_credential(wifi_client_profile.config.credential_id, SL_NET_WIFI_PSK, pwd, strlen((char *)pwd));
        if (status != SL_STATUS_OK) {
          printf("\r\nFailed to set client credentials: 0x%lx\r\n", status);
          continue;
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
          osDelay(1000);

          // Update WLAN application state
          wifi_app_send_to_ble(WIFI_APP_CONNECTION_STATUS, (uint8_t *)&connected, 1);
          wifi_app_set_event(WIFI_APP_IPCONFIG_DONE_STATE);
          osDelay(1000);
        }
      } break;

      case WIFI_APP_IPCONFIG_DONE_STATE: {
        wifi_app_clear_event(WIFI_APP_IPCONFIG_DONE_STATE);
        wlan_app_cb.state = WIFI_APP_IDLE_STATE;

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
        osDelay(1000);

        //! initiating power save in BLE mode
        status = rsi_bt_power_save_profile(PSP_MODE, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\nFailed to initiate power save in BLE mode\r\n");
        }
        //        uint32_t rc                                       = SL_STATUS_FAIL;
        //        sl_wifi_performance_profile_v2_t performance_profile = { .profile         = HIGH_PERFORMANCE,
        //                                                              .listen_interval = 1000 };
        //
        //        rc = sl_wifi_set_performance_profile_v2(&performance_profile);
        //        if (rc != SL_STATUS_OK) {
        //          printf("\r\nPower save configuration Failed, Error Code : 0x%lX\r\n", rc);
        //        }

        //        printf("\r\nAssociated power save enabled\r\n");

        sensor_app_init();

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
