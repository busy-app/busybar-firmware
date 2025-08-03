#include "http_api.h"
#include <power/power_service/power.h>
#include <version.h>

#define TAG "HttpStatus"

bool status_get_system(FuriString* json_str) {
    const Version* firmware_version = version_get();

    furi_string_cat_printf(json_str, "{");

    furi_string_cat_printf(json_str, "\"version\":\"%s\",", version_get_version(firmware_version));
    furi_string_cat_printf(
        json_str,
        "\"branch\":\"%s\",\"build_date\":\"%s\",",
        version_get_gitbranch(firmware_version),
        version_get_builddate(firmware_version));
    furi_string_cat_printf(
        json_str,
        "\"commit_hash\":\"%s%s\",",
        version_get_githash(firmware_version),
        version_get_dirty_flag(firmware_version) ? "-dirty" : "");

    uint32_t uptime = furi_get_tick() / furi_kernel_get_tick_frequency();
    furi_string_cat_printf(
        json_str,
        "\"uptime\":\"%02lud %02luh %02lum %02lus\"",
        uptime / 60 / 60 / 24,
        uptime / 60 / 60,
        uptime / 60 % 60,
        uptime % 60);

    furi_string_cat_printf(json_str, "}");

    return true;
}

bool status_get_power(FuriString* json_str) {
    Power* power = furi_record_open(RECORD_POWER);
    PowerInfo info;
    power_get_info(power, &info);
    furi_record_close(RECORD_POWER);

    furi_string_cat_printf(json_str, "{");

    if(info.is_charging) {
        furi_string_cat_printf(
            json_str, "\"%s\":\"%s\",", "state", (info.is_full_charged) ? "charged" : "charging");
    } else {
        furi_string_cat_printf(json_str, "\"%s\":\"%s\",", "state", "discharging");
    }

    furi_string_cat_printf(json_str, "\"%s\":%u,", "battery_charge", info.charge);
    furi_string_cat_printf(
        json_str, "\"%s\":%lu,", "battery_voltage", (uint32_t)info.voltage_battery);
    furi_string_cat_printf(json_str, "\"%s\":%ld,", "battery_current", info.current_battery);
    furi_string_cat_printf(json_str, "\"%s\":%lu", "usb_voltage", (uint32_t)info.voltage_usb);

    furi_string_cat_printf(json_str, "}");

    return true;
}

bool status_get_ble(FuriString* json_str) {
    Power* power = furi_record_open(RECORD_POWER);
    PowerInfo info;
    power_get_info(power, &info);
    furi_record_close(RECORD_POWER);

    furi_string_cat_printf(json_str, "{");

    furi_string_cat_printf(json_str, "\"%s\":\"%s\"", "state", "not implemented");
    // TODO: BLE is not implemented

    furi_string_cat_printf(json_str, "}");

    return true;
}

static const struct {
    char* name;
    bool (*callback)(FuriString* json_str);
} status_handlers[] = {
    {"system", status_get_system},
    {"power", status_get_power},
    // {"wifi", status_get_wifi}, // Implemented in /api/wifi/status
    {"ble", status_get_ble},
};

bool http_api_status_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    UNUSED(msg);

    if(IS_HTTP_ENDPOINT(path)) {
        FuriString* json_response = furi_string_alloc();

        bool success = false;
        bool is_first = true;
        for(size_t i = 0; i < COUNT_OF(status_handlers); i++) {
            if(!is_first) {
                furi_string_cat(json_response, ",");
            }
            is_first = false;
            furi_string_cat_printf(json_response, "\"%s\":", status_handlers[i].name);

            furi_assert(status_handlers[i].callback);
            success = status_handlers[i].callback(json_response);

            if(!success) break;
        }

        if(success) {
            MG_REPLY_OK_BODY(conn, "{%s}\n", furi_string_get_cstr(json_response));
        } else {
            MG_REPLY_INTERNAL_ERROR(conn, "Failed to get status");
        }

        furi_string_free(json_response);
        return true;
    }

    for(size_t i = 0; i < COUNT_OF(status_handlers); i++) {
        if(furi_string_equal(path, status_handlers[i].name)) {
            FuriString* json_response = furi_string_alloc();

            furi_assert(status_handlers[i].callback);
            bool success = status_handlers[i].callback(json_response);

            if(success) {
                MG_REPLY_OK_BODY(conn, "%s\n", furi_string_get_cstr(json_response));
            } else {
                MG_REPLY_INTERNAL_ERROR(conn, "Failed to get status");
            }

            furi_string_free(json_response);
            return true;
        }
    }

    return false;
}
