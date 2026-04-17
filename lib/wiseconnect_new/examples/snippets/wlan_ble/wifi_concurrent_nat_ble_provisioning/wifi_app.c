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

/*================================================================================
 * @brief : This file contains an example application for Wi-Fi Concurrent NAT with BLE 
 * Provisioning.
 * @section Description :
 * This application brings up the SiWG917 as a station using BLE provisioning.
 * SiWG917 is then brought up with AP+STA interfaces in concurrent mode 
 * with NAT.
 * Third-party Wi-Fi clients connected to the SiWG917 can access the internet through
 * a third-party AP using NAT.
 =================================================================================*/

/**
 * Include files
 * */

//! SL Wi-Fi SDK includes
#include "sl_constants.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "sl_utility.h"
#include "sl_status.h"
#include "sl_wifi_device.h"
#include "sl_si91x_driver.h"
#include "sl_board_configuration.h"
#include "sl_net_ip_types.h"
#include "sl_net_si91x.h"
#include "sl_net.h"
#include "sl_net_default_values.h"

#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>

// BLE includes
#include "rsi_common_apis.h"
#include "rsi_bt_common_apis.h"
#include "ble_config.h"
#include <rsi_ble_apis.h>

// Wi-Fi Configuration includes
#include "wifi_config.h"

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
typedef void socket_handler(void *);
void create_tcp_server(void);
void create_udp_server(void);

const osThreadAttr_t server_thread_attr = { .name = "server_thread", .stack_size = 3072, .priority = osPriorityNormal };

extern rsi_ble_event_conn_status_t conn_event_to_app;

/******************************************************
 *                    Constants
 ******************************************************/
#define DHCP_HOST_NAME    NULL
#define TIMEOUT_MS        20000
#define WIFI_SCAN_TIMEOUT 10000

/*
 *********************************************************************************************************
 *                                         LOCAL GLOBAL VARIABLES
 *********************************************************************************************************
 */
sl_wifi_scan_result_t *scan_result          = NULL;
static volatile bool scan_complete          = false;
static volatile sl_status_t callback_status = SL_STATUS_OK;
uint16_t scanbuf_size = (sizeof(sl_wifi_scan_result_t) + (SL_WIFI_MAX_SCANNED_AP * sizeof(scan_result->scan_info[0])));

uint8_t connected = 0, timeout = 0;
uint8_t disconnected = 0, disassosiated = 0;
uint8_t a = 0;

sl_wifi_client_configuration_t access_point = { 0 };
sl_net_ip_configuration_t ip_address        = { 0 };

static uint32_t wlan_app_event_map;

typedef struct wlan_app_cb_s {
  //! wifi application state
  volatile wifi_app_state_t state;

  //! length of buffer to copy
  uint32_t length;

  //! application buffer
  uint8_t buffer[APP_BUF_SIZE];

  //! to check application buffer availability
  uint8_t buf_in_use;

  //! application events bit map
  uint32_t event_map;

} wlan_app_cb_t;

wlan_app_cb_t wlan_app_cb; //! application control block

static sl_wifi_ap_configuration_t wifi_ap_profile = {

  .ssid.value          = WIFI_AP_PROFILE_SSID,
  .ssid.length         = sizeof(WIFI_AP_PROFILE_SSID) - 1,
  .channel.channel     = SL_WIFI_AUTO_CHANNEL,
  .channel.band        = SL_WIFI_AUTO_BAND,
  .channel.bandwidth   = SL_WIFI_AUTO_BANDWIDTH,
  .security            = SL_WIFI_WPA2,
  .encryption          = SL_WIFI_CCMP_ENCRYPTION,
  .rate_protocol       = SL_WIFI_RATE_PROTOCOL_AUTO,
  .options             = 0,
  .credential_id       = SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID,
  .keepalive_type      = SL_SI91X_AP_NULL_BASED_KEEP_ALIVE,
  .beacon_interval     = 100,
  .client_idle_timeout = 0xFF,
  .dtim_beacon_count   = 3,
  .maximum_clients     = WIFI_MAX_CLIENTS,
  .beacon_stop         = 0
};

static sl_net_ip_configuration_t ip_add = {
  .mode      = SL_IP_MANAGEMENT_STATIC_IP,
  .type      = SL_IPV4,
  .host_name = NULL,
  .ip        = { .v4.ip_address.value = DEFAULT_WIFI_MODULE_IP_ADDRESS,
                 .v4.gateway.value    = DEFAULT_WIFI_GATEWAY_ADDRESS,
                 .v4.netmask.value    = DEFAULT_WIFI_SN_MASK_ADDRESS },
};

#if (THROUGHPUT_TYPE == TCP_TX)
static sl_si91x_socket_config_t socket_config = {
  TOTAL_SOCKETS,                   // Total sockets
  TOTAL_TCP_SOCKETS,               // Total TCP sockets
  TOTAL_UDP_SOCKETS,               // Total UDP sockets
  TCP_TX_ONLY_SOCKETS,             // TCP TX only sockets
  TCP_RX_ONLY_SOCKETS,             // TCP RX only sockets
  UDP_TX_ONLY_SOCKETS,             // UDP TX only sockets
  UDP_RX_ONLY_SOCKETS,             // UDP RX only sockets
  TCP_RX_HIGH_PERFORMANCE_SOCKETS, // TCP RX high performance sockets
  TCP_RX_WINDOW_SIZE_CAP,          // TCP RX window size
  TCP_RX_WINDOW_DIV_FACTOR         // TCP RX window division factor
};
#endif

sl_net_nat_config_t nat_config = {
  .tcp_session_timeout     = 60,
  .non_tcp_session_timeout = 60,
  .interface               = SL_NET_WIFI_CLIENT_1_INTERFACE,
};

typedef struct {
  int server_sock;
  struct sockaddr_in client_addr;
  socklen_t client_addr_len;
} client_data_t;

/*
 *********************************************************************************************************
 *                                               DATA TYPES
 *********************************************************************************************************
 */
extern void wifi_app_send_to_ble(uint16_t msg_type, uint8_t *data, uint16_t data_len);
extern uint8_t coex_ssid[50], pwd[34], sec_type;
static sl_status_t show_scan_results();
void wifi_app_set_event(uint32_t event_num);

// WLAN include file for configuration
osSemaphoreId_t rsi_mqtt_sem;
extern osSemaphoreId_t wlan_thread_sem;
extern sl_wifi_device_configuration_t config;

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
void wlan_app_callbacks_init(void)
{
  //! Initialize join fail call back
  sl_wifi_set_join_callback_v2(join_callback_handler, NULL);
}

static sl_status_t show_scan_results()
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(scan_result);
  uint8_t *bssid;
  LOG_PRINT("\r\n%lu WLAN scan results:\r\n", scan_result->scan_count);

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
    callback_status = *(sl_status_t *)result;
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

#if (THROUGHPUT_TYPE == TCP_TX) || (THROUGHPUT_TYPE == UDP_TX)
static void measure_and_print_throughput(size_t total_num_of_bytes, uint32_t test_timeout)
{
  float duration = (test_timeout / 1000);                      // ms to sec
  float result   = ((float)total_num_of_bytes * 8) / duration; // bytes to bps
  result         = (result / 1000000);                         // bps to Mbps
  LOG_PRINT("\r\nThroughput achieved @ %0.02f Mbps in %0.03f sec successfully\r\n", result, duration);
}
#endif

void wifi_app_task(void)
{
  int32_t status   = SL_STATUS_OK;
  int32_t event_id = 0;

  // Allocate memory for scan buffer
  scan_result = (sl_wifi_scan_result_t *)malloc(scanbuf_size);
  if (scan_result == NULL) {
    LOG_PRINT("\r\n[Wi-Fi STA]: Failed to allocate memory for scan result\r\n");
    return;
  }
  wlan_app_callbacks_init();
  memset(scan_result, 0, scanbuf_size);
  wifi_app_set_event(WIFI_APP_INITIAL_STATE);
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
        sl_wifi_set_callback_v2(SL_WIFI_CLIENT_CONNECTED_EVENTS, ap_connected_event_handler, NULL);
        sl_wifi_set_callback_v2(SL_WIFI_CLIENT_DISCONNECTED_EVENTS, ap_disconnected_event_handler, NULL);
#if AP_ONLY_MODE
        wifi_app_set_event(WIFI_AP_BRING_UP_STATE);
#endif
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
        scan_complete   = 0;
        callback_status = 0;
        status          = sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, NULL, &wifi_scan_configuration);
        if (SL_STATUS_IN_PROGRESS == status) {
          const uint32_t start = osKernelGetTickCount();

          while (!scan_complete && (osKernelGetTickCount() - start) <= WIFI_SCAN_TIMEOUT) {
            osThreadYield();
          }
          status = scan_complete ? callback_status : SL_STATUS_TIMEOUT;
        }
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n[Wi-Fi STA]: WLAN Scan Failed, Error Code : 0x%lX\r\n", status);
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
            LOG_PRINT("\r\n[Wi-Fi STA]: Credentials set, id : %lu\r\n", id);
          }
          if (status != SL_STATUS_OK) {
            LOG_PRINT("\r\n[Wi-Fi STA]: Failed to set credentials, id : %lu\r\n", id);
            continue;
          }
        }
        access_point.ssid.length = strlen((char *)coex_ssid);
        memcpy(access_point.ssid.value, coex_ssid, access_point.ssid.length);
        access_point.security      = sec_type;
        access_point.encryption    = SL_WIFI_DEFAULT_ENCRYPTION;
        access_point.credential_id = id;

        LOG_PRINT("\r\n[Wi-Fi STA]: SSID %s\r\n", access_point.ssid.value);
        status = sl_wifi_connect(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &access_point, TIMEOUT_MS);

        if (status != SL_STATUS_OK) {
          timeout = 1;
          wifi_app_send_to_ble(WIFI_APP_TIMEOUT_NOTIFY, (uint8_t *)&timeout, 1);
          LOG_PRINT("\r\n[Wi-Fi STA]: WLAN Connect Failed, Error Code : 0x%lX\r\n", status);

          // update wlan application state
          disconnected = 1;
          connected    = 0;
        } else {
          LOG_PRINT("\r\n[Wi-Fi STA]: WLAN Connection Success\r\n");
          // update wlan application state
          wifi_app_set_event(WIFI_APP_CONNECTED_STATE);
        }
      } break;

      case WIFI_APP_CONNECTED_STATE: {
        wifi_app_clear_event(WIFI_APP_CONNECTED_STATE);

        ip_address.type      = SL_IPV4;
        ip_address.mode      = SL_IP_MANAGEMENT_DHCP;
        ip_address.host_name = DHCP_HOST_NAME;

        // Configure IP
        status = sl_si91x_configure_ip_address(&ip_address, SL_SI91X_WIFI_CLIENT_VAP_ID);
        if (status != SL_STATUS_OK) {
          a++;
          if (a == 3) {
            a       = 0;
            timeout = 1;
            status  = sl_wifi_disconnect(SL_WIFI_CLIENT_INTERFACE);
            if (status == SL_STATUS_OK) {
              connected     = 0;
              disassosiated = 1;
              wifi_app_send_to_ble(WIFI_APP_TIMEOUT_NOTIFY, (uint8_t *)&timeout, 1);
              wifi_app_set_event(WIFI_AP_INIT_STATE);
            }
          }
          LOG_PRINT("\r\n[Wi-Fi STA]: IP Config Failed, Error Code : 0x%lX\r\n", status);
          break;
        } else {
          a                  = 0;
          connected          = 1;
          disconnected       = 0;
          disassosiated      = 0;
          sl_ip_address_t ip = { 0 };
          ip.type            = ip_address.type;
          ip.ip.v4.value     = ip_address.ip.v4.ip_address.value;
          LOG_PRINT("\r\n[Wi-Fi STA]: STA IPv4 : ");
          print_sl_ip_address(&ip);
          LOG_PRINT("\r\n");

          // update wlan application state
          wifi_app_set_event(WIFI_APP_IPCONFIG_DONE_STATE);
          wifi_app_send_to_ble(WIFI_APP_CONNECTION_STATUS, (uint8_t *)&connected, 1);
        }
      } break;

      case WIFI_APP_IPCONFIG_DONE_STATE: {
        wifi_app_clear_event(WIFI_APP_IPCONFIG_DONE_STATE);
        LOG_PRINT("\r\n[Wi-Fi STA]: Wi-Fi App IPCONFIG Done State\r\n");
        wifi_app_send_to_ble(WIFI_APP_REQ_BLE_DISCONNECT, NULL, 0);
      } break;

      case WIFI_APP_DISCONNECTED_STATE: {
        disconnected = 1;
        wifi_app_clear_event(WIFI_APP_DISCONNECTED_STATE);
        LOG_PRINT("\r\n[Wi-Fi STA]: Wi-Fi App Disconnected State\r\n");
#if !AP_ONLY_MODE
        wifi_app_set_event(WIFI_DISABLE_NAT);
#endif
      } break;

      case WIFI_APP_DISCONN_NOTIFY_STATE: {
        wifi_app_clear_event(WIFI_APP_DISCONN_NOTIFY_STATE);

        status = sl_wifi_disconnect(SL_WIFI_CLIENT_INTERFACE);
        if (status == SL_STATUS_OK) {
          disassosiated = 1;
          connected     = 0;
          LOG_PRINT("\r\n[Wi-Fi STA]: WLAN Disconnected\r\n");
          wifi_app_send_to_ble(WIFI_APP_DISCONNECTION_NOTIFY, (uint8_t *)&disassosiated, 1);
          wifi_app_set_event(WIFI_APP_UNCONNECTED_STATE);
        } else {
          LOG_PRINT("\r\n[Wi-Fi STA]: WIFI Disconnect Failed, Error Code : 0x%lX\r\n", status);
        }
      } break;

      case WIFI_AP_BRING_UP_STATE: {

        wifi_app_clear_event(WIFI_AP_BRING_UP_STATE);
#if !AP_ONLY_MODE
        sl_wifi_channel_t client_channel = { 0 };
        status                           = sl_wifi_get_channel(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &client_channel);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n[Wi-Fi AP]: Failed to get client channel: 0x%lx\r\n", status);
          break;
        }

        wifi_ap_profile.channel.channel = client_channel.channel;
#endif

        sl_wifi_credential_id_t id = SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID;
        if (wifi_ap_profile.security != SL_WIFI_OPEN) {
          status = sl_net_set_credential(id, SL_NET_WIFI_PSK, WIFI_AP_CREDENTIAL, strlen((char *)WIFI_AP_CREDENTIAL));
          if (status != SL_STATUS_OK) {
            LOG_PRINT("\r\n[Wi-Fi AP]: Failed to set credentials, id : %lu\r\n", id);
            continue;
          }
        }

        status = sl_si91x_configure_ip_address(&ip_add, SL_SI91X_WIFI_AP_VAP_ID);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n[Wi-Fi AP]: Failed to configure IPv4 address : 0x%lx\r\n", status);
          break;
        }

        if (ip_add.type == SL_IPV4) {
          sl_ip_address_t ip_address = { 0 };
          ip_address.type            = SL_IPV4;
          LOG_PRINT("\r\n[Wi-Fi AP]: AP's default gateway: ");
          memcpy(&ip_address.ip.v4.bytes, &ip_add.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
          print_sl_ip_address(&ip_address);
          LOG_PRINT("\r\n");
        }

        status = sl_net_set_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_DEFAULT_WIFI_AP_PROFILE_ID, &wifi_ap_profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n[Wi-Fi AP]: Failed to set profile: 0x%lx\r\n", status);
          break;
        }

        status = sl_wifi_start_ap(SL_WIFI_AP_2_4GHZ_INTERFACE, &wifi_ap_profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n[Wi-Fi AP]: Failed to bring Wi-Fi AP interface up: 0x%lx\r\n", status);
          break;
        }
        LOG_PRINT("\r\n[Wi-Fi AP]: AP started\r\n");

#if (THROUGHPUT_TYPE == TCP_TX)
        if (osThreadNew((osThreadFunc_t)create_tcp_server, NULL, &server_thread_attr) == NULL) {
          LOG_PRINT("\r\nFailed to create TCP server thread\r\n");
        }
#elif (THROUGHPUT_TYPE == UDP_TX)
        if (osThreadNew((osThreadFunc_t)create_udp_server, NULL, &server_thread_attr) == NULL) {
          LOG_PRINT("\r\nFailed to create UDP server thread\r\n");
        }
#endif

#if !AP_ONLY_MODE
        wifi_app_set_event(WIFI_ENABLE_NAT);
#endif
      } break;

      case WIFI_AP_BRING_DOWN_STATE: {
        wifi_app_clear_event(WIFI_AP_BRING_DOWN_STATE);
        status = sl_wifi_stop_ap(SL_WIFI_AP_2_4GHZ_INTERFACE);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n[Wi-Fi AP]: AP stop failed: 0x%lx\r\n", status);
          break;
        }
        LOG_PRINT("\r\n[Wi-Fi AP]: AP Stop Success\r\n");
        wifi_app_send_to_ble(WIFI_APP_REQ_BLE_START_ADVERTISING, NULL, 0);
      } break;

      case WIFI_ENABLE_NAT: {
        wifi_app_clear_event(WIFI_ENABLE_NAT);
        status = sl_net_nat_enable(&nat_config);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nEnabling the NAT failed with error code: 0x%lx\r\n", status);
        } else {
          LOG_PRINT("\r\nEnabled NAT\r\n");
        }
      } break;

      case WIFI_DISABLE_NAT: {
        wifi_app_clear_event(WIFI_DISABLE_NAT);
        status = sl_net_nat_disable(nat_config.interface);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nDisabling the NAT failed with error code: 0x%lx\r\n", status);
        } else {
          LOG_PRINT("\r\nDisabled NAT\r\n");
        }
        wifi_app_set_event(WIFI_AP_BRING_DOWN_STATE);
      } break;

      default:
        break;
    }
  }

  if (scan_result != NULL) {
    free(scan_result);
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

  LOG_PRINT("\r\nRemote Client connected: ");
  print_mac_address((sl_mac_address_t *)data);
  LOG_PRINT("\r\n");

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

  LOG_PRINT("\r\nRemote Client disconnected: ");
  print_mac_address((sl_mac_address_t *)data);

  LOG_PRINT("\r\n");

  return SL_STATUS_OK;
}

#if (THROUGHPUT_TYPE == TCP_TX)
void tcp_client_handler(void *arg)
{
  int client_sock = (int)(intptr_t)arg;
  char buffer[BUFFER_SIZE];
  ssize_t sent_bytes      = 0;
  size_t total_bytes_sent = 0;

  LOG_PRINT("\r\nClient Socket ID : %d\r\n", client_sock);
  LOG_PRINT("\r\nTCP_TX Throughput test start\r\n");
  uint32_t start = osKernelGetTickCount();
  uint32_t now   = start;

  while (total_bytes_sent < BYTES_TO_SEND) {
    sent_bytes = send(client_sock, buffer, sizeof(buffer), 0);
    if (sent_bytes < 0) {
      if (errno == ENOBUFS)
        continue;
      LOG_PRINT("\r\nSocket send failed with bsd error: %d\r\n", errno);
      break;
    }
    total_bytes_sent += sent_bytes;
    now = osKernelGetTickCount();

    if ((now - start) > TEST_TIMEOUT) {
      LOG_PRINT("\r\nTest Time Out: %ld ms\r\n", (now - start));
      break;
    }
  }

  LOG_PRINT("\r\nTCP data transfer completed on: socket %d\r\n", client_sock);
  LOG_PRINT("\r\nTotal bytes sent : %d\r\n", total_bytes_sent);

  measure_and_print_throughput(total_bytes_sent, (now - start));

  close(client_sock);
  LOG_PRINT("\r\nExiting from client handler\r\n");
  osThreadTerminate(osThreadGetId());
}

void create_tcp_server(void)
{
  int socket_return_value        = 0;
  int server_sock                = -1;
  struct sockaddr_in server_addr = { 0 };
  int ap_vap                     = 1;
  sl_status_t status             = sl_si91x_config_socket(socket_config);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nSocket config failed: %ld\r\n", status);
  } else {
    LOG_PRINT("\r\nSocket config Done\r\n");
  }

  server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_sock < 0) {
    LOG_PRINT("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  LOG_PRINT("\r\nServer Socket ID : %d\r\n", server_sock);

  socket_return_value = sl_si91x_setsockopt(server_sock, SOL_SOCKET, SL_SI91X_SO_SOCK_VAP_ID, &ap_vap, sizeof(ap_vap));
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSocket set sock opt failed with bsd error: %d\r\n", errno);
    close(server_sock);
    return;
  } else {
    LOG_PRINT("\r\nServer socket bound to Wi-Fi AP VAP\r\n");
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port   = htons(LISTENING_PORT);

  socket_return_value = bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    close(server_sock);
    return;
  }

  socket_return_value = listen(server_sock, BACK_LOG);
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSocket listen failed with bsd error: %d\r\n", errno);
    close(server_sock);
    return;
  }
  LOG_PRINT("\r\nTCP server listening on port : %d\r\n", LISTENING_PORT);

  while (1) {
    LOG_PRINT("\r\nwaiting for connections..\r\n");
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    memset(&client_addr, 0, sizeof(client_addr));
    int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_sock < 0) {
      LOG_PRINT("\r\nSocket accept failed with bsd error: %d\r\n", errno);
      continue;
    }

    osThreadAttr_t attr = { 0 };
    attr.stack_size     = 3072;
    attr.priority       = osPriorityLow;
    osThreadId_t tid    = osThreadNew(tcp_client_handler, (void *)(intptr_t)client_sock, &attr);
    if (tid == NULL) {
      LOG_PRINT("\r\nFailed to create thread for client socket %d\r\n", client_sock);
      close(client_sock);
    }
  }
  close(server_sock);
}

#elif (THROUGHPUT_TYPE == UDP_TX)
void udp_client_handler(void *arg)
{
  client_data_t *client_data = (client_data_t *)arg;
  char buffer[BUFFER_SIZE];
  ssize_t sent_bytes = 0;
  size_t total_bytes_sent = 0;
  LOG_PRINT("\r\nUDP_TX Throughput test start\r\n");
  uint32_t start = osKernelGetTickCount();
  uint32_t now = start;

  while (total_bytes_sent < BYTES_TO_SEND) {
    sent_bytes = sendto(client_data->server_sock,
                        buffer,
                        sizeof(buffer),
                        0,
                        (struct sockaddr *)&client_data->client_addr,
                        client_data->client_addr_len);

    if (sent_bytes < 0) {
      if (errno == ENOBUFS)
        continue;
      LOG_PRINT("\r\nSocket operation failed with bsd error: %d\r\n", errno);
      break;
    }
    total_bytes_sent += sent_bytes;
    now = osKernelGetTickCount();

    if ((now - start) > TEST_TIMEOUT) {
      LOG_PRINT("\r\nTest Time Out: %ld ms\r\n", (now - start));
      break;
    }
  }

  LOG_PRINT("\r\nTotal bytes transferred: %d\r\n", total_bytes_sent);

  measure_and_print_throughput(total_bytes_sent, (now - start));

  free(client_data);
  LOG_PRINT("\r\nExiting from client handler\r\n");
  osThreadTerminate(osThreadGetId());
}

void create_udp_server(void)
{
  int socket_return_value = 0;
  int server_sock = -1;
  struct sockaddr_in server_addr = { 0 };

  server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (server_sock < 0) {
    LOG_PRINT("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  LOG_PRINT("\r\nServer Socket ID : %d\r\n", server_sock);

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(LISTENING_PORT);

  socket_return_value = bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    close(server_sock);
    return;
  }
  LOG_PRINT("\r\nUDP server listening on port : %d\r\n", LISTENING_PORT);

  while (1) {
    client_data_t *client_data = malloc(sizeof(client_data_t));
    if (!client_data) {
      LOG_PRINT("\r\nFailed to allocate memory for client data\r\n");
      continue;
    }

    client_data->server_sock = server_sock;
    memset(&client_data->client_addr, 0, sizeof(client_data->client_addr));
    client_data->client_addr_len = sizeof(client_data->client_addr);

    char buffer[BUFFER_SIZE];
    ssize_t received_bytes = recvfrom(server_sock,
                                      buffer,
                                      sizeof(buffer),
                                      0,
                                      (struct sockaddr *)&client_data->client_addr,
                                      &client_data->client_addr_len);
    if (received_bytes < 0) {
      LOG_PRINT("\r\nSocket receive failed with error: %d\n", errno);
      free(client_data);
      continue;
    }

    osThreadAttr_t attr = { 0 };
    attr.stack_size = 3072;
    attr.priority = osPriorityLow;
    osThreadId_t tid = osThreadNew(udp_client_handler, (void *)client_data, &attr);
    if (tid == NULL) {
      LOG_PRINT("\r\nFailed to create thread for client\r\n");
    }
  }
  close(server_sock);
}
#endif
