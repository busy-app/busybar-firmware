#include "setting_provider_i.h"
#include "types/interface.h"

#define SETTINGS_JSON_VERSION_KEY "version"
#define SETTINGS_JSON_VALUES_KEY  "values"

/* migrations implementation */

static const SettingProviderMigration* migrations_find(SettingProvider* instance, int version) {
    for(size_t i = 0; i < instance->migrations_count; i++) {
        const SettingProviderMigration* migration = &instance->migrations[i];
        if(migration->target_version == version) return migration;
    }

    return NULL;
}

static bool migrations_apply(SettingProvider* instance) {
    int stored_version = instance->json_version->valueint;
    if(stored_version == instance->settings_version) {
        FURI_LOG_D(
            TAG,
            "Version is up to date: v.%d, file: \"%s\"...",
            stored_version,
            furi_string_get_cstr(instance->file_path));
        return true;
    }

    if(stored_version > instance->settings_version) {
        FURI_LOG_W(
            TAG,
            "Stored version: v%d is newer than supported: v.%d, file: \"%s\".",
            stored_version,
            instance->settings_version,
            furi_string_get_cstr(instance->file_path));
        return false;
    }

    for(int source_version = stored_version; source_version < instance->settings_version;) {
        int target_version = source_version + 1;

        const SettingProviderMigration* migration_step = migrations_find(instance, target_version);

        if(!migration_step) {
            FURI_LOG_E(
                TAG,
                "Missing migration from: v.%d to: v.%d, file: \"%s\".",
                source_version,
                target_version,
                furi_string_get_cstr(instance->file_path));
            return false;
        }

        FURI_LOG_D(
            TAG,
            "Migrating from: v.%d to: v.%d, file: \"%s\"...",
            source_version,
            target_version,
            furi_string_get_cstr(instance->file_path));

        if(!migration_step->callback(instance)) {
            FURI_LOG_E(
                TAG,
                "Migration from: v.%d to: v.%d failed, file: \"%s\".",
                source_version,
                target_version,
                furi_string_get_cstr(instance->file_path));
            return false;
        }

        source_version = target_version;
    }

    cJSON_SetNumberValue(instance->json_version, instance->settings_version);
    instance->is_write_pending = true;

    return true;
}

/* JSON structure helpers */

static void json_structure_reset(SettingProvider* instance) {
    FURI_LOG_I(
        TAG,
        "Resetting settings JSON structure, file: \"%s\"...",
        furi_string_get_cstr(instance->file_path));

    if(instance->json_root) cJSON_Delete(instance->json_root);

    instance->json_root = cJSON_CreateObject();
    instance->json_version = cJSON_AddNumberToObject(
        instance->json_root, SETTINGS_JSON_VERSION_KEY, instance->settings_version);
    instance->json_values = cJSON_AddObjectToObject(instance->json_root, SETTINGS_JSON_VALUES_KEY);
    instance->is_write_pending = true;
}

static bool json_structure_setup(SettingProvider* instance) {
    if(!cJSON_IsObject(instance->json_root)) {
        FURI_LOG_W(
            TAG,
            "Missing or invalid JSON root object, file: \"%s\".",
            furi_string_get_cstr(instance->file_path));
        return false;
    }

    cJSON* version_item = cJSON_GetObjectItem(instance->json_root, SETTINGS_JSON_VERSION_KEY);
    if(!cJSON_IsNumber(version_item)) {
        FURI_LOG_W(
            TAG,
            "Missing or invalid \"version\" field, file: \"%s\".",
            furi_string_get_cstr(instance->file_path));
        return false;
    }

    cJSON* values_item = cJSON_GetObjectItem(instance->json_root, SETTINGS_JSON_VALUES_KEY);
    if(!cJSON_IsObject(values_item)) {
        FURI_LOG_W(
            TAG,
            "Missing or invalid \"values\" field, file: \"%s\".",
            furi_string_get_cstr(instance->file_path));
        return false;
    }

    instance->json_version = version_item;
    instance->json_values = values_item;

    return true;
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
    instance->is_write_pending = false;

    return instance;
}

void setting_provider_free(SettingProvider* instance) {
    furi_check(instance);

    furi_record_close(RECORD_STORAGE);

    cJSON_Delete(instance->json_root);
    furi_string_free(instance->file_path);
    free(instance);
}

void setting_provider_open(SettingProvider* instance) {
    furi_check(instance);
    furi_check(instance->file_path);
    furi_check(!instance->json_root);

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
            storage_file_free(file);
            free(file_buffer);
            break;
        }

        storage_file_free(file);

        file_buffer[file_size] = '\0';
        instance->json_root = cJSON_Parse(file_buffer);
        free(file_buffer);

        if(json_structure_setup(instance) && migrations_apply(instance)) return;
    } while(false);

    json_structure_reset(instance);
}

bool setting_provider_close(SettingProvider* instance) {
    furi_check(instance);
    furi_check(instance->json_root);

    if(!instance->is_write_pending) return true;

    bool is_successful = true;
    File* file = storage_file_alloc(instance->storage);

    do {
        const char* file_path = furi_string_get_cstr(instance->file_path);
        if(!storage_file_open(file, file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            FURI_LOG_E(TAG, "Failed to open file for write: \"%s\".", file_path);
            is_successful = false;
            break;
        }

        char* json_buffer = cJSON_Print(instance->json_root);
        size_t json_buffer_length = strlen(json_buffer);
        if(storage_file_write(file, json_buffer, json_buffer_length) != json_buffer_length) {
            FURI_LOG_E(TAG, "Failed to write file: \"%s\".", file_path);
            is_successful = false;
        }

        cJSON_free(json_buffer);
    } while(false);

    storage_file_free(file);

    cJSON_Delete(instance->json_root);
    instance->json_root = NULL;
    instance->is_write_pending = false;

    return is_successful;
}

bool setting_provider_save(
    SettingProvider* instance,
    const SettingProviderSetting* setting,
    const void* value) {
    furi_check(instance);
    furi_check(instance->json_root);
    furi_check(value);

    bool was_json_written = setting_provider_internal_save(instance->json_values, setting, value);
    instance->is_write_pending |= was_json_written;

    return was_json_written;
}

void setting_provider_load(
    SettingProvider* instance,
    const SettingProviderSetting* setting,
    void* value) {
    furi_check(instance);
    furi_check(instance->json_root);
    furi_check(value);

    bool was_json_written = setting_provider_internal_load(instance->json_values, setting, value);
    instance->is_write_pending |= was_json_written;
}

void setting_provider_reset(
    SettingProvider* instance,
    const SettingProviderSetting* setting,
    void* value) {
    furi_check(instance);
    furi_check(instance->json_root);

    setting_provider_internal_reset(instance->json_values, setting, value);
    instance->is_write_pending = true;
}

void setting_provider_drop(SettingProvider* instance, const SettingProviderSetting* setting) {
    furi_check(instance);
    furi_check(instance->json_root);

    if(setting) {
        furi_check(setting->name);

        cJSON_DeleteItemFromObject(instance->json_values, setting->name);
    } else {
        instance->json_values = cJSON_CreateObject();
        cJSON_ReplaceItemInObject(
            instance->json_root, SETTINGS_JSON_VALUES_KEY, instance->json_values);
    }

    instance->is_write_pending = true;
}
