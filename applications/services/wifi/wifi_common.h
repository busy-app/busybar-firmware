/**
 * @file wifi_common.h
 * @brief Common types and declarations for WiFi API
 */
#pragma once

#include <stdint.h>

#include <core/state.h>
#include <core/pubsub.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum length of the SSID field. */
#define SSID_MAX_LEN       (34U)
/** Maximum length of the passphrase field. */
#define PASSPHRASE_MAX_LEN (64U)
/** Maximum number of returned scan results. */
#define SCAN_MAX_RESULTS   (28U)
/** MAC address length in bytes. */
#define HW_ADDRESS_LEN     (6U)

#define WIFI_IP4_ADDR_FORMAT "%hhu.%hhu.%hhu.%hhu"
#define WIFI_IP4_ADDR_SPREAD(addr) \
    (addr)->bytes[0], (addr)->bytes[1], (addr)->bytes[2], (addr)->bytes[3]

/**
 * @brief The string key for Wifi instance access
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_WIFI)`
 */
#define RECORD_WIFI "wifi"

/** Opaque Wifi type declaration. */
typedef struct Wifi Wifi;

/** Enumeration of possible statuses returned by Wifi functions. */
typedef enum {
    WifiStatusOk, /**< No error has occurred. */
    WifiStatusError, /**< A generic error has occurred. */
    WifiStatusTimeout, /**< Command timed out. */
    WifiStatusAlreadyConnected, /**< Wifi has already been connected. */
    WifiStatusAlreadyDisconnected, /**< Wifi has already been disconnected. */
    WifiStatusScanNotPossible, /**< Failed to scan due to Wifi being connected. */
    WifiStatusAccessPointNotFound, /**< Wifi access point was not found. */
    WifiStatusAuthenticationFailed, /**< Wifi authentication failed. */
    WifiStatusConfigurationFailed, /**<  Wifi configuration failed (e.g. DHCP failure). */
    WifiStatusMax, /**< Special value, internal use */
} WifiStatus;

/** Enumeration of possible states for the Wifi system. */
typedef enum {
    WifiStateUnknown,
    WifiStateDisconnected, /**< The Wifi system is in disconnected state */
    WifiStateConnected, /**< The Wifi system is in connected state */
    WifiStateConnecting, /**< The Wifi system is trying to connect */
    WifiStateDisconnecting, /**< The Wifi system is disconnecting */
    WifiStateReconnecting, /**< The Wifi system is trying to reconnect */
    WifiStateMax, /**< Special value, internal use */
} WifiState;

/** Enumeration of supported security modes. */
typedef enum {
    WifiSecurityModeOpen, /**< No password, the network is open for everyone */
    WifiSecurityModeWpa,
    WifiSecurityModeWpa2,
    WifiSecurityModeWep,
    WifiSecurityModeWpaWpa2Mixed,
    WifiSecurityModeWpa3,
    WifiSecurityModeWpa3Transition,
    WifiSecurityModeUnsupported, /**< The security mode is not supported by this device */
    WifiSecurityModeMax, /**< Special value, internal use */
} WifiSecurityMode;

/** Credentials to connect to a Wifi access point. */
typedef struct {
    char ssid[SSID_MAX_LEN]; /**< Access point name (SSID) */
    char passphrase[PASSPHRASE_MAX_LEN]; /**< Passphrase for a protected network */
    WifiSecurityMode security_mode; /**< Type of protection to be used (usually WPA2 or 3) */
} WifiCredentials;

/** One element of the scan result array returned as a response to the scan request. */
typedef struct {
    char ssid[SSID_MAX_LEN]; /**< Access point name (SSID) */
    WifiSecurityMode security_mode; /**< Type of protection used by the AP */
    uint8_t rssi; /**< Received signal level (bigger numbers mean weaker signal) */
} WifiScanResult;

/** Enumeration of supported address management methods. */
typedef enum {
    WifiIpManagementStatic, /**< Static IP address - set manually */
    WifiIpManagementDynamic, /**< Dynamic IP address - set automatically via DHCP */
    WifiIpManagementMax, /**< Special value, internal use */
} WifiIpManagement;

/** Enumeration of supported IP protocol types. */
typedef enum {
    WifiIpTypeV4, /**< IP version 4 */
    WifiIpTypeV6, /**< IP version 6 */
    WifiIpTypeMax, /**< Special value, internal use */
} WifiIpType;

/** Union which represents IPv4 as byte sequence and single uint32_t (big-endian) */
typedef union {
    uint32_t value; ///< IPv4 address as a uint32_t
    uint8_t bytes[4]; ///< IPv4 address as uint8_t[4]
} WifiIpv4;

/** All 3 parts of IPv4. This must match to sl_net_ipv4_setting_t, because memcpy is done between them */
typedef struct {
    WifiIpv4 address;
    WifiIpv4 gateway;
    WifiIpv4 mask;
    WifiIpv4 dns;
} WifiIpv4Settings;

/** Union which represents IPv6 as byte, uint16_t, and uint32_t sequences */
typedef union {
    uint32_t value[4]; ///< IPv6 address as a uint32_t[4]
    uint16_t words[8]; ///< IPv6 address as uint16_t[8]
    uint8_t bytes[16]; ///< IPv6 address as uint8_t[16]
} WifiIpv6;

/** All 3 parts of IPv6. This must match to sl_net_ipv6_setting_t, because memcpy is done between them */
typedef struct {
    WifiIpv6 local;
    WifiIpv6 global;
    WifiIpv6 gateway;
} WifiIpv6Settings;

/** IP configuration structure. */
typedef struct {
    WifiIpManagement mgmt; /**< Address management method to use */
    WifiIpType type; /**< IP version to use */
    WifiIpv4Settings ip4;
    WifiIpv6Settings ip6;
} WifiIpConfig;

/**
 * @brief Wifi information structure.
 *
 * @note If state is NOT equal to WifiStateUp, then all other fields are invalid.
 */
typedef struct {
    char ssid[SSID_MAX_LEN]; /**< Access point name (SSID) */
    uint8_t bssid[HW_ADDRESS_LEN]; /**< Access point MAC address (BSSID) */
    int32_t rssi; /**< Signal strength (RSSI) in dBm */
    uint16_t channel; /**< Channel number */
    WifiSecurityMode security_mode; /**< Type of protection used by the current access point */
    WifiIpConfig ip_config; /**< Current IP configuration */
    WifiState state; /**< State of the Wifi system */
} WifiInfo;

#ifdef __cplusplus
}
#endif
