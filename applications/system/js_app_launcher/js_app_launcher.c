#include <stdint.h>

#include <furi.h>

#define TAG "JsAppLauncher"

typedef struct {
    FuriEventLoop* event_loop;
} JsAppLauncher;

static JsAppLauncher* js_app_launcher_alloc(const char* app_id) {
    JsAppLauncher* instance = malloc(sizeof(JsAppLauncher));

    instance->event_loop = furi_event_loop_alloc();

    // TODO: Implementation
    FURI_LOG_I(TAG, "Running JS application with id \"%s\"", app_id);

    return instance;
}

static void js_app_launcher_free(JsAppLauncher* instance) {
    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t js_app_launcher_app(void* arg) {
    UNUSED(arg);

    JsAppLauncher* instance = js_app_launcher_alloc(arg);
    furi_event_loop_run(instance->event_loop);
    js_app_launcher_free(instance);

    return 0;
}
