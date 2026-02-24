#include "intercom_watchdog.h"

#include "intercom_i.h"

#define TAG "IntercomWatchdog"

#define INTERCOM_WATCHDOG_QUEUE_SIZE (4)
#define INTERCOM_WATCHDOG_TIMEOUT_MS (900)

struct IntercomWatchdog {
    FuriEventLoop* event_loop;
    FuriMessageQueue* queue;
    FuriEventLoopTimer* timer;
};

typedef enum {
    IntercomWatchdogEventArm,
    IntercomWatchdogEventDisarm,
    IntercomWatchdogEventMax,
} IntercomWatchdogEvent;

static void intercom_watchdog_timer_callback(void* context) {
    UNUSED(context);

    FURI_LOG_E(TAG, "Stop condition reached");

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    intercom_dump_frame(&intercom->rx_frame);

    furi_delay_ms(100);
    furi_crash("Stop condition reached");
}

static void intercom_watchdog_message_queue_callback(FuriEventLoopObject* obj, void* context) {
    furi_assert(context);
    IntercomWatchdog* instance = context;

    furi_assert(instance->queue == obj);

    IntercomWatchdogEvent event;
    while(furi_message_queue_get(instance->queue, &event, 0) == FuriStatusOk) {
        if(event == IntercomWatchdogEventArm) {
            furi_event_loop_timer_start(instance->timer, INTERCOM_WATCHDOG_TIMEOUT_MS);
        } else if(event == IntercomWatchdogEventDisarm) {
            furi_event_loop_timer_stop(instance->timer);
        } else {
            furi_crash("Invalid IntercomWatchdogEvent value");
        }
    }
}

static IntercomWatchdog* intercom_watchdog_alloc(void) {
    IntercomWatchdog* instance = malloc(sizeof(IntercomWatchdog));
    instance->event_loop = furi_event_loop_alloc();
    instance->queue =
        furi_message_queue_alloc(INTERCOM_WATCHDOG_QUEUE_SIZE, sizeof(IntercomWatchdogEvent));
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop, intercom_watchdog_timer_callback, FuriEventLoopTimerTypeOnce, NULL);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->queue,
        FuriEventLoopEventIn,
        intercom_watchdog_message_queue_callback,
        instance);

    furi_record_create(RECORD_INTERCOM_WATCHDOG, instance);
    return instance;
}

void intercom_watchdog_arm(IntercomWatchdog* instance) {
    const IntercomWatchdogEvent event = IntercomWatchdogEventArm;
    furi_check(furi_message_queue_put(instance->queue, &event, FuriWaitForever) == FuriStatusOk);
}

void intercom_watchdog_disarm(IntercomWatchdog* instance) {
    const IntercomWatchdogEvent event = IntercomWatchdogEventDisarm;
    furi_check(furi_message_queue_put(instance->queue, &event, FuriWaitForever) == FuriStatusOk);
}

int32_t intercom_watchdog_srv(void* arg) {
    UNUSED(arg);

    IntercomWatchdog* instance = intercom_watchdog_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
