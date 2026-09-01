#include "js_settings.h"
#include "js_runner_i.h"

#include <js_app/js_app_registry.h>
#include <js_app/js_app_settings.h>

#define TAG "JsSettings"

typedef struct {
    JsAppSettings* settings;
} JsSettings;

static void js_settings_free_cb(void* native_p, jerry_object_native_info_t* info_p);

static const jerry_object_native_info_t js_settings_native_info = {
    .free_cb = js_settings_free_cb,
};

static jerry_value_t js_settings_resolved_promise(jerry_value_t value) {
    jerry_value_t promise = jerry_promise();
    js_check_and_free(jerry_promise_resolve(promise, value));
    jerry_value_free(value);
    return promise;
}

static jerry_value_t js_settings_load(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);

    JsSettings* instance =
        jerry_object_get_native_ptr(call_info->this_value, &js_settings_native_info);
    if(!instance) return js_rejected_promise("Invalid this value");
    if(!instance->settings) return js_rejected_promise("Application has no settings");

    // Re-read from disk: Setup and the HTTP API can both have written since we
    // started, and the documented contract is that load() shows those changes.
    js_app_settings_reload(instance->settings);

    char* json = cJSON_PrintUnformatted(js_app_settings_get_config_json(instance->settings));
    if(!json) return js_rejected_promise("Failed to serialize application settings");

    jerry_value_t value = jerry_json_parse((const jerry_char_t*)json, strlen(json));
    free(json);
    if(jerry_value_is_exception(value)) return js_rejected_promise_from_exception(value);

    return js_settings_resolved_promise(value);
}

static jerry_value_t js_settings_save(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    JsSettings* instance =
        jerry_object_get_native_ptr(call_info->this_value, &js_settings_native_info);
    if(!instance) return js_rejected_promise("Invalid this value");
    if(!instance->settings) return js_rejected_promise("Application has no settings");
    if(args_count < 1) return js_rejected_promise("At least 1 argument required");
    if(!jerry_value_is_object(args[0])) {
        return js_rejected_promise("Settings values must be an object");
    }

    jerry_value_t serialized = jerry_json_stringify(args[0]);
    if(jerry_value_is_exception(serialized)) {
        return js_rejected_promise_from_exception(serialized);
    }

    char* json = js_string_to_c_string(serialized);
    jerry_value_free(serialized);
    if(!json) return js_rejected_promise("Failed to serialize settings values");

    cJSON* values = cJSON_Parse(json);
    free(json);

    // Merge onto current on-disk state, not the snapshot taken at startup —
    // otherwise saving one key rewrites the whole file and silently reverts
    // anything Setup or the HTTP API changed in the meantime.
    js_app_settings_reload(instance->settings);
    const bool saved = cJSON_IsObject(values) &&
                       js_app_settings_update_json(instance->settings, values);
    cJSON_Delete(values);
    if(!saved) return js_rejected_promise("Invalid settings values");

    return js_settings_resolved_promise(jerry_undefined());
}

static JsSettings* js_settings_alloc(void) {
    JsSettings* instance = malloc(sizeof(*instance));
    instance->settings = NULL;

    WITH_JS_RUNNER_APP(app, {
        JsApp* js_app = js_app_registry_get_app(js_runner_app_get_id(app));
        if(js_app) {
            JsAppInfo info;
            if(js_app_get_info(js_app, &info) && info.path.settings) {
                instance->settings = js_app_settings_alloc();
                if(!js_app_settings_load(
                       instance->settings, info.manifest.id, info.path.settings)) {
                    js_app_settings_free(instance->settings);
                    instance->settings = NULL;
                    FURI_LOG_E(TAG, "Failed to load application settings");
                }
            }
            js_app_free(js_app);
        }
    });

    return instance;
}

static void js_settings_free_cb(void* native_p, jerry_object_native_info_t* info_p) {
    UNUSED(info_p);
    JsSettings* instance = native_p;
    if(instance->settings) js_app_settings_free(instance->settings);
    free(instance);
}

void js_setup_settings(void) {
    jerry_value_t global_obj = jerry_current_realm();
    jerry_value_t settings = jerry_object();

    jerry_object_set_native_ptr(settings, &js_settings_native_info, js_settings_alloc());
    js_set_method(settings, "load", js_settings_load);
    js_set_method(settings, "save", js_settings_save);
    js_set_property(global_obj, "Settings", settings);
    jerry_value_free(global_obj);
}
