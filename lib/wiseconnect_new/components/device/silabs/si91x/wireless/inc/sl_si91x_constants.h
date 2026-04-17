/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#pragma once

#include <stdint.h>
#include "sl_wifi_constants.h"
#include "sli_wifi_constants.h"
#include "sl_constants.h"
/** \addtogroup SI91X_LOAD_IMAGE_TYPES
  * @{ */
/// Load the default NWP firmware at location 0.
#define LOAD_NWP_FW 0x31

/// Load the NWP firmware using the image number (@ref SI91X_NWP_FW_IMAGE_NUMBERS)
/// @note This is ONLY applicable in System on Chip (SoC) mode for Si91x devices.
#define SL_SI91X_LOAD_NWP_FW_WITH_IMAGE_NUMBER 0x41

/// Load default NWP firmware active low.
/// @note This is ONLY applicable in Network Co-Processor (NCP) mode for Si91x devices.
#define LOAD_DEFAULT_NWP_FW_ACTIVE_LOW 0x71

/// Burn NWP firmware image
#define BURN_NWP_FW 'B'

/// Burn M4 firmware image
#define BURN_M4_FW '4'
/** @} */

/** \addtogroup SI91X_NWP_FW_IMAGE_NUMBERS
  * @{ */

#define SL_SI91X_NWP_FW_IMAGE_NUMBER_0 0 ///< Image number for the first NWP firmware image
#define SL_SI91X_NWP_FW_IMAGE_NUMBER_1 1 ///< Image number for the second NWP firmware image

/** @} */

/** \addtogroup SI91X_TLS_DEFINE
  * \ingroup SL_SI91X_CONSTANTS
  * @{ */
/*=========================================================================*/
// TLS feature parameters description !//
/*=========================================================================*/
/// Bit to enable SSL feature
#define SL_SI91X_ENABLE_TLS BIT(0)

/// Bitmap to enable TLS version 1.0
#define SL_SI91X_TLS_V_1_0 BIT(2)

/// Bitmap to enable TLS version 1.2
#define SL_SI91X_TLS_V_1_2 BIT(3)

/// Bitmap to enable TLS version 1.1
#define SL_SI91X_TLS_V_1_1 BIT(4)

#if defined(SLI_SI917) || defined(DOXYGEN)
/// Bitmap to enable TLS version 1.3
#define SL_SI91X_TLS_V_1_3 BIT(8)
#endif

/// Bitmap to enable DTLS version 1.0
#define SL_SI91X_DTLS_V_1_0 BIT(2)

/// Bitmap to enable DTLS version 1.2
#define SL_SI91X_DTLS_V_1_2 BIT(3)

/// Bit to enable DTLS feature
#define SL_SI91X_ENABLE_DTLS BIT(13)

/** @} */

/** \addtogroup SI91X_HTTP_CLIENT_DEFINE
  * \ingroup SL_SI91X_CONSTANTS
  * @{ */
/*=========================================================================*/
// HTTP Client feature parameters description !//
/*=========================================================================*/
/// Bit to enable NULL delimiter for HTTP buffer instead of comma
#define SL_SI91X_ENABLE_NULL_DELIMETER BIT(1)

/// HTTP client post big data support feature bitmap
#define SL_SI91X_SUPPORT_HTTP_POST_DATA BIT(5)

/// HTTP version 1.1 support feature bitmap
#define SL_SI91X_HTTP_V_1_1 BIT(6)

/// Bit to enable user given content type in extended header
#define SL_SI91X_HTTP_USER_DEFINED_CONTENT_TYPE BIT(7)

/// To specify index of SSL cert to be used for HTTPS, for index 0 leave them unset
#define SL_SI91X_HTTPS_CERTIFICATE_INDEX_1 BIT(9)
/// To specify index of SSL cert to be used for HTTPS, for index 0 leave them unset
#define SL_SI91X_HTTPS_CERTIFICATE_INDEX_2 BIT(10)

/// To enable SNI
#define SL_SI91X_HTTPS_USE_SNI BIT(11)
/** @} */

//! @cond Doxygen_Suppress
// M4 FW image number for Bootloader
#define SLI_M4_FW_IMAGE_NUMBER (1 << 8)

// Bootloader selects default NWP FW image number
#define SLI_SELECT_DEFAULT_NWP_FW_IMAGE_NUMBER '5'

#define SLI_PING_PONG_CHUNK_SIZE 4096
#define SLI_REG_READ             0xD1
#define SLI_REG_WRITE            0xD2
#define SLI_SEND_RPS_FILE        '2'
#define SLI_FWUP_SUCCESSFUL      'S'
#define SLI_EOF_REACHED          'E'
#define SLI_PONG_VALID           'O'
#define SLI_PING_VALID           'I'
#define SLI_PONG_WRITE           0xD4
#define SLI_PING_WRITE           0xD5
#define SLI_PONG_AVAIL           'O'
#define SLI_PING_AVAIL           'I'

#define SLI_PING_BUFFER_ADDR 0x418000
#define SLI_PONG_BUFFER_ADDR 0x419000

#define SLI_ACTIVE_LOW_INTR  0x2
#define SLI_ACTIVE_HIGH_INTR 0x0

#define SLI_OPERMODE_RESPONSE_WAIT_TIME      (1000) // Milliseconds
#define SLI_SEND_RAW_DATA_RESPONSE_WAIT_TIME (1000) // Milliseconds

//STM 32 Init Sequence
#define SLI_SI91X_INIT_CMD 0x005c4a12

// SPI transfer type (8 BIT/32 BIT)
#define SLI_MODE_8BIT  0
#define SLI_MODE_32BIT 1

// frame descriptor length
#define SLI_FRAME_DESC_LEN 16

// Bus interrupt register values
// Wi-Fi buffer full indication register value from NWP module
#ifndef SLI_WIFI_BUFFER_FULL
#define SLI_WIFI_BUFFER_FULL (1 << 0)
#endif

// BLE buffer full indication register value from NWP module
#ifndef SLI_BLE_BUFFER_FULL
#define SLI_BLE_BUFFER_FULL (1 << 4)
#endif
// Wi-Fi buffer empty indication register value from NWP module
#define SLI_BUFFER_EMPTY 0x02
// RX packet pending register value from NWP module
#define SLI_RX_PKT_PENDING 0x08
// Assertion Interrupt indication from NWP module
#define SLI_ASSERT_INTR 0x80

//Bootloader defines
#define SLI_SI91X_SAFE_UPGRADE_ADDR        0x55408
#define SLI_SI91X_SAFE_UPGRADE             BIT(12)
#define SLI_SI91X_FW_START_OF_FILE         BIT(0)
#define SLI_SI91X_FW_END_OF_FILE           BIT(1)
#define SLI_SI91X_MIN_CHUNK_SIZE           4096
#define SLI_SI91X_LOOP_COUNT_UPGRADE_IMAGE 0xFFFF
#define SLI_SI91X_RESET_LOOP_COUNTER(X)    X = 0;
#define SLI_SI91X_WHILE_LOOP(X, Y)         while ((X++) < (int32_t)Y)
#define SLI_SI91X_CHECK_LOOP_COUNTER(X, Y) \
  {                                        \
    if (X >= Y)                            \
      return -1;                           \
  }

//***************************** Macros for Crypto Start **********************************/

#define SL_SI91X_KEY_BUFFER_SIZE 32
#define SL_SI91X_TAG_SIZE        16
#define SL_SI91X_IV_SIZE         16

// ECDH defines
#define SLI_ECDH_BUFFER_SIZE 32

// TRNG defines
#define SLI_TRNG_INIT       1
#define SLI_TRNG_ENTROPY    2
#define SLI_TRNG_KEY        3
#define SLI_TRNG_GENERATION 4

#define SLI_TRNG_INIT_MSG_LENGTH 16
#define TRNG_KEY_SIZE            4
#define SLI_TRNG_TEST_DATA_SIZE  64

// CCM defines
#define SLI_SI91X_CCM_IV_MIN_SIZE  7
#define SLI_SI91X_CCM_IV_MAX_SIZE  13
#define SLI_SI91X_CCM_IV_BUFF_LEN  16  // SLI_SI91X_CCM_IV_MAX_SIZE + 3 Bytes for Padding.
#define SLI_SI91X_CCM_AD_MAX_SIZE  128 // Limiting header length to 128.
#define SLI_SI91X_CCM_MSG_MAX_SIZE 1200

// GCM defines
#define SLI_SI91X_GCM_IV_SIZE      12
#define SLI_SI91X_GCM_AD_MAX_SIZE  128
#define SLI_SI91X_GCM_MSG_MAX_SIZE 1400

// Chachapoly defines
#define SLI_SI91X_KEYR_SIZE               16
#define SLI_SI91X_KEYS_SIZE               16
#define SLI_SI91X_CHACHAPOLY_IV_SIZE      12
#define SLI_SI91X_CHACHAPOLY_MSG_MAX_SIZE 1200

//ECDSA defines
#define SLI_SI91X_PRIVATE_KEY_MAX_SIZE 64
#define SLI_SI91X_PUBLIC_KEY_MAX_SIZE  136
#define SLI_SI91X_SIGNATURE_MAX_SIZE   128
#define SLI_SI91X_ECDSA_MSG_MAX_SIZE   1000

// NWP Configuration defines
#define SL_SI91X_XO_CTUNE_FROM_HOST           BIT(0)
#define SL_SI91X_ENABLE_NWP_WDT_FROM_HOST     BIT(1)
#define SL_SI91X_DISABLE_NWP_WDT_FROM_HOST    BIT(2)
#define SL_SI91X_SET_XTAL_GOOD_TIME_FROM_HOST BIT(3)
#define SL_SI91X_SET_PMU_GOOD_TIME_FROM_HOST  BIT(4)
//! @endcond
/** \addtogroup SL_SI91X_CONSTANTS 
  * @{ */
#define GET_OPN_BOARD_CONFIG 1 ///< Get board OPN board configuration
#define GET_NWP_TIMESTAMP    2 ///< Get NWP timestamp
/** @} */

//***************************** Macros for Crypto End **********************************/
//! @cond Doxygen_Suppress
// Command packet 'unused' bytes
#define SLI_SI91X_COMMAND_FLAGS_INDEX    10
#define SLI_SI91X_COMMAND_RESPONSE_INDEX 11

typedef struct {
  uint32_t buffer_full : 1;
  uint32_t ready_to_transmit : 1;
  uint32_t _reserved2 : 1;
  uint32_t rx_packet_pending : 1;
  uint32_t _reserved4 : 1;
  uint32_t _reserved5 : 1;
  uint32_t _reserved6 : 1;
  uint32_t _reserved7 : 1;
} sli_si91x_interrupt_status_t;

// Timeout used in get_channel API
#define SL_SI91X_GET_CHANNEL_TIMEOUT 30200

/// Timeout used in get_interface_info API (milliseconds)
#define SL_SI91X_GET_INTERFACE_INFO_TIMEOUT 1000

//! @endcond

/** \addtogroup SL_SI91X_CONSTANTS 
  * @{ */

/// Si91x specific keepalive types.
#define SL_SI91X_AP_KEEP_ALIVE_DISABLE SL_WIFI_AP_KEEP_ALIVE_DISABLE ///< Disable keepalive functionality.
#define SL_SI91X_AP_DEAUTH_BASED_KEEP_ALIVE \
  SL_WIFI_AP_DEAUTH_BASED_KEEP_ALIVE ///< AP performs keepalive functionality based on the RX packets received from its stations.
///< If no packet is received from the station within the AP keep alive timeout period, the AP disconnects the station.
#define SL_SI91X_AP_NULL_BASED_KEEP_ALIVE \
  SL_WIFI_AP_NULL_BASED_KEEP_ALIVE ///< AP performs keepalive functionality by sending a NULL DATA packet to the station.
///< If no ACK is received from the station after a specific number of retries, the AP discards the station.

/// Si91x specific keepalive types
typedef sl_wifi_ap_keepalive_type_t SL_DEPRECATED_API_WISECONNECT_4_0
  sl_si91x_ap_keepalive_type_t; ///< Si91x specific keepalive types

/// Assertion type must be in the range of 0 to 15 (both included)
typedef enum {
  SL_SI91X_ASSERTION_TYPE_LMAC = 0, ///< Assertion type specific to the LMAC core.
  SL_SI91X_ASSERTION_TYPE_SME  = 1, ///< Assertion type specific to the SME (Station Management Entity) core.
  SL_SI91X_ASSERTION_TYPE_UMAC = 2, ///< Assertion type specific to the UMAC core.
  SL_SI91X_ASSERTION_TYPE_NETX = 4, ///< Assertion type specific to the NETX (Networking Stack) core.
  SL_SI91X_ASSERTION_TYPE_CA =
    8, ///< Enables critical assertion indication and provides a RAM dump during critical assertions.
  SL_SI91X_ASSERTION_TYPE_ALL = 15 ///< Enables assertion for all cores.
} sl_si91x_assertion_type_t;

/// Assertion level must be in the range of 0 to 15 (both included)
typedef enum {
  SL_SI91X_ASSERTION_LEVEL_MIN = 0, ///< Minimum assertion level. Indicates that an assertion is mandatory.
  SL_SI91X_ASSERTION_LEVEL_SP  = 1, ///< Assertion for specific print messages, used for debugging or analysis.
  SL_SI91X_ASSERTION_LEVEL_REC =
    2, ///< Recoverable assertion level. Indicates that the system can recover from the assertion.
  SL_SI91X_ASSERTION_LEVEL_INFO = 4, ///< Informational assertion level, used to log general information.
  SL_SI91X_ASSERTION_LEVEL_MAX  = 15 ///< Maximum assertion level. Enables all types of print statements.
} sl_si91x_assertion_level_t;
/** @} */

//! @cond Doxygen_Suppress
typedef enum {
  // (7+/-1)dBm in 2.4GHz band
  // (5+/-1)dBm in 5GHz band
  SLI_SI91X_LOW_POWER_LEVEL,

  // (5+/-1)dBm in 2.4GHz band
  // (7+/-1)dBm in 5GHz band
  SLI_SI91X_MEDIUM_POWER_LEVEL,

  SLI_SI91X_HIGH_POWER_LEVEL
} sli_si91x_transmit_power_level_t;

/// This macro is an alias for @ref SLI_WIFI_WAIT_FOR_EVER and sets the wait period bit flag to indicate infinite wait time.
/// This macro is deprecated, use SL_NET_WAIT_FOREVER instead.
#define SLI_SI91X_WAIT_FOR_EVER SLI_WIFI_WAIT_FOR_EVER
/*====================================================*/
// Constant Defines
// SPI Status
#define SLI_SPI_SUCCESS       0x58
#define SLI_SPI_BUSY          0x54
#define SLI_SPI_FAIL          0x52
#define RSI_SUCCESS           0
#define SLI_ERROR_BUFFER_FULL -3 // module buffer full  error code
#define SLI_ERROR_IN_SLEEP    -4 // module in sleep error code

//SPI Internal Register Offset
#define SLI_SPI_INT_REG_ADDR    0x00 //@ register access method
#define SLI_SPI_MODE_REG_ADDR   0x08 //@ register access method
#define SLI_SPI_LENGTH_REG_ADDR 0x20

#define SLI_INT_MASK_REG_ADDR 0x41050000 // Interrupt mask register
#define SLI_INT_CLR_REG_ADDR  0x22000010 // Interrupt clear register

// Packet queue identifiers
#define SLI_COMMON_Q    0
#define SLI_ZB_Q        1
#define SLI_BT_Q        2
#define SLI_WLAN_MGMT_Q 4
#define SLI_WLAN_DATA_Q 5
#ifdef SAPIS_BT_STACK_ON_HOST
#define SLI_BT_INT_MGMT_Q 6
#define SLI_BT_HCI_Q      7
#endif
#define SLI_LOG_Q 8

// Event IDs
#define SLI_RX_EVENT       0 // RX event number used in the driver
#define SLI_TX_EVENT       1 // TX event number used in the driver
#define SLI_SOCKET_EVENT   2 // Socket event number used in the driver
#define SLI_MAX_NUM_EVENTS 3 // Max number events used in the driver

#define SLI_HOST_INTF_REG_OUT    0x4105003C
#define SLI_HOST_INTF_REG_IN     0x41050034
#define SLI_HOST_INTF_STATUS_REG 0x41050004

// si91x boot results
#define SLI_EOF_REACHED                          'E'
#define SLI_BOOTUP_OPTIONS_LAST_CONFIG_NOT_SAVED 0xF1
#define SLI_BOOTUP_OPTIONS_CHECKSUM_FAIL         0xF2
#define SLI_INVALID_OPTION                       0xF3
#define SLI_CHECKSUM_SUCCESS                     0xAA
#define SLI_CHECKSUM_FAILURE                     0xCC
#define SLI_CHECKSUM_INVALID_ADDRESS             0x4C
#define SLI_VALID_FIRMWARE_NOT_PRESENT           0x23
#define SLI_BOOTLOADER_VERSION_1P0               0x10
#define SLI_BOOTLOADER_VERSION_1P1               0x11
#define SLI_ROM_VERSION_1P0                      1
#define SLI_ROM_VERSION_1P1                      2

#define SLI_WIFI_REGISTER_VALID (0xAB)

#define SLI_HOST_INTERACT_REG_VALID    (0xAB << 8)
#define SLI_HOST_INTERACT_REG_VALID_FW (0xA0 << 8)

#ifndef SLI_CONFIG_RTSTHRESHOLD
#define SLI_CONFIG_RTSTHRESHOLD 1
#endif

#ifndef SLI_RTS_THRESHOLD
#define SLI_RTS_THRESHOLD 2346
#endif

#define SLI_SEND_RAW_DATA                    0x1
#define SLI_RECEIVE_RAW_DATA                 0x0
#define SLI_NET_DUAL_STACK_RX_RAW_DATA_FRAME 0x1 // Rx data frame type in network dual stack mode

// enumeration for command request used in common control block
typedef enum {
  // Common command requests
  SLI_COMMON_REQ_OPERMODE      = 0x10,
  SLI_COMMON_REQ_FEATURE_FRAME = 0xC8,
  SLI_COMMON_REQ_PWRMODE       = 0x15,
  // Reusing SLI_WLAN_REQ_FW_VERSION as SLI_COMMON_REQ_FW_VERSION
  SLI_COMMON_REQ_FW_VERSION     = 0x49,
  SLI_COMMON_REQ_GET_EFUSE_DATA = 0xA0,

  // Unimplemented common command requests
  SLI_COMMON_REQ_ENCRYPT_CRYPTO        = 0x76,
  SLI_COMMON_REQ_UART_FLOW_CTRL_ENABLE = 0xA4,
  SLI_COMMON_REQ_TA_M4_COMMANDS        = 0xB0,
  SLI_COMMON_REQ_DEBUG_LOG             = 0x26
#ifdef SLI_WAC_MFI_ENABLE
  ,
  SLI_COMMON_REQ_IAP_GET_CERTIFICATE   = 0xB6,
  SLI_COMMON_REQ_IAP_INIT              = 0xB7,
  SLI_COMMON_REQ_IAP_GENERATE_SIGATURE = 0xB8
#endif
  ,
  SLI_COMMON_REQ_SWITCH_PROTO         = 0x77,
  SLI_COMMON_REQ_GET_RAM_DUMP         = 0x92,
  SLI_COMMON_REQ_ASSERT               = 0xE1,
  SLI_COMMON_REQ_SET_RTC_TIMER        = 0xE9,
  SLI_COMMON_REQ_GET_RTC_TIMER        = 0xF2,
  SLI_COMMON_REQ_SET_CONFIG           = 0xBA,
  SLI_COMMON_REQ_GET_CONFIG           = 0x0C,
  SLI_SI91X_FW_FALLBACK_REQ_FROM_HOST = 0x2C,
#ifdef CONFIGURE_GPIO_FROM_HOST
  ,
  SLI_COMMON_REQ_GPIO_CONFIG = 0x28
#endif
} sli_common_cmd_request_t;

typedef enum {
  // Common command responses
  SLI_COMMON_RSP_OPERMODE      = 0x10,
  SLI_COMMON_RSP_FEATURE_FRAME = 0xC8,
  SLI_COMMON_RSP_CARDREADY     = 0x89,
  SLI_COMMON_RSP_PWRMODE       = 0x15,

  // Unimplemented common command responses
  SLI_COMMON_RSP_CLEAR                 = 0x00,
  SLI_COMMON_RSP_ULP_NO_RAM_RETENTION  = 0xCD,
  SLI_COMMON_RSP_ASYNCHRONOUS          = 0xFF,
  SLI_COMMON_RSP_ENCRYPT_CRYPTO        = 0x76,
  SLI_COMMON_RSP_UART_FLOW_CTRL_ENABLE = 0xA4,
  SLI_COMMON_RSP_TA_M4_COMMANDS        = 0xB0,
  SLI_COMMON_RSP_DEBUG_LOG             = 0x26

#ifdef SLI_WAC_MFI_ENABLE
  ,
  SLI_COMMON_RSP_IAP_GET_CERTIFICATE   = 0xB6,
  SLI_COMMON_RSP_IAP_INIT              = 0xB7,
  SLI_COMMON_RSP_IAP_GENERATE_SIGATURE = 0xB8
#endif
  // Reusing SLI_WLAN_REQ_FW_VERSION as SLI_COMMON_REQ_FW_VERSION
  ,
  SLI_COMMON_RSP_GET_EFUSE_DATA       = 0xA0,
  SLI_COMMON_RSP_FW_VERSION           = 0x49,
  SLI_COMMON_RSP_SWITCH_PROTO         = 0x77,
  SLI_COMMON_RSP_GET_RAM_DUMP         = 0x92,
  SLI_COMMON_RSP_ASSERT               = 0xE1,
  SLI_COMMON_RSP_SET_RTC_TIMER        = 0xE9,
  SLI_COMMON_RSP_GET_RTC_TIMER        = 0xF2,
  SLI_COMMON_RSP_SET_CONFIG           = 0xBA,
  SLI_COMMON_RSP_GET_CONFIG           = 0x0C,
  SLI_SI91X_FW_FALLBACK_RSP_FROM_HOST = 0x2C,
#ifdef CONFIGURE_GPIO_FROM_HOST
  ,
  SLI_COMMON_RSP_GPIO_CONFIG = 0x28
#endif
} sli_common_cmd_response_t;

// enumeration for WLAN command request codes
typedef enum {
  // Wi-Fi commands
  SLI_WLAN_REQ_SET_REGION           = 0x1D,
  SLI_WLAN_REQ_EVM_OFFSET           = 0x36,
  SLI_WLAN_REQ_EVM_WRITE            = 0x37,
  SLI_WLAN_REQ_FW_VERSION           = 0x49,
  SLI_WLAN_REQ_SET_CERTIFICATE      = 0x4D,
  SLI_WLAN_REQ_WPS_METHOD           = 0x72,
  SLI_WLAN_REQ_EFUSE_READ           = 0x73,
  SLI_WLAN_REQ_ROAM_PARAMS          = 0x7B,
  SLI_WLAN_REQ_RX_STATS             = 0xA2,
  SLI_WLAN_REQ_RADIO                = 0x81,
  SLI_WLAN_REQ_TWT_AUTO_CONFIG      = 0x2E,
  SLI_WLAN_REQ_TWT_PARAMS           = 0x2F,
  SLI_WLAN_REQ_GAIN_TABLE           = 0x47,
  SLI_WLAN_REQ_SET_REGION_AP        = 0xBD,
  SLI_WLAN_REQ_CALIB_WRITE          = 0xCA,
  SLI_WLAN_REQ_FILTER_BCAST_PACKETS = 0xC9,
  SLI_WLAN_REQ_CALIB_READ           = 0xCF,
  SLI_WLAN_REQ_FULL_FW_VERSION      = 0xE0,
  SLI_WLAN_REQ_HTTP_OTAF            = 0xF4,
  SLI_WLAN_REQ_11AX_PARAMS          = 0xFF,

  // Network commands
  SLI_WLAN_REQ_PING_PACKET           = 0x29,
  SLI_WLAN_REQ_NAT                   = 0x2A,
  SLI_WLAN_REQ_IPCONFV4              = 0x41,
  SLI_WLAN_REQ_DNS_QUERY             = 0x44,
  SLI_WLAN_REQ_HTTP_CLIENT_GET       = 0x51,
  SLI_WLAN_REQ_HTTP_CLIENT_POST      = 0x52,
  SLI_WLAN_REQ_HTTP_CLIENT_PUT       = 0x53,
  SLI_WLAN_REQ_IPCONFV6              = 0x90,
  SLI_WLAN_REQ_MULTICAST             = 0xB1,
  SLI_WLAN_REQ_HTTP_ABORT            = 0xB3,
  SLI_WLAN_REQ_HTTP_CREDENTIALS      = 0xB4,
  SLI_WLAN_REQ_EMB_MQTT_CLIENT       = 0xCB,
  SLI_WLAN_REQ_SNTP_CLIENT           = 0xE4,
  SLI_WLAN_REQ_HTTP_CLIENT_POST_DATA = 0xEB,

  // Socket commands
  SLI_WLAN_REQ_SOCKET_CREATE    = 0x42,
  SLI_WLAN_REQ_SOCKET_CLOSE     = 0x43,
  SLI_WLAN_REQ_SOCKET_READ_DATA = 0x6B,
  SLI_WLAN_REQ_SOCKET_ACCEPT    = 0x6C,
  SLI_WLAN_REQ_SOCKET_CONFIG    = 0xA7,
  SLI_WLAN_REQ_SELECT_REQUEST   = 0x74,
  SLI_WLAN_REQ_SET_SNI_EMBEDDED = 0x6E,

  // Unimplemented commands
  SLI_WLAN_REQ_SET_SLEEP_TIMER      = 0x16,
  SLI_WLAN_REQ_CFG_SAVE             = 0x20,
  SLI_WLAN_REQ_AUTO_CONFIG_ENABLE   = 0x21,
  SLI_WLAN_REQ_GET_CFG              = 0x22,
  SLI_WLAN_REQ_USER_STORE_CONFIG    = 0x23,
  SLI_WLAN_REQ_SET_WEP_KEYS         = 0x25,
  SLI_WLAN_REQ_SET_PROFILE          = 0x31,
  SLI_WLAN_REQ_GET_PROFILE          = 0x32,
  SLI_WLAN_REQ_DELETE_PROFILE       = 0x33,
  SLI_WLAN_REQ_SET_MULTICAST_FILTER = 0x40,
  SLI_WLAN_REQ_CONNECTION_STATUS    = 0x48,
  SLI_WLAN_REQ_CONFIGURE_P2P        = 0x4B,
  SLI_WLAN_REQ_WIRELESS_FWUP        = 0x59,
  SLI_WLAN_REQ_WMM_PS               = 0x97,
  SLI_WLAN_REQ_FWUP                 = 0x99,
#ifdef SLI_WAC_MFI_ENABLE
  SLI_WLAN_REQ_ADD_MFI_IE = 0xB5,
#endif
#ifndef SLI_SI91X_MCU_INTERFACE
  SLI_WLAN_REQ_CERT_VALID = 0xBC,
#endif
  SLI_WLAN_REQ_FREQ_OFFSET                  = 0xF3,
  SLI_WLAN_REQ_DYNAMIC_POOL                 = 0xC7,
  SLI_WLAN_REQ_MDNSD                        = 0xDB,
  SLI_WLAN_REQ_DISCOVER_SERVICE             = 0x8F,
  SLI_WLAN_REQ_FTP                          = 0xE2,
  SLI_WLAN_REQ_FTP_FILE_WRITE               = 0xE3,
  SLI_WLAN_REQ_SMTP_CLIENT                  = 0xE6,
  SLI_WLAN_REQ_OTA_FWUP                     = 0xEF,
  SLI_WLAN_REQ_WEBPAGE_LOAD                 = 0x50,
  SLI_WLAN_REQ_JSON_LOAD                    = 0x9c,
  SLI_WLAN_REQ_WEBPAGE_ERASE                = 0x9A,
  SLI_WLAN_REQ_JSON_OBJECT_ERASE            = 0x9B,
  SLI_WLAN_REQ_WEBPAGE_CLEAR_ALL            = 0x7F,
  SLI_WLAN_REQ_HOST_WEBPAGE_SEND            = 0x56,
  SLI_WLAN_REQ_GET_RANDOM                   = 0xF8,
  SLI_WLAN_REQ_POP3_CLIENT                  = 0xE7,
  SLI_WLAN_REQ_DHCP_USER_CLASS              = 0xEC,
  SLI_WLAN_REQ_TIMEOUT                      = 0xEA,
  SLI_WLAN_REQ_UPDATE_TCP_WINDOW            = 0xF5,
  SLI_WLAN_REQ_DNS_UPDATE                   = 0xED,
  SLI_WLAN_REQ_DNS_SERVER_ADD               = 0x55,
  SLI_WLAN_REQ_SET_TRANSCEIVER_CHANNEL      = 0x7A,
  SLI_WLAN_REQ_TRANSCEIVER_PEER_LIST_UPDATE = 0x8B,
  SLI_WLAN_REQ_TRANSCEIVER_CONFIG_PARAMS    = 0x8C,
  SLI_WLAN_REQ_SET_TRANSCEIVER_MCAST_FILTER = 0x8D,
  SLI_WLAN_REQ_TRANSCEIVER_FLUSH_DATA_Q     = 0x8E,
} sli_wlan_cmd_request_t;

// enumeration for WLAN command response codes
typedef enum {
  // Wi-Fi command response
  SLI_WLAN_RSP_SET_REGION           = 0x1D,
  SLI_WLAN_RSP_TWT_AUTO_CONFIG      = 0x2E,
  SLI_WLAN_RSP_TWT_PARAMS           = 0x2F,
  SLI_WLAN_RSP_EVM_OFFSET           = 0x36,
  SLI_WLAN_RSP_EVM_WRITE            = 0x37,
  SLI_WLAN_RSP_GAIN_TABLE           = 0x47,
  SLI_WLAN_RSP_FW_VERSION           = 0x49,
  SLI_WLAN_RSP_SET_CERTIFICATE      = 0x4D,
  SLI_WLAN_RSP_EXT_STATS            = 0x68, // Neither part 22q2 nor alpha 2
  SLI_WLAN_RSP_EFUSE_READ           = 0x73,
  SLI_WLAN_RSP_TX_TEST_MODE         = 0x7C,
  SLI_WLAN_RSP_ROAM_PARAMS          = 0x7B,
  SLI_WLAN_RSP_RADIO                = 0x81,
  SLI_WLAN_RSP_RX_STATS             = 0xA2,
  SLI_WLAN_RSP_SCAN_RESULTS         = 0xAF,
  SLI_WLAN_RSP_SET_REGION_AP        = 0xBD,
  SLI_WLAN_RSP_FILTER_BCAST_PACKETS = 0xC9,
  SLI_WLAN_RSP_CALIB_READ           = 0xCF,
  SLI_WLAN_RSP_FULL_FW_VERSION      = 0xE0,
  SLI_WLAN_RSP_HTTP_OTAF            = 0xF4,
  SLI_WLAN_RSP_11AX_PARAMS          = 0xFF,

  // Network command response
  SLI_WLAN_RSP_PING_PACKET           = 0x29,
  SLI_WLAN_RSP_NAT                   = 0x2A,
  SLI_WLAN_RSP_IPCONFV4              = 0x41,
  SLI_WLAN_RSP_DNS_QUERY             = 0x44,
  SLI_WLAN_RSP_HTTP_CLIENT_GET       = 0x51,
  SLI_WLAN_RSP_HTTP_CLIENT_POST      = 0x52,
  SLI_WLAN_RSP_HTTP_CLIENT_PUT       = 0x53,
  SLI_WLAN_RSP_IPCONFV6              = 0xA1,
  SLI_WLAN_RSP_MULTICAST             = 0xB1,
  SLI_WLAN_RSP_HTTP_ABORT            = 0xB3,
  SLI_WLAN_RSP_HTTP_CREDENTIALS      = 0xB4,
  SLI_WLAN_RSP_EMB_MQTT_CLIENT       = 0xCB,
  SLI_WLAN_RSP_SNTP_CLIENT           = 0xE4,
  SLI_WLAN_RSP_HTTP_CLIENT_POST_DATA = 0xEB,

  // Socket command response
  SLI_WLAN_RSP_SOCKET_CREATE    = 0x42,
  SLI_WLAN_RSP_SOCKET_CLOSE     = 0x43,
  SLI_WLAN_RSP_SOCKET_READ_DATA = 0x6B,
  SLI_WLAN_RSP_SOCKET_ACCEPT    = 0x6C,
  SLI_WLAN_RSP_SOCKET_CONFIG    = 0xA7,
  SLI_WLAN_RSP_SELECT_REQUEST   = 0x74,
  SLI_WLAN_RSP_SET_SNI_EMBEDDED = 0x6E,

  //  Unimplemented command for 22q2
  SLI_WLAN_RSP_CLEAR                  = 0x00,
  SLI_WLAN_RSP_CFG_SAVE               = 0x20,
  SLI_WLAN_RSP_AUTO_CONFIG_ENABLE     = 0x21,
  SLI_WLAN_RSP_GET_CFG                = 0x22,
  SLI_WLAN_RSP_USER_STORE_CONFIG      = 0x23,
  SLI_WLAN_RSP_SET_WEP_KEYS           = 0x25,
  SLI_WLAN_RSP_P2P_CONNECTION_REQUEST = 0x30,
  SLI_WLAN_RSP_SET_PROFILE            = 0x31,
  SLI_WLAN_RSP_GET_PROFILE            = 0x32,
  SLI_WLAN_RSP_DELETE_PROFILE         = 0x33,
  SLI_WLAN_RSP_CONN_ESTABLISH         = 0x61,
  SLI_WLAN_RSP_REMOTE_TERMINATE       = 0x62,
  SLI_WLAN_RSP_IPV4_CHANGE            = 0xAA,
  SLI_WLAN_RSP_TCP_ACK_INDICATION     = 0xAB,
  SLI_WLAN_RSP_UART_DATA_ACK          = 0xAC,
  SLI_WLAN_RSP_SET_MULTICAST_FILTER   = 0x40,
  SLI_WLAN_RSP_DNS_UPDATE             = 0xED,
  SLI_WLAN_RSP_CONNECTION_STATUS      = 0x48,
  SLI_WLAN_RSP_CONFIGURE_P2P          = 0x4B,
  SLI_WLAN_RSP_WFD_DEVICE             = 0x54,
  SLI_WLAN_RSP_DNS_SERVER_ADD         = 0x55,
  SLI_WLAN_RSP_WIRELESS_FWUP_OK       = 0x59,
  SLI_WLAN_RSP_WIRELESS_FWUP_DONE     = 0x5A,
  SLI_WLAN_RSP_WPS_METHOD             = 0x72,
  SLI_WLAN_RSP_WMM_PS                 = 0x97,
  SLI_WLAN_RSP_FWUP                   = 0x99,
#ifdef SLI_WAC_MFI_ENABLE
  SLI_WLAN_RSP_ADD_MFI_IE = 0xB5,
#endif
#ifndef SLI_SI91X_MCU_INTERFACE
  SLI_WLAN_RSP_CERT_VALID = 0xBC,
#endif
  SLI_WLAN_RSP_CLIENT_CONNECTED             = 0xC2,
  SLI_WLAN_RSP_CLIENT_DISCONNECTED          = 0xC3,
  SLI_WLAN_RSP_FREQ_OFFSET                  = 0xF3,
  SLI_WLAN_RSP_CALIB_WRITE                  = 0xCA,
  SLI_WLAN_RSP_DYNAMIC_POOL                 = 0xC7,
  SLI_WLAN_RSP_EMB_MQTT_PUBLISH_PKT         = 0xCC,
  SLI_WLAN_RSP_MQTT_REMOTE_TERMINATE        = 0xF0,
  SLI_WLAN_RSP_MDNSD                        = 0xDB,
  SLI_WLAN_RSP_DISCOVER_SERVICE             = 0x8F,
  SLI_WLAN_RSP_GET_DPD_DATA                 = 0xDC,
  SLI_WLAN_RSP_FTP                          = 0xE2,
  SLI_WLAN_RSP_FTP_FILE_WRITE               = 0xE3,
  SLI_WLAN_RSP_SNTP_SERVER                  = 0xE5,
  SLI_WLAN_RSP_SMTP_CLIENT                  = 0xE6,
  SLI_WLAN_RSP_OTA_FWUP                     = 0xEF,
  SLI_WLAN_RSP_WEBPAGE_LOAD                 = 0x50,
  SLI_WLAN_RSP_JSON_LOAD                    = 0x9c,
  SLI_WLAN_RSP_WEBPAGE_ERASE                = 0x9A,
  SLI_WLAN_RSP_JSON_OBJECT_ERASE            = 0x9B,
  SLI_WLAN_RSP_WEBPAGE_CLEAR_ALL            = 0x7F,
  SLI_WLAN_RSP_HOST_WEBPAGE_SEND            = 0x56,
  SLI_WLAN_RSP_JSON_UPDATE                  = 0x9D,
  SLI_WLAN_RSP_GET_RANDOM                   = 0xF8,
  SLI_WLAN_RSP_ASYNCHRONOUS                 = 0xFF,
  SLI_WLAN_RSP_JSON_EVENT                   = 0xEE,
  SLI_WLAN_RSP_POP3_CLIENT                  = 0xE7,
  SLI_WLAN_RSP_POP3_CLIENT_TERMINATE        = 0xE8,
  SLI_WLAN_RSP_DHCP_USER_CLASS              = 0xEC,
  SLI_WLAN_RSP_TIMEOUT                      = 0xEA,
  SLI_WLAN_RSP_URL_REQUEST                  = 0x64,
  SLI_WLAN_RSP_MODULE_STATE                 = 0x70,
  SLI_WLAN_RSP_TWT_ASYNC                    = 0x71,
  SLI_WLAN_RSP_UPDATE_TCP_WINDOW            = 0xF5,
  SLI_WLAN_RSP_TRANSCEIVER_SET_CHANNEL      = 0x7A,
  SLI_WLAN_RSP_TRANSCEIVER_PEER_LIST_UPDATE = 0x8B,
  SLI_WLAN_RSP_TRANSCEIVER_CONFIG_PARAMS    = 0x8C,
  SLI_WLAN_RSP_TRANSCEIVER_SET_MCAST_FILTER = 0x8D,
  SLI_WLAN_RSP_TRANSCEIVER_FLUSH_DATA_Q     = 0x8E,
  SLI_WLAN_RSP_TRANSCEIVER_TX_DATA_STATUS   = 0x3D,
  SLI_SI91X_WIFI_RX_DOT11_DATA              = 0x03,
  // Unimplemented commands after 22Q2
  SLI_WLAN_RATE_RSP_STATS = 0x88
} sli_wlan_cmd_response_t;

typedef enum { SL_SI91X_SOCKET_REMOTE_TERMINATED_EVENT, SL_SI91X_SOCKET_EVENT_COUNT } sl_si91x_socket_event_t;

//! @endcond
