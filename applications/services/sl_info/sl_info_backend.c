#include <furi_hal_info.h>

#include <intercom/intercom.h>

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

static void sl_info_send_data(IntercomChannel* info_channel) {
    FURI_LOG_D(TAG, "Sending info ...");

    furi_hal_info_get(sl_info_property_callback, '_', info_channel);

    FURI_LOG_D(TAG, "Info sent");
}

void sl_info_on_system_start(void) {
    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    IntercomChannel* info_channel =
        intercom_channel_open(intercom, IntercomChannelIdSlInfo, NULL, NULL);

    sl_info_send_data(info_channel);
    furi_record_close(RECORD_INTERCOM);
}
