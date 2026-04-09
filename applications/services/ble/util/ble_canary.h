#pragma once

#include <furi.h>

typedef enum {
    BleCanaryTypeHitOnce,
    BleCanaryTypeHitAlways,
} BleCanaryType;

typedef struct BleDebugCanary BleDebugCanary;

typedef void (*BleDebugCanaryHitCallback)(void* ctx);

BleDebugCanary* ble_debug_canary_alloc(BleCanaryType type);
void ble_debug_canary_set_hit_callback(
    BleDebugCanary* instance,
    BleDebugCanaryHitCallback callback);

void ble_debug_canary_free(BleDebugCanary* instance);
void ble_debug_canary_reset(BleDebugCanary* instance);
void ble_debug_canary_test(BleDebugCanary* instance, void* ctx);
void ble_debug_canary_test_log(BleDebugCanary* instance, const char* tag, const char* format, ...);
