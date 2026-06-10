#include "state_publisher.h"
#include "state_publisher_i.h"
#include <furi/furi.h>

#include <nanopb/pb.h>
#include <nanopb/pb_encode.h>
#include <state.pb.h>

#include <time/time.h>

#define MAX_CONTROL_MESSAGES             64
#define MAX_PUBLISH_MESSAGES             16
#define CONTROL_MESSAGE_QUEUE_TIMEOUT_MS 3000

#define HEARTBEAT_INTERVAL_MS 991

typedef bool (*MessageHandler)(StatePublisher* instance, const ControlMessage* message);

static const MessageHandler message_handlers[];

static void heartbeat_timer_callback(void* context);
static void rate_limiter_timer_callback(void* context);
static int32_t fetch_thread_callback(void* context);

void screen_streamer_callback(
    GuiDisplayId display,
    const ScreenStreamerFrame* frame,
    uint8_t stream_flags,
    void* context);

static void publish_queue_callback(FuriEventLoopObject* object, void* context);

static void control_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    StatePublisher* instance = context;

    ControlMessage message;
    furi_check(furi_message_queue_get(instance->control_queue, &message, 0) == FuriStatusOk);

    message_handlers[message.type](instance, &message);
}

void state_publisher_send_control_message(StatePublisher* instance, const ControlMessage* message) {
    if(furi_thread_get_current_id() == instance->main_thread_id) {
        message_handlers[message->type](instance, message);
        return;
    }

    FuriStatus queue_status = furi_message_queue_put(
        instance->control_queue, message, furi_ms_to_ticks(CONTROL_MESSAGE_QUEUE_TIMEOUT_MS));

    if(queue_status != FuriStatusOk) {
        FURI_LOG_E(TAG, "Failed to put an item into message queue.");
    }
}

static uint32_t ms_to_ticks(uint32_t ms) {
    if(ms == UINT32_MAX) {
        return UINT32_MAX;
    } else {
        return furi_ms_to_ticks(ms);
    }
}

static StatePublisher* state_publisher_alloc(void) {
    StatePublisher* instance = malloc(sizeof(StatePublisher));

    instance->event_loop = furi_event_loop_alloc();
    instance->control_queue =
        furi_message_queue_alloc(MAX_CONTROL_MESSAGES, sizeof(ControlMessage));
    instance->publish_queue =
        furi_message_queue_alloc(MAX_PUBLISH_MESSAGES, sizeof(PublishMessage));
    instance->main_thread_id = furi_thread_get_current_id();

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->control_queue,
        FuriEventLoopEventIn,
        control_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->publish_queue,
        FuriEventLoopEventIn,
        publish_queue_callback,
        instance);

    instance->heartbeat_timer = furi_event_loop_timer_alloc(
        instance->event_loop, heartbeat_timer_callback, FuriEventLoopTimerTypePeriodic, instance);

    instance->rate_limiter_timer = furi_event_loop_timer_alloc(
        instance->event_loop, rate_limiter_timer_callback, FuriEventLoopTimerTypeOnce, instance);

    instance->gui = furi_record_open(RECORD_GUI);

    instance->screen_streamer_front = screen_streamer_alloc(
        GuiDisplayIdFront, instance->gui, screen_streamer_callback, instance);

    instance->transports_mutex = furi_mutex_alloc(FuriMutexTypeRecursive);
    for(uint32_t i = 0; i != COUNT_OF(instance->transports); ++i) {
        Transport* t = instance->transports + i;
        t->valid = false;
        SharedStateUpdateArray_init(t->seq_updates);
        SharedStateUpdateArray_init(t->state_updates);
    }

    instance->fetch_flags = furi_event_flag_alloc();
    instance->fetch_thread =
        furi_thread_alloc_ex("StatePublisherFetch", 16 * 1024, fetch_thread_callback, instance);
    furi_thread_start(instance->fetch_thread);

    state_publisher_subscribe(instance);

    screen_streamer_start(instance->screen_streamer_front);

    furi_event_loop_timer_start(
        instance->heartbeat_timer, furi_ms_to_ticks(HEARTBEAT_INTERVAL_MS));

    furi_record_create(RECORD_STATE_PUBLISHER, instance);

    return instance;
}

static void update_screen_streamer_outputs(StatePublisher* instance) {
    for(StatePublisherTransportClass transport_class = 0;
        transport_class != StatePublisherTransportClassMax;
        ++transport_class) {
        bool enabled = false;
        uint32_t frame_interval_ms = UINT32_MAX;
        for(size_t i = 0; i != MAX_TRANSPORTS; ++i) {
            Transport* t = instance->transports + i;
            if(t->valid && (t->flags & (1 << transport_class))) {
                enabled = true;
                frame_interval_ms = MIN(frame_interval_ms, t->frame_interval_ms);
            }
        }
        if(enabled) {
            screen_streamer_enable_output(
                instance->screen_streamer_front, transport_class, frame_interval_ms);
        } else {
            screen_streamer_disable_output(instance->screen_streamer_front, transport_class);
        }
    }
}

StatePublisherTransportHandle state_publisher_add_transport(
    StatePublisher* instance,
    StatePublisherTransportClass transport_class,
    uint32_t frame_interval_ms,
    RateLimiterLimit rate_limit,
    StatePublisherPublishCb cb,
    void* context) {
    size_t i = 0;
    furi_mutex_acquire(instance->transports_mutex, FuriWaitForever);
    for(; i != MAX_TRANSPORTS; ++i) {
        Transport* t = instance->transports + i;
        if(!t->valid) {
            t->valid = true;
            t->flags = 1 << transport_class;
            t->frame_interval_ms = frame_interval_ms;
            t->cb = cb;
            t->cb_context = context;
            t->limiter = rate_limiter_init(rate_limit);
            break;
        }
    }
    update_screen_streamer_outputs(instance);
    furi_mutex_release(instance->transports_mutex);
    furi_check(i < MAX_TRANSPORTS);
    return i;
}

void state_publisher_del_transport(StatePublisher* instance, StatePublisherTransportHandle handle) {
    furi_mutex_acquire(instance->transports_mutex, FuriWaitForever);
    instance->transports[handle].valid = false;
    update_screen_streamer_outputs(instance);
    furi_mutex_release(instance->transports_mutex);
}

void state_publisher_set_rate_limit(
    StatePublisher* instance,
    StatePublisherTransportHandle transport,
    RateLimiterLimit rate_limit) {
    furi_mutex_acquire(instance->transports_mutex, FuriWaitForever);
    rate_limiter_set_limit(&instance->transports[transport].limiter, rate_limit);
    furi_mutex_release(instance->transports_mutex);
}

void state_publisher_set_paused(
    StatePublisher* instance,
    StatePublisherTransportHandle transport,
    bool paused) {
    furi_mutex_acquire(instance->transports_mutex, FuriWaitForever);
    bool was_paused = rate_limiter_set_paused(&instance->transports[transport].limiter, paused);
    furi_mutex_release(instance->transports_mutex);

    if(!paused && was_paused) {
        ControlMessage msg = {
            .type = MessageTypeTransportResumed,
        };
        state_publisher_send_control_message(instance, &msg);
    }
}

int32_t state_publisher_srv(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Service starting...");

    StatePublisher* instance = state_publisher_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

static bool ostream_cb(pb_ostream_t* stream, const pb_byte_t* data, size_t count) {
    ByteArray_t* buf = stream->state;
    size_t old_size = ByteArray_size(*buf);
    ByteArray_resize(*buf, old_size + count);
    memcpy(ByteArray_get(*buf, old_size), data, count);
    return true;
}

static pb_ostream_t ostream_with_buffer(ByteArray_t* buf) {
    return (pb_ostream_t){
        .callback = ostream_cb,
        .bytes_written = 0,
        .errmsg = NULL,
        .max_size = SIZE_MAX,
        .state = buf};
}

void state_publisher_free_state_update(BSB_State_StateUpdate* update) {
    switch(update->which_state) {
    case BSB_State_StateUpdate_timer_tag:
        free(update->state.timer.json.data);
        break;
    case BSB_State_StateUpdate_frame_tag:
        free(update->state.frame.data);
        break;
    case BSB_State_StateUpdate_timer_profiles_tag:
        for(size_t i = 0; i != update->state.timer_profiles.profiles_count; ++i) {
            free(update->state.timer_profiles.profiles[i].json.data);
        }
        free(update->state.timer_profiles.profiles);
    default:
        break;
    }
}

static bool is_sequential_update(const BSB_State_StateUpdate* update) {
    if(update) {
        switch(update->which_state) {
        case BSB_State_StateUpdate_input_tag:
            return true;
        default:
            return false;
        }
    } else {
        return false;
    }
}

static bool send_out_for_transport(void* context, bool heartbeat) {
    Transport* t = context;
    SharedStateUpdateArray_t updates;
    SharedStateUpdateArray_init_move(updates, t->seq_updates);
    SharedStateUpdateArray_init(t->seq_updates);

    for(size_t i = 0; i != SharedStateUpdateArray_size(t->state_updates); ++i) {
        SharedStateUpdate_t* shared = SharedStateUpdateArray_get(t->state_updates, i);
        if(!SharedStateUpdate_NULL_p(*shared)) {
            SharedStateUpdateArray_push_move(updates, shared);
        }
    }

    size_t count = SharedStateUpdateArray_size(updates);

    bool sent = false;

    if(count > 0 || heartbeat) {
        // shallow copy
        BSB_State_StateUpdate* raw_updates =
            count ? malloc(sizeof(BSB_State_StateUpdate) * count) : NULL;
        for(size_t i = 0; i != count; ++i) {
            SharedStateUpdate_t* shared = SharedStateUpdateArray_get(updates, i);
            memcpy(
                raw_updates + i, SharedStateUpdate_cref(*shared), sizeof(BSB_State_StateUpdate));
        }

        BSB_State_State state = {
            .timestamp = time_get_timestamp_ms(),
            .updates_count = count,
            .updates = raw_updates,
            .has_error = false,
        };

        SharedByteArray_t data;
        SharedByteArray_init_new(data);

        ByteArray_t* buf = SharedByteArray_ref(data);
        pb_ostream_t stream = ostream_with_buffer(buf);

        bool result = pb_encode(&stream, BSB_State_State_fields, &state);
        if(!result) {
            FURI_LOG_E(TAG, "cannot encode");
        } else {
            t->cb(data, t->cb_context);
        }

        free(raw_updates);
        SharedByteArray_clear(data);

        sent = true;
    }
    SharedStateUpdateArray_clear(updates);
    return sent;
}

bool state_publisher_serialize_error_message(
    ByteArray_t* buf,
    BSB_Error_Severity severity,
    BSB_Error_Cause cause) {
    BSB_State_State state = {
        .timestamp = time_get_timestamp_ms(),
        .updates_count = 0,
        .updates = NULL,
        .has_error = true,
        .error = {
            .cause = cause,
            .severity = severity,
        }};
    pb_ostream_t stream = ostream_with_buffer(buf);
    bool result = pb_encode(&stream, BSB_State_State_fields, &state);
    if(!result) {
        FURI_LOG_E(TAG, "cannot encode");
        ByteArray_reset(*buf);
    }
    return result;
}

static uint32_t send_out(StatePublisher* instance, StreamFlag flags, bool heartbeat) {
    uint32_t min_sleep_time_ms = UINT32_MAX;
    time_t now = time_get_timestamp_ms();
    furi_mutex_acquire(instance->transports_mutex, FuriWaitForever);
    for(size_t i = 0; i != MAX_TRANSPORTS; ++i) {
        Transport* t = instance->transports + i;
        if(t->valid && (t->flags & flags)) {
            uint32_t sleep_time_ms = rate_limiter_send_if_allowed(
                &t->limiter, now, send_out_for_transport, t, heartbeat);
            min_sleep_time_ms = MIN(min_sleep_time_ms, sleep_time_ms);
        }
    }
    furi_mutex_release(instance->transports_mutex);
    return min_sleep_time_ms;
}

void state_publisher_store_state_update(
    StatePublisher* instance,
    BSB_State_StateUpdate* update,
    StreamFlag flags) {
    SharedStateUpdate_t shared_update;
    SharedStateUpdate_init2(shared_update, update);
    if(is_sequential_update(update)) {
        furi_mutex_acquire(instance->transports_mutex, FuriWaitForever);
        for(size_t i = 0; i != MAX_TRANSPORTS; ++i) {
            Transport* t = instance->transports + i;
            if(t->valid && (t->flags & flags)) {
                if(SharedStateUpdateArray_size(t->seq_updates) >= MAX_SEQ_UPDATES) {
                    FURI_LOG_W(TAG, "Sequential updates full, dropping");
                    SharedStateUpdateArray_reset(t->seq_updates);
                }
                SharedStateUpdateArray_push_back(t->seq_updates, shared_update);
            }
        }
        furi_mutex_release(instance->transports_mutex);
    } else {
        furi_mutex_acquire(instance->transports_mutex, FuriWaitForever);
        for(size_t i = 0; i != MAX_TRANSPORTS; ++i) {
            Transport* t = instance->transports + i;
            if(t->valid && (t->flags & flags)) {
                size_t old_size = SharedStateUpdateArray_size(t->state_updates);
                SharedStateUpdateArray_resize(
                    t->state_updates, MAX(old_size, (size_t)update->which_state + 1));

                SharedStateUpdate_t* cell =
                    SharedStateUpdateArray_get(t->state_updates, update->which_state);
                SharedStateUpdate_set(*cell, shared_update);
            }
        }
        furi_mutex_release(instance->transports_mutex);
    }
    SharedStateUpdate_clear(shared_update);
}

static void publish_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);
    StatePublisher* instance = context;
    PublishMessage message;
    StreamFlag flags = 0;
    bool heartbeat = false;
    while(furi_message_queue_get(instance->publish_queue, &message, 0) == FuriStatusOk) {
        BSB_State_StateUpdate* update = message.data;
        flags |= message.stream_flags;
        if(update) {
            state_publisher_store_state_update(instance, update, message.stream_flags);
        }
        if(message.heartbeat) {
            heartbeat = true;
        }
    }

    if(heartbeat) {
        flags = StreamFlagAll;
    }

    uint32_t sleep_time_ms = send_out(instance, flags, heartbeat);

    furi_event_loop_timer_start(instance->rate_limiter_timer, ms_to_ticks(sleep_time_ms));
}

static void rate_limiter_timer_callback(void* context) {
    StatePublisher* instance = context;
    uint32_t sleep_time_ms = send_out(instance, StreamFlagAll, false);

    furi_event_loop_timer_start(instance->rate_limiter_timer, ms_to_ticks(sleep_time_ms));
}

static bool handle_transport_resumed(StatePublisher* instance, const ControlMessage* message) {
    furi_assert(message->type == MessageTypeTransportResumed);

    send_out(instance, StreamFlagAll, false);
    return true;
}

static const MessageHandler message_handlers[] = {
    [MessageTypeTransportResumed] = handle_transport_resumed,
};

static_assert(COUNT_OF(message_handlers) == MessageTypesCount);

void state_publisher_schedule_state_update(
    StatePublisher* instance,
    BSB_State_StateUpdate* update,
    StreamFlag flags) {
    PublishMessage msg = {
        .data = update,
        .stream_flags = flags,
        .heartbeat = false,
    };

    FuriStatus queue_status = furi_message_queue_put(instance->publish_queue, &msg, 0);

    if(queue_status != FuriStatusOk) {
        FURI_LOG_E(TAG, "Queue full, update dropped");
    }
}

static void heartbeat_timer_callback(void* context) {
    StatePublisher* instance = context;
    PublishMessage msg = {
        .heartbeat = true,
    };

    FuriStatus queue_status = furi_message_queue_put(instance->publish_queue, &msg, 0);

    if(queue_status != FuriStatusOk) {
        FURI_LOG_E(TAG, "Queue full, heartbeat dropped");
    }
}

void state_publisher_send_complete_snapshot(
    StatePublisher* instance,
    StatePublisherTransportHandle transport) {
    BSB_State_State* state = state_publisher_collect_all(instance);

    SharedByteArray_t data;

    SharedByteArray_init_new(data);

    ByteArray_t* buf = SharedByteArray_ref(data);
    pb_ostream_t stream = ostream_with_buffer(buf);

    bool result = pb_encode(&stream, BSB_State_State_fields, state);
    {
        furi_mutex_acquire(instance->transports_mutex, FuriWaitForever);
        Transport* t = instance->transports + transport;
        if(!result) {
            FURI_LOG_E(TAG, "cannot encode");
        } else {
            t->cb(data, t->cb_context);
            SharedStateUpdateArray_reset(t->state_updates);
        }
        furi_mutex_release(instance->transports_mutex);
    }
    SharedByteArray_clear(data);
    for(size_t i = 0; i != state->updates_count; ++i) {
        state_publisher_free_state_update(state->updates + i);
    }
    free(state->updates);
    free(state);
}

static int32_t fetch_thread_callback(void* context) {
    StatePublisher* instance = context;
    while(true) {
        uint32_t result = furi_event_flag_wait(
            instance->fetch_flags, FetchFlagAll, FuriFlagWaitAny, FuriWaitForever);
        furi_check((result & FuriFlagError) == 0);
        if(result & FetchFlagPowerEvent) {
            state_publisher_store_power(instance);
        }
        if(result & FetchFlagAudioEvent) {
            state_publisher_store_audio(instance);
        }
        if(result & FetchFlagMatterEvent) {
            state_publisher_store_matter(instance);
        }
        if(result & FetchFlagUpdaterCheckEvent) {
            state_publisher_store_update_check(instance);
        }
        if(result & FetchFlagBusyTimer) {
            state_publisher_store_busy_timer(instance);
        }
        if(result & FetchFlagBusyTimerProfiles) {
            state_publisher_store_busy_timer_profiles(instance);
        }
        if(result & FetchFlagAutoupdateEvent) {
            state_publisher_store_autoupdate(instance);
        }
        if(result & FetchFlagBle) {
            state_publisher_store_ble(instance);
        }

        if(result) {
            PublishMessage msg = {
                .heartbeat = false,
            };

            FuriStatus queue_status = furi_message_queue_put(instance->publish_queue, &msg, 0);

            if(queue_status != FuriStatusOk) {
                FURI_LOG_E(TAG, "Queue full, update postponed");
            }
        }
    }
    return 0;
}
