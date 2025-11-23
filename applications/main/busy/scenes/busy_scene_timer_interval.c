#include "../busy.h"
#include "../busy_presets.h"
#include "../widgets/pause_overlay.h"
#include "../widgets/timer_label.h"

#include <gui/modules/lottie_animation.h>
#include <gui/modules/image.h>

#define COUNTDOWN_THRESHOLD_S  (3)
#define PROGRESS_TRANSITION_MS (1000)

typedef struct {
    Widget* root;
    TimerLabel* timer_label;
    PauseOverlay* pause_overlay;
    AnimImage* anim_image;
    LottieAnimation* lottie;
    FuriString* lottie_text_store;
    Image* image;
    Widget* label;
    const BusySceneTimerIntervalAsset* asset;
    BusyTimerTime timer_time;
    BusyTimerState timer_state;
} BusySceneTimerInterval;

static void
    busy_scene_timer_interval_update_lights(BusyApp* instance, bool is_paused, bool is_rest) {
    if(is_paused) {
        busy_set_status_lights(instance, BusyStatusLightsTypeOff);
    } else if(is_rest) {
        busy_set_status_lights(instance, BusyStatusLightsTypeRest);
    } else {
        busy_set_status_lights(instance, BusyStatusLightsTypeWork);
    }
}

static void busy_scene_timer_interval_update_matter(BusyApp* instance, bool is_paused) {
    busy_set_matter(instance, !is_paused);
}

static void busy_scene_timer_interval_toggle_pause(BusyApp* instance) {
    BusySceneTimerInterval* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimerInterval);

    busy_timer_toggle(instance->busy_timer);
    bool is_paused = !busy_timer_is_running(instance->busy_timer);
    bool is_rest = busy_timer_get_state(instance->busy_timer) == BusyTimerStateRest;

    with_gui(instance->gui, {
        pause_overlay_show(data->pause_overlay, is_paused);
        timer_card_show_header(instance->timer_card, !is_paused);

        if(is_paused) {
            anim_image_stop(data->anim_image);
        } else {
            anim_image_start(data->anim_image);
        }
    });

    busy_scene_timer_interval_update_lights(instance, is_paused, is_rest);
    busy_scene_timer_interval_update_matter(instance, is_paused);
}

static bool busy_scene_timer_interval_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            busy_send_custom_event(instance, BusyCustomEventTimeIncrement);
            return true;
        } else if(event->key == InputKeyDown) {
            busy_send_custom_event(instance, BusyCustomEventTimeDecrement);
            return true;
        } else if(event->key == InputKeyStart) {
            busy_send_custom_event(instance, BusyCustomEventTimerTogglePause);
            return true;
        } else if(event->key == InputKeyOk) {
            busy_send_custom_event(instance, BusyCustomEventTimerSkip);
            return true;
        }
    }

    return false;
}

static void busy_scene_timer_interval_event_callback(const BusyTimerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimerInterval* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimerInterval);

    if(event->type == BusyTimerEventTypeTick) {
        data->timer_time = event->time;
        busy_send_custom_event(instance, BusyCustomEventTimerTick);
    } else if(event->type == BusyTimerEventTypeIntervalEnded) {
        busy_send_custom_event(instance, BusyCustomEventTimerIntervalEnded);
    } else if(event->type == BusyTimerEventTypeStateChanged) {
        data->timer_state = event->state;
        busy_send_custom_event(instance, BusyCustomEventTimerStateChanged);
    }
}

static void busy_scene_timer_interval_update_gui_assets(BusyApp* instance) {
    BusySceneTimerInterval* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimerInterval);
    BusyTimerState state = busy_timer_get_state(instance->busy_timer);
    data->asset = (state == BusyTimerStateWork) ?
                      &busy_scene_timer_interval_assets[BusySceneTimerIntervalAssetIdBusy] :
                      &busy_scene_timer_interval_assets[BusySceneTimerIntervalAssetIdRest];

    if(data->anim_image) anim_image_free(data->anim_image);
    if(data->lottie) lottie_animation_free(data->lottie);
    if(data->image) image_free(data->image);

    data->anim_image = anim_image_alloc(data->label);
    data->lottie = lottie_animation_alloc(data->label);
    data->image = image_alloc(data->label);

    anim_image_set_source(data->anim_image, data->asset->anim_path);
    anim_image_set_loop(data->anim_image, true);
    anim_image_start(data->anim_image);

    lottie_animation_set_source(data->lottie, data->asset->lottie_path, 0);

    image_set_source(data->image, data->asset->image_path);

    timer_label_set_countdown_colors(
        data->timer_label, data->asset->countdown_main_color, data->asset->countdown_blink_color);

    busy_scene_timer_interval_update_lights(instance, false, state == BusyTimerStateRest);
}

void busy_scene_timer_interval_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimerInterval* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimerInterval);
    data->lottie_text_store = furi_string_alloc();

    busy_timer_set_callback(
        instance->busy_timer, busy_scene_timer_interval_event_callback, instance);
    busy_timer_start(instance->busy_timer);

    busy_timer_get_time(instance->busy_timer, &data->timer_time);

    data->asset = (busy_timer_get_state(instance->busy_timer) == BusyTimerStateWork) ?
                      &busy_scene_timer_interval_assets[BusySceneTimerIntervalAssetIdBusy] :
                      &busy_scene_timer_interval_assets[BusySceneTimerIntervalAssetIdRest];

    data->anim_image = NULL;
    data->lottie = NULL;
    data->image = NULL;

    with_gui(instance->gui, {
        gui_layer_add_input_callback(
            gui_get_layer(instance->gui, GuiLayerIdMain),
            busy_scene_timer_interval_input_callback,
            instance);

        data->root = widget_alloc(instance->front_window);

        data->label = widget_alloc(data->root);
        widget_set_pos(data->label, 0, 0);

        data->timer_label = timer_label_alloc(data->root);
        timer_label_set_time(data->timer_label, data->timer_time.remain_s);
        widget_set_pos(timer_label_get_base(data->timer_label), 31 + 11, 1);

        data->pause_overlay = pause_overlay_alloc(instance->front_window);

        busy_scene_timer_interval_update_gui_assets(instance);

        widget_set_visible(timer_card_get_base(instance->timer_card), true);
        timer_card_show_header(instance->timer_card, true);
        timer_card_set_time(instance->timer_card, data->timer_time.remain_s);
        timer_card_show_time(instance->timer_card, true);
    });

    busy_start_transition(instance);
}

void busy_scene_timer_interval_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;

    busy_set_status_lights(instance, BusyStatusLightsTypeOff);
    busy_timer_set_callback(instance->busy_timer, NULL, NULL);

    BusySceneTimerInterval* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimerInterval);
    furi_string_free(data->lottie_text_store);

    with_gui(instance->gui, {
        gui_layer_remove_input_callback(
            gui_get_layer(instance->gui, GuiLayerIdMain),
            busy_scene_timer_interval_input_callback);

        timer_card_show_header(instance->timer_card, false);
        timer_card_show_time(instance->timer_card, false);

        widget_free(data->root);
        pause_overlay_free(data->pause_overlay);
    });
}

static void busy_scene_timer_interval_handle_back(BusyApp* instance) {
    if(busy_timer_is_running(instance->busy_timer)) {
        busy_timer_stop(instance->busy_timer);

        busy_prepare_transition(instance, BusyTransitionTypeDefault);

        scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, BusyAppSceneIdStart);
    } else {
        busy_scene_timer_interval_toggle_pause(instance);
    }
}

static void busy_scene_timer_interval_lottie_override_slot(BusyApp* instance, Vector3 vector) {
    const BusySceneTimerInterval* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimerInterval);

    furi_string_printf(
        data->lottie_text_store, BUSY_LOTTIE_SLOT_TEMPLATE, vector.x, vector.y, vector.z);

    if(!lottie_animation_override_slot(
           data->lottie, furi_string_get_cstr(data->lottie_text_store))) {
        FURI_LOG_E(TAG, "Failed to override slot");
    }
}

static void busy_scene_timer_interval_update_tick(BusyApp* instance) {
    const BusySceneTimerInterval* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimerInterval);
    const BusyTimerTime* time = &data->timer_time;

    const float progress = (float)time->elapsed_s / (time->elapsed_s + time->remain_s);

    with_gui(instance->gui, {
        Vector3 position =
            vector3_lerp(data->asset->position_start, data->asset->position_end, progress);
        busy_scene_timer_interval_lottie_override_slot(instance, position);

        timer_label_set_time(data->timer_label, data->timer_time.remain_s);
        timer_card_set_time(instance->timer_card, data->timer_time.remain_s);
    });

    if(time->remain_s == 0) {
        audio_play_file(instance->audio, BUSY_SOUND_PATH("countdown_finish.snd"));
    } else if(time->remain_s <= COUNTDOWN_THRESHOLD_S) {
        audio_play_file(instance->audio, BUSY_SOUND_PATH("countdown_tick.snd"));
    }
}

static void busy_scene_timer_interval_go_to_progress_scene(BusyApp* instance) {
    busy_prepare_transition(instance, BusyTransitionTypeWorkDone);
    scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdNext);
}

static void busy_scene_timer_interval_handle_skip(BusyApp* instance) {
    if(busy_timer_is_running(instance->busy_timer)) {
        busy_prepare_transition(instance, BusyTransitionTypeSkip);
        busy_start_transition(instance);
        busy_timer_skip(instance->busy_timer);
        with_gui(instance->gui, { busy_scene_timer_interval_update_gui_assets(instance); });
    }
}

static void busy_scene_timer_interval_handle_state_change(BusyApp* instance) {
    with_gui(instance->gui, { busy_scene_timer_interval_update_gui_assets(instance); });
}

static bool busy_scene_timer_interval_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusyCustomEventTimerTick) {
            busy_scene_timer_interval_update_tick(instance);
        } else if(event->event == BusyCustomEventTimerIntervalEnded) {
            busy_scene_timer_interval_go_to_progress_scene(instance);
        } else if(event->event == BusyCustomEventTimerTogglePause) {
            busy_scene_timer_interval_toggle_pause(instance);
        } else if(event->event == BusyCustomEventTimerSkip) {
            busy_scene_timer_interval_handle_skip(instance);
        } else if(event->event == BusyCustomEventTimerStateChanged) {
            busy_scene_timer_interval_handle_state_change(instance);
        } else if(event->event == BusyCustomEventTimeIncrement) {
            busy_timer_add_time(instance->busy_timer, BUSY_TIMER_TIME_INCREMENT_MN);
        } else if(event->event == BusyCustomEventTimeDecrement) {
            busy_timer_add_time(instance->busy_timer, -BUSY_TIMER_TIME_INCREMENT_MN);
        }

        consumed = true;
    } else if(event->type == SceneManagerEventTypeBack) {
        busy_scene_timer_interval_handle_back(instance);

        consumed = true;
    }

    return consumed;
}
const Scene busy_scene_timer_interval = {
    .enter_callback = busy_scene_timer_interval_on_enter,
    .exit_callback = busy_scene_timer_interval_on_exit,
    .event_callback = busy_scene_timer_interval_on_event,
    .data_size = sizeof(BusySceneTimerInterval),
};
