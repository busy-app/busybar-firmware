/**
 * @brief
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>

typedef void (*WifiModelStateCallback)(void* context);

typedef struct WifiModel WifiModel;

typedef enum {
    WifiModelStateNotConfigured,
    WifiModelStateDisconnected,
    WifiModelStateConnected,
} WifiModelState;

WifiModel* wifi_model_alloc(void);

void wifi_model_free(WifiModel* model);

WifiModelState wifi_model_get_state(WifiModel* model);

void wifi_model_get_ssid(WifiModel* model, FuriString* ssid);

void wifi_model_get_ip(WifiModel* model, FuriString* addr);

void wifi_model_set_state_callback(
    WifiModel* model,
    WifiModelStateCallback callback,
    void* context);

void wifi_model_forget(WifiModel* model);

#ifdef __cplusplus
}
#endif
