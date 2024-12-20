#include <furi.h>

#include <wifi/wifi.h>

#define TAG "WifiSetup"

typedef struct {
    Wifi* wifi;
    FuriString* ssids[SCAN_MAX_RESULTS];
} WifiSetupApp;

WifiSetupApp* wifi_setup_alloc(void) {
    WifiSetupApp* instance = malloc(sizeof(WifiSetupApp));

    instance->wifi = furi_record_open(RECORD_WIFI);

    for(uint32_t i = 0; i < SCAN_MAX_RESULTS; ++i) {
        instance->ssids[i] = furi_string_alloc();
    }

    return instance;
}

void wifi_setup_free(WifiSetupApp* instance) {
    furi_record_close(RECORD_WIFI);

    for(uint32_t i = 0; i < SCAN_MAX_RESULTS; ++i) {
        furi_string_free(instance->ssids[i]);
    }

    free(instance);
}

int32_t wifi_setup_app(void* arg) {
    UNUSED(arg);

    WifiSetupApp* instance = wifi_setup_alloc();

    do {
        WifiStatus status;
        status = wifi_init(instance->wifi);

        if(status != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to initialise Wifi");
            break;
        }

        FURI_LOG_I(TAG, "Wifi Init OK");

        status = wifi_scan(instance->wifi, instance->ssids, SCAN_MAX_RESULTS);

        if(status != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to scan for networks");
            break;
        }

        for(uint32_t i = 0; i < SCAN_MAX_RESULTS; ++i) {
            const FuriString* ssid = instance->ssids[i];
            if(furi_string_empty(ssid)) break;
            FURI_LOG_I(TAG, "SSID: %s", furi_string_get_cstr(ssid));
        }

    } while(false);

    wifi_setup_free(instance);

    return 0;
}
