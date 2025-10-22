/**
 * @file wifi_common.h
 * @brief Common types and declarations for WiFi API - private
 */
#pragma once

#include <stdint.h>

#include "wifi_common.h"

typedef enum {
    WifiRequestTypeScan,
    WifiRequestTypeConnect,
    WifiRequestTypeDisconnect,
    WifiRequestTypeGetInfo,
    WifiRequestTypeGetHwAddress,
    WifiRequestTypeMax,
} WifiRequestType;

typedef enum {
    WifiBackendStateDisconnected,
    WifiBackendStateConnected,
    WifiBackendStateMax,
} WifiBackendState;

typedef struct {
    WifiCredentials credentials;
} WifiConnectRequest;

typedef struct {
    uint8_t type;
    union {
        WifiConnectRequest connect_request;
    };
} WifiRequest;

typedef struct {
    WifiScanResult data[SCAN_MAX_RESULTS];
    uint8_t count;
} WifiScanResults;

typedef struct {
    int32_t rssi;
    uint16_t channel;
} WifiBackendInfo;

typedef struct {
    uint8_t type;
    uint8_t status;
    union {
        WifiScanResults scan_results;
        WifiBackendInfo backend_info;
        WifiHardwareAddress hw_address;
    };
} WifiResponse;
