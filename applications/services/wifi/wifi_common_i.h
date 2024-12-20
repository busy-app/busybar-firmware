#pragma once

#include <stdint.h>

#include "wifi_common.h"

typedef enum {
    WifiRequestTypeInit,
    WifiRequestTypeDeinit,
    WifiRequestTypeScan,
    WifiRequestTypeConnect,
    WifiRequestTypeDisconnect,
    WifiRequestTypeMax,
} WifiRequestType;

typedef struct {
    // TODO: Add more network params
    char ssid[SSID_MAX_LEN + 1];
    char passphrase[PASSPHRASE_MAX_LEN + 1];
} WifiCredentials;

typedef struct {
    WifiRequestType type;
    union {
        WifiCredentials credentials;
    };
} WifiRequest;

typedef struct {
    char ssid[SSID_MAX_LEN + 1];
} WifiScanResult;

typedef struct {
    WifiRequestType type;
    WifiStatus status;
    union {
        WifiScanResult scan_results[SCAN_MAX_RESULTS];
    };
} WifiResponse;
