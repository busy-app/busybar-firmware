#include "js_local_storage.h"
#include <storage/storage.h>

#include <m-dict.h>
#include <cjson/cJSON.h>

#define TAG "JsLocalStorage"

#define DATABASE_DIR EXT_PATH("apps_data/jsrunner")

#define JSON_VERSION_KEY "format_version"
#define JSON_DATA_KEY    "data"

#define JSON_VERSION 1

DICT_DEF2(LocalStorageDict, FuriString*, FURI_STRING_OPLIST, FuriString*, FURI_STRING_OPLIST);

typedef struct LocalStorage {
    FuriString* filename;

    bool loaded;
    LocalStorageDict_t dict;
} LocalStorage;

static void local_storage_free_cb(void* native_p, jerry_object_native_info_t* info_p);

static const jerry_object_native_info_t local_storage_native_info = {
    .free_cb = local_storage_free_cb};

static char* serialize(const LocalStorage* instance) {
    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, JSON_VERSION_KEY, JSON_VERSION);
    cJSON* data = cJSON_AddObjectToObject(root, JSON_DATA_KEY);

    LocalStorageDict_it_t iter;
    for(LocalStorageDict_it(iter, instance->dict); !LocalStorageDict_end_p(iter);
        LocalStorageDict_next(iter)) {
        const LocalStorageDict_itref_t* ref = LocalStorageDict_cref(iter);
        cJSON_AddStringToObject(
            data, furi_string_get_cstr(ref->key), furi_string_get_cstr(ref->value));
    }

    char* contents = cJSON_Print(root);

    cJSON_Delete(root);
    return contents;
}

static bool save(const LocalStorage* instance) {
    JS_TRACE("save %s", furi_string_get_cstr(instance->filename));
    bool result = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    do {
        if(!storage_simply_mkpath(storage, DATABASE_DIR)) {
            FURI_LOG_E(TAG, "Failed to make path: \"%s\".", DATABASE_DIR);
            break;
        }

        File* file = storage_file_alloc(storage);
        do {
            if(!storage_file_open(
                   file,
                   furi_string_get_cstr(instance->filename),
                   FSAM_WRITE,
                   FSOM_CREATE_ALWAYS)) {
                FURI_LOG_E(
                    TAG,
                    "Failed to open file for writing: \"%s\".",
                    furi_string_get_cstr(instance->filename));
                break;
            }

            char* contents = serialize(instance);
            size_t contents_len = strlen(contents);
            if(!storage_file_write(file, contents, contents_len)) {
                FURI_LOG_E(
                    TAG,
                    "Failed to write file: \"%s\".",
                    furi_string_get_cstr(instance->filename));
                result = false;
            } else {
                result = true;
            }
            free(contents);
        } while(false);
        storage_file_free(file);
    } while(false);
    furi_record_close(RECORD_STORAGE);
    return result;
}

static void load(LocalStorage* instance) {
    if(instance->loaded) {
        return;
    }

    JS_TRACE("load %s", furi_string_get_cstr(instance->filename));

    Storage* storage = furi_record_open(RECORD_STORAGE);

    File* file = storage_file_alloc(storage);
    do {
        if(!storage_file_open(
               file, furi_string_get_cstr(instance->filename), FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_W(
                TAG, "Failed to open file: \"%s\".", furi_string_get_cstr(instance->filename));
            break;
        }

        size_t file_size = storage_file_size(file);
        if(file_size == 0) {
            break;
        }

        char* file_buffer = malloc(file_size + 1);
        do {
            if(storage_file_read(file, file_buffer, file_size) != file_size) {
                FURI_LOG_W(
                    TAG, "Failed to read file: \"%s\".", furi_string_get_cstr(instance->filename));
                break;
            }
            file_buffer[file_size] = 0;
            cJSON* root = cJSON_Parse(file_buffer);
            if(!root) {
                break;
            }
            do {
                if(!cJSON_IsObject(root)) {
                    break;
                }
                cJSON* format_version = cJSON_GetObjectItem(root, JSON_VERSION_KEY);
                if(!cJSON_IsNumber(format_version) ||
                   (int)cJSON_GetNumberValue(format_version) != JSON_VERSION) {
                    FURI_LOG_W(
                        TAG, "Wrong version (\"%s\").", furi_string_get_cstr(instance->filename));
                    break;
                }
                cJSON* data = cJSON_GetObjectItem(root, JSON_DATA_KEY);
                if(!cJSON_IsObject(data)) {
                    break;
                }

                cJSON* child = data->child;
                while(child) {
                    const char* key = child->string;
                    const char* value = cJSON_GetStringValue(child);
                    if(key && value) {
                        JS_TRACE("%s: %s", key, value);
                        FuriString* key_string = furi_string_alloc_set(key);
                        FuriString* value_string = furi_string_alloc_set(value);
                        LocalStorageDict_set_at(instance->dict, key_string, value_string);
                        furi_string_free(key_string);
                        furi_string_free(value_string);
                    }
                    child = child->next;
                }
                JS_TRACE("loaded");
                instance->loaded = true;
            } while(false);
            cJSON_free(root);
        } while(false);
        free(file_buffer);
    } while(false);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    if(!instance->loaded) {
        save(instance);
    }
}

static jerry_value_t method_get_length(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);

    LocalStorage* instance =
        jerry_object_get_native_ptr(call_info->this_value, &local_storage_native_info);
    JS_CHECK_INSTANCE();
    load(instance);

    return jerry_number(LocalStorageDict_size(instance->dict));
}

static jerry_value_t method_key(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    JS_CHECK_ARGS_COUNT(1);

    LocalStorage* instance =
        jerry_object_get_native_ptr(call_info->this_value, &local_storage_native_info);
    JS_CHECK_INSTANCE();
    load(instance);

    jerry_value_t arg = JS_ARG(0);

    int idx = 0;
    bool ok = js_value_to_integer(arg, &idx);

    if(!ok || idx < 0 || (size_t)idx >= LocalStorageDict_size(instance->dict)) {
        return jerry_null();
    }

    LocalStorageDict_it_t iter;
    for(LocalStorageDict_it(iter, instance->dict); !LocalStorageDict_end_p(iter);
        LocalStorageDict_next(iter)) {
        if(idx == 0) {
            const LocalStorageDict_itref_t* ref = LocalStorageDict_cref(iter);
            return js_utf8_string(ref->key);
        }
        idx -= 1;
    }

    furi_check(false);

    return jerry_undefined();
}

static jerry_value_t method_get_item(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    JS_CHECK_ARGS_COUNT(1);

    LocalStorage* instance =
        jerry_object_get_native_ptr(call_info->this_value, &local_storage_native_info);
    JS_CHECK_INSTANCE();
    load(instance);

    jerry_value_t arg = JS_ARG(0);
    jerry_value_t arg_str = jerry_value_to_string(arg);
    if(jerry_value_is_exception(arg_str)) {
        return arg_str;
    }
    FuriString* key = js_string_to_furi_string(arg_str);
    jerry_value_free(arg_str);
    if(!key) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Argument is not a string");
    }

    FuriString* const* value = LocalStorageDict_cget(instance->dict, key);

    jerry_value_t result;
    if(!value) {
        result = jerry_null();
    } else {
        result = js_utf8_string(*value);
    }

    furi_string_free(key);

    return result;
}

static jerry_value_t method_set_item(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    JS_CHECK_ARGS_COUNT(2);

    LocalStorage* instance =
        jerry_object_get_native_ptr(call_info->this_value, &local_storage_native_info);
    JS_CHECK_INSTANCE();
    load(instance);

    jerry_value_t key_str = jerry_value_to_string(JS_ARG(0));
    if(jerry_value_is_exception(key_str)) {
        return key_str;
    }
    FuriString* key = js_string_to_furi_string(key_str);
    jerry_value_free(key_str);
    if(!key) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Argument is not a string");
    }

    jerry_value_t value_str = jerry_value_to_string(JS_ARG(1));
    if(jerry_value_is_exception(value_str)) {
        return value_str;
    }
    FuriString* value = js_string_to_furi_string(value_str);
    jerry_value_free(value_str);
    if(!value) {
        furi_string_free(key);
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Argument is not a string");
    }

    LocalStorageDict_set_at(instance->dict, key, value);

    furi_string_free(key);
    furi_string_free(value);

    if(save(instance)) {
        return jerry_undefined();
    } else {
        return jerry_throw_sz(JERRY_ERROR_COMMON, "Local storage database cannot be saved");
    }
}

static jerry_value_t method_remove_item(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    JS_CHECK_ARGS_COUNT(1);

    LocalStorage* instance =
        jerry_object_get_native_ptr(call_info->this_value, &local_storage_native_info);
    JS_CHECK_INSTANCE();
    load(instance);

    jerry_value_t key_str = jerry_value_to_string(JS_ARG(0));
    if(jerry_value_is_exception(key_str)) {
        return key_str;
    }
    FuriString* key = js_string_to_furi_string(key_str);
    jerry_value_free(key_str);
    if(!key) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Argument is not a string");
    }

    LocalStorageDict_erase(instance->dict, key);

    furi_string_free(key);

    if(save(instance)) {
        return jerry_undefined();
    } else {
        return jerry_throw_sz(JERRY_ERROR_COMMON, "Local storage database cannot be saved");
    }
}

static jerry_value_t method_clear(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);

    LocalStorage* instance =
        jerry_object_get_native_ptr(call_info->this_value, &local_storage_native_info);
    JS_CHECK_INSTANCE();

    LocalStorageDict_reset(instance->dict);

    if(save(instance)) {
        return jerry_undefined();
    } else {
        return jerry_throw_sz(JERRY_ERROR_COMMON, "Local storage database cannot be saved");
    }
}

static LocalStorage* local_storage_alloc(void) {
    LocalStorage* instance = malloc(sizeof(LocalStorage));
    WITH_JS_RUNNER_APP(app, {
        instance->filename = furi_string_alloc_printf(
            "%s/%s.localstorage.json", DATABASE_DIR, js_runner_app_get_id(app));
    });
    instance->loaded = false;
    LocalStorageDict_init(instance->dict);
    return instance;
}

static void local_storage_free_cb(void* native_p, jerry_object_native_info_t* info_p) {
    UNUSED(info_p);
    LocalStorage* instance = native_p;
    furi_string_free(instance->filename);
    LocalStorageDict_clear(instance->dict);
    free(instance);
}

void js_setup_local_storage(void) {
    jerry_value_t global_obj = jerry_current_realm();
    jerry_value_t local_storage = jerry_object();

    LocalStorage* instance = local_storage_alloc();

    jerry_object_set_native_ptr(local_storage, &local_storage_native_info, instance);

    js_set_property_getset(local_storage, "length", method_get_length, NULL);
    js_set_method(local_storage, "key", method_key);
    js_set_method(local_storage, "getItem", method_get_item);
    js_set_method(local_storage, "setItem", method_set_item);
    js_set_method(local_storage, "removeItem", method_remove_item);
    js_set_method(local_storage, "clear", method_clear);

    js_set_property(global_obj, "localStorage", local_storage);
    jerry_value_free(global_obj);
}
