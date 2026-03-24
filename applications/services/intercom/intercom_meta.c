#include "intercom_i.h"

#define TAG "IntercomMeta"

#define INTERCOM_META_CHANNEL_ID (IntercomChannelIdMax)

typedef enum {
    IntercomMetaFrameTypeChannelReady,
    IntercomMetaFrameTypeHeartbeat,
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

static void intercom_meta_handle_channel_ready(
    Intercom* instance,
    const IntercomMetaFrameChannelReady* frame) {
    const IntercomChannelId channel_id = frame->channel_id;
    furi_check(channel_id < IntercomChannelIdMax);

    IntercomChannel* channel = &instance->channels[channel_id];
    intercom_channel_mark_as_ready(channel);

    FURI_LOG_D(TAG, "OTHER side ready: %s", intercom_channel_get_name(channel_id));
}

static void intercom_meta_handle_heartbeat_received(Intercom* instance) {
    UNUSED(instance);
    INTERCOM_LOG_D("Heartbeat received");
}

static void intercom_meta_send_frame(Intercom* instance, const IntercomMetaFrame* frame) {
    const size_t tx_size = intercom_tx_internal(
        instance, INTERCOM_META_CHANNEL_ID, frame, sizeof(IntercomMetaFrame), FuriWaitForever);
    furi_check(tx_size == sizeof(IntercomMetaFrame));
}

void intercom_meta_activate_channel(Intercom* instance, IntercomChannelId channel_id) {
    const IntercomMetaFrame frame = {
        .type = IntercomMetaFrameTypeChannelReady,
        .channel_ready.channel_id = channel_id,
    };

    intercom_meta_send_frame(instance, &frame);

    FURI_LOG_D(TAG, "THIS side ready: %s", intercom_channel_get_name(channel_id));
}

void intercom_meta_send_heartbeat(Intercom* instance) {
    const IntercomMetaFrame frame = {
        .type = IntercomMetaFrameTypeHeartbeat,
    };

    intercom_meta_send_frame(instance, &frame);

    INTERCOM_LOG_D(TAG, "Heartbeat sent");
}

void intercom_meta_process_frame(Intercom* instance, const IntercomFrame* frame) {
    furi_assert(frame->channel_id == INTERCOM_META_CHANNEL_ID);
    furi_assert(frame->data_size == sizeof(IntercomMetaFrame));

    const IntercomMetaFrame* meta_frame = (const IntercomMetaFrame*)frame->data;
    const IntercomMetaFrameType type = meta_frame->type;

    if(type == IntercomMetaFrameTypeChannelReady) {
        intercom_meta_handle_channel_ready(instance, &meta_frame->channel_ready);
    } else if(type == IntercomMetaFrameTypeHeartbeat) {
        intercom_meta_handle_heartbeat_received(instance);
    } else {
        furi_crash("Invalid IntercomMetaFrameType");
    }
}
