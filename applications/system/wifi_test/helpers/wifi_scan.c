#include "wifi_scan.h"

#include <sl_wifi.h>
#include <sl_si91x_driver.h>
#include <sl_wifi_callback_framework.h>

#define TAG "WifiScan"

#define WIFI_SCAN_TIMEOUT 10000
#define MAX_SCANNED_AP    20

typedef struct {
    sl_wifi_extended_scan_result_parameters_t extended_scan_result;
    sl_wifi_extended_scan_result_t extended_scan_result_info[MAX_SCANNED_AP];
    sl_status_t callback_status;
    uint16_t scan_count;
    FuriSemaphore* scan_complete;
} WifiScan;

static sl_status_t wifi_scan_show_extended_results(WifiScan* instance, FuriString* msg) {
    furi_string_printf(msg, "%u Scan results:\r\n", *instance->extended_scan_result.result_count);

    if(*instance->extended_scan_result.result_count) {
        furi_string_cat_printf(msg, "\r\n   %s %24s %s", "SSID", "SECURITY", "NETWORK");
        furi_string_cat_printf(msg, "%12s %12s %s\r\n", "BSSID", "CHANNEL", "RSSI");

        for(int a = 0; a < (int)*instance->extended_scan_result.result_count; ++a) {
            uint8_t* bssid = (uint8_t*)&instance->extended_scan_result.scan_results[a].bssid;
            furi_string_cat_printf(
                msg,
                "%-24s %4u,  %4u, ",
                instance->extended_scan_result.scan_results[a].ssid,
                instance->extended_scan_result.scan_results[a].security_mode,
                instance->extended_scan_result.scan_results[a].network_type);
            furi_string_cat_printf(
                msg,
                "  %02x:%02x:%02x:%02x:%02x:%02x, %4u,  -%u\r\n",
                bssid[0],
                bssid[1],
                bssid[2],
                bssid[3],
                bssid[4],
                bssid[5],
                instance->extended_scan_result.scan_results[a].rf_channel,
                instance->extended_scan_result.scan_results[a].rssi);
        }
    }

    return SL_STATUS_OK;
}

sl_status_t wifi_scan_callback_handler(
    sl_wifi_event_t event,
    sl_wifi_scan_result_t* result,
    uint32_t result_length,
    void* arg) {
    UNUSED_PARAMETER(result_length);

    WifiScan* instance = (WifiScan*)arg;

    if(SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
        instance->callback_status = *(sl_status_t*)result;
        return SL_STATUS_FAIL;
    }

    instance->callback_status = SL_STATUS_OK;
    furi_semaphore_release(instance->scan_complete);

    return SL_STATUS_OK;
}

sl_status_t wifi_scan(FuriString* msg) {
    WifiScan* instance = (WifiScan*)malloc(sizeof(WifiScan));
    instance->scan_complete = furi_semaphore_alloc(1, 0);
    instance->extended_scan_result.scan_results = instance->extended_scan_result_info;
    instance->extended_scan_result.array_length =
        MAX_SCANNED_AP * sizeof(sl_wifi_extended_scan_result_t);
    instance->extended_scan_result.result_count = &instance->scan_count;
    instance->callback_status = SL_STATUS_FAIL;

    sl_status_t status = SL_STATUS_FAIL;
    sl_wifi_scan_configuration_t wifi_scan_configuration = {0};
    wifi_scan_configuration = default_wifi_scan_configuration;
    wifi_scan_configuration.type = SL_WIFI_SCAN_TYPE_EXTENDED;

    // //if set timeout scan doesn't work
    // sl_si91x_configure_timeout(SL_SI91X_CHANNEL_ACTIVE_SCAN_TIMEOUT, 2000);

    sl_wifi_set_scan_callback(wifi_scan_callback_handler, instance);
    status = sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, NULL, &wifi_scan_configuration);
    if(SL_STATUS_IN_PROGRESS == status) {
        FuriStatus satus_semaphore =
            furi_semaphore_acquire(instance->scan_complete, WIFI_SCAN_TIMEOUT);

        if(satus_semaphore == FuriStatusOk) {
            if(instance->callback_status == SL_STATUS_OK) {
                status = sl_wifi_get_stored_scan_results(
                    SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &instance->extended_scan_result);
                wifi_scan_show_extended_results(instance, msg);

                //Todo: if you need to add processing of scan results

            } else {
                status = instance->callback_status;
                furi_string_printf(msg, "WLAN Scan Failed, Error Code : 0x%lX\r\n", status);
            }
        } else {
            status = SL_STATUS_TIMEOUT;
            furi_string_printf(msg, "WLAN Scan Wait Failed, Error Code : 0x%lX\r\n", status);
        }
    }
    //Todo: delete the database of scanned points so that it does not take up space in the RAM
    sli_wifi_flush_scan_results_database();
    furi_semaphore_free(instance->scan_complete);
    free(instance);

    return status;
}