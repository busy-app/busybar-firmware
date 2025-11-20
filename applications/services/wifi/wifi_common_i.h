/**
 * @file wifi_common.h
 * @brief Common types and declarations for WiFi API - private
 */
#pragma once

#include <stdint.h>

#include "wifi_common.h"

typedef enum {
    WifiRequestTypeInit,
    WifiRequestTypeScan,
    WifiRequestTypeConnect,
    WifiRequestTypeDisconnect,
    WifiRequestTypeBackendInfo,
    WifiRequestTypeMax,
} WifiRequestType;

typedef enum {
    WifiBackendStateDisconnected,
    WifiBackendStateConnected,
    WifiBackendStateReconnecting,
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
    uint8_t state;
    uint8_t bssid[HW_ADDRESS_LEN];
    int32_t rssi;
    uint16_t channel;
} WifiBackendInfo;

typedef struct {
    uint8_t type;
    uint8_t status;
    union {
        WifiScanResults scan_results;
        WifiBackendInfo backend_info;
        uint8_t hw_address[HW_ADDRESS_LEN];
    };
} WifiResponse;
