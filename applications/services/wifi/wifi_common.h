/**
 * @file wifi_common.h
 * @brief Common types and declarations for WiFi API
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define SSID_MAX_LEN       (32)
#define PASSPHRASE_MAX_LEN (63)
#define SCAN_MAX_RESULTS   (20)

typedef struct Wifi Wifi;

typedef enum {
    WifiStatusOk,
    WifiStatusError,
} WifiStatus;

#ifdef __cplusplus
}
#endif
