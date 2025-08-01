#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BleStatusIndicator BleStatusIndicator;

BleStatusIndicator* ble_status_indicator_alloc(Widget* parent);

void ble_status_indicator_free(BleStatusIndicator* instance);

Widget* ble_status_indicator_get_base(BleStatusIndicator* instance);

#ifdef __cplusplus
}
#endif
