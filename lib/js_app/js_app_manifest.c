#include "js_app_manifest.h"

#include <core/check.h>
#include <core/log.h>

#include <cjson/cJSON.h>

#include <storage/storage.h>

#define TAG "JsAppManifest"

struct JsAppManifest {
    // TODO: Implementation
    uint32_t test_idx;
};

JsAppManifest* js_app_manifest_alloc(void) {
    JsAppManifest* instance = malloc(sizeof(JsAppManifest));
    instance->test_idx = UINT32_MAX;
    return instance;
}

void js_app_manifest_free(JsAppManifest* instance) {
    furi_check(instance);
    free(instance);
}

bool js_app_manifest_parse_from_file(JsAppManifest* instance, const char* file_path) {
    furi_check(instance);
    furi_check(file_path);

    // TODO: Implementation
    FURI_LOG_I(TAG, "Parsing manifest: %s", file_path);
    ++instance->test_idx;

    return true;
}

bool js_app_manifest_get_info(const JsAppManifest* instance, JsAppManifestInfo* info) {
    furi_check(instance);
    furi_check(info);

    static const char* const test_names[] = {
        "Weather",
        "Social Stats",
        "My Automation",
    };

    // TODO: Implementation
    const uint32_t test_idx = instance->test_idx % COUNT_OF(test_names);

    info->name = test_names[test_idx];
    info->is_debug = false;

    return true;
}
