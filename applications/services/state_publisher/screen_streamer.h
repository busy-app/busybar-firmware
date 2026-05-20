#pragma once
#include <gui/gui.h>
#include "state_publisher.h"

typedef struct ScreenStreamer ScreenStreamer;

typedef enum ScreenStreamerPixelFormat {
    ScreenStreamerPixelFormatR8G8B8,
    ScreenStreamerPixelFormatL8,
    ScreenStreamerPixelFormatL4,
} ScreenStreamerPixelFormat;

typedef enum ScreenStreamerCompression {
    ScreenStreamerCompressionPlain,
    ScreenStreamerCompressionRLE,
} ScreenStreamerCompression;

typedef struct ScreenStreamerFrame {
    /// Width in pixels.
    uint32_t width;
    /// Height in pixels.
    uint32_t height;
    /// Pixel format.
    ScreenStreamerPixelFormat pixel_format;
    /// Compression method.
    ScreenStreamerCompression compression;
    /// Frame data. Points to an internal buffer, not thread-safe.
    const void* data;
    /// Data size in bytes.
    size_t data_size;
} ScreenStreamerFrame;

typedef void (*ScreenStreamerFrameCb)(
    GuiDisplayId display,
    const ScreenStreamerFrame* frame,
    uint8_t stream_flags,
    void* context);

ScreenStreamer*
    screen_streamer_alloc(GuiDisplayId display, Gui* gui, ScreenStreamerFrameCb cb, void* context);

void screen_streamer_free(ScreenStreamer* instance);

void screen_streamer_start(ScreenStreamer* instance);

void screen_streamer_stop(ScreenStreamer* instance);

void screen_streamer_enable_output(
    ScreenStreamer* instance,
    StatePublisherTransportClass transport_class,
    uint32_t frame_interval_ms);

void screen_streamer_disable_output(
    ScreenStreamer* instance,
    StatePublisherTransportClass transport_class);

void screen_streamer_get_single_frame(
    ScreenStreamer* instance,
    ScreenStreamerFrameCb cb,
    void* context);
