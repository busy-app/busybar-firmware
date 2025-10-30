#include "busy_timer_i.h"

typedef void (*BusyTimerApiHandler)(BusyTimer* instance, BusyTimerApiMessage* message);

static BusyTimerApiHandler busy_timer_handlers[];

static void busy_timer_get_snapshot_handler(BusyTimer* instance, BusyTimerApiMessage* message) {
    const BusyTimerApiMessageGetSnapshot* get_snapshot = &message->get_snapshot;
    *get_snapshot->snapshot = instance->snapshot;
    // TODO: Implement actual logic
}

static void busy_timer_set_snapshot_handler(BusyTimer* instance, BusyTimerApiMessage* message) {
    const BusyTimerApiMessageSetSnapshot* set_snapshot = &message->set_snapshot;
    instance->snapshot = *set_snapshot->snapshot;
    // TODO: Implement actual logic
}

static void busy_timer_api_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    BusyTimer* instance = context;

    furi_assert(object == instance->api_queue);

    BusyTimerApiMessage message;
    while(furi_message_queue_get(instance->api_queue, &message, 0) == FuriStatusOk) {
        furi_assert(message.type < BusyTimerApiMessageTypeMax);
        busy_timer_handlers[message.type](instance, &message);
        api_lock_unlock(message.lock);
    }
}

static BusyTimer* busy_timer_alloc(void) {
    BusyTimer* instance = malloc(sizeof(BusyTimer));

    instance->event_loop = furi_event_loop_alloc();
    instance->api_queue = furi_message_queue_alloc(4, sizeof(BusyTimerApiMessage));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->api_queue,
        FuriEventLoopEventIn,
        busy_timer_api_queue_callback,
        instance);

    furi_record_create(RECORD_BUSY_TIMER, instance);
    return instance;
}

static BusyTimerApiHandler busy_timer_handlers[BusyTimerApiMessageTypeMax] = {
    [BusyTimerApiMessageTypeGetSnapshot] = busy_timer_get_snapshot_handler,
    [BusyTimerApiMessageTypeSetSnapshot] = busy_timer_set_snapshot_handler,
};

int busy_timer_srv(void* arg) {
    UNUSED(arg);

    BusyTimer* instance = busy_timer_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
