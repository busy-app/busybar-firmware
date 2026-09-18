#include "../js_app_launcher_i.h"
#include "js_app_launcher_scenes.h"

#include <gui/modules/flex_box.h>
#include <gui/modules/label.h>
#include <gui/modules/anim_player.h>

#define MESSAGE_LOADING "Loading..."
#define MESSAGE_RUNNING "Running..."

typedef struct {
    FlexBox* flex;
    Label* label;
    AnimPlayer* spinner;
} JsAppLauncherSceneRunWidgets;

typedef struct {
    JsAppLauncherSceneRunWidgets front_widgets;
    JsAppLauncherSceneRunWidgets back_widgets;
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

static void js_app_launcher_scene_run_event_callback(const JsRunnerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    JsAppLauncher* instance = context;

    if(event->type == JsRunnerEventTypeScriptStarted) {
        js_app_launcher_send_custom_event(instance, JsAppLauncherCustomEventScriptStarted);
    } else if(event->type == JsRunnerEventTypeScriptFinished) {
        js_app_launcher_send_custom_event(instance, JsAppLauncherCustomEventScriptFinished);
    }
}

static void js_app_launcher_scene_run_init_widgets(JsAppLauncher* instance) {
    JsAppLauncherSceneRun* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdRun);

    with_gui(instance->gui, {
        FlexBox* front_flex = flex_box_alloc(instance->front_window);
        FlexBox* back_flex = flex_box_alloc(instance->back_window);

        flex_box_set_flow(front_flex, FlexBoxFlowRow);
        flex_box_set_align(front_flex, FlexBoxAlignCenter, FlexBoxAlignCenter);
        flex_box_set_spacing(front_flex, 2);

        flex_box_set_flow(back_flex, FlexBoxFlowColumn);
        flex_box_set_align(back_flex, FlexBoxAlignCenter, FlexBoxAlignCenter);
        flex_box_set_spacing(back_flex, 9);

        widget_set_align(flex_box_get_base(front_flex), AlignCenter);
        widget_set_align(flex_box_get_base(back_flex), AlignCenter);

        AnimPlayer* front_spinner = anim_player_alloc(flex_box_get_base(front_flex));
        AnimPlayer* back_spinner = anim_player_alloc(flex_box_get_base(back_flex));

        anim_player_set_source(front_spinner, SHARED_ANIM_PATH("spinner_front_8x8.anim"));
        anim_player_set_source(back_spinner, SHARED_ANIM_PATH("spinner_back_16x16.anim"));

        Label* front_label = label_alloc(flex_box_get_base(front_flex));
        Label* back_label = label_alloc(flex_box_get_base(back_flex));

        label_set_text(front_label, MESSAGE_LOADING);
        label_set_text(back_label, MESSAGE_LOADING);

        data->front_widgets.flex = front_flex;
        data->front_widgets.label = front_label;
        data->front_widgets.spinner = front_spinner;

        data->back_widgets.flex = back_flex;
        data->back_widgets.label = back_label;
        data->back_widgets.spinner = back_spinner;
    });
}

static void js_app_launcher_scene_run_deinit_widgets(JsAppLauncher* instance) {
    JsAppLauncherSceneRun* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdRun);

    with_gui(instance->gui, {
        JsAppLauncherSceneRunWidgets* front_widgets = &data->front_widgets;
        JsAppLauncherSceneRunWidgets* back_widgets = &data->back_widgets;

        if(front_widgets->flex != NULL) {
            flex_box_free(front_widgets->flex);
        }
        if(back_widgets->flex != NULL) {
            flex_box_free(back_widgets->flex);
        }
    });
}

static bool js_app_launcher_scene_run_start_app(JsAppLauncher* instance) {
    bool success = false;

    JsRunner* runner = furi_record_open(RECORD_JS_RUNNER);
    JsAppLauncherSceneRun* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdRun);

    data->js_error = JsRunnerErrorUnknown;

    do {
        JsAppInfo js_info;

        if(!js_app_get_info(instance->js_app, &js_info)) {
            break;
        }

        js_app_launcher_scene_run_init_widgets(instance);

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
            js_app_launcher_scene_run_event_callback,
            instance);

        if(run_result.error != JsRunnerErrorNone) {
            data->js_error = run_result.error;
            break;
        }

        data->js_runner_exec_handle = run_result.handle;
        data->js_error = JsRunnerErrorNone;

        success = true;
    } while(false);

    instance->error = js_app_launcher_translate_from_js_runner_error(data->js_error);

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

    js_app_launcher_scene_run_deinit_widgets(instance);

    furi_record_close(RECORD_JS_RUNNER);
}

static void js_app_launcher_scene_run_handle_script_started(JsAppLauncher* instance) {
    JsAppLauncherSceneRun* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdRun);

    with_gui(instance->gui, {
        JsAppLauncherSceneRunWidgets* front_widgets = &data->front_widgets;
        JsAppLauncherSceneRunWidgets* back_widgets = &data->back_widgets;

        anim_player_pause(front_widgets->spinner);
        anim_player_pause(back_widgets->spinner);

        widget_set_visible(anim_player_get_base(front_widgets->spinner), false);
        widget_set_visible(anim_player_get_base(back_widgets->spinner), false);

        label_set_text(front_widgets->label, MESSAGE_RUNNING);
        label_set_text(back_widgets->label, MESSAGE_RUNNING);
    });
}

static void js_app_launcher_scene_run_handle_script_finished(JsAppLauncher* instance) {
    JsAppLauncherSceneRun* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdRun);

    if(data->js_error == JsRunnerErrorNone) {
        scene_manager_previous_scene(instance->scene_manager);
    } else {
        instance->error = js_app_launcher_translate_from_js_runner_error(data->js_error);
        scene_manager_next_scene(instance->scene_manager, JsAppLauncherSceneIdError);
    }
}

static bool js_app_launcher_scene_run_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;
    JsAppLauncher* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == JsAppLauncherCustomEventScriptStarted) {
            js_app_launcher_scene_run_handle_script_started(instance);
        } else if(event->event == JsAppLauncherCustomEventScriptFinished) {
            js_app_launcher_scene_run_handle_script_finished(instance);
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
