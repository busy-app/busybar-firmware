#include "run_later.h"

#include <furi.h>

typedef struct {
    FuriEventLoopTimer* timer;
    RunLaterCallback callback;
    void* callback_context;
} RunLater;

static void run_later_free(RunLater* instance) {
    furi_event_loop_timer_free(instance->timer);
    free(instance);
}

static void run_later_timer_callback(void* context) {
    furi_assert(context);
    RunLater* instance = context;

    instance->callback(instance->callback_context);
    run_later_free(instance);
}

void run_later(
    FuriEventLoop* event_loop,
    RunLaterCallback callback,
    void* context,
    uint32_t delay_ms) {
    furi_assert(event_loop);
    furi_assert(callback);

    RunLater* instance = malloc(sizeof(RunLater));

    instance->timer = furi_event_loop_timer_alloc(
        event_loop, run_later_timer_callback, FuriEventLoopTimerTypeOnce, instance);
    instance->callback = callback;
    instance->callback_context = context;

    furi_event_loop_timer_start(instance->timer, delay_ms);
}
