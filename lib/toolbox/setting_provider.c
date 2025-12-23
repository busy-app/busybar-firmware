#include "setting_provider.h"

#include <storage/storage.h>
#include <cjson/cJSON.h>

#define TAG "SettingProvider"

#define SETTINGS_VERSION_KEY "version"
#define SETTINGS_VALUES_KEY  "values"

struct SettingProvider {
    char* file_path;
    const SettingProviderMigration* migrations;
    size_t migrations_count;
    int settings_version;

    cJSON* json_root;
    cJSON* json_version;
    cJSON* json_values;

    bool is_write_pending;
};

typedef void (*SettingLoadCallback)(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value);

typedef bool (*SettingSaveCallback)(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value);

typedef struct {
    SettingLoadCallback load;
    SettingSaveCallback save;
} SettingTypeActions;

static const SettingTypeActions setting_type_actions[];

/* JSON helpers */

static inline bool json_read_bool(cJSON* root, const char* key, bool* value) {
    cJSON* item = cJSON_GetObjectItem(root, key);
    return (item && cJSON_IsBool(item)) ? (*value = cJSON_IsTrue(item), true) : false;
}

static inline void json_write_bool(cJSON* root, const char* key, bool value) {
    cJSON_DeleteItemFromObject(root, key);
    cJSON_AddBoolToObject(root, key, value);
}

static inline bool json_read_int(cJSON* root, const char* key, int* value) {
    cJSON* item = cJSON_GetObjectItem(root, key);
    return (item && cJSON_IsNumber(item)) ? (*value = item->valueint, true) : false;
}

static inline void json_write_int(cJSON* root, const char* key, int value) {
    cJSON_DeleteItemFromObject(root, key);
    cJSON_AddNumberToObject(root, key, value);
}

static inline bool json_read_float(cJSON* root, const char* key, float* value) {
    cJSON* item = cJSON_GetObjectItem(root, key);
    return (item && cJSON_IsNumber(item)) ? (*value = item->valuedouble, true) : false;
}

static inline void json_write_float(cJSON* root, const char* key, float value) {
    cJSON_DeleteItemFromObject(root, key);
    cJSON_AddNumberToObject(root, key, value);
}

static inline bool json_read_string(cJSON* root, const char* key, FuriString* value) {
    cJSON* item = cJSON_GetObjectItem(root, key);
    return (item && cJSON_IsString(item)) ? (furi_string_set(value, item->valuestring), true) :
                                            false;
}

static inline void json_write_string(cJSON* root, const char* key, const char* value) {
    cJSON_DeleteItemFromObject(root, key);
    cJSON_AddStringToObject(root, key, value);
}

static inline bool json_read_custom(cJSON* root, const char* key, cJSON** value) {
    cJSON* item = cJSON_GetObjectItem(root, key);
    return (item) ? (*value = item, true) : false;
}

static inline void json_write_custom(cJSON* root, const char* key, cJSON* value) {
    cJSON_DeleteItemFromObject(root, key);
    cJSON_AddItemToObject(root, key, value);
}

/* setting types save/load implementations */

static void setting_load_bool(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderBoolInterface* interface = setting->interface;

    bool _value;
    if(!json_read_bool(provider->json_values, setting->name, &_value)) {
        _value = interface->default_value;

        FURI_LOG_W(
            TAG,
            "Failed to load \"%s\" as bool, using default: %s",
            setting->name,
            (_value) ? "true" : "false");

        json_write_bool(provider->json_values, setting->name, _value);
        provider->is_write_pending = true;
    }

    memcpy(value, &_value, sizeof(_value));
}

static bool setting_save_bool(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value) {
    bool _value;
    memcpy(&_value, value, sizeof(_value));

    json_write_bool(provider->json_values, setting->name, _value);
    provider->is_write_pending = true;

    return true;
}

static void setting_load_int(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderIntInterface* interface = setting->interface;

    int _value;
    bool is_read_successful = json_read_int(provider->json_values, setting->name, &_value);

    do {
        if(!is_read_successful) {
            FURI_LOG_W(
                TAG,
                "Failed to load \"%s\" as int, using default: %d",
                setting->name,
                interface->default_value);
        } else if(interface->is_valid_callback && !interface->is_valid_callback(_value)) {
            FURI_LOG_W(
                TAG,
                "Invalid \"%s\" value: %d, using default: %d",
                setting->name,
                _value,
                interface->default_value);
        } else {
            break;
        }

        _value = interface->default_value;
        json_write_int(provider->json_values, setting->name, _value);
        provider->is_write_pending = true;
    } while(false);

    memcpy(value, &_value, sizeof(_value));
}

static bool setting_save_int(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value) {
    const SettingProviderIntInterface* interface = setting->interface;

    int _value;
    memcpy(&_value, value, sizeof(_value));

    bool is_value_valid = (!interface->is_valid_callback || interface->is_valid_callback(_value));
    if(is_value_valid) {
        json_write_int(provider->json_values, setting->name, _value);
        provider->is_write_pending = true;
    } else {
        FURI_LOG_W(TAG, "Invalid \"%s\" save attempt with value: %d", setting->name, _value);
    }

    return is_value_valid;
}

static void setting_load_float(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderFloatInterface* interface = setting->interface;

    float _value;
    do {
        if(!json_read_float(provider->json_values, setting->name, &_value)) {
            FURI_LOG_W(
                TAG,
                "Failed to load \"%s\" as float, using default: %f",
                setting->name,
                interface->default_value);
        } else if(interface->is_valid_callback && !interface->is_valid_callback(_value)) {
            FURI_LOG_W(
                TAG,
                "Invalid \"%s\" value: %f, using default: %f",
                setting->name,
                _value,
                interface->default_value);
        } else {
            break;
        }

        _value = interface->default_value;
        json_write_float(provider->json_values, setting->name, _value);
        provider->is_write_pending = true;
    } while(false);

    memcpy(value, &_value, sizeof(_value));
}

static bool setting_save_float(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value) {
    const SettingProviderFloatInterface* interface = setting->interface;

    float _value;
    memcpy(&_value, value, sizeof(_value));

    bool is_value_valid = (!interface->is_valid_callback || interface->is_valid_callback(_value));
    if(is_value_valid) {
        json_write_float(provider->json_values, setting->name, _value);
        provider->is_write_pending = true;
    } else {
        FURI_LOG_W(TAG, "Invalid \"%s\" save attempt with value: %f", setting->name, _value);
    }

    return is_value_valid;
}

static void setting_load_string(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderStringInterface* interface = setting->interface;

    furi_check(interface->default_value);

    FuriString* _value = value;
    do {
        if(!json_read_string(provider->json_values, setting->name, _value)) {
            FURI_LOG_W(
                TAG,
                "Failed to load \"%s\" as string, using default: %s",
                setting->name,
                interface->default_value);
        } else if(interface->is_valid_callback && !interface->is_valid_callback(_value)) {
            FURI_LOG_W(
                TAG,
                "Invalid \"%s\" value: %s, using default: %s",
                setting->name,
                furi_string_get_cstr(_value),
                interface->default_value);
        } else {
            break;
        }

        furi_string_set(_value, interface->default_value);
        json_write_string(provider->json_values, setting->name, interface->default_value);
        provider->is_write_pending = true;
    } while(false);
}

static bool setting_save_string(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value) {
    const SettingProviderStringInterface* interface = setting->interface;

    const FuriString* _value = value;

    bool is_value_valid = (!interface->is_valid_callback || interface->is_valid_callback(_value));
    if(is_value_valid) {
        json_write_string(provider->json_values, setting->name, furi_string_get_cstr(_value));
        provider->is_write_pending = true;
    } else {
        FURI_LOG_W(
            TAG,
            "Invalid \"%s\" save attempt with value: %s",
            setting->name,
            furi_string_get_cstr(_value));
    }

    return is_value_valid;
}

static void setting_load_custom(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderCustomInterface* interface = setting->interface;

    furi_check(interface->default_value);
    furi_check(interface->deep_copy_callback);
    furi_check(interface->serialize_callback);
    furi_check(interface->deserialize_callback);

    cJSON* _value;
    do {
        if(!json_read_custom(provider->json_values, setting->name, &_value)) {
            FURI_LOG_W(TAG, "Failed to load \"%s\", using default", setting->name);
        } else {
            interface->deserialize_callback(value, _value);

            if(interface->is_valid_callback && !interface->is_valid_callback(value)) {
                FURI_LOG_W(TAG, "Invalid \"%s\" value, using default", setting->name);
            } else {
                break;
            }
        }

        interface->deep_copy_callback(value, interface->default_value);

        _value = cJSON_CreateObject();
        interface->serialize_callback(_value, interface->default_value);
        json_write_custom(provider->json_values, setting->name, _value);
        provider->is_write_pending = true;
    } while(false);
}

static bool setting_save_custom(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value) {
    const SettingProviderCustomInterface* interface = setting->interface;

    furi_check(interface->serialize_callback);

    bool is_value_valid = (!interface->is_valid_callback || interface->is_valid_callback(value));
    if(is_value_valid) {
        cJSON* _value = cJSON_CreateObject();
        interface->serialize_callback(_value, value);
        json_write_custom(provider->json_values, setting->name, _value);
        provider->is_write_pending = true;
    } else {
        FURI_LOG_W(TAG, "Invalid \"%s\" save attempt", setting->name);
    }

    return is_value_valid;
}

/* migrations */

static const SettingProviderMigration*
    settings_migrations_find(const SettingProvider* provider, int target_version) {
    const SettingProviderMigration* found_migration = NULL;

    for(size_t i = 0; i < provider->migrations_count; i++) {
        const SettingProviderMigration* migration = &provider->migrations[i];

        if(migration->target_version == target_version) {
            found_migration = migration;
            break;
        }
    }

    return found_migration;
}

static bool settings_migrations_apply(SettingProvider* provider) {
    bool is_success = true;

    do {
        int stored_version = provider->json_version->valueint;
        if(stored_version == provider->settings_version) {
            FURI_LOG_D(TAG, "Version is up to date: v%d...", stored_version);
            break;
        }

        if(stored_version > provider->settings_version) {
            FURI_LOG_W(
                TAG,
                "Stored version v%d is newer than supported v%d",
                stored_version,
                provider->settings_version);
            is_success = false;
            break;
        }

        for(int source_version = stored_version; source_version < provider->settings_version;) {
            int target_version = source_version + 1;

            const SettingProviderMigration* migration_step =
                settings_migrations_find(provider, target_version);

            if(!migration_step) {
                FURI_LOG_W(
                    TAG, "Missing migration from v%d to v%d", source_version, target_version);
                is_success = false;
                break;
            }

            FURI_LOG_D(TAG, "Migrating from v%d to v%d...", source_version, target_version);

            if(!migration_step->callback(provider)) {
                FURI_LOG_E(
                    TAG, "Migration from v%d to v%d failed", source_version, target_version);
                is_success = false;
                break;
            }

            source_version = target_version;
        }

        if(is_success) {
            cJSON_SetNumberValue(provider->json_version, provider->settings_version);
            provider->is_write_pending = true;
        }
    } while(false);

    return is_success;
}

/* JSON structure helpers */

static void json_structure_reset(SettingProvider* provider) {
    FURI_LOG_I(TAG, "Resetting settings JSON, file: %s", provider->file_path);

    if(provider->json_root) cJSON_Delete(provider->json_root);

    provider->json_root = cJSON_CreateObject();
    provider->json_version = cJSON_AddNumberToObject(
        provider->json_root, SETTINGS_VERSION_KEY, provider->settings_version);
    provider->json_values = cJSON_AddObjectToObject(provider->json_root, SETTINGS_VALUES_KEY);

    provider->is_write_pending = true;
}

static bool json_structure_setup(SettingProvider* provider) {
    bool is_success = false;

    do {
        if(!provider->json_root || !cJSON_IsObject(provider->json_root)) {
            FURI_LOG_W(TAG, "Missing or invalid JSON root object, file: %s", provider->file_path);
            break;
        }

        cJSON* version_item = cJSON_GetObjectItem(provider->json_root, SETTINGS_VERSION_KEY);
        if(!version_item || !cJSON_IsNumber(version_item)) {
            FURI_LOG_W(TAG, "Missing or invalid \"version\" field, file: %s", provider->file_path);
            break;
        }

        cJSON* values_item = cJSON_GetObjectItem(provider->json_root, SETTINGS_VALUES_KEY);
        if(!values_item || !cJSON_IsObject(values_item)) {
            FURI_LOG_W(TAG, "Missing or invalid \"values\" field, file: %s", provider->file_path);
            break;
        }

        provider->json_version = version_item;
        provider->json_values = values_item;

        is_success = true;
    } while(false);

    return is_success;
}

/* public api implementation */

SettingProvider* setting_provider_alloc(
    const char* file_path,
    int settings_version,
    const SettingProviderMigration* migrations,
    size_t migrations_count) {
    furi_check(file_path);
    furi_check(settings_version > 0);
    furi_check(migrations || migrations_count == 0);

    SettingProvider* provider = malloc(sizeof(*provider));

    provider->file_path = strdup(file_path);
    provider->migrations = migrations;
    provider->migrations_count = migrations_count;
    provider->settings_version = settings_version;

    provider->json_root = NULL;
    provider->json_values = NULL;
    provider->is_write_pending = false;

    return provider;
}

void setting_provider_free(SettingProvider* provider) {
    furi_check(provider);

    if(provider->json_root) cJSON_Delete(provider->json_root);

    free(provider->file_path);
    free(provider);
}

void setting_provider_open(SettingProvider* provider) {
    furi_check(provider);
    furi_check(provider->file_path);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    bool is_success = false;

    do {
        if(!storage_file_open(file, provider->file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_W(TAG, "Failed to open file for read: %s", provider->file_path);
            break;
        }

        size_t file_size = storage_file_size(file);
        if(file_size == 0) {
            break;
        }

        char* file_buffer = malloc(file_size + 1);
        if(storage_file_read(file, file_buffer, file_size) != file_size) {
            FURI_LOG_W(TAG, "Failed to read file: %s", provider->file_path);
            free(file_buffer);
            break;
        }

        provider->json_root = cJSON_Parse(file_buffer);
        free(file_buffer);

        is_success = json_structure_setup(provider) && settings_migrations_apply(provider);
    } while(false);

    if(!is_success) json_structure_reset(provider);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

bool setting_provider_close(SettingProvider* provider) {
    furi_check(provider);
    furi_check(provider->json_root);

    bool is_success = true;

    if(provider->is_write_pending) {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        File* file = storage_file_alloc(storage);

        do {
            if(!storage_file_open(file, provider->file_path, FSAM_WRITE, FSOM_OPEN_ALWAYS)) {
                FURI_LOG_E(TAG, "Failed to open file for write: %s", provider->file_path);
                is_success = false;
                break;
            }

            char* buffer = cJSON_Print(provider->json_root);
            size_t buffer_length = strlen(buffer);

            if(buffer_length != storage_file_write(file, buffer, buffer_length)) {
                FURI_LOG_E(TAG, "Failed to write file: %s", provider->file_path);
                is_success = false;
            }

            free(buffer);
        } while(false);

        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);

        provider->is_write_pending = false;
    }

    return is_success;
}

void setting_provider_load(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value) {
    furi_check(provider);
    furi_check(provider->json_root);
    furi_check(setting);
    furi_check(setting->name);
    furi_check(setting->interface);
    furi_check(setting->type < SettingProviderSettingTypesCount);
    furi_check(value);

    setting_type_actions[setting->type].load(provider, setting, value);
}

bool setting_provider_save(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value) {
    furi_check(provider);
    furi_check(provider->json_root);
    furi_check(setting);
    furi_check(setting->name);
    furi_check(setting->interface);
    furi_check(setting->type < SettingProviderSettingTypesCount);
    furi_check(value);

    return setting_type_actions[setting->type].save(provider, setting, value);
}

void setting_provider_drop(SettingProvider* provider, const SettingProviderSetting* setting) {
    furi_check(provider);
    furi_check(provider->json_root);
    furi_check(setting);
    furi_check(setting->name);

    cJSON_DeleteItemFromObject(provider->json_values, setting->name);
    provider->is_write_pending = true;
}

static const SettingTypeActions setting_type_actions[] = {
    [SettingProviderSettingTypeBool] =
        {
            .load = setting_load_bool,
            .save = setting_save_bool,
        },
    [SettingProviderSettingTypeInt] =
        {
            .load = setting_load_int,
            .save = setting_save_int,
        },
    [SettingProviderSettingTypeFloat] =
        {
            .load = setting_load_float,
            .save = setting_save_float,
        },
    [SettingProviderSettingTypeString] =
        {
            .load = setting_load_string,
            .save = setting_save_string,
        },
    [SettingProviderSettingTypeCustom] =
        {
            .load = setting_load_custom,
            .save = setting_save_custom,
        },
};

static_assert(COUNT_OF(setting_type_actions) == SettingProviderSettingTypesCount);
