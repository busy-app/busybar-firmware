#include "setting_provider_i.h"
#include "types/interface.h"

#define JSON_STRUCTURE_VERSION_KEY "version"
#define JSON_STRUCTURE_VALUES_KEY  "values"

typedef enum {
    MigrationResultOk,
    MigrationResultSkipped,
    MigrationResultFailure,
} MigrationResult;

/* migrations implementation */

static const SettingProviderMigration* migrations_find(SettingProvider* instance, int version) {
    for(size_t i = 0; i < instance->migrations_count; i++) {
        const SettingProviderMigration* migration = &instance->migrations[i];
        if(migration->target_version == version) return migration;
    }

    return NULL;
}

static MigrationResult migrations_apply(SettingProvider* instance) {
    int stored_version = instance->json_version->valueint;
    if(stored_version == instance->settings_version) {
        FURI_LOG_T(TAG, "Version is up to date: v.%d...", stored_version);
        return MigrationResultSkipped;
    }

    if(stored_version > instance->settings_version) {
        FURI_LOG_W(
            TAG,
            "Stored version: v%d is newer than supported: v.%d.",
            stored_version,
            instance->settings_version);
        return MigrationResultFailure;
    }

    for(int source_version = stored_version; source_version < instance->settings_version;) {
        int target_version = source_version + 1;

        const SettingProviderMigration* migration = migrations_find(instance, target_version);

        if(!migration) {
            FURI_LOG_W(
                TAG, "Missing migration from: v.%d to: v.%d.", source_version, target_version);
            return MigrationResultFailure;
        }

        FURI_LOG_D(TAG, "Migrating from: v.%d to: v.%d...", source_version, target_version);

        if(!migration->migrate_callback(instance)) {
            FURI_LOG_W(
                TAG, "Migration from: v.%d to: v.%d failed.", source_version, target_version);
            return MigrationResultFailure;
        }

        source_version = target_version;
    }

    cJSON_SetNumberValue(instance->json_version, instance->settings_version);

    return MigrationResultOk;
}

/* JSON structure helpers */

static void json_structure_reset(SettingProvider* instance) {
    FURI_LOG_T(TAG, "Resetting settings JSON structure...");

    cJSON_Delete(instance->json_root);

    instance->json_root = cJSON_CreateObject();
    instance->json_version = cJSON_AddNumberToObject(
        instance->json_root, JSON_STRUCTURE_VERSION_KEY, instance->settings_version);
    instance->json_values =
        cJSON_AddObjectToObject(instance->json_root, JSON_STRUCTURE_VALUES_KEY);
}

static bool json_structure_setup(SettingProvider* instance) {
    if(!cJSON_IsObject(instance->json_root)) {
        FURI_LOG_W(TAG, "Missing or invalid JSON root object.");
        return false;
    }

    cJSON* version_item = cJSON_GetObjectItem(instance->json_root, JSON_STRUCTURE_VERSION_KEY);
    if(!cJSON_IsNumber(version_item)) {
        FURI_LOG_W(TAG, "Missing or invalid \"version\" field.");
        return false;
    }

    cJSON* values_item = cJSON_GetObjectItem(instance->json_root, JSON_STRUCTURE_VALUES_KEY);
    if(!cJSON_IsObject(values_item)) {
        FURI_LOG_W(TAG, "Missing or invalid \"values\" field.");
        return false;
    }

    instance->json_version = version_item;
    instance->json_values = values_item;

    return true;
}

/* JSON storage helpers */

static bool storage_parse_json(SettingProvider* instance) {
    File* file = storage_file_alloc(instance->storage);

    do {
        const char* file_path = furi_string_get_cstr(instance->file_path);
        if(!storage_file_open(file, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_W(TAG, "Failed to open file for read: \"%s\".", file_path);
            break;
        }

        size_t file_size = storage_file_size(file);
        if(file_size == 0) {
            FURI_LOG_W(TAG, "File is empty: \"%s\".", file_path);
            break;
        }

        char* file_buffer = malloc(file_size + 1);
        if(storage_file_read(file, file_buffer, file_size) != file_size) {
            FURI_LOG_W(TAG, "Failed to read file: \"%s\".", file_path);
            free(file_buffer);
            break;
        }

        storage_file_free(file);

        file_buffer[file_size] = '\0';
        instance->json_root = cJSON_Parse(file_buffer);
        free(file_buffer);

        return true;
    } while(false);

    storage_file_free(file);

    return false;
}

static bool storage_flush_json(SettingProvider* instance) {
    File* file = storage_file_alloc(instance->storage);

    bool is_successful = false;
    do {
        const char* file_path = furi_string_get_cstr(instance->file_path);
        if(!storage_file_open(file, file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            FURI_LOG_E(TAG, "Failed to open file for write: \"%s\".", file_path);
            break;
        }

        char* json_string = cJSON_Print(instance->json_root);
        size_t json_string_length = strlen(json_string);
        if(storage_file_write(file, json_string, json_string_length) != json_string_length) {
            FURI_LOG_E(TAG, "Failed to write file: \"%s\".", file_path);
            cJSON_free(json_string);
            break;
        }

        cJSON_free(json_string);
        is_successful = true;
    } while(false);

    storage_file_free(file);

    cJSON_Delete(instance->json_root);
    instance->json_root = NULL;

    return is_successful;
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

    SettingProvider* instance = malloc(sizeof(*instance));

    instance->storage = furi_record_open(RECORD_STORAGE);

    instance->file_path = furi_string_alloc_set(file_path);
    storage_common_resolve_path_and_ensure_app_directory(instance->storage, instance->file_path);

    instance->migrations = migrations;
    instance->migrations_count = migrations_count;
    instance->settings_version = settings_version;

    instance->json_root = NULL;

    return instance;
}

void setting_provider_free(SettingProvider* instance) {
    furi_check(instance);

    furi_record_close(RECORD_STORAGE);

    furi_string_free(instance->file_path);
    free(instance);
}

bool setting_provider_save(
    SettingProvider* instance,
    const SettingProviderSetting* setting,
    const void* value) {
    furi_check(instance);
    furi_check(value);

    FURI_LOG_T(TAG, "Saving, file: \"%s\"...", furi_string_get_cstr(instance->file_path));

    bool is_root_call = (instance->json_root == NULL);
    json_structure_reset(instance);

    bool was_json_written = setting_provider_internal_save(instance->json_values, setting, value);
    if(is_root_call) {
        if(was_json_written) return storage_flush_json(instance);

        cJSON_Delete(instance->json_root);
        instance->json_root = NULL;
    }

    return was_json_written;
}

bool setting_provider_load(
    SettingProvider* instance,
    const SettingProviderSetting* setting,
    void* value) {
    furi_check(instance);
    furi_check(value);

    FURI_LOG_T(TAG, "Loading, file: \"%s\"...", furi_string_get_cstr(instance->file_path));

    bool was_json_written = false;
    bool is_root_call = (instance->json_root == NULL);
    if(is_root_call) {
        if(storage_parse_json(instance) && json_structure_setup(instance)) {
            switch(migrations_apply(instance)) {
            case MigrationResultOk:
                was_json_written = true;
                break;

            case MigrationResultSkipped:
                break;

            case MigrationResultFailure:
                json_structure_reset(instance);
                was_json_written = true;
                break;

            default:
                furi_crash();
            }
        } else {
            json_structure_reset(instance);
            was_json_written = true;
        }
    }

    was_json_written |= setting_provider_internal_load(instance->json_values, setting, value);
    if(is_root_call) {
        if(was_json_written) return storage_flush_json(instance);

        cJSON_Delete(instance->json_root);
        instance->json_root = NULL;
    }

    return true;
}

bool setting_provider_reset(
    SettingProvider* instance,
    const SettingProviderSetting* setting,
    void* value) {
    furi_check(instance);

    FURI_LOG_T(TAG, "Resetting, file: \"%s\"...", furi_string_get_cstr(instance->file_path));

    bool is_root_call = (instance->json_root == NULL);
    json_structure_reset(instance);

    setting_provider_internal_reset(instance->json_values, setting, value);
    return is_root_call ? storage_flush_json(instance) : true;
}

bool setting_provider_validate(const SettingProviderSetting* setting, const void* value) {
    furi_check(value);

    return setting_provider_internal_validate(setting, value);
}
