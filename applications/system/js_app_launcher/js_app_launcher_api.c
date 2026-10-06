#include "js_app_launcher_i.h"

#include <desktop/desktop.h>

#include <js_app/js_app_common.h>

#define API_QUEUE_TIMEOUT_TICKS (50)
#define RECORD_TIMEOUT_TICKS    (50)

static bool js_app_launcher_send_api_message(
    JsAppLauncher* instance,
    const JsAppLauncherApiMessage* message) {
    bool success;

    const FuriStatus status =
        furi_message_queue_put(instance->api_queue, message, API_QUEUE_TIMEOUT_TICKS);

    if(status == FuriStatusOk) {
        if(message->lock != NULL) {
            api_lock_wait_unlock(message->lock);
        }
        success = true;

    } else {
        furi_check(status == FuriStatusErrorTimeout);
        success = false;
    }

    if(message->lock != NULL) {
        api_lock_free(message->lock);
    }

    return success;
}

void js_app_launcher_api_unlock_message(
    JsAppLauncherApiMessage* message,
    JsAppLauncherStatus status) {
    if(message->lock != NULL) {
        furi_check(message->status);
        *message->status = status;
        api_lock_unlock(message->lock);
    }
}

void js_app_launcher_api_abort_pending_messages(JsAppLauncher* instance) {
    FURI_CRITICAL_ENTER();

    JsAppLauncherApiMessage message;

    while(furi_message_queue_get(instance->api_queue, &message, 0) == FuriStatusOk) {
        js_app_launcher_api_unlock_message(&message, JsAppLauncherStatusAborted);
    }

    memset(&message, 0, sizeof(message));

    while(furi_message_queue_put(instance->api_queue, &message, 0) == FuriStatusOk) {
        // HACK: Fill up the queue so that it cannot receive any more messages
    }

    FURI_CRITICAL_EXIT();
}

JsAppLauncherStatus js_app_launcher_start(const char* app_id, JsAppLauncherStartMode start_mode) {
    furi_check(app_id);
    furi_check(start_mode < JsAppLauncherStartModeMax);

    char args[JS_APP_ID_LEN_MAX + sizeof(JS_APP_LAUNCHER_ARG_RESUME)];

    if(strlcpy(args, app_id, JS_APP_ID_LEN_MAX + 1) > JS_APP_ID_LEN_MAX) {
        return JsAppLauncherStatusInvalidAppId;
    }

    if(start_mode == JsAppLauncherStartModeResume) {
        strlcat(args, JS_APP_LAUNCHER_ARG_RESUME, sizeof(args));
    }

    JsAppLauncherStatus status = JsAppLauncherStatusOk;

    Loader* loader = furi_record_open(RECORD_LOADER);
    do {
        size_t current_priority = loader_get_priority(loader);
        if(current_priority > LOADER_DEFAULT_APP_PRIORITY) {
            status = JsAppLauncherStatusLowPriority;
            break;
        }

        Desktop* desktop = furi_record_open(RECORD_DESKTOP);
        if(!desktop_replace_current_app(desktop, JS_APP_LAUNCHER_APP_ID, args)) {
            status = JsAppLauncherStatusTimeout;
        }
        furi_record_close(RECORD_DESKTOP);
    } while(false);
    furi_record_close(RECORD_LOADER);

    return status;
}

JsAppLauncherStatus js_app_launcher_stop(JsAppLauncherStopMode stop_mode) {
    furi_check(stop_mode < JsAppLauncherStopModeMax);

    JsAppLauncher* instance = furi_record_open_ex(RECORD_JS_APP_LAUNCHER, RECORD_TIMEOUT_TICKS);
    if(instance == NULL) {
        return JsAppLauncherStatusNotRunning;
    }

    JsAppLauncherStatus status = JsAppLauncherStatusOk;

    const JsAppLauncherApiMessage message = {
        .type = JsAppLauncherApiMessageTypeStop,
        .status = &status,
        .lock = api_lock_alloc_locked(),
        .stop = {.mode = stop_mode},
    };

    if(!js_app_launcher_send_api_message(instance, &message)) {
        status = JsAppLauncherStatusTimeout;
    }

    furi_record_close(RECORD_JS_APP_LAUNCHER);
    return status;
}

JsAppLauncherStatus js_app_launcher_get_running_app_id(FuriString* app_id) {
    furi_check(app_id);

    JsAppLauncher* instance = furi_record_open_ex(RECORD_JS_APP_LAUNCHER, RECORD_TIMEOUT_TICKS);
    if(instance == NULL) {
        return JsAppLauncherStatusNotRunning;
    }

    JsAppLauncherStatus status = JsAppLauncherStatusOk;

    const JsAppLauncherApiMessage message = {
        .type = JsAppLauncherApiMessageTypeGetAppId,
        .status = &status,
        .lock = api_lock_alloc_locked(),
        .get_app_id = {.app_id = app_id},
    };

    if(!js_app_launcher_send_api_message(instance, &message)) {
        status = JsAppLauncherStatusTimeout;
    }

    furi_record_close(RECORD_JS_APP_LAUNCHER);
    return status;
}
