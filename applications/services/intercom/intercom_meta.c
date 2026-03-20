#include "intercom_i.h"

#define TAG "IntercomMeta"

#define INTERCOM_META_CHANNEL_ID (IntercomChannelIdMax)

typedef enum {
    IntercomMetaFrameTypeChannelReady,
    IntercomMetaFrameTypeKeepAlive,
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

void intercom_meta_activate_channel(Intercom* instance, IntercomChannelId channel_id) {
    const IntercomMetaFrame frame = {
        .type = IntercomMetaFrameTypeChannelReady,
        .channel_ready.channel_id = channel_id,
    };

    const size_t tx_size = intercom_tx_internal(
        instance, INTERCOM_META_CHANNEL_ID, &frame, sizeof(frame), FuriWaitForever);
    furi_check(tx_size == sizeof(frame));

    FURI_LOG_D(TAG, "THIS side ready: %s", intercom_channel_get_name(channel_id));
}

void intercom_meta_process_frame(Intercom* instance, const IntercomFrame* frame) {
    furi_assert(frame->data_size == sizeof(IntercomMetaFrame));

    const IntercomMetaFrame* meta_frame = (const IntercomMetaFrame*)frame->data;
    const IntercomMetaFrameType type = meta_frame->type;

    if(type == IntercomMetaFrameTypeChannelReady) {
        intercom_meta_handle_channel_ready(instance, &meta_frame->channel_ready);
    } else if(type == IntercomMetaFrameTypeKeepAlive) {
        // TODO: Implement keepalive
    } else {
        furi_crash("Invalid IntercomMetaFrameType");
    }
}
