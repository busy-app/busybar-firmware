#pragma once

typedef struct WifiAdaptiveTest WifiAdaptiveTest;

typedef enum {
    WifiAdaptiveTestStatusDisconnected = 0,
    WifiAdaptiveTestStatusConnected,
    WifiAdaptiveTestStatusConnecting,
    WifiAdaptiveTestStatusError,
} WifiAdaptiveTestStatus;

void wifi_adaptive_test_update(
    WifiAdaptiveTest* instance,
    WifiAdaptiveTestStatus sratus,
    FuriString* sta_ip_addr_str);
