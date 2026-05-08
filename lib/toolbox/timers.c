#include "timers.h"

#include <furi_hal_cpu.h>

#include <core/check.h>
#include <core/kernel.h>

#define TIMER_GET_START(t) ((t).data[TimerDataIdxStart])
#define TIMER_GET_VALUE(t) ((t).data[TimerDataIdxValue])

typedef enum {
    TimerDataIdxStart,
    TimerDataIdxValue,
} TimerDataIdx;

CoarseTimer coarse_timer_create(uint32_t timeout_ms) {
    return (CoarseTimer){
        .data = {
            [TimerDataIdxStart] = furi_get_tick(),
            [TimerDataIdxValue] = furi_ms_to_ticks(timeout_ms),
        }};
}

uint32_t coarse_timer_get_elapsed(const CoarseTimer timer) {
    return (furi_get_tick() - TIMER_GET_START(timer)) / furi_ms_to_ticks(1);
}

bool coarse_timer_is_expired(const CoarseTimer timer) {
    return (furi_get_tick() - TIMER_GET_START(timer)) >= TIMER_GET_VALUE(timer);
}

PreciseTimer precise_timer_create(uint32_t timeout_us) {
    furi_check(timeout_us < (UINT32_MAX / furi_hal_cpu_get_cycles_per_us()));
    return (PreciseTimer){
        .data = {
            [TimerDataIdxStart] = furi_hal_cpu_get_cycle_count(),
            [TimerDataIdxValue] = furi_hal_cpu_get_cycles_per_us() * timeout_us,
        }};
}

uint32_t precise_timer_get_elapsed(const PreciseTimer timer) {
    return (furi_hal_cpu_get_cycle_count() - TIMER_GET_START(timer)) /
           furi_hal_cpu_get_cycles_per_us();
}

bool precise_timer_is_expired(const PreciseTimer timer) {
    return (furi_hal_cpu_get_cycle_count() - TIMER_GET_START(timer)) >= TIMER_GET_VALUE(timer);
}

void precise_timer_wait(const PreciseTimer timer) {
    while(!precise_timer_is_expired(timer)) {
        // Empty
    }
}

bool precise_timer_wait_for(
    const PreciseTimer timer,
    TimerConditionCallback condition_cb,
    void* context) {
    furi_check(condition_cb);

    bool success;

    for(;;) {
        const bool condition_reached = condition_cb(context);
        const bool timer_expired = precise_timer_is_expired(timer);

        if(condition_reached || timer_expired) {
            success = condition_reached && !timer_expired;
            break;
        }
    }

    return success;
}
