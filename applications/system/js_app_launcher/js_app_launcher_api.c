#include "js_app_launcher_i.h"

#include <desktop/desktop.h>

#include <js_app/js_app_common.h>

#define API_QUEUE_TIMEOUT_TICKS (1000)
#define RECORD_TIMEOUT_TICKS    (50)

static bool js_app_launcher_send_api_message(
    JsAppLauncher* instance,
    const JsAppLauncherApiMessage* message) {
    bool success;

    const FuriStatus status =
        furi_message_queue_put(instance->api_queue, message, API_QUEUE_TIMEOUT_TICKS);

    if(status == FuriStatusOk) {
        success = true;
    } else {
        furi_check(status == FuriStatusErrorTimeout);
        success = false;
    }

    return success;
}

bool js_app_launcher_start(const char* app_id, JsAppLauncherStartMode mode) {
    furi_check(app_id);
    furi_check(mode < JsAppLauncherStartModeMax);

    char args[JS_APP_ID_LEN_MAX + sizeof(JS_APP_LAUNCHER_ARG_SKIP_MENU)];
    strlcpy(args, app_id, sizeof(args) - strlen(JS_APP_LAUNCHER_ARG_SKIP_MENU));

    if(mode == JsAppLauncherStartModeResume) {
        strlcat(args, JS_APP_LAUNCHER_ARG_SKIP_MENU, sizeof(args));
    }

    Desktop* desktop = furi_record_open(RECORD_DESKTOP);
    const bool success = desktop_replace_current_app(desktop, JS_APP_LAUNCHER_APP_ID, args);
    furi_record_close(RECORD_DESKTOP);

    return success;
}

bool js_app_launcher_stop(JsAppLauncherStopMode mode) {
    furi_check(mode < JsAppLauncherStopModeMax);

    JsAppLauncher* instance = furi_record_open_ex(RECORD_JS_APP_LAUNCHER, RECORD_TIMEOUT_TICKS);
    if(instance == NULL) {
        return false;
    }

    const JsAppLauncherApiMessage message = {
        .type = JsAppLauncherApiMessageTypeStop,
        .stop = {.mode = mode},
    };

    const bool success = js_app_launcher_send_api_message(instance, &message);

    furi_record_close(RECORD_JS_APP_LAUNCHER);
    return success;
}
