#include <furi.h>
#include <furi_hal_usb_pd.h>

#define TAG "PD Demo"

static void pd_demo_cap_change(const void* message, void* context) {
    const UsbPdCapability* caps = message;
    // TODO: notify thread
    UNUSED(context);
    FURI_LOG_I(TAG, "Capabilities update");
    for(size_t i = 0; i < caps->cap_number; i++) {
        if(caps->cap[i].is_fixed) {
            FURI_LOG_I(
                TAG,
                "cap%u fixed V: %lu mV C: %lu mA",
                i + 1,
                caps->cap[i].voltage_max,
                caps->cap[i].current_max);
        } else {
            FURI_LOG_I(
                TAG,
                "cap%u PPS V: %lu-%lu mV C: %lu mA",
                i + 1,
                caps->cap[i].voltage_min,
                caps->cap[i].voltage_max,
                caps->cap[i].current_max);
        }
    }
}

int32_t pd_demo(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Started");

    FuriPubSubSubscription* pd_event_sub =
        furi_pubsub_subscribe(furi_hal_usb_pd_get_pubsub(), pd_demo_cap_change, NULL);

    UNUSED(pd_event_sub);

    while(1) {
        furi_delay_ms(500);
    }

    return 0;
}