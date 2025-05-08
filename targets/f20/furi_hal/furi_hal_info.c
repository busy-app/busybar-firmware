#include <furi_hal_info.h>

#include <furi_hal_version.h>
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
                "U5"
                "firmware",
                "commit",
                "hash",
                version_get_githash(firmware_version));
        } else {
            property_value_out(
                &property_context,
                NULL,
                3,
                "U5",
                "firmware",
                "commit",
                version_get_githash(firmware_version));
        }

        property_value_out(
            &property_context,
            NULL,
            4,
            "U5",
            "firmware",
            "commit",
            "dirty",
            version_get_dirty_flag(firmware_version) ? "true" : "false");

        if(sep == '.') {
            property_value_out(
                &property_context,
                NULL,
                5,
                "U5",
                "firmware",
                "branch",
                "name",
                version_get_gitbranch(firmware_version));
        } else {
            property_value_out(
                &property_context,
                NULL,
                3,
                "U5",
                "firmware",
                "branch",
                version_get_gitbranch(firmware_version));
        }

        property_value_out(
            &property_context,
            NULL,
            4,
            "U5",
            "firmware",
            "branch",
            "num",
            version_get_gitbranchnum(firmware_version));
        property_value_out(
            &property_context,
            NULL,
            3,
            "U5",
            "firmware",
            "version",
            version_get_version(firmware_version));
        property_value_out(
            &property_context,
            NULL,
            4,
            "U5",
            "firmware",
            "build",
            "date",
            version_get_builddate(firmware_version));
        property_value_out(
            &property_context,
            "%d",
            3,
            "U5",
            "firmware",
            "target",
            version_get_target(firmware_version));

        uint16_t api_version_major, api_version_minor;
        furi_hal_info_get_api_version(&api_version_major, &api_version_minor);
        property_value_out(
            &property_context, "%d", 4, "U5", "firmware", "api", "major", api_version_major);
        property_value_out(
            &property_context, "%d", 4, "U5", "firmware", "api", "minor", api_version_minor);

        property_value_out(
            &property_context,
            NULL,
            4,
            "U5",
            "firmware",
            "origin",
            "fork",
            version_get_firmware_origin(firmware_version));

        property_value_out(
            &property_context,
            NULL,
            4,
            "U5",
            "firmware",
            "origin",
            "git",
            version_get_git_origin(firmware_version));
    }

    furi_string_free(key);
    furi_string_free(value);
}
