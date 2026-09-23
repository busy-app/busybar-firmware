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

bool js_app_launcher_start(const char* app_id, JsAppLauncherStartMode start_mode) {
    furi_check(app_id);
    furi_check(start_mode < JsAppLauncherStartModeMax);

    char args[JS_APP_ID_LEN_MAX + sizeof(JS_APP_LAUNCHER_ARG_RESUME)];

    if(strlcpy(args, app_id, JS_APP_ID_LEN_MAX) > JS_APP_ID_LEN_MAX) {
        return false;
    }

    if(start_mode == JsAppLauncherStartModeResume) {
        strlcat(args, JS_APP_LAUNCHER_ARG_RESUME, sizeof(args));
    }

    Desktop* desktop = furi_record_open(RECORD_DESKTOP);
    const bool success = desktop_replace_current_app(desktop, JS_APP_LAUNCHER_APP_ID, args);
    furi_record_close(RECORD_DESKTOP);

    return success;
}

bool js_app_launcher_stop(JsAppLauncherStopMode stop_mode) {
    furi_check(stop_mode < JsAppLauncherStopModeMax);

    JsAppLauncher* instance = furi_record_open_ex(RECORD_JS_APP_LAUNCHER, RECORD_TIMEOUT_TICKS);
    if(instance == NULL) {
        return false;
    }

    const JsAppLauncherApiMessage message = {
        .type = JsAppLauncherApiMessageTypeStop,
        .stop = {.mode = stop_mode},
    };

    const bool success = js_app_launcher_send_api_message(instance, &message);

    furi_record_close(RECORD_JS_APP_LAUNCHER);
    return success;
}
