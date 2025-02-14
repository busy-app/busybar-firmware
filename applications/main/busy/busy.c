#include "busy_i.h"

#include <input/input.h>

#include "scenes/busy_scene_start.h"
#include "scenes/busy_scene_timer.h"
#include "scenes/busy_scene_static.h"
#include "scenes/busy_scene_quit.h"
#include "scenes/busy_scene_next.h"
#include "scenes/busy_scene_setup.h"

#define TOTAL_TIME_DEFAULT_MN      (HM_TO_M(1, 35))
#define WORK_TIME_DEFAULT_MN       (45)
#define SHORT_REST_TIME_DEFAULT_MN (10)
#define LONG_REST_TIME_DEFAULT_MN  (20)

#define ENABLE_INTERVALS_DEFAULT      (true)
// #define ENABLE_AUTOSTART_WORK_DEFAULT (true)
// #define ENABLE_AUTOSTART_REST_DEFAULT (true)
#define ENABLE_AUTOSTART_WORK_DEFAULT (false)
#define ENABLE_AUTOSTART_REST_DEFAULT (false)
#define ENABLE_AUTORESTART_SESSION    (false)
#define ENABLE_SOUND_DEFAULT          (false)
#define ENABLE_SPEED_DEFAULT          (false)

#define CYCLE_COUNT_DEFAULT (3)
#define SPEED_MULTIPLIER    (50)

#define S_TO_MS(s) (s * 1000)

static const BusyAppScene* busy_scenes[BusyAppSceneIdMax] = {
    [BusyAppSceneIdStart] = &busy_scene_start,
    [BusyAppSceneIdTimer] = &busy_scene_timer,
    [BusyAppSceneIdStatic] = &busy_scene_static,
    [BusyAppSceneIdQuit] = &busy_scene_quit,
    [BusyAppSceneIdNext] = &busy_scene_next,
    [BusyAppSceneIdSetup] = &busy_scene_setup,
};

static void busy_send_custom_event_direct(BusyApp* instance, uint32_t value) {
    if(instance->current_scene_id < BusyAppSceneIdMax) {
        const BusyEvent event = {
            .type = BusyEventTypeCustom,
            .custom_value = value,
        };

        busy_scenes[instance->current_scene_id]->on_event(&event, instance);
    }
}

void busy_switch_to_scene(BusyApp* instance, BusyAppSceneId scene_id) {
    if(instance->current_scene_id < BusyAppSceneIdMax) {
        busy_scenes[instance->current_scene_id]->on_exit(instance);

        if(instance->scene_data[instance->current_scene_id]) {
            free(instance->scene_data[instance->current_scene_id]);
            instance->scene_data[instance->current_scene_id] = NULL;
        }
    }

    instance->current_scene_id = scene_id;

    if(busy_scenes[instance->current_scene_id]->data_size) {
        instance->scene_data[instance->current_scene_id] =
            malloc(busy_scenes[instance->current_scene_id]->data_size);
    }

    busy_scenes[instance->current_scene_id]->on_enter(instance);
}

void busy_send_custom_event(BusyApp* instance, uint32_t value) {
    const BusyEvent event = {
        .type = BusyEventTypeCustom,
        .custom_value = value,
    };

    furi_check(
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever) == FuriStatusOk);
}

void* busy_get_current_scene_data(BusyApp* instance) {
    furi_assert(instance->current_scene_id < BusyAppSceneIdMax);
    return instance->scene_data[instance->current_scene_id];
}

void busy_timer_start(BusyApp* instance) {
    instance->state = BusyTimerStateIdle;
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
    const uint32_t timeout_ms = instance->enable_speed ? S_TO_MS(1) / SPEED_MULTIPLIER :
                                                         S_TO_MS(1);
    furi_event_loop_timer_start(instance->busy_timer, timeout_ms);
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
    BusyTimerState next_state;

    if(instance->state == BusyTimerStateIdle) {
        instance->intervals_done = 0;
        next_state = BusyTimerStateBusy;
    } else if(instance->state == BusyTimerStateBusy) {
        if(instance->enable_intervals) {
            if(instance->intervals_done == instance->intervals_total - 1) {
                next_state = BusyTimerStateLongRest;
            } else {
                next_state = BusyTimerStateRest;
            }
        } else {
            next_state = BusyTimerStateBusy;
        }
    } else if(instance->state == BusyTimerStateRest || instance->state == BusyTimerStateLongRest) {
        if(++instance->intervals_done == instance->intervals_total) {
            instance->intervals_done = 0;
        }
        next_state = BusyTimerStateBusy;
    } else {
        furi_crash("Impossibru!");
    }

    if(next_state == BusyTimerStateBusy) {
        if(instance->enable_intervals) {
            instance->interval_time_s = M_TO_S(instance->work_time_mn);
        } else {
            instance->interval_time_s = M_TO_S(instance->total_time_mn);
        }
    } else if(next_state == BusyTimerStateRest) {
        instance->interval_time_s = M_TO_S(instance->short_rest_time_mn);
    } else if(next_state == BusyTimerStateLongRest) {
        instance->interval_time_s = M_TO_S(instance->long_rest_time_mn);
    } else {
        furi_crash("Impossibru!");
    }

    instance->state = next_state;
    instance->interval_time_left_s = instance->interval_time_s;

    const uint32_t timeout_ms = instance->enable_speed ? S_TO_MS(1) / SPEED_MULTIPLIER :
                                                         S_TO_MS(1);
    furi_event_loop_timer_start(instance->busy_timer, timeout_ms);

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

    if(instance->current_scene_id < BusyAppSceneIdMax) {
        busy_scenes[instance->current_scene_id]->on_event(&event, instance);
    }
}

static void busy_scene_busy_timer_callback(void* context) {
    BusyApp* instance = context;

    if(--instance->interval_time_left_s > 0) {
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

    instance->current_scene_id = BusyAppSceneIdMax;

    instance->total_time_mn = TOTAL_TIME_DEFAULT_MN;
    instance->work_time_mn = WORK_TIME_DEFAULT_MN;
    instance->short_rest_time_mn = SHORT_REST_TIME_DEFAULT_MN;
    instance->long_rest_time_mn = LONG_REST_TIME_DEFAULT_MN;
    instance->intervals_total = CYCLE_COUNT_DEFAULT;
    instance->enable_intervals = ENABLE_INTERVALS_DEFAULT;
    instance->enable_autostart_work = ENABLE_AUTOSTART_WORK_DEFAULT;
    instance->enable_autostart_rest = ENABLE_AUTOSTART_REST_DEFAULT;
    instance->enable_sound = ENABLE_SOUND_DEFAULT;
    instance->enable_speed = ENABLE_SPEED_DEFAULT;

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        busy_event_queue_callback,
        instance);

    // Create a single label for the back display
    gui_lvgl_acquire(instance->gui);

    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdBack, GuiLayerIdActive);
    instance->back_label = lv_label_create(active);
    lv_obj_center(instance->back_label);
    lv_obj_set_style_text_color(instance->back_label, lv_color_white(), LV_PART_MAIN);

    gui_lvgl_release(instance->gui);

    busy_switch_to_scene(instance, BusyAppSceneIdStart);

    return instance;
}

static void busy_free(BusyApp* instance) {
    if(instance->current_scene_id < BusyAppSceneIdMax) {
        busy_scenes[instance->current_scene_id]->on_exit(instance);

        if(instance->scene_data[instance->current_scene_id]) {
            free(instance->scene_data[instance->current_scene_id]);
        }
    }

    // Delete the back display label
    gui_lvgl_acquire(instance->gui);
    lv_obj_delete(instance->back_label);
    gui_lvgl_release(instance->gui);

    furi_record_close(RECORD_GUI_LVGL);
    furi_record_close(RECORD_INPUT_EVENTS);

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
