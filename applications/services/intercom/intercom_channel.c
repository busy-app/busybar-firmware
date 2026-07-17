#include "intercom_i.h"

#define TAG "IntercomChannel"

typedef enum {
    IntercomChannelFlagReady = (1UL << 0),
} IntercomChannelFlag;

static const char* const intercom_channel_names[IntercomChannelIdMax] = {
    [IntercomChannelIdInput] = "Input",
    [IntercomChannelIdWifiControl] = "Wifi",
    [IntercomChannelIdWifiData] = "WifiData",
    [IntercomChannelIdStatusLights] = "StatusLights",
    [IntercomChannelIdCli] = "Cli",
    [IntercomChannelIdBle] = "Ble",
    [IntercomChannelIdTlsCrypto] = "TlsCrypto",
    [IntercomChannelIdCryptoBackup] = "CryptoBackup",
    [IntercomChannelIdMatter] = "Matter",
    [IntercomChannelIdSlInfo] = "SlInfo",
    [IntercomChannelIdLogDump] = "LogDump",
    [IntercomChannelIdDebug] = "Debug",
};

void intercom_channel_init(IntercomChannel* channel, Intercom* owner) {
    furi_assert(channel);
    furi_assert(owner);

    channel->owner = owner;
    channel->flags = furi_event_flag_alloc();
}

void intercom_channel_set_callback(
    IntercomChannel* channel,
    IntercomRxCallback callback,
    void* context) {
    furi_assert(channel);

    channel->rx_callback = callback;
    channel->rx_callback_context = context;
}

void intercom_channel_call_callback(const IntercomChannel* channel, const IntercomFrame* frame) {
    furi_assert(channel);

    const IntercomRxCallback callback = channel->rx_callback;

    if(callback) {
        callback(frame->data, frame->data_size, channel->rx_callback_context);
    } else {
        FURI_LOG_W(TAG, "Receive callback is NOT set, but other side is sending data");
    }
}

bool intercom_channel_wait_until_ready(IntercomChannel* channel, uint32_t timeout) {
    furi_assert(channel);

    const uint32_t flags = furi_event_flag_wait(
        channel->flags, IntercomChannelFlagReady, FuriFlagNoClear | FuriFlagWaitAny, timeout);

    bool is_ready;

    if(flags == IntercomChannelFlagReady) {
        is_ready = true;
    } else {
        furi_check(flags == (timeout ? FuriFlagErrorTimeout : FuriFlagErrorResource));
        is_ready = false;
    }

    return is_ready;
}

void intercom_channel_mark_as_ready(IntercomChannel* channel) {
    furi_assert(channel);

    const uint32_t flags = furi_event_flag_set(channel->flags, IntercomChannelFlagReady);
    furi_check(flags == IntercomChannelFlagReady);
}

const char* intercom_channel_get_name(IntercomChannelId channel_id) {
    furi_assert(channel_id < IntercomChannelIdMax);
    return intercom_channel_names[channel_id];
}
