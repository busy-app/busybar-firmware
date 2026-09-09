#include "../js_app_launcher_i.h"
#include "js_app_launcher_scenes.h"

typedef struct {
    JsRunnerContextHandle* js_runner_handle;
    JsRunnerExecutionHandle* js_runner_exec_handle;
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

static void js_app_launcher_scene_run_terminated_callback(void* context) {
    furi_assert(context);

    JsAppLauncher* instance = context;
    js_app_launcher_send_custom_event(instance, JsAppLauncherCustomEventScriptFinished);
}

static bool js_app_launcher_scene_run_start_app(JsAppLauncher* instance) {
    bool success = false;

    JsRunner* runner = furi_record_open(RECORD_JS_RUNNER);
    JsAppLauncherSceneRun* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdRun);

    do {
        JsAppInfo js_info;

        if(!js_app_get_info(instance->js_app, &js_info)) {
            break;
        }

        const JsAppManifestInfo* js_manifest = &js_info.manifest;
        const JsRunnerContextInitResult init_result = js_runner_context_alloc(
            runner,
            js_manifest->id,
            js_manifest->heap_size,
            js_app_launcher_scene_run_console_out_callback,
            instance);

        if(init_result.error != JsRunnerErrorNone) {
            data->js_error = init_result.error;
            break;
        }

        data->js_runner_handle = init_result.handle;

        const JsRunnerRunResult run_result = js_runner_run(
            data->js_runner_handle,
            js_info.path.entry,
            js_app_launcher_scene_run_terminated_callback,
            instance);

        if(run_result.error != JsRunnerErrorNone) {
            data->js_error = run_result.error;
            break;
        }

        data->js_runner_exec_handle = run_result.handle;
        data->js_error = JsRunnerErrorNone;

        success = true;
    } while(false);

    return success;
}

static void js_app_launcher_scene_run_on_enter(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    if(!js_app_launcher_scene_run_start_app(instance)) {
        scene_manager_next_scene(instance->scene_manager, JsAppLauncherSceneIdError);
    }
}

static void js_app_launcher_scene_run_on_exit(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    JsAppLauncherSceneRun* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdRun);

    if(data->js_runner_exec_handle) {
        js_runner_abort(data->js_runner_exec_handle);
        furi_check(
            js_runner_join(data->js_runner_exec_handle, FuriWaitForever) == JsRunnerErrorNone);
        data->js_runner_exec_handle = NULL;
    }

    if(data->js_runner_handle) {
        js_runner_context_free(data->js_runner_handle);
        data->js_runner_handle = NULL;
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
