#include "../update_ui_i.h"

#include <gui/modules/flex_box.h>
#include <gui/modules/label.h>
#include <gui/modules/anim_player.h>

typedef enum {
    UpdateUiPrepareSceneEventUpdateStateChange = UpdateUiEventSceneEventsStart,
} UpdateUiPrepareSceneEvent;

typedef struct {
    FlexBox* front_box;
    FlexBox* back_box;

    FuriStateSub* update_state_subscription;
} UpdateUiPrepareScene;

static inline UpdateUiPrepareScene* update_ui_prepare_scene_get(UpdateUi* instance) {
    return scene_manager_get_scene_data(instance->scene_manager, UpdateUiSceneIdxPrepare);
}

static void update_ui_prepare_scene_update_state_callback(const void* item, void* context) {
    UNUSED(item);

    UpdateUi* instance = context;

    update_ui_internal_fire_event(instance, UpdateUiPrepareSceneEventUpdateStateChange);
}

static void update_ui_prepare_scene_on_update_state_change(UpdateUi* instance) {
    UpdaterUpdateState update_state;
    furi_state_get(updater_get_update_state(instance->updater), &update_state);

    if(update_state.status != UpdaterStatusOk && update_state.status != UpdaterStatusBusy) {
        furi_string_set(instance->failure_preset.front_text, "Update failed");
        furi_string_set(instance->failure_preset.back_primary_text, "Update failed");
        furi_string_set(
            instance->failure_preset.back_detail_text,
            updater_get_status_string(update_state.status));

        scene_manager_replace_current_scene(instance->scene_manager, UpdateUiSceneIdxFailure);
        return;
    }
}

static void update_ui_prepare_scene_on_enter(void* context) {
    UpdateUi* instance = context;
    UpdateUiPrepareScene* scene = update_ui_prepare_scene_get(instance);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_box = flex_box_alloc(instance->front_scene_window);
        flex_box_set_flow(scene->front_box, FlexBoxFlowRow);
        flex_box_set_align(scene->front_box, FlexBoxAlignStart, FlexBoxAlignCenter);
        flex_box_set_spacing(scene->front_box, 2);
        widget_set_align(flex_box_get_base(scene->front_box), AlignLeftMid);

        AnimPlayer* front_anim = anim_player_alloc(flex_box_get_base(scene->front_box));
        anim_player_set_source(front_anim, SHARED_ANIM_PATH("spinner_front_8x8.anim"));

        Label* front_label = label_alloc(flex_box_get_base(scene->front_box));
        label_set_line_spacing(front_label, 0);
        label_set_text(front_label, "Preparing update");

        /* back layout setup */
        scene->back_box = flex_box_alloc(instance->back_scene_window);
        flex_box_set_flow(scene->back_box, FlexBoxFlowColumn);
        flex_box_set_align(scene->back_box, FlexBoxAlignCenter, FlexBoxAlignCenter);
        flex_box_set_spacing(scene->back_box, 6);
        widget_set_align(flex_box_get_base(scene->back_box), AlignCenter);

        AnimPlayer* back_anim = anim_player_alloc(flex_box_get_base(scene->back_box));
        anim_player_set_source(back_anim, SHARED_ANIM_PATH("spinner_back_16x16.anim"));

        Label* back_label = label_alloc(flex_box_get_base(scene->back_box));
        label_set_text(back_label, "Preparing update...");
        label_set_text_align(back_label, TextAlignCenter);
    });

    scene->update_state_subscription = furi_state_subscribe(
        updater_get_update_state(instance->updater),
        update_ui_prepare_scene_update_state_callback,
        instance);

    update_ui_prepare_scene_on_update_state_change(instance);
}

static void update_ui_prepare_scene_on_exit(void* context) {
    UpdateUi* instance = context;
    UpdateUiPrepareScene* scene = update_ui_prepare_scene_get(instance);

    furi_state_unsubscribe(scene->update_state_subscription);

    with_gui(instance->gui, {
        flex_box_free(scene->back_box);
        flex_box_free(scene->front_box);
    });
}

static bool update_ui_prepare_scene_on_event(const SceneManagerEvent* event, void* context) {
    UpdateUi* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case UpdateUiPrepareSceneEventUpdateStateChange:
            update_ui_prepare_scene_on_update_state_change(instance);
            break;

        default:
            return false;
        }
    }

    return true;
}

const Scene update_ui_internal_scene_prepare = {
    .enter_callback = update_ui_prepare_scene_on_enter,
    .exit_callback = update_ui_prepare_scene_on_exit,
    .event_callback = update_ui_prepare_scene_on_event,
    .data_size = sizeof(UpdateUiPrepareScene),
};
