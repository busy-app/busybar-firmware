#pragma once
#include <gui/gui.h>

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

typedef void (*ScreenStreamerFrameCb)(GuiDisplayId display, const ScreenStreamerFrame* frame, void* context);

typedef size_t ScreenStreamerSubscriptionId;

ScreenStreamer* screen_streamer_alloc(GuiDisplayId display, Gui* gui);

void screen_streamer_free(ScreenStreamer* instance);

void screen_streamer_start(ScreenStreamer* instance);

void screen_streamer_stop(ScreenStreamer* instance);

ScreenStreamerSubscriptionId screen_streamer_subscrube(
    ScreenStreamer* instance,
    uint32_t framerate_ms,
    ScreenStreamerFrameCb cb,
    void* context);

void screen_streamer_unsubscrube(ScreenStreamer* instance, ScreenStreamerSubscriptionId id);
