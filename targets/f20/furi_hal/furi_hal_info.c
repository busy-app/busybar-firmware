#include <furi_hal_info.h>

#include <furi_hal_version.h>
#include <stm32u5xx_ll_utils.h>
#include <furi.h>

FURI_WEAK void furi_hal_info_get_api_version(uint16_t* major, uint16_t* minor) {
    *major = 0;
    *minor = 0;
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
    const Version* firmware_version = furi_hal_version_get_firmware_version();
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
        } else {
            property_value_out(
                &property_context,
                NULL,
                3,
                "u5",
                "firmware",
                "commit",
                version_get_githash(firmware_version));
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

        if(sep == '.') {
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
                "branch",
                version_get_gitbranch(firmware_version));
        }

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

        const uint8_t* mac = furi_hal_version_get_ble_mac();
        furi_string_printf(
            temp_str,
            "%02x:%02x:%02x:%02x:%02x:%02x",
            mac[0],
            mac[1],
            mac[2],
            mac[3],
            mac[4],
            mac[5]);
        property_value_out(
            &property_context, NULL, 3, "u5", "usb", "mac", furi_string_get_cstr(temp_str));
        furi_string_printf(
            temp_str, "%08lx%08lx%08lx", LL_GetUID_Word2(), LL_GetUID_Word1(), LL_GetUID_Word0());
        property_value_out(
            &property_context, NULL, 3, "u5", "hardware", "uid", furi_string_get_cstr(temp_str));
    }

    furi_string_free(temp_str);
    furi_string_free(key);
    furi_string_free(value);
}
