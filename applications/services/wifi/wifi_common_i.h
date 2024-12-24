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
    uint8_t type;
    union {
        WifiCredentials credentials;
    };
} WifiRequest;

typedef struct {
    uint8_t type;
    uint8_t status;
    union {
        struct {
            WifiScanResult data[SCAN_MAX_RESULTS];
            uint8_t count;
        } scan_results;
    };
} WifiResponse;
