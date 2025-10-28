#include "busy_timer_i.h"

static BusyTimer* busy_timer_alloc(void) {
    BusyTimer* instance = malloc(sizeof(BusyTimer));

    instance->event_loop = furi_event_loop_alloc();

    furi_record_create(RECORD_BUSY_TIMER, instance);
    return instance;
}

int busy_timer_srv(void* arg) {
    UNUSED(arg);

    BusyTimer* instance = busy_timer_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
