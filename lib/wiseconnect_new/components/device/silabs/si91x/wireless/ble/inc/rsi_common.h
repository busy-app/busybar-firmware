/*******************************************************************************
 * @file  rsi_common.h
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef RSI_COMMON_H
#define RSI_COMMON_H

#include "sl_constants.h"

#include "cmsis_os2.h"
#include "sl_si91x_status.h"

#include <rsi_bt_common.h>

#include "rsi_user.h"
#include "rsi_utils.h"
#include "rsi_common_apis.h"

#include <string.h>

/******************************************************
 * *                      Macros
 * ******************************************************/
//SL_PRINTF logging call
#ifndef SL_PRINTF
#define SL_PRINTF(...)
#endif

#define IS_LITTLE_ENDIAN 0
#define IS_BIG_ENDIAN    1

#define RSI_TRUE  1
#define RSI_FALSE 0

// Max packet length of common command responses
//changed cmd len for crypto and PUF
#if defined(SLI_PUF_ENABLE) || (defined RSI_CRYPTO_ENABLE)
#define RSI_COMMON_CMD_LEN 1600
#else
#define RSI_COMMON_CMD_LEN 100
#endif

// Max packet length of ssl rx packet
#define RSI_SSL_RECV_BUFFER_LENGTH 0

// Max packet length of rx packet
#define RSI_DRIVER_RX_PKT_LEN 0
#define SIZE_OF_HEADROOM      0

// Max packet length of BT COMMON tx packet
#define RSI_BT_COMMON_CMD_LEN (300 + SIZE_OF_HEADROOM)
#if ENCODER_IN_RS9116
#define RSI_BT_CLASSIC_CMD_LEN 4000
#else
#define RSI_BT_CLASSIC_CMD_LEN (1040 + SIZE_OF_HEADROOM)
#endif
#define RSI_BLE_CMD_LEN (300 + SIZE_OF_HEADROOM)

#define RSI_DRIVER_POOL_SIZE sizeof(uint32_t) + ((((uint32_t)(sizeof(rsi_driver_cb_t))) + 3) & ~3)

#define RSI_WAIT_TIME RSI_WAIT_FOREVER

#define RSI_TX_EVENT_WAIT_TIME (30000 + TX_WAIT_TIME)
#define DEFAULT_TIMEOUT        RSI_TX_EVENT_WAIT_TIME
#define WAIT_TIMEOOUT          5000

// Internal command timeout defines
#define RSI_BAND_RESPONSE_WAIT_TIME             ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_MAC_RESPONSE_WAIT_TIME              ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_DISCONNECT_RESPONSE_WAIT_TIME       ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_TRANSMIT_RESPONSE_WAIT_TIME         ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_RX_STATS_RESPONSE_WAIT_TIME         ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WMM_RESPONSE_WAIT_TIME              ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_AUTO_CONFIG_RESPONSE_WAIT_TIME      ((500 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_SOCK_CONFIG_RESPONSE_WAIT_TIME      ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_ANTENNA_SEL_RESPONSE_WAIT_TIME      ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_DEBUG_LOG_RESPONSE_WAIT_TIME        ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_GET_RAM_DUMP_RESPONSE_WAIT_TIME     ((1000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_FEATURE_FRAME_RESPONSE_WAIT_TIME    ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_UART_FLOW_CTRL_RESPONSE_WAIT_TIME   ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_PWRMODE_RESPONSE_WAIT_TIME          ((500 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_AES_ENCRYPT_RESPONSE_WAIT_TIME      ((500 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_AES_DECRYPT_RESPONSE_WAIT_TIME      ((500 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_AES_MAC_RESPONSE_WAIT_TIME          ((500 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_SEND_DATA_RESPONSE_WAIT_TIME        ((500 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_SWITCH_PROTO_RESPONSE_WAIT_TIME     ((5000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_RAW_DATA_RESPONSE_WAIT_TIME         ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_GAIN_TABLE_RESPONSE_WAIT_TIME       ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_GET_PROFILE_RESPONSE_WAIT_TIME      ((1000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_SET_PROFILE_RESPONSE_WAIT_TIME      ((1000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_EAP_RESPONSE_WAIT_TIME              ((5000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_REGION_RESPONSE_WAIT_TIME           ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_REGION_AP_RESPONSE_WAIT_TIME        ((5000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_DYNAMIC_RESPONSE_WAIT_TIME          ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_ROAMING_RESPONSE_WAIT_TIME          ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_MULTICAST_FIL_RESPONSE_WAIT_TIME    ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_HT_CAPS_RESPONSE_WAIT_TIME          ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_MULTICAST_RESPONSE_WAIT_TIME        ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_BCAST_RESPONSE_WAIT_TIME            ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_CARD_READY_WAIT_TIME                ((5000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_INIT_RESPONSE_WAIT_TIME             ((3000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WLAN_CONFIG_WAIT_TIME               ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WLAN_GET_RANDOM_WAIT_TIME           ((500 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WLAN_RSSI_RESPONSE_WAIT_TIME        ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WLAN_QUERY_NETWORK_PARAMS_WAIT_TIME ((200 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WLAN_REQ_GET_CFG_WAIT_TIME          ((200 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WLAN_CFG_SAVE_WAIT_TIME             ((1000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_BOARD_READY_WAIT_TIME               ((50000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_SET_SLEEP_TIMER_RESPONSE_WAIT_TIME  ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_DEINIT_RESPONSE_WAIT_TIME           ((5000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_CERTIFICATE_RESPONSE_WAIT_TIME      ((1000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_REJOIN_RESPONSE_WAIT_TIME           ((500 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_USER_SC_RESPONSE_WAIT_TIME          ((5000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WLAN_REQ_GET_STATS_WAIT_TIME        ((500 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_AUTO_JOIN_RESPONSE_WAIT_TIME \
  ((60000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT)) //! Here timeout should wait for init,scan and join commands
#define RSI_DELETE_PROFILE_RESPONSE_WAIT_TIME   ((1000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_IAP_RESPONSE_WAIT_TIME              ((5000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_IAP_GET_CERT_RESPONSE_WAIT_TIME     ((5000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_IAP_GEN_SIG_RESPONSE_WAIT_TIME      ((5000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_TA_M4_COMMAND_RESPONSE_WAIT_TIME    ((500 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_ADD_MFI_IE_RESPONSE_WAIT_TIME       ((500 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_FILTER_BROADCAST_RESPONSE_WAIT_TIME ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_SET_RTC_TIMER_RESPONSE_WAIT_TIME    ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#ifdef RSI_ASSERT_API
#define RSI_ASSERT_RESPONSE_WAIT_TIME ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#endif
#define RSI_WLAN_RADIO_RESPONSE_WAIT_TIME      ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_GET_RTC_TIMER_RESPONSE_WAIT_TIME   ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WLAN_TCP_WINDOW_RESPONSE_WAIT_TIME ((5000 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_TIMEOUT_RESPONSE_WAIT_TIME         ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_SET_CONFIG_RESPONSE_WAIT_TIME      ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
// WIFI WAIT timeout defines
#define RSI_SCAN_RESPONSE_WAIT_TIME     ((10000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_JOIN_RESPONSE_WAIT_TIME     ((10000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_P2P_RESPONSE_WAIT_TIME      ((1000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_BGSCAN_RESPONSE_WAIT_TIME   ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_PING_RESPONSE_WAIT_TIME     ((60000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WEP_RESPONSE_WAIT_TIME      ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_IPCONFV4_RESPONSE_WAIT_TIME ((20000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_IPCONFV6_RESPONSE_WAIT_TIME ((20000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_MDNSD_RESPONSE_WAIT_TIME \
  ((250000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT)) //!Firmware max timeout value is 240 seconds
#define RSI_HTTP_RESPONSE_WAIT_TIME \
  ((110000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT)) //!Firmware max timeout value is 100 seconds
#define RSI_AP_CONFIG_RESPONSE_WAIT_TIME        ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_FWUP_RESPONSE_WAIT_TIME             ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_MTYPE_RESPONSE_WAIT_TIME            ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WP_LOAD_RESPONSE_WAIT_TIME          ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_JSON_LOAD_RESPONSE_WAIT_TIME        ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WP_ERASE_RESPONSE_WAIT_TIME         ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_JSON_ERASE_RESPONSE_WAIT_TIME       ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WP_CLEAR_ALL_RESPONSE_WAIT_TIME     ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_CONNECT_RESPONSE_WAIT_TIME          ((10000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WPS_RESPONSE_WAIT_TIME              ((180000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_FREQ_OFFSET_WAIT_TIME               ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_CALIB_WRITE_WAIT_TIME               ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_SMTP_RESPONSE_WAIT_TIME             ((60000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_DNS_SERVER_ADD_RESPONSE_WAIT_TIME   ((150000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_FTP_RESPONSE_WAIT_TIME              ((120000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_OTA_FWUP_RESPONSE_WAIT_TIME         ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_CRYPTO_RESPONSE_WAIT_TIME           ((2000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_SOCKET_CREATE_RESPONSE_WAIT_TIME    ((100000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_CERT_RESPONSE_WAIT_TIME             ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_SCAN_WITH_BITMAP_RESPONSE_WAIT_TIME ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_DNS_QUERY_RESPONSE_WAIT_TIME        ((150000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_DNS_UPDATE_RESPONSE_WAIT_TIME       ((150000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_HOST_WP_SEND_RESPONSE_WAIT_TIME     ((200000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_POP3_RESPONSE_WAIT_TIME             ((60000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_HTTP_ABORT_RESPONSE_WAIT_TIME \
  ((100000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT)) //!Firmware max timeout value is 100 seconds
#define RSI_HTTP_CLIENT_PUT_RESPONSE_WAIT_TIME \
  ((100000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT)) //!Firmware max timeout value is 100 seconds
#define RSI_SNTP_RESPONSE_WAIT_TIME         ((100000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_EMB_MQTT_RESPONSE_WAIT_TIME     ((60000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_PSK_RESPONSE_WAIT_TIME          ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_SOCKET_CLOSE_RESPONSE_WAIT_TIME ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WLAN_11AX_WAIT_TIME             ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WLAN_TWT_RESPONSE_WAIT_TIME     ((5000 * WIFI_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))

// WIFI BLOCKED timeout defines
#define RSI_ACCEPT_RESPONSE_WAIT_TIME          (RSI_WAIT_FOREVER * WIFI_BLOCKED_TIMEOUT_SF)
#define RSI_SELECT_RESPONSE_WAIT_TIME          ((RSI_WAIT_FOREVER * WIFI_BLOCKED_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_SOCKET_RECVFROM_RESPONSE_WAIT_TIME ((RSI_WAIT_FOREVER * WIFI_BLOCKED_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_GPIO_WAIT_TIME                     10000
#define RSI_RX_EVENT_WAIT_TIME                 DEFAULT_TIMEOUT
#define RSI_COMMON_SEND_CMD_RESPONSE_WAIT_TIME ((250000 * WIFI_BLOCKED_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_WLAN_SEND_CMD_RESPONSE_WAIT_TIME   ((250000 * WIFI_BLOCKED_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_NWK_SEND_CMD_RESPONSE_WAIT_TIME    ((250000 * WIFI_BLOCKED_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_GPIO_CONFIG_RESP_WAIT_TIME         ((100 * WIFI_INTERNAL_TIMEOUT_SF) + (DEFAULT_TIMEOUT))

//BT/BLE command timeouts in ms
#define RSI_BT_COMMON_CMD_RESP_WAIT_TIME    ((500 * BT_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_BT_A2DP_CMD_RESP_WAIT_TIME      ((500 * BT_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_BT_A2DP_DATA_CMD_RESP_WAIT_TIME ((2000 * BT_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_BT_AVRCP_CMD_RESP_WAIT_TIME     ((200 * BT_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_BT_HFP_CMD_RESP_WAIT_TIME       ((500 * BT_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_BT_PBAP_CMD_RESP_WAIT_TIME      ((500 * BT_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_BT_HID_CMD_RESP_WAIT_TIME       ((500 * BT_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_BT_SPP_CMD_RESP_WAIT_TIME       ((500 * BT_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))

#define RSI_BLE_GAP_CMD_RESP_WAIT_TIME  ((500 * BLE_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define RSI_BLE_GATT_CMD_RESP_WAIT_TIME ((200 * BLE_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))

#define RSI_BT_BLE_CMD_MAX_RESP_WAIT_TIME ((2000 * BT_COMMON_WAIT_TIMEOUT_SF) + (DEFAULT_TIMEOUT))

// Module type
typedef enum {
  RSI_MODULE_TYPE_Q7   = 1,
  RSI_MODULE_TYPE_SB   = 4,
  RSI_MODULE_TYPE_M7DB = 5,
  RSI_MODULE_TYPE_M4SB = 6,
  RSI_MODULE_TYPE_WMS  = 10
} module_type;

//Antenna Output power in dBm
#define RSI_MIN_OUTPUT_POWER_IN_DBM -8
#define RSI_MAX_OUTPUT_POWER_IN_DBM 15

/******************************************************/

/******************************************************
 * *                    Constants
 * ******************************************************/
/******************************************************
 * *                   Enumerations
 * ******************************************************/
// enumeration for states used in initial control block in driver
typedef enum rsi_device_state_e {
  RSI_DEVICE_STATE_NONE = 0,
  RSI_DRIVER_INIT_DONE,
  RSI_DEVICE_INIT_DONE
} rsi_device_state_t;

#define RSI_DMA_VALID      0
#define RSI_SKIP_DMA_VALID 1
//#define DEBUGOUT(...)

#define BT_CMD 6

/******************************************************
 * *                 Type Definitions
 * ******************************************************/

/******************************************************
 * *                    Structures
 * ******************************************************/

// driver control block structure

/**
 * @brief Structure representing the driver control block.
 *
 * This structure is used to define the parameters for the driver control block,
 * that includes endianness, global Bluetooth callback, common Bluetooth callback,
 * BLE callback, and device state.
 */
typedef struct rsi_driver_cb_s {
  /** Endianness */
  uint8_t endian;

  /** Global Bluetooth callback */
  rsi_bt_global_cb_t *bt_global_cb;
  /** Common Bluetooth callback */
  rsi_bt_cb_t *bt_common_cb;
  /** BLE callback */
  rsi_bt_cb_t *ble_cb;

#if defined(SL_SI91X_PRINT_DBG_LOG)
  /** Mutex for debug prints */
  osMutexId_t debug_prints_mutex;
#endif

  /** Device state */
  volatile rsi_device_state_t device_state;
} rsi_driver_cb_t;

extern rsi_driver_cb_t *rsi_driver_cb;

/******************************************************
 * *                 Global Variables
 * ******************************************************/
/******************************************************
 * *               Function Declarations
 * ******************************************************/
#endif
