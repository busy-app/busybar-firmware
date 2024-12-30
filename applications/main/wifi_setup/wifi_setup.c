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
        "SSID                             ADDRESS         IP MGMT    SECURITY         \r\n");

    furi_string_cat_printf(str, "%-32s ", info->ssid);

    if(info->ip_config.type == WifiIpTypeV4) {
        const uint8_t* ip_address_v4 = info->ip_config.address.v4;
        furi_string_cat_printf(
            str,
            "%hhu.%hhu.%hhu.%hhu 4 ",
            ip_address_v4[0],
            ip_address_v4[1],
            ip_address_v4[2],
            ip_address_v4[3]);
    } else {
        const uint16_t* ip_address_v6 = (const uint16_t*)info->ip_config.address.v6;
        furi_string_cat_printf(
            str,
            "%hx:%hx:%hx:%hx:%hx:%hx:%hx:%hx 6 ",
            ip_address_v6[0],
            ip_address_v6[1],
            ip_address_v6[2],
            ip_address_v6[3],
            ip_address_v6[4],
            ip_address_v6[5],
            ip_address_v6[6],
            ip_address_v6[7]);
    }

    if(info->ip_config.mgmt == WifiIpManagementStatic) {
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
            FURI_LOG_E(TAG, "Init failed");
            break;
        }

        FURI_LOG_I(TAG, "Init OK");

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

        const WifiIpConfig ip_config = {
            .mgmt = WifiIpManagementDynamic,
            .type = WifiIpTypeV4,
        };

        status = wifi_connect(instance->wifi, &credentials, &ip_config);

        if(status != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to connect to access point");
            break;
        }

        WifiInfo info;
        status = wifi_get_info(instance->wifi, &info);

        if(status != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to get info");
            break;
        }

        FURI_LOG_I(TAG, "Connection successful");

        wifi_setup_print_info(&info);

        FURI_LOG_I(TAG, "Waiting 5 seconds before disconnecting");

        for(uint32_t i = 0; i < 5; ++i) {
            FURI_LOG_I(TAG, "%lu ...", 5 - i);
            furi_delay_ms(1000);
        }

        status = wifi_disconnect(instance->wifi);

        if(status != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to disconnect");
            break;
        }

        FURI_LOG_I(TAG, "Disconnected successfully");

        status = wifi_deinit(instance->wifi);

        if(status != WifiStatusOk) {
            FURI_LOG_E(TAG, "Deinit failed");
            break;
        }

        FURI_LOG_I(TAG, "Deinit OK");

    } while(false);

    wifi_setup_free(instance);

    return 0;
}
