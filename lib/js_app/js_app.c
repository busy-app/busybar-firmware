#include "js_app.h"

#include <core/check.h>
#include <core/log.h>

#include <toolbox/path.h>

#define TAG "JsApp"

#define APPMETA_PREFIX     "appmeta"
#define APPMETA_PATH(path) APPMETA_PREFIX "/" path

#define APP_MANIFEST_PATH APPMETA_PATH("manifest.json")

struct JsApp {
    JsAppManifest* manifest;
};

JsApp* js_app_alloc(void) {
    JsApp* instance = malloc(sizeof(JsApp));
    instance->manifest = js_app_manifest_alloc();
    return instance;
}

void js_app_free(JsApp* instance) {
    furi_check(instance);

    js_app_manifest_free(instance->manifest);
    free(instance);
}

bool js_app_parse_from_dir(JsApp* instance, const char* dir_path) {
    furi_check(instance);
    furi_check(dir_path);

    FuriString* tmp_path = furi_string_alloc();
    path_concat(dir_path, APP_MANIFEST_PATH, tmp_path);

    const bool success =
        js_app_manifest_parse_from_file(instance->manifest, furi_string_get_cstr(tmp_path));

    furi_string_free(tmp_path);
    return success;
}

bool js_app_get_info(const JsApp* instance, JsAppInfo* info) {
    furi_check(instance);
    furi_check(info);

    bool success = false;

    if(js_app_manifest_get_info(instance->manifest, &info->manifest)) {
        // TODO: Additional fields
        success = true;
    }

    return success;
}
