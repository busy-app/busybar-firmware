/*******************************************************************************
* @file  wifi_app.c
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
 * @file    wifi_app.c
 * @version 0.1
 * @date    01 Feb 2021
 *
 *  @brief : This file manages the WLAN connection, IP configurations and creation of wifi subtasks
 *
 *  @section Description  This file contains code to handle Module WLAN connections and Throughput/HTTP download task creations
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
#include "sl_si91x_driver.h"

#include "errno.h"
#include "socket.h"

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

/*=======================================================================*/
//   ! GLOBAL VARIABLES
/*=======================================================================*/
osMutexId_t thrput_compute_mutex;
rsi_wlan_app_cb_t rsi_wlan_app_cb; //! application control block

osSemaphoreId_t wlan_throughput_sync_sem[2];
wlan_throughput_config_t wlan_thrput_conf[2] = { 0 };

static volatile bool scan_complete          = false;
static volatile sl_status_t callback_status = SL_STATUS_OK;

sl_wifi_client_configuration_t access_point = { 0 };
sl_net_ip_configuration_t ip_address        = { 0 };
/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/
#if WLAN_SYNC_REQ
extern bool other_protocol_activity_enabled;
extern osSemaphoreId_t sync_coex_ble_sem;
#endif
osSemaphoreId_t wlan_pkt_trnsfer_compl_sem;      //! semaphore to indicate wifi data transfer complete
osSemaphoreId_t wlan_thrghput_measurement_compl; //! semaphore to indicate wifi throughput measurement complete

#if ENABLE_NWP_POWER_SAVE
extern bool powersave_cmd_given;
extern osMutexId_t power_cmd_mutex;
#endif

#if (BLE_THROUGHPUT_TEST)
extern bool rsi_ble_running;
#endif

#if (RSI_ENABLE_BLE_TEST && WLAN_THROUGHPUT_TEST)
extern osSemaphoreId_t ble_wlan_throughput_sync_sem;
uint8_t volatile wlan_tx_throughput_test;
#endif

/*=======================================================================*/
//   ! EXTERN FUNCTIONS
/*=======================================================================*/

/*=======================================================================*/
//   ! PROCEDURES
/*=======================================================================*/

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
    LOG_PRINT("F: Join Event received with %lu bytes payload\n", result_length);
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
  sl_wifi_set_join_callback_v2(join_callback_handler, NULL);
}

#if SSL && LOAD_CERTIFICATE
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
    return status_code;
  }

  if (result_length != 0) {
    callback_status = show_scan_results(result);
  }

  return SL_STATUS_OK;
}

/*====================================================*/
/**
 * @fn         int32_t  rsi_wlan_app_task(void)
 * @brief      Function to work with application task
 * @param[in]  void
 * @return     void
 *=====================================================*/

void rsi_wlan_app_thread(void *unused)
{
  int32_t status = RSI_SUCCESS;
  UNUSED_PARAMETER(unused);

  while (1) {
    switch (rsi_wlan_app_cb.state) {
      case RSI_WLAN_INITIAL_STATE: {
        rsi_wlan_app_callbacks_init();                      //! register callback to initialize WLAN
        rsi_wlan_app_cb.state = RSI_WLAN_UNCONNECTED_STATE; //! update Wi-Fi application state to unconnected state

#if ENABLE_NWP_POWER_SAVE
        osMutexAcquire(power_cmd_mutex, 0xFFFFFFFFUL);
        if (!powersave_cmd_given) {
#ifdef SLI_SI91X_MCU_INTERFACE
          uint8_t xtal_enable = 1;
          status              = sl_si91x_m4_ta_secure_handshake(SL_SI91X_ENABLE_XTAL, 1, &xtal_enable, 0, NULL);
          if (status != SL_STATUS_OK) {
            LOG_PRINT("Failed to bring m4_ta_secure_handshake: 0x%lx\r\n", status);
            return;
          }
          LOG_PRINT("m4_ta_secure_handshake Success\r\n");
#endif
          status = rsi_initiate_power_save();
          if (status != RSI_SUCCESS) {
            LOG_PRINT("Failed to keep module in power save\r\n");
            return;
          }
          powersave_cmd_given = true;
        }
        osMutexRelease(power_cmd_mutex);
        LOG_PRINT("Module is in deep sleep\r\n");
#endif
      } break;
      case RSI_WLAN_UNCONNECTED_STATE: {
        sl_wifi_scan_configuration_t wifi_scan_configuration = { 0 };
        wifi_scan_configuration                              = default_wifi_scan_configuration;
        scan_complete                                        = false;
        callback_status                                      = SL_STATUS_FAIL;

        LOG_PRINT("WLAN scan started \r\n");
        sl_wifi_set_scan_callback_v2(wlan_app_scan_callback_handler, NULL);
        status = sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, NULL, &wifi_scan_configuration);
        if (SL_STATUS_IN_PROGRESS == status) {
          LOG_PRINT("Scanning...\r\n");
          const uint32_t start = osKernelGetTickCount();

          while (!scan_complete && (osKernelGetTickCount() - start) <= WIFI_SCAN_TIMEOUT) {
            osThreadYield();
          }
          status = scan_complete ? callback_status : SL_STATUS_TIMEOUT;
        }
        if (status != RSI_SUCCESS) {
          LOG_PRINT("WLAN Scan failed %lx\r\n", status);
          break;
        } else {
          rsi_wlan_app_cb.state = RSI_WLAN_SCAN_DONE_STATE; //! update WLAN application state to connected state
#if ENABLE_NWP_POWER_SAVE
          LOG_PRINT("Module is in standby \r\n");
#endif
          LOG_PRINT("Scan done state \r\n");
        }

        //! while running BLE throughput test, go to idle state if WLAN_SCAN_ONLY is configured
#if (BLE_THROUGHPUT_TEST)
#if WLAN_SCAN_ONLY
        rsi_wlan_app_cb.state = RSI_WLAN_IDLE_STATE;
#if WLAN_SYNC_REQ
        if (other_protocol_activity_enabled == false) {
          //! unblock other protocol activities
          if (rsi_ble_running) {
            osSemaphoreRelease(sync_coex_ble_sem);
          }
          other_protocol_activity_enabled = true;
        }
#endif
        break;
#endif
#endif
      } break;
      case RSI_WLAN_SCAN_DONE_STATE: {
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

          LOG_PRINT("SSID %s\n", access_point.ssid.value);
          status = sl_wifi_connect(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &access_point, TIMEOUT_MS);
        }
        if (status != RSI_SUCCESS) {
          LOG_PRINT("WLAN connection failed %lx\r\n", status);
          break;
        } else {
          rsi_wlan_app_cb.state = RSI_WLAN_CONNECTED_STATE; //! update WLAN application state to connected state
          LOG_PRINT("WLAN connected state \r\n");
        }
      } break;
      case RSI_WLAN_CONNECTED_STATE: {
        //! Configure IP
        ip_address.type      = SL_IPV4;
        ip_address.mode      = SL_IP_MANAGEMENT_DHCP;
        ip_address.host_name = DHCP_HOST_NAME;

        // Configure IP
        status = sl_si91x_configure_ip_address(&ip_address, SL_SI91X_WIFI_CLIENT_VAP_ID);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("IP Config failed %lx\r\n", status);
          break;
        } else {
          rsi_wlan_app_cb.state = RSI_WLAN_IPCONFIG_DONE_STATE;
          LOG_PRINT("WLAN ipconfig done state \r\n");
          sl_ip_address_t ip = { 0 };
          ip.type            = ip_address.type;
          ip.ip.v4.value     = ip_address.ip.v4.ip_address.value;
          print_sl_ip_address(&ip);
        }
      } break;
      case RSI_WLAN_IPCONFIG_DONE_STATE: {
#if WLAN_SYNC_REQ
        //! start wifi tx/rx after ble start-up activities
#if RSI_ENABLE_BLE_TEST
        osSemaphoreRelease(sync_coex_ble_sem); // BLE will proceed with Adv.
        osDelay(100);
#endif
#endif

#if (RSI_ENABLE_BLE_TEST && WLAN_THROUGHPUT_TEST && WLAN_SYNC_REQ)
        LOG_PRINT("\r\n WLAN thread waiting for BLE activity to complete...\n");
        osSemaphoreAcquire(ble_wlan_throughput_sync_sem, osWaitForever);
#endif

#if BLE_DATA_TRANSFER_START && WLAN_SYNC_REQ
        if (THROUGHPUT_TYPE == TCP_TX || THROUGHPUT_TYPE == SSL_TX || THROUGHPUT_TYPE == UDP_TX) {
          // Block BLE thread in case of wlan tx
          wlan_tx_throughput_test = 1;
        }
#endif

#if WLAN_THROUGHPUT_TEST
        wlan_throughput_task();

#if BLE_DATA_TRANSFER_START && WLAN_SYNC_REQ
        // Release BLE thread if it is waiting to transmit data
        osSemaphoreRelease(ble_wlan_throughput_sync_sem);
        wlan_tx_throughput_test = 0;
#endif
#endif
        rsi_wlan_app_cb.state = RSI_WLAN_IDLE_STATE;
      } break;
      case RSI_WLAN_IDLE_STATE: {
        //! Thread has no work to do in this state, so adding a delay of 5sec
        osDelay(5000);
        break;
      }
      default:
        break;
    }
  }
}

#endif
