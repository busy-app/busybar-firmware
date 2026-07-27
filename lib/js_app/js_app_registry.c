#include "js_app_registry.h"

#include <core/log.h>
#include <core/check.h>

#include "js_app.h"

#define TAG "JsAppRegistry"

void js_app_registry_list_apps(JsAppRegistryListCallback callback, void* context) {
    furi_check(callback);

    const char* const test_names[] = {
        "Weather",
        "Social Stats",
        "My Automation",
    };

    JsAppInfo info = {0};

    for(uint32_t i = 0; i < COUNT_OF(test_names); ++i) {
        info.name = test_names[i];
        callback(&info, context);
    }
}
