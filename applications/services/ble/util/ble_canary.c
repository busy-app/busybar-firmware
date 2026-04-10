#include "ble_canary.h"

struct BleDebugCanary {
    bool state;
    BleCanaryType type;
    uint32_t hit_count;
    FuriMutex* lock;
    BleDebugCanaryHitCallback callback;
};

BleDebugCanary* ble_debug_canary_alloc(BleCanaryType type) {
    BleDebugCanary* instance = malloc(sizeof(BleDebugCanary));
    instance->lock = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->type = type;
    instance->state = false;
    return instance;
}

void ble_debug_canary_free(BleDebugCanary* instance) {
    furi_assert(instance);
    furi_mutex_free(instance->lock);
    free(instance);
}

void ble_debug_canary_set_hit_callback(
    BleDebugCanary* instance,
    BleDebugCanaryHitCallback callback) {
    furi_assert(instance);
    instance->callback = callback;
}

#define with_debug_canary(canary, code)                    \
    {                                                      \
        furi_mutex_acquire(canary->lock, FuriWaitForever); \
        {code};                                            \
        furi_mutex_release(canary->lock);                  \
    }

void ble_debug_canary_reset(BleDebugCanary* instance) {
    furi_assert(instance);
    with_debug_canary(instance, {
        instance->state = false;
        instance->hit_count = 0;
    });
}

static inline bool ble_debug_canary_hit(BleDebugCanary* instance) {
    if((instance->type == BleCanaryTypeHitAlways) ||
       (instance->type == BleCanaryTypeHitOnce && !instance->state)) {
        instance->state = true;
        instance->hit_count += 1;
        return true;
    }
    return false;
}

void ble_debug_canary_test(BleDebugCanary* instance, void* ctx) {
    furi_assert(instance);
    furi_assert(ctx);
    with_debug_canary(instance, {
        if(ble_debug_canary_hit(instance) && instance->callback) {
            instance->callback(ctx);
        }
    });
}

void ble_debug_canary_test_log(BleDebugCanary* instance, const char* tag, const char* format, ...) {
    furi_assert(instance);
    furi_assert(tag);
    furi_assert(format);
    with_debug_canary(instance, {
        if(ble_debug_canary_hit(instance)) {
            va_list args;
            va_start(args, format);
            FuriString* str = furi_string_alloc_vprintf(format, args);
            va_end(args);

            FURI_LOG_W(tag, "%s", furi_string_get_cstr(str));
            furi_string_free(str);
        }
    });
}
