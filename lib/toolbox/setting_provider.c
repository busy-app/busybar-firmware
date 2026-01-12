#include "setting_provider.h"

#include <storage/storage.h>

#include <cjson/cJSON.h>

#define TAG "SettingProvider"

#define SETTINGS_JSON_VERSION_KEY "version"
#define SETTINGS_JSON_VALUES_KEY  "values"

#define IS_VALID_SETTING_TYPE(type) ((type) < SettingProviderSettingTypesCount)

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

typedef void (*SettingResetCallback)(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value);

typedef void (*SettingLoadCallback)(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value);

typedef bool (*SettingSaveCallback)(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    const void* value);

typedef struct {
    SettingResetCallback reset;
    SettingLoadCallback load;
    SettingSaveCallback save;
} SettingTypeActions;

static const SettingTypeActions setting_type_actions[];

/* JSON helpers */

static inline bool json_read_bool(cJSON* json_node, const char* key, bool* value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);
    return cJSON_IsBool(item) ? *value = cJSON_IsTrue(item), true : false;
}

static inline void json_write_bool(cJSON* json_node, const char* key, bool value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);

    if(cJSON_IsBool(item)) {
        cJSON_SetBoolValue(item, value);
    } else {
        cJSON_DeleteItemFromObject(json_node, key);
        cJSON_AddBoolToObject(json_node, key, value);
    }
}

static inline bool json_read_int(cJSON* json_node, const char* key, int* value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);
    return cJSON_IsNumber(item) ? *value = item->valueint, true : false;
}

static inline void json_write_int(cJSON* json_node, const char* key, int value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);

    if(cJSON_IsNumber(item)) {
        cJSON_SetIntValue(item, value);
    } else {
        cJSON_DeleteItemFromObject(json_node, key);
        cJSON_AddNumberToObject(json_node, key, value);
    }
}

static inline bool json_read_float(cJSON* json_node, const char* key, float* value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);
    return cJSON_IsNumber(item) ? *value = item->valuedouble, true : false;
}

static inline void json_write_float(cJSON* json_node, const char* key, float value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);

    if(cJSON_IsNumber(item)) {
        cJSON_SetNumberValue(item, value);
    } else {
        cJSON_DeleteItemFromObject(json_node, key);
        cJSON_AddNumberToObject(json_node, key, value);
    }
}

static inline bool json_read_string(cJSON* json_node, const char* key, FuriString* value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);
    return cJSON_IsString(item) ? furi_string_set(value, item->valuestring), true : false;
}

static inline void json_write_string(cJSON* json_node, const char* key, const char* value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);

    if(cJSON_IsString(item)) {
        cJSON_SetValuestring(item, value);
    } else {
        cJSON_DeleteItemFromObject(json_node, key);
        cJSON_AddStringToObject(json_node, key, value);
    }
}

static inline bool json_read_object(cJSON* json_node, const char* key, cJSON** value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);
    return cJSON_IsObject(item) ? *value = item, true : false;
}

static inline void json_write_object(cJSON* json_node, const char* key, cJSON* value) {
    cJSON_DeleteItemFromObject(json_node, key);
    cJSON_AddItemToObject(json_node, key, value);
}

/* setting reset/save/load helpers */

static void setting_reset(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    furi_check(setting);
    furi_check(setting->interface);
    furi_check(IS_VALID_SETTING_TYPE(setting->type));
    furi_check(setting->name || setting->type == SettingProviderSettingTypeStructure);

    setting_type_actions[setting->type].reset(
        provider, json_node, setting, value ? value + setting->field_offset : NULL);
}

static void setting_load(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    furi_check(setting);
    furi_check(setting->interface);
    furi_check(IS_VALID_SETTING_TYPE(setting->type));
    furi_check(setting->name || setting->type == SettingProviderSettingTypeStructure);

    setting_type_actions[setting->type].load(
        provider, json_node, setting, value + setting->field_offset);
}

static bool setting_save(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    const void* value) {
    furi_check(setting);
    furi_check(setting->interface);
    furi_check(IS_VALID_SETTING_TYPE(setting->type));
    furi_check(setting->name || setting->type == SettingProviderSettingTypeStructure);

    return setting_type_actions[setting->type].save(
        provider, json_node, setting, value + setting->field_offset);
}

/* setting types reset/save/load implementations */

static void setting_reset_bool(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderBoolInterface* interface = setting->interface;

    FURI_LOG_D(
        TAG,
        "Loading default for \"%s\": %s",
        setting->name,
        interface->default_value ? "true" : "false");

    json_write_bool(json_node, setting->name, interface->default_value);
    provider->is_write_pending = true;

    if(value) memcpy(value, &interface->default_value, sizeof(interface->default_value));
}

static void setting_load_bool(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    bool read_value;
    if(!json_read_bool(json_node, setting->name, &read_value)) {
        FURI_LOG_W(TAG, "Failed to load \"%s\" as bool...", setting->name);

        setting_reset_bool(provider, json_node, setting, value);
    } else {
        memcpy(value, &read_value, sizeof(read_value));
    }
}

static bool setting_save_bool(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    const void* value) {
    bool _value;
    memcpy(&_value, value, sizeof(_value));

    json_write_bool(json_node, setting->name, _value);
    provider->is_write_pending = true;

    return true;
}

static void setting_reset_int(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderIntInterface* interface = setting->interface;

    FURI_LOG_D(TAG, "Loading default for \"%s\": %d", setting->name, interface->default_value);

    json_write_int(json_node, setting->name, interface->default_value);
    provider->is_write_pending = true;

    if(value) memcpy(value, &interface->default_value, sizeof(interface->default_value));
}

static void setting_load_int(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderIntInterface* interface = setting->interface;

    do {
        int read_value;
        if(!json_read_int(json_node, setting->name, &read_value)) {
            FURI_LOG_W(TAG, "Failed to load \"%s\" as int...", setting->name);
        } else if(interface->is_valid_callback && !interface->is_valid_callback(setting, read_value)) {
            FURI_LOG_W(TAG, "Invalid \"%s\" value: %d...", setting->name, read_value);
        } else {
            memcpy(value, &read_value, sizeof(read_value));
            break;
        }

        setting_reset_int(provider, json_node, setting, value);
    } while(false);
}

static bool setting_save_int(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    const void* value) {
    const SettingProviderIntInterface* interface = setting->interface;

    int _value;
    memcpy(&_value, value, sizeof(_value));
    bool is_valid = !interface->is_valid_callback || interface->is_valid_callback(setting, _value);

    if(is_valid) {
        json_write_int(json_node, setting->name, _value);
        provider->is_write_pending = true;
    } else {
        FURI_LOG_W(TAG, "Invalid \"%s\" save attempt with value: %d", setting->name, _value);
    }

    return is_valid;
}

static void setting_reset_float(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderFloatInterface* interface = setting->interface;

    FURI_LOG_D(TAG, "Loading default for \"%s\": %f", setting->name, interface->default_value);

    json_write_float(json_node, setting->name, interface->default_value);
    provider->is_write_pending = true;

    if(value) memcpy(value, &interface->default_value, sizeof(interface->default_value));
}

static void setting_load_float(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderFloatInterface* interface = setting->interface;

    float read_value;
    do {
        if(!json_read_float(json_node, setting->name, &read_value)) {
            FURI_LOG_W(TAG, "Failed to load \"%s\" as float...", setting->name);
        } else if(interface->is_valid_callback && !interface->is_valid_callback(setting, read_value)) {
            FURI_LOG_W(TAG, "Invalid \"%s\" value: %f...", setting->name, read_value);
        } else {
            memcpy(value, &read_value, sizeof(read_value));
            break;
        }

        setting_reset_float(provider, json_node, setting, value);
    } while(false);
}

static bool setting_save_float(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    const void* value) {
    const SettingProviderFloatInterface* interface = setting->interface;

    float _value;
    memcpy(&_value, value, sizeof(_value));
    bool is_valid = !interface->is_valid_callback || interface->is_valid_callback(setting, _value);

    if(is_valid) {
        json_write_float(json_node, setting->name, _value);
        provider->is_write_pending = true;
    } else {
        FURI_LOG_W(TAG, "Invalid \"%s\" save attempt with value: %f", setting->name, _value);
    }

    return is_valid;
}

static void setting_reset_string(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderStringInterface* interface = setting->interface;

    furi_check(interface->default_value);
    furi_check(strlen(interface->default_value) < interface->max_length);

    FURI_LOG_D(TAG, "Loading default for \"%s\": \"%s\"", setting->name, interface->default_value);

    json_write_string(json_node, setting->name, interface->default_value);
    provider->is_write_pending = true;

    if(value) strncpy(value, interface->default_value, interface->max_length);
}

static void setting_load_string(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderStringInterface* interface = setting->interface;

    FuriString* read_buffer = furi_string_alloc();
    do {
        if(!json_read_string(json_node, setting->name, read_buffer)) {
            FURI_LOG_W(TAG, "Failed to load \"%s\" as string...", setting->name);
        } else {
            const char* read_value = furi_string_get_cstr(read_buffer);
            bool is_valid = furi_string_size(read_buffer) < interface->max_length &&
                            (!interface->is_valid_callback ||
                             interface->is_valid_callback(setting, read_value));

            if(is_valid) {
                strncpy(value, read_value, interface->max_length);
                break;
            } else {
                FURI_LOG_W(TAG, "Invalid \"%s\" value: %s...", setting->name, read_value);
            }
        }

        setting_reset_string(provider, json_node, setting, value);
    } while(false);

    furi_string_free(read_buffer);
}

static bool setting_save_string(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    const void* value) {
    const SettingProviderStringInterface* interface = setting->interface;

    const char* _value = value;
    bool is_valid =
        strlen(_value) < interface->max_length &&
        (!interface->is_valid_callback || interface->is_valid_callback(setting, _value));

    if(is_valid) {
        json_write_string(json_node, setting->name, _value);
        provider->is_write_pending = true;
    } else {
        FURI_LOG_W(TAG, "Invalid \"%s\" save attempt with value: %s", setting->name, _value);
    }

    return is_valid;
}

static void setting_reset_furi_string(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderFuriStringInterface* interface = setting->interface;

    furi_check(interface->default_value);

    FURI_LOG_D(TAG, "Loading default for \"%s\": \"%s\"", setting->name, interface->default_value);

    json_write_string(json_node, setting->name, interface->default_value);
    provider->is_write_pending = true;

    if(value) furi_string_set(value, interface->default_value);
}

static void setting_load_furi_string(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderFuriStringInterface* interface = setting->interface;

    do {
        if(!json_read_string(json_node, setting->name, value)) {
            FURI_LOG_W(TAG, "Failed to load \"%s\" as string...", setting->name);
        } else if(interface->is_valid_callback && !interface->is_valid_callback(setting, value)) {
            FURI_LOG_W(
                TAG, "Invalid \"%s\" value: %s...", setting->name, furi_string_get_cstr(value));
        } else {
            break;
        }

        setting_reset_furi_string(provider, json_node, setting, value);
    } while(false);
}

static bool setting_save_furi_string(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    const void* value) {
    const SettingProviderFuriStringInterface* interface = setting->interface;

    const FuriString* _value = value;
    bool is_valid = !interface->is_valid_callback || interface->is_valid_callback(setting, _value);

    if(is_valid) {
        json_write_string(json_node, setting->name, furi_string_get_cstr(_value));
        provider->is_write_pending = true;
    } else {
        FURI_LOG_W(
            TAG,
            "Invalid \"%s\" save attempt with value: %s",
            setting->name,
            furi_string_get_cstr(_value));
    }

    return is_valid;
}

static void setting_reset_custom(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderCustomInterface* interface = setting->interface;

    furi_check(interface->default_value);
    furi_check(interface->serialize_callback);
    furi_check(interface->default_value_size > 0);

    FuriString* _value = furi_string_alloc();
    interface->serialize_callback(setting, _value, interface->default_value);

    FURI_LOG_D(TAG, "Loading default for \"%s\": %s", setting->name, furi_string_get_cstr(_value));

    json_write_string(json_node, setting->name, furi_string_get_cstr(_value));
    furi_string_free(_value);
    provider->is_write_pending = true;

    if(value) memcpy(value, interface->default_value, interface->default_value_size);
}

static void setting_load_custom(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderCustomInterface* interface = setting->interface;

    furi_check(interface->deserialize_callback);

    FuriString* read_value = furi_string_alloc();
    do {
        if(!json_read_string(json_node, setting->name, read_value)) {
            FURI_LOG_W(TAG, "Failed to load \"%s\" as custom...", setting->name);
        } else if(!interface->deserialize_callback(setting, value, read_value)) {
            FURI_LOG_W(TAG, "Invalid \"%s\" value...", setting->name);
        } else {
            break;
        }

        setting_reset_custom(provider, json_node, setting, value);
    } while(false);

    furi_string_free(read_value);
}

static bool setting_save_custom(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    const void* value) {
    const SettingProviderCustomInterface* interface = setting->interface;

    furi_check(interface->serialize_callback);

    FuriString* _value = furi_string_alloc();
    bool is_valid = interface->serialize_callback(setting, _value, value);

    if(is_valid) {
        json_write_string(json_node, setting->name, furi_string_get_cstr(_value));
        provider->is_write_pending = true;
    } else {
        FURI_LOG_W(TAG, "Invalid \"%s\" save attempt", setting->name);
    }

    furi_string_free(_value);

    return is_valid;
}

static void setting_reset_structure(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderStructureInterface* interface = setting->interface;

    furi_check(interface->inner_settings);
    furi_check(interface->inner_settings_count > 0);

    FURI_LOG_D(TAG, "Loading default for \"%s\"", setting->name ?: "anonymous");

    cJSON* inner_json_node;
    if(setting->name) {
        if(!json_read_object(json_node, setting->name, &inner_json_node)) {
            inner_json_node = cJSON_CreateObject();
            json_write_object(json_node, setting->name, inner_json_node);
            provider->is_write_pending = true;
        }
    } else {
        inner_json_node = json_node;
    }

    for(size_t i = 0; i < interface->inner_settings_count; i++) {
        setting_reset(provider, inner_json_node, &interface->inner_settings[i], value);
    }
}

static void setting_load_structure(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    const SettingProviderStructureInterface* interface = setting->interface;

    furi_check(interface->inner_settings);
    furi_check(interface->inner_settings_count > 0);

    do {
        cJSON* inner_json_node;
        if(setting->name) {
            if(!json_read_object(json_node, setting->name, &inner_json_node)) {
                FURI_LOG_W(
                    TAG, "Failed to load \"%s\" as structure...", setting->name ?: "anonymous");
                setting_reset_structure(provider, json_node, setting, value);
                break;
            }
        } else {
            inner_json_node = json_node;
        }

        for(size_t i = 0; i < interface->inner_settings_count; i++) {
            setting_load(provider, inner_json_node, &interface->inner_settings[i], value);
        }

        if(interface->is_valid_callback && !interface->is_valid_callback(setting, value)) {
            FURI_LOG_W(TAG, "Invalid \"%s\" value...", setting->name ?: "anonymous");
            setting_reset_structure(provider, json_node, setting, value);
        }
    } while(false);
}

static bool setting_save_structure(
    SettingProvider* provider,
    cJSON* json_node,
    const SettingProviderSetting* setting,
    const void* value) {
    const SettingProviderStructureInterface* interface = setting->interface;

    furi_check(interface->inner_settings);
    furi_check(interface->inner_settings_count > 0);

    bool is_valid = !interface->is_valid_callback || interface->is_valid_callback(setting, value);

    if(is_valid) {
        cJSON* inner_json_node = cJSON_CreateObject();
        for(size_t i = 0; i < interface->inner_settings_count && is_valid; i++) {
            is_valid =
                setting_save(provider, inner_json_node, &interface->inner_settings[i], value);
        }

        if(is_valid) {
            if(setting->name) {
                json_write_object(json_node, setting->name, inner_json_node);
            } else {
                cJSON_Delete(json_node->child);
                json_node->child = inner_json_node->child;
                cJSON_free(inner_json_node);
            }

            provider->is_write_pending = true;
        } else {
            cJSON_Delete(inner_json_node);
        }
    } else {
        FURI_LOG_W(TAG, "Invalid \"%s\" save attempt", setting->name ?: "anonymous");
    }

    return is_valid;
}

/* migrations implementation */

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
            FURI_LOG_D(
                TAG, "Version is up to date: v%d, file: %s", stored_version, provider->file_path);
            break;
        }

        if(stored_version > provider->settings_version) {
            FURI_LOG_W(
                TAG,
                "Stored version: v%d is newer than supported: v%d, file: %s",
                stored_version,
                provider->settings_version,
                provider->file_path);
            is_success = false;
            break;
        }

        for(int source_version = stored_version; source_version < provider->settings_version;) {
            int target_version = source_version + 1;

            const SettingProviderMigration* migration_step =
                settings_migrations_find(provider, target_version);

            if(!migration_step) {
                FURI_LOG_E(
                    TAG, "Missing migration from: v%d to: v%d", source_version, target_version);
                is_success = false;
                break;
            }

            FURI_LOG_D(TAG, "Migrating from: v%d to: v%d...", source_version, target_version);

            if(!migration_step->callback(provider)) {
                FURI_LOG_E(
                    TAG,
                    "Migration from: v%d to: v%d failed, file: %s",
                    source_version,
                    target_version,
                    provider->file_path);
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
        provider->json_root, SETTINGS_JSON_VERSION_KEY, provider->settings_version);
    provider->json_values = cJSON_AddObjectToObject(provider->json_root, SETTINGS_JSON_VALUES_KEY);

    provider->is_write_pending = true;
}

static bool json_structure_setup(SettingProvider* provider) {
    bool is_success = false;

    do {
        if(!cJSON_IsObject(provider->json_root)) {
            FURI_LOG_W(TAG, "Missing or invalid JSON root object, file: %s", provider->file_path);
            break;
        }

        cJSON* version_item = cJSON_GetObjectItem(provider->json_root, SETTINGS_JSON_VERSION_KEY);
        if(!cJSON_IsNumber(version_item)) {
            FURI_LOG_W(TAG, "Missing or invalid \"version\" field, file: %s", provider->file_path);
            break;
        }

        cJSON* values_item = cJSON_GetObjectItem(provider->json_root, SETTINGS_JSON_VALUES_KEY);
        if(!cJSON_IsObject(values_item)) {
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
            if(!storage_file_open(file, provider->file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
                FURI_LOG_E(TAG, "Failed to open file for write: %s", provider->file_path);
                is_success = false;
                break;
            }

            char* buffer = cJSON_Print(provider->json_root);
            size_t buffer_length = strlen(buffer);
            if(storage_file_write(file, buffer, buffer_length) != buffer_length) {
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
void setting_provider_reset(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value) {
    furi_check(provider);
    furi_check(provider->json_root);

    setting_reset(provider, provider->json_values, setting, value);
}

void setting_provider_load(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value) {
    furi_check(provider);
    furi_check(provider->json_root);
    furi_check(value);

    setting_load(provider, provider->json_values, setting, value);
}

bool setting_provider_save(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value) {
    furi_check(provider);
    furi_check(provider->json_root);
    furi_check(value);

    return setting_save(provider, provider->json_values, setting, value);
}

void setting_provider_drop(SettingProvider* provider, const SettingProviderSetting* setting) {
    furi_check(provider);
    furi_check(provider->json_root);
    furi_check(setting);

    if(setting) {
        furi_check(setting->name);

        cJSON_DeleteItemFromObject(provider->json_values, setting->name);
    } else {
        cJSON_ReplaceItemInObject(
            provider->json_root, SETTINGS_JSON_VALUES_KEY, cJSON_CreateObject());
    }

    provider->is_write_pending = true;
}

static const SettingTypeActions setting_type_actions[] = {
    [SettingProviderSettingTypeBool] =
        {
            .reset = setting_reset_bool,
            .load = setting_load_bool,
            .save = setting_save_bool,
        },
    [SettingProviderSettingTypeInt] =
        {
            .reset = setting_reset_int,
            .load = setting_load_int,
            .save = setting_save_int,
        },
    [SettingProviderSettingTypeFloat] =
        {
            .reset = setting_reset_float,
            .load = setting_load_float,
            .save = setting_save_float,
        },
    [SettingProviderSettingTypeString] =
        {
            .reset = setting_reset_string,
            .load = setting_load_string,
            .save = setting_save_string,
        },
    [SettingProviderSettingTypeFuriString] =
        {
            .reset = setting_reset_furi_string,
            .load = setting_load_furi_string,
            .save = setting_save_furi_string,
        },
    [SettingProviderSettingTypeCustom] =
        {
            .reset = setting_reset_custom,
            .load = setting_load_custom,
            .save = setting_save_custom,
        },
    [SettingProviderSettingTypeStructure] =
        {
            .reset = setting_reset_structure,
            .load = setting_load_structure,
            .save = setting_save_structure,
        },
};

static_assert(COUNT_OF(setting_type_actions) == SettingProviderSettingTypesCount);
