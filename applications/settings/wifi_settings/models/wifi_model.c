#include "wifi_model.h"
#include <wifi/wifi.h>

struct WifiModel {
    Wifi* wifi;
    FuriStateSub* state_sub;

    WifiModelStateCallback callback;
    void* context;
};

static void wifi_model_event_callback(const void* state, void* context) {
    WifiModel* model = context;
    furi_assert(model);
    UNUSED(state);

    if(model->callback) {
        model->callback(model->context);
    }
}

WifiModel* wifi_model_alloc(void) {
    WifiModel* model = malloc(sizeof(WifiModel));
    model->wifi = furi_record_open(RECORD_WIFI);
    model->state_sub =
        furi_state_subscribe(wifi_get_state(model->wifi), wifi_model_event_callback, model);
    return model;
}

void wifi_model_free(WifiModel* model) {
    furi_assert(model);
    model->callback = NULL;
    furi_state_unsubscribe(model->state_sub);
    furi_record_close(RECORD_WIFI);
    free(model);
}

WifiModelState wifi_model_get_state(WifiModel* model) {
    WifiInfo wifi_info;
    wifi_get_info(model->wifi, &wifi_info);

    if(wifi_info.state == WifiStateDisconnected) {
        return WifiModelStateNotConfigured;
    } else if(wifi_info.state == WifiStateConnected) {
        return WifiModelStateConnected;
    } else {
        return WifiModelStateDisconnected;
    }
}

void wifi_model_get_ssid(WifiModel* model, FuriString* ssid) {
    furi_assert(ssid);

    WifiInfo wifi_info;
    wifi_get_info(model->wifi, &wifi_info);
    furi_string_set_str(ssid, wifi_info.ssid);
}

void wifi_model_get_ip(WifiModel* model, FuriString* addr) {
    furi_assert(addr);

    WifiInfo wifi_info;
    wifi_get_info(model->wifi, &wifi_info);

    const WifiIpType type = wifi_info.ip_config.type;

    if(type == WifiIpTypeV4) {
        const uint8_t* bytes = wifi_info.ip_config.ip4.address.bytes;
        furi_string_printf(addr, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
    } else {
        furi_string_printf(addr, "IP v6"); // TODO: is v6 needed?
    }
}

void wifi_model_set_state_callback(
    WifiModel* model,
    WifiModelStateCallback callback,
    void* context) {
    furi_assert(model);
    model->context = context;
    model->callback = callback;
}

void wifi_model_forget(WifiModel* model) {
    furi_assert(model);
    wifi_disconnect(model->wifi);
}
