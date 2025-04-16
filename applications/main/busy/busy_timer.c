#include "busy_timer.h"

#include "time_macros.h"

#define TAG "BusyTimer"

#define TOTAL_TIME_DEFAULT_MN (HM_TO_M(1, 35))
#define WORK_TIME_DEFAULT_MN  (45)
#define REST_TIME_DEFAULT_MN  (10)

#define ENABLE_INTERVALS_DEFAULT      (true)
// #define ENABLE_AUTOSTART_WORK_DEFAULT (true)
// #define ENABLE_AUTOSTART_REST_DEFAULT (true)
#define ENABLE_AUTOSTART_WORK_DEFAULT (false)
#define ENABLE_AUTOSTART_REST_DEFAULT (false)
#define ENABLE_AUTORESTART_DEFAULT    (false)
#define ENABLE_SOUND_DEFAULT          (true)
#define ENABLE_SPEED_DEFAULT          (false)

#define CYCLE_COUNT_DEFAULT (3)
#define SPEED_MULTIPLIER    (60)

typedef enum {
    BusyTimerMessageTypeStart,
    BusyTimerMessageTypeStop,
} BusyTimerMessageType;

typedef struct {
    BusyTimerMessageType type;
} BusyTimerMessage;

struct BusyTimer {
    FuriThread* thread;
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    FuriMessageQueue* message_queue;
    uint32_t intervals_total;
    uint32_t intervals_done;
    uint32_t interval_time_s;
    uint32_t total_time_left_s;
    uint32_t interval_time_left_s;
    BusyTimerConfig config;
    BusyTimerState state;
};

static const BusyTimerConfig busy_timer_config_default = {
    .total_time_mn = TOTAL_TIME_DEFAULT_MN,
    .work_time_mn = WORK_TIME_DEFAULT_MN,
    .rest_time_mn = REST_TIME_DEFAULT_MN,
    .enable_intervals = ENABLE_INTERVALS_DEFAULT,
    .enable_autostart_work = ENABLE_AUTOSTART_WORK_DEFAULT,
    .enable_autostart_rest = ENABLE_AUTOSTART_REST_DEFAULT,
    .enable_autorestart = ENABLE_AUTORESTART_DEFAULT,
    .enable_sound = ENABLE_SOUND_DEFAULT,
    .enable_speed = ENABLE_SPEED_DEFAULT,
};

// void busy_timer_start(BusyApp* instance) {
//     FURI_LOG_I(TAG, "Timer starting");
//
//     instance->state = BusyTimerStateIdle;
//     busy_timer_next_state(instance, true);
//
//     FURI_LOG_I(TAG, "Timer started");
// }
//
// void busy_timer_stop(BusyApp* instance) {
//     FURI_LOG_I(TAG, "Timer stopping");
//
//     furi_event_loop_timer_stop(instance->busy_timer);
//     instance->state = BusyTimerStateIdle;
//
//     FURI_LOG_I(TAG, "Timer stopped");
// }
//
// void busy_timer_pause(BusyApp* instance) {
//     furi_event_loop_timer_stop(instance->busy_timer);
//
//     FURI_LOG_I(TAG, "Timer paused");
// }
//
// void busy_timer_resume(BusyApp* instance) {
//     const uint32_t timeout_ms = instance->enable_speed ? S_TO_MS(1) / SPEED_MULTIPLIER :
//                                                          S_TO_MS(1);
//     furi_event_loop_timer_start(instance->busy_timer, timeout_ms);
//     busy_send_custom_event(instance, instance->state);
//
//     FURI_LOG_I(TAG, "Timer resumed");
// }
//
// void busy_timer_toggle(BusyApp* instance) {
//     FURI_LOG_I(TAG, "Timer toggle");
//
//     if(busy_timer_is_running(instance)) {
//         busy_timer_pause(instance);
//     } else {
//         busy_timer_resume(instance);
//     }
// }
//
// bool busy_timer_is_running(BusyApp* instance) {
//     return furi_event_loop_timer_is_running(instance->busy_timer);
// }
//
// static const char* busy_timer_get_state_name(BusyTimerState state) {
//     furi_assert(state < BusyTimerStateMax);
//
//     static const char* state_names[BusyTimerStateMax] = {
//         [BusyTimerStateIdle] = "Idle",
//         [BusyTimerStateWork] = "Work",
//         [BusyTimerStateRest] = "Rest",
//         [BusyTimerStateLongRest] = "LongRest",
//     };
//
//     return state_names[state];
// }
//
// static BusyTimerState busy_timer_calc_state(BusyApp* instance) {
//     BusyTimerState state;
//
//     if(instance->state == BusyTimerStateIdle) {
//         state = BusyTimerStateWork;
//
//     } else if(instance->state == BusyTimerStateWork) {
//         if(instance->enable_intervals) {
//             if(instance->intervals_done == instance->intervals_total - 1) {
//                 state = BusyTimerStateLongRest;
//             } else {
//                 state = BusyTimerStateRest;
//             }
//
//         } else {
//             state = BusyTimerStateWork;
//         }
//
//     } else if(instance->state == BusyTimerStateRest || instance->state == BusyTimerStateLongRest) {
//         if(++instance->intervals_done == instance->intervals_total) {
//             instance->intervals_done = 0;
//         }
//
//         state = BusyTimerStateWork;
//
//     } else {
//         furi_crash("calc_state(): invalid state");
//     }
//
//     return state;
// }
//
// static uint32_t busy_timer_calc_interval(BusyApp* instance) {
//     uint32_t interval;
//
//     if(instance->state == BusyTimerStateWork) {
//         if(instance->enable_intervals) {
//             FURI_LOG_D(TAG, "Calculating work time based on intervals");
//             interval = MIN(M_TO_S(instance->work_time_mn), instance->total_time_left_s);
//         } else {
//             FURI_LOG_D(TAG, "Calculating work time based on total time");
//             interval = MIN(M_TO_S(instance->total_time_mn), instance->total_time_left_s);
//         }
//     } else if(instance->state == BusyTimerStateRest) {
//         interval = MIN(M_TO_S(instance->short_rest_time_mn), instance->total_time_left_s);
//     } else if(instance->state == BusyTimerStateLongRest) {
//         interval = MIN(M_TO_S(instance->long_rest_time_mn), instance->total_time_left_s);
//     } else {
//         furi_crash("calc_interval(): invalid state");
//     }
//
//     return interval;
// }
//
// static BusyCustomEvent busy_timer_calc_event(BusyApp* instance, bool skip_event) {
//     BusyCustomEvent event = BusyCustomEventUpdate;
//
//     if(!skip_event) {
//         if(instance->state == BusyTimerStateIdle) {
//             if(!instance->enable_autorestart_session) {
//                 event = BusyCustomEventSessionEnd;
//             } else {
//                 FURI_LOG_D(TAG, "Autorestart session OFF, skipping event");
//             }
//         } else if(instance->state == BusyTimerStateRest || instance->state == BusyTimerStateLongRest) {
//             if(!instance->enable_autostart_work) {
//                 event = BusyCustomEventIntervalEnd;
//             } else {
//                 FURI_LOG_D(TAG, "Autorestart work OFF, skipping event");
//             }
//         } else if(instance->state == BusyTimerStateWork) {
//             if(!instance->enable_autostart_rest) {
//                 event = BusyCustomEventIntervalEnd;
//             } else {
//                 FURI_LOG_D(TAG, "Autorestart rest OFF, skipping event");
//             }
//         }
//     } else {
//         FURI_LOG_D(TAG, "Skipping state change event");
//     }
//
//     return event;
// }
//
// static void busy_play_finished_sound(BusyApp* instance) {
//     if(instance->enable_sound) {
//         if(instance->state == BusyTimerStateWork) {
//             audio_play_file(instance->audio, EXT_PATH("audio/work_finished.snd"));
//         } else if(instance->state == BusyTimerStateRest || instance->state == BusyTimerStateLongRest) {
//             audio_play_file(instance->audio, EXT_PATH("audio/rest_finished.snd"));
//         }
//     }
// }
//
// static void busy_play_countdown_sound(BusyApp* instance) {
//     if(instance->enable_sound && instance->interval_time_left_s <= 4) {
//         if(instance->state == BusyTimerStateWork) {
//             audio_play_file(instance->audio, EXT_PATH("audio/work_countdown.snd"));
//         } else if(instance->state == BusyTimerStateRest || instance->state == BusyTimerStateLongRest) {
//             audio_play_file(instance->audio, EXT_PATH("audio/rest_countdown.snd"));
//         }
//     }
// }
//
// void busy_timer_next_state(BusyApp* instance, bool skip_event) {
//     FURI_LOG_I(TAG, "Current timer state: %s", busy_timer_get_state_name(instance->state));
//
//     busy_play_finished_sound(instance);
//
//     if(instance->total_time_left_s == 0 && instance->state != BusyTimerStateIdle) {
//         instance->state = BusyTimerStateIdle;
//         FURI_LOG_I(TAG, "Time is up, restarting the timer");
//     }
//
//     if(instance->state == BusyTimerStateIdle) {
//         instance->total_time_left_s = M_TO_S(instance->total_time_mn);
//         instance->intervals_done = 0;
//         FURI_LOG_I(TAG, "New session start with total time of %lu min", instance->total_time_mn);
//     }
//
//     const BusyCustomEvent event = busy_timer_calc_event(instance, skip_event);
//
//     instance->state = busy_timer_calc_state(instance);
//     instance->interval_time_s = busy_timer_calc_interval(instance);
//     instance->interval_time_left_s = instance->interval_time_s;
//     instance->total_time_left_s -= instance->interval_time_s;
//
//     FURI_LOG_I(TAG, "New timer state: %s", busy_timer_get_state_name(instance->state));
//     FURI_LOG_I(TAG, "New interval time: %lu min", S_TO_M(instance->interval_time_s));
//     FURI_LOG_I(TAG, "Remaining time: %lu min", S_TO_M(instance->total_time_left_s));
//
//     const uint32_t timeout_ms = instance->enable_speed ? S_TO_MS(1) / SPEED_MULTIPLIER :
//                                                          S_TO_MS(1);
//
//     furi_event_loop_timer_start(instance->busy_timer, timeout_ms);
//     busy_send_custom_event(instance, event);
// }

static void busy_timer_callback(void* context) {
    furi_assert(context);
    BusyTimer* instance = context;

    instance->interval_time_left_s -= 1;

    if(instance->interval_time_left_s) {
        //     busy_send_custom_event(instance, BusyCustomEventUpdate);
        //     busy_play_countdown_sound(instance);
    } else {
        //     busy_timer_next_state(instance, false);
    }
}

static void busy_timer_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    BusyTimer* instance = context;
    furi_assert(instance->message_queue == object);

    BusyTimerMessage message;
    while(furi_message_queue_get(instance->message_queue, &message, 0) == FuriStatusOk) {
        // TODO: Handle request
    }
}

static int32_t busy_timer_thread(void* arg) {
    furi_assert(arg);
    BusyTimer* instance = arg;

    instance->event_loop = furi_event_loop_alloc();
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop, busy_timer_callback, FuriEventLoopTimerTypePeriodic, instance);
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        busy_timer_message_queue_callback,
        instance);

    furi_event_loop_run(instance->event_loop);

    furi_event_loop_unsubscribe(instance->event_loop, instance->message_queue);
    furi_event_loop_timer_free(instance->timer);
    furi_event_loop_free(instance->event_loop);

    return 0;
}

static void busy_timer_send_message(BusyTimer* instance, const BusyTimerMessage* message) {
    UNUSED(instance);
    UNUSED(message);
}

// Public API

BusyTimer* busy_timer_alloc(void) {
    BusyTimer* instance = malloc(sizeof(BusyTimer));

    instance->thread = furi_thread_alloc_ex(TAG, 1024, busy_timer_thread, instance);
    instance->message_queue = furi_message_queue_alloc(1, sizeof(BusyTimerMessage));
    instance->config = busy_timer_config_default;

    furi_thread_start(instance->thread);
    return instance;
}

void busy_timer_free(BusyTimer* instance) {
    furi_assert(instance);

    furi_event_loop_stop(instance->event_loop);
    furi_thread_join(instance->thread);
    furi_thread_free(instance->thread);

    free(instance);
}

BusyTimerState busy_timer_get_state(const BusyTimer* instance) {
    furi_assert(instance);
    // TODO: Use message queue
    UNUSED(busy_timer_send_message);
    return instance->state;
}
