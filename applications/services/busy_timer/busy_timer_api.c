#include "busy_timer_i.h"

void busy_timer_get_snapshot(BusyTimer* instance, BusyTimerSnapshot* snapshot) {
    furi_check(instance);
    furi_check(snapshot);
    // TODO: Proper implementation
    *snapshot = instance->snapshot;
}

bool busy_timer_set_snapshot(BusyTimer* instance, const BusyTimerSnapshot* snapshot) {
    furi_check(instance);
    furi_check(snapshot);
    // TODO: Proper implementation
    instance->snapshot = *snapshot;
    return true;
}
