#include "busy_i.h"

#include <input/input.h>

#include "scenes/busy_scene_start.h"
#include "scenes/busy_scene_timer.h"
#include "scenes/busy_scene_static.h"
#include "scenes/busy_scene_quit.h"

#define BUSY_INTERVAL_DEFAULT_S      (15 * 60)
// #define BUSY_INTERVAL_DEFAULT_S      (BUSY_INTERVAL_INFINITE)
#define REST_INTERVAL_DEFAULT_S      (5 * 60)
#define LONG_REST_INTERVAL_DEFAULT_S (10 * 60)

#define CYCLE_COUNT_DEFAULT (3)

#define SECONDS_TO_MS(s) (s * 1000)

static const BusyAppScene* busy_scenes[BusyAppSceneIdMax] = {
    [BusyAppSceneIdStart] = &busy_scene_start,
    [BusyAppSceneIdTimer] = &busy_scene_timer,
    [BusyAppSceneIdStatic] = &busy_scene_static,
    [BusyAppSceneIdQuit] = &busy_scene_quit,
};

static void busy_send_custom_event_direct(BusyApp* instance, uint32_t value) {
    if(instance->current_scene) {
        const BusyEvent event = {
            .type = BusyEventTypeCustom,
            .custom_value = value,
        };

        instance->current_scene->on_event(&event, instance);
    }
}

void busy_switch_to_scene(BusyApp* instance, BusyAppSceneId scene_id) {
    if(instance->current_scene) {
        instance->current_scene->on_exit(instance);
    }

    instance->current_scene = busy_scenes[scene_id];
    instance->current_scene->on_enter(instance);
}

void busy_send_custom_event(BusyApp* instance, uint32_t value) {
    const BusyEvent event = {
        .type = BusyEventTypeCustom,
        .custom_value = value,
    };

    furi_check(
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever) == FuriStatusOk);
}

void busy_timer_start(BusyApp* instance) {
    instance->state = BusyTimerStateIdle;
    instance->cycles_left = instance->cycles_count;
    busy_timer_next_state(instance);
}

void busy_timer_stop(BusyApp* instance) {
    furi_event_loop_timer_stop(instance->busy_timer);
    instance->state = BusyTimerStateIdle;
}

void busy_timer_pause(BusyApp* instance) {
    furi_assert(busy_timer_is_running(instance));
    furi_event_loop_timer_stop(instance->busy_timer);
}

void busy_timer_resume(BusyApp* instance) {
    furi_assert(!busy_timer_is_running(instance));
    furi_event_loop_timer_start(instance->busy_timer, SECONDS_TO_MS(1));
    busy_send_custom_event_direct(instance, instance->state);
}

void busy_timer_toggle(BusyApp* instance) {
    if(busy_timer_is_running(instance)) {
        busy_timer_pause(instance);
    } else {
        busy_timer_resume(instance);
    }
}

bool busy_timer_is_running(BusyApp* instance) {
    return furi_event_loop_timer_is_running(instance->busy_timer);
}

void busy_timer_next_state(BusyApp* instance) {
    BusyTimerState new_state;

    if(instance->state == BusyTimerStateIdle) {
        new_state = BusyTimerStateBusy;
    } else if(instance->state == BusyTimerStateBusy) {
        if(--instance->cycles_left == 0) {
            new_state = BusyTimerStateLongRest;
        } else {
            new_state = BusyTimerStateRest;
        }
    } else if(instance->state == BusyTimerStateRest || instance->state == BusyTimerStateLongRest) {
        new_state = BusyTimerStateBusy;
    } else {
        furi_crash("Impossibru!");
    }

    if(new_state == BusyTimerStateBusy) {
        instance->time_total = instance->busy_interval_s;
    } else if(new_state == BusyTimerStateRest) {
        instance->time_total = instance->rest_interval_s;
    } else if(new_state == BusyTimerStateLongRest) {
        instance->time_total = instance->long_rest_interval_s;
        instance->cycles_left = instance->cycles_count;
    } else {
        furi_crash("Impossibru!");
    }

    instance->state = new_state;
    instance->time_left = instance->time_total;

    furi_event_loop_timer_start(instance->busy_timer, SECONDS_TO_MS(1));

    busy_send_custom_event_direct(instance, BusyCustomEventUpdate);
}

static void busy_input_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    BusyApp* instance = context;
    const InputEvent* event = message;

    if(event->type == InputTypeShort) {
        BusyEventType busy_event_type;

        if(event->key == InputKeyStart) {
            busy_event_type = BusyEventTypeStart;
        } else if(event->key == InputKeyBack) {
            busy_event_type = BusyEventTypeBack;
        } else if(event->key == InputKeyOk) {
            busy_event_type = BusyEventTypeOk;
        } else {
            return;
        }

        const BusyEvent busy_event = {
            .type = busy_event_type,
        };

        furi_check(
            furi_message_queue_put(instance->event_queue, &busy_event, FuriWaitForever) ==
            FuriStatusOk);
    }
}

static void busy_event_queue_callback(FuriEventLoopObject* object, void* context) {
    BusyApp* instance = context;
    furi_check(object == instance->event_queue);

    BusyEvent event;
    furi_check(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk);

    if(instance->current_scene) {
        instance->current_scene->on_event(&event, instance);
    }
}

static void busy_scene_busy_timer_callback(void* context) {
    BusyApp* instance = context;

    if(--instance->time_left > 0) {
        busy_send_custom_event_direct(instance, BusyCustomEventUpdate);
    } else {
        busy_timer_next_state(instance);
    }
}

static BusyApp* busy_alloc(void) {
    BusyApp* instance = malloc(sizeof(BusyApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_queue = furi_message_queue_alloc(16, sizeof(BusyEvent));
    instance->busy_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        busy_scene_busy_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);
    instance->gui = furi_record_open(RECORD_GUI_LVGL);

    FuriPubSub* input = furi_record_open(RECORD_INPUT_EVENTS);
    instance->input_events = furi_pubsub_subscribe(input, busy_input_callback, instance);

    instance->busy_interval_s = BUSY_INTERVAL_DEFAULT_S;
    instance->rest_interval_s = REST_INTERVAL_DEFAULT_S;
    instance->long_rest_interval_s = LONG_REST_INTERVAL_DEFAULT_S;
    instance->cycles_count = CYCLE_COUNT_DEFAULT;

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        busy_event_queue_callback,
        instance);

    busy_switch_to_scene(instance, BusyAppSceneIdStart);

    return instance;
}

static void busy_free(BusyApp* instance) {
    if(instance->current_scene) {
        instance->current_scene->on_exit(instance);
        instance->current_scene = NULL;
    }

    furi_record_close(RECORD_GUI_LVGL);

    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_event_loop_timer_free(instance->busy_timer);
    furi_message_queue_free(instance->event_queue);
    furi_event_loop_free(instance->event_loop);

    free(instance);
}

int32_t busy_app(void* arg) {
    UNUSED(arg);

    BusyApp* instance = busy_alloc();
    furi_event_loop_run(instance->event_loop);
    busy_free(instance);

    return 0;
}
