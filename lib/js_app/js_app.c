#include "js_app.h"

#include <core/check.h>
#include <core/log.h>

#include <storage/storage.h>

#include <toolbox/path.h>

#define TAG "JsApp"

#define APPMETA_PREFIX     "appmeta"
#define APPMETA_PATH(path) APPMETA_PREFIX "/" path

#define SCRIPTS_PREFIX     "scripts"
#define SCRIPTS_PATH(path) SCRIPTS_PREFIX "/" path

#define APP_MANIFEST_PATH   APPMETA_PATH("manifest.json")
#define APP_FRONT_ICON_PATH APPMETA_PATH("icon_front_8x8.png")
#define APP_BACK_ICON_PATH  APPMETA_PATH("icon_back_11x11.png")

#define APP_ENTRY_PATH SCRIPTS_PATH("main.js")

#define FRONT_DEFAULT_ICON_PATH SHARED_IMG_PATH("unknown_app_front_8x8.image")
#define BACK_DEFAULT_ICON_PATH  SHARED_IMG_PATH("unknown_app_back_11x11.image")

struct JsApp {
    JsAppManifest* manifest;
    FuriString* front_icon_path;
    FuriString* back_icon_path;
    FuriString* entry_script_path;
};

static bool js_app_process_manifest(JsApp* instance, const char* dir_path) {
    FuriString* tmp_path = furi_string_alloc();
    path_concat(dir_path, APP_MANIFEST_PATH, tmp_path);

    const bool success =
        js_app_manifest_load_from_file(instance->manifest, furi_string_get_cstr(tmp_path));

    furi_string_free(tmp_path);
    return success;
}

static bool js_apps_process_root_directory(JsApp* instance, const char* dir_path) {
    bool is_root_valid = false;
    FuriString* root_dir = furi_string_alloc();

    do {
        JsAppManifestInfo manifest_info;
        if(!js_app_manifest_get_info(instance->manifest, &manifest_info)) {
            break;
        }

        path_extract_basename(dir_path, root_dir);
        if(!furi_string_equal(root_dir, manifest_info.id)) {
            break;
        }

        is_root_valid = true;
    } while(false);

    furi_string_free(root_dir);
    return is_root_valid;
}

static bool js_app_process_scripts_directory(JsApp* instance, const char* dir_path) {
    bool success = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);

    path_concat(dir_path, APP_ENTRY_PATH, instance->entry_script_path);

    if(storage_file_exists(storage, furi_string_get_cstr(instance->entry_script_path))) {
        success = true;
    }

    furi_record_close(RECORD_STORAGE);
    return success;
}

static void js_app_process_icons(JsApp* instance, const char* dir_path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    path_concat(dir_path, APP_FRONT_ICON_PATH, instance->front_icon_path);

    if(!storage_file_exists(storage, furi_string_get_cstr(instance->front_icon_path))) {
        furi_string_set(instance->front_icon_path, FRONT_DEFAULT_ICON_PATH);
    }

    path_concat(dir_path, APP_BACK_ICON_PATH, instance->back_icon_path);

    if(!storage_file_exists(storage, furi_string_get_cstr(instance->back_icon_path))) {
        furi_string_set(instance->back_icon_path, BACK_DEFAULT_ICON_PATH);
    }

    furi_record_close(RECORD_STORAGE);
}

JsApp* js_app_alloc(void) {
    JsApp* instance = malloc(sizeof(JsApp));

    instance->manifest = js_app_manifest_alloc();
    instance->front_icon_path = furi_string_alloc();
    instance->back_icon_path = furi_string_alloc();
    instance->entry_script_path = furi_string_alloc();

    return instance;
}

void js_app_free(JsApp* instance) {
    furi_check(instance);

    js_app_manifest_free(instance->manifest);
    furi_string_free(instance->front_icon_path);
    furi_string_free(instance->back_icon_path);
    furi_string_free(instance->entry_script_path);

    free(instance);
}

bool js_app_load_from_directory(JsApp* instance, const char* dir_path) {
    furi_check(instance);
    furi_check(dir_path);

    bool success = false;

    do {
        if(!js_app_process_manifest(instance, dir_path)) {
            break;
        }

        if(!js_apps_process_root_directory(instance, dir_path)) {
            break;
        }

        if(!js_app_process_scripts_directory(instance, dir_path)) {
            break;
        }

        js_app_process_icons(instance, dir_path);

        success = true;
    } while(false);

    return success;
}

bool js_app_get_info(const JsApp* instance, JsAppInfo* info) {
    furi_check(instance);
    furi_check(info);

    bool success = false;

    if(js_app_manifest_get_info(instance->manifest, &info->manifest)) {
        JsAppPathInfo* path = &info->path;
        path->entry = furi_string_get_cstr(instance->entry_script_path);
        path->icon.front = furi_string_get_cstr(instance->front_icon_path);
        path->icon.back = furi_string_get_cstr(instance->back_icon_path);

        success = true;
    }

    return success;
}
