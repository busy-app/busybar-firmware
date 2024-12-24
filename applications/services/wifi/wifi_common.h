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
} WifiStatus;

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
} WifiSecurityMode;

typedef struct {
    char ssid[SSID_MAX_LEN];
    char passphrase[PASSPHRASE_MAX_LEN];
    uint8_t security_mode;
} WifiCredentials;

typedef struct {
    char ssid[SSID_MAX_LEN];
    uint8_t security_mode;
    uint8_t rssi;
} WifiScanResult;

#ifdef __cplusplus
}
#endif
