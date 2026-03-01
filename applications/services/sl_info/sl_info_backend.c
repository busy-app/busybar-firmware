#include <furi_hal_info.h>

#include <intercom/intercom.h>
#include <wifi/wifi_common.h>

#include "sl_info_common_i.h"

#define TAG "SlInfo"

static void
    sl_info_property_callback(const char* key, const char* value, bool last, void* context) {
    furi_assert(context);
    IntercomChannel* info_channel = context;

    SlInfoIntercomFrame frame;
    strlcpy(frame.key, key, sizeof(frame.key));
    strlcpy(frame.value, value, sizeof(frame.value));
    frame.is_last = last;

    furi_check(intercom_tx(info_channel, &frame, sizeof(frame), FuriWaitForever) == sizeof(frame));
}

void sl_info_on_system_start(void) {
    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    IntercomChannel* info_channel =
        intercom_channel_open(intercom, IntercomChannelIdSlInfo, NULL, NULL);

    // Wifi needs to be ready for furi_hal_info_get() to work
    furi_record_open(RECORD_WIFI);

    FURI_LOG_D(TAG, "Sending info ...");

    furi_hal_info_get(sl_info_property_callback, '_', info_channel);

    FURI_LOG_D(TAG, "Info sent");

    furi_record_close(RECORD_INTERCOM);
    furi_record_close(RECORD_WIFI);
}
