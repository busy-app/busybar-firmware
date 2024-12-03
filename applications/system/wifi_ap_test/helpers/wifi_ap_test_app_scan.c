#include "wifi_ap_test_app_scan.h"

#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_net_wifi_types.h>
#include <sl_net.h>
#include <sl_si91x_driver.h>
#include <sl_wifi_callback_framework.h>

#include <args.h>
#include <strint.h>

#define TAG "WifiApTestAppScan"

#define WIFI_SCAN_TIMEOUT 1000000

static volatile bool scan_complete = false;
sl_wifi_scan_result_t* scan_result = NULL;
uint16_t scanbuf_size =
    (sizeof(sl_wifi_scan_result_t) + (SL_WIFI_MAX_SCANNED_AP * sizeof(scan_result->scan_info[0])));
static volatile sl_status_t callback_status = SL_STATUS_OK;

static sl_status_t show_scan_results(FuriString* msg) {
    //printf("%lu Scan results:\n", scan_result->scan_count);
    furi_string_printf(msg, "%lu Scan results:\r\n", scan_result->scan_count);

    if(scan_result->scan_count) {
        // printf("\n   %s %24s %s", "SSID", "SECURITY", "NETWORK");
        furi_string_cat_printf(msg, "\r\n   %s %24s %s", "SSID", "SECURITY", "NETWORK");
        // printf("%12s %12s %s\n", "BSSID", "CHANNEL", "RSSI");
        furi_string_cat_printf(msg, "%12s %12s %s\r\n", "BSSID", "CHANNEL", "RSSI");

        for(int a = 0; a < (int)scan_result->scan_count; ++a) {
            uint8_t* bssid = (uint8_t*)&scan_result->scan_info[a].bssid;
            // printf(
            //     "%-24s %4u,  %4u, ",
            //     scan_result->scan_info[a].ssid,
            //     scan_result->scan_info[a].security_mode,
            //     scan_result->scan_info[a].network_type);
            furi_string_cat_printf(
                msg,
                "%-24s %4u,  %4u, ",
                scan_result->scan_info[a].ssid,
                scan_result->scan_info[a].security_mode,
                scan_result->scan_info[a].network_type);
            // printf(
            //     "  %02x:%02x:%02x:%02x:%02x:%02x, %4u,  -%u\n",
            //     bssid[0],
            //     bssid[1],
            //     bssid[2],
            //     bssid[3],
            //     bssid[4],
            //     bssid[5],
            //     scan_result->scan_info[a].rf_channel,
            //     scan_result->scan_info[a].rssi_val);
            furi_string_cat_printf(
                msg,
                "  %02x:%02x:%02x:%02x:%02x:%02x, %4u,  -%u\r\n",
                bssid[0],
                bssid[1],
                bssid[2],
                bssid[3],
                bssid[4],
                bssid[5],
                scan_result->scan_info[a].rf_channel,
                scan_result->scan_info[a].rssi_val);
        }
    }

    return SL_STATUS_OK;
}

sl_status_t wlan_app_scan_callback_handler(
    sl_wifi_event_t event,
    sl_wifi_scan_result_t* result,
    uint32_t result_length,
    void* arg) {
    UNUSED_PARAMETER(result_length);
    UNUSED_PARAMETER(arg);

    scan_complete = true;

    if(SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
        callback_status = *(sl_status_t*)result;
        return SL_STATUS_FAIL;
    }

    memset(scan_result, 0, scanbuf_size);
    memcpy(scan_result, result, scanbuf_size);

    // if(result_length != 0) {
    //     callback_status = show_scan_results();
    // }

    return SL_STATUS_OK;
}

void wifi_ap_test_app_scan(FuriString* msg) {
    scan_result = (sl_wifi_scan_result_t*)malloc(scanbuf_size);
    sl_status_t status = SL_STATUS_FAIL;
    sl_wifi_scan_configuration_t wifi_scan_configuration = {0};
    wifi_scan_configuration = default_wifi_scan_configuration;

    // uint8_t channel = 2;
    // wifi_scan_configuration.channel_bitmap_2g4 = (1 << (channel - 1));

    sl_wifi_set_scan_callback(wlan_app_scan_callback_handler, NULL);

    status = sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, NULL, &wifi_scan_configuration);
    if(SL_STATUS_IN_PROGRESS == status) {
        const uint32_t start = furi_get_tick();

        while(!scan_complete && (furi_get_tick() - start) <= WIFI_SCAN_TIMEOUT) {
            furi_thread_yield();
        }
        status = scan_complete ? callback_status : SL_STATUS_TIMEOUT;

        if(scan_complete) {
            callback_status = show_scan_results(msg);
        }
    }
    if(status != SL_STATUS_OK) {
        furi_string_printf(msg, "WLAN Scan Wait Failed, Error Code : 0x%lX\r\n", status);
        furi_delay_ms(1000);
    } else {
        // // Update WLAN application state
        // wifi_app_send_to_ble(WIFI_APP_SCAN_RESP, (uint8_t*)scan_result, scanbuf_size);
    }
    if(scan_result != NULL) {
        free(scan_result);
    }
    scan_complete = false;
}
