#include "js_app_registry.h"

#include <core/log.h>
#include <core/check.h>

#include <storage_utils/dir_walk.h>
#include <storage/storage.h>
#include <toolbox/path.h>

#define TAG "JsAppRegistry"

// TODO: Share with assets HTTP API
#define JS_APPS_PATH EXT_PATH("user_assets")

static void js_app_registry_list_apps_directory(
    DirWalk* dir_walk,
    JsAppRegistryListCallback callback,
    void* context) {
    FuriString* path = furi_string_alloc();
    JsApp* js_app = js_app_alloc();

    JsAppInfo app_info;

    while(dir_walk_read(dir_walk, path, NULL) == DirWalkOK) {
        if(js_app_load_from_directory(js_app, furi_string_get_cstr(path))) {
            if(js_app_get_info(js_app, &app_info)) {
                callback(&app_info, context);
            }
        }
    }

    js_app_free(js_app);
    furi_string_free(path);
}

void js_app_registry_list_apps(JsAppRegistryListCallback callback, void* context) {
    furi_check(callback);

    Storage* storage = furi_record_open(RECORD_STORAGE);

    DirWalk* dir_walk = dir_walk_alloc(storage);
    dir_walk_set_recursive(dir_walk, false);
    dir_walk_set_filter_cb(dir_walk, dir_walk_is_dir_callback, NULL);

    if(dir_walk_open(dir_walk, JS_APPS_PATH)) {
        js_app_registry_list_apps_directory(dir_walk, callback, context);
    }

    dir_walk_close(dir_walk);
    dir_walk_free(dir_walk);

    furi_record_close(RECORD_STORAGE);
}

JsApp* js_app_registry_get_app(const char* app_id) {
    furi_check(app_id);

    FuriString* app_path = js_app_registry_get_app_path(app_id);

    if(!app_path) {
        return NULL;
    }

    JsApp* js_app = js_app_alloc();

    if(!js_app_load_from_directory(js_app, furi_string_get_cstr(app_path))) {
        js_app_free(js_app);
        js_app = NULL;
    }

    furi_string_free(app_path);
    return js_app;
}

bool js_app_registry_validate_app_id(const char* app_id) {
    if(!*app_id) {
        return false;
    }
    size_t i = 0;
    while(app_id[i]) {
        char c = app_id[i];
        bool is_word = isalnum((int)c) || c == '_' || c == '-';
        bool is_dot = c == '.';
        if(i == 0) {
            if(!is_word) {
                return false;
            }
        } else {
            if(!is_word && !is_dot) {
                return false;
            }
        }
        ++i;
    }
    return true;
}

FuriString* js_app_registry_get_app_path(const char* app_id) {
    if(!js_app_registry_validate_app_id(app_id)) {
        return NULL;
    }
    FuriString* app_path = furi_string_alloc();
    path_concat(JS_APPS_PATH, app_id, app_path);
    return app_path;
}
