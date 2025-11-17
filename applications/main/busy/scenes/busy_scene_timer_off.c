#include "../busy.h"
#include "../widgets/pause_overlay.h"

#include <gui/modules/image.h>

typedef struct {
    Widget* root;
    PauseOverlay* pause_overlay;
    AnimImage* anim_image;
} BusySceneTimerOff;

static void busy_scene_timer_off_update_lights(BusyApp* instance, bool is_paused) {
    if(is_paused) {
        busy_set_status_lights(instance, BusyStatusLightsTypeOff);
    } else {
        busy_set_status_lights(instance, BusyStatusLightsTypeWork);
    }
}

static void busy_scene_timer_old_update_matter(BusyApp* instance, bool is_paused) {
    busy_set_matter(instance, !is_paused);
}

static void busy_scene_timer_off_toggle_pause(BusyApp* instance) {
    BusySceneTimerOff* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimerOff);

    busy_timer_toggle(instance->busy_timer);
    bool is_paused = !busy_timer_is_running(instance->busy_timer);

    with_gui(instance->gui, {
        pause_overlay_show(data->pause_overlay, is_paused);
        timer_card_show_header(instance->timer_card, !is_paused);

        if(is_paused) {
            anim_image_stop(data->anim_image);
        } else {
            anim_image_start(data->anim_image);
        }
    });

    busy_scene_timer_off_update_lights(instance, is_paused);
    busy_scene_timer_old_update_matter(instance, is_paused);
}

static bool busy_scene_timer_off_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            busy_send_custom_event(instance, BusyCustomEventTimeIncrement);
            return true;
        } else if(event->key == InputKeyStart) {
            busy_send_custom_event(instance, BusyCustomEventTimerTogglePause);
            return true;
        }
    }

    return false;
}

void busy_scene_timer_off_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimerOff* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimerOff);

    with_gui(instance->gui, {
        gui_layer_add_input_callback(
            gui_get_layer(instance->gui, GuiLayerIdMain),
            busy_scene_timer_off_input_callback,
            instance);

        data->root = widget_alloc(instance->front_window);

        data->anim_image = anim_image_alloc(data->root);
        anim_image_set_source(
            data->anim_image, "/ext/apps_assets/busy/animations/busy_label_72x16.anim");
        anim_image_set_loop(data->anim_image, true);
        anim_image_start(data->anim_image);

        data->pause_overlay = pause_overlay_alloc(instance->front_window);

        widget_set_visible(timer_card_get_base(instance->timer_card), true);
        timer_card_show_header(instance->timer_card, true);
        timer_card_show_time(instance->timer_card, true);
    });

    busy_timer_start(instance->busy_timer);

    busy_start_transition(instance);
}

void busy_scene_timer_off_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;

    busy_set_status_lights(instance, BusyStatusLightsTypeOff);

    BusySceneTimerOff* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimerOff);

    with_gui(instance->gui, {
        gui_layer_remove_input_callback(
            gui_get_layer(instance->gui, GuiLayerIdMain), busy_scene_timer_off_input_callback);

        timer_card_show_header(instance->timer_card, false);
        timer_card_show_time(instance->timer_card, false);

        widget_free(data->root);
        pause_overlay_free(data->pause_overlay);
    });
}

static void busy_scene_timer_off_handle_back(BusyApp* instance) {
    if(busy_timer_is_running(instance->busy_timer)) {
        busy_timer_stop(instance->busy_timer);

        busy_prepare_transition(instance, BusyTransitionTypeDefault);

        scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, BusyAppSceneIdStart);
    } else {
        busy_scene_timer_off_toggle_pause(instance);
    }
}

static bool busy_scene_timer_off_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusyCustomEventTimerTogglePause) {
            busy_scene_timer_off_toggle_pause(instance);
        } else if(event->event == BusyCustomEventTimeIncrement) {
            if(busy_timer_is_running(instance->busy_timer)) {
                busy_timer_add_time(instance->busy_timer, BUSY_TIMER_TIME_INCREMENT_MN);
                scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdTimerOffToSimple);
            }
        }

        consumed = true;
    } else if(event->type == SceneManagerEventTypeBack) {
        busy_scene_timer_off_handle_back(instance);

        consumed = true;
    }

    return consumed;
}

const Scene busy_scene_timer_off = {
    .enter_callback = busy_scene_timer_off_on_enter,
    .exit_callback = busy_scene_timer_off_on_exit,
    .event_callback = busy_scene_timer_off_on_event,
    .data_size = sizeof(BusySceneTimerOff),
};
