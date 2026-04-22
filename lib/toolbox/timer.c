#include "timer.h"

#include <core/check.h>
#include <core/kernel.h>

#define TIMER_GET_START(t) ((t).data[TimerDataIdxStart])
#define TIMER_GET_VALUE(t) ((t).data[TimerDataIdxValue])

typedef enum {
    TimerDataIdxStart,
    TimerDataIdxValue,
    TimerDataIdxMax,
} TimerDataIdx;

CoarseTimer coarse_timer_create(uint32_t timeout_ms) {
    return (CoarseTimer){
        .data = {
            [TimerDataIdxStart] = furi_get_tick(),
            [TimerDataIdxValue] = furi_ms_to_ticks(timeout_ms),
        }};
}

CoarseTimer coarse_timer_create_synced(const CoarseTimer previous, uint32_t timeout_ms) {
    return (CoarseTimer){
        .data = {
            [TimerDataIdxStart] = TIMER_GET_START(previous) + TIMER_GET_VALUE(previous),
            [TimerDataIdxValue] = furi_ms_to_ticks(timeout_ms),
        }};
}

bool coarse_timer_is_expired(const CoarseTimer timer) {
    return (furi_get_tick() - TIMER_GET_START(timer)) >= TIMER_GET_VALUE(timer);
}

PreciseTimer precise_timer_create(uint32_t timeout_us) {
    // TODO: Proper check
    furi_check(timeout_us);

    return (PreciseTimer){
        .data = {
            [TimerDataIdxStart] = 0, // TODO: Implementation
            [TimerDataIdxValue] = 0,
        }};
}

PreciseTimer precise_timer_create_synced(const PreciseTimer previous, uint32_t timeout_us) {
    // TODO: Proper check
    furi_check(timeout_us);

    return (PreciseTimer){
        .data = {
            [TimerDataIdxStart] = TIMER_GET_START(previous) + TIMER_GET_VALUE(previous),
            [TimerDataIdxValue] = 0, // TODO: Implementation
        }};
}

bool precise_timer_is_expired(const PreciseTimer timer) {
    UNUSED(timer);
    // TODO: Implementation
    return false;
}

void precise_timer_wait(const PreciseTimer timer) {
    while(!precise_timer_is_expired(timer)) {
        // Empty
    }
}

bool precise_timer_wait_for(const PreciseTimer timer, TimerConditionCallback condition_cb) {
    return precise_timer_wait_for_ex(timer, condition_cb, NULL);
}

bool precise_timer_wait_for_ex(
    const PreciseTimer timer,
    TimerConditionCallback condition_cb,
    void* context) {
    furi_check(condition_cb);

    bool condition_reached;

    do {
        condition_reached = condition_cb(context);
    } while(!(condition_reached || precise_timer_is_expired(timer)));

    return condition_reached;
}
