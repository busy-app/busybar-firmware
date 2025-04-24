#include "../busy.h"

#include "../widgets/timer_card.h"
#include "../widgets/timer_label.h"

typedef struct {
    TimerLabel* timer_label;
    TimerCard* timer_card;
    uint32_t timer_time_s;
    BusyTimerState timer_state;
} BusySceneTimer;

static void busy_scene_timer_event_callback(const BusyTimerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    if(event->type == BusyTimerEventTypeTick) {
        data->timer_time_s = event->time_s;
        busy_send_custom_event(instance, BusyCustomEventTimerTick);

    } else if(event->type == BusyTimerEventTypeStateChanged) {
        data->timer_state = event->state;
        busy_send_custom_event(instance, BusyCustomEventTimerStateChanged);
    }
}

static void busy_scene_timer_update(BusySceneTimer* data) {
    timer_label_set_time_left(data->timer_label, data->timer_time_s);
    timer_card_set_time_left(data->timer_card, data->timer_time_s);
}

static void busy_scene_timer_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        data->timer_label = timer_label_alloc(instance->front_window);
        widget_set_pos(timer_label_get_base(data->timer_label), 42, 1);

        data->timer_card = timer_card_alloc(instance->back_window);
        widget_set_pos(timer_card_get_base(data->timer_card), 2, 4);
    });

    busy_timer_set_callback(instance->busy_timer, busy_scene_timer_event_callback, instance);
    busy_timer_start(instance->busy_timer);
}

static void busy_scene_timer_on_exit(void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        timer_label_free(data->timer_label);
        timer_card_free(data->timer_card);
    });

    busy_timer_set_callback(instance->busy_timer, NULL, NULL);
}

static bool busy_scene_timer_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusyCustomEventTimerTick) {
            with_gui(instance->gui, { busy_scene_timer_update(data); });

        } else if(event->event == BusyCustomEventTimerStateChanged) {
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        // TODO: Ask to exit?
    }

    return consumed;
}

const Scene busy_scene_timer = {
    .enter_callback = busy_scene_timer_on_enter,
    .exit_callback = busy_scene_timer_on_exit,
    .event_callback = busy_scene_timer_on_event,
    .data_size = sizeof(BusySceneTimer),
};
