#include <furi_hal_info.h>

#include <version/version.h>
#include <furi_hal_version.h>
#include <stm32u5xx_ll_utils.h>
#include <furi.h>

FURI_WEAK void furi_hal_info_get_api_version(uint16_t* major, uint16_t* minor) {
    *major = 0;
    *minor = 0;
}

static void format_bytes_hex(FuriString* str, const uint8_t* data, size_t len) {
    furi_string_reset(str);
    for(size_t i = 0; i < len; i++) {
        furi_string_cat_printf(str, "%02x", data[i]);
    }
}

static void format_mac(FuriString* str, const uint8_t* mac) {
    furi_string_printf(
        str, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static void
    property_out_bool(PropertyValueContext* ctx, const char* k1, const char* k2, bool val) {
    property_value_out(ctx, NULL, 3, "u5", k1, k2, val ? "true" : "false");
}

static void property_out_int(PropertyValueContext* ctx, const char* k1, const char* k2, int val) {
    property_value_out(ctx, "%d", 3, "u5", k1, k2, val);
}

static void
    property_out_long(PropertyValueContext* ctx, const char* k1, const char* k2, uint32_t val) {
    property_value_out(ctx, "%lu", 3, "u5", k1, k2, val);
}

static void
    property_out_str(PropertyValueContext* ctx, const char* k1, const char* k2, const char* val) {
    property_value_out(ctx, NULL, 3, "u5", k1, k2, val);
}

static void property_out_hex(
    PropertyValueContext* ctx,
    FuriString* temp,
    const char* k1,
    const char* k2,
    const uint8_t* data,
    size_t len) {
    if(data) {
        format_bytes_hex(temp, data, len);
    } else {
        furi_string_set(temp, "null");
    }
    property_value_out(ctx, NULL, 3, "u5", k1, k2, furi_string_get_cstr(temp));
}

void furi_hal_info_get(PropertyValueCallback out, char sep, void* context) {
    UNUSED(out);
    UNUSED(sep);
    UNUSED(context);

    FuriString* key = furi_string_alloc();
    FuriString* value = furi_string_alloc();
    FuriString* temp_str = furi_string_alloc();

    PropertyValueContext property_context = {
        .key = key, .value = value, .out = out, .sep = sep, .last = false, .context = context};

    // Firmware version
    const Version* firmware_version = version_get();
    if(firmware_version) {
        if(sep == '.') {
            property_value_out(
                &property_context,
                NULL,
                4,
                "u5"
                "firmware",
                "commit",
                "hash",
                version_get_githash(firmware_version));
            property_value_out(
                &property_context,
                NULL,
                5,
                "u5",
                "firmware",
                "branch",
                "name",
                version_get_gitbranch(firmware_version));
        } else {
            property_value_out(
                &property_context,
                NULL,
                3,
                "u5",
                "firmware",
                "commit",
                version_get_githash(firmware_version));
            property_value_out(
                &property_context,
                NULL,
                3,
                "u5",
                "firmware",
                "branch",
                version_get_gitbranch(firmware_version));
        }

        property_value_out(
            &property_context,
            NULL,
            4,
            "u5",
            "firmware",
            "commit",
            "dirty",
            version_get_dirty_flag(firmware_version) ? "true" : "false");

        property_value_out(
            &property_context,
            NULL,
            3,
            "u5",
            "firmware",
            "version",
            version_get_version(firmware_version));
        property_value_out(
            &property_context,
            NULL,
            3,
            "u5",
            "firmware",
            "builddate",
            version_get_builddate(firmware_version));
        property_value_out(
            &property_context,
            "%d",
            3,
            "u5",
            "firmware",
            "target",
            version_get_target(firmware_version));

        uint16_t api_version_major, api_version_minor;
        furi_hal_info_get_api_version(&api_version_major, &api_version_minor);
        property_value_out(
            &property_context, "%d", 4, "u5", "firmware", "api", "major", api_version_major);
        property_value_out(
            &property_context, "%d", 4, "u5", "firmware", "api", "minor", api_version_minor);

        property_value_out(
            &property_context,
            NULL,
            4,
            "u5",
            "firmware",
            "origin",
            "fork",
            version_get_firmware_origin(firmware_version));
        property_value_out(
            &property_context,
            NULL,
            4,
            "u5",
            "firmware",
            "origin",
            "git",
            version_get_git_origin(firmware_version));

        // USB MAC
        const uint8_t* mac = furi_hal_version_get_usb_mac();
        if(mac) {
            format_mac(temp_str, mac);
            property_out_str(&property_context, "usb", "mac", furi_string_get_cstr(temp_str));
        }

        // Hardware UID
        furi_string_printf(
            temp_str, "%08lx%08lx%08lx", LL_GetUID_Word2(), LL_GetUID_Word1(), LL_GetUID_Word0());
        property_out_str(&property_context, "hardware", "uid", furi_string_get_cstr(temp_str));

        // OTP1 - Hardware/Production info
        property_out_bool(&property_context, "otp1", "valid", furi_hal_version_get_otp1_valid());
        property_out_str(&property_context, "otp1", "model", furi_hal_version_get_model_code());
        property_out_int(&property_context, "otp1", "version", furi_hal_version_get_hw_version());
        property_out_int(&property_context, "otp1", "target", furi_hal_version_get_hw_target());
        property_out_int(&property_context, "otp1", "body", furi_hal_version_get_hw_body());
        property_out_int(&property_context, "otp1", "connect", furi_hal_version_get_hw_connect());
        property_out_long(
            &property_context, "otp1", "timestamp", furi_hal_version_get_hw_timestamp());

        // OTP2 - QC info
        property_out_bool(&property_context, "otp2", "valid", furi_hal_version_get_otp2_valid());
        property_out_int(&property_context, "otp2", "color", furi_hal_version_get_hw_color());
        property_out_int(&property_context, "otp2", "region", furi_hal_version_get_hw_region());
        property_out_long(
            &property_context, "otp2", "timestamp", furi_hal_version_get_hw_timestamp_qc());

        // OTP3 - Public Key
        property_out_bool(&property_context, "otp3", "valid", furi_hal_version_get_otp3_valid());
        property_out_int(&property_context, "otp3", "curve", furi_hal_version_get_otp3_curve());
        property_out_hex(
            &property_context,
            temp_str,
            "otp3",
            "pubkey",
            furi_hal_version_get_otp3_pubkey(),
            furi_hal_version_get_otp3_pubkey_size());

        // OTP4 - Signatures
        property_out_bool(&property_context, "otp4", "valid", furi_hal_version_get_otp4_valid());
        property_out_hex(
            &property_context,
            temp_str,
            "otp4",
            "mcu_uid",
            furi_hal_version_get_otp4_mcu_uid(),
            12);
        property_out_hex(
            &property_context,
            temp_str,
            "otp4",
            "otp1_sig",
            furi_hal_version_get_otp4_otp1_signature(),
            furi_hal_version_get_otp4_signature_size());
        property_out_hex(
            &property_context,
            temp_str,
            "otp4",
            "otp2_sig",
            furi_hal_version_get_otp4_otp2_signature(),
            furi_hal_version_get_otp4_signature_size());
    }

    furi_string_free(temp_str);
    furi_string_free(key);
    furi_string_free(value);
}
