#include "wifi_i.h"

#ifdef SRV_POWER
#include <power/power_service/power.h>

// NOTE: Unlike most cases, this callback SHOULD
// block until it completes the necessary actions
static void wifi_power_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const PowerEvent* event = message;
    Wifi* instance = context;

    if(event->type == PowerEventShutdown) {
        wifi_disconnect(instance);
    }
}

#endif // SRV_POWER

void wifi_power_init(Wifi* instance) {
#ifdef SRV_POWER
    Power* power = furi_record_open(RECORD_POWER);
    furi_pubsub_subscribe(power_get_pubsub(power), wifi_power_pubsub_callback, instance);
    furi_record_close(RECORD_POWER);
#else // SRV_POWER
    UNUSED(instance);
#endif // SRV_POWER
}
