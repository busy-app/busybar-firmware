#include <furi.h>

#include <wifi/wifi.h>

#define TAG "WifiSetup"

#define SCAN_ITERATIONS (3)

#define WIFI_SSID "Your SSID"
#define WIFI_PASS "Your passphrase"
#define WIFI_MODE (WifiSecurityModeWpa2)

typedef struct {
    Wifi* wifi;
    WifiScanResult results[SCAN_MAX_RESULTS];
} WifiSetupApp;

static const char* wifi_security_str[] = {
    "Open",
    "WPA",
    "WPA2",
    "WEP",
    "WPA Enterprise",
    "WPA2 Enterprise",
    "WPA WPA2 Mixed",
    "WPA3",
    "WPA3 Transition",
    "WPA3 Enterprise",
    "WPA3 Transition Enterprise",
};

WifiSetupApp* wifi_setup_alloc(void) {
    WifiSetupApp* instance = malloc(sizeof(WifiSetupApp));
    instance->wifi = furi_record_open(RECORD_WIFI);
    return instance;
}

void wifi_setup_free(WifiSetupApp* instance) {
    furi_record_close(RECORD_WIFI);
    free(instance);
}

static void wifi_setup_print_scan_results(const WifiScanResult* results, uint8_t results_count) {
    FuriString* str =
        furi_string_alloc_set("#  SSID                             SECURITY         RSSI\r\n");

    for(uint8_t i = 0; i < results_count; ++i) {
        const WifiScanResult* result = &results[i];

        furi_string_cat_printf(
            str,
            "%-2u %-32s %-16s %2u\r\n",
            i + 1,
            result->ssid,
            wifi_security_str[result->security_mode],
            result->rssi);
    }

    furi_log_puts(furi_string_get_cstr(str));
    furi_string_free(str);
}

static void wifi_setup_print_info(const WifiInfo* info) {
    FuriString* str = furi_string_alloc_set(
        "SSID                             IP              VER MGMT    SECURITY         \r\n");

    furi_string_cat_printf(str, "%-32s ", info->ssid);

    if(info->ip.type == WifiIpAddressTypeV4) {
        furi_string_cat_printf(
            str,
            "%hhu.%hhu.%hhu.%hhu 4 ",
            info->ip.v4[0],
            info->ip.v4[1],
            info->ip.v4[2],
            info->ip.v4[3]);
    } else {
        FURI_LOG_E(TAG, "Something's wrong: %d", info->ip.type);
        // TODO: v6 representation
    }

    if(info->ip.mgmt == WifiIpAddressMgmtStatic) {
        furi_string_cat(str, "STATIC  ");
    } else {
        furi_string_cat(str, "DYNAMIC ");
    }

    furi_string_cat_printf(str, "%-16s\r\n", wifi_security_str[info->securiy_mode]);

    furi_log_puts(furi_string_get_cstr(str));
    furi_string_free(str);
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

        for(uint32_t iter = 0; iter < SCAN_ITERATIONS; ++iter) {
            FURI_LOG_I(TAG, "Scan iteration %lu", iter + 1);

            uint8_t results_count;
            status =
                wifi_scan(instance->wifi, instance->results, &results_count, SCAN_MAX_RESULTS);

            if(status != WifiStatusOk) {
                break;
            }

            wifi_setup_print_scan_results(instance->results, results_count);
        }

        if(status != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to scan for networks");
            break;
        }

        const WifiCredentials credentials = {
            .ssid = WIFI_SSID,
            .passphrase = WIFI_PASS,
            .security_mode = WIFI_MODE,
        };

        const WifiIpAddress ip_config = {
            .mgmt = WifiIpAddressMgmtDynamic,
            .type = WifiIpAddressTypeV4,
        };

        status = wifi_connect(instance->wifi, &credentials, &ip_config);

        if(status != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to connect to Wifi network");
            break;
        }

        WifiInfo info;
        status = wifi_get_info(instance->wifi, &info);

        if(status != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to get Wifi info");
            break;
        }

        FURI_LOG_I(TAG, "Connection successful");

        wifi_setup_print_info(&info);

    } while(false);

    wifi_setup_free(instance);

    return 0;
}
