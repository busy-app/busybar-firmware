#include "http_api.h"
#include <power/power_service/power.h>
#include <version.h>
#include <furi_hal_version.h>
#include <furi_hal_rtc.h>
#include <toolbox/hex.h>
#include <sl_info/sl_info.h>

#define TAG "HttpStatus"

typedef struct {
    time_t boot_timestamp;
} ApiStatusCtx;

static void format_mac(FuriString* str, const uint8_t* mac) {
    furi_string_printf(
        str, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static void status_append_sl_device_info(FuriString* json_str) {
    SlInfo* sl_info = furi_record_open(RECORD_SL_INFO);

    const char* value;

    if(sl_info_get_value(sl_info, "sl_wifi_mac", &value) == SlInfoStatusOk) {
        furi_string_cat_printf(json_str, ",\"wifi_mac\":\"%s\"", value);
    }

    if(sl_info_get_value(sl_info, "sl_ble_mac", &value) == SlInfoStatusOk) {
        furi_string_cat_printf(json_str, ",\"ble_mac\":\"%s\"", value);
    }

    furi_record_close(RECORD_SL_INFO);
}

static void status_append_sl_firmware_info(FuriString* json_str) {
    SlInfo* sl_info = furi_record_open(RECORD_SL_INFO);

    const char* value;

    if(sl_info_get_value(sl_info, "sl_nwp_firmware", &value) == SlInfoStatusOk) {
        furi_string_cat_printf(json_str, ",\"nwp_version\":\"%s\"", value);
    }

    furi_record_close(RECORD_SL_INFO);
}

static bool status_get_device(FuriString* json_str, ApiStatusCtx* context) {
    UNUSED(context);

    furi_string_cat_printf(json_str, "{");

    FuriString* temp_str = furi_string_alloc();
    hex_bytes_to_string(furi_hal_version_uid(), furi_hal_version_uid_size(), temp_str);
    furi_string_cat_printf(json_str, "\"serial_number\":\"%s\"", furi_string_get_cstr(temp_str));

    const uint8_t* mac = furi_hal_version_get_usb_mac();
    format_mac(temp_str, mac);
    furi_string_cat_printf(json_str, ",\"usb_mac\":\"%s\"", furi_string_get_cstr(temp_str));

    status_append_sl_device_info(json_str);

    bool otp_valid = furi_hal_version_is_otp_valid(FuriHalOtpBlockOtp1) &&
                     furi_hal_version_is_otp_valid(FuriHalOtpBlockOtp2) &&
                     furi_hal_version_is_otp_valid(FuriHalOtpBlockOtp3) &&
                     furi_hal_version_is_otp_valid(FuriHalOtpBlockOtp4);
    furi_string_cat_printf(json_str, ",\"otp_valid\":%s", otp_valid ? "true" : "false");

    if(otp_valid) {
        furi_string_cat_printf(json_str, ",\"otp_model\":\"%s\"", furi_hal_version_get_name_ptr());
        furi_string_cat_printf(
            json_str, ",\"otp_timestamp\":%lu", furi_hal_version_get_hw_timestamp());
    }

    furi_string_cat_printf(json_str, "}");
    furi_string_free(temp_str);
    return true;
}

static bool status_get_firmware(FuriString* json_str, ApiStatusCtx* context) {
    UNUSED(context);

    const Version* firmware_version = version_get();

    furi_string_cat_printf(json_str, "{");

    furi_string_cat_printf(
        json_str,
        "\"version\":\"%s\",\"target\":%u",
        version_get_version(firmware_version),
        version_get_target(firmware_version));
    furi_string_cat_printf(
        json_str,
        ",\"branch\":\"%s\",\"build_date\":\"%s\"",
        version_get_gitbranch(firmware_version),
        version_get_builddate(firmware_version));
    furi_string_cat_printf(
        json_str,
        ",\"commit_hash\":\"%s%s\"",
        version_get_githash(firmware_version),
        version_get_dirty_flag(firmware_version) ? "-dirty" : "");

    status_append_sl_firmware_info(json_str);

    furi_string_cat_printf(json_str, "}");

    return true;
}

static bool status_get_system(FuriString* json_str, ApiStatusCtx* context) {
    furi_string_cat_printf(json_str, "{");

    const uint8_t api_ver[] = API_VERSION;
    furi_string_cat_printf(
        json_str, "\"api_semver\":\"%u.%u.%u\",", api_ver[0], api_ver[1], api_ver[2]);

    uint32_t uptime = furi_get_tick() / furi_kernel_get_tick_frequency();
    furi_string_cat_printf(
        json_str,
        "\"uptime\":\"%02lud %02luh %02lum %02lus\",",
        uptime / 60 / 60 / 24,
        uptime / 60 / 60,
        uptime / 60 % 60,
        uptime % 60);

    time_t boot_timestamp = context->boot_timestamp;
    furi_string_cat_printf(json_str, "\"boot_time\":%lld", boot_timestamp);

    furi_string_cat_printf(json_str, "}");

    return true;
}

static bool status_get_power(FuriString* json_str, ApiStatusCtx* context) {
    UNUSED(context);

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

static const struct {
    char* name;
    bool (*callback)(FuriString* json_str, ApiStatusCtx* context);
} status_handlers[] = {
    {"device", status_get_device},
    {"firmware", status_get_firmware},
    {"system", status_get_system},
    {"power", status_get_power},
};

bool http_api_status_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);

    ApiStatusCtx* context = ctx;
    furi_assert(context);

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
            success = status_handlers[i].callback(json_response, context);

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
            bool success = status_handlers[i].callback(json_response, context);

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

void* http_api_status_alloc(void) {
    ApiStatusCtx* context = malloc(sizeof(ApiStatusCtx));
    context->boot_timestamp = furi_hal_rtc_get_timestamp();
    return context;
}

void http_api_status_free(void* ctx) {
    furi_assert(ctx);
    free(ctx);
}
