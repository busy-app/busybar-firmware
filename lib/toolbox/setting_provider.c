#include "setting_provider.h"

#include <toolbox/json_helper.h>

#define TAG "SettingProvider"

struct SettingProvider {
    JsonConfig* config;
    const char* file_path;
};

typedef void (*SettingProviderLoadCallback)(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value);

typedef void (*SettingProviderSaveCallback)(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value);

typedef struct {
    SettingProviderLoadCallback load;
    SettingProviderSaveCallback save;
} SettingProviderTypeActions;

/* setting types implementation */

static void setting_provider_load_bool(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderBoolInterface* interface = setting->interface;

    bool _value;
    JsonConfigStatus json_status =
        json_config_read_bool(provider->config, setting->name, &_value, &interface->default_value);

    if(json_status != JsonConfigStatusOk) {
        FURI_LOG_D(TAG, "Failed to load %s setting, using default: %d", setting->name, _value);
    }

    memcpy(value, &_value, sizeof(_value));
}

static void setting_provider_save_bool(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value) {
    bool _value;
    memcpy(&_value, value, sizeof(_value));
    JsonConfigStatus json_status = json_config_write_bool(provider->config, setting->name, _value);

    if(json_status != JsonConfigStatusOk) {
        FURI_LOG_D(TAG, "Failed to save %s setting with value: %d", setting->name, _value);
    }
}

static void setting_provider_load_int(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderIntInterface* interface = setting->interface;

    int _value;
    JsonConfigStatus json_status =
        json_config_read_int(provider->config, setting->name, &_value, &interface->default_value);

    if(json_status != JsonConfigStatusOk) {
        FURI_LOG_D(TAG, "Failed to load %s setting, using default: %d", setting->name, _value);
    } else if(interface->is_valid_callback && !interface->is_valid_callback(_value)) {
        FURI_LOG_D(
            TAG,
            "Invalid %s setting value: %d, using default: %d",
            setting->name,
            _value,
            interface->default_value);

        _value = interface->default_value;
    }

    memcpy(value, &_value, sizeof(_value));
}

static void setting_provider_save_int(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value) {
    const SettingProviderIntInterface* interface = setting->interface;

    int _value;
    memcpy(&_value, value, sizeof(_value));
    if(!interface->is_valid_callback || interface->is_valid_callback(_value)) {
        JsonConfigStatus json_status =
            json_config_write_int(provider->config, setting->name, _value);

        if(json_status != JsonConfigStatusOk) {
            FURI_LOG_D(TAG, "Failed to save %s setting with value: %d", setting->name, _value);
        }
    } else {
        FURI_LOG_D(TAG, "Invalid %s setting save attempt with value: %d", setting->name, _value);
    }
}

static void setting_provider_load_float(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderFloatInterface* interface = setting->interface;

    float _value;
    JsonConfigStatus json_status = json_config_read_number(
        provider->config, setting->name, &_value, &interface->default_value);

    if(json_status != JsonConfigStatusOk) {
        FURI_LOG_D(TAG, "Failed to load %s setting, using default: %f", setting->name, _value);
    } else if(interface->is_valid_callback && !interface->is_valid_callback(_value)) {
        FURI_LOG_D(
            TAG,
            "Invalid %s setting value: %f, using default: %f",
            setting->name,
            _value,
            interface->default_value);

        _value = interface->default_value;
    }

    memcpy(value, &_value, sizeof(_value));
}

static void setting_provider_save_float(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value) {
    const SettingProviderFloatInterface* interface = setting->interface;

    float _value;
    memcpy(&_value, value, sizeof(_value));
    if(!interface->is_valid_callback || interface->is_valid_callback(_value)) {
        JsonConfigStatus json_status =
            json_config_write_number(provider->config, setting->name, _value);

        if(json_status != JsonConfigStatusOk) {
            FURI_LOG_D(TAG, "Failed to save %s setting with value: %f", setting->name, _value);
        }
    } else {
        FURI_LOG_D(TAG, "Invalid %s setting save attempt with value: %f", setting->name, _value);
    }
}

static void setting_provider_load_string(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderStringInterface* interface = setting->interface;

    FuriString* _value = value;
    JsonConfigStatus json_status =
        json_config_read_str(provider->config, setting->name, _value, interface->default_value);

    if(json_status != JsonConfigStatusOk) {
        FURI_LOG_D(
            TAG,
            "Failed to load %s setting, using default: %s",
            setting->name,
            furi_string_get_cstr(_value));
    } else if(interface->is_valid_callback && !interface->is_valid_callback(_value)) {
        FURI_LOG_D(
            TAG,
            "Invalid %s setting value: %s, using default: %s",
            setting->name,
            furi_string_get_cstr(_value),
            interface->default_value);

        furi_string_set(_value, interface->default_value);
    }
}

static void setting_provider_save_string(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value) {
    const SettingProviderStringInterface* interface = setting->interface;

    const FuriString* _value = value;
    if(!interface->is_valid_callback || interface->is_valid_callback(_value)) {
        JsonConfigStatus json_status =
            json_config_write_str(provider->config, setting->name, furi_string_get_cstr(_value));

        if(json_status != JsonConfigStatusOk) {
            FURI_LOG_D(
                TAG,
                "Failed to save %s setting with value: %s",
                setting->name,
                furi_string_get_cstr(_value));
        }
    } else {
        FURI_LOG_D(
            TAG,
            "Invalid %s setting save attempt with value: %s",
            setting->name,
            furi_string_get_cstr(_value));
    }
}

static const SettingProviderTypeActions setting_type_actions[] = {
    [SettingProviderSettingTypeBool] =
        {
            .load = setting_provider_load_bool,
            .save = setting_provider_save_bool,
        },
    [SettingProviderSettingTypeInt] =
        {
            .load = setting_provider_load_int,
            .save = setting_provider_save_int,
        },
    [SettingProviderSettingTypeFloat] =
        {
            .load = setting_provider_load_float,
            .save = setting_provider_save_float,
        },
    [SettingProviderSettingTypeString] =
        {
            .load = setting_provider_load_string,
            .save = setting_provider_save_string,
        },
};

static_assert(COUNT_OF(setting_type_actions) == SettingProviderSettingTypesCount);

/* public api implementation */

SettingProvider* setting_provider_alloc(const char* file_path) {
    furi_check(file_path);

    SettingProvider* provider = malloc(sizeof(*provider));

    provider->config = NULL;
    provider->file_path = file_path;

    return provider;
}

void setting_provider_free(SettingProvider* provider) {
    furi_check(provider);

    if(provider->config) {
        json_config_free(provider->config);
    }

    free(provider);
}

bool setting_provider_open(SettingProvider* provider) {
    furi_check(provider);

    provider->config = json_config_alloc();
    JsonConfigStatus json_status = json_config_open(provider->config, provider->file_path);

    if(json_status != JsonConfigStatusOk && json_status != JsonConfigStatusMissing) {
        FURI_LOG_E(TAG, "Failed to open settings file: %s", provider->file_path);
        json_config_free(provider->config);
        provider->config = NULL;
    }

    return provider->config;
}

void setting_provider_load(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value) {
    furi_check(provider);
    furi_check(provider->config);
    furi_check(setting);
    furi_check(setting->type < SettingProviderSettingTypesCount);
    furi_check(value);

    setting_type_actions[setting->type].load(provider, setting, value);
}

void setting_provider_save(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value) {
    furi_check(provider);
    furi_check(provider->config);
    furi_check(setting);
    furi_check(setting->type < SettingProviderSettingTypesCount);
    furi_check(value);

    setting_type_actions[setting->type].save(provider, setting, value);
}

void setting_provider_drop(SettingProvider* provider, const SettingProviderSetting* setting) {
    furi_check(provider);
    furi_check(provider->config);
    furi_check(setting);

    json_config_delete(provider->config, setting->name);
}
