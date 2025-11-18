#include "update_app_i.h"
#include "scenes/updater_scenes.h"

#include <furi.h>
#include <furi_hal_flash.h>
#include <furi_hal_power.h>
#include <furi_hal_nvm.h>

#include <toolbox/path.h>
#include <toolbox/update_lib/dfu_file.h>
#include <toolbox/update_lib/update_manifest.h>
#include <toolbox/update_lib/common_vals.h>
#include <toolbox/update_lib/update_config.h>

#define TAG "UpdaterApp"

static void updater_task_on_progress(
    const char* status,
    UpdateTaskStage stage,
    uint8_t percent,
    void* context) {
    UNUSED(percent);
    UNUSED(stage);

    UpdaterApp* updater = context;

    updater->update_status = status;
    updater->update_percent = percent;

    UpdaterAppSceneId next_scene = (stage == UpdateTaskStageCompleted) ? UpdaterAppSceneIdSuccess :
                                   (update_stage_is_error(stage))      ? UpdaterAppSceneIdFail :
                                                                         UpdaterAppSceneIdInstall;

    if(next_scene != scene_manager_get_current_scene_id(updater->scene_manager)) {
        scene_manager_replace_current_scene(updater->scene_manager, next_scene);
    }

    scene_manager_handle_custom_event(updater->scene_manager, 0);
}

static UpdaterApp* updater_alloc(void) {
    UpdaterApp* instance = malloc(sizeof(UpdaterApp));

    instance->gui = furi_record_open(RECORD_GUI);
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->update_task = update_task_alloc();
    instance->event_loop = furi_event_loop_alloc();
    instance->scene_manager =
        scene_manager_alloc(updater_scenes, UpdaterAppSceneIdsCount, instance);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);

        Widget* root_back = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = widget_alloc(root_back);

        Widget* root_front = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_container = widget_alloc(root_front);
    });

    scene_manager_next_scene(instance->scene_manager, UpdaterAppSceneIdInstall);

    update_task_set_progress_callback(instance->update_task, updater_task_on_progress, instance);
    update_task_start(instance->update_task);

    return instance;
}

static void updater_free(UpdaterApp* instance) {
    with_gui(instance->gui, {
        widget_free(instance->back_container);
        widget_free(instance->front_container);
    });

    scene_manager_free(instance->scene_manager);
    furi_event_loop_free(instance->event_loop);

    if(instance->update_task) {
        update_task_set_progress_callback(instance->update_task, NULL, NULL);
        update_task_free(instance->update_task);
    }

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);
    free(instance);
}

int32_t updater_srv(void* arg) {
    UNUSED(arg);

    UpdaterApp* updater = updater_alloc();

    furi_event_loop_run(updater->event_loop);

    updater_free(updater);
    return 0;
}
