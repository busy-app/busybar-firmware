#pragma once

/**
 * @file wifi_poller.h
 * Repeatedly queries Wi-Fi state and keeps track of it.
 */

#include <furi.h>
#include <wifi/wifi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WifiPoller WifiPoller;

typedef enum {
    WifiPollerStateLinkUp = (1 << 0),
} WifiPollerState;

typedef void (*WifiPollerCallback)(void* context, WifiPollerState);

WifiPoller* wifi_poller_alloc(void);

void wifi_poller_free(WifiPoller* poller);

void wifi_poller_set_callback(WifiPoller* poller, WifiPollerCallback callback, void* context);

WifiPollerState wifi_poller_get_state(WifiPoller* poller);

#ifdef __cplusplus
}
#endif
