#include "../js_app_launcher_i.h"
#include "js_app_launcher_scenes.h"

#define JS_THREAD_STACK_SIZE (3 * 1024)

typedef struct {
    FuriThread* js_thread;
    JsAppInfo js_info;
    JsRunner* js_runner;
    JsRunnerError js_error;
} JsAppLauncherSceneRun;

static void js_app_launcher_scene_run_console_out_callback(
    JsRunnerConsoleSeverity severity,
    const char* buf,
    size_t size,
    JsRunnerConsoleSeparator separator,
    void* context) {
    UNUSED(separator);
    UNUSED(context);

    // TODO: Better logging ?
    if(severity == JsRunnerConsoleSeverityLog) {
        FURI_LOG_D(TAG, "%.*s", size, buf);
    } else if(severity == JsRunnerConsoleSeverityInfo) {
        FURI_LOG_I(TAG, "%.*s", size, buf);
    } else if(severity == JsRunnerConsoleSeverityError) {
        FURI_LOG_E(TAG, "%.*s", size, buf);
    }
}

static int32_t js_app_laucher_scene_run_thread_callback(void* arg) {
    furi_assert(arg);
    JsAppLauncherSceneRun* data = arg;

    const JsAppInfo* info = &data->js_info;

    const JsRunnerError error = js_runner_run(
        data->js_runner,
        info->manifest.id,
        info->path.entry,
        info->manifest.heap_size,
        js_app_launcher_scene_run_console_out_callback,
        NULL);

    if(error != JsRunnerErrorNone) {
        FURI_LOG_E(TAG, "JsRunner error: %d", error);
    }

    data->js_error = error;

    return 0;
}

static void js_app_laucher_scene_run_thread_state_callback(
    FuriThread* thread,
    FuriThreadState state,
    void* context) {
    UNUSED(thread);
    furi_assert(context);

    JsAppLauncher* instance = context;

    if(state == FuriThreadStateStopped) {
        js_app_launcher_send_custom_event(instance, JsAppLauncherCustomEventScriptFinished);
    }
}

static void js_app_launcher_scene_run_on_enter(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    JsAppLauncherSceneRun* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdRun);

    data->js_runner = furi_record_open(RECORD_JS_RUNNER);

    if(js_app_get_info(instance->js_app, &data->js_info)) {
        FuriThread* js_thread = furi_thread_alloc_ex(
            data->js_info.manifest.name,
            JS_THREAD_STACK_SIZE,
            js_app_laucher_scene_run_thread_callback,
            data);

        furi_thread_set_state_callback(js_thread, js_app_laucher_scene_run_thread_state_callback);
        furi_thread_set_state_context(js_thread, instance);

        data->js_thread = js_thread;
        furi_thread_start(js_thread);

    } else {
        scene_manager_next_scene(instance->scene_manager, JsAppLauncherSceneIdError);
    }
}

static void js_app_launcher_scene_run_on_exit(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    JsAppLauncherSceneRun* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdRun);

    FuriThread* js_thread = data->js_thread;

    if(js_thread != NULL) {
        // Not checking the return value of js_runner_abort
        // because it is completely ambiguous here.
        js_runner_abort(data->js_runner, js_thread);
        // Assuming this will not block forever during normal operation.
        // The script should have stopped at this point either due to
        // its internal logic or due to the above abort request.
        furi_thread_join(js_thread);
        furi_thread_free(js_thread);
        data->js_thread = NULL;
    }

    furi_record_close(RECORD_JS_RUNNER);
}

static bool js_app_launcher_scene_run_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;
    JsAppLauncher* instance = context;

    JsAppLauncherSceneRun* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdRun);

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == JsAppLauncherCustomEventScriptFinished) {
            if(data->js_error == JsRunnerErrorNone) {
                scene_manager_previous_scene(instance->scene_manager);
            } else {
                instance->error = js_app_launcher_translate_from_js_runner_error(data->js_error);
                scene_manager_next_scene(instance->scene_manager, JsAppLauncherSceneIdError);
            }
        }

        consumed = true;
    } else if(event->type == SceneManagerEventTypeBack) {
        // TODO: Special Back key treatment?
        consumed = true;
    }

    return consumed;
}

const Scene js_app_launcher_scene_run = {
    .data_size = sizeof(JsAppLauncherSceneRun),
    .enter_callback = js_app_launcher_scene_run_on_enter,
    .exit_callback = js_app_launcher_scene_run_on_exit,
    .event_callback = js_app_launcher_scene_run_on_event,
};
