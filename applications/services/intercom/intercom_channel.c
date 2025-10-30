#include "intercom_i.h"

#define TAG "IntercomChannel"

typedef enum {
    IntercomMetaTypeChannelReady,
    IntercomMetaTypeMax,
} IntercomMetaType;

/**
 * @brief Sub-frame of type `IntercomMetaTypeChannelReady`
 */
typedef struct {
    IntercomChannelId channel_id;
} IntercomMetaFrameChannelReady;

/**
 * @brief Frame transmitted on the `Meta` channel
 */
typedef struct {
    IntercomMetaType type;
    union {
        IntercomMetaFrameChannelReady channel_ready;
    };
} IntercomMetaFrame;

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
    [IntercomChannelIdDebug] = "Debug",
    [IntercomChannelIdMeta] = "Meta",
};

static const char* intercom_channel_id_name(IntercomChannelId channel_id) {
    furi_check(channel_id < IntercomChannelIdMax);
    const char* name = intercom_channel_names[channel_id];
    return name ? name : "Unknown";
}

static FURI_ALWAYS_INLINE IntercomChannelId intercom_channel_id(IntercomChannel* channel) {
    furi_assert(channel);
    return channel - channel->intercom->handles;
}

static void intercom_meta_channel_ready(Intercom* intercom, const IntercomMetaFrame* frame) {
    furi_assert(intercom);
    furi_assert(frame);
    furi_assert(frame->type == IntercomMetaTypeChannelReady);
    const IntercomMetaFrameChannelReady* sub_frame = &frame->channel_ready;

    IntercomChannelId channel_id = sub_frame->channel_id;
    furi_check(channel_id < IntercomChannelIdMax);
    furi_check(channel_id != IntercomChannelIdMeta);

    FURI_LOG_D(TAG, "OTHER side ready: %s", intercom_channel_id_name(channel_id));

    const IntercomChannel* channel = &intercom->handles[channel_id];
    furi_check(
        !(furi_event_flag_set(channel->flags, IntercomChannelFlagPeerReady) & FuriFlagError));
}

static void intercom_meta_channel_data(const void* data, size_t data_size, void* context) {
    furi_assert(data);
    furi_assert(context);
    furi_check(data_size == sizeof(IntercomMetaFrame));
    Intercom* intercom = context;
    const IntercomMetaFrame* frame = data;

    switch(frame->type) {
    case IntercomMetaTypeChannelReady:
        intercom_meta_channel_ready(intercom, frame);
        break;
    case IntercomMetaTypeMax:
        furi_crash();
    }
}

void intercom_channel_init(IntercomChannel* channel, Intercom* intercom) {
    furi_assert(channel);
    furi_assert(intercom);
    channel->intercom = intercom;
    channel->flags = furi_event_flag_alloc();

    IntercomChannelId channel_id = intercom_channel_id(channel);
    if(channel_id == IntercomChannelIdMeta) {
        intercom_channel_set_callback(channel, intercom_meta_channel_data, intercom);
    }
}

void intercom_channel_set_callback(
    IntercomChannel* channel,
    IntercomRxCallback callback,
    void* context) {
    furi_assert(channel);
    if(context) furi_assert(callback);

    channel->rx_callback = callback;
    channel->callback_context = context;
}

void intercom_channel_call_callback(IntercomChannel* channel, const IntercomFrame* rx_frame) {
    furi_assert(channel);

    IntercomRxCallback callback = channel->rx_callback;
    furi_check(callback, "rx_callback==NULL, other side sent data");

    callback(rx_frame->data, rx_frame->data_size, channel->callback_context);
}

void intercom_channel_send_ready(IntercomChannel* channel) {
    furi_assert(channel);
    IntercomChannelId channel_id = intercom_channel_id(channel);
    furi_check(channel_id != IntercomChannelIdMeta);

    IntercomFrame* tx_frame = intercom_do_acquire_tx(channel->intercom);
    tx_frame->channel_id = IntercomChannelIdMeta;

    IntercomMetaFrame* frame = (IntercomMetaFrame*)tx_frame->data;
    frame->type = IntercomMetaTypeChannelReady;
    frame->channel_ready.channel_id = channel_id;

    tx_frame->data_size = sizeof(*frame);
    tx_frame->check = intercom_frame_get_checksum(tx_frame);
    intercom_do_tx(channel->intercom);

    FURI_LOG_D(TAG, "THIS side ready: %s", intercom_channel_id_name(channel_id));
}

bool intercom_channel_await_peer_ready(IntercomChannel* channel, FuriWait timeout) {
    furi_assert(channel);
    IntercomChannelId channel_id = intercom_channel_id(channel);
    furi_check(channel_id != IntercomChannelIdMeta);

    const uint32_t expecting = IntercomChannelFlagPeerReady;
    uint32_t flags = furi_event_flag_wait(
        channel->flags, expecting, FuriFlagNoClear | FuriFlagWaitAll, timeout);

    if(flags == FuriFlagErrorTimeout) return false;
    if((timeout == 0) && (flags == FuriFlagErrorResource)) return false;

    furi_check(!(flags & FuriFlagError));
    return true;
}
