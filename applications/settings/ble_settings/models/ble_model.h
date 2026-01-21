#pragma once

#include <furi.h>
#include <ble/ble.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BleModelStateEventBleChanged,
    BleModelStateEventNameChanged,

    BleModelStateEventCount,
} BleModelStateEvent;

typedef void (*BleModelStateCallback)(BleModelStateEvent event, void* context);

typedef struct BleModel BleModel;

BleModel* ble_model_alloc(void);

void ble_model_free(BleModel* model);

void ble_model_get_status(BleModel* model, BleStatus* output);

void ble_model_get_name(BleModel* model, FuriString* name);

bool ble_model_is_device_paired(BleModel* model);

void ble_model_set_state_callback(BleModel* model, BleModelStateCallback callback, void* context);

void ble_model_start(BleModel* model);

void ble_model_stop(BleModel* model);

void ble_model_forget(BleModel* model);

#ifdef __cplusplus
}
#endif
