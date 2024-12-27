/**
 * @file wifi_common.h
 * @brief Common types and declarations for WiFi API
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SSID_MAX_LEN       (34U)
#define PASSPHRASE_MAX_LEN (64U)
#define SCAN_MAX_RESULTS   (28U)

typedef struct Wifi Wifi;

typedef enum {
    WifiStatusOk,
    WifiStatusError,
    // TODO: Add more status entries
} WifiStatus;

typedef enum {
    WifiStateDeinit,
    WifiStateDown,
    WifiStateUp,
} WifiState;

typedef enum {
    WifiSecurityModeOpen,
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

typedef struct {
    char ssid[SSID_MAX_LEN];
    char passphrase[PASSPHRASE_MAX_LEN];
    WifiSecurityMode security_mode;
} WifiCredentials;

typedef struct {
    char ssid[SSID_MAX_LEN];
    WifiSecurityMode security_mode;
    uint8_t rssi;
} WifiScanResult;

typedef enum {
    WifiIpManagementStatic,
    WifiIpManagementDynamic,
} WifiIpManagement;

typedef enum {
    WifiIpTypeV4,
    WifiIpTypeV6,
} WifiIpType;

typedef struct {
    WifiIpManagement mgmt;
    WifiIpType type;
    union {
        uint8_t v4[4];
        uint8_t v6[16];
    } address;
} WifiIpConfig;

typedef struct {
    char ssid[SSID_MAX_LEN];
    WifiSecurityMode securiy_mode;
    WifiIpConfig ip_config;
    WifiState state;
} WifiInfo;

#ifdef __cplusplus
}
#endif
