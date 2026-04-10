#include "update_executor_i.h"
#include "scenes/scenes.h"

#define TAG "UpdExec"

#define SUCCESS_SCENE_SHOW_TIME_MS 2500
#define REBOOT_SCENE_SHOW_TIME_MS  3000

static void update_executor_task_on_progress(
    const char* status,
    UpdateExecutorTaskStage stage,
    uint8_t percent,
    void* context) {
    UNUSED(percent);
    UNUSED(stage);

    UpdateExecutor* update_executor = context;

    update_executor->update_status = status;
    update_executor->update_percent = percent;

    if(stage == UpdateExecutorTaskStageCompleted) {
        scene_manager_replace_current_scene(
            update_executor->scene_manager, UpdateExecutorSceneIdxSuccess);
        furi_delay_ms(SUCCESS_SCENE_SHOW_TIME_MS);

        scene_manager_replace_current_scene(
            update_executor->scene_manager, UpdateExecutorSceneIdxReboot);
        furi_delay_ms(REBOOT_SCENE_SHOW_TIME_MS);

        return;
    }

    UpdateExecutorSceneIdx next_scene = (update_executor_task_stage_is_error(stage)) ?
                                            UpdateExecutorSceneIdxFailure :
                                            UpdateExecutorSceneIdxInstall;

    if(next_scene != scene_manager_get_current_scene_id(update_executor->scene_manager)) {
        scene_manager_replace_current_scene(update_executor->scene_manager, next_scene);
    }

    scene_manager_handle_custom_event(
        update_executor->scene_manager, UpdateExecutorEventUpdateProgress);
}

static UpdateExecutor* update_executor_alloc(void) {
    UpdateExecutor* instance = malloc(sizeof(*instance));

    instance->gui = furi_record_open(RECORD_GUI);
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->update_task = update_executor_task_alloc();

    instance->scene_manager = scene_manager_alloc(
        update_executor_internal_scenes, UpdateExecutorSceneIdxsCount, instance);

    instance->event_loop = furi_event_loop_alloc();

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);

        Widget* root_back = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = widget_alloc(root_back);

        Widget* root_front = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_container = widget_alloc(root_front);
    });

    scene_manager_next_scene(instance->scene_manager, UpdateExecutorSceneIdxInstall);

    update_executor_task_set_progress_callback(
        instance->update_task, update_executor_task_on_progress, instance);
    update_executor_task_start(instance->update_task);

    return instance;
}

static void update_executor_free(UpdateExecutor* instance) {
    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        widget_free(instance->back_container);
        widget_free(instance->front_container);
    });

    furi_event_loop_free(instance->event_loop);

    if(instance->update_task) {
        update_executor_task_set_progress_callback(instance->update_task, NULL, NULL);
        update_executor_task_free(instance->update_task);
    }

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);
    free(instance);
}

int32_t update_executor_srv(void* arg) {
    UNUSED(arg);

    UpdateExecutor* update_executor = update_executor_alloc();

    furi_event_loop_run(update_executor->event_loop);

    update_executor_free(update_executor);
    return 0;
}
