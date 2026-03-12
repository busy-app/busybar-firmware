#include "device_name_settings_interface_v1.h"

#include <ctype.h>
#include <string.h>

#define DEVICE_NAME_DEFAULT "BUSY Bar"

static bool device_name_settings_v1_name_is_valid_cb(
    const SettingProviderSetting* setting,
    const char* value) {
    UNUSED(setting);

    if(strlen(value) == 0) {
        return false;
    }

    if(strlen(value) > DEVICE_NAME_MAX_LENGTH) {
        return false;
    }

    static const char* const allowed_special_chars = " !()-_=+;:,.?'|@#$%^&*[]{}/\\\"<>";

    bool only_spaces = true;

    for(size_t i = 0; i < strlen(value); i++) {
        char c = value[i];

        if(c != ' ') only_spaces = false;

        bool allowed_ascii = isalnum((unsigned char)c) || strchr(allowed_special_chars, c);
        bool is_utf8 = (unsigned char)c >= 128;

        if(!allowed_ascii || is_utf8) {
            return false;
        }
    }

    if(only_spaces) {
        return false;
    }

    return true;
}

static const SettingProviderStringInterface device_name_settings_v1_name_interface = {
    .default_value = DEVICE_NAME_DEFAULT,
    .is_valid_callback = device_name_settings_v1_name_is_valid_cb,
    .max_size = DEVICE_NAME_MAX_LENGTH,
};

const SettingProviderSetting device_name_settings_v1[] = {
    [DeviceNameSettingsV1IdxName] =
        {
            .name = "name",
            .interface = &device_name_settings_v1_name_interface,
            .field_offset = offsetof(DeviceNameSettingsV1, name),
            .type = SettingProviderSettingTypeString,
        },
};

const SettingProviderSetting device_name_settings_v1_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = device_name_settings_v1,
            .inner_settings_count = COUNT_OF(device_name_settings_v1),
        },
    .type = SettingProviderSettingTypeStruct,
};
