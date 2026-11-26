#include "power.h"
#include "../power_intercom_i.h"

#include <furi_hal_power.h>
#include <furi_hal_resources.h>

#include <intercom/intercom.h>

#define TAG "Power"

static void power_intercom_rx(const void* data, size_t data_size, void* context) {
    UNUSED(context);
    furi_check(data_size == sizeof(PowerIntercomMessage));
    const PowerIntercomMessage* message = data;

    if(message->type == PowerIntercomMessageTypeDeepSleepWithWakeupOnModeSwitchOutOfOffPosition) {
        FURI_LOG_I(TAG, "Going to deep sleep with wakeup on mode switch out of OFF position");
        furi_hal_power_sleep_wakeup_clear();
        furi_hal_power_sleep_wakeup_gpio(&gpio_sw_off, GpioConditionRise);
        furi_hal_power_deep_sleep();
    }
}

void power_startup(void) {
    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    IntercomChannel* channel =
        intercom_channel_open(intercom, IntercomChannelIdPower, power_intercom_rx, NULL);
    UNUSED(channel);
}
