#include "screen_streamer.h"
#include <furi/furi.h>
#include <time/time.h>
#include <front_display/front_display.h>
#include <back_display/back_display.h>
#include <toolbox/color.h>
#include <toolbox/rle_encode.h>
#include <toolbox/crc32_calc.h>

#define MAX_MESSAGES 2

#define TAG "ScrStrm"

typedef struct Output {
    bool is_valid;
    uint32_t frame_interval_ms;
    time_t last_frame_timestamp_ms;
    uint32_t last_hash;
    bool last_frame_skipped;
} Output;

typedef struct ScreenStreamer {
    Gui* gui;

    FuriThread* thread;
    FuriMessageQueue* thread_command_queue;

    GuiDisplayId display_id;
    Output outputs[StatePublisherTransportClassMax];
    FuriMutex* outputs_mutex;

    ScreenStreamerPixelFormat pixel_format;
    uint32_t image_w;
    uint32_t image_h;
    size_t conversion_buf_size;
    void* conversion_buf;
    size_t compression_buf_size;
    void* compression_buf;

    ScreenStreamerFrameCb cb;
    void* cb_context;
} ScreenStreamer;

typedef enum Message {
    MSG_STOP,
    MSG_OUTPUTS_CHANGED,
} Message;

static int32_t screen_streamer_thread(void* context);

ScreenStreamer*
    screen_streamer_alloc(GuiDisplayId display, Gui* gui, ScreenStreamerFrameCb cb, void* context) {
    ScreenStreamer* instance = malloc(sizeof(ScreenStreamer));

    instance->gui = gui;

    instance->thread =
        furi_thread_alloc_ex("ScreenStreamer", 1 * 1024, screen_streamer_thread, instance);
    instance->thread_command_queue = furi_message_queue_alloc(MAX_MESSAGES, sizeof(Message));

    instance->display_id = display;
    instance->outputs_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    switch(display) {
    case GuiDisplayIdFront:
        instance->pixel_format = ScreenStreamerPixelFormatR8G8B8;
        instance->image_w = FRONT_DISPLAY_W;
        instance->image_h = FRONT_DISPLAY_H;
        instance->conversion_buf_size = FRONT_DISPLAY_BUF_SIZE;
        break;
    case GuiDisplayIdBack:
        instance->pixel_format = ScreenStreamerPixelFormatL4;
        instance->image_w = BACK_DISPLAY_W;
        instance->image_h = BACK_DISPLAY_H;
        instance->conversion_buf_size = BACK_DISPLAY_BUF_SIZE / 2; // L8->L4 conversion
        break;
    default:
        furi_check(false);
        break;
    }
    instance->conversion_buf = malloc(instance->conversion_buf_size);
    instance->compression_buf_size =
        instance->conversion_buf_size; // at max same size, otherwise compression failed
    instance->compression_buf = malloc(instance->compression_buf_size);

    instance->cb = cb;
    instance->cb_context = context;

    return instance;
}

void screen_streamer_start(ScreenStreamer* instance) {
    furi_thread_start(instance->thread);
}

void screen_streamer_stop(ScreenStreamer* instance) {
    if(furi_thread_get_state(instance->thread) == FuriThreadStateRunning) {
        Message stop = MSG_STOP;
        furi_message_queue_put(instance->thread_command_queue, &stop, FuriWaitForever);
        furi_thread_join(instance->thread);
    }
}

void screen_streamer_free(ScreenStreamer* instance) {
    screen_streamer_stop(instance);
    furi_mutex_free(instance->outputs_mutex);
    furi_message_queue_free(instance->thread_command_queue);
    furi_thread_free(instance->thread);
    free(instance->compression_buf);
    free(instance->conversion_buf);
    free(instance);
}

void screen_streamer_enable_output(
    ScreenStreamer* instance,
    StatePublisherTransportClass transport_class,
    uint32_t frame_interval_ms) {
    furi_mutex_acquire(instance->outputs_mutex, FuriWaitForever);
    Output* output = instance->outputs + transport_class;
    if(output->is_valid) {
        output->frame_interval_ms = MIN(frame_interval_ms, output->frame_interval_ms);
    } else {
        output->frame_interval_ms = frame_interval_ms;
        output->is_valid = true;
    }
    output->last_frame_timestamp_ms = 0;
    output->last_hash = 0;
    output->last_frame_skipped = false;
    furi_mutex_release(instance->outputs_mutex);

    if(furi_thread_get_state(instance->thread) == FuriThreadStateRunning) {
        Message msg = MSG_OUTPUTS_CHANGED;
        furi_message_queue_put(instance->thread_command_queue, &msg, FuriWaitForever);
    }
}

void screen_streamer_disable_output(
    ScreenStreamer* instance,
    StatePublisherTransportClass transport_class) {
    furi_mutex_acquire(instance->outputs_mutex, FuriWaitForever);
    bool changed = instance->outputs[transport_class].is_valid;
    instance->outputs[transport_class].is_valid = false;
    furi_mutex_release(instance->outputs_mutex);

    if(changed) {
        Message msg = MSG_OUTPUTS_CHANGED;
        furi_message_queue_put(instance->thread_command_queue, &msg, FuriWaitForever);
    }
}

/**
 * Dispatch frame and calculate sleep time.
 *
 * @param frame if NULL, only calculate sleep time.
 * @return sleep time in ms.
 */
static uint32_t dispatch_frame(ScreenStreamer* instance, const ScreenStreamerFrame* frame) {
    uint32_t hash = 0;
    if(frame) {
        hash = crc32_calc_buffer(0x12345678, frame->data, frame->data_size); // TODO xxhash
    }
    uint8_t stream_flags = 0;
    furi_mutex_acquire(instance->outputs_mutex, FuriWaitForever);
    time_t now = time_get_timestamp_ms();
    uint32_t time_to_next_update = UINT32_MAX;
    for(size_t i = 0; i != StatePublisherTransportClassMax; ++i) {
        Output* output = instance->outputs + i;
        if(output->is_valid) {
            if(frame) {
                time_t elapsed = now - output->last_frame_timestamp_ms;
                bool frame_due = elapsed >= output->frame_interval_ms;
                bool frame_differs = hash != output->last_hash;

                if(frame_differs) {
                    if(frame_due || output->last_frame_skipped) {
                        // send frame
                        stream_flags |= 1 << i;
                        output->last_frame_skipped = false;
                        output->last_hash = hash;
                        output->last_frame_timestamp_ms = now;
                    }
                } else if(frame_due) {
                    // skip frame
                    output->last_frame_skipped = true;
                    output->last_frame_timestamp_ms = now;
                }
            }

            time_t next_due = output->last_frame_timestamp_ms + output->frame_interval_ms;
            if(next_due <= now) {
                time_to_next_update = 0;
            } else {
                time_to_next_update = MIN(time_to_next_update, next_due - now);
            }
        }
    }
    furi_mutex_release(instance->outputs_mutex);
    if(stream_flags) {
        instance->cb(instance->display_id, frame, stream_flags, instance->cb_context);
    }
    return time_to_next_update;
}

static ScreenStreamerFrame get_frame(ScreenStreamer* instance) {
    with_gui(instance->gui, {
        const void* frame_data = gui_display_get_frame_buffer(instance->gui, instance->display_id);
        switch(instance->display_id) {
        case GuiDisplayIdFront:
            memcpy(instance->conversion_buf, frame_data, FRONT_DISPLAY_BUF_SIZE);
            break;
        case GuiDisplayIdBack:
            color_buf_l8_to_l4(instance->conversion_buf, frame_data, BACK_DISPLAY_BUF_SIZE);
            break;
        default:
            furi_check(false);
            break;
        }
    });

    const uint8_t blk_size = instance->display_id == GuiDisplayIdFront ? 3 : 2;
    size_t compressed_data_len = 0;
    bool compression_ok = rle_compress(
        instance->conversion_buf,
        instance->conversion_buf_size,
        instance->compression_buf,
        instance->compression_buf_size,
        blk_size,
        &compressed_data_len);

    ScreenStreamerFrame frame = {
        .compression = compression_ok ? ScreenStreamerCompressionRLE :
                                        ScreenStreamerCompressionPlain,
        .data = compression_ok ? instance->compression_buf : instance->conversion_buf,
        .data_size = compression_ok ? compressed_data_len : instance->conversion_buf_size,
        .pixel_format = instance->pixel_format,
        .width = instance->image_w,
        .height = instance->image_h,
    };
    return frame;
}

static int32_t screen_streamer_thread(void* context) {
    ScreenStreamer* instance = context;
    bool run = true;
    bool needs_frame = false;
    while(run) {
        uint32_t sleep_time_ms = 0;
        if(needs_frame) {
            needs_frame = false;
            ScreenStreamerFrame frame = get_frame(instance);
            sleep_time_ms = dispatch_frame(instance, &frame);
        } else {
            sleep_time_ms = dispatch_frame(instance, NULL);
        }

        uint32_t sleep_time_ticks = furi_ms_to_ticks(sleep_time_ms);

        // sleep and wait for message
        Message msg;
        switch(furi_message_queue_get(instance->thread_command_queue, &msg, sleep_time_ticks)) {
        case FuriStatusOk:
            switch(msg) {
            case MSG_STOP:
                run = false;
                break;
            case MSG_OUTPUTS_CHANGED:
                break;
            default:
                furi_assert(false);
                break;
            }
            break;
        case FuriStatusErrorTimeout:
        case FuriStatusErrorResource:
            needs_frame = true;
            break;
        default:
            furi_assert(false);
            break;
        }
    }
    return 0;
}
