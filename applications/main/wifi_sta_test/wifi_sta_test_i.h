#pragma once

typedef struct WifiStaTest WifiStaTest;

typedef enum {
    WifiStaTestStatusDisconnected = 0,
    WifiStaTestStatusConnected,
    WifiStaTestStatusConnecting,
    WifiStaTestStatusError,
} WifiStaTestStatus;

void wifi_sta_test_update(
    WifiStaTest* instance,
    WifiStaTestStatus sratus,
    FuriString* sta_ip_addr_str);
