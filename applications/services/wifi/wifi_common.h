/**
 * @file wifi_common.h
 * @brief Common types and declarations for WiFi API
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum length of the SSID field. */
#define SSID_MAX_LEN       (34U)
/** Maximum length of the passphrase field. */
#define PASSPHRASE_MAX_LEN (64U)
/** Maximum number of returned scan results. */
#define SCAN_MAX_RESULTS   (28U)

/** Opaque Wifi type declaration. */
typedef struct Wifi Wifi;

/** Enumeration of possible statuses returned by Wifi functions. */
typedef enum {
    WifiStatusOk, /**< No error has occurred. */
    WifiStatusError, /**< A generic error has occurred. */
    // TODO: Add more errors
} WifiStatus;

/** Enumeration of possible states for the Wifi system. */
typedef enum {
    WifiStateDeinit, /**< The Wifi system is de-initialised. */
    WifiStateDown, /**< The Wifi system is initialised, but no connection is active. */
    WifiStateUp, /**< The Wifi system is initialised, and there is an active connection. */
} WifiState;

/** Enumeration of supported security modes. */
typedef enum {
    WifiSecurityModeOpen, /**< No password, the network is open for everyone */
    WifiSecurityModeWpa,
    WifiSecurityModeWpa2,
    WifiSecurityModeWep,
    WifiSecurityModeWpaEnterprise,
    WifiSecurityModeWpa2Enterprise,
    WifiSecurityModeWpaWpa2Mixed,
    WifiSecurityModeWpa3,
    WifiSecurityModeWpa3Transition,
    WifiSecurityModeWpa3Enterprise,
    WifiSecurityModeWpa3TransitionEnterprise,
    WifiSecurityModeMax,
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
} WifiIpManagement;

/** Enumeration of supported IP protocol types. */
typedef enum {
    WifiIpTypeV4, /**< IP version 4 */
    WifiIpTypeV6, /**< IP version 6 */
} WifiIpType;

/** IP configuration structure. */
typedef struct {
    WifiIpManagement mgmt; /**< Address management method to use */
    WifiIpType type; /**< IP version to use */
    union {
        uint8_t v4[4]; /**< Value for IP address v4 */
        uint8_t v6[16]; /**< Value for IP address v6 */
    } address; /**< IP address */
} WifiIpConfig;

/**
 * @brief Wifi information structure.
 *
 * @note If state is NOT equal to WifiStateUp, then all other fields are invalid.
 */
typedef struct {
    char ssid[SSID_MAX_LEN]; /**< Access point name (SSID) */
    WifiSecurityMode securiy_mode; /**< Type of protection used by the current access point */
    WifiIpConfig ip_config; /**< Current IP confituration */
    WifiState state; /**< State of the Wifi system */
} WifiInfo;

#ifdef __cplusplus
}
#endif
