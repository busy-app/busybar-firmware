#include "device_name_settings_interface_v1.h"

#include "../device_name_i.h"

static bool device_name_settings_v1_name_is_valid_cb(
    const SettingProviderSetting* setting,
    const char* value) {
    UNUSED(setting);

    return device_name_validate(value) == DeviceNameErrorNone;
}

static const SettingProviderStringInterface device_name_settings_v1_name_interface = {
    .default_value = DEVICE_NAME_DEFAULT,
    .is_valid_callback = device_name_settings_v1_name_is_valid_cb,
    .max_size = DEVICE_NAME_MAX_SIZE,
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
