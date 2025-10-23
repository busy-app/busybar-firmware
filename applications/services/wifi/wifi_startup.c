#include "wifi_i.h"

#include "wifi_settings.h"

void wifi_on_system_start(void) {
    Wifi* wifi = furi_record_open(RECORD_WIFI);

    do {
        WifiSettings settings;

        if(!wifi_settings_load(&settings)) {
            FURI_LOG_W(TAG, "Failed to load settings, using defaults");
            wifi_settings_init_defaults(&settings);
            wifi_settings_save(&settings);
        }

        const WifiCredentials* credentials = &settings.credentials;
        const WifiIpConfig* ip_config = &settings.ip_config;

        if(strnlen(credentials->ssid, SSID_MAX_LEN) == 0) {
            FURI_LOG_I(TAG, "No SSID specified");
            break;
        }

        if(wifi_connect(wifi, credentials, ip_config) != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to connect");
            break;
        }

    } while(false);

    furi_record_close(RECORD_WIFI);
}
