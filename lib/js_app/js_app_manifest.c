#include "js_app_manifest.h"

#include <core/check.h>
#include <core/log.h>

#include <cjson/cJSON.h>

#include <storage/storage.h>

#define TAG "JsAppManifest"

#define JS_APP_MANIFEST_MAX_FILE_SIZE (512)

#define JS_APP_MANIFEST_FORMAT_VERSION (1)

#define JS_APP_MANIFEST_HEAP_SIZE_KIB_MIN     (1)
#define JS_APP_MANIFEST_HEAP_SIZE_KIB_MAX     (256)
#define JS_APP_MANIFEST_HEAP_SIZE_KIB_DEFAULT (32)

#define JS_APP_MANIFEST_FORMAT_VERSION_KEY "format_version"
#define JS_APP_MANIFEST_ID_KEY             "id"
#define JS_APP_MANIFEST_NAME_KEY           "name"
#define JS_APP_MANIFEST_VERSION_KEY        "version"
#define JS_APP_MANIFEST_DESCRIPTION_KEY    "description"
#define JS_APP_MANIFEST_AUTHOR_KEY         "author"
#define JS_APP_MANIFEST_HEAP_SIZE_KEY      "heap_size_kib"
#define JS_APP_MANIFEST_DEBUG_KEY          "debug"

struct JsAppManifest {
    cJSON* parsed_json;
    JsAppManifestInfo info;
};

static void js_app_manifest_reset(JsAppManifest* instance) {
    if(instance->parsed_json != NULL) {
        cJSON_Delete(instance->parsed_json);
        instance->parsed_json = NULL;
    }
}

static bool js_app_manifest_parse_heap_size(const cJSON* json, JsAppManifestInfo* info) {
    bool success = false;

    do {
        int32_t heap_size_kib;

        const cJSON* item = cJSON_GetObjectItem(json, JS_APP_MANIFEST_HEAP_SIZE_KEY);
        if(cJSON_IsNumber(item)) {
            heap_size_kib = cJSON_GetNumberValue(item);

            if((heap_size_kib < JS_APP_MANIFEST_HEAP_SIZE_KIB_MIN) ||
               (heap_size_kib > JS_APP_MANIFEST_HEAP_SIZE_KIB_MAX)) {
                break;
            }

        } else {
            heap_size_kib = JS_APP_MANIFEST_HEAP_SIZE_KIB_DEFAULT;
        }
        // Resulting value is in bytes for convenience
        info->heap_size = heap_size_kib * 1024;
        success = true;

    } while(false);

    return success;
}

static bool
    js_app_manifest_parse_data(JsAppManifest* instance, const char* data, uint32_t data_len) {
    bool success = false;

    JsAppManifestInfo* info = &instance->info;

    cJSON* json;

    do {
        json = cJSON_ParseWithLength(data, data_len);

        if(json == NULL) {
            break;
        }

        cJSON* item;

        item = cJSON_GetObjectItem(json, JS_APP_MANIFEST_FORMAT_VERSION_KEY);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        const uint32_t format_version = cJSON_GetNumberValue(item);
        if(format_version != JS_APP_MANIFEST_FORMAT_VERSION) {
            FURI_LOG_W(TAG, "Unknown format version: %lu", format_version);
            // Emit a warning, but try anyway
        }

        item = cJSON_GetObjectItem(json, JS_APP_MANIFEST_ID_KEY);
        if(!cJSON_IsString(item)) {
            break;
        }

        info->id = cJSON_GetStringValue(item);

        item = cJSON_GetObjectItem(json, JS_APP_MANIFEST_NAME_KEY);
        if(!cJSON_IsString(item)) {
            break;
        }

        info->name = cJSON_GetStringValue(item);

        item = cJSON_GetObjectItem(json, JS_APP_MANIFEST_VERSION_KEY);
        if(!cJSON_IsString(item)) {
            break;
        }

        info->version = cJSON_GetStringValue(item);

        item = cJSON_GetObjectItem(json, JS_APP_MANIFEST_DESCRIPTION_KEY);
        if(cJSON_IsString(item)) {
            info->description = cJSON_GetStringValue(item);
        } else {
            info->description = "";
        }

        item = cJSON_GetObjectItem(json, JS_APP_MANIFEST_AUTHOR_KEY);
        if(cJSON_IsString(item)) {
            info->author = cJSON_GetStringValue(item);
        } else {
            info->author = "";
        }

        if(!js_app_manifest_parse_heap_size(json, info)) {
            break;
        }

        item = cJSON_GetObjectItem(json, JS_APP_MANIFEST_DEBUG_KEY);
        if(cJSON_IsBool(item)) {
            info->is_debug = cJSON_IsTrue(item);
        } else {
            info->is_debug = false;
        }

        success = true;
    } while(false);

    if(success) {
        instance->parsed_json = json;
    } else {
        cJSON_Delete(json);
    }

    return success;
}

JsAppManifest* js_app_manifest_alloc(void) {
    JsAppManifest* instance = malloc(sizeof(JsAppManifest));
    return instance;
}

void js_app_manifest_free(JsAppManifest* instance) {
    furi_check(instance);

    js_app_manifest_reset(instance);
    free(instance);
}

bool js_app_manifest_load_from_file(JsAppManifest* instance, const char* file_path) {
    furi_check(instance);
    furi_check(file_path);

    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    char* file_buf = NULL;

    do {
        FileInfo file_info;

        if(storage_common_stat(storage, file_path, &file_info) != FSE_OK) {
            FURI_LOG_W(TAG, "File is not present");
            break;
        }

        if(file_info_is_dir(&file_info)) {
            FURI_LOG_E(TAG, "File path is a directory");
            break;
        }

        const uint32_t file_size = file_info.size;

        if(file_size == 0) {
            FURI_LOG_E(TAG, "File is empty");
            break;
        }

        if(file_size > JS_APP_MANIFEST_MAX_FILE_SIZE) {
            FURI_LOG_E(TAG, "File is too big");
            break;
        }

        if(!storage_file_open(file, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Failed to open file for reading");
            break;
        }

        file_buf = malloc(file_size);

        if(storage_file_read(file, file_buf, file_size) != file_size) {
            FURI_LOG_E(TAG, "Failed to read file");
            break;
        }

        if(!js_app_manifest_parse_data(instance, file_buf, file_size)) {
            FURI_LOG_E(TAG, "Failed to parse file");
            break;
        }

        success = true;
    } while(false);

    if(file_buf != NULL) {
        free(file_buf);
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success;
}

bool js_app_manifest_get_info(const JsAppManifest* instance, JsAppManifestInfo* info) {
    furi_check(instance);
    furi_check(info);

    bool success = false;

    if(instance->parsed_json) {
        *info = instance->info;
        success = true;
    }

    return success;
}
