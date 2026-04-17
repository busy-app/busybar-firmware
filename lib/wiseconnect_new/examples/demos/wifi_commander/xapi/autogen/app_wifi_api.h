/********************************************************************************
 * @file  app_wifi_api.h
 * @brief APP_WIFI_API command declarations
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

#ifndef APP_WIFI_API_H
#define APP_WIFI_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "sl_status.h"
#include "app_xapi.h"

/* APP_WIFI_ synonyms for XAPI header macros */
#define APP_WIFI_MSG_ID(HDR)        APP_XAPI_MSG_ID(HDR)
#define APP_WIFI_MSG_HEADER_LEN     APP_XAPI_MSG_HEADER_LEN
#define APP_WIFI_MSG_LEN(HDR)       APP_XAPI_MSG_LEN(HDR)
#define APP_WIFI_BIT_ENCRYPTED      APP_XAPI_BIT_ENCRYPTED
#define APP_WIFI_MSG_ENCRYPTED(HDR) APP_XAPI_MSG_ENCRYPTED(HDR)

/**
 * @brief 
                BSSID
            
 */
typedef struct {
  uint8_t data[6]; /**< 
                BSSID
             */
} app_wifi_bssid_t;

/**
 * @brief 
                Scan SSID.
            
 */
typedef struct {
  uint8_t data[32]; /**< 
                Scan SSID.
             */
} app_wifi_scan_ssid_t;

/**
 * @addtogroup app_wifi_system System
 * @{
 *
 * @brief System
 *
 * Commands and events in this class can be used to access and query the local
 * device.
 */

/**
 * @addtogroup app_wifi_evt_system_boot app_wifi_evt_system_boot
 * @{
 * @brief Indicates that the device has started and the radio is ready
 *
 * This event carries the module build number.
 */

/***************************************************************************/ /**
 * @brief Data structure of the boot event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_system_boot_s {
  uint8_t app_major;           /**< Major release version. */
  uint8_t app_minor;           /**< Minor release version. */
  uint8_t app_patch;           /**< Patch release number. */
  uint8_t fw_chip_id;          /**< Chip ID */
  uint8_t fw_rom_id;           /**< ROM ID */
  uint8_t fw_major;            /**< Major Version Number */
  uint8_t fw_minor;            /**< Minor version Number */
  uint8_t fw_security_version; /**< Security Version */
  uint8_t fw_patch_num;        /**< Patch Number */
  uint8_t fw_customer_id;      /**< Customer ID */
  uint16_t fw_build_num;       /**< Build Number */
});

typedef struct app_wifi_evt_system_boot_s app_wifi_evt_system_boot_t;

/** @} */ // end addtogroup app_wifi_evt_system_boot

/**
 * @addtogroup app_wifi_evt_system_error app_wifi_evt_system_error
 * @{
 * @brief Indicates that an error has occurred
 */

/***************************************************************************/ /**
 * @brief Data structure of the error event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_system_error_s {
  uint16_t reason; /**< Error code from module. */
  uint8array data; /**< Data related to the error; this field can be empty. */
});

typedef struct app_wifi_evt_system_error_s app_wifi_evt_system_error_t;

/** @} */ // end addtogroup app_wifi_evt_system_error

/**
 * @addtogroup app_wifi_net_intf Network Interface
 * @{
 *
 * @brief Network Interface
 *
 * Commands and Events in this class performs essential operations for managing
 * and controlling network interfaces. They handle initialization,
 * configuration, starting, and stopping of these interfaces, ensuring efficient
 * utilization within the system. Note: Currently, only Access Point and Client
 * interfaces are supported.
 */

/**
 * @brief Enumeration of available network interface types.
 */
typedef enum {
  APP_WIFI_NET_INTF_INTERFACE_CLIENT    = 0x8,  /**< (0x8) Wi-Fi Client
                                                     Interface. */
  APP_WIFI_NET_INTF_INTERFACE_AP        = 0x10, /**< (0x10) Wi-Fi Access Point
                                                     Interface. */
  APP_WIFI_NET_INTF_INTERFACE_ETHERNET  = 0x18, /**< (0x18) Ethernet Interface
                                                     (not currently supported). */
  APP_WIFI_NET_INTF_INTERFACE_THREAD    = 0x20, /**< (0x20) Thread Interface
                                                     (not currently supported). */
  APP_WIFI_NET_INTF_INTERFACE_BLUETOOTH = 0x28, /**< (0x28) Bluetooth Interface
                                                     (not currently supported). */
  APP_WIFI_NET_INTF_INTERFACE_ZWAVE     = 0x30  /**< (0x30) Z-Wave Interface
                                                     (not currently supported). */
} app_wifi_net_intf_interface_t;

/**
 * @brief Enumeration of profile IDs for various network profiles.
 */
typedef enum {
  APP_WIFI_NET_INTF_PROFILE_ID_DEFAULT_WIFI_AP_PROFILE     = 0x0, /**< (0x0)
                                                                       Default
                                                                       profile
                                                                       for Wi-Fi
                                                                       Access
                                                                       Point
                                                                       configuration. */
  APP_WIFI_NET_INTF_PROFILE_ID_DEFAULT_WIFI_CLIENT_PROFILE = 0x0  /**< (0x0)
                                                                       Default
                                                                       profile
                                                                       for Wi-Fi
                                                                       Client
                                                                       configuration. */
} app_wifi_net_intf_profile_id_t;

/**

 */
typedef enum {
  APP_WIFI_NET_INTF_REGION_CODE_DEFAULT_REGION = 0x0, /**< (0x0) The default
                                                           region setting
                                                           applied when the
                                                           device is initialized
                                                           to factory settings. */
  APP_WIFI_NET_INTF_REGION_CODE_US             = 0x1, /**< (0x1) The region code
                                                           for the United
                                                           States. */
  APP_WIFI_NET_INTF_REGION_CODE_EU             = 0x2, /**< (0x2) The region code
                                                           for the European
                                                           Union. */
  APP_WIFI_NET_INTF_REGION_CODE_JP             = 0x3, /**< (0x3) The region code
                                                           for Japan. */
  APP_WIFI_NET_INTF_REGION_CODE_WORLD_DOMAIN   = 0x4, /**< (0x4) The worldwide
                                                           domain region code,
                                                           which applies global
                                                           regulatory settings.
                                                           This setting is used
                                                           for devices that are
                                                           not restricted to a
                                                           specific region and
                                                           are intended for use
                                                           across multiple
                                                           countries. */
  APP_WIFI_NET_INTF_REGION_CODE_KR             = 0x5, /**< (0x5) The region code
                                                           for Korea. */
  APP_WIFI_NET_INTF_REGION_CODE_CH             = 0x7  /**< (0x7) The region code
                                                           for China. */
} app_wifi_net_intf_region_code_t;

/**

 */
typedef enum {
  APP_WIFI_NET_INTF_BAND_MODE_2_4_GHZ  = 0x0, /**< (0x0) 2.4 GHz Wi-Fi band. */
  APP_WIFI_NET_INTF_BAND_MODE_5_GHZ    = 0x1, /**< (0x1) 5 GHz Wi-Fi band
                                                   (currently not supported). */
  APP_WIFI_NET_INTF_BAND_MODE_DUAL_GHZ = 0x2  /**< (0x2) Both 2.4 GHz and 5 GHz
                                                   Wi-Fi bands (currently not
                                                   supported). */
} app_wifi_net_intf_band_t;

/**

 */
typedef enum {
  APP_WIFI_NET_INTF_BOOT_OPTION_LOAD_NWP_FW = 0x31 /**< (0x31) Specifies the
                                                         option to load the
                                                         Network Processor (NWP)
                                                         firmware during the
                                                         boot process. This
                                                         option is used to
                                                         initialize the device's
                                                         networking capabilities
                                                         by loading the
                                                         appropriate firmware. */
} app_wifi_net_intf_boot_option_t;

/**
 * @brief si91x operating mode
 */
typedef enum {
  APP_WIFI_NET_INTF_OPER_MODE_CLIENT            = 0x0, /**< (0x0) Wi-Fi personal
                                                            client mode */
  APP_WIFI_NET_INTF_OPER_MODE_ENTERPRISE_CLIENT = 0x2, /**< (0x2) Wi-Fi
                                                            enterprise client
                                                            mode */
  APP_WIFI_NET_INTF_OPER_MODE_AP                = 0x6, /**< (0x6) Wi-Fi access
                                                            point mode */
  APP_WIFI_NET_INTF_OPER_MODE_TRANSCEIVER       = 0x7, /**< (0x7) Wi-Fi
                                                            transceiver mode */
  APP_WIFI_NET_INTF_OPER_MODE_TRANSMIT_TEST     = 0x8, /**< (0x8) Wi-Fi transmit
                                                            test mode */
  APP_WIFI_NET_INTF_OPER_MODE_CONCURRENT        = 0x9  /**< (0x9) Wi-Fi
                                                            concurrent mode */
} app_wifi_net_intf_oper_mode_t;

/**

 */
typedef enum {
  APP_WIFI_NET_INTF_COEX_MODE_WLAN_ONLY = 0x0, /**< (0x0) Wireless local area
                                                    network (WLAN) only mode */
  APP_WIFI_NET_INTF_COEX_MODE_BLE       = 0xc, /**< (0xc) Bluetooth Low Energy
                                                    (BLE) only mode, used when
                                                    power save mode is not
                                                    needed. */
  APP_WIFI_NET_INTF_COEX_MODE_WLAN_BLE  = 0xd  /**< (0xd) WLAN and BLE mode */
} app_wifi_net_intf_coex_mode_t;

/**
 * @addtogroup app_wifi_evt_net_intf_network_up_completed app_wifi_evt_net_intf_network_up_completed
 * @{
 * @brief Indicates that the network interface has successfully completed the
 * network up procedure
 *
 * This event is triggered when the specified network interface is brought up
 * and initialized, or if there was an error during the process.
 */

/***************************************************************************/ /**
 * @brief Data structure of the network_up_completed event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_net_intf_network_up_completed_s {
  uint8_t net_interface; /**< Enum @ref app_wifi_net_intf_interface_t. Network
                               interface to be initialized. Currently, only
                               AP_INTERFACE and CLIENT_INTERFACE are supported. */
  uint16_t is_completed; /**< Status of the network interface. A value of 1
                               means the network interface is up, and 0 means
                               the network interface is down. */
});

typedef struct app_wifi_evt_net_intf_network_up_completed_s app_wifi_evt_net_intf_network_up_completed_t;

/** @} */ // end addtogroup app_wifi_evt_net_intf_network_up_completed

/**
 * @addtogroup app_wifi_net_profile Network Profile
 * @{
 *
 * @brief Network Profile
 *
 * Commands and events in this class are described by "profiles" that contain
 * all the information needed to configure a particular interface. Each type of
 * network interface has a unique profile structure to optimally store the
 * relevant information. For example, a Wi-Fi client interface requires an SSID,
 * a security mode, and a passphrase for a security-enabled access point.
 */

/**
 * @brief Enumeration for various Wi-Fi security types.
 */
typedef enum {
  APP_WIFI_NET_PROFILE_SECURITY_TYPE_OPEN                       = 0x0,   /**<
                                                                               (0x0)
                                                                               Wi-Fi
                                                                               Open
                                                                               security
                                                                               type. */
  APP_WIFI_NET_PROFILE_SECURITY_TYPE_WPA                        = 0x1,   /**<
                                                                               (0x1)
                                                                               Wi-Fi
                                                                               WPA
                                                                               security
                                                                               type. */
  APP_WIFI_NET_PROFILE_SECURITY_TYPE_WPA2                       = 0x2,   /**<
                                                                               (0x2)
                                                                               Wi-Fi
                                                                               WPA2
                                                                               security
                                                                               type. */
  APP_WIFI_NET_PROFILE_SECURITY_TYPE_WEP                        = 0x3,   /**<
                                                                               (0x3)
                                                                               Wi-Fi
                                                                               WEP
                                                                               security
                                                                               type. */
  APP_WIFI_NET_PROFILE_SECURITY_TYPE_WPA_ENTERPRISE             = 0x4,   /**<
                                                                               (0x4)
                                                                               Wi-Fi
                                                                               WPA
                                                                               enterprise
                                                                               security
                                                                               type. */
  APP_WIFI_NET_PROFILE_SECURITY_TYPE_WPA2_ENTERPRISE            = 0x5,   /**<
                                                                               (0x5)
                                                                               Wi-Fi
                                                                               WPA2
                                                                               enterprise
                                                                               security
                                                                               type. */
  APP_WIFI_NET_PROFILE_SECURITY_TYPE_WPA_WPA2_MIXED             = 0x6,   /**<
                                                                               (0x6)
                                                                               Wi-Fi
                                                                               WPA/WPA2
                                                                               mixed
                                                                               security
                                                                               type
                                                                               that
                                                                               supports
                                                                               both
                                                                               WPA
                                                                               and
                                                                               WPA2. */
  APP_WIFI_NET_PROFILE_SECURITY_TYPE_WPA3                       = 0x7,   /**<
                                                                               (0x7)
                                                                               Wi-Fi
                                                                               WPA3
                                                                               security
                                                                               type. */
  APP_WIFI_NET_PROFILE_SECURITY_TYPE_WPA3_TRANSITION            = 0x8,   /**<
                                                                               (0x8)
                                                                               Wi-Fi
                                                                               WPA3
                                                                               Transition
                                                                               security
                                                                               type
                                                                               (not
                                                                               currently
                                                                               supported
                                                                               in
                                                                               AP
                                                                               mode). */
  APP_WIFI_NET_PROFILE_SECURITY_TYPE_WPA3_ENTERPRISE            = 0x9,   /**<
                                                                               (0x9)
                                                                               Wi-Fi
                                                                               WPA3
                                                                               enterprise
                                                                               security
                                                                               type. */
  APP_WIFI_NET_PROFILE_SECURITY_TYPE_WPA3_TRANSITION_ENTERPRISE = 0xa,   /**<
                                                                               (0xa)
                                                                               Wi-Fi
                                                                               WPA3
                                                                               Transition
                                                                               enterprise
                                                                               security
                                                                               type. */
  APP_WIFI_NET_PROFILE_SECURITY_TYPE_SECURITY_UNKNOWN           = 0xffff /**<
                                                                               (0xffff)
                                                                               Wi-Fi
                                                                               Unknown
                                                                               Security
                                                                               type. */
} app_wifi_net_profile_security_type_t;

/**
 * @brief Enumeration for various Wi-Fi encryption types.
 */
typedef enum {
  APP_WIFI_NET_PROFILE_ENCRYPTION_TYPE_DEFAULT       = 0x0, /**< (0x0) Default
                                                                 Wi-Fi
                                                                 encryption. */
  APP_WIFI_NET_PROFILE_ENCRYPTION_TYPE_NO            = 0x1, /**< (0x1) Wi-Fi
                                                                 with no
                                                                 Encryption (not
                                                                 currently
                                                                 supported in
                                                                 STA mode). */
  APP_WIFI_NET_PROFILE_ENCRYPTION_TYPE_WEP           = 0x2, /**< (0x2) Wi-Fi
                                                                 with WEP
                                                                 Encryption (not
                                                                 currently
                                                                 supported in
                                                                 STA mode). */
  APP_WIFI_NET_PROFILE_ENCRYPTION_TYPE_TKIP          = 0x3, /**< (0x3) Wi-Fi
                                                                 with TKIP
                                                                 Encryption (not
                                                                 currently
                                                                 supported in
                                                                 STA mode). */
  APP_WIFI_NET_PROFILE_ENCRYPTION_TYPE_CCMP          = 0x4, /**< (0x4) Wi-Fi
                                                                 with CCMP
                                                                 Encryption. */
  APP_WIFI_NET_PROFILE_ENCRYPTION_TYPE_EAP_TLS       = 0x5, /**< (0x5) Wi-Fi
                                                                 with Enterprise
                                                                 TLS Encryption. */
  APP_WIFI_NET_PROFILE_ENCRYPTION_TYPE_EAP_TTLS      = 0x6, /**< (0x6) Wi-Fi
                                                                 with Enterprise
                                                                 TTLS
                                                                 Encryption. */
  APP_WIFI_NET_PROFILE_ENCRYPTION_TYPE_EAP_FAST      = 0x7, /**< (0x7) Wi-Fi
                                                                 with Enterprise
                                                                 FAST
                                                                 Encryption. */
  APP_WIFI_NET_PROFILE_ENCRYPTION_TYPE_PEAP_MSCHAPV2 = 0x8, /**< (0x8) Wi-Fi
                                                                 with Enterprise
                                                                 PEAP
                                                                 Encryption. */
  APP_WIFI_NET_PROFILE_ENCRYPTION_TYPE_EAP_LEAP      = 0x9  /**< (0x9) Wi-Fi
                                                                 with Enterprise
                                                                 LEAP
                                                                 Encryption. */
} app_wifi_net_profile_encryption_type_t;

/**
 * @brief Enumeration for different Wi-Fi rate protocols.
 */
typedef enum {
  APP_WIFI_NET_PROFILE_RATE_PROTOCOL_B_ONLY  = 0x0, /**< (0x0) 802.11b rates
                                                         only. */
  APP_WIFI_NET_PROFILE_RATE_PROTOCOL_G_ONLY  = 0x1, /**< (0x1) 802.11g rates
                                                         only. */
  APP_WIFI_NET_PROFILE_RATE_PROTOCOL_N_ONLY  = 0x2, /**< (0x2) 802.11n rates
                                                         only. */
  APP_WIFI_NET_PROFILE_RATE_PROTOCOL_AC_ONLY = 0x3, /**< (0x3) 802.11ac rates
                                                         only (not currently
                                                         supported). */
  APP_WIFI_NET_PROFILE_RATE_PROTOCOL_AX_ONLY = 0x4, /**< (0x4) 802.11ax rates
                                                         only. */
  APP_WIFI_NET_PROFILE_RATE_PROTOCOL_AUTO    = 0x5  /**< (0x5) Automatic rate
                                                         selection. */
} app_wifi_net_profile_rate_protocol_t;

/**

 */
typedef enum {
  APP_WIFI_NET_PROFILE_AP_OPTION_ENABLE_HIDDEN_SSID  = 0x1, /**< (0x1) Hide the
                                                                 SSID of the
                                                                 Access Point. */
  APP_WIFI_NET_PROFILE_AP_OPTION_DISABLE_HIDDEN_SSID = 0x0  /**< (0x0) Display
                                                                 the SSID of the
                                                                 Access Point. */
} app_wifi_net_profile_ap_option_t;

/**

 */
typedef enum {
  APP_WIFI_NET_PROFILE_CREDENTIAL_ID_NO_CREDENTIAL       = 0x0,   /**< (0x0) No
                                                                       Credential
                                                                       ID. */
  APP_WIFI_NET_PROFILE_CREDENTIAL_ID_DEFAULT_WIFI_CLIENT = 0x1,   /**< (0x1)
                                                                       Default
                                                                       Wi-Fi
                                                                       Client
                                                                       Credential
                                                                       ID. */
  APP_WIFI_NET_PROFILE_CREDENTIAL_ID_WIFI_CLIENT_1       = 0x2,   /**< (0x2)
                                                                       Wi-Fi
                                                                       Client
                                                                       Credential
                                                                       ID 1. */
  APP_WIFI_NET_PROFILE_CREDENTIAL_ID_DEFAULT_WIFI_AP     = 0x3,   /**< (0x3)
                                                                       Default
                                                                       Wi-Fi
                                                                       Access
                                                                       Point
                                                                       Credential
                                                                       ID. */
  APP_WIFI_NET_PROFILE_CREDENTIAL_ID_WIFI_AP_1           = 0x4,   /**< (0x4)
                                                                       Wi-Fi
                                                                       Access
                                                                       Point
                                                                       Credential
                                                                       ID 1. */
  APP_WIFI_NET_PROFILE_CREDENTIAL_ID_WIFI_EAP_CLIENT     = 0x5,   /**< (0x5)
                                                                       Wi-Fi EAP
                                                                       Client
                                                                       Credential
                                                                       ID. */
  APP_WIFI_NET_PROFILE_CREDENTIAL_ID_WIFI_EAP_SERVER     = 0x6,   /**< (0x6)
                                                                       Wi-Fi EAP
                                                                       Server
                                                                       Credential
                                                                       ID. */
  APP_WIFI_NET_PROFILE_CREDENTIAL_ID_USER                = 0x7,   /**< (0x7)
                                                                       User
                                                                       Credential
                                                                       ID. */
  APP_WIFI_NET_PROFILE_CREDENTIAL_ID_TLS_CLIENT          = 0x100, /**< (0x100)
                                                                       TLS
                                                                       Client
                                                                       Credential
                                                                       ID. */
  APP_WIFI_NET_PROFILE_CREDENTIAL_ID_TLS_SERVER          = 0x200, /**< (0x200)
                                                                       TLS
                                                                       Server
                                                                       Credential
                                                                       ID. */
  APP_WIFI_NET_PROFILE_CREDENTIAL_ID_MQTT_SERVER         = 0x300, /**< (0x300)
                                                                       MQTT
                                                                       Server
                                                                       Credential
                                                                       ID. */
  APP_WIFI_NET_PROFILE_CREDENTIAL_ID_MQTT_CLIENT         = 0x400, /**< (0x400)
                                                                       MQTT
                                                                       Client
                                                                       Credential
                                                                       ID. */
  APP_WIFI_NET_PROFILE_CREDENTIAL_ID_HTTP_SERVER         = 0x500, /**< (0x500)
                                                                       HTTP
                                                                       Server
                                                                       Credential
                                                                       ID. */
  APP_WIFI_NET_PROFILE_CREDENTIAL_ID_HTTP_CLIENT         = 0x600  /**< (0x600)
                                                                       HTTP
                                                                       Client
                                                                       Credential
                                                                       ID. */
} app_wifi_net_profile_credential_id_t;

/**

 */
typedef enum {
  APP_WIFI_NET_PROFILE_AP_KEEPALIVE_TYPE_DISABLE      = 0x0, /**< (0x0) Disables
                                                                  the keepalive
                                                                  functionality. */
  APP_WIFI_NET_PROFILE_AP_KEEPALIVE_TYPE_DEAUTH_BASED = 0x1, /**< (0x1) The AP
                                                                  utilizes
                                                                  keepalive
                                                                  functionality
                                                                  based on
                                                                  received
                                                                  packets from
                                                                  its stations.
                                                                  If no packets
                                                                  are received
                                                                  from a station
                                                                  within the
                                                                  keepalive
                                                                  timeout
                                                                  period, the AP
                                                                  disconnects
                                                                  that station. */
  APP_WIFI_NET_PROFILE_AP_KEEPALIVE_TYPE_NULL_BASED   = 0x3  /**< (0x3) The AP
                                                                  performs
                                                                  keepalive by
                                                                  sending a NULL
                                                                  DATA packet to
                                                                  the station.
                                                                  If an ACK is
                                                                  not received
                                                                  from the
                                                                  station after
                                                                  a specific
                                                                  number of
                                                                  retries, the
                                                                  AP will
                                                                  disconnect the
                                                                  station. */
} app_wifi_net_profile_ap_keepalive_type_t;

/**

 */
typedef enum {
  APP_WIFI_NET_PROFILE_IS_11N_ENABLED = 0x1, /**< (0x1) Indicates whether
                                                  802.11n is enabled. */
  APP_WIFI_NET_PROFILE_IS_11N_DISABLE = 0x0  /**< (0x0) Indicates whether
                                                  802.11n is disable. */
} app_wifi_net_profile_is_11n_t;

/**

 */
typedef enum {
  APP_WIFI_NET_PROFILE_TDI_NONE = 0x0, /**< (0x0) Allows stations to transition
                                             within the AP network. */
  APP_WIFI_NET_PROFILE_TDI_SAE  = 0x10 /**< (0x10) Disallows stations from
                                             transitioning within the AP network
                                             and only allows connections with
                                             WPA3-SAE security mode. */
} app_wifi_net_profile_tdi_t;

/**

 */
typedef enum {
  APP_WIFI_NET_PROFILE_BAND_MODE_AUTO    = 0x0, /**< (0x0) Wi-Fi Band Auto. */
  APP_WIFI_NET_PROFILE_BAND_MODE_900_MHZ = 0x1, /**< (0x1) Wi-Fi Band 900 MHz
                                                     (not currently supported). */
  APP_WIFI_NET_PROFILE_BAND_MODE_2_4_GHZ = 0x2, /**< (0x2) Wi-Fi Band 2.4 GHz. */
  APP_WIFI_NET_PROFILE_BAND_MODE_5_GHZ   = 0x3, /**< (0x3) Wi-Fi Band 5 GHz (not
                                                     currently supported). */
  APP_WIFI_NET_PROFILE_BAND_MODE_6_GHZ   = 0x4, /**< (0x4) Wi-Fi Band 6 GHz (not
                                                     currently supported). */
  APP_WIFI_NET_PROFILE_BAND_MODE_60_GHZ  = 0x5  /**< (0x5) Wi-Fi Band 60 GHz
                                                     (not currently supported). */
} app_wifi_net_profile_band_t;

/**

 */
typedef enum {
  APP_WIFI_NET_PROFILE_BANDWIDTH_MODE_AUTO    = 0x0, /**< (0x0) Wi-Fi Bandwidth
                                                          Auto. */
  APP_WIFI_NET_PROFILE_BANDWIDTH_MODE_10_MHZ  = 0x0, /**< (0x0) Wi-Fi Bandwidth
                                                          10 MHz (not currently
                                                          supported). */
  APP_WIFI_NET_PROFILE_BANDWIDTH_MODE_20_MHZ  = 0x1, /**< (0x1) Wi-Fi Bandwidth
                                                          20 MHz. */
  APP_WIFI_NET_PROFILE_BANDWIDTH_MODE_40_MHZ  = 0x2, /**< (0x2) Wi-Fi Bandwidth
                                                          40 MHz (not currently
                                                          supported). */
  APP_WIFI_NET_PROFILE_BANDWIDTH_MODE_80_MHZ  = 0x3, /**< (0x3) Wi-Fi Bandwidth
                                                          80 MHz (not currently
                                                          supported). */
  APP_WIFI_NET_PROFILE_BANDWIDTH_MODE_160_MHZ = 0x4  /**< (0x4) Wi-Fi Bandwidth
                                                          160 MHz (not currently
                                                          supported). */
} app_wifi_net_profile_bandwidth_t;

/**

 */
typedef enum {
  APP_WIFI_NET_PROFILE_IP_MANAGEMENT_STATIC_IP  = 0x1, /**< (0x1) Assign STATIC
                                                            IP address to an
                                                            interface. */
  APP_WIFI_NET_PROFILE_IP_MANAGEMENT_DHCP       = 0x2, /**< (0x2) Assign IP
                                                            address to an
                                                            interface
                                                            dynamically using
                                                            DHCP. */
  APP_WIFI_NET_PROFILE_IP_MANAGEMENT_LINK_LOCAL = 0x3  /**< (0x3) Assign IP
                                                            address using
                                                            link-local
                                                            addressing. */
} app_wifi_net_profile_ip_management_t;

/**

 */
typedef enum {
  APP_WIFI_NET_PROFILE_IP_ADDRESS_TYPE_IPV4            = 0x4, /**< (0x4) IPv4
                                                                   address. */
  APP_WIFI_NET_PROFILE_IP_ADDRESS_TYPE_IPV6            = 0x8, /**< (0x8) IPv6
                                                                   address,
                                                                   unspecified
                                                                   sub-type. */
  APP_WIFI_NET_PROFILE_IP_ADDRESS_TYPE_IPV6_LINK_LOCAL = 0x9, /**< (0x9) IPv6
                                                                   link-local
                                                                   address. */
  APP_WIFI_NET_PROFILE_IP_ADDRESS_TYPE_IPV6_SITE_LOCAL = 0xa, /**< (0xa) IPv6
                                                                   site-local
                                                                   address. */
  APP_WIFI_NET_PROFILE_IP_ADDRESS_TYPE_IPV6_GLOBAL     = 0xb, /**< (0xb) IPv6
                                                                   global
                                                                   address. */
  APP_WIFI_NET_PROFILE_IP_ADDRESS_TYPE_INVALID_IP      = 0x0  /**< (0x0) This
                                                                   enumeration
                                                                   value
                                                                   represents an
                                                                   invalid IP
                                                                   address. */
} app_wifi_net_profile_ip_address_type_t;

/**
 * @brief Enumeration of possible BSS types.
 */
typedef enum {
  APP_WIFI_NET_PROFILE_BSS_TYPE_INFRASTRUCTURE = 0x0, /**< (0x0) Standard Wi-Fi
                                                           connection with an
                                                           access point. */
  APP_WIFI_NET_PROFILE_BSS_TYPE_ADHOC          = 0x1, /**< (0x1) Direct
                                                           connection between
                                                           devices without an
                                                           access point. */
  APP_WIFI_NET_PROFILE_BSS_TYPE_ANY            = 0x2, /**< (0x2) Any available
                                                           BSS type. */
  APP_WIFI_NET_PROFILE_BSS_TYPE_UNKNOWN        = 0x3  /**< (0x3) unknown BSS
                                                           type. */
} app_wifi_net_profile_bss_type_t;

/**

 */
typedef enum {
  APP_WIFI_NET_PROFILE_CLIENT_FLAG_NO_JOIN_OPTION   = 0x0, /**< (0x0) Wi-Fi
                                                                Client Join with
                                                                no flags. */
  APP_WIFI_NET_PROFILE_CLIENT_FLAG_JOIN_WITH_NO_CSA = 0x1, /**< (0x1) Wi-Fi
                                                                Client Join
                                                                without Channel
                                                                Switch
                                                                Announcement
                                                                (CSA). */
  APP_WIFI_NET_PROFILE_CLIENT_FLAG_JOIN_WITH_SCAN   = 0x2  /**< (0x2) Wi-Fi
                                                                Client Join with
                                                                Scan. */
} app_wifi_net_profile_client_flag_t;

/**
 * @addtogroup app_wifi_ap Access point
 * @{
 *
 * @brief Access point
 *
 * Commands and events in this class can be used to implement a Wi-Fi Access
 * Point (AP).
 */

/**
 * @brief This enumeration defines various groups of Wi-Fi-related events, each
 * representing a distinct category of actions or status changes within the
 * Wi-Fi system. These groups cover events such as scanning for networks,
 * joining or disconnecting from a network, receiving packets, handling
 * statistics, firmware updates, client connections, and fail indications.
 * Certain event groups may not be supported in the current release, but are
 * included for future implementation or diagnostic purposes.
 */
typedef enum {
  APP_WIFI_AP_WIFI_EVENT_GROUP_CLIENT_CONNECTED    = 0x8, /**< (0x8) Event group
                                                               for Wi-Fi client
                                                               connected status */
  APP_WIFI_AP_WIFI_EVENT_GROUP_CLIENT_DISCONNECTED = 0xa  /**< (0xa) Event group
                                                               for Wi-Fi client
                                                               disconnection
                                                               status */
} app_wifi_ap_wifi_event_group_t;

/**
 * @brief Enumeration of de-authentication reasons from an access point.
 */
typedef enum {
  APP_WIFI_AP_DEAUTH_REASON_WIFI_DEAUTH             = 0x0, /**< (0x0)
                                                                De-Authentication
                                                                from radius
                                                                server. */
  APP_WIFI_AP_DEAUTH_REASON_WIFI_DEAUTH_UNSPECIFIED = 0x1  /**< (0x1)
                                                                Unspecified
                                                                de-authentication
                                                                reason. */
} app_wifi_ap_deauth_reason_t;

/**
 * @addtogroup app_wifi_evt_ap_client_connected_event app_wifi_evt_ap_client_connected_event
 * @{
 * @brief Indicates a client connection and displays the MAC address of the
 * connected client
 */

/***************************************************************************/ /**
 * @brief Data structure of the client_connected_event event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_ap_client_connected_event_s {
  mac_addr mac_address; /**< Displays the MAC address of the connected client. */
});

typedef struct app_wifi_evt_ap_client_connected_event_s app_wifi_evt_ap_client_connected_event_t;

/** @} */ // end addtogroup app_wifi_evt_ap_client_connected_event

/**
 * @addtogroup app_wifi_evt_ap_client_disconnected_event app_wifi_evt_ap_client_disconnected_event
 * @{
 * @brief Indicates a client disconnection and displays the MAC address of the
 * disconnected client
 */

/***************************************************************************/ /**
 * @brief Data structure of the client_disconnected_event event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_ap_client_disconnected_event_s {
  mac_addr mac_address; /**< Displays the MAC address of the disconnected
                             client. */
});

typedef struct app_wifi_evt_ap_client_disconnected_event_s app_wifi_evt_ap_client_disconnected_event_t;

/** @} */ // end addtogroup app_wifi_evt_ap_client_disconnected_event

/**
 * @addtogroup app_wifi_evt_ap_get_client_info_event app_wifi_evt_ap_get_client_info_event
 * @{
 * @brief Provides an array containing the IP address and MAC address of the
 * client
 */

/***************************************************************************/ /**
 * @brief Data structure of the get_client_info_event event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_ap_get_client_info_event_s {
  ipv4_addr ipv4_address; /**< IPv4 address of the client. */
  mac_addr mac_address;   /**< MAC address of the client. */
});

typedef struct app_wifi_evt_ap_get_client_info_event_s app_wifi_evt_ap_get_client_info_event_t;

/** @} */ // end addtogroup app_wifi_evt_ap_get_client_info_event

/**
 * @addtogroup app_wifi_net_cred Network Credential
 * @{
 *
 * @brief Network Credential
 *
 * This class provides commands and events, which can be used to manage network
 * credentials, including Wi-Fi client credentials, MQTT client credentials, TLS
 * client certificates, and others.
 */

/**
 * @brief Defines various types of credentials used for network authentication
 * and encryption.
 */
typedef enum {
  APP_WIFI_NET_CRED_CREDENTIAL_TYPE_INVALID                  = 0x0, /**< (0x0)
                                                                         Invalid
                                                                         Credential
                                                                         Type. */
  APP_WIFI_NET_CRED_CREDENTIAL_TYPE_PSK                      = 0x1, /**< (0x1)
                                                                         Wi-Fi
                                                                         Pre-Shared
                                                                         Key
                                                                         (PSK)
                                                                         Credential. */
  APP_WIFI_NET_CRED_CREDENTIAL_TYPE_PMK                      = 0x2, /**< (0x2)
                                                                         Wi-Fi
                                                                         Pairwise
                                                                         Master
                                                                         Key
                                                                         (PMK)
                                                                         Credential. */
  APP_WIFI_NET_CRED_CREDENTIAL_TYPE_WEP                      = 0x3, /**< (0x3)
                                                                         Wi-Fi
                                                                         Wired
                                                                         Equivalent
                                                                         Privacy
                                                                         (WEP)
                                                                         Credential. */
  APP_WIFI_NET_CRED_CREDENTIAL_TYPE_CERTIFICATE              = 0x4, /**< (0x4)
                                                                         TLS
                                                                         Client
                                                                         Certificate
                                                                         Credential. */
  APP_WIFI_NET_CRED_CREDENTIAL_TYPE_PUBLIC_KEY               = 0x5, /**< (0x5)
                                                                         TLS
                                                                         Certificate
                                                                         Public
                                                                         Key
                                                                         Credential. */
  APP_WIFI_NET_CRED_CREDENTIAL_TYPE_PRIVATE_KEY              = 0x6, /**< (0x6)
                                                                         TLS
                                                                         Certificate
                                                                         Private
                                                                         Key
                                                                         Credential. */
  APP_WIFI_NET_CRED_CREDENTIAL_TYPE_PACK_FILE                = 0x7, /**< (0x7)
                                                                         EAP
                                                                         Fast
                                                                         Pack
                                                                         File
                                                                         Credential
                                                                         for EAP
                                                                         authentication. */
  APP_WIFI_NET_CRED_CREDENTIAL_TYPE_SIGNING_CERTIFICATE      = 0x8, /**< (0x8)
                                                                         TLS
                                                                         Certificate
                                                                         Authority
                                                                         (CA)
                                                                         Certificate
                                                                         Credential
                                                                         for
                                                                         signing. */
  APP_WIFI_NET_CRED_CREDENTIAL_TYPE_HTTP_CLIENT_CREDENTIAL   = 0x9, /**< (0x9)
                                                                         HTTP
                                                                         Client
                                                                         Credential
                                                                         for
                                                                         authentication. */
  APP_WIFI_NET_CRED_CREDENTIAL_TYPE_EAP_CLIENT_CREDENTIAL    = 0xa, /**< (0xa)
                                                                         Wi-Fi
                                                                         EAP
                                                                         (Extensible
                                                                         Authentication
                                                                         Protocol)
                                                                         Client
                                                                         Credential
                                                                         for
                                                                         secure
                                                                         authentication. */
  APP_WIFI_NET_CRED_CREDENTIAL_TYPE_MQTT_CLIENT_CREDENTIAL   = 0xb, /**< (0xb)
                                                                         MQTT
                                                                         Client
                                                                         Credential
                                                                         for
                                                                         secure
                                                                         communication
                                                                         in MQTT
                                                                         protocol. */
  APP_WIFI_NET_CRED_CREDENTIAL_TYPE_TLS_PRIVATE_KEY_CBC_WRAP = 0xc, /**< (0xc)
                                                                         CBC
                                                                         Wrapped
                                                                         TLS
                                                                         Private
                                                                         Certificate
                                                                         Key. */
  APP_WIFI_NET_CRED_CREDENTIAL_TYPE_TLS_PRIVATE_KEY_ECB_WRAP = 0xd  /**< (0xd)
                                                                         ECB
                                                                         Wrapped
                                                                         TLS
                                                                         Private
                                                                         Certificate
                                                                         Key. */
} app_wifi_net_cred_credential_type_t;

/**
 * @addtogroup app_wifi_common common
 * @{
 *
 * @brief common
 */

/**
 * @brief Enumeration of available Wi-Fi interface types.
 */
typedef enum {
  APP_WIFI_COMMON_WIFI_INTERFACE_INVALID                 = 0x0,  /**< (0x0)
                                                                      Invalid
                                                                      interface. */
  APP_WIFI_COMMON_WIFI_INTERFACE_CLIENT_INTERFACE        = 0x1,  /**< (0x1)
                                                                      Wi-Fi
                                                                      client
                                                                      interface. */
  APP_WIFI_COMMON_WIFI_INTERFACE_AP_INTERFACE            = 0x2,  /**< (0x2)
                                                                      Wi-Fi
                                                                      access
                                                                      point
                                                                      interface. */
  APP_WIFI_COMMON_WIFI_INTERFACE_RADIO_2_4GHZ_INTERFACE  = 0x4,  /**< (0x4) 2.4
                                                                      GHz radio
                                                                      interface. */
  APP_WIFI_COMMON_WIFI_INTERFACE_RADIO_5GHZ_INTERFACE    = 0x8,  /**< (0x8) 5
                                                                      GHz radio
                                                                      interface. */
  APP_WIFI_COMMON_WIFI_INTERFACE_DUAL_INTERFACE          = 0xc,  /**< (0xc) Dual
                                                                      radio
                                                                      interface
                                                                      (2.4 GHz
                                                                      and 5
                                                                      GHz). */
  APP_WIFI_COMMON_WIFI_INTERFACE_TRANSCEIVER_INTERFACE   = 0x80, /**< (0x80)
                                                                      Wi-Fi
                                                                      Transceiver
                                                                      mode
                                                                      interface. */
  APP_WIFI_COMMON_WIFI_INTERFACE_CLIENT_2_4GHZ_INTERFACE = 0x5,  /**< (0x5)
                                                                      Wi-Fi
                                                                      client
                                                                      interface
                                                                      on the 2.4
                                                                      GHz radio. */
  APP_WIFI_COMMON_WIFI_INTERFACE_AP_2_4GHZ_INTERFACE     = 0x6,  /**< (0x6)
                                                                      Wi-Fi
                                                                      access
                                                                      point
                                                                      interface
                                                                      on the 2.4
                                                                      GHz radio. */
  APP_WIFI_COMMON_WIFI_INTERFACE_CLIENT_5GHZ_INTERFACE   = 0x9,  /**< (0x9)
                                                                      Wi-Fi
                                                                      client
                                                                      interface
                                                                      on the 5
                                                                      GHz radio. */
  APP_WIFI_COMMON_WIFI_INTERFACE_AP_5GHZ_INTERFACE       = 0xa,  /**< (0xa)
                                                                      Wi-Fi
                                                                      access
                                                                      point
                                                                      interface
                                                                      on the 5
                                                                      GHz radio. */
  APP_WIFI_COMMON_WIFI_INTERFACE_CLIENT_DUAL_INTERFACE   = 0xd,  /**< (0xd)
                                                                      Wi-Fi
                                                                      client
                                                                      interface
                                                                      on both
                                                                      2.4 GHz
                                                                      and 5 GHz
                                                                      radios. */
  APP_WIFI_COMMON_WIFI_INTERFACE_AP_DUAL_INTERFACE       = 0xe,  /**< (0xe)
                                                                      Wi-Fi
                                                                      access
                                                                      point
                                                                      interface
                                                                      on both
                                                                      2.4 GHz
                                                                      and 5 GHz
                                                                      radios. */
  APP_WIFI_COMMON_WIFI_INTERFACE_ALL_INTERFACES          = 0x1f  /**< (0x1f) All
                                                                      available
                                                                      Wi-Fi
                                                                      interfaces
                                                                      (Client,
                                                                      Access
                                                                      Point, 2.4
                                                                      GHz, 5
                                                                      GHz). */
} app_wifi_common_wifi_interface_t;

/**
 * @addtogroup app_wifi_scan Wi-Fi Scan
 * @{
 *
 * @brief Wi-Fi Scan
 */

/**
 * @brief Defines the available types of Wi-Fi scans that can be performed.
 */
typedef enum {
  APP_WIFI_SCAN_MODE_ACTIVE              = 0x0, /**< (0x0) Transmit probe
                                                     requests and listen for
                                                     responses. */
  APP_WIFI_SCAN_MODE_PASSIVE             = 0x1, /**< (0x1) No active
                                                     transmissions, listen for
                                                     AP beacons and broadcast
                                                     probe responses. */
  APP_WIFI_SCAN_MODE_EXTENDED            = 0x2, /**< (0x2) Transmit probe
                                                     requests and listen for
                                                     responses to get more than
                                                     SL_WIFI_MAX_SCANNED_AP
                                                     number of results. */
  APP_WIFI_SCAN_MODE_PROHIBITED_CHANNELS = 0x4, /**< (0x4) Scan channels
                                                     prohibited by regulatory
                                                     region. */
  APP_WIFI_SCAN_MODE_ADVANCE             = 0x8  /**< (0x8) Scan for Access
                                                     Points while the module is
                                                     in a connected state. This
                                                     is used for background
                                                     scanning and enables the
                                                     background scan feature,
                                                     which can later be stopped
                                                     using the stop_scan
                                                     command. */
} app_wifi_scan_mode_t;

/**
 * @addtogroup app_wifi_evt_scan_results_event app_wifi_evt_scan_results_event
 * @{
 * @brief The scan_results_event event is triggered when a Wi-Fi scan completes,
 * providing details about the discovered Wi-Fi networks
 */

/***************************************************************************/ /**
 * @brief Data structure of the results_event event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_scan_results_event_s {
  uint8_t rf_channel;        /**< Displays the Wi-Fi channel on which
                                           the detected AP is operating. */
  uint8_t security_mode;     /**< Displays the security type of the
                                           detected AP. */
  int8_t rssi;               /**< Displays the Received Signal Strength
                                           Indicator (RSSI) for the detected AP. */
  uint8_t network_type;      /**< Displays the network ID associated
                                           with the detected network (AP). This
                                           ID can be used to reference the
                                           network in subsequent operations. */
  app_wifi_scan_ssid_t ssid; /**< Displays the SSID (Service Set
                                           Identifier) of the detected AP. */
  app_wifi_bssid_t bssid;    /**< The BSSID (Basic Service Set
                                           Identifier) of the detected AP. */
});

typedef struct app_wifi_evt_scan_results_event_s app_wifi_evt_scan_results_event_t;

/** @} */ // end addtogroup app_wifi_evt_scan_results_event

/**
 * @addtogroup app_wifi_evt_scan_error_event app_wifi_evt_scan_error_event
 * @{
 * @brief The error_event is triggered when a Wi-Fi scan encounters an error
 */

/***************************************************************************/ /**
 * @brief Data structure of the error_event event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_scan_error_event_s {
  uint16_t status; /**< Represents the error status of the scan operation. */
});

typedef struct app_wifi_evt_scan_error_event_s app_wifi_evt_scan_error_event_t;

/** @} */ // end addtogroup app_wifi_evt_scan_error_event

/**
 * @addtogroup app_wifi_evt_scan_complete_event app_wifi_evt_scan_complete_event
 * @{
 * @brief Represents Wi-Fi scan complete
 */

/***************************************************************************/ /**
 * @brief Data structure of the complete_event event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_scan_complete_event_s {
  uint16_t status; /**< Represents the status of the scan operation. */
});

typedef struct app_wifi_evt_scan_complete_event_s app_wifi_evt_scan_complete_event_t;

/** @} */ // end addtogroup app_wifi_evt_scan_complete_event

/**
 * @addtogroup app_wifi_ping ping
 * @{
 *
 * @brief ping
 */

/**
 * @addtogroup app_wifi_evt_ping_ipv4_response_event app_wifi_evt_ping_ipv4_response_event
 * @{
 * @brief This event holds the response data for a ping operation, including the
 * IP version, ping size, and the destination IP address
 */

/***************************************************************************/ /**
 * @brief Data structure of the ipv4_response_event event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_ping_ipv4_response_event_s {
  uint32_t status;        /**< Represents the status of the ping operation. */
  uint16_t ip_version;    /**< IP version (for example, IPv4 or IPv6). */
  uint16_t ping_size;     /**< Size of the ping packet. */
  ipv4_addr ipv4_address; /**< Destination IPv4 address. */
});

typedef struct app_wifi_evt_ping_ipv4_response_event_s app_wifi_evt_ping_ipv4_response_event_t;

/** @} */ // end addtogroup app_wifi_evt_ping_ipv4_response_event

/**
 * @addtogroup app_wifi_client client
 * @{
 *
 * @brief client
 */

/**
 * @addtogroup app_wifi_evt_client_join_event app_wifi_evt_client_join_event
 * @{
 * @brief Event triggered when the client joins a network
 */

/***************************************************************************/ /**
 * @brief Data structure of the join_event event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_client_join_event_s {
  uint8_t is_connected; /**< Indicates whether the client successfully connected
                             (1 for connected, 0 for disconnected). */
});

typedef struct app_wifi_evt_client_join_event_s app_wifi_evt_client_join_event_t;

/** @} */ // end addtogroup app_wifi_evt_client_join_event

/**
 * @addtogroup app_wifi_evt_client_stats_response_event app_wifi_evt_client_stats_response_event
 * @{
 * @brief Event triggered when the client receives Wi-Fi statistics
 */

/***************************************************************************/ /**
 * @brief Data structure of the stats_response_event event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_client_stats_response_event_s {
  uint32_t timestamp;     /**< Timestamp. This is the value of the counter
                                     at the time of message. This counter is
                                     continuously incremented by one per 100ms
                                     time. */
  uint8_t state_code;     /**< State code indicating the module's current
                                     state: State code 96 means a better access
                                     point was found while roaming. State code
                                     128 means the module is associated with an
                                     access point. */
  uint8_t reason_code;    /**< Reason code. This field provides the reason
                                     code from the firmware's perspective:
                                     Reason code 0 means no specific reason was
                                     provided. */
  uint8_t channel;        /**< Channel number. If the value of the channel
                                     is 0, it means channel information is not
                                     available. */
  uint8_t rssi;           /**< RSSI VALUE. If value of rssi is 100, it
                                     means RSSI information is not available. */
  app_wifi_bssid_t bssid; /**< BSSID of access point. If the value of
                                     access point BSSID is 00:00:00:00:00:00, it
                                     means MAC information is not available. */
});

typedef struct app_wifi_evt_client_stats_response_event_s app_wifi_evt_client_stats_response_event_t;

/** @} */ // end addtogroup app_wifi_evt_client_stats_response_event

/**
 * @addtogroup app_wifi_mqtt_client mqtt_client
 * @{
 *
 * @brief mqtt_client
 */

/**

 */
typedef enum {
  APP_WIFI_MQTT_CLIENT_TLS_FLAG_ENABLE       = 0x0, /**< (0x0) Enable TLS for
                                                         the connection. */
  APP_WIFI_MQTT_CLIENT_TLS_FLAG_TLSV_1_0     = 0x1, /**< (0x1) TLS version 1.0. */
  APP_WIFI_MQTT_CLIENT_TLS_FLAG_TLSV_1_1     = 0x2, /**< (0x2) TLS version 1.1. */
  APP_WIFI_MQTT_CLIENT_TLS_FLAG_TLSV_1_2     = 0x3, /**< (0x3) TLS version 1.2. */
  APP_WIFI_MQTT_CLIENT_TLS_FLAG_TLSV_1_3     = 0x4, /**< (0x4) TLS version 1.3. */
  APP_WIFI_MQTT_CLIENT_TLS_FLAG_CERT_INDEX_1 = 0x5, /**< (0x5) Certificate index
                                                         1 for TLS connection. */
  APP_WIFI_MQTT_CLIENT_TLS_FLAG_CERT_INDEX_2 = 0x6  /**< (0x6) Certificate index
                                                         2 for TLS connection. */
} app_wifi_mqtt_client_tls_flag_t;

/**

 */
typedef enum {
  APP_WIFI_MQTT_CLIENT_MQTT_VERSION_V3   = 0x0, /**< (0x0) MQTT protocol version
                                                     3.0. */
  APP_WIFI_MQTT_CLIENT_MQTT_VERSION_V3_1 = 0x1  /**< (0x1) PMQTT protocol
                                                     version 3.1. */
} app_wifi_mqtt_client_mqtt_version_t;

/**

 */
typedef enum {
  APP_WIFI_MQTT_CLIENT_QOS_LEVEL_0 = 0x0, /**< (0x0) At most once delivery. The
                                               message is delivered according to
                                               the best efforts of the operating
                                               environment. Message loss can
                                               occur. */
  APP_WIFI_MQTT_CLIENT_QOS_LEVEL_1 = 0x1, /**< (0x1) At least once delivery. The
                                               message is assured to arrive but
                                               duplicates can occur. */
  APP_WIFI_MQTT_CLIENT_QOS_LEVEL_2 = 0x2  /**< (0x2) Exactly once delivery. The
                                               message is assured to arrive
                                               exactly once (Not currently
                                               supported). */
} app_wifi_mqtt_client_qos_t;

/**
 * @addtogroup app_wifi_evt_mqtt_client_connected_event app_wifi_evt_mqtt_client_connected_event
 * @{
 * @brief Indicates a successful MQTT connection establishment
 */

/***************************************************************************/ /**
 * @brief Data structure of the connected_event event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_mqtt_client_connected_event_s {
  uint32_t status; /**< Unsigned 32-bit integer */
});

typedef struct app_wifi_evt_mqtt_client_connected_event_s app_wifi_evt_mqtt_client_connected_event_t;

/** @} */ // end addtogroup app_wifi_evt_mqtt_client_connected_event

/**
 * @addtogroup app_wifi_evt_mqtt_client_error_event app_wifi_evt_mqtt_client_error_event
 * @{
 * @brief Reports an error that occurred during an MQTT operation
 */

/***************************************************************************/ /**
 * @brief Data structure of the error_event event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_mqtt_client_error_event_s {
  uint16_t status; /**< SL_STATUS_OK if successful. Error code otherwise. */
});

typedef struct app_wifi_evt_mqtt_client_error_event_s app_wifi_evt_mqtt_client_error_event_t;

/** @} */ // end addtogroup app_wifi_evt_mqtt_client_error_event

/**
 * @addtogroup app_wifi_evt_mqtt_client_message_received_on_topic app_wifi_evt_mqtt_client_message_received_on_topic
 * @{
 * @brief Triggered when a new MQTT message is received on a subscribed topic
 */

/***************************************************************************/ /**
 * @brief Data structure of the message_received_on_topic event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_mqtt_client_message_received_on_topic_s {
  uint8array message_topic; /**< Description */
});

typedef struct app_wifi_evt_mqtt_client_message_received_on_topic_s
  app_wifi_evt_mqtt_client_message_received_on_topic_t;

/** @} */ // end addtogroup app_wifi_evt_mqtt_client_message_received_on_topic

/**
 * @addtogroup app_wifi_evt_mqtt_client_message_received_content app_wifi_evt_mqtt_client_message_received_content
 * @{
 * @brief Provides the content of the received MQTT message
 */

/***************************************************************************/ /**
 * @brief Data structure of the message_received_content event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_mqtt_client_message_received_content_s {
  uint8array message_content; /**< Description */
});

typedef struct app_wifi_evt_mqtt_client_message_received_content_s app_wifi_evt_mqtt_client_message_received_content_t;

/** @} */ // end addtogroup app_wifi_evt_mqtt_client_message_received_content

/**
 * @addtogroup app_wifi_evt_mqtt_client_subscribe_state_event app_wifi_evt_mqtt_client_subscribe_state_event
 * @{
 * @brief Indicates the result of an MQTT topic subscribe operation
 */

/***************************************************************************/ /**
 * @brief Data structure of the subscribe_state_event event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_mqtt_client_subscribe_state_event_s {
  uint16_t status;  /**< SL_STATUS_OK if successful. Error code otherwise. */
  uint8array topic; /**< Description */
});

typedef struct app_wifi_evt_mqtt_client_subscribe_state_event_s app_wifi_evt_mqtt_client_subscribe_state_event_t;

/** @} */ // end addtogroup app_wifi_evt_mqtt_client_subscribe_state_event

/**
 * @addtogroup app_wifi_evt_mqtt_client_unsubscribe_state_event app_wifi_evt_mqtt_client_unsubscribe_state_event
 * @{
 * @brief Indicates the result of an MQTT topic unsubscribe operation
 */

/***************************************************************************/ /**
 * @brief Data structure of the unsubscribe_state_event event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_mqtt_client_unsubscribe_state_event_s {
  uint16_t status;  /**< SL_STATUS_OK if successful. Error code otherwise. */
  uint8array topic; /**< Description */
});

typedef struct app_wifi_evt_mqtt_client_unsubscribe_state_event_s app_wifi_evt_mqtt_client_unsubscribe_state_event_t;

/** @} */ // end addtogroup app_wifi_evt_mqtt_client_unsubscribe_state_event

/**
 * @addtogroup app_wifi_evt_mqtt_client_publish_state_event app_wifi_evt_mqtt_client_publish_state_event
 * @{
 * @brief Indicates the result of an MQTT message publish operation
 */

/***************************************************************************/ /**
 * @brief Data structure of the publish_state_event event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_mqtt_client_publish_state_event_s {
  uint16_t status;  /**< SL_STATUS_OK if successful. Error code otherwise. */
  uint8array topic; /**< Description */
});

typedef struct app_wifi_evt_mqtt_client_publish_state_event_s app_wifi_evt_mqtt_client_publish_state_event_t;

/** @} */ // end addtogroup app_wifi_evt_mqtt_client_publish_state_event

/**
 * @addtogroup app_wifi_evt_mqtt_client_disconnected_event app_wifi_evt_mqtt_client_disconnected_event
 * @{
 * @brief Indicates that the MQTT connection has been disconnected
 */

/***************************************************************************/ /**
 * @brief Data structure of the disconnected_event event
 ******************************************************************************/
PACKSTRUCT(struct app_wifi_evt_mqtt_client_disconnected_event_s {
  uint32_t status; /**< Unsigned 32-bit integer */
});

typedef struct app_wifi_evt_mqtt_client_disconnected_event_s app_wifi_evt_mqtt_client_disconnected_event_t;

/** @} */ // end addtogroup app_wifi_evt_mqtt_client_disconnected_event

/***************************************************************************/ /**
 * @addtogroup app_wifi_common_types WIFI Common Types
 * @{
 *  @brief WIFI common types
 */

/**
 * @brief Data structure of WIFI API messages
 */
PACKSTRUCT(struct app_wifi_msg {
  /** API protocol header consisting of event identifier and data length */
  uint32_t header;

  /** Union of API event types */
  union {
    uint8_t handle;
    app_wifi_evt_system_boot_t evt_system_boot;   /**< Data field for system boot event*/
    app_wifi_evt_system_error_t evt_system_error; /**< Data field for system error event*/
    app_wifi_evt_net_intf_network_up_completed_t
      evt_net_intf_network_up_completed; /**< Data field for net_intf network_up_completed event*/
    app_wifi_evt_ap_client_connected_event_t
      evt_ap_client_connected_event; /**< Data field for ap client_connected_event event*/
    app_wifi_evt_ap_client_disconnected_event_t
      evt_ap_client_disconnected_event; /**< Data field for ap client_disconnected_event event*/
    app_wifi_evt_ap_get_client_info_event_t
      evt_ap_get_client_info_event;                             /**< Data field for ap get_client_info_event event*/
    app_wifi_evt_scan_results_event_t evt_scan_results_event;   /**< Data field for scan results_event event*/
    app_wifi_evt_scan_error_event_t evt_scan_error_event;       /**< Data field for scan error_event event*/
    app_wifi_evt_scan_complete_event_t evt_scan_complete_event; /**< Data field for scan complete_event event*/
    app_wifi_evt_ping_ipv4_response_event_t
      evt_ping_ipv4_response_event;                         /**< Data field for ping ipv4_response_event event*/
    app_wifi_evt_client_join_event_t evt_client_join_event; /**< Data field for client join_event event*/
    app_wifi_evt_client_stats_response_event_t
      evt_client_stats_response_event; /**< Data field for client stats_response_event event*/
    app_wifi_evt_mqtt_client_connected_event_t
      evt_mqtt_client_connected_event; /**< Data field for mqtt_client connected_event event*/
    app_wifi_evt_mqtt_client_error_event_t
      evt_mqtt_client_error_event; /**< Data field for mqtt_client error_event event*/
    app_wifi_evt_mqtt_client_message_received_on_topic_t
      evt_mqtt_client_message_received_on_topic; /**< Data field for mqtt_client message_received_on_topic event*/
    app_wifi_evt_mqtt_client_message_received_content_t
      evt_mqtt_client_message_received_content; /**< Data field for mqtt_client message_received_content event*/
    app_wifi_evt_mqtt_client_subscribe_state_event_t
      evt_mqtt_client_subscribe_state_event; /**< Data field for mqtt_client subscribe_state_event event*/
    app_wifi_evt_mqtt_client_unsubscribe_state_event_t
      evt_mqtt_client_unsubscribe_state_event; /**< Data field for mqtt_client unsubscribe_state_event event*/
    app_wifi_evt_mqtt_client_publish_state_event_t
      evt_mqtt_client_publish_state_event; /**< Data field for mqtt_client publish_state_event event*/
    app_wifi_evt_mqtt_client_disconnected_event_t
      evt_mqtt_client_disconnected_event; /**< Data field for mqtt_client disconnected_event event*/
    uint8_t payload[APP_XAPI_MAX_PAYLOAD_SIZE];
  } data;
});

/**
 * @brief Type definition for the data structure of WIFI API messages
 */
typedef struct app_wifi_msg app_wifi_msg_t;

/** @} */ // end addtogroup app_wifi_common_types
/******************************************************************************/

/***************************************************************************/ /**
 * @addtogroup app_wifi_utility_functions Utility Functions
 *  @brief Utility functions for applications on SoC
 *  @{
 */

/**
 * Get the next event that requires processing by user application. Application
 * is not blocked if no event is waiting.
 *
 * @param event the pointer for storing the new event
 * @return SL_STATUS_OK if a new event is returned, or SL_STATUS_NOT_FOUND if no
 *   event is waiting; other value indicates an error occurred
 */
sl_status_t app_wifi_pop_event(app_wifi_msg_t *event);

/**
 * Run the Bluetooth stack to process scheduled tasks. Events for user
 * application may be generated as a result of this operation.
 */
sl_status_t app_wifi_init_device(void);

sl_status_t app_wifi_push_event(app_wifi_msg_t *event);
sl_status_t app_wifi_push_event_with_data(app_wifi_msg_t *event, size_t data_len, const void *data);

void app_wifi_on_event(app_wifi_msg_t *event);
/**
 * Handle an API command in binary format.
 *
 * This is provided to NCP target applications for processing commands received
 * from NCP transport. This function is a synonym for a generic XAPI function
 * @ref app_xapi_handle_command. If the NCP application uses an RTOS, use the
 * sequence documented in @ref app_xapi_handle_command to handle the XAPI
 * command in a thread-safe manner.
 *
 * @param hdr the command header
 * @param data the command payload in a byte array
 */
void app_wifi_handle_command(uint32_t hdr, void *data);

/**
 * Get the response to the command currently been handled.
 *
 * This is provided to NCP target applications for processing commands received
 * from NCP transport. This function is a synonym for a generic XAPI function
 * @ref app_xapi_get_command_response.
 */
static inline app_wifi_msg_t *app_wifi_get_command_response()
{
  return (app_wifi_msg_t *)app_xapi_get_command_response();
}

/**
 * Signals stack to send system_error event when in case of an error.
 */
void app_wifi_send_system_error(uint16_t reason, uint8_t data_len, const uint8_t *data);

/** @} */ // end addtogroup app_wifi_utility_functions
/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif