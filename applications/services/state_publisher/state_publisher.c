#include "state_publisher.h"
#include <furi/furi.h>

#define TAG "StPubSrv"

#define MAX_MESSAGES 16

struct StatePublisher {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
};

typedef enum {
    MessageTypePublish,

    MessageTypesCount,
} MessageType;

typedef struct {
    MessageType type;
} Message;

typedef bool (*MessageHandler)(StatePublisher* instance, const Message* message);

static const MessageHandler message_handlers[];

static void message_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    StatePublisher* instance = context;

    Message message;
    furi_check(furi_message_queue_get(instance->message_queue, &message, 0) == FuriStatusOk);

    message_handlers[message.type](instance, &message);
}

static void send_message(StatePublisher* instance, const Message* message) {
    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);
}

static StatePublisher* state_publisher_alloc() {
    StatePublisher* instance = malloc(sizeof(StatePublisher));

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(MAX_MESSAGES, sizeof(Message));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        message_queue_callback,
        instance);

    furi_record_create(RECORD_STATE_PUBLISHER, instance);

    return instance;
}

int32_t state_publisher_srv(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Service starting...");

    StatePublisher* instance = state_publisher_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

static bool do_publish(StatePublisher* instance, const Message* message) {
    UNUSED(instance);
    UNUSED(message);

    return true;
}

static const MessageHandler message_handlers[] = {
    [MessageTypePublish] = do_publish,
};

static_assert(COUNT_OF(message_handlers) == MessageTypesCount);

void state_publisher_publish(StatePublisher* app) {
    Message msg = {
        .type = MessageTypePublish,
    };
    send_message(app, &msg);
}
