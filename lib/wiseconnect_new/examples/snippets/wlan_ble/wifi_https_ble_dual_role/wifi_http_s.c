/*******************************************************************************
* @file  wifi_http_s.c
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
/**
 *
 *  @brief : This file contains example application for TCP+SSL client socket
 *
 *  @section Description  This file contains example application for TCP+SSL client socket
 */

/*=======================================================================*/
//  ! INCLUDES
/*=======================================================================*/

#include "app_common_config.h"

#if RSI_ENABLE_WLAN_TEST
#include "stdlib.h"
#include "wifi_app_config.h"

//! SL Wi-Fi SDK includes
#include "sl_constants.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "sl_net.h"
#include "sl_net_si91x.h"
#include "sl_utility.h"
#include "errno.h"
#include "socket.h"
#include "sl_si91x_core_utilities.h"
#include "cmsis_os2.h"
#include <rsi_common_apis.h>
#include <string.h>
#if SSL
#include "cacert.pem.h" //! Include SSL CA certificate
#endif

/*=======================================================================*/
//   ! MACROS
/*=======================================================================*/
#define DHCP_HOST_NAME    NULL
#define TIMEOUT_MS        25000
#define WIFI_SCAN_TIMEOUT 10000

#define SL_HIGH_PERFORMANCE_SOCKET BIT(7)
/*=======================================================================*/
//   ! GLOBAL VARIABLES
/*=======================================================================*/
rsi_wlan_app_cb_t rsi_wlan_app_cb; //! application control block

int32_t client_socket; //! client socket id
volatile uint8_t data_recvd = 0;
volatile uint64_t num_bytes = 0;

//! HTTP/HTTPS headers
const char httpreq[] = "GET /" DOWNLOAD_FILENAME " HTTP/1.1\r\n"
                       "Host: " SERVER_IP_ADDRESS "\r\n"
                       "User-Agent: silabs/1.0.4a\r\n"
                       "Accept: */*\r\n\r\n";

#if USE_CONNECTION_CLOSE
const char http_req_str_connection_close[] = "Connection: close\r\n";
#endif

int8_t recv_buffer2[RECV_BUFFER_SIZE];
int32_t recv_size2 = RECV_BUFFER_SIZE;

/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/
#if WLAN_SYNC_REQ
extern osSemaphoreId_t sync_coex_ble_sem;
#if (WLAN_SCAN_ONLY || WLAN_CONNECTION_ONLY)
extern osSemaphoreId_t sync_coex_wlan_sem;
#endif
#endif
extern bool rsi_ble_running;
#if ENABLE_NWP_POWER_SAVE
extern osMutexId_t power_cmd_mutex;
extern bool powersave_cmd_given;
#endif

sl_wifi_scan_result_t *scan_result          = NULL;
static volatile bool scan_complete          = false;
static volatile sl_status_t callback_status = SL_STATUS_OK;
static osSemaphoreId_t scan_complete_sem    = NULL;
uint16_t scanbuf_size = (sizeof(sl_wifi_scan_result_t) + (SL_WIFI_MAX_SCANNED_AP * sizeof(scan_result->scan_info[0])));

sl_wifi_client_configuration_t access_point = { 0 };
sl_net_ip_configuration_t ip_address        = { 0 };
extern osThreadId_t app_thread_id;
/*************************************************************************/
//!  CALLBACK FUNCTIONS
/*************************************************************************/

static sl_status_t show_scan_results(sl_wifi_scan_result_t *scan_result);

sl_status_t join_callback_handler(sl_wifi_event_t event,
                                  sl_status_t status_code,
                                  char *result,
                                  uint32_t result_length,
                                  void *arg)
{
  UNUSED_PARAMETER(result);
  UNUSED_PARAMETER(arg);
  int32_t status = 0;

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    LOG_PRINT("F: Join Event received with %lu bytes payload\n", result_length);
    if (client_socket) {
      status = close(client_socket);
      if (status != 0) {
        LOG_PRINT("\r\nsocket close failed with status = %ld and BSD error: %d\r\n", status, errno);
      } else {
        LOG_PRINT("\r\nsocket close success\r\n");
      }
    }
    rsi_wlan_app_cb.state = RSI_WLAN_UNCONNECTED_STATE;
    return status_code;
  }

  return SL_STATUS_OK;
}
/*====================================================*/
/**
 * @fn          void rsi_wlan_app_callbacks_init(void)
 * @brief       To initialize WLAN application callback
 * @param[in]   void
 * @return      void
 * @section description
 * This callback is used to initialize WLAN
 * ==================================================*/
void rsi_wlan_app_callbacks_init(void)
{
  //! Initialize join fail call back
  sl_wifi_set_join_callback_v2(join_callback_handler, NULL);
}

#if (SSL && LOAD_CERTIFICATE)
sl_status_t clear_and_load_certificates_in_flash(void)
{
  sl_status_t status;

  // Load SSL CA certificate
  status =
    sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0), SL_NET_SIGNING_CERTIFICATE, cacert, sizeof(cacert) - 1);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nLoading TLS CA certificate in to FLASH Failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  LOG_PRINT("\r\nLoad TLS CA certificate at index %d Success\r\n", 0);

  return status;
}
#endif

static sl_status_t show_scan_results(sl_wifi_scan_result_t *scan_result)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(scan_result);
  uint8_t *bssid = NULL;
  LOG_PRINT("%lu Scan results:\n", scan_result->scan_count);
  if (scan_result->scan_count) {
    LOG_PRINT("\n   %s %24s %s", "SSID", "SECURITY", "NETWORK");
    LOG_PRINT("%12s %12s %s\n", "BSSID", "CHANNEL", "RSSI");

    for (int a = 0; a < (int)scan_result->scan_count; ++a) {
      bssid = (uint8_t *)&scan_result->scan_info[a].bssid;
      LOG_PRINT("%-24s %4u,  %4u, ",
                scan_result->scan_info[a].ssid,
                scan_result->scan_info[a].security_mode,
                scan_result->scan_info[a].network_type);
      LOG_PRINT("  %02x:%02x:%02x:%02x:%02x:%02x, %4u,  -%u\n",
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
    if (scan_complete_sem != NULL) {
      osSemaphoreRelease(scan_complete_sem);
    }
    return status_code;
  }

  if (result_length != 0) {
    callback_status = show_scan_results(result);
  }

  if (scan_complete_sem != NULL) {
    osSemaphoreRelease(scan_complete_sem);
  }

  return SL_STATUS_OK;
}

int32_t rsi_app_wlan_socket_create()
{
  int32_t status                    = RSI_SUCCESS;
  sl_ipv4_address_t ip              = { 0 };
  struct sockaddr_in server_address = { 0 };
#if HIGH_PERFORMANCE_ENABLE
  uint8_t high_performance_socket = SL_HIGH_PERFORMANCE_SOCKET;
#endif

  //!Create socket
  client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_socket < 0) {
    LOG_PRINT("\r\nSocket creation failed with BSD error: %d\r\n", errno);
    return client_socket;
  }
  LOG_PRINT("\r\nSocket create success : %ld\r\n", client_socket);

  server_address.sin_family = AF_INET;
  server_address.sin_port   = SERVER_PORT;
  sl_net_inet_addr(SERVER_IP_ADDRESS, (uint32_t *)&ip);
  server_address.sin_addr.s_addr = ip.value;
#if HTTPS_DOWNLOAD
  //! Setting SSL socket option
  status = setsockopt(client_socket, SOL_TCP, TCP_ULP, TLS, sizeof(TLS));
  if (status < 0) {
    LOG_PRINT("\r\nSet socket failed with BSD error: %d\r\n", errno);
    status = close(client_socket);
    if (status != 0) {
      LOG_PRINT("\r\nsocket close failed with status = %ld and BSD error: %d\r\n", status, errno);
    } else {
      LOG_PRINT("\r\nsocket close success\r\n");
    }
    return status;
  }
#endif

#if HIGH_PERFORMANCE_ENABLE
  status = setsockopt(client_socket,
                      SOL_SOCKET,
                      SL_SO_HIGH_PERFORMANCE_SOCKET,
                      &high_performance_socket,
                      sizeof(high_performance_socket));
  if (status < 0) {
    LOG_PRINT("\r\nSet Socket option failed with bsd error: %d\r\n", errno);
    status = close(client_socket);
    if (status != 0) {
      LOG_PRINT("\r\nsocket close failed with status = 0x%lX and BSD error: %d\r\n", status, errno);
    } else {
      LOG_PRINT("\r\nsocket close success\r\n");
    }
    return status;
  }
#endif

  //! Connect to server socket
  status = connect(client_socket, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in));
  if (status < 0) {
    LOG_PRINT("\r\nSocket connect failed with BSD error: %d, return value %ld\r\n", errno, status);
    status = close(client_socket);
    if (status != 0) {
      LOG_PRINT("\r\nsocket close failed with status = %ld and BSD error: %d\r\n", status, errno);
    } else {
      LOG_PRINT("\r\nsocket close success\r\n");
    }
    return status;
  } else {
    rsi_wlan_app_cb.state = RSI_WLAN_SOCKET_CONNECTED_STATE;
    LOG_PRINT("\r\nTCP Socket Connect Success\r\n");
  }
  return status;
}

/*====================================================*/
/**
 * @fn         int32_t  rsi_wlan_app_task(void)
 * @brief      Function to work with application task
 * @param[in]  void
 * @return     void
 *=====================================================*/
int32_t rsi_wlan_app_task(void)
{
  int32_t status        = RSI_SUCCESS;
  uint8_t stop_download = 0;
  uint32_t bytes_cnt    = 0;
#if !SOCKET_ASYNC_FEATURE
  static uint32_t download_inprogress = 0;
  int read_bytes                      = 0;
#endif
  uint32_t total_read_bytes = 0;

  while (1) {
    switch (rsi_wlan_app_cb.state) {
      case RSI_POWER_SAVE_STATE: {

      } break;
      case RSI_WLAN_INITIAL_STATE: {
        rsi_wlan_app_callbacks_init(); //! register callback to initialize WLAN
        if (scan_complete_sem == NULL) {
          scan_complete_sem = osSemaphoreNew(1, 0, NULL);
        }
        rsi_wlan_app_cb.state = RSI_WLAN_SCAN_STATE;

#if ENABLE_NWP_POWER_SAVE
        osMutexAcquire(power_cmd_mutex, 0xFFFFFFFFUL);
        if (!powersave_cmd_given) {
          status = rsi_initiate_power_save();
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n failed to keep module in power save \r\n");
            return status;
          }
          powersave_cmd_given = true;
        }
        osMutexRelease(power_cmd_mutex);
        LOG_PRINT("\r\n Module is in deep sleep \r\n");
#endif
      } break;
      case RSI_WLAN_UNCONNECTED_STATE: {
        //! do nothing
      } break;
      case RSI_WLAN_SCAN_STATE: {
        LOG_PRINT("\r\n WLAN scan started \r\n");
#if (WLAN_SCAN_ONLY && WLAN_SYNC_REQ)
        static int8_t wlan_scan_only_check = 1;
        //! unblock other protocol activities
        if (wlan_scan_only_check) {
          osSemaphoreAcquire(sync_coex_wlan_sem, 0);
          if (rsi_ble_running) {
            osSemaphoreRelease(sync_coex_ble_sem);
          }
          wlan_scan_only_check = 0;
        }
#endif

        sl_wifi_scan_configuration_t wifi_scan_configuration = { 0 };
        wifi_scan_configuration                              = default_wifi_scan_configuration;
        scan_complete                                        = false;
        callback_status                                      = SL_STATUS_FAIL;

        sl_wifi_set_scan_callback_v2(wlan_app_scan_callback_handler, NULL);

        status = sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, NULL, &wifi_scan_configuration);
        if (SL_STATUS_IN_PROGRESS == status) {
          LOG_PRINT("Scanning...\r\n");
          if (osSemaphoreAcquire(scan_complete_sem, WIFI_SCAN_TIMEOUT) == osOK) {
            status = callback_status;
          } else {
            status = SL_STATUS_TIMEOUT;
          }
        }
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\n scan failed \r\n");
          break;
        } else {
          rsi_wlan_app_cb.state = RSI_WLAN_JOIN_STATE; //! update WLAN application state to connected state
#if ENABLE_NWP_POWER_SAVE
          LOG_PRINT("\r\n Module is in standby \r\n");
#endif
          LOG_PRINT("\r\n wlan scan done \r\n");
        }
#if WLAN_SCAN_ONLY
        rsi_wlan_app_cb.state = RSI_WLAN_SCAN_STATE;
#endif

      } break;
      case RSI_WLAN_JOIN_STATE: {
        sl_wifi_credential_t cred  = { 0 };
        sl_wifi_credential_id_t id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;

        cred.type = SL_WIFI_PSK_CREDENTIAL;
        memcpy(cred.psk.value, PSK, strlen((char *)PSK));

        status = sl_net_set_credential(id, SL_NET_WIFI_PSK, PSK, strlen((char *)PSK));
        if (SL_STATUS_OK == status) {
          LOG_PRINT("Credentials set, id : %lu\n", id);

          access_point.ssid.length = strlen((char *)SSID);
          memcpy(access_point.ssid.value, SSID, access_point.ssid.length);
          access_point.security      = SECURITY_TYPE;
          access_point.encryption    = SL_WIFI_DEFAULT_ENCRYPTION;
          access_point.credential_id = id;

          LOG_PRINT("\nSSID %s\n", access_point.ssid.value);
          status = sl_wifi_connect(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &access_point, TIMEOUT_MS);
        }
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\nWLAN Connect Failed, Error Code : 0x%lX\r\n", status);
          break;
        } else {
          rsi_wlan_app_cb.state = RSI_WLAN_CONNECTED_STATE; //! update WLAN application state to connected state
          LOG_PRINT("\r\nWLAN connected state \r\n");
        }
      } break;
      case RSI_WLAN_CONNECTED_STATE: {
        //! Configure IP
        ip_address.type      = SL_IPV4;
        ip_address.mode      = SL_IP_MANAGEMENT_DHCP;
        ip_address.host_name = DHCP_HOST_NAME;

        status = sl_si91x_configure_ip_address(&ip_address, SL_SI91X_WIFI_CLIENT_VAP_ID);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\nIP Config failed \r\n");
          break;
        } else {
          rsi_wlan_app_cb.state = RSI_WLAN_IPCONFIG_DONE_STATE;
          sl_ip_address_t ip    = { 0 };
          ip.type               = ip_address.type;
          ip.ip.v4.value        = ip_address.ip.v4.ip_address.value;
          print_sl_ip_address(&ip);
        }
#if (SSL && LOAD_CERTIFICATE)
        status = clear_and_load_certificates_in_flash();
        if (status != RSI_SUCCESS) {
          break;
        }
#endif
      } break;
      case RSI_WLAN_IPCONFIG_DONE_STATE: {
#if (WLAN_CONNECTION_ONLY && WLAN_SYNC_REQ)
        static int8_t wlan_conn_only_check = 1;
        //! unblock other protocol activities
        if (wlan_conn_only_check) {
          if (rsi_ble_running) {
            osSemaphoreRelease(sync_coex_ble_sem);
          }
          wlan_conn_only_check = 0;
        }
        //! Suspend wlan thread
        osSemaphoreAcquire(sync_coex_wlan_sem, osWaitForever);
#endif
        if (stop_download) {
          // http/https file download completed, suspend the common thread
          osThreadSuspend(app_thread_id);
          break;
        }

        if (data_recvd) {
          //! Clear data receive flag
          data_recvd = 0;
#if HTTPS_DOWNLOAD
          LOG_PRINT("\r\nHTTPS download completed, total received bytes: %lu\r\n", total_read_bytes);
#elif !HTTPS_DOWNLOAD
          LOG_PRINT("\r\nHTTP download completed, total received bytes: %lu\r\n", total_read_bytes);
#endif
#if !CONTINUOUS_HTTP_DOWNLOAD
          stop_download = 1;
          break;
#endif
          osDelay(200);
        }
        num_bytes        = 0;
        total_read_bytes = 0;

        //! Create socket and connect to server
        rsi_app_wlan_socket_create();
      } break;
      case RSI_WLAN_SOCKET_CONNECTED_STATE: {
#if WLAN_SYNC_REQ
        //! unblock other protocol activities
        if (rsi_ble_running) {
          osSemaphoreRelease(sync_coex_ble_sem);
        }
#endif
        /* Send first set of HTTP/HTTPS headers to server */
        bytes_cnt = 0;
        while (bytes_cnt != strlen(httpreq)) {
          status = send(client_socket, (const int8_t *)(httpreq + bytes_cnt), (strlen(httpreq) - bytes_cnt), 0);
          if (status < 0) {
            if (errno == ENOBUFS)
              continue;
            LOG_PRINT("\r\n send failed\n");
            status = close(client_socket);
            if (status != 0) {
              LOG_PRINT("\r\nsocket close failed with status = %ld and BSD error: %d\r\n", status, errno);
            } else {
              LOG_PRINT("\r\nsocket close success\r\n");
            }
            rsi_wlan_app_cb.state = RSI_WLAN_IPCONFIG_DONE_STATE;
            break;
          }
          bytes_cnt += status;
        }

        /* Send connection close headers to server */
#if USE_CONNECTION_CLOSE
        bytes_cnt = 0;
        while (bytes_cnt != strlen(http_req_str_connection_close)) {
          status = send(client_socket,
                        (const int8_t *)(http_req_str_connection_close + bytes_cnt),
                        (strlen(http_req_str_connection_close) - bytes_cnt),
                        0);
          if (status < 0) {
            if (errno == ENOBUFS)
              continue;
            LOG_PRINT("\r\n send failed\r\n");
            status = close(client_socket);
            if (status != 0) {
              LOG_PRINT("\r\nsocket close failed with status = 0x%lX and BSD error: %d\r\n", status, errno);
            } else {
              LOG_PRINT("\r\nsocket close success\r\n");
            }
            rsi_wlan_app_cb.state = RSI_WLAN_IPCONFIG_DONE_STATE;
            break;
          }
          bytes_cnt += status;
        }
#endif
        rsi_wlan_app_cb.state = RSI_WLAN_DATA_RECEIVE_STATE;
#if HTTPS_DOWNLOAD
        LOG_PRINT("\r\n HTTPS download started \r\n");
#elif !HTTPS_DOWNLOAD
        LOG_PRINT("\r\n HTTP download started \r\n");
#endif
        break;
      }
      case RSI_WLAN_DATA_RECEIVE_STATE: {
#if !SOCKET_ASYNC_FEATURE
        //Receive data on socket
        read_bytes = recv(client_socket, recv_buffer2, sizeof(recv_buffer2), 0);
        if (read_bytes < 0) {
          if (errno == 0) {
            // get the error code returned by the firmware
            status = sl_wifi_get_saved_firmware_status();
            if (status == SL_STATUS_SI91X_MEMORY_FAILED_FROM_MODULE) {
              continue;
            } else {
              LOG_PRINT("\r\nrecv failed with BSD error = %d and status = 0x%lx\r\n", errno, status);
            }
          } else if ((errno == ENOTCONN) && (download_inprogress)) {
            data_recvd          = 1;
            download_inprogress = 0;
          } else {
            LOG_PRINT("\r\nrecv failed with BSD error = %d\r\n", errno);
          }
          rsi_wlan_app_cb.state = RSI_WLAN_IPCONFIG_DONE_STATE;
          status                = close(client_socket);
          if (status < 0) {
            if (errno == 0) {
              // get the error code returned by the firmware
              status = sl_wifi_get_saved_firmware_status();
              LOG_PRINT("\r\nsocket close failed with status = %ld and BSD error: %d\r\n", status, errno);
            } else {
              LOG_PRINT("\r\nsocket close failed with BSD error: %d\r\n", errno);
            }
          } else {
            LOG_PRINT("\r\nsocket close success\r\n");
          }
          break;
        }
        if (!download_inprogress) {
          download_inprogress = 1;
        }
        total_read_bytes += read_bytes;

#endif
      } break;
      case RSI_WLAN_DISCONNECTED_STATE: {
        rsi_wlan_app_cb.state = RSI_WLAN_JOIN_STATE;
      } break;
      default:
        break;
    }
  }
}

#endif
