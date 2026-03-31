#include "state_publisher.h"
#include "state_publisher_i.h"
#include <furi/furi.h>

#include <nanopb/pb.h>
#include <nanopb/pb_encode.h>
#include <state.pb.h>

#include <time/time.h>

#define MAX_MESSAGES 16

#define HEARTBEAT_INTERVAL_MS 991

typedef bool (*MessageHandler)(StatePublisher* instance, const Message* message);

static const MessageHandler message_handlers[];

static void heartbeat_timer_callback(void* context);

void screen_streamer_callback(
    GuiDisplayId display,
    const ScreenStreamerFrame* frame,
    uint8_t stream_flags,
    void* context);

static void message_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    StatePublisher* instance = context;

    Message message;
    furi_check(furi_message_queue_get(instance->message_queue, &message, 0) == FuriStatusOk);

    message_handlers[message.type](instance, &message);
}

void state_publisher_send_message(StatePublisher* instance, const Message* message) {
    if(furi_thread_get_current_id() == instance->main_thread_id) {
        message_handlers[message->type](instance, message);
    } else {
        furi_check(
            furi_message_queue_put(instance->message_queue, message, FuriWaitForever) ==
            FuriStatusOk);
    }
}

static StatePublisher* state_publisher_alloc(void) {
    StatePublisher* instance = malloc(sizeof(StatePublisher));

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(MAX_MESSAGES, sizeof(Message));
    instance->main_thread_id = furi_thread_get_current_id();

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        message_queue_callback,
        instance);

    instance->heartbeat_timer = furi_event_loop_timer_alloc(
        instance->event_loop, heartbeat_timer_callback, FuriEventLoopTimerTypePeriodic, instance);

    instance->gui = furi_record_open(RECORD_GUI);

    instance->screen_streamer_front = screen_streamer_alloc(
        GuiDisplayIdFront, instance->gui, screen_streamer_callback, instance);

    instance->transports_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    bzero(instance->transports, sizeof(instance->transports));

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
            if(t->valid) {
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

static void free_state_update(BSB_State_StateUpdate* update) {
    switch(update->which_state) {
    case BSB_State_StateUpdate_timer_tag:
        free(update->state.timer.json.data);
        break;
    case BSB_State_StateUpdate_frame_tag:
        free(update->state.frame.data);
        break;
    default:
        break;
    }
    free(update);
}

static bool handle_publish_update(StatePublisher* instance, const Message* message) {
    furi_assert(message->type == MessageTypePublishUpdate);
    BSB_State_StateUpdate* update = message->update.data;
    BSB_State_State state = {
        .timestamp = time_get_timestamp_ms(),
        .updates_count = update ? 1 : 0,
        .updates = (BSB_State_StateUpdate*)update,
    };

    SharedByteArray_t data;
    SharedByteArray_init_new(data);

    ByteArray_t* buf = SharedByteArray_ref(data);
    pb_ostream_t stream = ostream_with_buffer(buf);

    bool result = pb_encode(&stream, BSB_State_State_fields, &state);
    if(!result) {
        FURI_LOG_E(TAG, "cannot encode");
    } else {
        furi_mutex_acquire(instance->transports_mutex, FuriWaitForever);
        for(size_t i = 0; i != MAX_TRANSPORTS; ++i) {
            Transport* t = instance->transports + i;
            if(t->valid && (t->flags & message->update.stream_flags)) {
                t->cb(data, t->cb_context);
            }
        }
        furi_mutex_release(instance->transports_mutex);
    }
    SharedByteArray_clear(data);

    if(update) {
        free_state_update(update);
    }
    return true;
}

static bool handle_power_event(StatePublisher* instance, const Message* message) {
    furi_assert(message->type == MessageTypePowerEvent);
    state_publisher_publish_power(instance);
    return true;
}

static bool handle_audio_event(StatePublisher* instance, const Message* message) {
    furi_assert(message->type == MessageTypeAudioEvent);
    state_publisher_publish_audio(instance);
    return true;
}

static bool handle_matter_event(StatePublisher* instance, const Message* message) {
    furi_assert(message->type == MessageTypeMatterEvent);
    state_publisher_publish_matter(instance);
    return true;
}

static bool handle_updater_check_event(StatePublisher* instance, const Message* message) {
    furi_assert(message->type == MessageTypeUpdaterCheckEvent);
    state_publisher_publish_update_check(instance, &message->updater_check_state);
    return true;
}

static bool handle_busy_timer(StatePublisher* instance, const Message* message) {
    furi_assert(message->type == MessageTypeBusyTimer);
    state_publisher_publish_busy_timer(instance);
    return true;
}

static const MessageHandler message_handlers[] = {
    [MessageTypePublishUpdate] = handle_publish_update,
    [MessageTypePowerEvent] = handle_power_event,
    [MessageTypeAudioEvent] = handle_audio_event,
    [MessageTypeMatterEvent] = handle_matter_event,
    [MessageTypeUpdaterCheckEvent] = handle_updater_check_event,
    [MessageTypeBusyTimer] = handle_busy_timer,
};

static_assert(COUNT_OF(message_handlers) == MessageTypesCount);

void state_publisher_schedule_state_update(
    StatePublisher* instance,
    BSB_State_StateUpdate* update,
    StreamFlag flags) {
    Message msg = {
        .type = MessageTypePublishUpdate,
        .update =
            {
                .data = update,
                .stream_flags = flags,
            },
    };
    state_publisher_send_message(instance, &msg);
}

static void heartbeat_timer_callback(void* context) {
    StatePublisher* instance = context;
    state_publisher_schedule_state_update(instance, NULL, StreamFlagAll);
}
