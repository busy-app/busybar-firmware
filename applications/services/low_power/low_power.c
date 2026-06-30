#include <furi.h>
#include "low_power.h"
#include <back_display/back_display.h>
#include <front_display/front_display.h>
#include <light_sensor/light_sensor.h>

#define TAG "LowPower"

typedef enum {
    LowPowerThreadFlagLock = (1 << 0),
    LowPowerThreadFlagUnlock = (1 << 1),
} LowPowerThreadFlag;

struct LowPower {
    FuriThread* thread;
    uint32_t lock_count;
    bool in_low_power;

    BackDisplaySrv* back_display;
    FrontDisplaySrv* front_display;
};

static void low_power_enter(LowPower* instance) {
    UNUSED(instance);
    front_display_sleep_mode(instance->front_display, true);
    back_display_sleep_mode(instance->back_display, true);
    light_sensor_sleep(true);
}

static void low_power_exit(LowPower* instance) {
    UNUSED(instance);
    front_display_sleep_mode(instance->front_display, false);
    back_display_sleep_mode(instance->back_display, false);
    light_sensor_sleep(false);
}

static LowPower* low_power_alloc(void) {
    LowPower* instance = malloc(sizeof(LowPower));
    instance->thread = furi_thread_get_current();
    instance->lock_count = 1; // Locked by default
    instance->in_low_power = false;

    instance->back_display = furi_record_open(RECORD_BACK_DISPLAY);
    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);

    return instance;
}

int32_t low_power_srv(void* arg) {
    UNUSED(arg);
    LowPower* instance = low_power_alloc();
    furi_record_create(RECORD_LOW_POWER, instance);

    while(1) {
        uint32_t flags = furi_thread_flags_wait(
            LowPowerThreadFlagLock | LowPowerThreadFlagUnlock, FuriFlagWaitAny, FuriWaitForever);
        furi_check((flags & FuriFlagError) == 0);
        if(flags & LowPowerThreadFlagLock) {
            instance->lock_count++;

            if((instance->lock_count > 0) && (instance->in_low_power)) {
                low_power_exit(instance);
                instance->in_low_power = false;
            }
        }
        if(flags & LowPowerThreadFlagUnlock) {
            if(instance->lock_count > 0) {
                instance->lock_count--;
            }

            if((instance->lock_count == 0) && (!instance->in_low_power)) {
                low_power_enter(instance);
                instance->in_low_power = true;
            }
        }
    }

    return 0;
}

void low_power_lock(LowPower* instance) {
    furi_assert(instance);
    furi_thread_flags_set(instance->thread, LowPowerThreadFlagLock);
}

void low_power_unlock(LowPower* instance) {
    furi_assert(instance);
    furi_thread_flags_set(instance->thread, LowPowerThreadFlagUnlock);
}
