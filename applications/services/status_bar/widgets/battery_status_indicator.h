#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BATTERY_STATUS_INDICATOR_MAX_CHARGE_AMOUNT 100

typedef struct BatteryStatusIndicator BatteryStatusIndicator;

BatteryStatusIndicator* battery_status_indicator_alloc(Widget* parent);

void battery_status_indicator_free(BatteryStatusIndicator* instance);

Widget* battery_status_indicator_get_base(BatteryStatusIndicator* instance);

void battery_status_indicator_set_charge_amount(BatteryStatusIndicator* instance, uint8_t charge);

void battery_status_indicator_set_charging_state(
    BatteryStatusIndicator* instance,
    bool is_charging);

void battery_status_indicator_set_error_state(BatteryStatusIndicator* instance, bool is_error);

#ifdef __cplusplus
}
#endif
