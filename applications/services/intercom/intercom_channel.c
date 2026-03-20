#include "intercom_i.h"

#define TAG "IntercomChannel"

typedef enum {
    IntercomChannelFlagPeerReady = (1UL << 0),
} IntercomChannelFlag;

typedef enum {
    IntercomMetaFrameTypeChannelReady,
    IntercomMetaFrameTypeMax,
} IntercomMetaFrameType;

typedef struct {
    IntercomChannelId channel_id;
} IntercomMetaFrameChannelReady;

typedef struct {
    IntercomMetaFrameType type;
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
    [IntercomChannelIdSlInfo] = "SlInfo",
    [IntercomChannelIdDebug] = "Debug",
};

static FURI_ALWAYS_INLINE const char*
    intercom_channel_get_name_by_id(IntercomChannelId channel_id) {
    furi_check(channel_id < IntercomChannelIdMax);
    return intercom_channel_names[channel_id];
}

static FURI_ALWAYS_INLINE IntercomChannelId
    intercom_channel_get_id(const IntercomChannel* channel) {
    furi_assert(channel);
    return channel - channel->owner->channels;
}

static void
    intercom_channel_handle_other_side_ready(Intercom* intercom, const IntercomMetaFrame* frame) {
    furi_assert(intercom);
    furi_assert(frame);
    furi_assert(frame->type == IntercomMetaFrameTypeChannelReady);
    const IntercomMetaFrameChannelReady* sub_frame = &frame->channel_ready;

    IntercomChannelId channel_id = sub_frame->channel_id;
    furi_check(channel_id < IntercomChannelIdMax);

    FURI_LOG_D(TAG, "OTHER side ready: %s", intercom_channel_get_name_by_id(channel_id));

    const IntercomChannel* channel = &intercom->channels[channel_id];
    furi_check(
        furi_event_flag_set(channel->flags, IntercomChannelFlagPeerReady) ==
        IntercomChannelFlagPeerReady);
}

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
        FURI_LOG_W(TAG, "rx_callback is NULL, but other side is sending data");
    }
}

void intercom_meta_process_frame(Intercom* instance, const IntercomFrame* frame) {
    const IntercomMetaFrame* meta_frame = (const IntercomMetaFrame*)frame->data;
    const IntercomMetaFrameType type = meta_frame->type;

    if(type == IntercomMetaFrameTypeChannelReady) {
        intercom_channel_handle_other_side_ready(instance, meta_frame);
    } else {
        furi_crash("Invalid IntercomMetaFrameType");
    }
}

void intercom_channel_send_ready(IntercomChannel* channel) {
    furi_assert(channel);

    const IntercomChannelId channel_id = intercom_channel_get_id(channel);

    const IntercomMetaFrame frame = {
        .type = IntercomMetaFrameTypeChannelReady,
        .channel_ready.channel_id = channel_id,
    };

    const size_t tx_size = intercom_tx_internal(
        channel->owner, IntercomChannelIdMax, &frame, sizeof(frame), FuriWaitForever);
    furi_check(tx_size == sizeof(frame));

    FURI_LOG_D(TAG, "THIS side ready: %s", intercom_channel_get_name_by_id(channel_id));
}

bool intercom_channel_wait_for_other_side(IntercomChannel* channel, uint32_t timeout) {
    furi_assert(channel);

    const uint32_t flags = furi_event_flag_wait(
        channel->flags, IntercomChannelFlagPeerReady, FuriFlagNoClear | FuriFlagWaitAny, timeout);

    bool success;

    if(flags == IntercomChannelFlagPeerReady) {
        success = true;
    } else {
        furi_check(flags == (timeout ? FuriFlagErrorTimeout : FuriFlagErrorResource));
        success = false;
    }

    return success;
}
