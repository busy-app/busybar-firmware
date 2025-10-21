#include "../busy.h"
#include "../widgets/pause_overlay.h"
#include "../widgets/timer_label.h"

#include <gui/modules/lottie_animation.h>
#include <gui/modules/image.h>

#include <toolbox/vector3.h>

#define COUNTDOWN_THRESHOLD_S  (3)
#define PROGRESS_TRANSITION_MS (1000)

#define SLOT_TEMPLATE     \
    "{"                   \
    " \"wave_offset\": {" \
    "  \"p\": {"          \
    "   \"a\": 0,"        \
    "   \"k\": ["         \
    "     %f,"            \
    "     %f,"            \
    "     %f"             \
    "   ]"                \
    "}}}"

static const Vector3 busy_position_start = {3.0f, 100.f, 100.f};
static const Vector3 busy_position_end = {96.5f, 100.f, 100.f};

typedef struct {
    Widget* root;
    TimerLabel* timer_label;
    PauseOverlay* pause_overlay;

    AnimImage* anim_image;
    LottieAnimation* lottie;
    FuriString* lottie_text_store;
    Image* image;

    RunLater* run_later;

    BusyTimerTime timer_time;
} BusySceneTimerSimple;

static void busy_scene_timer_simple_update_lights(BusyApp* instance, bool is_paused) {
    if(is_paused) {
        busy_set_status_lights(instance, BusyStatusLightsTypeOff);
    } else {
        busy_set_status_lights(instance, BusyStatusLightsTypeWork);
    }
}

static void busy_scene_timer_simple_toggle_pause(BusyApp* instance) {
    BusySceneTimerSimple* data = scene_manager_get_current_scene_data(instance->scene_manager);

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

    busy_scene_timer_simple_update_lights(instance, is_paused);
}

static bool busy_scene_timer_simple_input_callback(const InputEvent* event, void* context) {
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
        }
    }

    return false;
}

static void busy_scene_timer_simple_event_callback(const BusyTimerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimerSimple* data = scene_manager_get_current_scene_data(instance->scene_manager);

    if(event->type == BusyTimerEventTypeTick) {
        data->timer_time = event->time;
        busy_send_custom_event(instance, BusyCustomEventTimerTick);
    } else if(event->type == BusyTimerEventTypeIntervalEnded) {
        busy_send_custom_event(instance, BusyCustomEventTimerIntervalEnded);
    }
}

void busy_scene_timer_simple_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimerSimple* data = scene_manager_get_current_scene_data(instance->scene_manager);
    data->lottie_text_store = furi_string_alloc();

    with_gui(instance->gui, {
        gui_layer_add_input_callback(
            gui_get_layer(instance->gui, GuiLayerIdMain),
            busy_scene_timer_simple_input_callback,
            instance);

        data->root = widget_alloc(instance->front_window);

        data->anim_image = anim_image_alloc(data->root);
        anim_image_set_source(
            data->anim_image, "/ext/apps_assets/busy/animations/busy_particles_41x16.anim");
        anim_image_set_loop(data->anim_image, true);
        anim_image_start(data->anim_image);
        widget_set_pos(anim_image_get_base(data->anim_image), -1, 0);

        data->lottie = lottie_animation_alloc(data->root);
        lottie_animation_set_source(
            data->lottie, "/ext/apps_assets/busy/busy_label_progress_lottie_small.json", 0);
        widget_set_pos(lottie_animation_get_base(data->lottie), -1, 0);

        data->image = image_alloc(data->root);
        image_set_source(data->image, "/ext/apps_assets/busy/images/busy_text_label_41x16.bin");
        widget_set_pos(image_get_base(data->image), -1, 0);

        data->timer_label = timer_label_alloc(data->root);
        widget_set_pos(timer_label_get_base(data->timer_label), 31 + 11, 1);

        data->pause_overlay = pause_overlay_alloc(instance->front_window);

        widget_set_visible(timer_card_get_base(instance->timer_card), true);
        timer_card_show_header(instance->timer_card, true);
        timer_card_show_time(instance->timer_card, true);
    });

    busy_timer_set_callback(
        instance->busy_timer, busy_scene_timer_simple_event_callback, instance);
    busy_timer_start(instance->busy_timer);

    busy_start_transition(instance);
}

void busy_scene_timer_simple_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;

    busy_set_status_lights(instance, BusyStatusLightsTypeOff);
    busy_timer_set_callback(instance->busy_timer, NULL, NULL);

    BusySceneTimerSimple* data = scene_manager_get_current_scene_data(instance->scene_manager);
    furi_string_free(data->lottie_text_store);

    with_gui(instance->gui, {
        gui_layer_remove_input_callback(
            gui_get_layer(instance->gui, GuiLayerIdMain), busy_scene_timer_simple_input_callback);

        timer_card_show_header(instance->timer_card, false);
        timer_card_show_time(instance->timer_card, false);

        widget_free(data->root);
        pause_overlay_free(data->pause_overlay);
    });
}

static void busy_scene_timer_simple_handle_back(BusyApp* instance) {
    if(busy_timer_is_running(instance->busy_timer)) {
        busy_timer_stop(instance->busy_timer);

        busy_prepare_transition(instance, BusyTransitionTypeDefault);

        scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, BusyAppSceneIdStart);
    } else {
        busy_scene_timer_simple_toggle_pause(instance);
    }
}

static void busy_scene_timer_simple_lottie_override_slot(BusyApp* instance, Vector3 vector) {
    const BusySceneTimerSimple* data =
        scene_manager_get_current_scene_data(instance->scene_manager);

    furi_string_printf(data->lottie_text_store, SLOT_TEMPLATE, vector.x, vector.y, vector.z);

    if(!lottie_animation_override_slot(
           data->lottie, furi_string_get_cstr(data->lottie_text_store))) {
        FURI_LOG_E(TAG, "Failed to override slot");
    }
}

static void busy_scene_timer_simple_update_tick(BusyApp* instance) {
    const BusySceneTimerSimple* data =
        scene_manager_get_current_scene_data(instance->scene_manager);
    const BusyTimerTime* time = &data->timer_time;

    const float progress = (float)time->elapsed_s / (time->elapsed_s + time->remain_s);

    with_gui(instance->gui, {
        Vector3 position = vector3_lerp(busy_position_start, busy_position_end, progress);
        busy_scene_timer_simple_lottie_override_slot(instance, position);

        timer_label_set_time(data->timer_label, data->timer_time.remain_s);
        timer_card_set_time(instance->timer_card, data->timer_time.remain_s);
    });

    if(time->remain_s == 0) {
        audio_play_file(instance->audio, BUSY_SOUND_PATH("countdown_finish.snd"));
    } else if(time->remain_s <= COUNTDOWN_THRESHOLD_S) {
        audio_play_file(instance->audio, BUSY_SOUND_PATH("countdown_tick.snd"));
    }
}

static void busy_scene_timer_simple_run_later_callback(void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    busy_prepare_transition(instance, BusyTransitionTypeWorkDone);
    scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdNext);
}

static void busy_scene_timer_simple_go_to_progress_scene(BusyApp* instance) {
    BusySceneTimerSimple* data = scene_manager_get_current_scene_data(instance->scene_manager);

    furi_assert(data->run_later == NULL);

    data->run_later = run_later(
        instance->event_loop,
        busy_scene_timer_simple_run_later_callback,
        instance,
        PROGRESS_TRANSITION_MS);
}

static bool busy_scene_timer_simple_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusyCustomEventTimerTick) {
            busy_scene_timer_simple_update_tick(instance);
        } else if(event->event == BusyCustomEventTimerIntervalEnded) {
            busy_scene_timer_simple_go_to_progress_scene(instance);
        } else if(event->event == BusyCustomEventTimerTogglePause) {
            busy_scene_timer_simple_toggle_pause(instance);
        } else if(event->event == BusyCustomEventTimeIncrement) {
            busy_timer_add_time(instance->busy_timer, BUSY_TIMER_TIME_INCREMENT_MN);
        } else if(event->event == BusyCustomEventTimeDecrement) {
            busy_timer_add_time(instance->busy_timer, -BUSY_TIMER_TIME_INCREMENT_MN);
        }

        consumed = true;
    } else if(event->type == SceneManagerEventTypeBack) {
        busy_scene_timer_simple_handle_back(instance);

        consumed = true;
    }

    return consumed;
}
const Scene busy_scene_timer_simple = {
    .enter_callback = busy_scene_timer_simple_on_enter,
    .exit_callback = busy_scene_timer_simple_on_exit,
    .event_callback = busy_scene_timer_simple_on_event,
    .data_size = sizeof(BusySceneTimerSimple),
};
