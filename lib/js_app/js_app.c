#include "js_app.h"

#include <core/check.h>
#include <core/log.h>

#define TAG "JsApp"

struct JsApp {
    // TODO: Implementation
    uint32_t test_idx;
};

JsApp* js_app_alloc(void) {
    JsApp* instance = malloc(sizeof(JsApp));
    instance->test_idx = UINT32_MAX;
    return instance;
}

void js_app_free(JsApp* instance) {
    furi_check(instance);
    free(instance);
}

bool js_app_parse_from_dir(JsApp* instance, const char* dir_path) {
    furi_check(instance);
    furi_check(dir_path);

    // TODO: Implementation
    FURI_LOG_I(TAG, "Parsing directory: %s", dir_path);
    ++instance->test_idx;

    return true;
}

bool js_app_get_info(const JsApp* instance, JsAppInfo* info) {
    furi_check(instance);
    furi_check(info);

    // TODO: Implementation
    static const char* const test_names[] = {
        "Weather",
        "Social Stats",
        "My Automation",
    };

    const uint32_t test_idx = instance->test_idx % COUNT_OF(test_names);

    info->name = test_names[test_idx];
    info->is_debug = false;

    return true;
}
