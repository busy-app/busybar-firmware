#include "../busy.h"

#include <gui/modules/image.h>
#include <gui/modules/anim_image.h>

#include "../widgets/timer_card.h"
#include "../widgets/timer_label.h"
#include "../widgets/progress_bar.h"
#include "../widgets/pause_overlay.h"

#define PROGRESS_BAR_COLOR_BUSY color_hex_to_rgb(0x4A0000)
#define PROGRESS_BAR_COLOR_REST color_hex_to_rgb(0x011809)

typedef struct {
    AnimImage* state_image;
    TimerLabel* timer_label;
    ProgressBar* progress_bar;
    PauseOverlay* pause_overlay;
    TimerCard* timer_card;
    BusyTimerTime timer_time;
    BusyTimerState timer_state;
    bool is_paused;
} BusySceneTimer;

static bool busy_scene_timer_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;

    bool consumed = false;
    BusyCustomEvent custom_event;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            custom_event = BusyCustomEventTimeIncrement;
            consumed = true;

        } else if(event->key == InputKeyDown) {
            custom_event = BusyCustomEventTimeDecrement;
            consumed = true;

        } else if(event->key == InputKeyOk) {
            custom_event = BusyCustomEventTimerSkip;
            consumed = true;

        } else if(event->key == InputKeyStart) {
            custom_event = BusyCustomEventTimerToggle;
            consumed = true;
        }
    }

    if(consumed) {
        busy_send_custom_event(instance, custom_event);
    }

    return consumed;
}

static void busy_scene_timer_event_callback(const BusyTimerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    if(event->type == BusyTimerEventTypeTick) {
        data->timer_time = event->time;
        busy_send_custom_event(instance, BusyCustomEventTimerTick);

    } else if(event->type == BusyTimerEventTypeStateChanged) {
        data->timer_state = event->state;
        busy_send_custom_event(instance, BusyCustomEventTimerStateChanged);
    }
}

static void busy_scene_timer_update_tick(BusyApp* instance) {
    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);
    const BusyTimerTime* time = &data->timer_time;

    const float progress = (float)time->elapsed_s / (time->elapsed_s + time->remain_s);

    with_gui(instance->gui, {
        progress_bar_set_value(data->progress_bar, progress);
        timer_label_set_time(data->timer_label, data->timer_time.remain_s);
        timer_card_set_time(data->timer_card, data->timer_time.remain_s);
    });
}

static void busy_scene_timer_update_state(BusyApp* instance) {
    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        if(data->timer_state == BusyTimerStateWork) {
            anim_image_set_source(data->state_image, BUSY_ANIM_PATH("A_busy_label_40x14.anim"));
            anim_image_start(data->state_image);
            progress_bar_set_trough_color(data->progress_bar, PROGRESS_BAR_COLOR_BUSY);
            progress_bar_set_anim_source(
                data->progress_bar, BUSY_ANIM_PATH("A_progress_bar_busy_71x1.anim"));

        } else if(data->timer_state == BusyTimerStateRest) {
            anim_image_set_source(data->state_image, BUSY_ANIM_PATH("A_rest_label_40x14.anim"));
            anim_image_start(data->state_image);
            progress_bar_set_trough_color(data->progress_bar, PROGRESS_BAR_COLOR_REST);
            progress_bar_set_anim_source(
                data->progress_bar, BUSY_ANIM_PATH("A_progress_bar_rest_71x1.anim"));
        }
    });
}

static void busy_scene_timer_toggle_pause(BusyApp* instance) {
    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);
    data->is_paused = !data->is_paused;

    with_gui(instance->gui, {
        pause_overlay_show(data->pause_overlay, data->is_paused);
        timer_card_show_header(data->timer_card, !data->is_paused);

        if(data->is_paused) {
            anim_image_stop(data->state_image);
        } else {
            anim_image_start(data->state_image);
        }
    });
}

static void busy_scene_timer_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_scene_timer_input_callback, instance);

        data->state_image = anim_image_alloc(instance->front_window);

        data->timer_label = timer_label_alloc(instance->front_window);
        widget_set_pos(timer_label_get_base(data->timer_label), 42, 1);

        data->progress_bar = progress_bar_alloc(instance->front_window);
        widget_set_pos(progress_bar_get_base(data->progress_bar), 1, 15);

        data->pause_overlay = pause_overlay_alloc(instance->front_window);

        data->timer_card = timer_card_alloc(instance->back_window);
        widget_set_pos(timer_card_get_base(data->timer_card), 0, 4);
    });

    busy_timer_set_callback(instance->busy_timer, busy_scene_timer_event_callback, instance);
    busy_timer_start(instance->busy_timer);
}

static void busy_scene_timer_on_exit(void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_scene_timer_input_callback);

        anim_image_free(data->state_image);
        timer_label_free(data->timer_label);
        progress_bar_free(data->progress_bar);
        pause_overlay_free(data->pause_overlay);
        timer_card_free(data->timer_card);
    });

    busy_timer_set_callback(instance->busy_timer, NULL, NULL);
}

static bool busy_scene_timer_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusyCustomEventTimerTick) {
            busy_scene_timer_update_tick(instance);

        } else if(event->event == BusyCustomEventTimerStateChanged) {
            busy_scene_timer_update_state(instance);

        } else if(event->event == BusyCustomEventTimerToggle) {
            busy_scene_timer_toggle_pause(instance);
            busy_timer_toggle(instance->busy_timer);

        } else if(event->event == BusyCustomEventTimerSkip) {
            busy_timer_skip(instance->busy_timer);

        } else if(event->event == BusyCustomEventTimeIncrement) {
            busy_timer_add_time(instance->busy_timer, BUSY_TIMER_TIME_INCREMENT_MN);

        } else if(event->event == BusyCustomEventTimeDecrement) {
            busy_timer_add_time(instance->busy_timer, -BUSY_TIMER_TIME_INCREMENT_MN);
        }

        consumed = true;
    }

    return consumed;
}

const Scene busy_scene_timer = {
    .enter_callback = busy_scene_timer_on_enter,
    .exit_callback = busy_scene_timer_on_exit,
    .event_callback = busy_scene_timer_on_event,
    .data_size = sizeof(BusySceneTimer),
};
