#pragma once

#include <stdint.h>

#include "wifi_common.h"

typedef enum {
    WifiRequestTypeInit,
    WifiRequestTypeDeinit,
    WifiRequestTypeScan,
    WifiRequestTypeConnect,
    WifiRequestTypeDisconnect,
    WifiRequestTypeGetInfo,
    WifiRequestTypeMax,
} WifiRequestType;

typedef struct {
    uint8_t type;
    union {
        WifiCredentials credentials;
    };
} WifiRequest;

typedef struct {
    WifiScanResult data[SCAN_MAX_RESULTS];
    uint8_t count;
} WifiScanResults;

typedef struct {
    uint8_t type;
    uint8_t status;
    union {
        WifiScanResults scan_results;
        WifiInfo info;
    };
} WifiResponse;
