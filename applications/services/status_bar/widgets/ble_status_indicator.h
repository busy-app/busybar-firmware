#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BleStatusIndicatorStateUnknown,
    BleStatusIndicatorStateDisconnected,
    BleStatusIndicatorStateConnectable,
    BleStatusIndicatorStateConnected,
    BleStatusIndicatorStateMax,
} BleStatusIndicatorState;

typedef struct BleStatusIndicator BleStatusIndicator;

BleStatusIndicator* ble_status_indicator_alloc(Widget* parent);

void ble_status_indicator_free(BleStatusIndicator* instance);

Widget* ble_status_indicator_get_base(BleStatusIndicator* instance);

void ble_status_indicator_set_state(BleStatusIndicator* instance, BleStatusIndicatorState state);

#ifdef __cplusplus
}
#endif
