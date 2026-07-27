#include "js_app_registry.h"

#include <core/log.h>
#include <core/check.h>

#include <storage_utils/dir_walk.h>

#include "js_app.h"
#include "js_app_common_i.h"

#define TAG "JsAppRegistry"

static bool js_app_registry_is_dir_callback(const char* path, FileInfo* file_info, void* context) {
    UNUSED(path);
    UNUSED(context);

    furi_assert(file_info);
    return file_info->flags & FSF_DIRECTORY;
}

static void js_app_registry_list_apps_dir(
    DirWalk* dir_walk,
    JsAppRegistryListCallback callback,
    void* context) {
    FuriString* path = furi_string_alloc();
    JsApp* js_app = js_app_alloc();

    JsAppInfo app_info;

    while(dir_walk_read(dir_walk, path, NULL) == DirWalkOK) {
        if(js_app_parse_from_dir(js_app, furi_string_get_cstr(path))) {
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
    dir_walk_set_filter_cb(dir_walk, js_app_registry_is_dir_callback, NULL);

    if(dir_walk_open(dir_walk, JS_APPS_PATH)) {
        js_app_registry_list_apps_dir(dir_walk, callback, context);
    }

    dir_walk_close(dir_walk);
    dir_walk_free(dir_walk);

    furi_record_close(RECORD_STORAGE);
}
